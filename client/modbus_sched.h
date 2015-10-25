
#ifndef MODBUS_MASTER_SCHEDULER_H
#define MODBUS_MASTER_SCHEDULER_H

#include "../base/add/scheduler.h"

#include "../base/modbus_proto.h"

#ifdef __cplusplus
extern "C" {
#endif

/*

    |  high level  |
    |______________|
       |       ^
addTask|       | { on_response, on_resp_timeout }
     __V_______|___
    |  |           |
    | |_|q         |
    | |_|u         |
    | |_|e         |
    | |_|u         |
    | |_|e         |
    |__|___________|
       |       ^
       | proto |
     __V_______|___
    |              |
    |   low level  |

*/

struct modbus_task_t;
struct modbus_scheduler_t;

typedef void (*on_modbus_task_response_t)(struct modbus_task_t*,
                                          int _slave_addr,
                                          emb_const_pdu_t*);

typedef void (*on_modbus_task_timeout_t)(struct modbus_task_t*);

enum modbus_task_state_t {
    mt_state_no_task,
    mt_state_wait_processing,
    mt_state_sending_req,
    mt_state_wait_resp
};

// Одна задача
struct modbus_task_t {
    int slave_addr;
    emb_const_pdu_t* req;
    unsigned int resp_timeout;
    on_modbus_task_response_t on_response;
    on_modbus_task_timeout_t on_resp_timeout;
    void* user_data;
    struct modbus_scheduler_t* sched;
    enum modbus_task_state_t state;
    struct task_t task;
};

typedef void (*modbus_sched_start_wait_t)(struct modbus_scheduler_t* _sched, unsigned int _time);
typedef void (*modbus_sched_stop_wait_t)(struct modbus_scheduler_t* _sched);

// Планировщик пакетов
struct modbus_scheduler_t {

    modbus_sched_start_wait_t modbus_sched_start_wait;
    modbus_sched_stop_wait_t modbus_sched_stop_wait;
    void* user_data;
    struct emb_protocol_t* proto;

    struct modbus_task_t* current_task;
    struct scheduler_t sched;
};

void modbus_scheduler_init_task(struct modbus_task_t* _task, void* _user_data,
                                on_modbus_task_response_t _on_response,
                                on_modbus_task_timeout_t _on_timeout);

void modbus_scheduler_initialize(struct modbus_scheduler_t* _modbus_sched);

int modbus_scheduler_add_task(struct modbus_scheduler_t* _modbus_sched, struct modbus_task_t* _modbus_task);

int modbus_scheduler_process_one_task(struct modbus_scheduler_t* _modbus_sched);

void modbus_scheduler_wait_timeout(struct modbus_scheduler_t* _modbus_sched);

enum modbus_task_state_t modbus_scheduler_get_state(struct modbus_scheduler_t* _modbus_sched);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_SCHEDULER_H
