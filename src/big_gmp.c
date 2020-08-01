/********************************************************************/
/*                                                                  */
/*  big_gmp.c     Functions for bigInteger using the gmp library.   */
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
/*  File: seed7/src/big_gmp.c                                       */
/*  Changes: 2008, 2009, 2010  Thomas Mertes                        */
/*  Content: Functions for bigInteger using the gmp library.        */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "gmp.h"

#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "big_drv.h"



#ifdef ANSI_C

cstritype bigHexCStri (const_biginttype big1)
#else

cstritype bigHexCStri (big1)
biginttype big1;
#endif

  {
    size_t count;
    size_t export_count;
    size_t pos;
    int sign;
    unsigned short carry;
    ustritype buffer;
    memsizetype charIndex;
    size_t result_size;
    cstritype result;

  /* bigHexCStri */
    if (big1 != NULL) {
      count = (mpz_sizeinbase(big1, 2) + 7) / 8;
      buffer = malloc(count);
      if (buffer == NULL) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        mpz_export(buffer, &export_count, 1, 1, 0, 0, big1);
        sign = mpz_sgn(big1);
        if (sign < 0) {
          carry = 1;
          pos = export_count;
          while (pos > 0) {
            pos--;
            carry += ~buffer[pos] & 0xFF;
            buffer[pos] = carry & 0xFF;
            carry >>= 8;
          } /* while */
        } /* if */
        result_size = 3 + count * 2;
        if ((sign > 0 && buffer[0] >= 128) ||
            (sign < 0 && buffer[0] <= 127)) {
          result_size += 2;
        } /* if */
        if (!ALLOC_CSTRI(result, result_size)) {
          raise_error(MEMORY_ERROR);
        } else {
          if (sign == 0) {
            sprintf(result, "16#00");
          } else {
            sprintf(result, "16#");
            charIndex = 3;
            if (sign < 0) {
              if (buffer[0] <= 127) {
                sprintf(&result[charIndex], "FF");
                charIndex += 2;
              } /* if */
            } else {
              if (buffer[0] >= 128) {
                sprintf(&result[charIndex], "00");
                charIndex += 2;
              } /* if */
            } /* for */
            for (pos = 0; pos < export_count; pos++) {
              sprintf(&result[charIndex], "%02X", buffer[pos]);
              charIndex += 2;
            } /* for */
          } /* if */
        } /* if */
        free(buffer);
      } /* if */
    } else {
      buffer = " *NULL_BIGINT* ";
      if (!ALLOC_CSTRI(result, strlen(buffer))) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        strcpy(result, buffer);
      } /* if */
    } /* if */
    return(result);
  } /* bigHexCStri */



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
    biginttype result;

  /* bigAbs */
    result = malloc(sizeof(__mpz_struct));
    mpz_init(result);
    mpz_abs(result, big1);
    return(result);
  } /* bigAbs */



/**
 *  Returns the sum of two signed big integers.
 */
#ifdef ANSI_C

biginttype bigAdd (const_biginttype big1, const_biginttype big2)
#else

biginttype bigAdd (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    biginttype result;

  /* bigAdd */
    result = malloc(sizeof(__mpz_struct));
    mpz_init(result);
    mpz_add(result, big1, big2);
    return(result);
  } /* bigAdd */



/**
 *  Returns the sum of two signed big integers.
 *  Big1 is assumed to be a temporary value which is reused.
 */
#ifdef ANSI_C

biginttype bigAddTemp (biginttype big1, const_biginttype big2)
#else

biginttype bigAddTemp (big1, big2)
biginttype big1;
biginttype big2;
#endif

  { /* bigAddTemp */
    mpz_add(big1, big1, big2);
    return(big1);
  } /* bigAddTemp */



#ifdef ANSI_C

inttype bigBitLength (const const_biginttype big1)
#else

