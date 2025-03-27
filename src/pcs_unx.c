/********************************************************************/
/*                                                                  */
/*  pcs_unx.c     Process functions which use the Unix API.         */
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
/*  File: seed7/src/pcs_unx.c                                       */
/*  Changes: 2010, 2012, 2014  Thomas Mertes                        */
/*  Content: Process functions which use the Unix API.              */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#define _XOPEN_SOURCE 600
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "fcntl.h"
#include "signal.h"
#include "errno.h"

#if UNISTD_H_PRESENT
#include "unistd.h"
#endif

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "int_rtl.h"
#include "fil_rtl.h"
#include "rtl_err.h"


typedef struct {
    uintType usage_count;
    fileType stdIn;
    fileType stdOut;
    fileType stdErr;
    /* Up to here the structure is identical to struct processStruct */
    pid_t pid;
    boolType isTerminated;
    int exitValue;
  } unx_processRecord, *unx_processType;

typedef const unx_processRecord *const_unx_processType;

#if DO_HEAP_STATISTIC
size_t sizeof_processRecord = sizeof(unx_processRecord);
#endif

#define to_pid(process)          (((const_unx_processType) (process))->pid)
#define to_isTerminated(process) (((const_unx_processType) (process))->isTerminated)
#define to_exitValue(process)    (((const_unx_processType) (process))->exitValue)

#define to_var_pid(process)          (((unx_processType) (process))->pid)
#define to_var_isTerminated(process) (((unx_processType) (process))->isTerminated)
#define to_var_exitValue(process)    (((unx_processType) (process))->exitValue)



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
 *  Free an argument vector that was created by genArgVector().
 *  The individual arguments are freed in the reverse order
 *  of their creation. This allows that os_stri_free() works
 *  in a stack like manner.
 */
static void freeArgVector (os_striType *argv)

  {
    memSizeType pos = 0;

  /* freeArgVector */
    logFunction(printf("freeArgVector(");
                if (argv == NULL) {
                  printf("NULL)\n");
                } else {
                  printf("{");
                  for (pos = 0; argv[pos] != NULL; pos++) {
                    printf("\"" FMT_S_OS "\", ", argv[pos]);
                  } /* for */
                  printf("NULL})\n");
                } /* if */);
    while (argv[pos] != NULL) {
      pos++;
    } /* while */
    for (; pos > 0; pos--) {
      os_stri_free(argv[pos - 1]);
    } /* for */
    free(argv);
  } /* freeArgVector */



/**
 *  Generate an argument vector that can be used by execv().
 *  The argument vector must be freed with freeArgVector().
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if a memory allocation failed, and
 *                  RANGE_ERROR if a conversion to os_striType failed.
 *  @return the argument vector, or
 *          NULL if an error occurred.
 */
