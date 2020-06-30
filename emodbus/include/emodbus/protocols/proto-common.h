
#ifndef EMB_PROTOCOL_COMMON_H
#define EMB_PROTOCOL_COMMON_H

/*!
 * \file
 * \brief Protocol common definitions.
 *
 * This file contains an RTU protocol decalrations.
 *
 */

#include <emodbus/base/modbus_xdu.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*proto_encode_packet_t)(const emb_adu_t *_adu,
                                     uint8_t* _packet,
                                     unsigned int _pkt_size);

typedef int (*proto_decode_packet_t)(const uint8_t* _packet,
                                     unsigned int _pkt_size,
                                     emb_adu_t* _result);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMB_PROTOCOL_COMMON_H
