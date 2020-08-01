/********************************************************************/
/*                                                                  */
/*  int_rtl.c     Primitive actions for the integer type.           */
/*  Copyright (C) 1989 - 2016  Thomas Mertes                        */
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
/*  Changes: 1992 - 1994, 2000, 2005, 2009 - 2015  Thomas Mertes    */
/*  Content: Primitive actions for the integer type.                */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "limits.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "tim_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "int_rtl.h"


#define UINT_BITS(A)                     ((A) & UINTTYPE_MAX)
#define UINT_BITS_WITHOUT_HIGHEST_BIT(A) ((A) & (UINTTYPE_MAX >> 1))
#define UINT_HIGHEST_BIT(A)              ((A) >> (INTTYPE_SIZE - 1))
#define UINT_LOWER_HALF_BITS_SET         (UINTTYPE_MAX >> (INTTYPE_SIZE >> 1))
#define LOWER_HALF_OF_UINT(A)            ((A) & UINT_LOWER_HALF_BITS_SET)
#define UPPER_HALF_OF_UINT(A)            ((A) >> (INTTYPE_SIZE >> 1))

/**
 *  Number of characters needed when the most negative
 *  integer is written with radix 2. One character is
 *  needed for the sign. Assume that integers are just
 *  signed bytes. In this case the most negative number
 *  would be -128. -128 radix 2  returns  "-10000000"
 *  The result needs 9 digits although -128 fits into a
 *  signed byte with 8 bits.
 */
#define RADIX_BUFFER_SIZE (CHAR_BIT * sizeof(intType) + 1)

#define BYTE_BUFFER_SIZE sizeof(intType)

#if   INTTYPE_SIZE == 32
#define DECIMAL_DIGITS(num) \
  (num < UINT_SUFFIX(100000000) ?               \
      (num < UINT_SUFFIX(10000) ?               \
        (num < UINT_SUFFIX(100) ?               \
          (num < UINT_SUFFIX(10) ? 1 : 2)       \
        :                                       \
          (num < UINT_SUFFIX(1000) ? 3 : 4)     \
        )                                       \
      :                                         \
        (num < UINT_SUFFIX(1000000) ?           \
          (num < UINT_SUFFIX(100000) ? 5 : 6)   \
        :                                       \
          (num < UINT_SUFFIX(10000000) ? 7 : 8) \
        )                                       \
      )                                         \
    :                                           \
      (num < UINT_SUFFIX(1000000000) ? 9 : 10)  \
    )
