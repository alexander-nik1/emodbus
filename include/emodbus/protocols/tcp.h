
#ifndef MODBUS_MASTER_TCP_H
#define MODBUS_MASTER_TCP_H

/*!
 * \file
 * \brief TCP Protocol.
 *
 * This file contains an TCP protocol decalrations.
 *
 */

#include <emodbus/base/modbus_proto.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { emb_tcp_mbap_size = 7 };

enum { emb_tcp_rx_buf_size = MAX_PDU_SIZE + emb_tcp_mbap_size };
enum { emb_tcp_tx_buf_size = MAX_PDU_SIZE + emb_tcp_mbap_size };

struct emb_tcp_t {

    unsigned int rx_pkt_counter;
    unsigned char rx_buf[emb_tcp_rx_buf_size];

    unsigned int tx_pkt_size;
    unsigned int tx_pkt_counter;
    unsigned char tx_buf[emb_tcp_tx_buf_size];

    struct emb_protocol_t proto;

    /// (modbus-TCP Interface) This function are implemented by physical port.
    /// By using this call, modbus-TCP can read data from physical port.
    unsigned int (*read_from_port)(struct emb_tcp_t* _this, void* _p_buf, unsigned int _buf_size);

    /// (modbus-TCP Interface) This function are implemented by physical port.
    /// By using this call, modbus-TCP can write data into physical port.
    unsigned int (*write_to_port)(struct emb_tcp_t* _this, const void* _p_data, unsigned int _sz_to_write);
};

/**
 * @brief Initialize an TCP context
 *
 * (TCP Interface) This function initializes an TCP context.
 * User must set all variables, that marked "(must be set by user)",
 * before call this function.
 *
 * @param [in] _mbt Structure that will be initialized
 */
void emb_tcp_initialize(struct emb_tcp_t* _mbt);

/**
 * @brief Calls by user when error caused on low level.
 *
 * (TCP Interface) Send error to high level.
 *
 * @param [in] _mbt TCP context
 * @param [in] _errno Error number.
 */
void emb_tcp_on_error(struct emb_tcp_t* _mbt,
                      int _errno);

/**
 * @brief The emb_tcp_port_event_t enum
 *
 * An events on port.
 */
enum emb_tcp_port_event_t {
    emb_tcp_data_received_event,    ///< Port tells: I have a received data for you.
    emb_tcp_tx_buf_empty_event      ///< Port tells: My transmit buffer is empty, you can write to.
};

/**
 * @brief Calls by port-implementation.
 *
 * (TCP Interface) By this call a port can
 * say about an events in emb_tcp_port_event_t.
 *
 * @param [in] _mbt TCP context
 * @param [in] _event Event code
 */
void emb_tcp_port_event(struct emb_tcp_t* _mbt,
                        enum emb_tcp_port_event_t _event);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_TCP_H
