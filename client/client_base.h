
#ifndef EMODBUS_CLIENT_BASE_H
#define EMODBUS_CLIENT_BASE_H

#include "../base/modbus_pdu.h"

#ifdef __cplusplus
extern "C" {
#endif

enum { EMB_CLI_MAX_FUNCTIONS = 25 };

struct emb_client_function_i {
    int (*check_answer)(emb_const_pdu_t* _req,
                        emb_const_pdu_t* _ans);
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMODBUS_CLIENT_BASE_H
