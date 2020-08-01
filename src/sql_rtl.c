/********************************************************************/
/*                                                                  */
/*  sql_rtl.c     Database access functions.                        */
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
/*  File: seed7/src/sql_rtl.c                                       */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: Database access functions.                             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"
#include "math.h"
#include "float.h"

#include "common.h"
#include "data_rtl.h"
#include "flt_rtl.h"
#include "big_drv.h"
#include "rtl_err.h"
#include "sql_drv.h"


#undef ENCODE_INFINITY
#undef ENCODE_NAN


typedef struct dbStruct {
    uintType     usage_count;
    sqlFuncType  sqlFunc;
  } dbRecord, *dbType;

typedef struct preparedStmtStruct {
    uintType     usage_count;
    sqlFuncType  sqlFunc;
  } preparedStmtRecord, *preparedStmtType;



void sqlBindBigInt (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType value)

  { /* sqlBindBigInt */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBigInt == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBigInt(sqlStatement, pos, value);
    } /* if */
  } /* sqlBindBigInt */



void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType numerator, const const_bigIntType denominator)

  { /* sqlBindBigRat */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBigRat == NULL) {
      raise_error(RANGE_ERROR);
#if !defined ENCODE_INFINITY && !defined ENCODE_NAN
    } else if (bigEqSignedDigit(denominator, 0)) {
      raise_error(RANGE_ERROR);
#else
#ifndef ENCODE_INFINITY
    } else if (bigEqSignedDigit(denominator, 0) && !bigEqSignedDigit(numerator, 0)) {
      raise_error(RANGE_ERROR);
#endif
#ifndef ENCODE_NAN
    } else if (bigEqSignedDigit(denominator, 0) && bigEqSignedDigit(numerator, 0)) {
      raise_error(RANGE_ERROR);
#endif
#endif
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBigRat(sqlStatement, pos,
                                                                numerator, denominator);
    } /* if */
  } /* sqlBindBigRat */



void sqlBindBool (sqlStmtType sqlStatement, intType pos, boolType value)

  { /* sqlBindBool */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBool == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBool(sqlStatement, pos, value);
    } /* if */
  } /* sqlBindBool */



void sqlBindBStri (sqlStmtType sqlStatement, intType pos, bstriType bstri)

  { /* sqlBindBStri */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBStri == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBStri(sqlStatement, pos, bstri);
    } /* if */
  } /* sqlBindBStri */



void sqlBindDuration (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  { /* sqlBindDuration */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindDuration == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindDuration(sqlStatement, pos,
          year, month, day, hour, minute, second, micro_second);
    } /* if */
  } /* sqlBindDuration */



void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value)

  { /* sqlBindFloat */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindFloat == NULL) {
      raise_error(RANGE_ERROR);
#ifndef ENCODE_INFINITY
    } else if (value == POSITIVE_INFINITY || value == NEGATIVE_INFINITY) {
      raise_error(RANGE_ERROR);
#endif
#ifndef ENCODE_NAN
    } else if (isnan(value)) {
      raise_error(RANGE_ERROR);
#endif
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindFloat(sqlStatement, pos, value);
    } /* if */
  } /* sqlBindFloat */



void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value)

  { /* sqlBindInt */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindInt == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindInt(sqlStatement, pos, value);
    } /* if */
  } /* sqlBindInt */



void sqlBindNull (sqlStmtType sqlStatement, intType pos)

  { /* sqlBindNull */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindNull == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindNull(sqlStatement, pos);
    } /* if */
  } /* sqlBindNull */



void sqlBindStri (sqlStmtType sqlStatement, intType pos, striType stri)

  { /* sqlBindStri */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindStri == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindStri(sqlStatement, pos, stri);
    } /* if */
  } /* sqlBindStri */



void sqlBindTime (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  { /* sqlBindTime */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindTime == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindTime(sqlStatement, pos,
          year, month, day, hour, minute, second, micro_second);
    } /* if */
  } /* sqlBindTime */



void sqlClose (databaseType database)

  { /* sqlClose */
    if (database == NULL ||
        ((dbType) database)->sqlFunc == NULL ||
        ((dbType) database)->sqlFunc->sqlClose == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((dbType) database)->sqlFunc->sqlClose(database);
    } /* if */
  } /* sqlClose */



bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column)

  { /* sqlColumnBigInt */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBigInt == NULL) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBigInt(sqlStatement, column);
    } /* if */
  } /* sqlColumnBigInt */



void sqlColumnBigRat (sqlStmtType sqlStatement, intType column,
    bigIntType *numerator, bigIntType *denominator)

  { /* sqlColumnBigRat */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBigRat == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBigRat(sqlStatement, column,
          numerator, denominator);
    } /* if */
  } /* sqlColumnBigRat */



