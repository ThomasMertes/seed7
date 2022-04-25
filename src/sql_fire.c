/********************************************************************/
/*                                                                  */
/*  sql_fire.c    Database access functions for Firebird/InterBase. */
/*  Copyright (C) 1989 - 2019  Thomas Mertes                        */
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
/*  File: seed7/src/sql_fire.c                                      */
/*  Changes: 2018, 2019  Thomas Mertes                              */
/*  Content: Database access functions for Firebird/InterBase.      */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"
#include "time.h"
#ifdef FIRE_INCLUDE
#include FIRE_INCLUDE
#endif

#include "common.h"
#include "data_rtl.h"
#include "striutl.h"
#include "heaputl.h"
#include "numutl.h"
#include "str_rtl.h"
#include "flt_rtl.h"
#include "tim_rtl.h"
#include "cmd_rtl.h"
#include "big_drv.h"
#include "rtl_err.h"
#include "dll_drv.h"
#include "sql_base.h"
#include "sql_drv.h"

#ifdef FIRE_INCLUDE


typedef struct {
    uintType      usage_count;
    sqlFuncType   sqlFunc;
    intType       driver;
    isc_db_handle connection;
    isc_tr_handle trans_handle;
  } dbRecord, *dbType;

typedef struct {
    boolType     bound;
  } bindDataRecord, *bindDataType;

typedef struct {
    uintType        usage_count;
    sqlFuncType     sqlFunc;
    dbType          db;
    isc_stmt_handle ppStmt;
    XSQLDA         *in_sqlda;
    XSQLDA         *out_sqlda;
    bindDataType    param_array;
    int             statement_type;
    boolType        executeSuccessful;
    boolType        fetchOkay;
    boolType        fetchFinished;
  } preparedStmtRecord, *preparedStmtType;

typedef struct {
    memSizeType fileName8Length;
    cstriType fileName8;
    memSizeType user8Length;
    const_cstriType user8;
    memSizeType password8Length;
    const_cstriType password8;
} loginRecord, *loginType;

static sqlFuncType sqlFunc = NULL;

static char isc_tbp[] = {isc_tpb_version3, isc_tpb_write,
                         isc_tpb_concurrency, isc_tpb_wait};
static char type_item[] = {isc_info_sql_stmt_type};

#ifndef SQL_BOOLEAN
#define SQL_BOOLEAN   32764
#endif


#ifdef FIRE_DLL

#ifndef STDCALL
#if defined(_WIN32) && HAS_STDCALL
#define STDCALL __stdcall
#else
#define STDCALL
#endif
#endif

typedef ISC_STATUS (STDCALL *tp_isc_attach_database) (ISC_STATUS *status_vector,
                                                      short db_name_length,
                                                      char *db_name,
                                                      isc_db_handle *db_handle,
                                                      short parm_buffer_length,
                                                      char *parm_buffer);
typedef ISC_STATUS (STDCALL *tp_isc_blob_info) (ISC_STATUS *status_vector,
                                                isc_blob_handle *blob_handle,
                                                short item_list_buffer_length,
                                                char *item_list_buffer,
                                                short result_buffer_length,
                                                char *result_buffer);
typedef ISC_STATUS (STDCALL *tp_isc_close_blob) (ISC_STATUS *status_vector,
                                                 isc_blob_handle *blob_handle);
typedef ISC_STATUS (STDCALL *tp_isc_commit_transaction) (ISC_STATUS *status_vector,
                                                         isc_tr_handle *trans_handle);
typedef ISC_STATUS (STDCALL *tp_isc_create_blob2) (ISC_STATUS *status_vector,
                                                   isc_db_handle *db_handle,
                                                   isc_tr_handle *trans_handle,
                                                   isc_blob_handle *blob_handle,
                                                   ISC_QUAD *blob_id,
                                                   short bpb_length,
                                                   char *bpb_address);
typedef ISC_STATUS (STDCALL *tp_isc_create_database) (ISC_STATUS *status_vector,
                                                      short db_name_length,
                                                      char *db_name,
                                                      isc_db_handle *db_handle,
                                                      short parm_buffer_length,
                                                      char *parm_buffer,
                                                      short db_type);
typedef void (STDCALL *tp_isc_decode_sql_date) (ISC_DATE *ib_date,
                                                void *tm_date);
typedef void (STDCALL *tp_isc_decode_sql_time) (ISC_TIME *ib_time,
                                                void *tm_date);
typedef void (STDCALL *tp_isc_decode_timestamp) (ISC_TIMESTAMP *ib_date,
                                                 void *tm_date);
typedef ISC_STATUS (STDCALL *tp_isc_detach_database) (ISC_STATUS *status_vector,
                                                      isc_db_handle *db_handle);
typedef ISC_STATUS (STDCALL *tp_isc_dsql_allocate_statement) (ISC_STATUS *status_vector,
                                                              isc_db_handle *db_handle,
                                                              isc_stmt_handle *stmt_handle);
typedef ISC_STATUS (STDCALL *tp_isc_dsql_describe) (ISC_STATUS *status_vector,
                                                    isc_stmt_handle *stmt_handle,
                                                    unsigned short da_version,
                                                    XSQLDA *xsqlda);
typedef ISC_STATUS (STDCALL *tp_isc_dsql_describe_bind) (ISC_STATUS *status_vector,
                                                         isc_stmt_handle *stmt_handle,
                                                         unsigned short da_version,
                                                         XSQLDA *xsqlda);
typedef ISC_STATUS (STDCALL *tp_isc_dsql_execute2) (ISC_STATUS *status_vector,
                                                    isc_tr_handle *trans_handle,
                                                    isc_stmt_handle *stmt_handle,
                                                    unsigned short da_version,
                                                    XSQLDA *in_xsqlda,
                                                    XSQLDA *out_xsqlda);
typedef ISC_STATUS (STDCALL *tp_isc_dsql_fetch) (ISC_STATUS *status_vector,
                                                 isc_stmt_handle *stmt_handle,
                                                 unsigned short da_version,
                                                 XSQLDA *xsqlda);
typedef ISC_STATUS (STDCALL *tp_isc_dsql_free_statement) (ISC_STATUS *status_vector,
                                                          isc_stmt_handle *stmt_handle,
                                                          unsigned short option);
typedef ISC_STATUS (STDCALL *tp_isc_dsql_prepare) (ISC_STATUS *status_vector,
                                                   isc_tr_handle *trans_handle,
                                                   isc_stmt_handle *stmt_handle,
                                                   unsigned short length,
                                                   char *statement,
                                                   unsigned short dialect,
                                                   XSQLDA *xsqlda);
typedef ISC_STATUS (STDCALL *tp_isc_dsql_sql_info) (ISC_STATUS *status_vector,
                                                    isc_stmt_handle *stmt_handle,
                                                    unsigned short item_length,
                                                    char *items,
                                                    unsigned short buffer_length,
                                                    char *buffer);
typedef void (STDCALL *tp_isc_encode_sql_date) (void *tm_date,
                                                ISC_DATE *ib_date);
typedef void (STDCALL *tp_isc_encode_sql_time) (void *tm_date,
                                                ISC_TIME *ib_time);
typedef void (STDCALL *tp_isc_encode_timestamp) (void *tm_date,
                                                 ISC_TIMESTAMP *ib_timestamp);
typedef ISC_STATUS (STDCALL *tp_isc_get_segment) (ISC_STATUS *status_vector,
                                                  isc_blob_handle *blob_handle,
                                                  unsigned short *actual_seg_length,
                                                  unsigned short seg_buffer_length,
                                                  char *seg_buffer);
typedef ISC_STATUS (STDCALL *tp_isc_interprete) (char *buffer, ISC_STATUS **status_vector);
typedef ISC_STATUS (STDCALL *tp_isc_open_blob2) (ISC_STATUS *status_vector,
                                                 isc_db_handle *db_handle,
                                                 isc_tr_handle *trans_handle,
                                                 isc_blob_handle *blob_handle,
                                                 ISC_QUAD *blob_id,
                                                 short bpb_length,
                                                 char *bpb_address);
typedef ISC_INT64 (STDCALL *tp_isc_portable_integer) (char *buffer,
                                                      short length);
typedef ISC_STATUS (STDCALL *tp_isc_print_status) (ISC_STATUS *status_vector);
typedef ISC_STATUS (STDCALL *tp_isc_put_segment) (ISC_STATUS *status_vector,
                                                  isc_blob_handle *blob_handle,
                                                  unsigned short seg_buffer_length,
                                                  char *seg_buffer);
typedef ISC_STATUS (STDCALL *tp_isc_rollback_transaction) (ISC_STATUS *status_vector,
                                                           isc_tr_handle *trans_handle);
typedef ISC_STATUS (STDCALL *tp_isc_start_transaction) (ISC_STATUS *status_vector,
                                                        isc_tr_handle *trans_handle,
                                                        short db_handle_count,
                                                        isc_db_handle *db_handle,
                                                        unsigned short tpb_length,
                                                        char *tpb_address);

static tp_isc_attach_database         ptr_isc_attach_database;
static tp_isc_blob_info               ptr_isc_blob_info;
static tp_isc_close_blob              ptr_isc_close_blob;
static tp_isc_commit_transaction      ptr_isc_commit_transaction;
static tp_isc_create_blob2            ptr_isc_create_blob2;
static tp_isc_create_database         ptr_isc_create_database;
static tp_isc_decode_sql_date         ptr_isc_decode_sql_date;
static tp_isc_decode_sql_time         ptr_isc_decode_sql_time;
static tp_isc_decode_timestamp        ptr_isc_decode_timestamp;
static tp_isc_detach_database         ptr_isc_detach_database;
static tp_isc_dsql_allocate_statement ptr_isc_dsql_allocate_statement;
static tp_isc_dsql_describe           ptr_isc_dsql_describe;
static tp_isc_dsql_describe_bind      ptr_isc_dsql_describe_bind;
static tp_isc_dsql_execute2           ptr_isc_dsql_execute2;
static tp_isc_dsql_fetch              ptr_isc_dsql_fetch;
static tp_isc_dsql_free_statement     ptr_isc_dsql_free_statement;
static tp_isc_dsql_prepare            ptr_isc_dsql_prepare;
static tp_isc_dsql_sql_info           ptr_isc_dsql_sql_info;
static tp_isc_encode_sql_date         ptr_isc_encode_sql_date;
static tp_isc_encode_sql_time         ptr_isc_encode_sql_time;
static tp_isc_encode_timestamp        ptr_isc_encode_timestamp;
static tp_isc_get_segment             ptr_isc_get_segment;
static tp_isc_interprete              ptr_isc_interprete;
static tp_isc_open_blob2              ptr_isc_open_blob2;
static tp_isc_portable_integer        ptr_isc_portable_integer;
static tp_isc_print_status            ptr_isc_print_status;
static tp_isc_put_segment             ptr_isc_put_segment;
static tp_isc_rollback_transaction    ptr_isc_rollback_transaction;
static tp_isc_start_transaction       ptr_isc_start_transaction;

#define isc_attach_database         ptr_isc_attach_database
#define isc_blob_info               ptr_isc_blob_info
#define isc_close_blob              ptr_isc_close_blob
#define isc_commit_transaction      ptr_isc_commit_transaction
#define isc_create_blob2            ptr_isc_create_blob2
#define isc_create_database         ptr_isc_create_database
#define isc_decode_sql_date         ptr_isc_decode_sql_date
#define isc_decode_sql_time         ptr_isc_decode_sql_time
#define isc_decode_timestamp        ptr_isc_decode_timestamp
#define isc_detach_database         ptr_isc_detach_database
#define isc_dsql_allocate_statement ptr_isc_dsql_allocate_statement
#define isc_dsql_describe           ptr_isc_dsql_describe
#define isc_dsql_describe_bind      ptr_isc_dsql_describe_bind
#define isc_dsql_execute2           ptr_isc_dsql_execute2
#define isc_dsql_fetch              ptr_isc_dsql_fetch
#define isc_dsql_free_statement     ptr_isc_dsql_free_statement
#define isc_dsql_prepare            ptr_isc_dsql_prepare
#define isc_dsql_sql_info           ptr_isc_dsql_sql_info
#define isc_encode_sql_date         ptr_isc_encode_sql_date
#define isc_encode_sql_time         ptr_isc_encode_sql_time
#define isc_encode_timestamp        ptr_isc_encode_timestamp
#define isc_get_segment             ptr_isc_get_segment
#define isc_interprete              ptr_isc_interprete
#define isc_open_blob2              ptr_isc_open_blob2
#define isc_portable_integer        ptr_isc_portable_integer
#define isc_print_status            ptr_isc_print_status
#define isc_put_segment             ptr_isc_put_segment
#define isc_rollback_transaction    ptr_isc_rollback_transaction
#define isc_start_transaction       ptr_isc_start_transaction



