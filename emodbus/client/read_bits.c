
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/read_bits.h>
#include <emodbus/base/calc_pdu_size.h>

/*!
 * \file
 * \brief The realization of Read Coils and Read Discrete Inputs functions.
 *
 * Functions for working with Read Coils and Read Discrete Inputs function.
 *
 */

int emb_read_bits_calc_req_data_size() {
    return READ_COILS_REQ_SIZE();
}

int emb_read_bits_calc_answer_data_size(uint16_t _quantity) {
    if( 1 <= _quantity && _quantity <= EMB_READ_BITS_MAX_QUANTITY ) {
        const uint8_t bytes_count = (_quantity / 8) + ((_quantity & 7) ? 1 : 0);
        return READ_COILS_ANS_SIZE(bytes_count);
    }
    else {
        return -EINVAL;
    }
}

int emb_read_bits_make_req(emb_pdu_t* _result_req,
                           enum EMB_RB_TYPE _type,
                           uint16_t _starting_address, uint16_t _quantity) {

    if(_result_req->max_size < emb_read_bits_calc_req_data_size()) {
        return -ENOMEM;
    }

    if( 1 <= _quantity && _quantity <= EMB_READ_BITS_MAX_QUANTITY ) {
        ((uint16_t*)_result_req->data)[0] = SWAP_BYTES(_starting_address);
        ((uint16_t*)_result_req->data)[1] = SWAP_BYTES(_quantity);
        _result_req->data_size = emb_read_bits_calc_req_data_size();
        switch(_type) {
        case EMB_RB_COILS:
            _result_req->function = 0x01;
            break;
        case EMB_RB_DISCRETE_INPUTS:
            _result_req->function = 0x02;
            break;
        default:
            return -EINVAL;
        }
        return 0;
    }
    else {
        return -EINVAL;
    }
}

uint16_t emb_read_bits_get_starting_addr(emb_const_pdu_t *_req) {
    const uint16_t t = ((uint16_t*)_req->data)[0];
    return SWAP_BYTES(t);
}

uint16_t emb_read_bits_get_quantity(emb_const_pdu_t *_req) {
    const uint16_t t = ((uint16_t*)_req->data)[1];
    return SWAP_BYTES(t);
}

char emb_read_bits_get_bit(emb_const_pdu_t *_answer,
                             uint16_t _coil_offset) {
    // TODO: Make a _coil_offset value checking here.
    const uint8_t byte = ((uint8_t*)_answer->data)[_coil_offset / 8 + 1];
    return (byte >> (_coil_offset & 7)) & 1;
}

uint8_t emb_read_bits_get_byte(emb_const_pdu_t* _answer,
                                uint8_t _byte_offset) {
    // TODO: Make a _byte_offset value checking here.
    return ((uint8_t*)_answer->data)[1 + _byte_offset];
}
