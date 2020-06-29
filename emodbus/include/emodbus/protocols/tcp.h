
#ifndef EMB_PROTOCOL_TCP_H
#define EMB_PROTOCOL_TCP_H


/*!
 * \file
 * \brief TCP Protocol.
 *
 * This file contains an TCP protocol decalrations.
 *
 */

#include <emodbus/base/modbus_xdu.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__ ((packed))
{
    uint16_t transact_id;
    uint16_t proto_id;
    uint16_t length;
    uint8_t unit_id;
} emb_tcp_header_t;

enum { emb_tcp_header_size = sizeof(emb_tcp_header_t) };

enum { emb_tcp_rx_buf_size = MAX_PDU_SIZE + emb_tcp_header_size };
enum { emb_tcp_tx_buf_size = MAX_PDU_SIZE + emb_tcp_header_size };


/**
 * @brief TCP Encode packet
 *
 * Encodes packet to sending over TCP protocol.
 *
 * @param [in] _given_data All neccessary data to encode
 * @param [out] _packet A place to store packet
 * @param [in] _pkt_size A size of the place to store packet
 *
 * @return number bytes to send if no errors, otherwise negative value.
 */

int emb_tcp_encode_packet(const emb_adu_t *_adu,
                          uint8_t* _packet,
                          unsigned int _pkt_size);

/**
 * @brief TCP Decode packet
 *
 * Decodes (parses) a previously received packet
 *
 * @param [in] _packet All neccessary data to encode
 * @param [in] _pkt_size A place to store packet
 * @param [out] _result A size of the place to store packet
 *
 * @return 0 if no errors, otherwise negative value.
 */
int emb_tcp_decode_packet(const uint8_t* _packet,
                          unsigned int _pkt_size,
                          emb_adu_t* _result);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMB_PROTOCOL_TCP_H
