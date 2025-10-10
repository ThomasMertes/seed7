/********************************************************************/
/*                                                                  */
/*  sql_ifx.c     Database access functions for Informix.           */
/*  Copyright (C) 1989 - 2022  Thomas Mertes                        */
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
/*  File: seed7/src/sql_ifx.c                                       */
/*  Changes: 2022  Thomas Mertes                                    */
/*  Content: Database access functions for Informix.                */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#define LOG_CLI_FUNCTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "limits.h"
#ifdef INFORMIX_INCLUDE
#if INFORMIX_INCLUDE_WINDOWS_H
#include "windows.h"
#endif
#include INFORMIX_INCLUDE
#if INFORMIX_INCLUDE_SQLEXT_H
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

#ifdef INFORMIX_INCLUDE

#ifdef INFORMIX_DLL
#define CLI_DLL INFORMIX_DLL
static void loadBaseDlls (void);
#endif

#define ALLOW_EXECUTE_SUCCESS_WITH_INFO
#define ALLOW_FETCH_SUCCESS_WITH_INFO
#define SIZEOF_SQLWCHAR INFORMIX_SIZEOF_SQLWCHAR
#define INTERVAL_FRACTION_PRECISION 6

#include "sql_cli.c"

typedef struct {
    SQLWCHAR *hostname;
    memSizeType hostnameLength;
    intType port;
    SQLWCHAR *server;
    memSizeType serverLength;
    SQLWCHAR *database;
    memSizeType databaseLength;
    SQLWCHAR *uid;
    memSizeType uidLength;
    SQLWCHAR *pwd;
    memSizeType pwdLength;
    SQLWCHAR *connectionString1;
    memSizeType connectionStringLength1;
    SQLWCHAR *connectionString2;
    memSizeType connectionStringLength2;
  } connectDataRecordIfx, *connectDataType;

#define DEFAULT_PORT 1526



#ifdef INFORMIX_DLL
static void loadBaseDlls (void)

  { /* loadBaseDlls */
    logFunction(printf("loadBaseDlls\n"););
#ifdef IFGLS_DLL
    {
      const char *libIfglsDllList[] = { IFGLS_DLL };
      unsigned int pos;
      boolType found = FALSE;

      for (pos = 0; pos < sizeof(libIfglsDllList) / sizeof(char *) && !found; pos++) {
        found = dllOpen(libIfglsDllList[pos]) != NULL;
      } /* for */
    }
#endif
    logFunction(printf("loadBaseDlls -->\n"););
  } /* loadBaseDlls */
#endif



