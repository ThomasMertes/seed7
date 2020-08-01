/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: All primitive actions for the float type.              */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "math.h"
#include "float.h"

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



objecttype flt_a2tan (listtype arguments)

  { /* flt_a2tan */
    isit_float(arg_1(arguments));
    isit_float(arg_2(arguments));
    return bld_float_temp(
        atan2(take_float(arg_1(arguments)), take_float(arg_2(arguments))));
  } /* flt_a2tan */



objecttype flt_abs (listtype arguments)

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



objecttype flt_acos (listtype arguments)

  { /* flt_acos */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        acos(take_float(arg_1(arguments))));
  } /* flt_acos */



objecttype flt_add (listtype arguments)

  { /* flt_add */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return bld_float_temp(
        (double) take_float(arg_1(arguments)) +
        (double) take_float(arg_3(arguments)));
  } /* flt_add */



objecttype flt_asin (listtype arguments)

  { /* flt_asin */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        asin(take_float(arg_1(arguments))));
  } /* flt_asin */



objecttype flt_atan (listtype arguments)

  { /* flt_atan */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        atan(take_float(arg_1(arguments))));
  } /* flt_atan */



objecttype flt_cast (listtype arguments)

  { /* flt_cast */
    isit_float(arg_3(arguments));
    /* The float value is taken as int on purpose */
    return bld_int_temp(take_int(arg_3(arguments)));
  } /* flt_cast */



objecttype flt_ceil (listtype arguments)

  { /* flt_ceil */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        ceil(take_float(arg_1(arguments))));
  } /* flt_ceil */



objecttype flt_cmp (listtype arguments)

  { /* flt_cmp */
    isit_float(arg_1(arguments));
    isit_float(arg_2(arguments));
    return bld_int_temp(
        fltCmp(take_float(arg_1(arguments)), take_float(arg_2(arguments))));
  } /* flt_cmp */



objecttype flt_cos (listtype arguments)

  { /* flt_cos */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        cos(take_float(arg_1(arguments))));
  } /* flt_cos */



objecttype flt_cosh (listtype arguments)

  { /* flt_cosh */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        cosh(take_float(arg_1(arguments))));
  } /* flt_cosh */



