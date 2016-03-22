
#ifndef THE_MODBUS_CRC_H
#define THE_MODBUS_CRC_H

#include<stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t crc16(const uint8_t *nData, uint16_t wLength);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // THE_MODBUS_CRC_H
