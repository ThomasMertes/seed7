/********************************************************************/
/*                                                                  */
/*  pcs_rtl.c     Platform independent process handling functions.  */
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
/*  File: seed7/src/pcs_rtl.c                                       */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: Platform independent process handling functions.       */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdio.h"

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "fil_rtl.h"
#include "rtl_err.h"
#include "pcs_drv.h"



/**
 *  Returns the error output file (stderr) of the given child process.
 *  If the standard error file of the subprocess has been redirected
 *  then this function will return NULL.
 *  @return the error output file of 'process' or
 *          CLIB_NULL_FILE, if stderr has been redirected.
 */
fileType pcsChildStdErr (const const_processType process)

  {
    fileType stdErr;

  /* pcsChildStdErr */
    logFunction(printf("pcsChildStdErr(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) process,
                       process != NULL ? process->usage_count : (uintType) 0););
    if (unlikely(process == NULL)) {
      logError(printf("pcsChildStdErr: process == NULL\n"););
      raise_error(FILE_ERROR);
      stdErr = NULL;
    } else if (process->stdErr == NULL) {
      stdErr = &nullFileRecord;
    } else {
      stdErr = process->stdErr;
      stdErr->usage_count++;
    } /* if */
    logFunction(printf("pcsChildStdErr(" FMT_U_MEM " (usage=" FMT_U "))"
                       " --> " FMT_U_MEM " %s%d (usage=" FMT_U ")\n",
                       (memSizeType) process,
                       process != NULL ? process->usage_count : (uintType) 0,
                       (memSizeType) stdErr,
                       stdErr == NULL ? "NULL " : "",
                       stdErr != NULL ? safe_fileno(stdErr->cFile) : 0,
                       stdErr != NULL ? stdErr->usage_count : (uintType) 0););
    return stdErr;
  } /* pcsChildStdErr */



/**
 *  Returns the standard input file (stdin) of the given child process.
 *  If the standard input file of the subprocess has been redirected
 *  then this function will return NULL.
 *  @return the standard input file of 'process' or
 *          CLIB_NULL_FILE, if stdin has been redirected.
 */
fileType pcsChildStdIn (const const_processType process)

  {
    fileType stdIn;

  /* pcsChildStdIn */
    logFunction(printf("pcsChildStdIn(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) process,
                       process != NULL ? process->usage_count : (uintType) 0););
    if (unlikely(process == NULL)) {
      logError(printf("pcsChildStdIn: process == NULL\n"););
      raise_error(FILE_ERROR);
      stdIn = NULL;
    } else if (process->stdIn == NULL) {
      stdIn = &nullFileRecord;
    } else {
      stdIn = process->stdIn;
      stdIn->usage_count++;
    } /* if */
    logFunction(printf("pcsChildStdIn(" FMT_U_MEM " (usage=" FMT_U "))"
                       " --> " FMT_U_MEM " %s%d (usage=" FMT_U ")\n",
                       (memSizeType) process,
                       process != NULL ? process->usage_count : (uintType) 0,
                       (memSizeType) stdIn,
                       stdIn == NULL ? "NULL " : "",
                       stdIn != NULL ? safe_fileno(stdIn->cFile) : 0,
                       stdIn != NULL ? stdIn->usage_count : (uintType) 0););
    return stdIn;
  } /* pcsChildStdIn */



/**
 *  Returns the standard output file (stdout) of the given child process.
 *  If the standard output file of the subprocess has been redirected
 *  then this function will return NULL.
 *  @return the standard output file of 'process' or
 *          CLIB_NULL_FILE, if stdout has been redirected.
 */
fileType pcsChildStdOut (const const_processType process)

  {
    fileType stdOut;

  /* pcsChildStdOut */
    logFunction(printf("pcsChildStdOut(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) process,
                       process != NULL ? process->usage_count : (uintType) 0););
    if (unlikely(process == NULL)) {
      logError(printf("pcsChildStdOut: process == NULL\n"););
      raise_error(FILE_ERROR);
      stdOut = NULL;
    } else if (process->stdOut == NULL) {
      stdOut = &nullFileRecord;
    } else {
      stdOut = process->stdOut;
      stdOut->usage_count++;
    } /* if */
    logFunction(printf("pcsChildStdOut(" FMT_U_MEM " (usage=" FMT_U "))"
                       " --> " FMT_U_MEM " %s%d (usage=" FMT_U ")\n",
                       (memSizeType) process,
                       process != NULL ? process->usage_count : (uintType) 0,
                       (memSizeType) stdOut,
                       stdOut == NULL ? "NULL " : "",
                       stdOut != NULL ? safe_fileno(stdOut->cFile) : 0,
                       stdOut != NULL ? stdOut->usage_count : (uintType) 0););
    return stdOut;
  } /* pcsChildStdOut */



