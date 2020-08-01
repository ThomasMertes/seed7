/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2008  Thomas Mertes                        */
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
/*  File: seed7/src/itflib.c                                        */
/*  Changes: 1993, 1994, 2002, 2008  Thomas Mertes                  */
/*  Content: All primitive actions for interface types.             */
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
#include "objutl.h"
#include "runerr.h"
#include "name.h"
#include "match.h"

#undef EXTERN
#define EXTERN
#include "itflib.h"

#undef TRACE_ITFLIB



#ifdef ANSI_C

objecttype itf_cmp (listtype arguments)
#else

objecttype itf_cmp (arguments)
listtype arguments;
#endif

  {
    memsizetype interface1;
    memsizetype interface2;
    inttype result;

  /* itf_cmp */
    isit_interface(arg_1(arguments));
    isit_interface(arg_2(arguments));
    interface1 = (memsizetype) take_interface(arg_1(arguments));
    interface2 = (memsizetype) take_interface(arg_2(arguments));
    if (interface1 < interface2) {
      result = -1;
    } else if (interface1 > interface2) {
      result = 1;
    } else {
      result = 0;
    } /* if */
    return bld_int_temp(result);
  } /* itf_cmp */



#ifdef ANSI_C

objecttype itf_conv2 (listtype arguments)
#else

objecttype itf_conv2 (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* itf_conv2 */
    result = arg_3(arguments);
 /* printf("itf_conv2: ");
    trace1(result);
    printf("\n"); */
    return bld_interface_temp(result);
  } /* itf_conv2 */



#ifdef ANSI_C

objecttype itf_cpy (listtype arguments)
#else

objecttype itf_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype interface_to;
    objecttype interface_from;
    objecttype old_value;
    objecttype new_value;
    errinfotype err_info = OKAY_NO_ERROR;

  /* itf_cpy */
    interface_to = arg_1(arguments);
    interface_from = arg_3(arguments);
    isit_interface(interface_to);
    isit_interface(interface_from);
#ifdef TRACE_ITFLIB
    printf("itf_cpy old value: ");
    trace1(interface_to);
    printf("\n");
    printf("itf_cpy new value: ");
    trace1(interface_from);
    printf("\n");
#endif
    old_value = take_interface(interface_to);
    isit_struct(old_value);
    new_value = take_interface(interface_from);
    isit_struct(new_value);
    if (CATEGORY_OF_OBJ(interface_to) == STRUCTOBJECT) {
      /* printf("before SET_CATEGORY: ");
      trace1(interface_to);
      printf("\n"); */
      SET_CATEGORY_OF_OBJ(interface_to, INTERFACEOBJECT);
      /* interface_to->value.objvalue = NULL;
      printf("after SET_CATEGORY: ");
      trace1(interface_to);
      printf("\n"); */
    } /* if */
    if (TEMP_OBJECT(interface_from)) {
      if (CATEGORY_OF_OBJ(interface_from) == STRUCTOBJECT) {
        if (!ALLOC_OBJECT(new_value)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          memcpy(new_value, interface_from, sizeof(objectrecord));
          new_value->value.structvalue->usage_count++;
          interface_from->value.structvalue = NULL;
        } /* if */
      } else {
        interface_from->value.objvalue = NULL;
      } /* if */
    } else {
      if (new_value->value.structvalue->usage_count != 0) {
        new_value->value.structvalue->usage_count++;
      } /* if */
    } /* if */
    interface_to->value.objvalue = new_value;
    CLEAR_TEMP_FLAG(new_value);
    CLEAR_TEMP2_FLAG(new_value);
    /* printf("usage_count=%d\n", old_value->value.structvalue->usage_count); */
    if (old_value->value.structvalue->usage_count != 0) {
      old_value->value.structvalue->usage_count--;
      if (old_value->value.structvalue->usage_count == 0) {
        /* printf("really destroy ");
        trace1(old_value);
        printf("\n"); */
        CLEAR_TEMP_FLAG(old_value);
        do_destroy(old_value, &err_info);
        FREE_OBJECT(old_value);
      } /* if */
    } /* if */
#ifdef TRACE_ITFLIB
    printf("itf_cpy afterwards: ");
    trace1(interface_to);
    printf("\n");
#endif
    return SYS_EMPTY_OBJECT;
  } /* itf_cpy */



#ifdef ANSI_C

objecttype itf_cpy2 (listtype arguments)
#else

