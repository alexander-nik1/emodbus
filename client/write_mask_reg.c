
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/write_mask_reg.h>
#include <string.h>

/*!
 * \file
 * \brief The definition of "Mask Write Register" functions.
 *
 * Functions for working with "Mask Write Register" function.
 *
 */

int emb_write_mask_reg_calc_req_data_size() {
    return 6;
}

int emb_write_mask_reg_calc_answer_data_size() {
    return 6;
}

int emb_write_mask_reg_make_req(emb_pdu_t* _result_req,
                                uint16_t _address,
                                uint16_t _and_mask,
                                uint16_t _or_mask) {

    if(_result_req->max_size < emb_write_mask_reg_calc_req_data_size()) {
        return -ENOMEM;
    }

    _result_req->data_size = 6;
    _result_req->function = 0x16;

    ((uint16_t*)_result_req->data)[0] = SWAP_BYTES(_address);
    ((uint16_t*)_result_req->data)[1] = SWAP_BYTES(_and_mask);
    ((uint16_t*)_result_req->data)[2] = SWAP_BYTES(_or_mask);

    return 0;
}

uint16_t emb_write_mask_reg_get_address(emb_const_pdu_t* _pdu) {
    return ((uint16_t*)_pdu->data)[0];
}

uint16_t emb_write_mask_reg_get_and_mask(emb_const_pdu_t* _pdu) {
    return ((uint16_t*)_pdu->data)[1];
}

uint16_t emb_write_mask_reg_get_or_mask(emb_const_pdu_t* _pdu) {
    return ((uint16_t*)_pdu->data)[2];
}
