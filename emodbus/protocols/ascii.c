
#include <emodbus/protocols/ascii.h>
#include <emodbus/base/common.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <stdint.h>

/*!
 * \file
 * \brief ASCII Transport.
 *
 * This file contains an ASCII transport realization.
 *
 */

static uint8_t _get_digit(uint8_t digit, int* _errors)
{
    if ((digit >= '0') && (digit <= '9')) {
       return digit -  '0';
    }
    else if ((digit >= 'A') && (digit <= 'F')) {
       return digit - 'A' + 10;
    }
    else if ((digit >= 'a') && (digit <= 'f')) {
       return digit - 'a' + 10;
    }
    else {
        if(_errors)
            ++(*_errors);
    }
    return 0;
}

static uint8_t _ascii2bin(const uint8_t* _ascii, int* _errors)
{
    return (uint8_t)(_get_digit(_ascii[0], _errors) << 4) | _get_digit(_ascii[1], _errors);
}

static uint8_t _calc_crc(const uint8_t* _packet, unsigned int _size, int* _errors)
{
    uint8_t result = 0;
    unsigned int i;
    for(i=0; i<_size; i += 2)
        result -= _ascii2bin(_packet + i, _errors);
    return result;
}

static void _bin2ascii(uint8_t _bin, uint8_t* _ascii)
{
    static const uint8_t digits[] = "0123456789ABCDEF";
    _ascii[0] = digits[_bin >> 4];
    _ascii[1] = digits[_bin & 0xF];
}

enum { CR = 0x0D };
enum { LF = 0x0A };

int emb_ascii_encode_packet(const emb_adu_t* _adu,
                            uint8_t* _packet,
                            unsigned int _max_pkt_size)
{
    int pkt_size;

    if(!_adu || !_packet || _max_pkt_size == 0)
        return -modbus_invalid_argument;

    pkt_size = _adu->pdu.data_size * 2 + 1 + 2 + 2 + 2 + 2;

    if(pkt_size <= (int)_max_pkt_size) {
        unsigned int i;
        uint8_t* iterator = _packet;

        *iterator++ = ':';
        _bin2ascii(_adu->server_id, iterator);
        iterator += 2;
        _bin2ascii(_adu->pdu.function, iterator);
        iterator += 2;
        for(i=0; i<_adu->pdu.data_size; ++i) {
            _bin2ascii(((uint8_t*)_adu->pdu.data)[i], iterator);
            iterator += 2;
        }

        _bin2ascii(_calc_crc(_packet + 1, 2 + 2 + _adu->pdu.data_size * 2, NULL), iterator);
        iterator += 2;

        *iterator++ = CR;
        *iterator++ = LF;

        return pkt_size;
    }
    else {
        return -modbus_buffer_overflow;
    }
}

int emb_ascii_decode_packet(const uint8_t* _packet,
                            unsigned int _pkt_size,
                            emb_adu_t* _result)
{
    uint8_t data_size;
    uint8_t crc;
    unsigned int i;
    int errors = 0;

    if(!_result || !_packet || _pkt_size == 0)
        return -modbus_invalid_argument;

    data_size = (uint8_t)(_pkt_size - 1 - 2 - 2 - 2 - 2) / 2;

    if(data_size > _result->pdu.max_size)
        return -modbus_buffer_overflow;

    if(_pkt_size < 10)
        return -modbus_invalid_argument;

    if(_packet[0] != ':')
        return -modbus_invalid_packet_format;

    if(!(_packet[_pkt_size-2] == CR && _packet[_pkt_size-1] == LF))
        return -modbus_invalid_packet_format;

    _result->server_id = _ascii2bin(_packet + 1, &errors);
    _result->transaction_id = 0;
    _result->pdu.function = _ascii2bin(_packet + 3, &errors);

    crc = _ascii2bin(_packet + (_pkt_size - 4), &errors);

    if(crc != _calc_crc(_packet+1, (_pkt_size-1-2-2), &errors))
        return -modbus_bad_crc;

    for(i=0; i<data_size; ++i) {
        const uint8_t v = _ascii2bin(_packet + (5+i*2), &errors);
        ((uint8_t*)_result->pdu.data)[i] = v;
    }

    if(errors)
        return -modbus_invalid_packet_format;

    _result->pdu.data_size = data_size;

    return 0;
}
