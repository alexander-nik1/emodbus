
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/read_coils.h>
#include <emodbus/base/calc_pdu_size.h>

/*!
 * \file
 * \brief The realization of Read Coils functions.
 *
 * Functions for working with Read Coils function.
 *
 */

int emb_read_coils_calc_req_data_size() {
    return READ_COILS_REQ_SIZE();
}

int emb_read_coils_calc_answer_data_size(uint16_t _quantity) {
    if( 1 <= _quantity && _quantity <= 0x07D0 ) {
        return READ_COILS_ANS_SIZE(_quantity);
    }
    else {
        return -EINVAL;
    }
}

int emb_read_coils_make_req(emb_pdu_t *_result_req,
                               uint16_t _starting_address, uint16_t _quantity) {

    if(_result_req->max_size < emb_read_coils_calc_req_data_size()) {
        return -ENOMEM;
    }

    if( 1 <= _quantity && _quantity <= 0x07D0 ) {
        ((uint16_t*)_result_req->data)[0] = SWAP_BYTES(_starting_address);
        ((uint16_t*)_result_req->data)[1] = SWAP_BYTES(_quantity);
        _result_req->data_size = emb_read_coils_calc_req_data_size();
        _result_req->function = 0x01;
        return 0;
    }
    else {
        return -EINVAL;
    }
}

uint16_t emb_read_coils_get_starting_addr(emb_const_pdu_t *_req) {
    const uint16_t t = ((uint16_t*)_req->data)[0];
    return SWAP_BYTES(t);
}

uint16_t emb_read_coils_get_quantity(emb_const_pdu_t *_req) {
    const uint16_t t = ((uint16_t*)_req->data)[1];
    return SWAP_BYTES(t);
}

char emb_read_coils_get_coil(emb_const_pdu_t *_answer,
                             uint16_t _coil_offset) {
    const uint8_t byte = ((uint8_t*)_answer->data)[_coil_offset / 8 + 1];
    return (byte >> (_coil_offset % 8)) & 1;
}
