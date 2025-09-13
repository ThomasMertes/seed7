/********************************************************************/
/*                                                                  */
/*  sql_odbc.c    Database access functions for the ODBC interface. */
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
/*  File: seed7/src/sql_odbc.c                                      */
/*  Changes: 2014, 2015, 2017 - 2019  Thomas Mertes                 */
/*  Content: Database access functions for the ODBC interface.      */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "wchar.h"
#include "time.h"
#include "limits.h"
#ifdef ODBC_INCLUDE
#if ODBC_INCLUDE_WINDOWS_H
#include "windows.h"
#endif
#include ODBC_INCLUDE
#if ODBC_INCLUDE_SQLEXT_H
#include "sqlext.h"
#endif
#endif

#include "common.h"
#include "data_rtl.h"
#include "striutl.h"
#include "heaputl.h"
#include "numutl.h"
#include "str_rtl.h"
#include "bst_rtl.h"
#include "int_rtl.h"
#include "tim_rtl.h"
#include "cmd_rtl.h"
#include "big_drv.h"
#include "rtl_err.h"
#include "dll_drv.h"
#include "sql_base.h"
#include "sql_drv.h"

#ifdef ODBC_INCLUDE

#ifdef ODBC_DLL
#define CLI_DLL ODBC_DLL
#endif

#define loadBaseDlls()
#define ODBC_DRIVER_FUNCTIONS_NEEDED
#define SIZEOF_SQLWCHAR ODBC_SIZEOF_SQLWCHAR

#include "sql_cli.c"

typedef struct {
    SQLWCHAR *driverW;
    memSizeType driverW_length;
    SQLWCHAR *serverW;
    memSizeType serverW_length;
    SQLWCHAR *dbNameW;
    memSizeType dbNameW_length;
    SQLWCHAR *userW;
    memSizeType userW_length;
    SQLWCHAR *passwordW;
    memSizeType passwordW_length;
  } connectDataRecord, *connectDataType;



static SQLWCHAR *getRegularName (SQLWCHAR *wstri, memSizeType wstriLength)

  {
    SQLWCHAR *destWstri;
    SQLWCHAR *compressedWstri;

  /* getRegularName */
    if (ALLOC_SQLWSTRI(compressedWstri, wstriLength)) {
      destWstri = compressedWstri;
      while (*wstri != '\0') {
        if (*wstri >= 'A' && *wstri <= 'Z') {
          *destWstri = (SQLWCHAR) (*wstri - 'A' + 'a');
          destWstri++;
        } else if (*wstri != ' ') {
          *destWstri = *wstri;
          destWstri++;
        } /* if */
        wstri++;
      } /* while */
      *destWstri = '\0';
    } /* if */
    return compressedWstri;
  } /* getRegularName */



static const SQLWCHAR *wstriSearchCh (const SQLWCHAR *str, const SQLWCHAR ch)

  { /* wstriSearchCh */
    for (; *str != ch; str++) {
      if (*str == (const SQLWCHAR) 0) {
        return NULL;
      } /* if */
    } /* for */
    return str;
  } /* wstriSearchCh */



static const SQLWCHAR *wstriSearch (const SQLWCHAR *haystack, const SQLWCHAR *needle)

  {
    const SQLWCHAR *sc1;
    const SQLWCHAR *sc2;

  /* wstriSearch */
    if (*needle == (const SQLWCHAR) 0) {
      return haystack;
    } else {
      for (; (haystack = wstriSearchCh(haystack, *needle)) != NULL; haystack++) {
        for (sc1 = haystack, sc2 = needle; ; ) {
          if (*++sc2 == (const SQLWCHAR) 0) {
            return haystack;
          } else if (*++sc1 != *sc2) {
            break;
          } /* if */
        } /* for */
      } /* for */
    } /* if */
    return NULL;
  } /* wstriSearch */



