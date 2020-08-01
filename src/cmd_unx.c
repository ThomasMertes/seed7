/********************************************************************/
/*                                                                  */
/*  cmd_unx.c     Command functions which call the Unix API.        */
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

#ifdef EMULATE_ENVIRONMENT
char **environ7 = NULL;
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
    striType executablePath;

  /* getExecutablePath */
    logFunction(printf("getExecutablePath\n"););
#if HAS_SYMBOLIC_LINKS
    readlink_result = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (readlink_result != -1) {
      buffer[readlink_result] = '\0';
      executablePath = cp_from_os_path(buffer, &err_info);
      if (unlikely(executablePath == NULL)) {
        logError(printf("getExecutablePath: cp_from_os_path failed.\n"););
        raise_error(err_info);
#ifdef APPEND_EXTENSION_TO_EXECUTABLE_PATH
      } else {
        exeExtension = CSTRI_LITERAL_TO_STRI(EXECUTABLE_FILE_EXTENSION);
        strAppendTemp(&executablePath, exeExtension);
#endif
      } /* if */
    } else {
#endif
      if (strChPos(arg_0, (charType) '/') == 0) {
        executablePath = examineSearchPath(arg_0);
      } else if (arg_0->size >= 1 && arg_0->mem[0] == (charType) '/') {
        executablePath = strCreate(arg_0);
      } else {
        cwd = cmdGetcwd();
        executablePath = concatPath(cwd, arg_0);
        FREE_STRI(cwd, cwd->size);
      } /* if */
      if (unlikely(executablePath == NULL)) {
        raise_error(MEMORY_ERROR);
#if HAS_SYMBOLIC_LINKS
      } else {
        executablePath = followLink(executablePath);
#endif
      } /* if */
#if HAS_SYMBOLIC_LINKS
    } /* if */
#endif
    logFunction(printf("getExecutablePath --> \"%s\"\n",
                       striAsUnquotedCStri(executablePath)););
    return executablePath;
  } /* getExecutablePath */



#ifdef EMULATE_ENVIRONMENT
char *getenv7 (const char *name)

  {
    size_t len;
    char **p, *c;

  /* getenv7 */
    if (name == NULL || environ7 == NULL || strchr(name, '=') != NULL) {
      return NULL;
    } else {
      len = strlen(name);
      for (p = environ7; (c = *p) != NULL; ++p) {
        if (strncmp(c, name, len) == 0 && c[len] == '=') {
          return &c[len + 1];
        } /* if */
      } /* for */
      return NULL;
    } /* if */
  } /* getenv7 */



int setenv7 (const char *name, const char *value, int overwrite)

  {
    size_t len;
    char **p, *c;
    size_t nameCount = 0;

  /* setenv7 */
    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL) {
      errno = EINVAL;
      return -1;
    } else {
      len = strlen(name);
      if (environ7 != NULL) {
        for (p = environ7; (c = *p) != NULL; ++p) {
          nameCount ++;
          if (strncmp(c, name, len) == 0 && c[len] == '=') {
            if (overwrite) {
              if ((*p = realloc(*p, len + strlen(value) + 2)) == NULL) {
                errno = ENOMEM;
                return -1;
              } else {
                strcpy(&c[len + 1], value);
              } /* if */
            } /* if **/
            return 0;
          } /* if */
        } /* for */
      } /* if */
      if ((environ7 = realloc(environ7, sizeof(char *) * (nameCount + 2))) == NULL ||
          (c = malloc(len + strlen(value) + 2)) == NULL) {
        errno = ENOMEM;
        return -1;
      } else {
        memcpy(c, name, len);
        c[len] = '=';
        strcpy(&c[len + 1], value);
        environ7[nameCount] = c;
        environ7[nameCount + 1] = NULL;
        return 0;
      } /* if */
    } /* if */
  } /* setenv7 */
#endif
