/********************************************************************/
/*                                                                  */
/*  int_rtl.c     Primitive actions for the integer type.           */
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
/*  File: seed7/src/int_rtl.c                                       */
/*  Changes: 1992 - 1994, 2000, 2005, 2009 - 2014  Thomas Mertes    */
/*  Content: Primitive actions for the integer type.                */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "tim_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "int_rtl.h"

#undef TRACE_INT_RTL
#undef VERBOSE_EXCEPTIONS


#define UINT_BITS(A)                     ((A) & UINTTYPE_MAX)
#define UINT_BITS_WITHOUT_HIGHEST_BIT(A) ((A) & (UINTTYPE_MAX >> 1))
#define UINT_HIGHEST_BIT(A)              ((A) >> (INTTYPE_SIZE - 1))
#define UINT_LOWER_HALF_BITS_SET         (UINTTYPE_MAX >> (INTTYPE_SIZE >> 1))
#define LOWER_HALF_OF_UINT(A)            ((A) & UINT_LOWER_HALF_BITS_SET)
#define UPPER_HALF_OF_UINT(A)            ((A) >> (INTTYPE_SIZE >> 1))

/* Number of characters needed when the most negative  */
/* integer is written with radix 2. One character is   */
/* needed for the sign. Assume that integers are just  */
/* signed bytes. In this case the most negative number */
/* would be -128. -128 radix 2  returns  "-10000000"   */
/* The result needs 9 digits although -128 fits into a */
/* signed byte with 8 bits.                            */
#define RADIX_BUFFER_SIZE (8 * sizeof(intType) + 1)

#define BYTE_BUFFER_SIZE sizeof(intType)

#if   INTTYPE_SIZE == 32
#define DECIMAL_DIGITS(num)                    \
  (num < INT_SUFFIX(100000000) ?               \
      (num < INT_SUFFIX(10000) ?               \
        (num < INT_SUFFIX(100) ?               \
          (num < INT_SUFFIX(10) ? 1 : 2)       \
        :                                      \
          (num < INT_SUFFIX(1000) ? 3 : 4)     \
        )                                      \
      :                                        \
        (num < INT_SUFFIX(1000000) ?           \
          (num < INT_SUFFIX(100000) ? 5 : 6)   \
        :                                      \
          (num < INT_SUFFIX(10000000) ? 7 : 8) \
        )                                      \
      )                                        \
    :                                          \
      (num < INT_SUFFIX(1000000000) ? 9 : 10)  \
    )
#elif INTTYPE_SIZE == 64
#define DECIMAL_DIGITS(num) \
    (num < INT_SUFFIX(10000000000000000) ?                  \
      (num < INT_SUFFIX(100000000) ?                        \
        (num < INT_SUFFIX(10000) ?                          \
          (num < INT_SUFFIX(100) ?                          \
            (num < INT_SUFFIX(10) ? 1 : 2)                  \
          :                                                 \
            (num < INT_SUFFIX(1000) ? 3 : 4)                \
          )                                                 \
        :                                                   \
          (num < INT_SUFFIX(1000000) ?                      \
            (num < INT_SUFFIX(100000) ? 5 : 6)              \
          :                                                 \
            (num < INT_SUFFIX(10000000) ? 7 : 8)            \
          )                                                 \
        )                                                   \
      :                                                     \
        (num < INT_SUFFIX(1000000000000) ?                  \
          (num < INT_SUFFIX(10000000000) ?                  \
            (num < INT_SUFFIX(1000000000) ? 9 : 10)         \
          :                                                 \
            (num < INT_SUFFIX(100000000000) ? 11 : 12)      \
          )                                                 \
        :                                                   \
          (num < INT_SUFFIX(100000000000000) ?              \
            (num < INT_SUFFIX(10000000000000) ? 13 : 14)    \
          :                                                 \
            (num < INT_SUFFIX(1000000000000000) ? 15 : 16)  \
          )                                                 \
        )                                                   \
      )                                                     \
    :                                                       \
      (num < INT_SUFFIX(1000000000000000000) ?              \
        (num < INT_SUFFIX(100000000000000000) ? 17 : 18)    \
      :                                                     \
        (num < INT_SUFFIX(10000000000000000000U) ? 19 : 20) \
      )                                                     \
    )
#endif


static const int most_significant[] = {
   -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
  };

static const int least_significant[] = {
    8, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
  };

const intType maxExponentOfBase[] = {
    21, 22, 24, 27, 31, 39, 63,
    INTTYPE_MAX, INTTYPE_MAX, INTTYPE_MAX,
    62, 39, 31, 27, 24, 22, 20
  };

const intType minBaseOfExponent[] = {
    INTTYPE_MIN, INTTYPE_MIN,
    -INT_SUFFIX(3037000499), -2097152, -55108, -6208, -1448, -512, -234, -128,
    -78, -52, -38, -28, -22, -18, -15, -13, -11, -9, -8, -8, -7
  };

const intType maxBaseOfExponent[] = {
    INTTYPE_MAX, INTTYPE_MAX,
    INT_SUFFIX(3037000499), 2097151, 55108, 6208, 1448,
    511, 234, 127, 78, 52, 38, 28, 22, 18, 15, 13, 11, 9, 8, 7, 7
};

const char lcDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
const char ucDigits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const const_cstriType digitTable[] = {lcDigits, ucDigits};

#if INTTYPE_SIZE == 32
#define RAND_MULTIPLIER           1103515245
#define RAND_INCREMENT                 12345
#elif INTTYPE_SIZE == 64
#define RAND_MULTIPLIER  6364136223846793005
#define RAND_INCREMENT   1442695040888963407
#endif

#ifdef HAS_DOUBLE_INTTYPE
static doubleUintType seed;
#else
static uintType low_seed;
static uintType high_seed;
#endif

#ifdef VERBOSE_EXCEPTIONS
#define logError(logStatements) logStatements
#else
#define logError(logStatements)
#endif



void setupRand (void)

  {
    uintType micro_sec;
#ifdef HAS_DOUBLE_INTTYPE
    uintType low_seed;
    uintType high_seed;
#endif

  /* setupRand */
    micro_sec = (uintType) timMicroSec();
    high_seed = (uintType) time(NULL);
    high_seed = high_seed ^ (high_seed << 16);
    low_seed = (uintType) clock();
    low_seed = (low_seed ^ (low_seed << 16)) ^ high_seed;
    /* printf("%10lo %010lo seed\n", (long unsigned) high_seed, (long unsigned) low_seed); */
    high_seed ^= micro_sec ^ micro_sec << 8 ^ micro_sec << 16 ^ micro_sec << 24;
#if INTTYPE_SIZE >= 64
    high_seed ^= micro_sec << 32 ^ micro_sec << 40 ^ micro_sec << 48 ^ micro_sec << 56;
#endif
    low_seed ^= micro_sec ^ micro_sec << 8 ^ micro_sec << 16 ^ micro_sec << 24;
#if INTTYPE_SIZE >= 64
    low_seed ^= micro_sec << 32 ^ micro_sec << 40 ^ micro_sec << 48 ^ micro_sec << 56;
#endif
    /* printf("%10lo %010lo seed\n", (long unsigned) high_seed, (long unsigned) low_seed); */
#ifdef HAS_DOUBLE_INTTYPE
    seed = (doubleUintType) high_seed << INTTYPE_SIZE | (doubleUintType) low_seed;
#endif
  } /* setupRand */



#ifdef OUT_OF_ORDER
/**
 *  Multiply two uintType factors to an uintType product.
 *  A uintType number is splitted into two halve uintType parts.
 *  The bits of an uintType have the following memory layout:
 *  +-------------------+
 *  |     uintType      |
 *  +---------+---------+
 *  |  part1  |  part0  |
 *  +---------+---------+
 *   ^ highest bit     ^ lowest bit
 *  @exception OVERFLOW_ERROR When the multiplication overflows.
 *  @return the product
 */