static boolType connectToServer (connectDataType connectData,
    SQLHDBC sql_connection, const SQLWCHAR *driver, memSizeType driverLength,
    const SQLWCHAR *server, memSizeType serverLength)

  {
    const SQLWCHAR driverKey[] = {'D', 'R', 'I', 'V', 'E', 'R', '=', '\0'};
    const SQLWCHAR serverKey[] = {'S', 'E', 'R', 'V', 'E', 'R', '=', '\0'};
    const SQLWCHAR databaseKey[] = {'D', 'A', 'T', 'A', 'B', 'A', 'S', 'E', '=', '\0'};
    const SQLWCHAR uidKey[] = {'U', 'I', 'D', '=', '\0'};
    const SQLWCHAR pwdKey[] = {'P', 'W', 'D', '=', '\0'};
    SQLWCHAR inConnectionString[4096];
    memSizeType stringLength;
    SQLWCHAR outConnectionString[4096];
    SQLSMALLINT outConnectionStringLength;
    SQLRETURN returnCode;

  /* connectToServer */
    logFunction(printf("connectToServer(*, *, \"");
                printWstri(driver);
                printf("\", " FMT_U_MEM ", \"", driverLength);
                printWstri(server);
                printf("\", " FMT_U_MEM ")\n", serverLength););
    memcpy(inConnectionString, driverKey, STRLEN(driverKey) * sizeof(SQLWCHAR));
    stringLength = STRLEN(driverKey);
    memcpy(&inConnectionString[stringLength], driver, driverLength * sizeof(SQLWCHAR));
    stringLength += driverLength;
    inConnectionString[stringLength] = ';';
    stringLength++;
    memcpy(&inConnectionString[stringLength], serverKey, STRLEN(serverKey) * sizeof(SQLWCHAR));
    stringLength += STRLEN(serverKey);
    memcpy(&inConnectionString[stringLength], server, serverLength * sizeof(SQLWCHAR));
    stringLength += serverLength;
    if (connectData->dbNameW_length != 0) {
      inConnectionString[stringLength] = ';';
      stringLength++;
      memcpy(&inConnectionString[stringLength], databaseKey, STRLEN(databaseKey) * sizeof(SQLWCHAR));
      stringLength += STRLEN(databaseKey);
      memcpy(&inConnectionString[stringLength], connectData->dbNameW, connectData->dbNameW_length * sizeof(SQLWCHAR));
      stringLength += connectData->dbNameW_length;
    } /* if */
    if (connectData->userW_length != 0) {
      inConnectionString[stringLength] = ';';
      stringLength++;
      memcpy(&inConnectionString[stringLength], uidKey, STRLEN(uidKey) * sizeof(SQLWCHAR));
      stringLength += STRLEN(uidKey);
      memcpy(&inConnectionString[stringLength], connectData->userW, connectData->userW_length * sizeof(SQLWCHAR));
      stringLength += connectData->userW_length;
    } /* if */
    if (connectData->passwordW_length != 0) {
      inConnectionString[stringLength] = ';';
      stringLength++;
      memcpy(&inConnectionString[stringLength], pwdKey, STRLEN(pwdKey) * sizeof(SQLWCHAR));
      stringLength += STRLEN(pwdKey);
      memcpy(&inConnectionString[stringLength], connectData->passwordW, connectData->passwordW_length * sizeof(SQLWCHAR));
      stringLength += connectData->passwordW_length;
    } /* if */
    inConnectionString[stringLength] = '\0';
    /* printf("inConnectionString: ");
       printWstri(inConnectionString);
       printf("\n"); */
    outConnectionString[0] = '\0';
    returnCode = SQLDriverConnectW(sql_connection,
                                   NULL, /* GetDesktopWindow(), */
                                   (SQLWCHAR *) inConnectionString,
                                   (SQLSMALLINT) stringLength,
                                   outConnectionString,
                                   sizeof(outConnectionString) / sizeof(SQLWCHAR),
                                   &outConnectionStringLength,
                                   SQL_DRIVER_NOPROMPT);
    /* printf("returnCode: %d\n", returnCode); */
    /* printf("outConnectionString: ");
       printWstri(outConnectionString);
       printf("\n"); */
    if (returnCode != SQL_SUCCESS && returnCode != SQL_SUCCESS_WITH_INFO) {
      setDbErrorMsg("connectToServer", "SQLDriverConnectW",
                    SQL_HANDLE_DBC, sql_connection);
      logError(printf("connectToServer: SQLDriverConnectW:\n%s\n", dbError.message););
    } /* if */
    logFunction(printf("connectToServer --> %d\n",
                       returnCode == SQL_SUCCESS || returnCode == SQL_SUCCESS_WITH_INFO););
    return returnCode == SQL_SUCCESS || returnCode == SQL_SUCCESS_WITH_INFO;
  } /* connectToServer */



