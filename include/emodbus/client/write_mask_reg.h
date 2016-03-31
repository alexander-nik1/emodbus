
#ifndef MODBUS_MASTER_WRITE_MASK_REGISTER
#define MODBUS_MASTER_WRITE_MASK_REGISTER

#include <stdint.h>

/*!
 * \file
 * \brief The dclaration of "Mask Write Register" functions.
 *
 * Functions for working with "Mask Write Register" function.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate the request size
 * @return The size of request's data size
 */
int emb_write_mask_reg_calc_req_data_size();

/**
 * @brief Calculate the answer size
 * @return The size of answer's data size
 */
int emb_write_mask_reg_calc_answer_data_size();

/**
 * @brief Build request
 *
 * This fucntion builds the "Mask Write Register" request.
 *
 * \param[out] _result_req Result request.
 * \param[in] _result_req Starting address of holding register.
 * \param[in] _and_mask AND mask
 * \param[in] _or_mask OR mask
 * @return Zero if a request is ready, otherwise error code.
 */
int emb_write_mask_reg_make_req(emb_pdu_t* _result_req,
                                uint16_t _address,
                                uint16_t _and_mask,
                                uint16_t _or_mask);

/**
 * @brief Get address
 *
 * This function returns address, which got from _pdu parameter.
 * Because, an answer is a echo of the request, _pdu can be any:
 * request or answer.
 *
 * @param[in] _pdu Can be an answer or a request for this modbus-function.
 * @return Starting address of holding register.
 */
uint16_t emb_write_mask_reg_get_address(emb_const_pdu_t* _pdu);

/**
 * @brief Get AND mask
 *
 * This function returns AND amsk, which got from _pdu parameter.
 * Because, an answer is a echo of the request, _pdu can be any:
 * request or answer.
 *
 * @param[in] _pdu Can be an answer or a request for this modbus-function.
 * @return AND mask
 */
uint16_t emb_write_mask_reg_get_and_mask(emb_const_pdu_t* _pdu);

/**
 * @brief Get OR mask
 *
 * This function returns OR amsk, which got from _pdu parameter.
 * Because, an answer is a echo of the request, _pdu can be any:
 * request or answer.
 *
 * @param[in] _pdu Can be an answer or a request for this modbus-function.
 * @return OR mask
 */
uint16_t emb_write_mask_reg_get_or_mask(emb_const_pdu_t* _pdu);


#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_WRITE_MASK_REGISTER
