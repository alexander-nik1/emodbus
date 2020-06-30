
#ifndef EMB_CLIENT_H
#define EMB_CLIENT_H

#include <emodbus/base/modbus_xdu.h>

#include <emodbus/client/read_bits.h>
#include <emodbus/client/read_regs.h>

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
    emb_adu_t* req_adu;
    emb_adu_t* ans_adu;
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


int emb_sync_client_read_regs(emb_sync_client_t* _cli,
                              uint8_t _server_id,
                              enum EMB_RR_TYPE _rr_type,
                              uint16_t _start_address,
                              uint32_t _quantity,
                              uint16_t* _result);

int emb_sync_client_mask_reg(emb_sync_client_t* _cli,
                             uint8_t _server_id,
                             uint16_t _address,
                             uint16_t _and_mask,
                             uint16_t _or_mask);

int emb_sync_client_write_reg(emb_sync_client_t* _cli,
                              uint8_t _server_id,
                              uint16_t _address,
                              uint16_t _value);

int emb_sync_client_write_regs(emb_sync_client_t* _cli,
                               uint8_t _server_id,
                               uint16_t _start_address,
                               uint32_t _quantity,
                               const uint16_t* _values);

int emb_sync_client_rdwr_regs(emb_sync_client_t* _cli,
                              uint8_t _server_id,
                              uint16_t _wr_addr,
                              uint16_t _wr_quantity,
                              const uint16_t* _wr_values,
                              uint16_t _rd_addr,
                              uint16_t _rd_quantity,
                              uint16_t* _rd_values);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMB_CLIENT_H
