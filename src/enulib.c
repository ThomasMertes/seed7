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
#include "runerr.h"
#include "memory.h"

#undef EXTERN
#define EXTERN
#include "enulib.h"



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
    return(SYS_EMPTY_OBJECT);
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
      SET_CLASS_OF_OBJ(enum_to, VARENUMOBJECT);
    } else {
      SET_CLASS_OF_OBJ(enum_to, CONSTENUMOBJECT);
    } /* if */
    enum_to->value.objvalue = enum_from;
    return(SYS_EMPTY_OBJECT);
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
    return(result);
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
    SET_CLASS_OF_OBJ(enum_to, ENUMLITERALOBJECT);
    enum_to->value.nodevalue = (nodetype) NULL;
    return(SYS_EMPTY_OBJECT);
  } /* enu_genlit */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

objecttype enu_iconv (listtype arguments)
#else

objecttype enu_iconv (arguments)
listtype arguments;
#endif

  {
    chartype result;

  /* enu_iconv */
    isit_int(arg_3(arguments));
    result = take_int(arg_3(arguments));
    return(bld_char_temp(result));
  } /* enu_iconv */
#endif



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
    return(result);
  } /* enu_ne */



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
    return(bld_int_temp(size));
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
    isit_reference(arg_3(arguments));
    obj_arg = take_reference(arg_3(arguments));
    isit_enum(obj_arg);
    return(take_enum(obj_arg));
  } /* enu_value */
