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
/*  Content: Primitive actions for various system commands.         */
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
#include "flistutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "arrutl.h"
#include "objutl.h"
#include "runerr.h"
#include "dir_drv.h"
#include "dir_rtl.h"
#include "str_rtl.h"
#include "cmd_rtl.h"
#include "cmd_drv.h"

#undef EXTERN
#define EXTERN
#include "cmdlib.h"



/**
 *  Convert an rtlArrayType to an arrayType object.
 *  The strings in 'anRtlArray' are reused in the created arrayType
 *  object. The memory of 'anRtlArray' itself is freed. It is assumed
 *  that 'anRtlArray' is not used afterwards.
 *  @return the new created arrayType object.
 *  @exception MEMORY_ERROR Not enough memory to convert the path
 *             to the system string type.
 */
static objectType toArrayType (rtlArrayType anRtlArray)

  {
    memSizeType arraySize;
    memSizeType pos;
    arrayType anArray;
    typeType typeOfString;
    objectType element;
    objectType result;

  /* toArrayType */
    if (unlikely(anRtlArray == NULL)) {
      /* Assume that an exception was already raised */
      result = NULL;
    } else {
      arraySize = arraySize(anRtlArray);
      if (unlikely(!ALLOC_ARRAY(anArray, arraySize))) {
        for (pos = 0; pos < arraySize; pos++) {
          strDestr(anRtlArray->arr[pos].value.striValue);
        } /* for */
        FREE_RTL_ARRAY(anRtlArray, arraySize);
        result = raise_exception(SYS_MEM_EXCEPTION);
      } else {
        anArray->min_position = anRtlArray->min_position;
        anArray->max_position = anRtlArray->max_position;
        typeOfString = take_type(SYS_STRI_TYPE);
        pos = arraySize;
        element = &anArray->arr[pos];
        while (pos != 0) {
          pos--;
          element--;
          element->type_of = typeOfString;
          element->descriptor.property = NULL;
          element->value.striValue = anRtlArray->arr[pos].value.striValue;
          INIT_CATEGORY_OF_VAR(element, STRIOBJECT);
        } /* while */
        FREE_RTL_ARRAY(anRtlArray, arraySize);
        result = bld_array_temp(anArray);
      } /* if */
    } /* if */
    return result;
  } /* toArrayType */



/**
 *  Determine the size of a file.
 *  The function follows symbolic links. The file size is measured in bytes.
 *  For directories, fifos and sockets a size of 0 is returned.
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



/**
 *  Get a built-in C compiler/runtime configuration value.
 *  The makefile used to compile Seed7 and the program chkccomp.c
 *  write the configuration values to version.h. The configuration
 *  values are hard-coded in the Seed7 runtime library.
 *  @param name/arg_1 Name of the configuration value to be retrieved.
 *  @return the requested configuration value, and
 *          "" if a value with the name does not exist.
 *  @exception MEMORY_ERROR Not enough memory to convert the
 *             configuration value to a string.
 */
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
 *  Determine the size of a file.
 *  The function follows symbolic links. The file size is measured in bytes.
 *  For directories, fifos and sockets a size of 0 is returned.
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
 *  directories and invalid file names cause also 'FILE_ABSENT'.
 *  @return the type of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation.
 *  @exception FILE_ERROR The system function returns an error other
 *             than ENOENT, ENOTDIR, ENAMETOOLONG or EACCES.
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
 *  directories and invalid file names cause also 'FILE_ABSENT'.
 *  @return the type of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation.
 *  @exception FILE_ERROR The system function returns an error other
 *             than ENOENT, ENOTDIR, ENAMETOOLONG or EACCES.
 */
objectType cmd_filetype_sl (listType arguments)

  { /* cmd_filetype_sl */
    isit_stri(arg_1(arguments));
    return bld_int_temp(
        cmdFileTypeSL(take_stri(arg_1(arguments))));
  } /* cmd_filetype_sl */



/**
 *  Get the final path that functions like getMTime() and open() would use.
 *  If 'filePath' is not a symbolic link it is returned. For a symbolic link
 *  the function follows the symbolic link chain until the path is not a
 *  symbolic link again. The final path may refer to a non-existing file.
 *  @param filePath Relative or absolute path.
 *  @return The final path after possibly following a symbolic link chain.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type or not enough memory to
 *             represent the result string.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist or a system function returns an error.
 */