inttype bigBitLength (big1)
biginttype big1;
#endif

  {
    inttype result;

  /* bigBitLength */
    result = mpz_sizeinbase(big1, 2);
    return(result);
  } /* bigBitLength */



#ifdef ANSI_C

stritype bigCLit (const const_biginttype big1)
#else

stritype bigCLit (big1)
biginttype big1;
#endif

  {
    size_t count;
    size_t export_count;
    size_t pos;
    char byteBuffer[22];
    int sign;
    unsigned short carry;
    ustritype buffer;
    memsizetype charIndex;
    memsizetype result_size;
    stritype result;

  /* bigCLit */
    count = (mpz_sizeinbase(big1, 2) + 7) / 8;
    buffer = malloc(count);
    if (buffer == NULL) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      mpz_export(buffer, &export_count, 1, 1, 0, 0, big1);
      sign = mpz_sgn(big1);
      if (sign < 0) {
        carry = 1;
        pos = export_count;
        while (pos > 0) {
          pos--;
          carry += ~buffer[pos] & 0xFF;
          buffer[pos] = carry & 0xFF;
          carry >>= 8;
        } /* while */
      } /* if */
      result_size = 21 + count * 5;
      if ((sign > 0 && buffer[0] >= 128) ||
          (sign < 0 && buffer[0] <= 127)) {
        result_size += 5;
        count++;
      } /* if */
      if (!ALLOC_STRI(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        sprintf(byteBuffer, "{0x%02X,0x%02X,0x%02X,0x%02X,",
            (unsigned int) (count >> 24 & 0xFF),
            (unsigned int) (count >> 16 & 0xFF),
            (unsigned int) (count >>  8 & 0xFF),
            (unsigned int) (count       & 0xFF));
        cstri_expand(result->mem, byteBuffer, 21);
        charIndex = 21;
        if (sign == 0) {
          cstri_expand(&result->mem[charIndex], "0x00}", 5);
        } else {
          if (sign < 0) {
            if (buffer[0] <= 127) {
              cstri_expand(&result->mem[charIndex], "0xFF,", 5);
              charIndex += 5;
            } /* if */
          } else {
            if (buffer[0] >= 128) {
              cstri_expand(&result->mem[charIndex], "0x00,", 5);
              charIndex += 5;
            } /* if */
          } /* for */
          for (pos = 0; pos < export_count; pos++) {
            sprintf(byteBuffer, "0x%02X,", buffer[pos]);
            cstri_expand(&result->mem[charIndex], byteBuffer, 5);
            charIndex += 5;
          } /* for */
          charIndex -= 5;
          result->mem[charIndex + 4] = '}';
        } /* if */
      } /* if */
      free(buffer);
    } /* if */
    return(result);
  } /* bigCLit */



#ifdef ANSI_C

inttype bigCmp (const const_biginttype big1, const const_biginttype big2)
#else

inttype bigCmp (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    inttype result;

  /* bigCmp */
    result = mpz_cmp(big1, big2);
    if (result < 0) {
      result = -1;
    } else if (result > 0) {
      result = 1;
    } /* if */
    return(result);
  } /* bigCmp */



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
    result = mpz_cmp_si(big1, number);
    if (result < 0) {
      result = -1;
    } else if (result > 0) {
      result = 1;
    } /* if */
    return(result);
  } /* bigCmpSignedDigit */



#ifdef ANSI_C

void bigCpy (biginttype *const big_to, const const_biginttype big_from)
#else

void bigCpy (big_to, big_from)
biginttype *big_to;
biginttype big_from;
#endif

  { /* bigCpy */
    mpz_set(*big_to, big_from);
  } /* bigCpy */



#ifdef ANSI_C

biginttype bigCreate (const const_biginttype big_from)
#else

biginttype bigCreate (big_from)
biginttype big_from;
#endif

  {
    biginttype result;

  /* bigCreate */
    result = malloc(sizeof(__mpz_struct));
    mpz_init_set(result, big_from);
    return(result);
  } /* bigCreate */



