
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <string.h>
#include <emodbus/base/common.h>
#include <emodbus/client/write_coil.h>
#include <emodbus/base/calc_pdu_size.h>

/*!
 * \file
 * \brief The realization of Write Coil functions.
 *
 * Functions for working with Write Coil function.
 *
 */

int emb_write_coil_calc_req_data_size() {
    return WRITE_COIL_REQ_SIZE();
}

int emb_write_coil_calc_answer_data_size() {
    return WRITE_COIL_ANS_SIZE();
}

int emb_write_coil_make_req(emb_pdu_t *_result_req,
                             uint16_t _address,
                             char _value) {

    uint16_t value_code;

    if(_result_req->max_size < WRITE_COIL_REQ_SIZE()) {
        return -ENOMEM;
    }

    ((uint16_t*)_result_req->data)[0] = SWAP_BYTES(_address);

    value_code = (_value != 0) ? 0xFF00 : 0x0000;

    ((uint16_t*)_result_req->data)[1] = SWAP_BYTES(value_code);

    _result_req->data_size = WRITE_COIL_REQ_SIZE();
    _result_req->function = 0x05;

    return 0;
}

uint16_t emb_write_coil_get_addr(emb_const_pdu_t *_req) {
    const uint16_t t = ((uint16_t*)_req->data)[0];
    return SWAP_BYTES(t);
}
