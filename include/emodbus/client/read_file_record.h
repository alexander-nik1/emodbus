#ifndef MODBUS_MASTER_READ_FILE_RECORD
#define MODBUS_MASTER_READ_FILE_RECORD

#include <emodbus/base/modbus_pdu.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct emb_read_file_sub_answer_t;

typedef const struct emb_read_file_sub_answer_t emb_read_file_subansw_t;

struct _emb_read_file_req_t {
    uint16_t file_number;
    uint16_t record_number;
    uint16_t record_length;
};

typedef struct _emb_read_file_req_t emb_read_file_req_t;

int emb_read_file_calc_req_data_size(int _sub_resuests_number);

int emb_read_file_calc_answer_data_size(const emb_read_file_req_t* _sub_resuests,
                                        int _sub_resuests_number);

int emb_read_file_make_req(emb_pdu_t* _result_req,
                           const emb_read_file_req_t* _sub_resuests,
                           int _sub_resuests_number);

int emb_read_file_get_answer_length(emb_const_pdu_t* _answer);

const struct emb_read_file_sub_answer_t* emb_read_file_find_subanswer(emb_const_pdu_t* _answer,
                                                       int _sub_answer_number);

emb_read_file_subansw_t* emb_read_file_first_subanswer(emb_const_pdu_t* _answer);

emb_read_file_subansw_t* emb_read_file_next_subanswer(emb_const_pdu_t* _answer,
                                                      emb_read_file_subansw_t* _subanswer);

uint16_t emb_read_file_subanswer_quantity(emb_read_file_subansw_t* _subanswer);

uint16_t emb_read_file_subanswer_data(emb_read_file_subansw_t* _subanswer,
                                 uint16_t _offset);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_READ_FILE_RECORD
