
#include "modbus_sched.h"
#include "string.h"

#include <stdio.h>

static void on_receive_pkt(void* _user_data,
                           const void* _pkt,
                           unsigned int _pkt_size) {

    struct modbus_scheduler_t* sched = (struct modbus_scheduler_t*)_user_data;

    printf("%s:\n", __PRETTY_FUNCTION__);
    fflush(stdout);

    if(sched->current_task) {
        sched->modbus_sched_stop_wait(sched);
        sched->current_task->on_response(sched->current_task, _pkt, _pkt_size);
        sched->current_task = (struct modbus_task_t*)0;
    }
}

static void on_task(void *_data) {

    struct modbus_task_t* mt = (struct modbus_task_t*)_data;
    struct modbus_scheduler_t* sched = mt->sched;
    sched->current_task = mt;
    modbus_proto_send_packet(sched->proto, mt->request, mt->req_size);
    sched->modbus_sched_start_wait(sched, mt->resp_timeout);
}

void modbus_scheduler_initialize(struct modbus_scheduler_t *_modbus_sched) {
    memset(&_modbus_sched->sched, 0, sizeof(struct scheduler_t));
    scheduler_init(&_modbus_sched->sched, _modbus_sched);
    _modbus_sched->proto->modbus_proto_recv_packet = on_receive_pkt;
    _modbus_sched->proto->recv_user_data = _modbus_sched;
    _modbus_sched->current_task = (struct modbus_task_t*)0;
}

void modbus_scheduler_add_task(struct modbus_scheduler_t* _modbus_sched,
                               struct modbus_task_t* _modbus_task) {
    _modbus_task->task.proc_data = _modbus_task;
    _modbus_task->task.task_proc = on_task;
    _modbus_task->sched = _modbus_sched;
    scheduler_add_task(&_modbus_sched->sched, &_modbus_task->task);
}

int modbus_scheduler_process_one_task(struct modbus_scheduler_t* _modbus_sched) {
    scheduler_process_one_task(&_modbus_sched->sched);
    return !scheduler_queue_empty(&_modbus_sched->sched);
}

void modbus_scheduler_wait_timeout(struct modbus_scheduler_t* _modbus_sched) {
    struct modbus_task_t* mt = _modbus_sched->current_task;
    mt->on_resp_timeout(mt);
    _modbus_sched->current_task = (struct modbus_task_t*)0;
}
