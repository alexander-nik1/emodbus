
#ifndef MODBUS_ERROR_NUMBER_H
#define MODBUS_ERROR_NUMBER_H

#include <stdint.h>
#include <errno.h>

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

enum modbus_exception_code_t{
    MBE_ILLEGAL_FUNCTION = 0x01,
    MBE_ILLEGAL_DATA_ADDR = 0x02,
    MBE_ILLEGAL_DATA_VALUE = 0x03,
    MBE_SLAVE_FAILURE = 0x04,
    MBE_ACK = 0x05,
    MBE_SLAVE_BUSY = 0x06,
    MBE_MEMORY_CRC_ERROR = 0x08,
    MBE_GATEWAY_PATH_UNAVAIL = 0x0A,
    MBE_GATEWAY_DEVICE_NO_RESP = 0x0B
};

/**
 * @brief Returns an error string
 *
 * This function returns pointer to a static string.
 * Known error codes is in enum modbus_errno_t.
 * Also if _errno less than 1000, function returns an ontput
 * of strerror(_errno) function.
 *
 * @param [in] _errno error number
 * @return error string or NULL if _errno is unknown
 */

const char* emb_strerror(int _errno);

/**
 * @brief The modbus_errno_t enum
 * Possible errors
 */
enum emodbus_errno_t {
    modbus_success,             ///< No error
    modbus_bad_crc = 1000,      ///< Incorrect CRC in received packet
    modbus_buffer_overflow,     ///< Overflow of the buffer
    modbus_resp_without_req,    ///< Was a response, with no previous request
    modbus_no_such_function,    ///< No such function
    modbus_resp_wrong_address,  ///< Server returns incorrect address about itself
    modbus_resp_wrong_func,     ///< Server returns incorrect function number
    modbus_resp_timeout,        ///< Timeout of response waiting
    modbus_resp_buffer_ovf,     ///< Response buffer is too short to store all data
    modbus_resp_wrong_transaction_id, ///< Incorrect transaction Id in received response

    EMB_EXCEPTION_BASE = 1500,

    EMB_ILLEGAL_FUNCTION = EMB_EXCEPTION_BASE + MBE_ILLEGAL_FUNCTION,
    EMB_ILLEGEL_DATA_ADDR = EMB_EXCEPTION_BASE + MBE_ILLEGAL_DATA_ADDR,
    EMB_ILLEGAL_DATA_VALUE = EMB_EXCEPTION_BASE + MBE_ILLEGAL_DATA_VALUE,
    EMB_SLAVE_FAILURE = EMB_EXCEPTION_BASE + MBE_SLAVE_FAILURE,
    EMB_ACK = EMB_EXCEPTION_BASE + MBE_ACK,
    EMB_SLAVE_BUSY = EMB_EXCEPTION_BASE + MBE_SLAVE_BUSY,

    EMB_MEMORY_CRC_ERROR = EMB_EXCEPTION_BASE + MBE_MEMORY_CRC_ERROR,

    EMB_GATEWAY_PATH_UNAVAIL = EMB_EXCEPTION_BASE + MBE_GATEWAY_PATH_UNAVAIL,
    EMB_GATEWAY_DEVICE_NO_RESP = EMB_EXCEPTION_BASE + MBE_GATEWAY_DEVICE_NO_RESP,

};

#ifndef ENOMEM
#define ENOMEM 1
#endif

#ifndef E2BIG
#define E2BIG 2
#endif

#ifndef EINVAL
#define EINVAL 3
#endif

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_ERROR_NUMBER_H
