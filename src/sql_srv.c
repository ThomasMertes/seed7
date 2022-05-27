/********************************************************************/
/*                                                                  */
/*  sql_srv.c    Database access functions for MS SQL-Server.       */
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
/*  File: seed7/src/sql_srv.c                                       */
/*  Changes: 2019, 2020  Thomas Mertes                              */
/*  Content: Database access functions for MS SQL-Server.           */
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
#ifdef SQL_SERVER_INCLUDE
#if SQL_SERVER_INCLUDE_WINDOWS_H
#include "windows.h"
#endif
#include SQL_SERVER_INCLUDE
#if SQL_SERVER_INCLUDE_SQLEXT_H
#include "sqlext.h"
#endif
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

#ifdef SQL_SERVER_INCLUDE

#ifdef SQL_SERVER_DLL
#define CLI_DLL SQL_SERVER_DLL
#endif

#define loadBaseDlls()
#if FREETDS_SQL_SERVER_CONNECTION
#define WIDE_CHARS_SUPPORTED 1
#define TINY_INT_IS_UNSIGNED 1
#endif
#define SIZEOF_SQLWCHAR SQL_SERVER_SIZEOF_SQLWCHAR

#include "sql_cli.c"

typedef struct {
    SQLWCHAR *server;
    memSizeType serverLength;
    intType port;
    SQLWCHAR *database;
    memSizeType databaseLength;
    SQLWCHAR *uid;
    memSizeType uidLength;
    SQLWCHAR *pwd;
    memSizeType pwdLength;
    SQLWCHAR *connectionString;
    memSizeType connectionStringLength;
  } connectDataRecordSrv, *connectDataType;

#define DEFAULT_PORT 1433



