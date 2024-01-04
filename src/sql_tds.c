/********************************************************************/
/*                                                                  */
/*  sql_tds.c    Database access functions for Tabular Data Stream. */
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
/*  File: seed7/src/sql_tds.c                                       */
/*  Changes: 2017, 2019, 2020  Thomas Mertes                        */
/*  Content: Database access functions for Tabular Data Stream.     */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#ifdef TDS_INCLUDE
#if TDS_INCLUDE_SYBFRONT_H
#include "sybfront.h"
#endif
#include TDS_INCLUDE
#endif

#include "common.h"
#include "data_rtl.h"
#include "striutl.h"
#include "heaputl.h"
#include "numutl.h"
#include "int_rtl.h"
#include "flt_rtl.h"
#include "str_rtl.h"
#include "tim_rtl.h"
#include "bst_rtl.h"
#include "big_drv.h"
#include "rtl_err.h"
#include "dll_drv.h"
#include "sql_base.h"
#include "sql_drv.h"

#ifdef TDS_INCLUDE


typedef struct {
    uintType     usage_count;
    sqlFuncType  sqlFunc;
    intType      driver;
    DBPROCESS   *dbproc;
    boolType     autoCommit;
  } dbRecordTds, *dbType;

typedef struct {
    striType     buffer;
  } bindDataRecordTds, *bindDataType;

typedef struct {
    int          buffer_type;
    memSizeType  buffer_length;
    void        *buffer;
  } resultDataRecordTds, *resultDataType;

typedef struct {
    uintType        usage_count;
    sqlFuncType     sqlFunc;
    DBPROCESS      *dbproc;
    striType       *stmtPartArray;
    memSizeType     stmtPartArrayCharCount;
    memSizeType     param_array_size;
    bindDataType    param_array;
    memSizeType     result_array_size;
    resultDataType  result_array;
    boolType        executeSuccessful;
    boolType        fetchOkay;
    boolType        fetchFinished;
  } preparedStmtRecordTds, *preparedStmtType;

typedef struct {
    DBPROCESS *dbproc;
    int severity;
    int dberr;
    int oserr;
    char *dberrstr;
    char *oserrstr;
  } errorDescrRecord, *errorDescrType;

#define SQL_MAX_NUMERIC_LEN 32
typedef struct {
    unsigned char precision;
    unsigned char scale;
    unsigned char sign;
    unsigned char val[SQL_MAX_NUMERIC_LEN];
  } numericRecord, *numericType;

typedef const numericRecord *const_numericType;

static sqlFuncType sqlFunc = NULL;
static striType quoteQuote = NULL;

unsigned int precisionToBytes[] = {0, /* not used */
                                   1,  1,  2,  2,  3,  3,  3,  4,  4,  5,
                                   5,  5,  6,  6,  7,  7,  8,  8,  8,  9,
                                   9, 10, 10, 10, 11, 11, 12, 12, 13, 13,
                                  13, 14, 14, 15, 15, 15, 16, 16, 17, 17,
                                  18, 18, 18, 19, 19, 20, 20, 20, 21, 21,
                                  22, 22, 23, 23, 23, 24, 24, 25, 25, 25,
                                  26, 26, 27, 27, 27, 28, 28, 29, 29, 30,
                                  30, 30, 31, 31, 32, 32, 32, 33, 33, 34,
                                  34, 35, 35, 35, 36, 36, 37, 37, 37, 38,
                                  38, 39, 39, 40, 40, 40, 41, 41, 42, 42};

#define DEFAULT_DECIMAL_SCALE 1000
#define INITIAL_ARRAY_SIZE 256
#define STMT_TABLE_SIZE_INCREMENT 256
#define QUOTE_IN_STRING_LEN 2
#define MAX_TIME_LITERAL_LENGTH 34


#ifdef TDS_DLL
typedef RETCODE (*tp_dbbind) (DBPROCESS *dbproc,
                              int        column,
                              int        vartype,
                              DBINT      varlen,
                              BYTE      *varaddr);
typedef void (*tp_dbclose) (DBPROCESS * dbproc);
typedef RETCODE (*tp_dbcmd) (DBPROCESS *dbproc, const char *cmdstring);
typedef DBINT (*tp_dbcollen) (DBPROCESS *dbproc, int column);
typedef char *(*tp_dbcolname) (DBPROCESS *dbproc, int column);
typedef int (*tp_dbcoltype) (DBPROCESS *dbproc, int column);
typedef BYTE *(*tp_dbdata) (DBPROCESS *dbproc, int column);
typedef DBINT (*tp_dbdatlen) (DBPROCESS *dbproc, int column);
typedef EHANDLEFUNC (*tp_dberrhandle) (EHANDLEFUNC handler);
typedef void (*tp_dbexit) (void);
typedef RETCODE (*tp_dbinit) (void);
typedef LOGINREC *(*tp_dblogin) (void);
typedef void (*tp_dbloginfree) (LOGINREC *loginptr);
typedef MHANDLEFUNC (*tp_dbmsghandle) (MHANDLEFUNC handler);
typedef RETCODE (*tp_dbnextrow) (DBPROCESS *dbproc);
typedef int (*tp_dbnumcols) (DBPROCESS *dbproc);
typedef RETCODE (*tp_dbresults) (DBPROCESS *dbproc);
typedef RETCODE (*tp_dbsetlname) (LOGINREC * login, const char *value, int which);
typedef RETCODE (*tp_dbsqlexec) (DBPROCESS *dbproc);
typedef RETCODE (*tp_dbtablecolinfo) (DBPROCESS *dbproc,
                                      DBINT      column,
                                      DBCOL     *pdbcol);
typedef RETCODE (*tp_dbuse) (DBPROCESS * dbproc, const char *name);
typedef DBPROCESS *(*tp_tdsdbopen) (LOGINREC * login, const char *server, int msdblib);

static tp_dbbind         ptr_dbbind;
static tp_dbclose        ptr_dbclose;
static tp_dbcmd          ptr_dbcmd;
static tp_dbcollen       ptr_dbcollen;
static tp_dbcolname      ptr_dbcolname;
static tp_dbcoltype      ptr_dbcoltype;
static tp_dbdata         ptr_dbdata;
static tp_dbdatlen       ptr_dbdatlen;
static tp_dberrhandle    ptr_dberrhandle;
static tp_dbexit         ptr_dbexit;
static tp_dbinit         ptr_dbinit;
static tp_dblogin        ptr_dblogin;
static tp_dbloginfree    ptr_dbloginfree;
static tp_dbmsghandle    ptr_dbmsghandle;
static tp_dbnextrow      ptr_dbnextrow;
static tp_dbnumcols      ptr_dbnumcols;
static tp_dbresults      ptr_dbresults;
static tp_dbsetlname     ptr_dbsetlname;
static tp_dbsqlexec      ptr_dbsqlexec;
static tp_dbtablecolinfo ptr_dbtablecolinfo;
static tp_dbuse          ptr_dbuse;
static tp_tdsdbopen      ptr_tdsdbopen;

#define dbbind         ptr_dbbind
#define dbclose        ptr_dbclose
#define dbcmd          ptr_dbcmd
#define dbcollen       ptr_dbcollen
#define dbcolname      ptr_dbcolname
#define dbcoltype      ptr_dbcoltype
#define dbdata         ptr_dbdata
#define dbdatlen       ptr_dbdatlen
#define dberrhandle    ptr_dberrhandle
#define dbexit         ptr_dbexit
#define dbinit         ptr_dbinit
#define dblogin        ptr_dblogin
#define dbloginfree    ptr_dbloginfree
#define dbmsghandle    ptr_dbmsghandle
#define dbnextrow      ptr_dbnextrow
#define dbnumcols      ptr_dbnumcols
#define dbresults      ptr_dbresults
#define dbsetlname     ptr_dbsetlname
#define dbsqlexec      ptr_dbsqlexec
#define dbtablecolinfo ptr_dbtablecolinfo
#define dbuse          ptr_dbuse
#define tdsdbopen      ptr_tdsdbopen



