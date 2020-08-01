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

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "version.h"
#include "common.h"
#include "heaputl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "big_rtl.h"



#define LOWER_16(A) ((A) & 0177777L)
#define UPPER_16(A) (((A) >> 16) & 0177777L)
#define LOWER_32(A) ((A) & (uinttype) 037777777777L)

#define BIGDIGIT_MAX  037777777777L

#define BIGDIGIT_MASK 0177777
#define BIGDIGIT_SIGN 0100000

#define IS_NEGATIVE(digit) ((digit) & BIGDIGIT_SIGN)



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

static biginttype mult_bigdigit (biginttype big1, bigdigittype digit)
#else

static biginttype mult_bigdigit (big1, digit)
biginttype big1;
bigdigittype digit;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 0;

  /* mult_bigdigit */
    for (pos = 0; pos < big1->size; pos++) {
      carry += big1->bigdigits[pos] * digit;
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
    } /* for */
    if (carry != 0) {
      if (!RESIZE_BIG(big1, big1->size, big1->size + 1)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT3_BIG(big1->size, big1->size + 1);
        big1->bigdigits[pos] = carry;
        big1->size++;
      } /* if */
    } /* if */
    return(big1);
  } /* mult_bigdigit */



#ifdef ANSI_C

static biginttype add_bigdigit (biginttype big1, bigdigittype digit)
#else

static biginttype add_bigdigit (big1, digit)
biginttype big1;
bigdigittype digit;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry;

  /* add_bigdigit */
    carry = digit;
    for (; pos < big1->size; pos++) {
      carry += big1->bigdigits[pos];
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
    } /* for */
    if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
      big1->bigdigits[pos] = (bigdigittype) (carry - 1);
    } else {
      big1->bigdigits[pos] = (bigdigittype) carry;
    } /* if */
    big1->size = pos + 1;
    normalize(big1);
    return(big1);
  } /* add_bigdigit */



#ifdef ANSI_C

biginttype negate_big (biginttype big1)
#else

biginttype negate_big (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;
    bigdigittype negative;

  /* negate_big */
      negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
      for (pos = 0; pos < big1->size; pos++) {
        carry += ~big1->bigdigits[pos];
        big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
      } /* for */
      if (negative && IS_NEGATIVE(big1->bigdigits[pos - 1])) {
        if (!RESIZE_BIG(big1, pos, pos + 1)) {
          raise_error(MEMORY_ERROR);
          return(NULL);
        } else {
          COUNT3_BIG(pos, pos + 1);
          big1->bigdigits[pos] = 0;
          big1->size++;
        } /* if */
      } /* if */
      return(big1);
  } /* negate_big */



#ifdef ANSI_C

void negate_positive_big (biginttype big1)
#else

void negate_positive_big (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;

  /* negate_positive_big */
      for (pos = 0; pos < big1->size; pos++) {
        carry += ~big1->bigdigits[pos];
        big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
      } /* for */
      return(big1);
  } /* negate_positive_big */



#ifdef ANSI_C

static uinttype high_mult_32 (uinttype arg1, uinttype arg2)
#else

static uinttype high_mult_32 (arg1, arg2)
uinttype arg1;
uinttype arg2;
#endif

  {
    uinttype arg1_low;
    uinttype arg1_high;
    uinttype arg2_low;
    uinttype arg2_high;
    uinttype c1;
    uinttype c2;
    uinttype c3;
    uinttype c4;

  /* high_mult_32 */
    arg1_low  = LOWER_16(arg1);
    arg1_high = UPPER_16(arg1);
    arg2_low  = LOWER_16(arg2);
    arg2_high = UPPER_16(arg2);
    c1 = UPPER_16(arg1_low * arg2_low);
    c2 = arg1_low * arg2_high;
    c3 = arg1_high * arg2_low;
    c4 = UPPER_16(c1 + LOWER_16(c2) + LOWER_16(c3)) +
        UPPER_16(c2) + UPPER_16(c3) +
        arg1_high * arg2_high;
    return(LOWER_32(c4));
  } /* high_mult_32 */



#ifdef ANSI_C

static uinttype mult_32x16 (uinttype arg1, uinttype arg2)
#else

