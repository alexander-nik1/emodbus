
#include "stream-tcp-client.h"
#include <emodbus/base/add/container_of.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 16

static int input_stream_on_write(struct input_stream_t* _this,
                                 const void* _data,
                                 unsigned int _size) {
    int r;
    struct stream_tcp_client_t* psp = container_of(_this, struct stream_tcp_client_t, input_stream);
    const int res = write(psp->f, _data, _size);
    printf("%s sz(%d), f(%d)\n", __PRETTY_FUNCTION__, _size, psp->f);
    if((r = event_add(psp->write_event, NULL)))
        fprintf(stderr, "Error with event_add() call: %m (%d)\n", r);
    return res;
}

static int output_stream_on_read(struct output_stream_t* _this,
                          void* _data,
                          unsigned int _size) {
    struct stream_tcp_client_t* psp = container_of(_this, struct stream_tcp_client_t, output_stream);
    return read(psp->f, _data, _size);
}

static void fd_event(evutil_socket_t fd, short what, void *arg) {
    struct stream_tcp_client_t* _this = (struct stream_tcp_client_t*)arg;

    char buffer[BUFFER_SIZE];

    int t;

    printf("%s(%d, 0x%02X)\n", __FUNCTION__, fd, what);

    if(what & EV_READ) {
        t = read(_this->f, buffer, BUFFER_SIZE);
        if(t > 0)
            stream_write(&_this->output_stream, buffer, t);
    }

    if(what & EV_WRITE) {
        t = stream_read(&_this->input_stream, buffer, BUFFER_SIZE);
        if(t > 0) {
            write(_this->f, buffer, t);
        }
        else if(!t)     // no more data available, stop the transmitting.
            event_del(_this->write_event);
    }
}

int stream_tcp_client_open(struct stream_tcp_client_t* _psp,
                             struct event_base* _ev_base,
                             const char* _ip_addr,
                             unsigned int _port) {

    int r = 0;

    _psp->input_stream.on_write = input_stream_on_write;
    _psp->output_stream.on_read = output_stream_on_read;

    do {
        memset(&_psp->sin, 0, sizeof(_psp->sin));

        _psp->sin.sin_family = AF_INET;
        inet_aton(_ip_addr, &_psp->sin.sin_addr);
        _psp->sin.sin_port = htons(_port);

        if((_psp->f = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            return -1;

        if(connect(_psp->f, (struct sockaddr*)&_psp->sin, sizeof(_psp->sin)))
            return -1;

        _psp->read_event = event_new(_ev_base,
                                     _psp->f,
                                     EV_READ | EV_PERSIST,
                                     fd_event,
                                     (void*)_psp);

        if(!_psp->read_event) {
            fprintf(stderr, "Error with event_new() call: %m\n");
            r = -1;
            break;
        }

        _psp->write_event = event_new(_ev_base,
                                      _psp->f,
                                      EV_WRITE | EV_PERSIST,
                                      fd_event,
                                      (void*)_psp);

        if(!_psp->write_event) {
            fprintf(stderr, "Error with event_new() call: %m\n");
            r = -1;
            break;
        }

        if((r = event_add(_psp->read_event, NULL))) {
            fprintf(stderr, "Error with event_add() call: %m\n");
            break;
        }

        return 0;

    } while(0);

    stream_posix_serial_close(_psp);

    return errno == 0 ? r : errno;
}

void stream_tcp_client_close(struct stream_tcp_client_t* _psp) {
    if(_psp->read_event) {
        event_del(_psp->read_event);
        event_free(_psp->read_event);
        _psp->read_event = NULL;
    }

    if(_psp->write_event) {
        event_del(_psp->write_event);
        event_free(_psp->write_event);
        _psp->write_event = NULL;
    }

    if(_psp->f != -1) {
        close(_psp->f);
        _psp->f = -1;
    }
}
