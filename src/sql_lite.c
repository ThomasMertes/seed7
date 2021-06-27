/********************************************************************/
/*                                                                  */
/*  sql_lite.c    Database access functions for SQLite.             */
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
/*  File: seed7/src/sql_lite.c                                      */
/*  Changes: 2013, 2014, 2015, 2017 - 2020  Thomas Mertes           */
/*  Content: Database access functions for SQLite.                  */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "limits.h"
#include "float.h"
#ifdef SQLITE_INCLUDE
#include SQLITE_INCLUDE
#endif

#include "common.h"
#include "data_rtl.h"
#include "striutl.h"
#include "heaputl.h"
#include "numutl.h"
#include "flt_rtl.h"
#include "str_rtl.h"
#include "tim_rtl.h"
#include "cmd_rtl.h"
#include "big_drv.h"
#include "rtl_err.h"
#include "dll_drv.h"
#include "sql_base.h"
#include "sql_drv.h"

#ifdef SQLITE_INCLUDE


typedef struct {
    uintType     usage_count;
    sqlFuncType  sqlFunc;
    intType      driver;
    sqlite3     *connection;
  } dbRecord, *dbType;

typedef struct {
    boolType     bound;
  } bindDataRecord, *bindDataType;

typedef struct {
    uintType      usage_count;
    sqlFuncType   sqlFunc;
    sqlite3_stmt *ppStmt;
    memSizeType   param_array_size;
    bindDataType  param_array;
    unsigned int  result_column_count;
    boolType      executeSuccessful;
    boolType      useStoredFetchResult;
    boolType      storedFetchResult;
    boolType      fetchOkay;
    boolType      fetchFinished;
  } preparedStmtRecord, *preparedStmtType;

static sqlFuncType sqlFunc = NULL;

#define DEFAULT_DECIMAL_SCALE 1000
#define MAX_SIZE_ISO_DURATION 35


#ifdef SQLITE_DLL

#ifndef CDECL
#if defined(_WIN32) && HAS_CDECL
#define CDECL __cdecl
#else
#define CDECL
#endif
#endif

typedef int (CDECL *tp_sqlite3_bind_blob) (sqlite3_stmt *pStmt,
                                           int index,
                                           const void *value,
                                           int n,
                                           void (*destruct) (void*));
typedef int (CDECL *tp_sqlite3_bind_double) (sqlite3_stmt *pStmt, int index, double value);
typedef int (CDECL *tp_sqlite3_bind_int) (sqlite3_stmt *pStmt, int index, int value);
typedef int (CDECL *tp_sqlite3_bind_int64) (sqlite3_stmt *pStmt, int index, sqlite3_int64 value);
typedef int (CDECL *tp_sqlite3_bind_null) (sqlite3_stmt *pStmt, int index);
typedef int (CDECL *tp_sqlite3_bind_parameter_count) (sqlite3_stmt *pStmt);
typedef int (CDECL *tp_sqlite3_bind_text) (sqlite3_stmt *pStmt,
                                           int index,
                                           const char *value,
                                           int n,
                                           void (*destruct) (void*));
typedef int (CDECL *tp_sqlite3_close) (sqlite3 *db);
typedef const void *(CDECL *tp_sqlite3_column_blob) (sqlite3_stmt *pStmt, int iCol);
typedef int (CDECL *tp_sqlite3_column_bytes) (sqlite3_stmt *pStmt, int iCol);
typedef int (CDECL *tp_sqlite3_column_count) (sqlite3_stmt *pStmt);
typedef double (CDECL *tp_sqlite3_column_double) (sqlite3_stmt *pStmt, int iCol);
typedef int (CDECL *tp_sqlite3_column_int) (sqlite3_stmt *pStmt, int iCol);
typedef sqlite3_int64 (CDECL *tp_sqlite3_column_int64) (sqlite3_stmt *pStmt, int iCol);
typedef const char *(CDECL *tp_sqlite3_column_name) (sqlite3_stmt *pStmt, int N);
typedef const unsigned char *(CDECL *tp_sqlite3_column_text) (sqlite3_stmt *pStmt, int iCol);
typedef int (CDECL *tp_sqlite3_column_type) (sqlite3_stmt *pStmt, int iCol);
typedef sqlite3 *(CDECL *tp_sqlite3_db_handle) (sqlite3_stmt *pStmt);
typedef int (CDECL *tp_sqlite3_errcode) (sqlite3 *db);
typedef const char *(CDECL *tp_sqlite3_errmsg) (sqlite3 *db);
typedef int (CDECL *tp_sqlite3_exec) (sqlite3 *db,
                                      const char *sql,
                                      int (*callback) (void*, int, char**, char**),
                                      void *arg1OfCallback,
                                      char **errMsg);
typedef int (CDECL *tp_sqlite3_finalize) (sqlite3_stmt *pStmt);
typedef int (CDECL *tp_sqlite3_get_autocommit) (sqlite3 *db);
typedef int (CDECL *tp_sqlite3_open) (const char *filename, sqlite3 **ppDb);
typedef int (CDECL *tp_sqlite3_prepare) (sqlite3 *db,
                                         const char *sql,
                                         int nByte,
                                         sqlite3_stmt **ppStmt,
                                         const char **pzTail);
typedef int (CDECL *tp_sqlite3_reset) (sqlite3_stmt *pStmt);
typedef int (CDECL *tp_sqlite3_step) (sqlite3_stmt *pStmt);

static tp_sqlite3_bind_blob            ptr_sqlite3_bind_blob;
static tp_sqlite3_bind_double          ptr_sqlite3_bind_double;
static tp_sqlite3_bind_int             ptr_sqlite3_bind_int;
static tp_sqlite3_bind_int64           ptr_sqlite3_bind_int64;
static tp_sqlite3_bind_null            ptr_sqlite3_bind_null;
static tp_sqlite3_bind_parameter_count ptr_sqlite3_bind_parameter_count;
static tp_sqlite3_bind_text            ptr_sqlite3_bind_text;
static tp_sqlite3_close                ptr_sqlite3_close;
static tp_sqlite3_column_blob          ptr_sqlite3_column_blob;
static tp_sqlite3_column_bytes         ptr_sqlite3_column_bytes;
static tp_sqlite3_column_count         ptr_sqlite3_column_count;
static tp_sqlite3_column_double        ptr_sqlite3_column_double;
static tp_sqlite3_column_int           ptr_sqlite3_column_int;
static tp_sqlite3_column_int64         ptr_sqlite3_column_int64;
static tp_sqlite3_column_name          ptr_sqlite3_column_name;
static tp_sqlite3_column_text          ptr_sqlite3_column_text;
static tp_sqlite3_column_type          ptr_sqlite3_column_type;
static tp_sqlite3_db_handle            ptr_sqlite3_db_handle;
static tp_sqlite3_errcode              ptr_sqlite3_errcode;
static tp_sqlite3_errmsg               ptr_sqlite3_errmsg;
static tp_sqlite3_exec                 ptr_sqlite3_exec;
static tp_sqlite3_finalize             ptr_sqlite3_finalize;
static tp_sqlite3_get_autocommit       ptr_sqlite3_get_autocommit;
static tp_sqlite3_open                 ptr_sqlite3_open;
static tp_sqlite3_prepare              ptr_sqlite3_prepare;
static tp_sqlite3_reset                ptr_sqlite3_reset;
static tp_sqlite3_step                 ptr_sqlite3_step;

#define sqlite3_bind_blob            ptr_sqlite3_bind_blob
#define sqlite3_bind_double          ptr_sqlite3_bind_double
#define sqlite3_bind_int             ptr_sqlite3_bind_int
#define sqlite3_bind_int64           ptr_sqlite3_bind_int64
#define sqlite3_bind_null            ptr_sqlite3_bind_null
#define sqlite3_bind_parameter_count ptr_sqlite3_bind_parameter_count
#define sqlite3_bind_text            ptr_sqlite3_bind_text
#define sqlite3_close                ptr_sqlite3_close
#define sqlite3_column_blob          ptr_sqlite3_column_blob
#define sqlite3_column_bytes         ptr_sqlite3_column_bytes
#define sqlite3_column_count         ptr_sqlite3_column_count
#define sqlite3_column_double        ptr_sqlite3_column_double
#define sqlite3_column_int           ptr_sqlite3_column_int
#define sqlite3_column_int64         ptr_sqlite3_column_int64
#define sqlite3_column_name          ptr_sqlite3_column_name
#define sqlite3_column_text          ptr_sqlite3_column_text
#define sqlite3_column_type          ptr_sqlite3_column_type
#define sqlite3_db_handle            ptr_sqlite3_db_handle
#define sqlite3_errcode              ptr_sqlite3_errcode
#define sqlite3_errmsg               ptr_sqlite3_errmsg
#define sqlite3_exec                 ptr_sqlite3_exec
#define sqlite3_finalize             ptr_sqlite3_finalize
#define sqlite3_get_autocommit       ptr_sqlite3_get_autocommit
#define sqlite3_open                 ptr_sqlite3_open
#define sqlite3_prepare              ptr_sqlite3_prepare
#define sqlite3_reset                ptr_sqlite3_reset
#define sqlite3_step                 ptr_sqlite3_step



