
#include <emodbus/client/read_fifo.h>
#include <errno.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/calc_pdu_size.h>

/*!
 * \file
 * \brief Definition of Read Fifo functions.
 *
 * Functions for working with Read Fifo function.
 *
 */

int emb_read_fifo_calc_req_data_size() {
    return READ_FIFO_REQ_SIZE();
}

int emb_read_fifo_calc_answer_data_size() {
    return READ_FIFO_ANS_SIZE();
}

int emb_read_fifo_make_req(emb_pdu_t* _result_req,
                           uint16_t _starting_address) {

    if(_result_req->max_size < READ_FIFO_REQ_SIZE()) {
        return -ENOMEM;
    }

    _result_req->function = 0x18;
    _result_req->data_size = READ_FIFO_REQ_SIZE();

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
