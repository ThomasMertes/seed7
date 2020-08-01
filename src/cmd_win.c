/********************************************************************/
/*                                                                  */
/*  cmd_win.c     Command functions which call the Windows API.     */
/*  Copyright (C) 1989 - 2011  Thomas Mertes                        */
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
/*  Content: Command functions which call the Windows API.          */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
#include "sys/types.h"
#include "sys/stat.h"
#ifdef OS_STRI_WCHAR
#include "wchar.h"
#endif

#include "dir_drv.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "cmd_drv.h"

#ifndef PATH_MAX
#define PATH_MAX 2048
#endif

#define MAXIMUM_COMMAND_LINE_LENGTH 32768



#if defined OS_STRI_WCHAR && !defined USE_WMAIN
#ifdef DEFINE_COMMAND_LINE_TO_ARGV_W
#ifdef ANSI_C

static void processBackslash (const_os_stritype *sourcePos, os_stritype *destPos)
#else

static void processBackslash (sourcePos, destPos)
os_stritype *sourcePos;
os_stritype *destPos;
#endif

  {
    memsizetype backslashCount;
    memsizetype count;

  /* processBackslash */
    backslashCount = 1;
    (*sourcePos)++;
    while (**sourcePos == '\\') {
      backslashCount++;
      (*sourcePos)++;
    } /* while */
    if (**sourcePos == '"') {
      /* Backslashes in the result: backslashCount / 2   */
      for (count = backslashCount >> 1; count > 0; count--) {
        **destPos = '\\';
        (*destPos)++;
      } /* for */
      if (backslashCount & 1) {
        /* Add a quotation mark (") to the result.       */
        **destPos = '"';
        (*destPos)++;
        /* Stay in current quotation mode                */
        (*sourcePos)++;
      } else {
        /* Ignore the quotation mark (").                */
        /* Switch from inside to outside quotation mode  */
        /* and vice versa.                               */
      } /* if */
    } else {
      for (count = backslashCount; count > 0; count--) {
        **destPos = '\\';
        (*destPos)++;
      } /* for */
    } /* if */
  } /* processBackslash */



#ifdef ANSI_C

static os_stritype *CommandLineToArgvW (const_os_stritype commandLine, int *w_argc)
#else

static os_stritype *CommandLineToArgvW (commandLine, w_argc)
os_stritype commandLine;
int *w_argc;
#endif

  {
    size_t command_line_size;
    const_os_stritype sourcePos;
    os_stritype destPos;
    os_stritype destBuffer;
    memsizetype argumentCount;
    os_stritype *result;

  /* CommandLineToArgvW */
    command_line_size = os_stri_strlen(commandLine);
    argumentCount = 0;
    result = (os_stritype *) malloc(command_line_size * sizeof(os_stritype *));
    if (result != NULL) {
      sourcePos = commandLine;
      while (*sourcePos == ' ') {
        sourcePos++;
      } /* while */
      if (*sourcePos == 0) {
        result[0] = NULL;
      } else {
        if (unlikely(!os_stri_alloc(destBuffer, command_line_size))) {
          free(result);
          result = NULL;
        } else {
          result[0] = destBuffer;
          argumentCount = 1;
          destPos = destBuffer;
          do {
            /* printf("source char: %d\n", *sourcePos); */
            if (*sourcePos == '"') {
              /* Inside quotation mode */
              sourcePos++;
              while (*sourcePos != '"' && *sourcePos != 0) {
                if (*sourcePos == '\\') {
                  processBackslash(&sourcePos, &destPos);
                } else {
                  *destPos = *sourcePos;
                  sourcePos++;
                  destPos++;
                } /* if */
              } /* while */
              if (*sourcePos == '"') {
                /* Consume the terminating quotation mark */
                sourcePos++;
              } /* if */
            } /* if */
            if (*sourcePos != ' ' && *sourcePos != 0) {
              /* Outside quotation mode */
              do {
                if (*sourcePos == '\\') {
                  processBackslash(&sourcePos, &destPos);
                } else {
                  *destPos = *sourcePos;
                  sourcePos++;
                  destPos++;
                } /* if */
              } while (*sourcePos != ' ' && *sourcePos != '"' && *sourcePos != 0);
            } /* if */
            if (*sourcePos == ' ') {
              /* The current argument is terminated */
              *destPos = 0;
              destPos++;
              result[argumentCount] = destPos;
              argumentCount++;
              while (*sourcePos == ' ') {
                sourcePos++;
              } /* while */
            } /* if */
          } while (*sourcePos != 0);
          /* The last argument is terminated */
          *destPos = 0;
          result[argumentCount] = NULL;
        } /* if */
      } /* if */
    } /* if */
    *w_argc = argumentCount;
    return result;
  } /* CommandLineToArgvW */



