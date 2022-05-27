/********************************************************************/
/*                                                                  */
/*  sql_my.c      Database access functions for MariaDB and MySQL.  */
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
/*  File: seed7/src/sql_my.c                                        */
/*  Changes: 2014, 2015, 2017 - 2020  Thomas Mertes                 */
/*  Content: Database access functions for MariaDB and MySQL.       */
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
#ifdef MYSQL_INCLUDE
#include MYSQL_INCLUDE
#endif

#include "common.h"
#include "data_rtl.h"
#include "striutl.h"
#include "heaputl.h"
#include "numutl.h"
#include "str_rtl.h"
#include "int_rtl.h"
#include "flt_rtl.h"
#include "tim_rtl.h"
#include "big_drv.h"
#include "rtl_err.h"
#include "dll_drv.h"
#include "sql_base.h"
#include "sql_drv.h"

#ifdef MYSQL_INCLUDE


typedef struct {
    uintType     usage_count;
    sqlFuncType  sqlFunc;
    intType      driver;
    MYSQL       *connection;
    boolType     autoCommit;
    boolType     backslashEscapes;
  } dbRecordMy, *dbType;

typedef struct {
    memSizeType   buffer_capacity;
    boolType      bound;
  } bindDataRecordMy, *bindDataType;

typedef struct {
    char         *name;
    boolType      binary;
  } resultDataRecordMy, *resultDataType;

typedef struct {
    uintType       usage_count;
    sqlFuncType    sqlFunc;
    MYSQL_STMT    *ppStmt;
    memSizeType    param_array_size;
    MYSQL_BIND    *param_array;
    bindDataType   param_data_array;
    memSizeType    result_array_size;
    MYSQL_BIND    *result_array;
    resultDataType result_data_array;
    boolType       executeSuccessful;
    boolType       fetchOkay;
    boolType       fetchFinished;
  } preparedStmtRecordMy, *preparedStmtType;

static sqlFuncType sqlFunc = NULL;

#define DEFAULT_PORT 3306
#define MAX_DECIMAL_PRECISION 65
#define MAX_DECIMAL_SCALE 30
/* The maximum decimal length additionally needs place for a      */
/* decimal point, a sign and a possible leading or trailing zero. */
#define MAX_DECIMAL_LENGTH (MAX_DECIMAL_PRECISION + 3)
#define MOST_POSITIVE_FLOAT   3.4028234663852885e+38
#define MOST_NEGATIVE_FLOAT  -3.4028234663852885e+38
#define MOST_POSITIVE_DOUBLE  1.7976931348623157e+308
#define MOST_NEGATIVE_DOUBLE -1.7976931348623157e+308
#define MAX_TINY_BLOB_LENGTH                        255  /* 2 **  8 - 1 chars */
#define MAX_BLOB_LENGTH                           65535  /* 2 ** 16 - 1 chars */
#define MAX_MEDIUM_BLOB_LENGTH                 16777215  /* 2 ** 24 - 1 chars */
#define MAX_LONG_BLOB_LENGTH   UINT32_SUFFIX(4294967295) /* 2 ** 32 - 1 chars */


#ifdef MYSQL_DLL

#ifndef STDCALL
#if defined(_WIN32) && HAS_STDCALL
#define STDCALL __stdcall
#else
#define STDCALL
#endif
#endif

typedef my_bool (STDCALL *tp_mysql_autocommit) (MYSQL *mysql, my_bool mode);
typedef void (STDCALL *tp_mysql_close) (MYSQL *sock);
typedef my_bool (STDCALL *tp_mysql_commit) (MYSQL *mysql);
typedef unsigned int (STDCALL *tp_mysql_errno) (MYSQL *mysql);
typedef const char *(STDCALL *tp_mysql_error) (MYSQL *mysql);
typedef MYSQL_FIELD *(STDCALL *tp_mysql_fetch_field_direct) (MYSQL_RES *res,
                                                             unsigned int fieldnr);
typedef void (STDCALL *tp_mysql_free_result) (MYSQL_RES *result);
typedef MYSQL *(STDCALL *tp_mysql_init) (MYSQL *mysql);
typedef unsigned int (STDCALL *tp_mysql_num_fields) (MYSQL_RES *res);
typedef int (STDCALL *tp_mysql_options) (MYSQL *mysql,
                                         enum mysql_option option,
                                         const void *arg);
typedef MYSQL *(STDCALL *tp_mysql_real_connect) (MYSQL *mysql,
                                                 const char *host,
                                                 const char *user,
                                                 const char *passwd,
                                                 const char *db,
                                                 unsigned int port,
                                                 const char *unix_socket,
                                                 unsigned long clientflag);
typedef my_bool (STDCALL *tp_mysql_rollback) (MYSQL *mysql);
typedef int (STDCALL *tp_mysql_set_character_set) (MYSQL *mysql,
                                                   const char *csname);
typedef my_bool (STDCALL *tp_mysql_stmt_bind_param) (MYSQL_STMT *stmt,
                                                     MYSQL_BIND *bnd);
typedef my_bool (STDCALL *tp_mysql_stmt_bind_result) (MYSQL_STMT *stmt,
                                                      MYSQL_BIND *bnd);
typedef my_bool (STDCALL *tp_mysql_stmt_close) (MYSQL_STMT *stmt);
typedef unsigned int (STDCALL *tp_mysql_stmt_errno) (MYSQL_STMT *stmt);
typedef const char *(STDCALL *tp_mysql_stmt_error) (MYSQL_STMT *stmt);
typedef int (STDCALL *tp_mysql_stmt_execute) (MYSQL_STMT *stmt);
typedef int (STDCALL *tp_mysql_stmt_fetch) (MYSQL_STMT *stmt);
typedef int (STDCALL *tp_mysql_stmt_fetch_column) (MYSQL_STMT *stmt,
                                                   MYSQL_BIND *bnd,
                                                   unsigned int column,
                                                   unsigned long offset);
typedef MYSQL_STMT *(STDCALL *tp_mysql_stmt_init) (MYSQL *mysql);
typedef unsigned long (STDCALL *tp_mysql_stmt_param_count) (MYSQL_STMT *stmt);
typedef int (STDCALL *tp_mysql_stmt_prepare) (MYSQL_STMT *stmt,
                                              const char *query,
                                              unsigned long length);
typedef MYSQL_RES *(STDCALL *tp_mysql_stmt_result_metadata) (MYSQL_STMT *stmt);
typedef int *(STDCALL *tp_mysql_stmt_store_result) (MYSQL_STMT *stmt);

static tp_mysql_autocommit           ptr_mysql_autocommit;
static tp_mysql_close                ptr_mysql_close;
static tp_mysql_commit               ptr_mysql_commit;
static tp_mysql_errno                ptr_mysql_errno;
static tp_mysql_error                ptr_mysql_error;
static tp_mysql_fetch_field_direct   ptr_mysql_fetch_field_direct;
static tp_mysql_free_result          ptr_mysql_free_result;
static tp_mysql_init                 ptr_mysql_init;
static tp_mysql_num_fields           ptr_mysql_num_fields;
static tp_mysql_options              ptr_mysql_options;
static tp_mysql_real_connect         ptr_mysql_real_connect;
static tp_mysql_rollback             ptr_mysql_rollback;
static tp_mysql_set_character_set    ptr_mysql_set_character_set;
static tp_mysql_stmt_bind_param      ptr_mysql_stmt_bind_param;
static tp_mysql_stmt_bind_result     ptr_mysql_stmt_bind_result;
static tp_mysql_stmt_close           ptr_mysql_stmt_close;
static tp_mysql_stmt_errno           ptr_mysql_stmt_errno;
static tp_mysql_stmt_error           ptr_mysql_stmt_error;
static tp_mysql_stmt_execute         ptr_mysql_stmt_execute;
static tp_mysql_stmt_fetch           ptr_mysql_stmt_fetch;
static tp_mysql_stmt_fetch_column    ptr_mysql_stmt_fetch_column;
static tp_mysql_stmt_init            ptr_mysql_stmt_init;
static tp_mysql_stmt_param_count     ptr_mysql_stmt_param_count;
static tp_mysql_stmt_prepare         ptr_mysql_stmt_prepare;
static tp_mysql_stmt_result_metadata ptr_mysql_stmt_result_metadata;
static tp_mysql_stmt_store_result    ptr_mysql_stmt_store_result;

#define mysql_autocommit           ptr_mysql_autocommit
#define mysql_close                ptr_mysql_close
#define mysql_commit               ptr_mysql_commit
#define mysql_errno                ptr_mysql_errno
#define mysql_error                ptr_mysql_error
#define mysql_fetch_field_direct   ptr_mysql_fetch_field_direct
#define mysql_free_result          ptr_mysql_free_result
#define mysql_init                 ptr_mysql_init
#define mysql_num_fields           ptr_mysql_num_fields
#define mysql_options              ptr_mysql_options
#define mysql_real_connect         ptr_mysql_real_connect
#define mysql_rollback             ptr_mysql_rollback
#define mysql_set_character_set    ptr_mysql_set_character_set
#define mysql_stmt_bind_param      ptr_mysql_stmt_bind_param
#define mysql_stmt_bind_result     ptr_mysql_stmt_bind_result
#define mysql_stmt_close           ptr_mysql_stmt_close
#define mysql_stmt_errno           ptr_mysql_stmt_errno
#define mysql_stmt_error           ptr_mysql_stmt_error
#define mysql_stmt_execute         ptr_mysql_stmt_execute
#define mysql_stmt_fetch           ptr_mysql_stmt_fetch
#define mysql_stmt_fetch_column    ptr_mysql_stmt_fetch_column
#define mysql_stmt_init            ptr_mysql_stmt_init
#define mysql_stmt_param_count     ptr_mysql_stmt_param_count
#define mysql_stmt_prepare         ptr_mysql_stmt_prepare
#define mysql_stmt_result_metadata ptr_mysql_stmt_result_metadata
#define mysql_stmt_store_result    ptr_mysql_stmt_store_result



