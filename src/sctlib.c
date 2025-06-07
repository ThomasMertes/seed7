/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2002, 2012, 2013, 2015  Thomas Mertes      */
/*                2016, 2019, 2021  Thomas Mertes                   */
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
/*  File: seed7/src/sctlib.c                                        */
/*  Changes: 1993, 1994, 2002, 2012, 2013, 2015  Thomas Mertes      */
/*           2016, 2019, 2021  Thomas Mertes                        */
/*  Content: All primitive actions for structure types.             */
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
#include "sctlib.h"



objectType sct_alloc (listType arguments)

  {
    objectType stru_from;
    objectType struct_exec_object;
    memSizeType new_size;
    structType new_stru;
    objectType result;

  /* sct_alloc */
    stru_from = arg_1(arguments);
    isit_struct(stru_from);
    if (TEMP_OBJECT(stru_from)) {
      CLEAR_TEMP_FLAG(stru_from);
      result = stru_from;
      arg_1(arguments) = NULL;
    } else {
      if (unlikely(!ALLOC_OBJECT(result))) {
        logError(printf("sct_alloc: ALLOC_OBJECT() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_size = take_struct(stru_from)->size;
        if (unlikely(!ALLOC_STRUCT(new_stru, new_size))) {
          logError(printf("sct_alloc: ALLOC_STRUCT() failed.\n"););
          FREE_OBJECT(result);
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          struct_exec_object = curr_exec_object;
          new_stru->usage_count = 1;
          new_stru->size = new_size;
          if (unlikely(!crea_struct(new_stru->stru,
                                    take_struct(stru_from)->stru,
                                    new_size))) {
            logError(printf("sct_alloc: crea_struct() failed.\n"););
            FREE_OBJECT(result);
            FREE_STRUCT(new_stru, new_size);
            return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                           struct_exec_object,
                                           arguments);
          } /* if */
          result->type_of = stru_from->type_of;
          memcpy(&result->descriptor, &stru_from->descriptor,
              sizeof(descriptorUnion));
          /* Copies the POSINFO flag (and all other flags): */
          INIT_CATEGORY_OF_OBJ(result, stru_from->objcategory);
          result->value.structValue = new_stru;
        } /* if */
      } /* if */
    } /* if */
    return bld_reference_temp(result);
  } /* sct_alloc */



objectType sct_cat (listType arguments)

  {
    structType stru1;
    structType stru2;
    memSizeType stru1_size;
    objectType struct_exec_object;
    memSizeType result_size;
    structType result;

  /* sct_cat */
    isit_struct(arg_1(arguments));
    isit_struct(arg_3(arguments));
    logFunction(printf("sct_cat(");
                trace1(arg_1(arguments));
                printf(", ");
                trace1(arg_3(arguments));
                printf(")\n"););
    stru1 = take_struct(arg_1(arguments));
    stru2 = take_struct(arg_3(arguments));
    stru1_size = stru1->size;
    struct_exec_object = curr_exec_object;
    result_size = stru1_size + stru2->size;
    if (TEMP_OBJECT(arg_1(arguments))) {
      result = REALLOC_STRUCT(stru1, stru1_size, result_size);
      if (unlikely(result == NULL)) {
        logError(printf("sct_cat: REALLOC_STRUCT() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        COUNT3_STRUCT(stru1_size, result_size);
        result->size = result_size;
        arg_1(arguments)->value.structValue = NULL;
      } /* if */
    } else {
      if (unlikely(!ALLOC_STRUCT(result, result_size))) {
        logError(printf("sct_cat: ALLOC_STRUCT() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->usage_count = 1;
        result->size = result_size;
        if (unlikely(!crea_struct(result->stru, stru1->stru,
                                  stru1_size))) {
          logError(printf("sct_cat: crea_struct() failed.\n"););
          FREE_STRUCT(result, result_size);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         struct_exec_object,
                                         arguments);
        } /* if */
      } /* if */
    } /* if */
    if (TEMP_OBJECT(arg_3(arguments))) {
      memcpy(&result->stru[stru1_size], stru2->stru,
          (size_t) (stru2->size * sizeof(objectRecord)));
      FREE_STRUCT(stru2, stru2->size);
      arg_3(arguments)->value.structValue = NULL;
    } else {
      if (unlikely(!crea_struct(&result->stru[stru1_size],
                                stru2->stru, stru2->size))) {
        logError(printf("sct_cat: crea_struct() failed.\n"););
        destr_struct(result->stru, stru1_size);
        FREE_STRUCT(result, result_size);
        return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                       struct_exec_object,
                                       arguments);
      } /* if */
    } /* if */
    logFunction(printf("sct_cat -->\n"););
    return bld_struct_temp(result);
  } /* sct_cat */



objectType sct_conv (listType arguments)

  {
    objectType stru_arg;
    structType stru1;
    objectType struct_exec_object;
    structType result_struct;
    objectType result;

  /* sct_conv */
    stru_arg = arg_3(arguments);
    isit_struct(stru_arg);
    logFunction(printf("sct_conv(");
                trace1(stru_arg);
                printf(")\n"););
    if (TEMP_OBJECT(stru_arg)) {
      result = stru_arg;
      result->type_of = NULL;
      arg_3(arguments) = NULL;
    } else {
      stru1 = take_struct(stru_arg);
      if (unlikely(!ALLOC_STRUCT(result_struct, stru1->size))) {
        logError(printf("sct_conv: ALLOC_STRUCT() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        struct_exec_object = curr_exec_object;
        result_struct->usage_count = 1;
        result_struct->size = stru1->size;
        if (unlikely(!crea_struct(result_struct->stru,
                                  stru1->stru, stru1->size))) {
          logError(printf("sct_conv: crea_struct() failed.\n"););
          FREE_STRUCT(result_struct, stru1->size);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         struct_exec_object,
                                         arguments);
        } /* if */
        result = bld_struct_temp(result_struct);
      } /* if */
    } /* if */
    logFunction(printf("sct_conv -->\n"););
    return result;
  } /* sct_conv */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType sct_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    objectType struct_exec_object;
    memSizeType source_size;
    structType dest_struct;

  /* sct_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_struct(dest);
    isit_struct(source);
    is_variable(dest);
    logFunction(printf("sct_cpy(");
                trace1(dest);
                printf(", ");
                trace1(source);
                printf(")\n"););
    dest_struct = take_struct(dest);
    if (TEMP_OBJECT(source)) {
      destr_struct(dest_struct->stru, dest_struct->size);
      FREE_STRUCT(dest_struct, dest_struct->size);
      dest->value.structValue = take_struct(source);
      source->value.structValue = NULL;
    } else {
      source_size = take_struct(source)->size;
      if (dest_struct->size != source_size) {
        if (unlikely(!ALLOC_STRUCT(dest_struct, source_size))) {
          logError(printf("sct_cpy: ALLOC_STRUCT() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          struct_exec_object = curr_exec_object;
          dest_struct->usage_count = 1;
          dest_struct->size = source_size;
          if (unlikely(!crea_struct(dest_struct->stru,
                                    take_struct(source)->stru,
                                    source_size))) {
            logError(printf("sct_cpy: crea_struct() failed.\n"););
            FREE_STRUCT(dest_struct, source_size);
            return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                           struct_exec_object,
                                           arguments);
          } /* if */
          destr_struct(take_struct(dest)->stru,
              take_struct(dest)->size);
          FREE_STRUCT(take_struct(dest),
              take_struct(dest)->size);
          dest->value.structValue = dest_struct;
        } /* if */
      } else {
        /* The usage_count is left unchanged for a deep copy. */
        cpy_array(dest_struct->stru, take_struct(source)->stru,
            source_size);
      } /* if */
    } /* if */
    logFunction(printf("sct_cpy(");
                trace1(dest);
                printf(", *) -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* sct_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType sct_create (listType arguments)

  {
    objectType dest;
    objectType source;
    objectType struct_exec_object;
    memSizeType new_size;
    structType new_stru;

  /* sct_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    logFunction(printf("sct_create(");
                trace1(dest);
                printf(", ");
                trace1(source);
                printf(")\n"););
    SET_CATEGORY_OF_OBJ(dest, STRUCTOBJECT);
    isit_struct(source);
    if (TEMP_OBJECT(source)) {
/*
printf("create: pointer assignment\n");
*/
      dest->value.structValue = take_struct(source);
      /* printf("sct_create: usage_count=%u %lu\n",
          dest->value.structValue->usage_count,
          (unsigned long) dest->value.structValue); */
      source->value.structValue = NULL;
    } else {
      new_size = take_struct(source)->size;
      if (unlikely(!ALLOC_STRUCT(new_stru, new_size))) {
        logError(printf("sct_create: ALLOC_STRUCT() failed.\n"););
        dest->value.structValue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        struct_exec_object = curr_exec_object;
        new_stru->usage_count = 1;
        new_stru->size = new_size;
        dest->value.structValue = new_stru;
        if (unlikely(!crea_struct(new_stru->stru,
                                  take_struct(source)->stru,
                                  new_size))) {
          logError(printf("sct_create: crea_struct() failed.\n"););
          FREE_STRUCT(new_stru, new_size);
          dest->value.structValue = NULL;
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         struct_exec_object,
                                         arguments);
        } /* if */
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* sct_create */



/**
 *  Free the memory referred by 'old_struct/arg_1'.
 *  This function maintains a usage count. If the usage count
 *  reaches zero 'old_struct/arg_1' is freed and set to NULL.
 *  The memory where 'old_struct/arg_1' is stored can be freed
 *  after sct_destr.
 */
objectType sct_destr (listType arguments)

  {
    structType old_struct;

  /* sct_destr */
    isit_struct(arg_1(arguments));
    old_struct = take_struct(arg_1(arguments));
    logFunction(printf("sct_destr(" FMT_U_MEM
                       " (usage=" FMT_U_MEM "))\n",
                       (memSizeType) old_struct,
                       old_struct != NULL ? old_struct->usage_count
                                          : (memSizeType) 0););
    if (old_struct != NULL) {
      logMessage(printf("sct_destr: %s usage_count=" FMT_U_MEM
                        ", " FMT_U_MEM " ",
                        old_struct->usage_count != 0 ? "Decrease"
                                                     : "Keep",
                        old_struct->usage_count,
                        (memSizeType) arg_1(arguments));
                 trace1(arg_1(arguments));
                 printf("\n"););
      if (old_struct->usage_count != 0) {
        old_struct->usage_count--;
        if (old_struct->usage_count == 0) {
          destr_struct(old_struct->stru, old_struct->size);
          FREE_STRUCT(old_struct, old_struct->size);
          arg_1(arguments)->value.structValue = NULL;
          SET_UNUSED_FLAG(arg_1(arguments));
        } /* if */
      } /* if */
    } else {
      SET_UNUSED_FLAG(arg_1(arguments));
    } /* if */
    logFunction(printf("sct_destr -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* sct_destr */



objectType sct_empty (listType arguments)

  {
    structType result;

  /* sct_empty */
    if (unlikely(!ALLOC_STRUCT(result, 0))) {
      logError(printf("sct_empty: ALLOC_STRUCT() failed.\n"););
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      /* Note that the size of the allocated memory is smaller than */
      /* the size of the struct. But this is okay, because the */
      /* element 'stru' is not used. */
      result->usage_count = 1;
      result->size = 0;
    } /* if */
    return bld_struct_temp(result);
  } /* sct_empty */



objectType sct_incl (listType arguments)

  {
    objectType stru_variable;
    structType stru_to;
    objectType elem;
    memSizeType stru_size;

  /* sct_incl */
    stru_variable = arg_1(arguments);
    isit_struct(stru_variable);
    is_variable(stru_variable);
    stru_to = take_struct(stru_variable);
    isit_reference(arg_2(arguments));
    elem = take_reference(arg_2(arguments));
    logFunction(printf("sct_incl(");
                trace1(stru_variable);
                printf(", ");
                trace1(elem);
                printf(")\n"););
    stru_size = stru_to->size;
    stru_to = REALLOC_STRUCT(stru_to, stru_size, stru_size + 1);
    if (unlikely(stru_to == NULL)) {
      logError(printf("sct_incl: REALLOC_STRUCT() failed.\n"););
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    COUNT3_STRUCT(stru_size, stru_size + 1);
    stru_variable->value.structValue = stru_to;
    memcpy(&stru_to->stru[stru_size], elem, sizeof(objectRecord));
    stru_to->size = stru_size + 1;
    /* The elem->value has been moved to the struct. */
    memset(&elem->value, 0, sizeof(valueUnion));
    return SYS_EMPTY_OBJECT;
  } /* sct_incl */



objectType sct_lng (listType arguments)

  { /* sct_lng */
    isit_struct(arg_1(arguments));
    return bld_int_temp((intType) take_struct(arg_1(arguments))->size);
  } /* sct_lng */



objectType sct_refidx (listType arguments)

  {
    structType stru1;
    intType position;
    objectType struct_pointer;
    objectType result;

  /* sct_refidx */
    isit_struct(arg_1(arguments));
    isit_int(arg_3(arguments));
    stru1 = take_struct(arg_1(arguments));
    position = take_int(arg_3(arguments));
    struct_pointer = stru1->stru;
    if (position >= 1 && ((memSizeType) position) <= stru1->size) {
      result = bld_reference_temp(&struct_pointer[position - 1]);
    } else {
      logError(printf("sct_refidx(" FMT_U_MEM ", " FMT_D "): "
                      "Index out of range.\n",
                      (memSizeType) stru1, position););
      result = raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    return result;
  } /* sct_refidx */



objectType sct_select (listType arguments)

  {
    structType stru1;
    objectType selector;
    objectType selector_syobject;
    memSizeType position;
    objectType struct_pointer;
    objectType result;

  /* sct_select */
    logFunction(printf("sct_select(");
                trace1(arg_1(arguments));
                printf(")\n"););
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
      struct_pointer = &stru1->stru[position - 1];
      while (position > 0) {
/*
printf("test " FMT_U_MEM ": ", position);
trace1(struct_pointer);
printf("\n");
*/
        if (HAS_ENTITY(struct_pointer) &&
            GET_ENTITY(struct_pointer)->syobject == selector_syobject) {
          if (TEMP_OBJECT(arg_1(arguments))) {
/*
            printf("sct_select of TEMP_OBJECT\n");
            printf("stru1 ");
            trace1(arg_1(arguments));
            printf("\n");
            printf("selector ");
            trace1(selector);
            printf("\n");
*/
            /* The struct will be destroyed after selecting. */
            /* A copy is necessary here to avoid a crash !!!!! */
            if (unlikely(!ALLOC_OBJECT(result))) {
              logError(printf("sct_select: ALLOC_OBJECT() failed.\n"););
              result = raise_exception(SYS_MEM_EXCEPTION);
            } else {
              memcpy(result, struct_pointer, sizeof(objectRecord));
              SET_TEMP_FLAG(result);
              destr_struct(stru1->stru,
                  (memSizeType) (struct_pointer - stru1->stru));
              destr_struct(&struct_pointer[1],
                  (stru1->size - (memSizeType) (struct_pointer - stru1->stru) - 1));
              FREE_STRUCT(stru1, stru1->size);
              arg_1(arguments)->value.structValue = NULL;
            } /* if */
            return result;
          } else {
            return struct_pointer;
          } /* if */
        } /* if */
        position--;
        struct_pointer--;
      } /* while */
    } /* if */
    logError(printf("sct_select(");
             trace1(arg_1(arguments));
             printf(", ");
             trace1(arg_3(arguments));
             printf("): Selector not found.\n"););
    return raise_exception(SYS_RNG_EXCEPTION);
  } /* sct_select */
