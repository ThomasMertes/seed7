/********************************************************************/
/*                                                                  */
/*  big_rtl.c     Functions for the built-in bigInteger support.    */
/*  Copyright (C) 2006 - 2020, 2022 - 2025  Thomas Mertes           */
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
/*  Changes: 2006 - 2020, 2022 - 2025  Thomas Mertes                */
/*  Content: Functions for the built-in bigInteger support.         */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#if BIGINT_LIBRARY == BIG_RTL_LIBRARY
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "int_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "big_drv.h"


#define KARATSUBA_MULT_THRESHOLD 32
#define KARATSUBA_SQUARE_THRESHOLD 32
#define OCTAL_DIGIT_BITS 3


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
typedef int16Type                signedDoubleBigDigitType;
#define digitMostSignificantBit  uint8MostSignificantBit
#define digitLeastSignificantBit uint8LeastSignificantBit
#define BIGDIGIT_MASK                    0xFF
#define BIGDIGIT_SIGN                    0x80
#define BIGDIGIT_SIZE_MASK                0x7
#define BIGDIGIT_LOG2_SIZE                  3
#define POWER_OF_10_IN_BIGDIGIT           100
#define DECIMAL_DIGITS_IN_BIGDIGIT          2
#define POWER_OF_5_IN_BIGDIGIT            125
#define QUINARY_DIGITS_IN_BIGDIGIT          3
#define F_D_DIG(width) F_D8(width)
#define F_U_DIG(width) F_U8(width)
#define F_X_DIG(width) F_X8(width)
#define FMT_D_DIG FMT_D8
#define FMT_U_DIG FMT_U8
#define FMT_X_DIG FMT_X8
#define F_D_DIG2(width) F_D16(width)
#define F_U_DIG2(width) F_U16(width)
#define F_X_DIG2(width) F_X16(width)
#define FMT_D_DIG2 FMT_D16
#define FMT_U_DIG2 FMT_U16
#define FMT_X_DIG2 FMT_X16
static const bigDigitType powerOfRadixInBigdigit[] = {
    /*  2 */ 128, 243,  64, 125, 216,
    /*  7 */  49,  64,  81, 100, 121,
    /* 12 */ 144, 169, 196, 225,  16,
    /* 17 */  17,  18,  19,  20,  21,
    /* 22 */  22,  23,  24,  25,  26,
    /* 27 */  27,  28,  29,  30,  31,
    /* 32 */  32,  33,  34,  35,  36
  };
static const uint8Type radixDigitsInBigdigit[] = {
    /*  2 */  7,  5,  3,  3,  3,  2,  2,  2,  2,  2,
    /* 12 */  2,  2,  2,  2,  1,  1,  1,  1,  1,  1,
    /* 22 */  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    /* 32 */  1,  1,  1,  1,  1
  };

#elif BIGDIGIT_SIZE == 16

/* typedef uint16Type            bigDigitType; */
typedef int16Type                signedBigDigitType;
typedef uint32Type               doubleBigDigitType;
typedef int32Type                signedDoubleBigDigitType;
#define digitMostSignificantBit  uint16MostSignificantBit
#define digitLeastSignificantBit uint16LeastSignificantBit
#define BIGDIGIT_MASK                  0xFFFF
#define BIGDIGIT_SIGN                  0x8000
#define BIGDIGIT_SIZE_MASK                0xF
#define BIGDIGIT_LOG2_SIZE                  4
#define POWER_OF_10_IN_BIGDIGIT         10000
#define DECIMAL_DIGITS_IN_BIGDIGIT          4
#define POWER_OF_5_IN_BIGDIGIT          15625
#define QUINARY_DIGITS_IN_BIGDIGIT          6
#define F_D_DIG(width) F_D16(width)
#define F_U_DIG(width) F_U16(width)
#define F_X_DIG(width) F_X16(width)
#define FMT_D_DIG FMT_D16
#define FMT_U_DIG FMT_U16
#define FMT_X_DIG FMT_X16
#define F_D_DIG2(width) F_D32(width)
#define F_U_DIG2(width) F_U32(width)
#define F_X_DIG2(width) F_X32(width)
#define FMT_D_DIG2 FMT_D32
#define FMT_U_DIG2 FMT_U32
#define FMT_X_DIG2 FMT_X32
static const bigDigitType powerOfRadixInBigdigit[] = {
    /*  2 */ 32768, 59049, 16384, 15625, 46656,
    /*  7 */ 16807, 32768, 59049, 10000, 14641,
    /* 12 */ 20736, 28561, 38416, 50625,  4096,
    /* 17 */  4913,  5832,  6859,  8000,  9261,
    /* 22 */ 10648, 12167, 13824, 15625, 17576,
    /* 27 */ 19683, 21952, 24389, 27000, 29791,
    /* 32 */ 32768, 35937, 39304, 42875, 46656
  };
static const uint8Type radixDigitsInBigdigit[] = {
    /*  2 */ 15, 10,  7,  6,  6,  5,  5,  5,  4,  4,
    /* 12 */  4,  4,  4,  4,  3,  3,  3,  3,  3,  3,
    /* 22 */  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
    /* 32 */  3,  3,  3,  3,  3
  };

#elif BIGDIGIT_SIZE == 32

/* typedef uint32Type            bigDigitType; */
typedef int32Type                signedBigDigitType;
typedef uint64Type               doubleBigDigitType;
typedef int64Type                signedDoubleBigDigitType;
#define digitMostSignificantBit  uint32MostSignificantBit
#define digitLeastSignificantBit uint32LeastSignificantBit
#define BIGDIGIT_MASK              0xFFFFFFFF
#define BIGDIGIT_SIGN              0x80000000
#define BIGDIGIT_SIZE_MASK               0x1F
#define BIGDIGIT_LOG2_SIZE                  5
#define POWER_OF_10_IN_BIGDIGIT    1000000000
#define DECIMAL_DIGITS_IN_BIGDIGIT          9
#define POWER_OF_5_IN_BIGDIGIT     1220703125
#define QUINARY_DIGITS_IN_BIGDIGIT         13
#define F_D_DIG(width) F_D32(width)
#define F_U_DIG(width) F_U32(width)
#define F_X_DIG(width) F_X32(width)
#define FMT_D_DIG FMT_D32
#define FMT_U_DIG FMT_U32
#define FMT_X_DIG FMT_X32
#define F_D_DIG2(width) F_D64(width)
#define F_U_DIG2(width) F_U64(width)
#define F_X_DIG2(width) F_X64(width)
#define FMT_D_DIG2 FMT_D64
#define FMT_U_DIG2 FMT_U64
#define FMT_X_DIG2 FMT_X64
static const bigDigitType powerOfRadixInBigdigit[] = {
    /*  2 */ 2147483648u, 3486784401u, 1073741824u, 1220703125u, 2176782336u,
    /*  7 */ 1977326743u, 1073741824u, 3486784401u, 1000000000u, 2357947691u,
    /* 12 */  429981696u,  815730721u, 1475789056u, 2562890625u,  268435456u,
    /* 17 */  410338673u,  612220032u,  893871739u, 1280000000u, 1801088541u,
    /* 22 */ 2494357888u, 3404825447u,  191102976u,  244140625u,  308915776u,
    /* 27 */  387420489u,  481890304u,  594823321u,  729000000u,  887503681u,
    /* 32 */ 1073741824u, 1291467969u, 1544804416u, 1838265625u, 2176782336u
  };
static const uint8Type radixDigitsInBigdigit[] = {
    /*  2 */ 31, 20, 15, 13, 12, 11, 10, 10,  9,  9,
    /* 12 */  8,  8,  8,  8,  7,  7,  7,  7,  7,  7,
    /* 22 */  7,  7,  6,  6,  6,  6,  6,  6,  6,  6,
    /* 32 */  6,  6,  6,  6,  6
  };

#endif


bigIntType *conversionDivisorCache[] = {
    /*  0 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 10 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 20 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 30 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL
  };

unsigned int conversionDivisorCacheSize[] = {
    /*  0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 20 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };


#define IS_NEGATIVE(digit) (((digit) & BIGDIGIT_SIGN) != 0)

static const unsigned int digit_value[] = {
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9,              /* Digits 0 - 9   */
    36, 36, 36, 36, 36, 36, 36,                          /* Illegal digits */
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,  /* Digits A - M */
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,  /* Digits N - Z */
    36, 36, 36, 36, 36, 36,                              /* Illegal digits */
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,  /* Digits a - m */
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35   /* Digits n - z */
  };


#if DO_HEAP_STATISTIC || DO_HEAPSIZE_COMPUTATION || DO_HEAP_CHECK
const size_t sizeof_bigDigitType = sizeof(bigDigitType);
const size_t sizeof_bigIntRecord = sizeof(bigIntRecord);
#endif


#define SIZ_RTLBIG(len)  ((sizeof(bigIntRecord) - sizeof(bigDigitType)) + \
                         (len) * sizeof(bigDigitType))
#define MAX_BIG_LEN      ((MAX_MEMSIZETYPE - sizeof(bigIntRecord) + \
                         sizeof(bigDigitType)) / sizeof(bigDigitType))

#define COUNT3_BIG(len1,len2)    CNT3(CNT2_BIG(len1, SIZ_RTLBIG(len1)), \
                                 CNT1_BIG(len2, SIZ_RTLBIG(len2)))

#if WITH_BIGINT_CAPACITY
#define HEAP_ALLOC_BIG(var,cap)  (ALLOC_HEAP(var, bigIntType, SIZ_RTLBIG(cap))? \
                                 ((var)->capacity = (cap), CNT(CNT1_BIG(cap, SIZ_RTLBIG(cap))) TRUE): \
                                 FALSE)
#define HEAP_FREE_BIG(var,cap)   (CNT(CNT2_BIG(cap, SIZ_RTLBIG(cap))) \
                                 FREE_HEAP(var, SIZ_RTLBIG(cap)))
#define HEAP_REALLOC_BIG(v1,v2,cap) if((v1=REALLOC_HEAP(v2, bigIntType, SIZ_RTLBIG(cap)))!=NULL) \
                                      {COUNT3_BIG((v1)->capacity,cap); (v1)->capacity = (cap);}
#else
#define HEAP_ALLOC_BIG(var,len)  (ALLOC_HEAP(var, bigIntType, SIZ_RTLBIG(len))? \
                                 CNT(CNT1_BIG(len, SIZ_RTLBIG(len))) TRUE:FALSE)
#define HEAP_FREE_BIG(var,len)   (CNT(CNT2_BIG(len, SIZ_RTLBIG(len))) \
                                 FREE_HEAP(var, SIZ_RTLBIG(len)))
#define HEAP_REALLOC_BIG(v1,v2,len) if((v1=REALLOC_HEAP(v2, bigIntType, SIZ_RTLBIG(len)))!=NULL) { \
                                      {COUNT3_BIG((v1)->size,len);}
#endif

#if WITH_BIGINT_FREELIST
#if WITH_BIGINT_CAPACITY

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
#define PUSH_BIG_OK(var)   (var)->capacity < BIG_FREELIST_ARRAY_SIZE && \
                           flist_allowed[(var)->capacity] > 0

#define POP_BIG(var,len)   (var = (bigIntType) flist[len], flist[len] = flist[len]->next, \
                           flist_allowed[len]++, TRUE)
#define PUSH_BIG(var,len)  {((freeListElemType) (var))->next = flist[len]; \
                           flist[len] = (freeListElemType) (var); flist_allowed[len]--; }

#define ALLOC_BIG_SIZE_OK(var,len)    (POP_BIG_OK(len) ? POP_BIG(var, len) : \
                                      HEAP_ALLOC_BIG(var, len))
#define ALLOC_BIG_CHECK_SIZE(var,len) (POP_BIG_OK(len) ? POP_BIG(var, len) : \
                                      ((len)<=MAX_BIG_LEN?HEAP_ALLOC_BIG(var, len): \
                                      (var=NULL, FALSE)))
#define FREE_BIG2(var,unused)  if (PUSH_BIG_OK(var)) PUSH_BIG(var, (var)->capacity) else \
                               HEAP_FREE_BIG(var, (var)->capacity);
#define FREE_BIG(var)          FREE_BIG2(var, **not-used**)
#else

static freeListElemType flist = NULL;
static unsigned int flist_allowed = 100;

#define POP_BIG_OK(len)    (len) == 1 && flist != NULL
#define PUSH_BIG_OK(len)   (len) == 1 && flist_allowed > 0

#define POP_BIG(var)       (var = (bigIntType) flist, flist = flist->next, flist_allowed++, TRUE)
#define PUSH_BIG(var)      {((freeListElemType) var)->next = flist; \
                           flist = (freeListElemType) var; flist_allowed--; }

#define ALLOC_BIG_SIZE_OK(var,len)    (POP_BIG_OK(len) ? POP_BIG(var) : HEAP_ALLOC_BIG(var, len))
#define ALLOC_BIG_CHECK_SIZE(var,len) (POP_BIG_OK(len) ? POP_BIG(var) : \
                                      ((len) <= MAX_BIG_LEN?HEAP_ALLOC_BIG(var, len): \
                                      (var=NULL, FALSE)))
#define FREE_BIG2(var,len)  if (PUSH_BIG_OK(len)) PUSH_BIG(var) else HEAP_FREE_BIG(var, len);
#define FREE_BIG(var)       FREE_BIG2(var, (var)->size)

#endif
#else

#define ALLOC_BIG_SIZE_OK(var,len)    HEAP_ALLOC_BIG(var, len)
#define ALLOC_BIG_CHECK_SIZE(var,len) ((len) <= MAX_BIG_LEN?HEAP_ALLOC_BIG(var, len): \
                                      (var=NULL, FALSE))
#define FREE_BIG2(var,len)            HEAP_FREE_BIG(var, len)

#if WITH_BIGINT_CAPACITY
#define FREE_BIG(var)       FREE_BIG2(var, (var)->capacity)
#else
#define FREE_BIG(var)       FREE_BIG2(var, (var)->size)
#endif

#endif

#define ALLOC_BIG(var,len)                ALLOC_BIG_CHECK_SIZE(var, len)
#define REALLOC_BIG_SIZE_OK(v1,v2,len)    HEAP_REALLOC_BIG(v1,v2,len)
#define REALLOC_BIG_CHECK_SIZE(v1,v2,len) if((len) <= MAX_BIG_LEN) \
                                            {HEAP_REALLOC_BIG(v1,v2,len)}else v1=NULL;


void bigDestr (const const_bigIntType old_bigint);
void bigAddAssign (bigIntType *const big_variable, const const_bigIntType delta);
intType bigLowestSetBit (const const_bigIntType big1);
void bigLShiftAssign (bigIntType *const big_variable, intType lshift);
bigIntType bigRem (const const_bigIntType dividend, const const_bigIntType divisor);
bigIntType bigRShift (const const_bigIntType big1, const intType rshift);
void bigRShiftAssign (bigIntType *const big_variable, intType rshift);
bigIntType bigSbtr (const const_bigIntType minuend, const const_bigIntType subtrahend);
void bigSbtrAssign (bigIntType *const big_variable, const const_bigIntType big2);
striType bigStr (const const_bigIntType big1);



/**
 *  Setup bigInteger computations.
 *  This function must be called before doing any bigInteger computations.
 */
void setupBig (void)

  { /* setupBig */
  } /* setupBig */



void closeBig (void)

  {
    unsigned int base;
    unsigned int pos;

  /* closeBig */
    for (base = 0; base <= 36; base++) {
      for (pos = 0; pos < conversionDivisorCacheSize[base]; pos++) {
        bigDestr(conversionDivisorCache[base][pos]);
      } /* for */
    } /* for */
  } /* closeBig */



#if (DO_HEAPSIZE_COMPUTATION || DO_HEAP_STATISTIC) && WITH_BIGINT_FREELIST
#if WITH_BIGINT_CAPACITY
unsigned long bigFListCount (memSizeType *bigDigitCountAddr)

  {
    unsigned int len;
    register freeListElemType flistElem;
    register memSizeType bigDigitCount = 0;
    register unsigned long numBigInts = 0;

  /* bigFListCount */
    for (len = 0; len < BIG_FREELIST_ARRAY_SIZE; len++) {
      flistElem = flist[len];
      while (flistElem != NULL) {
        flistElem = flistElem->next;
        bigDigitCount += len;
        numBigInts++;
      } /* while */
    } /* for */
    *bigDigitCountAddr = bigDigitCount;
    return numBigInts;
  } /* bigFListCount */

#else



unsigned long bigFListCount (memSizeType *bigDigitCountAddr)

  {
    register freeListElemType flistElem;
    register unsigned long numBigInts = 0;

  /* bigFListCount */
    flistElem = flist;
    while (flistElem != NULL) {
      flistElem = flistElem->next;
      numBigInts++;
    } /* while */
    *bigDigitCountAddr = (memSizeType) numBigInts;
    return numBigInts;
  } /* bigFListCount */
#endif
#endif



