
#include <emodbus/implementations/posix/mb-tcp-via-tcp-server.h>
#include <emodbus/implementations/posix/tcp-server.h>
#include <emodbus/base/add/container_of.h>
#include <emodbus/protocols/tcp.h>
#include <emodbus/base/modbus_pdu.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct emb_tcp_via_tcp_server_t {
    struct emb_tcp_t modbus_tcp;
    struct tcp_server_t* tcp_server;
};

static int read_from_port(struct emb_tcp_t* _mbt,
                          void* _p_buf,
                          unsigned int _buf_size) {
    if(_mbt) {
        struct emb_tcp_via_tcp_server_t* _this = container_of(_mbt, struct emb_tcp_via_tcp_server_t, modbus_tcp);
        return tcp_server_read(_this->tcp_server, _mbt->tcp_client_id, _p_buf, _buf_size);
    }
    return 0;
}

static int write_to_port(struct emb_tcp_t* _mbt,
                         const void* _p_data,
                         unsigned int _sz_to_write) {
    if(_mbt && _sz_to_write) {
        struct emb_tcp_via_tcp_server_t* _this = container_of(_mbt, struct emb_tcp_via_tcp_server_t, modbus_tcp);
        return tcp_server_write(_this->tcp_server, _mbt->tcp_client_id, _p_data, _sz_to_write);
    }
    return 0;
}

static void tcp_cient_notifier(struct tcp_server_t* _ctx,
                               void* _client_id,
                               enum tcp_server_events_t _event) {

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

struct emb_tcp_via_tcp_server_t*
emb_tcp_via_tcp_server_create(struct event_base *_base, int _port) {
    struct emb_tcp_via_tcp_server_t* res =
            (struct emb_tcp_via_tcp_server_t*)malloc(sizeof(struct emb_tcp_via_tcp_server_t));
    if(res) {
        memset(res, 0, sizeof(struct emb_tcp_via_tcp_server_t));

        emb_tcp_initialize(&res->modbus_tcp);

        res->modbus_tcp.read_from_port = read_from_port;
        res->modbus_tcp.write_to_port = write_to_port;

        res->tcp_server = tcp_server_new(_base, tcp_cient_notifier, _port);
        if(!res->tcp_server) {
            free(res);
            return NULL;
        }
        tcp_server_set_user_data(res->tcp_server, res);
    }
    return res;
}

void emb_tcp_via_tcp_server_destroy(struct emb_tcp_via_tcp_server_t* _ctx) {
    if(_ctx) {
        if(_ctx->tcp_server)
            tcp_server_free(_ctx->tcp_server);
        free(_ctx);
    }
}

struct emb_protocol_t*
emb_tcp_via_tcp_server_get_proto(struct emb_tcp_via_tcp_server_t* _ctx) {
    if(_ctx) {
        return &_ctx->modbus_tcp.proto;
    }
    return NULL;
}
