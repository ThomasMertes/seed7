/********************************************************************/
/*                                                                  */
/*  int_rtl.c     Primitive actions for the integer type.           */
/*  Copyright (C) 1989 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/int_rtl.c                                       */
/*  Changes: 1992, 1993, 1994, 2000, 2005  Thomas Mertes            */
/*  Content: Primitive actions for the integer type.                */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "time.h"

#include "version.h"
#include "common.h"
#include "heaputl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "int_rtl.h"


#define LOWER_16(A) ((A) & 0177777L)
#define UPPER_16(A) (((A) >> 16) & 0177777L)
#define LOWER_31(A) ((A) & (uinttype) 017777777777L)
#define LOWER_32(A) ((A) & (uinttype) 037777777777L)
#define BIT_32_SET(A) (((A) & (uinttype) 020000000000L) != 0)


static int most_significant[] = {
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

static int least_significant[] = {
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



#ifdef ANSI_C

static void mult_64 (uinttype a_high, uinttype a_low,
    uinttype b_high, uinttype b_low,
    uinttype *c_high, uinttype *c_low)
#else

static void mult_64 (a_high, a_low, b_high, b_low, c_high, c_low)
uinttype a_high;
uinttype a_low;
uinttype b_high;
uinttype b_low;
uinttype *c_high;
uinttype *c_low;
#endif

  {
    uinttype a_low1;
    uinttype a_low2;
    uinttype b_low1;
    uinttype b_low2;
    uinttype c1;
    uinttype c2;
    uinttype c3;
    uinttype c4;

  /* mult_64 */
#ifdef TRACE_RANDOM
    printf("BEGIN mult_64\n");
#endif
    a_low1 = LOWER_16(a_low);
    a_low2 = UPPER_16(a_low);
    b_low1 = LOWER_16(b_low);
    b_low2 = UPPER_16(b_low);
    c1 = UPPER_16(a_low1 * b_low1);
    c2 = a_low1 * b_low2;
    c3 = a_low2 * b_low1;
    c4 = UPPER_16(c1 + LOWER_16(c2) + LOWER_16(c3)) +
        UPPER_16(c2) + UPPER_16(c3) +
        a_low2 * b_low2;
    *c_low = LOWER_32(a_low * b_low);
    *c_high = LOWER_32(a_low * b_high + a_high * b_low + c4);
#ifdef TRACE_RANDOM
    printf("END mult_64\n");
#endif
  } /* mult_64 */



#ifdef ANSI_C

static INLINE void add_64 (uinttype a_high, uinttype a_low,
    uinttype b_high, uinttype b_low,
    uinttype *c_high, uinttype *c_low)
#else

static INLINE void add_64 (a_high, a_low, b_high, b_low, c_high, c_low)
uinttype a_high;
uinttype a_low;
uinttype b_high;
uinttype b_low;
uinttype *c_high;
uinttype *c_low;
#endif

  { /* add_64 */
#ifdef TRACE_RANDOM
    printf("BEGIN add_64\n");
#endif
    *c_low = LOWER_32(a_low + b_low);
    if (BIT_32_SET(a_low) + BIT_32_SET(b_low) +
        BIT_32_SET(LOWER_31(a_low) + LOWER_31(b_low)) >= 2) {
      *c_high = LOWER_32(a_high + b_high + 1);
    } else {
      *c_high = LOWER_32(a_high + b_high);
    } /* if */
#ifdef TRACE_RANDOM
    printf("END add_64\n");
#endif
  } /* add_64 */



#ifdef ANSI_C

uinttype rand_32 (void)
#else

uinttype rand_32 ()
#endif

  {
    static booltype seed_necessary = TRUE;
    static uinttype low_seed;
    static uinttype high_seed;

  /* rand_32 */
#ifdef TRACE_RANDOM
    printf("BEGIN rand_32\n");
#endif
    if (seed_necessary) {
      high_seed = (uinttype) time(NULL);
      high_seed = high_seed ^ (high_seed << 16);
      low_seed = (uinttype) clock();
      low_seed = (low_seed ^ (low_seed << 16)) ^ high_seed;
/*    printf("%10lo %010lo seed\n", high_seed, low_seed); */
      seed_necessary = FALSE;
    } /* if */
    /* SEED = SEED * 1073741825 + 1234567891 */
    mult_64(high_seed, low_seed, (uinttype) 0L, (uinttype) 1073741825L,
        &high_seed, &low_seed);
/*  printf("%10lo %010lo\n", high_seed, low_seed); */
    add_64(high_seed, low_seed, (uinttype) 0L, (uinttype) 1234567891L,
        &high_seed, &low_seed);
/*  printf("%10lo %010lo\n", high_seed, low_seed); */
#ifdef TRACE_RANDOM
    printf("END rand_32\n");
#endif
    return(high_seed);
  } /* rand_32 */



#ifdef ANSI_C

inttype most_significant_bit (uinttype number)
#else

inttype most_significant_bit (number)
uinttype number;
#endif

  {
    int result;

  /* most_significant_bit */
    result = 0;
    if (number & 0xffff0000) {
      number >>= 16;
      result = 16;
    } /* if */
    if (number & 0xff00) {
      number >>= 8;
      result += 8;
    } /* if */
    result += most_significant[number];
    return((inttype) result);
  } /* most_significant_bit */



#ifdef ANSI_C

inttype least_significant_bit (uinttype number)
#else

inttype least_significant_bit (number)
uinttype number;
#endif

  {
    int result;

  /* least_significant_bit */
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
    return((inttype) result);
  } /* least_significant_bit */



#ifdef ANSI_C

inttype intBinom (inttype n_number, inttype k_number)
#else

inttype intBinom (n_number, k_number)
inttype n_number;
inttype k_number;
#endif

  {
    inttype number;
    uinttype result;

  /* intBinom */
    /* printf("(%ld ! %ld) ", k_number, n_number); */
    if (2 * k_number > n_number) {
      k_number = n_number - k_number;
    } /* if */
    if (k_number < 0) {
      result = 0;
    } else if (k_number == 0) {
      result = 1;
    } else /* if (n_number <= 30 || k_number <= 7)  */{
      result = n_number;
      for (number = 2; number <= k_number; number++) {
        result *= (n_number - number + 1);
        result /= number;
      } /* for */
/*
    } else {
      result = binom(n_number - 1, k_number - 1) +
          binom(n_number - 1, k_number);
*/
    } /* if */
    /* printf("--> %ld\n", result); */
    return((inttype) result);
  } /* intBinom */



#ifdef ANSI_C

inttype intCmp (inttype number1, inttype number2)
#else

inttype intCmp (number1, number2)
inttype number1;
inttype number2;
#endif

  { /* intCmp */
    if (number1 < number2) {
      return(-1);
    } else if (number1 > number2) {
      return(1);
    } else {
      return(0);
    } /* if */
  } /* intCmp */



#ifdef ANSI_C

void intCpy (inttype *dest, inttype source)
#else

void intCpy (dest, source)
inttype *dest;
inttype source;
#endif

  { /* intCpy */
    *dest = source;
  } /* intCpy */



#ifdef ANSI_C

void intCreate (inttype *dest, inttype source)
#else

void intCreate (dest, source)
inttype *dest;
inttype source;
#endif

  { /* intCreate */
    *dest = source;
  } /* intCreate */



#ifdef ANSI_C

inttype intLd (inttype number)
#else

inttype intLd (number)
inttype number;
#endif

  {
    int result;

  /* intLd */
    if (number < 0) {
      raise_error(NUMERIC_ERROR);
      return(0);
    } else {
      result = 0;
      if (number & 0xffff0000) {
        number >>= 16;
        result = 16;
      } /* if */
      if (number & 0xff00) {
        number >>= 8;
        result += 8;
      } /* if */
      result += most_significant[number];
      return(result);
    } /* if */
  } /* intLd */



#ifdef ANSI_C

inttype intParse (stritype stri)
#else

inttype intParse (stri)
stritype stri;
#endif

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
    if (stri->size >= 1 && stri->mem[0] == ((strelemtype) '-')) {
      negative = TRUE;
      position++;
    } else {
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
    if (position == 0 || position < stri->size) {
      okay = FALSE;
    } /* if */
    if (okay) {
      if (negative) {
        return(-integer_value);
      } else {
        return(integer_value);
      } /* if */
    } else {
      raise_error(RANGE_ERROR);
      return(0);
    } /* if */
  } /* intParse */



#ifdef ANSI_C

inttype intPow (inttype base, inttype exp)
#else

inttype intPow (base, exp)
inttype base;
inttype exp;
#endif

  {
    inttype result;

  /* intPow */
    if (exp < 0 || (base == 0 && exp == 0)) {
      raise_error(NUMERIC_ERROR);
      return(0);
    } else {
      if (exp & 1) {
        result = base;
      } else {
        result = 1;
      } /* if */
      exp = exp >> 1;
      while (exp != 0) {
        base = base * base;
        if (exp & 1) {
          result = result * base;
        } /* if */
        exp = exp >> 1;
      } /* while */
      return(result);
    } /* if */
  } /* intPow */



#ifdef ANSI_C

inttype intRand (inttype lower_limit, inttype upper_limit)
#else

inttype intRand (lower_limit, upper_limit)
inttype lower_limit;
inttype upper_limit;
#endif

  {
    uinttype scale_limit;
    uinttype low_factor;
    uinttype high_factor;
    uinttype low_rand;
    uinttype high_rand;
    inttype result;

  /* intRand */
    if (lower_limit > upper_limit) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      scale_limit = upper_limit - lower_limit;
      if (scale_limit <= ULONG_MAX - 2) {
        high_factor = 0L;
      } else {
        high_factor = 1L;
      } /* if */
      low_factor = scale_limit + 2;
      do {
        /* RAND = (rand_32() * FACTOR) >> 32; */
        mult_64((uinttype) 0L, rand_32(),
            high_factor, low_factor,
            &high_rand, &low_rand);
      } while (high_rand > scale_limit);
      result = lower_limit + high_rand;
      return(result);
    } /* if */
  } /* intRand */



#ifdef ANSI_C

inttype intSqrt (inttype num)
#else

inttype intSqrt (num)
inttype num;
#endif

  {
    register unsigned long result;
    register unsigned long res2;
    unsigned long number;

  /* intSqrt */
    if (num < 0) {
      raise_error(NUMERIC_ERROR);
      return(0);
    } else if (num == 0) {
      return((inttype) 0);
    } else {
      number = (unsigned long) num;
      res2 = number;
      do {
        result = res2;
        res2 = (result + number / result) >> 1;
      } while (result > res2);
      return((inttype) result);
    } /* if */
  } /* intSqrt */



#ifdef ANSI_C

stritype intStr (inttype arg1)
#else

stritype intStr (arg1)
inttype arg1;
#endif

  {
    uinttype number;
    booltype sign;
    strelemtype buffer_1[50];
    strelemtype *buffer;
    memsizetype len;
    stritype result;

  /* intStr */
    if ((sign = (arg1 < 0))) {
      number = -arg1;
    } else {
      number = arg1;
    } /* if */
    buffer = &buffer_1[50];
    do {
      *(--buffer) = (strelemtype) (number % 10 + '0');
    } while ((number /= 10) != 0);
    if (sign) {
      *(--buffer) = (strelemtype) '-';
    } /* if */
    len = &buffer_1[50] - buffer;
    if (!ALLOC_STRI(result, len)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_STRI(len);
      result->size = len;
      memcpy(result->mem, buffer, (SIZE_TYPE) (len * sizeof(strelemtype)));
      return(result);
    } /* if */
  } /* intStr */
