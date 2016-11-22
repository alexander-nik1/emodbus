
#include <emodbus/server/server.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/calc_pdu_size.h>
#include <stdint.h>

uint8_t emb_srv_read_fifo(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv) {

    uint8_t* rx_data = _ssrv->rx_pdu->data;
    uint8_t* tx_data = _ssrv->tx_pdu->data;
    uint8_t res, fifo_count;
    unsigned char i;

    const uint16_t addr = GET_BIG_END16(rx_data + 0);

    if(!_srv->read_fifo)
        return MBE_SLAVE_FAILURE;

    res = _srv->read_fifo(_srv,
                          addr,
                          (uint16_t*)(tx_data+4),
                          &fifo_count);

    if(res)
        return res;

    if(fifo_count > EMB_SRV_READ_FIFO_MAX_REGS)
        return MBE_ILLEGAL_DATA_VALUE;

    _ssrv->tx_pdu->function = 0x18;
    _ssrv->tx_pdu->data_size = READ_FIFO_ANS_SIZE(fifo_count);

    ((uint16_t*)tx_data)[0] = fifo_count * 2 + 2;   // bytes count
    ((uint16_t*)tx_data)[1] = fifo_count;           // fifo count

    if(_ssrv->tx_pdu->data_size > _ssrv->tx_pdu->max_size)
        return MBE_SLAVE_FAILURE;

    // Swap all words
    for(i=0; i<fifo_count+2; ++i) {
        const uint16_t tmp = *((uint16_t*)tx_data);
        *((uint16_t*)tx_data) = SWAP_BYTES(tmp);
        tx_data += sizeof(uint16_t);
    }

    return 0;
}
