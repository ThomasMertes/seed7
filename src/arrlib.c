/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2019  Thomas Mertes                        */
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
/*  File: seed7/src/arrlib.c                                        */
/*  Changes: 1993, 1994, 2013, 2015, 2016, 2019  Thomas Mertes      */
/*  Content: All primitive actions for array types.                 */
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
#include "executl.h"
#include "objutl.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "arrlib.h"


#define QSORT_LIMIT 8



/**
 *  Sort an array of 'objectRecord' elements with the quicksort algorithm.
 *  In contrast to qsort() this function uses a different compare function.
 *  The compare function of qsort() has two void pointers as parameters.
 *  @param begin_sort Pointer to first element to be sorted.
 *  @param end_sort Pointer to the last element to be sorted.
 *  @param cmp_func Object describing the compare function to be used.
 */
static void qsort_array (objectType begin_sort, objectType end_sort,
    objectType cmp_func)

  {
    objectRecord compare_elem;
    objectRecord help_element;
    objectType middle_elem;
    objectType less_elem;
    objectType greater_elem;
    objectType cmp_obj;
    intType cmp;

  /* qsort_array */
    if (end_sort - begin_sort < QSORT_LIMIT) {
      /* Use insertion sort */
      for (middle_elem = begin_sort + 1; middle_elem <= end_sort; middle_elem++) {
        memcpy(&compare_elem, middle_elem, sizeof(objectRecord));
        less_elem = begin_sort - 1;
        do {
          less_elem++;
          cmp_obj = param3_call(cmp_func, less_elem, &compare_elem, cmp_func);
          isit_int2(cmp_obj);
          cmp = take_int(cmp_obj);
          FREE_OBJECT(cmp_obj);
        } while (cmp < 0);
        memmove(&less_elem[1], less_elem,
                (memSizeType) (middle_elem - less_elem) * sizeof(objectRecord));
        memcpy(less_elem, &compare_elem, sizeof(objectRecord));
      } /* for */
    } else {
      middle_elem = &begin_sort[(memSizeType) (end_sort - begin_sort) >> 1];
      memcpy(&compare_elem, middle_elem, sizeof(objectRecord));
      memcpy(middle_elem, end_sort, sizeof(objectRecord));
      memcpy(end_sort, &compare_elem, sizeof(objectRecord));
      less_elem = begin_sort - 1;
      greater_elem = end_sort;
      do {
        do {
          less_elem++;
          cmp_obj = param3_call(cmp_func, less_elem, &compare_elem, cmp_func);
          isit_int2(cmp_obj);
          cmp = take_int(cmp_obj);
          FREE_OBJECT(cmp_obj);
        } while (cmp < 0);
        do {
          greater_elem--;
          cmp_obj = param3_call(cmp_func, greater_elem, &compare_elem, cmp_func);
          isit_int2(cmp_obj);
          cmp = take_int(cmp_obj);
          FREE_OBJECT(cmp_obj);
        } while (cmp > 0 && greater_elem != begin_sort);
        memcpy(&help_element, less_elem, sizeof(objectRecord));
        memcpy(less_elem, greater_elem, sizeof(objectRecord));
        memcpy(greater_elem, &help_element, sizeof(objectRecord));
      } while (greater_elem > less_elem);
      memcpy(greater_elem, less_elem, sizeof(objectRecord));
      memcpy(less_elem, &compare_elem, sizeof(objectRecord));
      memcpy(end_sort, &help_element, sizeof(objectRecord));
      qsort_array(begin_sort, less_elem - 1, cmp_func);
      qsort_array(less_elem + 1, end_sort, cmp_func);
    } /* if */
  } /* qsort_array */



/**
 *  Reverse sort an array of 'objectRecord' elements with the quicksort algorithm.
 *  In contrast to qsort() this function uses a different compare function.
 *  The compare function of qsort() has two void pointers as parameters.
 *  @param begin_sort Pointer to first element to be sorted.
 *  @param end_sort Pointer to the last element to be sorted.
 *  @param cmp_func Object describing the compare function to be used.
 */
static void qsort_array_reverse (objectType begin_sort, objectType end_sort,
    objectType cmp_func)

  {
    objectRecord compare_elem;
    objectRecord help_element;
    objectType middle_elem;
    objectType less_elem;
    objectType greater_elem;
    objectType cmp_obj;
    intType cmp;

  /* qsort_array_reverse */
    if (end_sort - begin_sort < QSORT_LIMIT) {
      /* Use insertion sort */
      for (middle_elem = begin_sort + 1; middle_elem <= end_sort; middle_elem++) {
        memcpy(&compare_elem, middle_elem, sizeof(objectRecord));
        greater_elem = begin_sort - 1;
        do {
          greater_elem++;
          cmp_obj = param3_call(cmp_func, greater_elem, &compare_elem, cmp_func);
          isit_int2(cmp_obj);
          cmp = take_int(cmp_obj);
          FREE_OBJECT(cmp_obj);
        } while (cmp > 0);
        memmove(&greater_elem[1], greater_elem,
                (memSizeType) (middle_elem - greater_elem) * sizeof(objectRecord));
        memcpy(greater_elem, &compare_elem, sizeof(objectRecord));
      } /* for */
    } else {
      middle_elem = &begin_sort[(memSizeType) (end_sort - begin_sort) >> 1];
      memcpy(&compare_elem, middle_elem, sizeof(objectRecord));
      memcpy(middle_elem, end_sort, sizeof(objectRecord));
      memcpy(end_sort, &compare_elem, sizeof(objectRecord));
      greater_elem = begin_sort - 1;
      less_elem = end_sort;
      do {
        do {
          greater_elem++;
          cmp_obj = param3_call(cmp_func, greater_elem, &compare_elem, cmp_func);
          isit_int2(cmp_obj);
          cmp = take_int(cmp_obj);
          FREE_OBJECT(cmp_obj);
        } while (cmp > 0);
        do {
          less_elem--;
          cmp_obj = param3_call(cmp_func, less_elem, &compare_elem, cmp_func);
          isit_int2(cmp_obj);
          cmp = take_int(cmp_obj);
          FREE_OBJECT(cmp_obj);
        } while (cmp < 0 && less_elem != begin_sort);
        memcpy(&help_element, greater_elem, sizeof(objectRecord));
        memcpy(greater_elem, less_elem, sizeof(objectRecord));
        memcpy(less_elem, &help_element, sizeof(objectRecord));
      } while (less_elem > greater_elem);
      memcpy(less_elem, greater_elem, sizeof(objectRecord));
      memcpy(greater_elem, &compare_elem, sizeof(objectRecord));
      memcpy(end_sort, &help_element, sizeof(objectRecord));
      qsort_array_reverse(begin_sort, greater_elem - 1, cmp_func);
      qsort_array_reverse(greater_elem + 1, end_sort, cmp_func);
    } /* if */
  } /* qsort_array_reverse */



/**
 *  Append the array 'extension' to the array 'arr_variable'.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             array.
 */
