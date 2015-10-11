
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

#include "modbus_pdu.h"

int modbus_check_answer(const struct modbus_const_pdu_t* _req, const struct modbus_const_pdu_t* _answ);

#endif // MODBUS_MASTER_COMMON_H
