/********************************************************************/
/*                                                                  */
/*  arr_rtl.c     Primitive actions for the array type.             */
/*  Copyright (C) 1989 - 2007  Thomas Mertes                        */
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
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/arr_rtl.c                                       */
/*  Changes: 1991, 1992, 1993, 1994, 2005, 2006  Thomas Mertes      */
/*  Content: Primitive actions for the array type.                  */
/*                                                                  */
/*  The functions from this file should only be used in compiled    */
/*  Seed7 programs. The interpreter should not use functions of     */
/*  this file.                                                      */
/*                                                                  */
/*  The functions in this file use type declarations from the       */
/*  include file data_rtl.h instead of data.h. Therefore the types  */
/*  rtlArraytype and rtlObjecttype are declared different than the  */
/*  types arraytype and objecttype in the interpreter.              */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "str_rtl.h"
#include "cmd_rtl.h"
#include "cmd_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "arr_rtl.h"


rtlArraytype strChSplit (const const_stritype main_stri, const chartype delimiter);



#ifdef ANSI_C

static void rtl_qsort_array (rtlObjecttype *begin_sort, rtlObjecttype *end_sort,
    inttype cmp_func (rtlGenerictype, rtlGenerictype))
#else

static void rtl_qsort_array (begin_sort, end_sort, cmp_func)
rtlObjecttype *begin_sort;
rtlObjecttype *end_sort;
inttype cmp_func (rtlGenerictype, rtlGenerictype);
#endif

  {
    rtlGenerictype compare_elem;
    rtlGenerictype help_element;
    rtlObjecttype *middle_elem;
    rtlObjecttype *less_elem;
    rtlObjecttype *greater_elem;
    inttype cmp;

  /* rtl_qsort_array */
    if (end_sort - begin_sort < 8) {
      for (middle_elem = begin_sort + 1; middle_elem <= end_sort; middle_elem++) {
        compare_elem = middle_elem->value.genericvalue;
        less_elem = begin_sort - 1;
        do {
          less_elem++;
          cmp = cmp_func(less_elem->value.genericvalue, compare_elem);
        } while (cmp < 0);
        memmove(&less_elem[1], less_elem, (memsizetype) (middle_elem - less_elem) * sizeof(rtlObjecttype));
        less_elem->value.genericvalue = compare_elem;
      } /* for */
    } else {
      middle_elem = &begin_sort[((memsizetype)(end_sort - begin_sort)) >> 1];
      compare_elem = middle_elem->value.genericvalue;
      middle_elem->value.genericvalue = end_sort->value.genericvalue;
      end_sort->value.genericvalue = compare_elem;
      less_elem = begin_sort - 1;
      greater_elem = end_sort;
      do {
        do {
          less_elem++;
          cmp = cmp_func(less_elem->value.genericvalue, compare_elem);
        } while (cmp < 0);
        do {
          greater_elem--;
          cmp = cmp_func(greater_elem->value.genericvalue, compare_elem);
        } while (cmp > 0 && greater_elem != begin_sort);
        help_element = less_elem->value.genericvalue;
        less_elem->value.genericvalue = greater_elem->value.genericvalue;
        greater_elem->value.genericvalue = help_element;
      } while (greater_elem > less_elem);
      greater_elem->value.genericvalue = less_elem->value.genericvalue;
      less_elem->value.genericvalue = compare_elem;
      end_sort->value.genericvalue = help_element;
      rtl_qsort_array(begin_sort, less_elem - 1, cmp_func);
      rtl_qsort_array(less_elem + 1, end_sort, cmp_func);
    } /* if */
  } /* rtl_qsort_array */



#ifdef ANSI_C

static rtlArraytype copyArgv (const int argc, const os_stritype *const argv)
#else

