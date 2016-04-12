
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <list>
#include <errno.h>

#include <emodbus/emodbus.hpp>

#include <emodbus/client/client.h>
#include <emodbus/base/common.h>
#include <emodbus/protocols/rtu.h>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_errno.h>

#include <emodbus/server/server.h>
#include <emodbus/server/holdings.h>

#include <emodbus/base/add/stream.h>
#include "posix-serial-port.h"

#include <pthread.h>

#include <event2/event.h>

#include "timespec_operations.h"

class posix_serial_port_rtu_t {
public:
    posix_serial_port_rtu_t(struct event_base *_base,
                            const char* _dev_name,
                            unsigned int _baudrate) {

        posix_serial_port_open(&posix_serial_port, _base, _dev_name, _baudrate);

        rx_buffer.resize(128);
        tx_buffer.resize(128);

        modbus_rtu.user_data = this;
        modbus_rtu.rx_buffer = &rx_buffer[0];
        modbus_rtu.tx_buffer = &tx_buffer[0];
        modbus_rtu.rx_buf_size = rx_buffer.size();
        modbus_rtu.tx_buf_size = tx_buffer.size();
        modbus_rtu.modbus_rtu_on_char = modbus_rtu_on_char;

        modbus_rtu_initialize(&modbus_rtu);

        stream_connect(&posix_serial_port.output_stream, &modbus_rtu.input_stream);
        stream_connect(&modbus_rtu.output_stream, &posix_serial_port.input_stream);

        char_pause.tv_sec = 0;
      //  enum { pause = 100 };
        char_pause.tv_usec = 1000 * 10; //(1000 * 1000) / (_baudrate / pause);

        char_timeout_timer = event_new(_base, -1, EV_TIMEOUT/* | EV_PERSIST*/, on_timer, this);
    }

    ~posix_serial_port_rtu_t() {

        event_del(char_timeout_timer);
        event_free(char_timeout_timer);

        posix_serial_port_close(&posix_serial_port);
    }

    struct emb_protocol_t* get_proto() {
        return &modbus_rtu.proto;
    }

private:
    static void modbus_rtu_on_char(void* _user_data) {
        posix_serial_port_rtu_t* _this = (posix_serial_port_rtu_t*)_user_data;
        event_add(_this->char_timeout_timer, &_this->char_pause);
    }

    static void on_timer(evutil_socket_t fd, short what, void *arg) {
        posix_serial_port_rtu_t* _this = (posix_serial_port_rtu_t*)arg;
        modbus_rtu_on_char_timeout(&_this->modbus_rtu);
    }

    std::vector<unsigned char> rx_buffer, tx_buffer;

private:
    struct posix_serial_port_t posix_serial_port;
    struct modbus_rtu_t modbus_rtu;
    struct timeval char_pause;
    struct event *char_timeout_timer;
};

class my_holdings_t : public emb::server_holdings_t {
public:
    my_holdings_t() : emb::server_holdings_t(0xFFE0, 0x0020) {

    }

    uint8_t on_read_regs(emb_const_pdu_t* _req,
                         uint16_t _offset,
                         uint16_t _quantity,
                         uint16_t* _pvalues) {

        printf("%s: 0x%04X, 0x%04X\n", __PRETTY_FUNCTION__, _offset, _quantity);
        return 0;
    }

    uint8_t on_write_regs(emb_const_pdu_t* _req,
                          uint16_t _offset,
                          uint16_t _quantity,
                          const uint16_t* _pvalues) {
        printf("%s: 0x%04X, 0x%04X\n", __PRETTY_FUNCTION__, _offset, _quantity);
        return 0;
    }
};

void dbg_print_packet(FILE* _f, const char* _prefix, const void* _pkt, unsigned int _size) {
    if(_f) {
        int i;
        fprintf(_f, "%s", _prefix);
        for(i=0; i<_size; ++i) {
            fprintf(_f, "%02X ", ((uint8_t*)_pkt)[i]);
        }
        fprintf(_f, "\n");
        fflush(_f);
    }
}

int on_write_rx(struct input_stream_t* _this, const void* _data, unsigned int _size) {
    dbg_print_packet(stdout, ">>", _data, _size);
    return _size;
}

int on_write_tx(struct input_stream_t* _this, const void* _data, unsigned int _size) {
    dbg_print_packet(stdout, "<<", _data, _size);
    return _size;
}

input_stream_t emb_dumpi_rx = { on_write_rx, 0 };
input_stream_t emb_dumpi_tx = { on_write_tx, 0 };

int main(int argc, char* argv[]) {

    printf("emodbus server test\n");

    emb::super_server_t ssrv;

    struct event_base *base = event_base_new();

    posix_serial_port_rtu_t psp(base, "/dev/ttyUSB0", 115200);

    ssrv.set_proto(psp.get_proto());

    psp.get_proto()->flags |= EMB_PROTO_FLAG_DUMD_PAKETS;

    stream_connect(&emb_dump_rx, &emb_dumpi_rx);
    stream_connect(&emb_dump_tx, &emb_dumpi_tx);

    sleep(1);

    my_holdings_t h;

    emb::server_t srv1(16);

    srv1.add_function(0x03, emb_srv_read_holdings);

    srv1.add_holdings(h);

    ssrv.add_server(srv1);

    event_base_dispatch(base);

    return 0;
}
