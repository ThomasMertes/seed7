/********************************************************************/
/*                                                                  */
/*  arr_rtl.c     Primitive actions for the array type.             */
/*  Copyright (C) 1989 - 2016, 2018, 2019  Thomas Mertes            */
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


#define QSORT_LIMIT 8



#if ANY_LOG_ACTIVE
#ifdef USE_WMAIN
static void printArgv (const int argc, const wstriType *const argv)
#else
static void printArgv (const int argc, const cstriType *const argv)
#endif

  {
    int pos;

  /* printArgv */
    if (argv == NULL) {
      printf("NULL");
    } else {
      printf("{");
      for (pos = 0; pos <= argc; pos++) {
        if (pos != 0) {
          printf(", ");
        } /* if */
#ifdef USE_WMAIN
        printf("\"" FMT_S_OS "\"", argv[pos]);
#else
        printf("\"%s\"", argv[pos]);
#endif
      } /* for */
      printf("}");
    }
  } /* printArgv */
#endif



/**
 *  Sort an array of 'rtlObjectType' elements with the quicksort algorithm.
 *  In contrast to qsort() this function uses a different compare function.
 *  The compare function of qsort() has two void pointers as parameters.
 *  @param begin_sort Pointer to first element to be sorted.
 *  @param end_sort Pointer to the last element to be sorted.
 *  @param cmp_func Pointer to a compare function that gets two values as
 *         'genericType' and compares them.
 */
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
    if (end_sort - begin_sort < QSORT_LIMIT) {
      /* Use insertion sort */
      for (middle_elem = begin_sort + 1; middle_elem <= end_sort; middle_elem++) {
        compare_elem = middle_elem->value.genericValue;
        less_elem = begin_sort - 1;
        do {
          less_elem++;
          cmp = cmp_func(less_elem->value.genericValue, compare_elem);
        } while (cmp < 0);
        memmove(&less_elem[1], less_elem, (memSizeType)
                (middle_elem - less_elem) * sizeof(rtlObjectType));
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
#if defined LINKED_PROGRAM_EXTENSION || defined EXECUTABLE_FILE_EXTENSION
    striType exeExtension;
#endif
    intType lastSlashPos;
    striType program_name;

  /* getProgramName */
    logFunction(printf("getProgramName(\"%s\")",
                       striAsUnquotedCStri(arg_0));
                fflush(stdout););
    name_len = arg_0->size;
#if defined LINKED_PROGRAM_EXTENSION || defined EXECUTABLE_FILE_EXTENSION
#ifdef LINKED_PROGRAM_EXTENSION
    exeExtension = CSTRI_LITERAL_TO_STRI(LINKED_PROGRAM_EXTENSION);
#else
    exeExtension = CSTRI_LITERAL_TO_STRI(EXECUTABLE_FILE_EXTENSION);
#endif
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
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(program_name)););
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
          if (likely(stri != NULL)) {
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
    logFunction(printf("copyArgv -->\n"););
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
    logFunction(printf("getArgv(%d, ", argc);
                printArgv(argc, argv);
                printf(", %s", arg_0 != NULL ? "*" : "NULL");
                printf(", %s", programName != NULL ? "*" : "NULL");
                printf(", %s)\n", exePath != NULL ? "*" : "NULL"););
#if EMULATE_ROOT_CWD
    initEmulatedCwd(&err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("getArgv(%d, ...): initEmulatedCwd(*) failed:\n"
                      "err_info=%d\n",
                      argc, err_info););
      raise_error(err_info);
      arg_v = NULL;
    } else
