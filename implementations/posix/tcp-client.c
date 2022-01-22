
#include <emodbus/impl/posix/tcp-client.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <sys/time.h>

#define DBG(...)    //fputs(__FUNCTION__, stdout), printf("(): " __VA_ARGS__)
#define ERR(...)    fputs(__FUNCTION__, stderr), fprintf(stderr, "(): " __VA_ARGS__)

static const char* str_state(emb_tcp_client_state_t _state)
{
    static const char* str_states[] = {
        "connecting",
        "connected",
        "disconnected"
    };

    static const char* str_unknown = "?unknown?";

    if(_state <= emb_tcs_disconnected)
        return str_states[_state];
    else
        return str_unknown;
}

static void emb_tcp_srv_ch_state(emb_tcp_client_t* _cli, emb_tcp_client_state_t _new_state)
{
    if(_cli->state != _new_state) {
        DBG(" %s -> %s\n", str_state(_cli->state), str_state(_new_state));
        _cli->state = _new_state;
    }
}

static int emb_tcp_client_is_connected(const emb_tcp_client_t* _cli)
{
    return _cli->state == emb_tcs_connected;
}

static int emb_tcp_client_close(emb_tcp_client_t* _cli)
{
    DBG("\n");
    if(emb_tcp_client_is_connected(_cli)) {
        shutdown(_cli->fd, SHUT_RDWR);
        close(_cli->fd);
    }
    gettimeofday(&_cli->disconnect_time, NULL);
    emb_tcp_srv_ch_state(_cli, emb_tcs_disconnected);
    _cli->fd = -1;
    return 0;
}

static void emb_tcp_client_set_blocking(emb_tcp_client_t* _cli, int _enable)
{
    long arg;
    arg = fcntl(_cli->fd, F_GETFL, NULL);
    if(_enable)
        arg &= (~O_NONBLOCK);
    else
        arg |= O_NONBLOCK;
    fcntl(_cli->fd, F_SETFL, arg);
}

static long get_time_period_ms_from(const struct timeval* _from)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return ((now.tv_sec - _from->tv_sec) * 1000 + (now.tv_usec - _from->tv_usec)/1000);
}

#define get_time_period_s_from(_from_) (get_time_period_ms_from(_from_) / 1000)

static void timeval_set_ms(struct timeval* _tv, long _ms)
{
    _tv->tv_sec = _ms / 1000;
    _tv->tv_usec = (_ms % 1000) * 1000;
}

static int emb_tcp_client_start_connect(emb_tcp_client_t* _cli)
{
    int ret = 0;

    _cli->fd = socket(AF_INET, SOCK_STREAM, 0);
    if(_cli->fd < 0) {
        ERR("Error with socket(): %m\n");
        gettimeofday(&_cli->disconnect_time, NULL);
        emb_tcp_srv_ch_state(_cli, emb_tcs_disconnected);
        return -1;
    }

    //int value = 1;
    //setsockopt(_cli->fd, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));

    emb_tcp_client_set_blocking(_cli, 0);

    ret = connect(_cli->fd, (struct sockaddr*)&_cli->serveraddr, sizeof(_cli->serveraddr));
    if(ret == 0) {
        gettimeofday(&_cli->connect_time, NULL);
        emb_tcp_srv_ch_state(_cli, emb_tcs_connected);
        emb_tcp_client_set_blocking(_cli, 1);
        _cli->is_first_reconnect = 1;
        _cli->rx_timeouts_counter = 0;
        _cli->tx_timeouts_counter = 0;
        return 0;
    }

    if(ret < 0) {
        if(errno == EINPROGRESS) {
            emb_tcp_srv_ch_state(_cli, emb_tcs_connecting);
            ret = -EINPROGRESS;
        }
    }

    return ret;
}

static int emb_tcp_client_wait4connect(emb_tcp_client_t* _cli)
{
    fd_set wr_fds;
    int sel_res;
    struct timeval tv;

    if(_cli->flags & EMB_TCP_CLI_NO_DELAY_WHILE_CONNECT)
        timeval_set_ms(&tv, 0);
    else
        timeval_set_ms(&tv, _cli->connect_timeout_ms);

    FD_ZERO(&wr_fds);

    FD_SET(_cli->fd, &wr_fds);

    sel_res = select(_cli->fd + 1, NULL, &wr_fds, NULL, &tv);
    if(sel_res == 0) {
        if(_cli->flags & EMB_TCP_CLI_NO_DELAY_WHILE_CONNECT) {
            if(get_time_period_ms_from(&_cli->connection_start_time) >= _cli->connect_timeout_ms) {
                DBG("Timeout for connection\n");
                emb_tcp_client_close(_cli);
                return -ETIMEDOUT;
            }
        }
        DBG("Timeout for connection\n");
        emb_tcp_client_close(_cli);
        return -ETIMEDOUT;
    }
    else if(sel_res < 0) {
        ERR("select() error: %m\n");
        emb_tcp_client_close(_cli);
        return sel_res;
    }
    else {
        int so_error = -1;
        socklen_t len = sizeof(so_error);
        if(getsockopt(_cli->fd, SOL_SOCKET, SO_ERROR, (void*)&so_error, &len) != 0) {
            ERR("Error with getsockopt(): %m\n");
            emb_tcp_client_close(_cli);
            return -errno;
        }
        if(so_error) {
            ERR("Error while connection(): %d\n", so_error);
            emb_tcp_client_close(_cli);
            return -so_error;
        }
        else {    // Successful connection
            gettimeofday(&_cli->connect_time, NULL);
            emb_tcp_srv_ch_state(_cli, emb_tcs_connected);
            emb_tcp_client_set_blocking(_cli, 1);
            _cli->is_first_reconnect = 1;
            _cli->rx_timeouts_counter = 0;
            _cli->tx_timeouts_counter = 0;
            return 0;
        }
    }
}

