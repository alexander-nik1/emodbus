
#include <emodbus/transport/rtu.h>
#include <emodbus/base/common.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/transport/add/crc.h>
#include <emodbus/transport/add/simple-crc.h>
#include <emodbus/base/modbus_errno.h>
#include <stdint.h>

/*!
 * \file
 * \brief RTU Transport.
 *
 * This file contains an RTU transport realization.
 *
 */

int emb_rtu_encode_packet(const emb_adu_t* _adu,
                          uint8_t* _packet,
                          unsigned int _pkt_size)
{
    if(!_adu || !_packet || _pkt_size == 0)
        return -modbus_invalid_argument;

    if((_adu->pdu.data_size + 4) <= _pkt_size) {

        emb_const_pdu_t* pdu = MB_CONST_PDU(&_adu->pdu);

        const unsigned int sz = pdu->data_size + 2;
        uint16_t crc;
        uint8_t* p;

        _packet[0] = _adu->server_id;
        _packet[1] = pdu->function;

        //if(_adu->flags & EMB_RTU_DO_DATA_COPY)
            memcpy(_packet + 2, pdu->data, pdu->data_size);

        crc = EMB_RTU_CRC_FUNCTION(_packet, (uint16_t)sz);
        p = _packet + sz;
        LIT_END_MK16(p, crc);

        return (int)sz + 2;
    }
    else {
        printf("_adu->pdu->data_size = %d, _pkt_size = %d\n", _adu->pdu.data_size, _pkt_size);
        return -modbus_buffer_overflow;
    }
}

int emb_rtu_decode_packet(const uint8_t* _packet,
                          unsigned int _pkt_size,
                          emb_adu_t* _result)
{
    if(!_result || !_packet || _pkt_size == 0)
        return -modbus_invalid_argument;

    if(_pkt_size >= 4) {
        const unsigned int size = _pkt_size - 2;
        const uint16_t crc1 = EMB_RTU_CRC_FUNCTION(_packet, (uint16_t)size);
        const uint16_t crc2 = (uint16_t)MKWORD(_packet[size], _packet[size+1]);
        const unsigned int data_sz = size - 2;

        if(crc1 != crc2)
            return -modbus_bad_crc;

        if(data_sz > (_result->pdu.max_size))
            return -modbus_buffer_overflow;

        _result->server_id = _packet[0];
        _result->transaction_id = 0;

        _result->pdu.function = _packet[1];
        _result->pdu.data_size = (uint8_t)size - 2;

        //if(_result->flags & EMB_RTU_DO_DATA_COPY)
            memcpy(_result->pdu.data, _packet + 2, data_sz);

        return 0;
    }
    else
        return -1;
}
