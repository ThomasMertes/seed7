/********************************************************************/
/*                                                                  */
/*  sql_rtl.c     Database access functions.                        */
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
/*  File: seed7/src/sql_rtl.c                                       */
/*  Changes: 2014, 2015, 2017 - 2020  Thomas Mertes                 */
/*  Content: Database access functions.                             */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "float.h"

#include "common.h"
#include "data_rtl.h"
#include "striutl.h"
#include "flt_rtl.h"
#include "big_drv.h"
#include "rtl_err.h"
#include "sql_base.h"
#include "sql_drv.h"

#undef EXTERN
#define EXTERN
#include "sql_rtl.h"


#define ENCODE_INFINITY 0
#define ENCODE_NAN 0


typedef struct dbStruct {
    uintType     usage_count;
    boolType     isOpen;
    sqlFuncType  sqlFunc;
    intType      driver;
  } dbRecord, *dbType;

typedef struct preparedStmtStruct {
    uintType     usage_count;
    dbType       db;
    sqlFuncType  sqlFunc;
  } preparedStmtRecord, *preparedStmtType;



/**
 *  Bind a bigInteger parameter to a prepared SQL statement.
 *  @param sqlStatement Prepared statement.
 *  @param pos Position of the bind variable (starting with 1).
 *  @param value Value for the binding.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if 'pos' is negative or too big or
 *                         if 'value' cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlBindBigInt (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType value)

  { /* sqlBindBigInt */
    logFunction(printf("sqlBindBigInt(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos,
                       bigHexCStri(value)););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlBindBigInt(" FMT_U_MEM ", " FMT_D ", %s): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, pos,
                      bigHexCStri(value)););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlBindBigInt");
      logError(printf("sqlBindBigInt(" FMT_U_MEM ", " FMT_D ", %s): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, pos,
                      bigHexCStri(value)););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBigInt == NULL)) {
      dbNoFuncPtr("sqlBindBigInt");
      logError(printf("sqlBindBigInt(" FMT_U_MEM ", " FMT_D ", %s): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, pos,
                      bigHexCStri(value)););
      raise_error(DATABASE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBigInt(sqlStatement, pos, value);
    } /* if */
  } /* sqlBindBigInt */



/**
 *  Bind a bigRational parameter to a prepared SQL statement.
 *  @param sqlStatement Prepared statement.
 *  @param pos Position of the bind variable (starting with 1).
 *  @param numerator Numerator of the Value for the binding.
 *  @param denominator Denominator of the Value for the binding.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if 'pos' is negative or too big or
 *                         if the big rational cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType numerator, const const_bigIntType denominator)

  { /* sqlBindBigRat */
    logFunction(printf("sqlBindBigRat(" FMT_U_MEM ", " FMT_D ", %s, ",
                       (memSizeType) sqlStatement, pos,
                       bigHexCStri(numerator));
                printf("%s)\n", bigHexCStri(denominator)););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlBindBigRat(" FMT_U_MEM ", " FMT_D ", %s, ",
                      (memSizeType) sqlStatement, pos,
                      bigHexCStri(numerator));
               printf("%s): SQL statement is empty.\n",
                      bigHexCStri(denominator)););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlBindBigRat");
      logError(printf("sqlBindBigRat(" FMT_U_MEM ", " FMT_D ", %s, ",
                      (memSizeType) sqlStatement, pos,
                      bigHexCStri(numerator));
               printf("%s): Database is not open.\n",
                      bigHexCStri(denominator)););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBigRat == NULL)) {
      dbNoFuncPtr("sqlBindBigRat");
      logError(printf("sqlBindBigRat(" FMT_U_MEM ", " FMT_D ", %s, ",
                      (memSizeType) sqlStatement, pos,
                      bigHexCStri(numerator));
               printf("%s): Function pointer missing.\n",
                      bigHexCStri(denominator)););
      raise_error(DATABASE_ERROR);
#if !ENCODE_INFINITY && !ENCODE_NAN
    } else if (unlikely(bigEqSignedDigit(denominator, 0))) {
      raise_error(RANGE_ERROR);
#else
#if !ENCODE_INFINITY
    } else if (unlikely(bigEqSignedDigit(denominator, 0) && !bigEqSignedDigit(numerator, 0))) {
      raise_error(RANGE_ERROR);
#endif
#if !ENCODE_NAN
    } else if (unlikely(bigEqSignedDigit(denominator, 0) && bigEqSignedDigit(numerator, 0))) {
      raise_error(RANGE_ERROR);
#endif
#endif
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBigRat(sqlStatement, pos,
                                                                numerator, denominator);
    } /* if */
  } /* sqlBindBigRat */



/**
 *  Bind a boolean parameter to a prepared SQL statement.
 *  @param sqlStatement Prepared statement.
 *  @param pos Position of the bind variable (starting with 1).
 *  @param value Value for the binding.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if 'pos' is negative or too big or
 *                         if 'value' cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlBindBool (sqlStmtType sqlStatement, intType pos, boolType value)

  { /* sqlBindBool */
    logFunction(printf("sqlBindBool(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos,
                       value ? "TRUE" : "FALSE"););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlBindBool(" FMT_U_MEM ", " FMT_D ", %s): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, pos,
                      value ? "TRUE" : "FALSE"););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlBindBool");
      logError(printf("sqlBindBool(" FMT_U_MEM ", " FMT_D ", %s): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, pos,
                      value ? "TRUE" : "FALSE"););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBool == NULL)) {
      dbNoFuncPtr("sqlBindBool");
      logError(printf("sqlBindBool(" FMT_U_MEM ", " FMT_D ", %s): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, pos,
                      value ? "TRUE" : "FALSE"););
      raise_error(DATABASE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBool(sqlStatement, pos, value);
    } /* if */
  } /* sqlBindBool */



