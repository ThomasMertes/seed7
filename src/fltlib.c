/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
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

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "runerr.h"
#include "memory.h"
#include "flt_rtl.h"

#undef EXTERN
#define EXTERN
#include "fltlib.h"



#ifdef ANSI_C

objecttype flt_a2tan (listtype arguments)
#else

objecttype flt_a2tan (arguments)
listtype arguments;
#endif

  { /* flt_a2tan */
    isit_float(arg_1(arguments));
    isit_float(arg_2(arguments));
    return(bld_float_temp(atan2(
        take_float(arg_1(arguments)),
        take_float(arg_2(arguments)))));
  } /* flt_a2tan */



#ifdef ANSI_C

objecttype flt_abs (listtype arguments)
#else

objecttype flt_abs (arguments)
listtype arguments;
#endif

  {
    double number;

  /* flt_abs */
    isit_float(arg_1(arguments));
    number = take_float(arg_1(arguments));
    if (number < (double) 0.0) {
      number = -number;
    } /* if */
    return(bld_float_temp(number));
  } /* flt_abs */



#ifdef ANSI_C

objecttype flt_acos (listtype arguments)
#else

objecttype flt_acos (arguments)
listtype arguments;
#endif

  { /* flt_acos */
    isit_float(arg_1(arguments));
    return(bld_float_temp(acos(take_float(arg_1(arguments)))));
  } /* flt_acos */



#ifdef ANSI_C

objecttype flt_add (listtype arguments)
#else

objecttype flt_add (arguments)
listtype arguments;
#endif

  { /* flt_add */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return(bld_float_temp(
        (double) take_float(arg_1(arguments)) +
        (double) take_float(arg_3(arguments))));
  } /* flt_add */



#ifdef ANSI_C

objecttype flt_asin (listtype arguments)
#else

objecttype flt_asin (arguments)
listtype arguments;
#endif

  { /* flt_asin */
    isit_float(arg_1(arguments));
    return(bld_float_temp(asin(take_float(arg_1(arguments)))));
  } /* flt_asin */



#ifdef ANSI_C

objecttype flt_atan (listtype arguments)
#else

objecttype flt_atan (arguments)
listtype arguments;
#endif

  { /* flt_atan */
    isit_float(arg_1(arguments));
    return(bld_float_temp(atan(take_float(arg_1(arguments)))));
  } /* flt_atan */



#ifdef ANSI_C

objecttype flt_ceil (listtype arguments)
#else

objecttype flt_ceil (arguments)
listtype arguments;
#endif

  { /* flt_ceil */
    isit_float(arg_1(arguments));
    return(bld_float_temp(ceil(take_float(arg_1(arguments)))));
  } /* flt_ceil */



#ifdef ANSI_C

objecttype flt_cmp (listtype arguments)
#else

objecttype flt_cmp (arguments)
listtype arguments;
#endif

  {
    floattype number1;
    floattype number2;

  /* flt_cmp */
    isit_float(arg_1(arguments));
    isit_float(arg_2(arguments));
    number1 = take_float(arg_1(arguments));
    number2 = take_float(arg_2(arguments));
    if (number1 < number2) {
      return(bld_int_temp(-1));
    } else if (number1 > number2) {
      return(bld_int_temp(1));
    } else {
      return(bld_int_temp(0));
    } /* if */
  } /* flt_cmp */



#ifdef ANSI_C

objecttype flt_cos (listtype arguments)
#else

objecttype flt_cos (arguments)
listtype arguments;
#endif

  { /* flt_cos */
    isit_float(arg_1(arguments));
    return(bld_float_temp(cos(take_float(arg_1(arguments)))));
  } /* flt_cos */



#ifdef ANSI_C

objecttype flt_cpy (listtype arguments)
#else

