/********************************************************************/
/*                                                                  */
/*  sql_drv.h     Prototypes for database access functions.         */
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
/*  File: seed7/src/sql_drv.h                                       */
/*  Changes: 2013, 2014  Thomas Mertes                              */
/*  Content: Prototypes for database access functions.              */
/*                                                                  */
/********************************************************************/

typedef struct {
    void (*freeDatabase) (databaseType database);
    void (*freePreparedStmt) (sqlStmtType sqlStatement);
    void (*sqlBindBigInt) (sqlStmtType sqlStatement, intType pos,
                           const const_bigIntType value);
    void (*sqlBindBigRat) (sqlStmtType sqlStatement, intType pos,
                           const const_bigIntType numerator,
                           const const_bigIntType denominator);
    void (*sqlBindBool) (sqlStmtType sqlStatement, intType pos, boolType value);
    void (*sqlBindBStri) (sqlStmtType sqlStatement, intType pos, bstriType bstri);
    void (*sqlBindDuration) (sqlStmtType sqlStatement, intType pos,
                             intType year, intType month, intType day, intType hour,
                             intType minute, intType second, intType micro_second);
    void (*sqlBindFloat) (sqlStmtType sqlStatement, intType pos, floatType value);
    void (*sqlBindInt) (sqlStmtType sqlStatement, intType pos, intType value);
    void (*sqlBindNull) (sqlStmtType sqlStatement, intType pos);
    void (*sqlBindStri) (sqlStmtType sqlStatement, intType pos, striType stri);
    void (*sqlBindTime) (sqlStmtType sqlStatement, intType pos,
                         intType year, intType month, intType day, intType hour,
                         intType min, intType sec, intType micro_sec,
                         intType time_zone);
    void (*sqlClose) (databaseType database);
    bigIntType (*sqlColumnBigInt) (sqlStmtType sqlStatement, intType column);
    void (*sqlColumnBigRat) (sqlStmtType sqlStatement, intType column,
                          bigIntType *numerator, bigIntType *denominator);
    boolType (*sqlColumnBool) (sqlStmtType sqlStatement, intType column);
    bstriType (*sqlColumnBStri) (sqlStmtType sqlStatement, intType column);
    void (*sqlColumnDuration) (sqlStmtType sqlStatement, intType column,
                               intType *year, intType *month, intType *day, intType *hour,
                               intType *minute, intType *second, intType *micro_second);
    floatType (*sqlColumnFloat) (sqlStmtType sqlStatement, intType column);
    intType (*sqlColumnInt) (sqlStmtType sqlStatement, intType column);
    striType (*sqlColumnStri) (sqlStmtType sqlStatement, intType column);
    void (*sqlColumnTime) (sqlStmtType sqlStatement, intType column,
                           intType *year, intType *month, intType *day, intType *hour,
                           intType *minute, intType *second, intType *micro_second,
                           intType *time_zone, boolType *is_dst);
    void (*sqlCommit) (databaseType database);
    void (*sqlExecute) (sqlStmtType sqlStatement);
    boolType (*sqlFetch) (sqlStmtType sqlStatement);
    boolType (*sqlGetAutoCommit) (databaseType database);
    boolType (*sqlIsNull) (sqlStmtType sqlStatement, intType column);
    sqlStmtType (*sqlPrepare) (databaseType database, striType sqlStatementStri);
    void (*sqlRollback) (databaseType database);
    void (*sqlSetAutoCommit) (databaseType database, boolType autoCommit);
    intType (*sqlStmtColumnCount) (sqlStmtType sqlStatement);
    striType (*sqlStmtColumnName) (sqlStmtType sqlStatement, intType column);
  } sqlFuncRecord, *sqlFuncType;

#define DB_CATEGORY_MYSQL      1
#define DB_CATEGORY_SQLITE     2
#define DB_CATEGORY_POSTGRESQL 3
#define DB_CATEGORY_OCI        4
#define DB_CATEGORY_ODBC       5
#define DB_CATEGORY_FIREBIRD   6
#define DB_CATEGORY_DB2        7
#define DB_CATEGORY_SQL_SERVER 8
#define DB_CATEGORY_TDS        9


databaseType sqlOpenDb2 (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password);
databaseType sqlOpenFire (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password);
databaseType sqlOpenLite (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password);
databaseType sqlOpenMy (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password);
databaseType sqlOpenOci (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password);
databaseType sqlOpenOdbc (const const_striType driver,
    const const_striType server, const const_striType dbName,
    const const_striType user, const const_striType password);
databaseType sqlOpenPost (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password);
databaseType sqlOpenSqlServer (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password);
databaseType sqlOpenTds (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password);
