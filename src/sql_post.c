/********************************************************************/
/*                                                                  */
/*  sql_post.c    Database access functions for PostgreSQL.         */
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
/*  File: seed7/src/sql_post.c                                      */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: Database access functions for PostgreSQL.              */
/*                                                                  */
/********************************************************************/

#include "version.h"

#ifdef POSTGRESQL_INCLUDE
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "limits.h"
#ifdef USE_WINSOCK
#include "winsock2.h"
#else
#include "netinet/in.h"
#endif
#ifdef POSTGRESQL_POSTGRES_H
#include POSTGRESQL_POSTGRES_H
#endif
#include POSTGRESQL_INCLUDE
#include POSTGRESQL_PG_TYPE_H

#include "common.h"
#include "data_rtl.h"
#include "striutl.h"
#include "heaputl.h"
#include "flt_rtl.h"
#include "str_rtl.h"
#include "tim_rtl.h"
#include "sql_util.h"
#include "big_drv.h"
#include "rtl_err.h"
#include "dll_drv.h"
#include "sql_drv.h"

#undef VERBOSE_EXCEPTIONS


#define DECSIZE 30
typedef unsigned char NumericDigit;

typedef struct {
    int ndigits; /* number of digits in digits[] - can be 0! */
    int weight;  /* weight of first digit */
    int rscale;  /* result scale */
    int dscale;  /* display scale */
    int sign;    /* NUMERIC_POS, NUMERIC_NEG, or NUMERIC_NAN */
    NumericDigit *buf; /* start of alloc'd space for digits[] */
    NumericDigit *digits; /* decimal digits */
  } numeric;

typedef struct {
    int ndigits; /* number of digits in digits[] - can be 0! */
    int weight;  /* weight of first digit */
    int rscale;  /* result scale */
    int dscale;  /* display scale */
    int sign;    /* NUMERIC_POS, NUMERIC_NEG, or NUMERIC_NAN */
    NumericDigit digits[DECSIZE]; /* decimal digits */
  } decimal;

typedef struct {
    uintType     usage_count;
    sqlFuncType  sqlFunc;
    PGconn      *connection;
    boolType     integerDatetimes;
    uintType     nextStmtNum;
  } dbRecord, *dbType;

typedef struct {
    uintType       usage_count;
    sqlFuncType    sqlFunc;
    PGconn        *connection;
    boolType       integerDatetimes;
    uintType       stmtNum;
    char           stmtName[30];
    memSizeType    param_array_capacity;
    memSizeType    param_array_size;
    Oid           *paramTypes;
    cstriType     *paramValues;
    int           *paramLengths;
    int           *paramFormats;
    memSizeType    result_column_count;
    boolType       executeSuccessful;
    PGresult      *execute_result;
    ExecStatusType execute_status;
    boolType       fetchOkay;
    memSizeType    num_tuples;
    int            fetch_index;
    boolType       increment_index;
  } preparedStmtRecord, *preparedStmtType;

static sqlFuncType sqlFunc = NULL;

#define MAX_BIND_VAR_SIZE 5
#define MIN_BIND_VAR_NUM 1
#define MAX_BIND_VAR_NUM 9999
/* Seconds between 1970-01-01 and 2000-01-01 */
#define SECONDS_1970_2000 INT64_SUFFIX(946684800)


#ifdef VERBOSE_EXCEPTIONS
#define logError(logStatements) printf(" *** "); logStatements
#else
#define logError(logStatements)
#endif


#ifdef POSTGRESQL_DLL
void (*ptr_PQclear) (PGresult *res);
PGresult *(*ptr_PQdescribePrepared) (PGconn *conn, const char *stmt);
char *(*ptr_PQerrorMessage) (const PGconn *conn);
PGresult *(*ptr_PQexec) (PGconn *conn, const char *query);
PGresult *(*ptr_PQexecPrepared) (PGconn *conn,
                                 const char *stmtName,
                                 int nParams,
                                 const char *const * paramValues,
                                 const int *paramLengths,
                                 const int *paramFormats,
                                 int resultFormat);
void (*ptr_PQfinish) (PGconn *conn);
char *(*ptr_PQfname) (const PGresult *res, int field_num);
Oid (*ptr_PQftype) (const PGresult *res, int field_num);
int (*ptr_PQgetisnull) (const PGresult *res, int tup_num, int field_num);
int (*ptr_PQgetlength) (const PGresult *res, int tup_num, int field_num);
char *(*ptr_PQgetvalue) (const PGresult *res, int tup_num, int field_num);
int (*ptr_PQnfields) (const PGresult *res);
int (*ptr_PQnparams) (const PGresult *res);
int (*ptr_PQntuples) (const PGresult *res);
const char *(*ptr_PQparameterStatus) (const PGconn *conn, const char *paramName);
Oid (*ptr_PQparamtype) (const PGresult *res, int param_num);
PGresult *(*ptr_PQprepare) (PGconn *conn, const char *stmtName,
                            const char *query, int nParams,
                            const Oid *paramTypes);
char *(*ptr_PQresStatus) (ExecStatusType status);
char *(*ptr_PQresultErrorMessage) (const PGresult *res);
ExecStatusType (*ptr_PQresultStatus) (const PGresult *res);
int (*ptr_PQsetClientEncoding) (PGconn *conn, const char *encoding);
PGconn *(*ptr_PQsetdbLogin) (const char *pghost, const char *pgport,
                             const char *pgoptions, const char *pgtty,
                             const char *dbName,
                             const char *login, const char *pwd);
ConnStatusType (*ptr_PQstatus) (const PGconn *conn);

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



static boolType setupDll (const char *dllName)

  {
    static void *dbDll = NULL;

  /* setupDll */
    /* printf("setupDll(\"%s\")\n", dllName); */
    if (dbDll == NULL) {
#ifdef LIBINTL_DLL
      if (dllOpen(LIBINTL_DLL_PATH) != NULL || dllOpen(LIBINTL_DLL) != NULL) {
        dbDll = dllOpen(dllName);
      } /* if */
#else
      dbDll = dllOpen(dllName);
#endif
      if (dbDll != NULL) {
        if ((ptr_PQclear              = dllSym(dbDll, "PQclear"))              == NULL ||
            (ptr_PQdescribePrepared   = dllSym(dbDll, "PQdescribePrepared"))   == NULL ||
            (ptr_PQerrorMessage       = dllSym(dbDll, "PQerrorMessage"))       == NULL ||
            (ptr_PQexec               = dllSym(dbDll, "PQexec"))               == NULL ||
            (ptr_PQexecPrepared       = dllSym(dbDll, "PQexecPrepared"))       == NULL ||
            (ptr_PQfinish             = dllSym(dbDll, "PQfinish"))             == NULL ||
            (ptr_PQfname              = dllSym(dbDll, "PQfname"))              == NULL ||
            (ptr_PQftype              = dllSym(dbDll, "PQftype"))              == NULL ||
            (ptr_PQgetisnull          = dllSym(dbDll, "PQgetisnull"))          == NULL ||
            (ptr_PQgetlength          = dllSym(dbDll, "PQgetlength"))          == NULL ||
            (ptr_PQgetvalue           = dllSym(dbDll, "PQgetvalue"))           == NULL ||
            (ptr_PQnfields            = dllSym(dbDll, "PQnfields"))            == NULL ||
            (ptr_PQnparams            = dllSym(dbDll, "PQnparams"))            == NULL ||
            (ptr_PQntuples            = dllSym(dbDll, "PQntuples"))            == NULL ||
            (ptr_PQparameterStatus    = dllSym(dbDll, "PQparameterStatus"))    == NULL ||
            (ptr_PQparamtype          = dllSym(dbDll, "PQparamtype"))          == NULL ||
            (ptr_PQprepare            = dllSym(dbDll, "PQprepare"))            == NULL ||
            (ptr_PQresStatus          = dllSym(dbDll, "PQresStatus"))          == NULL ||
            (ptr_PQresultErrorMessage = dllSym(dbDll, "PQresultErrorMessage")) == NULL ||
            (ptr_PQresultStatus       = dllSym(dbDll, "PQresultStatus"))       == NULL ||
            (ptr_PQsetClientEncoding  = dllSym(dbDll, "PQsetClientEncoding"))  == NULL ||
            (ptr_PQsetdbLogin         = dllSym(dbDll, "PQsetdbLogin"))         == NULL ||
            (ptr_PQstatus             = dllSym(dbDll, "PQstatus"))             == NULL) {
          dbDll = NULL;
        } /* if */
      } /* if */
    } /* if */
    /* printf("setupDll --> %d\n", dbDll != NULL); */
    return dbDll != NULL;
  } /* setupDll */

#else

#define setupDll(dllName) TRUE
#define POSTGRESQL_DLL ""

#endif

#ifndef POSTGRESQL_DLL_PATH
#define POSTGRESQL_DLL_PATH ""
#endif



#ifdef BIG_ENDIAN_INTTYPE

#define htonll(n) (n)
#define ntohll(n) (n)
#define htonf(n) (n)
#define ntohf(n) (n)
#define htond(n) (n)
#define ntohd(n) (n)