static int emb_tcp_client_try_connect(emb_tcp_client_t* _cli)
{
    int ret = 0;

//    printf("%s:%d state:%s, attempts:%d, is_first_reconnect:%d\n",
//        inet_ntoa(_cli->serveraddr.sin_addr),
//        htons(_cli->serveraddr.sin_port),
//        str_state(_cli->state),
//        _cli->connection_attempts,
//        _cli->is_first_reconnect);



    switch(_cli->state) {
    case emb_tcs_disconnected:
        // Connection begining:

        // Provide delays between re-connections
        if(_cli->is_first_reconnect) {
            if(get_time_period_ms_from(&_cli->disconnect_time) < _cli->first_reconnect_delay_ms)
                return -ETIMEDOUT;
        }
        else {
            if(get_time_period_ms_from(&_cli->disconnect_time) < _cli->next_reconnects_delay_ms)
                return -ETIMEDOUT;
        }

//        printf("====================> starting connection %s\n",
//               _cli->is_first_reconnect ? "FIRST" : "SECOND");

        _cli->is_first_reconnect = 0;

        emb_tcp_client_close(_cli);
        _cli->connection_attempts++;

        gettimeofday(&_cli->connection_start_time, NULL);

        ret = emb_tcp_client_start_connect(_cli);

        if(_cli->flags & EMB_TCP_CLI_NO_DELAY_WHILE_CONNECT)
            break;

    case emb_tcs_connecting:
        return emb_tcp_client_wait4connect(_cli);

    default:
        break;
    }

    return ret;
}

int emb_tcp_client_set_connection_options(emb_tcp_client_t* _cli, const char* _ip, uint16_t _port)
{
    if(_cli && _ip) {
        int ret;
        _cli->serveraddr.sin_family = AF_INET;
        _cli->serveraddr.sin_port = htons(_port);

        ret = inet_pton(AF_INET, _ip, &_cli->serveraddr.sin_addr);
        if(ret != 1) {
            if(ret == -1) {
                ERR("Error with inet_pton(): %m\n");
            }
            ERR("Fail with convert IP address: '%s' to binary net address: %m\n", _ip);
            return ret;
        }
        return 0;
    }
    else
        return -EINVAL;
}

int emb_tcp_client_init(emb_tcp_client_t* _cli)
{
    if(_cli) {
        _cli->state = emb_tcs_disconnected;
        _cli->fd = -1;
        _cli->rx_bytes = 0ULL;
        _cli->tx_bytes = 0ULL;
        _cli->rx_timeouts_counter = 0;
        _cli->tx_timeouts_counter = 0;
        _cli->connection_attempts = 0;
        _cli->is_first_reconnect = 1;
        _cli->connection_start_time.tv_sec = 0;
        _cli->connection_start_time.tv_usec = 0;
        _cli->disconnect_time.tv_sec = 0;
        _cli->disconnect_time.tv_usec = 0;
        return 0;
    }
    return -EINVAL;
}

int emb_tcp_client_deinit(emb_tcp_client_t* _cli)
{
    if(_cli) {
        return emb_tcp_client_close(_cli);
    }
    return -EINVAL;
}

static void emb_tcp_cli_check4rxtx_timeouts(emb_tcp_client_t* _cli)
{
    if(_cli->flags & EMB_TCP_CLI_RECONNECT_AT_TIMEOUTS_COUNTER) {
        if(_cli->rx_timeouts_counter >= _cli->rxtx_timeouts_to_reconnect) {
            DBG("Reached maximum number of receive timeouts (%d), force reconnection\n", _cli->rxtx_timeouts_to_reconnect);
            _cli->rx_timeouts_counter = 0;
            emb_tcp_client_close(_cli);
        }
        if(_cli->tx_timeouts_counter >= _cli->rxtx_timeouts_to_reconnect) {
            DBG("Reached maximum number of transmit timeouts (%d), force reconnection\n", _cli->rxtx_timeouts_to_reconnect);
            _cli->tx_timeouts_counter = 0;
            emb_tcp_client_close(_cli);
        }
    }
}

