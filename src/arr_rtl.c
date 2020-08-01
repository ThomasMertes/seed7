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
/*  rtlArrayType and rtlObjectType are declared different than the  */
/*  types arrayType and objectType in the interpreter.              */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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



static void rtl_qsort_array (rtlObjectType *begin_sort, rtlObjectType *end_sort,
    compareType cmp_func)

  {
    genericType compare_elem;
    genericType help_element;
    rtlObjectType *middle_elem;
    rtlObjectType *less_elem;
    rtlObjectType *greater_elem;
    intType cmp;

  /* rtl_qsort_array */
    if (end_sort - begin_sort < 8) {
      for (middle_elem = begin_sort + 1; middle_elem <= end_sort; middle_elem++) {
        compare_elem = middle_elem->value.genericValue;
        less_elem = begin_sort - 1;
        do {
          less_elem++;
          cmp = cmp_func(less_elem->value.genericValue, compare_elem);
        } while (cmp < 0);
        memmove(&less_elem[1], less_elem, (memSizeType) (middle_elem - less_elem) * sizeof(rtlObjectType));
        less_elem->value.genericValue = compare_elem;
      } /* for */
    } else {
      middle_elem = &begin_sort[((memSizeType)(end_sort - begin_sort)) >> 1];
      compare_elem = middle_elem->value.genericValue;
      middle_elem->value.genericValue = end_sort->value.genericValue;
      end_sort->value.genericValue = compare_elem;
      less_elem = begin_sort - 1;
      greater_elem = end_sort;
      do {
        do {
          less_elem++;
          cmp = cmp_func(less_elem->value.genericValue, compare_elem);
        } while (cmp < 0);
        do {
          greater_elem--;
          cmp = cmp_func(greater_elem->value.genericValue, compare_elem);
        } while (cmp > 0 && greater_elem != begin_sort);
        help_element = less_elem->value.genericValue;
        less_elem->value.genericValue = greater_elem->value.genericValue;
        greater_elem->value.genericValue = help_element;
      } while (greater_elem > less_elem);
      greater_elem->value.genericValue = less_elem->value.genericValue;
      less_elem->value.genericValue = compare_elem;
      end_sort->value.genericValue = help_element;
      rtl_qsort_array(begin_sort, less_elem - 1, cmp_func);
      rtl_qsort_array(less_elem + 1, end_sort, cmp_func);
    } /* if */
  } /* rtl_qsort_array */



/**
 *  Get the name of the program without path and extension.
 *  @param arg_0 Parameter argv[0] from the function main() as string.
 *  @return the name of the program.
 */
static striType getProgramName (const const_striType arg_0)

  {
    memSizeType name_len;
#ifdef EXECUTABLE_FILE_EXTENSION
    striType exeExtension;
#endif
    intType lastSlashPos;
    striType program_name;

  /* getProgramName */
    logFunction(printf("getProgramName(\"%s\")",
                       striAsUnquotedCStri(arg_0));
                fflush(stdout););
    name_len = arg_0->size;
#ifdef EXECUTABLE_FILE_EXTENSION
    exeExtension = cstri8_or_cstri_to_stri(EXECUTABLE_FILE_EXTENSION);
    if (name_len > exeExtension->size &&
        memcmp(&arg_0->mem[arg_0->size - exeExtension->size],
               exeExtension->mem, exeExtension->size * sizeof(strElemType)) == 0) {
      name_len -= exeExtension->size;
    } /* if */
    FREE_STRI(exeExtension, exeExtension->size);
#endif
    lastSlashPos = strRChPos(arg_0, (charType) '/');
    name_len -= (memSizeType) lastSlashPos;
    if (ALLOC_STRI_SIZE_OK(program_name, name_len)) {
      program_name->size = name_len;
      memcpy(program_name->mem, &arg_0->mem[lastSlashPos],
          name_len * sizeof(strElemType));
    } /* if */
    logFunction(printf(" --> \"%s\"\n",
                       striAsUnquotedCStri(program_name)););
    return program_name;
  } /* getProgramName */



/**
 *  Copy the arguments from argv to an array of strings.
 *  @param argv Parameter from the function main().
 */