objecttype itf_cpy2 (arguments)
listtype arguments;
#endif

  {
    objecttype interface_to;
    objecttype interface_from;
    objecttype new_value;

  /* itf_cpy2 */
    interface_to = arg_1(arguments);
    interface_from = arg_3(arguments);
    isit_interface(interface_to);
    isit_struct(interface_from);
    new_value = interface_from;
    if (CATEGORY_OF_OBJ(interface_to) == STRUCTOBJECT) {
      /* printf("before SET_CATEGORY: ");
      trace1(interface_to);
      printf("\n"); */
      SET_CATEGORY_OF_OBJ(interface_to, INTERFACEOBJECT);
      /* interface_to->value.objvalue = NULL;
      printf("after SET_CATEGORY: ");
      trace1(interface_to);
      printf("\n"); */
    } /* if */
    if (TEMP_OBJECT(interface_from)) {
      if (!ALLOC_OBJECT(new_value)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        memcpy(new_value, interface_from, sizeof(objectrecord));
        new_value->value.structvalue->usage_count++;
        interface_from->value.structvalue = NULL;
      } /* if */
    } else {
      if (new_value->value.structvalue->usage_count != 0) {
        new_value->value.structvalue->usage_count++;
      } /* if */
    } /* if */
    interface_to->value.objvalue = new_value;
    CLEAR_TEMP_FLAG(new_value);
    CLEAR_TEMP2_FLAG(new_value);
    return SYS_EMPTY_OBJECT;
  } /* itf_cpy2 */



#ifdef ANSI_C

objecttype itf_create (listtype arguments)
#else

objecttype itf_create (arguments)
listtype arguments;
#endif

  {
    objecttype interface_to;
    objecttype interface_from;
    objecttype new_value;

  /* itf_create */
    interface_to = arg_1(arguments);
    interface_from = arg_3(arguments);
    SET_CATEGORY_OF_OBJ(interface_to, INTERFACEOBJECT);
    isit_interface(interface_from);
#ifdef TRACE_ITFLIB
    printf("itf_create from: ");
    trace1(interface_from);
    printf("\n");
#endif
    new_value = take_interface(interface_from);
    isit_struct(new_value);
    interface_to->value.objvalue = new_value;
    if (TEMP_OBJECT(interface_from)) {
      interface_from->value.objvalue = NULL;
    } else {
      if (new_value->value.structvalue->usage_count != 0) {
        new_value->value.structvalue->usage_count++;
      } /* if */
    } /* if */
#ifdef TRACE_ITFLIB
    printf("itf_create to: ");
    trace1(interface_to);
    printf("\n");
#endif
    return SYS_EMPTY_OBJECT;
  } /* itf_create */



#ifdef ANSI_C

objecttype itf_create2 (listtype arguments)
#else

