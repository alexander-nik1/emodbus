
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/read_bits.h>
#include <emodbus/base/calc_pdu_size.h>
#include <emodbus/base/modbus_errno.h>

/*!
 * \file
 * \brief The realization of Read Coils and Read Discrete Inputs functions.
 *
 * Functions for working with Read Coils and Read Discrete Inputs function.
 *
 */

int emb_read_bits_calc_req_data_size() {
    return READ_COILS_REQ_SIZE();
}

int emb_read_bits_calc_answer_data_size(uint16_t _quantity) {
    if( 1 <= _quantity && _quantity <= EMB_READ_BITS_MAX_QUANTITY ) {
        const uint8_t bytes_count = (uint8_t)((_quantity / 8) + ((_quantity & 7) ? 1 : 0));
        return READ_COILS_ANS_SIZE(bytes_count);
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_read_bits_make_req(emb_pdu_t* _result_req,
                           enum EMB_RB_TYPE _type,
                           uint16_t _starting_address, uint16_t _quantity) {
    if (!_result_req)
        return -modbus_invalid_argument;

    if (_result_req->max_size < emb_read_bits_calc_req_data_size())
        return -modbus_buffer_overflow;

    if ( 1 <= _quantity && _quantity <= EMB_READ_BITS_MAX_QUANTITY ) {
        BIG_END_MK16(_result_req->data, _starting_address);
        BIG_END_MK16(_result_req->data+2, _quantity);
        _result_req->data_size = (uint8_t)emb_read_bits_calc_req_data_size();
        switch(_type) {
        case EMB_RB_COILS:
            _result_req->function = 0x01;
            break;
        case EMB_RB_DISCRETE_INPUTS:
            _result_req->function = 0x02;
            break;
        default:
            return -modbus_invalid_argument;
        }
        return modbus_success;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_read_bits_get_starting_addr(emb_const_pdu_t *_req) {
    if (_req && _req->data_size > 1) {
        return GET_BIG_END16(_req->data);
    }
    return -modbus_invalid_argument;
}

int emb_read_bits_get_quantity(emb_const_pdu_t *_req) {
    if (_req && _req->data_size > 3) {
        return GET_BIG_END16(_req->data);
    }
    return -modbus_invalid_argument;
}

int emb_read_bits_get_bit(emb_const_pdu_t *_answer,
                          uint16_t _coil_offset) {
    if (_answer) {
        const uint16_t boff = _coil_offset / 8 + 1;
        if (boff < _answer->data_size) {
            const uint8_t byte = ((const uint8_t*)(_answer->data))[boff];
            return (byte >> (_coil_offset & 7)) & 1;
        }
    }
    return -modbus_invalid_argument;
}

int emb_read_bits_get_byte(emb_const_pdu_t* _answer,
                           uint8_t _byte_offset) {
    if (_answer) {
        const uint8_t boff = _byte_offset + 1;
        if(boff < _answer->data_size) {
            return ((const uint8_t*)(_answer->data))[boff];
        }
    }
    return -modbus_invalid_argument;
}

int emb_read_bits_get_answ_bytes_count(emb_const_pdu_t* _answer) {
    if (!_answer || _answer->data_size < 1)
        return -modbus_invalid_argument;
    return _answer->data[0];
}

int emb_read_bits_get_answ_data(emb_const_pdu_t* _answer,
                                const uint8_t** _ptr)
{
    if (_answer && _ptr && _answer->data_size > 1) {
        *_ptr = &_answer->data[1];
        return modbus_success;
    }
    return -modbus_invalid_argument;
}
