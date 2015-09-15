
#ifndef MODBUS_MASTER_READ_HOLDING_REGISTERS
#define MODBUS_MASTER_READ_HOLDING_REGISTERS

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Build request
int read_holding_regs_make_req(void* _result_req, uint8_t _slave_addr, 
                               uint16_t _starting_address, uint16_t _quantity);

// Check and valid answer
int read_holding_regs_valid_answer(const void* _req, unsigned int _req_size, 
                                   const void* _answer, unsigned int _answer_size);

// Get register from answer
uint16_t read_holding_regs_get_reg(const void* _answer, unsigned int _answer_size,
                                   uint16_t _reg_addr);

// Get registers number from answer
int read_holding_regs_get_regs_n(const void* _answer, unsigned int _answer_size);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_READ_HOLDING_REGISTERS
