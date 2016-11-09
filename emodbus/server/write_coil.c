
#include <emodbus/server/server.h>
#include <emodbus/server/bits.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/calc_pdu_size.h>
#include <stdint.h>

uint8_t emb_srv_write_coil(struct emb_super_server_t* _ssrv,
                           struct emb_server_t* _srv) {

    uint8_t* rx_data = _ssrv->rx_pdu->data;
    uint8_t* tx_data = _ssrv->tx_pdu->data;
    uint8_t coil_value;

    struct emb_srv_bits_t* coils;

    const uint16_t
            addr = GET_BIG_END16(rx_data + 0),
            value = GET_BIG_END16(rx_data + 2);

    switch(value) {
    case 0x0000:
        coil_value = 0;
        break;

    case 0xFF00:
        coil_value = 1;
        break;

    default:
        return MBE_ILLEGAL_DATA_VALUE;
    }

    if(!_srv->get_coils)
        return MBE_SLAVE_FAILURE;

    coils = _srv->get_coils(_srv, addr);

    if(!coils)
        return MBE_ILLEGAL_DATA_ADDR;

    if(!coils->write_bits)
        return MBE_ILLEGAL_DATA_ADDR;

    _ssrv->tx_pdu->function = 0x05;
    _ssrv->tx_pdu->data_size = WRITE_COIL_ANS_SIZE();

    if(_ssrv->tx_pdu->max_size < _ssrv->tx_pdu->data_size)
        return MBE_SLAVE_FAILURE;

    ((uint16_t*)(tx_data))[0] = SWAP_BYTES(addr);
    ((uint16_t*)(tx_data))[1] = SWAP_BYTES(value);

    return coils->write_bits(coils,
                              addr - coils->start,
                              1,
                              &coil_value);
}
