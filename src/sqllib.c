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
#include "sql_drv.h"



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
    isit_int(arg_10(arguments));
    sqlBindTime(take_sqlstmt(arg_1(arguments)),
                take_int(arg_2(arguments)),
                take_int(arg_3(arguments)),
                take_int(arg_4(arguments)),
                take_int(arg_5(arguments)),
                take_int(arg_6(arguments)),
                take_int(arg_7(arguments)),
                take_int(arg_8(arguments)),
                take_int(arg_9(arguments)),
                take_int(arg_10(arguments)));
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



objectType sql_cmp_db (listType arguments)

  {
    memSizeType ref1;
    memSizeType ref2;
    intType signumValue;

  /* sql_cmp_db */
    isit_database(arg_1(arguments));
    isit_database(arg_2(arguments));
    ref1 = (memSizeType) take_database(arg_1(arguments));
    ref2 = (memSizeType) take_database(arg_2(arguments));
    if (ref1 < ref2) {
      signumValue = -1;
    } else if (ref1 > ref2) {
      signumValue = 1;
    } else {
      signumValue = 0;
    } /* if */
    return bld_int_temp(signumValue);
  } /* sql_cmp_db */



objectType sql_cmp_stmt (listType arguments)

  {
    memSizeType ref1;
    memSizeType ref2;
    intType signumValue;

  /* sql_cmp_stmt */
    isit_sqlstmt(arg_1(arguments));
    isit_sqlstmt(arg_2(arguments));
    ref1 = (memSizeType) take_sqlstmt(arg_1(arguments));
    ref2 = (memSizeType) take_sqlstmt(arg_2(arguments));
    if (ref1 < ref2) {
      signumValue = -1;
    } else if (ref1 > ref2) {
      signumValue = 1;
    } else {
      signumValue = 0;
    } /* if */
    return bld_int_temp(signumValue);
  } /* sql_cmp_stmt */



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



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType sql_cpy_db (listType arguments)

  {
    objectType dest;

  /* sql_cpy_db */
    dest = arg_1(arguments);
    isit_database(dest);
    is_variable(dest);
    isit_database(arg_3(arguments));
    sqlCpyDb(&dest->value.databaseValue, take_database(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_cpy_db */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType sql_cpy_stmt (listType arguments)

  {
    objectType dest;

  /* sql_cpy_stmt */
    dest = arg_1(arguments);
    isit_sqlstmt(dest);
    is_variable(dest);
    isit_sqlstmt(arg_3(arguments));
    sqlCpyStmt(&dest->value.sqlStmtValue, take_sqlstmt(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_cpy_stmt */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType sql_create_db (listType arguments)

  { /* sql_create_db */
    isit_database(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), DATABASEOBJECT);
    arg_1(arguments)->value.databaseValue = sqlCreateDb(take_database(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* sql_create_db */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
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



objectType sql_driver (listType arguments)

  { /* sql_driver */
    isit_database(arg_1(arguments));
    return bld_int_temp(sqlDriver(take_database(arg_1(arguments))));
  } /* sql_driver */



objectType sql_empty_db (listType arguments)

  { /* sql_empty_db */
    return bld_database_temp(NULL);
  } /* sql_empty_db */



objectType sql_empty_stmt (listType arguments)

  { /* sql_empty_stmt */
    return bld_sqlstmt_temp(NULL);
  } /* sql_empty_stmt */



objectType sql_eq_db (listType arguments)

  { /* sql_eq_db */
    isit_database(arg_1(arguments));
    isit_database(arg_3(arguments));
    if (take_database(arg_1(arguments)) ==
        take_database(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* sql_eq_db */



objectType sql_eq_stmt (listType arguments)

  { /* sql_eq_stmt */
    isit_sqlstmt(arg_1(arguments));
    isit_sqlstmt(arg_3(arguments));
    if (take_sqlstmt(arg_1(arguments)) ==
        take_sqlstmt(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* sql_eq_stmt */



objectType sql_err_code (listType arguments)

  { /* sql_err_code */
    return bld_int_temp(sqlErrCode());
  } /* sql_err_code */



objectType sql_err_db_func (listType arguments)

  { /* sql_err_db_func */
    return bld_stri_temp(sqlErrDbFunc());
  } /* sql_err_db_func */



objectType sql_err_lib_func (listType arguments)

  { /* sql_err_lib_func */
    return bld_stri_temp(sqlErrLibFunc());
  } /* sql_err_lib_func */



objectType sql_err_message (listType arguments)

  { /* sql_err_message */
    return bld_stri_temp(sqlErrMessage());
  } /* sql_err_message */



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



objectType sql_ne_db (listType arguments)

  { /* sql_ne_db */
    isit_database(arg_1(arguments));
    isit_database(arg_3(arguments));
    if (take_database(arg_1(arguments)) !=
        take_database(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* sql_ne_db */



objectType sql_ne_stmt (listType arguments)

  { /* sql_ne_stmt */
    isit_sqlstmt(arg_1(arguments));
    isit_sqlstmt(arg_3(arguments));
    if (take_sqlstmt(arg_1(arguments)) !=
        take_sqlstmt(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* sql_ne_stmt */



objectType sql_open_fire (listType arguments)

  {
    databaseType database;

  /* sql_open_fire */
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_stri(arg_4(arguments));
    isit_stri(arg_5(arguments));
    isit_stri(arg_6(arguments));
    database = sqlOpenFire(take_stri(arg_2(arguments)),
                           take_int(arg_3(arguments)),
                           take_stri(arg_4(arguments)),
                           take_stri(arg_5(arguments)),
                           take_stri(arg_6(arguments)));
    return bld_database_temp(database);
  } /* sql_open_fire */



objectType sql_open_lite (listType arguments)

  {
    databaseType database;

  /* sql_open_lite */
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_stri(arg_4(arguments));
    isit_stri(arg_5(arguments));
    isit_stri(arg_6(arguments));
    database = sqlOpenLite(take_stri(arg_2(arguments)),
                           take_int(arg_3(arguments)),
                           take_stri(arg_4(arguments)),
                           take_stri(arg_5(arguments)),
                           take_stri(arg_6(arguments)));
    return bld_database_temp(database);
  } /* sql_open_lite */



objectType sql_open_my (listType arguments)

  {
    databaseType database;

  /* sql_open_my */
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_stri(arg_4(arguments));
    isit_stri(arg_5(arguments));
    isit_stri(arg_6(arguments));
    database = sqlOpenMy(take_stri(arg_2(arguments)),
                         take_int(arg_3(arguments)),
                         take_stri(arg_4(arguments)),
                         take_stri(arg_5(arguments)),
                         take_stri(arg_6(arguments)));
    return bld_database_temp(database);
  } /* sql_open_my */



objectType sql_open_oci (listType arguments)

  {
    databaseType database;

  /* sql_open_oci */
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_stri(arg_4(arguments));
    isit_stri(arg_5(arguments));
    isit_stri(arg_6(arguments));
    database = sqlOpenOci(take_stri(arg_2(arguments)),
                          take_int(arg_3(arguments)),
                          take_stri(arg_4(arguments)),
                          take_stri(arg_5(arguments)),
                          take_stri(arg_6(arguments)));
    return bld_database_temp(database);
  } /* sql_open_oci */



objectType sql_open_odbc (listType arguments)

  {
    databaseType database;

  /* sql_open_odbc */
    isit_stri(arg_2(arguments));
    isit_stri(arg_3(arguments));
    isit_stri(arg_4(arguments));
    isit_stri(arg_5(arguments));
    isit_stri(arg_6(arguments));
    database = sqlOpenOdbc(take_stri(arg_2(arguments)),
                           take_stri(arg_3(arguments)),
                           take_stri(arg_4(arguments)),
                           take_stri(arg_5(arguments)),
                           take_stri(arg_6(arguments)));
    return bld_database_temp(database);
  } /* sql_open_odbc */



objectType sql_open_post (listType arguments)

  {
    databaseType database;

  /* sql_open_post */
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_stri(arg_4(arguments));
    isit_stri(arg_5(arguments));
    isit_stri(arg_6(arguments));
    database = sqlOpenPost(take_stri(arg_2(arguments)),
                           take_int(arg_3(arguments)),
                           take_stri(arg_4(arguments)),
                           take_stri(arg_5(arguments)),
                           take_stri(arg_6(arguments)));
    return bld_database_temp(database);
  } /* sql_open_post */



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
