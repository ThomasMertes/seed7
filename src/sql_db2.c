/********************************************************************/
/*                                                                  */
/*  sql_db2.c     Database access functions for Db2.                */
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
/*  File: seed7/src/sql_db2.c                                       */
/*  Changes: 2019 - 2020  Thomas Mertes                             */
/*  Content: Database access functions for Db2.                     */
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
#ifdef DB2_INCLUDE
#include DB2_INCLUDE
#endif

#include "common.h"
#include "striutl.h"
#include "heaputl.h"
#include "numutl.h"
#include "str_rtl.h"
#include "tim_rtl.h"
#include "big_drv.h"
#include "rtl_err.h"
#include "dll_drv.h"
#include "sql_base.h"
#include "sql_drv.h"

#ifdef DB2_INCLUDE

#ifdef DB2_DLL
#define CLI_DLL DB2_DLL
#endif

#define ALLOW_EXECUTE_SUCCESS_WITH_INFO
#define ALLOW_FETCH_SUCCESS_WITH_INFO

#include "sql_cli.c"

typedef struct {
    wstriType hostname;
    memSizeType hostnameLength;
    intType port;
    wstriType database;
    memSizeType databaseLength;
    wstriType uid;
    memSizeType uidLength;
    wstriType pwd;
    memSizeType pwdLength;
    wstriType connectionString;
    memSizeType connectionStringLength;
  } connectDataRecord, *connectDataType;

#define DEFAULT_PORT 50000



