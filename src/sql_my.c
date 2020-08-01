/********************************************************************/
/*                                                                  */
/*  sql_my.c      Database access functions for MariaDB and MySQL.  */
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
/*  File: seed7/src/sql_my.c                                        */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: Database access functions for MariaDB and MySQL.       */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#ifdef MYSQL_INCLUDE
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "limits.h"
#include MYSQL_INCLUDE

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


typedef struct {
    uintType     usage_count;
    sqlFuncType  sqlFunc;
    MYSQL       *connection;
  } dbRecord, *dbType;

typedef struct {
    memSizeType   buffer_capacity;
  } bindDataRecord, *bindDataType;

typedef struct {
    char         *name;
  } resultDataRecord, *resultDataType;

typedef struct {
    uintType       usage_count;
    sqlFuncType    sqlFunc;
    MYSQL_STMT    *ppStmt;
    memSizeType    param_array_capacity;
    memSizeType    param_array_size;
    MYSQL_BIND    *param_array;
    bindDataType   param_data_array;
    memSizeType    result_array_size;
    MYSQL_BIND    *result_array;
    resultDataType result_data_array;
    boolType       executeSuccessful;
    boolType       fetchOkay;
    boolType       fetchFinished;
  } preparedStmtRecord, *preparedStmtType;

static sqlFuncType sqlFunc = NULL;

#define MAX_DECIMAL_PRECISION 65
#define MAX_DECIMAL_SCALE 30
/* Maximum length needs additionally 1 digit for sign and 1 for decimal point. */
#define MAX_DECIMAL_LENGTH (MAX_DECIMAL_PRECISION + 2)
#define MOST_POSITIVE_FLOAT   3.4028234663852885e+38
#define MOST_NEGATIVE_FLOAT  -3.4028234663852885e+38
#define MOST_POSITIVE_DOUBLE  1.7976931348623157e+308
#define MOST_NEGATIVE_DOUBLE -1.7976931348623157e+308


#ifdef MYSQL_DLL

#ifndef STDCALL
#if defined(_WIN32)
#define STDCALL __stdcall
#else
#define STDCALL
#endif
#endif

typedef void (STDCALL *tp_mysql_close) (MYSQL *sock);
typedef my_bool (STDCALL *tp_mysql_commit) (MYSQL *mysql);
typedef unsigned int (STDCALL *tp_mysql_errno) (MYSQL *mysql);
typedef const char *(STDCALL *tp_mysql_error) (MYSQL *mysql);
typedef MYSQL_FIELD *(STDCALL *tp_mysql_fetch_field_direct) (MYSQL_RES *res, unsigned int fieldnr);
typedef void (STDCALL *tp_mysql_free_result) (MYSQL_RES *result);
typedef MYSQL *(STDCALL *tp_mysql_init) (MYSQL *mysql);
typedef unsigned int (STDCALL *tp_mysql_num_fields) (MYSQL_RES *res);
typedef int (STDCALL *tp_mysql_options) (MYSQL *mysql,enum mysql_option option, const void *arg);
typedef MYSQL *(STDCALL *tp_mysql_real_connect) (MYSQL *mysql, const char *host,
                                                 const char *user,
                                                 const char *passwd,
                                                 const char *db,
                                                 unsigned int port,
                                                 const char *unix_socket,
                                                 unsigned long clientflag);
typedef int (STDCALL *tp_mysql_set_character_set) (MYSQL *mysql, const char *csname);
typedef my_bool (STDCALL *tp_mysql_stmt_bind_param) (MYSQL_STMT *stmt, MYSQL_BIND *bnd);
typedef my_bool (STDCALL *tp_mysql_stmt_bind_result) (MYSQL_STMT *stmt, MYSQL_BIND *bnd);
typedef my_bool (STDCALL *tp_mysql_stmt_close) (MYSQL_STMT *stmt);
typedef unsigned int (STDCALL *tp_mysql_stmt_errno) (MYSQL_STMT *stmt);
typedef const char *(STDCALL *tp_mysql_stmt_error) (MYSQL_STMT *stmt);
typedef int (STDCALL *tp_mysql_stmt_execute) (MYSQL_STMT *stmt);
typedef int (STDCALL *tp_mysql_stmt_fetch) (MYSQL_STMT *stmt);
typedef MYSQL_STMT *(STDCALL *tp_mysql_stmt_init) (MYSQL *mysql);
typedef int (STDCALL *tp_mysql_stmt_prepare) (MYSQL_STMT *stmt, const char *query, unsigned long length);
typedef MYSQL_RES *(STDCALL *tp_mysql_stmt_result_metadata) (MYSQL_STMT *stmt);

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
static tp_mysql_set_character_set    ptr_mysql_set_character_set;
static tp_mysql_stmt_bind_param      ptr_mysql_stmt_bind_param;
static tp_mysql_stmt_bind_result     ptr_mysql_stmt_bind_result;
static tp_mysql_stmt_close           ptr_mysql_stmt_close;
static tp_mysql_stmt_errno           ptr_mysql_stmt_errno;
static tp_mysql_stmt_error           ptr_mysql_stmt_error;
static tp_mysql_stmt_execute         ptr_mysql_stmt_execute;
static tp_mysql_stmt_fetch           ptr_mysql_stmt_fetch;
static tp_mysql_stmt_init            ptr_mysql_stmt_init;
static tp_mysql_stmt_prepare         ptr_mysql_stmt_prepare;
static tp_mysql_stmt_result_metadata ptr_mysql_stmt_result_metadata;

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
#define mysql_set_character_set    ptr_mysql_set_character_set
#define mysql_stmt_bind_param      ptr_mysql_stmt_bind_param
#define mysql_stmt_bind_result     ptr_mysql_stmt_bind_result
#define mysql_stmt_close           ptr_mysql_stmt_close
#define mysql_stmt_errno           ptr_mysql_stmt_errno
#define mysql_stmt_error           ptr_mysql_stmt_error
#define mysql_stmt_execute         ptr_mysql_stmt_execute
#define mysql_stmt_fetch           ptr_mysql_stmt_fetch
#define mysql_stmt_init            ptr_mysql_stmt_init
#define mysql_stmt_prepare         ptr_mysql_stmt_prepare
#define mysql_stmt_result_metadata ptr_mysql_stmt_result_metadata



