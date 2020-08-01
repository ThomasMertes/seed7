/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
#include "runerr.h"
#include "memory.h"
#include "int_rtl.h"

#undef EXTERN
#define EXTERN
#include "intlib.h"


static inttype fact[] = {
    1, 1, 2, 6, 24, 120, 720, 5040, 40320,
    362880, 3628800, 39916800, 479001600
  };



#ifdef ANSI_C

objecttype int_abs (listtype arguments)
#else

objecttype int_abs (arguments)
listtype arguments;
#endif

  {
    inttype number;

  /* int_abs */
    isit_int(arg_1(arguments));
    number = take_int(arg_1(arguments));
    if (number < 0) {
      number = -number;
    } /* if */
    return(bld_int_temp(number));
  } /* int_abs */



#ifdef ANSI_C

objecttype int_add (listtype arguments)
#else

objecttype int_add (arguments)
listtype arguments;
#endif

  { /* int_add */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_int_temp(
        take_int(arg_1(arguments)) +
        take_int(arg_3(arguments))));
  } /* int_add */



#ifdef ANSI_C

objecttype int_binom (listtype arguments)
#else

objecttype int_binom (arguments)
listtype arguments;
#endif

  { /* int_binom */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_int_temp(intBinom(
        take_int(arg_1(arguments)),
        take_int(arg_3(arguments)))));
  } /* int_binom */



#ifdef ANSI_C

objecttype int_bit_length (listtype arguments)
#else

objecttype int_bit_length (arguments)
listtype arguments;
#endif

  { /* int_bit_length */
    isit_int(arg_1(arguments));
    return(bld_int_temp(intBitLength(take_int(arg_1(arguments)))));
  } /* int_bit_length */



#ifdef ANSI_C

objecttype int_cmp (listtype arguments)
#else

objecttype int_cmp (arguments)
listtype arguments;
#endif

  {
    inttype number1;
    inttype number2;

  /* int_cmp */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    number1 = take_int(arg_1(arguments));
    number2 = take_int(arg_2(arguments));
    if (number1 < number2) {
      return(bld_int_temp(-1));
    } else if (number1 > number2) {
      return(bld_int_temp(1));
    } else {
      return(bld_int_temp(0));
    } /* if */
  } /* int_cmp */



#ifdef ANSI_C

objecttype int_conv (listtype arguments)
#else

objecttype int_conv (arguments)
listtype arguments;
#endif

  { /* int_conv */
    isit_int(arg_3(arguments));
    return(bld_int_temp(take_int(arg_3(arguments))));
  } /* int_conv */



#ifdef ANSI_C

objecttype int_cpy (listtype arguments)
#else

