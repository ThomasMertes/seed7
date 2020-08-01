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



objecttype big_abs (listtype arguments)

  { /* big_abs */
    isit_bigint(arg_1(arguments));
    return bld_bigint_temp(
        bigAbs(take_bigint(arg_1(arguments))));
  } /* big_abs */



objecttype big_add (listtype arguments)

  { /* big_add */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigAdd(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_add */



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



objecttype big_cmp (listtype arguments)

  { /* big_cmp */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_2(arguments));
    return bld_int_temp(
        bigCmp(take_bigint(arg_1(arguments)), take_bigint(arg_2(arguments))));
  } /* big_cmp */



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



objecttype big_div (listtype arguments)

  { /* big_div */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigDiv(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_div */



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



objecttype big_gcd (listtype arguments)

  { /* big_gcd */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_2(arguments));
    return bld_bigint_temp(
        bigGcd(take_bigint(arg_1(arguments)), take_bigint(arg_2(arguments))));
  } /* big_gcd */



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



objecttype big_grow (listtype arguments)

  { /* big_grow */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigGrow(&take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* big_grow */



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



objecttype big_hashcode (listtype arguments)

  { /* big_hashcode */
    isit_bigint(arg_1(arguments));
    return bld_int_temp(
        bigHashCode(take_bigint(arg_1(arguments))));
  } /* big_hashcode */



objecttype big_iconv (listtype arguments)

  { /* big_iconv */
    isit_int(arg_3(arguments));
    return bld_bigint_temp(
        bigIConv(take_int(arg_3(arguments))));
  } /* big_iconv */



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



objecttype big_ipow (listtype arguments)

  { /* big_ipow */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_bigint_temp(
        bigIPow(take_bigint(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* big_ipow */



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



objecttype big_log2 (listtype arguments)

  { /* big_log2 */
    isit_bigint(arg_1(arguments));
    return bld_bigint_temp(
        bigLog2(take_bigint(arg_1(arguments))));
  } /* big_log2 */



objecttype big_lowest_set_bit (listtype arguments)

  { /* big_lowest_set_bit */
    isit_bigint(arg_1(arguments));
    return bld_int_temp(
        bigLowestSetBit(take_bigint(arg_1(arguments))));
  } /* big_lowest_set_bit */



objecttype big_lshift (listtype arguments)

  { /* big_lshift */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_bigint_temp(
        bigLShift(take_bigint(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* big_lshift */



objecttype big_lshift_assign (listtype arguments)

  { /* big_lshift_assign */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigLShiftAssign(&take_bigint(arg_1(arguments)), take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* big_lshift_assign */



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



objecttype big_mdiv (listtype arguments)

  { /* big_mdiv */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigMDiv(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_mdiv */



objecttype big_minus (listtype arguments)

  { /* big_minus */
    isit_bigint(arg_2(arguments));
    return bld_bigint_temp(
        bigMinus(take_bigint(arg_2(arguments))));
  } /* big_minus */



objecttype big_mod (listtype arguments)

  { /* big_mod */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigMod(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_mod */



objecttype big_mult (listtype arguments)

  { /* big_mult */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigMult(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_mult */



objecttype big_mult_assign (listtype arguments)

  { /* big_mult_assign */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigMultAssign(&take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* big_mult_assign */



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



objecttype big_odd (listtype arguments)

  { /* big_odd */
    isit_bigint(arg_1(arguments));
    if (bigOdd(take_bigint(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* big_odd */



objecttype big_ord (listtype arguments)

  { /* big_ord */
    isit_bigint(arg_1(arguments));
    return bld_int_temp(
        bigOrd(take_bigint(arg_1(arguments))));
  } /* big_ord */



objecttype big_parse (listtype arguments)

  { /* big_parse */
    isit_stri(arg_3(arguments));
    return bld_bigint_temp(
        bigParse(take_stri(arg_3(arguments))));
  } /* big_parse */



objecttype big_parse_based (listtype arguments)

  { /* big_parse_based */
    isit_stri(arg_1(arguments));
    isit_int(arg_2(arguments));
    return bld_bigint_temp(
        bigParseBased(take_stri(arg_1(arguments)), take_int(arg_2(arguments))));
  } /* big_parse_based */



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



objecttype big_pred (listtype arguments)

  { /* big_pred */
    isit_bigint(arg_1(arguments));
    return bld_bigint_temp(
        bigPred(take_bigint(arg_1(arguments))));
  } /* big_pred */



objecttype big_rand (listtype arguments)

  { /* big_rand */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_2(arguments));
    return bld_bigint_temp(
        bigRand(take_bigint(arg_1(arguments)), take_bigint(arg_2(arguments))));
  } /* big_rand */



objecttype big_rem (listtype arguments)

  { /* big_rem */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigRem(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_rem */



objecttype big_rshift (listtype arguments)

  { /* big_rshift */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_bigint_temp(
        bigRShift(take_bigint(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* big_rshift */



objecttype big_rshift_assign (listtype arguments)

  { /* big_rshift_assign */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigRShiftAssign(&take_bigint(arg_1(arguments)), take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* big_rshift_assign */



objecttype big_sbtr (listtype arguments)

  { /* big_sbtr */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigSbtr(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* big_sbtr */



objecttype big_shrink (listtype arguments)

  { /* big_shrink */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigShrink(&take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* big_shrink */



objecttype big_str (listtype arguments)

  { /* big_str */
    isit_bigint(arg_1(arguments));
    return bld_stri_temp(
        bigStr(take_bigint(arg_1(arguments))));
  } /* big_str */



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
