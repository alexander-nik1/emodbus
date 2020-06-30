
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/read_write_regs.h>
#include <emodbus/base/calc_pdu_size.h>
#include <emodbus/base/limits.h>

/*!
 * \file
 * \brief Definition of Read Write Registers functions.
 *
 * Functions for working with Read Write Registers function.
 *
 */

int emb_rdwr_regs_calc_req_data_size(uint16_t _wr_quantity)
{
    if(!(EMB_RDWR_REGS_MIN_WR_QUANTITY <= _wr_quantity && _wr_quantity <= EMB_RDWR_REGS_MAX_WR_QUANTITY))
        return -modbus_invalid_argument;
    else
        return READ_WRITE_REGS_REQ_SIZE(_wr_quantity);
}

int emb_rdwr_regs_calc_answer_data_size(uint16_t _rd_quantity)
{
    if(!(EMB_RDWR_REGS_MIN_RD_QUANTITY <= _rd_quantity && _rd_quantity <= EMB_RDWR_REGS_MAX_RD_QUANTITY))
        return -modbus_invalid_argument;
    else
        return READ_WRITE_REGS_ANS_SIZE(_rd_quantity);
}

int emb_rdwr_regs_make_req(emb_pdu_t* _result_req,
                           uint16_t _rd_address,
                           uint16_t _rd_quantity,
                           uint16_t _wr_address,
                           uint16_t _wr_quantity,
                           const uint16_t* _wr_data)
{
    int i;
    uint8_t* data_addr;

    if(!(_result_req && _result_req->data && _result_req->max_size && _wr_data))
        return -modbus_invalid_argument;

    if(_result_req->max_size < emb_rdwr_regs_calc_req_data_size(_wr_quantity))
        return -modbus_buffer_overflow;

    if(!(EMB_RDWR_REGS_MIN_RD_QUANTITY <= _rd_quantity && _rd_quantity <= EMB_RDWR_REGS_MAX_RD_QUANTITY))
        return -modbus_invalid_argument;

    if(!(EMB_RDWR_REGS_MIN_WR_QUANTITY <= _wr_quantity && _wr_quantity <= EMB_RDWR_REGS_MAX_WR_QUANTITY))
        return -modbus_invalid_argument;

    _result_req->function = 0x17;
    _result_req->data_size = (uint8_t)emb_rdwr_regs_calc_req_data_size(_wr_quantity);

    _result_req->data[0] = (uint8_t)(_rd_address >> 8);
    _result_req->data[1] = (uint8_t)_rd_address;

    _result_req->data[2] = (uint8_t)(_rd_quantity >> 8);
    _result_req->data[3] = (uint8_t)_rd_quantity;

    _result_req->data[4] = (uint8_t)(_wr_address >> 8);
    _result_req->data[5] = (uint8_t)_wr_address;

    _result_req->data[6] = (uint8_t)(_wr_quantity >> 8);
    _result_req->data[7] = (uint8_t)_wr_quantity;

    _result_req->data[8] = (uint8_t)_wr_quantity * 2;

    data_addr = _result_req->data + 9;

    for(i=0; i<_wr_quantity; ++i) {
        const uint16_t data = _wr_data[i];
        *data_addr++ = (uint8_t)(data >> 8);
        *data_addr++ = (uint8_t)data;
    }

    return 0;
}

int emb_rdwr_regs_get_req_rd_address(emb_const_pdu_t* _req)
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

int emb_rdwr_regs_get_req_rd_quantity(emb_const_pdu_t* _req)
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

int emb_rdwr_regs_get_req_wr_address(emb_const_pdu_t* _req)
{
    if(_req && _req->data && _req->data_size >= 6) {
        uint16_t x = _req->data[4];
        x <<= 8;
        x |= _req->data[5];
        return x;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_rdwr_regs_get_req_wr_quantity(emb_const_pdu_t* _req)
{
    if(_req && _req->data && _req->data_size >= 8) {
        uint16_t x = _req->data[6];
        x <<= 8;
        x |= _req->data[7];
        return x;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_rdwr_regs_get_answ_reg(emb_const_pdu_t* _answer, uint16_t _offset)
{
    const unsigned int byte_off = _offset * 2 + 1;
    if(_answer && _answer->data && _answer->data_size >= (byte_off + 2)) {
        uint16_t x = _answer->data[_offset];
        x <<= 8;
        x |= _answer->data[_offset + 1];
        return x;
    }
    else {
        return -modbus_invalid_argument;
    }
}

int emb_rdwr_regs_get_answ_regs(emb_const_pdu_t* _answer, uint16_t _offset,
                                uint16_t _n_regs, uint16_t* _p_data)
{
    uint16_t i;
    if(_answer && _answer->data && _p_data) {
        if(_answer->data_size >= ((_offset + _n_regs) * 2 + 1)) {
            for(i=0; i<_n_regs; ++i) {
                const unsigned int byte_offset = (_offset + i) * 2;
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

int emb_rdwr_regs_get_answ_regs_n(emb_const_pdu_t* _answer)
{
    if(_answer && _answer->data && _answer->data_size >= 1) {
        return _answer->data[0] / 2;
    }
    else {
        return -modbus_invalid_argument;
    }
}
