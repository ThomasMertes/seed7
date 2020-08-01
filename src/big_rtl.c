/********************************************************************/
/*                                                                  */
/*  big_rtl.c     Primitive actions for the bigInteger type.        */
/*  Copyright (C) 1989 - 2006  Thomas Mertes                        */
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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/big_rtl.c                                       */
/*  Changes: 2005, 2006  Thomas Mertes                              */
/*  Content: Primitive actions for the bigInteger type.             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "heaputl.h"
#include "int_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "big_rtl.h"


/* Defines to describe a bigdigit:                                  */
/* BIGDIGIT_MASK              All bits in a bigdigit are set.       */
/* BIGDIGIT_SIGN              The highest bit of a bigdigit is set. */
/* POWER_OF_10_IN_BIGDIGIT    The biggest power of 10 which fits    */
/*                            in a bigdigit.                        */
/* DECIMAL_DIGITS_IN_BIGDIGIT The number of zero digits in          */
/*                            POWER_OF_10_IN_BIGDIGIT.              */

#if BIGDIGIT_SIZE == 8

#define BIGDIGIT_MASK                    0xFF
#define BIGDIGIT_SIGN                    0x80
#define POWER_OF_10_IN_BIGDIGIT           100
#define DECIMAL_DIGITS_IN_BIGDIGIT          2

#elif BIGDIGIT_SIZE == 16

#define BIGDIGIT_MASK                  0xFFFF
#define BIGDIGIT_SIGN                  0x8000
#define POWER_OF_10_IN_BIGDIGIT         10000
#define DECIMAL_DIGITS_IN_BIGDIGIT          4

#elif BIGDIGIT_SIZE == 32

#define BIGDIGIT_MASK              0xFFFFFFFF
#define BIGDIGIT_SIGN              0x80000000
#define POWER_OF_10_IN_BIGDIGIT    1000000000
#define DECIMAL_DIGITS_IN_BIGDIGIT          9

#endif

#define IS_NEGATIVE(digit) ((digit) & BIGDIGIT_SIGN)



/**
 *  Remove leading zero (or BIGDIGIT_MASK) digits from a
 *  signed big integer.
 */
#ifdef ANSI_C

static void normalize (biginttype big1)
#else

static void normalize (arg1)
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
      if (big1->size != pos) {
        if (!RESIZE_BIG(big1, big1->size, pos)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT3_BIG(big1->size, pos);
          big1->size = pos;
        } /* if */
      } /* if */
    } /* if */
  } /* normalize */



#ifdef ANSI_C

static void negate_positive_big (biginttype big1)
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
      carry >>= 8 * sizeof(bigdigittype);
      pos++;
    } while (pos < big1->size);
  } /* negate_positive_big */



#ifdef ANSI_C

static void positive_copy_of_negative_big (biginttype dest, biginttype big1)
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
      carry >>= 8 * sizeof(bigdigittype);
      pos++;
    } while (pos < big1->size);
    if (IS_NEGATIVE(dest->bigdigits[pos - 1])) {
      dest->bigdigits[pos] = 0;
      pos++;
    } /* if */
    dest->size = pos;
  } /* positive_copy_of_negative_big */



#ifdef ANSI_C

static biginttype alloc_positive_copy_of_negative_big (biginttype big1)
#else

static biginttype alloc_positive_copy_of_negative_big (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;
    biginttype result;

  /* alloc_positive_copy_of_negative_big */
    if (!ALLOC_BIG(result, big1->size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(big1->size);
      pos = 0;
      do {
        carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
        pos++;
      } while (pos < big1->size);
      result->size = pos;
      return(result);
    } /* if */
  } /* alloc_positive_copy_of_negative_big */



/**
 *  Multiplies big1 by 10 and adds carry to the result. This
 *  function works for unsigned big integers. It is assumed
 *  that big1 contains enough memory.
 */
#ifdef ANSI_C

static void uBigMultBy10AndAdd (biginttype big1, doublebigdigittype carry)
#else

static void uBigMultBy10AndAdd (big1, carry)
biginttype big1;
doublebigdigittype carry;
#endif

  {
    memsizetype pos;

  /* uBigMultBy10AndAdd */
    pos = 0;
    do {
      carry += (doublebigdigittype) big1->bigdigits[pos] * 10;
      big1->bigdigits[pos] = (bigdigittype) carry;
      carry >>= 8 * sizeof(bigdigittype);
      pos++;
    } while (pos < big1->size);
    if (carry != 0) {
      big1->bigdigits[pos] = carry;
      big1->size++;
    } /* if */
  } /* uBigMultBy10AndAdd */



/**
 *  Divides the unsigned big integer big1 by POWER_OF_10_IN_BIGDIGIT
 *  and returns the remainder.
 */
#ifdef ANSI_C

static bigdigittype uBigDivideByPowerOf10 (biginttype big1)
#else

static bigdigittype uBigDivideByPowerOf10 (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;

  /* uBigDivideByPowerOf10 */
    pos = big1->size;
    do {
      pos--;
      carry <<= 8 * sizeof(bigdigittype);
      carry += big1->bigdigits[pos];
      big1->bigdigits[pos] = (bigdigittype) (carry / POWER_OF_10_IN_BIGDIGIT);
      carry %= POWER_OF_10_IN_BIGDIGIT;
    } while (pos > 0);
    return(carry);
  } /* uBigDivideByPowerOf10 */



/**
 *  Shifts the big integer big1 to the left by lshift bits.
 *  Bits which are shifted out at the left of big1 are lost.
 *  At the right of big1 zero bits are shifted in.
 */
#ifdef ANSI_C

static void uBigLShift (biginttype big1, unsigned int lshift)
#else

static void uBigLShift (big1, lshift)
biginttype big1;
unsigned int lshift;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;

  /* uBigLShift */
    pos = 0;
    do {
      carry |= ((doublebigdigittype) big1->bigdigits[pos]) << lshift;
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
      pos++;
    } while (pos < big1->size);
  } /* uBigLShift */



/**
 *  Shifts the big integer big1 to the right by rshift bits.
 *  Bits which are shifted out at the right of big1 are lost.
 *  At the left of big1 zero bits are shifted in.
 */
#ifdef ANSI_C

static void uBigRShift (biginttype big1, unsigned int rshift)
#else

static void uBigRShift (big1, rshift)
biginttype big1;
unsigned int rshift;
#endif

  {
    unsigned int lshift = 8 * sizeof(bigdigittype) - rshift;
    bigdigittype low_digit;
    bigdigittype high_digit;
    memsizetype pos;

  /* uBigRShift */
    if (lshift == 0) {
      for (pos = 1; pos < big1->size; pos++) {
        big1->bigdigits[pos - 1] = big1->bigdigits[pos];
      } /* for */
      big1->bigdigits[pos - 1] = 0;
    } else if (rshift != 0) {
      low_digit = big1->bigdigits[0];
      for (pos = 1; pos < big1->size; pos++) {
        high_digit = big1->bigdigits[pos];
        big1->bigdigits[pos - 1] = (bigdigittype)
            ((low_digit >> rshift) | (high_digit << lshift));
        low_digit = high_digit;
      } /* for */
      big1->bigdigits[pos - 1] = (bigdigittype) (low_digit >> rshift);
    } /* if */
  } /* uBigRShift */



/**
 *  Increments an unsigned big integer by 1. This function does
 *  overflow silently when big1 contains not enough digits.
 */
#ifdef ANSI_C

static void uBigIncr (biginttype big1)
#else

static void uBigIncr (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;

  /* uBigIncr */
    pos = 0;
    do {
      carry += big1->bigdigits[pos];
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
      pos++;
    } while (carry != 0 && pos < big1->size);
  } /* uBigIncr */



/**
 *  Computes an integer division of big1 by one digit for
 *  nonnegative big integers. The digit must not be zero.
 */
#ifdef ANSI_C

static void uBigDiv1 (biginttype big1, bigdigittype digit, biginttype result)
#else

static void uBigDiv1 (big1, digit, result)
biginttype big1;
bigdigittype digit;
biginttype result;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;

  /* uBigDiv1 */
    pos = big1->size;
    do {
      pos--;
      carry <<= 8 * sizeof(bigdigittype);
      carry += big1->bigdigits[pos];
      result->bigdigits[pos] = (bigdigittype) (carry / digit);
      carry %= digit;
    } while (pos > 0);
  } /* uBigDiv1 */



/**
 *  Computes an integer division of big1 by one digit for
 *  signed big integers. The memory for the result is
 *  requested and the normalized result is returned. This
 *  function handles also the special case of a division by
 *  zero.
 */
#ifdef ANSI_C

static biginttype bigDiv1 (biginttype big1, bigdigittype digit)
#else

static biginttype bigDiv1 (big1, digit)
biginttype big1;
bigdigittype digit;
#endif

  {
    booltype negative = FALSE;
    biginttype big1_help = NULL;
    biginttype result;

  /* bigDiv1 */
    if (digit == 0) {
      raise_error(NUMERIC_ERROR);
      return(NULL);
    } else {
      if (!ALLOC_BIG(result, big1->size + 1)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big1->size + 1);
        result->size = big1->size + 1;
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          negative = TRUE;
          big1_help = alloc_positive_copy_of_negative_big(big1);
          big1 = big1_help;
          if (big1_help == NULL) {
            FREE_BIG(result, result->size);
            return(NULL);
          } /* if */
        } /* if */
        result->bigdigits[result->size - 1] = 0;
        if (IS_NEGATIVE(digit)) {
          negative = !negative;
          digit = -digit;
        } /* if */
        uBigDiv1(big1, digit, result);
        if (negative) {
          negate_positive_big(result);
        } /* if */
        normalize(result);
        if (big1_help != NULL) {
          FREE_BIG(big1_help, big1_help->size);
        } /* if */
        return(result);
      } /* if */
    } /* if */
  } /* bigDiv1 */



/**
 *  Computes an integer division of big1 by big2 for signed big
 *  integers when big1 has less digits than big2. The memory for
 *  the result is requested and the normalized result is returned.
 *  Normally big1->size < big2->size implies abs(big1) < abs(big2).
 *  When abs(big1) < abs(big2) holds the result is 0. The cases
 *  when big1->size < big2->size and abs(big1) = abs(big2) are if
 *  big1->size + 1 == big2->size and big1 = 0x8000 (0x80000000...)
 *  and big2 = 0x00008000 (0x000080000000...). In this cases the
 *  result is -1. In all other cases the result is 0.
 */
