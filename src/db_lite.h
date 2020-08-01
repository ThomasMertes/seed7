/********************************************************************/
/*                                                                  */
/*  db_lite.h     SQLite interfaces used by Seed7.                  */
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
/*  File: seed7/src/db_lite.h                                       */
/*  Changes: 2014  Thomas Mertes                                    */
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

#if defined(__WIN__)
#define SQLITE_API __stdcall
#else
#define SQLITE_API
#endif

SQLITE_API int sqlite3_bind_blob (sqlite3_stmt *pStmt,
                                  int index,
                                  const void *value,
                                  int n,
                                  void (*destruct) (void*));
SQLITE_API int sqlite3_bind_double (sqlite3_stmt *pStmt, int index, double value);
SQLITE_API int sqlite3_bind_int (sqlite3_stmt *pStmt, int index, int value);
SQLITE_API int sqlite3_bind_int64 (sqlite3_stmt *pStmt, int index, sqlite3_int64 value);
SQLITE_API int sqlite3_bind_null (sqlite3_stmt *pStmt, int index);
SQLITE_API int sqlite3_bind_parameter_count(sqlite3_stmt *pStmt);
SQLITE_API int sqlite3_bind_text (sqlite3_stmt *pStmt,
                                  int index,
                                  const char *value,
                                  int n,
                                  void (*destruct) (void*));
SQLITE_API int sqlite3_close (sqlite3 *db);
SQLITE_API const void *sqlite3_column_blob (sqlite3_stmt *pStmt, int iCol);
SQLITE_API int sqlite3_column_bytes (sqlite3_stmt *pStmt, int iCol);
SQLITE_API int sqlite3_column_count (sqlite3_stmt *pStmt);
SQLITE_API double sqlite3_column_double (sqlite3_stmt *pStmt, int iCol);
SQLITE_API int sqlite3_column_int (sqlite3_stmt *pStmt, int iCol);
SQLITE_API sqlite3_int64 sqlite3_column_int64 (sqlite3_stmt *pStmt, int iCol);
SQLITE_API const char *sqlite3_column_name (sqlite3_stmt *pStmt, int N);
SQLITE_API const unsigned char *sqlite3_column_text (sqlite3_stmt *pStmt, int iCol);
SQLITE_API int sqlite3_column_type (sqlite3_stmt *pStmt, int iCol);
SQLITE_API sqlite3 *sqlite3_db_handle (sqlite3_stmt *pStmt);
SQLITE_API int sqlite3_errcode (sqlite3 *db);
SQLITE_API const char *sqlite3_errmsg (sqlite3 *db);
SQLITE_API int sqlite3_finalize (sqlite3_stmt *pStmt);
SQLITE_API int sqlite3_open (const char *filename, sqlite3 **ppDb);
SQLITE_API int sqlite3_prepare (sqlite3 *db,
                                const char *zSql,
                                int nByte,
                                sqlite3_stmt **ppStmt,
                                const char **pzTail);
SQLITE_API int sqlite3_reset (sqlite3_stmt *pStmt);
SQLITE_API int sqlite3_step (sqlite3_stmt *pStmt);
