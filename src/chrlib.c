/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2004  Thomas Mertes                        */
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
/*  File: seed7/src/chrlib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions for the char type.               */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "ctype.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "runerr.h"
#include "memory.h"

#undef EXTERN
#define EXTERN
#include "chrlib.h"



#ifdef ANSI_C

objecttype chr_chr (listtype arguments)
#else

objecttype chr_chr (arguments)
listtype arguments;
#endif

  { /* chr_chr */
    isit_int(arg_1(arguments));
    return(bld_char_temp((chartype) take_int(arg_1(arguments))));
  } /* chr_chr */



#ifdef ANSI_C

objecttype chr_cmp (listtype arguments)
#else

objecttype chr_cmp (arguments)
listtype arguments;
#endif

  {
    chartype char1;
    chartype char2;

  /* chr_cmp */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    char1 = take_char(arg_1(arguments));
    char2 = take_char(arg_2(arguments));
    if (char1 < char2) {
      return(bld_int_temp(-1));
    } else if (char1 > char2) {
      return(bld_int_temp(1));
    } else {
      return(bld_int_temp(0));
    } /* if */
  } /* chr_cmp */



#ifdef ANSI_C

objecttype chr_conv (listtype arguments)
#else

objecttype chr_conv (arguments)
listtype arguments;
#endif

  { /* chr_conv */
    isit_char(arg_3(arguments));
    return(bld_char_temp(take_char(arg_3(arguments))));
  } /* chr_conv */



#ifdef ANSI_C

objecttype chr_cpy (listtype arguments)
#else