objecttype flt_cpy (listtype arguments)

  {
    objecttype flt_variable;

  /* flt_cpy */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    flt_variable->value.floatvalue = take_float(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* flt_cpy */



objecttype flt_create (listtype arguments)

  { /* flt_create */
    isit_float(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), FLOATOBJECT);
    arg_1(arguments)->value.floatvalue = take_float(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* flt_create */



objecttype flt_dgts (listtype arguments)

  { /* flt_dgts */
    isit_float(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        fltDgts(take_float(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* flt_dgts */



objecttype flt_div (listtype arguments)

  {
    floattype dividend;
    floattype divisor;

  /* flt_div */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    dividend = take_float(arg_1(arguments));
    divisor = take_float(arg_3(arguments));
#ifdef CHECK_FLOAT_DIV_BY_ZERO
    if (divisor == 0.0) {
      if (dividend == 0.0 || isnan(dividend)) {
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



objecttype flt_div_assign (listtype arguments)

  {
    objecttype flt_variable;
#ifdef CHECK_FLOAT_DIV_BY_ZERO
    floattype dividend;
#endif
    floattype divisor;

  /* flt_div_assign */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    divisor = take_float(arg_3(arguments));
#ifdef CHECK_FLOAT_DIV_BY_ZERO
    if (divisor == 0.0) {
      dividend = take_float(flt_variable);
      if (dividend == 0.0 || isnan(dividend)) {
        flt_variable->value.floatvalue = NOT_A_NUMBER;
      } else if ((dividend < 0.0) == fltIsNegativeZero(divisor)) {
        flt_variable->value.floatvalue = POSITIVE_INFINITY;
      } else {
        flt_variable->value.floatvalue = NEGATIVE_INFINITY;
      } /* if */
    } else {
      flt_variable->value.floatvalue /= divisor;
    } /* if */
#else
    flt_variable->value.floatvalue /= divisor;
#endif
    return SYS_EMPTY_OBJECT;
  } /* flt_div_assign */



objecttype flt_eq (listtype arguments)

  { /* flt_eq */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#ifdef NAN_COMPARISON_WRONG
    if (fltEq(take_float(arg_1(arguments)),
              take_float(arg_3(arguments)))) {
#else
    if (take_float(arg_1(arguments)) ==
        take_float(arg_3(arguments))) {
#endif
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_eq */



objecttype flt_exp (listtype arguments)

  { /* flt_exp */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        exp(take_float(arg_1(arguments))));
  } /* flt_exp */



objecttype flt_floor (listtype arguments)

  { /* flt_floor */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        floor(take_float(arg_1(arguments))));
  } /* flt_floor */



objecttype flt_ge (listtype arguments)

  { /* flt_ge */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#ifdef NAN_COMPARISON_WRONG
    if (fltGe(take_float(arg_1(arguments)),
              take_float(arg_3(arguments)))) {
#else
    if (take_float(arg_1(arguments)) >=
        take_float(arg_3(arguments))) {
#endif
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_ge */



objecttype flt_grow (listtype arguments)

  {
    objecttype flt_variable;

  /* flt_grow */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    flt_variable->value.floatvalue += take_float(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* flt_grow */



objecttype flt_gt (listtype arguments)

  { /* flt_gt */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#ifdef NAN_COMPARISON_WRONG
    if (fltGt(take_float(arg_1(arguments)),
              take_float(arg_3(arguments)))) {
#else
    if (take_float(arg_1(arguments)) >
        take_float(arg_3(arguments))) {
#endif
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_gt */



objecttype flt_hashcode (listtype arguments)

  { /* flt_hashcode */
    isit_float(arg_1(arguments));
    /* The float value is taken as int on purpose */
    return bld_int_temp(take_int(arg_1(arguments)));
  } /* flt_hashcode */



objecttype flt_icast (listtype arguments)

  { /* flt_icast */
    isit_int(arg_3(arguments));
    /* The int value is taken as float on purpose */
    return bld_float_temp(take_float(arg_3(arguments)));
  } /* flt_icast */



objecttype flt_iconv (listtype arguments)

  { /* flt_iconv */
    isit_int(arg_3(arguments));
    return bld_float_temp((double) take_int(arg_3(arguments)));
  } /* flt_iconv */



objecttype flt_iflt (listtype arguments)

  { /* flt_iflt */
    isit_int(arg_1(arguments));
    return bld_float_temp((double) take_int(arg_1(arguments)));
  } /* flt_iflt */



objecttype flt_ipow (listtype arguments)

  { /* flt_ipow */
    isit_float(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_float_temp(
        fltIPow(take_float(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* flt_ipow */



objecttype flt_isnan (listtype arguments)

  { /* flt_isnan */
    isit_float(arg_1(arguments));
    if (isnan(take_float(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_isnan */



objecttype flt_isnegativezero (listtype arguments)

  { /* flt_isnegativezero */
    isit_float(arg_1(arguments));
    if (fltIsNegativeZero(take_float(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_isnegativezero */



objecttype flt_le (listtype arguments)

  { /* flt_le */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#ifdef NAN_COMPARISON_WRONG
    if (fltLe(take_float(arg_1(arguments)),
              take_float(arg_3(arguments)))) {
#else
    if (take_float(arg_1(arguments)) <=
        take_float(arg_3(arguments))) {
#endif
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_le */



objecttype flt_log (listtype arguments)

  { /* flt_log */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        log(take_float(arg_1(arguments))));
  } /* flt_log */



objecttype flt_log10 (listtype arguments)

  { /* flt_log10 */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        log10(take_float(arg_1(arguments))));
  } /* flt_log10 */



objecttype flt_lt (listtype arguments)

  { /* flt_lt */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#ifdef NAN_COMPARISON_WRONG
    if (fltLt(take_float(arg_1(arguments)),
              take_float(arg_3(arguments)))) {
#else
    if (take_float(arg_1(arguments)) <
        take_float(arg_3(arguments))) {
#endif
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_lt */



objecttype flt_minus (listtype arguments)

  { /* flt_minus */
    isit_float(arg_2(arguments));
    return bld_float_temp(
        -(double) take_float(arg_2(arguments)));
  } /* flt_minus */



objecttype flt_mult (listtype arguments)

  { /* flt_mult */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return bld_float_temp(
        (double) take_float(arg_1(arguments)) *
        (double) take_float(arg_3(arguments)));
  } /* flt_mult */



objecttype flt_mult_assign (listtype arguments)

  {
    objecttype flt_variable;

  /* flt_mult_assign */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    flt_variable->value.floatvalue *= take_float(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* flt_mult_assign */



objecttype flt_ne (listtype arguments)

  { /* flt_ne */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#ifdef NAN_COMPARISON_WRONG
    if (!fltEq(take_float(arg_1(arguments)),
               take_float(arg_3(arguments)))) {
#else
    if (take_float(arg_1(arguments)) !=
        take_float(arg_3(arguments))) {
#endif
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* flt_ne */



objecttype flt_parse (listtype arguments)

  { /* flt_parse */
    isit_stri(arg_3(arguments));
    return bld_float_temp(
        fltParse(take_stri(arg_3(arguments))));
  } /* flt_parse */



objecttype flt_plus (listtype arguments)

  { /* flt_plus */
    isit_float(arg_2(arguments));
    return bld_float_temp((double) take_float(arg_2(arguments)));
  } /* flt_plus */



objecttype flt_pow (listtype arguments)

  { /* flt_pow */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
#ifdef POWER_OF_ZERO_WRONG
    return bld_float_temp(
        fltPow(take_float(arg_1(arguments)), take_float(arg_3(arguments))));
#else
    return bld_float_temp(
        pow(take_float(arg_1(arguments)), take_float(arg_3(arguments))));
#endif
  } /* flt_pow */



objecttype flt_rand (listtype arguments)

  { /* flt_rand */
    isit_float(arg_1(arguments));
    isit_float(arg_2(arguments));
    return bld_float_temp(
        fltRand(take_float(arg_1(arguments)), take_float(arg_2(arguments))));
  } /* flt_rand */



objecttype flt_round (listtype arguments)

  {
    floattype number;

  /* flt_round */
    isit_float(arg_1(arguments));
    number = take_float(arg_1(arguments));
    if (number < (floattype) 0.0) {
      return bld_int_temp(-((inttype) (0.5 - number)));
    } else {
      return bld_int_temp((inttype) (0.5 + number));
    } /* if */
  } /* flt_round */



objecttype flt_sbtr (listtype arguments)

  { /* flt_sbtr */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return bld_float_temp(
        (double) take_float(arg_1(arguments)) -
        (double) take_float(arg_3(arguments)));
  } /* flt_sbtr */



objecttype flt_sci (listtype arguments)

  { /* flt_sci */
    isit_float(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        fltSci(take_float(arg_1(arguments)),
               take_int(arg_3(arguments))));
  } /* flt_sci */



objecttype flt_shrink (listtype arguments)

  {
    objecttype flt_variable;

  /* flt_shrink */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    flt_variable->value.floatvalue -= take_float(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* flt_shrink */



objecttype flt_sin (listtype arguments)

  { /* flt_sin */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        sin(take_float(arg_1(arguments))));
  } /* flt_sin */



objecttype flt_sinh (listtype arguments)

  { /* flt_sinh */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        sinh(take_float(arg_1(arguments))));
  } /* flt_sinh */



objecttype flt_sqrt (listtype arguments)

  { /* flt_sqrt */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        sqrt(take_float(arg_1(arguments))));
  } /* flt_sqrt */



objecttype flt_str (listtype arguments)

  { /* flt_str */
    isit_float(arg_1(arguments));
    return bld_stri_temp(
        fltStr(take_float(arg_1(arguments))));
  } /* flt_str */



objecttype flt_tan (listtype arguments)

  { /* flt_tan */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        tan(take_float(arg_1(arguments))));
  } /* flt_tan */



objecttype flt_tanh (listtype arguments)

  { /* flt_tanh */
    isit_float(arg_1(arguments));
    return bld_float_temp(
        tanh(take_float(arg_1(arguments))));
  } /* flt_tanh */



objecttype flt_trunc (listtype arguments)

  { /* flt_trunc */
    isit_float(arg_1(arguments));
    return bld_int_temp((inttype) take_float(arg_1(arguments)));
  } /* flt_trunc */



objecttype flt_value (listtype arguments)

  {
    objecttype obj_arg;

  /* flt_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != FLOATOBJECT) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_float_temp(take_float(obj_arg));
    } /* if */
  } /* flt_value */
