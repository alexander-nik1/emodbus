
#ifndef MODBUS_MASTER_RTU_H
#define MODBUS_MASTER_RTU_H

/*!
 * \file
 * \brief RTU Protocol.
 *
 * This file contains an RTU protocol decalrations.
 *
 */

#include "../base/add/stream.h"

#include "../base/modbus_proto.h"

#ifdef __cplusplus
extern "C" {
#endif

//typedef void (*modbus_rtu_on_packet_t)(void* _user_data, const void* _packet, unsigned int _size);

/**
 * @brief Function type for signaling of received character.
 *
 *
 *
 */
typedef void (*modbus_rtu_on_char_t)(void* _user_data);

/**
 * @brief RTU Protocol context.
 *
 * Object of this structure is a one RTU protocol context.
 * Here we have twoo interfaces:
 * 1) modbus_protocol_t - interface for connect to high level
 * 2) RTU Interface - interface for connect to hardware transceiver.
 */
struct modbus_rtu_t {

    /// (RTU Interface) Pointer to receive buffer (must be set by user)
    unsigned char* rx_buffer;
    /// (RTU Interface) Pointer to transmit buffer (must be set by user)
    unsigned char* tx_buffer;
    /// (RTU Interface) Size of receive buffer (must be set by user)
    unsigned int rx_buf_size;
    /// (RTU Interface) Size of transmit buffer (must be set by user)
    unsigned int tx_buf_size;

    /// Counter of receiver's bytes (internal variable)
    unsigned int rx_buf_counter;

    /// Counter of transmitter's bytes (internal variable)
    unsigned int tx_buf_counter;
    /// Size of packet for transmitting (internal variable)
    unsigned int tx_pkt_size;

    /// In this PDU stores received packet, and this PDU gets to high level (internal variable)
    emb_pdu_t rx_pdu;
    /// This PDU contains a packet, that will be sent (internal variable)
    emb_pdu_t tx_pdu;

    /// (RTU Interface) User context (Argument of callback functions) (must be set by user)
    void* user_data;

    ///< Protocol (abstract connector to high level)
    struct emb_protocol_t proto;

    /// (RTU Interface) This function calls when a new char was received (must be set by user)
    modbus_rtu_on_char_t modbus_rtu_on_char;

    /// (RTU Interface) Stream for reading from it. (must be connected to hardware receiver by user)
    struct input_stream_t input_stream;

    /// (RTU Interface) Stream for writing to. (must be connected to hardware transmitter by user)
    struct output_stream_t output_stream;
};

/**
 * @brief Initialize an RTU context
 *
 * (RTU Interface) This function initializes an RTU context.
 * User must set all variables, that marked "(must be set by user)",
 * before call this function.
 *
 * @param [in] _mbt Structure that will be initialized
 */
void modbus_rtu_initialize(struct modbus_rtu_t* _mbt);

/**
 * @brief Signal of timeout between characters.
 *
 * (RTU Interface) User should call this function, when time of
 * last received character is expired.
 *
 * @param [in] _mbt RTU context
 */
void modbus_rtu_on_char_timeout(struct modbus_rtu_t* _mbt);

/**
 * @brief Calls by user when error caused on low level.
 *
 * (RTU Interface) Send error to high level.
 *
 * @param [in] _mbt RTU context
 * @param [in] _errno Error number.
 */
void modbus_rtu_on_error(struct modbus_rtu_t* _mbt,
                         int _errno);

/**
 * @brief Send a packet synchronously.
 *
 * (RTU Interface) Same as modbus_rtu_send_packet, but it will wait untill
 * all data has been sent.
 *
 * @param [in] _mbt RTU context
 * @param [in] _slave_addr Address of slave device.
 * @param [in] _pdu PDU, that will be sent.
 *
 * @return Zero on success, or error code on fail.
 */
int modbus_rtu_send_packet_sync(struct modbus_rtu_t* _mbt,
                                int _slave_addr,
                                emb_const_pdu_t* _pdu);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_RTU_H
