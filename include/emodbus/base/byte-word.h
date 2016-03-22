
#ifndef BYTE_WORD_H 
#define BYTE_WORD_H

#include <stdint.h>

#define SET16BIT_REGISTER(reg_h, reg_l, value)	\
    reg_h = (unsigned char)(value >> 8);		\
    reg_l = (unsigned char)(value);

#define GET16BIT_REGISTER(reg_h, reg_l, var)	\
    var = reg_l;	\
    var |= (((int)reg_h) << 8);

#define GET16BIT_REG(reg_h, reg_l)  (((uint16_t)(reg_h) << 8) | (uint16_t)(reg_l))

#define UINT_FROM_PTR(ptr)  (*((unsigned int*)(ptr)))
#define ULONG_FROM_PTR(ptr)  (*((unsigned long*)(ptr)))

#define MKWORD(lbyte, hbyte)    (((uint16_t)((uint8_t)(lbyte))) | (((uint16_t)((uint8_t)(hbyte))) << 8))
#define MKDWORD(lword, hword)   (((uint32_t)((uint16_t)(lword))) | (((uint32_t)((uint16_t)(hword)) << 8))

#define LIT_END_MK16(ptr, word)     ((unsigned char*)(ptr))[1] = ((word) >> 8), ((unsigned char*)(ptr))[0] = (word);
#define GET_LIT_END16(ptr)          (((uint16_t)((unsigned char*)(ptr))[1]) << 8 | ((uint16_t)((unsigned char*)(ptr))[0]))

#define BIG_END_MK16(ptr, word)     ((unsigned char*)(ptr))[0] = ((word) >> 8), ((unsigned char*)(ptr))[1] = (word);
#define GET_BIG_END16(ptr)          (((uint16_t)((unsigned char*)(ptr))[0]) << 8 | ((uint16_t)((unsigned char*)(ptr))[1]))

#define SWAP_BYTES(x)               ((((uint16_t)(x)) >> 8) | (((uint16_t)(x)) << 8))
#define SWAP_WORDS(x)               ((((uint32_t)(x)) >> 16) | (((uint32_t)(x)) << 16))

#define LOBYTE(word)            ((uint8_t)(word))
#define HIBYTE(word)            (((uint8_t)(word)) >> 8)

#define LOWORD(dword)            ((uint16_t)(dword))
#define HIWORD(dword)            (((uint16_t)(dword)) >> 16)

#endif	// BYTE_WORD_H