static rtlArraytype copyArgv (argc, argv)
int argc;
os_stritype *argv;
#endif

  {
    memsizetype arg_c;
    rtlArraytype arg_v;
    memsizetype number;
    stritype stri;

  /* copyArgv */
    if (unlikely(argc < 0)) {
      raise_error(RANGE_ERROR);
      arg_v = NULL;
    } else {
      arg_c = (memsizetype) (argc);
      if (unlikely(!ALLOC_RTL_ARRAY(arg_v, arg_c))) {
        raise_error(MEMORY_ERROR);
      } else {
        arg_v->min_position = 1;
        arg_v->max_position = (inttype) (arg_c);
        for (number = 0; number < arg_c; number++) {
          stri = os_stri_to_stri(argv[number]);
          if (stri != NULL) {
            arg_v->arr[number].value.strivalue = stri;
          } else {
            while (number >= 1) {
              number--;
              stri = arg_v->arr[number].value.strivalue;
              FREE_STRI(stri, stri->size);
            } /* while */
            FREE_RTL_ARRAY(arg_v, arg_c);
            raise_error(MEMORY_ERROR);
            arg_v = NULL;
            number = arg_c; /* leave for-loop */
          } /* if */
        } /* for */
      } /* if */
    } /* if */
    return arg_v;
  } /* copyArgv */



#ifdef USE_WMAIN
#ifdef ANSI_C

rtlArraytype getArgv (const int argc, const wstritype *const argv, stritype *arg_0,
    stritype *exePath)
#else

rtlArraytype getArgv (argc, argv, arg_0, exePath)
int argc;
os_stritype *argv;
stritype *arg_0;
stritype *exePath;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    rtlArraytype arg_v;

  /* getArgv */
    if (arg_0 != NULL) {
      *arg_0 = cp_from_os_path(argv[0]);
      if (*arg_0 == NULL) {
        err_info = MEMORY_ERROR;
      } /* if */
    } /* if */
    if (exePath != NULL && arg_0 != NULL && *arg_0 != NULL) {
      *exePath = getExecutablePath(*arg_0);
      if (*exePath == NULL) {
        err_info = MEMORY_ERROR;
      } /* if */
    } /* if */
    if (err_info == OKAY_NO_ERROR) {
      arg_v = copyArgv(argc - 1, &argv[1]);
    } else {
      raise_error(MEMORY_ERROR);
      arg_v = NULL;
    } /* if */
    return arg_v;
  } /* getArgv */

#else



#ifdef ANSI_C

rtlArraytype getArgv (const int argc, const cstritype *const argv, stritype *arg_0,
    stritype *exePath)
#else

rtlArraytype getArgv (argc, argv, arg_0, exePath)
int argc;
char **argv;
stritype *arg_0;
stritype *exePath;
#endif

  {
#ifdef OS_PATH_WCHAR
    int w_argc;
    os_stritype *w_argv;
#endif
    errinfotype err_info = OKAY_NO_ERROR;
    rtlArraytype arg_v;

  /* getArgv */
#ifdef OS_PATH_WCHAR
    w_argv = getUtf16Argv(&w_argc);
    if (w_argv == NULL) {
      raise_error(MEMORY_ERROR);
      arg_v = NULL;
    } else {
      if (arg_0 != NULL) {
        *arg_0 = cp_from_os_path(w_argv[0]);
        if (*arg_0 == NULL) {
          err_info = MEMORY_ERROR;
        } /* if */
      } /* if */
      if (exePath != NULL && arg_0 != NULL && *arg_0 != NULL) {
        *exePath = getExecutablePath(*arg_0);
        if (*exePath == NULL) {
          err_info = MEMORY_ERROR;
        } /* if */
      } /* if */
      if (err_info == OKAY_NO_ERROR) {
        arg_v = copyArgv(w_argc - 1, &w_argv[1]);
      } else {
        raise_error(err_info);
        arg_v = NULL;
      } /* if */
      freeUtf16Argv(w_argv);
    } /* if */
#else
    if (arg_0 != NULL) {
      *arg_0 = cp_from_os_path(argv[0]);
      if (*arg_0 == NULL) {
        err_info = MEMORY_ERROR;
      } /* if */
    } /* if */
    if (exePath != NULL && arg_0 != NULL && *arg_0 != NULL) {
      *exePath = getExecutablePath(*arg_0);
      if (*exePath == NULL) {
        err_info = MEMORY_ERROR;
      } /* if */
    } /* if */
    if (err_info == OKAY_NO_ERROR) {
      arg_v = copyArgv(argc - 1, &argv[1]);
    } else {
      raise_error(err_info);
      arg_v = NULL;
    } /* if */
#endif
    return arg_v;
  } /* getArgv */

