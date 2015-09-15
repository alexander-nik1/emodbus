 
#include "byte-word.h"
#include <errno.h>
#include "common.h"
#include "read_holding_regs.h"

int read_holding_regs_make_req(void* _result_req, uint8_t _slave_addr, 
							   uint16_t _starting_address, uint16_t _quantity) {
	
	if( 1 <= _quantity && _quantity <= 125 ) {
		((uint8_t*)_result_req)[0] = _slave_addr;
		((uint8_t*)_result_req)[1] = 0x03;
		((uint16_t*)_result_req)[1] = SWAP_BYTES(_starting_address);
		((uint16_t*)_result_req)[2] = SWAP_BYTES(_quantity);
		return 6;
	}
	else {
		return -EINVAL;
	}
}

int read_holding_regs_valid_answer(const void* _req, unsigned int _req_size, 
								   const void* _answer, unsigned int _answer_size) {
    int r;
    uint16_t quantity;

	if(_req_size != 6)
		return -EINVAL;

    if((r = modbus_check_answer(_req, _answer)) != 0)
        return r;

    quantity = SWAP_BYTES(((uint16_t*)_req)[2]);

    if(_answer_size != (5 + (quantity * 2)))
        return -E2BIG;

    if((quantity * 2) != ((uint8_t*)_answer)[2])
        return -ERANGE;

	return 0;
}

uint16_t read_holding_regs_get_reg(const void* _answer, unsigned int _answer_size,
                                   uint16_t _reg_addr) {
    return SWAP_BYTES(((uint16_t*)(((uint8_t*)_answer) + 3))[_reg_addr]);
}

int read_holding_regs_get_regs_n(const void* _answer, unsigned int _answer_size) {
    return ((uint8_t*)_answer)[2] >> 1;
}
