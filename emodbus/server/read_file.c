
#include <emodbus/server/server.h>
#include <emodbus/server/file.h>
#include <emodbus/base/byte-word.h>
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/calc_pdu_size.h>
#include <stdint.h>

uint8_t emb_srv_read_file(struct emb_super_server_t* _ssrv,
                          struct emb_server_t* _srv) {

    enum { request_size = 7 };

    uint8_t* rx_data = _ssrv->rx_pdu->data;
    uint8_t* tx_data = _ssrv->tx_pdu->data;

    const uint8_t byte_count = rx_data[0];
    unsigned char byte_counter=0, i;

    if(byte_count < request_size || byte_count > (request_size*35))
        return MBE_ILLEGAL_DATA_VALUE;

    ++rx_data;
    ++tx_data;

    for(i=0; i<byte_count; i += request_size) {
        const uint16_t file_number = GET_BIG_END16(rx_data + 1);
        const uint16_t start_addr = GET_BIG_END16(rx_data + 3);
        const uint16_t reg_count = GET_BIG_END16(rx_data + 5);

        struct emb_srv_file_t* file;

        if(!_srv->get_file)
            return MBE_SLAVE_FAILURE;

        file = _srv->get_file(_srv, file_number/*, start_addr*/);

        if((rx_data[0] == EMB_FILE_REF_TYPE) && (file) /*&& ((file->start + file->size) >= (start_addr + reg_count))*/) {

            uint8_t res;
            uint16_t j;
            const unsigned char bytes = reg_count * 2 + 1;

            *tx_data++ = bytes;
            byte_counter += bytes + 1;

            *tx_data++ = EMB_FILE_REF_TYPE;

            if(!file->read_file)
                return MBE_ILLEGAL_DATA_ADDR;

            if(_ssrv->tx_pdu->max_size < byte_counter)
                return MBE_SLAVE_FAILURE;

            res = file->read_file(file,
                                  start_addr,
                                  reg_count,
                                  (uint16_t*)(tx_data));
            if(res)
                return res;

            for(j=0; j<reg_count; ++j) {
                const uint16_t tmp = *((uint16_t*)tx_data);
                *((uint16_t*)tx_data) = SWAP_BYTES(tmp);
                tx_data += 2;
            }
        }
        else {
            return MBE_ILLEGAL_DATA_ADDR;
        }
        rx_data += request_size;
    }

    tx_data = _ssrv->tx_pdu->data;
    *tx_data = byte_counter;

    _ssrv->tx_pdu->data_size = byte_counter + 1;
    _ssrv->tx_pdu->function = 0x14;

    return 0;
}
