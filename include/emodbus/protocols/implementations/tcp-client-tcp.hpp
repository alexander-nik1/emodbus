
#ifndef EMB_TCP_CLIENT_TCP
#define EMB_TCP_CLIENT_TCP

#include <emodbus/protocols/tcp.h>
#include <emodbus/base/modbus_pdu.h>
#include <emodbus/protocols/implementations/tcp-client.h>
#include <event2/event.h>
#include <vector>


class tcp_client_tcp_t {
public:
    tcp_client_tcp_t();
    ~tcp_client_tcp_t();

    int open(struct event_base *_base,
             const char* _ip_addr,
             unsigned int _port);

    void close();

    struct emb_protocol_t* get_proto();

private:
    void tcp_init();
    static void on_timer(evutil_socket_t fd, short what, void *arg);

    static unsigned int read_from_port(struct emb_tcp_t* _mbt, void* _p_buf, unsigned int _buf_size);
    static unsigned int write_to_port(struct emb_tcp_t* _mbt, const void* _p_data, unsigned int _sz_to_write);

    static void tcp_cient_notifier(struct tcp_client_t* _ctx,
                                   enum tcp_client_events_t _event);

    std::vector<unsigned char> rx_buffer, tx_buffer;

private:
    struct emb_tcp_t modbus_tcp;

    struct tcp_client_t* tcp_client;
    bool opened_flag;
};

#endif // EMB_TCP_CLIENT_TCP
