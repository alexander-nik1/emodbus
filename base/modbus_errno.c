
#include <emodbus/base/modbus_errno.h>
#include "string.h"

#include <stdio.h>

#define EMB_CASE_ERROR_STRING(_errno, ...)  \
    case _errno: {                              \
        static const char* str = __VA_ARGS__;   \
        return str; }


#define EMB_CASE_ERROR_STRING_MB_CODE(_errno, ...)      \
    EMB_CASE_ERROR_STRING(_errno, "Returned modbus error code: '" __VA_ARGS__ "'")

const char* emb_strerror(int _errno) {

    if (_errno < modbus_bad_crc) {
        return strerror(_errno);
    }
    else if (modbus_bad_crc <= _errno && _errno < EMB_EXCEPTION_BASE){
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
    else if(EMB_EXCEPTION_BASE <= _errno) {
        switch(_errno) {
            EMB_CASE_ERROR_STRING_MB_CODE(EMB_EXCEPTION_FUNC, "Illegal function")
            EMB_CASE_ERROR_STRING_MB_CODE(EMB_EXCEPTION_DATA_ADDR, "Illegal data address")
            EMB_CASE_ERROR_STRING_MB_CODE(EMB_EXCEPTION_DATA_VALUE, "Illegal data value")
            EMB_CASE_ERROR_STRING_MB_CODE(EMB_EXCEPTION_SERV_FAILURE, "Server device failure")
            EMB_CASE_ERROR_STRING_MB_CODE(EMB_EXCEPTION_ACK, "Acknowledge")
            EMB_CASE_ERROR_STRING_MB_CODE(EMB_EXCEPTION_SERVER_BUSY, "Server device busy")
            EMB_CASE_ERROR_STRING_MB_CODE(EMB_EXCEPTION_MEM_PARITY, "Memory parity error")
            EMB_CASE_ERROR_STRING_MB_CODE(EMB_EXCEPTION_GATEWAY_PATH, "Gateway path unavailable")
            EMB_CASE_ERROR_STRING_MB_CODE(EMB_EXCEPTION_GATEWAY_NO_RESP, "Gateway target device failed to respond")
            default: {
                static const char* str = "Returned unknown modbus error code";
                return str;
            }
        }
    }
}