static boolType setupDll (const char *dllName)

  {
    static void *dbDll = NULL;

  /* setupDll */
    logFunction(printf("setupDll(\"%s\")\n", dllName););
    if (dbDll == NULL) {
      dbDll = dllOpen(dllName);
      if (dbDll != NULL) {
        if ((isc_attach_database         = (tp_isc_attach_database)         dllFunc(dbDll, "isc_attach_database"))         == NULL ||
            (isc_blob_info               = (tp_isc_blob_info)               dllFunc(dbDll, "isc_blob_info"))               == NULL ||
            (isc_close_blob              = (tp_isc_close_blob)              dllFunc(dbDll, "isc_close_blob"))              == NULL ||
            (isc_commit_transaction      = (tp_isc_commit_transaction)      dllFunc(dbDll, "isc_commit_transaction"))      == NULL ||
            (isc_create_blob2            = (tp_isc_create_blob2)            dllFunc(dbDll, "isc_create_blob2"))            == NULL ||
            (isc_create_database         = (tp_isc_create_database)         dllFunc(dbDll, "isc_create_database"))         == NULL ||
            (isc_decode_sql_date         = (tp_isc_decode_sql_date)         dllFunc(dbDll, "isc_decode_sql_date"))         == NULL ||
            (isc_decode_sql_time         = (tp_isc_decode_sql_time)         dllFunc(dbDll, "isc_decode_sql_time"))         == NULL ||
            (isc_decode_timestamp        = (tp_isc_decode_timestamp)        dllFunc(dbDll, "isc_decode_timestamp"))        == NULL ||
            (isc_detach_database         = (tp_isc_detach_database)         dllFunc(dbDll, "isc_detach_database"))         == NULL ||
            (isc_dsql_allocate_statement = (tp_isc_dsql_allocate_statement) dllFunc(dbDll, "isc_dsql_allocate_statement")) == NULL ||
            (isc_dsql_describe           = (tp_isc_dsql_describe)           dllFunc(dbDll, "isc_dsql_describe"))           == NULL ||
            (isc_dsql_describe_bind      = (tp_isc_dsql_describe_bind)      dllFunc(dbDll, "isc_dsql_describe_bind"))      == NULL ||
            (isc_dsql_execute2           = (tp_isc_dsql_execute2)           dllFunc(dbDll, "isc_dsql_execute2"))           == NULL ||
            (isc_dsql_fetch              = (tp_isc_dsql_fetch)              dllFunc(dbDll, "isc_dsql_fetch"))              == NULL ||
            (isc_dsql_free_statement     = (tp_isc_dsql_free_statement)     dllFunc(dbDll, "isc_dsql_free_statement"))     == NULL ||
            (isc_dsql_prepare            = (tp_isc_dsql_prepare)            dllFunc(dbDll, "isc_dsql_prepare"))            == NULL ||
            (isc_dsql_sql_info           = (tp_isc_dsql_sql_info)           dllFunc(dbDll, "isc_dsql_sql_info"))           == NULL ||
            (isc_encode_sql_date         = (tp_isc_encode_sql_date)         dllFunc(dbDll, "isc_encode_sql_date"))         == NULL ||
            (isc_encode_sql_time         = (tp_isc_encode_sql_time)         dllFunc(dbDll, "isc_encode_sql_time"))         == NULL ||
            (isc_encode_timestamp        = (tp_isc_encode_timestamp)        dllFunc(dbDll, "isc_encode_timestamp"))        == NULL ||
            (isc_get_segment             = (tp_isc_get_segment)             dllFunc(dbDll, "isc_get_segment"))             == NULL ||
            (isc_interprete              = (tp_isc_interprete)              dllFunc(dbDll, "isc_interprete"))              == NULL ||
            (isc_open_blob2              = (tp_isc_open_blob2)              dllFunc(dbDll, "isc_open_blob2"))              == NULL ||
            (isc_portable_integer        = (tp_isc_portable_integer)        dllFunc(dbDll, "isc_portable_integer"))        == NULL ||
            (isc_print_status            = (tp_isc_print_status)            dllFunc(dbDll, "isc_print_status"))            == NULL ||
            (isc_put_segment             = (tp_isc_put_segment)             dllFunc(dbDll, "isc_put_segment"))             == NULL ||
            (isc_rollback_transaction    = (tp_isc_rollback_transaction)    dllFunc(dbDll, "isc_rollback_transaction"))    == NULL ||
            (isc_start_transaction       = (tp_isc_start_transaction)       dllFunc(dbDll, "isc_start_transaction"))       == NULL) {
          dbDll = NULL;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setupDll --> %d\n", dbDll != NULL););
    return dbDll != NULL;
  } /* setupDll */



static boolType findDll (void)

  {
    const char *dllList[] = { FIRE_DLL };
    unsigned int pos;
    boolType found = FALSE;

  /* findDll */
    for (pos = 0; pos < sizeof(dllList) / sizeof(char *) && !found; pos++) {
      found = setupDll(dllList[pos]);
    } /* for */
    if (!found) {
      dllErrorMessage("sqlOpenFire", "findDll", dllList,
                      sizeof(dllList) / sizeof(char *));
    } /* if */
    return found;
  } /* findDll */

#else

#define findDll() TRUE

#endif



static void setDbErrorMsg (const char *funcName, const char *dbFuncName,
    ISC_STATUS *status_vector)

  {
    ISC_STATUS *pvector;
    char messageText[512];

  /* setDbErrorMsg */
    dbError.funcName = funcName;
    dbError.dbFuncName = dbFuncName;
    dbError.errorCode = 0;
    pvector = status_vector;
    isc_interprete(messageText, &pvector);
    strcpy(dbError.message, messageText);
    messageText[0] = '\n';
    messageText[1] = '-';
    while (isc_interprete(&messageText[2], &pvector) != 0) {
      strcat(dbError.message, messageText);
    } /* while */
  } /* setDbErrorMsg */



static void sqlClose (databaseType database);



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
    ISC_STATUS status_vector[20];
    int column;
    int numColumns;
    XSQLVAR *var;

  /* freePreparedStmt */
    logFunction(printf("freePreparedStmt(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (preparedStmt->ppStmt != 0) {
      if (unlikely((isc_dsql_free_statement(status_vector,
                                            &preparedStmt->ppStmt,
                                            DSQL_drop),
                    status_vector[0] == 1 && status_vector[1] != 0))) {
        setDbErrorMsg("freePreparedStmt", "isc_dsql_free_statement",
                      status_vector);
        logError(printf("freePreparedStmt: isc_dsql_free_statement error:\n%s\n",
                        dbError.message););
        raise_error(DATABASE_ERROR);
      } /* if */
    } /* if */
    if (preparedStmt->in_sqlda != NULL) {
      numColumns = preparedStmt->in_sqlda->sqld;
      for (column = 0, var = preparedStmt->in_sqlda->sqlvar;
           column < numColumns; column++, var++) {
        free(var->sqldata);
        free(var->sqlind);
      } /* for */
      free(preparedStmt->in_sqlda);
    } /* if */
    if (preparedStmt->out_sqlda != NULL) {
      numColumns = preparedStmt->out_sqlda->sqld;
      for (column = 0, var = preparedStmt->out_sqlda->sqlvar;
           column < numColumns; column++, var++) {
        free(var->sqldata);
        free(var->sqlind);
      } /* for */
      free(preparedStmt->out_sqlda);
    } /* if */
    if (preparedStmt->param_array != NULL) {
      FREE_TABLE(preparedStmt->param_array, bindDataRecord,
                 (memSizeType) preparedStmt->in_sqlda->sqld);
    } /* if */
    FREE_RECORD2(preparedStmt, preparedStmtRecord,
                 count.prepared_stmt, count.prepared_stmt_bytes);
    logFunction(printf("freePreparedStmt -->\n"););
  } /* freePreparedStmt */



#if LOG_FUNCTIONS_EVERYWHERE || LOG_FUNCTIONS || VERBOSE_EXCEPTIONS_EVERYWHERE || VERBOSE_EXCEPTIONS
static const char *nameOfSqlType (int dtype)

  {
    static char buffer[50];
    const char *typeName;

  /* nameOfSqlType */
    logFunction(printf("nameOfSqlType(%d)\n", dtype););
    switch (dtype) {
      case SQL_TEXT:      typeName = "SQL_TEXT";      break;
      case SQL_VARYING:   typeName = "SQL_VARYING";   break;
      case SQL_SHORT:     typeName = "SQL_SHORT";     break;
      case SQL_LONG:      typeName = "SQL_LONG";      break;
      case SQL_FLOAT:     typeName = "SQL_FLOAT";     break;
      case SQL_DOUBLE:    typeName = "SQL_DOUBLE";    break;
      case SQL_D_FLOAT:   typeName = "SQL_D_FLOAT";   break;
      case SQL_TIMESTAMP: typeName = "SQL_TIMESTAMP"; break;
      case SQL_BLOB:      typeName = "SQL_BLOB";      break;
      case SQL_ARRAY:     typeName = "SQL_ARRAY";     break;
      case SQL_QUAD:      typeName = "SQL_QUAD";      break;
      case SQL_TYPE_TIME: typeName = "SQL_TYPE_TIME"; break;
      case SQL_TYPE_DATE: typeName = "SQL_TYPE_DATE"; break;
      case SQL_INT64:     typeName = "SQL_INT64";     break;
      case SQL_BOOLEAN:   typeName = "SQL_BOOLEAN";   break;
      case SQL_NULL:      typeName = "SQL_NULL";      break;
      default:
        sprintf(buffer, "%d", dtype);
        typeName = buffer;
        break;
    } /* switch */
    logFunction(printf("nameOfSqlType --> %s\n", typeName););
    return typeName;
  } /* nameOfSqlType */
#endif



static errInfoType assign_in_sqlda (preparedStmtType preparedStmt)

  {
    XSQLDA *in_sqlda;
    ISC_STATUS status_vector[20];
    errInfoType err_info = OKAY_NO_ERROR;

  /* assign_in_sqlda */
    in_sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(1));
    if (unlikely(in_sqlda == NULL)) {
      err_info = MEMORY_ERROR;
    } else {
      memset(in_sqlda, 0, XSQLDA_LENGTH(1));
      in_sqlda->version = SQLDA_VERSION1;
      in_sqlda->sqln = 1;
      if ((isc_dsql_describe_bind(status_vector,
          &preparedStmt->ppStmt, 1, in_sqlda),
          status_vector[0] == 1 && status_vector[1] != 0)) {
        setDbErrorMsg("sqlPrepare", "isc_dsql_describe_bind",
                      status_vector);
        logError(printf("sqlPrepare: isc_dsql_describe_bind error:\n%s\n",
                        dbError.message););
        free(in_sqlda);
        err_info = DATABASE_ERROR;
      } else {
        if (in_sqlda->sqld > in_sqlda->sqln) {
          short numInVars = in_sqlda->sqld;
          free(in_sqlda);
          in_sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH((memSizeType) numInVars));
          if (unlikely(in_sqlda == NULL)) {
            err_info = MEMORY_ERROR;
          } else {
            memset(in_sqlda, 0, XSQLDA_LENGTH((memSizeType) numInVars));
            in_sqlda->version = SQLDA_VERSION1;
            in_sqlda->sqln = numInVars;
            if ((isc_dsql_describe_bind(status_vector,
                &preparedStmt->ppStmt, 1, in_sqlda),
                status_vector[0] == 1 && status_vector[1] != 0)) {
              setDbErrorMsg("sqlPrepare", "isc_dsql_describe_bind",
                            status_vector);
              logError(printf("sqlPrepare: isc_dsql_describe_bind error:\n%s\n",
                              dbError.message););
              free(in_sqlda);
              err_info = DATABASE_ERROR;
            } else {
              preparedStmt->in_sqlda = in_sqlda;
            } /* if */
          } /* if */
        } else {
          preparedStmt->in_sqlda = in_sqlda;
        } /* if */
      } /* if */
    } /* if */
    return err_info;
  } /* assign_in_sqlda */



static errInfoType assign_out_sqlda (preparedStmtType preparedStmt,
    XSQLDA *out_sqlda)

  {
    ISC_STATUS status_vector[20];
    errInfoType err_info = OKAY_NO_ERROR;

  /* assign_out_sqlda */
    if (out_sqlda->sqld > out_sqlda->sqln) {
      short numOutVars = out_sqlda->sqld;
      free(out_sqlda);
      out_sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH((memSizeType) numOutVars));
      if (unlikely(out_sqlda == NULL)) {
        err_info = MEMORY_ERROR;
      } else {
        memset(out_sqlda, 0, XSQLDA_LENGTH((memSizeType) numOutVars));
        out_sqlda->version = SQLDA_VERSION1;
        out_sqlda->sqln = numOutVars;
        if ((isc_dsql_describe(status_vector, &preparedStmt->ppStmt,
            1, out_sqlda),
            status_vector[0] == 1 && status_vector[1] != 0)) {
          setDbErrorMsg("sqlPrepare", "isc_dsql_describe",
                        status_vector);
          logError(printf("sqlPrepare: isc_dsql_describe error:\n%s\n",
                          dbError.message););
          free(out_sqlda);
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->out_sqlda = out_sqlda;
        } /* if */
      } /* if */
    } else {
      preparedStmt->out_sqlda = out_sqlda;
    } /* if */
    return err_info;
  } /* assign_out_sqlda */



