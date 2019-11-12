
#ifndef MODBUS_MASTER_RTU_H
#define MODBUS_MASTER_RTU_H

/*!
 * \file
 * \brief RTU Transport.
 *
 * This file contains an RTU transport decalrations.
 *
 */

#include <emodbus/base/modbus_transport.h>

#ifndef EMB_RTU_CRC_FUNCTION
#define EMB_RTU_CRC_FUNCTION(_buf_, _size_)  crc16(_buf_, _size_)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// How much bytes in the packet before PDU
/// data begins: address + function = 2 bytes
enum { EMB_RTU_PKT_PREFIX_SIZE = 2 };

/// How much bytes in the packet after PDU
/// data ends: crc = 2 bytes
enum { EMB_RTU_PKT_SUFFIX_SIZE = 2 };

#define EMB_RTU_DO_DATA_COPY  (1 << 0)

int emb_rtu_encode_packet(const struct emb_transport_info_t *_given_data,
                          uint8_t* _packet,
                          unsigned int _pkt_size);

int emb_rtu_decode_packet(const uint8_t* _packet,
                          unsigned int _pkt_size,
                          struct emb_transport_info_t* _result);


#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_RTU_H
