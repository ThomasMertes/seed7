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

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "sys/types.h"
#include "fcntl.h"
#include "errno.h"

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
#ifdef HAS_SYMLINKS
    os_chartype buffer[PATH_MAX];
    ssize_t readlink_result;
    errinfotype err_info = OKAY_NO_ERROR;
#ifdef APPEND_EXTENSION_TO_EXECUTABLE_PATH
    stritype exeExtension;
#endif
#endif
    stritype cwd;
    stritype result;

  /* getExecutablePath */
#ifdef HAS_SYMLINKS
    readlink_result = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (readlink_result != -1) {
      buffer[readlink_result] = '\0';
      result = cp_from_os_path(buffer, &err_info);
      if (err_info != OKAY_NO_ERROR) {
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
      if (strChPos(arg_0, (chartype) '/') == 0) {
        result = examineSearchPath(arg_0);
      } else if (arg_0->size >= 1 && arg_0->mem[0] == (chartype) '/') {
        result = strCreate(arg_0);
      } else {
        cwd = cmdGetcwd();
        result = concat_path(cwd, arg_0);
        FREE_STRI(cwd, cwd->size);
      } /* if */
      if (result == NULL) {
        raise_error(MEMORY_ERROR);
#ifdef HAS_SYMLINKS
      } else {
        result = followLink(result);
#endif
      } /* if */
#ifdef HAS_SYMLINKS
    } /* if */
#endif
    /* printf("getExecutablePath --> ");
       prot_stri(result);
       printf("\n"); */
    return result;
  } /* getExecutablePath */



#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#ifdef ANSI_C

volumeListType *openVolumeList (void)
#else

volumeListType *openVolumeList ()
#endif

  {
    volumeListType *result;

  /* openVolumeList */
    if ((result = (volumeListType *) malloc(sizeof(volumeListType))) != NULL) {
      result->magicValue = UINT32TYPE_MAX;
      result->driveBitmask = 0x3FFFFFF;
      result->currentDrive = 0;
    } /* if */
    return result;
  } /* openVolumeList */
#endif



#ifdef ANSI_C

void cmdPipe2 (const const_stritype command, const const_rtlArraytype parameters,
    filetype *childStdin, filetype *childStdout)
#else

void cmdPipe2 (command, parameters, childStdin, childStdout)
stritype command;
rtlArraytype parameters;
filetype *childStdin;
filetype *childStdout;
#endif

  {
    os_stritype os_command_stri;
    int childStdinPipes[2];
    int childStdoutPipes[2];
    int savedStdin;
    int savedStdout;
    int path_info = PATH_IS_NORMAL;
    errinfotype err_info = OKAY_NO_ERROR;
    pid_t pid;

  /* cmdPipe2 */
    os_command_stri = cp_to_os_path(command, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } else if (access(os_command_stri, X_OK) != 0) {
      os_stri_free(os_command_stri);
      raise_error(FILE_ERROR);
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
        os_stritype *argv;
        memsizetype arraySize = (uinttype) (parameters->max_position - parameters->min_position + 1);
        memsizetype pos;
        argv = (os_stritype *) malloc(sizeof(os_stritype) * (arraySize + 2));
        argv[0] = os_command_stri;
        /* fprintf(stderr, "argv[0]=%s\n", argv[0]); */
        for (pos = 0; pos < arraySize && err_info == OKAY_NO_ERROR; pos++) {
          argv[pos + 1] = stri_to_os_stri(parameters->arr[pos].value.strivalue, &err_info);
          /* fprintf(stderr, "argv[%d]=%s\n", pos + 1, argv[pos + 1]); */
        } /* for */
        argv[arraySize + 1] = NULL;
        /* fprintf(stderr, "argv[%d]=NULL\n", arraySize + 1); */
        close(childStdinPipes[0]); /* Not required for the child */
        close(childStdinPipes[1]);
        close(childStdoutPipes[0]);
        close(childStdoutPipes[1]);
        execv(argv[0], argv);
        /* printf("errno=%d\n", errno);
        printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
            EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS);
        printf("EFAULT=%d  EISDIR=%d  ENAMETOOLONG=%d  ENODEV=%d  EINVAL=%d\n",
            EFAULT, EISDIR, ENAMETOOLONG, ENODEV, EINVAL); */
        /* printf("cannot exec: %s\n",os_command_stri ); */
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
      os_stri_free(os_command_stri);
    } /* if */
  } /* cmdPipe2 */



#ifdef IMPLEMENT_PTY_WITH_PIPE2
#ifdef ANSI_C

void cmdPty (const const_stritype command, const const_rtlArraytype parameters,
    filetype *childStdin, filetype *childStdout)
#else

void cmdPty (command, parameters, childStdin, childStdout)
stritype command;
rtlArraytype parameters;
filetype *childStdin;
filetype *childStdout;
#endif

  { /* cmdPty */
    cmdPipe2(command, parameters, childStdin, childStdout);
  } /* cmdPty */

#else



#ifdef ANSI_C

void cmdPty (const const_stritype command, const const_rtlArraytype parameters,
    filetype *childStdin, filetype *childStdout)
#else

