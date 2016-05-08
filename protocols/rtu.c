
#include <emodbus/protocols/rtu.h>
#include <emodbus/base/modbus_proto.h>
#include <emodbus/base/common.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/add/crc.h>
#include <emodbus/base/modbus_errno.h>

/*!
 * \file
 * \brief RTU Protocol.
 *
 * This file contains an RTU protocol realizations.
 *
 */

#define read_data_from_port(_mbt_)  emb_rtu_port_event((_mbt_), emb_rtu_data_received_event)
#define write_data_to_port(_mbt_)   emb_rtu_port_event((_mbt_), emb_rtu_tx_buf_empty_event)

/**
 * @brief Parse packet
 *
 * This function checks validity of received packet.
 * And if it is valid, then sends it to high level.
 *
 * @param [in] _mbt RTU context
 */
static void parse_packet(struct emb_rtu_t* _mbt) {
    int res;
    const unsigned char* buf = _mbt->rx_buffer;
    const unsigned int size = _mbt->rx_buf_counter-2;
    if(size >= 2) { // 4 bytes - minimal packet size
        const uint16_t crc1 = crc16(buf, size);
        const uint16_t crc2 = MKWORD(buf[size], buf[size+1]);
#if EMODBUS_PACKETS_DUMPING
        if(_mbt->proto.flags & EMB_PROTO_FLAG_DUMD_PAKETS)
            if(emb_dump_rx_data)
                emb_dump_rx_data(_mbt->rx_buffer, _mbt->rx_buf_counter);
#endif // EMODBUS_PACKETS_DUMPING
        if(crc1 != crc2) {
            emb_proto_error(&_mbt->proto, -modbus_bad_crc);
            return;
        }
        emb_pdu_t* const rx_pdu = _mbt->proto.rx_pdu;
        if(rx_pdu) {
            const unsigned int data_sz = size - 2;
            const unsigned int max_sz = rx_pdu->max_size;
            if(data_sz > max_sz) {
                emb_proto_error(&_mbt->proto, -modbus_resp_buffer_ovf);
                return;
            }
            rx_pdu->function = buf[1];
            rx_pdu->data_size = size - 2;
            memcpy(rx_pdu->data, buf + 2, data_sz);
            emb_proto_recv_packet(&_mbt->proto, (int)buf[0], MB_CONST_PDU(rx_pdu));
        }
    }
}

/**
 * @brief Send a PDU.
 *
 * (Protocol interface) This function will send packet (CRC suffix automatically added).
 *
 * @param [in] _mbt RTU context
 * @param [in] _slave_addr Address of slave
 * @param [in] _pdu PDU, that will be sent.
 *
 * @return Zero on success, error on fail.
 */
static int modbus_rtu_send_packet(void *_mbt,
                           int _slave_addr,
                           emb_const_pdu_t *_pdu) {

    struct emb_rtu_t* mbt = (struct emb_rtu_t*)_mbt;

    if((_pdu->data_size + 4) <= mbt->tx_buf_size) {

        const int sz = _pdu->data_size + 2;
        uint16_t crc;
        mbt->tx_buffer[0] = _slave_addr;
        mbt->tx_buffer[1] = _pdu->function;
        memcpy(mbt->tx_buffer + 2, _pdu->data, _pdu->data_size);
        crc = crc16(mbt->tx_buffer, sz);
        memcpy(mbt->tx_buffer + sz, &crc, 2);
        mbt->tx_pkt_size = sz + 2;
        mbt->tx_buf_counter = 0;
        write_data_to_port(mbt);
#if EMODBUS_PACKETS_DUMPING
        if(mbt->proto.flags & EMB_PROTO_FLAG_DUMD_PAKETS)
            if(emb_dump_tx_data)
                emb_dump_tx_data(mbt->tx_buffer, mbt->tx_pkt_size);
#endif // EMODBUS_PACKETS_DUMPING
        return 0;
    }
    else
        return -modbus_buffer_overflow;
}

void emb_rtu_initialize(struct emb_rtu_t* _mbt) {
    _mbt->rx_buf_counter = 0;
    _mbt->tx_buf_counter = 0;
    _mbt->tx_pkt_size = 0;

    // Setup protocol
    _mbt->proto.send_packet = modbus_rtu_send_packet;
    _mbt->proto.low_level_context = _mbt;

    _mbt->tx_pdu = NULL;
}

void emb_rtu_on_char_timeout(struct emb_rtu_t* _mbt) {
    parse_packet(_mbt);
    _mbt->rx_buf_counter = 0;
}

void emb_rtu_on_error(struct emb_rtu_t* _mbt,
                      int _errno) {
    emb_proto_error(&_mbt->proto, _errno);
}

void emb_rtu_port_event(struct emb_rtu_t* _mbt,
                        enum emb_rtu_port_event_t _event) {

    switch(_event) {
    case emb_rtu_data_received_event:
        _mbt->rx_buf_counter += _mbt->read_from_port(_mbt,
                                                     _mbt->rx_buffer + _mbt->rx_buf_counter,
                                                     _mbt->rx_buf_size - _mbt->rx_buf_counter);
        _mbt->emb_rtu_on_char(_mbt);
        break;

    case emb_rtu_tx_buf_empty_event:
        _mbt->tx_buf_counter += _mbt->write_to_port(_mbt,
                                                    _mbt->tx_buffer + _mbt->tx_buf_counter,
                                                    _mbt->tx_pkt_size - _mbt->tx_buf_counter);
        break;
    }
}

int emb_rtu_send_packet_sync(struct emb_rtu_t* _mbt,
                             int _slave_addr,
                             emb_const_pdu_t *_pdu) {
    int r;
    _mbt->tx_pdu = _pdu;
    if((r = modbus_rtu_send_packet(_mbt, _slave_addr, _pdu)) == 0) {
        while(_mbt->tx_buf_counter < _mbt->tx_pkt_size) {
            write_data_to_port(_mbt);
        }
        return 0;
    }
    else
        return r;
}
