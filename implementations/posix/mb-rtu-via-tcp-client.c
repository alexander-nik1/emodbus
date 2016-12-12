
#include <emodbus/transport/rtu.h>
#include <emodbus/impl/posix/mb-rtu-via-tcp-client.h>
#include <emodbus/impl/posix/tcp-client.h>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_pdu.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct emb_rtu_via_tcp_client_t
{
    struct emb_rtu_t modbus_rtu;
    struct tcp_client_t* tcp_client;
    int opened_flag;
    struct event* char_timeout_timer;
    struct timeval char_pause;

    char rx_buf[MAX_PDU_SIZE];
    char tx_buf[MAX_PDU_SIZE];

    emb_rtu_via_tcp_client_notifier_t event_notifier;
};

static void tcp_cient_notifier(struct tcp_client_t* _ctx,
                               enum tcp_client_events_t _event)
{
    struct emb_rtu_via_tcp_client_t* _this =
            (struct emb_rtu_via_tcp_client_t*)tcp_client_get_user_data(_ctx);

    switch(_event) {
    case tcp_cli_data_received:
        emb_rtu_port_event(&_this->modbus_rtu, emb_rtu_data_received_event);
        break;

    case tcp_cli_data_sent:
        emb_rtu_port_event(&_this->modbus_rtu, emb_rtu_tx_buf_empty_event);
        break;
    }

    if(_this->event_notifier)
        _this->event_notifier(_this, _event);
}

static int read_from_port(struct emb_rtu_t* _mbt,
                          void* _p_buf,
                          unsigned int _buf_size)
{
    struct emb_rtu_via_tcp_client_t* _this =
            container_of(_mbt, struct emb_rtu_via_tcp_client_t, modbus_rtu);

    if(!_this->opened_flag) {
        return 0;
    }

    return tcp_client_read(_this->tcp_client, _p_buf, _buf_size);
}

static int write_to_port(struct emb_rtu_t* _mbt,
                         const void* _p_data,
                         unsigned int _sz_to_write,
                         unsigned int* _wrote)
{
    int res;
    struct emb_rtu_via_tcp_client_t* _this =
            container_of(_mbt, struct emb_rtu_via_tcp_client_t, modbus_rtu);

    if(!_this->opened_flag)
        return 0;

    if(!_sz_to_write)
        return 0;

    res = tcp_client_write(_this->tcp_client, _p_data, _sz_to_write);
    if(res > 0 && _wrote)
        *_wrote += res;

    if(res > 0 && res < _sz_to_write)
        tcp_client_enable_write_event(_this->tcp_client);

    return res;
}

static void modbus_rtu_on_char(struct emb_rtu_t* _emb)
{
    struct emb_rtu_via_tcp_client_t* _this =
            container_of(_emb, struct emb_rtu_via_tcp_client_t, modbus_rtu);
    if(_this->opened_flag) {
        event_add(_this->char_timeout_timer, &_this->char_pause);
    }
}

static void on_timer(evutil_socket_t _fd, short _what, void *_arg)
{
    struct emb_rtu_via_tcp_client_t* _this = (struct emb_rtu_via_tcp_client_t*)_arg;
    emb_rtu_on_char_timeout(&_this->modbus_rtu);
}

struct emb_rtu_via_tcp_client_t*
emb_rtu_via_tcp_client_create(struct event_base *_base,
                              unsigned int _timeout_ms,
                              const char* _ip_addr,
                              unsigned int _port)
{
    int r;
    struct emb_rtu_via_tcp_client_t* ctx = NULL;

    do {
        ctx = (struct emb_rtu_via_tcp_client_t*)malloc(sizeof(struct emb_rtu_via_tcp_client_t));

        if(!ctx) {
            fprintf(stderr, "%s: Error with malloc(): %m\n", __FUNCTION__);
            fflush(stdout);
            break;
        }

        memset(ctx, 0, sizeof(struct emb_rtu_via_tcp_client_t));

        ctx->modbus_rtu.rx_buffer = ctx->rx_buf;
        ctx->modbus_rtu.tx_buffer = ctx->tx_buf;
        ctx->modbus_rtu.rx_buf_size = MAX_PDU_SIZE;
        ctx->modbus_rtu.tx_buf_size = MAX_PDU_SIZE;

        ctx->modbus_rtu.emb_rtu_on_char = modbus_rtu_on_char;
        ctx->modbus_rtu.read_from_port = read_from_port;
        ctx->modbus_rtu.write_to_port = write_to_port;

        emb_rtu_initialize(&ctx->modbus_rtu);

        ctx->tcp_client = tcp_client_new(_base, tcp_cient_notifier);
        if(!ctx->tcp_client) {
            fprintf(stderr, "%s: Error with tcp_client_new(): %m\n", __FUNCTION__);
            fflush(stdout);
            break;
        }

        tcp_client_set_user_data(ctx->tcp_client, ctx);

        if((r=tcp_client_start_connection(ctx->tcp_client, _ip_addr, _port))) {
            fprintf(stderr, "%s: Error: with tcp_client_start_connection(): (%d) %m\n",
                    __FUNCTION__, r);
            fflush(stdout);
            break;
        }

        emb_rtu_via_tcp_client_set_timeout(ctx, _timeout_ms);

        ctx->char_timeout_timer = event_new(_base,
                                            -1,
                                            EV_TIMEOUT/* | EV_PERSIST*/,
                                            on_timer,
                                            ctx);
        if(!ctx->char_timeout_timer) {
            fprintf(stderr, "%s: Error with event_new() call: %m\n", __FUNCTION__);
            fflush(stderr);
            break;
        }

        return ctx;
    }
    while(0);

    emb_rtu_via_tcp_client_destroy(ctx);

    return NULL;
}

void emb_rtu_via_tcp_client_set_timeout(struct emb_rtu_via_tcp_client_t* _ctx,
                                        unsigned int _timeout_ms)
{
    if(_ctx) {
        _ctx->char_pause.tv_sec = _timeout_ms / 1000;
        _ctx->char_pause.tv_usec = 1000 * (_timeout_ms % 1000);
    }
}

void emb_rtu_via_tcp_client_destroy(struct emb_rtu_via_tcp_client_t* _ctx)
{
    if(_ctx) {
        if(_ctx->tcp_client)
            tcp_client_free(_ctx->tcp_client);
        if(_ctx->char_timeout_timer)
            event_free(_ctx->char_timeout_timer);
        free(_ctx);
    }
}

struct emb_transport_t*
emb_rtu_via_tcp_client_get_transport(struct emb_rtu_via_tcp_client_t* _ctx)
{
    if(_ctx)
        return &_ctx->modbus_rtu.transport;
    return NULL;
}

struct tcp_client_t*
emb_rtu_via_tcp_cli_get_tcp_client(struct emb_rtu_via_tcp_client_t* _ctx)
{
    if(_ctx)
        return _ctx->tcp_client;
    return NULL;
}

int emb_rtu_via_tcp_client_set_notifier(struct emb_rtu_via_tcp_client_t* _ctx,
                                        emb_rtu_via_tcp_client_notifier_t _notifier)
{
    if(_ctx) {
        _ctx->event_notifier = _notifier;
        return 0;
    }
    return -EINVAL;
}
