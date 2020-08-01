/********************************************************************/
/*                                                                  */
/*  cmd_win.c     Command functions which call the Windows API.     */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
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
/*  Changes: 2010, 2012 - 2013  Thomas Mertes                       */
/*  Content: Command functions which call the Windows API.          */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
#include "io.h"
#include "fcntl.h"
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

#if POINTER_SIZE == 32
typedef int32type intptr_type;
#elif POINTER_SIZE == 64
typedef int64type intptr_type;
#endif



#if defined OS_STRI_WCHAR && !defined USE_WMAIN
#ifdef DEFINE_COMMAND_LINE_TO_ARGV_W
/**
 *  Special handling of backslash characters for CommandLineToArgvW.
 *  CommandLineToArgvW reads arguments in two modes. Inside and
 *  outside quotation mode. The following rules apply when a
 *  backslash is encountered:
 *  * 2n backslashes followed by a quotation mark produce
 *    n backslashes and a switch from inside to outside quotation
 *    mode and vice versa. In this case the quotation mark is not
 *    added to the argument.
 *  * (2n) + 1 backslashes followed by a quotation mark produce
 *    n backslashes followed by a quotation mark. In this case the
 *    quotation mark is added to the argument and the quotation mode
 +    is not changed.
 *  * n backslashes not followed by a quotation mark simply produce
 *    n backslashes.
 */
static void processBackslash (const_os_stritype *sourcePos, os_stritype *destPos)

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
      /* N backslashes not followed by a quotation mark  */
      /* simply produce n backslashes.                   */
      for (count = backslashCount; count > 0; count--) {
        **destPos = '\\';
        (*destPos)++;
      } /* for */
    } /* if */
  } /* processBackslash */



/**
 *  Parse commandLine and generate an array of pointers to the arguments.
 *  The parameter w_argc and the returned array of pointers (w_argv)
 *  correspond to the parameters argc and argv of main().
 *  The rules to recognize the first argument (the command) are
 *  different from the rules to recognize the other (normal) arguments.
 *  Arguments can be quoted or unquoted. Normal arguments (all except
 *  the first argument) can consist of quoted and unquoted parts. The
 *  quoted and unquoted parts that are concatenated to form one argument.
 *  To handle quoted and unquoted parts the function works with two
 *  modes: Inside and outside quotation mode.
 *  @param w_argc Address to which the argument count is copied.
 *  @return an array of pointers to the arguments of commandLine.
 */
static os_stritype *CommandLineToArgvW (const_os_stritype commandLine, int *w_argc)

  {
    size_t command_line_size;
    const_os_stritype sourcePos;
    os_stritype destPos;
    os_stritype destBuffer;
    memsizetype argumentCount;
    os_stritype *w_argv;

  /* CommandLineToArgvW */
    command_line_size = os_stri_strlen(commandLine);
    argumentCount = 0;
    w_argv = (os_stritype *) malloc(command_line_size * sizeof(os_stritype *));
    if (w_argv != NULL) {
      sourcePos = commandLine;
      while (*sourcePos == ' ') {
        sourcePos++;
      } /* while */
      if (*sourcePos == 0) {
        w_argv[0] = NULL;
      } else {
        if (unlikely(!os_stri_alloc(destBuffer, command_line_size))) {
          free(w_argv);
          w_argv = NULL;
        } else {
          /* Set pointer to first char of first argument */
          w_argv[0] = destBuffer;
          argumentCount = 1;
          destPos = destBuffer;
          if (*sourcePos == '"') {
            sourcePos++;
            while (*sourcePos != '"' && *sourcePos != 0) {
              *destPos = *sourcePos;
              sourcePos++;
              destPos++;
            } /* if */
          } else {
            while (*sourcePos != ' ' && *sourcePos != 0) {
              *destPos = *sourcePos;
              sourcePos++;
              destPos++;
            } /* if */
          } /* if */
          if (*sourcePos != 0) {
            do {
              sourcePos++;
            } while (*sourcePos == ' ');
            if (*sourcePos != 0) {
              /* Terminate the current argument */
              *destPos = 0;
              destPos++;
              /* Set pointer to first char of next argument */
              w_argv[argumentCount] = destPos;
              argumentCount++;
            } /* if */
          } /* if */
          while (*sourcePos != 0) {
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
              do {
                sourcePos++;
              } while (*sourcePos == ' ');
              if (*sourcePos != 0) {
                /* Terminate the current argument */
                *destPos = 0;
                destPos++;
                /* Set pointer to first char of next argument */
                w_argv[argumentCount] = destPos;
                argumentCount++;
              } /* if */
            } /* if */
          } /* while */
          /* Terminate the last argument */
          *destPos = 0;
          w_argv[argumentCount] = NULL;
        } /* if */
      } /* if */
    } /* if */
    *w_argc = argumentCount;
    return w_argv;
  } /* CommandLineToArgvW */



