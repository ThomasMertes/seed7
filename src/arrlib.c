/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  Changes: 1993, 1994, 2013  Thomas Mertes                        */
/*  Content: All primitive actions for array types.                 */
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
#include "executl.h"
#include "objutl.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "arrlib.h"



static void qsort_array (objecttype begin_sort, objecttype end_sort,
    objecttype cmp_func)

  {
    objectrecord compare_elem;
    objectrecord help_element;
    objecttype middle_elem;
    objecttype less_elem;
    objecttype greater_elem;
    objecttype cmp_obj;
    inttype cmp;

  /* qsort_array */
    if (end_sort - begin_sort < 8) {
      for (middle_elem = begin_sort + 1; middle_elem <= end_sort; middle_elem++) {
        memcpy(&compare_elem, middle_elem, sizeof(objectrecord));
        less_elem = begin_sort - 1;
        do {
          less_elem++;
          cmp_obj = param3_call(cmp_func, less_elem, &compare_elem, cmp_func);
          isit_int2(cmp_obj);
          cmp = take_int(cmp_obj);
          FREE_OBJECT(cmp_obj);
        } while (cmp < 0);
        memmove(&less_elem[1], less_elem,
            (memsizetype) (middle_elem - less_elem) * sizeof(objectrecord));
        memcpy(less_elem, &compare_elem, sizeof(objectrecord));
      } /* for */
    } else {
      middle_elem = &begin_sort[(end_sort - begin_sort) >> 1];
      memcpy(&compare_elem, middle_elem, sizeof(objectrecord));
      memcpy(middle_elem, end_sort, sizeof(objectrecord));
      memcpy(end_sort, &compare_elem, sizeof(objectrecord));
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
        memcpy(&help_element, less_elem, sizeof(objectrecord));
        memcpy(less_elem, greater_elem, sizeof(objectrecord));
        memcpy(greater_elem, &help_element, sizeof(objectrecord));
      } while (greater_elem > less_elem);
      memcpy(greater_elem, less_elem, sizeof(objectrecord));
      memcpy(less_elem, &compare_elem, sizeof(objectrecord));
      memcpy(end_sort, &help_element, sizeof(objectrecord));
      qsort_array(begin_sort, less_elem - 1, cmp_func);
      qsort_array(less_elem + 1, end_sort, cmp_func);
    } /* if */
  } /* qsort_array */



