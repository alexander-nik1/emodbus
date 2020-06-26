
#ifndef EMB_CLIENT_H
#define EMB_CLIENT_H

#include <emodbus/base/modbus_xdu.h>
#include <stdint.h>

/*!
 * \file
 * \brief Declarations of modbus client (master) side.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The emb_client_t struct
 *
 * This is a ONE modbus client cntext.
 *
 */
typedef struct __emb_sync_client_t
{
    int (*send_adu)(struct __emb_sync_client_t* _cli, const emb_adu_t* _adu);
    int (*recv_adu)(struct __emb_sync_client_t* _cli, emb_adu_t* _adu);
} emb_sync_client_t;

/**
 * @brief Do sync request
 *
 * This function performs a ONE request.
 *
 * @param [in] _cli a pointer to client.
 * @param [in] _req_adu request ADU.
 * @param [out] _ans_req response ADU.
 * @return if there is no errors, it will return zero, otherwize
 * it will return a error code. You can see it by emb_strerror() function.
 */
int emb_sync_client_do_request(emb_sync_client_t* _cli, const emb_adu_t* _req_adu, emb_adu_t* _ans_adu);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMB_CLIENT_H
