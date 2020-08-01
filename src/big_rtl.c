/********************************************************************/
/*                                                                  */
/*  big_rtl.c     Functions for the built-in bigInteger support.    */
/*  Copyright (C) 1989 - 2014  Thomas Mertes                        */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/big_rtl.c                                       */
/*  Changes: 2005, 2006, 2008, 2009, 2010, 2013, 2014 Thomas Mertes */
/*  Content: Functions for the built-in bigInteger support.         */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "int_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "big_drv.h"


#define KARATSUBA_THRESHOLD 32
#define KARATSUBA_SQUARE_THRESHOLD 32


/* Defines to describe a bigdigit:                                  */
/* BIGDIGIT_MASK              All bits in a bigdigit are set.       */
/* BIGDIGIT_SIGN              The highest bit of a bigdigit is set. */
/* POWER_OF_10_IN_BIGDIGIT    The biggest power of 10 which fits    */
/*                            in a bigdigit.                        */
/* DECIMAL_DIGITS_IN_BIGDIGIT The number of zero digits in          */
/*                            POWER_OF_10_IN_BIGDIGIT.              */


#if BIGDIGIT_SIZE == 8

/* typedef uint8Type             bigDigitType; */
typedef int8Type                 signedBigDigitType;
typedef uint16Type               doubleBigDigitType;
#define digitMostSignificantBit  uint8MostSignificantBit
#define digitLeastSignificantBit uint8LeastSignificantBit
#define BIGDIGIT_MASK                    0xFF
#define BIGDIGIT_SIGN                    0x80
#define BIGDIGIT_SIZE_MASK                0x7
#define BIGDIGIT_LOG2_SIZE                  3
#define POWER_OF_10_IN_BIGDIGIT           100
#define DECIMAL_DIGITS_IN_BIGDIGIT          2
bigDigitType powerOfRadixInBigdigit[] = {
    /*  2 */ 128, 243,  64, 125, 216,
    /*  7 */  49,  64,  81, 100, 121,
    /* 12 */ 144, 169, 196, 225,  16,
    /* 17 */  17,  18,  19,  20,  21,
    /* 22 */  22,  23,  24,  25,  26,
    /* 27 */  27,  28,  29,  30,  31,
    /* 32 */  32,  33,  34,  35,  36
  };
uint8Type radixDigitsInBigdigit[] = {
    /*  2 */  7,  5,  3,  3,  3,  2,  2,  2,  2,  2,
    /* 12 */  2,  2,  2,  2,  1,  1,  1,  1,  1,  1,
    /* 22 */  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    /* 32 */  1,  1,  1,  1,  1
  };

#elif BIGDIGIT_SIZE == 16

/* typedef uint16Type            bigDigitType; */
typedef int16Type                signedBigDigitType;
typedef uint32Type               doubleBigDigitType;
#define digitMostSignificantBit  uint16MostSignificantBit
#define digitLeastSignificantBit uint16LeastSignificantBit
#define BIGDIGIT_MASK                  0xFFFF
#define BIGDIGIT_SIGN                  0x8000
#define BIGDIGIT_SIZE_MASK                0xF
#define BIGDIGIT_LOG2_SIZE                  4
#define POWER_OF_10_IN_BIGDIGIT         10000
#define DECIMAL_DIGITS_IN_BIGDIGIT          4
bigDigitType powerOfRadixInBigdigit[] = {
    /*  2 */ 32768, 59049, 16384, 15625, 46656,
    /*  7 */ 16807, 32768, 59049, 10000, 14641,
    /* 12 */ 20736, 28561, 38416, 50625,  4096,
    /* 17 */  4913,  5832,  6859,  8000,  9261,
    /* 22 */ 10648, 12167, 13824, 15625, 17576,
    /* 27 */ 19683, 21952, 24389, 27000, 29791,
    /* 32 */ 32768, 35937, 39304, 42875, 46656
  };
uint8Type radixDigitsInBigdigit[] = {
    /*  2 */ 15, 10,  7,  6,  6,  5,  5,  5,  4,  4,
    /* 12 */  4,  4,  4,  4,  3,  3,  3,  3,  3,  3,
    /* 22 */  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    /* 32 */  3,  3,  3,  3,  3
  };

#elif BIGDIGIT_SIZE == 32

/* typedef uint32Type            bigDigitType; */
typedef int32Type                signedBigDigitType;
typedef uint64Type               doubleBigDigitType;
#define digitMostSignificantBit  uint32MostSignificantBit
#define digitLeastSignificantBit uint32LeastSignificantBit
#define BIGDIGIT_MASK              0xFFFFFFFF
#define BIGDIGIT_SIGN              0x80000000
#define BIGDIGIT_SIZE_MASK               0x1F
#define BIGDIGIT_LOG2_SIZE                  5
#define POWER_OF_10_IN_BIGDIGIT    1000000000
#define DECIMAL_DIGITS_IN_BIGDIGIT          9
bigDigitType powerOfRadixInBigdigit[] = {
    /*  2 */ 2147483648u, 3486784401u, 1073741824u, 1220703125u, 2176782336u,
    /*  7 */ 1977326743u, 1073741824u, 3486784401u, 1000000000u, 2357947691u,
    /* 12 */  429981696u,  815730721u, 1475789056u, 2562890625u,  268435456u,
    /* 17 */  410338673u,  612220032u,  893871739u, 1280000000u, 1801088541u,
    /* 22 */ 2494357888u, 3404825447u,  191102976u,  244140625u,  308915776u,
    /* 27 */  387420489u,  481890304u,  594823321u,  729000000u,  887503681u,
    /* 32 */ 1073741824u, 1291467969u, 1544804416u, 1838265625u, 2176782336u
  };
uint8Type radixDigitsInBigdigit[] = {
    /*  2 */ 31, 20, 15, 13, 12, 11, 10, 10,  9,  9,
    /* 12 */  8,  8,  8,  8,  7,  7,  7,  7,  7,  7,
    /* 22 */  7,  7,  6,  6,  6,  6,  6,  6,  6,  6,
    /* 32 */  6,  6,  6,  6,  6
  };

#endif


#define IS_NEGATIVE(digit) (((digit) & BIGDIGIT_SIGN) != 0)

static const int digit_value[] = {
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,              /* Digits 0 - 9   */
    -1, -1, -1, -1, -1, -1, -1,                          /* Illegal digits */
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,  /* Digits A - M */
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,  /* Digits N - Z */
    -1, -1, -1, -1, -1, -1,                              /* Illegal digits */
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,  /* Digits a - m */
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35   /* Digits n - z */
  };


#ifdef DO_HEAP_STATISTIC
size_t sizeof_bigDigitType = sizeof(bigDigitType);
size_t sizeof_bigIntRecord = sizeof(bigIntRecord);
#endif


#define SIZ_RTLBIG(len)  ((sizeof(bigIntRecord) - sizeof(bigDigitType)) + (len) * sizeof(bigDigitType))
#define MAX_BIG_LEN      ((MAX_MEMSIZETYPE - sizeof(bigIntRecord) + sizeof(bigDigitType)) / sizeof(bigDigitType))

#ifdef WITH_BIGINT_CAPACITY
#define HEAP_ALLOC_BIG(var,len)       (ALLOC_HEAP(var, bigIntType, SIZ_RTLBIG(len))?((var)->capacity = len, CNT(CNT1_BIG(len, SIZ_RTLBIG(len))) TRUE):FALSE)
#define HEAP_FREE_BIG(var,len)        (CNT(CNT2_BIG(len, SIZ_RTLBIG(len))) FREE_HEAP(var, SIZ_RTLBIG(len)))
#define HEAP_REALLOC_BIG(v1,v2,l1,l2) if((v1=REALLOC_HEAP(v2, bigIntType, SIZ_RTLBIG(l2)))!=NULL)(v1)->capacity=l2;
#else
#define HEAP_ALLOC_BIG(var,len)       (ALLOC_HEAP(var, bigIntType, SIZ_RTLBIG(len))?CNT(CNT1_BIG(len, SIZ_RTLBIG(len))) TRUE:FALSE)
#define HEAP_FREE_BIG(var,len)        (CNT(CNT2_BIG(len, SIZ_RTLBIG(len))) FREE_HEAP(var, SIZ_RTLBIG(len)))
#define HEAP_REALLOC_BIG(v1,v2,l1,l2) v1=REALLOC_HEAP(v2, bigIntType, SIZ_RTLBIG(l2));
#endif
#define COUNT3_BIG(len1,len2)         CNT3(CNT2_BIG(len1, SIZ_RTLBIG(len1)), CNT1_BIG(len2, SIZ_RTLBIG(len2)))

#ifdef WITH_BIGINT_FREELIST
#ifdef WITH_BIGINT_CAPACITY

#define BIG_FREELIST_ARRAY_SIZE 32

static freeListElemType flist[BIG_FREELIST_ARRAY_SIZE] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static unsigned int flist_allowed[BIG_FREELIST_ARRAY_SIZE] = {
    0, 100, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20};

#define POP_BIG_OK(len)    (len) < BIG_FREELIST_ARRAY_SIZE && flist[len] != NULL
#define PUSH_BIG_OK(var)   (var)->capacity < BIG_FREELIST_ARRAY_SIZE && flist_allowed[(var)->capacity] > 0

#define POP_BIG(var,len)   (var = (bigIntType) flist[len], flist[len] = flist[len]->next, flist_allowed[len]++, TRUE)
#define PUSH_BIG(var,len)  {((freeListElemType) var)->next = flist[len]; flist[len] = (freeListElemType) var; flist_allowed[len]--; }

#define ALLOC_BIG_SIZE_OK(var,len)    (POP_BIG_OK(len) ? POP_BIG(var, len) : HEAP_ALLOC_BIG(var, len))
#define ALLOC_BIG_CHECK_SIZE(var,len) (POP_BIG_OK(len) ? POP_BIG(var, len) : ((len)<=MAX_BIG_LEN?HEAP_ALLOC_BIG(var, len):(var=NULL, FALSE)))
#define FREE_BIG(var,len)  if (PUSH_BIG_OK(var)) PUSH_BIG(var, (var)->capacity) else HEAP_FREE_BIG(var, (var)->capacity);

#else

static freeListElemType flist = NULL;
static unsigned int flist_allowed = 100;

#define POP_BIG_OK(len)    (len) == 1 && flist != NULL
#define PUSH_BIG_OK(len)   (len) == 1 && flist_allowed > 0

#define POP_BIG(var)       (var = (bigIntType) flist, flist = flist->next, flist_allowed++, TRUE)
#define PUSH_BIG(var)      {((freeListElemType) var)->next = flist; flist = (freeListElemType) var; flist_allowed--; }

#define ALLOC_BIG_SIZE_OK(var,len)    (POP_BIG_OK(len) ? POP_BIG(var) : HEAP_ALLOC_BIG(var, len))
#define ALLOC_BIG_CHECK_SIZE(var,len) (POP_BIG_OK(len) ? POP_BIG(var) : ((len) <= MAX_BIG_LEN?HEAP_ALLOC_BIG(var, len):(var=NULL, FALSE)))
#define FREE_BIG(var,len)  if (PUSH_BIG_OK(len)) PUSH_BIG(var) else HEAP_FREE_BIG(var, len);

#endif
#else

#define ALLOC_BIG_SIZE_OK(var,len)    HEAP_ALLOC_BIG(var, len)
#define ALLOC_BIG_CHECK_SIZE(var,len) ((len) <= MAX_BIG_LEN?HEAP_ALLOC_BIG(var, len):(var=NULL, FALSE))
#define FREE_BIG(var,len)             HEAP_FREE_BIG(var, len)

#endif

#define ALLOC_BIG(var,len)                  ALLOC_BIG_CHECK_SIZE(var, len)
#define REALLOC_BIG_SIZE_OK(v1,v2,l1,l2)    HEAP_REALLOC_BIG(v1,v2,l1,l2)
#define REALLOC_BIG_CHECK_SIZE(v1,v2,l1,l2) if((l2) <= MAX_BIG_LEN){HEAP_REALLOC_BIG(v1,v2,l1,l2)}else v1=NULL;


void bigGrow (bigIntType *const big_variable, const const_bigIntType delta);
intType bigLowestSetBit (const const_bigIntType big1);
bigIntType bigLShift (const const_bigIntType big1, const intType lshift);
void bigLShiftAssign (bigIntType *const big_variable, intType lshift);
bigIntType bigNegate (const const_bigIntType big1);
bigIntType bigRem (const const_bigIntType dividend, const const_bigIntType divisor);
void bigRShiftAssign (bigIntType *const big_variable, intType rshift);
bigIntType bigSbtr (const const_bigIntType minuend, const const_bigIntType subtrahend);
void bigShrink (bigIntType *const big_variable, const const_bigIntType big2);



cstriType bigHexCStri (const const_bigIntType big1)

  {
    memSizeType pos;
    memSizeType byteCount;
    const_cstriType stri_ptr;
    cstriType buffer;
    cstriType result;

  /* bigHexCStri */
    if (likely(big1 != NULL && big1->size > 0)) {
      if (unlikely(big1->size > (MAX_CSTRI_LEN - 3) / 2 / (BIGDIGIT_SIZE >> 3) ||
          !ALLOC_CSTRI(result, big1->size * (BIGDIGIT_SIZE >> 3) * 2 + 3))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        buffer = result;
        strcpy(buffer, "16#");
        buffer += 3;
        pos = big1->size - 1;
#if BIGDIGIT_SIZE == 8
        sprintf(buffer, "%02hhx", big1->bigdigits[pos]);
#elif BIGDIGIT_SIZE == 16
        sprintf(buffer, "%04hx", big1->bigdigits[pos]);
#elif BIGDIGIT_SIZE == 32
#ifdef INT32TYPE_FORMAT_L
        sprintf(buffer, "%08lx", big1->bigdigits[pos]);
#else
        sprintf(buffer, "%08x", big1->bigdigits[pos]);
#endif
#endif
        if (IS_NEGATIVE(big1->bigdigits[pos])) {
          byteCount = BIGDIGIT_SIZE >> 3;
          while (byteCount > 1 && memcmp(buffer, "ff", 2) == 0 &&
            ((buffer[2] >= '8' && buffer[2] <= '9') ||
             (buffer[2] >= 'a' && buffer[2] <= 'f'))) {
            memmove(buffer, &buffer[2], strlen(&buffer[2]) + 1);
            byteCount--;
          } /* while */
        } else {
          byteCount = BIGDIGIT_SIZE >> 3;
          while (byteCount > 1 && memcmp(buffer, "00", 2) == 0 &&
            buffer[2] >= '0' && buffer[2] <= '7') {
            memmove(buffer, &buffer[2], strlen(&buffer[2]) + 1);
            byteCount--;
          } /* while */
        } /* if */
        buffer += strlen(buffer);
        while (pos > 0) {
          pos--;
#if BIGDIGIT_SIZE == 8
          sprintf(buffer, "%02hhx", big1->bigdigits[pos]);
#elif BIGDIGIT_SIZE == 16
          sprintf(buffer, "%04hx", big1->bigdigits[pos]);
#elif BIGDIGIT_SIZE == 32
#ifdef INT32TYPE_FORMAT_L
          sprintf(buffer, "%08lx", big1->bigdigits[pos]);
#else
          sprintf(buffer, "%08x", big1->bigdigits[pos]);
#endif
#endif
          buffer += (BIGDIGIT_SIZE >> 3) * 2;
        } /* while */
      } /* if */
    } else {
      if (big1 == NULL) {
        stri_ptr = " *NULL_BIGINT* ";
      } else {
        stri_ptr = " *ZERO_SIZE_BIGINT* ";
      } /* if */
      if (unlikely(!ALLOC_CSTRI(result, strlen(stri_ptr)))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        strcpy(result, stri_ptr);
      } /* if */
    } /* if */
    return result;
  } /* bigHexCStri */



/**
 *  Remove leading zero (or BIGDIGIT_MASK) digits from a signed big integer.
 *  @return the normalized big integer.
 */
static bigIntType normalize (bigIntType big1)

  {
    memSizeType pos;
    bigDigitType digit;

  /* normalize */
    pos = big1->size;
    if (pos >= 2) {
      pos--;
      digit = big1->bigdigits[pos];
      if (digit == BIGDIGIT_MASK) {
        do {
          pos--;
          digit = big1->bigdigits[pos];
        } while (pos > 0 && digit == BIGDIGIT_MASK);
        if (!IS_NEGATIVE(digit)) {
          pos++;
        } /* if */
      } else if (digit == 0) {
        do {
          pos--;
          digit = big1->bigdigits[pos];
        } while (pos > 0 && digit == 0);
        if (IS_NEGATIVE(digit)) {
          pos++;
        } /* if */
      } /* if */
      pos++;
#ifdef NORMALIZE_DOES_RESIZE
      if (big1->size != pos) {
        bigIntType resized_big1;

        REALLOC_BIG_SIZE_OK(resized_big1, big1, big1->size, pos);
        /* Avoid a MEMORY_ERROR in the strange case     */
        /* when a 'realloc' which shrinks memory fails. */
        if (likely(resized_big1 != NULL)) {
          big1 = resized_big1;
        } /* if */
        COUNT3_BIG(big1->size, pos);
        big1->size = pos;
      } /* if */
#else
      big1->size = pos;
#endif
    } /* if */
    return big1;
  } /* normalize */



static void negate_positive_big (const bigIntType big1)

  {
    memSizeType pos;
    doubleBigDigitType carry = 1;

  /* negate_positive_big */
    pos = 0;
    do {
      carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big1->size);
  } /* negate_positive_big */



static void positive_copy_of_negative_big (const bigIntType dest,
    const const_bigIntType big1)

  {
    memSizeType pos;
    doubleBigDigitType carry = 1;

  /* positive_copy_of_negative_big */
    pos = 0;
    do {
      carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
      dest->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big1->size);
    if (unlikely(IS_NEGATIVE(dest->bigdigits[pos - 1]))) {
      dest->bigdigits[pos] = 0;
      pos++;
    } /* if */
    dest->size = pos;
  } /* positive_copy_of_negative_big */



static bigIntType alloc_positive_copy_of_negative_big (const const_bigIntType big1)

  {
    memSizeType pos;
    doubleBigDigitType carry = 1;
    bigIntType result;

  /* alloc_positive_copy_of_negative_big */
    if (likely(ALLOC_BIG_SIZE_OK(result, big1->size))) {
      pos = 0;
      do {
        carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
        result->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < big1->size);
      result->size = pos;
    } /* if */
    return result;
  } /* alloc_positive_copy_of_negative_big */



/**
 *  Multiplies big1 by POWER_OF_10_IN_BIGDIGIT and adds carry to the product.
 *  The resulting sum is assigned to big1. This function works
 *  for unsigned big integers. It is assumed that big1 contains
 *  enough memory.
 */
static inline void uBigMultByPowerOf10AndAdd (const bigIntType big1, doubleBigDigitType carry)

  {
    memSizeType pos;

  /* uBigMultByPowerOf10AndAdd */
    pos = 0;
    do {
      carry += (doubleBigDigitType) big1->bigdigits[pos] * POWER_OF_10_IN_BIGDIGIT;
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big1->size);
    if (carry != 0) {
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      big1->size++;
    } /* if */
  } /* uBigMultByPowerOf10AndAdd */



/**
 *  Multiplies big1 by a factor and adds carry to the product.
 *  The resulting sum is assigned to big1. This function works
 *  for unsigned big integers. It is assumed that big1 contains
 *  enough memory.
 */
static inline void uBigMultiplyAndAdd (const bigIntType big1, bigDigitType factor,
    doubleBigDigitType carry)

  {
    memSizeType pos;

  /* uBigMultiplyAndAdd */
    pos = 0;
    do {
      carry += (doubleBigDigitType) big1->bigdigits[pos] * factor;
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big1->size);
    if (carry != 0) {
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      big1->size++;
    } /* if */
  } /* uBigMultiplyAndAdd */



/**
 *  Divides the unsigned big integer big1 by POWER_OF_10_IN_BIGDIGIT.
 *  The quotient is assigned to big1.
 *  @return the remainder of the unsigned big integer division.
 */
static inline bigDigitType uBigDivideByPowerOf10 (const bigIntType big1)

  {
    memSizeType pos;
    doubleBigDigitType carry = 0;
    bigDigitType bigdigit;

  /* uBigDivideByPowerOf10 */
    pos = big1->size;
    do {
      pos--;
      carry <<= BIGDIGIT_SIZE;
      carry += big1->bigdigits[pos];
      bigdigit = (bigDigitType) (carry / POWER_OF_10_IN_BIGDIGIT);
#if POINTER_SIZE <= BIGDIGIT_SIZE
      /* There is probably no machine instruction for division    */
      /* and remainder of doubleBigDigitType values available.    */
      /* To compute the remainder fast the % operator is avoided  */
      /* and the remainder is computed with a multiplication and  */
      /* a subtraction. The overflow in the multiplication can be */
      /* ignored, since the result of the subtraction fits in the */
      /* lower bigdigit of carry. The wrong bits in the higher    */
      /* bigdigit of carry are masked away.                       */
      carry = (carry - bigdigit * POWER_OF_10_IN_BIGDIGIT) & BIGDIGIT_MASK;
#else
      /* There is probably a machine instruction for division     */
      /* and remainder of doubleBigDigitType values available.    */
      /* In the optimal case quotient and remainder can be        */
      /* computed with one instruction.                           */
      carry %= POWER_OF_10_IN_BIGDIGIT;
#endif
      big1->bigdigits[pos] = bigdigit;
    } while (pos > 0);
    return (bigDigitType) (carry);
  } /* uBigDivideByPowerOf10 */



/**
 *  Divides the unsigned big integer big1 by divisor_digit.
 *  The quotient is assigned to big1.
 *  @return the remainder of the unsigned big integer division.
 */
static inline bigDigitType uBigDivideByDigit (const bigIntType big1,
    const bigDigitType divisor_digit)

  {
    memSizeType pos;
    doubleBigDigitType carry = 0;
    bigDigitType bigdigit;

  /* uBigDivideByDigit */
    pos = big1->size;
    do {
      pos--;
      carry <<= BIGDIGIT_SIZE;
      carry += big1->bigdigits[pos];
      bigdigit = (bigDigitType) (carry / divisor_digit);
#if POINTER_SIZE <= BIGDIGIT_SIZE
      /* There is probably no machine instruction for division    */
      /* and remainder of doubleBigDigitType values available.    */
      /* To compute the remainder fast the % operator is avoided  */
      /* and the remainder is computed with a multiplication and  */
      /* a subtraction. The overflow in the multiplication can be */
      /* ignored, since the result of the subtraction fits in the */
      /* lower bigdigit of carry. The wrong bits in the higher    */
      /* bigdigit of carry are masked away.                       */
      carry = (carry - bigdigit * divisor_digit) & BIGDIGIT_MASK;
#else
      /* There is probably a machine instruction for division     */
      /* and remainder of doubleBigDigitType values available.    */
      /* In the optimal case quotient and remainder can be        */
      /* computed with one instruction.                           */
      carry %= divisor_digit;
#endif
      big1->bigdigits[pos] = bigdigit;
    } while (pos > 0);
    return (bigDigitType) (carry);
  } /* uBigDivideByDigit */