#elif INTTYPE_SIZE == 64
#define DECIMAL_DIGITS(num) \
    (num < UINT_SUFFIX(10000000000000000) ?                 \
      (num < UINT_SUFFIX(100000000) ?                       \
        (num < UINT_SUFFIX(10000) ?                         \
          (num < UINT_SUFFIX(100) ?                         \
            (num < UINT_SUFFIX(10) ? 1 : 2)                 \
          :                                                 \
            (num < UINT_SUFFIX(1000) ? 3 : 4)               \
          )                                                 \
        :                                                   \
          (num < UINT_SUFFIX(1000000) ?                     \
            (num < UINT_SUFFIX(100000) ? 5 : 6)             \
          :                                                 \
            (num < UINT_SUFFIX(10000000) ? 7 : 8)           \
          )                                                 \
        )                                                   \
      :                                                     \
        (num < UINT_SUFFIX(1000000000000) ?                 \
          (num < UINT_SUFFIX(10000000000) ?                 \
            (num < UINT_SUFFIX(1000000000) ? 9 : 10)        \
          :                                                 \
            (num < UINT_SUFFIX(100000000000) ? 11 : 12)     \
          )                                                 \
        :                                                   \
          (num < UINT_SUFFIX(100000000000000) ?             \
            (num < UINT_SUFFIX(10000000000000) ? 13 : 14)   \
          :                                                 \
            (num < UINT_SUFFIX(1000000000000000) ? 15 : 16) \
          )                                                 \
        )                                                   \
      )                                                     \
    :                                                       \
      (num < UINT_SUFFIX(1000000000000000000) ?             \
        (num < UINT_SUFFIX(100000000000000000) ? 17 : 18)   \
      :                                                     \
        (num < UINT_SUFFIX(10000000000000000000) ? 19 : 20) \
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

/**
 *  Table to support the overflow checking of intPowOvfChk().
 *  For a base between -8 and 8 use maxExponentOfBase[base + 8]
 *  to determine the maximum exponent. When the exponent is
 *  between 0 and maxExponentOfBase[base + 8] the expression
 *  base ** exponent can be computed without overflow.
 */
static const intType maxExponentOfBase[] = {
    21, 22, 24, 27, 31, 39, 63,
    INTTYPE_MAX, INTTYPE_MAX, INTTYPE_MAX,
    62, 39, 31, 27, 24, 22, 20
  };

/**
 *  Table to support the overflow checking of intPowOvfChk().
 *  For an exponent between 0 and 22 minBaseOfExponent[exponent]
 *  is used to detemine the minimum base. When a base is between
 *  minBaseOfExponent[exponent] and maxBaseOfExponent[exponent] the
 *  expression base ** exponent can be computed without overflow.
 */
static const intType minBaseOfExponent[] = {
    INTTYPE_MIN, INTTYPE_MIN,
    -INT_SUFFIX(3037000499), -2097152, -55108, -6208, -1448, -512, -234,
    -128, -78, -52, -38, -28, -22, -18, -15, -13, -11, -9, -8, -8, -7
  };

/**
 *  Table to support the overflow checking of intPowOvfChk().
 *  For an exponent between 0 and 22 maxBaseOfExponent[exponent]
 *  is used to detemine the maximum base. When a base is between
 *  minBaseOfExponent[exponent] and maxBaseOfExponent[exponent] the
 *  expression base ** exponent can be computed without overflow.
 */
static const intType maxBaseOfExponent[] = {
    INTTYPE_MAX, INTTYPE_MAX,
    INT_SUFFIX(3037000499), 2097151, 55108, 6208, 1448, 511, 234,
    127, 78, 52, 38, 28, 22, 18, 15, 13, 11, 9, 8, 7, 7
};

const const_ustriType digitTable[] = {
    (const_ustriType) "0123456789abcdefghijklmnopqrstuvwxyz",
    (const_ustriType) "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  };

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



/**
 *  Initialize the seed of the random number generator.
 *  Data from the current time and from the clock() function is used
 *  to set up the random number generator. This function is called
 *  once from the interpreter or from the main function of a compiled
 *  program.
 */
void setupRand (void)

  {
    uintType micro_sec;
#ifdef HAS_DOUBLE_INTTYPE
    uintType low_seed;
    uintType high_seed;
#endif

  /* setupRand */
    logFunction(printf("setupRand()\n"););
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



/**
 *  Multiply two uintType factors to a double uintType product.
 *  The whole product fits into the double uintType number.
 *  The product is returned in product_high and product_low.
 *  A double uintType number consists of a low and a high uintType
 *  number. A double uintType number can also be split into
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
uintType uintMult (uintType factor1, uintType factor2, uintType *product_high)

  {
    uintType factor1_part[2];  /* parts 2 and 3 are not used */
    uintType factor2_part[2];  /* parts 2 and 3 are not used */
    uintType c1;  /* memory layout:   | part[1] | part[0] |  */
    uintType c2;  /* memory layout:   | part[2] | part[1] |  */
    uintType c3;  /* memory layout:   | part[2] | part[1] |  */
    uintType c4;  /* memory layout:   | part[2] | part[1] |  */
    uintType c5;  /* memory layout:   | part[3] | part[2] |  */
    uintType product_low;

  /* uintMult */
    logFunction(printf("uintMult(" F_X(08) ", " F_X(08) ")\n",
                       factor1, factor2););
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
    logFunction(printf("uintMult --> " F_X(08) F_X(08) "\n",
                       *product_high, product_low););
    return product_low;
  } /* uintMult */



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
    logFunction(printf("uintRand\n"););
    seed = seed * RAND_MULTIPLIER + RAND_INCREMENT;
    logFunction(printf("uintRand --> " F_X(08) "\n",
                       (uintType) (seed >> INTTYPE_SIZE)););
    return (uintType) (seed >> INTTYPE_SIZE);
  } /* uintRand */

#else



/**
 *  Multiply two double uintType factors to a double uintType product.
 *  The low bits of the product are returned as double uintType
 *  number (in product_high and product_low). The higher bits of
 *  the product (the bits higher than product_high) are discarded.
 *  A double uintType number consists of a low and a high uintType
 *  number. A double uintType number can also be split into
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
static inline uintType uint2Mult (uintType factor1_high, uintType factor1_low,
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

  /* uint2Mult */
    logFunction(printf("uint2Mult(" F_X(08) F_X(08) ", " F_X(08) F_X(08) ")\n",
                       factor1_high, factor1_low, factor2_high, factor2_low););
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
    logFunction(printf("uint2Mult --> " F_X(08) F_X(08) "\n",
                       *product_high, product_low););
    return product_low;
  } /* uint2Mult */



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
static inline uintType uint2Add (uintType summand1_high, uintType summand1_low,
    uintType summand2_high, uintType summand2_low, uintType *sum_high)

  {
    uintType sum_low;

  /* uint2Add */
    logFunction(printf("uint2Add(" F_X(08) F_X(08) ", " F_X(08) F_X(08) ")\n",
                       summand1_high, summand1_low, summand2_high, summand2_low););
    sum_low = UINT_BITS(summand1_low + summand2_low);
    if (UINT_HIGHEST_BIT(summand1_low) + UINT_HIGHEST_BIT(summand2_low) +
        UINT_HIGHEST_BIT(UINT_BITS_WITHOUT_HIGHEST_BIT(summand1_low) +
                         UINT_BITS_WITHOUT_HIGHEST_BIT(summand2_low)) >= 2) {
      *sum_high = UINT_BITS(summand1_high + summand2_high + 1);
    } else {
      *sum_high = UINT_BITS(summand1_high + summand2_high);
    } /* if */
    logFunction(printf("uint2Add --> " F_X(08) F_X(08) "\n",
                       *sum_high, sum_low););
    return sum_low;
  } /* uint2Add */



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
    logFunction(printf("uintRand\n"););
    /* SEED = SEED * RAND_MULTIPLIER + RAND_INCREMENT */
    low_seed = uint2Mult(high_seed, low_seed, (uintType) INT_SUFFIX(0),
                         (uintType) INT_SUFFIX(RAND_MULTIPLIER), &high_seed);
    low_seed = uint2Add(high_seed, low_seed, (uintType) INT_SUFFIX(0),
                        (uintType) INT_SUFFIX(RAND_INCREMENT), &high_seed);
    logFunction(printf("uintRand --> " F_X(08) "\n", high_seed););
    return high_seed;
  } /* uintRand */
#endif



/**
 *  Compute a pseudo-random number in the range 0 .. rand_max.
 *  The random values are uniform distributed.
 *  This function is used by the compiler, when lower and upper bound
 *  of a random number are known at compile time.
 *  @param rand_max Maximum random number. Rand_max should be near to
 *         UINTTYPE_MAX to avoid that the loop is traversed to often.
 *  @return a random number such that 0 <= uintRandLimited(rand_max) and
 *          uintRandLimited(rand_max) <= rand_max holds.
*/
uintType uintRandLimited (uintType rand_max)

  {
    uintType rand_val;

  /* uintRandLimited */
    logFunction(printf("uintRandLimited(" FMT_U ")\n", rand_max););
    do {
      rand_val = uintRand();
    } while (rand_val > rand_max);
    logFunction(printf("uintRandLimited --> " FMT_U "\n", rand_val););
    return rand_val;
  } /* uintRandLimited */



