
#include <emodbus/server/server.h>
#include <emodbus/server/regs.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/calc_pdu_size.h>
#include <stdint.h>

uint8_t emb_srv_read_regs(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv) {

    struct emb_srv_regs_t* r = 0;
    uint8_t* rx_data = _ssrv->rx_pdu->data;
    uint8_t* tx_data = _ssrv->tx_pdu->data;
    uint8_t res;
    unsigned char i;
    unsigned char function = _ssrv->rx_pdu->function;

    const uint16_t
            start_addr = GET_BIG_END16(rx_data + 0),
            quantity = GET_BIG_END16(rx_data + 2);

    if(function == 0x03) {
        if(!_srv->get_holding_regs)
            return MBE_SLAVE_FAILURE;
        r = _srv->get_holding_regs(_srv, start_addr);
    }
    else if(function == 0x04) {
        if(!_srv->get_input_regs)
            return MBE_SLAVE_FAILURE;
        r = _srv->get_input_regs(_srv, start_addr);
    }

    if(!r)
        return MBE_ILLEGAL_DATA_ADDR;

    if(!(0x0001 <= quantity && quantity <= 0x007D))
        return MBE_ILLEGAL_DATA_VALUE;

    if((start_addr+quantity) > (r->start+r->size))
        return MBE_ILLEGAL_DATA_ADDR;

    if(!r->read_regs)
        return MBE_ILLEGAL_DATA_ADDR;

    *tx_data = quantity*2;  // byte count

    ++tx_data; // skip byte-count

    _ssrv->tx_pdu->function = function;
    _ssrv->tx_pdu->data_size = READ_HOLDINGS_ANS_SIZE(quantity);

    if(_ssrv->tx_pdu->data_size > _ssrv->tx_pdu->max_size)
        return MBE_SLAVE_FAILURE;

    res = r->read_regs(r,
                       start_addr - r->start,
                       quantity,
                       (void*)tx_data);
    if(res)
        return res;

    // Swap all words
    for(i=0; i<quantity; ++i) {
        SWAP_BYTES_PTR(tx_data);
        tx_data += sizeof(uint16_t);
    }

    return 0;
}
