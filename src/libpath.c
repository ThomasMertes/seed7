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
/*  Module: Analyzer - Libraries                                    */
/*  File: seed7/src/libpath.c                                       */
/*  Changes: 1990 - 1994, 2013  Thomas Mertes                       */
/*  Content: Functions to manage the include library search path.   */
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
#include "data_rtl.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "infile.h"
#include "hsh_rtl.h"
#include "str_rtl.h"
#include "cmd_rtl.h"
#include "cmd_drv.h"

#undef EXTERN
#define EXTERN
#include "libpath.h"


static rtlArrayType lib_path;

intType strCmpGeneric (const genericType value1, const genericType value2);
genericType strCreateGeneric (const genericType source);
void strDestrGeneric (const genericType old_value);



rtlHashType initIncludeFileHash (void)

  {
    rtlHashType includeFileHash;

  /* initIncludeFileHash */
    logFunction(printf("initIncludeFileHash()\n"););
    includeFileHash = hshEmpty();
    logFunction(printf("initIncludeFileHash --> " FMT_U_MEM "\n",
                       (memSizeType) includeFileHash););
    return includeFileHash;
  } /* initIncludeFileHash */



void shutIncludeFileHash (const const_rtlHashType includeFileHash)

  { /* shutIncludeFileHash */
    logFunction(printf("shutIncludeFileHash(" FMT_U_MEM ")\n",
                       (memSizeType) includeFileHash););
    hshDestr(includeFileHash, &strDestrGeneric, &genericDestr);
    logFunction(printf("shutIncludeFileHash -->\n"););
  } /* shutIncludeFileHash */



static includeResultType openIncludeFile (const rtlHashType includeFileHash,
    const_striType includeFileName, errInfoType *err_info)

  {
    striType absolutePath;
    includeResultType includeResult = INCLUDE_FAILED;

  /* openIncludeFile */
    logFunction(printf("openIncludeFile(" FMT_U_MEM ", \"%s\", %d)\n",
                       (memSizeType) includeFileHash,
                       striAsUnquotedCStri(includeFileName),
                       *err_info););
    absolutePath = getAbsolutePath(includeFileName);
    if (unlikely(absolutePath == NULL)) {
      *err_info = MEMORY_ERROR;
    } else {
      if (hshContains(includeFileHash, (genericType) (memSizeType) absolutePath,
                      strHashCode(absolutePath), &strCmpGeneric)) {
        /* already included */
        logMessage(printf("already included: \"%s\"\n",
                          striAsUnquotedCStri(absolutePath)););
        includeResult = INCLUDE_ALREADY;
      } else if (openInfile(includeFileName, in_file.write_library_names,
                            in_file.write_line_numbers, err_info)) {
        /* add to list of include files */
        hshIncl(includeFileHash, (genericType) (memSizeType) absolutePath,
                (genericType) 1, strHashCode(absolutePath),
                &strCmpGeneric, &strCreateGeneric,
                &genericCreate, &genericCpy);
        includeResult = INCLUDE_SUCCESS;
      } /* if */
      FREE_STRI(absolutePath, absolutePath->size);
    } /* if */
    logFunction(printf("openIncludeFile --> %d (err_info=%d)\n",
                       includeResult, *err_info););
    return includeResult;
  } /* openIncludeFile */



/**
 *  Search for an include library and open it, if it was found.
 *  A library with an absolute path is opened directly.
 *  For a library with a relative path the directories of the
 *  include library search path are checked for the library.
 *  The directories of the search path are checked one after
 *  another for the requested include library file. As soon as
 *  the include library is found the search is stopped and the
 *  include library is opened.
 */
