
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/write_multi_regs.h>
#include <string.h>

/*!
 * \file
 * \brief Definition of Write Multiple Registers functions.
 *
 * Functions for working with Write Multiple Registers function.
 *
 */

int emb_write_regs_calc_req_data_size(uint16_t _quantity) {
    return 5 + _quantity * 2;
}

int emb_write_regs_calc_answer_data_size() {
    return 4;
}

int emb_write_regs_make_req(emb_pdu_t *_result_req,
                            uint16_t _address,
                            uint16_t _quantity,
                            const uint16_t* _data) {

    if(_result_req->max_size < emb_write_regs_calc_req_data_size(_quantity)) {
        return -ENOMEM;
    }

    if(1 <= _quantity && _quantity <= 123) {

        uint16_t* data_addr = (uint16_t*)(((uint8_t*)_result_req->data) + 5);

        int i;

        _result_req->function = 0x10;
        _result_req->data_size = emb_write_regs_calc_req_data_size(_quantity);

        ((uint16_t*)_result_req->data)[0] = SWAP_BYTES(_address);
        ((uint16_t*)_result_req->data)[1] = SWAP_BYTES(_quantity);
        ((uint8_t*)_result_req->data)[4] = (uint8_t)(_quantity * 2);

        for(i=0; i<_quantity; ++i) {
            const uint16_t data = ((uint16_t*)_data)[i];
            data_addr[i] = SWAP_BYTES(data);
        }

        return 0;
    }
    else
        return -EINVAL;
}

uint16_t emb_write_regs_get_req_address(emb_const_pdu_t* _req) {
    const uint16_t x = ((uint16_t*)_req->data)[0];
    return SWAP_BYTES(x);
}

uint16_t emb_write_regs_get_req_quantity(emb_const_pdu_t* _req) {
    const uint16_t x = ((uint16_t*)_req->data)[1];
    return SWAP_BYTES(x);
}

int emb_write_regs_get_req_data(emb_const_pdu_t* _req, uint16_t _offset) {
    if(_offset < emb_write_regs_get_req_quantity(_req)) {
        const uint16_t* data_addr = (uint16_t*)(((uint8_t*)_req->data) + 5);
        const uint16_t x = data_addr[_offset];
        return SWAP_BYTES(x);
    }
    return 0xDEAD;
}

uint16_t emb_write_regs_get_answer_address(emb_const_pdu_t* _answer) {
    const uint16_t x = ((uint16_t*)_answer->data)[0];
    return SWAP_BYTES(x);
}

uint16_t emb_write_regs_get_answer_quantity(emb_const_pdu_t* _answer) {
    const uint16_t x = ((uint16_t*)_answer->data)[1];
    return SWAP_BYTES(x);
}
