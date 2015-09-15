
#include "byte-word.h"
#include <errno.h>
#include "common.h"
#include "write_multi_regs.h"
#include <string.h>

int write_multi_regs_make_req(void* _result_req, uint8_t _slave_addr,
                              uint16_t _address, uint16_t _quantity, const void* _data) {

    if(1 <= _quantity && _quantity <= 123) {
        int i;
        ((uint8_t*)_result_req)[0] = _slave_addr;
        ((uint8_t*)_result_req)[1] = 0x10;
        ((uint16_t*)_result_req)[1] = SWAP_BYTES(_address);
        ((uint16_t*)_result_req)[2] = SWAP_BYTES(_quantity);
        ((uint8_t*)_result_req)[6] = (uint8_t)(_quantity * 2);
        for(i=0; i<_quantity; ++i) {
            uint16_t* addr = (uint16_t*)(((uint8_t*)_result_req) + 7 + i * 2);
            const uint16_t data = ((uint16_t*)_data)[i];
            *addr = SWAP_BYTES(data);
        }
        return 7 + _quantity * 2;
    }
    else
        return -EINVAL;
}

int write_multi_regs_valid_answer(const void* _req, unsigned int _req_size,
                                   const void* _answer, unsigned int _answer_size) {
    int r;

    if((r = modbus_check_answer(_req, _answer)) != 0)
        return r;

    if(_answer_size != 8)
        return -ERANGE;

    if(((uint16_t*)_req)[1] != ((uint16_t*)_answer)[1])
        return -ERANGE;

    if(((uint16_t*)_req)[2] != ((uint16_t*)_answer)[2])
        return -ERANGE;

    return 0;
}
