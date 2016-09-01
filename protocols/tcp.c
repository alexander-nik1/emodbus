
#include <emodbus/protocols/tcp.h>
#include <emodbus/base/modbus_proto.h>
#include <emodbus/base/common.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct emb_tcp_mbap_t {
    uint16_t transact_id;
    uint16_t proto_id;
    uint16_t length;
    uint8_t unit_id;
} __attribute__ ((packed));

#define read_data_from_port(_mbt_)  emb_tcp_port_event((_mbt_), emb_tcp_data_received_event)
#define write_data_to_port(_mbt_)   emb_tcp_port_event((_mbt_), emb_tcp_tx_buf_empty_event)

static void parse_packet(struct emb_tcp_t* _mbt) {

    // TODO: Maybe I need to loop-based searching in receive buffer ?
    if(_mbt->rx_pkt_counter > (emb_tcp_mbap_size + 2)) {
        struct emb_tcp_mbap_t* mbap = (struct emb_tcp_mbap_t*)_mbt->rx_buf;
        // TODO Check here transaction id
        const uint16_t pkt_length = SWAP_BYTES(mbap->length) - 1;
        if(_mbt->rx_pkt_counter >= emb_tcp_mbap_size + pkt_length) {
            emb_pdu_t* const rx_pdu = _mbt->proto.rx_pdu;
#if EMODBUS_PACKETS_DUMPING
            if(_mbt->proto.flags & EMB_PROTO_FLAG_DUMD_PAKETS)
                if(emb_dump_rx_data)
                    emb_dump_rx_data(_mbt->rx_buf, _mbt->rx_pkt_counter);
#endif // EMODBUS_PACKETS_DUMPING
            if(rx_pdu) {
                const int pkt_data_length = pkt_length - 1;
                if(rx_pdu->max_size < pkt_data_length) {
                    emb_proto_error(&_mbt->proto, -modbus_resp_buffer_ovf);
                }
                else {
                    rx_pdu->function = _mbt->rx_buf[emb_tcp_mbap_size];
                    rx_pdu->data_size = pkt_data_length;
                    memcpy(rx_pdu->data, _mbt->rx_buf + (emb_tcp_mbap_size+1), pkt_data_length);
                    emb_proto_recv_packet(&_mbt->proto, mbap->unit_id, MB_CONST_PDU(rx_pdu));
                }
            }
            _mbt->rx_pkt_counter = 0;
        }
    }
}

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

    struct emb_tcp_t* mbt = (struct emb_tcp_t*)_mbt;

    if(_pdu->data_size <= MAX_PDU_DATA_SIZE) {

        struct emb_tcp_mbap_t* mbap = (struct emb_tcp_mbap_t*)mbt->tx_buf;

        const uint16_t length = _pdu->data_size + 2;

        mbap->transact_id = SWAP_BYTES(0); // TODO Set here transaction id
        mbap->proto_id = SWAP_BYTES(0);
        mbap->length = SWAP_BYTES(length);
        mbap->unit_id = _slave_addr;

        mbt->tx_buf[emb_tcp_mbap_size] = _pdu->function;

        memcpy((mbt->tx_buf + emb_tcp_mbap_size+1), _pdu->data, _pdu->data_size);

        mbt->tx_pkt_size = length + 6;
        mbt->tx_pkt_counter = 0;

#if EMODBUS_PACKETS_DUMPING
        if(mbt->proto.flags & EMB_PROTO_FLAG_DUMD_PAKETS)
            if(emb_dump_tx_data)
                emb_dump_tx_data(mbt->tx_buf, mbt->tx_pkt_size);
#endif // EMODBUS_PACKETS_DUMPING
        write_data_to_port(mbt);
        return 0;
    }
    else
        return -modbus_buffer_overflow;
}

void emb_tcp_initialize(struct emb_tcp_t* _mbt) {
    _mbt->proto.send_packet = modbus_tcp_send_packet;
    _mbt->proto.low_level_context = _mbt;
}

void emb_tcp_port_event(struct emb_tcp_t* _mbt,
                        enum emb_tcp_port_event_t _event) {

    switch(_event) {
    case emb_tcp_data_received_event:
        _mbt->rx_pkt_counter += _mbt->read_from_port(_mbt,
                                                     _mbt->rx_buf + _mbt->rx_pkt_counter,
                                                     emb_tcp_rx_buf_size - _mbt->rx_pkt_counter);
        parse_packet(_mbt);
        break;

    case emb_tcp_tx_buf_empty_event:
        _mbt->tx_pkt_counter += _mbt->write_to_port(_mbt,
                                                    _mbt->tx_buf + _mbt->tx_pkt_counter,
                                                    _mbt->tx_pkt_size - _mbt->tx_pkt_counter);
        break;
    }
}