objecttype itf_create2 (arguments)
listtype arguments;
#endif

  {
    objecttype interface_to;
    objecttype interface_from;

  /* itf_create2 */
    interface_to = arg_1(arguments);
    interface_from = arg_3(arguments);
    SET_CATEGORY_OF_OBJ(interface_to, INTERFACEOBJECT);
    if (CATEGORY_OF_OBJ(interface_from) == INTERFACEOBJECT) {
      interface_to->value.objvalue = take_reference(interface_from);
    } else {
      interface_to->value.objvalue = interface_from;
      CLEAR_TEMP_FLAG(interface_from);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* itf_create2 */



#ifdef ANSI_C

objecttype itf_destr (listtype arguments)
#else

objecttype itf_destr (arguments)
listtype arguments;
#endif

  {
    objecttype old_value;
    structtype struct_value;
    errinfotype err_info = OKAY_NO_ERROR;

  /* itf_destr */
#ifdef TRACE_ITFLIB
    printf("itf_destr(");
    trace1(arg_1(arguments));
    printf("\n");
#endif
    just_interface(arg_1(arguments));
    old_value = take_interface(arg_1(arguments));
    if (old_value != NULL) {
      isit_struct(old_value);
      struct_value = take_struct(old_value);
      if (struct_value != NULL) {
        /* printf("usage_count=%d\n", struct_value->usage_count); */
        if (struct_value->usage_count != 0) {
          struct_value->usage_count--;
          if (struct_value->usage_count == 0) {
            /* printf("really destroy ");
            trace1(old_value);
            printf("\n"); */
            CLEAR_TEMP_FLAG(old_value);
            do_destroy(old_value, &err_info);
            FREE_OBJECT(old_value);
          } /* if */
        } /* if */
      } /* if */
      arg_1(arguments)->value.objvalue = NULL;
      /* if (CATEGORY_OF_OBJ(arg) == INTERFACEOBJECT) {
        old_value->value.objvalue = NULL;
      } ** if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* itf_destr */



#ifdef ANSI_C

objecttype itf_eq (listtype arguments)
#else

objecttype itf_eq (arguments)
listtype arguments;
#endif

  { /* itf_eq */
    isit_interface(arg_1(arguments));
    isit_interface(arg_3(arguments));
    if (take_interface(arg_1(arguments)) ==
        take_interface(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* itf_eq */



#ifdef ANSI_C

objecttype itf_hashcode (listtype arguments)
#else

objecttype itf_hashcode (arguments)
listtype arguments;
#endif

  { /* itf_hashcode */
    isit_interface(arg_1(arguments));
    return bld_int_temp((inttype)
        (((memsizetype) take_interface(arg_1(arguments))) >> 6));
  } /* itf_hashcode */



#ifdef ANSI_C

objecttype itf_ne (listtype arguments)
#else

objecttype itf_ne (arguments)
listtype arguments;
#endif

  { /* itf_ne */
    isit_interface(arg_1(arguments));
    isit_interface(arg_3(arguments));
    if (take_interface(arg_1(arguments)) !=
        take_interface(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* itf_ne */



#ifdef ANSI_C

objecttype itf_new (listtype arguments)
#else

objecttype itf_new (arguments)
listtype arguments;
#endif

  {
    objecttype stru_arg;
    structtype stru1;
    structtype result_struct;
    objecttype result;

  /* itf_new */
    stru_arg = arg_1(arguments);
    isit_struct(stru_arg);
    if (TEMP_OBJECT(stru_arg)) {
      result = stru_arg;
      result->type_of = NULL;
      arg_1(arguments) = NULL;
    } else {
      stru1 = take_struct(stru_arg);
      if (!ALLOC_STRUCT(result_struct, stru1->size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result_struct->size = stru1->size;
      if (!crea_array(result_struct->stru, stru1->stru, stru1->size)) {
        FREE_STRUCT(result_struct, stru1->size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
      result = bld_struct_temp(result_struct);
    } /* if */
    return result;
  } /* itf_new */



#ifdef ANSI_C

objecttype itf_select (listtype arguments)
#else

objecttype itf_select (arguments)
listtype arguments;
#endif

  {
    structtype stru1;
    objecttype selector;
    objecttype selector_syobject;
    memsizetype position;
    objecttype struct_pointer;

  /* itf_select */
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
    if (HAS_ENTITY(selector) &&
        GET_ENTITY(selector)->syobject != NULL) {
      selector_syobject = GET_ENTITY(selector)->syobject;
      position = stru1->size;
      struct_pointer = stru1->stru;
      while (position > 0) {
/*
printf("test ");
trace1(struct_pointer);
printf("\n");
*/
        if (HAS_ENTITY(struct_pointer) &&
            GET_ENTITY(struct_pointer)->syobject == selector_syobject) {
          if (TEMP_OBJECT(struct_pointer)) {
            printf("sct_select of TEMP_OBJECT\n");
          } /* if */
          return struct_pointer;
        } /* if */
        position--;
        struct_pointer++;
      } /* while */
    } /* if */
    return raise_exception(SYS_RNG_EXCEPTION);
  } /* itf_select */



#ifdef ANSI_C

objecttype itf_to_heap (listtype arguments)
#else

objecttype itf_to_heap (arguments)
listtype arguments;
#endif

  {
    objecttype interface_from;
    objecttype result;

  /* itf_to_heap */
    interface_from = arg_1(arguments);
#ifdef TRACE_ITFLIB
    printf("itf_to_heap: ");
       trace1(interface_from);
       printf("\n");
#endif
    if (CATEGORY_OF_OBJ(interface_from) == INTERFACEOBJECT) {
      result = take_reference(interface_from);
      isit_struct(result);
      if (take_struct(result) != NULL && take_struct(result)->usage_count != 0) {
        take_struct(result)->usage_count++;
      } /* if */
    } else if (CATEGORY_OF_OBJ(interface_from) == STRUCTOBJECT) {
      if (TEMP2_OBJECT(interface_from)) {
        if (!ALLOC_OBJECT(result)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          memcpy(result, interface_from, sizeof(objectrecord));
          CLEAR_TEMP2_FLAG(result);
          result->value.structvalue = take_struct(interface_from);
          interface_from->value.structvalue = NULL;
        } /* if */
      } else {
        result = interface_from;
      } /* if */
    } else {
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    result = bld_interface_temp(result);
#ifdef TRACE_ITFLIB
    printf("itf_to_heap --> ");
       trace1(result);
       printf("\n");
#endif
    return result;
  } /* itf_to_heap */



#ifdef ANSI_C

objecttype itf_to_interface (listtype arguments)
#else

objecttype itf_to_interface (arguments)
listtype arguments;
#endif

  {
    objecttype stru_arg;
    structtype new_stru;
    objecttype result;

  /* itf_to_interface */
    stru_arg = arg_1(arguments);
#ifdef TRACE_ITFLIB
    printf("itf_to_interface: ");
       trace1(stru_arg);
       printf("\n");
#endif
    isit_struct(stru_arg);
    if (!ALLOC_OBJECT(result)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      if (!ALLOC_STRUCT(new_stru, 0)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        memcpy(result, stru_arg, sizeof(objectrecord));
        CLEAR_TEMP_FLAG(result);
        CLEAR_TEMP2_FLAG(result);
        result->value.structvalue->usage_count++;
        new_stru->size = 0;
        stru_arg->value.structvalue = new_stru;
      } /* if */
    } /* if */
    result = bld_interface_temp(result);
#ifdef TRACE_ITFLIB
    printf("itf_to_interface --> ");
       trace1(result);
       printf("\n");
#endif
    return result;
  } /* itf_to_interface */