static uintType uint_safe_mult (uintType factor1, uintType factor2)

  {
    uintType factor1_part0;
    uintType factor1_part1;
    uintType factor2_part0;
    uintType factor2_part1;
    uintType c1;  /* memory layout:   | part[1] | part[0] |  */
    uintType c2;  /* memory layout:   | part[2] | part[1] |  */
    uintType product;

  /* uint_safe_mult */
    factor1_part0 = LOWER_HALF_OF_UINT(factor1);
    factor1_part1 = UPPER_HALF_OF_UINT(factor1);
    factor2_part0 = LOWER_HALF_OF_UINT(factor2);
    factor2_part1 = UPPER_HALF_OF_UINT(factor2);
    if (factor1_part1 == 0) {
      c1 = factor1_part0 * factor2_part1;
    } else if (factor2_part1 == 0) {
      c1 = factor1_part1 * factor2_part0;
    } else {
      raise_error(OVERFLOW_ERROR);
      return 0;
    } /* if */
    if (unlikely(c1 > UINT_LOWER_HALF_BITS_SET)) {
      raise_error(OVERFLOW_ERROR);
      return 0;
    } else {
      c1 <<= 32;
      c2 = factor1_part0 * factor2_part0;
      if (unlikely(c1 > UINTTYPE_MAX - c2)) {
        raise_error(OVERFLOW_ERROR);
        return 0;
      } else {
        product = c1 + c2;
      } /* if */
    } /* if */
    return product;
  } /* uint_safe_mult */



static uintType uint_safe_mult2 (uintType factor1, uintType factor2)

  {
    uintType product;

  /* uint_safe_mult2 */
    if (factor1 != 0) {
      if (unlikely(factor2 > UINTTYPE_MAX / factor1)) {
        raise_error(OVERFLOW_ERROR);
        return 0;
      } /* if */
    } /* if */
    product = factor1 * factor2;
    return product;
  } /* uint_safe_mult2 */
#endif



/**
 *  Multiply two uintType factors to a double uintType product.
 *  The whole product fits into the double uintType number.
 *  The product is returned in product_high and product_low.
 *  A double uintType number consists of a low and a high uintType
 *  number. A double uintType number can also be splitted into
 *  four halve uintType parts. The bits of a double uintType have
 *  the following memory layout:
 *  +---------------------------------------+
 *  |            double uintType            |
 *  +-------------------+-------------------+
 *  |   high uintType   |    low uintType   |
 *  +---------+---------+---------+---------+
 *  | part[3] | part[2] | part[1] | part[0] |
 *  +---------+---------+---------+---------+
 *   ^ highest bit                         ^ lowest bit
 *  @param product_high The address to return the high product.
 *  @return the low product
 */
uintType uint_mult (uintType factor1, uintType factor2, uintType *product_high)

  {
    uintType factor1_part[2];  /* parts 2 and 3 are not used */
    uintType factor2_part[2];  /* parts 2 and 3 are not used */
    uintType c1;  /* memory layout:   | part[1] | part[0] |  */
    uintType c2;  /* memory layout:   | part[2] | part[1] |  */
    uintType c3;  /* memory layout:   | part[2] | part[1] |  */
    uintType c4;  /* memory layout:   | part[2] | part[1] |  */
    uintType c5;  /* memory layout:   | part[3] | part[2] |  */
    uintType product_low;

  /* uint_mult */
#ifdef TRACE_RANDOM
    printf("BEGIN uint_mult(%08x, %08x)\n",
        (unsigned int) factor1, (unsigned int) factor2);
#endif
    factor1_part[0] = LOWER_HALF_OF_UINT(factor1);
    factor1_part[1] = UPPER_HALF_OF_UINT(factor1);
    factor2_part[0] = LOWER_HALF_OF_UINT(factor2);
    factor2_part[1] = UPPER_HALF_OF_UINT(factor2);
    c1 = factor1_part[0] * factor2_part[0];
    c2 = factor1_part[0] * factor2_part[1];
    c3 = factor1_part[1] * factor2_part[0];
    c4 = UPPER_HALF_OF_UINT(c1) + LOWER_HALF_OF_UINT(c2) + LOWER_HALF_OF_UINT(c3);
    c5 = UPPER_HALF_OF_UINT(c2) + UPPER_HALF_OF_UINT(c3) + UPPER_HALF_OF_UINT(c4) +
         factor1_part[1] * factor2_part[1];
    /* c5 contains the high uintType of factor1 * factor2 */
    product_low = UINT_BITS(factor1 * factor2);
    *product_high = UINT_BITS(c5);
#ifdef TRACE_RANDOM
    printf("END uint_mult ==> %08x%08x\n",
        (unsigned int) *product_high, (unsigned int) product_low);
#endif
    return product_low;
  } /* uint_mult */



/**
 *  Multiply two double uintType factors to a double uintType product.
 *  The low bits of the product are returned as double uintType
 *  number (in product_high and product_low). The higher bits of
 *  the product (the bits higher than product_high) are discarded.
 *  A double uintType number consists of a low and a high uintType
 *  number. A double uintType number can also be splitted into
 *  four halve uintType parts. The bits of a double uintType have
 *  the following memory layout:
 *  +---------------------------------------+
 *  |            double uintType            |
 *  +-------------------+-------------------+
 *  |   high uintType   |    low uintType   |
 *  +---------+---------+---------+---------+
 *  | part[3] | part[2] | part[1] | part[0] |
 *  +---------+---------+---------+---------+
 *   ^ highest bit                         ^ lowest bit
 *  @param product_high The address to return the high product.
 *  @return the low product
 */
static inline uintType uint2_mult (uintType factor1_high, uintType factor1_low,
    uintType factor2_high, uintType factor2_low, uintType *product_high)

  {
    uintType factor1_part[2];  /* parts 2 and 3 are not used */
    uintType factor2_part[2];  /* parts 2 and 3 are not used */
    uintType c1;  /* memory layout:   | part[1] | part[0] |  */
    uintType c2;  /* memory layout:   | part[2] | part[1] |  */
    uintType c3;  /* memory layout:   | part[2] | part[1] |  */
    uintType c4;  /* memory layout:   | part[2] | part[1] |  */
    uintType c5;  /* memory layout:   | part[3] | part[2] |  */
    uintType product_low;

  /* uint2_mult */
#ifdef TRACE_RANDOM
    printf("BEGIN uint2_mult(%08x%08x, %08x%08x)\n",
        (unsigned int) factor1_high, (unsigned int) factor1_low,
        (unsigned int) factor2_high, (unsigned int) factor2_low);
#endif
    factor1_part[0] = LOWER_HALF_OF_UINT(factor1_low);
    factor1_part[1] = UPPER_HALF_OF_UINT(factor1_low);
    factor2_part[0] = LOWER_HALF_OF_UINT(factor2_low);
    factor2_part[1] = UPPER_HALF_OF_UINT(factor2_low);
    c1 = factor1_part[0] * factor2_part[0];
    c2 = factor1_part[0] * factor2_part[1];
    c3 = factor1_part[1] * factor2_part[0];
    c4 = UPPER_HALF_OF_UINT(c1) + LOWER_HALF_OF_UINT(c2) + LOWER_HALF_OF_UINT(c3);
    c5 = UPPER_HALF_OF_UINT(c2) + UPPER_HALF_OF_UINT(c3) + UPPER_HALF_OF_UINT(c4) +
         factor1_part[1] * factor2_part[1];
    /* c5 contains the high uintType of factor1_low * factor2_low */
    product_low = UINT_BITS(factor1_low * factor2_low);
    *product_high = UINT_BITS(factor1_low * factor2_high + factor1_high * factor2_low + c5);
    /* factor1_high * factor2_high is not computed. All bits of it  */
    /* would be discarded, since they are higher than product_high. */
#ifdef TRACE_RANDOM
    printf("END uint2_mult ==> %08x%08x\n",
        (unsigned int) *product_high, (unsigned int) product_low);
#endif
    return product_low;
  } /* uint2_mult */