#else

#define htonll(n) swap64(n)
#define ntohll(n) swap64(n)
#define htonf(n) swap32float(n)
#define ntohf(n) swap32float(n)
#define htond(n) swap64double(n)
#define ntohd(n) swap64double(n)

static inline uint64Type swap64 (uint64Type n)
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



static PGresult *PQdeallocate (PGconn *conn, const char *stmtName)

  {
    const char *const deallocateCommand = "DEALLOCATE ";
    memSizeType length;
    char *command;
    PGresult *deallocate_result;

  /* PQdeallocate */
    /* printf("PQdeallocate(%lx, %s)\n", conn, stmtName); */
    length = strlen(deallocateCommand) + strlen(stmtName) + 1;
    command = malloc(length);
    if (command == NULL) {
      deallocate_result = NULL;
    } else {
      strcpy(command, deallocateCommand);
      strcat(command, stmtName);
      deallocate_result = PQexec(conn, command);
      free(command);
    } /* if */
    return deallocate_result;
  } /* PQdeallocate */



static void freeDatabase (databaseType database)

  {
    dbType db;

  /* freeDatabase */
    sqlClose(database);
    db = (dbType) database;
    FREE_RECORD(db, dbRecord, count.database);
  } /* freeDatabase */



static void freePreparedStmt (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    static PGresult *deallocate_result;

  /* freePreparedStmt */
    /* printf("begin freePreparedStmt(%lx)\n", (unsigned long) sqlStatement); */
    preparedStmt = (preparedStmtType) sqlStatement;
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
    deallocate_result = PQdeallocate(preparedStmt->connection, preparedStmt->stmtName);
    if (deallocate_result != NULL) {
      PQclear(deallocate_result);
    } /* if */
    FREE_RECORD(preparedStmt, preparedStmtRecord, count.prepared_stmt);
    /* printf("end freePreparedStmt\n"); */
  } /* freePreparedStmt */



static const char *nameOfBufferType (Oid buffer_type)

  {
    static char buffer[50];
    const char *typeName;

  /* nameOfBufferType */
    /* printf("nameOfBuffer(%d)\n", buffer_type); */
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
      case ABSTIMEOID:          typeName = "ABSTIMEOID"; break;
      case RELTIMEOID:          typeName = "RELTIMEOID"; break;
      case TINTERVALOID:        typeName = "TINTERVALOID"; break;
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
      case OPAQUEOID:           typeName = "OPAQUEOID"; break;
      case ANYELEMENTOID:       typeName = "ANYELEMENTOID"; break;
      case ANYNONARRAYOID:      typeName = "ANYNONARRAYOID"; break;
      case ANYENUMOID:          typeName = "ANYENUMOID"; break;
#ifdef FDW_HANDLEROID
      case FDW_HANDLEROID:      typeName = "FDW_HANDLEROID"; break;
#endif
      default:
        sprintf(buffer, "%d", buffer_type);
        typeName = buffer;
        break;
    } /* switch */
    /* printf("nameOfBufferType --> %s\n", typeName); */
    return typeName;
  } /* nameOfBufferType */



static striType processBindVarsInStatement (const const_striType sqlStatementStri,
    errInfoType *err_info)

  {
    memSizeType pos = 0;
    strElemType ch;
    memSizeType destPos = 0;
    unsigned int varNum = MIN_BIND_VAR_NUM;
    striType processed;

  /* processBindVarsInStatement */
    if (unlikely(sqlStatementStri->size > MAX_STRI_LEN / MAX_BIND_VAR_SIZE ||
                 !ALLOC_STRI_SIZE_OK(processed, sqlStatementStri->size * MAX_BIND_VAR_SIZE))) {
      *err_info = MEMORY_ERROR;
      processed = NULL;
    } else {
      while (pos < sqlStatementStri->size && *err_info == OKAY_NO_ERROR) {
        ch = sqlStatementStri->mem[pos];
        /* printf("%c", ch); */
        if (ch == '?') {
          if (varNum > MAX_BIND_VAR_NUM) {
            logError(printf("processBindVarsInStatement: Too many variables\n"););
            *err_info = RANGE_ERROR;
            FREE_STRI(processed, sqlStatementStri->size * MAX_BIND_VAR_SIZE);
            processed = NULL;
          } else {
            processed->mem[destPos] = '$';
            destPos++;
            if (varNum >= 1000) {
              processed->mem[destPos] = '0' + ( varNum / 1000);
              destPos++;
            } /* if */
            if (varNum >= 100) {
              processed->mem[destPos] = '0' + ((varNum /  100) % 10);
              destPos++;
            } /* if */
            if (varNum >= 10) {
              processed->mem[destPos] = '0' + ((varNum /   10) % 10);
              destPos++;
            } /* if */
            processed->mem[destPos] = '0' + ( varNum         % 10);
            destPos++;
            varNum++;
          } /* if */
          pos++;
        } else if (ch == '\'') {
          processed->mem[destPos] = '\'';
          destPos++;
          pos++;
          while (pos < sqlStatementStri->size && (ch = sqlStatementStri->mem[pos]) != '\'') {
            /* printf("%c", ch); */
            processed->mem[destPos] = ch;
            destPos++;
            pos++;
          } /* while */
          if (pos < sqlStatementStri->size) {
            /* printf("%c", ch); */
            processed->mem[destPos] = '\'';
            destPos++;
            pos++;
          } /* if */
        } else {
          processed->mem[destPos] = ch;
          destPos++;
          pos++;
        } /* if */
      } /* while */
      processed->size = destPos;
      /* printf("\n"); */
    } /* if */
    return processed;
  } /* processBindVarsInStatement */



static void setupParameters (preparedStmtType preparedStmt,
    errInfoType *err_info)

  {
    PGresult *describe_result;
    int num_params;
    int param_number;

  /* setupParameters */
    /* printf("begin setupParameters\n"); */
    describe_result = PQdescribePrepared(preparedStmt->connection, preparedStmt->stmtName);
    if (describe_result == NULL) {
      *err_info = MEMORY_ERROR;
    } else {
      num_params = PQnparams(describe_result);
      if (num_params < 0) {
        *err_info = FILE_ERROR;
      } else {
        preparedStmt->param_array_size = (memSizeType) num_params;
        if (!ALLOC_TABLE(preparedStmt->paramTypes, Oid, preparedStmt->param_array_size) ||
            !ALLOC_TABLE(preparedStmt->paramValues, cstriType, preparedStmt->param_array_size) ||
            !ALLOC_TABLE(preparedStmt->paramLengths, int, preparedStmt->param_array_size) ||
            !ALLOC_TABLE(preparedStmt->paramFormats, int, preparedStmt->param_array_size)) {
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
          preparedStmt->paramTypes = NULL;
          preparedStmt->paramValues = NULL;
          preparedStmt->paramLengths = NULL;
          preparedStmt->paramFormats = NULL;
          *err_info = MEMORY_ERROR;
        } else {
          for (param_number = 0; param_number < num_params; param_number++) {
            preparedStmt->paramTypes[param_number] = PQparamtype(describe_result, param_number);
            /* printf("paramType: %s\n",
               nameOfBufferType(preparedStmt->paramTypes[param_number])); */
            preparedStmt->paramValues[param_number] = NULL;
            preparedStmt->paramLengths[param_number] = 0;
            preparedStmt->paramFormats[param_number] = 0;
          } /* if */
        } /* if */
      } /* if */
      PQclear(describe_result);
    } /* if */
    /* printf("end setupParameters\n"); */
  } /* setupParameters */



static void setupResult (preparedStmtType preparedStmt,
    errInfoType *err_info)

  {
    PGresult *describe_result;
    int num_columns;
    /* int column_index; */

  /* setupResult */
    /* printf("begin setupResult\n"); */
      describe_result = PQdescribePrepared(preparedStmt->connection, preparedStmt->stmtName);
      if (describe_result == NULL) {
        *err_info = MEMORY_ERROR;
      } else {
        num_columns = PQnfields(describe_result);
        if (num_columns < 0) {
          *err_info = FILE_ERROR;
        } else {
          preparedStmt->result_column_count = (memSizeType) num_columns;
          /* for (column_index = 0; column_index < num_columns &&
               *err_info == OKAY_NO_ERROR; column_index++) {
            printf("Column %d name: %s\n", column_index + 1, PQfname(describe_result, column_index));
            printf("Column %d type: %d\n", column_index + 1, PQftype(describe_result, column_index));
          } */
        } /* if */
        PQclear(describe_result);
      } /* if */
    /* printf("end setupResult\n"); */
  } /* setupResult */



static void dumpNumeric (const unsigned char *buffer)

  {
    numeric numStruct;
    int idx;

  /* dumpNumeric */
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
    }
  } /* dumpNumeric */