#ifdef OUT_OF_ORDER
static bigIntType bigParseBasedPow2 (const const_striType stri, unsigned int shift)

  {
    boolType okay;
    boolType negative;
    memSizeType mostSignificantDigitPos;
    memSizeType charPos;
    strElemType digit;
    int digitval;
    memSizeType bigDigitPos;
    doubleBigDigitType bigDigit;
    unsigned int bigDigitShift;
    memSizeType result_size;
    bigIntType result;

  /* bigParseBasedPow2 */
    /* printf("bigParseBasedPow2(");
       prot_stri(stri);
       printf(", %u)\n", shift); */
    mostSignificantDigitPos = 0;
    if (stri->size != 0) {
      if (stri->mem[0] == ((strElemType) '-')) {
        negative = TRUE;
        mostSignificantDigitPos++;
      } else {
        if (stri->mem[0] == ((strElemType) '+')) {
          mostSignificantDigitPos++;
        } /* if */
        negative = FALSE;
      } /* if */
    } /* if */
    if (unlikely(stri->size <= mostSignificantDigitPos)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (unlikely(stri->size - mostSignificantDigitPos >
                        MAX_MEMSIZETYPE / (memSizeType) shift)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      /* Compute the number of bits necessary: */
      result_size = (stri->size - mostSignificantDigitPos) * (memSizeType) shift;
      /* Compute the number of bigDigits: */
      result_size = (result_size - 1) / BIGDIGIT_SIZE + 1;
      if (unlikely(!ALLOC_BIG(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        okay = TRUE;
        bigDigit = 0;
        bigDigitPos = result_size;
        bigDigitShift = 0;
        for (charPos = result_size; charPos > mostSignificantDigitPos && okay; charPos++) {
          digit = stri->mem[charPos - 1];
          if (digit >= '0' && digit <= 'z') {
            digitval = digit_value[digit - (strElemType) '0'];
            if (digitval == -1) {
              okay = FALSE;
            } /* if */
          } else {
            digitval = -1;
            okay = FALSE;
          } /* if */
          bigDigit |= (doubleBigDigitType) digitval << bigDigitShift;
          bigDigitShift += shift;
          if (bigDigitShift >= BIGDIGIT_SIZE) {
            bigDigitPos--;
            result->bigdigits[bigDigitPos] = (bigDigitType) (bigDigit & BIGDIGIT_MASK);
            bigDigit >>= BIGDIGIT_SIZE;
            bigDigitShift -= BIGDIGIT_SIZE;
          } /* if */
        } /* for */
        if (unlikely(!okay)) {
          FREE_BIG(result, result_size);
          raise_error(RANGE_ERROR);
          result = NULL;
        } else {
          result->size = result_size;
          while (bigDigitPos >= 1) {
            bigDigitPos--;
            result->bigdigits[bigDigitPos] = (bigDigitType) (bigDigit & BIGDIGIT_MASK);
            bigDigit >>= BIGDIGIT_SIZE;
          } /* while */
          if (negative) {
            negate_positive_big(result);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* bigParseBasedPow2 */
#endif



/**
 *  Convert a big integer number to a string using a radix.
 *  The conversion uses the numeral system with the specified base.
 *  The base is a power of two and it is specified indirectly with
 *  shift and mask. Digit values from 10 upward are encoded with
 *  letters.
 *  @param shift Logarithm (log2) of the base (=number of bits in mask).
 *  @param mask Mask to get the bits of a digit (equivalent to base-1).
 *  @param upperCase Decides about the letter case.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
static striType bigRadixPow2 (const const_bigIntType big1, unsigned int shift,
    bigDigitType mask, boolType upperCase)

  {
    const_bigIntType unsigned_big;
    memSizeType unsigned_size;
    doubleBigDigitType unsigned_digit;
    boolType negative;
    unsigned int most_significant;
    unsigned int digit_shift;
    memSizeType digit_index;
    const_cstriType digits;
    memSizeType pos;
    memSizeType result_size;
    striType result;

  /* bigRadixPow2 */
    /* printf("bigRadixPow2(%s, %u, %x, %d)\n", bigHexCStri(big1), shift, mask, upperCase); */
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    if (negative) {
      unsigned_big = alloc_positive_copy_of_negative_big(big1);
      unsigned_size = unsigned_big->size;
      while (unsigned_size > 1 && unsigned_big->bigdigits[unsigned_size - 1] == 0) {
        unsigned_size--;
      } /* while */
    } else {
      unsigned_big = big1;
      if (big1->size > 1 && big1->bigdigits[big1->size - 1] == 0) {
        unsigned_size = big1->size - 1;
      } else {
        unsigned_size = big1->size;
      } /* if */
    } /* if */
    /* Unsigned_size is reduced to avoid a leading zero digit.        */
    /* Except for the value 0 unsigned_big has no leading zero digit. */
    if (unlikely(unsigned_big == NULL)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      if (unlikely((MAX_STRI_LEN <= (MAX_MEMSIZETYPE - 1) / shift +
                                    (unsigned int) negative + 1 &&
                   unsigned_size > ((MAX_STRI_LEN - (unsigned int) negative - 1) *
                                shift + 1) / BIGDIGIT_SIZE) ||
                   unsigned_size > MAX_MEMSIZETYPE / BIGDIGIT_SIZE)) {
        if (unsigned_big != big1) {
          FREE_BIG(unsigned_big, unsigned_big->size);
        } /* if */
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (unsigned_size == 1 && unsigned_big->bigdigits[0] == 0) {
          /* The size of zero is always 1. */
          result_size = 1;
        } else {
          /* The size of the result is computed by computing */
          /* the number of radix digits plus one optional    */
          /* character for the sign.                         */
          result_size = (unsigned_size * BIGDIGIT_SIZE - 1) / shift + (unsigned int) negative + 1;
          most_significant = (unsigned int) digitMostSignificantBit(unsigned_big->bigdigits[unsigned_size - 1]);
          digit_shift = (unsigned int) (BIGDIGIT_SIZE - (unsigned_size * BIGDIGIT_SIZE) % shift);
          if (most_significant < digit_shift) {
            /* Reduce result_size because of leading zero digits. */
            result_size -= (digit_shift - most_significant - 1) / shift;
            if (digit_shift != BIGDIGIT_SIZE) {
              result_size--;
            } /* if */
          } /* if */
        } /* if */
        /* printf("result_size: %lu\n", result_size); */
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
          if (unsigned_big != big1) {
            FREE_BIG(unsigned_big, unsigned_big->size);
          } /* if */
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          digits = digitTable[upperCase];
          pos = result_size;
          unsigned_digit = unsigned_big->bigdigits[0];
          digit_index = 0;
          digit_shift = 0;
          do {
            while (digit_shift <= BIGDIGIT_SIZE - shift && pos > (memSizeType) negative) {
              pos--;
              /* printf("A result->mem[%lu] = %c\n", pos, digits[(unsigned_digit >> digit_shift) & mask]); */
              result->mem[pos] = (strElemType) (digits[(unsigned_digit >> digit_shift) & mask]);
              digit_shift += shift;
            } /* while */
            digit_index++;
            if (digit_index < unsigned_size) {
              if (digit_shift == BIGDIGIT_SIZE) {
                unsigned_digit = unsigned_big->bigdigits[digit_index];
                digit_shift = 0;
              } else {
                unsigned_digit >>= digit_shift;
                unsigned_digit |= unsigned_big->bigdigits[digit_index] << (BIGDIGIT_SIZE - digit_shift);
                if (pos > (memSizeType) negative) {
                  pos--;
                  /* printf("B result->mem[%lu] = %c\n", pos, digits[unsigned_digit & mask]); */
                  result->mem[pos] = (strElemType) (digits[unsigned_digit & mask]);
                  unsigned_digit = unsigned_big->bigdigits[digit_index];
                  digit_shift += shift - BIGDIGIT_SIZE;
                } /* if */
              } /* if */
            } else if (digit_shift != BIGDIGIT_SIZE && pos > (memSizeType) negative) {
              pos--;
              /* printf("C result->mem[%lu] = %c\n", pos, digits[(unsigned_digit >> digit_shift) & mask]); */
              result->mem[pos] = (strElemType) (digits[(unsigned_digit >> digit_shift) & mask]);
            } /* if */
          } while (digit_index < unsigned_size && pos > (memSizeType) negative);
          if (negative) {
            pos--;
            /* printf("result->mem[%lu] = -\n", pos); */
            result->mem[pos] = (strElemType) '-';
          } /* if */
          result->size = result_size;
        } /* if */
      } /* if */
      if (unsigned_big != big1) {
        FREE_BIG(unsigned_big, unsigned_big->size);
      } /* if */
    } /* if */
    /* printf("bigRadixPow2 --> ");
       prot_stri(result);
       printf("\n"); */
    return result;
  } /* bigRadixPow2 */



/**
 *  Convert a big integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with letters.
 *  For negative numbers a minus sign is prepended.
 *  @param base Base of numeral system (base >= 2 and base <= 36 holds).
 *  @param upperCase Decides about the letter case.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
static striType bigRadix2To36 (const const_bigIntType big1, unsigned int base,
    boolType upperCase)

  {
    unsigned int estimate_shift;
    bigIntType unsigned_big;
    boolType negative;
    const_cstriType digits;
    bigDigitType divisor_digit;
    uint8Type digits_in_bigdigit;
    uint8Type digit_pos;
    bigDigitType digit;
    memSizeType pos;
    memSizeType result_size;
    memSizeType final_result_size;
    striType resized_result;
    striType result;

  /* bigRadix2To36 */
    /* printf("bigRadix2To36(%s, %u, %d)\n", bigHexCStri(big1), base, upperCase); */
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    /* A power of two that is less or equal than the base is */
    /* used to estimate the size of the result.              */
    estimate_shift = (unsigned int) uint8MostSignificantBit((uint8Type) base);
    if (unlikely((MAX_STRI_LEN <= (MAX_MEMSIZETYPE - 1) / estimate_shift +
                                  (unsigned int) negative + 1 &&
                 big1->size > ((MAX_STRI_LEN - (unsigned int) negative - 1) *
                               estimate_shift + 1) / BIGDIGIT_SIZE) ||
                 big1->size > MAX_MEMSIZETYPE / BIGDIGIT_SIZE)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      /* The size of the result is computed by computing the     */
      /* number of radix digits plus one character for the sign. */
      result_size = (big1->size * BIGDIGIT_SIZE - 1) / estimate_shift + (unsigned int) negative + 1;
      /* printf("result_size: %lu\n", result_size); */
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (negative) {
          unsigned_big = alloc_positive_copy_of_negative_big(big1);
        } else if (likely(ALLOC_BIG_SIZE_OK(unsigned_big, big1->size))) {
          unsigned_big->size = big1->size;
          memcpy(unsigned_big->bigdigits, big1->bigdigits,
              (size_t) big1->size * sizeof(bigDigitType));
        } /* if */
        if (unlikely(unsigned_big == NULL)) {
          FREE_STRI(result, result_size);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          digits = digitTable[upperCase];
          divisor_digit = powerOfRadixInBigdigit[base - 2];
          /* printf("divisor_digit: %u\n", divisor_digit); */
          digits_in_bigdigit = radixDigitsInBigdigit[base - 2];
          /* printf("digits_in_bigdigit: %hd\n", digits_in_bigdigit); */
          pos = result_size - 1;
          do {
            digit = uBigDivideByDigit(unsigned_big, divisor_digit);
            /* printf("unsigned_big->size=%lu, digit=%u\n", unsigned_big->size, digit); */
            if (unsigned_big->bigdigits[unsigned_big->size - 1] == 0) {
              unsigned_big->size--;
            } /* if */
            if (unsigned_big->size > 1 || unsigned_big->bigdigits[0] != 0) {
              for (digit_pos = digits_in_bigdigit;
                  digit_pos != 0; digit_pos--) {
                result->mem[pos] = (strElemType) (digits[digit % base]);
                digit /= base;
                pos--;
              } /* for */
            } else {
              do {
                result->mem[pos] = (strElemType) (digits[digit % base]);
                digit /= base;
                pos--;
              } while (digit != 0);
            } /* if */
          } while (unsigned_big->size > 1 || unsigned_big->bigdigits[0] != 0);
          FREE_BIG(unsigned_big, big1->size + 1);
          pos++;
          if (negative) {
            final_result_size = result_size - pos + 1;
            result->mem[0] = '-';
            memmove(&result->mem[1], &result->mem[pos],
                (result_size - pos) * sizeof(strElemType));
          } else {
            final_result_size = result_size - pos;
            memmove(&result->mem[0], &result->mem[pos],
                (result_size - pos) * sizeof(strElemType));
          } /* if */
          result->size = final_result_size;
          if (final_result_size < result_size) {
            REALLOC_STRI_SIZE_SMALLER(resized_result, result, result_size, final_result_size);
            if (unlikely(resized_result == NULL)) {
              FREE_STRI(result, result_size);
              raise_error(MEMORY_ERROR);
              result = NULL;
            } else {
              result = resized_result;
              COUNT3_STRI(result_size, final_result_size);
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    /* printf("bigRadix2To36 --> ");
       prot_stri(result);
       printf("\n"); */
    return result;
  } /* bigRadix2To36 */



/**
 *  Shifts the big integer big1 to the left by lshift bits.
 *  Bits which are shifted out at the left of big1 are lost.
 *  At the right of big1 zero bits are shifted in. The function
 *  is called for 0 < lshift < BIGDIGIT_SIZE.
 */
static void uBigLShift (const bigIntType big1, const unsigned int lshift)

  {
    doubleBigDigitType carry = 0;
    memSizeType pos;

  /* uBigLShift */
    pos = 0;
    do {
      carry |= ((doubleBigDigitType) big1->bigdigits[pos]) << lshift;
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big1->size);
  } /* uBigLShift */



#ifdef OUT_OF_ORDER
/**
 *  Shifts the big integer big1 to the left by lshift bits.
 *  Bits which are shifted out at the left of big1 are lost.
 *  At the right of big1 zero bits are shifted in. The function
 *  is called for 0 < lshift < BIGDIGIT_SIZE.
 */
static void uBigLShift (const bigIntType big1, const unsigned int lshift)

  {
    unsigned int rshift = BIGDIGIT_SIZE - lshift;
    bigDigitType low_digit;
    bigDigitType high_digit;
    memSizeType pos;

  /* uBigLShift */
    low_digit = big1->bigdigits[0];
    big1->bigdigits[0] = (bigDigitType) ((low_digit << lshift) & BIGDIGIT_MASK);
    for (pos = 1; pos < big1->size; pos++) {
      high_digit = big1->bigdigits[pos];
      big1->bigdigits[pos] = (bigDigitType)
          (((low_digit >> rshift) | (high_digit << lshift)) & BIGDIGIT_MASK);
      low_digit = high_digit;
    } /* for */
  } /* uBigLShift */
#endif



#ifdef OUT_OF_ORDER
/**
 *  Shifts the big integer big1 to the right by rshift bits.
 *  Bits which are shifted out at the right of big1 are lost.
 *  At the left of big1 zero bits are shifted in. The function
 *  is called for 0 < rshift < BIGDIGIT_SIZE.
 */
static void uBigRShift (const bigIntType big1, const unsigned int rshift)

  {
    unsigned int lshift = BIGDIGIT_SIZE - rshift;
    doubleBigDigitType carry = 0;
    memSizeType pos;

  /* uBigRShift */
    pos = big1->size;
    do {
      carry <<= BIGDIGIT_SIZE;
      carry |= ((doubleBigDigitType) big1->bigdigits[pos]) << lshift;
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      pos--;
    } while (pos > 0);
  } /* uBigRShift */
#endif



/**
 *  Shifts the big integer big1 to the right by rshift bits.
 *  Bits which are shifted out at the right of big1 are lost.
 *  At the left of big1 zero bits are shifted in. The function
 *  is called for 0 < rshift < BIGDIGIT_SIZE.
 */
static void uBigRShift (const bigIntType big1, const unsigned int rshift)

  {
    unsigned int lshift = BIGDIGIT_SIZE - rshift;
    bigDigitType low_digit;
    bigDigitType high_digit;
    memSizeType pos;

  /* uBigRShift */
    high_digit = 0;
    for (pos = big1->size - 1; pos != 0; pos--) {
      low_digit = big1->bigdigits[pos];
      big1->bigdigits[pos] = (bigDigitType)
          (((low_digit >> rshift) | (high_digit << lshift)) & BIGDIGIT_MASK);
      high_digit = low_digit;
    } /* for */
    low_digit = big1->bigdigits[0];
    big1->bigdigits[0] = (bigDigitType)
        (((low_digit >> rshift) | (high_digit << lshift)) & BIGDIGIT_MASK);
  } /* uBigRShift */



/**
 *  Increments an unsigned big integer by 1.
 *  This function does overflow silently when big1 contains
 *  not enough digits.
 */
static void uBigIncr (const bigIntType big1)

  {
    memSizeType pos;

  /* uBigIncr */
    pos = 0;
    if (unlikely(big1->bigdigits[pos] == BIGDIGIT_MASK)) {
      if (big1->size == 1) {
        big1->bigdigits[pos] = 0;
        pos++;
      } else {
        do {
          big1->bigdigits[pos] = 0;
          pos++;
        } while (big1->bigdigits[pos] == BIGDIGIT_MASK);
        /* memset(big1->bigdigits, 0, pos * sizeof(bigDigitType)); */
      } /* if */
    } /* if */
    if (pos < big1->size) {
      big1->bigdigits[pos]++;
    } /* if */
  } /* uBigIncr */



/**
 *  Decrements an unsigned big integer by 1.
 *  This function does overflow silently when big1 contains
 *  not enough digits. The function works correctly when there
 *  are leading zereo digits.
 */
static void uBigDecr (const bigIntType big1)

  {
    memSizeType pos;

  /* uBigDecr */
    pos = 0;
    if (unlikely(big1->bigdigits[pos] == 0)) {
      do {
        big1->bigdigits[pos] = BIGDIGIT_MASK;
        pos++;
      } while (pos < big1->size && big1->bigdigits[pos] == 0);
    } /* if */
    if (pos < big1->size) {
      big1->bigdigits[pos]--;
    } /* if */
  } /* uBigDecr */



/**
 *  Computes an integer division of dividend by one divisor_digit for
 *  nonnegative big integers. The divisor_digit must not be zero.
 */
static void uBigDiv1 (const const_bigIntType dividend,
    const bigDigitType divisor_digit, const bigIntType quotient)

  {
    memSizeType pos;
    doubleBigDigitType carry = 0;

  /* uBigDiv1 */
    pos = dividend->size;
    do {
      pos--;
      carry <<= BIGDIGIT_SIZE;
      carry += dividend->bigdigits[pos];
      quotient->bigdigits[pos] = (bigDigitType) ((carry / divisor_digit) & BIGDIGIT_MASK);
      carry %= divisor_digit;
    } while (pos > 0);
  } /* uBigDiv1 */



/**
 *  Computes an integer division of dividend by one divisor_digit
 *  for signed big integers. The memory for the result is requested
 *  and the normalized result is returned. This function handles
 *  also the special case of a division by zero.
 *  @return the quotient of the integer division.
 */
static bigIntType bigDiv1 (const_bigIntType dividend, bigDigitType divisor_digit)

  {
    boolType negative = FALSE;
    bigIntType dividend_help = NULL;
    bigIntType quotient;

  /* bigDiv1 */
    if (unlikely(divisor_digit == 0)) {
      raise_error(NUMERIC_ERROR);
      return NULL;
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(quotient, dividend->size + 1))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        quotient->size = dividend->size + 1;
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          negative = TRUE;
          dividend_help = alloc_positive_copy_of_negative_big(dividend);
          dividend = dividend_help;
          if (unlikely(dividend_help == NULL)) {
            FREE_BIG(quotient, quotient->size);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
        } /* if */
        quotient->bigdigits[quotient->size - 1] = 0;
        if (IS_NEGATIVE(divisor_digit)) {
          negative = !negative;
          /* The unsigned value is negated to avoid a signed integer */
          /* overflow when the smallest signed integer is negated.   */
          divisor_digit = -divisor_digit;
        } /* if */
        uBigDiv1(dividend, divisor_digit, quotient);
        if (negative) {
          negate_positive_big(quotient);
        } /* if */
        quotient = normalize(quotient);
        if (dividend_help != NULL) {
          FREE_BIG(dividend_help, dividend_help->size);
        } /* if */
        return quotient;
      } /* if */
    } /* if */
  } /* bigDiv1 */



/**
 *  Computes an integer division of dividend by divisor for signed big
 *  integers when dividend has less digits than divisor. The memory for
 *  the result is requested and the normalized result is returned. Normally
 *  dividend->size < divisor->size implies abs(dividend) < abs(divisor).
 *  When abs(dividend) < abs(divisor) holds the result is 0. The cases when
 *  dividend->size < divisor->size and abs(dividend) = abs(divisor) are if
 *  dividend->size + 1 == divisor->size and dividend = 0x8000 (0x80000000...)
 *  and divisor = 0x00008000 (0x000080000000...). In this cases the
 *  result is -1. In all other cases the result is 0.
 *  @return the quotient of the integer division.
 */
static bigIntType bigDivSizeLess (const const_bigIntType dividend,
    const const_bigIntType divisor)

  {
    memSizeType pos;
    bigIntType quotient;

  /* bigDivSizeLess */
    if (unlikely(!ALLOC_BIG_SIZE_OK(quotient, 1))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      quotient->size = 1;
      if (unlikely(dividend->size + 1 == divisor->size &&
                   dividend->bigdigits[dividend->size - 1] == BIGDIGIT_SIGN &&
                   divisor->bigdigits[divisor->size - 1] == 0 &&
                   divisor->bigdigits[divisor->size - 2] == BIGDIGIT_SIGN)) {
        quotient->bigdigits[0] = BIGDIGIT_MASK;
        pos = dividend->size - 1;
        while (pos > 0) {
          pos--;
          if (likely(dividend->bigdigits[pos] != 0 || divisor->bigdigits[pos] != 0)) {
            quotient->bigdigits[0] = 0;
            pos = 0;
          } /* if */
        } /* while */
      } else {
        quotient->bigdigits[0] = 0;
      } /* if */
      return quotient;
    } /* if */
  } /* bigDivSizeLess */



/**
 *  Multiplies big2 with multiplier and subtracts the result from
 *  big1 at the digit position pos1 of big1. Big1, big2 and
 *  multiplier are nonnegative big integer values.
 *  The algorithm tries to save computations. Therefore
 *  there are checks for mult_carry != 0 and sbtr_carry == 0.
 */
static bigDigitType uBigMultSub (const bigIntType big1, const const_bigIntType big2,
    const bigDigitType multiplier, const memSizeType pos1)

  {
    memSizeType pos;
    doubleBigDigitType mult_carry = 0;
    doubleBigDigitType sbtr_carry = 1;

  /* uBigMultSub */
    pos = 0;
    do {
      mult_carry += (doubleBigDigitType) big2->bigdigits[pos] * multiplier;
      sbtr_carry += big1->bigdigits[pos1 + pos] + (~mult_carry & BIGDIGIT_MASK);
      big1->bigdigits[pos1 + pos] = (bigDigitType) (sbtr_carry & BIGDIGIT_MASK);
      mult_carry >>= BIGDIGIT_SIZE;
      sbtr_carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big2->size);
    for (pos += pos1; mult_carry != 0 && pos < big1->size; pos++) {
      sbtr_carry += big1->bigdigits[pos] + (~mult_carry & BIGDIGIT_MASK);
      big1->bigdigits[pos] = (bigDigitType) (sbtr_carry & BIGDIGIT_MASK);
      mult_carry >>= BIGDIGIT_SIZE;
      sbtr_carry >>= BIGDIGIT_SIZE;
    } /* for */
    for (; sbtr_carry == 0 && pos < big1->size; pos++) {
      sbtr_carry = (doubleBigDigitType) big1->bigdigits[pos] + BIGDIGIT_MASK;
      big1->bigdigits[pos] = (bigDigitType) (sbtr_carry & BIGDIGIT_MASK);
      sbtr_carry >>= BIGDIGIT_SIZE;
    } /* for */
    return (bigDigitType) (sbtr_carry & BIGDIGIT_MASK);
  } /* uBigMultSub */



/**
 *  Adds big2 to big1 at the digit position pos1. Big1 and big2
 *  are nonnegative big integer values. The size of big1 must be
 *  greater or equal the size of big2. The final carry is ignored.
 */
static void uBigAddTo (const bigIntType big1, const const_bigIntType big2,
    const memSizeType pos1)

  {
    memSizeType pos;
    doubleBigDigitType carry = 0;

  /* uBigAddTo */
    pos = 0;
    do {
      carry += (doubleBigDigitType) big1->bigdigits[pos1 + pos] + big2->bigdigits[pos];
      big1->bigdigits[pos1 + pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big2->size);
    for (pos += pos1; pos < big1->size; pos++) {
      carry += big1->bigdigits[pos];
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
    } /* for */
  } /* uBigAddTo */



/**
 *  Computes an integer division of dividend by divisor for nonnegative big
 *  integers. The remainder is delivered in dividend. There are several
 *  preconditions. Divisor must have at least 2 digits and dividend must
 *  have at least one digit more than divisor. If dividend and divisor have
 *  the same length in digits nothing is done. The most significant bit of
 *  divisor must be set. The most significant digit of dividend must be
 *  less than the most significant digit of divisor. The computations to
 *  meet this predonditions are done outside this function. The special
 *  cases with a one digit divisor or a dividend with less digits than
 *  divisor are handled in other functions. This algorithm based on the
 *  algorithm from D.E. Knuth described in "The art of computer programming"
 *  volume 2 (Seminumerical algorithms).
 */
static void uBigDiv (const bigIntType dividend, const const_bigIntType divisor,
    const bigIntType quotient)

  {
    memSizeType pos1;
    doubleBigDigitType twodigits;
    doubleBigDigitType remainder;
    bigDigitType quotientdigit;
    bigDigitType sbtr_carry;

  /* uBigDiv */
    for (pos1 = dividend->size - 1; pos1 >= divisor->size; pos1--) {
      twodigits = (((doubleBigDigitType) dividend->bigdigits[pos1]) << BIGDIGIT_SIZE) |
          dividend->bigdigits[pos1 - 1];
      if (unlikely(dividend->bigdigits[pos1] == divisor->bigdigits[divisor->size - 1])) {
        quotientdigit = BIGDIGIT_MASK;
      } else {
        quotientdigit = (bigDigitType) (twodigits / divisor->bigdigits[divisor->size - 1]);
      } /* if */
      remainder = twodigits - (doubleBigDigitType) quotientdigit *
          divisor->bigdigits[divisor->size - 1];
      while (remainder <= BIGDIGIT_MASK &&
          (doubleBigDigitType) divisor->bigdigits[divisor->size - 2] * quotientdigit >
          (remainder << BIGDIGIT_SIZE | dividend->bigdigits[pos1 - 2])) {
        quotientdigit--;
        remainder = twodigits - (doubleBigDigitType) quotientdigit *
            divisor->bigdigits[divisor->size - 1];
      } /* while */
      sbtr_carry = uBigMultSub(dividend, divisor, quotientdigit, pos1 - divisor->size);
      if (sbtr_carry == 0) {
        uBigAddTo(dividend, divisor, pos1 - divisor->size);
        quotientdigit--;
      } /* if */
      quotient->bigdigits[pos1 - divisor->size] = quotientdigit;
    } /* for */
  } /* uBigDiv */



/**
 *  Computes the remainder of a integer division of dividend by
 *  one divisor_digit for nonnegative big integers. The divisor_digit must
 *  not be zero.
 */
static bigDigitType uBigRem1 (const const_bigIntType dividend, const bigDigitType divisor_digit)

  {
    memSizeType pos;
    doubleBigDigitType carry = 0;

  /* uBigRem1 */
    pos = dividend->size;
    do {
      pos--;
      carry <<= BIGDIGIT_SIZE;
      carry += dividend->bigdigits[pos];
      carry %= divisor_digit;
    } while (pos > 0);
    return (bigDigitType) carry;
  } /* uBigRem1 */



/**
 *  Computes the remainder of the integer division dividend by one
 *  divisor_digit for signed big integers. The memory for the
 *  remainder is requested and the normalized remainder is
 *  returned. This function handles also the special case of a
 *  division by zero.
 */
static bigIntType bigRem1 (const_bigIntType dividend, bigDigitType divisor_digit)

  {
    boolType negative = FALSE;
    bigIntType dividend_help = NULL;
    bigIntType remainder;

  /* bigRem1 */
    if (unlikely(divisor_digit == 0)) {
      raise_error(NUMERIC_ERROR);
      return NULL;
    } else {
      if (unlikely(!ALLOC_BIG_SIZE_OK(remainder, 1))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        remainder->size = 1;
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          negative = TRUE;
          dividend_help = alloc_positive_copy_of_negative_big(dividend);
          dividend = dividend_help;
          if (unlikely(dividend_help == NULL)) {
            FREE_BIG(remainder, remainder->size);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
        } /* if */
        if (IS_NEGATIVE(divisor_digit)) {
          /* The unsigned value is negated to avoid a signed integer */
          /* overflow when the smallest signed integer is negated.   */
          divisor_digit = -divisor_digit;
        } /* if */
        remainder->bigdigits[0] = uBigRem1(dividend, divisor_digit);
        if (negative) {
          negate_positive_big(remainder);
        } /* if */
        if (dividend_help != NULL) {
          FREE_BIG(dividend_help, dividend_help->size);
        } /* if */
        return remainder;
      } /* if */
    } /* if */
  } /* bigRem1 */



/**
 *  Computes an integer division of dividend by one divisor_digit for
 *  nonnegative big integers. The divisor_digit must not be zero.
 *  The remainder of the division is returned.
 */
static bigDigitType uBigMDiv1 (const const_bigIntType dividend,
    const bigDigitType divisor_digit, const bigIntType quotient)

  {
    memSizeType pos;
    doubleBigDigitType carry = 0;

  /* uBigMDiv1 */
    pos = dividend->size;
    do {
      pos--;
      carry <<= BIGDIGIT_SIZE;
      carry += dividend->bigdigits[pos];
      quotient->bigdigits[pos] = (bigDigitType) ((carry / divisor_digit) & BIGDIGIT_MASK);
      carry %= divisor_digit;
    } while (pos > 0);
    return (bigDigitType) carry;
  } /* uBigMDiv1 */



/**
 *  Computes an integer modulo division of dividend by one
 *  divisor_digit for signed big integers. The memory for the
 *  result is requested and the normalized result is returned.
 *  This function handles also the special case of a division by
 *  zero.
 *  @return the quotient of the integer division.
 */
static bigIntType bigMDiv1 (const_bigIntType dividend, bigDigitType divisor_digit)

  {
    boolType negative = FALSE;
    bigIntType dividend_help = NULL;
    bigDigitType remainder;
    bigIntType quotient;

  /* bigMDiv1 */
    if (unlikely(divisor_digit == 0)) {
      raise_error(NUMERIC_ERROR);
      return NULL;
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(quotient, dividend->size + 1))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        quotient->size = dividend->size + 1;
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          negative = TRUE;
          dividend_help = alloc_positive_copy_of_negative_big(dividend);
          dividend = dividend_help;
          if (unlikely(dividend_help == NULL)) {
            FREE_BIG(quotient, quotient->size);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
        } /* if */
        quotient->bigdigits[quotient->size - 1] = 0;
        if (IS_NEGATIVE(divisor_digit)) {
          negative = !negative;
          /* The unsigned value is negated to avoid a signed integer */
          /* overflow when the smallest signed integer is negated.   */
          divisor_digit = -divisor_digit;
        } /* if */
        remainder = uBigMDiv1(dividend, divisor_digit, quotient);
        if (negative) {
          if (remainder != 0) {
            uBigIncr(quotient);
          } /* if */
          negate_positive_big(quotient);
        } /* if */
        quotient = normalize(quotient);
        if (dividend_help != NULL) {
          FREE_BIG(dividend_help, dividend_help->size);
        } /* if */
        return quotient;
      } /* if */
    } /* if */
  } /* bigMDiv1 */



/**
 *  Computes a modulo integer division of dividend by divisor for signed
 *  big integers when dividend has less digits than divisor. The memory for
 *  the result is requested and the normalized result is returned. Normally
 *  dividend->size < divisor->size implies abs(dividend) < abs(divisor).
 *  When abs(dividend) < abs(divisor) holds the result is 0 or -1. The cases
 *  when dividend->size < divisor->size and abs(dividend) = abs(divisor) are if
 *  dividend->size + 1 == divisor->size and dividend = 0x8000 (0x80000000...)
 *  and divisor = 0x00008000 (0x000080000000...). In this cases the
 *  result is -1. In the cases when the result is 0 or -1 the
 *  following check is done: When dividend and divisor have different signs
 *  the result is -1 otherwise the result is 0.
 *  @return the quotient of the integer division.
 */
static bigIntType bigMDivSizeLess (const const_bigIntType dividend,
    const const_bigIntType divisor)

  {
    memSizeType pos;
    bigIntType quotient;

  /* bigMDivSizeLess */
    if (unlikely(!ALLOC_BIG_SIZE_OK(quotient, 1))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      quotient->size = 1;
      if (unlikely(dividend->size + 1 == divisor->size &&
                   dividend->bigdigits[dividend->size - 1] == BIGDIGIT_SIGN &&
                   divisor->bigdigits[divisor->size - 1] == 0 &&
                   divisor->bigdigits[divisor->size - 2] == BIGDIGIT_SIGN)) {
        quotient->bigdigits[0] = BIGDIGIT_MASK;
        pos = dividend->size - 1;
        while (pos > 0) {
          pos--;
          if (likely(dividend->bigdigits[pos] != 0 || divisor->bigdigits[pos] != 0)) {
            quotient->bigdigits[0] = 0;
            pos = 0;
          } /* if */
        } /* while */
      } else {
        quotient->bigdigits[0] = 0;
      } /* if */
      if (quotient->bigdigits[0] == 0 &&
          IS_NEGATIVE(dividend->bigdigits[dividend->size - 1]) !=
          IS_NEGATIVE(divisor->bigdigits[divisor->size - 1])) {
        quotient->bigdigits[0] = BIGDIGIT_MASK;
      } /* if */
      return quotient;
    } /* if */
  } /* bigMDivSizeLess */



/**
 *  Computes the modulo of the integer division dividend by one
 *  digit for signed big integers. The memory for the modulo is
 *  requested and the normalized modulo is returned. This function
 *  handles also the special case of a division by zero.
 */
static bigIntType bigMod1 (const const_bigIntType dividend, const bigDigitType divisor_digit)

  {
    bigIntType modulo;

  /* bigMod1 */
    modulo = bigRem1(dividend, divisor_digit);
    if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1]) != IS_NEGATIVE(divisor_digit) &&
        modulo != NULL && modulo->bigdigits[0] != 0) {
      modulo->bigdigits[0] += divisor_digit;
    } /* if */
    return modulo;
  } /* bigMod1 */



/**
 *  Computes the remainder of the integer division dividend by divisor for
 *  signed big integers when dividend has less digits than divisor. The memory
 *  for the remainder is requested and the normalized remainder is returned.
 *  Normally dividend->size < divisor->size implies abs(dividend) < abs(divisor).
 *  When abs(dividend) < abs(divisor) holds the remainder is dividend. The cases
 *  when dividend->size < divisor->size and abs(dividend) = abs(divisor) are if
 *  dividend->size + 1 == divisor->size and dividend = 0x8000 (0x80000000...)
 *  and divisor = 0x00008000 (0x000080000000...). In this cases the
 *  remainder is 0. In all other cases the remainder is dividend.
 */
static bigIntType bigRemSizeLess (const const_bigIntType dividend,
    const const_bigIntType divisor)

  {
    memSizeType pos;
    boolType remainderIs0;
    bigIntType remainder;

  /* bigRemSizeLess */
    if (dividend->size + 1 == divisor->size &&
        dividend->bigdigits[dividend->size - 1] == BIGDIGIT_SIGN &&
        divisor->bigdigits[divisor->size - 1] == 0 &&
        divisor->bigdigits[divisor->size - 2] == BIGDIGIT_SIGN) {
      remainderIs0 = TRUE;
      for (pos = 0; pos < dividend->size - 1; pos++) {
        if (dividend->bigdigits[pos] != 0 || divisor->bigdigits[pos] != 0) {
          remainderIs0 = FALSE;
        } /* if */
      } /* for */
    } else {
      remainderIs0 = FALSE;
    } /* if */
    if (remainderIs0) {
      if (unlikely(!ALLOC_BIG_SIZE_OK(remainder, 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        remainder->size = 1;
        remainder->bigdigits[0] = 0;
      } /* if */
    } else {
      if (unlikely(!ALLOC_BIG_SIZE_OK(remainder, dividend->size))) {
        raise_error(MEMORY_ERROR);
      } else {
        remainder->size = dividend->size;
        memcpy(remainder->bigdigits, dividend->bigdigits,
            (size_t) dividend->size * sizeof(bigDigitType));
      } /* if */
    } /* if */
    return remainder;
  } /* bigRemSizeLess */



/**
 *  Adds big2 to big1 at the digit position pos1. Big1 and big2
 *  are signed big integer values. The size of big1 must be
 *  greater or equal the size of big2.
 */
static void bigAddTo (const bigIntType big1, const const_bigIntType big2)

  {
    memSizeType pos;
    doubleBigDigitType carry = 0;
    doubleBigDigitType big2_sign;

  /* bigAddTo */
    pos = 0;
    do {
      carry += (doubleBigDigitType) big1->bigdigits[pos] + big2->bigdigits[pos];
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big2->size);
    big2_sign = IS_NEGATIVE(big2->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
    for (; pos < big1->size; pos++) {
      carry += big1->bigdigits[pos] + big2_sign;
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
    } /* for */
  } /* bigAddTo */



/**
 *  Computes the modulo of the integer division dividend by divisor for
 *  signed big integers when dividend has less digits than divisor. The memory
 *  for the modulo is requested and the normalized modulo is returned.
 *  Normally dividend->size < divisor->size implies abs(dividend) < abs(divisor).
 *  When abs(dividend) < abs(divisor) holds the division gives 0. The cases
 *  when dividend->size < divisor->size and abs(dividend) = abs(divisor) are if
 *  dividend->size + 1 == divisor->size and dividend = 0x8000 (0x80000000...)
 *  and divisor = 0x00008000 (0x000080000000...). In this cases the
 *  modulo is 0. In all other cases the modulo is dividend or dividend +
 *  divisor when dividend and divisor have different signs.
 */
static bigIntType bigModSizeLess (const const_bigIntType dividend,
    const const_bigIntType divisor)

  {
    memSizeType pos;
    boolType moduloIs0;
    bigIntType modulo;

  /* bigModSizeLess */
    if (dividend->size + 1 == divisor->size &&
        dividend->bigdigits[dividend->size - 1] == BIGDIGIT_SIGN &&
        divisor->bigdigits[divisor->size - 1] == 0 &&
        divisor->bigdigits[divisor->size - 2] == BIGDIGIT_SIGN) {
      moduloIs0 = TRUE;
      for (pos = 0; pos < dividend->size - 1; pos++) {
        if (dividend->bigdigits[pos] != 0 || divisor->bigdigits[pos] != 0) {
          moduloIs0 = FALSE;
        } /* if */
      } /* for */
    } else {
      moduloIs0 = FALSE;
    } /* if */
    if (moduloIs0) {
      if (unlikely(!ALLOC_BIG_SIZE_OK(modulo, 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        modulo->size = 1;
        modulo->bigdigits[0] = 0;
      } /* if */
    } else {
      if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1]) !=
          IS_NEGATIVE(divisor->bigdigits[divisor->size - 1])) {
        if (unlikely(!ALLOC_BIG_SIZE_OK(modulo, divisor->size))) {
          raise_error(MEMORY_ERROR);
        } else {
          modulo->size = divisor->size;
          memcpy(modulo->bigdigits, divisor->bigdigits,
              (size_t) divisor->size * sizeof(bigDigitType));
          bigAddTo(modulo, dividend);
          modulo = normalize(modulo);
        } /* if */
      } else {
        if (unlikely(!ALLOC_BIG_SIZE_OK(modulo, dividend->size))) {
          raise_error(MEMORY_ERROR);
        } else {
          modulo->size = dividend->size;
          memcpy(modulo->bigdigits, dividend->bigdigits,
              (size_t) dividend->size * sizeof(bigDigitType));
        } /* if */
      } /* if */
    } /* if */
    return modulo;
  } /* bigModSizeLess */



/**
 *  Computes the remainder of an integer division of dividend by divisor
 *  for nonnegative big integers. The remainder is delivered in
 *  dividend. There are several preconditions for this function. Divisor
 *  must have at least 2 digits and dividend must have at least one
 *  digit more than divisor. If dividend and divisor have the same length in
 *  digits nothing is done. The most significant bit of divisor must be
 *  set. The most significant digit of dividend must be less than the
 *  most significant digit of divisor. The computations to meet this
 *  predonditions are done outside this function. The special cases
 *  with a one digit divisor or a dividend with less digits than divisor are
 *  handled in other functions. This algorithm based on the algorithm
 *  from D.E. Knuth described in "The art of computer programming"
 *  volume 2 (Seminumerical algorithms).
 */
static void uBigRem (const bigIntType dividend, const const_bigIntType divisor)

  {
    memSizeType pos1;
    doubleBigDigitType twodigits;
    doubleBigDigitType remainder;
    bigDigitType quotientdigit;
    bigDigitType sbtr_carry;

  /* uBigRem */
    for (pos1 = dividend->size - 1; pos1 >= divisor->size; pos1--) {
      twodigits = (((doubleBigDigitType) dividend->bigdigits[pos1]) << BIGDIGIT_SIZE) |
          dividend->bigdigits[pos1 - 1];
      if (unlikely(dividend->bigdigits[pos1] == divisor->bigdigits[divisor->size - 1])) {
        quotientdigit = BIGDIGIT_MASK;
      } else {
        quotientdigit = (bigDigitType) (twodigits / divisor->bigdigits[divisor->size - 1]);
      } /* if */
      remainder = twodigits - (doubleBigDigitType) quotientdigit *
          divisor->bigdigits[divisor->size - 1];
      while (remainder <= BIGDIGIT_MASK &&
          (doubleBigDigitType) divisor->bigdigits[divisor->size - 2] * quotientdigit >
          (remainder << BIGDIGIT_SIZE | dividend->bigdigits[pos1 - 2])) {
        quotientdigit--;
        remainder = twodigits - (doubleBigDigitType) quotientdigit *
            divisor->bigdigits[divisor->size - 1];
      } /* while */
      sbtr_carry = uBigMultSub(dividend, divisor, quotientdigit, pos1 - divisor->size);
      if (sbtr_carry == 0) {
        uBigAddTo(dividend, divisor, pos1 - divisor->size);
      } /* if */
    } /* for */
  } /* uBigRem */



static void uBigDigitAdd (const bigDigitType *const big1, const memSizeType size1,
    const bigDigitType *const big2, const memSizeType size2, bigDigitType *const result)

  {
    memSizeType pos;
    doubleBigDigitType carry = 0;

  /* uBigDigitAdd */
    pos = 0;
    do {
      carry += (doubleBigDigitType) big1[pos] + big2[pos];
      result[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < size2);
    for (; pos < size1; pos++) {
      carry += big1[pos];
      result[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
    } /* for */
    result[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
  } /* uBigDigitAdd */



static void uBigDigitSbtrFrom (bigDigitType *const big1, const memSizeType size1,
    const bigDigitType *const big2, const memSizeType size2)

  {
    memSizeType pos;
    doubleBigDigitType carry = 1;

  /* uBigDigitSbtrFrom */
    pos = 0;
    do {
      carry += (doubleBigDigitType) big1[pos] +
          (~big2[pos] & BIGDIGIT_MASK);
      big1[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < size2);
    for (; carry == 0 && pos < size1; pos++) {
      carry = (doubleBigDigitType) big1[pos] + BIGDIGIT_MASK;
      big1[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
    } /* for */
  } /* uBigDigitSbtrFrom */



static void uBigDigitAddTo (bigDigitType *const big1,  const memSizeType size1,
    const bigDigitType *const big2, const memSizeType size2)

  {
    memSizeType pos;
    doubleBigDigitType carry = 0;

  /* uBigDigitAddTo */
    pos = 0;
    do {
      carry += (doubleBigDigitType) big1[pos] + big2[pos];
      big1[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < size2);
    for (; carry != 0 && pos < size1; pos++) {
      carry += big1[pos];
      big1[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
    } /* for */
  } /* uBigDigitAddTo */



static void uBigDigitMult (const bigDigitType *const factor1,
    const bigDigitType *const factor2, const memSizeType size,
    bigDigitType *const product)

  {
    memSizeType pos1;
    memSizeType pos2;
    doubleBigDigitType carry;
    doubleBigDigitType carry2 = 0;
    doubleBigDigitType prod;

  /* uBigDigitMult */
    carry = (doubleBigDigitType) factor1[0] * factor2[0];
    product[0] = (bigDigitType) (carry & BIGDIGIT_MASK);
    carry >>= BIGDIGIT_SIZE;
    for (pos1 = 1; pos1 < size; pos1++) {
      pos2 = 0;
      do {
        prod = (doubleBigDigitType) factor1[pos2] * factor2[pos1 - pos2];
        carry2 += carry > (doubleBigDigitType) ~prod ? 1 : 0;
        carry += prod;
        pos2++;
      } while (pos2 <= pos1);
      product[pos1] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      carry |= (carry2 & BIGDIGIT_MASK) << BIGDIGIT_SIZE;
      carry2 >>= BIGDIGIT_SIZE;
    } /* for */
    for (; pos1 < size + size - 1; pos1++) {
      pos2 = pos1 - size + 1;
      do {
        prod = (doubleBigDigitType) factor1[pos2] * factor2[pos1 - pos2];
        carry2 += carry > (doubleBigDigitType) ~prod ? 1 : 0;
        carry += prod;
        pos2++;
      } while (pos2 < size);
      product[pos1] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      carry |= (carry2 & BIGDIGIT_MASK) << BIGDIGIT_SIZE;
      carry2 >>= BIGDIGIT_SIZE;
    } /* for */
    product[pos1] = (bigDigitType) (carry & BIGDIGIT_MASK);
  } /* uBigDigitMult */



static void uBigKaratsubaMult (const bigDigitType *const factor1,
    const bigDigitType *const factor2, const memSizeType size,
    bigDigitType *const product, bigDigitType *const temp)

  {
    memSizeType sizeLo;
    memSizeType sizeHi;

  /* uBigKaratsubaMult */
    if (size < KARATSUBA_THRESHOLD) {
      uBigDigitMult(factor1, factor2, size, product);
    } else {
      sizeHi = size >> 1;
      sizeLo = size - sizeHi;
      uBigDigitAdd(factor1, sizeLo, &factor1[sizeLo], sizeHi, product);
      uBigDigitAdd(factor2, sizeLo, &factor2[sizeLo], sizeHi, &product[size]);
      uBigKaratsubaMult(product, &product[size], sizeLo + 1, temp, &temp[(sizeLo + 1) << 1]);
      uBigKaratsubaMult(factor1, factor2, sizeLo, product, &temp[(sizeLo + 1) << 1]);
      uBigKaratsubaMult(&factor1[sizeLo], &factor2[sizeLo], sizeHi, &product[sizeLo << 1], &temp[(sizeLo + 1) << 1]);
      uBigDigitSbtrFrom(temp, (sizeLo + 1) << 1, product, sizeLo << 1);
      uBigDigitSbtrFrom(temp, (sizeLo + 1) << 1, &product[sizeLo << 1], sizeHi << 1);
      uBigDigitAddTo(&product[sizeLo], sizeLo + (sizeHi << 1), temp, (sizeLo + 1) << 1);
    } /* if */
  } /* uBigKaratsubaMult */



static void uBigDigitSquare (const bigDigitType *const big1,
    const memSizeType size, bigDigitType *const result)

  {
    memSizeType pos1;
    memSizeType pos2;
    doubleBigDigitType carry;
    doubleBigDigitType product;
    bigDigitType digit;

  /* uBigDigitSquare */
    digit = big1[0];
    carry = (doubleBigDigitType) digit * digit;
    result[0] = (bigDigitType) (carry & BIGDIGIT_MASK);
    carry >>= BIGDIGIT_SIZE;
    if (size == 1) {
      result[1] = (bigDigitType) (carry);
    } else {
      for (pos2 = 1; pos2 < size; pos2++) {
        product = (doubleBigDigitType) digit * big1[pos2];
        carry += (product << 1) & BIGDIGIT_MASK;
        result[pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        carry += product >> (BIGDIGIT_SIZE - 1);
      } /* for */
      result[pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
      result[pos2 + 1] = (bigDigitType) (carry >> BIGDIGIT_SIZE);
      for (pos1 = 1; pos1 < size; pos1++) {
        digit = big1[pos1];
        carry = (doubleBigDigitType) result[pos1 << 1] +
            (doubleBigDigitType) digit * digit;
        result[pos1 << 1] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        for (pos2 = pos1 + 1; pos2 < size; pos2++) {
          product = (doubleBigDigitType) digit * big1[pos2];
          carry += (doubleBigDigitType) result[pos1 + pos2] +
              ((product << 1) & BIGDIGIT_MASK);
          result[pos1 + pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          carry += product >> (BIGDIGIT_SIZE - 1);
        } /* for */
        carry += (doubleBigDigitType) result[pos1 + pos2];
        result[pos1 + pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
        if (pos1 < size - 1) {
          result[pos1 + pos2 + 1] = (bigDigitType) (carry >> BIGDIGIT_SIZE);
        } /* if */
      } /* for */
    } /* if */
  } /* uBigDigitSquare */



static void uBigKaratsubaSquare (const bigDigitType *const big1,
    const memSizeType size, bigDigitType *const result, bigDigitType *const temp)

  {
    memSizeType sizeLo;
    memSizeType sizeHi;

  /* uBigKaratsubaSquare */
    /* printf("uBigKaratsubaSquare: size=%lu\n", size); */
    if (size < KARATSUBA_SQUARE_THRESHOLD) {
      uBigDigitSquare(big1, size, result);
    } else {
      sizeHi = size >> 1;
      sizeLo = size - sizeHi;
      uBigDigitAdd(big1, sizeLo, &big1[sizeLo], sizeHi, result);
      uBigKaratsubaSquare(result, sizeLo + 1, temp, &temp[(sizeLo + 1) << 1]);
      uBigKaratsubaSquare(big1, sizeLo, result, &temp[(sizeLo + 1) << 1]);
      uBigKaratsubaSquare(&big1[sizeLo], sizeHi, &result[sizeLo << 1], &temp[(sizeLo + 1) << 1]);
      uBigDigitSbtrFrom(temp, (sizeLo + 1) << 1, result, sizeLo << 1);
      uBigDigitSbtrFrom(temp, (sizeLo + 1) << 1, &result[sizeLo << 1], sizeHi << 1);
      uBigDigitAddTo(&result[sizeLo], sizeLo + (sizeHi << 1), temp, (sizeLo + 1) << 1);
    } /* if */
  } /* uBigKaratsubaSquare */


static void uBigMultPositiveWithDigit (const const_bigIntType factor1,
    const bigDigitType factor2_digit, const bigIntType product)

  {
    memSizeType pos;
    doubleBigDigitType mult_carry;

  /* uBigMultPositiveWithDigit */
    mult_carry = (doubleBigDigitType) factor1->bigdigits[0] * factor2_digit;
    product->bigdigits[0] = (bigDigitType) (mult_carry & BIGDIGIT_MASK);
    mult_carry >>= BIGDIGIT_SIZE;
    for (pos = 1; pos < factor1->size; pos++) {
      mult_carry += (doubleBigDigitType) factor1->bigdigits[pos] * factor2_digit;
      product->bigdigits[pos] = (bigDigitType) (mult_carry & BIGDIGIT_MASK);
      mult_carry >>= BIGDIGIT_SIZE;
    } /* for */
    product->bigdigits[pos] = (bigDigitType) (mult_carry & BIGDIGIT_MASK);
  } /* uBigMultPositiveWithDigit */



static void uBigMultNegativeWithDigit (const const_bigIntType factor1,
    const bigDigitType factor2_digit, const bigIntType product)

  {
    memSizeType pos;
    doubleBigDigitType negate_carry = 1;
    doubleBigDigitType mult_carry;
    doubleBigDigitType product_carry = 1;

  /* uBigMultNegativeWithDigit */
    negate_carry += ~factor1->bigdigits[0] & BIGDIGIT_MASK;
    mult_carry = (negate_carry & BIGDIGIT_MASK) * factor2_digit;
    product_carry += ~mult_carry & BIGDIGIT_MASK;
    product->bigdigits[0] = (bigDigitType) (product_carry & BIGDIGIT_MASK);
    negate_carry >>= BIGDIGIT_SIZE;
    mult_carry >>= BIGDIGIT_SIZE;
    product_carry >>= BIGDIGIT_SIZE;
    for (pos = 1; pos < factor1->size; pos++) {
      negate_carry += ~factor1->bigdigits[pos] & BIGDIGIT_MASK;
      mult_carry += (negate_carry & BIGDIGIT_MASK) * factor2_digit;
      product_carry += ~mult_carry & BIGDIGIT_MASK;
      product->bigdigits[pos] = (bigDigitType) (product_carry & BIGDIGIT_MASK);
      negate_carry >>= BIGDIGIT_SIZE;
      mult_carry >>= BIGDIGIT_SIZE;
      product_carry >>= BIGDIGIT_SIZE;
    } /* for */
    product_carry += ~mult_carry & BIGDIGIT_MASK;
    product->bigdigits[pos] = (bigDigitType) (product_carry & BIGDIGIT_MASK);
  } /* uBigMultNegativeWithDigit */



static void uBigMultPositiveWithNegatedDigit (const const_bigIntType factor1,
    const bigDigitType factor2_digit, const bigIntType product)

  {
    memSizeType pos;
    doubleBigDigitType mult_carry;
    doubleBigDigitType product_carry = 1;

  /* uBigMultPositiveWithNegatedDigit */
    mult_carry = (doubleBigDigitType) factor1->bigdigits[0] * factor2_digit;
    product_carry += ~mult_carry & BIGDIGIT_MASK;
    product->bigdigits[0] = (bigDigitType) (product_carry & BIGDIGIT_MASK);
    mult_carry >>= BIGDIGIT_SIZE;
    product_carry >>= BIGDIGIT_SIZE;
    for (pos = 1; pos < factor1->size; pos++) {
      mult_carry += (doubleBigDigitType) factor1->bigdigits[pos] * factor2_digit;
      product_carry += ~mult_carry & BIGDIGIT_MASK;
      product->bigdigits[pos] = (bigDigitType) (product_carry & BIGDIGIT_MASK);
      mult_carry >>= BIGDIGIT_SIZE;
      product_carry >>= BIGDIGIT_SIZE;
    } /* for */
    product_carry += ~mult_carry & BIGDIGIT_MASK;
    product->bigdigits[pos] = (bigDigitType) (product_carry & BIGDIGIT_MASK);
  } /* uBigMultPositiveWithNegatedDigit */



static void uBigMultNegativeWithNegatedDigit (const const_bigIntType factor1,
    const bigDigitType factor2_digit, const bigIntType product)

  {
    memSizeType pos;
    doubleBigDigitType negate_carry = 1;
    doubleBigDigitType mult_carry;

  /* uBigMultNegativeWithNegatedDigit */
    negate_carry += ~factor1->bigdigits[0] & BIGDIGIT_MASK;
    mult_carry = (negate_carry & BIGDIGIT_MASK) * factor2_digit;
    product->bigdigits[0] = (bigDigitType) (mult_carry & BIGDIGIT_MASK);
    negate_carry >>= BIGDIGIT_SIZE;
    mult_carry >>= BIGDIGIT_SIZE;
    for (pos = 1; pos < factor1->size; pos++) {
      negate_carry += ~factor1->bigdigits[pos] & BIGDIGIT_MASK;
      mult_carry += (negate_carry & BIGDIGIT_MASK) * factor2_digit;
      product->bigdigits[pos] = (bigDigitType) (mult_carry & BIGDIGIT_MASK);
      negate_carry >>= BIGDIGIT_SIZE;
      mult_carry >>= BIGDIGIT_SIZE;
    } /* for */
    product->bigdigits[pos] = (bigDigitType) (mult_carry & BIGDIGIT_MASK);
  } /* uBigMultNegativeWithNegatedDigit */



static void uBigMult (const_bigIntType factor1, const_bigIntType factor2,
    const bigIntType product)

  {
    const_bigIntType help_big;
    memSizeType pos1;
    memSizeType pos2;
    doubleBigDigitType carry;
    doubleBigDigitType carry2 = 0;
    doubleBigDigitType prod;

  /* uBigMult */
    if (factor2->size > factor1->size) {
      help_big = factor1;
      factor1 = factor2;
      factor2 = help_big;
    } /* if */
    carry = (doubleBigDigitType) factor1->bigdigits[0] * factor2->bigdigits[0];
    product->bigdigits[0] = (bigDigitType) (carry & BIGDIGIT_MASK);
    carry >>= BIGDIGIT_SIZE;
    if (factor2->size == 1) {
      for (pos1 = 1; pos1 < factor1->size; pos1++) {
        carry += (doubleBigDigitType) factor1->bigdigits[pos1] * factor2->bigdigits[0];
        product->bigdigits[pos1] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
      } /* for */
    } else {
      for (pos1 = 1; pos1 < factor2->size; pos1++) {
        pos2 = 0;
        do {
          prod = (doubleBigDigitType) factor1->bigdigits[pos2] * factor2->bigdigits[pos1 - pos2];
          carry2 += carry > (doubleBigDigitType) ~prod ? 1 : 0;
          carry += prod;
          pos2++;
        } while (pos2 <= pos1);
        product->bigdigits[pos1] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        carry |= (carry2 & BIGDIGIT_MASK) << BIGDIGIT_SIZE;
        carry2 >>= BIGDIGIT_SIZE;
      } /* for */
      for (; pos1 < factor1->size; pos1++) {
        pos2 = pos1 - factor2->size + 1;
        do {
          prod = (doubleBigDigitType) factor1->bigdigits[pos2] * factor2->bigdigits[pos1 - pos2];
          carry2 += carry > (doubleBigDigitType) ~prod ? 1 : 0;
          carry += prod;
          pos2++;
        } while (pos2 <= pos1);
        product->bigdigits[pos1] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        carry |= (carry2 & BIGDIGIT_MASK) << BIGDIGIT_SIZE;
        carry2 >>= BIGDIGIT_SIZE;
      } /* for */
      for (; pos1 < factor1->size + factor2->size - 1; pos1++) {
        pos2 = pos1 - factor2->size + 1;
        do {
          prod = (doubleBigDigitType) factor1->bigdigits[pos2] * factor2->bigdigits[pos1 - pos2];
          carry2 += carry > (doubleBigDigitType) ~prod ? 1 : 0;
          carry += prod;
          pos2++;
        } while (pos2 < factor1->size);
        product->bigdigits[pos1] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        carry |= (carry2 & BIGDIGIT_MASK) << BIGDIGIT_SIZE;
        carry2 >>= BIGDIGIT_SIZE;
      } /* for */
    } /* if */
    product->bigdigits[pos1] = (bigDigitType) (carry & BIGDIGIT_MASK);
  } /* uBigMult */



#ifdef OUT_OF_ORDER
static void uBigMult (const const_bigIntType factor1, const const_bigIntType factor2,
    const bigIntType product)

  {
    memSizeType pos1;
    memSizeType pos2;
    doubleBigDigitType carry;
    doubleBigDigitType carry2 = 0;
    doubleBigDigitType prod;

  /* uBigMult */
    carry = (doubleBigDigitType) factor1->bigdigits[0] * factor2->bigdigits[0];
    product->bigdigits[0] = (bigDigitType) (carry & BIGDIGIT_MASK);
    carry >>= BIGDIGIT_SIZE;
    for (pos1 = 1; pos1 < factor1->size + factor2->size - 1; pos1++) {
      if (pos1 < factor2->size) {
        pos2 = 0;
      } else {
        pos2 = pos1 - factor2->size + 1;
      } /* if */
      if (pos1 < factor1->size) {
        pos3 = pos1 + 1;
      } else {
        pos3 = factor1->size;
      } /* if */
      do {
        prod = (doubleBigDigitType) factor1->bigdigits[pos2] * factor2->bigdigits[pos1 - pos2];
        /* To avoid overflows of carry + prod it is necessary     */
        /* to check if carry + prod > DOUBLEBIGDIGIT_MASK which   */
        /* is equivalent to carry > DOUBLEBIGDIGIT_MASK - prod.   */
        /* A subtraction can be replaced by adding the negated    */
        /* value: carry > DOUBLEBIGDIGIT_MASK + ~prod + 1. This   */
        /* can be simplified to carry > ~prod.                    */
        carry2 += carry > ~prod ? 1 : 0;
        carry += prod;
        pos2++;
      } while (pos2 < pos3);
      product->bigdigits[pos1] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      carry |= (carry2 & BIGDIGIT_MASK) << BIGDIGIT_SIZE;
      carry2 >>= BIGDIGIT_SIZE;
    } /* for */
    product->bigdigits[pos1] = (bigDigitType) (carry & BIGDIGIT_MASK);
  } /* uBigMult */
#endif



#ifdef OUT_OF_ORDER
static void uBigMult (const const_bigIntType factor1, const const_bigIntType factor2,
    const bigIntType product)

  {
    memSizeType pos1;
    memSizeType pos2;
    doubleBigDigitType carry = 0;

  /* uBigMult */
    pos2 = 0;
    do {
      carry += (doubleBigDigitType) factor1->bigdigits[0] * factor2->bigdigits[pos2];
      product->bigdigits[pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos2++;
    } while (pos2 < factor2->size);
    product->bigdigits[factor2->size] = (bigDigitType) (carry & BIGDIGIT_MASK);
    for (pos1 = 1; pos1 < factor1->size; pos1++) {
      carry = 0;
      pos2 = 0;
      do {
        carry += (doubleBigDigitType) product->bigdigits[pos1 + pos2] +
            (doubleBigDigitType) factor1->bigdigits[pos1] * factor2->bigdigits[pos2];
        product->bigdigits[pos1 + pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos2++;
      } while (pos2 < factor2->size);
      product->bigdigits[pos1 + factor2->size] = (bigDigitType) (carry & BIGDIGIT_MASK);
    } /* for */
  } /* uBigMult */
#endif



static void bigMultAssign1 (bigIntType *const big_variable, bigDigitType factor_digit)

  {
    const_bigIntType big1;
    boolType negative = FALSE;
    const_bigIntType big1_help = NULL;
    memSizeType pos;
    doubleBigDigitType carry = 0;
    bigIntType product;

  /* bigMultAssign1 */
    big1 = *big_variable;
    if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
      negative = TRUE;
      big1_help = alloc_positive_copy_of_negative_big(big1);
      big1 = big1_help;
      if (unlikely(big1_help == NULL)) {
        raise_error(MEMORY_ERROR);
        return;
      } /* if */
    } /* if */
    if (IS_NEGATIVE(factor_digit)) {
      negative = !negative;
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow when the smallest signed integer is negated.   */
      factor_digit = -factor_digit;
    } /* if */
    if (unlikely(!ALLOC_BIG(product, big1->size + 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      for (pos = 0; pos < big1->size; pos++) {
        carry += (doubleBigDigitType) big1->bigdigits[pos] * factor_digit;
        product->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
      } /* for */
      product->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      product->size = big1->size + 1;
      if (negative) {
        negate_positive_big(product);
      } /* if */
      product = normalize(product);
      FREE_BIG(*big_variable, (*big_variable)->size);
      *big_variable = product;
    } /* if */
    if (big1_help != NULL) {
      FREE_BIG(big1_help, big1_help->size);
    } /* if */
  } /* bigMultAssign1 */



static bigIntType uBigMultK (const_bigIntType factor1, const_bigIntType factor2,
    const boolType negative)

  {
    const_bigIntType help_big;
    bigIntType factor2_help;
    bigIntType temp;
    bigIntType product;

  /* uBigMultK */
    if (factor1->size >= KARATSUBA_THRESHOLD && factor2->size >= KARATSUBA_THRESHOLD) {
      if (factor2->size > factor1->size) {
        help_big = factor1;
        factor1 = factor2;
        factor2 = help_big;
      } /* if */
      if (factor2->size << 1 <= factor1->size) {
        if (unlikely(!ALLOC_BIG_SIZE_OK(factor2_help, factor1->size - (factor1->size >> 1)))) {
          raise_error(MEMORY_ERROR);
          return NULL;
        } else {
          factor2_help->size = factor1->size - (factor1->size >> 1);
          memcpy(factor2_help->bigdigits, factor2->bigdigits,
              (size_t) factor2->size * sizeof(bigDigitType));
          memset(&factor2_help->bigdigits[factor2->size], 0,
              (size_t) (factor2_help->size - factor2->size) * sizeof(bigDigitType));
          factor2 = factor2_help;
          if (unlikely(!ALLOC_BIG(product, (factor1->size >> 1) + (factor2->size << 1)))) {
            raise_error(MEMORY_ERROR);
          } else {
            product->size = (factor1->size >> 1) + (factor2->size << 1);
            if (unlikely(!ALLOC_BIG(temp, factor1->size << 2))) {
              raise_error(MEMORY_ERROR);
            } else {
              uBigKaratsubaMult(factor1->bigdigits, factor2->bigdigits,
                  factor1->size >> 1, product->bigdigits, temp->bigdigits);
              uBigKaratsubaMult(&factor1->bigdigits[factor1->size >> 1], factor2->bigdigits,
                  factor2->size, temp->bigdigits, &temp->bigdigits[factor2->size << 1]);
              memset(&product->bigdigits[(factor1->size >> 1) << 1], 0,
                  (size_t) (product->size - ((factor1->size >> 1) << 1)) * sizeof(bigDigitType));
              uBigDigitAddTo(&product->bigdigits[factor1->size >> 1], product->size - (factor1->size >> 1),
                  temp->bigdigits, factor2->size << 1);
              if (negative) {
                negate_positive_big(product);
              } /* if */
              product = normalize(product);
              FREE_BIG(temp, factor1->size << 2);
            } /* if */
          } /* if */
          FREE_BIG(factor2_help, factor1->size - (factor1->size >> 1));
        } /* if */
      } else {
        if (unlikely(!ALLOC_BIG_SIZE_OK(factor2_help, factor1->size))) {
          raise_error(MEMORY_ERROR);
          return NULL;
        } else {
          factor2_help->size = factor1->size;
          memcpy(factor2_help->bigdigits, factor2->bigdigits,
              (size_t) factor2->size * sizeof(bigDigitType));
          memset(&factor2_help->bigdigits[factor2->size], 0,
              (size_t) (factor2_help->size - factor2->size) * sizeof(bigDigitType));
          factor2 = factor2_help;
          if (unlikely(!ALLOC_BIG(product, factor1->size << 1))) {
            raise_error(MEMORY_ERROR);
          } else {
            if (unlikely(!ALLOC_BIG(temp, factor1->size << 2))) {
              raise_error(MEMORY_ERROR);
            } else {
              uBigKaratsubaMult(factor1->bigdigits, factor2->bigdigits,
                  factor1->size, product->bigdigits, temp->bigdigits);
              product->size = factor1->size << 1;
              if (negative) {
                negate_positive_big(product);
              } /* if */
              product = normalize(product);
              FREE_BIG(temp, factor1->size << 2);
            } /* if */
          } /* if */
          FREE_BIG(factor2_help, factor1->size);
        } /* if */
      } /* if */
    } else {
      if (unlikely(!ALLOC_BIG(product, factor1->size + factor2->size))) {
        raise_error(MEMORY_ERROR);
      } else {
        product->size = factor1->size + factor2->size;
        uBigMult(factor1, factor2, product);
        if (negative) {
          negate_positive_big(product);
        } /* if */
        product = normalize(product);
      } /* if */
    } /* if */
    return product;
  } /* uBigMultK */



static bigIntType uBigSquareK (const_bigIntType big1)

  {
    bigIntType temp;
    bigIntType result;

  /* uBigSquareK */
    if (big1->size >= KARATSUBA_SQUARE_THRESHOLD) {
      if (unlikely(!ALLOC_BIG(result, big1->size << 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        if (unlikely(!ALLOC_BIG(temp, big1->size << 2))) {
          raise_error(MEMORY_ERROR);
        } else {
          uBigKaratsubaSquare(big1->bigdigits, big1->size,
              result->bigdigits, temp->bigdigits);
          result->size = big1->size << 1;
          result = normalize(result);
          FREE_BIG(temp, big1->size << 2);
        } /* if */
      } /* if */
    } else {
      if (unlikely(!ALLOC_BIG(result, big1->size + big1->size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = big1->size + big1->size;
        uBigDigitSquare(big1->bigdigits, big1->size, result->bigdigits);
        result = normalize(result);
      } /* if */
    } /* if */
    return result;
  } /* uBigSquareK */



/**
 *  Returns the product of factor1 by factor2 for nonnegative big integers.
 *  The result is written into big_help (which is a scratch variable
 *  and is assumed to contain enough memory). Before returning the
 *  result the variable factor1 is assigned to big_help. This way it is
 *  possible to write number = uBigMultIntoHelp(number, base, &big_help).
 *  Note that the old number is in the scratch variable big_help
 *  afterwards.
 */
static bigIntType uBigMultIntoHelp (const bigIntType factor1,
    const const_bigIntType factor2, bigIntType *const big_help)

  {
    memSizeType pos1;
    bigDigitType digit;
    bigIntType product;

  /* uBigMultIntoHelp */
    /* printf("uBigMultIntoHelp(factor1->size=%lu, factor2->size=%lu)\n", factor1->size, factor2->size); */
    product = *big_help;
    uBigMult(factor1, factor2, product);
    pos1 = factor1->size + factor2->size;
    pos1--;
    digit = product->bigdigits[pos1];
    if (digit == 0) {
      do {
        pos1--;
        digit = product->bigdigits[pos1];
      } while (pos1 > 0 && digit == 0);
      if (IS_NEGATIVE(digit)) {
        pos1++;
      } /* if */
    } /* if */
    pos1++;
    product->size = pos1;
    *big_help = factor1;
    /* printf("uBigMultIntoHelp(factor1->size=%lu, factor2->size=%lu) => product->size=%lu\n",
           factor1->size, factor2->size, product->size);
       prot_bigint(product);
       printf("\n"); */
    return product;
  } /* uBigMultIntoHelp */



/**
 *  Returns the square of the nonnegative big integer big1. The result
 *  is written into big_help (which is a scratch variable and is
 *  assumed to contain enough memory). Before returning the result
 *  the variable big1 is assigned to big_help. This way it is possible
 *  to square a given base with base = uBigSquare(base, &big_help).
 *  Note that the old base is in the scratch variable big_help
 *  afterwards. This squaring algorithm takes into account that
 *  digit1 * digit2 + digit2 * digit1 == (digit1 * digit2) << 1.
 *  This reduces the number of multiplications approx. by factor 2.
 *  Unfortunately one bit more than sizeof(doubleBigDigitType) is
 *  needed to store the shifted product. Therefore extra effort is
 *  necessary to avoid an overflow.
 */
static bigIntType uBigSquare (const bigIntType big1, bigIntType *big_help)

  {
    memSizeType pos1;
    memSizeType pos2;
    doubleBigDigitType carry;
    doubleBigDigitType product;
    bigDigitType digit;
    bigIntType result;

  /* uBigSquare */
    /* printf("uBigSquare(big1->size=%lu)\n", big1->size); */
    result = *big_help;
    digit = big1->bigdigits[0];
    carry = (doubleBigDigitType) digit * digit;
    result->bigdigits[0] = (bigDigitType) (carry & BIGDIGIT_MASK);
    carry >>= BIGDIGIT_SIZE;
    for (pos2 = 1; pos2 < big1->size; pos2++) {
      product = (doubleBigDigitType) digit * big1->bigdigits[pos2];
      carry += (product << 1) & BIGDIGIT_MASK;
      result->bigdigits[pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      carry += product >> (BIGDIGIT_SIZE - 1);
    } /* for */
    result->bigdigits[pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
    for (pos1 = 1; pos1 < big1->size; pos1++) {
      digit = big1->bigdigits[pos1];
      carry = (doubleBigDigitType) result->bigdigits[pos1 + pos1] +
          (doubleBigDigitType) digit * digit;
      result->bigdigits[pos1 + pos1] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      for (pos2 = pos1 + 1; pos2 < big1->size; pos2++) {
        product = (doubleBigDigitType) digit * big1->bigdigits[pos2];
        carry += (doubleBigDigitType) result->bigdigits[pos1 + pos2] +
            ((product << 1) & BIGDIGIT_MASK);
        result->bigdigits[pos1 + pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        carry += product >> (BIGDIGIT_SIZE - 1);
      } /* for */
      result->bigdigits[pos1 + pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
    } /* for */
    pos1 = big1->size + big1->size;
    pos1--;
    digit = result->bigdigits[pos1];
    if (digit == 0) {
      do {
        pos1--;
        digit = result->bigdigits[pos1];
      } while (pos1 > 0 && digit == 0);
      if (IS_NEGATIVE(digit)) {
        pos1++;
      } /* if */
    } /* if */
    pos1++;
    result->size = pos1;
    *big_help = big1;
    /* printf("uBigSquare(big1->size=%lu) => result->size=%lu\n", big1->size, result->size);
       prot_bigint(result);
       printf("\n"); */
    return result;
  } /* uBigSquare */



/**
 *  Computes base to the power of exponent for signed big integers.
 *  It is assumed that the exponent and base both are >= 1.
 *  The result variable is set to base or 1 depending on the
 *  rightmost bit of the exponent. After that the base is
 *  squared in a loop and every time the corresponding bit of
 *  the exponent is set the current square is multiplied
 *  with the result variable. This reduces the number of square
 *  operations to ld(exponent).
 */
static bigIntType bigIPowN (const bigDigitType base, intType exponent, unsigned int bit_size)

  {
    memSizeType help_size;
    bigIntType square;
    bigIntType big_help;
    bigIntType result;

  /* bigIPowN */
    /* printf("bigIPowN(%lu, %lu, %u)\n", base, exponent, bit_size); */
    /* help_size = (bit_size * ((uintType) exponent) - 1) / BIGDIGIT_SIZE + 2; */
    /* printf("help_sizeA=%ld\n", help_size); */
    if (unlikely((uintType) exponent + 1 > MAX_BIG_LEN)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      help_size = (memSizeType) ((uintType) exponent + 1);
      /* printf("help_sizeB=%ld\n", help_size); */
      if (unlikely(!ALLOC_BIG_SIZE_OK(square, help_size))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else if (unlikely(!ALLOC_BIG_SIZE_OK(big_help, help_size))) {
        FREE_BIG(square,  help_size);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else if (unlikely(!ALLOC_BIG_SIZE_OK(result, help_size))) {
        FREE_BIG(square,  help_size);
        FREE_BIG(big_help,  help_size);
        raise_error(MEMORY_ERROR);
      } else {
        square->size = 1;
        square->bigdigits[0] = base;
        if (exponent & 1) {
          result->size = square->size;
          memcpy(result->bigdigits, square->bigdigits,
              (size_t) square->size * sizeof(bigDigitType));
        } else {
          result->size = 1;
          result->bigdigits[0] = 1;
        } /* if */
        exponent >>= 1;
        while (exponent != 0) {
          square = uBigSquare(square, &big_help);
          if (exponent & 1) {
            result = uBigMultIntoHelp(result, square, &big_help);
          } /* if */
          exponent >>= 1;
        } /* while */
        memset(&result->bigdigits[result->size], 0,
            (size_t) (help_size - result->size) * sizeof(bigDigitType));
        result->size = help_size;
        result = normalize(result);
        FREE_BIG(square, help_size);
        FREE_BIG(big_help, help_size);
      } /* if */
    } /* if */
    /* printf("bigIPowN() => result->size=%lu\n", result->size); */
    return result;
  } /* bigIPowN */



/**
 *  Computes base to the power of exponent for signed big integers.
 *  It is assumed that the exponent is >= 1.
 *  The function recognizes the special case of base with a value
 *  of a power of two. In this case the function bigLShiftOne is
 *  used.
 */
static bigIntType bigIPow1 (bigDigitType base, intType exponent)

  {
    boolType negative;
    unsigned int bit_size;
    bigIntType result;

  /* bigIPow1 */
    /* printf("bigIPow1(%ld, %lu)\n", base, exponent); */
    if (base == 0) {
      if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->size = 1;
        result->bigdigits[0] = 0;
      } /* if */
    } else {
      if (IS_NEGATIVE(base)) {
        /* The unsigned value is negated to avoid a signed integer */
        /* overflow when the smallest signed integer is negated.   */
        base = -base;
        negative = (boolType) (exponent & 1);
      } else {
        negative = FALSE;
      } /* if */
      bit_size = (unsigned int) (digitMostSignificantBit(base) + 1);
      if (base == (bigDigitType) (1 << (bit_size - 1))) {
        result = bigLShiftOne((intType) (bit_size - 1) * exponent);
        if (negative) {
          negate_positive_big(result);
          result = normalize(result);
        } /* if */
      } else {
        result = bigIPowN(base, exponent, bit_size);
        if (negative) {
          negate_positive_big(result);
        } /* if */
        result = normalize(result);
      } /* if */
    } /* if */
    /* printf("bigIPow1 => result->size=%lu\n", result->size); */
    return result;
  } /* bigIPow1 */



static int uBigIsNot0 (const const_bigIntType big)

  {
    memSizeType pos;

  /* uBigIsNot0 */
    pos = 0;
    do {
      if (big->bigdigits[pos] != 0) {
        return TRUE;
      } /* if */
      pos++;
    } while (pos < big->size);
    return FALSE;
  } /* uBigIsNot0 */



/**
 *  Compute the absolute value of a 'bigInteger' number.
 *  @return the absolute value.
 */
bigIntType bigAbs (const const_bigIntType big1)

  {
    memSizeType pos;
    doubleBigDigitType carry = 1;
    memSizeType result_size;
    bigIntType resized_result;
    bigIntType result;

  /* bigAbs */
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, big1->size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = big1->size;
      if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        pos = 0;
        do {
          carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
          result->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          pos++;
        } while (pos < big1->size);
        if (IS_NEGATIVE(result->bigdigits[pos - 1])) {
          result_size = result->size + 1;
          REALLOC_BIG_CHECK_SIZE(resized_result, result, big1->size, result_size);
          if (unlikely(resized_result == NULL)) {
            FREE_BIG(result, big1->size);
            raise_error(MEMORY_ERROR);
            return NULL;
          } else {
            result = resized_result;
            COUNT3_BIG(big1->size, result->size);
            result->size = result_size;
            result->bigdigits[big1->size] = 0;
          } /* if */
        } /* if */
      } else {
        memcpy(result->bigdigits, big1->bigdigits,
            (size_t) big1->size * sizeof(bigDigitType));
      } /* if */
    } /* if */
    return result;
  } /* bigAbs */



/**
 *  Add two 'bigInteger' numbers.
 *  The function sorts the two values by size. This way there is a
 *  loop up to the shorter size and a second loop up to the longer size.
 *  @return the sum of the two numbers.
 */
bigIntType bigAdd (const_bigIntType summand1, const_bigIntType summand2)

  {
    const_bigIntType help_big;
    memSizeType pos;
    doubleBigDigitType carry = 0;
    doubleBigDigitType summand2_sign;
    bigIntType sum;

  /* bigAdd */
    if (summand2->size > summand1->size) {
      help_big = summand1;
      summand1 = summand2;
      summand2 = help_big;
    } /* if */
    if (unlikely(!ALLOC_BIG_CHECK_SIZE(sum, summand1->size + 1))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      pos = 0;
      do {
        carry += (doubleBigDigitType) summand1->bigdigits[pos] + summand2->bigdigits[pos];
        sum->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < summand2->size);
      summand2_sign = IS_NEGATIVE(summand2->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
      for (; pos < summand1->size; pos++) {
        carry += summand1->bigdigits[pos] + summand2_sign;
        sum->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
      } /* for */
      if (IS_NEGATIVE(summand1->bigdigits[pos - 1])) {
        summand2_sign--;
      } /* if */
      sum->bigdigits[pos] = (bigDigitType) ((carry + summand2_sign) & BIGDIGIT_MASK);
      sum->size = pos + 1;
      sum = normalize(sum);
      return sum;
    } /* if */
  } /* bigAdd */



/**
 *  Add two 'bigInteger' numbers.
 *  Summand1 is assumed to be a temporary value which is reused.
 *  @return the sum of the two numbers in 'summand1'.
 */
bigIntType bigAddTemp (bigIntType summand1, const const_bigIntType summand2)

  { /* bigAddTemp */
    bigGrow(&summand1, summand2);
    return summand1;
  } /* bigAddTemp */



bigIntType bigAnd (const_bigIntType big1, const_bigIntType big2)

  {
    const_bigIntType help_big;
    memSizeType pos;
    bigDigitType big2_sign;
    bigIntType result;

  /* bigAnd */
    if (big2->size > big1->size) {
      help_big = big1;
      big1 = big2;
      big2 = help_big;
    } /* if */
    if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, big1->size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      pos = 0;
      do {
        result->bigdigits[pos] = big1->bigdigits[pos] & big2->bigdigits[pos];
        pos++;
      } while (pos < big2->size);
      big2_sign = IS_NEGATIVE(big2->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
      for (; pos < big1->size; pos++) {
        result->bigdigits[pos] = big1->bigdigits[pos] & big2_sign;
      } /* for */
      result->size = pos;
      result = normalize(result);
      return result;
    } /* if */
  } /* bigAnd */



#ifdef OUT_OF_ORDER
bigIntType bigBinom (bigIntType n_number, bigIntType k_number)

  {
    bigIntType number;
    bigIntType result;

  /* bigBinom */
    if (2 * k_number > n_number) {
      k_number = n_number - k_number;
    } /* if */
    if (k_number < 0) {
      result = 0;
    } else if (k_number == 0) {
      result = 1;
    } else {
      result = n_number;
      for (number = 2; number <= k_number; number++) {
        result *= (n_number - number + 1);
        result /= number;
      } /* for */
    } /* if */
    return (intType) result;
  } /* bigBinom */
#endif



/**
 *  Number of bits in the minimal two's-complement representation.
 *  The high bits equivalent to the sign bit are not part of the
 *  minimal two's-complement representation.
 *  @return the number of bits.
 *  @exception RANGE_ERROR The result does not fit into an integer.
 */
intType bigBitLength (const const_bigIntType big1)

  {
    intType result;

  /* bigBitLength */
    if (unlikely(big1->size >= MAX_MEM_INDEX >> BIGDIGIT_LOG2_SIZE)) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = (intType) ((big1->size - 1) << BIGDIGIT_LOG2_SIZE);
      if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        result += digitMostSignificantBit(~big1->bigdigits[big1->size - 1]) + 1;
      } else {
        result += digitMostSignificantBit(big1->bigdigits[big1->size - 1]) + 1;
      } /* if */
    } /* if */
    return result;
  } /* bigBitLength */



/**
 *  Compare two 'bigInteger' numbers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType bigCmp (const const_bigIntType big1, const const_bigIntType big2)

  {
    boolType big1_negative;
    boolType big2_negative;
    memSizeType pos;

  /* bigCmp */
    big1_negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    big2_negative = IS_NEGATIVE(big2->bigdigits[big2->size - 1]);
    if (big1_negative != big2_negative) {
      return big1_negative ? -1 : 1;
    } else if (big1->size != big2->size) {
      return (big1->size < big2->size) != big1_negative ? -1 : 1;
    } else {
      pos = big1->size;
      do {
        pos--;
        if (big1->bigdigits[pos] != big2->bigdigits[pos]) {
          return big1->bigdigits[pos] < big2->bigdigits[pos] ? -1 : 1;
        } /* if */
      } while (pos > 0);
      return 0;
    } /* if */
  } /* bigCmp */



/**
 *  Reinterpret the generic parameters as bigIntType and call bigCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(bigIntType).
 */
intType bigCmpGeneric (const genericType value1, const genericType value2)

  { /* bigCmpGeneric */
    return bigCmp(((const_rtlObjectType *) &value1)->value.bigIntValue,
                  ((const_rtlObjectType *) &value2)->value.bigIntValue);
  } /* bigCmpGeneric */



/**
 *  Compare 'big1' with the bigdigit 'number'.
 *  The range of 'number' is restricted and it is the job of the
 *  compiler to assure that 'number' is within the allowed range.
 *  @param number Number that must be in the range of
 *         signedBigDigitType.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType bigCmpSignedDigit (const const_bigIntType big1, intType number)

  {
    intType result;

  /* bigCmpSignedDigit */
    if (number < 0) {
      if (!IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        result = 1;
      } else if (big1->size != 1) {
        result = -1;
      } else if (big1->bigdigits[0] != (bigDigitType) number) {
        if (big1->bigdigits[0] < (bigDigitType) number) {
          result = -1;
        } else {
          result = 1;
        } /* if */
      } else {
        result = 0;
      } /* if */
    } else {
      if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        result = -1;
      } else if (big1->size != 1) {
        result = 1;
      } else if (big1->bigdigits[0] != (bigDigitType) number) {
        if (big1->bigdigits[0] < (bigDigitType) number) {
          result = -1;
        } else {
          result = 1;
        } /* if */
      } else {
        result = 0;
      } /* if */
    } /* if */
    return result;
  } /* bigCmpSignedDigit */



void bigCpy (bigIntType *const big_to, const const_bigIntType big_from)

  {
    memSizeType new_size;
    bigIntType big_dest;

  /* bigCpy */
    big_dest = *big_to;
    new_size = big_from->size;
    if (big_dest->size != new_size) {
      if (unlikely(!ALLOC_BIG_SIZE_OK(big_dest, new_size))) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        FREE_BIG(*big_to, (*big_to)->size);
        big_dest->size = new_size;
        *big_to = big_dest;
      } /* if */
    } /* if */
    /* It is possible that *big_to == big_from holds. The */
    /* behavior of memcpy() is undefined when source and  */
    /* destination areas overlap (or are identical).      */
    /* Therefore memmove() is used instead of memcpy().   */
    memmove(big_dest->bigdigits, big_from->bigdigits,
        (size_t) new_size * sizeof(bigDigitType));
  } /* bigCpy */



void bigCpyGeneric (genericType *const dest, const genericType source)

  { /* bigCpyGeneric */
    bigCpy(&((rtlObjectType *) dest)->value.bigIntValue,
           ((const_rtlObjectType *) &source)->value.bigIntValue);
  } /* bigCpyGeneric */



bigIntType bigCreate (const const_bigIntType big_from)

  {
    memSizeType new_size;
    bigIntType result;

  /* bigCreate */
    new_size = big_from->size;
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, new_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = new_size;
      memcpy(result->bigdigits, big_from->bigdigits,
          (size_t) new_size * sizeof(bigDigitType));
    } /* if */
    return result;
  } /* bigCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(bigIntType).
 */
genericType bigCreateGeneric (const genericType from_value)

  {
    rtlObjectType result;

  /* bigCreateGeneric */
    result.value.bigIntValue =
        bigCreate(((const_rtlObjectType *) &from_value)->value.bigIntValue);
    return result.value.genericValue;
  } /* bigCreateGeneric */



/**
 *  Decrement a 'bigInteger' variable.
 *  Decrements *big_variable by 1. The operation is done in
 *  place and *big_variable is only enlarged when necessary.
 *  In case the enlarging fails the old content of *big_variable
 *  is restored and the exception MEMORY_ERROR is raised.
 *  This ensures that bigDecr works as a transaction.
 */
void bigDecr (bigIntType *const big_variable)

  {
    bigIntType big1;
    memSizeType pos;
    boolType negative;
    bigIntType resized_big1;

  /* bigDecr */
    big1 = *big_variable;
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    pos = 0;
    if (big1->bigdigits[pos] == 0) {
      if (big1->size == 1) {
        big1->bigdigits[pos] = BIGDIGIT_MASK;
        pos++;
      } else {
        do {
          big1->bigdigits[pos] = BIGDIGIT_MASK;
          pos++;
        } while (big1->bigdigits[pos] == 0);
        /* memset(big1->bigdigits, 0xFF, pos * sizeof(bigDigitType)); */
      } /* if */
    } /* if */
    if (pos < big1->size) {
      big1->bigdigits[pos]--;
    } /* if */
    pos = big1->size;
    if (!IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      if (negative) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, pos, pos + 1);
        if (unlikely(resized_big1 == NULL)) {
          /* This error situation is very unlikely, but we need to */
          /* make sure that 'big_variable' contains a legal value. */
          /* We UNDO the change done for 'big_variable' by setting */
          /* it to the old value: The highest bit is set to 1 and  */
          /* the other bits are set to 0. Note that only values    */
          /* with this pattern need an additional digit when they  */
          /* are decremented.                                      */
          pos--;
          big1->bigdigits[pos] = BIGDIGIT_SIGN;
          while (pos != 0) {
            pos--;
            big1->bigdigits[pos] = 0;
          } /* while */
          raise_error(MEMORY_ERROR);
        } else {
          big1 = resized_big1;
          COUNT3_BIG(pos, pos + 1);
          big1->size++;
          big1->bigdigits[pos] = BIGDIGIT_MASK;
          *big_variable = big1;
        } /* if */
      } else if (big1->bigdigits[pos - 1] == 0 &&
          pos >= 2 && !IS_NEGATIVE(big1->bigdigits[pos - 2])) {
        REALLOC_BIG_SIZE_OK(resized_big1, big1, pos, pos - 1);
        /* Avoid a MEMORY_ERROR in the strange case     */
        /* when a 'realloc' which shrinks memory fails. */
        if (likely(resized_big1 != NULL)) {
          big1 = resized_big1;
          *big_variable = big1;
        } /* if */
        COUNT3_BIG(pos, pos - 1);
        big1->size--;
      } /* if */
    } /* if */
  } /* bigDecr */



void bigDestr (const bigIntType old_bigint)

  { /* bigDestr */
    if (old_bigint != NULL) {
      FREE_BIG(old_bigint, old_bigint->size);
    } /* if */
  } /* bigDestr */



/**
 *  Generic Destr function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(bigIntType).
 */
void bigDestrGeneric (const genericType old_value)

  { /* bigDestrGeneric */
    bigDestr(((const_rtlObjectType *) &old_value)->value.bigIntValue);
  } /* bigDestrGeneric */



/**
 *  Integer division truncated towards zero.
 *  The remainder of this division is computed with bigRem.
 *  The memory for the result is requested and the normalized
 *  result is returned. When divisor has just one digit or when
 *  dividend has less digits than divisor the bigDiv1() or
 *  bigDivSizeLess() functions are called. In the general case
 *  the absolute values of dividend and divisor are taken. Then
 *  dividend is extended by one leading zero digit. After that
 *  dividend and divisor are shifted to the left such that the
 *  most significant bit of divisor is set. This fulfills the
 *  preconditions for calling uBigDiv() which does the main
 *  work of the division.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
bigIntType bigDiv (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    boolType negative = FALSE;
    bigIntType dividend_help;
    bigIntType divisor_help;
    unsigned int shift;
    bigIntType quotient;

  /* bigDiv */
    if (divisor->size == 1) {
      quotient = bigDiv1(dividend, divisor->bigdigits[0]);
      return quotient;
    } else if (dividend->size < divisor->size) {
      quotient = bigDivSizeLess(dividend, divisor);
      return quotient;
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(dividend_help, dividend->size + 2))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          negative = TRUE;
          positive_copy_of_negative_big(dividend_help, dividend);
        } else {
          dividend_help->size = dividend->size;
          memcpy(dividend_help->bigdigits, dividend->bigdigits,
              (size_t) dividend->size * sizeof(bigDigitType));
        } /* if */
        dividend_help->bigdigits[dividend_help->size] = 0;
        dividend_help->size++;
      } /* if */
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(divisor_help, divisor->size + 1))) {
        FREE_BIG(dividend_help,  dividend->size + 2);
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(divisor->bigdigits[divisor->size - 1])) {
          negative = !negative;
          positive_copy_of_negative_big(divisor_help, divisor);
        } else {
          divisor_help->size = divisor->size;
          memcpy(divisor_help->bigdigits, divisor->bigdigits,
              (size_t) divisor->size * sizeof(bigDigitType));
        } /* if */
      } /* if */
      if (unlikely(!ALLOC_BIG_SIZE_OK(quotient, dividend_help->size - divisor_help->size + 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        quotient->size = dividend_help->size - divisor_help->size + 1;
        quotient->bigdigits[quotient->size - 1] = 0;
        shift = (unsigned int) (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
        if (shift == 0) {
          /* The most significant digit of divisor_help is 0. Just ignore it */
          dividend_help->size--;
          divisor_help->size--;
          if (divisor_help->size == 1) {
            uBigDiv1(dividend_help, divisor_help->bigdigits[0], quotient);
          } else {
            uBigDiv(dividend_help, divisor_help, quotient);
          } /* if */
        } else {
          shift = BIGDIGIT_SIZE - shift;
          uBigLShift(dividend_help, shift);
          uBigLShift(divisor_help, shift);
          uBigDiv(dividend_help, divisor_help, quotient);
        } /* if */
        if (negative) {
          negate_positive_big(quotient);
        } /* if */
        quotient = normalize(quotient);
      } /* if */
      FREE_BIG(dividend_help, dividend->size + 2);
      FREE_BIG(divisor_help, divisor->size + 1);
      return quotient;
    } /* if */
  } /* bigDiv */



/**
 *  Check if two 'bigInteger' numbers are equal.
 *  @return TRUE if both numbers are equal,
 *          FALSE otherwise.
 */
boolType bigEq (const const_bigIntType big1, const const_bigIntType big2)

  { /* bigEq */
    if (big1->size == big2->size &&
      memcmp(big1->bigdigits, big2->bigdigits,
          (size_t) big1->size * sizeof(bigDigitType)) == 0) {
      return TRUE;
    } else {
      return FALSE;
    } /* if */
  } /* bigEq */



/**
 *  Check if 'big1' is equal to the bigdigit 'number'.
 *  The range of 'number' is restricted and it is the job of the
 *  compiler to assure that 'number' is within the allowed range.
 *  @param number Number that must be in the range of
 *         signedBigDigitType.
 *  @return TRUE if 'big1' and 'number' are equal,
 *          FALSE otherwise.
 */
boolType bigEqSignedDigit (const const_bigIntType big1, intType number)

  { /* bigEqSignedDigit */
    return big1->size == 1 && big1->bigdigits[0] == (bigDigitType) number;
  } /* bigEqSignedDigit */



bigIntType bigFromByteBufferBe (const memSizeType size,
    const const_ustriType buffer, const boolType isSigned)

  {
    memSizeType byteIndex;
    memSizeType pos;
    memSizeType num_bigdigits;
    memSizeType result_size;
    ucharType buffer2[BIGDIGIT_SIZE >> 3];
    bigIntType result;

  /* bigFromByteBufferBe */
    /* printf("bigFromByteBufferBe(%lu, *, %d)\n", size, isSigned); */
    if (size == 0) {
      num_bigdigits = 0;
      result_size = 1;
    } else {
      num_bigdigits = (size + (BIGDIGIT_SIZE >> 3) - 1) / (BIGDIGIT_SIZE >> 3);
      result_size = num_bigdigits;
      if (!isSigned && size % (BIGDIGIT_SIZE >> 3) == 0 && buffer[0] >= 128) {
        /* The number is unsigned, but highest bit is one: */
        /* A leading zero bigdigit must be added.          */
        result_size++;
      } /* if */
    } /* if */
    if (unlikely(!ALLOC_BIG(result, result_size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = result_size;
      if (num_bigdigits == 0) {
        result->bigdigits[0] = (bigDigitType) 0;
      } else {
        byteIndex = size;
        for (pos = 0; pos < num_bigdigits - 1; pos++) {
#if BIGDIGIT_SIZE == 8
          result->bigdigits[pos] =  (bigDigitType) buffer[byteIndex - 1];
          byteIndex--;
#elif BIGDIGIT_SIZE == 16
          result->bigdigits[pos] = ((bigDigitType) buffer[byteIndex - 2]) <<  8 |
                                    (bigDigitType) buffer[byteIndex - 1];
          byteIndex -= 2;
#elif BIGDIGIT_SIZE == 32
          result->bigdigits[pos] = ((bigDigitType) buffer[byteIndex - 4]) << 24 |
                                   ((bigDigitType) buffer[byteIndex - 3]) << 16 |
                                   ((bigDigitType) buffer[byteIndex - 2]) <<  8 |
                                    (bigDigitType) buffer[byteIndex - 1];
          byteIndex -= 4;
#endif
        } /* for */
        memcpy(&buffer2[(BIGDIGIT_SIZE >> 3) - byteIndex], buffer, byteIndex);
        if (isSigned && buffer[0] >= 128) {
          memset(buffer2, 0xFF, (BIGDIGIT_SIZE >> 3) - byteIndex);
        } else {
          memset(buffer2, 0, (BIGDIGIT_SIZE >> 3) - byteIndex);
        } /* if */
#if BIGDIGIT_SIZE == 8
        result->bigdigits[pos] =  (bigDigitType) buffer2[0];
#elif BIGDIGIT_SIZE == 16
        result->bigdigits[pos] = ((bigDigitType) buffer2[0]) <<  8 |
                                  (bigDigitType) buffer2[1];
#elif BIGDIGIT_SIZE == 32
        result->bigdigits[pos] = ((bigDigitType) buffer2[0]) << 24 |
                                 ((bigDigitType) buffer2[1]) << 16 |
                                 ((bigDigitType) buffer2[2]) <<  8 |
                                  (bigDigitType) buffer2[3];
#endif
        if (num_bigdigits != result_size) {
          /* The number is unsigned, but highest bit is one: */
          /* A leading zero bigdigit must be added.          */
          result->bigdigits[num_bigdigits] = (bigDigitType) 0;
        } /* if */
      } /* if */
    } /* if */
    result = normalize(result);
    /* printf("bigFromByteBufferBe() -> %s\n", bigHexCStri(result)); */
    return result;
  } /* bigFromByteBufferBe */



bigIntType bigFromByteBufferLe (const memSizeType size,
    const const_ustriType buffer, const boolType isSigned)

  {
    memSizeType byteIndex;
    memSizeType pos;
    memSizeType num_bigdigits;
    memSizeType result_size;
    ucharType buffer2[BIGDIGIT_SIZE >> 3];
    bigIntType result;

  /* bigFromByteBufferLe */
    /* printf("bigFromByteBufferLe(%lu, *, %d)\n", size, isSigned); */
    if (size == 0) {
      num_bigdigits = 0;
      result_size = 1;
    } else {
      num_bigdigits = (size + (BIGDIGIT_SIZE >> 3) - 1) / (BIGDIGIT_SIZE >> 3);
      result_size = num_bigdigits;
      if (!isSigned && buffer[size - 1] >= 128) {
        /* The number is unsigned, but highest bit is one: */
        /* A leading zero bigdigit must be added.          */
        result_size++;
      } /* if */
    } /* if */
    if (unlikely(!ALLOC_BIG(result, result_size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = result_size;
      if (num_bigdigits == 0) {
        result->bigdigits[0] = (bigDigitType) 0;
      } else {
        byteIndex = 0;
        for (pos = 0; pos < num_bigdigits - 1; pos++) {
#if BIGDIGIT_SIZE == 8
          result->bigdigits[pos] =  (bigDigitType) buffer[byteIndex];
          byteIndex++;
#elif BIGDIGIT_SIZE == 16
          result->bigdigits[pos] = ((bigDigitType) buffer[byteIndex + 1]) <<  8 |
                                    (bigDigitType) buffer[byteIndex];
          byteIndex += 2;
#elif BIGDIGIT_SIZE == 32
          result->bigdigits[pos] = ((bigDigitType) buffer[byteIndex + 3]) << 24 |
                                   ((bigDigitType) buffer[byteIndex + 2]) << 16 |
                                   ((bigDigitType) buffer[byteIndex + 1]) <<  8 |
                                    (bigDigitType) buffer[byteIndex];
          byteIndex += 4;
#endif
        } /* for */
        memcpy(buffer2, &buffer[byteIndex], size - byteIndex);
        if (isSigned && buffer[size - 1] >= 128) {
          memset(&buffer2[size - byteIndex], 0xFF, (BIGDIGIT_SIZE >> 3) - (size - byteIndex));
        } else {
          memset(&buffer2[size - byteIndex], 0, (BIGDIGIT_SIZE >> 3) - (size - byteIndex));
        } /* if */
#if BIGDIGIT_SIZE == 8
        result->bigdigits[pos] =  (bigDigitType) buffer2[0];
#elif BIGDIGIT_SIZE == 16
        result->bigdigits[pos] = ((bigDigitType) buffer2[1]) <<  8 |
                                  (bigDigitType) buffer2[0];
#elif BIGDIGIT_SIZE == 32
        result->bigdigits[pos] = ((bigDigitType) buffer2[3]) << 24 |
                                 ((bigDigitType) buffer2[2]) << 16 |
                                 ((bigDigitType) buffer2[1]) <<  8 |
                                  (bigDigitType) buffer2[0];
#endif
        if (num_bigdigits != result_size) {
          /* The number is unsigned, but highest bit is one: */
          /* A leading zero bigdigit must be added.          */
          result->bigdigits[num_bigdigits] = (bigDigitType) 0;
        } /* if */
      } /* if */
    } /* if */
    result = normalize(result);
    /* printf("bigFromByteBufferLe() -> %s\n", bigHexCStri(result)); */
    return result;
  } /* bigFromByteBufferLe */



/**
 *  Convert a bstring (interpreted as big-endian) to a bigInteger.
 *  @param bstri Bstring to be converted. The bytes are interpreted
 *         as binary big-endian representation with a base of 256.
 *  @param isSigned Defines if 'bstri' is interpreted as signed value.
 *         When 'isSigned' is TRUE the twos-complement representation
 *         is used. In this case the result is negative when the most
 *         significant byte (the first byte) has an ordinal >= 128.
 *  @return a bigInteger created from the big-endian bytes.
 */
bigIntType bigFromBStriBe (const const_bstriType bstri, const boolType isSigned)

  { /* bigFromBStriBe */
    return bigFromByteBufferBe(bstri->size, bstri->mem, isSigned);
  } /* bigFromBStriBe */



/**
 *  Convert a bstring (interpreted as little-endian) to a bigInteger.
 *  @param bstri Bstring to be converted. The bytes are interpreted
 *         as binary little-endian representation with a base of 256.
 *  @param isSigned Defines if 'bstri' is interpreted as signed value.
 *         When 'isSigned' is TRUE the twos-complement representation
 *         is used. In this case the result is negative when the most
 *         significant byte (the last byte) has an ordinal >= 128.
 *  @return a bigInteger created from the little-endian bytes.
 */
bigIntType bigFromBStriLe (const const_bstriType bstri, const boolType isSigned)

  { /* bigFromBStriLe */
    return bigFromByteBufferLe(bstri->size, bstri->mem, isSigned);
  } /* bigFromBStriLe */



/**
 *  Convert an int32Type number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigFromInt32 (int32Type number)

  {
    memSizeType result_size;
    bigIntType result;

  /* bigFromInt32 */
#if BIGDIGIT_SIZE < 32
    result_size = sizeof(int32Type) / (BIGDIGIT_SIZE >> 3);
#else
    result_size = 1;
#endif
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
#if BIGDIGIT_SIZE <= 32
      result->bigdigits[0] = (bigDigitType) (((uint32Type) number) & BIGDIGIT_MASK);
#if BIGDIGIT_SIZE < 32
      {
        memSizeType pos;

        for (pos = 1; pos < result_size; pos++) {
          number >>= BIGDIGIT_SIZE;
          result->bigdigits[pos] = (bigDigitType) (((uint32Type) number) & BIGDIGIT_MASK);
        } /* for */
      }
#endif
#else
      if (number < 0) {
        result->bigdigits[0] = (bigDigitType) ((uint32Type) number) | (BIGDIGIT_MASK ^ 0xFFFFFFFF);
      } else {
        result->bigdigits[0] = (bigDigitType) ((uint32Type) number);
      } /* if */
#endif
#if BIGDIGIT_SIZE < 32
      result = normalize(result);
#endif
    } /* if */
    return result;
  } /* bigFromInt32 */



#ifdef INT64TYPE
/**
 *  Convert an int64Type number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigFromInt64 (int64Type number)

  {
    memSizeType pos;
    memSizeType result_size;
    bigIntType result;

  /* bigFromInt64 */
    result_size = sizeof(int64Type) / (BIGDIGIT_SIZE >> 3);
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = result_size;
      for (pos = 0; pos < result_size; pos++) {
        result->bigdigits[pos] = (bigDigitType) (number & BIGDIGIT_MASK);
        number >>= BIGDIGIT_SIZE;
      } /* for */
      result = normalize(result);
      return result;
    } /* if */
  } /* bigFromInt64 */
#endif



/**
 *  Convert an uint32Type number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigFromUInt32 (uint32Type number)

  {
    memSizeType result_size;
    bigIntType result;

  /* bigFromUInt32 */
#if BIGDIGIT_SIZE <= 32
    result_size = sizeof(uint32Type) / (BIGDIGIT_SIZE >> 3) + 1;
#else
    result_size = 1;
#endif
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
      result->bigdigits[0] = (bigDigitType) (number & BIGDIGIT_MASK);
#if BIGDIGIT_SIZE < 32
      {
        memSizeType pos;

        for (pos = 1; pos < result_size - 1; pos++) {
          number >>= BIGDIGIT_SIZE;
          result->bigdigits[pos] = (bigDigitType) (number & BIGDIGIT_MASK);
        } /* for */
      }
#endif
      result->bigdigits[result_size - 1] = (bigDigitType) 0;
      result = normalize(result);
    } /* if */
    return result;
  } /* bigFromUInt32 */



#ifdef INT64TYPE
/**
 *  Convert an uint64Type number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigFromUInt64 (uint64Type number)

  {
    memSizeType pos;
    memSizeType result_size;
    bigIntType result;

  /* bigFromUInt64 */
    result_size = sizeof(uint64Type) / (BIGDIGIT_SIZE >> 3) + 1;
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = result_size;
      for (pos = 0; pos < result_size - 1; pos++) {
        result->bigdigits[pos] = (bigDigitType) (number & BIGDIGIT_MASK);
        number >>= BIGDIGIT_SIZE;
      } /* for */
      result->bigdigits[result_size - 1] = (bigDigitType) 0;
      result = normalize(result);
      return result;
    } /* if */
  } /* bigFromUInt64 */
#endif



/**
 *  Compute the greatest common divisor of two 'bigInteger' numbers.
 *  @return the greatest common divisor of the two numbers.
 */
bigIntType bigGcd (const const_bigIntType big1,
    const const_bigIntType big2)

  {
    bigIntType big1_help;
    bigIntType big2_help;
    intType lowestSetBitA;
    intType shift;
    bigIntType help_big;
    bigIntType result;

  /* bigGcd */
    if (big1->size == 1 && big1->bigdigits[0] == 0) {
      result = bigCreate(big2);
    } else if (big2->size == 1 && big2->bigdigits[0] == 0) {
      result = bigCreate(big1);
    } else {
      if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        big1_help = bigNegate(big1);
      } else {
        big1_help = bigCreate(big1);
      } /* if */
      if (IS_NEGATIVE(big2->bigdigits[big2->size - 1])) {
        big2_help = bigNegate(big2);
      } else {
        big2_help = bigCreate(big2);
      } /* if */
      if ((big1_help->size > big2_help->size &&
          big1_help->size - big2_help->size > 10) ||
          (big1_help->size < big2_help->size &&
          big2_help->size - big1_help->size > 10)) {
        while (big1_help->size != 1 || big1_help->bigdigits[0] != 0) {
          help_big = bigRem(big2_help, big1_help);
          bigDestr(big2_help);
          big2_help = big1_help;
          big1_help = help_big;
        } /* while */
        result = big2_help;
        bigDestr(big1_help);
        return result;
      } else {
        lowestSetBitA = bigLowestSetBit(big1_help);
        shift = bigLowestSetBit(big2_help);
        if (lowestSetBitA < shift) {
          shift = lowestSetBitA;
        } /* if */
        bigRShiftAssign(&big1_help, lowestSetBitA);
        do {
          bigRShiftAssign(&big2_help, bigLowestSetBit(big2_help));
          if (bigCmp(big1_help, big2_help) < 0) {
            bigShrink(&(big2_help), big1_help);
          } else {
            help_big = bigSbtr(big1_help, big2_help);
            bigDestr(big1_help);
            big1_help = big2_help;
            big2_help = help_big;
          } /* if */
        } while (big2_help->size != 1 || big2_help->bigdigits[0] != 0);
        bigLShiftAssign(&big1_help, shift);
        result = big1_help;
        bigDestr(big2_help);
      } /* if */
    } /* if */
    return result;
  } /* bigGcd */



/**
 *  Increment a 'bigInteger' variable by a delta.
 *  Adds delta to *big_variable. The operation is done in
 *  place and *big_variable is only resized when necessary.
 *  When the size of delta is smaller than *big_variable the
 *  algorithm tries to save computations. Therefore there are
 *  checks for carry == 0 and carry != 0.
 *  In case the resizing fails the content of *big_variable
 *  is freed and *big_variable is set to NULL.
 */
void bigGrow (bigIntType *const big_variable, const const_bigIntType delta)

  {
    bigIntType big1;
    memSizeType pos;
    memSizeType big1_size;
    doubleBigDigitType carry = 0;
    doubleBigDigitType big1_sign;
    doubleBigDigitType delta_sign;
    bigIntType resized_big1;

  /* bigGrow */
    big1 = *big_variable;
    if (big1->size >= delta->size) {
      big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
      pos = 0;
      do {
        carry += (doubleBigDigitType) big1->bigdigits[pos] + delta->bigdigits[pos];
        big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < delta->size);
      if (IS_NEGATIVE(delta->bigdigits[pos - 1])) {
        for (; carry == 0 && pos < big1->size; pos++) {
          carry = (doubleBigDigitType) big1->bigdigits[pos] + BIGDIGIT_MASK;
          big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        carry += BIGDIGIT_MASK;
      } else {
        for (; carry != 0 && pos < big1->size; pos++) {
          carry += big1->bigdigits[pos];
          big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
      } /* if */
      big1_size = big1->size;
      carry += big1_sign;
      carry &= BIGDIGIT_MASK;
      if ((carry != 0 || IS_NEGATIVE(big1->bigdigits[big1_size - 1])) &&
          (carry != BIGDIGIT_MASK || !IS_NEGATIVE(big1->bigdigits[big1_size - 1]))) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, big1_size, big1_size + 1);
        if (unlikely(resized_big1 == NULL)) {
          FREE_BIG(big1, big1_size);
          *big_variable = NULL;
          raise_error(MEMORY_ERROR);
        } else {
          /* It is possible that big1 == delta holds. Since */
          /* 'delta' is not used after realloc() enlarged   */
          /* 'big1' a correction of delta is not necessary. */
          big1 = resized_big1;
          COUNT3_BIG(big1_size, big1_size + 1);
          big1->size++;
          big1->bigdigits[big1_size] = (bigDigitType) (carry & BIGDIGIT_MASK);
          *big_variable = big1;
        } /* if */
      } else {
        *big_variable = normalize(big1);
      } /* if */
    } else {
      REALLOC_BIG_CHECK_SIZE(resized_big1, big1, big1->size, delta->size + 1);
      if (unlikely(resized_big1 == NULL)) {
        FREE_BIG(big1, big1->size);
        *big_variable = NULL;
        raise_error(MEMORY_ERROR);
      } else {
        big1 = resized_big1;
        COUNT3_BIG(big1->size, delta->size + 1);
        big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
        pos = 0;
        do {
          carry += (doubleBigDigitType) big1->bigdigits[pos] + delta->bigdigits[pos];
          big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          pos++;
        } while (pos < big1->size);
        delta_sign = IS_NEGATIVE(delta->bigdigits[delta->size - 1]) ? BIGDIGIT_MASK : 0;
        for (; pos < delta->size; pos++) {
          carry += big1_sign + delta->bigdigits[pos];
          big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        carry += big1_sign + delta_sign;
        big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        big1->size = pos + 1;
        *big_variable = normalize(big1);
      } /* if */
    } /* if */
  } /* bigGrow */



/**
 *  Compute the hash value of a 'bigInteger' number.
 *  @return the hash value.
 */
intType bigHashCode (const const_bigIntType big1)

  {
    intType result;

  /* bigHashCode */
    result = (intType) (big1->bigdigits[0] << 5 ^ big1->size << 3 ^ big1->bigdigits[big1->size - 1]);
    return result;
  } /* bigHashCode */



/**
 *  Increment a 'bigInteger' variable.
 *  Increments *big_variable by 1. The operation is done in
 *  place and *big_variable is only enlarged when necessary.
 *  In case the enlarging fails the old content of *big_variable
 *  is restored and the exception MEMORY_ERROR is raised.
 *  This ensures that bigIncr works as a transaction.
 */
void bigIncr (bigIntType *const big_variable)

  {
    bigIntType big1;
    memSizeType pos;
    boolType negative;
    bigIntType resized_big1;

  /* bigIncr */
    big1 = *big_variable;
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    pos = 0;
    if (big1->bigdigits[pos] == BIGDIGIT_MASK) {
      if (big1->size == 1) {
        big1->bigdigits[pos] = 0;
        pos++;
      } else {
        do {
          big1->bigdigits[pos] = 0;
          pos++;
        } while (big1->bigdigits[pos] == BIGDIGIT_MASK);
        /* memset(big1->bigdigits, 0, pos * sizeof(bigDigitType)); */
      } /* if */
    } /* if */
    if (pos < big1->size) {
      big1->bigdigits[pos]++;
    } /* if */
    pos = big1->size;
    if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      if (!negative) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, pos, pos + 1);
        if (unlikely(resized_big1 == NULL)) {
          /* This error situation is very unlikely, but we need to */
          /* make sure that 'big_variable' contains a legal value. */
          /* We UNDO the change done for 'big_variable' by setting */
          /* it to the old value: The highest bit is set to 0 and  */
          /* the other bits are set to 1. Note that only values    */
          /* with this pattern need an additional digit when they  */
          /* are incremented.                                      */
          pos--;
          big1->bigdigits[pos] = BIGDIGIT_MASK ^ BIGDIGIT_SIGN;
          while (pos != 0) {
            pos--;
            big1->bigdigits[pos] = BIGDIGIT_MASK;
          } /* while */
          raise_error(MEMORY_ERROR);
        } else {
          big1 = resized_big1;
          COUNT3_BIG(pos, pos + 1);
          big1->size++;
          big1->bigdigits[pos] = 0;
          *big_variable = big1;
        } /* if */
      } else if (big1->bigdigits[pos - 1] == BIGDIGIT_MASK &&
          pos >= 2 && IS_NEGATIVE(big1->bigdigits[pos - 2])) {
        REALLOC_BIG_SIZE_OK(resized_big1, big1, pos, pos - 1);
        /* Avoid a MEMORY_ERROR in the strange case     */
        /* when a 'realloc' which shrinks memory fails. */
        if (likely(resized_big1 != NULL)) {
          big1 = resized_big1;
          *big_variable = big1;
        } /* if */
        COUNT3_BIG(pos, pos - 1);
        big1->size--;
      } /* if */
    } /* if */
  } /* bigIncr */



/**
 *  Compute the exponentiation of a 'bigInteger' base with an integer exponent.
 *  The result variable is set to base or 1 depending on the
 *  rightmost bit of the exponent. After that the base is
 *  squared in a loop and every time the corresponding bit of
 *  the exponent is set the current square is multiplied
 *  with the result variable. This reduces the number of square
 *  operations to ld(exponent).
 *  @return the result of the exponentation.
 *  @exception NUMERIC_ERROR When the exponent is negative.
 */
bigIntType bigIPow (const const_bigIntType base, intType exponent)

  {
    boolType negative = FALSE;
    memSizeType help_size;
    bigIntType square;
    bigIntType big_help;
    bigIntType result;

  /* bigIPow */
    if (exponent <= 1) {
      if (exponent == 0) {
        if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 1;
          result->bigdigits[0] = 1;
        } /* if */
      } else if (exponent == 1) {
        result = bigCreate(base);
      } else {
        raise_error(NUMERIC_ERROR);
        result = NULL;
      } /* if */
    } else if (base->size == 1) {
      result = bigIPow1(base->bigdigits[0], exponent);
    } else if (unlikely((uintType) exponent + 1 > MAX_BIG_LEN / base->size)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      help_size = base->size * (memSizeType) ((uintType) exponent + 1);
      if (unlikely(!ALLOC_BIG_SIZE_OK(square, help_size))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else if (unlikely(!ALLOC_BIG_SIZE_OK(big_help, help_size))) {
        FREE_BIG(square,  help_size);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else if (unlikely(!ALLOC_BIG_SIZE_OK(result, help_size))) {
        FREE_BIG(square,  help_size);
        FREE_BIG(big_help,  help_size);
        raise_error(MEMORY_ERROR);
      } else {
        if (IS_NEGATIVE(base->bigdigits[base->size - 1])) {
          negative = TRUE;
          positive_copy_of_negative_big(square, base);
        } else {
          square->size = base->size;
          memcpy(square->bigdigits, base->bigdigits,
              (size_t) base->size * sizeof(bigDigitType));
        } /* if */
        if (exponent & 1) {
          result->size = square->size;
          memcpy(result->bigdigits, square->bigdigits,
              (size_t) square->size * sizeof(bigDigitType));
        } else {
          negative = FALSE;
          result->size = 1;
          result->bigdigits[0] = 1;
        } /* if */
        exponent >>= 1;
        while (exponent != 0) {
          square = uBigSquare(square, &big_help);
          if (exponent & 1) {
            result = uBigMultIntoHelp(result, square, &big_help);
          } /* if */
          exponent >>= 1;
        } /* while */
        memset(&result->bigdigits[result->size], 0,
            (size_t) (help_size - result->size) * sizeof(bigDigitType));
        result->size = help_size;
        if (negative) {
          negate_positive_big(result);
        } /* if */
        result = normalize(result);
        FREE_BIG(square, help_size);
        FREE_BIG(big_help, help_size);
      } /* if */
    } /* if */
    return result;
  } /* bigIPow */



/**
 *  Compute the truncated base 2 logarithm of a 'bigInteger' number.
 *  The definition of 'log2' is extended by defining log2(0) = -1_.
 *  @return the truncated base 2 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
bigIntType bigLog2 (const const_bigIntType big1)

  {
    memSizeType number;
    memSizeType pos;
    intType bigdigit_log2;
    memSizeType logarithm_size;
    bigIntType logarithm;

  /* bigLog2 */
    if (unlikely(IS_NEGATIVE(big1->bigdigits[big1->size - 1]))) {
      raise_error(NUMERIC_ERROR);
      logarithm = NULL;
    } else {
      /* The logarithm_size is incremented by one to take the space */
      /* needed for the shift by BIGDIGIT_LOG2_SIZE into account.   */
      logarithm_size = sizeof(memSizeType) / (BIGDIGIT_SIZE >> 3) + 1;
      if (unlikely(!ALLOC_BIG_SIZE_OK(logarithm, logarithm_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        logarithm->size = logarithm_size;
        number = big1->size - 1;
        logarithm->bigdigits[0] = (bigDigitType) (number & BIGDIGIT_MASK);
        for (pos = 1; pos < logarithm_size; pos++) {
          /* POINTER_SIZE is equal to sizeof(memSizeType) << 3 */
#if POINTER_SIZE > BIGDIGIT_SIZE
          /* The number does not fit into one bigdigit */
          number >>= BIGDIGIT_SIZE;
          logarithm->bigdigits[pos] = (bigDigitType) (number & BIGDIGIT_MASK);
#else
          /* The number fits into one bigdigit */
          logarithm->bigdigits[pos] = 0;
#endif
        } /* for */
        uBigLShift(logarithm, BIGDIGIT_LOG2_SIZE);
        bigdigit_log2 = digitMostSignificantBit(big1->bigdigits[big1->size - 1]);
        if (bigdigit_log2 == -1) {
          uBigDecr(logarithm);
        } else {
          logarithm->bigdigits[0] |= (bigDigitType) bigdigit_log2;
        } /* if */
        logarithm = normalize(logarithm);
      } /* if */
    } /* if */
    return logarithm;
  } /* bigLog2 */



/**
 *  Create a number from the lower bits of big1.
 *  This corresponds to the modulo when the dividend is a power of two:
 *   bigLowerBits(big1, bits)  corresponds to  big1 mod (2_ ** bits)
 *  @param bits Number of lower bits to select from big1.
 *  @return a number in the range 0 .. pred(2_ ** bits).
 */
bigIntType bigLowerBits (const const_bigIntType big1, const intType bits)

  {
    memSizeType big1_size;
    memSizeType pos;
    int bit_pos;
    boolType add_sign_digit = FALSE;
    bigDigitType digit_mask;
    memSizeType idx;
    memSizeType result_size;
    bigIntType result;

  /* bigLowerBits */
    /* printf("bigLowerBits(%s, %ld)", bigHexCStri(big1), bits); */
    if (unlikely(bits <= 0)) {
      if (bits == 0) {
        if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, 1))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 1;
          result->bigdigits[0] = (bigDigitType) 0;
        } /* if */
      } else {
        raise_error(NUMERIC_ERROR);
        result = NULL;
      } /* if */
    } else {
      big1_size = big1->size;
      pos = (memSizeType) (bits - 1) >> BIGDIGIT_LOG2_SIZE;
      if (pos >= big1_size) {
        if (IS_NEGATIVE(big1->bigdigits[big1_size - 1])) {
          result_size = pos + 1;
          bit_pos = (int) ((bits - 1) & BIGDIGIT_SIZE_MASK);
          digit_mask = BIGDIGIT_MASK >> (BIGDIGIT_SIZE - bit_pos - 1);
          if (bit_pos == BIGDIGIT_SIZE_MASK) {
            add_sign_digit = TRUE;
            result_size++;
          } /* if */
        } else {
          result_size = big1_size;
          digit_mask = BIGDIGIT_MASK;
        } /* if */
      } else {
        result_size = pos + 1;
        bit_pos = (int) ((bits - 1) & BIGDIGIT_SIZE_MASK);
        digit_mask = BIGDIGIT_MASK >> (BIGDIGIT_SIZE - bit_pos - 1);
        if (bit_pos == BIGDIGIT_SIZE_MASK && IS_NEGATIVE(big1->bigdigits[pos])) {
          add_sign_digit = TRUE;
          result_size++;
        } /* if */
      } /* if */
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        idx = result_size - 1;
        if (add_sign_digit) {
          result->bigdigits[idx] = (bigDigitType) 0;
          idx--;
        } /* if */
        if (idx >= big1_size) {
          result->bigdigits[idx] = digit_mask;
          while (idx > big1_size) {
            idx--;
            result->bigdigits[idx] = BIGDIGIT_MASK;
          } /* while */
        } else {
          /* printf("mask = %08x\n", digit_mask); */
          result->bigdigits[idx] = big1->bigdigits[idx] & digit_mask;
        } /* if */
        memcpy(result->bigdigits, big1->bigdigits,
            (size_t) idx * sizeof(bigDigitType));
      } /* if */
    } /* if */
    result = normalize(result);
    /* printf(" --> %s (size=%lu)\n", bigHexCStri(result), result->size); */
    return result;
  } /* bigLowerBits */



/**
 *  Create a number from the lower bits of big1.
 *  Big1 is assumed to be a temporary value which is reused.
 *  This corresponds to the modulo when the dividend is a power of two:
 *   bigLowerBits(big1, bits)  corresponds to  big1 mod (2_ ** bits)
 *  @param bits Number of lower bits to select from big1.
 *  @return a number in the range 0 .. pred(2_ ** bits).
 */
bigIntType bigLowerBitsTemp (const bigIntType big1, const intType bits)

  {
    memSizeType big1_size;
    memSizeType pos;
    int bit_pos;
    boolType add_sign_digit = FALSE;
    bigDigitType digit_mask;
    memSizeType idx;
    memSizeType result_size;
    bigIntType result;

  /* bigLowerBitsTemp */
    /* printf("bigLowerBits(%s, %ld)", bigHexCStri(big1), bits); */
    big1_size = big1->size;
    if (unlikely(bits <= 0)) {
      FREE_BIG(big1, big1_size);
      if (bits == 0) {
        if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, 1))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 1;
          result->bigdigits[0] = (bigDigitType) 0;
        } /* if */
      } else {
        raise_error(NUMERIC_ERROR);
        result = NULL;
      } /* if */
    } else {
      pos = (memSizeType) (bits - 1) >> BIGDIGIT_LOG2_SIZE;
      if (pos >= big1_size) {
        if (IS_NEGATIVE(big1->bigdigits[big1_size - 1])) {
          result_size = pos + 1;
          bit_pos = (int) ((bits - 1) & BIGDIGIT_SIZE_MASK);
          digit_mask = BIGDIGIT_MASK >> (BIGDIGIT_SIZE - bit_pos - 1);
          if (bit_pos == BIGDIGIT_SIZE_MASK) {
            add_sign_digit = TRUE;
            result_size++;
          } /* if */
        } else {
          result_size = big1_size;
          digit_mask = BIGDIGIT_MASK;
        } /* if */
      } else {
        result_size = pos + 1;
        bit_pos = (int) ((bits - 1) & BIGDIGIT_SIZE_MASK);
        digit_mask = BIGDIGIT_MASK >> (BIGDIGIT_SIZE - bit_pos - 1);
        if (bit_pos == BIGDIGIT_SIZE_MASK && IS_NEGATIVE(big1->bigdigits[pos])) {
          add_sign_digit = TRUE;
          result_size++;
        } /* if */
      } /* if */
      if (big1_size != result_size) {
        REALLOC_BIG_CHECK_SIZE(result, big1, big1_size, result_size);
      } else {
        result = big1;
      } /* if */
      if (unlikely(result == NULL)) {
        FREE_BIG(big1, big1_size);
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_BIG(big1_size, result_size);
        result->size = result_size;
        idx = result_size - 1;
        if (add_sign_digit) {
          result->bigdigits[idx] = (bigDigitType) 0;
          idx--;
        } /* if */
        if (idx >= big1_size) {
          result->bigdigits[idx] = digit_mask;
          while (idx > big1_size) {
            idx--;
            result->bigdigits[idx] = BIGDIGIT_MASK;
          } /* while */
        } else {
          /* printf("mask = %08x\n", digit_mask); */
          result->bigdigits[idx] = result->bigdigits[idx] & digit_mask;
        } /* if */
      } /* if */
    } /* if */
    result = normalize(result);
    /* printf(" --> %s (size=%lu)\n", bigHexCStri(result), result->size); */
    return result;
  } /* bigLowerBitsTemp */



/**
 *  Index of the lowest-order one bit.
 *  For A <> 0 this is equal to the number of lowest-order zero bits.
 *  @return the number of lowest-order zero bits or -1 for lowestSetBit(0).
 */
intType bigLowestSetBit (const const_bigIntType big1)

  {
    memSizeType big1_size;
    memSizeType pos;
    intType result;

  /* bigLowestSetBit */
    big1_size = big1->size;
    pos = 0;
    while (pos < big1_size && big1->bigdigits[pos] == 0) {
      pos++;
    } /* while */
    if (pos < big1_size) {
      result = digitLeastSignificantBit(big1->bigdigits[pos]);
      if (unlikely(pos > (memSizeType) (MAX_MEM_INDEX - result) >> BIGDIGIT_LOG2_SIZE)) {
        raise_error(RANGE_ERROR);
        result = 0;
      } else {
        result += (intType) (pos << BIGDIGIT_LOG2_SIZE);
      } /* if */
    } else {
      result = -1;
    } /* if */
    return result;
  } /* bigLowestSetBit */



/**
 *  Shift a 'bigInteger' number left by lshift bits.
 *  When lshift is negative a right shift is done instead.
 *  A << B is equivalent to A * 2_ ** B when B >= 0 holds.
 *  A << B is equivalent to A mdiv 2_ ** -B when B < 0 holds.
 *  @return the left shifted number.
 */
bigIntType bigLShift (const const_bigIntType big1, const intType lshift)

  {
    unsigned int digit_rshift;
    unsigned int digit_lshift;
    bigDigitType digit_mask;
    bigDigitType low_digit;
    bigDigitType high_digit;
    const bigDigitType *source_digits;
    bigDigitType *dest_digits;
    memSizeType size_reduction;
    memSizeType pos;
    memSizeType result_size;
    bigIntType result;

  /* bigLShift */
    if (unlikely(lshift < 0)) {
      result = bigRShift(big1, -lshift);
    } else if (unlikely(big1->size == 1 && big1->bigdigits[0] == 0)) {
      if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 1;
        result->bigdigits[0] = 0;
      } /* if */
    } else if ((lshift & BIGDIGIT_SIZE_MASK) == 0) {
      if (unlikely((uintType) lshift >> BIGDIGIT_LOG2_SIZE > MAX_BIG_LEN - big1->size)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result_size = big1->size + (memSizeType) ((uintType) lshift >> BIGDIGIT_LOG2_SIZE);
        if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = result_size;
          memcpy(&result->bigdigits[lshift >> BIGDIGIT_LOG2_SIZE], big1->bigdigits,
              (size_t) big1->size * sizeof(bigDigitType));
          memset(result->bigdigits, 0,
              (memSizeType) ((uintType) lshift >> BIGDIGIT_LOG2_SIZE) * sizeof(bigDigitType));
        } /* if */
      } /* if */
    } else if (unlikely(((uintType) lshift >> BIGDIGIT_LOG2_SIZE) + 1 > MAX_BIG_LEN - big1->size)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = big1->size + (memSizeType) ((uintType) lshift >> BIGDIGIT_LOG2_SIZE) + 1;
      digit_lshift = (unsigned int) ((uintType) lshift & BIGDIGIT_SIZE_MASK);
      digit_rshift = BIGDIGIT_SIZE - digit_lshift;
      size_reduction = 0;
      low_digit = big1->bigdigits[big1->size - 1];
      if (IS_NEGATIVE(low_digit)) {
        digit_mask = (BIGDIGIT_MASK << (digit_rshift - 1)) & BIGDIGIT_MASK;
        if ((low_digit & digit_mask) == digit_mask) {
          result_size--;
          size_reduction = 1;
        } else {
          low_digit = BIGDIGIT_MASK;
        } /* if */
      } else {
        if (low_digit >> (digit_rshift - 1) == 0) {
          result_size--;
          size_reduction = 1;
        } else {
          low_digit = 0;
        } /* if */
      } /* if */
      if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        dest_digits = &result->bigdigits[result_size];
        if (size_reduction) {
          source_digits = &big1->bigdigits[big1->size - 1];
        } else {
          source_digits = &big1->bigdigits[big1->size];
        } /* if */
        high_digit = (low_digit << digit_lshift) & BIGDIGIT_MASK;
        for (pos = big1->size - size_reduction; pos != 0; pos--) {
          low_digit = *--source_digits;
          *--dest_digits = high_digit | (low_digit >> digit_rshift);
          high_digit = (low_digit << digit_lshift) & BIGDIGIT_MASK;
        } /* for */
        *--dest_digits = high_digit;
        if (dest_digits > result->bigdigits) {
          memset(result->bigdigits, 0,
              (memSizeType) (dest_digits - result->bigdigits) * sizeof(bigDigitType));
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* bigLShift */



/**
 *  Shift a number left by lshift bits and assign the result back to number.
 *  When lshift is negative a right shift is done instead.
 */
void bigLShiftAssign (bigIntType *const big_variable, intType lshift)

  {
    bigIntType big1;
    unsigned int digit_rshift;
    unsigned int digit_lshift;
    bigDigitType digit_mask;
    bigDigitType low_digit;
    bigDigitType high_digit;
    const bigDigitType *source_digits;
    bigDigitType *dest_digits;
    memSizeType size_reduction;
    memSizeType pos;
    memSizeType result_size;
    bigIntType result;

  /* bigLShiftAssign */
    if (unlikely(lshift < 0)) {
      bigRShiftAssign(big_variable, -lshift);
    } else if (likely(lshift != 0)) {
      big1 = *big_variable;
      if (big1->size == 1 && big1->bigdigits[0] == 0) {
        if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 1;
          result->bigdigits[0] = 0;
          *big_variable = result;
          FREE_BIG(big1, big1->size);
        } /* if */
      } else if ((lshift & BIGDIGIT_SIZE_MASK) == 0) {
        if (unlikely((uintType) lshift >> BIGDIGIT_LOG2_SIZE > MAX_BIG_LEN - big1->size)) {
          raise_error(MEMORY_ERROR);
        } else {
          result_size = big1->size + (memSizeType) ((uintType) lshift >> BIGDIGIT_LOG2_SIZE);
          if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
            raise_error(MEMORY_ERROR);
          } else {
            result->size = result_size;
            memcpy(&result->bigdigits[lshift >> BIGDIGIT_LOG2_SIZE], big1->bigdigits,
                (size_t) big1->size * sizeof(bigDigitType));
            memset(result->bigdigits, 0,
                (memSizeType) ((uintType) lshift >> BIGDIGIT_LOG2_SIZE) * sizeof(bigDigitType));
            *big_variable = result;
            FREE_BIG(big1, big1->size);
          } /* if */
        } /* if */
      } else if (unlikely(((uintType) lshift >> BIGDIGIT_LOG2_SIZE) + 1 > MAX_BIG_LEN - big1->size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result_size = big1->size + (memSizeType) ((uintType) lshift >> BIGDIGIT_LOG2_SIZE) + 1;
        digit_lshift = (unsigned int) ((uintType) lshift & BIGDIGIT_SIZE_MASK);
        digit_rshift = BIGDIGIT_SIZE - digit_lshift;
        size_reduction = 0;
        low_digit = big1->bigdigits[big1->size - 1];
        if (IS_NEGATIVE(low_digit)) {
          digit_mask = (BIGDIGIT_MASK << (digit_rshift - 1)) & BIGDIGIT_MASK;
          if ((low_digit & digit_mask) == digit_mask) {
            result_size--;
            size_reduction = 1;
          } else {
            low_digit = BIGDIGIT_MASK;
          } /* if */
        } else {
          if (low_digit >> (digit_rshift - 1) == 0) {
            result_size--;
            size_reduction = 1;
          } else {
            low_digit = 0;
          } /* if */
        } /* if */
        if (result_size != big1->size) {
          if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
            raise_error(MEMORY_ERROR);
          } /* if */
        } else {
          result = big1;
        } /* if */
        if (result != NULL) {
          result->size = result_size;
          dest_digits = &result->bigdigits[result_size];
          if (size_reduction) {
            source_digits = &big1->bigdigits[big1->size - 1];
          } else {
            source_digits = &big1->bigdigits[big1->size];
          } /* if */
          high_digit = (low_digit << digit_lshift) & BIGDIGIT_MASK;
          for (pos = big1->size - size_reduction; pos != 0; pos--) {
            low_digit = *--source_digits;
            *--dest_digits = high_digit | (low_digit >> digit_rshift);
            high_digit = (low_digit << digit_lshift) & BIGDIGIT_MASK;
          } /* for */
          *--dest_digits = high_digit;
          if (dest_digits > result->bigdigits) {
            memset(result->bigdigits, 0,
                (memSizeType) (dest_digits - result->bigdigits) * sizeof(bigDigitType));
          } /* if */
          if (result != big1) {
            *big_variable = result;
            FREE_BIG(big1, big1->size);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* bigLShiftAssign */



/**
 * Shift one left by 'lshift' bits.
 * When 'lshift' is positive or zero this corresponts to
 * the computation of a power of two:
 *  bigLShiftOne(lshift)  corresponds to  2_ ** lshift
 * When 'lshift' is negative the result is zero.
 * @return one shifted left by 'lshift'.
 */
bigIntType bigLShiftOne (const intType lshift)

  {
    memSizeType result_size;
    int bit_pos;
    bigIntType result;

  /* bigLShiftOne */
    if (unlikely(lshift < 0)) {
      if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 1;
        result->bigdigits[0] = 0;
      } /* if */
    } else if (unlikely((((uintType) lshift + 1) >> BIGDIGIT_LOG2_SIZE) + 1 > MAX_BIG_LEN)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = (memSizeType) (((uintType) lshift + 1) >> BIGDIGIT_LOG2_SIZE) + 1;
      if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        bit_pos = (int) (lshift & BIGDIGIT_SIZE_MASK);
        if (bit_pos == BIGDIGIT_SIZE_MASK) {
          memset(result->bigdigits, 0, (result_size - 2) * sizeof(bigDigitType));
          result->bigdigits[result_size - 2] = ((bigDigitType) 1) << bit_pos;
          result->bigdigits[result_size - 1] = 0;
        } else {
          memset(result->bigdigits, 0, (result_size - 1) * sizeof(bigDigitType));
          result->bigdigits[result_size - 1] = ((bigDigitType) 1) << bit_pos;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* bigLShiftOne */



/**
 *  Exponentiation when the base is a power of two.
 *  @return (2 ** log2base) ** exponent
 *  @exception NUMERIC_ERROR When the exponent is negative.
 */
bigIntType bigLog2BaseIPow (const intType log2base, const intType exponent)

  {
    uintType high_shift;
    uintType low_shift;
    bigIntType result;

  /* bigLog2BaseIPow */
    if (unlikely(log2base < 0 || exponent < 0)) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else if (likely(log2base == 1)) {
      result = bigLShiftOne(exponent);
    } else if (log2base <= 10 && exponent <= MAX_DIV_10) {
      result = bigLShiftOne(log2base * exponent);
    } else {
      low_shift = uint_mult((uintType) log2base, (uintType) exponent, &high_shift);
      if (high_shift != 0 || (intType) low_shift < 0) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = bigLShiftOne((intType) low_shift);
      } /* if */
    } /* if */
    return result;
  } /* bigLog2BaseIPow */



/**
 *  Integer division truncated towards negative infinity.
 *  The modulo (remainder) of this division is computed with bigMod.
 *  Therefore this division is called modulo division (MDiv).
 *  The memory for the result is requested and the normalized result
 *  is returned. When divisor has just one digit or when dividend
 *  has less digits than divisor the functions bigMDiv1() or
 *  bigMDivSizeLess() are called. In the general case the absolute
 *  values of dividend and divisor are taken. Then dividend is
 *  extended by one leading zero digit. After that dividend and divisor
 *  are shifted to the left such that the most significant bit
 *  of divisor is set. This fulfills the preconditions for calling
 *  uBigDiv() which does the main work of the division.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
bigIntType bigMDiv (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    boolType negative = FALSE;
    bigIntType dividend_help;
    bigIntType divisor_help;
    unsigned int shift;
    bigDigitType mdiv1_remainder = 0;
    bigIntType quotient;

  /* bigMDiv */
    if (divisor->size == 1) {
      quotient = bigMDiv1(dividend, divisor->bigdigits[0]);
      return quotient;
    } else if (dividend->size < divisor->size) {
      quotient = bigMDivSizeLess(dividend, divisor);
      return quotient;
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(dividend_help, dividend->size + 2))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          negative = TRUE;
          positive_copy_of_negative_big(dividend_help, dividend);
        } else {
          dividend_help->size = dividend->size;
          memcpy(dividend_help->bigdigits, dividend->bigdigits,
              (size_t) dividend->size * sizeof(bigDigitType));
        } /* if */
        dividend_help->bigdigits[dividend_help->size] = 0;
        dividend_help->size++;
      } /* if */
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(divisor_help, divisor->size + 1))) {
        FREE_BIG(dividend_help,  dividend->size + 2);
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(divisor->bigdigits[divisor->size - 1])) {
          negative = !negative;
          positive_copy_of_negative_big(divisor_help, divisor);
        } else {
          divisor_help->size = divisor->size;
          memcpy(divisor_help->bigdigits, divisor->bigdigits,
              (size_t) divisor->size * sizeof(bigDigitType));
        } /* if */
      } /* if */
      if (unlikely(!ALLOC_BIG_SIZE_OK(quotient, dividend_help->size - divisor_help->size + 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        quotient->size = dividend_help->size - divisor_help->size + 1;
        quotient->bigdigits[quotient->size - 1] = 0;
        shift = (unsigned int) (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
        if (shift == 0) {
          /* The most significant digit of divisor_help is 0. Just ignore it */
          dividend_help->size--;
          divisor_help->size--;
          if (divisor_help->size == 1) {
            mdiv1_remainder = uBigMDiv1(dividend_help, divisor_help->bigdigits[0], quotient);
          } else {
            uBigDiv(dividend_help, divisor_help, quotient);
          } /* if */
        } else {
          shift = BIGDIGIT_SIZE - shift;
          uBigLShift(dividend_help, shift);
          uBigLShift(divisor_help, shift);
          uBigDiv(dividend_help, divisor_help, quotient);
        } /* if */
        if (negative) {
          if ((divisor_help->size == 1 && mdiv1_remainder != 0) ||
              (divisor_help->size != 1 && uBigIsNot0(dividend_help))) {
            uBigIncr(quotient);
          } /* if */
          negate_positive_big(quotient);
        } /* if */
        quotient = normalize(quotient);
      } /* if */
      FREE_BIG(dividend_help, dividend->size + 2);
      FREE_BIG(divisor_help, divisor->size + 1);
      return quotient;
    } /* if */
  } /* bigMDiv */



/**
 *  Compute the modulo (remainder) of the integer division bigMDiv.
 *  The modulo has the same sign as the divisor. The memory for the result
 *  is requested and the normalized result is returned. When divisor has
 *  just one digit or when dividend has less digits than divisor the
 *  functions bigMod1() or bigModSizeLess() are called. In the general case
 *  the absolute values of dividend and divisor are taken. Then dividend is
 *  extended by one leading zero digit. After that dividend and divisor
 *  are shifted to the left such that the most significant bit
 *  of divisor is set. This fulfills the preconditions for calling
 *  uBigRem() which does the main work of the division. Afterwards
 *  the result must be shifted to the right to get the remainder.
 *  If dividend and divisor have the same sign the modulo has the same
 *  value as the remainder. When the remainder is zero the modulo
 *  is also zero. If the signs of dividend and divisor are different the
 *  modulo is computed from the remainder by adding dividend.
 *  @return the modulo of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
bigIntType bigMod (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    boolType negative1 = FALSE;
    boolType negative2 = FALSE;
    bigIntType divisor_help;
    unsigned int shift;
    bigIntType modulo;

  /* bigMod */
    if (divisor->size == 1) {
      modulo = bigMod1(dividend, divisor->bigdigits[0]);
      return modulo;
    } else if (dividend->size < divisor->size) {
      modulo = bigModSizeLess(dividend, divisor);
      return modulo;
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(modulo, dividend->size + 2))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          negative1 = TRUE;
          positive_copy_of_negative_big(modulo, dividend);
        } else {
          modulo->size = dividend->size;
          memcpy(modulo->bigdigits, dividend->bigdigits,
              (size_t) dividend->size * sizeof(bigDigitType));
        } /* if */
        modulo->bigdigits[modulo->size] = 0;
        modulo->size++;
      } /* if */
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(divisor_help, divisor->size + 1))) {
        FREE_BIG(modulo,  dividend->size + 2);
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(divisor->bigdigits[divisor->size - 1])) {
          negative2 = TRUE;
          positive_copy_of_negative_big(divisor_help, divisor);
        } else {
          divisor_help->size = divisor->size;
          memcpy(divisor_help->bigdigits, divisor->bigdigits,
              (size_t) divisor->size * sizeof(bigDigitType));
        } /* if */
      } /* if */
      shift = (unsigned int) (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
      if (shift == 0) {
        /* The most significant digit of divisor_help is 0. Just ignore it */
        modulo->size--;
        divisor_help->size--;
        if (divisor_help->size == 1) {
          modulo->bigdigits[0] = uBigRem1(modulo, divisor_help->bigdigits[0]);
          memset(&modulo->bigdigits[1], 0,
              (size_t) (modulo->size - 1) * sizeof(bigDigitType));
        } else {
          uBigRem(modulo, divisor_help);
        } /* if */
        modulo->bigdigits[modulo->size] = 0;
        divisor_help->size++;
      } else {
        shift = BIGDIGIT_SIZE - shift;
        uBigLShift(modulo, shift);
        uBigLShift(divisor_help, shift);
        uBigRem(modulo, divisor_help);
        uBigRShift(modulo, shift);
      } /* if */
      modulo->bigdigits[dividend->size + 1] = 0;
      modulo->size = dividend->size + 2;
      if (negative1) {
        if (negative2) {
          negate_positive_big(modulo);
        } else {
          if (uBigIsNot0(modulo)) {
            negate_positive_big(modulo);
            bigAddTo(modulo, divisor);
          } /* if */
        } /* if */
      } else {
        if (negative2) {
          if (uBigIsNot0(modulo)) {
            bigAddTo(modulo, divisor);
          } /* if */
        } /* if */
      } /* if */
      modulo = normalize(modulo);
      FREE_BIG(divisor_help, divisor->size + 1);
      return modulo;
    } /* if */
  } /* bigMod */