/**
 *  Reinterpret the generic parameters as processType and call pcsCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(processType).
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType pcsCmpGeneric (const genericType value1, const genericType value2)

  { /* pcsCmpGeneric */
    return pcsCmp(((const_rtlObjectType *) &value1)->value.processValue,
                  ((const_rtlObjectType *) &value2)->value.processValue);
  } /* pcsCmpGeneric */



/**
 *  Assign source to *dest.
 *  A copy function assumes that *dest contains a legal value.
 */
void pcsCpy (processType *const dest, const processType source)

  { /* pcsCpy */
    logFunction(printf("pcsCpy(" FMT_U_MEM " (usage=" FMT_U "), "
                       FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) *dest,
                       *dest != NULL ? (*dest)->usage_count : (uintType) 0,
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    if (source != NULL) {
      source->usage_count++;
    } /* if */
    if (*dest != NULL) {
      (*dest)->usage_count--;
      if ((*dest)->usage_count == 0) {
        pcsFree(*dest);
      } /* if */
    } /* if */
    *dest = source;
    logFunction(printf("pcsCpy(" FMT_U_MEM " (usage=" FMT_U "), "
                       FMT_U_MEM " (usage=" FMT_U ")) -->\n",
                       (memSizeType) *dest,
                       *dest != NULL ? (*dest)->usage_count : (uintType) 0,
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
  } /* pcsCpy */



/**
 *  Reinterpret the generic parameters as processType and call pcsCpy.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(processType).
 */
void pcsCpyGeneric (genericType *const dest, const genericType source)

  { /* pcsCpyGeneric */
    pcsCpy(&((rtlObjectType *) dest)->value.processValue,
           ((const_rtlObjectType *) &source)->value.processValue);
  } /* pcsCpyGeneric */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 */
processType pcsCreate (const processType source)

  { /* pcsCreate */
    logFunction(printf("pcsCreate(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    if (source != NULL) {
      source->usage_count++;
    } /* if */
    logFunction(printf("pcsCreate --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    return source;
  } /* pcsCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(processType).
 */
genericType pcsCreateGeneric (const genericType from_value)

  {
    rtlObjectType result;

  /* pcsCreateGeneric */
    INIT_GENERIC_PTR(result.value.genericValue);
    result.value.processValue =
        pcsCreate(((const_rtlObjectType *) &from_value)->value.processValue);
    return result.value.genericValue;
  } /* pcsCreateGeneric */



/**
 *  Free the memory referred by 'oldProcess'.
 *  After pcsDestr is left 'oldProcess' refers to not existing memory.
 *  The memory where 'oldProcess' is stored can be freed afterwards.
 */
void pcsDestr (const processType oldProcess)

  { /* pcsDestr */
    logFunction(printf("pcsDestr(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) oldProcess,
                       oldProcess != NULL ? oldProcess->usage_count : (uintType) 0););
    if (oldProcess != NULL) {
      oldProcess->usage_count--;
      if (oldProcess->usage_count == 0) {
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
        pcsFree(oldProcess);
      } /* if */
    } /* if */
  } /* pcsDestr */



/**
 *  Generic Destr function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(processType).
 */
void pcsDestrGeneric (const genericType old_value)

  { /* pcsDestrGeneric */
    pcsDestr(((const_rtlObjectType *) &old_value)->value.processValue);
  } /* pcsDestrGeneric */



/**
 *  Generic hashCode function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(processType).
 */
intType pcsHashCodeGeneric (const genericType aValue)

  { /* pcsHashCodeGeneric */
    return pcsHashCode(((const_rtlObjectType *) &aValue)->value.processValue);
  } /* pcsHashCodeGeneric */