/**
 *  Add two double uintType summands to a double uintType sum.
 *  A possible excess bit, that does not fit into the sum
 *  (the excess bit is higher than sum_high), is discarded.
 *  A double uintType number consists of a low and a high uintType
 *  number. The bits of a double uintType have the following
 *  memory layout:
 *  +---------------------------------------+
 *  |            double uintType            |
 *  +-------------------+-------------------+
 *  |   high uintType   |    low uintType   |
 *  +-------------------+-------------------+
 *   ^ highest bit                         ^ lowest bit
 *  @param sum_high The address to return the high sum.
 *  @return the low sum
 */
static inline uintType uint2_add (uintType summand1_high, uintType summand1_low,
    uintType summand2_high, uintType summand2_low, uintType *sum_high)

  {
    uintType sum_low;

  /* uint2_add */
#ifdef TRACE_RANDOM
    printf("BEGIN uint2_add(%08x%08x, %08x%08x)\n",
        (unsigned int) summand1_high, (unsigned int) summand1_low,
        (unsigned int) summand2_high, (unsigned int) summand2_low);
#endif
    sum_low = UINT_BITS(summand1_low + summand2_low);
    if (UINT_HIGHEST_BIT(summand1_low) + UINT_HIGHEST_BIT(summand2_low) +
        UINT_HIGHEST_BIT(UINT_BITS_WITHOUT_HIGHEST_BIT(summand1_low) +
                         UINT_BITS_WITHOUT_HIGHEST_BIT(summand2_low)) >= 2) {
      *sum_high = UINT_BITS(summand1_high + summand2_high + 1);
    } else {
      *sum_high = UINT_BITS(summand1_high + summand2_high);
    } /* if */
#ifdef TRACE_RANDOM
    printf("END uint2_add ==> %08x%08x\n",
        (unsigned int) *sum_high, (unsigned int) sum_low);
#endif
    return sum_low;
  } /* uint2_add */



#ifdef HAS_DOUBLE_INTTYPE
/**
 *  Compute a random unsigned number in the range 0 .. UINTTYPE_MAX.
 *  The linear congruential method is used to generate the random
 *  sequence of uintType numbers. The generator uses double uintType
 *  numbers for the seed. Only the high bits of the seed (high_seed)
 *  are used as random number. This avoids that the lower-order bits
 *  of the generated sequence have a short period.
 *  @return the random number.
 */
uintType uintRand (void)

  { /* uintRand */
#ifdef TRACE_RANDOM
    printf("BEGIN uintRand\n");
#endif
    seed = seed * RAND_MULTIPLIER + RAND_INCREMENT;
#ifdef TRACE_RANDOM
    printf("END uintRand ==> " F_X(08) "\n", (uintType) (seed >> INTTYPE_SIZE));
#endif
    return (uintType) (seed >> INTTYPE_SIZE);
  } /* uintRand */

#else



/**
 *  Compute a random unsigned number in the range 0 .. UINTTYPE_MAX.
 *  The linear congruential method is used to generate the random
 *  sequence of uintType numbers. The generator uses double uintType
 *  numbers for the seed. Only the high bits of the seed (high_seed)
 *  are used as random number. This avoids that the lower-order bits
 *  of the generated sequence have a short period.
 *  @return the random number.
 */
uintType uintRand (void)

  { /* uintRand */
#ifdef TRACE_RANDOM
    printf("BEGIN uintRand\n");
#endif
    /* SEED = SEED * RAND_MULTIPLIER + RAND_INCREMENT */
    low_seed = uint2_mult(high_seed, low_seed, (uintType) INT_SUFFIX(0), (uintType) INT_SUFFIX(RAND_MULTIPLIER),
        &high_seed);
    low_seed = uint2_add(high_seed, low_seed, (uintType) INT_SUFFIX(0), (uintType) INT_SUFFIX(RAND_INCREMENT),
        &high_seed);
#ifdef TRACE_RANDOM
    printf("END uintRand ==> " F_X(08) "\n", high_seed);
#endif
    return high_seed;
  } /* uintRand */
#endif



uintType uintRandLimited (uintType rand_max)

  {
    uintType rand_val;

  /* uintRandLimited */
    do {
      rand_val = uintRand();
    } while (rand_val > rand_max);
    return rand_val;
  } /* uintRandLimited */



static uintType uintGcd (uintType number1, uintType number2)

  {
    uintType temp;

  /* uintGcd */
    if (number2 < number1) {
      temp = number1;
      number1 = number2;
      number2 = temp;
    } /* if */
    while (number2 > 0) {
      temp = number2;
      number2 = number1 % number2;
      number1 = temp;
    } /* while */
    return number1;
  } /* uintGcd */



int uint8MostSignificantBit (uint8Type number)

  {
    int result;

  /* uint8MostSignificantBit */
    result = most_significant[number];
    return result;
  } /* uint8MostSignificantBit */



int uint16MostSignificantBit (uint16Type number)

  {
    int result;

  /* uint16MostSignificantBit */
    if (number > 0xff) {
      result = 8 + most_significant[number >> 8];
    } else {
      result = most_significant[number];
    } /* if */
    return result;
  } /* uint16MostSignificantBit */



int uint32MostSignificantBit (uint32Type number)

  {
    int result;

  /* uint32MostSignificantBit */
    result = 0;
    if (number > 0xffff) {
      number >>= 16;
      result = 16;
    } /* if */
    if (number & 0xff00) {
      number >>= 8;
      result += 8;
    } /* if */
    result += most_significant[number];
    return result;
  } /* uint32MostSignificantBit */



#ifdef INT64TYPE
int uint64MostSignificantBit (uint64Type number)

  {
    int result;

  /* uint64MostSignificantBit */
    result = 0;
    if (number > 0xffffffff) {
      number >>= 32;
      result = 32;
    } /* if */
    if (number & 0xffff0000) {
      number >>= 16;
      result += 16;
    } /* if */
    if (number & 0xff00) {
      number >>= 8;
      result += 8;
    } /* if */
    result += most_significant[number];
    return result;
  } /* uint64MostSignificantBit */
#endif



int uint8LeastSignificantBit (uint8Type number)

  {
    int result;

  /* uint8LeastSignificantBit */
    result = least_significant[number & 0xff];
    return result;
  } /* uint8LeastSignificantBit */



int uint16LeastSignificantBit (uint16Type number)

  {
    int result;

  /* uint16LeastSignificantBit */
    if ((number & 0xff) == 0) {
      result = 8 + least_significant[(number >> 8) & 0xff];
    } else {
      result = least_significant[number & 0xff];
    } /* if */
    return result;
  } /* uint16LeastSignificantBit */



int uint32LeastSignificantBit (uint32Type number)

  {
    int result;

  /* uint32LeastSignificantBit */
    result = 0;
    if ((number & 0xffff) == 0) {
      number >>= 16;
      result = 16;
    } /* if */
    if ((number & 0xff) == 0) {
      number >>= 8;
      result += 8;
    } /* if */
    result += least_significant[number & 0xff];
    return result;
  } /* uint32LeastSignificantBit */



#ifdef INT64TYPE
int uint64LeastSignificantBit (uint64Type number)

  {
    int result;

  /* uint64LeastSignificantBit */
    result = 0;
    if ((number & 0xffffffff) == 0) {
      number >>= 32;
      result = 32;
    } /* if */
    if ((number & 0xffff) == 0) {
      number >>= 16;
      result += 16;
    } /* if */
    if ((number & 0xff) == 0) {
      number >>= 8;
      result += 8;
    } /* if */
    result += least_significant[number & 0xff];
    return result;
  } /* uint64LeastSignificantBit */