/**
 *  Multiply two 'bigInteger' numbers.
 *  @return the product of the two numbers.
 */
bigIntType bigMult (const_bigIntType factor1, const_bigIntType factor2)

  {
    boolType negative = FALSE;
    bigIntType factor1_help = NULL;
    bigIntType factor2_help = NULL;
    bigIntType product;

  /* bigMult */
    if (IS_NEGATIVE(factor1->bigdigits[factor1->size - 1])) {
      negative = TRUE;
      factor1_help = alloc_positive_copy_of_negative_big(factor1);
      factor1 = factor1_help;
      if (unlikely(factor1_help == NULL)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
    } /* if */
    if (IS_NEGATIVE(factor2->bigdigits[factor2->size - 1])) {
      negative = !negative;
      factor2_help = alloc_positive_copy_of_negative_big(factor2);
      factor2 = factor2_help;
      if (unlikely(factor2_help == NULL)) {
        if (factor1_help != NULL) {
          FREE_BIG(factor1_help, factor1_help->size);
        } /* if */
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
    } /* if */
    /* printf("bigMult(%u, %u)\n", factor1->size, factor2->size); */
#if 0
    if (unlikely(!ALLOC_BIG(product, factor1->size + factor2->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      uBigMult(factor1, factor2, product);
      product->size = factor1->size + factor2->size;
      if (negative) {
        negate_positive_big(product);
      } /* if */
      product = normalize(product);
    } /* if */
#else
    product = uBigMultK(factor1, factor2, negative);
#endif
    if (factor1_help != NULL) {
      FREE_BIG(factor1_help, factor1_help->size);
    } /* if */
    if (factor2_help != NULL) {
      FREE_BIG(factor2_help, factor2_help->size);
    } /* if */
    return product;
  } /* bigMult */



/**
 *  Multiply a 'bigInteger' number by a factor and assign the result back to number.
 */
void bigMultAssign (bigIntType *const big_variable, const_bigIntType factor)

  {
    const_bigIntType big1;
    boolType negative = FALSE;
    const_bigIntType big1_help = NULL;
    const_bigIntType factor_help = NULL;
    memSizeType pos1;
    memSizeType pos2;
    doubleBigDigitType carry = 0;
    bigIntType product;

  /* bigMultAssign */
    if (factor->size == 1) {
      bigMultAssign1(big_variable, factor->bigdigits[0]);
    } else {
      big1 = *big_variable;
      if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        negative = TRUE;
        big1_help = alloc_positive_copy_of_negative_big(big1);
        big1 = big1_help;
        if (unlikely(big1_help == NULL)) {
          raise_error(MEMORY_ERROR);
          return;
        } /* if */
      } /* if */
      if (IS_NEGATIVE(factor->bigdigits[factor->size - 1])) {
        negative = !negative;
        factor_help = alloc_positive_copy_of_negative_big(factor);
        factor = factor_help;
        if (unlikely(factor_help == NULL)) {
          if (big1_help != NULL) {
            FREE_BIG(big1_help, big1_help->size);
          } /* if */
          raise_error(MEMORY_ERROR);
          return;
        } /* if */
      } /* if */
      if (unlikely(!ALLOC_BIG(product, big1->size + factor->size))) {
        raise_error(MEMORY_ERROR);
      } else {
        pos2 = 0;
        do {
          carry += (doubleBigDigitType) big1->bigdigits[0] * factor->bigdigits[pos2];
          product->bigdigits[pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          pos2++;
        } while (pos2 < factor->size);
        product->bigdigits[factor->size] = (bigDigitType) (carry & BIGDIGIT_MASK);
        for (pos1 = 1; pos1 < big1->size; pos1++) {
          carry = 0;
          pos2 = 0;
          do {
            carry += (doubleBigDigitType) product->bigdigits[pos1 + pos2] +
                (doubleBigDigitType) big1->bigdigits[pos1] * factor->bigdigits[pos2];
            product->bigdigits[pos1 + pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
            carry >>= BIGDIGIT_SIZE;
            pos2++;
          } while (pos2 < factor->size);
          product->bigdigits[pos1 + factor->size] = (bigDigitType) (carry & BIGDIGIT_MASK);
        } /* for */
        product->size = big1->size + factor->size;
        if (negative) {
          negate_positive_big(product);
        } /* if */
        product = normalize(product);
        FREE_BIG(*big_variable, (*big_variable)->size);
        *big_variable = product;
      } /* if */
      if (big1_help != NULL) {
        FREE_BIG(big1_help, big1_help->size);
      } /* if */
      if (factor_help != NULL) {
        FREE_BIG(factor_help, factor_help->size);
      } /* if */
    } /* if */
  } /* bigMultAssign */



/**
 *  Multiply factor1 with the bigdigit factor2.
 *  The range of factor2 is restricted and it is the job of the
 *  compiler to assure that factor2 is within the allowed range.
 *  @param factor2 Multiplication factor that must be
 *         in the range of signedBigDigitType.
 *  @return the product of factor1 * factor2.
 */
bigIntType bigMultSignedDigit (const_bigIntType factor1, intType factor2)

  {
    bigIntType product;

  /* bigMultSignedDigit */
    /* printf("bigMultSignedDigit(factor1->size=%lu, %ld)\n", factor1->size, factor2); */
    if (unlikely(!ALLOC_BIG_CHECK_SIZE(product, factor1->size + 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      product->size = factor1->size + 1;
      if (factor2 < 0) {
        if (IS_NEGATIVE(factor1->bigdigits[factor1->size - 1])) {
          uBigMultNegativeWithNegatedDigit(factor1, (bigDigitType) -factor2 & BIGDIGIT_MASK, product);
        } else {
          uBigMultPositiveWithNegatedDigit(factor1, (bigDigitType) -factor2 & BIGDIGIT_MASK, product);
        } /* if */
      } else {
        if (IS_NEGATIVE(factor1->bigdigits[factor1->size - 1])) {
          uBigMultNegativeWithDigit(factor1, (bigDigitType) factor2, product);
        } else {
          uBigMultPositiveWithDigit(factor1, (bigDigitType) factor2, product);
        } /* if */
      } /* if */
      product = normalize(product);
    } /* if */
    return product;
  } /* bigMultSignedDigit */



/**
 *  Minus sign, negate a 'bigInteger' number.
 *  @return the negated value of the number.
 */
bigIntType bigNegate (const const_bigIntType big1)

  {
    memSizeType pos;
    doubleBigDigitType carry = 1;
    bigIntType resized_result;
    bigIntType result;

  /* bigNegate */
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, big1->size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = big1->size;
      pos = 0;
      do {
        carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
        result->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < big1->size);
      if (IS_NEGATIVE(result->bigdigits[pos - 1])) {
        if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
          REALLOC_BIG_CHECK_SIZE(resized_result, result, pos, pos + 1);
          if (unlikely(resized_result == NULL)) {
            FREE_BIG(result, pos);
            raise_error(MEMORY_ERROR);
            return NULL;
          } else {
            result = resized_result;
            COUNT3_BIG(pos, pos + 1);
            result->size++;
            result->bigdigits[pos] = 0;
          } /* if */
        } else if (result->bigdigits[pos - 1] == BIGDIGIT_MASK &&
            pos >= 2 && IS_NEGATIVE(result->bigdigits[pos - 2])) {
          REALLOC_BIG_SIZE_OK(resized_result, result, pos, pos - 1);
          /* Avoid a MEMORY_ERROR in the strange case     */
          /* when a 'realloc' which shrinks memory fails. */
          if (likely(resized_result != NULL)) {
            result = resized_result;
          } /* if */
          COUNT3_BIG(pos, pos - 1);
          result->size--;
        } /* if */
      } /* if */
      return result;
    } /* if */
  } /* bigNegate */



/**
 *  Minus sign, negate a 'bigInteger' number.
 *  Big1 is assumed to be a temporary value which is reused.
 *  @return the negated value of the number.
 */
bigIntType bigNegateTemp (bigIntType big1)

  {
    memSizeType pos;
    doubleBigDigitType carry = 1;
    boolType negative;
    bigIntType resized_big1;

  /* bigNegateTemp */
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    pos = 0;
    do {
      carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big1->size);
    if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      if (negative) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, pos, pos + 1);
        if (unlikely(resized_big1 == NULL)) {
          FREE_BIG(big1, pos);
          raise_error(MEMORY_ERROR);
          return NULL;
        } else {
          big1 = resized_big1;
          COUNT3_BIG(pos, pos + 1);
          big1->size++;
          big1->bigdigits[pos] = 0;
        } /* if */
      } else if (big1->bigdigits[pos - 1] == BIGDIGIT_MASK &&
          pos >= 2 && IS_NEGATIVE(big1->bigdigits[pos - 2])) {
        REALLOC_BIG_SIZE_OK(resized_big1, big1, pos, pos - 1);
        /* Avoid a MEMORY_ERROR in the strange case     */
        /* when a 'realloc' which shrinks memory fails. */
        if (likely(resized_big1 != NULL)) {
          big1 = resized_big1;
        } /* if */
        COUNT3_BIG(pos, pos - 1);
        big1->size--;
      } /* if */
    } /* if */
    return big1;
  } /* bigNegateTemp */



/**
 *  Determine if a 'bigInteger' number is odd.
 *  @return TRUE if the number is odd,
 *          FALSE otherwise.
 */
boolType bigOdd (const const_bigIntType big1)

  { /* bigOdd */
    return (boolType) (big1->bigdigits[0] & 1);
  } /* bigOdd */



bigIntType bigOr (const_bigIntType big1, const_bigIntType big2)

  {
    const_bigIntType help_big;
    memSizeType pos;
    bigDigitType big2_sign;
    bigIntType result;

  /* bigOr */
    if (big2->size > big1->size) {
      help_big = big1;
      big1 = big2;
      big2 = help_big;
    } /* if */
    if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, big1->size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      pos = 0;
      do {
        result->bigdigits[pos] = big1->bigdigits[pos] | big2->bigdigits[pos];
        pos++;
      } while (pos < big2->size);
      big2_sign = IS_NEGATIVE(big2->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
      for (; pos < big1->size; pos++) {
        result->bigdigits[pos] = big1->bigdigits[pos] | big2_sign;
      } /* for */
      result->size = pos;
      result = normalize(result);
      return result;
    } /* if */
  } /* bigOr */



/**
 *  Convert a string to a 'bigInteger' number.
 *  The string must contain an integer literal consisting of an
 *  optional + or - sign, followed by a sequence of digits. Other
 *  characters as well as leading or trailing whitespace characters are
 *  not allowed. The sequence of digits is taken to be decimal.
 *  @return the 'bigInteger' result of the conversion.
 *  @exception RANGE_ERROR When the string is empty or does not contain
 *             an integer literal.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
bigIntType bigParse (const const_striType stri)

  {
    memSizeType result_size;
    boolType okay;
    boolType negative;
    memSizeType position;
    memSizeType limit;
    bigDigitType digitval;
    bigIntType result;

  /* bigParse */
    if (unlikely(stri->size == 0)) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      result_size = (stri->size - 1) / DECIMAL_DIGITS_IN_BIGDIGIT + 1;
      if (unlikely(!ALLOC_BIG(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        result->size = 1;
        result->bigdigits[0] = 0;
        okay = TRUE;
        position = 0;
        if (stri->mem[0] == ((strElemType) '-')) {
          negative = TRUE;
          position++;
        } else {
          if (stri->mem[0] == ((strElemType) '+')) {
            position++;
          } /* if */
          negative = FALSE;
        } /* if */
        if (unlikely(position >= stri->size)) {
          okay = FALSE;
        } else {
          limit = (stri->size - position - 1) % DECIMAL_DIGITS_IN_BIGDIGIT + position + 1;
          do {
            digitval = 0;
            while (position < limit && okay) {
              if (likely(stri->mem[position] >= ((strElemType) '0') &&
                         stri->mem[position] <= ((strElemType) '9'))) {
                digitval = (bigDigitType) 10 * digitval +
                    (bigDigitType) stri->mem[position] - (bigDigitType) '0';
              } else {
                okay = FALSE;
              } /* if */
              position++;
            } /* while */
            uBigMultByPowerOf10AndAdd(result, (doubleBigDigitType) digitval);
            limit += DECIMAL_DIGITS_IN_BIGDIGIT;
          } while (position < stri->size && okay);
        } /* if */
        if (likely(okay)) {
          memset(&result->bigdigits[result->size], 0,
              (size_t) (result_size - result->size) * sizeof(bigDigitType));
          result->size = result_size;
          if (negative) {
            negate_positive_big(result);
          } /* if */
          result = normalize(result);
          return result;
        } else {
          FREE_BIG(result, result_size);
          raise_error(RANGE_ERROR);
          return NULL;
        } /* if */
      } /* if */
    } /* if */
  } /* bigParse */



/**
 *  Convert a numeric string, with a specified radix, to a 'bigInteger'.
 *  The numeric string must contain the representation of an integer
 *  in the specified radix. It consists of an optional + or - sign,
 *  followed by a sequence of digits in the specified radix. Digit values
 *  from 10 upward can be encoded with upper or lower case letters.
 *  E.g.: 10 can be encoded with A or a, 11 with B or b, etc. Other
 *  characters as well as leading or trailing whitespace characters
 *  are not allowed.
 *  @return the 'bigInteger' result of the conversion.
 *  @exception RANGE_ERROR When base < 2 or base > 36 holds or
 *             the string is empty or it does not contain an integer
 *             literal with the specified base.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
bigIntType bigParseBased (const const_striType stri, intType base)

  {
    memSizeType result_size;
    boolType okay;
    boolType negative;
    memSizeType position;
    uint8Type based_digit_size;
    uint8Type based_digits_in_bigdigit;
    bigDigitType power_of_base_in_bigdigit;
    memSizeType limit;
    bigDigitType digitval;
    bigIntType result;

  /* bigParseBased */
    /* printf("bigParseBased(");
       prot_stri(stri);
       printf(", %d)\n", base); */
    if (unlikely(stri->size == 0 || base < 2 || base > 36)) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else if (unlikely(stri->size > MAX_MEMSIZETYPE / 6)) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      based_digit_size = (uint8Type) (uint8MostSignificantBit((uint8Type) (base - 1)) + 1);
      /* Estimate the number of bits necessary: */
      result_size = stri->size * (memSizeType) based_digit_size;
      /* Compute the number of bigDigits: */
      result_size = result_size / BIGDIGIT_SIZE + 1;
      if (unlikely(!ALLOC_BIG(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        result->size = 1;
        result->bigdigits[0] = 0;
        okay = TRUE;
        position = 0;
        if (stri->mem[0] == ((strElemType) '-')) {
          negative = TRUE;
          position++;
        } else {
          if (stri->mem[0] == ((strElemType) '+')) {
            position++;
          } /* if */
          negative = FALSE;
        } /* if */
        if (unlikely(position >= stri->size)) {
          okay = FALSE;
        } else {
          based_digits_in_bigdigit = radixDigitsInBigdigit[base - 2];
          power_of_base_in_bigdigit = powerOfRadixInBigdigit[base - 2];
          limit = (stri->size - position - 1) % based_digits_in_bigdigit + position + 1;
          do {
            digitval = 0;
            while (position < limit && okay) {
              if (stri->mem[position] >= ((strElemType) '0') &&
                  stri->mem[position] <= ((strElemType) '9')) {
                digitval = (bigDigitType) base * digitval +
                    (bigDigitType) stri->mem[position] - (bigDigitType) '0';
              } else if (stri->mem[position] >= ((strElemType) 'a') &&
                         stri->mem[position] <= ((strElemType) 'z')) {
                digitval = (bigDigitType) base * digitval +
                    (bigDigitType) stri->mem[position] - (bigDigitType) 'a' + (bigDigitType) 10;
              } else if (stri->mem[position] >= ((strElemType) 'A') &&
                         stri->mem[position] <= ((strElemType) 'Z')) {
                digitval = (bigDigitType) base * digitval +
                    (bigDigitType) stri->mem[position] - (bigDigitType) 'A' + (bigDigitType) 10;
              } else {
                okay = FALSE;
              } /* if */
              position++;
            } /* while */
            uBigMultiplyAndAdd(result, power_of_base_in_bigdigit, (doubleBigDigitType) digitval);
            limit += based_digits_in_bigdigit;
          } while (position < stri->size && okay);
        } /* if */
        if (likely(okay)) {
          memset(&result->bigdigits[result->size], 0,
              (size_t) (result_size - result->size) * sizeof(bigDigitType));
          result->size = result_size;
          if (negative) {
            negate_positive_big(result);
          } /* if */
          result = normalize(result);
          return result;
        } else {
          FREE_BIG(result, result_size);
          raise_error(RANGE_ERROR);
          return NULL;
        } /* if */
      } /* if */
    } /* if */
  } /* bigParseBased */



/**
 *  Predecessor of a 'bigInteger' number.
 *  pred(A) is equivalent to A-1 .
 *  @return big1 - 1 .
 */
bigIntType bigPred (const const_bigIntType big1)

  {
    memSizeType pos;
    bigIntType resized_result;
    bigIntType result;

  /* bigPred */
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, big1->size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = big1->size;
      pos = 0;
      if (big1->bigdigits[pos] == 0) {
        if (big1->size == 1) {
          result->bigdigits[pos] = BIGDIGIT_MASK;
          pos++;
        } else {
          do {
            result->bigdigits[pos] = BIGDIGIT_MASK;
            pos++;
          } while (big1->bigdigits[pos] == 0);
          /* memset(result->bigdigits, 0xFF, pos * sizeof(bigDigitType)); */
        } /* if */
      } /* if */
      if (pos < big1->size) {
        result->bigdigits[pos] = big1->bigdigits[pos] - 1;
        pos++;
        memcpy(&result->bigdigits[pos], &big1->bigdigits[pos],
            (big1->size - pos) * sizeof(bigDigitType));
        pos = big1->size;
        /* while (pos < big1->size) {
          result->bigdigits[pos] = big1->bigdigits[pos];
          pos++;
        } ** while */
      } /* if */
      if (!IS_NEGATIVE(result->bigdigits[pos - 1])) {
        if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
          REALLOC_BIG_CHECK_SIZE(resized_result, result, pos, pos + 1);
          if (unlikely(resized_result == NULL)) {
            FREE_BIG(result, pos);
            raise_error(MEMORY_ERROR);
            return NULL;
          } else {
            result = resized_result;
            COUNT3_BIG(pos, pos + 1);
            result->size++;
            result->bigdigits[pos] = BIGDIGIT_MASK;
          } /* if */
        } else if (result->bigdigits[pos - 1] == 0 &&
            pos >= 2 && !IS_NEGATIVE(result->bigdigits[pos - 2])) {
          REALLOC_BIG_SIZE_OK(resized_result, result, pos, pos - 1);
          /* Avoid a MEMORY_ERROR in the strange case     */
          /* when a 'realloc' which shrinks memory fails. */
          if (likely(resized_result != NULL)) {
            result = resized_result;
          } /* if */
          COUNT3_BIG(pos, pos - 1);
          result->size--;
        } /* if */
      } /* if */
      return result;
    } /* if */
  } /* bigPred */