static boolType createConnectionString (connectDataType connectData, boolType withDbLocale,
    SQLWCHAR **connectionStringDest, memSizeType *connectionStringLengthDest)

  {
    const SQLWCHAR databaseKey[] = {'D', 'a', 't', 'a', 'b', 'a', 's', 'e', '=', '\0'};
    const SQLWCHAR hostnameKey[] = {'H', 'o', 's', 't', '=', '\0'};
    const SQLWCHAR localhost[] = {'l', 'o', 'c', 'a', 'l', 'h', 'o', 's', 't', '\0'};
    const SQLWCHAR serverKey[] = {'S', 'e', 'r', 'v', 'e', 'r', '=', '\0'};
    const SQLWCHAR portKey[] = {'S', 'e', 'r', 'v', 'i', 'c', 'e', '=', '\0'};
    const SQLWCHAR protocolKeyAndValue[] = {'P', 'r', 'o', 't', 'o', 'c', 'o', 'l', '=', 'o', 'n', 's', 'o', 'c', 't', 'c', 'p', '\0'};
    const SQLWCHAR dbLocaleKeyAndValue[] = {'D', 'B', '_', 'L', 'O', 'C', 'A', 'L', 'E', '=', 'e', 'n', '_', 'U', 'S', '.', 'U', 'T', 'F', '8', '\0'};
    const SQLWCHAR clientLocaleKeyAndValue[] = {'C', 'L', 'I', 'E', 'N', 'T', '_', 'L', 'O', 'C', 'A', 'L', 'E', '=', 'e', 'n', '_', 'U', 'S', '.', 'U', 'T', 'F', '8', '\0'};
    const SQLWCHAR uidKey[] = {'U', 'I', 'D', '=', '\0'};
    const SQLWCHAR pwdKey[] = {'P', 'W', 'D', '=', '\0'};
    const SQLWCHAR *hostname;
    memSizeType hostnameLength;
    intType port;
    char portName[INTTYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    memSizeType portNameLength;
    memSizeType connectionStringLength;
    SQLWCHAR *connectionString;
    memSizeType pos = 0;
    boolType okay = FALSE;

  /* createConnectionString */
    logFunction(printf("createConnectionString([hostname=\"%s\", port=" FMT_D,
                       sqlwstriAsUnquotedCStri(connectData->hostname),
                       connectData->port);
                printf(", database=\"%s\"])\n",
                       sqlwstriAsUnquotedCStri(connectData->database)););

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

    if (likely(connectData->databaseLength <= SHRT_MAX &&
               connectData->hostnameLength <= SHRT_MAX &&
               connectData->serverLength <= SHRT_MAX &&
               connectData->uidLength <= SHRT_MAX &&
               connectData->pwdLength <= SHRT_MAX)) {
      connectionStringLength = STRLEN(databaseKey) + connectData->databaseLength +
                               1 + STRLEN(hostnameKey) + hostnameLength +
                               1 + STRLEN(serverKey) + connectData->serverLength +
                               1 + STRLEN(portKey) + portNameLength +
                               1 + STRLEN(protocolKeyAndValue) +
                               (withDbLocale ? 1 + STRLEN(dbLocaleKeyAndValue) : 0) +
                               1 + STRLEN(clientLocaleKeyAndValue);
      /* printf("connectionStringLength: " FMT_U_MEM "\n", connectionStringLength); >*/
      if (connectData->uidLength != 0) {
        connectionStringLength += 1 + STRLEN(uidKey) + connectData->uidLength;
      } /* if */
      if (connectData->pwdLength != 0) {
        connectionStringLength += 1 + STRLEN(pwdKey) + connectData->pwdLength;
      } /* if */

      if (likely(connectionStringLength <= SHRT_MAX &&
                 ALLOC_SQLWSTRI(connectionString, connectionStringLength))) {
        *connectionStringDest = connectionString;
        *connectionStringLengthDest = connectionStringLength;

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
        memcpy_to_sqlwstri(&connectionString[pos], portName, portNameLength);
        pos += portNameLength;

        connectionString[pos++] = ';';
        memcpy(&connectionString[pos], serverKey, STRLEN(serverKey) * sizeof(SQLWCHAR));
        pos += STRLEN(serverKey);
        memcpy(&connectionString[pos], connectData->server, connectData->serverLength * sizeof(SQLWCHAR));
        pos += connectData->serverLength;

        connectionString[pos++] = ';';
        memcpy(&connectionString[pos], protocolKeyAndValue, STRLEN(protocolKeyAndValue) * sizeof(SQLWCHAR));
        pos += STRLEN(protocolKeyAndValue);

        if (withDbLocale) {
          connectionString[pos++] = ';';
          memcpy(&connectionString[pos], dbLocaleKeyAndValue, STRLEN(dbLocaleKeyAndValue) * sizeof(SQLWCHAR));
          pos += STRLEN(dbLocaleKeyAndValue);
        } /* if */

        connectionString[pos++] = ';';
        memcpy(&connectionString[pos], clientLocaleKeyAndValue, STRLEN(clientLocaleKeyAndValue) * sizeof(SQLWCHAR));
        pos += STRLEN(clientLocaleKeyAndValue);

        if (connectData->uidLength != 0) {
          connectionString[pos++] = ';';
          memcpy(&connectionString[pos], uidKey, STRLEN(uidKey) * sizeof(SQLWCHAR));
          pos += STRLEN(uidKey);
          memcpy(&connectionString[pos], connectData->uid, connectData->uidLength * sizeof(SQLWCHAR));
          pos += connectData->uidLength;
        } /* if */

        logFunction(connectionString[pos] = '\0';
                    printf("createConnectionString --> TRUE (connectionString=\"%s%s\")\n",
                           sqlwstriAsUnquotedCStri(connectionString),
                           connectData->pwdLength != 0 ? ";PWD=*" : ""););

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
    logFunctionIfTrue(!okay,
                      printf("createConnectionString --> FALSE\n"););
    return okay;
  } /* createConnectionString */



static databaseType doOpenInformix (connectDataType connectData, errInfoType *err_info)

  {
    SQLHENV sql_environment = NULL;
    SQLHDBC sql_connection = NULL;
    SQLRETURN returnCode;
    SQLSMALLINT outConnectionStringLength;
    databaseType database;

  /* doOpenInformix */
    logFunction(printf("doOpenInformix(*, %d)\n", *err_info););
    if (unlikely(connectData->databaseLength > SHRT_MAX ||
                 connectData->uidLength > SHRT_MAX ||
                 connectData->pwdLength > SHRT_MAX ||
                 connectData->connectionStringLength1 > SHRT_MAX ||
                 connectData->connectionStringLength2 > SHRT_MAX)) {
      *err_info = MEMORY_ERROR;
      database = NULL;
    } else {
      *err_info = prepareSqlConnection(&sql_environment, &sql_connection);
      if (unlikely(*err_info != OKAY_NO_ERROR)) {
        database = NULL;
      } else {
        if (connectData->hostnameLength == 0 && connectData->port == 0) {
          logMessage(printf("doOpenInformix: SQLConnectW(" FMT_U_MEM
                            ", \"%s\", " FMT_U_MEM,
                            (memSizeType) sql_connection,
                            sqlwstriAsUnquotedCStri(connectData->database),
                            connectData->databaseLength);
                     printf(", \"%s\", " FMT_U_MEM ", *, *)\n",
                            sqlwstriAsUnquotedCStri(connectData->uid),
                            connectData->uidLength););
          returnCode = SQLConnectW(sql_connection,
              connectData->database, (SQLSMALLINT) connectData->databaseLength,
              connectData->uid, (SQLSMALLINT) connectData->uidLength,
              connectData->pwd, (SQLSMALLINT) connectData->pwdLength);
          if (returnCode != SQL_SUCCESS &&
              returnCode != SQL_SUCCESS_WITH_INFO) {
            setDbErrorMsg("sqlOpenInformix", "SQLConnectW",
                          SQL_HANDLE_DBC, sql_connection);
            logError(printf("sqlOpenInformix: SQLConnectW(" FMT_U_MEM
                            ", \"%s\", " FMT_U_MEM,
                            (memSizeType) sql_connection,
                            sqlwstriAsUnquotedCStri(connectData->database),
                            connectData->databaseLength);
                     printf(", \"%s\", " FMT_U_MEM ", *, *):\n"
                            "returnCode: " FMT_D16 "\n%s\n",
                            sqlwstriAsUnquotedCStri(connectData->uid),
                            connectData->uidLength,
                            returnCode, dbError.message););
          } /* if */
        } else {
          returnCode = SQL_ERROR;
        } /* if */
        if (returnCode != SQL_SUCCESS &&
            returnCode != SQL_SUCCESS_WITH_INFO) {
          returnCode = SQLDriverConnectW(sql_connection,
                                         NULL, /* GetDesktopWindow(), */
                                         (SQLWCHAR *) connectData->connectionString1,
                                         (SQLSMALLINT) connectData->connectionStringLength1,
                                         NULL, /* outConnectionString */
                                         0,
                                         &outConnectionStringLength,
                                         SQL_DRIVER_NOPROMPT);
        } /* if */
        if (returnCode != SQL_SUCCESS &&
            returnCode != SQL_SUCCESS_WITH_INFO) {
          returnCode = SQLDriverConnectW(sql_connection,
                                         NULL, /* GetDesktopWindow(), */
                                         (SQLWCHAR *) connectData->connectionString2,
                                         (SQLSMALLINT) connectData->connectionStringLength2,
                                         NULL, /* outConnectionString */
                                         0,
                                         &outConnectionStringLength,
                                         SQL_DRIVER_NOPROMPT);
        } /* if */
        if (returnCode != SQL_SUCCESS &&
            returnCode != SQL_SUCCESS_WITH_INFO) {
          if (connectData->hostnameLength != 0 || connectData->port != 0) {
            setDbErrorMsg("sqlOpenInformix", "SQLDriverConnect",
                          SQL_HANDLE_DBC, sql_connection);
            logError(printf("sqlOpenInformix: SQLDriverConnect:\n%s\n",
                            dbError.message););
          } /* if */
          *err_info = DATABASE_ERROR;
          SQLFreeHandle(SQL_HANDLE_DBC, sql_connection);
          SQLFreeHandle(SQL_HANDLE_ENV, sql_environment);
          database = NULL;
        } else {
          database = createDbRecord(sql_environment, sql_connection, DB_CATEGORY_INFORMIX, err_info);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("doOpenInformix --> " FMT_U_MEM " (err_info=%d)\n",
                       (memSizeType) database, *err_info););
    return database;
  } /* doOpenInformix */



databaseType sqlOpenInformix (const const_striType host, intType port,
    const const_striType server, const const_striType dbName,
    const const_striType user, const const_striType password)

  {
    connectDataRecordIfx connectData;
    errInfoType err_info = OKAY_NO_ERROR;
    databaseType database;

  /* sqlOpenInformix */
    logFunction(printf("sqlOpenInformix(\"%s\", ",
                       striAsUnquotedCStri(host));
                printf(FMT_D ", \"%s\", ",
                       port, striAsUnquotedCStri(server));
                printf("\"%s\", ", striAsUnquotedCStri(dbName));
                printf("\"%s\", *)\n", striAsUnquotedCStri(user)););
    if (!findDll()) {
      logError(printf("sqlOpenInformix: findDll() failed\n"););
      err_info = DATABASE_ERROR;
      database = NULL;
    } else {
      memset(&connectData, 0, sizeof(connectDataRecordIfx));
      connectData.port = port;
      connectData.hostname = stri_to_sqlwstri(host, &connectData.hostnameLength, &err_info);
      if (unlikely(connectData.hostname == NULL)) {
        database = NULL;
      } else {
        connectData.server = stri_to_sqlwstri(server, &connectData.serverLength, &err_info);
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
                if (unlikely(!createConnectionString(&connectData, FALSE,
                                                     &connectData.connectionString1,
                                                     &connectData.connectionStringLength1) ||
                             !createConnectionString(&connectData, TRUE,
                                                     &connectData.connectionString2,
                                                     &connectData.connectionStringLength2))) {
                  err_info = MEMORY_ERROR;
                  database = NULL;
                } else {
                  database = doOpenInformix(&connectData, &err_info);
                  UNALLOC_SQLWSTRI(connectData.connectionString1, connectData.connectionStringLength1);
                  UNALLOC_SQLWSTRI(connectData.connectionString2, connectData.connectionStringLength2);
                } /* if */
                UNALLOC_SQLWSTRI(connectData.pwd, connectData.pwdLength);
              } /* if */
              UNALLOC_SQLWSTRI(connectData.uid, connectData.uidLength);
            } /* if */
            UNALLOC_SQLWSTRI(connectData.database, connectData.databaseLength);
          } /* if */
          UNALLOC_SQLWSTRI(connectData.server, connectData.serverLength);
        } /* if */
        UNALLOC_SQLWSTRI(connectData.hostname, connectData.hostnameLength);
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("sqlOpenInformix --> " FMT_U_MEM "\n",
                       (memSizeType) database););
    return database;
  } /* sqlOpenInformix */

#else



databaseType sqlOpenInformix (const const_striType host, intType port,
    const const_striType server, const const_striType dbName,
    const const_striType user, const const_striType password)

  { /* sqlOpenInformix */
    logFunction(printf("sqlOpenInformix(\"%s\", ",
                       striAsUnquotedCStri(host));
                printf(FMT_D ", \"%s\", ",
                       port, striAsUnquotedCStri(server));
                printf("\"%s\", ", striAsUnquotedCStri(dbName));
                printf("\"%s\", *): Informix driver not present.\n",
                       striAsUnquotedCStri(user)););
    raise_error(RANGE_ERROR);
    return NULL;
  } /* sqlOpenInformix */

#endif