objectType cmd_final_path (listType arguments)

  { /* cmd_final_path */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdFinalPath(take_stri(arg_1(arguments))));
  } /* cmd_final_path */



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
 *  with the given 'name'. If such an environment variable exists the
 *  corresponding string value is returned.
 *  @return the value of an environment variable, and
 *          "" if the requested environment variable does not exist.
 *  @exception MEMORY_ERROR Not enough memory to convert 'name' to the
 *             system string type or not enough memory to represent the
 *             result string.
 *  @exception RANGE_ERROR 'name' cannot be converted to the system string
 *             type or a system function returns an error.
 */
objectType cmd_getenv (listType arguments)

  { /* cmd_getenv */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdGetenv(take_stri(arg_1(arguments))));
  } /* cmd_getenv */



/**
 *  Determine the access time of a file.
 *  The function follows symbolic links.
 *  @return the access time of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
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
    logFunction(printf("cmd_get_atime(\"%s\") --> "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "."
                       F_D(06) ", " FMT_D ", %d\n",
                       striAsUnquotedCStri(take_stri(arg_1(arguments))),
                       arg_2(arguments)->value.intValue,
                       arg_3(arguments)->value.intValue,
                       arg_4(arguments)->value.intValue,
                       arg_5(arguments)->value.intValue,
                       arg_6(arguments)->value.intValue,
                       arg_7(arguments)->value.intValue,
                       arg_8(arguments)->value.intValue,
                       arg_9(arguments)->value.intValue, is_dst););
    return SYS_EMPTY_OBJECT;
  } /* cmd_get_atime */



/**
 *  Determine the access time of a symbolic link.
 *  The function only works for symbolic links and does not follow the
 *  symbolic link.
 *  @return the access time of the symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or it is not a symbolic link, or a system function
 *             returns an error.
 */
objectType cmd_get_atime_of_symlink (listType arguments)

  {
    boolType is_dst;

  /* cmd_get_atime_of_symlink */
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
    cmdGetATimeOfSymlink(take_stri(arg_1(arguments)),
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
    logFunction(printf("cmd_get_atime_of_symlink(\"%s\") --> "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "."
                       F_D(06) ", " FMT_D ", %d\n",
                       striAsUnquotedCStri(take_stri(arg_1(arguments))),
                       arg_2(arguments)->value.intValue,
                       arg_3(arguments)->value.intValue,
                       arg_4(arguments)->value.intValue,
                       arg_5(arguments)->value.intValue,
                       arg_6(arguments)->value.intValue,
                       arg_7(arguments)->value.intValue,
                       arg_8(arguments)->value.intValue,
                       arg_9(arguments)->value.intValue, is_dst););
    return SYS_EMPTY_OBJECT;
  } /* cmd_get_atime_of_symlink */



/**
 *  Determine the change time of a file.
 *  The function follows symbolic links.
 *  @return the change time of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
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
    logFunction(printf("cmd_get_ctime(\"%s\") --> "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "."
                       F_D(06) ", " FMT_D ", %d\n",
                       striAsUnquotedCStri(take_stri(arg_1(arguments))),
                       arg_2(arguments)->value.intValue,
                       arg_3(arguments)->value.intValue,
                       arg_4(arguments)->value.intValue,
                       arg_5(arguments)->value.intValue,
                       arg_6(arguments)->value.intValue,
                       arg_7(arguments)->value.intValue,
                       arg_8(arguments)->value.intValue,
                       arg_9(arguments)->value.intValue, is_dst););
    return SYS_EMPTY_OBJECT;
  } /* cmd_get_ctime */



/**
 *  Determine the file mode (permissions) of a file.
 *  The function follows symbolic links.
 *  @return the file mode.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
 */
objectType cmd_get_file_mode (listType arguments)

  { /* cmd_get_file_mode */
    isit_stri(arg_1(arguments));
    return bld_set_temp(
        cmdGetFileMode(take_stri(arg_1(arguments))));
  } /* cmd_get_file_mode */



/**
 *  Determine the file mode (permissions) of a symbolic link.
 *  The function only works for symbolic links and does not follow the
 *  symbolic link.
 *  @return the file mode.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or it is not a symbolic link, or a system function
 *             returns an error.
 */
objectType cmd_get_file_mode_of_symlink (listType arguments)

  { /* cmd_get_file_mode_of_symlink */
    isit_stri(arg_1(arguments));
    return bld_set_temp(
        cmdGetFileModeOfSymlink(take_stri(arg_1(arguments))));
  } /* cmd_get_file_mode_of_symlink */



/**
 *  Determine the name of the group (GID) to which a file belongs.
 *  The function follows symbolic links.
 *  @return the name of the file group.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
 */
objectType cmd_get_group (listType arguments)

  { /* cmd_get_group */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdGetGroup(take_stri(arg_1(arguments))));
  } /* cmd_get_group */



