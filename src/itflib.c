/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2008, 2013, 2014, 2016  Thomas Mertes      */
/*                2021, 2024, 2025  Thomas Mertes                   */
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
/*           2016, 2021, 2024, 2025  Thomas Mertes                  */
/*  Content: All primitive actions for interface types.             */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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
    } else {
      signumValue = interface1 > interface2;
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
    logFunction(printf("itf_cpy(");
                trace1(dest);
                printf(", ");
                trace1(source);
                printf(")\n"););
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
          logError(printf("itf_cpy: ALLOC_OBJECT() failed.\n"););
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
    logFunction(printf("itf_cpy(");
                trace1(dest);
                printf(", *) -->\n"););
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
          logError(printf("itf_cpy2: ALLOC_OBJECT() failed.\n"););
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
    logFunction(printf("itf_create(");
                trace1(source);
                printf(")\n"););
    new_value = take_interface(source);
    if (CATEGORY_OF_OBJ(new_value) == STRUCTOBJECT) {
      if ((TEMP_OBJECT(source) || TEMP2_OBJECT(source)) &&
          CATEGORY_OF_OBJ(source) == STRUCTOBJECT) {
        if (!ALLOC_OBJECT(new_value)) {
          logError(printf("itf_create: ALLOC_OBJECT() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          /* printf("itf_create: memcpy %lu %lu %lu ",
              take_struct(source), new_value, source);
          trace1(source);
          printf("\n"); */
          memcpy(new_value, source, sizeof(objectRecord));
        } /* if */
      } else {
        isit_struct_ok(new_value);
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
    logFunction(printf("itf_create --> ");
                trace1(dest);
                printf("\n"););
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
    logFunction(printf("itf_create2(");
                trace1(source);
                printf(")\n"););
    new_value = source;
    if (CATEGORY_OF_OBJ(new_value) == STRUCTOBJECT) {
      if (TEMP_OBJECT(source) || TEMP2_OBJECT(source)) {
        if (!ALLOC_OBJECT(new_value)) {
          logError(printf("itf_create2: ALLOC_OBJECT() failed.\n"););
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
    logFunction(printf("itf_create2 --> ");
                trace1(dest);
                printf("\n"););
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
    logFunction(printf("itf_destr(");
                trace1(arg_1(arguments));
                printf(")\n"););
    just_interface(arg_1(arguments));
    old_value = take_interface(arg_1(arguments));
    if (unlikely(CATEGORY_OF_OBJ(old_value) != STRUCTOBJECT)) {
      logError(printf("itf_destr(");
               trace1(arg_1(arguments));
               printf("): Category of value is not STRUCTOBJECT.\n"););
      run_error(STRUCTOBJECT, old_value);
    } else {
      old_struct = take_struct(old_value);
      if (old_struct != NULL) {
        logMessage(printf("itf_destr: %s usage_count=" FMT_U_MEM
                          ", " FMT_U_MEM " ",
                          old_struct->usage_count != 0 ? "Decrease"
                                                       : "Keep",
                          old_struct->usage_count,
                          (memSizeType) old_value);
                   trace1(old_value);
                   printf("\n"););
        if (old_struct->usage_count != 0) {
          old_struct->usage_count--;
          if (old_struct->usage_count == 0) {
            destr_struct(old_struct->stru, old_struct->size);
            FREE_STRUCT(old_struct, old_struct->size);
            /* This function just removes objects without property. */
            /* Objects with property just lose their struct value.  */
            /* For these objects the HAS_PROPERTY flag and the      */
            /* descriptor.property stay unchanged. Objects with     */
            /* property will be removed later by close_stack(). The */
            /* descriptor.property will be freed together with the  */
            /* object.                                              */
            if (HAS_PROPERTY(old_value)) {
              old_value->value.structValue = NULL;
              logMessage(printf("itf_destr: Struct object with property "
                                FMT_U_MEM " ", (memSizeType) old_value);
                         trace1(old_value);
                         printf("\n"););
            } else {
              FREE_OBJECT(old_value);
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    arg_1(arguments)->value.objValue = NULL;
    SET_UNUSED_FLAG(arg_1(arguments));
    logFunction(printf("itf_destr -->\n"););
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
        logError(printf("itf_new: ALLOC_STRUCT() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result_struct->usage_count = 1;
      result_struct->size = stru1->size;
      if (!crea_struct(result_struct->stru, stru1->stru, stru1->size)) {
        logError(printf("itf_new: crea_struct() failed.\n"););
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
    logError(printf("itf_select(");
             trace1(arg_1(arguments));
             printf(", ");
             trace1(arg_3(arguments));
             printf("): Selector not found.\n"););
    return raise_exception(SYS_RNG_EXCEPTION);
  } /* itf_select */



objectType itf_to_interface (listType arguments)

  {
    objectType stru_arg;
    objectType new_value;
    objectType result;

  /* itf_to_interface */
    stru_arg = arg_1(arguments);
    logFunction(printf("itf_to_interface(" FMT_U_MEM " ",
                       (memSizeType) stru_arg);
                trace1(stru_arg);
                printf(")\n"););
    isit_struct(stru_arg);
    if (!ALLOC_OBJECT(new_value)) {
      logError(printf("itf_to_interface: ALLOC_OBJECT() failed.\n"););
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
    logFunction(printf("itf_to_interface --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* itf_to_interface */
