/********************************************************************/
/*                                                                  */
/*  big_gmp.c     Functions for bigInteger using the gmp library.   */
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
/*  File: seed7/src/big_gmp.c                                       */
/*  Changes: 2008, 2009, 2010, 2013, 2014  Thomas Mertes            */
/*  Content: Functions for bigInteger using the gmp library.        */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "gmp.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "int_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "big_drv.h"


#define HEAP_ALLOC_BIG     (bigIntType) malloc(sizeof(__mpz_struct))
#define HEAP_FREE_BIG(var) free(var)

#ifdef WITH_BIGINT_FREELIST

static freeListElemType flist = NULL;
static unsigned int flist_allowed = 100;

#define POP_BIG_OK      flist != NULL
#define PUSH_BIG_OK     flist_allowed > 0

#define POP_BIG(var)    {var = (bigIntType) flist; flist = flist->next; flist_allowed++; }
#define PUSH_BIG(var)   {((freeListElemType) var)->next = flist; flist = (freeListElemType) var; flist_allowed--; }

#define ALLOC_BIG(var)  if (POP_BIG_OK) POP_BIG(var) else var = HEAP_ALLOC_BIG;
#define FREE_BIG(var)   if (PUSH_BIG_OK) PUSH_BIG(var) else HEAP_FREE_BIG(var);

#else

#define ALLOC_BIG(var)  var = HEAP_ALLOC_BIG
#define FREE_BIG(var)   HEAP_FREE_BIG(var)

#endif



cstriType bigHexCStri (const const_bigIntType big1)

  {
    size_t count;
    size_t export_count;
    size_t pos;
    int sign;
    unsigned int carry;
    const_cstriType stri_ptr;
    ustriType buffer;
    memSizeType charIndex;
    size_t result_size;
    cstriType result;

  /* bigHexCStri */
    if (big1 != NULL) {
      count = (mpz_sizeinbase(big1, 2) + 7) / 8;
      buffer = (ustriType) malloc(count);
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
            buffer[pos] = (ucharType) (carry & 0xFF);
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
            strcpy(result, "16#00");
          } else {
            strcpy(result, "16#");
            charIndex = 3;
            if (sign < 0) {
              if (buffer[0] <= 127) {
                strcpy(&result[charIndex], "ff");
                charIndex += 2;
              } /* if */
            } else {
              if (buffer[0] >= 128) {
                strcpy(&result[charIndex], "00");
                charIndex += 2;
              } /* if */
            } /* for */
            for (pos = 0; pos < export_count; pos++) {
              sprintf(&result[charIndex], "%02x", buffer[pos]);
              charIndex += 2;
            } /* for */
          } /* if */
        } /* if */
        free(buffer);
      } /* if */
    } else {
      stri_ptr = " *NULL_BIGINT* ";
      if (!ALLOC_CSTRI(result, strlen(stri_ptr))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        strcpy(result, stri_ptr);
      } /* if */
    } /* if */
    return result;
  } /* bigHexCStri */



/**
 *  Compute the absolute value of a 'bigInteger' number.
 *  @return the absolute value.
 */
bigIntType bigAbs (const const_bigIntType big1)

  {
    bigIntType result;

  /* bigAbs */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_abs(result, big1);
    return result;
  } /* bigAbs */



/**
 *  Add two 'bigInteger' numbers.
 *  @return the sum of the two numbers.
 */
bigIntType bigAdd (const_bigIntType summand1, const_bigIntType summand2)

  {
    bigIntType sum;

  /* bigAdd */
    ALLOC_BIG(sum);
    mpz_init(sum);
    mpz_add(sum, summand1, summand2);
    return sum;
  } /* bigAdd */



/**
 *  Add two 'bigInteger' numbers.
 *  Summand1 is assumed to be a temporary value which is reused.
 *  @return the sum of the two numbers in 'summand1'.
 */
bigIntType bigAddTemp (bigIntType summand1, const const_bigIntType summand2)

  { /* bigAddTemp */
    mpz_add(summand1, summand1, summand2);
    return summand1;
  } /* bigAddTemp */



bigIntType bigAnd (const_bigIntType big1, const_bigIntType big2)

  {
    bigIntType result;

  /* bigAnd */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_and(result, big1, big2);
    return result;
  } /* bigAnd */



/**
 *  Number of bits in the minimal two's-complement representation.
 *  The high bits equivalent to the sign bit are not part of the
 *  minimal two's-complement representation.
 *  @return the number of bits.
 *  @exception RANGE_ERROR The result does not fit into an integer.
 */