/**
 *  Bind a bstring parameter to a prepared SQL statement.
 *  @param sqlStatement Prepared statement.
 *  @param pos Position of the bind variable (starting with 1).
 *  @param bstri Value for the binding.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if 'pos' is negative or too big or
 *                         if 'bstri' cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlBindBStri (sqlStmtType sqlStatement, intType pos,
    const const_bstriType bstri)

  { /* sqlBindBStri */
    logFunction(printf("sqlBindBStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos,
                       bstriAsUnquotedCStri(bstri)););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlBindBStri(" FMT_U_MEM ", " FMT_D ", \"%s\"): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, pos,
                      bstriAsUnquotedCStri(bstri)););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlBindBStri");
      logError(printf("sqlBindBStri(" FMT_U_MEM ", " FMT_D ", \"%s\"): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, pos,
                      bstriAsUnquotedCStri(bstri)););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBStri == NULL)) {
      dbNoFuncPtr("sqlBindBStri");
      logError(printf("sqlBindBStri(" FMT_U_MEM ", " FMT_D ", \"%s\"): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, pos,
                      bstriAsUnquotedCStri(bstri)););
      raise_error(DATABASE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindBStri(sqlStatement, pos, bstri);
    } /* if */
  } /* sqlBindBStri */



/**
 *  Bind a duration parameter to a prepared SQL statement.
 *  @param sqlStatement Prepared statement.
 *  @param pos Position of the bind variable (starting with 1).
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if 'pos' is negative or too big or
 *                         if the duration cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
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
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlBindDuration(" FMT_U_MEM ", " FMT_D ", "
                      F_D(04) "-" F_D(02) "-" F_D(02) " "
                      F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, pos,
                      year, month, day,
                      hour, minute, second, micro_second););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlBindDuration");
      logError(printf("sqlBindDuration(" FMT_U_MEM ", " FMT_D ", "
                      F_D(04) "-" F_D(02) "-" F_D(02) " "
                      F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, pos,
                      year, month, day,
                      hour, minute, second, micro_second););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindDuration == NULL)) {
      dbNoFuncPtr("sqlBindDuration");
      logError(printf("sqlBindDuration(" FMT_U_MEM ", " FMT_D ", "
                      F_D(04) "-" F_D(02) "-" F_D(02) " "
                      F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, pos,
                      year, month, day,
                      hour, minute, second, micro_second););
      raise_error(DATABASE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindDuration(sqlStatement, pos,
          year, month, day, hour, minute, second, micro_second);
    } /* if */
  } /* sqlBindDuration */



/**
 *  Bind a float parameter to a prepared SQL statement.
 *  @param sqlStatement Prepared statement.
 *  @param pos Position of the bind variable (starting with 1).
 *  @param value Value for the binding.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if 'pos' is negative or too big or
 *                         if 'value' cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value)

  { /* sqlBindFloat */
    logFunction(printf("sqlBindFloat(" FMT_U_MEM ", " FMT_D ", " FMT_E ")\n",
                       (memSizeType) sqlStatement, pos, value););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlBindFloat(" FMT_U_MEM ", " FMT_D ", " FMT_E "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, pos, value););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlBindFloat");
      logError(printf("sqlBindFloat(" FMT_U_MEM ", " FMT_D ", " FMT_E "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, pos, value););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindFloat == NULL)) {
      dbNoFuncPtr("sqlBindFloat");
      logError(printf("sqlBindFloat(" FMT_U_MEM ", " FMT_D ", " FMT_E "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, pos, value););
      raise_error(DATABASE_ERROR);
#if !ENCODE_INFINITY
    } else if (unlikely(value == POSITIVE_INFINITY || value == NEGATIVE_INFINITY)) {
      raise_error(RANGE_ERROR);
#endif
#if !ENCODE_NAN
    } else if (unlikely(os_isnan(value))) {
      raise_error(RANGE_ERROR);
#endif
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindFloat(sqlStatement, pos, value);
    } /* if */
  } /* sqlBindFloat */



/**
 *  Bind an integer parameter to a prepared SQL statement.
 *  @param sqlStatement Prepared statement.
 *  @param pos Position of the bind variable (starting with 1).
 *  @param value Value for the binding.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if 'pos' is negative or too big or
 *                         if 'value' cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value)

  { /* sqlBindInt */
    logFunction(printf("sqlBindInt(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos, value););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlBindInt(" FMT_U_MEM ", " FMT_D ", " FMT_D "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, pos, value););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlBindInt");
      logError(printf("sqlBindInt(" FMT_U_MEM ", " FMT_D ", " FMT_D "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, pos, value););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindInt == NULL)) {
      dbNoFuncPtr("sqlBindInt");
      logError(printf("sqlBindInt(" FMT_U_MEM ", " FMT_D ", " FMT_D "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, pos, value););
      raise_error(DATABASE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindInt(sqlStatement, pos, value);
    } /* if */
  } /* sqlBindInt */



/**
 *  Bind a NULL parameter to a prepared SQL statement.
 *  @param sqlStatement Prepared statement.
 *  @param pos Position of the bind variable (starting with 1).
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if 'pos' is negative or too big.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlBindNull (sqlStmtType sqlStatement, intType pos)

  { /* sqlBindNull */
    logFunction(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, pos););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlBindNull");
      logError(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, pos););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                       ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindNull == NULL)) {
      dbNoFuncPtr("sqlBindNull");
      logError(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, pos););
      raise_error(DATABASE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindNull(sqlStatement, pos);
    } /* if */
  } /* sqlBindNull */



/**
 *  Bind a string parameter to a prepared SQL statement.
 *  @param sqlStatement Prepared statement.
 *  @param pos Position of the bind variable (starting with 1).
 *  @param stri Value for the binding.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if 'pos' is negative or too big or
 *                         if 'stri' cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlBindStri (sqlStmtType sqlStatement, intType pos,
    const const_striType stri)

  { /* sqlBindStri */
    logFunction(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos,
                       striAsUnquotedCStri(stri)););
    if (unlikely(sqlStatement == NULL)){
      logError(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\"): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, pos,
                      striAsUnquotedCStri(stri)););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlBindStri");
      logError(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\"): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, pos,
                      striAsUnquotedCStri(stri)););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindStri == NULL)) {
      dbNoFuncPtr("sqlBindStri");
      logError(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\"): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, pos,
                      striAsUnquotedCStri(stri)););
      raise_error(DATABASE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindStri(sqlStatement, pos, stri);
    } /* if */
  } /* sqlBindStri */



