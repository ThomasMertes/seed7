/********************************************************************/
/*                                                                  */
/*  big_rtl.c     Functions for bigInteger without helping library. */
/*  Copyright (C) 1989 - 2010  Thomas Mertes                        */
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
/*  Changes: 2005, 2006, 2008, 2009, 2010  Thomas Mertes            */
/*  Content: Functions for bigInteger without helping library.      */
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


#define WITH_BIGINT_FREELIST
#define BIG_FREELIST_LENGTH_LIMIT 20
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

/* typedef uint8type             bigdigittype; */
typedef int8type                 signedbigdigittype;
typedef uint16type               doublebigdigittype;
#define digitMostSignificantBit  uint8MostSignificantBit
#define digitLeastSignificantBit uint8LeastSignificantBit
#define BIGDIGIT_MASK                    0xFF
#define BIGDIGIT_SIGN                    0x80
#define BIGDIGIT_SIZE_MASK                0x7
#define BIGDIGIT_LOG2_SIZE                  3
#define POWER_OF_10_IN_BIGDIGIT           100
#define DECIMAL_DIGITS_IN_BIGDIGIT          2

#elif BIGDIGIT_SIZE == 16

/* typedef uint16type            bigdigittype; */
typedef int16type                signedbigdigittype;
typedef uint32type               doublebigdigittype;
#define digitMostSignificantBit  uint16MostSignificantBit
#define digitLeastSignificantBit uint16LeastSignificantBit
#define BIGDIGIT_MASK                  0xFFFF
#define BIGDIGIT_SIGN                  0x8000
#define BIGDIGIT_SIZE_MASK                0xF
#define BIGDIGIT_LOG2_SIZE                  4
#define POWER_OF_10_IN_BIGDIGIT         10000
#define DECIMAL_DIGITS_IN_BIGDIGIT          4

#elif BIGDIGIT_SIZE == 32

/* typedef uint32type            bigdigittype; */
typedef int32type                signedbigdigittype;
typedef uint64type               doublebigdigittype;
#define digitMostSignificantBit  uint32MostSignificantBit
#define digitLeastSignificantBit uint32LeastSignificantBit
#define BIGDIGIT_MASK              0xFFFFFFFF
#define BIGDIGIT_SIGN              0x80000000
#define BIGDIGIT_SIZE_MASK               0x1F
#define BIGDIGIT_LOG2_SIZE                  5
#define POWER_OF_10_IN_BIGDIGIT    1000000000
#define DECIMAL_DIGITS_IN_BIGDIGIT          9

#endif


#define IS_NEGATIVE(digit) (((digit) & BIGDIGIT_SIGN) != 0)


size_t sizeof_bigdigittype = sizeof(bigdigittype);
size_t sizeof_bigintrecord = sizeof(bigintrecord);


#define SIZ_RTLBIG(len)  ((sizeof(bigintrecord) - sizeof(bigdigittype)) + (len) * sizeof(bigdigittype))
#define MAX_BIG_LEN      ((MAX_MEMSIZETYPE - sizeof(bigintrecord) + sizeof(bigdigittype)) / sizeof(bigdigittype))

#ifdef WITH_BIGINT_CAPACITY
#define HEAP_ALLOC_BIG(var,len)       (ALLOC_HEAP(var, biginttype, SIZ_RTLBIG(len))?((var)->capacity = len, CNT1_BIG(len, SIZ_RTLBIG(len)), TRUE):FALSE)
#define HEAP_FREE_BIG(var,len)        (CNT2_BIG(len, SIZ_RTLBIG(len)) FREE_HEAP(var, SIZ_RTLBIG(len)))
#define HEAP_REALLOC_BIG(v1,v2,l1,l2) if((v1=REALLOC_HEAP(v2, biginttype, SIZ_RTLBIG(l2)))!=NULL)(v1)->capacity=l2;
#else
#define HEAP_ALLOC_BIG(var,len)       (ALLOC_HEAP(var, biginttype, SIZ_RTLBIG(len))?CNT1_BIG(len, SIZ_RTLBIG(len)), TRUE:FALSE)
#define HEAP_FREE_BIG(var,len)        (CNT2_BIG(len, SIZ_RTLBIG(len)) FREE_HEAP(var, SIZ_RTLBIG(len)))
#define HEAP_REALLOC_BIG(v1,v2,l1,l2) v1=REALLOC_HEAP(v2, biginttype, SIZ_RTLBIG(l2));
#endif
#define COUNT3_BIG(len1,len2)         CNT3(CNT2_BIG(len1, SIZ_RTLBIG(len1)) CNT1_BIG(len2, SIZ_RTLBIG(len2)))

#ifdef WITH_BIGINT_FREELIST
#ifdef WITH_BIGINT_CAPACITY

#define BIG_FREELIST_ARRAY_SIZE 32

static flisttype flist[BIG_FREELIST_ARRAY_SIZE] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static unsigned int flist_len[BIG_FREELIST_ARRAY_SIZE] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define POP_BIG_OK(len)    (len) < BIG_FREELIST_ARRAY_SIZE && flist_len[len] != 0
#define PUSH_BIG_OK(var)   (var)->capacity < BIG_FREELIST_ARRAY_SIZE && flist_len[(var)->capacity] < BIG_FREELIST_LENGTH_LIMIT

#define POP_BIG(var,len)   (var = (biginttype) flist[len], flist[len] = flist[len]->next, flist_len[len]--, TRUE)
#define PUSH_BIG(var,len)  {((flisttype) var)->next = flist[len]; flist[len] = (flisttype) var; flist_len[len]++; }

#define ALLOC_BIG_SIZE_OK(var,len)    (POP_BIG_OK(len) ? POP_BIG(var, len) : HEAP_ALLOC_BIG(var, len))
#define ALLOC_BIG_CHECK_SIZE(var,len) (POP_BIG_OK(len) ? POP_BIG(var, len) : ((len)<=MAX_BIG_LEN?HEAP_ALLOC_BIG(var, len):(var=NULL, FALSE)))
#define FREE_BIG(var,len)  if (PUSH_BIG_OK(var)) PUSH_BIG(var, (var)->capacity) else HEAP_FREE_BIG(var, (var)->capacity);

#else

static flisttype flist = NULL;
static unsigned int flist_len = 0;

#define POP_BIG_OK(len)    (len) == 1 && flist_len != 0
#define PUSH_BIG_OK(len)   (len) == 1 && flist_len < BIG_FREELIST_LENGTH_LIMIT

#define POP_BIG(var)       (var = (biginttype) flist, flist = flist->next, flist_len--, TRUE)
#define PUSH_BIG(var)      {((flisttype) var)->next = flist; flist = (flisttype) var; flist_len++; }

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


#ifdef ANSI_C

void bigGrow (biginttype *const big_variable, const const_biginttype big2);
inttype bigLowestSetBit (const const_biginttype big1);
biginttype bigLShift (const const_biginttype big1, const inttype lshift);
void bigLShiftAssign (biginttype *const big_variable, inttype lshift);
biginttype bigMinus (const const_biginttype big1);
biginttype bigRem (const const_biginttype dividend, const const_biginttype divisor);
void bigRShiftAssign (biginttype *const big_variable, inttype rshift);
biginttype bigSbtr (const const_biginttype minuend, const const_biginttype subtrahend);
void bigShrink (biginttype *const big_variable, const const_biginttype big2);

#else

void bigGrow ();
inttype bigLowestSetBit ();
biginttype bigLShift ();
void bigLShiftAssign ();
biginttype bigMinus ();
biginttype bigRem ();
void bigRShiftAssign ();
biginttype bigSbtr ();
void bigShrink ();

#endif



#ifdef ANSI_C

cstritype bigHexCStri (const const_biginttype big1)
#else

