
#ifndef MODBUS_ERROR_NUMBER_H
#define MODBUS_ERROR_NUMBER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum modbus_errno_t {
    modbus_success,
    modbus_bad_crc = 1000,
    modbus_buffer_overflow,
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_ERROR_NUMBER_H