objecttype chr_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* chr_cpy */
    ch_variable = arg_1(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    isit_char(arg_3(arguments));
    ch_variable->value.charvalue = take_char(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* chr_cpy */



#ifdef ANSI_C

objecttype chr_create (listtype arguments)
#else

objecttype chr_create (arguments)
listtype arguments;
#endif

  { /* chr_create */
    isit_char(arg_3(arguments));
    SET_CLASS_OF_OBJ(arg_1(arguments), CHAROBJECT);
    arg_1(arguments)->value.charvalue = take_char(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* chr_create */



#ifdef ANSI_C

objecttype chr_decr (listtype arguments)
#else

objecttype chr_decr (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* chr_decr */
    ch_variable = arg_1(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    ch_variable->value.charvalue =
        (chartype) (((int) take_char(ch_variable)) - 1);
    return(SYS_EMPTY_OBJECT);
  } /* chr_decr */



#ifdef ANSI_C

objecttype chr_eq (listtype arguments)
#else

objecttype chr_eq (arguments)
listtype arguments;
#endif

  { /* chr_eq */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) ==
        take_char(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* chr_eq */



#ifdef ANSI_C

objecttype chr_ge (listtype arguments)
#else

objecttype chr_ge (arguments)
listtype arguments;
#endif

  { /* chr_ge */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) >=
        take_char(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* chr_ge */



#ifdef ANSI_C

objecttype chr_gt (listtype arguments)
#else

objecttype chr_gt (arguments)
listtype arguments;
#endif

  { /* chr_gt */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) >
        take_char(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* chr_gt */



#ifdef ANSI_C

objecttype chr_iconv (listtype arguments)
#else

objecttype chr_iconv (arguments)
listtype arguments;
#endif

  { /* chr_iconv */
    isit_int(arg_3(arguments));
    return(bld_char_temp((chartype) take_int(arg_3(arguments))));
  } /* chr_iconv */



#ifdef ANSI_C

objecttype chr_hashcode (listtype arguments)
#else

objecttype chr_hashcode (arguments)
listtype arguments;
#endif

  { /* chr_hashcode */
    isit_char(arg_1(arguments));
    return(bld_int_temp((inttype) take_char(arg_1(arguments))));
  } /* chr_hashcode */



#ifdef ANSI_C

objecttype chr_incr (listtype arguments)
#else

objecttype chr_incr (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* chr_incr */
    ch_variable = arg_1(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    ch_variable->value.charvalue =
        (chartype) (((int) take_char(ch_variable)) + 1);
    return(SYS_EMPTY_OBJECT);
  } /* chr_incr */



#ifdef ANSI_C

objecttype chr_le (listtype arguments)
#else

objecttype chr_le (arguments)
listtype arguments;
#endif

  { /* chr_le */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) <=
        take_char(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* chr_le */



#ifdef ANSI_C

objecttype chr_low (listtype arguments)
#else

objecttype chr_low (arguments)
listtype arguments;
#endif

  {
    chartype ch;

  /* chr_low */
    isit_char(arg_1(arguments));
    ch = take_char(arg_1(arguments));
    if (ch <= (chartype) 255) {
      return(bld_char_temp((chartype) tolower((int) ch)));
    } else {
      return(bld_char_temp(ch));
    } /* if */
  } /* chr_low */



#ifdef ANSI_C

objecttype chr_lt (listtype arguments)
#else

objecttype chr_lt (arguments)
listtype arguments;
#endif

  { /* chr_lt */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) <
        take_char(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* chr_lt */



#ifdef ANSI_C

objecttype chr_ne (listtype arguments)
#else

objecttype chr_ne (arguments)
listtype arguments;
#endif

  { /* chr_ne */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) !=
        take_char(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* chr_ne */



#ifdef ANSI_C

objecttype chr_ord (listtype arguments)
#else

objecttype chr_ord (arguments)
listtype arguments;
#endif

  { /* chr_ord */
    isit_char(arg_1(arguments));
    return(bld_int_temp((inttype) take_char(arg_1(arguments))));
  } /* chr_ord */



#ifdef ANSI_C

objecttype chr_pred (listtype arguments)
#else

objecttype chr_pred (arguments)
listtype arguments;
#endif

  { /* chr_pred */
    isit_char(arg_1(arguments));
    return(bld_char_temp((chartype)
        (((int) take_char(arg_1(arguments))) - 1)));
  } /* chr_pred */



#ifdef ANSI_C

objecttype chr_str (listtype arguments)
#else

objecttype chr_str (arguments)
listtype arguments;
#endif

  {
    stritype result;

  /* chr_str */
    isit_char(arg_1(arguments));
#ifndef WIDE_CHAR_STRINGS
    if (take_char(arg_1(arguments)) > (chartype) 255) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } else {
#endif
      if (!ALLOC_STRI(result, 1)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        COUNT_STRI(1);
        result->size = 1;
        result->mem[0] = (strelemtype) take_char(arg_1(arguments));
        return(bld_stri_temp(result));
      } /* if */
#ifndef WIDE_CHAR_STRINGS
    } /* if */
#endif
  } /* chr_str */



#ifdef ANSI_C

objecttype chr_succ (listtype arguments)
#else

objecttype chr_succ (arguments)
listtype arguments;
#endif

  { /* chr_succ */
    isit_char(arg_1(arguments));
    return(bld_char_temp((chartype)
        (((int) take_char(arg_1(arguments))) + 1)));
  } /* chr_succ */



#ifdef ANSI_C

objecttype chr_up (listtype arguments)
#else

objecttype chr_up (arguments)
listtype arguments;
#endif

  {
    chartype ch;

  /* chr_up */
    isit_char(arg_1(arguments));
    ch = take_char(arg_1(arguments));
    if (ch <= (chartype) 255) {
      return(bld_char_temp((chartype) toupper((int) ch)));
    } else {
      return(bld_char_temp(ch));
    } /* if */
  } /* chr_up */



#ifdef ANSI_C

objecttype chr_value (listtype arguments)
#else

objecttype chr_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;

  /* chr_value */
    isit_reference(arg_3(arguments));
    obj_arg = take_reference(arg_3(arguments));
    isit_char(obj_arg);
    return(bld_char_temp(take_char(obj_arg)));
  } /* chr_value */
