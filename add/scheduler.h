
#ifndef THE_MY_OWN_SCHEDULER_H
#define THE_MY_OWN_SCHEDULER_H

#include "list.h"

// Планировщик
// Задачи сохраняются в виде связанного сипска (struct list_head),
// и исполняются в виде очереди, по принципу первым пришёл, первым вышел.
// Задачи после выполнения удаляются из очереди

#ifdef __cplusplus
extern "C" {
#endif

// Тип вызываемой планировщиком функции
typedef void (*task_proc_t)(void *_data);

// Эта структура описывает одну задачу
struct task_t {
    // функция, которая будет вызвана, когда до этой задачи дойдёт очередь
    task_proc_t task_proc;

    // параметр функции task_proc
    void* proc_data;

    // Элемент двусвязного списка
    struct list_head list;
};

// Структура описывает один планировщик
// каждый планировщик имеет свой список задач
struct scheduler_t {

    // функции, обеспечивающие безопасность при вставлении/удалении
    // в очередь. (Если не используются, то можно ставить в 0)
    void (*list_rw_lock)(void* _user_data);
    void (*list_rw_unlock)(void* _user_data);

    // функции для блокировки/разблокировки потока, который
    // выталкивает задачи из очереди и исполняет их
    // (Если не используются, то можно ставить в 0)
    void (*list_empty_lock)(void* _user_data);
    void (*list_empty_unlock)(void* _user_data);

    // параметр для вышеуказанных функций
    void* user_data;

    // Якорь для очереди задач
    struct list_head tasks_list;
};

// Инициализирует планировщик
void scheduler_init(struct scheduler_t* _sched, void *_user_data);

// Вернёт 0 если очередь не пуста
int scheduler_queue_empty(struct scheduler_t* _sched);

// Вытащить из очереди следующую задачу, и выполнить её
void scheduler_process_one_task(struct scheduler_t* _sched);

// Добавить в очередь следующую задачу
void scheduler_add_task(struct scheduler_t* _sched, struct task_t* _task);

// Установить параметры задачи одной строкой
struct task_t* scheduler_set_task_parameters(struct task_t* _task,
                               task_proc_t _task_proc,
                               void * _proc_data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // THE_MY_OWN_SCHEDULER_H
