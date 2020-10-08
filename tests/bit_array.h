
#ifndef BIT_ARRAY_H
#define BIT_ARRAY_H

#include <stdlib.h>
#include "bit_array.conf.h"

enum { BA_N_WORD_BYTES = sizeof(ba_word_t) };

enum { BA_N_WORD_BITS = BA_N_WORD_BYTES * 8 };

int bit_arr_get_bits(const ba_word_t* _arr, size_t _arr_size, ba_word_t* _result, size_t _bits_offset, size_t _quantity);
int bit_arr_set_bits(ba_word_t* _arr, size_t _arr_size, const ba_word_t* _bits, size_t _bits_offset, size_t _quantity);
int bit_arr_cmp(const void* _a1, const void* _a2, size_t _quantity);

#endif // BIT_ARRAY_H
