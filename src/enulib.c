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

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "striutl.h"
#include "traceutl.h"
#include "objutl.h"
#include "runerr.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "enulib.h"



objectType enu_conv (listType arguments)

  { /* enu_conv */
    isit_enum(arg_3(arguments));
    return take_enum(arg_3(arguments));
  } /* enu_conv */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType enu_cpy (listType arguments)

  {
    objectType dest;

  /* enu_cpy */
    dest = arg_1(arguments);
    isit_enum(dest);
    is_variable(dest);
    isit_enum(arg_3(arguments));
    dest->value.objValue = take_enum(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* enu_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType enu_create (listType arguments)

  {
    objectType dest;
    objectType source;

  /* enu_create */
    dest = arg_1(arguments);
    source = take_enum(arg_3(arguments));
    if (VAR_OBJECT(dest)) {
      SET_CATEGORY_OF_OBJ(dest, VARENUMOBJECT);
    } else {
      SET_CATEGORY_OF_OBJ(dest, CONSTENUMOBJECT);
    } /* if */
    dest->value.objValue = source;
    return SYS_EMPTY_OBJECT;
  } /* enu_create */



objectType enu_eq (listType arguments)

  {
    objectType obj_arg1;
    objectType obj_arg2;

  /* enu_eq */
    obj_arg1 = take_enum(arg_1(arguments));
    obj_arg2 = take_enum(arg_3(arguments));
    if (obj_arg1 == obj_arg2) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* enu_eq */



objectType enu_genlit (listType arguments)

  {
    objectType enum_to;

  /* enu_genlit */
    enum_to = arg_1(arguments);
    logFunction(printf("enu_genlit(");
                trace1(enum_to);
                if (HAS_ENTITY(enum_to) &&
                    GET_ENTITY(enum_to)->syobject != NULL) {
                  printf(" (");
                  trace1(GET_ENTITY(enum_to)->syobject);
                  printf(")");
                } /* if */
                printf(")\n"););
    SET_CATEGORY_OF_OBJ(enum_to, ENUMLITERALOBJECT);
    enum_to->value.objValue = NULL;
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
        logError(printf("enu_iconv2(" FMT_D ", ...): "
                        "Ordinal number too big.", ordinal););
        result = raise_exception(SYS_RNG_EXCEPTION);
      } /* if */
    } else {
      logError(printf("enu_iconv2(" FMT_D ", ...): "
                      "Ordinal number less than zero.", ordinal););
      result = raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    return result;
  } /* enu_iconv2 */



objectType enu_lit (listType arguments)

  {
    objectType enum_value;
    const_cstriType name8;
    striType name;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType literalName;

  /* enu_lit */
    logFunction(printf("enu_lit(");
                trace1(arg_1(arguments));
                printf(")\n"););
    isit_enum(arg_1(arguments));
    enum_value = take_enum(arg_1(arguments));
    if (HAS_ENTITY(enum_value) &&
        GET_ENTITY(enum_value)->ident != NULL) {
      name8 = (const_cstriType) GET_ENTITY(enum_value)->ident->name;
      if (name8 == NULL) {
        literalName = raise_exception(SYS_RNG_EXCEPTION);
      } else {
        name = cstri8_to_stri(name8, &err_info);
        if (unlikely(name == NULL)) {
          raise_error(err_info);
          literalName = NULL;
        } else {
          literalName = bld_stri_temp(name);
        } /* if */
      } /* if */
    } else {
      literalName = raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    return literalName;
  } /* enu_lit */



objectType enu_ne (listType arguments)

  {
    objectType obj_arg1;
    objectType obj_arg2;

  /* enu_ne */
    obj_arg1 = take_enum(arg_1(arguments));
    obj_arg2 = take_enum(arg_3(arguments));
    if (obj_arg1 != obj_arg2) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
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
      logError(printf("enu_ord2: Enum value not found in list of values."););
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
