
#include <emodbus/server/server.h>
#include <emodbus/server/regs.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/calc_pdu_size.h>
#include <stdint.h>

uint8_t emb_srv_read_write_regs(struct emb_super_server_t* _ssrv,
                                struct emb_server_t* _srv) {

    struct emb_srv_regs_t* wr_regs,*rd_regs;
    uint8_t* rx_data = _ssrv->rx_pdu->data;
    uint8_t* tx_data = _ssrv->tx_pdu->data;
    uint8_t i;

    const uint16_t rd_start_addr = GET_BIG_END16(rx_data + 0);
    const uint16_t rd_quantity =   GET_BIG_END16(rx_data + 2);
    const uint16_t wr_start_addr = GET_BIG_END16(rx_data + 4);
    const uint16_t wr_quantity =   GET_BIG_END16(rx_data + 6);
    const uint8_t  byte_count =    rx_data[8];

    if(!(0x0001 <= rd_quantity && rd_quantity <= EMB_SRV_RDWR_REGS_MAX_READ_REGS))
        return MBE_ILLEGAL_DATA_VALUE;

    if(!(0x0001 <= wr_quantity && wr_quantity <= EMB_SRV_RDWR_REGS_MAX_WRITE_REGS))
        return MBE_ILLEGAL_DATA_VALUE;

    if(byte_count != (wr_quantity*2))
        return MBE_ILLEGAL_DATA_VALUE;

    if(!_srv->get_holding_regs)
        return MBE_SLAVE_FAILURE;

    // find and check regs, to be write
    wr_regs = _srv->get_holding_regs(_srv, wr_start_addr);
    if(!wr_regs)
        return MBE_ILLEGAL_DATA_ADDR;

    if((wr_start_addr+wr_quantity) > (wr_regs->start+wr_regs->size))
        return MBE_ILLEGAL_DATA_ADDR;

    if(!wr_regs->write_regs)
        return MBE_ILLEGAL_FUNCTION;

    // find and check regs, to be read
    rd_regs = _srv->get_holding_regs(_srv, rd_start_addr);
    if(!rd_regs)
        return MBE_ILLEGAL_DATA_ADDR;

    if((rd_start_addr+rd_quantity) > (rd_regs->start+rd_regs->size))
        return MBE_ILLEGAL_DATA_ADDR;

    if(!rd_regs->read_regs)
        return MBE_ILLEGAL_FUNCTION;

    if(READ_WRITE_REGS_ANS_SIZE(rd_quantity) > _ssrv->tx_pdu->max_size)
        return MBE_SLAVE_FAILURE;

    rx_data += 9;
    // swap registers for write
    for(i=0; i<wr_quantity; ++i) {
        const uint16_t tmp = ((uint16_t*)rx_data)[i];
        ((uint16_t*)rx_data)[i] = SWAP_BYTES(tmp);
    }

    i = wr_regs->write_regs(wr_regs,
                            wr_start_addr - wr_regs->start,
                            wr_quantity,
                            (uint16_t*)rx_data);
    if(i)
        return i;

    *tx_data++ = rd_quantity*2; // write bytes_count

    i = rd_regs->read_regs(rd_regs,
                           rd_start_addr - rd_regs->start,
                           rd_quantity,
                           (uint16_t*)tx_data);
    if(i)
        return i;

    // swap readed registers
    for(i=0; i<rd_quantity; ++i) {
        const uint16_t tmp = *((uint16_t*)tx_data);
        *((uint16_t*)tx_data) = SWAP_BYTES(tmp);
        tx_data += sizeof(uint16_t);
    }

    _ssrv->tx_pdu->function = 0x17;
    _ssrv->tx_pdu->data_size = READ_WRITE_REGS_ANS_SIZE(rd_quantity);

    return 0;
}
