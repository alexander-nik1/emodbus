
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <emodbus/base/common.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/client/write_mask_reg.h>
#include <emodbus/base/calc_pdu_size.h>

/*!
 * \file
 * \brief The definition of "Mask Write Register" functions.
 *
 * Functions for working with "Mask Write Register" function.
 *
 */

int emb_write_mask_reg_calc_req_data_size() {
    return MASK_REGISTER_REQ_SIZE();
}

int emb_write_mask_reg_calc_answer_data_size() {
    return MASK_REGISTER_ANS_SIZE();
}

int emb_write_mask_reg_make_req(emb_pdu_t* _result_req,
                                uint16_t _address,
                                uint16_t _and_mask,
                                uint16_t _or_mask) {

    if(!_result_req)
        return -modbus_invalid_argument;

    if(_result_req->max_size < emb_write_mask_reg_calc_req_data_size())
        return -modbus_invalid_argument;

    _result_req->data_size = MASK_REGISTER_REQ_SIZE();
    _result_req->function = 0x16;

    _result_req->data[0] = (uint8_t)(_address >> 8);
    _result_req->data[1] = (uint8_t)_address;

    _result_req->data[2] = (uint8_t)(_and_mask >> 8);
    _result_req->data[3] = (uint8_t)_and_mask;

    _result_req->data[4] = (uint8_t)(_or_mask >> 8);
    _result_req->data[5] = (uint8_t)_or_mask;

    return modbus_success;
}

int emb_write_mask_reg_get_address(emb_const_pdu_t* _pdu)
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

int emb_write_mask_reg_get_and_mask(emb_const_pdu_t* _pdu)
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

int emb_write_mask_reg_get_or_mask(emb_const_pdu_t* _pdu)
{
    if(_pdu && _pdu->data && _pdu->data_size >= 6) {
        uint16_t x = _pdu->data[4];
        x <<= 8;
        x |= _pdu->data[5];
        return x;
    }
    else {
        return -modbus_invalid_argument;
    }
}
