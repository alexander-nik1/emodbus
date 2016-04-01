
#include <emodbus/client/read_fifo.h>
#include <errno.h>
#include <emodbus/base/byte-word.h>

/*!
 * \file
 * \brief Definition of Read Fifo functions.
 *
 * Functions for working with Read Fifo function.
 *
 */

int emb_read_fifo_calc_req_data_size() {
    return 2;
}

int emb_read_fifo_calc_answer_data_size() {
    return 2 + 2 + 2 * 31;
}

int emb_read_fifo_make_req(emb_pdu_t* _result_req,
                           uint16_t _starting_address) {

    const int req_size = emb_read_fifo_calc_req_data_size();

    if(_result_req->max_size < req_size) {
        return -ENOMEM;
    }

    _result_req->function = 0x18;
    _result_req->data_size = req_size;

    ((uint16_t*)_result_req->data)[0] = SWAP_BYTES(_starting_address);

    return 0;
}

uint16_t emb_read_fifo_byte_count(emb_const_pdu_t* _answer) {
    if(_answer) {
        const uint16_t x = ((uint16_t*)_answer->data)[0];
        return SWAP_BYTES(x);
    }
    return -1;
}

uint16_t emb_read_fifo_regs_count(emb_const_pdu_t* _answer) {
    if(_answer) {
        const uint16_t x = ((uint16_t*)_answer->data)[1];
        return SWAP_BYTES(x);
    }
    return -1;
}

uint16_t emb_read_fifo_get_data(emb_const_pdu_t* _answer,
                                uint16_t _offset) {
    if(_answer && _offset < emb_read_fifo_regs_count(_answer)) {
        const uint16_t x = ((uint16_t*)_answer->data)[2 + _offset];
        return SWAP_BYTES(x);
    }
    return -1;
}
