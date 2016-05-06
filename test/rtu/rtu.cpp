
#include "rtu.hpp"
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_pdu.h>
#include <string.h>

rtu_t::rtu_t(struct event_base *_base,
                        const char* _dev_name,
                        unsigned int _baudrate) {

    tcp_client = NULL;
    posix_serial_port = new stream_posix_serial_t;

    memset(posix_serial_port, 0, sizeof(stream_posix_serial_t));

    stream_posix_serial_open(posix_serial_port, _base, _dev_name, _baudrate);

    rx_buffer.resize(MAX_PDU_SIZE);
    tx_buffer.resize(MAX_PDU_SIZE);

    modbus_rtu.rx_buffer = &rx_buffer[0];
    modbus_rtu.tx_buffer = &tx_buffer[0];
    modbus_rtu.rx_buf_size = rx_buffer.size();
    modbus_rtu.tx_buf_size = tx_buffer.size();
    modbus_rtu.emb_rtu_on_char = modbus_rtu_on_char;

    emb_rtu_initialize(&modbus_rtu);

    stream_connect(&posix_serial_port->output_stream, &modbus_rtu.input_stream);
    stream_connect(&modbus_rtu.output_stream, &posix_serial_port->input_stream);

    char_pause.tv_sec = 0;
  //  enum { pause = 100 };
    char_pause.tv_usec = 1000 * 10; //(1000 * 1000) / (_baudrate / pause);

    char_timeout_timer = event_new(_base, -1, EV_TIMEOUT/* | EV_PERSIST*/, on_timer, this);
}

rtu_t::rtu_t(struct event_base *_base,
             unsigned int _port,
             const char* _ip_address) {

    posix_serial_port = NULL;
    tcp_client = new stream_tcp_client_t;

    memset(tcp_client, 0, sizeof(stream_tcp_client_t));

    stream_tcp_client_open(tcp_client, _base, _ip_address, _port);

    rx_buffer.resize(MAX_PDU_SIZE);
    tx_buffer.resize(MAX_PDU_SIZE);

    modbus_rtu.rx_buffer = &rx_buffer[0];
    modbus_rtu.tx_buffer = &tx_buffer[0];
    modbus_rtu.rx_buf_size = rx_buffer.size();
    modbus_rtu.tx_buf_size = tx_buffer.size();
    modbus_rtu.emb_rtu_on_char = modbus_rtu_on_char;

    emb_rtu_initialize(&modbus_rtu);

    stream_connect(&tcp_client->output_stream, &modbus_rtu.input_stream);
    stream_connect(&modbus_rtu.output_stream, &tcp_client->input_stream);

    char_pause.tv_sec = 0;
  //  enum { pause = 100 };
    char_pause.tv_usec = 1000 * 1000; //(1000 * 1000) / (_baudrate / pause);

    char_timeout_timer = event_new(_base, -1, EV_TIMEOUT/* | EV_PERSIST*/, on_timer, this);
}

rtu_t::~rtu_t() {

    event_del(char_timeout_timer);
    event_free(char_timeout_timer);

    if(posix_serial_port) {
        stream_posix_serial_close(posix_serial_port);
        delete posix_serial_port;
    }

    if(tcp_client) {
        stream_tcp_client_close(tcp_client);
        delete tcp_client;
    }
}

struct emb_protocol_t* rtu_t::get_proto() {
    return &modbus_rtu.proto;
}


void rtu_t::modbus_rtu_on_char(struct emb_rtu_t* _emb) {
    rtu_t* _this = container_of(_emb, rtu_t, modbus_rtu);
    event_add(_this->char_timeout_timer, &_this->char_pause);
}

void rtu_t::on_timer(evutil_socket_t fd, short what, void *arg) {
    rtu_t* _this = (rtu_t*)arg;
    emb_rtu_on_char_timeout(&_this->modbus_rtu);
}
