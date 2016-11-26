
#include <emodbus/impl/posix/mb-tcp-via-tcp-server.h>
#include <emodbus/impl/posix/tcp-server.h>
#include <emodbus/base/add/container_of.h>
#include <emodbus/transport/tcp.h>
#include <emodbus/base/modbus_pdu.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct emb_tcp_via_tcp_server_t
{
    struct emb_tcp_t modbus_tcp;
    struct tcp_server_t* tcp_server;
    struct event* rx_timeout_timer;
    struct timeval rx_timeout;
};

static int read_from_port(struct emb_tcp_t* _mbt,
                          void* _p_buf,
                          unsigned int _buf_size)
{
    if(_mbt) {
        struct emb_tcp_via_tcp_server_t* _this = container_of(_mbt, struct emb_tcp_via_tcp_server_t, modbus_tcp);
        event_add(_this->rx_timeout_timer, &_this->rx_timeout);
        return tcp_server_read(_this->tcp_server, _mbt->tcp_client_id, _p_buf, _buf_size);
    }
    return 0;
}

static int write_to_port(struct emb_tcp_t* _mbt,
                         const void* _p_data,
                         unsigned int _sz_to_write)
{
    if(_mbt && _sz_to_write) {
        struct emb_tcp_via_tcp_server_t* _this = container_of(_mbt, struct emb_tcp_via_tcp_server_t, modbus_tcp);
        return tcp_server_write(_this->tcp_server, _mbt->tcp_client_id, _p_data, _sz_to_write);
    }
    return 0;
}

static void tcp_cient_notifier(struct tcp_server_t* _ctx,
                               void* _client_id,
                               enum tcp_server_events_t _event)
{
    struct emb_tcp_via_tcp_server_t* _this = (struct emb_tcp_via_tcp_server_t*)tcp_server_get_user_data(_ctx);

    switch(_event) {
    case tcp_srv_data_received:
        emb_tcp_port_event(&_this->modbus_tcp, _client_id, emb_tcp_data_received_event);
        break;

    case tcp_srv_data_sent:
        emb_tcp_port_event(&_this->modbus_tcp, _client_id, emb_tcp_tx_buf_empty_event);
        break;
    }
}

static void on_rx_timeout(evutil_socket_t _fd, short _what, void *_arg) {
    struct emb_tcp_via_tcp_server_t* _this = (struct emb_tcp_via_tcp_server_t*)_arg;
    emb_tcp_port_event(&_this->modbus_tcp, _this->modbus_tcp.tcp_client_id, emb_tcp_last_rx_timeout);
}


struct emb_tcp_via_tcp_server_t*
emb_tcp_via_tcp_server_create(struct event_base *_base,
                              int _port,
                              unsigned int _rx_timeout_ms)
{
    struct emb_tcp_via_tcp_server_t* res = NULL;


    do {

        res = (struct emb_tcp_via_tcp_server_t*)malloc(sizeof(struct emb_tcp_via_tcp_server_t));
        if(!res)
            break;

        memset(res, 0, sizeof(struct emb_tcp_via_tcp_server_t));

        res->rx_timeout.tv_sec = _rx_timeout_ms / 1000;
        res->rx_timeout.tv_usec = 1000 * (_rx_timeout_ms % 1000);

        res->rx_timeout_timer = event_new(_base, -1, EV_TIMEOUT, on_rx_timeout, res);
        if(!res->rx_timeout_timer)
            break;

        res->tcp_server = tcp_server_new(_base, tcp_cient_notifier, _port);
        if(!res->tcp_server) {
            break;
        }
        tcp_server_set_user_data(res->tcp_server, res);

        emb_tcp_initialize(&res->modbus_tcp);

        res->modbus_tcp.read_from_port = read_from_port;
        res->modbus_tcp.write_to_port = write_to_port;

        return res;
    }
    while(0);

    if(res) {
        if(res->rx_timeout_timer)
            event_free(res->rx_timeout_timer);
        if(res->tcp_server)
            tcp_server_free(res->tcp_server);
        free(res);
    }

    return NULL;
}

void emb_tcp_via_tcp_server_destroy(struct emb_tcp_via_tcp_server_t* _ctx)
{
    if(_ctx) {
        if(_ctx->rx_timeout_timer)
            event_free(_ctx->rx_timeout_timer);
        if(_ctx->tcp_server)
            tcp_server_free(_ctx->tcp_server);
        free(_ctx);
    }
}

struct emb_transport_t*
emb_tcp_via_tcp_server_get_transport(struct emb_tcp_via_tcp_server_t* _ctx)
{
    if(_ctx) {
        return &_ctx->modbus_tcp.transport;
    }
    return NULL;
}

void* emb_tcp_via_tcp_server_get_curr_client_id(
        struct emb_tcp_via_tcp_server_t* _ctx)
{
    if(_ctx) {
        return _ctx->modbus_tcp.tcp_client_id;
    }
    return NULL;
}

struct tcp_server_t* emb_tcp_via_tcp_server_get_srv(
        struct emb_tcp_via_tcp_server_t* _ctx)
{
    if(_ctx) {
        return _ctx->tcp_server;
    }
    return NULL;
}
