
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

/**
 * @brief emb_sync_client_read_regs
 *
 * Synchronous reading bits (discrete inputs or coils)
 *
 * @param [in] _cli Client context
 * @param [in] _server_id Server id
 * @param [in] _rb_type What to read: EMB_RB_COILS or EMB_RB_DISCRETE_INPUTS
 * @param [in] _start_address Begin address of reading
 * @param [in] _quantity Number of bits to read
 * @param [out] _result Place to store readed bits
 * @return if there is no errors, it will return zero, otherwize
 * it will return a error code. You can see it by emb_strerror() function.
 */
int emb_sync_client_read_bits(emb_sync_client_t* _cli,
							  uint8_t _server_id,
							  enum EMB_RB_TYPE _rb_type,
							  uint16_t _start_address,
							  uint32_t _quantity,
							  uint8_t* _result);


/**
 * @brief emb_sync_client_read_regs
 *
 * Synchronous reading registers
 *
 * @param [in] _cli Client context
 * @param [in] _server_id Server id
 * @param [in] _rr_type What to read: EMB_RR_HOLDINGS or EMB_RR_INPUTS
 * @param [in] _start_address Begin address of reading
 * @param [in] _quantity Number of registers to read
 * @param [out] _result Place to store readed registers
 * @return if there is no errors, it will return zero, otherwize
 * it will return a error code. You can see it by emb_strerror() function.
 */
int emb_sync_client_read_regs(emb_sync_client_t* _cli,
                              uint8_t _server_id,
                              enum EMB_RR_TYPE _rr_type,
                              uint16_t _start_address,
                              uint32_t _quantity,
                              uint16_t* _result);

/**
 * @brief emb_sync_client_mask_reg
 *
 * Synchronous mask register
 *
 * @param [in] _cli Client context
 * @param [in] _server_id Server id
 * @param [in] _address Address of register
 * @param [in] _and_mask AND mask
 * @param [in] _or_mask OR mask
 * @return if there is no errors, it will return zero, otherwize
 * it will return a error code. You can see it by emb_strerror() function.
 */
int emb_sync_client_mask_reg(emb_sync_client_t* _cli,
                             uint8_t _server_id,
                             uint16_t _address,
                             uint16_t _and_mask,
                             uint16_t _or_mask);

/**
 * @brief emb_sync_client_write_reg
 *
 * Synchronous write single register
 *
 * @param [in] _cli Client context
 * @param [in] _server_id Server id
 * @param [in] _address Address of register to write
 * @param [in] _value Value to wrote
 * @return if there is no errors, it will return zero, otherwize
 * it will return a error code. You can see it by emb_strerror() function.
 */
int emb_sync_client_write_reg(emb_sync_client_t* _cli,
                              uint8_t _server_id,
                              uint16_t _address,
                              uint16_t _value);

/**
 * @brief emb_sync_client_write_regs
 *
 * Synchronous write mulptple registers
 *
 * @param [in] _cli Client context
 * @param [in] _server_id Server id
 * @param [in] _start_address Begining address to write
 * @param [in] _quantity Number of registers to write
 * @param [in] _values Values to write
 * @return if there is no errors, it will return zero, otherwize
 * it will return a error code. You can see it by emb_strerror() function.
 */
int emb_sync_client_write_regs(emb_sync_client_t* _cli,
                               uint8_t _server_id,
                               uint16_t _start_address,
                               uint32_t _quantity,
                               const uint16_t* _values);

/**
 * @brief emb_sync_client_rdwr_regs
 *
 * Synchronous write and read multiple registers.
 *
 * @param [in] _cli Client context
 * @param [in] _server_id Server id
 * @param [in] _wr_addr Begining address to write
 * @param [in] _wr_quantity Numbers of registers to write
 * @param [in] _wr_values Values to write
 * @param [in] _rd_addr Begining address to read
 * @param [in] _rd_quantity Nuumber of registers to read
 * @param [out] _rd_values Place to store readed registers
 * @return if there is no errors, it will return zero, otherwize
 * it will return a error code. You can see it by emb_strerror() function.
 */
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