#ifdef ANSI_C

static biginttype bigDivSizeLess (biginttype big1, biginttype big2)
#else

static biginttype bigDivSizeLess (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    memsizetype pos;
    biginttype result;

  /* bigDivSizeLess */
    if (!ALLOC_BIG(result, 1)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(1);
      result->size = 1;
      if (big1->size + 1 == big2->size &&
          big1->bigdigits[big1->size - 1] == BIGDIGIT_SIGN &&
          big2->bigdigits[big2->size - 1] == 0 &&
          big2->bigdigits[big2->size - 2] == BIGDIGIT_SIGN) {
        result->bigdigits[0] = BIGDIGIT_MASK;
        for (pos = 0; pos < big1->size - 1; pos++) {
          if (big1->bigdigits[pos] != 0 || big2->bigdigits[pos] != 0) {
            result->bigdigits[0] = 0;
          } /* if */
        } /* for */
      } else {
        result->bigdigits[0] = 0;
      } /* if */
      return(result);
    } /* if */
  } /* bigDivSizeLess */



/**
 *  Multiplies big2 with multiplier and subtracts the result from
 *  big1 at the digit position pos1 of big1. Big1, big2 and
 *  multiplier are nonnegative big integer values.
 */
#ifdef ANSI_C

static bigdigittype uBigMultSub (biginttype big1, biginttype big2,
    bigdigittype multiplier, memsizetype pos1)
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
      sbtr_carry += big1->bigdigits[pos + pos1] + (~mult_carry & BIGDIGIT_MASK);
      big1->bigdigits[pos + pos1] = (bigdigittype) (sbtr_carry & BIGDIGIT_MASK);
      mult_carry >>= 8 * sizeof(bigdigittype);
      sbtr_carry >>= 8 * sizeof(bigdigittype);
      pos++;
    } while (pos < big2->size);
    for (pos += pos1; pos < big1->size; pos++) {
      sbtr_carry += big1->bigdigits[pos] + (~mult_carry & BIGDIGIT_MASK);
      big1->bigdigits[pos] = (bigdigittype) (sbtr_carry & BIGDIGIT_MASK);
      mult_carry >>= 8 * sizeof(bigdigittype);
      sbtr_carry >>= 8 * sizeof(bigdigittype);
    } /* for */
    return((sbtr_carry + BIGDIGIT_MASK) & BIGDIGIT_MASK);
  } /* uBigMultSub */



/**
 *  Adds big2 to big1 at the digit position pos1. Big1 and big2
 *  are nonnegative big integer values. The size of big1 must be
 *  greater or equal the size of big2.
 */
#ifdef ANSI_C

static void uBigAddTo (biginttype big1, biginttype big2, memsizetype pos1)
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
      carry += (doublebigdigittype) big1->bigdigits[pos + pos1] + big2->bigdigits[pos];
      big1->bigdigits[pos + pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
      pos++;
    } while (pos < big2->size);
    for (; pos < big1->size; pos++) {
      carry += big1->bigdigits[pos + pos1];
      big1->bigdigits[pos + pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
    } /* for */
  } /* uBigAddTo */



/**
 *  Computes an integer division of big1 by big2 for nonnegative big
 *  integers. The remainder is delivered in big1. There are several
 *  preconditions. Big2 must have at least 2 digits and big1 must have
 *  at least one digit more than big2. The most significant bit of big2
 *  must be set. The most significant digit of big1 must be less than
 *  the most significant digit of big2. The computations to meet this
 *  predonditions are done outside this function. The special cases
 *  with a one digit big2 or a big1 with less digits than big2 are
 *  handled in other functions. This algorithm based on the algorithm
 *  from D.E. Knuth described in "The art of computer programming"
 *  volume 2 (Seminumerical algorithms).
 */
#ifdef ANSI_C

static void uBigDiv (biginttype big1, biginttype big2, biginttype result)
#else

static void uBigDiv (big1, big2, result)
biginttype big1;
biginttype big2;
biginttype result;
#endif

  {
    memsizetype pos1;
    doublebigdigittype twodigits;
    doublebigdigittype remainder;
    bigdigittype quotientdigit;
    bigdigittype borrow;

  /* uBigDiv */
    for (pos1 = big1->size - 1; pos1 >= big2->size; pos1--) {
      twodigits = (((doublebigdigittype) big1->bigdigits[pos1]) <<
          8 * sizeof(bigdigittype)) | big1->bigdigits[pos1 - 1];
      if (big1->bigdigits[pos1] == big2->bigdigits[big2->size - 1]) {
        quotientdigit = BIGDIGIT_MASK;
      } else {
        quotientdigit = twodigits / big2->bigdigits[big2->size - 1];
      } /* if */
      remainder = twodigits - (doublebigdigittype) quotientdigit *
          big2->bigdigits[big2->size - 1];
      while (remainder <= BIGDIGIT_MASK &&
          (doublebigdigittype) big2->bigdigits[big2->size - 2] * quotientdigit >
          (remainder << 8 * sizeof(bigdigittype) | big1->bigdigits[pos1 - 2])) {
        quotientdigit--;
        remainder = twodigits - (doublebigdigittype) quotientdigit *
            big2->bigdigits[big2->size - 1];
      } /* while */
      borrow = uBigMultSub(big1, big2, quotientdigit, pos1 - big2->size);
      if (borrow != 0) {
        uBigAddTo(big1, big2, pos1 - big2->size);
        quotientdigit--;
      } /* if */
      result->bigdigits[pos1 - big2->size] = quotientdigit;
    } /* for */
  } /* uBigDiv */



/**
 *  Computes the remainder of a integer division of big1 by
 *  one digit for nonnegative big integers. The digit must
 *  not be zero.
 */
#ifdef ANSI_C

static bigdigittype uBigRem1 (biginttype big1, bigdigittype digit)
#else

static bigdigittype uBigRem1 (big1, digit)
biginttype big1;
bigdigittype digit;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;

  /* uBigRem1 */
    pos = big1->size;
    do {
      pos--;
      carry <<= 8 * sizeof(bigdigittype);
      carry += big1->bigdigits[pos];
      carry %= digit;
    } while (pos > 0);
    return(carry);
  } /* uBigRem1 */



/**
 *  Computes the remainder of the integer division big1 by one digit
 *  for signed big integers. The memory for the remainder is requested
 *  and the normalized remainder is returned. This function handles also
 *  the special case of a division by zero.
 */
#ifdef ANSI_C

static biginttype bigRem1 (biginttype big1, bigdigittype digit)
#else

static biginttype bigRem1 (big1, digit)
biginttype big1;
bigdigittype digit;
#endif

  {
    booltype negative = FALSE;
    biginttype big1_help = NULL;
    biginttype remainder;

  /* bigRem1 */
    if (digit == 0) {
      raise_error(NUMERIC_ERROR);
      return(NULL);
    } else {
      if (!ALLOC_BIG(remainder, 1)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(1);
        remainder->size = 1;
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          negative = TRUE;
          big1_help = alloc_positive_copy_of_negative_big(big1);
          big1 = big1_help;
          if (big1_help == NULL) {
            FREE_BIG(remainder, remainder->size);
            return(NULL);
          } /* if */
        } /* if */
        if (IS_NEGATIVE(digit)) {
          digit = -digit;
        } /* if */
        remainder->bigdigits[0] = uBigRem1(big1, digit);
        if (negative) {
          negate_positive_big(remainder);
        } /* if */
        if (big1_help != NULL) {
          FREE_BIG(big1_help, big1_help->size);
        } /* if */
        return(remainder);
      } /* if */
    } /* if */
  } /* bigRem1 */



/**
 *  Computes an integer division of big1 by one digit for
 *  nonnegative big integers. The digit must not be zero.
 *  The remainder of the division is returned.
 */
#ifdef ANSI_C

static bigdigittype uBigMDiv1 (biginttype big1, bigdigittype digit, biginttype result)
#else

static bigdigittype uBigMDiv1 (big1, digit, result)
biginttype big1;
bigdigittype digit;
biginttype result;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;

  /* uBigMDiv1 */
    pos = big1->size;
    do {
      pos--;
      carry <<= 8 * sizeof(bigdigittype);
      carry += big1->bigdigits[pos];
      result->bigdigits[pos] = (bigdigittype) (carry / digit);
      carry %= digit;
    } while (pos > 0);
    return(carry);
  } /* uBigMDiv1 */



/**
 *  Computes an integer modulo division of big1 by one digit
 *  for signed big integers. The memory for the result is
 *  requested and the normalized result is returned. This
 *  function handles also the special case of a division by
 *  zero.
 */
#ifdef ANSI_C

static biginttype bigMDiv1 (biginttype big1, bigdigittype digit)
#else

static biginttype bigMDiv1 (big1, digit)
biginttype big1;
bigdigittype digit;
#endif

  {
    booltype negative = FALSE;
    biginttype big1_help = NULL;
    bigdigittype remainder;
    biginttype result;

  /* bigMDiv1 */
    if (digit == 0) {
      raise_error(NUMERIC_ERROR);
      return(NULL);
    } else {
      if (!ALLOC_BIG(result, big1->size + 1)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big1->size + 1);
        result->size = big1->size + 1;
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          negative = TRUE;
          big1_help = alloc_positive_copy_of_negative_big(big1);
          big1 = big1_help;
          if (big1_help == NULL) {
            FREE_BIG(result, result->size);
            return(NULL);
          } /* if */
        } /* if */
        result->bigdigits[result->size - 1] = 0;
        if (IS_NEGATIVE(digit)) {
          negative = !negative;
          digit = -digit;
        } /* if */
        remainder = uBigMDiv1(big1, digit, result);
        if (negative) {
          if (remainder != 0) {
            uBigIncr(result);
          } /* if */
          negate_positive_big(result);
        } /* if */
        normalize(result);
        if (big1_help != NULL) {
          FREE_BIG(big1_help, big1_help->size);
        } /* if */
        return(result);
      } /* if */
    } /* if */
  } /* bigMDiv1 */