#endif



#ifndef OS_PATH_WCHAR
#ifdef ANSI_C

stritype examineSearchPath (const const_stritype fileName)
#else

stritype examineSearchPath (fileName)
stritype fileName;
#endif

  {
    os_stritype env_value;
    stritype pathStri;
    rtlArraytype searchPath;
    memsizetype arraySize;
    memsizetype pos;
    stritype path;
    stritype result;

  /* examineSearchPath */
    result = NULL;
    env_value = getenv("PATH");
    if (env_value != NULL) {
      pathStri = os_stri_to_stri(env_value);
      searchPath = strChSplit(pathStri, (chartype) ':');
      if (searchPath != NULL) {
        arraySize = (uinttype) (searchPath->max_position - searchPath->min_position + 1);
        for (pos = 0; result == NULL && pos < arraySize; pos++) {
          path = searchPath->arr[pos].value.strivalue;
          while (path->size > 1 && path->mem[path->size - 1] == (chartype) '/') {
            path->size--;
          } /* while */
          if (path->size != 0 && path->mem[path->size - 1] != (chartype) '/') {
            strPush(&path, (chartype) '/');
          } /* if */
          strAppend(&path, fileName);
          if (cmdFileType(path) == 2) {
            result = path;
          } else {
            FREE_STRI(path, path->size);
          } /* if */
        } /* for */
        FREE_RTL_ARRAY(searchPath, arraySize);
      } /* if */
    } /* if */
    return result;
  } /* examineSearchPath */
#endif



#ifdef ANSI_C

void arrAppend (rtlArraytype *const arr_variable, const const_rtlArraytype arr_from)
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
    arr_from_size = (uinttype) (arr_from->max_position - arr_from->min_position + 1);
    if (arr_from_size != 0) {
      arr_to = *arr_variable;
      arr_to_size = (uinttype) (arr_to->max_position - arr_to->min_position + 1);
      if (arr_to->max_position > (inttype) (MAX_MEM_INDEX - arr_from_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        new_size = arr_to_size + arr_from_size;
        arr_to = REALLOC_RTL_ARRAY(arr_to, arr_to_size, new_size);
        if (arr_to == NULL) {
          raise_error(MEMORY_ERROR);
        } else {
          COUNT3_RTL_ARRAY(arr_to_size, new_size);
          *arr_variable = arr_to;
          arr_to->max_position += (inttype) arr_from_size;
          memcpy(&arr_to->arr[arr_to_size], arr_from->arr,
              (size_t) (arr_from_size * sizeof(rtlObjecttype)));
          FREE_RTL_ARRAY(arr_from, arr_from_size);
        } /* if */
      } /* if */
    } /* if */
  } /* arrAppend */



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
    result_size = (uinttype) (arr1->max_position - arr1->min_position + 1);
    arr1->min_position = start_position;
    arr1->max_position = (inttype) ((memsizetype) start_position + result_size - 1);
    return arr1;
  } /* arrArrlit2 */



#ifdef ANSI_C

rtlArraytype arrBaselit (const rtlGenerictype element)
#else