#ifdef ANSI_C

void bigDecr (biginttype *const big_variable)
#else

void bigDecr (big_variable)
biginttype *big_variable;
#endif

  { /* bigDecr */
    mpz_sub_ui(*big_variable, *big_variable, 1);
  } /* bigDecr */



#ifdef ANSI_C

void bigDestr (const biginttype old_bigint)
#else

void bigDestr (old_bigint)
biginttype old_bigint;
#endif

  { /* bigDestr */
    if (old_bigint != NULL) {
      mpz_clear(old_bigint);
      free(old_bigint);
    } /* if */
  } /* bigDestr */



/**
 *  Computes an integer division of big1 by big2 for signed big
 *  integers.
 */
#ifdef ANSI_C

biginttype bigDiv (const const_biginttype big1, const const_biginttype big2)
#else

biginttype bigDiv (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    biginttype result;

  /* bigDiv */
    if (mpz_sgn(big2) == 0) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else {
      result = malloc(sizeof(__mpz_struct));
      mpz_init(result);
      mpz_tdiv_q(result, big1, big2);
    } /* if */
    return(result);
  } /* bigDiv */



#ifdef ANSI_C

booltype bigEq (const const_biginttype big1, const const_biginttype big2)
#else

booltype bigEq (big1, big2)
biginttype big1;
biginttype big2;
#endif

  { /* bigEq */
    return(mpz_cmp(big1, big2) == 0);
  } /* bigEq */



#ifdef ANSI_C

biginttype bigFromInt32 (int32type number)
#else

biginttype bigFromInt32 (number)
int32type number;
#endif

  {
    biginttype result;

  /* bigFromInt32 */
    result = malloc(sizeof(__mpz_struct));
    mpz_init_set_si(result, number);
    return(result);
  } /* bigFromInt32 */



#ifdef INT64TYPE
#ifdef ANSI_C

biginttype bigFromInt64 (int64type number)
#else

biginttype bigFromInt64 (number)
int64type number;
#endif

  {
    mpz_t help;
    biginttype result;

  /* bigFromInt64 */
    result = malloc(sizeof(__mpz_struct));
    mpz_init_set_si(result, number >> 32);
    mpz_mul_2exp(result, result, 32);
    mpz_init_set_ui(help, number);
    mpz_ior(result, result, help);
    return(result);
  } /* bigFromInt64 */
#endif



#ifdef ANSI_C

biginttype bigFromUInt32 (uint32type number)
#else

biginttype bigFromUInt32 (number)
uint32type number;
#endif

  {
    biginttype result;

  /* bigFromUInt32 */
    result = malloc(sizeof(__mpz_struct));
    mpz_init_set_ui(result, number);
    return(result);
  } /* bigFromUInt32 */



#ifdef INT64TYPE
#ifdef ANSI_C

biginttype bigFromUInt64 (uint64type number)
#else

biginttype bigFromUInt64 (number)
uint64type number;
#endif

  {
    mpz_t help;
    biginttype result;

  /* bigFromUInt64 */
    result = malloc(sizeof(__mpz_struct));
    mpz_init_set_ui(result, number >> 32);
    mpz_mul_2exp(result, result, 32);
    mpz_init_set_ui(help, number);
    mpz_ior(result, result, help);
    return(result);
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
    biginttype result;

  /* bigGcd */
    result = malloc(sizeof(__mpz_struct));
    mpz_init(result);
    mpz_gcd(result, big1, big2);
    return(result);
  } /* bigGcd */



/**
 *  Adds big2 to *big_variable.
 */
#ifdef ANSI_C

void bigGrow (biginttype *const big_variable, const const_biginttype big2)
#else

void bigGrow (big_variable, big2)
biginttype *big_variable;
biginttype big2;
#endif

  { /* bigGrow */
    mpz_add(*big_variable, *big_variable, big2);
  } /* bigGrow */



#ifdef ANSI_C

