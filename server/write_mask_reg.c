
#include <emodbus/server/server.h>
#include <emodbus/server/holdings.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/calc_pdu_size.h>
#include <stdint.h>

uint8_t emb_srv_mask_reg(struct emb_super_server_t* _ssrv,
                         struct emb_server_t* _srv) {

    struct emb_srv_holdings_t* r;
    uint8_t* rx_data = _ssrv->rx_pdu->data;
    uint8_t* tx_data = _ssrv->tx_pdu->data;
    uint8_t res;

    uint16_t tmp;

    const uint16_t addr = GET_BIG_END16(rx_data + 0),
                   or_mask =  GET_BIG_END16(rx_data + 2),
                   and_mask = GET_BIG_END16(rx_data + 4);

    r = _srv->get_holdings(_srv, addr);

    if(!r)
        return MBE_ILLEGEL_DATA_ADDR;

    if(!r->write_regs)
        return MBE_ILLEGEL_DATA_ADDR;

    res = r->read_regs(r,
                       MB_CONST_PDU(_ssrv->rx_pdu),
                       addr,
                       1,
                       &tmp);
    if(res)
        return res;

    tmp = (tmp & and_mask) | (or_mask & ~and_mask);

    res = r->write_regs(r,
                        MB_CONST_PDU(_ssrv->rx_pdu),
                        addr,
                        1,
                        &tmp);
    if(res)
        return res;

    ((uint16_t*)tx_data)[0] = SWAP_BYTES(addr);
    ((uint16_t*)tx_data)[1] = SWAP_BYTES(or_mask);
    ((uint16_t*)tx_data)[2] = SWAP_BYTES(and_mask);

    _ssrv->tx_pdu->function = 0x16;
    _ssrv->tx_pdu->data_size = MASK_REGISTER_ANS_SIZE();

    if(_ssrv->tx_pdu->data_size > _ssrv->tx_pdu->max_size)
        return MBE_SLAVE_FAILURE;

    return 0;
}
