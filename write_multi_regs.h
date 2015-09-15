
#ifndef MODBUS_MASTER_WRITE_MULTIPLE_REGISTER
#define MODBUS_MASTER_WRITE_MULTIPLE_REGISTER

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Build request
int write_multi_regs_make_req(void* _result_req, uint8_t _slave_addr,
                               uint16_t _address, uint16_t _quantity, const void* _data);

// Check and valid answer
int write_multi_regs_valid_answer(const void* _req, unsigned int _req_size,
                                   const void* _answer, unsigned int _answer_size);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_WRITE_MULTIPLE_REGISTER