static rtlArrayType copyArgv (const int argc, const os_striType *const argv)

  {
    memSizeType arg_c;
    rtlArrayType arg_v;
    memSizeType number;
    errInfoType err_info = OKAY_NO_ERROR;
    striType stri;

  /* copyArgv */
    logFunction(printf("copyArgv(%d, ...)\n", argc););
    if (unlikely(argc < 0)) {
      logError(printf("copyArgv(%d, ...): Argc is negative.\n",
                      argc););
      raise_error(RANGE_ERROR);
      arg_v = NULL;
    } else {
      arg_c = (memSizeType) (argc);
      if (unlikely(!ALLOC_RTL_ARRAY(arg_v, arg_c))) {
        raise_error(MEMORY_ERROR);
      } else {
        arg_v->min_position = 1;
        arg_v->max_position = (intType) (arg_c);
        for (number = 0; number < arg_c; number++) {
          stri = os_stri_to_stri(argv[number], &err_info);
          if (likely(err_info == OKAY_NO_ERROR)) {
            arg_v->arr[number].value.striValue = stri;
          } else {
            while (number >= 1) {
              number--;
              stri = arg_v->arr[number].value.striValue;
              FREE_STRI(stri, stri->size);
            } /* while */
            FREE_RTL_ARRAY(arg_v, arg_c);
            logError(printf("copyArgv(%d, ...): "
                            "os_stri_to_stri(\"" FMT_S_OS "\", *) failed:\n"
                            "err_info=%d\n",
                            argc, argv[number], err_info););
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
rtlArrayType getArgv (const int argc, const wstriType *const argv,
    striType *arg_0, striType *programName, striType *exePath)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    striType arg_0_temp;
    rtlArrayType arg_v;

  /* getArgv */
#ifdef EMULATE_ROOT_CWD
    initEmulatedCwd(&err_info);
    if (err_info != OKAY_NO_ERROR) {
      logError(printf("getArgv(%d, ...): initEmulatedCwd(*) failed:\n"
                      "err_info=%d\n",
                      argc, err_info););
      raise_error(err_info);
      arg_v = NULL;
    } else {
#endif
      arg_0_temp = cp_from_os_path(argv[0], &err_info);
      if (arg_0_temp == NULL) {
        logError(printf("getArgv(%d, ...): "
                        "cp_from_os_path(\"%ls\", *) failed:\n"
                        "err_info=%d\n",
                        argc, argv[0], err_info););
      } else { 
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

#elif defined OS_STRI_WCHAR



rtlArrayType getArgv (const int argc, const cstriType *const argv,
    striType *arg_0, striType *programName, striType *exePath)

  {
    int w_argc;
    os_striType *w_argv;
    errInfoType err_info = OKAY_NO_ERROR;
    striType arg_0_temp;
    rtlArrayType arg_v;

  /* getArgv */
#ifdef EMULATE_ROOT_CWD
    initEmulatedCwd(&err_info);
    if (err_info != OKAY_NO_ERROR) {
      logError(printf("getArgv(%d, ...): initEmulatedCwd(*) failed:\n"
                      "err_info=%d\n",
                      argc, err_info););
      raise_error(err_info);
      arg_v = NULL;
    } else {
#endif
      w_argv = getUtf16Argv(&w_argc);
      if (w_argv == NULL) {
        raise_error(MEMORY_ERROR);
        arg_v = NULL;
      } else {
        arg_0_temp = cp_from_os_path(w_argv[0], &err_info);
        if (arg_0_temp == NULL) {
          logError(printf("getArgv(%d, ...): "
                          "cp_from_os_path(\"" FMT_S_OS "\", *) failed:\n"
                          "err_info=%d\n",
                          argc, w_argv[0], err_info););
        } else {
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
#ifdef EMULATE_ROOT_CWD
    } /* if */
#endif
    return arg_v;
  } /* getArgv */

#else



rtlArrayType getArgv (const int argc, const cstriType *const argv,
    striType *arg_0, striType *programName, striType *exePath)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    striType arg_0_temp;
    rtlArrayType arg_v;

  /* getArgv */
#ifdef EMULATE_ROOT_CWD
    initEmulatedCwd(&err_info);
    if (err_info != OKAY_NO_ERROR) {
      logError(printf("getArgv(%d, ...): initEmulatedCwd(*) failed:\n"
                      "err_info=%d\n",
                      argc, err_info););
      raise_error(err_info);
      arg_v = NULL;
    } else {
#endif
      arg_0_temp = cp_from_os_path(argv[0], &err_info);
      if (arg_0_temp == NULL) {
        logError(printf("getArgv(%d, ...): "
                        "cp_from_os_path(\"%s\", *) failed:\n"
                        "err_info=%d\n",
                        argc, argv[0], err_info););
      } else {
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

#endif



#ifndef OS_STRI_WCHAR
striType examineSearchPath (const const_striType fileName)

  {
    rtlArrayType searchPath;
    memSizeType searchPathSize;
    memSizeType pos;
    striType aPath;
    striType result;

  /* examineSearchPath */
    logFunction(printf("examineSearchPath\n"););
    result = NULL;
    searchPath = cmdGetSearchPath();
    if (searchPath != NULL) {
      searchPathSize = arraySize(searchPath);
      for (pos = 0; result == NULL && pos < searchPathSize; pos++) {
        aPath = searchPath->arr[pos].value.striValue;
        if (aPath->size != 0 && aPath->mem[aPath->size - 1] != (charType) '/') {
          strPush(&aPath, (charType) '/');
        } /* if */
        strAppend(&aPath, fileName);
        if (cmdFileType(aPath) == 2) {
          result = aPath;
        } else {
          FREE_STRI(aPath, aPath->size);
        } /* if */
      } /* for */
      for (; pos < searchPathSize; pos++) {
        aPath = searchPath->arr[pos].value.striValue;
        FREE_STRI(aPath, aPath->size);
      } /* for */
      FREE_RTL_ARRAY(searchPath, searchPathSize);
    } /* if */
    logFunction(printf("examineSearchPath --> \"%s\"\n",
                       striAsUnquotedCStri(result)););
    return result;
  } /* examineSearchPath */
#endif



/**
 *  Append the array 'extension' to the array 'arr_variable'.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             array.
 */
void arrAppend (rtlArrayType *const arr_variable, const rtlArrayType extension)

  {
    rtlArrayType arr_to;
    memSizeType new_size;
    memSizeType arr_to_size;
    memSizeType extension_size;

  /* arrAppend */
    extension_size = arraySize(extension);
    if (extension_size != 0) {
      arr_to = *arr_variable;
      arr_to_size = arraySize(arr_to);
      if (arr_to_size > MAX_RTL_ARR_LEN - extension_size ||
          arr_to->max_position > (intType) (MAX_MEM_INDEX - extension_size)) {
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
              (size_t) (extension_size * sizeof(rtlObjectType)));
          FREE_RTL_ARRAY(extension, extension_size);
        } /* if */
      } /* if */
    } /* if */
  } /* arrAppend */



rtlArrayType arrArrlit2 (intType start_position, rtlArrayType arr1)

  {
    memSizeType result_size;

  /* arrArrlit2 */
    result_size = arraySize(arr1);
    if (start_position < MIN_MEM_INDEX || start_position > MAX_MEM_INDEX ||
        (result_size != 0 && start_position > (intType) (MAX_MEM_INDEX - result_size + 1)) ||
        (result_size == 0 && start_position == MIN_MEM_INDEX)) {
      logError(printf("arrArrlit2(" FMT_D ", arr1 (size=" FMT_U_MEM ")): "
                      "Minimal or maximal index out of range.\n",
                      start_position, result_size););
      raise_error(RANGE_ERROR);
      arr1 = NULL;
    } else {
      arr1->min_position = start_position;
      arr1->max_position = arrayMaxPos(start_position, result_size);
    } /* if */
    return arr1;
  } /* arrArrlit2 */



rtlArrayType arrBaselit (const genericType element)

  {
    memSizeType result_size;
    rtlArrayType result;

  /* arrBaselit */
    result_size = 1;
    if (!ALLOC_RTL_ARRAY(result, result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = 1;
      result->max_position = 1;
      result->arr[0].value.genericValue = element;
    } /* if */
    return result;
  } /* arrBaselit */



rtlArrayType arrBaselit2 (intType start_position, const genericType element)

  {
    memSizeType result_size;
    rtlArrayType result;

  /* arrBaselit2 */
    result_size = 1;
    if (!ALLOC_RTL_ARRAY(result, result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = start_position;
      result->max_position = start_position;
      result->arr[0].value.genericValue = element;
    } /* if */
    return result;
  } /* arrBaselit2 */



/**
 *  Concatenate two arrays.
 *  @return the result of the concatenation.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             array.
 */
rtlArrayType arrCat (rtlArrayType arr1, const rtlArrayType arr2)

  {
    memSizeType arr1_size;
    memSizeType arr2_size;
    memSizeType result_size;
    rtlArrayType result;

  /* arrCat */
    arr1_size = arraySize(arr1);
    arr2_size = arraySize(arr2);
    if (arr1_size > MAX_RTL_ARR_LEN - arr2_size ||
        arr1->max_position > (intType) (MAX_MEM_INDEX - arr2_size)) {
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
        memcpy(&result->arr[arr1_size], arr2->arr, arr2_size * sizeof(rtlObjectType));
        FREE_RTL_ARRAY(arr2, arr2_size);
      } /* if */
    } /* if */
    return result;
  } /* arrCat */



rtlArrayType arrExtend (rtlArrayType arr1, const genericType element)

  {
    memSizeType arr1_size;
    memSizeType result_size;
    rtlArrayType result;

  /* arrExtend */
    arr1_size = arraySize(arr1);
    if (arr1_size > MAX_RTL_ARR_LEN - 1 ||
        arr1->max_position > (intType) (MAX_MEM_INDEX - 1)) {
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
        result->arr[arr1_size].value.genericValue = element;
      } /* if */
    } /* if */
    return result;
  } /* arrExtend */



void arrFree (rtlArrayType oldArray)

  {
    memSizeType size;

  /* arrFree */
    size = arraySize(oldArray);
    FREE_RTL_ARRAY(oldArray, size);
  } /* arrFree */



rtlArrayType arrGen (const genericType element1, const genericType element2)

  {
    memSizeType result_size;
    rtlArrayType result;

  /* arrGen */
    result_size = 2;
    if (!ALLOC_RTL_ARRAY(result, result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = 1;
      result->max_position = 2;
      result->arr[0].value.genericValue = element1;
      result->arr[1].value.genericValue = element2;
    } /* if */
    return result;
  } /* arrGen */



/**
 *  Get a sub array ending at the position 'stop'.
 *  @return the sub array ending at the stop position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlArrayType arrHead (const const_rtlArrayType arr1, intType stop)

  {
    memSizeType length;
    memSizeType result_size;
    rtlArrayType result;

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
            (size_t) (result_size * sizeof(rtlObjectType)));
      } /* if */
    } else if (arr1->min_position == MIN_MEM_INDEX) {
      logError(printf("arrHead(arr1 (size=" FMT_U_MEM "), " FMT_D "): "
                      "Cannot create empty array with minimal index.\n",
                      length, stop););
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
rtlArrayType arrHeadTemp (rtlArrayType *arr_temp, intType stop)

  {
    rtlArrayType arr1;
    memSizeType length;
    memSizeType result_size;
    rtlArrayType new_arr1;
    rtlArrayType result;

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
              (size_t) ((length - result_size) * sizeof(rtlObjectType)));
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
      logError(printf("arrHeadTemp(arr1 (size=" FMT_U_MEM "), " FMT_D "): "
                      "Cannot create empty array with minimal index.\n",
                      length, stop););
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
genericType arrIdxTemp (rtlArrayType *arr_temp, intType position)

  {
    rtlArrayType arr1;
    memSizeType length;
    rtlArrayType resized_arr1;
    genericType result;

  /* arrIdxTemp */
    arr1 = *arr_temp;
    if (position >= arr1->min_position && position <= arr1->max_position) {
      length = arraySize(arr1);
      result = arr1->arr[position - arr1->min_position].value.genericValue;
      if (position != arr1->max_position) {
        arr1->arr[position - arr1->min_position].value.genericValue =
            arr1->arr[length - 1].value.genericValue;
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
      logError(printf("arrIdxTemp(arr1, " FMT_D "): "
                      "Index out of range (" FMT_D " .. " FMT_D ").\n",
                      position, arr1->min_position, arr1->max_position););
      raise_error(RANGE_ERROR);
      result = 0;
    } /* if */
    return result;
  } /* arrIdxTemp */



rtlArrayType arrMalloc (intType min_position, intType max_position)

  {
    memSizeType size;
    rtlArrayType result;

  /* arrMalloc */
    if (min_position < MIN_MEM_INDEX || max_position > MAX_MEM_INDEX ||
        (min_position == MIN_MEM_INDEX && max_position == MAX_MEM_INDEX) ||
        (min_position > MIN_MEM_INDEX && min_position - 1 > max_position)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      size = arraySize2(min_position, max_position);
      if (size > MAX_RTL_ARR_LEN ||
          !ALLOC_RTL_ARRAY(result, (memSizeType) size)) {
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
void arrPush (rtlArrayType *const arr_variable, const genericType element)

  {
    rtlArrayType arr_to;
    memSizeType new_size;
    memSizeType arr_to_size;

  /* arrPush */
    arr_to = *arr_variable;
    arr_to_size = arraySize(arr_to);
    if (arr_to_size > MAX_RTL_ARR_LEN - 1 ||
        arr_to->max_position > (intType) (MAX_MEM_INDEX - 1)) {
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
        arr_to->arr[arr_to_size].value.genericValue = element;
      } /* if */
    } /* if */
  } /* arrPush */



/**
 *  Get a sub array from the position 'start' to the position 'stop'.
 *  @return the sub array from position 'start' to 'stop'.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlArrayType arrRange (const const_rtlArrayType arr1, intType start, intType stop)

  {
    memSizeType length;
    memSizeType result_size;
    memSizeType start_idx;
    rtlArrayType result;

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
            (size_t) (result_size * sizeof(rtlObjectType)));
      } /* if */
    } else if (arr1->min_position == MIN_MEM_INDEX) {
      logError(printf("arrRange(arr1 (size=" FMT_U_MEM "), " FMT_D ", " FMT_D "): "
                      "Cannot create empty array with minimal index.\n",
                      length, start, stop););
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
rtlArrayType arrRangeTemp (rtlArrayType *arr_temp, intType start, intType stop)

  {
    rtlArrayType arr1;
    memSizeType length;
    memSizeType result_size;
    memSizeType start_idx;
    memSizeType stop_idx;
    rtlArrayType resized_arr1;
    rtlArrayType result;

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
              (size_t) (result_size * sizeof(rtlObjectType)));
          memmove(&arr1->arr[start_idx], &arr1->arr[stop_idx + 1],
              (size_t) ((length - stop_idx - 1) * sizeof(rtlObjectType)));
          resized_arr1 = REALLOC_RTL_ARRAY(arr1, length, length - result_size);
          if (resized_arr1 == NULL) {
            memcpy(&arr1->arr[length - result_size], result->arr,
                (size_t) (result_size * sizeof(rtlObjectType)));
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
      logError(printf("arrRangeTemp(arr1 (size=" FMT_U_MEM "), " FMT_D ", " FMT_D "): "
                      "Cannot create empty array with minimal index.\n",
                      length, start, stop););
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



rtlArrayType arrRealloc (rtlArrayType arr, memSizeType oldSize, memSizeType newSize)

  {
    rtlArrayType resized_arr;

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
genericType arrRemove (rtlArrayType *arr_to, intType position)

  {
    rtlArrayType arr1;
    rtlArrayType resized_arr1;
    rtlObjectType *array_pointer;
    memSizeType arr1_size;
    genericType result;

  /* arrRemove */
    arr1 = *arr_to;
    if (position >= arr1->min_position && position <= arr1->max_position) {
      array_pointer = arr1->arr;
      result = array_pointer[position - arr1->min_position].value.genericValue;
      memmove(&array_pointer[position - arr1->min_position],
          &array_pointer[position - arr1->min_position + 1],
          (arraySize2(position, arr1->max_position) - 1) * sizeof(rtlObjectType));
      arr1_size = arraySize(arr1);
      resized_arr1 = REALLOC_RTL_ARRAY(arr1, arr1_size, arr1_size - 1);
      if (unlikely(resized_arr1 == NULL)) {
        /* A realloc, which shrinks memory, usually succeeds. */
        /* The probability that this code path is executed is */
        /* probably zero. The code below restores the old     */
        /* value of arr1.                                     */
        memmove(&array_pointer[position - arr1->min_position + 1],
            &array_pointer[position - arr1->min_position],
            (arraySize2(position, arr1->max_position) - 1) * sizeof(rtlObjectType));
        array_pointer[position - arr1->min_position].value.genericValue = result;
        raise_error(MEMORY_ERROR);
        result = 0;
      } else {
        arr1 = resized_arr1;
        COUNT3_RTL_ARRAY(arr1_size, arr1_size - 1);
        arr1->max_position--;
        *arr_to = arr1;
      } /* if */
    } else {
      logError(printf("arrRemove(arr1, " FMT_D "): "
                      "Index out of range (" FMT_D " .. " FMT_D ").\n",
                      position, arr1->min_position, arr1->max_position););
      raise_error(RANGE_ERROR);
      result = 0;
    } /* if */
    return result;
  } /* arrRemove */



rtlArrayType arrSort (rtlArrayType arr1, compareType cmp_func)

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
rtlArrayType arrSubarr (const const_rtlArrayType arr1, intType start, intType len)

  {
    memSizeType length;
    memSizeType result_size;
    memSizeType start_idx;
    rtlArrayType result;

  /* arrSubarr */
    length = arraySize(arr1);
    if (len >= 1 && start <= arr1->max_position && length >= 1 &&
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
      if (!ALLOC_RTL_ARRAY(result, result_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arrayMaxPos(arr1->min_position, result_size);
        start_idx = arrayIndex(arr1, start);
        memcpy(result->arr, &arr1->arr[start_idx],
            (size_t) (result_size * sizeof(rtlObjectType)));
      } /* if */
    } else if (arr1->min_position == MIN_MEM_INDEX) {
      logError(printf("arrSubarr(arr1 (size=" FMT_U_MEM "), " FMT_D ", " FMT_D "): "
                      "Cannot create empty array with minimal index.\n",
                      length, start, len););
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
rtlArrayType arrSubarrTemp (rtlArrayType *arr_temp, intType start, intType len)

  {
    rtlArrayType arr1;
    memSizeType length;
    memSizeType result_size;
    memSizeType start_idx;
    memSizeType stop_idx;
    rtlArrayType resized_arr1;
    rtlArrayType result;

  /* arrSubarrTemp */
    arr1 = *arr_temp;
    length = arraySize(arr1);
    if (len >= 1 && start <= arr1->max_position && length >= 1 &&
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
              (size_t) (result_size * sizeof(rtlObjectType)));
          memmove(&arr1->arr[start_idx], &arr1->arr[stop_idx + 1],
              (size_t) ((length - stop_idx - 1) * sizeof(rtlObjectType)));
          resized_arr1 = REALLOC_RTL_ARRAY(arr1, length, length - result_size);
          if (resized_arr1 == NULL) {
            memcpy(&arr1->arr[length - result_size], result->arr,
                (size_t) (result_size * sizeof(rtlObjectType)));
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
      logError(printf("arrSubarrTemp(arr1 (size=" FMT_U_MEM "), " FMT_D ", " FMT_D "): "
                      "Cannot create empty array with minimal index.\n",
                      length, start, len););
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
rtlArrayType arrTail (const const_rtlArrayType arr1, intType start)

  {
    memSizeType length;
    memSizeType result_size;
    memSizeType start_idx;
    rtlArrayType result;

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
            (size_t) (result_size * sizeof(rtlObjectType)));
      } /* if */
    } else if (arr1->min_position == MIN_MEM_INDEX) {
      logError(printf("arrTail(arr1 (size=" FMT_U_MEM "), " FMT_D "): "
                      "Cannot create empty array with minimal index.\n",
                      length, start););
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
rtlArrayType arrTailTemp (rtlArrayType *arr_temp, intType start)

  {
    rtlArrayType arr1;
    memSizeType length;
    memSizeType result_size;
    memSizeType start_idx;
    rtlArrayType resized_arr1;
    rtlArrayType result;

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
              (size_t) (result_size * sizeof(rtlObjectType)));
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
      logError(printf("arrTailTemp(arr1 (size=" FMT_U_MEM "), " FMT_D "): "
                      "Cannot create empty array with minimal index.\n",
                      length, start););
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
