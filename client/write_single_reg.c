
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/write_single_reg.h>
#include <string.h>

/*!
 * \file
 * \brief The definition of a "Write Single Register" functions.
 *
 * Functions for working with "Write Single Register" function.
 *
 */

int emb_write_reg_calc_req_data_size() {
    return 4;
}

int emb_write_reg_calc_answer_data_size() {
    return 4;
}

int emb_write_reg_make_req(emb_pdu_t* _result_req,
                           uint16_t _address,
                           uint16_t _value) {

    if(_result_req->max_size < emb_write_reg_calc_req_data_size()) {
        return -ENOMEM;
    }

    _result_req->data_size = 4;
    _result_req->function = 0x06;

    ((uint16_t*)_result_req->data)[0] = SWAP_BYTES(_address);
    ((uint16_t*)_result_req->data)[1] = SWAP_BYTES(_value);

    return 0;
}

uint16_t emb_write_reg_get_address(emb_const_pdu_t* _pdu) {
    const uint16_t x = ((uint16_t*)_pdu->data)[0];
    return SWAP_BYTES(x);
}

uint16_t emb_write_reg_get_value(emb_const_pdu_t* _pdu) {
    const uint16_t x = ((uint16_t*)_pdu->data)[1];
    return SWAP_BYTES(x);
}
