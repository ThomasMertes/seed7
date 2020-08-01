/********************************************************************/
/*                                                                  */
/*  arr_rtl.c     Primitive actions for the array type.             */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
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
/*  Changes: 1991 - 1994, 2005, 2006, 2013  Thomas Mertes           */
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
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "str_rtl.h"
#include "cmd_rtl.h"
#include "cmd_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "arr_rtl.h"



static void rtl_qsort_array (rtlObjecttype *begin_sort, rtlObjecttype *end_sort,
    inttype cmp_func (generictype, generictype))

  {
    generictype compare_elem;
    generictype help_element;
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



/**
 *  Get the name of the program without path and extension.
 *  @param arg_0 Parameter argv[0] from the function main() as string.
 *  @return the name of the program.
 */
static stritype getProgramName (const const_stritype arg_0)

  {
    memsizetype name_len;
#ifdef EXECUTABLE_FILE_EXTENSION
    stritype exeExtension;
#endif
    inttype lastSlashPos;
    stritype program_name;

  /* getProgramName */
    name_len = arg_0->size;
#ifdef EXECUTABLE_FILE_EXTENSION
    exeExtension = cstri8_or_cstri_to_stri(EXECUTABLE_FILE_EXTENSION);
    if (name_len > exeExtension->size &&
        memcmp(&arg_0->mem[arg_0->size - exeExtension->size],
               exeExtension->mem, exeExtension->size * sizeof(strelemtype)) == 0) {
      name_len -= exeExtension->size;
    } /* if */
    FREE_STRI(exeExtension, exeExtension->size);
#endif
    lastSlashPos = strRChPos(arg_0, (chartype) '/');
    name_len -= (memsizetype) lastSlashPos;
    if (ALLOC_STRI_SIZE_OK(program_name, name_len)) {
      program_name->size = name_len;
      memcpy(program_name->mem, &arg_0->mem[lastSlashPos],
          name_len * sizeof(strelemtype));
    } /* if */
    return program_name;
  } /* getProgramName */



/**
 *  Copy the arguments from argv to an array of strings.
 *  @param argv Parameter from the function main().
 */
static rtlArraytype copyArgv (const int argc, const os_stritype *const argv)

  {
    memsizetype arg_c;
    rtlArraytype arg_v;
    memsizetype number;
    errinfotype err_info = OKAY_NO_ERROR;
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
          stri = os_stri_to_stri(argv[number], &err_info);
          if (likely(err_info == OKAY_NO_ERROR)) {
            arg_v->arr[number].value.strivalue = stri;
          } else {
            while (number >= 1) {
              number--;
              stri = arg_v->arr[number].value.strivalue;
              FREE_STRI(stri, stri->size);
            } /* while */
            FREE_RTL_ARRAY(arg_v, arg_c);
            raise_error(err_info);
            arg_v = NULL;
            number = arg_c; /* leave for-loop */
          } /* if */
        } /* for */
      } /* if */
    } /* if */
    return arg_v;
  } /* copyArgv */



/**
 *  Get the Seed7 argument vector of a program.
 *  The name of the program is not part of the Seed7 argument vector.
 *  @param argc Parameter from the function main().
 *  @param argv Parameter from the function main().
 *  @param arg_0 NULL or address to which argv[0] is copied.
 *  @param programName NULL or address to which the program name
 *         (without path and extension) is copied.
 *  @param exePath NULL or address to which the absolute path of the
 *         executable is copied (symbolic links of the path are resolved).
 *  @return an array with the argument vector.
 */
#ifdef USE_WMAIN
rtlArraytype getArgv (const int argc, const wstritype *const argv, stritype *arg_0,
    stritype *programName, stritype *exePath)

  {
    errinfotype err_info = OKAY_NO_ERROR;
    stritype arg_0_temp;
    rtlArraytype arg_v;

  /* getArgv */
#ifdef EMULATE_ROOT_CWD
    initEmulatedCwd(&err_info);
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
      arg_v = NULL;
    } else {
#endif
      arg_0_temp = cp_from_os_path(argv[0], &err_info);
      if (arg_0_temp != NULL) {
        if (exePath != NULL) {
          *exePath = getExecutablePath(arg_0_temp);
          if (*exePath == NULL) {
            err_info = MEMORY_ERROR;
          } /* if */
        } /* if */
        if (programName != NULL) {
          *programName = getProgramName(arg_0_temp);
          if (*programName == NULL) {
            err_info = MEMORY_ERROR;
          } /* if */
        } /* if */
        if (arg_0 != NULL) {
          *arg_0 = arg_0_temp;
        } else {
          FREE_STRI(arg_0_temp, arg_0_temp->size);
        } /* if */
      } /* if */
      if (err_info == OKAY_NO_ERROR) {
        arg_v = copyArgv(argc - 1, &argv[1]);
      } else {
        raise_error(err_info);
        arg_v = NULL;
      } /* if */
#ifdef EMULATE_ROOT_CWD
    } /* if */
