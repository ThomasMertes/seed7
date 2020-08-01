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


#define UINT_BITS(A)                     ((A) & UINTTYPE_MAX)
#define UINT_BITS_WITHOUT_HIGHEST_BIT(A) ((A) & (UINTTYPE_MAX >> 1))
#define UINT_HIGHEST_BIT(A)              ((A) >> (INTTYPE_SIZE - 1))
#define UINT_LOWER_HALVE_BITS_SET        (UINTTYPE_MAX >> (INTTYPE_SIZE >> 1))
#define LOWER_HALVE_OF_UINT(A)           ((A) & UINT_LOWER_HALVE_BITS_SET)
#define UPPER_HALVE_OF_UINT(A)           ((A) >> (INTTYPE_SIZE >> 1))


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
   -1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
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

static const char lcDigits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char ucDigits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const const_cstritype digitTable[] = {lcDigits, ucDigits};



/**
 *  Multiply two uinttype factors to a double uinttype product.
 *  The whole product fits into the double uinttype number.
 *  The product is returned in product_high and product_low.
 *  A double uinttype number consists of a low and a high uinttype
 *  number. A double uinttype number can also be splitted into
 *  four halve uinttype parts. The bits of a double uinttype have
 *  the following memory layout:
 *  +---------------------------------------+
 *  |            double uinttype            |
 *  +-------------------+-------------------+
 *  |   high uinttype   |    low uinttype   |
 *  +---------+---------+---------+---------+
 *  | part[3] | part[2] | part[1] | part[0] |
 *  +---------+---------+---------+---------+
 *   ^ highest bit                         ^ lowest bit
 *  @param product_high The address to return the high product.
 *  @return the low product
 */
uinttype uint_mult (uinttype factor1, uinttype factor2, uinttype *product_high)

  {
    uinttype factor1_part[2];  /* parts 2 and 3 are not used */
    uinttype factor2_part[2];  /* parts 2 and 3 are not used */
    uinttype c1;  /* memory layout:   | part[1] | part[0] |  */
    uinttype c2;  /* memory layout:   | part[2] | part[1] |  */
    uinttype c3;  /* memory layout:   | part[2] | part[1] |  */
    uinttype c4;  /* memory layout:   | part[2] | part[1] |  */
    uinttype c5;  /* memory layout:   | part[3] | part[2] |  */
    uinttype product_low;

  /* uint_mult */
#ifdef TRACE_RANDOM
    printf("BEGIN uint_mult(%08x, %08x)\n",
        (unsigned int) factor1, (unsigned int) factor2);
#endif
    factor1_part[0] = LOWER_HALVE_OF_UINT(factor1);
    factor1_part[1] = UPPER_HALVE_OF_UINT(factor1);
    factor2_part[0] = LOWER_HALVE_OF_UINT(factor2);
    factor2_part[1] = UPPER_HALVE_OF_UINT(factor2);
    c1 = factor1_part[0] * factor2_part[0];
    c2 = factor1_part[0] * factor2_part[1];
    c3 = factor1_part[1] * factor2_part[0];
    c4 = UPPER_HALVE_OF_UINT(c1) + LOWER_HALVE_OF_UINT(c2) + LOWER_HALVE_OF_UINT(c3);
    c5 = UPPER_HALVE_OF_UINT(c2) + UPPER_HALVE_OF_UINT(c3) + UPPER_HALVE_OF_UINT(c4) +
         factor1_part[1] * factor2_part[1];
    /* c5 contains the high uinttype of factor1 * factor2 */
    product_low = UINT_BITS(factor1 * factor2);
    *product_high = UINT_BITS(c5);
#ifdef TRACE_RANDOM
    printf("END uint_mult ==> %08x%08x\n",
        (unsigned int) *product_high, (unsigned int) product_low);
#endif
    return product_low;
  } /* uint_mult */



/**
 *  Multiply two double uinttype factors to a double uinttype product.
 *  The low bits of the product are returned as double uinttype
 *  number (in product_high and product_low). The higher bits of
 *  the product (the bits higher than product_high) are discarded.
 *  A double uinttype number consists of a low and a high uinttype
 *  number. A double uinttype number can also be splitted into
 *  four halve uinttype parts. The bits of a double uinttype have
 *  the following memory layout:
 *  +---------------------------------------+
 *  |            double uinttype            |
 *  +-------------------+-------------------+
 *  |   high uinttype   |    low uinttype   |
 *  +---------+---------+---------+---------+
 *  | part[3] | part[2] | part[1] | part[0] |
 *  +---------+---------+---------+---------+
 *   ^ highest bit                         ^ lowest bit
 *  @param product_high The address to return the high product.
 *  @return the low product
 */