inttype bigHashCode (const const_biginttype big1)
#else

inttype bigHashCode (big1)
biginttype big1;
#endif

  {
    size_t count;
    inttype result;

  /* bigHashCode */
    count = mpz_size(big1);
    if (count == 0) {
      result = 0;
    } /* if */
      result = mpz_getlimbn(big1, 0) << 5 ^ count << 3 ^ mpz_getlimbn(big1, count - 1);
    return(result);
  } /* bigHashCode */



#ifdef ANSI_C

biginttype bigImport (ustritype buffer)
#else

biginttype bigImport (buffer)
ustritype buffer;
#endif

  {
    size_t count;
    size_t pos;
    ustritype negated_buffer;
    unsigned short carry;
    biginttype result;

  /* bigImport */
    result = malloc(sizeof(__mpz_struct));
    mpz_init(result);
    count = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
    if (buffer[4] >= 128) {
      negated_buffer = malloc(count);
      carry = 1;
      pos = count;
      while (pos > 0) {
        pos--;
        carry += ~buffer[4 + pos] & 0xFF;
        negated_buffer[pos] = (uchartype) (carry & 0xFF);
        carry >>= 8;
      } /* for */
      mpz_import(result, count, 1, 1, 0, 0, negated_buffer);
      free(negated_buffer);
      mpz_neg(result, result);
    } else {
      mpz_import(result, count, 1, 1, 0, 0, &buffer[4]);
    } /* if */
    return(result);
  } /* bigImport */



#ifdef ANSI_C

void bigIncr (biginttype *const big_variable)
#else

void bigIncr (big_variable)
biginttype *big_variable;
#endif

  { /* bigIncr */
    mpz_add_ui(*big_variable, *big_variable, 1);
  } /* bigIncr */



/**
 *  Computes base to the power of exponent for signed big integers.
 */
#ifdef ANSI_C

biginttype bigIPow (const const_biginttype base, inttype exponent)
#else

biginttype bigIPow (base, exponent)
biginttype base;
inttype exponent;
#endif

  {
    biginttype result;

  /* bigIPow */
    if (exponent < 0) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else {
      result = malloc(sizeof(__mpz_struct));
      mpz_init(result);
      mpz_pow_ui(result, base, exponent);
    } /* if */
    return(result);
  } /* bigIPow */



#ifdef ANSI_C

biginttype bigLog2 (const const_biginttype big1)
#else

biginttype bigLog2 (big1)
biginttype big1;
#endif

  {
    int sign;
    biginttype result;

  /* bigLog2 */
    sign = mpz_sgn(big1);
    if (sign < 0) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else if (sign == 0) {
      result = malloc(sizeof(__mpz_struct));
      mpz_init_set_si(result, -1);
    } else {
      result = malloc(sizeof(__mpz_struct));
      mpz_init_set_ui(result, mpz_sizeinbase(big1, 2) - 1);
    } /* if */
    return(result);
  } /* bigLog2 */



#ifdef ANSI_C

inttype bigLowestSetBit (const const_biginttype big1)
#else

inttype bigLowestSetBit (big1)
biginttype big1;
#endif

  { /* bigLowestSetBit */
    return((inttype) mpz_scan1(big1, 0));
  } /* bigLowestSetBit */



#ifdef ANSI_C

biginttype bigLShift (const const_biginttype big1, const inttype lshift)
#else

biginttype bigLShift (big1, lshift)
biginttype big1;
inttype rshift;
#endif

  {
    biginttype result;

  /* bigLShift */
    if (lshift < 0) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else {
      result = malloc(sizeof(__mpz_struct));
      mpz_init(result);
      mpz_mul_2exp(result, big1, lshift);
    } /* if */
    return(result);
  } /* bigLShift */



#ifdef ANSI_C

void bigLShiftAssign (biginttype *const big_variable, inttype lshift)
#else

