
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/read_write_regs.h>
#include <emodbus/base/calc_pdu_size.h>

/*!
 * \file
 * \brief Definition of Read Write Registers functions.
 *
 * Functions for working with Read Write Registers function.
 *
 */

int emb_rdwr_regs_calc_req_data_size(uint16_t _wr_quantity)
{
    return READ_WRITE_REGS_REQ_SIZE(_wr_quantity);
}

int emb_rdwr_regs_calc_answer_data_size(uint16_t _rd_quantity)
{
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
    uint16_t* data_addr;

    if(!_result_req || !_wr_data)
        return -EINVAL;

    if(_result_req->max_size < emb_rdwr_regs_calc_req_data_size(_wr_quantity))
        return -ENOMEM;

    if(!(1 <= _rd_quantity && _rd_quantity <= 0x007D))
        return -EINVAL;

    if(!(1 <= _wr_quantity && _wr_quantity <= 0x0079))
        return -EINVAL;

    _result_req->function = 0x17;
    _result_req->data_size = emb_rdwr_regs_calc_req_data_size(_wr_quantity);

    ((uint16_t*)_result_req->data)[0] = SWAP_BYTES(_rd_address);
    ((uint16_t*)_result_req->data)[1] = SWAP_BYTES(_rd_quantity);
    ((uint16_t*)_result_req->data)[2] = SWAP_BYTES(_wr_address);
    ((uint16_t*)_result_req->data)[3] = SWAP_BYTES(_wr_quantity);
    ((uint8_t*)_result_req->data)[8] = _wr_quantity * 2;

    data_addr = (uint16_t*)(((uint8_t*)_result_req->data) + 9);

    for(i=0; i<_wr_quantity; ++i) {
        const uint16_t data = ((uint16_t*)_wr_data)[i];
        data_addr[i] = SWAP_BYTES(data);
    }

    return 0;
}

uint16_t emb_rdwr_regs_get_req_rd_address(emb_const_pdu_t* _req)
{
    const uint16_t x = ((uint16_t*)_req->data)[0];
    return SWAP_BYTES(x);
}

uint16_t emb_rdwr_regs_get_req_rd_quantity(emb_const_pdu_t* _req)
{
    const uint16_t x = ((uint16_t*)_req->data)[1];
    return SWAP_BYTES(x);
}

uint16_t emb_rdwr_regs_get_req_wr_address(emb_const_pdu_t* _req)
{
    const uint16_t x = ((uint16_t*)_req->data)[2];
    return SWAP_BYTES(x);
}

uint16_t emb_rdwr_regs_get_req_wr_quantity(emb_const_pdu_t* _req)
{
    const uint16_t x = ((uint16_t*)_req->data)[3];
    return SWAP_BYTES(x);
}

uint16_t emb_rdwr_regs_get_answ_reg(emb_const_pdu_t* _answer, uint16_t _offset)
{
    const uint16_t x = ((uint16_t*)(((uint8_t*)_answer->data) + 1))[_offset];
    return SWAP_BYTES(x);
}

int emb_rdwr_regs_get_answ_regs(emb_const_pdu_t* _answer, uint16_t _offset,
                                uint16_t _n_regs, uint16_t* _p_data)
{
    uint16_t i;
    for(i=0; i<_n_regs; ++i) {
        const uint16_t x = ((uint16_t*)(((uint8_t*)_answer->data) + 1))[_offset + i];
        _p_data[i] = SWAP_BYTES(x);
    }
    return i;
}

int emb_rdwr_regs_get_answ_regs_n(emb_const_pdu_t* _answer)
{
    return ((uint8_t*)_answer->data)[0] >> 1;
}
