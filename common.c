
#include "common.h"
#include <stdint.h>

int modbus_check_answer(const void *_req, const void *_answ) {

    // Check address
    if(((uint8_t*)_req)[0] != ((uint8_t*)_answ)[0])
        return -1;

    // Check function
    if(((uint8_t*)_req)[1] != ((uint8_t*)_answ)[1]) {

        if(((uint8_t*)_answ)[1] & 0x80)   // Error code
            return -(((uint8_t*)_answ)[2] + 1000);
        else
            return -2;
    }

    return 0;
}