intType bigBitLength (const const_bigIntType big1)

  {
    int sign;
    mpz_t help;
    intType result;

  /* bigBitLength */
    sign = mpz_sgn(big1);
    if (sign < 0) {
      if (mpz_cmp_si(big1, -1) == 0) {
        result = 0;
      } else {
        mpz_init(help);
        mpz_add_ui(help, big1, 1);
        result = (intType) mpz_sizeinbase(help, 2);
        mpz_clear(help);
      } /* if */
    } else if (sign == 0) {
      result = 0;
    } else {
      result = (intType) mpz_sizeinbase(big1, 2);
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
    intType result;

  /* bigCmp */
    result = mpz_cmp(big1, big2);
    if (result < 0) {
      result = -1;
    } else if (result > 0) {
      result = 1;
    } /* if */
    return result;
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
 *         a long int.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType bigCmpSignedDigit (const const_bigIntType big1, intType number)

  {
    intType result;

  /* bigCmpSignedDigit */
    result = mpz_cmp_si(big1, number);
    if (result < 0) {
      result = -1;
    } else if (result > 0) {
      result = 1;
    } /* if */
    return result;
  } /* bigCmpSignedDigit */



void bigCpy (bigIntType *const big_to, const const_bigIntType big_from)

  { /* bigCpy */
    mpz_set(*big_to, big_from);
  } /* bigCpy */



void bigCpyGeneric (genericType *const dest, const genericType source)

  { /* bigCpyGeneric */
    bigCpy(&((rtlObjectType *) dest)->value.bigIntValue,
           ((const_rtlObjectType *) &source)->value.bigIntValue);
  } /* bigCpyGeneric */



bigIntType bigCreate (const const_bigIntType big_from)

  {
    bigIntType result;

  /* bigCreate */
    ALLOC_BIG(result);
    mpz_init_set(result, big_from);
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
 *  Decrements *big_variable by 1.
 */
void bigDecr (bigIntType *const big_variable)

  { /* bigDecr */
    mpz_sub_ui(*big_variable, *big_variable, 1);
  } /* bigDecr */



void bigDestr (const const_bigIntType old_bigint)

  { /* bigDestr */
    if (old_bigint != NULL) {
      mpz_clear((bigIntType) old_bigint);
      FREE_BIG((bigIntType) old_bigint);
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
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
bigIntType bigDiv (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    bigIntType quotient;

  /* bigDiv */
    if (mpz_sgn(divisor) == 0) {
      raise_error(NUMERIC_ERROR);
      quotient = NULL;
    } else {
      ALLOC_BIG(quotient);
      mpz_init(quotient);
      mpz_tdiv_q(quotient, dividend, divisor);
    } /* if */
    return quotient;
  } /* bigDiv */



/**
 *  Check if two 'bigInteger' numbers are equal.
 *  @return TRUE if both numbers are equal,
 *          FALSE otherwise.
 */
boolType bigEq (const const_bigIntType big1, const const_bigIntType big2)

  { /* bigEq */
    return mpz_cmp(big1, big2) == 0;
  } /* bigEq */



/**
 *  Check if 'big1' is equal to the bigdigit 'number'.
 *  The range of 'number' is restricted and it is the job of the
 *  compiler to assure that 'number' is within the allowed range.
 *  @param number Number that must be in the range of
 *         a long int.
 *  @return TRUE if 'big1' and 'number' are equal,
 *          FALSE otherwise.
 */
boolType bigEqSignedDigit (const const_bigIntType big1, intType number)

  { /* bigEqSignedDigit */
    return mpz_cmp_si(big1, number) == 0;
  } /* bigEqSignedDigit */



bigIntType bigFromByteBufferBe (const memSizeType size,
    const const_ustriType buffer, const boolType isSigned)

  {
    memSizeType pos;
    ustriType negated_buffer;
    unsigned int carry;
    bigIntType result;

  /* bigFromByteBufferBe */
    /* printf("bigFromByteBufferBe(%lu, *, %d)\n", size, isSigned); */
    ALLOC_BIG(result);
    mpz_init(result);
    if (isSigned && size != 0 && buffer[0] >= 128) {
      negated_buffer = (ustriType) malloc(size);
      carry = 1;
      pos = size;
      while (pos > 0) {
        pos--;
        carry += ~buffer[pos] & 0xFF;
        negated_buffer[pos] = (ucharType) (carry & 0xFF);
        carry >>= 8;
      } /* for */
      mpz_import(result, (size_t) size, 1, 1, 0, 0, negated_buffer);
      free(negated_buffer);
      mpz_neg(result, result);
    } else {
      mpz_import(result, (size_t) size, 1, 1, 0, 0, buffer);
    } /* if */
    return result;
  } /* bigFromByteBufferBe */



bigIntType bigFromByteBufferLe (const memSizeType size,
    const const_ustriType buffer, const boolType isSigned)

  {
    memSizeType pos;
    ustriType negated_buffer;
    unsigned int carry;
    bigIntType result;

  /* bigFromByteBufferLe */
    /* printf("bigFromByteBufferLe(%lu, *, %d)\n", size, isSigned); */
    ALLOC_BIG(result);
    mpz_init(result);
    if (isSigned && size != 0 && buffer[size - 1] >= 128) {
      negated_buffer = (ustriType) malloc(size);
      carry = 1;
      pos = 0;
      while (pos < size) {
        carry += ~buffer[pos] & 0xFF;
        negated_buffer[pos] = (ucharType) (carry & 0xFF);
        carry >>= 8;
        pos++;
      } /* for */
      mpz_import(result, (size_t) size, -1, 1, 0, 0, negated_buffer);
      free(negated_buffer);
      mpz_neg(result, result);
    } else {
      mpz_import(result, (size_t) size, -1, 1, 0, 0, buffer);
    } /* if */
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
 */
bigIntType bigFromInt32 (int32Type number)

  {
    bigIntType result;

  /* bigFromInt32 */
    ALLOC_BIG(result);
    mpz_init_set_si(result, number);
    return result;
  } /* bigFromInt32 */



#ifdef INT64TYPE
/**
 *  Convert an int64Type number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 */
bigIntType bigFromInt64 (int64Type number)

  {
    bigIntType result;

  /* bigFromInt64 */
    ALLOC_BIG(result);
#if LONG_SIZE == 64
    mpz_init_set_si(result, number);
#else
    {
      mpz_t help;

      mpz_init_set_si(result, (int32Type) ((number >> 32) & 0xFFFFFFFF));
      mpz_mul_2exp(result, result, 32);
      mpz_init_set_ui(help, (uint32Type) (number & 0xFFFFFFFF));
      mpz_ior(result, result, help);
      mpz_clear(help);
    }
#endif
    return result;
  } /* bigFromInt64 */
#endif



/**
 *  Convert an uint32Type number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 */
bigIntType bigFromUInt32 (uint32Type number)

  {
    bigIntType result;

  /* bigFromUInt32 */
    ALLOC_BIG(result);
    mpz_init_set_ui(result, number);
    return result;
  } /* bigFromUInt32 */



#ifdef INT64TYPE
/**
 *  Convert an uint64Type number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 */
bigIntType bigFromUInt64 (uint64Type number)

  {
    mpz_t help;
    bigIntType result;

  /* bigFromUInt64 */
    ALLOC_BIG(result);
    mpz_init_set_ui(result, (uint32Type) ((number >> 32) && 0xFFFFFFFF));
    mpz_mul_2exp(result, result, 32);
    mpz_init_set_ui(help, (uint32Type) (number && 0xFFFFFFFF));
    mpz_ior(result, result, help);
    mpz_clear(help);
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
    bigIntType result;

  /* bigGcd */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_gcd(result, big1, big2);
    return result;
  } /* bigGcd */



/**
 *  Increment a 'bigInteger' variable by a delta.
 *  Adds delta to *big_variable.
 *  @param delta The delta to be added to *big_variable.
 */
void bigGrow (bigIntType *const big_variable, const const_bigIntType delta)

  { /* bigGrow */
    mpz_add(*big_variable, *big_variable, delta);
  } /* bigGrow */



/**
 *  Increment a 'bigInteger' variable by a delta.
 *  Adds delta to *big_variable.
 *  @param delta The delta to be added to *big_variable.
 *         Delta must be in the range of a long int.
 */
void bigGrowSignedDigit (bigIntType *const big_variable, const intType delta)

  { /* bigGrowSignedDigit */
    if (delta < 0) {
      mpz_sub_ui(*big_variable, *big_variable, (unsigned long int) -delta);
    } else {
      mpz_add_ui(*big_variable, *big_variable, (unsigned long int) delta);
    } /* if */
  } /* bigGrowSignedDigit */



/**
 *  Compute the hash value of a 'bigInteger' number.
 *  @return the hash value.
 */
intType bigHashCode (const const_bigIntType big1)

  {
    size_t count;
    intType result;

  /* bigHashCode */
    count = mpz_size(big1);
    if (count == 0) {
      result = 0;
    } /* if */
    result = (intType) (mpz_getlimbn(big1, 0) << 5 ^ count << 3 ^ mpz_getlimbn(big1, count - 1));
    return result;
  } /* bigHashCode */



/**
 *  Increment a 'bigInteger' variable.
 *  Increments *big_variable by 1.
 */
void bigIncr (bigIntType *const big_variable)

  { /* bigIncr */
    mpz_add_ui(*big_variable, *big_variable, 1);
  } /* bigIncr */



/**
 *  Compute the exponentiation of a 'bigInteger' base with an integer exponent.
 *  @return the result of the exponentation.
 *  @exception NUMERIC_ERROR When the exponent is negative.
 */
bigIntType bigIPow (const const_bigIntType base, intType exponent)

  {
    bigIntType power;

  /* bigIPow */
    if (unlikely(exponent < 0)) {
      raise_error(NUMERIC_ERROR);
      power = NULL;
    } else {
      ALLOC_BIG(power);
      mpz_init(power);
      mpz_pow_ui(power, base, (uintType) exponent);
    } /* if */
    return power;
  } /* bigIPow */



/**
 *  Compute the exponentiation of a bigdigit base with an integer exponent.
 *  @param base Base that must be in the range of a long int.
 *  @return the result of the exponentation.
 *  @exception NUMERIC_ERROR When the exponent is negative.
 */
bigIntType bigIPowSignedDigit (intType base, intType exponent)

  {
    bigIntType power;

  /* bigIPowSignedDigit */
    if (unlikely(exponent < 0)) {
      raise_error(NUMERIC_ERROR);
      power = NULL;
    } else {
      ALLOC_BIG(power);
      mpz_init_set_si(power, base);
      mpz_pow_ui(power, power, (uintType) exponent);
    } /* if */
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
    int sign;
    char *cstri;
    bigIntType logarithm;

  /* bigLog10 */
    sign = mpz_sgn(big1);
    if (sign < 0) {
      raise_error(NUMERIC_ERROR);
      logarithm = NULL;
    } else if (sign == 0) {
      ALLOC_BIG(logarithm);
      mpz_init_set_si(logarithm, -1);
    } else {
      cstri = mpz_get_str(NULL, 10, big1);
      ALLOC_BIG(logarithm);
      mpz_init_set_ui(logarithm, strlen(cstri) - 1);
      free(cstri);
    } /* if */
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
    int sign;
    bigIntType logarithm;

  /* bigLog2 */
    sign = mpz_sgn(big1);
    if (sign < 0) {
      raise_error(NUMERIC_ERROR);
      logarithm = NULL;
    } else if (sign == 0) {
      ALLOC_BIG(logarithm);
      mpz_init_set_si(logarithm, -1);
    } else {
      ALLOC_BIG(logarithm);
      mpz_init_set_ui(logarithm, mpz_sizeinbase(big1, 2) - 1);
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
    bigIntType result;

  /* bigLowerBits */
    if (unlikely(bits < 0)) {
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else {
      ALLOC_BIG(result);
      mpz_init(result);
      mpz_fdiv_r_2exp(result, big1, (uintType) bits);
    } /* if */
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

  { /* bigLowerBitsTemp */
    if (unlikely(bits < 0)) {
      FREE_BIG(big1);
      raise_error(NUMERIC_ERROR);
      return NULL;
    } else {
      mpz_fdiv_r_2exp(big1, big1, (uintType) bits);
    } /* if */
    return big1;
  } /* bigLowerBitsTemp */



/**
 *  Index of the lowest-order one bit.
 *  For A <> 0 this is equal to the number of lowest-order zero bits.
 *  @return the number of lowest-order zero bits or -1 for lowestSetBit(0).
 */
intType bigLowestSetBit (const const_bigIntType big1)

  { /* bigLowestSetBit */
    return (intType) mpz_scan1(big1, 0);
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
    bigIntType result;

  /* bigLShift */
    ALLOC_BIG(result);
    mpz_init(result);
    if (lshift < 0) {
      mpz_fdiv_q_2exp(result, big1, (uintType) -lshift);
    } else {
      mpz_mul_2exp(result, big1, (uintType) lshift);
    } /* if */
    return result;
  } /* bigLShift */



/**
 *  Shift a number left by lshift bits and assign the result back to number.
 *  When lshift is negative a right shift is done instead.
 */
void bigLShiftAssign (bigIntType *const big_variable, intType lshift)

  { /* bigLShiftAssign */
    if (lshift < 0) {
      mpz_fdiv_q_2exp(*big_variable, *big_variable, (uintType) -lshift);
    } else if (lshift != 0) {
      mpz_mul_2exp(*big_variable, *big_variable, (uintType) lshift);
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
    mpz_t one;
    bigIntType result;

  /* bigLShiftOne */
    ALLOC_BIG(result);
    if (lshift < 0) {
      mpz_init_set_ui(result, 0);
    } else {
      mpz_init(result);
      mpz_init_set_ui(one, 1);
      mpz_mul_2exp(result, one, (uintType) lshift);
      mpz_clear(one);
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
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
bigIntType bigMDiv (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    bigIntType quotient;

  /* bigMDiv */
    if (mpz_sgn(divisor) == 0) {
      raise_error(NUMERIC_ERROR);
      quotient = NULL;
    } else {
      ALLOC_BIG(quotient);
      mpz_init(quotient);
      mpz_fdiv_q(quotient, dividend, divisor);
    } /* if */
    return quotient;
  } /* bigMDiv */



/**
 *  Compute the modulo (remainder) of the integer division bigMDiv.
 *  The modulo has the same sign as the divisor.
 *  @return the modulo of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
bigIntType bigMod (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    bigIntType modulo;

  /* bigMod */
    if (mpz_sgn(divisor) == 0) {
      raise_error(NUMERIC_ERROR);
      modulo = NULL;
    } else {
      ALLOC_BIG(modulo);
      mpz_init(modulo);
      mpz_fdiv_r(modulo, dividend, divisor);
    } /* if */
    return modulo;
  } /* bigMod */



/**
 *  Multiply two 'bigInteger' numbers.
 *  @return the product of the two numbers.
 */
bigIntType bigMult (const const_bigIntType factor1, const const_bigIntType factor2)

  {
    bigIntType product;

  /* bigMult */
    ALLOC_BIG(product);
    mpz_init(product);
    mpz_mul(product, factor1, factor2);
    return product;
  } /* bigMult */



/**
 *  Multiply a 'bigInteger' number by a factor and assign the result back to number.
 */
void bigMultAssign (bigIntType *const big_variable, const_bigIntType factor)

  { /* bigMultAssign */
    mpz_mul(*big_variable, *big_variable, factor);
  } /* bigMultAssign */



/**
 *  Multiply factor1 with the bigdigit factor2.
 *  The range of factor2 is restricted and it is the job of the
 *  compiler to assure that factor2 is within the allowed range.
 *  @param factor2 Multiplication factor that must be
 *         in the range of a long int.
 *  @return the product of factor1 * factor2.
 */
bigIntType bigMultSignedDigit (const_bigIntType factor1, intType factor2)

  {
    bigIntType product;

  /* bigMultSignedDigit */
    ALLOC_BIG(product);
    mpz_init(product);
    mpz_mul_si(product, factor1, factor2);
    return product;
  } /* bigMultSignedDigit */



/**
 *  Minus sign, negate a 'bigInteger' number.
 *  @return the negated value of the number.
 */
bigIntType bigNegate (const const_bigIntType big1)

  {
    bigIntType result;

  /* bigNegate */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_neg(result, big1);
    return result;
  } /* bigNegate */



/**
 *  Minus sign, negate a 'bigInteger' number.
 *  Big1 is assumed to be a temporary value which is reused.
 *  @return the negated value of the number.
 */
bigIntType bigNegateTemp (bigIntType big1)

  { /* bigNegateTemp */
    mpz_neg(big1, big1);
    return big1;
  } /* bigNegateTemp */



/**
 *  Determine if a 'bigInteger' number is odd.
 *  @return TRUE if the number is odd,
 *          FALSE otherwise.
 */
boolType bigOdd (const const_bigIntType big1)

  { /* bigOdd */
    return mpz_odd_p(big1);
  } /* bigOdd */



bigIntType bigOr (const_bigIntType big1, const_bigIntType big2)

  {
    bigIntType result;

  /* bigOr */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_ior(result, big1, big2);
    return result;
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
    cstriType cstri;
    int mpz_result;
    errInfoType err_info = OKAY_NO_ERROR;
    bigIntType result;

  /* bigParse */
    cstri = stri_to_cstri(stri, &err_info);
    if (unlikely(cstri == NULL)) {
      raise_error(err_info);
      result = NULL;
    } else {
      ALLOC_BIG(result);
      if (cstri[0] == '+' && cstri[1] != '-') {
        mpz_result = mpz_init_set_str(result, &cstri[1], 10);
      } else {
        mpz_result = mpz_init_set_str(result, cstri, 10);
      } /* if */
      free_cstri(cstri, stri);
      if (mpz_result != 0) {
        mpz_clear(result);
        FREE_BIG(result);
        raise_error(RANGE_ERROR);
        result = NULL;
      } /* if */
    } /* if */
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
 *  @return the 'bigInteger' result of the conversion.
 *  @exception RANGE_ERROR When base < 2 or base > 36 holds or
 *             the string is empty or it does not contain an integer
 *             literal with the specified base.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
bigIntType bigParseBased (const const_striType stri, intType base)

  {
    cstriType cstri;
    int mpz_result;
    errInfoType err_info = OKAY_NO_ERROR;
    bigIntType result;

  /* bigParseBased */
    if (unlikely(stri->size == 0 || base < 2 || base > 36)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      cstri = stri_to_cstri(stri, &err_info);
      if (unlikely(cstri == NULL)) {
        raise_error(err_info);
        result = NULL;
      } else {
        ALLOC_BIG(result);
        if (cstri[0] == '+' && cstri[1] != '-') {
          mpz_result = mpz_init_set_str(result, &cstri[1], (int) base);
        } else {
          mpz_result = mpz_init_set_str(result, cstri, (int) base);
        } /* if */
        free_cstri(cstri, stri);
        if (mpz_result != 0) {
          mpz_clear(result);
          FREE_BIG(result);
          raise_error(RANGE_ERROR);
          result = NULL;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* bigParseBased */



/**
 *  Predecessor of a 'bigInteger' number.
 *  pred(A) is equivalent to A-1 .
 *  @return big1 - 1 .
 */
bigIntType bigPred (const const_bigIntType big1)

  {
    bigIntType result;

  /* bigPred */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_sub_ui(result, big1, 1);
    return result;
  } /* bigPred */



/**
 *  Returns a signed big integer decremented by 1.
 *  Big1 is assumed to be a temporary value which is reused.
 */
bigIntType bigPredTemp (bigIntType big1)

  { /* bigPredTemp */
    mpz_sub_ui(big1, big1, 1);
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
    char *cstri;
    striType result;

  /* bigRadix */
    if (unlikely(base < 2 || base > 36)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (upperCase) {
        cstri = mpz_get_str(NULL, -(int) base, big1);
      } else {
        cstri = mpz_get_str(NULL, (int) base, big1);
      } /* if */
      result = cstri_to_stri(cstri);
      free(cstri);
      if (result == NULL) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* bigRadix */



/**
 *  Compute pseudo-random number in the range [low, high].
 *  The random values are uniform distributed.
 *  @return a random number such that low <= rand(low, high) and
 *          rand(low, high) <= high holds.
 *  @exception RANGE_ERROR The range is empty (low > high holds).
 */
bigIntType bigRand (const const_bigIntType low,
    const const_bigIntType high)

  {
    static boolType seed_necessary = TRUE;
    static gmp_randstate_t state;
    mpz_t range_limit;
    bigIntType result;

  /* bigRand */
    mpz_init(range_limit);
    mpz_sub(range_limit, high, low);
    if (mpz_sgn(range_limit) < 0) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      mpz_add_ui(range_limit, range_limit, 1);
      ALLOC_BIG(result);
      mpz_init(result);
      if (seed_necessary) {
        gmp_randinit_default(state);
        seed_necessary = FALSE;
      } /* if */
      mpz_urandomm(result, state, range_limit);
      mpz_add(result, result, low);
    } /* if */
    mpz_clear(range_limit);
    return result;
  } /* bigRand */



/**
 *  Compute the remainder of the integer division bigDiv.
 *  The remainder has the same sign as the dividend.
 *  @return the remainder of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
bigIntType bigRem (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    bigIntType remainder;

  /* bigRem */
    if (mpz_sgn(divisor) == 0) {
      raise_error(NUMERIC_ERROR);
      remainder = NULL;
    } else {
      ALLOC_BIG(remainder);
      mpz_init(remainder);
      mpz_tdiv_r(remainder, dividend, divisor);
    } /* if */
    return remainder;
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
    bigIntType result;

  /* bigRShift */
    ALLOC_BIG(result);
    mpz_init(result);
    if (rshift < 0) {
      mpz_mul_2exp(result, big1, (uintType) -rshift);
    } else {
      mpz_fdiv_q_2exp(result, big1, (uintType) rshift);
    } /* if */
    return result;
  } /* bigRShift */



/**
 *  Shift a number right by rshift bits and assign the result back to number.
 *  When rshift is negative a left shift is done instead.
 */
void bigRShiftAssign (bigIntType *const big_variable, intType rshift)

  { /* bigRShiftAssign */
    if (rshift < 0) {
      mpz_mul_2exp(*big_variable, *big_variable, (uintType) -rshift);
    } else if (rshift != 0) {
      mpz_fdiv_q_2exp(*big_variable, *big_variable, (uintType) rshift);
    } /* if */
  } /* bigRShiftAssign */



/**
 *  Compute the subtraction of two 'bigInteger' numbers.
 *  @return the difference of the two numbers.
 */
bigIntType bigSbtr (const const_bigIntType minuend, const const_bigIntType subtrahend)

  {
    bigIntType difference;

  /* bigSbtr */
    ALLOC_BIG(difference);
    mpz_init(difference);
    mpz_sub(difference, minuend, subtrahend);
    return difference;
  } /* bigSbtr */



/**
 *  Compute the subtraction of two 'bigInteger' numbers.
 *  Minuend is assumed to be a temporary value which is reused.
 *  @return the difference of the two numbers in 'minuend'.
 */
bigIntType bigSbtrTemp (bigIntType minuend, const_bigIntType subtrahend)

  { /* bigSbtrTemp */
    mpz_sub(minuend, minuend, subtrahend);
    return minuend;
  } /* bigSbtrTemp */



/**
 *  Decrement a 'bigInteger' variable by a delta.
 *  Subtracts delta from *big_variable.
 */
void bigShrink (bigIntType *const big_variable, const const_bigIntType delta)

  { /* bigShrink */
    mpz_sub(*big_variable, *big_variable, delta);
  } /* bigShrink */



/**
 *  Compute the square of a 'bigInteger'.
 *  This function is used by the compiler to optimize
 *  multiplication and exponentiation operations.
 *  @return the square of big1.
 */
bigIntType bigSquare (const_bigIntType big1)

  {
    bigIntType result;

  /* bigSquare */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_mul(result, big1, big1);
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
    char *cstri;
    striType result;

  /* bigStr */
    cstri = mpz_get_str(NULL, 10, big1);
    result = cstri_to_stri(cstri);
    free(cstri);
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
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
    bigIntType result;

  /* bigSucc */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_add_ui(result, big1, 1);
    return result;
  } /* bigSucc */



/**
 *  Successor of a 'bigInteger' number.
 *  Big1 is assumed to be a temporary value which is reused.
 *  @return big1 + 1 .
 */
bigIntType bigSuccTemp (bigIntType big1)

  { /* bigSuccTemp */
    mpz_add_ui(big1, big1, 1);
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
    size_t count;
    size_t export_count;
    size_t pos;
    int sign;
    unsigned int carry;
    ustriType buffer;
    memSizeType charIndex;
    memSizeType result_size;
    bstriType result;

  /* bigToBStriBe */
    count = (mpz_sizeinbase(big1, 2) + 7) / 8;
    buffer = (ustriType) malloc(count);
    if (buffer == NULL) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      mpz_export(buffer, &export_count, 1, 1, 0, 0, big1);
      sign = mpz_sgn(big1);
      if (isSigned) {
        if (sign < 0) {
          carry = 1;
          pos = export_count;
          while (pos > 0) {
            pos--;
            carry += ~buffer[pos] & 0xFF;
            buffer[pos] = (ucharType) (carry & 0xFF);
            carry >>= 8;
          } /* while */
        } /* if */
        result_size = count;
        if ((sign > 0 && buffer[0] >= 128) ||
            (sign < 0 && buffer[0] <= 127)) {
          result_size++;
        } /* if */
      } else {
        if (unlikely(sign < 0)) {
          raise_error(RANGE_ERROR);
          free(buffer);
          return NULL;
        } else {
          result_size = count;
        } /* if */
      } /* if */
      if (!ALLOC_BSTRI_CHECK_SIZE(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        if (sign == 0) {
          result->mem[0] = 0;
        } else {
          charIndex = 0;
          if (isSigned) {
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
            } /* if */
          } /* if */
          memcpy(&result->mem[charIndex], buffer, export_count);
        } /* if */
      } /* if */
      free(buffer);
    } /* if */
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
    size_t count;
    size_t export_count;
    size_t pos;
    int sign;
    unsigned int carry;
    ustriType buffer;
    memSizeType result_size;
    bstriType result;

  /* bigToBStriLe */
    count = (mpz_sizeinbase(big1, 2) + 7) / 8;
    buffer = (ustriType) malloc(count);
    if (buffer == NULL) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      mpz_export(buffer, &export_count, -1, 1, 0, 0, big1);
      sign = mpz_sgn(big1);
      if (isSigned) {
        if (sign < 0) {
          carry = 1;
          pos = 0;
          while (pos < export_count) {
            carry += ~buffer[pos] & 0xFF;
            buffer[pos] = (ucharType) (carry & 0xFF);
            carry >>= 8;
            pos++;
          } /* while */
        } /* if */
        result_size = count;
        if ((sign > 0 && buffer[export_count - 1] >= 128) ||
            (sign < 0 && buffer[export_count - 1] <= 127)) {
          result_size++;
        } /* if */
      } else {
        if (unlikely(sign < 0)) {
          raise_error(RANGE_ERROR);
          free(buffer);
          return NULL;
        } else {
          result_size = count;
        } /* if */
      } /* if */
      if (!ALLOC_BSTRI_CHECK_SIZE(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        if (sign == 0) {
          result->mem[0] = 0;
        } else {
          memcpy(result->mem, buffer, export_count);
          if (isSigned) {
            if (sign < 0) {
              if (buffer[export_count - 1] <= 127) {
                result->mem[export_count] = 255;
              } /* if */
            } else {
              if (buffer[export_count - 1] >= 128) {
                result->mem[export_count] = 0;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* if */
      free(buffer);
    } /* if */
    return result;
  } /* bigToBStriLe */



int16Type bigToInt16 (const const_bigIntType big1)

  {
    long int result;

  /* bigToInt16 */
    if (!mpz_fits_slong_p(big1)) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      result = mpz_get_si(big1);
#if LONG_SIZE > 16
      if (result < INT16TYPE_MIN || result > INT16TYPE_MAX) {
        raise_error(RANGE_ERROR);
        return 0;
      } /* if */
#endif
      return (int16Type) result;
    } /* if */
  } /* bigToInt16 */



int32Type bigToInt32 (const const_bigIntType big1)

  {
    long int result;

  /* bigToInt32 */
    if (!mpz_fits_slong_p(big1)) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      result = mpz_get_si(big1);
#if LONG_SIZE > 32
      if (result < INT32TYPE_MIN || result > INT32TYPE_MAX) {
        raise_error(RANGE_ERROR);
        return 0;
      } /* if */
#endif
      return (int32Type) result;
    } /* if */
  } /* bigToInt32 */



#ifdef INT64TYPE
int64Type bigToInt64 (const const_bigIntType big1)

  {
    int64Type result;

  /* bigToInt64 */
#if LONG_SIZE == 64
    if (!mpz_fits_slong_p(big1)) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = mpz_get_si(big1);
    } /* if */
#else
    {
      mpz_t help;

      mpz_init_set(help, big1);
      mpz_fdiv_q_2exp(help, help, 32);
      if (!mpz_fits_slong_p(help)) {
        raise_error(RANGE_ERROR);
        result = 0;
      } else {
        result = (int64Type) (mpz_get_si(help) & 0xFFFFFFFF) << 32 |
                 (int64Type) (mpz_get_si(big1) & 0xFFFFFFFF);
        mpz_clear(help);
      } /* if */
    }
#endif
    return result;
  } /* bigToInt64 */
#endif



bigIntType bigXor (const_bigIntType big1, const_bigIntType big2)

  {
    bigIntType result;

  /* bigXor */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_xor(result, big1, big2);
    return result;
  } /* bigXor */



bigIntType bigZero (void)

  {
    bigIntType result;

  /* bigZero */
    ALLOC_BIG(result);
    mpz_init_set_ui(result, 0);
    return result;
  } /* bigZero */
