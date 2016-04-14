
#ifndef EMB_POSIX_SERIAL_RTU_H
#define EMB_POSIX_SERIAL_RTU

#include "posix-serial-port.h"
#include <emodbus/protocols/rtu.h>
#include <event2/event.h>
#include <emodbus/base/modbus_pdu.h>
#include <vector>

class posix_serial_rtu_t {
public:
    posix_serial_rtu_t(struct event_base *_base,
                            const char* _dev_name,
                            unsigned int _baudrate);

    ~posix_serial_rtu_t();

    struct emb_protocol_t* get_proto();

private:
    static void modbus_rtu_on_char(void* _user_data);
    static void on_timer(evutil_socket_t fd, short what, void *arg);
    std::vector<unsigned char> rx_buffer, tx_buffer;

private:
    struct posix_serial_port_t posix_serial_port;
    struct emb_rtu_t modbus_rtu;
    struct timeval char_pause;
    struct event *char_timeout_timer;
};

#endif // EMB_POSIX_SERIAL_RTU
