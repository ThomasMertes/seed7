/********************************************************************/
/*                                                                  */
/*  pcs_dos.c     Process functions using the dos capabilities.     */
/*  Copyright (C) 1989 - 2018  Thomas Mertes                        */
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
/*  File: seed7/src/pcs_dos.c                                       */
/*  Changes: 2018  Thomas Mertes                                    */
/*  Content: Process functions using the dos capabilities.          */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "errno.h"

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "int_rtl.h"
#include "cmd_rtl.h"
#include "rtl_err.h"


#if DO_HEAP_STATISTIC
size_t sizeof_processRecord = 0;
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
    } else if ((memSizeType) process1 < (memSizeType) process2) {
      signumValue = -1;
    } else {
      signumValue = (memSizeType) process1 > (memSizeType) process2;
    } /* if */
    return signumValue;
  } /* pcsCmp */



boolType pcsEq (const const_processType process1, const const_processType process2)

  { /* pcsEq */
    if (process1 == NULL) {
      return process2 == NULL;
    } else if (process2 == NULL) {
      return FALSE;
    } else {
      return (memSizeType) process1 == (memSizeType) process2;
    } /* if */
  } /* pcsEq */



intType pcsExitValue (const const_processType process)

  { /* pcsExitValue */
    return 0;
  } /* pcsExitValue */



void pcsFree (processType oldProcess)

  { /* pcsFree */
  } /* pcsFree */



intType pcsHashCode (const const_processType process)

  { /* pcsHashCode */
    return 0;
  } /* pcsHashCode */



boolType pcsIsAlive (const processType process)

  { /* pcsIsAlive */
    return FALSE;
  } /* pcsIsAlive */



void pcsKill (const processType process)

  { /* pcsKill */
  } /* pcsKill */



void pcsPipe2 (const const_striType command, const const_rtlArrayType parameters,
    fileType *childStdin, fileType *childStdout)

  { /* pcsPipe2 */
  } /* pcsPipe2 */



void pcsPty (const const_striType command, const const_rtlArrayType parameters,
    fileType *childStdin, fileType *childStdout)

  { /* pcsPty */
  } /* pcsPty */



static void tempName (char *temp_name)

  {
    uintType random_value;
    memSizeType pos = 0;
    unsigned int digit;

  /* tempName */
    random_value = uintRand();
    for (; pos < 8; pos++) {
      digit = (unsigned int) (random_value % 36);
      random_value /= 36;
      temp_name[pos] = (os_charType) "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[digit];
    } /* for */
    memcpy(&temp_name[pos], ".tmp", 5);
  } /* tempName */



