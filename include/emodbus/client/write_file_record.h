#ifndef MODBUS_MASTER_WRITE_FILE_RECORD
#define MODBUS_MASTER_WRITE_FILE_RECORD

/*!
 * \file
 * \brief Declaration of Write File Record functions.
 *
 * Functions for working with Write File Record function.
 *
 */

#include <emodbus/base/modbus_pdu.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The emb_write_file_req_t struct
 *
 * This structure describes a one subrequest.
 */
typedef struct _emb_write_file_req_t {
    uint16_t file_number;       ///< File number (file id)
    uint16_t record_number;     ///< Record number (starting address for writing)
    uint16_t record_length;     ///< Record length (how much registers will be write to)
    const uint16_t* data;       ///< Data to write
} emb_write_file_req_t;

/**
 * @brief Calculate the request size
 * @param[in] _sub_resuests_number The number of subrequests.
 * @return The size of request's data size
 */
int emb_write_file_calc_req_data_size(const emb_write_file_req_t *_sub_resuests,
                                      int _sub_resuests_number);

/**
 * @brief Calculate the answer size
 * @param[in] _sub_resuests Subrequests array.
 * @param[in] _sub_resuests_number The number of subrequests.
 * @return The size of answer's data size
 */
#define emb_write_file_calc_answer_data_size(_sub_resuests_, _sub_resuests_number_)   \
    emb_write_file_calc_req_data_size(_sub_resuests_, _sub_resuests_number_)

/**
 * @brief Build request
 *
 * This fucntion builds the "Write File Record" request.
 *
 * @param[out] _result_req Result request.
 * @param[in] _sub_resuests Subrequests array.
 * @param[in] _sub_resuests_number The number of subrequests.
 * @return Zero if a request is ready, otherwise error code.
 */
int emb_write_file_make_req(emb_pdu_t* _result_req,
                            const emb_write_file_req_t* _sub_resuests,
                            int _sub_resuests_number);


#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_WRITE_FILE_RECORD
