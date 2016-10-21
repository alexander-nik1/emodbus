
#include <emodbus/impl/posix/mb-rtu-via-serial.h>
#include <emodbus/impl/posix/serial-port.h>
#include <emodbus/transport/rtu.h>
#include <emodbus/base/add/container_of.h>

#include <stdlib.h>
#include <string.h>

struct emb_rtu_via_serial_t {
    struct emb_rtu_t modbus_rtu;
    struct serial_port_t* serial;
    struct event* char_timeout_timer;
    struct timeval char_pause;

    char rx_buf[MAX_PDU_SIZE];
    char tx_buf[MAX_PDU_SIZE];
};

void serial_port_notifier(void* _ctx,
                          enum serial_port_events_t _event) {

    struct emb_rtu_via_serial_t* ctx = (struct emb_rtu_via_serial_t*)_ctx;
    if(ctx) {
        switch(_event) {
        case serial_port_data_received_event:
            emb_rtu_port_event(&ctx->modbus_rtu, emb_rtu_data_received_event);
            break;
        case serial_port_data_sent_event:
            emb_rtu_port_event(&ctx->modbus_rtu, emb_rtu_tx_buf_empty_event);
            break;
        }
    }
}

static void modbus_rtu_on_char(struct emb_rtu_t* _emb) {
    struct emb_rtu_via_serial_t* _this =
            container_of(_emb, struct emb_rtu_via_serial_t, modbus_rtu);
    event_add(_this->char_timeout_timer, &_this->char_pause);
}

static void on_timer(evutil_socket_t _fd, short _what, void *_arg) {
    struct emb_rtu_via_serial_t* _this = (struct emb_rtu_via_serial_t*)_arg;
    emb_rtu_on_char_timeout(&_this->modbus_rtu);
}

static int read_from_port(struct emb_rtu_t* _mbt,
                          void* _p_buf,
                          unsigned int _buf_size) {
    if(_mbt) {
        struct emb_rtu_via_serial_t* _this = container_of(_mbt, struct emb_rtu_via_serial_t, modbus_rtu);
        return serial_port_read(_this->serial, _p_buf, _buf_size);
    }
    return 0;
}

static int write_to_port(struct emb_rtu_t* _mbt,
                         const void* _p_data,
                         unsigned int _sz_to_write) {
    if(_mbt) {
        struct emb_rtu_via_serial_t* _this = container_of(_mbt, struct emb_rtu_via_serial_t, modbus_rtu);
        return serial_port_write(_this->serial, _p_data, _sz_to_write);
    }
    return 0;
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

        ctx->modbus_rtu.rx_buffer = ctx->rx_buf;
        ctx->modbus_rtu.tx_buffer = ctx->tx_buf;
        ctx->modbus_rtu.rx_buf_size = MAX_PDU_SIZE;
        ctx->modbus_rtu.tx_buf_size = MAX_PDU_SIZE;

        ctx->modbus_rtu.emb_rtu_on_char = modbus_rtu_on_char;

        ctx->modbus_rtu.read_from_port = read_from_port;
        ctx->modbus_rtu.write_to_port = write_to_port;

        emb_rtu_initialize(&ctx->modbus_rtu);

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
        free(_ctx);
    }
}

struct emb_transport_t*
emb_rtu_via_serial_get_transport(struct emb_rtu_via_serial_t* _ctx) {
    if(_ctx) {
        return &_ctx->modbus_rtu.transport;
    }
    return NULL;
}
