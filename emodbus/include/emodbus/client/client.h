
#ifndef MODBUS_CLIENT_H
#define MODBUS_CLIENT_H

#include <emodbus/base/modbus_transport.h>
#include <emodbus/client/client_base.h>
#include <stdint.h>

/*!
 * \file
 * \brief Declarations of modbus client (master) side.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

struct emb_client_transaction_t;
struct emb_client_t;

/**
 * @brief The emb_client_req_procs_t struct
 *
 * The callbacks for request.
 */
struct emb_client_req_procs_t {
    /// Calls when a correct response was received.
    void (*on_response)(struct emb_client_transaction_t*,
                        int _slave_addr);

    /// Calls when happens a some error, like: system error, receiving error(for example, CRC error),
    /// and modbus error(error, returned by server (slave) device)
    void (*on_error)(struct emb_client_transaction_t*,
                     int _slave_addr, int _errno);
};

/**
 * @brief The emb_client_request_t struct
 *
 * This is a ONE request descriptor for client.
 *
 */
struct emb_client_transaction_t {
    emb_const_pdu_t* req_pdu;               ///< Receive PDU
    emb_pdu_t* resp_pdu;                    ///< Response PDU
    struct emb_client_req_procs_t* procs;   ///< Callbacks for this request
    void* user_data;                        ///< Some user data for high-level :)
};

/**
 * @brief The emb_client_t struct
 *
 * This is a ONE modbus client cntext.
 *
 */
struct emb_client_t {

    /// Low level context
    struct emb_transport_t* transport;

    /// This variable saves a current-request address
    int curr_addr;

    /// This variable saves a pointer to a current request.
    struct emb_client_transaction_t* curr_transaction;

    /// This function calls by client. A call says about a response was received.
    void (*on_response)(struct emb_client_t* _cli, int _slave_addr);

    /// This function calls by client. A call indicates an error.
    void (*on_error)(struct emb_client_t* _cli, int _slave_addr, int _errno);
};

/**
 * @brief Initialization
 *
 * This function performs a initialization of a client.
 *
 * @param [in] _cli a pointer to client.
 */
void emb_client_init(struct emb_client_t* _cli);

/**
 * @brief Timeout signal
 *
 * This function must be called by high level context,
 * when a response-waiting time was expired.
 *
 * @param [in] _cli a pointer to client.
 */
void emb_client_wait_timeout(struct emb_client_t* _cli);

/**
 * @brief Do request
 *
 * This function performs a ONE request.
 *
 * @param [in] _cli a pointer to client.
 * @param [in] _slave_addr address of target server.
 * @param [in] _req request rescription.
 * @return if there is no errors, it will return zero, otherwize
 * it will return a error code. You can see it by emb_strerror() function.
 */
int emb_client_do_transaction(struct emb_client_t* _cli,
                              int _slave_addr,
                              struct emb_client_transaction_t* _transact);

/**
 * @brief Set low level
 *
 * This function set's a new low-level context for this client.
 *
 * @param _cli a pointer to client.
 * @param _transport a new transport.
 */
void emb_client_set_transport(struct emb_client_t* _cli,
                              struct emb_transport_t* _transport);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_CLIENT_H
