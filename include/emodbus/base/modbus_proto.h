
#ifndef MODBUS_MASTER_PROTOCOL_H
#define MODBUS_MASTER_PROTOCOL_H

#include "modbus_pdu.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The protocol abstractions.
 *
 * This file contains an abstractions of modbus protocols
 * like: RTU, ASCII, TCP and so on.
 *
 */

/* ____________
  |            |
  | high level |
  | of modbus  |
  |____________|
      |    ^
      |    |
   ___V____|___
  |            |
  |   proto    | <-- this level
  |____________|
      |    ^
      |    |
   ___V____|__
  |           |
  | low level |
  | RTU,TCP,  |
  | and so on |
  |___________|
*/

/**
 * @brief Interface of modbus protocol
 *
 * This interface connects high and low level of modbus.
 */
struct emb_protocol_t {

    /**
     * @brief This is a pointer to high-level context.
     */
    void* high_level_context;

    /**
     * @brief This is a pointer to low level context.
     */
    void* low_level_context;

    /**
     * @brief Send packet to device
     *
     * This function calls from high to low level.
     * This function calls for send a PDU via this protocol.
     *
     * \param[in] _user_data Low level context.
     * \param[in] _slave_addr Address of modbus device.
     * \param[in] _pkt PDU, that will be sent to modbus device.
     * \return Zero on success or error code.
     */
    int (*send_packet)(void* _user_data,
                       int _slave_addr,
                       emb_const_pdu_t* _pkt);

    /**
     * @brief Receive packet from device
     *
     * This function calls from low to high level.
     * This function calls by low level when, a new PDU has
     * received via this protocol.
     *
     * \param[in] _user_data High level context.
     * \param[in] _slave_addr Address of modbus device.
     * \param[in] _pkt PDU, that was received by low level.
     */
    void (*recv_packet)(void* _user_data,
                        int _slave_addr,
                        emb_const_pdu_t* _pkt);

    /**
     * @brief Error on low-level
     *
     * This function calls from low to high level.
     * This function tells about errors.
     *
     * \param[in] _user_data High level context.
     * \param[in] _errno Number of error (may be error from system's
     * errno.h or from modbus_errno.h files).
     */
    void (*error)(void* _user_data, int _errno);

    /**
     * @brief Transmit PDU (property of protocol)
     * This PDU already have a buffer (buffer inside protocol)
     * and this PDU can be used to store the new request packet, and send it.
     * In general, you will use protocol's buffer instead another buffer.
     * It is useful in applications, that have no enough RAM space.
     * NOTE: In cae of ASCII protocols you should NOT use this PDU.
     */
//    emb_pdu_t* tx_pdu;
/*
    NOTE:
        [high_level_context, recv_packet, error] variables
        are filled by high level.

        [low_level_context, send_packet, tx_pdu] variables
        are filled by low level.
*/
};

/**
 * @brief Send one PDU
 *
 * This function calls from high level to low level.
 * This function is simple wrapper of call:
 * _proto->send_packet(...)
 *
 * \param[in] _proto_ Protocol context.
 * \param[in] _slave_addr_ Address of modbus device.
 * \param[in] _pkt_ PDU, that will be sent to modbus device.
 * \return Zero on success or error code.
 */

#define emb_proto_send_packet(_proto_, _slave_addr_, _pkt_) \
    (_proto_)->send_packet((_proto_)->low_level_context, _slave_addr_, _pkt_)

/**
 * @brief Receive one PDU
 *
 * This function calls from low level to high level.
 * This function is simple wrapper of call:
 * _proto->recv_packet(...)
 *
 * \param[in] _proto_ Protocol context.
 * \param[in] _slave_addr_ Address of modbus device.
 * \param[in] _pkt_ PDU, that was received by low level.
 */

#define emb_proto_recv_packet(_proto_, _slave_addr_, _pkt_) \
    (_proto_)->recv_packet((_proto_)->high_level_context, _slave_addr_, _pkt_)

/**
 * @brief Send low-level errors to high-level
 *
 * This function calls from low level to high level.
 * This function is simple wrapper of call:
 * _proto->error(...)
 *
 * \param[in] _proto_ Protocol context.
 * \param[in] _errno_ Number of error (may be error from system's
 * errno.h or from modbus_errno.h files).
 */

#define emb_proto_error(_proto_, _errno_)   \
    if((_proto_)->error)   \
        (_proto_)->error((_proto_)->high_level_context, (_errno_))

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_PROTOCOL_H
