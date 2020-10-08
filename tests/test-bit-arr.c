
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "bit_array.h"

static ba_word_t buffer[4] = { 0x11, 0x22, 0x33, 0x77 };
enum { BUF_SIZE = sizeof(buffer)/sizeof(buffer[0]) };

void print_buffer(const ba_word_t* _buffer, size_t _n_bits)
{
    size_t i;
    fputc(' ', stdout);
    for (i=0; i<_n_bits; ++i) {
        const ba_word_t w = _buffer[i/BA_N_WORD_BITS];
        const char bit = !!(w & (1 << (i % BA_N_WORD_BITS)));
//        if(!(i % N_W_BITS))
//            fputc(' ', stdout);
        fputc(bit ? '1' : '0', stdout);
    }
    fputc('\n', stdout);
}



void randomize_bits(ba_word_t* _buf, size_t _sz)
{
    while((_sz--))
        *_buf++ = rand()*rand();
}

int main(void)
{
    srand((unsigned int)time(NULL));
    printf("Test bit array\n");

    int r;
    size_t i;
//    word_t w[4];
//    size_t s = 17;

//    memset(buffer, 0x00, sizeof(buffer));
//    print_buffer(buffer, BUF_SIZE*N_W_BITS);

//    set_arr_word(buffer, 0xFF, 1, 5);
//    print_buffer(buffer, BUF_SIZE*N_W_BITS);

//    size_t sz = BUF_SIZE*N_W_BITS - s + 1;

    int errors = 0;

    for(i=0; i<10000000; ++i) {
        ba_word_t ww[4];
        ba_word_t rw[4];

        size_t s = ((size_t)rand() % 20) + 1;
        size_t o = ((size_t)rand() % (BUF_SIZE*BA_N_WORD_BITS - s));

        randomize_bits(buffer, BUF_SIZE);

        randomize_bits(ww, sizeof(ww)/sizeof(ww[0]));

        memset(rw, 0, sizeof(rw));

        bit_arr_set_bits(buffer, BUF_SIZE, ww, o, s);

        bit_arr_get_bits(buffer, BUF_SIZE, rw, o, s);

        int res = bit_arr_cmp(ww, rw, s);
        errors += res;
        if(res) {
            print_buffer(buffer, BUF_SIZE*BA_N_WORD_BITS);
            print_buffer(ww, BUF_SIZE*BA_N_WORD_BITS);
            print_buffer(rw, BUF_SIZE*BA_N_WORD_BITS);
        }
    }

    printf("errors: %d\n", errors);

//    for(i=0; i<(BUF_SIZE*N_W_BITS-s+1); ++i) {
//        //r = get_arr_word(buffer, BUF_SIZE, w, (size_t)i, s);
//        r = get_arr_bits(buffer, BUF_SIZE, w, (size_t)i, s);
//        if(r != 0)
//            printf("Error: %d\n", r);

//        size_t j;
//        for(j=0; j<i; ++j)
//            fputc(' ', stdout);

//        print_buffer(w, s);
//    }

    return 0;
}
