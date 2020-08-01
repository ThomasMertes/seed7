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
/*  File: seed7/src/enulib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions for the enumeration values.      */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "traceutl.h"
#include "objutl.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "enulib.h"



objectType enu_conv (listType arguments)

  { /* enu_conv */
    isit_enum(arg_3(arguments));
    return take_enum(arg_3(arguments));
  } /* enu_conv */



objectType enu_cpy (listType arguments)

  {
    objectType enum_variable;

  /* enu_cpy */
    enum_variable = arg_1(arguments);
    isit_enum(enum_variable);
    is_variable(enum_variable);
    isit_enum(arg_3(arguments));
    enum_variable->value.objValue = take_enum(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* enu_cpy */



objectType enu_create (listType arguments)

  {
    objectType enum_to;
    objectType enum_from;

  /* enu_create */
    enum_to = arg_1(arguments);
    enum_from = take_enum(arg_3(arguments));
    if (VAR_OBJECT(enum_to)) {
      SET_CATEGORY_OF_OBJ(enum_to, VARENUMOBJECT);
    } else {
      SET_CATEGORY_OF_OBJ(enum_to, CONSTENUMOBJECT);
    } /* if */
    enum_to->value.objValue = enum_from;
    return SYS_EMPTY_OBJECT;
  } /* enu_create */



objectType enu_eq (listType arguments)

  {
    objectType obj_arg1;
    objectType obj_arg2;
    objectType result;

  /* enu_eq */
    obj_arg1 = take_enum(arg_1(arguments));
    obj_arg2 = take_enum(arg_3(arguments));
    if (obj_arg1 == obj_arg2) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* enu_eq */



objectType enu_genlit (listType arguments)

  {
    objectType enum_to;

  /* enu_genlit */
    enum_to = arg_1(arguments);
    SET_CATEGORY_OF_OBJ(enum_to, ENUMLITERALOBJECT);
    enum_to->value.nodeValue = NULL;
    return SYS_EMPTY_OBJECT;
  } /* enu_genlit */



objectType enu_iconv2 (listType arguments)

  {
    intType ordinal;
    intType number;
    listType list_element;
    objectType result;

  /* enu_iconv2 */
    isit_int(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    ordinal = take_int(arg_1(arguments));
    list_element = take_reflist(arg_2(arguments));
    if (ordinal >= 0) {
      number = 0;
      while (number < ordinal && list_element != NULL) {
        number++;
        list_element = list_element->next;
      } /* while */
      if (list_element != NULL) {
        isit_enum(list_element->obj);
        result = list_element->obj;
      } else {
        result = raise_exception(SYS_RNG_EXCEPTION);
      } /* if */
    } else {
      result = raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    return result;
  } /* enu_iconv2 */



objectType enu_ne (listType arguments)

  {
    objectType obj_arg1;
    objectType obj_arg2;
    objectType result;

  /* enu_ne */
    obj_arg1 = take_enum(arg_1(arguments));
    obj_arg2 = take_enum(arg_3(arguments));
    if (obj_arg1 != obj_arg2) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* enu_ne */



objectType enu_ord2 (listType arguments)

  {
    objectType enum_value;
    intType number;
    listType list_element;
    objectType result;

  /* enu_ord2 */
    isit_enum(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    enum_value = take_enum(arg_1(arguments));
    list_element = take_reflist(arg_2(arguments));
    number = 0;
    while (list_element != NULL && list_element->obj != enum_value) {
      number++;
      list_element = list_element->next;
    } /* while */
    if (list_element != NULL) {
      result = bld_int_temp(number);
    } else {
      result = raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    return result;
  } /* enu_ord2 */



objectType enu_value (listType arguments)

  {
    objectType obj_arg;

  /* enu_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    isit_enum(obj_arg);
    return take_enum(obj_arg);
  } /* enu_value */