void freeUtf16Argv (os_stritype *w_argv)

  { /* freeUtf16Argv */
    if (w_argv != NULL) {
      os_stri_free(w_argv[0]);
      free(w_argv);
    } /* if */
  } /* freeUtf16Argv */

#else



void freeUtf16Argv (os_stritype *w_argv)

  { /* freeUtf16Argv */
    LocalFree(w_argv);
  } /* freeUtf16Argv */
#endif



os_stritype *getUtf16Argv (int *w_argc)

  {
    os_stritype commandLine;
    os_stritype *w_argv;

  /* getUtf16Argv */
    commandLine = GetCommandLineW();
    w_argv = CommandLineToArgvW(commandLine, w_argc);
    return w_argv;
  } /* getUtf16Argv */
#endif



/**
 *  Get the absolute path of the executable of the current process.
 *  @param arg_0 Parameter argv[0] from the function main() as string.
 *  @return the absolute path of the current process.
 */
stritype getExecutablePath (const const_stritype arg_0)

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
os_stritype wgetenv (const const_os_stritype name)

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
          FREE_OS_STRI(result);
          result = NULL;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* wgetenv */
#endif



#ifdef DEFINE_WSETENV
int wsetenv (const const_os_stritype name, const const_os_stritype value,
    int overwrite)

  {
    int result;

  /* wsetenv */
    /* printf("wsetenv(%ls, %ls, &d)\n", name, value, overwrite); */
    result = !SetEnvironmentVariableW(name, value);
    return result;
  } /* wsetenv */
#endif



#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
volumeListType *openVolumeList (void)

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



