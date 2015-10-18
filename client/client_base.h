
#ifndef EMODBUS_CLIENT_BASE_H
#define EMODBUS_CLIENT_BASE_H

#include <stdint.h>
#include "../base/modbus_pdu.h"
#include "../base/modbus_proto.h"
#include "modbus_sched.h"
#include "../base/common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct emb_client_function_i {
    int (*check_answer)(const struct modbus_const_pdu_t* _req,
                        const struct modbus_const_pdu_t* _ans);
};

enum { EMB_CLI_MAX_FUNCTIONS = 25 };

struct emb_client_t {

    struct modbus_protocol_t* protocol;

    const struct emb_client_function_i* functions[EMB_CLI_MAX_FUNCTIONS];

    const struct modbus_const_pdu_t* current_request;

    int curr_req_server_addr;

    const struct modbus_const_pdu_t* current_response;

    int error_code;

    int state;

    struct emb_timed_mutex_i resp_timeout_mutex;

    void *user_data;
};

void emb_client_initialize(struct emb_client_t* _cli);

int emb_client_add_function(struct emb_client_t* _cli,
                            uint8_t _fucntion,
                            const struct emb_client_function_i* _func_i);

int emb_client_remove_function(struct emb_client_t* _cli,
                               uint8_t _fucntion);

int emb_client_do_request(struct emb_client_t* _cli,
                          int _server_addr,
                          unsigned int _timeout,
                          const struct modbus_const_pdu_t* _request,
                          const struct modbus_const_pdu_t** _response);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMODBUS_CLIENT_BASE_H
