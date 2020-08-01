/********************************************************************/
/*                                                                  */
/*  arr_rtl.c     Primitive actions for the array type.             */
/*  Copyright (C) 1989 - 2006  Thomas Mertes                        */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/arr_rtl.c                                       */
/*  Changes: 1991, 1992, 1993, 1994, 2005, 2006  Thomas Mertes      */
/*  Content: Primitive actions for the array type.                  */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "arr_rtl.h"



#ifdef ANSI_C

void arrAppend (rtlArraytype *arr_variable, rtlArraytype arr_from)
#else

void arrAppend (arr_variable, arr_from)
rtlArraytype *arr_variable;
rtlArraytype arr_from;
#endif

  {
    rtlArraytype arr_to;
    memsizetype new_size;
    memsizetype arr_to_size;
    memsizetype arr_from_size;

  /* arrAppend */
    arr_to = *arr_variable;
    arr_from_size = arr_from->max_position - arr_from->min_position + 1;
    if (arr_from_size != 0) {
      arr_to_size = arr_to->max_position - arr_to->min_position + 1;
      new_size = arr_to_size + arr_from_size;
      if (!RESIZE_RTL_ARRAY(arr_to, arr_to_size, new_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_RTL_ARRAY(arr_to_size, new_size);
        *arr_variable = arr_to;
        memcpy(&arr_to->arr[arr_to_size], arr_from->arr,
            (SIZE_TYPE) (arr_from_size * sizeof(rtlObjecttype)));
        arr_to->max_position += arr_from_size;
        FREE_RTL_ARRAY(arr_from, arr_from_size);
      } /* if */
    } /* if */
  } /* arrAppend */



#ifdef ANSI_C

rtlArraytype arrArrlit (rtlArraytype arr1)
#else

rtlArraytype arrArrlit (arr1)
rtlArraytype arr1;
#endif

  { /* arrArrlit */
    return(arr1);
  } /* arrArrlit */



#ifdef ANSI_C

rtlArraytype arrArrlit2 (inttype start_position, rtlArraytype arr1)
#else

rtlArraytype arrArrlit2 (start_position, arr1)
inttype start_position;
rtlArraytype arr1;
#endif

  {
    memsizetype result_size;

  /* arrArrlit2 */
    result_size = arr1->max_position - arr1->min_position + 1;
    arr1->min_position = start_position;
    arr1->max_position = start_position + result_size - 1;
    return(arr1);
  } /* arrArrlit2 */



#ifdef ANSI_C

rtlArraytype arrBaselit (rtlObjecttype element)
#else

rtlArraytype arrBaselit (element)
objecttype element;
#endif

  {
    memsizetype result_size;
    rtlArraytype result;

  /* arrBaselit */
    result_size = 1;
    if (!ALLOC_RTL_ARRAY(result, result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      COUNT_RTL_ARRAY(result_size);
      result->min_position = 1;
      result->max_position = 1;
      result->arr[0] = element;
    } /* if */
    return(result);
  } /* arrBaselit */



#ifdef ANSI_C

rtlArraytype arrBaselit2 (inttype start_position, rtlObjecttype element)
#else

rtlArraytype arrBaselit2 (start_position, element)
inttype start_position;
objecttype element;
#endif

  {
    memsizetype result_size;
    rtlArraytype result;

  /* arrBaselit2 */
    result_size = 1;
    if (!ALLOC_RTL_ARRAY(result, result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      COUNT_RTL_ARRAY(result_size);
      result->min_position = start_position;
      result->max_position = start_position;
      result->arr[0] = element;
    } /* if */
    return(result);
  } /* arrBaselit2 */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

rtlArraytype arrCat (rtlArraytype arr1, rtlArraytype arr2)
#else

rtlArraytype arrCat (arr1, arr2)
rtlArraytype arr1;
rtlArraytype arr2;
#endif

  {
    memsizetype arr1_size;
    memsizetype arr2_size;
    memsizetype result_size;
    arraytype result;

  /* arrCat */
    arr1_size = arr1->max_position - arr1->min_position + 1;
    arr2_size = arr2->max_position - arr2->min_position + 1;
    result_size = arr1_size + arr2_size;
    if (!ALLOC_RTL_ARRAY(result, result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      COUNT_RTL_ARRAY(result_size);
      result->min_position = arr1->min_position;
      result->max_position = arr1->max_position + arr2_size;
      memcpy(result->arr, arr1->arr, arr1_size * sizeof(rtlObjecttype));
      memcpy(&result->arr[arr1_size], arr2->arr, arr2_size * sizeof(rtlObjecttype));
      FREE_RTL_ARRAY(arr1, arr1_size);
      FREE_RTL_ARRAY(arr2, arr2_size);
    } /* if */
    return(result);
  } /* arrCat */
#endif



#ifdef ANSI_C

rtlArraytype arrExtend (rtlArraytype arr1, rtlObjecttype element)
#else

rtlArraytype arrExtend (arr1, element)
rtlArraytype arr1;
rtlObjecttype element;
#endif

  {
    memsizetype arr1_size;
    memsizetype result_size;
    rtlArraytype result;

  /* arrExtend */
    arr1_size = arr1->max_position - arr1->min_position + 1;
    result_size = arr1_size + 1;
    result = arr1;
    if (!RESIZE_RTL_ARRAY(result, arr1_size, result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      COUNT3_RTL_ARRAY(arr1_size, result_size);
      result->max_position++;
      result->arr[arr1_size] = element;
    } /* if */
    return(result);
  } /* arrExtend */



#ifdef ANSI_C

rtlArraytype arrGen (rtlObjecttype element1, rtlObjecttype element2)
#else

rtlArraytype arrGen (element1, element2)
rtlObjecttype element1;
rtlObjecttype element2;
#endif

  {
    memsizetype result_size;
    rtlArraytype result;

  /* arrGen */
    result_size = 2;
    if (!ALLOC_RTL_ARRAY(result, result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      COUNT_RTL_ARRAY(result_size);
      result->min_position = 1;
      result->max_position = 2;
      result->arr[0] = element1;
      result->arr[1] = element2;
    } /* if */
    return(result);
  } /* arrGen */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

rtlArraytype arrRange (arraytype arr1, inttype start, inttype stop)
#else

rtlArraytype arrRange (arr1, start, stop)
arraytype arr1;
inttype start;
inttype stop;
#endif

  {
    memsizetype length;
    memsizetype result_size;
    rtlArraytype result;

  /* arrRange */
    length = arr1->max_position - arr1->min_position + 1;
    if (stop >= start && start <= arr1->max_position &&
        stop >= arr1->min_position && length >= 1) {
      if (start < arr1->min_position) {
        start = arr1->min_position;
      } /* if */
      if (stop > arr1->max_position) {
        stop = arr1->max_position;
      } /* if */
      result_size = (memsizetype) (stop - start + 1);
      if (!ALLOC_ARRAY(result, result_size)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } /* if */
      COUNT_ARRAY(result_size);
      result->min_position = arr1->min_position;
      result->max_position = arr1->min_position + result_size - 1;
        memcpy(result->arr, &arr1->arr[(memsizetype) (start - 1)],
            (SIZE_TYPE) (result_size * sizeof(objectrecord)));
        destr_array(arr1->arr, (memsizetype) start - 1);
        destr_array(&arr1->arr[stop], length - (memsizetype) stop);
        FREE_ARRAY(arr1, length);
        arg_1(arguments)->value.arrayvalue = NULL;
    } else {
      if (!ALLOC_ARRAY(result, 0)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } /* if */
      COUNT_ARRAY(0);
      result->min_position = arr1->min_position;
      result->max_position = arr1->min_position - 1;
    } /* if */
    return(result);
  } /* arrRange */
#endif



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

void arrSort (rtlObjecttype begin_sort, rtlObjecttype end_sort,
    inttype cmp_func (rtlObjecttype, rtoObjecttype))
#else

void arrSort (begin_sort, end_sort, cmp_func)
rtlObjecttype begin_sort;
rtlObjecttype end_sort;
inttype cmp_func (rtlObjecttype, rtlObjecttype);
#endif

  {
    rtlObjectrecord compare_elem;
    rtlObjectrecord help_element;
    rtlObjecttype middle_elem;
    rtlObjecttype less_elem;
    rtlObjecttype greater_elem;
    int cmp;

  /* arrSort */
    if (end_sort - begin_sort < 8) {
      for (middle_elem = begin_sort + 1; middle_elem <= end_sort; middle_elem++) {
        memcpy(&compare_elem, middle_elem, sizeof(rtlObjectrecord));
        less_elem = begin_sort - 1;
        do {
          less_elem++;
          cmp = cmp_func(less_elem, &compare_elem);
        } while (cmp < 0);
        memmove(&less_elem[1], less_elem, (middle_elem - less_elem) * sizeof(rtlObjectrecord));
        memcpy(less_elem, &compare_elem, sizeof(rtlObjectrecord));
      } /* for */
    } else {
      middle_elem = &begin_sort[(end_sort - begin_sort) >> 1];
      memcpy(&compare_elem, middle_elem, sizeof(rtlObjectrecord));
      memcpy(middle_elem, end_sort, sizeof(rtlObjectrecord));
      memcpy(end_sort, &compare_elem, sizeof(rtlObjectrecord));
      less_elem = begin_sort - 1;
      greater_elem = end_sort;
      do {
        do {
          less_elem++;
          cmp = cmp_func(less_elem, &compare_elem);
        } while (cmp < 0);
        do {
          greater_elem--;
          cmp = cmp_func(greater_elem, &compare_elem);
        } while (cmp > 0 && greater_elem != begin_sort);
        memcpy(&help_element, less_elem, sizeof(rtlObjectrecord));
        memcpy(less_elem, greater_elem, sizeof(rtlObjectrecord));
        memcpy(greater_elem, &help_element, sizeof(rtlObjectrecord));
      } while (greater_elem > less_elem);
      memcpy(greater_elem, less_elem, sizeof(rtlObjectrecord));
      memcpy(less_elem, &compare_elem, sizeof(rtlObjectrecord));
      memcpy(end_sort, &help_element, sizeof(rtlObjectrecord));
      arrSort(begin_sort, less_elem - 1, cmp_func);
      arrSort(less_elem + 1, end_sort, cmp_func);
    } /* if */
  } /* arrSort */
#endif
