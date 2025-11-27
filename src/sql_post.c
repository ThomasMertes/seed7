/********************************************************************/
/*                                                                  */
/*  sql_post.c    Database access functions for PostgreSQL.         */
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
/*  File: seed7/src/sql_post.c                                      */
/*  Changes: 2014, 2015, 2017 - 2020  Thomas Mertes                 */
/*  Content: Database access functions for PostgreSQL.              */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "locale.h"
#include "time.h"
#include "limits.h"
#include "ctype.h"
#if SOCKET_LIB == UNIX_SOCKETS
#include "netinet/in.h"
#elif SOCKET_LIB == WINSOCK_SOCKETS
#include "winsock2.h"
#endif
#ifdef POSTGRESQL_INCLUDE
#include POSTGRESQL_INCLUDE
#ifdef POSTGRESQL_PG_TYPE_H
#include POSTGRESQL_PG_TYPE_H
#endif
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
#include "big_drv.h"
#include "rtl_err.h"
#include "dll_drv.h"
#include "sql_base.h"
#include "sql_drv.h"

#ifdef POSTGRESQL_INCLUDE


typedef struct {
    int ndigits; /* number of digits in digits[] - can be 0! */
    int weight;  /* weight of first digit */
    int rscale;  /* result scale */
    int dscale;  /* display scale */
    int sign;    /* NUMERIC_POS, NUMERIC_NEG, or NUMERIC_NAN */
    const unsigned char *digits; /* decimal digits */
  } numeric;

typedef struct {
    uintType     usage_count;
    boolType     isOpen;
    sqlFuncType  sqlFunc;
    int          driver;
    int          dbCategory;
    PGconn      *connection;
    boolType     integerDatetimes;
    uintType     nextStmtNum;
    boolType     autoCommit;
    floatType    moneyDenominator;
  } dbRecordPost, *dbType;

typedef struct {
    cstriType    buffer;
    boolType     bound;
  } bindDataRecordPost, *bindDataType;

#define STMT_NAME_PREFIX "prepstat_"
#define STMT_NAME_BUFFER_SIZE STRLEN(STMT_NAME_PREFIX) + \
                              UINTTYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN

typedef struct {
    uintType       usage_count;
    dbType         db;
    sqlFuncType    sqlFunc;
    boolType       integerDatetimes;
    boolType       implicitCommit;
    uintType       stmtNum;
    char           stmtName[STMT_NAME_BUFFER_SIZE];
    memSizeType    param_array_size;
    bindDataType   param_array;
    Oid           *paramTypes;
    cstriType     *paramValues;
    int           *paramLengths;
    int           *paramFormats;
    memSizeType    result_column_count;
    boolType       executeSuccessful;
    PGresult      *execute_result;
    ExecStatusType execute_status;
    boolType       fetchOkay;
    int            num_tuples;
    int            fetch_index;
    boolType       increment_index;
  } preparedStmtRecordPost, *preparedStmtType;

static sqlFuncType sqlFunc = NULL;

#define DEFAULT_PORT 5432
#define MAX_BIND_VAR_SIZE 5
#define MIN_BIND_VAR_NUM 1
#define MAX_BIND_VAR_NUM 9999
/* Seconds between 1970-01-01 and 2000-01-01 */
#define SECONDS_FROM_1970_TO_2000 INT64_SUFFIX(946684800)
#define DEFAULT_DECIMAL_SCALE 1000
#define SHOW_DETAILS 0


#ifdef POSTGRESQL_DLL

#ifndef CDECL
#if defined(_WIN32) && HAS_CDECL
#define CDECL __cdecl
#else
#define CDECL
#endif
#endif

typedef void (CDECL *tp_PQclear) (PGresult *res);
typedef PGresult *(CDECL *tp_PQdescribePrepared) (PGconn *conn, const char *stmt);
typedef char *(CDECL *tp_PQerrorMessage) (const PGconn *conn);
typedef PGresult *(CDECL *tp_PQexec) (PGconn *conn, const char *query);
typedef PGresult *(CDECL *tp_PQexecPrepared) (PGconn *conn,
                                              const char *stmtName,
                                              int nParams,
                                              const char *const * paramValues,
                                              const int *paramLengths,
                                              const int *paramFormats,
                                              int resultFormat);
typedef void (CDECL *tp_PQfinish) (PGconn *conn);
typedef char *(CDECL *tp_PQfname) (const PGresult *res, int field_num);
typedef Oid (CDECL *tp_PQftype) (const PGresult *res, int field_num);
typedef int (CDECL *tp_PQgetisnull) (const PGresult *res, int tup_num, int field_num);
typedef int (CDECL *tp_PQgetlength) (const PGresult *res, int tup_num, int field_num);
typedef char *(CDECL *tp_PQgetvalue) (const PGresult *res, int tup_num, int field_num);
typedef int (CDECL *tp_PQnfields) (const PGresult *res);
typedef int (CDECL *tp_PQnparams) (const PGresult *res);
typedef int (CDECL *tp_PQntuples) (const PGresult *res);
typedef const char *(CDECL *tp_PQparameterStatus) (const PGconn *conn, const char *paramName);
typedef Oid (CDECL *tp_PQparamtype) (const PGresult *res, int param_num);
typedef PGresult *(CDECL *tp_PQprepare) (PGconn *conn, const char *stmtName,
                                         const char *query, int nParams,
                                         const Oid *paramTypes);
typedef char *(CDECL *tp_PQresStatus) (ExecStatusType status);
typedef char *(CDECL *tp_PQresultErrorMessage) (const PGresult *res);
typedef ExecStatusType (CDECL *tp_PQresultStatus) (const PGresult *res);
typedef int (CDECL *tp_PQsetClientEncoding) (PGconn *conn, const char *encoding);
typedef PGconn *(CDECL *tp_PQsetdbLogin) (const char *pghost, const char *pgport,
                                          const char *pgoptions, const char *pgtty,
                                          const char *dbName,
                                          const char *login, const char *pwd);
typedef ConnStatusType (CDECL *tp_PQstatus) (const PGconn *conn);

static tp_PQclear              ptr_PQclear;
static tp_PQdescribePrepared   ptr_PQdescribePrepared;
static tp_PQerrorMessage       ptr_PQerrorMessage;
static tp_PQexec               ptr_PQexec;
static tp_PQexecPrepared       ptr_PQexecPrepared;
static tp_PQfinish             ptr_PQfinish;
static tp_PQfname              ptr_PQfname;
static tp_PQftype              ptr_PQftype;
static tp_PQgetisnull          ptr_PQgetisnull;
static tp_PQgetlength          ptr_PQgetlength;
static tp_PQgetvalue           ptr_PQgetvalue;
static tp_PQnfields            ptr_PQnfields;
static tp_PQnparams            ptr_PQnparams;
static tp_PQntuples            ptr_PQntuples;
static tp_PQparameterStatus    ptr_PQparameterStatus;
static tp_PQparamtype          ptr_PQparamtype;
static tp_PQprepare            ptr_PQprepare;
static tp_PQresStatus          ptr_PQresStatus;
static tp_PQresultErrorMessage ptr_PQresultErrorMessage;
static tp_PQresultStatus       ptr_PQresultStatus;
static tp_PQsetClientEncoding  ptr_PQsetClientEncoding;
static tp_PQsetdbLogin         ptr_PQsetdbLogin;
static tp_PQstatus             ptr_PQstatus;

#define PQclear              ptr_PQclear
#define PQdescribePrepared   ptr_PQdescribePrepared
#define PQerrorMessage       ptr_PQerrorMessage
#define PQexec               ptr_PQexec
#define PQexecPrepared       ptr_PQexecPrepared
#define PQfinish             ptr_PQfinish
#define PQfname              ptr_PQfname
#define PQftype              ptr_PQftype
#define PQgetisnull          ptr_PQgetisnull
#define PQgetlength          ptr_PQgetlength
#define PQgetvalue           ptr_PQgetvalue
#define PQnfields            ptr_PQnfields
#define PQnparams            ptr_PQnparams
#define PQntuples            ptr_PQntuples
#define PQparameterStatus    ptr_PQparameterStatus
#define PQparamtype          ptr_PQparamtype
#define PQprepare            ptr_PQprepare
#define PQresStatus          ptr_PQresStatus
#define PQresultErrorMessage ptr_PQresultErrorMessage
#define PQresultStatus       ptr_PQresultStatus
#define PQsetClientEncoding  ptr_PQsetClientEncoding
#define PQsetdbLogin         ptr_PQsetdbLogin
#define PQstatus             ptr_PQstatus



static void loadBaseDlls (void)

  { /* loadBaseDlls */
#ifdef LIBINTL_DLL
    {
      const char *libIntlDllList[] = { LIBINTL_DLL };
      unsigned int pos;
      boolType found = FALSE;

      for (pos = 0; pos < sizeof(libIntlDllList) / sizeof(char *) && !found; pos++) {
        found = dllOpen(libIntlDllList[pos]) != NULL;
      } /* for */
    }
#endif
#ifdef LIBEAY32_DLL
    {
      const char *libeay32DllList[] = { LIBEAY32_DLL };
      unsigned int pos;
      boolType found = FALSE;

      for (pos = 0; pos < sizeof(libeay32DllList) / sizeof(char *) && !found; pos++) {
        found = dllOpen(libeay32DllList[pos]) != NULL;
      } /* for */
    }
#endif
#ifdef LIBCRYPTO_DLL
    {
      const char *libcryptoDllList[] = { LIBCRYPTO_DLL };
      unsigned int pos;
      boolType found = FALSE;

      for (pos = 0; pos < sizeof(libcryptoDllList) / sizeof(char *) && !found; pos++) {
        found = dllOpen(libcryptoDllList[pos]) != NULL;
      } /* for */
    }
#endif
#ifdef LIBSSL_DLL
    {
      const char *libsslDllList[] = { LIBSSL_DLL };
      unsigned int pos;
      boolType found = FALSE;

      for (pos = 0; pos < sizeof(libsslDllList) / sizeof(char *) && !found; pos++) {
        found = dllOpen(libsslDllList[pos]) != NULL;
      } /* for */
    }
#endif
  } /* loadBaseDlls */



