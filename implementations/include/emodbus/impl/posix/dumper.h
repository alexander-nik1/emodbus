
#ifndef EMB_POSIX_IMPL_DUMPER_H
#define EMB_POSIX_IMPL_DUMPER_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#if EMODBUS_PACKETS_DUMPING

extern FILE* emb_posix_dumping_stream;

void emb_posix_dumper_enable_rx_tx();
void emb_posix_dumper_disable_rx_tx();
void emb_posix_dumper_enable_rx();
void emb_posix_dumper_disable_rx();
void emb_posix_dumper_enable_tx();
void emb_posix_dumper_disable_tx();

unsigned long emb_posix_dumper_rx_bytes();
unsigned long emb_posix_dumper_tx_bytes();

#endif // EMODBUS_PACKETS_DUMPING

#ifdef __cplusplus
} // extern "C"
#endif

#endif // EMB_POSIX_IMPL_DUMPER_H
