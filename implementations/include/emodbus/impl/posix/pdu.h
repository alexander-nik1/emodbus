
#ifndef EMB_POSIX_IMPL_PDU_H
#define EMB_POSIX_IMPL_PDU_H

#include <emodbus/base/modbus_pdu.h>

/**
 * @brief emb_posix_alloc_pdu_data
 * Allocates the memory for pdu.
 * @param _pdu The pdu
 * @return Zero, if ok, otherwise error code.
 */
int emb_posix_alloc_pdu_data(emb_pdu_t* _pdu);

/**
 * @brief emb_posix_alloc_pdu_data
 * De-allocates the memory for pdu.
 * @param _pdu The pdu
 * @return Zero, if ok, otherwise error code.
 */
int emb_posix_free_pdu_data(emb_pdu_t* _pdu);

#endif // EMB_POSIX_IMPL_PDU_H
