
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <emodbus/protocols/implementations/tcp-client.h>

struct tcp_client_t {
    int sock_fd;
    struct event_base* base;
    struct sockaddr_in sin;
    struct event* sock_event_read;
    struct event* sock_event_write;
    struct event* timer_event;
    struct timeval reconnect_timeout;
    tcp_cient_notifier_t event_notifier;
    enum tcp_client_state_t state;
    void* user_data;
};

//#define tcp_client_dbg(...)
//#define tcp_client_err(...)

#define tcp_client_dbg(...)     fprintf(stdout, __VA_ARGS__); fflush(stdout);
#define tcp_client_err(...)     fprintf(stderr, __VA_ARGS__); fflush(stderr);

static void sock_fd_read_cb(evutil_socket_t _fd, short _event, void * _arg);
static void sock_fd_write_cb(evutil_socket_t _fd, short _event, void * _arg);

static void do_unconnect(struct tcp_client_t* _ctx);

static int do_connect(struct tcp_client_t* _ctx) {
    do {

        if ((_ctx->sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            tcp_client_err("%s: couldn't get socket: %s\n", __PRETTY_FUNCTION__, strerror(errno));
            return -1;
        }
        tcp_client_dbg("New fd = %d\n", _ctx->sock_fd);

        if (connect(_ctx->sock_fd, (struct sockaddr *)&(_ctx->sin), sizeof(_ctx->sin)) == -1) {
            tcp_client_err("%s: couldn't connect: %s\n", __PRETTY_FUNCTION__, strerror(errno));
            break;
        }

        _ctx->sock_event_write = event_new(_ctx->base, _ctx->sock_fd, EV_WRITE, sock_fd_write_cb, _ctx);
        if(!_ctx->sock_event_write) {
            tcp_client_err("Error with event_new() call: %m\n");
            break;
        }

        _ctx->sock_event_read = event_new(_ctx->base, _ctx->sock_fd, EV_READ | EV_PERSIST, sock_fd_read_cb, _ctx);
        if(!_ctx->sock_event_read) {
            tcp_client_err("Error with event_new() call: %m\n");
            break;
        }

        if(event_add(_ctx->sock_event_read, NULL)) {
            tcp_client_err("Error with event_add() call: %m\n");
            break;
        }

        if(event_add(_ctx->sock_event_write, NULL)) {
            tcp_client_err("Error with event_add() call: %m\n");
            break;
        }

        if(_ctx->state != tcp_client_connected) {
            _ctx->state = tcp_client_connected;
            if(_ctx->event_notifier)
                _ctx->event_notifier(_ctx, tcp_cli_connected);
        }

        return 0;
    }
    while(0);

    do_unconnect(_ctx);

    if(_ctx->event_notifier)
        _ctx->event_notifier(_ctx, tcp_cli_bad_try_of_reconnection);

    return -1;
}

static void do_unconnect(struct tcp_client_t* _ctx) {
    if(_ctx->sock_event_read) {
        event_del(_ctx->sock_event_read);
        event_free(_ctx->sock_event_read);
        _ctx->sock_event_read = NULL;
    }

    if(_ctx->sock_event_write) {
        event_del(_ctx->sock_event_write);
        event_free(_ctx->sock_event_write);
        _ctx->sock_event_write = NULL;
    }

    if(_ctx->sock_fd > 0) {
        tcp_client_dbg("closing fd %d\n", _ctx->sock_fd);
        close(_ctx->sock_fd);
    }

    if(_ctx->state != tcp_client_disconnected) {
        _ctx->state = tcp_client_disconnected;
        if(_ctx->event_notifier)
            _ctx->event_notifier(_ctx, tcp_cli_disconnected);
    }
}

static void reconnect(int fd, short event, void *arg) {
    struct tcp_client_t* ctx = (struct tcp_client_t*)arg;
    tcp_client_dbg("reconnection: ...\n");
    if(do_connect(ctx) == 0) {
        event_del(ctx->timer_event);
        tcp_client_dbg("reconnection: ok\n");
    }
    else {
        tcp_client_dbg("reconnection: fail\n");
    }
}

static void start_reconnect_timer(struct tcp_client_t* _ctx);

static void first_time_connect(int fd, short event, void *arg) {
    struct tcp_client_t* ctx = (struct tcp_client_t*)arg;
    tcp_client_dbg("connection: ...\n");
    if(do_connect(ctx) == 0) {
        event_del(ctx->timer_event);
        tcp_client_dbg("connection: ok\n");
    }
    else {
        start_reconnect_timer(ctx);
        tcp_client_dbg("connection: fail\n");
    }
}

static void start_reconnect_timer(struct tcp_client_t* _ctx) {
    event_assign(_ctx->timer_event, _ctx->base, -1, EV_TIMEOUT | EV_PERSIST, reconnect, _ctx);
    event_add(_ctx->timer_event, &_ctx->reconnect_timeout);
    tcp_client_dbg("%s\n", __FUNCTION__);
}

static void sock_fd_read_cb(evutil_socket_t _fd, short _event, void * _arg) {
    struct tcp_client_t* ctx = (struct tcp_client_t*)_arg;
    if(_event & EV_READ) {
        if(ctx->event_notifier)
            ctx->event_notifier(ctx, tcp_cli_data_received);
    }
}

static void sock_fd_write_cb(evutil_socket_t _fd, short _event, void * _arg) {
    struct tcp_client_t* ctx = (struct tcp_client_t*)_arg;
    if(_event & EV_WRITE) {
        if(ctx->event_notifier)
            ctx->event_notifier(ctx, tcp_cli_data_sent);
    }
}

struct tcp_client_t* tcp_client_new(struct event_base* _base,
                                    tcp_cient_notifier_t _event_notifier) {

    struct tcp_client_t* res = malloc(sizeof(struct tcp_client_t));

    memset(res, 0, sizeof(struct tcp_client_t));
    res->base = _base;
    res->event_notifier = _event_notifier;
    res->state = tcp_client_default;
    res->timer_event = event_new(_base, -1, EV_TIMEOUT /*| EV_PERSIST*/, first_time_connect, res);
    if(!res->timer_event) {
        tcp_client_err("%s: Error with evtimer_new() call: %m\n", __FUNCTION__);
        free(res);
        return NULL;
    }
    tcp_client_set_reconnection_delay(res, TCP_CLIENT_DEFAULT_RECONNECT_DLY);
    return res;
}

void tcp_client_free(struct tcp_client_t* _ctx) {
    if(_ctx) {
        tcp_client_stop_connection(_ctx);
        if(_ctx->timer_event)
            event_free(_ctx->timer_event);
        free(_ctx);
    }
}

int tcp_client_start_connection(struct tcp_client_t* _ctx,
                                const char* _ip_address,
                                unsigned short _port) {

    struct timeval tv;

    if(!_ctx)
        return -EINVAL;
    _ctx->sin.sin_family = AF_INET;
    inet_aton(_ip_address, &_ctx->sin.sin_addr);
    _ctx->sin.sin_port = htons(_port);
    _ctx->state = tcp_client_disconnected;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    event_add(_ctx->timer_event, &tv);

    return 0;
}

void tcp_client_stop_connection(struct tcp_client_t* _ctx) {
    if(!_ctx)
        return;
    if(_ctx->timer_event)
        event_del(_ctx->timer_event);
    do_unconnect(_ctx);
    _ctx->state = tcp_client_default;
}

int tcp_client_read(struct tcp_client_t* _ctx,
                    void* _p_buffer,
                    size_t _buff_size) {
    if(_ctx) {
        const int r = recv(_ctx->sock_fd, _p_buffer, _buff_size, 0);
        if(r <= 0) {
            tcp_client_dbg("disconnection by recv error(%d)\n", r);
            do_unconnect(_ctx);
            start_reconnect_timer(_ctx);
            return 0;
        }
        return r;
    }
    else
        return -EINVAL;
}

int tcp_client_write(struct tcp_client_t* _ctx,
                     const void* _p_data,
                     size_t _data_size) {
    if(_ctx) {
        const int r = send(_ctx->sock_fd, _p_data, _data_size, MSG_NOSIGNAL);
        if(r < 0) {
            tcp_client_dbg("disconnection by send error(%d)\n", r);
            do_unconnect(_ctx);
            start_reconnect_timer(_ctx);
            return 0;
        }
        return r;
    }
    else
        return -EINVAL;
}

void tcp_client_enable_write_event(struct tcp_client_t* _ctx) {
    if(_ctx && _ctx->sock_event_write)
        if(event_add(_ctx->sock_event_write, NULL))
            tcp_client_err("Error with event_add() call: %m\n");
}

struct event_base* tcp_client_get_base(const struct tcp_client_t *_ctx) {
    return _ctx ? _ctx->base : NULL;
}

int tcp_client_get_fd(const struct tcp_client_t* _ctx) {
    return _ctx ? _ctx->sock_fd : -1;
}

void tcp_client_set_reconnection_delay(struct tcp_client_t *_ctx,
                                       unsigned int _sec) {

    _ctx->reconnect_timeout.tv_sec = _sec;
    _ctx->reconnect_timeout.tv_usec = 0;
}

void tcp_client_set_user_data(struct tcp_client_t* _ctx, void* _user_data) {
    _ctx->user_data = _user_data;
}

void* tcp_client_get_user_data(struct tcp_client_t* _ctx) {
    return _ctx->user_data;
}

