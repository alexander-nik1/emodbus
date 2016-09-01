
#include <emodbus/protocols/implementations/tcp-client-tcp.hpp>
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

tcp_client_tcp_t::tcp_client_tcp_t()
    : tcp_client(NULL)
    , opened_flag(false)
{ }

tcp_client_tcp_t::~tcp_client_tcp_t() {
    close();
}

int tcp_client_tcp_t::open(event_base *_base,
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

    tcp_init();

    return 0;
}

void tcp_client_tcp_t::close() {

    if(tcp_client) {
        tcp_client_free(tcp_client);
        tcp_client = NULL;
    }

    opened_flag = false;
}

struct emb_protocol_t* tcp_client_tcp_t::get_proto() {
    return &modbus_tcp.proto;
}

void tcp_client_tcp_t::tcp_init() {
    rx_buffer.resize(MAX_PDU_SIZE);
    tx_buffer.resize(MAX_PDU_SIZE);

    modbus_tcp.read_from_port = read_from_port;
    modbus_tcp.write_to_port = write_to_port;

    emb_tcp_initialize(&modbus_tcp);
}

unsigned int tcp_client_tcp_t::read_from_port(struct emb_tcp_t* _mbt,
                                          void* _p_buf,
                                          unsigned int _buf_size) {

    tcp_client_tcp_t* _this = container_of(_mbt, tcp_client_tcp_t, modbus_tcp);

    if(!_this->opened_flag) {
        //emb_tcp_on_error(_mbt, -EFAULT);
        return 0;
    }

    return tcp_client_read(_this->tcp_client, _p_buf, _buf_size);
}

unsigned int tcp_client_tcp_t::write_to_port(struct emb_tcp_t* _mbt,
                                         const void* _p_data,
                                         unsigned int _sz_to_write) {
    int wrote;
    tcp_client_tcp_t* _this = container_of(_mbt, tcp_client_tcp_t, modbus_tcp);

    if(!_this->opened_flag) {
//        emb_tcp_on_error(_mbt, -EFAULT);
        return 0;
    }

    if(!_sz_to_write)
        return 0;

    wrote = tcp_client_write(_this->tcp_client, _p_data, _sz_to_write);

    if(wrote > 0 && wrote < _sz_to_write)
        tcp_client_enable_write_event(_this->tcp_client);

    return wrote;
}

void tcp_client_tcp_t::tcp_cient_notifier(struct tcp_client_t* _ctx,
                                          enum tcp_client_events_t _event) {

    tcp_client_tcp_t* _this = (tcp_client_tcp_t*)tcp_client_get_user_data(_ctx);

    switch(_event) {
    case tcp_cli_data_received:
        emb_tcp_port_event(&_this->modbus_tcp, NULL, emb_tcp_data_received_event);
        break;

    case tcp_cli_data_sent:
        emb_tcp_port_event(&_this->modbus_tcp, NULL, emb_tcp_tx_buf_empty_event);
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