objecttype int_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype int_variable;

  /* int_cpy */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue = take_int(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* int_cpy */



#ifdef ANSI_C

objecttype int_create (listtype arguments)
#else

objecttype int_create (arguments)
listtype arguments;
#endif

  { /* int_create */
    isit_int(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), INTOBJECT);
    arg_1(arguments)->value.intvalue = take_int(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* int_create */



#ifdef ANSI_C

objecttype int_decr (listtype arguments)
#else

objecttype int_decr (arguments)
listtype arguments;
#endif

  {
    objecttype int_variable;

  /* int_decr */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    int_variable->value.intvalue--;
    return(SYS_EMPTY_OBJECT);
  } /* int_decr */



#ifdef ANSI_C

objecttype int_div (listtype arguments)
#else

objecttype int_div (arguments)
listtype arguments;
#endif

  {
    inttype divisor;

  /* int_div */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    divisor = take_int(arg_3(arguments));
    if (divisor == 0) {
      return(raise_exception(SYS_NUM_EXCEPTION));
    } else {
      return(bld_int_temp(
          take_int(arg_1(arguments)) / divisor));
    } /* if */
  } /* int_div */



#ifdef ANSI_C

objecttype int_eq (listtype arguments)
#else

objecttype int_eq (arguments)
listtype arguments;
#endif

  { /* int_eq */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    if (take_int(arg_1(arguments)) ==
        take_int(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* int_eq */



#ifdef ANSI_C

objecttype int_fact (listtype arguments)
#else

objecttype int_fact (arguments)
listtype arguments;
#endif

  {
    int number;

  /* int_fact */
    isit_int(arg_2(arguments));
    number = (int) take_int(arg_2(arguments));
    if (number < 0 || number > 12) {
      return(raise_exception(SYS_NUM_EXCEPTION));
    } else {
      return(bld_int_temp(fact[number]));
    } /* if */
  } /* int_fact */



#ifdef ANSI_C

objecttype int_ge (listtype arguments)
#else

objecttype int_ge (arguments)
listtype arguments;
#endif

  { /* int_ge */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    if (take_int(arg_1(arguments)) >=
        take_int(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* int_ge */



#ifdef ANSI_C

objecttype int_grow (listtype arguments)
#else

objecttype int_grow (arguments)
listtype arguments;
#endif

  {
    objecttype int_variable;

  /* int_grow */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue += take_int(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* int_grow */



#ifdef ANSI_C

objecttype int_gt (listtype arguments)
#else

objecttype int_gt (arguments)
listtype arguments;
#endif

  { /* int_gt */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    if (take_int(arg_1(arguments)) >
        take_int(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* int_gt */



#ifdef ANSI_C

objecttype int_hashcode (listtype arguments)
#else

objecttype int_hashcode (arguments)
listtype arguments;
#endif

  { /* int_hashcode */
    isit_int(arg_1(arguments));
    return(bld_int_temp(take_int(arg_1(arguments))));
  } /* int_hashcode */



#ifdef ANSI_C

objecttype int_incr (listtype arguments)
#else

objecttype int_incr (arguments)
listtype arguments;
#endif

  {
    objecttype int_variable;

  /* int_incr */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    int_variable->value.intvalue++;
    return(SYS_EMPTY_OBJECT);
  } /* int_incr */



#ifdef ANSI_C

objecttype int_le (listtype arguments)
#else

objecttype int_le (arguments)
listtype arguments;
#endif

  { /* int_le */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    if (take_int(arg_1(arguments)) <=
        take_int(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* int_le */



#ifdef ANSI_C

objecttype int_log2 (listtype arguments)
#else

objecttype int_log2 (arguments)
listtype arguments;
#endif

  { /* int_log2 */
    isit_int(arg_1(arguments));
    return(bld_int_temp(intLog2(take_int(arg_1(arguments)))));
  } /* int_log2 */



#ifdef ANSI_C

objecttype int_lowest_set_bit (listtype arguments)
#else

objecttype int_lowest_set_bit (arguments)
listtype arguments;
#endif

  { /* int_lowest_set_bit */
    isit_int(arg_1(arguments));
    return(bld_int_temp(intLowestSetBit(take_int(arg_1(arguments)))));
  } /* int_lowest_set_bit */



#ifdef ANSI_C

objecttype int_lpad0 (listtype arguments)
#else

objecttype int_lpad0 (arguments)
listtype arguments;
#endif

  { /* int_lpad0 */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_stri_temp(intLpad0(
        take_int(arg_1(arguments)), take_int(arg_3(arguments)))));
  } /* int_lpad0 */



#ifdef ANSI_C

objecttype int_lshift (listtype arguments)
#else

objecttype int_lshift (arguments)
listtype arguments;
#endif

  { /* int_lshift */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_int_temp(
        take_int(arg_1(arguments)) <<
        take_int(arg_3(arguments))));
  } /* int_lshift */



#ifdef ANSI_C

objecttype int_lshift_assign (listtype arguments)
#else

objecttype int_lshift_assign (arguments)
listtype arguments;
#endif

  {
    objecttype int_variable;

  /* int_lshift_assign */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue <<= take_int(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* int_lshift_assign */



#ifdef ANSI_C

objecttype int_lt (listtype arguments)
#else

objecttype int_lt (arguments)
listtype arguments;
#endif

  { /* int_lt */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    if (take_int(arg_1(arguments)) <
        take_int(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* int_lt */



#ifdef ANSI_C

objecttype int_mcpy (listtype arguments)
#else

objecttype int_mcpy (arguments)
listtype arguments;
#endif

  {
    objecttype int_variable;

  /* int_mcpy */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue *= take_int(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* int_mcpy */



#ifdef ANSI_C

objecttype int_mdiv (listtype arguments)
#else

objecttype int_mdiv (arguments)
listtype arguments;
#endif

  {
    inttype numerator;
    inttype denominator;
    inttype result;

  /* int_mdiv */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    numerator = take_int(arg_1(arguments));
    denominator = take_int(arg_3(arguments));
    if (denominator == 0) {
      return(raise_exception(SYS_NUM_EXCEPTION));
    } else {
      if (numerator > 0 && denominator < 0) {
        result = (numerator - 1) / denominator - 1;
      } else if (numerator < 0 && denominator > 0) {
        result = (numerator + 1) / denominator - 1;
      } else {
        result = numerator / denominator;
      } /* if */
      return(bld_int_temp(result));
    } /* if */
  } /* int_mdiv */



#ifdef ANSI_C

objecttype int_minus (listtype arguments)
#else

objecttype int_minus (arguments)
listtype arguments;
#endif

  { /* int_minus */
    isit_int(arg_2(arguments));
    return(bld_int_temp( - take_int(arg_2(arguments))));
  } /* int_minus */



#ifdef ANSI_C

objecttype int_mod (listtype arguments)
#else

objecttype int_mod (arguments)
listtype arguments;
#endif

  {
    inttype numerator;
    inttype denominator;
    inttype result;

  /* int_mod */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    numerator = take_int(arg_1(arguments));
    denominator = take_int(arg_3(arguments));
    if (denominator == 0) {
      return(raise_exception(SYS_NUM_EXCEPTION));
    } else {
      result = numerator % denominator;
      if (((numerator > 0 && denominator < 0) ||
          (numerator < 0 && denominator > 0)) && result != 0) {
        result = result + denominator;
      } /* if */
      return(bld_int_temp(result));
    } /* if */
  } /* int_mod */



#ifdef ANSI_C

objecttype int_mult (listtype arguments)
#else

objecttype int_mult (arguments)
listtype arguments;
#endif

  { /* int_mult */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_int_temp(
        take_int(arg_1(arguments)) *
        take_int(arg_3(arguments))));
  } /* int_mult */



#ifdef ANSI_C

objecttype int_ne (listtype arguments)
#else

objecttype int_ne (arguments)
listtype arguments;
#endif

  { /* int_ne */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    if (take_int(arg_1(arguments)) !=
        take_int(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* int_ne */



#ifdef ANSI_C

objecttype int_odd (listtype arguments)
#else

objecttype int_odd (arguments)
listtype arguments;
#endif

  { /* int_odd */
    isit_int(arg_1(arguments));
    if (take_int(arg_1(arguments)) & 1) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* int_odd */



#ifdef ANSI_C

objecttype int_ord (listtype arguments)
#else

objecttype int_ord (arguments)
listtype arguments;
#endif

  { /* int_ord */
    isit_int(arg_1(arguments));
    return(bld_int_temp(take_int(arg_1(arguments))));
  } /* int_ord */



#ifdef ANSI_C

objecttype int_parse (listtype arguments)
#else

objecttype int_parse (arguments)
listtype arguments;
#endif

  { /* int_parse */
    isit_stri(arg_3(arguments));
    return(bld_int_temp(intParse(
        take_stri(arg_3(arguments)))));
  } /* int_parse */



#ifdef ANSI_C

objecttype int_plus (listtype arguments)
#else

objecttype int_plus (arguments)
listtype arguments;
#endif

  { /* int_plus */
    isit_int(arg_2(arguments));
    return(bld_int_temp(take_int(arg_2(arguments))));
  } /* int_plus */



#ifdef ANSI_C

objecttype int_pow (listtype arguments)
#else

objecttype int_pow (arguments)
listtype arguments;
#endif

  { /* int_pow */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_int_temp(intPow(
        take_int(arg_1(arguments)),
        take_int(arg_3(arguments)))));
  } /* int_pow */



#ifdef ANSI_C

objecttype int_pred (listtype arguments)
#else

objecttype int_pred (arguments)
listtype arguments;
#endif

  { /* int_pred */
    isit_int(arg_1(arguments));
    return(bld_int_temp(take_int(arg_1(arguments)) - 1));
  } /* int_pred */



#ifdef ANSI_C

objecttype int_rand (listtype arguments)
#else

objecttype int_rand (arguments)
listtype arguments;
#endif

  { /* int_rand */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    return(bld_int_temp(intRand(
        take_int(arg_1(arguments)),
        take_int(arg_2(arguments)))));
  } /* int_rand */



#ifdef ANSI_C

objecttype int_rem (listtype arguments)
#else

objecttype int_rem (arguments)
listtype arguments;
#endif

  {
    inttype divisor;

  /* int_rem */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    divisor = take_int(arg_3(arguments));
    if (divisor == 0) {
      return(raise_exception(SYS_NUM_EXCEPTION));
    } else {
      return(bld_int_temp(
          take_int(arg_1(arguments)) % divisor));
    } /* if */
  } /* int_rem */



#ifdef ANSI_C

objecttype int_rshift (listtype arguments)
#else

objecttype int_rshift (arguments)
listtype arguments;
#endif

  { /* int_rshift */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
#ifdef RSHIFT_DOES_SIGN_EXTEND
    return(bld_int_temp(
        take_int(arg_1(arguments)) >>
        take_int(arg_3(arguments))));
#else
    if (take_int(arg_1(arguments)) < 0) {
      return(bld_int_temp(
          ~(~take_int(arg_1(arguments)) >>
          take_int(arg_3(arguments)))));
    } else {
      return(bld_int_temp(
          take_int(arg_1(arguments)) >>
          take_int(arg_3(arguments))));
    } /* if */
#endif
  } /* int_rshift */



#ifdef ANSI_C

objecttype int_rshift_assign (listtype arguments)
#else

objecttype int_rshift_assign (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* int_rshift_assign */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

objecttype int_urshift (listtype arguments)
#else

objecttype int_urshift (arguments)
listtype arguments;
#endif

  { /* int_urshift */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_stri_temp((inttype)
        (((uinttype) (take_int(arg_1(arguments)))) >>
        take_int(arg_3(arguments)))));
  } /* int_urshift */
#endif



#ifdef ANSI_C

objecttype int_sbtr (listtype arguments)
#else

objecttype int_sbtr (arguments)
listtype arguments;
#endif

  { /* int_sbtr */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_int_temp(
        take_int(arg_1(arguments)) -
        take_int(arg_3(arguments))));
  } /* int_sbtr */



#ifdef ANSI_C

objecttype int_shrink (listtype arguments)
#else

objecttype int_shrink (arguments)
listtype arguments;
#endif

  {
    objecttype int_variable;

  /* int_shrink */
    int_variable = arg_1(arguments);
    isit_int(int_variable);
    is_variable(int_variable);
    isit_int(arg_3(arguments));
    int_variable->value.intvalue -= take_int(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* int_shrink */



#ifdef ANSI_C

objecttype int_sqrt (listtype arguments)
#else

objecttype int_sqrt (arguments)
listtype arguments;
#endif

  { /* int_sqrt */
    isit_int(arg_1(arguments));
    return(bld_int_temp(intSqrt(
        take_int(arg_1(arguments)))));
  } /* int_sqrt */



#ifdef ANSI_C

objecttype int_str (listtype arguments)
#else

objecttype int_str (arguments)
listtype arguments;
#endif

  { /* int_str */
    isit_int(arg_1(arguments));
    return(bld_stri_temp(intStr(
        take_int(arg_1(arguments)))));
  } /* int_str */



#ifdef ANSI_C

objecttype int_succ (listtype arguments)
#else

objecttype int_succ (arguments)
listtype arguments;
#endif

  { /* int_succ */
    isit_int(arg_1(arguments));
    return(bld_int_temp(take_int(arg_1(arguments)) + 1));
  } /* int_succ */



#ifdef ANSI_C

objecttype int_value (listtype arguments)
#else

objecttype int_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;

  /* int_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != INTOBJECT) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } else {
      return(bld_int_temp(take_int(obj_arg)));
    } /* if */
  } /* int_value */
