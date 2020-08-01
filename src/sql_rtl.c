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

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdio.h"
#include "math.h"
#include "float.h"

#include "common.h"
#include "data_rtl.h"
#include "striutl.h"
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
    logFunction(printf("sqlBindBigInt(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, bigHexCStri(value)););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBigInt == NULL)) {
      logError(printf("sqlBindBigInt(" FMT_U_MEM ", " FMT_D ", *): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, pos););
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBigInt(sqlStatement, pos, value);
    } /* if */
  } /* sqlBindBigInt */



void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType numerator, const const_bigIntType denominator)

  { /* sqlBindBigRat */
    logFunction(printf("sqlBindBigRat(" FMT_U_MEM ", " FMT_D ", %s, %s)\n",
                       (memSizeType) sqlStatement, pos,
                       bigHexCStri(numerator), bigHexCStri(denominator)););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBigRat == NULL)) {
      logError(printf("sqlBindBigRat(" FMT_U_MEM ", " FMT_D ", *): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, pos););
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
    logFunction(printf("sqlBindBool(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, value ? "TRUE" : "FALSE"););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBool == NULL)) {
      logError(printf("sqlBindBool(" FMT_U_MEM ", " FMT_D ", %s): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, pos, value ? "TRUE" : "FALSE"););
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBool(sqlStatement, pos, value);
    } /* if */
  } /* sqlBindBool */



void sqlBindBStri (sqlStmtType sqlStatement, intType pos, bstriType bstri)

  { /* sqlBindBStri */
    logFunction(printf("sqlBindBStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, bstriAsUnquotedCStri(bstri)););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBStri == NULL)) {
      logError(printf("sqlBindBStri(" FMT_U_MEM ", " FMT_D ", \"%s\"): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, pos, bstriAsUnquotedCStri(bstri)););
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBStri(sqlStatement, pos, bstri);
    } /* if */
  } /* sqlBindBStri */



void sqlBindDuration (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  { /* sqlBindDuration */
    logFunction(printf("sqlBindDuration(" FMT_U_MEM ", " FMT_D ", "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) ")\n",
                       (memSizeType) sqlStatement, pos,
                       year, month, day,
                       hour, minute, second, micro_second););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindDuration == NULL)) {
      logError(printf("sqlBindDuration(" FMT_U_MEM ", " FMT_D ", "
                      F_D(04) "-" F_D(02) "-" F_D(02) " "
                      F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) "): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, pos,
                      year, month, day,
                      hour, minute, second, micro_second););
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindDuration(sqlStatement, pos,
          year, month, day, hour, minute, second, micro_second);
    } /* if */
  } /* sqlBindDuration */



void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value)

  { /* sqlBindFloat */
    logFunction(printf("sqlBindFloat(" FMT_U_MEM ", " FMT_D ", " FMT_E ")\n",
                       (memSizeType) sqlStatement, pos, value););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindFloat == NULL)) {
      logError(printf("sqlBindFloat(" FMT_U_MEM ", " FMT_D ", " FMT_E "): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, pos, value););
      raise_error(RANGE_ERROR);
#ifndef ENCODE_INFINITY
    } else if (value == POSITIVE_INFINITY || value == NEGATIVE_INFINITY) {
      raise_error(RANGE_ERROR);
#endif
#ifndef ENCODE_NAN
    } else if (os_isnan(value)) {
      raise_error(RANGE_ERROR);
#endif
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindFloat(sqlStatement, pos, value);
    } /* if */
  } /* sqlBindFloat */



void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value)

  { /* sqlBindInt */
    logFunction(printf("sqlBindInt(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos, value););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindInt == NULL)) {
      logError(printf("sqlBindInt(" FMT_U_MEM ", " FMT_D ", " FMT_D "): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, pos, value););
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindInt(sqlStatement, pos, value);
    } /* if */
  } /* sqlBindInt */



void sqlBindNull (sqlStmtType sqlStatement, intType pos)

  { /* sqlBindNull */
    logFunction(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindNull == NULL)) {
      logError(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, pos););
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindNull(sqlStatement, pos);
    } /* if */
  } /* sqlBindNull */



void sqlBindStri (sqlStmtType sqlStatement, intType pos, striType stri)

  { /* sqlBindStri */
    logFunction(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, striAsUnquotedCStri(stri)););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindStri == NULL)) {
      logError(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\"): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, pos, striAsUnquotedCStri(stri)););
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindStri(sqlStatement, pos, stri);
    } /* if */
  } /* sqlBindStri */



void sqlBindTime (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  { /* sqlBindTime */
    logFunction(printf("sqlBindTime(" FMT_U_MEM ", " FMT_D ", "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) ")\n",
                       (memSizeType) sqlStatement, pos,
                       year, month, day,
                       hour, minute, second, micro_second););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindTime == NULL)) {
      logError(printf("sqlBindTime(" FMT_U_MEM ", " FMT_D ", "
                      F_D(04) "-" F_D(02) "-" F_D(02) " "
                      F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) "): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, pos,
                      year, month, day,
                      hour, minute, second, micro_second););
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindTime(sqlStatement, pos,
          year, month, day, hour, minute, second, micro_second);
    } /* if */
  } /* sqlBindTime */



