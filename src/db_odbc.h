/********************************************************************/
/*                                                                  */
/*  db_odbc.h     ODBC interfaces used by Seed7.                    */
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
/*  File: seed7/src/db_odbc.h                                       */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: ODBC interfaces used by Seed7.                         */
/*                                                                  */
/********************************************************************/

typedef unsigned char SQLCHAR;
typedef signed char   SQLSCHAR;
typedef UINT16TYPE    SQLWCHAR;

typedef INT16TYPE  SQLSMALLINT;
typedef UINT16TYPE SQLUSMALLINT;

typedef INT32TYPE  SQLINTEGER;
typedef UINT32TYPE SQLUINTEGER;

typedef INT64TYPE  SQLLEN;
typedef UINT64TYPE SQLULEN;

typedef SQLSMALLINT SQLRETURN;

#define SQL_MAX_NUMERIC_LEN 16
typedef struct {
  SQLCHAR  precision;
  SQLSCHAR scale;
  SQLCHAR  sign;
  SQLCHAR  val[SQL_MAX_NUMERIC_LEN];
} SQL_NUMERIC_STRUCT;

typedef struct {
  SQLSMALLINT  year;
  SQLUSMALLINT month;
  SQLUSMALLINT day;
} SQL_DATE_STRUCT;

typedef struct {
  SQLUSMALLINT hour;
  SQLUSMALLINT minute;
  SQLUSMALLINT second;
} SQL_TIME_STRUCT;

typedef struct {
  SQLSMALLINT  year;
  SQLUSMALLINT month;
  SQLUSMALLINT day;
  SQLUSMALLINT hour;
  SQLUSMALLINT minute;
  SQLUSMALLINT second;
  SQLUINTEGER  fraction;
} SQL_TIMESTAMP_STRUCT;

typedef enum {
  SQL_IS_YEAR             =  1,
  SQL_IS_MONTH            =  2,
  SQL_IS_DAY              =  3,
  SQL_IS_HOUR             =  4,
  SQL_IS_MINUTE           =  5,
  SQL_IS_SECOND           =  6,
  SQL_IS_YEAR_TO_MONTH    =  7,
  SQL_IS_DAY_TO_HOUR      =  8,
  SQL_IS_DAY_TO_MINUTE    =  9,
  SQL_IS_DAY_TO_SECOND    = 10,
  SQL_IS_HOUR_TO_MINUTE   = 11,
  SQL_IS_HOUR_TO_SECOND   = 12,
  SQL_IS_MINUTE_TO_SECOND = 13
} SQLINTERVAL;

typedef struct {
  SQLUINTEGER year;
  SQLUINTEGER month;
} SQL_YEAR_MONTH_STRUCT;

typedef struct {
  SQLUINTEGER day;
  SQLUINTEGER hour;
  SQLUINTEGER minute;
  SQLUINTEGER second;
  SQLUINTEGER fraction;
} SQL_DAY_SECOND_STRUCT;

typedef struct {
 SQLINTERVAL interval_type;
 SQLSMALLINT interval_sign;
 union {
  SQL_YEAR_MONTH_STRUCT year_month;
  SQL_DAY_SECOND_STRUCT day_second;
 } intval;
} SQL_INTERVAL_STRUCT;

typedef void *SQLPOINTER;

typedef void *SQLHANDLE;
typedef SQLHANDLE SQLHENV;
typedef SQLHANDLE SQLHDBC;
typedef SQLHANDLE SQLHSTMT;
typedef SQLHANDLE SQLHDESC;

#define SQL_NULL_HANDLE  0L
#define SQL_HANDLE_ENV   1
#define SQL_HANDLE_DBC   2
#define SQL_HANDLE_STMT  3
#define SQL_HANDLE_DESC  4

#define SQL_FALSE  0
#define SQL_TRUE   1

#define SQL_OV_ODBC3  3UL

#define SQL_SUCCESS             0
#define SQL_SUCCESS_WITH_INFO   1
#define SQL_NO_DATA           100
#define SQL_NULL_DATA         (-1)
#define SQL_DATA_AT_EXEC      (-2)

#define SQL_NO_TOTAL          (-4)

#define SQL_ERROR             (-1)

#define SQL_PARAM_INPUT  1

#define SQL_ATTR_ODBC_VERSION    200
#define SQL_ATTR_APP_ROW_DESC  10010

#define SQL_CLOSE  0

#define SQL_DESC_TYPE             1002
#define SQL_DESC_LENGTH           1003
#define SQL_DESC_OCTET_LENGTH_PTR 1004
#define SQL_DESC_PRECISION        1005
#define SQL_DESC_SCALE            1006
#define SQL_DESC_INDICATOR_PTR    1009
#define SQL_DESC_DATA_PTR         1010
#define SQL_DESC_NAME             1011
#define SQL_DESC_OCTET_LENGTH     1013