/**
 *  Determine the name of the group (GID) to which a symbolic link belongs.
 *  The function only works for symbolic links and does not follow the
 *  symbolic link.
 *  @return the name of the file group.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or it is not a symbolic link, or a system function
 *             returns an error.
 */
objectType cmd_get_group_of_symlink (listType arguments)

  { /* cmd_get_group_of_symlink */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdGetGroupOfSymlink(take_stri(arg_1(arguments))));
  } /* cmd_get_group_of_symlink */



/**
 *  Determine the modification time of a file.
 *  The function follows symbolic links.
 *  @return the modification time of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
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
    logFunction(printf("cmd_get_mtime(\"%s\") --> "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "."
                       F_D(06) ", " FMT_D ", %d\n",
                       striAsUnquotedCStri(take_stri(arg_1(arguments))),
                       arg_2(arguments)->value.intValue,
                       arg_3(arguments)->value.intValue,
                       arg_4(arguments)->value.intValue,
                       arg_5(arguments)->value.intValue,
                       arg_6(arguments)->value.intValue,
                       arg_7(arguments)->value.intValue,
                       arg_8(arguments)->value.intValue,
                       arg_9(arguments)->value.intValue, is_dst););
    return SYS_EMPTY_OBJECT;
  } /* cmd_get_mtime */



/**
 *  Determine the modification time of a symbolic link.
 *  The function only works for symbolic links and does not follow the
 *  symbolic link.
 *  @return the modification time of the symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or it is not a symbolic link, or a system function
 *             returns an error.
 */
objectType cmd_get_mtime_of_symlink (listType arguments)

  {
    boolType is_dst;

  /* cmd_get_mtime_of_symlink */
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
    cmdGetMTimeOfSymlink(take_stri(arg_1(arguments)),
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
    logFunction(printf("cmd_get_mtime_of_symlink(\"%s\") --> "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "."
                       F_D(06) ", " FMT_D ", %d\n",
                       striAsUnquotedCStri(take_stri(arg_1(arguments))),
                       arg_2(arguments)->value.intValue,
                       arg_3(arguments)->value.intValue,
                       arg_4(arguments)->value.intValue,
                       arg_5(arguments)->value.intValue,
                       arg_6(arguments)->value.intValue,
                       arg_7(arguments)->value.intValue,
                       arg_8(arguments)->value.intValue,
                       arg_9(arguments)->value.intValue, is_dst););
    return SYS_EMPTY_OBJECT;
  } /* cmd_get_mtime_of_symlink */



/**
 *  Determine the name of the owner (UID) of a file.
 *  The function follows symbolic links.
 *  @return the name of the file owner.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
 */
objectType cmd_get_owner (listType arguments)

  { /* cmd_get_owner */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdGetOwner(take_stri(arg_1(arguments))));
  } /* cmd_get_owner */



/**
 *  Determine the name of the owner (UID) of a symbolic link.
 *  The function only works for symbolic links and does not follow the
 *  symbolic link.
 *  @return the name of the file owner.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or it is not a symbolic link, or a system function
 *             returns an error.
 */
objectType cmd_get_owner_of_symlink (listType arguments)

  { /* cmd_get_owner_of_symlink */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdGetOwnerOfSymlink(take_stri(arg_1(arguments))));
  } /* cmd_get_owner_of_symlink */



/**
 *  Returns the search path of the system as array of strings.
 *  @return the search path of the system.
 *  @exception MEMORY_ERROR Not enough memory to create the result.
 */
objectType cmd_get_search_path (listType arguments)

  { /* cmd_get_search_path */
    return toArrayType(cmdGetSearchPath());
  } /* cmd_get_search_path */



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
objectType cmd_home_dir (listType arguments)

  { /* cmd_home_dir */
    return bld_stri_temp(cmdHomeDir());
  } /* cmd_home_dir */