rtlArraytype arrBaselit (element)
rtlGenerictype element;
#endif

  {
    memsizetype result_size;
    rtlArraytype result;

  /* arrBaselit */
    result_size = 1;
    if (!ALLOC_RTL_ARRAY(result, result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = 1;
      result->max_position = 1;
      result->arr[0].value.genericvalue = element;
    } /* if */
    return result;
  } /* arrBaselit */



#ifdef ANSI_C

rtlArraytype arrBaselit2 (inttype start_position, const rtlGenerictype element)
#else

rtlArraytype arrBaselit2 (start_position, element)
inttype start_position;
rtlGenerictype element;
#endif

  {
    memsizetype result_size;
    rtlArraytype result;

  /* arrBaselit2 */
    result_size = 1;
    if (!ALLOC_RTL_ARRAY(result, result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = start_position;
      result->max_position = start_position;
      result->arr[0].value.genericvalue = element;
    } /* if */
    return result;
  } /* arrBaselit2 */



#ifdef ANSI_C

rtlArraytype arrCat (rtlArraytype arr1, const_rtlArraytype arr2)
#else

rtlArraytype arrCat (arr1, arr2)
rtlArraytype arr1;
rtlArraytype arr2;
#endif

  {
    memsizetype arr1_size;
    memsizetype arr2_size;
    memsizetype result_size;
    rtlArraytype result;

  /* arrCat */
    arr1_size = (uinttype) (arr1->max_position - arr1->min_position + 1);
    arr2_size = (uinttype) (arr2->max_position - arr2->min_position + 1);
    if (arr1->max_position > (inttype) (MAX_MEM_INDEX - arr2_size)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = arr1_size + arr2_size;
      result = REALLOC_RTL_ARRAY(arr1, arr1_size, result_size);
      if (result == NULL) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_RTL_ARRAY(arr1_size, result_size);
        result->max_position += (inttype) arr2_size;
        memcpy(&result->arr[arr1_size], arr2->arr, arr2_size * sizeof(rtlObjecttype));
        FREE_RTL_ARRAY(arr2, arr2_size);
      } /* if */
    } /* if */
    return result;
  } /* arrCat */



#ifdef ANSI_C

rtlArraytype arrExtend (rtlArraytype arr1, const rtlGenerictype element)
#else

rtlArraytype arrExtend (arr1, element)
rtlArraytype arr1;
rtlGenerictype element;
#endif

  {
    memsizetype arr1_size;
    memsizetype result_size;
    rtlArraytype result;

  /* arrExtend */
    arr1_size = (uinttype) (arr1->max_position - arr1->min_position + 1);
    if (arr1->max_position > (inttype) (MAX_MEM_INDEX - 1)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = arr1_size + 1;
      result = REALLOC_RTL_ARRAY(arr1, arr1_size, result_size);
      if (result == NULL) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_RTL_ARRAY(arr1_size, result_size);
        result->max_position++;
        result->arr[arr1_size].value.genericvalue = element;
      } /* if */
    } /* if */
    return result;
  } /* arrExtend */



#ifdef ANSI_C

rtlArraytype arrGen (const rtlGenerictype element1, const rtlGenerictype element2)
#else