void cmdPty (command, parameters, childStdin, childStdout)
stritype command;
rtlArraytype parameters;
filetype *childStdin;
filetype *childStdout;
#endif

  {
    os_stritype os_command_stri;
    int masterfd;
    int slavefd;
    char *slavedevice;
    int savedStdin;
    int savedStdout;
    int path_info = PATH_IS_NORMAL;
    errinfotype err_info = OKAY_NO_ERROR;
    pid_t pid;

  /* cmdPty */
    os_command_stri = cp_to_os_path(command, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } else if (access(os_command_stri, X_OK) != 0) {
      os_stri_free(os_command_stri);
      raise_error(FILE_ERROR);
    } else {
      masterfd = posix_openpt(O_RDWR|O_NOCTTY);
      if (masterfd == -1 || grantpt(masterfd) == -1 ||
          unlockpt (masterfd) == -1 || (slavedevice = ptsname(masterfd)) == NULL) {
        os_stri_free(os_command_stri);
        raise_error(FILE_ERROR);
      } else {
        /* printf("slave device is: %s\n", slavedevice); */
        slavefd = open(slavedevice, O_RDWR|O_NOCTTY);
        if (slavefd < 0) {
          os_stri_free(os_command_stri);
          raise_error(FILE_ERROR);
        } else {
          savedStdin  = dup(0);
          savedStdout = dup(1);
          close(0);
          close(1);
          dup2(slavefd, 0); /* Make the read end of slavefd as stdin */
          dup2(slavefd, 1);  /* Make the write end of slavefd as stdout */
          pid = fork();
          if (pid == 0) {
            os_stritype *argv;
            memsizetype arraySize = (uinttype) (parameters->max_position - parameters->min_position + 1);
            memsizetype pos;
            argv = (os_stritype *) malloc(sizeof(os_stritype) * (arraySize + 2));
            argv[0] = os_command_stri;
            /* fprintf(stderr, "argv[0]=%s\n", argv[0]); */
            for (pos = 0; pos < arraySize && err_info == OKAY_NO_ERROR; pos++) {
              argv[pos + 1] = stri_to_os_stri(parameters->arr[pos].value.strivalue, &err_info);
              /* fprintf(stderr, "argv[%d]=%s\n", pos + 1, argv[pos + 1]); */
            } /* for */
            argv[arraySize + 1] = NULL;
            /* fprintf(stderr, "argv[%d]=NULL\n", arraySize + 1); */
            close(masterfd); /* Not required for the child */
            close(slavefd);
            execv(argv[0], argv);
            /* printf("errno=%d\n", errno);
            printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
                EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS);
            printf("EFAULT=%d  EISDIR=%d  ENAMETOOLONG=%d  ENODEV=%d  EINVAL=%d\n",
                EFAULT, EISDIR, ENAMETOOLONG, ENODEV, EINVAL); */
            /* printf("cannot exec: %s\n",os_command_stri ); */
          } else {
            close(0); /* Restore the original std fds of parent */
            close(1);
            dup2(savedStdin, 0);
            dup2(savedStdout, 1);
            close(slavefd); /* This is being used by the child */
            *childStdin  = fdopen(masterfd, "w");
            *childStdout = fdopen(masterfd, "r");
          } /* if */
        } /* if */
      } /* if */
      os_stri_free(os_command_stri);
    } /* if */
  } /* cmdPty */
#endif



#ifdef ANSI_C

void cmdStartProcess (const const_stritype command, const const_rtlArraytype parameters)
#else

void cmdStartProcess (command, parameters)
stritype command;
rtlArraytype parameters;
#endif

  {
    os_stritype os_command_stri;
    int path_info = PATH_IS_NORMAL;
    errinfotype err_info = OKAY_NO_ERROR;
    pid_t pid;

  /* cmdStartProcess */
    os_command_stri = cp_to_os_path(command, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } else if (access(os_command_stri, X_OK) != 0) {
      os_stri_free(os_command_stri);
      raise_error(FILE_ERROR);
    } else {
      pid = fork();
      if (pid == 0) {
        os_stritype *argv;
        memsizetype arraySize = (uinttype) (parameters->max_position - parameters->min_position + 1);
        memsizetype pos;
        argv = (os_stritype *) malloc(sizeof(os_stritype) * (arraySize + 2));
        argv[0] = os_command_stri;
        /* fprintf(stderr, "argv[0]=%s\n", argv[0]); */
        for (pos = 0; pos < arraySize && err_info == OKAY_NO_ERROR; pos++) {
          argv[pos + 1] = stri_to_os_stri(parameters->arr[pos].value.strivalue, &err_info);
          /* fprintf(stderr, "argv[%d]=%s\n", pos + 1, argv[pos + 1]); */
        } /* for */
        argv[arraySize + 1] = NULL;
        /* fprintf(stderr, "argv[%d]=NULL\n", arraySize + 1); */
        execv(argv[0], argv);
        /* printf("errno=%d\n", errno);
        printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
            EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS);
        printf("EFAULT=%d  EISDIR=%d  ENAMETOOLONG=%d  ENODEV=%d  EINVAL=%d\n",
            EFAULT, EISDIR, ENAMETOOLONG, ENODEV, EINVAL); */
        /* printf("cannot exec: %s\n",os_command_stri ); */
      } /* if */
      os_stri_free(os_command_stri);
    } /* if */
  } /* cmdStartProcess */