static boolType setupDll (const char *dllName)

  {
    static void *dbDll = NULL;

  /* setupDll */
    logFunction(printf("setupDll(\"%s\")\n", dllName););
    if (dbDll == NULL) {
      dbDll = dllOpen(dllName);
      if (dbDll != NULL) {
        if ((mysql_autocommit           = (tp_mysql_autocommit)           dllFunc(dbDll, "mysql_autocommit"))           == NULL ||
            (mysql_close                = (tp_mysql_close)                dllFunc(dbDll, "mysql_close"))                == NULL ||
            (mysql_commit               = (tp_mysql_commit)               dllFunc(dbDll, "mysql_commit"))               == NULL ||
            (mysql_errno                = (tp_mysql_errno)                dllFunc(dbDll, "mysql_errno"))                == NULL ||
            (mysql_error                = (tp_mysql_error)                dllFunc(dbDll, "mysql_error"))                == NULL ||
            (mysql_fetch_field_direct   = (tp_mysql_fetch_field_direct)   dllFunc(dbDll, "mysql_fetch_field_direct"))   == NULL ||
            (mysql_free_result          = (tp_mysql_free_result)          dllFunc(dbDll, "mysql_free_result"))          == NULL ||
            (mysql_init                 = (tp_mysql_init)                 dllFunc(dbDll, "mysql_init"))                 == NULL ||
            (mysql_num_fields           = (tp_mysql_num_fields)           dllFunc(dbDll, "mysql_num_fields"))           == NULL ||
            (mysql_options              = (tp_mysql_options)              dllFunc(dbDll, "mysql_options"))              == NULL ||
            (mysql_real_connect         = (tp_mysql_real_connect)         dllFunc(dbDll, "mysql_real_connect"))         == NULL ||
            (mysql_rollback             = (tp_mysql_rollback)             dllFunc(dbDll, "mysql_rollback"))             == NULL ||
            (mysql_set_character_set    = (tp_mysql_set_character_set)    dllFunc(dbDll, "mysql_set_character_set"))    == NULL ||
            (mysql_stmt_bind_param      = (tp_mysql_stmt_bind_param)      dllFunc(dbDll, "mysql_stmt_bind_param"))      == NULL ||
            (mysql_stmt_bind_result     = (tp_mysql_stmt_bind_result)     dllFunc(dbDll, "mysql_stmt_bind_result"))     == NULL ||
            (mysql_stmt_close           = (tp_mysql_stmt_close)           dllFunc(dbDll, "mysql_stmt_close"))           == NULL ||
            (mysql_stmt_errno           = (tp_mysql_stmt_errno)           dllFunc(dbDll, "mysql_stmt_errno"))           == NULL ||
            (mysql_stmt_error           = (tp_mysql_stmt_error)           dllFunc(dbDll, "mysql_stmt_error"))           == NULL ||
            (mysql_stmt_execute         = (tp_mysql_stmt_execute)         dllFunc(dbDll, "mysql_stmt_execute"))         == NULL ||
            (mysql_stmt_fetch           = (tp_mysql_stmt_fetch)           dllFunc(dbDll, "mysql_stmt_fetch"))           == NULL ||
            (mysql_stmt_fetch_column    = (tp_mysql_stmt_fetch_column)    dllFunc(dbDll, "mysql_stmt_fetch_column"))    == NULL ||
            (mysql_stmt_init            = (tp_mysql_stmt_init)            dllFunc(dbDll, "mysql_stmt_init"))            == NULL ||
            (mysql_stmt_param_count     = (tp_mysql_stmt_param_count)     dllFunc(dbDll, "mysql_stmt_param_count"))     == NULL ||
            (mysql_stmt_prepare         = (tp_mysql_stmt_prepare)         dllFunc(dbDll, "mysql_stmt_prepare"))         == NULL ||
            (mysql_stmt_result_metadata = (tp_mysql_stmt_result_metadata) dllFunc(dbDll, "mysql_stmt_result_metadata")) == NULL ||
            (mysql_stmt_store_result    = (tp_mysql_stmt_store_result)    dllFunc(dbDll, "mysql_stmt_store_result"))    == NULL) {
          dbDll = NULL;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setupDll --> %d\n", dbDll != NULL););
    return dbDll != NULL;
  } /* setupDll */



static boolType findDll (void)

  {
    const char *dllList[] = { MYSQL_DLL };
    unsigned int pos;
    boolType found = FALSE;

  /* findDll */
    for (pos = 0; pos < sizeof(dllList) / sizeof(char *) && !found; pos++) {
      found = setupDll(dllList[pos]);
    } /* for */
    if (!found) {
      dllErrorMessage("sqlOpenMy", "findDll", dllList,
                      sizeof(dllList) / sizeof(char *));
    } /* if */
    return found;
  } /* findDll */

#else

#define findDll() TRUE

#endif



static void sqlClose (databaseType database);



static void setDbErrorMsg (const char *funcName, const char *dbFuncName,
    unsigned int my_errno, const char *my_error)

  { /* setDbErrorMsg */
    dbError.funcName = funcName;
    dbError.dbFuncName = dbFuncName;
    dbError.errorCode = my_errno;
    snprintf(dbError.message, DB_ERR_MESSAGE_SIZE, "%s", my_error);
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
    FREE_RECORD2(db, dbRecordMy, count.database, count.database_bytes);
    logFunction(printf("freeDatabase -->\n"););
  } /* freeDatabase */



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
    if (preparedStmt->param_array != NULL) {
      for (pos = 0; pos < preparedStmt->param_array_size; pos++) {
        free(preparedStmt->param_array[pos].buffer);
      } /* for */
      FREE_TABLE(preparedStmt->param_array, MYSQL_BIND, preparedStmt->param_array_size);
      FREE_TABLE(preparedStmt->param_data_array, bindDataRecordMy, preparedStmt->param_array_size);
    } /* if */
    if (preparedStmt->result_array != NULL) {
      for (pos = 0; pos < preparedStmt->result_array_size; pos++) {
        free(preparedStmt->result_array[pos].buffer);
      } /* for */
      FREE_TABLE(preparedStmt->result_array, MYSQL_BIND, preparedStmt->result_array_size);
    } /* if */
    if (preparedStmt->result_data_array != NULL) {
      FREE_TABLE(preparedStmt->result_data_array, resultDataRecordMy, preparedStmt->result_array_size);
    } /* if */
    mysql_stmt_close(preparedStmt->ppStmt);
    FREE_RECORD2(preparedStmt, preparedStmtRecordMy,
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
      case MYSQL_TYPE_TINY:        typeName = "MYSQL_TYPE_TINY"; break;
      case MYSQL_TYPE_SHORT:       typeName = "MYSQL_TYPE_SHORT"; break;
      case MYSQL_TYPE_INT24:       typeName = "MYSQL_TYPE_INT24"; break;
      case MYSQL_TYPE_LONG:        typeName = "MYSQL_TYPE_LONG"; break;
      case MYSQL_TYPE_LONGLONG:    typeName = "MYSQL_TYPE_LONGLONG"; break;
      case MYSQL_TYPE_FLOAT:       typeName = "MYSQL_TYPE_FLOAT"; break;
      case MYSQL_TYPE_DOUBLE:      typeName = "MYSQL_TYPE_DOUBLE"; break;
      case MYSQL_TYPE_DECIMAL:     typeName = "MYSQL_TYPE_DECIMAL"; break;
      case MYSQL_TYPE_NEWDECIMAL:  typeName = "MYSQL_TYPE_NEWDECIMAL"; break;
      case MYSQL_TYPE_TIME:        typeName = "MYSQL_TYPE_TIME"; break;
      case MYSQL_TYPE_DATE:        typeName = "MYSQL_TYPE_DATE"; break;
      case MYSQL_TYPE_NEWDATE:     typeName = "MYSQL_TYPE_NEWDATE"; break;
      case MYSQL_TYPE_DATETIME:    typeName = "MYSQL_TYPE_DATETIME"; break;
      case MYSQL_TYPE_TIMESTAMP:   typeName = "MYSQL_TYPE_TIMESTAMP"; break;
      case MYSQL_TYPE_VARCHAR:     typeName = "MYSQL_TYPE_VARCHAR"; break;
      case MYSQL_TYPE_STRING:      typeName = "MYSQL_TYPE_STRING"; break;
      case MYSQL_TYPE_VAR_STRING:  typeName = "MYSQL_TYPE_VAR_STRING"; break;
      case MYSQL_TYPE_TINY_BLOB:   typeName = "MYSQL_TYPE_TINY_BLOB"; break;
      case MYSQL_TYPE_BLOB:        typeName = "MYSQL_TYPE_BLOB"; break;
      case MYSQL_TYPE_MEDIUM_BLOB: typeName = "MYSQL_TYPE_MEDIUM_BLOB"; break;
      case MYSQL_TYPE_LONG_BLOB:   typeName = "MYSQL_TYPE_LONG_BLOB"; break;
      case MYSQL_TYPE_BIT:         typeName = "MYSQL_TYPE_BIT"; break;
      case MYSQL_TYPE_YEAR:        typeName = "MYSQL_TYPE_YEAR"; break;
      case MYSQL_TYPE_SET:         typeName = "MYSQL_TYPE_SET"; break;
      case MYSQL_TYPE_ENUM:        typeName = "MYSQL_TYPE_ENUM"; break;
      case MYSQL_TYPE_GEOMETRY:    typeName = "MYSQL_TYPE_GEOMETRY"; break;
      case MYSQL_TYPE_NULL:        typeName = "MYSQL_TYPE_NULL"; break;
      default:
        sprintf(buffer, "%d", buffer_type);
        typeName = buffer;
        break;
    } /* switch */
    logFunction(printf("nameOfBufferType --> %s\n", typeName););
    return typeName;
  } /* nameOfBufferType */
#endif



/**
 *  Process strings delimited by single quotes (') and double quotes (").
 *  Depending on the parameter backslashEscapes backslashes (\) are
 *  replaced with double backslashes (\\) in strings, that are delimited
 *  by single quotes ('). This way a backslash in a SQL statement has no
 *  special meaning (like it is common practice in other SQL databases).
 *  Strings delimited with double quotes (") are converted into
 *  strings, that are delimited with a backtick (`). This way
 *  identifiers in a SQL statement can be enclosed in double
 *  quotes (") (like it is common proctice in other SQL databases).
 *  The function processes (and removes) also comments, to avoid that
 *  a quote (') inside a comment is misinterpreted as literal.
 *  This processing does not prohibit any form of SQL injection.
 *  It is strongly recommended that string literals in SQL statements
 *  are constant and never be constructed with data from outside.
 */
static striType processStatementStri (const const_striType sqlStatementStri,
    boolType backslashEscapes)

  {
    memSizeType pos = 0;
    strElemType ch;
    memSizeType destPos = 0;
    striType processed;

  /* processStatementStri */
    logFunction(printf("processStatementStri(\"%s\", %d)\n",
                       striAsUnquotedCStri(sqlStatementStri), backslashEscapes););
    if (unlikely(sqlStatementStri->size > MAX_STRI_LEN / 2 ||
                 !ALLOC_STRI_SIZE_OK(processed, sqlStatementStri->size * 2))) {
      processed = NULL;
    } else {
      while (pos < sqlStatementStri->size) {
        ch = sqlStatementStri->mem[pos];
        if (ch == '\'') {
          processed->mem[destPos++] = '\'';
          pos++;
          while (pos < sqlStatementStri->size &&
              (ch = sqlStatementStri->mem[pos]) != '\'') {
            if (ch == '\\' && backslashEscapes) {
              processed->mem[destPos++] = ch;
            } /* if */
            processed->mem[destPos++] = ch;
            pos++;
          } /* while */
          if (pos < sqlStatementStri->size) {
            processed->mem[destPos++] = '\'';
            pos++;
          } /* if */
        } else if (ch == '"') {
          processed->mem[destPos++] = '`';
          pos++;
          do {
            while (pos < sqlStatementStri->size &&
                (ch = sqlStatementStri->mem[pos]) != '"') {
              if (ch == '`') {
                processed->mem[destPos++] = ch;
              } /* if */
              processed->mem[destPos++] = ch;
              pos++;
            } /* while */
            if (pos < sqlStatementStri->size) {
              pos++;
              if (pos < sqlStatementStri->size &&
                  (ch = sqlStatementStri->mem[pos]) == '"') {
                processed->mem[destPos++] = '"';
                pos++;
              } /* if */
            } /* if */
          } while (pos < sqlStatementStri->size && ch == '"');
          if (pos < sqlStatementStri->size) {
            processed->mem[destPos++] = '`';
          } /* if */
        } else if (ch == '`') {
          processed->mem[destPos++] = '`';
          pos++;
          while (pos < sqlStatementStri->size &&
              (ch = sqlStatementStri->mem[pos]) != '`') {
            processed->mem[destPos++] = ch;
            pos++;
          } /* while */
          if (pos < sqlStatementStri->size) {
            processed->mem[destPos++] = '`';
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
      processed->size = destPos;
    } /* if */
    logFunction(printf("processStatementStri --> \"%s\"\n",
                       striAsUnquotedCStri(processed)););
    return processed;
  } /* processStatementStri */



static errInfoType setupParameters (preparedStmtType preparedStmt)

  {
    unsigned long num_params;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupParameters */
    logFunction(printf("setupParameters\n"););
    num_params = mysql_stmt_param_count(preparedStmt->ppStmt);
    if (num_params == 0) {
      /* malloc(0) may return NULL, which would wrongly trigger a MEMORY_ERROR. */
      preparedStmt->param_array_size = 0;
      preparedStmt->param_array = NULL;
      preparedStmt->param_data_array = NULL;
    } else if (unlikely(!ALLOC_TABLE(preparedStmt->param_array,
                                     MYSQL_BIND, (memSizeType) num_params))) {
      err_info = MEMORY_ERROR;
    } else if (unlikely(!ALLOC_TABLE(preparedStmt->param_data_array,
                                     bindDataRecordMy, (memSizeType) num_params))) {
      FREE_TABLE(preparedStmt->param_array, MYSQL_BIND, (memSizeType) num_params);
      err_info = MEMORY_ERROR;
    } else {
      preparedStmt->param_array_size = (memSizeType) num_params;
      memset(preparedStmt->param_array, 0,
             (memSizeType) num_params * sizeof(MYSQL_BIND));
      memset(preparedStmt->param_data_array, 0,
             (memSizeType) num_params * sizeof(bindDataRecordMy));
    } /* if */
    logFunction(printf("setupParameters --> %d\n", err_info););
    return err_info;
  } /* setupParameters */



static errInfoType setupResultColumn (preparedStmtType preparedStmt,
    unsigned int column_num, MYSQL_RES *result_metadata,
    MYSQL_BIND *resultData)

  {
    MYSQL_FIELD *column;
    memSizeType buffer_length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupResultColumn */
    logFunction(printf("setupResultColumn: column_num=%d\n", column_num););
    column = mysql_fetch_field_direct(result_metadata, column_num - 1);
    /* printf("column[%u]->type: %s\n", column_num, nameOfBufferType(column->type)); */
    /* printf("charsetnr: %u\n", column->charsetnr); */
    switch (column->type) {
      case MYSQL_TYPE_TINY:
        buffer_length = 1;
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_SHORT:
        buffer_length = 2;
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_INT24:
      case MYSQL_TYPE_LONG:
        buffer_length = 4;
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_LONGLONG:
        /* printf("MYSQL_LONGLONG: column->length = %lu\n", column->length);
        printf("MYSQL_LONGLONG: column->decimals = %d\n", column->decimals); */
        buffer_length = 8;
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_FLOAT:
        /* printf("MYSQL_TYPE_FLOAT: column->length = %lu\n", column->length);
        printf("MYSQL_TYPE_FLOAT: column->decimals = %d\n", column->decimals); */
        buffer_length = 4;
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_DOUBLE:
        /* printf("MYSQL_TYPE_DOUBLE: column->length = %lu\n", column->length);
        printf("MYSQL_TYPE_DOUBLE: column->decimals = %d\n", column->decimals); */
        buffer_length = 8;
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_DECIMAL:
      case MYSQL_TYPE_NEWDECIMAL:
        /* printf("MYSQL_TYPE_NEWDECIMAL: column->length = %lu\n", column->length);
        printf("MYSQL_TYPE_NEWDECIMAL: column->decimals = %d\n", column->decimals); */
        buffer_length = column->length;
        resultData->length = &resultData->length_value;
        break;
      case MYSQL_TYPE_TIME:
      case MYSQL_TYPE_DATE:
      case MYSQL_TYPE_DATETIME:
      case MYSQL_TYPE_TIMESTAMP:
        buffer_length = sizeof(MYSQL_TIME);
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_STRING:
      case MYSQL_TYPE_VAR_STRING:
        buffer_length = column->length;
        resultData->length = &resultData->length_value;
        break;
      case MYSQL_TYPE_TINY_BLOB:
      case MYSQL_TYPE_BLOB:
      case MYSQL_TYPE_MEDIUM_BLOB:
      case MYSQL_TYPE_LONG_BLOB:
        /* For BLOB types no buffer is provided (buffer == NULL).   */
        /* This way mysql_stmt_fetch() will not fetch data. Instead */
        /* mysql_stmt_fetch() returns MYSQL_DATA_TRUNCATED, which   */
        /* is ignored by sqlFetch(). The actual BLOB data is        */
        /* fetched with mysql_stmt_fetch_column().                  */
        buffer_length = 0;
        resultData->length = &resultData->length_value;
        break;
      default:
        logError(printf("setupResultColumn: Column %s has the unknown type %s.\n",
                        column->name, nameOfBufferType(column->type)););
        err_info = RANGE_ERROR;
        break;
    } /* switch */
    /* printf("buffer_length: " FMT_U_MEM "\n", buffer_length); */
    if (err_info == OKAY_NO_ERROR) {
      if (buffer_length != 0) {
        resultData->buffer = malloc(buffer_length);
        if (unlikely(resultData->buffer == NULL)) {
          err_info = MEMORY_ERROR;
        } else {
          resultData->buffer_type   = column->type;
          resultData->buffer_length = buffer_length;
          resultData->is_unsigned   = 0;
          resultData->is_null = &resultData->is_null_value;
          resultData->is_null_value = 0;
          preparedStmt->result_data_array[column_num - 1].name = column->name;
          /* A charsetnr value of 63 indicates that the          */
          /* character set is binary. This allows to distinguish */
          /* BINARY from CHAR, and VARBINARY from VARCHAR.       */
          preparedStmt->result_data_array[column_num - 1].binary = column->charsetnr == 63;
        } /* if */
      } else {
        /* This triggers that mysql_stmt_fetch() returns MYSQL_DATA_TRUNCATED. */
        /* Only BLOBs and CLOBs will have a buffer of NULL. */
        resultData->buffer = NULL;
        resultData->buffer_type   = column->type;
        resultData->buffer_length = 0;
        resultData->is_unsigned   = 0;
        resultData->is_null = &resultData->is_null_value;
        resultData->is_null_value = 0;
        preparedStmt->result_data_array[column_num - 1].name = column->name;
        /* A charsetnr value of 63 indicates that the */
        /* character set is binary. This allows to    */
        /* distinguish BLOB types from TEXT types.    */
        preparedStmt->result_data_array[column_num - 1].binary = column->charsetnr == 63;
      } /* if */
    } /* if */
    logFunction(printf("setupResultColumn --> %d\n", err_info););
    return err_info;
  } /* setupResultColumn */



static errInfoType setupResult (preparedStmtType preparedStmt)

  {
    MYSQL_RES *result_metadata;
    unsigned int num_columns;
    unsigned int column_index;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupResult */
    logFunction(printf("setupResult\n"););
    result_metadata = mysql_stmt_result_metadata(preparedStmt->ppStmt);
    if (result_metadata == NULL) {
      preparedStmt->result_array = NULL;
      preparedStmt->result_array_size = 0;
      preparedStmt->result_data_array = NULL;
    } else {
      num_columns = mysql_num_fields(result_metadata);
      /* printf("num_columns: %u\n", num_columns); */
      if (num_columns == 0) {
        /* malloc(0) may return NULL, which would wrongly trigger a MEMORY_ERROR. */
        preparedStmt->result_array_size = 0;
        preparedStmt->result_array = NULL;
        preparedStmt->result_data_array = NULL;
      } else if (unlikely(!ALLOC_TABLE(preparedStmt->result_array,
                                       MYSQL_BIND, num_columns))) {
        err_info = MEMORY_ERROR;
      } else if (unlikely(!ALLOC_TABLE(preparedStmt->result_data_array,
                                       resultDataRecordMy, num_columns))) {
        FREE_TABLE(preparedStmt->result_array, MYSQL_BIND, num_columns);
        err_info = MEMORY_ERROR;
      } else {
        preparedStmt->result_array_size = num_columns;
        memset(preparedStmt->result_array, 0, num_columns * sizeof(MYSQL_BIND));
        memset(preparedStmt->result_data_array, 0, num_columns * sizeof(resultDataRecordMy));
        for (column_index = 0; column_index < num_columns &&
             err_info == OKAY_NO_ERROR; column_index++) {
          err_info = setupResultColumn(preparedStmt, column_index + 1,
                                       result_metadata,
                                       &preparedStmt->result_array[column_index]);
        } /* for */
      } /* if */
      mysql_free_result(result_metadata);
    } /* if */
    logFunction(printf("setupResult --> %d\n", err_info););
    return err_info;
  } /* setupResult */



static boolType allParametersBound (preparedStmtType preparedStmt)

  {
    memSizeType column_index;
    boolType okay = TRUE;

  /* allParametersBound */
    for (column_index = 0; column_index < preparedStmt->param_array_size;
         column_index++) {
      if (unlikely(!preparedStmt->param_data_array[column_index].bound)) {
        logError(printf("sqlExecute: Unbound parameter " FMT_U_MEM ".\n",
                        column_index + 1););
        okay = FALSE;
      } /* if */
    } /* for */
    return okay;
  } /* allParametersBound */



static unsigned int setBigInt (const void *buffer, const const_bigIntType bigIntValue,
    errInfoType *err_info)

  {
    striType stri;
    unsigned char *decimal;
    unsigned int srcIndex;
    unsigned int destIndex;
    unsigned int length = 0;

  /* setBigInt */
    logFunction(printf("setBigInt(%s)\n", bigHexCStri(bigIntValue)););
    stri = bigStr(bigIntValue);
    if (unlikely(stri == NULL)) {
      *err_info = MEMORY_ERROR;
    } else {
      /* printf("%s\n", striAsUnquotedCStri(stri)); */
      decimal = (unsigned char *) buffer;
      srcIndex = 0;
      destIndex = 0;
      if (stri->mem[0] == '-') {
        decimal[0] = '-';
        srcIndex++;
        destIndex++;
      } /* if */
      if (unlikely(stri->size - srcIndex > MAX_DECIMAL_PRECISION)) {
        logError(printf("setBigInt: Number of digits (" FMT_U_MEM
                        ") larger than the allowed maximum (%d)\n.",
                        stri->size - srcIndex, MAX_DECIMAL_PRECISION););
        *err_info = RANGE_ERROR;
      } else {
        for (; srcIndex < stri->size; srcIndex++) {
          decimal[destIndex] = (unsigned char) stri->mem[srcIndex];
          destIndex++;
        } /* for */
        length = destIndex;
      } /* if */
      FREE_STRI(stri, stri->size);
    } /* if */
    logFunction(printf("setBigInt --> %u\n", length););
    return length;
  } /* setBigInt */



static unsigned int setBigRat (const void *buffer, const const_bigIntType numerator,
    const const_bigIntType denominator, errInfoType *err_info)

  {
    bigIntType number;
    bigIntType mantissaValue;
    striType stri;
    unsigned int striSizeUsed;
    unsigned int scale;
    unsigned char *decimal;
    memSizeType decimalSize;
    unsigned int srcIndex;
    unsigned int destIndex;
    unsigned int length = 0;

  /* setBigRat */
    logFunction(printf("setBigRat(*, %s, %s, *)\n",
                       bigHexCStri(numerator), bigHexCStri(denominator)););
    if (unlikely(bigEqSignedDigit(denominator, 0))) {
      /* Decimal values do not support Infinity and NaN. */
      logError(printf("setBigRat: Decimal values do not support Infinity and NaN.\n"););
      raise_error(RANGE_ERROR);
    } else {
      number = bigIPowSignedDigit(10, MAX_DECIMAL_SCALE);
      if (unlikely(number == NULL)) {
        mantissaValue = NULL;
      } else {
        bigMultAssign(&number, numerator);
        mantissaValue = bigDiv(number, denominator);
        bigDestr(number);
      } /* if */
      if (unlikely(mantissaValue == NULL)) {
        decimal = NULL;
      } else if (bigEqSignedDigit(mantissaValue, 0)) {
        memcpy((char *) buffer, "0.0", 3);
      } else {
        stri = bigStr(mantissaValue);
        if (unlikely(stri == NULL)) {
          *err_info = MEMORY_ERROR;
        } else if (unlikely(stri->size > UINT_MAX)) {
          /* It is not possible to cast stri->size to unsigned int. */
          FREE_STRI(stri, stri->size);
          *err_info = MEMORY_ERROR;
        } else {
          /* printf("%s\n", striAsUnquotedCStri(stri)); */
          striSizeUsed = (unsigned int) stri->size;
          scale = MAX_DECIMAL_SCALE;
          while (scale >= 2 && stri->mem[striSizeUsed - 1] == '0') {
            scale--;
            striSizeUsed--;
          } /* while */
          decimalSize = striSizeUsed;
          if (decimalSize - (stri->mem[0] == '-') > scale) {
            /* Add one character for the decimal point. */
            decimalSize += 1;
          } else {
            /* Add space for sign, zero and decimal point. */
            decimalSize = (stri->mem[0] == '-') + scale + 2;
          } /* if */
          if (unlikely(decimalSize > MAX_DECIMAL_LENGTH)) {
            logError(printf("setBigRat: decimalSize (" FMT_U_MEM
                            ") larger than maximum (%d).\n",
                            decimalSize, MAX_DECIMAL_LENGTH););
            *err_info = RANGE_ERROR;
          } else {
            decimal = (unsigned char *) buffer;
            srcIndex = 0;
            destIndex = 0;
            if (stri->mem[0] == '-') {
              decimal[0] = '-';
              srcIndex++;
              destIndex++;
            } /* if */
            if (striSizeUsed - srcIndex > scale) {
              for (; srcIndex < striSizeUsed - scale; srcIndex++) {
                decimal[destIndex] = (unsigned char) stri->mem[srcIndex];
                destIndex++;
              } /* for */
              decimal[destIndex] = '.';
              destIndex++;
            } else {
              decimal[destIndex] = '0';
              destIndex++;
              decimal[destIndex] = '.';
              destIndex++;
              memset(&decimal[destIndex], '0', scale - striSizeUsed + srcIndex);
              destIndex += scale - striSizeUsed + srcIndex;
            } /* if */
            for (; srcIndex < striSizeUsed; srcIndex++) {
              decimal[destIndex] = (unsigned char) stri->mem[srcIndex];
              destIndex++;
            } /* for */
            length = destIndex;
          } /* if */
          FREE_STRI(stri, stri->size);
        } /* if */
        bigDestr(mantissaValue);
      } /* if */
    } /* if */
#if 0
    printf("length: %u\n", length);
    for (srcIndex = 0; srcIndex < length; srcIndex++) {
      printf(" %u", ((unsigned char *) buffer)[srcIndex]);
    } /* for */
    printf("\n");
    for (srcIndex = 0; srcIndex < length; srcIndex++) {
      printf("%c", ((unsigned char *) buffer)[srcIndex]);
    } /* for */
    printf("\n");
#endif
    return length;
  } /* setBigRat */



static void sqlBindBigInt (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType value)

  {
    preparedStmtType preparedStmt;
    MYSQL_BIND *param;
    unsigned int length;
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
      param = &preparedStmt->param_array[pos - 1];
      if (preparedStmt->param_data_array[pos - 1].buffer_capacity < MAX_DECIMAL_LENGTH) {
        free(param->buffer);
        if (unlikely((param->buffer = malloc(MAX_DECIMAL_LENGTH)) == NULL)) {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = MAX_DECIMAL_LENGTH;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        length = setBigInt(param->buffer,
                           value, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          param->buffer_type = MYSQL_TYPE_NEWDECIMAL;
          param->is_unsigned = 0;
          param->is_null     = NULL;
          param->length      = NULL;
          param->buffer_length = length;
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_data_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBigInt */



static void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType numerator, const const_bigIntType denominator)

  {
    preparedStmtType preparedStmt;
    MYSQL_BIND *param;
    unsigned int length;
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
      param = &preparedStmt->param_array[pos - 1];
      if (preparedStmt->param_data_array[pos - 1].buffer_capacity < MAX_DECIMAL_LENGTH) {
        free(param->buffer);
        if (unlikely((param->buffer = malloc(MAX_DECIMAL_LENGTH)) == NULL)) {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = MAX_DECIMAL_LENGTH;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        length = setBigRat(param->buffer,
                           numerator, denominator, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          param->buffer_type = MYSQL_TYPE_NEWDECIMAL;
          param->is_unsigned = 0;
          param->is_null     = NULL;
          param->length      = NULL;
          param->buffer_length = length;
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_data_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBigRat */



static void sqlBindBool (sqlStmtType sqlStatement, intType pos, boolType value)

  {
    preparedStmtType preparedStmt;
    MYSQL_BIND *param;
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
      param = &preparedStmt->param_array[pos - 1];
      if (preparedStmt->param_data_array[pos - 1].buffer_capacity < sizeof(char)) {
        free(param->buffer);
        if (unlikely((param->buffer = malloc(sizeof(char))) == NULL)) {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = sizeof(char);
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        param->buffer_type   = MYSQL_TYPE_STRING;
        param->buffer_length = 1;
        param->is_unsigned   = 0;
        param->is_null       = NULL;
        param->length        = NULL;
        ((char *) param->buffer)[0] = (char) ('0' + value);
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->param_data_array[pos - 1].bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindBool */



static void sqlBindBStri (sqlStmtType sqlStatement, intType pos,
    const const_bstriType bstri)

  {
    preparedStmtType preparedStmt;
    MYSQL_BIND *param;
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
      param = &preparedStmt->param_array[pos - 1];
      if (unlikely(bstri->size > MAX_LONG_BLOB_LENGTH)) {
        raise_error(MEMORY_ERROR);
      } else {
        if (preparedStmt->param_data_array[pos - 1].buffer_capacity < bstri->size) {
          free(param->buffer);
          if (unlikely((param->buffer = malloc(bstri->size)) == NULL)) {
            preparedStmt->param_data_array[pos - 1].buffer_capacity = 0;
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->param_data_array[pos - 1].buffer_capacity = bstri->size;
          } /* if */
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          if (bstri->size <= MAX_TINY_BLOB_LENGTH) {
            param->buffer_type = MYSQL_TYPE_TINY_BLOB;
          } else if (bstri->size <= MAX_BLOB_LENGTH) {
            param->buffer_type = MYSQL_TYPE_BLOB;
          } else if (bstri->size <= MAX_MEDIUM_BLOB_LENGTH) {
            param->buffer_type = MYSQL_TYPE_MEDIUM_BLOB;
          } else {
            param->buffer_type = MYSQL_TYPE_LONG_BLOB;
          } /* if */
          /* printf("sqlBindBStri: buffer_type: %s\n",
             nameOfBufferType(param->buffer_type)); */
          param->is_unsigned   = 0;
          param->is_null       = NULL;
          param->length        = NULL;
          memcpy(param->buffer, bstri->mem, bstri->size);
          param->buffer_length = (unsigned long) bstri->size;
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_data_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBStri */



static void sqlBindDuration (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  {
    preparedStmtType preparedStmt;
    MYSQL_BIND *param;
    int64Type monthDuration;
    int64Type microsecDuration;
    MYSQL_TIME *timeValue;
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
    } else if (unlikely(year < -INT_MAX || year > INT_MAX || month < -12 || month > 12 ||
                        day < -31 || day > 31 || hour <= -24 || hour >= 24 ||
                        minute <= -60 || minute >= 60 || second <= -60 || second >= 60 ||
                        micro_second <= -1000000 || micro_second >= 1000000)) {
      logError(printf("sqlBindDuration: Duration not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      if (preparedStmt->param_data_array[pos - 1].buffer_capacity < sizeof(MYSQL_TIME)) {
        free(param->buffer);
        if (unlikely((param->buffer = malloc(sizeof(MYSQL_TIME))) == NULL)) {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = sizeof(MYSQL_TIME);
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        monthDuration = (int64Type) year * 12 + (int64Type) month;
        microsecDuration = (((((int64Type) day) * 24 +
                               (int64Type) hour) * 60 +
                               (int64Type) minute) * 60 +
                               (int64Type) second) * 1000000 +
                               (int64Type) micro_second;
        /* printf("monthDuration: " FMT_D64 "\n", monthDuration);
           printf("microsecDuration: " FMT_D64 "\n", microsecDuration); */
        if (unlikely(!((monthDuration >= 0 && microsecDuration >= 0) ||
                       (monthDuration <= 0 && microsecDuration <= 0)))) {
          logError(printf("sqlBindDuration: Duration neither clearly positive nor negative.\n"););
          raise_error(RANGE_ERROR);
        } else {
          param->buffer_type = MYSQL_TYPE_DATETIME;
          param->is_unsigned = 0;
          param->is_null     = NULL;
          param->length      = NULL;
          timeValue = (MYSQL_TIME *) param->buffer;
          if (monthDuration >= 0 && microsecDuration >= 0) {
            timeValue->neg    = 0;
          } else {
            monthDuration = -monthDuration;
            microsecDuration = -microsecDuration;
            timeValue->neg    = 1;
          } /* if */
          timeValue->month       = (unsigned int) monthDuration % 12;
          timeValue->year        = (unsigned int) monthDuration / 12;
          timeValue->second_part = (unsigned long) microsecDuration % 1000000;
          microsecDuration /= 1000000;
          timeValue->second      = (unsigned int) microsecDuration % 60;
          microsecDuration /= 60;
          timeValue->minute      = (unsigned int) microsecDuration % 60;
          microsecDuration /= 60;
          timeValue->hour        = (unsigned int) microsecDuration % 24;
          timeValue->day         = (unsigned int) microsecDuration / 24;
          timeValue->time_type   = MYSQL_TIMESTAMP_DATETIME;
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_data_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindDuration */



static void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value)

  {
    preparedStmtType preparedStmt;
    MYSQL_BIND *param;
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
      param = &preparedStmt->param_array[pos - 1];
      if (preparedStmt->param_data_array[pos - 1].buffer_capacity < sizeof(floatType)) {
        free(param->buffer);
        if (unlikely((param->buffer = malloc(sizeof(floatType))) == NULL)) {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = sizeof(floatType);
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
#if FLOATTYPE_SIZE == 32
        param->buffer_type = MYSQL_TYPE_FLOAT;
#elif FLOATTYPE_SIZE == 64
        param->buffer_type = MYSQL_TYPE_DOUBLE;
#endif
        param->is_unsigned = 0;
        param->is_null     = NULL;
        param->length      = NULL;
        *(floatType *) param->buffer = value;
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->param_data_array[pos - 1].bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindFloat */



static void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value)

  {
    preparedStmtType preparedStmt;
    MYSQL_BIND *param;
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
      param = &preparedStmt->param_array[pos - 1];
      if (preparedStmt->param_data_array[pos - 1].buffer_capacity < sizeof(intType)) {
        free(param->buffer);
        if (unlikely((param->buffer = malloc(sizeof(intType))) == NULL)) {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = sizeof(intType);
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
#if INTTYPE_SIZE == 32
        param->buffer_type = MYSQL_TYPE_LONG;
#elif INTTYPE_SIZE == 64
        param->buffer_type = MYSQL_TYPE_LONGLONG;
#endif
        param->is_unsigned = 0;
        param->is_null     = NULL;
        param->length      = NULL;
        *(intType *) param->buffer = value;
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->param_data_array[pos - 1].bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindInt */



static void sqlBindNull (sqlStmtType sqlStatement, intType pos)

  {
    preparedStmtType preparedStmt;
    MYSQL_BIND *param;

  /* sqlBindNull */
    logFunction(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindNull: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      param->is_null = &param->is_null_value;
      param->is_null_value = 1;
      preparedStmt->executeSuccessful = FALSE;
      preparedStmt->fetchOkay = FALSE;
      preparedStmt->param_data_array[pos - 1].bound = TRUE;
    } /* if */
  } /* sqlBindNull */



static void sqlBindStri (sqlStmtType sqlStatement, intType pos,
    const const_striType stri)

  {
    preparedStmtType preparedStmt;
    MYSQL_BIND *param;
    cstriType stri8;
    cstriType resized_stri8;
    memSizeType length;

  /* sqlBindStri */
    logFunction(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, striAsUnquotedCStri(stri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindStri: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      stri8 = stri_to_cstri8_buf(stri, &length);
      if (unlikely(stri8 == NULL)) {
        raise_error(MEMORY_ERROR);
      } else if (unlikely(length > ULONG_MAX)) {
        /* It is not possible to cast length to unsigned long. */
        free(stri8);
        raise_error(MEMORY_ERROR);
      } else {
        resized_stri8 = REALLOC_CSTRI(stri8, length);
        if (likely(resized_stri8 != NULL)) {
          stri8 = resized_stri8;
        } /* if */
        free(param->buffer);
        param->buffer_type   = MYSQL_TYPE_STRING;
        param->is_unsigned   = 0;
        param->is_null       = NULL;
        param->length        = NULL;
        param->buffer        = stri8;
        param->buffer_length = (unsigned long) length;
        preparedStmt->param_data_array[pos - 1].buffer_capacity = length;
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->param_data_array[pos - 1].bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindStri */



static void sqlBindTime (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second,
    intType time_zone)

  {
    preparedStmtType preparedStmt;
    MYSQL_BIND *param;
    MYSQL_TIME *timeValue;
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
    } else if (unlikely(year < -INT_MAX || year > INT_MAX || month < 1 || month > 12 ||
                        day < 1 || day > 31 || hour < 0 || hour >= 24 ||
                        minute < 0 || minute >= 60 || second < 0 || second >= 60 ||
                        micro_second < 0 || micro_second >= 1000000)) {
      logError(printf("sqlBindTime: Time not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      if (preparedStmt->param_data_array[pos - 1].buffer_capacity < sizeof(MYSQL_TIME)) {
        free(param->buffer);
        if (unlikely((param->buffer = malloc(sizeof(MYSQL_TIME))) == NULL)) {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          preparedStmt->param_data_array[pos - 1].buffer_capacity = sizeof(MYSQL_TIME);
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        param->buffer_type = MYSQL_TYPE_DATETIME;
        param->is_unsigned = 0;
        param->is_null     = NULL;
        param->length      = NULL;
        timeValue = (MYSQL_TIME *) param->buffer;
        if (year < 0) {
          timeValue->year = (unsigned int) -year;
        } else {
          timeValue->year = (unsigned int) year;
        } /* if */
        timeValue->month  = (unsigned int) month;
        timeValue->day    = (unsigned int) day;
        timeValue->hour   = (unsigned int) hour;
        timeValue->minute = (unsigned int) minute;
        timeValue->second = (unsigned int) second;
        timeValue->neg    = year < 0;
        timeValue->second_part = (unsigned long) micro_second;
        timeValue->time_type = MYSQL_TIMESTAMP_DATETIME;
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->param_data_array[pos - 1].bound = TRUE;
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
      mysql_close(db->connection);
      db->connection = NULL;
    } /* if */
    logFunction(printf("sqlClose -->\n"););
  } /* sqlClose */



static bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    MYSQL_BIND *columnData;
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
      if (columnData->is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        columnValue = bigZero();
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(columnData->buffer_type)); */
        switch (columnData->buffer_type) {
          case MYSQL_TYPE_TINY:
            columnValue = bigFromInt32((int32Type)
                *(int8Type *) columnData->buffer);
            break;
          case MYSQL_TYPE_SHORT:
            columnValue = bigFromInt32((int32Type)
                *(int16Type *) columnData->buffer);
            break;
          case MYSQL_TYPE_INT24:
          case MYSQL_TYPE_LONG:
            columnValue = bigFromInt32(
                *(int32Type *) columnData->buffer);
            break;
          case MYSQL_TYPE_LONGLONG:
            columnValue = bigFromInt64(
                *(int64Type *) columnData->buffer);
            break;
          case MYSQL_TYPE_DECIMAL:
          case MYSQL_TYPE_NEWDECIMAL:
            columnValue = getDecimalBigInt(
                (const_ustriType) columnData->buffer,
                columnData->length_value);
            break;
          default:
            logError(printf("sqlColumnBigInt: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            columnData->buffer_type)););
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
    MYSQL_BIND *columnData;
    float floatValue;
    double doubleValue;

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
      if (columnData->is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        *numerator = bigZero();
        *denominator = bigFromInt32(1);
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(columnData->buffer_type)); */
        switch (columnData->buffer_type) {
          case MYSQL_TYPE_TINY:
            *numerator = bigFromInt32((int32Type)
                *(int8Type *) columnData->buffer);
            *denominator = bigFromInt32(1);
            break;
          case MYSQL_TYPE_SHORT:
            *numerator = bigFromInt32((int32Type)
                *(int16Type *) columnData->buffer);
            *denominator = bigFromInt32(1);
            break;
          case MYSQL_TYPE_INT24:
          case MYSQL_TYPE_LONG:
            *numerator = bigFromInt32(
                *(int32Type *) columnData->buffer);
            *denominator = bigFromInt32(1);
            break;
          case MYSQL_TYPE_LONGLONG:
            *numerator = bigFromInt64(
                *(int64Type *) columnData->buffer);
            *denominator = bigFromInt32(1);
            break;
          case MYSQL_TYPE_FLOAT:
            floatValue = *(float *) columnData->buffer;
            if (floatValue == MOST_POSITIVE_FLOAT) {
              /* The IEEE 754 value infinity is stored as the most positive value. */
              floatValue = (float) POSITIVE_INFINITY;
            } else if (floatValue == MOST_NEGATIVE_FLOAT) {
              /* The IEEE 754 value -infinity is stored as the most negative value. */
              floatValue = (float) NEGATIVE_INFINITY;
            } /* if */
            /* printf("sqlColumnBigRat: float: %f\n", floatValue); */
            *numerator = roundDoubleToBigRat(floatValue, FALSE, denominator);
            break;
          case MYSQL_TYPE_DOUBLE:
            doubleValue = *(double *) columnData->buffer;
            if (doubleValue == MOST_POSITIVE_DOUBLE) {
              /* The IEEE 754 value infinity is stored as the most positive value. */
              doubleValue = POSITIVE_INFINITY;
            } else if (doubleValue == MOST_NEGATIVE_DOUBLE) {
              /* The IEEE 754 value -infinity is stored as the most negative value. */
              doubleValue = NEGATIVE_INFINITY;
            } /* if */
            /* printf("sqlColumnBigRat: double: %f\n", doubleValue); */
            *numerator = roundDoubleToBigRat(doubleValue, TRUE, denominator);
            break;
          case MYSQL_TYPE_DECIMAL:
          case MYSQL_TYPE_NEWDECIMAL:
            *numerator = getDecimalBigRational(
                (const_ustriType) columnData->buffer,
                columnData->length_value, denominator);
            break;
          default:
            logError(printf("sqlColumnBigRat: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            columnData->buffer_type)););
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
    MYSQL_BIND *columnData;
    int64Type columnValue;

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
      if (columnData->is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: FALSE\n"); */
        columnValue = 0;
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(columnData->buffer_type)); */
        switch (columnData->buffer_type) {
          case MYSQL_TYPE_TINY:
            columnValue = *(int8Type *) columnData->buffer;
            break;
          case MYSQL_TYPE_SHORT:
            columnValue = *(int16Type *) columnData->buffer;
            break;
          case MYSQL_TYPE_INT24:
          case MYSQL_TYPE_LONG:
            columnValue = *(int32Type *) columnData->buffer;
            break;
          case MYSQL_TYPE_LONGLONG:
            columnValue = *(int64Type *) columnData->buffer;
            break;
          case MYSQL_TYPE_STRING:
          case MYSQL_TYPE_VAR_STRING:
            if (unlikely(columnData->length_value != 1)) {
              logError(printf("sqlColumnBool: Column " FMT_D ": "
                              "The size of a boolean field must be 1.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = 0;
            } else {
              columnValue = *(const_cstriType) columnData->buffer - '0';
            } /* if */
            break;
          default:
            logError(printf("sqlColumnBool: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            columnData->buffer_type)););
            raise_error(RANGE_ERROR);
            columnValue = 0;
            break;
        } /* switch */
        if (unlikely((uint64Type) columnValue >= 2)) {
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
    MYSQL_BIND *columnData;
    memSizeType length;
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
      if (columnData->is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: \"\"\n"); */
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(columnValue, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          columnValue->size = 0;
        } /* if */
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(columnData->buffer_type)); */
        switch (columnData->buffer_type) {
          case MYSQL_TYPE_TINY_BLOB:
          case MYSQL_TYPE_BLOB:
          case MYSQL_TYPE_MEDIUM_BLOB:
          case MYSQL_TYPE_LONG_BLOB:
            if (!preparedStmt->result_data_array[column - 1].binary) {
              logError(printf("sqlColumnBStri: Column " FMT_D " is a CLOB.\n",
                              column););
              raise_error(RANGE_ERROR);
              columnValue = NULL;
            } else {
              length = columnData->length_value;
              /* printf("length: %lu\n", length); */
              if (length > 0) {
                if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(columnValue, length))) {
                  raise_error(MEMORY_ERROR);
                  columnValue = NULL;
                } else {
                  columnData->buffer = columnValue->mem;
                  columnData->buffer_length = (unsigned long) length;
                  if (unlikely(mysql_stmt_fetch_column(preparedStmt->ppStmt,
                                                       preparedStmt->result_array,
                                                       (unsigned int) column - 1,
                                                       0) != 0)) {
                    setDbErrorMsg("sqlColumnBStri", "mysql_stmt_fetch_column",
                                  mysql_stmt_errno(preparedStmt->ppStmt),
                                  mysql_stmt_error(preparedStmt->ppStmt));
                    logError(printf("sqlColumnBStri: mysql_stmt_fetch_column error: %s\n",
                                    mysql_stmt_error(preparedStmt->ppStmt)););
                    columnData->buffer = NULL;
                    columnData->buffer_length = 0;
                    FREE_BSTRI(columnValue, length);
                    raise_error(DATABASE_ERROR);
                    columnValue = NULL;
                  } else {
                    columnValue->size = length;
                    /* Restore the state that no buffer is provided.  */
                    /* This way mysql_stmt_fetch() will not fetch     */
                    /* data. Instead mysql_stmt_fetch() returns       */
                    /* MYSQL_DATA_TRUNCATED, which is ignored by      */
                    /* sqlFetch().                                    */
                    columnData->buffer = NULL;
                    columnData->buffer_length = 0;
                  } /* if */
                } /* if */
              } else {
                if (unlikely(!ALLOC_BSTRI_SIZE_OK(columnValue, 0))) {
                  raise_error(MEMORY_ERROR);
                } else {
                  columnValue->size = 0;
                } /* if */
              } /* if */
            } /* if */
            break;
          default:
            logError(printf("sqlColumnBStri: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            columnData->buffer_type)););
            raise_error(RANGE_ERROR);
            columnValue = NULL;
            break;
        } /* switch */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnBStri --> \"%s\"\n", bstriAsUnquotedCStri(columnValue)););
    return columnValue;
  } /* sqlColumnBStri */



static void sqlColumnDuration (sqlStmtType sqlStatement, intType column,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second)

  {
    preparedStmtType preparedStmt;
    MYSQL_BIND *columnData;
    MYSQL_TIME *timeValue;

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
      if (columnData->is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: P0D\n"); */
        *year         = 0;
        *month        = 0;
        *day          = 0;
        *hour         = 0;
        *minute       = 0;
        *second       = 0;
        *micro_second = 0;
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(columnData->buffer_type)); */
        switch (columnData->buffer_type) {
          case MYSQL_TYPE_TIME:
          case MYSQL_TYPE_DATE:
          case MYSQL_TYPE_DATETIME:
          case MYSQL_TYPE_TIMESTAMP:
            timeValue = (MYSQL_TIME *) columnData->buffer;
            if (timeValue == NULL) {
              *year         = 0;
              *month        = 0;
              *day          = 0;
              *hour         = 0;
              *minute       = 0;
              *second       = 0;
              *micro_second = 0;
            } else {
              *hour         = timeValue->hour;
              *minute       = timeValue->minute;
              *second       = timeValue->second;
              *micro_second = (intType) timeValue->second_part;
              if (timeValue->time_type == MYSQL_TIMESTAMP_DATE ||
                  timeValue->time_type == MYSQL_TIMESTAMP_DATETIME) {
                *year      = timeValue->year;
                *month     = timeValue->month;
                *day       = timeValue->day;
              } else {
                *year      = 0;
                *month     = 0;
                *day       = 0;
              } /* if */
              if (timeValue->neg) {
                *year         = -*year;
                *month        = -*month;
                *day          = -*day;
                *hour         = -*hour;
                *minute       = -*minute;
                *second       = -*second;
                *micro_second = -*micro_second;
              } /* if */
            } /* if */
            break;
          default:
            logError(printf("sqlColumnDuration: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            columnData->buffer_type)););
            raise_error(RANGE_ERROR);
            break;
        } /* switch */
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
    MYSQL_BIND *columnData;
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
      if (columnData->is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: 0.0\n"); */
        columnValue = 0.0;
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(columnData->buffer_type)); */
        switch (columnData->buffer_type) {
          case MYSQL_TYPE_TINY:
            columnValue = (floatType) *(int8Type *) columnData->buffer;
            break;
          case MYSQL_TYPE_SHORT:
            columnValue = (floatType) *(int16Type *) columnData->buffer;
            break;
          case MYSQL_TYPE_INT24:
          case MYSQL_TYPE_LONG:
            columnValue = (floatType) *(int32Type *) columnData->buffer;
            break;
          case MYSQL_TYPE_LONGLONG:
            columnValue = (floatType) *(int64Type *) columnData->buffer;
            break;
          case MYSQL_TYPE_FLOAT:
            columnValue = *(float *) columnData->buffer;
            if (columnValue == MOST_POSITIVE_FLOAT) {
              /* The IEEE 754 value infinity is stored as the most positive value. */
              columnValue = POSITIVE_INFINITY;
            } else if (columnValue == MOST_NEGATIVE_FLOAT) {
              /* The IEEE 754 value -infinity is stored as the most negative value. */
              columnValue = NEGATIVE_INFINITY;
            } /* if */
            break;
          case MYSQL_TYPE_DOUBLE:
            columnValue = *(double *) columnData->buffer;
            if (columnValue == MOST_POSITIVE_DOUBLE) {
              /* The IEEE 754 value infinity is stored as the most positive value. */
              columnValue = POSITIVE_INFINITY;
            } else if (columnValue == MOST_NEGATIVE_DOUBLE) {
              /* The IEEE 754 value -infinity is stored as the most negative value. */
              columnValue = NEGATIVE_INFINITY;
            } /* if */
            break;
          case MYSQL_TYPE_DECIMAL:
          case MYSQL_TYPE_NEWDECIMAL:
            columnValue = getDecimalFloat(
                (const_ustriType) columnData->buffer,
                columnData->length_value);
            break;
          default:
            logError(printf("sqlColumnFloat: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            columnData->buffer_type)););
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
    MYSQL_BIND *columnData;
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
      if (columnData->is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        columnValue = 0;
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(columnData->buffer_type)); */
        switch (columnData->buffer_type) {
          case MYSQL_TYPE_TINY:
            columnValue = *(int8Type *) columnData->buffer;
            break;
          case MYSQL_TYPE_SHORT:
            columnValue = *(int16Type *) columnData->buffer;
            break;
          case MYSQL_TYPE_INT24:
          case MYSQL_TYPE_LONG:
            columnValue = *(int32Type *) columnData->buffer;
            break;
          case MYSQL_TYPE_LONGLONG:
            columnValue = *(int64Type *) columnData->buffer;
            break;
          case MYSQL_TYPE_DECIMAL:
          case MYSQL_TYPE_NEWDECIMAL:
            columnValue = getDecimalInt(
                (const_ustriType) columnData->buffer,
                columnData->length_value);
            break;
          default:
            logError(printf("sqlColumnInt: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            columnData->buffer_type)););
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
    MYSQL_BIND *columnData;
    cstriType utf8_stri;
    memSizeType length;
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
      if (columnData->is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: \"\"\n"); */
        columnValue = strEmpty();
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(columnData->buffer_type)); */
        switch (columnData->buffer_type) {
          case MYSQL_TYPE_VAR_STRING:
          case MYSQL_TYPE_STRING:
            utf8_stri = (cstriType) columnData->buffer;
            length = columnData->length_value;
            if (utf8_stri == NULL) {
              columnValue = strEmpty();
            } else {
              columnValue = cstri8_buf_to_stri(utf8_stri, length, &err_info);
              if (unlikely(columnValue == NULL)) {
                raise_error(err_info);
              } /* if */
            } /* if */
            break;
          case MYSQL_TYPE_TINY_BLOB:
          case MYSQL_TYPE_BLOB:
          case MYSQL_TYPE_MEDIUM_BLOB:
          case MYSQL_TYPE_LONG_BLOB:
            length = columnData->length_value;
            /* printf("length: %lu\n", length); */
            if (length > 0) {
              if (preparedStmt->result_data_array[column - 1].binary) {
                if (unlikely(!ALLOC_STRI_CHECK_SIZE(columnValue, length))) {
                  err_info = MEMORY_ERROR;
                } else {
                  columnData->buffer = columnValue->mem;
                  columnData->buffer_length = (unsigned long) length;
                  if (unlikely(mysql_stmt_fetch_column(preparedStmt->ppStmt,
                                                       preparedStmt->result_array,
                                                       (unsigned int) column - 1,
                                                       0) != 0)) {
                    setDbErrorMsg("sqlColumnStri", "mysql_stmt_fetch_column",
                                  mysql_stmt_errno(preparedStmt->ppStmt),
                                  mysql_stmt_error(preparedStmt->ppStmt));
                    logError(printf("sqlColumnStri: mysql_stmt_fetch_column error: %s\n",
                                    mysql_stmt_error(preparedStmt->ppStmt)););
                    FREE_STRI(columnValue, length);
                    err_info = DATABASE_ERROR;
                  } else {
                    columnValue->size = length;
                    memcpy_to_strelem(columnValue->mem,
                        (ustriType) columnValue->mem, length);
                  } /* if */
                } /* if */
              } else {
                if (unlikely(!ALLOC_BYTES(utf8_stri, length))) {
                  err_info = MEMORY_ERROR;
                  columnValue = NULL;
                } else {
                  columnData->buffer = utf8_stri;
                  columnData->buffer_length = (unsigned long) length;
                  if (unlikely(mysql_stmt_fetch_column(preparedStmt->ppStmt,
                                                       preparedStmt->result_array,
                                                       (unsigned int) column - 1,
                                                       0) != 0)) {
                    setDbErrorMsg("sqlColumnStri", "mysql_stmt_fetch_column",
                                  mysql_stmt_errno(preparedStmt->ppStmt),
                                  mysql_stmt_error(preparedStmt->ppStmt));
                    logError(printf("sqlColumnStri: mysql_stmt_fetch_column error: %s\n",
                                    mysql_stmt_error(preparedStmt->ppStmt)););
                    FREE_BYTES(utf8_stri, length);
                    err_info = DATABASE_ERROR;
                    columnValue = NULL;
                  } else {
                    columnValue = cstri8_buf_to_stri(utf8_stri, length, &err_info);
                    FREE_BYTES(utf8_stri, length);
                  } /* if */
                } /* if */
              } /* if */
              /* Restore the state that no buffer is provided.  */
              /* This way mysql_stmt_fetch() will not fetch     */
              /* data. Instead mysql_stmt_fetch() returns       */
              /* MYSQL_DATA_TRUNCATED, which is ignored by      */
              /* sqlFetch().                                    */
              columnData->buffer = NULL;
              columnData->buffer_length = 0;
              if (unlikely(err_info != OKAY_NO_ERROR)) {
                raise_error(err_info);
                columnValue = NULL;
              } /* if */
            } else {
              columnValue = strEmpty();
            } /* if */
            break;
          default:
            logError(printf("sqlColumnStri: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            columnData->buffer_type)););
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
    MYSQL_BIND *columnData;
    MYSQL_TIME *timeValue;

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
      if (columnData->is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: 0-01-01 00:00:00\n"); */
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
        /* printf("buffer_type: %s\n",
           nameOfBufferType(columnData->buffer_type)); */
        switch (columnData->buffer_type) {
          case MYSQL_TYPE_TIME:
          case MYSQL_TYPE_DATE:
          case MYSQL_TYPE_DATETIME:
          case MYSQL_TYPE_TIMESTAMP:
            timeValue = (MYSQL_TIME *) columnData->buffer;
            if (timeValue == NULL) {
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
              /* printf("timeValue->time_type: %d\n", timeValue->time_type); */
              *hour         = timeValue->hour;
              *minute       = timeValue->minute;
              *second       = timeValue->second;
              *micro_second = (intType) timeValue->second_part;
              if ((columnData->buffer_type != MYSQL_TYPE_TIME ||
                  timeValue->time_type == MYSQL_TIMESTAMP_DATE ||
                  timeValue->time_type == MYSQL_TIMESTAMP_DATETIME) &&
                  timeValue->month != 0 && timeValue->day != 0) {
                /* For 00:00:00 buffer_type is always MYSQL_TYPE_TIME. But */
                /* for this time time_type might be MYSQL_TIMESTAMP_DATE.  */
                /* In this case month and day are both zero. The condition */
                /* above corrects this bug.                                */
                *year  = timeValue->year;
                *month = timeValue->month;
                *day   = timeValue->day;
                timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                              time_zone, is_dst);
              } else {
                *year  = 2000;
                *month = 1;
                *day   = 1;
                /* It actually happens that hour is outside */
                /* of the allowed range from 0 to 23.       */
                if (*hour >= 24) {
                  *hour = *hour % 24;
                } else if (*hour < 0) {
                  *hour = *hour % 24;
                  if (*hour != 0) {
                    *hour += 24;
                  } /* if */
                } /* if */
                timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                              time_zone, is_dst);
                *year      = 0;
              } /* if */
            } /* if */
            break;
          default:
            logError(printf("sqlColumnTime: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            columnData->buffer_type)););
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

  /* sqlCommit */
    logFunction(printf("sqlCommit(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (unlikely(mysql_commit(db->connection) != 0)) {
      setDbErrorMsg("sqlCommit", "mysql_commit",
                    mysql_errno(db->connection),
                    mysql_error(db->connection));
      logError(printf("sqlCommit: mysql_commit error: %s\n",
                      mysql_error(db->connection)););
      raise_error(DATABASE_ERROR);
    } /* if */
    logFunction(printf("sqlCommit -->\n"););
  } /* sqlCommit */



static void sqlExecute (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;

  /* sqlExecute */
    logFunction(printf("sqlExecute(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!allParametersBound(preparedStmt))) {
      dbLibError("sqlExecute", "SQLExecute",
                 "Unbound statement parameter(s).\n");
      raise_error(DATABASE_ERROR);
    } else {
      /* printf("ppStmt: " FMT_U_MEM "\n", (memSizeType) preparedStmt->ppStmt); */
      preparedStmt->fetchOkay = FALSE;
      if (unlikely(mysql_stmt_bind_param(preparedStmt->ppStmt,
                                         preparedStmt->param_array) != 0)) {
        setDbErrorMsg("sqlExecute", "mysql_stmt_bind_param",
                      mysql_stmt_errno(preparedStmt->ppStmt),
                      mysql_stmt_error(preparedStmt->ppStmt));
        logError(printf("sqlExecute: mysql_stmt_bind_param error: %s\n",
                        mysql_stmt_error(preparedStmt->ppStmt)););
        preparedStmt->executeSuccessful = FALSE;
        raise_error(DATABASE_ERROR);
      } else if (unlikely(mysql_stmt_execute(preparedStmt->ppStmt) != 0)) {
        setDbErrorMsg("sqlExecute", "mysql_stmt_execute",
                      mysql_stmt_errno(preparedStmt->ppStmt),
                      mysql_stmt_error(preparedStmt->ppStmt));
        logError(printf("sqlExecute: mysql_stmt_execute error: %s\n",
                        mysql_stmt_error(preparedStmt->ppStmt)););
        preparedStmt->executeSuccessful = FALSE;
        raise_error(DATABASE_ERROR);
      } else if (unlikely(preparedStmt->result_array_size != 0 &&
                          mysql_stmt_bind_result(preparedStmt->ppStmt,
                                                 preparedStmt->result_array) != 0)) {
        setDbErrorMsg("sqlExecute", "mysql_stmt_bind_result",
                      mysql_stmt_errno(preparedStmt->ppStmt),
                      mysql_stmt_error(preparedStmt->ppStmt));
        logError(printf("sqlExecute: mysql_stmt_bind_result error: %s\n",
                        mysql_stmt_error(preparedStmt->ppStmt)););
        preparedStmt->executeSuccessful = FALSE;
        raise_error(DATABASE_ERROR);
      } else if (unlikely(mysql_stmt_store_result(preparedStmt->ppStmt) != 0)) {
        setDbErrorMsg("sqlExecute", "mysql_stmt_store_result",
                      mysql_stmt_errno(preparedStmt->ppStmt),
                      mysql_stmt_error(preparedStmt->ppStmt));
        logError(printf("sqlExecute: mysql_stmt_store_result error: %s\n",
                        mysql_stmt_error(preparedStmt->ppStmt)););
        preparedStmt->executeSuccessful = FALSE;
        raise_error(DATABASE_ERROR);
      } else {
        preparedStmt->executeSuccessful = TRUE;
        preparedStmt->fetchFinished = FALSE;
      } /* if */
    } /* if */
    logFunction(printf("sqlExecute -->\n"););
  } /* sqlExecute */



static boolType sqlFetch (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    int fetch_result;

  /* sqlFetch */
    logFunction(printf("sqlFetch(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->executeSuccessful)) {
      dbLibError("sqlFetch", "mysql_stmt_execute",
                 "Execute was not successful.\n");
      logError(printf("sqlFetch: Execute was not successful.\n"););
      preparedStmt->fetchOkay = FALSE;
      raise_error(DATABASE_ERROR);
    } else if (preparedStmt->result_array_size == 0) {
      preparedStmt->fetchOkay = FALSE;
    } else if (!preparedStmt->fetchFinished) {
      /* printf("ppStmt: " FMT_U_MEM "\n", (memSizeType) preparedStmt->ppStmt); */
      fetch_result = mysql_stmt_fetch(preparedStmt->ppStmt);
      if (fetch_result == 0) {
        preparedStmt->fetchOkay = TRUE;
      } else if (fetch_result == MYSQL_DATA_TRUNCATED) {
        /* For BLOBs buffer == NULL and buffer_length == 0 holds.    */
        /* Therefore BLOBs are truncated (MYSQL_DATA_TRUNCATED).     */
        /* Instead BLOBs are fetched with mysql_stmt_fetch_column(). */
        preparedStmt->fetchOkay = TRUE;
      } else if (fetch_result == MYSQL_NO_DATA) {
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->fetchFinished = TRUE;
      } else {
        setDbErrorMsg("sqlFetch", "mysql_stmt_fetch",
                      mysql_stmt_errno(preparedStmt->ppStmt),
                      mysql_stmt_error(preparedStmt->ppStmt));
        logError(printf("sqlFetch: mysql_stmt_fetch returns %d: %s\n",
                        fetch_result, dbError.message););
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
    /* There seems to be no function to retrieve the current         */
    /* autocommit mode. Therefore the mode is retrieved from the db. */
    autoCommit = db->autoCommit;
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
      isNull = preparedStmt->result_array[column - 1].is_null_value != 0;
    } /* if */
    logFunction(printf("sqlIsNull --> %s\n", isNull ? "TRUE" : "FALSE"););
    return isNull;
  } /* sqlIsNull */



static sqlStmtType sqlPrepare (databaseType database,
    const const_striType sqlStatementStri)

  {
    dbType db;
    striType statementStri;
    cstriType query;
    memSizeType queryLength;
    int prepare_result;
    errInfoType err_info = OKAY_NO_ERROR;
    preparedStmtType preparedStmt;

  /* sqlPrepare */
    logFunction(printf("sqlPrepare(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) database,
                       striAsUnquotedCStri(sqlStatementStri)););
    db = (dbType) database;
    if (db->connection == NULL) {
      logError(printf("sqlPrepare: Database is not open.\n"););
      err_info = RANGE_ERROR;
      preparedStmt = NULL;
    } else {
      statementStri = processStatementStri(sqlStatementStri, db->backslashEscapes);
      if (unlikely(statementStri == NULL)) {
        err_info = MEMORY_ERROR;
        preparedStmt = NULL;
      } else {
        query = stri_to_cstri8_buf(statementStri, &queryLength);
        if (unlikely(query == NULL)) {
          err_info = MEMORY_ERROR;
          preparedStmt = NULL;
        } else {
          if (unlikely(queryLength > ULONG_MAX)) {
            /* It is not possible to cast queryLength to unsigned long. */
            logError(printf("sqlPrepare: Statement string too long (length = " FMT_U_MEM ")\n",
                            queryLength););
            err_info = RANGE_ERROR;
            preparedStmt = NULL;
          } else if (unlikely(!ALLOC_RECORD2(preparedStmt, preparedStmtRecordMy,
                                             count.prepared_stmt, count.prepared_stmt_bytes))) {
            err_info = MEMORY_ERROR;
          } else {
            memset(preparedStmt, 0, sizeof(preparedStmtRecordMy));
            preparedStmt->ppStmt = mysql_stmt_init(db->connection);
            if (preparedStmt->ppStmt == NULL) {
              setDbErrorMsg("sqlPrepare", "mysql_stmt_init",
                            mysql_errno(db->connection),
                            mysql_error(db->connection));
              logError(printf("sqlPrepare: mysql_stmt_init error: %s\n",
                              mysql_error(db->connection)););
              FREE_RECORD2(preparedStmt, preparedStmtRecordMy,
                           count.prepared_stmt, count.prepared_stmt_bytes);
              err_info = DATABASE_ERROR;
              preparedStmt = NULL;
            } else {
              prepare_result = mysql_stmt_prepare(preparedStmt->ppStmt,
                                                  query,
                                                  (unsigned long) queryLength);
              if (prepare_result != 0) {
                setDbErrorMsg("sqlPrepare", "mysql_stmt_prepare",
                              mysql_stmt_errno(preparedStmt->ppStmt),
                              mysql_stmt_error(preparedStmt->ppStmt));
                logError(printf("sqlPrepare: mysql_stmt_prepare error: %s\n",
                                mysql_stmt_error(preparedStmt->ppStmt)););
                mysql_stmt_close(preparedStmt->ppStmt);
                FREE_RECORD2(preparedStmt, preparedStmtRecordMy,
                             count.prepared_stmt, count.prepared_stmt_bytes);
                err_info = DATABASE_ERROR;
                preparedStmt = NULL;
              } else {
                preparedStmt->usage_count = 1;
                preparedStmt->sqlFunc = db->sqlFunc;
                preparedStmt->executeSuccessful = FALSE;
                preparedStmt->fetchOkay = FALSE;
                preparedStmt->fetchFinished = TRUE;
                err_info = setupParameters(preparedStmt);
                if (unlikely(err_info != OKAY_NO_ERROR)) {
                  preparedStmt->result_array = NULL;
                } else {
                  err_info = setupResult(preparedStmt);
                } /* if */
                if (unlikely(err_info != OKAY_NO_ERROR)) {
                  freePreparedStmt((sqlStmtType) preparedStmt);
                  preparedStmt = NULL;
                } /* if */
              } /* if */
            } /* if */
          } /* if */
          free_cstri8(query, statementStri);
        } /* if */
        FREE_STRI(statementStri, sqlStatementStri->size * 2);
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
    if (unlikely(mysql_rollback(db->connection) != 0)) {
      setDbErrorMsg("sqlRollback", "mysql_rollback",
                    mysql_errno(db->connection),
                    mysql_error(db->connection));
      logError(printf("sqlRollback: mysql_rollback error: %s\n",
                      mysql_error(db->connection)););
      raise_error(DATABASE_ERROR);
    } /* if */
    logFunction(printf("sqlRollback -->\n"););
  } /* sqlRollback */



static void sqlSetAutoCommit (databaseType database, boolType autoCommit)

  {
    dbType db;

  /* sqlSetAutoCommit */
    logFunction(printf("sqlSetAutoCommit(" FMT_U_MEM ", %d)\n",
                       (memSizeType) database, autoCommit););
    db = (dbType) database;
    if (unlikely(mysql_autocommit(db->connection, autoCommit) != 0)) {
      setDbErrorMsg("sqlSetAutoCommit", "mysql_autocommit",
                    mysql_errno(db->connection),
                    mysql_error(db->connection));
      logError(printf("sqlSetAutoCommit: mysql_autocommit: %s\n",
                      mysql_error(db->connection)););
      raise_error(DATABASE_ERROR);
    } else {
      /* There seems to be no function to retrieve the current    */
      /* autocommit mode. Therefore the mode is stored in the db. */
      db->autoCommit = autoCommit;
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
                      "Result_array_size does not fit into an integer.\n"););
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
    const_cstriType col_name;
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
      raise_error(RANGE_ERROR);
      name = NULL;
    } else {
      col_name = preparedStmt->result_data_array[column - 1].name;
      if (unlikely(col_name == NULL)) {
        dbInconsistent("sqlStmtColumnName", "mysql_fetch_field_direct");
        logError(printf("sqlStmtColumnName: Column " FMT_D ": "
                        "Column name is NULL.\n", column););
        raise_error(DATABASE_ERROR);
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



#if 0
static rtlTypeType sqlStmtColumnType (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    const_cstriType col_name;
    errInfoType err_info = OKAY_NO_ERROR;
    rtlTypeType columnType;

  /* sqlStmtColumnType */
    logFunction(printf("sqlStmtColumnType(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlStmtColumnType: column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      column, preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnType = SYS_VOID_TYPE;
    } else {
      columnData = &preparedStmt->result_array[column - 1];
      switch (columnData->buffer_type) {
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_INT24:
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_LONGLONG:
          columnType = SYS_INT_TYPE;
          break;
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
          columnType = SYS_FLOAT_TYPE;
          break;
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
          columnType = SYS_BIGINT_TYPE;
          break;
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_NEWDATE:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP:
          columnType = SYS_VOID_TYPE;
          break;
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VAR_STRING:
          columnType = SYS_STRI_TYPE;
          break;
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
          columnType = SYS_BSTRI_TYPE;
          break;
        case MYSQL_TYPE_BIT:
        case MYSQL_TYPE_YEAR:
        case MYSQL_TYPE_SET:
        case MYSQL_TYPE_ENUM:
        case MYSQL_TYPE_GEOMETRY:
        case MYSQL_TYPE_NULL:
          columnType = SYS_VOID_TYPE;
          break;
        default:
          logError(printf("sqlStmtColumnType: Column " FMT_D " has the unknown type %s.\n",
                          column, nameOfBufferType(
                          columnData->buffer_type)););
          raise_error(RANGE_ERROR);
          columnType = SYS_VOID_TYPE;
          break;
      } /* switch */
    } /* if */
    return columnType;
  } /* sqlStmtColumnType */
#endif



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



static void determineIfBackslashEscapes (dbType database)

  {
    striType statementStri;
    sqlStmtType preparedStmt;
    striType data;

  /* determineIfBackslashEscapes */
    database->backslashEscapes = FALSE;
    statementStri = cstri_to_stri("SELECT '\\\\'");
    if (likely(statementStri != NULL)) {
      preparedStmt = sqlPrepare((databaseType) database, statementStri);
      if (likely(preparedStmt != NULL)) {
        sqlExecute(preparedStmt);
        if (likely(sqlFetch(preparedStmt))) {
          data = sqlColumnStri(preparedStmt, 1);
          if (data->size == 1 && data->mem[0] == '\\') {
            /* A select for two backslashes returns just one backslash. */
            /* This happens if the database uses backslash as escape char. */
            database->backslashEscapes = TRUE;
          } /* if */
          FREE_STRI(data, data->size);
        } /* if */
        freePreparedStmt(preparedStmt);
      } /* if */
      FREE_STRI(statementStri, statementStri->size);
    } /* if */
    /* printf("backslashEscapes: %d\n", database->backslashEscapes); */
  } /* determineIfBackslashEscapes */



databaseType sqlOpenMy (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  {
    const_cstriType host8;
    const_cstriType dbName8;
    const_cstriType user8;
    const_cstriType password8;
    MYSQL *connection;
    MYSQL *connect_result;
    errInfoType err_info = OKAY_NO_ERROR;
    dbType database;

  /* sqlOpenMy */
    logFunction(printf("sqlOpenMy(\"%s\", ",
                       striAsUnquotedCStri(host));
                printf(FMT_D ", \"%s\", ",
                       port, striAsUnquotedCStri(dbName));
                printf("\"%s\", ", striAsUnquotedCStri(user));
                printf("\"%s\")\n", striAsUnquotedCStri(password)););
    if (!findDll()) {
      logError(printf("sqlOpenMy: findDll() failed\n"););
      err_info = DATABASE_ERROR;
      database = NULL;
    } else if (unlikely(port < 0 || port > UINT_MAX)) {
      err_info = RANGE_ERROR;
      database = NULL;
    } else if (unlikely((host8 = stri_to_cstri8(host, &err_info)) == NULL)) {
      database = NULL;
    } else {
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
            connection = mysql_init(NULL);
            if (unlikely(connection == NULL)) {
              err_info = MEMORY_ERROR;
              database = NULL;
            } else {
              if (mysql_options(connection, MYSQL_SET_CHARSET_NAME, "utf8") != 0) {
                logError(printf("sqlOpenMy: Unable to set charset to UTF-8.\n"););
                err_info = RANGE_ERROR;
                mysql_close(connection);
                database = NULL;
              } else {
                /* A host of NULL means "localhost" and a port of 0 means DEFAULT_PORT. */
                connect_result = mysql_real_connect(connection, host8[0] == '\0' ? NULL : host8,
                                                    user8, password8, dbName8,
                                                    (unsigned int) port, NULL, 0);
                if (connect_result == NULL) {
                  setDbErrorMsg("sqlOpenMy", "mysql_real_connect",
                                mysql_errno(connection),
                                mysql_error(connection));
                  logError(printf("sqlOpenMy: mysql_real_connect(conn, "
                                  "\"%s\", \"%s\", \"%s\", \"%s\", " FMT_D ") error:\n%s\n",
                                  host8[0] == '\0' ? "NULL" : host8,
                                  user8, password8, dbName8, port,
                                  mysql_error(connection)););
                  err_info = DATABASE_ERROR;
                  mysql_close(connection);
                  database = NULL;
                } else if (mysql_set_character_set(connection, "utf8") != 0) {
                  setDbErrorMsg("sqlOpenMy", "mysql_set_character_set",
                                mysql_errno(connection),
                                mysql_error(connection));
                  logError(printf("sqlOpenMy: mysql_set_character_set error: %s\n",
                                  mysql_error(connection)););
                  err_info = DATABASE_ERROR;
                  mysql_close(connection);
                  database = NULL;
                } else if (unlikely(!setupFuncTable() ||
                                    !ALLOC_RECORD2(database, dbRecordMy,
                                                  count.database, count.database_bytes))) {
                  err_info = MEMORY_ERROR;
                  mysql_close(connection);
                  database = NULL;
                } else {
                  memset(database, 0, sizeof(dbRecordMy));
                  database->usage_count = 1;
                  database->sqlFunc = sqlFunc;
                  database->driver = DB_CATEGORY_MYSQL;
                  database->connection = connection;
                  database->autoCommit = TRUE;
                  determineIfBackslashEscapes(database);
                } /* if */
              } /* if */
            } /* if */
            free_cstri8(password8, password);
          } /* if */
          free_cstri8(user8, user);
        } /* if */
        free_cstri8(dbName8, dbName);
      } /* if */
      free_cstri8(host8, host);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("sqlOpenMy --> " FMT_U_MEM "\n",
                       (memSizeType) database););
    return (databaseType) database;
  } /* sqlOpenMy */

#else



databaseType sqlOpenMy (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  { /* sqlOpenMy */
    logError(printf("sqlOpenMy(\"%s\", ",
                    striAsUnquotedCStri(host));
             printf(FMT_D ", \"%s\", ",
                    port, striAsUnquotedCStri(dbName));
             printf("\"%s\", ", striAsUnquotedCStri(user));
             printf("\"%s\"): MariaDB/MySQL driver not present.\n",
                    striAsUnquotedCStri(password)););
    raise_error(RANGE_ERROR);
    return NULL;
  } /* sqlOpenMy */

#endif
