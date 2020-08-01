/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2014  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/sqllib.c                                        */
/*  Changes: 2013, 2014  Thomas Mertes                              */
/*  Content: All primitive actions for database access.             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"

#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "objutl.h"
#include "runerr.h"
#include "sql_rtl.h"



objectType sql_bind_bigint (listType arguments)

  { /* sql_bind_bigint */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_bigint(arg_3(arguments));
    sqlBindBigInt(take_sqlstmt(arg_1(arguments)),
                  take_int(arg_2(arguments)),
                  take_bigint(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_bind_bigint */



objectType sql_bind_bigrat (listType arguments)

  { /* sql_bind_bigrat */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_bigint(arg_3(arguments));
    isit_bigint(arg_4(arguments));
    sqlBindBigRat(take_sqlstmt(arg_1(arguments)),
                  take_int(arg_2(arguments)),
                  take_bigint(arg_3(arguments)),
                  take_bigint(arg_4(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_bind_bigrat */



objectType sql_bind_bool (listType arguments)

  { /* sql_bind_bool */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_bool(arg_3(arguments));
    sqlBindBool(take_sqlstmt(arg_1(arguments)),
                take_int(arg_2(arguments)),
                take_bool(arg_3(arguments)) == SYS_TRUE_OBJECT);
    return SYS_EMPTY_OBJECT;
  } /* sql_bind_bool */



objectType sql_bind_bstri (listType arguments)

  { /* sql_bind_bstri */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_bstri(arg_3(arguments));
    sqlBindBStri(take_sqlstmt(arg_1(arguments)),
                 take_int(arg_2(arguments)),
                 take_bstri(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_bind_bstri */



objectType sql_bind_duration (listType arguments)

  { /* sql_bind_duration */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    isit_int(arg_9(arguments));
    sqlBindDuration(take_sqlstmt(arg_1(arguments)),
                    take_int(arg_2(arguments)),
                    take_int(arg_3(arguments)),
                    take_int(arg_4(arguments)),
                    take_int(arg_5(arguments)),
                    take_int(arg_6(arguments)),
                    take_int(arg_7(arguments)),
                    take_int(arg_8(arguments)),
                    take_int(arg_9(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_bind_duration */



objectType sql_bind_float (listType arguments)

  { /* sql_bind_float */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_float(arg_3(arguments));
    sqlBindFloat(take_sqlstmt(arg_1(arguments)),
               take_int(arg_2(arguments)),
               take_float(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_bind_float */



objectType sql_bind_int (listType arguments)

  { /* sql_bind_int */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    sqlBindInt(take_sqlstmt(arg_1(arguments)),
               take_int(arg_2(arguments)),
               take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_bind_int */



objectType sql_bind_null (listType arguments)

  { /* sql_bind_null */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    sqlBindNull(take_sqlstmt(arg_1(arguments)),
                take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_bind_null */



objectType sql_bind_stri (listType arguments)

  { /* sql_bind_stri */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_stri(arg_3(arguments));
    sqlBindStri(take_sqlstmt(arg_1(arguments)),
                take_int(arg_2(arguments)),
                take_stri(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_bind_stri */



objectType sql_bind_time (listType arguments)

  { /* sql_bind_time */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    isit_int(arg_9(arguments));
    sqlBindTime(take_sqlstmt(arg_1(arguments)),
                take_int(arg_2(arguments)),
                take_int(arg_3(arguments)),
                take_int(arg_4(arguments)),
                take_int(arg_5(arguments)),
                take_int(arg_6(arguments)),
                take_int(arg_7(arguments)),
                take_int(arg_8(arguments)),
                take_int(arg_9(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_bind_time */



objectType sql_column_bigint (listType arguments)

  {
    bigIntType number;

  /* sql_column_bigint */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    number = sqlColumnBigInt(take_sqlstmt(arg_1(arguments)),
                             take_int(arg_2(arguments)));
    return bld_bigint_temp(number);
  } /* sql_column_bigint */



objectType sql_column_bigrat (listType arguments)

  { /* sql_column_bigrat */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_bigint(arg_3(arguments));
    isit_bigint(arg_4(arguments));
    sqlColumnBigRat(take_sqlstmt(arg_1(arguments)),
                    take_int(arg_2(arguments)),
                    &arg_3(arguments)->value.bigIntValue,
                    &arg_4(arguments)->value.bigIntValue);
    return SYS_EMPTY_OBJECT;
  } /* sql_column_bigrat */



objectType sql_close (listType arguments)

  { /* sql_close */
    isit_database(arg_1(arguments));
    sqlClose(take_database(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_close */



objectType sql_column_bool (listType arguments)

  { /* sql_column_bool */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    if (sqlColumnBool(take_sqlstmt(arg_1(arguments)),
                      take_int(arg_2(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* sql_column_bool */



objectType sql_column_bstri (listType arguments)

  {
    bstriType bstri;

  /* sql_column_bstri */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    bstri = sqlColumnBStri(take_sqlstmt(arg_1(arguments)),
                           take_int(arg_2(arguments)));
    return bld_bstri_temp(bstri);
  } /* sql_column_bstri */



objectType sql_column_duration (listType arguments)

  { /* sql_column_duration */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    isit_int(arg_9(arguments));
    sqlColumnDuration(take_sqlstmt(arg_1(arguments)),
                      take_int(arg_2(arguments)),
                      &arg_3(arguments)->value.intValue,
                      &arg_4(arguments)->value.intValue,
                      &arg_5(arguments)->value.intValue,
                      &arg_6(arguments)->value.intValue,
                      &arg_7(arguments)->value.intValue,
                      &arg_8(arguments)->value.intValue,
                      &arg_9(arguments)->value.intValue);
    return SYS_EMPTY_OBJECT;
  } /* sql_column_duration */



objectType sql_column_float (listType arguments)

  {
    floatType number;

  /* sql_column_float */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    number = sqlColumnFloat(take_sqlstmt(arg_1(arguments)),
                          take_int(arg_2(arguments)));
    return bld_float_temp(number);
  } /* sql_column_float */



objectType sql_column_int (listType arguments)

  {
    intType number;

  /* sql_column_int */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    number = sqlColumnInt(take_sqlstmt(arg_1(arguments)),
                          take_int(arg_2(arguments)));
    return bld_int_temp(number);
  } /* sql_column_int */



objectType sql_column_stri (listType arguments)

  {
    striType stri;

  /* sql_column_stri */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    stri = sqlColumnStri(take_sqlstmt(arg_1(arguments)),
                         take_int(arg_2(arguments)));
    return bld_stri_temp(stri);
  } /* sql_column_stri */



objectType sql_column_time (listType arguments)

  {
    boolType is_dst;

  /* sql_column_time */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    isit_int(arg_9(arguments));
    isit_int(arg_10(arguments));
    isit_bool(arg_11(arguments));
    sqlColumnTime(take_sqlstmt(arg_1(arguments)),
                  take_int(arg_2(arguments)),
                  &arg_3(arguments)->value.intValue,
                  &arg_4(arguments)->value.intValue,
                  &arg_5(arguments)->value.intValue,
                  &arg_6(arguments)->value.intValue,
                  &arg_7(arguments)->value.intValue,
                  &arg_8(arguments)->value.intValue,
                  &arg_9(arguments)->value.intValue,
                  &arg_10(arguments)->value.intValue,
                  &is_dst);
    if (is_dst) {
      arg_11(arguments)->value.objValue = SYS_TRUE_OBJECT;
    } else {
      arg_11(arguments)->value.objValue = SYS_FALSE_OBJECT;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* sql_column_time */



objectType sql_commit (listType arguments)

  { /* sql_commit */
    isit_database(arg_1(arguments));
    sqlCommit(take_database(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_commit */



objectType sql_cpy_db (listType arguments)

  {
    objectType db_variable;

  /* sql_cpy_db */
    db_variable = arg_1(arguments);
    isit_database(db_variable);
    is_variable(db_variable);
    isit_database(arg_3(arguments));
    sqlCpyDb(&db_variable->value.databaseValue, take_database(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_cpy_db */



objectType sql_cpy_stmt (listType arguments)

  {
    objectType stmt_variable;

  /* sql_cpy_stmt */
    stmt_variable = arg_1(arguments);
    isit_sqlstmt(stmt_variable);
    is_variable(stmt_variable);
    isit_sqlstmt(arg_3(arguments));
    sqlCpyStmt(&stmt_variable->value.sqlStmtValue, take_sqlstmt(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_cpy_stmt */



objectType sql_create_db (listType arguments)

  { /* sql_create_db */
    isit_database(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), DATABASEOBJECT);
    arg_1(arguments)->value.databaseValue = sqlCreateDb(take_database(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_create_db */



objectType sql_create_stmt (listType arguments)

  { /* sql_create_stmt */
    isit_sqlstmt(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), SQLSTMTOBJECT);
    arg_1(arguments)->value.sqlStmtValue = sqlCreateStmt(take_sqlstmt(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_create_stmt */



objectType sql_destr_db (listType arguments)

  { /* sql_destr_db */
    isit_database(arg_1(arguments));
    sqlDestrDb(take_database(arg_1(arguments)));
    arg_1(arguments)->value.databaseValue = NULL;
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* sql_destr_db */



objectType sql_destr_stmt (listType arguments)

  { /* sql_destr_stmt */
    isit_sqlstmt(arg_1(arguments));
    sqlDestrStmt(take_sqlstmt(arg_1(arguments)));
    arg_1(arguments)->value.sqlStmtValue = NULL;
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* sql_destr_stmt */



objectType sql_empty_db (listType arguments)

  { /* sql_empty_db */
    return bld_database_temp(NULL);
  } /* sql_empty_db */



objectType sql_empty_stmt (listType arguments)

  { /* sql_empty_stmt */
    return bld_sqlstmt_temp(NULL);
  } /* sql_empty_stmt */



objectType sql_execute (listType arguments)

  { /* sql_execute */
    isit_sqlstmt(arg_1(arguments));
    sqlExecute(take_sqlstmt(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_execute */



objectType sql_fetch (listType arguments)

  {
    boolType success;

  /* sql_fetch */
    isit_sqlstmt(arg_1(arguments));
    success = sqlFetch(take_sqlstmt(arg_1(arguments)));
    if (success) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* sql_fetch */



objectType sql_is_null (listType arguments)

  { /* sql_is_null */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    if (sqlIsNull(take_sqlstmt(arg_1(arguments)),
                  take_int(arg_2(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* sql_is_null */



objectType sql_open (listType arguments)

  {
    databaseType database;

  /* sql_open */
    isit_int(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_stri(arg_3(arguments));
    isit_stri(arg_4(arguments));
    database = sqlOpen(take_int(arg_1(arguments)),
                       take_stri(arg_2(arguments)),
                       take_stri(arg_3(arguments)),
                       take_stri(arg_4(arguments)));
    return bld_database_temp(database);
  } /* sql_open */



objectType sql_prepare (listType arguments)

  {
    sqlStmtType sqlStatement;

  /* sql_prepare */
    isit_database(arg_1(arguments));
    isit_stri(arg_2(arguments));
    sqlStatement = sqlPrepare(take_database(arg_1(arguments)),
                              take_stri(arg_2(arguments)));
    return bld_sqlstmt_temp(sqlStatement);
  } /* sql_prepare */



objectType sql_stmt_column_count (listType arguments)

  {
    intType column_count;

  /* sql_stmt_column_count */
    isit_sqlstmt(arg_1(arguments));
    column_count = sqlStmtColumnCount(take_sqlstmt(arg_1(arguments)));
    return bld_int_temp(column_count);
  } /* sql_stmt_column_count */



objectType sql_stmt_column_name (listType arguments)

  {
    striType name;

  /* sql_stmt_column_name */
    isit_sqlstmt(arg_1(arguments));
    isit_int(arg_2(arguments));
    name = sqlStmtColumnName(take_sqlstmt(arg_1(arguments)),
                             take_int(arg_2(arguments)));
    return bld_stri_temp(name);
  } /* sql_stmt_column_name */
