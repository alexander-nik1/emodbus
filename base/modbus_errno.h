
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

const char* emb_strerror(int _errno);

/**
 * @brief The modbus_errno_t enum
 * Possible errors
 */
enum modbus_errno_t {
    modbus_success,             ///< No error
    modbus_bad_crc = 1000,      ///< Incorrect CRC in received packet
    modbus_buffer_overflow,     ///< Overflow of the buffer
    modbus_resp_without_req,    ///< Was a response, with no previous request
    modbus_no_such_function,    ///< No such function
    modbus_resp_wrong_address,  ///< Server returns incorrect address about itself
    modbus_resp_timeout,        ///< Timeout of response waiting

    EMB_EXCEPTION_BASE = 1500,

    EMB_EXCEPTION_FUNC = EMB_EXCEPTION_BASE + 0x1,
    EMB_EXCEPTION_DATA_ADDR,
    EMB_EXCEPTION_DATA_VALUE,
    EMB_EXCEPTION_SERV_FAILURE,
    EMB_EXCEPTION_ACK,
    EMB_EXCEPTION_SERVER_BUSY,

    EMB_EXCEPTION_MEM_PARITY = EMB_EXCEPTION_BASE + 0x08,

    EMB_EXCEPTION_GATEWAY_PATH = EMB_EXCEPTION_BASE + 0x0A,
    EMB_EXCEPTION_GATEWAY_NO_RESP,

};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_ERROR_NUMBER_H