static inline uinttype uint2_mult (uinttype factor1_high, uinttype factor1_low,
    uinttype factor2_high, uinttype factor2_low, uinttype *product_high)

  {
    uinttype factor1_part[2];  /* parts 2 and 3 are not used */
    uinttype factor2_part[2];  /* parts 2 and 3 are not used */
    uinttype c1;  /* memory layout:   | part[1] | part[0] |  */
    uinttype c2;  /* memory layout:   | part[2] | part[1] |  */
    uinttype c3;  /* memory layout:   | part[2] | part[1] |  */
    uinttype c4;  /* memory layout:   | part[2] | part[1] |  */
    uinttype c5;  /* memory layout:   | part[3] | part[2] |  */
    uinttype product_low;

  /* uint2_mult */
#ifdef TRACE_RANDOM
    printf("BEGIN uint2_mult(%08x%08x, %08x%08x)\n",
        (unsigned int) factor1_high, (unsigned int) factor1_low,
        (unsigned int) factor2_high, (unsigned int) factor2_low);
#endif
    factor1_part[0] = LOWER_HALVE_OF_UINT(factor1_low);
    factor1_part[1] = UPPER_HALVE_OF_UINT(factor1_low);
    factor2_part[0] = LOWER_HALVE_OF_UINT(factor2_low);
    factor2_part[1] = UPPER_HALVE_OF_UINT(factor2_low);
    c1 = factor1_part[0] * factor2_part[0];
    c2 = factor1_part[0] * factor2_part[1];
    c3 = factor1_part[1] * factor2_part[0];
    c4 = UPPER_HALVE_OF_UINT(c1) + LOWER_HALVE_OF_UINT(c2) + LOWER_HALVE_OF_UINT(c3);
    c5 = UPPER_HALVE_OF_UINT(c2) + UPPER_HALVE_OF_UINT(c3) + UPPER_HALVE_OF_UINT(c4) +
         factor1_part[1] * factor2_part[1];
    /* c5 contains the high uinttype of factor1_low * factor2_low */
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
 *  Add two double uinttype summands to a double uinttype sum.
 *  A possible excess bit, that does not fit into the sum
 *  (the excess bit is higher than sum_high), is discarded.
 *  A double uinttype number consists of a low and a high uinttype
 *  number. The bits of a double uinttype have the following
 *  memory layout:
 *  +---------------------------------------+
 *  |            double uinttype            |
 *  +-------------------+-------------------+
 *  |   high uinttype   |    low uinttype   |
 *  +-------------------+-------------------+
 *   ^ highest bit                         ^ lowest bit
 *  @param sum_high The address to return the high sum.
 *  @return the low sum
 */
static inline uinttype uint2_add (uinttype summand1_high, uinttype summand1_low,
    uinttype summand2_high, uinttype summand2_low, uinttype *sum_high)

  {
    uinttype sum_low;

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



/**
 *  Compute a random unsigned number in the range 0 .. UINTTYPE_MAX.
 *  The linear congruential method is used to generate the random
 *  sequence of uinttype numbers. The generator uses double uinttype
 *  numbers for the seed. Only the high bits of the seed (high_seed)
 *  are used as random number. This avoids that the lower-order bits
 *  of the generated sequence have a short period.
 *  @return the random number.
 */
uinttype uint_rand (void)

  {
    static booltype seed_necessary = TRUE;
    static uinttype low_seed;
    static uinttype high_seed;

  /* uint_rand */
#ifdef TRACE_RANDOM
    printf("BEGIN uint_rand\n");
#endif
    if (unlikely(seed_necessary)) {
      uinttype micro_sec = (uinttype) timMicroSec();

      high_seed = (uinttype) time(NULL);
      high_seed = high_seed ^ (high_seed << 16);
      low_seed = (uinttype) clock();
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
      seed_necessary = FALSE;
    } /* if */
#if INTTYPE_SIZE == 32
    /* SEED = SEED * 1103515245 + 12345 */
    low_seed = uint2_mult(high_seed, low_seed, (uinttype) INT_SUFFIX(0), (uinttype) INT_SUFFIX(1103515245),
        &high_seed);
    low_seed = uint2_add(high_seed, low_seed, (uinttype) INT_SUFFIX(0), (uinttype) INT_SUFFIX(12345),
        &high_seed);
#elif INTTYPE_SIZE == 64
    /* SEED = SEED * 6364136223846793005 + 1442695040888963407 */
    low_seed = uint2_mult(high_seed, low_seed, (uinttype) INT_SUFFIX(0), (uinttype) INT_SUFFIX(6364136223846793005),
        &high_seed);
    low_seed = uint2_add(high_seed, low_seed, (uinttype) INT_SUFFIX(0), (uinttype) INT_SUFFIX(1442695040888963407),
        &high_seed);
#endif
#ifdef TRACE_RANDOM
    printf("END uint_rand ==> %08x\n", (unsigned int) high_seed);
#endif
    return high_seed;
  } /* uint_rand */



int uint8MostSignificantBit (uint8type number)

  {
    int result;

  /* uint8MostSignificantBit */
    result = most_significant[number];
    return result;
  } /* uint8MostSignificantBit */



int uint16MostSignificantBit (uint16type number)

  {
    int result;

  /* uint16MostSignificantBit */
    if (number & 0xff00) {
      result = 8 + most_significant[number >> 8];
    } else {
      result = most_significant[number];
    } /* if */
    return result;
  } /* uint16MostSignificantBit */



int uint32MostSignificantBit (uint32type number)

  {
    int result;

  /* uint32MostSignificantBit */
    result = 0;
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
  } /* uint32MostSignificantBit */



#ifdef INT64TYPE
int uint64MostSignificantBit (uint64type number)

  {
    int result;

  /* uint64MostSignificantBit */
    result = 0;
#ifdef INT64TYPE_SUFFIX_LL
    if (number & 0xffffffff00000000LL) {
#else
    if (number & 0xffffffff00000000) {
#endif
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



int uint8LeastSignificantBit (uint8type number)

  {
    int result;

  /* uint8LeastSignificantBit */
    result = least_significant[number & 0xff];
    return result;
  } /* uint8LeastSignificantBit */



int uint16LeastSignificantBit (uint16type number)

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



int uint32LeastSignificantBit (uint32type number)

  {
    int result;

  /* uint32LeastSignificantBit */
    result = 0;
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
  } /* uint32LeastSignificantBit */



#ifdef INT64TYPE
int uint64LeastSignificantBit (uint64type number)

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
inttype genericCmp (const generictype value1, const generictype value2)

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
 *  E.g.: inttype, floattype, chartype, booltype.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. The function genericCpy is
 *  used from hashtables where the keys and the data is stored in
 *  generictype data elements. This assures correct behaviour
 *  even when sizeof(generictype) != sizeof(inttype).
 */
void genericCpy (generictype *const dest, const generictype source)

  { /* genericCpy */
    *dest = source;
  } /* genericCpy */



/**
 *  Generic Create function to be used via function pointers.
 *  This functions is used to create new values for all types
 *  where a binary copy without special funtionality can be used:
 *  E.g.: inttype, floattype, chartype, booltype.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. The function genericCreate is
 *  used from hashtables where the keys and the data is stored in
 *  generictype data elements. This assures correct behaviour
 *  even when sizeof(generictype) != sizeof(inttype). Even for
 *  sizeof(generictype) == sizeof(inttype) some things must
 *  be considered: On some architectures (linux with gcc)
 *  functions with float results seem to be returned in a
 *  different way (may be another register). Therefore
 *  genericCreate (with generictype) must be used even
 *  when sizeof(generictype) == sizeof(floattype).
 */
generictype genericCreate (generictype source)

  { /* genericCreate */
    return source;
  } /* genericCreate */



/**
 *  Generic Destr function to be used via function pointers.
 *  This functions is used to destroy old values for all types
 *  where a noop without special funtionality can be used:
 *  E.g.: inttype, floattype, chartype, booltype.
 *  In contrast to prcNoop the number of parameters (1) of
 *  genericDestr is correct. This is important since some
 *  C compilers generate wrong code when prcNoop is used.
 */
void genericDestr (generictype old_value)

  { /* genericDestr */
  } /* genericDestr */



/**
 *  Compare two pointers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
inttype ptrCmp (const void *const value1, const void *const value2)

  { /* ptrCmp */
    if ((memsizetype) value1 < (memsizetype) value2) {
      return -1;
    } else if ((memsizetype) value1 > (memsizetype) value2) {
      return 1;
    } else {
      return 0;
    } /* if */
  } /* ptrCmp */



/**
 *  Reinterpret the generic parameters as rtlPtrtype and call ptrCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(generictype) != sizeof(rtlPtrtype).
 */
inttype ptrCmpGeneric (const generictype value1, const generictype value2)

  { /* ptrCmpGeneric */
    return ptrCmp(((const_rtlObjecttype *) &value1)->value.ptrvalue,
                  ((const_rtlObjecttype *) &value2)->value.ptrvalue);
  } /* ptrCmpGeneric */



void ptrCpyGeneric (generictype *const dest, const generictype source)

  { /* ptrCpyGeneric */
    ((rtlObjecttype *) dest)->value.ptrvalue =
        ((const_rtlObjecttype *) &source)->value.ptrvalue;
  } /* ptrCpyGeneric */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(generictype) != sizeof(rtlPtrtype).
 */
generictype ptrCreateGeneric (const generictype from_value)

  {
    rtlObjecttype result;

  /* ptrCreateGeneric */
    result.value.ptrvalue =
        ((const_rtlObjecttype *) &from_value)->value.ptrvalue;
    return result.value.genericvalue;
  } /* ptrCreateGeneric */



/**
 *  Binomial coefficient
 *  @return n over k
 */
inttype intBinom (inttype n_number, inttype k_number)

  {
    inttype number;
    uinttype unsigned_result;
    inttype result;

  /* intBinom */
    /* printf("(%ld ! %ld) ", k_number, n_number); */
    if (n_number > 0 && 2 * k_number > n_number) {
      k_number = n_number - k_number;
    } /* if */
    if (k_number < 0) {
      result = 0;
    } else if (k_number == 0) {
      result = 1;
    } else /* if (n_number <= 30 || k_number <= 7)  */{
      if (n_number < 0) {
        result = n_number;
        for (number = 2; number <= k_number; number++) {
          result *= (n_number - number + 1);
          result /= number;
        } /* for */
      } else {
        unsigned_result = (uinttype) n_number;
        for (number = 2; number <= k_number; number++) {
          unsigned_result *= (uinttype) (n_number - number + 1);
          unsigned_result /= (uinttype) number;
        } /* for */
        result = (inttype) unsigned_result;
      } /* if */
/*
    } else {
      result = binom(n_number - 1, k_number - 1) +
          binom(n_number - 1, k_number);
*/
    } /* if */
    /* printf("--> %ld\n", result); */
    return result;
  } /* intBinom */



/**
 *  Number of bits in the minimal two's-complement representation.
 *  The high bits equivalent to the sign bit are not part of the
 *  minimal two's-complement representation.
 *  @return the number of bits.
 */
inttype intBitLength (inttype number)

  {
    inttype result;

  /* intBitLength */
    if (number < 0) {
      number = ~number;
    } /* if */
    result = uintMostSignificantBit((uinttype) number) + 1;
    return result;
  } /* intBitLength */



/**
 *  Compare two integer numbers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
inttype intCmp (inttype number1, inttype number2)

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
 *  Reinterpret the generic parameters as inttype and call intCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(generictype) != sizeof(inttype).
 */
inttype intCmpGeneric (const generictype value1, const generictype value2)

  { /* intCmpGeneric */
    return intCmp(((const_rtlObjecttype *) &value1)->value.intvalue,
                  ((const_rtlObjecttype *) &value2)->value.intvalue);
  } /* intCmpGeneric */



void intCpy (inttype *dest, inttype source)

  { /* intCpy */
    *dest = source;
  } /* intCpy */



/**
 *  Compute the truncated base 2 logarithm of an integer number.
 *  The definition of intLog2 is extended by defining intLog2(0) = -1.
 *  @return the truncated base 2 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
inttype intLog2 (inttype number)

  {
    int result;

  /* intLog2 */
    if (unlikely(number < 0)) {
      raise_error(NUMERIC_ERROR);
      result = 0;
    } else {
      result = uintMostSignificantBit((uinttype) number);
    } /* if */
    return result;
  } /* intLog2 */



/**
 *  Index of the lowest-order one bit.
 *  For A <> 0 this is equal to the number of lowest-order zero bits.
 *  @return the number of lowest-order zero bits or -1 for lowestSetBit(0).
 */
inttype intLowestSetBit (inttype number)

  {
    inttype result;

  /* intLowestSetBit */
    result = 0;
    if ((number & 0xffff) == 0) {
      number >>= 16;
      result += 16;
    } /* if */
    if ((number & 0xff) == 0) {
      number >>= 8;
      result += 8;
    } /* if */
    result += least_significant[number & 0xff];
    return (inttype) result;
  } /* intLowestSetBit */



stritype intLpad0 (inttype number, const inttype pad_size)

  {
    uinttype unsigned_number;
    booltype negative;
    strelemtype *buffer;
    memsizetype length;
    memsizetype result_size;
    stritype result;

  /* intLpad0 */
    negative = (number < 0);
    if (negative) {
      unsigned_number = (uinttype) -number;
    } else {
      unsigned_number = (uinttype) number;
    } /* if */
    length = DECIMAL_DIGITS(unsigned_number);
    if (negative) {
      length++;
    } /* if */
    if (pad_size > (inttype) length) {
      if (unlikely((uinttype) pad_size > MAX_STRI_LEN)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        result_size = (memsizetype) pad_size;
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
        *(--buffer) = (strelemtype) (unsigned_number % 10 + '0');
      } while ((unsigned_number /= 10) != 0);
      if (buffer != result->mem) {
        while (buffer != &result->mem[1]) {
          *(--buffer) = (strelemtype) '0';
        } /* while */
        if (negative) {
          result->mem[0] = (strelemtype) '-';
        } else {
          result->mem[0] = (strelemtype) '0';
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
inttype intParse (const const_stritype stri)

  {
    booltype okay;
    booltype negative;
    memsizetype position;
    inttype digitval;
    inttype integer_value;

  /* intParse */
    okay = TRUE;
    position = 0;
    integer_value = 0;
    if (stri->size != 0) {
      if (stri->mem[0] == ((strelemtype) '-')) {
        negative = TRUE;
        position++;
      } else {
        if (stri->mem[0] == ((strelemtype) '+')) {
          position++;
        } /* if */
        negative = FALSE;
      } /* if */
      while (position < stri->size &&
          stri->mem[position] >= ((strelemtype) '0') &&
          stri->mem[position] <= ((strelemtype) '9')) {
        digitval = ((inttype) stri->mem[position]) - ((inttype) '0');
        if (integer_value < MAX_DIV_10 ||
            (integer_value == MAX_DIV_10 &&
            digitval <= MAX_REM_10)) {
          integer_value = ((inttype) 10) * integer_value + digitval;
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
        return -integer_value;
      } else {
        return integer_value;
      } /* if */
    } else {
      raise_error(RANGE_ERROR);
      return 0;
    } /* if */
  } /* intParse */



/**
 *  Compute the exponentiation of a integer base with an integer exponent.
 *  @return the result of the exponentation.
 *  @exception NUMERIC_ERROR When the exponent is negative.
 */
inttype intPow (inttype base, inttype exponent)

  {
    inttype result;

  /* intPow */
    if (unlikely(exponent < 0)) {
      raise_error(NUMERIC_ERROR);
      result = 0;
    } else {
      if (exponent & 1) {
        result = base;
      } else {
        result = 1;
      } /* if */
      exponent >>= 1;
      while (exponent != 0) {
        base *= base;
        if (exponent & 1) {
          result *= base;
        } /* if */
        exponent >>= 1;
      } /* while */
    } /* if */
    return result;
  } /* intPow */



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
stritype intRadix (inttype number, inttype base, booltype upperCase)

  {
    uinttype unsigned_number;
    booltype negative;
    const_cstritype digits;
    strelemtype buffer_1[75];
    strelemtype *buffer;
    memsizetype length;
    stritype result;

  /* intRadix */
    if (unlikely(base < 2 || base > 36)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      negative = (number < 0);
      if (negative) {
        /* The unsigned value is negated to avoid a signed integer */
        /* overflow when the smallest signed integer is negated.   */
        unsigned_number = -(uinttype) number;
      } else {
        unsigned_number = (uinttype) number;
      } /* if */
      digits = digitTable[upperCase];
      buffer = &buffer_1[75];
      do {
        *(--buffer) = (strelemtype) (digits[unsigned_number % (uinttype) base]);
      } while ((unsigned_number /= (uinttype) base) != 0);
      if (negative) {
        *(--buffer) = (strelemtype) '-';
      } /* if */
      length = (memsizetype) (&buffer_1[75] - buffer);
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = length;
        memcpy(result->mem, buffer, (size_t) (length * sizeof(strelemtype)));
      } /* if */
    } /* if */
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
stritype intRadixPow2 (inttype number, int shift, int mask, booltype upperCase)

  {
    uinttype unsigned_number;
    booltype negative;
    const_cstritype digits;
    strelemtype buffer_1[50];
    strelemtype *buffer;
    memsizetype length;
    stritype result;

  /* intRadixPow2 */
    negative = (number < 0);
    if (negative) {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow when the smallest signed integer is negated.   */
      unsigned_number = -(uinttype) number;
    } else {
      unsigned_number = (uinttype) number;
    } /* if */
    digits = digitTable[upperCase];
    buffer = &buffer_1[50];
    do {
      *(--buffer) = (strelemtype) (digits[unsigned_number & (uinttype) mask]);
    } while ((unsigned_number >>= shift) != 0);
    if (negative) {
      *(--buffer) = (strelemtype) '-';
    } /* if */
    length = (memsizetype) (&buffer_1[50] - buffer);
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = length;
      memcpy(result->mem, buffer, (size_t) (length * sizeof(strelemtype)));
      return result;
    } /* if */
  } /* intRadixPow2 */



/**
 *  Compute pseudo-random number in the range [low, high].
 *  The random values are uniform distributed.
 *  @return a random number such that low <= rand(low, high) and
 *          rand(low, high) <= high holds.
 *  @exception RANGE_ERROR The range is empty (low > high holds).
 */
inttype intRand (inttype low, inttype high)

  {
    uinttype scale_limit;
    uinttype rand_max;
    uinttype rand_val;
    inttype result;

  /* intRand */
    if (unlikely(low >= high)) {
      if (low == high) {
        result = low;
      } else {
        raise_error(RANGE_ERROR);
        result = 0;
      } /* if */
    } else {
      scale_limit = (uinttype) high - (uinttype) low;
      if (scale_limit == UINTTYPE_MAX) {
        rand_val = uint_rand();
      } else {
        scale_limit++;
        rand_max = UINTTYPE_MAX - (UINTTYPE_MAX % scale_limit);
        do {
          rand_val = uint_rand();
        } while (rand_val > rand_max);
      } /* if */
      result = (inttype) ((uinttype) low + rand_val % scale_limit);
    } /* if */
    return result;
  } /* intRand */



/**
 *  Compute the integer square root of an integer number.
 *  @return the integer square root.
 *  @exception NUMERIC_ERROR When number is negative.
 */
inttype intSqrt (inttype number)

  {
    register uinttype result;
    register uinttype res2;

  /* intSqrt */
    if (unlikely(number < 0)) {
      raise_error(NUMERIC_ERROR);
      return 0;
    } else if (number == 0) {
      return (inttype) 0;
    } else {
      res2 = (uinttype) number;
      do {
        result = res2;
        res2 = (result + (uinttype) number / result) >> 1;
      } while (result > res2);
      return (inttype) result;
    } /* if */
  } /* intSqrt */



/**
 *  Convert an integer number to a string.
 *  The number is converted to a string with decimal representation.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
stritype intStr (inttype number)

  {
    register uinttype unsigned_number;
    booltype negative;
    strelemtype *buffer;
    memsizetype length;
    stritype result;

  /* intStr */
    negative = (number < 0);
    if (negative) {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow when the smallest signed integer is negated.   */
      unsigned_number = -(uinttype) number;
    } else {
      unsigned_number = (uinttype) number;
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
        *(--buffer) = (strelemtype) (unsigned_number % 10 + '0');
      } while ((unsigned_number /= 10) != 0);
      if (negative) {
        result->mem[0] = (strelemtype) '-';
      } /* if */
    } /* if */
    return result;
  } /* intStr */



#ifdef ALLOW_STRITYPE_SLICES
stritype intStrToBuffer (inttype number, stritype buffer)

  {
    register uinttype unsigned_number;
    booltype negative;
    strelemtype *bufferPtr;

  /* intStrToBuffer */
    /* printf("intStrToBuffer(");
    prot_int(number);
    printf(")");
    prot_flush(); */
    negative = (number < 0);
    if (negative) {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow when the smallest signed integer is negated.   */
      unsigned_number = -(uinttype) number;
    } else {
      unsigned_number = (uinttype) number;
    } /* if */
    bufferPtr = &buffer->mem1[INTTYPE_DECIMAL_DIGITS + 1];
    do {
      *(--bufferPtr) = (strelemtype) (unsigned_number % 10 + '0');
    } while ((unsigned_number /= 10) != 0);
    if (negative) {
      *(--bufferPtr) = (strelemtype) '-';
    } /* if */
    buffer->mem = bufferPtr;
    buffer->size = (memsizetype) (&buffer->mem1[INTTYPE_DECIMAL_DIGITS + 1] - bufferPtr);
    /* printf(" --> ");
    prot_stri(buffer);
    printf("\n"); */
    return buffer;
  } /* intStrToBuffer */
#endif



/**
 *  Convert an integer into a big-endian bstring.
 *  The result uses a twos-complement representation with a base of 256.
 *  For a negative 'number' the most significant byte of the result
 *  (the first byte) has an ordinal >= 128.
 *  @return a bstring with the big-endian representation.
 */
bstritype intToBStriBe (inttype number)

  {
    uchartype buffer[8];
    int pos = 7;
    bstritype result;

  /* intToBStriBe */
    /* printf("intToBStriBe(%016lx)\n", number); */
    if (number > 0) {
      do {
        buffer[pos] = (uchartype) (number & 0xff);
        number >>= 8;
        pos--;
      } while (number != 0);
      if (buffer[pos + 1] <= 127) {
        pos++;
      } else {
        buffer[pos] = 0;
      } /* if */
    } else if (number < 0) {
      do {
        buffer[pos] = (uchartype) (number & 0xff);
#ifdef RSHIFT_DOES_SIGN_EXTEND
        number >>= 8;
#else
        number = ~(~number >> 8);
#endif
        pos--;
      } while (number != -1);
      if (buffer[pos + 1] >= 128) {
        pos++;
      } else {
        buffer[pos] = 255;
      } /* if */
    } else {
      buffer[7] = '\0';
    } /* if */
    if (!ALLOC_BSTRI_SIZE_OK(result, (memsizetype) (8 - pos))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = (memsizetype) (8 - pos);
      memcpy(result->mem, &buffer[pos],
             (memsizetype) (8 - pos) * sizeof(uchartype));
    } /* if */
    return result;
  } /* intToBStriBe */



/**
 *  Convert an integer into a little-endian bstring.
 *  The result uses a twos-complement representation with a base of 256.
 *  For a negative 'number' the most significant byte of the result
 *  (the last byte) has an ordinal >= 128.
 *  @return a bstring with the little-endian representation.
 */
bstritype intToBStriLe (inttype number)

  {
    uchartype buffer[8];
    int pos = 0;
    bstritype result;

  /* intToBStriLe */
    /* printf("intToBStriLe(%016lx)\n", number); */
    if (number > 0) {
      do {
        buffer[pos] = (uchartype) (number & 0xff);
        number >>= 8;
        pos++;
      } while (number != 0);
      if (buffer[pos - 1] <= 127) {
        pos--;
      } else {
        buffer[pos] = 0;
      } /* if */
    } else if (number < 0) {
      do {
        buffer[pos] = (uchartype) (number & 0xff);
#ifdef RSHIFT_DOES_SIGN_EXTEND
        number >>= 8;
#else
        number = ~(~number >> 8);
#endif
        pos++;
      } while (number != -1);
      if (buffer[pos - 1] >= 128) {
        pos--;
      } else {
        buffer[pos] = 255;
      } /* if */
    } else {
      buffer[0] = '\0';
    } /* if */
    if (!ALLOC_BSTRI_SIZE_OK(result, (memsizetype) (pos + 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = (memsizetype) (pos + 1);
      memcpy(result->mem, &buffer[0],
             (memsizetype) (pos + 1) * sizeof(uchartype));
    } /* if */
    return result;
  } /* intToBStriLe */