void bigLShiftAssign (big_variable, rshift)
biginttype *const big_variable;
inttype lshift;
#endif

  { /* bigLShiftAssign */
    if (lshift < 0) {
      raise_error(NUMERIC_ERROR);
    } else if (lshift != 0) {
      mpz_mul_2exp(*big_variable, *big_variable, lshift);
    } /* if */
  } /* bigLShiftAssign */



#ifdef ANSI_C

biginttype bigLShiftOne (const inttype lshift)
#else

biginttype bigLShiftOne (lshift)
inttype rshift;
#endif

  {
    mpz_t one;
    biginttype result;

  /* bigLShiftOne */
    if (lshift < 0) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else {
      result = malloc(sizeof(__mpz_struct));
      mpz_init(result);
      mpz_init_set_ui(one, 1);
      mpz_mul_2exp(result, one, lshift);
    } /* if */
    return(result);
  } /* bigLShiftOne */



/**
 *  Computes an integer modulo division of big1 by big2 for signed
 *  big integers.
 */
#ifdef ANSI_C

biginttype bigMDiv (const const_biginttype big1, const const_biginttype big2)
#else

biginttype bigMDiv (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    biginttype result;

  /* bigMDiv */
    if (mpz_sgn(big2) == 0) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else {
      result = malloc(sizeof(__mpz_struct));
      mpz_init(result);
      mpz_fdiv_q(result, big1, big2);
    } /* if */
    return(result);
  } /* bigMDiv */



#ifdef ANSI_C

biginttype bigMinus (const const_biginttype big1)
#else

biginttype bigMinus (big1)
biginttype big1;
#endif

  {
    biginttype result;

  /* bigMinus */
    result = malloc(sizeof(__mpz_struct));
    mpz_init(result);
    mpz_neg(result, big1);
    return(result);
  } /* bigMinus */



/**
 *  Computes the modulo of an integer division of big1 by big2
 *  for signed big integers.
 */
#ifdef ANSI_C

biginttype bigMod (const const_biginttype big1, const const_biginttype big2)
#else

biginttype bigMod (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    biginttype result;

  /* bigMod */
    if (mpz_sgn(big2) == 0) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else {
      result = malloc(sizeof(__mpz_struct));
      mpz_init(result);
      mpz_fdiv_r(result, big1, big2);
    } /* if */
    return(result);
  } /* bigMod */



/**
 *  Returns the product of two signed big integers.
 */
#ifdef ANSI_C

biginttype bigMult (const const_biginttype big1, const const_biginttype big2)
#else

biginttype bigMult (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    biginttype result;

  /* bigMult */
    result = malloc(sizeof(__mpz_struct));
    mpz_init(result);
    mpz_mul(result, big1, big2);
    return(result);
  } /* bigMult */



#ifdef ANSI_C

void bigMultAssign (biginttype *const big_variable, const_biginttype big2)
#else

void bigMultAssign (big_variable, big2)
biginttype *big_variable;
biginttype big2;
#endif

  { /* bigMultAssign */
    mpz_mul(*big_variable, *big_variable, big2);
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
    result = malloc(sizeof(__mpz_struct));
    mpz_init(result);
    mpz_mul_si(result, big1, number);
    return(result);
  } /* bigMultSignedDigit */



#ifdef ANSI_C

booltype bigOdd (const const_biginttype big1)
#else

booltype bigOdd (big1)
biginttype big1;
#endif

  { /* bigOdd */
    return(mpz_odd_p(big1));
  } /* bigOdd */



#ifdef ANSI_C

biginttype bigParse (const const_stritype stri)
#else

biginttype bigParse (stri)
stritype stri;
#endif

  {
    cstritype cstri;
    biginttype result;

  /* bigParse */
    cstri = cp_to_cstri(stri);
    if (cstri == NULL) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result = malloc(sizeof(__mpz_struct));
      if (mpz_init_set_str(result, cstri, 10) != 0) {
        free_cstri(cstri, stri);
        mpz_clear(result);
        free(result);
        raise_error(RANGE_ERROR);
        result = NULL;
      } else {
        free_cstri(cstri, stri);
      } /* if */
    } /* if */
    return(result);
  } /* bigParse */



