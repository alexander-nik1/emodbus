
#ifndef MODBUS_MASTER_WRITE_MULTIPLE_REGISTER
#define MODBUS_MASTER_WRITE_MULTIPLE_REGISTER

#include <stdint.h>
#include "../base/modbus_pdu.h"
#include "client_base.h"

#ifdef __cplusplus
extern "C" {
#endif

extern struct emb_client_function_i write_multi_regs_interface;

// Build request
int write_multi_regs_make_req(emb_pdu_t* _result_req,
                              uint16_t _address,
                              uint16_t _quantity,
                              const void* _data);

int write_multi_regs_get_addr(emb_const_pdu_t* _req);

int write_multi_regs_get_quantity(emb_const_pdu_t* _req);

// Check and valid answer
int write_multi_regs_valid_answer(emb_const_pdu_t* _req,
                                  emb_const_pdu_t* _ans);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_WRITE_MULTIPLE_REGISTER
