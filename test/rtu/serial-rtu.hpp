
#ifndef EMB_POSIX_SERIAL_RTU
#define EMB_POSIX_SERIAL_RTU

#include <emodbus/protocols/rtu.h>
#include <event2/event.h>
#include <emodbus/base/modbus_pdu.h>
#include <vector>

class serial_rtu_t {
public:
    serial_rtu_t();
    ~serial_rtu_t();

    int open(struct event_base *_base,
             const char* _dev_name,
             unsigned int _baudrate);

    int set_baudrate(unsigned int _baudrate);

    void close();

    struct emb_protocol_t* get_proto();

private:
    void rtu_init();
    static void modbus_rtu_on_char(struct emb_rtu_t *_emb);
    static void on_timer(evutil_socket_t fd, short what, void *arg);

    static unsigned int read_from_port(struct emb_rtu_t* _mbt, void* _p_buf, unsigned int _buf_size);
    static unsigned int write_to_port(struct emb_rtu_t* _mbt, const void* _p_data, unsigned int _sz_to_write);


    static void fd_event(evutil_socket_t fd, short what, void *arg);

    std::vector<unsigned char> rx_buffer, tx_buffer;

private:
    struct emb_rtu_t modbus_rtu;
    struct timeval char_pause;
    struct event *char_timeout_timer;
    struct event* read_event, *write_event;
    int fd;
};

#endif // EMB_POSIX_SERIAL_RTU
