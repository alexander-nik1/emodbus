
#include "scheduler.h"

#define LIST_RW_LOCK(_sched_)       \
    if((_sched_)->list_rw_lock)     \
        (_sched_)->list_rw_lock((_sched_)->user_data);

#define LIST_RW_UNLOCK(_sched_)     \
    if((_sched_)->list_rw_unlock)   \
        (_sched_)->list_rw_unlock((_sched_)->user_data);

#define LIST_EMPTY_LOCK(_sched_)    \
    if((_sched_)->list_empty_lock)  \
        (_sched_)->list_empty_lock((_sched_)->user_data);

#define LIST_EMPTY_UNLOCK(_sched_)      \
    if((_sched_)->list_empty_unlock)    \
        (_sched_)->list_empty_unlock((_sched_)->user_data);

void scheduler_init(struct scheduler_t* _sched, void* _user_data) {

    LIST_HEAD_INIT2(_sched->tasks_list);
    _sched->user_data = _user_data;
}

int scheduler_queue_empty(struct scheduler_t* _sched) {
    return list_empty(&_sched->tasks_list);
}

void scheduler_process_one_task(struct scheduler_t* _sched) {

    struct task_t* task;

    if(_sched->list_empty_lock) {
        while(list_empty(&_sched->tasks_list)) {
            _sched->list_empty_lock(_sched->user_data);
        }
    }
    else {
        if(list_empty(&_sched->tasks_list)) {
            return;
        }
    }

    task = list_entry(_sched->tasks_list.prev, struct task_t, list);

    LIST_RW_LOCK(_sched);
    list_del(&task->list);
    LIST_RW_UNLOCK(_sched);

    if(task->task_proc) {
        task->task_proc(task->proc_data);
    }
}

void scheduler_add_task(struct scheduler_t* _sched, struct task_t* _task) {
    LIST_RW_LOCK(_sched);
    list_add(&_task->list, &_sched->tasks_list);
    LIST_RW_UNLOCK(_sched);
    LIST_EMPTY_UNLOCK(_sched);
}

struct task_t* scheduler_set_task_parameters(struct task_t* _task,
                               task_proc_t _task_proc,
                               void * _proc_data) {
    _task->task_proc = _task_proc;
    _task->proc_data = _proc_data;
    return _task;
}
