
#ifndef MODBUS_MASTER_WRITE_MULTIPLE_REGISTER
#define MODBUS_MASTER_WRITE_MULTIPLE_REGISTER

#include <stdint.h>
#include <emodbus/base/modbus_pdu.h>
#include <emodbus/client/client_base.h>

#ifdef __cplusplus
extern "C" {
#endif

// Build request
int write_multi_regs_make_req(emb_pdu_t* _result_req,
                              uint16_t _address,
                              uint16_t _quantity,
                              const void* _data);

int write_multi_regs_get_addr(emb_const_pdu_t* _req);

int write_multi_regs_get_quantity(emb_const_pdu_t* _req);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_WRITE_MULTIPLE_REGISTER
