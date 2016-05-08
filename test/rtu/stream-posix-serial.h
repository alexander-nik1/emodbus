
#ifndef STREAM_POSIX_SERIAL_H
#define STREAM_POSIX_SERIAL_H

#include <streams/stream.h>
#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

struct stream_posix_serial_t {
    struct input_stream_t input_stream;
    struct output_stream_t output_stream;
    struct event* read_event, *write_event;
    int f;
};

int stream_posix_serial_open(struct stream_posix_serial_t* _psp,
                             struct event_base* _ev_base,
                             const char* _dev_name,
                             unsigned int _baudrate);

void stream_posix_serial_close(struct stream_posix_serial_t* _psp);

int stream_posix_serial_set_baudrate(struct stream_posix_serial_t* _psp,
                                     unsigned int _baudrate);

int stream_posix_serial_set_blocking(struct stream_posix_serial_t* _psp,
                                     int _blocking_enable);

#ifdef __cplusplus
}
#endif

#endif // STREAM_POSIX_SERIAL_H
