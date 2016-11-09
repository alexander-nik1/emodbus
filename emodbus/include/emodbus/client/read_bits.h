
#ifndef MODBUS_MASTER_READ_BITS
#define MODBUS_MASTER_READ_BITS

/*!
 * \file
 * \brief Read Coils and Read Discrete Inputs functions.
 *
 * Functions for working with Read Coilss and Read Discrete Inputs function.
 *
 */

#include <stdint.h>

#include <emodbus/base/modbus_pdu.h>
#include <emodbus/client/client_base.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { EMB_READ_BITS_MAX_QUANTITY = 0x07D0 };

/**
 * @brief Calculate the request size
 * @return The size of request's data size
 */
int emb_read_bits_calc_req_data_size();


/**
 * @brief Calculate the answer size
 * @param[in] _quantity The quantity of bits
 * @return The size of answer's data size
 */
int emb_read_bits_calc_answer_data_size(uint16_t _quantity);

enum EMB_RB_TYPE {
    EMB_RB_COILS,
    EMB_RB_DISCRETE_INPUTS,
};

/**
 * @brief Build request
 *
 * This fucntion builds the "Read Coils" or "Read Discrete Inputs" request.
 *
 * \param[out] _result_req Result request.
 * \param[in] _starting_address Starting address of bits.
 * \param[in] _type The EMB_RB_COILS or EMB_RB_DISCRETE_INPUTS, depent of
 * Read Coils or Read Discrete Inputs you want to build.
 * \param[in] _quantity Number of bits, that will be readed from device.
 * @return Zero if a request is ready, otherwise error code.
 */
int emb_read_bits_make_req(emb_pdu_t* _result_req,
                           enum EMB_RB_TYPE _type,
                           uint16_t _starting_address, uint16_t _quantity);

/**
 * @brief Get starting address from request
 * @param[in] _req Request, from which reads a starting address
 * @return Starting address
 */
uint16_t emb_read_bits_get_starting_addr(emb_const_pdu_t* _req);

/**
 * @brief Get quantity from request
 * @param[in] _req Request, from which reads a quantity
 * @return Quantity
 */
uint16_t emb_read_bits_get_quantity(emb_const_pdu_t* _req);

/**
 * @brief Get bits from answer
 *
 * Function returns a bit value from answer.
 *
 * @param[in] _ans Answer
 * @param[in] _bit_offset Offset of the bit inside answer.
 * @return Bit value.
 */
char emb_read_bits_get_bit(emb_const_pdu_t* _answer,
                             uint16_t _bit_offset);

/**
 * @brief Get eight bits from answer.
 *
 * Function returns a eight bits values from answer.
 *
 * @param[in] _ans Answer
 * @param[in] _byte_offset Offset to the byte in the answer.
 * @return An eight bits values. (One byte from answer)
 */
uint8_t emb_read_bits_get_byte(emb_const_pdu_t* _answer,
                               uint8_t _byte_offset);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_READ_BITS
