
#include <emodbus/base/bit_array.h>
#include <errno.h>
#include <string.h>

static int emb_bit_arr_get_word(const emb_ba_word_t* _arr, emb_ba_word_t* _result, size_t _bits_offset, size_t _quantity)
{
    size_t wo = _bits_offset / EMB_BA_N_WORD_BITS;
    size_t bo = _bits_offset % EMB_BA_N_WORD_BITS;
    if(_arr && _result && _quantity <= EMB_BA_N_WORD_BITS) {
        emb_ba_word_t w1 = _arr[wo];
        emb_ba_word_t w2 = bo ? _arr[wo+1] : 0;
        *_result = ((w1 >> bo) | (w2 << (EMB_BA_N_WORD_BITS - bo))) & (emb_ba_word_t)((1ull << _quantity) - 1ull);
        return 0;
    }
    else {
        return -EINVAL;
    }
}

static int emb_bit_arr_set_word(emb_ba_word_t* _arr, emb_ba_word_t _w, size_t _bits_offset, size_t _quantity)
{
    size_t wo = _bits_offset / EMB_BA_N_WORD_BITS;
    size_t bo = _bits_offset % EMB_BA_N_WORD_BITS;
    if(_arr && _quantity <= EMB_BA_N_WORD_BITS) {
        _w &= ((1ull << _quantity) - 1ull);
        emb_ba_word_t w1 = (emb_ba_word_t)(_w << bo);
        emb_ba_word_t w2 = _w >> (EMB_BA_N_WORD_BITS - bo);
        unsigned long long mask = ((1ull << _quantity) - 1ull) << bo;
        _arr[wo] &= ~mask;
        _arr[wo] |= w1;
        if((bo + _quantity) > EMB_BA_N_WORD_BITS) {
            mask >>= EMB_BA_N_WORD_BITS;
            _arr[wo+1] &= ~mask;
            _arr[wo+1] |= w2;
        }
        return 0;
    }
    else {
        return -EINVAL;
    }
}

int emb_bit_arr_get_bits(const emb_ba_word_t* _arr, size_t _arr_size, emb_ba_word_t* _result, size_t _bits_offset, size_t _quantity)
{
    if(_arr && _result && (_arr_size * EMB_BA_N_WORD_BITS) >= (_bits_offset + _quantity)) {
        size_t bit_count = 0;
        while(bit_count < _quantity) {
            int r;
            size_t q = _quantity - bit_count;
            if(q > EMB_BA_N_WORD_BITS)
                q = EMB_BA_N_WORD_BITS;
            r = emb_bit_arr_get_word(_arr, _result++, _bits_offset + bit_count, q);
            if(r != 0)
                return r;
            bit_count += q;
        }
        return 0;
    }
    else {
        return -EINVAL;
    }
}

int emb_bit_arr_set_bits(emb_ba_word_t* _arr, size_t _arr_size, const emb_ba_word_t* _bits, size_t _bits_offset, size_t _quantity)
{
    if(_arr && _bits && (_arr_size * EMB_BA_N_WORD_BITS) >= (_bits_offset + _quantity)) {
        size_t bit_count = 0;
        while(bit_count < _quantity) {
            int r;
            size_t q = _quantity - bit_count;
            if(q > EMB_BA_N_WORD_BITS)
                q = EMB_BA_N_WORD_BITS;
            r = emb_bit_arr_set_word(_arr, *_bits++, _bits_offset + bit_count, q);
            if(r != 0)
                return r;
            bit_count += q;
        }
        return 0;
    }
    else {
        return -EINVAL;
    }
}

int emb_bit_arr_cmp(const void* _a1, const void* _a2, size_t _quantity)
{
    const size_t ws = _quantity / 8;
    const size_t bo = _quantity % 8;
    if(ws && memcmp(_a1, _a2, ws))
        return 1;
    if(bo) {
        const uint8_t m = (uint8_t)((1 << bo) - 1);
        return (((const uint8_t*)_a1)[ws] & m) != (((const uint8_t*)_a2)[ws] & m);
    }
    return 0;
}
