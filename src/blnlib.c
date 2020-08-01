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
#include "objutl.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "blnlib.h"



objecttype bln_and (listtype arguments)

  {
    objecttype result;

  /* bln_and */
    isit_bool(arg_1(arguments));
    if (take_bool(arg_1(arguments)) == SYS_FALSE_OBJECT) {
      return SYS_FALSE_OBJECT;
    } else {
      result = evaluate(arg_3(arguments));
      if (!fail_flag) {
        isit_bool(result);
      } /* if */
      return result ;
    } /* if */
  } /* bln_and */



objecttype bln_cpy (listtype arguments)

  {
    objecttype bool_variable;

  /* bln_cpy */
    bool_variable = arg_1(arguments);
    isit_bool(bool_variable);
    is_variable(bool_variable);
    isit_bool(arg_3(arguments));
    bool_variable->value.objvalue = take_bool(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* bln_cpy */



objecttype bln_create (listtype arguments)

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
    return SYS_EMPTY_OBJECT;
  } /* bln_create */



objecttype bln_ge (listtype arguments)

  { /* bln_ge */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      return SYS_TRUE_OBJECT;
    } else {
      if (take_bool(arg_3(arguments)) == SYS_TRUE_OBJECT) {
        return SYS_FALSE_OBJECT;
      } else {
        return SYS_TRUE_OBJECT;
      } /* if */
    } /* if */
  } /* bln_ge */



objecttype bln_gt (listtype arguments)

  { /* bln_gt */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == SYS_FALSE_OBJECT) {
      return SYS_FALSE_OBJECT;
    } else {
      if (take_bool(arg_3(arguments)) == SYS_TRUE_OBJECT) {
        return SYS_FALSE_OBJECT;
      } else {
        return SYS_TRUE_OBJECT;
      } /* if */
    } /* if */
  } /* bln_gt */



objecttype bln_iconv (listtype arguments)

  { /* bln_iconv */
    isit_int(arg_3(arguments));
    if (take_int(arg_3(arguments)) & 1) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* bln_iconv */



objecttype bln_le (listtype arguments)

  { /* bln_le */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == SYS_FALSE_OBJECT) {
      return SYS_TRUE_OBJECT;
    } else {
      return take_bool(arg_3(arguments));
    } /* if */
  } /* bln_le */



objecttype bln_lt (listtype arguments)

  { /* bln_lt */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      return SYS_FALSE_OBJECT;
    } else {
      return take_bool(arg_3(arguments));
    } /* if */
  } /* bln_lt */



objecttype bln_not (listtype arguments)

  { /* bln_not */
    isit_bool(arg_2(arguments));
    if (take_bool(arg_2(arguments)) == SYS_TRUE_OBJECT) {
      return SYS_FALSE_OBJECT;
    } else {
      return SYS_TRUE_OBJECT;
    } /* if */
  } /* bln_not */



objecttype bln_or (listtype arguments)

  {
    objecttype result;

  /* bln_or */
    isit_bool(arg_1(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      return SYS_TRUE_OBJECT;
    } else {
      result = evaluate(arg_3(arguments));
      if (!fail_flag) {
        isit_bool(result);
      } /* if */
      return result;
    } /* if */
  } /* bln_or */



objecttype bln_ord (listtype arguments)

  { /* bln_ord */
    isit_bool(arg_1(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      return bld_int_temp(1);
    } else {
      return bld_int_temp(0);
    } /* if */
  } /* bln_ord */
