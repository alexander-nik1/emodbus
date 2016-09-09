
#ifndef MODBUS_MASTER_COMMON_H
#define MODBUS_MASTER_COMMON_H

/*! \mainpage Modbus library
 *
 * \section Brief
 *
 * This is the introduction.
 *
 *  Class relations expressed via an inline dot graph:
 *  \dot
 *  digraph example {
 *      node [shape=record, fontname=Helvetica, fontsize=10];
 *      b [ label="class B" URL="\ref B"];
 *      c [ label="class C" URL="\ref C"];
 *      b -> c [ arrowhead="open", style="dashed" ];
 *  }
 *  \enddot
 *  Note that the classes
 */

#ifndef NULL
#define NULL ((void*)0)
#endif

#include "modbus_pdu.h"
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#if EMODBUS_PACKETS_DUMPING

typedef void (*emb_dumping_data_t)(const void* _data, unsigned int _size);

extern emb_dumping_data_t emb_dump_rx_data;
extern emb_dumping_data_t emb_dump_tx_data;

#endif // EMODBUS_PACKETS_DUMPING

#ifdef __cplusplus
}
#endif

#endif // MODBUS_MASTER_COMMON_H
