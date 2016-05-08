
#include "dumping_helper.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <emodbus/base/common.h>

emb_debug_helper_t::emb_debug_helper_t() {

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
    emb_dump_rx_data = on_write_rx;
}

void emb_debug_helper_t::disable_rx_dumping() {
    emb_dump_rx_data = 0;
}

void emb_debug_helper_t::enable_tx_dumping() {
    emb_dump_tx_data = on_write_tx;
}

void emb_debug_helper_t::disable_tx_dumping() {
    emb_dump_tx_data = 0;
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

void emb_debug_helper_t::on_write_rx(const void* _data, unsigned int _size) {
    dbg_print_packet(stdout, ">>", _data, _size);
}

void emb_debug_helper_t::on_write_tx(const void* _data, unsigned int _size) {
    dbg_print_packet(stdout, "<<", _data, _size);
}
