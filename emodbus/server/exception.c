
#include <emodbus/base/modbus_pdu.h>
#include <emodbus/server/server.h>
#include <errno.h>
#include <stdint.h>

#define MB_EXCEPTION_PDU_DATA_SIZE  1

int emb_build_exception_pdu(emb_pdu_t* _result,
                            uint8_t _func,
                            uint8_t _errno) {

    uint8_t* const data = _result->data;

    if(_result->max_size < MB_EXCEPTION_PDU_DATA_SIZE) {
        return -ENOMEM;
    }

    _result->function = _func | 0x80;

    _result->data_size = MB_EXCEPTION_PDU_DATA_SIZE;

    data[0] = _errno;

    return 0;
}
