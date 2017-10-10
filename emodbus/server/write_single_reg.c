
#include <emodbus/server/server.h>
#include <emodbus/server/regs.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/calc_pdu_size.h>
#include <stdint.h>

uint8_t emb_srv_write_reg(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv) {

    struct emb_srv_regs_t* r;
    uint8_t* rx_data = _ssrv->rx_pdu->data;
    uint8_t* tx_data = _ssrv->tx_pdu->data;
    uint8_t res;

    const uint16_t addr = GET_BIG_END16(rx_data + 0),
                   data = GET_BIG_END16(rx_data + 2);

    if(!_srv->get_holding_regs)
        return MBE_SLAVE_FAILURE;

    r = _srv->get_holding_regs(_srv, addr);

    if(!r)
        return MBE_ILLEGAL_DATA_ADDR;

    if(!r->write_regs)
        return MBE_ILLEGAL_DATA_ADDR;

    res = r->write_regs(r,
                        addr - r->start,
                        1,
                        &data);
    if(res)
        return res;

    if(WRITE_REGISTER_ANS_SIZE() > _ssrv->tx_pdu->max_size)
        return MBE_SLAVE_FAILURE;

    BIG_END_MK16(tx_data, addr);
    tx_data += 2;
    BIG_END_MK16(tx_data, data);

    _ssrv->tx_pdu->function = 0x06;
    _ssrv->tx_pdu->data_size = WRITE_REGISTER_ANS_SIZE();

    return 0;
}