rtlArraytype arrGen (element1, element2)
rtlGenerictype element1;
rtlGenerictype element2;
#endif

  {
    memsizetype result_size;
    rtlArraytype result;

  /* arrGen */
    result_size = 2;
    if (!ALLOC_RTL_ARRAY(result, result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = 1;
      result->max_position = 2;
      result->arr[0].value.genericvalue = element1;
      result->arr[1].value.genericvalue = element2;
    } /* if */
    return result;
  } /* arrGen */



#ifdef ANSI_C

rtlArraytype arrHead (const const_rtlArraytype arr1, inttype stop)
#else

rtlArraytype arrHead (arr1, stop)
rtlArraytype arr1;
inttype stop;
#endif

  {
    memsizetype length;
    memsizetype result_size;
    rtlArraytype result;

  /* arrHead */
    length = (uinttype) (arr1->max_position - arr1->min_position + 1);
    if (stop >= arr1->min_position && length >= 1) {
      if (stop > arr1->max_position) {
        stop = arr1->max_position;
      } /* if */
      result_size = (uinttype) (stop - arr1->min_position + 1);
      if (!ALLOC_RTL_ARRAY(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = stop;
        memcpy(result->arr, arr1->arr,
            (size_t) (result_size * sizeof(rtlObjecttype)));
      } /* if */
    } else {
      if (!ALLOC_RTL_ARRAY(result, 0)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    return result;
  } /* arrHead */



#ifdef ANSI_C

rtlArraytype arrHeadTemp (rtlArraytype *arr_temp, inttype stop)
#else

rtlArraytype arrHeadTemp (arr_temp, stop)
rtlArraytype *arr_temp;
inttype stop;
#endif

  {
    rtlArraytype arr1;
    memsizetype length;
    memsizetype result_size;
    rtlArraytype new_arr1;
    rtlArraytype result;

  /* arrHeadTemp */
    arr1 = *arr_temp;
    length = (uinttype) (arr1->max_position - arr1->min_position + 1);
    if (stop >= arr1->min_position && length >= 1) {
      if (stop > arr1->max_position) {
        stop = arr1->max_position;
      } /* if */
      result_size = (uinttype) (stop - arr1->min_position + 1);
      if (!ALLOC_RTL_ARRAY(new_arr1, length - result_size)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        new_arr1->min_position = stop + 1;
        new_arr1->max_position = arr1->max_position;
        memcpy(new_arr1->arr, &arr1->arr[result_size],
            (size_t) ((length - result_size) * sizeof(rtlObjecttype)));
        result = REALLOC_RTL_ARRAY(arr1, length, result_size);
        if (result == NULL) {
          FREE_RTL_ARRAY(new_arr1, length - result_size);
          raise_error(MEMORY_ERROR);
        } else {
          COUNT3_RTL_ARRAY(length, result_size);
          result->max_position = stop;
          *arr_temp = new_arr1;
        } /* if */
      } /* if */
    } else {
      if (!ALLOC_RTL_ARRAY(result, 0)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    return result;
  } /* arrHeadTemp */



/**
 *  Index access when the array is destroyed after indexing.
 *  To avoid problems the indexed element is removed from the array.
 */
#ifdef ANSI_C

rtlGenerictype arrIdxTemp (rtlArraytype *arr_temp, inttype pos)
#else

rtlGenerictype arrIdxTemp (arr_temp, pos)
rtlArraytype *arr_temp;
inttype pos;
#endif

  {
    rtlArraytype arr1;
    memsizetype length;
    rtlArraytype resized_arr1;
    rtlGenerictype result;

  /* arrIdxTemp */
    arr1 = *arr_temp;
    if (pos >= arr1->min_position && pos <= arr1->max_position) {
      length = (uinttype) (arr1->max_position - arr1->min_position + 1);
      result = arr1->arr[pos - arr1->min_position].value.genericvalue;
      if (pos != arr1->max_position) {
        arr1->arr[pos - arr1->min_position].value.genericvalue =
            arr1->arr[length - 1].value.genericvalue;
      } /* if */
      resized_arr1 = REALLOC_RTL_ARRAY(arr1, length, length - 1);
      if (resized_arr1 == NULL) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_RTL_ARRAY(length, length - 1);
        resized_arr1->max_position--;
        *arr_temp = resized_arr1;
      } /* if */
    } else {
      raise_error(RANGE_ERROR);
      result = 0;
    } /* if */
    return result;
  } /* arrIdxTemp */



#ifdef ANSI_C

void arrPush (rtlArraytype *arr_variable, const rtlGenerictype element)
#else

void arrPush (arr_variable, element)
rtlArraytype *arr_variable;
rtlGenerictype element;
#endif

  {
    rtlArraytype arr_to;
    memsizetype new_size;
    memsizetype arr_to_size;

  /* arrPush */
    arr_to = *arr_variable;
    arr_to_size = (uinttype) (arr_to->max_position - arr_to->min_position + 1);
    if (arr_to->max_position > (inttype) (MAX_MEM_INDEX - 1)) {
      raise_error(MEMORY_ERROR);
    } else {
      new_size = arr_to_size + 1;
      arr_to = REALLOC_RTL_ARRAY(arr_to, arr_to_size, new_size);
      if (arr_to == NULL) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_RTL_ARRAY(arr_to_size, new_size);
        *arr_variable = arr_to;
        arr_to->max_position ++;
        arr_to->arr[arr_to_size].value.genericvalue = element;
      } /* if */
    } /* if */
  } /* arrPush */



#ifdef ANSI_C

rtlArraytype arrRange (const const_rtlArraytype arr1, inttype start, inttype stop)
#else

rtlArraytype arrRange (arr1, start, stop)
rtlArraytype arr1;
inttype start;
inttype stop;
#endif

  {
    memsizetype length;
    memsizetype result_size;
    memsizetype start_idx;
    rtlArraytype result;

  /* arrRange */
    length = (uinttype) (arr1->max_position - arr1->min_position + 1);
    if (stop >= start && start <= arr1->max_position &&
        stop >= arr1->min_position && length >= 1) {
      if (start < arr1->min_position) {
        start = arr1->min_position;
      } /* if */
      if (stop > arr1->max_position) {
        stop = arr1->max_position;
      } /* if */
      result_size = (uinttype) (stop - start + 1);
      if (!ALLOC_RTL_ARRAY(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = (inttype) ((memsizetype) arr1->min_position + result_size - 1);
        start_idx = (uinttype) (start - arr1->min_position);
        memcpy(result->arr, &arr1->arr[start_idx],
            (size_t) (result_size * sizeof(rtlObjecttype)));
      } /* if */
    } else {
      if (!ALLOC_RTL_ARRAY(result, 0)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    return result;
  } /* arrRange */



#ifdef ANSI_C

rtlArraytype arrRangeTemp (rtlArraytype *arr_temp, inttype start, inttype stop)
#else

rtlArraytype arrRangeTemp (arr_temp, start, stop)
rtlArraytype *arr_temp;
inttype start;
inttype stop;
#endif

  {
    rtlArraytype arr1;
    memsizetype length;
    memsizetype result_size;
    memsizetype start_idx;
    memsizetype stop_idx;
    rtlArraytype resized_arr1;
    rtlArraytype result;

  /* arrRangeTemp */
    arr1 = *arr_temp;
    length = (uinttype) (arr1->max_position - arr1->min_position + 1);
    if (stop >= start && start <= arr1->max_position &&
        stop >= arr1->min_position && length >= 1) {
      if (start < arr1->min_position) {
        start = arr1->min_position;
      } /* if */
      if (stop > arr1->max_position) {
        stop = arr1->max_position;
      } /* if */
      result_size = (uinttype) (stop - start + 1);
      if (!ALLOC_RTL_ARRAY(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = (inttype) ((memsizetype) arr1->min_position + result_size - 1);
        start_idx = (uinttype) (start - arr1->min_position);
        stop_idx = (uinttype) (stop - arr1->min_position);
        memcpy(result->arr, &arr1->arr[start_idx],
            (size_t) (result_size * sizeof(rtlObjecttype)));
        memmove(&arr1->arr[start_idx], &arr1->arr[stop_idx + 1],
            (size_t) ((length - stop_idx - 1) * sizeof(rtlObjecttype)));
        resized_arr1 = REALLOC_RTL_ARRAY(arr1, length, length - result_size);
        if (resized_arr1 == NULL) {
          memcpy(&arr1->arr[length - result_size], result->arr,
              (size_t) (result_size * sizeof(rtlObjecttype)));
          FREE_RTL_ARRAY(result, result_size);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          COUNT3_RTL_ARRAY(length, length - result_size);
          resized_arr1->max_position -= (inttype) result_size;
          *arr_temp = resized_arr1;
        } /* if */
      } /* if */
    } else {
      if (!ALLOC_RTL_ARRAY(result, 0)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    return result;
  } /* arrRangeTemp */



#ifdef ANSI_C

rtlGenerictype arrRemove (rtlArraytype *arr_to, inttype position)
#else

rtlGenerictype arrRemove (arr_to, position)
rtlArraytype *arr_to;
inttype position;
#endif

  {
    rtlArraytype arr1;
    rtlArraytype resized_arr1;
    rtlObjecttype *array_pointer;
    memsizetype arr1_size;
    rtlGenerictype result;

  /* arrRemove */
    arr1 = *arr_to;
    if (position >= arr1->min_position && position <= arr1->max_position) {
      array_pointer = arr1->arr;
      result = array_pointer[position - arr1->min_position].value.genericvalue;
      memmove(&array_pointer[position - arr1->min_position],
          &array_pointer[position - arr1->min_position + 1],
          (uinttype) (arr1->max_position - position) * sizeof(rtlObjecttype));
      arr1->max_position--;
      arr1_size = (uinttype) (arr1->max_position - arr1->min_position + 1);
      resized_arr1 = REALLOC_RTL_ARRAY(arr1, arr1_size + 1, arr1_size);
      if (resized_arr1 == NULL) {
        raise_error(MEMORY_ERROR);
        result = 0;
      } else {
        arr1 = resized_arr1;
        COUNT3_RTL_ARRAY(arr1_size + 1, arr1_size);
        *arr_to = arr1;
      } /* if */
    } else {
      raise_error(RANGE_ERROR);
      result = 0;
    } /* if */
    return result;
  } /* arrRemove */



#ifdef ANSI_C

rtlArraytype arrSort (rtlArraytype arr1, inttype cmp_func (rtlGenerictype, rtlGenerictype))
#else

rtlArraytype arrSort (arr1, cmp_func)
rtlArraytype arr1;
inttype cmp_func (rtlGenerictype, rtlGenerictype);
#endif

  { /* arrSort */
    /* printf("arrSort(%lX, %ld, %ld)\n", arr1, arr1->min_position, arr1->max_position); */
    rtl_qsort_array(arr1->arr, &arr1->arr[arr1->max_position - arr1->min_position], cmp_func);
    return arr1;
  } /* arrSort */



#ifdef ANSI_C

rtlArraytype arrTail (const const_rtlArraytype arr1, inttype start)
#else

rtlArraytype arrTail (arr1, start)
rtlArraytype arr1;
inttype start;
#endif

  {
    memsizetype length;
    memsizetype result_size;
    memsizetype start_idx;
    rtlArraytype result;

  /* arrTail */
    length = (uinttype) (arr1->max_position - arr1->min_position + 1);
    if (start <= arr1->max_position && length >= 1) {
      if (start < arr1->min_position) {
        start = arr1->min_position;
      } /* if */
      result_size = (uinttype) (arr1->max_position - start + 1);
      if (!ALLOC_RTL_ARRAY(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = (inttype) ((memsizetype) arr1->min_position + result_size - 1);
        start_idx = (uinttype) (start - arr1->min_position);
        memcpy(result->arr, &arr1->arr[start_idx],
            (size_t) (result_size * sizeof(rtlObjecttype)));
      } /* if */
    } else {
      if (!ALLOC_RTL_ARRAY(result, 0)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    return result;
  } /* arrTail */



#ifdef ANSI_C

rtlArraytype arrTailTemp (rtlArraytype *arr_temp, inttype start)
#else

rtlArraytype arrTailTemp (arr_temp, start)
rtlArraytype *arr_temp;
inttype start;
#endif

  {
    rtlArraytype arr1;
    memsizetype length;
    memsizetype result_size;
    memsizetype start_idx;
    rtlArraytype resized_arr1;
    rtlArraytype result;

  /* arrTailTemp */
    arr1 = *arr_temp;
    length = (uinttype) (arr1->max_position - arr1->min_position + 1);
    if (start <= arr1->max_position && length >= 1) {
      if (start < arr1->min_position) {
        start = arr1->min_position;
      } /* if */
      result_size = (uinttype) (arr1->max_position - start + 1);
      if (!ALLOC_RTL_ARRAY(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = (inttype) ((memsizetype) arr1->min_position + result_size - 1);
        start_idx = (uinttype) (start - arr1->min_position);
        memcpy(result->arr, &arr1->arr[start_idx],
            (size_t) (result_size * sizeof(rtlObjecttype)));
        resized_arr1 = REALLOC_RTL_ARRAY(arr1, length, length - result_size);
        if (resized_arr1 == NULL) {
          FREE_RTL_ARRAY(result, result_size);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          COUNT3_RTL_ARRAY(length, length - result_size);
          resized_arr1->max_position = start - 1;
          *arr_temp = resized_arr1;
        } /* if */
      } /* if */
    } else {
      if (!ALLOC_RTL_ARRAY(result, 0)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    return result;
  } /* arrTailTemp */
