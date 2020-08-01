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
/*  File: seed7/src/intlib.c                                        */
/*  Changes: 1992, 1993, 1994, 2000, 2005  Thomas Mertes            */
/*  Content: All primitive actions for the integer type.            */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "objutl.h"
#include "runerr.h"
#include "int_rtl.h"

#undef EXTERN
#define EXTERN
#include "intlib.h"


static inttype fact[] = {
    1, 1, 2, 6, 24, 120, 720, 5040, 40320,
    362880, 3628800, 39916800, 479001600,
#if INTTYPE_SIZE == 64
    INT_SUFFIX(6227020800), INT_SUFFIX(87178291200),
    INT_SUFFIX(1307674368000), INT_SUFFIX(20922789888000),
    INT_SUFFIX(355687428096000), INT_SUFFIX(6402373705728000),
    INT_SUFFIX(121645100408832000), INT_SUFFIX(2432902008176640000)
#endif
  };



/**
 *  Generic Destr function.
 *  Used for all types that require no special action to
 *  destroy the data.
 */
objecttype gen_destr (listtype arguments)

  { /* gen_destr */
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* gen_destr */



/**
 *  Compute the absolute value of an integer number.
 *  @return the absolute value.
 */
objecttype int_abs (listtype arguments)

  {
    inttype number;

  /* int_abs */
    isit_int(arg_1(arguments));
    number = take_int(arg_1(arguments));
    if (number < 0) {
      number = -number;
    } /* if */
    return bld_int_temp(number);
  } /* int_abs */



/**
 *  Add two integer numbers.
 *  @return the sum of the two numbers.
 */
objecttype int_add (listtype arguments)

  { /* int_add */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        take_int(arg_1(arguments)) + take_int(arg_3(arguments)));
  } /* int_add */



/**
 *  Compute a bitwise 'and' of two integer values.
 *  @return the bitwise 'and' of the two values.
 */
objecttype int_and (listtype arguments)

  { /* int_and */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        take_int(arg_1(arguments)) & take_int(arg_3(arguments)));
  } /* int_and */



/**
 *  Compute a bitwise 'and' and assign the result back.
 */
