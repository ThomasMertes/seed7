/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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
/*  File: seed7/src/cmdlib.c                                        */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Primitive actions for various commands.                */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "dir_drv.h"

#include "common.h"
#include "data.h"
#include "data_rtl.h" /*!!##*/
#include "os_decls.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "objutl.h"
#include "runerr.h"
#include "dir_rtl.h"
#include "str_rtl.h"
#include "cmd_rtl.h"
#include "cmd_drv.h"

#undef EXTERN
#define EXTERN
#include "cmdlib.h"



#ifdef ANSI_C

#ifdef USE_CDECL
static int _cdecl cmp_mem (char *strg1, char *strg2)
#else
static int cmp_mem (void const *strg1, void const *strg2)
#endif
#else

static int cmp_mem (strg1, strg2)
char *strg1;
char *strg2;
#endif

  { /* cmp_mem */
    return strCompare(
        ((const_objecttype) strg1)->value.strivalue,
        ((const_objecttype) strg2)->value.strivalue);
  } /* cmp_mem */



#ifdef ANSI_C

static arraytype read_dir (dirtype directory)
#else

static arraytype read_dir (directory)
dirtype directory;
#endif

  {
    arraytype dir_array;
    arraytype resized_dir_array;
    memsizetype max_array_size;
    inttype used_array_size;
    memsizetype position;
    stritype stri1;
    booltype okay;

  /* read_dir */
    max_array_size = 256;
    if (ALLOC_ARRAY(dir_array, max_array_size)) {
      used_array_size = 0;
      stri1 = dirRead(directory);
      okay = TRUE;
      while (okay && stri1 != NULL) {
        if ((memsizetype) used_array_size >= max_array_size) {
          if (max_array_size >= MAX_MEM_INDEX - 256) {
            resized_dir_array = NULL;
          } else {
            resized_dir_array = REALLOC_ARRAY(dir_array,
                max_array_size, max_array_size + 256);
          } /* if */
          if (resized_dir_array == NULL) {
            okay = FALSE;
          } else {
            dir_array = resized_dir_array;
            COUNT3_ARRAY(max_array_size, max_array_size + 256);
            max_array_size += 256;
          } /* if */
        } /* if */
        if (okay) {
          dir_array->arr[used_array_size].type_of = take_type(SYS_STRI_TYPE);
          dir_array->arr[used_array_size].descriptor.property = NULL;
          dir_array->arr[used_array_size].value.strivalue = stri1;
          INIT_CATEGORY_OF_VAR(&dir_array->arr[used_array_size],
              STRIOBJECT);
          used_array_size++;
          stri1 = dirRead(directory);
        } /* if */
      } /* while */
      if (okay) {
        resized_dir_array = REALLOC_ARRAY(dir_array,
            max_array_size, (memsizetype) used_array_size);
        if (resized_dir_array == NULL) {
          okay = FALSE;
        } else {
          dir_array = resized_dir_array;
          COUNT3_ARRAY(max_array_size, (memsizetype) used_array_size);
          dir_array->min_position = 1;
          dir_array->max_position = used_array_size;
        } /* if */
      } /* if */
      if (!okay) {
        for (position = 0; position < (memsizetype) used_array_size; position++) {
          FREE_STRI(dir_array->arr[position].value.strivalue,
              dir_array->arr[position].value.strivalue->size);
        } /* for */
        FREE_ARRAY(dir_array, max_array_size);
        dir_array = NULL;
      } /* if */
    } /* if */
    return dir_array;
  } /* read_dir */



#ifdef ANSI_C

static objecttype toArraytype (rtlArraytype anRtlArray)
#else

