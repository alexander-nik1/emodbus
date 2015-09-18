
#ifndef MODBUS_MASTER_SCHEDULER_H
#define MODBUS_MASTER_SCHEDULER_H

#include "add/scheduler.h"

#include "rtu.h"

#ifdef __cplusplus
extern "C" {
#endif

/*

    |  high level  |
    |______________|
       |       ^
       |       |
     __V_______|___
    |  _           |
    | |_|q         |
    | |_|u         |
    | |_|e         |
    | |_|u         |
    | |_|e         |
    |______________|
       |       ^
       |       |
     __V_______|___
    |              |
    |   low level  |

*/

struct modbus_task_t;

typedef (*on_modbus_task_response_t)(struct modbus_task_t*,
                                     void* user_data,
                                     const void* _pkt,
                                     unsigned int _pkt_size);

typedef (*on_modbus_task_error_t)(struct modbus_task_t*,
                                  void* user_data,
                                  int _err_code);

struct modbus_task_t {
    const void* request;
    unsigned int req_size;
    unsigned int resp_timeout;
    on_modbus_task_response_t on_response;
    on_modbus_task_error_t on_error;
    void* user_data;
    struct task_t task;
};

struct modbus_scheduler_t {
    struct scheduler_t sched;
};

void modbus_scheduler_initialize(struct modbus_scheduler_t* _modbus_sched);

void modbus_scheduler_add_task(struct modbus_scheduler_t* _modbus_sched, struct modbus_task_t* _modbus_task);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_SCHEDULER_H
