
#include <emodbus/implementations/posix/tcp-client-rtu.hpp>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_pdu.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <errno.h>

tcp_client_rtu_t::tcp_client_rtu_t()
    : char_timeout_timer(NULL)
    , tcp_client(NULL)
    , opened_flag(false)
{ }

tcp_client_rtu_t::~tcp_client_rtu_t() {
    close();
}

int tcp_client_rtu_t::open(event_base *_base,
                       const char* _ip_addr,
                       unsigned int _port) {
    int res;

    printf("%s: opening TCP connection to %s:%d\n", __PRETTY_FUNCTION__, _ip_addr, _port);

    tcp_client = tcp_client_new(_base, tcp_cient_notifier);
    if(!tcp_client) {
        fprintf(stderr, "Error: tcp_client_new() returns NULL!: %m\n");
        return -1;
    }

    tcp_client_set_user_data(tcp_client, this);
    tcp_client_set_reconnection_delay(tcp_client, 5);

    if((res = tcp_client_start_connection(tcp_client, _ip_addr, _port))) {
        fprintf(stderr, "Error: with tcp_client_start_connection(): %m\n");
        return res;
    }

    char_pause.tv_sec = 0;
  //  enum { pause = 100 };
    char_pause.tv_usec = 1000 * 1; //(1000 * 1000) / (_baudrate / pause);

    char_timeout_timer = event_new(_base,
                                   -1,
                                   EV_TIMEOUT/* | EV_PERSIST*/,
                                   on_timer,
                                   this);
    if(!char_timeout_timer) {
        fprintf(stderr, "Error with event_new() call: %m\n");
        return -1;
    }

    rtu_init();

    return 0;
}

void tcp_client_rtu_t::close() {
    if(char_timeout_timer) {
        event_del(char_timeout_timer);
        event_free(char_timeout_timer);
        char_timeout_timer = NULL;
    }

    if(tcp_client) {
        tcp_client_free(tcp_client);
        tcp_client = NULL;
    }

    opened_flag = false;
}

struct emb_protocol_t* tcp_client_rtu_t::get_proto() {
    return &modbus_rtu.proto;
}

void tcp_client_rtu_t::rtu_init() {
    rx_buffer.resize(MAX_PDU_SIZE);
    tx_buffer.resize(MAX_PDU_SIZE);

    modbus_rtu.rx_buffer = &rx_buffer[0];
    modbus_rtu.tx_buffer = &tx_buffer[0];
    modbus_rtu.rx_buf_size = rx_buffer.size();
    modbus_rtu.tx_buf_size = tx_buffer.size();
    modbus_rtu.emb_rtu_on_char = modbus_rtu_on_char;

    modbus_rtu.read_from_port = read_from_port;
    modbus_rtu.write_to_port = write_to_port;

    emb_rtu_initialize(&modbus_rtu);
}

void tcp_client_rtu_t::modbus_rtu_on_char(struct emb_rtu_t* _emb) {
    tcp_client_rtu_t* _this = container_of(_emb, tcp_client_rtu_t, modbus_rtu);
    if(!_this->opened_flag) {
        //emb_rtu_on_error(_emb, -EFAULT);
        return;
    }
    event_add(_this->char_timeout_timer, &_this->char_pause);
}

void tcp_client_rtu_t::on_timer(evutil_socket_t fd, short what, void *arg) {
    tcp_client_rtu_t* _this = (tcp_client_rtu_t*)arg;
    emb_rtu_on_char_timeout(&_this->modbus_rtu);
}

int tcp_client_rtu_t::read_from_port(struct emb_rtu_t* _mbt,
                                          void* _p_buf,
                                          unsigned int _buf_size) {

    tcp_client_rtu_t* _this = container_of(_mbt, tcp_client_rtu_t, modbus_rtu);

    if(!_this->opened_flag) {
        //emb_rtu_on_error(_mbt, -EFAULT);
        return 0;
    }

    return tcp_client_read(_this->tcp_client, _p_buf, _buf_size);
}

int tcp_client_rtu_t::write_to_port(struct emb_rtu_t* _mbt,
                                         const void* _p_data,
                                         unsigned int _sz_to_write) {
    int wrote;
    tcp_client_rtu_t* _this = container_of(_mbt, tcp_client_rtu_t, modbus_rtu);

    if(!_this->opened_flag) {
//        emb_rtu_on_error(_mbt, -EFAULT);
        return 0;
    }

    if(!_sz_to_write)
        return 0;

    wrote = tcp_client_write(_this->tcp_client, _p_data, _sz_to_write);

    if(wrote > 0 && wrote < _sz_to_write)
        tcp_client_enable_write_event(_this->tcp_client);

    return wrote;
}

void tcp_client_rtu_t::tcp_cient_notifier(struct tcp_client_t* _ctx,
                                          enum tcp_client_events_t _event) {

    tcp_client_rtu_t* _this = (tcp_client_rtu_t*)tcp_client_get_user_data(_ctx);

    switch(_event) {
    case tcp_cli_data_received:
        emb_rtu_port_event(&_this->modbus_rtu, emb_rtu_data_received_event);
        break;

    case tcp_cli_data_sent:
        emb_rtu_port_event(&_this->modbus_rtu, emb_rtu_tx_buf_empty_event);
        break;

    case tcp_cli_connected:
        printf("================> event: tcp_cli_connected\n"); fflush(stdout);
        _this->opened_flag = true;
        break;

    case tcp_cli_disconnected:
        printf("================> event: tcp_cli_disconnected\n"); fflush(stdout);
        _this->opened_flag = false;
        break;

    case tcp_cli_bad_try_of_reconnection:
        printf("================> event: tcp_cli_bad_try_of_connection\n"); fflush(stdout);
        break;
    }
}
