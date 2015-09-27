
#ifndef MODBUS_MASTER_READ_HOLDING_REGISTERS
#define MODBUS_MASTER_READ_HOLDING_REGISTERS

#include <stdint.h>
#include "modbus_pdu.h"

#ifdef __cplusplus
extern "C" {
#endif

// Build request
int read_holding_regs_make_req(struct modbus_pdu_t* _result_req,
                               uint16_t _starting_address, uint16_t _quantity);

// Get starting address from request
uint16_t read_holding_regs_get_starting_addr(const struct modbus_const_pdu_t* _req);

// Get quantity from request
uint16_t read_holding_regs_get_quantity(const struct modbus_const_pdu_t* _req);

// Check and valid answer
int read_holding_regs_valid_answer(const struct modbus_const_pdu_t* _req,
                                   const struct modbus_const_pdu_t* _ans);

// Get register from answer
uint16_t read_holding_regs_get_reg(const struct modbus_const_pdu_t* _req,
                                   uint16_t _reg_addr);

// Get registers number from answer
int read_holding_regs_get_regs_n(const struct modbus_const_pdu_t* _req);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_READ_HOLDING_REGISTERS
