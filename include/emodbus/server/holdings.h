
#ifndef EMODBUS_SERVER_HOLDINGS_H
#define EMODBUS_SERVER_HOLDINGS_H

#include <stdint.h>
#include <emodbus/base/modbus_pdu.h>
#include <emodbus/base/add/s-list.h>

#ifdef __cplusplus
extern "C" {
#endif

struct emb_srv_holdings_t;

struct emb_srv_holdings_procs_t {

    int (*read_regs)(struct emb_srv_holdings_t* _rr,
                     uint16_t _offset,
                     uint16_t _quantity,
                     uint16_t* _pvalues);

    int (*write_regs)(struct emb_srv_holdings_t* _rr,
                      uint16_t _offset,
                      uint16_t _quantity,
                      const uint16_t* _pvalues);
};

struct emb_srv_holdings_t {
    uint16_t offset;
    uint16_t size;
    struct emb_srv_holdings_procs_t* procs;
    struct sb_list_head list;
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif //EMODBUS_SERVER_HOLDINGS_H
