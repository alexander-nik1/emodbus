
#ifndef EMB_TCP_CLIENT_RTU
#define EMB_TCP_CLIENT_RTU

#include <emodbus/protocols/rtu.h>
#include <event2/event.h>
#include <emodbus/base/modbus_pdu.h>
#include <vector>

class tcp_client_rtu_t {
public:
    tcp_client_rtu_t();
    ~tcp_client_rtu_t();

    int open(struct event_base *_base,
             const char* _ip_addr,
             unsigned int _port);

    void close();

    struct emb_protocol_t* get_proto();

private:
    void rtu_init();
    static void modbus_rtu_on_char(struct emb_rtu_t *_emb);
    static void on_timer(evutil_socket_t fd, short what, void *arg);

    static unsigned int read_from_port(struct emb_rtu_t* _mbt, void* _p_buf, unsigned int _buf_size);
    static unsigned int write_to_port(struct emb_rtu_t* _mbt, const void* _p_data, unsigned int _sz_to_write);


    static void readcb(struct bufferevent *bev, void *ctx);
    static void writecb(struct bufferevent *bev, void *ctx);
    static void eventcb(struct bufferevent *bev, short events, void *ptr);

    std::vector<unsigned char> rx_buffer, tx_buffer;

private:
    struct emb_rtu_t modbus_rtu;
    struct timeval char_pause;
    struct event *char_timeout_timer;
    struct event *connect_timeout_timer;
    struct bufferevent* bev;
    struct sockaddr_in sin;
    bool opened_flag;
};

#endif // EMB_TCP_CLIENT_RTU