#ifdef ANSI_C

biginttype bigPred (const const_biginttype big1)
#else

biginttype bigPred (big1)
biginttype big1;
#endif

  {
    biginttype result;

  /* bigPred */
    result = malloc(sizeof(__mpz_struct));
    mpz_init(result);
    mpz_sub_ui(result, big1, 1);
    return(result);
  } /* bigPred */



#ifdef ANSI_C

biginttype bigPredTemp (biginttype big1)
#else

biginttype bigPredTemp (big1)
biginttype big1;
#endif

  { /* bigPredTemp */
    mpz_sub_ui(big1, big1, 1);
    return(big1);
  } /* bigPredTemp */



/**
 *  Computes a random number between lower_limit and upper_limit
 *  for signed big integers.
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
    static booltype seed_necessary = TRUE;
    static gmp_randstate_t state;
    mpz_t range_limit;
    biginttype result;

  /* bigRand */
    mpz_init(range_limit);
    mpz_sub(range_limit, upper_limit, lower_limit);
    if (mpz_sgn(range_limit) < 0) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      mpz_add_ui(range_limit, range_limit, 1);
      result = malloc(sizeof(__mpz_struct));
      mpz_init(result);
      if (seed_necessary) {
        gmp_randinit_default(state);
        seed_necessary = FALSE;
      } /* if */
      mpz_urandomm(result, state, range_limit);
      mpz_add(result, result, lower_limit);
      return(result);
    } /* if */
  } /* bigRand */



/**
 *  Computes the remainder of an integer division of big1 by big2
 *  for signed big integers.
 */
#ifdef ANSI_C

biginttype bigRem (const const_biginttype big1, const const_biginttype big2)
#else

biginttype bigRem (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    biginttype result;

  /* bigRem */
    if (mpz_sgn(big2) == 0) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else {
      result = malloc(sizeof(__mpz_struct));
      mpz_init(result);
      mpz_tdiv_r(result, big1, big2);
    } /* if */
    return(result);
  } /* bigRem */



#ifdef ANSI_C

biginttype bigRShift (const const_biginttype big1, const inttype rshift)
#else

biginttype bigRShift (big1, rshift)
biginttype big1;
inttype rshift;
#endif

  {
    biginttype result;

  /* bigRShift */
    if (rshift < 0) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else {
      result = malloc(sizeof(__mpz_struct));
      mpz_init(result);
      mpz_fdiv_q_2exp(result, big1, rshift);
    } /* if */
    return(result);
  } /* bigRShift */



#ifdef ANSI_C

void bigRShiftAssign (biginttype *const big_variable, inttype rshift)
#else

void bigRShiftAssign (big_variable, rshift)
biginttype *const big_variable;
inttype rshift;
#endif

  { /* bigRShiftAssign */
    if (rshift < 0) {
      raise_error(NUMERIC_ERROR);
    } else {
      mpz_fdiv_q_2exp(*big_variable, *big_variable, rshift);
    } /* if */
  } /* bigRShiftAssign */



/**
 *  Returns the difference of two signed big integers.
 */
#ifdef ANSI_C

biginttype bigSbtr (const const_biginttype big1, const const_biginttype big2)
#else

biginttype bigSbtr (big1, big2)
biginttype big1;
biginttype big2;
#endif

  {
    biginttype result;

  /* bigSbtr */
    result = malloc(sizeof(__mpz_struct));
    mpz_init(result);
    mpz_sub(result, big1, big2);
    return(result);
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
    mpz_sub(big1, big1, big2);
    return(big1);
  } /* bigSbtrTemp */



/**
 *  Subtracts big2 from *big_variable.
 */
#ifdef ANSI_C

