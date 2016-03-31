
#ifndef MODBUS_MASTER_WRITE_MULTIPLE_REGISTER
#define MODBUS_MASTER_WRITE_MULTIPLE_REGISTER

/*!
 * \file
 * \brief Declaration of Write Multiple Registers functions.
 *
 * Functions for working with Write Multiple Registers function.
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
int emb_write_regs_calc_req_data_size(uint16_t _quantity);


/**
 * @brief Calculate the answer size
 * @return The size of answer's data size
 */
int emb_write_regs_calc_answer_data_size();


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
int emb_write_regs_make_req(emb_pdu_t* _result_req,
                            uint16_t _address,
                            uint16_t _quantity,
                            const uint16_t* _data);

/**
 * @brief Get starting address
 * @param[in] _req Request, from which reads an address.
 * @return Starting address.
 */
uint16_t emb_write_regs_get_req_address(emb_const_pdu_t* _req);

/**
 * @brief Get quantity
 * @param[in] _req Request, from which reads the quantity.
 * @return The quantity.
 */
uint16_t emb_write_regs_get_req_quantity(emb_const_pdu_t* _req);

/**
 * @brief Get request's register data
 * @param[in] _req Request, from which reads the register data.
 * @param[in] _offset The offset within request to the needed register.
 * @return The register data.
 */
int emb_write_regs_get_req_data(emb_const_pdu_t* _req, uint16_t _offset);


/**
 * @brief Get starting address
 * @param[in] _answer THe answer, from which reads the address.
 * @return Starting address.
 */
uint16_t emb_write_regs_get_answer_address(emb_const_pdu_t* _answer);

/**
 * @brief Get quantityb
 * @param[in] _answer THe answer, from which reads the quantity.
 * @return Starting address.
 */
uint16_t emb_write_regs_get_answer_quantity(emb_const_pdu_t* _answer);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_WRITE_MULTIPLE_REGISTER
