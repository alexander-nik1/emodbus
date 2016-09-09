
#ifndef MODBUS_MASTER_WRITE_COIL
#define MODBUS_MASTER_WRITE_COIL

/*!
 * \file
 * \brief Write Coil functions.
 *
 * Functions for working with Write Coil function.
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
 * @return The size of request's data size
 */
int emb_write_coil_calc_req_data_size();


/**
 * @brief Calculate the answer size
 * @return The size of answer's data size
 */
int emb_write_coil_calc_answer_data_size();

/**
 * @brief Build request
 *
 * This fucntion builds the "Write Coil" request.
 *
 * \param[out] _result_req Result request.
 * \param[in] _address Address of the coil.
 * \param[in] _value A value for this coil.
 * @return Zero if a request is ready, otherwise error code.
 */
int emb_write_coil_make_req(emb_pdu_t* _result_req,
                             uint16_t _address,
                             char _value);

/**
 * @brief Get the address from request
 * @param[in] _req Request, from which reads an address
 * @return The address
 */
uint16_t emb_write_coil_get_addr(emb_const_pdu_t* _req);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_WRITE_COILS