void bigShrink (biginttype *const big_variable, const const_biginttype big2)
#else

void bigShrink (big_variable, big2)
biginttype *big_variable;
biginttype big2;
#endif

  { /* bigShrink */
    mpz_sub(*big_variable, *big_variable, big2);
  } /* bigShrink */



#ifdef ANSI_C

stritype bigStr (const const_biginttype big1)
#else

stritype bigStr (big1)
biginttype big1;
#endif

  {
    char *cstri;
    stritype result;

  /* bigStr */
    cstri = mpz_get_str(NULL, 10, big1);
    result = cstri_to_stri(cstri);
    free(cstri);
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return(result);
  } /* bigStr */



#ifdef ANSI_C

biginttype bigSucc (const const_biginttype big1)
#else

biginttype bigSucc (big1)
biginttype big1;
#endif

  {
    biginttype result;

  /* bigSucc */
    result = malloc(sizeof(__mpz_struct));
    mpz_init(result);
    mpz_add_ui(result, big1, 1);
    return(result);
  } /* bigSucc */



#ifdef ANSI_C

biginttype bigSuccTemp (biginttype big1)
#else

biginttype bigSuccTemp (big1)
biginttype big1;
#endif

  { /* bigSuccTemp */
    mpz_add_ui(big1, big1, 1);
    return(big1);
  } /* bigSuccTemp */



#ifdef ANSI_C

bstritype bigToBStri (const_biginttype big1)
#else

bstritype bigToBStri (big1)
biginttype big1;
#endif

  {
    size_t count;
    size_t export_count;
    size_t pos;
    int sign;
    unsigned short carry;
    ustritype buffer;
    memsizetype charIndex;
    memsizetype result_size;
    bstritype result;

  /* bigToBStri */
    count = (mpz_sizeinbase(big1, 2) + 7) / 8;
    buffer = malloc(count);
    if (buffer == NULL) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      mpz_export(buffer, &export_count, 1, 1, 0, 0, big1);
      sign = mpz_sgn(big1);
      if (sign < 0) {
        carry = 1;
        pos = export_count;
        while (pos > 0) {
          pos--;
          carry += ~buffer[pos] & 0xFF;
          buffer[pos] = carry & 0xFF;
          carry >>= 8;
        } /* while */
      } /* if */
      result_size = count;
      if ((sign > 0 && buffer[0] >= 128) ||
          (sign < 0 && buffer[0] <= 127)) {
        result_size++;
      } /* if */
      if (!ALLOC_BSTRI(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        if (sign == 0) {
          result->mem[0] = 0;
        } else {
          charIndex = 0;
          if (sign < 0) {
            if (buffer[0] <= 127) {
              result->mem[charIndex] = 255;
              charIndex++;
            } /* if */
          } else {
            if (buffer[0] >= 128) {
              result->mem[charIndex] = 0;
              charIndex++;
            } /* if */
          } /* for */
          for (pos = 0; pos < export_count; pos++) {
            result->mem[charIndex] = buffer[pos];
            charIndex++;
          } /* for */
        } /* if */
      } /* if */
      free(buffer);
    } /* if */
    return(result);
  } /* bigToBStri */



#ifdef ANSI_C

int32type bigToInt32 (const const_biginttype big1)
#else

int32type bigToInt32 (big1)
biginttype big1;
#endif

  { /* bigToInt32 */
    if (!mpz_fits_slong_p(big1)) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      return(mpz_get_si(big1));
    } /* if */
  } /* bigToInt32 */



#ifdef INT64TYPE
#ifdef ANSI_C

int64type bigToInt64 (const const_biginttype big1)
#else

int64type bigToInt64 (big1)
biginttype big1;
#endif

  { /* bigToInt64 */
    if (!mpz_fits_slong_p(big1)) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      return(mpz_get_si(big1));
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
    result = malloc(sizeof(__mpz_struct));
    mpz_init_set_ui(result, 0);
    return(result);
  } /* bigZero */
