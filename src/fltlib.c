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
/*  File: seed7/src/fltlib.c                                        */
/*  Changes: 1993, 1994, 2008, 2011 - 2016, 2019  Thomas Mertes     */
/*  Content: All primitive actions for the float type.              */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "math.h"
#include "float.h"
#include "limits.h"

#include "common.h"
#include "data.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "objutl.h"
#include "runerr.h"
#include "flt_rtl.h"

#undef EXTERN
#define EXTERN
#include "fltlib.h"

typedef union {
#if FLOAT_SIZE == 32
  uint32Type bits;
#endif
  float aFloat;
} float2BitsType;

typedef union {
#if DOUBLE_SIZE == 64
  uint64Type bits;
#endif
  double aDouble;
} double2BitsType;

static const intType minimumTruncArgument = 
#if TWOS_COMPLEMENT_INTTYPE
    INTTYPE_MIN;
#else
    INT_SUFFIX(MINIMUM_TRUNC_ARGUMENT);
#endif



/**
 *  Compute the absolute value of a float number.
 *  @return the absolute value.
 */
objectType flt_abs (listType arguments)

  {
    double number;

  /* flt_abs */
    isit_float(arg_1(arguments));
    number = take_float(arg_1(arguments));
    if (number < (double) 0.0) {
      number = -number;
    } /* if */
    return bld_float_temp(number);
  } /* flt_abs */



/**
 *  Compute the arc cosine of x; that is the value whose cosine is x.
 *  @return the arc cosine of x in radians. The returned angle is in
 *          the range [0.0, PI].
 */
objectType flt_acos (listType arguments)

  { /* flt_acos */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        acos(take_float(arg_1(arguments))));
  } /* flt_acos */



/**
 *  Add two float numbers.
 *  @return the sum of the two numbers.
 */
objectType flt_add (listType arguments)

  { /* flt_add */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return bld_float_temp(
        (double) take_float(arg_1(arguments)) +
        (double) take_float(arg_3(arguments)));
  } /* flt_add */



/**
 *  Increment a float 'number' by a 'delta'.
 */
objectType flt_add_assign (listType arguments)

  {
    objectType flt_variable;

  /* flt_add_assign */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    flt_variable->value.floatValue += take_float(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* flt_add_assign */



/**
 *  Compute the arc sine of x; that is the value whose sine is x.
 *  @return the arc sine of x in radians. The return angle is in the
 *          range [-PI/2, PI/2].
 */
objectType flt_asin (listType arguments)

  { /* flt_asin */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        asin(take_float(arg_1(arguments))));
  } /* flt_asin */



/**
 *  Compute the arc tangent of x; that is the value whose tangent is x.
 *  @return the arc tangent of x in radians. The returned angle is in
 *          the range [-PI/2, PI/2].
 */
objectType flt_atan (listType arguments)

  { /* flt_atan */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        atan(take_float(arg_1(arguments))));
  } /* flt_atan */



/**
 *  Compute the arc tangent of y/x.
 *  The signs of x and y are used to determine the quadrant of the result.
 *  It determines the angle theta from the conversion of rectangular
 *  coordinates (x, y) to polar coordinates (r, theta).
 *  @return the arc tangent of y/x in radians. The returned angle is in
 *          the range [-PI, PI].
 */
objectType flt_atan2 (listType arguments)

  { /* flt_atan2 */
    isit_float(arg_1(arguments));
    isit_float(arg_2(arguments));
    return bld_float_temp(
        atan2(take_float(arg_1(arguments)), take_float(arg_2(arguments))));
  } /* flt_atan2 */



/**
 *  Get a float from bits in IEEE 754 double-precision representation.
 *  @param bits(arg_1 Bits to be converted to a float.
 *  @return a float from bits in double-precision float representation.
 */
objectType flt_bits2double (listType arguments)

  {
    double2BitsType conv;
    floatType number;

  /* flt_bits2double */
    isit_int(arg_1(arguments));
    conv.bits = (uintType) take_int(arg_1(arguments));
    number = conv.aDouble;
    logFunction(printf("flt_bits2double(" FMT_D ") --> " FMT_E "\n",
                       take_int(arg_1(arguments)), number););
    return bld_float_temp(number);
  } /* flt_bits2double */



/**
 *  Get a float from bits in IEEE 754 single-precision representation.
 *  @param bits/arg_1 Bits to be converted to a float.
 *  @return a float from bits in single-precision float representation.
 */
