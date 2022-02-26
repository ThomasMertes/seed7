/********************************************************************/
/*                                                                  */
/*  sql_base.c    Basic database functions.                         */
/*  Copyright (C) 2017  Thomas Mertes                               */
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
/*  File: seed7/src/sql_base.c                                      */
/*  Changes: 2017  Thomas Mertes                                    */
/*  Content: Basic database functions.                              */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

#include "common.h"
#include "heaputl.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "sql_base.h"



void dbLibError (const char *funcName, const char *dbFuncName,
    const char *format, ...)

  {
    va_list ap;

  /* dbLibError */
    dbError.funcName = funcName;
    dbError.dbFuncName = dbFuncName;
    dbError.errorCode = 0;
    va_start(ap, format);
    vsnprintf(dbError.message, DB_ERR_MESSAGE_SIZE, format, ap);
    va_end(ap);
  } /* dbLibError */



void dbInconsistentMsg (const char *funcName, const char *dbFuncName,
    const char *file, int line)

  { /* dbInconsistentMsg */
    dbLibError(funcName, dbFuncName, "Db interface inconsistent: %s(%d)",
               file, line);
  } /* dbInconsistentMsg */



void dllErrorMessage (const char *funcName, const char *dbFuncName,
    const char *dllList[], memSizeType dllListLength)

  {
    unsigned int pos;
    memSizeType dllNamesSize = 0;
    char *dllNames;
    char *currPos;

  /* dllErrorMessage */
    for (pos = 0; pos < dllListLength; pos++) {
      dllNamesSize += strlen(dllList[pos]) + 2; /* 2 chars for comma and space */
    } /* for */
    if (ALLOC_CSTRI(dllNames, dllNamesSize)) {
      currPos = dllNames;
      currPos[0] = '\0';
      for (pos = 0; pos < dllListLength; pos++) {
        currPos += sprintf(currPos, "%s, ", dllList[pos]);
      } /* for */
      if (currPos != dllNames) {
        currPos[-2] = '\0';
      } /* if */
      dbLibError(funcName, dbFuncName,
                 "Searching dynamic libraries failed: %s\n", dllNames);
      logError(printf("%s: Searching dynamic libraries failed: %s\n",
                      dbFuncName, dllNames););
      UNALLOC_CSTRI(dllNames, dllNamesSize);
    } /* if */
  } /* dllErrorMessage */
