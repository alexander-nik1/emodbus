
#ifndef MODBUS_MASTER_READ_REGISTERS
#define MODBUS_MASTER_READ_REGISTERS

/*!
 * \file
 * \brief Read Holding Registers and Read Input Registers functions.
 *
 * Functions for working with Read Holding Registers
 * and Read Input Registers function.
 *
 */

#include <stdint.h>

#include <emodbus/base/modbus_pdu.h>
#include <emodbus/client/client_base.h>

#ifdef __cplusplus
extern "C" {
#endif

enum EMB_RR_TYPE {
    EMB_RR_HOLDINGS,
    EMB_RR_INPUTS,
};

/**
 * @brief Calculate the request size
 * @return The size of request's data size
 */
int emb_read_regs_calc_req_data_size();


/**
 * @brief Calculate the answer size
 * @param[in] _quantity The quantity of registers
 * @return The size of answer's data size
 */
int emb_read_regs_calc_answer_data_size(uint16_t _quantity);

/**
 * @brief Build request
 *
 * This fucntion builds the request.
 *
 * \param[out] _result_req Result request.
 * \param[in] _type The EMB_RR_HOLDINGS or EMB_RR_INPUTS, depent of
 * Read Holding Registers or Read Input Registers you want to build.
 * \param[in] _starting_address Starting address of register.
 * \param[in] _quantity Number of registers, that will be readed from device.
 * @return Zero if a request is ready, otherwise error code.
 */
int emb_read_regs_make_req(emb_pdu_t* _result_req,
                           enum EMB_RR_TYPE _type,
                           uint16_t _starting_address,
                           uint16_t _quantity);

/**
 * @brief Get starting address from request
 * @param[in] _req Request, from which reads a starting address
 * @return Starting address
 */
uint16_t emb_read_regs_get_starting_addr(emb_const_pdu_t* _req);

/**
 * @brief Get quantity from request
 * @param[in] _req Request, from which reads a quantity
 * @return Quantity
 */
uint16_t emb_read_regs_get_quantity(emb_const_pdu_t* _req);

/**
 * @brief Get register from answer
 *
 * Function returns a register from answer.
 *
 * @param[in] _ans Answer
 * @param[in] _reg_offset Offset of the register inside answer.
 * @return Register value.
 */
uint16_t emb_read_regs_get_reg(emb_const_pdu_t* _answer,
                                    uint16_t _reg_offset);

/**
 * @brief Get registers from answer
 *
 * Function returns a registers from answer.
 *
 * @param[in] _ans Answer
 * @param[in] _reg_offset Offset of the first register inside answer.
 * @param[in] _n_regs Number of registers to read
 * @param[in] _p_data Pointer to place, where data to be copied.
 */
void emb_read_regs_get_regs(emb_const_pdu_t* _answer,
                            uint16_t _reg_offset,
                            uint16_t _n_regs,
                            uint16_t* _p_data);

/**
 * @brief Get registers number from answer
 *
 * Function returns a registers number from answer
 *
 * @param[in] _ans Answer
 * @return Registers number.
 */
int emb_read_regs_get_regs_n(emb_const_pdu_t* _answer);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_READ_REGISTERS
