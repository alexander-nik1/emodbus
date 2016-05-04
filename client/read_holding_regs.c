 
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/read_holding_regs.h>
#include <emodbus/base/calc_pdu_size.h>

/*!
 * \file
 * \brief The realization of Read Holding Registers functions.
 *
 * Functions for working with Read Holding Registers function.
 *
 */

int emb_read_hold_regs_calc_req_data_size() {
    return READ_HOLDINGS_REQ_SIZE();
}

int emb_read_hold_regs_calc_answer_data_size(uint16_t _quantity) {
    if( 1 <= _quantity && _quantity <= 125 ) {
        return READ_HOLDINGS_ANS_SIZE(_quantity);
    }
    else {
        return -EINVAL;
    }
}

int emb_read_hold_regs_make_req(emb_pdu_t *_result_req,
                               uint16_t _starting_address, uint16_t _quantity) {

    if(_result_req->max_size < emb_read_hold_regs_calc_req_data_size()) {
        return -ENOMEM;
    }

	if( 1 <= _quantity && _quantity <= 125 ) {
        ((uint16_t*)_result_req->data)[0] = SWAP_BYTES(_starting_address);
        ((uint16_t*)_result_req->data)[1] = SWAP_BYTES(_quantity);
        _result_req->data_size = 4;
        _result_req->function = 3;
        return 0;
	}
	else {
		return -EINVAL;
	}
}

uint16_t emb_read_hold_regs_get_starting_addr(emb_const_pdu_t *_req) {
    const uint16_t t = ((uint16_t*)_req->data)[0];
    return SWAP_BYTES(t);
}

uint16_t emb_read_hold_regs_get_quantity(emb_const_pdu_t *_req) {
    const uint16_t t = ((uint16_t*)_req->data)[1];
    return SWAP_BYTES(t);
}

uint16_t emb_read_hold_regs_get_reg(emb_const_pdu_t *_answer,
                                   uint16_t _reg_offset) {
    const uint16_t x = ((uint16_t*)(((uint8_t*)_answer->data) + 1))[_reg_offset];
    return SWAP_BYTES(x);
}

void emb_read_hold_regs_get_regs(emb_const_pdu_t* _answer,
                                 uint16_t _reg_offset,
                                 uint16_t _n_regs,
                                 uint16_t* _p_data) {
    uint16_t i;
    for(i=0; i<_n_regs; ++i) {
        const uint16_t x = ((uint16_t*)(((uint8_t*)_answer->data) + 1))[_reg_offset + i];
        _p_data[i] = SWAP_BYTES(x);
    }
}

int emb_read_hold_regs_get_regs_n(emb_const_pdu_t *_answer) {
    return ((uint8_t*)_answer->data)[0] >> 1;
}