static uinttype mult_32x16 (arg1, arg2)
uinttype arg1;
uinttype arg2;
#endif

  {
    uinttype arg1_low;
    uinttype arg1_high;

  /* mult_32x16 */
    arg1_low  = LOWER_16(arg1);
    arg1_high = UPPER_16(arg1);
    return(UPPER_16(arg1_low * arg2) + arg1_high * arg2);
  } /* mult_32x16 */



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
        for (pos = 0; pos < big1->size; pos++) {
          carry += ~big1->bigdigits[pos];
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
        } /* for */
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
      for (pos = 0; pos < big2->size; pos++) {
        carry += big1->bigdigits[pos] + big2->bigdigits[pos];
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
      } /* for */
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
    big1_negative = big1->bigdigits[big1->size - 1] >> 8 * sizeof(bigdigittype) - 1;
    big2_negative = big2->bigdigits[big2->size - 1] >> 8 * sizeof(bigdigittype) - 1;
    if (big1_negative != big2_negative) {
      return(big1_negative ? -1 : 1);
    } else if (big1->size != big2->size) {
      return((big1->size < big2->size) != big1_negative ? -1 : 1);
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

void bigDecr (biginttype *big_variable)
#else

void bigDecr (big_variable)
biginttype *big_variable;
#endif

  {
    biginttype big1;
    memsizetype pos;
    doublebigdigittype sum;
    doublebigdigittype carry = BIGDIGIT_MASK;

  /* bigDecr */
    big1 = *big_variable;
    for (pos = 0; carry && pos < big1->size; pos++) {
      sum = carry + big1->bigdigits[pos];
      carry = sum >> 8 * sizeof(bigdigittype);
      big1->bigdigits[pos] = (bigdigittype) (sum & BIGDIGIT_MASK);
    } /* for */
    if (carry) {
      if (!RESIZE_BIG(big1, big1->size, big1->size + 1)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_BIG(big1->size, big1->size + 1);
        big1->bigdigits[pos] = carry;
        big1->size++;
        *big_variable = big1;
      } /* if */
    } /* if */
  } /* bigDecr */



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



#ifdef OUT_OF_ORDER
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
    biginttype result;

  /* bigIncr */
    big1 = *big_variable;
    if (!ALLOC_BIG(result, big1->size + 1)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(big1->size + 1);
      for (; pos < big1->size; pos++) {
        carry += big1->bigdigits[pos];
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
      } /* for */
      if (IS_NEGATIVE(big1->bigdigits[pos - 1])) {
        result->bigdigits[pos] = (bigdigittype) (carry - 1);
      } else {
        result->bigdigits[pos] = (bigdigittype) carry;
      } /* if */
      result->size = pos + 1;
      normalize(result);
      return(result);
    } /* if */
  } /* bigIncr */
#endif



#ifdef ANSI_C

void bigIncr (biginttype *big_variable)
#else

void bigIncr (big_variable)
biginttype *big_variable;
#endif

  {
    biginttype big1;
    doublebigdigittype sign;
    memsizetype pos;
    doublebigdigittype carry = 1;

  /* bigIncr */
    big1 = *big_variable;
    sign = big1->bigdigits[big1->size - 1] >> 8 * sizeof(bigdigittype) - 1;
    /* printf("  bigIncr size=%ld sign=%ld\n", big1->size, sign); */
    for (pos = 0; /* carry != sign && */ pos < big1->size; pos++) {
      carry += big1->bigdigits[pos];
      big1->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
      carry >>= 8 * sizeof(bigdigittype);
      /* printf("  carry=%ld\n", carry); */
    } /* for */
    /* printf("sign=%d, high_bit=%d\n", sign, big1->bigdigits[big1->size - 1] >> 8 * sizeof(bigdigittype) - 1); */
    if (!sign && big1->bigdigits[big1->size - 1] >> 8 * sizeof(bigdigittype) - 1) {
      /* printf(" **** A carry=%ld\n", carry); */
      if (!RESIZE_BIG(big1, big1->size, big1->size + 1)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_BIG(big1->size, big1->size + 1);
        big1->bigdigits[pos] = carry;
        big1->size++;
        *big_variable = big1;
      } /* if */
    } else if (big1->size >= 2 &&
        big1->bigdigits[big1->size - 1] == BIGDIGIT_MASK &&
        big1->bigdigits[big1->size - 2] >> 8 * sizeof(bigdigittype) - 1) {
      /* printf(" **** B carry=%ld\n", carry); */
      if (!RESIZE_BIG(big1, big1->size, big1->size - 1)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_BIG(big1->size, big1->size - 1);
        big1->size--;
        *big_variable = big1;
      } /* if */
    } /* if */
  } /* bigIncr */



#ifdef ANSI_C

biginttype bigMinus (biginttype big1)
#else

