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
/*  File: seed7/src/blnlib.c                                        */
/*  Changes: 1999  Thomas Mertes                                    */
/*  Content: All primitive actions for the boolean type.            */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "exec.h"
#include "runerr.h"
#include "memory.h"

#undef EXTERN
#define EXTERN
#include "blnlib.h"



#ifdef ANSI_C

objecttype bln_and (listtype arguments)
#else

objecttype bln_and (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* bln_and */
    isit_bool(arg_1(arguments));
    if (take_bool(arg_1(arguments)) == SYS_FALSE_OBJECT) {
      return(SYS_FALSE_OBJECT);
    } else {
      result = evaluate(arg_3(arguments));
      if (!fail_flag) {
        isit_bool(result);
      } /* if */
      return(result);
    } /* if */
  } /* bln_and */



#ifdef ANSI_C

objecttype bln_cpy (listtype arguments)
#else

objecttype bln_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype bool_variable;

  /* bln_cpy */
    bool_variable = arg_1(arguments);
    isit_bool(bool_variable);
    is_variable(bool_variable);
    isit_bool(arg_3(arguments));
    bool_variable->value.objvalue = take_bool(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* bln_cpy */



#ifdef ANSI_C

objecttype bln_create (listtype arguments)
#else

objecttype bln_create (arguments)
listtype arguments;
#endif

  {
    objecttype bool_to;
    objecttype bool_from;

  /* bln_create */
    bool_to = arg_1(arguments);
    isit_bool(arg_3(arguments));
    bool_from = take_bool(arg_3(arguments));
    if (VAR_OBJECT(bool_to)) {
      SET_CATEGORY_OF_OBJ(bool_to, VARENUMOBJECT);
    } else {
      SET_CATEGORY_OF_OBJ(bool_to, CONSTENUMOBJECT);
    } /* if */
    bool_to->value.objvalue = bool_from;
    return(SYS_EMPTY_OBJECT);
  } /* bln_create */



#ifdef ANSI_C

objecttype bln_ge (listtype arguments)
#else

objecttype bln_ge (arguments)
listtype arguments;
#endif

  { /* bln_ge */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      return(SYS_TRUE_OBJECT);
    } else {
      if (take_bool(arg_3(arguments)) == SYS_TRUE_OBJECT) {
        return(SYS_FALSE_OBJECT);
      } else {
        return(SYS_TRUE_OBJECT);
      } /* if */
    } /* if */
  } /* bln_ge */



#ifdef ANSI_C

objecttype bln_gt (listtype arguments)
#else

objecttype bln_gt (arguments)
listtype arguments;
#endif

  { /* bln_gt */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == SYS_FALSE_OBJECT) {
      return(SYS_FALSE_OBJECT);
    } else {
      if (take_bool(arg_3(arguments)) == SYS_TRUE_OBJECT) {
        return(SYS_FALSE_OBJECT);
      } else {
        return(SYS_TRUE_OBJECT);
      } /* if */
    } /* if */
  } /* bln_gt */



#ifdef ANSI_C

objecttype bln_iconv (listtype arguments)
#else

objecttype bln_iconv (arguments)
listtype arguments;
#endif

  { /* bln_iconv */
    isit_int(arg_3(arguments));
    if (take_int(arg_3(arguments)) & 1) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* bln_iconv */



#ifdef ANSI_C

objecttype bln_le (listtype arguments)
#else

objecttype bln_le (arguments)
listtype arguments;
#endif

  { /* bln_le */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == SYS_FALSE_OBJECT) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(take_bool(arg_3(arguments)));
    } /* if */
  } /* bln_le */



#ifdef ANSI_C

objecttype bln_lt (listtype arguments)
#else

objecttype bln_lt (arguments)
listtype arguments;
#endif

  { /* bln_lt */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      return(SYS_FALSE_OBJECT);
    } else {
      return(take_bool(arg_3(arguments)));
    } /* if */
  } /* bln_lt */



#ifdef ANSI_C

objecttype bln_not (listtype arguments)
#else

objecttype bln_not (arguments)
listtype arguments;
#endif

  { /* bln_not */
    isit_bool(arg_2(arguments));
    if (take_bool(arg_2(arguments)) == SYS_TRUE_OBJECT) {
      return(SYS_FALSE_OBJECT);
    } else {
      return(SYS_TRUE_OBJECT);
    } /* if */
  } /* bln_not */



#ifdef ANSI_C

objecttype bln_or (listtype arguments)
#else

objecttype bln_or (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* bln_or */
    isit_bool(arg_1(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      return(SYS_TRUE_OBJECT);
    } else {
      result = evaluate(arg_3(arguments));
      if (!fail_flag) {
        isit_bool(result);
      } /* if */
      return(result);
    } /* if */
  } /* bln_or */



#ifdef ANSI_C

objecttype bln_ord (listtype arguments)
#else

objecttype bln_ord (arguments)
listtype arguments;
#endif

  { /* bln_ord */
    isit_bool(arg_1(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      return(bld_int_temp(1));
    } else {
      return(bld_int_temp(0));
    } /* if */
  } /* bln_ord */