#endif
    {
      arg_0_temp = cp_from_os_path(argv[0], &err_info);
      if (unlikely(arg_0_temp == NULL)) {
        logError(printf("getArgv(%d, ...): "
                        "cp_from_os_path(\"%ls\", *) failed:\n"
                        "err_info=%d\n",
                        argc, argv[0], err_info););
      } else {
        if (programName != NULL) {
          *programName = getProgramName(arg_0_temp);
          if (*programName == NULL) {
            err_info = MEMORY_ERROR;
          } /* if */
        } /* if */
        if (exePath != NULL) {
          *exePath = getExecutablePath(arg_0_temp);
          if (*exePath == NULL) {
            err_info = MEMORY_ERROR;
          } /* if */
        } /* if */
        if (arg_0 != NULL) {
          *arg_0 = arg_0_temp;
        } else {
          FREE_STRI(arg_0_temp, arg_0_temp->size);
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
        arg_v = NULL;
      } else {
        arg_v = copyArgv(argc - 1, &argv[1]);
      } /* if */
    }
    logFunction(printf("getArgv(%d, ", argc);
                printArgv(argc, argv);
                printf(", \"%s\"",
                       striAsUnquotedCStri(arg_0 != NULL ? *arg_0 : NULL));
                printf(", \"%s\"",
                       striAsUnquotedCStri(programName != NULL ? *programName : NULL));
                printf(", \"%s\") -->\n",
                       striAsUnquotedCStri(exePath != NULL ? *exePath : NULL)));
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
    logFunction(printf("getArgv(%d, ", argc);
                printArgv(argc, argv);
                printf(", %s", arg_0 != NULL ? "*" : "NULL");
                printf(", %s", programName != NULL ? "*" : "NULL");
                printf(", %s)\n", exePath != NULL ? "*" : "NULL"););
#if EMULATE_ROOT_CWD
    initEmulatedCwd(&err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("getArgv(%d, ...): initEmulatedCwd(*) failed:\n"
                      "err_info=%d\n",
                      argc, err_info););
      raise_error(err_info);
      arg_v = NULL;
    } else
#endif
    {
      w_argv = getUtf16Argv(&w_argc);
      if (unlikely(w_argv == NULL)) {
        raise_error(MEMORY_ERROR);
        arg_v = NULL;
      } else {
        arg_0_temp = cp_from_os_path(w_argv[0], &err_info);
        if (unlikely(arg_0_temp == NULL)) {
          logError(printf("getArgv(%d, ...): "
                          "cp_from_os_path(\"" FMT_S_OS "\", *) failed:\n"
                          "err_info=%d\n",
                          argc, w_argv[0], err_info););
        } else {
          if (programName != NULL) {
            *programName = getProgramName(arg_0_temp);
            if (*programName == NULL) {
              err_info = MEMORY_ERROR;
            } /* if */
          } /* if */
          if (exePath != NULL) {
            *exePath = getExecutablePath(arg_0_temp);
            if (*exePath == NULL) {
              err_info = MEMORY_ERROR;
            } /* if */
          } /* if */
          if (arg_0 != NULL) {
            *arg_0 = arg_0_temp;
          } else {
            FREE_STRI(arg_0_temp, arg_0_temp->size);
          } /* if */
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
          arg_v = NULL;
        } else {
          arg_v = copyArgv(w_argc - 1, &w_argv[1]);
        } /* if */
        freeUtf16Argv(w_argv);
      } /* if */
    }
    logFunction(printf("getArgv(%d, ", argc);
                printArgv(argc, argv);
                printf(", \"%s\"",
                       striAsUnquotedCStri(arg_0 != NULL ? *arg_0 : NULL));
                printf(", \"%s\"",
                       striAsUnquotedCStri(programName != NULL ? *programName : NULL));
                printf(", \"%s\") -->\n",
                       striAsUnquotedCStri(exePath != NULL ? *exePath : NULL)));
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
    logFunction(printf("getArgv(%d, ", argc);
                printArgv(argc, argv);
                printf(", %s", arg_0 != NULL ? "*" : "NULL");
                printf(", %s", programName != NULL ? "*" : "NULL");
                printf(", %s)\n", exePath != NULL ? "*" : "NULL"););
