
#ifndef MODBUS_MASTER_COMMON_H
#define MODBUS_MASTER_COMMON_H

#include "modbus_pdu.h"

int modbus_check_answer(const struct modbus_const_pdu_t* _req, const struct modbus_const_pdu_t* _answ);

#endif // MODBUS_MASTER_COMMON_H
