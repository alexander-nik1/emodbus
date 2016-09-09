#ifndef MODBUS_MASTER_READ_FILE_RECORD
#define MODBUS_MASTER_READ_FILE_RECORD

/*!
 * \file
 * \brief Declaration of Read File Record functions.
 *
 * Functions for working with Read File Record function.
 *
 */

#include <emodbus/base/modbus_pdu.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct emb_read_file_sub_answer_t;

/**
 * @brief The subanswer type.
 *
 * This is a hidden type, you must never try to read/write data by this
 * pointer by yourself. Here you have a emb_read_file_subanswer_quantity()
 * and emb_read_file_subanswer_data() functions to read data from subanswer.
 */
typedef const struct emb_read_file_sub_answer_t emb_read_file_subansw_t;

/**
 * @brief The emb_read_file_req_t struct
 *
 * This structure describes a one subrequest.
 */
typedef struct _emb_read_file_req_t {
    uint16_t file_number;       ///< File number (file id)
    uint16_t record_number;     ///< Record number (starting address for reading)
    uint16_t record_length;     ///< Record length (how much registers will be read from)
} emb_read_file_req_t;

/**
 * @brief Calculate the request size
 * @param[in] _sub_resuests_number The number of subrequests.
 * @return The size of request's data size
 */
int emb_read_file_calc_req_data_size(int _sub_resuests_number);

/**
 * @brief Calculate the answer size
 * @param[in] _sub_resuests Subrequests array.
 * @param[in] _sub_resuests_number The number of subrequests.
 * @return The size of answer's data size
 */
int emb_read_file_calc_answer_data_size(const emb_read_file_req_t* _sub_resuests,
                                        int _sub_resuests_number);

/**
 * @brief Build request
 *
 * This fucntion builds the "Read File Record" request.
 *
 * @param[out] _result_req Result request.
 * @param[in] _sub_resuests Subrequests array.
 * @param[in] _sub_resuests_number The number of subrequests.
 * @return Zero if a request is ready, otherwise error code.
 */
int emb_read_file_make_req(emb_pdu_t* _result_req,
                           const emb_read_file_req_t* _sub_resuests,
                           int _sub_resuests_number);

/**
 * @brief Get answer's byte count
 *
 * This function returns a "byte count" of answer.
 *
 * @param[in] _answer The answer
 * @return byte count.
 */
int emb_read_file_get_answer_length(emb_const_pdu_t* _answer);

/**
 * @brief Find subanswer
 *
 * This function finds a subanswer by a order number within
 * answer and returns it. You must know: this function is a
 * loop-based searching in the answer.
 *
 * @param[in] _answer The answer
 * @param[in] _sub_answer_number Order-number of sub-request.
 * @return Subrequest, or zero, if it was not found.
 */
const struct emb_read_file_sub_answer_t* emb_read_file_find_subanswer(emb_const_pdu_t* _answer,
                                                                      int _sub_answer_number);

/**
 * @brief Get first subanswer
 *
 * Returns a first subanswer for this answer.
 *
 * @param[in] _answer The answer
 * @return Subrequest, or zero, if it was not found.
 */
#define emb_read_file_first_subanswer(_answer_) \
    emb_read_file_next_subanswer(_answer_, NULL)

/**
 * @brief Get next subanswer
 *
 * Returns a next subanswer for this answer.
 * If _subanswer is a zero, then function shall return first subanswer.
 *
 * @param[in] _answer The answer
 * @param[in] _subanswer The previous subanswer.
 * @return Subrequest, or zero, if it was not found.
 */
emb_read_file_subansw_t* emb_read_file_next_subanswer(emb_const_pdu_t* _answer,
                                                      emb_read_file_subansw_t* _subanswer);

/**
 * @brief Quantity of subanswer
 *
 * This function returns the quantity of
 * registers in given subanswer.
 *
 * @param[in] _subanswer The subanswer
 * @return
 */
uint16_t emb_read_file_subanswer_quantity(emb_read_file_subansw_t* _subanswer);

/**
 * @brief Subanswer's data
 *
 * By using this function you can read data
 * from subanswer.
 *
 * @param[in] _subanswer The subanswer
 * @param[in] _offset The offset within subanswer.
 * @return
 */
uint16_t emb_read_file_subanswer_data(emb_read_file_subansw_t* _subanswer,
                                      uint16_t _offset);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // MODBUS_MASTER_READ_FILE_RECORD