#if EMULATE_ROOT_CWD
    initEmulatedCwd(&err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("getArgv(%d, ...): initEmulatedCwd(*) failed:\n"
                      "err_info=%d\n",
                      argc, err_info););
      raise_error(err_info);
      arg_v = NULL;
    } else
#endif
    {
      arg_0_temp = cp_from_os_path(argv[0], &err_info);
      if (unlikely(arg_0_temp == NULL)) {
        logError(printf("getArgv(%d, ...): "
                        "cp_from_os_path(\"%s\", *) failed:\n"
                        "err_info=%d\n",
                        argc, argv[0], err_info););
      } else {
        if (programName != NULL) {
          *programName = getProgramName(arg_0_temp);
          if (*programName == NULL) {
            err_info = MEMORY_ERROR;
          } /* if */
        } /* if */
        if (exePath != NULL) {
          *exePath = getExecutablePath(arg_0_temp);
          if (*exePath == NULL) {
            err_info = MEMORY_ERROR;
          } /* if */
        } /* if */
        if (arg_0 != NULL) {
          *arg_0 = arg_0_temp;
        } else {
          FREE_STRI(arg_0_temp, arg_0_temp->size);
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
        arg_v = NULL;
      } else {
        arg_v = copyArgv(argc - 1, &argv[1]);
      } /* if */
    }
    logFunction(printf("getArgv(%d, ", argc);
                printArgv(argc, argv);
                printf(", \"%s\"",
                       striAsUnquotedCStri(arg_0 != NULL ? *arg_0 : NULL));
                printf(", \"%s\"",
                       striAsUnquotedCStri(programName != NULL ? *programName : NULL));
                printf(", \"%s\") -->\n",
                       striAsUnquotedCStri(exePath != NULL ? *exePath : NULL)));
    return arg_v;
  } /* getArgv */

#endif



