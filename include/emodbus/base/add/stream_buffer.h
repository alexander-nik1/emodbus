
#ifndef THE_STREAM_BUFFER_H
#define THE_STREAM_BUFFER_H

#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

struct stream_buffer_t {
    char* buffer;
    unsigned int buf_size;
    struct input_stream_t input_stream;
    struct output_stream_t output_stream;
};

int stream_buffer_on_write(struct input_stream_t* _this, const void* _data, unsigned int _size);
int stream_buffer_on_read(struct output_stream_t* _this, void* _data, unsigned int _size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // THE_STREAM_BUFFER_H