#define SQL_FETCH_NEXT   1
#define SQL_FETCH_FIRST  2

#define SQL_IS_INTEGER (-6)

#define SQL_CHAR            1
#define SQL_NUMERIC         2
#define SQL_DECIMAL         3
#define SQL_INTEGER         4
#define SQL_SMALLINT        5
#define SQL_FLOAT           6
#define SQL_REAL            7
#define SQL_DOUBLE          8
#define SQL_DATETIME        9
#define SQL_VARCHAR        12
#define SQL_TYPE_DATE      91
#define SQL_TYPE_TIME      92
#define SQL_TYPE_TIMESTAMP 93

#define SQL_INTERVAL_YEAR             101
#define SQL_INTERVAL_MONTH            102
#define SQL_INTERVAL_DAY              103
#define SQL_INTERVAL_HOUR             104
#define SQL_INTERVAL_MINUTE           105
#define SQL_INTERVAL_SECOND           106
#define SQL_INTERVAL_YEAR_TO_MONTH    107
#define SQL_INTERVAL_DAY_TO_HOUR      108
#define SQL_INTERVAL_DAY_TO_MINUTE    109
#define SQL_INTERVAL_DAY_TO_SECOND    110
#define SQL_INTERVAL_HOUR_TO_MINUTE   111
#define SQL_INTERVAL_HOUR_TO_SECOND   112
#define SQL_INTERVAL_MINUTE_TO_SECOND 113

#define SQL_LONGVARCHAR    (-1)
#define SQL_BINARY         (-2)
#define SQL_VARBINARY      (-3)
#define SQL_LONGVARBINARY  (-4)
#define SQL_BIGINT         (-5)
#define SQL_TINYINT        (-6)
#define SQL_WCHAR          (-8)
#define SQL_WVARCHAR       (-9)
#define SQL_WLONGVARCHAR  (-10)

#define SQL_C_CHAR           SQL_CHAR
#define SQL_C_WCHAR          SQL_WCHAR
#define SQL_C_LONG           SQL_INTEGER
#define SQL_C_SHORT          SQL_SMALLINT
#define SQL_C_FLOAT          SQL_REAL
#define SQL_C_DOUBLE         SQL_DOUBLE
#define SQL_C_NUMERIC        SQL_NUMERIC
#define SQL_C_BINARY         SQL_BINARY
#define SQL_C_TYPE_DATE      SQL_TYPE_DATE
#define SQL_C_TYPE_TIME      SQL_TYPE_TIME
#define SQL_C_TYPE_TIMESTAMP SQL_TYPE_TIMESTAMP

#define SQL_C_INTERVAL_YEAR             SQL_INTERVAL_YEAR
#define SQL_C_INTERVAL_MONTH            SQL_INTERVAL_MONTH
#define SQL_C_INTERVAL_DAY              SQL_INTERVAL_DAY
#define SQL_C_INTERVAL_HOUR             SQL_INTERVAL_HOUR
#define SQL_C_INTERVAL_MINUTE           SQL_INTERVAL_MINUTE
#define SQL_C_INTERVAL_SECOND           SQL_INTERVAL_SECOND
#define SQL_C_INTERVAL_YEAR_TO_MONTH    SQL_INTERVAL_YEAR_TO_MONTH
#define SQL_C_INTERVAL_DAY_TO_HOUR      SQL_INTERVAL_DAY_TO_HOUR
#define SQL_C_INTERVAL_DAY_TO_MINUTE    SQL_INTERVAL_DAY_TO_MINUTE
#define SQL_C_INTERVAL_DAY_TO_SECOND    SQL_INTERVAL_DAY_TO_SECOND
#define SQL_C_INTERVAL_HOUR_TO_MINUTE   SQL_INTERVAL_HOUR_TO_MINUTE
#define SQL_C_INTERVAL_HOUR_TO_SECOND   SQL_INTERVAL_HOUR_TO_SECOND
#define SQL_C_INTERVAL_MINUTE_TO_SECOND SQL_INTERVAL_MINUTE_TO_SECOND

#define SQL_C_SSHORT   (-15)
#define SQL_C_SLONG    (-16)
#define SQL_C_SBIGINT  (-25)
#define SQL_C_STINYINT (-26)

#ifndef STDCALL
#if defined(_WIN32)
#define STDCALL __stdcall
#else
#define STDCALL
#endif
#endif

SQLRETURN STDCALL SQLAllocHandle (SQLSMALLINT HandleType,
                                  SQLHANDLE InputHandle, SQLHANDLE *OutputHandle);
SQLRETURN STDCALL SQLBindCol (SQLHSTMT StatementHandle,
                              SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType,
                              SQLPOINTER TargetValue, SQLLEN BufferLength,
                              SQLLEN *StrLen_or_Ind);
