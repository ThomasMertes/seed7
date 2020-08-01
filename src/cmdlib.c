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
#include "data_rtl.h"
#include "os_decls.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "arrutl.h"
#include "objutl.h"
#include "runerr.h"
#include "dir_rtl.h"
#include "str_rtl.h"
#include "cmd_rtl.h"
#include "cmd_drv.h"

#undef EXTERN
#define EXTERN
#include "cmdlib.h"



#ifdef USE_CDECL
static int _cdecl cmp_mem (char *strg1, char *strg2)
#else
static int cmp_mem (void const *strg1, void const *strg2)
#endif

  { /* cmp_mem */
    return (int) strCompare(
        ((const_objectType) strg1)->value.striValue,
        ((const_objectType) strg2)->value.striValue);
  } /* cmp_mem */



static arrayType read_dir (dirType directory)

  {
    arrayType dir_array;
    arrayType resized_dir_array;
    memSizeType max_array_size;
    intType used_array_size;
    memSizeType position;
    striType stri1;
    boolType okay;

  /* read_dir */
    max_array_size = 256;
    if (ALLOC_ARRAY(dir_array, max_array_size)) {
      used_array_size = 0;
      stri1 = dirRead(directory);
      okay = TRUE;
      while (okay && stri1 != NULL) {
        if ((memSizeType) used_array_size >= max_array_size) {
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
          dir_array->arr[used_array_size].value.striValue = stri1;
          INIT_CATEGORY_OF_VAR(&dir_array->arr[used_array_size],
              STRIOBJECT);
          used_array_size++;
          stri1 = dirRead(directory);
        } /* if */
      } /* while */
      if (okay) {
        resized_dir_array = REALLOC_ARRAY(dir_array,
            max_array_size, (memSizeType) used_array_size);
        if (resized_dir_array == NULL) {
          okay = FALSE;
        } else {
          dir_array = resized_dir_array;
          COUNT3_ARRAY(max_array_size, (memSizeType) used_array_size);
          dir_array->min_position = 1;
          dir_array->max_position = used_array_size;
        } /* if */
      } /* if */
      if (!okay) {
        for (position = 0; position < (memSizeType) used_array_size; position++) {
          FREE_STRI(dir_array->arr[position].value.striValue,
              dir_array->arr[position].value.striValue->size);
        } /* for */
        FREE_ARRAY(dir_array, max_array_size);
        dir_array = NULL;
      } /* if */
    } /* if */
    return dir_array;
  } /* read_dir */



static objectType toArrayType (rtlArrayType anRtlArray)

  {
    memSizeType arraySize;
    memSizeType pos;
    arrayType anArray;
    objectType result;

  /* toArrayType */
    if (anRtlArray == NULL) {
      /* Assume that an exception was already raised */
      result = NULL;
    } else {
      arraySize = arraySize(anRtlArray);
      if (!ALLOC_ARRAY(anArray, arraySize)) {
        for (pos = 0; pos < arraySize; pos++) {
          strDestr(anRtlArray->arr[pos].value.striValue);
        } /* for */
        FREE_RTL_ARRAY(anRtlArray, arraySize);
        result = raise_exception(SYS_MEM_EXCEPTION);
      } else {
        anArray->min_position = anRtlArray->min_position;
        anArray->max_position = anRtlArray->max_position;
        for (pos = 0; pos < arraySize; pos++) {
          anArray->arr[pos].type_of = take_type(SYS_STRI_TYPE);
          anArray->arr[pos].descriptor.property = NULL;
          anArray->arr[pos].value.striValue = anRtlArray->arr[pos].value.striValue;
          INIT_CATEGORY_OF_VAR(&anArray->arr[pos], STRIOBJECT);
        } /* for */
        FREE_RTL_ARRAY(anRtlArray, arraySize);
        result = bld_array_temp(anArray);
      } /* if */
    } /* if */
    return result;
  } /* toArrayType */



