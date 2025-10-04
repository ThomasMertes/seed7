/********************************************************************/
/*                                                                  */
/*  sql_cli.c     Database access functions for ODBC/CLI interface. */
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
/*  File: seed7/src/sql_cli.c                                       */
/*  Changes: 2014, 2015, 2017 - 2020, 2022  Thomas Mertes           */
/*  Content: Database access functions for ODBC/CLI interface.      */
/*                                                                  */
/********************************************************************/

typedef struct {
    uintType     usage_count;
    boolType     isOpen;
    sqlFuncType  sqlFunc;
    intType      driver;
    SQLHENV      sql_environment;
    SQLHDBC      connection;
    boolType     wideCharsSupported;
    boolType     tinyintIsUnsigned;
    SQLUSMALLINT maxConcurrentActivities;
  } dbRecordCli, *dbType;

typedef struct {
    int          sql_type;
    memSizeType  buffer_length;
    memSizeType  buffer_capacity;
    void        *buffer;
    SQLLEN       length;
    SQLSMALLINT  dataType;
    SQLULEN      paramSize;
    SQLSMALLINT  decimalDigits;
    SQLSMALLINT  nullable;
    boolType     bound;
  } bindDataRecordCli, *bindDataType;

typedef struct {
    SQLSMALLINT  c_type;
    memSizeType  buffer_length;
    SQLSMALLINT  dataType;
    SQLULEN      columnSize;
    SQLSMALLINT  decimalDigits;
    SQLSMALLINT  nullable;
    boolType     sql_data_at_exec;
  } resultDescrRecord, *resultDescrType;

typedef struct {
    void        *buffer;
    SQLLEN       length;
  } resultDataRecordCli, *resultDataType;


typedef struct fetchDataStruct *fetchDataType;

typedef struct fetchDataStruct {
    resultDataType result_array;
    SQLRETURN      fetch_result;
    fetchDataType  next;
  } fetchDataRecord;

typedef struct {
    uintType        usage_count;
    dbType          db;
    sqlFuncType     sqlFunc;
    SQLHSTMT        ppStmt;
    memSizeType     param_array_size;
    bindDataType    param_array;
    memSizeType     result_array_size;
    resultDescrType result_descr_array;
    fetchDataType   prefetched;
    fetchDataType   currentFetch;
    fetchDataRecord fetchRecord;
    boolType        hasBlob;
    boolType        executeSuccessful;
    boolType        fetchOkay;
    boolType        fetchFinished;
  } preparedStmtRecordCli, *preparedStmtType;

static sqlFuncType sqlFunc = NULL;

#if SIZEOF_SQLWCHAR == 2
#define SIZ_SQLWSTRI(len)                         SIZ_UTF16(len)
#define MAX_SQLWSTRI_LEN                          MAX_UTF16_LEN
#define ALLOC_SQLWSTRI(var,len)                   ALLOC_HEAP(var, SQLWCHAR *, SIZ_UTF16(len))
#define UNALLOC_SQLWSTRI(var,len)                 UNALLOC_UTF16(var,len)
#define stri_to_sqlwstri(stri, length, err_info)  (SQLWCHAR *) stri_to_wstri16(stri, length, err_info)
#define sqlwstri_to_stri(wstri, length, err_info) wstri16_to_stri((const_utf16striType) wstri, length, err_info)
#define copy_to_sqlwstri(wstri, stri, err_info)   stri_to_utf16(wstri, (stri)->mem, (stri)->size, err_info);
#define sqlwstriAsUnquotedCStri(wstri)            wstri16AsUnquotedCStri(wstri)
#elif SIZEOF_SQLWCHAR == 4
#define SIZ_SQLWSTRI(len)                         SIZ_UTF32(len)
#define MAX_SQLWSTRI_LEN                          MAX_UTF32_LEN
#define ALLOC_SQLWSTRI(var,len)                   ALLOC_HEAP(var, SQLWCHAR *, SIZ_UTF32(len))
#define UNALLOC_SQLWSTRI(var,len)                 UNALLOC_UTF32(var,len)
#define stri_to_sqlwstri(stri, length, err_info)  (SQLWCHAR *) stri_to_wstri32(stri, length, err_info)
#define sqlwstri_to_stri(wstri, length, err_info) wstri32_to_stri((const_utf32striType) wstri, length, err_info)
#define copy_to_sqlwstri(wstri, stri, err_info)   (memcpy(wstri, (stri)->mem, (stri)->size * sizeof(SQLWCHAR)), (stri)->size)
#define sqlwstriAsUnquotedCStri(wstri)            wstri32AsUnquotedCStri(wstri)
#else
#error "sizeof(SQLWCHAR) is neither 2 nor 4."
#endif
#if INTERVAL_FRACTION_PRECISION == 6
#define INTERVAL_FRACTION_FACTOR 1
#elif INTERVAL_FRACTION_PRECISION == 9
#define INTERVAL_FRACTION_FACTOR 1000
#else
#error "INTERVAL_FRACTION_PRECISION is neither 6 nor 9."
#endif

/* ODBC provides two possibilities to encode decimal values.        */
/*  1. As string of decimal digits.                                 */
/*  2. As binary encoded data in the struct SQL_NUMERIC_STRUCT.     */
/* Some databases provide decimal values beyond the capabilities of */
/* SQL_NUMERIC_STRUCT. Additionally SQL_NUMERIC_STRUCT values are   */
/* not correctly supported by some drivers. Therefore decimal       */
/* encoding is the default and encoding DECODE_NUMERIC_STRUCT       */
/* should be used with care.                                        */
#define DECODE_NUMERIC_STRUCT 0
#define ENCODE_NUMERIC_STRUCT 0
/* Maximum number of decimal digits that fits in SQL_NUMERIC_STRUCT */
#define MAX_NUMERIC_PRECISION 38
#define MIN_PRECISION_FOR_NUMERIC_AS_DECIMAL 100
#define MAX_PRECISION_FOR_NUMERIC_AS_DECIMAL 1000
#define DEFAULT_DECIMAL_SCALE 1000
#define SQLLEN_MAX (SQLLEN) (((SQLULEN) 1 << (8 * sizeof(SQLLEN) - 1)) - 1)
#define SQLINTEGER_MAX (SQLINTEGER) (((SQLUINTEGER) 1 << (8 * sizeof(SQLINTEGER) - 1)) - 1)
#define SQLINTEGER_MIN -SQLINTEGER_MAX - (SQLINTEGER) 1
#define SQLSMALLINT_MAX (SQLSMALLINT) (((SQLUSMALLINT) 1 << (8 * sizeof(SQLSMALLINT) - 1)) - 1)
#define ERROR_MESSAGE_BUFFER_SIZE 1000
#define MAX_DATETIME2_LENGTH 27
#define MAX_DURATION_LENGTH 53
#define CHARS_IN_NAME_BUFFER 256
#define MAX_WSTRI_TO_UTF8_EXPANSION_FACTOR 4

#ifndef SQL_BLOB
#define SQL_BLOB -98
#endif
#ifndef SQL_CLOB
#define SQL_CLOB -99
#endif
#ifndef SQL_INFX_UDT_VARYING
#define SQL_INFX_UDT_VARYING -101
#endif
#ifndef SQL_SS_TIME2
#define SQL_SS_TIME2 -154
#endif
#ifndef SQL_XML
#define SQL_XML -370
#endif

typedef struct {
    SQLUSMALLINT hour;
    SQLUSMALLINT minute;
    SQLUSMALLINT second;
    SQLUINTEGER fraction;
  } sqlSsTime2Struct;


/* Define formats for SQLLEN and SQLULEN */
#if POINTER_SIZE == 32
#define FMT_D_LEN FMT_D32
#define FMT_U_LEN FMT_U32
#elif POINTER_SIZE == 64
#define FMT_D_LEN FMT_D64
#define FMT_U_LEN FMT_U64
#endif


#ifdef CLI_DLL

#define FUNCTION_PRESENT(func) ((func) != NULL)

#ifndef STDCALL
#if defined(_WIN32) && HAS_STDCALL
#define STDCALL __stdcall
#else
#define STDCALL
#endif
#endif

typedef SQLRETURN (STDCALL *tp_SQLAllocHandle) (SQLSMALLINT handleType,
                                                SQLHANDLE   inputHandle,
                                                SQLHANDLE  *outputHandle);
typedef SQLRETURN (STDCALL *tp_SQLBindCol) (SQLHSTMT     statementHandle,
                                            SQLUSMALLINT columnNumber,
                                            SQLSMALLINT  targetType,
                                            SQLPOINTER   targetValue,
                                            SQLLEN       bufferLength,
                                            SQLLEN      *strLen_or_Ind);
typedef SQLRETURN (STDCALL *tp_SQLBindParameter) (SQLHSTMT     hstmt,
                                                  SQLUSMALLINT ipar,
                                                  SQLSMALLINT  fParamType,
                                                  SQLSMALLINT  fCType,
                                                  SQLSMALLINT  fSqlType,
                                                  SQLULEN      cbColDef,
                                                  SQLSMALLINT  ibScale,
                                                  SQLPOINTER   rgbValue,
                                                  SQLLEN       cbValueMax,
                                                  SQLLEN      *pcbValue);
typedef SQLRETURN (STDCALL *tp_SQLBrowseConnectW) (SQLHDBC      connectionHandle,
                                                   SQLWCHAR    *inConnectionString,
                                                   SQLSMALLINT  stringLength1,
                                                   SQLWCHAR    *outConnectionString,
                                                   SQLSMALLINT  bufferLength,
                                                   SQLSMALLINT *stringLength2Ptr);
typedef SQLRETURN (STDCALL *tp_SQLColAttributeW) (SQLHSTMT     statementHandle,
                                                  SQLUSMALLINT columnNumber,
                                                  SQLUSMALLINT fieldIdentifier,
                                                  SQLPOINTER   characterAttribute,
                                                  SQLSMALLINT  bufferLength,
                                                  SQLSMALLINT *stringLengthPtr,
                                                  SQLLEN      *numericAttribute);
typedef SQLRETURN (STDCALL *tp_SQLConnectW) (SQLHDBC     connectionHandle,
                                             SQLWCHAR   *serverName,
                                             SQLSMALLINT nameLength1,
                                             SQLWCHAR   *userName,
                                             SQLSMALLINT nameLength2,
                                             SQLWCHAR   *authentication,
                                             SQLSMALLINT nameLength3);
typedef SQLRETURN (STDCALL *tp_SQLDataSources) (SQLHENV      environmentHandle,
                                                SQLUSMALLINT direction,
                                                SQLCHAR     *serverName,
                                                SQLSMALLINT  bufferLength1,
                                                SQLSMALLINT *nameLength1,
                                                SQLCHAR     *description,
                                                SQLSMALLINT  bufferLength2,
                                                SQLSMALLINT *nameLength2);
typedef SQLRETURN (STDCALL *tp_SQLDescribeColW) (SQLHSTMT     statementHandle,
                                                 SQLUSMALLINT columnNumber,
                                                 SQLWCHAR    *columnName,
                                                 SQLSMALLINT  bufferLength,
                                                 SQLSMALLINT *nameLengthPtr,
                                                 SQLSMALLINT *dataTypePtr,
                                                 SQLULEN     *columnSizePtr,
                                                 SQLSMALLINT *decimalDigitsPtr,
                                                 SQLSMALLINT *nullablePtr);
typedef SQLRETURN (STDCALL *tp_SQLDescribeParam) (SQLHSTMT     statementHandle,
                                                  SQLUSMALLINT parameterNumber,
                                                  SQLSMALLINT *dataTypePtr,
                                                  SQLULEN     *parameterSizePtr,
                                                  SQLSMALLINT *decimalDigitsPtr,
                                                  SQLSMALLINT *nullablePtr);
typedef SQLRETURN (STDCALL *tp_SQLDisconnect) (SQLHDBC connectionHandle);
typedef SQLRETURN (STDCALL *tp_SQLDriverConnectW) (SQLHDBC      connectionHandle,
                                                   SQLHWND      windowHandle,
                                                   SQLWCHAR    *inConnectionString,
                                                   SQLSMALLINT  stringLength1,
                                                   SQLWCHAR    *outConnectionString,
                                                   SQLSMALLINT  bufferLength,
                                                   SQLSMALLINT *stringLength2Ptr,
                                                   SQLUSMALLINT driverCompletion);
typedef SQLRETURN (STDCALL *tp_SQLDriversW) (SQLHENV      henv,
                                             SQLUSMALLINT fDirection,
                                             SQLWCHAR     *szDriverDesc,
                                             SQLSMALLINT  cbDriverDescMax,
                                             SQLSMALLINT *pcbDriverDesc,
                                             SQLWCHAR     *szDriverAttributes,
                                             SQLSMALLINT  cbDrvrAttrMax,
                                             SQLSMALLINT *pcbDrvrAttr);
typedef SQLRETURN (STDCALL *tp_SQLEndTran) (SQLSMALLINT handleType,
                                            SQLHANDLE   handle,
                                            SQLSMALLINT completionType);
typedef SQLRETURN (STDCALL *tp_SQLExecute) (SQLHSTMT statementHandle);
typedef SQLRETURN (STDCALL *tp_SQLFetch) (SQLHSTMT statementHandle);
typedef SQLRETURN (STDCALL *tp_SQLFreeHandle) (SQLSMALLINT handleType,
                                               SQLHANDLE   handle);
typedef SQLRETURN (STDCALL *tp_SQLFreeStmt) (SQLHSTMT     statementHandle,
                                             SQLUSMALLINT option);
typedef SQLRETURN (STDCALL *tp_SQLGetConnectAttrW) (SQLHDBC     connectionHandle,
                                                    SQLINTEGER  attribute,
                                                    SQLPOINTER  valuePtr,
                                                    SQLINTEGER  bufferLength,
                                                    SQLINTEGER *stringLengthPtr);
typedef SQLRETURN (STDCALL *tp_SQLGetData) (SQLHSTMT     statementHandle,
                                            SQLUSMALLINT columnNumber,
                                            SQLSMALLINT  targetType,
                                            SQLPOINTER   targetValue,
                                            SQLLEN       bufferLength,
                                            SQLLEN      *strLen_or_Ind);
typedef SQLRETURN (STDCALL *tp_SQLGetDiagRecW) (SQLSMALLINT  handleType,
                                                SQLHANDLE    handle,
                                                SQLSMALLINT  recNumber,
                                                SQLWCHAR    *sqlstate,
                                                SQLINTEGER  *nativeError,
                                                SQLWCHAR    *messageText,
                                                SQLSMALLINT  bufferLength,
                                                SQLSMALLINT *textLength);
typedef SQLRETURN (STDCALL *tp_SQLGetInfoW) (SQLHDBC      connectionHandle,
                                             SQLUSMALLINT infoType,
                                             SQLPOINTER   infoValuePtr,
                                             SQLSMALLINT  bufferLength,
                                             SQLSMALLINT *stringLengthPtr);
typedef SQLRETURN (STDCALL *tp_SQLGetStmtAttrW) (SQLHSTMT    statementHandle,
                                                 SQLINTEGER  attribute,
                                                 SQLPOINTER  value,
                                                 SQLINTEGER  bufferLength,
                                                 SQLINTEGER *stringLength);
typedef SQLRETURN (STDCALL *tp_SQLGetTypeInfoW) (SQLHSTMT    statementHandle,
                                                 SQLSMALLINT dataType);
typedef SQLRETURN (STDCALL *tp_SQLNumParams) (SQLHSTMT     statementHandle,
                                              SQLSMALLINT *parameterCountPtr);
typedef SQLRETURN (STDCALL *tp_SQLNumResultCols) (SQLHSTMT     statementHandle,
                                                  SQLSMALLINT *columnCount);
typedef SQLRETURN (STDCALL *tp_SQLPrepareW) (SQLHSTMT   statementHandle,
                                             SQLWCHAR  *statementText,
                                             SQLINTEGER textLength);
typedef SQLRETURN (STDCALL *tp_SQLSetConnectAttrW) (SQLHDBC    connectionHandle,
                                                    SQLINTEGER attribute,
                                                    SQLPOINTER valuePtr,
                                                    SQLINTEGER stringLength);
typedef SQLRETURN (STDCALL *tp_SQLSetDescFieldW) (SQLHDESC    descriptorHandle,
                                                  SQLSMALLINT recNumber,
                                                  SQLSMALLINT fieldIdentifier,
                                                  SQLPOINTER  value,
                                                  SQLINTEGER  bufferLength);
typedef SQLRETURN (STDCALL *tp_SQLSetEnvAttr) (SQLHENV    environmentHandle,
                                               SQLINTEGER attribute,
                                               SQLPOINTER value,
                                               SQLINTEGER stringLength);

static tp_SQLAllocHandle     ptr_SQLAllocHandle;
static tp_SQLBindCol         ptr_SQLBindCol;
static tp_SQLBindParameter   ptr_SQLBindParameter;
static tp_SQLBrowseConnectW  ptr_SQLBrowseConnectW;
static tp_SQLColAttributeW   ptr_SQLColAttributeW;
static tp_SQLConnectW        ptr_SQLConnectW;
static tp_SQLDataSources     ptr_SQLDataSources;
static tp_SQLDescribeColW    ptr_SQLDescribeColW;
static tp_SQLDescribeParam   ptr_SQLDescribeParam;
static tp_SQLDisconnect      ptr_SQLDisconnect;
static tp_SQLDriverConnectW  ptr_SQLDriverConnectW;
static tp_SQLDriversW        ptr_SQLDriversW;
static tp_SQLEndTran         ptr_SQLEndTran;
static tp_SQLExecute         ptr_SQLExecute;
static tp_SQLFetch           ptr_SQLFetch;
static tp_SQLFreeHandle      ptr_SQLFreeHandle;
static tp_SQLFreeStmt        ptr_SQLFreeStmt;
static tp_SQLGetConnectAttrW ptr_SQLGetConnectAttrW;
static tp_SQLGetData         ptr_SQLGetData;
static tp_SQLGetDiagRecW     ptr_SQLGetDiagRecW;
static tp_SQLGetInfoW        ptr_SQLGetInfoW;
static tp_SQLGetStmtAttrW    ptr_SQLGetStmtAttrW;
static tp_SQLGetTypeInfoW    ptr_SQLGetTypeInfoW;
static tp_SQLNumParams       ptr_SQLNumParams;
static tp_SQLNumResultCols   ptr_SQLNumResultCols;
static tp_SQLPrepareW        ptr_SQLPrepareW;
static tp_SQLSetConnectAttrW ptr_SQLSetConnectAttrW;
static tp_SQLSetDescFieldW   ptr_SQLSetDescFieldW;
static tp_SQLSetEnvAttr      ptr_SQLSetEnvAttr;

#define SQLAllocHandle     ptr_SQLAllocHandle
#define SQLBindCol         ptr_SQLBindCol
#define SQLBindParameter   ptr_SQLBindParameter
#define SQLBrowseConnectW  ptr_SQLBrowseConnectW
#define SQLColAttributeW   ptr_SQLColAttributeW
#define SQLConnectW        ptr_SQLConnectW
#define SQLDataSources     ptr_SQLDataSources
#define SQLDescribeColW    ptr_SQLDescribeColW
#define SQLDescribeParam   ptr_SQLDescribeParam
#define SQLDisconnect      ptr_SQLDisconnect
#define SQLDriverConnectW  ptr_SQLDriverConnectW
#define SQLDriversW        ptr_SQLDriversW
#define SQLEndTran         ptr_SQLEndTran
#define SQLExecute         ptr_SQLExecute
#define SQLFetch           ptr_SQLFetch
#define SQLFreeHandle      ptr_SQLFreeHandle
#define SQLFreeStmt        ptr_SQLFreeStmt
#define SQLGetConnectAttrW ptr_SQLGetConnectAttrW
#define SQLGetData         ptr_SQLGetData
#define SQLGetDiagRecW     ptr_SQLGetDiagRecW
#define SQLGetInfoW        ptr_SQLGetInfoW
#define SQLGetStmtAttrW    ptr_SQLGetStmtAttrW
#define SQLGetTypeInfoW    ptr_SQLGetTypeInfoW
#define SQLNumParams       ptr_SQLNumParams
#define SQLNumResultCols   ptr_SQLNumResultCols
#define SQLPrepareW        ptr_SQLPrepareW
#define SQLSetConnectAttrW ptr_SQLSetConnectAttrW
#define SQLSetDescFieldW   ptr_SQLSetDescFieldW
#define SQLSetEnvAttr      ptr_SQLSetEnvAttr