objecttype arr_append (listtype arguments)

  {
    objecttype arr_variable;
    arraytype arr_to;
    arraytype arr_from;
    arraytype new_arr;
    memsizetype new_size;
    memsizetype arr_to_size;
    memsizetype arr_from_size;

  /* arr_append */
    /* printf("begin arr_append %lu\n", heapsize()); */
    arr_variable = arg_1(arguments);
    isit_array(arr_variable);
    is_variable(arr_variable);
    arr_to = take_array(arr_variable);
    isit_array(arg_3(arguments));
    arr_from = take_array(arg_3(arguments));
    arr_from_size = arraySize(arr_from);
    if (arr_from_size != 0) {
      arr_to_size = arraySize(arr_to);
      if (arr_to_size > MAX_ARR_LEN - arr_from_size ||
          arr_to->max_position > (inttype) (MAX_MEM_INDEX - arr_from_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_size = arr_to_size + arr_from_size;
        new_arr = REALLOC_ARRAY(arr_to, arr_to_size, new_size);
        if (new_arr == NULL) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          COUNT3_ARRAY(arr_to_size, new_size);
          arr_variable->value.arrayvalue = new_arr;
          if (TEMP_OBJECT(arg_3(arguments))) {
            memcpy(&new_arr->arr[arr_to_size], arr_from->arr,
                (size_t) (arr_from_size * sizeof(objectrecord)));
            new_arr->max_position += (inttype) arr_from_size;
            FREE_ARRAY(arr_from, arr_from_size);
            arg_3(arguments)->value.arrayvalue = NULL;
          } else {
            /* It is possible that arr_to == arr_from holds. */
            /* In this case 'arr_from' must be corrected     */
            /* after realloc() enlarged 'arr_to'.            */
            if (arr_to == arr_from) {
              arr_from = new_arr;
            } /* if */
            if (!crea_array(&new_arr->arr[arr_to_size], arr_from->arr,
                arr_from_size)) {
              arr_to = REALLOC_ARRAY(new_arr, new_size, arr_to_size);
              if (arr_to == NULL) {
                return raise_exception(SYS_MEM_EXCEPTION);
              } /* if */
              COUNT3_ARRAY(new_size, arr_to_size);
              arr_variable->value.arrayvalue = arr_to;
              return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
            } else {
              new_arr->max_position += (inttype) arr_from_size;
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    /* printf("end   arr_append %lu\n", heapsize()); */
    return SYS_EMPTY_OBJECT;
  } /* arr_append */



objecttype arr_arrlit (listtype arguments)

  {
    objecttype arr_arg;
    arraytype arr1;
    memsizetype result_size;
    arraytype result_array;
    objecttype result;

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
      if (!ALLOC_ARRAY(result_array, result_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result_array->min_position = 1;
      result_array->max_position = (inttype) result_size;
      if (!crea_array(result_array->arr, arr1->arr, result_size)) {
        FREE_ARRAY(result_array, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
      result = bld_array_temp(result_array);
    } /* if */
    return result;
  } /* arr_arrlit */



objecttype arr_arrlit2 (listtype arguments)

  {
    inttype start_position;
    objecttype arr_arg;
    arraytype arr1;
    memsizetype result_size;
    arraytype result_array;
    objecttype result;

  /* arr_arrlit2 */
    isit_int(arg_2(arguments));
    start_position = take_int(arg_2(arguments));
    arr_arg = arg_4(arguments);
    isit_array(arr_arg);
    arr1 = take_array(arr_arg);
    result_size = arraySize(arr1);
    if (start_position < MIN_MEM_INDEX ||
        start_position > MAX_MEM_INDEX - result_size + 1) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      if (TEMP_OBJECT(arr_arg)) {
        arr1->min_position = start_position;
        arr1->max_position = (inttype) ((memsizetype) start_position + result_size - 1);
        result = arr_arg;
        result->type_of = NULL;
        arg_4(arguments) = NULL;
      } else {
        if (!ALLOC_ARRAY(result_array, result_size)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result_array->min_position = start_position;
        result_array->max_position = (inttype) ((memsizetype) start_position + result_size - 1);
        if (!crea_array(result_array->arr, arr1->arr, result_size)) {
          FREE_ARRAY(result_array, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
        result = bld_array_temp(result_array);
      } /* if */
    } /* if */
    return result;
  } /* arr_arrlit2 */



objecttype arr_baselit (listtype arguments)

  {
    objecttype element;
    typetype result_element_type;
    memsizetype result_size;
    arraytype result;

  /* arr_baselit */
    element = arg_3(arguments);
    result_size = 1;
    if (!ALLOC_ARRAY(result, result_size)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    result->min_position = 1;
    result->max_position = 1;
    /* The element type of the result is the type of the 3rd formal parameter */
    result_element_type = curr_exec_object->value.listvalue->obj->
                          descriptor.property->params->next->next->obj->type_of;
    if (TEMP_OBJECT(element) && element->type_of == result_element_type) {
      CLEAR_TEMP_FLAG(element);
      SET_VAR_FLAG(element);
      memcpy(&result->arr[0], element, sizeof(objectrecord));
      FREE_OBJECT(element);
      arg_3(arguments) = NULL;
    } else {
      if (!arr_elem_initialisation(result_element_type, &result->arr[0], element)) {
        FREE_ARRAY(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_baselit */



objecttype arr_baselit2 (listtype arguments)

  {
    inttype start_position;
    objecttype element;
    typetype result_element_type;
    memsizetype result_size;
    arraytype result;

  /* arr_baselit2 */
    isit_int(arg_2(arguments));
    start_position = take_int(arg_2(arguments));
    element = arg_4(arguments);
    result_size = 1;
    if (!ALLOC_ARRAY(result, result_size)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    result->min_position = start_position;
    result->max_position = start_position;
    /* The element type of the result is the type of the 4th formal parameter */
    result_element_type = curr_exec_object->value.listvalue->obj->
                          descriptor.property->params->next->next->next->obj->type_of;
    if (TEMP_OBJECT(element) && element->type_of == result_element_type) {
      CLEAR_TEMP_FLAG(element);
      SET_VAR_FLAG(element);
      memcpy(&result->arr[0], element, sizeof(objectrecord));
      FREE_OBJECT(element);
      arg_4(arguments) = NULL;
    } else {
      if (!arr_elem_initialisation(result_element_type, &result->arr[0], element)) {
        FREE_ARRAY(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_baselit2 */



objecttype arr_cat (listtype arguments)

  {
    arraytype arr1;
    arraytype arr2;
    memsizetype arr1_size;
    memsizetype arr2_size;
    memsizetype result_size;
    arraytype result;

  /* arr_cat */
    isit_array(arg_1(arguments));
    isit_array(arg_3(arguments));
    arr1 = take_array(arg_1(arguments));
    arr2 = take_array(arg_3(arguments));
    arr1_size = arraySize(arr1);
    arr2_size = arraySize(arr2);
    if (arr1_size > MAX_ARR_LEN - arr2_size ||
        arr1->max_position > (inttype) (MAX_MEM_INDEX - arr2_size)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result_size = arr1_size + arr2_size;
      if (TEMP_OBJECT(arg_1(arguments))) {
        result = arr1;
        result = REALLOC_ARRAY(result, arr1_size, result_size);
        if (result == NULL) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        COUNT3_ARRAY(arr1_size, result_size);
        result->max_position += (inttype) arr2_size;
        arg_1(arguments)->value.arrayvalue = NULL;
      } else {
        if (!ALLOC_ARRAY(result, result_size)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->min_position = arr1->min_position;
        result->max_position = (inttype) ((memsizetype) arr1->max_position + arr2_size);
        if (!crea_array(result->arr, arr1->arr, arr1_size)) {
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
      if (TEMP_OBJECT(arg_3(arguments))) {
        memcpy(&result->arr[arr1_size], arr2->arr,
            (size_t) (arr2_size * sizeof(objectrecord)));
        FREE_ARRAY(arr2, arr2_size);
        arg_3(arguments)->value.arrayvalue = NULL;
      } else {
        if (!crea_array(&result->arr[arr1_size], arr2->arr,
            arr2_size)) {
          destr_array(result->arr, arr1_size);
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_cat */



objecttype arr_conv (listtype arguments)

  {
    objecttype arr_arg;
    arraytype arr1;
    memsizetype result_size;
    arraytype result;

  /* arr_conv */
    arr_arg = arg_3(arguments);
    isit_array(arr_arg);
    if (TEMP_OBJECT(arr_arg)) {
      result = take_array(arr_arg);
      arr_arg->value.arrayvalue = NULL;
      return bld_array_temp(result);
    } else if (VAR_OBJECT(arr_arg)) {
      return arr_arg;
    } else {
      arr1 = take_array(arr_arg);
      result_size = arraySize(arr1);
      if (!ALLOC_ARRAY(result, result_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arr1->max_position;
      if (!crea_array(result->arr, arr1->arr, result_size)) {
        FREE_ARRAY(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
      return bld_array_temp(result);
    } /* if */
  } /* arr_conv */



objecttype arr_cpy (listtype arguments)

  {
    objecttype arr_to;
    objecttype arr_from;
    arraytype arr_dest;
    arraytype arr_source;
    memsizetype arr_dest_size;
    memsizetype arr_source_size;
    arraytype new_arr;

  /* arr_cpy */
    arr_to = arg_1(arguments);
    arr_from = arg_3(arguments);
    isit_array(arr_to);
    isit_array(arr_from);
    is_variable(arr_to);
    arr_dest = take_array(arr_to);
    arr_source = take_array(arr_from);
    if (TEMP_OBJECT(arr_from)) {
      arr_dest_size = arraySize(arr_dest);
      destr_array(arr_dest->arr, arr_dest_size);
      FREE_ARRAY(arr_dest, arr_dest_size);
      arr_to->value.arrayvalue = arr_source;
      arr_from->value.arrayvalue = NULL;
    } else {
      arr_source_size = arraySize(arr_source);
      if (arr_dest->min_position != arr_source->min_position ||
          arr_dest->max_position != arr_source->max_position) {
        if (!ALLOC_ARRAY(new_arr, arr_source_size)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          new_arr->min_position = arr_source->min_position;
          new_arr->max_position = arr_source->max_position;
          if (!crea_array(new_arr->arr, arr_source->arr, arr_source_size)) {
            FREE_ARRAY(new_arr, arr_source_size);
            return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
          } /* if */
          arr_dest_size = arraySize(arr_dest);
          destr_array(arr_dest->arr, arr_dest_size);
          FREE_ARRAY(arr_dest, arr_dest_size);
          arr_to->value.arrayvalue = new_arr;
        } /* if */
      } else {
        cpy_array(arr_dest->arr, arr_source->arr, arr_source_size);
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* arr_cpy */



objecttype arr_create (listtype arguments)

  {
    objecttype arr_to;
    objecttype arr_from;
    arraytype arr_source;
    memsizetype new_size;
    arraytype new_arr;

  /* arr_create */
    arr_to = arg_1(arguments);
    arr_from = arg_3(arguments);
    isit_array(arr_from);
    arr_source = take_array(arr_from);
    SET_CATEGORY_OF_OBJ(arr_to, ARRAYOBJECT);
    if (TEMP_OBJECT(arr_from)) {
      arr_to->value.arrayvalue = arr_source;
      arr_from->value.arrayvalue = NULL;
    } else {
      new_size = arraySize(arr_source);
      if (!ALLOC_ARRAY(new_arr, new_size)) {
        arr_to->value.arrayvalue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_arr->min_position = arr_source->min_position;
        new_arr->max_position = arr_source->max_position;
        if (!crea_array(new_arr->arr, arr_source->arr, new_size)) {
          FREE_ARRAY(new_arr, new_size);
          arr_to->value.arrayvalue = NULL;
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
        arr_to->value.arrayvalue = new_arr;
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* arr_create */



objecttype arr_destr (listtype arguments)

  {
    arraytype old_arr;
    memsizetype old_size;

  /* arr_destr */
    isit_array(arg_1(arguments));
    old_arr = take_array(arg_1(arguments));
    if (old_arr != NULL) {
      old_size = arraySize(old_arr);
      destr_array(old_arr->arr, old_size);
      FREE_ARRAY(old_arr, old_size);
      arg_1(arguments)->value.arrayvalue = NULL;
    } /* if */
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* arr_destr */



objecttype arr_empty (listtype arguments)

  {
    arraytype result;

  /* arr_empty */
    if (!ALLOC_ARRAY(result, 0)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->min_position = 1;
      result->max_position = 0;
    } /* if */
    return bld_array_temp(result);
  } /* arr_empty */



objecttype arr_extend (listtype arguments)

  {
    arraytype arr1;
    objecttype element;
    typetype result_element_type;
    memsizetype arr1_size;
    memsizetype result_size;
    arraytype result;

  /* arr_extend */
    isit_array(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    element = arg_3(arguments);
    arr1_size = arraySize(arr1);
    if (arr1_size > MAX_ARR_LEN - 1 ||
        arr1->max_position > (inttype) (MAX_MEM_INDEX - 1)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result_size = arr1_size + 1;
      if (TEMP_OBJECT(arg_1(arguments))) {
        result = arr1;
        result = REALLOC_ARRAY(result, arr1_size, result_size);
        if (result == NULL) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        COUNT3_ARRAY(arr1_size, result_size);
        result->max_position++;
        arg_1(arguments)->value.arrayvalue = NULL;
      } else {
        if (!ALLOC_ARRAY(result, result_size)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->min_position = arr1->min_position;
        result->max_position = arr1->max_position + 1;
        if (!crea_array(result->arr, arr1->arr, arr1_size)) {
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
      /* The element type of the result is the type of the 3rd formal parameter */
      result_element_type = curr_exec_object->value.listvalue->obj->
                            descriptor.property->params->next->next->obj->type_of;
      if (TEMP_OBJECT(element) && element->type_of == result_element_type) {
        CLEAR_TEMP_FLAG(element);
        SET_VAR_FLAG(element);
        memcpy(&result->arr[arr1_size], element, sizeof(objectrecord));
        FREE_OBJECT(element);
        arg_3(arguments) = NULL;
      } else {
        if (!arr_elem_initialisation(result_element_type, &result->arr[arr1_size], element)) {
          destr_array(result->arr, arr1_size);
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_extend */



objecttype arr_gen (listtype arguments)

  {
    objecttype element1;
    objecttype element2;
    typetype result_element_type;
    memsizetype result_size;
    arraytype result;

  /* arr_gen */
    element1 = arg_1(arguments);
    element2 = arg_3(arguments);
    result_size = 2;
    if (!ALLOC_ARRAY(result, result_size)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    result->min_position = 1;
    result->max_position = 2;
    /* The element type of the result is the type of the 1st formal parameter */
    result_element_type = curr_exec_object->value.listvalue->obj->
                          descriptor.property->params->obj->type_of;
    if (TEMP_OBJECT(element1) && element1->type_of == result_element_type) {
      CLEAR_TEMP_FLAG(element1);
      SET_VAR_FLAG(element1);
      memcpy(&result->arr[0], element1, sizeof(objectrecord));
      FREE_OBJECT(element1);
      arg_1(arguments) = NULL;
    } else {
      if (!arr_elem_initialisation(result_element_type, &result->arr[0], element1)) {
        FREE_ARRAY(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    if (TEMP_OBJECT(element2) && element2->type_of == result_element_type) {
      CLEAR_TEMP_FLAG(element2);
      SET_VAR_FLAG(element2);
      memcpy(&result->arr[1], element2, sizeof(objectrecord));
      FREE_OBJECT(element2);
      arg_3(arguments) = NULL;
    } else {
      if (!arr_elem_initialisation(result_element_type, &result->arr[1], element2)) {
        destr_array(result->arr, 1);
        FREE_ARRAY(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    return bld_array_temp(result);
  } /* arr_gen */



objecttype arr_head (listtype arguments)

  {
    arraytype arr1;
    inttype stop;
    memsizetype length;
    memsizetype result_size;
    arraytype resized_result;
    arraytype result;

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
        arg_1(arguments)->value.arrayvalue = NULL;
        destr_array(&result->arr[result_size], length - result_size);
        resized_result = REALLOC_ARRAY(result, length, result_size);
        if (resized_result == NULL) {
          destr_array(result->arr, result_size);
          FREE_ARRAY(result, length);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
        result = resized_result;
        COUNT3_ARRAY(length, result_size);
        result->max_position = stop;
      } else {
        if (!ALLOC_ARRAY(result, result_size)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->min_position = arr1->min_position;
        result->max_position = stop;
        if (!crea_array(result->arr, arr1->arr, result_size)) {
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } else {
      if (!ALLOC_ARRAY(result, 0)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arr1->min_position - 1;
    } /* if */
    return bld_array_temp(result);
  } /* arr_head */



objecttype arr_idx (listtype arguments)

  {
    arraytype arr1;
    inttype position;
    objecttype array_pointer;
    objecttype result;

  /* arr_idx */
    isit_array(arg_1(arguments));
    isit_int(arg_3(arguments));
    arr1 = take_array(arg_1(arguments));
    position = take_int(arg_3(arguments));
    if (position >= arr1->min_position && position <= arr1->max_position) {
      array_pointer = arr1->arr;
      if (TEMP_OBJECT(arg_1(arguments))) {
        /* The array will be destroyed after indexing. */
        /* A copy is necessary here to avoid a crash !!!!! */
        if (!ALLOC_OBJECT(result)) {
          result = raise_exception(SYS_MEM_EXCEPTION);
        } else {
          memcpy(result, &array_pointer[position - arr1->min_position], sizeof(objectrecord));
          SET_TEMP_FLAG(result);
          destr_array(array_pointer, arraySize2(arr1->min_position, position) - 1);
          destr_array(&array_pointer[position - arr1->min_position + 1],
              arraySize2(position, arr1->max_position) - 1);
          FREE_ARRAY(arr1, arraySize(arr1));
          arg_1(arguments)->value.arrayvalue = NULL;
          /* code to avoid destr_array:
          if (position != arr1->max_position) {
            memcpy(&array_pointer[position - arr1->min_position],
                &array_pointer[arr1->max_position - arr1->min_position], sizeof(objectrecord));
          }
          arr1->max_position--; */
        } /* if */
      } else {
        /* This is the normal case: The array will exist after indexing. */
        result = &array_pointer[position - arr1->min_position];
      } /* if */
    } else {
      result = raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    /* printf("arr_idx => %08lx ", result);
       trace1(result);
       printf("\n"); */
    return result;
  } /* arr_idx */



objecttype arr_lng (listtype arguments)

  {
    arraytype arr1;

  /* arr_lng */
    isit_array(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    return bld_int_temp(arr1->max_position - arr1->min_position + 1);
  } /* arr_lng */



objecttype arr_maxidx (listtype arguments)

  {
    arraytype arr1;

  /* arr_maxidx */
    isit_array(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    return bld_int_temp(arr1->max_position);
  } /* arr_maxidx */



objecttype arr_minidx (listtype arguments)

  {
    arraytype arr1;

  /* arr_minidx */
    isit_array(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    return bld_int_temp(arr1->min_position);
  } /* arr_minidx */



objecttype arr_push (listtype arguments)

  {
    objecttype arr_variable;
    arraytype arr_to;
    objecttype element;
    typetype result_element_type;
    arraytype new_arr;
    memsizetype new_size;
    memsizetype arr_to_size;

  /* arr_push */
    /* printf("begin arr_push %lu\n", heapsize()); */
    arr_variable = arg_1(arguments);
    isit_array(arr_variable);
    is_variable(arr_variable);
    arr_to = take_array(arr_variable);
    element = arg_3(arguments);
    arr_to_size = arraySize(arr_to);
    if (arr_to_size > MAX_ARR_LEN - 1 ||
        arr_to->max_position > (inttype) (MAX_MEM_INDEX - 1)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      new_size = arr_to_size + 1;
      new_arr = REALLOC_ARRAY(arr_to, arr_to_size, new_size);
      if (new_arr == NULL) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        COUNT3_ARRAY(arr_to_size, new_size);
        arr_variable->value.arrayvalue = new_arr;
        /* The element type of the result is the type of the 3rd formal parameter */
        result_element_type = curr_exec_object->value.listvalue->obj->
                              descriptor.property->params->next->next->obj->type_of;
        if (TEMP_OBJECT(element) && element->type_of == result_element_type) {
          CLEAR_TEMP_FLAG(element);
          SET_VAR_FLAG(element);
          memcpy(&new_arr->arr[arr_to_size], element, sizeof(objectrecord));
          new_arr->max_position ++;
          FREE_OBJECT(element);
          arg_3(arguments) = NULL;
        } else {
          if (!arr_elem_initialisation(result_element_type, &new_arr->arr[arr_to_size], element)) {
            arr_to = REALLOC_ARRAY(new_arr, new_size, arr_to_size);
            if (arr_to == NULL) {
              return raise_exception(SYS_MEM_EXCEPTION);
            } /* if */
            COUNT3_ARRAY(new_size, arr_to_size);
            arr_variable->value.arrayvalue = arr_to;
            return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
          } else {
            new_arr->max_position ++;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    /* printf("end   arr_push %lu\n", heapsize()); */
    return SYS_EMPTY_OBJECT;
  } /* arr_push */



objecttype arr_range (listtype arguments)

  {
    arraytype arr1;
    inttype start;
    inttype stop;
    memsizetype length;
    memsizetype result_size;
    memsizetype start_idx;
    memsizetype stop_idx;
    arraytype result;

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
      if (!ALLOC_ARRAY(result, result_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = (inttype) ((memsizetype) arr1->min_position + result_size - 1);
      start_idx = arrayIndex(arr1, start);
      stop_idx = arrayIndex(arr1, stop);
      if (TEMP_OBJECT(arg_1(arguments))) {
        memcpy(result->arr, &arr1->arr[start_idx],
            (size_t) (result_size * sizeof(objectrecord)));
        destr_array(arr1->arr, start_idx);
        destr_array(&arr1->arr[stop_idx + 1], length - stop_idx - 1);
        FREE_ARRAY(arr1, length);
        arg_1(arguments)->value.arrayvalue = NULL;
      } else {
        if (!crea_array(result->arr, &arr1->arr[start_idx],
            result_size)) {
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } else {
      if (!ALLOC_ARRAY(result, 0)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arr1->min_position - 1;
    } /* if */
    return bld_array_temp(result);
  } /* arr_range */



objecttype arr_remove (listtype arguments)

  {
    arraytype arr1;
    arraytype resized_arr1;
    inttype position;
    objecttype array_pointer;
    memsizetype arr1_size;
    objecttype result;

  /* arr_remove */
    isit_array(arg_1(arguments));
    isit_int(arg_2(arguments));
    is_variable(arg_1(arguments));
    arr1 = take_array(arg_1(arguments));
    position = take_int(arg_2(arguments));
    if (position >= arr1->min_position && position <= arr1->max_position) {
      array_pointer = arr1->arr;
      if (!ALLOC_OBJECT(result)) {
        result = raise_exception(SYS_MEM_EXCEPTION);
      } else {
        memcpy(result, &array_pointer[position - arr1->min_position], sizeof(objectrecord));
        memmove(&array_pointer[position - arr1->min_position],
            &array_pointer[position - arr1->min_position + 1],
            (arraySize2(position, arr1->max_position) - 1) * sizeof(objectrecord));
        arr1_size = arraySize(arr1);
        resized_arr1 = REALLOC_ARRAY(arr1, arr1_size, arr1_size - 1);
        if (unlikely(resized_arr1 == NULL)) {
          /* A realloc, which shrinks memory, usually succeeds. */
          /* The probability that this code path is executed is */
          /* probably zero. The code below restores the old     */
          /* value of arr1.                                     */
          memmove(&array_pointer[position - arr1->min_position + 1],
              &array_pointer[position - arr1->min_position],
              (arraySize2(position, arr1->max_position) - 1) * sizeof(objectrecord));
          memcpy(&array_pointer[position - arr1->min_position], result, sizeof(objectrecord));
          FREE_OBJECT(result);
          result = raise_exception(SYS_MEM_EXCEPTION);
        } else {
          arr1 = resized_arr1;
          COUNT3_ARRAY(arr1_size, arr1_size - 1);
          arr1->max_position--;
          arg_1(arguments)->value.arrayvalue = arr1;
          SET_TEMP_FLAG(result);
        } /* if */
      } /* if */
    } else {
      result = raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    /* printf("arr_remove => ");
       trace1(result);
       printf("\n"); */
    return result;
  } /* arr_remove */



objecttype arr_sort (listtype arguments)

  {
    objecttype arr_arg;
    objecttype data_cmp_func;
    arraytype arr1;
    memsizetype result_size;
    arraytype result;

  /* arr_sort */
    arr_arg = arg_1(arguments);
    isit_array(arr_arg);
    data_cmp_func    = take_reference(arg_2(arguments));
    if (TEMP2_OBJECT(arr_arg)) {
      result = take_array(arr_arg);
      arr_arg->value.arrayvalue = NULL;
    } else {
      arr1 = take_array(arr_arg);
      result_size = arraySize(arr1);
      if (!ALLOC_ARRAY(result, result_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arr1->max_position;
      if (!crea_array(result->arr, arr1->arr, result_size)) {
        FREE_ARRAY(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    qsort_array(result->arr,
        &result->arr[result->max_position - result->min_position],
        data_cmp_func);
    return bld_array_temp(result);
  } /* arr_sort */



objecttype arr_subarr (listtype arguments)

  {
    arraytype arr1;
    inttype start;
    inttype len;
    memsizetype length;
    memsizetype result_size;
    memsizetype start_idx;
    memsizetype stop_idx;
    arraytype result;

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
        (uinttype) len > (uinttype) (arr1->min_position - start))) {
      if (start < arr1->min_position) {
        start = arr1->min_position;
        len -= arr1->min_position - start;
      } /* if */
      if (len - 1 > arr1->max_position - start) {
        len = arr1->max_position - start + 1;
      } /* if */
      result_size = (memsizetype) (uinttype) (len);
      if (!ALLOC_ARRAY(result, result_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = (inttype) ((memsizetype) arr1->min_position + result_size - 1);
      start_idx = arrayIndex(arr1, start);
      stop_idx = arrayIndex(arr1, start + len - 1);
      if (TEMP_OBJECT(arg_1(arguments))) {
        memcpy(result->arr, &arr1->arr[start_idx],
            (size_t) (result_size * sizeof(objectrecord)));
        destr_array(arr1->arr, start_idx);
        destr_array(&arr1->arr[stop_idx + 1], length - stop_idx - 1);
        FREE_ARRAY(arr1, length);
        arg_1(arguments)->value.arrayvalue = NULL;
      } else {
        if (!crea_array(result->arr, &arr1->arr[start_idx],
            result_size)) {
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } else {
      if (!ALLOC_ARRAY(result, 0)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arr1->min_position - 1;
    } /* if */
    return bld_array_temp(result);
  } /* arr_subarr */



objecttype arr_tail (listtype arguments)

  {
    arraytype arr1;
    inttype start;
    memsizetype length;
    memsizetype result_size;
    arraytype result;

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
      if (!ALLOC_ARRAY(result, result_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = (inttype) ((memsizetype) arr1->min_position + result_size - 1);
      if (TEMP_OBJECT(arg_1(arguments))) {
        memcpy(result->arr, &arr1->arr[start - arr1->min_position],
            (size_t) (result_size * sizeof(objectrecord)));
        destr_array(arr1->arr, arraySize2(arr1->min_position, start) - 1);
        FREE_ARRAY(arr1, length);
        arg_1(arguments)->value.arrayvalue = NULL;
        /* code to avoid destr_array:
        arr1->max_position = start - 1; */
      } else {
        if (!crea_array(result->arr, &arr1->arr[start - arr1->min_position],
            result_size)) {
          FREE_ARRAY(result, result_size);
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } else {
      if (!ALLOC_ARRAY(result, 0)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->min_position = arr1->min_position;
      result->max_position = arr1->min_position - 1;
    } /* if */
    return bld_array_temp(result);
  } /* arr_tail */



objecttype arr_times (listtype arguments)

  {
    inttype factor;
    objecttype element;
    memsizetype position;
    objecttype elem_to;
    typetype result_element_type;
    memsizetype result_size;
    arraytype result;

  /* arr_times */
    isit_int(arg_1(arguments));
    factor = take_int(arg_1(arguments));
    element = arg_3(arguments);
    if (factor < 0) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else if ((uinttype) factor > MAX_ARR_LEN ||
               (uinttype) factor > MAX_MEM_INDEX) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result_size = (memsizetype) (uinttype) factor;
      if (!ALLOC_ARRAY(result, result_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->min_position = 1;
        result->max_position = factor;
        elem_to = result->arr;
        if (result_size > 0) {
          /* printf("arr_times: ");
          trace1(element);
          printf("\n");
          trace1(curr_exec_object);
          printf("\n");
          trace1(curr_exec_object->value.listvalue->obj);
          printf("\n");
          printf("%d\n", HAS_ENTITY(curr_exec_object->value.listvalue->obj));
          prot_list(GET_ENTITY(curr_exec_object->value.listvalue->obj)->name_list);
          printf("\n");
          trace1(GET_ENTITY(curr_exec_object->value.listvalue->obj)->name_list->next->next->obj);
          printf("\n");
          trace1(GET_ENTITY(curr_exec_object->value.listvalue->obj)->name_list->next->next->obj->value.objvalue);
          printf("\n");
          printtype(GET_ENTITY(curr_exec_object->value.listvalue->obj)->name_list->next->next->obj->value.objvalue->type_of);
          printf("\n"); */
          /* The element type of the result is the type of the 3rd formal parameter */
          result_element_type = curr_exec_object->value.listvalue->obj->
                                descriptor.property->params->next->next->obj->type_of;
          if (TEMP_OBJECT(element) && element->type_of == result_element_type) {
            CLEAR_TEMP_FLAG(element);
            SET_VAR_FLAG(element);
            memcpy(elem_to, element, sizeof(objectrecord));
            FREE_OBJECT(element);
            arg_3(arguments) = NULL;
          } else {
            if (!arr_elem_initialisation(result_element_type, elem_to, element)) {
              FREE_ARRAY(result, result_size);
              return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
            } /* if */
          } /* if */
          position = 1;
          while (position < result_size) {
            if (!arr_elem_initialisation(result_element_type, &elem_to[position], elem_to)) {
              /* When one create fails (mostly no memory) all elements */
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