cstriType bigHexCStri (const const_bigIntType big1)

  {
    /* Length of the hex prefix (16#): */
    const memSizeType hexPrefixLen = STRLEN("16#");
    /* Number of hex digits in a byte: */
    const memSizeType hexDigitsInByte = 2;
    memSizeType pos;
    memSizeType digitPos;
    memSizeType len;
    const_cstriType stri_ptr;
    cstriType buffer;
    cstriType result;

  /* bigHexCStri */
    if (likely(big1 != NULL && big1->size > 0)) {
      if (unlikely(big1->size > (MAX_CSTRI_LEN - hexPrefixLen) /
                                hexDigitsInByte / (BIGDIGIT_SIZE >> 3) ||
                   !ALLOC_CSTRI(result, big1->size * (BIGDIGIT_SIZE >> 3) *
                                hexDigitsInByte + hexPrefixLen))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        buffer = result;
        memcpy(buffer, "16#", hexPrefixLen);
        buffer += hexPrefixLen;
        pos = big1->size - 1;
#if BIGDIGIT_SIZE == 8
        sprintf(buffer, F_X_DIG(02), big1->bigdigits[pos]);
#elif BIGDIGIT_SIZE == 16
        sprintf(buffer, F_X_DIG(04), big1->bigdigits[pos]);
#elif BIGDIGIT_SIZE == 32
        sprintf(buffer, F_X_DIG(08), big1->bigdigits[pos]);
#endif
        digitPos = 0;
        if (IS_NEGATIVE(big1->bigdigits[pos])) {
          while (digitPos < BIGDIGIT_SIZE >> 2 &&
                 memcmp(&buffer[digitPos], "ff", 2) == 0 &&
                 ((buffer[digitPos + 2] >= '8' && buffer[digitPos + 2] <= '9') ||
                  (buffer[digitPos + 2] >= 'a' && buffer[digitPos + 2] <= 'f'))) {
            digitPos += 2;
          } /* while */
        } else {
          while (digitPos < BIGDIGIT_SIZE >> 2 &&
                 memcmp(&buffer[digitPos], "00", 2) == 0 &&
                 buffer[digitPos + 2] >= '0' && buffer[digitPos + 2] <= '7') {
            digitPos += 2;
          } /* while */
        } /* if */
        len = (BIGDIGIT_SIZE >> 2) - digitPos;
        memmove(buffer, &buffer[digitPos], len + 1);
        buffer += len;
        while (pos > 0) {
          pos--;
#if BIGDIGIT_SIZE == 8
          sprintf(buffer, F_X_DIG(02), big1->bigdigits[pos]);
#elif BIGDIGIT_SIZE == 16
          sprintf(buffer, F_X_DIG(04), big1->bigdigits[pos]);
#elif BIGDIGIT_SIZE == 32
          sprintf(buffer, F_X_DIG(08), big1->bigdigits[pos]);
#endif
          buffer += (BIGDIGIT_SIZE >> 3) * hexDigitsInByte;
        } /* while */
      } /* if */
    } else {
      if (big1 == NULL) {
        stri_ptr = " *NULL_BIGINT* ";
        len = STRLEN(" *NULL_BIGINT* ");
      } else {
        stri_ptr = " *ZERO_SIZE_BIGINT* ";
        len = STRLEN(" *ZERO_SIZE_BIGINT* ");
      } /* if */
      if (unlikely(!ALLOC_CSTRI(result, len))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        memcpy(result, stri_ptr, len);
        result[len] = '\0';
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

        REALLOC_BIG_SIZE_OK(resized_big1, big1, pos);
        /* Avoid a MEMORY_ERROR in the strange case   */
        /* if a 'realloc' which shrinks memory fails. */
        if (likely(resized_big1 != NULL)) {
          big1 = resized_big1;
        } /* if */
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
    memSizeType pos = 0;
    doubleBigDigitType carry = 1;

  /* negate_positive_big */
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
    memSizeType pos = 0;
    doubleBigDigitType carry = 1;

  /* positive_copy_of_negative_big */
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
    memSizeType pos = 0;

  /* uBigMultByPowerOf10AndAdd */
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
    memSizeType pos = 0;

  /* uBigMultiplyAndAdd */
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
 *  Divides the unsigned big integer big1 by POWER_OF_5_IN_BIGDIGIT.
 *  The quotient is assigned to big1.
 *  @return the remainder of the unsigned big integer division.
 */
static inline void uBigDivideByPowerOf5 (const bigIntType big1)

  {
    memSizeType pos;
    doubleBigDigitType carry = 0;
    bigDigitType bigdigit;

  /* uBigDivideByPowerOf5 */
    pos = big1->size;
    do {
      pos--;
      carry <<= BIGDIGIT_SIZE;
      carry += big1->bigdigits[pos];
      bigdigit = (bigDigitType) (carry / POWER_OF_5_IN_BIGDIGIT);
#if POINTER_SIZE <= BIGDIGIT_SIZE
      /* There is probably no machine instruction for division    */
      /* and remainder of doubleBigDigitType values available.    */
      /* To compute the remainder fast the % operator is avoided  */
      /* and the remainder is computed with a multiplication and  */
      /* a subtraction. The overflow in the multiplication can be */
      /* ignored, since the result of the subtraction fits in the */
      /* lower bigdigit of carry. The wrong bits in the higher    */
      /* bigdigit of carry are masked away.                       */
      carry = (carry - bigdigit * POWER_OF_5_IN_BIGDIGIT) & BIGDIGIT_MASK;
#else
      /* There is probably a machine instruction for division     */
      /* and remainder of doubleBigDigitType values available.    */
      /* In the optimal case quotient and remainder can be        */
      /* computed with one instruction.                           */
      carry %= POWER_OF_5_IN_BIGDIGIT;
#endif
      big1->bigdigits[pos] = bigdigit;
    } while (pos > 0);
  } /* uBigDivideByPowerOf5 */



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



/**
 *  Convert a numeric string, with a specified radix, to a 'bigInteger'.
 *  The numeric string must contain the representation of an integer
 *  in the specified radix. It consists of an optional + or - sign,
 *  followed by a sequence of digits in the specified radix. Digit values
 *  from 10 upward can be encoded with upper or lower case letters.
 *  E.g.: 10 can be encoded with A or a, 11 with B or b, etc. Other
 *  characters as well as leading or trailing whitespace characters
 *  are not allowed. The radix of the conversion is a power of two and
 *  it is specified indirectly with the parameter shift.
 *  @param stri Numeric string with integer in the specified radix.
 *  @param shift Logarithm (log2) of the specified radix.
 *  @return the 'bigInteger' result of the conversion.
 *  @exception RANGE_ERROR If the string does not contain an integer
 *             literal with the specified base.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
static bigIntType bigParseBasedPow2 (const const_striType stri, unsigned int shift)

  {
    memSizeType mostSignificantDigitPos = 0;
    boolType negative;
    memSizeType bits_necessary;
    boolType okay;
    memSizeType charPos;
    strElemType digit;
    unsigned int base;
    unsigned int digitval;
    memSizeType bigDigitPos;
    doubleBigDigitType bigDigit;
    unsigned int bigDigitShift;
    memSizeType result_size;
    bigIntType result;

  /* bigParseBasedPow2 */
    logFunction(printf("bigParseBasedPow2(\"%s\", %u)\n",
                       striAsUnquotedCStri(stri), shift););
    if (likely(stri->size != 0)) {
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
    /* printf("mostSignificantDigitPos: " FMT_U_MEM "\n", mostSignificantDigitPos); */
    if (unlikely(mostSignificantDigitPos >= stri->size)) {
      logError(printf("bigParseBasedPow2(\"%s\", %u): "
                      "Digit missing.\n",
                      striAsUnquotedCStri(stri), shift););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (unlikely(stri->size - mostSignificantDigitPos >
                        MAX_MEMSIZETYPE / (memSizeType) shift)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      base = (unsigned int) 1 << shift;
      /* Compute the number of bits necessary: */
      bits_necessary = (stri->size - mostSignificantDigitPos) * (memSizeType) shift;
      /* printf("bits_necessary: " FMT_U_MEM "\n", bits_necessary); */
      /* Compute the number of bigDigits: */
      result_size = (bits_necessary - 1) / BIGDIGIT_SIZE + 1;
      if ((bits_necessary & BIGDIGIT_SIZE_MASK) == 0) {
        digit = stri->mem[mostSignificantDigitPos];
        if (digit >= '0' && digit <= 'z') {
          digitval = digit_value[digit - (strElemType) '0'];
          if (digitval < base &&
              (digitval >> (shift - 1) & 1) == 1) {
            /* The highest bit of the most significant digit is set. */
            result_size++;
          } /* if */
        } /* if */
      } /* if */
      /* printf("result_size: " FMT_U_MEM "\n", result_size); */
      if (unlikely(!ALLOC_BIG(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        okay = TRUE;
        bigDigit = 0;
        bigDigitPos = 0;
        bigDigitShift = 0;
        for (charPos = stri->size; charPos > mostSignificantDigitPos && okay; charPos--) {
          digit = stri->mem[charPos - 1];
          if (likely(digit >= '0' && digit <= 'z')) {
            digitval = digit_value[digit - (strElemType) '0'];
            if (likely(digitval < base)) {
              bigDigit |= (doubleBigDigitType) digitval << bigDigitShift;
            } else {
              okay = FALSE;
            } /* if */
          } else {
            okay = FALSE;
          } /* if */
          bigDigitShift += shift;
          if (bigDigitShift >= BIGDIGIT_SIZE) {
            /* printf("result->bigdigits[" FMT_U_MEM "] = " F_X_DIG(08) "\n",
               bigDigitPos, (bigDigitType) (bigDigit & BIGDIGIT_MASK)); */
            result->bigdigits[bigDigitPos] = (bigDigitType) (bigDigit & BIGDIGIT_MASK);
            bigDigitPos++;
            bigDigit >>= BIGDIGIT_SIZE;
            bigDigitShift -= BIGDIGIT_SIZE;
          } /* if */
        } /* for */
        if (unlikely(!okay)) {
          FREE_BIG2(result, result_size);
          logError(printf("bigParseBasedPow2(\"%s\", %u): "
                          "Illegal digit.\n",
                          striAsUnquotedCStri(stri), shift););
          raise_error(RANGE_ERROR);
          result = NULL;
        } else {
          result->size = result_size;
          while (bigDigitPos < result_size) {
            /* printf("result->bigdigits[" FMT_U_MEM "] = " F_X_DIG(08) "\n",
               bigDigitPos, (bigDigitType) (bigDigit & BIGDIGIT_MASK)); */
            result->bigdigits[bigDigitPos] = (bigDigitType) (bigDigit & BIGDIGIT_MASK);
            bigDigitPos++;
            bigDigit >>= BIGDIGIT_SIZE;
          } /* while */
          if (negative) {
            negate_positive_big(result);
          } /* if */
          result = normalize(result);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigParseBasedPow2 --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigParseBasedPow2 */



/**
 *  Convert a numeric string, with a specified radix, to a 'bigInteger'.
 *  The numeric string must contain the representation of an integer
 *  in the specified radix. It consists of an optional + or - sign,
 *  followed by a sequence of digits in the specified radix. Digit values
 *  from 10 upward can be encoded with upper or lower case letters.
 *  E.g.: 10 can be encoded with A or a, 11 with B or b, etc. Other
 *  characters as well as leading or trailing whitespace characters
 *  are not allowed.
 *  @param stri Numeric string with integer in the specified radix.
 *  @param base Radix of the integer in the 'stri' parameter.
 *  @return the 'bigInteger' result of the conversion.
 *  @exception RANGE_ERROR If base < 2 or base > 36 holds or
 *             the string does not contain an integer
 *             literal with the specified base.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
static bigIntType bigParseBased2To36 (const const_striType stri, unsigned int base)

  {
    boolType okay;
    boolType negative;
    memSizeType position = 0;
    uint8Type based_digit_size;
    uint8Type based_digits_in_bigdigit;
    bigDigitType power_of_base_in_bigdigit;
    memSizeType limit;
    strElemType digit;
    unsigned int digitval;
    bigDigitType bigDigit;
    memSizeType result_size;
    bigIntType result;

  /* bigParseBased2To36 */
    logFunction(printf("bigParseBased2To36(\"%s\", %u)\n",
                       striAsUnquotedCStri(stri), base););
    if (likely(stri->size != 0)) {
      if (stri->mem[0] == ((strElemType) '-')) {
        negative = TRUE;
        position++;
      } else {
        if (stri->mem[0] == ((strElemType) '+')) {
          position++;
        } /* if */
        negative = FALSE;
      } /* if */
    } /* if */
    /* printf("position: " FMT_U_MEM "\n", position); */
    if (unlikely(position >= stri->size)) {
      logError(printf("bigParseBased2To36(\"%s\", %u): "
                      "Digit missing.\n",
                      striAsUnquotedCStri(stri), base););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (unlikely(stri->size > MAX_MEMSIZETYPE / 6)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      based_digit_size = (uint8Type) (uint8MostSignificantBit((uint8Type) (base - 1)) + 1);
      /* Estimate the number of bits necessary: */
      result_size = stri->size * (memSizeType) based_digit_size;
      /* Compute the number of bigDigits: */
      result_size = result_size / BIGDIGIT_SIZE + 1;
      if (unlikely(!ALLOC_BIG(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 1;
        result->bigdigits[0] = 0;
        okay = TRUE;
        based_digits_in_bigdigit = radixDigitsInBigdigit[base - 2];
        power_of_base_in_bigdigit = powerOfRadixInBigdigit[base - 2];
        limit = (stri->size - position - 1) % based_digits_in_bigdigit + position + 1;
        do {
          bigDigit = 0;
          while (position < limit && okay) {
            digit = stri->mem[position];
            if (likely(digit >= '0' && digit <= 'z')) {
              digitval = digit_value[digit - (strElemType) '0'];
              if (likely(digitval < base)) {
                bigDigit = (bigDigitType) base * bigDigit + digitval;
              } else {
                okay = FALSE;
              } /* if */
            } else {
              okay = FALSE;
            } /* if */
            position++;
          } /* while */
          uBigMultiplyAndAdd(result, power_of_base_in_bigdigit, (doubleBigDigitType) bigDigit);
          limit += based_digits_in_bigdigit;
        } while (position < stri->size && okay);
        if (likely(okay)) {
          memset(&result->bigdigits[result->size], 0,
                 (size_t) (result_size - result->size) * sizeof(bigDigitType));
          result->size = result_size;
          if (negative) {
            negate_positive_big(result);
          } /* if */
          result = normalize(result);
        } else {
          FREE_BIG2(result, result_size);
          logError(printf("bigParseBased2To36(\"%s\", %u): "
                          "Illegal digit.\n",
                          striAsUnquotedCStri(stri), base););
          raise_error(RANGE_ERROR);
          result = NULL;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigParseBased2To36 --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigParseBased2To36 */



/**
 *  Computes base ** (2 ** exponent) to be used as conversion divisor.
 *  The function uses a cache to avoid a recomputation.
 *  The result is used by binaryToStri and binaryRadix2To36 as
 *  divisor. The functions binaryToStri and binaryRadix2To36 use
 *  the binary algorithm to convert a bigInteger to a string.
 *  @return base ** (2 ** exponent).
 */
static bigIntType getConversionDivisor (unsigned int base, unsigned int exponent)

  {
    bigIntType *divisorCache;
    unsigned int size;
    unsigned int pos;
    bigIntType divisor;

  /* getConversionDivisor */
    logFunction(printf("getConversionDivisor(%u, %u)\n", base, exponent););
    divisorCache = conversionDivisorCache[base];
    size = conversionDivisorCacheSize[base];
    if (exponent < size) {
      divisor = divisorCache[exponent];
    } else {
      divisorCache = (bigIntType *) realloc(divisorCache,
          (exponent + 1) * sizeof(bigIntType));
      if (unlikely(divisorCache == NULL)) {
        raise_error(MEMORY_ERROR);
        divisor = NULL;
      } else {
        if (size == 0) {
          divisorCache[0] = bigFromUInt32(base);
          size = 1;
        } /* if */
        for (pos = size; pos <= exponent; ++pos) {
          divisorCache[pos] = bigSquare(divisorCache[pos - 1]);
        } /* for */
        conversionDivisorCache[base] = divisorCache;
        conversionDivisorCacheSize[base] = exponent + 1;
        divisor = divisorCache[exponent];
      } /* if */
    } /* if */
    logFunction(printf("getConversionDivisor --> %s\n", bigHexCStri(divisor)););
    return divisor;
  } /* getConversionDivisor */



static memSizeType basicToStri (const bigIntType unsignedBig,
    striType buffer, memSizeType pos)

  {
    bigDigitType digit;
    int digit_pos;

  /* basicToStri */
    logFunction(printf("basicToStri(%s, *, " FMT_U_MEM ")\n",
                       bigHexCStri(unsignedBig), pos););
    do {
      digit = uBigDivideByPowerOf10(unsignedBig);
      /* printf("unsignedBig->size=" FMT_U_MEM ", digit=" FMT_U_DIG "\n",
         unsignedBig->size, digit); */
      if (unsignedBig->bigdigits[unsignedBig->size - 1] == 0) {
        unsignedBig->size--;
      } /* if */
      if (unsignedBig->size > 1 || unsignedBig->bigdigits[0] != 0) {
        for (digit_pos = DECIMAL_DIGITS_IN_BIGDIGIT;
            digit_pos != 0; digit_pos--) {
          buffer->mem[pos] = '0' + digit % 10;
          digit /= 10;
          pos--;
        } /* for */
      } else {
        do {
          buffer->mem[pos] = '0' + digit % 10;
          digit /= 10;
          pos--;
        } while (digit != 0);
      } /* if */
    } while (unsignedBig->size > 1 || unsignedBig->bigdigits[0] != 0);
    logFunction(printf("basicToStri --> " FMT_U_MEM "\n", pos););
    return pos;
  } /* basicToStri */



static memSizeType binaryToStri (bigIntType unsignedBig, striType buffer,
    unsigned int exponent, boolType zeroPad, memSizeType pos)

  {
    bigIntType divisor;
    bigIntType quotient;
    bigIntType remainder;
    memSizeType endPos;

  /* binaryToStri */
    logFunction(printf("binaryToStri(%s, *, %u, %d, " FMT_U_MEM ")\n",
                       bigHexCStri(unsignedBig), exponent, zeroPad, pos););
    if (exponent > 8) {
      exponent--;
      divisor = getConversionDivisor(10, exponent);
      if (divisor != NULL) {
        quotient = bigDivRem(unsignedBig, divisor, &remainder);
        if (quotient != NULL) {
          if (zeroPad || (quotient->size > 1 || quotient->bigdigits[0] != 0)) {
            pos = binaryToStri(remainder, buffer, exponent, TRUE, pos);
            pos = binaryToStri(quotient, buffer, exponent, zeroPad, pos);
          } else {
            pos = binaryToStri(remainder, buffer, exponent, FALSE, pos);
          } /* if */
          FREE_BIG(remainder);
          FREE_BIG(quotient);
        } /* if */
      } /* if */
    } else {
      endPos = pos;
      pos = basicToStri(unsignedBig, buffer, pos);
      if (zeroPad) {
        /* printf(FMT_U_MEM " " FMT_U_MEM " " FMT_U_MEM
               " insert " FMT_U_MEM " zero digits.\n",
               pos, endPos, (memSizeType) 1 << exponent,
               pos - (endPos - ((memSizeType) 1 << exponent))); */
        endPos -= (memSizeType) 1 << exponent;
        while (pos > endPos) {
          buffer->mem[pos] = '0';
          pos--;
        } /* while */
      } /* if */
    } /* if */
    logFunction(printf("binaryToStri --> " FMT_U_MEM "\n", pos););
    return pos;
  } /* binaryToStri */



static memSizeType basicRadix2To36 (const bigIntType unsignedBig,
    striType buffer, unsigned int base, const const_ustriType digits,
    memSizeType pos)

  {
    bigDigitType divisor_digit;
    uint8Type digits_in_bigdigit;
    bigDigitType digit;
    int digit_pos;

  /* basicRadix2To36 */
    logFunction(printf("basicRadix2To36(%s, *, %u, *, " FMT_U_MEM ")\n",
                       bigHexCStri(unsignedBig), base, pos););
    divisor_digit = powerOfRadixInBigdigit[base - 2];
    /* printf("divisor_digit: " FMT_U_DIG "\n", divisor_digit); */
    digits_in_bigdigit = radixDigitsInBigdigit[base - 2];
    /* printf("digits_in_bigdigit: %hd\n", digits_in_bigdigit); */
    do {
      digit = uBigDivideByDigit(unsignedBig, divisor_digit);
      /* printf("unsignedBig->size=" FMT_U_MEM ", digit=" FMT_U_DIG "\n",
         unsignedBig->size, digit); */
      if (unsignedBig->bigdigits[unsignedBig->size - 1] == 0) {
        unsignedBig->size--;
      } /* if */
      if (unsignedBig->size > 1 || unsignedBig->bigdigits[0] != 0) {
        for (digit_pos = digits_in_bigdigit;
            digit_pos != 0; digit_pos--) {
          buffer->mem[pos] = (strElemType) (digits[digit % base]);
          digit /= base;
          pos--;
        } /* for */
      } else {
        do {
          buffer->mem[pos] = (strElemType) (digits[digit % base]);
          digit /= base;
          pos--;
        } while (digit != 0);
      } /* if */
    } while (unsignedBig->size > 1 || unsignedBig->bigdigits[0] != 0);
    logFunction(printf("basicRadix2To36 --> " FMT_U_MEM "\n", pos););
    return pos;
  } /* basicRadix2To36 */



static memSizeType binaryRadix2To36 (bigIntType unsignedBig,
    striType buffer, unsigned int base, const const_ustriType digits,
    unsigned int exponent, boolType zeroPad, memSizeType pos)

  {
    bigIntType divisor;
    bigIntType quotient;
    bigIntType remainder;
    memSizeType endPos;

  /* binaryRadix2To36 */
    logFunction(printf("binaryRadix2To36(%s, *, %u, *, %u, %u, "
                       FMT_U_MEM ")\n", bigHexCStri(unsignedBig),
                       base, exponent, zeroPad, pos););
    if (exponent > 8) {
      exponent--;
      divisor = getConversionDivisor(base, exponent);
      if (divisor != NULL) {
        quotient = bigDivRem(unsignedBig, divisor, &remainder);
        if (quotient != NULL) {
          if (zeroPad || (quotient->size > 1 || quotient->bigdigits[0] != 0)) {
            pos = binaryRadix2To36(remainder, buffer, base, digits,
                                  exponent, TRUE, pos);
            pos = binaryRadix2To36(quotient, buffer, base, digits,
                                  exponent, zeroPad, pos);
          } else {
            pos = binaryRadix2To36(remainder, buffer, base, digits,
                                  exponent, FALSE, pos);
          } /* if */
          FREE_BIG(remainder);
          FREE_BIG(quotient);
        } /* if */
      } /* if */
    } else {
      endPos = pos;
      pos = basicRadix2To36(unsignedBig, buffer, base, digits, pos);
      if (zeroPad) {
        /* printf(FMT_U_MEM " " FMT_U_MEM " " FMT_U_MEM
               " insert " FMT_U_MEM " zero digits.\n",
               pos, endPos, (memSizeType) 1 << exponent,
               pos - (endPos - ((memSizeType) 1 << exponent))); */
        endPos -= (memSizeType) 1 << exponent;
        while (pos > endPos) {
          buffer->mem[pos] = '0';
          pos--;
        } /* while */
      } /* if */
    } /* if */
    logFunction(printf("binaryRadix2To36 --> " FMT_U_MEM "\n", pos););
    return pos;
  } /* binaryRadix2To36 */



/**
 *  Convert a big integer number to a string using a radix.
 *  The conversion uses the numeral system with the specified base.
 *  The base is a power of two and it is specified indirectly with
 *  shift and mask. Digit values from 10 upward are encoded with
 *  letters.
 *  @param big1 BigInteger number to be converted.
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
    const_ustriType digits;
    memSizeType pos;
    memSizeType result_size;
    striType result;

  /* bigRadixPow2 */
    logFunction(printf("bigRadixPow2(%s, %u, " FMT_X_DIG ", %d)\n",
                       bigHexCStri(big1), shift, mask, upperCase););
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    if (negative) {
      unsigned_big = alloc_positive_copy_of_negative_big(big1);
    } else {
      unsigned_big = big1;
    } /* if */
    if (unlikely(unsigned_big == NULL)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      unsigned_size = unsigned_big->size;
      /* Unsigned_size is reduced to avoid a leading zero digit.        */
      /* Except for the value 0 unsigned_big has no leading zero digit. */
      while (unsigned_size > 1 && unsigned_big->bigdigits[unsigned_size - 1] == 0) {
        unsigned_size--;
      } /* while */
      if (unlikely((MAX_STRI_LEN <= (MAX_MEMSIZETYPE - 1) / shift +
                                    (unsigned int) negative + 1 &&
                   unsigned_size > ((MAX_STRI_LEN - (unsigned int) negative - 1) *
                                shift + 1) / BIGDIGIT_SIZE) ||
                   unsigned_size > MAX_MEMSIZETYPE / BIGDIGIT_SIZE)) {
        if (unsigned_big != big1) {
          FREE_BIG(unsigned_big);
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
          most_significant = (unsigned int)
              digitMostSignificantBit(unsigned_big->bigdigits[unsigned_size - 1]);
          digit_shift = (unsigned int) (BIGDIGIT_SIZE - (unsigned_size * BIGDIGIT_SIZE) % shift);
          if (most_significant < digit_shift) {
            /* Reduce result_size because of leading zero digits. */
            result_size -= (digit_shift - most_significant - 1) / shift;
            if (digit_shift != BIGDIGIT_SIZE) {
              result_size--;
            } /* if */
          } /* if */
        } /* if */
        /* printf("result_size: " FMT_U_MEM "\n", result_size); */
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
          if (unsigned_big != big1) {
            FREE_BIG(unsigned_big);
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
                unsigned_digit |=
                    unsigned_big->bigdigits[digit_index] << (BIGDIGIT_SIZE - digit_shift);
                if (pos > (memSizeType) negative) {
                  pos--;
                  result->mem[pos] = (strElemType) (digits[unsigned_digit & mask]);
                  unsigned_digit = unsigned_big->bigdigits[digit_index];
                  digit_shift += shift - BIGDIGIT_SIZE;
                } /* if */
              } /* if */
            } else if (digit_shift != BIGDIGIT_SIZE && pos > (memSizeType) negative) {
              pos--;
              result->mem[pos] = (strElemType) (digits[(unsigned_digit >> digit_shift) & mask]);
            } /* if */
          } while (digit_index < unsigned_size && pos > (memSizeType) negative);
          if (negative) {
            pos--;
            /* printf("result->mem[" FMT_U_MEM "] = -\n", pos); */
            result->mem[pos] = (strElemType) '-';
          } /* if */
          result->size = result_size;
        } /* if */
      } /* if */
      if (unsigned_big != big1) {
        FREE_BIG(unsigned_big);
      } /* if */
    } /* if */
    logFunction(printf("bigRadixPow2 --> \"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* bigRadixPow2 */



/**
 *  Convert a big integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with letters.
 *  For negative numbers a minus sign is prepended.
 *  @param big1 BigInteger number to be converted.
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
    memSizeType pos;
    memSizeType result_size;
    memSizeType final_result_size;
    striType resized_result;
    striType result;

  /* bigRadix2To36 */
    logFunction(printf("bigRadix2To36(%s, %u, %d)\n",
                       bigHexCStri(big1), base, upperCase););
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
      /* printf("result_size: " FMT_U_MEM "\n", result_size); */
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
          FREE_STRI2(result, result_size);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          pos = binaryRadix2To36(unsigned_big, result, base,
                                 digitTable[upperCase], (unsigned int)
                                 memSizeMostSignificantBit(result_size) + 1,
                                 FALSE, result_size - 1);
          FREE_BIG2(unsigned_big, big1->size);
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
              FREE_STRI2(result, result_size);
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
    logFunction(printf("bigRadix2To36 --> \"%s\"\n", striAsUnquotedCStri(result)););
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
    memSizeType pos = 0;

  /* uBigLShift */
    do {
      carry |= ((doubleBigDigitType) big1->bigdigits[pos]) << lshift;
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big1->size);
  } /* uBigLShift */



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
    bigDigitType high_digit = 0;
    memSizeType pos;

  /* uBigRShift */
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
 *  This function does overflow silently if big1 contains
 *  not enough digits.
 */
static void uBigIncr (const bigIntType big1)

  {
    memSizeType pos = 0;

  /* uBigIncr */
    if (unlikely(big1->bigdigits[pos] == BIGDIGIT_MASK)) {
      if (big1->size == 1) {
        big1->bigdigits[pos] = 0;
        pos++;
      } else {
        do {
          big1->bigdigits[pos] = 0;
          pos++;
        } while (big1->bigdigits[pos] == BIGDIGIT_MASK);
      } /* if */
    } /* if */
    if (pos < big1->size) {
      big1->bigdigits[pos]++;
    } /* if */
  } /* uBigIncr */



/**
 *  Decrements an unsigned big integer by 1.
 *  This function does overflow silently if big1 contains
 *  not enough digits. The function works correctly if there
 *  are leading zereo digits.
 */
static void uBigDecr (const bigIntType big1)

  {
    memSizeType pos = 0;

  /* uBigDecr */
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
 *  Computes the quotient of an integer division of dividend by one
 *  divisor_digit for nonnegative big integers. The divisor_digit
 *  must not be zero.
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
 *  for signed big integers. The memory for the quotient is requested
 *  and the normalized quotient is returned. This function handles
 *  also the special case of a division by zero.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
static bigIntType bigDiv1 (const_bigIntType dividend, bigDigitType divisor_digit)

  {
    boolType negative = FALSE;
    bigIntType dividend_help = NULL;
    bigIntType quotient;

  /* bigDiv1 */
    if (unlikely(divisor_digit == 0)) {
      logError(printf("bigDiv1(%s, " FMT_U_DIG "): Division by zero.\n",
                      bigHexCStri(dividend), divisor_digit););
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
            FREE_BIG(quotient);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
        } /* if */
        quotient->bigdigits[quotient->size - 1] = 0;
        if (IS_NEGATIVE(divisor_digit)) {
          negative = !negative;
          /* The unsigned value is negated to avoid a signed integer */
          /* overflow if the smallest signed integer is negated.     */
          divisor_digit = -divisor_digit;
        } /* if */
        uBigDiv1(dividend, divisor_digit, quotient);
        if (negative) {
          negate_positive_big(quotient);
        } /* if */
        quotient = normalize(quotient);
        if (dividend_help != NULL) {
          FREE_BIG(dividend_help);
        } /* if */
        return quotient;
      } /* if */
    } /* if */
  } /* bigDiv1 */



/**
 *  Computes an integer division of dividend by divisor for signed big
 *  integers if dividend has less digits than divisor. The memory for
 *  the quotient is requested and the normalized quotient is returned. Normally
 *  dividend->size < divisor->size implies abs(dividend) < abs(divisor).
 *  If abs(dividend) < abs(divisor) holds the quotient is 0. The cases
 *  dividend->size < divisor->size and abs(dividend) = abs(divisor) are if
 *  dividend->size + 1 == divisor->size and dividend = 0x8000 (0x80000000...)
 *  and divisor = 0x00008000 (0x000080000000...). In this cases the
 *  quotient is -1. In all other cases the quotient is 0.
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
 *  Multiplies big2 with multiplier and subtracts the product from
 *  big1 at the digit position pos1 of big1. Big1, big2 and
 *  multiplier are nonnegative big integer values.
 *  The algorithm tries to save computations. Therefore
 *  there are checks for mult_carry != 0 and sbtr_carry == 0.
 */
static bigDigitType uBigMultSub (const bigIntType big1, const const_bigIntType big2,
    const bigDigitType multiplier, const memSizeType pos1)

  {
    memSizeType pos = 0;
    doubleBigDigitType mult_carry = 0;
    doubleBigDigitType sbtr_carry = 1;

  /* uBigMultSub */
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
    memSizeType pos = 0;
    doubleBigDigitType carry = 0;

  /* uBigAddTo */
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
 *  Computes quotient and remainder of an integer division of dividend by
 *  divisor for nonnegative big integers. The remainder is delivered in
 *  dividend. There are several preconditions for this function. Divisor
 *  must have at least 2 digits and dividend must have at least one
 *  digit more than divisor. If dividend and divisor have the same length in
 *  digits nothing is done. The most significant bit of divisor must be
 *  set. The most significant digit of dividend must be less than the
 *  most significant digit of divisor. The computations to meet this
 *  preconditions are done outside this function. The special cases
 *  with a one digit divisor or a dividend with less digits than divisor are
 *  handled in other functions. This algorithm based on the algorithm
 *  from D.E. Knuth described in "The art of computer programming"
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
 *  Computes quotient and remainder of an integer division of dividend
 *  by one divisor_digit for nonnegative big integers. The divisor_digit
 *  must not be zero. The remainder of the division is returned.
 */
static bigDigitType uBigDivRem1 (const const_bigIntType dividend,
    const bigDigitType divisor_digit, const bigIntType quotient)

  {
    memSizeType pos;
    doubleBigDigitType carry = 0;

  /* uBigDivRem1 */
    pos = dividend->size;
    do {
      pos--;
      carry <<= BIGDIGIT_SIZE;
      carry += dividend->bigdigits[pos];
      quotient->bigdigits[pos] = (bigDigitType) ((carry / divisor_digit) & BIGDIGIT_MASK);
      carry %= divisor_digit;
    } while (pos > 0);
    return (bigDigitType) carry;
  } /* uBigDivRem1 */


/**
 *  Computes quotient and remainder of the integer division dividend
 *  by one divisor_digit for signed big integers. The memory for the
 *  quotient is requested and the normalized quotient is returned.
 *  The memory for the remainder is requested and the normalized
 *  remainder is assigned to *remainder. This function handles also
 *  the special case of a division by zero.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
static bigIntType bigDivRem1 (const_bigIntType dividend, bigDigitType divisor_digit,
    bigIntType *remainder)

  {
    boolType quotientNegative = FALSE;
    boolType remainderNegative = FALSE;
    bigIntType dividend_help = NULL;
    bigIntType quotient;

  /* bigDivRem1 */
    if (unlikely(divisor_digit == 0)) {
      logError(printf("bigDivRem1(%s, " FMT_U_DIG "): Division by zero.\n",
                      bigHexCStri(dividend), divisor_digit););
      *remainder = NULL;
      raise_error(NUMERIC_ERROR);
      return NULL;
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(quotient, dividend->size + 1))) {
        *remainder = NULL;
        raise_error(MEMORY_ERROR);
        return NULL;
      } else if (unlikely(!ALLOC_BIG_SIZE_OK(*remainder, 1))) {
        FREE_BIG(quotient);
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        quotient->size = dividend->size + 1;
        (*remainder)->size = 1;
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          quotientNegative = TRUE;
          remainderNegative = TRUE;
          dividend_help = alloc_positive_copy_of_negative_big(dividend);
          dividend = dividend_help;
          if (unlikely(dividend_help == NULL)) {
            FREE_BIG(quotient);
            FREE_BIG(*remainder);
            *remainder = NULL;
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
        } /* if */
        quotient->bigdigits[quotient->size - 1] = 0;
        if (IS_NEGATIVE(divisor_digit)) {
          quotientNegative = !quotientNegative;
          /* The unsigned value is negated to avoid a signed integer */
          /* overflow if the smallest signed integer is negated.     */
          divisor_digit = -divisor_digit;
        } /* if */
        (*remainder)->bigdigits[0] = uBigDivRem1(dividend, divisor_digit, quotient);
        if (quotientNegative) {
          negate_positive_big(quotient);
        } /* if */
        quotient = normalize(quotient);
        if (remainderNegative) {
          negate_positive_big(*remainder);
        } /* if */
        if (dividend_help != NULL) {
          FREE_BIG(dividend_help);
        } /* if */
        return quotient;
      } /* if */
    } /* if */
  } /* bigDivRem1 */



/**
 *  Computes quotient and remainder of the integer division of dividend
 *  by divisor for signed big integers if dividend has less digits than
 *  divisor. The memory for the quotient is requested and the normalized
 *  quotient is returned. The memory for the remainder is requested and
 *  the normalized remainder is assigned to *remainder. Normally
 *  dividend->size < divisor->size implies abs(dividend) < abs(divisor).
 *  If abs(dividend) < abs(divisor) holds the quotient is 0 and the
 *  remainder is dividend. The cases
 *  dividend->size < divisor->size and abs(dividend) = abs(divisor) are if
 *  dividend->size + 1 == divisor->size and dividend = 0x8000 (0x80000000...)
 *  and divisor = 0x00008000 (0x000080000000...). In this cases the
 *  quotient is -1 and the remainder is 0. In all other cases the quotient
 *  is 0 and the remainder is dividend.
 *  @return the quotient of the integer division.
 */
static bigIntType bigDivRemSizeLess (const const_bigIntType dividend,
    const const_bigIntType divisor, bigIntType *remainder)

  {
    memSizeType pos;
    boolType remainderIs0;
    bigIntType quotient;

  /* bigDivRemSizeLess */
    if (unlikely(!ALLOC_BIG_SIZE_OK(quotient, 1))) {
      *remainder = NULL;
      raise_error(MEMORY_ERROR);
    } else {
      quotient->size = 1;
      if (unlikely(dividend->size + 1 == divisor->size &&
                   dividend->bigdigits[dividend->size - 1] == BIGDIGIT_SIGN &&
                   divisor->bigdigits[divisor->size - 1] == 0 &&
                   divisor->bigdigits[divisor->size - 2] == BIGDIGIT_SIGN)) {
        remainderIs0 = TRUE;
        pos = dividend->size - 1;
        while (pos > 0) {
          pos--;
          if (likely(dividend->bigdigits[pos] != 0 || divisor->bigdigits[pos] != 0)) {
            remainderIs0 = FALSE;
            pos = 0;
          } /* if */
        } /* while */
      } else {
        remainderIs0 = FALSE;
      } /* if */
      if (remainderIs0) {
        if (unlikely(!ALLOC_BIG_SIZE_OK(*remainder, 1))) {
          FREE_BIG2(quotient, 1);
          raise_error(MEMORY_ERROR);
          quotient = NULL;
        } else {
          (*remainder)->size = 1;
          (*remainder)->bigdigits[0] = 0;
          quotient->bigdigits[0] = BIGDIGIT_MASK;
        } /* if */
      } else {
        if (unlikely(!ALLOC_BIG_SIZE_OK(*remainder, dividend->size))) {
          FREE_BIG2(quotient, 1);
          raise_error(MEMORY_ERROR);
          quotient = NULL;
        } else {
          (*remainder)->size = dividend->size;
          memcpy((*remainder)->bigdigits, dividend->bigdigits,
                 (size_t) dividend->size * sizeof(bigDigitType));
          quotient->bigdigits[0] = 0;
        } /* if */
      } /* if */
    } /* if */
    return quotient;
  } /* bigDivRemSizeLess */



/**
 *  Computes the remainder of an integer division of dividend by
 *  one divisor_digit for nonnegative big integers. The divisor_digit must
 *  not be zero.
 */
static bigDigitType uBigRem1 (const const_bigIntType dividend,
    const bigDigitType divisor_digit)

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
 *  @return the remainder of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
static bigIntType bigRem1 (const_bigIntType dividend, bigDigitType divisor_digit)

  {
    boolType negative = FALSE;
    bigIntType dividend_help = NULL;
    bigIntType remainder;

  /* bigRem1 */
    if (unlikely(divisor_digit == 0)) {
      logError(printf("bigRem1(%s, " FMT_U_DIG "): Division by zero.\n",
                      bigHexCStri(dividend), divisor_digit););
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
            FREE_BIG(remainder);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
        } /* if */
        if (IS_NEGATIVE(divisor_digit)) {
          /* The unsigned value is negated to avoid a signed integer */
          /* overflow if the smallest signed integer is negated.     */
          divisor_digit = -divisor_digit;
        } /* if */
        remainder->bigdigits[0] = uBigRem1(dividend, divisor_digit);
        if (negative) {
          negate_positive_big(remainder);
        } /* if */
        if (dividend_help != NULL) {
          FREE_BIG(dividend_help);
        } /* if */
        return remainder;
      } /* if */
    } /* if */
  } /* bigRem1 */



/**
 *  Computes an integer modulo division of dividend by one
 *  divisor_digit for signed big integers. The memory for the
 *  quotient is requested and the normalized quotient is returned.
 *  This function handles also the special case of a division by
 *  zero.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
static bigIntType bigMDiv1 (const_bigIntType dividend, bigDigitType divisor_digit)

  {
    boolType negative = FALSE;
    bigIntType dividend_help = NULL;
    bigDigitType remainder;
    bigIntType quotient;

  /* bigMDiv1 */
    if (unlikely(divisor_digit == 0)) {
      logError(printf("bigMDiv1(%s, " FMT_U_DIG "): Division by zero.\n",
                      bigHexCStri(dividend), divisor_digit););
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
            FREE_BIG(quotient);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
        } /* if */
        quotient->bigdigits[quotient->size - 1] = 0;
        if (IS_NEGATIVE(divisor_digit)) {
          negative = !negative;
          /* The unsigned value is negated to avoid a signed integer */
          /* overflow if the smallest signed integer is negated.     */
          divisor_digit = -divisor_digit;
        } /* if */
        remainder = uBigDivRem1(dividend, divisor_digit, quotient);
        if (negative) {
          if (remainder != 0) {
            uBigIncr(quotient);
          } /* if */
          negate_positive_big(quotient);
        } /* if */
        quotient = normalize(quotient);
        if (dividend_help != NULL) {
          FREE_BIG(dividend_help);
        } /* if */
        return quotient;
      } /* if */
    } /* if */
  } /* bigMDiv1 */



/**
 *  Computes a modulo integer division of dividend by divisor for signed
 *  big integers if dividend has less digits than divisor. The memory for
 *  the quotient is requested and the normalized quotient is returned.
 *  If the dividend is zero the quotient is always zero. Otherwise if
 *  the signs of dividend and divisor differ the quotient is -1.
 *  In all other cases the quotient is zero.
 *  @return the quotient of the integer division.
 */
static bigIntType bigMDivSizeLess (const const_bigIntType dividend,
    const const_bigIntType divisor)

  {
    bigIntType quotient;

  /* bigMDivSizeLess */
    logFunction(printf("bigMDivSizeLess(%s,", bigHexCStri(dividend));
                printf("%s)\n", bigHexCStri(divisor)););
    if (unlikely(!ALLOC_BIG_SIZE_OK(quotient, 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      quotient->size = 1;
      if ((dividend->size == 1 && dividend->bigdigits[0] == 0) ||
          IS_NEGATIVE(dividend->bigdigits[dividend->size - 1]) ==
          IS_NEGATIVE(divisor->bigdigits[divisor->size - 1])) {
        quotient->bigdigits[0] = 0;
      } else {
        quotient->bigdigits[0] = BIGDIGIT_MASK;
      } /* if */
    } /* if */
    logFunction(printf("bigMDivSizeLess --> %s\n", bigHexCStri(quotient)););
    return quotient;
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
 *  signed big integers if dividend has less digits than divisor. The memory
 *  for the remainder is requested and the normalized remainder is returned.
 *  Normally dividend->size < divisor->size implies abs(dividend) < abs(divisor).
 *  If abs(dividend) < abs(divisor) holds the remainder is dividend. The cases
 *  dividend->size < divisor->size and abs(dividend) = abs(divisor) are if
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
    memSizeType pos = 0;
    doubleBigDigitType carry = 0;
    doubleBigDigitType big2_sign;

  /* bigAddTo */
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
 *  signed big integers if dividend has less digits than divisor. The memory
 *  for the modulo is requested and the normalized modulo is returned.
 *  There are two cases: If the modulo division (see bigMDivSizeLess() )
 *  would compute a quotient of zero the modulo is equal to the dividend.
 *  In all other cases the quotient would be -1 and the modulo is computed
 *  as divisor + dividend.
 *  @return the modulo of the integer division.
 */
static bigIntType bigModSizeLess (const const_bigIntType dividend,
    const const_bigIntType divisor)

  {
    bigIntType modulo;

  /* bigModSizeLess */
    logFunction(printf("bigModSizeLess(%s,", bigHexCStri(dividend));
                printf("%s)\n", bigHexCStri(divisor)););
    if ((dividend->size == 1 && dividend->bigdigits[0] == 0) ||
        IS_NEGATIVE(dividend->bigdigits[dividend->size - 1]) ==
        IS_NEGATIVE(divisor->bigdigits[divisor->size - 1])) {
      /* The quotient is zero. */
      if (unlikely(!ALLOC_BIG_SIZE_OK(modulo, dividend->size))) {
        raise_error(MEMORY_ERROR);
      } else {
        modulo->size = dividend->size;
        memcpy(modulo->bigdigits, dividend->bigdigits,
               (size_t) dividend->size * sizeof(bigDigitType));
      } /* if */
    } else {
      /* The quotient is -1. */
      if (unlikely(!ALLOC_BIG_SIZE_OK(modulo, divisor->size))) {
        raise_error(MEMORY_ERROR);
      } else {
        modulo->size = divisor->size;
        memcpy(modulo->bigdigits, divisor->bigdigits,
               (size_t) divisor->size * sizeof(bigDigitType));
        bigAddTo(modulo, dividend);
        modulo = normalize(modulo);
      } /* if */
    } /* if */
    logFunction(printf("bigModSizeLess --> %s\n", bigHexCStri(modulo)););
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
 *  preconditions are done outside this function. The special cases
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
    memSizeType pos = 0;
    doubleBigDigitType carry = 0;

  /* uBigDigitAdd */
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
    memSizeType pos = 0;
    doubleBigDigitType carry = 1;

  /* uBigDigitSbtrFrom */
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
    memSizeType pos = 0;
    doubleBigDigitType carry = 0;

  /* uBigDigitAddTo */
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
    memSizeType pos2 = 0;
    doubleBigDigitType carry = 0;

  /* uBigDigitMult */
    do {
      carry += (doubleBigDigitType) factor1[0] * factor2[pos2];
      product[pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos2++;
    } while (pos2 < size);
    product[size] = (bigDigitType) (carry & BIGDIGIT_MASK);
    for (pos1 = 1; pos1 < size; pos1++) {
      carry = 0;
      pos2 = 0;
      do {
        carry += (doubleBigDigitType) product[pos1 + pos2] +
            (doubleBigDigitType) factor1[pos1] * factor2[pos2];
        product[pos1 + pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos2++;
      } while (pos2 < size);
      product[pos1 + size] = (bigDigitType) (carry & BIGDIGIT_MASK);
    } /* for */
  } /* uBigDigitMult */



static void uBigKaratsubaMult (const bigDigitType *const factor1,
    const bigDigitType *const factor2, const memSizeType size,
    bigDigitType *const product, bigDigitType *const temp)

  {
    memSizeType sizeLo;
    memSizeType sizeHi;

  /* uBigKaratsubaMult */
    /* printf("uBigKaratsubaMult: size=" FMT_U_MEM "\n", size); */
    if (size < KARATSUBA_MULT_THRESHOLD) {
      uBigDigitMult(factor1, factor2, size, product);
    } else {
      sizeHi = size >> 1;
      sizeLo = size - sizeHi;
      uBigDigitAdd(factor1, sizeLo, &factor1[sizeLo], sizeHi, product);
      uBigDigitAdd(factor2, sizeLo, &factor2[sizeLo], sizeHi, &product[size]);
      uBigKaratsubaMult(product, &product[size], sizeLo + 1, temp, &temp[(sizeLo + 1) << 1]);
      uBigKaratsubaMult(factor1, factor2, sizeLo, product, &temp[(sizeLo + 1) << 1]);
      uBigKaratsubaMult(&factor1[sizeLo], &factor2[sizeLo], sizeHi, &product[sizeLo << 1],
                        &temp[(sizeLo + 1) << 1]);
      uBigDigitSbtrFrom(temp, (sizeLo + 1) << 1, product, sizeLo << 1);
      uBigDigitSbtrFrom(temp, (sizeLo + 1) << 1, &product[sizeLo << 1], sizeHi << 1);
      uBigDigitAddTo(&product[sizeLo], sizeLo + (sizeHi << 1), temp, (sizeLo + 1) << 1);
    } /* if */
  } /* uBigKaratsubaMult */



static void uBigDigitSquare (const bigDigitType *const big1,
    const memSizeType size, bigDigitType *const square)

  {
    memSizeType pos1;
    memSizeType pos2;
    doubleBigDigitType carry;
    doubleBigDigitType product;
    bigDigitType digit;

  /* uBigDigitSquare */
    digit = big1[0];
    carry = (doubleBigDigitType) digit * digit;
    square[0] = (bigDigitType) (carry & BIGDIGIT_MASK);
    carry >>= BIGDIGIT_SIZE;
    if (size == 1) {
      square[1] = (bigDigitType) (carry);
    } else {
      for (pos2 = 1; pos2 < size; pos2++) {
        product = (doubleBigDigitType) digit * big1[pos2];
        carry += (product << 1) & BIGDIGIT_MASK;
        square[pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        carry += product >> (BIGDIGIT_SIZE - 1);
      } /* for */
      square[pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
      square[pos2 + 1] = (bigDigitType) (carry >> BIGDIGIT_SIZE);
      for (pos1 = 1; pos1 < size; pos1++) {
        digit = big1[pos1];
        carry = (doubleBigDigitType) square[pos1 << 1] +
            (doubleBigDigitType) digit * digit;
        square[pos1 << 1] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        for (pos2 = pos1 + 1; pos2 < size; pos2++) {
          product = (doubleBigDigitType) digit * big1[pos2];
          carry += (doubleBigDigitType) square[pos1 + pos2] +
              ((product << 1) & BIGDIGIT_MASK);
          square[pos1 + pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          carry += product >> (BIGDIGIT_SIZE - 1);
        } /* for */
        carry += (doubleBigDigitType) square[pos1 + pos2];
        square[pos1 + pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
        if (pos1 < size - 1) {
          square[pos1 + pos2 + 1] = (bigDigitType) (carry >> BIGDIGIT_SIZE);
        } /* if */
      } /* for */
    } /* if */
  } /* uBigDigitSquare */



static void uBigKaratsubaSquare (const bigDigitType *const big1,
    const memSizeType size, bigDigitType *const square, bigDigitType *const temp)

  {
    memSizeType sizeLo;
    memSizeType sizeHi;

  /* uBigKaratsubaSquare */
    /* printf("uBigKaratsubaSquare: size=" FMT_U_MEM "\n", size);); */
    if (size < KARATSUBA_SQUARE_THRESHOLD) {
      uBigDigitSquare(big1, size, square);
    } else {
      sizeHi = size >> 1;
      sizeLo = size - sizeHi;
      uBigDigitAdd(big1, sizeLo, &big1[sizeLo], sizeHi, square);
      uBigKaratsubaSquare(square, sizeLo + 1, temp, &temp[(sizeLo + 1) << 1]);
      uBigKaratsubaSquare(big1, sizeLo, square, &temp[(sizeLo + 1) << 1]);
      uBigKaratsubaSquare(&big1[sizeLo], sizeHi, &square[sizeLo << 1], &temp[(sizeLo + 1) << 1]);
      uBigDigitSbtrFrom(temp, (sizeLo + 1) << 1, square, sizeLo << 1);
      uBigDigitSbtrFrom(temp, (sizeLo + 1) << 1, &square[sizeLo << 1], sizeHi << 1);
      uBigDigitAddTo(&square[sizeLo], sizeLo + (sizeHi << 1), temp, (sizeLo + 1) << 1);
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



static void uBigMult (const const_bigIntType factor1, const const_bigIntType factor2,
    const bigIntType product)

  {
    memSizeType pos1 = 0;
    memSizeType pos2;
    doubleBigDigitType carry = 0;

  /* uBigMult */
    do {
      carry += (doubleBigDigitType) factor1->bigdigits[pos1] * factor2->bigdigits[0];
      product->bigdigits[pos1] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos1++;
    } while (pos1 < factor1->size);
    product->bigdigits[factor1->size] = (bigDigitType) (carry & BIGDIGIT_MASK);
    for (pos2 = 1; pos2 < factor2->size; pos2++) {
      carry = 0;
      pos1 = 0;
      do {
        carry += (doubleBigDigitType) product->bigdigits[pos1 + pos2] +
            (doubleBigDigitType) factor1->bigdigits[pos1] * factor2->bigdigits[pos2];
        product->bigdigits[pos1 + pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos1++;
      } while (pos1 < factor1->size);
      product->bigdigits[factor1->size + pos2] = (bigDigitType) (carry & BIGDIGIT_MASK);
    } /* for */
  } /* uBigMult */



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
      /* overflow if the smallest signed integer is negated.     */
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
      FREE_BIG(*big_variable);
      *big_variable = product;
    } /* if */
    if (big1_help != NULL) {
      FREE_BIG(big1_help);
    } /* if */
  } /* bigMultAssign1 */



/**
 *  Multiply two unsigned big integers with the Karatsuba multiplication.
 *  @return the product, and NULL if there is not enough memory.
 */
static bigIntType uBigMultK (const_bigIntType factor1, const_bigIntType factor2,
    const boolType negative)

  {
    const_bigIntType help_big;
    bigIntType factor2_help;
    bigIntType temp;
    bigIntType product;

  /* uBigMultK */
    logFunction(printf("uBigMultK(size= " FMT_U_MEM ", size=" FMT_U_MEM ", *)\n",
                       factor1->size, factor2->size););
    if (factor2->size > factor1->size) {
      help_big = factor1;
      factor1 = factor2;
      factor2 = help_big;
    } /* if */
    if (factor1->size >= KARATSUBA_MULT_THRESHOLD &&
        factor2->size >= KARATSUBA_MULT_THRESHOLD) {
      if (factor2->size << 1 <= factor1->size) {
        if (unlikely(!ALLOC_BIG_SIZE_OK(factor2_help, factor1->size - (factor1->size >> 1)))) {
          product = NULL;
        } else {
          factor2_help->size = factor1->size - (factor1->size >> 1);
          memcpy(factor2_help->bigdigits, factor2->bigdigits,
                 (size_t) factor2->size * sizeof(bigDigitType));
          memset(&factor2_help->bigdigits[factor2->size], 0,
                 (size_t) (factor2_help->size - factor2->size) * sizeof(bigDigitType));
          factor2 = factor2_help;
          if (likely(ALLOC_BIG(product, (factor1->size >> 1) + (factor2->size << 1)))) {
            product->size = (factor1->size >> 1) + (factor2->size << 1);
            if (unlikely(!ALLOC_BIG(temp, factor1->size << 2))) {
              FREE_BIG2(product, (factor1->size >> 1) + (factor2->size << 1));
              product = NULL;
            } else {
              uBigKaratsubaMult(factor1->bigdigits, factor2->bigdigits,
                                factor1->size >> 1, product->bigdigits, temp->bigdigits);
              uBigKaratsubaMult(&factor1->bigdigits[factor1->size >> 1], factor2->bigdigits,
                                factor2->size, temp->bigdigits,
                                &temp->bigdigits[factor2->size << 1]);
              memset(&product->bigdigits[(factor1->size >> 1) << 1], 0,
                     (size_t) (product->size - ((factor1->size >> 1) << 1)) * sizeof(bigDigitType));
              uBigDigitAddTo(&product->bigdigits[factor1->size >> 1],
                             product->size - (factor1->size >> 1),
                             temp->bigdigits, factor2->size << 1);
              if (negative) {
                negate_positive_big(product);
              } /* if */
              product = normalize(product);
              FREE_BIG2(temp, factor1->size << 2);
            } /* if */
          } /* if */
          FREE_BIG2(factor2_help, factor1->size - (factor1->size >> 1));
        } /* if */
      } else {
        if (unlikely(!ALLOC_BIG_SIZE_OK(factor2_help, factor1->size))) {
          product = NULL;
        } else {
          factor2_help->size = factor1->size;
          memcpy(factor2_help->bigdigits, factor2->bigdigits,
                 (size_t) factor2->size * sizeof(bigDigitType));
          memset(&factor2_help->bigdigits[factor2->size], 0,
                 (size_t) (factor2_help->size - factor2->size) * sizeof(bigDigitType));
          factor2 = factor2_help;
          if (likely(ALLOC_BIG(product, factor1->size << 1))) {
            if (unlikely(!ALLOC_BIG(temp, factor1->size << 2))) {
              FREE_BIG2(product, factor1->size << 1);
              product = NULL;
            } else {
              uBigKaratsubaMult(factor1->bigdigits, factor2->bigdigits,
                  factor1->size, product->bigdigits, temp->bigdigits);
              product->size = factor1->size << 1;
              if (negative) {
                negate_positive_big(product);
              } /* if */
              product = normalize(product);
              FREE_BIG2(temp, factor1->size << 2);
            } /* if */
          } /* if */
          FREE_BIG2(factor2_help, factor1->size);
        } /* if */
      } /* if */
    } else {
      if (likely(ALLOC_BIG(product, factor1->size + factor2->size))) {
        product->size = factor1->size + factor2->size;
        uBigMult(factor1, factor2, product);
        if (negative) {
          negate_positive_big(product);
        } /* if */
        product = normalize(product);
      } /* if */
    } /* if */
    logFunction(printf("uBigMultK -> size= " FMT_U_MEM "\n", product->size););
    return product;
  } /* uBigMultK */



/**
 *  Square an unsigned big integer with the Karatsuba multiplication.
 *  @return the square, and NULL if there is not enough memory.
 */
static bigIntType uBigSquareK (const_bigIntType big1)

  {
    bigIntType temp;
    bigIntType square;

  /* uBigSquareK */
    logFunction(printf("uBigSquareK(size= " FMT_U_MEM ")\n",
                       big1->size););
    if (big1->size >= KARATSUBA_SQUARE_THRESHOLD) {
      if (likely(ALLOC_BIG(square, big1->size << 1))) {
        if (unlikely(!ALLOC_BIG(temp, big1->size << 2))) {
          FREE_BIG2(square, big1->size << 1);
          square = NULL;
        } else {
          uBigKaratsubaSquare(big1->bigdigits, big1->size,
              square->bigdigits, temp->bigdigits);
          square->size = big1->size << 1;
          square = normalize(square);
          FREE_BIG2(temp, big1->size << 2);
        } /* if */
      } /* if */
    } else {
      if (likely(ALLOC_BIG(square, big1->size + big1->size))) {
        square->size = big1->size + big1->size;
        uBigDigitSquare(big1->bigdigits, big1->size, square->bigdigits);
        square = normalize(square);
      } /* if */
    } /* if */
    logFunction(printf("uBigSquareK -> size= " FMT_U_MEM "\n", square->size););
    return square;
  } /* uBigSquareK */



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
static bigIntType bigIPowN (const bigDigitType base, intType exponent)

  {
    bigIntType square;
    bigIntType big_help;
    bigIntType power;

  /* bigIPowN */
    logFunction(printf("bigIPowN(" FMT_U_DIG ", " FMT_D ")\n",
                       base, exponent););
    if (unlikely(!ALLOC_BIG_SIZE_OK(square, 1))) {
      raise_error(MEMORY_ERROR);
      power = NULL;
    } else if (unlikely(!ALLOC_BIG_SIZE_OK(power, 1))) {
      FREE_BIG2(square, 1);
      raise_error(MEMORY_ERROR);
    } else {
      square->size = 1;
      square->bigdigits[0] = base;
      power->size = 1;
      if (exponent & 1) {
        power->bigdigits[0] = base;
      } else {
        power->bigdigits[0] = 1;
      } /* if */
      exponent >>= 1;
      while (exponent != 0 && square != NULL && power != NULL) {
        big_help = square;
        square = uBigSquareK(square);
        FREE_BIG(big_help);
        if (square != NULL) {
          if (exponent & 1) {
            big_help = power;
            power = uBigMultK(power, square, FALSE);
            FREE_BIG(big_help);
          } /* if */
          exponent >>= 1;
        } /* if */
      } /* while */
      if (unlikely(square == NULL)) {
        if (power != NULL) {
          FREE_BIG(power);
        } /* if */
        raise_error(MEMORY_ERROR);
        power = NULL;
      } else {
        FREE_BIG(square);
        if (unlikely(power == NULL)) {
          raise_error(MEMORY_ERROR);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigIPowN --> power->size=" FMT_U_MEM "\n",
                       power != NULL ? power->size : 0););
    return power;
  } /* bigIPowN */



/**
 *  Computes base to the power of exponent for signed big integers.
 *  It is assumed that the exponent is >= 1.
 *  The function recognizes the special case of base with a value
 *  of a power of two. In this case the function bigLog2BaseIPow is
 *  used.
 */
static bigIntType bigIPow1 (bigDigitType base, intType exponent)

  {
    boolType negative;
    unsigned int bit_size;
    bigIntType power;

  /* bigIPow1 */
    logFunction(printf("bigIPow1(" FMT_D_DIG ", " FMT_D ")\n",
                       base, exponent););
    if (base == 0) {
      if (unlikely(!ALLOC_BIG_SIZE_OK(power, 1))) {
        raise_error(MEMORY_ERROR);
        power = NULL;
      } else {
        power->size = 1;
        power->bigdigits[0] = 0;
      } /* if */
    } else {
      if (IS_NEGATIVE(base)) {
        /* The unsigned value is negated to avoid a signed integer */
        /* overflow if the smallest signed integer is negated.     */
        base = -base;
        negative = (boolType) (exponent & 1);
      } else {
        negative = FALSE;
      } /* if */
      bit_size = (unsigned int) (digitMostSignificantBit(base) + 1);
      if (base == (bigDigitType) (1 << (bit_size - 1))) {
        power = bigLog2BaseIPow((intType) (bit_size - 1), exponent);
        if (power != NULL) {
          if (negative) {
            negate_positive_big(power);
            power = normalize(power);
          } /* if */
        } /* if */
      } else {
        power = bigIPowN(base, exponent);
        if (power != NULL) {
          if (negative) {
            negate_positive_big(power);
          } /* if */
          power = normalize(power);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigIPow1 --> power->size=" FMT_U_MEM "\n",
                       power != NULL ? power->size : 0););
    return power;
  } /* bigIPow1 */



static int uBigIsNot0 (const const_bigIntType big)

  {
    memSizeType pos = 0;

  /* uBigIsNot0 */
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
 *  @exception MEMORY_ERROR Not enough memory to create the result.
 */
bigIntType bigAbs (const const_bigIntType big1)

  {
    memSizeType pos;
    doubleBigDigitType carry = 1;
    memSizeType absoluteValue_size;
    bigIntType resized_absoluteValue;
    bigIntType absoluteValue;

  /* bigAbs */
    logFunction(printf("bigAbs(%s)\n", bigHexCStri(big1)););
    if (unlikely(!ALLOC_BIG_SIZE_OK(absoluteValue, big1->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      absoluteValue->size = big1->size;
      if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        pos = 0;
        do {
          carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
          absoluteValue->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          pos++;
        } while (pos < big1->size);
        if (IS_NEGATIVE(absoluteValue->bigdigits[pos - 1])) {
          absoluteValue_size = absoluteValue->size + 1;
          REALLOC_BIG_CHECK_SIZE(resized_absoluteValue, absoluteValue,
                                 absoluteValue_size);
          if (unlikely(resized_absoluteValue == NULL)) {
            FREE_BIG2(absoluteValue, big1->size);
            raise_error(MEMORY_ERROR);
            absoluteValue = NULL;
          } else {
            absoluteValue = resized_absoluteValue;
            absoluteValue->size = absoluteValue_size;
            absoluteValue->bigdigits[big1->size] = 0;
          } /* if */
        } /* if */
      } else {
        memcpy(absoluteValue->bigdigits, big1->bigdigits,
               (size_t) big1->size * sizeof(bigDigitType));
      } /* if */
    } /* if */
    logFunction(printf("bigAbs --> %s\n", bigHexCStri(absoluteValue)););
    return absoluteValue;
  } /* bigAbs */



/**
 *  Compute the absolute value of a 'bigInteger' number.
 *  Big1 is assumed to be a temporary value which is reused.
 *  @return the absolute value.
 *  @exception MEMORY_ERROR Not enough memory to create the result.
 */
bigIntType bigAbsTemp (bigIntType big1)

  {
    memSizeType pos;
    doubleBigDigitType carry = 1;
    boolType negative;
    bigIntType resized_big1;

  /* bigAbsTemp */
    logFunction(printf("bigAbsTemp(%s)\n", bigHexCStri(big1)););
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    if (negative) {
      pos = 0;
      do {
        carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
        big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < big1->size);
      if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, pos + 1);
        if (unlikely(resized_big1 == NULL)) {
          FREE_BIG(big1);
          raise_error(MEMORY_ERROR);
          big1 = NULL;
        } else {
          big1 = resized_big1;
          big1->size++;
          big1->bigdigits[pos] = 0;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigAbsTemp --> %s\n", bigHexCStri(big1)););
    return big1;
  } /* bigAbsTemp */



/**
 *  Add two 'bigInteger' numbers.
 *  The function sorts the two values by size. This way there is a
 *  loop up to the shorter size and a second loop up to the longer size.
 *  @return the sum of the two numbers.
 *  @exception MEMORY_ERROR Not enough memory to create the result.
 */
bigIntType bigAdd (const_bigIntType summand1, const_bigIntType summand2)

  {
    const_bigIntType help_big;
    memSizeType pos;
    doubleBigDigitType carry = 0;
    doubleBigDigitType summand2_sign;
    bigIntType sum;

  /* bigAdd */
    logFunction(printf("bigAdd(%s,", bigHexCStri(summand1));
                printf("%s)\n", bigHexCStri(summand2)););
    if (summand2->size > summand1->size) {
      help_big = summand1;
      summand1 = summand2;
      summand2 = help_big;
    } /* if */
    if (unlikely(!ALLOC_BIG_CHECK_SIZE(sum, summand1->size + 1))) {
      raise_error(MEMORY_ERROR);
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
    } /* if */
    logFunction(printf("bigAdd --> %s\n", bigHexCStri(sum)););
    return sum;
  } /* bigAdd */



/**
 *  Increment a 'bigInteger' variable by a delta.
 *  Adds delta to *big_variable. The operation is done in
 *  place and *big_variable is only resized if necessary.
 *  If the size of delta is smaller than *big_variable the
 *  algorithm tries to save computations. Therefore there are
 *  checks for carry == 0 and carry != 0.
 *  In case the resizing fails the content of *big_variable
 *  is freed and *big_variable is set to NULL.
 *  @param delta The delta to be added to *big_variable.
 *  @exception MEMORY_ERROR If the resizing of *big_variable fails.
 */
void bigAddAssign (bigIntType *const big_variable, const const_bigIntType delta)

  {
    bigIntType big1;
    memSizeType pos;
    memSizeType big1_size;
    boolType delta_negative;
    doubleBigDigitType carry = 0;
    doubleBigDigitType big1_sign;
    doubleBigDigitType delta_sign;
    bigIntType resized_big1;

  /* bigAddAssign */
    logFunction(printf("bigAddAssign(%s,", bigHexCStri(*big_variable));
                printf("%s)\n", bigHexCStri(delta)););
    big1 = *big_variable;
    if (big1->size >= delta->size) {
      big1_size = big1->size;
      big1_sign = IS_NEGATIVE(big1->bigdigits[big1_size - 1]) ? BIGDIGIT_MASK : 0;
      /* It is possible that big1 == delta holds. Therefore the check */
      /* for negative delta must be done before big1 is changed.      */
      delta_negative = IS_NEGATIVE(delta->bigdigits[delta->size - 1]);
      pos = 0;
      do {
        carry += (doubleBigDigitType) big1->bigdigits[pos] + delta->bigdigits[pos];
        big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < delta->size);
      if (delta_negative) {
        for (; carry == 0 && pos < big1_size; pos++) {
          carry = (doubleBigDigitType) big1->bigdigits[pos] + BIGDIGIT_MASK;
          big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        carry += BIGDIGIT_MASK;
      } else {
        for (; carry != 0 && pos < big1_size; pos++) {
          carry += big1->bigdigits[pos];
          big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
      } /* if */
      carry += big1_sign;
      carry &= BIGDIGIT_MASK;
      /* Now the only possible values for carry are 0 and BIGDIGIT_MASK. */
      if ((carry != 0 || IS_NEGATIVE(big1->bigdigits[big1_size - 1])) &&
          (carry != BIGDIGIT_MASK || !IS_NEGATIVE(big1->bigdigits[big1_size - 1]))) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, big1_size + 1);
        if (unlikely(resized_big1 == NULL)) {
          FREE_BIG(big1);
          *big_variable = NULL;
          raise_error(MEMORY_ERROR);
        } else {
          /* It is possible that big1 == delta holds. Since */
          /* 'delta' is not used after realloc() enlarged   */
          /* 'big1' a correction of delta is not necessary. */
          big1 = resized_big1;
          big1->size++;
          big1->bigdigits[big1_size] = (bigDigitType) (carry);
          *big_variable = big1;
        } /* if */
      } else {
        *big_variable = normalize(big1);
      } /* if */
    } else {
      REALLOC_BIG_CHECK_SIZE(resized_big1, big1, delta->size + 1);
      if (unlikely(resized_big1 == NULL)) {
        FREE_BIG(big1);
        *big_variable = NULL;
        raise_error(MEMORY_ERROR);
      } else {
        big1 = resized_big1;
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
    logFunction(printf("bigAddAssign --> %s\n", bigHexCStri(*big_variable)););
  } /* bigAddAssign */



/**
 *  Increment a 'bigInteger' variable by a delta.
 *  Adds delta to *big_variable. The operation is done in
 *  place and *big_variable is only resized if necessary.
 *  In case the resizing fails the content of *big_variable
 *  is freed and *big_variable is set to NULL.
 *  @param delta The delta to be added to *big_variable.
 *         Delta must be in the range of signedBigDigitType.
 *  @exception MEMORY_ERROR If the resizing of *big_variable fails.
 */
void bigAddAssignSignedDigit (bigIntType *const big_variable, const intType delta)

  {
    bigIntType big1;
    memSizeType pos;
    memSizeType big1_size;
    doubleBigDigitType carry = 0;
    doubleBigDigitType big1_sign;
    bigIntType resized_big1;

  /* bigAddAssignSignedDigit */
    logFunction(printf("bigAddAssignSignedDigit(%s, " FMT_D ")\n",
                       bigHexCStri(*big_variable), delta););
    big1 = *big_variable;
    big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
    carry += (doubleBigDigitType) big1->bigdigits[0] + (bigDigitType) (delta & BIGDIGIT_MASK);
    big1->bigdigits[0] = (bigDigitType) (carry & BIGDIGIT_MASK);
    carry >>= BIGDIGIT_SIZE;
    pos = 1;
    if (delta < 0) {
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
      REALLOC_BIG_CHECK_SIZE(resized_big1, big1, big1_size + 1);
      if (unlikely(resized_big1 == NULL)) {
        FREE_BIG(big1);
        *big_variable = NULL;
        raise_error(MEMORY_ERROR);
      } else {
        big1 = resized_big1;
        big1->size++;
        big1->bigdigits[big1_size] = (bigDigitType) (carry & BIGDIGIT_MASK);
        *big_variable = big1;
      } /* if */
    } else {
      *big_variable = normalize(big1);
    } /* if */
    logFunction(printf("bigAddAssignSignedDigit --> %s\n",
                       bigHexCStri(*big_variable)););
  } /* bigAddAssignSignedDigit */



/**
 *  Add two 'bigInteger' numbers.
 *  Summand1 is assumed to be a temporary value which is reused.
 *  @return the sum of the two numbers in 'summand1'.
 */
bigIntType bigAddTemp (bigIntType summand1, const const_bigIntType summand2)

  { /* bigAddTemp */
    bigAddAssign(&summand1, summand2);
    return summand1;
  } /* bigAddTemp */



bigIntType bigAnd (const_bigIntType big1, const_bigIntType big2)

  {
    const_bigIntType help_big;
    memSizeType pos;
    bigDigitType big2_sign;
    bigIntType result;

  /* bigAnd */
    logFunction(printf("bigAnd(%s,", bigHexCStri(big1));
                printf("%s)\n", bigHexCStri(big2)););
    if (big2->size > big1->size) {
      help_big = big1;
      big1 = big2;
      big2 = help_big;
    } /* if */
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, big1->size))) {
      raise_error(MEMORY_ERROR);
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
    } /* if */
    logFunction(printf("bigAnd --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigAnd */



/**
 *  Number of bits in the minimum two's-complement representation.
 *  The high bits equivalent to the sign bit are not part of the
 *  minimum two's-complement representation.
 *   bigBitLength(0_)   returns 0
 *   bigBitLength(1_)   returns 1
 *   bigBitLength(4_)   returns 3
 *   bigBitLength(-1_)  returns 0
 *   bigBitLength(-2_)  returns 1
 *   bigBitLength(-4_)  returns 2
 *  @return the number of bits.
 *  @exception RANGE_ERROR The result does not fit into an integer.
 */
intType bigBitLength (const const_bigIntType big1)

  {
    intType bitLength;

  /* bigBitLength */
    logFunction(printf("bigBitLength(%s)\n", bigHexCStri(big1)););
    if (unlikely(big1->size >= MAX_MEM_INDEX >> BIGDIGIT_LOG2_SIZE)) {
      logError(printf("bigBitLength(%s): "
                      "Result does not fit into an integer.\n",
                      bigHexCStri(big1)););
      raise_error(RANGE_ERROR);
      bitLength = 0;
    } else {
      bitLength = (intType) ((big1->size - 1) << BIGDIGIT_LOG2_SIZE);
      if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        bitLength += digitMostSignificantBit(~big1->bigdigits[big1->size - 1]) + 1;
      } else {
        bitLength += digitMostSignificantBit(big1->bigdigits[big1->size - 1]) + 1;
      } /* if */
    } /* if */
    logFunction(printf("bigBitLength --> " FMT_D "\n", bitLength););
    return bitLength;
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
    pos = big1->size;
    big1_negative = IS_NEGATIVE(big1->bigdigits[pos - 1]);
    big2_negative = IS_NEGATIVE(big2->bigdigits[big2->size - 1]);
    if (big1_negative != big2_negative) {
      return big1_negative ? -1 : 1;
    } else if (pos != big2->size) {
      return (pos < big2->size) != big1_negative ? -1 : 1;
    } else {
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
 *  if sizeof(genericType) != sizeof(bigIntType).
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
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
    intType signumValue;

  /* bigCmpSignedDigit */
    if (number < 0) {
      if (!IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        signumValue = 1;
      } else if (big1->size != 1) {
        signumValue = -1;
      } else if (big1->bigdigits[0] < (bigDigitType) number) {
        signumValue = -1;
      } else {
        signumValue = big1->bigdigits[0] > (bigDigitType) number;
      } /* if */
    } else {
      if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        signumValue = -1;
      } else if (big1->size != 1) {
        signumValue = 1;
      } else if (big1->bigdigits[0] < (bigDigitType) number) {
        signumValue = -1;
      } else {
        signumValue = big1->bigdigits[0] > (bigDigitType) number;
      } /* if */
    } /* if */
    return signumValue;
  } /* bigCmpSignedDigit */



/**
 *  Assign source to *dest.
 *  A copy function assumes that *dest contains a legal value.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void bigCpy (bigIntType *const dest, const const_bigIntType source)

  {
    memSizeType new_size;
    bigIntType big_dest;

  /* bigCpy */
    big_dest = *dest;
    new_size = source->size;
    if (big_dest->size != new_size) {
      if (unlikely(!ALLOC_BIG_SIZE_OK(big_dest, new_size))) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        FREE_BIG(*dest);
        big_dest->size = new_size;
        *dest = big_dest;
      } /* if */
    } /* if */
    /* It is possible that *dest == source holds. The    */
    /* behavior of memcpy() is undefined if source and   */
    /* destination areas overlap (or are identical).     */
    /* Therefore memmove() is used instead of memcpy().  */
    memmove(big_dest->bigdigits, source->bigdigits,
            (size_t) new_size * sizeof(bigDigitType));
  } /* bigCpy */



/**
 *  Reinterpret the generic parameters as bigIntType and call bigCpy.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(bigIntType).
 */
void bigCpyGeneric (genericType *const dest, const genericType source)

  { /* bigCpyGeneric */
    bigCpy(&((rtlObjectType *) dest)->value.bigIntValue,
           ((const_rtlObjectType *) &source)->value.bigIntValue);
  } /* bigCpyGeneric */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigCreate (const const_bigIntType source)

  {
    memSizeType new_size;
    bigIntType result;

  /* bigCreate */
    new_size = source->size;
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, new_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = new_size;
      memcpy(result->bigdigits, source->bigdigits,
             (size_t) new_size * sizeof(bigDigitType));
    } /* if */
    return result;
  } /* bigCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(bigIntType).
 */
genericType bigCreateGeneric (const genericType source)

  {
    rtlObjectType result;

  /* bigCreateGeneric */
    INIT_GENERIC_PTR(result.value.genericValue);
    result.value.bigIntValue =
        bigCreate(((const_rtlObjectType *) &source)->value.bigIntValue);
    return result.value.genericValue;
  } /* bigCreateGeneric */



/**
 *  Decrement a 'bigInteger' variable.
 *  Decrements *big_variable by 1. The operation is done in
 *  place and *big_variable is only enlarged if necessary.
 *  In case the enlarging fails the old content of *big_variable
 *  is restored and the exception MEMORY_ERROR is raised.
 *  This ensures that bigDecr works as a transaction.
 *  @exception MEMORY_ERROR If the resizing of *big_variable fails.
 */
void bigDecr (bigIntType *const big_variable)

  {
    bigIntType big1;
    memSizeType pos = 0;
    boolType negative;
    bigIntType resized_big1;

  /* bigDecr */
    logFunction(printf("bigDecr(%s)\n", bigHexCStri(*big_variable)););
    big1 = *big_variable;
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    if (big1->bigdigits[pos] == 0) {
      if (big1->size == 1) {
        big1->bigdigits[pos] = BIGDIGIT_MASK;
        pos++;
      } else {
        do {
          big1->bigdigits[pos] = BIGDIGIT_MASK;
          pos++;
        } while (big1->bigdigits[pos] == 0);
      } /* if */
    } /* if */
    if (pos < big1->size) {
      big1->bigdigits[pos]--;
    } /* if */
    pos = big1->size;
    if (!IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      if (negative) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, pos + 1);
        if (unlikely(resized_big1 == NULL)) {
          /* This error situation is very unlikely, but we need to */
          /* make sure that 'big_variable' contains a legal value. */
          /* We UNDO the change done for 'big_variable' by setting */
          /* it to the old value: The highest bit is set to 1 and  */
          /* the other bits are set to 0. Note that only values    */
          /* with this pattern need an additional digit if they    */
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
          big1->size++;
          big1->bigdigits[pos] = BIGDIGIT_MASK;
          *big_variable = big1;
        } /* if */
      } else if (big1->bigdigits[pos - 1] == 0 &&
          pos >= 2 && !IS_NEGATIVE(big1->bigdigits[pos - 2])) {
        REALLOC_BIG_SIZE_OK(resized_big1, big1, pos - 1);
        /* Avoid a MEMORY_ERROR in the strange case   */
        /* if a 'realloc' which shrinks memory fails. */
        if (likely(resized_big1 != NULL)) {
          big1 = resized_big1;
          *big_variable = big1;
        } /* if */
        big1->size--;
      } /* if */
    } /* if */
    logFunction(printf("bigDecr --> %s\n", bigHexCStri(*big_variable)););
  } /* bigDecr */



/**
 *  Free the memory referred by 'old_bigint'.
 *  After bigDestr is left 'old_bigint' refers to not existing memory.
 *  The memory where 'old_bigint' is stored can be freed afterwards.
 */
void bigDestr (const const_bigIntType old_bigint)

  { /* bigDestr */
    if (old_bigint != NULL) {
      FREE_BIG(old_bigint);
    } /* if */
  } /* bigDestr */



/**
 *  Generic Destr function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(bigIntType).
 */
void bigDestrGeneric (const genericType old_value)

  { /* bigDestrGeneric */
    bigDestr(((const_rtlObjectType *) &old_value)->value.bigIntValue);
  } /* bigDestrGeneric */



/**
 *  Integer division truncated towards zero.
 *  The remainder of this division is computed with bigRem.
 *  The memory for the result is requested and the normalized
 *  result is returned. If divisor has just one digit or if
 *  dividend has less digits than divisor the bigDiv1() or
 *  bigDivSizeLess() functions are called. In the general case
 *  the absolute values of dividend and divisor are taken. Then
 *  dividend is extended by one leading zero digit. After that
 *  dividend and divisor are shifted to the left such that the
 *  most significant bit of divisor is set. This fulfills the
 *  preconditions for calling uBigDiv() which does the main
 *  work of the division.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
bigIntType bigDiv (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    boolType negative = FALSE;
    bigIntType dividend_help;
    bigIntType divisor_help;
    unsigned int shift;
    bigIntType quotient;

  /* bigDiv */
    logFunction(printf("bigDiv(%s, ", bigHexCStri(dividend));
                printf("%s)\n", bigHexCStri(divisor)););
    if (divisor->size == 1) {
      quotient = bigDiv1(dividend, divisor->bigdigits[0]);
    } else if (dividend->size < divisor->size) {
      quotient = bigDivSizeLess(dividend, divisor);
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
        FREE_BIG2(dividend_help, dividend->size + 2);
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
        FREE_BIG2(dividend_help, dividend->size + 2);
        FREE_BIG2(divisor_help, divisor->size + 1);
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        quotient->size = dividend_help->size - divisor_help->size + 1;
        quotient->bigdigits[quotient->size - 1] = 0;
        shift = (unsigned int)
            (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
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
      FREE_BIG2(dividend_help, dividend->size + 2);
      FREE_BIG2(divisor_help, divisor->size + 1);
    } /* if */
    logFunction(printf("bigDiv --> %s\n", bigHexCStri(quotient)););
    return quotient;
  } /* bigDiv */



/**
 *  Integer division truncated towards zero.
 *  The memory for the quotient is requested and the normalized
 *  quotient is returned. The memory for the remainder is
 *  requested and the normalized remainder is assigned to
 *  *remainderAddr. If divisor has just one digit or if
 *  dividend has less digits than divisor the bigDivRem1() or
 *  bigDivRemSizeLess() functions are called. In the general case
 *  the absolute values of dividend and divisor are taken. Then
 *  dividend is extended by one leading zero digit. After that
 *  dividend and divisor are shifted to the left such that the
 *  most significant bit of divisor is set. This fulfills the
 *  preconditions for calling uBigDiv() which does the main
 *  work of the division.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
bigIntType bigDivRem (const const_bigIntType dividend, const const_bigIntType divisor,
    bigIntType *remainderAddr)

  {
    boolType quotientNegative = FALSE;
    boolType remainderNegative = FALSE;
    bigIntType divisor_help;
    unsigned int shift;
    bigIntType quotient;
    bigIntType remainder;

  /* bigDivRem */
    logFunction(printf("bigDivRem(%s, ", bigHexCStri(dividend));
                printf("%s, *)\n", bigHexCStri(divisor)););
    if (divisor->size == 1) {
      quotient = bigDivRem1(dividend, divisor->bigdigits[0], remainderAddr);
    } else if (dividend->size < divisor->size) {
      quotient = bigDivRemSizeLess(dividend, divisor, remainderAddr);
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(remainder, dividend->size + 2))) {
        *remainderAddr = NULL;
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          quotientNegative = TRUE;
          remainderNegative = TRUE;
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
        FREE_BIG2(remainder, dividend->size + 2);
        *remainderAddr = NULL;
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(divisor->bigdigits[divisor->size - 1])) {
          quotientNegative = !quotientNegative;
          positive_copy_of_negative_big(divisor_help, divisor);
        } else {
          divisor_help->size = divisor->size;
          memcpy(divisor_help->bigdigits, divisor->bigdigits,
                 (size_t) divisor->size * sizeof(bigDigitType));
        } /* if */
      } /* if */
      if (unlikely(!ALLOC_BIG_SIZE_OK(quotient, remainder->size - divisor_help->size + 1))) {
        FREE_BIG2(remainder, dividend->size + 2);
        FREE_BIG2(divisor_help, divisor->size + 1);
        *remainderAddr = NULL;
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        quotient->size = remainder->size - divisor_help->size + 1;
        quotient->bigdigits[quotient->size - 1] = 0;
        shift = (unsigned int)
            (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
        if (shift == 0) {
          /* The most significant digit of divisor_help is 0. Just ignore it */
          remainder->size--;
          divisor_help->size--;
          if (divisor_help->size == 1) {
            remainder->bigdigits[0] = uBigDivRem1(remainder, divisor_help->bigdigits[0], quotient);
            memset(&remainder->bigdigits[1], 0,
                   (size_t) (remainder->size - 1) * sizeof(bigDigitType));
          } else {
            uBigDiv(remainder, divisor_help, quotient);
          } /* if */
          remainder->bigdigits[remainder->size] = 0;
          divisor_help->size++;
        } else {
          shift = BIGDIGIT_SIZE - shift;
          uBigLShift(remainder, shift);
          uBigLShift(divisor_help, shift);
          uBigDiv(remainder, divisor_help, quotient);
          uBigRShift(remainder, shift);
        } /* if */
        remainder->bigdigits[dividend->size + 1] = 0;
        remainder->size = dividend->size + 2;
        if (quotientNegative) {
          negate_positive_big(quotient);
        } /* if */
        quotient = normalize(quotient);
        if (remainderNegative) {
          negate_positive_big(remainder);
        } /* if */
        remainder = normalize(remainder);
      } /* if */
      FREE_BIG2(divisor_help, divisor->size + 1);
      *remainderAddr = remainder;
    } /* if */
    logFunction(printf("bigDivRem --> %s", bigHexCStri(quotient));
                printf(" (%s)\n", bigHexCStri(*remainderAddr)););
    return quotient;
  } /* bigDivRem */



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



/**
 *  Convert a byte buffer (interpreted as big-endian) to a bigInteger.
 *  @param size Size of the byte buffer to be converted (in bytes).
 *  @param buffer Byte buffer to be converted. The bytes are interpreted
 *         as binary big-endian representation with a base of 256.
 *  @param isSigned Defines if 'buffer' is interpreted as signed value.
 *         If 'isSigned' is TRUE the twos-complement representation
 *         is used. In this case the result is negative if the most
 *         significant byte (the first byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bigInteger created from the big-endian bytes.
 *  @exception RANGE_ERROR If 'size' is zero ('buffer' is empty).
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
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
    logFunction(printf("bigFromByteBufferBe(" FMT_U_MEM ", 0x" FMT_X_MEM ", %d)\n",
                       size, (memSizeType) buffer, isSigned););
    if (unlikely(size == 0)) {
      logError(printf("bigFromByteBufferBe(0, \"\", %d): "
                      "Buffer is empty.\n", isSigned););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      num_bigdigits = (size + (BIGDIGIT_SIZE >> 3) - 1) / (BIGDIGIT_SIZE >> 3);
      result_size = num_bigdigits;
      if (!isSigned && size % (BIGDIGIT_SIZE >> 3) == 0 && buffer[0] > BYTE_MAX) {
        /* The number is unsigned, but highest bit is one: */
        /* A leading zero bigdigit must be added.          */
        result_size++;
      } /* if */
      if (unlikely(!ALLOC_BIG(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        result->size = result_size;
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
        if (isSigned && buffer[0] > BYTE_MAX) {
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
        result = normalize(result);
      } /* if */
    } /* if */
    logFunction(printf("bigFromByteBufferBe --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigFromByteBufferBe */



/**
 *  Convert a byte buffer (interpreted as little-endian) to a bigInteger.
 *  @param size Size of the byte buffer to be converted (in bytes).
 *  @param buffer Byte buffer to be converted. The bytes are interpreted
 *         as binary little-endian representation with a base of 256.
 *  @param isSigned Defines if 'buffer' is interpreted as signed value.
 *         If 'isSigned' is TRUE the twos-complement representation
 *         is used. In this case the result is negative if the most
 *         significant byte (the last byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bigInteger created from the little-endian bytes.
 *  @exception RANGE_ERROR If 'size' is zero ('buffer' is empty).
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
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
    logFunction(printf("bigFromByteBufferLe(" FMT_U_MEM ", 0x" FMT_X_MEM ", %d)\n",
                       size, (memSizeType) buffer, isSigned););
    if (unlikely(size == 0)) {
      logError(printf("bigFromByteBufferLe(0, \"\", %d): "
                      "Buffer is empty.\n", isSigned););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      num_bigdigits = (size + (BIGDIGIT_SIZE >> 3) - 1) / (BIGDIGIT_SIZE >> 3);
      result_size = num_bigdigits;
      if (!isSigned && buffer[size - 1] > BYTE_MAX) {
        /* The number is unsigned, but highest bit is one: */
        /* A leading zero bigdigit must be added.          */
        result_size++;
      } /* if */
      if (unlikely(!ALLOC_BIG(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        result->size = result_size;
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
        if (isSigned && buffer[size - 1] > BYTE_MAX) {
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
        result = normalize(result);
      } /* if */
    } /* if */
    logFunction(printf("bigFromByteBufferLe --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigFromByteBufferLe */



/**
 *  Convert a bstring (interpreted as big-endian) to a bigInteger.
 *  @param bstri Bstring to be converted. The bytes are interpreted
 *         as binary big-endian representation with a base of 256.
 *  @param isSigned Defines if 'bstri' is interpreted as signed value.
 *         If 'isSigned' is TRUE the twos-complement representation
 *         is used. In this case the result is negative if the most
 *         significant byte (the first byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bigInteger created from the big-endian bytes.
 *  @exception RANGE_ERROR If 'bStri' is empty.
 */
bigIntType bigFromBStriBe (const const_bstriType bstri, const boolType isSigned)

  { /* bigFromBStriBe */
    logFunction(printf("bigFromBStriBe(\"%s\", %d)\n",
                       bstriAsUnquotedCStri(bstri), isSigned););
    return bigFromByteBufferBe(bstri->size, bstri->mem, isSigned);
  } /* bigFromBStriBe */



/**
 *  Convert a bstring (interpreted as little-endian) to a bigInteger.
 *  @param bstri Bstring to be converted. The bytes are interpreted
 *         as binary little-endian representation with a base of 256.
 *  @param isSigned Defines if 'bstri' is interpreted as signed value.
 *         If 'isSigned' is TRUE the twos-complement representation
 *         is used. In this case the result is negative if the most
 *         significant byte (the last byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bigInteger created from the little-endian bytes.
 *  @exception RANGE_ERROR If 'bStri' is empty.
 */
bigIntType bigFromBStriLe (const const_bstriType bstri, const boolType isSigned)

  { /* bigFromBStriLe */
    logFunction(printf("bigFromBStriLe(\"%s\", %d)\n",
                       bstriAsUnquotedCStri(bstri), isSigned););
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
    logFunction(printf("bigFromInt32(" FMT_D32 ")\n", number););
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
    logFunction(printf("bigFromInt32 --> %s\n", bigHexCStri(result)););
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
    logFunction(printf("bigFromInt64(" FMT_D64 ")\n", number););
    result_size = sizeof(int64Type) / (BIGDIGIT_SIZE >> 3);
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
      for (pos = 0; pos < result_size; pos++) {
        result->bigdigits[pos] = (bigDigitType) (number & BIGDIGIT_MASK);
        number >>= BIGDIGIT_SIZE;
      } /* for */
      result = normalize(result);
    } /* if */
    logFunction(printf("bigFromInt64 --> %s\n", bigHexCStri(result)););
    return result;
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
    logFunction(printf("bigFromUInt32(" FMT_U32 ")\n", number););
#if BIGDIGIT_SIZE == 32
    if (number > UINT32_SUFFIX(2147483647)) {
      result_size = 2;
    } else {
      result_size = 1;
    } /* if */
#elif BIGDIGIT_SIZE == 16
    if (number > UINT32_SUFFIX(2147483647)) {
      result_size = 3;
    } else if (number > UINT32_SUFFIX(32767)) {
      result_size = 2;
    } else {
      result_size = 1;
    } /* if */
#elif BIGDIGIT_SIZE == 8
    if (number > UINT32_SUFFIX(2147483647)) {
      result_size = 5;
    } else if (number > UINT32_SUFFIX(8388607)) {
      result_size = 4;
    } else if (number > UINT32_SUFFIX(32767)) {
      result_size = 3;
    } else if (number > UINT32_SUFFIX(127)) {
      result_size = 2;
    } else {
      result_size = 1;
    } /* if */
#endif
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
      if (result_size == sizeof(uint32Type) / (BIGDIGIT_SIZE >> 3) + 1) {
        result_size--;
        result->bigdigits[result_size] = (bigDigitType) 0;
      } /* if */
      result->bigdigits[0] = (bigDigitType) (number & BIGDIGIT_MASK);
#if BIGDIGIT_SIZE < 32
      {
        memSizeType pos;

        for (pos = 1; pos < result_size; pos++) {
          number >>= BIGDIGIT_SIZE;
          result->bigdigits[pos] = (bigDigitType) (number & BIGDIGIT_MASK);
        } /* for */
      }
#endif
    } /* if */
    logFunction(printf("bigFromUInt32 --> %s\n", bigHexCStri(result)););
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
    logFunction(printf("bigFromUInt64(" FMT_U64 ")\n", number););
#if BIGDIGIT_SIZE == 32
    if (number > UINT64_SUFFIX(9223372036854775807)) {
      result_size = 3;
    } else if (number > UINT64_SUFFIX(2147483647)) {
      result_size = 2;
    } else {
      result_size = 1;
    } /* if */
#elif BIGDIGIT_SIZE == 16
    if (number > UINT64_SUFFIX(9223372036854775807)) {
      result_size = 5;
    } else if (number > UINT64_SUFFIX(140737488355327)) {
      result_size = 4;
    } else if (number > UINT64_SUFFIX(2147483647)) {
      result_size = 3;
    } else if (number > UINT64_SUFFIX(32767)) {
      result_size = 2;
    } else {
      result_size = 1;
    } /* if */
#elif BIGDIGIT_SIZE == 8
    if (number > UINT64_SUFFIX(9223372036854775807)) {
      result_size = 9;
    } else if (number > UINT64_SUFFIX(36028797018963967)) {
      result_size = 8;
    } else if (number > UINT64_SUFFIX(140737488355327)) {
      result_size = 7;
    } else if (number > UINT64_SUFFIX(549755813887)) {
      result_size = 6;
    } else if (number > UINT64_SUFFIX(2147483647)) {
      result_size = 5;
    } else if (number > UINT64_SUFFIX(8388607)) {
      result_size = 4;
    } else if (number > UINT64_SUFFIX(32767)) {
      result_size = 3;
    } else if (number > UINT64_SUFFIX(127)) {
      result_size = 2;
    } else {
      result_size = 1;
    } /* if */
#endif
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
      if (result_size == sizeof(uint64Type) / (BIGDIGIT_SIZE >> 3) + 1) {
        result_size--;
        result->bigdigits[result_size] = (bigDigitType) 0;
      } /* if */
      result->bigdigits[0] = (bigDigitType) (number & BIGDIGIT_MASK);
      for (pos = 1; pos < result_size; pos++) {
        number >>= BIGDIGIT_SIZE;
        result->bigdigits[pos] = (bigDigitType) (number & BIGDIGIT_MASK);
      } /* for */
    } /* if */
    logFunction(printf("bigFromUInt64 --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigFromUInt64 */
#endif



/**
 *  Compute the greatest common divisor of two 'bigInteger' numbers.
 *  @return the greatest common divisor of the two numbers.
 *          The greatest common divisor is positive or zero.
 */
bigIntType bigGcd (const const_bigIntType big1,
    const const_bigIntType big2)

  {
    bigIntType big1_help;
    bigIntType big2_help;
    intType lowestSetBitA;
    intType shift;
    bigIntType help_big;
    bigIntType gcd;

  /* bigGcd */
    logFunction(printf("bigGcd(%s,", bigHexCStri(big1));
                printf("%s)\n", bigHexCStri(big2)););
    if (big1->size == 1 && big1->bigdigits[0] == 0) {
      gcd = bigAbs(big2);
    } else if (big2->size == 1 && big2->bigdigits[0] == 0) {
      gcd = bigAbs(big1);
    } else if (unlikely((big1_help = bigAbs(big1)) == NULL)) {
      /* An exception was raised in bigAbs(). */
      gcd = NULL;
    } else if (unlikely((big2_help = bigAbs(big2)) == NULL)) {
      /* An exception was raised in bigAbs(). */
      bigDestr(big1_help);
      gcd = NULL;
    } else {
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
        gcd = big2_help;
        bigDestr(big1_help);
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
            bigSbtrAssign(&(big2_help), big1_help);
          } else {
            help_big = bigSbtr(big1_help, big2_help);
            bigDestr(big1_help);
            big1_help = big2_help;
            big2_help = help_big;
          } /* if */
        } while (big2_help->size != 1 || big2_help->bigdigits[0] != 0);
        bigLShiftAssign(&big1_help, shift);
        gcd = big1_help;
        bigDestr(big2_help);
      } /* if */
    } /* if */
    logFunction(printf("bigGcd -->%s\n", bigHexCStri(gcd)););
    return gcd;
  } /* bigGcd */



/**
 *  Compute the hash value of a 'bigInteger' number.
 *  @return the hash value.
 */
intType bigHashCode (const const_bigIntType big1)

  {
    intType hashCode;

  /* bigHashCode */
    hashCode = (intType)
        (big1->bigdigits[0] << 5 ^ big1->size << 3 ^ big1->bigdigits[big1->size - 1]);
    return hashCode;
  } /* bigHashCode */



/**
 *  Generic hashCode function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(bigIntType).
 */
intType bigHashCodeGeneric (const genericType aValue)

  { /* bigHashCodeGeneric */
    return bigHashCode(((const_rtlObjectType *) &aValue)->value.bigIntValue);
  } /* bigHashCodeGeneric */



/**
 *  Increment a 'bigInteger' variable.
 *  Increments *big_variable by 1. The operation is done in
 *  place and *big_variable is only enlarged if necessary.
 *  In case the enlarging fails the old content of *big_variable
 *  is restored and the exception MEMORY_ERROR is raised.
 *  This ensures that bigIncr works as a transaction.
 *  @exception MEMORY_ERROR If the resizing of *big_variable fails.
 */
void bigIncr (bigIntType *const big_variable)

  {
    bigIntType big1;
    memSizeType pos = 0;
    boolType negative;
    bigIntType resized_big1;

  /* bigIncr */
    logFunction(printf("bigIncr(%s)\n", bigHexCStri(*big_variable)););
    big1 = *big_variable;
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    if (big1->bigdigits[pos] == BIGDIGIT_MASK) {
      if (big1->size == 1) {
        big1->bigdigits[pos] = 0;
        pos++;
      } else {
        do {
          big1->bigdigits[pos] = 0;
          pos++;
        } while (big1->bigdigits[pos] == BIGDIGIT_MASK);
      } /* if */
    } /* if */
    if (pos < big1->size) {
      big1->bigdigits[pos]++;
    } /* if */
    pos = big1->size;
    if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      if (!negative) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, pos + 1);
        if (unlikely(resized_big1 == NULL)) {
          /* This error situation is very unlikely, but we need to */
          /* make sure that 'big_variable' contains a legal value. */
          /* We UNDO the change done for 'big_variable' by setting */
          /* it to the old value: The highest bit is set to 0 and  */
          /* the other bits are set to 1. Note that only values    */
          /* with this pattern need an additional digit if they    */
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
          big1->size++;
          big1->bigdigits[pos] = 0;
          *big_variable = big1;
        } /* if */
      } else if (big1->bigdigits[pos - 1] == BIGDIGIT_MASK &&
          pos >= 2 && IS_NEGATIVE(big1->bigdigits[pos - 2])) {
        REALLOC_BIG_SIZE_OK(resized_big1, big1, pos - 1);
        /* Avoid a MEMORY_ERROR in the strange case   */
        /* if a 'realloc' which shrinks memory fails. */
        if (likely(resized_big1 != NULL)) {
          big1 = resized_big1;
          *big_variable = big1;
        } /* if */
        big1->size--;
      } /* if */
    } /* if */
    logFunction(printf("bigIncr --> %s\n", bigHexCStri(*big_variable)););
  } /* bigIncr */



/**
 *  Compute the exponentiation of a 'bigInteger' base with an integer exponent.
 *  The result variable is set to base or 1 depending on the
 *  rightmost bit of the exponent. After that the base is
 *  squared in a loop and every time the corresponding bit of
 *  the exponent is set the current square is multiplied
 *  with the result variable. This reduces the number of square
 *  operations to ld(exponent).
 *  @return the result of the exponentiation.
 *  @exception NUMERIC_ERROR If the exponent is negative.
 */
bigIntType bigIPow (const const_bigIntType base, intType exponent)

  {
    boolType negative = FALSE;
    bigIntType square;
    bigIntType big_help;
    bigIntType power;

  /* bigIPow */
    logFunction(printf("bigIPow(%s, " FMT_D ")\n",
                       bigHexCStri(base), exponent););
    if (exponent <= 1) {
      if (exponent == 0) {
        if (unlikely(!ALLOC_BIG_SIZE_OK(power, 1))) {
          raise_error(MEMORY_ERROR);
        } else {
          power->size = 1;
          power->bigdigits[0] = 1;
        } /* if */
      } else if (exponent == 1) {
        power = bigCreate(base);
      } else {
        logError(printf("bigIPow(%s, " FMT_D "): "
                        "Exponent is negative.\n",
                        bigHexCStri(base), exponent););
        raise_error(NUMERIC_ERROR);
        power = NULL;
      } /* if */
    } else if (base->size == 1) {
      power = bigIPow1(base->bigdigits[0], exponent);
    } else if (unlikely(!ALLOC_BIG_CHECK_SIZE(square, base->size + 1))) {
      raise_error(MEMORY_ERROR);
      power = NULL;
    } else if (unlikely(!ALLOC_BIG_CHECK_SIZE(power, base->size + 1))) {
      FREE_BIG2(square, base->size + 1);
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
        power->size = square->size;
        memcpy(power->bigdigits, square->bigdigits,
               (size_t) square->size * sizeof(bigDigitType));
      } else {
        negative = FALSE;
        power->size = 1;
        power->bigdigits[0] = 1;
      } /* if */
      exponent >>= 1;
      while (exponent != 0 && square != NULL && power != NULL) {
        big_help = square;
        square = uBigSquareK(square);
        FREE_BIG(big_help);
        if (square != NULL) {
          if (exponent & 1) {
            big_help = power;
            power = uBigMultK(power, square, FALSE);
            FREE_BIG(big_help);
          } /* if */
          exponent >>= 1;
        } /* if */
      } /* while */
      if (unlikely(square == NULL)) {
        if (power != NULL) {
          FREE_BIG(power);
        } /* if */
        raise_error(MEMORY_ERROR);
        power = NULL;
      } else {
        FREE_BIG(square);
        if (unlikely(power == NULL)) {
          raise_error(MEMORY_ERROR);
        } else {
          if (negative) {
            negate_positive_big(power);
          } /* if */
          power = normalize(power);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigIPow --> %s (size=" FMT_U_MEM ")\n",
                       bigHexCStri(power), power != NULL ? power->size : 0););
    return power;
  } /* bigIPow */



/**
 *  Compute the exponentiation of a bigdigit base with an integer exponent.
 *  @param base Base that must be in the range of signedBigDigitType.
 *  @return the result of the exponentiation.
 *  @exception NUMERIC_ERROR If the exponent is negative.
 */
bigIntType bigIPowSignedDigit (intType base, intType exponent)

  {
    bigIntType power;

  /* bigIPowSignedDigit */
    logFunction(printf("bigIPowSignedDigit(" FMT_D ", " FMT_D ")\n",
                       base, exponent););
    if (exponent <= 1) {
      if (unlikely(exponent < 0)) {
        logError(printf("bigIPowSignedDigit(" FMT_D ", " FMT_D "): "
                        "Exponent is negative.\n",
                        base, exponent););
        raise_error(NUMERIC_ERROR);
        power = NULL;
      } else {
        if (unlikely(!ALLOC_BIG_SIZE_OK(power, 1))) {
          raise_error(MEMORY_ERROR);
        } else {
          power->size = 1;
          power->bigdigits[0] = exponent == 1 ? (bigDigitType) base : 1;
        } /* if */
      } /* if */
    } else {
      power = bigIPow1((bigDigitType) base, exponent);
    } /* if */
    logFunction(printf("bigIPowSignedDigit --> %s (size=" FMT_U_MEM ")\n",
                       bigHexCStri(power), power != NULL ? power->size : 0););
    return power;
  } /* bigIPowSignedDigit */



/**
 *  Compute the truncated base 10 logarithm of a 'bigInteger' number.
 *  The definition of 'log10' is extended by defining log10(0) = -1_.
 *  @return the truncated base 10 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
bigIntType bigLog10 (const const_bigIntType big1)

  {
    bigIntType unsigned_big;
    bigIntType powerOf10;
    bigIntType dividend;
    bigDigitType digit;
    memSizeType largeDecimalBlockCount;
    memSizeType decimalBlockCount;
    bigIntType logarithm;

  /* bigLog10 */
    logFunction(printf("bigLog10(%s)\n", bigHexCStri(big1)););
    if (unlikely(IS_NEGATIVE(big1->bigdigits[big1->size - 1]))) {
      logError(printf("bigLog10(%s): Number is negative.\n",
                      bigHexCStri(big1)););
      raise_error(NUMERIC_ERROR);
      logarithm = NULL;
    } else if (big1->size == 1 && big1->bigdigits[0] == 0) {
      if (unlikely(!ALLOC_BIG_SIZE_OK(logarithm, 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        logarithm->size = 1;
        logarithm->bigdigits[0] = BIGDIGIT_MASK;
      } /* if */
    } else {
      if (unlikely(!ALLOC_BIG_SIZE_OK(unsigned_big, big1->size))) {
        raise_error(MEMORY_ERROR);
        logarithm = NULL;
      } else {
        unsigned_big->size = big1->size;
        memcpy(unsigned_big->bigdigits, big1->bigdigits,
               (size_t) big1->size * sizeof(bigDigitType));
        decimalBlockCount = 0;
        if (unsigned_big->size > 2) {
          if (unsigned_big->size - 1 <= MAX_MEM_INDEX) {
            decimalBlockCount = unsigned_big->size - 1;
          } else {
            decimalBlockCount = MAX_MEM_INDEX;
          } /* if */
          powerOf10 = bigIPow1(POWER_OF_10_IN_BIGDIGIT, (intType) (decimalBlockCount));
          if (unlikely(powerOf10 == NULL)) {
            FREE_BIG2(unsigned_big, big1->size);
            unsigned_big = NULL;
          } else {
            dividend = unsigned_big;
            unsigned_big = bigMDiv(dividend, powerOf10);
            FREE_BIG2(dividend, big1->size);
            bigDestr(powerOf10);
          } /* if */
        } /* if */
        if (unlikely(unsigned_big == NULL)) {
          logarithm = NULL;
        } else {
          largeDecimalBlockCount = 0;
          while (unsigned_big->size > 2) {
            uBigRShift(unsigned_big, QUINARY_DIGITS_IN_BIGDIGIT);
            if (unsigned_big->bigdigits[unsigned_big->size - 1] == 0) {
              unsigned_big->size--;
            } /* if */
            uBigDivideByPowerOf5(unsigned_big);
            if (unsigned_big->bigdigits[unsigned_big->size - 1] == 0) {
              unsigned_big->size--;
            } /* if */
            largeDecimalBlockCount++;
          } /* while */
          while (unsigned_big->size > 1 ||
                 unsigned_big->bigdigits[0] >= POWER_OF_10_IN_BIGDIGIT) {
            (void) uBigDivideByPowerOf10(unsigned_big);
            /* printf("unsigned_big->size=" FMT_U_MEM ", digit=" FMT_U_DIG "\n",
               unsigned_big->size, digit); */
            if (unsigned_big->bigdigits[unsigned_big->size - 1] == 0) {
              unsigned_big->size--;
            } /* if */
            decimalBlockCount++;
          } /* while */
          digit = unsigned_big->bigdigits[0];
          FREE_BIG(unsigned_big);
#if POINTER_SIZE == 32
          logarithm = bigFromUInt32(decimalBlockCount);
#elif POINTER_SIZE == 64
          logarithm = bigFromUInt64(decimalBlockCount);
#endif
          if (logarithm != NULL) {
            bigMultAssign1(&logarithm, DECIMAL_DIGITS_IN_BIGDIGIT);
#if BIGDIGIT_SIZE < 32
            {
              bigIntType numDigits = bigFromUInt32(
                  largeDecimalBlockCount * QUINARY_DIGITS_IN_BIGDIGIT);
              bigAddAssign(&logarithm, numDigits);
              bigDestr(numDigits);
            }
#else
            bigAddAssignSignedDigit(&logarithm,
                (intType) (largeDecimalBlockCount * QUINARY_DIGITS_IN_BIGDIGIT));
#endif
            /* printf("digit: " FMT_U_DIG "\n", digit); */
            digit /= 10;
            while (digit != 0) {
              bigIncr(&logarithm);
              digit /= 10;
            } /* while */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigLog10 --> %s\n", bigHexCStri(logarithm)););
    return logarithm;
  } /* bigLog10 */



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
    logFunction(printf("bigLog2(%s)\n", bigHexCStri(big1)););
    if (unlikely(IS_NEGATIVE(big1->bigdigits[big1->size - 1]))) {
      logError(printf("bigLog2(%s): Number is negative.\n",
                      bigHexCStri(big1)););
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
    logFunction(printf("bigLog2 --> %s\n", bigHexCStri(logarithm)););
    return logarithm;
  } /* bigLog2 */



/**
 *  Create a number from the lower bits of big1.
 *  This corresponds to the modulo if the dividend is a power of two:
 *   bigLowerBits(big1, bits)  corresponds to  big1 mod (2_ ** bits)
 *  @param bits Number of lower bits to select from big1.
 *  @return a number in the range 0 .. pred(2_ ** bits).
 *  @exception NUMERIC_ERROR The number of bits is negative.
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
    logFunction(printf("bigLowerBits(%s, " FMT_D ")\n",
                       bigHexCStri(big1), bits););
    if (unlikely(bits <= 0)) {
      if (unlikely(bits != 0)) {
        logError(printf("bigLowerBits(%s, " FMT_D "): "
                        "Number of bits is negative.\n",
                        bigHexCStri(big1), bits););
        raise_error(NUMERIC_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 1;
          result->bigdigits[0] = (bigDigitType) 0;
        } /* if */
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
          /* printf("mask = " F_X_DIG(08) "\n", digit_mask); */
          result->bigdigits[idx] = big1->bigdigits[idx] & digit_mask;
        } /* if */
        memcpy(result->bigdigits, big1->bigdigits,
               (size_t) idx * sizeof(bigDigitType));
      } /* if */
    } /* if */
    result = normalize(result);
    logFunction(printf("bigLowerBits --> %s (size=" FMT_U_MEM ")\n",
                       bigHexCStri(result), result->size););
    return result;
  } /* bigLowerBits */



/**
 *  Create a number from the lower bits of big1.
 *  Big1 is assumed to be a temporary value which is reused.
 *  This corresponds to the modulo if the dividend is a power of two:
 *   bigLowerBits(big1, bits)  corresponds to  big1 mod (2_ ** bits)
 *  @param bits Number of lower bits to select from big1.
 *  @return a number in the range 0 .. pred(2_ ** bits).
 *  @exception NUMERIC_ERROR The number of bits is negative.
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
    logFunction(printf("bigLowerBitsTemp(%s, " FMT_D ")\n",
                       bigHexCStri(big1), bits););
    big1_size = big1->size;
    if (unlikely(bits <= 0)) {
      FREE_BIG(big1);
      if (unlikely(bits != 0)) {
        logError(printf("bigLowerBitsTemp(%s, " FMT_D "): "
                        "Number of bits is negative.\n",
                        bigHexCStri(big1), bits););
        raise_error(NUMERIC_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 1;
          result->bigdigits[0] = (bigDigitType) 0;
        } /* if */
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
        REALLOC_BIG_CHECK_SIZE(result, big1, result_size);
      } else {
        result = big1;
      } /* if */
      if (unlikely(result == NULL)) {
        FREE_BIG(big1);
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
          /* printf("mask = " F_X_DIG(08) "\n", digit_mask); */
          result->bigdigits[idx] = result->bigdigits[idx] & digit_mask;
        } /* if */
      } /* if */
    } /* if */
    result = normalize(result);
    logFunction(printf("bigLowerBitsTemp --> %s (size=" FMT_U_MEM ")\n",
                       bigHexCStri(result), result->size););
    return result;
  } /* bigLowerBitsTemp */



uint64Type bigLowerBits64 (const const_bigIntType big1)

  {
    memSizeType pos;
    uint64Type result;

  /* bigLowerBits64 */
    logFunction(printf("bigLowerBits64(%s)\n", bigHexCStri(big1)););
    pos = big1->size - 1;
    if (pos >= sizeof(uint64Type) / (BIGDIGIT_SIZE >> 3)) {
      pos = sizeof(uint64Type) / (BIGDIGIT_SIZE >> 3) - 1;
    } /* if */
    result = (uint64Type) big1->bigdigits[pos];
#if BIGDIGIT_SIZE < 64
    while (pos > 0) {
      pos--;
      result <<= BIGDIGIT_SIZE;
      result |= (uint64Type) big1->bigdigits[pos];
    } /* while */
#endif
    logFunction(printf("bigLowerBits64(%s) --> " FMT_U64 "\n",
                       bigHexCStri(big1), result););
    return result;
  } /* bigLowerBits64 */



/**
 *  Number of lowest-order zero bits in the two's-complement representation.
 *  This is equal to the index of the lowest-order one bit (indices start with 0).
 *  If there are only zero bits ('number' is 0_) the result is -1.
 *   bigLowestSetBit(0_)   returns -1
 *   bigLowestSetBit(1_)   returns  0
 *   bigLowestSetBit(4_)   returns  2
 *   bigLowestSetBit(-1_)  returns  0
 *   bigLowestSetBit(-2_)  returns  1
 *   bigLowestSetBit(-4_)  returns  2
 *  @return the number of lowest-order zero bits or -1 for bigLowestSetBit(0_).
 *  @exception RANGE_ERROR The result does not fit into an integer.
 */
intType bigLowestSetBit (const const_bigIntType big1)

  {
    memSizeType big1_size;
    memSizeType pos = 0;
    intType result;

  /* bigLowestSetBit */
    logFunction(printf("bigLowestSetBit(%s)\n", bigHexCStri(big1)););
    big1_size = big1->size;
    while (pos < big1_size && big1->bigdigits[pos] == 0) {
      pos++;
    } /* while */
    if (pos < big1_size) {
      result = digitLeastSignificantBit(big1->bigdigits[pos]);
      if (unlikely(pos > (memSizeType) (MAX_MEM_INDEX - result) >> BIGDIGIT_LOG2_SIZE)) {
        logError(printf("bigLowestSetBit(%s): "
                        "Result does not fit into an integer.\n",
                        bigHexCStri(big1)););
        raise_error(RANGE_ERROR);
        result = 0;
      } else {
        result += (intType) (pos << BIGDIGIT_LOG2_SIZE);
      } /* if */
    } else {
      result = -1;
    } /* if */
    logFunction(printf("bigLowestSetBit --> " FMT_D "\n", result););
    return result;
  } /* bigLowestSetBit */



/**
 *  Shift a 'bigInteger' number left by lshift bits.
 *  If lshift is negative a right shift is done instead.
 *  A << B is equivalent to A * 2_ ** B if B >= 0 holds.
 *  A << B is equivalent to A mdiv 2_ ** -B if B < 0 holds.
 *  @return the left shifted number.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
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
    logFunction(printf("bigLShift(%s, " FMT_D ")\n",
                       bigHexCStri(big1), lshift););
    if (unlikely(lshift < 0)) {
      if (unlikely(TWOS_COMPLEMENT_INTTYPE && lshift == INTTYPE_MIN)) {
        result = bigRShift(big1, INTTYPE_MAX);
        if (result != NULL) {
          bigRShiftAssign(&result, 1);
        } /* if */
      } else {
        result = bigRShift(big1, -lshift);
      } /* if */
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
    logFunction(printf("bigLShift --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigLShift */



/**
 *  Shift a number left by lshift bits and assign the result back to number.
 *  If lshift is negative a right shift is done instead.
 *  @exception MEMORY_ERROR Not enough memory to represent the new value.
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
    logFunction(printf("bigLShiftAssign(%s, " FMT_D ")\n",
                       bigHexCStri(*big_variable), lshift););
    if (unlikely(lshift < 0)) {
      if (unlikely(TWOS_COMPLEMENT_INTTYPE && lshift == INTTYPE_MIN)) {
        bigRShiftAssign(big_variable, INTTYPE_MAX);
        bigRShiftAssign(big_variable, 1);
      } else {
        bigRShiftAssign(big_variable, -lshift);
      } /* if */
    } else if (likely(lshift != 0)) {
      big1 = *big_variable;
      if (big1->size == 1 && big1->bigdigits[0] == 0) {
        if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 1;
          result->bigdigits[0] = 0;
          *big_variable = result;
          FREE_BIG(big1);
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
            FREE_BIG(big1);
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
            FREE_BIG(big1);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigLShiftAssign --> %s\n", bigHexCStri(*big_variable)););
  } /* bigLShiftAssign */



/**
 *  Shift one left by 'lshift' bits.
 *  If 'lshift' is positive or zero this corresponds to
 *  the computation of a power of two:
 *   bigLShiftOne(lshift)  corresponds to  2_ ** lshift
 *  If 'lshift' is negative the result is zero.
 *  @return one shifted left by 'lshift'.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigLShiftOne (const intType lshift)

  {
    memSizeType result_size;
    int bit_pos;
    bigIntType result;

  /* bigLShiftOne */
    logFunction(printf("bigLShiftOne(" FMT_D ")\n", lshift););
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
    logFunction(printf("bigLShiftOne --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigLShiftOne */



/**
 *  Exponentiation if the base is a power of two.
 *  @param log2base Logarithm of the actual base ( =log2(base) )
 *  @return (2 ** log2base) ** exponent
 *  @exception NUMERIC_ERROR If log2base or exponent is negative.
 */
bigIntType bigLog2BaseIPow (const intType log2base, const intType exponent)

  {
    uintType high_shift;
    uintType low_shift;
    bigIntType power;

  /* bigLog2BaseIPow */
    logFunction(printf("bigLog2BaseIPow(" FMT_D ", " FMT_D ")\n",
                       log2base, exponent););
    if (unlikely(log2base < 0 || exponent < 0)) {
      logError(printf("bigLog2BaseIPow(" FMT_D ", " FMT_D "): "
                      "Log2base or exponent is negative.\n",
                      log2base, exponent););
      raise_error(NUMERIC_ERROR);
      power = NULL;
    } else if (likely(log2base == 1)) {
      power = bigLShiftOne(exponent);
    } else if (log2base <= 10 && exponent <= MAX_DIV_10) {
      power = bigLShiftOne(log2base * exponent);
    } else {
      low_shift = uintMult((uintType) log2base, (uintType) exponent, &high_shift);
      if (unlikely(high_shift != 0 || (intType) low_shift < 0)) {
        raise_error(MEMORY_ERROR);
        power = NULL;
      } else {
        power = bigLShiftOne((intType) low_shift);
      } /* if */
    } /* if */
    logFunction(printf("bigLog2BaseIPow --> %s\n", bigHexCStri(power)););
    return power;
  } /* bigLog2BaseIPow */



/**
 *  Integer division truncated towards negative infinity.
 *  The modulo (remainder) of this division is computed with bigMod.
 *  Therefore this division is called modulo division (MDiv).
 *  The memory for the result is requested and the normalized result
 *  is returned. If divisor has just one digit or if dividend
 *  has less digits than divisor the functions bigMDiv1() or
 *  bigMDivSizeLess() are called. In the general case the absolute
 *  values of dividend and divisor are taken. Then dividend is
 *  extended by one leading zero digit. After that dividend and divisor
 *  are shifted to the left such that the most significant bit
 *  of divisor is set. This fulfills the preconditions for calling
 *  uBigDiv() which does the main work of the division.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
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
    logFunction(printf("bigMDiv(%s,", bigHexCStri(dividend));
                printf("%s)\n", bigHexCStri(divisor)););
    if (divisor->size == 1) {
      quotient = bigMDiv1(dividend, divisor->bigdigits[0]);
    } else if (dividend->size < divisor->size) {
      quotient = bigMDivSizeLess(dividend, divisor);
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
        FREE_BIG2(dividend_help, dividend->size + 2);
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
        FREE_BIG2(dividend_help, dividend->size + 2);
        FREE_BIG2(divisor_help, divisor->size + 1);
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        quotient->size = dividend_help->size - divisor_help->size + 1;
        quotient->bigdigits[quotient->size - 1] = 0;
        shift = (unsigned int)
            (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
        if (shift == 0) {
          /* The most significant digit of divisor_help is 0. Just ignore it */
          dividend_help->size--;
          divisor_help->size--;
          if (divisor_help->size == 1) {
            mdiv1_remainder = uBigDivRem1(dividend_help, divisor_help->bigdigits[0], quotient);
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
      FREE_BIG2(dividend_help, dividend->size + 2);
      FREE_BIG2(divisor_help, divisor->size + 1);
    } /* if */
    logFunction(printf("bigMDiv --> %s\n", bigHexCStri(quotient)););
    return quotient;
  } /* bigMDiv */



/**
 *  Compute the modulo (remainder) of the integer division bigMDiv.
 *  The modulo has the same sign as the divisor. The memory for the result
 *  is requested and the normalized result is returned. If divisor has
 *  just one digit or if dividend has less digits than divisor the
 *  functions bigMod1() or bigModSizeLess() are called. In the general case
 *  the absolute values of dividend and divisor are taken. Then dividend is
 *  extended by one leading zero digit. After that dividend and divisor
 *  are shifted to the left such that the most significant bit
 *  of divisor is set. This fulfills the preconditions for calling
 *  uBigRem() which does the main work of the division. Afterwards
 *  the result must be shifted to the right to get the remainder.
 *  If dividend and divisor have the same sign the modulo has the same
 *  value as the remainder. If the remainder is zero the modulo
 *  is also zero. If the signs of dividend and divisor are different the
 *  modulo is computed from the remainder by adding dividend.
 *  @return the modulo of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
bigIntType bigMod (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    boolType negative1 = FALSE;
    boolType negative2 = FALSE;
    bigIntType divisor_help;
    unsigned int shift;
    bigIntType modulo;

  /* bigMod */
    logFunction(printf("bigMod(%s,", bigHexCStri(dividend));
                printf("%s)\n", bigHexCStri(divisor)););
    if (divisor->size == 1) {
      modulo = bigMod1(dividend, divisor->bigdigits[0]);
    } else if (dividend->size < divisor->size) {
      modulo = bigModSizeLess(dividend, divisor);
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
        FREE_BIG2(modulo,  dividend->size + 2);
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
      shift = (unsigned int)
          (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
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
      FREE_BIG2(divisor_help, divisor->size + 1);
    } /* if */
    logFunction(printf("bigMod --> %s\n", bigHexCStri(modulo)););
    return modulo;
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
    logFunction(printf("bigMult(%s,", bigHexCStri(factor1));
                printf("%s)\n", bigHexCStri(factor2)););
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
          FREE_BIG(factor1_help);
        } /* if */
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
    } /* if */
    /* printf("bigMult(" FMT_U_MEM ", " FMT_U_MEM ")\n",
        factor1->size, factor2->size); */
    product = uBigMultK(factor1, factor2, negative);
    if (factor1_help != NULL) {
      FREE_BIG(factor1_help);
    } /* if */
    if (factor2_help != NULL) {
      FREE_BIG(factor2_help);
    } /* if */
    if (unlikely(product == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("bigMult --> %s\n", bigHexCStri(product)););
    return product;
  } /* bigMult */



/**
 *  Multiply a 'bigInteger' number by a factor and assign the result back to number.
 */
void bigMultAssign (bigIntType *const big_variable, const_bigIntType factor)

  {
    bigIntType product;

  /* bigMultAssign */
    logFunction(printf("bigMultAssign(%s,", bigHexCStri(*big_variable));
                printf("%s)\n", bigHexCStri(factor)););
    if (factor->size == 1) {
      bigMultAssign1(big_variable, factor->bigdigits[0]);
    } else {
      product = bigMult(*big_variable, factor);
      FREE_BIG(*big_variable);
      *big_variable = product;
    } /* if */
    logFunction(printf("bigMultAssign --> %s\n", bigHexCStri(*big_variable)););
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
    logFunction(printf("bigMultSignedDigit(%s, " FMT_D ")\n",
                       bigHexCStri(factor1), factor2););
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
    logFunction(printf("bigMultSignedDigit --> %s\n", bigHexCStri(product)););
    return product;
  } /* bigMultSignedDigit */



/**
 *  Minus sign, negate a 'bigInteger' number.
 *  @return the negated value of the number.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigNegate (const const_bigIntType big1)

  {
    memSizeType pos;
    doubleBigDigitType carry = 1;
    bigIntType resized_negatedValue;
    bigIntType negatedValue;

  /* bigNegate */
    logFunction(printf("bigNegate(%s)\n", bigHexCStri(big1)););
    if (unlikely(!ALLOC_BIG_SIZE_OK(negatedValue, big1->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      negatedValue->size = big1->size;
      pos = 0;
      do {
        carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
        negatedValue->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < big1->size);
      if (IS_NEGATIVE(negatedValue->bigdigits[pos - 1])) {
        if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
          REALLOC_BIG_CHECK_SIZE(resized_negatedValue, negatedValue, pos + 1);
          if (unlikely(resized_negatedValue == NULL)) {
            FREE_BIG2(negatedValue, pos);
            raise_error(MEMORY_ERROR);
            negatedValue = NULL;
          } else {
            negatedValue = resized_negatedValue;
            negatedValue->size++;
            negatedValue->bigdigits[pos] = 0;
          } /* if */
        } else if (negatedValue->bigdigits[pos - 1] == BIGDIGIT_MASK &&
            pos >= 2 && IS_NEGATIVE(negatedValue->bigdigits[pos - 2])) {
          REALLOC_BIG_SIZE_OK(resized_negatedValue, negatedValue, pos - 1);
          /* Avoid a MEMORY_ERROR in the strange case   */
          /* if a 'realloc' which shrinks memory fails. */
          if (likely(resized_negatedValue != NULL)) {
            negatedValue = resized_negatedValue;
          } /* if */
          negatedValue->size--;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigNegate --> %s\n", bigHexCStri(negatedValue)););
    return negatedValue;
  } /* bigNegate */



/**
 *  Minus sign, negate a 'bigInteger' number.
 *  Big1 is assumed to be a temporary value which is reused.
 *  @return the negated value of the number.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigNegateTemp (bigIntType big1)

  {
    memSizeType pos = 0;
    doubleBigDigitType carry = 1;
    boolType negative;
    bigIntType resized_big1;

  /* bigNegateTemp */
    logFunction(printf("bigNegateTemp(%s)\n", bigHexCStri(big1)););
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    do {
      carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
      big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big1->size);
    if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      if (negative) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, pos + 1);
        if (unlikely(resized_big1 == NULL)) {
          FREE_BIG(big1);
          raise_error(MEMORY_ERROR);
          big1 = NULL;
        } else {
          big1 = resized_big1;
          big1->size++;
          big1->bigdigits[pos] = 0;
        } /* if */
      } else if (big1->bigdigits[pos - 1] == BIGDIGIT_MASK &&
          pos >= 2 && IS_NEGATIVE(big1->bigdigits[pos - 2])) {
        REALLOC_BIG_SIZE_OK(resized_big1, big1, pos - 1);
        /* Avoid a MEMORY_ERROR in the strange case   */
        /* if a 'realloc' which shrinks memory fails. */
        if (likely(resized_big1 != NULL)) {
          big1 = resized_big1;
        } /* if */
        big1->size--;
      } /* if */
    } /* if */
    logFunction(printf("bigNegateTemp --> %s\n", bigHexCStri(big1)););
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
    logFunction(printf("bigOr(%s,", bigHexCStri(big1));
                printf("%s)\n", bigHexCStri(big2)););
    if (big2->size > big1->size) {
      help_big = big1;
      big1 = big2;
      big2 = help_big;
    } /* if */
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, big1->size))) {
      raise_error(MEMORY_ERROR);
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
    } /* if */
    logFunction(printf("bigOr --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigOr */



/**
 *  Convert a string to a 'bigInteger' number.
 *  The string must contain an integer literal consisting of an
 *  optional + or - sign, followed by a sequence of digits. Other
 *  characters as well as leading or trailing whitespace characters are
 *  not allowed. The sequence of digits is taken to be decimal.
 *  @return the 'bigInteger' result of the conversion.
 *  @exception RANGE_ERROR If the string is empty or does not contain
 *             an integer literal.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
bigIntType bigParse (const const_striType stri)

  {
    memSizeType result_size;
    boolType okay;
    boolType negative;
    memSizeType position = 0;
    memSizeType limit;
    bigDigitType bigDigit;
    bigIntType result;

  /* bigParse */
    logFunction(printf("bigParse(\"%s\")\n", striAsUnquotedCStri(stri)););
    if (likely(stri->size != 0)) {
      if (stri->mem[0] == ((strElemType) '-')) {
        negative = TRUE;
        position++;
      } else {
        if (stri->mem[0] == ((strElemType) '+')) {
          position++;
        } /* if */
        negative = FALSE;
      } /* if */
    } /* if */
    if (unlikely(position >= stri->size)) {
      logError(printf("bigParse(\"%s\"): "
                      "Digit missing.\n",
                      striAsUnquotedCStri(stri)););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result_size = (stri->size - 1) / DECIMAL_DIGITS_IN_BIGDIGIT + 1;
      if (unlikely(!ALLOC_BIG(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 1;
        result->bigdigits[0] = 0;
        okay = TRUE;
        limit = (stri->size - position - 1) % DECIMAL_DIGITS_IN_BIGDIGIT + position + 1;
        do {
          bigDigit = 0;
          while (position < limit && okay) {
            if (likely(stri->mem[position] >= ((strElemType) '0') &&
                       stri->mem[position] <= ((strElemType) '9'))) {
              bigDigit = (bigDigitType) 10 * bigDigit +
                  (bigDigitType) stri->mem[position] - (bigDigitType) '0';
            } else {
              okay = FALSE;
            } /* if */
            position++;
          } /* while */
          uBigMultByPowerOf10AndAdd(result, (doubleBigDigitType) bigDigit);
          limit += DECIMAL_DIGITS_IN_BIGDIGIT;
        } while (position < stri->size && okay);
        if (likely(okay)) {
          memset(&result->bigdigits[result->size], 0,
                 (size_t) (result_size - result->size) * sizeof(bigDigitType));
          result->size = result_size;
          if (negative) {
            negate_positive_big(result);
          } /* if */
          result = normalize(result);
        } else {
          FREE_BIG2(result, result_size);
          logError(printf("bigParse(\"%s\"): "
                          "Illegal digit.\n",
                          striAsUnquotedCStri(stri)););
          raise_error(RANGE_ERROR);
          result = NULL;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigParse --> %s\n", bigHexCStri(result)););
    return result;
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
 *  @param stri Numeric string with integer in the specified radix.
 *  @param base Radix of the integer in the 'stri' parameter.
 *  @return the 'bigInteger' result of the conversion.
 *  @exception RANGE_ERROR If base < 2 or base > 36 holds or
 *             the string does not contain an integer
 *             literal with the specified base.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
bigIntType bigParseBased (const const_striType stri, intType base)

  {
    bigIntType result;

  /* bigParseBased */
    logFunction(printf("bigParseBased(\"%s\", " FMT_D ")\n",
                       striAsUnquotedCStri(stri), base););
    switch (castIntTypeForSwitch(base)) {
      /* Cases sorted by probability. */
      case 16: result = bigParseBasedPow2(stri, 4); break;
      case  8: result = bigParseBasedPow2(stri, 3); break;
      case 10: result = bigParse(stri);             break;
      case  2: result = bigParseBasedPow2(stri, 1); break;
      case  4: result = bigParseBasedPow2(stri, 2); break;
      case 32: result = bigParseBasedPow2(stri, 5); break;
      default:
        if (unlikely(base < 2 || base > 36)) {
          logError(printf("bigParseBased(\"%s\", " FMT_D "): "
                          "Base not in allowed range.\n",
                          striAsUnquotedCStri(stri), base););
          raise_error(RANGE_ERROR);
          result = NULL;
        } else {
          result = bigParseBased2To36(stri, (unsigned int) base);
        } /* if */
        break;
    } /* switch */
    logFunction(printf("bigParseBased --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigParseBased */



/**
 *  Predecessor of a 'bigInteger' number.
 *  pred(A) is equivalent to A-1 .
 *  @return big1 - 1 .
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigPred (const const_bigIntType big1)

  {
    memSizeType pos;
    bigIntType resized_predecessor;
    bigIntType predecessor;

  /* bigPred */
    logFunction(printf("bigPred(%s)\n", bigHexCStri(big1)););
    if (unlikely(!ALLOC_BIG_SIZE_OK(predecessor, big1->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      predecessor->size = big1->size;
      pos = 0;
      if (big1->bigdigits[pos] == 0) {
        if (big1->size == 1) {
          predecessor->bigdigits[pos] = BIGDIGIT_MASK;
          pos++;
        } else {
          do {
            predecessor->bigdigits[pos] = BIGDIGIT_MASK;
            pos++;
          } while (big1->bigdigits[pos] == 0);
        } /* if */
      } /* if */
      if (pos < big1->size) {
        predecessor->bigdigits[pos] = big1->bigdigits[pos] - 1;
        pos++;
        memcpy(&predecessor->bigdigits[pos], &big1->bigdigits[pos],
               (big1->size - pos) * sizeof(bigDigitType));
        pos = big1->size;
        /* while (pos < big1->size) {
          predecessor->bigdigits[pos] = big1->bigdigits[pos];
          pos++;
        } ** while */
      } /* if */
      if (!IS_NEGATIVE(predecessor->bigdigits[pos - 1])) {
        if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
          REALLOC_BIG_CHECK_SIZE(resized_predecessor, predecessor, pos + 1);
          if (unlikely(resized_predecessor == NULL)) {
            FREE_BIG2(predecessor, pos);
            raise_error(MEMORY_ERROR);
            predecessor = NULL;
          } else {
            predecessor = resized_predecessor;
            predecessor->size++;
            predecessor->bigdigits[pos] = BIGDIGIT_MASK;
          } /* if */
        } else if (predecessor->bigdigits[pos - 1] == 0 &&
            pos >= 2 && !IS_NEGATIVE(predecessor->bigdigits[pos - 2])) {
          REALLOC_BIG_SIZE_OK(resized_predecessor, predecessor, pos - 1);
          /* Avoid a MEMORY_ERROR in the strange case   */
          /* if a 'realloc' which shrinks memory fails. */
          if (likely(resized_predecessor != NULL)) {
            predecessor = resized_predecessor;
          } /* if */
          predecessor->size--;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigPred --> %s\n", bigHexCStri(predecessor)););
    return predecessor;
  } /* bigPred */



/**
 *  Returns a signed big integer decremented by 1.
 *  Big1 is assumed to be a temporary value which is reused.
 */
bigIntType bigPredTemp (bigIntType big1)

  {
    memSizeType pos = 0;
    boolType negative;
    bigIntType resized_big1;

  /* bigPredTemp */
    logFunction(printf("bigPredTemp(%s)\n", bigHexCStri(big1)););
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    if (big1->bigdigits[pos] == 0) {
      if (big1->size == 1) {
        big1->bigdigits[pos] = BIGDIGIT_MASK;
        pos++;
      } else {
        do {
          big1->bigdigits[pos] = BIGDIGIT_MASK;
          pos++;
        } while (big1->bigdigits[pos] == 0);
      } /* if */
    } /* if */
    if (pos < big1->size) {
      big1->bigdigits[pos]--;
    } /* if */
    pos = big1->size;
    if (!IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      if (negative) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, pos + 1);
        if (unlikely(resized_big1 == NULL)) {
          FREE_BIG(big1);
          raise_error(MEMORY_ERROR);
          big1 = NULL;
        } else {
          big1 = resized_big1;
          big1->size++;
          big1->bigdigits[pos] = BIGDIGIT_MASK;
        } /* if */
      } else if (big1->bigdigits[pos - 1] == 0 &&
          pos >= 2 && !IS_NEGATIVE(big1->bigdigits[pos - 2])) {
        REALLOC_BIG_SIZE_OK(resized_big1, big1, pos - 1);
        /* Avoid a MEMORY_ERROR in the strange case   */
        /* if a 'realloc' which shrinks memory fails. */
        if (likely(resized_big1 != NULL)) {
          big1 = resized_big1;
        } /* if */
        big1->size--;
      } /* if */
    } /* if */
    logFunction(printf("bigPredTemp --> %s\n", bigHexCStri(big1)););
    return big1;
  } /* bigPredTemp */



/**
 *  Convert a big integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with letters.
 *  For negative numbers a minus sign is prepended.
 *  @param big1 BigInteger number to be converted.
 *  @param upperCase Decides about the letter case.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR If base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType bigRadix (const const_bigIntType big1, intType base,
    boolType upperCase)

  {
    striType result;

  /* bigRadix */
    logFunction(printf("bigRadix(%s, " FMT_D ", %d)\n",
                       bigHexCStri(big1), base, upperCase););
    switch (castIntTypeForSwitch(base)) {
      /* Cases sorted by probability. */
      case 16: result = bigRadixPow2(big1, 4,  0xf, upperCase); break;
      case  8: result = bigRadixPow2(big1, 3,  0x7, upperCase); break;
      case 10: result = bigStr(big1);                           break;
      case  2: result = bigRadixPow2(big1, 1,  0x1, upperCase); break;
      case  4: result = bigRadixPow2(big1, 2,  0x3, upperCase); break;
      case 32: result = bigRadixPow2(big1, 5, 0x1f, upperCase); break;
      default:
        if (unlikely(base < 2 || base > 36)) {
          logError(printf("bigRadix((%s, " FMT_D ", %d): "
                          "Base not in allowed range.\n",
                          bigHexCStri(big1), base, upperCase););
          raise_error(RANGE_ERROR);
          result = NULL;
        } else {
          result = bigRadix2To36(big1, (unsigned int) base, upperCase);
        } /* if */
        break;
    } /* switch */
    logFunction(printf("bigRadix --> \"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* bigRadix */



/**
 *  Compute pseudo-random number in the range [low, high].
 *  The random values are uniform distributed. The memory for
 *  the result is requested and the normalized result is returned.
 *  @return a random number such that low <= rand(low, high) and
 *          rand(low, high) <= high holds.
 *  @exception RANGE_ERROR The range is empty (low > high holds).
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigRand (const const_bigIntType low,
    const const_bigIntType high)

  {
    bigIntType scale_limit;
    int usedBits;
    bigDigitType mask;
    memSizeType pos;
    doubleBigDigitType random_number = 0;
    memSizeType randomNumber_size;
    bigIntType randomNumber;

  /* bigRand */
    logFunction(printf("bigRand(%s, %s)\n", bigHexCStri(low), bigHexCStri(high)););
    if (unlikely(bigCmp(low, high) > 0)) {
      logError(printf("bigRand(%s, %s): "
                      "The range is empty (low > high holds).\n",
                      bigHexCStri(low), bigHexCStri(high)););
      raise_error(RANGE_ERROR);
      randomNumber = NULL;
    } else {
      scale_limit = bigSbtr(high, low);
      if (low->size > scale_limit->size) {
        randomNumber_size = low->size + 1;
      } else {
        randomNumber_size = scale_limit->size + 1;
      } /* if */
      if (unlikely(!ALLOC_BIG(randomNumber, randomNumber_size))) {
        raise_error(MEMORY_ERROR);
        randomNumber = NULL;
      } else {
        memset(&randomNumber->bigdigits[scale_limit->size], 0,
               (size_t) (randomNumber_size - scale_limit->size) * sizeof(bigDigitType));
        randomNumber->size = scale_limit->size;
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
              random_number = uintRand();
            } /* if */
            randomNumber->bigdigits[pos] = (bigDigitType) (random_number & BIGDIGIT_MASK);
            random_number >>= BIGDIGIT_SIZE;
            pos++;
          } while (pos < scale_limit->size);
          randomNumber->bigdigits[pos - 1] &= mask;
        } while (bigCmp(randomNumber, scale_limit) > 0);
        randomNumber->size = randomNumber_size;
        bigAddTo(randomNumber, low);
        randomNumber = normalize(randomNumber);
        FREE_BIG(scale_limit);
      } /* if */
    } /* if */
    logFunction(printf("bigRand --> %s\n", bigHexCStri(randomNumber)););
    return randomNumber;
  } /* bigRand */



/**
 *  Compute the remainder of the integer division bigDiv.
 *  The remainder has the same sign as the dividend. The memory for the result
 *  is requested and the normalized result is returned. If divisor has
 *  just one digit or if dividend has less digits than divisor the
 *  functions bigRem1() or bigRemSizeLess() are called. In the general case
 *  the absolute values of dividend and divisor are taken. Then dividend is
 *  extended by one leading zero digit. After that dividend and divisor
 *  are shifted to the left such that the most significant bit
 *  of divisor is set. This fulfills the preconditions for calling
 *  uBigRem() which does the main work of the division. Afterwards
 *  the result must be shifted to the right to get the remainder.
 *  @return the remainder of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
bigIntType bigRem (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    boolType negative = FALSE;
    bigIntType divisor_help;
    unsigned int shift;
    bigIntType remainder;

  /* bigRem */
    logFunction(printf("bigRem(%s, ", bigHexCStri(dividend));
                printf("%s)\n", bigHexCStri(divisor)););
    if (divisor->size == 1) {
      remainder = bigRem1(dividend, divisor->bigdigits[0]);
    } else if (dividend->size < divisor->size) {
      remainder = bigRemSizeLess(dividend, divisor);
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
        FREE_BIG2(remainder, dividend->size + 2);
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
      shift = (unsigned int)
          (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
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
      FREE_BIG2(divisor_help, divisor->size + 1);
    } /* if */
    logFunction(printf("bigRem --> %s\n", bigHexCStri(remainder)););
    return remainder;
  } /* bigRem */



/**
 *  Shift a 'bigInteger' number right by rshift bits.
 *  If rshift is negative a left shift is done instead.
 *  A >> B is equivalent to A mdiv 2_ ** B if B >= 0 holds.
 *  A >> B is equivalent to A * 2_ ** -B if B < 0 holds.
 *  @return the right shifted number.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
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
    logFunction(printf("bigRShift(%s, " FMT_D ")\n",
                       bigHexCStri(big1), rshift););
    if (unlikely(rshift < 0)) {
      if (unlikely(TWOS_COMPLEMENT_INTTYPE && rshift == INTTYPE_MIN)) {
        result = bigLShift(big1, INTTYPE_MAX);
        if (result != NULL) {
          bigLShiftAssign(&result, 1);
        } /* if */
      } else {
        result = bigLShift(big1, -rshift);
      } /* if */
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
    logFunction(printf("bigRShift --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigRShift */



/**
 *  Shift a number right by rshift bits and assign the result back to number.
 *  If rshift is negative a left shift is done instead.
 *  @exception MEMORY_ERROR Not enough memory to represent the new value.
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
    logFunction(printf("bigRShiftAssign(%s, " FMT_D ")\n",
                       bigHexCStri(*big_variable), rshift););
    if (unlikely(rshift < 0)) {
      if (unlikely(TWOS_COMPLEMENT_INTTYPE && rshift == INTTYPE_MIN)) {
        bigLShiftAssign(big_variable, INTTYPE_MAX);
        bigLShiftAssign(big_variable, 1);
      } else {
        bigLShiftAssign(big_variable, -rshift);
      } /* if */
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
          FREE_BIG(big1);
        } /* if */
      } else {
        size_reduction = (memSizeType) ((uintType) rshift >> BIGDIGIT_LOG2_SIZE);
        if ((rshift & BIGDIGIT_SIZE_MASK) == 0) {
          if (rshift != 0) {
            big1_size = big1->size;
            memmove(big1->bigdigits, &big1->bigdigits[size_reduction],
                    (size_t) (big1_size - size_reduction) * sizeof(bigDigitType));
            REALLOC_BIG_SIZE_OK(resized_big1, big1, big1_size - size_reduction);
            /* Avoid a MEMORY_ERROR in the strange case   */
            /* if a 'realloc' which shrinks memory fails. */
            if (likely(resized_big1 != NULL)) {
              big1 = resized_big1;
              *big_variable = big1;
            } /* if */
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
                REALLOC_BIG_SIZE_OK(resized_big1, big1, pos);
                /* Avoid a MEMORY_ERROR in the strange case   */
                /* if a 'realloc' which shrinks memory fails. */
                if (likely(resized_big1 != NULL)) {
                  big1 = resized_big1;
                  *big_variable = big1;
                } /* if */
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
                REALLOC_BIG_SIZE_OK(resized_big1, big1, pos);
                /* Avoid a MEMORY_ERROR in the strange case   */
                /* if a 'realloc' which shrinks memory fails. */
                if (likely(resized_big1 != NULL)) {
                  big1 = resized_big1;
                  *big_variable = big1;
                } /* if */
                big1->size = pos;
                size_reduction = 0;
              } /* if */
            } /* if */
          } /* if */
          if (size_reduction != 0) {
            big1_size = big1->size;
            REALLOC_BIG_SIZE_OK(resized_big1, big1, big1_size - size_reduction);
            /* Avoid a MEMORY_ERROR in the strange case   */
            /* if a 'realloc' which shrinks memory fails. */
            if (likely(resized_big1 != NULL)) {
              big1 = resized_big1;
              *big_variable = big1;
            } /* if */
            big1->size -= size_reduction;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigRShiftAssign --> %s\n", bigHexCStri(*big_variable)););
  } /* bigRShiftAssign */



/**
 *  Compute the subtraction of two 'bigInteger' numbers.
 *  @return the difference of the two numbers.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigSbtr (const const_bigIntType minuend, const const_bigIntType subtrahend)

  {
    memSizeType pos;
    doubleBigDigitType carry = 1;
    doubleBigDigitType minuend_sign;
    doubleBigDigitType subtrahend_sign;
    bigIntType difference;

  /* bigSbtr */
    logFunction(printf("bigSbtr(%s,", bigHexCStri(minuend));
                printf("%s)\n", bigHexCStri(subtrahend)););
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
      } /* if */
    } /* if */
    logFunction(printf("bigSbtr --> %s\n", bigHexCStri(difference)););
    return difference;
  } /* bigSbtr */



/**
 *  Decrement a 'bigInteger' variable by a delta.
 *  Subtracts delta from *big_variable. The operation is done in
 *  place and *big_variable is only resized if necessary.
 *  If the size of delta is smaller than *big_variable the
 *  algorithm tries to save computations. Therefore there are
 *  checks for carry != 0 and carry == 0.
 *  In case the resizing fails the content of *big_variable
 *  is freed and *big_variable is set to NULL.
 *  @param delta The delta to be subtracted from *big_variable.
 *  @exception MEMORY_ERROR If the resizing of *big_variable fails.
 */
void bigSbtrAssign (bigIntType *const big_variable, const const_bigIntType delta)

  {
    bigIntType big1;
    memSizeType pos;
    memSizeType big1_size;
    boolType delta_negative;
    doubleBigDigitType carry = 1;
    doubleBigDigitType big1_sign;
    doubleBigDigitType delta_sign;
    bigIntType resized_big1;

  /* bigSbtrAssign */
    logFunction(printf("bigSbtrAssign(%s,", bigHexCStri(*big_variable));
                printf("%s)\n", bigHexCStri(delta)););
    big1 = *big_variable;
    if (big1->size >= delta->size) {
      big1_size = big1->size;
      big1_sign = IS_NEGATIVE(big1->bigdigits[big1_size - 1]) ? BIGDIGIT_MASK : 0;
      /* It is possible that big1 == delta holds. Therefore the check */
      /* for negative delta must be done before big1 is changed.      */
      delta_negative = IS_NEGATIVE(delta->bigdigits[delta->size - 1]);
      pos = 0;
      do {
        carry += (doubleBigDigitType) big1->bigdigits[pos] +
            (~delta->bigdigits[pos] & BIGDIGIT_MASK);
        big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < delta->size);
      if (delta_negative) {
        for (; carry != 0 && pos < big1_size; pos++) {
          carry += big1->bigdigits[pos];
          big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
      } else {
        for (; carry == 0 && pos < big1_size; pos++) {
          carry = (doubleBigDigitType) big1->bigdigits[pos] + BIGDIGIT_MASK;
          big1->bigdigits[pos] = (bigDigitType) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        carry += BIGDIGIT_MASK;
      } /* if */
      carry += big1_sign;
      carry &= BIGDIGIT_MASK;
      /* Now the only possible values for carry are 0 and BIGDIGIT_MASK. */
      if ((carry != 0 || IS_NEGATIVE(big1->bigdigits[big1_size - 1])) &&
          (carry != BIGDIGIT_MASK || !IS_NEGATIVE(big1->bigdigits[big1_size - 1]))) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, big1_size + 1);
        if (unlikely(resized_big1 == NULL)) {
          FREE_BIG(big1);
          *big_variable = NULL;
          raise_error(MEMORY_ERROR);
        } else {
          /* It is possible that big1 == delta holds. Since */
          /* 'delta' is not used after realloc() enlarged   */
          /* 'big1' a correction of delta is not necessary. */
          big1 = resized_big1;
          big1->size++;
          big1->bigdigits[big1_size] = (bigDigitType) (carry);
          *big_variable = big1;
        } /* if */
      } else {
        *big_variable = normalize(big1);
      } /* if */
    } else {
      REALLOC_BIG_CHECK_SIZE(resized_big1, big1, delta->size + 1);
      if (unlikely(resized_big1 == NULL)) {
        FREE_BIG(big1);
        *big_variable = NULL;
        raise_error(MEMORY_ERROR);
      } else {
        big1 = resized_big1;
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
    logFunction(printf("bigSbtrAssign --> %s\n", bigHexCStri(*big_variable)););
  } /* bigSbtrAssign */



/**
 *  Compute the subtraction of two 'bigInteger' numbers.
 *  Minuend is assumed to be a temporary value which is reused.
 *  @return the difference of the two numbers in 'minuend'.
 */
bigIntType bigSbtrTemp (bigIntType minuend, const_bigIntType subtrahend)

  { /* bigSbtrTemp */
    bigSbtrAssign(&minuend, subtrahend);
    return minuend;
  } /* bigSbtrTemp */



/**
 *  Compute the square of a 'bigInteger'.
 *  This function is used by the compiler to optimize
 *  multiplication and exponentiation operations.
 *  @return the square of big1.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigSquare (const_bigIntType big1)

  {
    bigIntType big1_help = NULL;
    bigIntType square;

  /* bigSquare */
    logFunction(printf("bigSquare(%s)\n", bigHexCStri(big1)););
    if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
      big1_help = alloc_positive_copy_of_negative_big(big1);
      big1 = big1_help;
      if (unlikely(big1_help == NULL)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
    } /* if */
    /* printf("bigSquare(" FMT_U_MEM ")\n", big1->size); */
    square = uBigSquareK(big1);
    if (big1_help != NULL) {
      FREE_BIG(big1_help);
    } /* if */
    if (unlikely(square == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("bigSquare --> %s\n", bigHexCStri(square)););
    return square;
  } /* bigSquare */



/**
 *  Convert a 'bigInteger' number to a string.
 *  The number is converted to a string with decimal representation.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion, or NULL
 *          if there is not enough memory to represent the result.
 */
striType bigStrDecimal (const const_bigIntType big1)

  {
    bigIntType unsigned_big;
    memSizeType pos;
    memSizeType result_size;
    memSizeType final_result_size;
    striType resized_result;
    striType result;

  /* bigStrDecimal */
    logFunction(printf("bigStrDecimal(%s)\n", bigHexCStri(big1)););
    if (unlikely((MAX_STRI_LEN <= (MAX_MEMSIZETYPE - 1) / OCTAL_DIGIT_BITS + 2 &&
                 big1->size > ((MAX_STRI_LEN - 2) * OCTAL_DIGIT_BITS + 1) / BIGDIGIT_SIZE) ||
                 big1->size > MAX_MEMSIZETYPE / BIGDIGIT_SIZE)) {
      result = NULL;
    } else {
      /* The size of the result is estimated by computing the    */
      /* number of octal digits plus one character for the sign. */
      result_size = (big1->size * BIGDIGIT_SIZE - 1) / OCTAL_DIGIT_BITS + 2;
      if (likely(ALLOC_STRI_SIZE_OK(result, result_size))) {
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          unsigned_big = alloc_positive_copy_of_negative_big(big1);
        } else if (likely(ALLOC_BIG_SIZE_OK(unsigned_big, big1->size))) {
          unsigned_big->size = big1->size;
          memcpy(unsigned_big->bigdigits, big1->bigdigits,
                 (size_t) big1->size * sizeof(bigDigitType));
        } /* if */
        if (unlikely(unsigned_big == NULL)) {
          FREE_STRI2(result, result_size);
          result = NULL;
        } else {
          /* pos = basicToStri(unsigned_big, result, result_size - 1); */
          pos = binaryToStri(unsigned_big, result, (unsigned int)
                             memSizeMostSignificantBit(result_size) + 1,
                             FALSE, result_size - 1);
          FREE_BIG2(unsigned_big, big1->size);
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
              FREE_STRI2(result, result_size);
              result = NULL;
            } else {
              result = resized_result;
              COUNT3_STRI(result_size, final_result_size);
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigStrDecimal --> \"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* bigStrDecimal */



striType bigStr (const const_bigIntType big1)

  {
    striType result;

  /* bigStr */
    logFunction(printf("bigStr(%s)\n", bigHexCStri(big1)););
    result = bigStrDecimal(big1);
    if (unlikely(result == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("bigStr --> \"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* bigStr */



/**
 *  Successor of a 'bigInteger' number.
 *  succ(A) is equivalent to A+1 .
 *  @return big1 + 1 .
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bigIntType bigSucc (const const_bigIntType big1)

  {
    memSizeType pos;
    bigIntType resized_successor;
    bigIntType successor;

  /* bigSucc */
    logFunction(printf("bigSucc(%s)\n", bigHexCStri(big1)););
    if (unlikely(!ALLOC_BIG_SIZE_OK(successor, big1->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      successor->size = big1->size;
      pos = 0;
      if (big1->bigdigits[pos] == BIGDIGIT_MASK) {
        if (big1->size == 1) {
          successor->bigdigits[pos] = 0;
          pos++;
        } else {
          do {
            successor->bigdigits[pos] = 0;
            pos++;
          } while (big1->bigdigits[pos] == BIGDIGIT_MASK);
        } /* if */
      } /* if */
      if (pos < big1->size) {
        successor->bigdigits[pos] = big1->bigdigits[pos] + 1;
        pos++;
        memcpy(&successor->bigdigits[pos], &big1->bigdigits[pos],
               (big1->size - pos) * sizeof(bigDigitType));
        pos = big1->size;
        /* while (pos < big1->size) {
          successor->bigdigits[pos] = big1->bigdigits[pos];
          pos++;
        } ** while */
      } /* if */
      if (IS_NEGATIVE(successor->bigdigits[pos - 1])) {
        if (!IS_NEGATIVE(big1->bigdigits[pos - 1])) {
          REALLOC_BIG_CHECK_SIZE(resized_successor, successor, pos + 1);
          if (unlikely(resized_successor == NULL)) {
            FREE_BIG2(successor, pos);
            raise_error(MEMORY_ERROR);
            successor = NULL;
          } else {
            successor = resized_successor;
            successor->size++;
            successor->bigdigits[pos] = 0;
          } /* if */
        } else if (successor->bigdigits[pos - 1] == BIGDIGIT_MASK &&
            pos >= 2 && IS_NEGATIVE(successor->bigdigits[pos - 2])) {
          /* Avoid a MEMORY_ERROR in the strange case   */
          /* if a 'realloc' which shrinks memory fails. */
          REALLOC_BIG_SIZE_OK(resized_successor, successor, pos - 1);
          if (likely(resized_successor != NULL)) {
            successor = resized_successor;
          } /* if */
          successor->size--;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("bigSucc --> %s\n", bigHexCStri(successor)););
    return successor;
  } /* bigSucc */



/**
 *  Successor of a 'bigInteger' number.
 *  Big1 is assumed to be a temporary value which is reused.
 *  @return big1 + 1 .
 */
bigIntType bigSuccTemp (bigIntType big1)

  {
    memSizeType pos = 0;
    boolType negative;
    bigIntType resized_big1;

  /* bigSuccTemp */
    logFunction(printf("bigSuccTemp(%s)\n", bigHexCStri(big1)););
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    if (big1->bigdigits[pos] == BIGDIGIT_MASK) {
      if (big1->size == 1) {
        big1->bigdigits[pos] = 0;
        pos++;
      } else {
        do {
          big1->bigdigits[pos] = 0;
          pos++;
        } while (big1->bigdigits[pos] == BIGDIGIT_MASK);
      } /* if */
    } /* if */
    if (pos < big1->size) {
      big1->bigdigits[pos]++;
    } /* if */
    pos = big1->size;
    if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      if (!negative) {
        REALLOC_BIG_CHECK_SIZE(resized_big1, big1, pos + 1);
        if (unlikely(resized_big1 == NULL)) {
          FREE_BIG(big1);
          raise_error(MEMORY_ERROR);
          big1 = NULL;
        } else {
          big1 = resized_big1;
          big1->size++;
          big1->bigdigits[pos] = 0;
        } /* if */
      } else if (big1->bigdigits[pos - 1] == BIGDIGIT_MASK &&
          pos >= 2 && IS_NEGATIVE(big1->bigdigits[pos - 2])) {
        REALLOC_BIG_SIZE_OK(resized_big1, big1, pos - 1);
        /* Avoid a MEMORY_ERROR in the strange case   */
        /* if a 'realloc' which shrinks memory fails. */
        if (likely(resized_big1 != NULL)) {
          big1 = resized_big1;
        } /* if */
        big1->size--;
      } /* if */
    } /* if */
    logFunction(printf("bigSuccTemp --> %s\n", bigHexCStri(big1)););
    return big1;
  } /* bigSuccTemp */



/**
 *  Convert a 'bigInteger' into a big-endian 'bstring'.
 *  The result uses binary representation with a base of 256.
 *  @param big1 BigInteger number to be converted.
 *  @param isSigned Determines the signedness of the result.
 *         If 'isSigned' is TRUE the result is encoded with the
 *         twos-complement representation. In this case a negative
 *         'big1' is converted to a result where the most significant
 *         byte (the first byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bstring with the big-endian representation.
 *  @exception RANGE_ERROR If 'big1' is negative and 'isSigned' is FALSE.
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
    logFunction(printf("bigToBStriBe(%s, %d)\n", bigHexCStri(big1), isSigned););
    /* The expression computing result_size does not overflow           */
    /* because the number of bytes in a bigInteger fits in memSizeType. */
    result_size = big1->size * (BIGDIGIT_SIZE >> 3);
    pos = big1->size - 1;
    digit = big1->bigdigits[pos];
    byteNum = (BIGDIGIT_SIZE >> 3) - 1;
    if (isSigned) {
      if (IS_NEGATIVE(digit)) {
        while (byteNum > 0 && (digit >> byteNum * CHAR_BIT & 0xFF) == UBYTE_MAX) {
          result_size--;
          byteNum--;
        } /* while */
        if (byteNum < 3 && (digit >> byteNum * CHAR_BIT & 0xFF) <= BYTE_MAX) {
          result_size++;
          byteNum++;
        } /* if */
      } else {
        while (byteNum > 0 && (digit >> byteNum * CHAR_BIT & 0xFF) == 0) {
          result_size--;
          byteNum--;
        } /* while */
        if (byteNum < 3 && (digit >> byteNum * CHAR_BIT & 0xFF) > BYTE_MAX) {
          result_size++;
          byteNum++;
        } /* if */
      } /* if */
    } else {
      if (unlikely(IS_NEGATIVE(digit))) {
        logError(printf("bigToBStriBe(%s, %d): "
                        "Number is negative and 'isSigned' is FALSE.\n",
                        bigHexCStri(big1), isSigned););
        raise_error(RANGE_ERROR);
        return NULL;
      } else {
        if (digit == 0 && pos > 0) {
          result_size -= (BIGDIGIT_SIZE >> 3);
          pos--;
          digit = big1->bigdigits[pos];
        } /* if */
        while (byteNum > 0 && (digit >> byteNum * CHAR_BIT & 0xFF) == 0) {
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
        result->mem[charIndex] = (ucharType) (digit >> byteNum * CHAR_BIT & 0xFF);
        charIndex++;
      } /* for */
      while (pos > 0) {
        pos--;
        digit = big1->bigdigits[pos];
        for (byteNum = (BIGDIGIT_SIZE >> 3) - 1; byteNum >= 0; byteNum--) {
          result->mem[charIndex] = (ucharType) (digit >> byteNum * CHAR_BIT & 0xFF);
          charIndex++;
        } /* for */
      } /* while */
    } /* if */
    logFunction(printf("bigToBStriBe --> \"%s\"\n",
                       bstriAsUnquotedCStri(result)););
    return result;
  } /* bigToBStriBe */



/**
 *  Convert a 'bigInteger' into a little-endian 'bstring'.
 *  The result uses binary representation with a base of 256.
 *  @param big1 BigInteger number to be converted.
 *  @param isSigned Determines the signedness of the result.
 *         If 'isSigned' is TRUE the result is encoded with the
 *         twos-complement representation. In this case a negative
 *         'big1' is converted to a result where the most significant
 *         byte (the last byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bstring with the little-endian representation.
 *  @exception RANGE_ERROR If 'big1' is negative and 'isSigned' is FALSE.
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
    logFunction(printf("bigToBStriLe(%s, %d)\n", bigHexCStri(big1), isSigned););
    /* The expression computing result_size does not overflow           */
    /* because the number of bytes in a bigInteger fits in memSizeType. */
    result_size = big1->size * (BIGDIGIT_SIZE >> 3);
    pos = big1->size - 1;
    digit = big1->bigdigits[pos];
    byteNum = (BIGDIGIT_SIZE >> 3) - 1;
    if (isSigned) {
      if (IS_NEGATIVE(digit)) {
        while (byteNum > 0 && (digit >> byteNum * CHAR_BIT & 0xFF) == UBYTE_MAX) {
          result_size--;
          byteNum--;
        } /* while */
        if (byteNum < 3 && (digit >> byteNum * CHAR_BIT & 0xFF) <= BYTE_MAX) {
          result_size++;
          byteNum++;
        } /* if */
      } else {
        while (byteNum > 0 && (digit >> byteNum * CHAR_BIT & 0xFF) == 0) {
          result_size--;
          byteNum--;
        } /* while */
        if (byteNum < 3 && (digit >> byteNum * CHAR_BIT & 0xFF) > BYTE_MAX) {
          result_size++;
          byteNum++;
        } /* if */
      } /* if */
    } else {
      if (unlikely(IS_NEGATIVE(digit))) {
        logError(printf("bigToBStriLe(%s, %d): "
                        "Number is negative and 'isSigned' is FALSE.\n",
                        bigHexCStri(big1), isSigned););
        raise_error(RANGE_ERROR);
        return NULL;
      } else {
        if (digit == 0 && pos > 0) {
          result_size -= (BIGDIGIT_SIZE >> 3);
          pos--;
          digit = big1->bigdigits[pos];
        } /* if */
        while (byteNum > 0 && (digit >> byteNum * CHAR_BIT & 0xFF) == 0) {
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
        result->mem[charIndex] = (ucharType) (digit >> byteNum * CHAR_BIT & 0xFF);
        charIndex--;
      } /* for */
      while (pos > 0) {
        pos--;
        digit = big1->bigdigits[pos];
        for (byteNum = (BIGDIGIT_SIZE >> 3) - 1; byteNum >= 0; byteNum--) {
          result->mem[charIndex] = (ucharType) (digit >> byteNum * CHAR_BIT & 0xFF);
          charIndex--;
        } /* for */
      } /* while */
    } /* if */
    logFunction(printf("bigToBStriLe -->\"%s\"\n",
                       bstriAsUnquotedCStri(result)););
    return result;
  } /* bigToBStriLe */



/**
 *  Convert a 'bigInteger' to an 'int16Type' number.
 *  @return the int16Type result of the conversion.
 *  @param err_info Unchanged if the function succeeds or
 *                  RANGE_ERROR The number is too small or too big
 *                  to fit into a int16Type value.
 */
int16Type bigToInt16 (const const_bigIntType big1, errInfoType *err_info)

  {
    memSizeType pos;
    int32Type result;

  /* bigToInt16 */
    logFunction(printf("bigToInt16(%s)\n", bigHexCStri(big1)););
#if BIGDIGIT_SIZE > 16
    if (unlikely(big1->size > 1)) {
#else
    if (unlikely(big1->size > sizeof(int16Type) / (BIGDIGIT_SIZE >> 3))) {
#endif
      logError(printf("bigToInt16(%s): Number too big or too small.\n",
                      bigHexCStri(big1)););
      *err_info = RANGE_ERROR;
      result = 0;
    } else {
      pos = big1->size - 1;
      result = (int32Type) (signedBigDigitType) big1->bigdigits[pos];
#if BIGDIGIT_SIZE > 16
      if (unlikely(result < INT16TYPE_MIN || result > INT16TYPE_MAX)) {
        logError(printf("bigToInt16(%s): Number too big or too small.\n",
                        bigHexCStri(big1)););
        *err_info = RANGE_ERROR;
        result = 0;
      } /* if */
#elif BIGDIGIT_SIZE < 16
      while (pos > 0) {
        pos--;
        result <<= BIGDIGIT_SIZE;
        result |= (int16Type) big1->bigdigits[pos];
      } /* while */
#endif
    } /* if */
    logFunction(printf("bigToInt16 --> " FMT_D32 "\n", result););
    return (int16Type) result;
  } /* bigToInt16 */



/**
 *  Convert a 'bigInteger' to an 'int32Type' number.
 *  @return the int32Type result of the conversion.
 *  @param big1 BigInteger to be converted.
 *  @param err_info Only used if err_info is not NULL.
 *                  Unchanged if the function succeeds or
 *                  RANGE_ERROR The number is too small or too big
 *                  to fit into a int32Type value.
 *  @exception RANGE_ERROR If err_info is NULL and the number is
 *             too small or too big to fit into a int32Type value.
 */
int32Type bigToInt32 (const const_bigIntType big1, errInfoType *err_info)

  {
    memSizeType pos;
    int32Type result;

  /* bigToInt32 */
    logFunction(printf("bigToInt32(%s)\n", bigHexCStri(big1)););
    /* Assume that BIGDIGIT_SIZE <= 32 holds. */
    if (unlikely(big1->size > sizeof(int32Type) / (BIGDIGIT_SIZE >> 3))) {
      logError(printf("bigToInt32(%s): Number too big or too small.\n",
                      bigHexCStri(big1)););
      if (err_info == NULL) {
        raise_error(RANGE_ERROR);
      } else {
        *err_info = RANGE_ERROR;
      } /* if */
      result = 0;
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
    } /* if */
    logFunction(printf("bigToInt32 --> " FMT_D32 "\n", result););
    return result;
  } /* bigToInt32 */



#ifdef INT64TYPE
/**
 *  Convert a 'bigInteger' to an 'int64Type' number.
 *  @return the int64Type result of the conversion.
 *  @param big1 BigInteger to be converted.
 *  @param err_info Only used if err_info is not NULL.
 *                  Unchanged if the function succeeds or
 *                  RANGE_ERROR The number is too small or too big
 *                  to fit into a int64Type value.
 *  @exception RANGE_ERROR If err_info is NULL and the number is
 *             too small or too big to fit into a int64Type value.
 */
int64Type bigToInt64 (const const_bigIntType big1, errInfoType *err_info)

  {
    memSizeType pos;
    int64Type result;

  /* bigToInt64 */
    logFunction(printf("bigToInt64(%s)\n", bigHexCStri(big1)););
    /* Assume that BIGDIGIT_SIZE <= 32 holds. */
    if (unlikely(big1->size > sizeof(int64Type) / (BIGDIGIT_SIZE >> 3))) {
      logError(printf("bigToInt64(%s): Number too big or too small.\n",
                      bigHexCStri(big1)););
      if (err_info == NULL) {
        raise_error(RANGE_ERROR);
      } else {
        *err_info = RANGE_ERROR;
      } /* if */
      result = 0;
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
    } /* if */
    logFunction(printf("bigToInt64 --> " FMT_D64 "\n", result););
    return result;
  } /* bigToInt64 */



/**
 *  Convert a 'bigInteger' to an 'uint64Type' number.
 *  @return the uint64Type result of the conversion.
 *  @exception RANGE_ERROR The number is negative or too big to fit
 *             into a uint64Type value.
 */
uint64Type bigToUInt64 (const const_bigIntType big1)

  {
    memSizeType pos;
    uint64Type result;

  /* bigToUInt64 */
    logFunction(printf("bigToUInt64(%s)\n", bigHexCStri(big1)););
    pos = big1->size - 1;
    if (unlikely(IS_NEGATIVE(big1->bigdigits[pos]))) {
      logError(printf("bigToUInt64(%s): Number is negative.\n",
                      bigHexCStri(big1)););
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      /* Assume that BIGDIGIT_SIZE <= 32 holds. */
      if (big1->bigdigits[pos] == 0 && pos > 0) {
        pos--;
      } /* if */
      if (unlikely(pos >= sizeof(uint64Type) / (BIGDIGIT_SIZE >> 3))) {
        logError(printf("bigToUInt64(%s): Number too big.\n",
                        bigHexCStri(big1)););
        raise_error(RANGE_ERROR);
        result = 0;
      } else {
        result = (uint64Type) big1->bigdigits[pos];
#if BIGDIGIT_SIZE < 64
        while (pos > 0) {
          pos--;
          result <<= BIGDIGIT_SIZE;
          result |= (uint64Type) big1->bigdigits[pos];
        } /* while */
#endif
      } /* if */
    } /* if */
    logFunction(printf("bigToUInt64(%s) --> " FMT_U64 "\n",
                       bigHexCStri(big1), result););
    return result;
  } /* bigToUInt64 */
#endif



bigIntType bigXor (const_bigIntType big1, const_bigIntType big2)

  {
    const_bigIntType help_big;
    memSizeType pos;
    bigDigitType big2_sign;
    bigIntType result;

  /* bigXor */
    logFunction(printf("bigXor(%s,", bigHexCStri(big1));
                printf("%s)\n", bigHexCStri(big2)););
    if (big2->size > big1->size) {
      help_big = big1;
      big1 = big2;
      big2 = help_big;
    } /* if */
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, big1->size))) {
      raise_error(MEMORY_ERROR);
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
    } /* if */
    logFunction(printf("bigXor --> %s\n", bigHexCStri(result)););
    return result;
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

#endif
