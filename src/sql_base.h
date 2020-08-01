/********************************************************************/
/*                                                                  */
/*  sql_base.h    Basic database functions.                         */
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
/*  File: seed7/src/sql_base.h                                      */
/*  Changes: 2017  Thomas Mertes                                    */
/*  Content: Basic database functions.                              */
/*                                                                  */
/********************************************************************/

#define DB_ERR_MESSAGE_SIZE 4096

typedef struct dbErrorStruct {
    const char *funcName;
    const char *dbFuncName;
    intType errorCode;
    char message[DB_ERR_MESSAGE_SIZE];
  } dbErrorRecord, *dbErrorType;

#ifdef DO_INIT
dbErrorRecord dbError = {"", "", 0, ""};
#else
EXTERN dbErrorRecord dbError;
#endif


#define dbInconsistent(funcName, dbFuncName) \
    dbInconsistentMsg(funcName, dbFuncName, __FILE__, __LINE__)


void dbLibError (const char *funcName, const char *dbFuncName,
                 const char *format, ...);
void dbInconsistentMsg (const char *funcName, const char *dbFuncName,
                        const char *file, int line);
void dllErrorMessage (const char *funcName, const char *dbFuncName,
                      const char *dllList[], memSizeType dllListLength);