biginttype bigMinus (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    doublebigdigittype carry = 1;
    bigdigittype negative;
    biginttype result;

  /* bigMinus */
    if (!ALLOC_BIG(result, big1->size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(big1->size);
      negative = IS_NEGATIVE(big1->bigdigits[big1->size - 1]);
      for (pos = 0; pos < big1->size; pos++) {
        carry += ~big1->bigdigits[pos];
        result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
        carry >>= 8 * sizeof(bigdigittype);
      } /* for */
      if (negative && IS_NEGATIVE(result->bigdigits[pos - 1])) {
        result->size++;
        if (!RESIZE_BIG(result, big1->size, result->size)) {
          raise_error(MEMORY_ERROR);
          return(NULL);
        } else {
          COUNT3_BIG(big1->size, result->size);
          result->bigdigits[big1->size] = 0;
        } /* if */
      } /* if */
      return(result);
    } /* if */
  } /* bigMinus */



#ifdef ANSI_C

biginttype bigMult (biginttype big1, biginttype big2)
#else

biginttype bigMult (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    memsizetype pos1;
    memsizetype pos2;
    bigdigittype digit1;
    bigdigittype digit2;
    bigdigittype carry = 0;
    biginttype result;

  /* bigMult */
    if (!ALLOC_BIG(result, big1->size + big2->size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(big1->size + big2->size);
#ifdef OUT_OF_ORDER
      for (pos1 = 0; pos1 < big1->size + big2->size; pos1++) {
        result->bigdigits[pos1] = 0;
      } /* for */
      for (pos1 = 0; pos1 < big1->size; pos1++) {
        for (pos2 = 0; pos2 < big2->size; pos2++) {
          digit1 = big1->bigdigits[pos1] * big2->bigdigits[pos1];
          if (BIGDIGIT_MAX - digit1 >= carry) {
            digit1 += carry;
          } else {
            digit1 += carry - BIGDIGIT_MAX;
          } /* if */
          carry = high_mult_32(big1->bigdigits[pos1], big2->bigdigits[pos1]);
          if (BIGDIGIT_MAX - result->bigdigits[pos1 + pos2] >= digit1) {
            result->bigdigits[pos1 + pos2] += digit1;
          } else {
            result->bigdigits[pos1 + pos2] += digit1 - BIGDIGIT_MAX;
          } /* if */
          result->bigdigits[pos1 + pos2] = 

        if (pos >= big1->size) {
          digit1 = big1->sign;
        } else {
          digit1 = big1->bigdigits[pos];
        } /* if */
        if (pos >= big2->size) {
          digit2 = big2->sign;
        } else {
          digit2 = big2->bigdigits[pos];
        } /* if */
        if (digit1 > digit2) {
          result->bigdigits[pos] = digit1 - digit2 - carry;
          carry = 0;
        } else if (digit1 == digit2) {
          if (carry) {
            result->bigdigits[pos] = BIGDIGIT_MAX;
          } else {
            result->bigdigits[pos] = 0;
          } /* if */
        } else {
          /* result->bigdigits[pos] = BIGDIGIT_BASE + digit1 - digit2; */
          result->bigdigits[pos] = BIGDIGIT_MAX - (digit2 - 1) + digit1;
          carry = 1;
        } /* if */
      } /* for */
      if (carry) {
        result->sign = BIGDIGIT_MAX;
      } else {
        result->sign = 0;
      } /* if */
      result->size = big1->size + big2->size;
#endif
      return(result);
    } /* if */
  } /* bigMult */



#ifdef ANSI_C

void bigMultNumber (biginttype big1, inttype number)
#else

void bigMultNumber (big1, number)
biginttype big1;inttype number;
#endif

  {
    memsizetype pos;
    bigdigittype digit1;
    bigdigittype carry = 0;

  /* bigMultNumber */
    for (pos = 0; pos < big1->size; pos++) {
      digit1 = big1->bigdigits[pos] * number;
      if (BIGDIGIT_MASK - digit1 >= carry) {
        digit1 += carry;
      } else {
        digit1 += carry - BIGDIGIT_MASK;
      } /* if */
      carry = high_mult_32(big1->bigdigits[pos], number);
      big1->bigdigits[pos] = digit1;
    } /* for */
    if (carry) {
      if (!RESIZE_BIG(big1, big1->size, big1->size + 1)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_BIG(big1->size, big1->size + 1);
        big1->bigdigits[big1->size] = carry;
        big1->size++;
      } /* if */
    } /* if */
  } /* bigMultNumber */



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

biginttype bigParse (stritype stri)
#else

biginttype bigParse (stri)
stritype stri;
#endif

  {
    booltype okay;
    booltype negative;
    memsizetype position;
    bigdigittype digitval;
    biginttype integer_value;

  /* bigParse */
    if (!ALLOC_BIG(integer_value, 1)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(1);
      integer_value->size = 1;
      integer_value->bigdigits[0] = 0;
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
        digitval = ((bigdigittype) stri->mem[position]) - ((bigdigittype) '0');
        mult_bigdigit(integer_value, 10);
        add_bigdigit(integer_value, digitval);
        position++;
      } /* while */
      if (position == 0 || position < stri->size) {
        okay = FALSE;
      } /* if */
      if (okay) {
        if (negative) {
          negate_positive_big(integer_value);
        } /* if */
        return(integer_value);
      } else {
        raise_error(RANGE_ERROR);
        return(NULL);
      } /* if */
    } /* if */
  } /* bigParse */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

biginttype bigParse (stritype stri)
#else

biginttype bigParse (stri)
stritype stri;
#endif

  {
    booltype okay;
    booltype negative;
    memsizetype position;
    inttype digitval;
    inttype integer_value;

  /* bigParse */
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
      bigMultNumber(integer_value, 10);
      bigAddNumber(integer_value, digitval);
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
  } /* bigParse */
#endif



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
        for (pos = 0; pos < big2->size; pos++) {
          carry += big1->bigdigits[pos] + ~big2->bigdigits[pos];
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
        } /* for */
        big2_sign = IS_NEGATIVE(big2->bigdigits[pos - 1]) ? 0: BIGDIGIT_MASK;
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
        for (pos = 0; pos < big1->size; pos++) {
          carry += big1->bigdigits[pos] + ~big2->bigdigits[pos];
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
        } /* for */
        big1_sign = IS_NEGATIVE(big1->bigdigits[pos - 1]) ? BIGDIGIT_MASK : 0;
        for (; pos < big2->size; pos++) {
          carry += big1_sign + ~big2->bigdigits[pos];
          result->bigdigits[pos] = (bigdigittype) (carry & BIGDIGIT_MASK);
          carry >>= 8 * sizeof(bigdigittype);
        } /* for */
        if (IS_NEGATIVE(big2->bigdigits[pos - 1])) {
          big1_sign--;
        } /* if */
        result->bigdigits[pos] = (bigdigittype) (carry + big1_sign);
        result->size = pos + 1;
        normalize(result);
        return(result);
      } /* if */
    } /* if */
  } /* bigSbtr */