/**
 *  Bind a time parameter to a prepared SQL statement.
 *  @param sqlStatement Prepared statement.
 *  @param pos Position of the bind variable (starting with 1).
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if 'pos' is negative or too big or
 *                         if the time cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlBindTime (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second,
    intType time_zone)

  { /* sqlBindTime */
    logFunction(printf("sqlBindTime(" FMT_U_MEM ", " FMT_D ", "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) ", "
                       FMT_D ")\n",
                       (memSizeType) sqlStatement, pos,
                       year, month, day,
                       hour, minute, second, micro_second,
                       time_zone););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlBindTime(" FMT_U_MEM ", " FMT_D ", "
                      F_D(04) "-" F_D(02) "-" F_D(02) " "
                      F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) ", "
                      FMT_D "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, pos,
                      year, month, day,
                      hour, minute, second, micro_second,
                      time_zone););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlBindTime");
      logError(printf("sqlBindTime(" FMT_U_MEM ", " FMT_D ", "
                      F_D(04) "-" F_D(02) "-" F_D(02) " "
                      F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) ", "
                      FMT_D "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, pos,
                      year, month, day,
                      hour, minute, second, micro_second,
                      time_zone););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindTime == NULL)) {
      dbNoFuncPtr("sqlBindTime");
      logError(printf("sqlBindTime(" FMT_U_MEM ", " FMT_D ", "
                      F_D(04) "-" F_D(02) "-" F_D(02) " "
                      F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) ", "
                      FMT_D "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, pos,
                      year, month, day,
                      hour, minute, second, micro_second,
                      time_zone););
      raise_error(DATABASE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlBindTime(sqlStatement, pos,
          year, month, day, hour, minute, second, micro_second, time_zone);
    } /* if */
  } /* sqlBindTime */



/**
 *  Close the specified database 'database'.
 *  @param database Database to be closed.
 *  @exception RANGE_ERROR If the database was not open.
 */
void sqlClose (databaseType database)

  { /* sqlClose */
    logFunction(printf("sqlClose(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) database,
                       database != NULL ? database->usage_count
                                        : (uintType) 0););
    if (unlikely(database == NULL)) {
      logError(printf("sqlClose(" FMT_U_MEM "): "
                      "Database is empty.\n",
                      (memSizeType) database););
      raise_error(RANGE_ERROR);
    } else if (unlikely(!database->isOpen)) {
      dbNotOpen("sqlClose");
      logError(printf("sqlClose(" FMT_U_MEM "): "
                      "Database is not open.\n",
                      (memSizeType) database););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((dbType) database)->sqlFunc == NULL ||
                        ((dbType) database)->sqlFunc->sqlClose == NULL)) {
      dbNoFuncPtr("sqlClose");
      logError(printf("sqlClose(" FMT_U_MEM "): "
                      "Function pointer missing.\n",
                      (memSizeType) database););
      raise_error(DATABASE_ERROR);
    } else {
      ((dbType) database)->sqlFunc->sqlClose(database);
    } /* if */
    logFunction(printf("sqlClose -->\n"););
  } /* sqlClose */