/**
 *  Computes a modulo integer division of big1 by big2 for signed
 *  big integers when big1 has less digits than big2. The memory for
 *  the result is requested and the normalized result is returned.
 *  Normally big1->size < big2->size implies abs(big1) < abs(big2).
 *  When abs(big1) < abs(big2) holds the result is 0 or -1. The cases
 *  when big1->size < big2->size and abs(big1) = abs(big2) are if
 *  big1->size + 1 == big2->size and big1 = 0x8000 (0x80000000...)
 *  and big2 = 0x00008000 (0x000080000000...). In this cases the
 *  result is -1. In the cases when the result is 0 or -1 the
 *  following check is done: When big1 and big2 have different signs
 *  the result is -1 otherwise the result is 0.
 */
#ifdef ANSI_C

static biginttype bigMDivSizeLess (biginttype big1, biginttype big2)
#else

static biginttype bigMDivSizeLess (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    memsizetype pos;
    biginttype result;

  /* bigMDivSizeLess */
    if (!ALLOC_BIG(result, 1)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(1);
      result->size = 1;
      if (big1->size + 1 == big2->size &&
          big1->bigdigits[big1->size - 1] == BIGDIGIT_SIGN &&
          big2->bigdigits[big2->size - 1] == 0 &&
          big2->bigdigits[big2->size - 2] == BIGDIGIT_SIGN) {
        result->bigdigits[0] = BIGDIGIT_MASK;
        for (pos = 0; pos < big1->size - 1; pos++) {
          if (big1->bigdigits[pos] != 0 || big2->bigdigits[pos] != 0) {
            result->bigdigits[0] = 0;
          } /* if */
        } /* for */
      } else {
        result->bigdigits[0] = 0;
      } /* if */
      if (result->bigdigits[0] == 0 &&
          IS_NEGATIVE(big1->bigdigits[big1->size - 1]) !=
          IS_NEGATIVE(big2->bigdigits[big2->size - 1])) {
        result->bigdigits[0] = BIGDIGIT_MASK;
      } /* if */
      return(result);
    } /* if */
  } /* bigMDivSizeLess */



/**
 *  Computes the modulo of the integer division big1 by one digit
 *  for signed big integers. The memory for the modulo is requested
 *  and the normalized modulo is returned. This function handles also
 *  the special case of a division by zero.
 */
#ifdef ANSI_C

static biginttype bigMod1 (biginttype big1, bigdigittype digit)
#else

static biginttype bigMod1 (big1, digit)
biginttype big1;
bigdigittype digit;
#endif

  {
    biginttype modulo;

  /* bigMod1 */
    modulo = bigRem1(big1, digit);
    if (IS_NEGATIVE(big1->bigdigits[big1->size - 1]) != IS_NEGATIVE(digit) &&
        modulo != NULL && modulo->bigdigits[0] != 0) {
      modulo->bigdigits[0] += digit;
    } /* if */
    return(modulo);
  } /* bigMod1 */



/**
 *  Computes the remainder of the integer division big1 by big2 for
 *  signed big integers when big1 has less digits than big2. The memory
 *  for the remainder is requested and the normalized remainder is returned.
 *  Normally big1->size < big2->size implies abs(big1) < abs(big2).
 *  When abs(big1) < abs(big2) holds the remainder is big1. The cases
 *  when big1->size < big2->size and abs(big1) = abs(big2) are if
 *  big1->size + 1 == big2->size and big1 = 0x8000 (0x80000000...)
 *  and big2 = 0x00008000 (0x000080000000...). In this cases the
 *  remainder is 0. In all other cases the remainder is big1.
 */
#ifdef ANSI_C

static biginttype bigRemSizeLess (biginttype big1, biginttype big2)
#else

static biginttype bigRemSizeLess (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    memsizetype pos;
    booltype remainderIs0;
    biginttype remainder;

  /* bigRemSizeLess */
    if (big1->size + 1 == big2->size &&
        big1->bigdigits[big1->size - 1] == BIGDIGIT_SIGN &&
        big2->bigdigits[big2->size - 1] == 0 &&
        big2->bigdigits[big2->size - 2] == BIGDIGIT_SIGN) {
      remainderIs0 = TRUE;
      for (pos = 0; pos < big1->size - 1; pos++) {
        if (big1->bigdigits[pos] != 0 || big2->bigdigits[pos] != 0) {
          remainderIs0 = FALSE;
        } /* if */
      } /* for */
    } else {
      remainderIs0 = FALSE;
    } /* if */
    if (remainderIs0) {
      if (!ALLOC_BIG(remainder, 1)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT_BIG(1);
        remainder->size = 1;
        remainder->bigdigits[0] = 0;
      } /* if */
    } else {
      if (!ALLOC_BIG(remainder, big1->size)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT_BIG(big1->size);
        remainder->size = big1->size;
        memcpy(remainder->bigdigits, big1->bigdigits,
            (SIZE_TYPE) big1->size * sizeof(bigdigittype));
      } /* if */
    } /* if */
    return(remainder);
  } /* bigRemSizeLess */



/**
 *  Adds big2 to big1 at the digit position pos1. Big1 and big2
 *  are signed big integer values. The size of big1 must be
 *  greater or equal the size of big2.
 */
#ifdef ANSI_C

static void bigAddTo (biginttype big1, biginttype big2)
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
      carry >>= 8 * sizeof(bigdigittype);
      pos++;
    } while (pos < big2->size);
    big2_sign = IS_NEGATIVE(big2->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
    for (; pos < big1->size; pos++) {
      carry += big1->bigdigits[pos] + big2_sign;
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
    } /* for */
  } /* bigAddTo */



/**
 *  Computes the modulo of the integer division big1 by big2 for
 *  signed big integers when big1 has less digits than big2. The memory
 *  for the modulo is requested and the normalized modulo is returned.
 *  Normally big1->size < big2->size implies abs(big1) < abs(big2).
 *  When abs(big1) < abs(big2) holds the division gives 0. The cases
 *  when big1->size < big2->size and abs(big1) = abs(big2) are if
 *  big1->size + 1 == big2->size and big1 = 0x8000 (0x80000000...)
 *  and big2 = 0x00008000 (0x000080000000...). In this cases the
 *  modulo is 0. In all other cases the modulo is big1 or big1 +
 *  big2 when big1 and big2 have different signs.
 */
#ifdef ANSI_C

static biginttype bigModSizeLess (biginttype big1, biginttype big2)
#else

static biginttype bigModSizeLess (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    memsizetype pos;
    booltype moduloIs0;
    biginttype modulo;

  /* bigModSizeLess */
    if (big1->size + 1 == big2->size &&
        big1->bigdigits[big1->size - 1] == BIGDIGIT_SIGN &&
        big2->bigdigits[big2->size - 1] == 0 &&
        big2->bigdigits[big2->size - 2] == BIGDIGIT_SIGN) {
      moduloIs0 = TRUE;
      for (pos = 0; pos < big1->size - 1; pos++) {
        if (big1->bigdigits[pos] != 0 || big2->bigdigits[pos] != 0) {
          moduloIs0 = FALSE;
        } /* if */
      } /* for */
    } else {
      moduloIs0 = FALSE;
    } /* if */
    if (moduloIs0) {
      if (!ALLOC_BIG(modulo, 1)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT_BIG(1);
        modulo->size = 1;
        modulo->bigdigits[0] = 0;
      } /* if */
    } else {
      if (IS_NEGATIVE(big1->bigdigits[big1->size - 1]) !=
          IS_NEGATIVE(big2->bigdigits[big2->size - 1])) {
        if (!ALLOC_BIG(modulo, big2->size)) {
          raise_error(MEMORY_ERROR);
        } else {
          COUNT_BIG(big2->size);
          modulo->size = big2->size;
          memcpy(modulo->bigdigits, big2->bigdigits,
              (SIZE_TYPE) big2->size * sizeof(bigdigittype));
          bigAddTo(modulo, big1);
          normalize(modulo);
        } /* if */
      } else {
        if (!ALLOC_BIG(modulo, big1->size)) {
          raise_error(MEMORY_ERROR);
        } else {
          COUNT_BIG(big1->size);
          modulo->size = big1->size;
          memcpy(modulo->bigdigits, big1->bigdigits,
              (SIZE_TYPE) big1->size * sizeof(bigdigittype));
        } /* if */
      } /* if */
    } /* if */
    return(modulo);
  } /* bigModSizeLess */



/**
 *  Computes the remainder of an integer division of big1 by big2
 *  for nonnegative big integers. The remainder is delivered in
 *  big1. There are several preconditions for this function. Big2
 *  must have at least 2 digits and big1 must have at least one
 *  digit more than big2. The most significant bit of big2 must be
 *  set. The most significant digit of big1 must be less than the
 *  most significant digit of big2. The computations to meet this
 *  predonditions are done outside this function. The special cases
 *  with a one digit big2 or a big1 with less digits than big2 are
 *  handled in other functions. This algorithm based on the algorithm
 *  from D.E. Knuth described in "The art of computer programming"
 *  volume 2 (Seminumerical algorithms).
 */
#ifdef ANSI_C

static void uBigRem (biginttype big1, biginttype big2)
#else

static void uBigRem (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    memsizetype pos1;
    doublebigdigittype twodigits;
    doublebigdigittype remainder;
    bigdigittype quotientdigit;
    bigdigittype borrow;

  /* uBigRem */
    for (pos1 = big1->size - 1; pos1 >= big2->size; pos1--) {
      twodigits = (((doublebigdigittype) big1->bigdigits[pos1]) <<
          8 * sizeof(bigdigittype)) | big1->bigdigits[pos1 - 1];
      if (big1->bigdigits[pos1] == big2->bigdigits[big2->size - 1]) {
        quotientdigit = BIGDIGIT_MASK;
      } else {
        quotientdigit = twodigits / big2->bigdigits[big2->size - 1];
      } /* if */
      remainder = twodigits - (doublebigdigittype) quotientdigit *
          big2->bigdigits[big2->size - 1];
      while (remainder <= BIGDIGIT_MASK &&
          (doublebigdigittype) big2->bigdigits[big2->size - 2] * quotientdigit >
          (remainder << 8 * sizeof(bigdigittype) | big1->bigdigits[pos1 - 2])) {
        quotientdigit--;
        remainder = twodigits - (doublebigdigittype) quotientdigit *
            big2->bigdigits[big2->size - 1];
      } /* while */
      borrow = uBigMultSub(big1, big2, quotientdigit, pos1 - big2->size);
      if (borrow != 0) {
        uBigAddTo(big1, big2, pos1 - big2->size);
        quotientdigit--;
      } /* if */
    } /* for */
  } /* uBigRem */



