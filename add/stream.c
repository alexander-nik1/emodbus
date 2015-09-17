
#include "stream.h"

int stream_write(struct output_stream_t* _output_stream, const void* _data, unsigned int _size) {
	struct input_stream_t* is = _output_stream->input_stream;
    if(is->on_write)
        return is->on_write(is, _data, _size);
    else
        return 0;
}

int stream_read(struct input_stream_t* _input_stream, void* _data, unsigned int _size) {
	struct output_stream_t* os = _input_stream->output_stream;
    if(os->on_read)
        return os->on_read(os, _data, _size);
    else
        return 0;
}

void stream_connect(struct output_stream_t* _output_stream, struct input_stream_t* _input_stream) {
	_output_stream->input_stream = _input_stream;
	_input_stream->output_stream = _output_stream;
}