objecttype flt_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype flt_variable;

  /* flt_cpy */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    flt_variable->value.floatvalue = take_float(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* flt_cpy */



#ifdef ANSI_C

objecttype flt_create (listtype arguments)
#else

objecttype flt_create (arguments)
listtype arguments;
#endif

  { /* flt_create */
    isit_float(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), FLOATOBJECT);
    arg_1(arguments)->value.floatvalue = take_float(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* flt_create */



#ifdef ANSI_C

objecttype flt_dgts (listtype arguments)
#else

objecttype flt_dgts (arguments)
listtype arguments;
#endif

  { /* flt_dgts */
    isit_float(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_stri_temp(fltDgts(
        take_float(arg_1(arguments)),
        take_int(arg_3(arguments)))));
  } /* flt_dgts */



#ifdef ANSI_C

objecttype flt_div (listtype arguments)
#else

objecttype flt_div (arguments)
listtype arguments;
#endif

  { /* flt_div */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return(bld_float_temp(
        ((double) take_float(arg_1(arguments))) / ((double) take_float(arg_3(arguments)))));
  } /* flt_div */



#ifdef ANSI_C

objecttype flt_eq (listtype arguments)
#else

objecttype flt_eq (arguments)
listtype arguments;
#endif

  { /* flt_eq */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    if (take_float(arg_1(arguments)) ==
        take_float(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* flt_eq */



#ifdef ANSI_C

objecttype flt_exp (listtype arguments)
#else

objecttype flt_exp (arguments)
listtype arguments;
#endif

  { /* flt_exp */
    isit_float(arg_1(arguments));
    return(bld_float_temp(exp(take_float(arg_1(arguments)))));
  } /* flt_exp */



#ifdef ANSI_C

objecttype flt_floor (listtype arguments)
#else

objecttype flt_floor (arguments)
listtype arguments;
#endif

  { /* flt_floor */
    isit_float(arg_1(arguments));
    return(bld_float_temp(floor(take_float(arg_1(arguments)))));
  } /* flt_floor */



#ifdef ANSI_C

objecttype flt_ge (listtype arguments)
#else

objecttype flt_ge (arguments)
listtype arguments;
#endif

  { /* flt_ge */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    if (take_float(arg_1(arguments)) >=
        take_float(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* flt_ge */



#ifdef ANSI_C

objecttype flt_grow (listtype arguments)
#else

objecttype flt_grow (arguments)
listtype arguments;
#endif

  {
    objecttype flt_variable;

  /* flt_grow */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    flt_variable->value.floatvalue += take_float(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* flt_grow */



#ifdef ANSI_C

objecttype flt_gt (listtype arguments)
#else

objecttype flt_gt (arguments)
listtype arguments;
#endif

  { /* flt_gt */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    if (take_float(arg_1(arguments)) >
        take_float(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* flt_gt */



#ifdef ANSI_C

objecttype flt_hashcode (listtype arguments)
#else

objecttype flt_hashcode (arguments)
listtype arguments;
#endif

  { /* flt_hashcode */
    isit_float(arg_1(arguments));
    /* The float value is taken as int on purpose */
    return(bld_int_temp(take_int(arg_1(arguments))));
  } /* flt_hashcode */



#ifdef ANSI_C

objecttype flt_hcos (listtype arguments)
#else

objecttype flt_hcos (arguments)
listtype arguments;
#endif

  { /* flt_hcos */
    isit_float(arg_1(arguments));
    return(bld_float_temp(cosh(take_float(arg_1(arguments)))));
  } /* flt_hcos */



#ifdef ANSI_C

objecttype flt_hsin (listtype arguments)
#else

objecttype flt_hsin (arguments)
listtype arguments;
#endif

  { /* flt_hsin */
    isit_float(arg_1(arguments));
    return(bld_float_temp(sinh(take_float(arg_1(arguments)))));
  } /* flt_hsin */



#ifdef ANSI_C

objecttype flt_htan (listtype arguments)
#else

objecttype flt_htan (arguments)
listtype arguments;
#endif

  { /* flt_htan */
    isit_float(arg_1(arguments));
    return(bld_float_temp(tanh(take_float(arg_1(arguments)))));
  } /* flt_htan */



#ifdef ANSI_C

objecttype flt_iflt (listtype arguments)
#else

objecttype flt_iflt (arguments)
listtype arguments;
#endif

  { /* flt_iflt */
    isit_int(arg_1(arguments));
    return(bld_float_temp((double) take_int(arg_1(arguments))));
  } /* flt_iflt */



#ifdef ANSI_C

objecttype flt_ipow (listtype arguments)
#else

objecttype flt_ipow (arguments)
listtype arguments;
#endif

  { /* flt_ipow */
    isit_float(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_float_temp(fltIPow(
        take_float(arg_1(arguments)),
        take_int(arg_3(arguments)))));
  } /* flt_ipow */



#ifdef ANSI_C

objecttype flt_isnan (listtype arguments)
#else

objecttype flt_isnan (arguments)
listtype arguments;
#endif

  { /* flt_isnan */
    isit_float(arg_1(arguments));
    if (isnan(take_float(arg_1(arguments)))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* flt_isnan */



#ifdef ANSI_C

objecttype flt_le (listtype arguments)
#else

objecttype flt_le (arguments)
listtype arguments;
#endif

  { /* flt_le */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    if (take_float(arg_1(arguments)) <=
        take_float(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* flt_le */



#ifdef ANSI_C

objecttype flt_log (listtype arguments)
#else

objecttype flt_log (arguments)
listtype arguments;
#endif

  { /* flt_log */
    isit_float(arg_1(arguments));
    return(bld_float_temp(log(take_float(arg_1(arguments)))));
  } /* flt_log */



#ifdef ANSI_C

objecttype flt_log10 (listtype arguments)
#else

objecttype flt_log10 (arguments)
listtype arguments;
#endif

  { /* flt_log10 */
    isit_float(arg_1(arguments));
    return(bld_float_temp(log10(take_float(arg_1(arguments)))));
  } /* flt_log10 */



#ifdef ANSI_C

objecttype flt_lt (listtype arguments)
#else

objecttype flt_lt (arguments)
listtype arguments;
#endif

  { /* flt_lt */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    if (take_float(arg_1(arguments)) <
        take_float(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* flt_lt */



#ifdef ANSI_C

objecttype flt_mcpy (listtype arguments)
#else

objecttype flt_mcpy (arguments)
listtype arguments;
#endif

  {
    objecttype flt_variable;

  /* flt_mcpy */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    flt_variable->value.floatvalue += take_float(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* flt_mcpy */



#ifdef ANSI_C

objecttype flt_minus (listtype arguments)
#else

objecttype flt_minus (arguments)
listtype arguments;
#endif

  { /* flt_minus */
    isit_float(arg_2(arguments));
    return(bld_float_temp( - (double) take_float(arg_2(arguments))));
  } /* flt_minus */



#ifdef ANSI_C

objecttype flt_mult (listtype arguments)
#else

objecttype flt_mult (arguments)
listtype arguments;
#endif

  { /* flt_mult */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return(bld_float_temp(
        (double) take_float(arg_1(arguments)) *
        (double) take_float(arg_3(arguments))));
  } /* flt_mult */



#ifdef ANSI_C

objecttype flt_ne (listtype arguments)
#else

objecttype flt_ne (arguments)
listtype arguments;
#endif

  { /* flt_ne */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    if (take_float(arg_1(arguments)) !=
        take_float(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* flt_ne */



#ifdef ANSI_C

objecttype flt_parse (listtype arguments)
#else

objecttype flt_parse (arguments)
listtype arguments;
#endif

  { /* flt_parse */
    isit_stri(arg_3(arguments));
    return(bld_float_temp(fltParse(
        take_stri(arg_3(arguments)))));
  } /* flt_parse */



#ifdef ANSI_C

objecttype flt_plus (listtype arguments)
#else

objecttype flt_plus (arguments)
listtype arguments;
#endif

  { /* flt_plus */
    isit_float(arg_2(arguments));
    return(bld_float_temp((double) take_float(arg_2(arguments))));
  } /* flt_plus */



#ifdef ANSI_C

objecttype flt_pow (listtype arguments)
#else

objecttype flt_pow (arguments)
listtype arguments;
#endif

  { /* flt_pow */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return(bld_float_temp(pow(
        take_float(arg_1(arguments)),
        take_float(arg_3(arguments)))));
  } /* flt_pow */



#ifdef ANSI_C

objecttype flt_rand (listtype arguments)
#else

objecttype flt_rand (arguments)
listtype arguments;
#endif

  { /* flt_rand */
    isit_float(arg_1(arguments));
    isit_float(arg_2(arguments));
    return(bld_float_temp(fltRand(
        take_float(arg_1(arguments)),
        take_float(arg_2(arguments)))));
  } /* flt_rand */



#ifdef ANSI_C

objecttype flt_round (listtype arguments)
#else

objecttype flt_round (arguments)
listtype arguments;
#endif

  {
    floattype number;

  /* flt_round */
    isit_float(arg_1(arguments));
    number = take_float(arg_1(arguments));
    if (number < (floattype) 0.0) {
      return(bld_int_temp(-((inttype) (0.5 - number))));
    } else {
      return(bld_int_temp((inttype) (0.5 + number)));
    } /* if */
  } /* flt_round */



#ifdef ANSI_C

objecttype flt_sbtr (listtype arguments)
#else

objecttype flt_sbtr (arguments)
listtype arguments;
#endif

  { /* flt_sbtr */
    isit_float(arg_1(arguments));
    isit_float(arg_3(arguments));
    return(bld_float_temp(
        (double) take_float(arg_1(arguments)) -
        (double) take_float(arg_3(arguments))));
  } /* flt_sbtr */



#ifdef ANSI_C

objecttype flt_shrink (listtype arguments)
#else

objecttype flt_shrink (arguments)
listtype arguments;
#endif

  {
    objecttype flt_variable;

  /* flt_shrink */
    flt_variable = arg_1(arguments);
    isit_float(flt_variable);
    is_variable(flt_variable);
    isit_float(arg_3(arguments));
    flt_variable->value.floatvalue -= take_float(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* flt_shrink */



#ifdef ANSI_C

objecttype flt_sin (listtype arguments)
#else

objecttype flt_sin (arguments)
listtype arguments;
#endif

  { /* flt_sin */
    isit_float(arg_1(arguments));
    return(bld_float_temp(sin(take_float(arg_1(arguments)))));
  } /* flt_sin */



#ifdef ANSI_C

objecttype flt_sqrt (listtype arguments)
#else

objecttype flt_sqrt (arguments)
listtype arguments;
#endif

  { /* flt_sqrt */
    isit_float(arg_1(arguments));
    return(bld_float_temp(sqrt(take_float(arg_1(arguments)))));
  } /* flt_sqrt */



#ifdef ANSI_C

objecttype flt_str (listtype arguments)
#else

objecttype flt_str (arguments)
listtype arguments;
#endif

  { /* flt_str */
    isit_float(arg_1(arguments));
    return(bld_stri_temp(fltStr(
        take_float(arg_1(arguments)))));
  } /* flt_str */



#ifdef ANSI_C

objecttype flt_tan (listtype arguments)
#else

objecttype flt_tan (arguments)
listtype arguments;
#endif

  { /* flt_tan */
    isit_float(arg_1(arguments));
    return(bld_float_temp(tan(take_float(arg_1(arguments)))));
  } /* flt_tan */



#ifdef ANSI_C

objecttype flt_trunc (listtype arguments)
#else

objecttype flt_trunc (arguments)
listtype arguments;
#endif

  { /* flt_trunc */
    isit_float(arg_1(arguments));
    return(bld_int_temp((inttype) take_float(arg_1(arguments))));
  } /* flt_trunc */



#ifdef ANSI_C

objecttype flt_value (listtype arguments)
#else

objecttype flt_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;

  /* flt_value */
    isit_reference(arg_3(arguments));
    obj_arg = take_reference(arg_3(arguments));
    isit_float(obj_arg);
    return(bld_float_temp(take_float(obj_arg)));
  } /* flt_value */