static boolType setupDll (const char *dllName)

  {
    static void *dbDll = NULL;

  /* setupDll */
    logFunction(printf("setupDll(\"%s\")\n", dllName););
    if (dbDll == NULL) {
      dbDll = dllOpen(dllName);
      if (dbDll != NULL) {
        if ((sqlite3_bind_blob            = (tp_sqlite3_bind_blob)            dllFunc(dbDll, "sqlite3_bind_blob"))            == NULL ||
            (sqlite3_bind_double          = (tp_sqlite3_bind_double)          dllFunc(dbDll, "sqlite3_bind_double"))          == NULL ||
            (sqlite3_bind_int             = (tp_sqlite3_bind_int)             dllFunc(dbDll, "sqlite3_bind_int"))             == NULL ||
            (sqlite3_bind_int64           = (tp_sqlite3_bind_int64)           dllFunc(dbDll, "sqlite3_bind_int64"))           == NULL ||
            (sqlite3_bind_null            = (tp_sqlite3_bind_null)            dllFunc(dbDll, "sqlite3_bind_null"))            == NULL ||
            (sqlite3_bind_parameter_count = (tp_sqlite3_bind_parameter_count) dllFunc(dbDll, "sqlite3_bind_parameter_count")) == NULL ||
            (sqlite3_bind_text            = (tp_sqlite3_bind_text)            dllFunc(dbDll, "sqlite3_bind_text"))            == NULL ||
            (sqlite3_close                = (tp_sqlite3_close)                dllFunc(dbDll, "sqlite3_close"))                == NULL ||
            (sqlite3_column_blob          = (tp_sqlite3_column_blob)          dllFunc(dbDll, "sqlite3_column_blob"))          == NULL ||
            (sqlite3_column_bytes         = (tp_sqlite3_column_bytes)         dllFunc(dbDll, "sqlite3_column_bytes"))         == NULL ||
            (sqlite3_column_count         = (tp_sqlite3_column_count)         dllFunc(dbDll, "sqlite3_column_count"))         == NULL ||
            (sqlite3_column_double        = (tp_sqlite3_column_double)        dllFunc(dbDll, "sqlite3_column_double"))        == NULL ||
            (sqlite3_column_int           = (tp_sqlite3_column_int)           dllFunc(dbDll, "sqlite3_column_int"))           == NULL ||
            (sqlite3_column_int64         = (tp_sqlite3_column_int64)         dllFunc(dbDll, "sqlite3_column_int64"))         == NULL ||
            (sqlite3_column_name          = (tp_sqlite3_column_name)          dllFunc(dbDll, "sqlite3_column_name"))          == NULL ||
            (sqlite3_column_text          = (tp_sqlite3_column_text)          dllFunc(dbDll, "sqlite3_column_text"))          == NULL ||
            (sqlite3_column_type          = (tp_sqlite3_column_type)          dllFunc(dbDll, "sqlite3_column_type"))          == NULL ||
            (sqlite3_db_handle            = (tp_sqlite3_db_handle)            dllFunc(dbDll, "sqlite3_db_handle"))            == NULL ||
            (sqlite3_errcode              = (tp_sqlite3_errcode)              dllFunc(dbDll, "sqlite3_errcode"))              == NULL ||
            (sqlite3_errmsg               = (tp_sqlite3_errmsg)               dllFunc(dbDll, "sqlite3_errmsg"))               == NULL ||
            (sqlite3_exec                 = (tp_sqlite3_exec)                 dllFunc(dbDll, "sqlite3_exec"))                 == NULL ||
            (sqlite3_finalize             = (tp_sqlite3_finalize)             dllFunc(dbDll, "sqlite3_finalize"))             == NULL ||
            (sqlite3_get_autocommit       = (tp_sqlite3_get_autocommit)       dllFunc(dbDll, "sqlite3_get_autocommit"))       == NULL ||
            (sqlite3_open                 = (tp_sqlite3_open)                 dllFunc(dbDll, "sqlite3_open"))                 == NULL ||
            (sqlite3_prepare              = (tp_sqlite3_prepare)              dllFunc(dbDll, "sqlite3_prepare"))              == NULL ||
            (sqlite3_reset                = (tp_sqlite3_reset)                dllFunc(dbDll, "sqlite3_reset"))                == NULL ||
            (sqlite3_step                 = (tp_sqlite3_step)                 dllFunc(dbDll, "sqlite3_step"))                 == NULL) {
          dbDll = NULL;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setupDll --> %d\n", dbDll != NULL););
    return dbDll != NULL;
  } /* setupDll */



static boolType findDll (void)

  {
    const char *dllList[] = { SQLITE_DLL };
    unsigned int pos;
    boolType found = FALSE;

  /* findDll */
    for (pos = 0; pos < sizeof(dllList) / sizeof(char *) && !found; pos++) {
      found = setupDll(dllList[pos]);
    } /* for */
    if (!found) {
      dllErrorMessage("sqlOpenLite", "findDll", dllList,
                      sizeof(dllList) / sizeof(char *));
    } /* if */
    return found;
  } /* findDll */

#else

#define findDll() TRUE

#endif



static void sqlClose (databaseType database);



static void setDbErrorMsg (const char *funcName, const char *dbFuncName,
    sqlite3 *connection)

  { /* setDbErrorMsg */
    dbError.funcName = funcName;
    dbError.dbFuncName = dbFuncName;
    dbError.errorCode = sqlite3_errcode(connection);
    snprintf(dbError.message, DB_ERR_MESSAGE_SIZE, "%s", sqlite3_errmsg(connection));
  } /* setDbErrorMsg */



/**
 *  Closes a database and frees the memory used by it.
 */
static void freeDatabase (databaseType database)

  {
    dbType db;

  /* freeDatabase */
    logFunction(printf("freeDatabase(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    sqlClose(database);
    db = (dbType) database;
    FREE_RECORD2(db, dbRecord, count.database, count.database_bytes);
    logFunction(printf("freeDatabase -->\n"););
  } /* freeDatabase */



/**
 *  Closes a prepared statement and frees the memory used by it.
 */
static void freePreparedStmt (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;

  /* freePreparedStmt */
    logFunction(printf("freePreparedStmt(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (preparedStmt->param_array != NULL) {
      FREE_TABLE(preparedStmt->param_array, bindDataRecord, preparedStmt->param_array_size);
    } /* if */
    sqlite3_finalize(preparedStmt->ppStmt);
    FREE_RECORD2(preparedStmt, preparedStmtRecord,
                 count.prepared_stmt, count.prepared_stmt_bytes);
    logFunction(printf("freePreparedStmt -->\n"););
  } /* freePreparedStmt */



#if LOG_FUNCTIONS_EVERYWHERE || LOG_FUNCTIONS || VERBOSE_EXCEPTIONS_EVERYWHERE || VERBOSE_EXCEPTIONS
static const char *nameOfBufferType (int buffer_type)

  {
    static char buffer[50];
    const char *typeName;

  /* nameOfBufferType */
    logFunction(printf("nameOfBufferType(%d)\n", buffer_type););
    switch (buffer_type) {
      case SQLITE_INTEGER: typeName = "SQLITE_INTEGER"; break;
      case SQLITE_FLOAT:   typeName = "SQLITE_FLOAT"; break;
      case SQLITE_TEXT:    typeName = "SQLITE_TEXT"; break;
      case SQLITE_BLOB:    typeName = "SQLITE_BLOB"; break;
      case SQLITE_NULL:    typeName = "SQLITE_NULL"; break;
      default:
        sprintf(buffer, "%d", buffer_type);
        typeName = buffer;
        break;
    } /* switch */
    logFunction(printf("nameOfBufferType --> %s\n", typeName););
    return typeName;
  } /* nameOfBufferType */
#endif



static errInfoType setupParameters (preparedStmtType preparedStmt)

  {
    int param_count;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupParameters */
    logFunction(printf("setupParameters\n"););
    param_count = sqlite3_bind_parameter_count(preparedStmt->ppStmt);
    if (unlikely(param_count < 0)) {
      dbInconsistent("setupParameters", "sqlite3_bind_parameter_count");
      logError(printf("setupParameters: Sqlite3_bind_parameter_count "
                      "returns negative column count: %d\n", param_count););
      err_info = DATABASE_ERROR;
    } else if (param_count == 0) {
      /* malloc(0) may return NULL, which would wrongly trigger a MEMORY_ERROR. */
      preparedStmt->param_array_size = 0;
      preparedStmt->param_array = NULL;
    } else if (unlikely(!ALLOC_TABLE(preparedStmt->param_array,
                                     bindDataRecord, (memSizeType) param_count))) {
      err_info = MEMORY_ERROR;
    } else {
      preparedStmt->param_array_size = (memSizeType) param_count;
      memset(preparedStmt->param_array, 0,
             (memSizeType) param_count * sizeof(bindDataRecord));
    } /* if */
    logFunction(printf("setupParameters --> %d\n", err_info););
    return err_info;
  } /* setupParameters */



static boolType allParametersBound (preparedStmtType preparedStmt)

  {
    memSizeType column_index;
    boolType okay = TRUE;

  /* allParametersBound */
    for (column_index = 0; column_index < preparedStmt->param_array_size;
         column_index++) {
      if (unlikely(!preparedStmt->param_array[column_index].bound)) {
        logError(printf("sqlExecute: Unbound parameter " FMT_U_MEM ".\n",
                        column_index + 1););
        okay = FALSE;
      } /* if */
    } /* for */
    return okay;
  } /* allParametersBound */



static void freeText (void *text)

  { /* freeText */
    free(text);
  } /* freeText */



static memSizeType decimalPrecision (const const_ustriType decimal,
    const memSizeType length)

  {
    memSizeType digitPos;
    memSizeType precision;

  /* decimalPrecision */
    precision = length;
    while (decimal[precision - 1] == '0') {
      precision--; /* Subtract trailing zero characters. */
    } /* while */
    precision--; /* Subtract decimal point character. */
    if (decimal[precision] == '.') {
      /* Trailing zero characters left from the decimal point do */
      /* not reduce the precision. This works convenient for the */
      /* function roundDoubleToBigRat, that does the conversion  */
      /* back to a rational. RoundDoubleToBigRat does not round  */
      /* digits left of the decimal point. Integer numbers with  */
      /* more than DBL_DIG digits cannot be guaranteed to be     */
      /* without change when they are converted to a double and  */
      /* back. In this case trailing zero digits are not kept.   */
      /* Therefore such numbers must be stored as blob.          */
    } else {
      digitPos = decimal[0] == '-';
      if (decimal[digitPos] == '0') {
        precision--; /* Subtract leading zero character. */
        digitPos++;
        if (decimal[digitPos] == '.') {
          /* The decimal point character has already been subtracted. */
          digitPos++;
          while (decimal[digitPos] == '0') {
            precision--; /* Subtract leading zero characters. */
            digitPos++;
          } /* while */
        } /* if */
      } /* if */
    } /* if */
    if (decimal[0] == '-') {
      precision--; /* Subtract sign character. */
    } /* if */
    return precision;
  } /* decimalPrecision */



static void sqlBindBigInt (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType value)

  {
    preparedStmtType preparedStmt;
    ustriType decimal;
    memSizeType length;
    int64Type int64Value;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBigInt */
    logFunction(printf("sqlBindBigInt(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, bigHexCStri(value)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBigInt: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(sqlite3_reset(preparedStmt->ppStmt) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindBigInt", "sqlite3_reset",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindBigInt: sqlite3_reset error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else if (bigBitLength(value) >= 64) {
        decimal = bigIntToDecimal(value, &length, &err_info);
        if (unlikely(decimal == NULL)) {
          raise_error(err_info);
        } else if (unlikely(length > INT_MAX)) {
          /* It is not possible to cast length to int. */
          free(decimal);
          raise_error(MEMORY_ERROR);
        } else if (unlikely(sqlite3_bind_blob(preparedStmt->ppStmt,
                                              (int) pos,
                                              (const void *) decimal,
                                              (int) length,
                                              &freeText) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindBigInt", "sqlite3_bind_blob",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindBigInt: sqlite3_bind_blob error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          /* The destructor (freeText) is called to dispose the */
          /* BLOB even if sqlite3_bind_blob() fails. So a call  */
          /* of free(decimal) here would corrupt the heap.      */
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } else {
        int64Value = bigToInt64(value, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else if (unlikely(sqlite3_bind_int64(preparedStmt->ppStmt,
                                               (int) pos,
                                               int64Value) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindBigInt", "sqlite3_bind_int64",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindBigInt: sqlite3_bind_int64 error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBigInt */



static void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType numerator, const const_bigIntType denominator)

  {
    preparedStmtType preparedStmt;
    double doubleValue;
    ustriType decimal;
    memSizeType length;
    memSizeType precision;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBigRat */
    logFunction(printf("sqlBindBigRat(" FMT_U_MEM ", " FMT_D ", %s, %s)\n",
                       (memSizeType) sqlStatement, pos,
                       bigHexCStri(numerator), bigHexCStri(denominator)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBigRat: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(sqlite3_reset(preparedStmt->ppStmt) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindBigRat", "sqlite3_reset",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindBigRat: sqlite3_reset error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else if (bigEqSignedDigit(denominator, 0)) {
        if (bigCmpSignedDigit(numerator, 0) > 0) {
          doubleValue = POSITIVE_INFINITY;
        } else if (bigEqSignedDigit(numerator, 0)) {
          doubleValue = NOT_A_NUMBER;
        } else {
          doubleValue = NEGATIVE_INFINITY;
        } /* if */
        if (unlikely(sqlite3_bind_double(preparedStmt->ppStmt,
                                         (int) pos,
                                         doubleValue) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindBigRat", "sqlite3_bind_double",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindBigRat: sqlite3_bind_double error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } else {
        decimal = bigRatToDecimal(numerator, denominator, DEFAULT_DECIMAL_SCALE,
                                  &length, &err_info);
        if (unlikely(decimal == NULL)) {
          raise_error(err_info);
        } else {
          /* printf("decimal: %s\n", decimal); */
          precision = decimalPrecision(decimal, length);
          /* printf("precision: " FMT_U_MEM "\n", precision); */
          if (precision > DBL_DIG) {
            if (unlikely(length > INT_MAX)) {
              /* It is not possible to cast length to int. */
              free(decimal);
              raise_error(MEMORY_ERROR);
            } else {
              if (unlikely(sqlite3_bind_blob(preparedStmt->ppStmt,
                                             (int) pos,
                                             (const void *) decimal,
                                             (int) length,
                                             &freeText) != SQLITE_OK)) {
                setDbErrorMsg("sqlBindBigRat", "sqlite3_bind_blob",
                              sqlite3_db_handle(preparedStmt->ppStmt));
                logError(printf("sqlBindBigRat: sqlite3_bind_blob error: %s\n",
                                sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
                /* The destructor (freeText) is called to dispose the */
                /* BLOB even if sqlite3_bind_blob() fails. So a call  */
                /* of free(decimal) here would corrupt the heap.      */
                raise_error(DATABASE_ERROR);
              } else {
                preparedStmt->fetchOkay = FALSE;
                preparedStmt->param_array[pos - 1].bound = TRUE;
              } /* if */
            } /* if */
          } else {
            doubleValue = strtod((char *) decimal, NULL);
            free(decimal);
            if (unlikely(sqlite3_bind_double(preparedStmt->ppStmt,
                                             (int) pos,
                                             doubleValue) != SQLITE_OK)) {
              setDbErrorMsg("sqlBindBigRat", "sqlite3_bind_double",
                            sqlite3_db_handle(preparedStmt->ppStmt));
              logError(printf("sqlBindBigRat: sqlite3_bind_double error: %s\n",
                              sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
              raise_error(DATABASE_ERROR);
            } else {
              preparedStmt->fetchOkay = FALSE;
              preparedStmt->param_array[pos - 1].bound = TRUE;
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBigRat */



static void sqlBindBool (sqlStmtType sqlStatement, intType pos, boolType value)

  {
    preparedStmtType preparedStmt;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBool */
    logFunction(printf("sqlBindBool(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, value ? "TRUE" : "FALSE"););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBool: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(sqlite3_reset(preparedStmt->ppStmt) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindBool", "sqlite3_reset",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindBool: sqlite3_reset error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else if (unlikely(sqlite3_bind_int(preparedStmt->ppStmt,
                                           (int) pos,
                                           (int) value) != SQLITE_OK)) {
        setDbErrorMsg("sqlBindBool", "sqlite3_bind_int",
                      sqlite3_db_handle(preparedStmt->ppStmt));
        logError(printf("sqlBindBool: sqlite3_bind_int error: %s\n",
                        sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
        raise_error(DATABASE_ERROR);
      } else {
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->param_array[pos - 1].bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindBool */



static void sqlBindBStri (sqlStmtType sqlStatement, intType pos,
    const const_bstriType bstri)

  {
    preparedStmtType preparedStmt;
    cstriType blob;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBStri */
    logFunction(printf("sqlBindBStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, bstriAsUnquotedCStri(bstri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBStri: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(sqlite3_reset(preparedStmt->ppStmt) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindBStri", "sqlite3_reset",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindBStri: sqlite3_reset error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else if (unlikely(bstri->size > INT_MAX ||
                          !ALLOC_CSTRI(blob, bstri->size))) {
        /* It is not possible to cast length to int or malloc() failed. */
        raise_error(MEMORY_ERROR);
      } else {
        memcpy(blob, bstri->mem, bstri->size);
        if (unlikely(sqlite3_bind_blob(preparedStmt->ppStmt,
                                       (int) pos,
                                       (const void *) blob,
                                       (int) bstri->size,
                                       &freeText) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindBStri", "sqlite3_bind_blob",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindBStri: sqlite3_bind_blob error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          /* The destructor (freeText) is called to dispose the */
          /* BLOB even if sqlite3_bind_blob() fails. So a call  */
          /* of free(blob) here would corrupt the heap.         */
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBStri */



static void sqlBindDuration (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  {
    preparedStmtType preparedStmt;
    cstriType isoDuration;
    cstriType stri;
    int length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindDuration */
    logFunction(printf("sqlBindDuration(" FMT_U_MEM ", " FMT_D ", P"
                                          FMT_D "Y" FMT_D "M" FMT_D "DT"
                                          FMT_D "H" FMT_D "M%s" FMT_U "." F_U(06) "S)\n",
                       (memSizeType) sqlStatement, pos,
                       year, month, day, hour, minute,
                       second < 0 || micro_second < 0 ? "-" : "",
                       intAbs(second), intAbs(micro_second)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindDuration: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else if (unlikely(year < -9999 || year > 9999 || month < -12 || month > 12 ||
                        day < -31 || day > 31 || hour <= -24 || hour >= 24 ||
                        minute <= -60 || minute >= 60 || second <= -60 || second >= 60 ||
                        micro_second <= -1000000 || micro_second >= 1000000)) {
      logError(printf("sqlBindDuration: Duration not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(sqlite3_reset(preparedStmt->ppStmt) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindDuration", "sqlite3_reset",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindDuration: sqlite3_reset error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else if (unlikely(!ALLOC_CSTRI(isoDuration, MAX_SIZE_ISO_DURATION))) {
        raise_error(MEMORY_ERROR);
      } else {
        /* Write the duration in the format: P[nY][nM][nD][T[nH][nM][n[.n]S]] */
        stri = isoDuration;
        *stri = 'P';
        stri++;
        if (year != 0) {
          stri += sprintf(stri, FMT_D "Y", year);
        } /* if */
        if (month != 0) {
          stri += sprintf(stri, FMT_D "M", month);
        } /* if */
        if (day != 0 || (year == 0 && month == 0 && hour == 0 &&
            minute == 0 && second == 0 && micro_second == 0)) {
          stri += sprintf(stri, FMT_D "D", day);
        } /* if */
        if (hour != 0 || minute != 0 || second != 0 || micro_second != 0) {
          *stri = 'T';
          stri++;
          if (hour != 0) {
            stri += sprintf(stri, FMT_D "H", hour);
          } /* if */
          if (minute != 0) {
            stri += sprintf(stri, FMT_D "M", minute);
          } /* if */
          if (second != 0 || micro_second != 0) {
            if (second == 0 && micro_second < 0) {
              stri += sprintf(stri, "-0");
            } else {
              stri += sprintf(stri, FMT_D, second);
            } /* if */
            if (micro_second != 0) {
              stri += sprintf(stri, ".%06lu", labs((long) micro_second));
            } /* if */
            sprintf(stri, "S");
          } /* if */
        } /* if */
        length = (int) strlen(isoDuration);
        if (unlikely(sqlite3_bind_text(preparedStmt->ppStmt,
                                       (int) pos,
                                       isoDuration,
                                       length,
                                       &freeText) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindDuration", "sqlite3_bind_text",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindDuration: sqlite3_bind_text error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          /* The destructor (freeText) is called to dispose the  */
          /* string even if sqlite3_bind_text() fails. So a call */
          /* of free(isoDuration) here would corrupt the heap.   */
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindDuration */



static void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value)

  {
    preparedStmtType preparedStmt;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindFloat */
    logFunction(printf("sqlBindFloat(" FMT_U_MEM ", " FMT_D ", " FMT_E ")\n",
                       (memSizeType) sqlStatement, pos, value););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindFloat: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(sqlite3_reset(preparedStmt->ppStmt) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindFloat", "sqlite3_reset",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindFloat: sqlite3_reset error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else if (unlikely(sqlite3_bind_double(preparedStmt->ppStmt,
                                              (int) pos,
                                              (double) value) != SQLITE_OK)) {
        setDbErrorMsg("sqlBindFloat", "sqlite3_bind_double",
                      sqlite3_db_handle(preparedStmt->ppStmt));
        logError(printf("sqlBindFloat: sqlite3_bind_double error: %s\n",
                        sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
        raise_error(DATABASE_ERROR);
      } else {
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->param_array[pos - 1].bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindFloat */



static void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value)

  {
    preparedStmtType preparedStmt;
    int bind_result;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindInt */
    logFunction(printf("sqlBindInt(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos, value););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindInt: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(sqlite3_reset(preparedStmt->ppStmt) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindInt", "sqlite3_reset",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindInt: sqlite3_reset error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
#if INTTYPE_SIZE == 32
        bind_result = sqlite3_bind_int(preparedStmt->ppStmt,
                                       (int) pos,
                                       (int) value);
#elif INTTYPE_SIZE == 64
        bind_result = sqlite3_bind_int64(preparedStmt->ppStmt,
                                         (int) pos,
                                         (sqlite3_int64) value);
#else
#error "INTTYPE_SIZE is neither 32 nor 64."
        bind_result = SQLITE_ERROR;
#endif
        if (unlikely(bind_result != SQLITE_OK)) {
          setDbErrorMsg("sqlBindInt", "sqlite3_bind_int",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindInt: sqlite3_bind_int error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindInt */



static void sqlBindNull (sqlStmtType sqlStatement, intType pos)

  {
    preparedStmtType preparedStmt;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindNull */
    logFunction(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindNull: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(sqlite3_reset(preparedStmt->ppStmt) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindNull", "sqlite3_reset",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindNull: sqlite3_reset error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else if (unlikely(sqlite3_bind_null(preparedStmt->ppStmt,
                                            (int) pos) != SQLITE_OK)) {
        setDbErrorMsg("sqlBindNull", "sqlite3_bind_null",
                      sqlite3_db_handle(preparedStmt->ppStmt));
        logError(printf("sqlBindNull: sqlite3_bind_null error: %s\n",
                        sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
        raise_error(DATABASE_ERROR);
      } else {
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->param_array[pos - 1].bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindNull */



static void sqlBindStri (sqlStmtType sqlStatement, intType pos,
    const const_striType stri)

  {
    preparedStmtType preparedStmt;
    cstriType stri8;
    cstriType resized_stri8;
    memSizeType length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindStri */
    logFunction(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, striAsUnquotedCStri(stri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindStri: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(sqlite3_reset(preparedStmt->ppStmt) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindStri", "sqlite3_reset",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindStri: sqlite3_reset error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else if (unlikely((stri8 = stri_to_cstri8_buf(stri, &length)) == NULL)) {
        raise_error(MEMORY_ERROR);
      } else if (unlikely(length > INT_MAX)) {
        /* It is not possible to cast length to int. */
        free_cstri8(stri8, stri);
        raise_error(MEMORY_ERROR);
      } else {
        resized_stri8 = REALLOC_CSTRI(stri8, length);
        if (likely(resized_stri8 != NULL)) {
          stri8 = resized_stri8;
        } /* if */
        if (unlikely(sqlite3_bind_text(preparedStmt->ppStmt,
                                       (int) pos,
                                       stri8,
                                       (int) length,
                                       &freeText) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindStri", "sqlite3_bind_text",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindStri: sqlite3_bind_text error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          /* The destructor (freeText) is called to dispose the     */
          /* string even if sqlite3_bind_text() fails. So a call of */
          /* free_cstri8(stri8, stri) here would corrupt the heap.  */
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindStri */



static void sqlBindTime (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second,
    intType time_zone)

  {
    preparedStmtType preparedStmt;
    cstriType isoDate;
    int length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindTime */
    logFunction(printf("sqlBindTime(" FMT_U_MEM ", " FMT_D ", "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) ", "
                       FMT_D ")\n",
                       (memSizeType) sqlStatement, pos,
                       year, month, day,
                       hour, minute, second, micro_second,
                       time_zone););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindTime: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else if (unlikely(year < -999 || year > 9999 || month < 1 || month > 12 ||
                        day < 1 || day > 31 || hour < 0 || hour >= 24 ||
                        minute < 0 || minute >= 60 || second < 0 || second >= 60 ||
                        micro_second < 0 || micro_second >= 1000000)) {
      logError(printf("sqlBindTime: Time not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(sqlite3_reset(preparedStmt->ppStmt) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindTime", "sqlite3_reset",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindTime: sqlite3_reset error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else if (unlikely(!ALLOC_CSTRI(isoDate, 26))) {
        raise_error(MEMORY_ERROR);
      } else {
        if (hour == 0 && minute == 0 && second == 0 && micro_second == 0) {
          sprintf(isoDate, F_D(04) "-" F_U(02) "-" F_U(02),
                  year, month, day);
          length = 10;
        } else if (year == 0 && month == 1 && day == 1) {
          if (micro_second == 0) {
            sprintf(isoDate, F_U(02) ":" F_U(02) ":" F_U(02),
                    hour, minute, second);
            length = 8;
          } else {
            sprintf(isoDate, F_U(02) ":" F_U(02) ":" F_U(02) "." F_U(06),
                    hour, minute, second, micro_second);
            length = 15;
          } /* if */
        } else if (micro_second == 0) {
          sprintf(isoDate, F_D(04) "-" F_U(02) "-" F_U(02) " "
                           F_U(02) ":" F_U(02) ":" F_U(02),
                  year, month, day, hour, minute, second);
          length = 19;
        } else {
          sprintf(isoDate, F_D(04) "-" F_U(02) "-" F_U(02) " "
                           F_U(02) ":" F_U(02) ":" F_U(02) "." F_U(06),
                  year, month, day, hour, minute, second,  micro_second);
          length = 26;
        } /* if */
        if (unlikely(sqlite3_bind_text(preparedStmt->ppStmt,
                                       (int) pos,
                                       isoDate,
                                       length,
                                       &freeText) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindTime", "sqlite3_bind_text",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindTime: sqlite3_bind_text error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          /* The destructor (freeText) is called to dispose the  */
          /* string even if sqlite3_bind_text() fails. So a call */
          /* of free(isoDate) here would corrupt the heap.       */
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindTime */



static void sqlClose (databaseType database)

  {
    dbType db;

  /* sqlClose */
    logFunction(printf("sqlClose(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (db->connection != NULL) {
      sqlite3_close(db->connection);
      db->connection = NULL;
    } /* if */
    logFunction(printf("sqlClose -->\n"););
  } /* sqlClose */



static bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    int64Type columnValue64;
    const void *blob;
    int length;
    bigIntType columnValue;

  /* sqlColumnBigInt */
    logFunction(printf("sqlColumnBigInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnBigInt: Fetch okay: %d, column: " FMT_D ", max column: %u.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      /* printf("type: %s\n",
         nameOfBufferType(sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))); */
      switch (sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1)) {
        case SQLITE_NULL:
          columnValue = bigZero();
          break;
        case SQLITE_INTEGER:
          columnValue64 = sqlite3_column_int64(preparedStmt->ppStmt, (int) column - 1);
          columnValue = bigFromInt64(columnValue64);
          break;
        case SQLITE_BLOB:
          blob = sqlite3_column_blob(preparedStmt->ppStmt, (int) column - 1);
          length = sqlite3_column_bytes(preparedStmt->ppStmt, (int) column - 1);
          if (unlikely(blob == NULL || length < 0)) {
            dbInconsistent("sqlColumnBigInt", "sqlite3_column_bytes");
            logError(printf("sqlColumnBigInt: Column " FMT_D ": Blob " FMT_U_MEM
                            " is NULL or has a negative length (%d).\n",
                            column, (memSizeType) blob, length););
            raise_error(DATABASE_ERROR);
            columnValue = NULL;
          } else if (length >= 2 && memcmp(&((char *) blob)[length - 2], ".0", 2) == 0) {
            columnValue = getDecimalBigInt((const_ustriType) blob, (memSizeType) length - 2);
          } else {
            columnValue = getDecimalBigInt((const_ustriType) blob, (memSizeType) length);
          } /* if */
          break;
        default:
          logError(printf("sqlColumnBigInt: Column " FMT_D " has the unknown type %s.\n",
                          column, nameOfBufferType(
                          sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))););
          raise_error(RANGE_ERROR);
          columnValue = NULL;
          break;
      } /* switch */
    } /* if */
    logFunction(printf("sqlColumnBigInt --> %s\n", bigHexCStri(columnValue)););
    return columnValue;
  } /* sqlColumnBigInt */



static void sqlColumnBigRat (sqlStmtType sqlStatement, intType column,
    bigIntType *numerator, bigIntType *denominator)

  {
    preparedStmtType preparedStmt;
    int64Type columnValue64;
    const void *blob;
    int length;

  /* sqlColumnBigRat */
    logFunction(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", *, *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnBigRat: Fetch okay: %d, column: " FMT_D ", max column: %u.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
    } else {
      /* printf("type: %s\n",
         nameOfBufferType(sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))); */
      switch (sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1)) {
        case SQLITE_NULL:
          *numerator = bigZero();
          *denominator = bigFromInt32(1);
          break;
        case SQLITE_INTEGER:
          columnValue64 = sqlite3_column_int64(preparedStmt->ppStmt, (int) column - 1);
          *numerator = bigFromInt64(columnValue64);
          *denominator = bigFromInt32(1);
          break;
        case SQLITE_FLOAT:
          *numerator = roundDoubleToBigRat(
              sqlite3_column_double(preparedStmt->ppStmt, (int) column - 1),
              TRUE, denominator);
          break;
        case SQLITE_BLOB:
          blob = sqlite3_column_blob(preparedStmt->ppStmt, (int) column - 1);
          length = sqlite3_column_bytes(preparedStmt->ppStmt, (int) column - 1);
          if (unlikely(blob == NULL || length < 0)) {
            dbInconsistent("sqlColumnBigRat", "sqlite3_column_bytes");
            logError(printf("sqlColumnBigRat: Column " FMT_D ": Blob " FMT_U_MEM
                            " is NULL or has a negative length (%d).\n",
                            column, (memSizeType) blob, length););
            raise_error(DATABASE_ERROR);
          } else {
            *numerator = getDecimalBigRational(
                (const_ustriType) blob, (memSizeType) length, denominator);
          } /* if */
          break;
        default:
          logError(printf("sqlColumnBigRat: Column " FMT_D " has the unknown type %s.\n",
                          column, nameOfBufferType(
                          sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))););
          raise_error(RANGE_ERROR);
          break;
      } /* switch */
    } /* if */
    logFunction(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", %s, %s) -->\n",
                       (memSizeType) sqlStatement, column,
                       bigHexCStri(*numerator), bigHexCStri(*denominator)););
  } /* sqlColumnBigRat */



static boolType sqlColumnBool (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    const_ustriType stri;
    intType columnValue;

  /* sqlColumnBool */
    logFunction(printf("sqlColumnBool(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnBool: Fetch okay: %d, column: " FMT_D ", max column: %u.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      /* printf("type: %s\n",
         nameOfBufferType(sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))); */
      switch (sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1)) {
        case SQLITE_NULL:
          columnValue = 0;
          break;
        case SQLITE_INTEGER:
          columnValue = sqlite3_column_int(preparedStmt->ppStmt, (int) column - 1);
          break;
        case SQLITE_TEXT:
          stri = sqlite3_column_text(preparedStmt->ppStmt, (int) column - 1);
          if (stri == NULL) {
            columnValue = 0;
          } else {
            if (unlikely(sqlite3_column_bytes(preparedStmt->ppStmt, (int) column - 1) != 1)) {
              logError(printf("sqlColumnBool: Column " FMT_D ": "
                              "The size of a boolean field must be 1.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = 0;
            } else {
              columnValue = stri[0] - '0';
            } /* if */
          } /* if */
          break;
        default:
          logError(printf("sqlColumnBool: Column " FMT_D " has the unknown type %s.\n",
                          column, nameOfBufferType(
                          sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))););
          raise_error(RANGE_ERROR);
          columnValue = 0;
          break;
      } /* switch */
      if (unlikely((uintType) columnValue >= 2)) {
        logError(printf("sqlColumnBool: Column " FMT_D ": "
                        FMT_D " is not an allowed boolean value.\n",
                        column, columnValue););
        raise_error(RANGE_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnBool --> %s\n", columnValue ? "TRUE" : "FALSE"););
    return columnValue != 0;
  } /* sqlColumnBool */



static bstriType sqlColumnBStri (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    const_ustriType blob;
    const_ustriType stri8;
    int length;
    striType stri;
    errInfoType err_info = OKAY_NO_ERROR;
    bstriType columnValue;

  /* sqlColumnBStri */
    logFunction(printf("sqlColumnBStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnBStri: Fetch okay: %d, column: " FMT_D ", max column: %u.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      /* printf("type: %s\n",
         nameOfBufferType(sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))); */
      switch (sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1)) {
        case SQLITE_NULL:
          if (unlikely(!ALLOC_BSTRI_SIZE_OK(columnValue, 0))) {
            raise_error(MEMORY_ERROR);
          } else {
            columnValue->size = 0;
          } /* if */
          break;
        case SQLITE_BLOB:
          blob = (const_ustriType) sqlite3_column_blob(preparedStmt->ppStmt, (int) column - 1);
          if (blob == NULL) {
            if (unlikely(!ALLOC_BSTRI_SIZE_OK(columnValue, 0))) {
              raise_error(MEMORY_ERROR);
            } else {
              columnValue->size = 0;
            } /* if */
          } else {
            length = sqlite3_column_bytes(preparedStmt->ppStmt, (int) column - 1);
            if (unlikely(length < 0)) {
              dbInconsistent("sqlColumnBStri", "sqlite3_column_bytes");
              logError(printf("sqlColumnBStri: Column " FMT_D ": "
                              "Sqlite3_column_bytes returns negative length: %d\n",
                              column, length););
              raise_error(DATABASE_ERROR);
              columnValue = NULL;
            } else if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(columnValue, (memSizeType) length))) {
              raise_error(MEMORY_ERROR);
            } else {
              columnValue->size = (memSizeType) length;
              memcpy(columnValue->mem, blob, (memSizeType) length);
            } /* if */
          } /* if */
          break;
        case SQLITE_TEXT:
          stri8 = sqlite3_column_text(preparedStmt->ppStmt, (int) column - 1);
          if (stri8 == NULL) {
            if (unlikely(!ALLOC_BSTRI_SIZE_OK(columnValue, 0))) {
              raise_error(MEMORY_ERROR);
            } else {
              columnValue->size = 0;
            } /* if */
          } else {
            length = sqlite3_column_bytes(preparedStmt->ppStmt, (int) column - 1);
            if (unlikely(length < 0)) {
              dbInconsistent("sqlColumnBStri", "sqlite3_column_bytes");
              logError(printf("sqlColumnBStri: Column " FMT_D ": "
                              "Sqlite3_column_bytes returns negative length: %d\n",
                              column, length););
              raise_error(DATABASE_ERROR);
              columnValue = NULL;
            } else {
              stri = cstri8_buf_to_stri((cstriType) stri8, (memSizeType) length, &err_info);
              if (unlikely(stri == NULL)) {
                raise_error(err_info);
                columnValue = NULL;
              } else {
                columnValue = stri_to_bstri(stri, &err_info);
                strDestr(stri);
                if (unlikely(columnValue == NULL)) {
                  raise_error(err_info);
                } /* if */
              } /* if */
            } /* if */
          } /* if */
          break;
        default:
          logError(printf("sqlColumnBStri: Column " FMT_D " has the unknown type %s.\n",
                          column, nameOfBufferType(
                          sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))););
          raise_error(RANGE_ERROR);
          columnValue = NULL;
          break;
      } /* switch */
    } /* if */
    logFunction(printf("sqlColumnBStri --> \"%s\"\n", bstriAsUnquotedCStri(columnValue)););
    return columnValue;
  } /* sqlColumnBStri */



static void sqlColumnDuration (sqlStmtType sqlStatement, intType column,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second)

  {
    preparedStmtType preparedStmt;
    const_ustriType isoDuration;
    const_ustriType numStri;
    const_ustriType stri;
    boolType datePart = TRUE;
    boolType secondsPart = TRUE;
    boolType negativeSeconds = FALSE;
    intType seconds;
    memSizeType microsecStriLen;
    char microsecStri[6 + NULL_TERMINATION_LEN];
    boolType okay = TRUE;

  /* sqlColumnDuration */
    logFunction(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnDuration: Fetch okay: %d, column: " FMT_D ", max column: %u.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
    } else {
      *year         = 0;
      *month        = 0;
      *day          = 0;
      *hour         = 0;
      *minute       = 0;
      *second       = 0;
      *micro_second = 0;
      /* printf("type: %s\n",
         nameOfBufferType(sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))); */
      isoDuration = sqlite3_column_text(preparedStmt->ppStmt, (int) column - 1);
      /* printf("isoDuration: %lx\n", (unsigned long int) isoDuration); */
      if (isoDuration != NULL) {
        stri = isoDuration;
        if (*stri != 'P') {
          okay = FALSE;
        } else {
          stri++;
          while (*stri != '\0') {
            numStri = stri;
            if (*stri == '-') {
              stri++;
            } /* if */
            stri += strspn((const char *) stri, "0123456789");
            if (*stri != '\0') {
              switch (*stri) {
                case 'Y':
                  okay &= sscanf((const char *) numStri, FMT_D "Y", year) == 1;
                  stri++;
                  break;
                case 'M':
                  if (datePart) {
                    okay &= sscanf((const char *) numStri, FMT_D "M", month) == 1;
                  } else {
                    okay &= sscanf((const char *) numStri, FMT_D "M", minute) == 1;
                  } /* if */
                  stri++;
                  break;
                case 'D':
                  okay &= sscanf((const char *) numStri, FMT_D "D", day) == 1;
                  stri++;
                  break;
                case 'T':
                  break;
                case 'H':
                  okay &= sscanf((const char *) numStri, FMT_D "H", hour) == 1;
                  stri++;
                  break;
                case 'S':
                  if (secondsPart) {
                    okay &= sscanf((const char *) numStri, FMT_D "S", second) == 1;
                  } else {
                    *second = seconds;
                    sprintf(microsecStri, "%.6s", numStri);
                    microsecStriLen = (memSizeType) (stri - numStri);
                    if (microsecStriLen < 6) {
                      memset(&microsecStri[microsecStriLen], '0', 6 - microsecStriLen);
                      microsecStri[6] = '\0';
                    } /* if */
                    okay &= sscanf((const char *) microsecStri, FMT_D "S", micro_second) == 1;
                    if (negativeSeconds) {
                      *micro_second = -*micro_second;
                    } /* if */
                  } /* if */
                  stri++;
                  break;
                case '.':
                  secondsPart = FALSE;
                  negativeSeconds = *numStri == '-';
                  okay &= sscanf((const char *) numStri, FMT_D ".", &seconds) == 1;
                  stri++;
                  break;
                default:
                  okay = FALSE;
                  break;
              } /* switch */
              if (*stri == 'T') {
                if (datePart) {
                  datePart = FALSE;
                  stri++;
                } else {
                  okay = FALSE;
                } /* if */
              } /* if */
            } else {
              okay = FALSE;
            } /* if */
          } /* while */
        } /* if */
        if (unlikely(!okay)) {
          logError(printf("sqlColumnDuration: Column " FMT_D ": "
                          "Failed to recognize duration: %s\n",
                          column, isoDuration););
          raise_error(RANGE_ERROR);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ") -> P"
                                            FMT_D "Y" FMT_D "M" FMT_D "DT"
                                            FMT_D "H" FMT_D "M%s" FMT_U "." F_U(06) "S\n",
                       (memSizeType) sqlStatement, column,
                       *year, *month, *day, *hour, *minute,
                       *second < 0 || *micro_second < 0 ? "-" : "",
                       intAbs(*second), intAbs(*micro_second)););
  } /* sqlColumnDuration */



static floatType sqlColumnFloat (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    const_ustriType blob;
    int length;
    floatType columnValue;

  /* sqlColumnFloat */
    logFunction(printf("sqlColumnFloat(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnFloat: Fetch okay: %d, column: " FMT_D ", max column: %u.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = 0.0;
    } else {
      /* printf("type: %s\n",
         nameOfBufferType(sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))); */
      switch (sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1)) {
        case SQLITE_NULL:
          columnValue = 0.0;
          break;
        case SQLITE_INTEGER:
          columnValue = (floatType) sqlite3_column_int64(preparedStmt->ppStmt, (int) column - 1);
          break;
        case SQLITE_FLOAT:
          columnValue = sqlite3_column_double(preparedStmt->ppStmt, (int) column - 1);
          break;
        case SQLITE_BLOB:
          blob = (const_ustriType) sqlite3_column_blob(preparedStmt->ppStmt, (int) column - 1);
          length = sqlite3_column_bytes(preparedStmt->ppStmt, (int) column - 1);
          if (unlikely(blob == NULL || length < 0)) {
            dbInconsistent("sqlColumnFloat", "sqlite3_column_bytes");
            logError(printf("sqlColumnFloat: Column " FMT_D ": Blob " FMT_U_MEM
                            " is NULL or has a negative length (%d).\n",
                            column, (memSizeType) blob, length););
            raise_error(DATABASE_ERROR);
            columnValue = 0.0;
          } else {
            columnValue = getDecimalFloat((const_ustriType) blob, (memSizeType) length);
          } /* if */
          break;
        default:
          logError(printf("sqlColumnFloat: Column " FMT_D " has the unknown type %s.\n",
                          column, nameOfBufferType(
                          sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))););
          raise_error(RANGE_ERROR);
          columnValue = 0.0;
          break;
      } /* switch */
    } /* if */
    logFunction(printf("sqlColumnFloat --> " FMT_E "\n", columnValue););
    return columnValue;
  } /* sqlColumnFloat */



static intType sqlColumnInt (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    const void *blob;
    int length;
    intType columnValue;

  /* sqlColumnInt */
    logFunction(printf("sqlColumnInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnInt: Fetch okay: %d, column: " FMT_D ", max column: %u.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      /* printf("type: %s\n",
         nameOfBufferType(sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))); */
      switch (sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1)) {
        case SQLITE_NULL:
          columnValue = 0;
          break;
        case SQLITE_INTEGER:
#if INTTYPE_SIZE == 32
          columnValue = sqlite3_column_int(preparedStmt->ppStmt, (int) column - 1);
#elif INTTYPE_SIZE == 64
          columnValue = sqlite3_column_int64(preparedStmt->ppStmt, (int) column - 1);
#else
#error "INTTYPE_SIZE is neither 32 nor 64."
          raise_error(RANGE_ERROR);
#endif
          break;
        case SQLITE_BLOB:
          blob = sqlite3_column_blob(preparedStmt->ppStmt, (int) column - 1);
          length = sqlite3_column_bytes(preparedStmt->ppStmt, (int) column - 1);
          if (unlikely(blob == NULL || length < 0)) {
            dbInconsistent("sqlColumnInt", "sqlite3_column_bytes");
            logError(printf("sqlColumnInt: Column " FMT_D ": Blob " FMT_U_MEM
                            " is NULL or has a negative length (%d).\n",
                            column, (memSizeType) blob, length););
            raise_error(DATABASE_ERROR);
            columnValue = 0;
          } else if (length >= 2 && memcmp(&((char *) blob)[length - 2], ".0", 2) == 0) {
            columnValue = getDecimalInt((const_ustriType) blob, (memSizeType) length - 2);
          } else {
            columnValue = getDecimalInt((const_ustriType) blob, (memSizeType) length);
          } /* if */
          break;
        default:
          logError(printf("sqlColumnInt: Column " FMT_D " has the unknown type %s.\n",
                          column, nameOfBufferType(
                          sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))););
          raise_error(RANGE_ERROR);
          columnValue = 0;
          break;
      } /* switch */
    } /* if */
    logFunction(printf("sqlColumnInt --> " FMT_D "\n", columnValue););
    return columnValue;
  } /* sqlColumnInt */



static striType sqlColumnStri (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    const_ustriType stri8;
    const_ustriType blob;
    int length;
    errInfoType err_info = OKAY_NO_ERROR;
    striType columnValue;

  /* sqlColumnStri */
    logFunction(printf("sqlColumnStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnStri: Fetch okay: %d, column: " FMT_D ", max column: %u.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      /* printf("type: %s\n",
         nameOfBufferType(sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))); */
      switch (sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1)) {
        case SQLITE_NULL:
          columnValue = strEmpty();
          break;
        case SQLITE_TEXT:
          stri8 = sqlite3_column_text(preparedStmt->ppStmt, (int) column - 1);
          if (stri8 == NULL) {
            columnValue = strEmpty();
          } else {
            length = sqlite3_column_bytes(preparedStmt->ppStmt, (int) column - 1);
            if (unlikely(length < 0)) {
              dbInconsistent("sqlColumnStri", "sqlite3_column_bytes");
              logError(printf("sqlColumnStri: Column " FMT_D ": "
                              "Sqlite3_column_bytes returns negative length: %d\n",
                              column, length););
              raise_error(DATABASE_ERROR);
              columnValue = NULL;
            } else {
              columnValue = cstri8_buf_to_stri((cstriType) stri8, (memSizeType) length, &err_info);
              if (unlikely(columnValue == NULL)) {
                raise_error(err_info);
              } /* if */
            } /* if */
          } /* if */
          break;
        case SQLITE_BLOB:
          blob = (const_ustriType) sqlite3_column_blob(preparedStmt->ppStmt, (int) column - 1);
          if (blob == NULL) {
            columnValue = strEmpty();
          } else {
            length = sqlite3_column_bytes(preparedStmt->ppStmt, (int) column - 1);
            if (unlikely(length < 0)) {
              dbInconsistent("sqlColumnStri", "sqlite3_column_bytes");
              logError(printf("sqlColumnStri: Column " FMT_D ": "
                              "Sqlite3_column_bytes returns negative length: %d\n",
                              column, length););
              raise_error(DATABASE_ERROR);
              columnValue = NULL;
            } else {
              columnValue = cstri_buf_to_stri((const_cstriType) blob, (memSizeType) length);
              if (unlikely(columnValue == NULL)) {
                raise_error(MEMORY_ERROR);
              } /* if */
            } /* if */
          } /* if */
          break;
        default:
          logError(printf("sqlColumnStri: Column " FMT_D " has the unknown type %s.\n",
                          column, nameOfBufferType(
                          sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))););
          raise_error(RANGE_ERROR);
          columnValue = NULL;
          break;
      } /* switch */
    } /* if */
    logFunction(printf("sqlColumnStri --> \"%s\"\n", striAsUnquotedCStri(columnValue)););
    return columnValue;
  } /* sqlColumnStri */



static void sqlColumnTime (sqlStmtType sqlStatement, intType column,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second,
    intType *time_zone, boolType *is_dst)

  {
    preparedStmtType preparedStmt;
    const_cstriType isoDate;
    boolType okay;
    boolType isTime;

  /* sqlColumnTime */
    logFunction(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnTime: Fetch okay: %d, column: " FMT_D ", max column: %u.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
    } else {
      /* printf("type: %s\n",
         nameOfBufferType(sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1))); */
      isoDate = (const_cstriType) sqlite3_column_text(preparedStmt->ppStmt,
                                                      (int) column - 1);
      /* printf("isoDate: %lx\n", (unsigned long int) isoDate); */
      if (isoDate == NULL) {
        *year         = 0;
        *month        = 1;
        *day          = 1;
        *hour         = 0;
        *minute       = 0;
        *second       = 0;
        *micro_second = 0;
        *time_zone    = 0;
        *is_dst       = 0;
      } else {
        okay = assignTime(isoDate, year, month, day, hour, minute, second,
                          micro_second, &isTime);
        if (unlikely(!okay)) {
          logError(printf("sqlColumnTime: Column " FMT_D ": "
                          "Failed to recognize time: %s\n",
                          column, isoDate););
          raise_error(RANGE_ERROR);
        } else if (isTime) {
          *year = 2000;
          timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                        time_zone, is_dst);
          *year = 0;
        } else {
          timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                        time_zone, is_dst);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ", "
                                        F_D(04) "-" F_D(02) "-" F_D(02) " "
                                        F_D(02) ":" F_D(02) ":" F_D(02) "."
                                        F_D(06) ", " FMT_D ", %d) -->\n",
                       (memSizeType) sqlStatement, column,
                       *year, *month, *day, *hour, *minute, *second,
                       *micro_second, *time_zone, *is_dst););
  } /* sqlColumnTime */



static void sqlCommit (databaseType database)

  {
    dbType db;

  /* sqlCommit */
    logFunction(printf("sqlCommit(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (unlikely(db->connection == NULL)) {
      logError(printf("sqlCommit: Database is not open.\n"););
      raise_error(RANGE_ERROR);
    } else {
      if (sqlite3_get_autocommit(db->connection) != 0) {
        sqlite3_exec(db->connection, "COMMIT", NULL, NULL, NULL);
        sqlite3_exec(db->connection, "BEGIN", NULL, NULL, NULL);
      } /* if */
    } /* if */
    logFunction(printf("sqlCommit -->\n"););
  } /* sqlCommit */



static void sqlExecute (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    int step_result;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlExecute */
    logFunction(printf("sqlExecute(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!allParametersBound(preparedStmt))) {
      dbLibError("sqlExecute", "SQLExecute",
                 "Unbound statement parameter(s).\n");
      logError(printf("sqlExecute: Unbound statement parameter(s).\n"););
      raise_error(DATABASE_ERROR);
    } else {
      /* printf("ppStmt: %lx\n", (unsigned long) preparedStmt->ppStmt); */
      if (preparedStmt->executeSuccessful) {
        if (unlikely(sqlite3_reset(preparedStmt->ppStmt) != SQLITE_OK)) {
          setDbErrorMsg("sqlBindTime", "sqlite3_reset",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlBindTime: sqlite3_reset error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          err_info = DATABASE_ERROR;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        preparedStmt->fetchOkay = FALSE;
        step_result = sqlite3_step(preparedStmt->ppStmt);
        if (step_result == SQLITE_ROW) {
          /* printf("set_result: SQLITE_ROW\n"); */
          preparedStmt->executeSuccessful = TRUE;
          preparedStmt->fetchFinished = FALSE;
          preparedStmt->useStoredFetchResult = TRUE;
          preparedStmt->storedFetchResult = TRUE;
        } else if (step_result == SQLITE_DONE) {
          /* printf("set_result: SQLITE_DONE\n"); */
          preparedStmt->executeSuccessful = TRUE;
          preparedStmt->fetchFinished = FALSE;
          preparedStmt->useStoredFetchResult = TRUE;
          preparedStmt->storedFetchResult = FALSE;
        } else {
          setDbErrorMsg("sqlExecute", "sqlite3_step",
                        sqlite3_db_handle(preparedStmt->ppStmt));
          logError(printf("sqlExecute: sqlite3_step error: %s\n",
                          sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
          preparedStmt->executeSuccessful = FALSE;
          raise_error(DATABASE_ERROR);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("sqlExecute -->\n"););
  } /* sqlExecute */



static boolType sqlFetch (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    int step_result;

  /* sqlFetch */
    logFunction(printf("sqlFetch(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->executeSuccessful)) {
      dbLibError("sqlFetch", "sqlite3_step",
                 "Execute was not successful.\n");
      logError(printf("sqlFetch: Execute was not successful.\n"););
      preparedStmt->fetchOkay = FALSE;
      raise_error(DATABASE_ERROR);
    } else if (preparedStmt->useStoredFetchResult) {
      preparedStmt->useStoredFetchResult = FALSE;
      preparedStmt->fetchOkay = preparedStmt->storedFetchResult;
    } else if (!preparedStmt->fetchFinished) {
      /* printf("ppStmt: %lx\n", (unsigned long) preparedStmt->ppStmt); */
      step_result = sqlite3_step(preparedStmt->ppStmt);
      if (step_result == SQLITE_ROW) {
        preparedStmt->fetchOkay = TRUE;
      } else if (step_result == SQLITE_DONE) {
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->fetchFinished = TRUE;
      } else {
        setDbErrorMsg("sqlFetch", "sqlite3_step",
                      sqlite3_db_handle(preparedStmt->ppStmt));
        logError(printf("sqlFetch: sqlite3_step error: %s\n",
                        sqlite3_errmsg(sqlite3_db_handle(preparedStmt->ppStmt))););
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->fetchFinished = TRUE;
        raise_error(DATABASE_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("sqlFetch --> %d\n", preparedStmt->fetchOkay););
    return preparedStmt->fetchOkay;
  } /* sqlFetch */



static boolType sqlGetAutoCommit (databaseType database)

  {
    dbType db;
    boolType autoCommit;

  /* sqlGetAutoCommit */
    logFunction(printf("sqlGetAutoCommit(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (unlikely(db->connection == NULL)) {
      logError(printf("sqlGetAutoCommit: Database is not open.\n"););
      raise_error(RANGE_ERROR);
      autoCommit = FALSE;
    } else {
      autoCommit = sqlite3_get_autocommit(db->connection) != 0;
    } /* if */
    logFunction(printf("sqlGetAutoCommit --> %d\n", autoCommit););
    return autoCommit;
  } /* sqlGetAutoCommit */



static boolType sqlIsNull (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    boolType isNull;

  /* sqlIsNull */
    logFunction(printf("sqlIsNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlIsNull: Fetch okay: %d, column: " FMT_D ", max column: %u.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      isNull = FALSE;
    } else {
      isNull = sqlite3_column_type(preparedStmt->ppStmt, (int) column - 1) == SQLITE_NULL;
    } /* if */
    logFunction(printf("sqlIsNull --> %s\n", isNull ? "TRUE" : "FALSE"););
    return isNull;
  } /* sqlIsNull */



static sqlStmtType sqlPrepare (databaseType database,
    const const_striType sqlStatementStri)

  {
    dbType db;
    cstriType query;
    memSizeType queryLength;
    int prepare_result;
    int column_count;
    errInfoType err_info = OKAY_NO_ERROR;
    preparedStmtType preparedStmt;

  /* sqlPrepare */
    logFunction(printf("sqlPrepare(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) database,
                       striAsUnquotedCStri(sqlStatementStri)););
    db = (dbType) database;
    if (unlikely(db->connection == NULL)) {
      logError(printf("sqlPrepare: Database is not open.\n"););
      err_info = RANGE_ERROR;
      preparedStmt = NULL;
    } else {
      query = stri_to_cstri8_buf(sqlStatementStri, &queryLength);
      if (unlikely(query == NULL)) {
        err_info = MEMORY_ERROR;
        preparedStmt = NULL;
      } else {
        if (unlikely(queryLength > INT_MAX)) {
          /* It is not possible to cast queryLength to int. */
          logError(printf("sqlPrepare: Statement string too long (length = " FMT_U_MEM ")\n",
                          queryLength););
          err_info = RANGE_ERROR;
          preparedStmt = NULL;
        } else if (unlikely(!ALLOC_RECORD2(preparedStmt, preparedStmtRecord,
                                           count.prepared_stmt, count.prepared_stmt_bytes))) {
          err_info = MEMORY_ERROR;
        } else {
          memset(preparedStmt, 0, sizeof(preparedStmtRecord));
          prepare_result = sqlite3_prepare(db->connection,
                                           query,
                                           (int) queryLength,
                                           &preparedStmt->ppStmt,
                                           NULL);
          if (unlikely(prepare_result != SQLITE_OK)) {
            setDbErrorMsg("sqlPrepare", "sqlite3_prepare", db->connection);
            logError(printf("sqlPrepare: sqlite3_prepare error %d: %s\n",
                            prepare_result, sqlite3_errmsg(db->connection)););
            FREE_RECORD2(preparedStmt, preparedStmtRecord,
                         count.prepared_stmt, count.prepared_stmt_bytes);
            err_info = DATABASE_ERROR;
            preparedStmt = NULL;
          } else {
            column_count = sqlite3_column_count(preparedStmt->ppStmt);
            if (unlikely(column_count < 0)) {
              dbInconsistent("sqlPrepare", "sqlite3_column_count");
              logError(printf("sqlPrepare: Sqlite3_column_count "
                              "returns negative column count: %d\n",
                              column_count););
              err_info = DATABASE_ERROR;
              preparedStmt->param_array = NULL;
            } else {
              preparedStmt->usage_count = 1;
              preparedStmt->sqlFunc = db->sqlFunc;
              preparedStmt->result_column_count = (unsigned int) column_count;
              preparedStmt->executeSuccessful = FALSE;
              preparedStmt->fetchOkay = FALSE;
              preparedStmt->fetchFinished = TRUE;
              err_info = setupParameters(preparedStmt);
            } /* if */
            if (unlikely(err_info != OKAY_NO_ERROR)) {
              freePreparedStmt((sqlStmtType) preparedStmt);
              preparedStmt = NULL;
            } /* if */
          } /* if */
        } /* if */
        free_cstri8(query, sqlStatementStri);
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("sqlPrepare --> " FMT_U_MEM "\n",
                       (memSizeType) preparedStmt););
    return (sqlStmtType) preparedStmt;
  } /* sqlPrepare */



static void sqlRollback (databaseType database)

  {
    dbType db;

  /* sqlRollback */
    logFunction(printf("sqlRollback(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (unlikely(db->connection == NULL)) {
      logError(printf("sqlRollback: Database is not open.\n"););
      raise_error(RANGE_ERROR);
    } else {
      if (sqlite3_get_autocommit(db->connection) != 0) {
        sqlite3_exec(db->connection, "ROLLBACK", NULL, NULL, NULL);
        sqlite3_exec(db->connection, "BEGIN", NULL, NULL, NULL);
      } /* if */
    } /* if */
    logFunction(printf("sqlRollback -->\n"););
  } /* sqlRollback */



static void sqlSetAutoCommit (databaseType database, boolType autoCommit)

  {
    dbType db;
    boolType inAutoCommitMode;

  /* sqlSetAutoCommit */
    logFunction(printf("sqlSetAutoCommit(" FMT_U_MEM ", %d)\n",
                       (memSizeType) database, autoCommit););
    db = (dbType) database;
    if (unlikely(db->connection == NULL)) {
      logError(printf("sqlSetAutoCommit: Database is not open.\n"););
      raise_error(RANGE_ERROR);
    } else {
      inAutoCommitMode = sqlite3_get_autocommit(db->connection) != 0;
      if (inAutoCommitMode != autoCommit) {
        if (autoCommit) {
          sqlite3_exec(db->connection, "COMMIT", NULL, NULL, NULL);
        } else {
          sqlite3_exec(db->connection, "BEGIN", NULL, NULL, NULL);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("sqlSetAutoCommit -->\n"););
  } /* sqlSetAutoCommit */



static intType sqlStmtColumnCount (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    intType columnCount;

  /* sqlStmtColumnCount */
    logFunction(printf("sqlStmtColumnCount(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(preparedStmt->result_column_count > INTTYPE_MAX)) {
      logError(printf("sqlStmtColumnCount: "
                      "Result_array_size does not fit into an integer.\n"););
      raise_error(RANGE_ERROR);
      columnCount = 0;
    } else {
      columnCount = (intType) preparedStmt->result_column_count;
    } /* if */
    logFunction(printf("sqlStmtColumnCount --> " FMT_D "\n", columnCount););
    return columnCount;
  } /* sqlStmtColumnCount */



static striType sqlStmtColumnName (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    const_cstriType col_name;
    errInfoType err_info = OKAY_NO_ERROR;
    striType name;

  /* sqlStmtColumnName */
    logFunction(printf("sqlStmtColumnName(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlStmtColumnName: column: " FMT_D ", max column: %u.\n",
                      column, preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      name = NULL;
    } else {
      col_name = sqlite3_column_name(preparedStmt->ppStmt, (int) column - 1);
      if (unlikely(col_name == NULL)) {
        raise_error(MEMORY_ERROR);
        name = NULL;
      } else {
        name = cstri8_to_stri(col_name, &err_info);
        if (unlikely(name == NULL)) {
          raise_error(err_info);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("sqlStmtColumnName --> \"%s\"\n",
                       striAsUnquotedCStri(name)););
    return name;
  } /* sqlStmtColumnName */



static boolType setupFuncTable (void)

  { /* setupFuncTable */
    if (sqlFunc == NULL) {
      if (ALLOC_RECORD(sqlFunc, sqlFuncRecord, count.sql_func)) {
        memset(sqlFunc, 0, sizeof(sqlFuncRecord));
        sqlFunc->freeDatabase       = &freeDatabase;
        sqlFunc->freePreparedStmt   = &freePreparedStmt;
        sqlFunc->sqlBindBigInt      = &sqlBindBigInt;
        sqlFunc->sqlBindBigRat      = &sqlBindBigRat;
        sqlFunc->sqlBindBool        = &sqlBindBool;
        sqlFunc->sqlBindBStri       = &sqlBindBStri;
        sqlFunc->sqlBindDuration    = &sqlBindDuration;
        sqlFunc->sqlBindFloat       = &sqlBindFloat;
        sqlFunc->sqlBindInt         = &sqlBindInt;
        sqlFunc->sqlBindNull        = &sqlBindNull;
        sqlFunc->sqlBindStri        = &sqlBindStri;
        sqlFunc->sqlBindTime        = &sqlBindTime;
        sqlFunc->sqlClose           = &sqlClose;
        sqlFunc->sqlColumnBigInt    = &sqlColumnBigInt;
        sqlFunc->sqlColumnBigRat    = &sqlColumnBigRat;
        sqlFunc->sqlColumnBool      = &sqlColumnBool;
        sqlFunc->sqlColumnBStri     = &sqlColumnBStri;
        sqlFunc->sqlColumnDuration  = &sqlColumnDuration;
        sqlFunc->sqlColumnFloat     = &sqlColumnFloat;
        sqlFunc->sqlColumnInt       = &sqlColumnInt;
        sqlFunc->sqlColumnStri      = &sqlColumnStri;
        sqlFunc->sqlColumnTime      = &sqlColumnTime;
        sqlFunc->sqlCommit          = &sqlCommit;
        sqlFunc->sqlExecute         = &sqlExecute;
        sqlFunc->sqlFetch           = &sqlFetch;
        sqlFunc->sqlGetAutoCommit   = &sqlGetAutoCommit;
        sqlFunc->sqlIsNull          = &sqlIsNull;
        sqlFunc->sqlPrepare         = &sqlPrepare;
        sqlFunc->sqlRollback        = &sqlRollback;
        sqlFunc->sqlSetAutoCommit   = &sqlSetAutoCommit;
        sqlFunc->sqlStmtColumnCount = &sqlStmtColumnCount;
        sqlFunc->sqlStmtColumnName  = &sqlStmtColumnName;
      } /* if */
    } /* if */
    return sqlFunc != NULL;
  } /* setupFuncTable */



databaseType sqlOpenLite (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  {
    striType fileName;
    const_cstriType fileName8;
    const const_cstriType fileNameMemory = ":memory:";
    const strElemType dbExtension[] = {'.', 'd', 'b'};
    const memSizeType extensionLength = sizeof(dbExtension) / sizeof(strElemType);
    striType dbNameWithExtension = NULL;
    sqlite3 *connection;
    errInfoType err_info = OKAY_NO_ERROR;
    int open_result;
    dbType database;

  /* sqlOpenLite */
    logFunction(printf("sqlOpenLite(\"%s\", ",
                       striAsUnquotedCStri(dbName));
                printf("\"%s\", ", striAsUnquotedCStri(user));
                printf("\"%s\")\n", striAsUnquotedCStri(password)););
    if (!findDll()) {
      logError(printf("sqlOpenLite: findDll() failed\n"););
      err_info = DATABASE_ERROR;
      database = NULL;
    } else if (unlikely(host->size != 0 || port != 0)) {
      dbLibError("sqlOpenLite", "sqlOpenLite",
                 "Host or port has been specified.\n");
      logError(printf("sqlOpenLite: Host or port has been specified.\n"););
      err_info = DATABASE_ERROR;
      database = NULL;
    } else {
      if (dbName->size == 0) {
        /* Use in memory database: */
        fileName = NULL;
        fileName8 = fileNameMemory;
      } else {
        if (cmdFileType(dbName) == FILE_REGULAR) {
          fileName = cmdToOsPath(dbName);
        } else if (dbName->size < extensionLength ||
                   memcmp(&dbName->mem[dbName->size - extensionLength],
                          dbExtension, sizeof(dbExtension)) != 0) {
          if (unlikely(dbName->size > MAX_STRI_LEN - extensionLength ||
                       !ALLOC_STRI_SIZE_OK(dbNameWithExtension,
                                           dbName->size + extensionLength))) {
            err_info = MEMORY_ERROR;
            fileName = NULL;
          } else {
            dbNameWithExtension->size = dbName->size + extensionLength;
            memcpy(dbNameWithExtension->mem, dbName->mem,
                   dbName->size * sizeof(strElemType));
            memcpy(&dbNameWithExtension->mem[dbName->size], dbExtension,
                   sizeof(dbExtension));
            if (cmdFileType(dbNameWithExtension) == FILE_REGULAR) {
              fileName = cmdToOsPath(dbNameWithExtension);
            } else {
              dbLibError("sqlOpenLite", "sqlite3_open",
                         "File \"%s\" not found.\n",
                         striAsUnquotedCStri(dbNameWithExtension));
              logError(printf("sqlOpenLite: File \"%s\" not found.\n",
                              striAsUnquotedCStri(dbNameWithExtension)););
              err_info = DATABASE_ERROR;
              fileName = NULL;
            } /* if */
            FREE_STRI(dbNameWithExtension, dbNameWithExtension->size);
          } /* if */
        } else {
          dbLibError("sqlOpenLite", "sqlite3_open",
                     "File \"%s\" not found.\n",
                     striAsUnquotedCStri(dbName));
          logError(printf("sqlOpenLite: File \"%s\" not found.\n",
                          striAsUnquotedCStri(dbName)););
          err_info = DATABASE_ERROR;
          fileName = NULL;
        } /* if */
        /* printf("fileName: %s\n", striAsUnquotedCStri(fileName)); */
        if (fileName == NULL) {
          /* err_info was set before. */
          fileName8 = NULL;
        } else {
          fileName8 = stri_to_cstri8(fileName, &err_info);
        } /* if */
      } /* if */
      if (fileName8 == NULL) {
        /* err_info was set before. */
        database = NULL;
      } else {
        open_result = sqlite3_open(fileName8, &connection);
        if (open_result != SQLITE_OK) {
          if (connection != NULL) {
            setDbErrorMsg("sqlOpenLite", "sqlite3_open", connection);
            logError(printf("sqlOpenLite: sqlite3_open error %d: %s\n",
                            open_result, sqlite3_errmsg(connection)););
            sqlite3_close(connection);
          } /* if */
          err_info = DATABASE_ERROR;
          database = NULL;
        } else if (unlikely(!setupFuncTable() ||
                            !ALLOC_RECORD2(database, dbRecord,
                                           count.database, count.database_bytes))) {
          err_info = MEMORY_ERROR;
          sqlite3_close(connection);
          database = NULL;
        } else {
          memset(database, 0, sizeof(dbRecord));
          database->usage_count = 1;
          database->sqlFunc = sqlFunc;
          database->driver = DB_CATEGORY_SQLITE;
          database->connection = connection;
        } /* if */
      } /* if */
      if (fileName8 != NULL && fileName8 != fileNameMemory) {
        free_cstri8(fileName8, fileName);
      } /* if */
      if (fileName != NULL) {
        strDestr(fileName);
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("sqlOpenLite --> " FMT_U_MEM "\n",
                       (memSizeType) database););
    return (databaseType) database;
  } /* sqlOpenLite */

#else



databaseType sqlOpenLite (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  { /* sqlOpenLite */
    logError(printf("sqlOpenLite(\"%s\", ",
                    striAsUnquotedCStri(host));
             printf(FMT_D ", \"%s\", ",
                    port, striAsUnquotedCStri(dbName));
             printf("\"%s\", ", striAsUnquotedCStri(user));
             printf("\"%s\"): SQLite driver not present.\n",
                    striAsUnquotedCStri(password)););
    raise_error(RANGE_ERROR);
    return NULL;
  } /* sqlOpenLite */

#endif
