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
/*  Content: Procedures to manage the include library search path.  */
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
#include "cmd_drv.h"

#undef EXTERN
#define EXTERN
#include "libpath.h"


#ifdef PATHS_RELATIVE_TO_EXECUTABLE
extern striType programPath; /* defined in s7.c or in the executable of a program */
#endif

static rtlArrayType lib_path;



void find_include_file (const_striType include_file_name, errInfoType *err_info)

  {
    boolType found;
    memSizeType lib_path_size;
    memSizeType position;
    striType curr_path;
    memSizeType length;
    striType stri;

  /* find_include_file */
    logFunction(printf("find_include_file(\"%s\"\n",
                       striAsUnquotedCStri(include_file_name)););
    if (*err_info == OKAY_NO_ERROR) {
      if (include_file_name->size >= 1 && include_file_name->mem[0] == '/') {
        open_infile(include_file_name, in_file.write_library_names, in_file.write_line_numbers, err_info);
      } else {
        found = FALSE;
        lib_path_size = arraySize(lib_path);
        for (position = 0; !found && *err_info == OKAY_NO_ERROR && position < lib_path_size; position++) {
          curr_path = lib_path->arr[position].value.striValue;
          if (curr_path->size == 0) {
            open_infile(include_file_name, in_file.write_library_names, in_file.write_line_numbers, err_info);
          } else {
            if (curr_path->size > MAX_STRI_LEN - include_file_name->size) {
              *err_info = MEMORY_ERROR;
            } else {
              length = curr_path->size + include_file_name->size;
              if (!ALLOC_STRI_SIZE_OK(stri, length)) {
                *err_info = MEMORY_ERROR;
              } else {
                stri->size = length;
                memcpy(stri->mem, curr_path->mem,
                    (size_t) curr_path->size * sizeof(strElemType));
                memcpy(&stri->mem[curr_path->size], include_file_name->mem,
                    (size_t) include_file_name->size * sizeof(strElemType));
                open_infile(stri, in_file.write_library_names, in_file.write_line_numbers, err_info);
                FREE_STRI(stri, length);
              } /* if */
            } /* if */
          } /* if */
          if (*err_info == OKAY_NO_ERROR) {
            found = TRUE;
          } else if (*err_info == FILE_ERROR) {
            *err_info = OKAY_NO_ERROR;
          } /* if */
        } /* for */
        if (!found && *err_info == OKAY_NO_ERROR) {
          *err_info = FILE_ERROR;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("find_include_file --> (err_info=%d)\n", *err_info););
  } /* find_include_file */



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



void append_to_lib_path (const_striType path, errInfoType *err_info)

  {
    memSizeType stri_len;
    striType stri;
    rtlArrayType resized_lib_path;
    memSizeType position;

  /* append_to_lib_path */
    logFunction(printf("append_to_lib_path(\"%s\")\n",
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
    logFunction(printf("append_to_lib_path --> (err_info=%d)\n", *err_info););
  } /* append_to_lib_path */



void init_lib_path (const_striType source_file_name,
    const const_rtlArrayType seed7_libraries, errInfoType *err_info)

  {
    striType path;
    memSizeType position;
    memSizeType dir_path_size;
    static const os_charType seed7_library[] =
        {'S', 'E', 'E', 'D', '7', '_', 'L', 'I', 'B', 'R', 'A', 'R', 'Y', 0};
    os_striType library_environment_variable;
    intType idx;

  /* init_lib_path */
    logFunction(printf("init_lib_path\n"););
    if (!ALLOC_RTL_ARRAY(lib_path, 0)) {
      *err_info = MEMORY_ERROR;
    } else {
      lib_path->min_position = 1;
      lib_path->max_position = 0;

      /* Add directory of the source file to the lib_path. */
      dir_path_size = 0;
      for (position = 0; position < source_file_name->size; position++) {
        if (source_file_name->mem[position] == '/') {
          dir_path_size = position + 1;
        } /* if */
      } /* for */
      if (!ALLOC_STRI_SIZE_OK(path, dir_path_size)) {
        *err_info = MEMORY_ERROR;
      } else {
        path->size = dir_path_size;
        memcpy(path->mem, source_file_name->mem, dir_path_size * sizeof(strElemType));
        append_to_lib_path(path, err_info);
        FREE_STRI(path, path->size);
      } /* if */

#ifdef PATHS_RELATIVE_TO_EXECUTABLE
      /* When the path to the interpreter or to the current        */
      /* executable ends with "bin" or "prg": Replace "bin"        */
      /* respectively "prg" with "lib" and add it to the lib_path. */
      /* prot_cstri("programPath: ");
         prot_stri(programPath);
         prot_nl(); */
      path = relativeToProgramPath(programPath, "lib");
      if (unlikely(path == NULL)) {
        *err_info = MEMORY_ERROR;
      } else {
        append_to_lib_path(path, err_info);
        FREE_STRI(path, path->size);
      } /* if */
#endif

      /* Add the libraries from the commandline to the lib_path */
      if (seed7_libraries != NULL) {
        for (idx = 0; idx <= seed7_libraries->max_position - seed7_libraries->min_position; idx++) {
          append_to_lib_path(seed7_libraries->arr[idx].value.striValue, err_info);
        } /* for */
      } /* if */

      /* Add the hardcoded library of the interpreter to the lib_path. */
      path = cstri8_or_cstri_to_stri(SEED7_LIBRARY);
      if (path == NULL) {
        *err_info = MEMORY_ERROR;
      } else {
        append_to_lib_path(path, err_info);
        FREE_STRI(path, path->size);
      } /* if */

      /* Add the SEED7_LIBRARY environment variable to the lib_path */
      library_environment_variable = os_getenv(seed7_library);
      if (library_environment_variable != NULL) {
        path = cp_from_os_path(library_environment_variable, err_info);
        os_getenv_string_free(library_environment_variable);
        if (path != NULL) {
          append_to_lib_path(path, err_info);
          FREE_STRI(path, path->size);
        } /* if */
      } /* if */

      /* print_lib_path(); */
    } /* if */
    logFunction(printf("init_lib_path\n"););
  } /* init_lib_path */



void free_lib_path (void)

  {
    memSizeType length;
    memSizeType position;
    striType stri;

  /* free_lib_path */
    logFunction(printf("free_lib_path\n"););
    length = arraySize(lib_path);
    for (position = 0; position < length; position++) {
      stri = lib_path->arr[position].value.striValue;
      FREE_STRI(stri, stri->size);
    } /* for */
    FREE_RTL_ARRAY(lib_path, length);
    lib_path = NULL;
    logFunction(printf("free_lib_path -->\n"););
  } /* free_lib_path */