objectType flt_bits2single (listType arguments)

  {
    intType bits;
    float2BitsType conv;
    floatType number;

  /* flt_bits2single */
    isit_int(arg_1(arguments));
    bits = take_int(arg_1(arguments));
    if (unlikely((uintType) bits > UINT32TYPE_MAX)) {
      logError(printf("flt_bits2single(" FMT_D
                      "): Argument does not fit in 32 bits.\n",
                      bits););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      conv.bits = (uint32Type) bits;
      number = conv.aFloat;
      logFunction(printf("flt_bits2single(" FMT_D ") --> " FMT_E "\n",
                         bits, number););
      return bld_float_temp(number);
    } /* if */
  } /* flt_bits2single */



objectType flt_cast (listType arguments)

  { /* flt_cast */
    isit_float(arg_3(arguments));
    /* The float value is taken as int on purpose */
    return bld_int_temp(take_int(arg_3(arguments)));
  } /* flt_cast */



/**
 *  Round up towards positive infinity.
 *  Determine the smallest value that is greater than or equal
 *  to the argument and is equal to a mathematical integer.
 *  @return the rounded value.
 */
objectType flt_ceil (listType arguments)

  { /* flt_ceil */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        ceil(take_float(arg_1(arguments))));
  } /* flt_ceil */



/**
 *  Compare two float numbers.
 *  Because flt_cmp is used to sort float values, a unique
 *  sort sequence of all values is needed. Therefore flt_cmp
 *  considers NaN as equal to itself and greater than Infinity.
 *  Negative zero (-0.0) is considered by flt_cmp to be equal to
 *  positive zero (+0.0). This conforms to the behavior of all
 *  other float comparisons with zero.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objectType flt_cmp (listType arguments)

  { /* flt_cmp */
    isit_float(arg_1(arguments));
    isit_float(arg_2(arguments));
    return bld_int_temp(
        fltCmp(take_float(arg_1(arguments)), take_float(arg_2(arguments))));
  } /* flt_cmp */



/**
 *  Compute the cosine of x, where x is given in radians.
 *  @return the trigonometric cosine of an angle.
 */
objectType flt_cos (listType arguments)

  { /* flt_cos */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        cos(take_float(arg_1(arguments))));
  } /* flt_cos */



/**
 *  Compute the hyperbolic cosine of x.
 *  cosh(x) is mathematically defined as: (exp(x) + exp(-x)) / 2.0
 *  @return the hyperbolic cosine.
 */