static bigIntType getNumericBigInt (const unsigned char *buffer)

  {
    numeric numStruct;
    striType stri;
    unsigned int idx;
    unsigned int fourDigits;
    unsigned int digitNum;
    bigIntType powerOfTen;
    bigIntType bigIntValue;

  /* getNumericBigInt */
    /* dumpNumeric(buffer); */
    numStruct.ndigits = 256 * buffer[0] + buffer[1];
    numStruct.weight  = (int16Type) (buffer[2] << 8 | buffer[3]);
    numStruct.rscale  = 256 * (buffer[4] & 0x3f) + buffer[5];
    numStruct.dscale  = 256 * (buffer[6] & 0x3f) + buffer[7];
    numStruct.sign    = (buffer[4] & 0x40) != 0;
    if (numStruct.ndigits == 0) {
      bigIntValue = bigZero();
    } else if (numStruct.weight < numStruct.ndigits - 1 ||
               numStruct.rscale != 0 ||
               numStruct.dscale != 0) {
      logError(printf("getNumericBigInt: Number not integer\n"););
      raise_error(RANGE_ERROR);
      bigIntValue = NULL;
    } else if (unlikely(!ALLOC_STRI_SIZE_OK(stri, 4 * (memSizeType) numStruct.ndigits + 1))) {
      raise_error(MEMORY_ERROR);
      bigIntValue = NULL;
    } else {
      stri->size = 4 * (memSizeType) numStruct.ndigits + 1;
      stri->mem[0] = numStruct.sign ? '-' : '+';
      for (idx = 0; idx < numStruct.ndigits; idx++) {
        fourDigits = 256 * (unsigned int) buffer[8 + 2 * idx] +
                           (unsigned int) buffer[9 + 2 * idx];
        for (digitNum = 4; digitNum >= 1; digitNum--) {
          stri->mem[4 * idx + digitNum] = (strElemType) (fourDigits % 10 + '0');
          fourDigits /= 10;
        } /* for */
      } /* for */
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
    /* printf("getNumericBigInt --> ");
       if (bigIntValue == NULL) {
         printf("NULL");
       } else {
         prot_stri_unquoted(bigStr(bigIntValue));
       }
       printf("\n"); */
    return bigIntValue;
  } /* getNumericBigInt */



static bigIntType getNumericBigRat (const unsigned char *buffer,
    bigIntType *denominator)

  {
    numeric numStruct;
    striType stri;
    unsigned int idx;
    unsigned int fourDigits;
    unsigned int digitNum;
    bigIntType powerOfTen;
    bigIntType numerator;

  /* getNumericBigRat */
    /* dumpNumeric(buffer); */
    numStruct.ndigits = 256 * buffer[0] + buffer[1];
    numStruct.weight  = (int16Type) (buffer[2] << 8 | buffer[3]);
    numStruct.rscale  = 256 * (buffer[4] & 0x3f) + buffer[5];
    numStruct.dscale  = 256 * (buffer[6] & 0x3f) + buffer[7];
    numStruct.sign    = (buffer[4] & 0x40) != 0;
    if (numStruct.ndigits == 0) {
      numerator = bigZero();
      *denominator = bigFromInt32(1);
    } else if (unlikely(!ALLOC_STRI_SIZE_OK(stri, 4 * (memSizeType) numStruct.ndigits + 1))) {
      raise_error(MEMORY_ERROR);
      numerator = NULL;
    } else {
      stri->size = 4 * (memSizeType) numStruct.ndigits + 1;
      stri->mem[0] = numStruct.sign ? '-' : '+';
      for (idx = 0; idx < numStruct.ndigits; idx++) {
        fourDigits = 256 * (unsigned int) buffer[8 + 2 * idx] +
                           (unsigned int) buffer[9 + 2 * idx];
        for (digitNum = 4; digitNum >= 1; digitNum--) {
          stri->mem[4 * idx + digitNum] = (strElemType) (fourDigits % 10 + '0');
          fourDigits /= 10;
        } /* for */
      } /* for */
      /* printf("stri: ");
         prot_stri(stri);
         printf("\n"); */
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
    return numerator;
  } /* getNumericBigRat */



static void getInterval (int64Type interval,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second)

  { /* getInterval */
    /* printf("getInterval(" FMT_D64 ")\n", interval); */
    *micro_second = interval % 1000000;
    interval /= 1000000;
    *second = interval % 60;
    interval /= 60;
    *minute = interval % 60;
    interval /= 60;
    *hour = interval % 24;
    interval /= 24;
    *year = interval / 365;
    interval %= 365;
    *month = interval / 30;
    interval %= 30;
    *day = interval;
  } /* getInterval */



static void getTimestamp (int64Type timestamp,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second,
    intType *time_zone, boolType *is_dst)

  {
    intType dummy_micro_second;

  /* getTimestamp */
    /* printf("getTimestamp(" FMT_D64 ")\n", timestamp); */
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
    if (timestamp > TIME_T_MAX - SECONDS_1970_2000) {
      raise_error(RANGE_ERROR);
    } else {
      timestamp += SECONDS_1970_2000;
      /* pprintf("timestamp1970: " FMT_U64 "\n", timestamp); */
      if (!inTimeTRange(timestamp)) {
        raise_error(RANGE_ERROR);
      } else {
        timFromTimestamp((time_t) timestamp, year, month, day,
            hour, minute, second, &dummy_micro_second, time_zone, is_dst);
      } /* if */
    } /* if */
  } /* getTimestamp */



static void sqlBindBigInt (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType value)

  {
    preparedStmtType preparedStmt;
    memSizeType length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBigInt */
    /* printf("sqlBindBigInt(%lx, " FMT_D ", %s)\n",
       (unsigned long) sqlStatement, pos, bigHexCStri(value)); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBigInt: pos: " FMT_D ", max pos: %lu.\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      /* printf("paramType: %s\n",
         nameOfBufferType(preparedStmt->paramTypes[pos - 1])); */
      switch (preparedStmt->paramTypes[pos - 1]) {
        case INT2OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(int16Type));
          *(int16Type *) preparedStmt->paramValues[pos - 1] =
              (int16Type) htons((uint16Type) bigToInt16(value));
          preparedStmt->paramLengths[pos - 1] = sizeof(int16Type);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case INT4OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(int32Type));
          *(int32Type *) preparedStmt->paramValues[pos - 1] =
              (int32Type) htonl((uint32Type) bigToInt32(value));
          preparedStmt->paramLengths[pos - 1] = sizeof(int32Type);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case INT8OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(int64Type));
          *(int64Type *) preparedStmt->paramValues[pos - 1] =
              (int64Type) htonll((uint64Type) bigToInt64(value));
          preparedStmt->paramLengths[pos - 1] = sizeof(int64Type);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case FLOAT4OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(float));
          *(float *) preparedStmt->paramValues[pos - 1] =
              htonf((float) bigIntToDouble(value));
          preparedStmt->paramLengths[pos - 1] = sizeof(float);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case FLOAT8OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(double));
          *(double *) preparedStmt->paramValues[pos - 1] =
              htond(bigIntToDouble(value));
          preparedStmt->paramLengths[pos - 1] = sizeof(double);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case NUMERICOID:
        case BPCHAROID:
        case VARCHAROID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = (cstriType)
              bigIntToDecimal(value, &length, &err_info);
          if (err_info != OKAY_NO_ERROR) {
            raise_error(err_info);
          } else if (length > INT_MAX) {
            raise_error(MEMORY_ERROR);
          } else {
            preparedStmt->paramLengths[pos - 1] = (int) length;
            preparedStmt->paramFormats[pos - 1] = 0;
          } /* if */
          break;
        default:
          logError(printf("sqlBindBigInt: Pos " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          raise_error(RANGE_ERROR);
          break;
      } /* switch */
      preparedStmt->executeSuccessful = FALSE;
      preparedStmt->fetchOkay = FALSE;
    } /* if */
  } /* sqlBindBigInt */



static void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType numerator, const const_bigIntType denominator)

  {
    preparedStmtType preparedStmt;
    memSizeType length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBigRat */
    /* printf("sqlBindBigRat(%lx, " FMT_D ", %s/%s)\n",
       (unsigned long) sqlStatement, pos,
       bigHexCStri(numerator), bigHexCStri(denominator)); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBigRat: pos: " FMT_D ", max pos: %lu.\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      /* printf("paramType: %s\n",
         nameOfBufferType(preparedStmt->paramTypes[pos - 1])); */
      switch (preparedStmt->paramTypes[pos - 1]) {
        case FLOAT4OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(float));
          *(float *) preparedStmt->paramValues[pos - 1] =
              htonf((float) bigRatToDouble(numerator, denominator));
          preparedStmt->paramLengths[pos - 1] = sizeof(float);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case FLOAT8OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(double));
          *(double *) preparedStmt->paramValues[pos - 1] =
              htond(bigRatToDouble(numerator, denominator));
          preparedStmt->paramLengths[pos - 1] = sizeof(double);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case NUMERICOID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = (cstriType)
              bigRatToDecimal(numerator, denominator, 1000, &length, &err_info);
          if (err_info != OKAY_NO_ERROR) {
            raise_error(err_info);
          } else {
            preparedStmt->paramLengths[pos - 1] = (int) length;
            preparedStmt->paramFormats[pos - 1] = 0;
          } /* if */
          break;
        default:
          logError(printf("sqlBindBigRat: Pos " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          raise_error(RANGE_ERROR);
          break;
      } /* switch */
      preparedStmt->executeSuccessful = FALSE;
      preparedStmt->fetchOkay = FALSE;
    } /* if */
  } /* sqlBindBigRat */