/**
 *  Returns the sqare of the nonnegative big integer big1. The result
 *  is written into big_help (which is a scratch variable and is
 *  assumed to contain enough memory). Before returning the result
 *  the variable big1 is assigned to big_help. This way it is possible
 *  to square a given base with base = uBigSqare(base, &big_help).
 *  Note that the old base is in the scratch variable big_help
 *  afterwards. This squaring algorithm takes into account that
 *  digit1 * digit2 + digit2 * digit1 == (digit1 * digit2) << 1.
 *  This reduces the number of multiplications approx. by factor 2.
 *  Unfortunately one bit more than sizeof(doublebigdigittype) is
 *  needed to store the shifted product. Therefore extra effort is
 *  necessary to avoid an overflow.
 */
#ifdef ANSI_C

static biginttype uBigSqare (biginttype big1, biginttype *big_help)
#else

static biginttype uBigSqare (big1, big_help)
biginttype big1;
biginttype *big_help;
#endif

  {
    memsizetype pos1;
    memsizetype pos2;
    doublebigdigittype carry = 0;
    doublebigdigittype product;
    bigdigittype digit;
    biginttype result;

  /* uBigSqare */
    result = *big_help;
    digit = big1->bigdigits[0];
    carry = (doublebigdigittype) digit * digit;
    result->bigdigits[0] = (bigdigittype) (carry & BIGDIGIT_MASK);
    carry >>= 8 * sizeof(bigdigittype);
    for (pos2 = 1; pos2 < big1->size; pos2++) {
      product = (doublebigdigittype) digit * big1->bigdigits[pos2];
      carry += (product << 1) & BIGDIGIT_MASK;
      result->bigdigits[pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
      carry += product >> (8 * sizeof(bigdigittype) - 1);
    } /* for */
    result->bigdigits[pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
    for (pos1 = 1; pos1 < big1->size; pos1++) {
      digit = big1->bigdigits[pos1];
      carry = (doublebigdigittype) result->bigdigits[pos1 + pos1] +
          (doublebigdigittype) digit * digit;
      result->bigdigits[pos1 + pos1] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
      for (pos2 = pos1 + 1; pos2 < big1->size; pos2++) {
        product = (doublebigdigittype) digit * big1->bigdigits[pos2];
        carry += (doublebigdigittype) result->bigdigits[pos1 + pos2] +
            ((product << 1) & BIGDIGIT_MASK);
        result->bigdigits[pos1 + pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
        carry += product >> (8 * sizeof(bigdigittype) - 1);
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
    return(result);
  } /* uBigSqare */



/**
 *  Returns the product of big1 by big2 for nonnegative big integers.
 *  The result is written into big_help (which is a scratch variable
 *  and is assumed to contain enough memory). Before returning the
 *  result the variable big1 is assigned to big_help. This way it is
 *  possible to write number = uBigMult(number, base, &big_help).
 *  Note that the old number is in the scratch variable big_help
 *  afterwards.
 */
#ifdef ANSI_C

static biginttype uBigMult (biginttype big1, biginttype big2, biginttype *big_help)
#else

static biginttype uBigMult (big1, big2, big_help)
biginttype big1;
biginttype big2;
biginttype *big_help;
#endif

  {
    memsizetype pos1;
    memsizetype pos2;
    doublebigdigittype carry = 0;
    bigdigittype digit;
    biginttype result;

  /* uBigMult */
    result = *big_help;
    pos2 = 0;
    do {
      carry += (doublebigdigittype) big1->bigdigits[0] * big2->bigdigits[pos2];
      result->bigdigits[pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
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
        carry >>= 8 * sizeof(bigdigittype);
        pos2++;
      } while (pos2 < big2->size);
      result->bigdigits[pos1 + big2->size] = (bigdigittype) (carry & BIGDIGIT_MASK);
    } /* for */
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
    return(result);
  } /* uBigMult */



#ifdef ANSI_C

static int uBigIsNot0 (biginttype big)
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
        return(TRUE);
      } /* if */
      pos++;
    } while (pos < big->size);
    return(FALSE);
  } /* uBigIsNot0 */



/**
 *  Returns the absolute value of a signed big integer.
 */
#ifdef ANSI_C

biginttype bigAbs (biginttype big1)
#else

biginttype bigAbs (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;
    biginttype result;

  /* bigAbs */
    if (!ALLOC_BIG(result, big1->size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(big1->size);
      result->size = big1->size;
      if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
        pos = 0;
        do {
          carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
          pos++;
        } while (pos < big1->size);
        if (IS_NEGATIVE(result->bigdigits[pos - 1])) {
          result->size++;
          if (!RESIZE_BIG(result, big1->size, result->size)) {
            raise_error(MEMORY_ERROR);
            return(NULL);
          } else {
            COUNT3_BIG(big1->size, result->size);
            result->bigdigits[big1->size] = 0;
          } /* if */
        } /* if */
      } else {
        memcpy(result->bigdigits, big1->bigdigits,
            (SIZE_TYPE) big1->size * sizeof(bigdigittype));
      } /* if */
    } /* if */
    return(result);
  } /* bigAbs */



/**
 *  Returns the sum of two signed big integers. The two values
 *  are sorted by size. This way there is a loop up to the
 *  shorter size and a second loop up to the longer size.
 */
#ifdef ANSI_C

biginttype bigAdd (biginttype big1, biginttype big2)
#else

biginttype bigAdd (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    biginttype help_big;
    memsizetype pos;
    doublebigdigittype carry = 0;
    doublebigdigittype big2_sign;
    biginttype result;

  /* bigAdd */
    if (big2->size > big1->size) {
      help_big = big1;
      big1 = big2;
      big2 = help_big;
    } /* if */
    if (!ALLOC_BIG(result, big1->size + 1)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(big1->size + 1);
      pos = 0;
      do {
        carry += (doublebigdigittype) big1->bigdigits[pos] + big2->bigdigits[pos];
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
        pos++;
      } while (pos < big2->size);
      big2_sign = IS_NEGATIVE(big2->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
      for (; pos < big1->size; pos++) {
        carry += big1->bigdigits[pos] + big2_sign;
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
      } /* for */
      if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
        big2_sign--;
      } /* if */
      result->bigdigits[pos] = (bigdigittype) (carry + big2_sign);
      result->size = pos + 1;
      normalize(result);
      return(result);
    } /* if */
  } /* bigAdd */



#ifdef ANSI_C

stritype bigCLit (biginttype big1)
#else

stritype bigCLit (big1)
biginttype big1;
#endif

  {
    union {
      struct {
        bigdigittype digit1;
        bigdigittype digit2;
      } digits;
      memsizetype size;
    } size_union;
    memsizetype pos;
    char buffer[21];
    stritype result;

  /* bigCLit */
    if (!ALLOC_STRI(result, big1->size * 7 + 15)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      size_union.size = big1->size;
      result->size = big1->size * 7 + 15;
      result->mem[ 0] = '{';
      sprintf(buffer, "%04hX", size_union.digits.digit1);
      result->mem[ 1] = '0';
      result->mem[ 2] = 'x';
      result->mem[ 3] = buffer[0];
      result->mem[ 4] = buffer[1];
      result->mem[ 5] = buffer[2];
      result->mem[ 6] = buffer[3];
      result->mem[ 7] = ',';
      sprintf(buffer, "%04hX", size_union.digits.digit2);
      result->mem[ 8] = '0';
      result->mem[ 9] = 'x';
      result->mem[10] = buffer[0];
      result->mem[11] = buffer[1];
      result->mem[12] = buffer[2];
      result->mem[13] = buffer[3];
      result->mem[14] = ',';
      pos = 0;
      do {
        sprintf(buffer, "%04hX", big1->bigdigits[pos]);
        result->mem[15 + 7 * pos] = '0';
        result->mem[16 + 7 * pos] = 'x';
        result->mem[17 + 7 * pos] = buffer[0];
        result->mem[18 + 7 * pos] = buffer[1];
        result->mem[19 + 7 * pos] = buffer[2];
        result->mem[20 + 7 * pos] = buffer[3];
        result->mem[21 + 7 * pos] = ',';
        pos++;
      } while (pos < big1->size);
      result->mem[14 + 7 * pos] = '}';
      return(result);
    } /* if */
  } /* bigCLit */



#ifdef ANSI_C

inttype bigCmp (biginttype big1, biginttype big2)
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
      return(big1_negative ? -1 : 1);
    } else if (big1->size != big2->size) {
      return((big1->size < big2->size) != (big1_negative != 0) ? -1 : 1);
    } else {
      pos = big1->size;
      while (pos > 0) {
        pos--;
        if (big1->bigdigits[pos] != big2->bigdigits[pos]) {
          return(big1->bigdigits[pos] < big2->bigdigits[pos] ? -1 : 1);
        } /* if */
      } /* while */
      return(0);
    } /* if */
  } /* bigCmp */



#ifdef ANSI_C

void bigCpy (biginttype *big_to, biginttype big_from)
#else

objecttype bigCpy (big_to, big_from)
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
      if (!ALLOC_BIG(big_dest, new_size)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        COUNT_BIG(new_size);
        FREE_BIG(*big_to, (*big_to)->size);
        big_dest->size = new_size;
        *big_to = big_dest;
      } /* if */
    } /* if */
    memcpy(big_dest->bigdigits, big_from->bigdigits,
        (SIZE_TYPE) new_size * sizeof(bigdigittype));
  } /* bigCpy */



#ifdef ANSI_C

biginttype bigCreate (biginttype big_from)
#else

