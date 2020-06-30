
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/write_single_reg.h>
#include <emodbus/base/calc_pdu_size.h>
#include <emodbus/base/modbus_errno.h>

/*!
 * \file
 * \brief The definition of a "Write Single Register" functions.
 *
 * Functions for working with "Write Single Register" function.
 *
 */

int emb_write_reg_calc_req_data_size() {
    return WRITE_REGISTER_REQ_SIZE();
}

int emb_write_reg_calc_answer_data_size() {
    return WRITE_REGISTER_ANS_SIZE();
}

int emb_write_reg_make_req(emb_pdu_t* _result_req,
                           uint16_t _address,
                           uint16_t _value) {

    if(_result_req->max_size < emb_write_reg_calc_req_data_size()) {
        return -modbus_invalid_argument;
    }

    _result_req->data_size = WRITE_REGISTER_REQ_SIZE();
    _result_req->function = 0x06;

    _result_req->data[0] = (uint8_t)(_address >> 8);
    _result_req->data[1] = (uint8_t)_address;

    _result_req->data[2] = (uint8_t)(_value >> 8);
    _result_req->data[3] = (uint8_t)_value;


    return modbus_success;
}

int emb_write_reg_get_address(emb_const_pdu_t* _pdu)
{
    if(_pdu && _pdu->data && _pdu->data_size >= 2) {
        uint16_t x = _pdu->data[0];
        x <<= 8;
        x |= _pdu->data[1];
        return x;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_write_reg_get_value(emb_const_pdu_t* _pdu)
{
    if(_pdu && _pdu->data && _pdu->data_size >= 4) {
        uint16_t x = _pdu->data[2];
        x <<= 8;
        x |= _pdu->data[3];
        return x;
    }
    else {
        return -modbus_invalid_argument;
    }
}
