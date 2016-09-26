
#ifndef EMODBUS_TRANSPORT_H
#define EMODBUS_TRANSPORT_H

#include <emodbus/base/modbus_pdu.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The transport abstractions.
 *
 * This file contains an abstractions of modbus transport
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
  | transport  | <-- this level
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

#if EMODBUS_PACKETS_DUMPING

/// Flag, that enables a dumping all packets into a dbg_print_packet function.
#define EMB_TRANSPORT_FLAG_DUMD_PAKETS (1 << 0)

/// Flag, that tells from high level to low level, about this is a modbus-server.
#define EMB_TRANSPORT_FLAG_IS_SERVER   (1 << 1)

#endif // EMODBUS_PACKETS_DUMPING

/**
 * @brief Interface of modbus transport
 *
 * This interface connects high and low level of modbus.
 */
struct emb_transport_t {

    /**
     * @brief This is a pointer to high-level context.
     */
    void* high_level_context;

    /**
     * @brief This is a pointer to low level context.
     */
    void* transport_context;

    /**
     * @brief Send packet to device
     *
     * This function calls from high to low level.
     * This function calls for send a PDU via this transport.
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
     * received via this transport.
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
     * @brief Receive PDU.
     * A newest received PDU will placed in this rx_pdu.
     * If you want to receive a PDU, you must set this pointer
     * to a emb_pdu_t object.
     * If rx_pdu is NULL, then no receiving of data will be perfomed.
     */
     emb_pdu_t* rx_pdu;

     /**
      * @brief Some flags for low level
      */
     int flags;

/*
    NOTE:
        [high_level_context, recv_packet, error] variables
        are filled by high level.

        [transport_context, send_packet, tx_pdu] variables
        are filled by low level.
*/
};

/**
 * @brief Send one PDU
 *
 * This function calls from high level to low level.
 * This function is simple wrapper of call:
 * _transport_->send_packet(...)
 *
 * \param[in] _transport_ Transport context.
 * \param[in] _slave_addr_ Address of modbus device.
 * \param[in] _pkt_ PDU, that will be sent to modbus device.
 * \return Zero on success or error code.
 */

#define emb_transport_send_packet(_transport_, _slave_addr_, _pkt_) \
    (_transport_)->send_packet((_transport_)->transport_context, _slave_addr_, _pkt_)

/**
 * @brief Receive one PDU
 *
 * This function calls from low level to high level.
 * This function is simple wrapper of call:
 * _transport_->recv_packet(...)
 *
 * \param[in] _transport_ Transport context.
 * \param[in] _slave_addr_ Address of modbus device.
 * \param[in] _pkt_ PDU, that was received by low level.
 */

#define emb_transport_recv_packet(_transport_, _slave_addr_, _pkt_) \
    (_transport_)->recv_packet((_transport_)->high_level_context, _slave_addr_, _pkt_)

/**
 * @brief Send low-level errors to high-level
 *
 * This function calls from low level to high level.
 * This function is simple wrapper of call:
 * _transport_->error(...)
 *
 * \param[in] _transport_ Transport context.
 * \param[in] _errno_ Number of error (may be error from system's
 * errno.h or from modbus_errno.h files).
 */

#define emb_transport_error(_transport_, _errno_)   \
    if((_transport_)->error)   \
        (_transport_)->error((_transport_)->high_level_context, (_errno_))

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMODBUS_TRANSPORT_H