#ifdef ANSI_C

void bigDump (biginttype big1)
#else

void bigDump (big1)
biginttype big1;
#endif

  {
    memsizetype pos;
    bigdigittype *digitPos;

  /* bigDump */
    printf("[%d] (%hd",
        big1->size,
        (short int) big1->bigdigits[big1->size - 1]);
    if (big1->size >= 2) {
      for (digitPos = &big1->bigdigits[big1->size - 2]; digitPos >= big1->bigdigits; digitPos--) {
        printf(", %hu", *digitPos);
      } /* for */
    } /* if */
    printf(")  [%04hx",
        big1->bigdigits[big1->size - 1]);
    if (big1->size >= 2) {
      for (digitPos = &big1->bigdigits[big1->size - 2]; digitPos >= big1->bigdigits; digitPos--) {
        printf(", %04hx", *digitPos);
      } /* for */
    } /* if */
    printf("]\n");
  } /* bigDump */



#ifdef ANSI_C

void bigDump2 (biginttype big1)
#else

void bigDump2 (big1)
biginttype big1;
#endif

  {
    bigdigittype *digitPos;

  /* bigDump2 */
    printf("[%d] [%04hx",
        big1->size,
        big1->bigdigits[big1->size - 1]);
    if (big1->size >= 2) {
      for (digitPos = &big1->bigdigits[big1->size - 2]; digitPos >= big1->bigdigits; digitPos--) {
        printf(", %04hx", *digitPos);
      } /* for */
    } /* if */
    printf("]\n");
  } /* bigDump2 */



