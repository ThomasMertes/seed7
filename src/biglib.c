/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  Changes: 2005, 2006  Thomas Mertes                              */
/*  Content: All primitive actions for the bigInteger type.         */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "syvarutl.h"
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
objecttype big_abs (listtype arguments)

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
objecttype big_add (listtype arguments)

  { /* big_add */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigAdd(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_add */



/**
 *  Number of bits in the minimal two's-complement representation.
 *  The high bits equivalent to the sign bit are not part of the
 *  minimal two's-complement representation.
 *  @return the number of bits.
 *  @exception RANGE_ERROR The result does not fit into an integer.
 */
objecttype big_bit_length (listtype arguments)

  { /* big_bit_length */
    isit_bigint(arg_1(arguments));
    return bld_int_temp(
        bigBitLength(take_bigint(arg_1(arguments))));
  } /* big_bit_length */



objecttype big_clit (listtype arguments)

  { /* big_clit */
    isit_bigint(arg_1(arguments));
    return bld_stri_temp(
        bigCLit(take_bigint(arg_1(arguments))));
  } /* big_clit */



/**
 *  Compare two 'bigInteger' numbers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objecttype big_cmp (listtype arguments)

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
objecttype big_conv (listtype arguments)

  {
    biginttype arg;
    biginttype result;

  /* big_conv */
    isit_bigint(arg_3(arguments));
    arg = take_bigint(arg_3(arguments));
    if (TEMP_OBJECT(arg_3(arguments))) {
      result = arg;
      arg_3(arguments)->value.bigintvalue = NULL;
    } else {
      result = bigCreate(arg);
    } /* if */
    return bld_bigint_temp(result);
  } /* big_conv */



objecttype big_cpy (listtype arguments)

  {
    objecttype big_to;
    objecttype big_from;

  /* big_cpy */
    big_to = arg_1(arguments);
    big_from = arg_3(arguments);
    isit_bigint(big_to);
    isit_bigint(big_from);
    is_variable(big_to);
    if (TEMP_OBJECT(big_from)) {
      bigDestr(take_bigint(big_to));
      big_to->value.bigintvalue = take_bigint(big_from);
      big_from->value.bigintvalue = NULL;
    } else {
      bigCpy(&big_to->value.bigintvalue, take_bigint(big_from));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* big_cpy */



objecttype big_create (listtype arguments)

  {
    objecttype big_to;
    objecttype big_from;

  /* big_create */
    big_to = arg_1(arguments);
    big_from = arg_3(arguments);
    isit_bigint(big_from);
    SET_CATEGORY_OF_OBJ(big_to, BIGINTOBJECT);
    if (TEMP_OBJECT(big_from)) {
      big_to->value.bigintvalue = take_bigint(big_from);
      big_from->value.bigintvalue = NULL;
    } else {
      big_to->value.bigintvalue = bigCreate(take_bigint(big_from));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* big_create */



/**
 *  Decrement a 'bigInteger' variable.
 *  Decrements the number by 1.
 */
objecttype big_decr (listtype arguments)

  {
    objecttype big_variable;

  /* big_decr */
    big_variable = arg_1(arguments);
    isit_bigint(big_variable);
    is_variable(big_variable);
    bigDecr(&take_bigint(big_variable));
    return SYS_EMPTY_OBJECT;
  } /* big_decr */



objecttype big_destr (listtype arguments)

  { /* big_destr */
    isit_bigint(arg_1(arguments));
    bigDestr(take_bigint(arg_1(arguments)));
    arg_1(arguments)->value.bigintvalue = NULL;
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* big_destr */



/**
 *  Integer division truncated towards zero.
 *  The remainder of this division is computed with big_rem.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
objecttype big_div (listtype arguments)

  { /* big_div */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigDiv(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_div */



/**
 *  Check if two 'bigInteger' numbers are equal.
 *  @return TRUE if both numbers are equal,
 *          FALSE otherwise.
 */
objecttype big_eq (listtype arguments)

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
 *  Compute the greatest common divisor of two 'bigInteger' numbers.
 *  @return the greatest common divisor of the two numbers.
 */
objecttype big_gcd (listtype arguments)

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
objecttype big_ge (listtype arguments)

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
 *  Increment a 'bigInteger' variable by a delta.
 */
objecttype big_grow (listtype arguments)

  { /* big_grow */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigGrow(&take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* big_grow */



/**
 *  Check if number1 is greater than number2.
 *  @return TRUE if number1 is greater than number2,
 *          FALSE otherwise.
 */
objecttype big_gt (listtype arguments)

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
objecttype big_hashcode (listtype arguments)

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
objecttype big_iconv (listtype arguments)

  { /* big_iconv */
    isit_int(arg_3(arguments));
    return bld_bigint_temp(
        bigIConv(take_int(arg_3(arguments))));
  } /* big_iconv */



/**
 *  Increment a 'bigInteger' variable.
 *  Increments the number by 1.
 */
objecttype big_incr (listtype arguments)

  {
    objecttype big_variable;

  /* big_incr */
    big_variable = arg_1(arguments);
    isit_bigint(big_variable);
    is_variable(big_variable);
    bigIncr(&take_bigint(big_variable));
    return SYS_EMPTY_OBJECT;
  } /* big_incr */



/**
 *  Compute the exponentiation of a 'bigInteger' base by an integer exponent.
 *  @return the result of the exponentation.
 *  @exception NUMERIC_ERROR When the exponent is negative.
 */
objecttype big_ipow (listtype arguments)

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
objecttype big_le (listtype arguments)

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
 *  Compute the truncated base 2 logarithm of a 'bigInteger' number.
 *  The definition of 'log2' is extended by defining log2(0) = -1_.
 *  @return the truncated base 2 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
objecttype big_log2 (listtype arguments)

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
objecttype big_lowest_set_bit (listtype arguments)

  { /* big_lowest_set_bit */
    isit_bigint(arg_1(arguments));
    return bld_int_temp(
        bigLowestSetBit(take_bigint(arg_1(arguments))));
  } /* big_lowest_set_bit */



/**
 *  Shift a 'bigInteger' number left by lshift bits.
 *  When lshift is negative a right shift is done instead.
 *  A << B is equivalent to A * 2_ ** B when B >= 0 holds.
 *  A << B is equivalent to A mdiv 2_ ** -B when B < 0 holds.
 *  @return the left shifted number.
 */
objecttype big_lshift (listtype arguments)

  { /* big_lshift */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_bigint_temp(
        bigLShift(take_bigint(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* big_lshift */



/**
 *  Shift a number left by lshift bits and assign the result back to number.
 *  When lshift is negative a right shift is done instead.
 */
objecttype big_lshift_assign (listtype arguments)

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
objecttype big_lt (listtype arguments)

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
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
objecttype big_mdiv (listtype arguments)

  { /* big_mdiv */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigMDiv(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_mdiv */



/**
 *  Minus sign, negate a 'bigInteger' number.
 *  @return the negated value of the number.
 */
objecttype big_minus (listtype arguments)

  { /* big_minus */
    isit_bigint(arg_2(arguments));
    return bld_bigint_temp(
        bigMinus(take_bigint(arg_2(arguments))));
  } /* big_minus */



/**
 *  Compute the modulo (remainder) of the integer division big_mdiv.
 *  The modulo has the same sign as the divisor.
 *  @return the modulo of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
objecttype big_mod (listtype arguments)

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
objecttype big_mult (listtype arguments)

  { /* big_mult */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigMult(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_mult */



/**
 *  Multiply a 'bigInteger' number by a factor and assign the result back to number.
 */
objecttype big_mult_assign (listtype arguments)

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
objecttype big_ne (listtype arguments)

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
 *  Determine if a 'bigInteger' number is odd.
 *  @return TRUE if the number is odd,
 *          FALSE otherwise.
 */
objecttype big_odd (listtype arguments)

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
objecttype big_ord (listtype arguments)

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
 *  @exception RANGE_ERROR When the string is empty or does not contain
 *             an integer literal.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
objecttype big_parse (listtype arguments)

  { /* big_parse */
    isit_stri(arg_3(arguments));
    return bld_bigint_temp(
        bigParse(take_stri(arg_3(arguments))));
  } /* big_parse */



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
objecttype big_parse_based (listtype arguments)

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
objecttype big_plus (listtype arguments)

  {
    biginttype result;

  /* big_plus */
    isit_bigint(arg_2(arguments));
    if (TEMP_OBJECT(arg_2(arguments))) {
      result = take_bigint(arg_2(arguments));
      arg_2(arguments)->value.bigintvalue = NULL;
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
objecttype big_pred (listtype arguments)

  { /* big_pred */
    isit_bigint(arg_1(arguments));
    return bld_bigint_temp(
        bigPred(take_bigint(arg_1(arguments))));
  } /* big_pred */



/**
 *  Compute pseudo-random number in the range [low, high].
 *  The random values are uniform distributed.
 *  @return a random number such that low <= rand(low, high) and
 *          rand(low, high) <= high holds.
 *  @exception RANGE_ERROR The range is empty (low > high holds).
 */
objecttype big_rand (listtype arguments)

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
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
objecttype big_rem (listtype arguments)

  { /* big_rem */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigRem(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_rem */



/**
 *  Shift a 'bigInteger' number right by rshift bits.
 *  When rshift is negative a left shift is done instead.
 *  A >> B is equivalent to A mdiv 2_ ** B when B >= 0 holds.
 *  A >> B is equivalent to A * 2_ ** -B when B < 0 holds.
 *  @return the right shifted number.
 */
objecttype big_rshift (listtype arguments)

  { /* big_rshift */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_bigint_temp(
        bigRShift(take_bigint(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* big_rshift */



/**
 *  Shift a number right by rshift bits and assign the result back to number.
 *  When rshift is negative a left shift is done instead.
 */
objecttype big_rshift_assign (listtype arguments)

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
objecttype big_sbtr (listtype arguments)

  { /* big_sbtr */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigSbtr(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_sbtr */



/**
 *  Decrement a 'bigInteger' variable by a delta.
 */
objecttype big_shrink (listtype arguments)

  { /* big_shrink */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigShrink(&take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* big_shrink */



/**
 *  Convert a 'bigInteger' number to a string.
 *  The number is converted to a string with decimal representation.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
objecttype big_str (listtype arguments)

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
objecttype big_succ (listtype arguments)

  { /* big_succ */
    isit_bigint(arg_1(arguments));
    return bld_bigint_temp(
        bigSucc(take_bigint(arg_1(arguments))));
  } /* big_succ */



objecttype big_value (listtype arguments)

  {
    objecttype obj_arg;

  /* big_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != BIGINTOBJECT) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_bigint_temp(bigCreate(take_bigint(obj_arg)));
    } /* if */
  } /* big_value */