static boolType setupDll (const char *dllName)

  {
    static void *dbDll = NULL;

  /* setupDll */
    logFunction(printf("setupDll(\"%s\")\n", dllName););
    if (dbDll == NULL) {
      loadBaseDlls();
      dbDll = dllOpen(dllName);
      if (dbDll != NULL) {
        if ((PQclear              = (tp_PQclear)              dllFunc(dbDll, "PQclear"))              == NULL ||
            (PQdescribePrepared   = (tp_PQdescribePrepared)   dllFunc(dbDll, "PQdescribePrepared"))   == NULL ||
            (PQerrorMessage       = (tp_PQerrorMessage)       dllFunc(dbDll, "PQerrorMessage"))       == NULL ||
            (PQexec               = (tp_PQexec)               dllFunc(dbDll, "PQexec"))               == NULL ||
            (PQexecPrepared       = (tp_PQexecPrepared)       dllFunc(dbDll, "PQexecPrepared"))       == NULL ||
            (PQfinish             = (tp_PQfinish)             dllFunc(dbDll, "PQfinish"))             == NULL ||
            (PQfname              = (tp_PQfname)              dllFunc(dbDll, "PQfname"))              == NULL ||
            (PQftype              = (tp_PQftype)              dllFunc(dbDll, "PQftype"))              == NULL ||
            (PQgetisnull          = (tp_PQgetisnull)          dllFunc(dbDll, "PQgetisnull"))          == NULL ||
            (PQgetlength          = (tp_PQgetlength)          dllFunc(dbDll, "PQgetlength"))          == NULL ||
            (PQgetvalue           = (tp_PQgetvalue)           dllFunc(dbDll, "PQgetvalue"))           == NULL ||
            (PQnfields            = (tp_PQnfields)            dllFunc(dbDll, "PQnfields"))            == NULL ||
            (PQnparams            = (tp_PQnparams)            dllFunc(dbDll, "PQnparams"))            == NULL ||
            (PQntuples            = (tp_PQntuples)            dllFunc(dbDll, "PQntuples"))            == NULL ||
            (PQparameterStatus    = (tp_PQparameterStatus)    dllFunc(dbDll, "PQparameterStatus"))    == NULL ||
            (PQparamtype          = (tp_PQparamtype)          dllFunc(dbDll, "PQparamtype"))          == NULL ||
            (PQprepare            = (tp_PQprepare)            dllFunc(dbDll, "PQprepare"))            == NULL ||
            (PQresStatus          = (tp_PQresStatus)          dllFunc(dbDll, "PQresStatus"))          == NULL ||
            (PQresultErrorMessage = (tp_PQresultErrorMessage) dllFunc(dbDll, "PQresultErrorMessage")) == NULL ||
            (PQresultStatus       = (tp_PQresultStatus)       dllFunc(dbDll, "PQresultStatus"))       == NULL ||
            (PQsetClientEncoding  = (tp_PQsetClientEncoding)  dllFunc(dbDll, "PQsetClientEncoding"))  == NULL ||
            (PQsetdbLogin         = (tp_PQsetdbLogin)         dllFunc(dbDll, "PQsetdbLogin"))         == NULL ||
            (PQstatus             = (tp_PQstatus)             dllFunc(dbDll, "PQstatus"))             == NULL) {
          dbDll = NULL;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setupDll --> %d\n", dbDll != NULL););
    return dbDll != NULL;
  } /* setupDll */



static boolType findDll (void)

  {
    const char *dllList[] = { POSTGRESQL_DLL };
    unsigned int pos;
    boolType found = FALSE;

  /* findDll */
    for (pos = 0; pos < sizeof(dllList) / sizeof(char *) && !found; pos++) {
      found = setupDll(dllList[pos]);
    } /* for */
    if (!found) {
      dllErrorMessage("sqlOpenPost", "findDll", dllList,
                      sizeof(dllList) / sizeof(char *));
    } /* if */
    return found;
  } /* findDll */

#else

#define findDll() TRUE

#endif



#undef htonll
#undef ntohll

#if BIG_ENDIAN_INTTYPE

#define htonll(n) (n)
#define ntohll(n) (n)
#define htonf(n) (n)
#define ntohf(n) (n)
#define htond(n) (n)
#define ntohd(n) (n)
#if SOCKET_LIB == NO_SOCKETS
#define htons(n) (n)
#define ntohs(n) (n)
#define htonl(n) (n)
#define ntohl(n) (n)
#endif

#else

#define htonll(n) swapUInt64(n)
#define ntohll(n) swapUInt64(n)
#define htonf(n) swap32float(n)
#define ntohf(n) swap32float(n)
#define htond(n) swap64double(n)
#define ntohd(n) swap64double(n)
#if SOCKET_LIB == NO_SOCKETS
#define htons(n) swapUInt16(n)
#define ntohs(n) swapUInt16(n)
#define htonl(n) swapUInt32(n)
#define ntohl(n) swapUInt32(n)

static inline uint16Type swapUInt16 (uint16Type n)
  {
    return (uint16Type) (((n & 0xFF) << 8) | ((n & 0xFF00) >> 8));
  }

static inline uint32Type swapUInt32 (uint32Type n)
  {
    return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) |
      ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
  }
#endif

static inline uint64Type swapUInt64 (uint64Type n)
  {
    return ((n & 0xFF) << 56) | ((n & 0xFF00) << 40) |
           ((n & 0xFF0000) << 24) | ((n & 0xFF000000) << 8) |
           ((n & 0xFF00000000) >> 8) | ((n & 0xFF0000000000) >> 24) |
           ((n & 0xFF000000000000) >> 40) | ((n & 0xFF00000000000000) >> 56);
  }

static inline float swap32float (float n)
  {
    union {
      uint32Type i;
      float f;
    } flt2int;

    flt2int.f = n;
    flt2int.i = ntohl(flt2int.i);
    return flt2int.f;
  }

static inline double swap64double (double n)
  {
    union {
      uint64Type i;
      double f;
    } flt2int;

    flt2int.f = n;
    flt2int.i = ntohll(flt2int.i);
    return flt2int.f;
  }

#endif



static void sqlClose (databaseType database);



static void setDbErrorMsg (const char *funcName, const char *dbFuncName,
    PGconn *connection)

  { /* setDbErrorMsg */
    dbError.funcName = funcName;
    dbError.dbFuncName = dbFuncName;
    dbError.errorCode = 0;
    snprintf(dbError.message, DB_ERR_MESSAGE_SIZE, "%s",
             PQerrorMessage(connection));
  } /* setDbErrorMsg */



static errInfoType doExecSql (PGconn *connection, const char *query,
    errInfoType err_info)

  {
    PGresult *execResult;

  /* doExecSql */
    logFunction(printf("doExecSql(" FMT_U_MEM ", \"%s\", %d)\n",
                       (memSizeType) connection, query, err_info););
    if (likely(err_info == OKAY_NO_ERROR)) {
      execResult = PQexec(connection, query);
      if (unlikely(execResult == NULL)) {
        err_info = MEMORY_ERROR;
      } else {
        if (PQresultStatus(execResult) != PGRES_COMMAND_OK) {
          setDbErrorMsg("doExecSql", "PQexec", connection);
          logError(printf("doExecSql: PQexec(" FMT_U_MEM ", \"%s\") "
                          "returns a status of %s:\n%s",
                          (memSizeType) connection, query,
                          PQresStatus(PQresultStatus(execResult)),
                          dbError.message););
          err_info = DATABASE_ERROR;
        } /* if */
        PQclear(execResult);
      } /* if */
    } /* if */
    logFunction(printf("doExecSql --> %d\n", err_info););
    return err_info;
  } /* doExecSql */



static boolType implicitCommit (const_cstriType query)

  {
    const char *explicitCommit[] = {
      /* DML */ "SELECT", "INSERT", "UPDATE", "DELETE", "MERGE",
      /* TCS */ "COMMIT", "ROLLBACK", "SAVEPOINT", "BEGIN"};
    const_cstriType startPos;
    const_cstriType beyond;
    const_cstriType pos;
    char keyword[20];
    memSizeType idx;
    boolType implicitCommit = TRUE;

  /* implicitCommit */
    logFunction(printf("implicitCommit(\"%s\")\n", query););
    startPos = query;
    while (*startPos == ' ' || *startPos == '\t' ||
           *startPos == '\n' || *startPos == '\r') {
      startPos++;
    } /* while */
    beyond = startPos;
    while (isalpha(*beyond)) {
      beyond++;
    } /* while */
    if ((memSizeType) (beyond - startPos) < sizeof(keyword)) {
      for (pos = startPos; pos != beyond; pos++) {
        keyword[pos - startPos] = (char) toupper(*pos);
      } /* for */
      keyword[beyond - startPos] = '\0';
      for (idx = 0; idx < sizeof(explicitCommit) / sizeof(char *) &&
           implicitCommit; idx++) {
        if (strcmp(keyword, explicitCommit[idx]) == 0) {
          implicitCommit = FALSE;
        } /* if */
      } /* for */
    } /* if */
    logFunction(printf("implicitCommit --> %d\n", implicitCommit););
    return implicitCommit;
  } /* implicitCommit */



static PGresult *PQdeallocate (PGconn *conn, const char *stmtName)

  {
    const char deallocateCommand[] = "DEALLOCATE ";
    memSizeType stmtNameLength;
    memSizeType length;
    char *command;
    PGresult *deallocate_result;

  /* PQdeallocate */
    logFunction(printf("PQdeallocate(" FMT_X_MEM ", %s)\n",
                       (memSizeType) conn, stmtName););
    stmtNameLength = strlen(stmtName);
    length = STRLEN(deallocateCommand) + stmtNameLength;
    if (unlikely(!ALLOC_CSTRI(command, length))) {
      deallocate_result = NULL;
    } else {
      memcpy(command, deallocateCommand, STRLEN(deallocateCommand));
      memcpy(&command[STRLEN(deallocateCommand)], stmtName, stmtNameLength);
      command[length] = '\0';
      deallocate_result = PQexec(conn, command);
      UNALLOC_CSTRI(command, length);
    } /* if */
    return deallocate_result;
  } /* PQdeallocate */



/**
 *  Closes a database and frees the memory used by it.
 */
static void freeDatabase (databaseType database)

  {
    dbType db;

  /* freeDatabase */
    logFunction(printf("freeDatabase(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    if (database->isOpen) {
      sqlClose(database);
    } /* if */
    db = (dbType) database;
    FREE_RECORD2(db, dbRecordPost, count.database, count.database_bytes);
    logFunction(printf("freeDatabase -->\n"););
  } /* freeDatabase */



/**
 *  Closes a prepared statement and frees the memory used by it.
 */
static void freePreparedStmt (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    memSizeType pos;
    static PGresult *deallocate_result;

  /* freePreparedStmt */
    logFunction(printf("freePreparedStmt(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (preparedStmt->param_array != NULL) {
      for (pos = 0; pos < preparedStmt->param_array_size; pos++) {
        free(preparedStmt->param_array[pos].buffer);
      } /* for */
      FREE_TABLE(preparedStmt->param_array, bindDataRecordPost, preparedStmt->param_array_size);
    } /* if */
    if (preparedStmt->paramTypes != NULL) {
      FREE_TABLE(preparedStmt->paramTypes, Oid, preparedStmt->param_array_size);
    } /* if */
    if (preparedStmt->paramValues != NULL) {
      FREE_TABLE(preparedStmt->paramValues, cstriType, preparedStmt->param_array_size);
    } /* if */
    if (preparedStmt->paramLengths != NULL) {
      FREE_TABLE(preparedStmt->paramLengths, int, preparedStmt->param_array_size);
    } /* if */
    if (preparedStmt->paramFormats != NULL) {
      FREE_TABLE(preparedStmt->paramFormats, int, preparedStmt->param_array_size);
    } /* if */
    if (preparedStmt->execute_result != NULL) {
      PQclear(preparedStmt->execute_result);
    } /* if */
    deallocate_result = PQdeallocate(preparedStmt->db->connection, preparedStmt->stmtName);
    if (unlikely(deallocate_result != NULL)) {
      /* Ignore possible errors. */
      PQclear(deallocate_result);
    } /* if */
    if (preparedStmt->db != NULL &&
        preparedStmt->db->usage_count != 0) {
      preparedStmt->db->usage_count--;
      if (preparedStmt->db->usage_count == 0) {
        logMessage(printf("FREE " FMT_U_MEM "\n", (memSizeType) preparedStmt->db););
        freeDatabase((databaseType) preparedStmt->db);
      } /* if */
    } /* if */
    FREE_RECORD2(preparedStmt, preparedStmtRecordPost,
                 count.prepared_stmt, count.prepared_stmt_bytes);
    logFunction(printf("freePreparedStmt -->\n"););
  } /* freePreparedStmt */



#if ANY_LOG_ACTIVE
static const char *nameOfBufferType (Oid buffer_type)

  {
    static char buffer[UINT_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    const char *typeName;

  /* nameOfBufferType */
    logFunction(printf("nameOfBufferType(%d)\n", buffer_type););
    switch (buffer_type) {
      case BOOLOID:             typeName = "BOOLOID"; break;
      case BYTEAOID:            typeName = "BYTEAOID"; break;
      case CHAROID:             typeName = "CHAROID"; break;
      case NAMEOID:             typeName = "NAMEOID"; break;
      case INT8OID:             typeName = "INT8OID"; break;
      case INT2OID:             typeName = "INT2OID"; break;
      case INT2VECTOROID:       typeName = "INT2VECTOROID"; break;
      case INT4OID:             typeName = "INT4OID"; break;
      case REGPROCOID:          typeName = "REGPROCOID"; break;
      case TEXTOID:             typeName = "TEXTOID"; break;
      case OIDOID:              typeName = "OIDOID"; break;
      case TIDOID:              typeName = "TIDOID"; break;
      case XIDOID:              typeName = "XIDOID"; break;
      case CIDOID:              typeName = "CIDOID"; break;
      case OIDVECTOROID:        typeName = "OIDVECTOROID"; break;
      case XMLOID:              typeName = "XMLOID"; break;
#ifdef PGNODETREEOID
      case PGNODETREEOID:       typeName = "PGNODETREEOID"; break;
#endif
      case POINTOID:            typeName = "POINTOID"; break;
      case LSEGOID:             typeName = "LSEGOID"; break;
      case PATHOID:             typeName = "PATHOID"; break;
      case BOXOID:              typeName = "BOXOID"; break;
      case POLYGONOID:          typeName = "POLYGONOID"; break;
      case LINEOID:             typeName = "LINEOID"; break;
      case FLOAT4OID:           typeName = "FLOAT4OID"; break;
      case FLOAT8OID:           typeName = "FLOAT8OID"; break;
#ifdef ABSTIMEOID
      case ABSTIMEOID:          typeName = "ABSTIMEOID"; break;
#endif
#ifdef RELTIMEOID
      case RELTIMEOID:          typeName = "RELTIMEOID"; break;
#endif
#ifdef TINTERVALOID
      case TINTERVALOID:        typeName = "TINTERVALOID"; break;
#endif
      case UNKNOWNOID:          typeName = "UNKNOWNOID"; break;
      case CIRCLEOID:           typeName = "CIRCLEOID"; break;
      case CASHOID:             typeName = "CASHOID"; break;
      case MACADDROID:          typeName = "MACADDROID"; break;
      case INETOID:             typeName = "INETOID"; break;
      case CIDROID:             typeName = "CIDROID"; break;
#ifdef INT2ARRAYOID
      case INT2ARRAYOID:        typeName = "INT2ARRAYOID"; break;
#endif
      case INT4ARRAYOID:        typeName = "INT4ARRAYOID"; break;
      case TEXTARRAYOID:        typeName = "TEXTARRAYOID"; break;
#ifdef OIDARRAYOID
      case OIDARRAYOID:         typeName = "OIDARRAYOID"; break;
#endif
      case FLOAT4ARRAYOID:      typeName = "FLOAT4ARRAYOID"; break;
      case ACLITEMOID:          typeName = "ACLITEMOID"; break;
      case CSTRINGARRAYOID:     typeName = "CSTRINGARRAYOID"; break;
      case BPCHAROID:           typeName = "BPCHAROID"; break;
      case VARCHAROID:          typeName = "VARCHAROID"; break;
      case DATEOID:             typeName = "DATEOID"; break;
      case TIMEOID:             typeName = "TIMEOID"; break;
      case TIMESTAMPOID:        typeName = "TIMESTAMPOID"; break;
      case TIMESTAMPTZOID:      typeName = "TIMESTAMPTZOID"; break;
      case INTERVALOID:         typeName = "INTERVALOID"; break;
      case TIMETZOID:           typeName = "TIMETZOID"; break;
      case BITOID:              typeName = "BITOID"; break;
      case VARBITOID:           typeName = "VARBITOID"; break;
      case NUMERICOID:          typeName = "NUMERICOID"; break;
      case REFCURSOROID:        typeName = "REFCURSOROID"; break;
      case REGPROCEDUREOID:     typeName = "REGPROCEDUREOID"; break;
      case REGOPEROID:          typeName = "REGOPEROID"; break;
      case REGOPERATOROID:      typeName = "REGOPERATOROID"; break;
      case REGCLASSOID:         typeName = "REGCLASSOID"; break;
      case REGTYPEOID:          typeName = "REGTYPEOID"; break;
      case REGTYPEARRAYOID:     typeName = "REGTYPEARRAYOID"; break;
      case TSVECTOROID:         typeName = "TSVECTOROID"; break;
      case GTSVECTOROID:        typeName = "GTSVECTOROID"; break;
      case TSQUERYOID:          typeName = "TSQUERYOID"; break;
      case REGCONFIGOID:        typeName = "REGCONFIGOID"; break;
      case REGDICTIONARYOID:    typeName = "REGDICTIONARYOID"; break;
      case RECORDOID:           typeName = "RECORDOID"; break;
      case RECORDARRAYOID:      typeName = "RECORDARRAYOID"; break;
      case CSTRINGOID:          typeName = "CSTRINGOID"; break;
      case ANYOID:              typeName = "ANYOID"; break;
      case ANYARRAYOID:         typeName = "ANYARRAYOID"; break;
      case VOIDOID:             typeName = "VOIDOID"; break;
      case TRIGGEROID:          typeName = "TRIGGEROID"; break;
      case LANGUAGE_HANDLEROID: typeName = "LANGUAGE_HANDLEROID"; break;
      case INTERNALOID:         typeName = "INTERNALOID"; break;
#ifdef OPAQUEOID
      case OPAQUEOID:           typeName = "OPAQUEOID"; break;
#endif
      case ANYELEMENTOID:       typeName = "ANYELEMENTOID"; break;
      case ANYNONARRAYOID:      typeName = "ANYNONARRAYOID"; break;
      case ANYENUMOID:          typeName = "ANYENUMOID"; break;
#ifdef FDW_HANDLEROID
      case FDW_HANDLEROID:      typeName = "FDW_HANDLEROID"; break;
#endif
      default:
        sprintf(buffer, "%u", buffer_type);
        typeName = buffer;
        break;
    } /* switch */
    logFunction(printf("nameOfBufferType --> %s\n", typeName););
    return typeName;
  } /* nameOfBufferType */
#endif



/**
 *  Process the bind variables in a statement string.
 *  Literals and comments are processed to avoid the misinterpretation
 *  of question marks (?).
 */
static striType processStatementStri (const const_striType sqlStatementStri,
    errInfoType *err_info)

  {
    memSizeType pos = 0;
    strElemType ch;
    strElemType delimiter;
    memSizeType destPos = 0;
    unsigned int varNum = MIN_BIND_VAR_NUM;
    striType processed;

  /* processStatementStri */
    logFunction(printf("processStatementStri(\"%s\")\n",
                       striAsUnquotedCStri(sqlStatementStri)););
    if (unlikely(sqlStatementStri->size > MAX_STRI_LEN / MAX_BIND_VAR_SIZE ||
                 !ALLOC_STRI_SIZE_OK(processed, sqlStatementStri->size * MAX_BIND_VAR_SIZE))) {
      *err_info = MEMORY_ERROR;
      processed = NULL;
    } else {
      while (pos < sqlStatementStri->size && *err_info == OKAY_NO_ERROR) {
        ch = sqlStatementStri->mem[pos];
        if (ch == '?') {
          if (varNum > MAX_BIND_VAR_NUM) {
            logError(printf("processStatementStri: Too many variables\n"););
            *err_info = RANGE_ERROR;
            FREE_STRI2(processed, sqlStatementStri->size * MAX_BIND_VAR_SIZE);
            processed = NULL;
          } else {
            processed->mem[destPos++] = '$';
            if (varNum >= 1000) {
              processed->mem[destPos++] = '0' + ( varNum / 1000);
            } /* if */
            if (varNum >= 100) {
              processed->mem[destPos++] = '0' + ((varNum /  100) % 10);
            } /* if */
            if (varNum >= 10) {
              processed->mem[destPos++] = '0' + ((varNum /   10) % 10);
            } /* if */
            processed->mem[destPos++] = '0' + ( varNum         % 10);
            varNum++;
          } /* if */
          pos++;
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
      processed->size = destPos;
    } /* if */
    logFunction(printf("processStatementStri --> \"%s\"\n",
                       striAsUnquotedCStri(processed)););
    return processed;
  } /* processStatementStri */



static errInfoType setupParameterColumn (preparedStmtType preparedStmt,
    int param_index, bindDataType param)

  {
    memSizeType paramLength = 0;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupParameterColumn */
    switch (preparedStmt->paramTypes[param_index]) {
      case INT2OID:
        paramLength = sizeof(int16Type);
        break;
      case INT4OID:
        paramLength = sizeof(int32Type);
        break;
      case INT8OID:
        paramLength = sizeof(int64Type);
        break;
      case FLOAT4OID:
        paramLength = sizeof(float);
        break;
      case FLOAT8OID:
        paramLength = sizeof(double);
        break;
      case DATEOID:
        paramLength = sizeof(int32Type);
        break;
      case TIMEOID:
        if (preparedStmt->integerDatetimes) {
          paramLength = sizeof(int64Type);
        } else {
          paramLength = sizeof(double);
        } /* if */
        break;
      case TIMETZOID:
        if (preparedStmt->integerDatetimes) {
          paramLength = sizeof(int64Type) + sizeof(int32Type);
        } else {
          paramLength = sizeof(double) + sizeof(int32Type);
        } /* if */
        break;
      case TIMESTAMPOID:
        if (preparedStmt->integerDatetimes) {
          paramLength = sizeof(int64Type);
        } else {
          paramLength = sizeof(double);
        } /* if */
        break;
      case TIMESTAMPTZOID:
        if (preparedStmt->integerDatetimes) {
          paramLength = sizeof(int64Type);
        } else {
          paramLength = sizeof(double);
        } /* if */
        break;
      case INTERVALOID:
        if (preparedStmt->integerDatetimes) {
          paramLength = sizeof(int64Type) + 2 * sizeof(int32Type);
        } else {
          paramLength = sizeof(double) + 2 * sizeof(int32Type);
        } /* if */
        break;
    } /* switch */
    if (paramLength != 0) {
      param->buffer = (cstriType) malloc(paramLength);
      if (unlikely(param->buffer == NULL)) {
        err_info = MEMORY_ERROR;
      } else {
        preparedStmt->paramValues[param_index] = NULL;
        preparedStmt->paramLengths[param_index] = (int) paramLength;
        preparedStmt->paramFormats[param_index] = 1;
      } /* if */
    } else {
      param->buffer = NULL;
      preparedStmt->paramValues[param_index] = NULL;
      preparedStmt->paramLengths[param_index] = 0;
      preparedStmt->paramFormats[param_index] = 0;
    } /* if */
    return err_info;
  } /* setupParameterColumn */



static errInfoType setupParameters (PGresult *describe_result, preparedStmtType preparedStmt)

  {
    int num_params;
    int param_index;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupParameters */
    logFunction(printf("setupParameters\n"););
    num_params = PQnparams(describe_result);
    if (unlikely(num_params < 0)) {
      dbInconsistent("setupParameters", "PQnparams");
      logError(printf("setupParameters: PQnparams returns negative number: %d\n",
                      num_params););
      err_info = DATABASE_ERROR;
    } else if (num_params == 0) {
      /* malloc(0) may return NULL, which would wrongly trigger a MEMORY_ERROR. */
      preparedStmt->param_array_size = 0;
      preparedStmt->param_array = NULL;
      preparedStmt->paramTypes = NULL;
      preparedStmt->paramValues = NULL;
      preparedStmt->paramLengths = NULL;
      preparedStmt->paramFormats = NULL;
    } else {
      preparedStmt->param_array_size = (memSizeType) num_params;
      if (unlikely(
          !ALLOC_TABLE(preparedStmt->param_array, bindDataRecordPost, preparedStmt->param_array_size) ||
          !ALLOC_TABLE(preparedStmt->paramTypes, Oid, preparedStmt->param_array_size) ||
          !ALLOC_TABLE(preparedStmt->paramValues, cstriType, preparedStmt->param_array_size) ||
          !ALLOC_TABLE(preparedStmt->paramLengths, int, preparedStmt->param_array_size) ||
          !ALLOC_TABLE(preparedStmt->paramFormats, int, preparedStmt->param_array_size))) {
        if (preparedStmt->param_array != NULL) {
          FREE_TABLE(preparedStmt->param_array, bindDataRecordPost, preparedStmt->param_array_size);
        } /* if */
        if (preparedStmt->paramTypes != NULL) {
          FREE_TABLE(preparedStmt->paramTypes, Oid, preparedStmt->param_array_size);
        } /* if */
        if (preparedStmt->paramValues != NULL) {
          FREE_TABLE(preparedStmt->paramValues, cstriType, preparedStmt->param_array_size);
        } /* if */
        if (preparedStmt->paramLengths != NULL) {
          FREE_TABLE(preparedStmt->paramLengths, int, preparedStmt->param_array_size);
        } /* if */
        if (preparedStmt->paramFormats != NULL) {
          FREE_TABLE(preparedStmt->paramFormats, int, preparedStmt->param_array_size);
        } /* if */
        preparedStmt->param_array_size = 0;
        preparedStmt->param_array = NULL;
        preparedStmt->paramTypes = NULL;
        preparedStmt->paramValues = NULL;
        preparedStmt->paramLengths = NULL;
        preparedStmt->paramFormats = NULL;
        err_info = MEMORY_ERROR;
      } else {
        memset(preparedStmt->param_array, 0,
               (memSizeType) num_params * sizeof(bindDataRecordPost));
        for (param_index = 0; param_index < num_params &&
             err_info == OKAY_NO_ERROR; param_index++) {
          preparedStmt->paramTypes[param_index] = PQparamtype(describe_result, param_index);
          /* printf("paramType: %s\n",
             nameOfBufferType(preparedStmt->paramTypes[param_index])); */
          err_info = setupParameterColumn(preparedStmt, param_index,
              &preparedStmt->param_array[param_index]);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setupParameters --> %d\n", err_info););
    return err_info;
  } /* setupParameters */



static errInfoType setupResult (PGresult *describe_result, preparedStmtType preparedStmt)

  {
    int num_columns;
    /* int column_index; */
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupResult */
    logFunction(printf("setupResult\n"););
    num_columns = PQnfields(describe_result);
    if (num_columns < 0) {
      dbInconsistent("setupResult", "PQnfields");
      logError(printf("setupResult: PQnfields returns negative number: %d\n",
                      num_columns););
      err_info = DATABASE_ERROR;
    } else {
      preparedStmt->result_column_count = (memSizeType) num_columns;
      /* for (column_index = 0; column_index < num_columns &&
           err_info == OKAY_NO_ERROR; column_index++) {
        printf("Column %d name: %s\n", column_index + 1, PQfname(describe_result, column_index));
        printf("Column %d type: %d\n", column_index + 1, PQftype(describe_result, column_index));
      } */
    } /* if */
    logFunction(printf("setupResult --> %d\n", err_info););
    return err_info;
  } /* setupResult */



static errInfoType setupParametersAndResult (preparedStmtType preparedStmt)

  {
    PGresult *describe_result;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupParametersAndResult */
    logFunction(printf("setupParametersAndResult\n"););
    describe_result = PQdescribePrepared(preparedStmt->db->connection, preparedStmt->stmtName);
    if (unlikely(describe_result == NULL)) {
      err_info = MEMORY_ERROR;
    } else {
      if (PQresultStatus(describe_result) != PGRES_COMMAND_OK) {
        setDbErrorMsg("setupParametersAndResult", "PQdescribePrepared", preparedStmt->db->connection);
        logError(printf("setupParametersAndResult: PQdescribePrepared returns a status of %s:\n%s",
                        PQresStatus(PQresultStatus(describe_result)),
                        dbError.message););
        err_info = DATABASE_ERROR;
      } else {
        err_info = setupParameters(describe_result, preparedStmt);
        if (likely(err_info == OKAY_NO_ERROR)) {
          err_info = setupResult(describe_result, preparedStmt);
        } /* if */
      } /* if */
      PQclear(describe_result);
    } /* if */
    logFunction(printf("setupParametersAndResult --> %d\n", err_info););
    return err_info;
  } /* setupParametersAndResult */



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



#if ANY_LOG_ACTIVE
#if SHOW_DETAILS
static void showNumeric (const unsigned char *buffer)

  {
    numeric numStruct;
    int idx;

  /* showNumeric */
    printf("NUMERICOID: %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
           buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7],
           buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[15], buffer[15]);
    numStruct.ndigits = 256 * buffer[0] + buffer[1];
    numStruct.weight  = (int16Type) (buffer[2] << 8 | buffer[3]);
    numStruct.rscale  = 256 * (buffer[4] & 0x3f) + buffer[5];
    numStruct.dscale  = 256 * (buffer[6] & 0x3f) + buffer[7];
    numStruct.sign    = (buffer[4] & 0x40) != 0;

    printf("ndigits: %d\n", numStruct.ndigits);
    printf("weight: %d\n",  numStruct.weight);
    printf("rscale: %d\n",  numStruct.rscale);
    printf("dscale: %d\n",  numStruct.dscale);
    printf("sign: %d\n",    numStruct.sign);

    for (idx = 0; idx < numStruct.ndigits; idx++) {
      printf("digit %d: %u\n", idx, 256 * buffer[8 + 2 * idx] + buffer[9 + 2 * idx]);
    } /* for */
  } /* showNumeric */

#else
#define showNumeric(buffer)
#endif
#endif



static cstriType getNumericAsCStri (numeric *numStruct)

  {
    unsigned int length;
    unsigned int idx;
    unsigned int fourDigits;
    unsigned int digitNum;
    unsigned int scale;
    cstriType decimal;

   /* getNumericAsCStri */
    if (numStruct->weight < numStruct->ndigits - 1) {
      scale = 4 * (unsigned int) (numStruct->ndigits - 1 - numStruct->weight);
      /* printf("getNumericAsCStri: 1 scale: %d\n", scale); */
      /* Provide space for sign, decimal point and a possible leading zero. */
      if (likely(ALLOC_CSTRI(decimal, 4 * (memSizeType) numStruct->ndigits + 3 + scale))) {
        length = 4 * (unsigned int) numStruct->ndigits + 1;
        /* printf("length: %u\n", length); */
        decimal[0] = numStruct->sign ? '-' : '+';
        for (idx = 0; idx < (unsigned int) numStruct->ndigits; idx++) {
          fourDigits = 256 * (unsigned int) numStruct->digits[2 * idx] +
                             (unsigned int) numStruct->digits[2 * idx + 1];
          for (digitNum = 4; digitNum >= 1; digitNum--) {
            decimal[4 * idx + digitNum] = (char) (fourDigits % 10 + '0');
            fourDigits /= 10;
          } /* for */
        } /* for */
        if (scale >= length - 1) {
          memmove(&decimal[scale - length + 4], &decimal[1], length - 1);
          decimal[1] = '0';
          decimal[2] = '.';
          memset(&decimal[3], '0', scale - length + 1);
          decimal[scale + 3] = '\0';
        } else {
          memmove(&decimal[length - scale + 1], &decimal[length - scale], scale);
          decimal[length - scale] = '.';
          decimal[length + 1] = '\0';
        } /* if */
      } /* if */
    } else {
      scale = 4 * (unsigned int) (numStruct->weight - (numStruct->ndigits - 1));
      /* printf("getNumericAsCStri: 2 scale: %d\n", scale); */
      /* Provide space for sign, decimal point and a possible trailing zero. */
      if (likely(ALLOC_CSTRI(decimal, 4 * (memSizeType) numStruct->ndigits + 3 + scale))) {
        length = 4 * (unsigned int) numStruct->ndigits + 1;
        /* printf("length: %u\n", length); */
        decimal[0] = numStruct->sign ? '-' : '+';
        for (idx = 0; idx < (unsigned int) numStruct->ndigits; idx++) {
          fourDigits = 256 * (unsigned int) numStruct->digits[2 * idx] +
                             (unsigned int) numStruct->digits[2 * idx + 1];
          for (digitNum = 4; digitNum >= 1; digitNum--) {
            decimal[4 * idx + digitNum] = (char) (fourDigits % 10 + '0');
            fourDigits /= 10;
          } /* for */
        } /* for */
        memset(&decimal[length], '0', scale);
        decimal[length + scale] = '.';
        decimal[length + scale + 1] = '0';
        decimal[length + scale + 2] = '\0';
      } /* if */
    } /* if */
    logFunction(printf("getNumericAsCStri --> %s\n",
                       decimal != NULL ? decimal : "NULL"));
    return decimal;
  } /* getNumericAsCStri */



static striType getNumericAsStri (numeric *numStruct)

  {
    unsigned int idx;
    unsigned int fourDigits;
    unsigned int digitNum;
    striType stri;

   /* getNumericAsStri */
    /* Provide space for sign. */
    if (likely(ALLOC_STRI_SIZE_OK(stri, 4 * (memSizeType) numStruct->ndigits + 1))) {
      stri->size = 4 * (memSizeType) numStruct->ndigits + 1;
      stri->mem[0] = numStruct->sign ? '-' : '+';
      for (idx = 0; idx < (unsigned int) numStruct->ndigits; idx++) {
        fourDigits = 256 * (unsigned int) numStruct->digits[2 * idx] +
                           (unsigned int) numStruct->digits[2 * idx + 1];
        for (digitNum = 4; digitNum >= 1; digitNum--) {
          stri->mem[4 * idx + digitNum] = (strElemType) (fourDigits % 10 + '0');
          fourDigits /= 10;
        } /* for */
      } /* for */
    } /* if */
    logFunction(printf("getNumericAsStri --> %s\n",
                       striAsUnquotedCStri(stri)));
    return stri;
  } /* getNumericAsStri */



static intType getNumericAsInt (const unsigned char *buffer)

  {
    numeric numStruct;
    striType stri;
    intType factor;
    intType intValue;

  /* getNumericAsInt */
    logFunction(printf("getNumericAsInt()\n");
                showNumeric(buffer););
    numStruct.ndigits = 256 * buffer[0] + buffer[1];
    numStruct.weight  = (int16Type) (buffer[2] << 8 | buffer[3]);
    numStruct.rscale  = 256 * (buffer[4] & 0x3f) + buffer[5];
    numStruct.dscale  = 256 * (buffer[6] & 0x3f) + buffer[7];
    numStruct.sign    = (buffer[4] & 0x40) != 0;
    numStruct.digits  = &buffer[8];
    if (numStruct.ndigits == 0) {
      intValue = 0;
    } else if (unlikely(numStruct.weight < numStruct.ndigits - 1 ||
                        numStruct.rscale != 0 ||
                        numStruct.dscale != 0)) {
      logError(printf("getNumericAsInt: Number not integer\n"););
      raise_error(RANGE_ERROR);
      intValue = 0;
    } else if (unlikely((stri = getNumericAsStri(&numStruct)) == NULL)) {
      raise_error(MEMORY_ERROR);
      intValue = 0;
    } else {
      intValue = intParse(stri);
      strDestr(stri);
      if (intValue != 0 && numStruct.weight != numStruct.ndigits - 1) {
        switch (4 * (numStruct.weight - (numStruct.ndigits - 1))) {
          case  0: factor =                 1; break;
          case  4: factor =             10000; break;
          case  8: factor =         100000000; break;
          case 12: factor =     1000000000000; break;
          case 16: factor = 10000000000000000; break;
          default:
            logError(printf("getNumericAsInt: Wrong decimal scale: %d\n",
                            4 * (numStruct.weight - (numStruct.ndigits - 1))););
            raise_error(RANGE_ERROR);
            factor = 1;
            break;
        } /* switch */
        if (intValue < 0) {
          if (unlikely(intValue < INTTYPE_MIN / factor)) {
            logError(printf("getNumericAsInt: Value (" FMT_D
                            ") smaller than minimum (" FMT_D ").\n",
                            intValue, INTTYPE_MIN / factor));
            raise_error(RANGE_ERROR);
            factor = 1;
          } /* if */
        } else {
          if (unlikely(intValue > INTTYPE_MAX / factor)) {
            logError(printf("getNumericAsInt: Value (" FMT_D
                            ") larger than maximum (" FMT_D ").\n",
                            intValue, INTTYPE_MAX / factor));
            raise_error(RANGE_ERROR);
            factor = 1;
          } /* if */
        } /* if */
        intValue *= factor;
      } /* if */
    } /* if */
    logFunction(printf("getNumericAsInt --> " FMT_D "\n", intValue););
    return intValue;
  } /* getNumericAsInt */



static bigIntType getNumericAsBigInt (const unsigned char *buffer)

  {
    numeric numStruct;
    striType stri;
    bigIntType powerOfTen;
    bigIntType bigIntValue;

  /* getNumericAsBigInt */
    logFunction(printf("getNumericAsBigInt()\n");
                showNumeric(buffer););
    numStruct.ndigits = 256 * buffer[0] + buffer[1];
    numStruct.weight  = (int16Type) (buffer[2] << 8 | buffer[3]);
    numStruct.rscale  = 256 * (buffer[4] & 0x3f) + buffer[5];
    numStruct.dscale  = 256 * (buffer[6] & 0x3f) + buffer[7];
    numStruct.sign    = (buffer[4] & 0x40) != 0;
    numStruct.digits  = &buffer[8];
    if (numStruct.ndigits == 0) {
      bigIntValue = bigZero();
    } else if (unlikely(numStruct.weight < numStruct.ndigits - 1 ||
                        numStruct.rscale != 0 ||
                        numStruct.dscale != 0)) {
      logError(printf("getNumericAsBigInt: Number not integer\n"););
      raise_error(RANGE_ERROR);
      bigIntValue = NULL;
    } else if (unlikely((stri = getNumericAsStri(&numStruct)) == NULL)) {
      raise_error(MEMORY_ERROR);
      bigIntValue = NULL;
    } else {
      bigIntValue = bigParse(stri);
      strDestr(stri);
      if (bigIntValue != NULL && numStruct.weight != numStruct.ndigits - 1) {
        powerOfTen = bigIPowSignedDigit(10,
            (intType) (4 * (numStruct.weight - (numStruct.ndigits - 1))));
        if (powerOfTen != NULL) {
          bigMultAssign(&bigIntValue, powerOfTen);
          bigDestr(powerOfTen);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("getNumericAsBigInt --> %s\n",
                       bigHexCStri(bigIntValue)););
    return bigIntValue;
  } /* getNumericAsBigInt */



static bigIntType getNumericAsBigRat (const unsigned char *buffer,
    bigIntType *denominator)

  {
    numeric numStruct;
    striType stri;
    bigIntType powerOfTen;
    bigIntType numerator;

  /* getNumericAsBigRat */
    logFunction(printf("getNumericAsBigRat()\n");
                showNumeric(buffer););
    numStruct.ndigits = 256 * buffer[0] + buffer[1];
    numStruct.weight  = (int16Type) (buffer[2] << 8 | buffer[3]);
    numStruct.rscale  = 256 * (buffer[4] & 0x3f) + buffer[5];
    numStruct.dscale  = 256 * (buffer[6] & 0x3f) + buffer[7];
    numStruct.sign    = (buffer[4] & 0x40) != 0;
    numStruct.digits  = &buffer[8];
    if (numStruct.ndigits == 0) {
      numerator = bigZero();
      *denominator = bigFromInt32(1);
    } else if (unlikely((stri = getNumericAsStri(&numStruct)) == NULL)) {
      raise_error(MEMORY_ERROR);
      numerator = NULL;
    } else {
      numerator = bigParse(stri);
      strDestr(stri);
      if (numerator != NULL) {
        if (numStruct.weight >= numStruct.ndigits - 1) {
          if (numStruct.weight != numStruct.ndigits - 1) {
            powerOfTen = bigIPowSignedDigit(10,
                (intType) (4 * (numStruct.weight - (numStruct.ndigits - 1))));
            if (powerOfTen != NULL) {
              bigMultAssign(&numerator, powerOfTen);
              bigDestr(powerOfTen);
            } /* if */
          } /* if */
          *denominator = bigFromInt32(1);
        } else {
          *denominator = bigIPowSignedDigit(10,
              (intType) (4 * (numStruct.ndigits - 1 - numStruct.weight)));
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("getNumericAsBigRat --> %s",
                       striAsUnquotedCStri(bigStr(numerator)));
                printf(" (denominator = %s)\n",
                       striAsUnquotedCStri(bigStr(*denominator))););
    return numerator;
  } /* getNumericAsBigRat */



static floatType getNumericAsFloat (const unsigned char *buffer)

  {
    numeric numStruct;
    cstriType decimal;
    floatType floatValue;

  /* getNumericAsFloat */
    logFunction(printf("getNumericAsFloat()\n");
                showNumeric(buffer););
    numStruct.ndigits = 256 * buffer[0] + buffer[1];
    numStruct.weight  = (int16Type) (buffer[2] << 8 | buffer[3]);
    numStruct.rscale  = 256 * (buffer[4] & 0x3f) + buffer[5];
    numStruct.dscale  = 256 * (buffer[6] & 0x3f) + buffer[7];
    numStruct.sign    = (buffer[4] & 0x40) != 0;
    numStruct.digits  = &buffer[8];
    if (numStruct.ndigits == 0) {
      floatValue = 0.0;
    } else if (unlikely((decimal = getNumericAsCStri(&numStruct)) == NULL)) {
      raise_error(MEMORY_ERROR);
      floatValue = 0.0;
    } else {
      floatValue = (floatType) strtod(decimal, NULL);
      UNALLOC_CSTRI(decimal, strlen(decimal));
    } /* if */
    logFunction(printf("getNumericAsFloat --> " FMT_E "\n", floatValue););
    return floatValue;
  } /* getNumericAsFloat */



static timeStampType getTimestamp1970 (timeStampType timestamp,
    intType *micro_second)

  { /* getTimestamp1970 */
    logFunction(printf("getTimestamp1970(" FMT_D64 ")\n", timestamp););
    *micro_second = timestamp % 1000000;
    if (timestamp < 0) {
      if (*micro_second != 0) {
        *micro_second += 1000000;
      } /* if */
      timestamp = (timestamp + 1) / 1000000 - 1;
    } else {
      timestamp /= 1000000;
    } /* if */
    /* printf("timestamp2000: " FMT_U64 "\n", timestamp); */
    if (unlikely(timestamp > INT64TYPE_MAX - SECONDS_FROM_1970_TO_2000)) {
      logError(printf("getTimestamp1970: "
                      "Cannot compute timestamp1970 from timestamp2000 (" FMT_D64 ").\n",
                      timestamp););
      raise_error(RANGE_ERROR);
    } else {
      timestamp += SECONDS_FROM_1970_TO_2000;
    } /* if */
    logFunction(printf("getTimestamp1970 --> " FMT_D64 "\n", timestamp););
    return timestamp;
  } /* getTimestamp1970 */



static void sqlBindBigInt (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType value)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    cstriType decimalNumber;
    memSizeType length;
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
      logMessage(printf("paramType: %s\n",
                        nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
      switch (preparedStmt->paramTypes[pos - 1]) {
        case INT2OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(int16Type *) param->buffer =
              (int16Type) htons((uint16Type) bigToInt16(value, &err_info));
          break;
        case INT4OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(int32Type *) param->buffer =
              (int32Type) htonl((uint32Type) bigToInt32(value, &err_info));
          break;
        case INT8OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(int64Type *) param->buffer =
              (int64Type) htonll((uint64Type) bigToInt64(value, &err_info));
          break;
        case FLOAT4OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(float *) param->buffer =
              htonf((float) bigIntToDouble(value));
          break;
        case FLOAT8OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(double *) param->buffer =
              htond(bigIntToDouble(value));
          break;
        case NUMERICOID:
        case BPCHAROID:
        case VARCHAROID:
        case CASHOID:
          decimalNumber = (cstriType) bigIntToDecimal(value, &length, &err_info);
          if (likely(decimalNumber != NULL)) {
            if (unlikely(length > INT_MAX)) {
              /* It is not possible to cast length to int. */
              free(decimalNumber);
              err_info = MEMORY_ERROR;
            } else {
              free(param->buffer);
              param->buffer = decimalNumber;
              preparedStmt->paramValues[pos - 1] = decimalNumber;
              preparedStmt->paramLengths[pos - 1] = (int) length;
              preparedStmt->paramFormats[pos - 1] = 0;
            } /* if */
          } /* if */
          break;
        default:
          logError(printf("sqlBindBigInt: Parameter " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          err_info = RANGE_ERROR;
          break;
      } /* switch */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindBigInt */



static void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType numerator, const const_bigIntType denominator)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    cstriType decimalNumber;
    memSizeType length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBigRat */
    logFunction(printf("sqlBindBigRat(" FMT_U_MEM ", " FMT_D ", %s, ",
                       (memSizeType) sqlStatement, pos,
                       bigHexCStri(numerator));
                printf("%s)\n", bigHexCStri(denominator)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBigRat: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      logMessage(printf("paramType: %s\n",
                        nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
      switch (preparedStmt->paramTypes[pos - 1]) {
        case FLOAT4OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(float *) param->buffer =
              htonf((float) bigRatToDouble(numerator, denominator));
          break;
        case FLOAT8OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(double *) param->buffer =
              htond(bigRatToDouble(numerator, denominator));
          break;
        case NUMERICOID:
        case CASHOID:
          decimalNumber = (cstriType) bigRatToDecimal(numerator, denominator,
              DEFAULT_DECIMAL_SCALE, &length, &err_info);
          if (likely(decimalNumber != NULL)) {
            if (unlikely(length > INT_MAX)) {
              /* It is not possible to cast length to int. */
              free(decimalNumber);
              err_info = MEMORY_ERROR;
            } else {
              free(param->buffer);
              param->buffer = decimalNumber;
              preparedStmt->paramValues[pos - 1] = decimalNumber;
              preparedStmt->paramLengths[pos - 1] = (int) length;
              preparedStmt->paramFormats[pos - 1] = 0;
            } /* if */
          } /* if */
          break;
        default:
          logError(printf("sqlBindBigRat: Parameter " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          err_info = RANGE_ERROR;
          break;
      } /* switch */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindBigRat */



static void sqlBindBool (sqlStmtType sqlStatement, intType pos, boolType value)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
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
      logMessage(printf("paramType: %s\n",
                        nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
      switch (preparedStmt->paramTypes[pos - 1]) {
        case INT2OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(int16Type *) param->buffer =
              (int16Type) htons((uint16Type) value);
          break;
        case INT4OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(int32Type *) param->buffer =
              (int32Type) htonl((uint32Type) value);
          break;
        case INT8OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(int64Type *) param->buffer =
              (int64Type) htonll((uint64Type) value);
          break;
        case NUMERICOID:
        case BPCHAROID:
        case VARCHAROID:
          free(param->buffer);
          if (unlikely((param->buffer = (cstriType) malloc(2)) == NULL)) {
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->paramValues[pos - 1] = param->buffer;
            param->buffer[0] = (char) ('0' + value);
            param->buffer[1] = '\0';
            preparedStmt->paramLengths[pos - 1] = 1;
            preparedStmt->paramFormats[pos - 1] = 0;
          } /* if */
          break;
        default:
          logError(printf("sqlBindBool: Parameter " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          err_info = RANGE_ERROR;
          break;
      } /* switch */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindBool */



static void sqlBindBStri (sqlStmtType sqlStatement, intType pos,
    const const_bstriType bstri)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
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
      logMessage(printf("paramType: %s\n",
                        nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
      switch (preparedStmt->paramTypes[pos - 1]) {
        case BYTEAOID:
        case BPCHAROID:
        case VARCHAROID:
          if (unlikely(bstri->size > INT_MAX)) {
            /* It is not possible to cast bstri->size to int. */
            err_info = MEMORY_ERROR;
          } else {
            free(param->buffer);
            if (unlikely((param->buffer = (cstriType) malloc(
                              bstri->size + NULL_TERMINATION_LEN)) == NULL)) {
              err_info = MEMORY_ERROR;
            } else {
              preparedStmt->paramValues[pos - 1] = param->buffer;
              memcpy(param->buffer, bstri->mem, bstri->size);
              param->buffer[bstri->size] = '\0';
              preparedStmt->paramLengths[pos - 1] = (int) bstri->size;
              preparedStmt->paramFormats[pos - 1] = 1;
            } /* if */
          } /* if */
          break;
        default:
          logError(printf("sqlBindBStri: Parameter " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          err_info = RANGE_ERROR;
          break;
      } /* switch */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindBStri */



static void sqlBindDuration (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    int64Type microsecDuration;
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
    } else if (unlikely(year < INT32TYPE_MIN || year > INT32TYPE_MAX ||
                        month < -12 || month > 12 ||
                        day < -31 || day > 31 ||
                        hour <= -24 || hour >= 24 ||
                        minute <= -60 || minute >= 60 ||
                        second <= -60 || second >= 60 ||
                        micro_second <= -1000000 || micro_second >= 1000000)) {
      logError(printf("sqlBindDuration: Duration not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      logMessage(printf("paramType: %s\n",
                        nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
      switch (preparedStmt->paramTypes[pos - 1]) {
        case INTERVALOID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          microsecDuration = ((((int64Type) hour) * 60 +
                                (int64Type) minute) * 60 +
                                (int64Type) second) * 1000000 +
                                (int64Type) micro_second;
          /* printf("microsecDuration: " FMT_D64 "\n", microsecDuration); */
          /* The interval is either an int64Type representing       */
          /* microseconds, or a double representing seconds.        */
          /* PQparameterStatus(connection, "integer_datetimes") is  */
          /* used to determine if an int64Type or a double is used. */
          if (preparedStmt->integerDatetimes) {
            *(int64Type *) param->buffer =
                (int64Type) htonll((uint64Type) microsecDuration);
          } else {
            *(double *) param->buffer =
                htond((double) microsecDuration / 1000000.0);
          } /* if */
          *(int32Type *) &param->buffer[8] =
              (int32Type) htonl((uint32Type) day);
          *(int32Type *) &param->buffer[12] =
              (int32Type) htonl((uint32Type) (12 * year + month));
          break;
        default:
          logError(printf("sqlBindDuration: Parameter " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          err_info = RANGE_ERROR;
          break;
      } /* switch */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindDuration */



static void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
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
      logMessage(printf("paramType: %s\n",
                        nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
      switch (preparedStmt->paramTypes[pos - 1]) {
        case FLOAT4OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(float *) param->buffer = htonf((float) value);
          break;
        case FLOAT8OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(double *) param->buffer = htond(value);
          break;
        case CASHOID:
          free(param->buffer);
          if (unlikely((param->buffer = (cstriType) malloc(
                            FLOATTYPE_SIZE + NULL_TERMINATION_LEN)) == NULL)) {
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->paramValues[pos - 1] = param->buffer;
            preparedStmt->paramLengths[pos - 1] =
                (int) sprintf(param->buffer, "%lf", value);
            preparedStmt->paramFormats[pos - 1] = 0;
          } /* if */
          break;
        default:
          logError(printf("sqlBindFloat: Parameter " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          err_info = RANGE_ERROR;
          break;
      } /* switch */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindFloat */



static void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
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
      logMessage(printf("paramType: %s\n",
                        nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
      switch (preparedStmt->paramTypes[pos - 1]) {
        case INT2OID:
          if (unlikely(value < INT16TYPE_MIN || value > INT16TYPE_MAX)) {
            logError(printf("sqlBindInt: Parameter " FMT_D ": "
                            FMT_D " does not fit into a 16-bit integer.\n",
                            pos, value));
            err_info = RANGE_ERROR;
          } else {
            preparedStmt->paramValues[pos - 1] = param->buffer;
            *(int16Type *) param->buffer =
                (int16Type) htons((uint16Type) value);
          } /* if */
          break;
        case INT4OID:
          if (unlikely(value < INT32TYPE_MIN || value > INT32TYPE_MAX)) {
            logError(printf("sqlBindInt: Parameter " FMT_D ": "
                            FMT_D " does not fit into a 32-bit integer.\n",
                            pos, value));
            err_info = RANGE_ERROR;
          } else {
            preparedStmt->paramValues[pos - 1] = param->buffer;
            *(int32Type *) param->buffer =
                (int32Type) htonl((uint32Type) value);
          } /* if */
          break;
        case INT8OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(int64Type *) param->buffer =
              (int64Type) htonll((uint64Type) value);
          break;
        case FLOAT4OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(float *) param->buffer = htonf((float) value);
          break;
        case FLOAT8OID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          *(double *) param->buffer = htond((double) value);
          break;
        case CASHOID:
        case NUMERICOID:
        case BPCHAROID:
        case VARCHAROID:
          free(param->buffer);
          if (unlikely((param->buffer = (cstriType) malloc(
                            INTTYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN)) == NULL)) {
            err_info = MEMORY_ERROR;
          } else {
            preparedStmt->paramValues[pos - 1] = param->buffer;
            preparedStmt->paramLengths[pos - 1] =
                (int) sprintf(param->buffer, FMT_D, value);
            preparedStmt->paramFormats[pos - 1] = 0;
          } /* if */
          break;
        default:
          logError(printf("sqlBindInt: Parameter " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          err_info = RANGE_ERROR;
          break;
      } /* switch */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
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
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindNull: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      logMessage(printf("paramType: %s\n",
                        nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
      preparedStmt->paramValues[pos - 1] = NULL;
      preparedStmt->executeSuccessful = FALSE;
      preparedStmt->fetchOkay = FALSE;
      preparedStmt->param_array[pos - 1].bound = TRUE;
    } /* if */
  } /* sqlBindNull */



static void sqlBindStri (sqlStmtType sqlStatement, intType pos,
    const const_striType stri)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    cstriType stri8;
    cstriType resized_stri8;
    memSizeType length;
    cstriType cstri;
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
      param = &preparedStmt->param_array[pos - 1];
      logMessage(printf("paramType: %s\n",
                        nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
      switch (preparedStmt->paramTypes[pos - 1]) {
        case BPCHAROID:
        case VARCHAROID:
        case TEXTOID:
          /* PostgreSQL doesn't support storing Null characters     */
          /* ('\0;') in text fields. PQexecPrepared() returns a     */
          /* status of PGRES_FATAL_ERROR, because 0x00 and 0xc080   */
          /* (the overlong UTF-8 encoding for the Null character)   */
          /* are seen as invalid byte sequence for encoding "UTF8". */
          /* Therefore binding of strings with a Null character     */
          /* ('\0;') inside will fail later with sqlExecute().      */
          stri8 = stri_to_cstri8_buf(stri, &length);
          if (unlikely(stri8 == NULL)) {
            err_info = MEMORY_ERROR;
          } else if (unlikely(length > INT_MAX)) {
            /* It is not possible to cast length to int. */
            free(stri8);
            err_info = MEMORY_ERROR;
          } else {
            resized_stri8 = REALLOC_CSTRI(stri8, length);
            if (likely(resized_stri8 != NULL)) {
              stri8 = resized_stri8;
            } /* if */
            free(param->buffer);
            param->buffer = stri8;
            preparedStmt->paramValues[pos - 1] = stri8;
            preparedStmt->paramLengths[pos - 1] = (int) length;
            preparedStmt->paramFormats[pos - 1] = 1;
          } /* if */
          break;
        case BYTEAOID:
          if (unlikely(stri->size > INT_MAX ||
                       (cstri = (cstriType) malloc(stri->size)) == NULL)) {
            err_info = MEMORY_ERROR;
          } else if (unlikely(memcpy_from_strelem((ustriType) cstri,
                                                  stri->mem, stri->size))) {
            free(cstri);
            err_info = RANGE_ERROR;
          } else {
            free(param->buffer);
            param->buffer = cstri;
            preparedStmt->paramValues[pos - 1] = cstri;
            preparedStmt->paramLengths[pos - 1] = (int) stri->size;
            preparedStmt->paramFormats[pos - 1] = 1;
          } /* if */
          break;
        default:
          logError(printf("sqlBindStri: Parameter " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          err_info = RANGE_ERROR;
          break;
      } /* switch */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindStri */



static void sqlBindTime (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second,
    intType time_zone)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    timeStampType timestamp;
    int32Type zone;
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
    } else if (unlikely(year <= INT16TYPE_MIN || year > INT16TYPE_MAX ||
                        month < 1 || month > 12 ||
                        day < 1 || day > 31 ||
                        hour < 0 || hour >= 24 ||
                        minute < 0 || minute >= 60 ||
                        second < 0 || second >= 60 ||
                        micro_second < 0 || micro_second >= 1000000)) {
      logError(printf("sqlBindTime: Time not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      logMessage(printf("paramType: %s\n",
                        nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
      switch (preparedStmt->paramTypes[pos - 1]) {
        case DATEOID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          timestamp = timToTimestamp(year, month, day, 0, 0, 0, 0);
          timestamp = (timestamp - SECONDS_FROM_1970_TO_2000) / (24 * 60 * 60);
          /* printf("DATEOID timestamp: " FMT_D64 "\n", timestamp); */
          *(int32Type *) param->buffer =
              (int32Type) htonl((uint32Type) timestamp);
          break;
        case TIMEOID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          timestamp = timToTimestamp(2000, 1, 1, hour, minute, second, 0);
          /* printf("timestamp1970: " FMT_D64 "\n", timestamp); */
          timestamp = (timestamp - SECONDS_FROM_1970_TO_2000) * 1000000 + micro_second;
          /* printf("TIMEOID timestamp: " FMT_D64 "\n", timestamp); */
          /* The time is either an int64Type representing           */
          /* microseconds away from 2000-01-01 00:00:00 UTC, or a   */
          /* double representing seconds away from the same origin. */
          /* PQparameterStatus(connection, "integer_datetimes") is  */
          /* used to determine if an int64Type or a double is used. */
          if (preparedStmt->integerDatetimes) {
            *(int64Type *) param->buffer =
                (int64Type) htonll((uint64Type) timestamp);
          } else {
            *(double *) param->buffer =
                htond((double) timestamp / 1000000.0);
          } /* if */
          break;
        case TIMETZOID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          timestamp = timToTimestamp(2000, 1, 1, hour, minute, second, 0);
          /* printf("timestamp1970: " FMT_D64 "\n", timestamp); */
          timestamp = (timestamp - SECONDS_FROM_1970_TO_2000) * 1000000 + micro_second;
          /* printf("TIMETZOID timestamp: " FMT_D64 "\n", timestamp); */
          /* The time is either an int64Type representing           */
          /* microseconds away from 2000-01-01 00:00:00 UTC, or a   */
          /* double representing seconds away from the same origin. */
          /* PQparameterStatus(connection, "integer_datetimes") is  */
          /* used to determine if an int64Type or a double is used. */
          if (preparedStmt->integerDatetimes) {
            *(int64Type *) param->buffer =
                (int64Type) htonll((uint64Type) timestamp);
          } else {
            *(double *) param->buffer =
                htond((double) timestamp / 1000000.0);
          } /* if */
          /* printf("zone: " FMT_D32 "\n", (int32Type) (-time_zone * 60)); */
          zone = (int32Type) htonl((uint32Type) (-time_zone * 60));
          *(int64Type *) &param->buffer[8] = zone;
          break;
        case TIMESTAMPOID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          timestamp = timToTimestamp(year, month, day, hour, minute, second, 0);
          /* printf("timestamp1970: " FMT_D64 "\n", timestamp); */
          timestamp = (timestamp - SECONDS_FROM_1970_TO_2000) * 1000000 + micro_second;
          /* printf("TIMESTAMPOID timestamp: " FMT_D64 "\n", timestamp); */
          /* The timestamp is either an int64Type representing      */
          /* microseconds away from 2000-01-01 00:00:00 UTC, or a   */
          /* double representing seconds away from the same origin. */
          /* PQparameterStatus(connection, "integer_datetimes") is  */
          /* used to determine if an int64Type or a double is used. */
          if (preparedStmt->integerDatetimes) {
            *(int64Type *) param->buffer =
                (int64Type) htonll((uint64Type) timestamp);
          } else {
            *(double *) param->buffer =
                htond((double) timestamp / 1000000.0);
          } /* if */
          break;
        case TIMESTAMPTZOID:
          preparedStmt->paramValues[pos - 1] = param->buffer;
          timestamp = timToTimestamp(year, month, day, hour, minute, second,
                                     time_zone);
          /* printf("timestamp1970: " FMT_D64 "\n", timestamp); */
          timestamp = (timestamp - SECONDS_FROM_1970_TO_2000) * 1000000 + micro_second;
          /* printf("TIMESTAMPTZOID timestamp: " FMT_D64 "\n", timestamp); */
          /* The timestamp is either an int64Type representing      */
          /* microseconds away from 2000-01-01 00:00:00 UTC, or a   */
          /* double representing seconds away from the same origin. */
          /* PQparameterStatus(connection, "integer_datetimes") is  */
          /* used to determine if an int64Type or a double is used. */
          if (preparedStmt->integerDatetimes) {
            *(int64Type *) param->buffer =
                (int64Type) htonll((uint64Type) timestamp);
          } else {
            *(double *) param->buffer =
                htond((double) timestamp / 1000000.0);
          } /* if */
          break;
        default:
          logError(printf("sqlBindTime: Parameter " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          err_info = RANGE_ERROR;
          break;
      } /* switch */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
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
      PQfinish(db->connection);
      db->connection = NULL;
    } /* if */
    db->isOpen = FALSE;
    logFunction(printf("sqlClose -->\n"););
  } /* sqlClose */



static bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    int isNull;
    const_cstriType buffer;
    Oid buffer_type;
    bigIntType columnValue;

  /* sqlColumnBigInt */
    logFunction(printf("sqlColumnBigInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnBigInt: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        logMessage(printf("Column is NULL -> Use default value: 0\n"););
        columnValue = bigZero();
      } else if (unlikely(isNull != 0)) {
        dbInconsistent("sqlColumnBigInt", "PQgetisnull");
        logError(printf("sqlColumnBigInt: Column " FMT_D ": "
                        "PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (unlikely(buffer == NULL)) {
          dbInconsistent("sqlColumnBigInt", "PQgetvalue");
          logError(printf("sqlColumnBigInt: Column " FMT_D ": "
                          "PQgetvalue returns NULL.\n",
                          column););
          raise_error(DATABASE_ERROR);
          columnValue = NULL;
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          logMessage(printf("buffer_type: %s\n", nameOfBufferType(buffer_type)););
          switch (buffer_type) {
            case INT2OID:
              columnValue = bigFromInt32((int16Type) ntohs(*(const uint16Type *) buffer));
              break;
            case INT4OID:
              columnValue = bigFromInt32((int32Type) ntohl(*(const uint32Type *) buffer));
              break;
            case INT8OID:
              columnValue = bigFromInt64((int64Type) ntohll(*(const uint64Type *) buffer));
              break;
            case CASHOID:
              columnValue = bigFromInt64(((int64Type) ntohll(*(const uint64Type *) buffer)) / (int32Type) preparedStmt->db->moneyDenominator);
              break;
            case NUMERICOID:
              columnValue = getNumericAsBigInt((const_ustriType) buffer);
              break;
            default:
              logError(printf("sqlColumnBigInt: Column " FMT_D " has the unknown type %s.\n",
                              column, nameOfBufferType(buffer_type)););
              raise_error(RANGE_ERROR);
              columnValue = NULL;
              break;
          } /* switch */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnBigInt --> %s\n", bigHexCStri(columnValue)););
    return columnValue;
  } /* sqlColumnBigInt */



static void sqlColumnBigRat (sqlStmtType sqlStatement, intType column,
    bigIntType *numerator, bigIntType *denominator)

  {
    preparedStmtType preparedStmt;
    int isNull;
    const_cstriType buffer;
    Oid buffer_type;

  /* sqlColumnBigRat */
    logFunction(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", *, *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnBigRat: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        logMessage(printf("Column is NULL -> Use default value: 0\n"););
        *numerator = bigZero();
        *denominator = bigFromInt32(1);
      } else if (unlikely(isNull != 0)) {
        dbInconsistent("sqlColumnBigRat", "PQgetisnull");
        logError(printf("sqlColumnBigRat: Column " FMT_D ": "
                        "PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(DATABASE_ERROR);
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (unlikely(buffer == NULL)) {
          dbInconsistent("sqlColumnBigRat", "PQgetvalue");
          logError(printf("sqlColumnBigRat: Column " FMT_D ": "
                          "PQgetvalue returns NULL.\n", column););
          raise_error(DATABASE_ERROR);
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          logMessage(printf("buffer_type: %s\n", nameOfBufferType(buffer_type)););
          switch (buffer_type) {
            case INT2OID:
              *numerator = bigFromInt32((int16Type) ntohs(*(const uint16Type *) buffer));
              *denominator = bigFromInt32(1);
              break;
            case INT4OID:
              *numerator = bigFromInt32((int32Type) ntohl(*(const uint32Type *) buffer));
              *denominator = bigFromInt32(1);
              break;
            case INT8OID:
              *numerator = bigFromInt64((int64Type) ntohll(*(const uint64Type *) buffer));
              *denominator = bigFromInt32(1);
              break;
            case CASHOID:
              *numerator = bigFromInt64((int64Type) ntohll(*(const uint64Type *) buffer));
              *denominator = bigFromInt32((int32Type) preparedStmt->db->moneyDenominator);
              break;
            case FLOAT4OID:
              *numerator = roundDoubleToBigRat(ntohf(*(const float *) buffer),
                                               FALSE, denominator);
              break;
            case FLOAT8OID:
              *numerator = roundDoubleToBigRat(ntohd(*(const double *) buffer),
                                               TRUE, denominator);
              break;
            case NUMERICOID:
              *numerator = getNumericAsBigRat((const_ustriType) buffer, denominator);
              break;
            default:
              logError(printf("sqlColumnBigRat: Column " FMT_D " has the unknown type %s.\n",
                              column, nameOfBufferType(buffer_type)););
              raise_error(RANGE_ERROR);
          } /* switch */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", %s, ",
                       (memSizeType) sqlStatement, column,
                       bigHexCStri(*numerator));
                printf("%s) -->\n", bigHexCStri(*denominator)););
  } /* sqlColumnBigRat */



static boolType sqlColumnBool (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    int isNull;
    const_cstriType buffer;
    Oid buffer_type;
    int length;
    intType columnValue;

  /* sqlColumnBool */
    logFunction(printf("sqlColumnBool(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnBool: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        logMessage(printf("Column is NULL -> Use default value: 0\n"););
        columnValue = 0;
      } else if (unlikely(isNull != 0)) {
        dbInconsistent("sqlColumnBool", "PQgetisnull");
        logError(printf("sqlColumnBool: Column " FMT_D ": "
                        "PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(DATABASE_ERROR);
        columnValue = 0;
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (unlikely(buffer == NULL)) {
          dbInconsistent("sqlColumnBool", "PQgetvalue");
          logError(printf("sqlColumnBool: Column " FMT_D ": "
                          "PQgetvalue returns NULL.\n",
                          column););
          raise_error(DATABASE_ERROR);
          columnValue = 0;
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          logMessage(printf("buffer_type: %s\n", nameOfBufferType(buffer_type)););
          switch (buffer_type) {
            case INT2OID:
              columnValue = (int16Type) ntohs(*(const uint16Type *) buffer);
              break;
            case INT4OID:
              columnValue = (int32Type) ntohl(*(const uint32Type *) buffer);
              break;
            case INT8OID:
              columnValue = (int64Type) ntohll(*(const uint64Type *) buffer);
              break;
            case CHAROID:
            case BPCHAROID:
            case VARCHAROID:
            case TEXTOID:
              length = PQgetlength(preparedStmt->execute_result,
                                   preparedStmt->fetch_index,
                                   (int) column - 1);
              if (unlikely(length != 1)) {
                logError(printf("sqlColumnBool: Column " FMT_D ": "
                                "The size of a boolean field must be 1.\n", column););
                raise_error(RANGE_ERROR);
                columnValue = 0;
              } else {
                columnValue = buffer[0] != '0';
              } /* if */
              break;
            case BOOLOID:
              columnValue = buffer[0];
              break;
            default:
              logError(printf("sqlColumnBool: Column " FMT_D " has the unknown type %s.\n",
                              column, nameOfBufferType(buffer_type)););
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
    } /* if */
    logFunction(printf("sqlColumnBool --> %s\n", columnValue ? "TRUE" : "FALSE"););
    return columnValue != 0;
  } /* sqlColumnBool */



static bstriType sqlColumnBStri (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    int isNull;
    const_cstriType buffer;
    Oid buffer_type;
    int length;
    errInfoType err_info = OKAY_NO_ERROR;
    bstriType columnValue;

  /* sqlColumnBStri */
    logFunction(printf("sqlColumnBStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnBStri: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        logMessage(printf("Column is NULL -> Use default value: \"\"\n"););
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(columnValue, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          columnValue->size = 0;
        } /* if */
      } else if (unlikely(isNull != 0)) {
        dbInconsistent("sqlColumnBStri", "PQgetisnull");
        logError(printf("sqlColumnBStri: Column " FMT_D ": "
                        "PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (unlikely(buffer == NULL)) {
          dbInconsistent("sqlColumnBStri", "PQgetvalue");
          logError(printf("sqlColumnBStri: Column " FMT_D ": "
                          "PQgetvalue returns NULL.\n",
                          column););
          raise_error(DATABASE_ERROR);
          columnValue = NULL;
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          logMessage(printf("buffer_type: %s\n", nameOfBufferType(buffer_type)););
          switch (buffer_type) {
            case BYTEAOID:
              length = PQgetlength(preparedStmt->execute_result,
                                   preparedStmt->fetch_index,
                                   (int) column - 1);
              if (unlikely(length < 0)) {
                dbInconsistent("sqlColumnBStri", "PQgetlength");
                logError(printf("sqlColumnBStri: Column " FMT_D ": "
                                "PQgetlength returns %d.\n",
                                column, length););
                raise_error(DATABASE_ERROR);
                columnValue = NULL;
              } else if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(columnValue, (memSizeType) length))) {
                raise_error(MEMORY_ERROR);
              } else {
                memcpy(columnValue->mem, buffer, (memSizeType) length);
                columnValue->size = (memSizeType) length;
              } /* if */
              break;
#if 0
            case NAMEOID:
            case CHAROID:
            case VARCHAROID:
            case TEXTOID:
              length = PQgetlength(preparedStmt->execute_result,
                                   preparedStmt->fetch_index,
                                   (int) column - 1);
              if (unlikely(length < 0)) {
                dbInconsistent("sqlColumnBStri", "PQgetlength");
                logError(printf("sqlColumnBStri: Column " FMT_D ": "
                                "PQgetlength returns %d.\n",
                                column, length););
                raise_error(DATABASE_ERROR);
                columnValue = NULL;
              } else {
                columnValue = cstri8_buf_to_stri(buffer, (memSizeType) length, &err_info);
                if (unlikely(columnValue == NULL)) {
                  raise_error(err_info);
                } /* if */
              } /* if */
              break;
            case BPCHAROID:
              length = PQgetlength(preparedStmt->execute_result,
                                   preparedStmt->fetch_index,
                                   (int) column - 1);
              if (unlikely(length < 0)) {
                dbInconsistent("sqlColumnBStri", "PQgetlength");
                logError(printf("sqlColumnBStri: Column " FMT_D ": "
                                "PQgetlength returns %d.\n",
                                column, length););
                raise_error(DATABASE_ERROR);
                columnValue = NULL;
              } else {
                while (length > 0 && buffer[length - 1] == ' ') {
                  length--;
                } /* if */
                columnValue = cstri8_buf_to_stri(buffer, (memSizeType) length, &err_info);
                if (unlikely(columnValue == NULL)) {
                  raise_error(err_info);
                } /* if */
              } /* if */
              break;
#endif
            default:
              logError(printf("sqlColumnBStri: Column " FMT_D " has the unknown type %s.\n",
                              column, nameOfBufferType(buffer_type)););
              raise_error(RANGE_ERROR);
              columnValue = NULL;
              break;
          } /* switch */
        } /* if */
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
    int isNull;
    const_cstriType buffer;
    Oid buffer_type;
    int64Type microsecDuration;
    int32Type dayDuration;
    int32Type monthDuration;

  /* sqlColumnDuration */
    logFunction(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnDuration: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        logMessage(printf("Column is NULL -> Use default value: P0D\n"););
        *year         = 0;
        *month        = 0;
        *day          = 0;
        *hour         = 0;
        *minute       = 0;
        *second       = 0;
        *micro_second = 0;
      } else if (unlikely(isNull != 0)) {
        dbInconsistent("sqlColumnDuration", "PQgetisnull");
        logError(printf("sqlColumnDuration: Column " FMT_D ": "
                        "PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(DATABASE_ERROR);
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (unlikely(buffer == NULL)) {
          dbInconsistent("sqlColumnDuration", "PQgetvalue");
          logError(printf("sqlColumnDuration: Column " FMT_D ": "
                          "PQgetvalue returns NULL.\n",
                          column););
          raise_error(DATABASE_ERROR);
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          logMessage(printf("buffer_type: %s\n", nameOfBufferType(buffer_type)););
          switch (buffer_type) {
            case INTERVALOID:
              /* The interval is either an int64Type representing       */
              /* microseconds, or a double representing seconds.        */
              /* PQparameterStatus(connection, "integer_datetimes") is  */
              /* used to determine if an int64Type or a double is used. */
              if (preparedStmt->integerDatetimes) {
                microsecDuration = (int64Type) ntohll(*(const uint64Type *) buffer);
              } else {
                microsecDuration = (int64Type)
                    (1000000.0 * ntohd(*(const double *) buffer) + 0.5);
              } /* if */
              dayDuration = (int32Type) ntohl(*(const uint32Type *) &buffer[8]);
              monthDuration = (int32Type) ntohl(*(const uint32Type *) &buffer[12]);
              /* printf("microsecDuration: " FMT_D64 "\n", microsecDuration);
                 printf("dayDuration: " FMT_D32 "\n", dayDuration);
                 printf("monthDuration: " FMT_D32 "\n", monthDuration); */
              if (microsecDuration < 0) {
                microsecDuration = -microsecDuration;
                *micro_second = -(microsecDuration % 1000000);
                microsecDuration /= 1000000;
                *second = -(microsecDuration % 60);
                microsecDuration /= 60;
                *minute = -(microsecDuration % 60);
                microsecDuration /= 60;
                *hour = -(microsecDuration % 24);
                microsecDuration /= 24;
                microsecDuration = -microsecDuration;
              } else {
                *micro_second = microsecDuration % 1000000;
                microsecDuration /= 1000000;
                *second = microsecDuration % 60;
                microsecDuration /= 60;
                *minute = microsecDuration % 60;
                microsecDuration /= 60;
                *hour = microsecDuration % 24;
                microsecDuration /= 24;
              } /* if */
              *day = microsecDuration + dayDuration;
              if (monthDuration < 0) {
                monthDuration = -monthDuration;
                *month = -(monthDuration % 12);
                *year = -(monthDuration / 12);
              } else {
                *month = monthDuration % 12;
                *year = monthDuration / 12;
              } /* if */
              break;
            default:
              logError(printf("sqlColumnDuration: Column " FMT_D " has the unknown type %s.\n",
                              column, nameOfBufferType(buffer_type)););
              raise_error(RANGE_ERROR);
              break;
          } /* switch */
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
    int isNull;
    const_cstriType buffer;
    Oid buffer_type;
    floatType columnValue;

  /* sqlColumnFloat */
    logFunction(printf("sqlColumnFloat(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnFloat: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = 0.0;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        logMessage(printf("Column is NULL -> Use default value: 0.0\n"););
        columnValue = 0.0;
      } else if (unlikely(isNull != 0)) {
        dbInconsistent("sqlColumnFloat", "PQgetisnull");
        logError(printf("sqlColumnFloat: Column " FMT_D ": "
                        "PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(DATABASE_ERROR);
        columnValue = 0.0;
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (unlikely(buffer == NULL)) {
          dbInconsistent("sqlColumnFloat", "PQgetvalue");
          logError(printf("sqlColumnFloat: Column " FMT_D ": "
                          "PQgetvalue returns NULL.\n",
                          column););
          raise_error(DATABASE_ERROR);
          columnValue = 0.0;
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          logMessage(printf("buffer_type: %s\n", nameOfBufferType(buffer_type)););
          switch (buffer_type) {
            case INT2OID:
              columnValue = (floatType) (int16Type) ntohs(*(const uint16Type *) buffer);
              break;
            case INT4OID:
              columnValue = (floatType) (int32Type) ntohl(*(const uint32Type *) buffer);
              break;
            case INT8OID:
              columnValue = (floatType) (int64Type) ntohll(*(const uint64Type *) buffer);
              break;
            case CASHOID:
              columnValue = ((floatType) (int64Type) ntohll(*(const uint64Type *) buffer)) / preparedStmt->db->moneyDenominator;
              break;
            case FLOAT4OID:
              columnValue = ntohf(*(const float *) buffer);
              break;
            case FLOAT8OID:
              columnValue = ntohd(*(const double *) buffer);
              break;
            case NUMERICOID:
              columnValue = getNumericAsFloat((const_ustriType) buffer);
              break;
            default:
              logError(printf("sqlColumnFloat: Column " FMT_D " has the unknown type %s.\n",
                              column, nameOfBufferType(buffer_type)););
              raise_error(RANGE_ERROR);
              columnValue = 0.0;
              break;
          } /* switch */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnFloat --> " FMT_E "\n", columnValue););
    return columnValue;
  } /* sqlColumnFloat */



static intType sqlColumnInt (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    int isNull;
    const_cstriType buffer;
    Oid buffer_type;
    intType columnValue;

  /* sqlColumnInt */
    logFunction(printf("sqlColumnInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnInt: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        logMessage(printf("Column is NULL -> Use default value: 0\n"););
        columnValue = 0;
      } else if (unlikely(isNull != 0)) {
        dbInconsistent("sqlColumnInt", "PQgetisnull");
        logError(printf("sqlColumnInt: Column " FMT_D ": "
                        "PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(DATABASE_ERROR);
        columnValue = 0;
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (unlikely(buffer == NULL)) {
          dbInconsistent("sqlColumnInt", "PQgetvalue");
          logError(printf("sqlColumnInt: Column " FMT_D ": "
                          "PQgetvalue returns NULL.\n",
                          column););
          raise_error(DATABASE_ERROR);
          columnValue = 0;
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          logMessage(printf("buffer_type: %s\n", nameOfBufferType(buffer_type)););
          switch (buffer_type) {
            case INT2OID:
              columnValue = (int16Type) ntohs(*(const uint16Type *) buffer);
              break;
            case INT4OID:
              columnValue = (int32Type) ntohl(*(const uint32Type *) buffer);
              break;
            case INT8OID:
              columnValue = (int64Type) ntohll(*(const uint64Type *) buffer);
              break;
            case CASHOID:
              columnValue = ((int64Type) ntohll(*(const uint64Type *) buffer)) / preparedStmt->db->moneyDenominator;
              break;
            case NUMERICOID:
              columnValue = getNumericAsInt((const_ustriType) buffer);
              break;
            case OIDOID:
              columnValue = ntohl(*(const uint32Type *) buffer);
              break;
            default:
              logError(printf("sqlColumnInt: Column " FMT_D " has the unknown type %s.\n",
                              column, nameOfBufferType(buffer_type)););
              raise_error(RANGE_ERROR);
              columnValue = 0;
              break;
          } /* switch */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnInt --> " FMT_D "\n", columnValue););
    return columnValue;
  } /* sqlColumnInt */



static striType sqlColumnStri (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    int isNull;
    const_cstriType buffer;
    Oid buffer_type;
    int length;
    errInfoType err_info = OKAY_NO_ERROR;
    striType columnValue;

  /* sqlColumnStri */
    logFunction(printf("sqlColumnStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnStri: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        logMessage(printf("Column is NULL -> Use default value: \"\"\n"););
        columnValue = strEmpty();
      } else if (unlikely(isNull != 0)) {
        dbInconsistent("sqlColumnStri", "PQgetisnull");
        logError(printf("sqlColumnStri: Column " FMT_D ": "
                        "PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (unlikely(buffer == NULL)) {
          dbInconsistent("sqlColumnStri", "PQgetvalue");
          logError(printf("sqlColumnStri: Column " FMT_D ": "
                          "PQgetvalue returns NULL.\n",
                          column););
          raise_error(DATABASE_ERROR);
          columnValue = NULL;
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          logMessage(printf("buffer_type: %s\n", nameOfBufferType(buffer_type)););
          switch (buffer_type) {
            case NAMEOID:
            case CHAROID:
            case VARCHAROID:
            case TEXTOID:
              length = PQgetlength(preparedStmt->execute_result,
                                   preparedStmt->fetch_index,
                                   (int) column - 1);
              if (unlikely(length < 0)) {
                dbInconsistent("sqlColumnStri", "PQgetlength");
                logError(printf("sqlColumnStri: Column " FMT_D ": "
                                "PQgetlength returns %d.\n",
                                column, length););
                raise_error(DATABASE_ERROR);
                columnValue = NULL;
              } else {
                columnValue = cstri8_buf_to_stri(buffer, (memSizeType) length, &err_info);
                if (unlikely(columnValue == NULL)) {
                  raise_error(err_info);
                } /* if */
              } /* if */
              break;
            case BPCHAROID:
              length = PQgetlength(preparedStmt->execute_result,
                                   preparedStmt->fetch_index,
                                   (int) column - 1);
              if (unlikely(length < 0)) {
                dbInconsistent("sqlColumnStri", "PQgetlength");
                logError(printf("sqlColumnStri: Column " FMT_D ": "
                                "PQgetlength returns %d.\n",
                                column, length););
                raise_error(DATABASE_ERROR);
                columnValue = NULL;
              } else {
                while (length > 0 && buffer[length - 1] == ' ') {
                  length--;
                } /* if */
                columnValue = cstri8_buf_to_stri(buffer, (memSizeType) length, &err_info);
                if (unlikely(columnValue == NULL)) {
                  raise_error(err_info);
                } /* if */
              } /* if */
              break;
            case BYTEAOID:
              length = PQgetlength(preparedStmt->execute_result,
                                   preparedStmt->fetch_index,
                                   (int) column - 1);
              if (unlikely(length < 0)) {
                dbInconsistent("sqlColumnStri", "PQgetlength");
                logError(printf("sqlColumnStri: Column " FMT_D ": "
                                "PQgetlength returns %d.\n",
                                column, length););
                raise_error(DATABASE_ERROR);
                columnValue = NULL;
              } else if (unlikely(!ALLOC_STRI_CHECK_SIZE(columnValue, (memSizeType) length))) {
                raise_error(MEMORY_ERROR);
              } else {
                memcpy_to_strelem(columnValue->mem, (const_ustriType) buffer,
                    (memSizeType) length);
                columnValue->size = (memSizeType) length;
              } /* if */
              break;
            default:
              logError(printf("sqlColumnStri: Column " FMT_D " has the unknown type %s.\n",
                              column, nameOfBufferType(buffer_type)););
              raise_error(RANGE_ERROR);
              columnValue = NULL;
              break;
          } /* switch */
        } /* if */
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
    int isNull;
    const_cstriType buffer;
    Oid buffer_type;
    timeStampType timestamp;
    intType dummy_micro_second;

  /* sqlColumnTime */
    logFunction(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnTime: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
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
      } else if (unlikely(isNull != 0)) {
        dbInconsistent("sqlColumnTime", "PQgetisnull");
        logError(printf("sqlColumnTime: Column " FMT_D ": "
                        "PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(DATABASE_ERROR);
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (unlikely(buffer == NULL)) {
          dbInconsistent("sqlColumnTime", "PQgetvalue");
          logError(printf("sqlColumnTime: Column " FMT_D ": "
                          "PQgetvalue returns NULL.\n",
                          column););
          raise_error(DATABASE_ERROR);
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          logMessage(printf("buffer_type: %s\n", nameOfBufferType(buffer_type)););
          switch (buffer_type) {
            case DATEOID:
              timestamp = (timeStampType) (int32Type) ntohl(*(const uint32Type *) buffer);
              /* printf("DATEOID timestamp: " FMT_D64 "\n", timestamp); */
              timestamp = timestamp * 24 * 60 * 60 + SECONDS_FROM_1970_TO_2000;
              timUtcFromTimestamp(timestamp, year, month, day,
                                  hour, minute, second);
              *micro_second = 0;
              timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                            time_zone, is_dst);
              break;
            case TIMEOID:
              /* The time is either an int64Type representing           */
              /* microseconds away from 2000-01-01 00:00:00 UTC, or a   */
              /* double representing seconds away from the same origin. */
              /* PQparameterStatus(connection, "integer_datetimes") is  */
              /* used to determine if an int64Type or a double is used. */
              if (preparedStmt->integerDatetimes) {
                timestamp = (timeStampType) ntohll(*(const uint64Type *) buffer);
              } else {
                timestamp = (timeStampType)
                    (1000000.0 * ntohd(*(const double *) buffer) + 0.5);
              } /* if */
              /* printf("TIMEOID timestamp: " FMT_D64 "\n", timestamp); */
              timestamp = getTimestamp1970(timestamp, micro_second);
              timUtcFromTimestamp(timestamp, year, month, day,
                                  hour, minute, second);
              timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                            time_zone, is_dst);
              *year = 0;
              *month = 1;
              *day = 1;
              break;
            case TIMETZOID:
              /* The time is either an int64Type representing           */
              /* microseconds away from 2000-01-01 00:00:00 UTC, or a   */
              /* double representing seconds away from the same origin. */
              /* PQparameterStatus(connection, "integer_datetimes") is  */
              /* used to determine if an int64Type or a double is used. */
              if (preparedStmt->integerDatetimes) {
                timestamp = (timeStampType) ntohll(*(const uint64Type *) buffer);
              } else {
                timestamp = (timeStampType)
                    (1000000.0 * ntohd(*(const double *) buffer) + 0.5);
              } /* if */
              /* printf("TIMETZOID timestamp: " FMT_D64 "\n", timestamp); */
              timestamp = getTimestamp1970(timestamp, micro_second);
              timUtcFromTimestamp(timestamp, year, month, day,
                                  hour, minute, second);
              *is_dst = 0;
              /* printf("time_zone: " FMT_D32 "\n",
                  (int32Type) -ntohl(*(const uint32Type *) &buffer[8]) / 60); */
              *time_zone = -ntohl(*(const uint32Type *) &buffer[8]) / 60;
              *year = 0;
              *month = 1;
              *day = 1;
              break;
            case TIMESTAMPOID:
              /* The timestamp is either an int64Type representing      */
              /* microseconds away from 2000-01-01 00:00:00 UTC, or a   */
              /* double representing seconds away from the same origin. */
              /* PQparameterStatus(connection, "integer_datetimes") is  */
              /* used to determine if an int64Type or a double is used. */
              if (preparedStmt->integerDatetimes) {
                timestamp = (timeStampType) ntohll(*(const uint64Type *) buffer);
              } else {
                timestamp = (timeStampType)
                    (1000000.0 * ntohd(*(const double *) buffer) + 0.5);
              } /* if */
              /* printf("TIMESTAMPOID timestamp: " FMT_D64 "\n", timestamp); */
              timestamp = getTimestamp1970(timestamp, micro_second);
              timUtcFromTimestamp(timestamp, year, month, day,
                                  hour, minute, second);
              timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                            time_zone, is_dst);
              break;
            case TIMESTAMPTZOID:
              /* The timestamp is either an int64Type representing      */
              /* microseconds away from 2000-01-01 00:00:00 UTC, or a   */
              /* double representing seconds away from the same origin. */
              /* PQparameterStatus(connection, "integer_datetimes") is  */
              /* used to determine if an int64Type or a double is used. */
              if (preparedStmt->integerDatetimes) {
                timestamp = (timeStampType) ntohll(*(const uint64Type *) buffer);
              } else {
                timestamp = (timeStampType)
                    (1000000.0 * ntohd(*(const double *) buffer) + 0.5);
              } /* if */
              /* printf("TIMESTAMPTZOID timestamp: " FMT_D64 "\n", timestamp); */
              timestamp = getTimestamp1970(timestamp, micro_second);
              *time_zone = 0;
              *is_dst = 0;
              timFromIntTimestamp((intType) timestamp, year, month, day,
                                  hour, minute, second, &dummy_micro_second,
                                  time_zone, is_dst);
              break;
            default:
              logError(printf("sqlColumnTime: Column " FMT_D " has the unknown type %s.\n",
                              column, nameOfBufferType(buffer_type)););
              raise_error(RANGE_ERROR);
              break;
          } /* switch */
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
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlCommit */
    logFunction(printf("sqlCommit(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (unlikely(db->connection == NULL)) {
      dbNotOpen("sqlCommit");
      logError(printf("sqlCommit: Database is not open.\n"););
      raise_error(DATABASE_ERROR);
    } else if (!db->autoCommit) {
      err_info = doExecSql(db->connection, "COMMIT", err_info);
      err_info = doExecSql(db->connection, "BEGIN TRANSACTION", err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunction(printf("sqlCommit -->\n"););
  } /* sqlCommit */



static void sqlExecute (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    errInfoType err_info = OKAY_NO_ERROR;
    int num_tuples;

  /* sqlExecute */
    logFunction(printf("sqlExecute(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!allParametersBound(preparedStmt))) {
      dbLibError("sqlExecute", "SQLExecute",
                 "Unbound statement parameter(s).\n");
      raise_error(DATABASE_ERROR);
    } else {
      preparedStmt->fetchOkay = FALSE;
      if (preparedStmt->execute_result != NULL) {
        PQclear(preparedStmt->execute_result);
      } /* if */
      preparedStmt->execute_result = PQexecPrepared(preparedStmt->db->connection,
                                                    preparedStmt->stmtName,
                                                    (int) preparedStmt->param_array_size,
                                                    (const const_cstriType *) preparedStmt->paramValues,
                                                    preparedStmt->paramLengths,
                                                    preparedStmt->paramFormats,
                                                    1);
      if (unlikely(preparedStmt->execute_result == NULL)) {
        logError(printf("sqlExecute: PQexecPrepared returns NULL\n"););
        preparedStmt->executeSuccessful = FALSE;
        raise_error(MEMORY_ERROR);
      } else {
        preparedStmt->execute_status = PQresultStatus(preparedStmt->execute_result);
        if (preparedStmt->execute_status == PGRES_COMMAND_OK) {
          preparedStmt->executeSuccessful = TRUE;
          if (preparedStmt->implicitCommit && !preparedStmt->db->autoCommit) {
            err_info = doExecSql(preparedStmt->db->connection, "COMMIT", err_info);
            err_info = doExecSql(preparedStmt->db->connection, "BEGIN TRANSACTION", err_info);
            if (unlikely(err_info != OKAY_NO_ERROR)) {
              raise_error(err_info);
            } /* if */
          } /* if */
        } else if (preparedStmt->execute_status == PGRES_TUPLES_OK) {
          num_tuples = PQntuples(preparedStmt->execute_result);
          if (unlikely(num_tuples < 0)) {
            dbInconsistent("sqlExecute", "PQntuples");
            logError(printf("sqlExecute: PQntuples returns negative number: %d\n",
                            num_tuples););
            preparedStmt->executeSuccessful = FALSE;
            raise_error(DATABASE_ERROR);
          } else {
            preparedStmt->num_tuples = num_tuples;
            /* printf("Number of tubles: %d\n", preparedStmt->num_tuples);
              printf("Number of columns: %d\n", PQnfields(preparedStmt->execute_result));
              printf("Result_column_count: " FMT_U_MEM "\n", preparedStmt->result_column_count);
              { int idx;
                for (idx = 0; idx < PQnfields(preparedStmt->execute_result); idx++) {
                  printf("Type of column %d: %d\n", idx, PQftype(preparedStmt->execute_result, idx));
                }
              } */
            preparedStmt->executeSuccessful = TRUE;
            preparedStmt->fetch_index = 0;
            preparedStmt->increment_index = FALSE;
          } /* if */
        } else {
          setDbErrorMsg("sqlExecute", "PQexecPrepared", preparedStmt->db->connection);
          logError(printf("sqlExecute: PQexecPrepared returns a status of %s:\n%s",
                          PQresStatus(preparedStmt->execute_status),
                          dbError.message););
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
    } else if (preparedStmt->result_column_count == 0 ||
               preparedStmt->execute_status == PGRES_COMMAND_OK) {
      preparedStmt->fetchOkay = FALSE;
    } else if (preparedStmt->execute_status == PGRES_TUPLES_OK) {
      if (!preparedStmt->increment_index) {
        preparedStmt->increment_index = TRUE;
      } else {
        preparedStmt->fetch_index++;
      } /* if */
      preparedStmt->fetchOkay =
          preparedStmt->fetch_index < preparedStmt->num_tuples;
    } else {
      preparedStmt->fetchOkay = FALSE;
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
      dbNotOpen("sqlGetAutoCommit");
      logError(printf("sqlGetAutoCommit: Database is not open.\n"););
      raise_error(DATABASE_ERROR);
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
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlIsNull: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      isNull = FALSE;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
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
    PGresult *prepare_result;
    errInfoType err_info = OKAY_NO_ERROR;
    preparedStmtType preparedStmt;

  /* sqlPrepare */
    logFunction(printf("sqlPrepare(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) database,
                       striAsUnquotedCStri(sqlStatementStri)););
    db = (dbType) database;
    if (unlikely(db->connection == NULL)) {
      dbNotOpen("sqlPrepare");
      logError(printf("sqlPrepare: Database is not open.\n"););
      err_info = DATABASE_ERROR;
      preparedStmt = NULL;
    } else {
      statementStri = processStatementStri(sqlStatementStri, &err_info);
      if (statementStri == NULL) {
        preparedStmt = NULL;
      } else {
        query = stri_to_cstri8(statementStri, &err_info);
        if (unlikely(query == NULL)) {
          preparedStmt = NULL;
        } else {
          if (unlikely(!ALLOC_RECORD2(preparedStmt, preparedStmtRecordPost,
                                      count.prepared_stmt, count.prepared_stmt_bytes))) {
            err_info = MEMORY_ERROR;
          } else {
            memset(preparedStmt, 0, sizeof(preparedStmtRecordPost));
            preparedStmt->stmtNum = db->nextStmtNum;
            db->nextStmtNum++;
            sprintf(preparedStmt->stmtName, STMT_NAME_PREFIX FMT_U,
                    preparedStmt->stmtNum);
            prepare_result = PQprepare(db->connection, preparedStmt->stmtName, query, 0, NULL);
            if (unlikely(prepare_result == NULL)) {
              FREE_RECORD2(preparedStmt, preparedStmtRecordPost,
                           count.prepared_stmt, count.prepared_stmt_bytes);
              err_info = MEMORY_ERROR;
              preparedStmt = NULL;
            } else {
              if (PQresultStatus(prepare_result) != PGRES_COMMAND_OK) {
                setDbErrorMsg("sqlPrepare", "PQprepare", db->connection);
                logError(printf("sqlPrepare: PQprepare returns a status of %s:\n%s",
                                PQresStatus(PQresultStatus(prepare_result)),
                                dbError.message););
                FREE_RECORD2(preparedStmt, preparedStmtRecordPost,
                             count.prepared_stmt, count.prepared_stmt_bytes);
                err_info = DATABASE_ERROR;
                preparedStmt = NULL;
              } else {
                preparedStmt->usage_count = 1;
                preparedStmt->sqlFunc = db->sqlFunc;
                preparedStmt->integerDatetimes = db->integerDatetimes;
                preparedStmt->implicitCommit = implicitCommit(query);
                preparedStmt->executeSuccessful = FALSE;
                preparedStmt->execute_result = NULL;
                preparedStmt->fetchOkay = FALSE;
                preparedStmt->db = db;
                if (db->usage_count != 0) {
                  db->usage_count++;
                } /* if */
                err_info = setupParametersAndResult(preparedStmt);
                if (unlikely(err_info != OKAY_NO_ERROR)) {
                  freePreparedStmt((sqlStmtType) preparedStmt);
                  preparedStmt = NULL;
                } /* if */
              } /* if */
              PQclear(prepare_result);
            } /* if */
          } /* if */
          free_cstri8(query, statementStri);
        } /* if */
        FREE_STRI2(statementStri, sqlStatementStri->size * MAX_BIND_VAR_SIZE);
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
    if (unlikely(db->connection == NULL)) {
      dbNotOpen("sqlRollback");
      logError(printf("sqlRollback: Database is not open.\n"););
      raise_error(DATABASE_ERROR);
    } else if (!db->autoCommit) {
      err_info = doExecSql(db->connection, "ROLLBACK", err_info);
      err_info = doExecSql(db->connection, "BEGIN TRANSACTION", err_info);
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
    if (unlikely(db->connection == NULL)) {
      dbNotOpen("sqlSetAutoCommit");
      logError(printf("sqlSetAutoCommit: Database is not open.\n"););
      raise_error(DATABASE_ERROR);
    } else {
      if (db->autoCommit != autoCommit) {
        if (autoCommit) {
          err_info = doExecSql(db->connection, "COMMIT", err_info);
        } else {
          err_info = doExecSql(db->connection, "BEGIN TRANSACTION", err_info);
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
    columnCount = (intType) preparedStmt->result_column_count;
    logFunction(printf("sqlStmtColumnCount --> " FMT_D "\n", columnCount););
    return columnCount;
  } /* sqlStmtColumnCount */



static striType sqlStmtColumnName (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    cstriType col_name;
    errInfoType err_info = OKAY_NO_ERROR;
    striType name;

  /* sqlStmtColumnName */
    logFunction(printf("sqlStmtColumnName(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->executeSuccessful)) {
      dbLibError("sqlStmtColumnName", "PQexecPrepared",
                 "Execute was not successful.\n");
      logError(printf("sqlStmtColumnName: Execute was not successful.\n"););
      raise_error(DATABASE_ERROR);
      name = NULL;
    } else if (unlikely(column < 1 ||
                        (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlStmtColumnName: column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      column, preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      name = NULL;
    } else {
      col_name = PQfname(preparedStmt->execute_result,
                         (int) column - 1);
      if (unlikely(col_name == NULL)) {
        dbInconsistent("sqlStmtColumnName", "PQfname");
        logError(printf("sqlStmtColumnName: Column " FMT_D ": "
                        "PQfname returns NULL.\n",
                        column););
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

static boolType getLocale(dbType database, errInfoType *err_info)
{
  PGresult *execResult;

  // Fetch the locale used for the money type within the current database.
  execResult = PQexec(database->connection, "SHOW lc_monetary");
  if (unlikely(execResult == NULL))
    err_info = MEMORY_ERROR;
  else
  { if (PQresultStatus(execResult) != PGRES_TUPLES_OK)
    { setDbErrorMsg("getLocale", "PQexec", database->connection);
      logError(printf("doExecSql: PQexec(" FMT_U_MEM ", \"%s\") "
                      "returns a status of %s:\n%s",
                      (memSizeType) database->connection, "SHOW lc_monetary",
                      PQresStatus(PQresultStatus(execResult)),
                      dbError.message););
      err_info = DATABASE_ERROR;
    }
    else
    { char *locale = PQgetvalue(execResult, 0, 0);
      setlocale(LC_ALL, locale);
      struct lconv* lc = localeconv();
      database->moneyDenominator = pow(10.0, lc->frac_digits); // This will be 100.0 for dollars/pounds (indicating cents/pence precision).
      logFunction(printf("Money: precision of %d, resulting in a denominator of %f\n", lc->frac_digits, database->moneyDenominator););
      setlocale(LC_ALL, ""); // Return to default.
      return TRUE;
    }
    PQclear(execResult);
  }
  return FALSE;
}

databaseType sqlOpenPost (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  {
    const_cstriType host8;
    const_cstriType dbName8;
    const_cstriType user8;
    const_cstriType password8;
    char portBuffer[INTTYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    cstriType pgport;
    dbRecordPost db;
    const_cstriType setting;
    errInfoType err_info = OKAY_NO_ERROR;
    dbType database;

  /* sqlOpenPost */
    logFunction(printf("sqlOpenPost(\"%s\", ",
                       striAsUnquotedCStri(host));
                printf(FMT_D ", \"%s\", ",
                       port, striAsUnquotedCStri(dbName));
                printf("\"%s\", *)\n", striAsUnquotedCStri(user)););
    if (!findDll()) {
      logError(printf("sqlOpenPost: findDll() failed\n"););
      err_info = DATABASE_ERROR;
      database = NULL;
    } else if (unlikely((host8 = stri_to_cstri8(host, &err_info)) == NULL)) {
      database = NULL;
    } else {
      dbName8 = stri_to_cstri8(dbName, &err_info);
      if (dbName8 == NULL) {
        database = NULL;
      } else {
        user8 = stri_to_cstri8(user, &err_info);
        if (user8 == NULL) {
          database = NULL;
        } else {
          password8 = stri_to_cstri8(password, &err_info);
          if (password8 == NULL) {
            database = NULL;
          } else {
            if (port == 0) {
              /* Use the default value for port, which is probably DEFAULT_PORT. */
              pgport = NULL;
            } else {
              sprintf(portBuffer, FMT_D, port);
              pgport = portBuffer;
            } /* if */
            logMessage(printf("sqlOpenPost: PQsetdbLogin(%s%s%s, "
                              "%s%s%s, NULL, NULL, "
                              "\"%s\", \"%s\", *)\n",
                              host8[0] == '\0' ? "" : "\"",
                              host8[0] == '\0' ? "NULL" : host8,
                              host8[0] == '\0' ? "" : "\"",
                              pgport == NULL ? "" : "\"",
                              pgport == NULL ? "NULL" : pgport,
                              pgport == NULL ? "" : "\"",
                              dbName8, user8););
            db.connection = PQsetdbLogin(host8[0] == '\0' ? NULL : host8,
                                         pgport, NULL /* pgoptions */,
                                         NULL /* pgtty */,
                                         dbName8, user8, password8);
            logMessage(printf("sqlOpenPost: db.connection: " FMT_U_MEM "\n",
                              (memSizeType) db.connection););
            if (unlikely(db.connection == NULL)) {
              logError(printf("sqlOpenPost: PQsetdbLogin(\"%s\", ...  "
                              "\"%s\", \"%s\", *) returns NULL\n",
                              host8[0] == '\0' ? "NULL" : host8,
                              dbName8, user8););
              err_info = MEMORY_ERROR;
              database = NULL;
            } else if (PQstatus(db.connection) != CONNECTION_OK &&
                       host8[0] == '\0') {
              /* The attemt to connect with host set to NULL failed. */
              /* If PQsetdbLogin() is called with the host set to    */
              /* NULL the connection uses Unix domain sockets. Now   */
              /* try internet sockets with "localhost" as host. This */
              /* might succeed if PostgreSQL is installed in a local */
              /* container and the port has been made accessible.    */
              PQfinish(db.connection);
              db.connection = PQsetdbLogin("localhost",
                                           pgport, NULL /* pgoptions */,
                                           NULL /* pgtty */,
                                           dbName8, user8, password8);
              logMessage(printf("sqlOpenPost: db.connection: " FMT_U_MEM "\n",
                                (memSizeType) db.connection););
              if (unlikely(db.connection == NULL)) {
                logError(printf("sqlOpenPost: PQsetdbLogin(\"localhost\","
                                " ... \"%s\", \"%s\", *) returns NULL\n",
                                dbName8, user8););
                err_info = MEMORY_ERROR;
                database = NULL;
              } /* if */
            } /* if */
            if (likely(err_info == OKAY_NO_ERROR)) {
              if (PQstatus(db.connection) != CONNECTION_OK) {
                setDbErrorMsg("sqlOpenPost", "PQsetdbLogin", db.connection);
                logError(printf("sqlOpenPost: PQsetdbLogin(\"%s\", ...  "
                                "\"%s\", \"%s\", *) error:\n"
                                "status=%d\nerror: %s\n",
                                host8[0] == '\0' ? "localhost" : host8,
                                dbName8, user8,
                                PQstatus(db.connection),
                                dbError.message););
                err_info = DATABASE_ERROR;
                PQfinish(db.connection);
                database = NULL;
              } else if (PQsetClientEncoding(db.connection, "UNICODE") != 0) {
                setDbErrorMsg("sqlOpenPost", "PQsetClientEncoding", db.connection);
                logError(printf("sqlOpenPost: PQsetClientEncoding does not return 0:\n%s\n",
                                dbError.message););
                err_info = DATABASE_ERROR;
                PQfinish(db.connection);
                database = NULL;
              } else if (unlikely(!getLocale(&db, &err_info))) {
                PQfinish(db.connection);
                database = NULL;
              } else if (unlikely(!setupFuncTable() ||
                                  !ALLOC_RECORD2(database, dbRecordPost,
                                                 count.database, count.database_bytes))) {
                err_info = MEMORY_ERROR;
                PQfinish(db.connection);
                database = NULL;
              } else {
                memset(database, 0, sizeof(dbRecordPost));
                database->usage_count = 1;
                database->isOpen = TRUE;
                database->sqlFunc = sqlFunc;
                database->driver     = DB_CATEGORY_POSTGRESQL;
                database->dbCategory = DB_CATEGORY_POSTGRESQL;
                database->connection = db.connection;
                setting = PQparameterStatus(db.connection, "integer_datetimes");
                database->integerDatetimes = setting != NULL && strcmp(setting, "on") == 0;
                database->nextStmtNum = 1;
                database->autoCommit = TRUE;
                database->moneyDenominator = db.moneyDenominator;
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
    logFunction(printf("sqlOpenPost --> " FMT_U_MEM "\n",
                       (memSizeType) database););
    return (databaseType) database;
  } /* sqlOpenPost */

#else



databaseType sqlOpenPost (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  { /* sqlOpenPost */
    logError(printf("sqlOpenPost(\"%s\", ",
                    striAsUnquotedCStri(host));
             printf(FMT_D ", \"%s\", ",
                    port, striAsUnquotedCStri(dbName));
             printf("\"%s\", *): PostgreSQL driver not present.\n",
                    striAsUnquotedCStri(user)););
    raise_error(RANGE_ERROR);
    return NULL;
  } /* sqlOpenPost */

#endif
