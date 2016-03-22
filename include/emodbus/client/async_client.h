
#ifndef MODBUS_ASYNCHRONOUS_CLIENT_H
#define MODBUS_ASYNCHRONOUS_CLIENT_H

#include "../base/add/scheduler.h"
#include "../base/modbus_proto.h"
#include "client_base.h"
#include <stdint.h>

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

struct emb_task_t;
struct emb_async_client_t;

typedef void (*on_emb_task_response_t)(struct emb_task_t*,
                                       int _slave_addr,
                                       emb_const_pdu_t*);

typedef void (*on_emb_task_error_t)(struct emb_task_t*, int _errno);

enum emb_task_state_t {
    mt_state_no_task,
    mt_state_wait_processing,
    mt_state_sending_req,
    mt_state_wait_resp
};

struct emb_task_t {
    int slave_addr;
    emb_const_pdu_t* req;
    unsigned int resp_timeout;
    on_emb_task_response_t on_response;
    on_emb_task_error_t on_error;
    struct emb_async_client_t* sched;
    enum emb_task_state_t state;
    struct task_t task;
    void* user_data;
};

struct emb_async_client_t {

    const struct emb_client_function_i* functions[EMB_CLI_MAX_FUNCTIONS];

    void (*start_wait)(struct emb_async_client_t* _sched, unsigned int _time);
    void (*stop_wait)(struct emb_async_client_t* _sched);

    struct emb_protocol_t* proto;

    struct emb_task_t* current_task;
    struct scheduler_t sched;

    void* user_data;
};

void emb_init_task(struct emb_task_t* _task, void* _user_data,
                                on_emb_task_response_t _on_response,
                                on_emb_task_error_t _on_error);

void emb_async_client_init(struct emb_async_client_t* _modbus_sched);

int emb_async_client_add_task(struct emb_async_client_t* _modbus_sched, struct emb_task_t* _modbus_task);

int emb_async_client_process_one_task(struct emb_async_client_t* _modbus_sched);

void emb_async_client_wait_timeout(struct emb_async_client_t* _modbus_sched);

enum emb_task_state_t emb_async_client_get_state(struct emb_async_client_t* _modbus_sched);

/**
 * @brief Add a modbus-function to this client.
 *
 * This function adds a one function to this client.
 * After this call, an added modbus-function can
 * be used for send/receive packets.
 *
 * @param [in] _cli a synchronous client context.
 * @param [in] _func_i a fucntion interface.
 * @return Zero if success,
 *    the -EINVAL if _fucntion greater a EMB_CLI_MAX_FUNCTIONS number,
 *    the -EBUSY if this function number was already set.
 */
int emb_async_client_add_function(struct emb_async_client_t* _cli,
                            const struct emb_client_function_i* _func_i);

/**
 * @brief Removes a modbus-function from this client.
 *
 * After this call, a removed modbus-function can not
 * be used for send/receive packets.
 *
 * @param [in] _cli a synchronous client context.
 * @param [in] _fucntion the function number to be removed.
 * @return Zero if success,
 *    the -EINVAL if _fucntion greater a EMB_CLI_MAX_FUNCTIONS number,
 *    the -ENXIO if this function number was already removed or was not set.
 */
int emb_async_client_remove_function(struct emb_async_client_t* _cli,
                               uint8_t _fucntion);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_ASYNCHRONOUS_CLIENT_H
