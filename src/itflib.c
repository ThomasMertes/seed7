/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2014  Thomas Mertes                        */
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
/*  Changes: 1993, 1994, 2002, 2008, 2013, 2014  Thomas Mertes      */
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



/**
 *  Compare two interface pointers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objectType itf_cmp (listType arguments)

  {
    memSizeType interface1;
    memSizeType interface2;
    intType signumValue;

  /* itf_cmp */
    isit_interface(arg_1(arguments));
    isit_interface(arg_2(arguments));
    interface1 = (memSizeType) take_interface(arg_1(arguments));
    interface2 = (memSizeType) take_interface(arg_2(arguments));
    if (interface1 < interface2) {
      signumValue = -1;
    } else if (interface1 > interface2) {
      signumValue = 1;
    } else {
      signumValue = 0;
    } /* if */
    return bld_int_temp(signumValue);
  } /* itf_cmp */



objectType itf_conv2 (listType arguments)

  {
    objectType result;

  /* itf_conv2 */
    result = arg_3(arguments);
 /* printf("itf_conv2: ");
    trace1(result);
    printf("\n"); */
    return bld_interface_temp(result);
  } /* itf_conv2 */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType itf_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    objectType old_value;
    objectType new_value;
    structType old_struct;

  /* itf_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_interface(dest);
    /* isit_interface(source); allow FORWARDOBJECT */
#ifdef TRACE_ITFLIB
    printf("itf_cpy old value: ");
    trace1(dest);
    printf("\n");
    printf("itf_cpy new value: ");
    trace1(source);
    printf("\n");
#endif
    if (CATEGORY_OF_OBJ(dest) == STRUCTOBJECT) {
      old_struct = take_struct(dest);
      old_value = NULL;
      /* printf("before SET_CATEGORY: ");
      trace1(dest);
      printf("\n"); */
      SET_CATEGORY_OF_OBJ(dest, INTERFACEOBJECT);
      /* dest->value.objValue = NULL;
      printf("after SET_CATEGORY: ");
      trace1(dest);
      printf("\n"); */
    } else {
      old_struct = NULL;
      old_value = take_interface(dest);
    } /* if */
    new_value = take_interface(source);
    if (CATEGORY_OF_OBJ(new_value) == STRUCTOBJECT) {
      if ((TEMP_OBJECT(source) || TEMP2_OBJECT(source)) &&
          CATEGORY_OF_OBJ(source) == STRUCTOBJECT) {
        if (!ALLOC_OBJECT(new_value)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          /* printf("itf_cpy: memcpy %lu %lu %lu ",
              take_struct(source), new_value, source);
          trace1(source);
          printf("\n"); */
          memcpy(new_value, source, sizeof(objectRecord));
        } /* if */
      } /* if */
      if (new_value->value.structValue->usage_count != 0) {
        new_value->value.structValue->usage_count++;
      } /* if */
    } else if (CATEGORY_OF_OBJ(new_value) != DECLAREDOBJECT &&
               CATEGORY_OF_OBJ(new_value) != FORWARDOBJECT) {
      run_exception(INTERFACEOBJECT, source);
    } /* if */
    dest->value.objValue = new_value;
    CLEAR_TEMP_FLAG(new_value);
    CLEAR_TEMP2_FLAG(new_value);
    if (old_value == NULL || CATEGORY_OF_OBJ(old_value) == STRUCTOBJECT) {
      if (old_struct == NULL) {
        old_struct = take_struct(old_value);
      } /* if */
      /* printf("itf_cpy: destroy usage_count=%lu %lu\n",
          old_struct->usage_count, (unsigned long) old_struct); */
      if (old_struct->usage_count != 0) {
        old_struct->usage_count--;
        if (old_struct->usage_count == 0) {
          destr_struct(old_struct->stru, old_struct->size);
          /* printf("FREE_STRUCT 12 %lu\n", old_struct); */
          FREE_STRUCT(old_struct, old_struct->size);
          if (old_value != NULL) {
            FREE_OBJECT(old_value);
          } /* if */
        } /* if */
      } /* if */
    } else if (CATEGORY_OF_OBJ(old_value) != DECLAREDOBJECT &&
               CATEGORY_OF_OBJ(old_value) != FORWARDOBJECT) {
      run_exception(INTERFACEOBJECT, old_value);
    } /* if */
#ifdef TRACE_ITFLIB
    printf("itf_cpy afterwards: ");
    trace1(dest);
    printf("\n");
#endif
    return SYS_EMPTY_OBJECT;
  } /* itf_cpy */



