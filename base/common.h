
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

// A timer interface
//  _______
// |       | <--- start
// | TIMER | <--- sync
// |_______| ----> event
//

struct emb_timer_i {
    void (*start)(void* _user_data, unsigned int _timer_value);
    void (*join)(void* _user_data);
    void (*event)(struct emb_timer_i*);
    void* user_data;
};

struct emb_timed_mutex_i {
    int (*lock_timeout)(void* _user_data, unsigned int _timeout);
    void (*unlock)(void* _user_data);
    void* user_data;
};

int modbus_check_answer(const struct modbus_const_pdu_t* _req,
                        const struct modbus_const_pdu_t* _answ);

#endif // MODBUS_MASTER_COMMON_H
