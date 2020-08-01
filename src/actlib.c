/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
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
/*  File: seed7/src/actlib.c                                        */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: All primitive actions for the action type.             */
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
#include "striutl.h"
#include "actutl.h"
#include "syvarutl.h"
#include "objutl.h"
#include "name.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "actlib.h"



objecttype act_cpy (listtype arguments)

  {
    objecttype act_variable;

  /* act_cpy */
    act_variable = arg_1(arguments);
    isit_action(act_variable);
    is_variable(act_variable);
    isit_action(arg_3(arguments));
    act_variable->value.actvalue = take_action(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* act_cpy */



objecttype act_create (listtype arguments)

  {
    objecttype act_to;
    objecttype act_from;

  /* act_create */
    act_to = arg_1(arguments);
    act_from = arg_3(arguments);
    isit_action(act_from);
    disconnect_param_entities(act_to);
    SET_CATEGORY_OF_OBJ(act_to, ACTOBJECT);
    act_to->value.actvalue = take_action(act_from);
    return SYS_EMPTY_OBJECT;
  } /* act_create */



objecttype act_gen (listtype arguments)

  {
    stritype stri;
    acttype result;

  /* act_gen */
    isit_stri(arg_2(arguments));
    stri = take_stri(arg_2(arguments));
    if (!find_action(stri, &result)) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_action_temp(result);
    } /* if */
  } /* act_gen */



objecttype act_iconv (listtype arguments)

  {
    inttype ordinal;

  /* act_iconv */
    isit_int(arg_3(arguments));
    ordinal = take_int(arg_3(arguments));
    if (ordinal < 0 || ordinal >= act_table.size) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_action_temp(act_table.primitive[ordinal].action);
    } /* if */
  } /* act_iconv */



objecttype act_illegal (listtype arguments)

  { /* act_illegal */
    return raise_exception(SYS_ACT_ILLEGAL_EXCEPTION);
  } /* act_illegal */



objecttype act_ord (listtype arguments)

  {
    primacttype primact;
    inttype result;

  /* act_ord */
    isit_action(arg_1(arguments));
    primact = get_primact(take_action(arg_1(arguments)));
    result = primact - act_table.primitive;
    return bld_int_temp(result);
  } /* act_ord */



objecttype act_str (listtype arguments)

  {
    stritype result;

  /* act_str */
    isit_action(arg_1(arguments));
    result = cstri_to_stri(get_primact(take_action(arg_1(arguments)))->name);
    if (result == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_stri_temp(result);
    } /* if */
  } /* act_str */



objecttype act_value (listtype arguments)

  {
    objecttype obj_arg;

  /* act_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != ACTOBJECT) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_action_temp(take_action(obj_arg));
    } /* if */
  } /* act_value */