biginttype bigCreate (big_from)
biginttype big_from;
#endif

  {
    memsizetype new_size;
    biginttype result;

  /* bigCreate */
    new_size = big_from->size;
    if (!ALLOC_BIG(result, new_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      COUNT_BIG(new_size);
      result->size = new_size;
      memcpy(result->bigdigits, big_from->bigdigits,
          (SIZE_TYPE) new_size * sizeof(bigdigittype));
    } /* if */
    return(result);
  } /* bigCreate */



#ifdef ANSI_C

void bigDecr (biginttype *big_variable)
#else

void bigDecr (big_variable)
biginttype *big_variable;
#endif

  {
    biginttype big1;
    memsizetype pos;
    doublebigdigittype carry = 0;
    bigdigittype negative;

  /* bigDecr */
    big1 = *big_variable;
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    pos = 0;
    do {
      carry += (doublebigdigittype) big1->bigdigits[pos] + BIGDIGIT_MASK;
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
      pos++;
    } while (carry == 0 && pos < big1->size);
    pos = big1->size;
    if (!IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      if (negative) {
        if (!RESIZE_BIG(big1, pos, pos + 1)) {
          raise_error(MEMORY_ERROR);
        } else {
          COUNT3_BIG(pos, pos + 1);
          big1->size++;
          big1->bigdigits[pos] = BIGDIGIT_MASK;
          *big_variable = big1;
        } /* if */
      } else if (big1->bigdigits[pos - 1] == 0 &&
          pos >= 2 && !IS_NEGATIVE(big1->bigdigits[pos - 2])) {
        if (!RESIZE_BIG(big1, pos, pos - 1)) {
          raise_error(MEMORY_ERROR);
        } else {
          COUNT3_BIG(pos, pos - 1);
          big1->size--;
          *big_variable = big1;
        } /* if */
      } /* if */
    } /* if */
  } /* bigDecr */



#ifdef ANSI_C

void bigDestr (biginttype old_bigint)
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
 *  Computes an integer division of big1 by big2 for signed big
 *  integers. The memory for the result is requested and the
 *  normalized result is returned. When big2 has just one digit
 *  or when big1 has less digits than big2 the bigDiv1() or
 *  bigDivSizeLess() functions are called. In the general case
 *  the absolute values of big1 and big2 are taken. Then big1 is
 *  extended by one leading zero digit. After that big1 and big2
 *  are shifted to the left such that the most significant bit
 *  of big2 is set. This fulfills the preconditions for calling
 *  uBigDiv() which does the main work of the division.
 */
#ifdef ANSI_C

biginttype bigDiv (biginttype big1, biginttype big2)
#else

biginttype bigDiv (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    booltype negative = FALSE;
    biginttype big1_help;
    biginttype big2_help;
    unsigned int shift;
    biginttype result;

  /* bigDiv */
    if (big2->size == 1) {
      result = bigDiv1(big1, big2->bigdigits[0]);
      return(result);
    } else if (big1->size < big2->size) {
      result = bigDivSizeLess(big1, big2);
      return(result);
    } else {
      if (!ALLOC_BIG(big1_help, big1->size + 2)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big1->size + 2);
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          negative = TRUE;
          positive_copy_of_negative_big(big1_help, big1);
        } else {
          big1_help->size = big1->size;
          memcpy(big1_help->bigdigits, big1->bigdigits,
              (SIZE_TYPE) big1->size * sizeof(bigdigittype));
        } /* if */
        big1_help->bigdigits[big1_help->size] = 0;
        big1_help->size++;
      } /* if */
      if (!ALLOC_BIG(big2_help, big2->size + 1)) {
        FREE_BIG(big1_help,  big1->size + 2);
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big2->size + 1);
        if (IS_NEGATIVE(big2->bigdigits[big2->size - 1])) {
          negative = !negative;
          positive_copy_of_negative_big(big2_help, big2);
        } else {
          big2_help->size = big2->size;
          memcpy(big2_help->bigdigits, big2->bigdigits,
              (SIZE_TYPE) big2->size * sizeof(bigdigittype));
        } /* if */
      } /* if */
      if (!ALLOC_BIG(result, big1_help->size - big2_help->size + 1)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT_BIG(big1_help->size - big2_help->size + 1);
        result->size = big1_help->size - big2_help->size + 1;
        result->bigdigits[result->size - 1] = 0;
        shift = 8 * sizeof(bigdigittype) -
            most_significant_bit(big2_help->bigdigits[big2_help->size - 1]) - 1;
        uBigLShift(big1_help, shift);
        uBigLShift(big2_help, shift);
        uBigDiv(big1_help, big2_help, result);
        if (negative) {
          negate_positive_big(result);
        } /* if */
        normalize(result);
      } /* if */
      FREE_BIG(big1_help, big1->size + 2);
      FREE_BIG(big2_help, big2->size + 1);
      return(result);
    } /* if */
  } /* bigDiv */



#ifdef ANSI_C

booltype bigEq (biginttype big1, biginttype big2)
#else

booltype bigEq (big1, big2)
biginttype big1;
biginttype big2;
#endif

  { /* bigEq */
    if (big1->size == big2->size &&
      memcmp(big1->bigdigits, big2->bigdigits,
          (SIZE_TYPE) big1->size * sizeof(bigdigittype)) == 0) {
      return(TRUE);
    } else {
      return(FALSE);
    } /* if */
  } /* bigEq */



#ifdef ANSI_C

void bigGrow (biginttype *big_variable, biginttype big2)
#else