static uintType uintGcd (uintType number1, uintType number2)

  {
    uintType temp;

  /* uintGcd */
    logFunction(printf("uintGcd(" FMT_D ", " FMT_D ")\n",
                       number1, number2););
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
    logFunction(printf("uintGcd --> " FMT_D "\n", number1););
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
    int result = 0;

  /* uint32MostSignificantBit */
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
    int result = 0;

  /* uint64MostSignificantBit */
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
    static const int MULTIPLY_DE_BRUIJN_BIT_POSITION[32] = {
         0,  1, 28,  2, 29, 14, 24,  3, 30, 22, 20, 15, 25, 17,  4,  8,
        31, 27, 13, 23, 21, 19, 16,  7, 26, 12, 18,  6, 11,  5, 10,  9};

  /* uint32LeastSignificantBit */
    return MULTIPLY_DE_BRUIJN_BIT_POSITION[
      ((uint32Type) ((number & -number) * UINT32_SUFFIX(0x077cb531))) >> 27];
  } /* uint32LeastSignificantBit */



#ifdef INT64TYPE
int uint64LeastSignificantBit (uint64Type number)

  {
    static const int MULTIPLY_DE_BRUIJN_BIT_POSITION[64] = {
         0,  1,  2, 53,  3,  7, 54, 27,  4, 38, 41,  8, 34, 55, 48, 28,
        62,  5, 39, 46, 44, 42, 22,  9, 24, 35, 59, 56, 49, 18, 29, 11,
        63, 52,  6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10,
        51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12};

  /* uint64LeastSignificantBit */
    return MULTIPLY_DE_BRUIJN_BIT_POSITION[
        (uint64Type) ((number & -number) * UINT64_SUFFIX(0x022fdd63cc95386d)) >> 58];
  } /* uint64LeastSignificantBit */
#endif



/**
 *  Determine the number of one bits in an unsigned number.
 *  The function uses a combination of sideways additions and
 *  a multiplication to count the bits set in a number element.
 *  @return the number of one bits.
 */
intType uintCard (uintType number)

  { /* uintCard */
#if INTTYPE_SIZE == 32
    number -= (number >> 1) & UINT32_SUFFIX(0x55555555);
    number =  (number       & UINT32_SUFFIX(0x33333333)) +
             ((number >> 2) & UINT32_SUFFIX(0x33333333));
    number = (number + (number >> 4)) & UINT32_SUFFIX(0x0f0f0f0f);
    return (intType) ((number * UINT32_SUFFIX(0x01010101)) >> 24);
#elif INTTYPE_SIZE == 64
    number -= (number >> 1) & UINT64_SUFFIX(0x5555555555555555);
    number =  (number       & UINT64_SUFFIX(0x3333333333333333)) +
             ((number >> 2) & UINT64_SUFFIX(0x3333333333333333));
    number = (number + (number >> 4)) & UINT64_SUFFIX(0x0f0f0f0f0f0f0f0f);
    return (intType) ((number * UINT64_SUFFIX(0x0101010101010101)) >> 56);
#endif
  } /* uintCard */



/**
 *  Compare two unsigned integer numbers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType uintCmp (uintType number1, uintType number2)

  {
    intType signumValue;

  /* uintCmp */
    if (number1 < number2) {
      signumValue = -1;
    } else {
      signumValue = number1 > number2;
    } /* if */
    return signumValue;
  } /* uintCmp */



/**
 *  Reinterpret the generic parameters as uintType and call uintCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(intType).
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType uintCmpGeneric (const genericType value1, const genericType value2)

  { /* uintCmpGeneric */
    return intCmp(((const_rtlObjectType *) &value1)->value.intValue,
                  ((const_rtlObjectType *) &value2)->value.intValue);
  } /* uintCmpGeneric */



/**
 *  Convert an unsigned number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with letters.
 *  @param upperCase Decides about the letter case.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR When base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType uintRadix (uintType number, intType base, boolType upperCase)

  {
    const_ustriType digits;
    strElemType buffer_1[RADIX_BUFFER_SIZE];
    strElemType *buffer;
    memSizeType length;
    striType result;

  /* uintRadix */
    logFunction(printf("uintRadix(" FMT_U ", " FMT_D ", %d)\n",
                       number, base, upperCase););
    if (unlikely(base < 2 || base > 36)) {
      logError(printf("uintRadix(" FMT_U ", " FMT_D ", %d): "
                      "base < 2 or base > 36.\n",
                      number, base, upperCase););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      digits = digitTable[upperCase];
      buffer = &buffer_1[RADIX_BUFFER_SIZE];
      do {
        *(--buffer) = (strElemType) (digits[number % (uintType) base]);
      } while ((number /= (uintType) base) != 0);
      length = (memSizeType) (&buffer_1[RADIX_BUFFER_SIZE] - buffer);
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = length;
        memcpy(result->mem, buffer, (size_t) (length * sizeof(strElemType)));
      } /* if */
    } /* if */
    logFunction(printf("uintRadix --> \"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* uintRadix */



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
striType uintRadixPow2 (uintType number, int shift, int mask, boolType upperCase)

  {
    const_ustriType digits;
    strElemType buffer_1[RADIX_BUFFER_SIZE];
    strElemType *buffer;
    memSizeType length;
    striType result;

  /* uintRadixPow2 */
    logFunction(printf("uintRadixPow2(" FMT_U ", %d, %x, %d)\n",
                       number, shift, mask, upperCase););
    digits = digitTable[upperCase];
    buffer = &buffer_1[RADIX_BUFFER_SIZE];
    do {
      *(--buffer) = (strElemType) (digits[number & (uintType) mask]);
    } while ((number >>= shift) != 0);
    length = (memSizeType) (&buffer_1[RADIX_BUFFER_SIZE] - buffer);
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = length;
      memcpy(result->mem, buffer, (size_t) (length * sizeof(strElemType)));
    } /* if */
    logFunction(printf("uintRadixPow2 --> \"%s\"\n",
                       striAsUnquotedCStri(result)););
    return result;
  } /* uintRadixPow2 */



/**
 *  Convert an unsigned integer number to a string.
 *  The number is converted to a string with decimal representation.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType uintStr (uintType number)

  {
    strElemType *buffer;
    memSizeType length;
    striType result;

  /* uintStr */
    logFunction(printf("uintStr(" FMT_U ")\n", number););
    length = DECIMAL_DIGITS(number);
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = length;
      buffer = &result->mem[length];
      do {
        *(--buffer) = (strElemType) (number % 10 + '0');
      } while ((number /= 10) != 0);
    } /* if */
    logFunction(printf("uintStr --> \"%s\"\n",
                       striAsUnquotedCStri(result)););
    return result;
  } /* uintStr */



