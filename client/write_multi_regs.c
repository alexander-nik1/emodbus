
#include <emodbus/base/byte-word.h>
#include <errno.h>
#include <emodbus/base/common.h>
#include <emodbus/client/write_multi_regs.h>
#include <string.h>

int write_multi_regs_make_req(emb_pdu_t *_result_req,
                              uint16_t _address,
                              uint16_t _quantity,
                              const void* _data) {

    if(1 <= _quantity && _quantity <= 123) {
        int i;
        _result_req->function = 0x10;
        ((uint16_t*)_result_req->data)[0] = SWAP_BYTES(_address);
        ((uint16_t*)_result_req->data)[1] = SWAP_BYTES(_quantity);
        ((uint8_t*)_result_req->data)[4] = (uint8_t)(_quantity * 2);
        for(i=0; i<_quantity; ++i) {
            uint16_t* addr = (uint16_t*)(((uint8_t*)_result_req->data) + 5 + i * 2);
            const uint16_t data = ((uint16_t*)_data)[i];
            *addr = SWAP_BYTES(data);
        }
        _result_req->data_size = 5 + _quantity * 2;
        return 0;
    }
    else
        return -EINVAL;
}

int write_multi_regs_get_addr(emb_const_pdu_t* _req) {
    const uint16_t v = ((uint16_t*)_req->data)[0];
    return SWAP_BYTES(v);
}

int write_multi_regs_get_quantity(emb_const_pdu_t* _req) {
    const uint16_t v = ((uint16_t*)_req->data)[1];
    return SWAP_BYTES(v);
}

int write_multi_regs_valid_answer(emb_const_pdu_t *_req,
                                  emb_const_pdu_t *_ans) {
    int r;

    if(_ans->function != 0x10)
        return -EINVAL;

    if(_ans->data_size != 4)
        return -ERANGE;

    if(((uint16_t*)_req->data)[0] != ((uint16_t*)_ans->data)[0])
        return -ERANGE;

    if(((uint16_t*)_req->data)[1] != ((uint16_t*)_ans->data)[1])
        return -ERANGE;

    return 0;
}

const struct emb_client_function_i write_multi_regs_interface = {
    0x10,
    write_multi_regs_valid_answer      // check_answer
};
