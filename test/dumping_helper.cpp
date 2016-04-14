
#include "dumping_helper.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <emodbus/base/common.h>

emb_debug_helper_t::emb_debug_helper_t() {
    emb_dumpi_rx.on_write = on_write_rx;
    emb_dumpi_rx.output_stream = NULL;

    emb_dumpi_tx.on_write = on_write_tx;
    emb_dumpi_tx.output_stream = NULL;
}

void emb_debug_helper_t::enable_dumping() {
    enable_rx_dumping();
    enable_tx_dumping();
}

void emb_debug_helper_t::disable_dumping() {
    disable_rx_dumping();
    disable_tx_dumping();
}

void emb_debug_helper_t::enable_rx_dumping() {
    stream_connect(&emb_dump_rx, &emb_dumpi_rx);
}

void emb_debug_helper_t::disable_rx_dumping() {
    stream_disconnect(&emb_dump_rx, &emb_dumpi_rx);
}

void emb_debug_helper_t::enable_tx_dumping() {
    stream_connect(&emb_dump_tx, &emb_dumpi_tx);
}

void emb_debug_helper_t::disable_tx_dumping() {
    stream_disconnect(&emb_dump_tx, &emb_dumpi_tx);
}

void emb_debug_helper_t::dbg_print_packet(void *_f, const char* _prefix, const void* _pkt, unsigned int _size) {
    if(_f) {
        FILE* f = (FILE*)_f;
        int i;
        fprintf(f, "%s", _prefix);
        for(i=0; i<_size; ++i) {
            fprintf(f, "%02X ", ((uint8_t*)_pkt)[i]);
        }
        fprintf(f, "\n");
        fflush(f);
    }
}

int emb_debug_helper_t::on_write_rx(struct input_stream_t* _this, const void* _data, unsigned int _size) {
    dbg_print_packet(stdout, ">>", _data, _size);
    return _size;
}

int emb_debug_helper_t::on_write_tx(struct input_stream_t* _this, const void* _data, unsigned int _size) {
    dbg_print_packet(stdout, "<<", _data, _size);
    return _size;
}