objectType arr_append (listType arguments)

  {
    objectType arr_variable;
    arrayType arr_to;
    arrayType extension;
    arrayType new_arr;
    memSizeType new_size;
    memSizeType arr_to_size;
    memSizeType extension_size;
    objectType array_exec_object;

  /* arr_append */
    logFunction(printf("arr_append\n"););
    arr_variable = arg_1(arguments);
    isit_array(arr_variable);
    is_variable(arr_variable);
    arr_to = take_array(arr_variable);
    isit_array(arg_3(arguments));
    extension = take_array(arg_3(arguments));
    extension_size = arraySize(extension);
    if (extension_size != 0) {
      arr_to_size = arraySize(arr_to);
      if (unlikely(arr_to_size > MAX_ARR_LEN - extension_size ||
                   arr_to->max_position > (intType) (MAX_MEM_INDEX - extension_size))) {
        logError(printf("arr_append: Result size bigger than MAX_ARR_LEN.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_size = arr_to_size + extension_size;
        if (unlikely(!REALLOC_ARRAY(new_arr, arr_to, new_size))) {
          logError(printf("arr_append: REALLOC_ARRAY() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          COUNT3_ARRAY(arr_to_size, new_size);
          arr_variable->value.arrayValue = new_arr;
          if (TEMP_OBJECT(arg_3(arguments))) {
            memcpy(&new_arr->arr[arr_to_size], extension->arr,
                   (size_t) (extension_size * sizeof(objectRecord)));
            new_arr->max_position = arrayMaxPos(new_arr->min_position, new_size);
            FREE_ARRAY(extension, extension_size);
            arg_3(arguments)->value.arrayValue = NULL;
          } else {
            array_exec_object = curr_exec_object;
            /* It is possible that arr_to == extension holds. */
            /* In this case 'extension' must be corrected     */
            /* after realloc() enlarged 'arr_to'.             */
            if (arr_to == extension) {
              extension = new_arr;
            } /* if */
            if (unlikely(!crea_array(&new_arr->arr[arr_to_size],
                                     extension->arr,
                                     extension_size))) {
              logError(printf("arr_append: crea_array() failed.\n"););
              if (unlikely(!REALLOC_ARRAY(arr_to, new_arr, arr_to_size))) {
                logError(printf("arr_append: REALLOC_ARRAY() failed.\n"););
                return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                               array_exec_object,
                                               arguments);
              } else {
                COUNT3_ARRAY(new_size, arr_to_size);
                arr_variable->value.arrayValue = arr_to;
                return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                               array_exec_object,
                                               arguments);
              } /* if */
            } else {
              new_arr->max_position = arrayMaxPos(new_arr->min_position, new_size);
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("arr_append -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* arr_append */



/**
 *  Generate an array literal from a tuple.
 */
objectType arr_arrlit (listType arguments)

  {
    objectType arr_arg;
    arrayType arr1;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result_array;
    objectType result;

  /* arr_arrlit */
    arr_arg = arg_3(arguments);
    isit_array(arr_arg);
    if (TEMP_OBJECT(arr_arg)) {
      result = arr_arg;
      result->type_of = NULL;
      arg_3(arguments) = NULL;
    } else {
      arr1 = take_array(arr_arg);
      result_size = arraySize(arr1);
      if (unlikely(result_size > MAX_MEM_INDEX)) {
        logError(printf("arr_arrlit(arr1 (size=" FMT_U_MEM ")): "
                        "Maximum index out of range.\n",
                        result_size););
        return raise_exception(SYS_RNG_EXCEPTION);
      } else if (unlikely(!ALLOC_ARRAY(result_array, result_size))) {
        logError(printf("arr_arrlit: ALLOC_ARRAY() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        array_exec_object = curr_exec_object;
        result_array->min_position = 1;
        result_array->max_position = (intType) result_size;
        if (unlikely(!crea_array(result_array->arr, arr1->arr,
                                 result_size))) {
          logError(printf("arr_arrlit: crea_array() failed.\n"););
          FREE_ARRAY(result_array, result_size);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         array_exec_object,
                                         arguments);
        } else {
          result = bld_array_temp(result_array);
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* arr_arrlit */



objectType arr_arrlit2 (listType arguments)

  {
    intType start_position;
    objectType arr_arg;
    arrayType arr1;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result_array;
    objectType result;

  /* arr_arrlit2 */
    logFunction(printf("arr_arrlit2\n"););
    isit_int(arg_2(arguments));
    start_position = take_int(arg_2(arguments));
    arr_arg = arg_4(arguments);
    isit_array(arr_arg);
    arr1 = take_array(arr_arg);
    result_size = arraySize(arr1);
    if (unlikely(start_position < MIN_MEM_INDEX ||
                 start_position > MAX_MEM_INDEX ||
                 (result_size != 0 &&
                  start_position > (intType) (MAX_MEM_INDEX - result_size + 1)) ||
                 (result_size == 0 && start_position == MIN_MEM_INDEX))) {
      logError(printf("arr_arrlit2(" FMT_D ", arr1 (size=" FMT_U_MEM ")): "
                      "Minimum or maximum index out of range.\n",
                      start_position, result_size););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      if (TEMP_OBJECT(arr_arg)) {
        arr1->min_position = start_position;
        arr1->max_position = arrayMaxPos(start_position, result_size);
        result = arr_arg;
        result->type_of = NULL;
        arg_4(arguments) = NULL;
      } else {
        if (unlikely(!ALLOC_ARRAY(result_array, result_size))) {
          logError(printf("arr_arrlit2: ALLOC_ARRAY() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          array_exec_object = curr_exec_object;
          result_array->min_position = start_position;
          result_array->max_position = arrayMaxPos(start_position, result_size);
          if (unlikely(!crea_array(result_array->arr, arr1->arr, result_size))) {
            logError(printf("arr_arrlit2: crea_array() failed.\n"););
            FREE_ARRAY(result_array, result_size);
            return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                           array_exec_object,
                                           arguments);
          } else {
            result = bld_array_temp(result_array);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("arr_arrlit2 -->\n"););
    return result;
  } /* arr_arrlit2 */



objectType arr_baselit (listType arguments)

  {
    objectType element;
    typeType element_type;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result;

  /* arr_baselit */
    element = arg_3(arguments);
    result_size = 1;
    if (unlikely(!ALLOC_ARRAY(result, result_size))) {
      logError(printf("arr_baselit: ALLOC_ARRAY() failed.\n"););
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->min_position = 1;
      result->max_position = 1;
      /* The element type is the type of the 3rd formal parameter */
      element_type = curr_exec_object->value.listValue->obj->
                     descriptor.property->params->next->next->obj->type_of;
      if (TEMP_OBJECT(element) && element->type_of == element_type) {
        CLEAR_TEMP_FLAG(element);
        SET_VAR_FLAG(element);
        SET_EMBEDDED_FLAG(element);
        memcpy(&result->arr[0], element, sizeof(objectRecord));
        FREE_OBJECT(element);
        arg_3(arguments) = NULL;
      } else {
        array_exec_object = curr_exec_object;
        if (unlikely(!arr_elem_initialisation(element_type,
                                              &result->arr[0],
                                              element))) {
          logError(printf("arr_baselit: arr_elem_initialisation() failed.\n"););
          FREE_ARRAY(result, result_size);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         array_exec_object,
                                         arguments);
        } /* if */
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_baselit */



objectType arr_baselit2 (listType arguments)

  {
    intType start_position;
    objectType element;
    typeType element_type;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result;

  /* arr_baselit2 */
    isit_int(arg_2(arguments));
    start_position = take_int(arg_2(arguments));
    element = arg_4(arguments);
    result_size = 1;
    if (unlikely(!ALLOC_ARRAY(result, result_size))) {
      logError(printf("arr_baselit2: ALLOC_ARRAY() failed.\n"););
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->min_position = start_position;
      result->max_position = start_position;
      /* The element type is the type of the 4th formal parameter */
      element_type = curr_exec_object->value.listValue->obj->
                     descriptor.property->params->next->next->next->obj->type_of;
      if (TEMP_OBJECT(element) && element->type_of == element_type) {
        CLEAR_TEMP_FLAG(element);
        SET_VAR_FLAG(element);
        SET_EMBEDDED_FLAG(element);
        memcpy(&result->arr[0], element, sizeof(objectRecord));
        FREE_OBJECT(element);
        arg_4(arguments) = NULL;
      } else {
        array_exec_object = curr_exec_object;
        if (unlikely(!arr_elem_initialisation(element_type,
                                              &result->arr[0],
                                              element))) {
          logError(printf("arr_baselit2: arr_elem_initialisation() failed.\n"););
          FREE_ARRAY(result, result_size);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         array_exec_object,
                                         arguments);
        } /* if */
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_baselit2 */



/**
 *  Concatenate two arrays.
 *  @return the result of the concatenation.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             array.
 */
objectType arr_cat (listType arguments)

  {
    arrayType arr1;
    arrayType arr2;
    memSizeType arr1_size;
    memSizeType arr2_size;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result;

  /* arr_cat */
    logFunction(printf("arr_cat\n"););
    isit_array(arg_1(arguments));
    isit_array(arg_3(arguments));
    arr1 = take_array(arg_1(arguments));
    arr2 = take_array(arg_3(arguments));
    arr1_size = arraySize(arr1);
    arr2_size = arraySize(arr2);
    if (unlikely(arr1_size > MAX_ARR_LEN - arr2_size ||
                 arr1->max_position > (intType) (MAX_MEM_INDEX - arr2_size))) {
      logError(printf("arr_cat: Result size bigger than MAX_ARR_LEN.\n"););
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      array_exec_object = curr_exec_object;
      result_size = arr1_size + arr2_size;
      if (TEMP_OBJECT(arg_1(arguments))) {
        if (unlikely(!REALLOC_ARRAY(result, arr1, result_size))) {
          logError(printf("arr_cat: REALLOC_ARRAY() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          COUNT3_ARRAY(arr1_size, result_size);
          result->max_position = arrayMaxPos(result->min_position, result_size);
          arg_1(arguments)->value.arrayValue = NULL;
        } /* if */
      } else {
        if (unlikely(!ALLOC_ARRAY(result, result_size))) {
          logError(printf("arr_cat: ALLOC_ARRAY() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          result->min_position = arr1->min_position;
          result->max_position = arrayMaxPos(result->min_position, result_size);
          if (unlikely(!crea_array(result->arr, arr1->arr, arr1_size))) {
            logError(printf("arr_cat: crea_array() failed.\n"););
            FREE_ARRAY(result, result_size);
            return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                           array_exec_object,
                                           arguments);
          } /* if */
        } /* if */
      } /* if */
      if (TEMP_OBJECT(arg_3(arguments))) {
        memcpy(&result->arr[arr1_size], arr2->arr,
               (size_t) (arr2_size * sizeof(objectRecord)));
        FREE_ARRAY(arr2, arr2_size);
        arg_3(arguments)->value.arrayValue = NULL;
      } else {
        if (unlikely(!crea_array(&result->arr[arr1_size],
                                 arr2->arr, arr2_size))) {
          logError(printf("arr_cat: crea_array() failed.\n"););
          destr_array(result->arr, arr1_size);
          FREE_ARRAY(result, result_size);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         array_exec_object,
                                         arguments);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("arr_cat -->\n"););
    return bld_array_temp(result);
  } /* arr_cat */



objectType arr_conv (listType arguments)

  {
    objectType arr_arg;
    arrayType arr1;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result;

  /* arr_conv */
    arr_arg = arg_3(arguments);
    isit_array(arr_arg);
    if (TEMP_OBJECT(arr_arg)) {
      result = take_array(arr_arg);
      arr_arg->value.arrayValue = NULL;
      return bld_array_temp(result);
    } else if (VAR_OBJECT(arr_arg)) {
      return arr_arg;
    } else {
      arr1 = take_array(arr_arg);
      result_size = arraySize(arr1);
      if (unlikely(!ALLOC_ARRAY(result, result_size))) {
        logError(printf("arr_conv: ALLOC_ARRAY() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        array_exec_object = curr_exec_object;
        result->min_position = arr1->min_position;
        result->max_position = arr1->max_position;
        if (unlikely(!crea_array(result->arr, arr1->arr, result_size))) {
          logError(printf("arr_conv: crea_array() failed.\n"););
          FREE_ARRAY(result, result_size);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         array_exec_object,
                                         arguments);
        } else {
          return bld_array_temp(result);
        } /* if */
      } /* if */
    } /* if */
  } /* arr_conv */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType arr_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    arrayType arr_dest;
    arrayType arr_source;
    memSizeType arr_source_size;
    arrayType new_arr;
    objectType array_exec_object;

  /* arr_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_array(dest);
    isit_array(source);
    is_variable(dest);
    arr_dest = take_array(dest);
    arr_source = take_array(source);
    if (TEMP_OBJECT(source)) {
      free_array(arr_dest);
      dest->value.arrayValue = arr_source;
      source->value.arrayValue = NULL;
    } else {
      arr_source_size = arraySize(arr_source);
      if (arr_dest->min_position != arr_source->min_position ||
          arr_dest->max_position != arr_source->max_position) {
        if (unlikely(!ALLOC_ARRAY(new_arr, arr_source_size))) {
          logError(printf("arr_cpy: ALLOC_ARRAY() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          array_exec_object = curr_exec_object;
          new_arr->min_position = arr_source->min_position;
          new_arr->max_position = arr_source->max_position;
          if (unlikely(!crea_array(new_arr->arr,
                                   arr_source->arr, arr_source_size))) {
            logError(printf("arr_cpy: crea_array() failed.\n"););
            FREE_ARRAY(new_arr, arr_source_size);
            return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                           array_exec_object,
                                           arguments);
          } else {
            free_array(arr_dest);
            dest->value.arrayValue = new_arr;
          } /* if */
        } /* if */
      } else {
        cpy_array(arr_dest->arr, arr_source->arr, arr_source_size);
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* arr_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType arr_create (listType arguments)

  {
    objectType dest;
    objectType source;
    arrayType arr_source;
    objectType array_exec_object;
    memSizeType new_size;
    arrayType new_arr;

  /* arr_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_array(source);
    arr_source = take_array(source);
    SET_CATEGORY_OF_OBJ(dest, ARRAYOBJECT);
    if (TEMP_OBJECT(source)) {
      dest->value.arrayValue = arr_source;
      source->value.arrayValue = NULL;
    } else {
      new_size = arraySize(arr_source);
      if (unlikely(!ALLOC_ARRAY(new_arr, new_size))) {
        logError(printf("arr_create: ALLOC_ARRAY() failed.\n"););
        dest->value.arrayValue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        array_exec_object = curr_exec_object;
        new_arr->min_position = arr_source->min_position;
        new_arr->max_position = arr_source->max_position;
        if (unlikely(!crea_array(new_arr->arr,
                                 arr_source->arr, new_size))) {
          logError(printf("arr_create: crea_array() failed.\n"););
          FREE_ARRAY(new_arr, new_size);
          dest->value.arrayValue = NULL;
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         array_exec_object,
                                         arguments);
        } else {
          dest->value.arrayValue = new_arr;
        } /* if */
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* arr_create */



/**
 *  Free the memory referred by 'old_arr/arg_1'.
 *  After arr_destr is left 'old_arr/arg_1' is NULL.
 *  The memory where 'old_arr/arg_1' is stored can be freed afterwards.
 */
objectType arr_destr (listType arguments)

  {
    arrayType old_arr;

  /* arr_destr */
    isit_array(arg_1(arguments));
    old_arr = take_array(arg_1(arguments));
    if (old_arr != NULL) {
      free_array(old_arr);
      arg_1(arguments)->value.arrayValue = NULL;
    } /* if */
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* arr_destr */



/**
 *  Get an empty array (an array without elements).
 *  @return an empty array.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType arr_empty (listType arguments)

  {
    arrayType result;

  /* arr_empty */
    if (unlikely(!ALLOC_ARRAY(result, 0))) {
      logError(printf("arr_empty: ALLOC_ARRAY() failed.\n"););
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      /* Note that the size of the allocated memory is smaller than */
      /* the size of the struct. But this is okay, because the */
      /* element 'arr' is not used. */
      result->min_position = 1;
      result->max_position = 0;
    } /* if */
    return bld_array_temp(result);
  } /* arr_empty */



objectType arr_extend (listType arguments)

  {
    arrayType arr1;
    objectType element;
    typeType element_type;
    memSizeType arr1_size;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result;

  /* arr_extend */
    logFunction(printf("arr_extend\n"););
    isit_array(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    element = arg_3(arguments);
    arr1_size = arraySize(arr1);
    if (unlikely(arr1_size > MAX_ARR_LEN - 1 ||
                 arr1->max_position > (intType) (MAX_MEM_INDEX - 1))) {
      logError(printf("arr_extend: Result size bigger than MAX_ARR_LEN.\n"););
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      array_exec_object = curr_exec_object;
      result_size = arr1_size + 1;
      if (TEMP_OBJECT(arg_1(arguments))) {
        if (unlikely(!REALLOC_ARRAY(result, arr1, result_size))) {
          logError(printf("arr_extend: REALLOC_ARRAY() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          COUNT3_ARRAY(arr1_size, result_size);
          result->max_position++;
          arg_1(arguments)->value.arrayValue = NULL;
        } /* if */
      } else {
        if (unlikely(!ALLOC_ARRAY(result, result_size))) {
          logError(printf("arr_extend: ALLOC_ARRAY() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          result->min_position = arr1->min_position;
          result->max_position = arr1->max_position + 1;
          if (unlikely(!crea_array(result->arr, arr1->arr, arr1_size))) {
            logError(printf("arr_extend: crea_array() failed.\n"););
            FREE_ARRAY(result, result_size);
            return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                           array_exec_object,
                                           arguments);
          } /* if */
        } /* if */
      } /* if */
      /* The element type is the type of the 3rd formal parameter */
      element_type = curr_exec_object->value.listValue->obj->
                     descriptor.property->params->next->next->obj->type_of;
      if (TEMP_OBJECT(element) && element->type_of == element_type) {
        CLEAR_TEMP_FLAG(element);
        SET_VAR_FLAG(element);
        SET_EMBEDDED_FLAG(element);
        memcpy(&result->arr[arr1_size], element, sizeof(objectRecord));
        FREE_OBJECT(element);
        arg_3(arguments) = NULL;
      } else {
        if (unlikely(!arr_elem_initialisation(element_type,
                                              &result->arr[arr1_size], element))) {
          logError(printf("arr_extend: arr_elem_initialisation() failed.\n"););
          destr_array(result->arr, arr1_size);
          FREE_ARRAY(result, result_size);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         array_exec_object,
                                         arguments);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("arr_extend -->\n"););
    return bld_array_temp(result);
  } /* arr_extend */



objectType arr_gen (listType arguments)

  {
    objectType element1;
    objectType element2;
    typeType element_type;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result;

  /* arr_gen */
    element1 = arg_1(arguments);
    element2 = arg_3(arguments);
    result_size = 2;
    if (unlikely(!ALLOC_ARRAY(result, result_size))) {
      logError(printf("arr_gen: ALLOC_ARRAY() failed.\n"););
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      array_exec_object = curr_exec_object;
      result->min_position = 1;
      result->max_position = 2;
      /* The element type is the type of the 1st formal parameter */
      element_type = curr_exec_object->value.listValue->obj->
                     descriptor.property->params->obj->type_of;
      if (TEMP_OBJECT(element1) && element1->type_of == element_type) {
        CLEAR_TEMP_FLAG(element1);
        SET_VAR_FLAG(element1);
        SET_EMBEDDED_FLAG(element1);
        memcpy(&result->arr[0], element1, sizeof(objectRecord));
        FREE_OBJECT(element1);
        arg_1(arguments) = NULL;
      } else {
        if (unlikely(!arr_elem_initialisation(element_type,
                                              &result->arr[0],
                                              element1))) {
          logError(printf("arr_gen: arr_elem_initialisation() failed.\n"););
          FREE_ARRAY(result, result_size);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         array_exec_object,
                                         arguments);
        } /* if */
      } /* if */
      if (TEMP_OBJECT(element2) && element2->type_of == element_type) {
        CLEAR_TEMP_FLAG(element2);
        SET_VAR_FLAG(element2);
        SET_EMBEDDED_FLAG(element2);
        memcpy(&result->arr[1], element2, sizeof(objectRecord));
        FREE_OBJECT(element2);
        arg_3(arguments) = NULL;
      } else {
        if (unlikely(!arr_elem_initialisation(element_type,
                                              &result->arr[1],
                                              element2))) {
          logError(printf("arr_gen: arr_elem_initialisation() failed.\n"););
          destr_array(result->arr, (memSizeType) 1);
          FREE_ARRAY(result, result_size);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         array_exec_object,
                                         arguments);
        } /* if */
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_gen */



/**
 *  Get a sub array ending at the position 'stop'.
 *  @return the sub array ending at the stop position.
 *  @exception INDEX_ERROR The stop position is less than pred(minIdx(arr1)).
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType arr_head (listType arguments)

  {
    arrayType arr1;
    intType stop;
    memSizeType arr1_size;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result;

  /* arr_head */
    logFunction(printf("arr_head\n"););
    isit_array(arg_1(arguments));
    isit_int(arg_4(arguments));
    arr1 = take_array(arg_1(arguments));
    stop = take_int(arg_4(arguments));
    arr1_size = arraySize(arr1);
    if (stop >= arr1->min_position && arr1_size >= 1) {
      if (stop > arr1->max_position) {
        stop = arr1->max_position;
      } /* if */
      result_size = arraySize2(arr1->min_position, stop);
      if (TEMP_OBJECT(arg_1(arguments))) {
        arg_1(arguments)->value.arrayValue = NULL;
        destr_array(&arr1->arr[result_size], arr1_size - result_size);
        if (unlikely(!REALLOC_ARRAY(result, arr1, result_size))) {
          logError(printf("arr_head: REALLOC_ARRAY() failed.\n"););
          destr_array(arr1->arr, result_size);
          FREE_ARRAY(arr1, arr1_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } else {
          COUNT3_ARRAY(arr1_size, result_size);
          result->max_position = stop;
        } /* if */
      } else {
        if (unlikely(!ALLOC_ARRAY(result, result_size))) {
          logError(printf("arr_head: ALLOC_ARRAY() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          array_exec_object = curr_exec_object;
          result->min_position = arr1->min_position;
          result->max_position = stop;
          if (unlikely(!crea_array(result->arr, arr1->arr, result_size))) {
            logError(printf("arr_head: crea_array() failed.\n"););
            FREE_ARRAY(result, result_size);
            return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                           array_exec_object,
                                           arguments);
          } /* if */
        } /* if */
      } /* if */
    } else if (unlikely(stop < arr1->min_position - 1)) {
      logError(printf("arr_head(arr1, " FMT_D "): "
                      "Stop index out of range (" FMT_D " .. " FMT_D ").\n",
                      stop, arr1->min_position, arr1->max_position););
      return raise_exception(SYS_IDX_EXCEPTION);
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arr_head(arr1 (size=" FMT_U_MEM "), " FMT_D "): "
                      "Cannot create empty array with minimum index.\n",
                      arr1_size, stop););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      if (unlikely(!ALLOC_ARRAY(result, 0))) {
        logError(printf("arr_head: ALLOC_ARRAY() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    logFunction(printf("arr_head -->\n"););
    return bld_array_temp(result);
  } /* arr_head */



/**
 *  Access one element from the array 'arr'.
 *  @return the element with the specified 'position' from 'arr'.
 *  @exception INDEX_ERROR If 'position' is less than arr_minidx(arr) or
 *                         greater than arr_maxidx(arr)
 */
objectType arr_idx (listType arguments)

  {
    arrayType arr1;
    intType position;
    objectType array_pointer;
    objectType result;

  /* arr_idx */
    logFunction(printf("arr_idx\n"););
    isit_array(arg_1(arguments));
    isit_int(arg_3(arguments));
    arr1 = take_array(arg_1(arguments));
    position = take_int(arg_3(arguments));
    if (unlikely(position < arr1->min_position ||
                 position > arr1->max_position)) {
      logError(printf("arr_idx(arr1, " FMT_D "): "
                      "Index out of range (" FMT_D " .. " FMT_D ").\n",
                      position, arr1->min_position, arr1->max_position););
      result = raise_exception(SYS_IDX_EXCEPTION);
    } else {
      array_pointer = arr1->arr;
      if (TEMP_OBJECT(arg_1(arguments))) {
        /* The array will be destroyed after indexing. */
        /* A copy is necessary here to avoid a crash !!!!! */
        if (unlikely(!ALLOC_OBJECT(result))) {
          logError(printf("arr_idx: ALLOC_OBJECT() failed.\n"););
          result = raise_exception(SYS_MEM_EXCEPTION);
        } else {
          memcpy(result, &array_pointer[position - arr1->min_position], sizeof(objectRecord));
          SET_TEMP_FLAG(result);
          CLEAR_EMBEDDED_FLAG(result);
          destr_array(array_pointer, arraySize2(arr1->min_position, position) - 1);
          destr_array(&array_pointer[position - arr1->min_position + 1],
              arraySize2(position, arr1->max_position) - 1);
          FREE_ARRAY(arr1, arraySize(arr1));
          arg_1(arguments)->value.arrayValue = NULL;
          /* code to avoid destr_array:
          if (position != arr1->max_position) {
            memcpy(&array_pointer[position - arr1->min_position],
                   &array_pointer[arr1->max_position - arr1->min_position],
                   sizeof(objectRecord));
          }
          arr1->max_position--; */
        } /* if */
      } else {
        /* This is the normal case: The array will exist after indexing. */
        result = &array_pointer[position - arr1->min_position];
      } /* if */
    } /* if */
    logFunction(printf("arr_idx --> " F_U_MEM(08) " ", (memSizeType) result);
                trace1(result);
                printf("\n"););
    return result;
  } /* arr_idx */



/**
 *  Insert 'element' at 'position' into 'arr1'.
 *  @exception INDEX_ERROR If 'position' is less than minIdx(arr1) or
 *                         greater than succ(maxIdx(arr1))
 */
objectType arr_insert (listType arguments)

  {
    arrayType arr1;
    arrayType resized_arr1;
    intType position;
    typeType element_type;
    objectType element;
    objectRecord elementStore;
    objectType array_pointer;
    memSizeType arr1_size;
    objectType array_exec_object;
    objectType result;

  /* arr_insert */
    logFunction(printf("arr_insert\n"););
    isit_array(arg_1(arguments));
    isit_int(arg_2(arguments));
    is_variable(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    position = take_int(arg_2(arguments));
    element = arg_3(arguments);
    if (unlikely(position < arr1->min_position ||
                 position > arr1->max_position + 1)) {
      logError(printf("arr_insert(arr1, " FMT_D "): "
                      "Index out of range (" FMT_D " .. " FMT_D ").\n",
                      position, arr1->min_position, arr1->max_position + 1););
      result = raise_exception(SYS_IDX_EXCEPTION);
    } else {
      /* The element type is the type of the 3rd formal parameter */
      element_type = curr_exec_object->value.listValue->obj->
                     descriptor.property->params->next->next->obj->type_of;
      if (TEMP_OBJECT(element) && element->type_of == element_type) {
        arr1_size = arraySize(arr1);
        if (unlikely(!REALLOC_ARRAY(resized_arr1, arr1, arr1_size + 1))) {
          logError(printf("arr_insert: REALLOC_ARRAY() failed.\n"););
          result = raise_exception(SYS_MEM_EXCEPTION);
        } else {
          arr1 = resized_arr1;
          COUNT3_ARRAY(arr1_size, arr1_size + 1);
          array_pointer = arr1->arr;
          memmove(&array_pointer[position - arr1->min_position + 1],
                  &array_pointer[position - arr1->min_position],
                  arraySize2(position, arr1->max_position) * sizeof(objectRecord));
          CLEAR_TEMP_FLAG(element);
          SET_VAR_FLAG(element);
          SET_EMBEDDED_FLAG(element);
          memcpy(&array_pointer[position - arr1->min_position], element,
                 sizeof(objectRecord));
          FREE_OBJECT(element);
          arg_3(arguments) = NULL;
          arr1->max_position++;
          arg_1(arguments)->value.arrayValue = arr1;
          result = SYS_EMPTY_OBJECT;
        } /* if */
      } else {
        array_exec_object = curr_exec_object;
        if (unlikely(!arr_elem_initialisation(element_type,
                                              &elementStore, element))) {
          logError(printf("arr_insert: arr_elem_initialisation() failed.\n"););
          result = raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                           array_exec_object,
                                           arguments);
        } else {
          arr1_size = arraySize(arr1);
          if (unlikely(!REALLOC_ARRAY(resized_arr1, arr1, arr1_size + 1))) {
            logError(printf("arr_insert: REALLOC_ARRAY() failed.\n"););
            result = raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                             array_exec_object,
                                             arguments);
          } else {
            arr1 = resized_arr1;
            COUNT3_ARRAY(arr1_size, arr1_size + 1);
            array_pointer = arr1->arr;
            memmove(&array_pointer[position - arr1->min_position + 1],
                    &array_pointer[position - arr1->min_position],
                    arraySize2(position, arr1->max_position) * sizeof(objectRecord));
            memcpy(&array_pointer[position - arr1->min_position], &elementStore,
                   sizeof(objectRecord));
            arr1->max_position++;
            arg_1(arguments)->value.arrayValue = arr1;
            result = SYS_EMPTY_OBJECT;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("arr_insert -->\n"););
    return result;
  } /* arr_insert */



/**
 *  Insert 'elements' at 'position' into 'arr1'.
 *  @exception INDEX_ERROR If 'position' is less than minIdx(arr1) or
 *                         greater than succ(maxIdx(arr1))
 */
objectType arr_insert_array (listType arguments)

  {
    objectType arr_variable;
    arrayType arr1;
    arrayType resized_arr1;
    intType position;
    arrayType elements;
    objectType array_pointer;
    memSizeType new_size;
    memSizeType arr1_size;
    memSizeType elements_size;
    objectType array_exec_object;
    boolType restore = FALSE;

  /* arr_insert_array */
    logFunction(printf("arr_insert_array\n"););
    arr_variable = arg_1(arguments);
    isit_array(arr_variable);
    is_variable(arr_variable);
    arr1 = take_array(arr_variable);
    isit_int(arg_2(arguments));
    isit_array(arg_3(arguments));
    position = take_int(arg_2(arguments));
    elements = take_array(arg_3(arguments));
    elements_size = arraySize(elements);
    if (unlikely(position < arr1->min_position ||
                 position > arr1->max_position + 1)) {
      logError(printf("arr_insert_array(arr1, " FMT_D "): "
                      "Index out of range (" FMT_D " .. " FMT_D ").\n",
                      position, arr1->min_position, arr1->max_position + 1););
      return raise_exception(SYS_IDX_EXCEPTION);
    } else if (elements_size != 0) {
      arr1_size = arraySize(arr1);
      if (unlikely(arr1_size > MAX_ARR_LEN - elements_size ||
                   arr1->max_position > (intType) (MAX_MEM_INDEX - elements_size))) {
        logError(printf("arr_insert_array: Result size bigger than MAX_ARR_LEN.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_size = arr1_size + elements_size;
        if (unlikely(!REALLOC_ARRAY(resized_arr1, arr1, new_size))) {
          logError(printf("arr_insert_array: REALLOC_ARRAY() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          COUNT3_ARRAY(arr1_size, new_size);
          arr_variable->value.arrayValue = resized_arr1;
          array_pointer = resized_arr1->arr;
          memmove(&array_pointer[arrayIndex(resized_arr1, position) + elements_size],
                  &array_pointer[arrayIndex(resized_arr1, position)],
                  arraySize2(position, resized_arr1->max_position) * sizeof(objectRecord));
          if (TEMP_OBJECT(arg_3(arguments))) {
            memcpy(&array_pointer[arrayIndex(resized_arr1, position)], elements->arr,
                   (size_t) (elements_size * sizeof(objectRecord)));
            resized_arr1->max_position = arrayMaxPos(resized_arr1->min_position, new_size);
            FREE_ARRAY(elements, elements_size);
            arg_3(arguments)->value.arrayValue = NULL;
          } else {
            array_exec_object = curr_exec_object;
            /* It is possible that arr1 == elements holds. */
            /* In this case the new hole in arr1 must be   */
            /* considered.                                   */
            if (unlikely(arr1 == elements)) {
              if (unlikely(!crea_array(&array_pointer[arrayIndex(resized_arr1, position)],
                                       array_pointer, arrayIndex(resized_arr1, position)))) {
                logError(printf("arr_insert_array: crea_array() failed.\n"););
                restore = TRUE;
              } else {
                if (unlikely(!crea_array(&array_pointer[2 * arrayIndex(resized_arr1, position)],
                                         &array_pointer[arrayIndex(resized_arr1, position) + elements_size],
                                         elements_size - arrayIndex(resized_arr1, position)))) {
                  logError(printf("arr_insert_array: crea_array() failed.\n"););
                  destr_array(&array_pointer[arrayIndex(resized_arr1, position)],
                              arrayIndex(resized_arr1, position));
                  restore = TRUE;
                } /* if */
              } /* if */
            } else if (unlikely(!crea_array(&array_pointer[arrayIndex(resized_arr1, position)],
                                            elements->arr, elements_size))) {
              logError(printf("arr_insert_array: crea_array() failed.\n"););
              restore = TRUE;
            } /* if */
            if (unlikely(restore)) {
              memmove(&array_pointer[arrayIndex(resized_arr1, position)],
                      &array_pointer[arrayIndex(resized_arr1, position) + elements_size],
                      arraySize2(position, resized_arr1->max_position) * sizeof(objectRecord));
              if (unlikely(!REALLOC_ARRAY(arr1, resized_arr1, arr1_size))) {
                logError(printf("arr_insert_array: REALLOC_ARRAY() failed.\n"););
                return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                               array_exec_object,
                                               arguments);
              } else {
                COUNT3_ARRAY(new_size, arr1_size);
                arr_variable->value.arrayValue = arr1;
                return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                               array_exec_object,
                                               arguments);
              } /* if */
            } else {
              resized_arr1->max_position = arrayMaxPos(resized_arr1->min_position, new_size);
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("arr_insert_array -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* arr_insert_array */



/**
 *  Determine the length of the array 'arr'.
 *  @return the length of the array.
 */
objectType arr_lng (listType arguments)

  {
    arrayType arr1;

  /* arr_lng */
    isit_array(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    return bld_int_temp(arr1->max_position - arr1->min_position + 1);
  } /* arr_lng */



/**
 *  Maximum index of array 'arr'.
 *  @return the maximum index of the array.
 */
objectType arr_maxidx (listType arguments)

  {
    arrayType arr1;

  /* arr_maxidx */
    isit_array(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    return bld_int_temp(arr1->max_position);
  } /* arr_maxidx */



/**
 *  Minimum index of array 'arr'.
 *  @return the minimum index of the array.
 */
objectType arr_minidx (listType arguments)

  {
    arrayType arr1;

  /* arr_minidx */
    isit_array(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    return bld_int_temp(arr1->min_position);
  } /* arr_minidx */



/**
 *  Append the given 'element' to the array 'arr_variable'.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             array.
 */
objectType arr_push (listType arguments)

  {
    objectType arr_variable;
    arrayType dest;
    objectType element;
    typeType element_type;
    arrayType new_arr;
    memSizeType new_size;
    memSizeType dest_size;
    objectType array_exec_object;

  /* arr_push */
    logFunction(printf("arr_push\n"););
    arr_variable = arg_1(arguments);
    isit_array(arr_variable);
    is_variable(arr_variable);
    dest = take_array(arr_variable);
    element = arg_3(arguments);
    dest_size = arraySize(dest);
    if (unlikely(dest_size > MAX_ARR_LEN - 1 ||
                 dest->max_position > (intType) (MAX_MEM_INDEX - 1))) {
      logError(printf("arr_push: Result size bigger than MAX_ARR_LEN.\n"););
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      new_size = dest_size + 1;
      if (unlikely(!REALLOC_ARRAY(new_arr, dest, new_size))) {
        logError(printf("arr_push: REALLOC_ARRAY() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        COUNT3_ARRAY(dest_size, new_size);
        arr_variable->value.arrayValue = new_arr;
        /* The element type is the type of the 3rd formal parameter */
        element_type = curr_exec_object->value.listValue->obj->
                       descriptor.property->params->next->next->obj->type_of;
        if (TEMP_OBJECT(element) && element->type_of == element_type) {
          CLEAR_TEMP_FLAG(element);
          SET_VAR_FLAG(element);
          SET_EMBEDDED_FLAG(element);
          memcpy(&new_arr->arr[dest_size], element, sizeof(objectRecord));
          new_arr->max_position ++;
          FREE_OBJECT(element);
          arg_3(arguments) = NULL;
        } else {
          array_exec_object = curr_exec_object;
          if (unlikely(!arr_elem_initialisation(element_type,
                                                &new_arr->arr[dest_size],
                                                element))) {
            logError(printf("arr_push: arr_elem_initialisation() failed.\n"););
            if (unlikely(!REALLOC_ARRAY(dest, new_arr, dest_size))) {
              logError(printf("arr_push: REALLOC_ARRAY() failed.\n"););
              return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                             array_exec_object,
                                             arguments);
            } else {
              COUNT3_ARRAY(new_size, dest_size);
              arr_variable->value.arrayValue = dest;
              return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                             array_exec_object,
                                             arguments);
            } /* if */
          } else {
            new_arr->max_position ++;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("arr_push -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* arr_push */



/**
 *  Get a sub array from the position 'start' to the position 'stop'.
 *  @return the sub array from position 'start' to 'stop'.
 *  @exception INDEX_ERROR The start position is less than minIdx(arr1), or
 *                         the stop position is less than pred(start).
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType arr_range (listType arguments)

  {
    arrayType arr1;
    intType start;
    intType stop;
    memSizeType arr1_size;
    memSizeType result_size;
    memSizeType start_idx;
    memSizeType stop_idx;
    objectType array_exec_object;
    arrayType result;

  /* arr_range */
    isit_array(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_5(arguments));
    arr1 = take_array(arg_1(arguments));
    start = take_int(arg_3(arguments));
    stop = take_int(arg_5(arguments));
    arr1_size = arraySize(arr1);
    if (unlikely(start < arr1->min_position)) {
      logError(printf("arr_range(arr1, " FMT_D ", " FMT_D "): "
                      "Start index out of range (" FMT_D " .. " FMT_D ").\n",
                      start, stop, arr1->min_position, arr1->max_position + 1););
      return raise_exception(SYS_IDX_EXCEPTION);
    } else if (stop >= start && start <= arr1->max_position && arr1_size >= 1) {
      if (stop > arr1->max_position) {
        stop = arr1->max_position;
      } /* if */
      result_size = arraySize2(start, stop);
      if (unlikely(!ALLOC_ARRAY(result, result_size))) {
        logError(printf("arr_range: ALLOC_ARRAY() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arrayMaxPos(arr1->min_position, result_size);
        start_idx = arrayIndex(arr1, start);
        stop_idx = arrayIndex(arr1, stop);
        if (TEMP_OBJECT(arg_1(arguments))) {
          memcpy(result->arr, &arr1->arr[start_idx],
                 (size_t) (result_size * sizeof(objectRecord)));
          destr_array(arr1->arr, start_idx);
          destr_array(&arr1->arr[stop_idx + 1], arr1_size - stop_idx - 1);
          FREE_ARRAY(arr1, arr1_size);
          arg_1(arguments)->value.arrayValue = NULL;
        } else {
          array_exec_object = curr_exec_object;
          if (unlikely(!crea_array(result->arr,
                                   &arr1->arr[start_idx], result_size))) {
            logError(printf("arr_range: crea_array() failed.\n"););
            FREE_ARRAY(result, result_size);
            return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                           array_exec_object,
                                           arguments);
          } /* if */
        } /* if */
      } /* if */
    } else if (unlikely(stop < start - 1)) {
      logError(printf("arr_range(arr1, " FMT_D ", " FMT_D "): "
                      "Stop index less than start index minus one.\n",
                      start, stop););
      return raise_exception(SYS_IDX_EXCEPTION);
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arr_range(arr1 (size=" FMT_U_MEM "), " FMT_D ", " FMT_D "): "
                      "Cannot create empty array with minimum index.\n",
                      arr1_size, start, stop););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      if (unlikely(!ALLOC_ARRAY(result, 0))) {
        logError(printf("arr_range: ALLOC_ARRAY() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_range */



/**
 *  Remove the element with 'position' from 'arr1' and return the removed element.
 *  @return the removed element.
 *  @exception INDEX_ERROR If 'position' is less than arr_minidx(arr2) or
 *                         greater than arr_maxidx(arr2)
 */
objectType arr_remove (listType arguments)

  {
    arrayType arr1;
    arrayType resized_arr1;
    intType position;
    objectType array_pointer;
    memSizeType arr1_size;
    objectType result;

  /* arr_remove */
    logFunction(printf("arr_remove\n"););
    isit_array(arg_1(arguments));
    isit_int(arg_2(arguments));
    is_variable(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    position = take_int(arg_2(arguments));
    if (unlikely(position < arr1->min_position ||
                 position > arr1->max_position)) {
      logError(printf("arr_remove(arr1, " FMT_D "): "
                      "Index out of range (" FMT_D " .. " FMT_D ").\n",
                      position, arr1->min_position, arr1->max_position););
      result = raise_exception(SYS_IDX_EXCEPTION);
    } else {
      array_pointer = arr1->arr;
      if (unlikely(!ALLOC_OBJECT(result))) {
        logError(printf("arr_remove: ALLOC_OBJECT() failed.\n"););
        result = raise_exception(SYS_MEM_EXCEPTION);
      } else {
        memcpy(result, &array_pointer[position - arr1->min_position], sizeof(objectRecord));
        memmove(&array_pointer[position - arr1->min_position],
                &array_pointer[position - arr1->min_position + 1],
                (arraySize2(position, arr1->max_position) - 1) * sizeof(objectRecord));
        arr1_size = arraySize(arr1);
        if (unlikely(!REALLOC_ARRAY(resized_arr1, arr1, arr1_size - 1))) {
          /* A realloc, which shrinks memory, usually succeeds. */
          /* The probability that this code path is executed is */
          /* probably zero. The code below restores the old     */
          /* value of arr1.                                     */
          logError(printf("arr_remove: REALLOC_ARRAY() failed.\n"););
          memmove(&array_pointer[position - arr1->min_position + 1],
                  &array_pointer[position - arr1->min_position],
                  (arraySize2(position, arr1->max_position) - 1) * sizeof(objectRecord));
          memcpy(&array_pointer[position - arr1->min_position], result, sizeof(objectRecord));
          FREE_OBJECT(result);
          result = raise_exception(SYS_MEM_EXCEPTION);
        } else {
          arr1 = resized_arr1;
          COUNT3_ARRAY(arr1_size, arr1_size - 1);
          arr1->max_position--;
          arg_1(arguments)->value.arrayValue = arr1;
          SET_TEMP_FLAG(result);
          CLEAR_EMBEDDED_FLAG(result);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("arr_remove --> " F_U_MEM(08) " ", (memSizeType) result);
                trace1(result);
                printf("\n"););
    return result;
  } /* arr_remove */



/**
 *  Remove the sub-array with 'position' and 'length' from 'arr1'.
 *  @return the removed sub-array.
 *  @exception INDEX_ERROR If 'position' is less than arr_minidx(arr2) or
 *                         greater than arr_maxidx(arr2)
 */
objectType arr_remove_array (listType arguments)

  {
    arrayType arr1;
    arrayType resized_arr1;
    intType position;
    intType length;
    objectType array_pointer;
    memSizeType arr1_size;
    memSizeType result_size;
    arrayType result;

  /* arr_remove_array */
    logFunction(printf("arr_remove_array\n"););
    isit_array(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    is_variable(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    position = take_int(arg_2(arguments));
    length = take_int(arg_3(arguments));
    if (unlikely(length < 0)) {
      logError(printf("arr_remove_array(arr1, " FMT_D ", " FMT_D "): "
                      "Length is negative.\n", position, length););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else if (unlikely(position < arr1->min_position ||
                        position > arr1->max_position)) {
      logError(printf("arr_remove_array(arr1, " FMT_D "): "
                      "Index out of range (" FMT_D " .. " FMT_D ").\n",
                      position, arr1->min_position, arr1->max_position););
      return raise_exception(SYS_IDX_EXCEPTION);
    } else {
      if ((uintType) length > MAX_ARR_LEN) {
        result_size = MAX_ARR_LEN;
      } else {
        result_size = (memSizeType) (uintType) (length);
      } /* if */
      if (result_size > arraySize2(position, arr1->max_position)) {
        result_size = arraySize2(position, arr1->max_position);
      } /* if */
      if (unlikely(!ALLOC_ARRAY(result, result_size))) {
        logError(printf("arr_remove_array: ALLOC_ARRAY() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arrayMaxPos(arr1->min_position, result_size);
        array_pointer = arr1->arr;
        memcpy(result->arr, &array_pointer[arrayIndex(arr1, position)],
               result_size * sizeof(objectRecord));
        memmove(&array_pointer[arrayIndex(arr1, position)],
                &array_pointer[arrayIndex(arr1, position) + result_size],
                (arraySize2(position, arr1->max_position) - result_size) * sizeof(objectRecord));
        arr1_size = arraySize(arr1);
        if (unlikely(!REALLOC_ARRAY(resized_arr1, arr1, arr1_size - result_size))) {
          /* A realloc, which shrinks memory, usually succeeds. */
          /* The probability that this code path is executed is */
          /* probably zero. The code below restores the old     */
          /* value of arr1.                                     */
          logError(printf("arr_remove_array: REALLOC_ARRAY() failed.\n"););
          memmove(&array_pointer[arrayIndex(arr1, position) + result_size],
                  &array_pointer[arrayIndex(arr1, position)],
                  (arraySize2(position, arr1->max_position) - result_size) * sizeof(objectRecord));
          memcpy(&array_pointer[arrayIndex(arr1, position)], result->arr,
                 result_size * sizeof(objectRecord));
          FREE_ARRAY(result, result_size);
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          arr1 = resized_arr1;
          COUNT3_ARRAY(arr1_size, arr1_size - result_size);
          arr1->max_position = arrayMaxPos(arr1->min_position, arr1_size - result_size);
          arg_1(arguments)->value.arrayValue = arr1;
        } /* if */
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_remove_array */



objectType arr_sort (listType arguments)

  {
    objectType arr_arg;
    objectType data_cmp_func;
    arrayType arr1;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result;

  /* arr_sort */
    arr_arg = arg_1(arguments);
    isit_array(arr_arg);
    data_cmp_func    = take_reference(arg_2(arguments));
    if (TEMP2_OBJECT(arr_arg)) {
      result = take_array(arr_arg);
      arr_arg->value.arrayValue = NULL;
    } else {
      arr1 = take_array(arr_arg);
      result_size = arraySize(arr1);
      if (unlikely(!ALLOC_ARRAY(result, result_size))) {
        logError(printf("arr_sort: ALLOC_ARRAY() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        array_exec_object = curr_exec_object;
        result->min_position = arr1->min_position;
        result->max_position = arr1->max_position;
        if (unlikely(!crea_array(result->arr, arr1->arr, result_size))) {
          logError(printf("arr_sort: crea_array() failed.\n"););
          FREE_ARRAY(result, result_size);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         array_exec_object,
                                         arguments);
        } /* if */
      } /* if */
    } /* if */
    qsort_array(result->arr,
        &result->arr[result->max_position - result->min_position],
        data_cmp_func);
    return bld_array_temp(result);
  } /* arr_sort */



objectType arr_sort_reverse (listType arguments)

  {
    objectType arr_arg;
    objectType data_cmp_func;
    arrayType arr1;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result;

  /* arr_sort_reverse */
    arr_arg = arg_1(arguments);
    isit_array(arr_arg);
    data_cmp_func    = take_reference(arg_2(arguments));
    if (TEMP2_OBJECT(arr_arg)) {
      result = take_array(arr_arg);
      arr_arg->value.arrayValue = NULL;
    } else {
      arr1 = take_array(arr_arg);
      result_size = arraySize(arr1);
      if (unlikely(!ALLOC_ARRAY(result, result_size))) {
        logError(printf("arr_sort_reverse: ALLOC_ARRAY() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        array_exec_object = curr_exec_object;
        result->min_position = arr1->min_position;
        result->max_position = arr1->max_position;
        if (unlikely(!crea_array(result->arr, arr1->arr, result_size))) {
          logError(printf("arr_sort_reverse: crea_array() failed.\n"););
          FREE_ARRAY(result, result_size);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         array_exec_object,
                                         arguments);
        } /* if */
      } /* if */
    } /* if */
    qsort_array_reverse(result->arr,
        &result->arr[result->max_position - result->min_position],
        data_cmp_func);
    return bld_array_temp(result);
  } /* arr_sort_reverse */



/**
 *  Get a sub array from the position 'start' with maximum length 'length'.
 *  @return the sub array from position 'start' with maximum length 'length'.
 *  @exception INDEX_ERROR The start position is less than minIdx(arr1), or
 *                         the length is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType arr_subarr (listType arguments)

  {
    arrayType arr1;
    intType start;
    intType length;
    memSizeType arr1_size;
    memSizeType result_size;
    memSizeType start_idx;
    memSizeType stop_idx;
    objectType array_exec_object;
    arrayType result;

  /* arr_subarr */
    isit_array(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_5(arguments));
    arr1 = take_array(arg_1(arguments));
    start = take_int(arg_3(arguments));
    length = take_int(arg_5(arguments));
    if (unlikely(start < arr1->min_position || length < 0)) {
      logError(printf("arr_subarr(arr1, " FMT_D ", " FMT_D "): "
                      "Start index out of range (" FMT_D " .. " FMT_D ").\n",
                      start, length, arr1->min_position, arr1->max_position + 1););
      return raise_exception(SYS_IDX_EXCEPTION);
    } else {
      arr1_size = arraySize(arr1);
      if (length != 0 && start <= arr1->max_position && arr1_size >= 1) {
        if (length - 1 > arr1->max_position - start) {
          length = arr1->max_position - start + 1;
        } /* if */
        result_size = (memSizeType) (uintType) (length);
        if (unlikely(!ALLOC_ARRAY(result, result_size))) {
          logError(printf("arr_subarr: ALLOC_ARRAY() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          result->min_position = arr1->min_position;
          result->max_position = arrayMaxPos(arr1->min_position, result_size);
          start_idx = arrayIndex(arr1, start);
          stop_idx = arrayIndex(arr1, start + length - 1);
          if (TEMP_OBJECT(arg_1(arguments))) {
            memcpy(result->arr, &arr1->arr[start_idx],
                   (size_t) (result_size * sizeof(objectRecord)));
            destr_array(arr1->arr, start_idx);
            destr_array(&arr1->arr[stop_idx + 1], arr1_size - stop_idx - 1);
            FREE_ARRAY(arr1, arr1_size);
            arg_1(arguments)->value.arrayValue = NULL;
          } else {
            array_exec_object = curr_exec_object;
            if (unlikely(!crea_array(result->arr,
                                     &arr1->arr[start_idx],
                                     result_size))) {
              logError(printf("arr_subarr: crea_array() failed.\n"););
              FREE_ARRAY(result, result_size);
              return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                             array_exec_object,
                                             arguments);
            } /* if */
          } /* if */
        } /* if */
      } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
        logError(printf("arr_subarr(arr1 (size=" FMT_U_MEM "), " FMT_D ", " FMT_D "): "
                        "Cannot create empty array with minimum index.\n",
                        arr1_size, start, length););
        return raise_exception(SYS_RNG_EXCEPTION);
      } else {
        if (unlikely(!ALLOC_ARRAY(result, 0))) {
          logError(printf("arr_subarr: ALLOC_ARRAY() failed.\n"););
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          result->min_position = arr1->min_position;
          result->max_position = arr1->min_position - 1;
        } /* if */
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_subarr */



/**
 *  Get a sub array beginning at the position 'start'.
 *  @return the sub array beginning at the start position.
 *  @exception INDEX_ERROR The start position is less than minIdx(arr1).
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType arr_tail (listType arguments)

  {
    arrayType arr1;
    intType start;
    memSizeType arr1_size;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result;

  /* arr_tail */
    isit_array(arg_1(arguments));
    isit_int(arg_3(arguments));
    arr1 = take_array(arg_1(arguments));
    start = take_int(arg_3(arguments));
    arr1_size = arraySize(arr1);
    if (unlikely(start < arr1->min_position)) {
      logError(printf("arr_tail(arr1, " FMT_D "): "
                      "Start index out of range (" FMT_D " .. " FMT_D ").\n",
                      start, arr1->min_position, arr1->max_position + 1););
      return raise_exception(SYS_IDX_EXCEPTION);
    } else if (start <= arr1->max_position && arr1_size >= 1) {
      result_size = arraySize2(start, arr1->max_position);
      if (unlikely(!ALLOC_ARRAY(result, result_size))) {
        logError(printf("arr_tail: ALLOC_ARRAY() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arrayMaxPos(arr1->min_position, result_size);
        if (TEMP_OBJECT(arg_1(arguments))) {
          memcpy(result->arr, &arr1->arr[start - arr1->min_position],
                 (size_t) (result_size * sizeof(objectRecord)));
          destr_array(arr1->arr, arraySize2(arr1->min_position, start) - 1);
          FREE_ARRAY(arr1, arr1_size);
          arg_1(arguments)->value.arrayValue = NULL;
          /* code to avoid destr_array:
          arr1->max_position = start - 1; */
        } else {
          array_exec_object = curr_exec_object;
          if (unlikely(!crea_array(result->arr,
                                   &arr1->arr[start - arr1->min_position],
                                   result_size))) {
            logError(printf("arr_tail: crea_array() failed.\n"););
            FREE_ARRAY(result, result_size);
            return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                           array_exec_object,
                                           arguments);
          } /* if */
        } /* if */
      } /* if */
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arr_tail(arr1 (size=" FMT_U_MEM "), " FMT_D "): "
                      "Cannot create empty array with minimum index.\n",
                      arr1_size, start););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      if (unlikely(!ALLOC_ARRAY(result, 0))) {
        logError(printf("arr_tail: ALLOC_ARRAY() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_tail */



/**
 *  Generate an array by using 'factor' 'elements'.
 *  @return an array with 'factor' 'elements'.
 *  @exception RANGE_ERROR If 'factor' is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType arr_times (listType arguments)

  {
    intType factor;
    objectType element;
    memSizeType position;
    objectType elem_to;
    typeType element_type;
    objectType array_exec_object;
    memSizeType result_size;
    arrayType result;

  /* arr_times */
    isit_int(arg_1(arguments));
    factor = take_int(arg_1(arguments));
    element = arg_3(arguments);
    if (unlikely(factor < 0)) {
      logError(printf("arr_times(" FMT_D ", ...): Factor negative.\n",
                      factor););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else if (unlikely((uintType) factor > MAX_ARR_LEN ||
                        (uintType) factor > MAX_MEM_INDEX)) {
      logError(printf("arr_times: Result size bigger than MAX_ARR_LEN.\n"););
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result_size = (memSizeType) (uintType) factor;
      if (unlikely(!ALLOC_ARRAY(result, result_size))) {
        logError(printf("arr_times: ALLOC_ARRAY() failed.\n"););
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->min_position = 1;
        result->max_position = factor;
        elem_to = result->arr;
        if (result_size > 0) {
          array_exec_object = curr_exec_object;
          /* The element type is the type of the 3rd formal parameter */
          element_type = curr_exec_object->value.listValue->obj->
                         descriptor.property->params->next->next->obj->type_of;
          if (TEMP_OBJECT(element) && element->type_of == element_type) {
            CLEAR_TEMP_FLAG(element);
            SET_VAR_FLAG(element);
            SET_EMBEDDED_FLAG(element);
            memcpy(elem_to, element, sizeof(objectRecord));
            FREE_OBJECT(element);
            arg_3(arguments) = NULL;
          } else {
            if (unlikely(!arr_elem_initialisation(element_type,
                                                  elem_to, element))) {
              logError(printf("arr_times: arr_elem_initialisation() failed.\n"););
              FREE_ARRAY(result, result_size);
              return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                             array_exec_object,
                                             arguments);
            } /* if */
          } /* if */
          position = 1;
          while (position < result_size) {
            if (unlikely(!arr_elem_initialisation(element_type,
                                                  &elem_to[position], elem_to))) {
              /* If the creation of an element fails (mostly no memory) */
              /* all elements created up to this point must be destroyed */
              /* to recycle the memory correctly. */
              logError(printf("arr_times: arr_elem_initialisation() failed.\n"););
              destr_array(elem_to, position);
              FREE_ARRAY(result, result_size);
              return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                             array_exec_object,
                                             arguments);
            } else {
              position++;
            } /* if */
          } /* for */
        } /* if */
        return bld_array_temp(result);
      } /* if */
    } /* if */
  } /* arr_times */
