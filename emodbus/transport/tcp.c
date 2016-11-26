
#include <emodbus/transport/tcp.h>
#include <emodbus/base/modbus_transport.h>
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

#define read_data_from_port(_mbt_)  emb_tcp_port_event((_mbt_), (_mbt_)->tcp_client_id, emb_tcp_data_received_event)
#define write_data_to_port(_mbt_)   emb_tcp_port_event((_mbt_), (_mbt_)->tcp_client_id, emb_tcp_tx_buf_empty_event)

static int parse_packet(struct emb_tcp_t* _mbt) {

    _mbt->curr_rx_length = -1;

    // TODO: Maybe I need to loop-based searching in receive buffer ?
    if(_mbt->rx_pkt_counter >= (emb_tcp_mbap_size + 2)) {
        struct emb_tcp_mbap_t* mbap = (struct emb_tcp_mbap_t*)_mbt->rx_buf;
        const uint16_t pkt_length = SWAP_BYTES(mbap->length) - 1;

        _mbt->curr_rx_length = (emb_tcp_mbap_size + pkt_length) - _mbt->rx_pkt_counter;

        // If we are received not all packet, then skip processing, and
        // wait for data remainder.
        if(_mbt->curr_rx_length > 0)
            return 0;

#if EMODBUS_PACKETS_DUMPING
        if(_mbt->transport.flags & EMB_TRANSPORT_FLAG_DUMD_PAKETS)
            if(emb_dump_rx_data)
                emb_dump_rx_data(_mbt->rx_buf, _mbt->rx_pkt_counter);
#endif // EMODBUS_PACKETS_DUMPING

        do {
            // If we are client's side, then we need to check a transaction id.
            // The transaction id must be the same as in sent packet.
            if(!(_mbt->transport.flags & EMB_TRANSPORT_FLAG_IS_SERVER)) {
                struct emb_tcp_mbap_t* tx_mbap = (struct emb_tcp_mbap_t*)_mbt->tx_buf;
                if(tx_mbap->transact_id != mbap->transact_id) {
                    emb_transport_error(&_mbt->transport, -modbus_resp_wrong_transaction_id);
                    break;
                }
            }
            if(_mbt->transport.rx_pdu) {
                emb_pdu_t* const rx_pdu = _mbt->transport.rx_pdu;
                const int pkt_data_length = pkt_length - 1;
                if(rx_pdu->max_size < pkt_data_length) {
                    emb_transport_error(&_mbt->transport, -modbus_resp_buffer_ovf);
                    break;
                }
                rx_pdu->function = _mbt->rx_buf[emb_tcp_mbap_size];
                rx_pdu->data_size = pkt_data_length;
                memcpy(rx_pdu->data, _mbt->rx_buf + (emb_tcp_mbap_size+1), pkt_data_length);
                emb_transport_recv_packet(&_mbt->transport, mbap->unit_id, MB_CONST_PDU(rx_pdu));
            }

        } while(0);
        return 1;
    }
    return 0;
}

/**
 * @brief Send a PDU.
 *
 * (Transport interface) This function will send packet (CRC suffix automatically added).
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

        // If we are server, then take a transaction-id from request,
        // if we are client, then transaction-id is a number, increased by 1 per transaction.
        if(mbt->transport.flags & EMB_TRANSPORT_FLAG_IS_SERVER) {
            mbap->transact_id = ((struct emb_tcp_mbap_t*)mbt->rx_buf)->transact_id;
        }
        else {
            uint16_t prev_transaction_id = SWAP_BYTES(mbap->transact_id);
            ++prev_transaction_id;
            mbap->transact_id = SWAP_BYTES(prev_transaction_id);
        }

        mbap->proto_id = SWAP_BYTES(0);
        mbap->length = SWAP_BYTES(length);
        mbap->unit_id = _slave_addr;

        mbt->tx_buf[emb_tcp_mbap_size] = _pdu->function;

        memcpy((mbt->tx_buf + emb_tcp_mbap_size+1), _pdu->data, _pdu->data_size);

        mbt->tx_pkt_size = length + 6;
        mbt->tx_pkt_counter = 0;

#if EMODBUS_PACKETS_DUMPING
        if(mbt->transport.flags & EMB_TRANSPORT_FLAG_DUMD_PAKETS)
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
    if(_mbt) {
        struct emb_tcp_mbap_t* mbap = (struct emb_tcp_mbap_t*)_mbt->tx_buf;
        // Set first transaction_id to zero.
        mbap->transact_id = SWAP_BYTES(0);
        _mbt->transport.send_packet = modbus_tcp_send_packet;
        _mbt->transport.transport_context = _mbt;
        _mbt->curr_rx_length = -1;
    }
}

void emb_tcp_port_event(struct emb_tcp_t* _mbt,
                        void *_tcp_client_id,
                        enum emb_tcp_port_event_t _event) {

    int result;
    _mbt->tcp_client_id = _tcp_client_id;

    switch(_event) {
    case emb_tcp_data_received_event:
        {
            int to_be_read = emb_tcp_rx_buf_size - _mbt->rx_pkt_counter;

            if(to_be_read < _mbt->curr_rx_length) {
                //printf("Cleaning by invalid curr_rx_length: (%d < %d)\n",
                      // to_be_read, _mbt->curr_rx_length);
                //fflush(stdout);
                _mbt->rx_pkt_counter = 0;
                return;
            }

            if(_mbt->curr_rx_length >= 0)
                to_be_read = _mbt->curr_rx_length;

            result = _mbt->read_from_port(_mbt,
                                          _mbt->rx_buf + _mbt->rx_pkt_counter,
                                          to_be_read);

//            printf("read_from_port = %d, rx_pkt_counter = %d,v to_be_read = %d\n",
//                   result, _mbt->rx_pkt_counter, to_be_read);
            fflush(stdout);

            if(result >= 0) {
                _mbt->rx_pkt_counter += result;

                if(parse_packet(_mbt)) {
                    _mbt->rx_pkt_counter = 0;
                }
                else {
                    if(emb_tcp_rx_buf_size == _mbt->rx_pkt_counter) {
                        _mbt->rx_pkt_counter = 0;
                       // printf("Cleaning by buffer overflow\n"); fflush(stdout);
                    }
                }
            }
        }
        break;

    case emb_tcp_tx_buf_empty_event:
        result = _mbt->write_to_port(_mbt,
                                     _mbt->tx_buf + _mbt->tx_pkt_counter,
                                     _mbt->tx_pkt_size - _mbt->tx_pkt_counter);
        if(result > 0) {
            _mbt->tx_pkt_counter += result;
        }
        break;
    case emb_tcp_last_rx_timeout:
        // By last rx timeout we are clean the rx buffer
//        if(_mbt->rx_pkt_counter) {
//            printf("Cleaning by timer (_mbt->rx_pkt_counter = %d)\n",
//                   _mbt->rx_pkt_counter);
//            fflush(stdout);
//        }
        _mbt->curr_rx_length = -1;
        _mbt->rx_pkt_counter = 0;
        break;
    }
}
