 
#include "byte-word.h"
#include <errno.h>
#include "common.h"
#include "read_holding_regs.h"

/*!
 * \file
 * \brief The realization of Read Holding Registers functions.
 *
 * Functions for working with Read Holding Registers function.
 *
 */

int read_holding_regs_make_req(struct modbus_pdu_t *_result_req,
                               uint16_t _starting_address, uint16_t _quantity) {

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

uint16_t read_holding_regs_get_starting_addr(const struct modbus_const_pdu_t *_req) {
    const uint16_t t = ((uint16_t*)_req->data)[0];
    return SWAP_BYTES(t);
}

uint16_t read_holding_regs_get_quantity(const struct modbus_const_pdu_t *_req) {
    const uint16_t t = ((uint16_t*)_req->data)[1];
    return SWAP_BYTES(t);
}

int read_holding_regs_valid_answer(const struct modbus_const_pdu_t* _req,
                                   const struct modbus_const_pdu_t* _ans) {
    int r;
    uint16_t quantity, tmp;

    if(_req->data_size != 4)
		return -EINVAL;

    if((r = modbus_check_answer(_req->data, _ans->data)) != 0)
        return r;

    tmp = ((uint16_t*)_req)[0];
    quantity = SWAP_BYTES(tmp);

    if(_ans->data_size != (7 + (quantity * 2)))
        return -E2BIG;

    if((quantity * 2) != ((uint8_t*)_ans->data)[0])
        return -ERANGE;

	return 0;
}

uint16_t read_holding_regs_get_reg(const struct modbus_const_pdu_t *_ans,
                                   uint16_t _reg_offset) {
    const uint16_t x = ((uint16_t*)(((uint8_t*)_ans->data) + 1))[_reg_offset];
    return SWAP_BYTES(x);
}

int read_holding_regs_get_regs_n(const struct modbus_const_pdu_t *_ans) {
    return ((uint8_t*)_ans->data)[0] >> 1;
}

struct emb_client_function_i read_holding_regs_interface = {
    read_holding_regs_valid_answer      // check_answer
};

