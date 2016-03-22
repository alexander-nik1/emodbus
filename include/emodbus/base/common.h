
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

#ifdef __cplusplus
extern "C" {
#endif


// A timer interface
//  _______
// |       | <--- start
// | TIMER | <--- sync
// |_______| ----> event
//

//int modbus_check_answer(emb_const_pdu_t *_req,
//                        emb_const_pdu_t *_answ);

#ifdef __cplusplus
}
#endif

#endif // MODBUS_MASTER_COMMON_H
