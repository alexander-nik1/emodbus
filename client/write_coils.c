
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <string.h>
#include <emodbus/base/common.h>
#include <emodbus/client/write_coils.h>
#include <emodbus/base/calc_pdu_size.h>

/*!
 * \file
 * \brief The realization of Write Coils functions.
 *
 * Functions for working with Write Coils function.
 *
 */

int emb_write_coils_calc_req_data_size(uint16_t _quantity) {
    if( 1 <= _quantity && _quantity <= 0x07D0 ) {
        const uint8_t bytes_count = (_quantity / 8) + ((_quantity & 7) ? 1 : 0);
        return WRITE_COILS_REQ_SIZE(bytes_count);
    }
    else {
        return -EINVAL;
    }
}

int emb_write_coils_calc_answer_data_size() {
    return WRITE_COILS_ANS_SIZE();
}

int emb_write_coils_make_req(emb_pdu_t *_result_req,
                             uint16_t _starting_address,
                             uint16_t _quantity,
                             const uint8_t* _pcoils) {

    const uint8_t byte_count = emb_write_coils_calc_req_data_size(_quantity);
    const uint8_t coils_byte_count = byte_count - 5;

    if(_result_req->max_size < byte_count) {
        return -ENOMEM;
    }

    if( 1 <= _quantity && _quantity <= 0x07D0 ) {
        ((uint16_t*)_result_req->data)[0] = SWAP_BYTES(_starting_address);
        ((uint16_t*)_result_req->data)[1] = SWAP_BYTES(_quantity);
        ((uint8_t*)_result_req->data)[4] = coils_byte_count;
        memcpy(((uint8_t*)_result_req->data) + 5, _pcoils, coils_byte_count);
        _result_req->data_size = byte_count;
        _result_req->function = 0x0F;
        return 0;
    }
    else {
        return -EINVAL;
    }
}

uint16_t emb_write_coils_get_starting_addr(emb_const_pdu_t *_req) {
    const uint16_t t = ((uint16_t*)_req->data)[0];
    return SWAP_BYTES(t);
}

uint16_t emb_write_coils_get_quantity(emb_const_pdu_t *_req) {
    const uint16_t t = ((uint16_t*)_req->data)[1];
    return SWAP_BYTES(t);
}
