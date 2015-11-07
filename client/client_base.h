
#ifndef EMODBUS_CLIENT_BASE_H
#define EMODBUS_CLIENT_BASE_H

#include "../base/modbus_pdu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A maximum function number for modbus-functions
 */
enum { EMB_CLI_MAX_FUNCTIONS = 25 };

/**
 * @brief The emb_client_function_i struct
 *
 * Interface of modbus-client function
 *
 */
struct emb_client_function_i {

    /// Number of modbus-function
    const int function_number;

    /// Validation of received packet with corresponding request.
    int (*check_answer)(emb_const_pdu_t* _req,
                        emb_const_pdu_t* _ans);
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMODBUS_CLIENT_BASE_H