static boolType createConnectionString (connectDataType connectData)

  {
    const SQLWCHAR serverKey[] = {'S', 'E', 'R', 'V', 'E', 'R', '=', '\0'};
    const SQLWCHAR databaseKey[] = {'D', 'A', 'T', 'A', 'B', 'A', 'S', 'E', '=', '\0'};
    const SQLWCHAR localhost[] = {'l', 'o', 'c', 'a', 'l', 'h', 'o', 's', 't', '\0'};
    const SQLWCHAR uidKey[] = {'U', 'I', 'D', '=', '\0'};
    const SQLWCHAR pwdKey[] = {'P', 'W', 'D', '=', '\0'};
    const SQLWCHAR *server;
    memSizeType serverLength;
    char portName[1 + INTTYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    memSizeType portNameLength;
    memSizeType connectionStringLength;
    SQLWCHAR *connectionString;
    memSizeType pos = 0;
    boolType okay = FALSE;

  /* createConnectionString */
    logFunction(printf("createConnectionString([server=\"");
                printWstri(connectData->server);
                printf("\", port=" FMT_D ", database=\"", connectData->port);
                printWstri(connectData->database);
                printf("\"])\n"););

    if (connectData->serverLength == 0) {
      server = localhost;
      serverLength = STRLEN(localhost);
    } else {
      server = connectData->server;
      serverLength = connectData->serverLength;
    } /* if */
    if (connectData->port == 0) {
#if SPECIFY_SQL_SERVER_PORT_EXPLICIT
      portNameLength = (memSizeType) sprintf(portName, ",%d", DEFAULT_PORT);
#else
      portNameLength = 0;
#endif
    } else {
      portNameLength = (memSizeType) sprintf(portName, "," FMT_D, connectData->port);
    } /* if */
    connectData->connectionString = NULL;
    connectData->connectionStringLength = 0;

    if (likely(connectData->serverLength <= SHRT_MAX &&
               connectData->databaseLength <= SHRT_MAX &&
               connectData->uidLength <= SHRT_MAX &&
               connectData->pwdLength <= SHRT_MAX)) {
      connectionStringLength = STRLEN(serverKey) + serverLength + portNameLength +
                               1 + STRLEN(databaseKey) + connectData->databaseLength;
      if (connectData->uidLength != 0) {
        connectionStringLength += 1 + STRLEN(uidKey) + connectData->uidLength;
      } /* if */
      if (connectData->pwdLength != 0) {
        connectionStringLength += 1 + STRLEN(pwdKey) + connectData->pwdLength;
      } /* if */

      if (likely(connectionStringLength <= SHRT_MAX &&
                 ALLOC_SQLWSTRI(connectionString, connectionStringLength))) {
        connectData->connectionString = connectionString;
        connectData->connectionStringLength = connectionStringLength;

        memcpy(&connectionString[pos], serverKey, STRLEN(serverKey) * sizeof(SQLWCHAR));
        pos += STRLEN(serverKey);
        memcpy(&connectionString[pos], server, serverLength * sizeof(SQLWCHAR));
        pos += serverLength;
        memcpy_to_sqlwstri(&connectionString[pos], portName, portNameLength);
        pos += portNameLength;

        connectionString[pos++] = ';';
        memcpy(&connectionString[pos], databaseKey, STRLEN(databaseKey) * sizeof(SQLWCHAR));
        pos += STRLEN(databaseKey);
        memcpy(&connectionString[pos], connectData->database, connectData->databaseLength * sizeof(SQLWCHAR));
        pos += connectData->databaseLength;

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
                if (connectData->connectionString != NULL) {
                  printWstri(connectData->connectionString);
                }
                printf("\")\n"););
    return okay;
  } /* createConnectionString */



static SQLWCHAR *wstriSearchCh (const SQLWCHAR *str, const SQLWCHAR ch)

  { /* wstriSearchCh */
    for (; *str != ch; str++) {
      if (*str == (SQLWCHAR) 0) {
        return NULL;
      } /* if */
    } /* for */
    return (SQLWCHAR *) str;
  } /* wstriSearchCh */



static SQLWCHAR *wstriSearch (const SQLWCHAR *haystack, const SQLWCHAR *needle)

  {
    const SQLWCHAR *sc1;
    const SQLWCHAR *sc2;

  /* wstriSearch */
    if (*needle == (SQLWCHAR) 0) {
      return (SQLWCHAR *) haystack;
    } else {
      for (; (haystack = wstriSearchCh(haystack, *needle)) != NULL; haystack++) {
        for (sc1 = haystack, sc2 = needle; ; ) {
          if (*++sc2 == (SQLWCHAR) 0) {
            return (SQLWCHAR *) haystack;
          } else if (*++sc1 != *sc2) {
            break;
          } /* if */
        } /* for */
      } /* for */
    } /* if */
    return NULL;
  } /* wstriSearch */



static boolType connectToServer (connectDataType connectData,
    SQLHDBC sql_connection, SQLWCHAR *server, memSizeType serverLength)

  {
    const SQLWCHAR serverKey[] = {'S', 'E', 'R', 'V', 'E', 'R', '=', '\0'};
    const SQLWCHAR databaseKey[] = {'D', 'A', 'T', 'A', 'B', 'A', 'S', 'E', '=', '\0'};
    const SQLWCHAR uidKey[] = {'U', 'I', 'D', '=', '\0'};
    const SQLWCHAR pwdKey[] = {'P', 'W', 'D', '=', '\0'};
    SQLWCHAR inConnectionString[4096];
    memSizeType pos = 0;
    SQLWCHAR outConnectionString[4096];
    SQLSMALLINT outConnectionStringLength;
    SQLRETURN returnCode;
    boolType okay = TRUE;

  /* connectToServer */
    logFunction(printf("connectToServer(\"");
                printWstri(server);
                printf("\", " FMT_U_MEM ")\n", serverLength););
    memcpy(&inConnectionString[pos], serverKey, STRLEN(serverKey) * sizeof(SQLWCHAR));
    pos += STRLEN(serverKey);
    memcpy(&inConnectionString[pos], server, serverLength * sizeof(SQLWCHAR));
    pos += serverLength;
    if (connectData->databaseLength != 0) {
      inConnectionString[pos++] = ';';
      memcpy(&inConnectionString[pos], databaseKey, STRLEN(databaseKey) * sizeof(SQLWCHAR));
      pos += STRLEN(databaseKey);
      memcpy(&inConnectionString[pos], connectData->database, connectData->databaseLength * sizeof(SQLWCHAR));
      pos += connectData->databaseLength;
    } /* if */
    if (connectData->uidLength != 0) {
      inConnectionString[pos++] = ';';
      memcpy(&inConnectionString[pos], uidKey, STRLEN(uidKey) * sizeof(SQLWCHAR));
      pos += STRLEN(uidKey);
      memcpy(&inConnectionString[pos], connectData->uid, connectData->uidLength * sizeof(SQLWCHAR));
      pos += connectData->uidLength;
    } /* if */
    if (connectData->pwdLength != 0) {
      inConnectionString[pos++] = ';';
      memcpy(&inConnectionString[pos], pwdKey, STRLEN(pwdKey) * sizeof(SQLWCHAR));
      pos += STRLEN(pwdKey);
      memcpy(&inConnectionString[pos], connectData->pwd, connectData->pwdLength * sizeof(SQLWCHAR));
      pos += connectData->pwdLength;
    } /* if */
    inConnectionString[pos] = '\0';
    logMessage(printf("inConnectionString: ");
               printWstri(inConnectionString);
               printf("\n"););
    outConnectionString[0] = '\0';
    returnCode = SQLDriverConnectW(sql_connection,
                                   NULL, /* GetDesktopWindow(), */
                                   (SQLWCHAR *) inConnectionString,
                                   (SQLSMALLINT) pos,
                                   outConnectionString,
                                   sizeof(outConnectionString) / sizeof(SQLWCHAR),
                                   &outConnectionStringLength,
                                   SQL_DRIVER_NOPROMPT);
    /* printf("returnCode: %d\n", returnCode); */
    logMessage(printf("outConnectionString: ");
               printWstri(outConnectionString);
               printf("\n"););
    if (unlikely(returnCode != SQL_SUCCESS &&
                 returnCode != SQL_SUCCESS_WITH_INFO)) {
      setDbErrorMsg("connectToServer", "SQLDriverConnectW",
                    SQL_HANDLE_DBC, sql_connection);
      logError(printf("connectToServer: SQLDriverConnectW:\n%s\n",
                      dbError.message););
      okay = FALSE;
    } /* if */
    logFunction(printf("connectToServer --> %s\n", okay ? "TRUE" : "FALSE"););
    return okay;
  } /* connectToServer */



static boolType connectToLocalServer (connectDataType connectData,
    SQLHDBC sql_connection)

  {
    const SQLWCHAR serverKey[] = {'S', 'E', 'R', 'V', 'E', 'R', '\0'};
    SQLWCHAR inConnectionString[1];
    SQLWCHAR outConnectionString[4096];
    SQLSMALLINT outConnectionStringLength;
    SQLWCHAR *posFound;
    SQLWCHAR *server;
    boolType lastServer;
    SQLRETURN returnCode;
    boolType triedToConnect = FALSE;
    boolType okay = FALSE;

  /* connectToLocalServer */
    logFunction(printf("connectToLocalServer\n"););
    inConnectionString[0] = '\0';
    logMessage(printf("inConnectionString: ");
               printWstri(inConnectionString);
               printf("\n"););
    outConnectionString[0] = '\0';
    returnCode = SQLBrowseConnectW(sql_connection,
                                   (SQLWCHAR *) inConnectionString,
                                   (SQLSMALLINT) 0,
                                   outConnectionString,
                                   sizeof(outConnectionString) / sizeof(SQLWCHAR),
                                   &outConnectionStringLength);
    logMessage(printf("returnCode: %d\n", returnCode);
               printf("outConnectionString: ");
               printWstri(outConnectionString);
               printf("\n"););
    if (returnCode == SQL_SUCCESS ||
        returnCode == SQL_SUCCESS_WITH_INFO ||
        returnCode == SQL_NEED_DATA) {
      SQLDisconnect(sql_connection);
      posFound = wstriSearch(outConnectionString, serverKey);
      if (posFound != NULL) {
        posFound += STRLEN(serverKey);
        while (*posFound != '=' && *posFound != '\0') {
          posFound++;
        } /* while */
        if (*posFound == '=') {
          posFound++;
          if (*posFound == '{') {
            posFound++;
          } /* if */
          do {
            server = posFound;
            while (*posFound != ',' && *posFound != '}' &&
                   *posFound != ';' && *posFound != '\0') {
              posFound++;
            } /* while */
            lastServer = *posFound != ',';
            *posFound = '\0';
            /* printf("check for server: ");
               printWstri(server);
               printf("\n"); */
            if (server != posFound) {
              /* Only try to connect if the server name is not empty. */
              okay = connectToServer(connectData, sql_connection, server,
                                     (memSizeType) (posFound - server));
              triedToConnect = TRUE;
            } /* if */
            posFound++;
          } while (!okay && !lastServer);
        } /* if */
      } /* if */
    } /* if */
    if (!triedToConnect) {
      dbLibError("connectToLocalServer", "SQLBrowseConnectW",
                 "No local database server found.\n");
      logError(printf("connectToLocalServer: SQLBrowseConnectW:\n%s\n",
                      dbError.message););
    } /* if */
    logFunction(printf("connectToLocalServer --> %s\n", okay ? "TRUE" : "FALSE"););
    return okay;
  } /* connectToLocalServer */



static databaseType doOpenSqlServer (connectDataType connectData, errInfoType *err_info)

  {
    SQLHENV sql_environment;
    SQLHDBC sql_connection;
    boolType okay;
    SQLRETURN returnCode = SQL_ERROR;
    SQLSMALLINT outConnectionStringLength;
    SQLWCHAR outConnectionString[4096];
    databaseType database;

  /* doOpenSqlServer */
    logFunction(printf("doOpenSqlServer(*, %d)\n", *err_info););
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
        if (connectData->serverLength == 0 && connectData->port == 0 &&
            FUNCTION_PRESENT(SQLBrowseConnectW)) {
          okay = connectToLocalServer(connectData, sql_connection);
        } else {
          dbLibError("doOpenSqlServer", "doOpenSqlServer",
                     "Cannot open database.\n");
          logError(printf("doOpenSqlServer:\n%s\n",
                          dbError.message););
          okay = FALSE;
        } /* if */
        if (!okay && connectData->databaseLength != 0) {
          /* The connection string must contain a database name. */
          logMessage(printf("inConnectionString: ");
                     printWstri(connectData->connectionString);
                     printf("\n"););
          outConnectionString[0] = '\0';
          returnCode = SQLDriverConnectW(sql_connection,
                                         NULL, /* GetDesktopWindow(), */
                                         (SQLWCHAR *) connectData->connectionString,
                                         (SQLSMALLINT) connectData->connectionStringLength,
                                         outConnectionString,
                                         sizeof(outConnectionString) / sizeof(SQLWCHAR),
                                         &outConnectionStringLength,
                                         SQL_DRIVER_NOPROMPT);
          /* printf("returnCode: %d\n", returnCode); */
          logMessage(printf("outConnectionString: ");
                     printWstri(outConnectionString);
                     printf("\n"););
          if (unlikely(returnCode != SQL_SUCCESS &&
                       returnCode != SQL_SUCCESS_WITH_INFO)) {
            setDbErrorMsg("sqlOpenSqlServer", "SQLDriverConnectW",
                          SQL_HANDLE_DBC, sql_connection);
            logError(printf("sqlOpenSqlServer: SQLDriverConnectW:\n%s\n",
                            dbError.message););
            okay = FALSE;
          } else {
            okay = TRUE;
          } /* if */
        } /* if */
        if (unlikely(!okay)) {
          *err_info = DATABASE_ERROR;
          SQLFreeHandle(SQL_HANDLE_DBC, sql_connection);
          SQLFreeHandle(SQL_HANDLE_ENV, sql_environment);
          database = NULL;
        } else {
          database = createDbRecord(sql_environment, sql_connection, DB_CATEGORY_SQL_SERVER, err_info);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("doOpenSqlServer --> " FMT_U_MEM " (err_info=%d)\n",
                       (memSizeType) database, *err_info););
    return database;
  } /* doOpenSqlServer */



databaseType sqlOpenSqlServer (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  {
    connectDataRecordSrv connectData;
    errInfoType err_info = OKAY_NO_ERROR;
    databaseType database;

  /* sqlOpenSqlServer */
    logFunction(printf("sqlOpenSqlServer(\"%s\", ",
                       striAsUnquotedCStri(host));
                printf(FMT_D ", \"%s\", ",
                       port, striAsUnquotedCStri(dbName));
                printf("\"%s\", ", striAsUnquotedCStri(user));
                printf("\"%s\")\n", striAsUnquotedCStri(password)););
    if (!findDll()) {
      logError(printf("sqlOpenSqlServer: findDll() failed\n"););
      err_info = DATABASE_ERROR;
      database = NULL;
    } else {
      connectData.port = port;
      connectData.server = stri_to_sqlwstri(host, &connectData.serverLength, &err_info);
      if (unlikely(connectData.server == NULL)) {
        database = NULL;
      } else {
        connectData.database = stri_to_sqlwstri(dbName, &connectData.databaseLength, &err_info);
        if (unlikely(connectData.database == NULL)) {
          database = NULL;
        } else {
          connectData.uid = stri_to_sqlwstri(user, &connectData.uidLength, &err_info);
          if (unlikely(connectData.uid == NULL)) {
            database = NULL;
          } else {
            connectData.pwd = stri_to_sqlwstri(password, &connectData.pwdLength, &err_info);
            if (unlikely(connectData.pwd == NULL)) {
              database = NULL;
            } else {
              if (unlikely(!createConnectionString(&connectData))) {
                err_info = MEMORY_ERROR;
                database = NULL;
              } else {
                database = doOpenSqlServer(&connectData, &err_info);
                UNALLOC_SQLWSTRI(connectData.connectionString, connectData.connectionStringLength);
              } /* if */
              UNALLOC_SQLWSTRI(connectData.pwd, connectData.pwdLength);
            } /* if */
            UNALLOC_SQLWSTRI(connectData.uid, connectData.uidLength);
          } /* if */
          UNALLOC_SQLWSTRI(connectData.database, connectData.databaseLength);
        } /* if */
        UNALLOC_SQLWSTRI(connectData.server, connectData.serverLength);
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("sqlOpenSqlServer --> " FMT_U_MEM "\n",
                       (memSizeType) database););
    return database;
  } /* sqlOpenSqlServer */

#else



databaseType sqlOpenSqlServer (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  { /* sqlOpenSqlServer */
    logError(printf("sqlOpenSqlServer(\"%s\", ",
                    striAsUnquotedCStri(host));
             printf(FMT_D ", \"%s\", ",
                    port, striAsUnquotedCStri(dbName));
             printf("\"%s\", ", striAsUnquotedCStri(user));
             printf("\"%s\"): MS SQL-Server driver not present.\n",
                    striAsUnquotedCStri(password)););
    raise_error(RANGE_ERROR);
    return NULL;
  } /* sqlOpenSqlServer */

#endif
