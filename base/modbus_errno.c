
#include "modbus_errno.h"
#include "string.h"

#include <stdio.h>

#define EMB_CASE_ERROR_STRING(_errno, _err_name)    \
    case _errno: {                                  \
        static const char* str = _err_name;         \
        return str; }

const char* emb_strerror(int _errno) {

    if (_errno < modbus_bad_crc) {
        return strerror(_errno);
    }
    else {
        switch(_errno) {
            EMB_CASE_ERROR_STRING(modbus_success, "Success")
            EMB_CASE_ERROR_STRING(modbus_bad_crc, "Incorrect CRC in received packet")
            EMB_CASE_ERROR_STRING(modbus_buffer_overflow, "Overflow of the buffer")
            EMB_CASE_ERROR_STRING(modbus_resp_without_req, "Was a response, with no previous request")
            EMB_CASE_ERROR_STRING(modbus_no_such_function, "No such function")
            EMB_CASE_ERROR_STRING(modbus_resp_wrong_address, "Server returns incorrect address about itself")
            EMB_CASE_ERROR_STRING(modbus_resp_timeout, "Timeout of response waiting")
            default: return (const char*)0;
        }
    }
}