/**
 *  Compare two generic values.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType genericCmp (const genericType value1, const genericType value2)

  {
    intType signumValue;

  /* genericCmp */
    if (value1 < value2) {
      signumValue = -1;
    } else {
      signumValue = value1 > value2;
    } /* if */
    return signumValue;
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

  {
    intType signumValue;

  /* ptrCmp */
    if ((memSizeType) value1 < (memSizeType) value2) {
      signumValue = -1;
    } else {
      signumValue = (memSizeType) value1 > (memSizeType) value2;
    } /* if */
    return signumValue;
  } /* ptrCmp */



/**
 *  Reinterpret the generic parameters as rtlPtrType and call ptrCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(rtlPtrType).
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType ptrCmpGeneric (const genericType value1, const genericType value2)

  { /* ptrCmpGeneric */
    return ptrCmp(((const_rtlObjectType *) &value1)->value.ptrValue,
                  ((const_rtlObjectType *) &value2)->value.ptrValue);
  } /* ptrCmpGeneric */



/**
 *  Reinterpret the generic parameters as rtlPtrType and assign source to dest.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(rtlPtrType).
 */
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
 *  @exception OVERFLOW_ERROR When the result would be less than
 *             integer.first or greater than integer.last.
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
    logFunction(printf("intBinom(" FMT_D ", " FMT_D ")\n",
                       n_number, k_number););
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
        negative = (boolType) (k_number & 1);
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
            if (unlikely(unsigned_result > UINTTYPE_MAX / reducedNumerator)) {
              /* Unavoidable overflow */
              logError(printf("intBinom(" FMT_D ", " FMT_D "): "
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
        /* The value INTTYPE_MIN below is casted, to avoid a signed   */
        /* integer overflow, when it is negated. A negated unsigned   */
        /* value should still be unsigned. But lcc-win32 thinks, that */
        /* negating an unsigned value results in a signed value.      */
        /* Therefore an explicit cast of the negated value is done.   */
        if (unlikely(unsigned_result > (uintType) -(uintType) INTTYPE_MIN)) {
          logError(printf("intBinom(" FMT_D ", " FMT_D "): "
                          "Negative result (-" FMT_U ") too small.\n",
                          n_number, k_number, unsigned_result););
          raise_error(OVERFLOW_ERROR);
          result = 0;
        } else {
          result = (intType) -unsigned_result;
        } /* if */
      } else {
        /* The cast below silences possible signed-unsigned comparison    */
        /* warnings and prevents that lcc-win32 does a signed comparison. */
        if (unlikely(unsigned_result > (uintType) INTTYPE_MAX)) {
          logError(printf("intBinom(" FMT_D ", " FMT_D "): "
                          "Positive result (" FMT_U ") too big.\n",
                          n_number, k_number, unsigned_result););
          raise_error(OVERFLOW_ERROR);
          result = 0;
        } else {
          result = (intType) unsigned_result;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("intBinom --> " FMT_D "\n", result););
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
    logFunction(printf("uintBinomNoChk(" FMT_D ", " FMT_U ")\n",
                       n_number, k_number););
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
    logFunction(printf("uintBinomNoChk --> " FMT_U "\n", result););
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
    intType bitLength;

  /* intBitLength */
    if (number < 0) {
      number = ~number;
    } /* if */
    bitLength = uintMostSignificantBit((uintType) number) + 1;
    return bitLength;
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
 *         byte has an ordinal > BYTE_MAX (=12/).
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
    logFunction(printf("intBytesBe(" FMT_D ", %d)\n", number, isSigned););
    if (number >= 0) {
      do {
        pos--;
        buffer[pos] = (strElemType) (number & 0xff);
        number >>= CHAR_BIT;
      } while (number != 0);
      if (isSigned && buffer[pos] > BYTE_MAX) {
        pos--;
        buffer[pos] = 0;
      } /* if */
    } else if (likely(isSigned)) {
      do {
        pos--;
        buffer[pos] = (strElemType) (number & 0xff);
#if RSHIFT_DOES_SIGN_EXTEND
        number >>= CHAR_BIT;
#else
        number = ~(~number >> CHAR_BIT);
#endif
      } while (number != -1);
      if (buffer[pos] <= BYTE_MAX) {
        pos--;
        buffer[pos] = UBYTE_MAX;
      } /* if */
    } else {
      logError(printf("intBytesBe(" FMT_D ", %d): "
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
    logFunction(printf("intBytesBe --> \"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* intBytesBe */



/**
 *  Convert a string of bytes (interpreted as big-endian) to an integer.
 *  @param byteStri String of bytes to be converted. The bytes are
 *         interpreted as binary big-endian representation with a
 *         base of 256. Negative values use the twos-complement
 *         representation.
 *  @return an integer created from 'byteStri'. The result is negative
 *          when the most significant byte (the first byte) of 'byteStri'
 *          has an ordinal > BYTE_MAX (=127).
 *  @exception RANGE_ERROR When characters beyond '\255;' are present or
 *             when the result value cannot be represented with an integer.
 */
intType intBytesBe2Int (const const_striType byteStri)

  {
    memSizeType pos = 0;
    intType result;

  /* intBytesBe2Int */
    logFunction(printf("intBytesBe2Int(\"%s\")\n",
                       striAsUnquotedCStri(byteStri)););
    if (byteStri->size == 0 || byteStri->mem[0] <= BYTE_MAX) {
      if (byteStri->size >= sizeof(intType)) {
        while (pos < byteStri->size && byteStri->mem[pos] == 0) {
          pos++;
        } /* if */
        if (unlikely(byteStri->size - pos > sizeof(intType) ||
                     (byteStri->size - pos == sizeof(intType) &&
                      byteStri->mem[pos] > BYTE_MAX))) {
          logError(printf("intBytesBe2Int(\"%s\"): "
                          "Number too big.\n",
                          striAsUnquotedCStri(byteStri)););
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } /* if */
      result = 0;
    } else { /* byteStri->size != 0 && byteStri->mem[0] > BYTE_MAX */
      if (byteStri->size >= sizeof(intType)) {
        while (pos < byteStri->size && byteStri->mem[pos] == UBYTE_MAX) {
          pos++;
        } /* if */
        if (unlikely(byteStri->size - pos > sizeof(intType) ||
                     (byteStri->size - pos == sizeof(intType) &&
                      byteStri->mem[pos] <= BYTE_MAX))) {
          logError(printf("intBytesBe2Int(\"%s\"): "
                          "Number too small.\n",
                          striAsUnquotedCStri(byteStri)););
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } /* if */
      result = -1;
    } /* if */
    for (; pos < byteStri->size; pos++) {
      if (unlikely(byteStri->mem[pos] > UBYTE_MAX)) {
        logError(printf("intBytesBe2Int(\"%s\"): "
                        "Character '\\%d;' is beyond '\\255;'.\n",
                        striAsUnquotedCStri(byteStri),
                        byteStri->mem[pos]););
        raise_error(RANGE_ERROR);
        return 0;
      } /* if */
      result <<= CHAR_BIT;
      result += byteStri->mem[pos];
    } /* for */
    logFunction(printf("intBytesBe2Int --> " FMT_D "\n", result););
    return result;
  } /* intBytesBe2Int */



/**
 *  Convert a string of bytes (interpreted as big-endian) to an integer.
 *  @param byteStri String of bytes to be converted. The bytes are
 *         interpreted as binary big-endian representation with a
 *         base of 256.
 *  @return an integer created from 'byteStri'. The result is always
 *          positive.
 *  @exception RANGE_ERROR When characters beyond '\255;' are present or
 *             when the result value cannot be represented with an integer.
 */
intType intBytesBe2UInt (const const_striType byteStri)

  {
    memSizeType pos = 0;
    intType result = 0;

  /* intBytesBe2UInt */
    logFunction(printf("intBytesBe2UInt(\"%s\")\n",
                       striAsUnquotedCStri(byteStri)););
    if (byteStri->size >= sizeof(intType)) {
      while (pos < byteStri->size && byteStri->mem[pos] == 0) {
        pos++;
      } /* if */
      if (unlikely(byteStri->size - pos > sizeof(intType) ||
                   (byteStri->size - pos == sizeof(intType) &&
                    byteStri->mem[pos] > BYTE_MAX))) {
        logError(printf("intBytesBe2UInt(\"%s\"): "
                        "Number too big.\n",
                        striAsUnquotedCStri(byteStri)););
        raise_error(RANGE_ERROR);
        return 0;
      } /* if */
    } /* if */
    for (; pos < byteStri->size; pos++) {
      if (unlikely(byteStri->mem[pos] > UBYTE_MAX)) {
        logError(printf("intBytesBe2UInt(\"%s\"): "
                        "Character '\\%d;' is beyond '\\255;'.\n",
                        striAsUnquotedCStri(byteStri),
                        byteStri->mem[pos]););
        raise_error(RANGE_ERROR);
        return 0;
      } /* if */
      result <<= CHAR_BIT;
      result += byteStri->mem[pos];
    } /* for */
    logFunction(printf("intBytesBe2UInt --> " FMT_D "\n", result););
    return result;
  } /* intBytesBe2UInt */



/**
 *  Convert an integer into a little-endian string of bytes.
 *  The result uses binary representation with a base of 256.
 *  The result contains chars (bytes) with an ordinal <= 255.
 *  @param number Integer number to be converted.
 *  @param isSigned Determines the signedness of the result.
 *         When 'isSigned' is TRUE the result is encoded with the
 *         twos-complement representation. In this case a negative
 *         'number' is converted to a result where the most significant
 *         byte has an ordinal > BYTE_MAX (=127).
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
    logFunction(printf("intBytesLe(" FMT_D ", %d)\n", number, isSigned););
    if (number >= 0) {
      do {
        buffer[pos] = (ucharType) (number & 0xff);
        number >>= CHAR_BIT;
        pos++;
      } while (number != 0);
      if (isSigned && buffer[pos - 1] > BYTE_MAX) {
        buffer[pos] = 0;
        pos++;
      } /* if */
    } else if (likely(isSigned)) {
      do {
        buffer[pos] = (ucharType) (number & 0xff);
#if RSHIFT_DOES_SIGN_EXTEND
        number >>= CHAR_BIT;
#else
        number = ~(~number >> CHAR_BIT);
#endif
        pos++;
      } while (number != -1);
      if (buffer[pos - 1] <= BYTE_MAX) {
        buffer[pos] = UBYTE_MAX;
        pos++;
      } /* if */
    } else {
      logError(printf("intBytesLe(" FMT_D ", %d): "
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
    logFunction(printf("intBytesLe --> \"%s\"\n",
                       striAsUnquotedCStri(result)););
    return result;
  } /* intBytesLe */



/**
 *  Convert a string of bytes (interpreted as little-endian) to an integer.
 *  @param byteStri String of bytes to be converted. The bytes are
 *         interpreted as binary little-endian representation with a
 *         base of 256. Negative values use the twos-complement
 *         representation.
 *  @return an integer created from 'byteStri'. The result is negative
 *          when the most significant byte (the last byte) of 'byteStri'
 *          has an ordinal > BYTE_MAX (=127).
 *  @exception RANGE_ERROR When characters beyond '\255;' are present or
 *             when the result value cannot be represented with an integer.
 */
intType intBytesLe2Int (const const_striType byteStri)

  {
    memSizeType pos;
    intType result;

  /* intBytesLe2Int */
    logFunction(printf("intBytesLe2Int(\"%s\")\n",
                       striAsUnquotedCStri(byteStri)););
    pos = byteStri->size;
    if (byteStri->size == 0 || byteStri->mem[pos - 1] <= BYTE_MAX) {
      if (unlikely(byteStri->size >= sizeof(intType))) {
        while (pos > 0 && byteStri->mem[pos - 1] == 0) {
          pos--;
        } /* if */
        if (unlikely(pos > sizeof(intType) ||
                     (pos == sizeof(intType) &&
                      byteStri->mem[pos - 1] > BYTE_MAX))) {
          logError(printf("intBytesLe2Int(\"%s\"): "
                          "Number too big.\n",
                          striAsUnquotedCStri(byteStri)););
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } /* if */
      result = 0;
    } else { /* byteStri->size != 0 && byteStri->mem[pos - 1] > BYTE_MAX */
      if (unlikely(byteStri->size >= sizeof(intType))) {
        while (pos > 0 && byteStri->mem[pos - 1] == UBYTE_MAX) {
          pos--;
        } /* if */
        if (unlikely(pos > sizeof(intType) ||
                     (pos == sizeof(intType) &&
                      byteStri->mem[pos - 1] <= BYTE_MAX))) {
          logError(printf("intBytesLe2Int(\"%s\"): "
                          "Number too small.\n",
                          striAsUnquotedCStri(byteStri)););
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } /* if */
      result = -1;
    } /* if */
    for (; pos > 0; pos--) {
      if (unlikely(byteStri->mem[pos - 1] > UBYTE_MAX)) {
        logError(printf("intBytesLe2Int(\"%s\"): "
                        "Character '\\%d;' is beyond '\\255;'.\n",
                        striAsUnquotedCStri(byteStri),
                        byteStri->mem[pos - 1]););
        raise_error(RANGE_ERROR);
        return 0;
      } /* if */
      result <<= CHAR_BIT;
      result += byteStri->mem[pos - 1];
    } /* for */
    logFunction(printf("intBytesLe2Int --> " FMT_D "\n", result););
    return result;
  } /* intBytesLe2Int */



/**
 *  Convert a string of bytes (interpreted as little-endian) to an integer.
 *  @param byteStri String of bytes to be converted. The bytes are
 *         interpreted as binary little-endian representation with a
 *         base of 256.
 *  @return an integer created from 'byteStri'. The result is always
 *          positive.
 *  @exception RANGE_ERROR When characters beyond '\255;' are present or
 *             when the result value cannot be represented with an integer.
 */
intType intBytesLe2UInt (const const_striType byteStri)

  {
    memSizeType pos;
    intType result = 0;

  /* intBytesLe2UInt */
    logFunction(printf("intBytesLe2UInt(\"%s\")\n",
                       striAsUnquotedCStri(byteStri)););
    pos = byteStri->size;
    if (unlikely(byteStri->size >= sizeof(intType))) {
      while (pos > 0 && byteStri->mem[pos - 1] == 0) {
        pos--;
      } /* if */
      if (unlikely(pos > sizeof(intType) ||
                   (pos == sizeof(intType) &&
                    byteStri->mem[pos - 1] > BYTE_MAX))) {
        logError(printf("intBytesLe2UInt(\"%s\"): "
                        "Number too big.\n",
                        striAsUnquotedCStri(byteStri)););
        raise_error(RANGE_ERROR);
        return 0;
      } /* if */
    } /* if */
    for (; pos > 0; pos--) {
      if (unlikely(byteStri->mem[pos - 1] > UBYTE_MAX)) {
        logError(printf("intBytesLe2UInt(\"%s\"): "
                        "Character '\\%d;' is beyond '\\255;'.\n",
                        striAsUnquotedCStri(byteStri),
                        byteStri->mem[pos - 1]););
        raise_error(RANGE_ERROR);
        return 0;
      } /* if */
      result <<= CHAR_BIT;
      result += byteStri->mem[pos - 1];
    } /* for */
    logFunction(printf("intBytesLe2UInt --> " FMT_D "\n", result););
    return result;
  } /* intBytesLe2UInt */



/**
 *  Compare two integer numbers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType intCmp (intType number1, intType number2)

  {
    intType signumValue;

  /* intCmp */
    if (number1 < number2) {
      signumValue = -1;
    } else {
      signumValue = number1 > number2;
    } /* if */
    return signumValue;
  } /* intCmp */



/**
 *  Reinterpret the generic parameters as intType and call intCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(intType).
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
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
    logFunction(printf("intLog10(" FMT_D ")\n", number););
    if (unlikely(number < 0)) {
      logError(printf("intLog10(" FMT_D "): Number is negative.\n",
                      number););
      raise_error(NUMERIC_ERROR);
      logarithm = 0;
    } else if (number == 0) {
      logarithm = -1;
    } else {
      logarithm = DECIMAL_DIGITS((uintType) number) - 1;
    } /* if */
    logFunction(printf("intLog10 --> %d\n", logarithm););
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
    logFunction(printf("intLog2(" FMT_D ")\n", number););
    if (unlikely(number < 0)) {
      logError(printf("intLog2(" FMT_D "): Number is negative.\n",
                      number););
      raise_error(NUMERIC_ERROR);
      logarithm = 0;
    } else {
      logarithm = uintMostSignificantBit((uintType) number);
    } /* if */
    logFunction(printf("intLog2 --> %d\n", logarithm););
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



/**
 *  Convert integer to string and pad it with zeros at the left side.
 *  The number is converted to a string with decimal representation.
 *  For negative numbers a minus sign is prepended.
 *   123 lpad0 5   returns "00123"
 *   -123 lpad0 5  returns "-0123"
 *   123 lpad0 2   returns "123"
 *   -123 lpad0 2  returns "-123"
 *  @param number Number to be converted to a [[string]].
 *  @param length Minimum length of the result.
 *  @return number as decimal string left padded with zeroes.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType intLpad0 (intType number, const intType pad_size)

  {
    uintType unsigned_number;
    boolType negative;
    strElemType *buffer;
    memSizeType length;
    memSizeType result_size;
    striType result;

  /* intLpad0 */
    logFunction(printf("intLpad0(" FMT_D ", " FMT_D ")\n",
                       number, pad_size););
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
    } /* if */
    logFunction(printf("intLpad0 --> \"%s\"\n",
                       striAsUnquotedCStri(result)););
    return result;
  } /* intLpad0 */



/**
 *  Multiply two integer numbers.
 *  @return the product of the two numbers.
 *  @exception OVERFLOW_ERROR When an integer overflow occurs.
 */
intType intMultOvfChk (intType factor1, intType factor2)

  {
    intType product;

  /* intMultOvfChk */
    logFunction(printf("intMultOvfChk(" FMT_D ", " FMT_D ")\n",
                       factor1, factor2););
    if (factor1 < 0) {
      if (factor2 < 0) {
        if (unlikely(factor1 < INTTYPE_MAX / factor2)) {
          logError(printf("intMultOvfChk(" FMT_D ", " FMT_D "): "
                          "factor1 < " FMT_D "\n",
                          factor1, factor2, INTTYPE_MAX / factor2););
          raise_error(OVERFLOW_ERROR);
          return 0;
        } /* if */
      } else if (factor2 != 0) {
        if (unlikely(factor1 < INTTYPE_MIN / factor2)) {
          logError(printf("intMultOvfChk(" FMT_D ", " FMT_D "): "
                          "factor1 < " FMT_D "\n",
                          factor1, factor2, INTTYPE_MIN / factor2););
          raise_error(OVERFLOW_ERROR);
          return 0;
        } /* if */
      } /* if */
    } else if (factor1 != 0) {
      if (factor2 < 0) {
        if (unlikely(factor2 < INTTYPE_MIN / factor1)) {
          logError(printf("intMultOvfChk(" FMT_D ", " FMT_D "): "
                          "factor2 < " FMT_D "\n",
                          factor1, factor2, INTTYPE_MIN / factor1););
          raise_error(OVERFLOW_ERROR);
          return 0;
        } /* if */
      } else if (factor2 != 0) {
        if (unlikely(factor2 > INTTYPE_MAX / factor1)) {
          logError(printf("intMultOvfChk(" FMT_D ", " FMT_D "): "
                          "factor2 > " FMT_D "\n",
                          factor1, factor2, INTTYPE_MAX / factor1););
          raise_error(OVERFLOW_ERROR);
          return 0;
        } /* if */
      } /* if */
    } /* if */
    product = factor1 * factor2;
    logFunction(printf("intMultOvfChk --> " FMT_D "\n", product););
    return product;
  } /* intMultOvfChk */



#ifdef INT_MULT64_COMPILE_ERROR
/**
 *  Multiply two signed 64-bit int values and check for overflow.
 *  Under Windows clang uses __mulodi4 when the option -ftrapv is used.
 *  Unfortunately __mulodi4 is not part of the runtime library.
 *  This rewrite of the original __mulodi4 needs only 56% of the runtime.
 */
int64Type __mulodi4 (int64Type factor1, int64Type factor2, int *overflow)

  {
    int64Type product;

  /* __mulodi4 */
    logFunction(printf("__mulodi4(" FMT_D64 ", " FMT_D64 ", *)\n",
                       factor1, factor2););
    product = (int64Type) ((uint64Type) factor1 * (uint64Type) factor2);
    if (factor1 < 0) {
      if (factor2 < 0) {
        if (unlikely(factor1 < INT64TYPE_MAX / factor2)) {
          *overflow = 1;
          logFunction(printf("__mulodi4(overflow=%d) --> " FMT_D64 "\n",
                             *overflow, product););
          return product;
        } /* if */
      } else if (factor2 != 0) {
        if (unlikely(factor1 < INT64TYPE_MIN / factor2)) {
          *overflow = 1;
          logFunction(printf("__mulodi4(overflow=%d) --> " FMT_D64 "\n",
                             *overflow, product););
          return product;
        } /* if */
      } /* if */
    } else if (factor1 != 0) {
      if (factor2 < 0) {
        if (unlikely(factor2 < INT64TYPE_MIN / factor1)) {
          *overflow = 1;
          logFunction(printf("__mulodi4(overflow=%d) --> " FMT_D64 "\n",
                             *overflow, product););
          return product;
        } /* if */
      } else if (factor2 != 0) {
        if (unlikely(factor2 > INT64TYPE_MAX / factor1)) {
          *overflow = 1;
          logFunction(printf("__mulodi4(overflow=%d) --> " FMT_D64 "\n",
                             *overflow, product););
          return product;
        } /* if */
      } /* if */
    } /* if */
    *overflow = 0;
    logFunction(printf("__mulodi4(overflow=%d) --> " FMT_D64 "\n",
                       *overflow, product););
    return product;
  } /* __mulodi4 */
#endif



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
    boolType okay;
    boolType negative;
    memSizeType position = 0;
    uintType digitval;
    uintType uintValue;
    intType intResult;

  /* intParse */
    logFunction(printf("intParse(\"%s\")\n", striAsUnquotedCStri(stri)););
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
      logError(printf("intParse(\"%s\"): "
                      "Digit missing.\n",
                      striAsUnquotedCStri(stri)););
      raise_error(RANGE_ERROR);
      intResult = 0;
    } else {
      uintValue = 0;
      okay = TRUE;
      while (position < stri->size &&
          stri->mem[position] >= ((strElemType) '0') &&
          stri->mem[position] <= ((strElemType) '9')) {
        digitval = ((uintType) stri->mem[position]) - ((uintType) '0');
        if (unlikely(uintValue > MAX_DIV_10)) {
          okay = FALSE;
        } else {
          uintValue = ((uintType) 10) * uintValue + digitval;
        } /* if */
        position++;
      } /* while */
      if (unlikely(position < stri->size)) {
        logError(printf("intParse(\"%s\"): "
                        "Illegal digit.\n",
                        striAsUnquotedCStri(stri)););
        raise_error(RANGE_ERROR);
        intResult = 0;
      } else if (unlikely(!okay)) {
        logError(printf("intParse(\"%s\"): "
                        "Absolute value of literal is too big.\n",
                        striAsUnquotedCStri(stri)););
        raise_error(RANGE_ERROR);
        intResult = 0;
      } else {
        if (negative) {
          if (uintValue > (uintType) INTTYPE_MAX + 1) {
            logError(printf("intParse(\"%s\"): Literal too small.\n",
                            striAsUnquotedCStri(stri)););
            raise_error(RANGE_ERROR);
            intResult = 0;
          } else {
            /* The unsigned value is negated to avoid an overflow */
            /* when the most negative intType value is negated.   */
            intResult = (intType) -uintValue;
          } /* if */
        } else if (uintValue > (uintType) INTTYPE_MAX) {
          logError(printf("intParse(\"%s\"): Literal too big.\n",
                          striAsUnquotedCStri(stri)););
          raise_error(RANGE_ERROR);
          intResult = 0;
        } else {
          intResult = (intType) uintValue;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("intParse --> " FMT_D "\n", intResult););
    return intResult;
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
    logFunction(printf("intPow(" FMT_D ", " FMT_D ")\n", base, exponent););
    if (unlikely(exponent < 0)) {
      logError(printf("intPow(" FMT_D ", " FMT_D "): "
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
    logFunction(printf("intPow --> " FMT_D "\n", power););
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
    logFunction(printf("intPowOvfChk(" FMT_D ", " FMT_D ")\n",
                       base, exponent););
    if (unlikely(exponent < 0)) {
      logError(printf("intPowOvfChk(" FMT_D ", " FMT_D "): "
                      "Exponent is negative.\n",
                      base, exponent););
      raise_error(NUMERIC_ERROR);
      power = 0;
    } else {
      if ((uintType) exponent < sizeof(minBaseOfExponent) / sizeof(intType)) {
        if (unlikely(base < minBaseOfExponent[exponent] ||
                     base > maxBaseOfExponent[exponent])) {
          logError(printf("intPowOvfChk(" FMT_D ", " FMT_D "): "
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
        logError(printf("intPowOvfChk(" FMT_D ", " FMT_D "): ",
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
    logFunction(printf("intPowOvfChk --> " FMT_D "\n", power););
    return power;
  } /* intPowOvfChk */



/**
 *  Convert an integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with letters.
 *  For negative numbers a minus sign is prepended.
 *  @param number Number to be converted to a string.
 *  @param base Base of the numeral system used for the conversion.
 *  @param upperCase Decides about the letter case.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR When base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType intRadix (intType number, intType base, boolType upperCase)

  {
    uintType unsigned_number;
    boolType negative;
    const_ustriType digits;
    strElemType buffer_1[RADIX_BUFFER_SIZE];
    strElemType *buffer;
    memSizeType length;
    striType result;

  /* intRadix */
    logFunction(printf("intRadix(" FMT_D ", " FMT_D ", %d)\n",
                       number, base, upperCase););
    if (unlikely(base < 2 || base > 36)) {
      logError(printf("intRadix(" FMT_D ", " FMT_D ", %d): "
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
    logFunction(printf("intRadix --> \"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* intRadix */



/**
 *  Convert an integer number to a string using a radix.
 *  The conversion uses the numeral system with the specified base.
 *  The base is a power of two and it is specified indirectly with
 *  shift and mask. Digit values from 10 upward are encoded with
 *  letters.
 *  @param number Number to be converted to a string.
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
    const_ustriType digits;
    strElemType buffer_1[RADIX_BUFFER_SIZE];
    strElemType *buffer;
    memSizeType length;
    striType result;

  /* intRadixPow2 */
    logFunction(printf("intRadixPow2(" FMT_D ", %d, %x, %d)\n",
                       number, shift, mask, upperCase););
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
    logFunction(printf("intRadixPow2 --> \"%s\"\n",
                       striAsUnquotedCStri(result)););
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
    logFunction(printf("intRand(" FMT_D ", " FMT_D ")\n", low, high););
    if (unlikely(low >= high)) {
      if (low == high) {
        randomNumber = low;
      } else {
        logError(printf("intRand(" FMT_D ", " FMT_D "): "
                        "The range is empty (low > high holds).\n",
                        low, high););
        raise_error(RANGE_ERROR);
        randomNumber = 0;
      } /* if */
    } else {
      scale_limit = (uintType) high - (uintType) low;
      if (unlikely(scale_limit == UINTTYPE_MAX)) {
        /* low must be INTTYPE_MIN and high must be INTTYPE_MAX. */
        randomNumber = (intType) uintRand();
      } else {
        scale_limit++;
        /* Here 2 <= scale_limit <= UINTTYPE_MAX holds. */
        /* Set rand_max to be one less than a multiple of scale_limit. */
        /* Furthermore rand_max is set to a value that is as big as    */
        /* possible but less than or equal to UINTTYPE_MAX.            */
        rand_max = UINTTYPE_MAX - (UINTTYPE_MAX - scale_limit + 1) % scale_limit;
        do {
          rand_val = uintRand();
        } while (rand_val > rand_max);
        randomNumber = (intType) ((uintType) low + rand_val % scale_limit);
      } /* if */
    } /* if */
    logFunction(printf("intRand --> " FMT_D "\n", randomNumber););
    return randomNumber;
  } /* intRand */



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
    logFunction(printf("intSqrt(" FMT_D ")\n", number););
    if (unlikely(number < 0)) {
      logError(printf("intSqrt(" FMT_D "): Number is negative.\n",
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
    logFunction(printf("intSqrt --> " FMT_U "\n", root););
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
    logFunction(printf("intStr(" FMT_D ")\n", number););
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
    logFunction(printf("intStr --> \"%s\"\n",
                       striAsUnquotedCStri(result)););
    return result;
  } /* intStr */



#if ALLOW_STRITYPE_SLICES
striType intStrToBuffer (intType number, striType buffer)

  {
    register uintType unsigned_number;
    boolType negative;
    strElemType *bufferPtr;

  /* intStrToBuffer */
    logFunction(printf("intStrToBuffer(" FMT_D ")\n", number););
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
    logFunction(printf("intStrToBuffer --> \"%s\"\n",
                       striAsUnquotedCStri(buffer)););
    return buffer;
  } /* intStrToBuffer */
#endif
