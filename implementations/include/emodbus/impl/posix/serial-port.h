
#ifndef EMB_SERIAL_H
#define EMB_SERIAL_H

#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

enum serial_port_events_t {
    serial_port_data_received_event,
    serial_port_data_sent_event
};

struct serial_port_t;

typedef void (*serial_port_notifier_t)(void* _param,
                                       enum serial_port_events_t _event);

struct serial_port_t*
serial_port_create(struct event_base *_base,
                   const char* _dev_name,
                   unsigned int _baudrate);

void serial_port_destroy(struct serial_port_t* _ctx);

int serial_port_set_notifier(struct serial_port_t* _ctx,
                             serial_port_notifier_t _notifier,
                             void* _param);

int serial_port_set_baudrate(struct serial_port_t* _ctx,
                             unsigned int _baudrate);

int serial_port_read(struct serial_port_t* _ctx,
                     void* _p_buf,
                     unsigned int _buf_size);

int serial_port_write(struct serial_port_t* _ctx,
                      const void* _p_data,
                      unsigned int _sz_to_write,
                      unsigned int* _wrote);

#ifdef __cplusplus
};
#endif

#endif // EMB_SERIAL_H
