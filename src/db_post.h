/********************************************************************/
/*                                                                  */
/*  db_post.h     PostgreSQL interfaces used by Seed7.              */
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
/*  File: seed7/src/db_post.h                                       */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: PostgreSQL interfaces used by Seed7.                   */
/*                                                                  */
/********************************************************************/

typedef struct pg_conn PGconn;
typedef struct pg_result PGresult;
typedef unsigned int Oid;

typedef enum {
  PGRES_EMPTY_QUERY = 0,
  PGRES_COMMAND_OK,
  PGRES_TUPLES_OK,
  PGRES_COPY_OUT,
  PGRES_COPY_IN,
  PGRES_BAD_RESPONSE,
  PGRES_NONFATAL_ERROR,
  PGRES_FATAL_ERROR,
  PGRES_COPY_BOTH
} ExecStatusType;

typedef enum {
  CONNECTION_OK,
  CONNECTION_BAD,
  CONNECTION_STARTED,
  CONNECTION_MADE,
  CONNECTION_AWAITING_RESPONSE,
  CONNECTION_AUTH_OK,
  CONNECTION_SETENV,
  CONNECTION_SSL_STARTUP,
  CONNECTION_NEEDED
} ConnStatusType;

#define BOOLOID              16
#define BYTEAOID             17
#define CHAROID              18
#define NAMEOID              19
#define INT8OID              20
#define INT2OID              21
#define INT2VECTOROID        22
#define INT4OID              23
#define REGPROCOID           24
#define TEXTOID              25
#define OIDOID               26
#define TIDOID               27
#define XIDOID               28
#define CIDOID               29
#define OIDVECTOROID         30
#define XMLOID              142
#define PGNODETREEOID       194
#define POINTOID            600
#define LSEGOID             601
#define PATHOID             602
#define BOXOID              603
#define POLYGONOID          604
#define LINEOID             628
#define FLOAT4OID           700
#define FLOAT8OID           701
#define ABSTIMEOID          702
#define RELTIMEOID          703
#define TINTERVALOID        704
#define UNKNOWNOID          705
#define CIRCLEOID           718
#define CASHOID             790
#define MACADDROID          829
#define INETOID             869
#define CIDROID             650
#define INT2ARRAYOID       1005
#define INT4ARRAYOID       1007
#define TEXTARRAYOID       1009
#define OIDARRAYOID        1028
#define FLOAT4ARRAYOID     1021
#define ACLITEMOID         1033
#define CSTRINGARRAYOID    1263
#define BPCHAROID          1042
#define VARCHAROID         1043
#define DATEOID            1082
#define TIMEOID            1083
#define TIMESTAMPOID       1114
#define TIMESTAMPTZOID     1184
#define INTERVALOID        1186
#define TIMETZOID          1266
#define BITOID             1560
#define VARBITOID          1562
#define NUMERICOID         1700
#define REFCURSOROID       1790
#define REGPROCEDUREOID    2202
#define REGOPEROID         2203
#define REGOPERATOROID     2204
#define REGCLASSOID        2205
#define REGTYPEOID         2206
#define REGTYPEARRAYOID    2211
#define TSVECTOROID        3614
#define GTSVECTOROID       3642
#define TSQUERYOID         3615
#define REGCONFIGOID       3734
#define REGDICTIONARYOID   3769
#define RECORDOID          2249
#define RECORDARRAYOID     2287
#define CSTRINGOID         2275
#define ANYOID              2276
#define ANYARRAYOID         2277
#define VOIDOID             2278
#define TRIGGEROID          2279
#define LANGUAGE_HANDLEROID 2280
#define INTERNALOID         2281
#define OPAQUEOID           2282
#define ANYELEMENTOID       2283
#define ANYNONARRAYOID      2776
#define ANYENUMOID          3500
#define FDW_HANDLEROID      3115

#ifndef CDECL
#if defined(_WIN32) && HAS_CDECL
#define CDECL __cdecl
#else
#define CDECL
#endif
#endif

extern void CDECL PQclear (PGresult *res);
extern PGresult *CDECL PQdescribePrepared (PGconn *conn, const char *stmt);
extern char *CDECL PQerrorMessage (const PGconn *conn);
extern PGresult *CDECL PQexec (PGconn *conn, const char *query);
extern PGresult *CDECL PQexecPrepared (PGconn *conn,
                                       const char *stmtName,
                                       int nParams,
                                       const char *const * paramValues,
                                       const int *paramLengths,
                                       const int *paramFormats,
                                       int resultFormat);
extern void CDECL PQfinish (PGconn *conn);
extern char *CDECL PQfname (const PGresult *res, int field_num);
extern Oid CDECL PQftype (const PGresult *res, int field_num);
extern int CDECL PQgetisnull (const PGresult *res, int tup_num, int field_num);
extern int CDECL PQgetlength (const PGresult *res, int tup_num, int field_num);
extern char *CDECL PQgetvalue (const PGresult *res, int tup_num, int field_num);
extern int CDECL PQnfields (const PGresult *res);
extern int CDECL PQnparams (const PGresult *res);
extern int CDECL PQntuples (const PGresult *res);
extern const char *CDECL PQparameterStatus (const PGconn *conn, const char *paramName);
extern Oid CDECL PQparamtype (const PGresult *res, int param_num);
extern PGresult *CDECL PQprepare (PGconn *conn, const char *stmtName,
                                  const char *query, int nParams,
                                  const Oid *paramTypes);
extern char *CDECL PQresStatus (ExecStatusType status);
extern char *CDECL PQresultErrorMessage (const PGresult *res);
extern ExecStatusType CDECL PQresultStatus (const PGresult *res);
extern int CDECL PQsetClientEncoding (PGconn *conn, const char *encoding);
extern PGconn *CDECL PQsetdbLogin (const char *pghost, const char *pgport,
                                   const char *pgoptions, const char *pgtty,
                                   const char *dbName,
                                   const char *login, const char *pwd);
extern ConnStatusType CDECL PQstatus (const PGconn *conn);