void bigGrow (big_variable, big2)
biginttype *big_variable;
biginttype big2;
#endif

  {
    biginttype big1;
    memsizetype pos;
    doublebigdigittype carry = 0;
    doublebigdigittype big1_sign;
    doublebigdigittype big2_sign;

  /* bigGrow */
    big1 = *big_variable;
    if (big1->size >= big2->size) {
      big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
      pos = 0;
      do {
        carry += (doublebigdigittype) big1->bigdigits[pos] + big2->bigdigits[pos];
        big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
        pos++;
      } while (pos < big2->size);
      big2_sign = IS_NEGATIVE(big2->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
      for (; pos < big1->size; pos++) {
        carry += big1->bigdigits[pos] + big2_sign;
        big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
      } /* for */
      carry += big1_sign + big2_sign;
      carry &= BIGDIGIT_MASK;
      if ((carry != 0 || IS_NEGATIVE(big1->bigdigits[pos - 1])) &&
          (carry != BIGDIGIT_MASK || !IS_NEGATIVE(big1->bigdigits[pos - 1]))) {
        if (!RESIZE_BIG(big1, big1->size, big1->size + 1)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT3_BIG(big1->size, big1->size + 1);
          big1->size++;
          big1->bigdigits[pos] = (bigdigittype) carry;
          *big_variable = big1;
        } /* if */
      } else {
        normalize(big1);
      } /* if */
    } else {
      if (!RESIZE_BIG(big1, big1->size, big2->size + 1)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        COUNT3_BIG(big1->size, big2->size + 1);
        big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
        pos = 0;
        do {
          carry += (doublebigdigittype) big1->bigdigits[pos] + big2->bigdigits[pos];
          big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
          pos++;
        } while (pos < big1->size);
        big2_sign = IS_NEGATIVE(big2->bigdigits[big2->size - 1]) ? BIGDIGIT_MASK : 0;
        for (; pos < big2->size; pos++) {
          carry += big1_sign + big2->bigdigits[pos];
          big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
        } /* for */
        carry += big1_sign + big2_sign;
        big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        big1->size = pos + 1;
        normalize(big1);
        *big_variable = big1;
      } /* if */
    } /* if */
  } /* bigGrow */



#ifdef ANSI_C

inttype bigHashCode (biginttype big1)
#else

inttype bigHashCode (big1)
biginttype big1;
#endif

  {
    inttype result;

  /* bigHashCode */
    result = big1->bigdigits[0] << 5 ^ big1->size << 3 ^ big1->bigdigits[big1->size - 1];
    return(result);
  } /* bigHashCode */



#ifdef ANSI_C

biginttype bigIConv (inttype number)
#else

biginttype bigIConv (number)
inttype number;
#endif

  {
    memsizetype pos;
    memsizetype result_size;
    biginttype result;

  /* bigIConv */
    result_size = sizeof(inttype) / sizeof(bigdigittype);
    if (!ALLOC_BIG(result, result_size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(result_size);
      result->size = result_size;
      for (pos = 0; pos < result_size; pos++) {
        result->bigdigits[pos] = (bigdigittype) (number & BIGDIGIT_MASK);
        number >>= 8 * sizeof(bigdigittype);
      } /* for */
      normalize(result);
      return(result);
    } /* if */
  } /* bigIConv */



#ifdef ANSI_C

void bigIncr (biginttype *big_variable)
#else

void bigIncr (big_variable)
biginttype *big_variable;
#endif

  {
    biginttype big1;
    memsizetype pos;
    doublebigdigittype carry = 1;
    bigdigittype negative;

  /* bigIncr */
    big1 = *big_variable;
    negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
    pos = 0;
    do {
      carry += big1->bigdigits[pos];
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
      pos++;
    } while (carry != 0 && pos < big1->size);
    pos = big1->size;
    if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      if (!negative) {
        if (!RESIZE_BIG(big1, pos, pos + 1)) {
          raise_error(MEMORY_ERROR);
        } else {
          COUNT3_BIG(pos, pos + 1);
          big1->size++;
          big1->bigdigits[pos] = 0;
          *big_variable = big1;
        } /* if */
      } else if (big1->bigdigits[pos - 1] == BIGDIGIT_MASK &&
          pos >= 2 && IS_NEGATIVE(big1->bigdigits[pos - 2])) {
        if (!RESIZE_BIG(big1, pos, pos - 1)) {
          raise_error(MEMORY_ERROR);
        } else {
          COUNT3_BIG(pos, pos - 1);
          big1->size--;
          *big_variable = big1;
        } /* if */
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

biginttype bigIPow (biginttype base, inttype exponent)
#else

biginttype bigIPow (base, exponent)
biginttype base;
inttype exponent;
#endif

  {
    booltype negative = FALSE;
    memsizetype help_size;
    biginttype base_help;
    biginttype big_help;
    biginttype result;

  /* bigIPow */
    if (exponent < 0) {
      raise_error(NUMERIC_ERROR);
      return(NULL);
    } else {
      help_size = base->size * (exponent + 1);
      if (!ALLOC_BIG(base_help, help_size)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(help_size);
        if (!ALLOC_BIG(big_help, help_size)) {
          FREE_BIG(base_help,  help_size);
          raise_error(MEMORY_ERROR);
          return(NULL);
        } else {
          COUNT_BIG(help_size);
          if (!ALLOC_BIG(result, help_size)) {
            FREE_BIG(base_help,  help_size);
            FREE_BIG(big_help,  help_size);
            raise_error(MEMORY_ERROR);
            return(NULL);
          } else {
            COUNT_BIG(help_size);
            if (IS_NEGATIVE(base->bigdigits[base->size - 1])) {
              negative = TRUE;
              positive_copy_of_negative_big(base_help, base);
            } else {
              base_help->size = base->size;
              memcpy(base_help->bigdigits, base->bigdigits,
                  (SIZE_TYPE) base->size * sizeof(bigdigittype));
            } /* if */
            base = base_help;
            if (exponent & 1) {
              result->size = base->size;
              memcpy(result->bigdigits, base->bigdigits,
                  (SIZE_TYPE) base->size * sizeof(bigdigittype));
            } else {
              negative = FALSE;
              result->size = 1;
              result->bigdigits[0] = 1;
            } /* if */
            exponent >>= 1;
            while (exponent != 0) {
              base = uBigSqare(base, &big_help);
              if (exponent & 1) {
                result = uBigMult(result, base, &big_help);
              } /* if */
              exponent >>= 1;
            } /* while */
            memset(&result->bigdigits[result->size], 0,
                (SIZE_TYPE) (help_size - result->size) * sizeof(bigdigittype));
            result->size = help_size;
            if (negative) {
              negate_positive_big(result);
            } /* if */
            normalize(result);
            FREE_BIG(base, help_size);
            FREE_BIG(big_help, help_size);
            return(result);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* bigIPow */



#ifdef HAS_LONGTYPE_64
#ifdef ANSI_C

biginttype bigLConv (longtype number)
#else

biginttype bigLConv (number)
longtype number;
#endif

  {
    memsizetype pos;
    memsizetype result_size;
    biginttype result;

  /* bigLConv */
    result_size = sizeof(longtype) / sizeof(bigdigittype);
    if (!ALLOC_BIG(result, result_size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(result_size);
      result->size = result_size;
      for (pos = 0; pos < result_size; pos++) {
        result->bigdigits[pos] = (bigdigittype) (number & BIGDIGIT_MASK);
        number >>= 8 * sizeof(bigdigittype);
      } /* for */
      normalize(result);
      return(result);
    } /* if */
  } /* bigLConv */
#endif



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

inttype bigLd (biginttype big1)
#else

inttype bigLd (big1)
biginttype big1;
#endif

  {
    inttype result;

  /* bigLd */
    if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
      raise_error(NUMERIC_ERROR);
      return(0);
    } else {
      result = (big1->size - 1) * 8 * sizeof(bigdigittype);
      result += most_significant_bit(big1->bigdigits[big1->size - 1]);
      return(result);
    } /* if */
  } /* bigLd */
#endif



#ifdef HAS_LONGTYPE_64
#ifdef ANSI_C

longtype bigLOrd (biginttype big1)
#else

longtype bigLOrd (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    longtype result;

  /* bigLOrd */
    if (big1->size > sizeof(longtype) / sizeof(bigdigittype)) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      pos = big1->size - 1;
      result = big1->bigdigits[pos];
      while (pos > 0) {
        pos--;
        result <<= 8 * sizeof(bigdigittype);
        result |= (inttype) big1->bigdigits[pos];
      } /* while */
      return(result);
    } /* if */
  } /* bigLOrd */
#endif



/**
 *  Computes an integer modulo division of big1 by big2 for signed
 *  big integers. The memory for the result is requested and the
 *  normalized result is returned. When big2 has just one digit
 *  or when big1 has less digits than big2 the bigDiv1() or
 *  bigDivSizeLess() functions are called. In the general case
 *  the absolute values of big1 and big2 are taken. Then big1 is
 *  extended by one leading zero digit. After that big1 and big2
 *  are shifted to the left such that the most significant bit
 *  of big2 is set. This fulfills the preconditions for calling
 *  uBigDiv() which does the main work of the division.
 */
#ifdef ANSI_C

biginttype bigMDiv (biginttype big1, biginttype big2)
#else

biginttype bigMDiv (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    booltype negative = FALSE;
    biginttype big1_help;
    biginttype big2_help;
    unsigned int shift;
    biginttype result;

  /* bigMDiv */
    if (big2->size == 1) {
      result = bigMDiv1(big1, big2->bigdigits[0]);
      return(result);
    } else if (big1->size < big2->size) {
      result = bigMDivSizeLess(big1, big2);
      return(result);
    } else {
      if (!ALLOC_BIG(big1_help, big1->size + 2)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big1->size + 2);
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          negative = TRUE;
          positive_copy_of_negative_big(big1_help, big1);
        } else {
          big1_help->size = big1->size;
          memcpy(big1_help->bigdigits, big1->bigdigits,
              (SIZE_TYPE) big1->size * sizeof(bigdigittype));
        } /* if */
        big1_help->bigdigits[big1_help->size] = 0;
        big1_help->size++;
      } /* if */
      if (!ALLOC_BIG(big2_help, big2->size + 1)) {
        FREE_BIG(big1_help,  big1->size + 2);
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big2->size + 1);
        if (IS_NEGATIVE(big2->bigdigits[big2->size - 1])) {
          negative = !negative;
          positive_copy_of_negative_big(big2_help, big2);
        } else {
          big2_help->size = big2->size;
          memcpy(big2_help->bigdigits, big2->bigdigits,
              (SIZE_TYPE) big2->size * sizeof(bigdigittype));
        } /* if */
      } /* if */
      if (!ALLOC_BIG(result, big1_help->size - big2_help->size + 1)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT_BIG(big1_help->size - big2_help->size + 1);
        result->size = big1_help->size - big2_help->size + 1;
        result->bigdigits[result->size - 1] = 0;
        shift = 8 * sizeof(bigdigittype) -
            most_significant_bit(big2_help->bigdigits[big2_help->size - 1]) - 1;
        uBigLShift(big1_help, shift);
        uBigLShift(big2_help, shift);
        uBigDiv(big1_help, big2_help, result);
        if (negative) {
          if (uBigIsNot0(big1_help)) {
            uBigIncr(result);
          } /* if */
          negate_positive_big(result);
        } /* if */
        normalize(result);
      } /* if */
      FREE_BIG(big1_help, big1->size + 2);
      FREE_BIG(big2_help, big2->size + 1);
      return(result);
    } /* if */
  } /* bigMDiv */



#ifdef ANSI_C

biginttype bigMinus (biginttype big1)
#else

biginttype bigMinus (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;
    biginttype result;

  /* bigMinus */
    if (!ALLOC_BIG(result, big1->size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(big1->size);
      result->size = big1->size;
      pos = 0;
      do {
        carry += ~big1->bigdigits[pos] & BIGDIGIT_MASK;
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
        pos++;
      } while (pos < big1->size);
      if (IS_NEGATIVE(result->bigdigits[pos - 1])) {
        if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
          if (!RESIZE_BIG(result, pos, pos + 1)) {
            raise_error(MEMORY_ERROR);
            return(NULL);
          } else {
            COUNT3_BIG(pos, pos + 1);
            result->size++;
            result->bigdigits[pos] = 0;
          } /* if */
        } else if (result->bigdigits[pos - 1] == BIGDIGIT_MASK &&
            pos >= 2 && IS_NEGATIVE(result->bigdigits[pos - 2])) {
          if (!RESIZE_BIG(result, pos, pos - 1)) {
            raise_error(MEMORY_ERROR);
            return(NULL);
          } else {
            COUNT3_BIG(pos, pos - 1);
            result->size--;
          } /* if */
        } /* if */
      } /* if */
      return(result);
    } /* if */
  } /* bigMinus */



/**
 *  Computes the modulo of an integer division of big1 by big2
 *  for signed big integers. The memory for the result is requested
 *  and the normalized result is returned. When big2 has just one
 *  digit or when big1 has less digits than big2 the bigMod1() or
 *  bigModSizeLess() functions are called. In the general case
 *  the absolute values of big1 and big2 are taken. Then big1 is
 *  extended by one leading zero digit. After that big1 and big2
 *  are shifted to the left such that the most significant bit
 *  of big2 is set. This fulfills the preconditions for calling
 *  uBigRem() which does the main work of the division. Afterwards
 *  the result must be shifted to the right to get the remainder.
 *  If big1 and big2 have the same sign the modulo has the same
 *  value as the remainder. When the remainder is zero the modulo
 *  is also zero. If the signs of big1 and big2 are different the
 *  modulo is computed from the remainder by adding big1.
 */
#ifdef ANSI_C

biginttype bigMod (biginttype big1, biginttype big2)
#else

biginttype bigMod (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    booltype negative1 = FALSE;
    booltype negative2 = FALSE;
    biginttype big2_help;
    unsigned int shift;
    biginttype result;

  /* bigMod */
    if (big2->size == 1) {
      result = bigMod1(big1, big2->bigdigits[0]);
      return(result);
    } else if (big1->size < big2->size) {
      result = bigModSizeLess(big1, big2);
      return(result);
    } else {
      if (!ALLOC_BIG(result, big1->size + 2)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big1->size + 2);
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          negative1 = TRUE;
          positive_copy_of_negative_big(result, big1);
        } else {
          result->size = big1->size;
          memcpy(result->bigdigits, big1->bigdigits,
              (SIZE_TYPE) big1->size * sizeof(bigdigittype));
        } /* if */
        result->bigdigits[result->size] = 0;
        result->size++;
      } /* if */
      if (!ALLOC_BIG(big2_help, big2->size + 1)) {
        FREE_BIG(result,  big1->size + 2);
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big2->size + 1);
        if (IS_NEGATIVE(big2->bigdigits[big2->size - 1])) {
          negative2 = TRUE;
          positive_copy_of_negative_big(big2_help, big2);
        } else {
          big2_help->size = big2->size;
          memcpy(big2_help->bigdigits, big2->bigdigits,
              (SIZE_TYPE) big2->size * sizeof(bigdigittype));
        } /* if */
      } /* if */
      shift = 8 * sizeof(bigdigittype) -
          most_significant_bit(big2_help->bigdigits[big2_help->size - 1]) - 1;
      uBigLShift(result, shift);
      uBigLShift(big2_help, shift);
      uBigRem(result, big2_help);
      uBigRShift(result, shift);
      result->bigdigits[big1->size + 1] = 0;
      result->size = big1->size + 2;
      if (negative1) {
        if (negative2) {
          negate_positive_big(result);
        } else {
          if (uBigIsNot0(result)) {
            negate_positive_big(result);
            bigAddTo(result, big2);
          } /* if */
        } /* if */
      } else {
        if (negative2) {
          if (uBigIsNot0(result)) {
            bigAddTo(result, big2);
          } /* if */
        } /* if */
      } /* if */
      normalize(result);
      FREE_BIG(big2_help, big2->size + 1);
      return(result);
    } /* if */
  } /* bigMod */



#ifdef ANSI_C

biginttype bigMult (biginttype big1, biginttype big2)
#else

biginttype bigMult (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    booltype negative = FALSE;
    biginttype big1_help = NULL;
    biginttype big2_help = NULL;
    memsizetype pos1;
    memsizetype pos2;
    doublebigdigittype carry = 0;
    biginttype result;

  /* bigMult */
    if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
      negative = TRUE;
      big1_help = alloc_positive_copy_of_negative_big(big1);
      big1 = big1_help;
      if (big1_help == NULL) {
        return(NULL);
      } /* if */
    } /* if */
    if (IS_NEGATIVE(big2->bigdigits[big2->size - 1])) {
      negative = !negative;
      big2_help = alloc_positive_copy_of_negative_big(big2);
      big2 = big2_help;
      if (big2_help == NULL) {
        if (big1_help != NULL) {
          FREE_BIG(big1_help, big1_help->size);
        } /* if */
        return(NULL);
      } /* if */
    } /* if */
    if (!ALLOC_BIG(result, big1->size + big2->size)) {
      raise_error(MEMORY_ERROR);
    } else {
      COUNT_BIG(big1->size + big2->size);
      pos2 = 0;
      do {
        carry += (doublebigdigittype) big1->bigdigits[0] * big2->bigdigits[pos2];
        result->bigdigits[pos2] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
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
          carry >>= 8 * sizeof(bigdigittype);
          pos2++;
        } while (pos2 < big2->size);
        result->bigdigits[pos1 + big2->size] = (bigdigittype) (carry & BIGDIGIT_MASK);
      } /* for */
      result->size = big1->size + big2->size;
      if (negative) {
        negate_positive_big(result);
      } /* if */
      normalize(result);
    } /* if */
    if (big1_help != NULL) {
      FREE_BIG(big1_help, big1_help->size);
    } /* if */
    if (big2_help != NULL) {
      FREE_BIG(big2_help, big2_help->size);
    } /* if */
    return(result);
  } /* bigMult */



