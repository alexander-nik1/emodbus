
#include "byte-word.h"
#include <errno.h>
#include "common.h"
#include "write_mask_reg.h"
#include <string.h>

int write_mask_reg_make_req(void* _result_req, uint8_t _slave_addr,
                               uint16_t _address, uint16_t _and_mask, uint16_t _or_mask) {

    ((uint8_t*)_result_req)[0] = _slave_addr;
    ((uint8_t*)_result_req)[1] = 0x16;
    ((uint16_t*)_result_req)[1] = SWAP_BYTES(_address);
    ((uint16_t*)_result_req)[2] = SWAP_BYTES(_and_mask);
    ((uint16_t*)_result_req)[3] = SWAP_BYTES(_or_mask);
    return 8;
}

int write_mask_reg_valid_answer(const void* _req, unsigned int _req_size,
                                   const void* _answer, unsigned int _answer_size) {
    int r;

//    if((r = modbus_check_answer(_req, _answer)) != 0)
//        return r;

    if(_req_size != 8)
        return -EINVAL;

    if(_req_size != _answer_size)
        return -1;

    return memcmp(_req, _answer, _req_size);
}