/**
 *  Returns a signed big integer decremented by 1.
 *  Big1 is assumed to be a temporary value which is reused.
 */
bigIntType bigPredTemp (bigIntType big1)

  {
    memSizeType pos;
    boolType negative;
    bigIntType resized_big1;

  /* bigPredTemp */
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    pos = 0;
    if (big1->bigdigits[pos] == 0) {
      if (big1->size == 1) {
        big1->bigdigits[pos] = BIGDIGIT_MASK;
        pos++;
      } else {
        do {
          big1->bigdigits[pos] = BIGDIGIT_MASK;
          pos++;
        } while (big1->bigdigits[pos] == 0);
        /* memset(big1->bigdigits, 0xFF, pos * sizeof(bigDigitType)); */
      } /* if */
    } /* if */
    if (pos < big1->size) {
      big1->bigdigits[pos]--;
    } /* if */
    pos = big1->size;
    if (!IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      if (negative) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, pos, pos + 1);
        if (unlikely(resized_big1 == NULL)) {
          FREE_BIG(big1, big1->size);
          raise_error(MEMORY_ERROR);
          big1 = NULL;
        } else {
          big1 = resized_big1;
          COUNT3_BIG(pos, pos + 1);
          big1->size++;
          big1->bigdigits[pos] = BIGDIGIT_MASK;
        } /* if */
      } else if (big1->bigdigits[pos - 1] == 0 &&
          pos >= 2 && !IS_NEGATIVE(big1->bigdigits[pos - 2])) {
        REALLOC_BIG_SIZE_OK(resized_big1, big1, pos, pos - 1);
        /* Avoid a MEMORY_ERROR in the strange case     */
        /* when a 'realloc' which shrinks memory fails. */
        if (likely(resized_big1 != NULL)) {
          big1 = resized_big1;
        } /* if */
        COUNT3_BIG(pos, pos - 1);
        big1->size--;
      } /* if */
    } /* if */
    return big1;
  } /* bigPredTemp */



