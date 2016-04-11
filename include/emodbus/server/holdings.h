
#ifndef EMODBUS_SERVER_HOLDINGS_H
#define EMODBUS_SERVER_HOLDINGS_H

#include <stdint.h>
#include <emodbus/base/modbus_pdu.h>
#include <emodbus/base/add/s-list.h>

#ifdef __cplusplus
extern "C" {
#endif

struct emb_srv_holdings_t {

    uint16_t start;
    uint16_t size;

    uint8_t (*read_regs)(struct emb_srv_holdings_t* _rr,
                         emb_const_pdu_t* _req,
                         uint16_t _offset,
                         uint16_t _quantity,
                         uint16_t* _pvalues);

    uint8_t (*write_regs)(struct emb_srv_holdings_t* _rr,
                          emb_const_pdu_t* _req,
                          uint16_t _offset,
                          uint16_t _quantity,
                          const uint16_t* _pvalues);
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif //EMODBUS_SERVER_HOLDINGS_H
