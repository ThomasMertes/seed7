/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2019  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/biglib.c                                        */
/*  Changes: 2005, 2006, 2013 - 2018  Thomas Mertes                 */
/*  Content: All primitive actions for the bigInteger type.         */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "syvarutl.h"
#include "executl.h"
#include "objutl.h"
#include "runerr.h"
#include "big_drv.h"

#undef EXTERN
#define EXTERN
#include "biglib.h"



/**
 *  Compute the absolute value of a 'bigInteger' number.
 *  @return the absolute value.
 */
objectType big_abs (listType arguments)

  { /* big_abs */
    isit_bigint(arg_1(arguments));
    return bld_bigint_temp(
        bigAbs(take_bigint(arg_1(arguments))));
  } /* big_abs */



/**
 *  Add two 'bigInteger' numbers.
 *  The function sorts the two values by size. This way there is a
 *  loop up to the shorter size and a second loop up to the longer size.
 *  @return the sum of the two numbers.
 */
objectType big_add (listType arguments)

  { /* big_add */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigAdd(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_add */



/**
 *  Increment a 'bigInteger' variable by a delta.
 */
objectType big_add_assign (listType arguments)

  { /* big_add_assign */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigAddAssign(&take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* big_add_assign */



/**
 *  Number of bits in the minimal two's-complement representation.
 *  The high bits equivalent to the sign bit are not part of the
 *  minimal two's-complement representation.
 *  @return the number of bits.
 *  @exception RANGE_ERROR The result does not fit into an integer.
 */
objectType big_bit_length (listType arguments)

  { /* big_bit_length */
    isit_bigint(arg_1(arguments));
    return bld_int_temp(
        bigBitLength(take_bigint(arg_1(arguments))));
  } /* big_bit_length */



/**
 *  Compare two 'bigInteger' numbers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objectType big_cmp (listType arguments)

  { /* big_cmp */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_2(arguments));
    return bld_int_temp(
        bigCmp(take_bigint(arg_1(arguments)), take_bigint(arg_2(arguments))));
  } /* big_cmp */



/**
 *  Convert a 'bigInteger' to 'bigInteger'.
 *  @return the unchanged 'bigInteger' number.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType big_conv (listType arguments)

  {
    bigIntType arg;
    bigIntType result;

  /* big_conv */
    isit_bigint(arg_3(arguments));
    arg = take_bigint(arg_3(arguments));
    if (TEMP_OBJECT(arg_3(arguments))) {
      result = arg;
      arg_3(arguments)->value.bigIntValue = NULL;
    } else {
      result = bigCreate(arg);
    } /* if */
    return bld_bigint_temp(result);
  } /* big_conv */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType big_cpy (listType arguments)

  {
    objectType dest;
    objectType source;

  /* big_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_bigint(dest);
    isit_bigint(source);
    is_variable(dest);
    if (TEMP_OBJECT(source)) {
      bigDestr(take_bigint(dest));
      dest->value.bigIntValue = take_bigint(source);
      source->value.bigIntValue = NULL;
    } else {
      bigCpy(&dest->value.bigIntValue, take_bigint(source));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* big_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType big_create (listType arguments)

  {
    objectType dest;
    objectType source;

  /* big_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_bigint(source);
    SET_CATEGORY_OF_OBJ(dest, BIGINTOBJECT);
    if (TEMP_OBJECT(source)) {
      dest->value.bigIntValue = take_bigint(source);
      source->value.bigIntValue = NULL;
    } else {
      dest->value.bigIntValue = bigCreate(take_bigint(source));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* big_create */



/**
 *  Decrement a 'bigInteger' variable.
 *  Decrements the number by 1.
 */
objectType big_decr (listType arguments)

  {
    objectType big_variable;

  /* big_decr */
    big_variable = arg_1(arguments);
    isit_bigint(big_variable);
    is_variable(big_variable);
    bigDecr(&take_bigint(big_variable));
    return SYS_EMPTY_OBJECT;
  } /* big_decr */



/**
 *  Free the memory referred by 'old_bigint/arg_1'.
 *  After big_destr is left 'old_bigint/arg_1' is NULL.
 *  The memory where 'old_bigint/arg_1' is stored can be freed afterwards.
 */
objectType big_destr (listType arguments)

  { /* big_destr */
    isit_bigint(arg_1(arguments));
    bigDestr(take_bigint(arg_1(arguments)));
    arg_1(arguments)->value.bigIntValue = NULL;
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* big_destr */



/**
 *  Integer division truncated towards zero.
 *  The remainder of this division is computed with big_rem.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
objectType big_div (listType arguments)

  { /* big_div */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigDiv(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_div */



/**
 *  Quotient and remainder of integer division truncated towards zero.
 *  Compute quotient and remainder of the integer division ''div''.
 *  @return quotRem with quotient and remainder of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
objectType big_div_rem (listType arguments)

  {
    objectType valueObj;
    structType quotRem;

  /* big_div_rem */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    valueObj = getValue(curr_exec_object->type_of->result_type->match_obj);
    if (!ALLOC_STRUCT(quotRem, 2)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      quotRem->usage_count = 1;
      quotRem->size = 2;
      quotRem->stru[0].value.bigIntValue = bigDivRem(
          take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)),
          &quotRem->stru[1].value.bigIntValue);
      quotRem->stru[0].type_of = SYS_BIGINT_TYPE->value.typeValue;
      quotRem->stru[0].descriptor.property = take_struct(valueObj)->stru[0].descriptor.property;
      INIT_CATEGORY_OF_VAR(&quotRem->stru[0], BIGINTOBJECT);
      quotRem->stru[1].type_of = SYS_BIGINT_TYPE->value.typeValue;
      quotRem->stru[1].descriptor.property = take_struct(valueObj)->stru[1].descriptor.property;
      INIT_CATEGORY_OF_VAR(&quotRem->stru[1], BIGINTOBJECT);
      return bld_struct_temp(quotRem);
    } /* if */
  } /* big_div_rem */



/**
 *  Check if two 'bigInteger' numbers are equal.
 *  @return TRUE if both numbers are equal,
 *          FALSE otherwise.
 */
objectType big_eq (listType arguments)

  { /* big_eq */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    if (bigEq(take_bigint(arg_1(arguments)),
              take_bigint(arg_3(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* big_eq */



/**
 *  Convert a bstring (interpreted as big-endian) to a bigInteger.
 *  @param bstri/arg_1 Bstring to be converted. The bytes are interpreted
 *         as binary big-endian representation with a base of 256.
 *  @param isSigned/arg_2 Defines if 'bstri' is interpreted as signed value.
 *         If 'isSigned' is TRUE the twos-complement representation
 *         is used. In this case the result is negative if the most
 *         significant byte (the first byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bigInteger created from the big-endian bytes.
 */
objectType big_from_bstri_be (listType arguments)

  { /* big_from_bstri_be */
    isit_bstri(arg_1(arguments));
    isit_bool(arg_2(arguments));
    return bld_bigint_temp(
        bigFromBStriBe(take_bstri(arg_1(arguments)),
                       take_bool(arg_2(arguments)) == SYS_TRUE_OBJECT));
  } /* big_from_bstri_be */



/**
 *  Convert a bstring (interpreted as little-endian) to a bigInteger.
 *  @param bstri/arg_1 Bstring to be converted. The bytes are interpreted
 *         as binary little-endian representation with a base of 256.
 *  @param isSigned/arg_2 Defines if 'bstri' is interpreted as signed value.
 *         If 'isSigned' is TRUE the twos-complement representation
 *         is used. In this case the result is negative if the most
 *         significant byte (the last byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bigInteger created from the little-endian bytes.
 */
objectType big_from_bstri_le (listType arguments)

  { /* big_from_bstri_le */
    isit_bstri(arg_1(arguments));
    isit_bool(arg_2(arguments));
    return bld_bigint_temp(
        bigFromBStriLe(take_bstri(arg_1(arguments)),
                       take_bool(arg_2(arguments)) == SYS_TRUE_OBJECT));
  } /* big_from_bstri_le */



/**
 *  Compute the greatest common divisor of two 'bigInteger' numbers.
 *  @return the greatest common divisor of the two numbers.
 */
objectType big_gcd (listType arguments)

  { /* big_gcd */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_2(arguments));
    return bld_bigint_temp(
        bigGcd(take_bigint(arg_1(arguments)), take_bigint(arg_2(arguments))));
  } /* big_gcd */



/**
 *  Check if number1 is greater than or equal to number2.
 *  @return TRUE if number1 is greater than or equal to number2,
 *          FALSE otherwise.
 */
objectType big_ge (listType arguments)

  { /* big_ge */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    if (bigCmp(take_bigint(arg_1(arguments)),
               take_bigint(arg_3(arguments))) >= 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* big_ge */



/**
 *  Check if number1 is greater than number2.
 *  @return TRUE if number1 is greater than number2,
 *          FALSE otherwise.
 */
objectType big_gt (listType arguments)

  { /* big_gt */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    if (bigCmp(take_bigint(arg_1(arguments)),
               take_bigint(arg_3(arguments))) > 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* big_gt */



/**
 *  Compute the hash value of a 'bigInteger' number.
 *  @return the hash value.
 */
objectType big_hashcode (listType arguments)

  { /* big_hashcode */
    isit_bigint(arg_1(arguments));
    return bld_int_temp(
        bigHashCode(take_bigint(arg_1(arguments))));
  } /* big_hashcode */



/**
 *  Convert an integer number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType big_iconv1 (listType arguments)

  { /* big_iconv1 */
    isit_int(arg_1(arguments));
    return bld_bigint_temp(
        bigIConv(take_int(arg_1(arguments))));
  } /* big_iconv1 */



/**
 *  Convert an integer number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType big_iconv3 (listType arguments)

  { /* big_iconv3 */
    isit_int(arg_3(arguments));
    return bld_bigint_temp(
        bigIConv(take_int(arg_3(arguments))));
  } /* big_iconv3 */



/**
 *  Increment a 'bigInteger' variable.
 *  Increments the number by 1.
 */
objectType big_incr (listType arguments)

  {
    objectType big_variable;

  /* big_incr */
    big_variable = arg_1(arguments);
    isit_bigint(big_variable);
    is_variable(big_variable);
    bigIncr(&take_bigint(big_variable));
    return SYS_EMPTY_OBJECT;
  } /* big_incr */



/**
 *  Compute the exponentiation of a 'bigInteger' base with an integer exponent.
 *  @return the result of the exponentiation.
 *  @exception NUMERIC_ERROR If the exponent is negative.
 */
objectType big_ipow (listType arguments)

  { /* big_ipow */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_bigint_temp(
        bigIPow(take_bigint(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* big_ipow */



/**
 *  Check if number1 is less than or equal to number2.
 *  @return TRUE if number1 is less than or equal to number2,
 *          FALSE otherwise.
 */
objectType big_le (listType arguments)

  { /* big_le */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    if (bigCmp(take_bigint(arg_1(arguments)),
               take_bigint(arg_3(arguments))) <= 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* big_le */



/**
 *  Compute the truncated base 10 logarithm of a 'bigInteger' number.
 *  The definition of 'log10' is extended by defining log10(0) = -1_.
 *  @return the truncated base 10 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
objectType big_log10 (listType arguments)

  { /* big_log10 */
    isit_bigint(arg_1(arguments));
    return bld_bigint_temp(
        bigLog10(take_bigint(arg_1(arguments))));
  } /* big_log10 */



/**
 *  Compute the truncated base 2 logarithm of a 'bigInteger' number.
 *  The definition of 'log2' is extended by defining log2(0) = -1_.
 *  @return the truncated base 2 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
objectType big_log2 (listType arguments)

  { /* big_log2 */
    isit_bigint(arg_1(arguments));
    return bld_bigint_temp(
        bigLog2(take_bigint(arg_1(arguments))));
  } /* big_log2 */



/**
 *  Index of the lowest-order one bit.
 *  For A <> 0 this is equal to the number of lowest-order zero bits.
 *  @return the number of lowest-order zero bits or -1 for lowestSetBit(0).
 */
objectType big_lowest_set_bit (listType arguments)

  { /* big_lowest_set_bit */
    isit_bigint(arg_1(arguments));
    return bld_int_temp(
        bigLowestSetBit(take_bigint(arg_1(arguments))));
  } /* big_lowest_set_bit */



/**
 *  Shift a 'bigInteger' number left by lshift bits.
 *  If lshift is negative a right shift is done instead.
 *  A << B is equivalent to A * 2_ ** B if B >= 0 holds.
 *  A << B is equivalent to A mdiv 2_ ** -B if B < 0 holds.
 *  @return the left shifted number.
 */
objectType big_lshift (listType arguments)

  { /* big_lshift */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_bigint_temp(
        bigLShift(take_bigint(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* big_lshift */



/**
 *  Shift a number left by lshift bits and assign the result back to number.
 *  If lshift is negative a right shift is done instead.
 */
objectType big_lshift_assign (listType arguments)

  { /* big_lshift_assign */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigLShiftAssign(&take_bigint(arg_1(arguments)), take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* big_lshift_assign */



/**
 *  Check if number1 is less than number2.
 *  @return TRUE if number1 is less than number2,
 *          FALSE otherwise.
 */
objectType big_lt (listType arguments)

  { /* big_lt */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    if (bigCmp(take_bigint(arg_1(arguments)),
               take_bigint(arg_3(arguments))) < 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* big_lt */



/**
 *  Integer division truncated towards negative infinity.
 *  The modulo (remainder) of this division is computed with big_mod.
 *  Therefore this division is called modulo division (mdiv).
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
objectType big_mdiv (listType arguments)

  { /* big_mdiv */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigMDiv(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_mdiv */



/**
 *  Compute the modulo (remainder) of the integer division big_mdiv.
 *  The modulo has the same sign as the divisor.
 *  @return the modulo of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
objectType big_mod (listType arguments)

  { /* big_mod */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigMod(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_mod */



/**
 *  Multiply two 'bigInteger' numbers.
 *  @return the product of the two numbers.
 */
objectType big_mult (listType arguments)

  { /* big_mult */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigMult(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_mult */



/**
 *  Multiply a 'bigInteger' number by a factor and assign the result back to number.
 */
objectType big_mult_assign (listType arguments)

  { /* big_mult_assign */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigMultAssign(&take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* big_mult_assign */



/**
 *  Check if two 'bigInteger' numbers are not equal.
 *  @return FALSE if both numbers are equal,
 *          TRUE otherwise.
 */
objectType big_ne (listType arguments)

  { /* big_ne */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    if (bigEq(take_bigint(arg_1(arguments)),
              take_bigint(arg_3(arguments)))) {
      return SYS_FALSE_OBJECT;
    } else {
      return SYS_TRUE_OBJECT;
    } /* if */
  } /* big_ne */



/**
 *  Minus sign, negate a 'bigInteger' number.
 *  @return the negated value of the number.
 */
objectType big_negate (listType arguments)

  { /* big_negate */
    isit_bigint(arg_2(arguments));
    return bld_bigint_temp(
        bigNegate(take_bigint(arg_2(arguments))));
  } /* big_negate */



/**
 *  Determine if a 'bigInteger' number is odd.
 *  @return TRUE if the number is odd,
 *          FALSE otherwise.
 */
objectType big_odd (listType arguments)

  { /* big_odd */
    isit_bigint(arg_1(arguments));
    if (bigOdd(take_bigint(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* big_odd */



/**
 *  Convert a 'bigInteger' number to integer.
 *  @return the integer result of the conversion.
 *  @exception RANGE_ERROR The result does not fit into an integer.
 */
objectType big_ord (listType arguments)

  { /* big_ord */
    isit_bigint(arg_1(arguments));
    return bld_int_temp(
        bigOrd(take_bigint(arg_1(arguments))));
  } /* big_ord */



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
objectType big_parse1 (listType arguments)

  { /* big_parse1 */
    isit_stri(arg_1(arguments));
    return bld_bigint_temp(
        bigParse(take_stri(arg_1(arguments))));
  } /* big_parse1 */



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
 *  @exception RANGE_ERROR If base < 2 or base > 36 holds or
 *             the string is empty or it does not contain an integer
 *             literal with the specified base.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
objectType big_parse_based (listType arguments)

  { /* big_parse_based */
    isit_stri(arg_1(arguments));
    isit_int(arg_2(arguments));
    return bld_bigint_temp(
        bigParseBased(take_stri(arg_1(arguments)), take_int(arg_2(arguments))));
  } /* big_parse_based */



/**
 *  Plus sign for 'bigInteger' numbers.
 *  @return its operand unchanged.
 */
objectType big_plus (listType arguments)

  {
    bigIntType result;

  /* big_plus */
    isit_bigint(arg_2(arguments));
    if (TEMP_OBJECT(arg_2(arguments))) {
      result = take_bigint(arg_2(arguments));
      arg_2(arguments)->value.bigIntValue = NULL;
    } else {
      result = bigCreate(take_bigint(arg_2(arguments)));
    } /* if */
    return bld_bigint_temp(result);
  } /* big_plus */



/**
 *  Predecessor of a 'bigInteger' number.
 *  pred(A) is equivalent to A-1 .
 *  @return number - 1 .
 */
objectType big_pred (listType arguments)

  { /* big_pred */
    isit_bigint(arg_1(arguments));
    return bld_bigint_temp(
        bigPred(take_bigint(arg_1(arguments))));
  } /* big_pred */



/**
 *  Convert a big integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with lower case letters.
 *  E.g.: 10 is encoded with a, 11 with b, etc.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR If base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType big_radix (listType arguments)

  { /* big_radix */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        bigRadix(take_bigint(arg_1(arguments)), take_int(arg_3(arguments)), FALSE));
  } /* big_radix */



/**
 *  Convert a big integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with upper case letters.
 *  E.g.: 10 is encoded with A, 11 with B, etc.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR If base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType big_RADIX (listType arguments)

  { /* big_RADIX */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        bigRadix(take_bigint(arg_1(arguments)), take_int(arg_3(arguments)), TRUE));
  } /* big_RADIX */



/**
 *  Compute pseudo-random number in the range [low, high].
 *  The random values are uniform distributed.
 *  @return a random number such that low <= rand(low, high) and
 *          rand(low, high) <= high holds.
 *  @exception RANGE_ERROR The range is empty (low > high holds).
 */
objectType big_rand (listType arguments)

  { /* big_rand */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_2(arguments));
    return bld_bigint_temp(
        bigRand(take_bigint(arg_1(arguments)), take_bigint(arg_2(arguments))));
  } /* big_rand */



/**
 *  Compute the remainder of the integer division big_div.
 *  The remainder has the same sign as the dividend.
 *  @return the remainder of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
objectType big_rem (listType arguments)

  { /* big_rem */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigRem(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_rem */



/**
 *  Shift a 'bigInteger' number right by rshift bits.
 *  If rshift is negative a left shift is done instead.
 *  A >> B is equivalent to A mdiv 2_ ** B if B >= 0 holds.
 *  A >> B is equivalent to A * 2_ ** -B if B < 0 holds.
 *  @return the right shifted number.
 */
objectType big_rshift (listType arguments)

  { /* big_rshift */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_bigint_temp(
        bigRShift(take_bigint(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* big_rshift */



/**
 *  Shift a number right by rshift bits and assign the result back to number.
 *  If rshift is negative a left shift is done instead.
 */
objectType big_rshift_assign (listType arguments)

  { /* big_rshift_assign */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigRShiftAssign(&take_bigint(arg_1(arguments)), take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* big_rshift_assign */



/**
 *  Compute the subtraction of two 'bigInteger' numbers.
 *  @return the difference of the two numbers.
 */
objectType big_sbtr (listType arguments)

  { /* big_sbtr */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigSbtr(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_sbtr */



/**
 *  Decrement a 'bigInteger' variable by a delta.
 */
objectType big_sbtr_assign (listType arguments)

  { /* big_sbtr_assign */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigSbtrAssign(&take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* big_sbtr_assign */



/**
 *  Convert a 'bigInteger' number to a string.
 *  The number is converted to a string with decimal representation.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
objectType big_str (listType arguments)

  { /* big_str */
    isit_bigint(arg_1(arguments));
    return bld_stri_temp(
        bigStr(take_bigint(arg_1(arguments))));
  } /* big_str */



/**
 *  Successor of a 'bigInteger' number.
 *  succ(A) is equivalent to A+1 .
 *  @return number + 1 .
 */
objectType big_succ (listType arguments)

  { /* big_succ */
    isit_bigint(arg_1(arguments));
    return bld_bigint_temp(
        bigSucc(take_bigint(arg_1(arguments))));
  } /* big_succ */



/**
 *  Convert a 'bigInteger' into a big-endian 'bstring'.
 *  The result uses binary representation with a base of 256.
 *  @param big1/arg_1 BigInteger number to be converted.
 *  @param isSigned/arg_2 Determines the signedness of the result.
 *         If 'isSigned' is TRUE the result is encoded with the
 *         twos-complement representation. In this case a negative
 *         'big1' is converted to a result where the most significant
 *         byte (the first byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bstring with the big-endian representation.
 *  @exception RANGE_ERROR If 'isSigned' is FALSE and 'big1' is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType big_to_bstri_be (listType arguments)

  { /* big_to_bstri_be */
    isit_bigint(arg_1(arguments));
    isit_bool(arg_2(arguments));
    return bld_bstri_temp(
        bigToBStriBe(take_bigint(arg_1(arguments)),
                     take_bool(arg_2(arguments)) == SYS_TRUE_OBJECT));
  } /* big_to_bstri_be */



/**
 *  Convert a 'bigInteger' into a little-endian 'bstring'.
 *  The result uses binary representation with a base of 256.
 *  @param big1/arg_1 BigInteger number to be converted.
 *  @param isSigned/arg_2 Determines the signedness of the result.
 *         If 'isSigned' is TRUE the result is encoded with the
 *         twos-complement representation. In this case a negative
 *         'big1' is converted to a result where the most significant
 *         byte (the last byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bstring with the little-endian representation.
 *  @exception RANGE_ERROR If 'isSigned' is FALSE and 'big1' is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType big_to_bstri_le (listType arguments)

  { /* big_to_bstri_le */
    isit_bigint(arg_1(arguments));
    isit_bool(arg_2(arguments));
    return bld_bstri_temp(
        bigToBStriLe(take_bigint(arg_1(arguments)),
                     take_bool(arg_2(arguments)) == SYS_TRUE_OBJECT));
  } /* big_to_bstri_le */



/**
 *  Get 'bigInteger' value of the object referenced by 'aReference/arg_1'.
 *  @return the 'bigInteger' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference/arg_1' is NIL or
 *             category(aReference) <> BIGINTOBJECT holds.
 */
objectType big_value (listType arguments)

  {
    objectType aReference;

  /* big_value */
    isit_reference(arg_1(arguments));
    aReference = take_reference(arg_1(arguments));
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != BIGINTOBJECT)) {
      logError(printf("big_value(");
               trace1(aReference);
               printf("): Category is not BIGINTOBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_bigint_temp(bigCreate(take_bigint(aReference)));
    } /* if */
  } /* big_value */
