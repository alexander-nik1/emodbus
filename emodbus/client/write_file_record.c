
#include <emodbus/client/write_file_record.h>
#include <errno.h>
#include <stdlib.h>
#include <emodbus/base/byte-word.h>

/*!
 * \file
 * \brief Definition of Write File Record functions.
 *
 * Functions for working with Write File Record function.
 *
 */

struct sub_req_t {
    uint8_t ref_type;
    uint16_t file_number;
    uint16_t record_number;
    uint16_t record_length;
    uint16_t data[1];
} __attribute__ ((packed));

int emb_write_file_calc_req_data_size(const emb_write_file_req_t* _sub_resuests,
                                      int _sub_resuests_number) {

    int i, result = 1;

    if(!_sub_resuests)
        return -EINVAL;

    for(i=0; i<_sub_resuests_number; ++i)
        result += 7 + 2 * _sub_resuests[i].record_length;
    return result;
}

int emb_write_file_make_req(emb_pdu_t* _result_req,
                            const emb_write_file_req_t* _sub_resuests,
                            int _sub_resuests_number) {

    int i,j;

    uint8_t* sub_req_iterator;

    const int req_size = emb_write_file_calc_req_data_size(_sub_resuests,
                                                           _sub_resuests_number);

    if((!_sub_resuests) || req_size <= 0 || (!_result_req))
        return -EINVAL;

    sub_req_iterator = ((uint8_t*)_result_req->data);

    if(_result_req->max_size < req_size) {
        return -ENOMEM;
    }

    _result_req->function = 0x15;
    _result_req->data_size = req_size;

    sub_req_iterator[0] = req_size-1;

    sub_req_iterator++; // skip "byte-count" field

    for(i=0; i<_sub_resuests_number; ++i) {

        uint16_t tmp;

        struct sub_req_t* curr_subreq = (struct sub_req_t*)sub_req_iterator;

        if(_sub_resuests[i].record_number > 0x270F)
            return -EINVAL;

        curr_subreq->ref_type = 6;

        tmp = _sub_resuests[i].file_number;
        curr_subreq->file_number = SWAP_BYTES(tmp);

        tmp = _sub_resuests[i].record_number;
        curr_subreq->record_number = SWAP_BYTES(tmp);

        tmp = _sub_resuests[i].record_length;
        curr_subreq->record_length = SWAP_BYTES(tmp);

        for(j=0; j<tmp; ++j) {
            const uint16_t data = _sub_resuests[i].data[j];
            curr_subreq->data[j] = SWAP_BYTES(data);
        }

        // Jump to the next sub-request
        sub_req_iterator += (7 + 2 * tmp);
    }

    return 0;
}
