
#include <emodbus/transport/rtu.h>
#include <emodbus/base/modbus_transport.h>
#include <emodbus/base/common.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/transport/add/crc.h>
#include <emodbus/transport/add/simple-crc.h>
#include <emodbus/base/modbus_errno.h>

/*!
 * \file
 * \brief RTU Transport.
 *
 * This file contains an RTU transport realizations.
 *
 */

/**
 * @brief RTU Encode packet
 *
 * Encodes packet to sending over RTU protocol.
 *
 * @param [in] _given_data All neccessary data to encode
 * @param [out] _packet A place to store packet
 * @param [in] _pkt_size A size of the place to store packet
 *
 * @return number bytes to send if no errors, otherwise negative value.
 */

int emb_rtu_encode_packet(const struct emb_transport_info_t* _given_data,
                          uint8_t* _packet,
                          unsigned int _pkt_size)
{
    if(!_given_data || !_given_data->pdu || !_packet || _pkt_size == 0)
        return -EINVAL;

    if((_given_data->pdu->data_size + 4) <= _pkt_size) {

        emb_const_pdu_t* pdu = MB_CONST_PDU(_given_data->pdu);

        const unsigned int sz = pdu->data_size + 2;
        uint16_t crc;
        uint8_t* p;

        _packet[0] = _given_data->server_id;
        _packet[1] = pdu->function;

        if(_given_data->flags & EMB_RTU_DO_DATA_COPY)
            memcpy(_packet + 2, pdu->data, pdu->data_size);

        crc = EMB_RTU_CRC_FUNCTION(_packet, (uint16_t)sz);
        p = _packet + sz;
        LIT_END_MK16(p, crc);

        return (int)sz + 2;
    }
    else
        return -modbus_buffer_overflow;
}

/**
 * @brief RTU Decode packet
 *
 * Decodes (parses) a previously received packet
 *
 * @param [in] _packet All neccessary data to encode
 * @param [in] _pkt_size A place to store packet
 * @param [out] _result A size of the place to store packet
 *
 * @return 0 if no errors, otherwise negative value.
 */

int emb_rtu_decode_packet(const uint8_t* _packet,
                          unsigned int _pkt_size,
                          struct emb_transport_info_t* _result)
{
    if(!_result || !_result->pdu || !_packet || _pkt_size == 0)
        return -EINVAL;

    if(_pkt_size >= 4) {
        const unsigned int size = _pkt_size - 2;
        const uint16_t crc1 = EMB_RTU_CRC_FUNCTION(_packet, (uint16_t)size);
        const uint16_t crc2 = (uint16_t)MKWORD(_packet[size], _packet[size+1]);
        const unsigned int data_sz = size - 2;

        if(crc1 != crc2)
            return -modbus_bad_crc;

        if(data_sz > (_result->pdu->max_size))
            return -modbus_buffer_overflow;

        _result->server_id = _packet[0];
        _result->transaction_id = 0;

        _result->pdu->function = _packet[1];
        _result->pdu->data_size = (uint8_t)size - 2;
        if(_result->flags & EMB_RTU_DO_DATA_COPY)
            memcpy(_result->pdu->data, _packet + 2, data_sz);

        return 0;
    }
    else
        return -EINVAL;
}
