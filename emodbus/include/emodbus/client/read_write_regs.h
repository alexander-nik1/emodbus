
#ifndef MODBUS_MASTER_READ_WRITE_REGISTERS
#define MODBUS_MASTER_READ_WRITE_REGISTERS

/*!
 * \file
 * \brief Declaration of Read Write Registers functions.
 *
 * Functions for working with Read Write Registers function.
 *
 */

#include <stdint.h>
#include <emodbus/base/modbus_pdu.h>
#include <emodbus/client/client_base.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate the request size
 * @param _quantity The quantity of registers
 * @return The size of request's data size
 */
int emb_rdwr_regs_calc_req_data_size(uint16_t _wr_quantity);


/**
 * @brief Calculate the answer size
 * @return The size of answer's data size
 */
int emb_rdwr_regs_calc_answer_data_size(uint16_t _rd_quantity);


/**
 * @brief Build request
 *
 * This fucntion builds the "Read holding registers" request.
 *
 * \param[out] _result_req Result request.
 * \param[in] _address Starting address for writing to.
 * \param[in] _quantity Number of registers, that will be written.
 * \param[in] _data The data, that will be written.
 * @return Zero if a request is ready, otherwise error code.
 */
int emb_rdwr_regs_make_req(emb_pdu_t* _result_req,
                           uint16_t _rd_address,
                           uint16_t _rd_quantity,
                           uint16_t _wr_address,
                           uint16_t _wr_quantity,
                           const uint16_t* _wr_data);

/**
 * @brief Get read address from request
 * @param[in] _req Request, from which reads an address.
 * @return Starting address.
 */
uint16_t emb_rdwr_regs_get_req_rd_address(emb_const_pdu_t* _req);

/**
 * @brief Get read quantity from request
 * @param[in] _req Request, from which reads the quantity.
 * @return The quantity.
 */
uint16_t emb_rdwr_regs_get_req_rd_quantity(emb_const_pdu_t* _req);

/**
 * @brief Get write address from request
 * @param[in] _req Request, from which reads an address.
 * @return Starting address.
 */
uint16_t emb_rdwr_regs_get_req_wr_address(emb_const_pdu_t* _req);

/**
 * @brief Get write quantity from request
 * @param[in] _req Request, from which reads the quantity.
 * @return The quantity.
 */
uint16_t emb_rdwr_regs_get_req_wr_quantity(emb_const_pdu_t* _req);


/**
 * @brief Get answer's readed data (one register)
 * @param[in] _req Answer, from which reads the register data.
 * @param[in] _offset The offset within answer to the needed register.
 * @return The readed data.
 */
uint16_t emb_rdwr_regs_get_answ_reg(emb_const_pdu_t* _answer, uint16_t _offset);


/**
 * @brief Get answer's readed data (multiple registers)
 * @param[in] _req Answer, from which reads the data.
 * @param[in] _offset The offset within answer to the needed registers.
 * @param[in] _n_regs The number of registers to be read
 * @param[in] A pointer to the buffer for writing the data.
 * @return The readed data.
 */
int emb_rdwr_regs_get_answ_regs(emb_const_pdu_t* _answer, uint16_t _offset,
                                uint16_t _n_regs, uint16_t* _p_data);

/**
 * @brief Get answer's number of readed registers
 * @param[in] _req Answer.
 * @return The number of previously readed registers.
 */
int emb_rdwr_regs_get_answ_regs_n(emb_const_pdu_t* _answer);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_READ_WRITE_REGISTERS