static boolType connectToDriver (connectDataType connectData,
    SQLHDBC sql_connection, const SQLWCHAR *driver, memSizeType driverLength)

  {
    const SQLWCHAR driverKey[] = {'D', 'R', 'I', 'V', 'E', 'R', '=', '\0'};
    const SQLWCHAR serverKey[] = {'S', 'E', 'R', 'V', 'E', 'R', '\0'};
    SQLWCHAR inConnectionString[4096];
    memSizeType stringLength;
    SQLWCHAR outConnectionString[4096];
    SQLSMALLINT outConnectionStringLength;
    SQLWCHAR *regularNameOfSearchedServer;
    const SQLWCHAR *posFound;
    const SQLWCHAR *server;
    memSizeType serverNameLength;
    SQLWCHAR *serverName;
    SQLWCHAR *regularServerName;
    boolType lastServer;
    SQLRETURN returnCode;
    boolType okay = FALSE;

  /* connectToDriver */
    logFunction(printf("connectToDriver(*, *, \"");
                printWstri(driver);
                printf("\", " FMT_U_MEM ")\n", driverLength););
    regularNameOfSearchedServer = getRegularName(connectData->serverW,
        connectData->serverW_length);
    if (regularNameOfSearchedServer != NULL) {
      /* printf("Searching for server: ");
         printWstri(regularNameOfSearchedServer);
         printf("\n"); */
      memcpy(inConnectionString, driverKey, STRLEN(driverKey) * sizeof(SQLWCHAR));
      stringLength = STRLEN(driverKey);
      memcpy(&inConnectionString[stringLength], driver, driverLength * sizeof(SQLWCHAR));
      stringLength += driverLength;
      inConnectionString[stringLength] = '\0';
      /* printf("inConnectionString: ");
         printWstri(inConnectionString);
         printf("\n"); */
      outConnectionString[0] = '\0';
      returnCode = SQLBrowseConnectW(sql_connection,
                                     (SQLWCHAR *) inConnectionString,
                                     (SQLSMALLINT) stringLength,
                                     outConnectionString,
                                     sizeof(outConnectionString) / sizeof(SQLWCHAR),
                                     &outConnectionStringLength);
      /* printf("returnCode: %d\n", returnCode); */
      /* printf("outConnectionString: ");
         printWstri(outConnectionString);
         printf("\n"); */
      if (returnCode == SQL_SUCCESS || returnCode == SQL_SUCCESS_WITH_INFO ||
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
              serverNameLength = (memSizeType) (posFound - server);
              if (ALLOC_SQLWSTRI(serverName, serverNameLength)) {
                memcpy(serverName, server, serverNameLength * sizeof(SQLWCHAR));
                serverName[serverNameLength] = '\0';
                /* printf("check for server: ");
                   printWstri(serverName);
                   printf("\n"); */
                regularServerName = getRegularName(serverName, serverNameLength);
                if (regularServerName != NULL) {
                  if (wstriSearch(regularServerName, regularNameOfSearchedServer) != NULL) {
                    /* printf("server that matches requested one: ");
                       printWstri(serverName);
                       printf("\n"); */
                    okay = connectToServer(connectData, sql_connection,
                                           driver, (memSizeType) driverLength,
                                           serverName, serverNameLength);
                  } /* if */
                  UNALLOC_SQLWSTRI(regularServerName, serverNameLength);
                } /* if */
                UNALLOC_SQLWSTRI(serverName, serverNameLength);
              } /* if */
              posFound++;
            } while (!okay && !lastServer);
          } /* if */
        } /* if */
      } /* if */
      UNALLOC_SQLWSTRI(regularNameOfSearchedServer, connectData->serverW_length);
    } /* if */
    if (!okay) {
      okay = connectToServer(connectData, sql_connection, driver,
                             (memSizeType) driverLength, connectData->serverW,
                             connectData->serverW_length);
    } /* if */
    logFunction(printf("connectToDriver --> %d\n", okay););
    return okay;
  } /* connectToDriver */