objectType itf_cpy2 (listType arguments)

  {
    objectType dest;
    objectType source;
    objectType old_value;
    objectType new_value;
    structType old_struct;

  /* itf_cpy2 */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_interface(dest);
    /* isit_struct(source); allow FORWARDOBJECT */
    if (CATEGORY_OF_OBJ(dest) == STRUCTOBJECT) {
      old_struct = take_struct(dest);
      old_value = NULL;
      /* printf("before SET_CATEGORY: ");
      trace1(dest);
      printf("\n"); */
      SET_CATEGORY_OF_OBJ(dest, INTERFACEOBJECT);
      /* dest->value.objValue = NULL;
      printf("after SET_CATEGORY: ");
      trace1(dest);
      printf("\n"); */
    } else {
      old_struct = NULL;
      old_value = take_interface(dest);
    } /* if */
    new_value = source;
    if (CATEGORY_OF_OBJ(source) == STRUCTOBJECT) {
      if (TEMP_OBJECT(source) || TEMP2_OBJECT(source)) {
        if (!ALLOC_OBJECT(new_value)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          /* printf("itf_cpy2: memcpy %lu %lu %lu ",
              take_struct(source), new_value, source);
          trace1(source);
          printf("\n"); */
          memcpy(new_value, source, sizeof(objectRecord));
        } /* if */
      } /* if */
      if (new_value->value.structValue->usage_count != 0) {
        new_value->value.structValue->usage_count++;
      } /* if */
    } else if (CATEGORY_OF_OBJ(source) != DECLAREDOBJECT &&
               CATEGORY_OF_OBJ(source) != FORWARDOBJECT) {
      run_exception(STRUCTOBJECT, source);
    } /* if */
    dest->value.objValue = new_value;
    CLEAR_TEMP_FLAG(new_value);
    CLEAR_TEMP2_FLAG(new_value);
    if (old_value == NULL || CATEGORY_OF_OBJ(old_value) == STRUCTOBJECT) {
      if (old_struct == NULL) {
        old_struct = take_struct(old_value);
      } /* if */
      /* printf("itf_cpy2: destroy usage_count=%lu %lu\n",
          old_struct->usage_count, (unsigned long) old_struct); */
      if (old_struct->usage_count != 0) {
        old_struct->usage_count--;
        if (old_struct->usage_count == 0) {
          destr_struct(old_struct->stru, old_struct->size);
          /* printf("FREE_STRUCT 13 %lu\n", old_struct); */
          FREE_STRUCT(old_struct, old_struct->size);
          if (old_value != NULL) {
            FREE_OBJECT(old_value);
          } /* if */
        } /* if */
      } /* if */
    } else if (CATEGORY_OF_OBJ(old_value) != DECLAREDOBJECT &&
               CATEGORY_OF_OBJ(old_value) != FORWARDOBJECT) {
      run_exception(INTERFACEOBJECT, old_value);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* itf_cpy2 */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType itf_create (listType arguments)

  {
    objectType dest;
    objectType source;
    objectType new_value;

  /* itf_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    /* isit_interface(source); allow FORWARDOBJECT */
    SET_CATEGORY_OF_OBJ(dest, INTERFACEOBJECT);
#ifdef TRACE_ITFLIB
    printf("itf_create from: ");
    trace1(source);
    printf("\n");
#endif
    new_value = take_interface(source);
    if (CATEGORY_OF_OBJ(new_value) == STRUCTOBJECT) {
      if ((TEMP_OBJECT(source) || TEMP2_OBJECT(source)) &&
          CATEGORY_OF_OBJ(source) == STRUCTOBJECT) {
        if (!ALLOC_OBJECT(new_value)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          /* printf("itf_create: memcpy %lu %lu %lu ",
              take_struct(source), new_value, source);
          trace1(source);
          printf("\n"); */
          memcpy(new_value, source, sizeof(objectRecord));
        } /* if */
      } /* if */
      if (new_value->value.structValue->usage_count != 0) {
        new_value->value.structValue->usage_count++;
      } /* if */
    } else if (CATEGORY_OF_OBJ(new_value) != DECLAREDOBJECT &&
               CATEGORY_OF_OBJ(new_value) != FORWARDOBJECT) {
      run_exception(INTERFACEOBJECT, source);
    } /* if */
    dest->value.objValue = new_value;
    CLEAR_TEMP_FLAG(new_value);
    CLEAR_TEMP2_FLAG(new_value);
#ifdef TRACE_ITFLIB
    printf("itf_create to: ");
    trace1(dest);
    printf("\n");
#endif
    return SYS_EMPTY_OBJECT;
  } /* itf_create */



objectType itf_create2 (listType arguments)

  {
    objectType dest;
    objectType source;
    objectType new_value;

  /* itf_create2 */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    /* isit_interface(source); allow FORWARDOBJECT */
    SET_CATEGORY_OF_OBJ(dest, INTERFACEOBJECT);
#ifdef TRACE_ITFLIB
    printf("itf_create2 from: ");
    trace1(source);
    printf("\n");
#endif
    new_value = source;
    if (CATEGORY_OF_OBJ(new_value) == STRUCTOBJECT) {
      if (TEMP_OBJECT(source) || TEMP2_OBJECT(source)) {
        if (!ALLOC_OBJECT(new_value)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          /* printf("itf_create2: memcpy %lu %lu %lu ",
              take_struct(source), new_value, source);
          trace1(source);
          printf("\n"); */
          memcpy(new_value, source, sizeof(objectRecord));
        } /* if */
      } /* if */
      if (new_value->value.structValue->usage_count != 0) {
        new_value->value.structValue->usage_count++;
      } /* if */
    } else if (CATEGORY_OF_OBJ(new_value) != DECLAREDOBJECT &&
               CATEGORY_OF_OBJ(new_value) != FORWARDOBJECT) {
      run_exception(STRUCTOBJECT, source);
    } /* if */
    dest->value.objValue = new_value;
    CLEAR_TEMP_FLAG(new_value);
    CLEAR_TEMP2_FLAG(new_value);
#ifdef TRACE_ITFLIB
    printf("itf_create2 to: ");
    trace1(dest);
    printf("\n");
#endif
    return SYS_EMPTY_OBJECT;
  } /* itf_create2 */



/**
 *  Free the memory referred by 'old_value/arg_1'.
 *  After itf_destr is left 'old_value/arg_1' is NULL.
 *  The memory where 'old_value/arg_1' is stored can be
 *  freed afterwards.
 */
objectType itf_destr (listType arguments)

  {
    objectType old_value;
    structType old_struct;

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
        /* printf("itf_destr: usage_count=%lu %lu\n",
            old_struct->usage_count, (unsigned long) old_struct);
        trace1(old_value);
        printf("\n"); */
        if (old_struct->usage_count != 0) {
          old_struct->usage_count--;
          if (old_struct->usage_count == 0) {
            destr_struct(old_struct->stru, old_struct->size);
            /* printf("FREE_STRUCT 14 %lu\n", old_struct); */
            FREE_STRUCT(old_struct, old_struct->size);
            arg_1(arguments)->value.objValue = NULL;
            /* The function close_stack leaves HAS_PROPERTY intact to    */
            /* allow checking for it here. Just objects without property */
            /* are removed here. Objects with property will be removed   */
            /* by close_stack or by other functions.                     */
            if (HAS_PROPERTY(old_value)) {
              old_value->value.structValue = NULL;
              /* printf("itf_destr: Struct object with property ");
              trace1(old_value);
              printf("\n"); */
            } else {
              FREE_OBJECT(old_value);
            } /* if */
          } /* if */
        } /* if */
      } /* if */
      arg_1(arguments)->value.objValue = NULL;
    } /* if */
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* itf_destr */



/**
 *  Check if two interfaces are equal.
 *  @return TRUE if both interfaces are equal,
 *          FALSE otherwise.
 */
objectType itf_eq (listType arguments)

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



/**
 *  Compute the hash value of an interface.
 *  @return the hash value.
 */
objectType itf_hashcode (listType arguments)

  { /* itf_hashcode */
    isit_interface(arg_1(arguments));
    return bld_int_temp((intType)
        (((memSizeType) take_interface(arg_1(arguments))) >> 6));
  } /* itf_hashcode */



/**
 *  Check if two interfaces are not equal.
 *  @return FALSE if both interfaces are equal,
 *          TRUE otherwise.
 */
objectType itf_ne (listType arguments)

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



objectType itf_new (listType arguments)

  {
    objectType stru_arg;
    structType stru1;
    structType result_struct;
    objectType result;

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



objectType itf_select (listType arguments)

  {
    structType stru1;
    objectType selector;
    objectType selector_syobject;
    memSizeType position;
    objectType struct_pointer;

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



objectType itf_to_interface (listType arguments)

  {
    objectType stru_arg;
    objectType new_value;
    objectType result;

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
      memcpy(new_value, stru_arg, sizeof(objectRecord));
      CLEAR_TEMP_FLAG(new_value);
      CLEAR_TEMP2_FLAG(new_value);
      if (new_value->value.structValue->usage_count != 0) {
        new_value->value.structValue->usage_count++;
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
