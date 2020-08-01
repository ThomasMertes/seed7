/********************************************************************/
/*                                                                  */
/*  sql_rtl.h     Database access functions.                        */
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
/*  File: seed7/src/sql_rtl.h                                       */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: Database access functions.                             */
/*                                                                  */
/********************************************************************/

void sqlBindBigInt (sqlStmtType sqlStatement, intType pos,
                    const const_bigIntType value);
void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
                    const const_bigIntType numerator,
                    const const_bigIntType denominator);
void sqlBindBool (sqlStmtType sqlStatement, intType pos, boolType value);
void sqlBindBStri (sqlStmtType sqlStatement, intType pos, bstriType bstri);
void sqlBindDuration (sqlStmtType sqlStatement, intType pos,
                      intType year, intType month, intType day, intType hour,
                      intType minute, intType second, intType micro_second);
void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value);
void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value);
void sqlBindNull (sqlStmtType sqlStatement, intType pos);
void sqlBindStri (sqlStmtType sqlStatement, intType pos, striType stri);
void sqlBindTime (sqlStmtType sqlStatement, intType pos,
                  intType year, intType month, intType day, intType hour,
                  intType min, intType sec, intType micro_sec);
void sqlClose (databaseType database);
bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column);
void sqlColumnBigRat (sqlStmtType sqlStatement, intType column,
                      bigIntType *numerator, bigIntType *denominator);
boolType sqlColumnBool (sqlStmtType sqlStatement, intType column);
bstriType sqlColumnBStri (sqlStmtType sqlStatement, intType column);
void sqlColumnDuration (sqlStmtType sqlStatement, intType column,
                        intType *year, intType *month, intType *day, intType *hour,
                        intType *minute, intType *second, intType *micro_second);
floatType sqlColumnFloat (sqlStmtType sqlStatement, intType column);
intType sqlColumnInt (sqlStmtType sqlStatement, intType column);
striType sqlColumnStri (sqlStmtType sqlStatement, intType column);
void sqlColumnTime (sqlStmtType sqlStatement, intType column,
                    intType *year, intType *month, intType *day, intType *hour,
                    intType *minute, intType *second, intType *micro_second,
                    intType *time_zone, boolType *is_dst);
void sqlCommit (databaseType database);
void sqlCpyDb (databaseType *const db_to, const databaseType db_from);
void sqlCpyStmt (sqlStmtType *const stmt_to, const sqlStmtType stmt_from);
databaseType sqlCreateDb (const databaseType db_from);
sqlStmtType sqlCreateStmt (const sqlStmtType stmt_from);
void sqlDestrDb (const databaseType old_db);
void sqlDestrStmt (const sqlStmtType old_stmt);
intType sqlErrCode (void);
striType sqlErrDbFunc (void);
striType sqlErrLibFunc (void);
striType sqlErrMessage (void);
void sqlExecute (sqlStmtType sqlStatement);
boolType sqlFetch (sqlStmtType sqlStatement);
boolType sqlIsNull (sqlStmtType sqlStatement, intType column);
databaseType sqlOpen (intType driver, const const_striType dbName,
                      const const_striType user,
                      const const_striType password);
sqlStmtType sqlPrepare (databaseType database, striType sqlStatementStri);
intType sqlStmtColumnCount (sqlStmtType sqlStatement);
striType sqlStmtColumnName (sqlStmtType sqlStatement, intType column);