static boolType driverConnect (connectDataType connectData, SQLHDBC sql_connection,
    SQLHENV sql_environment)

  {
    SQLWCHAR driver[4096];
    SQLWCHAR attr[4096];
    SQLWCHAR *regularNameOfSearchedDriver;
    SQLWCHAR *regularDriverName;
    SQLSMALLINT driverLength;
    SQLSMALLINT attrLength;
    SQLUSMALLINT direction;
    boolType okay = FALSE;

  /* driverConnect */
    logFunction(printf("driverConnect\n"););
    if (connectData->driverW_length != 0) {
      regularNameOfSearchedDriver = getRegularName(connectData->driverW,
          connectData->driverW_length);
      if (regularNameOfSearchedDriver != NULL) {
        /* printf("Searching for driver: ");
           printWstri(regularNameOfSearchedDriver);
           printf("\n"); */
        direction = SQL_FETCH_FIRST;
        while (!okay &&
               SQLDriversW(sql_environment, direction, driver,
                           sizeof(driver) / sizeof(SQLWCHAR), &driverLength,
                           attr, sizeof(attr) / sizeof(SQLWCHAR),
                           &attrLength) == SQL_SUCCESS) {
          direction = SQL_FETCH_NEXT;
          /* printWstri(driver);
          printf("\n"); */
          regularDriverName = getRegularName(driver, (memSizeType) driverLength);
          if (regularDriverName != NULL) {
            if (wstriSearch(regularDriverName, regularNameOfSearchedDriver) != NULL) {
              /* printf("driver that matches requested one: ");
                 printWstri(driver);
                 printf("\n"); */
              okay = connectToDriver(connectData, sql_connection, driver,
                                     (memSizeType) driverLength);
            } /* if */
            UNALLOC_SQLWSTRI(regularDriverName, driverLength);
          } /* if */
        } /* while */
        UNALLOC_SQLWSTRI(regularNameOfSearchedDriver, connectData->driverW_length);
      } /* if */
    } /* if */
    logFunction(printf("driverConnect --> %d\n", okay););
    return okay;
  } /* driverConnect */



#if ANY_LOG_ACTIVE
static void listDrivers (connectDataType connectData, SQLHDBC sql_connection,
    SQLHENV sql_environment)

  {
    SQLWCHAR driver[4096];
    SQLWCHAR attr[4096];
    SQLSMALLINT driverLength;
    SQLSMALLINT attrLength;
    SQLUSMALLINT direction;
    SQLWCHAR *attrPtr;

  /* listDrivers */
    printf("Available odbc drivers:\n");
    direction = SQL_FETCH_FIRST;
    while (SQLDriversW(sql_environment, direction,
                       driver, sizeof(driver) / sizeof(SQLWCHAR), &driverLength,
                       attr, sizeof(attr) / sizeof(SQLWCHAR), &attrLength) == SQL_SUCCESS) {
      direction = SQL_FETCH_NEXT;
      printWstri(driver);
      printf("\n");
      attrPtr = attr;
      while (*attrPtr != '\0') {
        printf("  ");
        printWstri(attrPtr);
        printf("\n");
        while (*attrPtr != '\0') {
          attrPtr++;
        } /* while */
        attrPtr++;
      } /* while */
      /* peekDriver(connectData, sql_connection, driver, driverLength); */
    } /* while */
    printf("--- end of list ---\n");
  } /* listDrivers */



static void listDataSources (SQLHENV sql_environment)

  {
    SQLCHAR dsn[4096];
    SQLCHAR desc[4096];
    SQLSMALLINT dsn_ret;
    SQLSMALLINT desc_ret;
    SQLUSMALLINT direction;

  /* listDataSources */
    printf("List of data sources:\n");
    direction = SQL_FETCH_FIRST;
    while (SQLDataSources(sql_environment, direction,
                          dsn, sizeof(dsn), &dsn_ret,
                          desc, sizeof(desc), &desc_ret) == SQL_SUCCESS) {
      direction = SQL_FETCH_NEXT;
      printf("%s - %s\n", dsn, desc);
    } /* while */
    printf("--- end of list ---\n");
  } /* listDataSources */
#endif



