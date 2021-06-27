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

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "striutl.h"


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




intType pcsHashCode (const const_processType aProcess)

  { /* pcsHashCode */
    return 0;
  } /* pcsHashCode */




boolType pcsIsAlive (const processType aProcess)

  { /* pcsIsAlive */
    return FALSE;
  } /* pcsIsAlive */




void pcsKill (const processType aProcess)

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




processType pcsStart (const const_striType command, const const_rtlArrayType parameters,
    fileType redirectStdin, fileType redirectStdout, fileType redirectStderr)

  { /* pcsStart */
    logFunction(printf("pcsStart(\"%s\"", striAsUnquotedCStri(command));
                printParameters(parameters);
                printf(", %s%d, %s%d, %s%d)\n",
                       redirectStdin == NULL ? "NULL " : "",
                       redirectStdin != NULL ? safe_fileno(redirectStdin->cFile) : 0,
                       redirectStdout == NULL ? "NULL " : "",
                       redirectStdout != NULL ? safe_fileno(redirectStdout->cFile) : 0,
                       redirectStderr == NULL ? "NULL " : "",
                       redirectStderr != NULL ? safe_fileno(redirectStderr->cFile) : 0););
    logFunction(printf("pcsStart -> 0\n"););
    return NULL;
  } /* pcsStart */




striType pcsStr (const const_processType process)

  { /* pcsStr */
    return NULL;
  } /* pcsStr */




void pcsWaitFor (const processType process)

  { /* pcsWaitFor */
  } /* pcsWaitFor */

