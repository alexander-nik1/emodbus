
#include <emodbus/impl/posix/dumper.h>
#include <emodbus/base/common.h>
#include <stdint.h>

#if EMODBUS_PACKETS_DUMPING

FILE* emb_posix_dumping_stream = NULL;

static unsigned long rx_bytes = 0UL;
static unsigned long tx_bytes = 0UL;

static void dbg_print_packet(void *_f, const char* _prefix,
                                const void* _pkt, unsigned int _size) {
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

static void emb_posix_on_write_rx(const void* _data,
                           unsigned int _size) {
    dbg_print_packet(emb_posix_dumping_stream, ">>", _data, _size);
    rx_bytes += _size;
}

static void emb_posix_on_write_tx(const void* _data,
                            unsigned int _size) {
    dbg_print_packet(emb_posix_dumping_stream, "<<", _data, _size);
    tx_bytes += _size;
}

void emb_posix_dumper_enable_rx_tx() {
    emb_posix_dumper_enable_rx();
    emb_posix_dumper_enable_tx();
}

void emb_posix_dumper_disable_rx_tx() {
    emb_posix_dumper_disable_rx();
    emb_posix_dumper_disable_tx();
}

void emb_posix_dumper_enable_rx() { emb_dump_rx_data = emb_posix_on_write_rx; }
void emb_posix_dumper_disable_rx() { emb_dump_rx_data = NULL; }
void emb_posix_dumper_enable_tx() { emb_dump_tx_data = emb_posix_on_write_tx; }
void emb_posix_dumper_disable_tx() { emb_dump_tx_data = NULL; }

unsigned long emb_posix_dumper_rx_bytes() { return rx_bytes; }
unsigned long emb_posix_dumper_tx_bytes() { return tx_bytes; }

#endif // EMODBUS_PACKETS_DUMPING