/**
 *  Convert a big integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with letters.
 *  For negative numbers a minus sign is prepended.
 *  @param upperCase Decides about the letter case.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR When base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType bigRadix (const const_bigIntType big1, intType base,
    boolType upperCase)

  {
    striType result;

  /* bigRadix */
    /* printf("bigRadix(%s, %ld, %d)\n", bigHexCStri(big1), base, upperCase); */
    switch (base) {  /* Cases sorted by probability. */
      case 16: result = bigRadixPow2(big1, 4,  0xf, upperCase); break;
      case  8: result = bigRadixPow2(big1, 3,  0x7, upperCase); break;
      case 10: result = bigStr(big1);                           break;
      case  2: result = bigRadixPow2(big1, 1,  0x1, upperCase); break;
      case  4: result = bigRadixPow2(big1, 2,  0x3, upperCase); break;
      case 32: result = bigRadixPow2(big1, 5, 0x1f, upperCase); break;
      default:
        if (unlikely(base < 2 || base > 36)) {
          raise_error(RANGE_ERROR);
          result = NULL;
        } else {
          result = bigRadix2To36(big1, (unsigned int) base, upperCase);
        } /* if */
        break;
    } /* switch */
    return result;
  } /* bigRadix */



/**
 *  Compute pseudo-random number in the range [low, high].
 *  The random values are uniform distributed. The memory for
 *  the result is requested and the normalized result is returned.
 *  @return a random number such that low <= rand(low, high) and
 *          rand(low, high) <= high holds.
 *  @exception RANGE_ERROR The range is empty (low > high holds).
 */