#ifdef ANSI_C

booltype bigNe (biginttype big1, biginttype big2)
#else

booltype bigNe (big1, big2)
biginttype big1;
biginttype big2;
#endif

  { /* bigNe */
    if (big1->size != big2->size ||
      memcmp(big1->bigdigits, big2->bigdigits,
          (SIZE_TYPE) big1->size * sizeof(bigdigittype)) != 0) {
      return(TRUE);
    } else {
      return(FALSE);
    } /* if */
  } /* bigNe */



#ifdef ANSI_C

inttype bigOrd (biginttype big1)
#else

inttype bigOrd (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    inttype result;

  /* bigOrd */
    if (big1->size > sizeof(inttype) / sizeof(bigdigittype)) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      pos = big1->size - 1;
      result = big1->bigdigits[pos];
      while (pos > 0) {
        pos--;
        result <<= 8 * sizeof(bigdigittype);
        result |= (inttype) big1->bigdigits[pos];
      } /* while */
      return(result);
    } /* if */
  } /* bigOrd */



#ifdef ANSI_C

biginttype bigParse (stritype stri)
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
    if (!ALLOC_BIG(result, result_size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(result_size);
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
      if (okay) {
        memset(&result->bigdigits[result->size], 0,
            (SIZE_TYPE) (result_size - result->size) * sizeof(bigdigittype));
        result->size = result_size;
        if (negative) {
          negate_positive_big(result);
        } /* if */
        normalize(result);
        return(result);
      } else {
        FREE_BIG(result, result_size);
        raise_error(RANGE_ERROR);
        return(NULL);
      } /* if */
    } /* if */
  } /* bigParse */



#ifdef ANSI_C

biginttype bigPred (biginttype big1)
#else

biginttype bigPred (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;
    biginttype result;

  /* bigPred */
    if (!ALLOC_BIG(result, big1->size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(big1->size);
      result->size = big1->size;
      pos = 0;
      do {
        carry += (doublebigdigittype) big1->bigdigits[pos] + BIGDIGIT_MASK;
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
        pos++;
      } while (pos < big1->size);
      if (!IS_NEGATIVE(result->bigdigits[pos - 1])) {
        if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
          if (!RESIZE_BIG(result, pos, pos + 1)) {
            raise_error(MEMORY_ERROR);
            return(NULL);
          } else {
            COUNT3_BIG(pos, pos + 1);
            result->size++;
            result->bigdigits[pos] = BIGDIGIT_MASK;
          } /* if */
        } else if (result->bigdigits[pos - 1] == 0 &&
            pos >= 2 && !IS_NEGATIVE(result->bigdigits[pos - 2])) {
          if (!RESIZE_BIG(result, pos, pos - 1)) {
            raise_error(MEMORY_ERROR);
            return(NULL);
          } else {
            COUNT3_BIG(pos, pos - 1);
            result->size--;
          } /* if */
        } /* if */
      } /* if */
      return(result);
    } /* if */
  } /* bigPred */



/**
 *  Computes a random number between lower_limit and upper_limit
 *  for signed big integers. The memory for the result is requested
 *  and the normalized result is returned. The random numbers are
 *  uniform distributed over the whole range.
 */
#ifdef ANSI_C

biginttype bigRand (biginttype lower_limit, biginttype upper_limit)
#else

biginttype bigRand (lower_limit, upper_limit)
biginttype lower_limit;
biginttype upper_limit;
#endif

  {
    biginttype scale_limit;
    bigdigittype mask;
    memsizetype pos;
    uinttype random_number = 0;
    memsizetype result_size;
    biginttype result;

  /* bigRand */
    if (bigCmp(lower_limit, upper_limit) > 0) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      scale_limit = bigSbtr(upper_limit, lower_limit);
      if (lower_limit->size > scale_limit->size) {
        result_size = lower_limit->size + 1;
      } else {
        result_size = scale_limit->size + 1;
      } /* if */
      if (!ALLOC_BIG(result, result_size)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(result_size);
        memset(&result->bigdigits[scale_limit->size], 0,
            (SIZE_TYPE) (result_size - scale_limit->size) * sizeof(bigdigittype));
        result->size = scale_limit->size;
        mask = ((bigdigittype) BIGDIGIT_MASK) >>
            (8 * sizeof(bigdigittype) -
            most_significant_bit(scale_limit->bigdigits[scale_limit->size - 1]) - 1);
        do {
          pos = 0;
          do {
            if (random_number == 0) {
              random_number = rand_32();
            } /* if */
            result->bigdigits[pos] = (bigdigittype) (random_number & BIGDIGIT_MASK);
            random_number >>= 8 * sizeof(bigdigittype);
            pos++;
          } while (pos < scale_limit->size);
          result->bigdigits[pos - 1] &= mask;
        } while (bigCmp(result, scale_limit) > 0);
        result->size = result_size;
        bigAddTo(result, lower_limit);
        normalize(result);
        FREE_BIG(scale_limit, scale_limit->size);
        return(result);
      } /* if */
    } /* if */
  } /* bigRand */



/**
 *  Computes the remainder of an integer division of big1 by big2
 *  for signed big integers. The memory for the result is requested
 *  and the normalized result is returned. When big2 has just one
 *  digit or when big1 has less digits than big2 the bigRem1() or
 *  bigRemSizeLess() functions are called. In the general case
 *  the absolute values of big1 and big2 are taken. Then big1 is
 *  extended by one leading zero digit. After that big1 and big2
 *  are shifted to the left such that the most significant bit
 *  of big2 is set. This fulfills the preconditions for calling
 *  uBigRem() which does the main work of the division. Afterwards
 *  the result must be shifted to the right to get the remainder.
 */
#ifdef ANSI_C

biginttype bigRem (biginttype big1, biginttype big2)
#else

biginttype bigRem (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    booltype negative = FALSE;
    biginttype big2_help;
    unsigned int shift;
    biginttype result;

  /* bigRem */
    if (big2->size == 1) {
      result = bigRem1(big1, big2->bigdigits[0]);
      return(result);
    } else if (big1->size < big2->size) {
      result = bigRemSizeLess(big1, big2);
      return(result);
    } else {
      if (!ALLOC_BIG(result, big1->size + 2)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big1->size + 2);
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          negative = TRUE;
          positive_copy_of_negative_big(result, big1);
        } else {
          result->size = big1->size;
          memcpy(result->bigdigits, big1->bigdigits,
              (SIZE_TYPE) big1->size * sizeof(bigdigittype));
        } /* if */
        result->bigdigits[result->size] = 0;
        result->size++;
      } /* if */
      if (!ALLOC_BIG(big2_help, big2->size + 1)) {
        FREE_BIG(result,  big1->size + 2);
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big2->size + 1);
        if (IS_NEGATIVE(big2->bigdigits[big2->size - 1])) {
          positive_copy_of_negative_big(big2_help, big2);
        } else {
          big2_help->size = big2->size;
          memcpy(big2_help->bigdigits, big2->bigdigits,
              (SIZE_TYPE) big2->size * sizeof(bigdigittype));
        } /* if */
      } /* if */
      shift = 8 * sizeof(bigdigittype) -
          most_significant_bit(big2_help->bigdigits[big2_help->size - 1]) - 1;
      uBigLShift(result, shift);
      uBigLShift(big2_help, shift);
      uBigRem(result, big2_help);
      uBigRShift(result, shift);
      result->bigdigits[big1->size + 1] = 0;
      result->size = big1->size + 2;
      if (negative) {
        negate_positive_big(result);
      } /* if */
      normalize(result);
      FREE_BIG(big2_help, big2->size + 1);
      return(result);
    } /* if */
  } /* bigRem */



#ifdef ANSI_C

biginttype bigSbtr (biginttype big1, biginttype big2)
#else

