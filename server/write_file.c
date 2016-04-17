#include <emodbus/server/server.h>
#include <emodbus/server/file.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/calc_pdu_size.h>
#include <stdint.h>
#include <string.h>

uint8_t emb_srv_write_file(struct emb_super_server_t* _ssrv,
                           struct emb_server_t* _srv) {

    enum { reference_type = 0x06 };

    uint8_t* rx_data = _ssrv->rx_pdu->data;

    const uint8_t byte_count = rx_data[0];

    const uint8_t* const rx_data_end = rx_data + _ssrv->rx_pdu->data_size;

    if(byte_count <= 0x09 || byte_count >= 0xFB) {
        return MBE_ILLEGAL_DATA_VALUE;
    }


    _ssrv->tx_pdu->function = 0x15;
    if(_ssrv->tx_pdu->max_size < _ssrv->rx_pdu->data_size)
        return MBE_SLAVE_FAILURE;
    // Copy all request data to response.
    _ssrv->tx_pdu->data_size = _ssrv->rx_pdu->data_size;
    memcpy(_ssrv->tx_pdu->data, rx_data, _ssrv->rx_pdu->data_size);

    ++rx_data;

    while(rx_data < rx_data_end) {

        struct emb_srv_file_t* file;

        if(*rx_data != EMB_FILE_REF_TYPE)
            return MBE_ILLEGAL_DATA_ADDR;

        file = _srv->get_file(_srv, GET_BIG_END16(rx_data + 1)/*, start_addr*/);

        if((file) && (file->write_file) /*&& ((file->start + file->size) >= (start_addr + reg_count))*/) {

            uint8_t res;
            uint16_t j;

            const uint16_t start_addr = GET_BIG_END16(rx_data + 3);
            const uint16_t reg_count = GET_BIG_END16(rx_data + 5);

            // Skip ref-type, fileno, start_addr and reg_count.
            rx_data += (sizeof(uint8_t) + 3*sizeof(uint16_t));

            if((rx_data + (reg_count*2)) > rx_data_end)
                return MBE_ILLEGAL_DATA_VALUE;

            for(j=0; j<reg_count; ++j) {
                const uint16_t tmp = ((uint16_t*)rx_data)[j];
                ((uint16_t*)rx_data)[j] = SWAP_BYTES(tmp);
            }

            res = file->write_file(file,
                                   start_addr,
                                   reg_count,
                                   (uint16_t*)rx_data);
            if(res)
                return res;

            // skip data
            rx_data += reg_count*2;
        }
        else {
            return MBE_ILLEGAL_DATA_ADDR;
        }
    }

    return 0;
}