/**
 *  Determine the size of a file.
 *  The file size is measured in bytes.
 *  For directories a size of 0 is returned.
 *  @return the size of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR It was not possible to determine the file size.
 */
objectType cmd_big_filesize (listType arguments)

  { /* cmd_big_filesize */
    isit_stri(arg_1(arguments));
    return bld_bigint_temp(
        cmdBigFileSize(take_stri(arg_1(arguments))));
  } /* cmd_big_filesize */



/**
 *  Changes the current working directory of the calling process.
 *  @exception MEMORY_ERROR Not enough memory to convert 'dirPath' to
 *             the system path type.
 *  @exception RANGE_ERROR 'dirPath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_chdir (listType arguments)

  { /* cmd_chdir */
    isit_stri(arg_1(arguments));
    cmdChdir(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_chdir */



/**
 *  Clone a file or directory tree.
 *  Permissions/mode, ownership and timestamps of the original are
 *  preserved. Symlinks are not followed. Instead the symlink is
 *  copied. Note that 'cloneFile' does not preserve hard links (they
 *  are resolved to distinct files).
 *  @exception MEMORY_ERROR Not enough memory to convert
 *             'sourcePath' or 'destPath' to the system path type.
 *  @exception RANGE_ERROR 'sourcePath' or 'destPath' does not use
 *             the standard path representation or one of them cannot be
 *             converted to the system path type.
 *  @exception FILE_ERROR Source file does not exist, destination file
 *             already exists or a system function returns an error.
 */
objectType cmd_clone_file (listType arguments)

  { /* cmd_clone_file */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdCloneFile(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_clone_file */



objectType cmd_config_value (listType arguments)

  { /* cmd_config_value */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdConfigValue(take_stri(arg_1(arguments))));
  } /* cmd_config_value */



/**
 *  Copy a file or directory tree.
 *  Permissions/mode, ownership and timestamps of the destination file
 *  are determined independent of the corresponding source properties.
 *  The destination file gets the permissions/mode defined by umask.
 *  The user executing the program is the owner of the destination file.
 *  The timestamps of the destination file are set to the current time.
 *  Symbolic links in 'sourcePath' are always followed.
 *  Therefore 'copyFile' will never create a symbolic link.
 *  Note that 'copyFile' does not preserve hard links (they are
 *  resolved to distinct files).
 *  @exception MEMORY_ERROR Not enough memory to convert 'sourcePath'
 *             or 'destPath' to the system path type.
 *  @exception RANGE_ERROR 'sourcePath' or 'destPath' does not use
 *             the standard path representation or one of them cannot be
 *             converted to the system path type.
 *  @exception FILE_ERROR Source file does not exist, destination file
 *             already exists or a system function returns an error.
 */
objectType cmd_copy_file (listType arguments)

  { /* cmd_copy_file */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdCopyFile(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_copy_file */



/**
 *  Returns the list of environment variable names as array of strings.
 *  @return the list of environment variable names.
 *  @exception MEMORY_ERROR Not enough memory to create the result.
 */
objectType cmd_environment (listType arguments)

  { /* cmd_environment */
    return toArrayType(cmdEnvironment());
  } /* cmd_environment */



/**
 *  Determine the file mode (permissions) of a file.
 *  @return the file mode.
 *  @exception MEMORY_ERROR Not enough memory to convert ''filePath''
 *             to the system path type.
 *  @exception RANGE_ERROR ''filePath'' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_filemode (listType arguments)

  { /* cmd_filemode */
    isit_stri(arg_1(arguments));
    return bld_set_temp(
        cmdFileMode(take_stri(arg_1(arguments))));
  } /* cmd_filemode */



/**
 *  Determine the size of a file.
 *  The file size is measured in bytes.
 *  For directories a size of 0 is returned.
 *  @return the size of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception RANGE_ERROR The file size is not representable as integer.
 *  @exception FILE_ERROR It was not possible to determine the file size.
 */
objectType cmd_filesize (listType arguments)

  { /* cmd_filesize */
    isit_stri(arg_1(arguments));
    return bld_int_temp(
        cmdFileSize(take_stri(arg_1(arguments))));
  } /* cmd_filesize */



/**
 *  Determine the type of a file.
 *  The function does follow symbolic links. Therefore it never
 *  returns 'FILE_SYMLINK'. A return value of 'FILE_ABSENT' does
 *  not imply that a file with this name can be created, since missing
 *  directories and illegal file names cause also 'FILE_ABSENT'.
 *  @return the type of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation.
 *  @exception FILE_ERROR The system function returns an error other
 *             than ENOENT, ENOTDIR or ENAMETOOLONG.
 */
objectType cmd_filetype (listType arguments)

  { /* cmd_filetype */
    isit_stri(arg_1(arguments));
    return bld_int_temp(
        cmdFileType(take_stri(arg_1(arguments))));
  } /* cmd_filetype */



/**
 *  Determine the type of a file.
 *  The function does not follow symbolic links. Therefore it may
 *  return 'FILE_SYMLINK'. A return value of 'FILE_ABSENT' does
 *  not imply that a file with this name can be created, since missing
 *  directories and illegal file names cause also 'FILE_ABSENT'.
 *  @return the type of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation.
 *  @exception FILE_ERROR The system function returns an error other
 *             than ENOENT, ENOTDIR or ENAMETOOLONG.
 */
objectType cmd_filetype_sl (listType arguments)

  { /* cmd_filetype_sl */
    isit_stri(arg_1(arguments));
    return bld_int_temp(
        cmdFileTypeSL(take_stri(arg_1(arguments))));
  } /* cmd_filetype_sl */



/**
 *  Determine the current working directory of the calling process.
 *  @return The absolute path of the current working directory.
 *  @exception MEMORY_ERROR Not enough memory to represent the
 *             result string.
 *  @exception FILE_ERROR The system function returns an error.
 */
objectType cmd_getcwd (listType arguments)

  { /* cmd_getcwd */
    return bld_stri_temp(cmdGetcwd());
  } /* cmd_getcwd */



/**
 *  Determine the value of an environment variable.
 *  The function getenv searches the environment for an environment variable
 *  with the given 'name'. When such an environment variable exists the
 *  corresponding string value is returned.
 *  @return the value of an environment variable or "",
 *          when the requested environment variable does not exist.
 *  @exception MEMORY_ERROR Not enough memory to convert 'name' to the
 *             system string type or not enough memory to represent the
 *             result string.
 */
objectType cmd_getenv (listType arguments)

  { /* cmd_getenv */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdGetenv(take_stri(arg_1(arguments))));
  } /* cmd_getenv */



/**
 *  Determine the access time of a file.
 *  @return the access time of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_get_atime (listType arguments)

  {
    boolType is_dst;

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
            &arg_2(arguments)->value.intValue,
            &arg_3(arguments)->value.intValue,
            &arg_4(arguments)->value.intValue,
            &arg_5(arguments)->value.intValue,
            &arg_6(arguments)->value.intValue,
            &arg_7(arguments)->value.intValue,
            &arg_8(arguments)->value.intValue,
            &arg_9(arguments)->value.intValue,
            &is_dst);
    if (is_dst) {
      arg_10(arguments)->value.objValue = SYS_TRUE_OBJECT;
    } else {
      arg_10(arguments)->value.objValue = SYS_FALSE_OBJECT;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_get_atime */



/**
 *  Determine the change time of a file.
 *  @return the change time of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_get_ctime (listType arguments)

  {
    boolType is_dst;

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
            &arg_2(arguments)->value.intValue,
            &arg_3(arguments)->value.intValue,
            &arg_4(arguments)->value.intValue,
            &arg_5(arguments)->value.intValue,
            &arg_6(arguments)->value.intValue,
            &arg_7(arguments)->value.intValue,
            &arg_8(arguments)->value.intValue,
            &arg_9(arguments)->value.intValue,
            &is_dst);
    if (is_dst) {
      arg_10(arguments)->value.objValue = SYS_TRUE_OBJECT;
    } else {
      arg_10(arguments)->value.objValue = SYS_FALSE_OBJECT;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_get_ctime */



/**
 *  Determine the modification time of a file.
 *  @return the modification time of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_get_mtime (listType arguments)

  {
    boolType is_dst;

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
            &arg_2(arguments)->value.intValue,
            &arg_3(arguments)->value.intValue,
            &arg_4(arguments)->value.intValue,
            &arg_5(arguments)->value.intValue,
            &arg_6(arguments)->value.intValue,
            &arg_7(arguments)->value.intValue,
            &arg_8(arguments)->value.intValue,
            &arg_9(arguments)->value.intValue,
            &is_dst);
    if (is_dst) {
      arg_10(arguments)->value.objValue = SYS_TRUE_OBJECT;
    } else {
      arg_10(arguments)->value.objValue = SYS_FALSE_OBJECT;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_get_mtime */



/**
 *  Returns the search path of the system as array of strings.
 *  @return the search path of the system.
 *  @exception MEMORY_ERROR Not enough memory to create the result.
 */
objectType cmd_getSearchPath (listType arguments)

  { /* cmd_getSearchPath */
    return toArrayType(cmdGetSearchPath());
  } /* cmd_getSearchPath */



/**
 *  Determine the home directory of the user.
 *  This function should be preferred over the use of an environment
 *  variable such as $HOME. $HOME is not supported under all operating
 *  systems and it is not guaranteed, that it uses the standard path
 *  representation.
 *  @return The absolute path of the home directory.
 *  @exception MEMORY_ERROR Not enough memory to represent the
 *             result string.
 *  @exception FILE_ERROR Not able to determine the home directory.
 */
objectType cmd_homeDir (listType arguments)

  { /* cmd_homeDir */
    return bld_stri_temp(cmdHomeDir());
  } /* cmd_homeDir */



/**
 *  Determine the filenames in a directory.
 *  The files "." and ".." are left out from the result.
 *  Note that the function returns only the filenames.
 *  Additional information must be obtained with other calls.
 *  @return a string-array containing the filenames in the directory.
 *  @exception MEMORY_ERROR Not enough memory to convert 'dirPath'
 *             to the system path type or not enough memory to
 *             represent the result 'string array'.
 *  @exception RANGE_ERROR 'dirPath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_ls (listType arguments)

  {
    striType dirPath;
    dirType directory;
    arrayType result;

  /* cmd_ls */
    isit_stri(arg_1(arguments));
    dirPath = take_stri(arg_1(arguments));
    if ((directory = dirOpen(dirPath)) != NULL) {
      result = read_dir(directory);
      dirClose(directory);
      if (result == NULL) {
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } else {
        qsort((void *) result->arr, (size_t) arraySize(result),
            sizeof(objectRecord), &cmp_mem);
        return bld_array_temp(result);
      } /* if */
    } else {
      return raise_with_arguments(SYS_FIL_EXCEPTION, arguments);
    } /* if */
  } /* cmd_ls */



/**
 *  Creates a new directory.
 *  @exception MEMORY_ERROR Not enough memory to convert 'dirPath' to
 *             the system path type.
 *  @exception RANGE_ERROR 'dirPath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_mkdir (listType arguments)

  { /* cmd_mkdir */
    isit_stri(arg_1(arguments));
    cmdMkdir(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_mkdir */



/**
 *  Move and rename a file or directory tree.
 *  The function uses the C 'rename()' function. When 'rename()' fails
 *  the file (or directory tree) is cloned with 'cloneFile' (which
 *  preserves permissions/mode, ownership and timestamps) to the new
 *  place and with the new name. When 'cloneFile' succeeds the original
 *  file is deleted. When 'cloneFile' fails (no space on device or
 *  other reason) all remains of the failed clone are removed. Note
 *  that 'cloneFile' works for symbolic links but does not preserve
 *  hard links (they are resolved to distinct files).
 *  @exception MEMORY_ERROR Not enough memory to convert 'sourcePath'
 *             or 'destPath' to the system path type.
 *  @exception RANGE_ERROR 'sourcePath' or 'destPath' does not use
 *             the standard path representation or one of them cannot be
 *             converted to the system path type.
 *  @exception FILE_ERROR Source file does not exist, destination file
 *             already exists or a system function returns an error.
 */
objectType cmd_move (listType arguments)

  { /* cmd_move */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdMove(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_move */



objectType cmd_pipe2 (listType arguments)

  {
    objectType childStdin_variable;
    objectType childStdout_variable;
    rtlArrayType parameters;

  /* cmd_pipe2 */
    isit_stri(arg_1(arguments));
    isit_array(arg_2(arguments));
    childStdin_variable = arg_3(arguments);
    isit_file(childStdin_variable);
    childStdout_variable = arg_4(arguments);
    isit_file(childStdout_variable);
    parameters = gen_rtl_array(take_array(arg_2(arguments)));
    if (parameters == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      cmdPipe2(take_stri(arg_1(arguments)), parameters,
               &childStdin_variable->value.fileValue,
               &childStdout_variable->value.fileValue);
      FREE_RTL_ARRAY(parameters, ARRAY_LENGTH(parameters));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_pipe2 */



objectType cmd_pty (listType arguments)

  {
    objectType childStdin_variable;
    objectType childStdout_variable;
    rtlArrayType parameters;

  /* cmd_pty */
    isit_stri(arg_1(arguments));
    isit_array(arg_2(arguments));
    childStdin_variable = arg_3(arguments);
    isit_file(childStdin_variable);
    childStdout_variable = arg_4(arguments);
    isit_file(childStdout_variable);
    parameters = gen_rtl_array(take_array(arg_2(arguments)));
    if (parameters == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      cmdPty(take_stri(arg_1(arguments)), parameters,
               &childStdin_variable->value.fileValue,
               &childStdout_variable->value.fileValue);
      FREE_RTL_ARRAY(parameters, ARRAY_LENGTH(parameters));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_pty */



/**
 *  Reads the destination of a symbolic link.
 *  @return The destination refered by the symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type or not enough memory to
 *             represent the result string.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with the path does not
 *             exist or is not a symbolic link.
 */
objectType cmd_readlink (listType arguments)

  { /* cmd_readlink */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdReadlink(take_stri(arg_1(arguments))));
  } /* cmd_readlink */



/**
 *  Remove a file or empty directory.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath' to
 *             the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file does not exist or a system function
 *             returns an error.
 */
objectType cmd_remove (listType arguments)

  { /* cmd_remove */
    isit_stri(arg_1(arguments));
    cmdRemove(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_remove */



/**
 *  Removes a file independent of its file type.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath' to
 *             the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file does not exist or a system function
 *             returns an error.
 */
objectType cmd_remove_any_file (listType arguments)

  { /* cmd_remove_any_file */
    isit_stri(arg_1(arguments));
    cmdRemoveAnyFile(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_remove_any_file */



/**
 *  Add or change an environment variable.
 *  The function setenv searches the environment for an environment variable
 *  with the given 'name'. When such an environment variable exists the
 *  corresponding value is changed to 'value'. When no environment variable
 *  with the given 'name' exists a new environment variable 'name' with
 *  the value 'value' is created.
 *  @exception MEMORY_ERROR Not enough memory to convert 'name' or 'value'
 *             to the system string type.
 *  @exception RANGE_ERROR A system function returns an error.
 */
objectType cmd_setenv (listType arguments)

  { /* cmd_setenv */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdSetenv(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_setenv */



/**
 *  Set the access time of a file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception RANGE_ERROR 'aTime' is invalid or cannot be
 *             converted to the system file time.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_set_atime (listType arguments)

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



/**
 *  Change the file mode (permissions) of a file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_set_filemode (listType arguments)

  { /* cmd_set_filemode */
    isit_stri(arg_1(arguments));
    isit_set(arg_2(arguments));
    cmdSetFileMode(take_stri(arg_1(arguments)),
                   take_set(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_set_filemode */



/**
 *  Set the modification time of a file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception RANGE_ERROR 'aTime' is invalid or cannot be
 *             converted to the system file time.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_set_mtime (listType arguments)

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



/**
 *  Use the shell to execute a 'command' with 'parameters'.
 *  Parameters which contain a space must be enclosed in double
 *  quotes (E.g.: shell("aCommand", "\"par 1\" par2"); ). The
 *  commands supported and the format of the 'parameters' are not
 *  covered by the description of the 'shell' function. Due to the
 *  usage of the operating system shell and external programs, it is
 *  hard to write portable programs, which use the 'shell' function.
 *  @param command/arg_1 Name of the command to be executed. A path
 *         must use the standard path representation.
 *  @param parameters/arg_2 Space separated list of parameters for
 *         the 'command', or "" when there are no parameters.
 *  @return the return code of the executed command or of the shell.
 */
objectType cmd_shell (listType arguments)

  { /* cmd_shell */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_int_temp(
        cmdShell(take_stri(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* cmd_shell */



/**
 *  Convert a string, such that it can be used as shell parameter.
 *  The function adds escape characters or quotations to a string.
 *  The result is useable as parameter for the functions 'cmdShell',
 *  and 'filPopen'. Shell parameters must be escaped individually.
 *  Afterwards escaped parameters are joined to a space separated
 *  list of parameters.
 *  @return a string which can be used as shell parameter.
 *  @exception MEMORY_ERROR Not enough memory to convert 'stri'.
 */
objectType cmd_shell_escape (listType arguments)

  { /* cmd_shell_escape */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdShellEscape(take_stri(arg_1(arguments))));
  } /* cmd_shell_escape */



objectType cmd_start_process (listType arguments)

  {
    rtlArrayType parameters;

  /* cmd_start_process */
    isit_stri(arg_1(arguments));
    isit_array(arg_2(arguments));
    parameters = gen_rtl_array(take_array(arg_2(arguments)));
    if (parameters == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      cmdStartProcess(take_stri(arg_1(arguments)), parameters);
      FREE_RTL_ARRAY(parameters, ARRAY_LENGTH(parameters));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_start_process */



/**
 *  Create a symbolic link.
 *  The symbolic link 'destPath' will refer to 'sourcePath' afterwards.
 *  @param sourcePath/arg_1 String to be contained in the symbolic link.
 *  @param destPath/arg_2 Name of the symbolic link to be created.
 *  @exception MEMORY_ERROR Not enough memory to convert sourcePath or
 *             destPath to the system path type.
 *  @exception RANGE_ERROR 'sourcePath' or 'destPath' does not use the
 *             standard path representation or one of them cannot be
 *             converted to the system path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_symlink (listType arguments)

  { /* cmd_symlink */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdSymlink(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_symlink */



/**
 *  Convert a standard path to the path of the operating system.
 *  The result must be escaped with 'cmdShellEscape' to be useable as
 *  parameter for the functions 'cmdShell' and 'filPopen'.
 *  @param standardPath/arg_1 Path in the standard path representation.
 *  @return a string containing an operating system path.
 *  @exception MEMORY_ERROR Not enough memory to convert 'standardPath'.
 *  @exception RANGE_ERROR 'standardPath' is not representable as operating
 *             system path.
 */
objectType cmd_to_os_path (listType arguments)

  { /* cmd_to_os_path */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdToOsPath(take_stri(arg_1(arguments))));
  } /* cmd_to_os_path */
