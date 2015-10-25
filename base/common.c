
#include "common.h"

#include <stdint.h>

int modbus_check_answer(emb_const_pdu_t* _req,
                        emb_const_pdu_t* _answ) {

    // Check function
    if(_req->function != _answ->function) {

        if(_answ->function & 0x80)   // Error code
            return -(((uint8_t*)_answ->data)[1] + 1000);
        else
            return -2;
    }

    return 0;
}