static objecttype toArraytype (anRtlArray)
rtlArraytype anRtlArray;
#endif

  {
    memsizetype arraySize;
    memsizetype pos;
    arraytype anArray;
    objecttype result;

  /* toArraytype */
    if (anRtlArray == NULL) {
      /* Assume that an exception was already raised */
      result = NULL;
    } else {
      arraySize = (uinttype) (anRtlArray->max_position - anRtlArray->min_position + 1);
      if (!ALLOC_ARRAY(anArray, arraySize)) {
        for (pos = 0; pos < arraySize; pos++) {
          strDestr(anRtlArray->arr[pos].value.strivalue);
        } /* for */
        FREE_RTL_ARRAY(anRtlArray, arraySize);
        result = raise_exception(SYS_MEM_EXCEPTION);
      } else {
        anArray->min_position = anRtlArray->min_position;
        anArray->max_position = anRtlArray->max_position;
        for (pos = 0; pos < arraySize; pos++) {
          anArray->arr[pos].type_of = take_type(SYS_STRI_TYPE);
          anArray->arr[pos].descriptor.property = NULL;
          anArray->arr[pos].value.strivalue = anRtlArray->arr[pos].value.strivalue;
          INIT_CATEGORY_OF_VAR(&anArray->arr[pos], STRIOBJECT);
        } /* for */
        FREE_RTL_ARRAY(anRtlArray, arraySize);
        result = bld_array_temp(anArray);
      } /* if */
    } /* if */
    return result;
  } /* toArraytype */



#ifdef ANSI_C

objecttype cmd_big_filesize (listtype arguments)
#else

objecttype cmd_big_filesize (arguments)
listtype arguments;
#endif

  { /* cmd_big_filesize */
    isit_stri(arg_1(arguments));
    return bld_bigint_temp(
        cmdBigFileSize(take_stri(arg_1(arguments))));
  } /* cmd_big_filesize */



#ifdef ANSI_C

objecttype cmd_chdir (listtype arguments)
#else

