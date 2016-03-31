
#include <emodbus/client/read_file_record.h>
#include <errno.h>
#include <stdlib.h>
#include <emodbus/base/byte-word.h>

struct sub_req_t {
    uint8_t ref_type;
    uint16_t file_number;
    uint16_t record_number;
    uint16_t record_length;
} __attribute__ ((packed));

struct emb_read_file_sub_answer_t {
    uint8_t length;
    uint8_t ref_type;
    uint16_t data[1];
} __attribute__ ((packed));

int emb_read_file_calc_req_data_size(int _sub_resuests_number) {

    return 1 + 7 * _sub_resuests_number;
}

int emb_read_file_calc_answer_data_size(const emb_read_file_req_t* _sub_resuests,
                                        int _sub_resuests_number) {
    int i, result = 1;
    for(i=0; i<_sub_resuests_number; ++i)
        result += 2 + 2 * _sub_resuests[i].record_length;
    return result;
}

int emb_read_file_make_req(emb_pdu_t* _result_req,
                           const emb_read_file_req_t* _sub_resuests,
                           int _sub_resuests_number) {

    int i;

    const int req_size = emb_read_file_calc_req_data_size(_sub_resuests_number);

    struct sub_req_t* reqs;

    if(_result_req->max_size < req_size) {
        return -ENOMEM;
    }

    _result_req->function = 0x14;
    _result_req->data_size = req_size;

    reqs = (struct sub_req_t*)(((uint8_t*)_result_req->data) + 1);

    ((uint8_t*)_result_req->data)[0] = req_size-1;

    for(i=0; i<_sub_resuests_number; ++i) {

        if(_sub_resuests[i].record_number > 0x270F)
            return -EINVAL;

        reqs[i].ref_type = 6;
        reqs[i].file_number = SWAP_BYTES(_sub_resuests[i].file_number);
        reqs[i].record_number = SWAP_BYTES(_sub_resuests[i].record_number);
        reqs[i].record_length = SWAP_BYTES(_sub_resuests[i].record_length);
    }

    return 0;
}

int emb_read_file_get_answer_length(emb_const_pdu_t* _answer) {
    return ((uint8_t*)_answer->data)[0];
}

emb_read_file_subansw_t* emb_read_file_first_subanswer(emb_const_pdu_t* _answer) {
    return emb_read_file_next_subanswer(_answer, NULL);
}

emb_read_file_subansw_t* emb_read_file_next_subanswer(emb_const_pdu_t* _answer,
                                                      emb_read_file_subansw_t* _subanswer) {

    emb_read_file_subansw_t* sa;

    const int answer_size = emb_read_file_get_answer_length(_answer) + 1;

    unsigned int offs_to_next, end_of_answer;

    if(_answer->data_size != answer_size)
        return NULL;

    if(!_subanswer) {
        sa = (emb_read_file_subansw_t*)(((uint8_t*)_answer->data)+1);
    }
    else {
        sa = (emb_read_file_subansw_t*)(((uint8_t*)_subanswer) + _subanswer->length + 1);
    }

    offs_to_next = (((unsigned int)sa) + sa->length + 1);
    end_of_answer = (((unsigned int)_answer->data) +  answer_size);

    if(offs_to_next > end_of_answer)
        return NULL;

    if(sa->ref_type != 6)
        return NULL;
    else
        return sa;
}

emb_read_file_subansw_t* emb_read_file_find_subanswer(emb_const_pdu_t* _answer,
                                          int _sub_answer_number) {

    int i;
    emb_read_file_subansw_t* sa = NULL;

    ++_sub_answer_number;

    for(i=0; i<_sub_answer_number; ++i) {
        sa = emb_read_file_next_subanswer(_answer, sa);
        if(!sa)
            break;
    }

    return sa;
}

uint16_t emb_read_file_subanswer_quantity(emb_read_file_subansw_t* _subanswer) {
    return (uint16_t)(_subanswer->length >> 1);
}

uint16_t emb_read_file_subanswer_data(emb_read_file_subansw_t* _subanswer,
                                 uint16_t _offset) {

    uint16_t x;
    if(_offset > (_subanswer->length >> 1))
        return -1;

    x = _subanswer->data[_offset];

    return SWAP_BYTES(x);
}
