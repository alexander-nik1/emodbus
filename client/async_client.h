
#ifndef EMODBUS_ASYNC_CLIENT_H
#define EMODBUS_ASYNC_CLIENT_H

#include <stdint.h>
#include "client_base.h"
#include "modbus_sched.h"
#include "../base/modbus_pdu.h"
#include "../base/modbus_proto.h"
#include "../base/compat.h"
#include "../base/common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct emb_async_client_t {

    struct emb_protocol_t* protocol;

    const struct emb_client_function_i* functions[EMB_CLI_MAX_FUNCTIONS];

    emb_const_pdu_t* current_request;

    void *user_data;
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMODBUS_ASYNC_CLIENT_H
