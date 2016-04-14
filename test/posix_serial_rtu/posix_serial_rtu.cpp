
#include "posix_serial_rtu.hpp"
#include <emodbus/base/modbus_pdu.h>

posix_serial_rtu_t::posix_serial_rtu_t(struct event_base *_base,
                        const char* _dev_name,
                        unsigned int _baudrate) {

    posix_serial_port_open(&posix_serial_port, _base, _dev_name, _baudrate);

    rx_buffer.resize(MAX_PDU_SIZE);
    tx_buffer.resize(MAX_PDU_SIZE);

    modbus_rtu.user_data = this;
    modbus_rtu.rx_buffer = &rx_buffer[0];
    modbus_rtu.tx_buffer = &tx_buffer[0];
    modbus_rtu.rx_buf_size = rx_buffer.size();
    modbus_rtu.tx_buf_size = tx_buffer.size();
    modbus_rtu.modbus_rtu_on_char = modbus_rtu_on_char;

    emb_rtu_initialize(&modbus_rtu);

    stream_connect(&posix_serial_port.output_stream, &modbus_rtu.input_stream);
    stream_connect(&modbus_rtu.output_stream, &posix_serial_port.input_stream);

    char_pause.tv_sec = 0;
  //  enum { pause = 100 };
    char_pause.tv_usec = 1000 * 10; //(1000 * 1000) / (_baudrate / pause);

    char_timeout_timer = event_new(_base, -1, EV_TIMEOUT/* | EV_PERSIST*/, on_timer, this);
}

posix_serial_rtu_t::~posix_serial_rtu_t() {

    event_del(char_timeout_timer);
    event_free(char_timeout_timer);

    posix_serial_port_close(&posix_serial_port);
}

struct emb_protocol_t* posix_serial_rtu_t::get_proto() {
    return &modbus_rtu.proto;
}


void posix_serial_rtu_t::modbus_rtu_on_char(void* _user_data) {
    posix_serial_rtu_t* _this = (posix_serial_rtu_t*)_user_data;
    event_add(_this->char_timeout_timer, &_this->char_pause);
}

void posix_serial_rtu_t::on_timer(evutil_socket_t fd, short what, void *arg) {
    posix_serial_rtu_t* _this = (posix_serial_rtu_t*)arg;

    emb_rtu_on_char_timeout(&_this->modbus_rtu);
}