#endif



/**
 *  Compare two generic values.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType genericCmp (const genericType value1, const genericType value2)

  { /* genericCmp */
    if (value1 < value2) {
      return -1;
    } else if (value1 > value2) {
      return 1;
    } else {
      return 0;
    } /* if */
  } /* genericCmp */



/**
 *  Generic Copy function to be used via function pointers.
 *  This functions is used to copy values for all types
 *  where a binary copy without special funtionality can be used:
 *  E.g.: intType, floatType, charType, boolType.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. The function genericCpy is
 *  used from hashtables where the keys and the data is stored in
 *  genericType data elements. This assures correct behaviour
 *  even when sizeof(genericType) != sizeof(intType).
 */
void genericCpy (genericType *const dest, const genericType source)

  { /* genericCpy */
    *dest = source;
  } /* genericCpy */



/**
 *  Generic Create function to be used via function pointers.
 *  This functions is used to create new values for all types
 *  where a binary copy without special funtionality can be used:
 *  E.g.: intType, floatType, charType, boolType.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. The function genericCreate is
 *  used from hashtables where the keys and the data is stored in
 *  genericType data elements. This assures correct behaviour
 *  even when sizeof(genericType) != sizeof(intType). Even for
 *  sizeof(genericType) == sizeof(intType) some things must
 *  be considered: On some architectures (linux with gcc)
 *  functions with float results seem to be returned in a
 *  different way (may be another register). Therefore
 *  genericCreate (with genericType) must be used even
 *  when sizeof(genericType) == sizeof(floatType).
 */
genericType genericCreate (genericType source)

  { /* genericCreate */
    return source;
  } /* genericCreate */



/**
 *  Generic Destr function to be used via function pointers.
 *  This functions is used to destroy old values for all types
 *  where a noop without special funtionality can be used:
 *  E.g.: intType, floatType, charType, boolType.
 *  In contrast to prcNoop the number of parameters (1) of
 *  genericDestr is correct. This is important since some
 *  C compilers generate wrong code when prcNoop is used.
 */
void genericDestr (genericType old_value)

  { /* genericDestr */
  } /* genericDestr */



/**
 *  Compare two pointers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType ptrCmp (const void *const value1, const void *const value2)

  { /* ptrCmp */
    if ((memSizeType) value1 < (memSizeType) value2) {
      return -1;
    } else if ((memSizeType) value1 > (memSizeType) value2) {
      return 1;
    } else {
      return 0;
    } /* if */
  } /* ptrCmp */



/**
 *  Reinterpret the generic parameters as rtlPtrType and call ptrCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(rtlPtrType).
 */
intType ptrCmpGeneric (const genericType value1, const genericType value2)

  { /* ptrCmpGeneric */
    return ptrCmp(((const_rtlObjectType *) &value1)->value.ptrValue,
                  ((const_rtlObjectType *) &value2)->value.ptrValue);
  } /* ptrCmpGeneric */



void ptrCpyGeneric (genericType *const dest, const genericType source)

  { /* ptrCpyGeneric */
    ((rtlObjectType *) dest)->value.ptrValue =
        ((const_rtlObjectType *) &source)->value.ptrValue;
  } /* ptrCpyGeneric */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(rtlPtrType).
 */
genericType ptrCreateGeneric (const genericType from_value)

  {
    rtlObjectType result;

  /* ptrCreateGeneric */
    INIT_GENERIC_PTR(result.value.genericValue);
    result.value.ptrValue =
        ((const_rtlObjectType *) &from_value)->value.ptrValue;
    return result.value.genericValue;
  } /* ptrCreateGeneric */



/**
 *  Binomial coefficient
 *  This is equivalent to n! / k! / (n - k)!
 *  This function checks for overflow, otherwise it could not
 *  provide the correct result in all situations. This keeps
 *  a design principle of Seed7 intact: All overflow situations,
 *  where OVERFLOW_ERROR is raised correspond to C situations,
 *  which have undefined behaviour (wrong results). In other
 *  words: Wrong results without overflow checking are only
 *  acceptable when overflow checking would raise OVERFLOW_ERROR.
 *  @return n over k
 *  @exception OVERFLOW_ERROR When an integer overflow occurs.
 */