bigIntType bigRand (const const_bigIntType low,
    const const_bigIntType high)

  {
    bigIntType scale_limit;
    int usedBits;
    bigDigitType mask;
    memSizeType pos;
    doubleBigDigitType random_number = 0;
    memSizeType result_size;
    bigIntType result;

  /* bigRand */
    if (unlikely(bigCmp(low, high) > 0)) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      scale_limit = bigSbtr(high, low);
      if (low->size > scale_limit->size) {
        result_size = low->size + 1;
      } else {
        result_size = scale_limit->size + 1;
      } /* if */
      if (unlikely(!ALLOC_BIG(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        memset(&result->bigdigits[scale_limit->size], 0,
            (size_t) (result_size - scale_limit->size) * sizeof(bigDigitType));
        result->size = scale_limit->size;
        usedBits = digitMostSignificantBit(scale_limit->bigdigits[scale_limit->size - 1]) + 1;
        if (usedBits == 0) {
          mask = 0;
        } else {
          mask = ((bigDigitType) BIGDIGIT_MASK) >> (BIGDIGIT_SIZE - (memSizeType) (usedBits));
        } /* if */
        do {
          pos = 0;
          do {
            if (random_number == 0) {
              random_number = uint_rand();
            } /* if */
            result->bigdigits[pos] = (bigDigitType) (random_number & BIGDIGIT_MASK);
            random_number >>= BIGDIGIT_SIZE;
            pos++;
          } while (pos < scale_limit->size);
          result->bigdigits[pos - 1] &= mask;
        } while (bigCmp(result, scale_limit) > 0);
        result->size = result_size;
        bigAddTo(result, low);
        result = normalize(result);
        FREE_BIG(scale_limit, scale_limit->size);
        return result;
      } /* if */
    } /* if */
  } /* bigRand */



/**
 *  Compute the remainder of the integer division bigDiv.
 *  The remainder has the same sign as the dividend. The memory for the result
 *  is requested and the normalized result is returned. When divisor has
 *  just one digit or when dividend has less digits than divisor the
 *  functions bigRem1() or bigRemSizeLess() are called. In the general case
 *  the absolute values of dividend and divisor are taken. Then dividend is
 *  extended by one leading zero digit. After that dividend and divisor
 *  are shifted to the left such that the most significant bit
 *  of divisor is set. This fulfills the preconditions for calling
 *  uBigRem() which does the main work of the division. Afterwards
 *  the result must be shifted to the right to get the remainder.
 *  @return the remainder of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
bigIntType bigRem (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    boolType negative = FALSE;
    bigIntType divisor_help;
    unsigned int shift;
    bigIntType remainder;

  /* bigRem */
    if (divisor->size == 1) {
      remainder = bigRem1(dividend, divisor->bigdigits[0]);
      return remainder;
    } else if (dividend->size < divisor->size) {
      remainder = bigRemSizeLess(dividend, divisor);
      return remainder;
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(remainder, dividend->size + 2))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          negative = TRUE;
          positive_copy_of_negative_big(remainder, dividend);
        } else {
          remainder->size = dividend->size;
          memcpy(remainder->bigdigits, dividend->bigdigits,
              (size_t) dividend->size * sizeof(bigDigitType));
        } /* if */
        remainder->bigdigits[remainder->size] = 0;
        remainder->size++;
      } /* if */
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(divisor_help, divisor->size + 1))) {
        FREE_BIG(remainder,  dividend->size + 2);
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(divisor->bigdigits[divisor->size - 1])) {
          positive_copy_of_negative_big(divisor_help, divisor);
        } else {
          divisor_help->size = divisor->size;
          memcpy(divisor_help->bigdigits, divisor->bigdigits,
              (size_t) divisor->size * sizeof(bigDigitType));
        } /* if */
      } /* if */
      shift = (unsigned int) (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
      if (shift == 0) {
        /* The most significant digit of divisor_help is 0. Just ignore it */
        remainder->size--;
        divisor_help->size--;
        if (divisor_help->size == 1) {
          remainder->bigdigits[0] = uBigRem1(remainder, divisor_help->bigdigits[0]);
          memset(&remainder->bigdigits[1], 0,
              (size_t) (remainder->size - 1) * sizeof(bigDigitType));
        } else {
          uBigRem(remainder, divisor_help);
        } /* if */
        remainder->bigdigits[remainder->size] = 0;
        divisor_help->size++;
      } else {
        shift = BIGDIGIT_SIZE - shift;
        uBigLShift(remainder, shift);
        uBigLShift(divisor_help, shift);
        uBigRem(remainder, divisor_help);
        uBigRShift(remainder, shift);
      } /* if */
      remainder->bigdigits[dividend->size + 1] = 0;
      remainder->size = dividend->size + 2;
      if (negative) {
        negate_positive_big(remainder);
      } /* if */
      remainder = normalize(remainder);
      FREE_BIG(divisor_help, divisor->size + 1);
      return remainder;
    } /* if */
  } /* bigRem */



