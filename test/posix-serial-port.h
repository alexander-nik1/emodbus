
#ifndef _POSIX_SERIAL_PORT_H_
#define _POSIX_SERIAL_PORT_H_

#include "../base/add/stream.h"
#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

struct posix_serial_port_t {
    struct input_stream_t input_stream;
    struct output_stream_t output_stream;
    struct event* read_event, *write_event;
    int f;
};

int posix_serial_port_open(struct posix_serial_port_t* _psp,
                           struct event_base* _ev_base,
                           const char* _dev_name,
                           unsigned int _baudrate);

void posix_serial_port_close(struct posix_serial_port_t* _psp);

int posix_serial_port_set_baudrate(struct posix_serial_port_t* _psp,
                                   unsigned int _baudrate);

int posix_serial_port_set_blocking(struct posix_serial_port_t* _psp,
                                   int _blocking_enable);

#ifdef __cplusplus
}
#endif

#endif // _POSIX_SERIAL_PORT_H_
