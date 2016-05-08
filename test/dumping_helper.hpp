
#ifndef EMB_DUMPING_HELPER_H
#define EMB_DUMPING_HELPER_H

#include <streams/stream.h>

class emb_debug_helper_t {
public:

    emb_debug_helper_t();

    void enable_dumping();
    void disable_dumping();
    void enable_rx_dumping();
    void disable_rx_dumping();
    void enable_tx_dumping();
    void disable_tx_dumping();

private:
    static void dbg_print_packet(void* _f, const char* _prefix, const void* _pkt, unsigned int _size);
    static int on_write_rx(struct input_stream_t* _this, const void* _data, unsigned int _size);
    static int on_write_tx(struct input_stream_t* _this, const void* _data, unsigned int _size);

private:
    input_stream_t emb_dumpi_rx;
    input_stream_t emb_dumpi_tx;
};

#endif // EMB_DUMPING_HELPER_H
