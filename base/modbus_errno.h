
#ifndef MODBUS_ERROR_NUMBER_H
#define MODBUS_ERROR_NUMBER_H

#include <stdint.h>

/*!
 * \file
 * \brief Error codes.
 *
 * This file contains the error codes, specific to modbus.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The modbus_errno_t enum
 * Possible errors
 */
enum modbus_errno_t {
    modbus_success,             ///< No error
    modbus_bad_crc = 1000,      ///< Incorrect CRC in received packet
    modbus_buffer_overflow,     ///< Overflow of buffer.
    modbus_resp_without_req,    ///< Was a response, with no previous request
    modbus_no_such_function,    ///< No such function
    modbus_resp_wrong_address,  ///< Server returns incorrect address about itself
    modbus_resp_timeout,        ///< Timeout of response waiting
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_ERROR_NUMBER_H