/**
 *  Creates a new directory.
 *  @exception MEMORY_ERROR Not enough memory to convert 'dirPath' to
 *             the system path type.
 *  @exception RANGE_ERROR 'dirPath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_make_dir (listType arguments)

  { /* cmd_make_dir */
    isit_stri(arg_1(arguments));
    cmdMakeDir(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_make_dir */



/**
 *  Create a symbolic link.
 *  The symbolic link 'symlinkPath' will refer to 'targetPath' afterwards.
 *  @param symlinkPath/arg_1 Name of the symbolic link to be created.
 *  @param targetPath/arg_2 String to be contained in the symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'symlinkPath' or
 *             'targetPath' to the system path type.
 *  @exception RANGE_ERROR 'symlinkPath' or 'targetPath' does not use the
 *             standard path representation or one of them cannot be
 *             converted to the system path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType cmd_make_link (listType arguments)

  { /* cmd_make_link */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdMakeLink(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_make_link */



/**
 *  Move and rename a file or directory tree.
 *  The function uses the C 'rename()' function. If 'rename()' fails
 *  the file (or directory tree) is cloned with 'cloneFile' (which
 *  preserves permissions/mode, ownership and timestamps) to the new
 *  place and with the new name. If 'cloneFile' succeeds the original
 *  file is deleted. If 'cloneFile' fails (no space on device or
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



/**
 *  Determine the filenames in a directory.
 *  The function does follow symbolic links.
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
 *  @exception FILE_ERROR The file described with 'dirPath' does not
 *             exist, or it is not a directory, or a system function
 *             returns an error.
 */
objectType cmd_read_dir (listType arguments)

  {
    striType dirPath;

  /* cmd_read_dir */
    isit_stri(arg_1(arguments));
    dirPath = take_stri(arg_1(arguments));
    return toArrayType(cmdReadDir(dirPath));
  } /* cmd_read_dir */



/**
 *  Reads the destination of a symbolic link.
 *  This function reads the link destination from the file system
 *  without any change. Symbolic links can be relative or absolute.
 *  Relative symbolic links are relative to their place in the
 *  file system and not relative to the current working directory.
 *  @param filePath/arg_1 Relative or absolute path of a symbolic link.
 *  @return The destination referred by the symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type or not enough memory to
 *             represent the result string.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist or is not a symbolic link.
 */
objectType cmd_read_link (listType arguments)

  { /* cmd_read_link */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdReadLink(take_stri(arg_1(arguments))));
  } /* cmd_read_link */



/**
 *  Reads the absolute destination path of a symbolic link.
 *  Symbolic links can be relative or absolute. Relative symbolic links
 *  are relative to their place in the file system and not relative to
 *  the current working directory. For a relative symbolic link
 *  'filePath' is converted to an absolute path and the symbolic
 *  link is concatenated to that path.
 *  @param filePath/arg_1 Relative or absolute path of a symbolic link.
 *  @return The absolute destination path referred by the symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type or not enough memory to
 *             represent the result string.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist or is not a symbolic link.
 */
objectType cmd_read_link_absolute (listType arguments)

  { /* cmd_read_link_absolute */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        cmdReadLinkAbsolute(take_stri(arg_1(arguments))));
  } /* cmd_read_link_absolute */



/**
 *  Remove a file of any type unless it is a directory that is not empty.
 *  An attempt to remove a directory that is not empty triggers FILE_ERROR.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath' to
 *             the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file does not exist or it is a directory
 *             that is not empty or a system function returns an error.
 */
