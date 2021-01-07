
#ifndef EMB_BIT_ARRAY_H
#define EMB_BIT_ARRAY_H

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t emb_ba_word_t;

enum { EMB_BA_N_WORD_BYTES = sizeof(emb_ba_word_t) };

enum { EMB_BA_N_WORD_BITS = EMB_BA_N_WORD_BYTES * 8 };

/**
 * @brief Copy bits
 *
 * Performs a bit-copying operation
 *
 * @param [in] _arr Source bit-array
 * @param [in] _arr_size The source size, in bytes
 * @param [out] _result The place to copy into
 * @param [in] _bits_offset The offset in source to begin copy from it
 * @param [in] _quantity Number of bits to be copied
 *
 * @return Zero if ok, or negative error code.
 */

int emb_bit_arr_get_bits(const emb_ba_word_t* _arr,
                         size_t _arr_size,
                         emb_ba_word_t* _result,
                         size_t _bits_offset,
                         size_t _quantity);

/**
 * @brief Insert bits
 *
 * Performs a bit-inserting operation
 *
 * @param [out] _arr Destination bit-array
 * @param [in] _arr_size The Destination size, in bytes
 * @param [in] _bits The source to copy from it
 * @param [in] _bits_offset The offset in destination to begin insert into
 * @param [in] _quantity Number of bits to be inserted
 *
 * @return Zero if ok, or negative error code.
 */

int emb_bit_arr_set_bits(emb_ba_word_t* _arr,
                         size_t _arr_size,
                         const emb_ba_word_t* _bits,
                         size_t _bits_offset,
                         size_t _quantity);

/**
 * @brief Compare bits
 *
 * Performs a bit-comparing operation
 *
 * @param [in] _a1 A bit-array to compare with _a2
 * @param [in] _a2 A bit-array to compare with _a1
 * @param [in] _quantity Number of bits to be compared
 *
 * @return Zero if both arrays is match, or non-zero if arrays is different
 */

int emb_bit_arr_cmp(const void* _a1, const void* _a2, size_t _quantity);

#ifdef __cplusplus
}
#endif

#endif // EMB_BIT_ARRAY_H