SQLRETURN STDCALL SQLBindParameter (SQLHSTMT hstmt,
                                    SQLUSMALLINT ipar,
                                    SQLSMALLINT  fParamType,
                                    SQLSMALLINT  fCType,
                                    SQLSMALLINT  fSqlType,
                                    SQLULEN      cbColDef,
                                    SQLSMALLINT  ibScale,
                                    SQLPOINTER   rgbValue,
                                    SQLLEN       cbValueMax,
                                    SQLLEN      *pcbValue);
SQLRETURN STDCALL SQLColAttribute (SQLHSTMT StatementHandle,
                                   SQLUSMALLINT ColumnNumber,
                                   SQLUSMALLINT FieldIdentifier,
                                   SQLPOINTER CharacterAttribute,
                                   SQLSMALLINT BufferLength,
                                   SQLSMALLINT *StringLength,
                                   SQLLEN *NumericAttribute);
SQLRETURN STDCALL SQLColAttributeW (SQLHSTMT hstmt,
                                    SQLUSMALLINT iCol,
                                    SQLUSMALLINT iField,
                                    SQLPOINTER  pCharAttr,
                                    SQLSMALLINT  cbCharAttrMax,
                                    SQLSMALLINT *pcbCharAttr,
                                    SQLLEN  *pNumAttr);
SQLRETURN STDCALL SQLConnectW (SQLHDBC ConnectionHandle,
                               SQLWCHAR *ServerName, SQLSMALLINT NameLength1,
                               SQLWCHAR *UserName, SQLSMALLINT NameLength2,
                               SQLWCHAR *Authentication, SQLSMALLINT NameLength3);
SQLRETURN STDCALL SQLDataSources (SQLHENV EnvironmentHandle,
                                  SQLUSMALLINT Direction, SQLCHAR *ServerName,
                                  SQLSMALLINT BufferLength1, SQLSMALLINT *NameLength1,
                                  SQLCHAR *Description, SQLSMALLINT BufferLength2,
                                  SQLSMALLINT *NameLength2);
SQLRETURN STDCALL SQLDisconnect (SQLHDBC ConnectionHandle);
SQLRETURN STDCALL SQLDrivers (SQLHENV      henv,
                              SQLUSMALLINT fDirection,
                              SQLCHAR     *szDriverDesc,
                              SQLSMALLINT  cbDriverDescMax,
                              SQLSMALLINT *pcbDriverDesc,
                              SQLCHAR     *szDriverAttributes,
                              SQLSMALLINT  cbDrvrAttrMax,
                              SQLSMALLINT *pcbDrvrAttr);
SQLRETURN STDCALL SQLExecute (SQLHSTMT StatementHandle);
SQLRETURN STDCALL SQLFetch (SQLHSTMT StatementHandle);
SQLRETURN STDCALL SQLFreeHandle (SQLSMALLINT HandleType, SQLHANDLE Handle);
SQLRETURN STDCALL SQLFreeStmt (SQLHSTMT StatementHandle, SQLUSMALLINT Option);
SQLRETURN STDCALL SQLGetData (SQLHSTMT     StatementHandle,
                              SQLUSMALLINT ColumnNumber,
                              SQLSMALLINT  TargetType,
                              SQLPOINTER   TargetValue,
                              SQLLEN       BufferLength,
                              SQLLEN      *StrLen_or_Ind);
SQLRETURN STDCALL SQLGetDiagRec (SQLSMALLINT HandleType, SQLHANDLE Handle,
                                 SQLSMALLINT RecNumber, SQLCHAR *Sqlstate,
                                 SQLINTEGER *NativeError, SQLCHAR *MessageText,
                                 SQLSMALLINT BufferLength, SQLSMALLINT *TextLength);
SQLRETURN STDCALL SQLGetStmtAttr (SQLHSTMT StatementHandle,
                                  SQLINTEGER Attribute, SQLPOINTER Value,
                                  SQLINTEGER BufferLength, SQLINTEGER *StringLength);
SQLRETURN STDCALL SQLNumResultCols (SQLHSTMT StatementHandle,
                                    SQLSMALLINT *ColumnCount);
SQLRETURN STDCALL SQLPrepareW (SQLHSTMT   hstmt,
                               SQLWCHAR  *szSqlStr,
                               SQLINTEGER cbSqlStr);
SQLRETURN STDCALL SQLSetDescField (SQLHDESC DescriptorHandle,
                                   SQLSMALLINT RecNumber, SQLSMALLINT FieldIdentifier,
                                   SQLPOINTER Value, SQLINTEGER BufferLength);
SQLRETURN STDCALL SQLSetEnvAttr (SQLHENV EnvironmentHandle,
                                 SQLINTEGER Attribute, SQLPOINTER Value,
                                 SQLINTEGER StringLength);
