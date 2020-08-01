/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2009  Thomas Mertes                        */
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
/*  File: seed7/src/chrlib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions for the char type.               */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "ctype.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "objutl.h"
#include "runerr.h"
#include "chr_rtl.h"

#undef EXTERN
#define EXTERN
#include "chrlib.h"



objecttype chr_chr (listtype arguments)

  {
    inttype number;

  /* chr_chr */
    isit_int(arg_1(arguments));
    number = take_int(arg_1(arguments));
#if INTTYPE_SIZE == 64
    if (number < INT32TYPE_MIN || number > INT32TYPE_MAX) {
      return raise_exception(SYS_RNG_EXCEPTION) ;
    } else {
      return bld_char_temp((chartype) number);
    } /* if */
#else
    return bld_char_temp((chartype) number);
#endif
  } /* chr_chr */



objecttype chr_clit (listtype arguments)

  { /* chr_clit */
    isit_char(arg_1(arguments));
    return bld_stri_temp(
        chrCLit(take_char(arg_1(arguments))));
  } /* chr_clit */



objecttype chr_cmp (listtype arguments)

  {
    chartype char1;
    chartype char2;
    inttype result;

  /* chr_cmp */
    isit_char(arg_1(arguments));
    isit_char(arg_2(arguments));
    char1 = take_char(arg_1(arguments));
    char2 = take_char(arg_2(arguments));
    if (char1 < char2) {
      result = -1;
    } else if (char1 > char2) {
      result = 1;
    } else {
      result = 0;
    } /* if */
    return bld_int_temp(result);
  } /* chr_cmp */



objecttype chr_conv (listtype arguments)

  { /* chr_conv */
    isit_char(arg_3(arguments));
    return bld_char_temp(take_char(arg_3(arguments)));
  } /* chr_conv */



objecttype chr_cpy (listtype arguments)

  {
    objecttype ch_variable;

  /* chr_cpy */
    ch_variable = arg_1(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    isit_char(arg_3(arguments));
    ch_variable->value.charvalue = take_char(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* chr_cpy */



objecttype chr_create (listtype arguments)

  { /* chr_create */
    isit_char(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), CHAROBJECT);
    arg_1(arguments)->value.charvalue = take_char(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* chr_create */



objecttype chr_decr (listtype arguments)

  {
    objecttype ch_variable;

  /* chr_decr */
    ch_variable = arg_1(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    ch_variable->value.charvalue =
        (chartype) (((int) take_char(ch_variable)) - 1);
    return SYS_EMPTY_OBJECT;
  } /* chr_decr */



objecttype chr_eq (listtype arguments)

  { /* chr_eq */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) ==
        take_char(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* chr_eq */



objecttype chr_ge (listtype arguments)

  { /* chr_ge */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) >=
        take_char(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* chr_ge */



objecttype chr_gt (listtype arguments)

  { /* chr_gt */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) >
        take_char(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* chr_gt */



objecttype chr_hashcode (listtype arguments)

  { /* chr_hashcode */
    isit_char(arg_1(arguments));
    return bld_int_temp((inttype)
        ((schartype) take_char(arg_1(arguments))));
  } /* chr_hashcode */



objecttype chr_iconv (listtype arguments)

  { /* chr_iconv */
    isit_int(arg_3(arguments));
    return bld_char_temp((chartype) take_int(arg_3(arguments)));
  } /* chr_iconv */



objecttype chr_incr (listtype arguments)

  {
    objecttype ch_variable;

  /* chr_incr */
    ch_variable = arg_1(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    ch_variable->value.charvalue =
        (chartype) (((int) take_char(ch_variable)) + 1);
    return SYS_EMPTY_OBJECT;
  } /* chr_incr */



objecttype chr_le (listtype arguments)

  { /* chr_le */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) <=
        take_char(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* chr_le */



objecttype chr_low (listtype arguments)

  {
    chartype ch;

  /* chr_low */
    isit_char(arg_1(arguments));
    ch = take_char(arg_1(arguments));
    if (ch <= (chartype) 255) {
      return bld_char_temp((chartype) tolower((int) ch));
    } else {
      return bld_char_temp(ch);
    } /* if */
  } /* chr_low */



objecttype chr_lt (listtype arguments)

  { /* chr_lt */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) <
        take_char(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* chr_lt */



objecttype chr_ne (listtype arguments)

  { /* chr_ne */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) !=
        take_char(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* chr_ne */



objecttype chr_ord (listtype arguments)

  { /* chr_ord */
    isit_char(arg_1(arguments));
    return bld_int_temp((inttype)
        ((schartype) take_char(arg_1(arguments))));
  } /* chr_ord */



objecttype chr_pred (listtype arguments)

  { /* chr_pred */
    isit_char(arg_1(arguments));
    return bld_char_temp((chartype)
        (((int) take_char(arg_1(arguments))) - 1));
  } /* chr_pred */



objecttype chr_str (listtype arguments)

  {
    stritype result;

  /* chr_str */
    isit_char(arg_1(arguments));
    if (!ALLOC_STRI_SIZE_OK(result, 1)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->size = 1;
      result->mem[0] = (strelemtype) take_char(arg_1(arguments));
      return bld_stri_temp(result);
    } /* if */
  } /* chr_str */



objecttype chr_succ (listtype arguments)

  { /* chr_succ */
    isit_char(arg_1(arguments));
    return bld_char_temp((chartype)
        (((int) take_char(arg_1(arguments))) + 1));
  } /* chr_succ */



objecttype chr_up (listtype arguments)

  {
    chartype ch;

  /* chr_up */
    isit_char(arg_1(arguments));
    ch = take_char(arg_1(arguments));
    if (ch <= (chartype) 255) {
      return bld_char_temp((chartype) toupper((int) ch));
    } else {
      return bld_char_temp(ch);
    } /* if */
  } /* chr_up */



objecttype chr_value (listtype arguments)

  {
    objecttype obj_arg;

  /* chr_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != CHAROBJECT) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_char_temp(take_char(obj_arg));
    } /* if */
  } /* chr_value */
