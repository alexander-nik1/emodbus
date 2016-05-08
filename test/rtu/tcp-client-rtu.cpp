
#include "tcp-client-rtu.hpp"
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
    , bev(NULL)
    , opened_flag(false)
{ }

tcp_client_rtu_t::~tcp_client_rtu_t() {
    close();
}

int tcp_client_rtu_t::open(event_base *_base,
                       const char* _ip_addr,
                       unsigned int _port) {

    printf("%s: opening TCP connection to %s:%d\n", __PRETTY_FUNCTION__, _ip_addr, _port);

    memset(&sin, 0, sizeof(struct sockaddr_in));

    sin.sin_family = AF_INET;
    inet_aton(_ip_addr, &sin.sin_addr);
    sin.sin_port = htons(_port);

    bev = bufferevent_socket_new(_base, -1, BEV_OPT_CLOSE_ON_FREE);

    if(!bev)
        return -1;

    bufferevent_setcb(bev, readcb, writecb, eventcb, this);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    if (bufferevent_socket_connect(bev,
                                   (struct sockaddr *)&sin,
                                   sizeof(sin)) < 0) {
        /* Error starting connection */
        bufferevent_free(bev);
        fprintf(stderr, "%s(): error with connection: %m\n", __FUNCTION__);
        return -1;
    }

    char_pause.tv_sec = 0;
  //  enum { pause = 100 };
    char_pause.tv_usec = 1000 * 10; //(1000 * 1000) / (_baudrate / pause);

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

    if(bev) {
        bufferevent_free(bev);
        bev = NULL;
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
        emb_rtu_on_error(_emb, -EFAULT);
        return;
    }
    event_add(_this->char_timeout_timer, &_this->char_pause);
}

void tcp_client_rtu_t::on_timer(evutil_socket_t fd, short what, void *arg) {
    tcp_client_rtu_t* _this = (tcp_client_rtu_t*)arg;
    emb_rtu_on_char_timeout(&_this->modbus_rtu);
}

unsigned int tcp_client_rtu_t::read_from_port(struct emb_rtu_t* _mbt,
                                          void* _p_buf,
                                          unsigned int _buf_size) {

    tcp_client_rtu_t* _this = container_of(_mbt, tcp_client_rtu_t, modbus_rtu);
    struct evbuffer *input;
    size_t readed;

    if(!_this->opened_flag) {
        emb_rtu_on_error(_mbt, -EFAULT);
        return 0;
    }

    input = bufferevent_get_input(_this->bev);
    readed = evbuffer_copyout(input, _p_buf, _buf_size);

    if(readed == (size_t)-1)
        return 0;
    evbuffer_drain(input, readed);

    return readed;
}

unsigned int tcp_client_rtu_t::write_to_port(struct emb_rtu_t* _mbt,
                                         const void* _p_data,
                                         unsigned int _sz_to_write) {
    tcp_client_rtu_t* _this = container_of(_mbt, tcp_client_rtu_t, modbus_rtu);
    struct evbuffer *output;

    if(!_this->opened_flag) {
        emb_rtu_on_error(_mbt, -EFAULT);
        return 0;
    }

    output = bufferevent_get_output(_this->bev);

    if(!_sz_to_write)
        return 0;

    evbuffer_add(output, _p_data, _sz_to_write);

    return _sz_to_write;
}

void tcp_client_rtu_t::readcb(struct bufferevent *bev, void *ctx) {
    tcp_client_rtu_t* _this = (tcp_client_rtu_t*)ctx;
    emb_rtu_port_event(&_this->modbus_rtu, emb_rtu_data_received_event);
}

void tcp_client_rtu_t::writecb(struct bufferevent *bev, void *ctx) {
    tcp_client_rtu_t* _this = (tcp_client_rtu_t*)ctx;
    emb_rtu_port_event(&_this->modbus_rtu, emb_rtu_tx_buf_empty_event);
}

static void set_tcp_no_delay(evutil_socket_t fd) {
    int one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
               &one, sizeof one);
}

void tcp_client_rtu_t::eventcb(struct bufferevent *bev, short events, void *ptr) {

    tcp_client_rtu_t* _this = (tcp_client_rtu_t*)ptr;
    printf("%s() events=0x%02X\n", __FUNCTION__, events);
    fflush(stdout);

    if (events & BEV_EVENT_CONNECTED) {
        evutil_socket_t fd = bufferevent_getfd(bev);
        set_tcp_no_delay(fd);
        _this->opened_flag = true;
    }
    else if (events & BEV_EVENT_ERROR) {
        printf("NOT Connected\n");
    }
}
