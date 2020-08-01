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
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_size = arr_to_size + extension_size;
        new_arr = REALLOC_ARRAY(arr_to, arr_to_size, new_size);
        if (unlikely(new_arr == NULL)) {
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
            /* It is possible that arr_to == extension holds. */
            /* In this case 'extension' must be corrected     */
            /* after realloc() enlarged 'arr_to'.             */
            if (arr_to == extension) {
              extension = new_arr;
            } /* if */
            if (unlikely(!crea_array(&new_arr->arr[arr_to_size], extension->arr,
                                     extension_size))) {
              arr_to = REALLOC_ARRAY(new_arr, new_size, arr_to_size);
              if (unlikely(arr_to == NULL)) {
                return raise_exception(SYS_MEM_EXCEPTION);
              } /* if */
              COUNT3_ARRAY(new_size, arr_to_size);
              arr_variable->value.arrayValue = arr_to;
              return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
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
                        "Maximal index out of range.\n",
                        result_size););
        return raise_exception(SYS_RNG_EXCEPTION);
      } else if (unlikely(!ALLOC_ARRAY(result_array, result_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result_array->min_position = 1;
        result_array->max_position = (intType) result_size;
        if (unlikely(!crea_array(result_array->arr, arr1->arr, result_size))) {
          FREE_ARRAY(result_array, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
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
                      "Minimal or maximal index out of range.\n",
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
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result_array->min_position = start_position;
        result_array->max_position = arrayMaxPos(start_position, result_size);
        if (unlikely(!crea_array(result_array->arr, arr1->arr, result_size))) {
          FREE_ARRAY(result_array, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
        result = bld_array_temp(result_array);
      } /* if */
    } /* if */
    logFunction(printf("arr_arrlit2 -->\n"););
    return result;
  } /* arr_arrlit2 */



objectType arr_baselit (listType arguments)

  {
    objectType element;
    typeType result_element_type;
    memSizeType result_size;
    arrayType result;

  /* arr_baselit */
    element = arg_3(arguments);
    result_size = 1;
    if (unlikely(!ALLOC_ARRAY(result, result_size))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    result->min_position = 1;
    result->max_position = 1;
    /* The element type of the result is the type of the 3rd formal parameter */
    result_element_type = curr_exec_object->value.listValue->obj->
                          descriptor.property->params->next->next->obj->type_of;
    if (TEMP_OBJECT(element) && element->type_of == result_element_type) {
      CLEAR_TEMP_FLAG(element);
      SET_VAR_FLAG(element);
      memcpy(&result->arr[0], element, sizeof(objectRecord));
      FREE_OBJECT(element);
      arg_3(arguments) = NULL;
    } else {
      if (unlikely(!arr_elem_initialisation(result_element_type,
                                            &result->arr[0], element))) {
        FREE_ARRAY(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_baselit */



objectType arr_baselit2 (listType arguments)

  {
    intType start_position;
    objectType element;
    typeType result_element_type;
    memSizeType result_size;
    arrayType result;

  /* arr_baselit2 */
    isit_int(arg_2(arguments));
    start_position = take_int(arg_2(arguments));
    element = arg_4(arguments);
    result_size = 1;
    if (unlikely(!ALLOC_ARRAY(result, result_size))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    result->min_position = start_position;
    result->max_position = start_position;
    /* The element type of the result is the type of the 4th formal parameter */
    result_element_type = curr_exec_object->value.listValue->obj->
                          descriptor.property->params->next->next->next->obj->type_of;
    if (TEMP_OBJECT(element) && element->type_of == result_element_type) {
      CLEAR_TEMP_FLAG(element);
      SET_VAR_FLAG(element);
      memcpy(&result->arr[0], element, sizeof(objectRecord));
      FREE_OBJECT(element);
      arg_4(arguments) = NULL;
    } else {
      if (unlikely(!arr_elem_initialisation(result_element_type,
                                            &result->arr[0], element))) {
        FREE_ARRAY(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
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
    memSizeType result_size;
    arrayType result;

  /* arr_cat */
    isit_array(arg_1(arguments));
    isit_array(arg_3(arguments));
    arr1 = take_array(arg_1(arguments));
    arr2 = take_array(arg_3(arguments));
    arr1_size = arraySize(arr1);
    arr2_size = arraySize(arr2);
    if (unlikely(arr1_size > MAX_ARR_LEN - arr2_size ||
                 arr1->max_position > (intType) (MAX_MEM_INDEX - arr2_size))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result_size = arr1_size + arr2_size;
      if (TEMP_OBJECT(arg_1(arguments))) {
        result = arr1;
        result = REALLOC_ARRAY(result, arr1_size, result_size);
        if (unlikely(result == NULL)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        COUNT3_ARRAY(arr1_size, result_size);
        result->max_position = arrayMaxPos(result->min_position, result_size);
        arg_1(arguments)->value.arrayValue = NULL;
      } else {
        if (unlikely(!ALLOC_ARRAY(result, result_size))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->min_position = arr1->min_position;
        result->max_position = arrayMaxPos(result->min_position, result_size);
        if (unlikely(!crea_array(result->arr, arr1->arr, arr1_size))) {
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
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
          destr_array(result->arr, arr1_size);
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_cat */



objectType arr_conv (listType arguments)

  {
    objectType arr_arg;
    arrayType arr1;
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
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arr1->max_position;
      if (unlikely(!crea_array(result->arr, arr1->arr, result_size))) {
        FREE_ARRAY(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
      return bld_array_temp(result);
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
    memSizeType arr_dest_size;
    memSizeType arr_source_size;
    arrayType new_arr;

  /* arr_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_array(dest);
    isit_array(source);
    is_variable(dest);
    arr_dest = take_array(dest);
    arr_source = take_array(source);
    if (TEMP_OBJECT(source)) {
      arr_dest_size = arraySize(arr_dest);
      destr_array(arr_dest->arr, arr_dest_size);
      FREE_ARRAY(arr_dest, arr_dest_size);
      dest->value.arrayValue = arr_source;
      source->value.arrayValue = NULL;
    } else {
      arr_source_size = arraySize(arr_source);
      if (arr_dest->min_position != arr_source->min_position ||
          arr_dest->max_position != arr_source->max_position) {
        if (unlikely(!ALLOC_ARRAY(new_arr, arr_source_size))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          new_arr->min_position = arr_source->min_position;
          new_arr->max_position = arr_source->max_position;
          if (unlikely(!crea_array(new_arr->arr,
                                   arr_source->arr, arr_source_size))) {
            FREE_ARRAY(new_arr, arr_source_size);
            return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
          } /* if */
          arr_dest_size = arraySize(arr_dest);
          destr_array(arr_dest->arr, arr_dest_size);
          FREE_ARRAY(arr_dest, arr_dest_size);
          dest->value.arrayValue = new_arr;
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
        dest->value.arrayValue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_arr->min_position = arr_source->min_position;
        new_arr->max_position = arr_source->max_position;
        if (unlikely(!crea_array(new_arr->arr,
                                 arr_source->arr, new_size))) {
          FREE_ARRAY(new_arr, new_size);
          dest->value.arrayValue = NULL;
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
        dest->value.arrayValue = new_arr;
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
    memSizeType old_size;

  /* arr_destr */
    isit_array(arg_1(arguments));
    old_arr = take_array(arg_1(arguments));
    if (old_arr != NULL) {
      old_size = arraySize(old_arr);
      destr_array(old_arr->arr, old_size);
      FREE_ARRAY(old_arr, old_size);
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
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      /* Note that the size of the allocated memory is smaller, than the */
      /* struct. But this is okay, because the element 'arr' is not used. */
      result->min_position = 1;
      result->max_position = 0;
    } /* if */
    return bld_array_temp(result);
  } /* arr_empty */



objectType arr_extend (listType arguments)

  {
    arrayType arr1;
    objectType element;
    typeType result_element_type;
    memSizeType arr1_size;
    memSizeType result_size;
    arrayType result;

  /* arr_extend */
    isit_array(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    element = arg_3(arguments);
    arr1_size = arraySize(arr1);
    if (unlikely(arr1_size > MAX_ARR_LEN - 1 ||
                 arr1->max_position > (intType) (MAX_MEM_INDEX - 1))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result_size = arr1_size + 1;
      if (TEMP_OBJECT(arg_1(arguments))) {
        result = arr1;
        result = REALLOC_ARRAY(result, arr1_size, result_size);
        if (unlikely(result == NULL)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        COUNT3_ARRAY(arr1_size, result_size);
        result->max_position++;
        arg_1(arguments)->value.arrayValue = NULL;
      } else {
        if (unlikely(!ALLOC_ARRAY(result, result_size))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->min_position = arr1->min_position;
        result->max_position = arr1->max_position + 1;
        if (unlikely(!crea_array(result->arr, arr1->arr, arr1_size))) {
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
      /* The element type of the result is the type of the 3rd formal parameter */
      result_element_type = curr_exec_object->value.listValue->obj->
                            descriptor.property->params->next->next->obj->type_of;
      if (TEMP_OBJECT(element) && element->type_of == result_element_type) {
        CLEAR_TEMP_FLAG(element);
        SET_VAR_FLAG(element);
        memcpy(&result->arr[arr1_size], element, sizeof(objectRecord));
        FREE_OBJECT(element);
        arg_3(arguments) = NULL;
      } else {
        if (unlikely(!arr_elem_initialisation(result_element_type,
                                              &result->arr[arr1_size], element))) {
          destr_array(result->arr, arr1_size);
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_extend */



objectType arr_gen (listType arguments)

  {
    objectType element1;
    objectType element2;
    typeType result_element_type;
    memSizeType result_size;
    arrayType result;

  /* arr_gen */
    element1 = arg_1(arguments);
    element2 = arg_3(arguments);
    result_size = 2;
    if (unlikely(!ALLOC_ARRAY(result, result_size))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    result->min_position = 1;
    result->max_position = 2;
    /* The element type of the result is the type of the 1st formal parameter */
    result_element_type = curr_exec_object->value.listValue->obj->
                          descriptor.property->params->obj->type_of;
    if (TEMP_OBJECT(element1) && element1->type_of == result_element_type) {
      CLEAR_TEMP_FLAG(element1);
      SET_VAR_FLAG(element1);
      memcpy(&result->arr[0], element1, sizeof(objectRecord));
      FREE_OBJECT(element1);
      arg_1(arguments) = NULL;
    } else {
      if (unlikely(!arr_elem_initialisation(result_element_type,
                                            &result->arr[0], element1))) {
        FREE_ARRAY(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    if (TEMP_OBJECT(element2) && element2->type_of == result_element_type) {
      CLEAR_TEMP_FLAG(element2);
      SET_VAR_FLAG(element2);
      memcpy(&result->arr[1], element2, sizeof(objectRecord));
      FREE_OBJECT(element2);
      arg_3(arguments) = NULL;
    } else {
      if (unlikely(!arr_elem_initialisation(result_element_type,
                                            &result->arr[1], element2))) {
        destr_array(result->arr, 1);
        FREE_ARRAY(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_gen */



/**
 *  Get a sub array ending at the position 'stop'.
 *  @return the sub array ending at the stop position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType arr_head (listType arguments)

  {
    arrayType arr1;
    intType stop;
    memSizeType length;
    memSizeType result_size;
    arrayType resized_result;
    arrayType result;

  /* arr_head */
    isit_array(arg_1(arguments));
    isit_int(arg_4(arguments));
    arr1 = take_array(arg_1(arguments));
    stop = take_int(arg_4(arguments));
    length = arraySize(arr1);
    if (stop >= arr1->min_position && length >= 1) {
      if (stop > arr1->max_position) {
        stop = arr1->max_position;
      } /* if */
      result_size = arraySize2(arr1->min_position, stop);
      if (TEMP_OBJECT(arg_1(arguments))) {
        result = arr1;
        arg_1(arguments)->value.arrayValue = NULL;
        destr_array(&result->arr[result_size], length - result_size);
        resized_result = REALLOC_ARRAY(result, length, result_size);
        if (unlikely(resized_result == NULL)) {
          destr_array(result->arr, result_size);
          FREE_ARRAY(result, length);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
        result = resized_result;
        COUNT3_ARRAY(length, result_size);
        result->max_position = stop;
      } else {
        if (unlikely(!ALLOC_ARRAY(result, result_size))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->min_position = arr1->min_position;
        result->max_position = stop;
        if (unlikely(!crea_array(result->arr, arr1->arr, result_size))) {
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arr_head(arr1 (size=" FMT_U_MEM "), " FMT_D "): "
                      "Cannot create empty array with minimal index.\n",
                      length, stop););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      if (unlikely(!ALLOC_ARRAY(result, 0))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arr1->min_position - 1;
    } /* if */
    return bld_array_temp(result);
  } /* arr_head */



/**
 *  Access one element from the array 'arr'.
 *  @return the element with the specified 'position' from 'arr'.
 *  @exception RANGE_ERROR If 'position' is less than arr_minidx(arr) or
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
      result = raise_exception(SYS_RNG_EXCEPTION);
    } else {
      array_pointer = arr1->arr;
      if (TEMP_OBJECT(arg_1(arguments))) {
        /* The array will be destroyed after indexing. */
        /* A copy is necessary here to avoid a crash !!!!! */
        if (unlikely(!ALLOC_OBJECT(result))) {
          result = raise_exception(SYS_MEM_EXCEPTION);
        } else {
          memcpy(result, &array_pointer[position - arr1->min_position], sizeof(objectRecord));
          SET_TEMP_FLAG(result);
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
 *  Maximal index of array 'arr'.
 *  @return the maximal index of the array.
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
 *  Minimal index of array 'arr'.
 *  @return the minimal index of the array.
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
    typeType result_element_type;
    arrayType new_arr;
    memSizeType new_size;
    memSizeType dest_size;

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
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      new_size = dest_size + 1;
      new_arr = REALLOC_ARRAY(dest, dest_size, new_size);
      if (unlikely(new_arr == NULL)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        COUNT3_ARRAY(dest_size, new_size);
        arr_variable->value.arrayValue = new_arr;
        /* The element type of the result is the type of the 3rd formal parameter */
        result_element_type = curr_exec_object->value.listValue->obj->
                              descriptor.property->params->next->next->obj->type_of;
        if (TEMP_OBJECT(element) && element->type_of == result_element_type) {
          CLEAR_TEMP_FLAG(element);
          SET_VAR_FLAG(element);
          memcpy(&new_arr->arr[dest_size], element, sizeof(objectRecord));
          new_arr->max_position ++;
          FREE_OBJECT(element);
          arg_3(arguments) = NULL;
        } else {
          if (unlikely(!arr_elem_initialisation(result_element_type,
                                                &new_arr->arr[dest_size], element))) {
            dest = REALLOC_ARRAY(new_arr, new_size, dest_size);
            if (unlikely(dest == NULL)) {
              return raise_exception(SYS_MEM_EXCEPTION);
            } /* if */
            COUNT3_ARRAY(new_size, dest_size);
            arr_variable->value.arrayValue = dest;
            return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
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
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType arr_range (listType arguments)

  {
    arrayType arr1;
    intType start;
    intType stop;
    memSizeType length;
    memSizeType result_size;
    memSizeType start_idx;
    memSizeType stop_idx;
    arrayType result;

  /* arr_range */
    isit_array(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_5(arguments));
    arr1 = take_array(arg_1(arguments));
    start = take_int(arg_3(arguments));
    stop = take_int(arg_5(arguments));
    length = arraySize(arr1);
    if (stop >= start && start <= arr1->max_position &&
        stop >= arr1->min_position && length >= 1) {
      if (start < arr1->min_position) {
        start = arr1->min_position;
      } /* if */
      if (stop > arr1->max_position) {
        stop = arr1->max_position;
      } /* if */
      result_size = arraySize2(start, stop);
      if (unlikely(!ALLOC_ARRAY(result, result_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arrayMaxPos(arr1->min_position, result_size);
      start_idx = arrayIndex(arr1, start);
      stop_idx = arrayIndex(arr1, stop);
      if (TEMP_OBJECT(arg_1(arguments))) {
        memcpy(result->arr, &arr1->arr[start_idx],
               (size_t) (result_size * sizeof(objectRecord)));
        destr_array(arr1->arr, start_idx);
        destr_array(&arr1->arr[stop_idx + 1], length - stop_idx - 1);
        FREE_ARRAY(arr1, length);
        arg_1(arguments)->value.arrayValue = NULL;
      } else {
        if (unlikely(!crea_array(result->arr,
                                 &arr1->arr[start_idx], result_size))) {
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arr_range(arr1 (size=" FMT_U_MEM "), " FMT_D ", " FMT_D "): "
                      "Cannot create empty array with minimal index.\n",
                      length, start, stop););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      if (unlikely(!ALLOC_ARRAY(result, 0))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arr1->min_position - 1;
    } /* if */
    return bld_array_temp(result);
  } /* arr_range */



/**
 *  Remove the element with 'position' from 'arr1' and return the removed element.
 *  @return the removed element.
 *  @exception RANGE_ERROR If 'position' is less than arr_minidx(arr2) or
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
      result = raise_exception(SYS_RNG_EXCEPTION);
    } else {
      array_pointer = arr1->arr;
      if (unlikely(!ALLOC_OBJECT(result))) {
        result = raise_exception(SYS_MEM_EXCEPTION);
      } else {
        memcpy(result, &array_pointer[position - arr1->min_position], sizeof(objectRecord));
        memmove(&array_pointer[position - arr1->min_position],
                &array_pointer[position - arr1->min_position + 1],
                (arraySize2(position, arr1->max_position) - 1) * sizeof(objectRecord));
        arr1_size = arraySize(arr1);
        resized_arr1 = REALLOC_ARRAY(arr1, arr1_size, arr1_size - 1);
        if (unlikely(resized_arr1 == NULL)) {
          /* A realloc, which shrinks memory, usually succeeds. */
          /* The probability that this code path is executed is */
          /* probably zero. The code below restores the old     */
          /* value of arr1.                                     */
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
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("arr_remove --> " F_U_MEM(08) " ", (memSizeType) result);
                trace1(result);
                printf("\n"););
    return result;
  } /* arr_remove */



objectType arr_sort (listType arguments)

  {
    objectType arr_arg;
    objectType data_cmp_func;
    arrayType arr1;
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
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arr1->max_position;
      if (unlikely(!crea_array(result->arr, arr1->arr, result_size))) {
        FREE_ARRAY(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    qsort_array(result->arr,
        &result->arr[result->max_position - result->min_position],
        data_cmp_func);
    return bld_array_temp(result);
  } /* arr_sort */



/**
 *  Get a sub array from the position 'start' with maximum length 'len'.
 *  @return the sub array from position 'start' with maximum length 'len'.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType arr_subarr (listType arguments)

  {
    arrayType arr1;
    intType start;
    intType len;
    memSizeType length;
    memSizeType result_size;
    memSizeType start_idx;
    memSizeType stop_idx;
    arrayType result;

  /* arr_subarr */
    isit_array(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_5(arguments));
    arr1 = take_array(arg_1(arguments));
    start = take_int(arg_3(arguments));
    len = take_int(arg_5(arguments));
    length = arraySize(arr1);
    if (len >= 1 && start <= arr1->max_position  && length >= 1 &&
        (start >= arr1->min_position ||
        (uintType) len > (uintType) (arr1->min_position - start))) {
      if (start < arr1->min_position) {
        start = arr1->min_position;
        len -= arr1->min_position - start;
      } /* if */
      if (len - 1 > arr1->max_position - start) {
        len = arr1->max_position - start + 1;
      } /* if */
      result_size = (memSizeType) (uintType) (len);
      if (unlikely(!ALLOC_ARRAY(result, result_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arrayMaxPos(arr1->min_position, result_size);
      start_idx = arrayIndex(arr1, start);
      stop_idx = arrayIndex(arr1, start + len - 1);
      if (TEMP_OBJECT(arg_1(arguments))) {
        memcpy(result->arr, &arr1->arr[start_idx],
               (size_t) (result_size * sizeof(objectRecord)));
        destr_array(arr1->arr, start_idx);
        destr_array(&arr1->arr[stop_idx + 1], length - stop_idx - 1);
        FREE_ARRAY(arr1, length);
        arg_1(arguments)->value.arrayValue = NULL;
      } else {
        if (unlikely(!crea_array(result->arr,
                                 &arr1->arr[start_idx], result_size))) {
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arr_subarr(arr1 (size=" FMT_U_MEM "), " FMT_D ", " FMT_D "): "
                      "Cannot create empty array with minimal index.\n",
                      length, start, len););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      if (unlikely(!ALLOC_ARRAY(result, 0))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arr1->min_position - 1;
    } /* if */
    return bld_array_temp(result);
  } /* arr_subarr */



/**
 *  Get a sub array beginning at the position 'start'.
 *  @return the sub array beginning at the start position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType arr_tail (listType arguments)

  {
    arrayType arr1;
    intType start;
    memSizeType length;
    memSizeType result_size;
    arrayType result;

  /* arr_tail */
    isit_array(arg_1(arguments));
    isit_int(arg_3(arguments));
    arr1 = take_array(arg_1(arguments));
    start = take_int(arg_3(arguments));
    length = arraySize(arr1);
    if (start <= arr1->max_position && length >= 1) {
      if (start < arr1->min_position) {
        start = arr1->min_position;
      } /* if */
      result_size = arraySize2(start, arr1->max_position);
      if (unlikely(!ALLOC_ARRAY(result, result_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arrayMaxPos(arr1->min_position, result_size);
      if (TEMP_OBJECT(arg_1(arguments))) {
        memcpy(result->arr, &arr1->arr[start - arr1->min_position],
               (size_t) (result_size * sizeof(objectRecord)));
        destr_array(arr1->arr, arraySize2(arr1->min_position, start) - 1);
        FREE_ARRAY(arr1, length);
        arg_1(arguments)->value.arrayValue = NULL;
        /* code to avoid destr_array:
        arr1->max_position = start - 1; */
      } else {
        if (unlikely(!crea_array(result->arr,
                                 &arr1->arr[start - arr1->min_position], result_size))) {
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arr_tail(arr1 (size=" FMT_U_MEM "), " FMT_D "): "
                      "Cannot create empty array with minimal index.\n",
                      length, start););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      if (unlikely(!ALLOC_ARRAY(result, 0))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arr1->min_position - 1;
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
    typeType result_element_type;
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
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result_size = (memSizeType) (uintType) factor;
      if (unlikely(!ALLOC_ARRAY(result, result_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->min_position = 1;
        result->max_position = factor;
        elem_to = result->arr;
        if (result_size > 0) {
          /* The element type of the result is the type of the 3rd formal parameter */
          result_element_type = curr_exec_object->value.listValue->obj->
                                descriptor.property->params->next->next->obj->type_of;
          if (TEMP_OBJECT(element) && element->type_of == result_element_type) {
            CLEAR_TEMP_FLAG(element);
            SET_VAR_FLAG(element);
            memcpy(elem_to, element, sizeof(objectRecord));
            FREE_OBJECT(element);
            arg_3(arguments) = NULL;
          } else {
            if (unlikely(!arr_elem_initialisation(result_element_type,
                                                  elem_to, element))) {
              FREE_ARRAY(result, result_size);
              return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
            } /* if */
          } /* if */
          position = 1;
          while (position < result_size) {
            if (unlikely(!arr_elem_initialisation(result_element_type,
                                                  &elem_to[position], elem_to))) {
              /* If a create fails (mostly no memory) all elements     */
              /* created up to this point must be destroyed to recycle */
              /* the memory correct. */
              destr_array(elem_to, position);
              FREE_ARRAY(result, result_size);
              return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
            } /* if */
            position++;
          } /* for */
        } /* if */
        return bld_array_temp(result);
      } /* if */
    } /* if */
  } /* arr_times */
