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
extern stritype programPath; /* defined in s7.c or in the executable of a program */
#endif

static rtlArraytype lib_path;



void find_include_file (const_stritype include_file_name, errinfotype *err_info)

  {
    booltype found;
    memsizetype lib_path_size;
    memsizetype position;
    stritype curr_path;
    memsizetype length;
    stritype stri;

  /* find_include_file */
#ifdef TRACE_INFILE
    printf("BEGIN find_include_file\n");
#endif
    if (*err_info == OKAY_NO_ERROR) {
      if (include_file_name->size >= 1 && include_file_name->mem[0] == '/') {
        open_infile(include_file_name, in_file.write_library_names, in_file.write_line_numbers, err_info);
      } else {
        found = FALSE;
        lib_path_size = arraySize(lib_path);
        for (position = 0; !found && *err_info == OKAY_NO_ERROR && position < lib_path_size; position++) {
          curr_path = lib_path->arr[position].value.strivalue;
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
                    (size_t) curr_path->size * sizeof(strelemtype));
                memcpy(&stri->mem[curr_path->size], include_file_name->mem,
                    (size_t) include_file_name->size * sizeof(strelemtype));
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
#ifdef TRACE_INFILE
    printf("END find_include_file\n");
#endif
  } /* find_include_file */



#ifdef OUT_OF_ORDER
static void print_lib_path (void)

  {
    memsizetype length;
    memsizetype position;
    stritype stri;

  /* print_lib_path */
    length = arraySize(lib_path);
    for (position = 0; position < length; position++) {
      stri = lib_path->arr[position].value.strivalue;
      prot_stri(stri);
      prot_nl();
    } /* for */
  } /* print_lib_path */
#endif



void append_to_lib_path (const_stritype path, errinfotype *err_info)

  {
    memsizetype stri_len;
    stritype stri;
    rtlArraytype resized_lib_path;
    memsizetype position;

  /* append_to_lib_path */
#ifdef TRACE_INFILE
    printf("BEGIN append_to_lib_path(");
    prot_stri(path);
    printf(")\n");
#endif
    stri_len = path->size;
    if (stri_len >= 1 && path->mem[stri_len - 1] != '/') {
      stri_len++;
    } /* if */
    if (!ALLOC_STRI_CHECK_SIZE(stri, stri_len)) {
      *err_info = MEMORY_ERROR;
    } else {
      resized_lib_path = REALLOC_RTL_ARRAY(lib_path,
          (memsizetype) lib_path->max_position, (memsizetype) (lib_path->max_position + 1));
      if (resized_lib_path == NULL) {
        FREE_STRI(stri, stri_len);
        *err_info = MEMORY_ERROR;
      } else {
        lib_path = resized_lib_path;
        COUNT3_RTL_ARRAY((memsizetype) lib_path->max_position, (memsizetype) (lib_path->max_position + 1));
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
        lib_path->arr[lib_path->max_position].value.strivalue = stri;
        lib_path->max_position++;
      } /* if */
    } /* if */
#ifdef TRACE_INFILE
    printf("END append_to_lib_path\n");
#endif
  } /* append_to_lib_path */



void init_lib_path (const_stritype source_file_name,
    const const_rtlArraytype seed7_libraries, errinfotype *err_info)

  {
    stritype path;
    memsizetype position;
    memsizetype dir_path_size;
    static const os_chartype seed7_library[] =
        {'S', 'E', 'E', 'D', '7', '_', 'L', 'I', 'B', 'R', 'A', 'R', 'Y', 0};
    os_stritype library_environment_variable;
    inttype idx;

  /* init_lib_path */
#ifdef TRACE_INFILE
    printf("BEGIN init_lib_path\n");
#endif
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
        memcpy(path->mem, source_file_name->mem, dir_path_size * sizeof(strelemtype));
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
          append_to_lib_path(seed7_libraries->arr[idx].value.strivalue, err_info);
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
#ifdef TRACE_INFILE
    printf("END init_lib_path\n");
#endif
  } /* init_lib_path */



void free_lib_path (void)

  {
    memsizetype length;
    memsizetype position;
    stritype stri;

  /* free_lib_path */
    length = arraySize(lib_path);
    for (position = 0; position < length; position++) {
      stri = lib_path->arr[position].value.strivalue;
      FREE_STRI(stri, stri->size);
    } /* for */
    FREE_RTL_ARRAY(lib_path, length);
    lib_path = NULL;
  } /* free_lib_path */