objectType cmd_remove_file (listType arguments)

  { /* cmd_remove_file */
    isit_stri(arg_1(arguments));
    cmdRemoveFile(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_remove_file */



/**
 *  Remove a file of any type inclusive a directory tree.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath' to
 *             the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file does not exist or a system function
 *             returns an error.
 */
objectType cmd_remove_tree (listType arguments)

  { /* cmd_remove_tree */
    isit_stri(arg_1(arguments));
    cmdRemoveTree(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_remove_tree */



/**
 *  Add or change an environment variable.
 *  The function setenv searches the environment for an environment variable
 *  with the given 'name'. If such an environment variable exists the
 *  corresponding value is changed to 'value'. If no environment variable
 *  with the given 'name' exists a new environment variable 'name' with
 *  the value 'value' is created.
 *  @exception MEMORY_ERROR Not enough memory to convert 'name' or 'value'
 *             to the system string type.
 *  @exception RANGE_ERROR 'name' or 'value' cannot be converted to the
 *             system string type or a system function returns an error.
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
 *  The function follows symbolic links.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception RANGE_ERROR The time is invalid or cannot be
 *             converted to the system time.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
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
 *  The function follows symbolic links.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
 */
objectType cmd_set_file_mode (listType arguments)

  { /* cmd_set_filemode */
    isit_stri(arg_1(arguments));
    isit_set(arg_2(arguments));
    cmdSetFileMode(take_stri(arg_1(arguments)),
                   take_set(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_set_file_mode */



/**
 *  Set the group of a file.
 *  The function follows symbolic links.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
 */
objectType cmd_set_group (listType arguments)

  { /* cmd_set_group */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdSetGroup(take_stri(arg_1(arguments)),
                take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_set_group */



/**
 *  Set the group of a symbolic link.
 *  The function only works for symbolic links and does not follow the
 *  symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or it is not a symbolic link, or a system function
 *             returns an error.
 */
objectType cmd_set_group_of_symlink (listType arguments)

  { /* cmd_set_group_of_symlink */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdSetGroupOfSymlink(take_stri(arg_1(arguments)),
                         take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_set_group_of_symlink */



/**
 *  Set the modification time of a file.
 *  The function follows symbolic links.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception RANGE_ERROR The time is invalid or cannot be
 *             converted to the system time.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
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
 *  Set the modification time of a symbolic link.
 *  The function only works for symbolic links and does not follow the
 *  symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception RANGE_ERROR The time is invalid or cannot be
 *             converted to the system time.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or it is not a symbolic link, or a system function
 *             returns an error.
 */
objectType cmd_set_mtime_of_symlink (listType arguments)

  { /* cmd_set_mtime_of_symlink */
    isit_stri(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    isit_int(arg_9(arguments));
    cmdSetMTimeOfSymlink(take_stri(arg_1(arguments)),
                         take_int(arg_2(arguments)),
                         take_int(arg_3(arguments)),
                         take_int(arg_4(arguments)),
                         take_int(arg_5(arguments)),
                         take_int(arg_6(arguments)),
                         take_int(arg_7(arguments)),
                         take_int(arg_8(arguments)),
                         take_int(arg_9(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_set_mtime_of_symlink */



/**
 *  Set the owner of a file.
 *  The function follows symbolic links.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
 */
objectType cmd_set_owner (listType arguments)

  { /* cmd_set_owner */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdSetOwner(take_stri(arg_1(arguments)),
                take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_set_owner */



/**
 *  Set the owner of a symbolic link.
 *  The function only works for symbolic links and does not follow the
 *  symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or it is not a symbolic link, or a system function
 *             returns an error.
 */
objectType cmd_set_owner_of_symlink (listType arguments)

  { /* cmd_set_owner_of_symlink */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdSetOwnerOfSymlink(take_stri(arg_1(arguments)),
                         take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_set_owner_of_symlink */



/**
 *  Sets the search path from an array of strings.
 *  The search path is used by the current process and its sub processes.
 *  The path of parent processes is not affected by this function.
 *  @exception MEMORY_ERROR Not enough memory to convert the path
 *             to the system string type.
 *  @exception RANGE_ERROR The path cannot be converted to the
 *             system string type or a system function returns an error.
 */
objectType cmd_set_search_path (listType arguments)

  {
    rtlArrayType searchPath;

  /* cmd_set_search_path */
    isit_array(arg_1(arguments));
    searchPath = gen_rtl_array(take_array(arg_1(arguments)));
    if (unlikely(searchPath == NULL)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      cmdSetSearchPath(searchPath);
      FREE_RTL_ARRAY(searchPath, ARRAY_LENGTH(searchPath));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* cmd_set_search_path */



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
 *         the 'command', or "" if there are no parameters.
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



/**
 *  Deletes the variable 'name' from the environment.
 *  If 'name' does not exist in the environment,
 *  then the function succeeds, and the environment is unchanged.
 *  @exception MEMORY_ERROR Not enough memory to convert 'name' to the
 *             system string type.
 *  @exception RANGE_ERROR 'name' cannot be converted to the system string
 *             type or a system function returns an error.
 */
objectType cmd_unsetenv (listType arguments)

  { /* cmd_unsetenv */
    isit_stri(arg_1(arguments));
    cmdUnsetenv(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* cmd_unsetenv */
