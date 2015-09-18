
#include "sched.h"

static void on_task(void *_data) {
    struct modbus_task_t* mt = (struct modbus_task_t*)_data;

    //mt->
    // send packet
    // receive packet or timeout
}

void modbus_scheduler_initialize(struct modbus_scheduler_t *_modbus_sched) {
    scheduler_init(&_modbus_sched->sched, _modbus_sched);
}

void modbus_scheduler_add_task(struct modbus_scheduler_t* _modbus_sched, struct modbus_task_t* _modbus_task) {
    _modbus_task->task.proc_data = _modbus_task;
    _modbus_task->task.task_proc = on_task;
    scheduler_add_task(&_modbus_sched->sched, &_modbus_task->task);
}