#ifdef ANSI_C

void freeUtf16Argv (os_stritype *w_argv)
#else

void freeUtf16Argv (w_argv)
os_stritype *w_argv;
#endif

  { /* freeUtf16Argv */
    if (w_argv != NULL) {
      os_stri_free(w_argv[0]);
      free(w_argv);
    } /* if */
  } /* freeUtf16Argv */

#else



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
#endif



#ifdef ANSI_C

stritype getExecutablePath (const const_stritype arg_0)
#else

stritype getExecutablePath (arg_0)
stritype arg_0;
#endif

  {
    os_chartype buffer[PATH_MAX];
    errinfotype err_info = OKAY_NO_ERROR;
    stritype result;

  /* getExecutablePath */
    if (GetModuleFileNameW(NULL, buffer, PATH_MAX) == 0) {
      raise_error(FILE_ERROR);
      result = NULL;
    } else {
      result = cp_from_os_path(buffer, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(err_info);
      } /* if */
    } /* if */
    return result;
  } /* getExecutablePath */



#ifdef DEFINE_WGETENV
#ifdef ANSI_C

os_stritype wgetenv (const const_os_stritype name)
#else

os_stritype wgetenv (name)
os_stritype name;
#endif

  {
    memsizetype result_size;
    os_stritype result;

  /* wgetenv */
    result_size = GetEnvironmentVariableW(name, NULL, 0);
    if (result_size == 0) {
      result = NULL;
    } else {
      if (ALLOC_WSTRI(result, result_size - 1)) {
        if (GetEnvironmentVariableW(name, result, result_size) != result_size - 1) {
          os_stri_free(result);
          result = NULL;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* wgetenv */
#endif



#ifdef DEFINE_WSETENV
#ifdef ANSI_C

int wsetenv (const const_os_stritype name, const const_os_stritype value,
    int overwrite)
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
#endif


static os_stritype prepareCommandLine (const const_os_stritype os_command_stri,
    const const_rtlArraytype parameters, errinfotype *err_info)

  {
    memsizetype arraySize;
    memsizetype striSize;
    memsizetype pos;
    os_stritype argument;
    os_chartype *sourceChar;
    os_chartype *destChar;
    os_chartype *beyondDest;
    memsizetype countBackslash;
    os_stritype command_line;

  /* prepareCommandLine */
    arraySize = (uinttype) (parameters->max_position - parameters->min_position + 1);
    if (unlikely(!os_stri_alloc(command_line, MAXIMUM_COMMAND_LINE_LENGTH - 1))) {
      *err_info = MEMORY_ERROR;
    } else {
      beyondDest = &command_line[MAXIMUM_COMMAND_LINE_LENGTH];
      /* fprintf(stderr, "\ncommand_stri=\"%ls\"\n", os_command_stri); */
      striSize = os_stri_strlen(command_line);
      if (striSize > MAXIMUM_COMMAND_LINE_LENGTH ||
          &command_line[striSize] > beyondDest) {
        *err_info = MEMORY_ERROR;
        destChar = beyondDest;
      } else {
        memcpy(command_line, os_command_stri, sizeof(os_chartype) * striSize);
        destChar = &command_line[striSize];
      } /* if */
      for (pos = 0; pos < arraySize && *err_info == OKAY_NO_ERROR; pos++) {
        argument = stri_to_os_stri(parameters->arr[pos].value.strivalue, err_info);
        if (argument != NULL) {
          /* fprintf(stderr, "argument[%d]=%ls\n", pos + 1, argument); */
          if (&destChar[2] > beyondDest) {
            destChar = beyondDest;
          } else {
            *(destChar++) = ' ';
            *(destChar++) = '"';
          } /* if */
          for (sourceChar = argument; *sourceChar != '\0' && destChar < beyondDest;
              sourceChar++, destChar++) {
            if (*sourceChar == '"') {
              if (&destChar[2] > beyondDest) {
                destChar = beyondDest;
              } else {
                *(destChar++) = '\\';
                *destChar = *sourceChar;
              } /* if */
            } else if (*sourceChar == '\\') {
              sourceChar++;
              countBackslash = 1;
              while (*sourceChar == '\\') {
                sourceChar++;
                countBackslash++;
              } /* while */
              /* fprintf(stderr, "countBackslash=%lu\n", countBackslash);
                 fprintf(stderr, "sourceChar=%c\n", *sourceChar); */
              if (*sourceChar == '"' || *sourceChar == '\0') {
                countBackslash *= 2;
              } /* if */
              sourceChar--;
              if (countBackslash > MAXIMUM_COMMAND_LINE_LENGTH ||
                  &destChar[countBackslash] > beyondDest) {
                destChar = beyondDest;
              } else {
                do {
                  *(destChar++) = '\\';
                  countBackslash--;
                } while (countBackslash != 0);
                destChar--;
              } /* if */
            } else {
              *destChar = *sourceChar;
            } /* if */
          } /* for */
          if (destChar >= beyondDest) {
            *err_info = MEMORY_ERROR;
          } else {
            *(destChar++) = '"';
          } /* if */
          os_stri_free(argument);
        } /* if */
      } /* for */
      if (destChar >= beyondDest) {
        *err_info = MEMORY_ERROR;
      } else {
        *(destChar++) = '\0';
      } /* if */
      if (*err_info != OKAY_NO_ERROR) {
        os_stri_free(command_line);
        command_line = NULL;
      } else {
        /* fprintf(stderr, "command_line=%ls\n", command_line); */
      } /* if */
    } /* if */
    return command_line;
  } /* prepareCommandLine */



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
    os_stritype command_line;
    STARTUPINFOW startupInfo;
    PROCESS_INFORMATION processInformation;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdPipe2 */
    os_command_stri = cp_to_os_path(command, &err_info);
    command_line = prepareCommandLine(os_command_stri, parameters, &err_info);
    /* printf("cmdPipe2(%ls, %ls, %d, %d)\n", os_command_stri,
       command_line, fileno(*childStdin), fileno(*childStdout)); */
    memset(&startupInfo, 0, sizeof(startupInfo));
    /* memset(&processInformation, 0, sizeof(processInformation)); */
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    startupInfo.wShowWindow = 0;
    startupInfo.hStdInput = *childStdin;
    startupInfo.hStdOutput = *childStdout;
    startupInfo.hStdError = stderr;
    /* printf("before CreateProcessW\n"); */
    if (err_info == OKAY_NO_ERROR &&
        CreateProcessW(os_command_stri,
                       command_line /* lpCommandLine */,
                       NULL /* lpProcessAttributes */,
                       NULL /* lpThreadAttributes */,
                       0  /* bInheritHandles */,
                       0 /* dwCreationFlags */,
                       NULL /* lpEnvironment */,
                       NULL /* lpCurrentDirectory */,
                       &startupInfo,
                       &processInformation) == 0) {
      err_info = FILE_ERROR;
    } /* if */
    /* printf("after CreateProcessW\n"); */
    os_stri_free(os_command_stri);
    os_stri_free(command_line);
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
    } /* if */
  } /* cmdPipe2 */



#ifdef ANSI_C

void cmdStartProcess (const const_stritype command, const const_rtlArraytype parameters)
#else

void cmdStartProcess (command, parameters)
stritype command;
rtlArraytype parameters;
#endif

  {
    os_stritype os_command_stri;
    os_stritype command_line;
    STARTUPINFOW startupInfo;
    PROCESS_INFORMATION processInformation;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdStartProcess */
    os_command_stri = cp_to_os_path(command, &err_info);
    command_line = prepareCommandLine(os_command_stri, parameters, &err_info);
    memset(&startupInfo, 0, sizeof(startupInfo));
    /* memset(&processInformation, 0, sizeof(processInformation)); */
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = 0;
    /* printf("err_info=%d\n", err_info); */
    /* printf("before CreateProcessW(%ls, %ls, ...)\n", os_command_stri, command_line); */
    if (err_info == OKAY_NO_ERROR &&
        CreateProcessW(os_command_stri,
                       command_line /* lpCommandLine */,
                       NULL /* lpProcessAttributes */,
                       NULL /* lpThreadAttributes */,
                       1  /* bInheritHandles */,
                       0 /* dwCreationFlags */,
                       NULL /* lpEnvironment */,
                       NULL /* lpCurrentDirectory */,
                       &startupInfo,
                       &processInformation) == 0) {
      /* printf("GetLastError=%d\n", GetLastError());
         printf("ERROR_FILE_NOT_FOUND=%d\n", ERROR_FILE_NOT_FOUND); */
      err_info = FILE_ERROR;
    } /* if */
    /* printf("after CreateProcessW\n"); */
    os_stri_free(os_command_stri);
    os_stri_free(command_line);
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
    } /* if */
  } /* cmdStartProcess */
