
#ifndef MODBUS_MASTER_READ_HOLDING_REGISTERS
#define MODBUS_MASTER_READ_HOLDING_REGISTERS

/*!
 * \file
 * \brief Read Holding Registers functions.
 *
 * Functions for working with Read Holding Registers function.
 *
 */

#include <stdint.h>

#include "../base/modbus_pdu.h"
#include "client_base.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct emb_client_function_i read_holding_regs_interface;

/**
 * @brief Build request
 *
 * This fucntion builds the "Read holding registers" request.
 *
 * \param[out] _result_req Result request.
 * \param[in] _starting_address Starting address of holding register.
 * \param[in] _quantity Number of registers, that will be readed from device.
 */
int read_holding_regs_make_req(struct modbus_pdu_t* _result_req,
                               uint16_t _starting_address, uint16_t _quantity);

/**
 * @brief Get starting address from request
 * @param[in] _req Request, from which reads a starting address
 * @return Starting address
 */
uint16_t read_holding_regs_get_starting_addr(const struct modbus_const_pdu_t* _req);

/**
 * @brief Get quantity from request
 * @param[in] _req Request, from which reads a quantity
 * @return Quantity
 */
uint16_t read_holding_regs_get_quantity(const struct modbus_const_pdu_t* _req);

/**
 * @brief Check and valid answer
 *
 * This function compares request PDU and answer PDU, and returns
 * zero if an answer was a valid.
 *
 * @param[in] _req Request
 * @param[in] _ans Answer
 * @return Zero if answer is correct for this request, or error code if not.
 */
int read_holding_regs_valid_answer(const struct modbus_const_pdu_t* _req,
                                   const struct modbus_const_pdu_t* _ans);

/**
 * @brief Get register from answer
 *
 * Function returns a register from answer.
 *
 * @param[in] _ans Answer
 * @param[in] _reg_offset Offset of the register inside answer.
 * @return Register value.
 */
uint16_t read_holding_regs_get_reg(const struct modbus_const_pdu_t* _ans,
                                   uint16_t _reg_offset);

/**
 * @brief Get registers number from answer
 *
 * Function returns a registers number from answer
 *
 * @param[in] _ans Answer
 * @return Registers number.
 */
int read_holding_regs_get_regs_n(const struct modbus_const_pdu_t* _ans);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_READ_HOLDING_REGISTERS
