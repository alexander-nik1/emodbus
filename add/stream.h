
#ifndef THE_STREAM_H
#define THE_STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

struct input_stream_t;
struct output_stream_t;

struct input_stream_t {
	int (*on_write)(struct input_stream_t* _this, const void* _data, unsigned int _size);
	struct output_stream_t* output_stream;
};

struct output_stream_t {
	int (*on_read)(struct output_stream_t* _this, void* _data, unsigned int _size);
	struct input_stream_t* input_stream;
};

int stream_write(struct output_stream_t* _output_stream, const void* _data, unsigned int _size);

int stream_read(struct input_stream_t* _input_stream, void* _data, unsigned int _size);

void stream_connect(struct output_stream_t* _output_stream, struct input_stream_t* _input_stream);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // THE_STREAM_H
