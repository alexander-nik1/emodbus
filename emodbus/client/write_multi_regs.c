
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/write_multi_regs.h>
#include <emodbus/base/calc_pdu_size.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/limits.h>

/*!
 * \file
 * \brief Definition of Write Multiple Registers functions.
 *
 * Functions for working with Write Multiple Registers function.
 *
 */

int emb_write_regs_calc_req_data_size(uint16_t _quantity) {
    return WRITE_REGISTERS_REQ_SIZE(_quantity);
}

int emb_write_regs_calc_answer_data_size()
{
    return WRITE_REGISTERS_ANS_SIZE();
}

int emb_write_regs_make_req(emb_pdu_t *_result_req,
                            uint16_t _address,
                            uint16_t _quantity,
                            const uint16_t* _data)
{
    if(!(_result_req && _data))
        return -modbus_invalid_argument;

    if(_result_req->max_size < emb_write_regs_calc_req_data_size(_quantity))
        return -modbus_invalid_argument;

    if(EMB_WRITE_REGS_MIN_QUANTITY <= _quantity && _quantity <= EMB_WRITE_REGS_MAX_QUANTITY) {

        int i;
        uint8_t* data_addr = _result_req->data;

        _result_req->function = 0x10;
        _result_req->data_size = (uint8_t)WRITE_REGISTERS_REQ_SIZE(_quantity);

        *data_addr++ = (uint8_t)(_address >> 8);
        *data_addr++ = (uint8_t)_address;

        *data_addr++ = (uint8_t)(_quantity >> 8);
        *data_addr++ = (uint8_t)_quantity;

        *data_addr++ = (uint8_t)(_quantity * 2);

        for(i=0; i<_quantity; ++i) {
            const uint16_t data = _data[i];
            *data_addr++ = (uint8_t)(data >> 8);
            *data_addr++ = (uint8_t)data;
        }

        return modbus_success;
    }
    else
        return -modbus_invalid_argument;
}

int emb_write_regs_get_req_address(emb_const_pdu_t* _req)
{
    if(_req && _req->data && _req->data_size >= 2) {
        uint16_t x = _req->data[0];
        x <<= 8;
        x |= _req->data[1];
        return x;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_write_regs_get_req_quantity(emb_const_pdu_t* _req)
{
    if(_req && _req->data && _req->data_size >= 4) {
        uint16_t x = _req->data[2];
        x <<= 8;
        x |= _req->data[3];
        return x;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_write_regs_get_req_data(emb_const_pdu_t* _req, uint16_t _offset)
{
    const unsigned int byte_off = _offset * 2 + 5;
    if(_req && _req->data && _req->data_size >= (byte_off + 1)) {
        uint16_t x = _req->data[byte_off];
        x <<= 8;
        x |= _req->data[byte_off + 1];
        return x;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_write_regs_get_answer_address(emb_const_pdu_t* _answer)
{
    if(_answer && _answer->data && _answer->data_size >= 2) {
        uint16_t x = _answer->data[0];
        x <<= 8;
        x |= _answer->data[1];
        return x;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_write_regs_get_answer_quantity(emb_const_pdu_t* _answer)
{
    if(_answer && _answer->data && _answer->data_size >= 4) {
        uint16_t x = _answer->data[2];
        x <<= 8;
        x |= _answer->data[3];
        return x;
    }
    else {
        return -modbus_invalid_argument;
    }
}