static boolType setupDll (const char *dllName)

  {
    static void *dbDll = NULL;

  /* setupDll */
    logFunction(printf("setupDll(\"%s\")\n", dllName););
    if (dbDll == NULL) {
      loadBaseDlls();
      dbDll = dllOpen(dllName);
      if (dbDll != NULL) {
        if ((SQLAllocHandle     = (tp_SQLAllocHandle)     dllFunc(dbDll, "SQLAllocHandle"))     == NULL ||
            (SQLBindCol         = (tp_SQLBindCol)         dllFunc(dbDll, "SQLBindCol"))         == NULL ||
            (SQLBindParameter   = (tp_SQLBindParameter)   dllFunc(dbDll, "SQLBindParameter"))   == NULL ||
#ifdef ODBC_DRIVER_FUNCTIONS_NEEDED
            (SQLBrowseConnectW  = (tp_SQLBrowseConnectW)  dllFunc(dbDll, "SQLBrowseConnectW"))  == NULL ||
#else
            (SQLBrowseConnectW  = (tp_SQLBrowseConnectW)  dllFunc(dbDll, "SQLBrowseConnectW"),  FALSE) ||
#endif
            (SQLColAttributeW   = (tp_SQLColAttributeW)   dllFunc(dbDll, "SQLColAttributeW"))   == NULL ||
            (SQLConnectW        = (tp_SQLConnectW)        dllFunc(dbDll, "SQLConnectW"))        == NULL ||
#ifdef ODBC_DRIVER_FUNCTIONS_NEEDED
            (SQLDataSources     = (tp_SQLDataSources)     dllFunc(dbDll, "SQLDataSources"))     == NULL ||
#else
            (SQLDataSources     = (tp_SQLDataSources)     NULL, FALSE) ||
#endif
            (SQLDescribeColW    = (tp_SQLDescribeColW)    dllFunc(dbDll, "SQLDescribeColW"))    == NULL ||
            (SQLDescribeParam   = (tp_SQLDescribeParam)   dllFunc(dbDll, "SQLDescribeParam"),   FALSE) ||
            (SQLDisconnect      = (tp_SQLDisconnect)      dllFunc(dbDll, "SQLDisconnect"))      == NULL ||
            (SQLDriverConnectW  = (tp_SQLDriverConnectW)  dllFunc(dbDll, "SQLDriverConnectW"))  == NULL ||
#ifdef ODBC_DRIVER_FUNCTIONS_NEEDED
            (SQLDriversW        = (tp_SQLDriversW)        dllFunc(dbDll, "SQLDriversW"))        == NULL ||
#else
            (SQLDriversW        = (tp_SQLDriversW)        NULL, FALSE) ||
#endif
            (SQLEndTran         = (tp_SQLEndTran)         dllFunc(dbDll, "SQLEndTran"))         == NULL ||
            (SQLExecute         = (tp_SQLExecute)         dllFunc(dbDll, "SQLExecute"))         == NULL ||
            (SQLFetch           = (tp_SQLFetch)           dllFunc(dbDll, "SQLFetch"))           == NULL ||
            (SQLFreeHandle      = (tp_SQLFreeHandle)      dllFunc(dbDll, "SQLFreeHandle"))      == NULL ||
            (SQLFreeStmt        = (tp_SQLFreeStmt)        dllFunc(dbDll, "SQLFreeStmt"))        == NULL ||
            (SQLGetConnectAttrW = (tp_SQLGetConnectAttrW) dllFunc(dbDll, "SQLGetConnectAttrW")) == NULL ||
            (SQLGetData         = (tp_SQLGetData)         dllFunc(dbDll, "SQLGetData"))         == NULL ||
            (SQLGetDiagRecW     = (tp_SQLGetDiagRecW)     dllFunc(dbDll, "SQLGetDiagRecW"))     == NULL ||
            (SQLGetInfoW        = (tp_SQLGetInfoW)        dllFunc(dbDll, "SQLGetInfoW"))        == NULL ||
            (SQLGetStmtAttrW    = (tp_SQLGetStmtAttrW)    dllFunc(dbDll, "SQLGetStmtAttrW"))    == NULL ||
            (SQLGetTypeInfoW    = (tp_SQLGetTypeInfoW)    dllFunc(dbDll, "SQLGetTypeInfoW"))    == NULL ||
            (SQLNumParams       = (tp_SQLNumParams )      dllFunc(dbDll, "SQLNumParams"))       == NULL ||
            (SQLNumResultCols   = (tp_SQLNumResultCols)   dllFunc(dbDll, "SQLNumResultCols"))   == NULL ||
            (SQLPrepareW        = (tp_SQLPrepareW)        dllFunc(dbDll, "SQLPrepareW"))        == NULL ||
            (SQLSetConnectAttrW = (tp_SQLSetConnectAttrW) dllFunc(dbDll, "SQLSetConnectAttrW")) == NULL ||
            (SQLSetDescFieldW   = (tp_SQLSetDescFieldW)   dllFunc(dbDll, "SQLSetDescFieldW"))   == NULL ||
            (SQLSetEnvAttr      = (tp_SQLSetEnvAttr)      dllFunc(dbDll, "SQLSetEnvAttr"))      == NULL) {
          dbDll = NULL;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setupDll --> %d\n", dbDll != NULL););
    return dbDll != NULL;
  } /* setupDll */



static boolType findDll (void)

  {
    const char *dllList[] = { CLI_DLL };
    unsigned int pos;
    boolType found = FALSE;

  /* findDll */
    for (pos = 0; pos < sizeof(dllList) / sizeof(char *) && !found; pos++) {
      found = setupDll(dllList[pos]);
    } /* for */
    if (!found) {
      dllErrorMessage("sqlOpen", "findDll", dllList,
                      sizeof(dllList) / sizeof(char *));
    } /* if */
    return found;
  } /* findDll */

#else

#define findDll() TRUE
#define FUNCTION_PRESENT(func) TRUE

#endif



static void sqlClose (databaseType database);



/**
 *  Copy a SQLWCHAR string with length to a null terminated C string.
 *  This function is used to convert (ASCII) date and time values.
 *  @param cstri Destination of the null terminated string.
 *  @param wstri Source SQLWCHAR string to be copied.
 *  @param length Length of wstri measured in wide characters.
 *  @return OKAY_NO_ERROR if the conversion succeeded, or
 *          RANGE_ERROR if non-ASCII characters were found.
 */
static errInfoType conv_sqlwstri_to_cstri (cstriType cstri, const SQLWCHAR *wstri,
    memSizeType length)

  {
    ustriType ustri;
    SQLWCHAR ch;
    errInfoType err_info = OKAY_NO_ERROR;

  /* conv_sqlwstri_to_cstri */
    ustri = (ustriType) cstri;
    for (; length > 0; ustri++, wstri++, length--) {
      ch = *wstri;
      if (likely(ch <= 0xFF)) {
        *ustri = (ucharType) ch;
      } else {
        err_info = RANGE_ERROR;
      } /* if */
    } /* if */
    *ustri = '\0';
    return err_info;
  } /* conv_sqlwstri_to_cstri */



static void memcpy_to_sqlwstri (SQLWCHAR *dest, const char *src, memSizeType len)

  { /* memcpy_to_sqlwstri */
    for (; len > 0; src++, dest++, len--) {
      *dest = (SQLWCHAR) *src;
    } /* for */
  } /* memcpy_to_sqlwstri */



static void wstri_to_cstri8 (ustriType cstri8, const SQLWCHAR *wstri)

  {
    strElemType ch1;
    SQLWCHAR ch2;

  /* wstri_to_cstri8 */
    for (; (ch1 = (strElemType) *wstri) != 0; wstri++) {
      if (unlikely(ch1 >= 0xD800 && ch1 <= 0xDBFF)) {
        ch2 = wstri[1];
        if (likely(ch2 >= 0xDC00 && ch2 <= 0xDFFF)) {
          ch1 = ((((strElemType) ch1 - 0xD800) << 10) +
                  ((strElemType) ch2 - 0xDC00) + 0x10000);
          wstri++;
        } /* if */
      } /* if */
      if (likely(ch1 != 0)) {
        if (ch1 <= 0x7F) {
          *cstri8++ = (ucharType) ch1;
        } else if (ch1 <= 0x7FF) {
          cstri8[0] = (ucharType) (0xC0 | (ch1 >>  6));
          cstri8[1] = (ucharType) (0x80 |( ch1        & 0x3F));
          cstri8 += 2;
        } else if (ch1 <= 0xFFFF) {
          cstri8[0] = (ucharType) (0xE0 | (ch1 >> 12));
          cstri8[1] = (ucharType) (0x80 |((ch1 >>  6) & 0x3F));
          cstri8[2] = (ucharType) (0x80 |( ch1        & 0x3F));
          cstri8 += 3;
        } else {
          cstri8[0] = (ucharType) (0xF0 | (ch1 >> 18));
          cstri8[1] = (ucharType) (0x80 |((ch1 >> 12) & 0x3F));
          cstri8[2] = (ucharType) (0x80 |((ch1 >>  6) & 0x3F));
          cstri8[3] = (ucharType) (0x80 |( ch1        & 0x3F));
          cstri8 += 4;
        } /* if */
      } /* if */
    } /* for */
    *cstri8 = '\0';
  } /* wstri_to_cstri8 */



static void setDbErrorMsg (const char *funcName, const char *dbFuncName,
    SQLSMALLINT handleType, SQLHANDLE handle)

  {
    SQLRETURN returnCode;
    SQLWCHAR sqlState[5 + NULL_TERMINATION_LEN];
    ucharType sqlState8[5 * MAX_WSTRI_TO_UTF8_EXPANSION_FACTOR + NULL_TERMINATION_LEN];
    SQLWCHAR messageText[ERROR_MESSAGE_BUFFER_SIZE];
    ucharType messageText8[ERROR_MESSAGE_BUFFER_SIZE * MAX_WSTRI_TO_UTF8_EXPANSION_FACTOR];
    SQLINTEGER nativeError = 0;
    SQLSMALLINT bufferLength;

  /* setDbErrorMsg */
    dbError.funcName = funcName;
    dbError.dbFuncName = dbFuncName;
    memset(sqlState, 0, sizeof(sqlState));
    memset(messageText, 0, sizeof(messageText));
    returnCode = SQLGetDiagRecW(handleType,
                                handle,
                                1,
                                sqlState,
                                &nativeError,
                                messageText,
                                (SQLSMALLINT) ERROR_MESSAGE_BUFFER_SIZE,
                                &bufferLength);
    if (returnCode == SQL_NO_DATA) {
      dbError.errorCode = 0;
      snprintf(dbError.message, DB_ERR_MESSAGE_SIZE,
               " *** SQLGetDiagRecW returned: SQL_NO_DATA");
    } else if (returnCode != SQL_SUCCESS &&
               returnCode != SQL_SUCCESS_WITH_INFO) {
      dbError.errorCode = 0;
      snprintf(dbError.message, DB_ERR_MESSAGE_SIZE,
               " *** SQLGetDiagRecW returned: %d\n", returnCode);
    } else {
      dbError.errorCode = (intType) nativeError;
      wstri_to_cstri8(sqlState8, sqlState);
      wstri_to_cstri8(messageText8, messageText);
      snprintf(dbError.message, DB_ERR_MESSAGE_SIZE,
               "%s\nSQLState: %s\nNativeError: %ld\n",
               messageText8, sqlState8, (long int) nativeError);
    } /* if */
  } /* setDbErrorMsg */



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
    FREE_RECORD2(db, dbRecordCli, count.database, count.database_bytes);
    logFunction(printf("freeDatabase -->\n"););
  } /* freeDatabase */



static void freeFetchData (preparedStmtType preparedStmt, fetchDataType fetchData)

  {
    memSizeType pos;

  /* freeFetchData */
    if (fetchData->result_array != NULL) {
      for (pos = 0; pos < preparedStmt->result_array_size; pos++) {
        free(fetchData->result_array[pos].buffer);
      } /* for */
      FREE_TABLE(fetchData->result_array, resultDataRecordCli, preparedStmt->result_array_size);
    } /* if */
  } /* freeFetchData */



static void freeFetch (preparedStmtType preparedStmt, fetchDataType fetchData)

  { /* freeFetch */
    freeFetchData(preparedStmt, fetchData);
    FREE_RECORD2(fetchData, fetchDataRecord,
                 count.fetch_data, count.fetch_data_bytes);
  } /* freeFetch */



static void freePrefetched (preparedStmtType preparedStmt)

  {
    fetchDataType prefetched;
    fetchDataType oldFetchData;

  /* freePrefetched */
    prefetched = preparedStmt->prefetched;
    while (prefetched != NULL) {
      oldFetchData = prefetched;
      prefetched = prefetched->next;
      freeFetch(preparedStmt, oldFetchData);
    } /* while */
    preparedStmt->prefetched = NULL;
  } /* freePrefetched */



/**
 *  Closes a prepared statement and frees the memory used by it.
 */
static void freePreparedStmt (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    memSizeType pos;

  /* freePreparedStmt */
    logFunction(printf("freePreparedStmt(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (preparedStmt->param_array != NULL) {
      for (pos = 0; pos < preparedStmt->param_array_size; pos++) {
        free(preparedStmt->param_array[pos].buffer);
      } /* for */
      FREE_TABLE(preparedStmt->param_array, bindDataRecordCli, preparedStmt->param_array_size);
    } /* if */
    if (preparedStmt->result_descr_array != NULL) {
      FREE_TABLE(preparedStmt->result_descr_array, resultDataRecordCli, preparedStmt->result_array_size);
    } /* if */
    freePrefetched(preparedStmt);
    freeFetchData(preparedStmt, &preparedStmt->fetchRecord);
    if (preparedStmt->db->connection != SQL_NULL_HANDLE) {
      if (preparedStmt->executeSuccessful) {
        /* Ignore possible errors. */
        SQLFreeStmt(preparedStmt->ppStmt, SQL_CLOSE);
      } /* if */
      SQLFreeHandle(SQL_HANDLE_STMT, preparedStmt->ppStmt);
    } /* if */
    if (preparedStmt->db->usage_count != 0) {
      preparedStmt->db->usage_count--;
      if (preparedStmt->db->usage_count == 0) {
        logMessage(printf("FREE " FMT_U_MEM "\n", (memSizeType) preparedStmt->db););
        freeDatabase((databaseType) preparedStmt->db);
      } /* if */
    } /* if */
    FREE_RECORD2(preparedStmt, preparedStmtRecordCli,
                 count.prepared_stmt, count.prepared_stmt_bytes);
    logFunction(printf("freePreparedStmt -->\n"););
  } /* freePreparedStmt */



#if ANY_LOG_ACTIVE
static const char *nameOfSqlType (SQLSMALLINT sql_type)

  {
    static char buffer[INT_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    const char *typeName;

  /* nameOfSqlType */
    logFunction(printf("nameOfSqlType(%d)\n", sql_type););
    switch (sql_type) {
      case SQL_CHAR:                      typeName = "SQL_CHAR"; break;
      case SQL_VARCHAR:                   typeName = "SQL_VARCHAR"; break;
      case SQL_LONGVARCHAR:               typeName = "SQL_LONGVARCHAR"; break;
      case SQL_WCHAR:                     typeName = "SQL_WCHAR"; break;
      case SQL_WVARCHAR:                  typeName = "SQL_WVARCHAR"; break;
      case SQL_WLONGVARCHAR:              typeName = "SQL_WLONGVARCHAR"; break;
      case SQL_BIT:                       typeName = "SQL_BIT"; break;
      case SQL_TINYINT:                   typeName = "SQL_TINYINT"; break;
      case SQL_SMALLINT:                  typeName = "SQL_SMALLINT"; break;
      case SQL_INTEGER:                   typeName = "SQL_INTEGER"; break;
      case SQL_BIGINT:                    typeName = "SQL_BIGINT"; break;
      case SQL_DECIMAL:                   typeName = "SQL_DECIMAL"; break;
      case SQL_NUMERIC:                   typeName = "SQL_NUMERIC"; break;
      case SQL_REAL:                      typeName = "SQL_REAL"; break;
      case SQL_FLOAT:                     typeName = "SQL_FLOAT"; break;
      case SQL_DOUBLE:                    typeName = "SQL_DOUBLE"; break;
      case SQL_TYPE_DATE:                 typeName = "SQL_TYPE_DATE"; break;
      case SQL_TYPE_TIME:                 typeName = "SQL_TYPE_TIME"; break;
      case SQL_DATETIME:                  typeName = "SQL_DATETIME"; break;
      case SQL_TYPE_TIMESTAMP:            typeName = "SQL_TYPE_TIMESTAMP"; break;
      case SQL_INTERVAL_YEAR:             typeName = "SQL_INTERVAL_YEAR"; break;
      case SQL_INTERVAL_MONTH:            typeName = "SQL_INTERVAL_MONTH"; break;
      case SQL_INTERVAL_DAY:              typeName = "SQL_INTERVAL_DAY"; break;
      case SQL_INTERVAL_HOUR:             typeName = "SQL_INTERVAL_HOUR"; break;
      case SQL_INTERVAL_MINUTE:           typeName = "SQL_INTERVAL_MINUTE"; break;
      case SQL_INTERVAL_SECOND:           typeName = "SQL_INTERVAL_SECOND"; break;
      case SQL_INTERVAL_YEAR_TO_MONTH:    typeName = "SQL_INTERVAL_YEAR_TO_MONTH"; break;
      case SQL_INTERVAL_DAY_TO_HOUR:      typeName = "SQL_INTERVAL_DAY_TO_HOUR"; break;
      case SQL_INTERVAL_DAY_TO_MINUTE:    typeName = "SQL_INTERVAL_DAY_TO_MINUTE"; break;
      case SQL_INTERVAL_DAY_TO_SECOND:    typeName = "SQL_INTERVAL_DAY_TO_SECOND"; break;
      case SQL_INTERVAL_HOUR_TO_MINUTE:   typeName = "SQL_INTERVAL_HOUR_TO_MINUTE"; break;
      case SQL_INTERVAL_HOUR_TO_SECOND:   typeName = "SQL_INTERVAL_HOUR_TO_SECOND"; break;
      case SQL_INTERVAL_MINUTE_TO_SECOND: typeName = "SQL_INTERVAL_MINUTE_TO_SECOND"; break;
      case SQL_BINARY:                    typeName = "SQL_BINARY"; break;
      case SQL_VARBINARY:                 typeName = "SQL_VARBINARY"; break;
      case SQL_LONGVARBINARY:             typeName = "SQL_LONGVARBINARY"; break;
      case SQL_BLOB:                      typeName = "SQL_BLOB"; break;
      case SQL_CLOB:                      typeName = "SQL_CLOB"; break;
      case SQL_INFX_UDT_VARYING:          typeName = "SQL_INFX_UDT_VARYING"; break;
      case SQL_SS_TIME2:                  typeName = "SQL_SS_TIME2"; break;
      case SQL_XML:                       typeName = "SQL_XML"; break;
      default:
        sprintf(buffer, FMT_D16, sql_type);
        typeName = buffer;
        break;
    } /* switch */
    logFunction(printf("nameOfSqlType --> %s\n", typeName););
    return typeName;
  } /* nameOfSqlType */



static const char *nameOfCType (int c_type)

  {
    static char buffer[INT_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    const char *typeName;

  /* nameOfCType */
    switch (c_type) {
      case SQL_C_CHAR:                      typeName = "SQL_C_CHAR"; break;
      case SQL_C_WCHAR:                     typeName = "SQL_C_WCHAR"; break;
      case SQL_C_BIT:                       typeName = "SQL_C_BIT"; break;
      case SQL_C_STINYINT:                  typeName = "SQL_C_STINYINT"; break;
      case SQL_C_UTINYINT:                  typeName = "SQL_C_UTINYINT"; break;
      case SQL_C_SSHORT:                    typeName = "SQL_C_SSHORT"; break;
      case SQL_C_SLONG:                     typeName = "SQL_C_SLONG"; break;
      case SQL_C_SBIGINT:                   typeName = "SQL_C_SBIGINT"; break;
      case SQL_C_NUMERIC:                   typeName = "SQL_C_NUMERIC"; break;
      case SQL_C_FLOAT:                     typeName = "SQL_C_FLOAT"; break;
      case SQL_C_DOUBLE:                    typeName = "SQL_C_DOUBLE"; break;
      case SQL_C_TYPE_DATE:                 typeName = "SQL_C_TYPE_DATE"; break;
      case SQL_C_TYPE_TIME:                 typeName = "SQL_C_TYPE_TIME"; break;
      case SQL_C_TYPE_TIMESTAMP:            typeName = "SQL_C_TYPE_TIMESTAMP"; break;
      case SQL_C_INTERVAL_YEAR:             typeName = "SQL_C_INTERVAL_YEAR"; break;
      case SQL_C_INTERVAL_MONTH:            typeName = "SQL_C_INTERVAL_MONTH"; break;
      case SQL_C_INTERVAL_DAY:              typeName = "SQL_C_INTERVAL_DAY"; break;
      case SQL_C_INTERVAL_HOUR:             typeName = "SQL_C_INTERVAL_HOUR"; break;
      case SQL_C_INTERVAL_MINUTE:           typeName = "SQL_C_INTERVAL_MINUTE"; break;
      case SQL_C_INTERVAL_SECOND:           typeName = "SQL_C_INTERVAL_SECOND"; break;
      case SQL_C_INTERVAL_YEAR_TO_MONTH:    typeName = "SQL_C_INTERVAL_YEAR_TO_MONTH"; break;
      case SQL_C_INTERVAL_DAY_TO_HOUR:      typeName = "SQL_C_INTERVAL_DAY_TO_HOUR"; break;
      case SQL_C_INTERVAL_DAY_TO_MINUTE:    typeName = "SQL_C_INTERVAL_DAY_TO_MINUTE"; break;
      case SQL_C_INTERVAL_DAY_TO_SECOND:    typeName = "SQL_C_INTERVAL_DAY_TO_SECOND"; break;
      case SQL_C_INTERVAL_HOUR_TO_MINUTE:   typeName = "SQL_C_INTERVAL_HOUR_TO_MINUTE"; break;
      case SQL_C_INTERVAL_HOUR_TO_SECOND:   typeName = "SQL_C_INTERVAL_HOUR_TO_SECOND"; break;
      case SQL_C_INTERVAL_MINUTE_TO_SECOND: typeName = "SQL_C_INTERVAL_MINUTE_TO_SECOND"; break;
      case SQL_C_BINARY:                    typeName = "SQL_C_BINARY"; break;
      default:
        sprintf(buffer, "%d", c_type);
        typeName = buffer;
        break;
    } /* switch */
    return typeName;
  } /* nameOfCType */
#endif



/**
 *  Remove comments from the statement string.
 *  This avoids problems with some ODBC drivers.
 *  Some ODBC drivers do not remove comments so question marks (?)
 *  or quotes (') in comments are misinterpreted.
 *  String literals are scanned also to avoid that a comment
 *  inside a literal is removed.
 */
static striType processStatementStri (const const_striType sqlStatementStri,
    errInfoType *err_info)

  {
    memSizeType pos = 0;
    strElemType ch;
    strElemType delimiter;
    memSizeType destPos = 0;
    striType processed;

  /* processStatementStri */
    logFunction(printf("processStatementStri(\"%s\")\n",
                       striAsUnquotedCStri(sqlStatementStri)););
    if (unlikely(sqlStatementStri->size > MAX_STRI_LEN ||
                 !ALLOC_STRI_SIZE_OK(processed, sqlStatementStri->size))) {
      *err_info = MEMORY_ERROR;
      processed = NULL;
    } else {
      while (pos < sqlStatementStri->size && *err_info == OKAY_NO_ERROR) {
        ch = sqlStatementStri->mem[pos];
        if (ch == '\'' || ch == '"') {
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



#ifndef WIDE_CHARS_SUPPORTED
static boolType hasDataType (SQLHDBC connection, SQLSMALLINT requestedDataType,
    errInfoType *err_info)

  {
    SQLHSTMT ppStmt;
    SQLRETURN returnCode;
    boolType hasType = FALSE;

  /* hasDataType */
    logFunction(printf("hasDataType(*, " FMT_D16 ", *)\n", requestedDataType););
    if (SQLAllocHandle(SQL_HANDLE_STMT,
                       connection,
                       &ppStmt) != SQL_SUCCESS) {
      setDbErrorMsg("hasDataType", "SQLAllocHandle",
                    SQL_HANDLE_DBC, connection);
      logError(printf("hasDataType: SQLAllocHandle SQL_HANDLE_STMT:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
    } else {
      returnCode = SQLGetTypeInfoW(ppStmt, requestedDataType);
      /* printf("returnCode: " FMT_D16 "\n", returnCode); */
      if (returnCode == SQL_SUCCESS) {
        if (SQLFetch(ppStmt) == SQL_SUCCESS) {
          hasType = TRUE;
        } /* if */
      } /* if */
      SQLFreeHandle(SQL_HANDLE_STMT, ppStmt);
    } /* if */
    logFunction(printf("hasDataType --> %d\n", hasType););
    return hasType;
  } /* hasDataType */
#endif



#ifndef TINY_INT_IS_UNSIGNED
static boolType dataTypeIsUnsigned (SQLHDBC connection, SQLSMALLINT requestedDataType,
    errInfoType *err_info)

  {
    SQLHSTMT ppStmt;
    SQLRETURN returnCode;
    SQLSMALLINT unsignedAttribute;
    SQLLEN unsignedAttribute_ind;
    SQLRETURN fetchResult;
    boolType isUnsigned = FALSE;

  /* dataTypeIsUnsigned */
    logFunction(printf("dataTypeIsUnsigned(*, " FMT_D16 ", *)\n", requestedDataType););
    if (SQLAllocHandle(SQL_HANDLE_STMT,
                       connection,
                       &ppStmt) != SQL_SUCCESS) {
      setDbErrorMsg("dataTypeIsUnsigned", "SQLAllocHandle",
                    SQL_HANDLE_DBC, connection);
      logError(printf("dataTypeIsUnsigned: SQLAllocHandle SQL_HANDLE_STMT:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
    } else {
      returnCode = SQLGetTypeInfoW(ppStmt, requestedDataType);
      /* printf("returnCode: " FMT_D16 "\n", returnCode); */
      if (returnCode != SQL_SUCCESS) {
        setDbErrorMsg("dataTypeIsUnsigned", "SQLGetTypeInfoW",
                      SQL_HANDLE_DBC, connection);
        logError(printf("dataTypeIsUnsigned: SQLGetTypeInfoW error:\n%s\n",
                        dbError.message););
        *err_info = DATABASE_ERROR;
      } else if (SQLBindCol(ppStmt,
                            10,
                            SQL_C_SHORT,
                            (SQLPOINTER) &unsignedAttribute,
                            (SQLLEN) sizeof(unsignedAttribute),
                            &unsignedAttribute_ind) != SQL_SUCCESS) {
        setDbErrorMsg("dataTypeIsUnsigned", "SQLBindCol",
                      SQL_HANDLE_DBC, connection);
        logError(printf("dataTypeIsUnsigned: SQLBindCol error:\n%s\n",
                        dbError.message););
        *err_info = DATABASE_ERROR;
      } else {
        fetchResult = SQLFetch(ppStmt);
        if (fetchResult == SQL_SUCCESS) {
          if (unsignedAttribute_ind == SQL_NULL_DATA) {
            logError(printf("dataTypeIsUnsigned: UNSIGNED_ATTRIBUTE is NULL.\n"););
            *err_info = RANGE_ERROR;
          } else {
            isUnsigned = unsignedAttribute;
          } /* if */
        } else if (fetchResult == SQL_NO_DATA) {
          /* No data found. Probably the dataType is not used at all. */
          /* Assume that the dataType is signed. */
        } else {
          setDbErrorMsg("dataTypeIsUnsigned", "SQLFetch",
                        SQL_HANDLE_DBC, connection);
          logError(printf("dataTypeIsUnsigned: SQLFetch error:\n%s\n",
                          dbError.message););
          *err_info = DATABASE_ERROR;
        } /* if */
      } /* if */
      SQLFreeHandle(SQL_HANDLE_STMT, ppStmt);
    } /* if */
    logFunction(printf("dataTypeIsUnsigned --> %d\n", isUnsigned););
    return isUnsigned;
  } /* dataTypeIsUnsigned */
#endif



static errInfoType setupParameterColumn (preparedStmtType preparedStmt,
    SQLUSMALLINT param_index, bindDataType param)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupParameterColumn */
    logFunction(printf("setupParameterColumn(*, " FMT_U16 ", *)\n",
                       param_index););
    if (!FUNCTION_PRESENT(SQLDescribeParam) ||
        SQLDescribeParam(preparedStmt->ppStmt,
                         (SQLUSMALLINT) (param_index + 1),
                         &param->dataType,
                         &param->paramSize,
                         &param->decimalDigits,
                         &param->nullable) != SQL_SUCCESS) {
      /* The function SQLDescribeParam() of the MySQL driver for */
      /* Unixodbc returns the values below. These are reasonable */
      /* defaults, if the ODBC driver does not support the       */
      /* function SQLDescribeParam().                            */
      /* printf("FUNCTION_PRESENT(SQLDescribeParam): %d\n",
         FUNCTION_PRESENT(SQLDescribeParam)); */
      param->dataType = SQL_VARCHAR;
      param->paramSize = 255;
      param->decimalDigits = 0;
      param->nullable = 1;
    } /* if */
    if (likely(err_info == OKAY_NO_ERROR)) {
      /* printf("parameter: " FMT_U16 ", dataType: " FMT_D16 ", paramSize: " FMT_U_MEM
          ", decimalDigits: " FMT_D16 "\n", param_index + 1,
          param->dataType, param->paramSize, param->decimalDigits); */
      switch (param->dataType) {
        case SQL_BIT:
          param->buffer_length = sizeof(char);
          break;
        case SQL_TINYINT:
          param->buffer_length = sizeof(int8Type);
          break;
        case SQL_SMALLINT:
          param->buffer_length = sizeof(int16Type);
          break;
        case SQL_INTEGER:
          param->buffer_length = sizeof(int32Type);
          break;
        case SQL_BIGINT:
          param->buffer_length = sizeof(int64Type);
          break;
        case SQL_REAL:
          param->buffer_length = sizeof(float);
          break;
        case SQL_FLOAT:
        case SQL_DOUBLE:
          param->buffer_length = sizeof(double);
          break;
        case SQL_TYPE_DATE:
          param->buffer_length = sizeof(SQL_DATE_STRUCT);
          break;
        case SQL_TYPE_TIME:
          param->buffer_length = sizeof(SQL_TIME_STRUCT);
          break;
        case SQL_DATETIME:
        case SQL_TYPE_TIMESTAMP:
          param->buffer_length = sizeof(SQL_TIMESTAMP_STRUCT);
          break;
        case SQL_DECIMAL:
        case SQL_NUMERIC:
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:
        case SQL_BINARY:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY:
        case SQL_BLOB:
        case SQL_CLOB:
          /* For these data types no buffer is reserved. Therefore  */
          /* param->buffer is NULL and param->buffer_capacity is 0. */
          /* All bind functions dealing with these data types check */
          /* the buffer_capacity and (re)allocate the buffer, if    */
          /* necessary.                                             */
          break;
      } /* switch */
      if (param->buffer_length != 0) {
        param->buffer = malloc(param->buffer_length);
        if (unlikely(param->buffer == NULL)) {
          param->buffer_length = 0;
          err_info = MEMORY_ERROR;
        } else {
          param->buffer_capacity = param->buffer_length;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setupParameterColumn --> %d\n", err_info););
    return err_info;
  } /* setupParameterColumn */



static errInfoType setupParameters (preparedStmtType preparedStmt)

  {
    SQLSMALLINT num_params;
    SQLUSMALLINT param_index;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupParameters */
    logFunction(printf("setupParameters\n"););
    if (SQLNumParams(preparedStmt->ppStmt,
                     &num_params) != SQL_SUCCESS) {
      setDbErrorMsg("setupParameters", "SQLNumParams",
                    SQL_HANDLE_STMT, preparedStmt->ppStmt);
      logError(printf("setupParameters: SQLNumParams:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } else if (unlikely(num_params < 0)) {
      dbInconsistent("setupParameters", "SQLNumParams");
      logError(printf("setupParameters: SQLNumParams returns negative number: %hd\n",
                      num_params););
      err_info = DATABASE_ERROR;
    } else if (num_params == 0) {
      /* malloc(0) may return NULL, which would wrongly trigger a MEMORY_ERROR. */
      preparedStmt->param_array_size = 0;
      preparedStmt->param_array = NULL;
    } else if (unlikely(!ALLOC_TABLE(preparedStmt->param_array,
                                     bindDataRecordCli, (memSizeType) num_params))) {
      err_info = MEMORY_ERROR;
    } else {
      preparedStmt->param_array_size = (memSizeType) num_params;
      memset(preparedStmt->param_array, 0,
             (memSizeType) num_params * sizeof(bindDataRecordCli));
      for (param_index = 0; param_index < num_params &&
           err_info == OKAY_NO_ERROR; param_index++) {
        err_info = setupParameterColumn(preparedStmt, param_index,
            &preparedStmt->param_array[param_index]);
      } /* for */
    } /* if */
    logFunction(printf("setupParameters --> %d\n", err_info););
    return err_info;
  } /* setupParameters */



#if DECODE_NUMERIC_STRUCT
/**
 * Set precision and scale of SQL_C_NUMERIC data.
 * Otherwise the driver defined default precision and scale would be used.
 * This function is used for bound and unbound SQL_C_NUMERIC data.
 * If the function is used for bound data the data pointer
 * (SQL_DESC_DATA_PTR) must be set afterwards, because according
 * to the documentation of SQLSetDescFieldW changes of the
 * attributes set SQL_DESC_DATA_PTR to a NULL pointer.
 * If the function is used for unbound data setting the type is
 * necessary and setting the data pointer is omitted.
 */
static errInfoType setNumericPrecisionAndScale (preparedStmtType preparedStmt,
    SQLSMALLINT column_num, resultDescrType columnDescr,
    resultDataType columnData, boolType withBinding)
  {
    SQLHDESC descriptorHandle;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setNumericPrecisionAndScale */
    /* printf("SQL_C_NUMERIC:\n");
    printf("columnSize: " FMT_U_MEM "\n", columnDescr->columnSize);
    printf("decimalDigits:" FMT_D16 "\n", columnDescr->decimalDigits);
    printf("buffer_length: " FMT_U_MEM "\n", columnDescr->buffer_length); */
    if (SQLGetStmtAttrW(preparedStmt->ppStmt,
                        SQL_ATTR_APP_ROW_DESC,
                        &descriptorHandle,
                        0, NULL) != SQL_SUCCESS) {
      setDbErrorMsg("setNumericPrecisionAndScale", "SQLGetStmtAttrW",
                    SQL_HANDLE_STMT, preparedStmt->ppStmt);
      logError(printf("setNumericPrecisionAndScale: SQLGetStmtAttrW "
                      "SQL_ATTR_APP_ROW_DESC:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } else if (!withBinding &&
               SQLSetDescFieldW(descriptorHandle,
                                column_num,
                                SQL_DESC_TYPE,
                                (SQLPOINTER) SQL_C_NUMERIC,
                                0) != SQL_SUCCESS) {
      setDbErrorMsg("setNumericPrecisionAndScale", "SQLSetDescFieldW",
                    SQL_HANDLE_DESC, descriptorHandle);
      logError(printf("setNumericPrecisionAndScale: SQLSetDescFieldW "
                      "SQL_DESC_TYPE:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } else if (SQLSetDescFieldW(descriptorHandle,
                                column_num,
                                SQL_DESC_PRECISION,
                                (SQLPOINTER) columnDescr->columnSize,
                                0) != SQL_SUCCESS) {
      setDbErrorMsg("setNumericPrecisionAndScale", "SQLSetDescFieldW",
                    SQL_HANDLE_DESC, descriptorHandle);
      logError(printf("setNumericPrecisionAndScale: SQLSetDescFieldW "
                      "SQL_DESC_PRECISION:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } else if (SQLSetDescFieldW(descriptorHandle,
                                column_num,
                                SQL_DESC_SCALE,
                                (SQLPOINTER) (memSizeType) columnDescr->decimalDigits,
                                0) != SQL_SUCCESS) {
      setDbErrorMsg("setNumericPrecisionAndScale", "SQLSetDescFieldW",
                    SQL_HANDLE_DESC, descriptorHandle);
      logError(printf("setNumericPrecisionAndScale: SQLSetDescFieldW "
                      "SQL_DESC_SCALE:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } else if (withBinding &&
               SQLSetDescFieldW(descriptorHandle,
                                column_num,
                                SQL_DESC_DATA_PTR,
                                columnData->buffer,
                                0) != SQL_SUCCESS) {
      setDbErrorMsg("setNumericPrecisionAndScale", "SQLSetDescFieldW",
                    SQL_HANDLE_DESC, descriptorHandle);
      logError(printf("setNumericPrecisionAndScale: SQLSetDescFieldW "
                      "SQL_DESC_DATA_PTR:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } /* if */
    return err_info;
  } /* setNumericPrecisionAndScale */
#endif



static errInfoType setupResultColumn (preparedStmtType preparedStmt,
    SQLSMALLINT column_num, resultDescrType columnDescr)

  {
    SQLRETURN returnCode;
    SQLSMALLINT nameLength;
    SQLSMALLINT c_type;
    memSizeType buffer_length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupResultColumn */
    logFunction(printf("setupResultColumn: column_num=%d\n", column_num););
    returnCode = SQLDescribeColW(preparedStmt->ppStmt,
                                 (SQLUSMALLINT) column_num,
                                 NULL,
                                 0,
                                 &nameLength,
                                 &columnDescr->dataType,
                                 &columnDescr->columnSize,
                                 &columnDescr->decimalDigits,
                                 &columnDescr->nullable);
    if (unlikely(returnCode != SQL_SUCCESS &&
                 returnCode != SQL_SUCCESS_WITH_INFO)) {
      setDbErrorMsg("setupResultColumn", "SQLDescribeColW",
                    SQL_HANDLE_STMT, preparedStmt->ppStmt);
      logError(printf("setupResultColumn: SQLDescribeColW returns "
                      FMT_D16 ":\n%s\n", returnCode, dbError.message););
      err_info = DATABASE_ERROR;
    } else {
      logMessage(printf("dataType: %s\n", nameOfSqlType(columnDescr->dataType)););
      switch (columnDescr->dataType) {
        case SQL_CHAR:
        case SQL_VARCHAR:
          if (preparedStmt->db->wideCharsSupported) {
            c_type = SQL_C_WCHAR;
          } else {
            c_type = SQL_C_CHAR;
          } /* if */
          if (unlikely(columnDescr->columnSize > (MAX_MEMSIZETYPE / 2) - 1)) {
            logError(printf("setupResultColumn: ColumnSize too big: " FMT_U_LEN "\n",
                            columnDescr->columnSize););
            err_info = MEMORY_ERROR;
          } else {
            buffer_length = ((memSizeType) columnDescr->columnSize + 1) * 2;
          } /* if */
          logMessage(printf("%s:\n",
                            nameOfSqlType(columnDescr->dataType));
                     printf("columnSize: " FMT_U_MEM "\n",
                            columnDescr->columnSize);
                     printf("SQLLEN_MAX: %ld\n", SQLLEN_MAX);
                     printf("buffer_length: " FMT_U_MEM "\n",
                            buffer_length);
                     printf("decimalDigits: " FMT_D16 "\n",
                            columnDescr->decimalDigits););
          break;
        case SQL_WCHAR:
        case SQL_WVARCHAR:
          c_type = SQL_C_WCHAR;
          if (unlikely(columnDescr->columnSize > (MAX_MEMSIZETYPE / 2) - 1)) {
            logError(printf("setupResultColumn: ColumnSize too big: " FMT_U_LEN "\n",
                            columnDescr->columnSize););
            err_info = MEMORY_ERROR;
          } else {
            buffer_length = ((memSizeType) columnDescr->columnSize + 1) * 2;
          } /* if */
          logMessage(printf("%s:\n",
                            nameOfSqlType(columnDescr->dataType));
                     printf("columnSize: " FMT_U_MEM "\n",
                            columnDescr->columnSize);
                     printf("SQLLEN_MAX: %ld\n", SQLLEN_MAX);
                     printf("buffer_length: " FMT_U_MEM "\n",
                            buffer_length);
                     printf("decimalDigits: " FMT_D16 "\n",
                            columnDescr->decimalDigits););
          break;
        case SQL_BINARY:
        case SQL_VARBINARY:
          c_type = SQL_C_BINARY;
          if (unlikely(columnDescr->columnSize > MAX_MEMSIZETYPE)) {
            logError(printf("setupResultColumn: ColumnSize too big: " FMT_U_LEN "\n",
                            columnDescr->columnSize););
            err_info = MEMORY_ERROR;
          } else {
            buffer_length = (memSizeType) columnDescr->columnSize;
          } /* if */
          logMessage(printf("%s:\n",
                            nameOfSqlType(columnDescr->dataType));
                     printf("columnSize: " FMT_U_MEM "\n",
                            columnDescr->columnSize);
                     printf("SQLLEN_MAX: %ld\n", SQLLEN_MAX);
                     printf("buffer_length: " FMT_U_MEM "\n",
                            buffer_length);
                     printf("decimalDigits: " FMT_D16 "\n",
                            columnDescr->decimalDigits););
          break;
        case SQL_LONGVARCHAR:
          if (preparedStmt->db->wideCharsSupported) {
            c_type = SQL_C_WCHAR;
          } else {
            c_type = SQL_C_CHAR;
          } /* if */
          columnDescr->sql_data_at_exec = TRUE;
          buffer_length = (memSizeType) SQL_DATA_AT_EXEC;
          break;
        case SQL_WLONGVARCHAR:
          c_type = SQL_C_WCHAR;
          columnDescr->sql_data_at_exec = TRUE;
          buffer_length = (memSizeType) SQL_DATA_AT_EXEC;
          break;
        case SQL_LONGVARBINARY:
          c_type = SQL_C_BINARY;
          columnDescr->sql_data_at_exec = TRUE;
          buffer_length = (memSizeType) SQL_DATA_AT_EXEC;
          break;
        case SQL_BIT:
          c_type = SQL_C_BIT;
          buffer_length = sizeof(char);
          break;
        case SQL_TINYINT:
          /* SQL_TINYINT can be signed (e.g. MySQL) or      */
          /* unsigned (e.g. MS SQL Server). We use a c_type */
          /* of SQL_C_SSHORT to be on the safe side.        */
        case SQL_SMALLINT:
          c_type = SQL_C_SSHORT;
          buffer_length = sizeof(int16Type);
          break;
        case SQL_INTEGER:
          c_type = SQL_C_SLONG;
          buffer_length = sizeof(int32Type);
          break;
        case SQL_BIGINT:
          c_type = SQL_C_SBIGINT;
          buffer_length = sizeof(int64Type);
          break;
        case SQL_DECIMAL:
          c_type = SQL_C_CHAR;
          if (unlikely(columnDescr->columnSize > MAX_MEMSIZETYPE - 4)) {
            logError(printf("setupResultColumn: ColumnSize too big: " FMT_U_LEN "\n",
                            columnDescr->columnSize););
            err_info = MEMORY_ERROR;
          } else {
            /* Add place for decimal point, sign, a possible       */
            /* leading or trailing zero and a terminating null byte. */
            buffer_length = ((memSizeType) columnDescr->columnSize + 4);
          } /* if */
          logMessage(printf("SQL_DECIMAL:\n");
                     printf("columnSize: "FMT_U_MEM "\n",
                            columnDescr->columnSize);
                     printf("buffer_length: " FMT_U_MEM "\n",
                            buffer_length);
                     printf("decimalDigits: " FMT_D16 "\n",
                            columnDescr->decimalDigits););
          break;
        case SQL_NUMERIC:
#if DECODE_NUMERIC_STRUCT
          c_type = SQL_C_NUMERIC;
          buffer_length = sizeof(SQL_NUMERIC_STRUCT);
#else
          c_type = SQL_C_CHAR;
          if (columnDescr->columnSize < MIN_PRECISION_FOR_NUMERIC_AS_DECIMAL) {
            buffer_length = MIN_PRECISION_FOR_NUMERIC_AS_DECIMAL;
          } else if (columnDescr->columnSize > MAX_PRECISION_FOR_NUMERIC_AS_DECIMAL) {
            buffer_length = MAX_PRECISION_FOR_NUMERIC_AS_DECIMAL;
          } else {
            buffer_length = (memSizeType) columnDescr->columnSize;
          } /* if */
          /* Place for sign, decimal point and zero byte. */
          buffer_length += 3;
#endif
          logMessage(printf("SQL_NUMERIC:\n");
                     printf("columnSize: " FMT_U_MEM "\n",
                            columnDescr->columnSize);
                     printf("buffer_length: " FMT_U_MEM "\n",
                            buffer_length);
                     printf("decimalDigits: " FMT_D16 "\n",
                            columnDescr->decimalDigits););
          break;
        case SQL_REAL:
          c_type = SQL_C_FLOAT;
          buffer_length = sizeof(float);
          break;
        case SQL_FLOAT:
        case SQL_DOUBLE:
          c_type = SQL_C_DOUBLE;
          buffer_length = sizeof(double);
          break;
        case SQL_TYPE_DATE:
          c_type = SQL_C_TYPE_DATE;
          buffer_length = sizeof(SQL_DATE_STRUCT);
          break;
        case SQL_TYPE_TIME:
          c_type = SQL_C_TYPE_TIME;
          buffer_length = sizeof(SQL_TIME_STRUCT);
          break;
        case SQL_DATETIME:
        case SQL_TYPE_TIMESTAMP:
          c_type = SQL_C_TYPE_TIMESTAMP;
          buffer_length = sizeof(SQL_TIMESTAMP_STRUCT);
          break;
        case SQL_INTERVAL_YEAR:
          c_type = SQL_C_INTERVAL_YEAR;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_INTERVAL_MONTH:
          c_type = SQL_C_INTERVAL_MONTH;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_INTERVAL_DAY:
          c_type = SQL_C_INTERVAL_DAY;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_INTERVAL_HOUR:
          c_type = SQL_C_INTERVAL_HOUR;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_INTERVAL_MINUTE:
          c_type = SQL_C_INTERVAL_MINUTE;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_INTERVAL_SECOND:
          c_type = SQL_C_INTERVAL_SECOND;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_INTERVAL_YEAR_TO_MONTH:
          c_type = SQL_C_INTERVAL_YEAR_TO_MONTH;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_INTERVAL_DAY_TO_HOUR:
          c_type = SQL_C_INTERVAL_DAY_TO_HOUR;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_INTERVAL_DAY_TO_MINUTE:
          c_type = SQL_C_INTERVAL_DAY_TO_MINUTE;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_INTERVAL_DAY_TO_SECOND:
          c_type = SQL_C_INTERVAL_DAY_TO_SECOND;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_INTERVAL_HOUR_TO_MINUTE:
          c_type = SQL_C_INTERVAL_HOUR_TO_MINUTE;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_INTERVAL_HOUR_TO_SECOND:
          c_type = SQL_C_INTERVAL_HOUR_TO_SECOND;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_INTERVAL_MINUTE_TO_SECOND:
          c_type = SQL_C_INTERVAL_MINUTE_TO_SECOND;
          buffer_length = sizeof(SQL_INTERVAL_STRUCT);
          break;
        case SQL_BLOB:
        case SQL_INFX_UDT_VARYING:
          columnDescr->dataType = SQL_LONGVARBINARY;
          c_type = SQL_C_BINARY;
          columnDescr->sql_data_at_exec = TRUE;
          buffer_length = (memSizeType) SQL_DATA_AT_EXEC;
          break;
        case SQL_CLOB:
        case SQL_XML:
          columnDescr->dataType = SQL_LONGVARCHAR;
          if (preparedStmt->db->wideCharsSupported) {
            c_type = SQL_C_WCHAR;
          } else {
            c_type = SQL_C_CHAR;
          } /* if */
          columnDescr->sql_data_at_exec = TRUE;
          buffer_length = (memSizeType) SQL_DATA_AT_EXEC;
          break;
        case SQL_SS_TIME2:
          c_type = SQL_C_BINARY;
          buffer_length = sizeof(sqlSsTime2Struct);
          break;
        default:
          logError(printf("setupResultColumn: Column %hd has the unknown type %s.\n",
                          column_num, nameOfSqlType(columnDescr->dataType)););
          err_info = RANGE_ERROR;
          break;
      } /* switch */
      if (err_info == OKAY_NO_ERROR) {
        logMessage(printf("c_type: %s\n", nameOfCType(c_type)););
        /* printf("buffer_length[%d]: " FMT_U_MEM " %d\n",
               column_num, buffer_length, columnDescr->sql_data_at_exec); */
        columnDescr->c_type = c_type;
        columnDescr->buffer_length = buffer_length;
      } /* if */
    } /* if */
    logFunction(printf("setupResultColumn --> %d\n", err_info););
    return err_info;
  } /* setupResultColumn */



static errInfoType setupResult (preparedStmtType preparedStmt)

  {
    SQLRETURN returnCode;
    SQLSMALLINT num_columns;
    SQLSMALLINT column_index;
    boolType blobFound = FALSE;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupResult */
    logFunction(printf("setupResult\n"););
    returnCode = SQLNumResultCols(preparedStmt->ppStmt, &num_columns);
    /* printf("returnCode: " FMT_D16 "\n", returnCode); */
    if (unlikely(returnCode != SQL_SUCCESS)) {
      setDbErrorMsg("setupResult", "SQLNumResultCols",
                    SQL_HANDLE_STMT, preparedStmt->ppStmt);
      logError(printf("setupResult: SQLNumResultCols:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } else if (unlikely(num_columns < 0)) {
      dbInconsistent("setupResult", "SQLNumResultCols");
      logError(printf("setupResult: SQLNumResultCols returns negative number: %hd\n",
                      num_columns););
      err_info = DATABASE_ERROR;
    } else if (num_columns == 0) {
      /* malloc(0) may return NULL, which would wrongly trigger a MEMORY_ERROR. */
      preparedStmt->result_array_size = 0;
      preparedStmt->result_descr_array = NULL;
    } else if (unlikely(!ALLOC_TABLE(preparedStmt->result_descr_array,
                                     resultDescrRecord, (memSizeType) num_columns))) {
      err_info = MEMORY_ERROR;
    } else {
      preparedStmt->result_array_size = (memSizeType) num_columns;
      memset(preparedStmt->result_descr_array, 0,
          (memSizeType) num_columns * sizeof(resultDescrRecord));
      for (column_index = 0; column_index < num_columns &&
           err_info == OKAY_NO_ERROR; column_index++) {
        err_info = setupResultColumn(preparedStmt, (SQLSMALLINT) (column_index + 1),
                                     &preparedStmt->result_descr_array[column_index]);
        blobFound |= preparedStmt->result_descr_array[column_index].sql_data_at_exec;
      } /* for */
      preparedStmt->hasBlob = blobFound;
    } /* if */
    logFunction(printf("setupResult --> %d\n", err_info););
    return err_info;
  } /* setupResult */



static errInfoType bindResultColumn (preparedStmtType preparedStmt,
    SQLSMALLINT column_num, resultDescrType columnDescr,
    resultDataType columnData, boolType blobFound)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* bindResultColumn */
    if (columnDescr->sql_data_at_exec) {
      columnData->buffer = NULL;
    } else {
      columnData->buffer = malloc(columnDescr->buffer_length);
      if (unlikely(columnData->buffer == NULL)) {
        err_info = MEMORY_ERROR;
      } else {
        memset(columnData->buffer, 0, columnDescr->buffer_length);
      } /* if */
    } /* if */
    if (err_info == OKAY_NO_ERROR) {
      /* The data of blobs (sql_data_at_exec = TRUE) is retrieved */
      /* with SQLGetData(). According to the SQL Server Native    */
      /* Client ODBC driver documentation SQLGetData() cannot     */
      /* retrieve data in random column order. Additionally all   */
      /* unbound columns processed with SQLGetData() must have    */
      /* higher column ordinals than the bound columns in the     */
      /* result set. Therefore binding of columns stops as soon   */
      /* as a blob column is found and the data is retrieved with */
      /* SQLGetData().                                            */
      if (!columnDescr->sql_data_at_exec && !blobFound) {
        /* printf("SQLBindCol(" FMT_U_MEM ", %d, " FMT_U_MEM ", ...)\n",
                  preparedStmt->ppStmt,
                  (int) column_num, columnDescr->buffer_length); */
        if (SQLBindCol(preparedStmt->ppStmt,
                       (SQLUSMALLINT) column_num,
                       columnDescr->c_type,
                       columnData->buffer,
                       (SQLLEN) columnDescr->buffer_length,
                       &columnData->length) != SQL_SUCCESS) {
          setDbErrorMsg("bindResultColumn", "SQLBindCol",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("bindResultColumn: SQLBindCol "
                          "c_type: %d = %s:\n%s\n",
                          columnDescr->c_type,
                          nameOfCType(columnDescr->c_type),
                          dbError.message););
          err_info = DATABASE_ERROR;
#if DECODE_NUMERIC_STRUCT
        } else if (columnDescr->c_type == SQL_C_NUMERIC) {
          err_info = setNumericPrecisionAndScale(preparedStmt, column_num,
                                                 columnDescr, columnData, TRUE);
#endif
        } /* if */
#if DECODE_NUMERIC_STRUCT
      } else if (columnDescr->c_type == SQL_C_NUMERIC) {
        err_info = setNumericPrecisionAndScale(preparedStmt, column_num,
                                               columnDescr, columnData, FALSE);
#endif
      } /* if */
    } /* if */
    return err_info;
  } /* bindResultColumn */



static errInfoType bindResult (preparedStmtType preparedStmt, fetchDataType fetchData)

  {
    memSizeType column_index;
    boolType blobFound = FALSE;
    errInfoType err_info = OKAY_NO_ERROR;

  /* bindResult */
    logFunction(printf("bindResult\n"););
    if (preparedStmt->result_array_size == 0) {
      /* malloc(0) may return NULL, which would wrongly trigger a MEMORY_ERROR. */
      fetchData->result_array = NULL;
    } else if (unlikely(!ALLOC_TABLE(fetchData->result_array, resultDataRecordCli,
                                     preparedStmt->result_array_size))) {
      err_info = MEMORY_ERROR;
    } else {
      memset(fetchData->result_array, 0,
          preparedStmt->result_array_size * sizeof(resultDataRecordCli));
      for (column_index = 0; column_index < preparedStmt->result_array_size &&
           err_info == OKAY_NO_ERROR; column_index++) {
        err_info = bindResultColumn(preparedStmt, (SQLSMALLINT) (column_index + 1),
                                    &preparedStmt->result_descr_array[column_index],
                                    &fetchData->result_array[column_index],
                                    blobFound);
        blobFound |= preparedStmt->result_descr_array[column_index].sql_data_at_exec;
      } /* for */
    } /* if */
    logFunction(printf("bindResult --> %d\n", err_info););
    return err_info;
  } /* bindResult */



static errInfoType copyNonBlobBuffers (resultDescrType columnDescr,
    resultDataType srcColumnData, resultDataType destColumnData)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* copyNonBlobBuffers */
    if (!columnDescr->sql_data_at_exec) {
      /* Copy the buffer contents from source to destination. */
      destColumnData->buffer = malloc(columnDescr->buffer_length);
      if (unlikely(destColumnData->buffer == NULL)) {
        err_info = MEMORY_ERROR;
      } else {
        memcpy(destColumnData->buffer, srcColumnData->buffer, columnDescr->buffer_length);
        destColumnData->length = srcColumnData->length;
      } /* if */
    } /* if */
    return err_info;
  } /* copyNonBlobBuffers */



static void moveBlobBuffers (resultDescrType columnDescr,
    resultDataType srcColumnData, resultDataType destColumnData)

  { /* moveBlobBuffers */
    if (columnDescr->sql_data_at_exec) {
      /* Copy the buffer pointer from source to destination. */
      destColumnData->buffer = srcColumnData->buffer;
      destColumnData->length = srcColumnData->length;
      srcColumnData->buffer = NULL;
      srcColumnData->length = 0;
    } /* if */
  } /* moveBlobBuffers */



static fetchDataType copyFetchData (preparedStmtType preparedStmt, fetchDataType source)

  {
    memSizeType column_index;
    errInfoType err_info = OKAY_NO_ERROR;
    fetchDataType fetchData;

  /* copyFetchData */
    logFunction(printf("copyFetchData\n"););
    if (likely(ALLOC_RECORD2(fetchData, fetchDataRecord,
                             count.fetch_data, count.fetch_data_bytes))) {
      memset(fetchData, 0, sizeof(fetchDataRecord));
      if (preparedStmt->result_array_size == 0) {
        /* malloc(0) may return NULL, which would wrongly trigger a MEMORY_ERROR. */
        fetchData->result_array = NULL;
      } else if (unlikely(!ALLOC_TABLE(fetchData->result_array, resultDataRecordCli,
                                       preparedStmt->result_array_size))) {
        FREE_RECORD2(fetchData, fetchDataRecord,
                     count.fetch_data, count.fetch_data_bytes);
        fetchData = NULL;
      } else {
        memset(fetchData->result_array, 0,
            preparedStmt->result_array_size * sizeof(resultDataRecordCli));
        for (column_index = 0; column_index < preparedStmt->result_array_size &&
             err_info == OKAY_NO_ERROR; column_index++) {
          err_info = copyNonBlobBuffers(&preparedStmt->result_descr_array[column_index],
                                        &source->result_array[column_index],
                                        &fetchData->result_array[column_index]);
        } /* for */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          freeFetch(preparedStmt, fetchData);
          fetchData = NULL;
        } else {
          for (column_index = 0; column_index < preparedStmt->result_array_size;
               column_index++) {
            moveBlobBuffers(&preparedStmt->result_descr_array[column_index],
                            &source->result_array[column_index],
                            &fetchData->result_array[column_index]);
          } /* for */
        } /* if */
      } /* if */
    } /* if */
    return fetchData;
  } /* copyFetchData */



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



#if DECODE_NUMERIC_STRUCT
static cstriType getNumericAsCStri (SQL_NUMERIC_STRUCT *numStruct)

  {
    bigIntType mantissa;
    striType stri;
    memSizeType decimalLen;
    memSizeType decimalIdx = 0;
    memSizeType idx;
    cstriType decimal;

  /* getNumericAsCStri */
    mantissa = bigFromByteBufferLe(SQL_MAX_NUMERIC_LEN, numStruct->val, FALSE);
    if (unlikely(mantissa == NULL)) {
      decimal = NULL;
    } else {
      stri = bigStrDecimal(mantissa);
      if (unlikely(stri == NULL)) {
        bigDestr(mantissa);
        raise_error(MEMORY_ERROR);
        decimal = NULL;
      } else {
        if (numStruct->scale < 0) {
          decimalLen = stri->size + (memSizeType) (-numStruct->scale) + 1 /* Space for decimal point */;
        } else if (numStruct->scale <= stri->size) {
          decimalLen = stri->size + 1 /* Space for decimal point */;
        } else {
          decimalLen = (memSizeType) numStruct->scale + 1 /* Space for decimal point */;
        } /* if */
        if (numStruct->sign != 1) {
          decimalLen++; /* Space for sign */
        } /* if */
        if (unlikely(!ALLOC_CSTRI(decimal, decimalLen))) {
          bigDestr(mantissa);
          strDestr(stri);
          raise_error(MEMORY_ERROR);
        } else {
          if (numStruct->sign != 1) {
            decimal[decimalIdx++] = '-';
          } /* if */
          for (idx = 0; idx < stri->size; idx++) {
            decimal[decimalIdx++] = (char) stri->mem[idx];
          } /* for */
          /* decimal[decimalIdx] = '\0';
          printf("# \"%s\", scale: %d, size: " FMT_U_MEM ", len: " FMT_U_MEM "\n",
              decimal, numStruct->scale, stri->size, decimalLen); */
          if (numStruct->scale < 0) {
            memset(&decimal[decimalIdx], '0', (memSizeType) (-numStruct->scale));
            decimal[decimalLen - 1] = '.';
            decimal[decimalLen] = '\0';
          } else if (numStruct->scale <= stri->size) {
            memmove(&decimal[decimalLen - (memSizeType) numStruct->scale],
                    &decimal[decimalLen - (memSizeType) numStruct->scale - 1],
                    (memSizeType) numStruct->scale);
            decimal[decimalLen - (memSizeType) numStruct->scale - 1] = '.';
            decimal[decimalLen] = '\0';
          } else {
            memmove(&decimal[decimalLen - stri->size],
                    &decimal[decimalLen - (memSizeType) numStruct->scale - 1],
                    stri->size);
            memset(&decimal[decimalLen - (memSizeType) numStruct->scale], '0',
                   (memSizeType) numStruct->scale - stri->size);
            decimal[decimalLen - (memSizeType) numStruct->scale - 1] = '.';
            decimal[decimalLen] = '\0';
          } /* if */
          bigDestr(mantissa);
          strDestr(stri);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("getNumericAsCStri --> %s\n", decimal));
    return decimal;
  } /* getNumericAsCStri */



static intType getNumericInt (const void *buffer)

  {
    SQL_NUMERIC_STRUCT *numStruct;
    bigIntType bigIntValue;
    bigIntType powerOfTen;
    intType intValue = 0;

  /* getNumericInt */
    numStruct = (SQL_NUMERIC_STRUCT *) buffer;
    logFunction(printf("getNumericInt\n");
                printf("numStruct->precision: %u\n", numStruct->precision);
                printf("numStruct->scale: %d\n", numStruct->scale);
                printf("numStruct->sign: %u\n", numStruct->sign);
                printf("numStruct->val:");
                {
                  int pos;
                  for (pos = 0; pos < SQL_MAX_NUMERIC_LEN; pos++) {
                    printf(" %d", numStruct->val[pos]);
                  }
                  printf("\n");
                });
    if (unlikely(numStruct->scale > 0)) {
      raise_error(RANGE_ERROR);
      intValue = 0;
    } else {
      bigIntValue = bigFromByteBufferLe(SQL_MAX_NUMERIC_LEN, numStruct->val, FALSE);
#if 0
      if (bigIntValue != NULL) {
        printf("numStruct->val: ");
        prot_bigint(bigIntValue);
        printf("\n");
      }
#endif
      if (bigIntValue != NULL && numStruct->scale < 0) {
        powerOfTen = bigIPowSignedDigit(10, (intType) -numStruct->scale);
        if (powerOfTen != NULL) {
          bigMultAssign(&bigIntValue, powerOfTen);
          bigDestr(powerOfTen);
        } /* if */
      } /* if */
      if (bigIntValue != NULL && numStruct->sign != 1) {
        bigIntValue = bigNegateTemp(bigIntValue);
      } /* if */
      if (bigIntValue != NULL) {
#if 0
        printf("numStruct->val: ");
        prot_bigint(bigIntValue);
        printf("\n");
#endif
#if INTTYPE_SIZE == 32
        intValue = bigToInt32(bigIntValue, NULL);
#elif INTTYPE_SIZE == 64
        intValue = bigToInt64(bigIntValue, NULL);
#endif
        bigDestr(bigIntValue);
      } /* if */
    } /* if */
    logFunction(printf("getNumericInt --> " FMT_D "\n", intValue););
    return intValue;
  } /* getNumericInt */



static bigIntType getNumericBigInt (const void *buffer)

  {
    SQL_NUMERIC_STRUCT *numStruct;
    bigIntType powerOfTen;
    bigIntType bigIntValue;

  /* getNumericBigInt */
    numStruct = (SQL_NUMERIC_STRUCT *) buffer;
    logFunction(printf("getNumericBigInt\n");
                printf("numStruct->precision: %u\n", numStruct->precision);
                printf("numStruct->scale: %d\n", numStruct->scale);
                printf("numStruct->sign: %u\n", numStruct->sign);
                printf("numStruct->val:");
                {
                  int pos;
                  for (pos = 0; pos < SQL_MAX_NUMERIC_LEN; pos++) {
                    printf(" %d", numStruct->val[pos]);
                  }
                  printf("\n");
                });
    if (unlikely(numStruct->scale > 0)) {
      raise_error(RANGE_ERROR);
      bigIntValue = NULL;
    } else {
      bigIntValue = bigFromByteBufferLe(SQL_MAX_NUMERIC_LEN, numStruct->val, FALSE);
#if 0
      if (bigIntValue != NULL) {
        printf("numStruct->val: ");
        prot_bigint(bigIntValue);
        printf("\n");
      } /* if */
#endif
      if (bigIntValue != NULL && numStruct->scale < 0) {
        powerOfTen = bigIPowSignedDigit(10, (intType) -numStruct->scale);
        if (powerOfTen != NULL) {
          bigMultAssign(&bigIntValue, powerOfTen);
          bigDestr(powerOfTen);
        } /* if */
      } /* if */
      if (bigIntValue != NULL && numStruct->sign != 1) {
        bigIntValue = bigNegateTemp(bigIntValue);
      } /* if */
    } /* if */
    logFunction(printf("getNumericBigInt --> ");
                prot_bigint(bigIntValue);
                printf("\n"););
    return bigIntValue;
 } /* getNumericBigInt */



static bigIntType getNumericBigRational (const void *buffer, bigIntType *denominator)

  {
    SQL_NUMERIC_STRUCT *numStruct;
    bigIntType powerOfTen;
    bigIntType numerator;

  /* getNumericBigRational */
    numStruct = (SQL_NUMERIC_STRUCT *) buffer;
    logFunction(printf("getNumericBigRational\n");
                printf("numStruct->precision: %u\n", numStruct->precision);
                printf("numStruct->scale: %d\n", numStruct->scale);
                printf("numStruct->sign: %u\n", numStruct->sign);
                printf("numStruct->val:");
                {
                  int pos;
                  for (pos = 0; pos < SQL_MAX_NUMERIC_LEN; pos++) {
                    printf(" %d", numStruct->val[pos]);
                  }
                  printf("\n");
                });
    numerator = bigFromByteBufferLe(SQL_MAX_NUMERIC_LEN, numStruct->val, FALSE);
#if 0
    if (numerator != NULL) {
      printf("numStruct->val: ");
      prot_bigint(numerator);
      printf("\n");
    } /* if */
#endif
    if (numerator != NULL && numStruct->sign != 1) {
      numerator = bigNegateTemp(numerator);
    } /* if */
    if (numerator != NULL) {
      if (numStruct->scale < 0) {
        powerOfTen = bigIPowSignedDigit(10, (intType) -numStruct->scale);
        if (powerOfTen != NULL) {
          bigMultAssign(&numerator, powerOfTen);
          bigDestr(powerOfTen);
        } /* if */
        *denominator = bigFromInt32(1);
      } else {
        *denominator = bigIPowSignedDigit(10, (intType) numStruct->scale);
      } /* if */
    } /* if */
    return numerator;
  } /* getNumericBigRational */



static floatType getNumericFloat (const void *buffer)

  {
    SQL_NUMERIC_STRUCT *numStruct;
    cstriType decimal;
    floatType floatValue;

  /* getNumericFloat */
    numStruct = (SQL_NUMERIC_STRUCT *) buffer;
    logFunction(printf("getNumericFloat\n");
                printf("numStruct->precision: %u\n", numStruct->precision);
                printf("numStruct->scale: %d\n", numStruct->scale);
                printf("numStruct->sign: %u\n", numStruct->sign);
                printf("numStruct->val:");
                {
                  int pos;
                  for (pos = 0; pos < SQL_MAX_NUMERIC_LEN; pos++) {
                    printf(" %d", numStruct->val[pos]);
                  }
                  printf("\n");
                });
    if (unlikely((decimal = getNumericAsCStri(numStruct)) == NULL)) {
      floatValue = 0.0;
    } else {
      floatValue = (floatType) strtod(decimal, NULL);
      UNALLOC_CSTRI(decimal, strlen(decimal));
    } /* if */
    logFunction(printf("getNumericFloat --> " FMT_E "\n", floatValue););
    return floatValue;
  } /* getNumericFloat */
#endif



static intType getInt (const void *buffer, memSizeType length)

  { /* getInt */
#if DECODE_NUMERIC_STRUCT
    return getNumericInt(buffer);
#else
    return getDecimalInt((const_ustriType) buffer, length);
#endif
  } /* getInt */



static bigIntType getBigInt (const void *buffer, memSizeType length)

  { /* getBigInt */
#if DECODE_NUMERIC_STRUCT
    return getNumericBigInt(buffer);
#else
    return getDecimalBigInt((const_ustriType) buffer, length);
#endif
  } /* getBigInt */



static bigIntType getBigRational (const void *buffer, memSizeType length,
    bigIntType *denominator)

  { /* getBigRational */
#if DECODE_NUMERIC_STRUCT
    return getNumericBigRational(buffer, denominator);
#else
    return getDecimalBigRational((const_ustriType) buffer, length, denominator);
#endif
  } /* getBigRational */



static floatType getFloat (const void *buffer, memSizeType length)

  { /* getFloat */
#if DECODE_NUMERIC_STRUCT
    return getNumericFloat(buffer);
#else
    return getDecimalFloat((const_ustriType) buffer, length);
#endif
  } /* getFloat */



#if ENCODE_NUMERIC_STRUCT
static memSizeType setNumericBigInt (void **buffer, memSizeType *buffer_capacity,
    const const_bigIntType bigIntValue, errInfoType *err_info)

  {
    bigIntType absoluteValue;
    boolType negative;
    bstriType bstri;
    SQL_NUMERIC_STRUCT *numStruct;

  /* setNumericBigInt */
    logFunction(printf("setNumericBigInt(*, *, %s, *)\n",
                       bigHexCStri(bigIntValue)););
    if (*buffer_capacity < sizeof(SQL_NUMERIC_STRUCT)) {
      free(*buffer);
      if (unlikely((*buffer = malloc(sizeof(SQL_NUMERIC_STRUCT))) == NULL)) {
        *buffer_capacity = 0;
        *err_info = MEMORY_ERROR;
      } else {
        *buffer_capacity = sizeof(SQL_NUMERIC_STRUCT);
      } /* if */
    } /* if */
    if (likely(*err_info == OKAY_NO_ERROR)) {
      negative = bigCmpSignedDigit(bigIntValue, 0) < 0;
      if (negative) {
        absoluteValue = bigAbs(bigIntValue);
        if (absoluteValue == NULL) {
          bstri = NULL;
        } else {
          bstri = bigToBStriLe(absoluteValue, FALSE);
          bigDestr(absoluteValue);
        } /* if */
      } else {
        bstri = bigToBStriLe(bigIntValue, FALSE);
      } /* if */
      if (bstri == NULL) {
        *err_info = MEMORY_ERROR;
      } else {
        if (bstri->size > SQL_MAX_NUMERIC_LEN) {
          logError(printf("setNumericBigInt: Data with length " FMT_U_MEM
                          " does not fit into a numeric.\n", bstri->size););
          *err_info = RANGE_ERROR;
        } else {
          numStruct = (SQL_NUMERIC_STRUCT *) *buffer;
          numStruct->precision = MAX_NUMERIC_PRECISION;
          numStruct->scale = 0;
          numStruct->sign = negative ? 0 : 1;
          memcpy(numStruct->val, bstri->mem, bstri->size);
          memset(&numStruct->val[bstri->size], 0, SQL_MAX_NUMERIC_LEN - bstri->size);
        } /* if */
        bstDestr(bstri);
      } /* if */
    } /* if */
    return sizeof(SQL_NUMERIC_STRUCT);
  } /* setNumericBigInt */



static memSizeType setNumericBigRat (void **buffer,
    const const_bigIntType numerator, const const_bigIntType denominator,
    SQLSMALLINT decimalDigits, errInfoType *err_info)

  {
    bigIntType number;
    bigIntType mantissaValue = NULL;
    bigIntType absoluteValue;
    boolType negative;
    bstriType bstri;
    SQL_NUMERIC_STRUCT *numStruct;

  /* setNumericBigRat */
    logFunction(printf("setNumericBigRat(*, %s, ",
                       bigHexCStri(numerator));
                printf("%s, " FMT_D16 ", *)\n",
                       bigHexCStri(denominator), decimalDigits););
    if (*buffer != NULL) {
      free(*buffer);
    } /* if */
    *buffer = malloc(sizeof(SQL_NUMERIC_STRUCT));
    if (unlikely(*buffer == NULL)) {
      *err_info = MEMORY_ERROR;
    } else {
      if (unlikely(bigEqSignedDigit(denominator, 0))) {
        /* Numeric values do not support Infinity and NaN. */
        logError(printf("setNumericBigRat: Decimal values do not support Infinity and NaN.\n"););
        *err_info = RANGE_ERROR;
      } else {
        number = bigIPowSignedDigit(10, decimalDigits);
        if (number != NULL) {
          bigMultAssign(&number, numerator);
          mantissaValue = bigDiv(number, denominator);
          bigDestr(number);
        } /* if */
        if (mantissaValue != NULL) {
          /* printf("mantissaValue: ");
             prot_bigint(mantissaValue);
             printf("\n"); */
          negative = bigCmpSignedDigit(mantissaValue, 0) < 0;
          if (negative) {
            absoluteValue = bigAbs(mantissaValue);
            if (absoluteValue == NULL) {
              bstri = NULL;
            } else {
              bstri = bigToBStriLe(absoluteValue, FALSE);
              bigDestr(absoluteValue);
            } /* if */
          } else {
            bstri = bigToBStriLe(mantissaValue, FALSE);
          } /* if */
          if (bstri == NULL) {
            *err_info = MEMORY_ERROR;
          } else {
            if (bstri->size > SQL_MAX_NUMERIC_LEN) {
              logError(printf("setNumericBigRat: Data with length " FMT_U_MEM
                              " does not fit into a numeric.\n", bstri->size););
              *err_info = RANGE_ERROR;
            } else {
              numStruct = (SQL_NUMERIC_STRUCT *) *buffer;
              numStruct->precision = MAX_NUMERIC_PRECISION;
              numStruct->scale = decimalDigits;
              numStruct->sign = negative ? 0 : 1;
              memcpy(numStruct->val, bstri->mem, bstri->size);
              memset(&numStruct->val[bstri->size], 0, SQL_MAX_NUMERIC_LEN - bstri->size);
              /* printf("setNumericToBigRat: \"%s\"\n", getNumericAsCStri(numStruct)); */
#if 0
              printf("numStruct->precision: %u\n", numStruct->precision);
              printf("numStruct->scale: %d\n", numStruct->scale);
              printf("numStruct->sign: %u\n", numStruct->sign);
              printf("numStruct->val: ");
              prot_bigint(mantissaValue);
              printf("\n");
              { int pos; for (pos = 0; pos < SQL_MAX_NUMERIC_LEN; pos++) {
                printf(" %d", numStruct->val[pos]); } printf("\n"); }
#endif
              /*{
                bigIntType numerator2;
                bigIntType denominator2;

                numerator2 = getNumericBigRational(*buffer, &denominator2);
                printf("stored: ");
                prot_bigint(numerator2);
                printf(" / ");
                prot_bigint(denominator2);
                printf("\n");
              } */
            } /* if */
            bstDestr(bstri);
          } /* if */
          bigDestr(mantissaValue);
        } /* if */
      } /* if */
    } /* if */
    return sizeof(SQL_NUMERIC_STRUCT);
  } /* setNumericBigRat */
#endif



static memSizeType setDecimalBigInt (void **buffer, memSizeType *buffer_capacity,
    const const_bigIntType bigIntValue, errInfoType *err_info)

  {
    striType stri;
    unsigned char *decimal;
    memSizeType srcIndex;
    memSizeType destIndex = 0;

  /* setDecimalBigInt */
    logFunction(printf("setDecimalBigInt(*, *, %s, *)\n",
                       bigHexCStri(bigIntValue)););
    stri = bigStrDecimal(bigIntValue);
    if (unlikely(stri == NULL)) {
      *err_info = MEMORY_ERROR;
    } else {
      /* printf("%s\n", striAsUnquotedCStri(stri)); */
      if (*buffer_capacity < stri->size + NULL_TERMINATION_LEN) {
        free(*buffer);
        *buffer = malloc(stri->size + NULL_TERMINATION_LEN);
        if (unlikely(*buffer == NULL)) {
          *buffer_capacity = 0;
          *err_info = MEMORY_ERROR;
        } else {
          *buffer_capacity = stri->size + NULL_TERMINATION_LEN;
        } /* if */
      } /* if */
      if (likely(*err_info == OKAY_NO_ERROR)) {
        decimal = (unsigned char *) *buffer;
        for (srcIndex = 0; srcIndex < stri->size; srcIndex++) {
          decimal[destIndex] = (unsigned char) stri->mem[srcIndex];
          destIndex++;
        } /* for */
        decimal[destIndex] = '\0';
        /* printf("%s\n", decimal); */
      } /* if */
      FREE_STRI(stri);
    } /* if */
    return destIndex;
  } /* setDecimalBigInt */



static memSizeType setDecimalBigRat (void **buffer,
    const const_bigIntType numerator, const const_bigIntType denominator,
    SQLSMALLINT decimalDigits, errInfoType *err_info)

  {
    memSizeType length;

  /* setDecimalBigRat */
    if (*buffer != NULL) {
      free(*buffer);
    } /* if */
    if (bigEqSignedDigit(denominator, 1)) {
      *buffer = bigIntToDecimal(numerator, &length, err_info);
    } else {
      *buffer = bigRatToDecimal(numerator, denominator, DEFAULT_DECIMAL_SCALE,
                                &length, err_info);
    } /* if */
    return length;
  } /* setDecimalBigRat */



static memSizeType setBigInt (void **buffer, memSizeType *buffer_capacity,
    const const_bigIntType bigIntValue, errInfoType *err_info)

  { /* setBigInt */
#if ENCODE_NUMERIC_STRUCT
    return setNumericBigInt(buffer, buffer_capacity, bigIntValue, err_info);
#else
    return setDecimalBigInt(buffer, buffer_capacity, bigIntValue, err_info);
#endif
  } /* setBigInt */



static memSizeType setBigRat (void **buffer, memSizeType *buffer_capacity,
    const const_bigIntType numerator, const const_bigIntType denominator,
    SQLSMALLINT decimalDigits, errInfoType *err_info)

  {
    memSizeType length;

  /* setBigRat */
#if ENCODE_NUMERIC_STRUCT
    length = setNumericBigRat(buffer, numerator, denominator, decimalDigits,
        err_info);
    *buffer_capacity = length;
#else
    length = setDecimalBigRat(buffer, numerator, denominator, decimalDigits,
        err_info);
    *buffer_capacity = length + NULL_TERMINATION_LEN;
#endif
    return length;
  } /* setBigRat */



static SQLSMALLINT assignToIntervalStruct (SQL_INTERVAL_STRUCT *interval,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  {
    int64Type monthDuration;
    int64Type microsecDuration;
    SQLSMALLINT c_type = 0;

  /* assignToIntervalStruct */
    logFunction(printf("assignToIntervalStruct(" FMT_U_MEM ", P"
                       FMT_D "Y" FMT_D "M" FMT_D "DT"
                       FMT_D "H" FMT_D "M%s" FMT_U "." F_U(06) "S)\n",
                       (memSizeType) interval,
                       year, month, day, hour, minute,
                       second < 0 || micro_second < 0 ? "-" : "",
                       intAbs(second), intAbs(micro_second)););
    /* Normalize the duration */
    monthDuration = (int64Type) year * 12 + (int64Type) month;
    microsecDuration = (((((int64Type) day) * 24 +
                           (int64Type) hour) * 60 +
                           (int64Type) minute) * 60 +
                           (int64Type) second) * 1000000 +
                           (int64Type) micro_second;
    logMessage(printf("monthDuration: " FMT_D64 "\n", monthDuration););
    logMessage(printf("microsecDuration: " FMT_D64 "\n", microsecDuration););
    month        = (intType) (monthDuration % 12);
    year         = (intType) (monthDuration / 12);
    micro_second = (intType) (microsecDuration % 1000000);
    microsecDuration /= 1000000;
    second       = (intType) (microsecDuration % 60);
    microsecDuration /= 60;
    minute       = (intType) (microsecDuration % 60);
    microsecDuration /= 60;
    hour         = (intType) (microsecDuration % 24);
    day          = (intType) (microsecDuration / 24);
    logMessage(printf("P" FMT_D "Y" FMT_D "M" FMT_D "DT"
                      FMT_D "H" FMT_D "M%s" FMT_U "." F_U(06) "S)\n",
                      year, month, day, hour, minute,
                      second < 0 || micro_second < 0 ? "-" : "",
                      intAbs(second), intAbs(micro_second)););

    /* Convert to SQL_INTERVAL_STRUCT */
    memset(interval, 0, sizeof(SQL_INTERVAL_STRUCT));
    if (day == 0 && hour == 0 && minute == 0 && second == 0 && micro_second == 0) {
      if (year != 0) {
        if (month != 0) {
          if (year > 0 && month > 0) {
            c_type = SQL_C_INTERVAL_YEAR_TO_MONTH;
            interval->interval_type = SQL_IS_YEAR_TO_MONTH;
            interval->interval_sign = SQL_FALSE;
            interval->intval.year_month.year  = (SQLUINTEGER) year;
            interval->intval.year_month.month = (SQLUINTEGER) month;
          } else if (year < 0 && month < 0) {
            c_type = SQL_C_INTERVAL_YEAR_TO_MONTH;
            interval->interval_type = SQL_IS_YEAR_TO_MONTH;
            interval->interval_sign = SQL_TRUE;
            interval->intval.year_month.year  = (SQLUINTEGER) -year;
            interval->intval.year_month.month = (SQLUINTEGER) -month;
          } /* if */
        } else {
          c_type = SQL_C_INTERVAL_YEAR;
          interval->interval_type = SQL_IS_YEAR;
          interval->interval_sign = year < 0 ? SQL_TRUE : SQL_FALSE;
          interval->intval.year_month.year = (SQLUINTEGER) abs((int) year);
        } /* if */
      } else if (month != 0) {
        c_type = SQL_C_INTERVAL_MONTH;
        interval->interval_type = SQL_IS_MONTH;
        interval->interval_sign = month < 0 ? SQL_TRUE : SQL_FALSE;
        interval->intval.year_month.month = (SQLUINTEGER) abs((int) month);
      } else {
        c_type = SQL_C_INTERVAL_SECOND;
        interval->interval_type = SQL_IS_SECOND;
        interval->interval_sign = SQL_FALSE;
        interval->intval.day_second.second = 0;
      } /* if */
    } else if (year == 0 && month == 0) {
      if (day != 0) {
        if (micro_second != 0) {
          if ((day >= 0 && hour >= 0 && minute >= 0 && second >= 0 && micro_second >= 0) ||
              (day <= 0 && hour <= 0 && minute <= 0 && second <= 0 && micro_second <= 0)) {
            c_type = SQL_C_INTERVAL_DAY_TO_SECOND;
            interval->interval_type = SQL_IS_DAY_TO_SECOND;
            interval->interval_sign = day < 0 ? SQL_TRUE : SQL_FALSE;
            interval->intval.day_second.day      = (SQLUINTEGER) abs((int) day);
            interval->intval.day_second.hour     = (SQLUINTEGER) abs((int) hour);
            interval->intval.day_second.minute   = (SQLUINTEGER) abs((int) minute);
            interval->intval.day_second.second   = (SQLUINTEGER) abs((int) second);
            interval->intval.day_second.fraction = (SQLUINTEGER) abs((int) micro_second);
          } /* if */
        } else if (second != 0) {
          if ((day >= 0 && hour >= 0 && minute >= 0 && second >= 0) ||
              (day <= 0 && hour <= 0 && minute <= 0 && second <= 0)) {
            c_type = SQL_C_INTERVAL_DAY_TO_SECOND;
            interval->interval_type = SQL_IS_DAY_TO_SECOND;
            interval->interval_sign = day < 0 ? SQL_TRUE : SQL_FALSE;
            interval->intval.day_second.day    = (SQLUINTEGER) abs((int) day);
            interval->intval.day_second.hour   = (SQLUINTEGER) abs((int) hour);
            interval->intval.day_second.minute = (SQLUINTEGER) abs((int) minute);
            interval->intval.day_second.second = (SQLUINTEGER) abs((int) second);
          } /* if */
        } else if (minute != 0) {
          if ((day >= 0 && hour >= 0 && minute >= 0) ||
              (day <= 0 && hour <= 0 && minute <= 0)) {
            c_type = SQL_C_INTERVAL_DAY_TO_MINUTE;
            interval->interval_type = SQL_IS_DAY_TO_MINUTE;
            interval->interval_sign = day < 0 ? SQL_TRUE : SQL_FALSE;
            interval->intval.day_second.day    = (SQLUINTEGER) abs((int) day);
            interval->intval.day_second.hour   = (SQLUINTEGER) abs((int) hour);
            interval->intval.day_second.minute = (SQLUINTEGER) abs((int) minute);
          } /* if */
        } else if (hour != 0) {
          if ((day >= 0 && hour >= 0) ||
              (day <= 0 && hour <= 0)) {
            c_type = SQL_C_INTERVAL_DAY_TO_HOUR;
            interval->interval_type = SQL_IS_DAY_TO_HOUR;
            interval->interval_sign = day < 0 ? SQL_TRUE : SQL_FALSE;
            interval->intval.day_second.day  = (SQLUINTEGER) abs((int) day);
            interval->intval.day_second.hour = (SQLUINTEGER) abs((int) hour);
          } /* if */
        } else {
          c_type = SQL_C_INTERVAL_DAY;
          interval->interval_type = SQL_IS_DAY;
          interval->interval_sign = day < 0 ? SQL_TRUE : SQL_FALSE;
          interval->intval.day_second.day = (SQLUINTEGER) abs((int) day);
        } /* if */
      } else if (hour != 0) {
        if (micro_second != 0) {
          if ((hour >= 0 && minute >= 0 && second >= 0 && micro_second >= 0) ||
              (hour <= 0 && minute <= 0 && second <= 0 && micro_second <= 0)) {
            c_type = SQL_C_INTERVAL_HOUR_TO_SECOND;
            interval->interval_type = SQL_IS_HOUR_TO_SECOND;
            interval->interval_sign = hour < 0 ? SQL_TRUE : SQL_FALSE;
            interval->intval.day_second.hour     = (SQLUINTEGER) abs((int) hour);
            interval->intval.day_second.minute   = (SQLUINTEGER) abs((int) minute);
            interval->intval.day_second.second   = (SQLUINTEGER) abs((int) second);
            interval->intval.day_second.fraction = (SQLUINTEGER) abs((int) micro_second);
          } /* if */
        } else if (second != 0) {
          if ((hour >= 0 && minute >= 0 && second >= 0) ||
              (hour <= 0 && minute <= 0 && second <= 0)) {
            c_type = SQL_C_INTERVAL_HOUR_TO_SECOND;
            interval->interval_type = SQL_IS_HOUR_TO_SECOND;
            interval->interval_sign = hour < 0 ? SQL_TRUE : SQL_FALSE;
            interval->intval.day_second.hour   = (SQLUINTEGER) abs((int) hour);
            interval->intval.day_second.minute = (SQLUINTEGER) abs((int) minute);
            interval->intval.day_second.second = (SQLUINTEGER) abs((int) second);
          } /* if */
        } else if (minute != 0) {
          if ((hour >= 0 && minute >= 0) ||
              (hour <= 0 && minute <= 0)) {
            c_type = SQL_C_INTERVAL_HOUR_TO_MINUTE;
            interval->interval_type = SQL_IS_HOUR_TO_MINUTE;
            interval->interval_sign = hour < 0 ? SQL_TRUE : SQL_FALSE;
            interval->intval.day_second.hour   = (SQLUINTEGER) abs((int) hour);
            interval->intval.day_second.minute = (SQLUINTEGER) abs((int) minute);
          } /* if */
        } else {
          c_type = SQL_C_INTERVAL_HOUR;
          interval->interval_type = SQL_IS_HOUR;
          interval->interval_sign = hour < 0 ? SQL_TRUE : SQL_FALSE;
          interval->intval.day_second.hour = (SQLUINTEGER) abs((int) hour);
        } /* if */
      } else if (minute != 0) {
        if (micro_second != 0) {
          if ((minute >= 0 && second >= 0 && micro_second >= 0) ||
              (minute <= 0 && second <= 0 && micro_second <= 0)) {
            c_type = SQL_C_INTERVAL_MINUTE_TO_SECOND;
            interval->interval_type = SQL_IS_MINUTE_TO_SECOND;
            interval->interval_sign = minute < 0 ? SQL_TRUE : SQL_FALSE;
            interval->intval.day_second.minute   = (SQLUINTEGER) abs((int) minute);
            interval->intval.day_second.second   = (SQLUINTEGER) abs((int) second);
            interval->intval.day_second.fraction = (SQLUINTEGER) abs((int) micro_second);
          } /* if */
        } else if (second != 0) {
          if ((minute >= 0 && second >= 0) ||
              (minute <= 0 && second <= 0)) {
            c_type = SQL_C_INTERVAL_MINUTE_TO_SECOND;
            interval->interval_type = SQL_IS_MINUTE_TO_SECOND;
            interval->interval_sign = minute < 0 ? SQL_TRUE : SQL_FALSE;
            interval->intval.day_second.minute = (SQLUINTEGER) abs((int) minute);
            interval->intval.day_second.second = (SQLUINTEGER) abs((int) second);
          } /* if */
        } else {
          c_type = SQL_C_INTERVAL_MINUTE;
          interval->interval_type = SQL_IS_MINUTE;
          interval->interval_sign = minute < 0 ? SQL_TRUE : SQL_FALSE;
          interval->intval.day_second.minute = (SQLUINTEGER) abs((int) minute);
        } /* if */
      } else if (second != 0) {
        if (micro_second != 0) {
          if ((second >= 0 && micro_second >= 0) ||
              (second <= 0 && micro_second <= 0)) {
            c_type = SQL_C_INTERVAL_SECOND;
            interval->interval_type = SQL_IS_SECOND;
            interval->interval_sign = second < 0 ? SQL_TRUE : SQL_FALSE;
            interval->intval.day_second.second   = (SQLUINTEGER) abs((int) second);
            interval->intval.day_second.fraction = (SQLUINTEGER) abs((int) micro_second);
          } /* if */
        } else {
          c_type = SQL_C_INTERVAL_SECOND;
          interval->interval_type = SQL_IS_SECOND;
          interval->interval_sign = second < 0 ? SQL_TRUE : SQL_FALSE;
          interval->intval.day_second.second = (SQLUINTEGER) abs((int) second);
        } /* if */
      } else {
        c_type = SQL_C_INTERVAL_SECOND;
        interval->interval_type = SQL_IS_SECOND;
        interval->interval_sign = micro_second < 0 ? SQL_TRUE : SQL_FALSE;
        interval->intval.day_second.fraction = (SQLUINTEGER) abs((int) micro_second);
      } /* if */
      interval->intval.day_second.fraction *= INTERVAL_FRACTION_FACTOR;
    } /* if */
    return c_type;
  } /* assignToIntervalStruct */



static boolType assignInterval (const const_cstriType interval,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second)
  {
    const char *apostrophePos;
    const char *numPos;
    const char *namePos;
    memSizeType numLength;
    memSizeType nameLength;
    char microsecStri[6 + NULL_TERMINATION_LEN];
    boolType aUnitIsPresent = FALSE;
    boolType roundUp = FALSE;
    boolType doNext = FALSE;
    boolType okay = TRUE;

  /* assignInterval */
    logFunction(printf("assignInterval(\"%s\")\n", interval););
    *year         = 0;
    *month        = 0;
    *day          = 0;
    *hour         = 0;
    *minute       = 0;
    *second       = 0;
    *micro_second = 0;
    if ((interval[0] == '+' || interval[0] == '-') &&
        interval[1] == '\'' &&
        (apostrophePos = strchr(&interval[2], '\'')) != NULL) {
      numPos = &interval[2];
      namePos = &apostrophePos[1];
      while (*namePos == ' ') {
        namePos++;
      } /* while */
      numLength = strspn(numPos, "0123456789");
      nameLength = strspn(namePos, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
      if (memcmp(namePos, "YEAR", nameLength) == 0) {
        okay = sscanf(numPos, FMT_D, year) == 1;
        if (okay) {
          numPos += numLength;
          if (*numPos == '-') {
            numPos++;
            numLength = strspn(numPos, "0123456789");
            okay = sscanf(numPos, FMT_D, month) == 1 &&
               numPos + numLength == apostrophePos;
          } else {
            okay = numPos == apostrophePos;
          } /* if */
        } /* if */
      } else if (memcmp(namePos, "MONTH", nameLength) == 0) {
        okay = sscanf(numPos, FMT_D, month) == 1 &&
           numPos + numLength == apostrophePos;
      } else {
        if (memcmp(namePos, "DAY", nameLength) == 0) {
          okay = sscanf(numPos, FMT_D, day) == 1;
          if (okay) {
            aUnitIsPresent = TRUE;
            numPos += numLength;
            if (*numPos == ' ') {
              numPos++;
              numLength = strspn(numPos, "0123456789");
              doNext = TRUE;
            } else {
              okay = numPos == apostrophePos;
            } /* if */
          } /* if */
        } /* if */
        if (okay && (doNext || memcmp(namePos, "HOUR", nameLength) == 0)) {
          doNext = FALSE;
          okay = sscanf(numPos, FMT_D, hour) == 1;
          if (okay) {
            aUnitIsPresent = TRUE;
            numPos += numLength;
            if (*numPos == ':') {
              numPos++;
              numLength = strspn(numPos, "0123456789");
              doNext = TRUE;
            } else {
              okay = numPos == apostrophePos;
            } /* if */
          } /* if */
        } /* if */
        if (okay && (doNext || memcmp(namePos, "MINUTE", nameLength) == 0)) {
          doNext = FALSE;
          okay = sscanf(numPos, FMT_D, minute) == 1;
          if (okay) {
            aUnitIsPresent = TRUE;
            numPos += numLength;
            if (*numPos == ':') {
              numPos++;
              numLength = strspn(numPos, "0123456789");
              doNext = TRUE;
            } else {
              okay = numPos == apostrophePos;
            } /* if */
          } /* if */
        } /* if */
        if (okay && (doNext || memcmp(namePos, "SECOND", nameLength) == 0)) {
          doNext = FALSE;
          okay = sscanf(numPos, FMT_D, second) == 1;
          if (okay) {
            aUnitIsPresent = TRUE;
            numPos += numLength;
            if (*numPos == '.') {
              numPos++;
              numLength = strspn(numPos, "0123456789");
              if (numLength > 6) {
                roundUp = numPos[6] >= '5';
                memcpy(microsecStri, numPos, 6);
              } else if (numLength == 6) {
                memcpy(microsecStri, numPos, 6);
              } else {
                memset(microsecStri, '0', 6 - (numLength));
                memcpy(&microsecStri[6 - (numLength)],
                        numPos, numLength);
              } /* if */
              microsecStri[6] = '\0';
              okay = sscanf(microsecStri, FMT_D, micro_second) == 1;
              if (okay) {
                numPos += numLength;
                okay = numPos == apostrophePos;
                if (okay) {
                  if (roundUp) {
                    (*micro_second)++;
                  } /* if */
                } /* if */
              } /* if */
            } else {
              okay = numPos == apostrophePos;
            } /* if */
          } /* if */
        } /* if */
        okay &= aUnitIsPresent;
      } /* if */
      if (okay && interval[0] == '-') {
        *year         = -*year;
        *month        = -*month;
        *day          = -*day;
        *hour         = -*hour;
        *minute       = -*minute;
        *second       = -*second;
        *micro_second = -*micro_second;
      } /* if */
    } else {
      okay = FALSE;
    } /* if */
    logFunction(printf("assignInterval(\"%s\") --> %d\n",
                       interval, okay););
    return okay;
  } /* assignInterval */



static errInfoType getBlob (preparedStmtType preparedStmt, memSizeType column,
    resultDataType columnData, SQLSMALLINT targetType)

  {
    char ch;
    SQLLEN totalLength;
    SQLRETURN returnCode;
    cstriType buffer;
    errInfoType err_info = OKAY_NO_ERROR;

  /* getBlob */
    logFunction(printf("getBlob(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) preparedStmt, column););
    if (columnData->buffer != NULL) {
      /* printf("getBlob: removing data\n"); */
      free(columnData->buffer);
      columnData->buffer = NULL;
      columnData->length = 0;
    } /* if */
    returnCode = SQLGetData(preparedStmt->ppStmt,
                            (SQLUSMALLINT) column,
                            targetType,
                            &ch, 0, &totalLength);
    if (returnCode == SQL_SUCCESS || returnCode == SQL_SUCCESS_WITH_INFO) {
      if (totalLength == SQL_NO_TOTAL) {
        err_info = RANGE_ERROR;
      } else if (totalLength == SQL_NULL_DATA || totalLength == 0) {
        logMessage(printf("Column is NULL or \"\" -> Use default value: \"\"\n"););
        columnData->length = totalLength;
      } else if (unlikely(totalLength < 0)) {
        dbInconsistent("getBlob", "SQLGetData");
        logError(printf("getBlob: Column " FMT_U_MEM ": "
                        "SQLGetData returns negative total length: " FMT_D_MEM "\n",
                        column, totalLength););
        err_info = DATABASE_ERROR;
      } else {
        /* printf("totalLength=" FMT_D_LEN "\n", totalLength); */
        if (unlikely((SQLULEN) totalLength > MAX_CSTRI_LEN ||
                     (buffer = (cstriType) malloc(
                         SIZ_CSTRI((memSizeType) (SQLULEN) totalLength))) == NULL)) {
          err_info = MEMORY_ERROR;
        } else {
          returnCode= SQLGetData(preparedStmt->ppStmt,
                                 (SQLUSMALLINT) column,
                                 targetType,
                                 buffer,
                                 SIZ_CSTRI(totalLength),
                                 &columnData->length);
          if (returnCode == SQL_SUCCESS || returnCode == SQL_SUCCESS_WITH_INFO) {
            columnData->buffer = buffer;
          } else {
            free(buffer);
            setDbErrorMsg("getBlob", "SQLGetData",
                          SQL_HANDLE_STMT, preparedStmt->ppStmt);
            logError(printf("getBlob: SQLGetData:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } /* if */
        } /* if */
      } /* if */
    } else {
      /* printf("returnCode: " FMT_D16 "\n", returnCode); */
      setDbErrorMsg("getBlob", "SQLGetData",
                    SQL_HANDLE_STMT, preparedStmt->ppStmt);
      logError(printf("getBlob: SQLGetData:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } /* if */
    logFunction(printf("getBlob --> %d\n", err_info););
    return err_info;
  } /* getBlob */



static errInfoType getWClob (preparedStmtType preparedStmt, memSizeType column,
    resultDataType columnData)

  {
    char ch;
    SQLLEN totalLength;
    memSizeType wstriLength;
    SQLRETURN returnCode;
    SQLWCHAR *wstri;
    errInfoType err_info = OKAY_NO_ERROR;

  /* getWClob */
    logFunction(printf("getWClob(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) preparedStmt, column););
    if (columnData->buffer != NULL) {
      /* printf("getWClob: removing data\n"); */
      free(columnData->buffer);
      columnData->buffer = NULL;
      columnData->length = 0;
    } /* if */
    returnCode = SQLGetData(preparedStmt->ppStmt,
                            (SQLUSMALLINT) column,
                            SQL_C_WCHAR,
                            &ch, 0, &totalLength);
    if (returnCode == SQL_SUCCESS || returnCode == SQL_SUCCESS_WITH_INFO) {
      if (totalLength == SQL_NO_TOTAL) {
        err_info = RANGE_ERROR;
      } else if (totalLength == SQL_NULL_DATA || totalLength == 0) {
        logMessage(printf("Column is NULL or \"\" -> Use default value: \"\"\n"););
        columnData->length = totalLength;
      } else if (unlikely(totalLength < 0)) {
        dbInconsistent("getWClob", "SQLGetData");
        logError(printf("getWClob: Column " FMT_U_MEM ": "
                        "SQLGetData returns negative total length: " FMT_D_LEN "\n",
                        column, totalLength););
        err_info = DATABASE_ERROR;
      } else if (unlikely((SQLULEN) totalLength > MAX_MEMSIZETYPE)){
        /* TotalLength is not representable as memSizeType. */
        /* Memory with this length cannot be allocated. */
        err_info = MEMORY_ERROR;
      } else {
        /* printf("totalLength=" FMT_D64 "\n", totalLength); */
        wstriLength = (memSizeType) totalLength / sizeof(SQLWCHAR);
        if (unlikely(wstriLength > MAX_SQLWSTRI_LEN ||
                     (wstri = (SQLWCHAR *) malloc(SIZ_SQLWSTRI(wstriLength))) == NULL)) {
          err_info = MEMORY_ERROR;
        } else {
          returnCode= SQLGetData(preparedStmt->ppStmt,
                                 (SQLUSMALLINT) column,
                                 SQL_C_WCHAR,
                                 wstri,
                                 (SQLLEN) SIZ_SQLWSTRI(wstriLength),
                                 &columnData->length);
          if (returnCode == SQL_SUCCESS || returnCode == SQL_SUCCESS_WITH_INFO) {
            columnData->buffer = (cstriType) wstri;
          } else {
            free(wstri);
            setDbErrorMsg("getWClob", "SQLGetData",
                          SQL_HANDLE_STMT, preparedStmt->ppStmt);
            logError(printf("getWClob: SQLGetData:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } /* if */
        } /* if */
      } /* if */
    } else {
      /* printf("returnCode: " FMT_D16 "\n", returnCode); */
      setDbErrorMsg("getWClob", "SQLGetData",
                    SQL_HANDLE_STMT, preparedStmt->ppStmt);
      logError(printf("getWClob: SQLGetData:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } /* if */
    logFunction(printf("getWClob --> %d\n", err_info););
    return err_info;
  } /* getWClob */



/**
 *  Get column data of an unbound column into an existing buffer.
 *  The buffer is allocated by bindResultColumn(), but the column
 *  is not bound with SQLBindCol(). This is done because this column
 *  has a higher column ordinal than a blob column.
 */
static errInfoType getData (preparedStmtType preparedStmt, memSizeType column,
    resultDataType columnData)

  {
    resultDescrType columnDescr;
    SQLSMALLINT c_type;
    SQLRETURN returnCode;
    errInfoType err_info = OKAY_NO_ERROR;

  /* getData */
    logFunction(printf("getData(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) preparedStmt, column););
    columnDescr = &preparedStmt->result_descr_array[column - 1];
    c_type = columnDescr->c_type;
#if DECODE_NUMERIC_STRUCT
    if (c_type == SQL_C_NUMERIC) {
      c_type = SQL_ARD_TYPE;
    } /* if */
#endif
    returnCode= SQLGetData(preparedStmt->ppStmt,
                           (SQLUSMALLINT) column,
                           c_type,
                           columnData->buffer,
                           (SQLLEN) columnDescr->buffer_length,
                           &columnData->length);
    if (returnCode != SQL_SUCCESS) {
      /* printf("returnCode: " FMT_D16 "\n", returnCode); */
      setDbErrorMsg("getData", "SQLGetData",
                    SQL_HANDLE_STMT, preparedStmt->ppStmt);
      logError(printf("getData: SQLGetData:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } /* if */
    logFunction(printf("getData --> %d\n", err_info););
    return err_info;
  } /* getData */



/**
 *  Get column data of blobs and all column data after the first blob.
 *  The data of blobs (sql_data_at_exec = TRUE) is retrieved
 *  with SQLGetData(). According to the SQL Server Native
 *  Client ODBC driver documentation SQLGetData() cannot
 *  retrieve data in random column order. Additionally all
 *  unbound columns processed with SQLGetData must have
 *  higher column ordinals than the bound columns in the
 *  result set. Therefore binding of columns stops as soon
 *  as a blob column is found and the data is retrieved with
 *  SQLGetData().
 */
static errInfoType fetchBlobs (preparedStmtType preparedStmt, fetchDataType fetchData)

  {
    memSizeType column;
    resultDescrType columnDescr;
    resultDataType columnData;
    boolType blobFound = FALSE;
    errInfoType err_info = OKAY_NO_ERROR;

  /* fetchBlobs */
    logFunction(printf("fetchBlobs(" FMT_U_MEM ")\n",
                       (memSizeType) preparedStmt););
    for (column = 1; column <= preparedStmt->result_array_size &&
         err_info == OKAY_NO_ERROR; column++) {
      columnDescr = &preparedStmt->result_descr_array[column - 1];
      columnData = &fetchData->result_array[column - 1];
      if (columnDescr->sql_data_at_exec) {
        blobFound = TRUE;
        /* printf("fetchBlobs: length: " FMT_D_LEN "\n", columnDescr->length); */
        logMessage(printf("dataType: %s\n", nameOfSqlType(columnDescr->dataType)););
        logMessage(printf("c_type: %s\n", nameOfCType(columnDescr->c_type)););
        switch (columnDescr->dataType) {
          case SQL_LONGVARCHAR:
          case SQL_WLONGVARCHAR:
            switch (columnDescr->c_type) {
              case SQL_C_CHAR:
                err_info = getBlob(preparedStmt, column, columnData, SQL_C_CHAR);
                break;
              case SQL_C_WCHAR:
                err_info = getWClob(preparedStmt, column, columnData);
                break;
              default:
                logError(printf("fetchBlobs: Parameter " FMT_U_MEM
                                " has the unknown C type %s.\n",
                                column, nameOfCType(columnDescr->c_type)););
                err_info = RANGE_ERROR;
                break;
            } /* switch */
            break;
          case SQL_LONGVARBINARY:
            err_info = getBlob(preparedStmt, column, columnData, SQL_C_BINARY);
            break;
          default:
            logError(printf("fetchBlobs: Parameter " FMT_U_MEM
                            " has the unknown type %s.\n",
                            column, nameOfSqlType(columnDescr->dataType)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
      } else if (blobFound) {
        err_info = getData(preparedStmt, column, columnData);
      } /* if */
    } /* for */
    logFunction(printf("fetchBlobs --> %d\n", err_info););
    return err_info;
  } /* fetchBlobs */



static fetchDataType prefetchOne (preparedStmtType preparedStmt,
    fetchDataType boundFetchData, errInfoType *err_info)

  {
    fetchDataType fetchData;

  /* prefetchOne */
    logFunction(printf("prefetchOne\n"););
    boundFetchData->fetch_result = SQLFetch(preparedStmt->ppStmt);
    if (boundFetchData->fetch_result == SQL_SUCCESS) {
      if (preparedStmt->hasBlob) {
        *err_info = fetchBlobs(preparedStmt, boundFetchData);
      } /* if */
      if (unlikely(*err_info != OKAY_NO_ERROR)) {
        fetchData = NULL;
      } else {
        fetchData = copyFetchData(preparedStmt, boundFetchData);
        if (unlikely(fetchData == NULL)) {
          *err_info = MEMORY_ERROR;
        } else {
          fetchData->fetch_result = boundFetchData->fetch_result;
        } /* if */
      } /* if */
    } else if (boundFetchData->fetch_result == SQL_NO_DATA) {
      if (unlikely(!ALLOC_RECORD2(fetchData, fetchDataRecord,
                                  count.fetch_data, count.fetch_data_bytes))) {
        *err_info = MEMORY_ERROR;
      } else {
        memset(fetchData, 0, sizeof(fetchDataRecord));
        fetchData->fetch_result = boundFetchData->fetch_result;
      } /* if */
    } else {
      setDbErrorMsg("prefetchOne", "SQLFetch",
                    SQL_HANDLE_STMT, preparedStmt->ppStmt);
      logError(printf("prefetchOne: SQLFetch fetch_result: %d:\n%s\n",
                      boundFetchData->fetch_result, dbError.message););
      *err_info = DATABASE_ERROR;
      fetchData = NULL;
    } /* if */
    logFunction(printf("prefetchOne --> " FMT_U_MEM " (err_info=%d)\n",
                       (memSizeType) fetchData, *err_info););
    return fetchData;
  } /* prefetchOne */



static errInfoType prefetchAll (preparedStmtType preparedStmt, fetchDataType boundFetchData)

  {
    fetchDataType *listEnd;
    fetchDataType fetchData;
    errInfoType err_info = OKAY_NO_ERROR;

  /* prefetchAll */
    logFunction(printf("prefetchAll\n"););
    listEnd = &preparedStmt->prefetched;
    while ((fetchData = prefetchOne(preparedStmt, boundFetchData, &err_info)) != NULL &&
            fetchData->fetch_result == SQL_SUCCESS) {
      *listEnd = fetchData;
      listEnd = &fetchData->next;
    } /* while */
    if (fetchData != NULL && fetchData->fetch_result == SQL_NO_DATA) {
      *listEnd = fetchData;
      listEnd = &fetchData->next;
    } /* if */
    *listEnd = NULL;
    logFunction(printf("prefetchAll --> %d\n", err_info););
    return err_info;
  } /* prefetchAll */



static errInfoType doFetch (preparedStmtType preparedStmt, fetchDataType boundFetchData)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* doFetch */
    if (preparedStmt->prefetched != NULL) {
      if (preparedStmt->currentFetch != NULL &&
          preparedStmt->currentFetch != boundFetchData) {
        freeFetch(preparedStmt, preparedStmt->currentFetch);
      } /* if */
      preparedStmt->currentFetch = preparedStmt->prefetched;
      preparedStmt->prefetched = preparedStmt->prefetched->next;
      preparedStmt->currentFetch->next = NULL;
    } else {
      boundFetchData->fetch_result = SQLFetch(preparedStmt->ppStmt);
#ifdef ALLOW_FETCH_SUCCESS_WITH_INFO
      if (boundFetchData->fetch_result == SQL_SUCCESS_WITH_INFO) {
        boundFetchData->fetch_result = SQL_SUCCESS;
      } /*if */
#endif
      if (boundFetchData->fetch_result == SQL_SUCCESS) {
        if (preparedStmt->hasBlob) {
          err_info = fetchBlobs(preparedStmt, boundFetchData);
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          preparedStmt->currentFetch = NULL;
        } else {
          boundFetchData->next = NULL;
          preparedStmt->currentFetch = boundFetchData;
        } /* if */
      } else if (boundFetchData->fetch_result == SQL_NO_DATA) {
        boundFetchData->next = NULL;
        preparedStmt->currentFetch = boundFetchData;
      } else {
        setDbErrorMsg("doFetch", "SQLFetch",
                      SQL_HANDLE_STMT, preparedStmt->ppStmt);
        logError(printf("doFetch: SQLFetch fetch_result: %d:\n%s\n",
                        boundFetchData->fetch_result, dbError.message););
        err_info = DATABASE_ERROR;
        preparedStmt->currentFetch = NULL;
      } /* if */
    } /* if */
    return err_info;
  } /* doFetch */



static void sqlBindBigInt (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType value)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    int16Type value16;
    SQLSMALLINT c_type;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBigInt */
    logFunction(printf("sqlBindBigInt(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, bigHexCStri(value)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBigInt: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      err_info = RANGE_ERROR;
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(SQLFreeStmt(preparedStmt->ppStmt, SQL_CLOSE) != SQL_SUCCESS)) {
          setDbErrorMsg("sqlBindBigInt", "SQLFreeStmt",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlBindBigInt: SQLFreeStmt SQL_CLOSE:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
          freePrefetched(preparedStmt);
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        param = &preparedStmt->param_array[pos - 1];
        logMessage(printf("dataType: %s\n", nameOfSqlType(param->dataType)););
        switch (param->dataType) {
          case SQL_BIT:
            value16 = bigToInt16(value, &err_info);
            if (likely(err_info == OKAY_NO_ERROR)) {
              if (unlikely(value16 < 0 || value16 > 1)) {
                logError(printf("sqlBindBigInt: Parameter " FMT_D ": "
                                FMT_D16 " does not fit into a bit.\n",
                                pos, value16));
                err_info = RANGE_ERROR;
              } else {
                c_type = SQL_C_BIT;
                *(char *) param->buffer = (char) value16;
              } /* if */
            } /* if */
            break;
          case SQL_TINYINT:
            value16 = bigToInt16(value, &err_info);
            if (likely(err_info == OKAY_NO_ERROR)) {
              if (preparedStmt->db->tinyintIsUnsigned) {
                if (unlikely(value16 < 0 || value16 > UINT8TYPE_MAX)) {
                  logError(printf("sqlBindBigInt: Parameter " FMT_D ": "
                                  FMT_D16 " does not fit into a 8-bit unsigned integer.\n",
                                  pos, value16));
                  err_info = RANGE_ERROR;
                } else {
                  c_type = SQL_C_UTINYINT;
                  *(uint8Type *) param->buffer = (uint8Type) value16;
                } /* if */
              } else {
                if (unlikely(value16 < INT8TYPE_MIN || value16 > INT8TYPE_MAX)) {
                  logError(printf("sqlBindBigInt: Parameter " FMT_D ": "
                                  FMT_D16 " does not fit into a 8-bit signed integer.\n",
                                  pos, value16));
                  err_info = RANGE_ERROR;
                } else {
                  c_type = SQL_C_STINYINT;
                  *(int8Type *) param->buffer = (int8Type) value16;
                } /* if */
              } /* if */
            } /* if */
            break;
          case SQL_SMALLINT:
            c_type = SQL_C_SSHORT;
            *(int16Type *) param->buffer = bigToInt16(value, &err_info);
            break;
          case SQL_INTEGER:
            c_type = SQL_C_SLONG;
            *(int32Type *) param->buffer = bigToInt32(value, &err_info);
            break;
          case SQL_BIGINT:
            c_type = SQL_C_SBIGINT;
            *(int64Type *) param->buffer = bigToInt64(value, &err_info);
            break;
          case SQL_REAL:
            c_type = SQL_C_FLOAT;
            *(float *) param->buffer = (float) bigIntToDouble(value);
            break;
          case SQL_FLOAT:
          case SQL_DOUBLE:
            c_type = SQL_C_DOUBLE;
            *(double *) param->buffer = bigIntToDouble(value);
            break;
          case SQL_DECIMAL:
          case SQL_NUMERIC:
          case SQL_CHAR:
          case SQL_VARCHAR:
          case SQL_LONGVARCHAR:
#if ENCODE_NUMERIC_STRUCT
            c_type = SQL_C_NUMERIC,
#else
            c_type = SQL_C_CHAR,
#endif
            param->buffer_length = setBigInt(&param->buffer,
                                             &param->buffer_capacity,
                                             value, &err_info);
            break;
          default:
            logError(printf("sqlBindBigInt: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(param->dataType)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (unlikely(SQLBindParameter(preparedStmt->ppStmt,
                                        (SQLUSMALLINT) pos,
                                        SQL_PARAM_INPUT,
                                        c_type,
                                        param->dataType,
                                        param->paramSize,
                                        param->decimalDigits,
                                        param->buffer,
                                        (SQLLEN) param->buffer_length,
                                        NULL) != SQL_SUCCESS)) {
            setDbErrorMsg("sqlBindBigInt", "SQLBindParameter",
                          SQL_HANDLE_STMT, preparedStmt->ppStmt);
            logError(printf("sqlBindBigInt: SQLBindParameter:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } else {
            preparedStmt->fetchOkay = FALSE;
            param->bound = TRUE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* sqlBindBigInt */



static void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType numerator, const const_bigIntType denominator)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    SQLSMALLINT c_type;
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
      err_info = RANGE_ERROR;
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(SQLFreeStmt(preparedStmt->ppStmt, SQL_CLOSE) != SQL_SUCCESS)) {
          setDbErrorMsg("sqlBindBigRat", "SQLFreeStmt",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlBindBigRat: SQLFreeStmt SQL_CLOSE:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
          freePrefetched(preparedStmt);
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        param = &preparedStmt->param_array[pos - 1];
        logMessage(printf("dataType: %s\n", nameOfSqlType(param->dataType)););
        switch (param->dataType) {
          case SQL_DECIMAL:
          case SQL_NUMERIC:
          case SQL_VARCHAR:
          case SQL_LONGVARCHAR:
#if ENCODE_NUMERIC_STRUCT
            c_type = SQL_C_NUMERIC,
#else
            c_type = SQL_C_CHAR,
#endif
            param->buffer_length =
                setBigRat(&param->buffer, &param->buffer_capacity,
                          numerator, denominator,
                          param->decimalDigits, &err_info);
            break;
          case SQL_REAL:
            c_type = SQL_C_FLOAT;
            *(float *) param->buffer =
                (float) bigRatToDouble(numerator, denominator);
            break;
          case SQL_FLOAT:
          case SQL_DOUBLE:
            c_type = SQL_C_DOUBLE;
            *(double *) param->buffer =
                bigRatToDouble(numerator, denominator);
            break;
          default:
            logError(printf("sqlBindBigRat: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(param->dataType)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (unlikely(SQLBindParameter(preparedStmt->ppStmt,
                                        (SQLUSMALLINT) pos,
                                        SQL_PARAM_INPUT,
                                        c_type,
                                        param->dataType,
                                        param->paramSize,
                                        param->decimalDigits,
                                        param->buffer,
                                        (SQLLEN) param->buffer_length,
                                        NULL) != SQL_SUCCESS)) {
            setDbErrorMsg("sqlBindBigRat", "SQLBindParameter",
                          SQL_HANDLE_STMT, preparedStmt->ppStmt);
            logError(printf("sqlBindBigRat: SQLBindParameter:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } else {
            preparedStmt->fetchOkay = FALSE;
            param->bound = TRUE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* sqlBindBigRat */



static void sqlBindBool (sqlStmtType sqlStatement, intType pos, boolType value)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    SQLSMALLINT c_type;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBool */
    logFunction(printf("sqlBindBool(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, value ? "TRUE" : "FALSE"););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBool: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      err_info = RANGE_ERROR;
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(SQLFreeStmt(preparedStmt->ppStmt, SQL_CLOSE) != SQL_SUCCESS)) {
          setDbErrorMsg("sqlBindBool", "SQLFreeStmt",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlBindBool: SQLFreeStmt SQL_CLOSE:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
          freePrefetched(preparedStmt);
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        param = &preparedStmt->param_array[pos - 1];
        logMessage(printf("dataType: %s\n", nameOfSqlType(param->dataType)););
        switch (param->dataType) {
          case SQL_BIT:
            c_type = SQL_C_BIT;
            *(char *) param->buffer = (char) value;
            break;
          case SQL_TINYINT:
            c_type = SQL_C_STINYINT;
            *(int8Type *) param->buffer = (int8Type) value;
            break;
          case SQL_SMALLINT:
            c_type = SQL_C_SSHORT;
            *(int16Type *) param->buffer = (int16Type) value;
            break;
          case SQL_INTEGER:
            c_type = SQL_C_SLONG;
            *(int32Type *) param->buffer = (int32Type) value;
            break;
          case SQL_BIGINT:
            c_type = SQL_C_SBIGINT;
            *(int64Type *) param->buffer = (int64Type) value;
            break;
          case SQL_REAL:
            c_type = SQL_C_FLOAT;
            *(float *) param->buffer = (float) value;
            break;
          case SQL_FLOAT:
          case SQL_DOUBLE:
            c_type = SQL_C_DOUBLE;
            *(double *) param->buffer = (double) value;
            break;
          case SQL_DECIMAL:
          case SQL_NUMERIC:
          case SQL_CHAR:
          case SQL_VARCHAR:
          case SQL_LONGVARCHAR:
            c_type = SQL_C_SLONG;
            if (param->buffer_capacity < sizeof(int32Type)) {
              free(param->buffer);
              if (unlikely((param->buffer = malloc(sizeof(int32Type))) == NULL)) {
                param->buffer_capacity = 0;
                err_info = MEMORY_ERROR;
              } else {
                param->buffer_capacity = sizeof(int32Type);
              } /* if */
            } /* if */
            if (likely(err_info == OKAY_NO_ERROR)) {
              param->buffer_length = sizeof(int32Type);
              *(int32Type *) param->buffer = (int32Type) value;
            } /* if */
            break;
          default:
            logError(printf("sqlBindBool: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(param->dataType)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (unlikely(SQLBindParameter(preparedStmt->ppStmt,
                                        (SQLUSMALLINT) pos,
                                        SQL_PARAM_INPUT,
                                        c_type,
                                        param->dataType,
                                        param->paramSize,
                                        param->decimalDigits,
                                        param->buffer,
                                        (SQLLEN) param->buffer_length,
                                        NULL) != SQL_SUCCESS)) {
            setDbErrorMsg("sqlBindBool", "SQLBindParameter",
                          SQL_HANDLE_STMT, preparedStmt->ppStmt);
            logError(printf("sqlBindBool: SQLBindParameter:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } else {
            preparedStmt->fetchOkay = FALSE;
            param->bound = TRUE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* sqlBindBool */



static void sqlBindBStri (sqlStmtType sqlStatement, intType pos,
    const const_bstriType bstri)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    SQLSMALLINT c_type;
    memSizeType minimum_size;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBStri */
    logFunction(printf("sqlBindBStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, bstriAsUnquotedCStri(bstri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBStri: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      err_info = RANGE_ERROR;
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(SQLFreeStmt(preparedStmt->ppStmt, SQL_CLOSE) != SQL_SUCCESS)) {
          setDbErrorMsg("sqlBindBStri", "SQLFreeStmt",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlBindBStri: SQLFreeStmt SQL_CLOSE:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
          freePrefetched(preparedStmt);
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        param = &preparedStmt->param_array[pos - 1];
        logMessage(printf("dataType: %s\n", nameOfSqlType(param->dataType)););
        switch (param->dataType) {
          case SQL_BINARY:
          case SQL_VARBINARY:
          case SQL_LONGVARBINARY:
          case SQL_VARCHAR:
          case SQL_LONGVARCHAR:
          case SQL_BLOB:
          case SQL_CLOB:
            c_type = SQL_C_BINARY;
            if (unlikely(bstri->size > SQLLEN_MAX)) {
              /* It is not possible to cast bstri->size to SQLLEN. */
              err_info = MEMORY_ERROR;
            } else {
              /* Use a buffer size with at least one byte. */
              minimum_size = bstri->size == 0 ? 1 : bstri->size;
              if (param->buffer_capacity < minimum_size) {
                free(param->buffer);
                if (unlikely((param->buffer = malloc(minimum_size)) == NULL)) {
                  param->buffer_capacity = 0;
                  err_info = MEMORY_ERROR;
                } else {
                  param->buffer_capacity = minimum_size;
                } /* if */
              } /* if */
              if (likely(err_info == OKAY_NO_ERROR)) {
                memcpy(param->buffer, bstri->mem, bstri->size);
                param->buffer_length = bstri->size;
                /* The length is necessary to avoid that a zero byte terminates the data. */
                param->length = (SQLLEN) bstri->size;
              } /* if */
            } /* if */
            break;
          default:
            logError(printf("sqlBindBStri: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(param->dataType)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (unlikely(SQLBindParameter(preparedStmt->ppStmt,
                                        (SQLUSMALLINT) pos,
                                        SQL_PARAM_INPUT,
                                        c_type,
                                        param->dataType,
                                        param->paramSize,
                                        param->decimalDigits,
                                        param->buffer,
                                        (SQLLEN) param->buffer_length,
                                        &param->length) != SQL_SUCCESS)) {
            setDbErrorMsg("sqlBindBStri", "SQLBindParameter",
                          SQL_HANDLE_STMT, preparedStmt->ppStmt);
            logError(printf("sqlBindBStri: SQLBindParameter:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } else {
            preparedStmt->fetchOkay = FALSE;
            param->bound = TRUE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* sqlBindBStri */



static void sqlBindDuration (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    SQLSMALLINT c_type = 0;
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
      err_info = RANGE_ERROR;
    } else if (unlikely(year < SQLINTEGER_MIN || year > SQLINTEGER_MAX ||
                        month < -12 || month > 12 ||
                        day < -31 || day > 31 ||
                        hour <= -24 || hour >= 24 ||
                        minute <= -60 || minute >= 60 ||
                        second <= -60 || second >= 60 ||
                        micro_second <= -1000000 || micro_second >= 1000000)) {
      logError(printf("sqlBindDuration: Duration not in allowed range.\n"););
      err_info = RANGE_ERROR;
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(SQLFreeStmt(preparedStmt->ppStmt, SQL_CLOSE) != SQL_SUCCESS)) {
          setDbErrorMsg("sqlBindDuration", "SQLFreeStmt",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlBindDuration: SQLFreeStmt SQL_CLOSE:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
          freePrefetched(preparedStmt);
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        param = &preparedStmt->param_array[pos - 1];
        logMessage(printf("dataType: %s\n", nameOfSqlType(param->dataType)););
        switch (param->dataType) {
          case SQL_VARCHAR:
          case SQL_LONGVARCHAR:
            if (param->buffer_capacity < sizeof(SQL_INTERVAL_STRUCT)) {
              free(param->buffer);
              if (unlikely((param->buffer = malloc(
                                sizeof(SQL_INTERVAL_STRUCT))) == NULL)) {
                param->buffer_capacity = 0;
                err_info = MEMORY_ERROR;
              } else {
                param->buffer_capacity = sizeof(SQL_INTERVAL_STRUCT);
              } /* if */
            } /* if */
            if (likely(err_info == OKAY_NO_ERROR)) {
              param->buffer_length = sizeof(SQL_INTERVAL_STRUCT);
              c_type = assignToIntervalStruct(
                  (SQL_INTERVAL_STRUCT *) param->buffer,
                  year, month, day, hour, minute, second, micro_second);
              if (unlikely(c_type == 0)) {
                logError(printf("sqlBindDuration(" FMT_U_MEM ", " FMT_D ", P"
                                                   FMT_D "Y" FMT_D "M" FMT_D "DT"
                                                   FMT_D "H" FMT_D "M%s" FMT_U "." F_U(06) "S): "
                                                 "There is no adequate interval type.\n",
                                (memSizeType) sqlStatement, pos,
                                year, month, day, hour, minute,
                                second < 0 || micro_second < 0 ? "-" : "",
                                intAbs(second), intAbs(micro_second)););
                err_info = RANGE_ERROR;
              } /* if */
            } /* if */
            break;
          default:
            logError(printf("sqlBindDuration: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(param->dataType)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (unlikely(SQLBindParameter(preparedStmt->ppStmt,
                                        (SQLUSMALLINT) pos,
                                        SQL_PARAM_INPUT,
                                        c_type,
                                        param->dataType,
                                        param->paramSize,
                                        param->decimalDigits,
                                        param->buffer,
                                        (SQLLEN) param->buffer_length,
                                        NULL) != SQL_SUCCESS)) {
            setDbErrorMsg("sqlBindDuration", "SQLBindParameter",
                          SQL_HANDLE_STMT, preparedStmt->ppStmt);
            logError(printf("sqlBindDuration: SQLBindParameter:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } else {
            preparedStmt->fetchOkay = FALSE;
            param->bound = TRUE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* sqlBindDuration */



static void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    SQLSMALLINT c_type;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindFloat */
    logFunction(printf("sqlBindFloat(" FMT_U_MEM ", " FMT_D ", " FMT_E ")\n",
                       (memSizeType) sqlStatement, pos, value););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindFloat: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      err_info = RANGE_ERROR;
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(SQLFreeStmt(preparedStmt->ppStmt, SQL_CLOSE) != SQL_SUCCESS)) {
          setDbErrorMsg("sqlBindFloat", "SQLFreeStmt",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlBindFloat: SQLFreeStmt SQL_CLOSE:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
          freePrefetched(preparedStmt);
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        param = &preparedStmt->param_array[pos - 1];
        logMessage(printf("dataType: %s\n", nameOfSqlType(param->dataType)););
        switch (param->dataType) {
          case SQL_REAL:
            c_type = SQL_C_FLOAT;
            *(float *) param->buffer = (float) value;
            break;
          case SQL_FLOAT:
          case SQL_DOUBLE:
            c_type = SQL_C_DOUBLE;
            *(double *) param->buffer = (double) value;
            break;
          case SQL_VARCHAR:
          case SQL_LONGVARCHAR:
            c_type = SQL_C_DOUBLE;
            if (param->buffer_capacity < sizeof(double)) {
              free(param->buffer);
              if (unlikely((param->buffer = malloc(sizeof(double))) == NULL)) {
                param->buffer_capacity = 0;
                err_info = MEMORY_ERROR;
              } else {
                param->buffer_capacity = sizeof(double);
              } /* if */
            } /* if */
            if (likely(err_info == OKAY_NO_ERROR)) {
              param->buffer_length = sizeof(double);
              *(double *) param->buffer = (double) value;
            } /* if */
            break;
          default:
            logError(printf("sqlBindFloat: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(param->dataType)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (unlikely(SQLBindParameter(preparedStmt->ppStmt,
                                        (SQLUSMALLINT) pos,
                                        SQL_PARAM_INPUT,
                                        c_type,
                                        param->dataType,
                                        param->paramSize,
                                        param->decimalDigits,
                                        param->buffer,
                                        (SQLLEN) param->buffer_length,
                                        NULL) != SQL_SUCCESS)) {
            setDbErrorMsg("sqlBindFloat", "SQLBindParameter",
                          SQL_HANDLE_STMT, preparedStmt->ppStmt);
            logError(printf("sqlBindFloat: SQLBindParameter:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } else {
            preparedStmt->fetchOkay = FALSE;
            param->bound = TRUE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* sqlBindFloat */



static void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    SQLSMALLINT c_type;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindInt */
    logFunction(printf("sqlBindInt(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos, value););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindInt: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      err_info = RANGE_ERROR;
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(SQLFreeStmt(preparedStmt->ppStmt, SQL_CLOSE) != SQL_SUCCESS)) {
          setDbErrorMsg("sqlBindInt", "SQLFreeStmt",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlBindInt: SQLFreeStmt SQL_CLOSE:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
          freePrefetched(preparedStmt);
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        param = &preparedStmt->param_array[pos - 1];
        logMessage(printf("dataType: %s\n", nameOfSqlType(param->dataType)););
        switch (param->dataType) {
          case SQL_BIT:
            if (unlikely(value < 0 || value > 1)) {
              logError(printf("sqlBindInt: Parameter " FMT_D ": "
                              FMT_D " does not fit into a bit.\n",
                              pos, value));
              err_info = RANGE_ERROR;
            } else {
              c_type = SQL_C_BIT;
              *(char *) param->buffer = (char) value;
            } /* if */
            break;
          case SQL_TINYINT:
            if (preparedStmt->db->tinyintIsUnsigned) {
              if (unlikely(value < 0 || value > UINT8TYPE_MAX)) {
                logError(printf("sqlBindInt: Parameter " FMT_D ": "
                                FMT_D " does not fit into a 8-bit unsigned integer.\n",
                                pos, value));
                err_info = RANGE_ERROR;
              } else {
                c_type = SQL_C_UTINYINT;
                *(uint8Type *) param->buffer = (uint8Type) value;
              } /* if */
            } else {
              if (unlikely(value < INT8TYPE_MIN || value > INT8TYPE_MAX)) {
                logError(printf("sqlBindInt: Parameter " FMT_D ": "
                                FMT_D " does not fit into a 8-bit signed integer.\n",
                                pos, value));
                err_info = RANGE_ERROR;
              } else {
                c_type = SQL_C_STINYINT;
                *(int8Type *) param->buffer = (int8Type) value;
              } /* if */
            } /* if */
            break;
          case SQL_SMALLINT:
            if (unlikely(value < INT16TYPE_MIN || value > INT16TYPE_MAX)) {
              logError(printf("sqlBindInt: Parameter " FMT_D ": "
                              FMT_D " does not fit into a 16-bit integer.\n",
                              pos, value));
              err_info = RANGE_ERROR;
            } else {
              c_type = SQL_C_SSHORT;
              *(int16Type *) param->buffer = (int16Type) value;
            } /* if */
            break;
          case SQL_INTEGER:
            if (unlikely(value < INT32TYPE_MIN || value > INT32TYPE_MAX)) {
              logError(printf("sqlBindInt: Parameter " FMT_D ": "
                              FMT_D " does not fit into a 32-bit integer.\n",
                              pos, value));
              err_info = RANGE_ERROR;
            } else {
              c_type = SQL_C_SLONG;
              *(int32Type *) param->buffer = (int32Type) value;
            } /* if */
            break;
          case SQL_BIGINT:
            c_type = SQL_C_SBIGINT;
            *(int64Type *) param->buffer = value;
            break;
          case SQL_REAL:
            c_type = SQL_C_FLOAT;
            *(float *) param->buffer = (float) value;
            break;
          case SQL_FLOAT:
          case SQL_DOUBLE:
            c_type = SQL_C_DOUBLE;
            *(double *) param->buffer = (double) value;
            break;
          case SQL_DECIMAL:
          case SQL_NUMERIC:
          case SQL_CHAR:
          case SQL_VARCHAR:
          case SQL_LONGVARCHAR:
            c_type = SQL_C_SBIGINT;
            if (param->buffer_capacity < sizeof(int64Type)) {
              free(param->buffer);
              if (unlikely((param->buffer = malloc(sizeof(int64Type))) == NULL)) {
                param->buffer_capacity = 0;
                err_info = MEMORY_ERROR;
              } else {
                param->buffer_capacity = sizeof(int64Type);
              } /* if */
            } /* if */
            if (likely(err_info == OKAY_NO_ERROR)) {
              param->buffer_length = sizeof(int64Type);
              *(int64Type *) param->buffer = value;
            } /* if */
            break;
          default:
            logError(printf("sqlBindInt: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(param->dataType)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (unlikely(SQLBindParameter(preparedStmt->ppStmt,
                                        (SQLUSMALLINT) pos,
                                        SQL_PARAM_INPUT,
                                        c_type,
                                        param->dataType,
                                        param->paramSize,
                                        param->decimalDigits,
                                        param->buffer,
                                        (SQLLEN) param->buffer_length,
                                        NULL) != SQL_SUCCESS)) {
            setDbErrorMsg("sqlBindInt", "SQLBindParameter",
                          SQL_HANDLE_STMT, preparedStmt->ppStmt);
            logError(printf("sqlBindInt: SQLBindParameter:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } else {
            preparedStmt->fetchOkay = FALSE;
            param->bound = TRUE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* sqlBindInt */



static void sqlBindNull (sqlStmtType sqlStatement, intType pos)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindNull */
    logFunction(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindNull: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(SQLFreeStmt(preparedStmt->ppStmt, SQL_CLOSE) != SQL_SUCCESS)) {
          setDbErrorMsg("sqlBindNull", "SQLFreeStmt",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlBindNull: SQLFreeStmt SQL_CLOSE:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
          freePrefetched(preparedStmt);
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        param = &preparedStmt->param_array[pos - 1];
        param->length = SQL_NULL_DATA;
        if (unlikely(SQLBindParameter(preparedStmt->ppStmt,
                                      (SQLUSMALLINT) pos,
                                      SQL_PARAM_INPUT,
                                      SQL_C_CHAR,
                                      param->dataType,
                                      param->paramSize,
                                      param->decimalDigits,
                                      NULL,
                                      0,
                                      &param->length) != SQL_SUCCESS)) {
          setDbErrorMsg("sqlBindNull", "SQLBindParameter",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlBindNull: SQLBindParameter:\n%s\n",
                          dbError.message););
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->fetchOkay = FALSE;
          param->bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindNull */



static void sqlBindStri (sqlStmtType sqlStatement, intType pos,
    const const_striType stri)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    SQLSMALLINT c_type;
    SQLWCHAR *wstri;
    memSizeType length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindStri */
    logFunction(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, striAsUnquotedCStri(stri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindStri: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      err_info = RANGE_ERROR;
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(SQLFreeStmt(preparedStmt->ppStmt, SQL_CLOSE) != SQL_SUCCESS)) {
          setDbErrorMsg("sqlBindStri", "SQLFreeStmt",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlBindStri: SQLFreeStmt SQL_CLOSE:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
          freePrefetched(preparedStmt);
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        param = &preparedStmt->param_array[pos - 1];
        logMessage(printf("dataType: %s\n", nameOfSqlType(param->dataType)););
        switch (param->dataType) {
          case SQL_CHAR:
          case SQL_VARCHAR:
          case SQL_LONGVARCHAR:
          case SQL_WCHAR:
          case SQL_WVARCHAR:
          case SQL_WLONGVARCHAR:
          case SQL_CLOB:
            c_type = SQL_C_WCHAR;
            if (unlikely(stri->size > MAX_SQLWSTRI_LEN / SURROGATE_PAIR_FACTOR)) {
              /* It is not possible to compute the memory size. */
              err_info = MEMORY_ERROR;
            } else {
              if (param->buffer_capacity < SIZ_SQLWSTRI(SURROGATE_PAIR_FACTOR * stri->size)) {
                free(param->buffer);
                if (unlikely(!ALLOC_SQLWSTRI(param->buffer, SURROGATE_PAIR_FACTOR * stri->size))) {
                  param->buffer_capacity = 0;
                  err_info = MEMORY_ERROR;
                } else {
                  param->buffer_capacity = SIZ_SQLWSTRI(SURROGATE_PAIR_FACTOR * stri->size);
                } /* if */
              } /* if */
              if (likely(err_info == OKAY_NO_ERROR)) {
                wstri = (SQLWCHAR *) param->buffer;
                length = copy_to_sqlwstri(wstri, stri, &err_info);
                wstri[length] = '\0';
                if (likely(err_info == OKAY_NO_ERROR)) {
                  if (unlikely(length > SQLLEN_MAX / sizeof(SQLWCHAR))) {
                    /* It is not possible to cast length << 1 to SQLLEN. */
                    free(param->buffer);
                    param->buffer = NULL;
                    param->buffer_capacity = 0;
                    err_info = MEMORY_ERROR;
                  } else {
                    param->buffer_length = length * sizeof(SQLWCHAR);
                    param->length = (SQLLEN) (length * sizeof(SQLWCHAR));
                  } /* if */
                } /* if */
              } /* if */
            } /* if */
            break;
          default:
            logError(printf("sqlBindStri: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(param->dataType)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          if (unlikely(SQLBindParameter(preparedStmt->ppStmt,
                                        (SQLUSMALLINT) pos,
                                        SQL_PARAM_INPUT,
                                        c_type,
                                        param->dataType,
                                        param->paramSize,
                                        param->decimalDigits,
                                        param->buffer,
                                        (SQLLEN) param->buffer_length,
                                        &param->length) != SQL_SUCCESS)) {
            setDbErrorMsg("sqlBindStri", "SQLBindParameter",
                          SQL_HANDLE_STMT, preparedStmt->ppStmt);
            logError(printf("sqlBindStri: SQLBindParameter:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } else {
            preparedStmt->fetchOkay = FALSE;
            param->bound = TRUE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* sqlBindStri */



static void sqlBindTime (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second,
    intType time_zone)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    SQLSMALLINT c_type;
    SQL_DATE_STRUCT *dateValue;
    SQL_TIME_STRUCT *timeValue;
    SQL_TIMESTAMP_STRUCT *timestampValue;
    char *datetime2;
    intType fraction;
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
      err_info = RANGE_ERROR;
    } else if (unlikely(year < SHRT_MIN || year > SHRT_MAX ||
                        month < 1 || month > 12 ||
                        day < 1 || day > 31 ||
                        hour < 0 || hour >= 24 ||
                        minute < 0 || minute >= 60 ||
                        second < 0 || second >= 60 ||
                        micro_second < 0 || micro_second >= 1000000)) {
      logError(printf("sqlBindTime: Time not in allowed range.\n"););
      err_info = RANGE_ERROR;
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(SQLFreeStmt(preparedStmt->ppStmt, SQL_CLOSE) != SQL_SUCCESS)) {
          setDbErrorMsg("sqlBindTime", "SQLFreeStmt",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlBindTime: SQLFreeStmt SQL_CLOSE:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
          freePrefetched(preparedStmt);
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        param = &preparedStmt->param_array[pos - 1];
        logMessage(printf("dataType: %s\n", nameOfSqlType(param->dataType)););
        switch (param->dataType) {
          case SQL_TYPE_DATE:
            c_type = SQL_C_TYPE_DATE;
            dateValue = (SQL_DATE_STRUCT *) param->buffer;
            dateValue->year  = (SQLSMALLINT)  year;
            dateValue->month = (SQLUSMALLINT) month;
            dateValue->day   = (SQLUSMALLINT) day;
            break;
          case SQL_TYPE_TIME:
            c_type = SQL_C_TYPE_TIME;
            timeValue = (SQL_TIME_STRUCT *) param->buffer;
            timeValue->hour   = (SQLUSMALLINT) hour;
            timeValue->minute = (SQLUSMALLINT) minute;
            timeValue->second = (SQLUSMALLINT) second;
            break;
          case SQL_DATETIME:
          case SQL_TYPE_TIMESTAMP:
            c_type = SQL_C_TYPE_TIMESTAMP;
            switch (param->decimalDigits) {
              case 0:  fraction = 0; break;
              case 1:  fraction = micro_second / 100000 * 100000000; break;
              case 2:  fraction = micro_second /  10000 *  10000000; break;
              case 3:  fraction = micro_second /   1000 *   1000000; break;
              case 4:  fraction = micro_second /    100 *    100000; break;
              case 5:  fraction = micro_second /     10 *     10000; break;
              default: fraction = micro_second * 1000; break;
            } /* switch */
            timestampValue = (SQL_TIMESTAMP_STRUCT *) param->buffer;
            timestampValue->year     = (SQLSMALLINT)  year;
            timestampValue->month    = (SQLUSMALLINT) month;
            timestampValue->day      = (SQLUSMALLINT) day;
            timestampValue->hour     = (SQLUSMALLINT) hour;
            timestampValue->minute   = (SQLUSMALLINT) minute;
            timestampValue->second   = (SQLUSMALLINT) second;
            timestampValue->fraction = (SQLUINTEGER)  fraction;
            /* printf("fraction: %lu\n", (unsigned long) timestampValue->fraction); */
            break;
          case SQL_WVARCHAR:
          case SQL_VARCHAR:
          case SQL_LONGVARCHAR:
            c_type = SQL_C_CHAR;
            if (param->buffer_capacity < SIZ_CSTRI(MAX_DATETIME2_LENGTH)) {
              free(param->buffer);
              if (unlikely((param->buffer = malloc(
                                SIZ_CSTRI(MAX_DATETIME2_LENGTH))) == NULL)) {
                param->buffer_capacity = 0;
                err_info = MEMORY_ERROR;
              } else {
                param->buffer_capacity = SIZ_CSTRI(MAX_DATETIME2_LENGTH);
              } /* if */
            } /* if */
            if (unlikely(year < -999 || year > 9999)) {
              logError(printf("sqlBindTime: Year not in allowed range.\n"););
              err_info = RANGE_ERROR;
            } /* if */
            if (likely(err_info == OKAY_NO_ERROR)) {
              datetime2 = (char *) param->buffer;
              /* printf("paramSize: " FMT_U_MEM "\n", param->paramSize); */
              if (param->paramSize == 10 && hour == 0 && minute == 0 &&
                  second == 0 && micro_second == 0 &&
                  (year != 0 || month != 1 || day != 1)) {
                sprintf(datetime2, F_D(04) "-" F_D(02) "-" F_D(02),
                        year, month, day);
                param->buffer_length = 10;
              } else if (param->paramSize >= 8 &&
                         year == 0 && month == 1 && day == 1) {
                sprintf(datetime2, F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(07),
                        hour, minute, second, micro_second * 10);
                param->buffer_length = 16;
              } else if (param->paramSize >= 19) {
                sprintf(datetime2, F_D(04) "-" F_D(02) "-" F_D(02) " "
                                   F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(07),
                        year, month, day,
                        hour, minute, second, micro_second * 10);
                param->buffer_length = 27;
                if (param->paramSize < param->buffer_length) {
                  datetime2[param->paramSize] = '\0';
                  param->buffer_length = (memSizeType) param->paramSize;
                } /* if */
              } else {
                err_info = RANGE_ERROR;
              } /* if */
              /* printf("datetime2: %s\n", datetime2); */
              /* printf("buffer_length: " FMT_U_MEM "\n", param->buffer_length); */
            } /* if */
            break;
          default:
            logError(printf("sqlBindTime: Parameter " FMT_D " has the unknown type %s.\n",
                            pos, nameOfSqlType(param->dataType)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (likely(err_info == OKAY_NO_ERROR)) {
          /* printf("paramSize: " FMT_U_MEM "\n", (long) param->paramSize); */
          /* printf("decimalDigits: " FMT_D16 "\n", param->decimalDigits); */
          if (unlikely(SQLBindParameter(preparedStmt->ppStmt,
                                        (SQLUSMALLINT) pos,
                                        SQL_PARAM_INPUT,
                                        c_type,
                                        param->dataType,
                                        param->paramSize,
                                        param->decimalDigits,
                                        param->buffer,
                                        (SQLLEN) param->buffer_length,
                                        NULL) != SQL_SUCCESS)) {
            setDbErrorMsg("sqlBindTime", "SQLBindParameter",
                          SQL_HANDLE_STMT, preparedStmt->ppStmt);
            logError(printf("sqlBindTime: SQLBindParameter:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } else {
            preparedStmt->fetchOkay = FALSE;
            param->bound = TRUE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* sqlBindTime */



static void sqlClose (databaseType database)

  {
    dbType db;

  /* sqlClose */
    logFunction(printf("sqlClose(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (db->connection != SQL_NULL_HANDLE) {
      SQLDisconnect(db->connection);
      SQLFreeHandle(SQL_HANDLE_DBC, db->connection);
      db->connection = SQL_NULL_HANDLE;
    } /* if */
    if (db->sql_environment != SQL_NULL_HANDLE) {
      SQLFreeHandle(SQL_HANDLE_ENV, db->sql_environment);
      db->sql_environment = SQL_NULL_HANDLE;
    } /* if */
    db->isOpen = FALSE;
    logFunction(printf("sqlClose -->\n"););
  } /* sqlClose */



static bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    resultDescrType columnDescr;
    resultDataType columnData;
    bigIntType columnValue;

  /* sqlColumnBigInt */
    logFunction(printf("sqlColumnBigInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnBigInt: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      columnDescr = &preparedStmt->result_descr_array[column - 1];
      columnData = &preparedStmt->currentFetch->result_array[column - 1];
      if (columnData->length == SQL_NULL_DATA) {
        logMessage(printf("Column is NULL -> Use default value: 0\n"););
        columnValue = bigZero();
      } else if (unlikely(columnData->length < 0)) {
        dbInconsistent("sqlColumnBigInt", "SQLBindCol");
        logError(printf("sqlColumnBigInt: Column " FMT_D ": "
                        "Negative length: " FMT_D_LEN "\n",
                        column, columnData->length););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else if (unlikely(!columnDescr->sql_data_at_exec &&
          (SQLULEN) columnData->length > columnDescr->buffer_length)) {
        dbInconsistent("sqlColumnBigInt", "SQLBindCol");
        logError(printf("sqlColumnBigInt: Column " FMT_D ": Length (" FMT_D_LEN
                        ") greater than buffer_length (" FMT_U_MEM ").\n",
                        column, columnData->length,
                        columnDescr->buffer_length););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else {
        logMessage(printf("length: " FMT_D_LEN "\n", columnData->length););
        logMessage(printf("dataType: %s\n", nameOfSqlType(columnDescr->dataType)););
        switch (columnDescr->dataType) {
          case SQL_BIT:
            columnValue = bigFromInt32((int32Type)
                (*(char *) columnData->buffer) != 0);
            break;
          case SQL_TINYINT:
            /* SQL_TINYINT can be signed or unsigned. */
            /* We use a c_type of SQL_C_SSHORT to be on the safe side. */
          case SQL_SMALLINT:
            columnValue = bigFromInt32((int32Type)
                *(int16Type *) columnData->buffer);
            break;
          case SQL_INTEGER:
            columnValue = bigFromInt32(
                *(int32Type *) columnData->buffer);
            break;
          case SQL_BIGINT:
            columnValue = bigFromInt64(
                *(int64Type *) columnData->buffer);
            break;
          case SQL_DECIMAL:
            columnValue = getDecimalBigInt(
                (const_ustriType) columnData->buffer,
                (memSizeType) columnData->length);
            break;
          case SQL_NUMERIC:
            columnValue = getBigInt(columnData->buffer,
                (memSizeType) columnData->length);
            break;
          default:
            logError(printf("sqlColumnBigInt: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(columnDescr->dataType)););
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
    resultDescrType columnDescr;
    resultDataType columnData;
    float floatValue;
    double doubleValue;

  /* sqlColumnBigRat */
    logFunction(printf("sqlColumnBigRat(" FMT_U_MEM ", " FMT_D ", *, *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnBigRat: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
    } else {
      columnDescr = &preparedStmt->result_descr_array[column - 1];
      columnData = &preparedStmt->currentFetch->result_array[column - 1];
      if (columnData->length == SQL_NULL_DATA) {
        logMessage(printf("Column is NULL -> Use default value: 0\n"););
        *numerator = bigZero();
        *denominator = bigFromInt32(1);
      } else if (unlikely(columnData->length < 0)) {
        dbInconsistent("sqlColumnBigRat", "SQLBindCol");
        logError(printf("sqlColumnBigRat: Column " FMT_D ": "
                        "Negative length: " FMT_D_LEN "\n",
                        column, columnData->length););
        raise_error(DATABASE_ERROR);
      } else if (unlikely(!columnDescr->sql_data_at_exec &&
          (SQLULEN) columnData->length > columnDescr->buffer_length)) {
        dbInconsistent("sqlColumnBigRat", "SQLBindCol");
        logError(printf("sqlColumnBigRat: Column " FMT_D ": Length (" FMT_D_LEN
                        ") greater than buffer_length (" FMT_U_MEM ").\n",
                        column, columnData->length,
                        columnDescr->buffer_length););
        raise_error(DATABASE_ERROR);
      } else {
        logMessage(printf("length: " FMT_D_LEN "\n", columnData->length););
        logMessage(printf("dataType: %s\n", nameOfSqlType(columnDescr->dataType)););
        switch (columnDescr->dataType) {
          case SQL_BIT:
            *numerator = bigFromInt32((int32Type)
                (*(char *) columnData->buffer) != 0);
            *denominator = bigFromInt32(1);
            break;
          case SQL_TINYINT:
            /* SQL_TINYINT can be signed or unsigned. */
            /* We use a c_type of SQL_C_SSHORT to be on the safe side. */
          case SQL_SMALLINT:
            *numerator = bigFromInt32((int32Type)
                *(int16Type *) columnData->buffer);
            *denominator = bigFromInt32(1);
            break;
          case SQL_INTEGER:
            *numerator = bigFromInt32(
                *(int32Type *) columnData->buffer);
            *denominator = bigFromInt32(1);
            break;
          case SQL_BIGINT:
            *numerator = bigFromInt64(
                *(int64Type *) columnData->buffer);
            *denominator = bigFromInt32(1);
            break;
          case SQL_REAL:
            floatValue = *(float *) columnData->buffer;
            /* printf("sqlColumnBigRat: float: %f\n", floatValue); */
            *numerator = roundDoubleToBigRat(floatValue, FALSE, denominator);
            break;
          case SQL_FLOAT:
          case SQL_DOUBLE:
            doubleValue = *(double *) columnData->buffer;
            /* printf("sqlColumnBigRat: double: %f\n", doubleValue); */
            *numerator = roundDoubleToBigRat(doubleValue, TRUE, denominator);
            break;
          case SQL_DECIMAL:
            *numerator = getDecimalBigRational((const_ustriType) columnData->buffer,
                (memSizeType) columnData->length, denominator);
            break;
          case SQL_NUMERIC:
            *numerator = getBigRational(columnData->buffer,
                (memSizeType) columnData->length, denominator);
            break;
          default:
            logError(printf("sqlColumnBigRat: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(columnDescr->dataType)););
            raise_error(RANGE_ERROR);
            break;
        } /* switch */
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
    resultDescrType columnDescr;
    resultDataType columnData;
    intType columnValue;

  /* sqlColumnBool */
    logFunction(printf("sqlColumnBool(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnBool: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      columnDescr = &preparedStmt->result_descr_array[column - 1];
      columnData = &preparedStmt->currentFetch->result_array[column - 1];
      if (columnData->length == SQL_NULL_DATA) {
        logMessage(printf("Column is NULL -> Use default value: FALSE\n"););
        columnValue = 0;
      } else if (unlikely(columnData->length < 0)) {
        dbInconsistent("sqlColumnBool", "SQLBindCol");
        logError(printf("sqlColumnBool: Column " FMT_D ": "
                        "Negative length: " FMT_D_LEN "\n",
                        column, columnData->length););
        raise_error(DATABASE_ERROR);
        columnValue = 0;
      } else {
        logMessage(printf("length: " FMT_D_LEN "\n", columnData->length););
        logMessage(printf("dataType: %s\n", nameOfSqlType(columnDescr->dataType)););
        logMessage(printf("c_type: %s\n", nameOfCType(columnDescr->c_type)););
        switch (columnDescr->dataType) {
          case SQL_CHAR:
          case SQL_VARCHAR:
          case SQL_LONGVARCHAR:
          case SQL_WCHAR:
          case SQL_WVARCHAR:
          case SQL_WLONGVARCHAR:
            switch (columnDescr->c_type) {
              case SQL_C_CHAR:
                if (unlikely(columnData->length != 1)) {
                  logError(printf("sqlColumnBool: Column " FMT_D ": "
                                  "The size of a boolean field must be 1.\n", column););
                  raise_error(RANGE_ERROR);
                  columnValue = 0;
                } else {
                  columnValue = *(const_cstriType) columnData->buffer - '0';
                } /* if */
                break;
              case SQL_C_WCHAR:
                if (unlikely(columnData->length != 2)) {
                  logError(printf("sqlColumnBool: Column " FMT_D ": "
                                  "The size of a boolean field must be 1.\n", column););
                  raise_error(RANGE_ERROR);
                  columnValue = 0;
                } else {
                  columnValue = *(const SQLWCHAR *) columnData->buffer - '0';
                } /* if */
                break;
              default:
                logError(printf("sqlColumnBool: Column " FMT_D " has the unknown C type %s.\n",
                                column, nameOfCType(columnDescr->c_type)););
                raise_error(RANGE_ERROR);
                columnValue = 0;
                break;
            } /* switch */
            break;
          case SQL_BIT:
            columnValue = *(char *) columnData->buffer;
            break;
          case SQL_TINYINT:
            /* SQL_TINYINT can be signed or unsigned. */
            /* We use a c_type of SQL_C_SSHORT to be on the safe side. */
          case SQL_SMALLINT:
            columnValue = *(int16Type *) columnData->buffer;
            break;
          case SQL_INTEGER:
            columnValue = *(int32Type *) columnData->buffer;
            break;
          case SQL_BIGINT:
            columnValue = *(int64Type *) columnData->buffer;
            break;
          case SQL_DECIMAL:
            columnValue = getDecimalInt(
                (const_ustriType) columnData->buffer,
                (memSizeType) columnData->length);
            break;
          case SQL_NUMERIC:
            columnValue = getInt(
                columnData->buffer,
                (memSizeType) columnData->length);
            break;
          default:
            logError(printf("sqlColumnBool: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(columnDescr->dataType)););
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
    resultDescrType columnDescr;
    resultDataType columnData;
    memSizeType length;
    bstriType columnValue;

  /* sqlColumnBStri */
    logFunction(printf("sqlColumnBStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnBStri: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      columnDescr = &preparedStmt->result_descr_array[column - 1];
      columnData = &preparedStmt->currentFetch->result_array[column - 1];
      if (columnData->length == SQL_NULL_DATA) {
        logMessage(printf("Column is NULL -> Use default value: \"\"\n"););
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(columnValue, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          columnValue->size = 0;
        } /* if */
      } else if (unlikely(columnData->length < 0)) {
        dbInconsistent("sqlColumnBStri", "SQLBindCol");
        logError(printf("sqlColumnBStri: Column " FMT_D ": "
                        "Negative length: " FMT_D_LEN "\n",
                        column, columnData->length););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else if (unlikely(!columnDescr->sql_data_at_exec &&
          (SQLULEN) columnData->length > columnDescr->buffer_length)) {
        dbInconsistent("sqlColumnBStri", "SQLBindCol");
        logError(printf("sqlColumnBStri: Column " FMT_D ": Length (" FMT_D_LEN
                        ") greater than buffer_length (" FMT_U_MEM ").\n",
                        column, columnData->length,
                        columnDescr->buffer_length););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else {
        logMessage(printf("length: " FMT_D_LEN "\n", columnData->length););
        logMessage(printf("dataType: %s\n", nameOfSqlType(columnDescr->dataType)););
        switch (columnDescr->dataType) {
          case SQL_BINARY:
          case SQL_VARBINARY:
          case SQL_LONGVARBINARY:
            /* This might be a blob, which is filled by fetchBlobs(). */
            length = (memSizeType) columnData->length;
            if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(columnValue, length))) {
              raise_error(MEMORY_ERROR);
            } else {
              columnValue->size = length;
              memcpy(columnValue->mem,
                     (ustriType) columnData->buffer,
                     length);
            } /* if */
            break;
          default:
            logError(printf("sqlColumnBStri: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(columnDescr->dataType)););
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
    resultDescrType columnDescr;
    resultDataType columnData;
    SQL_INTERVAL_STRUCT *interval;
    memSizeType length;
    char duration[MAX_DURATION_LENGTH + NULL_TERMINATION_LEN];
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlColumnDuration */
    logFunction(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnDuration: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
    } else {
      columnDescr = &preparedStmt->result_descr_array[column - 1];
      columnData = &preparedStmt->currentFetch->result_array[column - 1];
      if (columnData->length == SQL_NULL_DATA) {
        logMessage(printf("Column is NULL -> Use default value: P0D\n"););
        *year         = 0;
        *month        = 0;
        *day          = 0;
        *hour         = 0;
        *minute       = 0;
        *second       = 0;
        *micro_second = 0;
      } else if (unlikely(columnData->length < 0)) {
        dbInconsistent("sqlColumnDuration", "SQLBindCol");
        logError(printf("sqlColumnDuration: Column " FMT_D ": "
                        "Negative length: " FMT_D_LEN "\n",
                        column, columnData->length););
        raise_error(DATABASE_ERROR);
      } else {
        logMessage(printf("length: " FMT_D_LEN "\n", columnData->length););
        logMessage(printf("dataType: %s\n", nameOfSqlType(columnDescr->dataType)););
        logMessage(printf("c_type: %s\n", nameOfCType(columnDescr->c_type)););
        switch (columnDescr->dataType) {
          case SQL_INTERVAL_YEAR:
          case SQL_INTERVAL_MONTH:
          case SQL_INTERVAL_DAY:
          case SQL_INTERVAL_HOUR:
          case SQL_INTERVAL_MINUTE:
          case SQL_INTERVAL_SECOND:
          case SQL_INTERVAL_YEAR_TO_MONTH:
          case SQL_INTERVAL_DAY_TO_HOUR:
          case SQL_INTERVAL_DAY_TO_MINUTE:
          case SQL_INTERVAL_DAY_TO_SECOND:
          case SQL_INTERVAL_HOUR_TO_MINUTE:
          case SQL_INTERVAL_HOUR_TO_SECOND:
          case SQL_INTERVAL_MINUTE_TO_SECOND:
            *year         = 0;
            *month        = 0;
            *day          = 0;
            *hour         = 0;
            *minute       = 0;
            *second       = 0;
            *micro_second = 0;
            interval = (SQL_INTERVAL_STRUCT *) columnData->buffer;
            /* printf("interval_type: %d\n", interval->interval_type); */
            switch (interval->interval_type) {
              case SQL_IS_YEAR:
                *year = interval->intval.year_month.year;
                break;
              case SQL_IS_MONTH:
                *month = interval->intval.year_month.month;
                break;
              case SQL_IS_DAY:
                *day = interval->intval.day_second.day;
                break;
              case SQL_IS_HOUR:
                *hour = interval->intval.day_second.hour;
                break;
              case SQL_IS_MINUTE:
                *minute = interval->intval.day_second.minute;
                break;
              case SQL_IS_SECOND:
                *second = interval->intval.day_second.second;
                break;
              case SQL_IS_YEAR_TO_MONTH:
                *year = interval->intval.year_month.year;
                *month = interval->intval.year_month.month;
                break;
              case SQL_IS_DAY_TO_HOUR:
                *day = interval->intval.day_second.day;
                *hour = interval->intval.day_second.hour;
                break;
              case SQL_IS_DAY_TO_MINUTE:
                *day = interval->intval.day_second.day;
                *hour = interval->intval.day_second.hour;
                *minute = interval->intval.day_second.minute;
                break;
              case SQL_IS_DAY_TO_SECOND:
                *day = interval->intval.day_second.day;
                *hour = interval->intval.day_second.hour;
                *minute = interval->intval.day_second.minute;
                *second = interval->intval.day_second.second;
                break;
              case SQL_IS_HOUR_TO_MINUTE:
                *hour = interval->intval.day_second.hour;
                *minute = interval->intval.day_second.minute;
                break;
              case SQL_IS_HOUR_TO_SECOND:
                *hour = interval->intval.day_second.hour;
                *minute = interval->intval.day_second.minute;
                *second = interval->intval.day_second.second;
                break;
              case SQL_IS_MINUTE_TO_SECOND:
                *minute = interval->intval.day_second.minute;
                *second = interval->intval.day_second.second;
                break;
            } /* switch */
            if (interval->interval_sign == SQL_TRUE) {
              *year         = -*year;
              *month        = -*month;
              *day          = -*day;
              *hour         = -*hour;
              *minute       = -*minute;
              *second       = -*second;
              *micro_second = -*micro_second;
            } /* if */
            break;
          case SQL_VARCHAR:
          case SQL_WVARCHAR:
            switch (columnDescr->c_type) {
              case SQL_C_CHAR:
                length = (memSizeType) columnData->length;
                if (unlikely(length > MAX_DURATION_LENGTH)) {
                  logError(printf("sqlColumnDuration: In column " FMT_D
                                  " the duration length of " FMT_U_MEM " is too long.\n",
                                  column, length););
                  err_info = RANGE_ERROR;
                } else {
                  memcpy(duration, columnData->buffer, length);
                  duration[length] = '\0';
                } /* if */
                break;
              case SQL_C_WCHAR:
                length = (memSizeType) columnData->length / sizeof(SQLWCHAR);
                if (unlikely(length > MAX_DURATION_LENGTH)) {
                  logError(printf("sqlColumnDuration: In column " FMT_D
                                  " the duration length of " FMT_U_MEM " is too long.\n",
                                  column, length););
                  err_info = RANGE_ERROR;
                } else {
                  err_info = conv_sqlwstri_to_cstri(duration,
                      (SQLWCHAR *) columnData->buffer, length);
                  if (unlikely(err_info != OKAY_NO_ERROR)) {
                    logError(printf("sqlColumnDuration: In column " FMT_D
                                    " the duration contains characters beyond Latin-1.\n",
                                    column););
                  } /* if */
                } /* if */
                break;
              default:
                logError(printf("sqlColumnDuration: Column " FMT_D " has the unknown C type %s.\n",
                                column, nameOfCType(columnDescr->c_type)););
                err_info =  RANGE_ERROR;
                break;
            } /* switch */
            if (likely(err_info == OKAY_NO_ERROR)) {
              /* printf("sqlColumnDuration: Duration = \"%s\"\n", duration); */
              if (length >= 9 && memcmp(duration, "INTERVAL ", 9) == 0) {
                if (unlikely(!assignInterval(&duration[9], year, month, day,
                                             hour, minute, second, micro_second))) {
                  logError(printf("sqlColumnDuration: Column " FMT_D ": "
                                  "Failed to recognize interval: %s\n",
                                  column, &duration[9]););
                  err_info = RANGE_ERROR;
                } /* if */
              } else if (length == 8 && duration[2] == ':' && duration[5] == ':') {
                if (unlikely(sscanf(duration,
                                    F_U(02) ":" F_U(02) ":" F_U(02),
                                    hour, minute, second) != 3)) {
                  err_info = RANGE_ERROR;
                } else {
                  *year         = 0;
                  *month        = 0;
                  *day          = 0;
                  *micro_second = 0;
                } /* if */
              } else if (length == 9 && duration[0] == '-' &&
                         duration[3] == ':' && duration[6] == ':') {
                if (unlikely(sscanf(&duration[1],
                                    F_U(02) ":" F_U(02) ":" F_U(02),
                                    hour, minute, second) != 3)) {
                  err_info = RANGE_ERROR;
                } else {
                  *year         = 0;
                  *month        = 0;
                  *day          = 0;
                  *hour         = -*hour;
                  *minute       = -*minute;
                  *second       = -*second;
                  *micro_second = 0;
                } /* if */
              } else {
                logError(printf("sqlColumnDuration: In column " FMT_D
                                " the duration \"%s\" is unrecognized.\n",
                                column, duration););
                err_info = RANGE_ERROR;
              } /* if */
            } /* if */
            break;
          default:
            logError(printf("sqlColumnDuration: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(columnDescr->dataType)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } /* if */
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
    resultDescrType columnDescr;
    resultDataType columnData;
    floatType columnValue;

  /* sqlColumnFloat */
    logFunction(printf("sqlColumnFloat(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnFloat: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnValue = 0.0;
    } else {
      columnDescr = &preparedStmt->result_descr_array[column - 1];
      columnData = &preparedStmt->currentFetch->result_array[column - 1];
      if (columnData->length == SQL_NULL_DATA) {
        logMessage(printf("Column is NULL -> Use default value: 0.0\n"););
        columnValue = 0.0;
      } else if (unlikely(columnData->length < 0)) {
        dbInconsistent("sqlColumnFloat", "SQLBindCol");
        logError(printf("sqlColumnFloat: Column " FMT_D ": "
                        "Negative length: " FMT_D_LEN "\n",
                        column, columnData->length););
        raise_error(DATABASE_ERROR);
        columnValue = 0.0;
      } else if (unlikely(!columnDescr->sql_data_at_exec &&
          (SQLULEN) columnData->length > columnDescr->buffer_length)) {
        dbInconsistent("sqlColumnFloat", "SQLBindCol");
        logError(printf("sqlColumnFloat: Column " FMT_D ": Length (" FMT_D_LEN
                        ") greater than buffer_length (" FMT_U_MEM ").\n",
                        column, columnData->length,
                        columnDescr->buffer_length););
        raise_error(DATABASE_ERROR);
        columnValue = 0.0;
      } else {
        logMessage(printf("length: " FMT_D_LEN "\n", columnData->length););
        logMessage(printf("dataType: %s\n", nameOfSqlType(columnDescr->dataType)););
        switch (columnDescr->dataType) {
          case SQL_BIT:
            columnValue = (floatType) *(char *) columnData->buffer != 0;
            break;
          case SQL_TINYINT:
            /* SQL_TINYINT can be signed or unsigned. */
            /* We use a c_type of SQL_C_SSHORT to be on the safe side. */
          case SQL_SMALLINT:
            columnValue = (floatType) *(int16Type *) columnData->buffer;
            break;
          case SQL_INTEGER:
            columnValue = (floatType) *(int32Type *) columnData->buffer;
            break;
          case SQL_BIGINT:
            columnValue = (floatType) *(int64Type *) columnData->buffer;
            break;
          case SQL_REAL:
            columnValue = *(float *) columnData->buffer;
            /* printf("sqlColumnFloat: float: %f\n", columnValue); */
            break;
          case SQL_FLOAT:
          case SQL_DOUBLE:
            columnValue = *(double *) columnData->buffer;
            /* printf("sqlColumnFloat: double: %f\n", columnValue); */
            break;
          case SQL_DECIMAL:
            columnValue = getDecimalFloat(
                (const_ustriType) columnData->buffer,
                (memSizeType) columnData->length);
            break;
          case SQL_NUMERIC:
            columnValue = getFloat(
                columnData->buffer,
                (memSizeType) columnData->length);
            break;
          default:
            logError(printf("sqlColumnFloat: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(columnDescr->dataType)););
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
    resultDescrType columnDescr;
    resultDataType columnData;
    intType columnValue;

  /* sqlColumnInt */
    logFunction(printf("sqlColumnInt(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnInt: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnValue = 0;
    } else {
      columnDescr = &preparedStmt->result_descr_array[column - 1];
      columnData = &preparedStmt->currentFetch->result_array[column - 1];
      if (columnData->length == SQL_NULL_DATA) {
        logMessage(printf("Column is NULL -> Use default value: 0\n"););
        columnValue = 0;
      } else if (unlikely(columnData->length < 0)) {
        dbInconsistent("sqlColumnInt", "SQLBindCol");
        logError(printf("sqlColumnInt: Column " FMT_D ": "
                        "Negative length: " FMT_D_LEN "\n",
                        column, columnData->length););
        raise_error(DATABASE_ERROR);
        columnValue = 0;
      } else {
        logMessage(printf("length: " FMT_D_LEN "\n", columnData->length););
        logMessage(printf("dataType: %s\n", nameOfSqlType(columnDescr->dataType)););
        switch (columnDescr->dataType) {
          case SQL_BIT:
            columnValue = *(char *) columnData->buffer != 0;
            break;
          case SQL_TINYINT:
            /* SQL_TINYINT can be signed or unsigned. */
            /* We use a c_type of SQL_C_SSHORT to be on the safe side. */
          case SQL_SMALLINT:
            columnValue = *(int16Type *) columnData->buffer;
            break;
          case SQL_INTEGER:
            columnValue = *(int32Type *) columnData->buffer;
            break;
          case SQL_BIGINT:
            columnValue = *(int64Type *) columnData->buffer;
            break;
          case SQL_DECIMAL:
            columnValue = getDecimalInt(
                (const_ustriType) columnData->buffer,
                (memSizeType) columnData->length);
            break;
          case SQL_NUMERIC:
            columnValue = getInt(
                columnData->buffer,
                (memSizeType) columnData->length);
            break;
          default:
            logError(printf("sqlColumnInt: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(columnDescr->dataType)););
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
    resultDescrType columnDescr;
    resultDataType columnData;
    memSizeType length;
    SQLWCHAR *wstri;
    cstriType cstri;
    errInfoType err_info = OKAY_NO_ERROR;
    striType columnValue;

  /* sqlColumnStri */
    logFunction(printf("sqlColumnStri(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnStri: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnValue = NULL;
    } else {
      columnDescr = &preparedStmt->result_descr_array[column - 1];
      columnData = &preparedStmt->currentFetch->result_array[column - 1];
      if (columnData->length == SQL_NULL_DATA) {
        logMessage(printf("Column is NULL -> Use default value: \"\"\n"););
        columnValue = strEmpty();
      } else if (unlikely(columnData->length < 0)) {
        dbInconsistent("sqlColumnStri", "SQLBindCol");
        logError(printf("sqlColumnStri: Column " FMT_D ": "
                        "Negative length: " FMT_D_LEN "\n",
                        column, columnData->length););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else if (unlikely(!columnDescr->sql_data_at_exec &&
          (SQLULEN) columnData->length > columnDescr->buffer_length)) {
        dbInconsistent("sqlColumnStri", "SQLBindCol");
        logError(printf("sqlColumnStri: Column " FMT_D ": Length (" FMT_D_LEN
                        ") greater than buffer_length (" FMT_U_MEM ").\n",
                        column, columnData->length,
                        columnDescr->buffer_length););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else {
        logMessage(printf("length: " FMT_D_LEN "\n", columnData->length););
        logMessage(printf("dataType: %s\n", nameOfSqlType(columnDescr->dataType)););
        logMessage(printf("c_type: %s\n", nameOfCType(columnDescr->c_type)););
        switch (columnDescr->dataType) {
          case SQL_VARCHAR:
          case SQL_LONGVARCHAR:
          case SQL_WVARCHAR:
          case SQL_WLONGVARCHAR:
            switch (columnDescr->c_type) {
              case SQL_C_CHAR:
                columnValue = cstri_buf_to_stri(
                    (cstriType) columnData->buffer,
                    (memSizeType) columnData->length);
                if (unlikely(columnValue == NULL)) {
                  raise_error(MEMORY_ERROR);
                } /* if */
                break;
              case SQL_C_WCHAR:
                length = (memSizeType) columnData->length / sizeof(SQLWCHAR);
                columnValue = sqlwstri_to_stri(
                    (SQLWCHAR *) columnData->buffer,
                    length, &err_info);
                if (unlikely(columnValue == NULL)) {
                  raise_error(err_info);
                } /* if */
                break;
              default:
                logError(printf("sqlColumnStri: Column " FMT_D " has the unknown C type %s.\n",
                                column, nameOfCType(columnDescr->c_type)););
                raise_error(RANGE_ERROR);
                columnValue = NULL;
                break;
            } /* switch */
            break;
          case SQL_CHAR:
          case SQL_WCHAR:
            switch (columnDescr->c_type) {
              case SQL_C_CHAR:
                length = (memSizeType) columnData->length;
                cstri = (cstriType) columnData->buffer;
                while (length > 0 && cstri[length - 1] == ' ') {
                  length--;
                } /* if */
                columnValue = cstri_buf_to_stri(cstri, length);
                if (unlikely(columnValue == NULL)) {
                  raise_error(MEMORY_ERROR);
                } /* if */
                break;
              case SQL_C_WCHAR:
                length = (memSizeType) columnData->length / sizeof(SQLWCHAR);
                wstri = (SQLWCHAR *) columnData->buffer;
                while (length > 0 && wstri[length - 1] == ' ') {
                  length--;
                } /* if */
                columnValue = sqlwstri_to_stri(wstri, length, &err_info);
                if (unlikely(columnValue == NULL)) {
                  raise_error(err_info);
                } /* if */
                break;
              default:
                logError(printf("sqlColumnStri: Column " FMT_D " has the unknown C type %s.\n",
                                column, nameOfCType(columnDescr->c_type)););
                raise_error(RANGE_ERROR);
                columnValue = NULL;
                break;
            } /* switch */
            break;
          case SQL_BINARY:
          case SQL_VARBINARY:
          case SQL_LONGVARBINARY:
            /* This might be a blob, which is filled by fetchBlobs(). */
            columnValue = cstri_buf_to_stri(
                (cstriType) columnData->buffer,
                (memSizeType) columnData->length);
            if (unlikely(columnValue == NULL)) {
              raise_error(MEMORY_ERROR);
            } /* if */
            break;
          default:
            logError(printf("sqlColumnStri: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(columnDescr->dataType)););
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
    resultDescrType columnDescr;
    resultDataType columnData;
    SQL_DATE_STRUCT *dateValue;
    SQL_TIME_STRUCT *timeValue;
    SQL_TIMESTAMP_STRUCT *timestampValue;
    sqlSsTime2Struct *time2Value;
    memSizeType length;
    char datetime2[MAX_DATETIME2_LENGTH + NULL_TERMINATION_LEN];
    boolType okay;
    boolType isTime;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlColumnTime */
    logFunction(printf("sqlColumnTime(" FMT_U_MEM ", " FMT_D ", *)\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->fetchOkay || column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlColumnTime: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
    } else {
      columnDescr = &preparedStmt->result_descr_array[column - 1];
      columnData = &preparedStmt->currentFetch->result_array[column - 1];
      if (columnData->length == SQL_NULL_DATA) {
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
      } else if (unlikely(columnData->length < 0)) {
        dbInconsistent("sqlColumnTime", "SQLBindCol");
        logError(printf("sqlColumnTime: Column " FMT_D ": "
                        "Negative length: " FMT_D_LEN "\n",
                        column, columnData->length););
        raise_error(DATABASE_ERROR);
      } else {
        logMessage(printf("length: " FMT_D_LEN "\n", columnData->length););
        logMessage(printf("dataType: %s\n", nameOfSqlType(columnDescr->dataType)););
        switch (columnDescr->dataType) {
          case SQL_TYPE_DATE:
            dateValue = (SQL_DATE_STRUCT *) columnData->buffer;
            *year         = dateValue->year;
            *month        = dateValue->month;
            *day          = dateValue->day;
            *hour         = 0;
            *minute       = 0;
            *second       = 0;
            *micro_second = 0;
            timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                          time_zone, is_dst);
            break;
          case SQL_TYPE_TIME:
            timeValue = (SQL_TIME_STRUCT *) columnData->buffer;
            *year         = 2000;
            *month        = 1;
            *day          = 1;
            *hour         = timeValue->hour;
            *minute       = timeValue->minute;
            *second       = timeValue->second;
            *micro_second = 0;
            timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                          time_zone, is_dst);
            *year = 0;
            break;
          case SQL_DATETIME:
          case SQL_TYPE_TIMESTAMP:
            timestampValue = (SQL_TIMESTAMP_STRUCT *) columnData->buffer;
            *year         = timestampValue->year;
            *month        = timestampValue->month;
            *day          = timestampValue->day;
            *hour         = timestampValue->hour;
            *minute       = timestampValue->minute;
            *second       = timestampValue->second;
            *micro_second = timestampValue->fraction / 1000;
            timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                          time_zone, is_dst);
            break;
          case SQL_SS_TIME2:
            time2Value = (sqlSsTime2Struct *) columnData->buffer;
            *year         = 2000;
            *month        = 1;
            *day          = 1;
            *hour         = time2Value->hour;
            *minute       = time2Value->minute;
            *second       = time2Value->second;
            *micro_second = time2Value->fraction / 1000;
            timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                          time_zone, is_dst);
            *year = 0;
            break;
          case SQL_WVARCHAR:
            length = (memSizeType) columnData->length / sizeof(SQLWCHAR);
            if (unlikely(length > MAX_DATETIME2_LENGTH)) {
              logError(printf("sqlColumnTime: In column " FMT_D
                              " the datetime2 length of " FMT_U_MEM " is too long.\n",
                              column, length););
              err_info = RANGE_ERROR;
            } else {
              err_info = conv_sqlwstri_to_cstri(datetime2,
                  (SQLWCHAR *) columnData->buffer, length);
              if (unlikely(err_info != OKAY_NO_ERROR)) {
                logError(printf("sqlColumnTime: In column " FMT_D
                                " the datetime2 contains characters beyond Latin-1.\n",
                                column););
              } else {
                okay = assignTime(datetime2, year, month, day, hour, minute,
                                  second, micro_second, &isTime);
                if (unlikely(!okay)) {
                  logError(printf("sqlColumnTime: Column " FMT_D ": "
                                  "Failed to recognize time: %s\n",
                                  column, datetime2););
                  err_info = RANGE_ERROR;
                } else if (isTime) {
                  timSetLocalTZ(2000, *month, *day, *hour, *minute, *second,
                                time_zone, is_dst);
                } else {
                  timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                                time_zone, is_dst);
                } /* if */
              } /* if */
            } /* if */
            break;
          default:
            logError(printf("sqlColumnTime: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfSqlType(columnDescr->dataType)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
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

  /* sqlCommit */
    logFunction(printf("sqlCommit(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (unlikely(db->connection == SQL_NULL_HANDLE)) {
      dbNotOpen("sqlCommit");
      logError(printf("sqlCommit: Database is not open.\n"););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(SQLEndTran(SQL_HANDLE_DBC, db->connection,
                                   SQL_COMMIT) != SQL_SUCCESS)) {
      setDbErrorMsg("sqlCommit", "SQLEndTran",
                    SQL_HANDLE_DBC, db->connection);
      logError(printf("sqlCommit: SQLEndTran SQL_COMMIT:\n%s\n",
                      dbError.message););
      raise_error(DATABASE_ERROR);
    } /* if */
    logFunction(printf("sqlCommit -->\n"););
  } /* sqlCommit */



static void sqlExecute (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    SQLRETURN execute_result;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlExecute */
    logFunction(printf("sqlExecute(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    /* printf("ppStmt: " FMT_U_MEM "\n", (memSizeType) preparedStmt->ppStmt); */
    if (unlikely(!allParametersBound(preparedStmt))) {
      dbLibError("sqlExecute", "SQLExecute",
                 "Unbound statement parameter(s).\n");
      raise_error(DATABASE_ERROR);
    } else {
      if (preparedStmt->executeSuccessful) {
        if (unlikely(SQLFreeStmt(preparedStmt->ppStmt, SQL_CLOSE) != SQL_SUCCESS)) {
          setDbErrorMsg("sqlExecute", "SQLFreeStmt",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlExecute: SQLFreeStmt SQL_CLOSE:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          preparedStmt->executeSuccessful = FALSE;
          freePrefetched(preparedStmt);
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        preparedStmt->fetchOkay = FALSE;
        execute_result = SQLExecute(preparedStmt->ppStmt);
#ifdef ALLOW_EXECUTE_SUCCESS_WITH_INFO
      if (execute_result == SQL_SUCCESS_WITH_INFO) {
        execute_result = SQL_SUCCESS;
      } /*if */
#endif
        if (execute_result == SQL_NO_DATA || execute_result == SQL_SUCCESS) {
          if (preparedStmt->db->maxConcurrentActivities != 0) {
            /* The number of concurrent activities is limited. That */
            /* means: As long as not all data of this statement has */
            /* been fetched a second concurrent activity (e.g.      */
            /* preparing a new statement) may fail. To be on the    */
            /* safe side we fetch the whole result of this prepared */
            /* statement. The fetched results are stored in a       */
            /* prefetch list and used when sqlFetch() is called.    */
            if (preparedStmt->result_array_size != 0) {
              err_info = prefetchAll(preparedStmt, &preparedStmt->fetchRecord);
            } /* if */
            if (unlikely(err_info != OKAY_NO_ERROR)) {
              preparedStmt->executeSuccessful = FALSE;
              raise_error(err_info);
            } else {
              preparedStmt->executeSuccessful = TRUE;
              preparedStmt->fetchFinished = FALSE;
            } /* if */
          } else {
            preparedStmt->executeSuccessful = TRUE;
            preparedStmt->fetchFinished = FALSE;
          } /* if */
        } else {
          setDbErrorMsg("sqlExecute", "SQLExecute",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlExecute: SQLExecute execute_result: %d:\n%s\n",
                          execute_result, dbError.message););
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
    SQLRETURN fetch_result;
    errInfoType err_info = OKAY_NO_ERROR;

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
    } else if (preparedStmt->result_array_size == 0) {
      preparedStmt->fetchOkay = FALSE;
    } else if (!preparedStmt->fetchFinished) {
      /* printf("ppStmt: " FMT_U_MEM "\n", (memSizeType) preparedStmt->ppStmt); */
      err_info = doFetch(preparedStmt, &preparedStmt->fetchRecord);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->fetchFinished = TRUE;
        raise_error(err_info);
      } else {
        fetch_result = preparedStmt->currentFetch->fetch_result;
        if (fetch_result == SQL_SUCCESS) {
          /* printf("fetch success\n"); */
          if (likely(err_info == OKAY_NO_ERROR)) {
            preparedStmt->fetchOkay = TRUE;
          } else {
            preparedStmt->fetchOkay = FALSE;
            preparedStmt->fetchFinished = TRUE;
            raise_error(err_info);
          } /* if */
        } else if (fetch_result == SQL_NO_DATA) {
          preparedStmt->fetchOkay = FALSE;
          preparedStmt->fetchFinished = TRUE;
        } else {
          /* Errors of SQLFetch() have already been handled. */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("sqlFetch --> %d\n", preparedStmt->fetchOkay););
    return preparedStmt->fetchOkay;
  } /* sqlFetch */



static boolType sqlGetAutoCommit (databaseType database)

  {
    dbType db;
    SQLUINTEGER autoCommitState;
    boolType autoCommit;

  /* sqlGetAutoCommit */
    logFunction(printf("sqlGetAutoCommit(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (unlikely(db->connection == SQL_NULL_HANDLE)) {
      dbNotOpen("sqlGetAutoCommit");
      logError(printf("sqlGetAutoCommit: Database is not open.\n"););
      raise_error(DATABASE_ERROR);
      autoCommit = FALSE;
    } else if (unlikely(SQLGetConnectAttrW(db->connection,
        SQL_ATTR_AUTOCOMMIT, &autoCommitState, 0, NULL) != SQL_SUCCESS)) {
      setDbErrorMsg("sqlGetAutoCommit", "SQLGetConnectAttrW",
                    SQL_HANDLE_DBC, db->connection);
      logError(printf("sqlGetAutoCommit: SQLGetConnectAttrW SQL_ATTR_AUTOCOMMIT:\n%s\n",
                      dbError.message););
      raise_error(DATABASE_ERROR);
      autoCommit = FALSE;
    } else {
      autoCommit = autoCommitState == SQL_AUTOCOMMIT_ON;
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
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlIsNull: Fetch okay: %d, column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      preparedStmt->fetchOkay, column,
                      preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      isNull = FALSE;
    } else {
      isNull = preparedStmt->currentFetch->result_array[column - 1].length == SQL_NULL_DATA;
    } /* if */
    logFunction(printf("sqlIsNull --> %s\n", isNull ? "TRUE" : "FALSE"););
    return isNull;
  } /* sqlIsNull */



static sqlStmtType sqlPrepare (databaseType database,
    const const_striType sqlStatementStri)

  {
    dbType db;
    striType statementStri;
    SQLWCHAR *query;
    memSizeType queryLength;
    errInfoType err_info = OKAY_NO_ERROR;
    preparedStmtType preparedStmt;

  /* sqlPrepare */
    logFunction(printf("sqlPrepare(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) database,
                       striAsUnquotedCStri(sqlStatementStri)););
    db = (dbType) database;
    if (unlikely(db->connection == SQL_NULL_HANDLE)) {
      dbNotOpen("sqlPrepare");
      logError(printf("sqlPrepare: Database is not open.\n"););
      err_info = DATABASE_ERROR;
      preparedStmt = NULL;
    } else {
      statementStri = processStatementStri(sqlStatementStri, &err_info);
      if (statementStri == NULL) {
        preparedStmt = NULL;
      } else {
        query = stri_to_sqlwstri(statementStri, &queryLength, &err_info);
        if (unlikely(query == NULL)) {
          preparedStmt = NULL;
        } else {
          if (queryLength > SQLINTEGER_MAX) {
            /* It is not possible to cast queryLength to SQLINTEGER. */
            logError(printf("sqlPrepare: Statement string too long (length = " FMT_U_MEM ")\n",
                            queryLength););
            err_info = RANGE_ERROR;
            preparedStmt = NULL;
          } else if (unlikely(!ALLOC_RECORD2(preparedStmt, preparedStmtRecordCli,
                                             count.prepared_stmt, count.prepared_stmt_bytes))) {
            err_info = MEMORY_ERROR;
          } else {
            memset(preparedStmt, 0, sizeof(preparedStmtRecordCli));
            if (SQLAllocHandle(SQL_HANDLE_STMT,
                               db->connection,
                               &preparedStmt->ppStmt) != SQL_SUCCESS) {
              setDbErrorMsg("sqlPrepare", "SQLAllocHandle",
                            SQL_HANDLE_DBC, db->connection);
              logError(printf("sqlPrepare: SQLAllocHandle SQL_HANDLE_STMT:\n%s\n",
                              dbError.message););
              FREE_RECORD2(preparedStmt, preparedStmtRecordCli,
                           count.prepared_stmt, count.prepared_stmt_bytes);
              err_info = DATABASE_ERROR;
              preparedStmt = NULL;
            } else if (SQLPrepareW(preparedStmt->ppStmt,
                                   (SQLWCHAR *) query,
                                   (SQLINTEGER) queryLength) != SQL_SUCCESS) {
              setDbErrorMsg("sqlPrepare", "SQLPrepare",
                            SQL_HANDLE_STMT, preparedStmt->ppStmt);
              logError(printf("sqlPrepare: SQLPrepare:\n%s\n",
                              dbError.message););
              FREE_RECORD2(preparedStmt, preparedStmtRecordCli,
                          count.prepared_stmt, count.prepared_stmt_bytes);
              err_info = DATABASE_ERROR;
              preparedStmt = NULL;
            } else {
              preparedStmt->usage_count = 1;
              preparedStmt->sqlFunc = db->sqlFunc;
              preparedStmt->executeSuccessful = FALSE;
              preparedStmt->fetchOkay = FALSE;
              preparedStmt->fetchFinished = TRUE;
              preparedStmt->db = db;
              if (db->usage_count != 0) {
                db->usage_count++;
              } /* if */
              if (likely(err_info == OKAY_NO_ERROR)) {
                err_info = setupParameters(preparedStmt);
                if (likely(err_info == OKAY_NO_ERROR)) {
                  err_info = setupResult(preparedStmt);
                  if (likely(err_info == OKAY_NO_ERROR)) {
                    err_info = bindResult(preparedStmt, &preparedStmt->fetchRecord);
                  } /* if */
                } /* if */
              } /* if */
              if (unlikely(err_info != OKAY_NO_ERROR)) {
                freePreparedStmt((sqlStmtType) preparedStmt);
                preparedStmt = NULL;
              } /* if */
            } /* if */
          } /* if */
          UNALLOC_SQLWSTRI(query, statementStri);
        } /* if */
        FREE_STRI(statementStri);
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

  /* sqlRollback */
    logFunction(printf("sqlRollback(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (unlikely(db->connection == SQL_NULL_HANDLE)) {
      dbNotOpen("sqlRollback");
      logError(printf("sqlRollback: Database is not open.\n"););
      raise_error(DATABASE_ERROR);
    } else if (unlikely(SQLEndTran(SQL_HANDLE_DBC, db->connection,
                                   SQL_ROLLBACK) != SQL_SUCCESS)) {
      setDbErrorMsg("sqlRollback", "SQLEndTran",
                    SQL_HANDLE_DBC, db->connection);
      logError(printf("sqlRollback: SQLEndTran SQL_ROLLBACK:\n%s\n",
                      dbError.message););
      raise_error(DATABASE_ERROR);
    } /* if */
    logFunction(printf("sqlRollback -->\n"););
  } /* sqlRollback */



static void sqlSetAutoCommit (databaseType database, boolType autoCommit)

  {
    dbType db;
    SQLPOINTER autoCommitState;

  /* sqlSetAutoCommit */
    logFunction(printf("sqlSetAutoCommit(" FMT_U_MEM ", %d)\n",
                       (memSizeType) database, autoCommit););
    db = (dbType) database;
    if (unlikely(db->connection == SQL_NULL_HANDLE)) {
      dbNotOpen("sqlSetAutoCommit");
      logError(printf("sqlSetAutoCommit: Database is not open.\n"););
      raise_error(DATABASE_ERROR);
    } else {
      if (autoCommit) {
        autoCommitState = (SQLPOINTER) SQL_AUTOCOMMIT_ON;
      } else {
        autoCommitState = (SQLPOINTER) SQL_AUTOCOMMIT_OFF;
      } /* if */
      if (unlikely(SQLSetConnectAttrW(db->connection, SQL_ATTR_AUTOCOMMIT,
                                     autoCommitState, 0) != SQL_SUCCESS)) {
        setDbErrorMsg("sqlSetAutoCommit", "SQLSetConnectAttrW",
                      SQL_HANDLE_DBC, db->connection);
        logError(printf("sqlSetAutoCommit: SQLSetConnectAttrW SQL_ATTR_AUTOCOMMIT:\n%s\n",
                        dbError.message););
        raise_error(DATABASE_ERROR);
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
    if (unlikely(preparedStmt->result_array_size > INTTYPE_MAX)) {
      raise_error(RANGE_ERROR);
      columnCount = 0;
    } else {
      columnCount = (intType) preparedStmt->result_array_size;
    } /* if */
    logFunction(printf("sqlStmtColumnCount --> " FMT_D "\n", columnCount););
    return columnCount;
  } /* sqlStmtColumnCount */



static striType sqlStmtColumnName (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    SQLRETURN returnCode;
    SQLSMALLINT stringLength;
    SQLWCHAR wideNameBuffer[CHARS_IN_NAME_BUFFER + NULL_TERMINATION_LEN];
    SQLWCHAR *wideName;
    memSizeType wideNameLength;
    errInfoType err_info = OKAY_NO_ERROR;
    striType name;

  /* sqlStmtColumnName */
    logFunction(printf("sqlStmtColumnName(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlStmtColumnName: column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      column, preparedStmt->result_array_size););
      err_info = RANGE_ERROR;
      name = NULL;
    } else {
      returnCode = SQLColAttributeW(preparedStmt->ppStmt,
                                    (SQLUSMALLINT) column,
                                    SQL_DESC_NAME,
                                    wideNameBuffer,
                                    (SQLSMALLINT) SIZ_SQLWSTRI(CHARS_IN_NAME_BUFFER),
                                    &stringLength,
                                    NULL);
      if (returnCode != SQL_SUCCESS &&
          returnCode != SQL_SUCCESS_WITH_INFO) {
        setDbErrorMsg("sqlStmtColumnName", "SQLColAttributeW",
                      SQL_HANDLE_STMT, preparedStmt->ppStmt);
        logError(printf("sqlStmtColumnName: SQLColAttributeW SQL_DESC_NAME "
                        "returnCode: %d\n%s\n",
                        returnCode, dbError.message););
        err_info = DATABASE_ERROR;
        name = NULL;
      } else if (unlikely(stringLength < 0 ||
                          (SQLUSMALLINT) stringLength >
                              SQLSMALLINT_MAX - sizeof(SQLWCHAR) * NULL_TERMINATION_LEN)) {
        dbInconsistent("sqlStmtColumnName", "SQLColAttributeW");
        logError(printf("sqlStmtColumnName: "
                        "String length negative or too big: %hd\n",
                        stringLength););
        err_info = DATABASE_ERROR;
        name = NULL;
      } else {
        wideNameLength = (memSizeType) stringLength / sizeof(SQLWCHAR);
        if (returnCode == SQL_SUCCESS) {
          name = sqlwstri_to_stri(wideNameBuffer, wideNameLength, &err_info);
        } else if (unlikely(!ALLOC_SQLWSTRI(wideName, wideNameLength))) {
          err_info = MEMORY_ERROR;
          name = NULL;
        } else if (SQLColAttributeW(preparedStmt->ppStmt,
                                    (SQLUSMALLINT) column,
                                    SQL_DESC_NAME,
                                    wideName,
                                    (SQLSMALLINT) SIZ_SQLWSTRI(wideNameLength),
                                    NULL,
                                    NULL) != SQL_SUCCESS) {
          setDbErrorMsg("sqlStmtColumnName", "SQLColAttributeW",
                        SQL_HANDLE_STMT, preparedStmt->ppStmt);
          logError(printf("sqlStmtColumnName: SQLColAttributeW SQL_DESC_NAME:\n%s\n",
                          dbError.message););
          UNALLOC_SQLWSTRI(wideName, wideNameLength);
          err_info = DATABASE_ERROR;
          name = NULL;
        } else {
          name = sqlwstri_to_stri(wideName, wideNameLength, &err_info);
          UNALLOC_SQLWSTRI(wideName, wideNameLength);
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
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



#if ANY_LOG_ACTIVE
static const char null_string_marker[]      = "\\ *NULL_STRING* ";



static cstriType wstri16AsUnquotedCStri (const const_utf16striType wstri)

  {
    utf16charType ch;
    memSizeType idx = 0;
    memSizeType pos = 0;
    memSizeType len;
    static char buffer[AND_SO_ON_LIMIT * MAXIMUM_UTF16_ESCAPE_WIDTH +
                       AND_SO_ON_LENGTH];

  /* wstri16AsUnquotedCStri */
    if (wstri != NULL) {
      while (wstri[idx] != '\0' && idx < AND_SO_ON_LIMIT) {
        ch = (utf16charType) wstri[idx];
        if (ch < 127) {
          if (ch < ' ') {
            len = strlen(stri_escape_sequence[ch]);
            memcpy(&buffer[pos], stri_escape_sequence[ch], len);
            pos += len;
          } else if (ch == '\\') {
            memcpy(&buffer[pos], "\\\\", 2);
            pos += 2;
          } else if (ch == '\"') {
            memcpy(&buffer[pos], "\\\"", 2);
            pos += 2;
          } else {
            buffer[pos] = (char) ch;
            pos++;
          } /* if */
        } else {
          pos += (memSizeType) sprintf(&buffer[pos], "\\%u;", (unsigned int) ch);
        } /* if */
        idx++;
      } /* while */
      if (wstri[idx] != '\0') {
	do {
	  idx++;
        } while (wstri[idx] != '\0');
        pos += (memSizeType) sprintf(&buffer[pos], AND_SO_ON_TEXT FMT_U_MEM, idx);
      } /* if */
    } else {
      MEMCPY_STRING(buffer, null_string_marker);
      pos = STRLEN(null_string_marker);
    } /* if */
    buffer[pos] = '\0';
    return buffer;
  } /* wstri16AsUnquotedCStri */



static cstriType wstri32AsUnquotedCStri (const const_utf32striType wstri)

  {
    utf32charType ch;
    memSizeType idx = 0;
    memSizeType pos = 0;
    memSizeType len;
    static char buffer[AND_SO_ON_LIMIT * MAXIMUM_UTF32_ESCAPE_WIDTH +
                       AND_SO_ON_LENGTH];

  /* wstri32AsUnquotedCStri */
    if (wstri != NULL) {
      while (wstri[idx] != '\0' && idx < AND_SO_ON_LIMIT) {
        ch = (utf32charType) wstri[idx];
        if (ch < 127) {
          if (ch < ' ') {
            len = strlen(stri_escape_sequence[ch]);
            memcpy(&buffer[pos], stri_escape_sequence[ch], len);
            pos += len;
          } else if (ch == '\\') {
            memcpy(&buffer[pos], "\\\\", 2);
            pos += 2;
          } else if (ch == '\"') {
            memcpy(&buffer[pos], "\\\"", 2);
            pos += 2;
          } else {
            buffer[pos] = (char) ch;
            pos++;
          } /* if */
        } else {
          pos += (memSizeType) sprintf(&buffer[pos], "\\%lu;", (unsigned long) ch);
        } /* if */
        idx++;
      } /* while */
      if (wstri[idx] != '\0') {
	do {
	  idx++;
        } while (wstri[idx] != '\0');
        pos += (memSizeType) sprintf(&buffer[pos], AND_SO_ON_TEXT FMT_U_MEM, idx);
      } /* if */
    } else {
      MEMCPY_STRING(buffer, null_string_marker);
      pos = STRLEN(null_string_marker);
    } /* if */
    buffer[pos] = '\0';
    return buffer;
  } /* wstri32AsUnquotedCStri */
#endif



static errInfoType prepareSqlConnection (SQLHENV *sql_environment,
    SQLHDBC *connection)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* prepareSqlConnection */
    logFunction(printf("prepareSqlConnection\n"););
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE,
                       sql_environment) != SQL_SUCCESS) {
      logError(printf("prepareSqlConnection: SQLAllocHandle failed\n"););
      err_info = MEMORY_ERROR;
    } else if (SQLSetEnvAttr(*sql_environment,
                             SQL_ATTR_ODBC_VERSION,
                             (SQLPOINTER) SQL_OV_ODBC3,
                             SQL_IS_INTEGER) != SQL_SUCCESS) {
      setDbErrorMsg("prepareSqlConnection", "SQLSetEnvAttr",
                    SQL_HANDLE_ENV, *sql_environment);
      logError(printf("prepareSqlConnection: SQLSetEnvAttr "
                      "SQL_ATTR_ODBC_VERSION:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
      SQLFreeHandle(SQL_HANDLE_ENV, *sql_environment);
    } else if (SQLAllocHandle(SQL_HANDLE_DBC, *sql_environment,
                              connection) != SQL_SUCCESS) {
      setDbErrorMsg("prepareSqlConnection", "SQLAllocHandle",
                    SQL_HANDLE_ENV, *sql_environment);
      logError(printf("prepareSqlConnection: SQLAllocHandle "
                      "SQL_HANDLE_DBC:\n%s\n", dbError.message););
      err_info = DATABASE_ERROR;
      SQLFreeHandle(SQL_HANDLE_ENV, *sql_environment);
    } /* if */
    logFunction(printf("prepareSqlConnection --> %d\n", err_info););
    return err_info;
  } /* prepareSqlConnection */



static databaseType createDbRecord (SQLHENV sql_environment, SQLHDBC connection,
    intType driver, errInfoType *err_info)

  {
    SQLUSMALLINT maxConcurrentActivities;
    boolType wideCharsSupported;
    boolType tinyintIsUnsigned;
    dbType database;

  /* createDbRecord */
    logFunction(printf("createDbRecord(*, *, " FMT_D ", %d)\n",
                       driver, *err_info););
    if (SQLGetInfoW(connection,
                    SQL_MAX_CONCURRENT_ACTIVITIES,
                    (SQLPOINTER) &maxConcurrentActivities,
                    sizeof(maxConcurrentActivities),
                    NULL) != SQL_SUCCESS) {
      setDbErrorMsg("createDbRecord", "SQLGetInfoW",
                    SQL_HANDLE_DBC, connection);
      logError(printf("createDbRecord: SQLGetInfoW:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
    } else {
#ifdef WIDE_CHARS_SUPPORTED
      wideCharsSupported = WIDE_CHARS_SUPPORTED;
#else
      wideCharsSupported = hasDataType(connection, SQL_WCHAR, err_info);
#endif
#ifdef TINY_INT_IS_UNSIGNED
      tinyintIsUnsigned = TINY_INT_IS_UNSIGNED;
#else
      tinyintIsUnsigned = dataTypeIsUnsigned(connection, SQL_TINYINT, err_info);
#endif
      if (likely(*err_info == OKAY_NO_ERROR)) {
        if (unlikely(!setupFuncTable() ||
                     !ALLOC_RECORD2(database, dbRecordCli,
                                    count.database, count.database_bytes))) {
          *err_info = MEMORY_ERROR;
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(*err_info != OKAY_NO_ERROR)) {
      SQLDisconnect(connection);
      SQLFreeHandle(SQL_HANDLE_DBC, connection);
      SQLFreeHandle(SQL_HANDLE_ENV, sql_environment);
      database = NULL;
    } else {
      logMessage(printf("maxConcurrentActivities: %lu\n",
                        (unsigned long) maxConcurrentActivities););
      memset(database, 0, sizeof(dbRecordCli));
      database->usage_count = 1;
      database->isOpen = TRUE;
      database->sqlFunc = sqlFunc;
      database->driver = driver;
      database->sql_environment = sql_environment;
      database->connection = connection;
      database->wideCharsSupported = wideCharsSupported;
      database->tinyintIsUnsigned = tinyintIsUnsigned;
      database->maxConcurrentActivities = maxConcurrentActivities;
    } /* if */
    logFunction(printf("createDbRecord --> " FMT_U_MEM " (err_info=%d)\n",
                       (memSizeType) database, *err_info););
    return (databaseType) database;
  } /* createDbRecord */