int emb_tcp_client_send(emb_tcp_client_t* _cli, const void* _buf, unsigned int _length)
{
    if(_cli && _buf && _length) {

        int sel_res;
        fd_set write_fds;
        struct timeval tv;

        timeval_set_ms(&tv, _cli->transmit_timeout_ms);

        if(!emb_tcp_client_is_connected(_cli))
            emb_tcp_client_try_connect(_cli);

        if(!emb_tcp_client_is_connected(_cli))
            return -EBADFD;

        FD_ZERO(&write_fds);

        FD_SET(_cli->fd, &write_fds);

        sel_res = select(_cli->fd+1, NULL, &write_fds, NULL, &tv);
        if(sel_res == 0) {
            DBG("select() timeout\n");
            _cli->tx_timeouts_counter++;
            emb_tcp_cli_check4rxtx_timeouts(_cli);
            return -ETIMEDOUT;
        }
        else if(sel_res < 0) {
            ERR("select() error: %m\n");
            emb_tcp_client_close(_cli);
            return sel_res;
        }
        else {
            int nbytes;
            // MSG_NOSIGNAL will supress a SIG_PIPE generation at "Broke pipe" event
            if((nbytes = (int)send(_cli->fd, _buf, _length, MSG_NOSIGNAL)) <= 0) {
                if(nbytes == 0)
                    DBG("Connection closed\n");
                else
                    ERR("Error with send(): %m\n");

                emb_tcp_client_close(_cli);
                return -EBADFD;
            }

            if((_cli->flags & EMB_TCP_CLI_FORCE_RECONN_AT_SEND) && _cli->force_reconnect_delay_s) {
                if(get_time_period_s_from(&_cli->connect_time) > _cli->force_reconnect_delay_s) {
                    DBG("Force reconnection\n");
                    emb_tcp_client_close(_cli);
                }
            }

            if(_cli->tx_timeouts_counter > 0)
                _cli->tx_timeouts_counter--;

            _cli->tx_bytes += (unsigned long long)nbytes;
            return nbytes;
        }
    }
    return -EINVAL;
}

typedef struct __attribute__ ((packed))
{
    uint16_t transact_id;	///< Transaction ID, unique number for each transaction (big-endian)
    uint16_t proto_id;		///< Protocol ID, must be 0
    uint16_t length;		///< Packet length, excluding transact_id,proto_id,length (big-endian)
    uint8_t unit_id;		///< Modbus server ID
} emb_tcp_header_t;

int emb_tcp_client_recv_tcp(emb_tcp_client_t* _cli, void* _buf, unsigned int _length)
{
    if(_cli && _buf && _length) {

        int sel_res;
        fd_set read_fds;
        struct timeval tv;

        timeval_set_ms(&tv, _cli->receive_timeout_ms);

        if(!emb_tcp_client_is_connected(_cli))
            emb_tcp_client_try_connect(_cli);

        if(!emb_tcp_client_is_connected(_cli))
            return -EBADFD;

        FD_ZERO(&read_fds);

        FD_SET(_cli->fd, &read_fds);

        sel_res = select(_cli->fd+1, &read_fds, NULL, NULL, &tv);
        if(sel_res == 0) {
            DBG("select() timeout\n");
            _cli->rx_timeouts_counter++;
            emb_tcp_cli_check4rxtx_timeouts(_cli);
            return -ETIMEDOUT;
        }
        else if(sel_res < 0) {
            ERR("select() error: %m\n");
            emb_tcp_client_close(_cli);
            return sel_res;
        }
        else {
            int nbytes;

            if((nbytes = (int)recv(_cli->fd, _buf, sizeof(emb_tcp_header_t), 0)) != sizeof(emb_tcp_header_t)) {
                if(nbytes == 0)
                    DBG("Connection closed\n");
                else
                    ERR("Error with recv()\n");

                emb_tcp_client_close(_cli);
                return -EBADFD;
            }

            const emb_tcp_header_t* hdr = (const emb_tcp_header_t*)_buf;
            uint16_t pdu_length = SWAP_BYTES(hdr->length) - 1;

            if (_length < (pdu_length + sizeof(emb_tcp_header_t)))
                return -modbus_buffer_overflow;

            if((nbytes = (int)recv(_cli->fd, (uint8_t*)_buf + sizeof(emb_tcp_header_t), pdu_length, 0)) != pdu_length) {
                if(nbytes == 0)
                    DBG("Connection closed\n");
                else
                    ERR("Error with recv()\n");

                emb_tcp_client_close(_cli);
                return -EBADFD;
            }

            if((_cli->flags & EMB_TCP_CLI_FORCE_RECONN_AT_RECV) && _cli->force_reconnect_delay_s) {
                if(get_time_period_s_from(&_cli->connect_time) > _cli->force_reconnect_delay_s) {
                    DBG("Force reconnection\n");
                    emb_tcp_client_close(_cli);
                }
            }

            if(_cli->rx_timeouts_counter > 0)
                _cli->rx_timeouts_counter--;

            nbytes += sizeof(emb_tcp_header_t);

            _cli->rx_bytes += (unsigned long long)nbytes;
            return nbytes;
        }
    }
    return -EINVAL;
}
