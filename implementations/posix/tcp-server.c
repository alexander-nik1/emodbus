
#include <emodbus/impl/posix/tcp-server.h>
#include <emodbus/base/modbus_errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DBG(...)    //fputs(__FUNCTION__, stdout), printf("(): " __VA_ARGS__)
#define ERR(...)    fputs(__FUNCTION__, stderr), fprintf(stderr, "(): " __VA_ARGS__)

int emb_tcp_server_init(emb_tcp_server_t* _srv, in_addr_t _addr, uint16_t _port)
{
    if(_srv) {
        const int yes = 1;

        FD_ZERO(&_srv->master);
        FD_ZERO(&_srv->read_fds);

        _srv->conn_counter = 0;

        // get the listener
        if((_srv->listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            ERR("Error with socket(): %m\n");
            return -errno;
        }
        // Supress "address already in use" error message
        if(setsockopt(_srv->listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            ERR("Error with setsockopt(): %m\n");
            return -errno;
        }

        _srv->serveraddr.sin_family = AF_INET;
        _srv->serveraddr.sin_addr.s_addr = _addr;
        _srv->serveraddr.sin_port = htons(_port);
        memset(&(_srv->serveraddr.sin_zero), 0, sizeof(_srv->serveraddr.sin_zero));

        if(bind(_srv->listener, (struct sockaddr *)&_srv->serveraddr, sizeof(_srv->serveraddr)) == -1) {
            ERR("Error with bind(): %m\n");
            return -errno;
        }

        if(listen(_srv->listener, 10) == -1) {
            ERR("Error with listen(): %m\n");
            return -errno;
        }

        // add the listener to the master set
        FD_SET(_srv->listener, &_srv->master);

        // keep track of the biggest file descriptor
        _srv->fdmax = _srv->listener;

        _srv->clients = (emb_tcp_server_client_t*)malloc(_srv->max_connections * sizeof(emb_tcp_server_client_t));

        DBG("tcp_server_init(): Use '%s:%d', listening\n", inet_ntoa(_srv->serveraddr.sin_addr), _port);

        return modbus_success;
    }
    return -modbus_invalid_argument;
}

static void emb_tcp_server_add_client(emb_tcp_server_t* _srv, int _fd, struct sockaddr_in* _sa)
{
    if (_srv->clients) {
        unsigned int i;
        for (i=0; i<_srv->max_connections; ++i) {
            if (!_srv->clients[i].active) {
                _srv->clients[i].active = 1;
                _srv->clients[i].fd = _fd;
                _srv->clients[i].addr = *_sa;
                gettimeofday(&_srv->clients[i].connect_time, NULL);
                gettimeofday(&_srv->clients[i].last_activity_time, NULL);
                return;
            }
        }
    }
}

static emb_tcp_server_client_t* emb_tcp_server_find_client(emb_tcp_server_t* _srv, int _fd)
{
    if (_srv->clients) {
        unsigned int i;
        for (i=0; i<_srv->max_connections; ++i) {
            if (_srv->clients[i].fd == _fd && _srv->clients[i].active) {
                return &_srv->clients[i];
            }
        }
    }
    return NULL;
}

static void emb_tcp_server_close_client(emb_tcp_server_t* _srv, emb_tcp_server_client_t* _client)
{
    _client->active = 0;
    close(_client->fd);
    FD_CLR(_client->fd, &_srv->master);
}

static void emb_tcp_server_disconnect_client(emb_tcp_server_t* _srv, int _fd)
{
    emb_tcp_server_client_t* client = emb_tcp_server_find_client(_srv, _fd);
    if (client) {
        emb_tcp_server_close_client(_srv, client);
    }
}

static void emb_tcp_server_disconnect_all_clients(emb_tcp_server_t* _srv)
{
    if (_srv->clients) {
        unsigned int i;
        for (i=0; i<_srv->max_connections; ++i) {
            emb_tcp_server_close_client(_srv, &_srv->clients[i]);
        }
    }
}

int emb_tcp_server_deinit(emb_tcp_server_t* _srv)
{
    if(_srv) {
        if(_srv->listener >= 0) {
            close(_srv->listener);
            _srv->listener = -1;
        }

        if (_srv->clients)
        {
            emb_tcp_server_disconnect_all_clients(_srv);
            free(_srv->clients);
            _srv->clients = NULL;
        }

        return modbus_success;
    }
    return -modbus_invalid_argument;
}

static long get_time_period_ms_from(const struct timeval* _from)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return ((now.tv_sec - _from->tv_sec) * 1000 + (now.tv_usec - _from->tv_usec)/1000);
}

static void emb_tcp_server_check_timeouts(emb_tcp_server_t* _srv)
{
    if (_srv->clients) {
        unsigned int i;
        for (i=0; i<_srv->max_connections; ++i) {
            if (_srv->clients[i].active) {

                if (_srv->connection_reset_timeout_ms > 0) {
                    if (get_time_period_ms_from(&_srv->clients[i].connect_time) >= _srv->connection_reset_timeout_ms) {
                        emb_tcp_server_close_client(_srv, &_srv->clients[i]);
                        _srv->conn_counter--;

                        DBG("Socket %d connection closed because connection timeout, connections:%d\n", _srv->clients[i].fd, _srv->conn_counter);
                    }
                }

                if (_srv->connection_idle_to_reset_ms > 0) {
                    if (get_time_period_ms_from(&_srv->clients[i].last_activity_time) >= _srv->connection_idle_to_reset_ms) {
                        emb_tcp_server_close_client(_srv, &_srv->clients[i]);
                        _srv->conn_counter--;

                        DBG("Socket %d connection closed because idle timeout, connections:%d\n", _srv->clients[i].fd, _srv->conn_counter);
                    }
                }
            }
        }
    }
}

int emb_tcp_server_receive(emb_tcp_server_t* _srv,
                           emb_tcp_server_client_t** _client,
                           uint8_t* _buffer,
                           unsigned int _buf_size)
{
    if(_srv && _buffer && _buf_size) {
        int i;
        socklen_t addrlen;
        int newfd;
        int nbytes;
        int sel_res;
        struct sockaddr_in clientaddr;

        emb_tcp_server_check_timeouts(_srv);

        _srv->read_fds = _srv->master;

        if(_client)
            *_client = NULL;

        if (_srv->rx_timeout_ms >= 0) {
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = _srv->rx_timeout_ms * 1000;
            sel_res = select(_srv->fdmax+1, &_srv->read_fds, NULL, NULL, &tv);
        }
        else {
            sel_res = select(_srv->fdmax+1, &_srv->read_fds, NULL, NULL, NULL);
        }

        if(sel_res == 0) {
            //DBG("select(): timeout\n");
            return -modbus_timeout;
        }
        else if(sel_res < 0) {
            ERR("Error with select(): %m\n");
            return -errno;
        }

        // run through the existing connections looking for data to be read,
        // for errors and disconnections.

        for(i = 0; i <= _srv->fdmax; i++) {
            if(FD_ISSET(i, &_srv->read_fds)) {
                // we got one...
                if(i != _srv->listener) {
                    // handle data from a client
                    if((nbytes = (int)recv(i, _buffer, _buf_size, 0)) <= 0) {
                        // got error or connection closed by client

                        _srv->conn_counter--;

                        if(nbytes == 0) {
                            DBG("Socket %d connection closed, connections:%d\n", i, _srv->conn_counter);
                        }
                        else {
                            ERR("Error with recv(): %m\n");
                        }

                        emb_tcp_server_disconnect_client(_srv, i);
                    }
                    else {
                        if(_client) {
                            *_client = emb_tcp_server_find_client(_srv, i);
                            gettimeofday(&(*_client)->last_activity_time, NULL);
                        }
                        return nbytes;
                    }
                }
            }
        }

        if(FD_ISSET(_srv->listener, &_srv->read_fds)) {
            // we got one...
            // handle new connections
            addrlen = sizeof(clientaddr);
            if((newfd = accept(_srv->listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1) {
                ERR("Error with accept(): %m\n");
                return -errno;
            }
            else {
                if (_srv->conn_counter < _srv->max_connections) {
                    _srv->conn_counter++;

                    FD_SET(newfd, &_srv->master); // add to master set
                    if(newfd > _srv->fdmax) {
                        // keep track of the maximum
                        _srv->fdmax = newfd;
                    }

                    DBG("Accepted new coneection: '%s:%d', sock:%d, connections:%d\n",
                        inet_ntoa(clientaddr.sin_addr),
                        clientaddr.sin_port,
                        newfd,
                        _srv->conn_counter);

                    emb_tcp_server_add_client(_srv, newfd, &clientaddr);
                }
                else {
                    DBG("Discard from connection, because maximum connections (%d) are reached\n", _srv->max_connections);
                    // close it
                    close(newfd);
                }
            }
        }
        return 0;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_tcp_server_send(emb_tcp_server_t* _srv,
                        emb_tcp_server_client_t* _client,
                        const uint8_t* _data,
                        unsigned int _data_length)
{
    if(_srv && _data && _data_length && _client && _client->active) {
        gettimeofday(&_client->last_activity_time, NULL);
        return (int)send(_client->fd, _data, _data_length, 0);
    }
    else {
        return -modbus_invalid_argument;
    }
}