/**
 *  Shift a 'bigInteger' number right by rshift bits.
 *  When rshift is negative a left shift is done instead.
 *  A >> B is equivalent to A mdiv 2_ ** B when B >= 0 holds.
 *  A >> B is equivalent to A * 2_ ** -B when B < 0 holds.
 *  @return the right shifted number.
 */
bigIntType bigRShift (const const_bigIntType big1, const intType rshift)

  {
    memSizeType size_reduction;
    unsigned int digit_rshift;
    unsigned int digit_lshift;
    bigDigitType digit_mask;
    bigDigitType low_digit;
    bigDigitType high_digit;
    const bigDigitType *source_digits;
    bigDigitType *dest_digits;
    memSizeType pos;
    memSizeType result_size;
    bigIntType result;

  /* bigRShift */
    if (unlikely(rshift < 0)) {
      result = bigLShift(big1, -rshift);
    } else if (big1->size <= (uintType) rshift >> BIGDIGIT_LOG2_SIZE) {
      if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 1;
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          result->bigdigits[0] = BIGDIGIT_MASK;
        } else {
          result->bigdigits[0] = 0;
        } /* if */
      } /* if */
    } else {
      size_reduction = (memSizeType) ((uintType) rshift >> BIGDIGIT_LOG2_SIZE);
      result_size = big1->size - size_reduction;
      if ((rshift & BIGDIGIT_SIZE_MASK) == 0) {
        if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = result_size;
          memcpy(result->bigdigits, &big1->bigdigits[size_reduction],
              (size_t) result_size * sizeof(bigDigitType));
        } /* if */
      } else {
        digit_rshift = (unsigned int) ((uintType) rshift & BIGDIGIT_SIZE_MASK);
        digit_lshift = BIGDIGIT_SIZE - digit_rshift;
        if (result_size > 1) {
          high_digit = big1->bigdigits[big1->size - 1];
          if (IS_NEGATIVE(high_digit)) {
            digit_mask = (BIGDIGIT_MASK << (digit_rshift - 1)) & BIGDIGIT_MASK;
            if ((digit_rshift == 1 && high_digit == BIGDIGIT_MASK) ||
                (high_digit & digit_mask) == digit_mask) {
              result_size--;
            } /* if */
          } else {
            if ((digit_rshift == 1 && high_digit == 0) ||
                high_digit >> (digit_rshift - 1) == 0) {
              result_size--;
            } /* if */
          } /* if */
        } /* if */
        if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = result_size;
          source_digits = &big1->bigdigits[size_reduction];
          dest_digits = result->bigdigits;
          high_digit = *source_digits++;
          low_digit = high_digit >> digit_rshift;
          for (pos = big1->size - size_reduction - 1; pos != 0; pos--) {
            high_digit = *source_digits++;
            *dest_digits++ = low_digit | ((high_digit << digit_lshift) & BIGDIGIT_MASK);
            low_digit = high_digit >> digit_rshift;
          } /* for */
          if ((memSizeType) (dest_digits - result->bigdigits) < result_size) {
            if (IS_NEGATIVE(high_digit)) {
              *dest_digits = low_digit | ((BIGDIGIT_MASK << digit_lshift) & BIGDIGIT_MASK);
            } else {
              *dest_digits = low_digit;
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* bigRShift */



/**
 *  Shift a number right by rshift bits and assign the result back to number.
 *  When rshift is negative a left shift is done instead.
 */
void bigRShiftAssign (bigIntType *const big_variable, intType rshift)

  {
    bigIntType big1;
    memSizeType size_reduction;
    unsigned int digit_rshift;
    unsigned int digit_lshift;
    bigDigitType low_digit;
    bigDigitType high_digit;
    const bigDigitType *source_digits;
    bigDigitType *dest_digits;
    bigIntType resized_big1;
    memSizeType pos;
    memSizeType big1_size;

  /* bigRShiftAssign */
    if (unlikely(rshift < 0)) {
      bigLShiftAssign(big_variable, -rshift);
    } else {
      big1 = *big_variable;
      if (big1->size <= (uintType) rshift >> BIGDIGIT_LOG2_SIZE) {
        if (unlikely(!ALLOC_BIG_SIZE_OK(*big_variable, 1))) {
          raise_error(MEMORY_ERROR);
        } else {
          (*big_variable)->size = 1;
          if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
            (*big_variable)->bigdigits[0] = BIGDIGIT_MASK;
          } else {
            (*big_variable)->bigdigits[0] = 0;
          } /* if */
          FREE_BIG(big1, big1->size);
        } /* if */
      } else {
        size_reduction = (memSizeType) ((uintType) rshift >> BIGDIGIT_LOG2_SIZE);
        if ((rshift & BIGDIGIT_SIZE_MASK) == 0) {
          if (rshift != 0) {
            big1_size = big1->size;
            memmove(big1->bigdigits, &big1->bigdigits[size_reduction],
                (size_t) (big1_size - size_reduction) * sizeof(bigDigitType));
            REALLOC_BIG_SIZE_OK(resized_big1, big1, big1_size, big1_size - size_reduction);
            /* Avoid a MEMORY_ERROR in the strange case     */
            /* when a 'realloc' which shrinks memory fails. */
            if (likely(resized_big1 != NULL)) {
              big1 = resized_big1;
              *big_variable = big1;
            } /* if */
            COUNT3_BIG(big1_size, big1_size - size_reduction);
            big1->size -= size_reduction;
          } /* if */
        } else {
          digit_rshift = (unsigned int) ((uintType) rshift & BIGDIGIT_SIZE_MASK);
          digit_lshift = BIGDIGIT_SIZE - digit_rshift;
          source_digits = &big1->bigdigits[size_reduction];
          dest_digits = big1->bigdigits;
          high_digit = *source_digits++;
          low_digit = high_digit >> digit_rshift;
          for (pos = big1->size - size_reduction - 1; pos != 0; pos--) {
            high_digit = *source_digits++;
            *dest_digits++ = low_digit | ((high_digit << digit_lshift) & BIGDIGIT_MASK);
            low_digit = high_digit >> digit_rshift;
          } /* for */
          if (IS_NEGATIVE(high_digit)) {
            *dest_digits = low_digit | ((BIGDIGIT_MASK << digit_lshift) & BIGDIGIT_MASK);
            if (*dest_digits == BIGDIGIT_MASK) {
              if (size_reduction == 0) {
                *big_variable = normalize(big1);
              } else {
                pos = big1->size - size_reduction;
                if (pos >= 2 && IS_NEGATIVE(big1->bigdigits[pos - 2])) {
                  pos--;
                } /* if */
                REALLOC_BIG_SIZE_OK(resized_big1, big1, big1->size, pos);
                /* Avoid a MEMORY_ERROR in the strange case     */
                /* when a 'realloc' which shrinks memory fails. */
                if (likely(resized_big1 != NULL)) {
                  big1 = resized_big1;
                  *big_variable = big1;
                } /* if */
                COUNT3_BIG(big1->size, pos);
                big1->size = pos;
                size_reduction = 0;
              } /* if */
            } /* if */
          } else {
            *dest_digits = low_digit;
            if (low_digit == 0) {
              if (size_reduction == 0) {
                *big_variable = normalize(big1);
              } else {
                pos = big1->size - size_reduction;
                if (pos >= 2 && !IS_NEGATIVE(big1->bigdigits[pos - 2])) {
                  pos--;
                } /* if */
                REALLOC_BIG_SIZE_OK(resized_big1, big1, big1->size, pos);
                /* Avoid a MEMORY_ERROR in the strange case     */
                /* when a 'realloc' which shrinks memory fails. */
                if (likely(resized_big1 != NULL)) {
                  big1 = resized_big1;
                  *big_variable = big1;
                } /* if */
                COUNT3_BIG(big1->size, pos);
                big1->size = pos;
                size_reduction = 0;
              } /* if */
            } /* if */
          } /* if */
          if (size_reduction != 0) {
            big1_size = big1->size;
            REALLOC_BIG_SIZE_OK(resized_big1, big1, big1_size, big1_size - size_reduction);
            /* Avoid a MEMORY_ERROR in the strange case     */
            /* when a 'realloc' which shrinks memory fails. */
            if (likely(resized_big1 != NULL)) {
              big1 = resized_big1;
              *big_variable = big1;
            } /* if */
            COUNT3_BIG(big1_size, big1_size - size_reduction);
            big1->size -= size_reduction;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* bigRShiftAssign */



/**
 *  Compute the subtraction of two 'bigInteger' numbers.
 *  @return the difference of the two numbers.
 */
bigIntType bigSbtr (const const_bigIntType minuend, const const_bigIntType subtrahend)

  {
    memSizeType pos;
    doubleBigDigitType carry = 1;
    doubleBigDigitType minuend_sign;
    doubleBigDigitType subtrahend_sign;
    bigIntType difference;

  /* bigSbtr */
    if (minuend->size >= subtrahend->size) {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(difference, minuend->size + 1))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        pos = 0;
        do {
          carry += (doubleBigDigitType) minuend->bigdigits[pos] +
              (~subtrahend->bigdigits[pos] & BIGDIGIT_MASK);
          difference->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          pos++;
        } while (pos < subtrahend->size);
        subtrahend_sign = IS_NEGATIVE(subtrahend->bigdigits[pos - 1]) ? 0 : BIGDIGIT_MASK;
        for (; pos < minuend->size; pos++) {
          carry += minuend->bigdigits[pos] + subtrahend_sign;
          difference->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        if (IS_NEGATIVE(minuend->bigdigits[pos - 1])) {
          subtrahend_sign--;
        } /* if */
        difference->bigdigits[pos] = (bigDigitType) ((carry + subtrahend_sign) & BIGDIGIT_MASK);
        difference->size = pos + 1;
        difference = normalize(difference);
        return difference;
      } /* if */
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(difference, subtrahend->size + 1))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        pos = 0;
        do {
          carry += (doubleBigDigitType) minuend->bigdigits[pos] +
              (~subtrahend->bigdigits[pos] & BIGDIGIT_MASK);
          difference->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          pos++;
        } while (pos < minuend->size);
        minuend_sign = IS_NEGATIVE(minuend->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
        for (; pos < subtrahend->size; pos++) {
          carry += minuend_sign + (~subtrahend->bigdigits[pos] & BIGDIGIT_MASK);
          difference->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        subtrahend_sign = IS_NEGATIVE(subtrahend->bigdigits[pos - 1]) ? 0 : BIGDIGIT_MASK;
        if (IS_NEGATIVE(minuend->bigdigits[minuend->size - 1])) {
          subtrahend_sign--;
        } /* if */
        difference->bigdigits[pos] = (bigDigitType) ((carry + subtrahend_sign) & BIGDIGIT_MASK);
        difference->size = pos + 1;
        difference = normalize(difference);
        return difference;
      } /* if */
    } /* if */
  } /* bigSbtr */



/**
 *  Compute the subtraction of two 'bigInteger' numbers.
 *  Minuend is assumed to be a temporary value which is reused.
 *  @return the difference of the two numbers in 'minuend'.
 */
bigIntType bigSbtrTemp (bigIntType minuend, const_bigIntType subtrahend)

  { /* bigSbtrTemp */
    bigShrink(&minuend, subtrahend);
    return minuend;
  } /* bigSbtrTemp */



/**
 *  Decrement a 'bigInteger' variable by a delta.
 *  Subtracts delta from *big_variable. The operation is done in
 *  place and *big_variable is only resized when necessary.
 *  When the size of delta is smaller than *big_variable the
 *  algorithm tries to save computations. Therefore there are
 *  checks for carry != 0 and carry == 0.
 *  In case the resizing fails the content of *big_variable
 *  is freed and *big_variable is set to NULL.
 */
void bigShrink (bigIntType *const big_variable, const const_bigIntType delta)

  {
    bigIntType big1;
    memSizeType pos;
    memSizeType big1_size;
    doubleBigDigitType carry = 1;
    doubleBigDigitType big1_sign;
    doubleBigDigitType delta_sign;
    bigIntType resized_big1;

  /* bigShrink */
    big1 = *big_variable;
    if (big1->size >= delta->size) {
      big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
      pos = 0;
      do {
        carry += (doubleBigDigitType) big1->bigdigits[pos] +
            (~delta->bigdigits[pos] & BIGDIGIT_MASK);
        big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < delta->size);
      if (IS_NEGATIVE(delta->bigdigits[pos - 1])) {
        for (; carry != 0 && pos < big1->size; pos++) {
          carry += big1->bigdigits[pos];
          big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
      } else {
        for (; carry == 0 && pos < big1->size; pos++) {
          carry = (doubleBigDigitType) big1->bigdigits[pos] + BIGDIGIT_MASK;
          big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        carry += BIGDIGIT_MASK;
      } /* if */
      big1_size = big1->size;
      carry += big1_sign;
      carry &= BIGDIGIT_MASK;
      if ((carry != 0 || IS_NEGATIVE(big1->bigdigits[big1_size - 1])) &&
          (carry != BIGDIGIT_MASK || !IS_NEGATIVE(big1->bigdigits[big1_size - 1]))) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, big1_size, big1_size + 1);
        if (unlikely(resized_big1 == NULL)) {
          FREE_BIG(big1, big1_size);
          *big_variable = NULL;
          raise_error(MEMORY_ERROR);
        } else {
          /* It is possible that big1 == delta holds. Since */
          /* 'delta' is not used after realloc() enlarged   */
          /* 'big1' a correction of delta is not necessary. */
          big1 = resized_big1;
          COUNT3_BIG(big1_size, big1_size + 1);
          big1->size++;
          big1->bigdigits[big1_size] = (bigDigitType) (carry & BIGDIGIT_MASK);
          *big_variable = big1;
        } /* if */
      } else {
        *big_variable = normalize(big1);
      } /* if */
    } else {
      REALLOC_BIG_CHECK_SIZE(resized_big1, big1, big1->size, delta->size + 1);
      if (unlikely(resized_big1 == NULL)) {
        FREE_BIG(big1, big1->size);
        *big_variable = NULL;
        raise_error(MEMORY_ERROR);
      } else {
        big1 = resized_big1;
        COUNT3_BIG(big1->size, delta->size + 1);
        big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
        pos = 0;
        do {
          carry += (doubleBigDigitType) big1->bigdigits[pos] +
              (~delta->bigdigits[pos] & BIGDIGIT_MASK);
          big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          pos++;
        } while (pos < big1->size);
        delta_sign = IS_NEGATIVE(delta->bigdigits[delta->size - 1]) ? 0 : BIGDIGIT_MASK;
        for (; pos < delta->size; pos++) {
          carry += big1_sign + (~delta->bigdigits[pos] & BIGDIGIT_MASK);
          big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        carry += big1_sign + delta_sign;
        big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        big1->size = pos + 1;
        *big_variable = normalize(big1);
      } /* if */
    } /* if */
  } /* bigShrink */



/**
 *  Compute the square of a 'bigInteger'.
 *  This function is used by the compiler to optimize
 *  multiplication and exponentiation operations.
 *  @return the square of big1.
 */
bigIntType bigSquare (const_bigIntType big1)

  {
    bigIntType big1_help = NULL;
    bigIntType result;

  /* bigSquare */
    if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
      big1_help = alloc_positive_copy_of_negative_big(big1);
      big1 = big1_help;
      if (unlikely(big1_help == NULL)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
    } /* if */
    /* printf("bigSquare(%u)\n", big1->size); */
    result = uBigSquareK(big1);
    if (big1_help != NULL) {
      FREE_BIG(big1_help, big1_help->size);
    } /* if */
    return result;
  } /* bigSquare */



/**
 *  Convert a 'bigInteger' number to a string.
 *  The number is converted to a string with decimal representation.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
striType bigStr (const const_bigIntType big1)

  {
    bigIntType unsigned_big;
    memSizeType pos;
    bigDigitType digit;
    int digit_pos;
    memSizeType result_size;
    memSizeType final_result_size;
    striType resized_result;
    striType result;

  /* bigStr */
    if (unlikely((MAX_STRI_LEN <= (MAX_MEMSIZETYPE - 1) / 3 + 2 &&
                 big1->size > ((MAX_STRI_LEN - 2) * 3 + 1) / BIGDIGIT_SIZE) ||
                 big1->size > MAX_MEMSIZETYPE / BIGDIGIT_SIZE)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      /* The size of the result is estimated by computing the    */
      /* number of octal digits plus one character for the sign. */
      result_size = (big1->size * BIGDIGIT_SIZE - 1) / 3 + 2;
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_BIG_CHECK_SIZE(unsigned_big, big1->size + 1))) {
          FREE_STRI(result, result_size);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
            positive_copy_of_negative_big(unsigned_big, big1);
          } else {
            unsigned_big->size = big1->size;
            memcpy(unsigned_big->bigdigits, big1->bigdigits,
                (size_t) big1->size * sizeof(bigDigitType));
          } /* if */
          pos = result_size - 1;
          do {
            digit = uBigDivideByPowerOf10(unsigned_big);
            /* printf("unsigned_big->size=%lu, digit=%lu\n", unsigned_big->size, digit); */
            if (unsigned_big->bigdigits[unsigned_big->size - 1] == 0) {
              unsigned_big->size--;
            } /* if */
            if (unsigned_big->size > 1 || unsigned_big->bigdigits[0] != 0) {
              for (digit_pos = DECIMAL_DIGITS_IN_BIGDIGIT;
                  digit_pos != 0; digit_pos--) {
                result->mem[pos] = '0' + digit % 10;
                digit /= 10;
                pos--;
              } /* for */
            } else {
              do {
                result->mem[pos] = '0' + digit % 10;
                digit /= 10;
                pos--;
              } while (digit != 0);
            } /* if */
          } while (unsigned_big->size > 1 || unsigned_big->bigdigits[0] != 0);
          FREE_BIG(unsigned_big, big1->size + 1);
          pos++;
          if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
            final_result_size = result_size - pos + 1;
            result->mem[0] = '-';
            memmove(&result->mem[1], &result->mem[pos],
                (result_size - pos) * sizeof(strElemType));
          } else {
            final_result_size = result_size - pos;
            memmove(&result->mem[0], &result->mem[pos],
                (result_size - pos) * sizeof(strElemType));
          } /* if */
          result->size = final_result_size;
          if (final_result_size < result_size) {
            REALLOC_STRI_SIZE_SMALLER(resized_result, result, result_size, final_result_size);
            if (unlikely(resized_result == NULL)) {
              FREE_STRI(result, result_size);
              raise_error(MEMORY_ERROR);
              result = NULL;
            } else {
              result = resized_result;
              COUNT3_STRI(result_size, final_result_size);
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* bigStr */



/**
 *  Successor of a 'bigInteger' number.
 *  succ(A) is equivalent to A+1 .
 *  @return big1 + 1 .
 */
bigIntType bigSucc (const const_bigIntType big1)

  {
    memSizeType pos;
    bigIntType resized_result;
    bigIntType result;

  /* bigSucc */
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, big1->size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = big1->size;
      pos = 0;
      if (big1->bigdigits[pos] == BIGDIGIT_MASK) {
        if (big1->size == 1) {
          result->bigdigits[pos] = 0;
          pos++;
        } else {
          do {
            result->bigdigits[pos] = 0;
            pos++;
          } while (big1->bigdigits[pos] == BIGDIGIT_MASK);
          /* memset(result->bigdigits, 0, pos * sizeof(bigDigitType)); */
        } /* if */
      } /* if */
      if (pos < big1->size) {
        result->bigdigits[pos] = big1->bigdigits[pos] + 1;
        pos++;
        memcpy(&result->bigdigits[pos], &big1->bigdigits[pos],
            (big1->size - pos) * sizeof(bigDigitType));
        pos = big1->size;
        /* while (pos < big1->size) {
          result->bigdigits[pos] = big1->bigdigits[pos];
          pos++;
        } ** while */
      } /* if */
      if (IS_NEGATIVE(result->bigdigits[pos - 1])) {
        if (!IS_NEGATIVE(big1->bigdigits[pos - 1])) {
          REALLOC_BIG_CHECK_SIZE(resized_result, result, pos, pos + 1);
          if (unlikely(resized_result == NULL)) {
            FREE_BIG(result, pos);
            raise_error(MEMORY_ERROR);
            return NULL;
          } else {
            result = resized_result;
            COUNT3_BIG(pos, pos + 1);
            result->size++;
            result->bigdigits[pos] = 0;
          } /* if */
        } else if (result->bigdigits[pos - 1] == BIGDIGIT_MASK &&
            pos >= 2 && IS_NEGATIVE(result->bigdigits[pos - 2])) {
          /* Avoid a MEMORY_ERROR in the strange case     */
          /* when a 'realloc' which shrinks memory fails. */
          REALLOC_BIG_SIZE_OK(resized_result, result, pos, pos - 1);
          if (likely(resized_result != NULL)) {
            result = resized_result;
          } /* if */
          COUNT3_BIG(pos, pos - 1);
          result->size--;
        } /* if */
      } /* if */
      return result;
    } /* if */
  } /* bigSucc */



/**
 *  Successor of a 'bigInteger' number.
 *  Big1 is assumed to be a temporary value which is reused.
 *  @return big1 + 1 .
 */
bigIntType bigSuccTemp (bigIntType big1)

  {
    memSizeType pos;
    boolType negative;
    bigIntType resized_big1;

  /* bigSuccTemp */
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    pos = 0;
    if (big1->bigdigits[pos] == BIGDIGIT_MASK) {
      if (big1->size == 1) {
        big1->bigdigits[pos] = 0;
        pos++;
      } else {
        do {
          big1->bigdigits[pos] = 0;
          pos++;
        } while (big1->bigdigits[pos] == BIGDIGIT_MASK);
        /* memset(big1->bigdigits, 0, pos * sizeof(bigDigitType)); */
      } /* if */
    } /* if */
    if (pos < big1->size) {
      big1->bigdigits[pos]++;
    } /* if */
    pos = big1->size;
    if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      if (!negative) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, pos, pos + 1);
        if (unlikely(resized_big1 == NULL)) {
          FREE_BIG(big1, big1->size);
          raise_error(MEMORY_ERROR);
          big1 = NULL;
        } else {
          big1 = resized_big1;
          COUNT3_BIG(pos, pos + 1);
          big1->size++;
          big1->bigdigits[pos] = 0;
        } /* if */
      } else if (big1->bigdigits[pos - 1] == BIGDIGIT_MASK &&
          pos >= 2 && IS_NEGATIVE(big1->bigdigits[pos - 2])) {
        REALLOC_BIG_SIZE_OK(resized_big1, big1, pos, pos - 1);
        /* Avoid a MEMORY_ERROR in the strange case     */
        /* when a 'realloc' which shrinks memory fails. */
        if (likely(resized_big1 != NULL)) {
          big1 = resized_big1;
        } /* if */
        COUNT3_BIG(pos, pos - 1);
        big1->size--;
      } /* if */
    } /* if */
    return big1;
  } /* bigSuccTemp */



/**
 *  Convert a 'bigInteger' into a big-endian 'bstring'.
 *  The result uses binary representation with a base of 256.
 *  @param big1 BigInteger number to be converted.
 *  @param isSigned Determines the signedness of the result.
 *         When 'isSigned' is TRUE the result is encoded with the
 *         twos-complement representation. In this case a negative
 *         'big1' is converted to a result where the most significant
 *         byte (the first byte) has an ordinal >= 128.
 *  @return a bstring with the big-endian representation.
 *  @exception RANGE_ERROR When 'isSigned' is FALSE and 'big1' is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bstriType bigToBStriBe (const const_bigIntType big1, const boolType isSigned)

  {
    memSizeType pos;
    int byteNum;
    bigDigitType digit;
    memSizeType charIndex;
    memSizeType result_size;
    bstriType result;

  /* bigToBStriBe */
    /* printf("begin bigToBStriBe(%s, %d)\n", bigHexCStri(big1), isSigned); */
    /* The expression computing result_size does not overflow           */
    /* because the number of bytes in a bigInteger fits in memSizeType. */
    result_size = big1->size * (BIGDIGIT_SIZE >> 3);
    pos = big1->size - 1;
    digit = big1->bigdigits[pos];
    byteNum = (BIGDIGIT_SIZE >> 3) - 1;
    if (isSigned) {
      if (IS_NEGATIVE(digit)) {
        while (byteNum > 0 && (digit >> byteNum * 8 & 0xFF) == 0xFF) {
          result_size--;
          byteNum--;
        } /* while */
        if (byteNum < 3 && (digit >> byteNum * 8 & 0xFF) <= 127) {
          result_size++;
          byteNum++;
        } /* if */
      } else {
        while (byteNum > 0 && (digit >> byteNum * 8 & 0xFF) == 0) {
          result_size--;
          byteNum--;
        } /* while */
        if (byteNum < 3 && (digit >> byteNum * 8 & 0xFF) >= 128) {
          result_size++;
          byteNum++;
        } /* if */
      } /* if */
    } else {
      if (unlikely(IS_NEGATIVE(digit))) {
        /* printf("bigToBStriBe: RANGE_ERROR\n"); */
        raise_error(RANGE_ERROR);
        return NULL;
      } else {
        if (digit == 0 && pos > 0) {
          result_size -= (BIGDIGIT_SIZE >> 3);
          pos--;
          digit = big1->bigdigits[pos];
        } /* if */
        while (byteNum > 0 && (digit >> byteNum * 8 & 0xFF) == 0) {
          result_size--;
          byteNum--;
        } /* while */
      } /* if */
    } /* if */
    if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
      charIndex = 0;
      for (; byteNum >= 0; byteNum--) {
        result->mem[charIndex] = (ucharType) (digit >> byteNum * 8 & 0xFF);
        charIndex++;
      } /* for */
      while (pos > 0) {
        pos--;
        digit = big1->bigdigits[pos];
        for (byteNum = (BIGDIGIT_SIZE >> 3) - 1; byteNum >= 0; byteNum--) {
          result->mem[charIndex] = (ucharType) (digit >> byteNum * 8 & 0xFF);
          charIndex++;
        } /* for */
      } /* while */
    } /* if */
    /* printf("end bigToBStriBe\n"); */
    return result;
  } /* bigToBStriBe */



/**
 *  Convert a 'bigInteger' into a little-endian 'bstring'.
 *  The result uses binary representation with a base of 256.
 *  @param big1 BigInteger number to be converted.
 *  @param isSigned Determines the signedness of the result.
 *         When 'isSigned' is TRUE the result is encoded with the
 *         twos-complement representation. In this case a negative
 *         'big1' is converted to a result where the most significant
 *         byte (the last byte) has an ordinal >= 128.
 *  @return a bstring with the little-endian representation.
 *  @exception RANGE_ERROR When 'isSigned' is FALSE and 'big1' is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bstriType bigToBStriLe (const const_bigIntType big1, const boolType isSigned)

  {
    memSizeType pos;
    int byteNum;
    bigDigitType digit;
    memSizeType charIndex;
    memSizeType result_size;
    bstriType result;

  /* bigToBStriLe */
    /* printf("begin bigToBStriLe(%s, %d)\n", bigHexCStri(big1), isSigned); */
    /* The expression computing result_size does not overflow           */
    /* because the number of bytes in a bigInteger fits in memSizeType. */
    result_size = big1->size * (BIGDIGIT_SIZE >> 3);
    pos = big1->size - 1;
    digit = big1->bigdigits[pos];
    byteNum = (BIGDIGIT_SIZE >> 3) - 1;
    if (isSigned) {
      if (IS_NEGATIVE(digit)) {
        while (byteNum > 0 && (digit >> byteNum * 8 & 0xFF) == 0xFF) {
          result_size--;
          byteNum--;
        } /* while */
        if (byteNum < 3 && (digit >> byteNum * 8 & 0xFF) <= 127) {
          result_size++;
          byteNum++;
        } /* if */
      } else {
        while (byteNum > 0 && (digit >> byteNum * 8 & 0xFF) == 0) {
          result_size--;
          byteNum--;
        } /* while */
        if (byteNum < 3 && (digit >> byteNum * 8 & 0xFF) >= 128) {
          result_size++;
          byteNum++;
        } /* if */
      } /* if */
    } else {
      if (unlikely(IS_NEGATIVE(digit))) {
        /* printf("bigToBStriLe: RANGE_ERROR\n"); */
        raise_error(RANGE_ERROR);
        return NULL;
      } else {
        if (digit == 0 && pos > 0) {
          result_size -= (BIGDIGIT_SIZE >> 3);
          pos--;
          digit = big1->bigdigits[pos];
        } /* if */
        while (byteNum > 0 && (digit >> byteNum * 8 & 0xFF) == 0) {
          result_size--;
          byteNum--;
        } /* while */
      } /* if */
    } /* if */
    if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
      charIndex = result_size - 1;
      for (; byteNum >= 0; byteNum--) {
        result->mem[charIndex] = (ucharType) (digit >> byteNum * 8 & 0xFF);
        charIndex--;
      } /* for */
      while (pos > 0) {
        pos--;
        digit = big1->bigdigits[pos];
        for (byteNum = (BIGDIGIT_SIZE >> 3) - 1; byteNum >= 0; byteNum--) {
          result->mem[charIndex] = (ucharType) (digit >> byteNum * 8 & 0xFF);
          charIndex--;
        } /* for */
      } /* while */
    } /* if */
    /* printf("end bigToBStriLe\n"); */
    return result;
  } /* bigToBStriLe */