static void sqlBindBool (sqlStmtType sqlStatement, intType pos, boolType value)

  {
    preparedStmtType preparedStmt;

  /* sqlBindBool */
    /* printf("sqlBindBool(%lx, " FMT_D ", %ld)\n",
       (unsigned long) sqlStatement, pos, value); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBool: pos: " FMT_D ", max pos: %lu.\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      /* printf("paramType: %s\n",
         nameOfBufferType(preparedStmt->paramTypes[pos - 1])); */
      switch (preparedStmt->paramTypes[pos - 1]) {
        case INT2OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(int16Type));
          *(int16Type *) preparedStmt->paramValues[pos - 1] =
              (int16Type) htons((uint16Type) value);
          preparedStmt->paramLengths[pos - 1] = sizeof(int16Type);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case INT4OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(int32Type));
          *(int32Type *) preparedStmt->paramValues[pos - 1] =
              (int32Type) htonl((uint32Type) value);
          preparedStmt->paramLengths[pos - 1] = sizeof(int32Type);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case INT8OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(int64Type));
          *(int64Type *) preparedStmt->paramValues[pos - 1] =
              (int64Type) htonll((uint64Type) value);
          preparedStmt->paramLengths[pos - 1] = sizeof(int64Type);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case NUMERICOID:
        case BPCHAROID:
        case VARCHAROID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(2);
          ((char *) preparedStmt->paramValues[pos - 1])[0] = (char) ('0' + value);
          ((char *) preparedStmt->paramValues[pos - 1])[1] = '\0';
          preparedStmt->paramLengths[pos - 1] = 1;
          preparedStmt->paramFormats[pos - 1] = 0;
          break;
        default:
          logError(printf("sqlBindBool: Pos " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          raise_error(RANGE_ERROR);
          break;
      } /* switch */
      preparedStmt->executeSuccessful = FALSE;
      preparedStmt->fetchOkay = FALSE;
    } /* if */
  } /* sqlBindBool */



static void sqlBindBStri (sqlStmtType sqlStatement, intType pos, bstriType bstri)

  {
    preparedStmtType preparedStmt;

  /* sqlBindBStri */
    /* printf("sqlBindBStri(%lx, " FMT_D ", ",
       (unsigned long) sqlStatement, pos);
       prot_bstri(bstri);
       printf(")\n"); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBStri: pos: " FMT_D ", max pos: %lu.\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      /* printf("paramType: %s\n",
         nameOfBufferType(preparedStmt->paramTypes[pos - 1])); */
      switch (preparedStmt->paramTypes[pos - 1]) {
        case BYTEAOID:
        case BPCHAROID:
        case VARCHAROID:
          if (bstri->size > ULONG_MAX) {
            raise_error(MEMORY_ERROR);
          } else {
            free(preparedStmt->paramValues[pos - 1]);
            if ((preparedStmt->paramValues[pos - 1] = malloc(bstri->size + 1)) == NULL) {
              raise_error(MEMORY_ERROR);
            } else {
              memcpy(preparedStmt->paramValues[pos - 1], bstri->mem, bstri->size);
              preparedStmt->paramValues[pos - 1][bstri->size] = '\0';
              preparedStmt->paramLengths[pos - 1] = (int) bstri->size;
              preparedStmt->paramFormats[pos - 1] = 1;
            } /* if */
          } /* if */
          break;
        default:
          logError(printf("sqlBindBStri: Pos " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          raise_error(RANGE_ERROR);
          break;
      } /* switch */
      preparedStmt->executeSuccessful = FALSE;
      preparedStmt->fetchOkay = FALSE;
    } /* if */
  } /* sqlBindBStri */



static void sqlBindDuration (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  {
    preparedStmtType preparedStmt;

  /* sqlBindDuration */
    /* printf("sqlBindDuration(%lx, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld)\n",
       sqlStatement, pos, year, month, day, hour, minute, second, micro_second); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindDuration: pos: " FMT_D ", max pos: %lu.\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else if (year < INT32TYPE_MIN || year > INT32TYPE_MAX || month < -12 || month > 12 ||
               day < -31 || day > 31 || hour < -24 || hour > 24 || minute < -60 || minute > 60 ||
               second < -60 || second > 60 || micro_second < -1000000 || micro_second > 1000000) {
      logError(printf("sqlBindDuration: Duration not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else if (!((year >= 0 && month >= 0 && day >= 0 && hour >= 0 &&
                  minute >= 0 && second >= 0 && micro_second >= 0) ||
                 (year <= 0 && month <= 0 && day <= 0 && hour <= 0 &&
                  minute <= 0 && second <= 0 && micro_second <= 0))) {
      logError(printf("sqlBindDuration: Duration neither clearly positive nor negative.\n"););
      raise_error(RANGE_ERROR);
    } else {
      /* printf("paramType: %s\n",
         nameOfBufferType(preparedStmt->paramTypes[pos - 1])); */
      switch (preparedStmt->paramTypes[pos - 1]) {
#if 0
        case INTERVALOID:
          /* The interval is either an int64Type representing       */
          /* microseconds, or a double representing seconds.        */
          /* PQparameterStatus(connecton, "integer_datetimes") is   */
          /* used to determine if an int64Type or a double is used. */
          if (preparedStmt->integerDatetimes) {
            getInterval((int64Type) ntohll(*(uint64Type *) buffer),
                        year, month, day, hour, minute, second, micro_second);
          } else {
            getInterval((int64Type) (1000000.0 * ntohd(*(double *) buffer) + 0.5),
                        year, month, day, hour, minute, second, micro_second);
          } /* if */
#endif
        default:
          logError(printf("sqlBindDuration: Pos " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          raise_error(RANGE_ERROR);
          break;
      } /* switch */
      preparedStmt->executeSuccessful = FALSE;
      preparedStmt->fetchOkay = FALSE;
    } /* if */
  } /* sqlBindDuration */



static void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value)

  {
    preparedStmtType preparedStmt;

  /* sqlBindFloat */
    /* printf("sqlBindFloat(%lx, " FMT_D ", " FMT_E ")\n",
       (unsigned long) sqlStatement, pos, value); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindFloat: pos: " FMT_D ", max pos: %lu.\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      /* printf("paramType: %s\n",
         nameOfBufferType(preparedStmt->paramTypes[pos - 1])); */
      switch (preparedStmt->paramTypes[pos - 1]) {
        case FLOAT4OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(float));
          *(float *) preparedStmt->paramValues[pos - 1] = htonf((float) value);
          preparedStmt->paramLengths[pos - 1] = sizeof(float);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case FLOAT8OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(double));
          *(double *) preparedStmt->paramValues[pos - 1] = htond(value);
          preparedStmt->paramLengths[pos - 1] = sizeof(double);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        default:
          logError(printf("sqlBindFloat: Pos " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          raise_error(RANGE_ERROR);
          break;
      } /* switch */
      preparedStmt->executeSuccessful = FALSE;
      preparedStmt->fetchOkay = FALSE;
    } /* if */
  } /* sqlBindFloat */



static void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value)

  {
    preparedStmtType preparedStmt;

  /* sqlBindInt */
    /* printf("sqlBindInt(%lx, " FMT_D ", " FMT_D ")\n",
       (unsigned long) sqlStatement, pos, value); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindInt: pos: " FMT_D ", max pos: %lu.\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      /* printf("paramType: %s\n",
         nameOfBufferType(preparedStmt->paramTypes[pos - 1])); */
      switch (preparedStmt->paramTypes[pos - 1]) {
        case INT2OID:
          if (value < INT16TYPE_MIN || value > INT16TYPE_MAX) {
            raise_error(RANGE_ERROR);
          } else {
            free(preparedStmt->paramValues[pos - 1]);
            preparedStmt->paramValues[pos - 1] = malloc(sizeof(int16Type));
            *(int16Type *) preparedStmt->paramValues[pos - 1] =
                (int16Type) htons((uint16Type) value);
            preparedStmt->paramLengths[pos - 1] = sizeof(int16Type);
            preparedStmt->paramFormats[pos - 1] = 1;
          } /* if */
          break;
        case INT4OID:
          if (value < INT32TYPE_MIN || value > INT32TYPE_MAX) {
            raise_error(RANGE_ERROR);
          } else {
            free(preparedStmt->paramValues[pos - 1]);
            preparedStmt->paramValues[pos - 1] = malloc(sizeof(int32Type));
            *(int32Type *) preparedStmt->paramValues[pos - 1] =
                (int32Type) htonl((uint32Type) value);
            preparedStmt->paramLengths[pos - 1] = sizeof(int32Type);
            preparedStmt->paramFormats[pos - 1] = 1;
          } /* if */
          break;
        case INT8OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(int64Type));
          *(int64Type *) preparedStmt->paramValues[pos - 1] =
              (int64Type) htonll((uint64Type) value);
          preparedStmt->paramLengths[pos - 1] = sizeof(int64Type);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case FLOAT4OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(float));
          *(float *) preparedStmt->paramValues[pos - 1] = htonf((float) value);
          preparedStmt->paramLengths[pos - 1] = sizeof(float);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case FLOAT8OID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(sizeof(double));
          *(double *) preparedStmt->paramValues[pos - 1] = htond((double) value);
          preparedStmt->paramLengths[pos - 1] = sizeof(double);
          preparedStmt->paramFormats[pos - 1] = 1;
          break;
        case NUMERICOID:
        case BPCHAROID:
        case VARCHAROID:
          free(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramValues[pos - 1] = malloc(INTTYPE_DECIMAL_SIZE + 1);
          sprintf(preparedStmt->paramValues[pos - 1], FMT_D, value);
          preparedStmt->paramLengths[pos - 1] = (int) strlen(preparedStmt->paramValues[pos - 1]);
          preparedStmt->paramFormats[pos - 1] = 0;
          break;
        default:
          logError(printf("sqlBindInt: Pos " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          raise_error(RANGE_ERROR);
          break;
      } /* switch */
      preparedStmt->executeSuccessful = FALSE;
      preparedStmt->fetchOkay = FALSE;
    } /* if */
  } /* sqlBindInt */