processType pcsStart (const const_striType command, const const_rtlArrayType parameters,
    fileType redirectStdin, fileType redirectStdout, fileType redirectStderr)

  {
    char osRedirectStdinName[12 + NULL_TERMINATION_LEN];
    char osRedirectStdoutName[12 + NULL_TERMINATION_LEN];
    char osRedirectStderrName[12 + NULL_TERMINATION_LEN];
    striType redirectStdinName;
    striType redirectStdoutName;
    striType redirectStderrName;
    FILE *stdinFile;
    FILE *stdoutFile;
    FILE *stderrFile;
    intType returnCode;
    errInfoType err_info = OKAY_NO_ERROR;
    size_t bytes_read;
    char buffer[4096];

  /* pcsStart */
    logFunction(printf("pcsStart(\"%s\"", striAsUnquotedCStri(command));
#if ANY_LOG_ACTIVE
                printParameters(parameters);
#else
                printf(", **not shown **");
#endif
                printf(", %s%d, %s%d, %s%d)\n",
                       redirectStdin == NULL ? "NULL " : "",
                       redirectStdin != NULL ? safe_fileno(redirectStdin->cFile) : 0,
                       redirectStdout == NULL ? "NULL " : "",
                       redirectStdout != NULL ? safe_fileno(redirectStdout->cFile) : 0,
                       redirectStderr == NULL ? "NULL " : "",
                       redirectStderr != NULL ? safe_fileno(redirectStderr->cFile) : 0););
    if (redirectStdin->cFile == NULL) {
      redirectStdinName = cstri_to_stri("");
    } else {
      tempName(osRedirectStdinName);
      /* printf("redirectStdin\n"); */
      stdinFile = os_fopen(osRedirectStdinName, "w");
      if (unlikely(stdinFile == NULL)) {
        logError(printf("pcsStart: fopen(\"%s\", \"w\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        osRedirectStdinName,
                        errno, strerror(errno)););
        err_info = FILE_ERROR;
        redirectStdinName = NULL;
      } else {
        /* printf("opening ok\n"); */
        while (err_info == OKAY_NO_ERROR && (bytes_read =
                fread(buffer, 1, 4096, redirectStdin->cFile)) != 0) {
          /* printf("read " FMT_U_MEM "\n", (memSizeType) bytes_read); */
          if (fwrite(buffer, 1, bytes_read, stdinFile) != bytes_read) {
            logError(printf("pcsStart: copy_file(\"" FMT_S_OS "\", %d): "
                            "fwrite(*, 1, " FMT_U_MEM ", %d) failed:\n"
                            "errno=%d\nerror: %s\n",
                            osRedirectStdinName,
                            safe_fileno(redirectStdin->cFile),
                            (memSizeType) bytes_read,
                            safe_fileno(redirectStdin->cFile),
                            errno, strerror(errno)););
            err_info = FILE_ERROR;
          } /* if */
        } /* while */
        fclose(stdinFile);
        redirectStdinName = cstri_to_stri(osRedirectStdinName);
      } /* if */
    } /* if */
    if (redirectStdout->cFile == NULL) {
      redirectStdoutName = cstri_to_stri("");
    } else {
      tempName(osRedirectStdoutName);
      redirectStdoutName = cstri_to_stri(osRedirectStdoutName);
    } /* if */
    if (redirectStderr->cFile == NULL) {
      redirectStderrName = cstri_to_stri("");
    } else {
      tempName(osRedirectStderrName);
      redirectStderrName = cstri_to_stri(osRedirectStderrName);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR ||
                 redirectStdinName == NULL ||
                 redirectStdoutName == NULL ||
                 redirectStderrName == NULL)) {
      if (redirectStdin->cFile != NULL) {
        os_remove(osRedirectStdinName);
      } /* if */
      if (err_info == OKAY_NO_ERROR) {
        err_info = MEMORY_ERROR;
      } /* if */
    } else {
      returnCode = cmdShellExecute(command, parameters, redirectStdinName,
                                   redirectStdoutName, redirectStderrName);
      if (redirectStdin->cFile != NULL) {
        os_remove(osRedirectStdinName);
      } /* if */
      if (redirectStdout->cFile != NULL) {
        /* printf("redirectStdout\n"); */
        stdoutFile = os_fopen(osRedirectStdoutName, "r");
        if (stdoutFile != NULL) {
          /* printf("opening ok\n"); */
          while (err_info == OKAY_NO_ERROR && (bytes_read =
                  fread(buffer, 1, 4096, stdoutFile)) != 0) {
          /* printf("read " FMT_U_MEM "\n", (memSizeType) bytes_read); */
            if (fwrite(buffer, 1, bytes_read, redirectStdout->cFile) != bytes_read) {
              logError(printf("pcsStart: copy_file(\"" FMT_S_OS "\", %d): "
                              "fwrite(*, 1, " FMT_U_MEM ", %d) failed:\n"
                              "errno=%d\nerror: %s\n",
                              osRedirectStdoutName,
                              safe_fileno(redirectStdout->cFile),
                            (memSizeType) bytes_read,
                              safe_fileno(redirectStdout->cFile), errno, strerror(errno)););
              err_info = FILE_ERROR;
            } /* if */
          } /* while */
          fclose(stdoutFile);
        } /* if */
        os_remove(osRedirectStdoutName);
      } /* if */
      if (redirectStderr->cFile != NULL) {
        /* printf("redirectStderr\n"); */
        stderrFile = os_fopen(osRedirectStderrName, "r");
        if (stderrFile != NULL) {
          /* printf("opening ok\n"); */
          while (err_info == OKAY_NO_ERROR && (bytes_read =
                  fread(buffer, 1, 4096, stderrFile)) != 0) {
          /* printf("read " FMT_U_MEM "\n", (memSizeType) bytes_read); */
            if (fwrite(buffer, 1, bytes_read, redirectStderr->cFile) != bytes_read) {
              logError(printf("pcsStart: copy_file(\"" FMT_S_OS "\", %d): "
                              "fwrite(*, 1, " FMT_U_MEM ", %d) failed:\n"
                              "errno=%d\nerror: %s\n",
                              osRedirectStderrName,
                              safe_fileno(redirectStderr->cFile),
                            (memSizeType) bytes_read,
                              safe_fileno(redirectStderr->cFile), errno, strerror(errno)););
              err_info = FILE_ERROR;
            } /* if */
          } /* while */
          fclose(stderrFile);
        } /* if */
        os_remove(osRedirectStderrName);
      } /* if */
    } /* if */
    if (redirectStdinName != NULL) {
      FREE_STRI(redirectStdinName);
    } /* if */
    if (redirectStdoutName != NULL) {
      FREE_STRI(redirectStdoutName);
    } /* if */
    if (redirectStderrName != NULL) {
      FREE_STRI(redirectStderrName);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("pcsStart -> NULL\n"););
    return NULL;
  } /* pcsStart */



processType pcsStartPipe (const const_striType command, const const_rtlArrayType parameters)

  { /* pcsStartPipe */
    logFunction(printf("pcsStartPipe(\"%s\"", striAsUnquotedCStri(command));
#if ANY_LOG_ACTIVE
                printParameters(parameters);
#else
                printf(", **not shown **");
#endif
                printf(")\n"););
    logFunction(printf("pcsStartPipe -> 0\n"););
    return NULL;
  } /* pcsStartPipe */



striType pcsStr (const const_processType process)

  { /* pcsStr */
    return NULL;
  } /* pcsStr */



void pcsWaitFor (const processType process)

  { /* pcsWaitFor */
  } /* pcsWaitFor */

