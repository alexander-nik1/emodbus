
#ifndef EMODBUS_SERVER_BITS_H
#define EMODBUS_SERVER_BITS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct emb_srv_regs_t {

    uint16_t start;
    uint16_t size;

    uint8_t (*read_regs)(struct emb_srv_regs_t* _rr,
                         uint16_t _offset,
                         uint16_t _quantity,
                         uint16_t* _pvalues);

    uint8_t (*write_regs)(struct emb_srv_regs_t* _rr,
                          uint16_t _offset,
                          uint16_t _quantity,
                          const uint16_t* _pvalues);
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif //EMODBUS_SERVER_BITS_H