includeResultType findIncludeFile (const rtlHashType includeFileHash,
    const_striType includeFileName, errInfoType *err_info)

  {
    memSizeType lib_path_size;
    memSizeType position;
    striType curr_path;
    memSizeType length;
    striType stri;
    includeResultType includeResult = INCLUDE_FAILED;

  /* findIncludeFile */
    logFunction(printf("findIncludeFile(" FMT_U_MEM ", \"%s\", %d)\n",
                       (memSizeType) includeFileHash,
                       striAsUnquotedCStri(includeFileName),
                       *err_info););
    if (*err_info == OKAY_NO_ERROR) {
      if (includeFileName->size >= 1 && includeFileName->mem[0] == '/') {
        includeResult = openIncludeFile(includeFileHash, includeFileName, err_info);
      } else if (unlikely(lib_path == NULL)) {
        /* This is a compile-time function and it is called at run-time. */
        *err_info = ACTION_ERROR;
      } else {
        lib_path_size = arraySize(lib_path);
        for (position = 0;
             includeResult == INCLUDE_FAILED && position < lib_path_size;
             position++) {
          curr_path = lib_path->arr[position].value.striValue;
          if (curr_path->size == 0) {
            includeResult = openIncludeFile(includeFileHash, includeFileName, err_info);
          } else {
            if (curr_path->size > MAX_STRI_LEN - includeFileName->size) {
              *err_info = MEMORY_ERROR;
            } else {
              length = curr_path->size + includeFileName->size;
              if (!ALLOC_STRI_SIZE_OK(stri, length)) {
                *err_info = MEMORY_ERROR;
              } else {
                stri->size = length;
                memcpy(stri->mem, curr_path->mem,
                    (size_t) curr_path->size * sizeof(strElemType));
                memcpy(&stri->mem[curr_path->size], includeFileName->mem,
                    (size_t) includeFileName->size * sizeof(strElemType));
                includeResult = openIncludeFile(includeFileHash, stri, err_info);
                FREE_STRI(stri, length);
              } /* if */
            } /* if */
          } /* if */
        } /* for */
        if (includeResult == INCLUDE_FAILED && *err_info == OKAY_NO_ERROR) {
          *err_info = FILE_ERROR;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("findIncludeFile --> %d (err_info=%d)\n",
                       includeResult, *err_info););
    return includeResult;
  } /* findIncludeFile */



#ifdef OUT_OF_ORDER
static void print_lib_path (void)

  {
    memSizeType length;
    memSizeType position;
    striType stri;

  /* print_lib_path */
    length = arraySize(lib_path);
    for (position = 0; position < length; position++) {
      stri = lib_path->arr[position].value.striValue;
      prot_stri(stri);
      prot_nl();
    } /* for */
  } /* print_lib_path */
#endif



/**
 *  Add the given directory path to the include library search path.
 *  The function makes sure that all paths in the include library
 *  search path end with '/'.
 */
void appendToLibPath (const_striType path, errInfoType *err_info)

  {
    memSizeType stri_len;
    striType stri;
    rtlArrayType resized_lib_path;
    memSizeType position;

  /* appendToLibPath */
    logFunction(printf("appendToLibPath(\"%s\")\n",
                       striAsUnquotedCStri(path)););
    stri_len = path->size;
    if (stri_len >= 1 && path->mem[stri_len - 1] != '/') {
      stri_len++;
    } /* if */
    if (!ALLOC_STRI_CHECK_SIZE(stri, stri_len)) {
      *err_info = MEMORY_ERROR;
    } else {
      resized_lib_path = REALLOC_RTL_ARRAY(lib_path,
          (memSizeType) lib_path->max_position,
          (memSizeType) (lib_path->max_position + 1));
      if (resized_lib_path == NULL) {
        FREE_STRI(stri, stri_len);
        *err_info = MEMORY_ERROR;
      } else {
        lib_path = resized_lib_path;
        COUNT3_RTL_ARRAY((memSizeType) lib_path->max_position,
            (memSizeType) (lib_path->max_position + 1));
        stri->size = stri_len;
        for (position = 0; position < path->size; position++) {
          if (path->mem[position] == '\\') {
            stri->mem[position] = '/';
          } else {
            stri->mem[position] = path->mem[position];
          } /* if */
        } /* for */
        if (stri_len != path->size) {
          stri->mem[stri_len - 1] = '/';
        } /* if */
        lib_path->arr[lib_path->max_position].value.striValue = stri;
        lib_path->max_position++;
      } /* if */
    } /* if */
    logFunction(printf("appendToLibPath --> (err_info=%d)\n", *err_info););
  } /* appendToLibPath */



/**
 *  Initialize the include library search path.
 *  The initial search path consists of the following directories:
 *  1. The directory of the program source file.
 *  2. Directories from the commandline option -l.
 *  3. The directory containing the predefined Seed7 include libraries.
 *  4. The directory specified with the SEED7_LIBRARY environment variable.
 */
void initLibPath (const_striType sourceFileName,
    const const_rtlArrayType seed7_libraries, errInfoType *err_info)

  {
    striType path;
    memSizeType position;
    memSizeType dir_path_size;
    static const os_charType seed7_library[] =
        {'S', 'E', 'E', 'D', '7', '_', 'L', 'I', 'B', 'R', 'A', 'R', 'Y', 0};
    os_striType library_environment_variable;
    intType idx;

  /* initLibPath */
    logFunction(printf("initLibPath\n"););
    if (!ALLOC_RTL_ARRAY(lib_path, 0)) {
      *err_info = MEMORY_ERROR;
    } else {
      lib_path->min_position = 1;
      lib_path->max_position = 0;

      /* Add directory of the source file to the lib_path. */
      dir_path_size = 0;
      for (position = 0; position < sourceFileName->size; position++) {
        if (sourceFileName->mem[position] == '/') {
          dir_path_size = position + 1;
        } /* if */
      } /* for */
      if (!ALLOC_STRI_SIZE_OK(path, dir_path_size)) {
        *err_info = MEMORY_ERROR;
      } else {
        path->size = dir_path_size;
        memcpy(path->mem, sourceFileName->mem, dir_path_size * sizeof(strElemType));
        appendToLibPath(path, err_info);
        FREE_STRI(path, path->size);
      } /* if */

      /* Add the libraries from the commandline to the lib_path */
      if (seed7_libraries != NULL) {
        for (idx = 0;
             idx <= seed7_libraries->max_position - seed7_libraries->min_position;
             idx++) {
          appendToLibPath(seed7_libraries->arr[idx].value.striValue, err_info);
        } /* for */
      } /* if */

      /* Add the hardcoded library of the interpreter to the lib_path. */
      path = cstri8_or_cstri_to_stri(SEED7_LIBRARY);
      if (path == NULL) {
        *err_info = MEMORY_ERROR;
      } else {
        appendToLibPath(path, err_info);
        FREE_STRI(path, path->size);
      } /* if */

      /* Add the SEED7_LIBRARY environment variable to the lib_path */
      library_environment_variable = os_getenv(seed7_library);
      if (library_environment_variable != NULL) {
        path = cp_from_os_path(library_environment_variable, err_info);
        os_getenv_string_free(library_environment_variable);
        if (path != NULL) {
          appendToLibPath(path, err_info);
          FREE_STRI(path, path->size);
        } /* if */
      } /* if */

      /* print_lib_path(); */
    } /* if */
    logFunction(printf("initLibPath -->\n"););
  } /* initLibPath */



void freeLibPath (void)

  {
    memSizeType length;
    memSizeType position;
    striType stri;

  /* freeLibPath */
    logFunction(printf("freeLibPath\n"););
    length = arraySize(lib_path);
    for (position = 0; position < length; position++) {
      stri = lib_path->arr[position].value.striValue;
      FREE_STRI(stri, stri->size);
    } /* for */
    FREE_RTL_ARRAY(lib_path, length);
    lib_path = NULL;
    logFunction(printf("freeLibPath -->\n"););
  } /* freeLibPath */
