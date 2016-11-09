
#ifndef EMODBUS_SERVER_COILS_H
#define EMODBUS_SERVER_COILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct emb_srv_bits_t {

    uint16_t start;
    uint16_t size;

    uint8_t (*read_bits)(struct emb_srv_bits_t* _coils,
                          uint16_t _offset,
                          uint16_t _quantity,
                          uint8_t* _pvalues);

    uint8_t (*write_bits)(struct emb_srv_bits_t* _coils,
                           uint16_t _offset,
                           uint16_t _quantity,
                           const uint8_t* _pvalues);
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // EMODBUS_SERVER_COILS_H
