
#ifndef MODBUS_PROTOCOL_DATA_UNIT_H
#define MODBUS_PROTOCOL_DATA_UNIT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The PDU descriptions.
 *
 * This file contains the structures, that describes PDU.
 * And useful macros.
 *
 */

/**
 * @brief This structure describes one PDU
 *
 * @detailed You should use this object, where imply change data of this PDU
 *
 */
struct _emb_pdu_t {
    uint8_t function;   ///< Function of this PDU
    uint8_t data_size;  ///< Size of data of this PDU
    uint8_t max_size;   ///< Maximum size (when this PDU is used as receive place)
    void* data;         ///< Data of this PDU
};

typedef struct _emb_pdu_t emb_pdu_t;

/**
 * @brief This structure describes one PDU (const version)
 *
 * @detailed You should use this object, where you don't want
 * to change any data in this structure
 *
 */
struct _emb_const_pdu_t {
    uint8_t function;       ///< Function of this PDU
    uint8_t data_size;      ///< Size of data of this PDU
    uint8_t max_size;       ///< Maximum size (when this PDU is used as receive place)
    const void* data;       ///< Data of this PDU
};

typedef const struct _emb_const_pdu_t emb_const_pdu_t;

/**
 * @brief This macros gives you modbus_const_pdu_t* pointer from modbus_pdu_t*
 *
 */
#define MB_CONST_PDU(_pdu_) ((emb_const_pdu_t*)(_pdu_))

/**
 * @brief Check a PDU for modbus-exception
 *
 * This function checks a 8-th bit of a function byte, and
 * if it is 1, then function return a error code from this packet.
 *
 * @param _pdu a PDU for check
 * @return returns zero if this PDU have no exception code,
 * otherwise it returns a modbus-exception code plus 1500.
 */
int emb_check_pdu_for_exception(emb_const_pdu_t *_pdu);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_PROTOCOL_DATA_UNIT_H
