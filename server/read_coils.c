
#include <emodbus/server/server.h>
#include <emodbus/server/coils.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/calc_pdu_size.h>
#include <stdint.h>

uint8_t emb_srv_read_coils(struct emb_super_server_t* _ssrv,
                           struct emb_server_t* _srv) {

    uint8_t* rx_data = _ssrv->rx_pdu->data;
    uint8_t* tx_data = _ssrv->tx_pdu->data;
    uint8_t byte_count;

    struct emb_srv_coils_t* coils;

    const uint16_t
            start_addr = GET_BIG_END16(rx_data + 0),
            quantity = GET_BIG_END16(rx_data + 2);

    if(!(0x0001 <= quantity && quantity <= 0x07D0))
        return MBE_ILLEGAL_DATA_VALUE;

    if(!_srv->get_coils)
        return MBE_SLAVE_FAILURE;

    coils = _srv->get_coils(_srv, start_addr);

    if(!coils)
        return MBE_ILLEGAL_DATA_ADDR;

    if((coils->start + coils->size) < (start_addr + quantity))
        return MBE_ILLEGAL_DATA_ADDR;

    if(!coils->read_coils)
        return MBE_ILLEGAL_DATA_ADDR;

    byte_count = quantity >> 3;

    if((quantity & 0x07) != 0)
        ++byte_count;

    _ssrv->tx_pdu->function = 0x01;
    _ssrv->tx_pdu->data_size = READ_COILS_ANS_SIZE(byte_count);

    if(_ssrv->tx_pdu->max_size < _ssrv->tx_pdu->data_size)
        return MBE_SLAVE_FAILURE;

    *tx_data++ = byte_count;

    return coils->read_coils(coils,
                            start_addr - coils->start,
                            quantity,
                            tx_data);
}
