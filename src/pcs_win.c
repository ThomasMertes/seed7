/********************************************************************/
/*                                                                  */
/*  pcs_win.c     Process functions which use the Windows API.      */
/*  Copyright (C) 1989 - 2014  Thomas Mertes                        */
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
/*  File: seed7/src/pcs_win.c                                       */
/*  Changes: 2010, 2012 - 2014  Thomas Mertes                       */
/*  Content: Process functions which use the Windows API.           */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"
#include "io.h"
#include "fcntl.h"
#include "wchar.h"
#include "ctype.h"
#include "errno.h"

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "int_rtl.h"
#include "fil_rtl.h"
#include "rtl_err.h"


#define MAXIMUM_COMMAND_LINE_LENGTH 32768

typedef struct {
    uintType usage_count;
    fileType stdIn;
    fileType stdOut;
    fileType stdErr;
    /* Up to here the structure is identical to struct processStruct */
    HANDLE hProcess;
    HANDLE hThread;
    DWORD pid;
    boolType isTerminated;
    DWORD exitValue;
  } win_processRecord, *win_processType;

typedef const win_processRecord *const_win_processType;

#if DO_HEAP_STATISTIC
size_t sizeof_processRecord = sizeof(win_processRecord);
#endif

#define to_hProcess(process)     (((const_win_processType) process)->hProcess)
#define to_hThread(process)      (((const_win_processType) process)->hThread)
#define to_pid(process)          (((const_win_processType) process)->pid)
#define to_isTerminated(process) (((const_win_processType) process)->isTerminated)
#define to_exitValue(process)    (((const_win_processType) process)->exitValue)

#define to_var_hProcess(process)     (((win_processType) process)->hProcess)
#define to_var_hThread(process)      (((win_processType) process)->hThread)
#define to_var_pid(process)          (((win_processType) process)->pid)
#define to_var_isTerminated(process) (((win_processType) process)->isTerminated)
#define to_var_exitValue(process)    (((win_processType) process)->exitValue)

#if POINTER_SIZE == 32
typedef int32Type intPtrType;
#elif POINTER_SIZE == 64
typedef int64Type intPtrType;
#endif



#if ANY_LOG_ACTIVE
static void printParameters (const const_rtlArrayType parameters)

  {
    memSizeType paramSize;
    memSizeType pos;

  /* printParameters */
    paramSize = arraySize(parameters);
    for (pos = 0; pos < paramSize; pos++) {
      printf(", \"%s\"",
             striAsUnquotedCStri(parameters->arr[pos].value.striValue));
    } /* for */
  } /* printParameters */
#endif



/**
 *  Fill the contents of a quoted part to be used by prepareCommandLine.
 *  This function does not create the surrounding quotations (").
 *  A string with the backslash logic of Windows commandline parameters
 *  is produced. 2 * N backslashes followed by a quotation (") means
 *  N backslashes and the end of the parameter. Note that in this case
 *  the quotation is written by prepareCommandLine. 2 * N + 1 backslashes
 *  followed by a quotation (") means N backslashes and the quotation
 *  is part of the parameter. N backslashes not followed by a quotation
 *  means just N backslashes.
 */