biginttype newBigInt (memsizetype size, bigdigittype bigdigits[])

  {
    memsizetype pos;
    biginttype result;

  /* newBigInt */
    if (!ALLOC_BIG(result, size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BIG(size);
      result->size = size;
      for (pos = 0; pos < size; pos++) {
        result->bigdigits[pos] = bigdigits[pos];
      } /* for */
    } /* if */
    return(result);
  } /* newBigInt */


bigdigittype low[]           = {0, 0, -2};
bigdigittype bdm2147483650[] = {0, -32768};
bigdigittype bdm65538[]      = {65534, -2};
bigdigittype bdm65537[]      = {65535, -2};
bigdigittype bdm65536[]      = {0, -1};
bigdigittype bdm65535[]      = {1, -1};
bigdigittype bdm65534[]      = {2, -1};
bigdigittype bdm32770[]      = {32766, -1};
bigdigittype bdm32769[]      = {32767, -1};
bigdigittype bdm32768[]      = {-32768};
bigdigittype bdm32767[]      = {-32767};
bigdigittype bdm32766[]      = {-32766};
bigdigittype bdm2[]          = {-2};
bigdigittype bdm1[]          = {-1};
bigdigittype bd0[]           = {0};
bigdigittype bd1[]           = {1};
bigdigittype bd2[]           = {2};
bigdigittype bd32766[]       = {32766};
bigdigittype bd32767[]       = {32767};
bigdigittype bd32768[]       = {32768, 0};
bigdigittype bd32769[]       = {32769, 0};
bigdigittype bd65534[]       = {65534, 0};
bigdigittype bd65535[]       = {65535, 0};
bigdigittype bd65536[]       = {0, 1};
bigdigittype bd65537[]       = {1, 1};
bigdigittype bd65538[]       = {2, 1};
bigdigittype bd2147483646[]  = {65534, 32767};
bigdigittype bd2147483647[]  = {65535, 32767};
bigdigittype bd2147483648[]  = {0, 32768, 0};
bigdigittype bd2147483649[]  = {1, 32768, 0};
bigdigittype bd2147483650[]  = {2, 32768, 0};
bigdigittype bd4294967294[]  = {65534, 65535, 0};
bigdigittype bd4294967295[]  = {65535, 65535, 0};
bigdigittype bd4294967296[]  = {0, 0, 1};
bigdigittype bd4294967297[]  = {1, 0, 1};
bigdigittype bd4294967298[]  = {2, 0, 1};


biginttype bigm32769;
biginttype bigm32768;
biginttype bigm32767;
biginttype bigm32766;
biginttype bigm2;
biginttype bigm1;
biginttype big0;
biginttype big1;
biginttype big2;
biginttype big32766;
biginttype big32767;
biginttype big32768;
biginttype big32769;


void test_big (int argc, char **argv)
  {
    biginttype big_test1;
    int counter;

    printf("sizeof(bigdigittype)=%ld\n", sizeof(bigdigittype));

    bigm32769 = newBigInt(2, bdm32769);
    bigm32768 = newBigInt(1, bdm32768);
    bigm32767 = newBigInt(1, bdm32767);
    bigm32766 = newBigInt(1, bdm32766);
    bigm2     = newBigInt(1, bdm2);
    bigm1     = newBigInt(1, bdm1);
    big0      = newBigInt(1, bd0);
    big1      = newBigInt(1, bd1);
    big2      = newBigInt(1, bd2);
    big32766  = newBigInt(1, bd32766);
    big32767  = newBigInt(1, bd32767);
    big32768  = newBigInt(2, bd32768);
    big32769  = newBigInt(2, bd32769);

    bigDump(bigm32769);
    bigDump(bigm32768);
    bigDump(bigm32767);
    bigDump(bigm32766);
    bigDump(bigm2);
    bigDump(bigm1);
    bigDump(big0);
    bigDump(big1);
    bigDump(big2);
    bigDump(big32766);
    bigDump(big32767);
    bigDump(big32768);
    bigDump(big32769);

    printf("\n");
    big_test1 = newBigInt(2, bdm65538);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);

    printf("\n");
    big_test1 = newBigInt(2, bdm32770);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);

    printf("\n");
    big_test1 = newBigInt(1, bdm2);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);

    printf("\n");
    big_test1 = newBigInt(1, bd32766);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);

    printf("\n");
    big_test1 = newBigInt(2, bd65534);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);

    printf("\n");
    big_test1 = newBigInt(2, bd2147483646);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);

    printf("\n");
    big_test1 = newBigInt(3, bd4294967294);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);
    bigIncr(&big_test1);
    bigDump(big_test1);

#ifdef OUT_OF_ORDER
    printf("\n");
    big_test1 = newBigInt(3, low);
    bigDump2(big_test1);
    counter = 0;
    while (big_test1->size != 1 || big_test1->bigdigits[0] != 0) {
      bigIncr(&big_test1);
      counter++;
      if (counter == 1000000) {
        bigDump2(big_test1);
        counter = 0;
      } /* if */
    } /* while */

    printf("\n");
    big_test1 = newBigInt(1, bdm32768);
    bigDump2(big_test1);
    counter = 0;
    while (big_test1->size != 2) {
      bigIncr(&big_test1);
      counter++;
      if (counter == 1000000) {
        bigDump2(big_test1);
        counter = 0;
      } else {
        bigDump2(big_test1);
      } /* if */
    } /* while */
#endif
  }
