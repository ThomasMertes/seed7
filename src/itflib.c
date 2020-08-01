/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
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



objecttype itf_cmp (listtype arguments)

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



objecttype itf_conv2 (listtype arguments)

  {
    objecttype result;

  /* itf_conv2 */
    result = arg_3(arguments);
 /* printf("itf_conv2: ");
    trace1(result);
    printf("\n"); */
    return bld_interface_temp(result);
  } /* itf_conv2 */



objecttype itf_cpy (listtype arguments)

  {
    objecttype interface_to;
    objecttype interface_from;
    objecttype old_value;
    objecttype new_value;
    structtype old_struct;

  /* itf_cpy */
    interface_to = arg_1(arguments);
    interface_from = arg_3(arguments);
    isit_interface(interface_to);
    /* isit_interface(interface_from); allow FORWARDOBJECT */
#ifdef TRACE_ITFLIB
    printf("itf_cpy old value: ");
    trace1(interface_to);
    printf("\n");
    printf("itf_cpy new value: ");
    trace1(interface_from);
    printf("\n");
#endif
    old_value = take_interface(interface_to);
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
    new_value = take_interface(interface_from);
    if (CATEGORY_OF_OBJ(new_value) == STRUCTOBJECT) {
      if ((TEMP_OBJECT(interface_from) || TEMP2_OBJECT(interface_from)) &&
          CATEGORY_OF_OBJ(interface_from) == STRUCTOBJECT) {
        if (!ALLOC_OBJECT(new_value)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          /* printf("itf_cpy: memcpy %lu %lu %lu ", take_struct(interface_from), new_value, interface_from);
          trace1(interface_from);
          printf("\n"); */
          memcpy(new_value, interface_from, sizeof(objectrecord));
        } /* if */
      } /* if */
      if (new_value->value.structvalue->usage_count != 0) {
        new_value->value.structvalue->usage_count++;
      } /* if */
    } else if (CATEGORY_OF_OBJ(new_value) != DECLAREDOBJECT &&
               CATEGORY_OF_OBJ(new_value) != FORWARDOBJECT) {
      run_exception(INTERFACEOBJECT, interface_from);
    } /* if */
    interface_to->value.objvalue = new_value;
    CLEAR_TEMP_FLAG(new_value);
    CLEAR_TEMP2_FLAG(new_value);
    if (CATEGORY_OF_OBJ(old_value) == STRUCTOBJECT) {
      old_struct = take_struct(old_value);
      /* printf("itf_cpy: destroy usage_count=%lu %lu\n", old_struct->usage_count, (unsigned long) old_struct); */
      if (old_struct->usage_count != 0) {
        old_struct->usage_count--;
        if (old_struct->usage_count == 0) {
          destr_struct(old_struct->stru, old_struct->size);
          /* printf("FREE_STRUCT 12 %lu\n", old_struct); */
          FREE_STRUCT(old_struct, old_struct->size);
          FREE_OBJECT(old_value);
        } /* if */
      } /* if */
    } else if (CATEGORY_OF_OBJ(old_value) != DECLAREDOBJECT &&
               CATEGORY_OF_OBJ(old_value) != FORWARDOBJECT) {
      run_exception(INTERFACEOBJECT, old_value);
    } /* if */
#ifdef TRACE_ITFLIB
    printf("itf_cpy afterwards: ");
    trace1(interface_to);
    printf("\n");
#endif
    return SYS_EMPTY_OBJECT;
  } /* itf_cpy */



