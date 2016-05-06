
#ifndef STREAM_TCP_CLIENT_H
#define STREAM_TCP_CLIENT_H

#include <emodbus/base/add/stream.h>
#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

struct stream_tcp_client_t {
    struct input_stream_t input_stream;
    struct output_stream_t output_stream;
    struct event* read_event, *write_event;
    struct sockaddr_in sin;
    int f;
};

int stream_tcp_client_open(struct stream_tcp_client_t* _psp,
                             struct event_base* _ev_base,
                             const char* _ip_addr,
                             unsigned int _port);

void stream_tcp_client_close(struct stream_tcp_client_t* _psp);

#ifdef __cplusplus
}
#endif

#endif // STREAM_TCP_CLIENT_H
