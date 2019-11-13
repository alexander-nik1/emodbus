
#ifndef MODBUS_APPLICATION_DATA_UNIT_H
#define MODBUS_APPLICATION_DATA_UNIT_H

#include "modbus_pdu.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The PDU descriptions.
 *
 * This file contains the structures, that describes ADU.
 *
 */

/**
 * @brief Data store for ADU.
 *
 * Application data unit tructure
 */
struct _emb_adu_t
{
    uint32_t transaction_id;    ///< Transaction ID (used in ModbusTCP)
    uint8_t server_id;          ///< Server (slave) address (Id).
    uint32_t flags;             ///< Some flags for transport level
    emb_pdu_t* pdu;             ///< PDU storage pointer.
};

typedef struct _emb_adu_t emb_adu_t;

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_APPLICATION_DATA_UNIT_H
