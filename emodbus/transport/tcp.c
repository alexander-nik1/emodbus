
#include <emodbus/transport/tcp.h>
#include <emodbus/base/common.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/modbus_xdu.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int emb_tcp_decode_packet(const uint8_t* _packet,
                          unsigned int _pkt_size,
                          emb_adu_t* _result)
{
    const emb_tcp_header_t* hdr;
    uint16_t pdu_length;

    if(!_result || !_packet)
        return -modbus_invalid_argument;

    if(_pkt_size < (sizeof(emb_tcp_header_t) + 2))
        return -modbus_invalid_argument;

    hdr = (const emb_tcp_header_t*)_packet;
    pdu_length = SWAP_BYTES(hdr->length) - 1;

    if(_pkt_size != (sizeof(emb_tcp_header_t) + pdu_length))
        return -modbus_invalid_argument;

    if(hdr->proto_id != 0x0000)
        return -modbus_invalid_proto_id;

    _result->pdu.data_size = (uint8_t)pdu_length - 1;

    if(_result->pdu.max_size < _result->pdu.data_size)
        return -modbus_buffer_overflow;

    _result->transaction_id = SWAP_BYTES(hdr->transact_id);
    _result->server_id = hdr->unit_id;
    _result->pdu.function = _packet[emb_tcp_header_size];
//    if(_result->flags & EMB_TCP_DO_DATA_COPY)
        memcpy(_result->pdu.data, _packet + (emb_tcp_header_size+1), _result->pdu.data_size);
    return 0;
}

int emb_tcp_encode_packet(const emb_adu_t *_adu,
                          uint8_t* _packet,
                          unsigned int _pkt_size)
{
    emb_tcp_header_t* hdr;
    uint16_t length;

    if(!_adu || !_packet || _pkt_size < sizeof(emb_tcp_header_t))
        return -modbus_invalid_argument;

    hdr = (emb_tcp_header_t*)_packet;
    hdr->transact_id = SWAP_BYTES(_adu->transaction_id);
    hdr->proto_id = 0x0000;
    length = _adu->pdu.data_size + 2;
    hdr->length = SWAP_BYTES(length);
    hdr->unit_id = _adu->server_id;

    length = sizeof(emb_tcp_header_t) + _adu->pdu.data_size + 1;

    if(_pkt_size < length)
        return -modbus_buffer_overflow;

    _packet += sizeof(emb_tcp_header_t);

    *_packet++ = _adu->pdu.function;

//    if(_adu->flags & EMB_TCP_DO_DATA_COPY)
        memcpy(_packet, _adu->pdu.data, _adu->pdu.data_size);

    return length;
}