static os_striType *genArgVector (const const_striType command,
    const const_rtlArrayType parameters, errInfoType *err_info)

  {
    memSizeType arraySize;
    memSizeType pos;
    int path_info = PATH_IS_NORMAL;
    os_striType *argv;

  /* genArgVector */
    arraySize = arraySize(parameters);
    argv = (os_striType *) malloc(sizeof(os_striType) * (arraySize + 2));
    if (unlikely(argv == NULL)) {
      *err_info = MEMORY_ERROR;
    } else {
      argv[0] = cp_to_os_path(command, &path_info, err_info);
      if (unlikely(argv[0] == NULL)) {
        logError(printf("genArgVector: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "err_info=%d\n",
                        striAsUnquotedCStri(command), *err_info););
        free(argv);
        argv = NULL;
      } else {
        /* fprintf(stderr, "argv[0]=" FMT_S_OS "\n", argv[0]); */
        for (pos = 0; pos < arraySize && *err_info == OKAY_NO_ERROR; pos++) {
          argv[pos + 1] = stri_to_os_stri(parameters->arr[pos].value.striValue, err_info);
          if (unlikely(argv[pos + 1] == NULL)) {
            logError(printf("genArgVector: stri_to_os_stri(\"%s\", *) failed:\n"
                            "err_info=%d\n",
                            striAsUnquotedCStri(parameters->arr[pos].value.striValue),
                            *err_info););
          } /* if */
        } /* for */
        if (unlikely(*err_info != OKAY_NO_ERROR)) {
          /* Free the individual arguments in the reverse order */
          /* of their creation. This allows that os_stri_free() */
          /* works in a stack like manner.                      */
          while (pos >= 1) {
            pos--;
            os_stri_free(argv[pos]);
          } /* while */
          free(argv);
          argv = NULL;
        } else {
          argv[arraySize + 1] = NULL;
          /* fprintf(stderr, "argv[%d]=NULL\n", arraySize + 1); */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("genArgVector --> ");
                if (argv == NULL) {
                  printf("NULL\n");
                } else {
                  printf("{");
                  for (pos = 0; argv[pos] != NULL; pos++) {
                    printf("\"" FMT_S_OS "\", ", argv[pos]);
                  } /* for */
                  printf("NULL}\n");
                } /* if */);
    return argv;
  } /* genArgVector */



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
    logFunction(printf("pcsExitValue(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       process != NULL ? (memSizeType) to_pid(process) : (memSizeType) 0,
                       process != NULL ? process->usage_count : (uintType) 0););
    if (unlikely(process == NULL)) {
      logError(printf("pcsExitValue: process == NULL\n"););
      raise_error(FILE_ERROR);
      exitValue = -1;
    } else if (unlikely(!to_isTerminated(process))) {
      logError(printf("pcsExitValue(" FMT_U_MEM " (usage=" FMT_U ")): "
                      "Process has not terminated.\n",
                      process != NULL ? (memSizeType) to_pid(process) : (memSizeType) 0,
                      process != NULL ? process->usage_count : (uintType) 0););
      raise_error(FILE_ERROR);
      exitValue = -1;
    } else {
      exitValue = (intType) to_exitValue(process);
    } /* if */
    logFunction(printf("pcsExitValue(" FMT_U_MEM " (usage=" FMT_U ")) --> " FMT_D "\n",
                       process != NULL ? (memSizeType) to_pid(process) : (memSizeType) 0,
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
    logFunction(printf("pcsFree(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       oldProcess != NULL ? (memSizeType) to_pid(oldProcess) : (memSizeType) 0,
                       oldProcess != NULL ? oldProcess->usage_count : (uintType) 0););
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
    FREE_RECORD(oldProcess, unx_processRecord, count.process);
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
    int waitpid_result;
    int status;
    boolType isAlive;

  /* pcsIsAlive */
    logFunction(printf("pcsIsAlive(" FMT_U_MEM ")\n",
                       process != NULL ? (memSizeType) to_pid(process) : (memSizeType) 0););
    if (process == NULL || to_isTerminated(process)) {
      isAlive = FALSE;
    } else {
      status = 0;
      waitpid_result = waitpid(to_pid(process), &status, WNOHANG);
      /*
      printf("pcsIsAlive: waitpid_result=%d\n", waitpid_result);
      printf("pcsIsAlive: status=0x%08x\n", status);
      printf("pcsIsAlive: WIFEXITED(status)=%d\n", WIFEXITED(status));
      printf("pcsIsAlive: WEXITSTATUS(status)=%d\n", WEXITSTATUS(status));
      printf("pcsIsAlive: WIFSIGNALED(status)=%d\n", WIFSIGNALED(status));
      printf("pcsIsAlive: WTERMSIG(status)=%d\n", WTERMSIG(status));
      printf("pcsIsAlive: WIFSTOPPED(status)=%d\n", WIFSTOPPED(status));
      printf("pcsIsAlive: WSTOPSIG(status)=%d\n", WSTOPSIG(status));
      printf("pcsIsAlive: WIFCONTINUED(status)=%d\n", WIFCONTINUED(status));
      */
      if (waitpid_result == 0) {
        isAlive = TRUE;
      } else if (waitpid_result == to_pid(process)) {
        if (WIFEXITED(status)) {
          to_var_isTerminated(process) = TRUE;
          to_var_exitValue(process) = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
          to_var_isTerminated(process) = TRUE;
          to_var_exitValue(process) = 0;
        } /* if */
      } else {
        to_var_isTerminated(process) = TRUE;
        to_var_exitValue(process) = 0;
      } /* if */
      isAlive = !to_isTerminated(process);
    } /* if */
    return isAlive;
  } /* pcsIsAlive */



/**
 *  Kill the specified process.
 *  @exception FILE_ERROR It was not possible to kill the process.
 */
void pcsKill (const processType process)

  { /* pcsKill */
    logFunction(printf("pcsKill(" FMT_U_MEM ")\n",
                       process != NULL ? (memSizeType) to_pid(process) : (memSizeType) 0););
    if (unlikely(process == NULL)) {
      logError(printf("pcsKill: process == NULL\n"););
      raise_error(FILE_ERROR);
    } else if (unlikely(kill(to_pid(process), SIGKILL) != 0)) {
      logError(printf("pcsKill: kill(%d, SIGKILL) failed:\n"
                      "errno=%d\nerror: %s\n",
                      to_pid(process), errno, strerror(errno)););
      raise_error(FILE_ERROR);
    } /* if */
  } /* pcsKill */



void pcsPipe2 (const const_striType command, const const_rtlArrayType parameters,
    fileType *childStdin, fileType *childStdout)

  {
    os_striType *argv;
    fileType childStdinFile;
    fileType childStdoutFile;
    int childStdinPipes[2];
    int childStdoutPipes[2];
    errInfoType err_info = OKAY_NO_ERROR;
    pid_t pid;

  /* pcsPipe2 */
    logFunction(printf("pcsPipe2(\"%s\"", striAsUnquotedCStri(command));
                printParameters(parameters);
                printf(", %s%d, %s%d)\n",
                       *childStdin == NULL ? "NULL " : "",
                       *childStdin != NULL ? safe_fileno((*childStdin)->cFile) : 0,
                       *childStdout == NULL ? "NULL " : "",
                       *childStdout != NULL ? safe_fileno((*childStdout)->cFile) : 0););
    argv = genArgVector(command, parameters, &err_info);
    if (unlikely(argv == NULL)) {
      raise_error(err_info);
    } else if (unlikely(access(argv[0], X_OK) != 0)) {
      logError(printf("pcsPipe2: No execute permission for " FMT_S_OS "\n", argv[0]););
      freeArgVector(argv);
      raise_error(FILE_ERROR);
    } else if (unlikely(!ALLOC_RECORD(childStdinFile, fileRecord, count.files))) {
      freeArgVector(argv);
      raise_error(MEMORY_ERROR);
    } else if (unlikely(!ALLOC_RECORD(childStdoutFile, fileRecord, count.files))) {
      freeArgVector(argv);
      FREE_RECORD(childStdinFile, fileRecord, count.files);
      raise_error(MEMORY_ERROR);
    } else if (unlikely(pipe(childStdinPipes) != 0)) {
      logError(printf("pcsPipe2: pipe(childStdinPipes) failed:\n"
                      "errno=%d\nerror: %s\n",
                      errno, strerror(errno)););
      freeArgVector(argv);
      FREE_RECORD(childStdinFile, fileRecord, count.files);
      FREE_RECORD(childStdoutFile, fileRecord, count.files);
      raise_error(FILE_ERROR);
    } else if (unlikely(pipe(childStdoutPipes) != 0)) {
      logError(printf("pcsPipe2: pipe(childStdoutPipes) failed:\n"
                      "errno=%d\nerror: %s\n",
                      errno, strerror(errno)););
      freeArgVector(argv);
      FREE_RECORD(childStdinFile, fileRecord, count.files);
      FREE_RECORD(childStdoutFile, fileRecord, count.files);
      close(childStdinPipes[0]);
      close(childStdinPipes[1]);
      raise_error(FILE_ERROR);
    } else {
      pid = fork();
      if (pid == 0) {
        dup2(childStdinPipes[0], 0); /* Make the read end of childStdinPipes pipe as stdin */
        dup2(childStdoutPipes[1], 1);  /* Make the write end of childStdoutPipes as stdout */
        close(childStdinPipes[0]); /* Not required for the child */
        close(childStdinPipes[1]);
        close(childStdoutPipes[0]);
        close(childStdoutPipes[1]);
        execv(argv[0], argv);
        logError(printf("pcsPipe2: execv(" FMT_S_OS ") failed:\nerrno=%d\nerror: %s\n",
                        argv[0], errno, strerror(errno)););
      } else if (unlikely(pid == (pid_t) -1)) {
        logError(printf("pcsPipe2: fork failed:\nerrno=%d\nerror: %s\n",
                        errno, strerror(errno)););
        close(childStdinPipes[0]); /* The pipes are unused */
        close(childStdoutPipes[1]);
        close(childStdinPipes[1]);
        close(childStdoutPipes[0]);
        freeArgVector(argv);
        FREE_RECORD(childStdinFile, fileRecord, count.files);
        FREE_RECORD(childStdoutFile, fileRecord, count.files);
        raise_error(FILE_ERROR);
      } else {
        close(childStdinPipes[0]); /* These are being used by the child */
        close(childStdoutPipes[1]);
        initFileType(childStdinFile, FALSE, TRUE);
        childStdinFile->cFile = os_fdopen(childStdinPipes[1], "w");
        if (childStdinFile->cFile == NULL) {
          FREE_RECORD(childStdinFile, fileRecord, count.files);
        } else {
          filDestr(*childStdin);
          *childStdin  = childStdinFile;
        } /* if */
        initFileType(childStdoutFile, TRUE, FALSE);
        childStdoutFile->cFile = os_fdopen(childStdoutPipes[0], "r");
        if (childStdoutFile->cFile == NULL) {
          FREE_RECORD(childStdoutFile, fileRecord, count.files);
        } else {
          filDestr(*childStdout);
          *childStdout = childStdoutFile;
        } /* if */
        freeArgVector(argv);
      } /* if */
    } /* if */
    logFunction(printf("pcsPipe2(\"%s\"", striAsUnquotedCStri(command));
                printParameters(parameters);
                printf(", %s%d, %s%d) -->\n",
                       *childStdin == NULL ? "NULL " : "",
                       *childStdin != NULL ? safe_fileno((*childStdin)->cFile) : 0,
                       *childStdout == NULL ? "NULL " : "",
                       *childStdout != NULL ? safe_fileno((*childStdout)->cFile) : 0););
  } /* pcsPipe2 */



#ifdef IMPLEMENT_PTY_WITH_PIPE2
void pcsPty (const const_striType command, const const_rtlArrayType parameters,
    fileType *childStdin, fileType *childStdout)

  { /* pcsPty */
    pcsPipe2(command, parameters, childStdin, childStdout);
  } /* pcsPty */

#else



void pcsPty (const const_striType command, const const_rtlArrayType parameters,
    fileType *childStdin, fileType *childStdout)

  {
    os_striType *argv;
    fileType childStdinFile;
    fileType childStdoutFile;
    int masterfd;
    int slavefd;
    char *slavedevice;
    int savedStdin;
    int savedStdout;
    errInfoType err_info = OKAY_NO_ERROR;
    pid_t pid;

  /* pcsPty */
    logFunction(printf("pcsPty(\"%s\"", striAsUnquotedCStri(command));
                printParameters(parameters);
                printf(")\n"););
    argv = genArgVector(command, parameters, &err_info);
    if (unlikely(argv == NULL)) {
      raise_error(err_info);
    } else if (unlikely(access(argv[0], X_OK) != 0)) {
      logError(printf("pcsPty: No execute permission for " FMT_S_OS "\n", argv[0]););
      freeArgVector(argv);
      raise_error(FILE_ERROR);
    } else if (unlikely(!ALLOC_RECORD(childStdinFile, fileRecord, count.files))) {
      freeArgVector(argv);
      raise_error(MEMORY_ERROR);
    } else if (unlikely(!ALLOC_RECORD(childStdoutFile, fileRecord, count.files))) {
      freeArgVector(argv);
      FREE_RECORD(childStdinFile, fileRecord, count.files);
      raise_error(MEMORY_ERROR);
    } else {
      masterfd = posix_openpt(O_RDWR|O_NOCTTY);
      /* printf("masterfd: %d\n", masterfd); */
      if (unlikely(masterfd == -1 || grantpt(masterfd) == -1 ||
                   unlockpt(masterfd) == -1 ||
                   (slavedevice = ptsname(masterfd)) == NULL)) {
        logError(printf("pcsPty: Cannot open pty.\n"););
        freeArgVector(argv);
        FREE_RECORD(childStdinFile, fileRecord, count.files);
        FREE_RECORD(childStdoutFile, fileRecord, count.files);
        raise_error(FILE_ERROR);
      } else {
        /* printf("slave device is: %s\n", slavedevice); */
        slavefd = open(slavedevice, O_RDWR|O_NOCTTY);
        if (unlikely(slavefd < 0)) {
          logError(printf("pcsPty: No slavefd\n"););
          freeArgVector(argv);
          FREE_RECORD(childStdinFile, fileRecord, count.files);
          FREE_RECORD(childStdoutFile, fileRecord, count.files);
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
            close(masterfd); /* Not required for the child */
            close(slavefd);
            execv(argv[0], argv);
            logError(printf("pcsPty: execv(" FMT_S_OS ") failed:\nerrno=%d\nerror: %s\n",
                            argv[0], errno, strerror(errno)););
          } else if (unlikely(pid == (pid_t) -1)) {
            logError(printf("pcsPty: fork failed:\nerrno=%d\nerror: %s\n",
                            errno, strerror(errno)););
            close(0); /* Restore the original std fds of parent */
            close(1);
            dup2(savedStdin, 0);
            dup2(savedStdout, 1);
            close(slavefd); /* This is unused */
            freeArgVector(argv);
            FREE_RECORD(childStdinFile, fileRecord, count.files);
            FREE_RECORD(childStdoutFile, fileRecord, count.files);
            raise_error(FILE_ERROR);
          } else {
            close(0); /* Restore the original std fds of parent */
            close(1);
            dup2(savedStdin, 0);
            dup2(savedStdout, 1);
            close(slavefd); /* This is being used by the child */
            initFileType(childStdinFile, FALSE, TRUE);
            childStdinFile->cFile = os_fdopen(masterfd, "w");
            if (childStdinFile->cFile == NULL) {
              FREE_RECORD(childStdinFile, fileRecord, count.files);
            } else {
              filDestr(*childStdin);
              *childStdin = childStdinFile;
            } /* if */
            initFileType(childStdoutFile, TRUE, FALSE);
            childStdoutFile->cFile = os_fdopen(masterfd, "r");
            if (childStdoutFile->cFile == NULL) {
              FREE_RECORD(childStdoutFile, fileRecord, count.files);
            } else {
              *childStdout = childStdoutFile;
              filDestr(*childStdout);
            } /* if */
            freeArgVector(argv);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("pcsPty -->\n"););
  } /* pcsPty */
#endif



processType pcsStart (const const_striType command, const const_rtlArrayType parameters,
    fileType redirectStdin, fileType redirectStdout, fileType redirectStderr)

  {
    cFileType childStdin;
    cFileType childStdout;
    cFileType childStderr;
    os_striType *argv;
    int stdinFileNo;
    int stdoutFileNo;
    int stderrFileNo;
    errInfoType err_info = OKAY_NO_ERROR;
    pid_t pid;
    unx_processType process;

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
    if (childStdin == NULL) {
      stdinFileNo = open(NULL_DEVICE, O_RDONLY);
    } else {
      stdinFileNo = os_fileno(childStdin);
    } /* if */
    if (childStdout == NULL) {
      stdoutFileNo = open(NULL_DEVICE, O_WRONLY);
    } else {
      stdoutFileNo = os_fileno(childStdout);
    } /* if */
    if (childStderr == NULL) {
      stderrFileNo = open(NULL_DEVICE, O_WRONLY);
    } else {
      stderrFileNo = os_fileno(childStderr);
    } /* if */
    if (unlikely(stdinFileNo == -1 || stdoutFileNo == -1 || stderrFileNo == -1)) {
      logError(printf("pcsStart: os_fileno() or open() failed.\n"););
      err_info = FILE_ERROR;
      process = NULL;
    } else {
      argv = genArgVector(command, parameters, &err_info);
      if (unlikely(argv == NULL)) {
        process = NULL;
      } else if (unlikely(access(argv[0], X_OK) != 0)) {
        logError(printf("pcsStart: access(" FMT_S_OS ", X_OK) not granted:\n"
                        "errno=%d\nerror: %s\n",
                        argv[0], errno, strerror(errno)););
        freeArgVector(argv);
        err_info = FILE_ERROR;
        process = NULL;
      } else if (unlikely(!ALLOC_RECORD(process, unx_processRecord, count.process))) {
        freeArgVector(argv);
        err_info = MEMORY_ERROR;
      } else {
        pid = fork();
        if (pid == 0) {
          if (stdinFileNo != 0) {
            dup2(stdinFileNo, 0);
            close(stdinFileNo);
          } /* if */
          if (stdoutFileNo != 1) {
            dup2(stdoutFileNo, 1);
            close(stdoutFileNo);
          } /* if */
          if (stderrFileNo != 2) {
            dup2(stderrFileNo, 2);
            close(stderrFileNo);
          } /* if */
          execv(argv[0], argv);
          logError(printf("pcsStart: execv(" FMT_S_OS ") failed:\n"
                          "errno=%d\nerror: %s\n",
                          argv[0], errno, strerror(errno)););
          os_exit(1);
        } else if (unlikely(pid == (pid_t) -1)) {
          logError(printf("pcsStart: fork failed:\nerrno=%d\nerror: %s\n",
                          errno, strerror(errno)););
          freeArgVector(argv);
          FREE_RECORD(process, unx_processRecord, count.process);
          err_info = FILE_ERROR;
          process = NULL;
        } else {
          freeArgVector(argv);
          memset(process, 0, sizeof(unx_processRecord));
          process->usage_count = 1;
          process->pid = pid;
          process->isTerminated = FALSE;
        } /* if */
      } /* if */
    } /* if */
    if (childStdin == NULL && stdinFileNo != -1) {
      close(stdinFileNo);
    } /* if */
    if (childStdout == NULL && stdoutFileNo != -1) {
      close(stdoutFileNo);
    } /* if */
    if (childStderr == NULL && stderrFileNo != -1) {
      close(stderrFileNo);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("pcsStart --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       process != NULL ? (memSizeType) process->pid : (memSizeType) 0,
                       process != NULL ? process->usage_count : (uintType) 0););
    return (processType) process;
  } /* pcsStart */



processType pcsStartPipe (const const_striType command, const const_rtlArrayType parameters)

  {
    os_striType *argv;
    fileType childStdinFile;
    fileType childStdoutFile;
    fileType childStderrFile;
    int childStdinPipes[2];
    int childStdoutPipes[2];
    int childStderrPipes[2];
    errInfoType err_info = OKAY_NO_ERROR;
    pid_t pid;
    unx_processType process;

  /* pcsStartPipe */
    logFunction(printf("pcsStartPipe(\"%s\"", striAsUnquotedCStri(command));
                printParameters(parameters);
                printf(")\n"););
    argv = genArgVector(command, parameters, &err_info);
    if (unlikely(argv == NULL)) {
      raise_error(err_info);
      process = NULL;
    } else if (unlikely(access(argv[0], X_OK) != 0)) {
      logError(printf("pcsStartPipe: No execute permission for " FMT_S_OS "\n", argv[0]););
      freeArgVector(argv);
      raise_error(FILE_ERROR);
      process = NULL;
    } else if (unlikely(!ALLOC_RECORD(process, unx_processRecord, count.process))) {
      freeArgVector(argv);
      raise_error(MEMORY_ERROR);
    } else if (unlikely(!ALLOC_RECORD(childStdinFile, fileRecord, count.files))) {
      freeArgVector(argv);
      FREE_RECORD(process, unx_processRecord, count.process);
      raise_error(MEMORY_ERROR);
    } else if (unlikely(!ALLOC_RECORD(childStdoutFile, fileRecord, count.files))) {
      freeArgVector(argv);
      FREE_RECORD(childStdinFile, fileRecord, count.files);
      FREE_RECORD(process, unx_processRecord, count.process);
      raise_error(MEMORY_ERROR);
    } else if (unlikely(!ALLOC_RECORD(childStderrFile, fileRecord, count.files))) {
      freeArgVector(argv);
      FREE_RECORD(childStdinFile, fileRecord, count.files);
      FREE_RECORD(childStdoutFile, fileRecord, count.files);
      FREE_RECORD(process, unx_processRecord, count.process);
      raise_error(MEMORY_ERROR);
    } else if (unlikely(pipe(childStdinPipes) != 0)) {
      logError(printf("pcsStartPipe: pipe(childStdinPipes) failed:\n"
                      "errno=%d\nerror: %s\n",
                      errno, strerror(errno)););
      freeArgVector(argv);
      FREE_RECORD(childStdinFile, fileRecord, count.files);
      FREE_RECORD(childStdoutFile, fileRecord, count.files);
      FREE_RECORD(childStderrFile, fileRecord, count.files);
      FREE_RECORD(process, unx_processRecord, count.process);
      raise_error(FILE_ERROR);
      process = NULL;
    } else if (unlikely(pipe(childStdoutPipes) != 0)) {
      logError(printf("pcsStartPipe: pipe(childStdoutPipes) failed:\n"
                      "errno=%d\nerror: %s\n",
                      errno, strerror(errno)););
      freeArgVector(argv);
      FREE_RECORD(childStdinFile, fileRecord, count.files);
      FREE_RECORD(childStdoutFile, fileRecord, count.files);
      FREE_RECORD(childStderrFile, fileRecord, count.files);
      FREE_RECORD(process, unx_processRecord, count.process);
      close(childStdinPipes[0]);
      close(childStdinPipes[1]);
      raise_error(FILE_ERROR);
      process = NULL;
    } else if (unlikely(pipe(childStderrPipes) != 0)) {
      logError(printf("pcsStartPipe: pipe(childStderrPipes) failed:\n"
                      "errno=%d\nerror: %s\n",
                      errno, strerror(errno)););
      freeArgVector(argv);
      FREE_RECORD(childStdinFile, fileRecord, count.files);
      FREE_RECORD(childStdoutFile, fileRecord, count.files);
      FREE_RECORD(childStderrFile, fileRecord, count.files);
      FREE_RECORD(process, unx_processRecord, count.process);
      close(childStdinPipes[0]);
      close(childStdinPipes[1]);
      close(childStdoutPipes[0]);
      close(childStdoutPipes[1]);
      raise_error(FILE_ERROR);
      process = NULL;
    } else {
      pid = fork();
      if (pid == 0) {
        dup2(childStdinPipes[0], 0); /* Make the read end of childStdinPipes pipe as stdin */
        dup2(childStdoutPipes[1], 1);  /* Make the write end of childStdoutPipes as stdout */
        dup2(childStderrPipes[1], 2);  /* Make the write end of childStderrPipes as stderr */
        close(childStdinPipes[0]); /* Not required for the child */
        close(childStdinPipes[1]);
        close(childStdoutPipes[0]);
        close(childStdoutPipes[1]);
        close(childStderrPipes[0]);
        close(childStderrPipes[1]);
        execv(argv[0], argv);
        logError(printf("pcsStartPipe: execv(" FMT_S_OS ") failed:\nerrno=%d\nerror: %s\n",
                        argv[0], errno, strerror(errno)););
        /* printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
            EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS);
        printf("EFAULT=%d  EISDIR=%d  ENAMETOOLONG=%d  ENODEV=%d  EINVAL=%d\n",
            EFAULT, EISDIR, ENAMETOOLONG, ENODEV, EINVAL); */
      } else if (unlikely(pid == (pid_t) -1)) {
        logError(printf("pcsStartPipe: fork failed:\nerrno=%d\nerror: %s\n",
                        errno, strerror(errno)););
        close(childStdinPipes[0]); /* The pipes are unused */
        close(childStdoutPipes[1]);
        close(childStderrPipes[1]);
        close(childStdinPipes[1]);
        close(childStdoutPipes[0]);
        close(childStderrPipes[0]);
        freeArgVector(argv);
        FREE_RECORD(childStdinFile, fileRecord, count.files);
        FREE_RECORD(childStdoutFile, fileRecord, count.files);
        FREE_RECORD(childStderrFile, fileRecord, count.files);
        FREE_RECORD(process, unx_processRecord, count.process);
        raise_error(FILE_ERROR);
        process = NULL;
      } else {
        close(childStdinPipes[0]); /* These are being used by the child */
        close(childStdoutPipes[1]);
        close(childStderrPipes[1]);
        freeArgVector(argv);
        memset(process, 0, sizeof(unx_processRecord));
        process->usage_count = 1;
        process->pid = pid;
        process->isTerminated = FALSE;
        initFileType(childStdinFile, FALSE, TRUE);
        childStdinFile->cFile  = os_fdopen(childStdinPipes[1], "w");
        process->stdIn = childStdinFile;
        initFileType(childStdoutFile, TRUE, FALSE);
        childStdoutFile->cFile = os_fdopen(childStdoutPipes[0], "r");
        process->stdOut = childStdoutFile;
        initFileType(childStderrFile, TRUE, FALSE);
        childStderrFile->cFile = os_fdopen(childStderrPipes[0], "r");
        process->stdErr = childStderrFile;
      } /* if */
    } /* if */
    logFunction(printf("pcsStartPipe --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       process != NULL ? (memSizeType) process->pid : (memSizeType) 0,
                       process != NULL ? process->usage_count : (uintType) 0););
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
    int waitpid_result;
    int status;

  /* pcsWaitFor */
    logFunction(printf("pcsWaitFor(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       process != NULL ? (memSizeType) to_pid(process) : (memSizeType) 0,
                       process != NULL ? process->usage_count : (uintType) 0););
    if (unlikely(process == NULL)) {
      logError(printf("pcsWaitFor: process == NULL\n"););
      raise_error(FILE_ERROR);
    } else if (!to_isTerminated(process)) {
      status = 0;
      waitpid_result = waitpid(to_pid(process), &status, 0);
      if (waitpid_result == to_pid(process)) {
        to_var_isTerminated(process) = TRUE;
        if (WIFEXITED(status)) {
          to_var_exitValue(process) = WEXITSTATUS(status);
        } else {
          to_var_exitValue(process) = 0;
        } /* if */
      } else {
        logError(printf("pcsWaitFor: waitpid(" FMT_U_MEM ", *, 0) failed:\n"
                        "errno=%d\nerror: %s\n",
                        (memSizeType) to_pid(process), errno, strerror(errno)););
        raise_error(FILE_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("pcsWaitFor -->\n"););
  } /* pcsWaitFor */