boolType sqlColumnBool (sqlStmtType sqlStatement, intType column)

  { /* sqlColumnBool */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBool == NULL) {
      raise_error(RANGE_ERROR);
      return FALSE;
    } else {
      return ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBool(sqlStatement, column);
    } /* if */
  } /* sqlColumnBool */



bstriType sqlColumnBStri (sqlStmtType sqlStatement, intType column)

  { /* sqlColumnBStri */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBStri == NULL) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBStri(sqlStatement, column);
    } /* if */
  } /* sqlColumnBStri */



void sqlColumnDuration (sqlStmtType sqlStatement, intType column,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second)

  { /* sqlColumnDuration */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnDuration == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnDuration(sqlStatement, column,
          year, month, day, hour, minute, second, micro_second);
    } /* if */
  } /* sqlColumnDuration */



floatType sqlColumnFloat (sqlStmtType sqlStatement, intType column)

  { /* sqlColumnFloat */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnFloat == NULL) {
      raise_error(RANGE_ERROR);
      return 0.0;
    } else {
      return ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnFloat(sqlStatement, column);
    } /* if */
  } /* sqlColumnFloat */



intType sqlColumnInt (sqlStmtType sqlStatement, intType column)

  { /* sqlColumnInt */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnInt == NULL) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      return ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnInt(sqlStatement, column);
    } /* if */
  } /* sqlColumnInt */



striType sqlColumnStri (sqlStmtType sqlStatement, intType column)

  { /* sqlColumnStri */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnStri == NULL) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnStri(sqlStatement, column);
    } /* if */
  } /* sqlColumnStri */



void sqlColumnTime (sqlStmtType sqlStatement, intType column,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second,
    intType *time_zone, boolType *is_dst)

  { /* sqlColumnTime */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnTime == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnTime(sqlStatement, column,
          year, month, day, hour, minute, second,
          micro_second, time_zone, is_dst);
    } /* if */
  } /* sqlColumnTime */



void sqlCommit (databaseType database)

  { /* sqlCommit */
    if (database == NULL ||
        ((dbType) database)->sqlFunc == NULL ||
        ((dbType) database)->sqlFunc->sqlCommit == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((dbType) database)->sqlFunc->sqlCommit(database);
    } /* if */
  } /* sqlCommit */



void sqlCpyDb (databaseType *const db_to, const databaseType db_from)

  {
    dbType db_dest;
    dbType db_source;

  /* sqlCpyDb */
    /* printf("sqlCpyStmt(%lu, %ld)\n", db_to, db_from); */
    db_dest = (dbType) *db_to;
    db_source = (dbType) db_from;
    if (db_source != NULL) {
      db_source->usage_count++;
    } /* if */
    if (db_dest != NULL) {
      db_dest->usage_count--;
      if (db_dest->usage_count == 0 &&
          db_dest->sqlFunc != NULL &&
          db_dest->sqlFunc->freeDatabase != NULL) {
        /* printf("FREE %lx\n", (long unsigned int) db_dest); */
        db_dest->sqlFunc->freeDatabase((databaseType) db_dest);
      } /* if */
    } /* if */
    *db_to = (databaseType) db_source;
  } /* sqlCpyDb */



void sqlCpyStmt (sqlStmtType *const stmt_to, const sqlStmtType stmt_from)

  {
    preparedStmtType statement_dest;
    preparedStmtType statement_source;

  /* sqlCpyStmt */
    /* printf("sqlCpyStmt(%lu, %ld)\n", stmt_to, stmt_from); */
    statement_dest = (preparedStmtType) *stmt_to;
    statement_source = (preparedStmtType) stmt_from;
    if (statement_source != NULL) {
      statement_source->usage_count++;
    } /* if */
    if (statement_dest != NULL) {
      statement_dest->usage_count--;
      if (statement_dest->usage_count == 0 &&
          statement_dest->sqlFunc != NULL &&
          statement_dest->sqlFunc->freePreparedStmt != NULL) {
        /* printf("FREE %lx\n", (long unsigned int) statement_dest); */
        statement_dest->sqlFunc->freePreparedStmt((sqlStmtType) statement_dest);
      } /* if */
    } /* if */
    *stmt_to = (sqlStmtType) statement_source;
  } /* sqlCpyStmt */



databaseType sqlCreateDb (const databaseType db_from)

  { /* sqlCreateDb */
    if (db_from != NULL) {
      ((dbType) db_from)->usage_count++;
    } /* if */
    return db_from;
  } /* sqlCreateDb */



sqlStmtType sqlCreateStmt (const sqlStmtType stmt_from)

  { /* sqlCreateStmt */
    if (stmt_from != NULL) {
      ((preparedStmtType) stmt_from)->usage_count++;
    } /* if */
    return stmt_from;
  } /* sqlCreateStmt */