int32Type bigToInt32 (const const_bigIntType big1)

  {
    memSizeType pos;
    int32Type result;

  /* bigToInt32 */
    if (unlikely(big1->size > sizeof(int32Type) / (BIGDIGIT_SIZE >> 3))) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      pos = big1->size - 1;
      result = (int32Type) (signedBigDigitType) big1->bigdigits[pos];
#if BIGDIGIT_SIZE < 32
      while (pos > 0) {
        pos--;
        result <<= BIGDIGIT_SIZE;
        result |= (int32Type) big1->bigdigits[pos];
      } /* while */
#endif
      return result;
    } /* if */
  } /* bigToInt32 */



#ifdef INT64TYPE
int64Type bigToInt64 (const const_bigIntType big1)

  {
    memSizeType pos;
    int64Type result;

  /* bigToInt64 */
    if (unlikely(big1->size > sizeof(int64Type) / (BIGDIGIT_SIZE >> 3))) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      pos = big1->size - 1;
      result = (int64Type) (signedBigDigitType) big1->bigdigits[pos];
#if BIGDIGIT_SIZE < 64
      while (pos > 0) {
        pos--;
        result <<= BIGDIGIT_SIZE;
        result |= (int64Type) big1->bigdigits[pos];
      } /* while */
#endif
      return result;
    } /* if */
  } /* bigToInt64 */
#endif



bigIntType bigXor (const_bigIntType big1, const_bigIntType big2)

  {
    const_bigIntType help_big;
    memSizeType pos;
    bigDigitType big2_sign;
    bigIntType result;

  /* bigXor */
    if (big2->size > big1->size) {
      help_big = big1;
      big1 = big2;
      big2 = help_big;
    } /* if */
    if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, big1->size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      pos = 0;
      do {
        result->bigdigits[pos] = big1->bigdigits[pos] ^ big2->bigdigits[pos];
        pos++;
      } while (pos < big2->size);
      big2_sign = IS_NEGATIVE(big2->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
      for (; pos < big1->size; pos++) {
        result->bigdigits[pos] = big1->bigdigits[pos] ^ big2_sign;
      } /* for */
      result->size = pos;
      result = normalize(result);
      return result;
    } /* if */
  } /* bigXor */



bigIntType bigZero (void)

  {
    bigIntType result;

  /* bigZero */
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = 1;
      result->bigdigits[0] = 0;
    } /* if */
    return result;
  } /* bigZero */