static errInfoType setupParameters (preparedStmtType preparedStmt)

  {
    int column;
    int numColumns;
    XSQLVAR *sqlvar;
    int dtype;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupParameters */
    logFunction(printf("setupParameters\n"););
    err_info = assign_in_sqlda(preparedStmt);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      preparedStmt->param_array = NULL;
    } else if (preparedStmt->in_sqlda->sqld == 0) {
      /* malloc(0) may return NULL, which would wrongly trigger a MEMORY_ERROR. */
      preparedStmt->param_array = NULL;
    } else if (unlikely(!ALLOC_TABLE(preparedStmt->param_array,
                                     bindDataRecord,
                                     (memSizeType) preparedStmt->in_sqlda->sqld))) {
      err_info = MEMORY_ERROR;
    } else {
      memset(preparedStmt->param_array, 0,
             (memSizeType) preparedStmt->in_sqlda->sqld * sizeof(bindDataRecord));
    } /* if */
    if (likely(err_info == OKAY_NO_ERROR)) {
      numColumns = preparedStmt->in_sqlda->sqld;
      for (column = 0, sqlvar = preparedStmt->in_sqlda->sqlvar;
           column < numColumns; column++, sqlvar++) {
        /* printf("sqllen: %hd\n", sqlvar->sqllen); */
        dtype = sqlvar->sqltype & ~1; /* drop flag bit for now */
        switch(dtype) {
          case SQL_TEXT:
            if (unlikely(sqlvar->sqllen <= 0)) {
              dbInconsistent("setupParameters", "sqllen");
              logError(printf("setupParameters: Column %d: "
                              "sqllen %hd of SQL_TEXT is negative or zero.\n",
                              column + 1, sqlvar->sqllen););
              err_info = DATABASE_ERROR;
            } else {
              sqlvar->sqldata = (char *) malloc(sizeof(char) * (memSizeType) sqlvar->sqllen);
            } /* if */
            break;
          case SQL_VARYING:
            if (unlikely(sqlvar->sqllen <= 0)) {
              dbInconsistent("setupParameters", "sqllen");
              logError(printf("setupParameters: Column %d: "
                              "sqllen %hd of SQL_VARYING is negative or zero.\n",
                              column + 1, sqlvar->sqllen););
              err_info = DATABASE_ERROR;
            } else {
              sqlvar->sqldata = (char *) malloc(sizeof(uint16Type) +
                  sizeof(char) * (memSizeType) sqlvar->sqllen);
            } /* if */
            break;
          case SQL_BOOLEAN:
          case SQL_SHORT:
            if (unlikely(sqlvar->sqlscale != 0)) {
              dbInconsistent("setupParameters", "sqlscale");
              logError(printf("setupParameters: Column %d: "
                              "The scale of an SQL_SHORT field must be 0.\n", column););
              err_info = DATABASE_ERROR;
            } else {
              sqlvar->sqldata = (char *) malloc(sizeof(short));
            } /* if */
            break;
          case SQL_LONG:
            if (unlikely(sqlvar->sqlscale != 0)) {
              dbInconsistent("setupParameters", "sqlscale");
              logError(printf("setupParameters: Column %d: "
                              "The scale of an SQL_LONG field must be 0.\n", column););
              err_info = DATABASE_ERROR;
            } else {
              sqlvar->sqldata = (char *) malloc(sizeof(ISC_LONG));
            } /* if */
            break;
          case SQL_INT64:
            sqlvar->sqldata = (char *) malloc(sizeof(ISC_INT64));
            break;
          case SQL_FLOAT:
            sqlvar->sqldata = (char *) malloc(sizeof(float));
            break;
          case SQL_DOUBLE:
            sqlvar->sqldata = (char *) malloc(sizeof(double));
            break;
          case SQL_BLOB:
            sqlvar->sqldata = (char *) malloc(sizeof(ISC_QUAD));
            break;
          case SQL_TIMESTAMP:
            sqlvar->sqldata = (char *) malloc(sizeof(ISC_TIMESTAMP));
            break;
          case SQL_TYPE_TIME:
            sqlvar->sqldata = (char *) malloc(sizeof(ISC_TIME));
            break;
          case SQL_TYPE_DATE:
            sqlvar->sqldata = (char *) malloc(sizeof(ISC_DATE));
            break;
          default:
            logError(printf("setupParameters: Column %d has the unknown type %s.\n",
                            column + 1, nameOfSqlType(dtype)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (unlikely(sqlvar->sqldata == NULL)) {
            err_info = MEMORY_ERROR;
          } else if (sqlvar->sqltype & 1) {
            /* allocate variable to hold NULL status */
            sqlvar->sqlind = (short *) malloc(sizeof(short));
            if (unlikely(sqlvar->sqlind == NULL)) {
              err_info = MEMORY_ERROR;
            } /* if */
          } /* if */
        } /* if */
      } /* for */
    } /* if */
    logFunction(printf("setupParameters --> %d\n", err_info););
    return err_info;
  } /* setupParameters */



static errInfoType setupResult (preparedStmtType preparedStmt,
    XSQLDA *out_sqlda)

  {
    int column;
    int numColumns;
    XSQLVAR *sqlvar;
    int dtype;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupResult */
    logFunction(printf("setupResult\n"););
    err_info = assign_out_sqlda(preparedStmt, out_sqlda);
    if (likely(err_info == OKAY_NO_ERROR)) {
      numColumns = preparedStmt->out_sqlda->sqld;
      for (column = 0, sqlvar = preparedStmt->out_sqlda->sqlvar;
           column < numColumns; column++, sqlvar++) {
        /* printf("sqllen: %hd\n", sqlvar->sqllen); */
        dtype = sqlvar->sqltype & ~1; /* drop flag bit for now */
        switch(dtype) {
          case SQL_TEXT:
            if (unlikely(sqlvar->sqllen <= 0)) {
              dbInconsistent("setupResult", "sqllen");
              logError(printf("setupResult: Column %d: "
                              "sqllen %hd of SQL_TEXT is negative or zero.\n",
                              column + 1, sqlvar->sqllen););
              err_info = DATABASE_ERROR;
            } else {
              sqlvar->sqldata = (char *) malloc(sizeof(char) * (memSizeType) sqlvar->sqllen);
            } /* if */
            break;
          case SQL_VARYING:
            if (unlikely(sqlvar->sqllen <= 0)) {
              dbInconsistent("setupResult", "sqllen");
              logError(printf("setupResult: Column %d: "
                              "sqllen %hd of SQL_VARYING is negative or zero.\n",
                              column + 1, sqlvar->sqllen););
              err_info = DATABASE_ERROR;
            } else {
              sqlvar->sqldata = (char *) malloc(sizeof(uint16Type) +
                  sizeof(char) * (memSizeType) sqlvar->sqllen);
            } /* if */
            break;
          case SQL_BOOLEAN:
          case SQL_SHORT:
            sqlvar->sqldata = (char *) malloc(sizeof(short));
            break;
          case SQL_LONG:
            sqlvar->sqldata = (char *) malloc(sizeof(ISC_LONG));
            break;
          case SQL_INT64:
            sqlvar->sqldata = (char *) malloc(sizeof(ISC_INT64));
            break;
          case SQL_FLOAT:
            sqlvar->sqldata = (char *) malloc(sizeof(float));
            break;
          case SQL_DOUBLE:
            sqlvar->sqldata = (char *) malloc(sizeof(double));
            break;
          case SQL_BLOB:
            sqlvar->sqldata = (char *) malloc(sizeof(ISC_QUAD));
            break;
          case SQL_TIMESTAMP:
            sqlvar->sqldata = (char *) malloc(sizeof(ISC_TIMESTAMP));
            break;
          case SQL_TYPE_TIME:
            sqlvar->sqldata = (char *) malloc(sizeof(ISC_TIME));
            break;
          case SQL_TYPE_DATE:
            sqlvar->sqldata = (char *) malloc(sizeof(ISC_DATE));
            break;
          case SQL_ARRAY:
            sqlvar->sqldata = (char *) malloc(sizeof(ISC_QUAD));
            break;
          default:
            logError(printf("setupResult: Column %d has the unknown type %s.\n",
                            column + 1, nameOfSqlType(dtype)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (unlikely(sqlvar->sqldata == NULL)) {
            err_info = MEMORY_ERROR;
          } else if (sqlvar->sqltype & 1) {
            /* allocate variable to hold NULL status */
            sqlvar->sqlind = (short *) malloc(sizeof(short));
            if (unlikely(sqlvar->sqlind == NULL)) {
              err_info = MEMORY_ERROR;
            } /* if */
          } /* if */
        } /* if */
      } /* for */
    } /* if */
    logFunction(printf("setupResult --> %d\n", err_info););
    return err_info;
  } /* setupResult */



static boolType allParametersBound (preparedStmtType preparedStmt)

  {
    int column_index;
    boolType okay = TRUE;

  /* allParametersBound */
    for (column_index = 0; column_index < preparedStmt->in_sqlda->sqld;
         column_index++) {
      if (unlikely(!preparedStmt->param_array[column_index].bound)) {
        logError(printf("sqlExecute: Unbound parameter %d.\n",
                        column_index + 1););
        okay = FALSE;
      } /* if */
    } /* for */
    return okay;
  } /* allParametersBound */



static int64Type intToScaledInt64 (const intType value,
    int decimalDigits, errInfoType *err_info)

  {
    int64Type factor;

  /* intToScaledInt64 */
    logFunction(printf("intToScaledInt64(" FMT_D ", %d, *)\n",
                       value, decimalDigits););
    switch (decimalDigits) {
      case  0: factor =                   1; break;
      case  1: factor =                  10; break;
      case  2: factor =                 100; break;
      case  3: factor =                1000; break;
      case  4: factor =               10000; break;
      case  5: factor =              100000; break;
      case  6: factor =             1000000; break;
      case  7: factor =            10000000; break;
      case  8: factor =           100000000; break;
      case  9: factor =          1000000000; break;
      case 10: factor =         10000000000; break;
      case 11: factor =        100000000000; break;
      case 12: factor =       1000000000000; break;
      case 13: factor =      10000000000000; break;
      case 14: factor =     100000000000000; break;
      case 15: factor =    1000000000000000; break;
      case 16: factor =   10000000000000000; break;
      case 17: factor =  100000000000000000; break;
      case 18: factor = 1000000000000000000; break;
      default:
        logError(printf("intToScaledInt64: Wrong number of decimal digits: %d\n",
                        decimalDigits););
        raise_error(RANGE_ERROR);
        factor = 1;
        break;
    } /* switch */
    if (value < 0) {
      if (unlikely(value < INT64TYPE_MIN / factor)) {
        logError(printf("intToScaledInt64: Value (" FMT_D
                        ") smaller than minimum (" FMT_D ").\n",
                        value, INT64TYPE_MIN / factor));
        *err_info = RANGE_ERROR;
        factor = 1;
      } /* if */
    } else {
      if (unlikely(value > INT64TYPE_MAX / factor)) {
        logError(printf("intToScaledInt64: Value (" FMT_D
                        ") larger than maximum (" FMT_D ").\n",
                        value, INT64TYPE_MAX / factor));
        *err_info = RANGE_ERROR;
        factor = 1;
      } /* if */
    } /* if */
    return value * factor;
  } /* intToScaledInt64 */



static int64Type bigIntToScaledInt64 (const const_bigIntType value,
    int decimalDigits, errInfoType *err_info)

  {
    bigIntType number;
    int64Type int64Value = 0;

  /* bigIntToScaledInt64 */
    logFunction(printf("bigIntToScaledInt64(%s, %d, *)\n",
                       bigHexCStri(value), decimalDigits););
    number = bigIPowSignedDigit(10, decimalDigits);
    if (number != NULL) {
      bigMultAssign(&number, value);
      if (number != NULL) {
        /* printf("mantissaValue: ");
           prot_bigint(mantissaValue);
           printf("\n"); */
        int64Value = bigToInt64(number, err_info);
        bigDestr(number);
      } /* if */
    } /* if */
    return int64Value;
  } /* bigIntToScaledInt64 */



static int64Type bigRatToScaledInt64 (const const_bigIntType numerator,
    const const_bigIntType denominator, int decimalDigits,
    errInfoType *err_info)

  {
    bigIntType number;
    bigIntType mantissaValue;
    int64Type int64Value = 0;

  /* bigRatToScaledInt64 */
    logFunction(printf("bigRatToScaledInt64(%s, %s, %d, *)\n",
                       bigHexCStri(numerator), bigHexCStri(denominator),
                       decimalDigits););
    if (unlikely(bigEqSignedDigit(denominator, 0))) {
      /* Numeric values do not support Infinity and NaN. */
      logError(printf("bigRatToScaledInt64: Decimal values do not support Infinity and NaN.\n"););
      *err_info = RANGE_ERROR;
    } else {
      number = bigIPowSignedDigit(10, decimalDigits);
      if (number != NULL) {
        bigMultAssign(&number, numerator);
        mantissaValue = bigDiv(number, denominator);
        bigDestr(number);
        if (mantissaValue != NULL) {
          /* printf("mantissaValue: ");
             prot_bigint(mantissaValue);
             printf("\n"); */
          int64Value = bigToInt64(mantissaValue, err_info);
          bigDestr(mantissaValue);
        } /* if */
      } /* if */
    } /* if */
    return int64Value;
  } /* bigRatToScaledInt64 */



static memSizeType getBlobLength (isc_blob_handle blob_handle, errInfoType *err_info)

  {
    ISC_STATUS status_vector[20];
    char blob_items[] = {isc_info_blob_total_length};
    char res_buffer[20];
    char *ptr;
    char item;
    short length;
    boolType totalLengthAssigned = FALSE;
    ISC_INT64 totalLength;

  /* getBlobLength */
    logFunction(printf("getBlobLength(" FMT_U32 ", *)\n", blob_handle););
    if ((isc_blob_info(status_vector,
                       &blob_handle,
                       sizeof(blob_items),
                       blob_items,
                       sizeof(res_buffer),
                       res_buffer),
               status_vector[0] == 1 && status_vector[1] != 0)) {
      setDbErrorMsg("getBlobLength", "isc_blob_info",
                    status_vector);
      logError(printf("getBlobLength: isc_blob_info() error:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
      totalLength = 0;
    } else {
      for (ptr = res_buffer; *ptr != isc_info_end;) {
        item = *ptr++;
        length = (short) isc_portable_integer(ptr, 2);
        ptr += 2;
        switch (item) {
          case isc_info_blob_total_length:
            totalLength = isc_portable_integer(ptr, length);
            totalLengthAssigned = TRUE;
            break;
          case isc_info_truncated:
            /* handle error */
            break;
          default:
            break;
        } /* switch */
        ptr += length;
      } /* for */
    } /* if */
    if (unlikely(!totalLengthAssigned)) {
      dbInconsistent("getBlobLength", "isc_blob_info");
      logError(printf("getBlobLength: Isc_blob_info returns no value "
                      " for isc_info_blob_total_length.\n"););
      *err_info = DATABASE_ERROR;
      totalLength = 0;
    } else if (unlikely(totalLength < 0)) {
      dbInconsistent("getBlobLength", "isc_blob_info");
      logError(printf("getBlobLength: Isc_blob_info returns negative "
                      " value for isc_info_blob_total_length: " FMT_D64 ".\n",
                      (int64Type) totalLength););
      *err_info = DATABASE_ERROR;
      totalLength = 0;
    } if (unlikely(totalLength > MAX_MEMSIZETYPE)) {
      /* It is not possible to cast totalLength to memSizeType. */
      /* Memory with this length cannot be allocated. */
      *err_info = MEMORY_ERROR;
      totalLength = 0;
    } /* if */
    logFunction(printf("getBlobLength --> " FMT_U_MEM "\n",
                       (memSizeType) totalLength););
    return (memSizeType) totalLength;
  } /* getBlobLength */



static memSizeType copyBlobContents (isc_blob_handle blob_handle,
    memSizeType blobLength, ustriType destination)

  {
    memSizeType bytesToGet;
    unsigned short segmentLength;
    unsigned short actualSegmentLength;
    ISC_STATUS status_vector[20];
    ISC_STATUS blob_stat;

  /* copyBlobContents */
    bytesToGet = blobLength;
    if (bytesToGet > UINT16TYPE_MAX) {
      segmentLength = UINT16TYPE_MAX;
    } else {
      segmentLength = (unsigned short) bytesToGet;
    } /* if */
    blob_stat = isc_get_segment(status_vector,
                                &blob_handle,
                                &actualSegmentLength,
                                segmentLength,
                                (char *) destination);
    while (blob_stat == 0 || status_vector[1] == isc_segment) {
      /* isc_get_segment returns 0 if a segment was successfully read. */
      /* status_vector[1] is set to isc_segment if only part of a */
      /* segment was read due to the buffer (blob_segment) not being */
      /* large enough. In that case, the following calls to */
      /* isc_get_segment() read the rest of the buffer. */
      bytesToGet -= (memSizeType) actualSegmentLength;
      destination += actualSegmentLength;
      if (bytesToGet > UINT16TYPE_MAX) {
        segmentLength = UINT16TYPE_MAX;
      } else {
        segmentLength = (unsigned short) bytesToGet;
      } /* if */
      blob_stat = isc_get_segment(status_vector,
                                  &blob_handle,
                                  &actualSegmentLength,
                                  segmentLength,
                                  (char *) destination);
    } /* while */
    return blobLength - bytesToGet;
  } /* copyBlobContents */



static bstriType getBlob (preparedStmtType preparedStmt,
    ISC_QUAD *blob_id, errInfoType *err_info)

  {
    ISC_STATUS status_vector[20];
    isc_blob_handle blob_handle = 0;
    memSizeType blobLength;
    bstriType bstri;

  /* getBlob */
    logFunction(printf("getBlob(" FMT_U_MEM ", " FMT_X64 ", *)\n",
                       (memSizeType) preparedStmt, *(uint64Type *) blob_id););
    /* Blob_handle must be zero when isc_open_blob2 is called. */
    if ((isc_open_blob2(status_vector,
                        &preparedStmt->db->connection,
                        &preparedStmt->db->trans_handle,
                        &blob_handle,
                        blob_id,
                        0,
                        NULL),
        status_vector[0] == 1 && status_vector[1] != 0)) {
      setDbErrorMsg("getBlob", "isc_open_blob2",
                    status_vector);
      logError(printf("getBlob: isc_open_blob2() error:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
      bstri = NULL;
    } else {
      blobLength = getBlobLength(blob_handle, err_info);
      if (likely(*err_info == OKAY_NO_ERROR)) {
        if (unlikely(blobLength > MAX_BSTRI_LEN ||
                     !ALLOC_BSTRI_SIZE_OK(bstri, blobLength))) {
          *err_info = MEMORY_ERROR;
          bstri = NULL;
        } else {
          bstri->size = copyBlobContents(blob_handle, blobLength, bstri->mem);
        } /* if */
      } else {
        bstri = NULL;
      } /* if */
      isc_close_blob(status_vector, &blob_handle);
      if (unlikely(status_vector[0] == 1 && status_vector[1] != 0)) {
        setDbErrorMsg("getBlob", "isc_close_blob",
                      status_vector);
        logError(printf("getBlob: isc_close_blob() error:\n%s\n",
                        dbError.message););
        *err_info = DATABASE_ERROR;
        FREE_BSTRI(bstri, blobLength);
        bstri = NULL;
      } /* if */
    } /* if */
    logFunction(printf("getBlob --> \"%s\"\n",
                        bstriAsUnquotedCStri(bstri)););
    return bstri;
  } /* getBlob */



static striType getBlobAsStri (preparedStmtType preparedStmt,
    ISC_QUAD *blob_id, errInfoType *err_info)

  {
    ISC_STATUS status_vector[20];
    isc_blob_handle blob_handle = 0;
    memSizeType blobLength;
    memSizeType actualBlobLength;
    striType stri;

  /* getBlobAsStri */
    logFunction(printf("getBlobAsStri(" FMT_U_MEM ", " FMT_X64 ", *)\n",
                       (memSizeType) preparedStmt, *(uint64Type *) blob_id););
    /* Blob_handle must be zero when isc_open_blob2 is called. */
    if (unlikely((isc_open_blob2(status_vector,
                                 &preparedStmt->db->connection,
                                 &preparedStmt->db->trans_handle,
                                 &blob_handle,
                                 blob_id,
                                 0,
                                 NULL),
                  status_vector[0] == 1 && status_vector[1] != 0))) {
      setDbErrorMsg("getBlobAsStri", "isc_open_blob2",
                    status_vector);
      logError(printf("getBlobAsStri: isc_open_blob2() error:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
      stri = NULL;
    } else {
      blobLength = getBlobLength(blob_handle, err_info);
      if (likely(*err_info == OKAY_NO_ERROR)) {
        if (unlikely(blobLength > MAX_STRI_LEN ||
                     !ALLOC_STRI_SIZE_OK(stri, blobLength))) {
          *err_info = MEMORY_ERROR;
          stri = NULL;
        } else {
          actualBlobLength = copyBlobContents(blob_handle, blobLength, (ustriType) stri->mem);
          memcpy_to_strelem(stri->mem, (ustriType) stri->mem, actualBlobLength);
          stri->size = actualBlobLength;
        } /* if */
      } else {
        stri = NULL;
      } /* if */
      isc_close_blob(status_vector, &blob_handle);
      if (unlikely(status_vector[0] == 1 && status_vector[1] != 0)) {
        setDbErrorMsg("getBlobAsStri", "isc_close_blob",
                      status_vector);
        logError(printf("getBlobAsStri: isc_close_blob() error:\n%s\n",
                        dbError.message););
        *err_info = DATABASE_ERROR;
        FREE_STRI(stri, blobLength);
        stri = NULL;
      } /* if */
    } /* if */
    logFunction(printf("getBlobAsStri --> \"%s\"\n",
                       striAsUnquotedCStri(stri)););
    return stri;
  } /* getBlobAsStri */



static striType getClob (preparedStmtType preparedStmt,
    ISC_QUAD *blob_id, errInfoType *err_info)

  {
    ISC_STATUS status_vector[20];
    isc_blob_handle blob_handle = 0;
    memSizeType blobLength;
    memSizeType actualBlobLength;
    cstriType utf8_stri;
    striType stri;

  /* getClob */
    logFunction(printf("getClob(" FMT_U_MEM ", " FMT_X64 ", *)\n",
                       (memSizeType) preparedStmt, *(uint64Type *) blob_id););
    /* Blob_handle must be zero when isc_open_blob2 is called. */
    if (unlikely((isc_open_blob2(status_vector,
                                 &preparedStmt->db->connection,
                                 &preparedStmt->db->trans_handle,
                                 &blob_handle,
                                 blob_id,
                                 0,
                                 NULL),
                  status_vector[0] == 1 && status_vector[1] != 0))) {
      setDbErrorMsg("getClob", "isc_open_blob2",
                    status_vector);
      logError(printf("getClob: isc_open_blob2() error:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
      stri = NULL;
    } else {
      blobLength = getBlobLength(blob_handle, err_info);
      if (likely(*err_info == OKAY_NO_ERROR)) {
        if (unlikely(blobLength > MAX_MEMSIZETYPE ||
                     !ALLOC_BYTES(utf8_stri, blobLength))) {
          *err_info = MEMORY_ERROR;
          stri = NULL;
        } else {
          actualBlobLength = copyBlobContents(blob_handle, blobLength, (ustriType) utf8_stri);
          stri = cstri8_buf_to_stri(utf8_stri, actualBlobLength, err_info);
          FREE_BYTES(utf8_stri, blobLength);
        } /* if */
      } else {
        stri = NULL;
      } /* if */
      isc_close_blob(status_vector, &blob_handle);
      if (unlikely(status_vector[0] == 1 && status_vector[1] != 0)) {
        setDbErrorMsg("getClob", "isc_close_blob",
                      status_vector);
        logError(printf("getClob: isc_close_blob() error:\n%s\n",
                        dbError.message););
        *err_info = DATABASE_ERROR;
        FREE_STRI(stri, blobLength);
        stri = NULL;
      } /* if */
    } /* if */
    logFunction(printf("getClob --> \"%s\"\n",
                       striAsUnquotedCStri(stri)););
    return stri;
  } /* getClob */



static errInfoType putBlob (preparedStmtType preparedStmt,
    const const_bstriType bstri, XSQLVAR *sqlvar)

  {
    ISC_STATUS status_vector[20];
    isc_blob_handle blob_handle = 0;
    ISC_QUAD blob_id;
    memSizeType byteIndex;
    memSizeType bytesToWrite;
    unsigned short segmentLength;
    errInfoType err_info = OKAY_NO_ERROR;

  /* putBlob */
    logFunction(printf("putBlob(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) preparedStmt, bstriAsUnquotedCStri(bstri)));
    /* Blob_handle must be zero when isc_create_blob2() is called. */
    if (unlikely((isc_create_blob2(status_vector,
                                  &preparedStmt->db->connection,
                                  &preparedStmt->db->trans_handle,
                                  &blob_handle,
                                  &blob_id,
                                  0,
                                  NULL),
                  status_vector[0] == 1 && status_vector[1] != 0))) {
      setDbErrorMsg("putBlob", "isc_create_blob2",
                    status_vector);
      logError(printf("putBlob: isc_create_blob2() error:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } else {
      byteIndex = 0;
      bytesToWrite = bstri->size;
      while (bytesToWrite > 0 && err_info == OKAY_NO_ERROR) {
        if (bytesToWrite > UINT16TYPE_MAX) {
          segmentLength = UINT16TYPE_MAX;
        } else {
          segmentLength = (unsigned short) bytesToWrite;
        } /* if */
        if (unlikely((isc_put_segment(status_vector,
                                      &blob_handle,
                                      segmentLength,
                                      (char *) &bstri->mem[byteIndex]),
                      status_vector[0] == 1 && status_vector[1] != 0))) {
          setDbErrorMsg("putBlob", "isc_put_segment",
                        status_vector);
          logError(printf("putBlob: isc_put_segment() error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          byteIndex += segmentLength;
          bytesToWrite -= segmentLength;
        } /* if */
      } /* while */
      if (unlikely((isc_close_blob(status_vector,
                                   &blob_handle),
                    status_vector[0] == 1 && status_vector[1] != 0))) {
        setDbErrorMsg("getBlob", "isc_close_blob",
                      status_vector);
        logError(printf("getBlob: isc_close_blob() error:\n%s\n",
                        dbError.message););
        err_info = DATABASE_ERROR;
      } else if (likely(err_info == OKAY_NO_ERROR)) {
        memcpy(sqlvar->sqldata, &blob_id, sizeof(ISC_QUAD));
      } /* if */
    } /* if */
    logFunction(printf("putBlob --> %d\n", err_info););
    return err_info;
  } /* putBlob */



static void sqlBindBigInt (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType value)

  {
    preparedStmtType preparedStmt;
    ISC_STATUS status_vector[20];
    XSQLVAR *sqlvar;
    striType decimalNumber;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBigInt */
    logFunction(printf("sqlBindBigInt(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, bigHexCStri(value)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || pos > preparedStmt->in_sqlda->sqld)) {
      logError(printf("sqlBindBigInt: pos: " FMT_D ", max pos: %hd.\n",
                      pos, preparedStmt->in_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely((isc_dsql_free_statement(status_vector,
                                              &preparedStmt->ppStmt,
                                              DSQL_close),
                      status_vector[0] == 1 && status_vector[1] != 0))) {
          setDbErrorMsg("sqlBindBigInt", "isc_dsql_free_statement",
                        status_vector);
          logError(printf("sqlBindBigInt: isc_dsql_free_statement error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        sqlvar = &preparedStmt->in_sqlda->sqlvar[pos - 1];
        /* printf("paramType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_SHORT:
            *(int16Type *) sqlvar->sqldata = bigToInt16(value, &err_info);
            break;
          case SQL_LONG:
            *(int32Type *) sqlvar->sqldata = bigToInt32(value, &err_info);
            break;
          case SQL_INT64:
            /* printf("sqlBindBigInt: scale: %hd\n", sqlvar->sqlscale); */
            if (sqlvar->sqlscale == 0) {
              *(int64Type *) sqlvar->sqldata = bigToInt64(value, &err_info);
            } else {
              *(int64Type *) sqlvar->sqldata = bigIntToScaledInt64(value,
                  -sqlvar->sqlscale, &err_info);
            } /* if */
            break;
          case SQL_FLOAT:
            *(float *) sqlvar->sqldata = (float) bigIntToDouble(value);
            break;
          case SQL_DOUBLE:
            *(double *) sqlvar->sqldata = bigIntToDouble(value);
            break;
          case SQL_TEXT:
            decimalNumber = bigStr(value);
            if (decimalNumber == NULL) {
              err_info = MEMORY_ERROR;
            } else {
              /* setupParameters() has already checked that sqllen > 0 holds */
              if (unlikely((memSizeType) sqlvar->sqllen < decimalNumber->size)) {
                logError(printf("sqlBindBigInt: Parameter " FMT_D ": "
                                "Decimal representation of %s longer than allowed (%hd).\n",
                                pos, striAsUnquotedCStri(decimalNumber),
                                sqlvar->sqllen););
                err_info = RANGE_ERROR;
              } else {
                memcpy_from_strelem((ustriType) sqlvar->sqldata,
                    decimalNumber->mem, decimalNumber->size);
                memset(&sqlvar->sqldata[decimalNumber->size], ' ',
                    (memSizeType) sqlvar->sqllen - decimalNumber->size);
              } /* if */
              FREE_STRI(decimalNumber, decimalNumber->size);
            } /* if */
            break;
          case SQL_VARYING:
            decimalNumber = bigStr(value);
            if (decimalNumber == NULL) {
              err_info = MEMORY_ERROR;
            } else {
              /* setupParameters() has already checked that sqllen > 0 holds */
              if (unlikely((memSizeType) sqlvar->sqllen < decimalNumber->size)) {
                logError(printf("sqlBindBigInt: Parameter " FMT_D ": "
                                "Decimal representation of %s longer than allowed (%hd).\n",
                                pos, striAsUnquotedCStri(decimalNumber),
                                sqlvar->sqllen););
                err_info = RANGE_ERROR;
              } else {
                *(int16Type *) sqlvar->sqldata =
                    (int16Type) decimalNumber->size;
                memcpy_from_strelem((ustriType)
                    &sqlvar->sqldata[sizeof(uint16Type)],
                    decimalNumber->mem, decimalNumber->size);
                memset(&sqlvar->sqldata[sizeof(uint16Type) + decimalNumber->size],
                    '\0', (memSizeType) sqlvar->sqllen - decimalNumber->size);
              } /* if */
              FREE_STRI(decimalNumber, decimalNumber->size);
            } /* if */
            break;
          default:
            logError(printf("sqlBindBigInt: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(sqlvar->sqltype & ~1)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (sqlvar->sqltype & 1) {
            *sqlvar->sqlind = 0;
          } /* if */
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
  } /* sqlBindBigInt */



static void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType numerator, const const_bigIntType denominator)

  {
    preparedStmtType preparedStmt;
    ISC_STATUS status_vector[20];
    XSQLVAR *sqlvar;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBigRat */
    logFunction(printf("sqlBindBigRat(" FMT_U_MEM ", " FMT_D ", %s, %s)\n",
                       (memSizeType) sqlStatement, pos,
                       bigHexCStri(numerator), bigHexCStri(denominator)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || pos > preparedStmt->in_sqlda->sqld)) {
      logError(printf("sqlBindBigRat: pos: " FMT_D ", max pos: %hd.\n",
                      pos, preparedStmt->in_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely((isc_dsql_free_statement(status_vector,
                                              &preparedStmt->ppStmt,
                                              DSQL_close),
                      status_vector[0] == 1 && status_vector[1] != 0))) {
          setDbErrorMsg("sqlBindBigRat", "isc_dsql_free_statement",
                        status_vector);
          logError(printf("sqlBindBigRat: isc_dsql_free_statement error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        sqlvar = &preparedStmt->in_sqlda->sqlvar[pos - 1];
        /* printf("paramType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_INT64:
            /* printf("sqlBindBigRat: scale: %hd\n", sqlvar->sqlscale); */
            *(int64Type *) sqlvar->sqldata = bigRatToScaledInt64(numerator, denominator,
                -sqlvar->sqlscale, &err_info);
            break;
          case SQL_FLOAT:
            *(float *) sqlvar->sqldata =
                (float) bigRatToDouble(numerator, denominator);;
            break;
          case SQL_DOUBLE:
            *(double *) sqlvar->sqldata =
                bigRatToDouble(numerator, denominator);
            break;
          default:
            logError(printf("sqlBindBigRat: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(sqlvar->sqltype & ~1)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (sqlvar->sqltype & 1) {
            *sqlvar->sqlind = 0;
          } /* if */
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
  } /* sqlBindBigRat */



static void sqlBindBool (sqlStmtType sqlStatement, intType pos, boolType value)

  {
    preparedStmtType preparedStmt;
    ISC_STATUS status_vector[20];
    XSQLVAR *sqlvar;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBool */
    logFunction(printf("sqlBindBool(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, value ? "TRUE" : "FALSE"););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || pos > preparedStmt->in_sqlda->sqld)) {
      logError(printf("sqlBindBool: pos: " FMT_D ", max pos: %hd.\n",
                      pos, preparedStmt->in_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely((isc_dsql_free_statement(status_vector,
                                              &preparedStmt->ppStmt,
                                              DSQL_close),
                      status_vector[0] == 1 && status_vector[1] != 0))) {
          setDbErrorMsg("sqlBindBool", "isc_dsql_free_statement",
                        status_vector);
          logError(printf("sqlBindBool: isc_dsql_free_statement error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        sqlvar = &preparedStmt->in_sqlda->sqlvar[pos - 1];
        /* printf("paramType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_BOOLEAN:
          case SQL_SHORT:
            *(int16Type *) sqlvar->sqldata = (int16Type) value;
            break;
          case SQL_LONG:
            *(int32Type *) sqlvar->sqldata = (int32Type) value;
            break;
          case SQL_INT64:
            /* printf("sqlBindBool: scale: %hd\n", sqlvar->sqlscale); */
            if (unlikely(sqlvar->sqlscale != 0)) {
              logError(printf("sqlBindBool: Parameter " FMT_D ": "
                              "The scale of a boolean field must be 0.\n", pos););
              err_info = RANGE_ERROR;
            } else {
              *(int64Type *) sqlvar->sqldata = (int64Type) value;
            } /* if */
            break;
          case SQL_TEXT:
            if (unlikely(sqlvar->sqllen != 1)) {
              logError(printf("sqlBindBool: Parameter " FMT_D ": "
                              "The size of a boolean field must be 1.\n", pos););
              err_info = RANGE_ERROR;
            } else {
              ((char *) sqlvar->sqldata)[0] = (char) ('0' + value);
            } /* if */
           break;
          default:
            logError(printf("sqlBindBool: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(sqlvar->sqltype & ~1)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (sqlvar->sqltype & 1) {
            *sqlvar->sqlind = 0;
          } /* if */
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
  } /* sqlBindBool */



static void sqlBindBStri (sqlStmtType sqlStatement, intType pos,
    const const_bstriType bstri)

  {
    preparedStmtType preparedStmt;
    ISC_STATUS status_vector[20];
    XSQLVAR *sqlvar;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBStri */
    logFunction(printf("sqlBindBStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, bstriAsUnquotedCStri(bstri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || pos > preparedStmt->in_sqlda->sqld)) {
      logError(printf("sqlBindBStri: pos: " FMT_D ", max pos: %hd.\n",
                      pos, preparedStmt->in_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely((isc_dsql_free_statement(status_vector,
                                              &preparedStmt->ppStmt,
                                              DSQL_close),
                      status_vector[0] == 1 && status_vector[1] != 0))) {
          setDbErrorMsg("sqlBindBStri", "isc_dsql_free_statement",
                        status_vector);
          logError(printf("sqlBindBStri: isc_dsql_free_statement error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        sqlvar = &preparedStmt->in_sqlda->sqlvar[pos - 1];
        /* printf("paramType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_BLOB:
            err_info = putBlob(preparedStmt, bstri, sqlvar);
            break;
          case SQL_TEXT:
            /* setupParameters() has already checked that sqllen > 0 holds */
            if (unlikely((memSizeType) sqlvar->sqllen < bstri->size)) {
              logError(printf("sqlBindBStri(*, " FMT_D ", \"%s\"): "
                              "Bstring longer than allowed (%hd).\n",
                              pos, bstriAsUnquotedCStri(bstri),
                              sqlvar->sqllen););
              err_info = RANGE_ERROR;
            } else {
              memcpy(sqlvar->sqldata, bstri->mem, bstri->size);
              memset(&sqlvar->sqldata[bstri->size], ' ',
                     (memSizeType) sqlvar->sqllen - bstri->size);
            } /* if */
            break;
          case SQL_VARYING:
            /* setupParameters() has already checked that sqllen > 0 holds */
            if (unlikely((memSizeType) sqlvar->sqllen < bstri->size)) {
              logError(printf("sqlBindBStri(*, " FMT_D ", \"%s\"): "
                              "Bstring longer than allowed (%hd).\n",
                              pos, bstriAsUnquotedCStri(bstri),
                              sqlvar->sqllen););
              err_info = RANGE_ERROR;
            } else {
              *(int16Type *) sqlvar->sqldata = (int16Type) bstri->size;
              memcpy(&sqlvar->sqldata[sizeof(uint16Type)],
                     bstri->mem, bstri->size);
              memset(&sqlvar->sqldata[sizeof(uint16Type) + bstri->size],
                     '\0', (memSizeType) sqlvar->sqllen - bstri->size);
            } /* if */
            break;
          default:
            logError(printf("sqlBindBStri: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(sqlvar->sqltype & ~1)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (sqlvar->sqltype & 1) {
            *sqlvar->sqlind = 0;
          } /* if */
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
  } /* sqlBindBStri */



static void sqlBindDuration (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  {
    preparedStmtType preparedStmt;
    ISC_STATUS status_vector[20];
    XSQLVAR *sqlvar;
    struct tm tm_time;
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
    if (unlikely(pos < 1 || pos > preparedStmt->in_sqlda->sqld)) {
      logError(printf("sqlBindDuration: pos: " FMT_D ", max pos: %hd.\n",
                      pos, preparedStmt->in_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else if (unlikely(year < -INT_MAX || year > INT_MAX || month < -12 || month > 12 ||
                        day < -31 || day > 31 || hour <= -24 || hour >= 24 ||
                        minute <= -60 || minute >= 60 || second <= -60 || second >= 60 ||
                        micro_second <= -1000000 || micro_second >= 1000000)) {
      logError(printf("sqlBindDuration: Duration not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely((isc_dsql_free_statement(status_vector,
                                              &preparedStmt->ppStmt,
                                              DSQL_close),
                      status_vector[0] == 1 && status_vector[1] != 0))) {
          setDbErrorMsg("sqlBindDuration", "isc_dsql_free_statement",
                        status_vector);
          logError(printf("sqlBindDuration: isc_dsql_free_statement error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        sqlvar = &preparedStmt->in_sqlda->sqlvar[pos - 1];
        /* printf("paramType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
            case SQL_TIMESTAMP:
              tm_time.tm_year = (int) year;
              tm_time.tm_mon  = (int) month;
              tm_time.tm_mday = (int) day;
              tm_time.tm_hour = (int) hour;
              tm_time.tm_min  = (int) minute;
              tm_time.tm_sec  = (int) second;
              isc_encode_timestamp(&tm_time, (ISC_TIMESTAMP *) sqlvar->sqldata);
              break;
            case SQL_TYPE_TIME:
              tm_time.tm_year = (int) year;
              tm_time.tm_mon  = (int) month;
              tm_time.tm_mday = (int) day;
              tm_time.tm_hour = (int) hour;
              tm_time.tm_min  = (int) minute;
              tm_time.tm_sec  = (int) second;
              isc_encode_sql_time(&tm_time, (ISC_TIME *) sqlvar->sqldata);
              break;
            case SQL_TYPE_DATE:
              tm_time.tm_year = (int) year;
              tm_time.tm_mon  = (int) month;
              tm_time.tm_mday = (int) day;
              tm_time.tm_hour = (int) hour;
              tm_time.tm_min  = (int) minute;
              tm_time.tm_sec  = (int) second;
              isc_encode_sql_date(&tm_time, (ISC_DATE *) sqlvar->sqldata);
              break;
          default:
            logError(printf("sqlBindDuration: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(sqlvar->sqltype & ~1)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (sqlvar->sqltype & 1) {
            *sqlvar->sqlind = 0;
          } /* if */
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
  } /* sqlBindDuration */



static void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value)

  {
    preparedStmtType preparedStmt;
    ISC_STATUS status_vector[20];
    XSQLVAR *sqlvar;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindFloat */
    logFunction(printf("sqlBindFloat(" FMT_U_MEM ", " FMT_D ", " FMT_E ")\n",
                       (memSizeType) sqlStatement, pos, value););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || pos > preparedStmt->in_sqlda->sqld)) {
      logError(printf("sqlBindFloat: pos: " FMT_D ", max pos: %hd.\n",
                      pos, preparedStmt->in_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely((isc_dsql_free_statement(status_vector,
                                              &preparedStmt->ppStmt,
                                              DSQL_close),
                      status_vector[0] == 1 && status_vector[1] != 0))) {
          setDbErrorMsg("sqlBindFloat", "isc_dsql_free_statement",
                        status_vector);
          logError(printf("sqlBindFloat: isc_dsql_free_statement error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        sqlvar = &preparedStmt->in_sqlda->sqlvar[pos - 1];
        /* printf("paramType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_FLOAT:
            *(float *) sqlvar->sqldata = (float) value;
            break;
          case SQL_DOUBLE:
            *(double *) sqlvar->sqldata = (double) value;
            break;
          default:
            logError(printf("sqlBindFloat: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(sqlvar->sqltype & ~1)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (sqlvar->sqltype & 1) {
            *sqlvar->sqlind = 0;
          } /* if */
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
  } /* sqlBindFloat */



static void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value)

  {
    preparedStmtType preparedStmt;
    ISC_STATUS status_vector[20];
    XSQLVAR *sqlvar;
    char decimalInt[INTTYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    int decimalLength;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindInt */
    logFunction(printf("sqlBindInt(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos, value););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || pos > preparedStmt->in_sqlda->sqld)) {
      logError(printf("sqlBindInt: pos: " FMT_D ", max pos: %hd.\n",
                      pos, preparedStmt->in_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely((isc_dsql_free_statement(status_vector,
                                              &preparedStmt->ppStmt,
                                              DSQL_close),
                      status_vector[0] == 1 && status_vector[1] != 0))) {
          setDbErrorMsg("sqlBindInt", "isc_dsql_free_statement",
                        status_vector);
          logError(printf("sqlBindInt: isc_dsql_free_statement error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        sqlvar = &preparedStmt->in_sqlda->sqlvar[pos - 1];
        /* printf("paramType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_SHORT:
            if (unlikely(value < INT16TYPE_MIN || value > INT16TYPE_MAX)) {
              logError(printf("sqlBindInt: Parameter " FMT_D ": "
                              FMT_D " does not fit into a 16-bit integer.\n",
                              pos, value));
              err_info = RANGE_ERROR;
            } else {
              *(int16Type *) sqlvar->sqldata = (int16Type) value;
            } /* if */
            break;
          case SQL_LONG:
            if (unlikely(value < INT32TYPE_MIN || value > INT32TYPE_MAX)) {
              logError(printf("sqlBindInt: Parameter " FMT_D ": "
                              FMT_D " does not fit into a 32-bit integer.\n",
                              pos, value));
              err_info = RANGE_ERROR;
            } else {
              *(int32Type *) sqlvar->sqldata = (int32Type) value;
            } /* if */
            break;
          case SQL_INT64:
            /* printf("sqlBindInt: scale: %hd\n", sqlvar->sqlscale); */
            if (sqlvar->sqlscale == 0) {
              *(int64Type *) sqlvar->sqldata = value;
            } else {
              *(int64Type *) sqlvar->sqldata = intToScaledInt64(value,
                  -sqlvar->sqlscale, &err_info);
            } /* if */
            break;
          case SQL_FLOAT:
            *(float *) sqlvar->sqldata = (float) value;
            break;
          case SQL_DOUBLE:
            *(double *) sqlvar->sqldata = (double) value;
            break;
          case SQL_TEXT:
            decimalLength = sprintf(decimalInt, FMT_D, value);
            if (unlikely(sqlvar->sqllen < decimalLength)) {
              logError(printf("sqlBindInt: Parameter " FMT_D ": "
                              "Decimal representation of " FMT_D
                              " longer than allowed (%hd).\n",
                              pos, value, sqlvar->sqllen););
              err_info = RANGE_ERROR;
            } else {
              memcpy(sqlvar->sqldata, decimalInt, (memSizeType) decimalLength);
              memset(&sqlvar->sqldata[decimalLength], ' ',
                  (memSizeType) sqlvar->sqllen - (memSizeType) decimalLength);
            } /* if */
            break;
          case SQL_VARYING:
            decimalLength = sprintf(decimalInt, FMT_D, value);
            if (unlikely(sqlvar->sqllen < decimalLength)) {
              logError(printf("sqlBindInt: Parameter " FMT_D ": "
                              "Decimal representation of " FMT_D
                              " longer than allowed (%hd).\n",
                              pos, value, sqlvar->sqllen););
              err_info = RANGE_ERROR;
            } else {
              *(int16Type *) sqlvar->sqldata = (int16Type) decimalLength;
              memcpy(&sqlvar->sqldata[sizeof(uint16Type)], decimalInt,
                  (memSizeType) decimalLength);
              memset(&sqlvar->sqldata[sizeof(uint16Type) + (memSizeType) decimalLength],
                  '\0', (memSizeType) sqlvar->sqllen -
                  (memSizeType) decimalLength);
            } /* if */
            break;
          default:
            logError(printf("sqlBindInt: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(sqlvar->sqltype & ~1)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (sqlvar->sqltype & 1) {
            *sqlvar->sqlind = 0;
          } /* if */
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
  } /* sqlBindInt */



static void sqlBindNull (sqlStmtType sqlStatement, intType pos)

  {
    preparedStmtType preparedStmt;
    ISC_STATUS status_vector[20];
    XSQLVAR *sqlvar;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindNull */
    logFunction(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || pos > preparedStmt->in_sqlda->sqld)) {
      logError(printf("sqlBindNull: pos: " FMT_D ", max pos: %hd.\n",
                      pos, preparedStmt->in_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely((isc_dsql_free_statement(status_vector,
                                              &preparedStmt->ppStmt,
                                              DSQL_close),
                      status_vector[0] == 1 && status_vector[1] != 0))) {
          setDbErrorMsg("sqlBindNull", "isc_dsql_free_statement",
                        status_vector);
          logError(printf("sqlBindNull: isc_dsql_free_statement error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        sqlvar = &preparedStmt->in_sqlda->sqlvar[pos - 1];
        /* printf("paramType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        if (sqlvar->sqltype & 1) {
          *sqlvar->sqlind = -1;
        } /* if */
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->param_array[pos - 1].bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindNull */



static void sqlBindStri (sqlStmtType sqlStatement, intType pos,
    const const_striType stri)

  {
    preparedStmtType preparedStmt;
    ISC_STATUS status_vector[20];
    XSQLVAR *sqlvar;
    cstriType stri8;
    memSizeType length;
    bstriType bstri;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindStri */
    logFunction(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, striAsUnquotedCStri(stri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || pos > preparedStmt->in_sqlda->sqld)) {
      logError(printf("sqlBindStri: pos: " FMT_D ", max pos: %hd.\n",
                      pos, preparedStmt->in_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely((isc_dsql_free_statement(status_vector,
                                              &preparedStmt->ppStmt,
                                              DSQL_close),
                      status_vector[0] == 1 && status_vector[1] != 0))) {
          setDbErrorMsg("sqlBindStri", "isc_dsql_free_statement",
                        status_vector);
          logError(printf("sqlBindStri: isc_dsql_free_statement error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        sqlvar = &preparedStmt->in_sqlda->sqlvar[pos - 1];
        /* printf("paramType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_TEXT:
            stri8 = stri_to_cstri8_buf(stri, &length);
            if (unlikely(stri8 == NULL)) {
              err_info = MEMORY_ERROR;
            } else {
              /* setupParameters() has already checked that sqllen > 0 holds */
              if (unlikely((memSizeType) sqlvar->sqllen < length)) {
                logError(printf("sqlBindStri(*, " FMT_D ", \"%s\"): "
                                "UTF-8 length (" FMT_U_MEM ") longer than allowed (%hd).\n",
                                pos, striAsUnquotedCStri(stri), length, sqlvar->sqllen););
                err_info = RANGE_ERROR;
              } else {
                memcpy(sqlvar->sqldata, stri8, length);
                memset(&sqlvar->sqldata[length], ' ',
                       (memSizeType) sqlvar->sqllen - length);
              } /* if */
              free(stri8);
            } /* if */
            break;
          case SQL_VARYING:
            stri8 = stri_to_cstri8_buf(stri, &length);
            if (unlikely(stri8 == NULL)) {
              err_info = MEMORY_ERROR;
            } else {
              /* setupParameters() has already checked that sqllen > 0 holds */
              if (unlikely((memSizeType) sqlvar->sqllen < length)) {
                logError(printf("sqlBindStri(*, " FMT_D ", \"%s\"): "
                                "UTF-8 length (" FMT_U_MEM ") longer than allowed (%hd).\n",
                                pos, striAsUnquotedCStri(stri), length, sqlvar->sqllen););
                err_info = RANGE_ERROR;
              } else {
                *(int16Type *) sqlvar->sqldata = (int16Type) length;
                memcpy(&sqlvar->sqldata[sizeof(uint16Type)], stri8, length);
                memset(&sqlvar->sqldata[sizeof(uint16Type) + length], '\0',
                       (memSizeType) sqlvar->sqllen - length);
              } /* if */
              free(stri8);
            } /* if */
            break;
          case SQL_BLOB:
            if (sqlvar->sqlsubtype == 1) {
              /* BLOB SUB_TYPE 1 means essentially: textual Blob. */
              bstri = stri_to_bstri8(stri);
              if (unlikely(bstri == NULL)) {
                err_info = MEMORY_ERROR;
              } else {
                err_info = putBlob(preparedStmt, bstri, sqlvar);
                FREE_BSTRI(bstri, bstri->size);
              } /* if */
            } else {
              /* BLOB SUB_TYPE 0 is a binary Blob. */
              logError(printf("sqlBindStri: Parameter " FMT_D " is a BLOB column.\n", pos););
              err_info = RANGE_ERROR;
            } /* if */
            break;
          default:
            logError(printf("sqlBindStri: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(sqlvar->sqltype & ~1)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (sqlvar->sqltype & 1) {
            *sqlvar->sqlind = 0;
          } /* if */
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
  } /* sqlBindStri */



static void sqlBindTime (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second,
    intType time_zone)

  {
    preparedStmtType preparedStmt;
    ISC_STATUS status_vector[20];
    XSQLVAR *sqlvar;
    struct tm tm_time;
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
    if (unlikely(pos < 1 || pos > preparedStmt->in_sqlda->sqld)) {
      logError(printf("sqlBindTime: pos: " FMT_D ", max pos: %hd.\n",
                      pos, preparedStmt->in_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else if (unlikely(year < INT_MIN + 1900 || year > INT_MAX ||
                        month < 1 || month > 12 || day < 1 || day > 31 ||
                        hour < 0 || hour >= 24 || minute < 0 ||
                        minute >= 60 || second < 0 || second >= 60 ||
                        micro_second < 0 || micro_second >= 1000000)) {
      logError(printf("sqlBindTime: Time not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely((isc_dsql_free_statement(status_vector,
                                              &preparedStmt->ppStmt,
                                              DSQL_close),
                      status_vector[0] == 1 && status_vector[1] != 0))) {
          setDbErrorMsg("sqlBindTime", "isc_dsql_free_statement",
                        status_vector);
          logError(printf("sqlBindTime: isc_dsql_free_statement error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        sqlvar = &preparedStmt->in_sqlda->sqlvar[pos - 1];
        /* printf("paramType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
            case SQL_TIMESTAMP:
              tm_time.tm_year = (int) year - 1900;
              tm_time.tm_mon  = (int) month - 1;
              tm_time.tm_mday = (int) day;
              tm_time.tm_hour = (int) hour;
              tm_time.tm_min  = (int) minute;
              tm_time.tm_sec  = (int) second;
              isc_encode_timestamp(&tm_time, (ISC_TIMESTAMP *) sqlvar->sqldata);
              ((ISC_TIMESTAMP *) sqlvar->sqldata)->timestamp_time += (ISC_TIME) micro_second / 100;
              break;
            case SQL_TYPE_TIME:
              tm_time.tm_year = (int) year - 1900;
              tm_time.tm_mon  = (int) month - 1;
              tm_time.tm_mday = (int) day;
              tm_time.tm_hour = (int) hour;
              tm_time.tm_min  = (int) minute;
              tm_time.tm_sec  = (int) second;
              isc_encode_sql_time(&tm_time, (ISC_TIME *) sqlvar->sqldata);
              *(ISC_TIME *) sqlvar->sqldata += (ISC_TIME) micro_second / 100;
              break;
            case SQL_TYPE_DATE:
              tm_time.tm_year = (int) year - 1900;
              tm_time.tm_mon  = (int) month - 1;
              tm_time.tm_mday = (int) day;
              tm_time.tm_hour = (int) hour;
              tm_time.tm_min  = (int) minute;
              tm_time.tm_sec  = (int) second;
              isc_encode_sql_date(&tm_time, (ISC_DATE *) sqlvar->sqldata);
              break;
          default:
            logError(printf("sqlBindTime: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(sqlvar->sqltype & ~1)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (sqlvar->sqltype & 1) {
            *sqlvar->sqlind = 0;
          } /* if */
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->param_array[pos - 1].bound = TRUE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
  } /* sqlBindTime */



static void sqlClose (databaseType database)

  {
    dbType db;
    ISC_STATUS status_vector[20];
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlClose */
    logFunction(printf("sqlClose(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (db->connection != 0) {
      if (db->trans_handle != 0) {
        isc_commit_transaction(status_vector,
                               &db->trans_handle);
        if (unlikely(status_vector[0] == 1 && status_vector[1] != 0)) {
          setDbErrorMsg("sqlClose", "isc_commit_transaction",
                        status_vector);
          logError(printf("sqlClose: isc_commit_transaction error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          db->trans_handle = 0;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        isc_detach_database(status_vector,
                            &db->connection);
        if (unlikely(status_vector[0] == 1 && status_vector[1] != 0)) {
          setDbErrorMsg("sqlClose", "isc_detach_database",
                        status_vector);
          logError(printf("sqlClose: isc_detach_database error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          db->connection = 0;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunction(printf("sqlClose -->\n"););
  } /* sqlClose */



static bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    XSQLVAR *sqlvar;
    short *sqlind;
    bigIntType columnValue;

  /* sqlColumnBigInt */
    logFunction(printf("sqlColumnBigInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 column > preparedStmt->out_sqlda->sqld)) {
      logError(printf("sqlColumnBigInt: Fetch okay: %d, column: " FMT_D
                      ", max column: %hd.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->out_sqlda->sqld););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      sqlvar = &preparedStmt->out_sqlda->sqlvar[column - 1];
      sqlind = sqlvar->sqlind;
      if (sqlind != NULL && *sqlind == -1) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        columnValue = bigZero();
      } else if (unlikely(sqlind != NULL && *sqlind != 0)) {
        dbInconsistent("sqlColumnBigInt", "sqlind");
        logError(printf("sqlColumnBigInt: Column " FMT_D ": "
                        "sqlind has the value %d.\n",
                        column, *sqlind););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else {
        /* printf("columnType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_SHORT:
            if (unlikely(sqlvar->sqlscale != 0)) {
              logError(printf("sqlColumnBigInt: Column " FMT_D ": "
                              "The scale of an integer field must be 0.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = NULL;
            } else {
              columnValue = bigFromInt32((int32Type) *(int16Type *) sqlvar->sqldata);
            } /* if */
            break;
          case SQL_LONG:
            if (unlikely(sqlvar->sqlscale != 0)) {
              logError(printf("sqlColumnBigInt: Column " FMT_D ": "
                              "The scale of an integer field must be 0.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = NULL;
            } else {
              columnValue = bigFromInt32(*(int32Type *) sqlvar->sqldata);
            } /* if */
            break;
          case SQL_INT64:
            if (unlikely(sqlvar->sqlscale != 0)) {
              logError(printf("sqlColumnBigInt: Column " FMT_D ": "
                              "The scale of an integer field must be 0.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = NULL;
            } else {
              columnValue = bigFromInt64(*(int64Type *) sqlvar->sqldata);
            } /* if */
            break;
          default:
            logError(printf("sqlColumnBigInt: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(sqlvar->sqltype & ~1)););
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
    XSQLVAR *sqlvar;
    short *sqlind;
    float floatValue;
    double doubleValue;

  /* sqlColumnBigRat */
    logFunction(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", *, *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 column > preparedStmt->out_sqlda->sqld)) {
      logError(printf("sqlColumnBigRat: Fetch okay: %d, column: " FMT_D
                      ", max column: %hd.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->out_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else {
      sqlvar = &preparedStmt->out_sqlda->sqlvar[column - 1];
      sqlind = sqlvar->sqlind;
      if (sqlind != NULL && *sqlind == -1) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        *numerator = bigZero();
        *denominator = bigFromInt32(1);
      } else if (unlikely(sqlind != NULL && *sqlind != 0)) {
        dbInconsistent("sqlColumnBigRat", "sqlind");
        logError(printf("sqlColumnBigRat: Column " FMT_D ": "
                        "sqlind has the value %d.\n",
                        column, *sqlind););
        raise_error(DATABASE_ERROR);
      } else {
        /* printf("columnType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_SHORT:
            *numerator = bigFromInt32((int32Type)
                *(int16Type *) sqlvar->sqldata);
            if (sqlvar->sqlscale == 0) {
              *denominator = bigFromInt32(1);
            } else {
              *denominator = bigIPowSignedDigit(10, -sqlvar->sqlscale);
            } /* if */
            break;
          case SQL_LONG:
            *numerator = bigFromInt32(*(int32Type *) sqlvar->sqldata);
            if (sqlvar->sqlscale == 0) {
              *denominator = bigFromInt32(1);
            } else {
              *denominator = bigIPowSignedDigit(10, -sqlvar->sqlscale);
            } /* if */
            break;
          case SQL_INT64:
            *numerator = bigFromInt64(*(int64Type *) sqlvar->sqldata);
            if (sqlvar->sqlscale == 0) {
              *denominator = bigFromInt32(1);
            } else {
              *denominator = bigIPowSignedDigit(10, -sqlvar->sqlscale);
            } /* if */
            break;
          case SQL_FLOAT:
            floatValue = *(float *) sqlvar->sqldata;
            /* printf("sqlColumnBigRat: float: %f\n", floatValue); */
            *numerator = roundDoubleToBigRat(floatValue, FALSE, denominator);
            break;
          case SQL_DOUBLE:
            doubleValue = *(double *) sqlvar->sqldata;
            /* printf("sqlColumnBigRat: double: %f\n", doubleValue); */
            *numerator = roundDoubleToBigRat(doubleValue, TRUE, denominator);
            break;
          default:
            logError(printf("sqlColumnBigRat: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(sqlvar->sqltype & ~1)););
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
    XSQLVAR *sqlvar;
    short *sqlind;
    intType columnValue;

  /* sqlColumnBool */
    logFunction(printf("sqlColumnBool(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 column > preparedStmt->out_sqlda->sqld)) {
      logError(printf("sqlColumnBool: Fetch okay: %d, column: " FMT_D
                      ", max column: %hd.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->out_sqlda->sqld););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      sqlvar = &preparedStmt->out_sqlda->sqlvar[column - 1];
      sqlind = sqlvar->sqlind;
      if (sqlind != NULL && *sqlind == -1) {
        /* printf("Column is NULL -> Use default value: FALSE\n"); */
        columnValue = 0;
      } else if (unlikely(sqlind != NULL && *sqlind != 0)) {
        dbInconsistent("sqlColumnBool", "sqlind");
        logError(printf("sqlColumnBool: Column " FMT_D ": "
                        "sqlind has the value %d.\n",
                        column, *sqlind););
        raise_error(DATABASE_ERROR);
        columnValue = 0;
      } else {
        /* printf("columnType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_BOOLEAN:
          case SQL_SHORT:
            if (unlikely(sqlvar->sqlscale != 0)) {
              logError(printf("sqlColumnBool: Column " FMT_D ": "
                              "The scale of a boolean field must be 0.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = 0;
            } else {
              columnValue = *(int16Type *) sqlvar->sqldata;
            } /* if */
            break;
          case SQL_LONG:
            if (unlikely(sqlvar->sqlscale != 0)) {
              logError(printf("sqlColumnBool: Column " FMT_D ": "
                              "The scale of a boolean field must be 0.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = 0;
            } else {
              columnValue = *(int32Type *) sqlvar->sqldata;
            } /* if */
            break;
          case SQL_INT64:
            if (unlikely(sqlvar->sqlscale != 0)) {
              logError(printf("sqlColumnBool: Column " FMT_D ": "
                              "The scale of a boolean field must be 0.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = 0;
            } else {
              columnValue = *(int64Type *) sqlvar->sqldata;
            } /* if */
            break;
          case SQL_TEXT:
            if (unlikely(sqlvar->sqllen != 1)) {
              logError(printf("sqlColumnBool: Column " FMT_D ": "
                              "The size of a boolean field must be 1.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = 0;
            } else {
              columnValue = ((char *) sqlvar->sqldata)[0] != '0';
            } /* if */
            break;
          default:
            logError(printf("sqlColumnBool: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(sqlvar->sqltype & ~1)););
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
    XSQLVAR *sqlvar;
    short *sqlind;
    short length;
    uint16Type varyingLength;
    errInfoType err_info = OKAY_NO_ERROR;
    bstriType columnValue;

  /* sqlColumnBStri */
    logFunction(printf("sqlColumnBStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 column > preparedStmt->out_sqlda->sqld)) {
      logError(printf("sqlColumnBStri: Fetch okay: %d, column: " FMT_D
                      ", max column: %hd.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->out_sqlda->sqld););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      sqlvar = &preparedStmt->out_sqlda->sqlvar[column - 1];
      sqlind = sqlvar->sqlind;
      if (sqlind != NULL && *sqlind == -1) {
        /* printf("Column is NULL -> Use default value: \"\"\n"); */
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(columnValue, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          columnValue->size = 0;
        } /* if */
      } else if (unlikely(sqlind != NULL && *sqlind != 0)) {
        dbInconsistent("sqlColumnBStri", "sqlind");
        logError(printf("sqlColumnBStri: Column " FMT_D ": "
                        "sqlind has the value %d.\n",
                        column, *sqlind););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else {
        /* printf("columnType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_TEXT:
            length = sqlvar->sqllen;
            /* printf("length: %hd\n", length); */
            while (length > 0 && sqlvar->sqldata[length - 1] == ' ') {
              length--;
            } /* if */
            if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(columnValue, (memSizeType) length))) {
              raise_error(MEMORY_ERROR);
            } else {
              memcpy(columnValue->mem, sqlvar->sqldata, (memSizeType) length);
              columnValue->size = (memSizeType) length;
            } /* if */
            break;
          case SQL_VARYING:
            length = sqlvar->sqllen;
            /* printf("length: %hd\n", length); */
            varyingLength = *(uint16Type *) sqlvar->sqldata;
            /* printf("varyingLength: " FMT_U16 "\n", varyingLength); */
            if (unlikely(varyingLength > length)) {
              dbInconsistent("sqlColumnBStri", "sqllen");
              logError(printf("sqlColumnBStri: Column " FMT_D ": "
                              "Length of SQL_VARYING " FMT_U16 " larger than field size %hd.\n",
                              column, varyingLength, length););
              raise_error(DATABASE_ERROR);
              columnValue = NULL;
            } else if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(columnValue, (memSizeType) varyingLength))) {
              raise_error(MEMORY_ERROR);
            } else {
              memcpy(columnValue->mem,
                     &sqlvar->sqldata[sizeof(uint16Type)],
                     (memSizeType) varyingLength);
              columnValue->size = (memSizeType) varyingLength;
            } /* if */
            break;
          case SQL_BLOB:
            if (sqlvar->sqlsubtype == 1) {
              /* BLOB SUB_TYPE 1 means essentially: textual Blob. */
              logError(printf("sqlColumnBStri: Column " FMT_D " is a CLOB.\n",
                              column););
              raise_error(RANGE_ERROR);
              columnValue = NULL;
            } else {
              /* BLOB SUB_TYPE 0 is a binary Blob. */
              columnValue = getBlob(preparedStmt,
                                    (ISC_QUAD *) sqlvar->sqldata,
                                    &err_info);
              if (unlikely(columnValue == NULL)) {
                raise_error(err_info);
              } /* if */
            } /* if */
            break;
          default:
            logError(printf("sqlColumnBStri: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(sqlvar->sqltype & ~1)););
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
    XSQLVAR *sqlvar;
    short *sqlind;
    struct tm tm_time;

  /* sqlColumnDuration */
    logFunction(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 column > preparedStmt->out_sqlda->sqld)) {
      logError(printf("sqlColumnDuration: Fetch okay: %d, column: " FMT_D
                      ", max column: %hd.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->out_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else {
      sqlvar = &preparedStmt->out_sqlda->sqlvar[column - 1];
      sqlind = sqlvar->sqlind;
      if (sqlind != NULL && *sqlind == -1) {
        /* printf("Column is NULL -> Use default value: P0D\n"); */
        *year         = 0;
        *month        = 0;
        *day          = 0;
        *hour         = 0;
        *minute       = 0;
        *second       = 0;
        *micro_second = 0;
      } else if (unlikely(sqlind != NULL && *sqlind != 0)) {
        dbInconsistent("sqlColumnDuration", "sqlind");
        logError(printf("sqlColumnDuration: Column " FMT_D ": "
                        "sqlind has the value %d.\n",
                        column, *sqlind););
        raise_error(DATABASE_ERROR);
      } else {
        /* printf("columnType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_TIMESTAMP:
            isc_decode_timestamp((ISC_TIMESTAMP *) sqlvar->sqldata, &tm_time);
            *year   = tm_time.tm_year;
            *month  = tm_time.tm_mon - 13;
            *day    = tm_time.tm_mday;
            *hour   = tm_time.tm_hour;
            *minute = tm_time.tm_min;
            *second = tm_time.tm_sec;
            *micro_second = 0;
            break;
          case SQL_TYPE_TIME:
            isc_decode_sql_time((ISC_TIME *) sqlvar->sqldata, &tm_time);
            *year   = tm_time.tm_year;
            *month  = tm_time.tm_mon;
            *day    = tm_time.tm_mday;
            *hour   = tm_time.tm_hour;
            *minute = tm_time.tm_min;
            *second = tm_time.tm_sec;
            *micro_second = 0;
            break;
          case SQL_TYPE_DATE:
            isc_decode_sql_date((ISC_DATE *) sqlvar->sqldata, &tm_time);
            *year   = tm_time.tm_year;
            *month  = tm_time.tm_mon;
            *day    = tm_time.tm_mday;
            *hour   = tm_time.tm_hour;
            *minute = tm_time.tm_min;
            *second = tm_time.tm_sec;
            *micro_second = 0;
            break;
          default:
            logError(printf("sqlColumnDuration: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(sqlvar->sqltype & ~1)););
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
    XSQLVAR *sqlvar;
    short *sqlind;
    floatType columnValue;

  /* sqlColumnFloat */
    logFunction(printf("sqlColumnFloat(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 column > preparedStmt->out_sqlda->sqld)) {
      logError(printf("sqlColumnFloat: Fetch okay: %d, column: " FMT_D
                      ", max column: %hd.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->out_sqlda->sqld););
      raise_error(RANGE_ERROR);
      columnValue = 0.0;
    } else {
      sqlvar = &preparedStmt->out_sqlda->sqlvar[column - 1];
      sqlind = sqlvar->sqlind;
      if (sqlind != NULL && *sqlind == -1) {
        /* printf("Column is NULL -> Use default value: 0.0\n"); */
        columnValue = 0.0;
      } else if (unlikely(sqlind != NULL && *sqlind != 0)) {
        dbInconsistent("sqlColumnFloat", "sqlind");
        logError(printf("sqlColumnFloat: Column " FMT_D ": "
                        "sqlind has the value %d.\n",
                        column, *sqlind););
        raise_error(DATABASE_ERROR);
        columnValue = 0.0;
      } else {
        /* printf("columnType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_SHORT:
            if (sqlvar->sqlscale == 0) {
              columnValue = (floatType) *(int16Type *) sqlvar->sqldata;
            } else {
              columnValue = ((floatType) *(int16Type *) sqlvar->sqldata) /
                  fltIPow(10.0, -sqlvar->sqlscale);
            } /* if */
            break;
          case SQL_LONG:
            if (sqlvar->sqlscale == 0) {
              columnValue = (floatType) *(int32Type *) sqlvar->sqldata;
            } else {
              columnValue = ((floatType) *(int32Type *) sqlvar->sqldata) /
                  fltIPow(10.0, -sqlvar->sqlscale);
            } /* if */
            break;
          case SQL_INT64:
            if (sqlvar->sqlscale == 0) {
              columnValue = (floatType) *(int64Type *) sqlvar->sqldata;
            } else {
              columnValue = ((floatType) *(int64Type *) sqlvar->sqldata) /
                  fltIPow(10.0, -sqlvar->sqlscale);
            } /* if */
            break;
          case SQL_FLOAT:
            columnValue = *(float *) sqlvar->sqldata;
            break;
          case SQL_DOUBLE:
            columnValue = *(double *) sqlvar->sqldata;
            break;
          default:
            logError(printf("sqlColumnFloat: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(sqlvar->sqltype & ~1)););
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
    XSQLVAR *sqlvar;
    short *sqlind;
    intType columnValue;

  /* sqlColumnInt */
    logFunction(printf("sqlColumnInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 column > preparedStmt->out_sqlda->sqld)) {
      logError(printf("sqlColumnInt: Fetch okay: %d, column: " FMT_D
                      ", max column: %hd.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->out_sqlda->sqld););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      sqlvar = &preparedStmt->out_sqlda->sqlvar[column - 1];
      sqlind = sqlvar->sqlind;
      if (sqlind != NULL && *sqlind == -1) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        columnValue = 0;
      } else if (unlikely(sqlind != NULL && *sqlind != 0)) {
        dbInconsistent("sqlColumnInt", "sqlind");
        logError(printf("sqlColumnInt: Column " FMT_D ": "
                        "sqlind has the value %d.\n",
                        column, *sqlind););
        raise_error(DATABASE_ERROR);
        columnValue = 0;
      } else {
        /* printf("columnType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_SHORT:
            if (unlikely(sqlvar->sqlscale != 0)) {
              logError(printf("sqlColumnInt: Column " FMT_D ": "
                              "The scale of an integer field must be 0.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = 0;
            } else {
              columnValue = *(int16Type *) sqlvar->sqldata;
            } /* if */
            break;
          case SQL_LONG:
            if (unlikely(sqlvar->sqlscale != 0)) {
              logError(printf("sqlColumnInt: Column " FMT_D ": "
                              "The scale of an integer field must be 0.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = 0;
            } else {
              columnValue = *(int32Type *) sqlvar->sqldata;
            } /* if */
            break;
          case SQL_INT64:
            if (unlikely(sqlvar->sqlscale != 0)) {
              logError(printf("sqlColumnInt: Column " FMT_D ": "
                              "The scale of an integer field must be 0.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = 0;
            } else {
              columnValue = *(int64Type *) sqlvar->sqldata;
            } /* if */
            break;
          default:
            logError(printf("sqlColumnInt: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(sqlvar->sqltype & ~1)););
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
    XSQLVAR *sqlvar;
    short *sqlind;
    short length;
    uint16Type varyingLength;
    errInfoType err_info = OKAY_NO_ERROR;
    striType columnValue;

  /* sqlColumnStri */
    logFunction(printf("sqlColumnStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 column > preparedStmt->out_sqlda->sqld)) {
      logError(printf("sqlColumnStri: Fetch okay: %d, column: " FMT_D
                      ", max column: %hd.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->out_sqlda->sqld););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      sqlvar = &preparedStmt->out_sqlda->sqlvar[column - 1];
      sqlind = sqlvar->sqlind;
      if (sqlind != NULL && *sqlind == -1) {
        /* printf("Column is NULL -> Use default value: \"\"\n"); */
        columnValue = strEmpty();
      } else if (unlikely(sqlind != NULL && *sqlind != 0)) {
        dbInconsistent("sqlColumnStri", "sqlind");
        logError(printf("sqlColumnStri: Column " FMT_D ": "
                        "sqlind has the value %d.\n",
                        column, *sqlind););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else {
        /* printf("columnType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_TEXT:
            length = sqlvar->sqllen;
            /* printf("length: %hd\n", length); */
            while (length > 0 && sqlvar->sqldata[length - 1] == ' ') {
              length--;
            } /* if */
            columnValue = cstri8_buf_to_stri(sqlvar->sqldata,
                (memSizeType) length, &err_info);
            if (unlikely(columnValue == NULL)) {
              raise_error(err_info);
            } /* if */
            break;
          case SQL_VARYING:
            length = sqlvar->sqllen;
            /* printf("length: %hd\n", length); */
            varyingLength = *(uint16Type *) sqlvar->sqldata;
            /* printf("varyingLength: " FMT_U16 "\n", varyingLength); */
            if (unlikely(varyingLength > length)) {
              dbInconsistent("sqlColumnStri", "sqllen");
              logError(printf("sqlColumnStri: Column " FMT_D ": "
                              "Length of SQL_VARYING " FMT_U16 " larger than field size %hd.\n",
                              column, varyingLength, length););
              raise_error(DATABASE_ERROR);
              columnValue = NULL;
            } else {
              columnValue = cstri8_buf_to_stri(
                  &sqlvar->sqldata[sizeof(uint16Type)],
                  (memSizeType) varyingLength, &err_info);
              if (unlikely(columnValue == NULL)) {
                raise_error(err_info);
              } /* if */
            } /* if */
            break;
          case SQL_BLOB:
            if (sqlvar->sqlsubtype == 1) {
              /* BLOB SUB_TYPE 1 means essentially: textual Blob. */
              columnValue = getClob(preparedStmt,
                                    (ISC_QUAD *) sqlvar->sqldata,
                                    &err_info);
            } else {
              /* BLOB SUB_TYPE 0 is a binary Blob. */
              columnValue = getBlobAsStri(preparedStmt,
                                          (ISC_QUAD *) sqlvar->sqldata,
                                          &err_info);
            } /* if */
            if (unlikely(columnValue == NULL)) {
              raise_error(err_info);
            } /* if */
            break;
#if 0
          case SQL_ARRAY:
            length = sqlvar->sqllen;
            printf("length: %hd\n", length);
            printf("data[0]: %d\n", sqlvar->sqldata[0]);
            printf("data[1]: %d\n", sqlvar->sqldata[1]);
            printf("data[2]: %d\n", sqlvar->sqldata[2]);
            printf("data[3]: %d\n", sqlvar->sqldata[3]);
            printf("data[4]: %d\n", sqlvar->sqldata[4]);
            printf("data[5]: %d\n", sqlvar->sqldata[5]);
            printf("data[6]: %d\n", sqlvar->sqldata[6]);
            printf("data[7]: %d\n", sqlvar->sqldata[7]);
            printf("varyingLength: %u\n",
                   sqlvar->sqldata[0] +
                   sqlvar->sqldata[1] * 256);
            printf("sqldata: " FMT_U_MEM "\n", (memSizeType) sqlvar->sqldata);
            printf("varyingLength: %hu\n",
                   *(uint16Type *) sqlvar->sqldata);
            raise_error(RANGE_ERROR);
            columnValue = NULL;
            break;
#endif
          default:
            logError(printf("sqlColumnStri: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(sqlvar->sqltype & ~1)););
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
    XSQLVAR *sqlvar;
    short *sqlind;
    struct tm tm_time;

  /* sqlColumnTime */
    logFunction(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 column > preparedStmt->out_sqlda->sqld)) {
      logError(printf("sqlColumnTime: Fetch okay: %d, column: " FMT_D
                      ", max column: %hd.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->out_sqlda->sqld););
      raise_error(RANGE_ERROR);
    } else {
      sqlvar = &preparedStmt->out_sqlda->sqlvar[column - 1];
      sqlind = sqlvar->sqlind;
      if (sqlind != NULL && *sqlind == -1) {
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
      } else if (unlikely(sqlind != NULL && *sqlind != 0)) {
        dbInconsistent("sqlColumnTime", "sqlind");
        logError(printf("sqlColumnTime: Column " FMT_D ": "
                        "sqlind has the value %d.\n",
                        column, *sqlind););
        raise_error(DATABASE_ERROR);
      } else {
        /* printf("columnType: %s\n", nameOfSqlType(sqlvar->sqltype & ~1)); */
        switch (sqlvar->sqltype & ~1) {
          case SQL_TIMESTAMP:
            isc_decode_timestamp((ISC_TIMESTAMP *) sqlvar->sqldata, &tm_time);
            *year   = tm_time.tm_year + 1900;
            *month  = tm_time.tm_mon + 1;
            *day    = tm_time.tm_mday;
            *hour   = tm_time.tm_hour;
            *minute = tm_time.tm_min;
            *second = tm_time.tm_sec;
            *micro_second = (intType) (((ISC_TIMESTAMP *) sqlvar->sqldata)->timestamp_time % 10000) * 100;
            timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                          time_zone, is_dst);
            break;
          case SQL_TYPE_TIME:
            isc_decode_sql_time((ISC_TIME *) sqlvar->sqldata, &tm_time);
            *year   = tm_time.tm_year + 1900;
            *month  = tm_time.tm_mon + 1;
            *day    = tm_time.tm_mday;
            *hour   = tm_time.tm_hour;
            *minute = tm_time.tm_min;
            *second = tm_time.tm_sec;
            *micro_second = (intType) (*(ISC_TIME *) sqlvar->sqldata % 10000) * 100;
            timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                          time_zone, is_dst);
            break;
          case SQL_TYPE_DATE:
            isc_decode_sql_date((ISC_DATE *) sqlvar->sqldata, &tm_time);
            *year   = tm_time.tm_year + 1900;
            *month  = tm_time.tm_mon + 1;
            *day    = tm_time.tm_mday;
            *hour   = tm_time.tm_hour;
            *minute = tm_time.tm_min;
            *second = tm_time.tm_sec;
            *micro_second = 0;
            timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                          time_zone, is_dst);
            break;
          default:
            logError(printf("sqlColumnTime: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(sqlvar->sqltype & ~1)););
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
    ISC_STATUS status_vector[20];

  /* sqlCommit */
    logFunction(printf("sqlCommit(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    isc_commit_transaction(status_vector,
                           &db->trans_handle);
    if (unlikely(status_vector[0] == 1 && status_vector[1] != 0)) {
      setDbErrorMsg("sqlCommit", "isc_commit_transaction",
                    status_vector);
      logError(printf("sqlCommit: isc_commit_transaction error:\n%s\n",
                      dbError.message););
      raise_error(DATABASE_ERROR);
    } else {
      /* Set transaction handle to zero for isc_start_transaction(). */
      db->trans_handle = 0;
      isc_start_transaction(status_vector,
                            &db->trans_handle,
                            1,
                            &db->connection,
                            (unsigned short) sizeof(isc_tbp),
                            isc_tbp);
      if (unlikely(status_vector[0] == 1 && status_vector[1] != 0)) {
        setDbErrorMsg("sqlCommit", "isc_start_transaction",
                      status_vector);
        logError(printf("sqlCommit: isc_start_transaction error:\n%s\n",
                        dbError.message););
        raise_error(DATABASE_ERROR);
      } /* if */
    } /* if */
  } /* sqlCommit */



static void sqlExecute (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    ISC_STATUS status_vector[20];
    errInfoType err_info = OKAY_NO_ERROR;

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
      if (preparedStmt->executeSuccessful) {
        if (unlikely((isc_dsql_free_statement(status_vector,
                                              &preparedStmt->ppStmt,
                                              DSQL_close),
                      status_vector[0] == 1 && status_vector[1] != 0))) {
          setDbErrorMsg("sqlExecute", "isc_dsql_free_statement",
                        status_vector);
          logError(printf("sqlExecute: isc_dsql_free_statement error:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        preparedStmt->fetchOkay = FALSE;
        isc_dsql_execute2(status_vector,
                          &preparedStmt->db->trans_handle,
                          &preparedStmt->ppStmt,
                          1, /* in_sqlda uses an XSQLDA descriptor. */
                          preparedStmt->in_sqlda,
                          NULL); /* Multiple rows are accessed with isc_dsql_fetch(). */
        if (unlikely(status_vector[0] == 1 && status_vector[1] != 0)) {
          setDbErrorMsg("sqlExecute", "isc_dsql_execute2",
                        status_vector);
          logError(printf("sqlExecute: isc_dsql_execute2:\n%s\n",
                          dbError.message););
          preparedStmt->executeSuccessful = FALSE;
          raise_error(DATABASE_ERROR);
        } else {
          if (preparedStmt->statement_type == isc_info_sql_stmt_ddl) {
            sqlCommit((databaseType) preparedStmt->db);
          } /* if */
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
    ISC_STATUS status_vector[20];
    ISC_STATUS fetch_result;

  /* sqlFetch */
    logFunction(printf("sqlFetch(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->executeSuccessful)) {
      dbLibError("sqlFetch", "SQLExecute",
                 "Execute was not successful.\n");
      logError(printf("sqlFetch: Execute was not successful.\n"););
      preparedStmt->fetchOkay = FALSE;
      raise_error(DATABASE_ERROR);
    } else if (preparedStmt->out_sqlda->sqld == 0) {
      preparedStmt->fetchOkay = FALSE;
    } else if (!preparedStmt->fetchFinished) {
      /* printf("ppStmt: " FMT_U_MEM "\n", (memSizeType) preparedStmt->ppStmt); */
      fetch_result = isc_dsql_fetch(status_vector,
                                    &preparedStmt->ppStmt,
                                    1, /* out_sqlda uses an XSQLDA descriptor. */
                                    preparedStmt->out_sqlda);
      if (fetch_result == 0) {
        /* printf("fetch success\n"); */
        preparedStmt->fetchOkay = TRUE;
      } else if (fetch_result == 100) {
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->fetchFinished = TRUE;
      } else {
        setDbErrorMsg("sqlFetch", "isc_dsql_fetch",
                      status_vector);
        logError(printf("sqlFetch: isc_dsql_fetch fetch_result: " FMT_D_MEM ":\n%s\n",
                        (memSizeType) fetch_result, dbError.message););
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
    XSQLVAR *sqlvar;
    short *sqlind;
    boolType isNull;

  /* sqlIsNull */
    logFunction(printf("sqlIsNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 column > preparedStmt->out_sqlda->sqld)) {
      logError(printf("sqlIsNull: Fetch okay: %d, column: " FMT_D
                      ", max column: %hd.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->out_sqlda->sqld););
      raise_error(RANGE_ERROR);
      isNull = FALSE;
    } else {
      sqlvar = &preparedStmt->out_sqlda->sqlvar[column - 1];
      sqlind = sqlvar->sqlind;
      if (sqlind != NULL && *sqlind == -1) {
        isNull = TRUE;
      } else if (unlikely(sqlind != NULL && *sqlind != 0)) {
        dbInconsistent("sqlColumnInt", "sqlind");
        logError(printf("sqlColumnInt: Column " FMT_D ": "
                        "sqlind has the value %d.\n",
                        column, *sqlind););
        raise_error(DATABASE_ERROR);
        isNull = FALSE;
      } else {
        isNull = FALSE;
      } /* if */
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
    XSQLDA *out_sqlda;
    ISC_STATUS status_vector[20];
    char res_buffer[8];
    short length;
    errInfoType err_info = OKAY_NO_ERROR;
    preparedStmtType preparedStmt;

  /* sqlPrepare */
    logFunction(printf("sqlPrepare(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) database,
                       striAsUnquotedCStri(sqlStatementStri)););
    db = (dbType) database;
    if (db->connection == 0) {
      logError(printf("sqlPrepare: Database is not open.\n"););
      err_info = RANGE_ERROR;
      preparedStmt = NULL;
    } else {
      query = stri_to_cstri8_buf(sqlStatementStri, &queryLength);
      if (unlikely(query == NULL)) {
        err_info = MEMORY_ERROR;
        preparedStmt = NULL;
      } else {
        if (unlikely(queryLength > UINT16TYPE_MAX)) {
          /* It is not possible to cast queryLength to unsigned short. */
          logError(printf("sqlPrepare: Statement string too long (length = " FMT_U_MEM ")\n",
                          queryLength););
          err_info = RANGE_ERROR;
          preparedStmt = NULL;
        } else if (unlikely((out_sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(1))) == NULL ||
                            !ALLOC_RECORD2(preparedStmt, preparedStmtRecord,
                                           count.prepared_stmt, count.prepared_stmt_bytes))) {
          if (out_sqlda != NULL) {
            free(out_sqlda);
          } /* if */
          err_info = MEMORY_ERROR;
          preparedStmt = NULL;
        } else {
          /* printf("sqlPrepare: query: %s\n", query); */
          memset(out_sqlda, 0, XSQLDA_LENGTH(1));
          memset(preparedStmt, 0, sizeof(preparedStmtRecord));
          out_sqlda->version = SQLDA_VERSION1;
          out_sqlda->sqln = 1;
          /* The statement handle must be zero when isc_dsql_allocate_statement() is called. */
          preparedStmt->ppStmt = 0;
          if ((isc_dsql_allocate_statement(status_vector,
                                           &db->connection,
                                           &preparedStmt->ppStmt),
               status_vector[0] == 1 && status_vector[1] != 0)) {
            setDbErrorMsg("sqlPrepare", "isc_dsql_allocate_statement",
                          status_vector);
            logError(printf("sqlPrepare(" FMT_U_MEM ", \"%s\"): "
                            "isc_dsql_allocate_statement error:\n%s\n",
                            (memSizeType) database,
                            striAsUnquotedCStri(sqlStatementStri),
                            dbError.message););
            free(out_sqlda);
            err_info = DATABASE_ERROR;
          } else if ((isc_dsql_prepare(status_vector,
                                       &db->trans_handle,
                                       &preparedStmt->ppStmt,
                                       (unsigned short) queryLength,
                                       query,
                                       3, /* dialect, indicates V6.0 functionalities */
                                       out_sqlda),
                      status_vector[0] == 1 && status_vector[1] != 0)) {
            setDbErrorMsg("sqlPrepare", "isc_dsql_prepare",
                          status_vector);
            logError(printf("sqlPrepare(" FMT_U_MEM ", \"%s\"): "
                            "isc_dsql_prepare error:\n%s\n",
                            (memSizeType) database,
                            striAsUnquotedCStri(sqlStatementStri),
                            dbError.message););
            free(out_sqlda);
            err_info = DATABASE_ERROR;
          } else {
            preparedStmt->usage_count = 1;
            preparedStmt->sqlFunc = db->sqlFunc;
            preparedStmt->executeSuccessful = FALSE;
            preparedStmt->fetchOkay = FALSE;
            preparedStmt->fetchFinished = TRUE;
            preparedStmt->db = db;
            err_info = setupParameters(preparedStmt);
            if (unlikely(err_info != OKAY_NO_ERROR)) {
              free(out_sqlda);
            } else {
              err_info = setupResult(preparedStmt, out_sqlda);
              if (likely(err_info == OKAY_NO_ERROR)) {
                isc_dsql_sql_info(status_vector,
                                  &preparedStmt->ppStmt,
                                  sizeof(type_item),
                                  type_item,
                                  sizeof(res_buffer),
                                  res_buffer);
                if (unlikely(status_vector[0] == 1 && status_vector[1] != 0)) {
                  setDbErrorMsg("sqlPrepare", "isc_dsql_sql_info",
                                status_vector);
                  logError(printf("sqlPrepare: isc_dsql_sql_info:\n%s\n",
                                 dbError.message););
                } else {
                  if (res_buffer[0] == isc_info_sql_stmt_type) {
                    length = (short) isc_portable_integer(&res_buffer[1], 2);
                    preparedStmt->statement_type =
                        (int) isc_portable_integer(&res_buffer[3], length);
                    /* printf("statement_type: %d\n", preparedStmt->statement_type); */
                  } /* if */
                } /* if */
              } /* if */
            } /* if */
          } /* if */
          if (unlikely(err_info != OKAY_NO_ERROR)) {
            freePreparedStmt((sqlStmtType) preparedStmt);
            preparedStmt = NULL;
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
    /* The element sqld has the type ISC_SHORT (short).      */
    /* Therefore it will always fit into the intType result. */
    columnCount = (intType) preparedStmt->out_sqlda->sqld;
    logFunction(printf("sqlStmtColumnCount --> " FMT_D "\n", columnCount););
    return columnCount;
  } /* sqlStmtColumnCount */



static striType sqlStmtColumnName (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    XSQLVAR *sqlvar;
    errInfoType err_info = OKAY_NO_ERROR;
    striType name;

  /* sqlStmtColumnName */
    logFunction(printf("sqlStmtColumnName(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(column < 1 ||
                 column > preparedStmt->out_sqlda->sqld)) {
      logError(printf("sqlStmtColumnName: column: " FMT_D
                      ", max column: %hd.\n",
                      column, preparedStmt->out_sqlda->sqld););
      raise_error(RANGE_ERROR);
      name = NULL;
    } else {
      sqlvar = &preparedStmt->out_sqlda->sqlvar[column - 1];
      if (unlikely(sqlvar->sqlname_length < 0)) {
        dbInconsistent("sqlStmtColumnName", "length");
        logError(printf("sqlStmtColumnName: Column " FMT_D ": "
                        "sqlname_length %hd is negative.\n", column,
                        sqlvar->sqlname_length););
        raise_error(DATABASE_ERROR);
        name = NULL;
      } else {
        name = cstri8_buf_to_stri(sqlvar->sqlname,
            (memSizeType) sqlvar->sqlname_length, &err_info);
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



static errInfoType doAttach (loginType loginData, const_cstriType extension,
    isc_db_handle *db_handle)

  {
    const const_cstriType charset = "UTF8";
    memSizeType charset_length;
    memSizeType extension_length;
    memSizeType fileName8Length;
    cstriType fileName8;
    memSizeType dpb_buffer_length;
    char *dpb_buffer;
    short dpb_length;
    char *dpb;
    ISC_STATUS status_vector[20];
    errInfoType err_info = OKAY_NO_ERROR;

  /* doAttach */
    extension_length = strlen(extension);
    if (loginData->fileName8Length < extension_length ||
        memcmp(&loginData->fileName8[loginData->fileName8Length - extension_length],
               extension, extension_length) != 0) {
      fileName8Length = loginData->fileName8Length + extension_length;
      if (unlikely(fileName8Length > 255 ||
                   !ALLOC_CSTRI(fileName8, fileName8Length))) {
        fileName8 = NULL;
      } else {
        memcpy(fileName8, loginData->fileName8, loginData->fileName8Length);
        memcpy(&fileName8[loginData->fileName8Length], extension, extension_length);
        fileName8[fileName8Length] = '\0';
      } /* if */
    } else {
      fileName8Length = loginData->fileName8Length;
      fileName8 = loginData->fileName8;
    } /* if */
    if (unlikely(fileName8 == NULL)) {
      err_info = MEMORY_ERROR;
    } else {
      charset_length = strlen(charset);
      /* Assume that the setting bytes take less then 256 bytes space. */
      dpb_buffer_length = fileName8Length + loginData->user8Length +
          loginData->password8Length + charset_length + 256;
      if (unlikely(!ALLOC_CSTRI(dpb_buffer, dpb_buffer_length))) {
        err_info = MEMORY_ERROR;
      } else {
        /* Construct the database parameter buffer. */
        dpb = dpb_buffer;
        *dpb++ = isc_dpb_version1;
        *dpb++ = isc_dpb_utf8_filename;
        *dpb++ = (char) fileName8Length;
        memcpy(dpb, fileName8, fileName8Length);
        dpb += fileName8Length;
        *dpb++ = isc_dpb_user_name;
        *dpb++ = (char) loginData->user8Length;
        memcpy(dpb, loginData->user8, loginData->user8Length);
        dpb += loginData->user8Length;
        *dpb++ = isc_dpb_password;
        *dpb++ = (char) loginData->password8Length;
        memcpy(dpb, loginData->password8, loginData->password8Length);
        dpb += loginData->password8Length;
        *dpb++ = isc_dpb_set_db_charset;
        *dpb++ = (char) charset_length;
        memcpy(dpb, charset, charset_length);
        dpb += charset_length;
#if 0
        /* what does this? */
        *dpb++ = isc_num_buffers;
        *dpb++ = 1;
        *dpb++ = 90;
#endif
        /* Add (unnecessary) null byte, to be on the safe side. */
        *dpb = '\0';
        dpb_length = (short) (dpb - dpb_buffer);
        /* Set database handle to zero before attaching to a database. */
        *db_handle = 0;
        /* Attach to the database. */
        isc_attach_database(status_vector, (short) fileName8Length,
                            fileName8, db_handle, dpb_length, dpb_buffer);
        if (status_vector[0] == 1 && status_vector[1] != 0) {
          setDbErrorMsg("sqlOpenFire", "isc_attach_database",
                        status_vector);
          logError(printf("sqlOpenFire: isc_attach_database(*, \"%s\", ...  )"
                          " user: \"%s\", password: \"%s\", error:\n%s\n",
                          fileName8, loginData->user8, loginData->password8,
                          dbError.message););
          err_info = DATABASE_ERROR;
        } /* if */
        UNALLOC_CSTRI(dpb_buffer, dpb_buffer_length);
      } /* if */
      if (fileName8 != loginData->fileName8) {
        UNALLOC_CSTRI(fileName8, fileName8Length);
      } /* if */
    } /* if */
    return err_info;
  } /* doAttach */



databaseType sqlOpenFire (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  {
    striType fileName;
    const const_cstriType extensions[] = {".fdb", ".gdb", ""};
    unsigned int idx;
    loginRecord loginData;
    isc_db_handle db_handle;
    isc_tr_handle trans_handle = 0; /* Set to zero for isc_start_transaction(). */
    ISC_STATUS status_vector[20];
    errInfoType err_info = OKAY_NO_ERROR;
    dbType database;

  /* sqlOpenFire */
    logFunction(printf("sqlOpenFire(\"%s\", ",
                       striAsUnquotedCStri(host));
                printf(FMT_D ", \"%s\", ",
                       port, striAsUnquotedCStri(dbName));
                printf("\"%s\", ", striAsUnquotedCStri(user));
                printf("\"%s\")\n", striAsUnquotedCStri(password)););
    if (!findDll()) {
      logError(printf("sqlOpenFire: findDll() failed\n"););
      err_info = DATABASE_ERROR;
      database = NULL;
    } else {
      fileName = cmdToOsPath(dbName);
      if (fileName == NULL) {
        database = NULL;
      } else {
        loginData.fileName8 = stri_to_cstri8_buf(fileName, &loginData.fileName8Length);
        if (unlikely(loginData.fileName8 == NULL)) {
          err_info = MEMORY_ERROR;
          database = NULL;
        } else {
          loginData.user8 = stri_to_cstri8_buf(user, &loginData.user8Length);
          if (unlikely(loginData.user8 == NULL)) {
            err_info = MEMORY_ERROR;
            database = NULL;
          } else {
            loginData.password8 = stri_to_cstri8_buf(password, &loginData.password8Length);
            if (unlikely(loginData.password8 == NULL)) {
              err_info = MEMORY_ERROR;
              database = NULL;
            } else {
              if (loginData.fileName8Length > 255 || loginData.user8Length > 255 ||
                  loginData.password8Length > 255) {
                logError(printf("sqlOpenFire: File name, user or password too long.\n"););
                err_info = RANGE_ERROR;
                database = NULL;
              } else {
                idx = 0;
                do {
                  err_info = doAttach(&loginData, extensions[idx], &db_handle);
                  idx++;
                } while (err_info == DATABASE_ERROR &&
                         idx < sizeof(extensions) / sizeof(cstriType));
                if (unlikely(err_info != OKAY_NO_ERROR)) {
                  database = NULL;
                } else {
                  if ((isc_start_transaction(status_vector,
                                             &trans_handle,
                                             1,
                                             &db_handle,
                                             (unsigned short) sizeof(isc_tbp),
                                             isc_tbp),
                              status_vector[0] == 1 && status_vector[1] != 0)) {
                    setDbErrorMsg("sqlOpenFire", "isc_start_transaction",
                                  status_vector);
                    logError(printf("sqlOpenFire: isc_start_transaction error:\n%s\n",
                                    dbError.message););
                    err_info = DATABASE_ERROR;
                    isc_detach_database(status_vector, &db_handle);
                    database = NULL;
                  } else if (unlikely(!setupFuncTable() ||
                                      !ALLOC_RECORD2(database, dbRecord,
                                                     count.database, count.database_bytes))) {
                    err_info = MEMORY_ERROR;
                    isc_rollback_transaction(status_vector, &trans_handle);
                    isc_detach_database(status_vector, &db_handle);
                    database = NULL;
                  } else {
                    memset(database, 0, sizeof(dbRecord));
                    database->usage_count = 1;
                    database->sqlFunc = sqlFunc;
                    database->driver = DB_CATEGORY_FIREBIRD;
                    database->connection = db_handle;
                    database->trans_handle = trans_handle;
                  } /* if */
                } /* if */
              } /* if */
              free_cstri8(loginData.password8, password);
            } /* if */
            free_cstri8(loginData.user8, user);
          } /* if */
          free_cstri8(loginData.fileName8, fileName);
        } /* if */
        strDestr(fileName);
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("sqlOpenFire --> " FMT_U_MEM "\n",
                       (memSizeType) database););
    return (databaseType) database;
  } /* sqlOpenFire */

#else



databaseType sqlOpenFire (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  { /* sqlOpenFire */
    logError(printf("sqlOpenFire(\"%s\", ",
                    striAsUnquotedCStri(host));
             printf(FMT_D ", \"%s\", ",
                    port, striAsUnquotedCStri(dbName));
             printf("\"%s\", ", striAsUnquotedCStri(user));
             printf("\"%s\"): Firebird/InterBase driver not present.\n",
                    striAsUnquotedCStri(password)););
    raise_error(RANGE_ERROR);
    return NULL;
  } /* sqlOpenFire */

#endif
