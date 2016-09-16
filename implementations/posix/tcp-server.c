
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

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <emodbus/impl/posix/tcp-server.h>

struct tcp_server_t {
    struct event_base* base;
    struct sockaddr_in sin;
    struct evconnlistener *listener;
    tcp_server_notifier_t event_notifier;
    void* user_data;
    int n_clients_limit;
    int clients_counter;
};

//#define tcp_server_dbg(...)
//#define tcp_server_err(...)

#define tcp_server_dbg(...)     fprintf(stdout, __VA_ARGS__); fflush(stdout);
#define tcp_server_err(...)     fprintf(stderr, __VA_ARGS__); fflush(stderr);

static void read_callback(struct bufferevent *_bev, void *_ctx)
{
    struct tcp_server_t* ctx = (struct tcp_server_t*)_ctx;
    if(ctx->event_notifier)
        ctx->event_notifier(ctx, _bev, tcp_srv_data_received);
}

static void write_callback(struct bufferevent *_bev, void *_ctx)
{
    struct tcp_server_t* ctx = (struct tcp_server_t*)_ctx;
    if(ctx->event_notifier)
        ctx->event_notifier(ctx, _bev, tcp_srv_data_received);
}

static void event_callback(struct bufferevent *bev, short events, void *_ctx)
{
    struct tcp_server_t* ctx = (struct tcp_server_t*)_ctx;

    if (events & BEV_EVENT_ERROR)
        perror("Error from bufferevent");
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        bufferevent_free(bev);
        --ctx->clients_counter;
        printf("Disconnect, number of clients = %d\n", ctx->clients_counter); fflush(stdout);
    }
}


static void accept_conn_cb(struct evconnlistener *listener,
                           evutil_socket_t fd,
                           struct sockaddr *address,
                           int socklen,
                           void *_ctx)
{
    struct tcp_server_t* ctx = (struct tcp_server_t*)_ctx;
    struct bufferevent *bev;

    ++ctx->clients_counter;

    if(ctx->n_clients_limit > 0) {
        if(ctx->clients_counter > ctx->n_clients_limit) {
            printf("The clients number is exceed a limit number of clients (%d), closing connection\n",
                   ctx->n_clients_limit);
            fflush(stdout);
            close(fd);
            --ctx->clients_counter;
            return;
        }
    }

    /* We got a new connection! Set up a bufferevent for it. */
    bev = bufferevent_socket_new(ctx->base, fd, BEV_OPT_CLOSE_ON_FREE);

    printf("Connect, number of clients = %d\n", ctx->clients_counter); fflush(stdout);

    bufferevent_setcb(bev, read_callback, write_callback, event_callback, _ctx);

    bufferevent_enable(bev, EV_READ|EV_WRITE);
}

static void accept_error_cb(struct evconnlistener *listener, void *ctx)
{
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener.\n",
            err, evutil_socket_error_to_string(err));
}

struct tcp_server_t* tcp_server_new(struct event_base* _base,
                                    tcp_server_notifier_t _event_notifier,
                                    int _port) {

    struct tcp_server_t* res = (struct tcp_server_t*)malloc(sizeof(struct tcp_server_t));
    if(res) {

        memset(res, 0, sizeof(struct tcp_server_t));
        res->base = _base;
        res->event_notifier = _event_notifier;

        res->sin.sin_family = AF_INET;
        res->sin.sin_addr.s_addr = htonl(0);
        res->sin.sin_port = htons(_port);

        res->clients_counter = 0;
        res->n_clients_limit = -1;

        res->listener = evconnlistener_new_bind(res->base, accept_conn_cb, res,
              LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
              (struct sockaddr*)&res->sin, sizeof(struct sockaddr));

        if(!res->listener) {
            free(res);
            return NULL;
        }

        evconnlistener_set_error_cb(res->listener, accept_error_cb);
    }
    return res;
}

void tcp_server_free(struct tcp_server_t* _ctx) {
    if(_ctx) {
        evconnlistener_free(_ctx->listener);
        free(_ctx);
    }
}

int tcp_server_read(struct tcp_server_t* _ctx,
                    void *_client_id,
                    void* _p_buffer,
                    size_t _buff_size) {
    if(_ctx) {
        struct bufferevent* bev = (struct bufferevent*)_client_id;
        return bufferevent_read(bev, _p_buffer, _buff_size);
    }
    else
        return -EINVAL;
}

int tcp_server_write(struct tcp_server_t* _ctx,
                     void *_client_id,
                     const void* _p_data,
                     size_t _data_size) {
    if(_ctx) {
        struct bufferevent* bev = (struct bufferevent*)_client_id;
        return bufferevent_write(bev, _p_data, _data_size);
    }
    else
        return -EINVAL;
}

void tcp_server_set_user_data(struct tcp_server_t* _ctx, void* _user_data) {
    _ctx->user_data = _user_data;
}

void* tcp_server_get_user_data(struct tcp_server_t* _ctx) {
    return _ctx->user_data;
}

void tcp_server_set_clients_limit(struct tcp_server_t* _ctx,
                                  int _clients_limit) {
    if(_ctx)
        _ctx->n_clients_limit = _clients_limit;
}

int tcp_server_clients_limit(const struct tcp_server_t* _ctx) {
    if(_ctx)
        return _ctx->n_clients_limit;
    return -EINVAL;
}