objectType flt_cosh (listType arguments)

  { /* flt_cosh */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        cosh(take_float(arg_1(arguments))));
  } /* flt_cosh */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType flt_cpy (listType arguments)

  {
    objectType dest;

  /* flt_cpy */
    dest = arg_1(arguments);
    isit_float(dest);
    is_variable(dest);
    isit_float(arg_3(arguments));
    dest->value.floatValue = take_float(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* flt_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType flt_create (listType arguments)

  { /* flt_create */
    isit_float(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), FLOATOBJECT);
    arg_1(arguments)->value.floatValue = take_float(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* flt_create */



/**
 *  Decompose float into normalized fraction and integral exponent for 2.
 *  If the argument (number) is 0.0, -0.0, Infinity, -Infinity or NaN the
 *  fraction is set to the argument and the exponent is set to 0.
 *  For all other arguments the fraction is set to an absolute value
 *  between 0.5(included) and 1.0(excluded) and the exponent is set such
 *  that number = fraction * 2.0 ** exponent holds.
 *  @param number Number to be decomposed into fraction and exponent.
 *  @return floatElements with fraction and exponent set.
 */
objectType flt_decompose (listType arguments)

  {
    objectType fraction_var;
    objectType exponent_var;
    floatType number;

  /* flt_decompose */
    isit_float(arg_1(arguments));
    fraction_var = arg_2(arguments);
    isit_float(fraction_var);
    is_variable(fraction_var);
    exponent_var = arg_3(arguments);
    isit_int(exponent_var);
    is_variable(exponent_var);
    number = take_float(arg_1(arguments));
    logFunction(printf("flt_decompose(" FMT_E ", ...)\n", number););
#if FREXP_FUNCTION_OKAY
    {
      int exponent;

      fraction_var->value.floatValue = frexp(number, &exponent);
      exponent_var->value.intValue = (intType) exponent;
    }
#else
    fraction_var->value.floatValue =
        fltDecompose(number, &exponent_var->value.intValue);
#endif
    logFunction(printf("flt_decompose --> " FMT_E ", " FMT_D "\n",
                       fraction_var->value.floatValue,
                       exponent_var->value.intValue););
    return SYS_EMPTY_OBJECT;
  } /* flt_decompose */



/**
 *  Convert a float to a string in decimal fixed point notation.
 *  The number is rounded to the specified number of digits ('precision').
 *  Halfway cases are rounded away from zero. Except for a 'precision' of
 *  zero the representation has a decimal point and at least one digit
 *  before and after the decimal point. Negative numbers are preceded by
 *  a minus sign (e.g.: "-1.25"). If all digits in the result are 0 a
 *  possible negative sign is omitted.
 *  @param precision/arg_3 Number of digits after the decimal point.
 *         If the 'precision' is zero the decimal point is omitted.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR If the 'precision' is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType flt_dgts (listType arguments)

  { /* flt_dgts */
    isit_float(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        fltDgts(take_float(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* flt_dgts */



/**
 *  Compute the division of two float numbers.
 *  @return the quotient of the division.
 */
objectType flt_div (listType arguments)

  {
    floatType dividend;
    floatType divisor;

  /* flt_div */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    dividend = take_float(arg_1(arguments));
    divisor = take_float(arg_3(arguments));
#if CHECK_FLOAT_DIV_BY_ZERO
#if FLOAT_NAN_COMPARISON_OKAY
    if (divisor == 0.0) {
#else
    if (!os_isnan(divisor) && divisor == 0.0) {
#endif
      if (dividend == 0.0 || os_isnan(dividend)) {
        return bld_float_temp(NOT_A_NUMBER);
      } else if ((dividend < 0.0) == fltIsNegativeZero(divisor)) {
        return bld_float_temp(POSITIVE_INFINITY);
      } else {
        return bld_float_temp(NEGATIVE_INFINITY);
      } /* if */
    } /* if */
#endif
    return bld_float_temp(((double) dividend) / ((double) divisor));
  } /* flt_div */



/**
 *  Divide a float 'number' by a 'divisor' and assign the result back to 'number'.
 */
objectType flt_div_assign (listType arguments)

  {
    objectType flt_variable;
#if CHECK_FLOAT_DIV_BY_ZERO
    floatType dividend;
#endif
    floatType divisor;

  /* flt_div_assign */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    divisor = take_float(arg_3(arguments));
#if CHECK_FLOAT_DIV_BY_ZERO
#if FLOAT_NAN_COMPARISON_OKAY
    if (divisor == 0.0) {
#else
    if (!os_isnan(divisor) && divisor == 0.0) {
#endif
      dividend = take_float(flt_variable);
      if (dividend == 0.0 || os_isnan(dividend)) {
        flt_variable->value.floatValue = NOT_A_NUMBER;
      } else if ((dividend < 0.0) == fltIsNegativeZero(divisor)) {
        flt_variable->value.floatValue = POSITIVE_INFINITY;
      } else {
        flt_variable->value.floatValue = NEGATIVE_INFINITY;
      } /* if */
    } else {
      flt_variable->value.floatValue /= divisor;
    } /* if */
#else
    flt_variable->value.floatValue /= divisor;
#endif
    return SYS_EMPTY_OBJECT;
  } /* flt_div_assign */



/**
 *  Get bits in IEEE 754 double-precision representation from a float.
 *  @param number/arg_1 Float value to be converted to bin64.
 *  @return 64 bits in IEEE 754 double-precision float representation.
 */
objectType flt_double2bits (listType arguments)

  {
    floatType number;
    double2BitsType conv;
    intType bits;

  /* flt_double2bits */
    isit_float(arg_1(arguments));
    number = take_float(arg_1(arguments));
    conv.aDouble = number;
    bits = (intType) (uintType) conv.bits;
    logFunction(printf("flt_double2bits(" FMT_E ") --> " FMT_D "\n",
                       number, bits););
    return bld_int_temp(bits);
  } /* flt_double2bits */



/**
 *  Check if two float numbers are equal.
 *  According to IEEE 754 a NaN is not equal to any float value.
 *  Therefore 'NaN = any_value' and 'any_value = NaN'
 *  always return FALSE. Even 'NaN = NaN' returns FALSE.
 *  @return TRUE if both numbers are equal, FALSE otherwise.
 */
objectType flt_eq (listType arguments)

  { /* flt_eq */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#if FLOAT_COMPARISON_OKAY
    if (take_float(arg_1(arguments)) ==
        take_float(arg_3(arguments))) {
#else
    if (fltEq(take_float(arg_1(arguments)),
              take_float(arg_3(arguments)))) {
#endif
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_eq */



/**
 *  Compute Euler's number e raised to the power of x.
 *  @return e raised to the power of x.
 */
objectType flt_exp (listType arguments)

  { /* flt_exp */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        fltExp(take_float(arg_1(arguments))));
  } /* flt_exp */



/**
 *  Round down towards negative infinity.
 *  Returns the largest value that is less than or equal to the
 *  argument and is equal to a mathematical integer.
 *  @return the rounded value.
 */
objectType flt_floor (listType arguments)

  { /* flt_floor */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        floor(take_float(arg_1(arguments))));
  } /* flt_floor */



/**
 *  Check if 'number1' is greater than or equal to 'number2'.
 *  According to IEEE 754 a NaN is neither less than,
 *  equal to, nor greater than any value, including itself.
 *  If 'number1' or 'number2' is NaN, the result is FALSE.
 *  @return TRUE if 'number1' is greater than or equal to 'number2',
 *          FALSE otherwise.
 */
objectType flt_ge (listType arguments)

  { /* flt_ge */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#if FLOAT_COMPARISON_OKAY
    if (take_float(arg_1(arguments)) >=
        take_float(arg_3(arguments))) {
#else
    if (fltGe(take_float(arg_1(arguments)),
              take_float(arg_3(arguments)))) {
#endif
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_ge */



/**
 *  Check if 'number1' is greater than 'number2'.
 *  According to IEEE 754 a NaN is neither less than,
 *  equal to, nor greater than any value, including itself.
 *  If 'number1' or 'number2' is NaN, the result is FALSE.
 *  @return TRUE if 'number1' is greater than 'number2',
 *          FALSE otherwise.
 */
objectType flt_gt (listType arguments)

  { /* flt_gt */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#if FLOAT_COMPARISON_OKAY
    if (take_float(arg_1(arguments)) >
        take_float(arg_3(arguments))) {
#else
    if (fltGt(take_float(arg_1(arguments)),
              take_float(arg_3(arguments)))) {
#endif
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_gt */



/**
 *  Compute the hash value of a float number.
 *  @return the hash value.
 */
objectType flt_hashcode (listType arguments)

  { /* flt_hashcode */
    isit_float(arg_1(arguments));
    /* The float value is taken as int on purpose */
    return bld_int_temp(take_int(arg_1(arguments)));
  } /* flt_hashcode */



objectType flt_icast (listType arguments)

  { /* flt_icast */
    isit_int(arg_3(arguments));
    /* The int value is taken as float on purpose */
    return bld_float_temp(take_float(arg_3(arguments)));
  } /* flt_icast */



/**
 *  Convert an integer to a float.
 *  @return the float result of the conversion.
 */
objectType flt_iconv1 (listType arguments)

  { /* flt_iconv1 */
    isit_int(arg_1(arguments));
    return bld_float_temp((double) take_int(arg_1(arguments)));
  } /* flt_iconv1 */



/**
 *  Convert an integer to a float.
 *  @return the float result of the conversion.
 */
objectType flt_iconv3 (listType arguments)

  { /* flt_iconv3 */
    isit_int(arg_3(arguments));
    return bld_float_temp((double) take_int(arg_3(arguments)));
  } /* flt_iconv3 */



/**
 *  Compute the exponentiation of a float 'base' with an integer 'exponent'.
 *     A    ** 0  returns 1.0
 *     NaN  ** 0  returns 1.0
 *     NaN  ** B  returns NaN              for B <> 0
 *     0.0  ** B  returns 0.0              for B > 0
 *     0.0  ** 0  returns 1.0
 *     0.0  ** B  returns Infinity         for B < 0
 *   (-0.0) ** B  returns -Infinity        for B < 0 and odd(B)
 *     A    ** B  returns 1.0 / A ** (-B)  for B < 0
 *  @return the result of the exponentiation.
 */
objectType flt_ipow (listType arguments)

  { /* flt_ipow */
    isit_float(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_float_temp(
        fltIPow(take_float(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* flt_ipow */



/**
 *  Determine if a number has a Not-a-Number (NaN) value.
 *  NaN represents an undefined or unrepresentable value.
 *  @return TRUE if the number has a Not-a-Number (NaN) value,
 *          FALSE otherwise.
 */
objectType flt_isnan (listType arguments)

  { /* flt_isnan */
    isit_float(arg_1(arguments));
    if (os_isnan(take_float(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_isnan */



/**
 *  Determine if a number is -0.0.
 *  This function is the only possibility to determine if a number
 *  is -0.0. The comparison operators (=, <>, <, >, <=, >=) and
 *  the function 'compare' treat 0.0 and -0.0 as equal. The
 *  operators ''digits'' and ''sci'' and the function ''str''
 *  return the same [[string]] for -0.0 and +0.0.
 *  @return TRUE if the number is -0.0,
 *          FALSE otherwise.
 */
objectType flt_isnegativezero (listType arguments)

  { /* flt_isnegativezero */
    isit_float(arg_1(arguments));
    if (fltIsNegativeZero(take_float(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_isnegativezero */



/**
 *  Check if 'number1' is less than or equal to 'number2'.
 *  According to IEEE 754 a NaN is neither less than,
 *  equal to, nor greater than any value, including itself.
 *  If 'number1' or 'number2' is NaN, the result is FALSE.
 *  @return TRUE if 'number1' is less than or equal to 'number2',
 *          FALSE otherwise.
 */
objectType flt_le (listType arguments)

  { /* flt_le */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#if FLOAT_COMPARISON_OKAY
    if (take_float(arg_1(arguments)) <=
        take_float(arg_3(arguments))) {
#else
    if (fltLe(take_float(arg_1(arguments)),
              take_float(arg_3(arguments)))) {
#endif
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_le */



/**
 *  Return the natural logarithm (base e) of x.
 *  @return the natural logarithm of x.
 */
objectType flt_log (listType arguments)

  {
    floatType logarithm;

  /* flt_log */
    isit_float(arg_1(arguments));
    logarithm = fltLog(take_float(arg_1(arguments)));
    return bld_float_temp(logarithm);
  } /* flt_log */



/**
 *  Returns the base 10 logarithm of x.
 *  @return the base 10 logarithm of x.
 */
objectType flt_log10 (listType arguments)

  {
    floatType logarithm;

  /* flt_log10 */
    isit_float(arg_1(arguments));
    logarithm = fltLog10(take_float(arg_1(arguments)));
    return bld_float_temp(logarithm);
  } /* flt_log10 */



/**
 *  Returns the base 2 logarithm of x.
 *  @return the base 2 logarithm of x.
 */
objectType flt_log2 (listType arguments)

  {
    floatType logarithm;

  /* flt_log2 */
    isit_float(arg_1(arguments));
    logarithm = fltLog2(take_float(arg_1(arguments)));
    return bld_float_temp(logarithm);
  } /* flt_log2 */



/**
 *  Multiply number/arg_1 by 2 raised to the power of exponent/arg_3.
 *  In other words: A << B is equivalent to A * 2.0 ** B
 *  If the result underflows zero is returned.
 *  If the result overflows Infinity or -Infinity is returned,
 *  depending on the sign of number/arg_1.
 *  If the argument number/arg_1 is a NaN, Infinity or -Infinity the
 *  unchanged argument is returned.
 *  @return number * 2.0 ** exponent
 */
objectType flt_lshift (listType arguments)

  {
    intType lshift;
    floatType shifted;

  /* flt_lshift */
    isit_float(arg_1(arguments));
    isit_int(arg_3(arguments));
    lshift = take_int(arg_3(arguments));
    logFunction(printf("flt_lshift(" FMT_E ", " FMT_D ")\n",
                       take_float(arg_1(arguments)), lshift););
#if INT_SIZE < INTTYPE_SIZE
    if (unlikely(lshift > INT_MAX)) {
      lshift = INT_MAX;
    } else if (unlikely(lshift < INT_MIN)) {
      lshift = INT_MIN;
    } /* if */
#endif
    shifted = fltLdexp(take_float(arg_1(arguments)), (int) lshift);
    logFunction(printf("flt_lshift --> " FMT_E "\n", shifted););
    return bld_float_temp(shifted);
  } /* flt_lshift */



/**
 *  Check if 'number1' is less than 'number2'.
 *  According to IEEE 754 a NaN is neither less than,
 *  equal to, nor greater than any value, including itself.
 *  If 'number1' or 'number2' is NaN, the result is FALSE.
 *  @return TRUE if 'number1' is less than 'number2',
 *          FALSE otherwise.
 */
objectType flt_lt (listType arguments)

  { /* flt_lt */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#if FLOAT_COMPARISON_OKAY
    if (take_float(arg_1(arguments)) <
        take_float(arg_3(arguments))) {
#else
    if (fltLt(take_float(arg_1(arguments)),
              take_float(arg_3(arguments)))) {
#endif
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_lt */



/**
 *  Compute the floating-point modulo of a division.
 *  The modulo has the same sign as the divisor.
 *  The modulo is dividend - floor(dividend / divisor) * divisor
 *    A        mod  NaN       returns  NaN
 *    NaN      mod  B         returns  NaN
 *    A        mod  0.0       returns  NaN
 *    Infinity mod  B         returns  NaN
 *   -Infinity mod  B         returns  NaN
 *    0.0      mod  B         returns  0.0         for B &lt;> 0.0
 *    A        mod  Infinity  returns  A           for A > 0
 *    A        mod  Infinity  returns  Infinity    for A < 0
 *    A        mod -Infinity  returns  A           for A < 0
 *    A        mod -Infinity  returns -Infinity    for A > 0
 *  @return the floating-point modulo of the division.
 */
objectType flt_mod (listType arguments)

  { /* flt_mod */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return bld_float_temp(
        fltMod(take_float(arg_1(arguments)), take_float(arg_3(arguments))));
  } /* flt_mod */



/**
 *  Multiply two float numbers.
 *  @return the product of the two numbers.
 */
objectType flt_mult (listType arguments)

  { /* flt_mult */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return bld_float_temp(
        (double) take_float(arg_1(arguments)) *
        (double) take_float(arg_3(arguments)));
  } /* flt_mult */



/**
 *  Multiply a float 'number' by a 'factor' and assign the result back to 'number'.
 */
objectType flt_mult_assign (listType arguments)

  {
    objectType flt_variable;

  /* flt_mult_assign */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    flt_variable->value.floatValue *= take_float(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* flt_mult_assign */



/**
 *  Check if two float numbers are not equal.
 *  According to IEEE 754 a NaN is not equal to any float value.
 *  Therefore 'NaN <> any_value' and 'any_value <> NaN'
 *  always return TRUE. Even 'NaN <> NaN' returns TRUE.
 *  @return FALSE if both numbers are equal, TRUE otherwise.
 */
objectType flt_ne (listType arguments)

  { /* flt_ne */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#if FLOAT_COMPARISON_OKAY
    if (take_float(arg_1(arguments)) !=
        take_float(arg_3(arguments))) {
#else
    if (!fltEq(take_float(arg_1(arguments)),
               take_float(arg_3(arguments)))) {
#endif
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_ne */



/**
 *  Minus sign, negate a float 'number'.
 *  @return the negated value of the number.
 */
objectType flt_negate (listType arguments)

  { /* flt_negate */
    isit_float(arg_2(arguments));
    return bld_float_temp(
        -take_float(arg_2(arguments)));
  } /* flt_negate */



/**
 *  Convert a string to a float number.
 *  @return the float result of the conversion.
 *  @exception RANGE_ERROR If the string contains not a float literal.
 */
objectType flt_parse1 (listType arguments)

  { /* flt_parse1 */
    isit_stri(arg_1(arguments));
    return bld_float_temp(
        fltParse(take_stri(arg_1(arguments))));
  } /* flt_parse1 */



/**
 *  Plus sign for float numbers.
 *  @return its operand unchanged.
 */
objectType flt_plus (listType arguments)

  { /* flt_plus */
    isit_float(arg_2(arguments));
    return bld_float_temp((double) take_float(arg_2(arguments)));
  } /* flt_plus */



/**
 *  Compute the exponentiation of a float 'base' with a float 'exponent'.
 *     A    ** B    returns NaN        for A < 0.0 and B is not integer
 *     A    ** 0.0  returns 1.0
 *     NaN  ** 0.0  returns 1.0
 *     NaN  ** B    returns NaN        for B <> 0.0
 *     0.0  ** B    returns 0.0        for B > 0.0
 *     0.0  ** 0.0  returns 1.0
 *     0.0  ** B    returns Infinity   for B < 0.0
 *   (-0.0) ** B    returns -Infinity  for B < 0.0 and odd(B)
 *     1.0  ** B    returns 1.0
 *     1.0  ** NaN  returns 1.0
 *     A    ** NaN  returns NaN        for A <> 1.0
 *  @return the result of the exponentiation.
 */
objectType flt_pow (listType arguments)

  {
    floatType power;

  /* flt_pow */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    power = fltPow(take_float(arg_1(arguments)), take_float(arg_3(arguments)));
    return bld_float_temp(power);
  } /* flt_pow */



/**
 *  Compute pseudo-random number in the range [low, high].
 *  The random values are uniform distributed.
 *  @return the computed pseudo-random number.
 *  @exception RANGE_ERROR The range is empty (low > high holds).
 */
objectType flt_rand (listType arguments)

  { /* flt_rand */
    isit_float(arg_1(arguments));
    isit_float(arg_2(arguments));
    return bld_float_temp(
        fltRand(take_float(arg_1(arguments)), take_float(arg_2(arguments))));
  } /* flt_rand */



/**
 *  Compute the floating-point remainder of a division.
 *  The remainder has the same sign as the dividend.
 *  The remainder is dividend - flt(trunc(dividend / divisor)) * divisor
 *  The remainder is computed without a conversion to integer.
 *    A        rem NaN       returns NaN
 *    NaN      rem B         returns NaN
 *    A        rem 0.0       returns NaN
 *    Infinity rem B         returns NaN
 *   -Infinity rem B         returns NaN
 *    0.0      rem B         returns 0.0  for B &lt;> 0.0
 *    A        rem Infinity  returns A
 *  @return the floating-point remainder of the division.
 */
objectType flt_rem (listType arguments)

  { /* flt_rem */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return bld_float_temp(
        fltRem(take_float(arg_1(arguments)), take_float(arg_3(arguments))));
  } /* flt_rem */



/**
 *  Round towards the nearest integer.
 *  Halfway cases are rounded away from zero.
 *  @return the rounded value.
 *  @exception RANGE_ERROR If the number is NaN, -Infinity, Infinity,
 *             or does not fit into an integer.
 */
objectType flt_round (listType arguments)

  {
    floatType number;
    intType rounded;

  /* flt_round */
    isit_float(arg_1(arguments));
    number = take_float(arg_1(arguments));
    logFunction(printf("flt_round(" FMT_E ")\n", number););
    if (unlikely(os_isnan(number) ||
                 number < (floatType) minimumTruncArgument ||
                 number > (floatType) INT_SUFFIX(MAXIMUM_TRUNC_ARGUMENT))) {
      logError(printf("flt_round(" FMT_E "): "
                      "Number does not fit into an integer.\n",
                      number););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      if (number < (floatType) 0.0) {
        rounded = (intType) (number - 0.5);
      } else {
        rounded = (intType) (number + 0.5);
      } /* if */
      logFunction(printf("flt_round --> " FMT_D "\n", rounded););
      return bld_int_temp(rounded);
    } /* if */
  } /* flt_round */



/**
 *  Divide number/arg_1 by 2 raised to the power of exponent/arg_3.
 *  In other words: A >> B is equivalent to A / 2.0 ** B
 *  If the result underflows zero is returned.
 *  If the result overflows Infinity or -Infinity is returned,
 *  depending on the sign of number/arg_1.
 *  If the argument number/arg_1 is a NaN, Infinity or -Infinity the
 *  unchanged argument is returned.
 *  @return number / 2.0 ** exponent
 */
objectType flt_rshift (listType arguments)

  {
    intType rshift;
    floatType shifted;

  /* flt_rshift */
    isit_float(arg_1(arguments));
    isit_int(arg_3(arguments));
    rshift = take_int(arg_3(arguments));
    logFunction(printf("flt_rshift(" FMT_E ", " FMT_D ")\n",
                       take_float(arg_1(arguments)), rshift););
#if INT_SIZE < INTTYPE_SIZE
    if (unlikely(rshift > INT_MAX)) {
      rshift = INT_MAX;
    } else
#endif
#if INT_SIZE <= INTTYPE_SIZE
    if (unlikely(rshift <= INT_MIN)) {
      /* Avoid that negating the rshift overflows in     */
      /* case we have twos complement integers. Changing */
      /* the rshift does not change the result.          */
      rshift = -INT_MAX;
    } /* if */
#endif
    shifted = fltLdexp(take_float(arg_1(arguments)), (int) -rshift);
    logFunction(printf("flt_rshift --> " FMT_E "\n", shifted););
    return bld_float_temp(shifted);
  } /* flt_rshift */



/**
 *  Compute the subtraction of two float numbers.
 *  @return the difference of the two numbers.
 */
objectType flt_sbtr (listType arguments)

  { /* flt_sbtr */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return bld_float_temp(
        (double) take_float(arg_1(arguments)) -
        (double) take_float(arg_3(arguments)));
  } /* flt_sbtr */



/**
 *  Decrement a float 'number' by a 'delta'.
 */
objectType flt_sbtr_assign (listType arguments)

  {
    objectType flt_variable;

  /* flt_sbtr_assign */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    flt_variable->value.floatValue -= take_float(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* flt_sbtr_assign */



/**
 *  Convert a 'float' number to a [[string]] in scientific notation.
 *  Scientific notation uses a decimal significand and a decimal exponent.
 *  The significand has an optional sign and exactly one digit before the
 *  decimal point. The fractional part of the significand is rounded
 *  to the specified number of digits ('precision'). Halfway cases are
 *  rounded away from zero. The fractional part is followed by the
 *  letter e and an exponent, which is always signed. The value zero is
 *  never written with a negative sign.
 *  @param precision/arg_3 Number of digits after the decimal point.
 *         If the 'precision' is zero the decimal point is omitted.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR If the 'precision' is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType flt_sci (listType arguments)

  { /* flt_sci */
    isit_float(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        fltSci(take_float(arg_1(arguments)),
               take_int(arg_3(arguments))));
  } /* flt_sci */



/**
 *  Compute the sine of x, where x is given in radians.
 *  @return the trigonometric sine of an angle.
 */
objectType flt_sin (listType arguments)

  { /* flt_sin */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        sin(take_float(arg_1(arguments))));
  } /* flt_sin */



/**
 *  Get bits in IEEE 754 single-precision representation from a float.
 *  @param number/arg_1 Float value to be converted to bin32.
 *  @return 32 bits in IEEE 754 single-precision float representation.
 */
objectType flt_single2bits (listType arguments)

  {
    floatType number;
    float2BitsType conv;
    intType bits;

  /* flt_single2bits */
    isit_float(arg_1(arguments));
    number = take_float(arg_1(arguments));
    conv.aFloat = (float) number;
    bits = (intType) (uintType) conv.bits;
    logFunction(printf("flt_single2bits(" FMT_E ") --> " FMT_D "\n",
                       number, bits););
    return bld_int_temp(bits);;
  } /* flt_single2bits */



/**
 *  Compute the hyperbolic sine of x.
 *  sinh(x) is mathematically defined as: (exp(x) - exp(-x)) / 2.0
 *  @return the hyperbolic sine.
 */
objectType flt_sinh (listType arguments)

  { /* flt_sinh */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        sinh(take_float(arg_1(arguments))));
  } /* flt_sinh */



/**
 *  Returns the non-negative square root of x.
 *  @return the square root of x.
 */
objectType flt_sqrt (listType arguments)

  {
    floatType squareRoot;

  /* flt_sqrt */
    isit_float(arg_1(arguments));
    squareRoot = fltSqrt(take_float(arg_1(arguments)));
    return bld_float_temp(squareRoot);
  } /* flt_sqrt */



/**
 *  Convert a float number to a string.
 *  The number is converted to a string with decimal representation.
 *  The result string has the style [-]ddd.ddd where there is at least
 *  one digit before and after the decimal point. The number of digits
 *  after the decimal point is determined automatically. Except for the
 *  case if there is only one zero digit after the decimal point,
 *  the last digit is never zero. Negative zero (-0.0) and positive
 *  zero (+0.0) are both converted to "0.0".
 *   str(16.125)    returns "16.125"
 *   str(-0.0)      returns "0.0"
 *   str(Infinity)  returns "Infinity"
 *   str(-Infinity) returns "-Infinity"
 *   str(NaN)       returns "NaN"
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType flt_str (listType arguments)

  { /* flt_str */
    isit_float(arg_1(arguments));
    return bld_stri_temp(
        fltStr(take_float(arg_1(arguments))));
  } /* flt_str */



/**
 *  Compute the tangent of x, where x is given in radians.
 *  @return the trigonometric tangent of an angle.
 */
objectType flt_tan (listType arguments)

  { /* flt_tan */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        tan(take_float(arg_1(arguments))));
  } /* flt_tan */



/**
 *  Compute the hyperbolic tangent of x.
 *  tanh(x) is mathematically defined as: sinh(x) / cosh(x)
 *  @return the hyperbolic tangent.
 */
objectType flt_tanh (listType arguments)

  { /* flt_tanh */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        tanh(take_float(arg_1(arguments))));
  } /* flt_tanh */



/**
 *  Truncate towards zero.
 *  The fractional part of a number is discarded.
 *  @return the nearest integer not larger in absolute value
 *          than the argument.
 *  @exception RANGE_ERROR If the number is NaN, -Infinity, Infinity,
 *             or does not fit into an integer.
 */
objectType flt_trunc (listType arguments)

  {
    floatType number;

  /* flt_trunc */
    isit_float(arg_1(arguments));
    number = take_float(arg_1(arguments));
    logFunction(printf("flt_trunc(" FMT_E ")\n", number););
    if (unlikely(os_isnan(number) ||
                 number < (floatType) minimumTruncArgument ||
                 number > (floatType) INT_SUFFIX(MAXIMUM_TRUNC_ARGUMENT))) {
      logError(printf("flt_trunc(" FMT_E "): "
                      "Number does not fit into an integer.\n",
                      number););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      logFunction(printf("flt_trunc --> " FMT_D "\n", (intType) number););
      return bld_int_temp((intType) number);
    } /* if */
  } /* flt_trunc */



/**
 *  Get 'float' value of the object referenced by 'aReference/arg_1'.
 *  @return the 'float' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference/arg_1' is NIL or
 *             category(aReference) <> FLOATOBJECT holds.
 */
objectType flt_value (listType arguments)

  {
    objectType aReference;

  /* flt_value */
    isit_reference(arg_1(arguments));
    aReference = take_reference(arg_1(arguments));
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != FLOATOBJECT)) {
      logError(printf("flt_value(");
               trace1(aReference);
               printf("): Category is not FLOATOBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_float_temp(take_float(aReference));
    } /* if */
  } /* flt_value */
