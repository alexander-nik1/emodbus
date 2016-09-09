
#ifndef MODBUS_MASTER_WRITE_COILS
#define MODBUS_MASTER_WRITE_COILS

/*!
 * \file
 * \brief Write Coils functions.
 *
 * Functions for working with Write Coils function.
 *
 */

#include <stdint.h>

#include <emodbus/base/modbus_pdu.h>
#include <emodbus/client/client_base.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { EMB_WRITE_COILS_MAX_QUANTITY = 0x07B0 };

/**
 * @brief Calculate the request size
 * @param[in] _quantity The quantity of coils
 * @return The size of request's data size
 */
int emb_write_coils_calc_req_data_size(uint16_t _quantity);


/**
 * @brief Calculate the answer size
 * @return The size of answer's data size
 */
int emb_write_coils_calc_answer_data_size();

/**
 * @brief Build request
 *
 * This fucntion builds the "Write Coils" request.
 *
 * \param[out] _result_req Result request.
 * \param[in] _starting_address Starting address of coils.
 * \param[in] _quantity Number of coils, that will be readed from device.
 * \param[in] _pcoils The array of bits. Each byte has an eight values for eight coils.
 * @return Zero if a request is ready, otherwise error code.
 */
int emb_write_coils_make_req(emb_pdu_t* _result_req,
                             uint16_t _starting_address,
                             uint16_t _quantity,
                             const uint8_t* _pcoils);

/**
 * @brief Get starting address from request
 * @param[in] _req Request, from which reads a starting address
 * @return Starting address
 */
uint16_t emb_write_coils_get_starting_addr(emb_const_pdu_t* _req);

/**
 * @brief Get quantity from request
 * @param[in] _req Request, from which reads a quantity
 * @return Quantity
 */
uint16_t emb_write_coils_get_quantity(emb_const_pdu_t* _req);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_WRITE_COILS
