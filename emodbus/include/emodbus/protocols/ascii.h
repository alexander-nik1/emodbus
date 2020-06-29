
#ifndef EMB_PROTOCOL_ASCII_H
#define EMB_PROTOCOL_ASCII_H

/*!
 * \file
 * \brief RTU Protocol.
 *
 * This file contains an RTU protocol decalrations.
 *
 */

#include <emodbus/base/modbus_xdu.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ASCII Encode packet
 *
 * Encodes packet to sending over RTU protocol.
 *
 * @param [in] _given_data All neccessary data to encode
 * @param [out] _packet A place to store packet
 * @param [in] _pkt_size A size of the place to store packet
 *
 * @return number bytes to send if no errors, otherwise negative value.
 */

int emb_ascii_encode_packet(const emb_adu_t *_adu,
                            uint8_t* _packet,
                            unsigned int _max_pkt_size);

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

int emb_ascii_decode_packet(const uint8_t* _packet,
                            unsigned int _pkt_size,
                            emb_adu_t* _result);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMB_PROTOCOL_ASCII_H