cstritype bigHexCStri (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    memsizetype byteCount;
    const_cstritype stri_ptr;
    cstritype buffer;
    cstritype result;

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
        sprintf(buffer, "%02hhX", big1->bigdigits[pos]);
#elif BIGDIGIT_SIZE == 16
        sprintf(buffer, "%04hX", big1->bigdigits[pos]);
#elif BIGDIGIT_SIZE == 32
#ifdef INT32TYPE_FORMAT_L
        sprintf(buffer, "%08lX", big1->bigdigits[pos]);
#else
        sprintf(buffer, "%08X", big1->bigdigits[pos]);
#endif
#endif
        if (IS_NEGATIVE(big1->bigdigits[pos])) {
          byteCount = BIGDIGIT_SIZE >> 3;
          while (byteCount > 1 && memcmp(buffer, "FF", 2) == 0 &&
            ((buffer[2] >= '8' && buffer[2] <= '9') ||
             (buffer[2] >= 'A' && buffer[2] <= 'F'))) {
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
          sprintf(buffer, "%02hhX", big1->bigdigits[pos]);
#elif BIGDIGIT_SIZE == 16
          sprintf(buffer, "%04hX", big1->bigdigits[pos]);
#elif BIGDIGIT_SIZE == 32
#ifdef INT32TYPE_FORMAT_L
          sprintf(buffer, "%08lX", big1->bigdigits[pos]);
#else
          sprintf(buffer, "%08X", big1->bigdigits[pos]);
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
 *  Remove leading zero (or BIGDIGIT_MASK) digits from a
 *  signed big integer.
 */
#ifdef ANSI_C

static biginttype normalize (biginttype big1)
#else

static biginttype normalize (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    bigdigittype digit;

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
        biginttype resized_big1;

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



#ifdef ANSI_C

static void negate_positive_big (const biginttype big1)
#else

static void negate_positive_big (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;

  /* negate_positive_big */
    pos = 0;
    do {
      carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big1->size);
  } /* negate_positive_big */



#ifdef ANSI_C

static void positive_copy_of_negative_big (const biginttype dest,
    const const_biginttype big1)
#else

static void positive_copy_of_negative_big (dest, big1)
biginttype dest;
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;

  /* positive_copy_of_negative_big */
    pos = 0;
    do {
      carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
      dest->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big1->size);
    if (unlikely(IS_NEGATIVE(dest->bigdigits[pos - 1]))) {
      dest->bigdigits[pos] = 0;
      pos++;
    } /* if */
    dest->size = pos;
  } /* positive_copy_of_negative_big */



#ifdef ANSI_C

static biginttype alloc_positive_copy_of_negative_big (const const_biginttype big1)
#else

static biginttype alloc_positive_copy_of_negative_big (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;
    biginttype result;

  /* alloc_positive_copy_of_negative_big */
    if (likely(ALLOC_BIG_SIZE_OK(result, big1->size))) {
      pos = 0;
      do {
        carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < big1->size);
      result->size = pos;
    } /* if */
    return result;
  } /* alloc_positive_copy_of_negative_big */



/**
 *  Multiplies big1 by 10 and adds carry to the result. This
 *  function works for unsigned big integers. It is assumed
 *  that big1 contains enough memory.
 */
#ifdef ANSI_C

static INLINE void uBigMultBy10AndAdd (const biginttype big1, doublebigdigittype carry)
#else

static INLINE void uBigMultBy10AndAdd (big1, carry)
biginttype big1;
doublebigdigittype carry;
#endif

  {
    memsizetype pos;

  /* uBigMultBy10AndAdd */
    pos = 0;
    do {
      carry += (doublebigdigittype) big1->bigdigits[pos] * 10;
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big1->size);
    if (carry != 0) {
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      big1->size++;
    } /* if */
  } /* uBigMultBy10AndAdd */



/**
 *  Divides the unsigned big integer big1 by POWER_OF_10_IN_BIGDIGIT
 *  and returns the remainder.
 */
#ifdef ANSI_C

static INLINE bigdigittype uBigDivideByPowerOf10 (const biginttype big1)
#else

static INLINE bigdigittype uBigDivideByPowerOf10 (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;
    bigdigittype bigdigit;

  /* uBigDivideByPowerOf10 */
    pos = big1->size;
    do {
      pos--;
      carry <<= BIGDIGIT_SIZE;
      carry += big1->bigdigits[pos];
      bigdigit = (bigdigittype) (carry / POWER_OF_10_IN_BIGDIGIT);
#if POINTER_SIZE <= BIGDIGIT_SIZE
      /* There is probably no machine instruction for division    */
      /* and remainder of doublebigdigittype values available.    */
      /* To compute the remainder fast the % operator is avoided  */
      /* and the remainder is computed with a multiplication and  */
      /* a subtraction. The overflow in the multiplication can be */
      /* ignored, since the result of the subtraction fits in the */
      /* lower bigdigit of carry. The wrong bits in the higher    */
      /* bigdigit of carry are masked away.                       */
      carry = (carry - bigdigit * POWER_OF_10_IN_BIGDIGIT) & BIGDIGIT_MASK;
#else
      /* There is probably a machine instruction for division     */
      /* and remainder of doublebigdigittype values available.    */
      /* In the optimal case quotient and remainder can be        */
      /* computed with one instruction.                           */
      carry %= POWER_OF_10_IN_BIGDIGIT;
#endif
      big1->bigdigits[pos] = bigdigit;
    } while (pos > 0);
    return (bigdigittype) (carry);
  } /* uBigDivideByPowerOf10 */



/**
 *  Shifts the big integer big1 to the left by lshift bits.
 *  Bits which are shifted out at the left of big1 are lost.
 *  At the right of big1 zero bits are shifted in. The function
 *  is called for 0 < lshift < BIGDIGIT_SIZE.
 */
#ifdef ANSI_C

static void uBigLShift (const biginttype big1, const unsigned int lshift)
#else

static void uBigLShift (big1, lshift)
biginttype big1;
unsigned int lshift;
#endif

  {
    doublebigdigittype carry = 0;
    memsizetype pos;

  /* uBigLShift */
    pos = 0;
    do {
      carry |= ((doublebigdigittype) big1->bigdigits[pos]) << lshift;
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
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
#ifdef ANSI_C

static void uBigLShift (const biginttype big1, const unsigned int lshift)
#else

static void uBigLShift (big1, lshift)
biginttype big1;
unsigned int lshift;
#endif

  {
    unsigned int rshift = BIGDIGIT_SIZE - lshift;
    bigdigittype low_digit;
    bigdigittype high_digit;
    memsizetype pos;

  /* uBigLShift */
    low_digit = big1->bigdigits[0];
    big1->bigdigits[0] = (bigdigittype) ((low_digit << lshift) & BIGDIGIT_MASK);
    for (pos = 1; pos < big1->size; pos++) {
      high_digit = big1->bigdigits[pos];
      big1->bigdigits[pos] = (bigdigittype)
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
#ifdef ANSI_C

static void uBigRShift (const biginttype big1, const unsigned int rshift)
#else

static void uBigRShift (big1, rshift)
biginttype big1;
unsigned int rshift;
#endif

  {
    unsigned int lshift = BIGDIGIT_SIZE - rshift;
    doublebigdigittype carry = 0;
    memsizetype pos;

  /* uBigRShift */
    pos = big1->size;
    do {
      carry <<= BIGDIGIT_SIZE;
      carry |= ((doublebigdigittype) big1->bigdigits[pos]) << lshift;
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
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
#ifdef ANSI_C

static void uBigRShift (const biginttype big1, const unsigned int rshift)
#else

static void uBigRShift (big1, rshift)
biginttype big1;
unsigned int rshift;
#endif

  {
    unsigned int lshift = BIGDIGIT_SIZE - rshift;
    bigdigittype low_digit;
    bigdigittype high_digit;
    memsizetype pos;

  /* uBigRShift */
    high_digit = 0;
    for (pos = big1->size - 1; pos != 0; pos--) {
      low_digit = big1->bigdigits[pos];
      big1->bigdigits[pos] = (bigdigittype)
          (((low_digit >> rshift) | (high_digit << lshift)) & BIGDIGIT_MASK);
      high_digit = low_digit;
    } /* for */
    low_digit = big1->bigdigits[0];
    big1->bigdigits[0] = (bigdigittype)
        (((low_digit >> rshift) | (high_digit << lshift)) & BIGDIGIT_MASK);
  } /* uBigRShift */



/**
 *  Increments an unsigned big integer by 1. This function does
 *  overflow silently when big1 contains not enough digits.
 */
#ifdef ANSI_C

static void uBigIncr (const biginttype big1)
#else

static void uBigIncr (big1)
biginttype big1;
#endif

  {
    memsizetype pos;

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
        /* memset(big1->bigdigits, 0, pos * sizeof(bigdigittype)); */
      } /* if */
    } /* if */
    if (pos < big1->size) {
      big1->bigdigits[pos]++;
    } /* if */
  } /* uBigIncr */



/**
 *  Decrements an unsigned big integer by 1. This function does
 *  overflow silently when big1 contains not enough digits. The
 *  function works correctly when there are leading zereo digits.
 */
#ifdef ANSI_C

static void uBigDecr (const biginttype big1)
#else

static void uBigDecr (big1)
biginttype big1;
#endif

  {
    memsizetype pos;

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
#ifdef ANSI_C

static void uBigDiv1 (const const_biginttype dividend,
    const bigdigittype divisor_digit, const biginttype result)
#else

static void uBigDiv1 (dividend, divisor_digit, result)
biginttype dividend;
bigdigittype divisor_digit;
biginttype result;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;

  /* uBigDiv1 */
    pos = dividend->size;
    do {
      pos--;
      carry <<= BIGDIGIT_SIZE;
      carry += dividend->bigdigits[pos];
      result->bigdigits[pos] = (bigdigittype) ((carry / divisor_digit) & BIGDIGIT_MASK);
      carry %= divisor_digit;
    } while (pos > 0);
  } /* uBigDiv1 */



/**
 *  Computes an integer division of dividend by one divisor_digit
 *  for signed big integers. The memory for the result is requested
 *  and the normalized result is returned. This function handles
 *  also the special case of a division by zero.
 */
#ifdef ANSI_C

static biginttype bigDiv1 (const_biginttype dividend, bigdigittype divisor_digit)
#else

static biginttype bigDiv1 (dividend, divisor_digit)
biginttype dividend;
bigdigittype divisor_digit;
#endif

  {
    booltype negative = FALSE;
    biginttype dividend_help = NULL;
    biginttype result;

  /* bigDiv1 */
    if (unlikely(divisor_digit == 0)) {
      raise_error(NUMERIC_ERROR);
      return NULL;
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, dividend->size + 1))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        result->size = dividend->size + 1;
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          negative = TRUE;
          dividend_help = alloc_positive_copy_of_negative_big(dividend);
          dividend = dividend_help;
          if (unlikely(dividend_help == NULL)) {
            FREE_BIG(result, result->size);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
        } /* if */
        result->bigdigits[result->size - 1] = 0;
        if (IS_NEGATIVE(divisor_digit)) {
          negative = !negative;
          /* The unsigned value is negated to avoid a signed integer */
          /* overflow when the smallest signed integer is negated.   */
          divisor_digit = -divisor_digit;
        } /* if */
        uBigDiv1(dividend, divisor_digit, result);
        if (negative) {
          negate_positive_big(result);
        } /* if */
        result = normalize(result);
        if (dividend_help != NULL) {
          FREE_BIG(dividend_help, dividend_help->size);
        } /* if */
        return result;
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
 */
#ifdef ANSI_C

static biginttype bigDivSizeLess (const const_biginttype dividend,
    const const_biginttype divisor)
#else

static biginttype bigDivSizeLess (dividend, divisor)
biginttype dividend;
biginttype divisor;
#endif

  {
    memsizetype pos;
    biginttype result;

  /* bigDivSizeLess */
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = 1;
      if (unlikely(dividend->size + 1 == divisor->size &&
                   dividend->bigdigits[dividend->size - 1] == BIGDIGIT_SIGN &&
                   divisor->bigdigits[divisor->size - 1] == 0 &&
                   divisor->bigdigits[divisor->size - 2] == BIGDIGIT_SIGN)) {
        result->bigdigits[0] = BIGDIGIT_MASK;
        pos = dividend->size - 1;
        while (pos > 0) {
          pos--;
          if (likely(dividend->bigdigits[pos] != 0 || divisor->bigdigits[pos] != 0)) {
            result->bigdigits[0] = 0;
            pos = 0;
          } /* if */
        } /* while */
      } else {
        result->bigdigits[0] = 0;
      } /* if */
      return result;
    } /* if */
  } /* bigDivSizeLess */



/**
 *  Multiplies big2 with multiplier and subtracts the result from
 *  big1 at the digit position pos1 of big1. Big1, big2 and
 *  multiplier are nonnegative big integer values.
 *  The algorithm tries to save computations. Therefore
 *  there are checks for mult_carry != 0 and sbtr_carry == 0.
 */
#ifdef ANSI_C

static bigdigittype uBigMultSub (const biginttype big1, const const_biginttype big2,
    const bigdigittype multiplier, const memsizetype pos1)
#else

static bigdigittype uBigMultSub (big1, big2, multiplier, pos1)
biginttype big1;
biginttype big2;
bigdigittype multiplier;
memsizetype pos1;
#endif

  {
    memsizetype pos;
    doublebigdigittype mult_carry = 0;
    doublebigdigittype sbtr_carry = 1;

  /* uBigMultSub */
    pos = 0;
    do {
      mult_carry += (doublebigdigittype) big2->bigdigits[pos] * multiplier;
      sbtr_carry += big1->bigdigits[pos1 + pos] + (~mult_carry & BIGDIGIT_MASK);
      big1->bigdigits[pos1 + pos] = (bigdigittype) (sbtr_carry & BIGDIGIT_MASK);
      mult_carry >>= BIGDIGIT_SIZE;
      sbtr_carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big2->size);
    for (pos += pos1; mult_carry != 0 && pos < big1->size; pos++) {
      sbtr_carry += big1->bigdigits[pos] + (~mult_carry & BIGDIGIT_MASK);
      big1->bigdigits[pos] = (bigdigittype) (sbtr_carry & BIGDIGIT_MASK);
      mult_carry >>= BIGDIGIT_SIZE;
      sbtr_carry >>= BIGDIGIT_SIZE;
    } /* for */
    for (; sbtr_carry == 0 && pos < big1->size; pos++) {
      sbtr_carry = (doublebigdigittype) big1->bigdigits[pos] + BIGDIGIT_MASK;
      big1->bigdigits[pos] = (bigdigittype) (sbtr_carry & BIGDIGIT_MASK);
      sbtr_carry >>= BIGDIGIT_SIZE;
    } /* for */
    return (bigdigittype) (sbtr_carry & BIGDIGIT_MASK);
  } /* uBigMultSub */



/**
 *  Adds big2 to big1 at the digit position pos1. Big1 and big2
 *  are nonnegative big integer values. The size of big1 must be
 *  greater or equal the size of big2. The final carry is ignored.
 */
#ifdef ANSI_C

static void uBigAddTo (const biginttype big1, const const_biginttype big2,
    const memsizetype pos1)
#else

static void uBigAddTo (big1, big2, pos1)
biginttype big1;
biginttype big2;
memsizetype pos1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;

  /* uBigAddTo */
    pos = 0;
    do {
      carry += (doublebigdigittype) big1->bigdigits[pos1 + pos] + big2->bigdigits[pos];
      big1->bigdigits[pos1 + pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big2->size);
    for (pos += pos1; pos < big1->size; pos++) {
      carry += big1->bigdigits[pos];
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
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
#ifdef ANSI_C

static void uBigDiv (const biginttype dividend, const const_biginttype divisor,
    const biginttype result)
#else

static void uBigDiv (dividend, divisor, result)
biginttype dividend;
biginttype divisor;
biginttype result;
#endif

  {
    memsizetype pos1;
    doublebigdigittype twodigits;
    doublebigdigittype remainder;
    bigdigittype quotientdigit;
    bigdigittype sbtr_carry;

  /* uBigDiv */
    for (pos1 = dividend->size - 1; pos1 >= divisor->size; pos1--) {
      twodigits = (((doublebigdigittype) dividend->bigdigits[pos1]) << BIGDIGIT_SIZE) |
          dividend->bigdigits[pos1 - 1];
      if (unlikely(dividend->bigdigits[pos1] == divisor->bigdigits[divisor->size - 1])) {
        quotientdigit = BIGDIGIT_MASK;
      } else {
        quotientdigit = (bigdigittype) (twodigits / divisor->bigdigits[divisor->size - 1]);
      } /* if */
      remainder = twodigits - (doublebigdigittype) quotientdigit *
          divisor->bigdigits[divisor->size - 1];
      while (remainder <= BIGDIGIT_MASK &&
          (doublebigdigittype) divisor->bigdigits[divisor->size - 2] * quotientdigit >
          (remainder << BIGDIGIT_SIZE | dividend->bigdigits[pos1 - 2])) {
        quotientdigit--;
        remainder = twodigits - (doublebigdigittype) quotientdigit *
            divisor->bigdigits[divisor->size - 1];
      } /* while */
      sbtr_carry = uBigMultSub(dividend, divisor, quotientdigit, pos1 - divisor->size);
      if (sbtr_carry == 0) {
        uBigAddTo(dividend, divisor, pos1 - divisor->size);
        quotientdigit--;
      } /* if */
      result->bigdigits[pos1 - divisor->size] = quotientdigit;
    } /* for */
  } /* uBigDiv */



/**
 *  Computes the remainder of a integer division of dividend by
 *  one divisor_digit for nonnegative big integers. The divisor_digit must
 *  not be zero.
 */
#ifdef ANSI_C

static bigdigittype uBigRem1 (const const_biginttype dividend, const bigdigittype divisor_digit)
#else

static bigdigittype uBigRem1 (dividend, divisor_digit)
biginttype dividend;
bigdigittype divisor_digit;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;

  /* uBigRem1 */
    pos = dividend->size;
    do {
      pos--;
      carry <<= BIGDIGIT_SIZE;
      carry += dividend->bigdigits[pos];
      carry %= divisor_digit;
    } while (pos > 0);
    return (bigdigittype) carry;
  } /* uBigRem1 */



/**
 *  Computes the remainder of the integer division dividend by one
 *  divisor_digit for signed big integers. The memory for the
 *  remainder is requested and the normalized remainder is
 *  returned. This function handles also the special case of a
 *  division by zero.
 */
#ifdef ANSI_C

static biginttype bigRem1 (const_biginttype dividend, bigdigittype divisor_digit)
#else

static biginttype bigRem1 (dividend, divisor_digit)
biginttype dividend;
bigdigittype divisor_digit;
#endif

  {
    booltype negative = FALSE;
    biginttype dividend_help = NULL;
    biginttype remainder;

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
#ifdef ANSI_C

static bigdigittype uBigMDiv1 (const const_biginttype dividend,
    const bigdigittype divisor_digit, const biginttype result)
#else

static bigdigittype uBigMDiv1 (dividend, divisor_digit, result)
biginttype dividend;
bigdigittype divisor_digit;
biginttype result;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;

  /* uBigMDiv1 */
    pos = dividend->size;
    do {
      pos--;
      carry <<= BIGDIGIT_SIZE;
      carry += dividend->bigdigits[pos];
      result->bigdigits[pos] = (bigdigittype) ((carry / divisor_digit) & BIGDIGIT_MASK);
      carry %= divisor_digit;
    } while (pos > 0);
    return (bigdigittype) carry;
  } /* uBigMDiv1 */



/**
 *  Computes an integer modulo division of dividend by one
 *  divisor_digit for signed big integers. The memory for the
 *  result is requested and the normalized result is returned.
 *  This function handles also the special case of a division by
 *  zero.
 */
#ifdef ANSI_C

static biginttype bigMDiv1 (const_biginttype dividend, bigdigittype divisor_digit)
#else

static biginttype bigMDiv1 (dividend, divisor_digit)
biginttype dividend;
bigdigittype divisor_digit;
#endif

  {
    booltype negative = FALSE;
    biginttype dividend_help = NULL;
    bigdigittype remainder;
    biginttype result;

  /* bigMDiv1 */
    if (unlikely(divisor_digit == 0)) {
      raise_error(NUMERIC_ERROR);
      return NULL;
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, dividend->size + 1))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        result->size = dividend->size + 1;
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          negative = TRUE;
          dividend_help = alloc_positive_copy_of_negative_big(dividend);
          dividend = dividend_help;
          if (unlikely(dividend_help == NULL)) {
            FREE_BIG(result, result->size);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
        } /* if */
        result->bigdigits[result->size - 1] = 0;
        if (IS_NEGATIVE(divisor_digit)) {
          negative = !negative;
          /* The unsigned value is negated to avoid a signed integer */
          /* overflow when the smallest signed integer is negated.   */
          divisor_digit = -divisor_digit;
        } /* if */
        remainder = uBigMDiv1(dividend, divisor_digit, result);
        if (negative) {
          if (remainder != 0) {
            uBigIncr(result);
          } /* if */
          negate_positive_big(result);
        } /* if */
        result = normalize(result);
        if (dividend_help != NULL) {
          FREE_BIG(dividend_help, dividend_help->size);
        } /* if */
        return result;
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
 */
#ifdef ANSI_C

static biginttype bigMDivSizeLess (const const_biginttype dividend,
    const const_biginttype divisor)
#else

static biginttype bigMDivSizeLess (dividend, divisor)
biginttype dividend;
biginttype divisor;
#endif

  {
    memsizetype pos;
    biginttype result;

  /* bigMDivSizeLess */
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = 1;
      if (unlikely(dividend->size + 1 == divisor->size &&
                   dividend->bigdigits[dividend->size - 1] == BIGDIGIT_SIGN &&
                   divisor->bigdigits[divisor->size - 1] == 0 &&
                   divisor->bigdigits[divisor->size - 2] == BIGDIGIT_SIGN)) {
        result->bigdigits[0] = BIGDIGIT_MASK;
        pos = dividend->size - 1;
        while (pos > 0) {
          pos--;
          if (likely(dividend->bigdigits[pos] != 0 || divisor->bigdigits[pos] != 0)) {
            result->bigdigits[0] = 0;
            pos = 0;
          } /* if */
        } /* while */
      } else {
        result->bigdigits[0] = 0;
      } /* if */
      if (result->bigdigits[0] == 0 &&
          IS_NEGATIVE(dividend->bigdigits[dividend->size - 1]) !=
          IS_NEGATIVE(divisor->bigdigits[divisor->size - 1])) {
        result->bigdigits[0] = BIGDIGIT_MASK;
      } /* if */
      return result;
    } /* if */
  } /* bigMDivSizeLess */



/**
 *  Computes the modulo of the integer division dividend by one
 *  digit for signed big integers. The memory for the modulo is
 *  requested and the normalized modulo is returned. This function
 *  handles also the special case of a division by zero.
 */
#ifdef ANSI_C

static biginttype bigMod1 (const const_biginttype dividend, const bigdigittype digit)
#else

static biginttype bigMod1 (dividend, digit)
biginttype dividend;
bigdigittype digit;
#endif

  {
    biginttype modulo;

  /* bigMod1 */
    modulo = bigRem1(dividend, digit);
    if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1]) != IS_NEGATIVE(digit) &&
        modulo != NULL && modulo->bigdigits[0] != 0) {
      modulo->bigdigits[0] += digit;
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
#ifdef ANSI_C

static biginttype bigRemSizeLess (const const_biginttype dividend,
    const const_biginttype divisor)
#else

static biginttype bigRemSizeLess (dividend, divisor)
biginttype dividend;
biginttype divisor;
#endif

  {
    memsizetype pos;
    booltype remainderIs0;
    biginttype remainder;

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
            (size_t) dividend->size * sizeof(bigdigittype));
      } /* if */
    } /* if */
    return remainder;
  } /* bigRemSizeLess */



/**
 *  Adds big2 to big1 at the digit position pos1. Big1 and big2
 *  are signed big integer values. The size of big1 must be
 *  greater or equal the size of big2.
 */
#ifdef ANSI_C

static void bigAddTo (const biginttype big1, const const_biginttype big2)
#else

static void bigAddTo (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;
    doublebigdigittype big2_sign;

  /* bigAddTo */
    pos = 0;
    do {
      carry += (doublebigdigittype) big1->bigdigits[pos] + big2->bigdigits[pos];
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < big2->size);
    big2_sign = IS_NEGATIVE(big2->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
    for (; pos < big1->size; pos++) {
      carry += big1->bigdigits[pos] + big2_sign;
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
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
#ifdef ANSI_C

static biginttype bigModSizeLess (const const_biginttype dividend,
    const const_biginttype divisor)
#else

static biginttype bigModSizeLess (dividend, divisor)
biginttype dividend;
biginttype divisor;
#endif

  {
    memsizetype pos;
    booltype moduloIs0;
    biginttype modulo;

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
              (size_t) divisor->size * sizeof(bigdigittype));
          bigAddTo(modulo, dividend);
          modulo = normalize(modulo);
        } /* if */
      } else {
        if (unlikely(!ALLOC_BIG_SIZE_OK(modulo, dividend->size))) {
          raise_error(MEMORY_ERROR);
        } else {
          modulo->size = dividend->size;
          memcpy(modulo->bigdigits, dividend->bigdigits,
              (size_t) dividend->size * sizeof(bigdigittype));
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
#ifdef ANSI_C

static void uBigRem (const biginttype dividend, const const_biginttype divisor)
#else

static void uBigRem (dividend, divisor)
biginttype dividend;
biginttype divisor;
#endif

  {
    memsizetype pos1;
    doublebigdigittype twodigits;
    doublebigdigittype remainder;
    bigdigittype quotientdigit;
    bigdigittype sbtr_carry;

  /* uBigRem */
    for (pos1 = dividend->size - 1; pos1 >= divisor->size; pos1--) {
      twodigits = (((doublebigdigittype) dividend->bigdigits[pos1]) << BIGDIGIT_SIZE) |
          dividend->bigdigits[pos1 - 1];
      if (unlikely(dividend->bigdigits[pos1] == divisor->bigdigits[divisor->size - 1])) {
        quotientdigit = BIGDIGIT_MASK;
      } else {
        quotientdigit = (bigdigittype) (twodigits / divisor->bigdigits[divisor->size - 1]);
      } /* if */
      remainder = twodigits - (doublebigdigittype) quotientdigit *
          divisor->bigdigits[divisor->size - 1];
      while (remainder <= BIGDIGIT_MASK &&
          (doublebigdigittype) divisor->bigdigits[divisor->size - 2] * quotientdigit >
          (remainder << BIGDIGIT_SIZE | dividend->bigdigits[pos1 - 2])) {
        quotientdigit--;
        remainder = twodigits - (doublebigdigittype) quotientdigit *
            divisor->bigdigits[divisor->size - 1];
      } /* while */
      sbtr_carry = uBigMultSub(dividend, divisor, quotientdigit, pos1 - divisor->size);
      if (sbtr_carry == 0) {
        uBigAddTo(dividend, divisor, pos1 - divisor->size);
      } /* if */
    } /* for */
  } /* uBigRem */



#ifdef ANSI_C

static void uBigDigitAdd (const bigdigittype *const big1, const memsizetype size1,
    const bigdigittype *const big2, const memsizetype size2, bigdigittype *const result)
#else

static void uBigDigitAdd (big1, size1, big2, size2, result)
bigdigittype *big1;
memsizetype size1;
bigdigittype *big2;
memsizetype size2;
bigdigittype *result;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;

  /* uBigDigitAdd */
    pos = 0;
    do {
      carry += (doublebigdigittype) big1[pos] + big2[pos];
      result[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < size2);
    for (; pos < size1; pos++) {
      carry += big1[pos];
      result[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
    } /* for */
    result[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
  } /* uBigDigitAdd */



#ifdef ANSI_C

static void uBigDigitSbtrFrom (bigdigittype *const big1, const memsizetype size1,
    const bigdigittype *const big2, const memsizetype size2)
#else

static void uBigDigitSbtrFrom (big1, size1, big2, size2)
bigdigittype *big1;
memsizetype size1;
bigdigittype *big2;
memsizetype size2;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;

  /* uBigDigitSbtrFrom */
    pos = 0;
    do {
      carry += (doublebigdigittype) big1[pos] +
          (~big2[pos] & BIGDIGIT_MASK);
      big1[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < size2);
    for (; carry == 0 && pos < size1; pos++) {
      carry = (doublebigdigittype) big1[pos] + BIGDIGIT_MASK;
      big1[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
    } /* for */
  } /* uBigDigitSbtrFrom */



#ifdef ANSI_C

static void uBigDigitAddTo (bigdigittype *const big1,  const memsizetype size1,
    const bigdigittype *const big2, const memsizetype size2)
#else

static void uBigDigitAddTo (big1, size1, big2, size2)
bigdigittype *big1;
memsizetype size1;
bigdigittype *big2;
memsizetype size2;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;

  /* uBigDigitAddTo */
    pos = 0;
    do {
      carry += (doublebigdigittype) big1[pos] + big2[pos];
      big1[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos++;
    } while (pos < size2);
    for (; carry != 0 && pos < size1; pos++) {
      carry += big1[pos];
      big1[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
    } /* for */
  } /* uBigDigitAddTo */



#ifdef ANSI_C

static void uBigDigitMult (const bigdigittype *const big1,
    const bigdigittype *const big2, const memsizetype size,
    bigdigittype *const result)
#else

static void uBigDigitMult (big1, big2, size, result)
bigdigittype *big1;
bigdigittype *big2;
memsizetype size;
bigdigittype *result;
#endif

  {
    memsizetype pos1;
    memsizetype pos2;
    doublebigdigittype carry;
    doublebigdigittype carry2 = 0;
    doublebigdigittype prod;

  /* uBigDigitMult */
    carry = (doublebigdigittype) big1[0] * big2[0];
    result[0] = (bigdigittype) (carry & BIGDIGIT_MASK);
    carry >>= BIGDIGIT_SIZE;
    for (pos1 = 1; pos1 < size; pos1++) {
      pos2 = 0;
      do {
        prod = (doublebigdigittype) big1[pos2] * big2[pos1 - pos2];
        carry2 += carry > (doublebigdigittype) ~prod ? 1 : 0;
        carry += prod;
        pos2++;
      } while (pos2 <= pos1);
      result[pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      carry |= (carry2 & BIGDIGIT_MASK) << BIGDIGIT_SIZE;
      carry2 >>= BIGDIGIT_SIZE;
    } /* for */
    for (; pos1 < size + size - 1; pos1++) {
      pos2 = pos1 - size + 1;
      do {
        prod = (doublebigdigittype) big1[pos2] * big2[pos1 - pos2];
        carry2 += carry > (doublebigdigittype) ~prod ? 1 : 0;
        carry += prod;
        pos2++;
      } while (pos2 < size);
      result[pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      carry |= (carry2 & BIGDIGIT_MASK) << BIGDIGIT_SIZE;
      carry2 >>= BIGDIGIT_SIZE;
    } /* for */
    result[pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
  } /* uBigDigitMult */



#ifdef ANSI_C

static void uBigKaratsubaMult (const bigdigittype *const big1,
    const bigdigittype *const big2, const memsizetype size,
    bigdigittype *const result, bigdigittype *const temp)
#else

static void uBigKaratsubaMult (big1, big2, size, result, temp)
bigdigittype *big1;
bigdigittype *big2;
memsizetype size;
bigdigittype *result;
bigdigittype *temp;
#endif

  {
    memsizetype sizeLo;
    memsizetype sizeHi;

  /* uBigKaratsubaMult */
    if (size < KARATSUBA_THRESHOLD) {
      uBigDigitMult(big1, big2, size, result);
    } else {
      sizeHi = size >> 1;
      sizeLo = size - sizeHi;
      uBigDigitAdd(big1, sizeLo, &big1[sizeLo], sizeHi, result);
      uBigDigitAdd(big2, sizeLo, &big2[sizeLo], sizeHi, &result[size]);
      uBigKaratsubaMult(result, &result[size], sizeLo + 1, temp, &temp[(sizeLo + 1) << 1]);
      uBigKaratsubaMult(big1, big2, sizeLo, result, &temp[(sizeLo + 1) << 1]);
      uBigKaratsubaMult(&big1[sizeLo], &big2[sizeLo], sizeHi, &result[sizeLo << 1], &temp[(sizeLo + 1) << 1]);
      uBigDigitSbtrFrom(temp, (sizeLo + 1) << 1, result, sizeLo << 1);
      uBigDigitSbtrFrom(temp, (sizeLo + 1) << 1, &result[sizeLo << 1], sizeHi << 1);
      uBigDigitAddTo(&result[sizeLo], sizeLo + (sizeHi << 1), temp, (sizeLo + 1) << 1);
    } /* if */
  } /* uBigKaratsubaMult */



#ifdef ANSI_C

static void uBigDigitSquare (const bigdigittype *const big1,
    const memsizetype size, bigdigittype *const result)
#else

static void uBigDigitSquare (big1, size, result)
bigdigittype *big1;
memsizetype size;
bigdigittype *result;
#endif

  {
    memsizetype pos1;
    memsizetype pos2;
    doublebigdigittype carry;
    doublebigdigittype product;
    bigdigittype digit;

  /* uBigDigitSquare */
    digit = big1[0];
    carry = (doublebigdigittype) digit * digit;
    result[0] = (bigdigittype) (carry & BIGDIGIT_MASK);
    carry >>= BIGDIGIT_SIZE;
    if (size == 1) {
      result[1] = (bigdigittype) (carry);
    } else {
      for (pos2 = 1; pos2 < size; pos2++) {
        product = (doublebigdigittype) digit * big1[pos2];
        carry += (product << 1) & BIGDIGIT_MASK;
        result[pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        carry += product >> (BIGDIGIT_SIZE - 1);
      } /* for */
      result[pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
      result[pos2 + 1] = (bigdigittype) (carry >> BIGDIGIT_SIZE);
      for (pos1 = 1; pos1 < size; pos1++) {
        digit = big1[pos1];
        carry = (doublebigdigittype) result[pos1 << 1] +
            (doublebigdigittype) digit * digit;
        result[pos1 << 1] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        for (pos2 = pos1 + 1; pos2 < size; pos2++) {
          product = (doublebigdigittype) digit * big1[pos2];
          carry += (doublebigdigittype) result[pos1 + pos2] +
              ((product << 1) & BIGDIGIT_MASK);
          result[pos1 + pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          carry += product >> (BIGDIGIT_SIZE - 1);
        } /* for */
        carry += (doublebigdigittype) result[pos1 + pos2];
        result[pos1 + pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
        if (pos1 < size - 1) {
          result[pos1 + pos2 + 1] = (bigdigittype) (carry >> BIGDIGIT_SIZE);
        } /* if */
      } /* for */
    } /* if */
  } /* uBigDigitSquare */



#ifdef ANSI_C

static void uBigKaratsubaSquare (const bigdigittype *const big1,
    const memsizetype size, bigdigittype *const result, bigdigittype *const temp)
#else

static void uBigKaratsubaSquare (big1, size, result, temp)
bigdigittype *big1;
memsizetype size;
bigdigittype *result;
bigdigittype *temp;
#endif

  {
    memsizetype sizeLo;
    memsizetype sizeHi;

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


#ifdef ANSI_C

static void uBigMultPositiveWithDigit (const const_biginttype big1, bigdigittype bigDigit,
    const biginttype result)
#else

static void uBigMultPositiveWithDigit (big1, bigDigit, result)
biginttype big1;
bigdigittype bigDigit;
biginttype result;
#endif

  {
    memsizetype pos;
    doublebigdigittype mult_carry;

  /* uBigMultPositiveWithDigit */
    mult_carry = (doublebigdigittype) big1->bigdigits[0] * bigDigit;
    result->bigdigits[0] = (bigdigittype) (mult_carry & BIGDIGIT_MASK);
    mult_carry >>= BIGDIGIT_SIZE;
    for (pos = 1; pos < big1->size; pos++) {
      mult_carry += (doublebigdigittype) big1->bigdigits[pos] * bigDigit;
      result->bigdigits[pos] = (bigdigittype) (mult_carry & BIGDIGIT_MASK);
      mult_carry >>= BIGDIGIT_SIZE;
    } /* for */
    result->bigdigits[pos] = (bigdigittype) (mult_carry & BIGDIGIT_MASK);
  } /* uBigMultPositiveWithDigit */



#ifdef ANSI_C

static void uBigMultNegativeWithDigit (const const_biginttype big1, bigdigittype bigDigit,
    const biginttype result)
#else

static void uBigMultNegativeWithDigit (big1, bigDigit, result)
biginttype big1;
bigdigittype bigDigit;
biginttype result;
#endif

  {
    memsizetype pos;
    doublebigdigittype negate_carry = 1;
    doublebigdigittype mult_carry;
    doublebigdigittype result_carry = 1;

  /* uBigMultNegativeWithDigit */
    negate_carry += ~big1->bigdigits[0] & BIGDIGIT_MASK;
    mult_carry = (negate_carry & BIGDIGIT_MASK) * bigDigit;
    result_carry += ~mult_carry & BIGDIGIT_MASK;
    result->bigdigits[0] = (bigdigittype) (result_carry & BIGDIGIT_MASK);
    negate_carry >>= BIGDIGIT_SIZE;
    mult_carry >>= BIGDIGIT_SIZE;
    result_carry >>= BIGDIGIT_SIZE;
    for (pos = 1; pos < big1->size; pos++) {
      negate_carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
      mult_carry += (negate_carry & BIGDIGIT_MASK) * bigDigit;
      result_carry += ~mult_carry & BIGDIGIT_MASK;
      result->bigdigits[pos] = (bigdigittype) (result_carry & BIGDIGIT_MASK);
      negate_carry >>= BIGDIGIT_SIZE;
      mult_carry >>= BIGDIGIT_SIZE;
      result_carry >>= BIGDIGIT_SIZE;
    } /* for */
    result_carry += ~mult_carry & BIGDIGIT_MASK;
    result->bigdigits[pos] = (bigdigittype) (result_carry & BIGDIGIT_MASK);
  } /* uBigMultNegativeWithDigit */



#ifdef ANSI_C

static void uBigMultPositiveWithNegatedDigit (const const_biginttype big1, bigdigittype bigDigit,
    const biginttype result)
#else

static void uBigMultPositiveWithNegatedDigit (big1, bigDigit, result)
biginttype big1;
bigdigittype bigDigit;
biginttype result;
#endif

  {
    memsizetype pos;
    doublebigdigittype mult_carry;
    doublebigdigittype result_carry = 1;

  /* uBigMultPositiveWithNegatedDigit */
    mult_carry = (doublebigdigittype) big1->bigdigits[0] * bigDigit;
    result_carry += ~mult_carry & BIGDIGIT_MASK;
    result->bigdigits[0] = (bigdigittype) (result_carry & BIGDIGIT_MASK);
    mult_carry >>= BIGDIGIT_SIZE;
    result_carry >>= BIGDIGIT_SIZE;
    for (pos = 1; pos < big1->size; pos++) {
      mult_carry += (doublebigdigittype) big1->bigdigits[pos] * bigDigit;
      result_carry += ~mult_carry & BIGDIGIT_MASK;
      result->bigdigits[pos] = (bigdigittype) (result_carry & BIGDIGIT_MASK);
      mult_carry >>= BIGDIGIT_SIZE;
      result_carry >>= BIGDIGIT_SIZE;
    } /* for */
    result_carry += ~mult_carry & BIGDIGIT_MASK;
    result->bigdigits[pos] = (bigdigittype) (result_carry & BIGDIGIT_MASK);
  } /* uBigMultPositiveWithNegatedDigit */



#ifdef ANSI_C

static void uBigMultNegativeWithNegatedDigit (const const_biginttype big1, bigdigittype bigDigit,
    const biginttype result)
#else

static void uBigMultNegativeWithNegatedDigit (big1, bigDigit, result)
biginttype big1;
bigdigittype bigDigit;
biginttype result;
#endif

  {
    memsizetype pos;
    doublebigdigittype negate_carry = 1;
    doublebigdigittype mult_carry;

  /* uBigMultNegativeWithNegatedDigit */
    negate_carry += ~big1->bigdigits[0] & BIGDIGIT_MASK;
    mult_carry = (negate_carry & BIGDIGIT_MASK) * bigDigit;
    result->bigdigits[0] = (bigdigittype) (mult_carry & BIGDIGIT_MASK);
    negate_carry >>= BIGDIGIT_SIZE;
    mult_carry >>= BIGDIGIT_SIZE;
    for (pos = 1; pos < big1->size; pos++) {
      negate_carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
      mult_carry += (negate_carry & BIGDIGIT_MASK) * bigDigit;
      result->bigdigits[pos] = (bigdigittype) (mult_carry & BIGDIGIT_MASK);
      negate_carry >>= BIGDIGIT_SIZE;
      mult_carry >>= BIGDIGIT_SIZE;
    } /* for */
    result->bigdigits[pos] = (bigdigittype) (mult_carry & BIGDIGIT_MASK);
  } /* uBigMultNegativeWithNegatedDigit */



#ifdef ANSI_C

static void uBigMult (const_biginttype big1, const_biginttype big2,
    const biginttype result)
#else

static void uBigMult (big1, big2, result)
biginttype big1;
biginttype big2;
biginttype result;
#endif

  {
    const_biginttype help_big;
    memsizetype pos1;
    memsizetype pos2;
    doublebigdigittype carry;
    doublebigdigittype carry2 = 0;
    doublebigdigittype prod;

  /* uBigMult */
    if (big2->size > big1->size) {
      help_big = big1;
      big1 = big2;
      big2 = help_big;
    } /* if */
    carry = (doublebigdigittype) big1->bigdigits[0] * big2->bigdigits[0];
    result->bigdigits[0] = (bigdigittype) (carry & BIGDIGIT_MASK);
    carry >>= BIGDIGIT_SIZE;
    if (big2->size == 1) {
      for (pos1 = 1; pos1 < big1->size; pos1++) {
        carry += (doublebigdigittype) big1->bigdigits[pos1] * big2->bigdigits[0];
        result->bigdigits[pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
      } /* for */
    } else {
      for (pos1 = 1; pos1 < big2->size; pos1++) {
        pos2 = 0;
        do {
          prod = (doublebigdigittype) big1->bigdigits[pos2] * big2->bigdigits[pos1 - pos2];
          carry2 += carry > (doublebigdigittype) ~prod ? 1 : 0;
          carry += prod;
          pos2++;
        } while (pos2 <= pos1);
        result->bigdigits[pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        carry |= (carry2 & BIGDIGIT_MASK) << BIGDIGIT_SIZE;
        carry2 >>= BIGDIGIT_SIZE;
      } /* for */
      for (; pos1 < big1->size; pos1++) {
        pos2 = pos1 - big2->size + 1;
        do {
          prod = (doublebigdigittype) big1->bigdigits[pos2] * big2->bigdigits[pos1 - pos2];
          carry2 += carry > (doublebigdigittype) ~prod ? 1 : 0;
          carry += prod;
          pos2++;
        } while (pos2 <= pos1);
        result->bigdigits[pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        carry |= (carry2 & BIGDIGIT_MASK) << BIGDIGIT_SIZE;
        carry2 >>= BIGDIGIT_SIZE;
      } /* for */
      for (; pos1 < big1->size + big2->size - 1; pos1++) {
        pos2 = pos1 - big2->size + 1;
        do {
          prod = (doublebigdigittype) big1->bigdigits[pos2] * big2->bigdigits[pos1 - pos2];
          carry2 += carry > (doublebigdigittype) ~prod ? 1 : 0;
          carry += prod;
          pos2++;
        } while (pos2 < big1->size);
        result->bigdigits[pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        carry |= (carry2 & BIGDIGIT_MASK) << BIGDIGIT_SIZE;
        carry2 >>= BIGDIGIT_SIZE;
      } /* for */
    } /* if */
    result->bigdigits[pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
  } /* uBigMult */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

static void uBigMult (const const_biginttype big1, const const_biginttype big2,
    const biginttype result)
#else

static void uBigMult (big1, big2, result)
biginttype big1;
biginttype big2;
biginttype result;
#endif

  {
    memsizetype pos1;
    memsizetype pos2;
    doublebigdigittype carry;
    doublebigdigittype carry2 = 0;
    doublebigdigittype prod;

  /* uBigMult */
    carry = (doublebigdigittype) big1->bigdigits[0] * big2->bigdigits[0];
    result->bigdigits[0] = (bigdigittype) (carry & BIGDIGIT_MASK);
    carry >>= BIGDIGIT_SIZE;
    for (pos1 = 1; pos1 < big1->size + big2->size - 1; pos1++) {
      if (pos1 < big2->size) {
        pos2 = 0;
      } else {
        pos2 = pos1 - big2->size + 1;
      } /* if */
      if (pos1 < big1->size) {
        pos3 = pos1 + 1;
      } else {
        pos3 = big1->size;
      } /* if */
      do {
        prod = (doublebigdigittype) big1->bigdigits[pos2] * big2->bigdigits[pos1 - pos2];
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
      result->bigdigits[pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      carry |= (carry2 & BIGDIGIT_MASK) << BIGDIGIT_SIZE;
      carry2 >>= BIGDIGIT_SIZE;
    } /* for */
    result->bigdigits[pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
  } /* uBigMult */
#endif



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

static void uBigMult (const const_biginttype big1, const const_biginttype big2,
    const biginttype result)
#else

static void uBigMult (big1, big2, result)
biginttype big1;
biginttype big2;
biginttype result;
#endif

  {
    memsizetype pos1;
    memsizetype pos2;
    doublebigdigittype carry = 0;

  /* uBigMult */
    pos2 = 0;
    do {
      carry += (doublebigdigittype) big1->bigdigits[0] * big2->bigdigits[pos2];
      result->bigdigits[pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      pos2++;
    } while (pos2 < big2->size);
    result->bigdigits[big2->size] = (bigdigittype) (carry & BIGDIGIT_MASK);
    for (pos1 = 1; pos1 < big1->size; pos1++) {
      carry = 0;
      pos2 = 0;
      do {
        carry += (doublebigdigittype) result->bigdigits[pos1 + pos2] +
            (doublebigdigittype) big1->bigdigits[pos1] * big2->bigdigits[pos2];
        result->bigdigits[pos1 + pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos2++;
      } while (pos2 < big2->size);
      result->bigdigits[pos1 + big2->size] = (bigdigittype) (carry & BIGDIGIT_MASK);
    } /* for */
  } /* uBigMult */
#endif



#ifdef ANSI_C

static biginttype uBigMultK (const_biginttype big1, const_biginttype big2,
    const booltype negative)
#else

static biginttype uBigMultK (big1, big2, negative)
biginttype big1;
biginttype big2;
booltype negative;
#endif

  {
    const_biginttype help_big;
    biginttype big2_help;
    biginttype temp;
    biginttype result;

  /* uBigMultK */
    if (big1->size >= KARATSUBA_THRESHOLD && big2->size >= KARATSUBA_THRESHOLD) {
      if (big2->size > big1->size) {
        help_big = big1;
        big1 = big2;
        big2 = help_big;
      } /* if */
      if (big2->size << 1 <= big1->size) {
        if (unlikely(!ALLOC_BIG_SIZE_OK(big2_help, big1->size - (big1->size >> 1)))) {
          raise_error(MEMORY_ERROR);
          return NULL;
        } else {
          big2_help->size = big1->size - (big1->size >> 1);
          memcpy(big2_help->bigdigits, big2->bigdigits,
              (size_t) big2->size * sizeof(bigdigittype));
          memset(&big2_help->bigdigits[big2->size], 0,
              (size_t) (big2_help->size - big2->size) * sizeof(bigdigittype));
          big2 = big2_help;
          if (unlikely(!ALLOC_BIG(result, (big1->size >> 1) + (big2->size << 1)))) {
            raise_error(MEMORY_ERROR);
          } else {
            result->size = (big1->size >> 1) + (big2->size << 1);
            if (unlikely(!ALLOC_BIG(temp, big1->size << 2))) {
              raise_error(MEMORY_ERROR);
            } else {
              uBigKaratsubaMult(big1->bigdigits, big2->bigdigits,
                  big1->size >> 1, result->bigdigits, temp->bigdigits);
              uBigKaratsubaMult(&big1->bigdigits[big1->size >> 1], big2->bigdigits,
                  big2->size, temp->bigdigits, &temp->bigdigits[big2->size << 1]);
              memset(&result->bigdigits[(big1->size >> 1) << 1], 0,
                  (size_t) (result->size - ((big1->size >> 1) << 1)) * sizeof(bigdigittype));
              uBigDigitAddTo(&result->bigdigits[big1->size >> 1], result->size - (big1->size >> 1),
                  temp->bigdigits, big2->size << 1);
              if (negative) {
                negate_positive_big(result);
              } /* if */
              result = normalize(result);
              FREE_BIG(temp, big1->size << 2);
            } /* if */
          } /* if */
          FREE_BIG(big2_help, big1->size - (big1->size >> 1));
        } /* if */
      } else {
        if (unlikely(!ALLOC_BIG_SIZE_OK(big2_help, big1->size))) {
          raise_error(MEMORY_ERROR);
          return NULL;
        } else {
          big2_help->size = big1->size;
          memcpy(big2_help->bigdigits, big2->bigdigits,
              (size_t) big2->size * sizeof(bigdigittype));
          memset(&big2_help->bigdigits[big2->size], 0,
              (size_t) (big2_help->size - big2->size) * sizeof(bigdigittype));
          big2 = big2_help;
          if (unlikely(!ALLOC_BIG(result, big1->size << 1))) {
            raise_error(MEMORY_ERROR);
          } else {
            if (unlikely(!ALLOC_BIG(temp, big1->size << 2))) {
              raise_error(MEMORY_ERROR);
            } else {
              uBigKaratsubaMult(big1->bigdigits, big2->bigdigits,
                  big1->size, result->bigdigits, temp->bigdigits);
              result->size = big1->size << 1;
              if (negative) {
                negate_positive_big(result);
              } /* if */
              result = normalize(result);
              FREE_BIG(temp, big1->size << 2);
            } /* if */
          } /* if */
          FREE_BIG(big2_help, big1->size);
        } /* if */
      } /* if */
    } else {
      if (unlikely(!ALLOC_BIG(result, big1->size + big2->size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = big1->size + big2->size;
        uBigMult(big1, big2, result);
        if (negative) {
          negate_positive_big(result);
        } /* if */
        result = normalize(result);
      } /* if */
    } /* if */
    return result;
  } /* uBigMultK */



#ifdef ANSI_C

static biginttype uBigSquareK (const_biginttype big1)
#else

static biginttype uBigSquareK (big1)
biginttype big1;
#endif

  {
    biginttype temp;
    biginttype result;

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
 *  Returns the product of big1 by big2 for nonnegative big integers.
 *  The result is written into big_help (which is a scratch variable
 *  and is assumed to contain enough memory). Before returning the
 *  result the variable big1 is assigned to big_help. This way it is
 *  possible to write number = uBigMultIntoHelp(number, base, &big_help).
 *  Note that the old number is in the scratch variable big_help
 *  afterwards.
 */
#ifdef ANSI_C

static biginttype uBigMultIntoHelp (const biginttype big1,
    const const_biginttype big2, biginttype *const big_help)
#else

static biginttype uBigMultIntoHelp (big1, big2, big_help)
biginttype big1;
biginttype big2;
biginttype *big_help;
#endif

  {
    memsizetype pos1;
    bigdigittype digit;
    biginttype result;

  /* uBigMultIntoHelp */
    /* printf("uBigMultIntoHelp(big1->size=%lu, big2->size=%lu)\n", big1->size, big2->size); */
    result = *big_help;
    uBigMult(big1, big2, result);
    pos1 = big1->size + big2->size;
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
    /* printf("uBigMultIntoHelp(big1->size=%lu, big2->size=%lu) => result->size=%lu\n",
           big1->size, big2->size, result->size);
       prot_bigint(result);
       printf("\n"); */
    return result;
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
 *  Unfortunately one bit more than sizeof(doublebigdigittype) is
 *  needed to store the shifted product. Therefore extra effort is
 *  necessary to avoid an overflow.
 */
#ifdef ANSI_C

static biginttype uBigSquare (const biginttype big1, biginttype *big_help)
#else

static biginttype uBigSquare (big1, big_help)
biginttype big1;
biginttype *big_help;
#endif

  {
    memsizetype pos1;
    memsizetype pos2;
    doublebigdigittype carry;
    doublebigdigittype product;
    bigdigittype digit;
    biginttype result;

  /* uBigSquare */
    /* printf("uBigSquare(big1->size=%lu)\n", big1->size); */
    result = *big_help;
    digit = big1->bigdigits[0];
    carry = (doublebigdigittype) digit * digit;
    result->bigdigits[0] = (bigdigittype) (carry & BIGDIGIT_MASK);
    carry >>= BIGDIGIT_SIZE;
    for (pos2 = 1; pos2 < big1->size; pos2++) {
      product = (doublebigdigittype) digit * big1->bigdigits[pos2];
      carry += (product << 1) & BIGDIGIT_MASK;
      result->bigdigits[pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      carry += product >> (BIGDIGIT_SIZE - 1);
    } /* for */
    result->bigdigits[pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
    for (pos1 = 1; pos1 < big1->size; pos1++) {
      digit = big1->bigdigits[pos1];
      carry = (doublebigdigittype) result->bigdigits[pos1 + pos1] +
          (doublebigdigittype) digit * digit;
      result->bigdigits[pos1 + pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= BIGDIGIT_SIZE;
      for (pos2 = pos1 + 1; pos2 < big1->size; pos2++) {
        product = (doublebigdigittype) digit * big1->bigdigits[pos2];
        carry += (doublebigdigittype) result->bigdigits[pos1 + pos2] +
            ((product << 1) & BIGDIGIT_MASK);
        result->bigdigits[pos1 + pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        carry += product >> (BIGDIGIT_SIZE - 1);
      } /* for */
      result->bigdigits[pos1 + pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
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
#ifdef ANSI_C

static biginttype bigIPowN (const bigdigittype base, inttype exponent, unsigned int bit_size)
#else

static biginttype bigIPowN (base, exponent, bit_size)
bigdigittype base;
inttype exponent;
unsigned int bit_size;
#endif

  {
    memsizetype help_size;
    biginttype square;
    biginttype big_help;
    biginttype result;

  /* bigIPowN */
    /* printf("bigIPowN(%lu, %lu, %u)\n", base, exponent, bit_size); */
    /* help_size = (bit_size * ((uinttype) exponent) - 1) / BIGDIGIT_SIZE + 2; */
    /* printf("help_sizeA=%ld\n", help_size); */
    if (unlikely((uinttype) exponent + 1 > MAX_BIG_LEN)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      help_size = (memsizetype) ((uinttype) exponent + 1);
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
              (size_t) square->size * sizeof(bigdigittype));
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
            (size_t) (help_size - result->size) * sizeof(bigdigittype));
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
#ifdef ANSI_C

static biginttype bigIPow1 (bigdigittype base, inttype exponent)
#else

static biginttype bigIPow1 (base, exponent)
bigdigittype base;
inttype exponent;
#endif

  {
    booltype negative;
    unsigned int bit_size;
    biginttype result;

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
        negative = exponent & 1;
      } else {
        negative = FALSE;
      } /* if */
      bit_size = (unsigned int) (digitMostSignificantBit(base) + 1);
      if (base == (bigdigittype) (1 << (bit_size - 1))) {
        result = bigLShiftOne((inttype) (bit_size - 1) * exponent);
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



#ifdef ANSI_C

static int uBigIsNot0 (const const_biginttype big)
#else

static int uBigIsNot0 (big)
biginttype big;
#endif

  {
    memsizetype pos;

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
 *  Returns the absolute value of a signed big integer.
 */
#ifdef ANSI_C

biginttype bigAbs (const const_biginttype big1)
#else

biginttype bigAbs (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;
    memsizetype result_size;
    biginttype resized_result;
    biginttype result;

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
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
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
            (size_t) big1->size * sizeof(bigdigittype));
      } /* if */
    } /* if */
    return result;
  } /* bigAbs */



/**
 *  Returns the sum of two signed big integers.
 *  The function sorts the two values by size. This way there is a
 *  loop up to the shorter size and a second loop up to the longer
 *  size.
 */
#ifdef ANSI_C

biginttype bigAdd (const_biginttype summand1, const_biginttype summand2)
#else

biginttype bigAdd (summand1, summand2)
biginttype summand1;
biginttype summand2;
#endif

  {
    const_biginttype help_big;
    memsizetype pos;
    doublebigdigittype carry = 0;
    doublebigdigittype summand2_sign;
    biginttype result;

  /* bigAdd */
    if (summand2->size > summand1->size) {
      help_big = summand1;
      summand1 = summand2;
      summand2 = help_big;
    } /* if */
    if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, summand1->size + 1))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      pos = 0;
      do {
        carry += (doublebigdigittype) summand1->bigdigits[pos] + summand2->bigdigits[pos];
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < summand2->size);
      summand2_sign = IS_NEGATIVE(summand2->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
      for (; pos < summand1->size; pos++) {
        carry += summand1->bigdigits[pos] + summand2_sign;
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
      } /* for */
      if (IS_NEGATIVE(summand1->bigdigits[pos - 1])) {
        summand2_sign--;
      } /* if */
      result->bigdigits[pos] = (bigdigittype) ((carry + summand2_sign) & BIGDIGIT_MASK);
      result->size = pos + 1;
      result = normalize(result);
      return result;
    } /* if */
  } /* bigAdd */



/**
 *  Returns the sum of two signed big integers.
 *  Big1 is assumed to be a temporary value which is reused.
 */
#ifdef ANSI_C

biginttype bigAddTemp (biginttype big1, const const_biginttype big2)
#else

biginttype bigAddTemp (big1, big2)
biginttype big1;
biginttype big2;
#endif

  { /* bigAddTemp */
    bigGrow(&big1, big2);
    return big1;
  } /* bigAddTemp */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

biginttype bigBinom (biginttype n_number, biginttype k_number)
#else

biginttype bigBinom (n_number, k_number)
biginttype n_number;
biginttype k_number;
#endif

  {
    biginttype number;
    biginttype result;

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
    return (inttype) result;
  } /* bigBinom */
#endif



/**
 *  Number of bits in the minimal two's-complement representation.
 *  The high bits equivalent to the sign bit are not part of the
 *  minimal two's-complement representation.
 *  @return the number of bits.
 *  @exception RANGE_ERROR The result does not fit into an integer.
 */
#ifdef ANSI_C

inttype bigBitLength (const const_biginttype big1)
#else

inttype bigBitLength (big1)
biginttype big1;
#endif

  {
    inttype result;

  /* bigBitLength */
    if (unlikely(big1->size >= MAX_MEM_INDEX >> BIGDIGIT_LOG2_SIZE)) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = (inttype) ((big1->size - 1) << BIGDIGIT_LOG2_SIZE);
      if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        result += digitMostSignificantBit(~big1->bigdigits[big1->size - 1]) + 1;
      } else {
        result += digitMostSignificantBit(big1->bigdigits[big1->size - 1]) + 1;
      } /* if */
    } /* if */
    return result;
  } /* bigBitLength */



#ifdef ANSI_C

stritype bigCLit (const const_biginttype big1)
#else

stritype bigCLit (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    int byteNum;
    static const char hex_digit[] = "0123456789ABCDEF";
    char byteBuffer[22];
    bigdigittype digit;
    memsizetype byteDigitCount;
    memsizetype charIndex;
    memsizetype bufferDigitCount;
    memsizetype result_size;
    stritype result;

  /* bigCLit */
    /* The expression computing byteDigitCount does not overflow        */
    /* because the number of bytes in a bigInteger fits in memsizetype. */
    byteDigitCount = big1->size * (BIGDIGIT_SIZE >> 3);
    digit = big1->bigdigits[big1->size - 1];
    byteNum = (BIGDIGIT_SIZE >> 3) - 1;
    if (IS_NEGATIVE(digit)) {
      while (byteNum > 0 && (digit >> byteNum * 8 & 0xFF) == 0xFF) {
        byteDigitCount--;
        byteNum--;
      } /* while */
      if (byteNum < 3 && (digit >> byteNum * 8 & 0xFF) <= 127) {
        byteDigitCount++;
        /* Not used afterwards: byteNum++; */
      } /* if */
    } else {
      while (byteNum > 0 && (digit >> byteNum * 8 & 0xFF) == 0) {
        byteDigitCount--;
        byteNum--;
      } /* while */
      if (byteNum < 3 && (digit >> byteNum * 8 & 0xFF) >= 128) {
        byteDigitCount++;
        /* Not used afterwards: byteNum++; */
      } /* if */
    } /* if */
    if (unlikely(byteDigitCount > (MAX_STRI_LEN - 21) / 5 ||
                 byteDigitCount > 0xFFFFFFFF ||
                 (result_size = byteDigitCount * 5 + 21,
                 !ALLOC_STRI_SIZE_OK(result, result_size)))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result->size = result_size;
      sprintf(byteBuffer, "{0x%02X,0x%02X,0x%02X,0x%02X,",
          (unsigned int) (byteDigitCount >> 24 & 0xFF),
          (unsigned int) (byteDigitCount >> 16 & 0xFF),
          (unsigned int) (byteDigitCount >>  8 & 0xFF),
          (unsigned int) (byteDigitCount       & 0xFF));
      cstri_expand(result->mem, byteBuffer, 21);
      charIndex = 21;
      pos = big1->size;
      while (pos > 0) {
        pos--;
        digit = big1->bigdigits[pos];
#if BIGDIGIT_SIZE == 8
        memcpy(byteBuffer, "0x00,", 5);
        byteBuffer[2]  = hex_digit[digit >>  4 & 0xF];
        byteBuffer[3]  = hex_digit[digit       & 0xF];
#elif BIGDIGIT_SIZE == 16
        memcpy(byteBuffer, "0x00,0x00,", 10);
        byteBuffer[2]  = hex_digit[digit >> 12 & 0xF];
        byteBuffer[3]  = hex_digit[digit >>  8 & 0xF];
        byteBuffer[7]  = hex_digit[digit >>  4 & 0xF];
        byteBuffer[8]  = hex_digit[digit       & 0xF];
#elif BIGDIGIT_SIZE == 32
        memcpy(byteBuffer, "0x00,0x00,0x00,0x00,", 20);
        byteBuffer[2]  = hex_digit[digit >> 28 & 0xF];
        byteBuffer[3]  = hex_digit[digit >> 24 & 0xF];
        byteBuffer[7]  = hex_digit[digit >> 20 & 0xF];
        byteBuffer[8]  = hex_digit[digit >> 16 & 0xF];
        byteBuffer[12] = hex_digit[digit >> 12 & 0xF];
        byteBuffer[13] = hex_digit[digit >>  8 & 0xF];
        byteBuffer[17] = hex_digit[digit >>  4 & 0xF];
        byteBuffer[18] = hex_digit[digit       & 0xF];
#endif
        if ((pos + 1) * (BIGDIGIT_SIZE >> 3) <= byteDigitCount) {
          cstri_expand(&result->mem[charIndex], byteBuffer, 5 * (BIGDIGIT_SIZE >> 3));
          charIndex += 5 * (BIGDIGIT_SIZE >> 3);
        } else {
          bufferDigitCount = byteDigitCount - pos * (BIGDIGIT_SIZE >> 3);
          cstri_expand(&result->mem[charIndex],
              &byteBuffer[5 * ((BIGDIGIT_SIZE >> 3) - bufferDigitCount)],
              5 * bufferDigitCount);
          charIndex += 5 * bufferDigitCount;
        } /* if */
      } /* while */
      charIndex -= 5;
      result->mem[charIndex + 4] = '}';
    } /* if */
    return result;
  } /* bigCLit */



#ifdef ANSI_C

inttype bigCmp (const const_biginttype big1, const const_biginttype big2)
#else

inttype bigCmp (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    bigdigittype big1_negative;
    bigdigittype big2_negative;
    memsizetype pos;

  /* bigCmp */
    big1_negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    big2_negative = IS_NEGATIVE(big2->bigdigits[big2->size - 1]);
    if (big1_negative != big2_negative) {
      return big1_negative ? -1 : 1;
    } else if (big1->size != big2->size) {
      return (big1->size < big2->size) != (big1_negative != 0) ? -1 : 1;
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
 *  Reinterpret the generic parameters as biginttype and call bigCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(rtlGenerictype) != sizeof(biginttype).
 */
#ifdef ANSI_C

inttype bigCmpGeneric (const rtlGenerictype value1, const rtlGenerictype value2)
#else

inttype bigCmpGeneric (value1, value2)
rtlGenerictype value1;
rtlGenerictype value2;
#endif

  { /* bigCmpGeneric */
    return bigCmp((const_biginttype) value1, (const_biginttype) value2);
  } /* bigCmpGeneric */



#ifdef ANSI_C

inttype bigCmpSignedDigit (const const_biginttype big1, inttype number)
#else

inttype bigCmpSignedDigit (big1, number)
biginttype big1;
inttype number;
#endif

  {
    inttype result;

  /* bigCmpSignedDigit */
    if (number < 0) {
      if (!IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        result = 1;
      } else if (big1->size != 1) {
        result = -1;
      } else if (big1->bigdigits[0] != (bigdigittype) number) {
        if (big1->bigdigits[0] < (bigdigittype) number) {
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
      } else if (big1->bigdigits[0] != (bigdigittype) number) {
        if (big1->bigdigits[0] < (bigdigittype) number) {
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



#ifdef ANSI_C

void bigCpy (biginttype *const big_to, const const_biginttype big_from)
#else

void bigCpy (big_to, big_from)
biginttype *big_to;
biginttype big_from;
#endif

  {
    memsizetype new_size;
    biginttype big_dest;

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
        (size_t) new_size * sizeof(bigdigittype));
  } /* bigCpy */



#ifdef ANSI_C

biginttype bigCreate (const const_biginttype big_from)
#else

biginttype bigCreate (big_from)
biginttype big_from;
#endif

  {
    memsizetype new_size;
    biginttype result;

  /* bigCreate */
    new_size = big_from->size;
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, new_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = new_size;
      memcpy(result->bigdigits, big_from->bigdigits,
          (size_t) new_size * sizeof(bigdigittype));
    } /* if */
    return result;
  } /* bigCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(rtlGenerictype) != sizeof(biginttype).
 */
#ifdef ANSI_C

rtlGenerictype bigCreateGeneric (const rtlGenerictype from_value)
#else

rtlGenerictype bigCreateGeneric (from_value)
rtlGenerictype from_value;
#endif

  { /* bigCreateGeneric */
    return (rtlGenerictype) (memsizetype) bigCreate((const_biginttype) from_value);
  } /* bigCreateGeneric */



/**
 *  Decrements *big_variable by 1. The operation is done in
 *  place and *big_variable is only enlarged when necessary.
 *  In case the enlarging fails the old content of *big_variable
 *  is restored and the exception MEMORY_ERROR is raised.
 *  This ensures that bigDecr works as a transaction.
 */
#ifdef ANSI_C

void bigDecr (biginttype *const big_variable)
#else

void bigDecr (big_variable)
biginttype *big_variable;
#endif

  {
    biginttype big1;
    memsizetype pos;
    bigdigittype negative;
    biginttype resized_big1;

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
        /* memset(big1->bigdigits, 0xFF, pos * sizeof(bigdigittype)); */
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



#ifdef ANSI_C

void bigDestr (const const_biginttype old_bigint)
#else

void bigDestr (old_bigint)
biginttype old_bigint;
#endif

  { /* bigDestr */
    if (old_bigint != NULL) {
      FREE_BIG(old_bigint, old_bigint->size);
    } /* if */
  } /* bigDestr */



/**
 *  Compute a signed big integer division (dividend / divisor).
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
 */
#ifdef ANSI_C

biginttype bigDiv (const const_biginttype dividend, const const_biginttype divisor)
#else

biginttype bigDiv (dividend, divisor)
biginttype dividend;
biginttype divisor;
#endif

  {
    booltype negative = FALSE;
    biginttype dividend_help;
    biginttype divisor_help;
    unsigned int shift;
    biginttype result;

  /* bigDiv */
    if (divisor->size == 1) {
      result = bigDiv1(dividend, divisor->bigdigits[0]);
      return result;
    } else if (dividend->size < divisor->size) {
      result = bigDivSizeLess(dividend, divisor);
      return result;
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
              (size_t) dividend->size * sizeof(bigdigittype));
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
              (size_t) divisor->size * sizeof(bigdigittype));
        } /* if */
      } /* if */
      if (unlikely(!ALLOC_BIG_SIZE_OK(result, dividend_help->size - divisor_help->size + 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = dividend_help->size - divisor_help->size + 1;
        result->bigdigits[result->size - 1] = 0;
        shift = (unsigned int) (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
        if (shift == 0) {
          /* The most significant digit of divisor_help is 0. Just ignore it */
          dividend_help->size--;
          divisor_help->size--;
          if (divisor_help->size == 1) {
            uBigDiv1(dividend_help, divisor_help->bigdigits[0], result);
          } else {
            uBigDiv(dividend_help, divisor_help, result);
          } /* if */
        } else {
          shift = BIGDIGIT_SIZE - shift;
          uBigLShift(dividend_help, shift);
          uBigLShift(divisor_help, shift);
          uBigDiv(dividend_help, divisor_help, result);
        } /* if */
        if (negative) {
          negate_positive_big(result);
        } /* if */
        result = normalize(result);
      } /* if */
      FREE_BIG(dividend_help, dividend->size + 2);
      FREE_BIG(divisor_help, divisor->size + 1);
      return result;
    } /* if */
  } /* bigDiv */



#ifdef ANSI_C

booltype bigEq (const const_biginttype big1, const const_biginttype big2)
#else

booltype bigEq (big1, big2)
biginttype big1;
biginttype big2;
#endif

  { /* bigEq */
    if (big1->size == big2->size &&
      memcmp(big1->bigdigits, big2->bigdigits,
          (size_t) big1->size * sizeof(bigdigittype)) == 0) {
      return TRUE;
    } else {
      return FALSE;
    } /* if */
  } /* bigEq */



#ifdef ANSI_C

booltype bigEqSignedDigit (const const_biginttype big1, inttype number)
#else

booltype bigEqSignedDigit (big1, number)
biginttype big1;
inttype number;
#endif

  { /* bigEqSignedDigit */
    return big1->size == 1 && big1->bigdigits[0] == (bigdigittype) number;
  } /* bigEqSignedDigit */



#ifdef ANSI_C

biginttype bigFromInt32 (int32type number)
#else

biginttype bigFromInt32 (number)
int32type number;
#endif

  {
    memsizetype result_size;
    biginttype result;

  /* bigFromInt32 */
#if BIGDIGIT_SIZE < 32
    result_size = sizeof(int32type) / (BIGDIGIT_SIZE >> 3);
#else
    result_size = 1;
#endif
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
#if BIGDIGIT_SIZE <= 32
      result->bigdigits[0] = (bigdigittype) (((uint32type) number) & BIGDIGIT_MASK);
#if BIGDIGIT_SIZE < 32
      {
        memsizetype pos;

        for (pos = 1; pos < result_size; pos++) {
          number >>= BIGDIGIT_SIZE;
          result->bigdigits[pos] = (bigdigittype) (((uint32type) number) & BIGDIGIT_MASK);
        } /* for */
      }
#endif
#else
      if (number < 0) {
        result->bigdigits[0] = (bigdigittype) ((uint32type) number) | (BIGDIGIT_MASK ^ 0xFFFFFFFF);
      } else {
        result->bigdigits[0] = (bigdigittype) ((uint32type) number);
      } /* if */
#endif
#if BIGDIGIT_SIZE < 32
      result = normalize(result);
#endif
    } /* if */
    return result;
  } /* bigFromInt32 */



#ifdef INT64TYPE
#ifdef ANSI_C

biginttype bigFromInt64 (int64type number)
#else

biginttype bigFromInt64 (number)
int64type number;
#endif

  {
    memsizetype pos;
    memsizetype result_size;
    biginttype result;

  /* bigFromInt64 */
    result_size = sizeof(int64type) / (BIGDIGIT_SIZE >> 3);
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = result_size;
      for (pos = 0; pos < result_size; pos++) {
        result->bigdigits[pos] = (bigdigittype) (number & BIGDIGIT_MASK);
        number >>= BIGDIGIT_SIZE;
      } /* for */
      result = normalize(result);
      return result;
    } /* if */
  } /* bigFromInt64 */
#endif



#ifdef ANSI_C

biginttype bigFromUInt32 (uint32type number)
#else

biginttype bigFromUInt32 (number)
uint32type number;
#endif

  {
    memsizetype result_size;
    biginttype result;

  /* bigFromUInt32 */
#if BIGDIGIT_SIZE <= 32
    result_size = sizeof(uint32type) / (BIGDIGIT_SIZE >> 3) + 1;
#else
    result_size = 1;
#endif
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
      result->bigdigits[0] = (bigdigittype) (number & BIGDIGIT_MASK);
#if BIGDIGIT_SIZE < 32
      {
        memsizetype pos;

        for (pos = 1; pos < result_size - 1; pos++) {
          number >>= BIGDIGIT_SIZE;
          result->bigdigits[pos] = (bigdigittype) (number & BIGDIGIT_MASK);
        } /* for */
      }
#endif
      result->bigdigits[result_size - 1] = (bigdigittype) 0;
      result = normalize(result);
    } /* if */
    return result;
  } /* bigFromUInt32 */



#ifdef INT64TYPE
#ifdef ANSI_C

biginttype bigFromUInt64 (uint64type number)
#else

biginttype bigFromUInt64 (number)
uint64type number;
#endif

  {
    memsizetype pos;
    memsizetype result_size;
    biginttype result;

  /* bigFromUInt64 */
    result_size = sizeof(uint64type) / (BIGDIGIT_SIZE >> 3) + 1;
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = result_size;
      for (pos = 0; pos < result_size - 1; pos++) {
        result->bigdigits[pos] = (bigdigittype) (number & BIGDIGIT_MASK);
        number >>= BIGDIGIT_SIZE;
      } /* for */
      result->bigdigits[result_size - 1] = (bigdigittype) 0;
      result = normalize(result);
      return result;
    } /* if */
  } /* bigFromUInt64 */
#endif



#ifdef ANSI_C

biginttype bigGcd (const const_biginttype big1,
    const const_biginttype big2)
#else

biginttype bigGcd (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    biginttype big1_help;
    biginttype big2_help;
    inttype lowestSetBitA;
    inttype shift;
    biginttype help_big;
    biginttype result;

  /* bigGcd */
    if (big1->size == 1 && big1->bigdigits[0] == 0) {
      result = bigCreate(big2);
    } else if (big2->size == 1 && big2->bigdigits[0] == 0) {
      result = bigCreate(big1);
    } else {
      if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        big1_help = bigMinus(big1);
      } else {
        big1_help = bigCreate(big1);
      } /* if */
      if (IS_NEGATIVE(big2->bigdigits[big2->size - 1])) {
        big2_help = bigMinus(big2);
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
 *  Adds big2 to *big_variable. The operation is done in
 *  place and *big_variable is only resized when necessary.
 *  When the size of big2 is smaller than *big_variable the
 *  algorithm tries to save computations. Therefore there are
 *  checks for carry == 0 and carry != 0.
 *  In case the resizing fails the content of *big_variable
 *  is freed and *big_variable is set to NULL.
 */
#ifdef ANSI_C

void bigGrow (biginttype *const big_variable, const const_biginttype big2)
#else

void bigGrow (big_variable, big2)
biginttype *big_variable;
biginttype big2;
#endif

  {
    biginttype big1;
    memsizetype pos;
    memsizetype big1_size;
    doublebigdigittype carry = 0;
    doublebigdigittype big1_sign;
    doublebigdigittype big2_sign;
    biginttype resized_big1;

  /* bigGrow */
    big1 = *big_variable;
    if (big1->size >= big2->size) {
      big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
      pos = 0;
      do {
        carry += (doublebigdigittype) big1->bigdigits[pos] + big2->bigdigits[pos];
        big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < big2->size);
      if (IS_NEGATIVE(big2->bigdigits[pos - 1])) {
        for (; carry == 0 && pos < big1->size; pos++) {
          carry = (doublebigdigittype) big1->bigdigits[pos] + BIGDIGIT_MASK;
          big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        carry += BIGDIGIT_MASK;
      } else {
        for (; carry != 0 && pos < big1->size; pos++) {
          carry += big1->bigdigits[pos];
          big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
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
          /* It is possible that big1 == big2 holds. Since */
          /* 'big2' is not used after realloc() enlarged   */
          /* 'big1' a correction of big2 is not necessary. */
          big1 = resized_big1;
          COUNT3_BIG(big1_size, big1_size + 1);
          big1->size++;
          big1->bigdigits[big1_size] = (bigdigittype) (carry & BIGDIGIT_MASK);
          *big_variable = big1;
        } /* if */
      } else {
        *big_variable = normalize(big1);
      } /* if */
    } else {
      REALLOC_BIG_CHECK_SIZE(resized_big1, big1, big1->size, big2->size + 1);
      if (unlikely(resized_big1 == NULL)) {
        FREE_BIG(big1, big1->size);
        *big_variable = NULL;
        raise_error(MEMORY_ERROR);
      } else {
        big1 = resized_big1;
        COUNT3_BIG(big1->size, big2->size + 1);
        big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
        pos = 0;
        do {
          carry += (doublebigdigittype) big1->bigdigits[pos] + big2->bigdigits[pos];
          big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          pos++;
        } while (pos < big1->size);
        big2_sign = IS_NEGATIVE(big2->bigdigits[big2->size - 1]) ? BIGDIGIT_MASK : 0;
        for (; pos < big2->size; pos++) {
          carry += big1_sign + big2->bigdigits[pos];
          big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        carry += big1_sign + big2_sign;
        big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        big1->size = pos + 1;
        *big_variable = normalize(big1);
      } /* if */
    } /* if */
  } /* bigGrow */



#ifdef ANSI_C

inttype bigHashCode (const const_biginttype big1)
#else

inttype bigHashCode (big1)
biginttype big1;
#endif

  {
    inttype result;

  /* bigHashCode */
    result = (inttype) (big1->bigdigits[0] << 5 ^ big1->size << 3 ^ big1->bigdigits[big1->size - 1]);
    return result;
  } /* bigHashCode */



#ifdef ANSI_C

biginttype bigImport (const const_ustritype buffer)
#else

biginttype bigImport (buffer)
ustritype buffer;
#endif

  {
    memsizetype byteDigitCount;
    memsizetype byteIndex;
    memsizetype pos;
    memsizetype result_size;
    uchartype buffer2[BIGDIGIT_SIZE >> 3];
    biginttype result;

  /* bigImport */
    byteDigitCount = ((memsizetype) buffer[0]) << 24 |
                     ((memsizetype) buffer[1]) << 16 |
                     ((memsizetype) buffer[2]) <<  8 |
                     ((memsizetype) buffer[3]);
    result_size = (byteDigitCount + (BIGDIGIT_SIZE >> 3) - 1) / (BIGDIGIT_SIZE >> 3);
    if (unlikely(!ALLOC_BIG(result, result_size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = result_size;
      byteIndex = byteDigitCount;
      for (pos = 0; pos < result_size - 1; pos++) {
#if BIGDIGIT_SIZE == 8
        result->bigdigits[pos] =  (bigdigittype) buffer[byteIndex + 3];
        byteIndex--;
#elif BIGDIGIT_SIZE == 16
        result->bigdigits[pos] = ((bigdigittype) buffer[byteIndex + 2]) <<  8 |
                                  (bigdigittype) buffer[byteIndex + 3];
        byteIndex -= 2;
#elif BIGDIGIT_SIZE == 32
        result->bigdigits[pos] = ((bigdigittype) buffer[byteIndex])     << 24 |
                                 ((bigdigittype) buffer[byteIndex + 1]) << 16 |
                                 ((bigdigittype) buffer[byteIndex + 2]) <<  8 |
                                  (bigdigittype) buffer[byteIndex + 3];
        byteIndex -= 4;
#endif
      } /* for */
      memcpy(&buffer2[4 - byteIndex], &buffer[4], byteIndex);
      if (buffer[4] >= 128) {
        memset(buffer2, 0xFF, 4 - byteIndex);
      } else {
        memset(buffer2, 0, 4 - byteIndex);
      } /* if */
#if BIGDIGIT_SIZE == 8
        result->bigdigits[pos] =  (bigdigittype) buffer2[0];
#elif BIGDIGIT_SIZE == 16
        result->bigdigits[pos] = ((bigdigittype) buffer2[0]) <<  8 |
                                  (bigdigittype) buffer2[1];
#elif BIGDIGIT_SIZE == 32
        result->bigdigits[pos] = ((bigdigittype) buffer2[0]) << 24 |
                                 ((bigdigittype) buffer2[1]) << 16 |
                                 ((bigdigittype) buffer2[2]) <<  8 |
                                  (bigdigittype) buffer2[3];
#endif
    } /* if */
    /* printf("bigImport()-> %s\n", bigHexCStri(result)); */
    return result;
  } /* bigImport */



/**
 *  Increments *big_variable by 1. The operation is done in
 *  place and *big_variable is only enlarged when necessary.
 *  In case the enlarging fails the old content of *big_variable
 *  is restored and the exception MEMORY_ERROR is raised.
 *  This ensures that bigIncr works as a transaction.
 */
#ifdef ANSI_C

void bigIncr (biginttype *const big_variable)
#else

void bigIncr (big_variable)
biginttype *big_variable;
#endif

  {
    biginttype big1;
    memsizetype pos;
    bigdigittype negative;
    biginttype resized_big1;

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
        /* memset(big1->bigdigits, 0, pos * sizeof(bigdigittype)); */
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
 *  Computes base to the power of exponent for signed big integers.
 *  The result variable is set to base or 1 depending on the
 *  rightmost bit of the exponent. After that the base is
 *  squared in a loop and every time the corresponding bit of
 *  the exponent is set the current square is multiplied
 *  with the result variable. This reduces the number of square
 *  operations to ld(exponent).
 */
#ifdef ANSI_C

biginttype bigIPow (const const_biginttype base, inttype exponent)
#else

biginttype bigIPow (base, exponent)
biginttype base;
inttype exponent;
#endif

  {
    booltype negative = FALSE;
    memsizetype help_size;
    biginttype square;
    biginttype big_help;
    biginttype result;

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
    } else if (unlikely((uinttype) exponent + 1 > MAX_BIG_LEN / base->size)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      help_size = base->size * (memsizetype) ((uinttype) exponent + 1);
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
              (size_t) base->size * sizeof(bigdigittype));
        } /* if */
        if (exponent & 1) {
          result->size = square->size;
          memcpy(result->bigdigits, square->bigdigits,
              (size_t) square->size * sizeof(bigdigittype));
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
            (size_t) (help_size - result->size) * sizeof(bigdigittype));
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
 *  Compute the truncated base 2 logarithm of a bigInteger number.
 *  @return the truncated base 2 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
#ifdef ANSI_C

biginttype bigLog2 (const const_biginttype big1)
#else

biginttype bigLog2 (big1)
biginttype big1;
#endif

  {
    memsizetype result_size;
    memsizetype number;
    memsizetype pos;
    inttype bigdigit_log2;
    biginttype result;

  /* bigLog2 */
    if (unlikely(IS_NEGATIVE(big1->bigdigits[big1->size - 1]))) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else {
      /* The result_size is incremented by one to take the space  */
      /* needed for the shift by BIGDIGIT_LOG2_SIZE into account. */
      result_size = sizeof(memsizetype) / (BIGDIGIT_SIZE >> 3) + 1;
      if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        number = big1->size - 1;
        result->bigdigits[0] = (bigdigittype) (number & BIGDIGIT_MASK);
        for (pos = 1; pos < result_size; pos++) {
          /* POINTER_SIZE is equal to sizeof(memsizetype) << 3 */
#if POINTER_SIZE > BIGDIGIT_SIZE
          /* The number does not fit into one bigdigit */
          number >>= BIGDIGIT_SIZE;
          result->bigdigits[pos] = (bigdigittype) (number & BIGDIGIT_MASK);
#else
          /* The number fits into one bigdigit */
          result->bigdigits[pos] = 0;
#endif
        } /* for */
        uBigLShift(result, BIGDIGIT_LOG2_SIZE);
        bigdigit_log2 = digitMostSignificantBit(big1->bigdigits[big1->size - 1]);
        if (bigdigit_log2 == -1) {
          uBigDecr(result);
        } else {
          result->bigdigits[0] |= (bigdigittype) bigdigit_log2;
        } /* if */
        result = normalize(result);
      } /* if */
    } /* if */
    return result;
  } /* bigLog2 */



/**
 *  Index of the lowest-order one bit.
 *  For A <> 0 this is equal to the number of lowest-order zero bits.
 *  @return the number of lowest-order zero bits or -1 for lowestSetBit(0).
 */
#ifdef ANSI_C

inttype bigLowestSetBit (const const_biginttype big1)
#else

inttype bigLowestSetBit (big1)
biginttype big1;
#endif

  {
    memsizetype big1_size;
    memsizetype pos;
    inttype result;

  /* bigLowestSetBit */
    big1_size = big1->size;
    pos = 0;
    while (pos < big1_size && big1->bigdigits[pos] == 0) {
      pos++;
    } /* while */
    if (pos < big1_size) {
      result = digitLeastSignificantBit(big1->bigdigits[pos]);
      if (unlikely(pos > (memsizetype) (MAX_MEM_INDEX - result) >> BIGDIGIT_LOG2_SIZE)) {
        raise_error(RANGE_ERROR);
        result = 0;
      } else {
        result += (inttype) (pos << BIGDIGIT_LOG2_SIZE);
      } /* if */
    } else {
      result = -1;
    } /* if */
    return result;
  } /* bigLowestSetBit */



#ifdef ANSI_C

biginttype bigLShift (const const_biginttype big1, const inttype lshift)
#else

biginttype bigLShift (big1, lshift)
biginttype big1;
inttype lshift;
#endif

  {
    unsigned int digit_rshift;
    unsigned int digit_lshift;
    bigdigittype digit_mask;
    bigdigittype low_digit;
    bigdigittype high_digit;
    const bigdigittype *source_digits;
    bigdigittype *dest_digits;
    memsizetype size_reduction;
    memsizetype pos;
    memsizetype result_size;
    biginttype result;

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
      if (unlikely((uinttype) lshift >> BIGDIGIT_LOG2_SIZE > MAX_BIG_LEN - big1->size)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result_size = big1->size + (memsizetype) ((uinttype) lshift >> BIGDIGIT_LOG2_SIZE);
        if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = result_size;
          memcpy(&result->bigdigits[lshift >> BIGDIGIT_LOG2_SIZE], big1->bigdigits,
              (size_t) big1->size * sizeof(bigdigittype));
          memset(result->bigdigits, 0,
              (memsizetype) ((uinttype) lshift >> BIGDIGIT_LOG2_SIZE) * sizeof(bigdigittype));
        } /* if */
      } /* if */
    } else if (unlikely(((uinttype) lshift >> BIGDIGIT_LOG2_SIZE) + 1 > MAX_BIG_LEN - big1->size)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = big1->size + (memsizetype) ((uinttype) lshift >> BIGDIGIT_LOG2_SIZE) + 1;
      digit_lshift = (uinttype) lshift & BIGDIGIT_SIZE_MASK;
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
              (memsizetype) (dest_digits - result->bigdigits) * sizeof(bigdigittype));
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* bigLShift */



#ifdef ANSI_C

void bigLShiftAssign (biginttype *const big_variable, inttype lshift)
#else

void bigLShiftAssign (big_variable, lshift)
biginttype *const big_variable;
inttype lshift;
#endif

  {
    biginttype big1;
    unsigned int digit_rshift;
    unsigned int digit_lshift;
    bigdigittype digit_mask;
    bigdigittype low_digit;
    bigdigittype high_digit;
    const bigdigittype *source_digits;
    bigdigittype *dest_digits;
    memsizetype size_reduction;
    memsizetype pos;
    memsizetype result_size;
    biginttype result;

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
        if (unlikely((uinttype) lshift >> BIGDIGIT_LOG2_SIZE > MAX_BIG_LEN - big1->size)) {
          raise_error(MEMORY_ERROR);
        } else {
          result_size = big1->size + (memsizetype) ((uinttype) lshift >> BIGDIGIT_LOG2_SIZE);
          if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
            raise_error(MEMORY_ERROR);
          } else {
            result->size = result_size;
            memcpy(&result->bigdigits[lshift >> BIGDIGIT_LOG2_SIZE], big1->bigdigits,
                (size_t) big1->size * sizeof(bigdigittype));
            memset(result->bigdigits, 0,
                (memsizetype) ((uinttype) lshift >> BIGDIGIT_LOG2_SIZE) * sizeof(bigdigittype));
            *big_variable = result;
            FREE_BIG(big1, big1->size);
          } /* if */
        } /* if */
      } else if (unlikely(((uinttype) lshift >> BIGDIGIT_LOG2_SIZE) + 1 > MAX_BIG_LEN - big1->size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result_size = big1->size + (memsizetype) ((uinttype) lshift >> BIGDIGIT_LOG2_SIZE) + 1;
        digit_lshift = (uinttype) lshift & BIGDIGIT_SIZE_MASK;
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
                (memsizetype) (dest_digits - result->bigdigits) * sizeof(bigdigittype));
          } /* if */
          if (result != big1) {
            *big_variable = result;
            FREE_BIG(big1, big1->size);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* bigLShiftAssign */



#ifdef ANSI_C

biginttype bigLShiftOne (const inttype lshift)
#else

biginttype bigLShiftOne (lshift)
inttype lshift;
#endif

  {
    memsizetype result_size;
    int bit_pos;
    biginttype result;

  /* bigLShiftOne */
    if (unlikely(lshift < 0)) {
      if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 1;
        result->bigdigits[0] = 0;
      } /* if */
    } else if (unlikely((((uinttype) lshift + 1) >> BIGDIGIT_LOG2_SIZE) + 1 > MAX_BIG_LEN)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = (memsizetype) (((uinttype) lshift + 1) >> BIGDIGIT_LOG2_SIZE) + 1;
      if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        bit_pos = lshift & BIGDIGIT_SIZE_MASK;
        if (bit_pos == BIGDIGIT_SIZE_MASK) {
          memset(result->bigdigits, 0, (result_size - 2) * sizeof(bigdigittype));
          result->bigdigits[result_size - 2] = ((bigdigittype) 1) << bit_pos;
          result->bigdigits[result_size - 1] = 0;
        } else {
          memset(result->bigdigits, 0, (result_size - 1) * sizeof(bigdigittype));
          result->bigdigits[result_size - 1] = ((bigdigittype) 1) << bit_pos;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* bigLShiftOne */



#ifdef ANSI_C

biginttype bigLog2BaseLShift (const inttype log2base, const inttype lshift)
#else

biginttype bigLog2BaseLShift (log2base, lshift)
inttype log2base;
inttype lshift;
#endif

  {
    uinttype high_shift;
    uinttype low_shift;
    biginttype result;

  /* bigLog2BaseLShift */
    if (unlikely(lshift < 0)) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else if (likely(log2base == 1)) {
      result = bigLShiftOne(lshift);
    } else if (log2base <= 10 && lshift <= 214748364) {
      result = bigLShiftOne(log2base * lshift);
    } else {
      uint2_mult((uinttype) 0L, (uinttype) log2base, (uinttype) 0L, (uinttype) lshift,
          &high_shift, &low_shift);
      if (high_shift != 0 || (inttype) low_shift < 0) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = bigLShiftOne((inttype) low_shift);
      } /* if */
    } /* if */
    return result;
  } /* bigLog2BaseLShift */



/**
 *  Computes an integer modulo division of dividend by divisor for signed
 *  big integers. The memory for the result is requested and the
 *  normalized result is returned. When divisor has just one digit
 *  or when dividend has less digits than divisor the bigMDiv1() or
 *  bigMDivSizeLess() functions are called. In the general case
 *  the absolute values of dividend and divisor are taken. Then dividend is
 *  extended by one leading zero digit. After that dividend and divisor
 *  are shifted to the left such that the most significant bit
 *  of divisor is set. This fulfills the preconditions for calling
 *  uBigDiv() which does the main work of the division.
 */
#ifdef ANSI_C

biginttype bigMDiv (const const_biginttype dividend, const const_biginttype divisor)
#else

biginttype bigMDiv (dividend, divisor)
biginttype dividend;
biginttype divisor;
#endif

  {
    booltype negative = FALSE;
    biginttype dividend_help;
    biginttype divisor_help;
    unsigned int shift;
    bigdigittype mdiv1_remainder = 0;
    biginttype result;

  /* bigMDiv */
    if (divisor->size == 1) {
      result = bigMDiv1(dividend, divisor->bigdigits[0]);
      return result;
    } else if (dividend->size < divisor->size) {
      result = bigMDivSizeLess(dividend, divisor);
      return result;
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
              (size_t) dividend->size * sizeof(bigdigittype));
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
              (size_t) divisor->size * sizeof(bigdigittype));
        } /* if */
      } /* if */
      if (unlikely(!ALLOC_BIG_SIZE_OK(result, dividend_help->size - divisor_help->size + 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = dividend_help->size - divisor_help->size + 1;
        result->bigdigits[result->size - 1] = 0;
        shift = (unsigned int) (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
        if (shift == 0) {
          /* The most significant digit of divisor_help is 0. Just ignore it */
          dividend_help->size--;
          divisor_help->size--;
          if (divisor_help->size == 1) {
            mdiv1_remainder = uBigMDiv1(dividend_help, divisor_help->bigdigits[0], result);
          } else {
            uBigDiv(dividend_help, divisor_help, result);
          } /* if */
        } else {
          shift = BIGDIGIT_SIZE - shift;
          uBigLShift(dividend_help, shift);
          uBigLShift(divisor_help, shift);
          uBigDiv(dividend_help, divisor_help, result);
        } /* if */
        if (negative) {
          if ((divisor_help->size == 1 && mdiv1_remainder != 0) ||
              (divisor_help->size != 1 && uBigIsNot0(dividend_help))) {
            uBigIncr(result);
          } /* if */
          negate_positive_big(result);
        } /* if */
        result = normalize(result);
      } /* if */
      FREE_BIG(dividend_help, dividend->size + 2);
      FREE_BIG(divisor_help, divisor->size + 1);
      return result;
    } /* if */
  } /* bigMDiv */



#ifdef ANSI_C

biginttype bigMinus (const const_biginttype big1)
#else

biginttype bigMinus (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;
    biginttype resized_result;
    biginttype result;

  /* bigMinus */
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, big1->size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = big1->size;
      pos = 0;
      do {
        carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
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
  } /* bigMinus */



/**
 *  Computes the modulo of an integer division of dividend by divisor
 *  for signed big integers. The memory for the result is requested
 *  and the normalized result is returned. When divisor has just one
 *  digit or when dividend has less digits than divisor the bigMod1() or
 *  bigModSizeLess() functions are called. In the general case
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
 */
#ifdef ANSI_C

biginttype bigMod (const const_biginttype dividend, const const_biginttype divisor)
#else

biginttype bigMod (dividend, divisor)
biginttype dividend;
biginttype divisor;
#endif

  {
    booltype negative1 = FALSE;
    booltype negative2 = FALSE;
    biginttype divisor_help;
    unsigned int shift;
    biginttype result;

  /* bigMod */
    if (divisor->size == 1) {
      result = bigMod1(dividend, divisor->bigdigits[0]);
      return result;
    } else if (dividend->size < divisor->size) {
      result = bigModSizeLess(dividend, divisor);
      return result;
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, dividend->size + 2))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          negative1 = TRUE;
          positive_copy_of_negative_big(result, dividend);
        } else {
          result->size = dividend->size;
          memcpy(result->bigdigits, dividend->bigdigits,
              (size_t) dividend->size * sizeof(bigdigittype));
        } /* if */
        result->bigdigits[result->size] = 0;
        result->size++;
      } /* if */
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(divisor_help, divisor->size + 1))) {
        FREE_BIG(result,  dividend->size + 2);
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(divisor->bigdigits[divisor->size - 1])) {
          negative2 = TRUE;
          positive_copy_of_negative_big(divisor_help, divisor);
        } else {
          divisor_help->size = divisor->size;
          memcpy(divisor_help->bigdigits, divisor->bigdigits,
              (size_t) divisor->size * sizeof(bigdigittype));
        } /* if */
      } /* if */
      shift = (unsigned int) (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
      if (shift == 0) {
        /* The most significant digit of divisor_help is 0. Just ignore it */
        result->size--;
        divisor_help->size--;
        if (divisor_help->size == 1) {
          result->bigdigits[0] = uBigRem1(result, divisor_help->bigdigits[0]);
          memset(&result->bigdigits[1], 0,
              (size_t) (result->size - 1) * sizeof(bigdigittype));
        } else {
          uBigRem(result, divisor_help);
        } /* if */
        result->bigdigits[result->size] = 0;
        divisor_help->size++;
      } else {
        shift = BIGDIGIT_SIZE - shift;
        uBigLShift(result, shift);
        uBigLShift(divisor_help, shift);
        uBigRem(result, divisor_help);
        uBigRShift(result, shift);
      } /* if */
      result->bigdigits[dividend->size + 1] = 0;
      result->size = dividend->size + 2;
      if (negative1) {
        if (negative2) {
          negate_positive_big(result);
        } else {
          if (uBigIsNot0(result)) {
            negate_positive_big(result);
            bigAddTo(result, divisor);
          } /* if */
        } /* if */
      } else {
        if (negative2) {
          if (uBigIsNot0(result)) {
            bigAddTo(result, divisor);
          } /* if */
        } /* if */
      } /* if */
      result = normalize(result);
      FREE_BIG(divisor_help, divisor->size + 1);
      return result;
    } /* if */
  } /* bigMod */



/**
 *  Returns the product of two signed big integers.
 */
#ifdef ANSI_C

biginttype bigMult (const_biginttype factor1, const_biginttype factor2)
#else

biginttype bigMult (factor1, factor2)
biginttype factor1;
biginttype factor2;
#endif

  {
    booltype negative = FALSE;
    biginttype factor1_help = NULL;
    biginttype factor2_help = NULL;
    biginttype result;

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
    if (unlikely(!ALLOC_BIG(result, factor1->size + factor2->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      uBigMult(factor1, factor2, result);
      result->size = factor1->size + factor2->size;
      if (negative) {
        negate_positive_big(result);
      } /* if */
      result = normalize(result);
    } /* if */
#else
    result = uBigMultK(factor1, factor2, negative);
#endif
    if (factor1_help != NULL) {
      FREE_BIG(factor1_help, factor1_help->size);
    } /* if */
    if (factor2_help != NULL) {
      FREE_BIG(factor2_help, factor2_help->size);
    } /* if */
    return result;
  } /* bigMult */



#ifdef ANSI_C

void bigMultAssign (biginttype *const big_variable, const_biginttype big2)
#else

void bigMultAssign (big_variable, big2)
biginttype *big_variable;
biginttype big2;
#endif

  {
    biginttype big1;
    booltype negative = FALSE;
    biginttype big1_help = NULL;
    biginttype big2_help = NULL;
    memsizetype pos1;
    memsizetype pos2;
    doublebigdigittype carry = 0;
    biginttype result;

  /* bigMultAssign */
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
    if (IS_NEGATIVE(big2->bigdigits[big2->size - 1])) {
      negative = !negative;
      big2_help = alloc_positive_copy_of_negative_big(big2);
      big2 = big2_help;
      if (unlikely(big2_help == NULL)) {
        if (big1_help != NULL) {
          FREE_BIG(big1_help, big1_help->size);
        } /* if */
        raise_error(MEMORY_ERROR);
        return;
      } /* if */
    } /* if */
    if (unlikely(!ALLOC_BIG(result, big1->size + big2->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      pos2 = 0;
      do {
        carry += (doublebigdigittype) big1->bigdigits[0] * big2->bigdigits[pos2];
        result->bigdigits[pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos2++;
      } while (pos2 < big2->size);
      result->bigdigits[big2->size] = (bigdigittype) (carry & BIGDIGIT_MASK);
      for (pos1 = 1; pos1 < big1->size; pos1++) {
        carry = 0;
        pos2 = 0;
        do {
          carry += (doublebigdigittype) result->bigdigits[pos1 + pos2] +
              (doublebigdigittype) big1->bigdigits[pos1] * big2->bigdigits[pos2];
          result->bigdigits[pos1 + pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          pos2++;
        } while (pos2 < big2->size);
        result->bigdigits[pos1 + big2->size] = (bigdigittype) (carry & BIGDIGIT_MASK);
      } /* for */
      result->size = big1->size + big2->size;
      if (negative) {
        negate_positive_big(result);
      } /* if */
      result = normalize(result);
      FREE_BIG(*big_variable, (*big_variable)->size);
      *big_variable = result;
    } /* if */
    if (big1_help != NULL) {
      FREE_BIG(big1_help, big1_help->size);
    } /* if */
    if (big2_help != NULL) {
      FREE_BIG(big2_help, big2_help->size);
    } /* if */
  } /* bigMultAssign */



#ifdef ANSI_C

biginttype bigMultSignedDigit (const_biginttype big1, inttype number)
#else

biginttype bigMultSignedDigit (big1, number)
biginttype big1;
inttype number;
#endif

  {
    biginttype result;

  /* bigMultSignedDigit */
    /* printf("bigMultSignedDigit(big1->size=%lu, %ld)\n", big1->size, number); */
    if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, big1->size + 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = big1->size + 1;
      if (number < 0) {
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          uBigMultNegativeWithNegatedDigit(big1, (bigdigittype) -number & BIGDIGIT_MASK, result);
        } else {
          uBigMultPositiveWithNegatedDigit(big1, (bigdigittype) -number & BIGDIGIT_MASK, result);
        } /* if */
      } else {
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          uBigMultNegativeWithDigit(big1, (bigdigittype) number, result);
        } else {
          uBigMultPositiveWithDigit(big1, (bigdigittype) number, result);
        } /* if */
      } /* if */
      result = normalize(result);
    } /* if */
    return result;
  } /* bigMultSignedDigit */



#ifdef ANSI_C

booltype bigOdd (const const_biginttype big1)
#else

booltype bigOdd (big1)
biginttype big1;
#endif

  { /* bigOdd */
    return (booltype) (big1->bigdigits[0] & 1);
  } /* bigOdd */



#ifdef ANSI_C

biginttype bigParse (const const_stritype stri)
#else

biginttype bigParse (stri)
stritype stri;
#endif

  {
    memsizetype result_size;
    booltype okay;
    booltype negative;
    memsizetype position;
    doublebigdigittype digitval;
    biginttype result;

  /* bigParse */
    if (stri->size == 0) {
      result_size = 1;
    } else {
      result_size = (stri->size - 1) / DECIMAL_DIGITS_IN_BIGDIGIT + 1;
    } /* if */
    if (unlikely(!ALLOC_BIG(result, result_size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = 1;
      result->bigdigits[0] = 0;
      okay = TRUE;
      position = 0;
      if (stri->size >= 1 && stri->mem[0] == ((strelemtype) '-')) {
        negative = TRUE;
        position++;
      } else {
        negative = FALSE;
      } /* if */
      while (position < stri->size &&
          stri->mem[position] >= ((strelemtype) '0') &&
          stri->mem[position] <= ((strelemtype) '9')) {
        digitval = ((doublebigdigittype) (stri->mem[position]) - ((bigdigittype) '0'));
        uBigMultBy10AndAdd(result, digitval);
        position++;
      } /* while */
      if (position == 0 || position < stri->size) {
        okay = FALSE;
      } /* if */
      if (likely(okay)) {
        memset(&result->bigdigits[result->size], 0,
            (size_t) (result_size - result->size) * sizeof(bigdigittype));
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
  } /* bigParse */



/**
 *  Returns a signed big integer decremented by 1.
 */
#ifdef ANSI_C

biginttype bigPred (const const_biginttype big1)
#else

biginttype bigPred (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    biginttype resized_result;
    biginttype result;

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
          /* memset(result->bigdigits, 0xFF, pos * sizeof(bigdigittype)); */
        } /* if */
      } /* if */
      if (pos < big1->size) {
        result->bigdigits[pos] = big1->bigdigits[pos] - 1;
        pos++;
        memcpy(&result->bigdigits[pos], &big1->bigdigits[pos],
            (big1->size - pos) * sizeof(bigdigittype));
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
#ifdef ANSI_C

biginttype bigPredTemp (biginttype big1)
#else

biginttype bigPredTemp (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    bigdigittype negative;
    biginttype resized_big1;

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
        /* memset(big1->bigdigits, 0xFF, pos * sizeof(bigdigittype)); */
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
 *  Computes a random number between lower_limit and upper_limit
 *  for signed big integers. The memory for the result is requested
 *  and the normalized result is returned. The random numbers are
 *  uniform distributed over the whole range.
 */
#ifdef ANSI_C

biginttype bigRand (const const_biginttype lower_limit,
    const const_biginttype upper_limit)
#else

biginttype bigRand (lower_limit, upper_limit)
biginttype lower_limit;
biginttype upper_limit;
#endif

  {
    biginttype scale_limit;
    bigdigittype mask;
    memsizetype pos;
    doublebigdigittype random_number = 0;
    memsizetype result_size;
    biginttype result;

  /* bigRand */
    if (unlikely(bigCmp(lower_limit, upper_limit) > 0)) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      scale_limit = bigSbtr(upper_limit, lower_limit);
      if (lower_limit->size > scale_limit->size) {
        result_size = lower_limit->size + 1;
      } else {
        result_size = scale_limit->size + 1;
      } /* if */
      if (unlikely(!ALLOC_BIG(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        memset(&result->bigdigits[scale_limit->size], 0,
            (size_t) (result_size - scale_limit->size) * sizeof(bigdigittype));
        result->size = scale_limit->size;
        mask = ((bigdigittype) BIGDIGIT_MASK) >>
            (BIGDIGIT_SIZE -
            (memsizetype) (digitMostSignificantBit(scale_limit->bigdigits[scale_limit->size - 1]) + 1));
        do {
          pos = 0;
          do {
            if (random_number == 0) {
              random_number = uint_rand();
            } /* if */
            result->bigdigits[pos] = (bigdigittype) (random_number & BIGDIGIT_MASK);
            random_number >>= BIGDIGIT_SIZE;
            pos++;
          } while (pos < scale_limit->size);
          result->bigdigits[pos - 1] &= mask;
        } while (bigCmp(result, scale_limit) > 0);
        result->size = result_size;
        bigAddTo(result, lower_limit);
        result = normalize(result);
        FREE_BIG(scale_limit, scale_limit->size);
        return result;
      } /* if */
    } /* if */
  } /* bigRand */



/**
 *  Computes the remainder of an integer division of dividend by divisor
 *  for signed big integers. The memory for the result is requested
 *  and the normalized result is returned. When divisor has just one
 *  digit or when dividend has less digits than divisor the bigRem1() or
 *  bigRemSizeLess() functions are called. In the general case
 *  the absolute values of dividend and divisor are taken. Then dividend is
 *  extended by one leading zero digit. After that dividend and divisor
 *  are shifted to the left such that the most significant bit
 *  of divisor is set. This fulfills the preconditions for calling
 *  uBigRem() which does the main work of the division. Afterwards
 *  the result must be shifted to the right to get the remainder.
 */
#ifdef ANSI_C

biginttype bigRem (const const_biginttype dividend, const const_biginttype divisor)
#else

biginttype bigRem (dividend, divisor)
biginttype dividend;
biginttype divisor;
#endif

  {
    booltype negative = FALSE;
    biginttype divisor_help;
    unsigned int shift;
    biginttype result;

  /* bigRem */
    if (divisor->size == 1) {
      result = bigRem1(dividend, divisor->bigdigits[0]);
      return result;
    } else if (dividend->size < divisor->size) {
      result = bigRemSizeLess(dividend, divisor);
      return result;
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, dividend->size + 2))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(dividend->bigdigits[dividend->size - 1])) {
          negative = TRUE;
          positive_copy_of_negative_big(result, dividend);
        } else {
          result->size = dividend->size;
          memcpy(result->bigdigits, dividend->bigdigits,
              (size_t) dividend->size * sizeof(bigdigittype));
        } /* if */
        result->bigdigits[result->size] = 0;
        result->size++;
      } /* if */
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(divisor_help, divisor->size + 1))) {
        FREE_BIG(result,  dividend->size + 2);
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (IS_NEGATIVE(divisor->bigdigits[divisor->size - 1])) {
          positive_copy_of_negative_big(divisor_help, divisor);
        } else {
          divisor_help->size = divisor->size;
          memcpy(divisor_help->bigdigits, divisor->bigdigits,
              (size_t) divisor->size * sizeof(bigdigittype));
        } /* if */
      } /* if */
      shift = (unsigned int) (digitMostSignificantBit(divisor_help->bigdigits[divisor_help->size - 1]) + 1);
      if (shift == 0) {
        /* The most significant digit of divisor_help is 0. Just ignore it */
        result->size--;
        divisor_help->size--;
        if (divisor_help->size == 1) {
          result->bigdigits[0] = uBigRem1(result, divisor_help->bigdigits[0]);
          memset(&result->bigdigits[1], 0,
              (size_t) (result->size - 1) * sizeof(bigdigittype));
        } else {
          uBigRem(result, divisor_help);
        } /* if */
        result->bigdigits[result->size] = 0;
        divisor_help->size++;
      } else {
        shift = BIGDIGIT_SIZE - shift;
        uBigLShift(result, shift);
        uBigLShift(divisor_help, shift);
        uBigRem(result, divisor_help);
        uBigRShift(result, shift);
      } /* if */
      result->bigdigits[dividend->size + 1] = 0;
      result->size = dividend->size + 2;
      if (negative) {
        negate_positive_big(result);
      } /* if */
      result = normalize(result);
      FREE_BIG(divisor_help, divisor->size + 1);
      return result;
    } /* if */
  } /* bigRem */



#ifdef ANSI_C

biginttype bigRShift (const const_biginttype big1, const inttype rshift)
#else

biginttype bigRShift (big1, rshift)
biginttype big1;
inttype rshift;
#endif

  {
    memsizetype size_reduction;
    unsigned int digit_rshift;
    unsigned int digit_lshift;
    bigdigittype digit_mask;
    bigdigittype low_digit;
    bigdigittype high_digit;
    const bigdigittype *source_digits;
    bigdigittype *dest_digits;
    memsizetype pos;
    memsizetype result_size;
    biginttype result;

  /* bigRShift */
    if (unlikely(rshift < 0)) {
      result = bigLShift(big1, -rshift);
    } else if (big1->size <= (uinttype) rshift >> BIGDIGIT_LOG2_SIZE) {
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
      size_reduction = (memsizetype) ((uinttype) rshift >> BIGDIGIT_LOG2_SIZE);
      result_size = big1->size - size_reduction;
      if ((rshift & BIGDIGIT_SIZE_MASK) == 0) {
        if (unlikely(!ALLOC_BIG_SIZE_OK(result, result_size))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = result_size;
          memcpy(result->bigdigits, &big1->bigdigits[size_reduction],
              (size_t) result_size * sizeof(bigdigittype));
        } /* if */
      } else {
        digit_rshift = (uinttype) rshift & BIGDIGIT_SIZE_MASK;
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
          if ((memsizetype) (dest_digits - result->bigdigits) < result_size) {
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



#ifdef ANSI_C

void bigRShiftAssign (biginttype *const big_variable, inttype rshift)
#else

void bigRShiftAssign (big_variable, rshift)
biginttype *const big_variable;
inttype rshift;
#endif

  {
    biginttype big1;
    memsizetype size_reduction;
    unsigned int digit_rshift;
    unsigned int digit_lshift;
    bigdigittype low_digit;
    bigdigittype high_digit;
    const bigdigittype *source_digits;
    bigdigittype *dest_digits;
    biginttype resized_big1;
    memsizetype pos;
    memsizetype big1_size;

  /* bigRShiftAssign */
    if (unlikely(rshift < 0)) {
      bigLShiftAssign(big_variable, -rshift);
    } else {
      big1 = *big_variable;
      if (big1->size <= (uinttype) rshift >> BIGDIGIT_LOG2_SIZE) {
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
        size_reduction = (memsizetype) ((uinttype) rshift >> BIGDIGIT_LOG2_SIZE);
        if ((rshift & BIGDIGIT_SIZE_MASK) == 0) {
          if (rshift != 0) {
            big1_size = big1->size;
            memmove(big1->bigdigits, &big1->bigdigits[size_reduction],
                (size_t) (big1_size - size_reduction) * sizeof(bigdigittype));
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
          digit_rshift = (uinttype) rshift & BIGDIGIT_SIZE_MASK;
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
 *  Returns the difference of two signed big integers.
 */
#ifdef ANSI_C

biginttype bigSbtr (const const_biginttype minuend, const const_biginttype subtrahend)
#else

biginttype bigSbtr (minuend, subtrahend)
biginttype minuend;
biginttype subtrahend;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;
    doublebigdigittype minuend_sign;
    doublebigdigittype subtrahend_sign;
    biginttype result;

  /* bigSbtr */
    if (minuend->size >= subtrahend->size) {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, minuend->size + 1))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        pos = 0;
        do {
          carry += (doublebigdigittype) minuend->bigdigits[pos] +
              (~subtrahend->bigdigits[pos] & BIGDIGIT_MASK);
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          pos++;
        } while (pos < subtrahend->size);
        subtrahend_sign = IS_NEGATIVE(subtrahend->bigdigits[pos - 1]) ? 0 : BIGDIGIT_MASK;
        for (; pos < minuend->size; pos++) {
          carry += minuend->bigdigits[pos] + subtrahend_sign;
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        if (IS_NEGATIVE(minuend->bigdigits[pos - 1])) {
          subtrahend_sign--;
        } /* if */
        result->bigdigits[pos] = (bigdigittype) ((carry + subtrahend_sign) & BIGDIGIT_MASK);
        result->size = pos + 1;
        result = normalize(result);
        return result;
      } /* if */
    } else {
      if (unlikely(!ALLOC_BIG_CHECK_SIZE(result, subtrahend->size + 1))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        pos = 0;
        do {
          carry += (doublebigdigittype) minuend->bigdigits[pos] +
              (~subtrahend->bigdigits[pos] & BIGDIGIT_MASK);
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          pos++;
        } while (pos < minuend->size);
        minuend_sign = IS_NEGATIVE(minuend->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
        for (; pos < subtrahend->size; pos++) {
          carry += minuend_sign + (~subtrahend->bigdigits[pos] & BIGDIGIT_MASK);
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        subtrahend_sign = IS_NEGATIVE(subtrahend->bigdigits[pos - 1]) ? 0 : BIGDIGIT_MASK;
        if (IS_NEGATIVE(minuend->bigdigits[minuend->size - 1])) {
          subtrahend_sign--;
        } /* if */
        result->bigdigits[pos] = (bigdigittype) ((carry + subtrahend_sign) & BIGDIGIT_MASK);
        result->size = pos + 1;
        result = normalize(result);
        return result;
      } /* if */
    } /* if */
  } /* bigSbtr */



/**
 *  Returns the difference of two signed big integers.
 *  Big1 is assumed to be a temporary value which is reused.
 */
#ifdef ANSI_C

biginttype bigSbtrTemp (biginttype big1, const_biginttype big2)
#else

biginttype bigSbtrTemp (big1, big2)
biginttype big1;
biginttype big2;
#endif

  { /* bigSbtrTemp */
    bigShrink(&big1, big2);
    return big1;
  } /* bigSbtrTemp */



/**
 *  Subtracts big2 from *big_variable. The operation is done in
 *  place and *big_variable is only resized when necessary.
 *  When the size of big2 is smaller than *big_variable the
 *  algorithm tries to save computations. Therefore there are
 *  checks for carry != 0 and carry == 0.
 *  In case the resizing fails the content of *big_variable
 *  is freed and *big_variable is set to NULL.
 */
#ifdef ANSI_C

void bigShrink (biginttype *const big_variable, const const_biginttype big2)
#else

void bigShrink (big_variable, big2)
biginttype *big_variable;
biginttype big2;
#endif

  {
    biginttype big1;
    memsizetype pos;
    memsizetype big1_size;
    doublebigdigittype carry = 1;
    doublebigdigittype big1_sign;
    doublebigdigittype big2_sign;
    biginttype resized_big1;

  /* bigShrink */
    big1 = *big_variable;
    if (big1->size >= big2->size) {
      big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
      pos = 0;
      do {
        carry += (doublebigdigittype) big1->bigdigits[pos] +
            (~big2->bigdigits[pos] & BIGDIGIT_MASK);
        big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= BIGDIGIT_SIZE;
        pos++;
      } while (pos < big2->size);
      if (IS_NEGATIVE(big2->bigdigits[pos - 1])) {
        for (; carry != 0 && pos < big1->size; pos++) {
          carry += big1->bigdigits[pos];
          big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
      } else {
        for (; carry == 0 && pos < big1->size; pos++) {
          carry = (doublebigdigittype) big1->bigdigits[pos] + BIGDIGIT_MASK;
          big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
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
          /* It is possible that big1 == big2 holds. Since */
          /* 'big2' is not used after realloc() enlarged   */
          /* 'big1' a correction of big2 is not necessary. */
          big1 = resized_big1;
          COUNT3_BIG(big1_size, big1_size + 1);
          big1->size++;
          big1->bigdigits[big1_size] = (bigdigittype) (carry & BIGDIGIT_MASK);
          *big_variable = big1;
        } /* if */
      } else {
        *big_variable = normalize(big1);
      } /* if */
    } else {
      REALLOC_BIG_CHECK_SIZE(resized_big1, big1, big1->size, big2->size + 1);
      if (unlikely(resized_big1 == NULL)) {
        FREE_BIG(big1, big1->size);
        *big_variable = NULL;
        raise_error(MEMORY_ERROR);
      } else {
        big1 = resized_big1;
        COUNT3_BIG(big1->size, big2->size + 1);
        big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
        pos = 0;
        do {
          carry += (doublebigdigittype) big1->bigdigits[pos] +
              (~big2->bigdigits[pos] & BIGDIGIT_MASK);
          big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
          pos++;
        } while (pos < big1->size);
        big2_sign = IS_NEGATIVE(big2->bigdigits[big2->size - 1]) ? 0 : BIGDIGIT_MASK;
        for (; pos < big2->size; pos++) {
          carry += big1_sign + (~big2->bigdigits[pos] & BIGDIGIT_MASK);
          big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= BIGDIGIT_SIZE;
        } /* for */
        carry += big1_sign + big2_sign;
        big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        big1->size = pos + 1;
        *big_variable = normalize(big1);
      } /* if */
    } /* if */
  } /* bigShrink */



/**
 *  Returns the square of a signed big integer.
 */
#ifdef ANSI_C

biginttype bigSquare (const_biginttype big1)
#else

biginttype bigSquare (big1)
biginttype big1;
#endif

  {
    biginttype big1_help = NULL;
    biginttype result;

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



#ifdef ANSI_C

stritype bigStr (const const_biginttype big1)
#else

stritype bigStr (big1)
biginttype big1;
#endif

  {
    biginttype help_big;
    memsizetype pos;
    bigdigittype digit;
    int digit_pos;
    memsizetype result_size;
    stritype result;

  /* bigStr */
    if (unlikely((MAX_STRI_LEN <= (MAX_MEMSIZETYPE - 1) / 3 + 2 &&
        big1->size > ((MAX_STRI_LEN - 2) * 3 + 1) / BIGDIGIT_SIZE) ||
        big1->size > MAX_MEMSIZETYPE / BIGDIGIT_SIZE)) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      /* The size of the result is estimated by computing the */
      /* number of octal digits plus one digit for the sign.  */
      result_size = (big1->size * BIGDIGIT_SIZE - 1) / 3 + 2;
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        if (unlikely(!ALLOC_BIG_CHECK_SIZE(help_big, big1->size + 1))) {
          FREE_STRI(result, result_size);
          raise_error(MEMORY_ERROR);
          return NULL;
        } else {
          if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
            positive_copy_of_negative_big(help_big, big1);
          } else {
            help_big->size = big1->size;
            memcpy(help_big->bigdigits, big1->bigdigits,
                (size_t) big1->size * sizeof(bigdigittype));
          } /* if */
          pos = result_size - 1;
          do {
            digit = uBigDivideByPowerOf10(help_big);
            /* printf("help_big->size=%lu, digit=%lu\n", help_big->size, digit); */
            if (help_big->bigdigits[help_big->size - 1] == 0) {
              help_big->size--;
            } /* if */
            if (help_big->size > 1 || help_big->bigdigits[0] != 0) {
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
          } while (help_big->size > 1 || help_big->bigdigits[0] != 0);
          FREE_BIG(help_big, big1->size + 1);
          pos++;
          if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
            result->size = result_size - pos + 1;
            result->mem[0] = '-';
            memmove(&result->mem[1], &result->mem[pos],
                (result_size - pos) * sizeof(strelemtype));
          } else {
            result->size = result_size - pos;
            memmove(&result->mem[0], &result->mem[pos],
                (result_size - pos) * sizeof(strelemtype));
          } /* if */
          return result;
        } /* if */
      } /* if */
    } /* if */
  } /* bigStr */



/**
 *  Returns a signed big integer incremented by 1.
 */
#ifdef ANSI_C

biginttype bigSucc (const const_biginttype big1)
#else

biginttype bigSucc (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    biginttype resized_result;
    biginttype result;

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
          /* memset(result->bigdigits, 0, pos * sizeof(bigdigittype)); */
        } /* if */
      } /* if */
      if (pos < big1->size) {
        result->bigdigits[pos] = big1->bigdigits[pos] + 1;
        pos++;
        memcpy(&result->bigdigits[pos], &big1->bigdigits[pos],
            (big1->size - pos) * sizeof(bigdigittype));
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
 *  Returns a signed big integer incremented by 1.
 *  Big1 is assumed to be a temporary value which is reused.
 */
#ifdef ANSI_C

biginttype bigSuccTemp (biginttype big1)
#else

biginttype bigSuccTemp (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    bigdigittype negative;
    biginttype resized_big1;

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
        /* memset(big1->bigdigits, 0, pos * sizeof(bigdigittype)); */
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



#ifdef ANSI_C

bstritype bigToBStri (const_biginttype big1)
#else

bstritype bigToBStri (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    int byteNum;
    bigdigittype digit;
    memsizetype charIndex;
    memsizetype result_size;
    bstritype result;

  /* bigToBStri */
    /* The expression computing result_size does not overflow           */
    /* because the number of bytes in a bigInteger fits in memsizetype. */
    result_size = big1->size * (BIGDIGIT_SIZE >> 3);
    digit = big1->bigdigits[big1->size - 1];
    byteNum = (BIGDIGIT_SIZE >> 3) - 1;
    if (IS_NEGATIVE(digit)) {
      while (byteNum > 0 && (digit >> byteNum * 8 & 0xFF) == 0xFF) {
        result_size--;
        byteNum--;
      } /* while */
      if (byteNum < 3 && (digit >> byteNum * 8 & 0xFF) <= 127) {
        result_size++;
        /* Not used afterwards: byteNum++; */
      } /* if */
    } else {
      while (byteNum > 0 && (digit >> byteNum * 8 & 0xFF) == 0) {
        result_size--;
        byteNum--;
      } /* while */
      if (byteNum < 3 && (digit >> byteNum * 8 & 0xFF) >= 128) {
        result_size++;
        /* Not used afterwards: byteNum++; */
      } /* if */
    } /* if */
    if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
      charIndex = 0;
      pos = big1->size;
      while (pos > 0) {
        pos--;
        digit = big1->bigdigits[pos];
        for (byteNum = (BIGDIGIT_SIZE >> 3) - 1; byteNum >= 0; byteNum--) {
          if (pos * (BIGDIGIT_SIZE >> 3) + (memsizetype) byteNum < result_size) {
            result->mem[charIndex] = (uchartype) (digit >> byteNum * 8 & 0xFF);
            charIndex++;
          } /* if */
        } /* for */
      } /* for */
    } /* if */
    return result;
  } /* bigToBStri */



#ifdef ANSI_C

int32type bigToInt32 (const const_biginttype big1)
#else

int32type bigToInt32 (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    int32type result;

  /* bigToInt32 */
    if (unlikely(big1->size > sizeof(int32type) / (BIGDIGIT_SIZE >> 3))) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      pos = big1->size - 1;
      result = (int32type) (signedbigdigittype) big1->bigdigits[pos];
#if BIGDIGIT_SIZE < 32
      while (pos > 0) {
        pos--;
        result <<= BIGDIGIT_SIZE;
        result |= (int32type) big1->bigdigits[pos];
      } /* while */
#endif
      return result;
    } /* if */
  } /* bigToInt32 */



#ifdef INT64TYPE
#ifdef ANSI_C

int64type bigToInt64 (const const_biginttype big1)
#else

int64type bigToInt64 (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    int64type result;

  /* bigToInt64 */
    if (unlikely(big1->size > sizeof(int64type) / (BIGDIGIT_SIZE >> 3))) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      pos = big1->size - 1;
      result = (int64type) (signedbigdigittype) big1->bigdigits[pos];
#if BIGDIGIT_SIZE < 64
      while (pos > 0) {
        pos--;
        result <<= BIGDIGIT_SIZE;
        result |= (int64type) big1->bigdigits[pos];
      } /* while */
#endif
      return result;
    } /* if */
  } /* bigToInt64 */
#endif



#ifdef ANSI_C

biginttype bigZero (void)
#else

biginttype bigZero ()
#endif

  {
    biginttype result;

  /* bigZero */
    if (unlikely(!ALLOC_BIG_SIZE_OK(result, 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = 1;
      result->bigdigits[0] = 0;
    } /* if */
    return result;
  } /* bigZero */