intType intBinom (intType n_number, intType k_number)

  {
    uintType numerator;
    uintType denominator;
    boolType negative;
    uintType unsigned_result;
    uintType gcd;
    uintType reducedNumerator;
    uintType reducedDenominator;
    intType result;

  /* intBinom */
#ifdef TRACE_INT_RTL
    printf("intBinom(" FMT_D ", " FMT_D ")\n", k_number, n_number);
#endif
    if (n_number >= 0 && k_number > (intType) ((uintType) n_number >> 1)) {
      k_number = n_number - k_number;
    } /* if */
    if (unlikely(k_number <= 1)) {
      if (k_number < 0) {
        result = 0;
      } else if (k_number == 0) {
        result = 1;
      } else {
        result = n_number;
      } /* if */
    } else if (unlikely(n_number == -1)) {
      if (k_number & 1) {
        result = -1;
      } else {
        result = 1;
      } /* if */
    } else {
      if (n_number < 0) {
        negative = k_number & 1;
        numerator = -(uintType) n_number + (uintType) k_number - 1;
        if ((uintType) k_number > numerator >> 1) {
          k_number = (intType) (numerator - (uintType) k_number);
        } /* if */
      } else {
        negative = FALSE;
        numerator = (uintType) n_number;
      } /* if */
      unsigned_result = numerator;
      numerator--;
#if   INTTYPE_SIZE == 32
      if (numerator <= 29) {
#elif INTTYPE_SIZE == 64
      if (numerator <= 61) {
#endif
        for (denominator = 2; denominator <= (uintType) k_number; denominator++, numerator--) {
          unsigned_result *= numerator;
          unsigned_result /= denominator;
        } /* for */
      } else {
        for (denominator = 2; denominator <= (uintType) k_number; denominator++, numerator--) {
          if (unsigned_result > UINTTYPE_MAX / numerator) {
            /* Possible overflow */
            gcd = uintGcd(numerator, denominator);
            reducedNumerator = numerator / gcd;
            reducedDenominator = denominator / gcd;
            gcd = uintGcd(unsigned_result, reducedDenominator);
            unsigned_result = unsigned_result / gcd;
            reducedDenominator = reducedDenominator / gcd;
            if (unsigned_result > UINTTYPE_MAX / reducedNumerator) {
              /* Unavoidable overflow */
              logError(printf(" *** intBinom(" FMT_D ", " FMT_D "): "
                       "Unavoidable overflow.\n", n_number, k_number););
              raise_error(OVERFLOW_ERROR);
              return 0;
            } else {
              unsigned_result *= reducedNumerator;
              unsigned_result /= reducedDenominator;
            } /* if */
          } else {
            unsigned_result *= numerator;
            unsigned_result /= denominator;
          } /* if */
        } /* for */
      } /* if */
      if (negative) {
        if (unsigned_result > -(uintType) INTTYPE_MIN) {
          logError(printf(" *** intBinom(" FMT_D ", " FMT_D "): "
                   "Negative result too small.\n", n_number, k_number););
          raise_error(OVERFLOW_ERROR);
          result = 0;
        } else {
          result = (intType) -unsigned_result;
        } /* if */
      } else {
        if (unsigned_result > INTTYPE_MAX) {
          logError(printf(" *** intBinom(" FMT_D ", " FMT_D "): "
                   "Positive result too big.\n", n_number, k_number););
          raise_error(OVERFLOW_ERROR);
          result = 0;
        } else {
          result = (intType) unsigned_result;
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intBinom --> " FMT_D "\n", result);
#endif
    return result;
  } /* intBinom */



/**
 *  Binomial coefficient
 *  This is equivalent to n! / k! / (n - k)!
 *  This is a special case function that provides the correct
 *  result when n_number is less than a limit.
 *  For 32-bit integers the limit is 30 (n_number <= 30 must hold).
 *  For 64-bit integers the limit is 62 (n_number <= 62 must hold).
 *  @return n over k
 */
uintType uintBinomNoChk (uintType n_number, intType k_number)

  {
    uintType denominator;
    uintType result;

  /* uintBinomNoChk */
#ifdef TRACE_INT_RTL
    printf("uintBinomNoChk(" FMT_D ", " FMT_U ")\n", k_number, n_number);
#endif
    if (k_number > (intType) (n_number >> 1)) {
      k_number = (intType) n_number - k_number;
    } /* if */
    if (unlikely(k_number <= 1)) {
      if (k_number < 0) {
        result = 0;
      } else if (k_number == 0) {
        result = 1;
      } else {
        result = n_number;
      } /* if */
    } else {
      result = n_number;
      n_number--;
      for (denominator = 2; denominator <= (uintType) k_number; denominator++, n_number--) {
        result *= n_number;
        result /= denominator;
      } /* for */
    } /* if */
#ifdef TRACE_INT_RTL
    printf("uintBinomNoChk --> " FMT_U "\n", result);
#endif
    return result;
  } /* uintBinomNoChk */



/**
 *  Number of bits in the minimal two's-complement representation.
 *  The high bits equivalent to the sign bit are not part of the
 *  minimal two's-complement representation.
 *  @return the number of bits.
 */
intType intBitLength (intType number)

  {
    intType result;

  /* intBitLength */
    if (number < 0) {
      number = ~number;
    } /* if */
    result = uintMostSignificantBit((uintType) number) + 1;
    return result;
  } /* intBitLength */



/**
 *  Convert an integer into a big-endian string of bytes.
 *  The result uses binary representation with a base of 256.
 *  The result contains chars (bytes) with an ordinal <= 255.
 *  @param number Integer number to be converted.
 *  @param isSigned Determines the signedness of the result.
 *         When 'isSigned' is TRUE the result is encoded with the
 *         twos-complement representation. In this case a negative
 *         'number' is converted to a result where the most significant
 *         byte has an ordinal >= 128.
 *  @return a string with the shortest binary representation of 'number'.
 *  @exception RANGE_ERROR When 'number' is negative and 'isSigned' is FALSE.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType intBytesBe (intType number, boolType isSigned)

  {
    strElemType buffer[BYTE_BUFFER_SIZE];
    unsigned int pos = BYTE_BUFFER_SIZE;
    striType result;

  /* intBytesBe */
#ifdef TRACE_INT_RTL
    printf("intBytesBe(" FMT_D ", %d)\n", number, isSigned);
#endif
    if (number >= 0) {
      do {
        pos--;
        buffer[pos] = (strElemType) (number & 0xff);
        number >>= 8;
      } while (number != 0);
      if (isSigned && buffer[pos] >= 128) {
        pos--;
        buffer[pos] = 0;
      } /* if */
    } else if (isSigned) {
      do {
        pos--;
        buffer[pos] = (strElemType) (number & 0xff);
#ifdef RSHIFT_DOES_SIGN_EXTEND
        number >>= 8;
#else
        number = ~(~number >> 8);
#endif
      } while (number != -1);
      if (buffer[pos] <= 127) {
        pos--;
        buffer[pos] = 255;
      } /* if */
    } else {
      logError(printf(" *** intBytesBe(" FMT_D ", %d): "
                      "Negative number and isSigned is FALSE.\n",
                      number, isSigned););
      raise_error(RANGE_ERROR);
      return NULL;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memSizeType) (BYTE_BUFFER_SIZE - pos)))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = (memSizeType) (BYTE_BUFFER_SIZE - pos);
      memcpy(result->mem, &buffer[pos],
             (memSizeType) (BYTE_BUFFER_SIZE - pos) * sizeof(strElemType));
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intBytesBe --> ");
    prot_stri(result);
    printf("\n");
#endif
    return result;
  } /* intBytesBe */



/**
 *  Convert a string of bytes (interpreted as big-endian) to an integer.
 *  @param byteStri String of bytes to be converted. The bytes are
 *         interpreted as binary big-endian representation with a
 *         base of 256.
 *  @param isSigned Determines the signedness of 'byteStri'.
 *         When 'isSigned' is TRUE 'byteStri' is interpreted as
 *         signed value in the twos-complement representation.
 *         In this case the result is negative when the most significant
 *         byte (the first byte) has an ordinal >= 128.
 *  @return an integer created from 'byteStri'.
 *  @exception RANGE_ERROR When characters beyond '\255;' are present or
 *             when the result value cannot be represented with an integer.
 */
intType intBytesBe2Int (const const_striType byteStri, boolType isSigned)

  {
    memSizeType pos = 0;
    intType result = 0;

  /* intBytesBe2Int */
#ifdef TRACE_INT_RTL
    printf("intBytesBe2Int(");
    prot_stri(byteStri);
    printf(", %d)\n", isSigned);
#endif
    if (!isSigned || byteStri->size == 0 || byteStri->mem[0] <= 127) {
      if (unlikely(byteStri->size >= sizeof(intType))) {
        while (pos < byteStri->size && byteStri->mem[pos] == 0) {
          pos++;
        } /* if */
        if (unlikely(byteStri->size - pos > sizeof(intType) ||
                     (byteStri->size - pos == sizeof(intType) &&
                      byteStri->mem[pos] >= 128))) {
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } /* if */
    } else { /* isSigned && byteStri->size != 0 && byteStri->mem[0] >= 128 */
      if (unlikely(byteStri->size >= sizeof(intType))) {
        while (pos < byteStri->size && byteStri->mem[pos] == 255) {
          pos++;
        } /* if */
        if (unlikely(byteStri->size - pos > sizeof(intType) ||
                     (byteStri->size - pos == sizeof(intType) &&
                      byteStri->mem[pos] <= 127))) {
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } /* if */
      result = -1;
    } /* if */
    for (; pos < byteStri->size; pos++) {
      if (unlikely(byteStri->mem[pos] >= 256)) {
        raise_error(RANGE_ERROR);
        return 0;
      } /* if */
      result <<= 8;
      result += byteStri->mem[pos];
    } /* for */
#ifdef TRACE_INT_RTL
    printf("intBytesBe2Int --> " FMT_D "\n", result);
#endif
    return result;
  } /* intBytesBe2Int */



/**
 *  Convert an integer into a little-endian string of bytes.
 *  The result uses binary representation with a base of 256.
 *  The result contains chars (bytes) with an ordinal <= 255.
 *  @param number Integer number to be converted.
 *  @param isSigned Determines the signedness of the result.
 *         When 'isSigned' is TRUE the result is encoded with the
 *         twos-complement representation. In this case a negative
 *         'number' is converted to a result where the most significant
 *         byte has an ordinal >= 128.
 *  @return a string with the shortest binary representation of 'number'.
 *  @exception RANGE_ERROR When 'number' is negative and 'isSigned' is FALSE.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType intBytesLe (intType number, boolType isSigned)

  {
    strElemType buffer[BYTE_BUFFER_SIZE];
    unsigned int pos = 0;
    striType result;

  /* intBytesLe */
#ifdef TRACE_INT_RTL
    printf("intBytesLe(" FMT_D ", %d)\n", number, isSigned);
#endif
    if (number >= 0) {
      do {
        buffer[pos] = (ucharType) (number & 0xff);
        number >>= 8;
        pos++;
      } while (number != 0);
      if (isSigned && buffer[pos - 1] >= 128) {
        buffer[pos] = 0;
        pos++;
      } /* if */
    } else if (isSigned) {
      do {
        buffer[pos] = (ucharType) (number & 0xff);
#ifdef RSHIFT_DOES_SIGN_EXTEND
        number >>= 8;
#else
        number = ~(~number >> 8);
#endif
        pos++;
      } while (number != -1);
      if (buffer[pos - 1] <= 127) {
        buffer[pos] = 255;
        pos++;
      } /* if */
    } else {
      logError(printf(" *** intBytesBe(" FMT_D ", %d): "
                      "Negative number and isSigned is FALSE.\n",
                      number, isSigned););
      raise_error(RANGE_ERROR);
      return NULL;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memSizeType) (pos)))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = (memSizeType) (pos);
      memcpy(result->mem, &buffer[0],
             (memSizeType) pos * sizeof(strElemType));
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intBytesLe --> ");
    prot_stri(result);
    printf("\n");
#endif
    return result;
  } /* intBytesLe */



