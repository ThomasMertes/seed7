/********************************************************************/
/*                                                                  */
/*  db_lite.h     SQLite interfaces used by Seed7.                  */
/*  Copyright (C) 1989 - 2020  Thomas Mertes                        */
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
/*  File: seed7/src/db_lite.h                                       */
/*  Changes: 2014, 2015, 2019, 2020  Thomas Mertes                  */
/*  Content: SQLite interfaces used by Seed7.                       */
/*                                                                  */
/********************************************************************/

typedef void *sqlite3;
typedef void *sqlite3_stmt;

typedef INT64TYPE sqlite3_int64;

#define SQLITE_INTEGER  1
#define SQLITE_FLOAT    2
#define SQLITE_TEXT     3
#define SQLITE_BLOB     4
#define SQLITE_NULL     5

#define SQLITE_OK       0
#define SQLITE_ROW    100
#define SQLITE_DONE   101

#ifndef CDECL
#if defined(_WIN32) && HAS_CDECL
#define CDECL __cdecl
#else
#define CDECL
#endif
#endif

int CDECL sqlite3_bind_blob (sqlite3_stmt *pStmt,
                             int index,
                             const void *value,
                             int n,
                             void (*destruct) (void*));
int CDECL sqlite3_bind_double (sqlite3_stmt *pStmt, int index, double value);
int CDECL sqlite3_bind_int (sqlite3_stmt *pStmt, int index, int value);
int CDECL sqlite3_bind_int64 (sqlite3_stmt *pStmt, int index, sqlite3_int64 value);
int CDECL sqlite3_bind_null (sqlite3_stmt *pStmt, int index);
int CDECL sqlite3_bind_parameter_count(sqlite3_stmt *pStmt);
int CDECL sqlite3_bind_text (sqlite3_stmt *pStmt,
                             int index,
                             const char *value,
                             int n,
                             void (*destruct) (void*));
int CDECL sqlite3_close (sqlite3 *db);
const void *CDECL sqlite3_column_blob (sqlite3_stmt *pStmt, int iCol);
int CDECL sqlite3_column_bytes (sqlite3_stmt *pStmt, int iCol);
int CDECL sqlite3_column_count (sqlite3_stmt *pStmt);
double CDECL sqlite3_column_double (sqlite3_stmt *pStmt, int iCol);
int CDECL sqlite3_column_int (sqlite3_stmt *pStmt, int iCol);
sqlite3_int64 CDECL sqlite3_column_int64 (sqlite3_stmt *pStmt, int iCol);
const char *CDECL sqlite3_column_name (sqlite3_stmt *pStmt, int N);
const unsigned char *CDECL sqlite3_column_text (sqlite3_stmt *pStmt, int iCol);
int CDECL sqlite3_column_type (sqlite3_stmt *pStmt, int iCol);
sqlite3 *CDECL sqlite3_db_handle (sqlite3_stmt *pStmt);
int CDECL sqlite3_errcode (sqlite3 *db);
const char *CDECL sqlite3_errmsg (sqlite3 *db);
int CDECL sqlite3_exec (sqlite3 *db,
                        const char *sql,
                        int (*callback) (void*, int, char**, char**),
                        void *arg1OfCallback,
                        char **errMsg);
int CDECL sqlite3_finalize (sqlite3_stmt *pStmt);
int CDECL sqlite3_get_autocommit (sqlite3 *db);
int CDECL sqlite3_open (const char *filename, sqlite3 **ppDb);
int CDECL sqlite3_prepare (sqlite3 *db,
                           const char *sql,
                           int nByte,
                           sqlite3_stmt **ppStmt,
                           const char **pzTail);
int CDECL sqlite3_reset (sqlite3_stmt *pStmt);
int CDECL sqlite3_step (sqlite3_stmt *pStmt);
