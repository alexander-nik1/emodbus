
#include <emodbus/protocols/tcp.h>
#include <emodbus/base/modbus_proto.h>
#include <emodbus/base/common.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Send a PDU.
 *
 * (Protocol interface) This function will send packet (CRC suffix automatically added).
 *
 * @param [in] _mbt TCP context
 * @param [in] _slave_addr Address of slave
 * @param [in] _pdu PDU, that will be sent.
 *
 * @return Zero on success, error on fail.
 */
static int modbus_tcp_send_packet(void *_mbt,
                           int _slave_addr,
                           emb_const_pdu_t *_pdu) {

    /*struct emb_tcp_t* mbt = (struct emb_tcp_t*)_mbt;

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
        return -modbus_buffer_overflow;*/
}

void emb_tcp_initialize(struct emb_tcp_t* _mbt) {
    _mbt->proto.send_packet = modbus_tcp_send_packet;
    _mbt->proto.low_level_context = _mbt;
}

void emb_tcp_port_event(struct emb_tcp_t* _mbt,
                        enum emb_tcp_port_event_t _event) {

    switch(_event) {
    case emb_tcp_data_received_event:
//        _mbt->rx_buf_counter += _mbt->read_from_port(_mbt,
//                                                     _mbt->rx_buffer + _mbt->rx_buf_counter,
//                                                     _mbt->rx_buf_size - _mbt->rx_buf_counter);
//        _mbt->emb_tcp_on_char(_mbt);
        break;

    case emb_tcp_tx_buf_empty_event:
//        _mbt->tx_buf_counter += _mbt->write_to_port(_mbt,
//                                                    _mbt->tx_buffer + _mbt->tx_buf_counter,
//                                                    _mbt->tx_pkt_size - _mbt->tx_buf_counter);
        break;
    }
}
