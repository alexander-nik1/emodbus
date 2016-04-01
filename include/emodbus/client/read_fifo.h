
#ifndef MODBUS_MASTER_READ_FIFO
#define MODBUS_MASTER_READ_FIFO


/*!
 * \file
 * \brief Declaration of Read Fifo functions.
 *
 * Functions for working with Read Fifo function.
 *
 */

#include <emodbus/base/modbus_pdu.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate the request size
 * @return The size of request's data size
 */
int emb_read_fifo_calc_req_data_size();

/**
 * @brief Calculate the answer size
 * @return The size of answer's data size
 */
int emb_read_fifo_calc_answer_data_size();

/**
 * @brief Build request
 *
 * This fucntion builds the "Read Fifo" request.
 *
 * @param[out] _result_req Result request.
 * @param[in] _starting_address Address of starting register
 * @return Zero if a request is ready, otherwise error code.
 */
int emb_read_fifo_make_req(emb_pdu_t* _result_req,
                           uint16_t _starting_address);

/**
 * @brief Get byte count from answer
 *
 * This function can retrun a byte-count field
 * from a given answer.
 *
 * @param[in] _answer The answer
 * @return Byte count
 */
uint16_t emb_read_fifo_byte_count(emb_const_pdu_t* _answer);

/**
 * @brief Get regs count from answer
 *
 * This function can retrun a registers count field
 * from a given answer.
 *
 * @param[in] _answer The answer
 * @return Registers count
 */
uint16_t emb_read_fifo_regs_count(emb_const_pdu_t* _answer);

/**
 * @brief Get the data register from answer
 *
 * This function can retrun a data register
 * from a given answer.
 *
 * @param[in] _answer The answer
 * @param[in] _offset The offset to a data within the answer.
 * @return Data register.
 */
uint16_t emb_read_fifo_get_data(emb_const_pdu_t* _answer,
                                uint16_t _offset);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_READ_FIFO
