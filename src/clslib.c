/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2002  Thomas Mertes                        */
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
/*  File: seed7/src/clslib.c                                        */
/*  Changes: 1993, 1994, 2002  Thomas Mertes                        */
/*  Content: All primitive actions for structure types.             */
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
#include "syvarutl.h"
#include "traceutl.h"
#include "listutl.h"
#include "entutl.h"
#include "executl.h"
#include "runerr.h"
#include "memory.h"
#include "name.h"
#include "match.h"

#undef EXTERN
#define EXTERN
#include "clslib.h"



#ifdef ANSI_C

objecttype cls_conv2 (listtype arguments)
#else

objecttype cls_conv2 (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* cls_conv2 */
    result = arg_3(arguments);
 /* printf("cls_conv: ");
    trace1(result);
    printf("\n"); */
    return(bld_class_temp(result));
  } /* cls_conv2 */



#ifdef ANSI_C

objecttype cls_cpy (listtype arguments)
#else

objecttype cls_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype modu_to;
    objecttype modu_from;

  /* cls_cpy */
    modu_to = arg_1(arguments);
    modu_from = arg_3(arguments);
    isit_class(modu_to);
    isit_class(modu_from);
 /* printf("cls_cpy from: ");
    trace1(modu_from);
    printf("\n"); */
    modu_to->value.objvalue = take_reference(modu_from);
 /* printf("cls_cpy to: ");
    trace1(modu_to);
    printf("\n"); */
    return(SYS_EMPTY_OBJECT);
  } /* cls_cpy */



#ifdef ANSI_C

objecttype cls_cpy2 (listtype arguments)
#else

objecttype cls_cpy2 (arguments)
listtype arguments;
#endif

  {
    objecttype modu_to;
    objecttype modu_from;

  /* cls_cpy2 */
    modu_to = arg_1(arguments);
    modu_from = arg_3(arguments);
    if (CATEGORY_OF_OBJ(modu_from) == CLASSOBJECT) {
      modu_to->value.objvalue = take_reference(modu_from);
    } else {
      modu_to->value.objvalue = modu_from;
      CLEAR_TEMP_FLAG(modu_from);
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* cls_cpy2 */



#ifdef ANSI_C

objecttype cls_create (listtype arguments)
#else

objecttype cls_create (arguments)
listtype arguments;
#endif

  {
    objecttype modu_to;
    objecttype modu_from;

  /* cls_create */
    modu_to = arg_1(arguments);
    modu_from = arg_3(arguments);
    SET_CATEGORY_OF_OBJ(modu_to, CLASSOBJECT);
    isit_class(modu_from);
    modu_to->value.objvalue = take_reference(modu_from);
    return(SYS_EMPTY_OBJECT);
  } /* cls_create */



#ifdef ANSI_C

objecttype cls_create2 (listtype arguments)
#else

objecttype cls_create2 (arguments)
listtype arguments;
#endif

  {
    objecttype modu_to;
    objecttype modu_from;

  /* cls_create2 */
    modu_to = arg_1(arguments);
    modu_from = arg_3(arguments);
    SET_CATEGORY_OF_OBJ(modu_to, CLASSOBJECT);
    if (CATEGORY_OF_OBJ(modu_from) == CLASSOBJECT) {
      modu_to->value.objvalue = take_reference(modu_from);
    } else {
      modu_to->value.objvalue = modu_from;
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* cls_create2 */



#ifdef ANSI_C

objecttype cls_eq (listtype arguments)
#else

objecttype cls_eq (arguments)
listtype arguments;
#endif

  { /* cls_eq */
    isit_class(arg_1(arguments));
    isit_class(arg_3(arguments));
    if (take_reference(arg_1(arguments)) ==
        take_reference(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* cls_eq */



#ifdef ANSI_C

objecttype cls_ne (listtype arguments)
#else

objecttype cls_ne (arguments)
listtype arguments;
#endif

  { /* cls_ne */
    isit_class(arg_1(arguments));
    isit_class(arg_3(arguments));
    if (take_reference(arg_1(arguments)) !=
        take_reference(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* cls_ne */



#ifdef ANSI_C

objecttype cls_new (listtype arguments)
#else

objecttype cls_new (arguments)
listtype arguments;
#endif

  {
    objecttype stru_arg;
    structtype stru1;
    structtype result_struct;
    objecttype result;

  /* cls_new */
    stru_arg = arg_1(arguments);
    isit_struct(stru_arg);
    if (TEMP_OBJECT(stru_arg)) {
      result = stru_arg;
      result->type_of = NULL;
      arg_1(arguments) = NULL;
    } else {
      stru1 = take_struct(stru_arg);
      if (!ALLOC_STRUCT(result_struct, stru1->size)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } /* if */
      result_struct->size = stru1->size;
      if (!crea_array(result_struct->stru, stru1->stru, stru1->size)) {
        FREE_STRUCT(result_struct, stru1->size);
        return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
      } /* if */
      result = bld_struct_temp(result_struct);
    } /* if */
    return(result);
  } /* cls_new */



#ifdef ANSI_C

objecttype cls_select (listtype arguments)
#else

objecttype cls_select (arguments)
listtype arguments;
#endif

  {
    structtype stru1;
    objecttype selector;
    objecttype selector_syobject;
    inttype position;
    objecttype struct_pointer;

  /* cls_select */
    isit_struct(arg_1(arguments));
    stru1 = take_struct(arg_1(arguments));
    selector = arg_3(arguments);
/*
printf("stru1 ");
trace1(arg_1(arguments));
printf("\n");
printf("selector ");
trace1(selector);
printf("\n");
*/
    if (HAS_DESCRIPTOR_ENTITY(selector) &&
        selector->descriptor.entity->syobject != NULL) {
      selector_syobject = selector->descriptor.entity->syobject;
      position = stru1->size;
      struct_pointer = stru1->stru;
      while (position > 0) {
/*
printf("test ");
trace1(struct_pointer);
printf("\n");
*/
        if (HAS_DESCRIPTOR_ENTITY(struct_pointer) &&
            struct_pointer->descriptor.entity->syobject == selector_syobject) {
          if (TEMP_OBJECT(struct_pointer)) {
            printf("sct_select of TEMP_OBJECT\n");
          } /* if */
          return(struct_pointer);
        } /* if */
        position--;
        struct_pointer++;
      } /* while */
    } /* if */
    return(raise_exception(SYS_RNG_EXCEPTION));
  } /* cls_select */
