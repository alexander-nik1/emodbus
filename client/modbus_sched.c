
#include "modbus_sched.h"
#include "string.h"
#include <errno.h>

static void on_receive_pkt(void* _user_data,
                           int _slave_addr,
                           emb_const_pdu_t* _pkt) {

    struct modbus_scheduler_t* sched = (struct modbus_scheduler_t*)_user_data;
    struct modbus_task_t* mt = sched->current_task;

    if(mt) {
        sched->modbus_sched_stop_wait(sched);
        sched->current_task = (struct modbus_task_t*)0;
        mt->state = mt_state_no_task;
        mt->on_response(mt, _slave_addr, _pkt);
    }
}

static void on_task(void *_data) {

    struct modbus_task_t* mt = (struct modbus_task_t*)_data;
    struct modbus_scheduler_t* sched = mt->sched;
    sched->current_task = mt;
    mt->state = mt_state_sending_req;
    emb_proto_send_packet(sched->proto, mt->slave_addr, mt->req);
    mt->state = mt_state_wait_resp;
    sched->modbus_sched_start_wait(sched, mt->resp_timeout);
}

void modbus_scheduler_init_task(struct modbus_task_t* _task, void *_user_data,
                                on_modbus_task_response_t _on_response,
                                on_modbus_task_timeout_t _on_timeout) {
    memset(_task, 0, sizeof(struct modbus_task_t));
    _task->on_response = _on_response;
    _task->on_resp_timeout = _on_timeout;
    _task->user_data = _user_data;
    _task->state = mt_state_no_task;
}

void modbus_scheduler_initialize(struct modbus_scheduler_t *_modbus_sched) {
    memset(&_modbus_sched->sched, 0, sizeof(struct scheduler_t));
    scheduler_init(&_modbus_sched->sched, _modbus_sched);
    _modbus_sched->proto->recv_packet = on_receive_pkt;
    _modbus_sched->proto->high_level_context = _modbus_sched;
    _modbus_sched->current_task = (struct modbus_task_t*)0;
}

int modbus_scheduler_add_task(struct modbus_scheduler_t* _modbus_sched,
                               struct modbus_task_t* _modbus_task) {

    if(_modbus_task->state != mt_state_no_task)
        return -EBUSY;

    _modbus_task->task.proc_data = _modbus_task;
    _modbus_task->task.task_proc = on_task;
    _modbus_task->sched = _modbus_sched;
    _modbus_task->state = mt_state_wait_processing;
    scheduler_add_task(&_modbus_sched->sched, &_modbus_task->task);

    return 0;
}

int modbus_scheduler_process_one_task(struct modbus_scheduler_t* _modbus_sched) {
    if(!_modbus_sched->current_task)
        scheduler_process_one_task(&_modbus_sched->sched);
    return !scheduler_queue_empty(&_modbus_sched->sched);
}

void modbus_scheduler_wait_timeout(struct modbus_scheduler_t* _modbus_sched) {
    struct modbus_task_t* mt = _modbus_sched->current_task;
    mt->state = mt_state_no_task;
    _modbus_sched->current_task = (struct modbus_task_t*)0;
    mt->on_resp_timeout(mt);
}

enum modbus_task_state_t modbus_scheduler_get_state(struct modbus_scheduler_t* _modbus_sched) {
    if(_modbus_sched->current_task)
        return _modbus_sched->current_task->state;
    else
        return mt_state_no_task;
}
