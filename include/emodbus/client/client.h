
#ifndef MODBUS_CLIENT_H
#define MODBUS_CLIENT_H

#include <emodbus/base/modbus_proto.h>
#include <emodbus/client/client_base.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct emb_client_request_t;
struct emb_client_t;

enum emb_client_state_t {
    mt_state_no_task,
    mt_state_sending_req,
    mt_state_wait_resp
};

struct emb_client_req_procs_t {
    void (*on_response)(struct emb_client_request_t*,
                        int _slave_addr);
    void (*on_error)(struct emb_client_request_t*,
                     int _errno);
};

struct emb_client_request_t {
    emb_const_pdu_t* req_pdu;
    emb_pdu_t* resp_pdu;
    unsigned int resp_timeout;
    struct emb_client_req_procs_t* procs;
    void* user_data;
};

struct emb_client_t {

    const struct emb_client_function_i* functions[EMB_CLI_MAX_FUNCTIONS];

    void (*start_wait)(struct emb_client_t* _cli, unsigned int _time);
    void (*stop_wait)(struct emb_client_t* _cli);

    struct emb_protocol_t* proto;

    enum emb_client_state_t state;

    void* user_data;

    int curr_addr;
    struct emb_client_request_t* curr_req;
};

void emb_client_init(struct emb_client_t* _cli);

void emb_client_wait_timeout(struct emb_client_t* _cli);

int emb_client_do_request(struct emb_client_t* _cli,
                          int _slave_addr,
                          struct emb_client_request_t* _req);

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
int emb_client_add_function(struct emb_client_t* _cli,
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
int emb_client_remove_function(struct emb_client_t* _cli,
                               uint8_t _fucntion);


#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_CLIENT_H