objecttype int_and_assign (listtype arguments)

  {
    objecttype int_variable;

  /* int_and_assign */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue &= take_int(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* int_and_assign */



/**
 *  Binomial coefficient
 *  @return n over k
 */
objecttype int_binom (listtype arguments)

  { /* int_binom */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        intBinom(take_int(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* int_binom */



/**
 *  Number of bits in the minimal two's-complement representation.
 *  The high bits equivalent to the sign bit are not part of the
 *  minimal two's-complement representation.
 *  @return the number of bits.
 */
objecttype int_bit_length (listtype arguments)

  { /* int_bit_length */
    isit_int(arg_1(arguments));
    return bld_int_temp(
        intBitLength(take_int(arg_1(arguments))));
  } /* int_bit_length */



/**
 *  Compare two integer numbers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objecttype int_cmp (listtype arguments)

  {
    inttype number1;
    inttype number2;
    inttype result;

  /* int_cmp */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    number1 = take_int(arg_1(arguments));
    number2 = take_int(arg_2(arguments));
    if (number1 < number2) {
      result = -1;
    } else if (number1 > number2) {
      result = 1;
    } else {
      result = 0;
    } /* if */
    return bld_int_temp(result);
  } /* int_cmp */



/**
 *  Convert to integer.
 *  @return the unchanged number.
 */
objecttype int_conv (listtype arguments)

  { /* int_conv */
    isit_int(arg_3(arguments));
    return bld_int_temp(take_int(arg_3(arguments)));
  } /* int_conv */



objecttype int_cpy (listtype arguments)

  {
    objecttype int_variable;

  /* int_cpy */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue = take_int(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* int_cpy */



objecttype int_create (listtype arguments)

  { /* int_create */
    isit_int(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), INTOBJECT);
    arg_1(arguments)->value.intvalue = take_int(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* int_create */



/**
 *  Decrement an integer variable.
 *  Decrements the number by 1.
 */
objecttype int_decr (listtype arguments)

  {
    objecttype int_variable;

  /* int_decr */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    int_variable->value.intvalue--;
    return SYS_EMPTY_OBJECT;
  } /* int_decr */



/**
 *  Integer division truncated towards zero.
 *  The remainder of this division is computed with int_rem.
 *  This function assumes that the C operator / truncates
 *  towards zero. Tests in chkint.sd7 find out, if this
 *  assumption is wrong.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
objecttype int_div (listtype arguments)

  {
    inttype divisor;

  /* int_div */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    divisor = take_int(arg_3(arguments));
    if (divisor == 0) {
      return raise_exception(SYS_NUM_EXCEPTION);
    } else {
      return bld_int_temp(
          take_int(arg_1(arguments)) / divisor);
    } /* if */
  } /* int_div */



/**
 *  Check if two integer numbers are equal.
 *  @return TRUE if the two numbers are equal,
 *          FALSE otherwise.
 */
objecttype int_eq (listtype arguments)

  { /* int_eq */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    if (take_int(arg_1(arguments)) ==
        take_int(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* int_eq */



/**
 *  Compute the factorial of a number.
 *  @return the factorial of the number.
 *  @exception NUMERIC_ERROR The number is negative or the result
 *             does not fit into an integer.
 */
objecttype int_fact (listtype arguments)

  {
    int number;

  /* int_fact */
    isit_int(arg_2(arguments));
    number = (int) take_int(arg_2(arguments));
    if (number < 0 || (size_t) number >= sizeof(fact) / sizeof(inttype)) {
      return raise_exception(SYS_NUM_EXCEPTION);
    } else {
      return bld_int_temp(fact[number]);
    } /* if */
  } /* int_fact */



/**
 *  Check if number1 is greater than or equal to number2.
 *  @return TRUE if number1 is greater than or equal to number2,
 *          FALSE otherwise.
 */
objecttype int_ge (listtype arguments)

  { /* int_ge */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    if (take_int(arg_1(arguments)) >=
        take_int(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* int_ge */



/**
 *  Increment an integer variable by a delta.
 */
objecttype int_grow (listtype arguments)

  {
    objecttype int_variable;

  /* int_grow */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue += take_int(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* int_grow */



/**
 *  Check if number1 is greater than number2.
 *  @return TRUE if number1 is greater than number2,
 *          FALSE otherwise.
 */
objecttype int_gt (listtype arguments)

  { /* int_gt */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    if (take_int(arg_1(arguments)) >
        take_int(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* int_gt */



/**
 *  Compute the hash value of an integer number.
 *  @return the hash value.
 */
objecttype int_hashcode (listtype arguments)

  { /* int_hashcode */
    isit_int(arg_1(arguments));
    return bld_int_temp(take_int(arg_1(arguments)));
  } /* int_hashcode */



/**
 *  Increment an integer variable.
 *  Increments the number by 1.
 */
objecttype int_incr (listtype arguments)

  {
    objecttype int_variable;

  /* int_incr */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    int_variable->value.intvalue++;
    return SYS_EMPTY_OBJECT;
  } /* int_incr */



/**
 *  Check if number1 is less than or equal to number2.
 *  @return TRUE if number1 is less than or equal to number2,
 *          FALSE otherwise.
 */
objecttype int_le (listtype arguments)

  { /* int_le */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    if (take_int(arg_1(arguments)) <=
        take_int(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* int_le */



/**
 *  Compute the truncated base 2 logarithm of an integer number.
 *  The definition of log2 is extended by defining log2(0) = -1.
 *  @return the truncated base 2 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
objecttype int_log2 (listtype arguments)

  { /* int_log2 */
    isit_int(arg_1(arguments));
    return bld_int_temp(
        intLog2(take_int(arg_1(arguments))));
  } /* int_log2 */



/**
 *  Index of the lowest-order one bit.
 *  For A <> 0 this is equal to the number of lowest-order zero bits.
 *  @return the number of lowest-order zero bits or -1 for lowestSetBit(0).
 */
objecttype int_lowest_set_bit (listtype arguments)

  { /* int_lowest_set_bit */
    isit_int(arg_1(arguments));
    return bld_int_temp(
        intLowestSetBit(take_int(arg_1(arguments))));
  } /* int_lowest_set_bit */



objecttype int_lpad0 (listtype arguments)

  { /* int_lpad0 */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        intLpad0(take_int(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* int_lpad0 */



/**
 *  Shift an integer number left by lshift bits.
 *  A << B is equivalent to A * 2_ ** B
 *  @return the left shifted number.
 */
objecttype int_lshift (listtype arguments)

  { /* int_lshift */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        take_int(arg_1(arguments)) << take_int(arg_3(arguments)));
  } /* int_lshift */



/**
 *  Shift a number left by lshift bits and assign the result back to number.
 */
objecttype int_lshift_assign (listtype arguments)

  {
    objecttype int_variable;

  /* int_lshift_assign */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue <<= take_int(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* int_lshift_assign */



/**
 *  Check if number1 is less than number2.
 *  @return TRUE if number1 is less than number2,
 *          FALSE otherwise.
 */
objecttype int_lt (listtype arguments)

  { /* int_lt */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    if (take_int(arg_1(arguments)) <
        take_int(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* int_lt */



/**
 *  Integer division truncated towards negative infinity.
 *  The modulo (remainder) of this division is computed with int_mod.
 *  Therefore this division is called modulo division (mdiv).
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
objecttype int_mdiv (listtype arguments)

  {
    inttype dividend;
    inttype divisor;
    inttype result;

  /* int_mdiv */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    dividend = take_int(arg_1(arguments));
    divisor = take_int(arg_3(arguments));
    if (divisor == 0) {
      return raise_exception(SYS_NUM_EXCEPTION);
    } else {
      if (dividend > 0 && divisor < 0) {
        result = (dividend - 1) / divisor - 1;
      } else if (dividend < 0 && divisor > 0) {
        result = (dividend + 1) / divisor - 1;
      } else {
        result = dividend / divisor;
      } /* if */
      return bld_int_temp(result);
    } /* if */
  } /* int_mdiv */



/**
 *  Compute the modulo (remainder) of the integer division int_mdiv.
 *  The modulo has the same sign as the divisor.
 *  @return the modulo of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
objecttype int_mod (listtype arguments)

  {
    inttype dividend;
    inttype divisor;
    inttype result;

  /* int_mod */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    dividend = take_int(arg_1(arguments));
    divisor = take_int(arg_3(arguments));
    if (divisor == 0) {
      return raise_exception(SYS_NUM_EXCEPTION);
    } else {
      result = dividend % divisor;
      if ((dividend < 0) ^ (divisor < 0) && result != 0) {
        result = result + divisor;
      } /* if */
      return bld_int_temp(result);
    } /* if */
  } /* int_mod */



/**
 *  Multiply two integer numbers.
 *  @return the product of the two numbers.
 */
objecttype int_mult (listtype arguments)

  { /* int_mult */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        take_int(arg_1(arguments)) * take_int(arg_3(arguments)));
  } /* int_mult */



/**
 *  Multiply an integer number by a factor and assign the result back to number.
 */
objecttype int_mult_assign (listtype arguments)

  {
    objecttype int_variable;

  /* int_mult_assign */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue *= take_int(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* int_mult_assign */



/**
 *  Check if two integer numbers are not equal.
 *  @return FALSE if both numbers are equal,
 *          TRUE otherwise.
 */
objecttype int_ne (listtype arguments)

  { /* int_ne */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    if (take_int(arg_1(arguments)) !=
        take_int(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* int_ne */



/**
 *  Minus sign, negate an integer number.
 *  @return the negated value of the number.
 */
objecttype int_negate (listtype arguments)

  { /* int_negate */
    isit_int(arg_2(arguments));
    return bld_int_temp(
        -take_int(arg_2(arguments)));
  } /* int_negate */



/**
 *  Convert to integer.
 *  @return the unchanged number.
 */
objecttype int_odd (listtype arguments)

  { /* int_odd */
    isit_int(arg_1(arguments));
    if (take_int(arg_1(arguments)) & 1) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* int_odd */



/**
 *  Compute a bitwise inclusive 'or' of two integer values.
 *  @return the bitwise inclusive 'or' of the two values.
 */
objecttype int_or (listtype arguments)

  { /* int_or */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        take_int(arg_1(arguments)) | take_int(arg_3(arguments)));
  } /* int_or */



/**
 *  Convert to integer.
 *  @return the unchanged number.
 */
objecttype int_ord (listtype arguments)

  { /* int_ord */
    isit_int(arg_1(arguments));
    return bld_int_temp(take_int(arg_1(arguments)));
  } /* int_ord */



/**
 *  Compute a bitwise inclusive 'or' and assign the result back.
 */
objecttype int_or_assign (listtype arguments)

  {
    objecttype int_variable;

  /* int_or_assign */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue |= take_int(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* int_or_assign */



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
objecttype int_parse (listtype arguments)

  { /* int_parse */
    isit_stri(arg_3(arguments));
    return bld_int_temp(
        intParse(take_stri(arg_3(arguments))));
  } /* int_parse */



/**
 *  Plus sign for integer numbers.
 *  @return its operand unchanged.
 */
objecttype int_plus (listtype arguments)

  { /* int_plus */
    isit_int(arg_2(arguments));
    return bld_int_temp(take_int(arg_2(arguments)));
  } /* int_plus */



/**
 *  Compute the exponentiation of a integer base with an integer exponent.
 *  @return the result of the exponentation.
 *  @exception NUMERIC_ERROR When the exponent is negative.
 */
objecttype int_pow (listtype arguments)

  { /* int_pow */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        intPow(take_int(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* int_pow */



/**
 *  Predecessor of an integer number.
 *  @return number - 1
 */
objecttype int_pred (listtype arguments)

  { /* int_pred */
    isit_int(arg_1(arguments));
    return bld_int_temp(take_int(arg_1(arguments)) - 1);
  } /* int_pred */



/**
 *  Convert an integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with lower case letters.
 *  E.g.: 10 is encoded with a, 11 with b, etc.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR When base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objecttype int_radix (listtype arguments)

  { /* int_radix */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        intRadix(take_int(arg_1(arguments)), take_int(arg_3(arguments)), FALSE));
  } /* int_radix */



/**
 *  Convert an integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with upper case letters.
 *  E.g.: 10 is encoded with A, 11 with B, etc.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR When base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objecttype int_RADIX (listtype arguments)

  { /* int_RADIX */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        intRadix(take_int(arg_1(arguments)), take_int(arg_3(arguments)), TRUE));
  } /* int_RADIX */



/**
 *  Compute pseudo-random number in the range [low, high].
 *  The random values are uniform distributed.
 *  @return a random number such that low <= rand(low, high) and
 *          rand(low, high) <= high holds.
 *  @exception RANGE_ERROR The range is empty (low > high holds).
 */
objecttype int_rand (listtype arguments)

  { /* int_rand */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    return bld_int_temp(
        intRand(take_int(arg_1(arguments)), take_int(arg_2(arguments))));
  } /* int_rand */



/**
 *  Compute the remainder of the integer division int_div.
 *  The remainder has the same sign as the dividend.
 *  @return the remainder of the integer division.
 *  @exception NUMERIC_ERROR When a division by zero occurs.
 */
objecttype int_rem (listtype arguments)

  {
    inttype divisor;

  /* int_rem */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    divisor = take_int(arg_3(arguments));
    if (divisor == 0) {
      return raise_exception(SYS_NUM_EXCEPTION);
    } else {
      return bld_int_temp(
          take_int(arg_1(arguments)) % divisor);
    } /* if */
  } /* int_rem */



/**
 *  Shift an integer number right by rshift bits.
 *  A >> B is equivalent to A mdiv 2_ ** B
 *  @return the right shifted number.
 */
objecttype int_rshift (listtype arguments)

  { /* int_rshift */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
#ifdef RSHIFT_DOES_SIGN_EXTEND
    return bld_int_temp(
        take_int(arg_1(arguments)) >>
        take_int(arg_3(arguments)));
#else
    if (take_int(arg_1(arguments)) < 0) {
      return bld_int_temp(
          ~(~take_int(arg_1(arguments)) >>
          take_int(arg_3(arguments))));
    } else {
      return bld_int_temp(
          take_int(arg_1(arguments)) >>
          take_int(arg_3(arguments)));
    } /* if */
#endif
  } /* int_rshift */



/**
 *  Shift a number right by rshift bits and assign the result back to number.
 */
objecttype int_rshift_assign (listtype arguments)

  {
    objecttype int_variable;

  /* int_rshift_assign */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
#ifdef RSHIFT_DOES_SIGN_EXTEND
    int_variable->value.intvalue >>= take_int(arg_3(arguments));
#else
    if (take_int(arg_1(arguments)) < 0) {
      int_variable->value.intvalue =
         ~(~int_variable->value.intvalue >> take_int(arg_3(arguments)));
    } else {
      int_variable->value.intvalue >>= take_int(arg_3(arguments));
    } /* if */
#endif
    return SYS_EMPTY_OBJECT;
  } /* int_rshift_assign */



/**
 *  Compute the subtraction of two integer numbers.
 *  @return the difference of the two numbers.
 */
objecttype int_sbtr (listtype arguments)

  { /* int_sbtr */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        take_int(arg_1(arguments)) - take_int(arg_3(arguments)));
  } /* int_sbtr */



/**
 *  Decrement an integer variable by a delta.
 */
objecttype int_shrink (listtype arguments)

  {
    objecttype int_variable;

  /* int_shrink */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue -= take_int(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* int_shrink */



/**
 *  Compute the integer square root of an integer number.
 *  @return the integer square root.
 *  @exception NUMERIC_ERROR When number is negative.
 */
objecttype int_sqrt (listtype arguments)

  { /* int_sqrt */
    isit_int(arg_1(arguments));
    return bld_int_temp(
        intSqrt(take_int(arg_1(arguments))));
  } /* int_sqrt */



/**
 *  Convert an integer number to a string.
 *  The number is converted to a string with decimal representation.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objecttype int_str (listtype arguments)

  { /* int_str */
    isit_int(arg_1(arguments));
    return bld_stri_temp(
        intStr(take_int(arg_1(arguments))));
  } /* int_str */



/**
 *  Successor of an integer number.
 *  @return number + 1
 */
objecttype int_succ (listtype arguments)

  { /* int_succ */
    isit_int(arg_1(arguments));
    return bld_int_temp(take_int(arg_1(arguments)) + 1);
  } /* int_succ */



objecttype int_toBStriBe (listtype arguments)

  { /* int_toBStriBe */
    isit_int(arg_1(arguments));
    return bld_bstri_temp(
        intToBStriBe(take_int(arg_1(arguments))));
  } /* int_toBStriBe */



objecttype int_toBStriLe (listtype arguments)

  { /* int_toBStriLe */
    isit_int(arg_1(arguments));
    return bld_bstri_temp(
        intToBStriLe(take_int(arg_1(arguments))));
  } /* int_toBStriLe */



#ifdef OUT_OF_ORDER
objecttype int_uadd (listtype arguments)

  { /* int_uadd */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp((inttype) (
        (uinttype) (take_int(arg_1(arguments))) +
        (uinttype) (take_int(arg_3(arguments)))));
  } /* int_uadd */



objecttype int_umult (listtype arguments)

  { /* int_umult */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp((inttype) (
        (uinttype) (take_int(arg_1(arguments))) *
        (uinttype) (take_int(arg_3(arguments)))));
  } /* int_umult */
#endif



objecttype int_urshift (listtype arguments)

  { /* int_urshift */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp((inttype) (
        (uinttype) (take_int(arg_1(arguments))) >>
        take_int(arg_3(arguments))));
  } /* int_urshift */



objecttype int_urshift_assign (listtype arguments)

  {
    objecttype int_variable;

  /* int_urshift_assign */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue = (inttype) (
        (uinttype) (int_variable->value.intvalue) >>
        take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* int_urshift_assign */



#ifdef OUT_OF_ORDER
objecttype int_usbtr (listtype arguments)

  { /* int_usbtr */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp((inttype) (
        (uinttype) (take_int(arg_1(arguments))) -
        (uinttype) (take_int(arg_3(arguments)))));
  } /* int_usbtr */
#endif



objecttype int_value (listtype arguments)

  {
    objecttype obj_arg;

  /* int_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != INTOBJECT) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_int_temp(take_int(obj_arg));
    } /* if */
  } /* int_value */



objecttype int_xor (listtype arguments)

  { /* int_xor */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        take_int(arg_1(arguments)) ^ take_int(arg_3(arguments)));
  } /* int_xor */



/**
 *  Compute a bitwise exclusive or ('xor') and assign the result back.
 */
objecttype int_xor_assign (listtype arguments)

  {
    objecttype int_variable;

  /* int_xor_assign */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue ^= take_int(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* int_xor_assign */
