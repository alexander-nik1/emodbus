
#include "modbus_pdu.h"
#include <stdint.h>

int emb_check_pdu_for_exception(emb_const_pdu_t *_pdu) {
    if(_pdu->function & 0x80)   // Error code
        return -(((uint8_t*)_pdu->data)[0] + 1500);
    else
        return 0;
}
