
#ifndef MODBUS_MASTER_WRITE_SINGLE_REGISTER
#define MODBUS_MASTER_WRITE_SINGLE_REGISTER

#include <stdint.h>

/*!
 * \file
 * \brief The dclaration of a "Write Single Register" functions.
 *
 * Functions for working with "Write Single Register" function.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate the request size
 * @return The size of request's data size
 */
int emb_write_reg_calc_req_data_size();

/**
 * @brief Calculate the answer size
 * @return The size of answer's data size
 */
int emb_write_reg_calc_answer_data_size();

/**
 * @brief Build request
 *
 * This fucntion builds the "Write Single Register" request.
 *
 * \param[out] _result_req Result request.
 * \param[in] _address Address of a holding register.
 * \param[in] _value A value to be written to a register.
 * @return Zero if a request is ready, otherwise error code.
 */
int emb_write_reg_make_req(emb_pdu_t* _result_req,
                           uint16_t _address,
                           uint16_t _value);

/**
 * @brief Get address
 *
 * This function returns address, which got from _pdu parameter.
 * Because, an answer is a echo of the request, _pdu can be any:
 * request or answer.
 *
 * @param[in] _pdu Can be an answer or a request for this modbus-function.
 * @return Address of a holding register.
 */
uint16_t emb_write_reg_get_address(emb_const_pdu_t* _pdu);

/**
 * @brief Get value
 *
 * This function returns value, which got from _pdu parameter.
 * Because, an answer is a echo of the request, _pdu can be any:
 * request or answer.
 *
 * @param[in] _pdu Can be an answer or a request for this modbus-function.
 * @return Value for a holding register.
 */
uint16_t emb_write_reg_get_value(emb_const_pdu_t* _pdu);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_WRITE_SINGLE_REGISTER