/**
 *  Get the specified column of fetched data as bigInteger.
 *  If the column data is NULL it is interpreted as 0_.
 *  The function sqlIsNull can distinguish NULL from 0_.
 *  @param sqlStatement Prepared statement for which data was fetched.
 *  @param column Number of the column (starting with 1).
 *  @return the column data converted to a bigInteger, and
 *          0_ if the column data is NULL.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if no data was successfully fetched or
 *                         if the specified column does not exist or
 *                         if the column cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column)

  {
    bigIntType columnValue;

  /* sqlColumnBigInt */
    logFunction(printf("sqlColumnBigInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlColumnBigInt(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlColumnBigInt");
      logError(printf("sqlColumnBigInt(" FMT_U_MEM ", " FMT_D "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      columnValue = NULL;
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBigInt == NULL)) {
      dbNoFuncPtr("sqlColumnBigInt");
      logError(printf("sqlColumnBigInt(" FMT_U_MEM ", " FMT_D "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      columnValue = NULL;
    } else {
      columnValue = ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBigInt(
          sqlStatement, column);
    } /* if */
    logFunction(printf("sqlColumnBigInt --> %s\n", bigHexCStri(columnValue)););
    return columnValue;
  } /* sqlColumnBigInt */



/**
 *  Get the specified column of fetched data as bigRational.
 *  If the column data is NULL it is interpreted as 0_/1_.
 *  The function sqlIsNull can distinguish NULL from 0_/1_.
 *  @param sqlStatement Prepared statement for which data was fetched.
 *  @param column Number of the column (starting with 1).
 *  @param numerator Numerator of the column data.
 *                   Set to 0_ if the column data is NULL
 *  @param denominator Denominator of the column data.
 *                     Set to 1_ if the column data is NULL.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if no data was successfully fetched or
 *                         if the specified column does not exist or
 *                         if the column cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlColumnBigRat (sqlStmtType sqlStatement, intType column,
    bigIntType *numerator, bigIntType *denominator)

  {
    bigIntType oldNumerator;
    bigIntType oldDenominator;

  /* sqlColumnBigRat */
    logFunction(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", *, *)\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", *, *): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlColumnBigRat");
      logError(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", *, *): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBigRat == NULL)) {
      dbNoFuncPtr("sqlColumnBigRat");
      logError(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", *, *): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
    } else {
      oldNumerator = *numerator;
      oldDenominator = *denominator;
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBigRat(sqlStatement, column,
          numerator, denominator);
      if (oldNumerator != *numerator) {
        bigDestr(oldNumerator);
      } /* if */
      if (oldDenominator != *denominator) {
        bigDestr(oldDenominator);
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", %s, ",
                       (memSizeType) sqlStatement, column,
                       bigHexCStri(*numerator));
                printf("%s) -->\n", bigHexCStri(*denominator)););
  } /* sqlColumnBigRat */



/**
 *  Get the specified column of fetched data as boolean.
 *  If the column data is NULL it is interpreted as FALSE.
 *  The function sqlIsNull can distinguish NULL from FALSE.
 *  @param sqlStatement Prepared statement for which data was fetched.
 *  @param column Number of the column (starting with 1).
 *  @return the column data converted to a boolean, and
 *          FALSE if the column data is NULL.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if no data was successfully fetched or
 *                         if the specified column does not exist or
 *                         if the column cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
boolType sqlColumnBool (sqlStmtType sqlStatement, intType column)

  {
    boolType columnValue;

  /* sqlColumnBool */
    logFunction(printf("sqlColumnBool(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlColumnBool(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      columnValue = FALSE;
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlColumnBool");
      logError(printf("sqlColumnBool(" FMT_U_MEM ", " FMT_D "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      columnValue = FALSE;
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBool == NULL)) {
      dbNoFuncPtr("sqlColumnBool");
      logError(printf("sqlColumnBool(" FMT_U_MEM ", " FMT_D "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      columnValue = FALSE;
    } else {
      columnValue = ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBool(
          sqlStatement, column);
    } /* if */
    logFunction(printf("sqlColumnBool --> %s\n", columnValue ? "TRUE" : "FALSE"););
    return columnValue;
  } /* sqlColumnBool */



/**
 *  Get the specified column of fetched data as bstring.
 *  If the column data is NULL it is interpreted as empty bstring.
 *  The function sqlIsNull can distinguish NULL from an empty bstring.
 *  @param sqlStatement Prepared statement for which data was fetched.
 *  @param column Number of the column (starting with 1).
 *  @return the column data converted to a bstring, and
 *          an empty bstring if the column data is NULL.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if no data was successfully fetched or
 *                         if the specified column does not exist or
 *                         if the column cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
bstriType sqlColumnBStri (sqlStmtType sqlStatement, intType column)

  {
    bstriType columnValue;

  /* sqlColumnBStri */
    logFunction(printf("sqlColumnBStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlColumnBStri(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlColumnBStri");
      logError(printf("sqlColumnBStri(" FMT_U_MEM ", " FMT_D "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      columnValue = NULL;
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBStri == NULL)) {
      dbNoFuncPtr("sqlColumnBStri");
      logError(printf("sqlColumnBStri(" FMT_U_MEM ", " FMT_D "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      columnValue = NULL;
    } else {
      columnValue = ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnBStri(
          sqlStatement, column);
    } /* if */
    logFunction(printf("sqlColumnBStri --> \"%s\"\n",
                       bstriAsUnquotedCStri(columnValue)););
    return columnValue;
  } /* sqlColumnBStri */



/**
 *  Get the specified column of fetched data as duration.
 *  If the column data is NULL it is interpreted as empty duration.
 *  The function sqlIsNull can distinguish NULL from an empty duration.
 *  @param sqlStatement Prepared statement for which data was fetched.
 *  @param column Number of the column (starting with 1).
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if no data was successfully fetched or
 *                         if the specified column does not exist or
 *                         if the column cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlColumnDuration (sqlStmtType sqlStatement, intType column,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second)

  { /* sqlColumnDuration */
    logFunction(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", *): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlColumnDuration");
      logError(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", *): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnDuration == NULL)) {
      dbNoFuncPtr("sqlColumnDuration");
      logError(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", *): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
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



/**
 *  Get the specified column of fetched data as float.
 *  If the column data is NULL it is interpreted as 0.0.
 *  The function sqlIsNull can distinguish NULL from 0.0.
 *  @param sqlStatement Prepared statement for which data was fetched.
 *  @param column Number of the column (starting with 1).
 *  @return the column data converted to a float, and
 *          0.0 if the column data is NULL.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if no data was successfully fetched or
 *                         if the specified column does not exist or
 *                         if the column cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
floatType sqlColumnFloat (sqlStmtType sqlStatement, intType column)

  {
    floatType columnValue;

  /* sqlColumnFloat */
    logFunction(printf("sqlColumnFloat(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlColumnFloat(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      columnValue = 0.0;
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlColumnFloat");
      logError(printf("sqlColumnFloat(" FMT_U_MEM ", " FMT_D "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      columnValue = 0.0;
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnFloat == NULL)) {
      dbNoFuncPtr("sqlColumnFloat");
      logError(printf("sqlColumnFloat(" FMT_U_MEM ", " FMT_D "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      columnValue = 0.0;
    } else {
      columnValue = ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnFloat(
          sqlStatement, column);
    } /* if */
    logFunction(printf("sqlColumnFloat --> " FMT_E "\n", columnValue););
    return columnValue;
  } /* sqlColumnFloat */



/**
 *  Get the specified column of fetched data as integer.
 *  If the column data is NULL it is interpreted as 0.
 *  The function sqlIsNull can distinguish NULL from 0.
 *  @param sqlStatement Prepared statement for which data was fetched.
 *  @param column Number of the column (starting with 1).
 *  @return the column data converted to an integer, and
 *          0 if the column data is NULL.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if no data was successfully fetched or
 *                         if the specified column does not exist or
 *                         if the column cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
intType sqlColumnInt (sqlStmtType sqlStatement, intType column)

  {
    intType columnValue;

  /* sqlColumnInt */
    logFunction(printf("sqlColumnInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlColumnInt(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlColumnInt");
      logError(printf("sqlColumnInt(" FMT_U_MEM ", " FMT_D "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      columnValue = 0;
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnInt == NULL)) {
      dbNoFuncPtr("sqlColumnInt");
      logError(printf("sqlColumnInt(" FMT_U_MEM ", " FMT_D "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      columnValue = 0;
    } else {
      columnValue = ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnInt(sqlStatement,
          column);
    } /* if */
    logFunction(printf("sqlColumnInt --> " FMT_D "\n", columnValue););
    return columnValue;
  } /* sqlColumnInt */



/**
 *  Get the specified column of fetched data as string.
 *  If the column data is NULL it is interpreted as "".
 *  The function sqlIsNull can distinguish NULL from "".
 *  @param sqlStatement Prepared statement for which data was fetched.
 *  @param column Number of the column (starting with 1).
 *  @return the column data converted to a string, and
 *          "" if the column data is NULL.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if no data was successfully fetched or
 *                         if the specified column does not exist or
 *                         if the column cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
striType sqlColumnStri (sqlStmtType sqlStatement, intType column)

  {
    striType columnValue;

  /* sqlColumnStri */
    logFunction(printf("sqlColumnStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlColumnStri(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlColumnStri");
      logError(printf("sqlColumnStri(" FMT_U_MEM ", " FMT_D "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      columnValue = NULL;
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnStri == NULL)) {
      dbNoFuncPtr("sqlColumnStri");
      logError(printf("sqlColumnStri(" FMT_U_MEM ", " FMT_D "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      columnValue = NULL;
    } else {
      columnValue = ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnStri(
          sqlStatement, column);
    } /* if */
    logFunction(printf("sqlColumnStri --> \"%s\"\n", striAsUnquotedCStri(columnValue)););
    return columnValue;
  } /* sqlColumnStri */



/**
 *  Get the specified column of fetched data as time.
 *  If the column data is NULL it is interpreted as 0-01-01 00:00:00.
 *  The function sqlIsNull can distinguish NULL from 0-01-01 00:00:00.
 *  @param sqlStatement Prepared statement for which data was fetched.
 *  @param column Number of the column (starting with 1).
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if no data was successfully fetched or
 *                         if the specified column does not exist or
 *                         if the column cannot be converted.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlColumnTime (sqlStmtType sqlStatement, intType column,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second,
    intType *time_zone, boolType *is_dst)

  { /* sqlColumnTime */
    logFunction(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ", *): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlColumnTime");
      logError(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ", *): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlColumnTime == NULL)) {
      dbNoFuncPtr("sqlColumnTime");
      logError(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ", *): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
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



/**
 *  Execute a commit statement for the specified database 'database'.
 */
void sqlCommit (databaseType database)

  { /* sqlCommit */
    logFunction(printf("sqlCommit(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    if (unlikely(database == NULL)) {
      logError(printf("sqlCommit(" FMT_U_MEM "): "
                      "Database is empty.\n",
                      (memSizeType) database););
      raise_error(RANGE_ERROR);
    } else if (unlikely(!database->isOpen)) {
      dbNotOpen("sqlCommit");
      logError(printf("sqlCommit(" FMT_U_MEM "): "
                      "Database is not open.\n",
                      (memSizeType) database););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((dbType) database)->sqlFunc == NULL ||
                        ((dbType) database)->sqlFunc->sqlCommit == NULL)) {
      dbNoFuncPtr("sqlCommit");
      logError(printf("sqlCommit(" FMT_U_MEM "): "
                      "Function pointer missing.\n",
                      (memSizeType) database););
      raise_error(DATABASE_ERROR);
    } else {
      ((dbType) database)->sqlFunc->sqlCommit(database);
    } /* if */
    logFunction(printf("sqlCommit -->\n"););
  } /* sqlCommit */



/**
 *  Assign source to *dest.
 *  A copy function assumes that *dest contains a legal value.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void sqlCpyDb (databaseType *const dest, const databaseType source)

  {
    dbType db_dest;
    dbType db_source;

  /* sqlCpyDb */
    logFunction(printf("sqlCpyDb(%s" FMT_U_MEM " (usage=" FMT_U "), "
                       FMT_U_MEM " (usage=" FMT_U "))\n",
                       dest != NULL ? "" : "NULL ",
                       (memSizeType) (dest != NULL ? *dest : NULL),
                       dest != NULL && *dest != NULL ?
                           (*dest)->usage_count : (uintType) 0,
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    db_dest = (dbType) *dest;
    db_source = (dbType) source;
    if (db_source != NULL && db_source->usage_count != 0) {
      db_source->usage_count++;
    } /* if */
    if (db_dest != NULL && db_dest->usage_count != 0) {
      db_dest->usage_count--;
      if (db_dest->usage_count == 0 &&
          db_dest->sqlFunc != NULL &&
          db_dest->sqlFunc->freeDatabase != NULL) {
        logMessage(printf("FREE " FMT_U_MEM "\n", (memSizeType) db_dest););
        db_dest->sqlFunc->freeDatabase((databaseType) db_dest);
      } /* if */
    } /* if */
    *dest = (databaseType) db_source;
    logFunction(printf("sqlCpyDb(" FMT_U_MEM " (usage=" FMT_U "), "
                       FMT_U_MEM " (usage=" FMT_U ")) -->\n",
                       (memSizeType) *dest,
                       *dest != NULL ? (*dest)->usage_count : (uintType) 0,
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
  } /* sqlCpyDb */



/**
 *  Reinterpret the generic parameters as databaseType and call sqlCpyDb.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(databaseType).
 */
void sqlCpyDbGeneric (genericType *const dest, const genericType source)

  { /* sqlCpyDbGeneric */
    sqlCpyDb(&((rtlObjectType *) dest)->value.databaseValue,
             ((const_rtlObjectType *) &source)->value.databaseValue);
  } /* sqlCpyDbGeneric */



/**
 *  Assign source to *dest.
 *  A copy function assumes that *dest contains a legal value.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void sqlCpyStmt (sqlStmtType *const dest, const sqlStmtType source)

  {
    preparedStmtType statement_dest;
    preparedStmtType statement_source;

  /* sqlCpyStmt */
    logFunction(printf("sqlCpyStmt(%s" FMT_U_MEM " (usage=" FMT_U "), "
                       FMT_U_MEM " (usage=" FMT_U "))\n",
                       dest != NULL ? "" : "NULL ",
                       (memSizeType) (dest != NULL ? *dest : NULL),
                       dest != NULL && *dest != NULL ?
                           (*dest)->usage_count : (uintType) 0,
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    statement_dest = (preparedStmtType) *dest;
    statement_source = (preparedStmtType) source;
    if (statement_source != NULL && statement_source->usage_count != 0) {
      statement_source->usage_count++;
    } /* if */
    if (statement_dest != NULL && statement_dest->usage_count != 0) {
      statement_dest->usage_count--;
      if (statement_dest->usage_count == 0 &&
          statement_dest->sqlFunc != NULL &&
          statement_dest->sqlFunc->freePreparedStmt != NULL) {
        logMessage(printf("FREE " FMT_U_MEM "\n", (memSizeType) statement_dest););
        statement_dest->sqlFunc->freePreparedStmt((sqlStmtType) statement_dest);
      } /* if */
    } /* if */
    *dest = (sqlStmtType) statement_source;
    logFunction(printf("sqlCpyStmt(" FMT_U_MEM " (usage=" FMT_U "), "
                       FMT_U_MEM " (usage=" FMT_U ")) -->\n",
                       (memSizeType) *dest,
                       *dest != NULL ? (*dest)->usage_count : (uintType) 0,
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
  } /* sqlCpyStmt */



/**
 *  Reinterpret the generic parameters as sqlStmtType and call sqlCpyStmt.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(sqlStmtType).
 */
void sqlCpyStmtGeneric (genericType *const dest, const genericType source)

  { /* sqlCpyStmtGeneric */
    sqlCpyStmt(&((rtlObjectType *) dest)->value.sqlStmtValue,
               ((const_rtlObjectType *) &source)->value.sqlStmtValue);
  } /* sqlCpyStmtGeneric */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 */
databaseType sqlCreateDb (const databaseType source)

  { /* sqlCreateDb */
    logFunction(printf("sqlCreateDb(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    if (source != NULL && source->usage_count != 0) {
      source->usage_count++;
    } /* if */
    logFunction(printf("sqlCreateDb --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    return source;
  } /* sqlCreateDb */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(databaseType).
 */
genericType sqlCreateDbGeneric (const genericType from_value)

  {
    rtlObjectType result;

  /* sqlCreateDbGeneric */
    INIT_GENERIC_PTR(result.value.genericValue);
    result.value.databaseValue =
        sqlCreateDb(((const_rtlObjectType *) &from_value)->value.databaseValue);
    return result.value.genericValue;
  } /* sqlCreateDbGeneric */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 */
sqlStmtType sqlCreateStmt (const sqlStmtType source)

  { /* sqlCreateStmt */
    logFunction(printf("sqlCreateStmt(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    if (source != NULL && source->usage_count != 0) {
      source->usage_count++;
    } /* if */
    logFunction(printf("sqlCreateStmt --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    return source;
  } /* sqlCreateStmt */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(sqlStmtType).
 */
genericType sqlCreateStmtGeneric (const genericType from_value)

  {
    rtlObjectType result;

  /* sqlCreateStmtGeneric */
    INIT_GENERIC_PTR(result.value.genericValue);
    result.value.sqlStmtValue =
        sqlCreateStmt(((const_rtlObjectType *) &from_value)->value.sqlStmtValue);
    return result.value.genericValue;
  } /* sqlCreateStmtGeneric */



/**
 *  Maintain a usage count and free an unused database 'old_db'.
 *  After a database is freed 'old_db' refers to not existing memory.
 *  The memory where 'old_db' is stored can be freed after sqlDestrDb.
 */
void sqlDestrDb (const databaseType old_db)

  {
    dbType old_database;

  /* sqlDestrDb */
    logFunction(printf("sqlDestrDb(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) old_db,
                       old_db != NULL ? old_db->usage_count : (uintType) 0););
    old_database = (dbType) old_db;
    if (old_database != NULL && old_database->usage_count != 0) {
      old_database->usage_count--;
      if (old_database->usage_count == 0 &&
          old_database->sqlFunc != NULL &&
          old_database->sqlFunc->freeDatabase != NULL) {
        logMessage(printf("FREE " FMT_U_MEM "\n", (memSizeType) old_database););
        old_database->sqlFunc->freeDatabase((databaseType) old_database);
        old_database = NULL;
      } /* if */
    } /* if */
    logFunction(printf("sqlDestrDb(" FMT_U_MEM " (usage=" FMT_U ")) -->\n",
                       (memSizeType) old_database,
                       old_database != NULL ?
                           old_database->usage_count : (uintType) 0););
  } /* sqlDestrDb */



/**
 *  Generic Destr function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(databaseType).
 */
void sqlDestrDbGeneric (const genericType old_value)

  { /* sqlDestrDbGeneric */
    sqlDestrDb(((const_rtlObjectType *) &old_value)->value.databaseValue);
  } /* sqlDestrDbGeneric */



/**
 *  Maintain a usage count and free an unused statement 'old_stmt'.
 *  After a statement is freed 'old_stmt' refers to not existing memory.
 *  The memory where 'old_stmt' is stored can be freed after sqlDestrStmt.
 */
void sqlDestrStmt (const sqlStmtType old_stmt)

  {
    preparedStmtType old_statement;

  /* sqlDestrStmt */
    logFunction(printf("sqlDestrStmt(" FMT_U_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) old_stmt,
                       old_stmt != NULL ? old_stmt->usage_count : (uintType) 0););
    old_statement = (preparedStmtType) old_stmt;
    if (old_statement != NULL && old_statement->usage_count != 0) {
      old_statement->usage_count--;
      if (old_statement->usage_count == 0 &&
          old_statement->sqlFunc != NULL &&
          old_statement->sqlFunc->freePreparedStmt != NULL) {
        logMessage(printf("FREE " FMT_U_MEM "\n", (memSizeType) old_statement););
        old_statement->sqlFunc->freePreparedStmt((sqlStmtType) old_statement);
        old_statement = NULL;
      } /* if */
    } /* if */
    logFunction(printf("sqlDestrStmt(" FMT_U_MEM " (usage=" FMT_U ")) -->\n",
                       (memSizeType) old_statement,
                       old_statement != NULL ?
                           old_statement->usage_count : (uintType) 0););
  } /* sqlDestrStmt */



/**
 *  Generic Destr function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(sqlStmtType).
 */
void sqlDestrStmtGeneric (const genericType old_value)

  { /* sqlDestrStmtGeneric */
    sqlDestrStmt(((const_rtlObjectType *) &old_value)->value.sqlStmtValue);
  } /* sqlDestrStmtGeneric */



intType sqlDriver (databaseType database)

  {
    intType driver;

  /* sqlDriver */
    logFunction(printf("sqlDriver(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    if (unlikely(database == NULL)) {
      logError(printf("sqlDriver(" FMT_U_MEM "): "
                      "Database is empty.\n",
                      (memSizeType) database););
      raise_error(RANGE_ERROR);
      driver = 0;
    } else {
      driver = ((dbType) database)->driver;
    } /* if */
    logFunction(printf("sqlDriver --> " FMT_D "\n", driver););
    return driver;
  } /* sqlDriver */



intType sqlErrCode (void)

  { /* sqlErrCode */
    return dbError.errorCode;
  } /* sqlErrCode */



striType sqlErrDbFunc (void)

  {
    striType dbFuncName;

  /* sqlErrDbFunc */
    dbFuncName = cstri_to_stri(dbError.dbFuncName);
    if (unlikely(dbFuncName == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return dbFuncName;
  } /* sqlErrDbFunc */



striType sqlErrLibFunc (void)

  {
    striType funcName;

  /* sqlErrLibFunc */
    funcName = cstri_to_stri(dbError.funcName);
    if (unlikely(funcName == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return funcName;
  } /* sqlErrLibFunc */



striType sqlErrMessage (void)

  {
    striType message;

  /* sqlErrMessage */
    message = cstri8_or_cstri_to_stri(dbError.message);
    if (unlikely(message == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return message;
  } /* sqlErrMessage */



/**
 *  Execute the specified prepared SQL statement.
 *  Bind variable can be assigned with bind functions before
 *  sqlExecute is called.
 *  @param sqlStatement Prepared statement, which should be executed.
 *  @exception DATABASE_ERROR If a database function fails.
 */
void sqlExecute (sqlStmtType sqlStatement)

  { /* sqlExecute */
    logFunction(printf("sqlExecute(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    if (sqlStatement == NULL) {
      /* Do nothing */
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlExecute == NULL)) {
      dbNoFuncPtr("sqlExecute");
      logError(printf("sqlExecute(" FMT_U_MEM "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement););
      raise_error(DATABASE_ERROR);
    } else {
      ((preparedStmtType) sqlStatement)->sqlFunc->sqlExecute(sqlStatement);
    } /* if */
    logFunction(printf("sqlExecute -->\n"););
  } /* sqlExecute */



/**
 *  Fetch a row from the result data of an executed 'sqlStatement'.
 *  After the 'sqlStatement' has been executed successfully the
 *  function sqlFetch can be used to get the first and further
 *  rows of the statements result data. The columns of the
 *  result data can be obtained with the column functions.
 *  @param sqlStatement Prepared statement, which has been executed.
 *  @return TRUE if a row of result data could be fetched successfully.
 *          FALSE if no more result data is available.
 *  @exception DATABASE_ERROR If a database function fails.
 */
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
      dbNoFuncPtr("sqlFetch");
      logError(printf("sqlFetch(" FMT_U_MEM "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement););
      raise_error(DATABASE_ERROR);
      fetchOkay = FALSE;
    } else {
      fetchOkay = ((preparedStmtType) sqlStatement)->sqlFunc->sqlFetch(sqlStatement);
    } /* if */
    logFunction(printf("sqlFetch --> %d\n", fetchOkay););
    return fetchOkay;
  } /* sqlFetch */



/**
 *  Get the current auto-commit mode for the specified database 'database'.
 */
boolType sqlGetAutoCommit (databaseType database)

  {
    boolType autoCommit;

  /* sqlGetAutoCommit */
    logFunction(printf("sqlGetAutoCommit(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    if (unlikely(database == NULL)) {
      logError(printf("sqlGetAutoCommit(" FMT_U_MEM "): "
                      "Database is empty.\n",
                      (memSizeType) database););
      raise_error(RANGE_ERROR);
      autoCommit = FALSE;
    } else if (unlikely(!database->isOpen)) {
      dbNotOpen("sqlGetAutoCommit");
      logError(printf("sqlGetAutoCommit(" FMT_U_MEM "): "
                      "Database is not open.\n",
                      (memSizeType) database););
      raise_error(DATABASE_ERROR);
      autoCommit = FALSE;
    } else if (unlikely(((dbType) database)->sqlFunc == NULL ||
                        ((dbType) database)->sqlFunc->sqlGetAutoCommit == NULL)) {
      dbNoFuncPtr("sqlGetAutoCommit");
      logError(printf("sqlGetAutoCommit(" FMT_U_MEM "): "
                      "Function pointer missing.\n",
                      (memSizeType) database););
      raise_error(DATABASE_ERROR);
      autoCommit = FALSE;
    } else {
      autoCommit = ((dbType) database)->sqlFunc->sqlGetAutoCommit(database);
    } /* if */
    logFunction(printf("sqlGetAutoCommit --> %d\n", autoCommit););
    return autoCommit;
  } /* sqlGetAutoCommit */



/**
 *  Determine if the specified column of fetched data is NULL.
 *  @param sqlStatement Prepared statement for which data was fetched.
 *  @param column Number of the column (starting with 1).
 *  @return TRUE if the column data is NULL,
 *          FALSE otherwise.
 *  @exception RANGE_ERROR If the statement was not prepared or
 *                         if no data was successfully fetched or
 *                         if the specified column does not exist.
 */
boolType sqlIsNull (sqlStmtType sqlStatement, intType column)

  {
    boolType isNull;

  /* sqlIsNull */
    logFunction(printf("sqlIsNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlIsNull(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      isNull = FALSE;
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlIsNull");
      logError(printf("sqlIsNull(" FMT_U_MEM ", " FMT_D "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      isNull = FALSE;
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlIsNull == NULL)) {
      dbNoFuncPtr("sqlIsNull");
      logError(printf("sqlIsNull(" FMT_U_MEM ", " FMT_D "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      isNull = FALSE;
    } else {
      isNull = ((preparedStmtType) sqlStatement)->sqlFunc->sqlIsNull(sqlStatement,
          column);
    } /* if */
    logFunction(printf("sqlIsNull --> %s\n", isNull ? "TRUE" : "FALSE"););
    return isNull;
  } /* sqlIsNull */



/**
 *  Create a prepared statement for the given 'database'.
 *  @param database Database connection for which the prepared
 *         statement should be created.
 *  @param sqlStatementStri SQL statement in a string. For bind variables
 *         use a question mark (?).
 *  @exception RANGE_ERROR If the database is not open.
 */
sqlStmtType sqlPrepare (databaseType database,
    const const_striType sqlStatementStri)

  {
    sqlStmtType preparedStmt;

  /* sqlPrepare */
    logFunction(printf("sqlPrepare(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) database,
                       striAsUnquotedCStri(sqlStatementStri)););
    if (unlikely(database == NULL)) {
      logError(printf("sqlPrepare(" FMT_U_MEM ", \"%s\"): "
                      "Database is empty.\n",
                      (memSizeType) database,
                      striAsUnquotedCStri(sqlStatementStri)););
      raise_error(RANGE_ERROR);
      preparedStmt = NULL;
    } else if (unlikely(!database->isOpen)) {
      dbNotOpen("sqlPrepare");
      logError(printf("sqlPrepare(" FMT_U_MEM ", \"%s\"): "
                      "Database is not open.\n",
                      (memSizeType) database,
                      striAsUnquotedCStri(sqlStatementStri)););
      raise_error(DATABASE_ERROR);
      preparedStmt = NULL;
    } else if (unlikely(((dbType) database)->sqlFunc == NULL ||
                        ((dbType) database)->sqlFunc->sqlPrepare == NULL)) {
      dbNoFuncPtr("sqlPrepare");
      logError(printf("sqlPrepare(" FMT_U_MEM ", \"%s\"): "
                      "Function pointer missing.\n",
                      (memSizeType) database,
                      striAsUnquotedCStri(sqlStatementStri)););
      raise_error(DATABASE_ERROR);
      preparedStmt = NULL;
    } else {
      preparedStmt = ((dbType) database)->sqlFunc->sqlPrepare(database, sqlStatementStri);
    } /* if */
    logFunction(printf("sqlPrepare --> " FMT_U_MEM "\n",
                       (memSizeType) preparedStmt););
    return preparedStmt;
  } /* sqlPrepare */



/**
 *  Execute a rollback statement for the specified database 'database'.
 */
void sqlRollback (databaseType database)

  { /* sqlRollback */
    logFunction(printf("sqlRollback(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    if (unlikely(database == NULL)) {
      logError(printf("sqlRollback(" FMT_U_MEM "): "
                      "Database is empty.\n",
                      (memSizeType) database););
      raise_error(RANGE_ERROR);
    } else if (unlikely(!database->isOpen)) {
      dbNotOpen("sqlRollback");
      logError(printf("sqlRollback(" FMT_U_MEM "): "
                      "Database is not open.\n",
                      (memSizeType) database););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((dbType) database)->sqlFunc == NULL ||
                        ((dbType) database)->sqlFunc->sqlRollback == NULL)) {
      dbNoFuncPtr("sqlRollback");
      logError(printf("sqlRollback(" FMT_U_MEM "): "
                      "Function pointer missing.\n",
                      (memSizeType) database););
      raise_error(DATABASE_ERROR);
    } else {
      ((dbType) database)->sqlFunc->sqlRollback(database);
    } /* if */
    logFunction(printf("sqlRollback -->\n"););
  } /* sqlRollback */



/**
 *  Set the auto-commit mode for the specified database 'database'.
 */
void sqlSetAutoCommit (databaseType database, boolType autoCommit)

  { /* sqlSetAutoCommit */
    logFunction(printf("sqlSetAutoCommit(" FMT_U_MEM ", %d)\n",
                       (memSizeType) database, autoCommit););
    if (unlikely(database == NULL)) {
      logError(printf("sqlSetAutoCommit(" FMT_U_MEM ", %d): "
                      "Database is empty.\n",
                      (memSizeType) database, autoCommit););
      raise_error(RANGE_ERROR);
    } else if (unlikely(!database->isOpen)) {
      dbNotOpen("sqlSetAutoCommit");
      logError(printf("sqlSetAutoCommit(" FMT_U_MEM ", %d): "
                      "Database is not open.\n",
                      (memSizeType) database, autoCommit););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((dbType) database)->sqlFunc == NULL ||
                        ((dbType) database)->sqlFunc->sqlSetAutoCommit == NULL)) {
      dbNoFuncPtr("sqlSetAutoCommit");
      logError(printf("sqlSetAutoCommit(" FMT_U_MEM ", %d): "
                      "Function pointer missing.\n",
                      (memSizeType) database, autoCommit););
      raise_error(DATABASE_ERROR);
    } else {
      ((dbType) database)->sqlFunc->sqlSetAutoCommit(database, autoCommit);
    } /* if */
    logFunction(printf("sqlSetAutoCommit -->\n"););
  } /* sqlSetAutoCommit */



/**
 *  Return the number of columns in the result data of a ''statement''.
 *  It is not necessary to ''execute'' the prepared statement, before
 *  ''columnCount'' is called.
 *  @param statement Prepared statement.
 */
intType sqlStmtColumnCount (sqlStmtType sqlStatement)

  {
    intType columnCount;

  /* sqlStmtColumnCount */
    logFunction(printf("sqlStmtColumnCount(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlStmtColumnCount(" FMT_U_MEM "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement););
      raise_error(RANGE_ERROR);
      columnCount = 0;
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlStmtColumnCount");
      logError(printf("sqlStmtColumnCount(" FMT_U_MEM "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlStmtColumnCount == NULL)) {
      dbNoFuncPtr("sqlStmtColumnCount");
      logError(printf("sqlStmtColumnCount(" FMT_U_MEM "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement););
      raise_error(DATABASE_ERROR);
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
    if (unlikely(sqlStatement == NULL)) {
      logError(printf("sqlStmtColumnName(" FMT_U_MEM ", " FMT_D "): "
                      "SQL statement is empty.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(RANGE_ERROR);
      name = NULL;
    } else if (unlikely(sqlStatement->db == NULL ||
                        !sqlStatement->db->isOpen)) {
      dbNotOpen("sqlStmtColumnName");
      logError(printf("sqlStmtColumnName(" FMT_U_MEM ", " FMT_D "): "
                      "Database is not open.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      name = NULL;
    } else if (unlikely(((preparedStmtType) sqlStatement)->sqlFunc == NULL ||
                        ((preparedStmtType) sqlStatement)->sqlFunc->sqlStmtColumnName == NULL)) {
      dbNoFuncPtr("sqlStmtColumnName");
      logError(printf("sqlStmtColumnName(" FMT_U_MEM ", " FMT_D "): "
                      "Function pointer missing.\n",
                      (memSizeType) sqlStatement, column););
      raise_error(DATABASE_ERROR);
      name = NULL;
    } else {
      name = ((preparedStmtType) sqlStatement)->sqlFunc->sqlStmtColumnName(sqlStatement,
          column);
    } /* if */
    logFunction(printf("sqlStmtColumnName --> \"%s\"\n",
                       striAsUnquotedCStri(name)););
    return name;
  } /* sqlStmtColumnName */