#ifndef OS_STRI_WCHAR
/**
 *  Examine all directories in the search path and look for fileName.
 *  @return the path to the executable, or
 *          NULL if it was not found.
 */
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
        if (cmdFileType(aPath) == FILE_REGULAR) {
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



void freeRtlStriArray (rtlArrayType work_array, intType used_max_position)

  {
    memSizeType position;

  /* freeRtlStriArray */
    for (position = 0; position < (uintType) used_max_position; position++) {
      FREE_STRI(work_array->arr[position].value.striValue,
                work_array->arr[position].value.striValue->size);
    } /* for */
    FREE_RTL_ARRAY(work_array, (uintType) work_array->max_position);
  } /* freeRtlStriArray */



/**
 *  Fill an array of 'len' objects with the generic value 'element'.
 *  This function uses loop unrolling inspired by Duff's device.
 *  The use of indices relative to pos allows the C compiler
 *  to do more optimizations.
 *  @param dest Destination array with object values.
 *  @param element Generic value to be filled into 'dest'.
 *  @param len Specifies how often 'element' is filled into 'dest'.
 */
static void memsetGeneric (register rtlObjectType *const dest,
    register const genericType element, memSizeType len)

  {
    register memSizeType pos;

  /* memsetGeneric */
    if (len != 0) {
      pos = (len + 31) & ~(memSizeType) 31;
      switch (len & 31) {
        do {
          case  0: dest[pos -  1].value.genericValue = element;
          case 31: dest[pos -  2].value.genericValue = element;
          case 30: dest[pos -  3].value.genericValue = element;
          case 29: dest[pos -  4].value.genericValue = element;
          case 28: dest[pos -  5].value.genericValue = element;
          case 27: dest[pos -  6].value.genericValue = element;
          case 26: dest[pos -  7].value.genericValue = element;
          case 25: dest[pos -  8].value.genericValue = element;
          case 24: dest[pos -  9].value.genericValue = element;
          case 23: dest[pos - 10].value.genericValue = element;
          case 22: dest[pos - 11].value.genericValue = element;
          case 21: dest[pos - 12].value.genericValue = element;
          case 20: dest[pos - 13].value.genericValue = element;
          case 19: dest[pos - 14].value.genericValue = element;
          case 18: dest[pos - 15].value.genericValue = element;
          case 17: dest[pos - 16].value.genericValue = element;
          case 16: dest[pos - 17].value.genericValue = element;
          case 15: dest[pos - 18].value.genericValue = element;
          case 14: dest[pos - 19].value.genericValue = element;
          case 13: dest[pos - 20].value.genericValue = element;
          case 12: dest[pos - 21].value.genericValue = element;
          case 11: dest[pos - 22].value.genericValue = element;
          case 10: dest[pos - 23].value.genericValue = element;
          case  9: dest[pos - 24].value.genericValue = element;
          case  8: dest[pos - 25].value.genericValue = element;
          case  7: dest[pos - 26].value.genericValue = element;
          case  6: dest[pos - 27].value.genericValue = element;
          case  5: dest[pos - 28].value.genericValue = element;
          case  4: dest[pos - 29].value.genericValue = element;
          case  3: dest[pos - 30].value.genericValue = element;
          case  2: dest[pos - 31].value.genericValue = element;
          case  1: dest[pos - 32].value.genericValue = element;
        } while ((pos -= 32) != 0);
      } /* switch */
    } /* if */
  } /* memsetGeneric */



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
    logFunction(printf("arrAppend(arr1 (size=" FMT_U_MEM "), arr2 (size=" FMT_U_MEM "))\n",
                       arraySize(*arr_variable), arraySize(extension)););
    extension_size = arraySize(extension);
    if (extension_size != 0) {
      arr_to = *arr_variable;
      arr_to_size = arraySize(arr_to);
      if (unlikely(arr_to_size > MAX_RTL_ARR_LEN - extension_size ||
                   arr_to->max_position > (intType) (MAX_MEM_INDEX - extension_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        new_size = arr_to_size + extension_size;
        arr_to = REALLOC_RTL_ARRAY(arr_to, arr_to_size, new_size);
        if (unlikely(arr_to == NULL)) {
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
    logFunction(printf("arrAppend --> arr (size=" FMT_U_MEM ")\n",
                       arraySize(*arr_variable)););
  } /* arrAppend */



rtlArrayType arrArrlit2 (intType start_position, rtlArrayType arr1)

  {
    memSizeType result_size;

  /* arrArrlit2 */
    result_size = arraySize(arr1);
    if (unlikely(start_position < MIN_MEM_INDEX ||
                 start_position > MAX_MEM_INDEX ||
                 (result_size != 0 &&
                  start_position > (intType) (MAX_MEM_INDEX - result_size + 1)) ||
                 (result_size == 0 && start_position == MIN_MEM_INDEX))) {
      logError(printf("arrArrlit2(" FMT_D ", arr1 (size=" FMT_U_MEM ")): "
                      "Minimum or maximum index out of range.\n",
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
    if (unlikely(!ALLOC_RTL_ARRAY(result, result_size))) {
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
    if (unlikely(!ALLOC_RTL_ARRAY(result, result_size))) {
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
    if (unlikely(arr1_size > MAX_RTL_ARR_LEN - arr2_size ||
                 arr1->max_position > (intType) (MAX_MEM_INDEX - arr2_size))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = arr1_size + arr2_size;
      result = REALLOC_RTL_ARRAY(arr1, arr1_size, result_size);
      if (unlikely(result == NULL)) {
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



/**
 *  Concatenate an array and an element.
 *  @return the result of the concatenation.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             array.
 */
rtlArrayType arrExtend (rtlArrayType arr1, const genericType element)

  {
    memSizeType arr1_size;
    memSizeType result_size;
    rtlArrayType result;

  /* arrExtend */
    arr1_size = arraySize(arr1);
    if (unlikely(arr1_size > MAX_RTL_ARR_LEN - 1 ||
                 arr1->max_position > (intType) (MAX_MEM_INDEX - 1))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = arr1_size + 1;
      result = REALLOC_RTL_ARRAY(arr1, arr1_size, result_size);
      if (unlikely(result == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_RTL_ARRAY(arr1_size, result_size);
        result->max_position++;
        result->arr[arr1_size].value.genericValue = element;
      } /* if */
    } /* if */
    return result;
  } /* arrExtend */



/**
 *  Free the memory referred by 'oldArray'.
 *  This function is used by the compiler to define array destructor
 *  functions. It is asssumed that, before arrFree is called,
 *  destructors have been called for all elements of 'oldArray'.
 *  After arrFree is left 'oldArray' refers to not existing memory.
 *  The memory where 'oldArray' is stored can be freed afterwards.
 */
void arrFree (rtlArrayType oldArray)

  {
    memSizeType size;

  /* arrFree */
    size = arraySize(oldArray);
    FREE_RTL_ARRAY(oldArray, size);
  } /* arrFree */



/**
 *  Generate an array from the concatenation of two elements.
 *  @return the result of the concatenation.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             array.
 */
rtlArrayType arrGen (const genericType element1, const genericType element2)

  {
    memSizeType result_size;
    rtlArrayType result;

  /* arrGen */
    result_size = 2;
    if (unlikely(!ALLOC_RTL_ARRAY(result, result_size))) {
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
      if (unlikely(!ALLOC_RTL_ARRAY(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = stop;
        memcpy(result->arr, arr1->arr,
               (size_t) (result_size * sizeof(rtlObjectType)));
      } /* if */
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arrHead(arr1 (size=" FMT_U_MEM "), " FMT_D "): "
                      "Cannot create empty array with minimum index.\n",
                      length, stop););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_RTL_ARRAY(result, 0))) {
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
 *  ArrHeadTemp is used by the compiler if 'arr_temp' is a temporary
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
        if (unlikely(!ALLOC_RTL_ARRAY(new_arr1, length - result_size))) {
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          new_arr1->min_position = stop + 1;
          new_arr1->max_position = arr1->max_position;
          memcpy(new_arr1->arr, &arr1->arr[result_size],
                 (size_t) ((length - result_size) * sizeof(rtlObjectType)));
          result = REALLOC_RTL_ARRAY(arr1, length, result_size);
          if (unlikely(result == NULL)) {
            FREE_RTL_ARRAY(new_arr1, length - result_size);
            raise_error(MEMORY_ERROR);
          } else {
            COUNT3_RTL_ARRAY(length, result_size);
            result->max_position = stop;
            *arr_temp = new_arr1;
          } /* if */
        } /* if */
      } /* if */
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arrHeadTemp(arr1 (size=" FMT_U_MEM "), " FMT_D "): "
                      "Cannot create empty array with minimum index.\n",
                      length, stop););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_RTL_ARRAY(result, 0))) {
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
 *  The compiler uses this function if the array is destroyed
 *  after the indexing. To avoid problems the indexed element is
 *  removed from the array.
 *  @return the element with the specified 'position' from 'arr_temp'.
 *  @exception INDEX_ERROR If 'position' is less than minIdx(arr) or
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
    if (unlikely(position < arr1->min_position ||
                 position > arr1->max_position)) {
      logError(printf("arrIdxTemp(arr1, " FMT_D "): "
                      "Index out of range (" FMT_D " .. " FMT_D ").\n",
                      position, arr1->min_position, arr1->max_position););
      raise_error(INDEX_ERROR);
      result = 0;
    } else {
      length = arraySize(arr1);
      result = arr1->arr[position - arr1->min_position].value.genericValue;
      if (position != arr1->max_position) {
        arr1->arr[position - arr1->min_position].value.genericValue =
            arr1->arr[length - 1].value.genericValue;
      } /* if */
      resized_arr1 = REALLOC_RTL_ARRAY(arr1, length, length - 1);
      if (unlikely(resized_arr1 == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_RTL_ARRAY(length, length - 1);
        resized_arr1->max_position--;
        *arr_temp = resized_arr1;
      } /* if */
    } /* if */
    return result;
  } /* arrIdxTemp */



/**
 *  Allocate memory for an array with given min and max positions.
 *  The min and max positions of the created array are set.
 *  @return A reference to the memory suitable for the array.
 *  @exception MEMORY_ERROR The min or max index is not in the
 *             allowed range, or there is not enough memory to
 *             allocate the array.
 *  @exception RANGE_ERROR If min and max indicate that the array
 *             would have a negative number of elements.
 */
rtlArrayType arrMalloc (intType minPosition, intType maxPosition)

  {
    memSizeType size;
    rtlArrayType result;

  /* arrMalloc */
    if (unlikely(minPosition < MIN_MEM_INDEX ||
                 maxPosition > MAX_MEM_INDEX ||
                 (minPosition == MIN_MEM_INDEX &&
                  maxPosition == MAX_MEM_INDEX))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else if (unlikely(minPosition > MIN_MEM_INDEX &&
                        minPosition - 1 > maxPosition)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      size = arraySize2(minPosition, maxPosition);
      if (unlikely(size > MAX_RTL_ARR_LEN ||
                   !ALLOC_RTL_ARRAY(result, (memSizeType) size))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->min_position = minPosition;
        result->max_position = maxPosition;
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
    if (unlikely(arr_to_size > MAX_RTL_ARR_LEN - 1 ||
                 arr_to->max_position > (intType) (MAX_MEM_INDEX - 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      new_size = arr_to_size + 1;
      arr_to = REALLOC_RTL_ARRAY(arr_to, arr_to_size, new_size);
      if (unlikely(arr_to == NULL)) {
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
      if (unlikely(!ALLOC_RTL_ARRAY(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arrayMaxPos(arr1->min_position, result_size);
        start_idx = arrayIndex(arr1, start);
        memcpy(result->arr, &arr1->arr[start_idx],
               (size_t) (result_size * sizeof(rtlObjectType)));
      } /* if */
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arrRange(arr1 (size=" FMT_U_MEM "), " FMT_D ", " FMT_D "): "
                      "Cannot create empty array with minimum index.\n",
                      length, start, stop););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_RTL_ARRAY(result, 0))) {
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
 *  ArrRangeTemp is used by the compiler if 'arr_temp' is a temporary
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
        if (unlikely(!ALLOC_RTL_ARRAY(result, result_size))) {
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
          if (unlikely(resized_arr1 == NULL)) {
            memcpy(&arr1->arr[length - result_size], result->arr,
                   (size_t) (result_size * sizeof(rtlObjectType)));
            FREE_RTL_ARRAY(result, result_size);
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            COUNT3_RTL_ARRAY(length, length - result_size);
            resized_arr1->max_position = arrayMaxPos(resized_arr1->min_position,
                                                     length - result_size);
            *arr_temp = resized_arr1;
          } /* if */
        } /* if */
      } /* if */
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arrRangeTemp(arr1 (size=" FMT_U_MEM "), " FMT_D ", " FMT_D "): "
                      "Cannot create empty array with minimum index.\n",
                      length, start, stop););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_RTL_ARRAY(result, 0))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    return result;
  } /* arrRangeTemp */



/**
 *  Reallocate memory for an array from 'oldSize' to 'newSize'.
 *  This function is used by the compiler if an array is copied
 *  and the sizes of source and destination array are different.
 *  @return A reference to the memory of the array with 'newSize'.
 *  @exception MEMORY_ERROR There is not enough memory to
 *             reallocate the array.
 */
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
 *  @exception INDEX_ERROR If 'position' is less than minIdx(arr) or
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
    if (unlikely(position < arr1->min_position ||
                 position > arr1->max_position)) {
      logError(printf("arrRemove(arr1, " FMT_D "): "
                      "Index out of range (" FMT_D " .. " FMT_D ").\n",
                      position, arr1->min_position, arr1->max_position););
      raise_error(INDEX_ERROR);
      result = 0;
    } else {
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
      if (unlikely(!ALLOC_RTL_ARRAY(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arrayMaxPos(arr1->min_position, result_size);
        start_idx = arrayIndex(arr1, start);
        memcpy(result->arr, &arr1->arr[start_idx],
               (size_t) (result_size * sizeof(rtlObjectType)));
      } /* if */
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arrSubarr(arr1 (size=" FMT_U_MEM "), " FMT_D ", " FMT_D "): "
                      "Cannot create empty array with minimum index.\n",
                      length, start, len););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_RTL_ARRAY(result, 0))) {
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
 *  ArrSubarrTemp is used by the compiler if 'arr_temp' is a temporary
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
        if (unlikely(!ALLOC_RTL_ARRAY(result, result_size))) {
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
          if (unlikely(resized_arr1 == NULL)) {
            memcpy(&arr1->arr[length - result_size], result->arr,
                   (size_t) (result_size * sizeof(rtlObjectType)));
            FREE_RTL_ARRAY(result, result_size);
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            COUNT3_RTL_ARRAY(length, length - result_size);
            resized_arr1->max_position = arrayMaxPos(resized_arr1->min_position,
                                                     length - result_size);
            *arr_temp = resized_arr1;
          } /* if */
        } /* if */
      } /* if */
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arrSubarrTemp(arr1 (size=" FMT_U_MEM "), " FMT_D ", " FMT_D "): "
                      "Cannot create empty array with minimum index.\n",
                      length, start, len););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_RTL_ARRAY(result, 0))) {
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
      if (unlikely(!ALLOC_RTL_ARRAY(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arrayMaxPos(arr1->min_position, result_size);
        start_idx = arrayIndex(arr1, start);
        memcpy(result->arr, &arr1->arr[start_idx],
               (size_t) (result_size * sizeof(rtlObjectType)));
      } /* if */
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arrTail(arr1 (size=" FMT_U_MEM "), " FMT_D "): "
                      "Cannot create empty array with minimum index.\n",
                      length, start););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_RTL_ARRAY(result, 0))) {
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
 *  ArrTailTemp is used by the compiler if 'arr_temp' is a temporary
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
        if (unlikely(!ALLOC_RTL_ARRAY(result, result_size))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->min_position = arr1->min_position;
          result->max_position = arrayMaxPos(arr1->min_position, result_size);
          start_idx = arrayIndex(arr1, start);
          memcpy(result->arr, &arr1->arr[start_idx],
                 (size_t) (result_size * sizeof(rtlObjectType)));
          resized_arr1 = REALLOC_RTL_ARRAY(arr1, length, length - result_size);
          if (unlikely(resized_arr1 == NULL)) {
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
    } else if (unlikely(arr1->min_position == MIN_MEM_INDEX)) {
      logError(printf("arrTailTemp(arr1 (size=" FMT_U_MEM "), " FMT_D "): "
                      "Cannot create empty array with minimum index.\n",
                      length, start););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_RTL_ARRAY(result, 0))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = arr1->min_position;
        result->max_position = arr1->min_position - 1;
      } /* if */
    } /* if */
    return result;
  } /* arrTailTemp */



/**
 *  Create array with given 'min' and 'max' positions and 'element'.
 *  The 'min' and 'max' positions of the created array are set and
 *  the array is filled with 'element' values.
 *  @return The new created array.
 *  @exception MEMORY_ERROR The min or max index is not in the
 *             allowed range, or there is not enough memory to
 *             allocate the array.
 *  @exception RANGE_ERROR If min and max indicate that the array
 *             would have a negative number of elements.
 */
rtlArrayType arrTimes (intType minPosition, intType maxPosition,
    const genericType element)

  {
    memSizeType size;
    rtlArrayType result;

  /* arrTimes */
    result = arrMalloc(minPosition, maxPosition);
    if (result != NULL) {
      size = arraySize2(minPosition, maxPosition);
      memsetGeneric(result->arr, element, size);
    } /* if */
    return result;
  } /* arrTimes */