static void sqlBindNull (sqlStmtType sqlStatement, intType pos)

  {
    preparedStmtType preparedStmt;

  /* sqlBindNull */
    /* printf("sqlBindNull(%lx, %d)\n",
       (unsigned long) sqlStatement, pos); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindNull: pos: " FMT_D ", max pos: %lu.\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      /* printf("paramType: %s\n",
         nameOfBufferType(preparedStmt->paramTypes[pos - 1])); */
      free(preparedStmt->paramValues[pos - 1]);
      preparedStmt->paramValues[pos - 1] = NULL;
      preparedStmt->executeSuccessful = FALSE;
      preparedStmt->fetchOkay = FALSE;
    } /* if */
  } /* sqlBindNull */



static void sqlBindStri (sqlStmtType sqlStatement, intType pos, striType stri)

  {
    preparedStmtType preparedStmt;
    cstriType stri8;
    memSizeType length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindStri */
    /* printf("sqlBindStri(%lx, " FMT_D ", ",
       (unsigned long) sqlStatement, pos);
       prot_stri(stri);
       printf(")\n"); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindStri: pos: " FMT_D ", max pos: %lu.\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      /* printf("paramType: %s\n",
         nameOfBufferType(preparedStmt->paramTypes[pos - 1])); */
      switch (preparedStmt->paramTypes[pos - 1]) {
        case BPCHAROID:
        case VARCHAROID:
          stri8 = stri_to_cstri8_buf(stri, &length, &err_info);
          if (stri8 == NULL) {
            raise_error(err_info);
          } else if (length > ULONG_MAX) {
            free(stri8);
            raise_error(MEMORY_ERROR);
          } else {
            free(preparedStmt->paramValues[pos - 1]);
            preparedStmt->paramValues[pos - 1] = stri8;
            preparedStmt->paramLengths[pos - 1] = (int) length;
            preparedStmt->paramFormats[pos - 1] = 1;
          } /* if */
          break;
        case BYTEAOID:
          if (stri->size > ULONG_MAX) {
            raise_error(MEMORY_ERROR);
          } else {
            free(preparedStmt->paramValues[pos - 1]);
            preparedStmt->paramValues[pos - 1] = stri_to_cstri(stri, &err_info);
            if (preparedStmt->paramValues[pos - 1] == NULL) {
              raise_error(err_info);
            } else {
              preparedStmt->paramLengths[pos - 1] = (int) stri->size;
              preparedStmt->paramFormats[pos - 1] = 1;
            } /* if */
          } /* if */
          break;
        default:
          logError(printf("sqlBindStri: Pos " FMT_D " has the unknown type %s.\n",
                          pos, nameOfBufferType(preparedStmt->paramTypes[pos - 1])););
          raise_error(RANGE_ERROR);
          break;
      } /* switch */
      preparedStmt->executeSuccessful = FALSE;
      preparedStmt->fetchOkay = FALSE;
    } /* if */
  } /* sqlBindStri */



static void sqlBindTime (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  {
    preparedStmtType preparedStmt;

  /* sqlBindTime */
    /* printf("sqlBindTime(%lx, " FMT_D ", %ld, %ld, %ld, %ld, %ld, %ld, %ld)\n",
       sqlStatement, pos, year, month, day, hour, minute, second, micro_second); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindTime: pos: " FMT_D ", max pos: %lu.\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else if (year <= INT16TYPE_MIN || year > INT16TYPE_MAX || month < 1 || month > 12 ||
               day < 1 || day > 31 || hour < 0 || hour > 24 || minute < 0 || minute > 60 ||
               second < 0 || second > 60 || micro_second < 0 || micro_second > 1000000) {
      logError(printf("sqlBindTime: Time not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      /* printf("paramType: %s\n",
         nameOfBufferType(preparedStmt->paramTypes[pos - 1])); */
      preparedStmt->executeSuccessful = FALSE;
      preparedStmt->fetchOkay = FALSE;
    } /* if */
  } /* sqlBindTime */



static void sqlClose (databaseType database)

  {
    dbType db;

  /* sqlClose */
    db = (dbType) database;
    if (db->connection != NULL) {
      PQfinish(db->connection);
      db->connection = NULL;
    } /* if */
  } /* sqlClose */


static bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    int isNull;
    const_cstriType buffer;
    Oid buffer_type;
    bigIntType columnValue;

  /* sqlColumnBigInt */
    /* printf("sqlColumnBigInt(%lx, " FMT_D ")\n", (unsigned long) sqlStatement, column); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnBigInt: Fetch okay: %d, column: " FMT_D ", max column: %lu.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        columnValue = bigZero();
      } else if (isNull != 0) {
        logError(printf("sqlColumnBigInt: Column " FMT_D ": PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(FILE_ERROR);
        columnValue = NULL;
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (buffer == NULL) {
          logError(printf("sqlColumnBigInt: Column " FMT_D ": PQgetvalue returns NULL.\n",
                          column););
          raise_error(FILE_ERROR);
          columnValue = NULL;
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          /* printf("buffer_type: %s\n", nameOfBufferType(buffer_type)); */
          switch (buffer_type) {
            case INT2OID:
              columnValue = bigFromInt32((int16Type) ntohs(*(uint16Type *) buffer));
              break;
            case INT4OID:
              columnValue = bigFromInt32((int32Type) ntohl(*(uint32Type *) buffer));
              break;
            case INT8OID:
              columnValue = bigFromInt64((int64Type) ntohll(*(uint64Type *) buffer));
              break;
            case NUMERICOID:
              columnValue = getNumericBigInt((const_ustriType) buffer);
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
    /* printf("sqlColumnBigInt --> %s\n", bigHexCStri(columnValue)); */
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
    /* printf("sqlColumnBigRat(%lx, " FMT_D ")\n", (unsigned long) sqlStatement, column); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnBigRat: Fetch okay: %d, column: " FMT_D ", max column: %lu.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        *numerator = bigZero();
        *denominator = bigFromInt32(1);
      } else if (isNull != 0) {
        logError(printf("sqlColumnBigRat: Column " FMT_D ": PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(FILE_ERROR);
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (buffer == NULL) {
          logError(printf("sqlColumnBigRat: Column " FMT_D ": PQgetvalue returns NULL.\n",
                          column););
          raise_error(FILE_ERROR);
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          /* printf("buffer_type: %s\n", nameOfBufferType(buffer_type)); */
          switch (buffer_type) {
            case INT2OID:
              *numerator = bigFromInt32((int16Type) ntohs(*(uint16Type *) buffer));
              *denominator = bigFromInt32(1);
              break;
            case INT4OID:
              *numerator = bigFromInt32((int32Type) ntohl(*(uint32Type *) buffer));
              *denominator = bigFromInt32(1);
              break;
            case INT8OID:
              *numerator = bigFromInt64((int64Type) ntohll(*(uint64Type *) buffer));
              *denominator = bigFromInt32(1);
              break;
            case FLOAT4OID:
              *numerator = roundDoubleToBigRat(ntohf(*(float *) buffer), FALSE, denominator);
              break;
            case FLOAT8OID:
              *numerator = roundDoubleToBigRat(ntohd(*(double *) buffer), FALSE, denominator);
              break;
            case NUMERICOID:
              *numerator = getNumericBigRat((const_ustriType) buffer, denominator);
              break;
            default:
              logError(printf("sqlColumnBigRat: Column " FMT_D " has the unknown type %s.\n",
                              column, nameOfBufferType(buffer_type)););
              raise_error(RANGE_ERROR);
          } /* switch */
        } /* if */
      } /* if */
    } /* if */
    /* printf("sqlColumnBigRat --> %s / %s\n",
       bigHexCStri(*numerator), bigHexCStri(*denominator)); */
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
    /* printf("sqlColumnBool(%lx, " FMT_D ")\n", (unsigned long) sqlStatement, column); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnBool: Fetch okay: %d, column: " FMT_D ", max column: %lu.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        columnValue = 0;
      } else if (isNull != 0) {
        logError(printf("sqlColumnBool: Column " FMT_D ": PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(FILE_ERROR);
        columnValue = 0;
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (buffer == NULL) {
          logError(printf("sqlColumnBool: Column " FMT_D ": PQgetvalue returns NULL.\n",
                          column););
          raise_error(FILE_ERROR);
          columnValue = 0;
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          /* printf("buffer_type: %s\n", nameOfBufferType(buffer_type)); */
          switch (buffer_type) {
            case INT2OID:
              columnValue = (int16Type) ntohs(*(uint16Type *) buffer);
              break;
            case INT4OID:
              columnValue = (int32Type) ntohl(*(uint32Type *) buffer);
              break;
            case INT8OID:
              columnValue = (int64Type) ntohll(*(uint64Type *) buffer);
              break;
            case CHAROID:
            case BPCHAROID:
            case VARCHAROID:
            case TEXTOID:
              length = PQgetlength(preparedStmt->execute_result,
                                   preparedStmt->fetch_index,
                                   (int) column - 1);
              if (length != 1) {
                logError(printf("sqlColumnBool: Column " FMT_D ": Length of column must be 1.\n",
                                column););
                raise_error(FILE_ERROR);
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
          if ((uintType) columnValue >= 2) {
            raise_error(RANGE_ERROR);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    /* printf("sqlColumnBool --> %d\n", columnValue != 0); */
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
    bstriType bstri;

  /* sqlColumnBStri */
    /* printf("sqlColumnBStri(%lx, " FMT_D ")\n", (unsigned long) sqlStatement, column); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnBStri: Fetch okay: %d, column: " FMT_D ", max column: %lu.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      bstri = NULL;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        /* printf("Column is NULL -> Use default value: \"\"\n"); */
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(bstri, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          bstri->size = 0;
        } /* if */
      } else if (isNull != 0) {
        logError(printf("sqlColumnBStri: Column " FMT_D ": PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(FILE_ERROR);
        bstri = NULL;
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (buffer == NULL) {
          logError(printf("sqlColumnBStri: Column " FMT_D ": PQgetvalue returns NULL.\n",
                          column););
          raise_error(FILE_ERROR);
          bstri = NULL;
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          /* printf("buffer_type: %s\n", nameOfBufferType(buffer_type)); */
          switch (buffer_type) {
            case BYTEAOID:
              length = PQgetlength(preparedStmt->execute_result,
                                   preparedStmt->fetch_index,
                                   (int) column - 1);
              if (length < 0) {
                logError(printf("sqlColumnBStri: PQgetlength returns the length %d for column " FMT_D "\n",
                                length, column););
                raise_error(FILE_ERROR);
                bstri = NULL;
              } else if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(bstri, (memSizeType) length))) {
                raise_error(MEMORY_ERROR);
              } else {
                memcpy(bstri->mem, buffer, (memSizeType) length);
                bstri->size = (memSizeType) length;
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
              if (length < 0) {
                logError(printf("sqlColumnBStri: PQgetlength returns the length %d for column " FMT_D "\n",
                                length, column););
                raise_error(FILE_ERROR);
                bstri = NULL;
              } else {
                bstri = cstri8_buf_to_stri(buffer, (memSizeType) length, &err_info);
                if (err_info != OKAY_NO_ERROR) {
                  raise_error(err_info);
                } /* if */
              } /* if */
              break;
            case BPCHAROID:
              length = PQgetlength(preparedStmt->execute_result,
                                   preparedStmt->fetch_index,
                                   (int) column - 1);
              if (length < 0) {
                logError(printf("sqlColumnBStri: PQgetlength returns the length %d for column " FMT_D "\n",
                                length, column););
                raise_error(FILE_ERROR);
                bstri = NULL;
              } else {
                while (length > 0 && buffer[length - 1] == ' ') {
                  length--;
                } /* if */
                bstri = cstri8_buf_to_stri(buffer, (memSizeType) length, &err_info);
                if (err_info != OKAY_NO_ERROR) {
                  raise_error(err_info);
                } /* if */
              } /* if */
              break;
#endif
            default:
              logError(printf("sqlColumnBStri: Column " FMT_D " has the unknown type %s.\n",
                              column, nameOfBufferType(buffer_type)););
              raise_error(RANGE_ERROR);
              bstri = NULL;
              break;
          } /* switch */
        } /* if */
      } /* if */
    } /* if */
    /* printf("sqlColumnBStri --> ");
       prot_bstri(bstri);
       printf("\n"); */
    return bstri;
  } /* sqlColumnBStri */



static void sqlColumnDuration (sqlStmtType sqlStatement, intType column,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second)

  {
    preparedStmtType preparedStmt;
    int isNull;
    const_cstriType buffer;
    Oid buffer_type;

  /* sqlColumnDuration */
    /* printf("sqlColumnDuration(%lx, " FMT_D ")\n", (unsigned long) sqlStatement, column); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnDuration: Fetch okay: %d, column: " FMT_D ", max column: %lu.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        /* printf("Column is NULL -> Use default value: 0-00-00 00:00:00\n"); */
        *year         = 0;
        *month        = 0;
        *day          = 0;
        *hour         = 0;
        *minute       = 0;
        *second       = 0;
        *micro_second = 0;
      } else if (isNull != 0) {
        logError(printf("sqlColumnDuration: Column " FMT_D ": PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(FILE_ERROR);
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (buffer == NULL) {
          logError(printf("sqlColumnDuration: Column " FMT_D ": PQgetvalue returns NULL.\n",
                          column););
          raise_error(FILE_ERROR);
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          /* printf("buffer_type: %s\n", nameOfBufferType(buffer_type)); */
          switch (buffer_type) {
            case INTERVALOID:
              /* The interval is either an int64Type representing       */
              /* microseconds, or a double representing seconds.        */
              /* PQparameterStatus(connecton, "integer_datetimes") is   */
              /* used to determine if an int64Type or a double is used. */
              if (preparedStmt->integerDatetimes) {
                getInterval((int64Type) ntohll(*(uint64Type *) buffer),
                            year, month, day, hour, minute, second, micro_second);
              } else {
                getInterval((int64Type) (1000000.0 * ntohd(*(double *) buffer) + 0.5),
                            year, month, day, hour, minute, second, micro_second);
              } /* if */
#if 0
              printf("INTERVALOID: %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
                     buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7],
                     buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);
              if (sscanf(buffer, "-" FMT_U ":" FMT_U ":" FMT_U,
                         hour, minute, second) == 3) {
                *year         = 0;
                *month        = 0;
                *day          = 0;
                *hour         = -*hour;
                *minute       = -*minute;
                *second       = -*second;
                *micro_second = 0;
              } else if (sscanf(buffer, FMT_D ":" FMT_U ":" FMT_U,
                         hour, minute, second) == 3) {
                *year         = 0;
                *month        = 0;
                *day          = 0;
                *micro_second = 0;
              } else {
                raise_error(RANGE_ERROR);
              } /* if */
#endif
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
   /*  printf("sqlColumnDuration --> %d-%02d-%02d %02d:%02d:%02d\n",
       *year, *month, *day, *hour, *minute, *second); */
  } /* sqlColumnDuration */



static floatType sqlColumnFloat (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    int isNull;
    const_cstriType buffer;
    Oid buffer_type;
    floatType columnValue;

  /* sqlColumnFloat */
    /* printf("sqlColumnFloat(%lx, " FMT_D ")\n", (unsigned long) sqlStatement, column); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnFloat: Fetch okay: %d, column: " FMT_D ", max column: %lu.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = 0.0;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        /* printf("Column is NULL -> Use default value: 0.0\n"); */
        columnValue = 0.0;
      } else if (isNull != 0) {
        logError(printf("sqlColumnFloat: Column " FMT_D ": PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(FILE_ERROR);
        columnValue = 0.0;
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (buffer == NULL) {
          logError(printf("sqlColumnFloat: Column " FMT_D ": PQgetvalue returns NULL.\n",
                          column););
          raise_error(FILE_ERROR);
          columnValue = 0.0;
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          /* printf("buffer_type: %s\n", nameOfBufferType(buffer_type)); */
          switch (buffer_type) {
            case FLOAT4OID:
              columnValue = ntohf(*(float *) buffer);
              break;
            case FLOAT8OID:
              columnValue = ntohd(*(double *) buffer);
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
    /* printf("sqlColumnFloat --> %f\n", columnValue); */
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
    /* printf("sqlColumnInt(%lx, " FMT_D ")\n", (unsigned long) sqlStatement, column); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnInt: Fetch okay: %d, column: " FMT_D ", max column: %lu.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        /* printf("Column is NULL -> Use default value: 0\n"); */
        columnValue = 0;
      } else if (isNull != 0) {
        logError(printf("sqlColumnInt: Column " FMT_D ": PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(FILE_ERROR);
        columnValue = 0;
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (buffer == NULL) {
          logError(printf("sqlColumnInt: Column " FMT_D ": PQgetvalue returns NULL.\n",
                          column););
          raise_error(FILE_ERROR);
          columnValue = 0;
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          /* printf("buffer_type: %s\n", nameOfBufferType(buffer_type)); */
          switch (buffer_type) {
            case INT2OID:
              columnValue = (int16Type) ntohs(*(uint16Type *) buffer);
              break;
            case INT4OID:
              columnValue = (int32Type) ntohl(*(uint32Type *) buffer);
              break;
            case INT8OID:
              columnValue = (int64Type) ntohll(*(uint64Type *) buffer);
              break;
            case OIDOID:
              columnValue = ntohl(*(uint32Type *) buffer);
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
    /* printf("sqlColumnInt --> " FMT_D "\n", columnValue); */
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
    striType stri;

  /* sqlColumnStri */
    /* printf("sqlColumnStri(%lx, " FMT_D ")\n", (unsigned long) sqlStatement, column); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnStri: Fetch okay: %d, column: " FMT_D ", max column: %lu.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      stri = NULL;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
        /* printf("Column is NULL -> Use default value: \"\"\n"); */
        stri = strEmpty();
      } else if (isNull != 0) {
        logError(printf("sqlColumnStri: Column " FMT_D ": PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(FILE_ERROR);
        stri = NULL;
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (buffer == NULL) {
          logError(printf("sqlColumnStri: Column " FMT_D ": PQgetvalue returns NULL.\n",
                          column););
          raise_error(FILE_ERROR);
          stri = NULL;
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          /* printf("buffer_type: %s\n", nameOfBufferType(buffer_type)); */
          switch (buffer_type) {
            case NAMEOID:
            case CHAROID:
            case VARCHAROID:
            case TEXTOID:
              length = PQgetlength(preparedStmt->execute_result,
                                   preparedStmt->fetch_index,
                                   (int) column - 1);
              if (length < 0) {
                logError(printf("sqlColumnStri: PQgetlength returns the length %d for column " FMT_D "\n",
                                length, column););
                raise_error(FILE_ERROR);
                stri = NULL;
              } else {
                stri = cstri8_buf_to_stri(buffer, (memSizeType) length, &err_info);
                if (err_info != OKAY_NO_ERROR) {
                  raise_error(err_info);
                } /* if */
              } /* if */
              break;
            case BPCHAROID:
              length = PQgetlength(preparedStmt->execute_result,
                                   preparedStmt->fetch_index,
                                   (int) column - 1);
              if (length < 0) {
                logError(printf("sqlColumnStri: PQgetlength returns the length %d for column " FMT_D "\n",
                                length, column););
                raise_error(FILE_ERROR);
                stri = NULL;
              } else {
                while (length > 0 && buffer[length - 1] == ' ') {
                  length--;
                } /* if */
                stri = cstri8_buf_to_stri(buffer, (memSizeType) length, &err_info);
                if (err_info != OKAY_NO_ERROR) {
                  raise_error(err_info);
                } /* if */
              } /* if */
              break;
            default:
              logError(printf("sqlColumnStri: Column " FMT_D " has the unknown type %s.\n",
                              column, nameOfBufferType(buffer_type)););
              raise_error(RANGE_ERROR);
              stri = NULL;
              break;
          } /* switch */
        } /* if */
      } /* if */
    } /* if */
    /* printf("sqlColumnStri --> ");
       prot_stri(stri);
       printf("\n"); */
    return stri;
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

  /* sqlColumnTime */
    /* printf("sqlColumnTime(%lx, " FMT_D ")\n", (unsigned long) sqlStatement, column); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlColumnTime: Fetch okay: %d, column: " FMT_D ", max column: %lu.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
      if (isNull == 1) {
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
      } else if (isNull != 0) {
        logError(printf("sqlColumnTime: Column " FMT_D ": PQgetisnull returns %d.\n",
                        column, isNull););
        raise_error(FILE_ERROR);
      } else {
        buffer = PQgetvalue(preparedStmt->execute_result,
                            preparedStmt->fetch_index,
                            (int) column - 1);
        if (buffer == NULL) {
          logError(printf("sqlColumnTime: Column " FMT_D ": PQgetvalue returns NULL.\n",
                          column););
          raise_error(FILE_ERROR);
        } else {
          buffer_type = PQftype(preparedStmt->execute_result, (int) column - 1);
          /* printf("buffer_type: %s\n", nameOfBufferType(buffer_type)); */
          switch (buffer_type) {
            case DATEOID:
              printf("DATEOID: %s\n", buffer);
              break;
            case TIMEOID:
              printf("TIMEOID: %s\n", buffer);
              break;
            case TIMESTAMPOID:
              printf("TIMESTAMPOID: %s\n", buffer);
              break;
            case TIMESTAMPTZOID:
              /* The timestamp is either an int64Type representing      */
              /* microseconds away from 2000-01-01 00:00:00 UTC, or a   */
              /* double representing seconds away from the same origin. */
              /* PQparameterStatus(connecton, "integer_datetimes") is   */
              /* used to determine if an int64Type or a double is used. */
              if (preparedStmt->integerDatetimes) {
                getTimestamp((int64Type) ntohll(*(uint64Type *) buffer),
                             year, month, day, hour, minute, second,
                             micro_second, time_zone, is_dst);
              } else {
                getTimestamp((int64Type) (1000000.0 * ntohd(*(double *) buffer) + 0.5),
                             year, month, day, hour, minute, second,
                             micro_second, time_zone, is_dst);
              } /* if */
#if 0
              printf("TIMESTAMPTZOID: %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
                     buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7],
                     buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);
              if (sscanf(buffer,
                         FMT_D "-" FMT_U "-" FMT_U " "
                         FMT_U ":" FMT_U ":" FMT_U "." FMT_U FMT_D,
                         year, month, day,
                         hour, minute, second, micro_second, time_zone) == 8) {
                *is_dst = 0;
              } else {
                raise_error(RANGE_ERROR);
              } /* if */
#endif
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
    /* printf("sqlColumnTime --> %d-%02d-%02d %02d:%02d:%02d.%06d %d\n",
       *year, *month, *day, *hour, *minute, *second, *micro_second, *time_zone); */
  } /* sqlColumnTime */



static void sqlExecute (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    int num_tuples;

  /* sqlExecute */
    /* printf("begin sqlExecute(%lx)\n", (unsigned long) sqlStatement); */
    preparedStmt = (preparedStmtType) sqlStatement;
    preparedStmt->fetchOkay = FALSE;
    preparedStmt->execute_result = PQexecPrepared(preparedStmt->connection,
                                                  preparedStmt->stmtName,
                                                  (int) preparedStmt->param_array_size,
                                                  (const_cstriType *) preparedStmt->paramValues,
                                                  preparedStmt->paramLengths,
                                                  preparedStmt->paramFormats,
                                                  1);
    if (preparedStmt->execute_result == NULL) {
      logError(printf("sqlExecute: PQexecPrepared returns NULL\n"););
      preparedStmt->executeSuccessful = FALSE;
      raise_error(MEMORY_ERROR);
    } else {
      preparedStmt->execute_status = PQresultStatus(preparedStmt->execute_result);
      if (preparedStmt->execute_status == PGRES_COMMAND_OK) {
        preparedStmt->executeSuccessful = TRUE;
      } else if (preparedStmt->execute_status == PGRES_TUPLES_OK) {
        num_tuples = PQntuples(preparedStmt->execute_result);
        if (num_tuples < 0) {
          logError(printf("sqlExecute: PQntuples returns the number %d\n", num_tuples););
          preparedStmt->executeSuccessful = FALSE;
          raise_error(FILE_ERROR);
        } else {
          preparedStmt->num_tuples = (memSizeType) num_tuples;
          /* printf("Number of tubles: %lu\n", preparedStmt->num_tuples);
            printf("Number of columns: %d\n", PQnfields(preparedStmt->execute_result));
            printf("Result_column_count: %lu\n", preparedStmt->result_column_count);
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
        logError(printf("sqlExecute: PQexecPrepared error:\nstatus=%s\nerror:%s\n",
                        PQresStatus(preparedStmt->execute_status),
                        PQerrorMessage(preparedStmt->connection)););
        preparedStmt->executeSuccessful = FALSE;
        raise_error(FILE_ERROR);
      } /* if */
    } /* if */
    /* printf("end sqlExecute\n"); */
  } /* sqlExecute */



static boolType sqlFetch (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;

  /* sqlFetch */
    /* printf("begin sqlFetch(%lx)\n", (unsigned long) sqlStatement); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (!preparedStmt->executeSuccessful) {
      logError(printf("sqlFetch: Execute was not successful\n"););
      preparedStmt->fetchOkay = FALSE;
      raise_error(FILE_ERROR);
    } else if (preparedStmt->result_column_count == 0 ||
               preparedStmt->execute_status == PGRES_COMMAND_OK) {
      preparedStmt->fetchOkay = FALSE;
    } else if (preparedStmt->execute_status == PGRES_TUPLES_OK) {
      if (!preparedStmt->increment_index) {
        preparedStmt->increment_index = TRUE;
      } else {
        preparedStmt->fetch_index++;
      } /* if */
      preparedStmt->fetchOkay = preparedStmt->fetch_index < preparedStmt->num_tuples;
    } else {
      preparedStmt->fetchOkay = FALSE;
    } /* if */
    /* printf("end sqlFetch --> %d\n", preparedStmt->fetchOkay); */
    return preparedStmt->fetchOkay;
  } /* sqlFetch */



static boolType sqlIsNull (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    boolType isNull;

  /* sqlIsNull */
    /* printf("sqlIsNull(%lx, " FMT_D ")\n", (unsigned long) sqlStatement, column); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlIsNull: Fetch okay: %d, column: " FMT_D ", max column: %lu.\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      isNull = FALSE;
    } else {
      isNull = PQgetisnull(preparedStmt->execute_result,
                           preparedStmt->fetch_index,
                           (int) column - 1);
    } /* if */
    /* printf("sqlIsNull --> %d\n", isNull); */
    return isNull;
  } /* sqlIsNull */



static sqlStmtType sqlPrepare (databaseType database, striType sqlStatementStri)

  {
    dbType db;
    striType statementStri;
    cstriType query;
    PGresult *prepare_result;
    errInfoType err_info = OKAY_NO_ERROR;
    preparedStmtType preparedStmt;

  /* sqlPrepare */
    /* printf("begin sqlPrepare(database, ");
       prot_stri(sqlStatementStri);
       printf(")\n"); */
    db = (dbType) database;
    statementStri = processBindVarsInStatement(sqlStatementStri, &err_info);
    if (statementStri == NULL) {
      preparedStmt = NULL;
    } else {
      query = stri_to_cstri8(statementStri, &err_info);
      if (query == NULL) {
        preparedStmt = NULL;
      } else {
        if (!ALLOC_RECORD(preparedStmt, preparedStmtRecord, count.prepared_stmt)) {
          err_info = MEMORY_ERROR;
        } else {
          memset(preparedStmt, 0, sizeof(preparedStmtRecord));
          preparedStmt->stmtNum = db->nextStmtNum;
          db->nextStmtNum++;
          sprintf(preparedStmt->stmtName, "prepstat_" FMT_U, preparedStmt->stmtNum);
          prepare_result = PQprepare(db->connection, preparedStmt->stmtName, query, 0, NULL);
          if (prepare_result == NULL) {
            FREE_RECORD(preparedStmt, preparedStmtRecord, count.prepared_stmt);
            err_info = MEMORY_ERROR;
            preparedStmt = NULL;
          } else {
            if (PQresultStatus(prepare_result) != PGRES_COMMAND_OK) {
              logError(printf("sqlPrepare: PQprepare returns a status of: %s\n",
                              PQresStatus(PQresultStatus(prepare_result)));
                       printf("%s\n", PQresultErrorMessage(prepare_result)););
              FREE_RECORD(preparedStmt, preparedStmtRecord, count.prepared_stmt);
              err_info = FILE_ERROR;
              preparedStmt = NULL;
            } else {
              preparedStmt->usage_count = 1;
              preparedStmt->sqlFunc = db->sqlFunc;
              preparedStmt->connection = db->connection;
              preparedStmt->integerDatetimes = db->integerDatetimes;
              preparedStmt->executeSuccessful = FALSE;
              preparedStmt->fetchOkay = FALSE;
              setupParameters(preparedStmt, &err_info);
              if (err_info == OKAY_NO_ERROR) {
                setupResult(preparedStmt, &err_info);
              } /* if */
              if (err_info != OKAY_NO_ERROR) {
                freePreparedStmt((sqlStmtType) preparedStmt);
                preparedStmt = NULL;
              } /* if */
            } /* if */
            PQclear(prepare_result);
          } /* if */
        } /* if */
        free_cstri8(query, statementStri);
      } /* if */
      FREE_STRI(statementStri, sqlStatementStri->size * MAX_BIND_VAR_SIZE);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
    } /* if */
    /* printf("end sqlPrepare\n"); */
    return (sqlStmtType) preparedStmt;
  } /* sqlPrepare */



static intType sqlStmtColumnCount (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    intType columnCount;

  /* sqlStmtColumnCount */
    /* printf("sqlStmtColumnCount(%lx)\n", (unsigned long) sqlStatement); */
    preparedStmt = (preparedStmtType) sqlStatement;
    columnCount = (intType) preparedStmt->result_column_count;
    /* printf("sqlStmtColumnCount --> %d\n", columnCount); */
    return columnCount;
  } /* sqlStmtColumnCount */



static striType sqlStmtColumnName (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    cstriType col_name;
    errInfoType err_info = OKAY_NO_ERROR;
    striType name;

  /* sqlStmtColumnName */
    /* printf("sqlStmtColumnName(%lx, " FMT_D ")\n", (unsigned long) sqlStatement, column); */
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(column < 1 ||
                 (uintType) column > preparedStmt->result_column_count)) {
      logError(printf("sqlStmtColumnName: column: " FMT_D ", max column: %lu.\n",
                      column, preparedStmt->result_column_count););
      raise_error(RANGE_ERROR);
      name = NULL;
    } else {
      col_name = PQfname(preparedStmt->execute_result,
                         (int) column - 1);
      name = cstri8_to_stri(col_name, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(err_info);
      } /* if */
    } /* if */
    return name;
  } /* sqlStmtColumnName */



static boolType setupFuncTable (void)

  { /* setupFuncTable */
    if (sqlFunc == NULL) {
      if (ALLOC_RECORD(sqlFunc, sqlFuncRecord, cnt)) {
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
        /*
        sqlFunc->sqlCommit          = &sqlCommit;
        */
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



databaseType sqlOpenPost (const const_striType dbName,
    const const_striType user, const const_striType password)

  {
    cstriType dbName8;
    cstriType user8;
    cstriType password8;
    cstriType host;
    cstriType databaseName;
    dbRecord db;
    const_cstriType setting;
    errInfoType err_info = OKAY_NO_ERROR;
    dbType database;

  /* sqlOpenPost */
    /* printf("sqlOpenPost(");
       prot_stri(dbName);
       printf(", ");
       prot_stri(user);
       printf(", ");
       prot_stri(password);
       printf(")\n"); */
    if (!setupDll(POSTGRESQL_DLL_PATH) && !setupDll(POSTGRESQL_DLL)) {
      logError(printf("sqlOpenPost: setupDll(\"%s\") failed\n", POSTGRESQL_DLL););
      err_info = FILE_ERROR;
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
            databaseName = strchr(dbName8, '/');
            if (databaseName == NULL) {
              host = "localhost";
              databaseName = dbName8;
            } else {
              host = dbName8;
              *databaseName = '\0';
              databaseName++;
            } /* if */
            db.connection = PQsetdbLogin(host, NULL /* pgport */,
                NULL /* pgoptions */, NULL /* pgtty */,
                databaseName, user8, password8);
            if (db.connection == NULL) {
              logError(printf("sqlOpenPost: PQsetdbLogin(\"%s\", ...  "
                              "\"%s\", \"%s\", \"%s\") returns NULL\n",
                              host, databaseName, user8, password8););
              err_info = MEMORY_ERROR;
              database = NULL;
            } else if (PQstatus(db.connection) != CONNECTION_OK) {
              logError(printf("sqlOpenPost: PQsetdbLogin(\"%s\", ...  "
                              "\"%s\", \"%s\", \"%s\") error:\n"
                              "status=%d\nerror: %s\n",
                              host, databaseName, user8, password8,
                              PQstatus(db.connection),
                              PQerrorMessage(db.connection)););
              err_info = FILE_ERROR;
              PQfinish(db.connection);
              database = NULL;
            } else if (PQsetClientEncoding(db.connection, "UNICODE") != 0) {
              logError(printf("sqlOpenPost: PQsetClientEncoding des not return 0.\n"););
              err_info = FILE_ERROR;
              PQfinish(db.connection);
              database = NULL;
            } else if (unlikely(!setupFuncTable() ||
                                !ALLOC_RECORD(database, dbRecord, count.database))) {
              err_info = MEMORY_ERROR;
              PQfinish(db.connection);
              database = NULL;
            } else {
              memset(database, 0, sizeof(dbRecord));
              database->usage_count = 1;
              database->sqlFunc = sqlFunc;
              database->connection = db.connection;
              setting = PQparameterStatus(db.connection, "integer_datetimes");
              database->integerDatetimes = setting != NULL && strcmp(setting, "on") == 0;
              database->nextStmtNum = 1;
            } /* if */
            free_cstri8(password8, password);
          } /* if */
          free_cstri8(user8, user);
        } /* if */
        free_cstri8(dbName8, dbName);
      } /* if */
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
    } /* if */
    return (databaseType) database;
  } /* sqlOpenPost */

#endif
