/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2006  Thomas Mertes                        */
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
#include "heaputl.h"
#include "syvarutl.h"
#include "runerr.h"
#include "memory.h"
#include "big_rtl.h"

#undef EXTERN
#define EXTERN
#include "biglib.h"



#ifdef ANSI_C

objecttype big_abs (listtype arguments)
#else

objecttype big_abs (arguments)
listtype arguments;
#endif

  { /* big_abs */
    isit_bigint(arg_1(arguments));
    return(bld_bigint_temp(
        bigAbs(take_bigint(arg_1(arguments)))));
  } /* big_abs */



#ifdef ANSI_C

objecttype big_add (listtype arguments)
#else

objecttype big_add (arguments)
listtype arguments;
#endif

  { /* big_add */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return(bld_bigint_temp(
        bigAdd(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)))));
  } /* big_add */



#ifdef ANSI_C

objecttype big_bit_length (listtype arguments)
#else

objecttype big_bit_length (arguments)
listtype arguments;
#endif

  { /* big_bit_length */
    isit_bigint(arg_1(arguments));
    return(bld_int_temp(
        bigBitLength(take_bigint(arg_1(arguments)))));
  } /* big_bit_length */



#ifdef ANSI_C

objecttype big_clit (listtype arguments)
#else

objecttype big_clit (arguments)
listtype arguments;
#endif

  { /* big_clit */
    isit_bigint(arg_1(arguments));
    return(bld_stri_temp(bigCLit(
        take_bigint(arg_1(arguments)))));
  } /* big_clit */



#ifdef ANSI_C

objecttype big_cmp (listtype arguments)
#else

objecttype big_cmp (arguments)
listtype arguments;
#endif

  { /* big_cmp */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_2(arguments));
    return(bld_int_temp(
        bigCmp(take_bigint(arg_1(arguments)), take_bigint(arg_2(arguments)))));
  } /* big_cmp */



#ifdef ANSI_C

objecttype big_cpy (listtype arguments)
#else

