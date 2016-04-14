
#ifndef EMODBUS_SERVER_FILES_H
#define EMODBUS_SERVER_FILES_H

#include <stdint.h>
#include <emodbus/base/modbus_pdu.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { EMB_FILE_REF_TYPE = 0x06 };

struct emb_srv_file_t {

    uint16_t fileno;
    uint16_t start;
    uint16_t size;

    uint8_t (*read_file)(struct emb_srv_file_t* _f,
                         emb_const_pdu_t* _req,
                         uint16_t _offset,
                         uint16_t _quantity,
                         uint16_t* _pvalues);

    uint8_t (*write_file)(struct emb_srv_file_t* _f,
                          emb_const_pdu_t* _req,
                          uint16_t _offset,
                          uint16_t _quantity,
                          const uint16_t* _pvalues);
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif //EMODBUS_SERVER_FILES_H
