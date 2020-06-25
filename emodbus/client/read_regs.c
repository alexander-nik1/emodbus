
#include <string.h>

#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/read_regs.h>
#include <emodbus/base/calc_pdu_size.h>
#include <emodbus/base/limits.h>

/*!
 * \file
 * \brief The realization of Read Holding Registers functions.
 *
 * Functions for working with Read Holding Registers function.
 *
 */

int emb_read_regs_calc_req_data_size()
{
    return READ_HOLDINGS_REQ_SIZE();
}

int emb_read_regs_calc_answer_data_size(uint16_t _quantity)
{
    if( EMB_READ_REGS_MIN_QUANTITY <= _quantity && _quantity <= EMB_READ_REGS_MAX_QUANTITY ) {
        return READ_HOLDINGS_ANS_SIZE(_quantity);
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_read_regs_make_req(emb_pdu_t* _result_req,
                           enum EMB_RR_TYPE _type,
                           uint16_t _starting_address,
                           uint16_t _quantity)
{
    if(_result_req->max_size < emb_read_regs_calc_req_data_size()) {
        return -modbus_buffer_overflow;
    }

    if( EMB_READ_REGS_MIN_QUANTITY <= _quantity && _quantity <= EMB_READ_REGS_MAX_QUANTITY ) {
        uint16_t tmp;

        tmp = SWAP_BYTES(_starting_address);
        memcpy(_result_req->data, &tmp, sizeof(uint16_t));

        tmp = SWAP_BYTES(_quantity);
        memcpy(_result_req->data + sizeof(uint16_t), &tmp, sizeof(uint16_t));

        _result_req->data_size = 4;
        switch(_type) {
        case EMB_RR_HOLDINGS:
            _result_req->function = 3;
            break;
        case EMB_RR_INPUTS:
            _result_req->function = 4;
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

int emb_read_regs_get_req_starting_addr(emb_const_pdu_t *_req)
{
    if(_req && _req->data && _req->data_size >= 2) {
        uint16_t t = _req->data[0];
        t <<= 8;
        t |= _req->data[1];
        return t;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_read_regs_get_req_quantity(emb_const_pdu_t *_req)
{
    if(_req && _req->data && _req->data_size >= 4) {
        uint16_t t = _req->data[2];
        t <<= 8;
        t |= _req->data[3];
        return t;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_read_regs_get_ans_reg(emb_const_pdu_t *_answer,
                                   uint16_t _reg_offset)
{
    if(_answer && _answer->data) {
        if(_answer->data_size > (_reg_offset * 2 + 1)) {
            const unsigned int byte_offset = _reg_offset * 2;
            uint16_t t = _answer->data[byte_offset + 1];
            t <<= 8;
            t |= _answer->data[byte_offset + 2];
            return t;
        }
        else {
            return -modbus_buffer_overflow;
        }
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_read_regs_get_ans_regs(emb_const_pdu_t* _answer,
                                 uint16_t _reg_offset,
                                 uint16_t _n_regs,
                                 uint16_t* _p_data)
{
    uint16_t i;
    if(_answer && _answer->data && _p_data) {
        if(_answer->data_size >= ((_reg_offset + _n_regs) * 2 + 1)) {
            for(i=0; i<_n_regs; ++i) {
                const unsigned int byte_offset = (_reg_offset + i) * 2;
                uint16_t t = _answer->data[byte_offset + 1];
                t <<= 8;
                t |= _answer->data[byte_offset + 2];
                *_p_data++ = t;
            }
            return modbus_success;
        }
        else {
            return -modbus_buffer_overflow;
        }
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_read_regs_get_ans_regs_n(emb_const_pdu_t *_answer)
{
    if(_answer && _answer->data && _answer->data_size >= 1) {
        return _answer->data[0] / 2;
    }
    else {
        return -modbus_invalid_argument;
    }
}