/**
 *  Convert a string of bytes (interpreted as little-endian) to an integer.
 *  @param byteStri String of bytes to be converted. The bytes are
 *         interpreted as binary little-endian representation with a
 *         base of 256.
 *  @param isSigned Determines the signedness of 'byteStri'.
 *         When 'isSigned' is TRUE 'byteStri' is interpreted as
 *         signed value in the twos-complement representation.
 *         In this case the result is negative when the most significant
 *         byte (the last byte) has an ordinal >= 128.
 *  @return an integer created from 'byteStri'.
 *  @exception RANGE_ERROR When characters beyond '\255;' are present or
 *             when the result value cannot be represented with an integer.
 */
intType intBytesLe2Int (const const_striType byteStri, boolType isSigned)

  {
    memSizeType pos;
    intType result = 0;

  /* intBytesLe2Int */
#ifdef TRACE_INT_RTL
    printf("intBytesLe2Int(");
    prot_stri(byteStri);
    printf(", %d)\n", isSigned);
#endif
    pos = byteStri->size;
    if (!isSigned || byteStri->size == 0 || byteStri->mem[pos - 1] <= 127) {
      if (unlikely(byteStri->size >= sizeof(intType))) {
        while (pos > 0 && byteStri->mem[pos - 1] == 0) {
          pos--;
        } /* if */
        if (unlikely(pos > sizeof(intType) ||
                     (pos == sizeof(intType) &&
                      byteStri->mem[pos - 1] >= 128))) {
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } /* if */
    } else { /* isSigned && byteStri->size != 0 && byteStri->mem[pos - 1] >= 128 */
      if (unlikely(byteStri->size >= sizeof(intType))) {
        while (pos > 0 && byteStri->mem[pos - 1] == 255) {
          pos--;
        } /* if */
        if (unlikely(pos > sizeof(intType) ||
                     (pos == sizeof(intType) &&
                      byteStri->mem[pos - 1] <= 127))) {
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } /* if */
      result = -1;
    } /* if */
    for (; pos > 0; pos--) {
      if (unlikely(byteStri->mem[pos - 1] >= 256)) {
        raise_error(RANGE_ERROR);
        return 0;
      } /* if */
      result <<= 8;
      result += byteStri->mem[pos - 1];
    } /* for */
#ifdef TRACE_INT_RTL
    printf("intBytesLe2Int --> " FMT_D "\n", result);
#endif
    return result;
  } /* intBytesLe2Int */



/**
 *  Compare two integer numbers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType intCmp (intType number1, intType number2)

  { /* intCmp */
    if (number1 < number2) {
      return -1;
    } else if (number1 > number2) {
      return 1;
    } else {
      return 0;
    } /* if */
  } /* intCmp */



/**
 *  Reinterpret the generic parameters as intType and call intCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(intType).
 */
intType intCmpGeneric (const genericType value1, const genericType value2)

  { /* intCmpGeneric */
    return intCmp(((const_rtlObjectType *) &value1)->value.intValue,
                  ((const_rtlObjectType *) &value2)->value.intValue);
  } /* intCmpGeneric */



/**
 *  Compute the truncated base 10 logarithm of an integer number.
 *  The definition of intLog10 is extended by defining intLog10(0) = -1.
 *  @return the truncated base 10 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
intType intLog10 (intType number)

  {
    int logarithm;

  /* intLog10 */
#ifdef TRACE_INT_RTL
    printf("intLog10(" FMT_D ")\n", number);
#endif
    if (unlikely(number < 0)) {
      logError(printf(" *** intLog10(" FMT_D "): Number is negative.\n",
                      number););
      raise_error(NUMERIC_ERROR);
      logarithm = 0;
    } else if (number == 0) {
      logarithm = -1;
    } else {
      logarithm = DECIMAL_DIGITS((uintType) number) - 1;
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intLog10 --> %d\n", logarithm);
#endif
    return (intType) logarithm;
  } /* intLog10 */



/**
 *  Compute the truncated base 2 logarithm of an integer number.
 *  The definition of intLog2 is extended by defining intLog2(0) = -1.
 *  @return the truncated base 2 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
intType intLog2 (intType number)

  {
    int logarithm;

  /* intLog2 */
#ifdef TRACE_INT_RTL
    printf("intLog2(" FMT_D ")\n", number);
#endif
    if (unlikely(number < 0)) {
      logError(printf(" *** intLog2(" FMT_D "): Number is negative.\n",
                      number););
      raise_error(NUMERIC_ERROR);
      logarithm = 0;
    } else {
      logarithm = uintMostSignificantBit((uintType) number);
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intLog2 --> %d\n", logarithm);
#endif
    return (intType) logarithm;
  } /* intLog2 */



/**
 *  Index of the lowest-order one bit.
 *  For A <> 0 this is equal to the number of lowest-order zero bits.
 *  @return the number of lowest-order zero bits or -1 for lowestSetBit(0).
 */
intType intLowestSetBit (intType number)

  {
    intType result;

  /* intLowestSetBit */
    if (number == 0) {
      result = -1;
    } else {
      result = uintLeastSignificantBit((uintType) number);
    } /* if */
    return result;
  } /* intLowestSetBit */



striType intLpad0 (intType number, const intType pad_size)

  {
    uintType unsigned_number;
    boolType negative;
    strElemType *buffer;
    memSizeType length;
    memSizeType result_size;
    striType result;

  /* intLpad0 */
    negative = (number < 0);
    if (negative) {
      unsigned_number = (uintType) -number;
    } else {
      unsigned_number = (uintType) number;
    } /* if */
    length = DECIMAL_DIGITS(unsigned_number);
    if (negative) {
      length++;
    } /* if */
    if (pad_size > (intType) length) {
      if (unlikely((uintType) pad_size > MAX_STRI_LEN)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        result_size = (memSizeType) pad_size;
      } /* if */
    } else {
      result_size = length;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = result_size;
      buffer = &result->mem[result_size];
      do {
        *(--buffer) = (strElemType) (unsigned_number % 10 + '0');
      } while ((unsigned_number /= 10) != 0);
      if (buffer != result->mem) {
        while (buffer != &result->mem[1]) {
          *(--buffer) = (strElemType) '0';
        } /* while */
        if (negative) {
          result->mem[0] = (strElemType) '-';
        } else {
          result->mem[0] = (strElemType) '0';
        } /* if */
      } /* if */
      return result;
    } /* if */
  } /* intLpad0 */



/**
 *  Convert a string to an integer number.
 *  The string must contain an integer literal consisting of an
 *  optional + or - sign, followed by a sequence of digits. Other
 *  characters as well as leading or trailing whitespace characters
 *  are not allowed. The sequence of digits is taken to be decimal.
 *  @return the integer result of the conversion.
 *  @exception RANGE_ERROR When the string is empty or it does not contain
 *             an integer literal or when the integer literal is too big
 *             or too small to be represented as integer value.
 */
intType intParse (const const_striType stri)

  {
    boolType okay = TRUE;
    boolType negative = FALSE;
    memSizeType position = 0;
    uintType digitval;
    uintType uint_value = 0;
    intType int_value = 0;

  /* intParse */
#ifdef TRACE_INT_RTL
    printf("intParse(");
    prot_stri(stri);
    printf(")\n");
#endif
    if (stri->size != 0) {
      if (stri->mem[0] == ((strElemType) '-')) {
        negative = TRUE;
        position++;
      } else {
        if (stri->mem[0] == ((strElemType) '+')) {
          position++;
        } /* if */
      } /* if */
      while (position < stri->size &&
          stri->mem[position] >= ((strElemType) '0') &&
          stri->mem[position] <= ((strElemType) '9')) {
        digitval = ((uintType) stri->mem[position]) - ((uintType) '0');
        if (uint_value <= MAX_DIV_10) {
          uint_value = ((uintType) 10) * uint_value + digitval;
        } else {
          okay = FALSE;
        } /* if */
        position++;
      } /* while */
    } /* if */
    if (position == 0 || position < stri->size) {
      okay = FALSE;
    } /* if */
    if (likely(okay)) {
      if (negative) {
        if (uint_value > (uintType) INTTYPE_MAX + 1) {
          okay = FALSE;
        } else {
          int_value = (intType) -uint_value;
        } /* if */
      } else if (uint_value > (uintType) INTTYPE_MAX) {
        okay = FALSE;
      } else {
        int_value = (intType) uint_value;
      } /* if */
    } /* if */
    if (unlikely(!okay)) {
      raise_error(RANGE_ERROR);
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intParse --> " FMT_D "\n", int_value);
#endif
    return int_value;
  } /* intParse */



/**
 *  Compute the exponentiation of a integer base with an integer exponent.
 *  @return the result of the exponentation.
 *  @exception NUMERIC_ERROR When the exponent is negative.
 */
intType intPow (intType base, intType exponent)

  {
    intType power;

  /* intPow */
#ifdef TRACE_INT_RTL
    printf("intPow(" FMT_D ", " FMT_D ")\n", base, exponent);
#endif
    if (unlikely(exponent < 0)) {
      logError(printf(" *** intPow(" FMT_D ", " FMT_D "): "
                      "Exponent is negative.\n",
                      base, exponent););
      raise_error(NUMERIC_ERROR);
      power = 0;
    } else {
      if (exponent & 1) {
        power = base;
      } else {
        power = 1;
      } /* if */
      exponent >>= 1;
      while (exponent != 0) {
        base *= base;
        if (exponent & 1) {
          power *= base;
        } /* if */
        exponent >>= 1;
      } /* while */
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intPow --> " FMT_D "\n", power);
#endif
    return power;
  } /* intPow */



/**
 *  Compute the exponentiation of a integer base with an integer exponent.
 *  @return the result of the exponentation.
 *  @exception NUMERIC_ERROR When the exponent is negative.
 *  @exception OVERFLOW_ERROR When an integer overflow occurs.
 */
intType intPowOvfChk (intType base, intType exponent)

  {
    intType power;

  /* intPowOvfChk */
#ifdef TRACE_INT_RTL
    printf("intPowOvfChk(" FMT_D ", " FMT_D ")\n", base, exponent);
#endif
    if (unlikely(exponent < 0)) {
      logError(printf(" *** intPowOvfChk(" FMT_D ", " FMT_D "): "
                      "Exponent is negative.\n",
                      base, exponent););
      raise_error(NUMERIC_ERROR);
      power = 0;
    } else {
      if ((uintType) exponent < sizeof(minBaseOfExponent) / sizeof(intType)) {
        if (unlikely(base < minBaseOfExponent[exponent] ||
                     base > maxBaseOfExponent[exponent])) {
          logError(printf(" *** intPowOvfChk(" FMT_D ", " FMT_D "): "
                          "Base wrong: ",
                          base, exponent);
                   if (base < minBaseOfExponent[exponent]) {
                     printf(FMT_D " < " FMT_D "\n",
                            base, minBaseOfExponent[exponent]);
                   } else {
                     printf(FMT_D " > " FMT_D "\n",
                            base, maxBaseOfExponent[exponent]);
                   });
          raise_error(OVERFLOW_ERROR);
          return 0;
        } /* if */
      } else if (unlikely(base < -8 || base > 8 ||
                          exponent > maxExponentOfBase[base + 8])) {
        logError(printf(" *** intPowOvfChk(" FMT_D ", " FMT_D "): ",
                        base, exponent);
                 if (base < -8) {
                   printf("Base wrong: " FMT_D " < -8\n", base);
                 } else if (base > 8) {
                   printf("Base wrong: " FMT_D " > 8\n", base);
                 } else {
                   printf("Exponent wrong: " FMT_D " > " FMT_D "\n",
                          exponent, maxExponentOfBase[base + 8]);
                 });
        raise_error(OVERFLOW_ERROR);
        return 0;
      } /* if */
      if (exponent & 1) {
        power = base;
      } else {
        power = 1;
      } /* if */
      exponent >>= 1;
      while (exponent != 0) {
        base *= base;
        if (exponent & 1) {
          power *= base;
        } /* if */
        exponent >>= 1;
      } /* while */
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intPowOvfChk --> " FMT_D "\n", power);
#endif
    return power;
  } /* intPowOvfChk */



/**
 *  Convert an integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with letters.
 *  For negative numbers a minus sign is prepended.
 *  @param upperCase Decides about the letter case.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR When base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType intRadix (intType number, intType base, boolType upperCase)

  {
    uintType unsigned_number;
    boolType negative;
    const_cstriType digits;
    strElemType buffer_1[RADIX_BUFFER_SIZE];
    strElemType *buffer;
    memSizeType length;
    striType result;

  /* intRadix */
#ifdef TRACE_INT_RTL
    printf("intRadix(" FMT_D ", " FMT_D ", %d)\n",
           number, base, upperCase);
#endif
    if (unlikely(base < 2 || base > 36)) {
      logError(printf(" *** intRadix(" FMT_D ", " FMT_D ", %d): "
                      "base < 2 or base > 36.\n",
                      number, base, upperCase););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      negative = (number < 0);
      if (negative) {
        /* The unsigned value is negated to avoid a signed integer */
        /* overflow when the smallest signed integer is negated.   */
        unsigned_number = -(uintType) number;
      } else {
        unsigned_number = (uintType) number;
      } /* if */
      digits = digitTable[upperCase];
      buffer = &buffer_1[RADIX_BUFFER_SIZE];
      do {
        *(--buffer) = (strElemType) (digits[unsigned_number % (uintType) base]);
      } while ((unsigned_number /= (uintType) base) != 0);
      if (negative) {
        *(--buffer) = (strElemType) '-';
      } /* if */
      length = (memSizeType) (&buffer_1[RADIX_BUFFER_SIZE] - buffer);
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = length;
        memcpy(result->mem, buffer, (size_t) (length * sizeof(strElemType)));
      } /* if */
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intRadix --> ");
    prot_stri(result);
    printf("\n");
#endif
    return result;
  } /* intRadix */



/**
 *  Convert an integer number to a string using a radix.
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
striType intRadixPow2 (intType number, int shift, int mask, boolType upperCase)

  {
    uintType unsigned_number;
    boolType negative;
    const_cstriType digits;
    strElemType buffer_1[RADIX_BUFFER_SIZE];
    strElemType *buffer;
    memSizeType length;
    striType result;

  /* intRadixPow2 */
#ifdef TRACE_INT_RTL
    printf("intRadixPow2(" FMT_D ", %d, %x, %d)\n",
           number, shift, mask, upperCase);
#endif
    negative = (number < 0);
    if (negative) {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow when the smallest signed integer is negated.   */
      unsigned_number = -(uintType) number;
    } else {
      unsigned_number = (uintType) number;
    } /* if */
    digits = digitTable[upperCase];
    buffer = &buffer_1[RADIX_BUFFER_SIZE];
    do {
      *(--buffer) = (strElemType) (digits[unsigned_number & (uintType) mask]);
    } while ((unsigned_number >>= shift) != 0);
    if (negative) {
      *(--buffer) = (strElemType) '-';
    } /* if */
    length = (memSizeType) (&buffer_1[RADIX_BUFFER_SIZE] - buffer);
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = length;
      memcpy(result->mem, buffer, (size_t) (length * sizeof(strElemType)));
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intRadixPow2 --> ");
    prot_stri(result);
    printf("\n");
#endif
    return result;
  } /* intRadixPow2 */



/**
 *  Compute pseudo-random number in the range [low, high].
 *  The random values are uniform distributed.
 *  @return a random number such that low <= rand(low, high) and
 *          rand(low, high) <= high holds.
 *  @exception RANGE_ERROR The range is empty (low > high holds).
 */
intType intRand (intType low, intType high)

  {
    uintType scale_limit;
    uintType rand_max;
    uintType rand_val;
    intType randomNumber;

  /* intRand */
#ifdef TRACE_INT_RTL
    printf("intRand(" FMT_D ", " FMT_D ")\n", low, high);
#endif
    if (unlikely(low >= high)) {
      if (low == high) {
        randomNumber = low;
      } else {
        raise_error(RANGE_ERROR);
        randomNumber = 0;
      } /* if */
    } else {
      scale_limit = (uintType) high - (uintType) low;
      if (scale_limit == UINTTYPE_MAX) {
        /* low must be INTTYPE_MIN and high must be INTTYPE_MAX. */
        randomNumber = (intType) uintRand();
      } else {
        scale_limit++;
        /* scale_limit <= UINTTYPE_MAX holds. */
        rand_max = UINTTYPE_MAX - ((UINTTYPE_MAX - scale_limit + 1) % scale_limit);
        do {
          rand_val = uintRand();
        } while (rand_val > rand_max);
        randomNumber = (intType) ((uintType) low + rand_val % scale_limit);
      } /* if */
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intRand --> " FMT_D "\n", randomNumber);
#endif
    return randomNumber;
  } /* intRand */



/**
 *  Multiply two integer numbers.
 *  @return the product of the two numbers.
 *  @exception OVERFLOW_ERROR When an integer overflow occurs.
 */
intType intMultOvfChk (intType factor1, intType factor2)

  {
    intType product;

  /* intMultOvfChk */
    if (factor1 < 0) {
      if (factor2 < 0) {
        if (unlikely(factor1 < INTTYPE_MAX / factor2)) {
          logError(printf(" *** intMultOvfChk(" FMT_D ", " FMT_D "): "
                   "factor1 < " FMT_D "\n",
                   factor1, factor2, INTTYPE_MAX / factor2););
          raise_error(OVERFLOW_ERROR);
          return 0;
        } /* if */
      } else if (factor2 != 0) {
        if (unlikely(factor1 < INTTYPE_MIN / factor2)) {
          logError(printf(" *** intMultOvfChk(" FMT_D ", " FMT_D "): "
                   "factor1 < " FMT_D "\n",
                   factor1, factor2, INTTYPE_MIN / factor2););
          raise_error(OVERFLOW_ERROR);
          return 0;
        } /* if */
      } /* if */
    } else if (factor1 != 0) {
      if (factor2 < 0) {
        if (unlikely(factor2 < INTTYPE_MIN / factor1)) {
          logError(printf(" *** intMultOvfChk(" FMT_D ", " FMT_D "): "
                   "factor2 < " FMT_D "\n",
                   factor1, factor2, INTTYPE_MIN / factor1););
          raise_error(OVERFLOW_ERROR);
          return 0;
        } /* if */
      } else if (factor2 != 0) {
        if (unlikely(factor2 > INTTYPE_MAX / factor1)) {
          logError(printf(" *** intMultOvfChk(" FMT_D ", " FMT_D "): "
                   "factor2 > " FMT_D "\n",
                   factor1, factor2, INTTYPE_MAX / factor1););
          raise_error(OVERFLOW_ERROR);
          return 0;
        } /* if */
      } /* if */
    } /* if */
    product = factor1 * factor2;
    return product;
  } /* intMultOvfChk */



/**
 *  Compute the integer square root of an integer number.
 *  @return the integer square root.
 *  @exception NUMERIC_ERROR When number is negative.
 */
intType intSqrt (intType number)

  {
    register uintType root;
    register uintType root2;

  /* intSqrt */
#ifdef TRACE_INT_RTL
    printf("intSqrt(" FMT_D ")\n", number);
#endif
    if (unlikely(number < 0)) {
      logError(printf(" *** intSqrt(" FMT_D "): Number is negative.\n",
                      number););
      raise_error(NUMERIC_ERROR);
      root = 0;
    } else if (number == 0) {
      root = 0;
    } else {
      root2 = (uintType) number;
      do {
        root = root2;
        root2 = (root + (uintType) number / root) >> 1;
      } while (root > root2);
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intSqrt --> " FMT_U "\n", root);
#endif
    return (intType) root;
  } /* intSqrt */



/**
 *  Convert an integer number to a string.
 *  The number is converted to a string with decimal representation.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType intStr (intType number)

  {
    register uintType unsigned_number;
    boolType negative;
    strElemType *buffer;
    memSizeType length;
    striType result;

  /* intStr */
#ifdef TRACE_INT_RTL
    printf("intStr(" FMT_D ")\n", number);
#endif
    negative = (number < 0);
    if (negative) {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow when the smallest signed integer is negated.   */
      unsigned_number = -(uintType) number;
    } else {
      unsigned_number = (uintType) number;
    } /* if */
    length = DECIMAL_DIGITS(unsigned_number);
    if (negative) {
      length++;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = length;
      buffer = &result->mem[length];
      do {
        *(--buffer) = (strElemType) (unsigned_number % 10 + '0');
      } while ((unsigned_number /= 10) != 0);
      if (negative) {
        result->mem[0] = (strElemType) '-';
      } /* if */
    } /* if */
#ifdef TRACE_INT_RTL
    printf("intStr --> ");
    prot_stri(result);
    printf("\n");
#endif
    return result;
  } /* intStr */



#ifdef ALLOW_STRITYPE_SLICES
striType intStrToBuffer (intType number, striType buffer)

  {
    register uintType unsigned_number;
    boolType negative;
    strElemType *bufferPtr;

  /* intStrToBuffer */
#ifdef TRACE_INT_RTL
    printf("intStrToBuffer(" FMT_D ")\n", number);
#endif
    negative = (number < 0);
    if (negative) {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow when the smallest signed integer is negated.   */
      unsigned_number = -(uintType) number;
    } else {
      unsigned_number = (uintType) number;
    } /* if */
    bufferPtr = &buffer->mem1[INTTYPE_DECIMAL_SIZE];
    do {
      *(--bufferPtr) = (strElemType) (unsigned_number % 10 + '0');
    } while ((unsigned_number /= 10) != 0);
    if (negative) {
      *(--bufferPtr) = (strElemType) '-';
    } /* if */
    buffer->mem = bufferPtr;
    buffer->size = (memSizeType) (&buffer->mem1[INTTYPE_DECIMAL_SIZE] - bufferPtr);
#ifdef TRACE_INT_RTL
    printf("intStrToBuffer --> ");
    prot_stri(buffer);
    printf("\n");
#endif
    return buffer;
  } /* intStrToBuffer */
#endif