databaseType sqlOpenOdbc (const const_striType driver,
    const const_striType server, const const_striType dbName,
    const const_striType user, const const_striType password)

  {
    connectDataRecord connectData;
    SQLHENV sql_environment;
    SQLHDBC sql_connection;
    SQLRETURN returnCode;
    errInfoType err_info = OKAY_NO_ERROR;
    databaseType database;

  /* sqlOpenOdbc */
    logFunction(printf("sqlOpenOdbc(\"%s\", ",
                       striAsUnquotedCStri(driver));
                printf("\"%s\", ", striAsUnquotedCStri(server));
                printf("\"%s\", ", striAsUnquotedCStri(dbName));
                printf("\"%s\", *)\n", striAsUnquotedCStri(user)););
    if (!findDll()) {
      logError(printf("sqlOpenOdbc: findDll() failed\n"););
      err_info = DATABASE_ERROR;
      database = NULL;
    } else if (unlikely((connectData.driverW =  stri_to_sqlwstri(driver,
                             &connectData.driverW_length, &err_info)) == NULL)) {
      database = NULL;
    } else {
      connectData.serverW = stri_to_sqlwstri(server, &connectData.serverW_length, &err_info);
      if (unlikely(connectData.serverW == NULL)) {
        database = NULL;
      } else {
        connectData.dbNameW = stri_to_sqlwstri(dbName, &connectData.dbNameW_length, &err_info);
        if (unlikely(connectData.dbNameW == NULL)) {
          database = NULL;
        } else {
          connectData.userW = stri_to_sqlwstri(user, &connectData.userW_length, &err_info);
          if (unlikely(connectData.userW == NULL)) {
            database = NULL;
          } else {
            connectData.passwordW = stri_to_sqlwstri(password, &connectData.passwordW_length, &err_info);
            if (unlikely(connectData.passwordW == NULL)) {
              database = NULL;
            } else {
              /* printf("dbName:   %ls\n", connectData.dbNameW);
                 printf("user:     %ls\n", connectData.userW);
                 printf("password: *\n"); */
              if (unlikely(connectData.dbNameW_length   > SHRT_MAX ||
                           connectData.userW_length     > SHRT_MAX ||
                           connectData.passwordW_length > SHRT_MAX)) {
                err_info = MEMORY_ERROR;
                database = NULL;
              } else {
                err_info = prepareSqlConnection(&sql_environment, &sql_connection);
                if (unlikely(err_info != OKAY_NO_ERROR)) {
                  database = NULL;
                } else {
                  if (connectData.driverW_length == 0 && connectData.serverW_length == 0) {
                    returnCode = SQLConnectW(sql_connection,
                        (SQLWCHAR *) connectData.dbNameW, (SQLSMALLINT) connectData.dbNameW_length,
                        (SQLWCHAR *) connectData.userW, (SQLSMALLINT) connectData.userW_length,
                        (SQLWCHAR *) connectData.passwordW, (SQLSMALLINT) connectData.passwordW_length);
                    logMessage(printf("SQLConnectW returns " FMT_D16 "\n", returnCode););
                  } else {
                    returnCode = SQL_ERROR;
                  } /* if */
                  if ((returnCode != SQL_SUCCESS &&
                       returnCode != SQL_SUCCESS_WITH_INFO) &&
                      !driverConnect(&connectData, sql_connection, sql_environment)) {
                    setDbErrorMsg("sqlOpenOdbc", "SQLConnect",
                                  SQL_HANDLE_DBC, sql_connection);
                    logError(printf("sqlOpenOdbc: SQLConnect:\n%s\n",
                                    dbError.message);
                             listDrivers(&connectData, sql_connection, sql_environment);
                             listDataSources(sql_environment););
                    err_info = DATABASE_ERROR;
                    SQLFreeHandle(SQL_HANDLE_DBC, sql_connection);
                    SQLFreeHandle(SQL_HANDLE_ENV, sql_environment);
                    database = NULL;
                  } else {
                    database = createDbRecord(sql_environment, sql_connection, DB_CATEGORY_ODBC, &err_info);
                  } /* if */
                } /* if */
              } /* if */
              UNALLOC_SQLWSTRI(connectData.passwordW, password);
            } /* if */
            UNALLOC_SQLWSTRI(connectData.userW, user);
          } /* if */
          UNALLOC_SQLWSTRI(connectData.dbNameW, dbName);
        } /* if */
        UNALLOC_SQLWSTRI(connectData.serverW, server);
      } /* if */
      UNALLOC_SQLWSTRI(connectData.driverW, driver);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("sqlOpenOdbc --> " FMT_U_MEM "\n",
                       (memSizeType) database););
    return database;
  } /* sqlOpenOdbc */

#else



databaseType sqlOpenOdbc (const const_striType driver,
    const const_striType server, const const_striType dbName,
    const const_striType user, const const_striType password)

  { /* sqlOpenOdbc */
    logError(printf("sqlOpenOdbc(\"%s\", ",
                    striAsUnquotedCStri(driver));
             printf("\"%s\", ", striAsUnquotedCStri(server));
             printf("\"%s\", ", striAsUnquotedCStri(dbName));
             printf("\"%s\", *): ODBC driver not present.\n",
                    striAsUnquotedCStri(user)););
    raise_error(RANGE_ERROR);
    return NULL;
  } /* sqlOpenOdbc */

#endif
