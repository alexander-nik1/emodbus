
#ifndef EMODBUS_SYNC_CLIENT_H
#define EMODBUS_SYNC_CLIENT_H

/*!
 * \file
 * \brief Sunchronous client declarations.
 *
 * This file contains a context description and
 * method declarations for synchronous client.
 *
 */

#include <stdint.h>
#include "client_base.h"
#include "../base/modbus_pdu.h"
#include "../base/modbus_proto.h"
#include "../base/compat.h"
#include "../base/common.h"

#ifdef __cplusplus
extern "C" {
#endif

enum emb_cli_state_t {
    emb_cli_state_default,
    emb_cli_state_req_sending,
    emb_cli_state_wait_resp
};

enum emb_cli_resp_state_t {
    emb_cli_resp_state_no_resp,
    emb_cli_resp_state_resp_ok,
    emb_cli_resp_state_resp_fail
};

/**
 * @brief Synchronous client
 *
 * This structure represents an one client,
 * that can poll one device per one time.
 * Here you have the function 'do_request',
 * that calls by application for send one request
 * and receiving response or error.
 */

struct emb_sync_client_t {

    /**
     * @brief Protocol
     *
     * This is a pointer to low-level realisation of protocol
     */
    struct emb_protocol_t* protocol;

    /**
     * @brief A set of functions for this client.
     *
     * This array contains a pointers to functions, that will be used by this client.
     * Index in this array means a modbus-function number. If functions is not used
     * by this client, then pointer in this array is NULL.
     */
    const struct emb_client_function_i* functions[EMB_CLI_MAX_FUNCTIONS];

    /// Internally used. A current request
    emb_const_pdu_t* current_request;

    /// Internally used. A current server address
    int curr_req_server_addr;

    /// Internally used. A current response
    emb_const_pdu_t* current_response;

    /// Internally used. An error code
    int error_code;

    /// Internally used. The transaction phase
    enum emb_cli_state_t state;

    /// Internally used. Response state
    enum emb_cli_resp_state_t resp_state;

    /** Internally used.
     * Mutex, whereby 'do_request' held pending a response or timeout.
     */
    struct emb_timed_mutex_i resp_timeout_mutex;

    /// User context
    void *user_data;
};

/**
 * @brief Initialization of synchronous client
 *
 * This function must be called before all other functions.
 *
 * @param[in] _cli a synchronous client context.
 */
void emb_sync_client_initialize(struct emb_sync_client_t* _cli);

/**
 * @brief Set protocol.
 *
 * This function must be called after a emb_sync_client_initialize
 * call. This function sets a emb_sync_client_t::protocol variable.
 * And connects protocol's high-level to this client.
 *
 * @param [in] _cli a synchronous client context.
 * @param [in] _proto the protocol interface.
 */
void emb_sync_client_set_proto(struct emb_sync_client_t* _cli,
                          struct emb_protocol_t* _proto);

/**
 * @brief Add a modbus-function to this client.
 *
 * This function adds a one function to this client.
 * After this call, an added modbus-function can
 * be used for send/receive packets.
 *
 * @param [in] _cli a synchronous client context.
 * @param [in] _fucntion the function number.
 * @param [in] _func_i a fucntion interface.
 * @return Zero if success,
 *    the -EINVAL if _fucntion greater a EMB_CLI_MAX_FUNCTIONS number,
 *    the -EBUSY if this function number was already set.
 */
int emb_sync_client_add_function(struct emb_sync_client_t* _cli,
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
int emb_sync_client_remove_function(struct emb_sync_client_t* _cli,
                               uint8_t _fucntion);

/**
 * @brief Do modbus request.
 *
 * This function most important for sunchronous client.
 * This function perfroms one modbus-transaction.
 * Function returns by next events: by received response, by received error,
 * and by timeout.
 *
 * @param [in] _cli a synchronous client context.
 * @param _server_addr a server address to which will send request.
 * @param _timeout timeout of response waiting
 * (this value will sent to the timeout mutex)
 * @param _request the request (must be built before this call)
 * @param _response in this variable will be stored a pointer to response
 * (if transaction successfully finished)
 * @return Zero if success,
 *    the -EBUSY if previous transaction is not finished yet.
 *    the -ENXIO if this function number was already removed or was not set.
 */
int emb_sync_client_do_request(struct emb_sync_client_t* _cli,
                          int _server_addr,
                          unsigned int _timeout,
                          emb_const_pdu_t* _request,
                          emb_const_pdu_t** _response);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMODBUS_SYNC_CLIENT_H