void sqlClose (databaseType database)

  { /* sqlClose */
    logFunction(printf("sqlClose(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    if (unlikely(database == NULL ||
                 ((dbType) database)->sqlFunc == NULL ||
                 ((dbType) database)->sqlFunc->sqlClose == NULL)) {
      logError(printf("sqlClose(" FMT_U_MEM "): Database not okay.\n",
                      (memSizeType) database););
      raise_error(RANGE_ERROR);
    } else {
      ((dbType) database)->sqlFunc->sqlClose(database);
    } /* if */
  } /* sqlClose */



bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column)

  {
    bigIntType columnValue;

  /* sqlColumnBigInt */
    logFunction(printf("sqlColumnBigInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBigInt == NULL)) {
      logError(printf("sqlColumnBigInt(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      columnValue = ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBigInt(
          sqlStatement, column);
    } /* if */
    logFunction(printf("sqlColumnBigInt --> %s\n", bigHexCStri(columnValue)););
    return columnValue;
  } /* sqlColumnBigInt */



void sqlColumnBigRat (sqlStmtType sqlStatement, intType column,
    bigIntType *numerator, bigIntType *denominator)

  { /* sqlColumnBigRat */
    logFunction(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", *, *)\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBigRat == NULL)) {
      logError(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", *, *): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBigRat(sqlStatement, column,
          numerator, denominator);
    } /* if */
    logFunction(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", %s, %s) -->\n",
                       (memSizeType) sqlStatement, column,
                       bigHexCStri(*numerator), bigHexCStri(*denominator)););
  } /* sqlColumnBigRat */



boolType sqlColumnBool (sqlStmtType sqlStatement, intType column)

  {
    boolType columnValue;

  /* sqlColumnBool */
    logFunction(printf("sqlColumnBool(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBool == NULL)) {
      logError(printf("sqlColumnBool(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      columnValue = FALSE;
    } else {
      columnValue = ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBool(
          sqlStatement, column);
    } /* if */
    logFunction(printf("sqlColumnBool --> %s\n", columnValue ? "TRUE" : "FALSE"););
    return columnValue;
  } /* sqlColumnBool */



bstriType sqlColumnBStri (sqlStmtType sqlStatement, intType column)

  {
    bstriType columnValue;

  /* sqlColumnBStri */
    logFunction(printf("sqlColumnBStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBStri == NULL)) {
      logError(printf("sqlColumnBStri(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      columnValue = ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBStri(
          sqlStatement, column);
    } /* if */
    logFunction(printf("sqlColumnBStri --> \"%s\"\n",
                       bstriAsUnquotedCStri(columnValue)););
    return columnValue;
  } /* sqlColumnBStri */



void sqlColumnDuration (sqlStmtType sqlStatement, intType column,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second)

  { /* sqlColumnDuration */
    logFunction(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnDuration == NULL)) {
      logError(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", *): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnDuration(sqlStatement, column,
          year, month, day, hour, minute, second, micro_second);
    } /* if */
    logFunction(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", "
                                            F_D(04) "-" F_D(02) "-" F_D(02) " "
                                            F_D(02) ":" F_D(02) ":" F_D(02) "."
                                            F_D(06) ") -->\n",
                       (memSizeType) sqlStatement, column,
                       *year, *month, *day, *hour, *minute, *second,
                       *micro_second););
  } /* sqlColumnDuration */



floatType sqlColumnFloat (sqlStmtType sqlStatement, intType column)

  {
    floatType columnValue;

  /* sqlColumnFloat */
    logFunction(printf("sqlColumnFloat(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnFloat == NULL)) {
      logError(printf("sqlColumnFloat(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      columnValue = 0.0;
    } else {
      columnValue = ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnFloat(
          sqlStatement, column);
    } /* if */
    logFunction(printf("sqlColumnFloat --> " FMT_E "\n", columnValue););
    return columnValue;
  } /* sqlColumnFloat */



intType sqlColumnInt (sqlStmtType sqlStatement, intType column)

  {
    intType columnValue;

  /* sqlColumnInt */
    logFunction(printf("sqlColumnInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnInt == NULL)) {
      logError(printf("sqlColumnInt(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      columnValue = ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnInt(sqlStatement,
          column);
    } /* if */
    logFunction(printf("sqlColumnInt --> " FMT_D "\n", columnValue););
    return columnValue;
  } /* sqlColumnInt */



striType sqlColumnStri (sqlStmtType sqlStatement, intType column)

  {
    striType columnValue;

  /* sqlColumnStri */
    logFunction(printf("sqlColumnStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnStri == NULL)) {
      logError(printf("sqlColumnStri(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      columnValue = ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnStri(
          sqlStatement, column);
    } /* if */
    logFunction(printf("sqlColumnStri --> \"%s\"\n", striAsUnquotedCStri(columnValue)););
    return columnValue;
  } /* sqlColumnStri */



void sqlColumnTime (sqlStmtType sqlStatement, intType column,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second,
    intType *time_zone, boolType *is_dst)

  { /* sqlColumnTime */
    logFunction(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnTime == NULL)) {
      logError(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ", *): "
                      "SQL statement not okay.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnTime(sqlStatement, column,
          year, month, day, hour, minute, second,
          micro_second, time_zone, is_dst);
    } /* if */
    logFunction(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ", "
                                        F_D(04) "-" F_D(02) "-" F_D(02) " "
                                        F_D(02) ":" F_D(02) ":" F_D(02) "."
                                        F_D(06) ", " FMT_D ", %d) -->\n",
                       (memSizeType) sqlStatement, column,
                       *year, *month, *day, *hour, *minute, *second,
                       *micro_second, *time_zone, *is_dst););
  } /* sqlColumnTime */



void sqlCommit (databaseType database)

  { /* sqlCommit */
    logFunction(printf("sqlCommit(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    if (unlikely(database == NULL ||
                 ((dbType) database)->sqlFunc == NULL ||
                 ((dbType) database)->sqlFunc->sqlCommit == NULL)) {
      logError(printf("sqlCommit(" FMT_U_MEM "): Database not okay.\n",
                      (memSizeType) database););
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
    logFunction(printf("sqlCpyDb(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) db_to, (memSizeType) db_from););
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
    logFunction(printf("sqlCpyStmt(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) stmt_to, (memSizeType) stmt_from););
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
    logFunction(printf("sqlCreateDb(" FMT_U_MEM ")\n",
                       (memSizeType) db_from););
    if (db_from != NULL) {
      ((dbType) db_from)->usage_count++;
    } /* if */
    return db_from;
  } /* sqlCreateDb */



sqlStmtType sqlCreateStmt (const sqlStmtType stmt_from)

  { /* sqlCreateStmt */
    logFunction(printf("sqlCreateStmt(" FMT_U_MEM ")\n",
                       (memSizeType) stmt_from););
    if (stmt_from != NULL) {
      ((preparedStmtType) stmt_from)->usage_count++;
    } /* if */
    return stmt_from;
  } /* sqlCreateStmt */



void sqlDestrDb (const databaseType old_db)

  {
    dbType old_database;

  /* sqlDestrDb */
    logFunction(printf("sqlDestrDb(" FMT_U_MEM ")\n",
                       (memSizeType) old_db););
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
    logFunction(printf("sqlDestrStmt(" FMT_U_MEM ")\n",
                       (memSizeType) old_stmt););
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
    logFunction(printf("sqlExecute(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    if (sqlStatement == NULL) {
      /* Do nothing */
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlExecute == NULL)) {
      raise_error(RANGE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlExecute(sqlStatement);
    } /* if */
    logFunction(printf("sqlExecute -->\n"););
  } /* sqlExecute */



boolType sqlFetch (sqlStmtType sqlStatement)

  {
    boolType fetchOkay;

  /* sqlFetch */
    logFunction(printf("sqlFetch(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    if (sqlStatement == NULL) {
      fetchOkay = FALSE;
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlFetch == NULL)) {
      raise_error(RANGE_ERROR);
      fetchOkay = FALSE;
    } else {
      fetchOkay = ((preparedStmtType) sqlStatement)->sqlFunc->sqlFetch(sqlStatement);
    } /* if */
    logFunction(printf("sqlFetch --> %d\n", fetchOkay););
    return fetchOkay;
  } /* sqlFetch */



boolType sqlIsNull (sqlStmtType sqlStatement, intType column)

  {
    boolType isNull;

  /* sqlIsNull */
    logFunction(printf("sqlIsNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlIsNull == NULL)) {
      raise_error(RANGE_ERROR);
      isNull = FALSE;
    } else {
      isNull = ((preparedStmtType) sqlStatement)->sqlFunc->sqlIsNull(sqlStatement,
          column);
    } /* if */
    logFunction(printf("sqlIsNull --> %s\n", isNull ? "TRUE" : "FALSE"););
    return isNull;
  } /* sqlIsNull */



databaseType sqlOpen (intType driver, const const_striType dbName,
    const const_striType user, const const_striType password)

  {
    databaseType database;

  /* sqlOpen */
    logFunction(printf("sqlOpen(" FMT_D ", \"%s\", ",
                       driver, striAsUnquotedCStri(dbName));
                printf("\"%s\", ", striAsUnquotedCStri(user));
                printf("\"%s\")\n", striAsUnquotedCStri(password)););
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
        logError(printf("sqlOpen: unknown driver: " FMT_D "\n", driver););
        raise_error(RANGE_ERROR);
        database = NULL;
        break;
    } /* switch */
    logFunction(printf("sqlOpen --> " FMT_U_MEM "\n",
                       (memSizeType) database););
    return database;
  } /* sqlOpen */



sqlStmtType sqlPrepare (databaseType database, striType sqlStatementStri)

  {
    sqlStmtType preparedStmt;

  /* sqlPrepare */
    logFunction(printf("sqlPrepare(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) database,
                       striAsUnquotedCStri(sqlStatementStri)););
    if (unlikely(database == NULL ||
                 ((dbType) database)->sqlFunc == NULL ||
                 ((dbType) database)->sqlFunc->sqlPrepare == NULL)) {
      raise_error(RANGE_ERROR);
      preparedStmt = NULL;
    } else {
      preparedStmt = ((dbType) database)->sqlFunc->sqlPrepare(database, sqlStatementStri);
    } /* if */
    logFunction(printf("sqlPrepare --> " FMT_U_MEM "\n",
                       (memSizeType) preparedStmt););
    return preparedStmt;
  } /* sqlPrepare */



intType sqlStmtColumnCount (sqlStmtType sqlStatement)

  {
    intType columnCount;

  /* sqlStmtColumnCount */
    logFunction(printf("sqlStmtColumnCount(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlStmtColumnCount == NULL)) {
      raise_error(RANGE_ERROR);
      columnCount = 0;
    } else {
      columnCount = ((preparedStmtType) sqlStatement)->sqlFunc->sqlStmtColumnCount(
          sqlStatement);
    } /* if */
    logFunction(printf("sqlStmtColumnCount --> " FMT_D "\n", columnCount););
    return columnCount;
  } /* sqlStmtColumnCount */



striType sqlStmtColumnName (sqlStmtType sqlStatement, intType column)

  {
    striType name;

  /* sqlStmtColumnName */
    logFunction(printf("sqlStmtColumnName(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                 ((preparedStmtType) sqlStatement)->sqlFunc->sqlStmtColumnName == NULL)) {
      raise_error(RANGE_ERROR);
      name = NULL;
    } else {
      name = ((preparedStmtType) sqlStatement)->sqlFunc->sqlStmtColumnName(sqlStatement,
          column);
    } /* if */
    logFunction(printf("sqlStmtColumnName --> \"%s\"\n",
                       striAsUnquotedCStri(name)););
    return name;
  } /* sqlStmtColumnName */
