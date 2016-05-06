
#ifndef EMB_POSIX_SERIAL_RTU
#define EMB_POSIX_SERIAL_RTU

#include "stream-posix-serial.h"
#include "stream-tcp-client.h"

#include <emodbus/protocols/rtu.h>
#include <event2/event.h>
#include <emodbus/base/modbus_pdu.h>
#include <vector>

class rtu_t {
public:
    rtu_t(struct event_base *_base,
          unsigned int _baudrate,
          const char* _dev_name);

    rtu_t(struct event_base *_base,
          const char* _ip_address,
          unsigned int _port);

    ~rtu_t();

    struct emb_protocol_t* get_proto();

private:
    static void modbus_rtu_on_char(struct emb_rtu_t *_emb);
    static void on_timer(evutil_socket_t fd, short what, void *arg);
    std::vector<unsigned char> rx_buffer, tx_buffer;

private:

    struct stream_posix_serial_t* posix_serial_port;
    struct stream_tcp_client_t* tcp_client;

    struct emb_rtu_t modbus_rtu;
    struct timeval char_pause;
    struct event *char_timeout_timer;
};

#endif // EMB_POSIX_SERIAL_RTU
