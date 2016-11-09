
#include <emodbus/server/server.h>
#include <emodbus/server/bits.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/calc_pdu_size.h>
#include <stdint.h>

uint8_t emb_srv_write_coils(struct emb_super_server_t* _ssrv,
                            struct emb_server_t* _srv) {

    uint8_t* rx_data = _ssrv->rx_pdu->data;
    uint8_t* tx_data = _ssrv->tx_pdu->data;
    uint8_t i;

    struct emb_srv_bits_t* coils;

    const uint16_t
            start_addr = GET_BIG_END16(rx_data + 0),
            quantity = GET_BIG_END16(rx_data + 2);
    const uint8_t byte_count = *(rx_data + 4);

    if(!(0x0001 <= quantity && quantity <= 0x07B0))
        return MBE_ILLEGAL_DATA_VALUE;

    if((_ssrv->rx_pdu->data_size - 5) != byte_count)
        return MBE_ILLEGAL_DATA_VALUE;

    if(!_srv->get_coils)
        return MBE_SLAVE_FAILURE;

    coils = _srv->get_coils(_srv, start_addr);

    if(!coils)
        return MBE_ILLEGAL_DATA_ADDR;

    if((coils->start + coils->size) < (start_addr + quantity))
        return MBE_ILLEGAL_DATA_ADDR;

    if(!coils->write_bits)
        return MBE_ILLEGAL_DATA_ADDR;

    _ssrv->tx_pdu->function = 0x0F;
    _ssrv->tx_pdu->data_size = WRITE_COILS_ANS_SIZE();

    if(_ssrv->tx_pdu->max_size < _ssrv->tx_pdu->data_size)
        return MBE_SLAVE_FAILURE;

    ((uint16_t*)(tx_data))[0] = SWAP_BYTES(start_addr);
    ((uint16_t*)(tx_data))[1] = SWAP_BYTES(quantity);

    rx_data += 5;

    return coils->write_bits(coils,
                              start_addr - coils->start,
                              quantity,
                              rx_data);
}