objecttype cmd_chdir (arguments)
listtype arguments;
#endif

  { /* cmd_chdir */
    isit_stri(arg_1(arguments));
    cmdChdir(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_chdir */



#ifdef ANSI_C

objecttype cmd_clone_file (listtype arguments)
#else

objecttype cmd_clone_file (arguments)
listtype arguments;
#endif

  { /* cmd_clone_file */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdCloneFile(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_clone_file */



#ifdef ANSI_C

objecttype cmd_config_value (listtype arguments)
#else

objecttype cmd_config_value (arguments)
listtype arguments;
#endif

  { /* cmd_config_value */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdConfigValue(take_stri(arg_1(arguments))));
  } /* cmd_config_value */



#ifdef ANSI_C

objecttype cmd_copy_file (listtype arguments)
#else

objecttype cmd_copy_file (arguments)
listtype arguments;
#endif

  { /* cmd_copy_file */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdCopyFile(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_copy_file */



#ifdef ANSI_C

objecttype cmd_environment (listtype arguments)
#else

objecttype cmd_environment (arguments)
listtype arguments;
#endif

  { /* cmd_environment */
    return toArraytype(cmdEnvironment());
  } /* cmd_environment */



#ifdef ANSI_C

objecttype cmd_filemode (listtype arguments)
#else

objecttype cmd_filemode (arguments)
listtype arguments;
#endif

  { /* cmd_filemode */
    isit_stri(arg_1(arguments));
    return bld_set_temp(
        cmdFileMode(take_stri(arg_1(arguments))));
  } /* cmd_filemode */



#ifdef ANSI_C

objecttype cmd_filesize (listtype arguments)
#else

objecttype cmd_filesize (arguments)
listtype arguments;
#endif

  { /* cmd_filesize */
    isit_stri(arg_1(arguments));
    return bld_int_temp(
        cmdFileSize(take_stri(arg_1(arguments))));
  } /* cmd_filesize */



#ifdef ANSI_C

objecttype cmd_filetype (listtype arguments)
#else

objecttype cmd_filetype (arguments)
listtype arguments;
#endif

  { /* cmd_filetype */
    isit_stri(arg_1(arguments));
    return bld_int_temp(
        cmdFileType(take_stri(arg_1(arguments))));
  } /* cmd_filetype */



#ifdef ANSI_C

objecttype cmd_filetype_sl (listtype arguments)
#else

objecttype cmd_filetype_sl (arguments)
listtype arguments;
#endif

  { /* cmd_filetype_sl */
    isit_stri(arg_1(arguments));
    return bld_int_temp(
        cmdFileTypeSL(take_stri(arg_1(arguments))));
  } /* cmd_filetype_sl */



#ifdef ANSI_C

objecttype cmd_getcwd (listtype arguments)
#else

objecttype cmd_getcwd (arguments)
listtype arguments;
#endif

  { /* cmd_getcwd */
    return bld_stri_temp(cmdGetcwd());
  } /* cmd_getcwd */



#ifdef ANSI_C

objecttype cmd_getenv (listtype arguments)
#else

objecttype cmd_getenv (arguments)
listtype arguments;
#endif

  { /* cmd_getenv */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdGetenv(take_stri(arg_1(arguments))));
  } /* cmd_getenv */



#ifdef ANSI_C

objecttype cmd_get_atime (listtype arguments)
#else

objecttype cmd_get_atime (arguments)
listtype arguments;
#endif

  {
    booltype is_dst;

  /* cmd_get_atime */
    isit_stri(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    isit_int(arg_9(arguments));
    isit_bool(arg_10(arguments));
    cmdGetATime(take_stri(arg_1(arguments)),
            &arg_2(arguments)->value.intvalue,
            &arg_3(arguments)->value.intvalue,
            &arg_4(arguments)->value.intvalue,
            &arg_5(arguments)->value.intvalue,
            &arg_6(arguments)->value.intvalue,
            &arg_7(arguments)->value.intvalue,
            &arg_8(arguments)->value.intvalue,
            &arg_9(arguments)->value.intvalue,
            &is_dst);
    if (is_dst) {
      arg_10(arguments)->value.objvalue = SYS_TRUE_OBJECT;
    } else {
      arg_10(arguments)->value.objvalue = SYS_FALSE_OBJECT;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_get_atime */



#ifdef ANSI_C

objecttype cmd_get_ctime (listtype arguments)
#else

objecttype cmd_get_ctime (arguments)
listtype arguments;
#endif

  {
    booltype is_dst;

  /* cmd_get_ctime */
    isit_stri(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    isit_int(arg_9(arguments));
    isit_bool(arg_10(arguments));
    cmdGetCTime(take_stri(arg_1(arguments)),
            &arg_2(arguments)->value.intvalue,
            &arg_3(arguments)->value.intvalue,
            &arg_4(arguments)->value.intvalue,
            &arg_5(arguments)->value.intvalue,
            &arg_6(arguments)->value.intvalue,
            &arg_7(arguments)->value.intvalue,
            &arg_8(arguments)->value.intvalue,
            &arg_9(arguments)->value.intvalue,
            &is_dst);
    if (is_dst) {
      arg_10(arguments)->value.objvalue = SYS_TRUE_OBJECT;
    } else {
      arg_10(arguments)->value.objvalue = SYS_FALSE_OBJECT;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_get_ctime */



#ifdef ANSI_C

objecttype cmd_get_mtime (listtype arguments)
#else

objecttype cmd_get_mtime (arguments)
listtype arguments;
#endif

  {
    booltype is_dst;

  /* cmd_get_mtime */
    isit_stri(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    isit_int(arg_9(arguments));
    isit_bool(arg_10(arguments));
    cmdGetMTime(take_stri(arg_1(arguments)),
            &arg_2(arguments)->value.intvalue,
            &arg_3(arguments)->value.intvalue,
            &arg_4(arguments)->value.intvalue,
            &arg_5(arguments)->value.intvalue,
            &arg_6(arguments)->value.intvalue,
            &arg_7(arguments)->value.intvalue,
            &arg_8(arguments)->value.intvalue,
            &arg_9(arguments)->value.intvalue,
            &is_dst);
    if (is_dst) {
      arg_10(arguments)->value.objvalue = SYS_TRUE_OBJECT;
    } else {
      arg_10(arguments)->value.objvalue = SYS_FALSE_OBJECT;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_get_mtime */



#ifdef ANSI_C

objecttype cmd_getSearchPath (listtype arguments)
#else

objecttype cmd_getSearchPath (arguments)
listtype arguments;
#endif

  { /* cmd_getSearchPath */
    return toArraytype(cmdGetSearchPath());
  } /* cmd_getSearchPath */



#ifdef ANSI_C

objecttype cmd_homeDir (listtype arguments)
#else

objecttype cmd_homeDir (arguments)
listtype arguments;
#endif

  { /* cmd_homeDir */
    return bld_stri_temp(cmdHomeDir());
  } /* cmd_homeDir */



#ifdef ANSI_C

objecttype cmd_ls (listtype arguments)
#else

objecttype cmd_ls (arguments)
listtype arguments;
#endif

  {
    stritype dir_name;
    dirtype directory;
    arraytype result;

  /* cmd_ls */
    isit_stri(arg_1(arguments));
    dir_name = take_stri(arg_1(arguments));
    if ((directory = dirOpen(dir_name)) != NULL) {
      result = read_dir(directory);
      dirClose(directory);
      if (result == NULL) {
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } else {
        qsort((void *) result->arr,
            (size_t) (result->max_position - result->min_position + 1),
            sizeof(objectrecord), &cmp_mem);
        return bld_array_temp(result);
      } /* if */
    } else {
      return raise_with_arguments(SYS_FIL_EXCEPTION, arguments);
    } /* if */
  } /* cmd_ls */



#ifdef ANSI_C

objecttype cmd_mkdir (listtype arguments)
#else

objecttype cmd_mkdir (arguments)
listtype arguments;
#endif

  { /* cmd_mkdir */
    isit_stri(arg_1(arguments));
    cmdMkdir(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_mkdir */



#ifdef ANSI_C

objecttype cmd_move (listtype arguments)
#else

objecttype cmd_move (arguments)
listtype arguments;
#endif

  { /* cmd_move */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdMove(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_move */



#ifdef ANSI_C

objecttype cmd_pipe2 (listtype arguments)
#else

objecttype cmd_pipe2 (arguments)
listtype arguments;
#endif

  {
    objecttype childStdin_variable;
    objecttype childStdout_variable;
    arraytype parameters;
    memsizetype arraySize;
    rtlArraytype rtlParameters;
    memsizetype pos;

  /* cmd_pipe2 */
    isit_stri(arg_1(arguments));
    isit_array(arg_2(arguments));
    parameters = take_array(arg_2(arguments));
    childStdin_variable = arg_3(arguments);
    isit_file(childStdin_variable);
    childStdout_variable = arg_4(arguments);
    isit_file(childStdout_variable);
    arraySize = (uinttype) (parameters->max_position - parameters->min_position + 1);
    if (!ALLOC_RTL_ARRAY(rtlParameters, arraySize)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      rtlParameters->min_position = parameters->min_position;
      rtlParameters->max_position = parameters->max_position;
      for (pos = 0; pos < arraySize; pos++) {
        rtlParameters->arr[pos].value.strivalue = parameters->arr[pos].value.strivalue;
      } /* for */
      cmdPipe2(take_stri(arg_1(arguments)), rtlParameters,
               &childStdin_variable->value.filevalue,
               &childStdout_variable->value.filevalue);
      FREE_RTL_ARRAY(rtlParameters, arraySize);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_pipe2 */



#ifdef ANSI_C

objecttype cmd_pty (listtype arguments)
#else

objecttype cmd_pty (arguments)
listtype arguments;
#endif

  {
    objecttype childStdin_variable;
    objecttype childStdout_variable;
    arraytype parameters;
    memsizetype arraySize;
    rtlArraytype rtlParameters;
    memsizetype pos;

  /* cmd_pty */
    isit_stri(arg_1(arguments));
    isit_array(arg_2(arguments));
    parameters = take_array(arg_2(arguments));
    childStdin_variable = arg_3(arguments);
    isit_file(childStdin_variable);
    childStdout_variable = arg_4(arguments);
    isit_file(childStdout_variable);
    arraySize = (uinttype) (parameters->max_position - parameters->min_position + 1);
    if (!ALLOC_RTL_ARRAY(rtlParameters, arraySize)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      rtlParameters->min_position = parameters->min_position;
      rtlParameters->max_position = parameters->max_position;
      for (pos = 0; pos < arraySize; pos++) {
        rtlParameters->arr[pos].value.strivalue = parameters->arr[pos].value.strivalue;
      } /* for */
      cmdPty(take_stri(arg_1(arguments)), rtlParameters,
               &childStdin_variable->value.filevalue,
               &childStdout_variable->value.filevalue);
      FREE_RTL_ARRAY(rtlParameters, arraySize);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_pty */



#ifdef ANSI_C

objecttype cmd_readlink (listtype arguments)
#else

objecttype cmd_readlink (arguments)
listtype arguments;
#endif

  { /* cmd_readlink */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdReadlink(take_stri(arg_1(arguments))));
  } /* cmd_readlink */



#ifdef ANSI_C

objecttype cmd_remove (listtype arguments)
#else

objecttype cmd_remove (arguments)
listtype arguments;
#endif

  { /* cmd_remove */
    isit_stri(arg_1(arguments));
    cmdRemove(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_remove */



#ifdef ANSI_C

objecttype cmd_remove_any_file (listtype arguments)
#else

objecttype cmd_remove_any_file (arguments)
listtype arguments;
#endif

  { /* cmd_remove_any_file */
    isit_stri(arg_1(arguments));
    cmdRemoveAnyFile(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_remove_any_file */



#ifdef ANSI_C

objecttype cmd_setenv (listtype arguments)
#else

objecttype cmd_setenv (arguments)
listtype arguments;
#endif

  { /* cmd_setenv */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdSetenv(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_setenv */



#ifdef ANSI_C

objecttype cmd_set_atime (listtype arguments)
#else

objecttype cmd_set_atime (arguments)
listtype arguments;
#endif

  { /* cmd_set_atime */
    isit_stri(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    isit_int(arg_9(arguments));
    cmdSetATime(take_stri(arg_1(arguments)),
                take_int(arg_2(arguments)),
                take_int(arg_3(arguments)),
                take_int(arg_4(arguments)),
                take_int(arg_5(arguments)),
                take_int(arg_6(arguments)),
                take_int(arg_7(arguments)),
                take_int(arg_8(arguments)),
                take_int(arg_9(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_set_atime */



#ifdef ANSI_C

objecttype cmd_set_filemode (listtype arguments)
#else

objecttype cmd_set_filemode (arguments)
listtype arguments;
#endif

  { /* cmd_set_filemode */
    isit_stri(arg_1(arguments));
    isit_set(arg_2(arguments));
    cmdSetFileMode(take_stri(arg_1(arguments)),
                   take_set(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_set_filemode */



#ifdef ANSI_C

objecttype cmd_set_mtime (listtype arguments)
#else

objecttype cmd_set_mtime (arguments)
listtype arguments;
#endif

  { /* cmd_set_mtime */
    isit_stri(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    isit_int(arg_9(arguments));
    cmdSetMTime(take_stri(arg_1(arguments)),
                take_int(arg_2(arguments)),
                take_int(arg_3(arguments)),
                take_int(arg_4(arguments)),
                take_int(arg_5(arguments)),
                take_int(arg_6(arguments)),
                take_int(arg_7(arguments)),
                take_int(arg_8(arguments)),
                take_int(arg_9(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_set_mtime */



#ifdef ANSI_C

objecttype cmd_shell (listtype arguments)
#else

objecttype cmd_shell (arguments)
listtype arguments;
#endif

  { /* cmd_shell */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_int_temp(
        cmdShell(take_stri(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* cmd_shell */



#ifdef ANSI_C

objecttype cmd_shell_escape (listtype arguments)
#else

objecttype cmd_shell_escape (arguments)
listtype arguments;
#endif

  { /* cmd_shell_escape */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdShellEscape(take_stri(arg_1(arguments))));
  } /* cmd_shell_escape */



#ifdef ANSI_C

objecttype cmd_start_process (listtype arguments)
#else

objecttype cmd_start_process (arguments)
listtype arguments;
#endif

  {
    arraytype parameters;
    memsizetype arraySize;
    rtlArraytype rtlParameters;
    memsizetype pos;

  /* cmd_start_process */
    isit_stri(arg_1(arguments));
    isit_array(arg_2(arguments));
    parameters = take_array(arg_2(arguments));
    arraySize = (uinttype) (parameters->max_position - parameters->min_position + 1);
    if (!ALLOC_RTL_ARRAY(rtlParameters, arraySize)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      rtlParameters->min_position = parameters->min_position;
      rtlParameters->max_position = parameters->max_position;
      for (pos = 0; pos < arraySize; pos++) {
        rtlParameters->arr[pos].value.strivalue = parameters->arr[pos].value.strivalue;
      } /* for */
      cmdStartProcess(take_stri(arg_1(arguments)), rtlParameters);
      FREE_RTL_ARRAY(rtlParameters, arraySize);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_start_process */



#ifdef ANSI_C

objecttype cmd_symlink (listtype arguments)
#else

objecttype cmd_symlink (arguments)
listtype arguments;
#endif

  { /* cmd_symlink */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdSymlink(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_symlink */



#ifdef ANSI_C

objecttype cmd_to_os_path (listtype arguments)
#else

objecttype cmd_to_os_path (arguments)
listtype arguments;
#endif

  { /* cmd_to_os_path */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdToOsPath(take_stri(arg_1(arguments))));
  } /* cmd_to_os_path */
