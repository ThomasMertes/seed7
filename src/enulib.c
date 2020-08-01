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



#ifdef ANSI_C

objecttype enu_conv (listtype arguments)
#else

objecttype enu_conv (arguments)
listtype arguments;
#endif

  { /* enu_conv */
    isit_enum(arg_3(arguments));
    return take_enum(arg_3(arguments));
  } /* enu_conv */



#ifdef ANSI_C

objecttype enu_cpy (listtype arguments)
#else

objecttype enu_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype enum_variable;

  /* enu_cpy */
    enum_variable = arg_1(arguments);
    isit_enum(enum_variable);
    is_variable(enum_variable);
    isit_enum(arg_3(arguments));
    enum_variable->value.objvalue = take_enum(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* enu_cpy */



#ifdef ANSI_C

objecttype enu_create (listtype arguments)
#else

objecttype enu_create (arguments)
listtype arguments;
#endif

  {
    objecttype enum_to;
    objecttype enum_from;

  /* enu_create */
    enum_to = arg_1(arguments);
    enum_from = take_enum(arg_3(arguments));
    if (VAR_OBJECT(enum_to)) {
      SET_CATEGORY_OF_OBJ(enum_to, VARENUMOBJECT);
    } else {
      SET_CATEGORY_OF_OBJ(enum_to, CONSTENUMOBJECT);
    } /* if */
    enum_to->value.objvalue = enum_from;
    return SYS_EMPTY_OBJECT;
  } /* enu_create */



#ifdef ANSI_C

objecttype enu_eq (listtype arguments)
#else

objecttype enu_eq (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    objecttype obj_arg2;
    objecttype result;

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



#ifdef ANSI_C

objecttype enu_genlit (listtype arguments)
#else

objecttype enu_genlit (arguments)
listtype arguments;
#endif

  {
    objecttype enum_to;

  /* enu_genlit */
    enum_to = arg_1(arguments);
    SET_CATEGORY_OF_OBJ(enum_to, ENUMLITERALOBJECT);
    enum_to->value.nodevalue = (nodetype) NULL;
    return SYS_EMPTY_OBJECT;
  } /* enu_genlit */



#ifdef ANSI_C

objecttype enu_iconv2 (listtype arguments)
#else

objecttype enu_iconv2 (arguments)
listtype arguments;
#endif

  {
    inttype ordinal;
    inttype number;
    listtype list_element;
    objecttype result;

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



#ifdef ANSI_C

objecttype enu_ne (listtype arguments)
#else

objecttype enu_ne (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    objecttype obj_arg2;
    objecttype result;

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



#ifdef ANSI_C

objecttype enu_ord2 (listtype arguments)
#else

objecttype enu_ord2 (arguments)
listtype arguments;
#endif

  {
    objecttype enum_value;
    inttype number;
    listtype list_element;
    objecttype result;

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



#ifdef ANSI_C

objecttype enu_size (listtype arguments)
#else

objecttype enu_size (arguments)
listtype arguments;
#endif

  {
    objecttype modu;
    inttype size;

  /* enu_size */
    modu = arg_1(arguments);
    size = sizeof(objectrecord);
    return bld_int_temp(size);
  } /* enu_size */



#ifdef ANSI_C

objecttype enu_value (listtype arguments)
#else

objecttype enu_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;

  /* enu_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    isit_enum(obj_arg);
    return take_enum(obj_arg);
  } /* enu_value */
