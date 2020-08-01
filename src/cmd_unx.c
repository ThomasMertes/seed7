/********************************************************************/
/*                                                                  */
/*  cmd_unx.c     Driver functions for commands.                    */
/*  Copyright (C) 1989 - 2010  Thomas Mertes                        */
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
/*  Changes: 2010  Thomas Mertes                                    */
/*  Content: Driver functions for commands.                         */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "sys/types.h"

#ifdef USE_MYUNISTD_H
#include "myunistd.h"
#else
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



#ifdef ANSI_C

stritype getExecutablePath (const const_stritype arg_0)
#else

stritype getExecutablePath (arg_0)
stritype arg_0;
#endif

  {
    os_chartype buffer[PATH_MAX];
    ssize_t readlink_result;
    stritype cwd;
    stritype result;

  /* getExecutablePath */
    readlink_result = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (readlink_result == -1) {
      if (strChPos(arg_0, (chartype) '/') == 0) {
        result = examineSearchPath(arg_0);
        if (result == NULL) {
          raise_error(MEMORY_ERROR);
        } /* if */
      } else if (arg_0->size >= 1 && arg_0->mem[0] == (chartype) '/') {
        result = strCreate(arg_0);
      } else {
        cwd = cmdGetcwd();
        result = concat_path(cwd, arg_0);
        FREE_STRI(cwd, cwd->size);
      } /* if */
    } else {
      buffer[readlink_result] = '\0';
      result = cp_from_os_path(buffer);
      if (result == NULL) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* getExecutablePath */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

void cmdStartProcess (stritype command_stri, filetype childStdin,
    filetype childStdout, filetype childStderr)
#else

void cmdStartProcess (command_stri, dest_name)
stritype command_stri;
filetype childStdin;
filetype childStdout;
filetype childStderr;
#endif

  {
    os_stritype os_command_stri;
    booltype replaceStdin = FALSE;
    booltype replaceStdout = FALSE;
    booltype replaceStderr = FALSE;
    int childStdinFileno;
    int childStdoutFileno;
    int childStderrFileno;
    int savedStdin;
    int savedStdout;
    int savedStderr;
    pid_t pid;

  /* cmdStartProcess */
    childStdinFileno = fileno(childStdin);
    childStdoutFileno = fileno(childStdout);
    childStderrFileno = fileno(childStderr);
    if (childStdinFileno == -1 || childStdoutFileno == -1 || childStderrFileno == -1) {
      raise_error(RANGE_ERROR);
    } else {
      os_command_stri = cp_to_command(command_stri, &err_info);
      if (os_command_stri == NULL) {
        raise_error(RANGE_ERROR);
      } else {
        if (childStdinFileno != 0) {
          replaceStdin = TRUE;
          savedStdin = dup(0);
          close(0);
          dup2(childStdinFileno, 0);
        } /* if */
        if (childStdoutFileno != 1) {
          replaceStdout = TRUE;
          savedStdout = dup(1);
          close(1);
          dup2(childStdoutFileno, 1);
        } /* if */
        if (childStderrFileno != 2) {
          replaceStderr = TRUE;
          savedStderr = dup(2);
          close(2);
          dup2(childStderrFileno, 2);
        } /* if */
        pid = fork();
        if (pid == 0) {
          char *argv[]={os_command_stri, 0};
          if (replaceStdin) {
            close(savedStdin);
            close(childStdinFileno);
          } /* if */
          if (replaceStdout) {
            close(savedStdout);
            close(childStdoutFileno);
          } /* if */
          if (replaceStderr) {
            close(savedStderr);
            close(childStderrFileno);
          } /* if */
          execv(argv[0], argv);
        } else {
          if (replaceStdin) {
            close(0);
            dup2(savedStdin, 0);
          } /* if */
          if (replaceStdout) {
            close(1);
            dup2(savedStdout, 1);
          } /* if */
          if (replaceStderr) {
            close(2);
            dup2(savedStderr, 2);
          } /* if */
        } /* if */
        os_stri_free(os_path);
      } /* if */
    } /* if */
  } /* cmdStartProcess */



#ifdef ANSI_C

void cmdPipe2 (stritype source_name, filetype *childStdin, filetype *childStdout)
#else

void cmdPipe2 (source_name, dest_name)
stritype source_name;
stritype dest_name;
#endif

  {
    os_stritype os_command_stri;
    int childStdinPipes[2];
    int childStdoutPipes[2];
    int savedStdin;
    int savedStdout;
    pid_t pid;

  /* cmdPipe2 */
    os_command_stri = cp_to_command(command_stri, &err_info);
    if (os_command_stri == NULL) {
      raise_error(err_info);
      result = 0;
    } else {
      pipe(childStdinPipes);
      pipe(childStdoutPipes);
      savedStdin  = dup(0);
      savedStdout = dup(1);
      close(0);
      close(1);
      dup2(childStdinPipes[0], 0); /* Make the read end of childStdinPipes pipe as stdin */
      dup2(childStdoutPipes[1], 1);  /* Make the write end of childStdoutPipes as stdout */
      pid = fork();
      if (pid == 0) {
        char *argv[]={os_command_stri, 0};
        close(childStdinPipes[0]); /* Not required for the child */
        close(childStdinPipes[1]);
        close(childStdoutPipes[0]);
        close(childStdoutPipes[1]);
        execv(argv[0], argv);
      } else {
        close(0); /* Restore the original std fds of parent */
        close(1);
        dup2(savedStdin, 0);
        dup2(savedStdout, 1);
        close(childStdinPipes[0]); /* These are being used by the child */
        close(childStdoutPipes[1]);
        *childStdin  = fdopen(childStdinPipes[1], "w");
        *childStdout = fdopen(childStdoutPipes[0], "r");
      } /* if */
      os_stri_free(os_path);
    } /* if */
  } /* cmdPipe2 */
#endif
