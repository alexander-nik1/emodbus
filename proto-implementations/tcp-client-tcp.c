
#include <emodbus/proto-implementations/tcp-client-tcp.h>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_pdu.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include <emodbus/protocols/implementations/tcp-client.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <errno.h>

struct emb_tcp_over_tcp_client_t {
    struct emb_tcp_t modbus_tcp;
    struct tcp_client_t* tcp_client;
    int opened_flag;
};

static void tcp_cient_notifier(struct tcp_client_t* _ctx,
                               enum tcp_client_events_t _event) {

    struct emb_tcp_over_tcp_client_t* _this =
            (struct emb_tcp_over_tcp_client_t*)tcp_client_get_user_data(_ctx);

    switch(_event) {
    case tcp_cli_data_received:
        emb_tcp_port_event(&_this->modbus_tcp, NULL, emb_tcp_data_received_event);
        break;

    case tcp_cli_data_sent:
        emb_tcp_port_event(&_this->modbus_tcp, NULL, emb_tcp_tx_buf_empty_event);
        break;

    case tcp_cli_connected:
        printf("================> event: tcp_cli_connected\n"); fflush(stdout);
        _this->opened_flag = 1;
        break;

    case tcp_cli_disconnected:
        printf("================> event: tcp_cli_disconnected\n"); fflush(stdout);
        _this->opened_flag = 0;
        break;

    case tcp_cli_bad_try_of_reconnection:
        printf("================> event: tcp_cli_bad_try_of_connection\n"); fflush(stdout);
        break;
    }
}

static int read_from_port(struct emb_tcp_t* _mbt,
                          void* _p_buf,
                          unsigned int _buf_size) {

    struct emb_tcp_over_tcp_client_t* _this =
            container_of(_mbt, struct emb_tcp_over_tcp_client_t, modbus_tcp);

    if(!_this->opened_flag) {
        return 0;
    }

    return tcp_client_read(_this->tcp_client, _p_buf, _buf_size);
}

static int write_to_port(struct emb_tcp_t* _mbt,
                         const void* _p_data,
                         unsigned int _sz_to_write) {
    int wrote;
    struct emb_tcp_over_tcp_client_t* _this =
            container_of(_mbt, struct emb_tcp_over_tcp_client_t, modbus_tcp);

    if(!_this->opened_flag) {
        return 0;
    }

    if(!_sz_to_write)
        return 0;

    wrote = tcp_client_write(_this->tcp_client, _p_data, _sz_to_write);

    if(wrote > 0 && wrote < _sz_to_write)
        tcp_client_enable_write_event(_this->tcp_client);

    return wrote;
}

struct emb_tcp_over_tcp_client_t*
emb_tcp_over_tcp_client_create(struct event_base *_base,
                               const char* _ip_addr,
                               unsigned int _port) {
    int r;
    struct emb_tcp_over_tcp_client_t* res =
        (struct emb_tcp_over_tcp_client_t*)malloc(sizeof(struct emb_tcp_over_tcp_client_t));

    if(res) {
        memset(res, 0, sizeof(struct emb_tcp_over_tcp_client_t));
        emb_tcp_initialize(&res->modbus_tcp);
        res->tcp_client = tcp_client_new(_base, tcp_cient_notifier);
        if(!res->tcp_client) {
            free(res);
            return NULL;
        }

        tcp_client_set_user_data(res->tcp_client, res);
    }

    if((r=tcp_client_start_connection(res->tcp_client, _ip_addr, _port))) {
        fprintf(stderr, "Error: with tcp_client_start_connection(): (%d) %m\n", r);
        fflush(stdout);
        emb_tcp_over_tcp_client_destroy(res);
        return NULL;
    }

    res->modbus_tcp.read_from_port = read_from_port;
    res->modbus_tcp.write_to_port = write_to_port;

    return res;
}

void emb_tcp_over_tcp_client_destroy(struct emb_tcp_over_tcp_client_t* _ctx) {
    if(_ctx) {
        if(_ctx->tcp_client)
            tcp_client_free(_ctx->tcp_client);
        free(_ctx);
    }
}

struct emb_protocol_t*
emb_tcp_over_tcp_client_get_proto(struct emb_tcp_over_tcp_client_t* _ctx) {
    if(_ctx)
        return &_ctx->modbus_tcp.proto;
    return NULL;
}
