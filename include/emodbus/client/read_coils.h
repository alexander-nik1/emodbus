
#ifndef MODBUS_MASTER_READ_COILS
#define MODBUS_MASTER_READ_COILS

/*!
 * \file
 * \brief Read Coils functions.
 *
 * Functions for working with Read Coilss function.
 *
 */

#include <stdint.h>

#include <emodbus/base/modbus_pdu.h>
#include <emodbus/client/client_base.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { EMB_READ_COILS_MAX_QUANTITY = 0x07D0 };

/**
 * @brief Calculate the request size
 * @return The size of request's data size
 */
int emb_read_coils_calc_req_data_size();


/**
 * @brief Calculate the answer size
 * @param[in] _quantity The quantity of coils
 * @return The size of answer's data size
 */
int emb_read_coils_calc_answer_data_size(uint16_t _quantity);

/**
 * @brief Build request
 *
 * This fucntion builds the "Read Coils" request.
 *
 * \param[out] _result_req Result request.
 * \param[in] _starting_address Starting address of coils.
 * \param[in] _quantity Number of coils, that will be readed from device.
 * @return Zero if a request is ready, otherwise error code.
 */
int emb_read_coils_make_req(emb_pdu_t* _result_req,
                            uint16_t _starting_address, uint16_t _quantity);

/**
 * @brief Get starting address from request
 * @param[in] _req Request, from which reads a starting address
 * @return Starting address
 */
uint16_t emb_read_coils_get_starting_addr(emb_const_pdu_t* _req);

/**
 * @brief Get quantity from request
 * @param[in] _req Request, from which reads a quantity
 * @return Quantity
 */
uint16_t emb_read_coils_get_quantity(emb_const_pdu_t* _req);

/**
 * @brief Get coil from answer
 *
 * Function returns a coil value from answer.
 *
 * @param[in] _ans Answer
 * @param[in] _coil_offset Offset of the coil inside answer.
 * @return Coil value.
 */
char emb_read_coils_get_coil(emb_const_pdu_t* _answer,
                             uint16_t _coil_offset);

/**
 * @brief Get eight coils from answer.
 *
 * Function returns a eight coils values from answer.
 *
 * @param[in] _ans Answer
 * @param[in] _byte_offset Offset to the byte in the answer.
 * @return An eight coils values. (One byte from answer)
 */
uint8_t emb_read_coils_get_byte(emb_const_pdu_t* _answer,
                                uint8_t _byte_offset);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_READ_COILS
