/********************************************************************/
/*                                                                  */
/*  cmd_unx.c     Command functions which call the Unix API.        */
/*  Copyright (C) 1989 - 2012  Thomas Mertes                        */
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
/*  File: seed7/src/cmd_unx.c                                       */
/*  Changes: 2010, 2012  Thomas Mertes                              */
/*  Content: Command functions which call the Unix API.             */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#define _XOPEN_SOURCE
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sys/types.h"
#include "fcntl.h"
#include "signal.h"
#include "errno.h"

#if UNISTD_H_PRESENT
#include "unistd.h"
#endif

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "arr_rtl.h"
#include "cmd_rtl.h"
#include "str_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "cmd_drv.h"


#ifndef PATH_MAX
#define PATH_MAX 2048
#endif



/**
 *  Get the absolute path of the executable of the current process.
 *  @param arg_0 Parameter argv[0] from the function main() as string.
 *  @return the absolute path of the current process.
 */
striType getExecutablePath (const const_striType arg_0)

  {
#if HAS_SYMBOLIC_LINKS
    os_charType buffer[PATH_MAX];
    ssize_t readlink_result;
    errInfoType err_info = OKAY_NO_ERROR;
#ifdef APPEND_EXTENSION_TO_EXECUTABLE_PATH
    striType exeExtension;
#endif
#endif
    striType cwd;
    striType result;

  /* getExecutablePath */
    logFunction(printf("getExecutablePath\n"););
#if HAS_SYMBOLIC_LINKS
    readlink_result = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (readlink_result != -1) {
      buffer[readlink_result] = '\0';
      result = cp_from_os_path(buffer, &err_info);
      if (unlikely(result == NULL)) {
        logError(printf("getExecutablePath: cp_from_os_path failed.\n"););
        raise_error(err_info);
#ifdef APPEND_EXTENSION_TO_EXECUTABLE_PATH
      } else {
        exeExtension = cstri8_or_cstri_to_stri(EXECUTABLE_FILE_EXTENSION);
        result = strConcat(result, exeExtension);
        FREE_STRI(exeExtension, exeExtension->size);
#endif
      } /* if */
    } else {
#endif
      if (strChPos(arg_0, (charType) '/') == 0) {
        result = examineSearchPath(arg_0);
      } else if (arg_0->size >= 1 && arg_0->mem[0] == (charType) '/') {
        result = strCreate(arg_0);
      } else {
        cwd = cmdGetcwd();
        result = concatPath(cwd, arg_0);
        FREE_STRI(cwd, cwd->size);
      } /* if */
      if (unlikely(result == NULL)) {
        raise_error(MEMORY_ERROR);
#if HAS_SYMBOLIC_LINKS
      } else {
        result = followLink(result);
#endif
      } /* if */
#if HAS_SYMBOLIC_LINKS
    } /* if */
#endif
    logFunction(printf("getExecutablePath --> \"%s\"\n",
                       striAsUnquotedCStri(result)););
    return result;
  } /* getExecutablePath */
