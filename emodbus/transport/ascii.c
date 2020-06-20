
#include <emodbus/transport/ascii.h>
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
 * \brief ASCII Transport.
 *
 * This file contains an ASCII transport realization.
 *
 */

static uint8_t getDigit(uint8_t digit)
{
    uint8_t retVal = 0;

    if ((digit >= '0') && (digit <= '9'))
    {
       retVal = digit -  '0';
    }
    else if ((digit >= 'A') && (digit <= 'F'))
    {
       retVal = digit - 'A' + 10;
    }
    else if ((digit >= 'a') && (digit <= 'f'))
    {
       retVal = digit - 'a' + 10;
    }
    else
    {
        printf("Wrong char\n");
    }

    return retVal;
}

static uint8_t ascii2bin(uint8_t hi, uint8_t lo)
{
    return (getDigit(hi) << 4) | getDigit(lo);
}

static uint8_t calc_crc(const uint8_t* _packet, unsigned int _size)
{
    uint8_t result = 0;
    unsigned int i;
    for(i=0; i<_size; i += 2)
        result -= ascii2bin(_packet[i], _packet[i+1]);
    return result;
}

static void bin2ascii(uint8_t _bin, uint8_t* _ascii)
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
        bin2ascii(_adu->server_id, iterator);
        iterator += 2;
        bin2ascii(_adu->pdu.function, iterator);
        iterator += 2;
        for(i=0; i<_adu->pdu.data_size; ++i) {
            bin2ascii(((uint8_t*)_adu->pdu.data)[i], iterator);
            iterator += 2;
        }

        bin2ascii(calc_crc(_packet + 1, 2+2+_adu->pdu.data_size*2), iterator);
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

    _result->server_id = ascii2bin(_packet[1], _packet[2]);
    _result->transaction_id = 0;
    _result->pdu.function = ascii2bin(_packet[3], _packet[4]);

    crc = ascii2bin(_packet[_pkt_size-4], _packet[_pkt_size-3]);

    if(crc != calc_crc(_packet+1, (_pkt_size-1-2-2)))
        return -modbus_bad_crc;

    for(i=0; i<data_size; ++i) {
        const uint8_t v = ascii2bin(_packet[5+i*2], _packet[5+i*2+1]);
        ((uint8_t*)_result->pdu.data)[i] = v;
    }

    _result->pdu.data_size = data_size;

    return 0;
}