objecttype itf_cpy2 (listtype arguments)

  {
    objecttype interface_to;
    objecttype interface_from;
    objecttype old_value;
    objecttype new_value;
    structtype old_struct;

  /* itf_cpy2 */
    interface_to = arg_1(arguments);
    interface_from = arg_3(arguments);
    isit_interface(interface_to);
    /* isit_struct(interface_from); allow FORWARDOBJECT */
    old_value = take_interface(interface_to);
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
    new_value = interface_from;
    if (CATEGORY_OF_OBJ(interface_from) == STRUCTOBJECT) {
      if (TEMP_OBJECT(interface_from) || TEMP2_OBJECT(interface_from)) {
        if (!ALLOC_OBJECT(new_value)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          /* printf("itf_cpy2: memcpy %lu %lu %lu ", take_struct(interface_from), new_value, interface_from);
          trace1(interface_from);
          printf("\n"); */
          memcpy(new_value, interface_from, sizeof(objectrecord));
        } /* if */
      } /* if */
      if (new_value->value.structvalue->usage_count != 0) {
        new_value->value.structvalue->usage_count++;
      } /* if */
    } else if (CATEGORY_OF_OBJ(interface_from) != DECLAREDOBJECT &&
               CATEGORY_OF_OBJ(interface_from) != FORWARDOBJECT) {
      run_exception(STRUCTOBJECT, interface_from);
    } /* if */
    interface_to->value.objvalue = new_value;
    CLEAR_TEMP_FLAG(new_value);
    CLEAR_TEMP2_FLAG(new_value);
    if (CATEGORY_OF_OBJ(old_value) == STRUCTOBJECT) {
      old_struct = take_struct(old_value);
      /* printf("itf_cpy2: destroy usage_count=%lu %lu\n", old_struct->usage_count, (unsigned long) old_struct); */
      if (old_struct->usage_count != 0) {
        old_struct->usage_count--;
        if (old_struct->usage_count == 0) {
          destr_struct(old_struct->stru, old_struct->size);
          /* printf("FREE_STRUCT 13 %lu\n", old_struct); */
          FREE_STRUCT(old_struct, old_struct->size);
          FREE_OBJECT(old_value);
        } /* if */
      } /* if */
    } else if (CATEGORY_OF_OBJ(old_value) != DECLAREDOBJECT &&
               CATEGORY_OF_OBJ(old_value) != FORWARDOBJECT) {
      run_exception(INTERFACEOBJECT, old_value);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* itf_cpy2 */



objecttype itf_create (listtype arguments)

  {
    objecttype interface_to;
    objecttype interface_from;
    objecttype new_value;

  /* itf_create */
    interface_to = arg_1(arguments);
    interface_from = arg_3(arguments);
    /* isit_interface(interface_from); allow FORWARDOBJECT */
    SET_CATEGORY_OF_OBJ(interface_to, INTERFACEOBJECT);
#ifdef TRACE_ITFLIB
    printf("itf_create from: ");
    trace1(interface_from);
    printf("\n");
#endif
    new_value = take_interface(interface_from);
    if (CATEGORY_OF_OBJ(new_value) == STRUCTOBJECT) {
      if ((TEMP_OBJECT(interface_from) || TEMP2_OBJECT(interface_from)) &&
          CATEGORY_OF_OBJ(interface_from) == STRUCTOBJECT) {
        if (!ALLOC_OBJECT(new_value)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          /* printf("itf_create: memcpy %lu %lu %lu ", take_struct(interface_from), new_value, interface_from);
          trace1(interface_from);
          printf("\n"); */
          memcpy(new_value, interface_from, sizeof(objectrecord));
        } /* if */
      } /* if */
      if (new_value->value.structvalue->usage_count != 0) {
        new_value->value.structvalue->usage_count++;
      } /* if */
    } else if (CATEGORY_OF_OBJ(new_value) != DECLAREDOBJECT &&
               CATEGORY_OF_OBJ(new_value) != FORWARDOBJECT) {
      run_exception(INTERFACEOBJECT, interface_from);
    } /* if */
    interface_to->value.objvalue = new_value;
    CLEAR_TEMP_FLAG(new_value);
    CLEAR_TEMP2_FLAG(new_value);
#ifdef TRACE_ITFLIB
    printf("itf_create to: ");
    trace1(interface_to);
    printf("\n");
#endif
    return SYS_EMPTY_OBJECT;
  } /* itf_create */



objecttype itf_create2 (listtype arguments)

  {
    objecttype interface_to;
    objecttype interface_from;
    objecttype new_value;

  /* itf_create2 */
    interface_to = arg_1(arguments);
    interface_from = arg_3(arguments);
    /* isit_interface(interface_from); allow FORWARDOBJECT */
    SET_CATEGORY_OF_OBJ(interface_to, INTERFACEOBJECT);
#ifdef TRACE_ITFLIB
    printf("itf_create2 from: ");
    trace1(interface_from);
    printf("\n");
#endif
    new_value = interface_from;
    if (CATEGORY_OF_OBJ(interface_from) == STRUCTOBJECT) {
      if (TEMP_OBJECT(interface_from) || TEMP2_OBJECT(interface_from)) {
        if (!ALLOC_OBJECT(new_value)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          /* printf("itf_create2: memcpy %lu %lu %lu ", take_struct(interface_from), new_value, interface_from);
          trace1(interface_from);
          printf("\n"); */
          memcpy(new_value, interface_from, sizeof(objectrecord));
        } /* if */
      } /* if */
      if (new_value->value.structvalue->usage_count != 0) {
        new_value->value.structvalue->usage_count++;
      } /* if */
    } else if (CATEGORY_OF_OBJ(interface_from) != DECLAREDOBJECT &&
               CATEGORY_OF_OBJ(interface_from) != FORWARDOBJECT) {
      run_exception(STRUCTOBJECT, interface_from);
    } /* if */
    interface_to->value.objvalue = new_value;
    CLEAR_TEMP_FLAG(new_value);
    CLEAR_TEMP2_FLAG(new_value);
#ifdef TRACE_ITFLIB
    printf("itf_create2 to: ");
    trace1(interface_to);
    printf("\n");
#endif
    return SYS_EMPTY_OBJECT;
  } /* itf_create2 */



objecttype itf_destr (listtype arguments)

  {
    objecttype old_value;
    structtype old_struct;

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
      old_struct = take_struct(old_value);
      if (old_struct != NULL) {
        /* printf("itf_destr: usage_count=%lu %lu\n", old_struct->usage_count, (unsigned long) old_struct);
        trace1(old_value);
        printf("\n"); */
        if (old_struct->usage_count != 0) {
          old_struct->usage_count--;
          if (old_struct->usage_count == 0) {
            destr_struct(old_struct->stru, old_struct->size);
            /* printf("FREE_STRUCT 14 %lu\n", old_struct); */
            FREE_STRUCT(old_struct, old_struct->size);
            arg_1(arguments)->value.objvalue = NULL;
            /* The function close_stack leaves HAS_PROPERTY intact to    */
            /* allow checking for it here. Just objects without property */
            /* are removed here. Objects with property will be removed   */
            /* by close_stack or by other functions.                     */
            if (HAS_PROPERTY(old_value)) {
              old_value->value.structvalue = NULL;
              /* printf("itf_destr: Struct object with property ");
              trace1(old_value);
              printf("\n"); */
            } else {
              FREE_OBJECT(old_value);
            } /* if */
          } /* if */
        } /* if */
      } /* if */
      arg_1(arguments)->value.objvalue = NULL;
    } /* if */
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* itf_destr */



objecttype itf_eq (listtype arguments)

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



objecttype itf_hashcode (listtype arguments)

  { /* itf_hashcode */
    isit_interface(arg_1(arguments));
    return bld_int_temp((inttype)
        (((memsizetype) take_interface(arg_1(arguments))) >> 6));
  } /* itf_hashcode */



objecttype itf_ne (listtype arguments)

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



objecttype itf_new (listtype arguments)

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
      result_struct->usage_count = 1;
      result_struct->size = stru1->size;
      if (!crea_struct(result_struct->stru, stru1->stru, stru1->size)) {
        /* printf("FREE_STRUCT 15 %lu\n", result_struct); */
        FREE_STRUCT(result_struct, stru1->size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
      result = bld_struct_temp(result_struct);
    } /* if */
    return result;
  } /* itf_new */



objecttype itf_select (listtype arguments)

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



objecttype itf_to_interface (listtype arguments)

  {
    objecttype stru_arg;
    objecttype new_value;
    objecttype result;

  /* itf_to_interface */
    stru_arg = arg_1(arguments);
#ifdef TRACE_ITFLIB
    printf("itf_to_interface: ");
       trace1(stru_arg);
       printf("\n");
#endif
    isit_struct(stru_arg);
    if (!ALLOC_OBJECT(new_value)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      memcpy(new_value, stru_arg, sizeof(objectrecord));
      CLEAR_TEMP_FLAG(new_value);
      CLEAR_TEMP2_FLAG(new_value);
      if (new_value->value.structvalue->usage_count != 0) {
        new_value->value.structvalue->usage_count++;
      } /* if */
    } /* if */
    result = bld_interface_temp(new_value);
#ifdef TRACE_ITFLIB
    printf("itf_to_interface --> ");
       trace1(result);
       printf("\n");
#endif
    return result;
  } /* itf_to_interface */