static boolType setupDll (const char *dllName)

  {
    static void *dbDll = NULL;

  /* setupDll */
    logFunction(printf("setupDll(\"%s\")\n", dllName););
    if (dbDll == NULL) {
      dbDll = dllOpen(dllName);
      if (dbDll != NULL) {
        if ((mysql_close                = (tp_mysql_close)                dllFunc(dbDll, "mysql_close"))                == NULL ||
            (mysql_commit               = (tp_mysql_commit)               dllFunc(dbDll, "mysql_commit"))               == NULL ||
            (mysql_errno                = (tp_mysql_errno)                dllFunc(dbDll, "mysql_errno"))                == NULL ||
            (mysql_error                = (tp_mysql_error)                dllFunc(dbDll, "mysql_error"))                == NULL ||
            (mysql_fetch_field_direct   = (tp_mysql_fetch_field_direct)   dllFunc(dbDll, "mysql_fetch_field_direct"))   == NULL ||
            (mysql_free_result          = (tp_mysql_free_result)          dllFunc(dbDll, "mysql_free_result"))          == NULL ||
            (mysql_init                 = (tp_mysql_init)                 dllFunc(dbDll, "mysql_init"))                 == NULL ||
            (mysql_num_fields           = (tp_mysql_num_fields)           dllFunc(dbDll, "mysql_num_fields"))           == NULL ||
            (mysql_options              = (tp_mysql_options)              dllFunc(dbDll, "mysql_options"))              == NULL ||
            (mysql_real_connect         = (tp_mysql_real_connect)         dllFunc(dbDll, "mysql_real_connect"))         == NULL ||
            (mysql_set_character_set    = (tp_mysql_set_character_set)    dllFunc(dbDll, "mysql_set_character_set"))    == NULL ||
            (mysql_stmt_bind_param      = (tp_mysql_stmt_bind_param)      dllFunc(dbDll, "mysql_stmt_bind_param"))      == NULL ||
            (mysql_stmt_bind_result     = (tp_mysql_stmt_bind_result)     dllFunc(dbDll, "mysql_stmt_bind_result"))     == NULL ||
            (mysql_stmt_close           = (tp_mysql_stmt_close)           dllFunc(dbDll, "mysql_stmt_close"))           == NULL ||
            (mysql_stmt_errno           = (tp_mysql_stmt_errno)           dllFunc(dbDll, "mysql_stmt_errno"))           == NULL ||
            (mysql_stmt_error           = (tp_mysql_stmt_error)           dllFunc(dbDll, "mysql_stmt_error"))           == NULL ||
            (mysql_stmt_execute         = (tp_mysql_stmt_execute)         dllFunc(dbDll, "mysql_stmt_execute"))         == NULL ||
            (mysql_stmt_fetch           = (tp_mysql_stmt_fetch)           dllFunc(dbDll, "mysql_stmt_fetch"))           == NULL ||
            (mysql_stmt_init            = (tp_mysql_stmt_init)            dllFunc(dbDll, "mysql_stmt_init"))            == NULL ||
            (mysql_stmt_prepare         = (tp_mysql_stmt_prepare)         dllFunc(dbDll, "mysql_stmt_prepare"))         == NULL ||
            (mysql_stmt_result_metadata = (tp_mysql_stmt_result_metadata) dllFunc(dbDll, "mysql_stmt_result_metadata")) == NULL) {
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
      logError(printf("findDll: Searched for:\n");
               for (pos = 0; pos < sizeof(dllList) / sizeof(char *); pos++) {
                 printf("%s\n", dllList[pos]);
               });
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



static void freeDatabase (databaseType database)

  {
    dbType db;

  /* freeDatabase */
    logFunction(printf("freeDatabase(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    sqlClose(database);
    db = (dbType) database;
    FREE_RECORD(db, dbRecord, count.database);
    logFunction(printf("freeDatabase -->\n"););
  } /* freeDatabase */



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
      FREE_TABLE(preparedStmt->param_array, MYSQL_BIND, preparedStmt->param_array_capacity);
    } /* if */
    if (preparedStmt->result_array != NULL) {
      for (pos = 0; pos < preparedStmt->result_array_size; pos++) {
        free(preparedStmt->result_array[pos].buffer);
      } /* for */
      FREE_TABLE(preparedStmt->result_array, MYSQL_BIND, preparedStmt->result_array_size);
    } /* if */
    if (preparedStmt->result_data_array != NULL) {
      FREE_TABLE(preparedStmt->result_data_array, resultDataRecord, preparedStmt->result_array_size);
    } /* if */
    mysql_stmt_close(preparedStmt->ppStmt);
    FREE_RECORD(preparedStmt, preparedStmtRecord, count.prepared_stmt);
    logFunction(printf("freePreparedStmt -->\n"););
  } /* freePreparedStmt */



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



static void setupResultColumn (preparedStmtType preparedStmt,
    unsigned int column_num, MYSQL_RES *result_metadata,
    MYSQL_BIND *resultData, errInfoType *err_info)

  {
    MYSQL_FIELD *column;
    memSizeType column_size;

  /* setupResultColumn */
    column = mysql_fetch_field_direct(result_metadata, column_num - 1);
    /* printf("column[%u]->type: %s\n", column_num, nameOfBufferType(column->type)); */
    switch (column->type) {
      case MYSQL_TYPE_TINY:
        column_size = 1;
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_SHORT:
        column_size = 2;
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_INT24:
      case MYSQL_TYPE_LONG:
        column_size = 4;
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_LONGLONG:
        /* printf("MYSQL_LONGLONG: column->length = %lu\n", column->length);
        printf("MYSQL_LONGLONG: column->decimals = %d\n", column->decimals); */
        column_size = 8;
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_FLOAT:
        /* printf("MYSQL_TYPE_FLOAT: column->length = %lu\n", column->length);
        printf("MYSQL_TYPE_FLOAT: column->decimals = %d\n", column->decimals); */
        column_size = 4;
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_DOUBLE:
        /* printf("MYSQL_TYPE_DOUBLE: column->length = %lu\n", column->length);
        printf("MYSQL_TYPE_DOUBLE: column->decimals = %d\n", column->decimals); */
        column_size = 8;
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_DECIMAL:
      case MYSQL_TYPE_NEWDECIMAL:
        /* printf("MYSQL_TYPE_NEWDECIMAL: column->length = %lu\n", column->length);
        printf("MYSQL_TYPE_NEWDECIMAL: column->decimals = %d\n", column->decimals); */
        column_size = column->length;
        resultData->length = &resultData->length_value;
        break;
      case MYSQL_TYPE_TIME:
      case MYSQL_TYPE_DATE:
      case MYSQL_TYPE_DATETIME:
      case MYSQL_TYPE_TIMESTAMP:
        column_size = sizeof(MYSQL_TIME);
        resultData->length = NULL;
        break;
      case MYSQL_TYPE_STRING:
      case MYSQL_TYPE_VAR_STRING:
        column_size = column->length;
        resultData->length = &resultData->length_value;
        break;
      case MYSQL_TYPE_TINY_BLOB:
      case MYSQL_TYPE_BLOB:
      case MYSQL_TYPE_MEDIUM_BLOB:
      case MYSQL_TYPE_LONG_BLOB:
        column_size = column->length;
        resultData->length = &resultData->length_value;
        break;
      default:
        logError(printf("setupResultColumn: Column %s has the unknown type %s.\n",
                        column->name, nameOfBufferType(column->type)););
        *err_info = RANGE_ERROR;
        break;
    } /* switch */
    /* printf("column_size: " FMT_U_MEM "\n", column_size); */
    if (*err_info == OKAY_NO_ERROR) {
      resultData->buffer = malloc(column_size);
      if (resultData->buffer == NULL) {
        *err_info = MEMORY_ERROR;
      } else {
        resultData->buffer_type   = column->type;
        resultData->buffer_length = column_size;
        resultData->is_unsigned   = 0;
        resultData->is_null = &resultData->is_null_value;
        resultData->is_null_value = 0;
        preparedStmt->result_data_array[column_num - 1].name = column->name;
      } /* if */
    } /* if */
  } /* setupResultColumn */



static void setupResult (preparedStmtType preparedStmt,
    errInfoType *err_info)

  {
    MYSQL_RES *result_metadata;
    unsigned int num_columns;
    unsigned int column_index;

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
      if (!ALLOC_TABLE(preparedStmt->result_array, MYSQL_BIND, num_columns)) {
        *err_info = MEMORY_ERROR;
      } else if (!ALLOC_TABLE(preparedStmt->result_data_array, resultDataRecord, num_columns)) {
        FREE_TABLE(preparedStmt->result_array, MYSQL_BIND, num_columns);
        *err_info = MEMORY_ERROR;
      } else {
        preparedStmt->result_array_size = num_columns;
        memset(preparedStmt->result_array, 0, num_columns * sizeof(MYSQL_BIND));
        memset(preparedStmt->result_data_array, 0, num_columns * sizeof(resultDataRecord));
        for (column_index = 0; column_index < num_columns &&
             *err_info == OKAY_NO_ERROR; column_index++) {
          setupResultColumn(preparedStmt, column_index + 1, result_metadata,
                            &preparedStmt->result_array[column_index],
                            err_info);
        } /* for */
      } /* if */
      mysql_free_result(result_metadata);
    } /* if */
    logFunction(printf("setupResult -->\n"););
  } /* setupResult */



static void resizeBindArray (preparedStmtType preparedStmt, memSizeType pos)

  {
    memSizeType new_size;
    MYSQL_BIND *resized_param_array = NULL;
    bindDataType resized_data_array = NULL;

  /* resizeBindArray */
    if (pos > preparedStmt->param_array_capacity) {
      new_size = (((pos - 1) >> 3) + 1) << 3;  /* Round to next multiple of 8. */
      resized_param_array = REALLOC_TABLE(preparedStmt->param_array, MYSQL_BIND, preparedStmt->param_array_capacity, new_size);
      resized_data_array = REALLOC_TABLE(preparedStmt->param_data_array, bindDataRecord, preparedStmt->param_array_capacity, new_size);
      if (unlikely(resized_param_array == NULL || resized_data_array == NULL)) {
        if (resized_param_array != NULL) {
          FREE_TABLE(preparedStmt->param_array, MYSQL_BIND, preparedStmt->param_array_capacity);
        } /* if */
        if (resized_data_array != NULL) {
          FREE_TABLE(preparedStmt->param_data_array, bindDataRecord, preparedStmt->param_array_capacity);
        } /* if */
        preparedStmt->param_array_capacity = 0;
        preparedStmt->param_array = NULL;
        preparedStmt->param_data_array = NULL;
        raise_error(MEMORY_ERROR);
      } else {
        preparedStmt->param_array = resized_param_array;
        preparedStmt->param_data_array = resized_data_array;
        COUNT3_TABLE(MYSQL_BIND, preparedStmt->param_array_capacity, new_size);
        COUNT3_TABLE(bindDataRecord, preparedStmt->param_array_capacity, new_size);
        memset(&preparedStmt->param_array[preparedStmt->param_array_capacity], 0,
               (new_size - preparedStmt->param_array_capacity) * sizeof(MYSQL_BIND));
        memset(&preparedStmt->param_data_array[preparedStmt->param_array_capacity], 0,
               (new_size - preparedStmt->param_array_capacity) * sizeof(bindDataRecord));
        preparedStmt->param_array_capacity = new_size;
      } /* if */
    } /* if */
    if (pos > preparedStmt->param_array_size) {
      preparedStmt->param_array_size = pos;
    } /* if */
  } /* resizeBindArray */



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
    if (stri == NULL) {
      *err_info = MEMORY_ERROR;
    } else {
      /* prot_stri(stri);
         printf("\n"); */
      decimal = (unsigned char *) buffer;
      srcIndex = 0;
      destIndex = 0;
      if (stri->mem[0] == '-') {
        decimal[0] = '-';
        srcIndex++;
        destIndex++;
      } /* if */
      if (unlikely(stri->size - srcIndex > MAX_DECIMAL_PRECISION)) {
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
        if (stri == NULL) {
          *err_info = MEMORY_ERROR;
        } else if (stri->size > UINT_MAX) {
          FREE_STRI(stri, stri->size);
          *err_info = MEMORY_ERROR;
        } else {
          /* prot_stri(stri);
             printf("\n"); */
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
    unsigned int length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBigInt */
    logFunction(printf("sqlBindBigInt(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, bigHexCStri(value)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > MAX_MEM_INDEX)) {
      logError(printf("sqlBindBigInt: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, (memSizeType) MAX_MEM_INDEX););
      raise_error(RANGE_ERROR);
    } else {
      resizeBindArray(preparedStmt, (memSizeType) pos);
      if (preparedStmt->param_array != NULL) {
        if (preparedStmt->param_array[pos - 1].buffer == NULL) {
          if ((preparedStmt->param_array[pos - 1].buffer = malloc(MAX_DECIMAL_LENGTH)) == NULL) {
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->param_array[pos - 1].buffer_type = MYSQL_TYPE_NEWDECIMAL;
            preparedStmt->param_array[pos - 1].is_unsigned = 0;
            preparedStmt->param_array[pos - 1].is_null     = NULL;
            preparedStmt->param_array[pos - 1].length      = NULL;
          } /* if */
        } else if (preparedStmt->param_array[pos - 1].buffer_type != MYSQL_TYPE_NEWDECIMAL) {
          err_info = RANGE_ERROR;
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          length = setBigInt(preparedStmt->param_array[pos - 1].buffer,
                             value, &err_info);
          if (unlikely(err_info != OKAY_NO_ERROR)) {
            raise_error(err_info);
          } else {
            preparedStmt->param_array[pos - 1].buffer_length = length;
            preparedStmt->executeSuccessful = FALSE;
            preparedStmt->fetchOkay = FALSE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBigInt */



static void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType numerator, const const_bigIntType denominator)

  {
    preparedStmtType preparedStmt;
    unsigned int length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBigRat */
    logFunction(printf("sqlBindBigRat(" FMT_U_MEM ", " FMT_D ", %s, %s)\n",
                       (memSizeType) sqlStatement, pos,
                       bigHexCStri(numerator), bigHexCStri(denominator)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > MAX_MEM_INDEX)) {
      logError(printf("sqlBindBigRat: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, (memSizeType) MAX_MEM_INDEX););
      raise_error(RANGE_ERROR);
    } else {
      resizeBindArray(preparedStmt, (memSizeType) pos);
      if (preparedStmt->param_array != NULL) {
        if (preparedStmt->param_array[pos - 1].buffer == NULL) {
          if ((preparedStmt->param_array[pos - 1].buffer = malloc(MAX_DECIMAL_LENGTH)) == NULL) {
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->param_array[pos - 1].buffer_type = MYSQL_TYPE_NEWDECIMAL;
            preparedStmt->param_array[pos - 1].is_unsigned = 0;
            preparedStmt->param_array[pos - 1].is_null     = NULL;
            preparedStmt->param_array[pos - 1].length      = NULL;
          } /* if */
        } else if (preparedStmt->param_array[pos - 1].buffer_type != MYSQL_TYPE_NEWDECIMAL) {
          err_info = RANGE_ERROR;
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          length = setBigRat(preparedStmt->param_array[pos - 1].buffer,
                             numerator, denominator, &err_info);
          if (unlikely(err_info != OKAY_NO_ERROR)) {
            raise_error(err_info);
          } else {
            preparedStmt->param_array[pos - 1].buffer_length = length;
            preparedStmt->executeSuccessful = FALSE;
            preparedStmt->fetchOkay = FALSE;
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
    if (unlikely(pos < 1 || (uintType) pos > MAX_MEM_INDEX)) {
      logError(printf("sqlBindBool: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, (memSizeType) MAX_MEM_INDEX););
      raise_error(RANGE_ERROR);
    } else {
      resizeBindArray(preparedStmt, (memSizeType) pos);
      if (preparedStmt->param_array != NULL) {
        if (preparedStmt->param_array[pos - 1].buffer == NULL) {
          if ((preparedStmt->param_array[pos - 1].buffer = malloc(1)) == NULL) {
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->param_array[pos - 1].buffer_type   = MYSQL_TYPE_STRING;
            preparedStmt->param_array[pos - 1].buffer_length = 1;
            preparedStmt->param_array[pos - 1].is_unsigned   = 0;
            preparedStmt->param_array[pos - 1].is_null       = NULL;
            preparedStmt->param_array[pos - 1].length        = NULL;
          } /* if */
        } else if (preparedStmt->param_array[pos - 1].buffer_type != MYSQL_TYPE_STRING ||
                   preparedStmt->param_array[pos - 1].buffer_length != 1) {
          err_info = RANGE_ERROR;
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          ((char *) preparedStmt->param_array[pos - 1].buffer)[0] = (char) ('0' + value);
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBool */



static void sqlBindBStri (sqlStmtType sqlStatement, intType pos, bstriType bstri)

  {
    preparedStmtType preparedStmt;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBStri */
    logFunction(printf("sqlBindBStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, bstriAsUnquotedCStri(bstri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > MAX_MEM_INDEX)) {
      logError(printf("sqlBindBStri: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, (memSizeType) MAX_MEM_INDEX););
      raise_error(RANGE_ERROR);
    } else {
      resizeBindArray(preparedStmt, (memSizeType) pos);
      if (preparedStmt->param_array != NULL) {
        if (preparedStmt->param_array[pos - 1].buffer == NULL) {
          if ((preparedStmt->param_array[pos - 1].buffer = malloc(bstri->size)) == NULL) {
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->param_array[pos - 1].buffer_type   = MYSQL_TYPE_BLOB;
            preparedStmt->param_array[pos - 1].is_unsigned   = 0;
            preparedStmt->param_array[pos - 1].is_null       = NULL;
            preparedStmt->param_array[pos - 1].length        = NULL;
            preparedStmt->param_data_array[pos - 1].buffer_capacity = bstri->size;
          } /* if */
        } else if (preparedStmt->param_array[pos - 1].buffer_type != MYSQL_TYPE_BLOB) {
          err_info = RANGE_ERROR;
        } else if (bstri->size > preparedStmt->param_data_array[pos - 1].buffer_capacity) {
          free(preparedStmt->param_array[pos - 1].buffer);
          if ((preparedStmt->param_array[pos - 1].buffer = malloc(bstri->size)) == NULL) {
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->param_data_array[pos - 1].buffer_capacity = bstri->size;
          } /* if */
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else if (unlikely(bstri->size > ULONG_MAX)) {
          free(preparedStmt->param_array[pos - 1].buffer);
          preparedStmt->param_array[pos - 1].buffer = NULL;
          raise_error(MEMORY_ERROR);
        } else {
          memcpy(preparedStmt->param_array[pos - 1].buffer, bstri->mem, bstri->size);
          preparedStmt->param_array[pos - 1].buffer_length = (unsigned long) bstri->size;
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBStri */



static void sqlBindDuration (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  {
    preparedStmtType preparedStmt;
    enum enum_field_types buffer_type;
    MYSQL_TIME *timeValue;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindDuration */
    logFunction(printf("sqlBindDuration(" FMT_U_MEM ", " FMT_D ", "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) ")\n",
                       (memSizeType) sqlStatement, pos,
                       year, month, day,
                       hour, minute, second, micro_second););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > MAX_MEM_INDEX)) {
      logError(printf("sqlBindDuration: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, (memSizeType) MAX_MEM_INDEX););
      raise_error(RANGE_ERROR);
    } else if (unlikely(year < -INT_MAX || year > INT_MAX || month < -12 || month > 12 ||
                        day < -31 || day > 31 || hour < -24 || hour > 24 ||
                        minute < -60 || minute > 60 || second < -60 || second > 60 ||
                        micro_second < -1000000 || micro_second > 1000000)) {
      logError(printf("sqlBindDuration: Duration not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else if (unlikely(!((year >= 0 && month >= 0 && day >= 0 && hour >= 0 &&
                           minute >= 0 && second >= 0 && micro_second >= 0) ||
                          (year <= 0 && month <= 0 && day <= 0 && hour <= 0 &&
                           minute <= 0 && second <= 0 && micro_second <= 0)))) {
      logError(printf("sqlBindDuration: Duration neither clearly positive nor negative.\n"););
      raise_error(RANGE_ERROR);
    } else {
      buffer_type = MYSQL_TYPE_DATETIME;
      resizeBindArray(preparedStmt, (memSizeType) pos);
      if (preparedStmt->param_array != NULL) {
        if (preparedStmt->param_array[pos - 1].buffer == NULL) {
          if ((preparedStmt->param_array[pos - 1].buffer = malloc(sizeof(MYSQL_TIME))) == NULL) {
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->param_array[pos - 1].buffer_type = buffer_type;
            preparedStmt->param_array[pos - 1].is_unsigned = 0;
            preparedStmt->param_array[pos - 1].is_null     = NULL;
            preparedStmt->param_array[pos - 1].length      = NULL;
          } /* if */
        } else if (preparedStmt->param_array[pos - 1].buffer_type != buffer_type) {
          err_info = RANGE_ERROR;
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          timeValue = (MYSQL_TIME *) preparedStmt->param_array[pos - 1].buffer;
          timeValue->year   = (unsigned int) abs((int) year);
          timeValue->month  = (unsigned int) abs((int) month);
          timeValue->day    = (unsigned int) abs((int) day);
          timeValue->hour   = (unsigned int) abs((int) hour);
          timeValue->minute = (unsigned int) abs((int) minute);
          timeValue->second = (unsigned int) abs((int) second);
          timeValue->neg    = year < 0 || month < 0 || day < 0 ||
                              hour < 0 || minute < 0 || second < 0;
          timeValue->time_type = MYSQL_TIMESTAMP_DATETIME;
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindDuration */



static void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value)

  {
    preparedStmtType preparedStmt;
#if FLOATTYPE_SIZE == 32
    const enum enum_field_types buffer_type = MYSQL_TYPE_FLOAT;
#elif FLOATTYPE_SIZE == 64
    const enum enum_field_types buffer_type = MYSQL_TYPE_DOUBLE;
#endif
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindFloat */
    logFunction(printf("sqlBindFloat(" FMT_U_MEM ", " FMT_D ", " FMT_E ")\n",
                       (memSizeType) sqlStatement, pos, value););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > MAX_MEM_INDEX)) {
      logError(printf("sqlBindFloat: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, (memSizeType) MAX_MEM_INDEX););
      raise_error(RANGE_ERROR);
    } else {
      resizeBindArray(preparedStmt, (memSizeType) pos);
      if (preparedStmt->param_array != NULL) {
        if (preparedStmt->param_array[pos - 1].buffer == NULL) {
          if ((preparedStmt->param_array[pos - 1].buffer = malloc(sizeof(floatType))) == NULL) {
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->param_array[pos - 1].buffer_type = buffer_type;
            preparedStmt->param_array[pos - 1].is_unsigned = 0;
            preparedStmt->param_array[pos - 1].is_null     = NULL;
            preparedStmt->param_array[pos - 1].length      = NULL;
          } /* if */
        } else if (preparedStmt->param_array[pos - 1].buffer_type != buffer_type) {
          err_info = RANGE_ERROR;
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          *(floatType *) preparedStmt->param_array[pos - 1].buffer = value;
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindFloat */



static void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value)

  {
    preparedStmtType preparedStmt;
#if INTTYPE_SIZE == 32
    const enum enum_field_types buffer_type = MYSQL_TYPE_LONG;
#elif INTTYPE_SIZE == 64
    const enum enum_field_types buffer_type = MYSQL_TYPE_LONGLONG;
#endif
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindInt */
    logFunction(printf("sqlBindInt(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos, value););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > MAX_MEM_INDEX)) {
      logError(printf("sqlBindInt: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, (memSizeType) MAX_MEM_INDEX););
      raise_error(RANGE_ERROR);
    } else {
      resizeBindArray(preparedStmt, (memSizeType) pos);
      if (preparedStmt->param_array != NULL) {
        if (preparedStmt->param_array[pos - 1].buffer == NULL) {
          if ((preparedStmt->param_array[pos - 1].buffer = malloc(sizeof(intType))) == NULL) {
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->param_array[pos - 1].buffer_type = buffer_type;
            preparedStmt->param_array[pos - 1].is_unsigned = 0;
            preparedStmt->param_array[pos - 1].is_null     = NULL;
            preparedStmt->param_array[pos - 1].length      = NULL;
          } /* if */
        } else if (preparedStmt->param_array[pos - 1].buffer_type != buffer_type) {
          err_info = RANGE_ERROR;
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          *(intType *) preparedStmt->param_array[pos - 1].buffer = value;
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindInt */



static void sqlBindNull (sqlStmtType sqlStatement, intType pos)

  {
    preparedStmtType preparedStmt;

  /* sqlBindNull */
    logFunction(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > MAX_MEM_INDEX)) {
      logError(printf("sqlBindNull: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, (memSizeType) MAX_MEM_INDEX););
      raise_error(RANGE_ERROR);
    } else {
      resizeBindArray(preparedStmt, (memSizeType) pos);
      if (preparedStmt->param_array != NULL) {
        preparedStmt->param_array[pos - 1].is_null =
            &preparedStmt->param_array[pos - 1].is_null_value;
        preparedStmt->param_array[pos - 1].is_null_value = 1;
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
      } /* if */
    } /* if */
  } /* sqlBindNull */



static void sqlBindStri (sqlStmtType sqlStatement, intType pos, striType stri)

  {
    preparedStmtType preparedStmt;
    cstriType stri8;
    memSizeType length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindStri */
    logFunction(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, striAsUnquotedCStri(stri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > MAX_MEM_INDEX)) {
      logError(printf("sqlBindStri: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, (memSizeType) MAX_MEM_INDEX););
      raise_error(RANGE_ERROR);
    } else {
      resizeBindArray(preparedStmt, (memSizeType) pos);
      if (preparedStmt->param_array != NULL) {
        if (preparedStmt->param_array[pos - 1].buffer == NULL) {
          stri8 = stri_to_cstri8_buf(stri, &length, &err_info);
          if (stri8 != NULL) {
            preparedStmt->param_array[pos - 1].buffer_type   = MYSQL_TYPE_STRING;
            preparedStmt->param_array[pos - 1].is_unsigned   = 0;
            preparedStmt->param_array[pos - 1].is_null       = NULL;
            preparedStmt->param_array[pos - 1].length        = NULL;
          } /* if */
        } else if (preparedStmt->param_array[pos - 1].buffer_type != MYSQL_TYPE_STRING) {
          err_info = RANGE_ERROR;
        } else {
          stri8 = stri_to_cstri8_buf(stri, &length, &err_info);
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else if (unlikely(length > ULONG_MAX)) {
          free(stri8);
          raise_error(MEMORY_ERROR);
        } else {
          free(preparedStmt->param_array[pos - 1].buffer);
          preparedStmt->param_array[pos - 1].buffer        = stri8;
          preparedStmt->param_array[pos - 1].buffer_length = (unsigned long) length;
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindStri */



static void sqlBindTime (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  {
    preparedStmtType preparedStmt;
    enum enum_field_types buffer_type;
    MYSQL_TIME *timeValue;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindTime */
    logFunction(printf("sqlBindTime(" FMT_U_MEM ", " FMT_D ", "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) ")\n",
                       (memSizeType) sqlStatement, pos,
                       year, month, day,
                       hour, minute, second, micro_second););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > MAX_MEM_INDEX)) {
      logError(printf("sqlBindTime: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, (memSizeType) MAX_MEM_INDEX););
      raise_error(RANGE_ERROR);
    } else if (unlikely(year < -INT_MAX || year > INT_MAX || month < 1 || month > 12 ||
                        day < 1 || day > 31 || hour < 0 || hour > 24 ||
                        minute < 0 || minute > 60 || second < 0 || second > 60 ||
                        micro_second < 0 || micro_second > 1000000)) {
      logError(printf("sqlBindTime: Time not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      buffer_type = MYSQL_TYPE_DATETIME;
      resizeBindArray(preparedStmt, (memSizeType) pos);
      if (preparedStmt->param_array != NULL) {
        if (preparedStmt->param_array[pos - 1].buffer == NULL) {
          if ((preparedStmt->param_array[pos - 1].buffer = malloc(sizeof(MYSQL_TIME))) == NULL) {
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->param_array[pos - 1].buffer_type = buffer_type;
            preparedStmt->param_array[pos - 1].is_unsigned = 0;
            preparedStmt->param_array[pos - 1].is_null     = NULL;
            preparedStmt->param_array[pos - 1].length      = NULL;
          } /* if */
        } else if (preparedStmt->param_array[pos - 1].buffer_type != buffer_type) {
          err_info = RANGE_ERROR;
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          timeValue = (MYSQL_TIME *) preparedStmt->param_array[pos - 1].buffer;
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
          timeValue->time_type = MYSQL_TIMESTAMP_DATETIME;
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
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
      mysql_close(db->connection);
      db->connection = NULL;
    } /* if */
    logFunction(printf("sqlClose -->\n"););
  } /* sqlClose */



static bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
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
      if (preparedStmt->result_array[column - 1].is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        columnValue = bigZero();
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(preparedStmt->result_array[column - 1].buffer_type)); */
        switch (preparedStmt->result_array[column - 1].buffer_type) {
          case MYSQL_TYPE_TINY:
            columnValue = bigFromInt32((int32Type)
                *(int8Type *) preparedStmt->result_array[column - 1].buffer);
            break;
          case MYSQL_TYPE_SHORT:
            columnValue = bigFromInt32((int32Type)
                *(int16Type *) preparedStmt->result_array[column - 1].buffer);
            break;
          case MYSQL_TYPE_INT24:
          case MYSQL_TYPE_LONG:
            columnValue = bigFromInt32(
                *(int32Type *) preparedStmt->result_array[column - 1].buffer);
            break;
          case MYSQL_TYPE_LONGLONG:
            columnValue = bigFromInt64(
                *(int64Type *) preparedStmt->result_array[column - 1].buffer);
            break;
          case MYSQL_TYPE_DECIMAL:
          case MYSQL_TYPE_NEWDECIMAL:
            columnValue = getDecimalBigInt(
                (const_ustriType) preparedStmt->result_array[column - 1].buffer,
                preparedStmt->result_array[column - 1].length_value);
            break;
          default:
            logError(printf("sqlColumnBigInt: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            preparedStmt->result_array[column - 1].buffer_type)););
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
      if (preparedStmt->result_array[column - 1].is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        *numerator = bigZero();
        *denominator = bigFromInt32(1);
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(preparedStmt->result_array[column - 1].buffer_type)); */
        switch (preparedStmt->result_array[column - 1].buffer_type) {
          case MYSQL_TYPE_TINY:
            *numerator = bigFromInt32((int32Type)
                *(int8Type *) preparedStmt->result_array[column - 1].buffer);
            *denominator = bigFromInt32(1);
            break;
          case MYSQL_TYPE_SHORT:
            *numerator = bigFromInt32((int32Type)
                *(int16Type *) preparedStmt->result_array[column - 1].buffer);
            *denominator = bigFromInt32(1);
            break;
          case MYSQL_TYPE_INT24:
          case MYSQL_TYPE_LONG:
            *numerator = bigFromInt32(
                *(int32Type *) preparedStmt->result_array[column - 1].buffer);
            *denominator = bigFromInt32(1);
            break;
          case MYSQL_TYPE_LONGLONG:
            *numerator = bigFromInt64(
                *(int64Type *) preparedStmt->result_array[column - 1].buffer);
            *denominator = bigFromInt32(1);
            break;
          case MYSQL_TYPE_FLOAT:
            floatValue = *(float *) preparedStmt->result_array[column - 1].buffer;
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
            doubleValue = *(double *) preparedStmt->result_array[column - 1].buffer;
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
                (const_ustriType) preparedStmt->result_array[column - 1].buffer,
                preparedStmt->result_array[column - 1].length_value,
                denominator);
            break;
          default:
            logError(printf("sqlColumnBigRat: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            preparedStmt->result_array[column - 1].buffer_type)););
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
      if (preparedStmt->result_array[column - 1].is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: FALSE\n"); */
        columnValue = 0;
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(preparedStmt->result_array[column - 1].buffer_type)); */
        switch (preparedStmt->result_array[column - 1].buffer_type) {
          case MYSQL_TYPE_TINY:
            columnValue = *(int8Type *) preparedStmt->result_array[column - 1].buffer;
            break;
          case MYSQL_TYPE_SHORT:
            columnValue = *(int16Type *) preparedStmt->result_array[column - 1].buffer;
            break;
          case MYSQL_TYPE_INT24:
          case MYSQL_TYPE_LONG:
            columnValue = *(int32Type *) preparedStmt->result_array[column - 1].buffer;
            break;
          case MYSQL_TYPE_LONGLONG:
            columnValue = *(int64Type *) preparedStmt->result_array[column - 1].buffer;
            break;
          case MYSQL_TYPE_STRING:
          case MYSQL_TYPE_VAR_STRING:
            if (unlikely(preparedStmt->result_array[column - 1].length_value != 1)) {
              logError(printf("sqlColumnBool: Column " FMT_D ": "
                              "The size of a boolean field must be 1.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = 0;
            } else {
              columnValue = *(const_cstriType) preparedStmt->result_array[column - 1].buffer - '0';
            } /* if */
            break;
          default:
            logError(printf("sqlColumnBool: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            preparedStmt->result_array[column - 1].buffer_type)););
            raise_error(RANGE_ERROR);
            columnValue = 0;
            break;
        } /* switch */
        if (unlikely((uint64Type) columnValue >= 2)) {
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
    const_ustriType blob;
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
      if (preparedStmt->result_array[column - 1].is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: \"\"\n"); */
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(columnValue, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          columnValue->size = 0;
        } /* if */
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(preparedStmt->result_array[column - 1].buffer_type)); */
        switch (preparedStmt->result_array[column - 1].buffer_type) {
          case MYSQL_TYPE_TINY_BLOB:
          case MYSQL_TYPE_BLOB:
          case MYSQL_TYPE_MEDIUM_BLOB:
          case MYSQL_TYPE_LONG_BLOB:
            blob = (const_ustriType) preparedStmt->result_array[column - 1].buffer;
            length = preparedStmt->result_array[column - 1].length_value;
            if (blob == NULL) {
              if (unlikely(!ALLOC_BSTRI_SIZE_OK(columnValue, 0))) {
                raise_error(MEMORY_ERROR);
              } else {
                columnValue->size = 0;
              } /* if */
            } else {
              if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(columnValue, length))) {
                raise_error(MEMORY_ERROR);
              } else {
                columnValue->size = length;
                memcpy(columnValue->mem, blob, length);
              } /* if */
            } /* if */
            break;
          default:
            logError(printf("sqlColumnBStri: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            preparedStmt->result_array[column - 1].buffer_type)););
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
      if (preparedStmt->result_array[column - 1].is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: 0-00-00 00:00:00\n"); */
        *year         = 0;
        *month        = 0;
        *day          = 0;
        *hour         = 0;
        *minute       = 0;
        *second       = 0;
        *micro_second = 0;
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(preparedStmt->result_array[column - 1].buffer_type)); */
        switch (preparedStmt->result_array[column - 1].buffer_type) {
          case MYSQL_TYPE_TIME:
          case MYSQL_TYPE_DATE:
          case MYSQL_TYPE_DATETIME:
          case MYSQL_TYPE_TIMESTAMP:
            timeValue = (MYSQL_TIME *) preparedStmt->result_array[column - 1].buffer;
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
              *micro_second = 0;
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
            } /* if */
            break;
          default:
            logError(printf("sqlColumnDuration: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            preparedStmt->result_array[column - 1].buffer_type)););
            raise_error(RANGE_ERROR);
            break;
        } /* switch */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", "
                                            F_D(04) "-" F_D(02) "-" F_D(02) " "
                                            F_D(02) ":" F_D(02) ":" F_D(02) "."
                                            F_D(06) ") -->\n",
                       (memSizeType) sqlStatement, column,
                       *year, *month, *day, *hour, *minute, *second,
                       *micro_second););
  } /* sqlColumnDuration */



static floatType sqlColumnFloat (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
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
      if (preparedStmt->result_array[column - 1].is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: 0.0\n"); */
        columnValue = 0.0;
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(preparedStmt->result_array[column - 1].buffer_type)); */
        switch (preparedStmt->result_array[column - 1].buffer_type) {
          case MYSQL_TYPE_FLOAT:
            columnValue = *(float *) preparedStmt->result_array[column - 1].buffer;
            if (columnValue == MOST_POSITIVE_FLOAT) {
              /* The IEEE 754 value infinity is stored as the most positive value. */
              columnValue = POSITIVE_INFINITY;
            } else if (columnValue == MOST_NEGATIVE_FLOAT) {
              /* The IEEE 754 value -infinity is stored as the most negative value. */
              columnValue = NEGATIVE_INFINITY;
            } /* if */
            break;
          case MYSQL_TYPE_DOUBLE:
            columnValue = *(double *) preparedStmt->result_array[column - 1].buffer;
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
                (const_ustriType) preparedStmt->result_array[column - 1].buffer,
                preparedStmt->result_array[column - 1].length_value);
            break;
          default:
            logError(printf("sqlColumnFloat: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            preparedStmt->result_array[column - 1].buffer_type)););
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
      if (preparedStmt->result_array[column - 1].is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        columnValue = 0;
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(preparedStmt->result_array[column - 1].buffer_type)); */
        switch (preparedStmt->result_array[column - 1].buffer_type) {
          case MYSQL_TYPE_TINY:
            columnValue = *(int8Type *) preparedStmt->result_array[column - 1].buffer;
            break;
          case MYSQL_TYPE_SHORT:
            columnValue = *(int16Type *) preparedStmt->result_array[column - 1].buffer;
            break;
          case MYSQL_TYPE_INT24:
          case MYSQL_TYPE_LONG:
            columnValue = *(int32Type *) preparedStmt->result_array[column - 1].buffer;
            break;
          case MYSQL_TYPE_LONGLONG:
            columnValue = *(int64Type *) preparedStmt->result_array[column - 1].buffer;
            break;
          case MYSQL_TYPE_DECIMAL:
          case MYSQL_TYPE_NEWDECIMAL:
            columnValue = getDecimalInt(
                (const_ustriType) preparedStmt->result_array[column - 1].buffer,
                preparedStmt->result_array[column - 1].length_value);
            break;
          default:
            logError(printf("sqlColumnInt: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            preparedStmt->result_array[column - 1].buffer_type)););
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
    const_cstriType utf8_stri;
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
      if (preparedStmt->result_array[column - 1].is_null_value != 0) {
        /* printf("Column is NULL -> Use default value: \"\"\n"); */
        columnValue = strEmpty();
      } else {
        /* printf("buffer_type: %s\n",
           nameOfBufferType(preparedStmt->result_array[column - 1].buffer_type)); */
        switch (preparedStmt->result_array[column - 1].buffer_type) {
          case MYSQL_TYPE_VAR_STRING:
          case MYSQL_TYPE_STRING:
            utf8_stri = (const_cstriType) preparedStmt->result_array[column - 1].buffer;
            length = preparedStmt->result_array[column - 1].length_value;
            if (utf8_stri == NULL) {
              columnValue = strEmpty();
            } else {
              columnValue = cstri8_buf_to_stri(utf8_stri, length, &err_info);
              if (unlikely(columnValue == NULL)) {
                raise_error(err_info);
              } /* if */
            } /* if */
            break;
          default:
            logError(printf("sqlColumnStri: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            preparedStmt->result_array[column - 1].buffer_type)););
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
      if (preparedStmt->result_array[column - 1].is_null_value != 0) {
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
           nameOfBufferType(preparedStmt->result_array[column - 1].buffer_type)); */
        switch (preparedStmt->result_array[column - 1].buffer_type) {
          case MYSQL_TYPE_TIME:
          case MYSQL_TYPE_DATE:
          case MYSQL_TYPE_DATETIME:
          case MYSQL_TYPE_TIMESTAMP:
            timeValue = (MYSQL_TIME *) preparedStmt->result_array[column - 1].buffer;
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
              *hour         = timeValue->hour;
              *minute       = timeValue->minute;
              *second       = timeValue->second;
              *micro_second = 0;
              if (timeValue->time_type == MYSQL_TIMESTAMP_DATE ||
                  timeValue->time_type == MYSQL_TIMESTAMP_DATETIME) {
                *year      = timeValue->year;
                *month     = timeValue->month;
                *day       = timeValue->day;
                timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                              time_zone, is_dst);
              } else {
                *year      = 2000;
                *month     = 1;
                *day       = 1;
                timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                              time_zone, is_dst);
                *year      = 0;
              } /* if */
            } /* if */
            break;
          default:
            logError(printf("sqlColumnTime: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(
                            preparedStmt->result_array[column - 1].buffer_type)););
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
    db = (dbType) database;
    if (unlikely(mysql_commit(db->connection) != 0)) {
      setDbErrorMsg("sqlCommit", "mysql_commit",
                    mysql_errno(db->connection),
                    mysql_error(db->connection));
      raise_error(DATABASE_ERROR);
    } /* if */
  } /* sqlCommit */



static void sqlExecute (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    int bind_param_result;
    int bind_result_result;
    int execute_result;

  /* sqlExecute */
    logFunction(printf("sqlExecute(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    /* printf("ppStmt: " FMT_U_MEM "\n", (memSizeType) preparedStmt->ppStmt); */
    preparedStmt->fetchOkay = FALSE;
    bind_param_result = mysql_stmt_bind_param(preparedStmt->ppStmt, preparedStmt->param_array);
    if (unlikely(bind_param_result != 0)) {
      setDbErrorMsg("sqlExecute", "mysql_stmt_bind_param",
                    mysql_stmt_errno(preparedStmt->ppStmt),
                    mysql_stmt_error(preparedStmt->ppStmt));
      logError(printf("sqlExecute: mysql_stmt_bind_param error: %s\n",
                      mysql_stmt_error(preparedStmt->ppStmt)););
      preparedStmt->executeSuccessful = FALSE;
      raise_error(DATABASE_ERROR);
    } else {
      if (preparedStmt->result_array_size != 0) {
        bind_result_result = mysql_stmt_bind_result(preparedStmt->ppStmt, preparedStmt->result_array);
        if (unlikely(bind_result_result != 0)) {
          setDbErrorMsg("sqlExecute", "mysql_stmt_bind_result",
                        mysql_stmt_errno(preparedStmt->ppStmt),
                        mysql_stmt_error(preparedStmt->ppStmt));
          logError(printf("sqlExecute: mysql_stmt_bind_result error: %s\n",
                          mysql_stmt_error(preparedStmt->ppStmt)););
          preparedStmt->executeSuccessful = FALSE;
          raise_error(DATABASE_ERROR);
        } /* if */
      } else {
        bind_result_result = 0;
      } /* if */
      if (likely(bind_result_result == 0)) {
        execute_result = mysql_stmt_execute(preparedStmt->ppStmt);
        if (unlikely(execute_result != 0)) {
          setDbErrorMsg("sqlExecute", "mysql_stmt_execute",
                        mysql_stmt_errno(preparedStmt->ppStmt),
                        mysql_stmt_error(preparedStmt->ppStmt));
          logError(printf("sqlExecute: mysql_stmt_execute error: %s\n",
                          mysql_stmt_error(preparedStmt->ppStmt)););
          preparedStmt->executeSuccessful = FALSE;
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->executeSuccessful = TRUE;
          preparedStmt->fetchFinished = FALSE;
        } /* if */
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
      dbLibError("sqlFetch", "PQexecPrepared",
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
      } else if (fetch_result == MYSQL_NO_DATA) {
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->fetchFinished = TRUE;
      } else {
        setDbErrorMsg("sqlFetch", "mysql_stmt_fetch",
                      mysql_stmt_errno(preparedStmt->ppStmt),
                      mysql_stmt_error(preparedStmt->ppStmt));
        logError(printf("sqlFetch: mysql_stmt_fetch error: %s\n",
                        mysql_stmt_error(preparedStmt->ppStmt)););
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->fetchFinished = TRUE;
        raise_error(DATABASE_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("sqlFetch --> %d\n", preparedStmt->fetchOkay););
    return preparedStmt->fetchOkay;
  } /* sqlFetch */



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



static sqlStmtType sqlPrepare (databaseType database, striType sqlStatementStri)

  {
    dbType db;
    cstriType query;
    memSizeType query_length;
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
      query = stri_to_cstri8_buf(sqlStatementStri, &query_length, &err_info);
      if (query == NULL) {
        preparedStmt = NULL;
      } else {
        if (!ALLOC_RECORD(preparedStmt, preparedStmtRecord, count.prepared_stmt)) {
          err_info = MEMORY_ERROR;
        } else {
          memset(preparedStmt, 0, sizeof(preparedStmtRecord));
          preparedStmt->ppStmt = mysql_stmt_init(db->connection);
          if (preparedStmt->ppStmt == NULL) {
            setDbErrorMsg("sqlPrepare", "mysql_stmt_init",
                          mysql_errno(db->connection),
                          mysql_error(db->connection));
            logError(printf("sqlPrepare: mysql_stmt_init error: %s\n",
                            mysql_error(db->connection)););
            FREE_RECORD(preparedStmt, preparedStmtRecord, count.prepared_stmt);
            err_info = DATABASE_ERROR;
            preparedStmt = NULL;
          } else {
            prepare_result = mysql_stmt_prepare(preparedStmt->ppStmt, query, query_length);
            if (prepare_result != 0) {
              setDbErrorMsg("sqlPrepare", "mysql_stmt_prepare",
                            mysql_stmt_errno(preparedStmt->ppStmt),
                            mysql_stmt_error(preparedStmt->ppStmt));
              logError(printf("sqlPrepare: mysql_stmt_prepare error: %s\n",
                              mysql_stmt_error(preparedStmt->ppStmt)););
              mysql_stmt_close(preparedStmt->ppStmt);
              FREE_RECORD(preparedStmt, preparedStmtRecord, count.prepared_stmt);
              err_info = DATABASE_ERROR;
              preparedStmt = NULL;
            } else {
              preparedStmt->usage_count = 1;
              preparedStmt->sqlFunc = db->sqlFunc;
              preparedStmt->executeSuccessful = FALSE;
              preparedStmt->fetchOkay = FALSE;
              preparedStmt->fetchFinished = TRUE;
              setupResult(preparedStmt, &err_info);
              if (unlikely(err_info != OKAY_NO_ERROR)) {
                freePreparedStmt((sqlStmtType) preparedStmt);
                preparedStmt = NULL;
              } /* if */
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



static intType sqlStmtColumnCount (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    intType columnCount;

  /* sqlStmtColumnCount */
    logFunction(printf("sqlStmtColumnCount(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(preparedStmt->result_array_size > INTTYPE_MAX)) {
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
        sqlFunc->sqlIsNull          = &sqlIsNull;
        sqlFunc->sqlPrepare         = &sqlPrepare;
        sqlFunc->sqlStmtColumnCount = &sqlStmtColumnCount;
        sqlFunc->sqlStmtColumnName  = &sqlStmtColumnName;
      } /* if */
    } /* if */
    return sqlFunc != NULL;
  } /* setupFuncTable */



databaseType sqlOpenMy (const const_striType dbName,
    const const_striType user, const const_striType password)

  {
    cstriType dbName8;
    cstriType user8;
    cstriType password8;
    cstriType host;
    cstriType databaseName;
    MYSQL *connection;
    MYSQL *connect_result;
    errInfoType err_info = OKAY_NO_ERROR;
    dbType database;

  /* sqlOpenMy */
    logFunction(printf("sqlOpenMy(\"%s\", ",
                       striAsUnquotedCStri(dbName));
                printf("\"%s\", ", striAsUnquotedCStri(user));
                printf("\"%s\")\n", striAsUnquotedCStri(password)););
    if (!findDll()) {
      logError(printf("sqlOpenMy: findDll() failed\n"););
      err_info = FILE_ERROR;
      database = NULL;
    } else {
      dbName8 = stri_to_cstri8(dbName, &err_info);
      if (dbName8 == NULL) {
        err_info = MEMORY_ERROR;
        database = NULL;
      } else {
        user8 = stri_to_cstri8(user, &err_info);
        if (user8 == NULL) {
          err_info = MEMORY_ERROR;
          database = NULL;
        } else {
          password8 = stri_to_cstri8(password, &err_info);
          if (password8 == NULL) {
            err_info = MEMORY_ERROR;
            database = NULL;
          } else {
            connection = mysql_init(NULL);
            if (connection == NULL) {
              err_info = MEMORY_ERROR;
              database = NULL;
            } else {
              if (mysql_options(connection, MYSQL_SET_CHARSET_NAME, "utf8") != 0) {
                err_info = RANGE_ERROR;
                mysql_close(connection);
                database = NULL;
              } else {
                databaseName = strchr(dbName8, '/');
                if (databaseName == NULL) {
                  host = NULL;
                  databaseName = dbName8;
                } else {
                  host = dbName8;
                  *databaseName = '\0';
                  databaseName++;
                } /* if */
                connect_result = mysql_real_connect(connection, host,
                    user8, password8, databaseName, 0, NULL, 0);
                if (connect_result == NULL) {
                  setDbErrorMsg("sqlOpenMy", "mysql_real_connect",
                                mysql_errno(connection),
                                mysql_error(connection));
                  logError(printf("sqlOpenMy: mysql_real_connect(conn, "
                                  "\"%s\", \"%s\", \"%s\", \"%s\") error:\n%s\n",
                                  host != NULL ? host : "NULL", user8, password8, databaseName,
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
                                    !ALLOC_RECORD(database, dbRecord, count.database))) {
                  err_info = MEMORY_ERROR;
                  mysql_close(connection);
                  database = NULL;
                } else {
                  memset(database, 0, sizeof(dbRecord));
                  database->usage_count = 1;
                  database->sqlFunc = sqlFunc;
                  database->connection = connection;
                } /* if */
              } /* if */
            } /* if */
            free_cstri8(password8, password);
          } /* if */
          free_cstri8(user8, user);
        } /* if */
        free_cstri8(dbName8, dbName);
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("sqlOpenMy --> " FMT_U_MEM "\n",
                       (memSizeType) database););
    return (databaseType) database;
  } /* sqlOpenMy */

#endif