objecttype big_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype big_to;
    objecttype big_from;
    memsizetype new_size;
    biginttype new_big;

  /* big_cpy */
    big_to = arg_1(arguments);
    big_from = arg_3(arguments);
    isit_bigint(big_to);
    isit_bigint(big_from);
    is_variable(big_to);
    new_big = take_bigint(big_to);
    if (TEMP_OBJECT(big_from)) {
      FREE_BIG(new_big, new_big->size);
      big_to->value.bigintvalue = take_bigint(big_from);
      big_from->value.bigintvalue = NULL;
    } else {
      new_size = take_bigint(big_from)->size;
      if (new_big->size != new_size) {
        if (!ALLOC_BIG(new_big, new_size)) {
          return(raise_exception(SYS_MEM_EXCEPTION));
        } else {
          FREE_BIG(take_bigint(big_to), take_bigint(big_to)->size);
          big_to->value.bigintvalue = new_big;
          new_big->size = new_size;
        } /* if */
      } /* if */
      memcpy(new_big->bigdigits, take_bigint(big_from)->bigdigits,
          (SIZE_TYPE) new_size * sizeof(bigdigittype));
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* big_cpy */



#ifdef ANSI_C

objecttype big_create (listtype arguments)
#else

objecttype big_create (arguments)
listtype arguments;
#endif

  {
    objecttype big_to;
    objecttype big_from;
    memsizetype new_size;
    biginttype new_big;

  /* big_create */
    big_to = arg_1(arguments);
    big_from = arg_3(arguments);
    isit_bigint(big_from);
    SET_CATEGORY_OF_OBJ(big_to, BIGINTOBJECT);
    if (TEMP_OBJECT(big_from)) {
      big_to->value.bigintvalue = take_bigint(big_from);
      big_from->value.bigintvalue = NULL;
    } else {
      new_size = take_bigint(big_from)->size;
      if (!ALLOC_BIG(new_big, new_size)) {
        big_to->value.bigintvalue = NULL;
        return(raise_exception(SYS_MEM_EXCEPTION));
      } /* if */
      big_to->value.bigintvalue = new_big;
      new_big->size = new_size;
      memcpy(new_big->bigdigits, take_bigint(big_from)->bigdigits,
          (SIZE_TYPE) new_size * sizeof(bigdigittype));
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* big_create */



#ifdef ANSI_C

objecttype big_decr (listtype arguments)
#else

objecttype big_decr (arguments)
listtype arguments;
#endif

  {
    objecttype big_variable;

  /* big_decr */
    big_variable = arg_1(arguments);
    isit_bigint(big_variable);
    is_variable(big_variable);
    bigDecr(&take_bigint(big_variable));
    return(SYS_EMPTY_OBJECT);
  } /* big_decr */



#ifdef ANSI_C

objecttype big_destr (listtype arguments)
#else

objecttype big_destr (arguments)
listtype arguments;
#endif

  {
    biginttype old_bigint;

  /* big_destr */
    isit_bigint(arg_1(arguments));
    old_bigint = take_bigint(arg_1(arguments));
    if (old_bigint != NULL) {
      FREE_BIG(old_bigint, old_bigint->size);
      arg_1(arguments)->value.bigintvalue = NULL;
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* big_destr */



#ifdef ANSI_C

objecttype big_div (listtype arguments)
#else

objecttype big_div (arguments)
listtype arguments;
#endif

  { /* big_div */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return(bld_bigint_temp(
        bigDiv(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)))));
  } /* big_div */



#ifdef ANSI_C

objecttype big_eq (listtype arguments)
#else

objecttype big_eq (arguments)
listtype arguments;
#endif

  {
    biginttype big1;
    biginttype big2;

  /* big_eq */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    big1 = take_bigint(arg_1(arguments));
    big2 = take_bigint(arg_3(arguments));
    if (big1->size == big2->size &&
        memcmp(big1->bigdigits, big2->bigdigits,
        (SIZE_TYPE) big1->size * sizeof(bigdigittype)) == 0) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* big_eq */



#ifdef ANSI_C

objecttype big_ge (listtype arguments)
#else

objecttype big_ge (arguments)
listtype arguments;
#endif

  { /* big_ge */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    if (bigCmp(take_bigint(arg_1(arguments)),
        take_bigint(arg_3(arguments))) >= 0) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* big_ge */



#ifdef ANSI_C

objecttype big_grow (listtype arguments)
#else

objecttype big_grow (arguments)
listtype arguments;
#endif

  { /* big_grow */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigGrow(&take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* big_grow */



#ifdef ANSI_C

objecttype big_gt (listtype arguments)
#else

objecttype big_gt (arguments)
listtype arguments;
#endif

  { /* big_gt */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    if (bigCmp(take_bigint(arg_1(arguments)),
        take_bigint(arg_3(arguments))) > 0) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* big_gt */



#ifdef ANSI_C

objecttype big_hashcode (listtype arguments)
#else

objecttype big_hashcode (arguments)
listtype arguments;
#endif

  {
    biginttype big1;
    inttype result;

  /* big_hashcode */
    isit_bigint(arg_1(arguments));
    big1 = take_bigint(arg_1(arguments));
    result = big1->bigdigits[0] << 5 ^ big1->size << 3 ^ big1->bigdigits[big1->size - 1];
    return(bld_int_temp(result));
  } /* big_hashcode */



#ifdef ANSI_C

objecttype big_iconv (listtype arguments)
#else

objecttype big_iconv (arguments)
listtype arguments;
#endif

  { /* big_iconv */
    isit_int(arg_3(arguments));
    return(bld_bigint_temp(bigIConv(
        take_int(arg_3(arguments)))));
  } /* big_iconv */



#ifdef ANSI_C

objecttype big_incr (listtype arguments)
#else

objecttype big_incr (arguments)
listtype arguments;
#endif

  {
    objecttype big_variable;

  /* big_incr */
    big_variable = arg_1(arguments);
    isit_bigint(big_variable);
    is_variable(big_variable);
    bigIncr(&take_bigint(big_variable));
    return(SYS_EMPTY_OBJECT);
  } /* big_incr */



#ifdef ANSI_C

objecttype big_ipow (listtype arguments)
#else

objecttype big_ipow (arguments)
listtype arguments;
#endif

  { /* big_ipow */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_bigint_temp(
        bigIPow(take_bigint(arg_1(arguments)), take_int(arg_3(arguments)))));
  } /* big_ipow */



#ifdef ANSI_C

objecttype big_le (listtype arguments)
#else

objecttype big_le (arguments)
listtype arguments;
#endif

  { /* big_le */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    if (bigCmp(take_bigint(arg_1(arguments)),
        take_bigint(arg_3(arguments))) <= 0) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* big_le */



#ifdef ANSI_C

objecttype big_log2 (listtype arguments)
#else

objecttype big_log2 (arguments)
listtype arguments;
#endif

  { /* big_log2 */
    isit_bigint(arg_1(arguments));
    return(bld_bigint_temp(bigLog2(take_bigint(arg_1(arguments)))));
  } /* big_log2 */



#ifdef ANSI_C

objecttype big_lowest_set_bit (listtype arguments)
#else

objecttype big_lowest_set_bit (arguments)
listtype arguments;
#endif

  { /* big_lowest_set_bit */
    isit_bigint(arg_1(arguments));
    return(bld_int_temp(
        bigLowestSetBit(take_bigint(arg_1(arguments)))));
  } /* big_lowest_set_bit */



#ifdef ANSI_C

objecttype big_lshift (listtype arguments)
#else

objecttype big_lshift (arguments)
listtype arguments;
#endif

  { /* big_lshift */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_bigint_temp(
        bigLShift(take_bigint(arg_1(arguments)), take_int(arg_3(arguments)))));
  } /* big_lshift */



#ifdef ANSI_C

objecttype big_lshift_assign (listtype arguments)
#else

objecttype big_lshift_assign (arguments)
listtype arguments;
#endif

  { /* big_lshift_assign */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigLShiftAssign(&take_bigint(arg_1(arguments)), take_int(arg_3(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* big_lshift_assign */



#ifdef ANSI_C

objecttype big_lt (listtype arguments)
#else

objecttype big_lt (arguments)
listtype arguments;
#endif

  { /* big_lt */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    if (bigCmp(take_bigint(arg_1(arguments)),
        take_bigint(arg_3(arguments))) < 0) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* big_lt */



#ifdef ANSI_C

objecttype big_mcpy (listtype arguments)
#else

objecttype big_mcpy (arguments)
listtype arguments;
#endif

  { /* big_mcpy */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigMCpy(&take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* big_mcpy */



#ifdef ANSI_C

objecttype big_mdiv (listtype arguments)
#else

objecttype big_mdiv (arguments)
listtype arguments;
#endif

  { /* big_mdiv */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return(bld_bigint_temp(
        bigMDiv(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)))));
  } /* big_mdiv */



#ifdef ANSI_C

objecttype big_minus (listtype arguments)
#else

objecttype big_minus (arguments)
listtype arguments;
#endif

  { /* big_minus */
    isit_bigint(arg_2(arguments));
    return(bld_bigint_temp(
        bigMinus(take_bigint(arg_2(arguments)))));
  } /* big_minus */



#ifdef ANSI_C

objecttype big_mod (listtype arguments)
#else

objecttype big_mod (arguments)
listtype arguments;
#endif

  { /* big_mod */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return(bld_bigint_temp(
        bigMod(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)))));
  } /* big_mod */



#ifdef ANSI_C

objecttype big_mult (listtype arguments)
#else

objecttype big_mult (arguments)
listtype arguments;
#endif

  { /* big_mult */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return(bld_bigint_temp(
        bigMult(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)))));
  } /* big_mult */



#ifdef ANSI_C

objecttype big_ne (listtype arguments)
#else

objecttype big_ne (arguments)
listtype arguments;
#endif

  {
    biginttype big1;
    biginttype big2;

  /* big_ne */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    big1 = take_bigint(arg_1(arguments));
    big2 = take_bigint(arg_3(arguments));
    if (big1->size != big2->size ||
        memcmp(big1->bigdigits, big2->bigdigits,
        (SIZE_TYPE) big1->size * sizeof(bigdigittype)) != 0) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* big_ne */



#ifdef ANSI_C

objecttype big_odd (listtype arguments)
#else

objecttype big_odd (arguments)
listtype arguments;
#endif

  { /* big_odd */
    isit_bigint(arg_1(arguments));
    if (take_bigint(arg_1(arguments))->bigdigits[0] & 1) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* big_odd */



#ifdef ANSI_C

objecttype big_ord (listtype arguments)
#else

objecttype big_ord (arguments)
listtype arguments;
#endif

  { /* big_ord */
    isit_bigint(arg_1(arguments));
    return(bld_int_temp(
        bigOrd(take_bigint(arg_1(arguments)))));
  } /* big_ord */



#ifdef ANSI_C

objecttype big_parse (listtype arguments)
#else

objecttype big_parse (arguments)
listtype arguments;
#endif

  { /* big_parse */
    isit_stri(arg_3(arguments));
    return(bld_bigint_temp(bigParse(
        take_stri(arg_3(arguments)))));
  } /* big_parse */



#ifdef ANSI_C

objecttype big_plus (listtype arguments)
#else

objecttype big_plus (arguments)
listtype arguments;
#endif

  {
    biginttype big1;
    biginttype result;

  /* big_plus */
    isit_bigint(arg_2(arguments));
    big1 = take_bigint(arg_2(arguments));
    if (TEMP_OBJECT(arg_2(arguments))) {
      result = big1;
      arg_2(arguments)->value.bigintvalue = NULL;
      return(bld_bigint_temp(result));
    } else {
      if (!ALLOC_BIG(result, big1->size)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        result->size = big1->size;
        memcpy(result->bigdigits, big1->bigdigits,
            (SIZE_TYPE) big1->size * sizeof(bigdigittype));
        return(bld_bigint_temp(result));
      } /* if */
    } /* if */
  } /* big_plus */



#ifdef ANSI_C

objecttype big_pred (listtype arguments)
#else

objecttype big_pred (arguments)
listtype arguments;
#endif

  { /* big_pred */
    isit_bigint(arg_1(arguments));
    return(bld_bigint_temp(
        bigPred(take_bigint(arg_1(arguments)))));
  } /* big_pred */



#ifdef ANSI_C

objecttype big_rand (listtype arguments)
#else

objecttype big_rand (arguments)
listtype arguments;
#endif

  { /* big_rand */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_2(arguments));
    return(bld_bigint_temp(
        bigRand(take_bigint(arg_1(arguments)), take_bigint(arg_2(arguments)))));
  } /* big_rand */



#ifdef ANSI_C

objecttype big_rem (listtype arguments)
#else

objecttype big_rem (arguments)
listtype arguments;
#endif

  { /* big_rem */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return(bld_bigint_temp(
        bigRem(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)))));
  } /* big_rem */



#ifdef ANSI_C

objecttype big_rshift (listtype arguments)
#else

objecttype big_rshift (arguments)
listtype arguments;
#endif

  { /* big_rshift */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    return(bld_bigint_temp(
        bigRShift(take_bigint(arg_1(arguments)), take_int(arg_3(arguments)))));
  } /* big_rshift */



#ifdef ANSI_C

objecttype big_rshift_assign (listtype arguments)
#else

objecttype big_rshift_assign (arguments)
listtype arguments;
#endif

  { /* big_rshift_assign */
    isit_bigint(arg_1(arguments));
    isit_int(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigRShiftAssign(&take_bigint(arg_1(arguments)), take_int(arg_3(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* big_rshift_assign */



#ifdef ANSI_C

objecttype big_sbtr (listtype arguments)
#else

objecttype big_sbtr (arguments)
listtype arguments;
#endif

  { /* big_sbtr */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return(bld_bigint_temp(
        bigSbtr(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)))));
  } /* big_sbtr */



#ifdef ANSI_C

objecttype big_shrink (listtype arguments)
#else

objecttype big_shrink (arguments)
listtype arguments;
#endif

  { /* big_shrink */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    is_variable(arg_1(arguments));
    bigShrink(&take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* big_shrink */



#ifdef ANSI_C

objecttype big_str (listtype arguments)
#else

objecttype big_str (arguments)
listtype arguments;
#endif

  { /* big_str */
    isit_bigint(arg_1(arguments));
    return(bld_stri_temp(bigStr(
        take_bigint(arg_1(arguments)))));
  } /* big_str */



#ifdef ANSI_C

objecttype big_succ (listtype arguments)
#else

objecttype big_succ (arguments)
listtype arguments;
#endif

  { /* big_succ */
    isit_bigint(arg_1(arguments));
    return(bld_bigint_temp(
        bigSucc(take_bigint(arg_1(arguments)))));
  } /* big_succ */



#ifdef ANSI_C

objecttype big_value (listtype arguments)
#else

objecttype big_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;
    biginttype big1;
    biginttype result;

  /* big_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != BIGINTOBJECT) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } else {
      big1 = take_bigint(obj_arg);
      if (!ALLOC_BIG(result, big1->size)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        result->size = big1->size;
        memcpy(result->bigdigits, big1->bigdigits,
            (SIZE_TYPE) (result->size * sizeof(bigdigittype)));
        return(bld_bigint_temp(result));
      } /* if */
    } /* if */
  } /* big_value */
