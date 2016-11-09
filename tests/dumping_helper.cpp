
#include "dumping_helper.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <emodbus/base/common.h>

emb_debug_helper_t::emb_debug_helper_t() {
    rx_dumping = tx_dumping = false;
    rx_bytes_ = tx_bytes_ = 0UL;
    emb_dump_rx_data = on_write_rx;
    emb_dump_tx_data = on_write_tx;
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
    rx_dumping = true;
}

void emb_debug_helper_t::disable_rx_dumping() {
    rx_dumping = false;
}

void emb_debug_helper_t::enable_tx_dumping() {
    tx_dumping = true;
}

void emb_debug_helper_t::disable_tx_dumping() {
    tx_dumping = false;
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
    if(emb_debug_helper.rx_dumping)
        dbg_print_packet(stdout, ">>", _data, _size);
    emb_debug_helper.rx_bytes_ += _size;
}

void emb_debug_helper_t::on_write_tx(const void* _data, unsigned int _size) {
    if(emb_debug_helper.tx_dumping)
        dbg_print_packet(stdout, "<<", _data, _size);
    emb_debug_helper.tx_bytes_ += _size;
}

unsigned long emb_debug_helper_t::rx_bytes() const
{ return rx_bytes_; }

unsigned long emb_debug_helper_t::tx_bytes() const
{ return tx_bytes_; }

emb_debug_helper_t emb_debug_helper;
