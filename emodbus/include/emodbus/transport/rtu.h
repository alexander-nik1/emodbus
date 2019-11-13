
#ifndef MODBUS_MASTER_RTU_H
#define MODBUS_MASTER_RTU_H

/*!
 * \file
 * \brief RTU Transport.
 *
 * This file contains an RTU transport decalrations.
 *
 */

#include <emodbus/base/modbus_adu.h>

#ifndef EMB_RTU_CRC_FUNCTION
#define EMB_RTU_CRC_FUNCTION(_buf_, _size_)  crc16(_buf_, _size_)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define EMB_RTU_DO_DATA_COPY  (1 << 0)

/**
 * @brief RTU Encode packet
 *
 * Encodes packet to sending over RTU protocol.
 *
 * @param [in] _given_data All neccessary data to encode
 * @param [out] _packet A place to store packet
 * @param [in] _pkt_size A size of the place to store packet
 *
 * @return number bytes to send if no errors, otherwise negative value.
 */

int emb_rtu_encode_packet(const emb_adu_t *_adu,
                          uint8_t* _packet,
                          unsigned int _pkt_size);

/**
 * @brief RTU Decode packet
 *
 * Decodes (parses) a previously received packet
 *
 * @param [in] _packet All neccessary data to encode
 * @param [in] _pkt_size A place to store packet
 * @param [out] _result A size of the place to store packet
 *
 * @return 0 if no errors, otherwise negative value.
 */

int emb_rtu_decode_packet(const uint8_t* _packet,
                          unsigned int _pkt_size,
                          emb_adu_t* _result);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_RTU_H