static boolType setupDll (const char *dllName)

  {
    static void *dbDll = NULL;

  /* setupDll */
    logFunction(printf("setupDll(\"%s\")\n", dllName););
    if (dbDll == NULL) {
      dbDll = dllOpen(dllName);
      if (dbDll != NULL) {
        if ((dbbind         = (tp_dbbind)         dllFunc(dbDll, "dbbind"))         == NULL ||
            (dbclose        = (tp_dbclose)        dllFunc(dbDll, "dbclose"))        == NULL ||
            (dbcmd          = (tp_dbcmd)          dllFunc(dbDll, "dbcmd"))          == NULL ||
            (dbcollen       = (tp_dbcollen)       dllFunc(dbDll, "dbcollen"))       == NULL ||
            (dbcolname      = (tp_dbcolname)      dllFunc(dbDll, "dbcolname"))      == NULL ||
            (dbcoltype      = (tp_dbcoltype)      dllFunc(dbDll, "dbcoltype"))      == NULL ||
            (dbdata         = (tp_dbdata)         dllFunc(dbDll, "dbdata"))         == NULL ||
            (dbdatlen       = (tp_dbdatlen)       dllFunc(dbDll, "dbdatlen"))       == NULL ||
            (dberrhandle    = (tp_dberrhandle)    dllFunc(dbDll, "dberrhandle"))    == NULL ||
            (dbexit         = (tp_dbexit)         dllFunc(dbDll, "dbexit"))         == NULL ||
            (dbinit         = (tp_dbinit)         dllFunc(dbDll, "dbinit"))         == NULL ||
            (dblogin        = (tp_dblogin)        dllFunc(dbDll, "dblogin"))        == NULL ||
            (dbloginfree    = (tp_dbloginfree)    dllFunc(dbDll, "dbloginfree"))    == NULL ||
            (dbmsghandle    = (tp_dbmsghandle)    dllFunc(dbDll, "dbmsghandle"))    == NULL ||
            (dbnextrow      = (tp_dbnextrow)      dllFunc(dbDll, "dbnextrow"))      == NULL ||
            (dbnumcols      = (tp_dbnumcols)      dllFunc(dbDll, "dbnumcols"))      == NULL ||
            (dbresults      = (tp_dbresults)      dllFunc(dbDll, "dbresults"))      == NULL ||
            (dbsetlname     = (tp_dbsetlname)     dllFunc(dbDll, "dbsetlname"))     == NULL ||
            (dbsqlexec      = (tp_dbsqlexec)      dllFunc(dbDll, "dbsqlexec"))      == NULL ||
            (dbtablecolinfo = (tp_dbtablecolinfo) dllFunc(dbDll, "dbtablecolinfo")) == NULL ||
            (dbuse          = (tp_dbuse)          dllFunc(dbDll, "dbuse"))          == NULL ||
            (tdsdbopen      = (tp_tdsdbopen)      dllFunc(dbDll, "tdsdbopen"))      == NULL) {
          dbDll = NULL;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setupDll --> %d\n", dbDll != NULL););
    return dbDll != NULL;
  } /* setupDll */



static boolType findDll (void)

  {
    const char *dllList[] = { TDS_DLL };
    unsigned int pos;
    boolType found = FALSE;

  /* findDll */
    for (pos = 0; pos < sizeof(dllList) / sizeof(char *) && !found; pos++) {
      found = setupDll(dllList[pos]);
    } /* for */
    if (!found) {
      dllErrorMessage("sqlOpenTds", "findDll", dllList,
                      sizeof(dllList) / sizeof(char *));
    } /* if */
    return found;
  } /* findDll */

#else

#define findDll() TRUE

#endif



static void sqlClose (databaseType database);



static int err_handler (DBPROCESS *dbproc, int severity, int dberr,
    int oserr, char *dberrstr, char *oserrstr)

  { /* err_handler */
    logFunction(printf("err_handler(*, %d, %d, %d, %s, %s)\n",
                       severity, dberr, oserr, dberrstr, oserrstr););
    snprintf(dbError.message, DB_ERR_MESSAGE_SIZE,
             "%s\ndberr: %d\n%s\noserr: %d\nseverity: %d\n",
             dberrstr, dberr, oserrstr, oserr, severity);
    dbError.errorCode = dberr;
    if (dberr == SYBETIME) {
      return INT_TIMEOUT;
    } else {
      return INT_CANCEL;
    } /* if */
  } /* err_handler */



static int msg_handler (DBPROCESS *dbproc, DBINT msgno, int msgstate,
    int severity, char *msgtext, char *srvname, char *procname, int line)

  { /* msg_handler */
    logFunction(printf("msg_handler(*, %d, %d, %d, %s, %s, %s, %d)\n",
                       msgno, msgstate, severity, msgtext, srvname, procname, line););
    return 0;
  } /* msg_handler */



static void setDbErrorMsg (const char *funcName, const char *dbFuncName)

  { /* setDbErrorMsg */
    dbError.funcName = funcName;
    dbError.dbFuncName = dbFuncName;
  } /* setDbErrorMsg */



static void doExecute (DBPROCESS *dbproc, const const_cstriType query,
    errInfoType *err_info)

  {
    RETCODE retCode;

  /* doExecute */
    logFunction(printf("doExecute(*, \"%s\", %d)\n", query, *err_info););
    if (likely(*err_info == OKAY_NO_ERROR)) {
      if (unlikely(dbcmd(dbproc, query) != SUCCEED)) {
        logError(printf("doExecute: dbcmd(" FMT_U_MEM ", \"%s\") failed.\n",
                        (memSizeType) dbproc, query););
        *err_info = DATABASE_ERROR;
      } else if (unlikely(dbsqlexec(dbproc) != SUCCEED)) {
        logError(printf("doExecute: dbsqlexec(" FMT_U_MEM ") with \"%s\" failed.\n",
                        (memSizeType) dbproc, query););
        *err_info = DATABASE_ERROR;
      } else {
        retCode = dbresults(dbproc);
        /* printf("dbresults returns: %d\n", retCode); */
        if (unlikely(retCode == FAIL)) {
          logError(printf("doExecute: dbresults() failed.\n"););
          *err_info = DATABASE_ERROR;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("doExecute --> (err_info=%d)\n", *err_info););
  } /* doExecute */



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
    FREE_RECORD2(db, dbRecordTds, count.database, count.database_bytes);
    logFunction(printf("freeDatabase -->\n"););
  } /* freeDatabase */



static void freeStmtParts (striType *stmtPartArray, memSizeType partArraySize)

  {
    memSizeType pos;

  /* freeStmtParts */
    for (pos = 0; pos < partArraySize; pos++) {
      FREE_STRI(stmtPartArray[pos], stmtPartArray[pos]->size);
    } /* for */
  } /* freeStmtParts */



/**
 *  Closes a prepared statement and frees the memory used by it.
 */
static void freePreparedStmt (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    memSizeType pos;

  /* freePreparedStmt */
    logFunction(printf("freePreparedStmt(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    freeStmtParts(preparedStmt->stmtPartArray, preparedStmt->param_array_size + 1);
    if (preparedStmt->param_array != NULL) {
      for (pos = 0; pos < preparedStmt->param_array_size; pos++) {
        strDestr(preparedStmt->param_array[pos].buffer);
      } /* for */
      FREE_TABLE(preparedStmt->param_array, bindDataRecordTds, preparedStmt->param_array_size);
    } /* if */
    if (preparedStmt->result_array != NULL) {
      for (pos = 0; pos < preparedStmt->result_array_size; pos++) {
        if (preparedStmt->result_array[pos].buffer != NULL) {
          free(preparedStmt->result_array[pos].buffer);
        } /* if */
      } /* for */
      FREE_TABLE(preparedStmt->result_array, resultDataRecordTds, preparedStmt->result_array_size);
    } /* if */
    FREE_RECORD2(preparedStmt, preparedStmtRecordTds,
                 count.prepared_stmt, count.prepared_stmt_bytes);
    logFunction(printf("freePreparedStmt -->\n"););
  } /* freePreparedStmt */



#if ANY_LOG_ACTIVE
static const char *nameOfBufferType (int buffer_type)

  {
    static char buffer[50];
    const char *typeName;

  /* nameOfBufferType */
    logFunction(printf("nameOfBufferType(%d)\n", buffer_type););
    switch (buffer_type) {
      case SYBVOID:             typeName = "SYBVOID"; break;
      case SYBIMAGE:            typeName = "SYBIMAGE"; break;
      case SYBTEXT:             typeName = "SYBTEXT"; break;
      case SYBVARBINARY:        typeName = "SYBVARBINARY"; break;
      case SYBINTN:             typeName = "SYBINTN"; break;
      case SYBVARCHAR:          typeName = "SYBVARCHAR"; break;
      case SYBMSDATE:           typeName = "SYBMSDATE"; break;
      case SYBMSTIME:           typeName = "SYBMSTIME"; break;
      case SYBMSDATETIME2:      typeName = "SYBMSDATETIME2"; break;
      case SYBMSDATETIMEOFFSET: typeName = "SYBMSDATETIMEOFFSET"; break;
      case SYBBINARY:           typeName = "SYBBINARY"; break;
      case SYBCHAR:             typeName = "SYBCHAR"; break;
      case SYBINT1:             typeName = "SYBINT1"; break;
      case SYBDATE:             typeName = "SYBDATE"; break;
      case SYBBIT:              typeName = "SYBBIT"; break;
      case SYBTIME:             typeName = "SYBTIME"; break;
      case SYBINT2:             typeName = "SYBINT2"; break;
      case SYBINT4:             typeName = "SYBINT4"; break;
      case SYBDATETIME4:        typeName = "SYBDATETIME4"; break;
      case SYBREAL:             typeName = "SYBREAL"; break;
      case SYBMONEY:            typeName = "SYBMONEY"; break;
      case SYBDATETIME:         typeName = "SYBDATETIME"; break;
      case SYBFLT8:             typeName = "SYBFLT8"; break;
      case SYBNTEXT:            typeName = "SYBNTEXT"; break;
      case SYBNVARCHAR:         typeName = "SYBNVARCHAR"; break;
      case SYBBITN:             typeName = "SYBBITN"; break;
      case SYBDECIMAL:          typeName = "SYBDECIMAL"; break;
      case SYBNUMERIC:          typeName = "SYBNUMERIC"; break;
      case SYBFLTN:             typeName = "SYBFLTN"; break;
      case SYBMONEYN:           typeName = "SYBMONEYN"; break;
      case SYBDATETIMN:         typeName = "SYBDATETIMN"; break;
      case SYBMONEY4:           typeName = "SYBMONEY4"; break;
      case SYBINT8:             typeName = "SYBINT8"; break;
      case SYBBIGDATETIME:      typeName = "SYBBIGDATETIME"; break;
      case SYBBIGTIME:          typeName = "SYBBIGTIME"; break;
      default:
        sprintf(buffer, "%d", buffer_type);
        typeName = buffer;
        break;
    } /* switch */
    logFunction(printf("nameOfBufferType --> %s\n", typeName););
    return typeName;
  } /* nameOfBufferType */
#endif



static striType *storeStmtPart (striType *stmtPartArray, memSizeType partIndex,
    striType stmtPart, memSizeType stmtPartLength)

  {
    striType *resizedStmtPartArray;
    striType copiedStmtPart;

  /* storeStmtPart */
    if ((partIndex & (STMT_TABLE_SIZE_INCREMENT - 1)) == 0) {
      resizedStmtPartArray = REALLOC_TABLE(stmtPartArray, striType, partIndex,
                                           partIndex + STMT_TABLE_SIZE_INCREMENT);
      if (unlikely(resizedStmtPartArray == NULL)) {
        if (stmtPartArray != NULL) {
          freeStmtParts(stmtPartArray, partIndex);
          stmtPartArray = NULL;
        } /* if */
      } else {
        stmtPartArray = resizedStmtPartArray;
        COUNT3_TABLE(striType, partIndex, partIndex + STMT_TABLE_SIZE_INCREMENT);
      } /* if */
    } /* if */
    if (likely(stmtPartArray != NULL)) {
      if (unlikely(!ALLOC_STRI_SIZE_OK(copiedStmtPart, stmtPartLength))) {
        freeStmtParts(stmtPartArray, partIndex);
        stmtPartArray = NULL;
      } else {
        copiedStmtPart->size = stmtPartLength;
        memcpy(copiedStmtPart->mem, stmtPart->mem, stmtPartLength * sizeof(strElemType));
        /* printf("stmtPart: \"%s\"\n", striAsUnquotedCStri(copiedStmtPart)); */
        stmtPartArray[partIndex] = copiedStmtPart;
      } /* if */
    } /* if */
    return stmtPartArray;
  } /* storeStmtPart */



/**
 *  Process the bind variables in a statement string.
 *  Literals and comments are processed to avoid the misinterpretation
 *  of question marks (?).
 */
static striType *processStatementStri (const const_striType sqlStatementStri,
    memSizeType *numBindParameters, errInfoType *err_info)

  {
    memSizeType pos = 0;
    strElemType ch;
    strElemType delimiter;
    memSizeType destPos = 0;
    striType processed;
    memSizeType partIndex = 0;
    striType *stmtPartArray = NULL;

  /* processStatementStri */
    logFunction(printf("processStatementStri(\"%s\", *, *)\n",
                       striAsUnquotedCStri(sqlStatementStri)););
    if (unlikely(sqlStatementStri->size > MAX_STRI_LEN ||
                 !ALLOC_STRI_SIZE_OK(processed, sqlStatementStri->size))) {
      *err_info = MEMORY_ERROR;
      *numBindParameters = 0;
    } else {
      while (pos < sqlStatementStri->size && *err_info == OKAY_NO_ERROR) {
        ch = sqlStatementStri->mem[pos];
        if (ch == '?') {
          if (destPos == 0 || processed->mem[destPos - 1] != ' ') {
            /* Make sure that a space precedes the bind parameter. */
            processed->mem[destPos++] = ' ';
          } /* if */
          stmtPartArray = storeStmtPart(stmtPartArray, partIndex, processed, destPos);
          if (unlikely(stmtPartArray == NULL)) {
            *err_info = MEMORY_ERROR;
          } else {
            partIndex++;
            destPos = 0;
            pos++;
            if (pos < sqlStatementStri->size && sqlStatementStri->mem[pos] != ' ') {
              /* Make sure that a space succeeds the bind parameter. */
              processed->mem[destPos++] = ' ';
            } /* if */
          } /* if */
        } else if (ch == '\'' || ch == '"') {
          delimiter = ch;
          processed->mem[destPos++] = delimiter;
          pos++;
          while (pos < sqlStatementStri->size &&
              (ch = sqlStatementStri->mem[pos]) != delimiter) {
            processed->mem[destPos++] = ch;
            pos++;
          } /* while */
          if (pos < sqlStatementStri->size) {
            processed->mem[destPos++] = delimiter;
            pos++;
          } /* if */
        } else if (ch == '/') {
          pos++;
          if (pos >= sqlStatementStri->size || sqlStatementStri->mem[pos] != '*') {
            processed->mem[destPos++] = ch;
          } else {
            pos++;
            do {
              while (pos < sqlStatementStri->size && sqlStatementStri->mem[pos] != '*') {
                pos++;
              } /* while */
              pos++;
            } while (pos < sqlStatementStri->size && sqlStatementStri->mem[pos] != '/');
            pos++;
            /* Replace the comment with a space. */
            processed->mem[destPos++] = ' ';
          } /* if */
        } else if (ch == '-') {
          pos++;
          if (pos >= sqlStatementStri->size || sqlStatementStri->mem[pos] != '-') {
            processed->mem[destPos++] = ch;
          } else {
            pos++;
            while (pos < sqlStatementStri->size && sqlStatementStri->mem[pos] != '\n') {
              pos++;
            } /* while */
            /* The final newline replaces the comment. */
          } /* if */
        } else {
          processed->mem[destPos++] = ch;
          pos++;
        } /* if */
      } /* while */
      if (likely(*err_info == OKAY_NO_ERROR)) {
        stmtPartArray = storeStmtPart(stmtPartArray, partIndex, processed, destPos);
        if (unlikely(stmtPartArray == NULL)) {
          *err_info = MEMORY_ERROR;
        } /* if */
      } /* if */
      FREE_STRI(processed, sqlStatementStri->size);
      *numBindParameters = partIndex;
    } /* if */
    logFunction(printf("processStatementStri(\"%s\", " FMT_U_MEM ", %d) --> ",
                       striAsUnquotedCStri(sqlStatementStri),
                       *numBindParameters, *err_info);
                if (stmtPartArray == NULL) {
                  printf("NULL");
                } else {
                  for (partIndex = 0; partIndex <= *numBindParameters; partIndex++) {
                    printf("\"%s\" ", striAsUnquotedCStri(stmtPartArray[partIndex]));
                  }
                }
                printf("\n"););
    return stmtPartArray;
  } /* processStatementStri */



static memSizeType getSizeOfBoundStatement (preparedStmtType preparedStmt,
    errInfoType *err_info)

  {
    memSizeType paramIndex;
    bindDataType bindData;
    memSizeType sizeOfBoundStatement;

  /* getSizeOfBoundStatement */
    sizeOfBoundStatement = preparedStmt->stmtPartArrayCharCount;
    for (paramIndex = 0; paramIndex < preparedStmt->param_array_size; paramIndex++) {
      bindData = &preparedStmt->param_array[paramIndex];
      if (unlikely(bindData->buffer == NULL)) {
        logError(printf("sqlExecute: Unbound parameter " FMT_U_MEM ".\n",
                        paramIndex + 1););
        *err_info = RANGE_ERROR;
        return 0;
      } else if (unlikely(sizeOfBoundStatement > MAX_STRI_LEN - bindData->buffer->size)) {
        *err_info = MEMORY_ERROR;
        return 0;
      } else {
        sizeOfBoundStatement += bindData->buffer->size;
      } /* if */
    } /* for */
    return sizeOfBoundStatement;
  } /* getSizeOfBoundStatement */



static striType getBoundStatement (preparedStmtType preparedStmt, errInfoType *err_info)

  {
    memSizeType paramIndex;
    memSizeType pos = 0;
    memSizeType sizeOfBoundStatement;
    striType boundStatement;

  /* getBoundStatement */
    logFunction(printf("getBoundStatement(*, %d)\n", *err_info););
    sizeOfBoundStatement = getSizeOfBoundStatement(preparedStmt, err_info);
    if (unlikely(*err_info != OKAY_NO_ERROR)) {
      boundStatement = NULL;
    } else if (unlikely(sizeOfBoundStatement > MAX_STRI_LEN ||
                        !ALLOC_STRI_SIZE_OK(boundStatement, sizeOfBoundStatement))) {
      *err_info = MEMORY_ERROR;
      boundStatement = NULL;
    } else {
      boundStatement->size = sizeOfBoundStatement;
      for (paramIndex = 0; paramIndex < preparedStmt->param_array_size; paramIndex++) {
        memcpy(&boundStatement->mem[pos], preparedStmt->stmtPartArray[paramIndex]->mem,
               preparedStmt->stmtPartArray[paramIndex]->size * sizeof(strElemType));
        pos += preparedStmt->stmtPartArray[paramIndex]->size;
        memcpy(&boundStatement->mem[pos], preparedStmt->param_array[paramIndex].buffer->mem,
               preparedStmt->param_array[paramIndex].buffer->size * sizeof(strElemType));
        pos += preparedStmt->param_array[paramIndex].buffer->size;
      } /* while */
      memcpy(&boundStatement->mem[pos], preparedStmt->stmtPartArray[paramIndex]->mem,
             preparedStmt->stmtPartArray[paramIndex]->size * sizeof(strElemType));
      /* pos += preparedStmt->stmtPartArray[paramIndex]->size; */
      /* pos should be equal to boundStatement->size */
    } /* if */
    logFunction(printf("getBoundStatement --> \"%s\"\n",
                       striAsUnquotedCStri(boundStatement)););
    return boundStatement;
  } /* getBoundStatement */



static errInfoType setStmtPartArrayCharCount (preparedStmtType preparedStmt,
    memSizeType numBindParameters)

  {
    memSizeType partIndex;
    memSizeType stmtPartArrayCharCount = 0;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setStmtPartArrayCharCount */
    logFunction(printf("setStmtPartArrayCharCount\n"););
    for (partIndex = 0; partIndex <= numBindParameters; partIndex++) {
      if (unlikely(stmtPartArrayCharCount > MAX_STRI_LEN -
                   preparedStmt->stmtPartArray[partIndex]->size)) {
        err_info = MEMORY_ERROR;
        return 0;
      } else {
        stmtPartArrayCharCount += preparedStmt->stmtPartArray[partIndex]->size;
      } /* if */
    } /* for */
    preparedStmt->stmtPartArrayCharCount = stmtPartArrayCharCount;
    logFunction(printf("setStmtPartArrayCharCount --> %d (count=" FMT_U_MEM ")\n",
                       err_info, stmtPartArrayCharCount););
    return err_info;
  } /* setStmtPartArrayCharCount */



static errInfoType setupParameters (preparedStmtType preparedStmt,
    memSizeType numBindParameters)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupParameters */
    logFunction(printf("setupParameters\n"););
    if (!ALLOC_TABLE(preparedStmt->param_array,
                     bindDataRecordTds, (memSizeType) numBindParameters)) {
      err_info = MEMORY_ERROR;
    } else {
      preparedStmt->param_array_size = (memSizeType) numBindParameters;
      memset(preparedStmt->param_array, 0,
             (memSizeType) numBindParameters * sizeof(bindDataRecordTds));
    } /* if */
    logFunction(printf("setupParameters --> %d\n", err_info););
    return err_info;
  } /* setupParameters */



static errInfoType setupResultColumn (preparedStmtType preparedStmt,
    int column_num, resultDataType columnData)

  {
    DBINT buffer_length;
    DBCOL dbcol;
    int bind_type;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupResultColumn */
    logFunction(printf("setupResultColumn: column_num=%d\n", column_num););
    dbcol.SizeOfStruct = sizeof(DBCOL);
    /* The function dbtablecolinfo() distinguishes SYBCHAR from SYBVARCHAR.  */
    /* The functions dbcoltype() and dbcolinfo() do not distinguish SYBCHAR  */
    /* from SYBVARCHAR. In sqlColumnStri() trailing spaces of SYBCHAR fields */
    /* are removed. For SYBVARCHAR fields trailing spaces are preserved.     */
    if (unlikely(dbtablecolinfo(preparedStmt->dbproc,
                                column_num, &dbcol) != SUCCEED)) {
      setDbErrorMsg("setupResultColumn", "dbtablecolinfo");
      logError(printf("setupResultColumn: dbtablecolinfo did not succeed:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } else {
      /* printf("Type: " FMT_D16 ", VarLength: %d\n", dbcol.Type, dbcol.VarLength); */
      columnData->buffer_type = dbcol.Type;
      /* printf("buffer_type: %s\n", nameOfBufferType(columnData->buffer_type)); */
      switch (columnData->buffer_type) {
        case SYBCHAR:
          bind_type = STRINGBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBVARCHAR:
          bind_type = VARYCHARBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBINT1:
          bind_type = TINYBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBINT2:
          bind_type = SMALLBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBINT4:
          bind_type = INTBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBINT8:
          bind_type = INTBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBREAL:
          bind_type = REALBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBFLT8:
          bind_type = FLT8BIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBDECIMAL:
          bind_type = DECIMALBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBNUMERIC:
          bind_type = NUMERICBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBMSDATE:
          bind_type = DATEBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBMSTIME:
          bind_type = TIMEBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBMSDATETIME2:
          bind_type = DATETIME2BIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBBINARY:
          bind_type = BINARYBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        case SYBVARBINARY:
          bind_type = VARYBINBIND;
          buffer_length = dbcollen(preparedStmt->dbproc, column_num);
          break;
        default:
          logError(printf("setupResultColumn: Column %hd has the unknown type %s.\n",
                          column_num, nameOfBufferType(columnData->buffer_type)););
          err_info = RANGE_ERROR;
          break;
      } /* switch */
      if (err_info == OKAY_NO_ERROR) {
        /* printf("buffer_length[%d]: " FMT_D32 "\n", column_num, buffer_length); */
        columnData->buffer_length = (memSizeType) buffer_length;
        /* Binding with dbbind() is not used. The data is retrieved with dbdata() instead. */
        if (FALSE && buffer_length != 0) {
          columnData->buffer = malloc((memSizeType) buffer_length);
          if (unlikely(columnData->buffer == NULL)) {
            err_info = MEMORY_ERROR;
          } else {
            memset(columnData->buffer, 0, (memSizeType) buffer_length);
            if (unlikely(dbbind(preparedStmt->dbproc, column_num, bind_type,
                                buffer_length, (BYTE *) columnData->buffer) != SUCCEED)) {
              setDbErrorMsg("setupResultColumn", "dbbind");
              logError(printf("setupResultColumn: dbbind did not succeed:\n%s\n",
                              dbError.message););
              err_info = DATABASE_ERROR;
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setupResultColumn --> %d\n", err_info););
    return err_info;
  } /* setupResultColumn */



static errInfoType setupResult (preparedStmtType preparedStmt)

  {
    int num_columns;
    int column_index;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupResult */
    logFunction(printf("setupResult\n"););
    num_columns = dbnumcols(preparedStmt->dbproc);
    /* printf("num_columns: %d\n", num_columns); */
    if (unlikely(num_columns < 0)) {
      dbInconsistent("setupResult", "dbnumcols");
      logError(printf("setupResult: dbnumcols returns negative number: %d\n",
                      num_columns););
      err_info = DATABASE_ERROR;
    } else if (num_columns == 0) {
      /* malloc(0) may return NULL, which would wrongly trigger a MEMORY_ERROR. */
      preparedStmt->result_array_size = 0;
      preparedStmt->result_array = NULL;
    } else if (unlikely(!ALLOC_TABLE(preparedStmt->result_array,
                                     resultDataRecordTds, (memSizeType) num_columns))) {
      err_info = MEMORY_ERROR;
    } else {
      preparedStmt->result_array_size = (memSizeType) num_columns;
      memset(preparedStmt->result_array, 0,
          (memSizeType) num_columns * sizeof(resultDataRecordTds));
      for (column_index = 0; column_index < num_columns &&
           err_info == OKAY_NO_ERROR; column_index++) {
        err_info = setupResultColumn(preparedStmt, column_index + 1,
                                     &preparedStmt->result_array[column_index]);
      } /* for */
    } /* if */
    logFunction(printf("setupResult --> %d\n", err_info););
    return err_info;
  } /* setupResult */



static cstriType getNumericAsCStri (const_numericType numStruct)

  {
    memSizeType numBytes;
    bigIntType mantissa;
    striType stri;
    memSizeType decimalLen;
    memSizeType decimalIdx = 0;
    memSizeType idx;
    cstriType decimal;

  /* getNumericAsCStri */
    if (unlikely(numStruct->precision == 0 || numStruct->precision > 100)) {
      raise_error(RANGE_ERROR);
      decimal = NULL;
    } else {
      numBytes = (memSizeType) precisionToBytes[numStruct->precision];
      mantissa = bigFromByteBufferBe(numBytes, numStruct->val, FALSE);
      if (unlikely(mantissa == NULL)) {
        decimal = NULL;
      } else {
        stri = bigStr(mantissa);
        if (unlikely(stri == NULL)) {
          bigDestr(mantissa);
          raise_error(MEMORY_ERROR);
          decimal = NULL;
        } else {
          if (numStruct->scale < 0) {
            decimalLen = stri->size + (memSizeType) (-numStruct->scale) + 1 /* Space for decimal point */;
          } else if (numStruct->scale <= stri->size) {
            decimalLen = stri->size + 1 /* Space for decimal point */;
          } else {
            decimalLen = (memSizeType) numStruct->scale + 1 /* Space for decimal point */;
          } /* if */
          if (numStruct->sign != 0) {
            decimalLen++; /* Space for sign */
          } /* if */
          if (unlikely(!ALLOC_CSTRI(decimal, decimalLen))) {
            bigDestr(mantissa);
            strDestr(stri);
            raise_error(MEMORY_ERROR);
          } else {
            if (numStruct->sign != 0) {
              decimal[decimalIdx++] = '-';
            } /* if */
            for (idx = 0; idx < stri->size; idx++) {
              decimal[decimalIdx++] = (char) stri->mem[idx];
            } /* for */
            /* decimal[decimalIdx] = '\0';
            printf("# \"%s\", scale: %d, size: " FMT_U_MEM ", len: " FMT_U_MEM "\n",
                decimal, numStruct->scale, stri->size, decimalLen); */
            if (numStruct->scale < 0) {
              memset(&decimal[decimalIdx], '0', (memSizeType) (-numStruct->scale));
              decimal[decimalLen - 1] = '.';
              decimal[decimalLen] = '\0';
            } else if (numStruct->scale <= stri->size) {
              memmove(&decimal[decimalLen - (memSizeType) numStruct->scale],
                      &decimal[decimalLen - (memSizeType) numStruct->scale - 1],
                      (memSizeType) numStruct->scale);
              decimal[decimalLen - (memSizeType) numStruct->scale - 1] = '.';
              decimal[decimalLen] = '\0';
            } else {
              memmove(&decimal[decimalLen - stri->size],
                      &decimal[decimalLen - (memSizeType) numStruct->scale - 1],
                      stri->size);
              memset(&decimal[decimalLen - (memSizeType) numStruct->scale], '0',
                     (memSizeType) numStruct->scale - stri->size);
              decimal[decimalLen - (memSizeType) numStruct->scale - 1] = '.';
              decimal[decimalLen] = '\0';
            } /* if */
            bigDestr(mantissa);
            strDestr(stri);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("getNumericAsCStri --> %s\n",
                       decimal != NULL ? decimal : "NULL"));
    return decimal;
  } /* getNumericAsCStri */



static intType getNumericInt (const void *buffer)

  {
    const_numericType numStruct;
    memSizeType numBytes;
    bigIntType bigIntValue;
    bigIntType powerOfTen;
    intType intValue = 0;

  /* getNumericInt */
    numStruct = (const_numericType) buffer;
    logFunction(printf("getNumericInt\n");
                printf("numStruct->precision: %u\n", numStruct->precision);
                printf("numStruct->scale: %d\n", numStruct->scale);
                printf("numStruct->sign: %u\n", numStruct->sign);
                printf("numStruct->val:");
                {
                  int pos;
                  for (pos = 0; pos < SQL_MAX_NUMERIC_LEN; pos++) {
                    printf(" %d", numStruct->val[pos]);
                  }
                  printf("\n");
                });
    if (unlikely(numStruct->scale > 0 ||
                 numStruct->precision == 0 || numStruct->precision > 100)) {
      raise_error(RANGE_ERROR);
      intValue = 0;
    } else {
      numBytes = (memSizeType) precisionToBytes[numStruct->precision];
      bigIntValue = bigFromByteBufferBe(numBytes, numStruct->val, FALSE);
#if 0
      if (bigIntValue != NULL) {
        printf("numStruct->val: ");
        prot_bigint(bigIntValue);
        printf("\n");
      }
#endif
      if (bigIntValue != NULL && numStruct->scale < 0) {
        powerOfTen = bigIPowSignedDigit(10, (intType) -numStruct->scale);
        if (powerOfTen != NULL) {
          bigMultAssign(&bigIntValue, powerOfTen);
          bigDestr(powerOfTen);
        } /* if */
      } /* if */
      if (bigIntValue != NULL && numStruct->sign != 0) {
        bigIntValue = bigNegateTemp(bigIntValue);
      } /* if */
      if (bigIntValue != NULL) {
#if 0
        printf("numStruct->val: ");
        prot_bigint(bigIntValue);
        printf("\n");
#endif
#if INTTYPE_SIZE == 32
        intValue = bigToInt32(bigIntValue, NULL);
#elif INTTYPE_SIZE == 64
        intValue = bigToInt64(bigIntValue, NULL);
#endif
        bigDestr(bigIntValue);
      } /* if */
    } /* if */
    logFunction(printf("getNumericInt --> " FMT_D "\n", intValue););
    return intValue;
  } /* getNumericInt */



static bigIntType getNumericBigInt (const void *buffer)

  {
    const_numericType numStruct;
    memSizeType numBytes;
    bigIntType powerOfTen;
    bigIntType bigIntValue;

  /* getNumericBigInt */
    numStruct = (const_numericType) buffer;
    logFunction(printf("getNumericBigInt\n");
                printf("numStruct->precision: %u\n", numStruct->precision);
                printf("numStruct->scale: %d\n", numStruct->scale);
                printf("numStruct->sign: %u\n", numStruct->sign);
                printf("numStruct->val:");
                {
                  int pos;
                  for (pos = 0; pos < SQL_MAX_NUMERIC_LEN; pos++) {
                    printf(" %d", numStruct->val[pos]);
                  }
                  printf("\n");
                });
    if (unlikely(numStruct->scale > 0 ||
                 numStruct->precision == 0 || numStruct->precision > 100)) {
      raise_error(RANGE_ERROR);
      bigIntValue = NULL;
    } else {
      numBytes = (memSizeType) precisionToBytes[numStruct->precision];
      bigIntValue = bigFromByteBufferBe(numBytes, numStruct->val, FALSE);
#if 0
      if (bigIntValue != NULL) {
        printf("numStruct->val: ");
        prot_bigint(bigIntValue);
        printf("\n");
      } /* if */
#endif
      if (bigIntValue != NULL && numStruct->scale < 0) {
        powerOfTen = bigIPowSignedDigit(10, (intType) -numStruct->scale);
        if (powerOfTen != NULL) {
          bigMultAssign(&bigIntValue, powerOfTen);
          bigDestr(powerOfTen);
        } /* if */
      } /* if */
      if (bigIntValue != NULL && numStruct->sign != 0) {
        bigIntValue = bigNegateTemp(bigIntValue);
      } /* if */
    } /* if */
    logFunction(printf("getNumericBigInt --> %s\n",
                       bigHexCStri(bigIntValue)););
    return bigIntValue;
 } /* getNumericBigInt */



static bigIntType getNumericBigRational (const void *buffer, bigIntType *denominator)

  {
    const_numericType numStruct;
    memSizeType numBytes;
    bigIntType powerOfTen;
    bigIntType numerator;

  /* getNumericBigRational */
    numStruct = (const_numericType) buffer;
    logFunction(printf("getNumericBigRational\n");
                printf("numStruct->precision: %u\n", numStruct->precision);
                printf("numStruct->scale: %d\n", numStruct->scale);
                printf("numStruct->sign: %u\n", numStruct->sign);
                printf("numStruct->val:");
                {
                  int pos;
                  for (pos = 0; pos < SQL_MAX_NUMERIC_LEN; pos++) {
                    printf(" %d", numStruct->val[pos]);
                  }
                  printf("\n");
                });
    if (unlikely(numStruct->precision == 0 || numStruct->precision > 100)) {
      raise_error(RANGE_ERROR);
      numerator = NULL;
    } else {
      numBytes = (memSizeType) precisionToBytes[numStruct->precision];
      numerator = bigFromByteBufferBe(numBytes, numStruct->val, FALSE);
#if 0
      if (numerator != NULL) {
        printf("numStruct->val: ");
        prot_bigint(numerator);
        printf("\n");
      } /* if */
#endif
      if (numerator != NULL && numStruct->sign != 0) {
        numerator = bigNegateTemp(numerator);
      } /* if */
      if (numerator != NULL) {
        if (numStruct->scale < 0) {
          powerOfTen = bigIPowSignedDigit(10, (intType) -numStruct->scale);
          if (powerOfTen != NULL) {
            bigMultAssign(&numerator, powerOfTen);
            bigDestr(powerOfTen);
          } /* if */
          *denominator = bigFromInt32(1);
        } else {
          *denominator = bigIPowSignedDigit(10, (intType) numStruct->scale);
        } /* if */
      } /* if */
    } /* if */
    return numerator;
  } /* getNumericBigRational */



static floatType getNumericFloat (const void *buffer)

  {
    const_numericType numStruct;
    cstriType decimal;
    floatType floatValue;

  /* getNumericFloat */
    numStruct = (const_numericType) buffer;
    logFunction(printf("getNumericFloat\n");
                printf("numStruct->precision: %u\n", numStruct->precision);
                printf("numStruct->scale: %d\n", numStruct->scale);
                printf("numStruct->sign: %u\n", numStruct->sign);
                printf("numStruct->val:");
                {
                  int pos;
                  for (pos = 0; pos < SQL_MAX_NUMERIC_LEN; pos++) {
                    printf(" %d", numStruct->val[pos]);
                  }
                  printf("\n");
                });
    if (unlikely((decimal = getNumericAsCStri(numStruct)) == NULL)) {
      floatValue = 0.0;
    } else {
      floatValue = (floatType) strtod(decimal, NULL);
      UNALLOC_CSTRI(decimal, strlen(decimal));
    } /* if */
    logFunction(printf("getNumericFloat --> " FMT_E "\n", floatValue););
    return floatValue;
  } /* getNumericFloat */



static striType genSqlStringLiteral (const const_striType stri)

  {
    /* A string literal starts and ends with double quotes ("): */
    const memSizeType numOfQuotes = 2;
    register strElemType character;
    register memSizeType position;
    memSizeType striSize;
    memSizeType pos;
    striType resized_literal;
    striType literal;

  /* genSqlStringLiteral */
    striSize = stri->size;
    if (unlikely(striSize > (MAX_STRI_LEN - numOfQuotes) / QUOTE_IN_STRING_LEN ||
                 !ALLOC_STRI_SIZE_OK(literal, QUOTE_IN_STRING_LEN * striSize + numOfQuotes))) {
      raise_error(MEMORY_ERROR);
      literal = NULL;
    } else {
      literal->mem[0] = (strElemType) '\'';
      pos = 1;
      for (position = 0; position < striSize; position++) {
        character = (strElemType) stri->mem[position];
        if (character == '\'') {
          literal->mem[pos]     = (strElemType) '\'';
          literal->mem[pos + 1] = (strElemType) '\'';
          pos += 2;
        } else {
          literal->mem[pos] = character;
          pos++;
        } /* if */
      } /* for */
      literal->mem[pos] = (strElemType) '\'';
      pos++;
      literal->size = pos;
      REALLOC_STRI_SIZE_SMALLER(resized_literal, literal,
          QUOTE_IN_STRING_LEN * striSize + numOfQuotes, pos);
      if (unlikely(resized_literal == NULL)) {
        FREE_STRI(literal, QUOTE_IN_STRING_LEN * striSize + numOfQuotes);
        raise_error(MEMORY_ERROR);
        literal = NULL;
      } else {
        literal = resized_literal;
        COUNT3_STRI(QUOTE_IN_STRING_LEN * striSize + numOfQuotes, pos);
      } /* if */
    } /* if */
    return literal;
  } /* genSqlStringLiteral */



static void sqlBindBigInt (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType value)

  {
    preparedStmtType preparedStmt;

  /* sqlBindBigInt */
    logFunction(printf("sqlBindBigInt(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, bigHexCStri(value)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBigInt: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      strDestr(preparedStmt->param_array[pos - 1].buffer);
      preparedStmt->param_array[pos - 1].buffer = bigStr(value);
    } /* if */
  } /* sqlBindBigInt */



static void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType numerator, const const_bigIntType denominator)

  {
    preparedStmtType preparedStmt;
    ustriType decimal;
    memSizeType length;
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
      if (bigEqSignedDigit(denominator, 1)) {
        decimal = bigIntToDecimal(numerator, &length, &err_info);
      } else {
        decimal = bigRatToDecimal(numerator, denominator, DEFAULT_DECIMAL_SCALE,
                                  &length, &err_info);
      } /* if */
      if (unlikely(decimal == NULL)) {
        raise_error(err_info);
      } else {
        strDestr(preparedStmt->param_array[pos - 1].buffer);
        preparedStmt->param_array[pos - 1].buffer =
            cstri_buf_to_stri((cstriType) decimal, length);
        free(decimal);
        if (unlikely(preparedStmt->param_array[pos - 1].buffer == NULL)) {
          raise_error(MEMORY_ERROR);
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBigRat */



static void sqlBindBool (sqlStmtType sqlStatement, intType pos, boolType value)

  {
    preparedStmtType preparedStmt;

  /* sqlBindBool */
    logFunction(printf("sqlBindBool(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, value ? "TRUE" : "FALSE"););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBool: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      strDestr(preparedStmt->param_array[pos - 1].buffer);
      preparedStmt->param_array[pos - 1].buffer = intStr(value);
    } /* if */
  } /* sqlBindBool */



static void sqlBindBStri (sqlStmtType sqlStatement, intType pos,
    const const_bstriType bstri)

  {
    const unsigned char digit[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                   '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    preparedStmtType preparedStmt;
    striType stri;
    memSizeType idx;

  /* sqlBindBStri */
    logFunction(printf("sqlBindBStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, bstriAsUnquotedCStri(bstri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBStri: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      strDestr(preparedStmt->param_array[pos - 1].buffer);
      if (unlikely(bstri->size > (MAX_STRI_LEN - 2) / 2 ||
                   !ALLOC_STRI_CHECK_SIZE(stri, 2 + 2 * bstri->size))) {
        raise_error(MEMORY_ERROR);
      } else {
        stri->size = 2 + 2 * bstri->size;
        stri->mem[0] = '0';
        stri->mem[1] = 'x';
        for (idx = 0; idx < bstri->size; idx++) {
          stri->mem[(idx << 1) + 2] = digit[bstri->mem[idx] >> 4];
          stri->mem[(idx << 1) + 3] = digit[bstri->mem[idx] & 0xf];
        } /* for */
        preparedStmt->param_array[pos - 1].buffer = stri;
      } /* if */
    } /* if */
  } /* sqlBindBStri */



static void sqlBindDuration (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  {
    preparedStmtType preparedStmt;

  /* sqlBindDuration */
    logFunction(printf("sqlBindDuration(" FMT_U_MEM ", " FMT_D ", P"
                                          FMT_D "Y" FMT_D "M" FMT_D "DT"
                                          FMT_D "H" FMT_D "M%s%lu.%06luS)\n",
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
    } /* if */
  } /* sqlBindDuration */



static void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value)

  {
    preparedStmtType preparedStmt;

  /* sqlBindFloat */
    logFunction(printf("sqlBindFloat(" FMT_U_MEM ", " FMT_D ", " FMT_E ")\n",
                       (memSizeType) sqlStatement, pos, value););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindFloat: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      strDestr(preparedStmt->param_array[pos - 1].buffer);
      preparedStmt->param_array[pos - 1].buffer = fltStr(value);
    } /* if */
  } /* sqlBindFloat */



static void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value)

  {
    preparedStmtType preparedStmt;

  /* sqlBindInt */
    logFunction(printf("sqlBindInt(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos, value););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindInt: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      strDestr(preparedStmt->param_array[pos - 1].buffer);
      preparedStmt->param_array[pos - 1].buffer = intStr(value);
    } /* if */
  } /* sqlBindInt */



static void sqlBindNull (sqlStmtType sqlStatement, intType pos)

  {
    preparedStmtType preparedStmt;

  /* sqlBindNull */
    logFunction(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindNull: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      strDestr(preparedStmt->param_array[pos - 1].buffer);
      preparedStmt->param_array[pos - 1].buffer = cstri_to_stri("NULL");
      if (unlikely(preparedStmt->param_array[pos - 1].buffer == NULL)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
  } /* sqlBindNull */



static void sqlBindStri (sqlStmtType sqlStatement, intType pos,
    const const_striType stri)

  {
    preparedStmtType preparedStmt;

  /* sqlBindStri */
    logFunction(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, striAsUnquotedCStri(stri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindStri: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      strDestr(preparedStmt->param_array[pos - 1].buffer);
      preparedStmt->param_array[pos - 1].buffer = genSqlStringLiteral(stri);
    } /* if */
  } /* sqlBindStri */



static void sqlBindTime (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second,
    intType time_zone)

  {
    preparedStmtType preparedStmt;
    char timeLiteral[MAX_TIME_LITERAL_LENGTH + NULL_TERMINATION_LEN];
    memSizeType idx;

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
    } else if (unlikely(year < -9999 || year > 9999 || month < 1 || month > 12 ||
                        day < 1 || day > 31 || hour < 0 || hour >= 24 ||
                        minute < 0 || minute >= 60 || second < 0 || second >= 60 ||
                        micro_second < 0 || micro_second >= 1000000)) {
      logError(printf("sqlBindTime: Time not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      if ((hour == 0 && minute == 0 && second == 0 && micro_second == 0) &&
          (year != 0 || month != 1 || day != 1)) {
        sprintf(timeLiteral, "'" F_D(04) "-" F_D(02) "-" F_D(02) "'",
                year, month, day);
      } else {
        if (year == 0 && month == 1 && day == 1) {
          sprintf(timeLiteral, "'" F_D(02) ":" F_D(02) ":" F_D(02),
                  hour, minute, second);
        } else {
          sprintf(timeLiteral, "'" F_D(04) "-" F_D(02) "-" F_D(02)
                               " " F_D(02) ":" F_D(02) ":" F_D(02),
                  year, month, day, hour, minute, second);
        } /* if */
        if (micro_second == 0) {
          strcat(timeLiteral, "'");
        } else {
          sprintf(&timeLiteral[strlen(timeLiteral)], "." F_U(06),
                  micro_second);
          idx = strlen(timeLiteral);
          do {
            idx--;
          } while (timeLiteral[idx] == '0');
          timeLiteral[idx + 1] = '\'';
          timeLiteral[idx + 2] = '\0';
        } /* if */
      } /* if */
      strDestr(preparedStmt->param_array[pos - 1].buffer);
      preparedStmt->param_array[pos - 1].buffer = cstri_to_stri(timeLiteral);
      if (unlikely(preparedStmt->param_array[pos - 1].buffer == NULL)) {
        raise_error(MEMORY_ERROR);
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
    if (db->dbproc != NULL) {
      dbclose(db->dbproc);
      db->dbproc = NULL;
    } /* if */
    logFunction(printf("sqlClose -->\n"););
  } /* sqlClose */



static bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    resultDataType columnData;
    BYTE *data;
    DBINT length;
    bigIntType columnValue;

  /* sqlColumnBigInt */
    logFunction(printf("sqlColumnBigInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnBigInt: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      columnData = &preparedStmt->result_array[column - 1];
      data = dbdata(preparedStmt->dbproc, (int) column);
      if (data == NULL) {
        length = dbdatlen(preparedStmt->dbproc, (int) column);
        if (likely(length == 0)) {
          logMessage(printf("Column is NULL -> Use default value: 0\n"););
          columnValue = bigZero();
        } else {
          dbInconsistent("sqlColumnBigInt", "dbdatlen");
          logError(printf("sqlColumnBigInt: Column " FMT_D ": "
                          "dbdatlen returns %d.\n", column, length););
          raise_error(DATABASE_ERROR);
          columnValue = NULL;
        } /* if */
      } else {
        /* printf("buffer: %s\n", (unsigned char *) columnData->buffer); */
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SYBINT1:
#if SYBINT1_IS_SIGNED
            columnValue = bigFromInt32((int32Type) *(int8Type *) data);
#else
            columnValue = bigFromInt32((int32Type) *(uint8Type *) data);
#endif
            break;
          case SYBINT2:
            columnValue = bigFromInt32((int32Type) *(int16Type *) data);
            break;
          case SYBINT4:
            columnValue = bigFromInt32(*(int32Type *) data);
            break;
          case SYBINT8:
            columnValue = bigFromInt64(*(int64Type *) data);
            break;
          case SYBDECIMAL:
          case SYBNUMERIC:
            columnValue = getNumericBigInt(data);
            break;
          default:
            logError(printf("sqlColumnBigInt: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
            raise_error(RANGE_ERROR);
            columnValue = NULL;
            break;
        } /* switch */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnBigInt --> %s\n", bigHexCStri(columnValue)););
    return columnValue;
  } /* sqlColumnBigInt */



static void sqlColumnBigRat (sqlStmtType sqlStatement, intType column,
    bigIntType *numerator, bigIntType *denominator)

  {
    preparedStmtType preparedStmt;
    resultDataType columnData;
    BYTE *data;
    DBINT length;

  /* sqlColumnBigRat */
    logFunction(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", *, *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnBigRat: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
    } else {
      columnData = &preparedStmt->result_array[column - 1];
      data = dbdata(preparedStmt->dbproc, (int) column);
      if (data == NULL) {
        length = dbdatlen(preparedStmt->dbproc, (int) column);
        if (likely(length == 0)) {
          logMessage(printf("Column is NULL -> Use default value: 0\n"););
          *numerator = bigZero();
          *denominator = bigFromInt32(1);
        } else {
          dbInconsistent("sqlColumnBigRat", "dbdatlen");
          logError(printf("sqlColumnBigRat: Column " FMT_D ": "
                          "dbdatlen returns %d.\n", column, length););
          raise_error(DATABASE_ERROR);
        } /* if */
      } else {
        /* printf("buffer: %s\n", (unsigned char *) columnData->buffer); */
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SYBINT1:
#if SYBINT1_IS_SIGNED
            *numerator = bigFromInt32((int32Type) *(int8Type *) data);
#else
            *numerator = bigFromInt32((int32Type) *(uint8Type *) data);
#endif
            *denominator = bigFromInt32(1);
            break;
          case SYBINT2:
            *numerator = bigFromInt32((int32Type) *(int16Type *) data);
            *denominator = bigFromInt32(1);
            break;
          case SYBINT4:
            *numerator = bigFromInt32(*(int32Type *) data);
            *denominator = bigFromInt32(1);
            break;
          case SYBINT8:
            *numerator = bigFromInt64(*(int64Type *) data);
            *denominator = bigFromInt32(1);
            break;
          case SYBDECIMAL:
          case SYBNUMERIC:
            *numerator = getNumericBigRational(data, denominator);
            break;
          default:
            logError(printf("sqlColumnBigRat: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
            raise_error(RANGE_ERROR);
            break;
        } /* switch */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", %s, %s) -->\n",
                       (memSizeType) sqlStatement, column,
                       bigHexCStri(*numerator), bigHexCStri(*denominator)););
  } /* sqlColumnBigRat */



static boolType sqlColumnBool (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    resultDataType columnData;
    BYTE *data;
    DBINT length;
    intType columnValue;

  /* sqlColumnBool */
    logFunction(printf("sqlColumnBool(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnBool: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      columnData = &preparedStmt->result_array[column - 1];
      data = dbdata(preparedStmt->dbproc, (int) column);
      if (data == NULL) {
        length = dbdatlen(preparedStmt->dbproc, (int) column);
        if (likely(length == 0)) {
          logMessage(printf("Column is NULL -> Use default value: 0\n"););
          columnValue = 0;
        } else {
          dbInconsistent("sqlColumnBool", "dbdatlen");
          logError(printf("sqlColumnBool: Column " FMT_D ": "
                          "dbdatlen returns %d.\n", column, length););
          raise_error(DATABASE_ERROR);
          columnValue = 0;
        } /* if */
      } else {
        /* printf("buffer: %s\n", (unsigned char *) columnData->buffer); */
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SYBINT1:
#if SYBINT1_IS_SIGNED
            columnValue = *(int8Type *) data;
#else
            columnValue = *(uint8Type *) data;
#endif
            break;
          case SYBINT2:
            columnValue = *(int16Type *) data;
            break;
          case SYBINT4:
            columnValue = *(int32Type *) data;
            break;
          case SYBINT8:
            columnValue = *(int64Type *) data;
            break;
          case SYBDECIMAL:
          case SYBNUMERIC:
            columnValue = getNumericInt(data);
            break;
          case SYBCHAR:
          case SYBVARCHAR:
            length = dbdatlen(preparedStmt->dbproc, (int) column);
            /* printf("length: " FMT_D32 "\n", length); */
            if (length == -1) {
              columnValue = 0;
            } else if (unlikely(length != 1)) {
              logError(printf("sqlColumnBool: Column " FMT_D ": "
                              "The size of a boolean field must be 1.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = 0;
            } else {
              columnValue = *(const_cstriType) data - '0';
            } /* if */
            break;
          default:
            logError(printf("sqlColumnBool: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
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
    } /* if */
    logFunction(printf("sqlColumnBool --> %s\n", columnValue ? "TRUE" : "FALSE"););
    return columnValue != 0;
  } /* sqlColumnBool */



static bstriType sqlColumnBStri (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    resultDataType columnData;
    DBINT length;
    bstriType columnValue;

  /* sqlColumnBStri */
    logFunction(printf("sqlColumnBStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnBStri: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      columnData = &preparedStmt->result_array[column - 1];
      /* printf("buffer: %s\n", (unsigned char *) columnData->buffer); */
      logMessage(printf("buffer_type: %s\n",
                        nameOfBufferType(columnData->buffer_type)););
      switch (columnData->buffer_type) {
        case SYBBINARY:
        case SYBVARBINARY:
          length = dbdatlen(preparedStmt->dbproc, (int) column);
          /* printf("length: " FMT_D32 "\n", length); */
          if (unlikely(length < 0)) {
            dbInconsistent("sqlColumnBStri", "dbdatlen");
            logError(printf("sqlColumnBStri: Column " FMT_D ": "
                            "dbdatlen returns %d.\n", column, length););
            raise_error(DATABASE_ERROR);
            columnValue = NULL;
          } else if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(columnValue, (memSizeType) length))) {
            raise_error(MEMORY_ERROR);
          } else {
            columnValue->size = (memSizeType) length;
            memcpy(columnValue->mem,
                   (ustriType) dbdata(preparedStmt->dbproc, (int) column),
                   (memSizeType) length);
          } /* if */
          break;
        default:
          logError(printf("sqlColumnBStri: Column " FMT_D " has the unknown type %s.\n",
                          column, nameOfBufferType(columnData->buffer_type)););
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
    resultDataType columnData;
    BYTE *data;
    DBINT length;

  /* sqlColumnDuration */
    logFunction(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnDuration: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
    } else {
      columnData = &preparedStmt->result_array[column - 1];
      data = dbdata(preparedStmt->dbproc, (int) column);
      if (data == NULL) {
        length = dbdatlen(preparedStmt->dbproc, (int) column);
        /* printf("length: " FMT_D32 "\n", length); */
        if (likely(length == 0)) {
          logMessage(printf("Column is NULL -> Use default value: P0D\n"););
          *year         = 0;
          *month        = 0;
          *day          = 0;
          *hour         = 0;
          *minute       = 0;
          *second       = 0;
          *micro_second = 0;
        } else {
          dbInconsistent("sqlColumnDuration", "dbdatlen");
          logError(printf("sqlColumnDuration: Column " FMT_D ": "
                          "dbdatlen returns %d.\n", column, length););
          raise_error(DATABASE_ERROR);
        } /* if */
      } else {
        /* printf("buffer: %s\n", (unsigned char *) columnData->buffer); */
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          default:
            logError(printf("sqlColumnDuration: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
            raise_error(RANGE_ERROR);
            break;
        } /* switch */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ") --> P"
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
    resultDataType columnData;
    BYTE *data;
    DBINT length;
    floatType columnValue;

  /* sqlColumnFloat */
    logFunction(printf("sqlColumnFloat(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnFloat: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnValue = 0.0;
    } else {
      columnData = &preparedStmt->result_array[column - 1];
      data = dbdata(preparedStmt->dbproc, (int) column);
      if (data == NULL) {
        length = dbdatlen(preparedStmt->dbproc, (int) column);
        if (likely(length == 0)) {
          logMessage(printf("Column is NULL -> Use default value: 0.0\n"););
          columnValue = 0.0;
        } else {
          dbInconsistent("sqlColumnFloat", "dbdatlen");
          logError(printf("sqlColumnFloat: Column " FMT_D ": "
                          "dbdatlen returns %d.\n", column, length););
          raise_error(DATABASE_ERROR);
          columnValue = 0.0;
        } /* if */
      } else {
        /* printf("buffer: %s\n", (unsigned char *) columnData->buffer); */
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SYBINT1:
#if SYBINT1_IS_SIGNED
            columnValue = (floatType) *(int8Type *) data;
#else
            columnValue = (floatType) *(uint8Type *) data;
#endif
            break;
          case SYBINT2:
            columnValue = (floatType) *(int16Type *) data;
            break;
          case SYBINT4:
            columnValue = (floatType) *(int32Type *) data;
            break;
          case SYBINT8:
            columnValue = (floatType) *(int64Type *) data;
            break;
          case SYBREAL:
            columnValue = *(float *) data;
            break;
          case SYBFLT8:
            columnValue = *(double *) data;
            break;
          case SYBDECIMAL:
          case SYBNUMERIC:
            columnValue = getNumericFloat(data);
            break;
          default:
            logError(printf("sqlColumnFloat: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
            raise_error(RANGE_ERROR);
            columnValue = 0.0;
            break;
        } /* switch */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnFloat --> " FMT_E "\n", columnValue););
    return columnValue;
  } /* sqlColumnFloat */



static intType sqlColumnInt (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    resultDataType columnData;
    BYTE *data;
    DBINT length;
    intType columnValue;

  /* sqlColumnInt */
    logFunction(printf("sqlColumnInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnInt: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      columnData = &preparedStmt->result_array[column - 1];
      data = dbdata(preparedStmt->dbproc, (int) column);
      if (data == NULL) {
        length = dbdatlen(preparedStmt->dbproc, (int) column);
        if (likely(length == 0)) {
          logMessage(printf("Column is NULL -> Use default value: 0\n"););
          columnValue = 0;
        } else {
          dbInconsistent("sqlColumnInt", "dbdatlen");
          logError(printf("sqlColumnInt: Column " FMT_D ": "
                          "dbdatlen returns %d.\n", column, length););
          raise_error(DATABASE_ERROR);
          columnValue = 0;
        } /* if */
      } else {
        /* printf("buffer: %s\n", (unsigned char *) columnData->buffer); */
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SYBINT1:
#if SYBINT1_IS_SIGNED
            columnValue = *(int8Type *) data;
#else
            columnValue = *(uint8Type *) data;
#endif
            break;
          case SYBINT2:
            columnValue = *(int16Type *) data;
            break;
          case SYBINT4:
            columnValue = *(int32Type *) data;
            break;
          case SYBINT8:
            columnValue = *(int64Type *) data;
            break;
          case SYBDECIMAL:
          case SYBNUMERIC:
            columnValue = getNumericInt(data);
            break;
          default:
            logError(printf("sqlColumnInt: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
            raise_error(RANGE_ERROR);
            columnValue = 0;
            break;
        } /* switch */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnInt --> " FMT_D "\n", columnValue););
    return columnValue;
  } /* sqlColumnInt */



static striType sqlColumnStri (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    resultDataType columnData;
    BYTE *data;
    DBINT length;
    errInfoType err_info = OKAY_NO_ERROR;
    striType columnValue;

  /* sqlColumnStri */
    logFunction(printf("sqlColumnStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnStri: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      columnData = &preparedStmt->result_array[column - 1];
      data = dbdata(preparedStmt->dbproc, (int) column);
      /* printf("data: " FMT_U_MEM "\n", (memSizeType) data); */
      length = dbdatlen(preparedStmt->dbproc, (int) column);
      /* printf("length: " FMT_D32 "\n", length); */
      if (unlikely(length < 0)) {
        dbInconsistent("sqlColumnStri", "dbdatlen");
        logError(printf("sqlColumnStri: Column " FMT_D ": "
                        "dbdatlen returns %d.\n", column, length););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else if (data == NULL) {
        if (likely(length == 0)) {
          logMessage(printf("Column is NULL -> Use default value: \"\"\n"););
          columnValue = strEmpty();
        } else {
          dbInconsistent("sqlColumnStri", "dbdatlen");
          logError(printf("sqlColumnStri: Column " FMT_D ": "
                          "dbdatlen returns %d.\n", column, length););
          raise_error(DATABASE_ERROR);
          columnValue = NULL;
        } /* if */
      } else {
        /* printf("buffer: %s\n", (unsigned char *) columnData->buffer); */
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SYBCHAR:
            while (length > 0 && data[length - 1] == ' ') {
              length--;
            } /* if */
            columnValue = cstri8_buf_to_stri((const_cstriType) data,
                (memSizeType) length, &err_info);
            if (unlikely(columnValue == NULL)) {
              raise_error(err_info);
            } /* if */
            break;
          case SYBVARCHAR:
            columnValue = cstri8_buf_to_stri((const_cstriType) data,
                (memSizeType) length, &err_info);
            if (unlikely(columnValue == NULL)) {
              raise_error(err_info);
            } /* if */
            break;
          case SYBBINARY:
          case SYBVARBINARY:
            if (unlikely(!ALLOC_STRI_CHECK_SIZE(columnValue, (memSizeType) length))) {
              raise_error(MEMORY_ERROR);
            } else {
              memcpy_to_strelem(columnValue->mem, (ustriType) data,
                  (memSizeType) length);
              columnValue->size = (memSizeType) length;
            } /* if */
            break;
          default:
            logError(printf("sqlColumnStri: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
            raise_error(RANGE_ERROR);
            columnValue = NULL;
            break;
        } /* switch */
      } /* if */
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
    resultDataType columnData;
    BYTE *data;
    DBINT length;
    DBDATETIMEALL *dateTimeAll;
    uint64Type numTimeUnits;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlColumnTime */
    logFunction(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnTime: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
    } else {
      columnData = &preparedStmt->result_array[column - 1];
      data = dbdata(preparedStmt->dbproc, (int) column);
      if (data == NULL) {
        length = dbdatlen(preparedStmt->dbproc, (int) column);
        /* printf("length: " FMT_D32 "\n", length); */
        if (likely(length == 0)) {
          logMessage(printf("Column is NULL -> Use default value: 0-01-01 00:00:00\n"););
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
          dbInconsistent("sqlColumnTime", "dbdatlen");
          logError(printf("sqlColumnTime: Column " FMT_D ": "
                          "dbdatlen returns %d.\n", column, length););
          raise_error(DATABASE_ERROR);
        } /* if */
      } else {
        /* printf("buffer: %s\n", (unsigned char *) columnData->buffer); */
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SYBMSDATE:
          case SYBMSTIME:
          case SYBMSDATETIME2:
            length = dbdatlen(preparedStmt->dbproc, (int) column);
            /* printf("length: " FMT_D32 "\n", length); */
            if (unlikely(length < 0)) {
              dbInconsistent("sqlColumnTime", "dbdatlen");
              logError(printf("sqlColumnTime: Column " FMT_D ": "
                              "dbdatlen returns %d.\n", column, length););
              raise_error(DATABASE_ERROR);
            } else if (unlikely(length != sizeof(DBDATETIMEALL))) {
              logError(printf("sqlColumnTime: In column " FMT_D
                              " the DBDATETIMEALL length of " FMT_D32 " is wrong.\n",
                              column, length););
              err_info = RANGE_ERROR;
            } else {
              dateTimeAll = (DBDATETIMEALL *) data;
              if (columnData->buffer_type == SYBMSTIME) {
                *year = 0;
                *month = 1;
                *day = 1;
              } else {
                dateFromDaysSince1900(dateTimeAll->date, year, month, day);
              } /* if */
              *micro_second = (intType) ((dateTimeAll->time % 10000000) / 10);
              numTimeUnits = dateTimeAll->time / 10000000;
              *second = (intType) (numTimeUnits % 60);
              numTimeUnits /= 60;
              *minute = (intType) (numTimeUnits % 60);
              *hour = (intType) (numTimeUnits / 60);
            } /* if */
            if (unlikely(err_info != OKAY_NO_ERROR)) {
              raise_error(err_info);
            } /* if */
            break;
          default:
            logError(printf("sqlColumnTime: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
            raise_error(RANGE_ERROR);
            break;
        } /* switch */
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
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlCommit */
    logFunction(printf("sqlCommit(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (unlikely(db->dbproc == NULL)) {
      logError(printf("sqlCommit: Database is not open.\n"););
      raise_error(RANGE_ERROR);
    } else if (!db->autoCommit) {
      doExecute(db->dbproc, "COMMIT", &err_info);
      doExecute(db->dbproc, "BEGIN TRANSACTION", &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunction(printf("sqlCommit -->\n"););
  } /* sqlCommit */



static void sqlExecute (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    striType boundStatement;
    cstriType query;
    RETCODE retCode;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlExecute */
    logFunction(printf("sqlExecute(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    boundStatement = getBoundStatement(preparedStmt, &err_info);
    if (unlikely(boundStatement == NULL)) {
      if (err_info == RANGE_ERROR) {
        dbLibError("sqlExecute", "SQLExecute",
                   "Unbound statement parameter(s).\n");
        raise_error(DATABASE_ERROR);
      } else {
        raise_error(err_info);
      } /* if */
    } else {
      query = stri_to_cstri8(boundStatement, &err_info);
      if (likely(query != NULL)) {
        /* printf("ppStmt: " FMT_X_MEM "\n", (memSizeType) preparedStmt->ppStmt); */
        preparedStmt->fetchOkay = FALSE;
        if (unlikely(dbcmd(preparedStmt->dbproc, query) != SUCCEED)) {
          logError(printf("sqlExecute: dbcmd(" FMT_U_MEM ", \"%s\") failed.\n",
                          (memSizeType) preparedStmt->dbproc, query););
          preparedStmt->executeSuccessful = FALSE;
          err_info = DATABASE_ERROR;
        } else if (unlikely(dbsqlexec(preparedStmt->dbproc) != SUCCEED)) {
          logError(printf("sqlExecute: dbsqlexec(" FMT_U_MEM ") with \"%s\" failed.\n",
                          (memSizeType) preparedStmt->dbproc, query););
          preparedStmt->executeSuccessful = FALSE;
          err_info = DATABASE_ERROR;
        } else {
          retCode = dbresults(preparedStmt->dbproc);
          /* printf("dbresults returns: %d\n", retCode); */
          if (unlikely(retCode == FAIL)) {
            logError(printf("sqlExecute: dbresults() failed.\n"););
            preparedStmt->executeSuccessful = FALSE;
            err_info = DATABASE_ERROR;
          } else {
            preparedStmt->executeSuccessful = TRUE;
            preparedStmt->fetchFinished = FALSE;
            err_info = setupResult(preparedStmt);
          } /* if */
        } /* if */
        free_cstri8(query, boundStatement);
      } /* if */
      FREE_STRI(boundStatement, boundStatement->size);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunction(printf("sqlExecute -->\n"););
  } /* sqlExecute */



static boolType sqlFetch (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    RETCODE fetch_result;

  /* sqlFetch */
    logFunction(printf("sqlFetch(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->executeSuccessful)) {
      dbLibError("sqlFetch", "OCIStmtExecute",
                 "Execute was not successful.\n");
      logError(printf("sqlFetch: Execute was not successful.\n"););
      preparedStmt->fetchOkay = FALSE;
      raise_error(DATABASE_ERROR);
    } else {
      fetch_result = dbnextrow(preparedStmt->dbproc);
      /* printf("fetch_result: %d\n", fetch_result); */
      if (fetch_result == NO_MORE_ROWS) {
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->fetchFinished = TRUE;
      } else {
        preparedStmt->fetchOkay = TRUE;
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
    if (unlikely(db->dbproc == NULL)) {
      logError(printf("sqlGetAutoCommit: Database is not open.\n"););
      raise_error(RANGE_ERROR);
      autoCommit = FALSE;
    } else {
      autoCommit = db->autoCommit;
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
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlIsNull: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      isNull = FALSE;
    } else {
      isNull = dbdatlen(preparedStmt->dbproc, (int) column) == 0;
    } /* if */
    logFunction(printf("sqlIsNull --> %s\n", isNull ? "TRUE" : "FALSE"););
    return isNull;
  } /* sqlIsNull */



static sqlStmtType sqlPrepare (databaseType database,
    const const_striType sqlStatementStri)

  {
    dbType db;
    striType *stmtPartArray;
    memSizeType numBindParameters;
    errInfoType err_info = OKAY_NO_ERROR;
    preparedStmtType preparedStmt;

  /* sqlPrepare */
    logFunction(printf("sqlPrepare(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) database,
                       striAsUnquotedCStri(sqlStatementStri)););
    db = (dbType) database;
    if (unlikely(db->dbproc == NULL)) {
      logError(printf("sqlPrepare: Database is not open.\n"););
      err_info = RANGE_ERROR;
      preparedStmt = NULL;
    } else {
      stmtPartArray = processStatementStri(sqlStatementStri, &numBindParameters, &err_info);
      if (stmtPartArray == NULL) {
        preparedStmt = NULL;
      } else {
        if (!ALLOC_RECORD2(preparedStmt, preparedStmtRecordTds,
                           count.prepared_stmt, count.prepared_stmt_bytes)) {
          err_info = MEMORY_ERROR;
        } else {
          memset(preparedStmt, 0, sizeof(preparedStmtRecordTds));
          preparedStmt->usage_count = 1;
          preparedStmt->sqlFunc = db->sqlFunc;
          preparedStmt->dbproc = db->dbproc;
          preparedStmt->stmtPartArray = stmtPartArray;
          err_info = setStmtPartArrayCharCount(preparedStmt, numBindParameters);
          if (likely(err_info == OKAY_NO_ERROR)) {
            err_info = setupParameters(preparedStmt, numBindParameters);
          } /* if */
          if (unlikely(err_info != OKAY_NO_ERROR)) {
            freePreparedStmt((sqlStmtType) preparedStmt);
            preparedStmt = NULL;
          } /* if */
        } /* if */
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
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlRollback */
    logFunction(printf("sqlRollback(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (unlikely(db->dbproc == NULL)) {
      logError(printf("sqlRollback: Database is not open.\n"););
      raise_error(RANGE_ERROR);
    } else if (!db->autoCommit) {
      doExecute(db->dbproc, "ROLLBACK", &err_info);
      doExecute(db->dbproc, "BEGIN TRANSACTION", &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunction(printf("sqlRollback -->\n"););
  } /* sqlRollback */



static void sqlSetAutoCommit (databaseType database, boolType autoCommit)

  {
    dbType db;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlSetAutoCommit */
    logFunction(printf("sqlSetAutoCommit(" FMT_U_MEM ", %d)\n",
                       (memSizeType) database, autoCommit););
    db = (dbType) database;
    if (unlikely(db->dbproc == NULL)) {
      logError(printf("sqlSetAutoCommit: Database is not open.\n"););
      raise_error(RANGE_ERROR);
    } else {
      if (db->autoCommit != autoCommit) {
        if (autoCommit) {
          doExecute(db->dbproc, "COMMIT", &err_info);
        } else {
          doExecute(db->dbproc, "BEGIN TRANSACTION", &err_info);
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          db->autoCommit = autoCommit;
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
    if (unlikely(preparedStmt->result_array_size > INTTYPE_MAX)) {
      logError(printf("sqlStmtColumnCount: "
                      "result_array_size (=" FMT_U_MEM ") > INTTYPE_MAX\n",
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnCount = 0;
    } else {
      columnCount = (intType) preparedStmt->result_array_size;
    } /* if */
    logFunction(printf("sqlStmtColumnCount --> " FMT_D "\n", columnCount););
    return columnCount;
  } /* sqlStmtColumnCount */



static striType sqlStmtColumnName (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    char *columnName;
    errInfoType err_info = OKAY_NO_ERROR;
    striType name;

  /* sqlStmtColumnName */
    logFunction(printf("sqlStmtColumnName(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlStmtColumnName: column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      column, preparedStmt->result_array_size););
      err_info = RANGE_ERROR;
      name = NULL;
    } else {
      columnName = dbcolname(preparedStmt->dbproc, (int) column);
      if (unlikely(columnName == NULL)) {
        dbLibError("sqlStmtColumnName", "dbcolname", "dbcolname() returns NULL.\n");
        logError(printf("sqlStmtColumnName: dbcolname() returns NULL.\n"););
        err_info = DATABASE_ERROR;
        name = NULL;
      } else {
        name = cstri8_to_stri(columnName, &err_info);
      } /* if */
    } /* if */
    if (unlikely(name == NULL)) {
      raise_error(err_info);
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
    if (quoteQuote == NULL) {
      quoteQuote = CSTRI_LITERAL_TO_STRI("''");
    } /* if */
    return sqlFunc != NULL;
  } /* setupFuncTable */



databaseType sqlOpenTds (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  {
    const_cstriType host8;
    memSizeType host8Length;
    const_cstriType dbName8;
    const_cstriType user8;
    const_cstriType password8;
    char portName[1 + INTTYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    memSizeType portNameLength;
    cstriType hostAndPort = NULL;
    const_cstriType server;
    LOGINREC *login;
    DBPROCESS *dbproc;
    RETCODE erc;
    errInfoType err_info = OKAY_NO_ERROR;
    dbType database;

  /* sqlOpenTds */
    logFunction(printf("sqlOpenTds(\"%s\", ",
                       striAsUnquotedCStri(host));
                printf(FMT_D ", \"%s\", ",
                       port, striAsUnquotedCStri(dbName));
                printf("\"%s\", ", striAsUnquotedCStri(user));
                printf("\"%s\")\n", striAsUnquotedCStri(password)););
    if (!findDll()) {
      logError(printf("sqlOpenTds: findDll() failed\n"););
      err_info = DATABASE_ERROR;
      database = NULL;
    } else if (unlikely((host8 = stri_to_cstri8(host, &err_info)) == NULL)) {
      database = NULL;
    } else {
      if (port == 0) {
        server = host8;
      } else {
        host8Length = strlen(host8);
        portNameLength = (memSizeType) sprintf(portName, ":" FMT_D, port);
        if (unlikely(!ALLOC_CSTRI(hostAndPort, host8Length + portNameLength))) {
          err_info = MEMORY_ERROR;
          database = NULL;
        } else {
          memcpy(hostAndPort, host8, host8Length);
          memcpy(&hostAndPort[host8Length], portName,
                 portNameLength + NULL_TERMINATION_LEN);
          server = hostAndPort;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        dbName8 = stri_to_cstri8(dbName, &err_info);
        if (unlikely(dbName8 == NULL)) {
          database = NULL;
        } else {
          user8 = stri_to_cstri8(user, &err_info);
          if (unlikely(user8 == NULL)) {
            database = NULL;
          } else {
            password8 = stri_to_cstri8(password, &err_info);
            if (unlikely(password8 == NULL)) {
              database = NULL;
            } else {
              if (dbinit() == FAIL) {
                dbLibError("sqlOpenTds", "dbinit", "dbinit() failed.\n");
                logError(printf("sqlOpenTds: dbinit() failed.\n"););
                err_info = DATABASE_ERROR;
                database = NULL;
              } else {
                dberrhandle(err_handler);
                dbmsghandle(msg_handler);
                if ((login = dblogin()) == NULL) {
                  setDbErrorMsg("sqlOpenTds", "dblogin");
                  logError(printf("sqlOpenTds: dblogin() failed.\n%s\n",
                                  dbError.message););
                  err_info = MEMORY_ERROR;
                  database = NULL;
                } else {
                  if (server[0] != '\0') {
                    DBSETLHOST(login, server);
                  } /* if */
                  DBSETLUSER(login, user8);
                  DBSETLPWD(login, password8);
                  DBSETLCHARSET(login, "UTF-8");
                  if ((dbproc = dbopen(login, server)) == NULL) {
                    setDbErrorMsg("sqlOpenTds", "dbopen");
                    logError(printf("sqlOpenTds: dbopen(\"%s\"/\"***Pwd***\", \"%s\"):\n%s\n",
                                    user8, server, dbError.message););
                    err_info = DATABASE_ERROR;
                    dbexit();
                    database = NULL;
                  } else if ((erc = dbuse(dbproc, dbName8)) == FAIL) {
                    setDbErrorMsg("sqlOpenTds", "dbuse");
                    logError(printf("sqlOpenTds: dbuse(*, \"%s\"):\n%s\n",
                                    dbName8, dbError.message););
                    err_info = DATABASE_ERROR;
                    dbclose(dbproc);
                    dbexit();
                    database = NULL;
                  } else {
                    /* CHAR fields are padded with spaces up to the field  */
                    /* width and all spaces are retrieved. For VARCHAR     */
                    /* fields trailing spaces are stored and retrieved.    */
                    doExecute(dbproc, "SET ANSI_PADDING ON", &err_info);
                    /* Set the default nullability of new created columns. */
                    /* By default connections from DB-Library applications */
                    /* create non-nullable columns. This is changed such   */
                    /* that nullable columns are created by default.       */
                    doExecute(dbproc, "SET ANSI_NULL_DFLT_ON ON", &err_info);
                    /* Identifiers can be delimited by double quotation    */
                    /* marks ("), and literals must be delimited by single */
                    /* quotation marks (').                                */
                    doExecute(dbproc, "SET QUOTED_IDENTIFIER ON", &err_info);
                    /* Terminate a query if an overflow or divide-by-zero  */
                    /* error occurs during query execution.                */
                    doExecute(dbproc, "SET ARITHABORT ON", &err_info);
                    /* Specify the size of varchar(max), nvarchar(max) and */
                    /* varbinary(max) returned by a SELECT statement.      */
                    /* The default value would be 4 KB. Unlimited is -1.   */
                    doExecute(dbproc, "SET TEXTSIZE -1", &err_info);
                    if (unlikely(err_info != OKAY_NO_ERROR)) {
                      dbclose(dbproc);
                      dbexit();
                      database = NULL;
                    } else if (unlikely(!setupFuncTable() ||
                                        !ALLOC_RECORD2(database, dbRecordTds,
                                                       count.database,
                                                       count.database_bytes))) {
                      err_info = MEMORY_ERROR;
                      dbclose(dbproc);
                      dbexit();
                      database = NULL;
                    } else {
                      memset(database, 0, sizeof(dbRecordTds));
                      database->usage_count = 1;
                      database->sqlFunc = sqlFunc;
                      database->driver = DB_CATEGORY_TDS;
                      database->dbproc = dbproc;
                      database->autoCommit = TRUE;
                    } /* if */
                  } /* if */
                  dbloginfree(login);
                } /* if */
              } /* if */
              free_cstri8(password8, password);
            } /* if */
            free_cstri8(user8, user);
          } /* if */
          free_cstri8(dbName8, dbName);
        } /* if */
      } /* if */
      if (hostAndPort != NULL) {
        UNALLOC_CSTRI(hostAndPort, host8Length + portNameLength);
      } /* if */
      free_cstri8(host8, host);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("sqlOpenTds --> " FMT_U_MEM "\n",
                       (memSizeType) database););
    return (databaseType) database;
  } /* sqlOpenTds */

#else



databaseType sqlOpenTds (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  { /* sqlOpenTds */
    logError(printf("sqlOpenTds(\"%s\", ",
                    striAsUnquotedCStri(host));
             printf(FMT_D ", \"%s\", ",
                    port, striAsUnquotedCStri(dbName));
             printf("\"%s\", ", striAsUnquotedCStri(user));
             printf("\"%s\"): TDS driver not present.\n",
                    striAsUnquotedCStri(password)););
    raise_error(RANGE_ERROR);
    return NULL;
  } /* sqlOpenTds */

#endif