static boolType createConnectionString (connectDataType connectData)

  {
    const wcharType databaseKey[] = {'D', 'A', 'T', 'A', 'B', 'A', 'S', 'E', '=', '\0'};
    const wcharType hostnameKey[] = {'H', 'O', 'S', 'T', 'N', 'A', 'M', 'E', '=', '\0'};
    const wcharType localhost[] = {'l', 'o', 'c', 'a', 'l', 'h', 'o', 's', 't', '\0'};
    const wcharType portKey[] = {'P', 'O', 'R', 'T', '=', '\0'};
    const wcharType protocolKeyAndValue[] = {'P', 'R', 'O', 'T', 'O', 'C', 'O', 'L', '=', 'T', 'C', 'P', 'I', 'P', '\0'};
    const wcharType uidKey[] = {'U', 'I', 'D', '=', '\0'};
    const wcharType pwdKey[] = {'P', 'W', 'D', '=', '\0'};
    const_wstriType hostname;
    memSizeType hostnameLength;
    intType port;
    char portName[INTTYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    memSizeType portNameLength;
    memSizeType connectionStringLength;
    wstriType connectionString;
    memSizeType pos = 0;
    boolType okay = FALSE;

  /* createConnectionString */
    logFunction(printf("createConnectionString([hostname=\"");
                printWstri(connectData->hostname);
                printf("\", port=" FMT_D ", database=\"", connectData->port);
                printWstri(connectData->database);
                printf("\"])\n"););

    if (connectData->hostnameLength == 0) {
      hostname = localhost;
      hostnameLength = STRLEN(localhost);
    } else {
      hostname = connectData->hostname;
      hostnameLength = connectData->hostnameLength;
    } /* if */
    if (connectData->port == 0) {
      port = DEFAULT_PORT;
    } else {
      port = connectData->port;
    } /* if */
    portNameLength = (memSizeType) sprintf(portName, FMT_D, port);
    connectData->connectionString = NULL;
    connectData->connectionStringLength = 0;

    if (likely(connectData->databaseLength <= SHRT_MAX &&
               connectData->hostnameLength <= SHRT_MAX &&
               connectData->uidLength <= SHRT_MAX &&
               connectData->pwdLength <= SHRT_MAX)) {
      connectionStringLength = STRLEN(databaseKey) + connectData->databaseLength +
                               1 + STRLEN(hostnameKey) + hostnameLength +
                               1 + STRLEN(portKey) + portNameLength +
                               1 + STRLEN(protocolKeyAndValue);
      if (connectData->uidLength != 0) {
        connectionStringLength += 1 + STRLEN(uidKey) + connectData->uidLength;
      } /* if */
      if (connectData->pwdLength != 0) {
        connectionStringLength += 1 + STRLEN(pwdKey) + connectData->pwdLength;
      } /* if */

      if (likely(connectionStringLength <= SHRT_MAX &&
                 ALLOC_WSTRI(connectionString, connectionStringLength))) {
        connectData->connectionString = connectionString;
        connectData->connectionStringLength = connectionStringLength;

        memcpy(&connectionString[pos], databaseKey, STRLEN(databaseKey) * sizeof(SQLWCHAR));
        pos += STRLEN(databaseKey);
        memcpy(&connectionString[pos], connectData->database, connectData->databaseLength * sizeof(SQLWCHAR));
        pos += connectData->databaseLength;

        connectionString[pos++] = ';';
        memcpy(&connectionString[pos], hostnameKey, STRLEN(hostnameKey) * sizeof(SQLWCHAR));
        pos += STRLEN(hostnameKey);
        memcpy(&connectionString[pos], hostname, hostnameLength * sizeof(SQLWCHAR));
        pos += hostnameLength;

        connectionString[pos++] = ';';
        memcpy(&connectionString[pos], portKey, STRLEN(portKey) * sizeof(SQLWCHAR));
        pos += STRLEN(portKey);
        memcpy_to_wstri(&connectionString[pos], portName, portNameLength);
        pos += portNameLength;

        connectionString[pos++] = ';';
        memcpy(&connectionString[pos], protocolKeyAndValue, STRLEN(protocolKeyAndValue) * sizeof(SQLWCHAR));
        pos += STRLEN(protocolKeyAndValue);

        if (connectData->uidLength != 0) {
          connectionString[pos++] = ';';
          memcpy(&connectionString[pos], uidKey, STRLEN(uidKey) * sizeof(SQLWCHAR));
          pos += STRLEN(uidKey);
          memcpy(&connectionString[pos], connectData->uid, connectData->uidLength * sizeof(SQLWCHAR));
          pos += connectData->uidLength;
        } /* if */

        if (connectData->pwdLength != 0) {
          connectionString[pos++] = ';';
          memcpy(&connectionString[pos], pwdKey, STRLEN(pwdKey) * sizeof(SQLWCHAR));
          pos += STRLEN(pwdKey);
          memcpy(&connectionString[pos], connectData->pwd, connectData->pwdLength * sizeof(SQLWCHAR));
          pos += connectData->pwdLength;
        } /* if */

        connectionString[pos] = '\0';
        okay = TRUE;
      } /* if */
    } /* if */
    logFunction(printf("createConnectionString --> %d (connectionString=\"", okay);
                printWstri(connectData->connectionString);
                printf("\")\n"););
    return okay;
  } /* createConnectionString */



static databaseType doOpenDb2 (connectDataType connectData, errInfoType *err_info)

  {
    SQLHENV sql_environment;
    SQLHDBC sql_connection;
    SQLRETURN returnCode;
    SQLSMALLINT outConnectionStringLength;
    databaseType database;

  /* doOpenDb2 */
    logFunction(printf("doOpenDb2(*, %d)\n", *err_info););
    if (unlikely(connectData->databaseLength > SHRT_MAX ||
                 connectData->uidLength > SHRT_MAX ||
                 connectData->pwdLength > SHRT_MAX ||
                 connectData->connectionStringLength > SHRT_MAX)) {
      *err_info = MEMORY_ERROR;
      database = NULL;
    } else {
      *err_info = prepareSqlConnection(&sql_environment, &sql_connection);
      if (unlikely(*err_info != OKAY_NO_ERROR)) {
        database = NULL;
      } else {
        if (connectData->hostnameLength == 0 && connectData->port == 0) {
          returnCode = SQLConnectW(sql_connection,
              (SQLWCHAR *) connectData->database, (SQLSMALLINT) connectData->databaseLength,
              (SQLWCHAR *) connectData->uid, (SQLSMALLINT) connectData->uidLength,
              (SQLWCHAR *) connectData->pwd, (SQLSMALLINT) connectData->pwdLength);
          if ((returnCode != SQL_SUCCESS &&
               returnCode != SQL_SUCCESS_WITH_INFO)) {
            setDbErrorMsg("sqlOpenDb2", "SQLConnectW",
                          SQL_HANDLE_DBC, sql_connection);
            logError(printf("sqlOpenDb2: SQLConnectW:\n%s\n",
                            dbError.message););
          } /* if */
        } else {
          returnCode = SQL_ERROR;
        } /* if */
        if (returnCode != SQL_SUCCESS &&
            returnCode != SQL_SUCCESS_WITH_INFO) {
          returnCode = SQLDriverConnectW(sql_connection,
                                         NULL, /* GetDesktopWindow(), */
                                         (SQLWCHAR *) connectData->connectionString,
                                         (SQLSMALLINT) connectData->connectionStringLength,
                                         NULL, /* outConnectionString */
                                         0,
                                         &outConnectionStringLength,
                                         SQL_DRIVER_NOPROMPT);
        } /* if */
        if (returnCode != SQL_SUCCESS &&
            returnCode != SQL_SUCCESS_WITH_INFO) {
          if (connectData->hostnameLength != 0 || connectData->port != 0) {
            setDbErrorMsg("sqlOpenDb2", "SQLDriverConnect",
                          SQL_HANDLE_DBC, sql_connection);
            logError(printf("sqlOpenDb2: SQLDriverConnect:\n%s\n",
                            dbError.message););
          } /* if */
          *err_info = DATABASE_ERROR;
          SQLFreeHandle(SQL_HANDLE_DBC, sql_connection);
          SQLFreeHandle(SQL_HANDLE_ENV, sql_environment);
          database = NULL;
        } else {
          database = createDbRecord(sql_environment, sql_connection, DB_CATEGORY_DB2, err_info);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("doOpenDb2 --> " FMT_U_MEM " (err_info=%d)\n",
                       (memSizeType) database, *err_info););
    return database;
  } /* doOpenDb2 */



databaseType sqlOpenDb2 (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  {
    connectDataRecord connectData;
    errInfoType err_info = OKAY_NO_ERROR;
    databaseType database;

  /* sqlOpenDb2 */
    logFunction(printf("sqlOpenDb2(\"%s\", ",
                       striAsUnquotedCStri(host));
                printf(FMT_D ", \"%s\", ",
                       port, striAsUnquotedCStri(dbName));
                printf("\"%s\", ", striAsUnquotedCStri(user));
                printf("\"%s\")\n", striAsUnquotedCStri(password)););
    if (!findDll()) {
      logError(printf("sqlOpenDb2: findDll() failed\n"););
      err_info = DATABASE_ERROR;
      database = NULL;
    } else {
      connectData.port = port;
      connectData.hostname = stri_to_wstri_buf(host, &connectData.hostnameLength, &err_info);
      if (unlikely(connectData.hostname == NULL)) {
        database = NULL;
      } else {
        connectData.database = stri_to_wstri_buf(dbName, &connectData.databaseLength, &err_info);
        if (unlikely(connectData.database == NULL)) {
          database = NULL;
        } else {
          connectData.uid = stri_to_wstri_buf(user, &connectData.uidLength, &err_info);
          if (unlikely(connectData.uid == NULL)) {
            database = NULL;
          } else {
            connectData.pwd = stri_to_wstri_buf(password, &connectData.pwdLength, &err_info);
            if (unlikely(connectData.pwd == NULL)) {
              database = NULL;
            } else {
              if (unlikely(!createConnectionString(&connectData))) {
                err_info = MEMORY_ERROR;
                database = NULL;
              } else {
                database = doOpenDb2(&connectData, &err_info);
                UNALLOC_WSTRI(connectData.connectionString, connectData.connectionStringLength);
              } /* if */
              UNALLOC_WSTRI(connectData.pwd, connectData.pwdLength);
            } /* if */
            UNALLOC_WSTRI(connectData.uid, connectData.uidLength);
          } /* if */
          UNALLOC_WSTRI(connectData.database, connectData.databaseLength);
        } /* if */
        UNALLOC_WSTRI(connectData.hostname, connectData.hostnameLength);
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("sqlOpenDb2 --> " FMT_U_MEM "\n",
                       (memSizeType) database););
    return database;
  } /* sqlOpenDb2 */

#else



databaseType sqlOpenDb2 (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  { /* sqlOpenDb2 */
    logError(printf("sqlOpenDb2(\"%s\", ",
                    striAsUnquotedCStri(host));
             printf(FMT_D ", \"%s\", ",
                    port, striAsUnquotedCStri(dbName));
             printf("\"%s\", ", striAsUnquotedCStri(user));
             printf("\"%s\"): Db2 driver not present.\n",
                    striAsUnquotedCStri(password)););
    raise_error(RANGE_ERROR);
    return NULL;
  } /* sqlOpenDb2 */

#endif
