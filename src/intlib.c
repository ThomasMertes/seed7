/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
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
    6227020800, 87178291200, 1307674368000, 20922789888000,
    355687428096000, 6402373705728000, 121645100408832000,
    2432902008176640000
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



objecttype int_add (listtype arguments)

  { /* int_add */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        take_int(arg_1(arguments)) + take_int(arg_3(arguments)));
  } /* int_add */



objecttype int_binom (listtype arguments)

  { /* int_binom */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        intBinom(take_int(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* int_binom */



objecttype int_bit_length (listtype arguments)

  { /* int_bit_length */
    isit_int(arg_1(arguments));
    return bld_int_temp(
        intBitLength(take_int(arg_1(arguments))));
  } /* int_bit_length */



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



objecttype int_hashcode (listtype arguments)

  { /* int_hashcode */
    isit_int(arg_1(arguments));
    return bld_int_temp(take_int(arg_1(arguments)));
  } /* int_hashcode */



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



objecttype int_log2 (listtype arguments)

  { /* int_log2 */
    isit_int(arg_1(arguments));
    return bld_int_temp(
        intLog2(take_int(arg_1(arguments))));
  } /* int_log2 */



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



objecttype int_lshift (listtype arguments)

  { /* int_lshift */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        take_int(arg_1(arguments)) << take_int(arg_3(arguments)));
  } /* int_lshift */



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



objecttype int_minus (listtype arguments)

  { /* int_minus */
    isit_int(arg_2(arguments));
    return bld_int_temp(
        -take_int(arg_2(arguments)));
  } /* int_minus */



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



objecttype int_mult (listtype arguments)

  { /* int_mult */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        take_int(arg_1(arguments)) * take_int(arg_3(arguments)));
  } /* int_mult */



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



objecttype int_odd (listtype arguments)

  { /* int_odd */
    isit_int(arg_1(arguments));
    if (take_int(arg_1(arguments)) & 1) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* int_odd */



objecttype int_ord (listtype arguments)

  { /* int_ord */
    isit_int(arg_1(arguments));
    return bld_int_temp(take_int(arg_1(arguments)));
  } /* int_ord */



objecttype int_parse (listtype arguments)

  { /* int_parse */
    isit_stri(arg_3(arguments));
    return bld_int_temp(
        intParse(take_stri(arg_3(arguments))));
  } /* int_parse */



objecttype int_plus (listtype arguments)

  { /* int_plus */
    isit_int(arg_2(arguments));
    return bld_int_temp(take_int(arg_2(arguments)));
  } /* int_plus */



objecttype int_pow (listtype arguments)

  { /* int_pow */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        intPow(take_int(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* int_pow */



objecttype int_pred (listtype arguments)

  { /* int_pred */
    isit_int(arg_1(arguments));
    return bld_int_temp(take_int(arg_1(arguments)) - 1);
  } /* int_pred */



objecttype int_radix (listtype arguments)

  { /* int_radix */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        intRadix(take_int(arg_1(arguments)), take_int(arg_3(arguments)), FALSE));
  } /* int_radix */



objecttype int_RADIX (listtype arguments)

  { /* int_RADIX */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        intRadix(take_int(arg_1(arguments)), take_int(arg_3(arguments)), TRUE));
  } /* int_RADIX */



objecttype int_rand (listtype arguments)

  { /* int_rand */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    return bld_int_temp(
        intRand(take_int(arg_1(arguments)), take_int(arg_2(arguments))));
  } /* int_rand */



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



#ifdef OUT_OF_ORDER
objecttype int_urshift (listtype arguments)

  { /* int_urshift */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp((inttype)
        (((uinttype) (take_int(arg_1(arguments)))) >>
        take_int(arg_3(arguments))));
  } /* int_urshift */
#endif



objecttype int_sbtr (listtype arguments)

  { /* int_sbtr */
    isit_int(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        take_int(arg_1(arguments)) - take_int(arg_3(arguments)));
  } /* int_sbtr */



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



objecttype int_sqrt (listtype arguments)

  { /* int_sqrt */
    isit_int(arg_1(arguments));
    return bld_int_temp(
        intSqrt(take_int(arg_1(arguments))));
  } /* int_sqrt */



objecttype int_str (listtype arguments)

  { /* int_str */
    isit_int(arg_1(arguments));
    return bld_stri_temp(
        intStr(take_int(arg_1(arguments))));
  } /* int_str */



objecttype int_succ (listtype arguments)

  { /* int_succ */
    isit_int(arg_1(arguments));
    return bld_int_temp(take_int(arg_1(arguments)) + 1);
  } /* int_succ */



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
