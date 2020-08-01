/********************************************************************/
/*                                                                  */
/*  cmd_win.c     Driver functions for commands.                    */
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
/*  File: seed7/src/cmd_win.c                                       */
/*  Changes: 2010  Thomas Mertes                                    */
/*  Content: Driver functions for commands.                         */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"
#include "windows.h"
#include "sys/types.h"
#include "sys/stat.h"

#include "dir_drv.h"

#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "cmd_drv.h"

#ifndef PATH_MAX
#define PATH_MAX 2048
#endif



#ifndef USE_WMAIN
#ifdef ANSI_C

os_stritype *getUtf16Argv (int *w_argc)
#else

os_stritype *getUtf16Argv (w_argc)
int *w_argc;
#endif

  {
    os_stritype commandLine;
    os_stritype *w_argv;

  /* getUtf16Argv */
    commandLine = GetCommandLineW();
    w_argv = CommandLineToArgvW(commandLine, w_argc);
    return w_argv;
  } /* getUtf16Argv */



#ifdef ANSI_C

void freeUtf16Argv (os_stritype *w_argv)
#else

void freeUtf16Argv (w_argv)
os_stritype *w_argv;
#endif

  { /* freeUtf16Argv */
    LocalFree(w_argv);
  } /* freeUtf16Argv */
#endif



#ifdef ANSI_C

stritype getExecutablePath (const const_stritype arg_0)
#else

stritype getExecutablePath (arg_0)
stritype arg_0;
#endif

  {
    os_chartype buffer[PATH_MAX];
    stritype result;

  /* getExecutablePath */
    if (GetModuleFileNameW(NULL, buffer, PATH_MAX) == 0) {
      raise_error(FILE_ERROR);
      result = NULL;
    } else {
      result = cp_from_os_path(buffer);
      if (result == NULL) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* getExecutablePath */



#ifdef USE_WGETENV_WSTRI
#ifdef ANSI_C

os_stritype wgetenv_wstri (os_stritype name)
#else

os_stritype wgetenv_wstri (name)
os_stritype name;
#endif

  {
    memsizetype result_size;
    os_stritype result;

  /* wgetenv_wstri */
    result_size = GetEnvironmentVariableW(name, NULL, 0);
    if (result_size == 0) {
      result = NULL;
    } else {
      if (!ALLOC_WSTRI(result, result_size - 1)) {
        result = NULL;
      } else {
        if (GetEnvironmentVariableW(name, result, result_size) != result_size - 1) {
          os_stri_free(result);
          result = NULL;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* wgetenv_wstri */
#endif



#ifdef DEFINE_WSETENV
#ifdef ANSI_C

int wsetenv (os_stritype name, os_stritype value, int overwrite)
#else

int wsetenv (name, value, overwrite)
os_stritype name;
os_stritype value;
int overwrite;
#endif

  {
    int result;

  /* wsetenv */
    /* printf("wsetenv(%ls, %ls, &d)\n", name, value, overwrite); */
    result = !SetEnvironmentVariableW(name, value);
    return result;
  } /* wsetenv */
#endif



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
      result->driveBitmask = GetLogicalDrives();
      result->currentDrive = 0;
    } /* if */
    return result;
  } /* openVolumeList */



#ifdef ANSI_C

stritype readVolumeName (volumeListType *volumeList)
#else

stritype readVolumeName (volumeList)
volumeListType *volumeList;
#endif

  {
    os_chartype os_path[4];
    os_stat_struct stat_buf;
    int stat_result;
    os_DIR *directory;
    char buffer[2];
    stritype result;

  /* readVolumeName */
    os_path[1] = (os_chartype) ':';
    os_path[2] = (os_chartype) '\\';
    os_path[3] = (os_chartype) '\0';
    result = NULL;
    do {
      while (volumeList->currentDrive < 26 &&
             (volumeList->driveBitmask & (uint32type) 1 << volumeList->currentDrive) == 0) {
        volumeList->currentDrive++;
      } /* while */
      if (volumeList->currentDrive < 26) {
        /* printf("%c:\\\n", (char) ((int) 'a' + volumeList->currentDrive)); */
        os_path[0] = (os_chartype) ((int) 'a' + volumeList->currentDrive);
        stat_result = os_stat(os_path, &stat_buf);
        if (stat_result == 0) {
          /* printf("%c:\\ st_mode=%u\n",
              (char) ((int) 'a' + volumeList->currentDrive),
              S_ISDIR(stat_buf.st_mode)); */
          directory = os_opendir(os_path);
          if (directory != NULL) {
            os_closedir(directory);
            buffer[0] = (char) ((int) 'a' + volumeList->currentDrive);
            buffer[1] = '\0';
            result = cstri_to_stri(buffer);
          } /* if */
        } /* if */
        volumeList->currentDrive++;
      } /* if */
    } while (result == NULL && volumeList->currentDrive < 26);
    return result;
  } /* readVolumeName */



#ifdef ANSI_C

void closeVolumeList (volumeListType *volumeList)
#else

void closeVolumeList (volumeList)
volumeListType *volumeList;
#endif

  { /* closeVolumeList */
    free(volumeList);
  } /* closeVolumeList */
#endif



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
    int childStdinFileno;
    int childStdoutFileno;
    int childStderrFileno;
    STARTUPINFO startupInfo;

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
        startupInfo.dwFlags = STARTF_USESHOWWINDOW |STARTF_USESTDHANDLES;
        startupInfo.wShowWindow = 0;
        startupInfo.hStdInput = childStdinFileno;
        startupInfo.hStdOutput = childStdoutFileno;
        startupInfo.hStdError = childStderrFileno;
        CreateProcessW(os_command_stri,
                       NULL /* lpCommandLine */,
                       NULL /* lpProcessAttributes */,
                       NULL /* lpThreadAttributes */,
                       false  /* bInheritHandles */,
                       NULL /* dwCreationFlags */,
                       NULL /* lpEnvironment */,
                       NULL /* lpCurrentDirectory */,
                       &startupInfo,
                       NULL /* lpProcessInformation */);
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
    STARTUPINFO startupInfo;

  /* cmdPipe2 */
    os_command_stri = cp_to_command(command_stri, &err_info);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW |STARTF_USESTDHANDLES;
    startupInfo.wShowWindow = 0;
    startupInfo.hStdInput
    startupInfo.hStdOutput
    startupInfo.hStdError
    CreateProcessW(os_command_stri,
                   NULL /* lpCommandLine */,
                   NULL /* lpProcessAttributes */,
                   NULL /* lpThreadAttributes */,
                   false  /* bInheritHandles */,
                   NULL /* dwCreationFlags */,
                   NULL /* lpEnvironment */,
                   NULL /* lpCurrentDirectory */,
                   &startupInfo,
                   NULL /* lpProcessInformation */);
    os_stri_free(os_path);
  } /* cmdPipe2 */
#endif
