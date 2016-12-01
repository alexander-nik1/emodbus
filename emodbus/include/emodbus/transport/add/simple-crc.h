
#ifndef EMODBUS_SIMPLE_CRC_H
#define EMODBUS_SIMPLE_CRC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t simple_crc16(const uint8_t* buf, unsigned int len);

#ifdef __cplusplus
}	// extern "C"
#endif

#endif	// EMODBUS_SIMPLE_CRC_H