/**
 *  Create a command line string that can be used by CreateProcessW().
 *  The command line string must be freed with os_stri_free().
 */
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
    arraySize = arraySize(parameters);
    if (unlikely(!os_stri_alloc(command_line, MAXIMUM_COMMAND_LINE_LENGTH - 1))) {
      *err_info = MEMORY_ERROR;
    } else {
      beyondDest = &command_line[MAXIMUM_COMMAND_LINE_LENGTH];
      /* fprintf(stderr, "\ncommand_stri=\"%ls\"\n", os_command_stri); */
      striSize = os_stri_strlen(os_command_stri);
      if (striSize > MAXIMUM_COMMAND_LINE_LENGTH - 2 ||
          &command_line[striSize] > beyondDest) {
        *err_info = MEMORY_ERROR;
        destChar = beyondDest;
      } else {
        command_line[0] = '\"';
        memcpy(&command_line[1], os_command_stri, sizeof(os_chartype) * striSize);
        command_line[striSize + 1] = '\"';
        destChar = &command_line[striSize + 2];
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



void cmdPipe2 (const const_stritype command, const const_rtlArraytype parameters,
    filetype *childStdin, filetype *childStdout)

  {
    os_stritype os_command_stri;
    os_stritype command_line;
    SECURITY_ATTRIBUTES saAttr;
    HANDLE childInputRead;
    HANDLE childInputWrite;
    HANDLE childInputWriteTemp;
    HANDLE childOutputReadTemp;
    HANDLE childOutputRead;
    HANDLE childOutputWrite;
    HANDLE childErrorWrite;
    STARTUPINFOW startupInfo;
    PROCESS_INFORMATION processInformation;
    int path_info = PATH_IS_NORMAL;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdPipe2 */
    os_command_stri = cp_to_os_path(command, &path_info, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
      command_line = prepareCommandLine(os_command_stri, parameters, &err_info);
      if (likely(err_info == OKAY_NO_ERROR)) {
        /* printf("cmdPipe2(%ls, %ls)\n", os_command_stri, command_line); */
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;
        if (CreatePipe(&childInputRead, &childInputWriteTemp, &saAttr, 0) &&
            DuplicateHandle(GetCurrentProcess(), childInputWriteTemp,
                            GetCurrentProcess(), &childInputWrite,
                            0, FALSE, // Handle is not inherited
                            DUPLICATE_SAME_ACCESS) &&
            CreatePipe(&childOutputReadTemp, &childOutputWrite, &saAttr, 0) &&
            DuplicateHandle(GetCurrentProcess(),childOutputReadTemp,
                            GetCurrentProcess(), &childOutputRead,
                            0, FALSE, // Handle is not inherited
                            DUPLICATE_SAME_ACCESS) &&
            DuplicateHandle(GetCurrentProcess(),childOutputWrite,
                            GetCurrentProcess(),&childErrorWrite,
                            0, TRUE, DUPLICATE_SAME_ACCESS) &&
            CloseHandle(childInputWriteTemp) &&
            CloseHandle(childOutputReadTemp)) {
          memset(&startupInfo, 0, sizeof(startupInfo));
          /* memset(&processInformation, 0, sizeof(processInformation)); */
          startupInfo.cb = sizeof(startupInfo);
          startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
          startupInfo.wShowWindow = 0;
          startupInfo.hStdInput = childInputRead;
          startupInfo.hStdOutput = childOutputWrite;
          startupInfo.hStdError = childErrorWrite;
          /* printf("before CreateProcessW\n"); */
          if (CreateProcessW(os_command_stri,
                             command_line /* lpCommandLine */,
                             NULL /* lpProcessAttributes */,
                             NULL /* lpThreadAttributes */,
                             1  /* bInheritHandles */,
                             0 /* dwCreationFlags */,
                             NULL /* lpEnvironment */,
                             NULL /* lpCurrentDirectory */,
                             &startupInfo,
                             &processInformation) != 0) {
            CloseHandle(childInputRead);
            CloseHandle(childOutputWrite);
            CloseHandle(childErrorWrite);
            *childStdin  = fdopen(_open_osfhandle((intptr_type) (childInputWrite), _O_TEXT), "w");
            *childStdout = fdopen(_open_osfhandle((intptr_type) (childOutputRead), _O_TEXT), "r");
            CloseHandle(processInformation.hProcess);
            CloseHandle(processInformation.hThread);
          } else {
            /* printf("CreateProcessW failed (%d)\n", GetLastError()); */
            err_info = FILE_ERROR;
          } /* if */
        } else {
          err_info = FILE_ERROR;
        } /* if */
        /* printf("after CreateProcessW\n"); */
        os_stri_free(command_line);
      } /* if */
      os_stri_free(os_command_stri);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdPipe2 */



void cmdPty (const const_stritype command, const const_rtlArraytype parameters,
    filetype *childStdin, filetype *childStdout)

  { /* cmdPty */
    cmdPipe2(command, parameters, childStdin, childStdout);
  } /* cmdPty */



void cmdStartProcess (const const_stritype command, const const_rtlArraytype parameters)

  {
    os_stritype os_command_stri;
    os_stritype command_line;
    STARTUPINFOW startupInfo;
    PROCESS_INFORMATION processInformation;
    int path_info = PATH_IS_NORMAL;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdStartProcess */
    os_command_stri = cp_to_os_path(command, &path_info, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
      command_line = prepareCommandLine(os_command_stri, parameters, &err_info);
      if (likely(err_info == OKAY_NO_ERROR)) {
        memset(&startupInfo, 0, sizeof(startupInfo));
        /* memset(&processInformation, 0, sizeof(processInformation)); */
        startupInfo.cb = sizeof(startupInfo);
        startupInfo.dwFlags = STARTF_USESHOWWINDOW;
        startupInfo.wShowWindow = 1;
        /* printf("before CreateProcessW(%ls, %ls, ...)\n", os_command_stri, command_line); */
        if (CreateProcessW(os_command_stri,
                           command_line /* lpCommandLine */,
                           NULL /* lpProcessAttributes */,
                           NULL /* lpThreadAttributes */,
                           1  /* bInheritHandles */,
                           0 /* dwCreationFlags */,
                           NULL /* lpEnvironment */,
                           NULL /* lpCurrentDirectory */,
                           &startupInfo,
                           &processInformation) != 0) {
          CloseHandle(processInformation.hProcess);
          CloseHandle(processInformation.hThread);
        } else {
          /* printf("GetLastError=%d\n", GetLastError());
             printf("ERROR_FILE_NOT_FOUND=%d\n", ERROR_FILE_NOT_FOUND);
             prot_os_stri(os_command_stri);
             printf("\n"); */
          err_info = FILE_ERROR;
        } /* if */
        /* printf("after CreateProcessW\n"); */
        os_stri_free(command_line);
      } /* if */
      os_stri_free(os_command_stri);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdStartProcess */
