
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


        DBG("tcp_server_init(): Use '%s:%d', listening\n", inet_ntoa(_srv->serveraddr.sin_addr), _port);

        return modbus_success;
    }
    return -modbus_invalid_argument;
}

int emb_tcp_server_deinit(emb_tcp_server_t* _srv)
{
    if(_srv) {
        if(_srv->listener >= 0) {
            close(_srv->listener);
            _srv->listener = -1;
        }
        return modbus_success;
    }
    return -modbus_invalid_argument;
}

int emb_tcp_server_receive(emb_tcp_server_t* _srv, int *_client_id,
                           uint8_t* _buffer, unsigned int _buf_size)
{
    if(_srv && _buffer && _buf_size) {
        int i;
        socklen_t addrlen;
        int newfd;
        int nbytes;
        int sel_res;
        struct sockaddr_in clientaddr;

        _srv->read_fds = _srv->master;

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = _srv->rx_timeout_ms * 1000;
        sel_res = select(_srv->fdmax+1, &_srv->read_fds, NULL, NULL, &tv);

        if(sel_res == 0) {
            //DBG("select(): timeout\n");
            return -modbus_timeout;
        }
        else if(sel_res < 0) {
            ERR("Error with select(): %m\n");
            return -errno;
        }

        // run through the existing connections looking for data to be read

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

                        // close it
                        close(i);

                        // remove from master set
                        FD_CLR(i, &_srv->master);
                    }
                    else {
                        if(_client_id)
                            *_client_id = i;
                        return nbytes;
                    }
                }
            }
        }

        for(i = 0; i <= _srv->fdmax; i++) {
            if(FD_ISSET(i, &_srv->read_fds)) {
                // we got one...
                if(i == _srv->listener) {
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
                        }
                        else {
                            DBG("Discard from connection, because maximum connections (%d) are reached\n", _srv->max_connections);
                            // close it
                            close(newfd);
                        }


                    }
                }
            }
        }
        return 0;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_tcp_server_send(emb_tcp_server_t* _srv, int _client_id,
                        const uint8_t* _data, unsigned int _data_length)
{
    if(_srv && _client_id >= 0 && _data && _data_length) {
        ssize_t res = send(_client_id, _data, _data_length, 0);
        printf("Res: %ld\n", res);
        return (int)res;
    }
    else {
        return -modbus_invalid_argument;
    }
}
