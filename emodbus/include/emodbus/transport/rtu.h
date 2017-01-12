
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

/**
 * @brief RTU Transport context.
 *
 * Object of this structure is a one RTU transport context.
 * Here we have twoo interfaces:
 * 1) modbus_transport_t - interface for connect to high level
 * 2) RTU Interface - interface for connect to hardware transceiver.
 */
struct emb_rtu_t {

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

    /// This PDU contains a packet, that will be sent (internal variable)
    emb_const_pdu_t* tx_pdu;

    ///< Transport (abstract connector to high level)
    struct emb_transport_t transport;

    /// (RTU Interface) This function calls when a new char was received (must be set by user)
    void (*emb_rtu_on_char)(struct emb_rtu_t* _emb);

    /// (RTU Interface) This function are implemented by physical port.
    /// By using this call, RTU can read data from physical port.
    int (*read_from_port)(struct emb_rtu_t* _this, void* _p_buf, unsigned int _buf_size);

    /// (RTU Interface) This function are implemented by physical port.
    /// By using this call, RTU can write data into physical port.
    int (*write_to_port)(struct emb_rtu_t* _this,
                         const void* _p_data,
                         unsigned int _sz_to_write,
                         unsigned int* _wrote);
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
void emb_rtu_initialize(struct emb_rtu_t* _mbt);

/**
 * @brief Signal of timeout between characters.
 *
 * (RTU Interface) User should call this function, when time of
 * last received character is expired.
 *
 * @param [in] _mbt RTU context
 */
void emb_rtu_on_char_timeout(struct emb_rtu_t* _mbt);

/**
 * @brief Calls by user when error caused on low level.
 *
 * (RTU Interface) Send error to high level.
 *
 * @param [in] _mbt RTU context
 * @param [in] _errno Error number.
 */
void emb_rtu_on_error(struct emb_rtu_t* _mbt,
                         int _errno);

/**
 * @brief The emb_rtu_port_event_t enum
 *
 * An events on port.
 */
enum emb_rtu_port_event_t {
    emb_rtu_data_received_event,    ///< Port tells: I have a received data for you.
    emb_rtu_tx_buf_empty_event      ///< Port tells: My transmit buffer is empty, you can write to.
};

/**
 * @brief Calls by port-implementation.
 *
 * (RTU Interface) By this call a port can
 * say about an events in emb_rtu_port_event_t.
 *
 * @param [in] _mbt RTU context
 * @param [in] _event Event code
 */
void emb_rtu_port_event(struct emb_rtu_t* _mbt,
                        enum emb_rtu_port_event_t _event);

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
int emb_rtu_send_packet_sync(struct emb_rtu_t* _mbt,
                                int _slave_addr,
                                emb_const_pdu_t* _pdu);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_RTU_H