biginttype bigSbtr (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;
    doublebigdigittype big1_sign;
    doublebigdigittype big2_sign;
    biginttype result;

  /* bigSbtr */
    if (big1->size >= big2->size) {
      if (!ALLOC_BIG(result, big1->size + 1)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big1->size + 1);
        pos = 0;
        do {
          carry += (doublebigdigittype) big1->bigdigits[pos] +
              (~big2->bigdigits[pos] & BIGDIGIT_MASK);
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
          pos++;
        } while (pos < big2->size);
        big2_sign = IS_NEGATIVE(big2->bigdigits[pos - 1]) ? 0 : BIGDIGIT_MASK;
        for (; pos < big1->size; pos++) {
          carry += big1->bigdigits[pos] + big2_sign;
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
        } /* for */
        if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
          big2_sign--;
        } /* if */
        result->bigdigits[pos] = (bigdigittype) (carry + big2_sign);
        result->size = pos + 1;
        normalize(result);
        return(result);
      } /* if */
    } else {
      if (!ALLOC_BIG(result, big2->size + 1)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big2->size + 1);
        pos = 0;
        do {
          carry += (doublebigdigittype) big1->bigdigits[pos] +
              (~big2->bigdigits[pos] & BIGDIGIT_MASK);
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
          pos++;
        } while (pos < big1->size);
        big1_sign = IS_NEGATIVE(big1->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
        for (; pos < big2->size; pos++) {
          carry += big1_sign + (~big2->bigdigits[pos] & BIGDIGIT_MASK);
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
        } /* for */
        big2_sign = IS_NEGATIVE(big2->bigdigits[pos - 1]) ? 0 : BIGDIGIT_MASK;
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          big2_sign--;
        } /* if */
        result->bigdigits[pos] = (bigdigittype) (carry + big2_sign);
        result->size = pos + 1;
        normalize(result);
        return(result);
      } /* if */
    } /* if */
  } /* bigSbtr */



#ifdef ANSI_C

void bigShrink (biginttype *big_variable, biginttype big2)
#else

void bigShrink (big_variable, big2)
biginttype *big_variable;
biginttype big2;
#endif

  {
    biginttype big1;
    memsizetype pos;
    doublebigdigittype carry = 1;
    doublebigdigittype big1_sign;
    doublebigdigittype big2_sign;

  /* bigShrink */
    big1 = *big_variable;
    if (big1->size >= big2->size) {
      big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
      pos = 0;
      do {
        carry += (doublebigdigittype) big1->bigdigits[pos] +
            (~big2->bigdigits[pos] & BIGDIGIT_MASK);
        big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
        pos++;
      } while (pos < big2->size);
      big2_sign = IS_NEGATIVE(big2->bigdigits[pos - 1]) ? 0 : BIGDIGIT_MASK;
      for (; pos < big1->size; pos++) {
        carry += big1->bigdigits[pos] + big2_sign;
        big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
      } /* for */
      carry += big1_sign + big2_sign;
      carry &= BIGDIGIT_MASK;
      if ((carry != 0 || IS_NEGATIVE(big1->bigdigits[pos - 1])) &&
          (carry != BIGDIGIT_MASK || !IS_NEGATIVE(big1->bigdigits[pos - 1]))) {
        if (!RESIZE_BIG(big1, big1->size, big1->size + 1)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT3_BIG(big1->size, big1->size + 1);
          big1->size++;
          big1->bigdigits[pos] = (bigdigittype) carry;
          *big_variable = big1;
        } /* if */
      } else {
        normalize(big1);
      } /* if */
    } else {
      if (!RESIZE_BIG(big1, big1->size, big2->size + 1)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        COUNT3_BIG(big1->size, big2->size + 1);
        big1_sign = IS_NEGATIVE(big1->bigdigits[big1->size - 1]) ? BIGDIGIT_MASK : 0;
        pos = 0;
        do {
          carry += (doublebigdigittype) big1->bigdigits[pos] +
              (~big2->bigdigits[pos] & BIGDIGIT_MASK);
          big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
          pos++;
        } while (pos < big1->size);
        big2_sign = IS_NEGATIVE(big2->bigdigits[big2->size - 1]) ? 0 : BIGDIGIT_MASK;
        for (; pos < big2->size; pos++) {
          carry += big1_sign + (~big2->bigdigits[pos] & BIGDIGIT_MASK);
          big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
        } /* for */
        carry += big1_sign + big2_sign;
        big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        big1->size = pos + 1;
        normalize(big1);
        *big_variable = big1;
      } /* if */
    } /* if */
  } /* bigShrink */



#ifdef ANSI_C

stritype bigStr (biginttype big1)
#else

stritype bigStr (big1)
biginttype big1;
#endif

  {
    biginttype help_big;
    memsizetype pos;
    bigdigittype digit;
    int digit_pos;
    strelemtype digit_ch;
    memsizetype result_size;
    stritype result;

  /* bigStr */
    result_size = 256;
    if (!ALLOC_STRI(result, result_size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_STRI(result_size);
      if (!ALLOC_BIG(help_big, big1->size + 1)) {
        FREE_STRI(result, result_size);
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_BIG(big1->size + 1);
        pos = 0;
        if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
          positive_copy_of_negative_big(help_big, big1);
          result->mem[pos] = '-';
          pos++;
        } else {
          help_big->size = big1->size;
          memcpy(help_big->bigdigits, big1->bigdigits,
              (SIZE_TYPE) big1->size * sizeof(bigdigittype));
        } /* if */
        do {
          if (pos + DECIMAL_DIGITS_IN_BIGDIGIT > result_size) {
            if (!RESIZE_STRI(result, result_size, result_size + 256)) {
              FREE_STRI(result, result_size);
              FREE_BIG(help_big, big1->size + 1);
              raise_error(MEMORY_ERROR);
              return(NULL);
            } else {
              COUNT3_STRI(result_size, result_size + 256);
              result_size += 256;
            } /* if */
          } /* if */
          digit = uBigDivideByPowerOf10(help_big);
          if (help_big->bigdigits[help_big->size - 1] == 0) {
            help_big->size--;
          } /* if */
          if (help_big->size > 1 || help_big->bigdigits[0] != 0) {
            for (digit_pos = DECIMAL_DIGITS_IN_BIGDIGIT;
                digit_pos != 0; digit_pos--) {
              result->mem[pos] = '0' + digit % 10;
              digit /= 10;
              pos++;
            } /* for */
          } else {
            do {
              result->mem[pos] = '0' + digit % 10;
              digit /= 10;
              pos++;
            } while (digit != 0);
          } /* if */
        } while (help_big->size > 1 || help_big->bigdigits[0] != 0);
        FREE_BIG(help_big, big1->size + 1);
        result->size = pos;
        if (!RESIZE_STRI(result, result_size, pos)) {
          FREE_STRI(result, result_size);
          raise_error(MEMORY_ERROR);
          return(NULL);
        } else {
          COUNT3_STRI(result_size, pos);
          if (IS_NEGATIVE(big1->bigdigits[big1->size - 1])) {
            for (pos = 1; pos <= result->size >> 1; pos++) {
              digit_ch = result->mem[pos];
              result->mem[pos] = result->mem[result->size - pos];
              result->mem[result->size - pos] = digit_ch;
            } /* for */
          } else {
            for (pos = 0; pos < result->size >> 1; pos++) {
              digit_ch = result->mem[pos];
              result->mem[pos] = result->mem[result->size - pos - 1];
              result->mem[result->size - pos - 1] = digit_ch;
            } /* for */
          } /* if */
          return(result);
        } /* if */
      } /* if */
    } /* if */
  } /* bigStr */



#ifdef ANSI_C

biginttype bigSucc (biginttype big1)
#else

biginttype bigSucc (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;
    biginttype result;

  /* bigSucc */
    if (!ALLOC_BIG(result, big1->size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(big1->size);
      result->size = big1->size;
      pos = 0;
      do {
        carry += big1->bigdigits[pos];
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
        pos++;
      } while (pos < big1->size);
      if (IS_NEGATIVE(result->bigdigits[pos - 1])) {
        if (!IS_NEGATIVE(big1->bigdigits[pos - 1])) {
          if (!RESIZE_BIG(result, pos, pos + 1)) {
            raise_error(MEMORY_ERROR);
            return(NULL);
          } else {
            COUNT3_BIG(pos, pos + 1);
            result->size++;
            result->bigdigits[pos] = 0;
          } /* if */
        } else if (result->bigdigits[pos - 1] == BIGDIGIT_MASK &&
            pos >= 2 && IS_NEGATIVE(result->bigdigits[pos - 2])) {
          if (!RESIZE_BIG(result, pos, pos - 1)) {
            raise_error(MEMORY_ERROR);
            return(NULL);
          } else {
            COUNT3_BIG(pos, pos - 1);
            result->size--;
          } /* if */
        } /* if */
      } /* if */
      return(result);
    } /* if */
  } /* bigSucc */



#ifdef ANSI_C

biginttype bigUIConv (uinttype number)
#else

biginttype bigUIConv (number)
uinttype number;
#endif

  {
    memsizetype pos;
    memsizetype result_size;
    biginttype result;

  /* bigUIConv */
    result_size = sizeof(uinttype) / sizeof(bigdigittype) + 1;
    if (!ALLOC_BIG(result, result_size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(result_size);
      result->size = result_size;
      for (pos = 0; pos < result->size; pos++) {
        result->bigdigits[pos] = (bigdigittype) (number & BIGDIGIT_MASK);
        number >>= 8 * sizeof(bigdigittype);
      } /* for */
      normalize(result);
      return(result);
    } /* if */
  } /* bigUIConv */



#ifdef HAS_LONGTYPE_64
#ifdef ANSI_C

biginttype bigULConv (ulongtype number)
#else

biginttype bigULConv (number)
ulongtype number;
#endif

  {
    memsizetype pos;
    memsizetype result_size;
    biginttype result;

  /* bigULConv */
    result_size = sizeof(ulongtype) / sizeof(bigdigittype) + 1;
    if (!ALLOC_BIG(result, result_size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(result_size);
      result->size = result_size;
      for (pos = 0; pos < result->size; pos++) {
        result->bigdigits[pos] = (bigdigittype) (number & BIGDIGIT_MASK);
        number >>= 8 * sizeof(bigdigittype);
      } /* for */
      normalize(result);
      return(result);
    } /* if */
  } /* bigULConv */
#endif