static os_charType *copyQuotedPart (os_charType *sourceChar, os_charType *destChar,
    os_charType *beyondDest)

  {
    memSizeType countBackslash;

  /* copyQuotedPart */
    for (; *sourceChar != '\0' && destChar < beyondDest;
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
        /* fprintf(stderr, "countBackslash=" FMT_U_MEM "\n", countBackslash);
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
    return destChar;
  } /* copyQuotedPart */



/**
 *  Create a command line string that can be used by CreateProcessW().
 *  All parameters that contain a space or a quotation (") or a control
 *  character or a character beyond ASCII are quoted. All other parameters
 *  are not quoted. The command line string must be freed with os_stri_free().
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if a memory allocation failed, and
 *                  RANGE_ERROR if the conversion of a parameter failed.
 *  @return a null terminated os_striType command line, or
 *          NULL if an error occurred.
 */
static os_striType prepareCommandLine (const const_os_striType os_command_stri,
    const const_rtlArrayType parameters, errInfoType *err_info)

  {
    const_os_striType command_stri;
    memSizeType arraySize;
    memSizeType striSize;
    memSizeType pos;
    os_striType argument;
    os_charType *sourceChar;
    os_charType *destChar;
    os_charType *beyondDest;
    boolType quoteArgument;
    os_striType command_line;

  /* prepareCommandLine */
    logFunction(printf("prepareCommandLine(\"" FMT_S_OS "\"",
                       os_command_stri);
                printParameters(parameters);
                printf(", %d)\n", *err_info););
    arraySize = arraySize(parameters);
    if (unlikely(!os_stri_alloc(command_line, MAXIMUM_COMMAND_LINE_LENGTH - 1))) {
      *err_info = MEMORY_ERROR;
    } else {
      beyondDest = &command_line[MAXIMUM_COMMAND_LINE_LENGTH];
      if (USE_EXTENDED_LENGTH_PATH &&
          memcmp(os_command_stri, PATH_PREFIX, PREFIX_LEN * sizeof(os_charType)) == 0) {
        /* For extended path omit the prefix. */
        command_stri = &os_command_stri[PREFIX_LEN];
      } else {
        command_stri = os_command_stri;
      } /* if */
      /* fprintf(stderr, "\ncommand_stri=\"%ls\"\n", command_stri); */
#ifdef USE_MODULE_NAME_FOR_CREATE_PROCESS
      /* Pelles C cannot start the compiler (POCC.EXE) without this fix. */
      {
        os_charType *lastPathDelimiter;

        lastPathDelimiter = os_stri_strrchr(command_stri, OS_PATH_DELIMITER);
        if (lastPathDelimiter != NULL) {
          command_stri = &lastPathDelimiter[1];
        } /* if */
      }
#endif
      striSize = os_stri_strlen(command_stri);
      if (striSize > MAXIMUM_COMMAND_LINE_LENGTH - 2 ||
          &command_line[striSize] > beyondDest) {
        *err_info = MEMORY_ERROR;
        destChar = beyondDest;
      } else {
        command_line[0] = '\"';
        memcpy(&command_line[1], command_stri, sizeof(os_charType) * striSize);
        command_line[striSize + 1] = '\"';
        destChar = &command_line[striSize + 2];
      } /* if */
      for (pos = 0; pos < arraySize && *err_info == OKAY_NO_ERROR; pos++) {
        argument = stri_to_os_stri(parameters->arr[pos].value.striValue, err_info);
        if (argument != NULL) {
          /* fprintf(stderr, "argument[%d]=%ls\n", pos + 1, argument); */
          quoteArgument = FALSE;
          for (sourceChar = argument; *sourceChar != '\0'; sourceChar++) {
            if (*sourceChar <= ' ' || *sourceChar > '~' || *sourceChar == '"') {
              quoteArgument = TRUE;
            } /* if */
          } /* for */
          if (quoteArgument) {
            if (&destChar[2] > beyondDest) {
              destChar = beyondDest;
            } else {
              *(destChar++) = ' ';
              *(destChar++) = '"';
            } /* if */
            destChar = copyQuotedPart(argument, destChar, beyondDest);
            if (destChar >= beyondDest) {
              *err_info = MEMORY_ERROR;
            } else {
              *(destChar++) = '"';
            } /* if */
          } else {
            if (destChar < beyondDest) {
              *(destChar++) = ' ';
            } /* if */
            for (sourceChar = argument; *sourceChar != '\0' && destChar < beyondDest;
                 sourceChar++, destChar++) {
              *destChar = *sourceChar;
            } /* for */
          } /* if */
          os_stri_free(argument);
        } /* if */
      } /* for */
      if (destChar >= beyondDest) {
        *err_info = MEMORY_ERROR;
      } else {
        *destChar = '\0';
      } /* if */
      if (unlikely(*err_info != OKAY_NO_ERROR)) {
        os_stri_free(command_line);
        command_line = NULL;
      } else {
        /* fprintf(stderr, "command_line=%ls\n", command_line); */
      } /* if */
    } /* if */
    logFunction(printf("prepareCommandLine --> \"" FMT_S_OS "\"\n",
                       command_line););
    return command_line;
  } /* prepareCommandLine */



/**
 *  Compare two processes.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType pcsCmp (const const_processType process1, const const_processType process2)

  {
    intType signumValue;

  /* pcsCmp */
    if (process1 == NULL) {
      if (process2 != NULL) {
        signumValue = -1;
      } else {
        signumValue = 0;
      } /* if */
    } else if (process2 == NULL) {
      signumValue = 1;
    } else if (to_pid(process1) < to_pid(process2)) {
      signumValue = -1;
    } else {
      signumValue = to_pid(process1) > to_pid(process2);
    } /* if */
    return signumValue;
  } /* pcsCmp */



/**
 *  Check if two processes are equal.
 *  @return TRUE if both processes are equal,
 *          FALSE otherwise.
 */
boolType pcsEq (const const_processType process1, const const_processType process2)

  { /* pcsEq */
    if (process1 == NULL) {
      return process2 == NULL;
    } else if (process2 == NULL) {
      return FALSE;
    } else {
      return to_pid(process1) == to_pid(process2);
    } /* if */
  } /* pcsEq */



/**
 *  Return the exit value of the specified process.
 *  By convention, the value 0 indicates normal termination.
 *  @return the exit value of the specified process.
 */
intType pcsExitValue (const const_processType process)

  {
    intType exitValue;

  /* pcsExitValue */
    logFunction(printf("pcsExitValue(" FMT_U32 " (usage=" FMT_U "))\n",
                       (uint32Type) (process != NULL ? to_pid(process) : 0),
                       process != NULL ? process->usage_count : (uintType) 0););
    if (unlikely(process == NULL)) {
      logError(printf("pcsExitValue: process == NULL\n"););
      raise_error(FILE_ERROR);
      exitValue = -1;
    } else if (unlikely(!to_isTerminated(process))) {
      logError(printf("pcsExitValue(" FMT_U32 " (usage=" FMT_U ")): "
                      "Process has not terminated.\n",
                      (uint32Type) (process != NULL ? to_pid(process) : 0),
                      process != NULL ? process->usage_count : (uintType) 0););
      raise_error(FILE_ERROR);
      exitValue = -1;
    } else {
      exitValue = (intType) to_exitValue(process);
    } /* if */
    logFunction(printf("pcsExitValue(" FMT_U32 " (usage=" FMT_U ")) --> " FMT_D "\n",
                       (uint32Type) (process != NULL ? to_pid(process) : 0),
                       process != NULL ? process->usage_count : (uintType) 0,
                       exitValue););
    return exitValue;
  } /* pcsExitValue */



/**
 *  Free the memory referred by 'oldProcess'.
 *  After pcsFree is left 'oldProcess' refers to not existing memory.
 *  The memory where 'oldProcess' is stored can be freed afterwards.
 */
void pcsFree (processType oldProcess)

  { /* pcsFree */
    logFunction(printf("pcsFree(" FMT_U_MEM
                       " (pid=" FMT_U32 ", usage=" FMT_U "))\n",
                       (memSizeType) oldProcess,
                       oldProcess != NULL ? (uint32Type) to_pid(oldProcess)
                                          : (uint32Type) 0,
                       oldProcess != NULL ? oldProcess->usage_count
                                          : (uintType) 0););
    if (oldProcess->stdIn != NULL) {
      filClose(oldProcess->stdIn);
      filDestr(oldProcess->stdIn);
    } /* if */
    if (oldProcess->stdOut != NULL) {
      filClose(oldProcess->stdOut);
      filDestr(oldProcess->stdOut);
    } /* if */
    if (oldProcess->stdErr != NULL) {
      filClose(oldProcess->stdErr);
      filDestr(oldProcess->stdErr);
    } /* if */
    CloseHandle(to_hProcess(oldProcess));
    CloseHandle(to_hThread(oldProcess));
    FREE_RECORD(oldProcess, win_processRecord, count.process);
  } /* pcsFree */



/**
 *  Compute the hash value of a process.
 *  @return the hash value.
 */
intType pcsHashCode (const const_processType process)

  {
    intType hashCode;

  /* pcsHashCode */
    if (process == NULL) {
      hashCode = 0;
    } else {
      hashCode = to_pid(process);
    } /* if */
    return hashCode;
  } /* pcsHashCode */



/**
 *  Test whether the specified process is alive.
 *  @return TRUE if the specified process has not yet terminated,
 *          FALSE otherwise.
 */
boolType pcsIsAlive (const processType process)

  {
    DWORD exitCode = 0;
    boolType isAlive;

  /* pcsIsAlive */
    logFunction(printf("pcsIsAlive(" FMT_U_MEM
                       " (pid=" FMT_U32 ", hProcess=" FMT_U_MEM "))\n",
                       (memSizeType) process,
                       (uint32Type) (process != NULL ? to_pid(process) : 0),
                       process != NULL ? (memSizeType) to_hProcess(process)
                                       : (memSizeType) 0););
    if (process == NULL || to_isTerminated(process)) {
      isAlive = FALSE;
    } else {
      if (GetExitCodeProcess(to_hProcess(process), &exitCode) != 0) {
        if (exitCode == STILL_ACTIVE) {
          if (WaitForSingleObject(to_hProcess(process), 0) == WAIT_OBJECT_0) {
            to_var_isTerminated(process) = TRUE;
          } /* if */
        } else {
          to_var_isTerminated(process) = TRUE;
        } /* if */
        isAlive = !to_isTerminated(process);
        if (!isAlive) {
          to_var_exitValue(process) = exitCode;
        } /* if */
      } else {
        logError(printf("pcsIsAlive: GetExitCodeProcess(" FMT_U_MEM ", 0) failed:\n"
                        "GetLastError=" FMT_U32 "\n",
                        (memSizeType) to_hProcess(process),
                        (uint32Type) GetLastError());
                 printf("PID=" FMT_U32 "\n", (uint32Type) to_pid(process)););
        raise_error(FILE_ERROR);
        isAlive = TRUE;
      } /* if */
    } /* if */
    return isAlive;
  } /* pcsIsAlive */



/**
 *  Kill the specified process.
 *  @exception FILE_ERROR It was not possible to kill the process.
 */
void pcsKill (const processType process)

  { /* pcsKill */
    logFunction(printf("pcsKill(" FMT_U_MEM
                       " (pid=" FMT_U32 ", hProcess=" FMT_U_MEM "))\n",
                       (memSizeType) process,
                       (uint32Type) (process != NULL ? to_pid(process) : 0),
                       process != NULL ? (memSizeType) to_hProcess(process)
                                       : (memSizeType) 0););
    if (unlikely(process == NULL)) {
      logError(printf("pcsKill: process == NULL\n"););
      raise_error(FILE_ERROR);
    } else if (unlikely(TerminateProcess(to_hProcess(process), 0) == 0)) {
      logError(printf("pcsKill: TerminateProcess(" FMT_U_MEM ", 0) failed:\n"
                      "GetLastError=" FMT_U32 "\n",
                      (memSizeType) to_hProcess(process), (uint32Type) GetLastError());
               printf("PID=" FMT_U32 "\n", (uint32Type) to_pid(process)););
      raise_error(FILE_ERROR);
    } /* if */
  } /* pcsKill */



void pcsPipe2 (const const_striType command, const const_rtlArrayType parameters,
    fileType *childStdin, fileType *childStdout)

  {
    os_striType os_command_stri;
    os_striType command_line;
    fileType childStdinFile = NULL;
    fileType childStdoutFile = NULL;
    SECURITY_ATTRIBUTES saAttr;
    HANDLE childInputRead = INVALID_HANDLE_VALUE;
    HANDLE childInputWrite = INVALID_HANDLE_VALUE;
    HANDLE childOutputRead = INVALID_HANDLE_VALUE;
    HANDLE childOutputWrite = INVALID_HANDLE_VALUE;
    int childStdinFileno;
    int childStdoutFileno;
    STARTUPINFOW startupInfo;
    PROCESS_INFORMATION processInformation;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;

  /* pcsPipe2 */
    logFunction(printf("pcsPipe2(\"%s\"", striAsUnquotedCStri(command));
                printParameters(parameters);
                printf(", %s%d, %s%d)\n",
                       *childStdin == NULL ? "NULL " : "",
                       *childStdin != NULL ? safe_fileno((*childStdin)->cFile) : 0,
                       *childStdout == NULL ? "NULL " : "",
                       *childStdout != NULL ? safe_fileno((*childStdout)->cFile) : 0););
    os_command_stri = cp_to_os_path(command, &path_info, &err_info);
    if (likely(os_command_stri != NULL)) {
      command_line = prepareCommandLine(os_command_stri, parameters, &err_info);
      if (likely(command_line != NULL)) {
        if (unlikely(!ALLOC_RECORD(childStdinFile, fileRecord, count.files) ||
                     !ALLOC_RECORD(childStdoutFile, fileRecord, count.files))) {
          err_info = MEMORY_ERROR;
        } else {
          /* printf("pcsPipe2(%ls, %ls)\n", os_command_stri, command_line); */
          saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
          saAttr.bInheritHandle = TRUE;
          saAttr.lpSecurityDescriptor = NULL;
          if (unlikely(CreatePipe(&childInputRead,
                                  &childInputWrite, &saAttr, 0) == 0 ||
                       CreatePipe(&childOutputRead,
                                  &childOutputWrite, &saAttr, 0) == 0)) {
            logError(printf("pcsPipe2(\"%s\", ...): CreatePipe() failed.\n",
                            striAsUnquotedCStri(command)););
            err_info = FILE_ERROR;
          } else if (unlikely(SetHandleInformation(childInputWrite,
                                                   HANDLE_FLAG_INHERIT, 0) == 0 ||
                              SetHandleInformation(childOutputRead,
                                                   HANDLE_FLAG_INHERIT, 0) == 0)) {
            logError(printf("pcsPipe2(\"%s\", ...): SetHandleInformation() failed.\n",
                            striAsUnquotedCStri(command)););
            err_info = FILE_ERROR;
          } else {
            memset(&startupInfo, 0, sizeof(startupInfo));
            /* memset(&processInformation, 0, sizeof(processInformation)); */
            startupInfo.cb = sizeof(startupInfo);
            startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
            startupInfo.wShowWindow = SW_HIDE;
            startupInfo.hStdInput = childInputRead;
            startupInfo.hStdOutput = childOutputWrite;
            startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            logMessage(printf("before CreateProcessW(\"" FMT_S_OS "\", \"" FMT_S_OS "\", ...)\n",
                              os_command_stri, command_line););
            if (CreateProcessW(os_command_stri,
                               command_line /* lpCommandLine */,
                               NULL /* lpProcessAttributes */,
                               NULL /* lpThreadAttributes */,
                               1  /* bInheritHandles */,
                               CREATE_UNICODE_ENVIRONMENT /* dwCreationFlags */,
                               NULL /* lpEnvironment */,
                               NULL /* lpCurrentDirectory */,
                               &startupInfo,
                               &processInformation) != 0) {
              CloseHandle(childInputRead);
              CloseHandle(childOutputWrite);
              childStdinFileno = _open_osfhandle((intPtrType) (childInputWrite), _O_TEXT);
              childStdoutFileno = _open_osfhandle((intPtrType) (childOutputRead), _O_TEXT);
              logMessage(printf("pcsPipe2: childStdinFileno=%d\n",
                                childStdinFileno););
              logMessage(printf("pcsPipe2: childStdoutFileno=%d\n",
                                childStdoutFileno););
              os_setmode(childStdinFileno, _O_BINARY);
              os_setmode(childStdoutFileno, _O_BINARY);
              initFileType(childStdinFile, FALSE, TRUE);
              childStdinFile->cFile = os_fdopen(childStdinFileno, "w");
              if (unlikely(childStdinFile->cFile == NULL)) {
                logError(printf("pcsPipe2: stdin "
                                "os_fdopen(%d, \"w\") returned NULL\n"
                                "errno=%d\nerror: %s\n",
                                childStdinFileno, errno, strerror(errno)););
                FREE_RECORD(childStdinFile, fileRecord, count.files);
              } else {
                filDestr(*childStdin);
                *childStdin = childStdinFile;
              } /* if */
              initFileType(childStdoutFile, TRUE, FALSE);
              childStdoutFile->cFile = os_fdopen(childStdoutFileno, "r");
              if (unlikely(childStdoutFile->cFile == NULL)) {
                logError(printf("pcsPipe2: stdout "
                                "os_fdopen(%d, \"r\") returned NULL\n"
                                "errno=%d\nerror: %s\n",
                                childStdoutFileno, errno, strerror(errno)););
                FREE_RECORD(childStdoutFile, fileRecord, count.files);
              } else {
                filDestr(*childStdout);
                *childStdout = childStdoutFile;
              } /* if */
              CloseHandle(processInformation.hProcess);
              CloseHandle(processInformation.hThread);
            } else {
              logError(printf("pcsPipe2: CreateProcessW(\"" FMT_S_OS "\", \"" FMT_S_OS "\", ...) failed.\n"
                              "GetLastError=" FMT_U32 "\n",
                              os_command_stri, command_line, (uint32Type) GetLastError());
                       printf("ERROR_FILE_NOT_FOUND=%d\n", (int) ERROR_FILE_NOT_FOUND););
              err_info = FILE_ERROR;
            } /* if */
            /* printf("after CreateProcessW\n"); */
          } /* if */
        } /* if */
        os_stri_free(command_line);
      } /* if */
      os_stri_free(os_command_stri);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      if (childStdinFile != NULL) {
        FREE_RECORD(childStdinFile, fileRecord, count.files);
      } /* if */
      if (childStdoutFile != NULL) {
        FREE_RECORD(childStdoutFile, fileRecord, count.files);
      } /* if */
      if (childInputRead != INVALID_HANDLE_VALUE) {
        CloseHandle(childInputRead);
      } /* if */
      if (childInputWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(childInputWrite);
      } /* if */
      if (childOutputRead != INVALID_HANDLE_VALUE) {
        CloseHandle(childOutputRead);
      } /* if */
      if (childOutputWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(childOutputWrite);
      } /* if */
      raise_error(err_info);
    } /* if */
    logFunction(printf("pcsPipe2(\"%s\"", striAsUnquotedCStri(command));
                printParameters(parameters);
                printf(", " FMT_U_MEM " %s%d, " FMT_U_MEM " %s%d) -->\n",
                       (memSizeType) *childStdin,
                       *childStdin == NULL ? "NULL " : "",
                       *childStdin != NULL ? safe_fileno((*childStdin)->cFile) : 0,
                       (memSizeType) *childStdout,
                       *childStdout == NULL ? "NULL " : "",
                       *childStdout != NULL ? safe_fileno((*childStdout)->cFile) : 0););
  } /* pcsPipe2 */



void pcsPty (const const_striType command, const const_rtlArrayType parameters,
    fileType *childStdin, fileType *childStdout)

  { /* pcsPty */
    pcsPipe2(command, parameters, childStdin, childStdout);
  } /* pcsPty */



static HANDLE getHandleFromFile (cFileType aFile, errInfoType *err_info)

  {
    int file_no;
    HANDLE fileHandle;

  /* getHandleFromFile */
    if (aFile == NULL) {
      fileHandle = CreateFile(NULL_DEVICE, GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL, OPEN_EXISTING, 0, NULL);
      if (unlikely(fileHandle == INVALID_HANDLE_VALUE)) {
        logError(printf("CreateFile(\"nul:\", ...) failed.\n"););
      } /* if */
    } else {
      file_no = os_fileno(aFile);
      if (unlikely(file_no == -1)) {
        logError(printf("getHandleFromFile(%d): os_fileno(%d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        safe_fileno(aFile), safe_fileno(aFile),
                        errno, strerror(errno)););
        *err_info = FILE_ERROR;
        fileHandle = INVALID_HANDLE_VALUE;
      } else {
        fileHandle = (HANDLE) _get_osfhandle(file_no);
        if (unlikely(fileHandle == INVALID_HANDLE_VALUE)) {
          logError(printf("getHandleFromFile(%d): _get_osfhandle(%d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          safe_fileno(aFile), safe_fileno(aFile),
                          errno, strerror(errno)););
          *err_info = FILE_ERROR;
        } /* if */
      } /* if */
    } /* if */
    return fileHandle;
  } /* getHandleFromFile */



processType pcsStart (const const_striType command, const const_rtlArrayType parameters,
    fileType redirectStdin, fileType redirectStdout, fileType redirectStderr)

  {
    cFileType childStdin;
    cFileType childStdout;
    cFileType childStderr;
    os_striType os_command_stri;
    os_striType command_line;
    STARTUPINFOW startupInfo;
    PROCESS_INFORMATION processInformation;
    int path_info = PATH_IS_NORMAL;
    HANDLE stdinFileHandle;
    HANDLE stdoutFileHandle;
    HANDLE stderrFileHandle;
    errInfoType err_info = OKAY_NO_ERROR;
    win_processType process = NULL;

  /* pcsStart */
    logFunction(printf("pcsStart(\"%s\"", striAsUnquotedCStri(command));
                printParameters(parameters);
                printf(", %s%d, %s%d, %s%d)\n",
                       redirectStdin == NULL ? "NULL " : "",
                       redirectStdin != NULL ? safe_fileno(redirectStdin->cFile) : 0,
                       redirectStdout == NULL ? "NULL " : "",
                       redirectStdout != NULL ? safe_fileno(redirectStdout->cFile) : 0,
                       redirectStderr == NULL ? "NULL " : "",
                       redirectStderr != NULL ? safe_fileno(redirectStderr->cFile) : 0););
    childStdin = redirectStdin->cFile;
    childStdout = redirectStdout->cFile;
    childStderr = redirectStderr->cFile;
    stdinFileHandle = getHandleFromFile(childStdin, &err_info);
    stdoutFileHandle = getHandleFromFile(childStdout, &err_info);
    stderrFileHandle = getHandleFromFile(childStderr, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
      os_command_stri = cp_to_os_path(command, &path_info, &err_info);
      if (likely(os_command_stri != NULL)) {
        command_line = prepareCommandLine(os_command_stri, parameters, &err_info);
        if (likely(command_line != NULL)) {
          if (!ALLOC_RECORD(process, win_processRecord, count.process)) {
            err_info = MEMORY_ERROR;
          } else {
            memset(&startupInfo, 0, sizeof(startupInfo));
            /* memset(&processInformation, 0, sizeof(processInformation)); */
            startupInfo.cb = sizeof(startupInfo);
            startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
            startupInfo.wShowWindow = SW_SHOWNORMAL;
            startupInfo.hStdInput = stdinFileHandle;
            startupInfo.hStdOutput = stdoutFileHandle;
            startupInfo.hStdError = stderrFileHandle;
            logMessage(printf("before CreateProcessW(\"" FMT_S_OS "\", \"" FMT_S_OS "\", ...)\n",
                              os_command_stri, command_line););
            if (CreateProcessW(os_command_stri,
                               command_line /* lpCommandLine */,
                               NULL /* lpProcessAttributes */,
                               NULL /* lpThreadAttributes */,
                               1  /* bInheritHandles */,
                               CREATE_UNICODE_ENVIRONMENT /* dwCreationFlags */,
                               NULL /* lpEnvironment */,
                               NULL /* lpCurrentDirectory */,
                               &startupInfo,
                               &processInformation) != 0) {
              /* printf("pcsStart: pProcess=" FMT_U_MEM "\n",
                  (memSizeType) (processInformation.hProcess)); */
              /* printf("pcsStart: PID=" FMT_U32 "\n", processInformation.dwProcessId); */
              memset(process, 0, sizeof(win_processRecord));
              process->usage_count = 1;
              process->hProcess = processInformation.hProcess;
              process->hThread  = processInformation.hThread;
              process->pid      = processInformation.dwProcessId;
              process->isTerminated = FALSE;
            } else {
              logError(printf("pcsStart: CreateProcessW(\"" FMT_S_OS "\", \"" FMT_S_OS "\", ...) failed.\n"
                              "GetLastError=" FMT_U32 "\n",
                              os_command_stri, command_line, (uint32Type) GetLastError());
                       printf("ERROR_FILE_NOT_FOUND=%d\n", (int) ERROR_FILE_NOT_FOUND););
              FREE_RECORD(process, win_processRecord, count.process);
              process = NULL;
              err_info = FILE_ERROR;
            } /* if */
            /* printf("after CreateProcessW\n"); */
          } /* if */
          os_stri_free(command_line);
        } /* if */
        os_stri_free(os_command_stri);
      } /* if */
    } /* if */
    if (childStdin == NULL && stdinFileHandle != INVALID_HANDLE_VALUE) {
      CloseHandle(stdinFileHandle);
    } /* if */
    if (childStdout == NULL && stdoutFileHandle != INVALID_HANDLE_VALUE) {
      CloseHandle(stdoutFileHandle);
    } /* if */
    if (childStderr == NULL && stderrFileHandle != INVALID_HANDLE_VALUE) {
      CloseHandle(stderrFileHandle);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("pcsStart --> " FMT_U_MEM
                       " (pid=" FMT_U32 ", usage=" FMT_U ")\n",
                       (memSizeType) process,
                       (uint32Type) (process != NULL ? process->pid : 0),
                       process != NULL ? process->usage_count
                                       : (uintType) 0););
    return (processType) process;
  } /* pcsStart */



processType pcsStartPipe (const const_striType command, const const_rtlArrayType parameters)

  {
    os_striType os_command_stri;
    os_striType command_line;
    fileType childStdinFile = NULL;
    fileType childStdoutFile = NULL;
    fileType childStderrFile = NULL;
    SECURITY_ATTRIBUTES saAttr;
    HANDLE childInputRead = INVALID_HANDLE_VALUE;
    HANDLE childInputWrite = INVALID_HANDLE_VALUE;
    HANDLE childOutputRead = INVALID_HANDLE_VALUE;
    HANDLE childOutputWrite = INVALID_HANDLE_VALUE;
    HANDLE childErrorRead = INVALID_HANDLE_VALUE;
    HANDLE childErrorWrite = INVALID_HANDLE_VALUE;
    int childStdinFileno;
    int childStdoutFileno;
    int childStderrFileno;
    STARTUPINFOW startupInfo;
    PROCESS_INFORMATION processInformation;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    win_processType process = NULL;

  /* pcsStartPipe */
    logFunction(printf("pcsStartPipe(\"%s\"", striAsUnquotedCStri(command));
                printParameters(parameters);
                printf(")\n"););
    os_command_stri = cp_to_os_path(command, &path_info, &err_info);
    if (likely(os_command_stri != NULL)) {
      command_line = prepareCommandLine(os_command_stri, parameters, &err_info);
      if (likely(command_line != NULL)) {
        if (unlikely(!ALLOC_RECORD(process, win_processRecord, count.process) ||
                     !ALLOC_RECORD(childStdinFile, fileRecord, count.files) ||
                     !ALLOC_RECORD(childStdoutFile, fileRecord, count.files) ||
                     !ALLOC_RECORD(childStderrFile, fileRecord, count.files))) {
          err_info = MEMORY_ERROR;
        } else {
          /* printf("pcsStartPipe(%ls, %ls)\n", os_command_stri, command_line); */
          saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
          saAttr.bInheritHandle = TRUE;
          saAttr.lpSecurityDescriptor = NULL;
          if (unlikely(CreatePipe(&childInputRead,
                                  &childInputWrite, &saAttr, 0) == 0 ||
                       CreatePipe(&childOutputRead,
                                  &childOutputWrite, &saAttr, 0) == 0 ||
                       CreatePipe(&childErrorRead,
                                  &childErrorWrite, &saAttr, 0) == 0)) {
            logError(printf("pcsPipe2(\"%s\", ...): CreatePipe() failed.\n",
                            striAsUnquotedCStri(command)););
            err_info = FILE_ERROR;
          } else if (unlikely(SetHandleInformation(childInputWrite,
                                                   HANDLE_FLAG_INHERIT, 0) == 0 ||
                              SetHandleInformation(childOutputRead,
                                                   HANDLE_FLAG_INHERIT, 0) == 0 ||
                              SetHandleInformation(childErrorRead,
                                                   HANDLE_FLAG_INHERIT, 0) == 0)) {
            logError(printf("pcsPipe2(\"%s\", ...): SetHandleInformation() failed.\n",
                            striAsUnquotedCStri(command)););
            err_info = FILE_ERROR;
          } else {
            memset(&startupInfo, 0, sizeof(startupInfo));
            /* memset(&processInformation, 0, sizeof(processInformation)); */
            startupInfo.cb = sizeof(startupInfo);
            startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
            startupInfo.wShowWindow = SW_HIDE;
            startupInfo.hStdInput = childInputRead;
            startupInfo.hStdOutput = childOutputWrite;
            startupInfo.hStdError = childErrorWrite;
            logMessage(printf("before CreateProcessW(\"" FMT_S_OS "\", \"" FMT_S_OS "\", ...)\n",
                              os_command_stri, command_line););
            if (CreateProcessW(os_command_stri,
                               command_line /* lpCommandLine */,
                               NULL /* lpProcessAttributes */,
                               NULL /* lpThreadAttributes */,
                               1  /* bInheritHandles */,
                               CREATE_UNICODE_ENVIRONMENT /* dwCreationFlags */,
                               NULL /* lpEnvironment */,
                               NULL /* lpCurrentDirectory */,
                               &startupInfo,
                               &processInformation) != 0) {
              CloseHandle(childInputRead);
              CloseHandle(childOutputWrite);
              CloseHandle(childErrorWrite);
              childStdinFileno = _open_osfhandle((intPtrType) (childInputWrite), _O_TEXT);
              childStdoutFileno = _open_osfhandle((intPtrType) (childOutputRead), _O_TEXT);
              childStderrFileno = _open_osfhandle((intPtrType) (childErrorRead), _O_TEXT);
              logMessage(printf("pcsStartPipe: childStdinFileno=%d\n",
                                childStdinFileno););
              logMessage(printf("pcsStartPipe: childStdoutFileno=%d\n",
                                childStdoutFileno););
              logMessage(printf("pcsStartPipe: childStderrFileno=%d\n",
                                childStderrFileno););
              os_setmode(childStdinFileno, _O_BINARY);
              os_setmode(childStdoutFileno, _O_BINARY);
              os_setmode(childStderrFileno, _O_BINARY);
              memset(process, 0, sizeof(win_processRecord));
              process->usage_count = 1;
              process->hProcess = processInformation.hProcess;
              process->hThread  = processInformation.hThread;
              process->pid      = processInformation.dwProcessId;
              process->isTerminated = FALSE;
              initFileType(childStdinFile, FALSE, TRUE);
              childStdinFile->cFile = os_fdopen(childStdinFileno, "w");
              if (unlikely(childStdinFile->cFile == NULL)) {
                logError(printf("pcsStartPipe: stdin "
                                "os_fdopen(%d, \"w\") returned NULL\n"
                                "errno=%d\nerror: %s\n",
                                childStdinFileno, errno, strerror(errno)););
                FREE_RECORD(childStdinFile, fileRecord, count.files);
                process->stdIn = NULL;
              } else {
                logMessage(printf("pcsStartPipe: childStdinFile=%s%d\n",
                                  childStdinFile->cFile == NULL ?
                                      "NULL " : "",
                                  childStdinFile->cFile != NULL ?
                                      safe_fileno(childStdinFile->cFile) : 0););
                process->stdIn = childStdinFile;
              } /* if */
              initFileType(childStdoutFile, TRUE, FALSE);
              childStdoutFile->cFile = os_fdopen(childStdoutFileno, "r");
              if (unlikely(childStdoutFile->cFile == NULL)) {
                logError(printf("pcsStartPipe: stdout "
                                "os_fdopen(%d, \"r\") returned NULL\n"
                                "errno=%d\nerror: %s\n",
                                childStdoutFileno, errno, strerror(errno)););
                FREE_RECORD(childStdoutFile, fileRecord, count.files);
                process->stdOut = NULL;
              } else {
                logMessage(printf("pcsStartPipe: childStdoutFile=%s%d\n",
                                  childStdoutFile->cFile == NULL ?
                                      "NULL " : "",
                                  childStdoutFile->cFile != NULL ?
                                      safe_fileno(childStdoutFile->cFile) : 0););
                process->stdOut = childStdoutFile;
              } /* if */
              initFileType(childStderrFile, TRUE, FALSE);
              childStderrFile->cFile = os_fdopen(childStderrFileno, "r");
              if (unlikely(childStderrFile->cFile == NULL)) {
                logError(printf("pcsStartPipe: stderr "
                                "os_fdopen(%d, \"r\") returned NULL\n"
                                "errno=%d\nerror: %s\n",
                                childStderrFileno, errno, strerror(errno)););
                FREE_RECORD(childStderrFile, fileRecord, count.files);
                process->stdErr = NULL;
              } else {
                logMessage(printf("pcsStartPipe: childStderrFile=%s%d\n",
                                  childStderrFile->cFile == NULL ?
                                      "NULL " : "",
                                  childStderrFile->cFile != NULL ?
                                      safe_fileno(childStderrFile->cFile) : 0););
                process->stdErr = childStderrFile;
              } /* if */
            } else {
              logError(printf("pcsStartPipe: CreateProcessW(\"" FMT_S_OS "\", \"" FMT_S_OS "\", ...) failed.\n"
                              "GetLastError=" FMT_U32 "\n",
                              os_command_stri, command_line, (uint32Type) GetLastError());
                       printf("ERROR_FILE_NOT_FOUND=%d\n", (int) ERROR_FILE_NOT_FOUND););
              FREE_RECORD(process, win_processRecord, count.process);
              process = NULL;
              err_info = FILE_ERROR;
            } /* if */
            /* printf("after CreateProcessW\n"); */
          } /* if */
        } /* if */
        os_stri_free(command_line);
      } /* if */
      os_stri_free(os_command_stri);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      if (childStdinFile != NULL) {
        FREE_RECORD(childStdinFile, fileRecord, count.files);
      } /* if */
      if (childStdoutFile != NULL) {
        FREE_RECORD(childStdoutFile, fileRecord, count.files);
      } /* if */
      if (childStderrFile != NULL) {
        FREE_RECORD(childStderrFile, fileRecord, count.files);
      } /* if */
      if (childInputRead != INVALID_HANDLE_VALUE) {
        CloseHandle(childInputRead);
      } /* if */
      if (childInputWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(childInputWrite);
      } /* if */
      if (childOutputRead != INVALID_HANDLE_VALUE) {
        CloseHandle(childOutputRead);
      } /* if */
      if (childOutputWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(childOutputWrite);
      } /* if */
      if (childErrorRead != INVALID_HANDLE_VALUE) {
        CloseHandle(childErrorRead);
      } /* if */
      if (childErrorWrite != INVALID_HANDLE_VALUE) {
        CloseHandle(childErrorWrite);
      } /* if */
      if (process != NULL) {
        FREE_RECORD(process, win_processRecord, count.process);
        process = NULL;
      } /* if */
      raise_error(err_info);
    } /* if */
    logFunction(printf("pcsStartPipe --> " FMT_U_MEM
                       " (pid=" FMT_U32 ", usage=" FMT_U ")\n",
                       (memSizeType) process,
                       (uint32Type) (process != NULL ? process->pid : 0),
                       process != NULL ? process->usage_count
                                       : (uintType) 0););
    return (processType) process;
  } /* pcsStartPipe */



/**
 *  Convert a 'process' to a string.
 *  The process is converted to a string with the process identifier (PID).
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType pcsStr (const const_processType process)

  {
    striType result;

  /* pcsStr */
    if (process == NULL) {
      result = CSTRI_LITERAL_TO_STRI("NULL");
      if (unlikely(result == NULL)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } else {
      result = intStr((intType) to_pid(process));
    } /* if */
    return result;
  } /* pcsStr */



/**
 *  Wait until the specified child process has terminated.
 *  Suspend the execution of the calling process until the
 *  specified child has terminated.
 */
void pcsWaitFor (const processType process)

  {
    DWORD exitCode = 0;

  /* pcsWaitFor */
    logFunction(printf("pcsWaitFor(" FMT_U_MEM
                       " (pid=" FMT_U32 ", usage=" FMT_U ", hProcess=" FMT_U_MEM "))\n",
                       (memSizeType) process,
                       (uint32Type) (process != NULL ? to_pid(process) : 0),
                       process != NULL ? process->usage_count : (uintType) 0,
                       process != NULL ? (memSizeType) to_hProcess(process) : (memSizeType) 0););
    if (unlikely(process == NULL)) {
      logError(printf("pcsWaitFor: process == NULL\n"););
      raise_error(FILE_ERROR);
    } else if (!to_isTerminated(process)) {
      if (WaitForSingleObject(to_hProcess(process), INFINITE) == WAIT_OBJECT_0) {
        if (GetExitCodeProcess(to_hProcess(process), &exitCode) != 0) {
          to_var_isTerminated(process) = TRUE;
          to_var_exitValue(process) = exitCode;
        } else {
          logError(printf("pcsWaitFor: GetExitCodeProcess(" FMT_U_MEM ", *) failed:\n"
                          "GetLastError=" FMT_U32 "\n",
                          (memSizeType) to_hProcess(process), (uint32Type) GetLastError());
                   printf("PID=" FMT_U32 "\n", (uint32Type) to_pid(process)););
          raise_error(FILE_ERROR);
        } /* if */
      } else {
        logError(printf("pcsWaitFor: WaitForSingleObject(" FMT_U_MEM ", INFINITE) failed:\n"
                        "GetLastError=" FMT_U32 "\n",
                        (memSizeType) to_hProcess(process), (uint32Type) GetLastError());
                 printf("PID=" FMT_U32 "\n", (uint32Type) to_pid(process)););
        raise_error(FILE_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("pcsWaitFor -->\n"););
  } /* pcsWaitFor */
