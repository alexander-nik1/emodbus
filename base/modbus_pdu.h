
#ifndef MODBUS_PROTOCOL_DATA_UNIT_H
#define MODBUS_PROTOCOL_DATA_UNIT_H

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
    int function;   ///< Function of this PDU
    int data_size;  ///< Size of data of this PDU
    void* data;     ///< Data of this PDU
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
    int function;       ///< Function of this PDU
    int data_size;      ///< Size of data of this PDU
    const void* data;   ///< Data of this PDU
};

typedef const struct _emb_const_pdu_t emb_const_pdu_t;

/**
 * @brief This macros gives you modbus_const_pdu_t* pointer from modbus_pdu_t*
 *
 */
#define MB_CONST_PDU(_pdu_) ((emb_const_pdu_t*)(_pdu_))

#endif // MODBUS_PROTOCOL_DATA_UNIT_H
