
#include "async_client.h"
#include "string.h"
#include <errno.h>
#include "modbus_errno.h"

#include <stdio.h>

static void emb_async_client_on_receive_pkt(void* _user_data,
                           int _slave_addr,
                           emb_const_pdu_t* _pkt) {

    struct emb_async_client_t* cli = (struct emb_async_client_t*)_user_data;
    struct emb_task_t* mt = cli->current_task;
    const struct emb_client_function_i* func;
    int res;


    cli->stop_wait(cli);
    cli->current_task = (struct emb_task_t*)0;
    mt->state = mt_state_no_task;

    do {
        if(!mt) {
            mt->on_error(mt, -modbus_resp_without_req);
            break;
        }

        if(mt->slave_addr != _slave_addr) {
            mt->on_error(mt, -modbus_resp_wrong_address);
            break;
        }

        func = cli->functions[mt->req->function];

        if(!func) {
            mt->on_error(mt, -modbus_no_such_function);
            break;
        }

        if((res = func->check_answer(mt->req, _pkt))) {
            mt->on_error(mt, res);
            break;
        }

        mt->on_response(mt, _slave_addr, _pkt);
    }
    while(0);
}

static void emb_async_client_on_error(void* _user_data, int _errno) {
    struct emb_async_client_t* sched = (struct emb_async_client_t*)_user_data;
    struct emb_task_t* mt = sched->current_task;
    if(mt) {
        sched->stop_wait(sched);
        sched->current_task = (struct emb_task_t*)0;
        mt->state = mt_state_no_task;
        mt->on_error(mt, _errno);
    }
}

static void emb_async_client_on_task(void *_data) {

    struct emb_task_t* mt = (struct emb_task_t*)_data;
    struct emb_async_client_t* sched = mt->sched;
    sched->current_task = mt;
    mt->state = mt_state_sending_req;
    emb_proto_send_packet(sched->proto, mt->slave_addr, mt->req);
    mt->state = mt_state_wait_resp;
    sched->start_wait(sched, mt->resp_timeout);
}

void emb_init_task(struct emb_task_t* _task, void *_user_data,
                                on_emb_task_response_t _on_response,
                                on_emb_task_error_t _on_error) {
    memset(_task, 0, sizeof(struct emb_task_t));
    _task->on_response = _on_response;
    _task->on_error = _on_error;
    _task->user_data = _user_data;
    _task->state = mt_state_no_task;
}

void emb_async_client_init(struct emb_async_client_t *_modbus_sched) {
    memset(&_modbus_sched->sched, 0, sizeof(struct scheduler_t));
    scheduler_init(&_modbus_sched->sched, _modbus_sched);
    _modbus_sched->proto->recv_packet = emb_async_client_on_receive_pkt;
    _modbus_sched->proto->error = emb_async_client_on_error;
    _modbus_sched->proto->high_level_context = _modbus_sched;
    _modbus_sched->current_task = (struct emb_task_t*)0;
}

int emb_async_client_add_task(struct emb_async_client_t* _modbus_sched,
                               struct emb_task_t* _modbus_task) {

    if(_modbus_task->state != mt_state_no_task)
        return -EBUSY;

    _modbus_task->task.proc_data = _modbus_task;
    _modbus_task->task.task_proc = emb_async_client_on_task;
    _modbus_task->sched = _modbus_sched;
    _modbus_task->state = mt_state_wait_processing;
    scheduler_add_task(&_modbus_sched->sched, &_modbus_task->task);

    return 0;
}

int emb_async_client_process_one_task(struct emb_async_client_t* _modbus_sched) {
    if(!_modbus_sched->current_task)
        scheduler_process_one_task(&_modbus_sched->sched);
    return !scheduler_queue_empty(&_modbus_sched->sched);
}

void emb_async_client_wait_timeout(struct emb_async_client_t* _modbus_sched) {
    struct emb_task_t* mt = _modbus_sched->current_task;
    mt->state = mt_state_no_task;
    _modbus_sched->current_task = (struct emb_task_t*)0;
    mt->on_error(mt, -modbus_resp_timeout);
}

enum emb_task_state_t emb_async_client_get_state(struct emb_async_client_t* _modbus_sched) {
    if(_modbus_sched->current_task)
        return _modbus_sched->current_task->state;
    else
        return mt_state_no_task;
}

int emb_async_client_add_function(struct emb_async_client_t *_cli,
                            const struct emb_client_function_i* _func_i) {
    const int function = _func_i->function_number;
    if(function >= EMB_CLI_MAX_FUNCTIONS)
        return -EINVAL;
    if(_cli->functions[function])
        return -EBUSY;
    _cli->functions[function] = _func_i;
    return 0;
}

int emb_async_client_remove_function(struct emb_async_client_t* _cli,
                                    uint8_t _fucntion) {
    if(_fucntion >= EMB_CLI_MAX_FUNCTIONS)
        return -EINVAL;
    if(!_cli->functions[_fucntion])
        return -ENXIO;
    _cli->functions[_fucntion] = NULL;
    return 0;
}