#endif
    return arg_v;
  } /* getArgv */

#else



rtlArraytype getArgv (const int argc, const cstritype *const argv, stritype *arg_0,
    stritype *programName, stritype *exePath)

  {
#ifdef OS_STRI_WCHAR
    int w_argc;
    os_stritype *w_argv;
#endif
    errinfotype err_info = OKAY_NO_ERROR;
    stritype arg_0_temp;
    rtlArraytype arg_v;

  /* getArgv */
#ifdef EMULATE_ROOT_CWD
    initEmulatedCwd(&err_info);
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
      arg_v = NULL;
    } else {
#endif
#ifdef OS_STRI_WCHAR
      w_argv = getUtf16Argv(&w_argc);
      if (w_argv == NULL) {
        raise_error(MEMORY_ERROR);
        arg_v = NULL;
      } else {
        arg_0_temp = cp_from_os_path(w_argv[0], &err_info);
        if (arg_0_temp != NULL) {
          if (exePath != NULL) {
            *exePath = getExecutablePath(arg_0_temp);
            if (*exePath == NULL) {
              err_info = MEMORY_ERROR;
            } /* if */
          } /* if */
          if (programName != NULL) {
            *programName = getProgramName(arg_0_temp);
            if (*programName == NULL) {
              err_info = MEMORY_ERROR;
            } /* if */
          } /* if */
          if (arg_0 != NULL) {
            *arg_0 = arg_0_temp;
          } else {
            FREE_STRI(arg_0_temp, arg_0_temp->size);
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
      arg_0_temp = cp_from_os_path(argv[0], &err_info);
      if (arg_0_temp != NULL) {
        if (exePath != NULL) {
          *exePath = getExecutablePath(arg_0_temp);
          if (*exePath == NULL) {
            err_info = MEMORY_ERROR;
          } /* if */
        } /* if */
        if (programName != NULL) {
          *programName = getProgramName(arg_0_temp);
          if (*programName == NULL) {
            err_info = MEMORY_ERROR;
          } /* if */
        } /* if */
        if (arg_0 != NULL) {
          *arg_0 = arg_0_temp;
        } else {
          FREE_STRI(arg_0_temp, arg_0_temp->size);
        } /* if */
      } /* if */
      if (err_info == OKAY_NO_ERROR) {
        arg_v = copyArgv(argc - 1, &argv[1]);
      } else {
        raise_error(err_info);
        arg_v = NULL;
      } /* if */
#endif
#ifdef EMULATE_ROOT_CWD
    } /* if */
#endif
    return arg_v;
  } /* getArgv */

#endif



#ifndef OS_STRI_WCHAR
stritype examineSearchPath (const const_stritype fileName)

  {
    rtlArraytype searchPath;
    memsizetype searchPathSize;
    memsizetype pos;
    stritype aPath;
    stritype result;

  /* examineSearchPath */
    /* printf("examineSearchPath\n"); */
    result = NULL;
    searchPath = cmdGetSearchPath();
    if (searchPath != NULL) {
      searchPathSize = arraySize(searchPath);
      for (pos = 0; result == NULL && pos < searchPathSize; pos++) {
        aPath = searchPath->arr[pos].value.strivalue;
        if (aPath->size != 0 && aPath->mem[aPath->size - 1] != (chartype) '/') {
          strPush(&aPath, (chartype) '/');
        } /* if */
        strAppend(&aPath, fileName);
        if (cmdFileType(aPath) == 2) {
          result = aPath;
        } else {
          FREE_STRI(aPath, aPath->size);
        } /* if */
      } /* for */
      for (; pos < searchPathSize; pos++) {
        aPath = searchPath->arr[pos].value.strivalue;
        FREE_STRI(aPath, aPath->size);
      } /* for */
      FREE_RTL_ARRAY(searchPath, searchPathSize);
    } /* if */
    /* printf("examineSearchPath --> ");
       prot_stri(result);
       printf("\n"); */
    return result;
  } /* examineSearchPath */
#endif



/**
 *  Append the array 'extension' to the array 'arr_variable'.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             array.
 */
void arrAppend (rtlArraytype *const arr_variable, const rtlArraytype extension)

  {
    rtlArraytype arr_to;
    memsizetype new_size;
    memsizetype arr_to_size;
    memsizetype extension_size;

  /* arrAppend */
    extension_size = arraySize(extension);
    if (extension_size != 0) {
      arr_to = *arr_variable;
      arr_to_size = arraySize(arr_to);
      if (arr_to_size > MAX_RTL_ARR_LEN - extension_size ||
          arr_to->max_position > (inttype) (MAX_MEM_INDEX - extension_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        new_size = arr_to_size + extension_size;
        arr_to = REALLOC_RTL_ARRAY(arr_to, arr_to_size, new_size);
        if (arr_to == NULL) {
          raise_error(MEMORY_ERROR);
        } else {
          COUNT3_RTL_ARRAY(arr_to_size, new_size);
          *arr_variable = arr_to;
          arr_to->max_position = arrayMaxPos(arr_to->min_position, new_size);
          memcpy(&arr_to->arr[arr_to_size], extension->arr,
              (size_t) (extension_size * sizeof(rtlObjecttype)));
          FREE_RTL_ARRAY(extension, extension_size);
        } /* if */
      } /* if */
    } /* if */
  } /* arrAppend */



rtlArraytype arrArrlit2 (inttype start_position, rtlArraytype arr1)

  {
    memsizetype result_size;

  /* arrArrlit2 */
    result_size = arraySize(arr1);
    if (start_position < MIN_MEM_INDEX || start_position > MAX_MEM_INDEX ||
        (result_size != 0 && start_position > (inttype) (MAX_MEM_INDEX - result_size + 1)) ||
        (result_size == 0 && start_position == MIN_MEM_INDEX)) {
      raise_error(RANGE_ERROR);
      arr1 = NULL;
    } else {
      arr1->min_position = start_position;
      arr1->max_position = arrayMaxPos(start_position, result_size);
    } /* if */
    return arr1;
  } /* arrArrlit2 */



rtlArraytype arrBaselit (const generictype element)

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



rtlArraytype arrBaselit2 (inttype start_position, const generictype element)

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



/**
 *  Concatenate two arrays.
 *  @return the result of the concatenation.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             array.
 */
rtlArraytype arrCat (rtlArraytype arr1, const rtlArraytype arr2)

  {
    memsizetype arr1_size;
    memsizetype arr2_size;
    memsizetype result_size;
    rtlArraytype result;

  /* arrCat */
    arr1_size = arraySize(arr1);
    arr2_size = arraySize(arr2);
    if (arr1_size > MAX_RTL_ARR_LEN - arr2_size ||
        arr1->max_position > (inttype) (MAX_MEM_INDEX - arr2_size)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = arr1_size + arr2_size;
      result = REALLOC_RTL_ARRAY(arr1, arr1_size, result_size);
      if (result == NULL) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_RTL_ARRAY(arr1_size, result_size);
        result->max_position = arrayMaxPos(result->min_position, result_size);
        memcpy(&result->arr[arr1_size], arr2->arr, arr2_size * sizeof(rtlObjecttype));
        FREE_RTL_ARRAY(arr2, arr2_size);
      } /* if */
    } /* if */
    return result;
  } /* arrCat */



rtlArraytype arrExtend (rtlArraytype arr1, const generictype element)

  {
    memsizetype arr1_size;
    memsizetype result_size;
    rtlArraytype result;

  /* arrExtend */
    arr1_size = arraySize(arr1);
    if (arr1_size > MAX_RTL_ARR_LEN - 1 ||
        arr1->max_position > (inttype) (MAX_MEM_INDEX - 1)) {
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



void arrFree (rtlArraytype oldArray)

  {
    memsizetype size;

  /* arrFree */
    size = arraySize(oldArray);
    FREE_RTL_ARRAY(oldArray, size);
  } /* arrFree */



rtlArraytype arrGen (const generictype element1, const generictype element2)

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



/**
 *  Get a sub array ending at the position 'stop'.
 *  @return the sub array ending at the stop position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlArraytype arrHead (const const_rtlArraytype arr1, inttype stop)

  {
    memsizetype length;
    memsizetype result_size;
    rtlArraytype result;

  /* arrHead */
    length = arraySize(arr1);
    if (stop >= arr1->min_position && length >= 1) {
      if (stop > arr1->max_position) {
        stop = arr1->max_position;
      } /* if */
      result_size = arraySize2(arr1->min_position, stop);
      if (!ALLOC_RTL_ARRAY(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = stop;
        memcpy(result->arr, arr1->arr,
            (size_t) (result_size * sizeof(rtlObjecttype)));
      } /* if */
    } else if (arr1->min_position == MIN_MEM_INDEX) {
      raise_error(RANGE_ERROR);
      result = NULL;
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



/**
 *  Get a sub array ending at the position 'stop'.
 *  ArrHeadTemp is used by the compiler when 'arr_temp' is temporary
 *  value that can be reused.
 *  @return the sub array ending at the stop position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlArraytype arrHeadTemp (rtlArraytype *arr_temp, inttype stop)

  {
    rtlArraytype arr1;
    memsizetype length;
    memsizetype result_size;
    rtlArraytype new_arr1;
    rtlArraytype result;

  /* arrHeadTemp */
    arr1 = *arr_temp;
    length = arraySize(arr1);
    if (stop >= arr1->min_position && length >= 1) {
      if (stop >= arr1->max_position) {
        result = arr1;
        *arr_temp = NULL;
      } else {
        result_size = arraySize2(arr1->min_position, stop);
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
      } /* if */
    } else if (arr1->min_position == MIN_MEM_INDEX) {
      raise_error(RANGE_ERROR);
      result = NULL;
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
 *  Access one element from the array 'arr_temp'.
 *  The compiler uses this function when the array is destroyed
 *  after the indexing. To avoid problems the indexed element is
 *  removed from the array.
 *  @return the element with the specified 'position' from 'arr_temp'.
 *  @exception RANGE_ERROR When 'position' is less than minIdx(arr) or
 *                         greater than maxIdx(arr)
 */
generictype arrIdxTemp (rtlArraytype *arr_temp, inttype position)

  {
    rtlArraytype arr1;
    memsizetype length;
    rtlArraytype resized_arr1;
    generictype result;

  /* arrIdxTemp */
    arr1 = *arr_temp;
    if (position >= arr1->min_position && position <= arr1->max_position) {
      length = arraySize(arr1);
      result = arr1->arr[position - arr1->min_position].value.genericvalue;
      if (position != arr1->max_position) {
        arr1->arr[position - arr1->min_position].value.genericvalue =
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



rtlArraytype arrMalloc (inttype min_position, inttype max_position)

  {
    memsizetype size;
    rtlArraytype result;

  /* arrMalloc */
    if (min_position < MIN_MEM_INDEX || max_position > MAX_MEM_INDEX ||
        (min_position == MIN_MEM_INDEX && max_position == MAX_MEM_INDEX) ||
        (min_position > MIN_MEM_INDEX && min_position - 1 > max_position)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      size = arraySize2(min_position, max_position);
      if (size > MAX_RTL_ARR_LEN ||
          !ALLOC_RTL_ARRAY(result, (memsizetype) size)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->min_position = min_position;
        result->max_position = max_position;
      } /* if */
    } /* if */
    return result;
  } /* arrMalloc */



/**
 *  Append the given 'element' to the array 'arr_variable'.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             array.
 */
void arrPush (rtlArraytype *const arr_variable, const generictype element)

  {
    rtlArraytype arr_to;
    memsizetype new_size;
    memsizetype arr_to_size;

  /* arrPush */
    arr_to = *arr_variable;
    arr_to_size = arraySize(arr_to);
    if (arr_to_size > MAX_RTL_ARR_LEN - 1 ||
        arr_to->max_position > (inttype) (MAX_MEM_INDEX - 1)) {
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



/**
 *  Get a sub array from the position 'start' to the position 'stop'.
 *  @return the sub array from position 'start' to 'stop'.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlArraytype arrRange (const const_rtlArraytype arr1, inttype start, inttype stop)

  {
    memsizetype length;
    memsizetype result_size;
    memsizetype start_idx;
    rtlArraytype result;

  /* arrRange */
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
      if (!ALLOC_RTL_ARRAY(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arrayMaxPos(arr1->min_position, result_size);
        start_idx = arrayIndex(arr1, start);
        memcpy(result->arr, &arr1->arr[start_idx],
            (size_t) (result_size * sizeof(rtlObjecttype)));
      } /* if */
    } else if (arr1->min_position == MIN_MEM_INDEX) {
      raise_error(RANGE_ERROR);
      result = NULL;
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



/**
 *  Get a sub array from the position 'start' to the position 'stop'.
 *  ArrRangeTemp is used by the compiler when 'arr_temp' is temporary
 *  value that can be reused.
 *  @return the sub array from position 'start' to 'stop'.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlArraytype arrRangeTemp (rtlArraytype *arr_temp, inttype start, inttype stop)

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
      if (result_size == length) {
        result = arr1;
        *arr_temp = NULL;
      } else {
        if (!ALLOC_RTL_ARRAY(result, result_size)) {
          raise_error(MEMORY_ERROR);
        } else {
          result->min_position = arr1->min_position;
          result->max_position = arrayMaxPos(arr1->min_position, result_size);
          start_idx = arrayIndex(arr1, start);
          stop_idx = arrayIndex(arr1, stop);
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
            resized_arr1->max_position = arrayMaxPos(resized_arr1->min_position, length - result_size);
            *arr_temp = resized_arr1;
          } /* if */
        } /* if */
      } /* if */
    } else if (arr1->min_position == MIN_MEM_INDEX) {
      raise_error(RANGE_ERROR);
      result = NULL;
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



rtlArraytype arrRealloc (rtlArraytype arr, memsizetype oldSize, memsizetype newSize)

  {
    rtlArraytype resized_arr;

  /* arrRealloc */
    resized_arr = REALLOC_RTL_ARRAY(arr, oldSize, newSize);
    if (unlikely(resized_arr == NULL)) {
      if (oldSize >= newSize) {
        resized_arr = arr;
      } /* if */
      raise_error(MEMORY_ERROR);
    } else {
      COUNT3_RTL_ARRAY(oldSize, newSize);
    } /* if */
    return resized_arr;
  } /* arrRealloc */



/**
 *  Remove the element with 'position' from 'arr_to' and return the removed element.
 *  @return the removed element.
 *  @exception RANGE_ERROR When 'position' is less than minIdx(arr) or
 *                         greater than maxIdx(arr)
 */
generictype arrRemove (rtlArraytype *arr_to, inttype position)

  {
    rtlArraytype arr1;
    rtlArraytype resized_arr1;
    rtlObjecttype *array_pointer;
    memsizetype arr1_size;
    generictype result;

  /* arrRemove */
    arr1 = *arr_to;
    if (position >= arr1->min_position && position <= arr1->max_position) {
      array_pointer = arr1->arr;
      result = array_pointer[position - arr1->min_position].value.genericvalue;
      memmove(&array_pointer[position - arr1->min_position],
          &array_pointer[position - arr1->min_position + 1],
          (arraySize2(position, arr1->max_position) - 1) * sizeof(rtlObjecttype));
      arr1_size = arraySize(arr1);
      resized_arr1 = REALLOC_RTL_ARRAY(arr1, arr1_size, arr1_size - 1);
      if (unlikely(resized_arr1 == NULL)) {
        /* A realloc, which shrinks memory, usually succeeds. */
        /* The probability that this code path is executed is */
        /* probably zero. The code below restores the old     */
        /* value of arr1.                                     */
        memmove(&array_pointer[position - arr1->min_position + 1],
            &array_pointer[position - arr1->min_position],
            (arraySize2(position, arr1->max_position) - 1) * sizeof(rtlObjecttype));
        array_pointer[position - arr1->min_position].value.genericvalue = result;
        raise_error(MEMORY_ERROR);
        result = 0;
      } else {
        arr1 = resized_arr1;
        COUNT3_RTL_ARRAY(arr1_size, arr1_size - 1);
        arr1->max_position--;
        *arr_to = arr1;
      } /* if */
    } else {
      raise_error(RANGE_ERROR);
      result = 0;
    } /* if */
    return result;
  } /* arrRemove */



rtlArraytype arrSort (rtlArraytype arr1, inttype cmp_func (generictype, generictype))

  { /* arrSort */
    /* printf("arrSort(%lX, %ld, %ld)\n", arr1, arr1->min_position, arr1->max_position); */
    rtl_qsort_array(arr1->arr, &arr1->arr[arr1->max_position - arr1->min_position], cmp_func);
    return arr1;
  } /* arrSort */



/**
 *  Get a sub array from the position 'start' with maximum length 'len'.
 *  @return the sub array from position 'start' with maximum length 'len'.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlArraytype arrSubarr (const const_rtlArraytype arr1, inttype start, inttype len)

  {
    memsizetype length;
    memsizetype result_size;
    memsizetype start_idx;
    rtlArraytype result;

  /* arrSubarr */
    length = arraySize(arr1);
    if (len >= 1 && start <= arr1->max_position && length >= 1 &&
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
      if (!ALLOC_RTL_ARRAY(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arrayMaxPos(arr1->min_position, result_size);
        start_idx = arrayIndex(arr1, start);
        memcpy(result->arr, &arr1->arr[start_idx],
            (size_t) (result_size * sizeof(rtlObjecttype)));
      } /* if */
    } else if (arr1->min_position == MIN_MEM_INDEX) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (!ALLOC_RTL_ARRAY(result, 0)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    return result;
  } /* arrSubarr */



/**
 *  Get a sub array from the position 'start' with maximum length 'len'.
 *  ArrSubarrTemp is used by the compiler when 'arr_temp' is temporary
 *  value that can be reused.
 *  @return the sub array from position 'start' with maximum length 'len'.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlArraytype arrSubarrTemp (rtlArraytype *arr_temp, inttype start, inttype len)

  {
    rtlArraytype arr1;
    memsizetype length;
    memsizetype result_size;
    memsizetype start_idx;
    memsizetype stop_idx;
    rtlArraytype resized_arr1;
    rtlArraytype result;

  /* arrSubarrTemp */
    arr1 = *arr_temp;
    length = arraySize(arr1);
    if (len >= 1 && start <= arr1->max_position && length >= 1 &&
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
      if (result_size == length) {
        result = arr1;
        *arr_temp = NULL;
      } else {
        if (!ALLOC_RTL_ARRAY(result, result_size)) {
          raise_error(MEMORY_ERROR);
        } else {
          result->min_position = arr1->min_position;
          result->max_position = arrayMaxPos(arr1->min_position, result_size);
          start_idx = arrayIndex(arr1, start);
          stop_idx = arrayIndex(arr1, start + len - 1);
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
            resized_arr1->max_position = arrayMaxPos(resized_arr1->min_position, length - result_size);
            *arr_temp = resized_arr1;
          } /* if */
        } /* if */
      } /* if */
    } else if (arr1->min_position == MIN_MEM_INDEX) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (!ALLOC_RTL_ARRAY(result, 0)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    return result;
  } /* arrSubarrTemp */



/**
 *  Get a sub array beginning at the position 'start'.
 *  @return the sub array beginning at the start position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlArraytype arrTail (const const_rtlArraytype arr1, inttype start)

  {
    memsizetype length;
    memsizetype result_size;
    memsizetype start_idx;
    rtlArraytype result;

  /* arrTail */
    length = arraySize(arr1);
    if (start <= arr1->max_position && length >= 1) {
      if (start < arr1->min_position) {
        start = arr1->min_position;
      } /* if */
      result_size = arraySize2(start, arr1->max_position);
      if (!ALLOC_RTL_ARRAY(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arrayMaxPos(arr1->min_position, result_size);
        start_idx = arrayIndex(arr1, start);
        memcpy(result->arr, &arr1->arr[start_idx],
            (size_t) (result_size * sizeof(rtlObjecttype)));
      } /* if */
    } else if (arr1->min_position == MIN_MEM_INDEX) {
      raise_error(RANGE_ERROR);
      result = NULL;
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



/**
 *  Get a sub array beginning at the position 'start'.
 *  ArrTailTemp is used by the compiler when 'arr_temp' is temporary
 *  value that can be reused.
 *  @return the sub array beginning at the start position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlArraytype arrTailTemp (rtlArraytype *arr_temp, inttype start)

  {
    rtlArraytype arr1;
    memsizetype length;
    memsizetype result_size;
    memsizetype start_idx;
    rtlArraytype resized_arr1;
    rtlArraytype result;

  /* arrTailTemp */
    arr1 = *arr_temp;
    length = arraySize(arr1);
    if (start <= arr1->max_position && length >= 1) {
      if (start <= arr1->min_position) {
        result = arr1;
        *arr_temp = NULL;
      } else {
        result_size = arraySize2(start, arr1->max_position);
        if (!ALLOC_RTL_ARRAY(result, result_size)) {
          raise_error(MEMORY_ERROR);
        } else {
          result->min_position = arr1->min_position;
          result->max_position = arrayMaxPos(arr1->min_position, result_size);
          start_idx = arrayIndex(arr1, start);
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
      } /* if */
    } else if (arr1->min_position == MIN_MEM_INDEX) {
      raise_error(RANGE_ERROR);
      result = NULL;
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