void sqlDestrDb (const databaseType old_db)

  {
    dbType old_database;

  /* sqlDestrDb */
    old_database = (dbType) old_db;
    if (old_database != NULL) {
      old_database->usage_count--;
      if (old_database->usage_count == 0 &&
          old_database->sqlFunc != NULL &&
          old_database->sqlFunc->freeDatabase != NULL) {
        /* printf("FREE %lx\n", (long unsigned int) old_database); */
        old_database->sqlFunc->freeDatabase((databaseType) old_database);
      } /* if */
    } /* if */
  } /* sqlDestrDb */



void sqlDestrStmt (const sqlStmtType old_stmt)

  {
    preparedStmtType old_statement;

  /* sqlDestrStmt */
    old_statement = (preparedStmtType) old_stmt;
    if (old_statement != NULL) {
      old_statement->usage_count--;
      if (old_statement->usage_count == 0 &&
          old_statement->sqlFunc != NULL &&
          old_statement->sqlFunc->freePreparedStmt != NULL) {
        /* printf("FREE %lx\n", (long unsigned int) old_statement); */
        old_statement->sqlFunc->freePreparedStmt((sqlStmtType) old_statement);
      } /* if */
    } /* if */
  } /* sqlDestrStmt */



void sqlExecute (sqlStmtType sqlStatement)

  { /* sqlExecute */
    if (sqlStatement == NULL) {
      /* Do nothing */
    } else if (((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
               ((preparedStmtType) sqlStatement)->sqlFunc->sqlExecute == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlExecute(sqlStatement);
    } /* if */
  } /* sqlExecute */



boolType sqlFetch (sqlStmtType sqlStatement)

  { /* sqlFetch */
    if (sqlStatement == NULL) {
      return FALSE;
    } else if (((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
               ((preparedStmtType) sqlStatement)->sqlFunc->sqlFetch == NULL) {
      raise_error(RANGE_ERROR);
      return FALSE;
    } else {
      return ((preparedStmtType) sqlStatement)->sqlFunc->sqlFetch(sqlStatement);
    } /* if */
  } /* sqlFetch */



boolType sqlIsNull (sqlStmtType sqlStatement, intType column)

  { /* sqlIsNull */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlIsNull == NULL) {
      raise_error(RANGE_ERROR);
      return FALSE;
    } else {
      return ((preparedStmtType) sqlStatement)->sqlFunc->sqlIsNull(sqlStatement, column);
    } /* if */
  } /* sqlIsNull */



databaseType sqlOpen (intType driver, const const_striType dbName,
    const const_striType user, const const_striType password)

  {
    databaseType database;

  /* sqlOpen */
    /* printf("sqlOpen(" FMT_D ", ...)\n", driver); */
    switch (driver) {
#ifdef MYSQL_INCLUDE
      case 1:
        database = sqlOpenMy(dbName, user, password);
        break;
#endif
#ifdef SQLITE_INCLUDE
      case 2:
        database = sqlOpenLite(dbName, user, password);
        break;
#endif
#ifdef POSTGRESQL_INCLUDE
      case 3:
        database = sqlOpenPost(dbName, user, password);
        break;
#endif
#ifdef OCI_INCLUDE
      case 4:
        database = sqlOpenOci(dbName, user, password);
        break;
#endif
#ifdef ODBC_INCLUDE
      case 5:
        database = sqlOpenOdbc(dbName, user, password);
        break;
#endif
      default:
        /* printf("driver: " FMT_D "\n", driver); */
        raise_error(RANGE_ERROR);
        database = NULL;
        break;
    } /* switch */
    return database;
  } /* sqlOpen */



sqlStmtType sqlPrepare (databaseType database, striType sqlStatementStri)

  { /* sqlPrepare */
    if (database == NULL ||
        ((dbType) database)->sqlFunc == NULL ||
        ((dbType) database)->sqlFunc->sqlPrepare == NULL) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return ((dbType) database)->sqlFunc->sqlPrepare(database, sqlStatementStri);
    } /* if */
  } /* sqlPrepare */



intType sqlStmtColumnCount (sqlStmtType sqlStatement)

  { /* sqlStmtColumnCount */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlStmtColumnCount == NULL) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      return ((preparedStmtType) sqlStatement)->sqlFunc->sqlStmtColumnCount(sqlStatement);
    } /* if */
  } /* sqlStmtColumnCount */



striType sqlStmtColumnName (sqlStmtType sqlStatement, intType column)

  { /* sqlStmtColumnName */
    if (sqlStatement == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
        ((preparedStmtType) sqlStatement)->sqlFunc->sqlStmtColumnName == NULL) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return ((preparedStmtType) sqlStatement)->sqlFunc->sqlStmtColumnName(sqlStatement, column);
    } /* if */
  } /* sqlStmtColumnName */
