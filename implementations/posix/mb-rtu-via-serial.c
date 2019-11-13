
#include <emodbus/impl/posix/mb-rtu-via-serial.h>
#include <emodbus/impl/posix/serial-port.h>
#include <emodbus/transport/rtu.h>
#include <emodbus/base/add/container_of.h>

#include <stdlib.h>
#include <string.h>

struct emb_rtu_via_serial_t
{
    struct serial_port_t* serial;
    struct event* char_timeout_timer;
    struct timeval char_pause;

    uint8_t rx_buf[MAX_PDU_SIZE];
    uint8_t tx_buf[MAX_PDU_SIZE];
    unsigned int rx_counter;

    emb_pdu_t rx_pdu;

    emb_on_rx_pdu_t on_rx_pdu;
    emb_on_error_t on_error;
    void* cb_ctx;
};

static void serial_port_notifier(void* _ctx,
                                 enum serial_port_events_t _event) {

    struct emb_rtu_via_serial_t* ctx = (struct emb_rtu_via_serial_t*)_ctx;
    if(ctx) {
        int r;
        if(_event == serial_port_data_received_event) {
            r = serial_port_read(ctx->serial,
                                 ctx->rx_buf + ctx->rx_counter,
                                 sizeof(ctx->rx_buf) - ctx->rx_counter);
            if(r > 0) {
                ctx->rx_counter += (unsigned int)r;
                event_add(ctx->char_timeout_timer, &ctx->char_pause);
            }
            else
                ctx->rx_counter = 0;
        }
    }
}

static void on_timer(evutil_socket_t _fd, short _what, void *_arg) {
    (void)_fd;
    (void)_what;
    struct emb_rtu_via_serial_t* ctx = (struct emb_rtu_via_serial_t*)_arg;
    struct emb_transport_info_t info;
    info.pdu = &ctx->rx_pdu;
    if(emb_rtu_decode_packet(ctx->rx_buf, ctx->rx_counter, &info) == 0) {

    }
    ctx->rx_counter = 0;
}

struct emb_rtu_via_serial_t*
emb_rtu_via_serial_create(struct event_base *_base,
                          unsigned int _timeout_ms,
                          const char* _dev_name,
                          unsigned int _baudrate) {

    struct emb_rtu_via_serial_t* ctx =
            (struct emb_rtu_via_serial_t*)malloc(sizeof(struct emb_rtu_via_serial_t));

    if(ctx) {
        memset(ctx, 0, sizeof(struct emb_rtu_via_serial_t));

        ctx->rx_pdu.data = ctx->tx_buf + 2;
        ctx->rx_pdu.max_size = MAX_PDU_SIZE - 4;

        ctx->serial = serial_port_create(_base, _dev_name, _baudrate);
        if(!ctx->serial) {
            fprintf(stderr, "%s: Error with serial_port_create() : %m\n", __FUNCTION__);
            fflush(stderr);
            emb_rtu_via_serial_destroy(ctx);
            return NULL;
        }

        serial_port_set_notifier(ctx->serial, serial_port_notifier, ctx);

        ctx->char_pause.tv_sec = 0;
        ctx->char_pause.tv_usec = 1000 * _timeout_ms;

        ctx->char_timeout_timer = event_new(_base,
                                            -1,
                                            EV_TIMEOUT/* | EV_PERSIST*/,
                                            on_timer,
                                            ctx);
        if(!ctx->char_timeout_timer) {
            fprintf(stderr, "%s: Error with event_new() call: %m\n", __FUNCTION__);
            fflush(stderr);
            emb_rtu_via_serial_destroy(ctx);
            return NULL;
        }
    }

    return ctx;
}

void emb_rtu_via_serial_destroy(struct emb_rtu_via_serial_t* _ctx) {
    if(_ctx) {
        if(_ctx->serial)
            serial_port_destroy(_ctx->serial);
        if(_ctx->char_timeout_timer) {
            event_del(_ctx->char_timeout_timer);
            event_free(_ctx->char_timeout_timer);
        }
        free(_ctx);
    }
}

void emb_rtu_via_serial_set_cb(struct emb_rtu_via_serial_t* _ctx,
                               emb_on_rx_pdu_t _on_rx,
                               emb_on_error_t _on_err,
                               void* _context)
{
    if(_ctx) {
        _ctx->on_rx_pdu = _on_rx;
        _ctx->on_error = _on_err;
        _ctx->cb_ctx = _context;
    }
}

void emb_rtu_via_serial_send(struct emb_rtu_via_serial_t* _ctx,
                             const struct emb_transport_info_t* _info)
{
    if(_ctx && _info) {
        const int r = emb_rtu_encode_packet(_info, _ctx->tx_buf, sizeof(_ctx->tx_buf));
        if(r > 0) {
            serial_port_write(&_ctx->serial, _ctx->tx_buf, (unsigned int)r, NULL);
        }
    }
}
