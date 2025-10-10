/********************************************************************/
/*                                                                  */
/*  sql_log.c     Logging for SQL ODBC/CLI interface functions.     */
/*  Copyright (C) 2025  Thomas Mertes                               */
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
/*  File: seed7/src/sql_log.c                                       */
/*  Changes: 2025  Thomas Mertes                                    */
/*  Content: Logging for SQL ODBC/CLI interface functions.          */
/*                                                                  */
/********************************************************************/

#define logCliFunc logFunctionX



static SQLRETURN my_SQLAllocHandle (SQLSMALLINT handleType,
    SQLHANDLE inputHandle, SQLHANDLE *outputHandle)

  {
    SQLRETURN funcResult;

  /* my_SQLAllocHandle */
    logCliFunc(printf("SQLAllocHandle(" FMT_D16 ", " FMT_U_MEM ", "
                      FMT_U_MEM ")\n",
                      handleType, (memSizeType) inputHandle,
                      (memSizeType) outputHandle););
    funcResult = SQLAllocHandle(handleType, inputHandle, outputHandle);
    logCliFunc(printf("SQLAllocHandle(" FMT_D16 ", " FMT_U_MEM ", "
                      FMT_U_MEM ") --> " FMT_D16 "\n",
                      handleType, (memSizeType) inputHandle,
                      (memSizeType) *outputHandle, funcResult););
    return funcResult;
  } /* my_SQLAllocHandle */



static SQLRETURN my_SQLBindCol (SQLHSTMT statementHandle,
    SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
    SQLPOINTER targetValue, SQLLEN bufferLength,
    SQLLEN *strLen_or_Ind)

  {
    SQLRETURN funcResult;

  /* my_SQLBindCol */
    logCliFunc(printf("SQLBindCol(" FMT_U_MEM ", " FMT_U16 ", "
                      FMT_D16 ", " FMT_U_MEM ", " FMT_D64 ", "
                      FMT_U_MEM ")\n",
                      (memSizeType) statementHandle,
                      columnNumber, targetType,
                      (memSizeType) targetValue, bufferLength,
                      (memSizeType) strLen_or_Ind););
    funcResult = SQLBindCol(statementHandle, columnNumber, targetType,
                            targetValue, bufferLength,strLen_or_Ind);
    logCliFunc(printf("SQLBindCol(" FMT_U_MEM ", " FMT_U16 ", "
                      FMT_D16 ", " FMT_U_MEM ", " FMT_D64 ", "
                      FMT_D64 ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle,
                      columnNumber, targetType,
                      (memSizeType) targetValue,
                      bufferLength, *strLen_or_Ind, funcResult););
    return funcResult;
  } /* my_SQLBindCol */



static SQLRETURN my_SQLBindParameter (SQLHSTMT statementHandle,
    SQLUSMALLINT parameterNumber, SQLSMALLINT inputOutputType,
    SQLSMALLINT valueType, SQLSMALLINT parameterType,
    SQLULEN columnSize, SQLSMALLINT decimalDigits,
    SQLPOINTER parameterValuePtr, SQLLEN bufferLength,
    SQLLEN *strLen_or_IndPtr)

  {
    SQLRETURN funcResult;

  /* my_SQLBindParameter */
    logCliFunc(printf("SQLBindParameter(" FMT_U_MEM ", " FMT_U16
                      ", " FMT_D16 ", " FMT_D16 ", " FMT_D16 ", "
                      FMT_U64 ", " FMT_D16 ", " FMT_U_MEM ", "
                      FMT_D64 ", " FMT_U_MEM ")\n",
                      (memSizeType) statementHandle, parameterNumber,
                      inputOutputType, valueType, parameterType,
                      columnSize, decimalDigits,
                      (memSizeType) parameterValuePtr, bufferLength,
                      (memSizeType) strLen_or_IndPtr););
    funcResult = SQLBindParameter(statementHandle, parameterNumber,
                                  inputOutputType, valueType,
                                  parameterType, columnSize,
                                  decimalDigits, parameterValuePtr,
                                  bufferLength, strLen_or_IndPtr);
    logCliFunc(printf("SQLBindParameter(" FMT_U_MEM ", " FMT_U16
                      ", " FMT_D16 ", " FMT_D16 ", " FMT_D16 ", "
                      FMT_U64 ", " FMT_D16 ", " FMT_U_MEM ", "
                      FMT_D64 ", " FMT_U_MEM ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle, parameterNumber,
                      inputOutputType, valueType, parameterType,
                      columnSize, decimalDigits,
                      (memSizeType) parameterValuePtr, bufferLength,
                      (memSizeType) strLen_or_IndPtr,
                      funcResult););
    return funcResult;
  } /* my_SQLBindParameter */



#ifdef ODBC_DRIVER_FUNCTIONS_NEEDED
static SQLRETURN my_SQLBrowseConnectW (SQLHDBC connectionHandle,
    SQLWCHAR *inConnectionString, SQLSMALLINT stringLength1,
    SQLWCHAR *outConnectionString, SQLSMALLINT bufferLength,
    SQLSMALLINT *stringLength2Ptr)

  {
    SQLRETURN funcResult;

  /* my_SQLBrowseConnectW */
    logCliFunc(printf("SQLBrowseConnectW(" FMT_U_MEM ", \"%s\", "
                      FMT_D16 ", " FMT_U_MEM ", " FMT_D16 ", "
                      FMT_U_MEM ")\n",
                      (memSizeType) connectionHandle,
                      sqlwstriAsUnquotedCStri(inConnectionString),
                      stringLength1,
                      (memSizeType) outConnectionString,
                      bufferLength,
                      (memSizeType) stringLength2Ptr););
    funcResult = SQLBrowseConnectW(connectionHandle,
                                   inConnectionString, stringLength1,
                                   outConnectionString, bufferLength,
                                   stringLength2Ptr);
    logCliFunc(printf("SQLBrowseConnectW(" FMT_U_MEM
                      ", \"%s\", " FMT_D16,
                      (memSizeType) connectionHandle,
                      sqlwstriAsUnquotedCStri(inConnectionString),
                      stringLength1);
                printf(", \"%s\", " FMT_D16 ", "
                      FMT_D16 ") --> " FMT_D16 "\n",
                      sqlwstriAsUnquotedCStri(outConnectionString),
                      bufferLength, *stringLength2Ptr, funcResult););
    return funcResult;
  } /* my_SQLBrowseConnectW */
#endif



static SQLRETURN my_SQLColAttributeW (SQLHSTMT statementHandle,
    SQLUSMALLINT columnNumber, SQLUSMALLINT fieldIdentifier,
    SQLPOINTER characterAttribute, SQLSMALLINT bufferLength,
    SQLSMALLINT *stringLengthPtr, SQLLEN *numericAttribute)

  {
    SQLRETURN funcResult;

  /* my_SQLColAttributeW */
    logCliFunc(printf("SQLColAttributeW(" FMT_U_MEM ", " FMT_U16
                      ", " FMT_U16 ", " FMT_U_MEM ", " FMT_D16 ", "
                      FMT_U_MEM ", " FMT_U_MEM ")\n",
                      (memSizeType) statementHandle,
                      columnNumber, fieldIdentifier,
                      (memSizeType) characterAttribute,
                      bufferLength, (memSizeType) stringLengthPtr,
                      (memSizeType) numericAttribute););
    funcResult = SQLColAttributeW(statementHandle, columnNumber,
                                  fieldIdentifier, characterAttribute,
                                  bufferLength, stringLengthPtr,
                                  numericAttribute);
    logCliFunc(printf("SQLColAttributeW(" FMT_U_MEM ", " FMT_U16
                      ", " FMT_U16 ", " FMT_U_MEM ", " FMT_D16 ", "
                      FMT_D16 ", " FMT_D64 ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle,
                      columnNumber, fieldIdentifier,
                      (memSizeType) characterAttribute,
                      bufferLength, *stringLengthPtr,
                      *numericAttribute, funcResult););
    return funcResult;
  } /* my_SQLColAttributeW */



static SQLRETURN my_SQLConnectW (SQLHDBC connectionHandle,
    SQLWCHAR *serverName, SQLSMALLINT nameLength1,
    SQLWCHAR *userName, SQLSMALLINT nameLength2,
    SQLWCHAR *authentication, SQLSMALLINT nameLength3)

  {
    SQLRETURN funcResult;

  /* my_SQLConnectW */
    logCliFunc(printf("SQLConnectW(" FMT_U_MEM ", \"%s\", " FMT_D16,
                      (memSizeType) connectionHandle,
                      sqlwstriAsUnquotedCStri(serverName),
                      nameLength1);
                printf(", \"%s\", " FMT_D16,
                      sqlwstriAsUnquotedCStri(userName),
                      nameLength2);
                printf(", \"%s\", " FMT_D16 ")\n",
                      sqlwstriAsUnquotedCStri(authentication),
                      nameLength3););
    funcResult = SQLConnectW(connectionHandle, serverName,
                             nameLength1, userName, nameLength2,
                             authentication, nameLength3);
    logCliFunc(printf("SQLConnectW(" FMT_U_MEM ", \"%s\", " FMT_D16,
                      (memSizeType) connectionHandle,
                      sqlwstriAsUnquotedCStri(serverName),
                      nameLength1);
                printf(", \"%s\", " FMT_D16,
                      sqlwstriAsUnquotedCStri(userName),
                      nameLength2);
                printf(", \"%s\", " FMT_D16 ") --> " FMT_D16 "\n",
                      sqlwstriAsUnquotedCStri(authentication),
                      nameLength3, funcResult););
    return funcResult;
  } /* my_SQLConnectW */



#ifdef ODBC_DRIVER_FUNCTIONS_NEEDED
static SQLRETURN my_SQLDataSources (SQLHENV environmentHandle,
    SQLUSMALLINT direction, SQLCHAR *serverName,
    SQLSMALLINT bufferLength1, SQLSMALLINT *nameLength1,
    SQLCHAR *description, SQLSMALLINT bufferLength2,
    SQLSMALLINT *nameLength2)

  {
    SQLRETURN funcResult;

  /* my_SQLDataSources */
    logCliFunc(printf("SQLDataSources(" FMT_U_MEM ", " FMT_U16
                      ", " FMT_U_MEM ", " FMT_D16 ", " FMT_U_MEM
                      ", " FMT_U_MEM ", " FMT_D16 ", " FMT_U_MEM
                      ")\n",
                      (memSizeType) environmentHandle,
                      direction, (memSizeType) serverName,
                      bufferLength1, (memSizeType) nameLength1,
                      (memSizeType) description, bufferLength2,
                      (memSizeType) nameLength2););
    funcResult = SQLDataSources(environmentHandle, direction,
                                serverName, bufferLength1,
                                nameLength1, description,
                                bufferLength2, nameLength2);
    logCliFunc(printf("SQLDataSources(" FMT_U_MEM ", " FMT_U16
                      ", \"%s\", " FMT_D16 ", " FMT_D16,
                      (memSizeType) environmentHandle,
                      direction, sqlwstriAsUnquotedCStri(serverName),
                      bufferLength1, *nameLength1);
               printf(", \"%s\", " FMT_D16 ", " FMT_D16
                      ") --> " FMT_D16 "\n",
                      sqlwstriAsUnquotedCStri(description),
                      bufferLength2, *nameLength2, funcResult););
    return funcResult;
  } /* my_SQLDataSources */
#endif



static SQLRETURN my_SQLDescribeColW (SQLHSTMT statementHandle,
    SQLUSMALLINT columnNumber, SQLWCHAR *columnName,
    SQLSMALLINT bufferLength, SQLSMALLINT *nameLengthPtr,
    SQLSMALLINT *dataTypePtr, SQLULEN *columnSizePtr,
    SQLSMALLINT *decimalDigitsPtr, SQLSMALLINT *nullablePtr)

  {
    SQLRETURN funcResult;

  /* my_SQLDescribeColW */
    logCliFunc(printf("SQLDescribeColW(" FMT_U_MEM ", " FMT_U16 ", "
                      FMT_U_MEM ", " FMT_D16 ", " FMT_U_MEM ", "
                      FMT_U_MEM ", " FMT_U_MEM ", " FMT_U_MEM ", "
                      FMT_U_MEM ")\n",
                      (memSizeType) statementHandle, columnNumber,
                      (memSizeType) columnName, bufferLength,
                      (memSizeType) nameLengthPtr,
                      (memSizeType) dataTypePtr,
                      (memSizeType) columnSizePtr,
                      (memSizeType) decimalDigitsPtr,
                      (memSizeType) nullablePtr););
    funcResult = SQLDescribeColW(statementHandle, columnNumber,
                                 columnName, bufferLength,
                                 nameLengthPtr, dataTypePtr,
                                 columnSizePtr, decimalDigitsPtr,
                                 nullablePtr);
    logCliFunc(printf("SQLDescribeColW(" FMT_U_MEM ", " FMT_U16
                      ", \"%s\", " FMT_D16 ", " FMT_D16 ", "
                      FMT_D16 ", " FMT_U64 ", " FMT_D16 ", "
                      FMT_D16 ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle, columnNumber,
                      sqlwstriAsUnquotedCStri(columnName),
                      bufferLength, *nameLengthPtr, *dataTypePtr,
                      *columnSizePtr, *decimalDigitsPtr,
                      *nullablePtr, funcResult););
    return funcResult;
  } /* my_SQLDescribeColW */



static SQLRETURN my_SQLDescribeParam (SQLHSTMT statementHandle,
    SQLUSMALLINT parameterNumber, SQLSMALLINT *dataTypePtr,
    SQLULEN *parameterSizePtr, SQLSMALLINT *decimalDigitsPtr,
    SQLSMALLINT *nullablePtr)

  {
    SQLRETURN funcResult;

  /* my_SQLDescribeParam */
    logCliFunc(printf("SQLDescribeParam(" FMT_U_MEM ", " FMT_U16
                      ", " FMT_U_MEM ", " FMT_U_MEM ", " FMT_U_MEM
                      ", " FMT_U_MEM ")\n",
                      (memSizeType) statementHandle, parameterNumber,
                      (memSizeType) dataTypePtr,
                      (memSizeType) parameterSizePtr,
                      (memSizeType) decimalDigitsPtr,
                      (memSizeType) nullablePtr););
    funcResult = SQLDescribeParam(statementHandle, parameterNumber,
                                  dataTypePtr, parameterSizePtr,
                                  decimalDigitsPtr, nullablePtr);
    logCliFunc(printf("SQLDescribeParam(" FMT_U_MEM ", " FMT_U16
                      ", " FMT_D16 ", " FMT_U64 ", " FMT_D16
                      ", " FMT_D16 ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle, parameterNumber,
                      *dataTypePtr, *parameterSizePtr,
                      *decimalDigitsPtr, *nullablePtr, funcResult););
    return funcResult;
  } /* my_SQLDescribeParam */



static SQLRETURN my_SQLDisconnect (SQLHDBC connectionHandle)

  {
    SQLRETURN funcResult;

  /* my_SQLDisconnect */
    logCliFunc(printf("SQLDisconnect(" FMT_U_MEM ")\n",
                      (memSizeType) connectionHandle););
    funcResult = SQLDisconnect(connectionHandle);
    logCliFunc(printf("SQLDisconnect(" FMT_U_MEM ") --> " FMT_D16 "\n",
                      (memSizeType) connectionHandle, funcResult););
    return funcResult;
  } /* my_SQLDisconnect */



static SQLRETURN my_SQLDriverConnectW (SQLHDBC connectionHandle,
    SQLHWND windowHandle, SQLWCHAR *inConnectionString,
    SQLSMALLINT stringLength1, SQLWCHAR *outConnectionString,
    SQLSMALLINT bufferLength, SQLSMALLINT *stringLength2Ptr,
    SQLUSMALLINT driverCompletion)

  {
    SQLRETURN funcResult;

  /* my_SQLDriverConnectW */
    logCliFunc(printf("SQLDriverConnectW(" FMT_U_MEM ", " FMT_U_MEM
                      ", \"%s\", " FMT_D16 ", " FMT_U_MEM ", "
                      FMT_D16 ", " FMT_U_MEM ", " FMT_U16 ")\n",
                      (memSizeType) connectionHandle,
                      (memSizeType) windowHandle,
                      sqlwstriAsUnquotedCStri(inConnectionString),
                      stringLength1,
                      (memSizeType) outConnectionString,
                      bufferLength, (memSizeType) stringLength2Ptr,
                      driverCompletion););
    funcResult = SQLDriverConnectW(connectionHandle, windowHandle,
                                   inConnectionString, stringLength1,
                                   outConnectionString, bufferLength,
                                   stringLength2Ptr,
                                   driverCompletion);
    logCliFunc(printf("SQLDriverConnectW(" FMT_U_MEM ", " FMT_U_MEM
                      ", \"%s\", " FMT_D16,
                      (memSizeType) connectionHandle,
                      (memSizeType) windowHandle,
                      sqlwstriAsUnquotedCStri(inConnectionString),
                      stringLength1);
               printf(", \"%s\", " FMT_D16 ", " FMT_D16
                      ", " FMT_U16 ") --> " FMT_D16 "\n",
                      sqlwstriAsUnquotedCStri(outConnectionString),
                      bufferLength, *stringLength2Ptr,
                      driverCompletion, funcResult););
    return funcResult;
  } /* my_SQLDriverConnectW */



#ifdef ODBC_DRIVER_FUNCTIONS_NEEDED
static SQLRETURN my_SQLDriversW (SQLHENV environmentHandle,
    SQLUSMALLINT direction, SQLWCHAR *driverDescription,
    SQLSMALLINT bufferLength1, SQLSMALLINT *descriptionLengthPtr,
    SQLWCHAR *driverAttributes, SQLSMALLINT bufferLength2,
    SQLSMALLINT *attributesLengthPtr)

  {
    SQLRETURN funcResult;

  /* my_SQLDriversW */
    logCliFunc(printf("SQLDriversW(" FMT_U_MEM ", " FMT_U16 ", "
                      FMT_U_MEM ", " FMT_D16 ", " FMT_U_MEM ", "
                      FMT_U_MEM ", " FMT_D16 ", " FMT_U_MEM ")\n",
                      (memSizeType) environmentHandle, direction,
                      (memSizeType) driverDescription, bufferLength1,
                      (memSizeType) descriptionLengthPtr,
                      (memSizeType) driverAttributes, bufferLength2,
                      (memSizeType) attributesLengthPtr););
    funcResult = SQLDriversW(environmentHandle, direction,
                             driverDescription, bufferLength1,
                             descriptionLengthPtr, driverAttributes,
                             bufferLength2, attributesLengthPtr);
    logCliFunc(printf("SQLDriversW(" FMT_U_MEM ", " FMT_U16
                      ", \"%s\", " FMT_D16 ", " FMT_D16,
                      (memSizeType) environmentHandle, direction,
                      sqlwstriAsUnquotedCStri(driverDescription),
                      bufferLength1, *descriptionLengthPtr);
               printf(", \"%s\", " FMT_D16 ", " FMT_D16
                      ") --> " FMT_D16 "\n",
                      sqlwstriAsUnquotedCStri(driverAttributes),
                      bufferLength2, *attributesLengthPtr,
                      funcResult););
    return funcResult;
  } /* my_SQLDriversW */
#endif



static SQLRETURN my_SQLEndTran (SQLSMALLINT handleType,
    SQLHANDLE handle, SQLSMALLINT completionType)

  {
    SQLRETURN funcResult;

  /* my_SQLEndTran */
    logCliFunc(printf("SQLEndTran(" FMT_D16 ", " FMT_U_MEM ", "
                      FMT_D16 ")\n",
                      handleType, (memSizeType) handle,
                      completionType););
    funcResult = SQLEndTran(handleType, handle, completionType);
    logCliFunc(printf("SQLEndTran(" FMT_D16 ", " FMT_U_MEM ", "
                      FMT_D16 ") --> " FMT_D16 "\n",
                      handleType, (memSizeType) handle,
                      completionType, funcResult););
    return funcResult;
  } /* my_SQLEndTran */



static SQLRETURN my_SQLExecute (SQLHSTMT statementHandle)

  {
    SQLRETURN funcResult;

  /* my_SQLExecute */
    logCliFunc(printf("SQLExecute(" FMT_U_MEM ")\n",
                      (memSizeType) statementHandle););
    funcResult = SQLExecute(statementHandle);
    logCliFunc(printf("SQLExecute(" FMT_U_MEM ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle, funcResult););
    return funcResult;
  } /* my_SQLExecute */



static SQLRETURN my_SQLFetch (SQLHSTMT statementHandle)

  {
    SQLRETURN funcResult;

  /* my_SQLFetch */
    logCliFunc(printf("SQLFetch(" FMT_U_MEM ")\n",
                      (memSizeType) statementHandle););
    funcResult = SQLFetch(statementHandle);
    logCliFunc(printf("SQLFetch(" FMT_U_MEM ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle, funcResult););
    return funcResult;
  } /* my_SQLFetch */



static SQLRETURN my_SQLFreeHandle (SQLSMALLINT handleType,
    SQLHANDLE handle)

  {
    SQLRETURN funcResult;

  /* my_SQLFreeHandle */
    logCliFunc(printf("SQLFreeHandle(" FMT_D16 ", " FMT_U_MEM ")\n",
                      handleType, (memSizeType) handle););
    funcResult = SQLFreeHandle(handleType, handle);
    logCliFunc(printf("SQLFreeHandle(" FMT_D16 ", " FMT_U_MEM
                      ") --> " FMT_D16 "\n",
                      handleType, (memSizeType) handle,
                      funcResult););
    return funcResult;
  } /* my_SQLFreeHandle */



static SQLRETURN my_SQLFreeStmt (SQLHSTMT statementHandle,
    SQLUSMALLINT option)

  {
    SQLRETURN funcResult;

  /* my_SQLFreeStmt */
    logCliFunc(printf("SQLFreeStmt(" FMT_U_MEM ", " FMT_U16 ")\n",
                      (memSizeType) statementHandle, option););
    funcResult = SQLFreeStmt(statementHandle, option);
    logCliFunc(printf("SQLFreeStmt(" FMT_U_MEM ", " FMT_U16
                      ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle, option,
                      funcResult););
    return funcResult;
  } /* my_SQLFreeStmt */



static SQLRETURN my_SQLGetConnectAttrW (SQLHDBC connectionHandle,
    SQLINTEGER attribute, SQLPOINTER valuePtr,
    SQLINTEGER bufferLength, SQLINTEGER *stringLengthPtr)

  {
    SQLRETURN funcResult;

  /* my_SQLGetConnectAttrW */
    logCliFunc(printf("SQLGetConnectAttrW(" FMT_U_MEM ", "
                      FMT_D32 ", " FMT_U_MEM ", " FMT_D32 ", "
                      FMT_U_MEM ")\n",
                      (memSizeType) connectionHandle, attribute,
                      (memSizeType) valuePtr, bufferLength,
                      (memSizeType) stringLengthPtr););
    funcResult = SQLGetConnectAttrW(connectionHandle, attribute,
                                    valuePtr, bufferLength,
                                    stringLengthPtr);
    logCliFunc(printf("SQLGetConnectAttrW(" FMT_U_MEM ", "
                      FMT_D32 ", " FMT_U_MEM ", " FMT_D32 ", "
                      FMT_D32 ") --> " FMT_D16 "\n",
                      (memSizeType) connectionHandle, attribute,
                      (memSizeType) valuePtr, bufferLength,
                      *stringLengthPtr, funcResult););
    return funcResult;
  } /* my_SQLGetConnectAttrW */



static SQLRETURN my_SQLGetData (SQLHSTMT statementHandle,
    SQLUSMALLINT columnNumber, SQLSMALLINT targetType,
    SQLPOINTER targetValue, SQLLEN bufferLength,
    SQLLEN *strLen_or_Ind)

  {
    SQLRETURN funcResult;

  /* my_SQLGetData */
    logCliFunc(printf("SQLGetData(" FMT_U_MEM ", " FMT_U16 ", "
                      FMT_D16 ", " FMT_U_MEM ", " FMT_D64 ", "
                      FMT_U_MEM ")\n",
                      (memSizeType) statementHandle, columnNumber,
                      targetType, (memSizeType) targetValue,
                      bufferLength, (memSizeType) strLen_or_Ind););
    funcResult = SQLGetData(statementHandle, columnNumber, targetType,
                            targetValue, bufferLength, strLen_or_Ind);
    logCliFunc(printf("SQLGetData(" FMT_U_MEM ", " FMT_U16 ", "
                      FMT_D16 ", " FMT_U_MEM ", " FMT_D64 ", "
                      FMT_U_MEM ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle, columnNumber,
                      targetType, (memSizeType) targetValue,
                      bufferLength, (memSizeType) strLen_or_Ind,
                      funcResult););
    return funcResult;
  } /* my_SQLGetData */



static SQLRETURN my_SQLGetDiagRecW (SQLSMALLINT handleType,
    SQLHANDLE handle, SQLSMALLINT recNumber, SQLWCHAR *sqlstate,
    SQLINTEGER *nativeError, SQLWCHAR *messageText,
    SQLSMALLINT bufferLength, SQLSMALLINT *textLength)

  {
    SQLRETURN funcResult;

  /* my_SQLGetDiagRecW */
    logCliFunc(printf("SQLGetDiagRecW(" FMT_D16 ", " FMT_U_MEM
                      ", " FMT_D16 ", " FMT_U_MEM ", " FMT_U_MEM
                      ", " FMT_U_MEM ", " FMT_D16 ", " FMT_U_MEM ")\n",
                      handleType, (memSizeType) handle, recNumber,
                      (memSizeType) sqlstate,
                      (memSizeType) nativeError,
                      (memSizeType) messageText, bufferLength,
                      (memSizeType) textLength););
    funcResult = SQLGetDiagRecW(handleType, handle, recNumber,
                                sqlstate, nativeError, messageText,
                                bufferLength, textLength);
    logCliFunc(printf("SQLGetDiagRecW(" FMT_D16 ", " FMT_U_MEM
                      ", " FMT_D16 ", \"%s\", " FMT_D32,
                      handleType, (memSizeType) handle, recNumber,
                      sqlwstriAsUnquotedCStri(sqlstate),
                      *nativeError);
                printf(", \"%s\", " FMT_D16 ", " FMT_D16
                      ") --> " FMT_D16 "\n",
                      sqlwstriAsUnquotedCStri(messageText),
                      bufferLength, *textLength, funcResult););
    return funcResult;
  } /* my_SQLGetDiagRecW */



static SQLRETURN my_SQLGetInfoW (SQLHDBC connectionHandle,
    SQLUSMALLINT infoType, SQLPOINTER infoValuePtr,
    SQLSMALLINT bufferLength, SQLSMALLINT *stringLengthPtr)

  {
    SQLRETURN funcResult;

  /* my_SQLGetInfoW */
    logCliFunc(printf("SQLGetInfoW(" FMT_U_MEM ", " FMT_U16 ", "
                      FMT_U_MEM ", "FMT_D16 ", " FMT_U_MEM ")\n",
                      (memSizeType) connectionHandle, infoType,
                      (memSizeType) infoValuePtr, bufferLength,
                      (memSizeType) stringLengthPtr););
    funcResult = SQLGetInfoW(connectionHandle, infoType, infoValuePtr,
                             bufferLength, stringLengthPtr);
    logCliFunc(printf("SQLGetInfoW(" FMT_U_MEM ", " FMT_U16 ", "
                      FMT_U_MEM ", "FMT_D16 ", " FMT_U_MEM ") --> "
                      FMT_D16 "\n",
                      (memSizeType) connectionHandle, infoType,
                      (memSizeType) infoValuePtr, bufferLength,
                      (memSizeType) stringLengthPtr, funcResult););
    return funcResult;
  } /* my_SQLGetInfoW */



static SQLRETURN my_SQLGetStmtAttrW (SQLHSTMT statementHandle,
    SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER bufferLength,
    SQLINTEGER *stringLength)

  {
    SQLRETURN funcResult;

  /* my_SQLGetStmtAttrW */
    logCliFunc(printf("SQLGetStmtAttrW(" FMT_U_MEM ", " FMT_D32 ", "
                      FMT_U_MEM ", " FMT_D32 ", " FMT_U_MEM ")\n",
                      (memSizeType) statementHandle, attribute,
                      (memSizeType) value, bufferLength,
                      (memSizeType) stringLength););
    funcResult = SQLGetStmtAttrW(statementHandle, attribute, value,
                                 bufferLength, stringLength);
    logCliFunc(printf("SQLGetStmtAttrW(" FMT_U_MEM ", " FMT_D32 ", "
                      FMT_U_MEM ", " FMT_D32 ", " FMT_D32 ") -> "
                      FMT_D16 "\n",
                      (memSizeType) statementHandle, attribute,
                      (memSizeType) value, bufferLength,
                      *stringLength, funcResult););
    return funcResult;
  } /* my_SQLGetStmtAttrW */



static SQLRETURN my_SQLGetTypeInfoW (SQLHSTMT statementHandle,
    SQLSMALLINT dataType)

  {
    SQLRETURN funcResult;

  /* my_SQLGetTypeInfoW */
    logCliFunc(printf("SQLGetTypeInfoW(" FMT_U_MEM ", "
                      FMT_D16 ")\n",
                      (memSizeType) statementHandle,
                      dataType););
    funcResult = SQLGetTypeInfoW(statementHandle, dataType);
    logCliFunc(printf("SQLGetTypeInfoW(" FMT_U_MEM ", "
                      FMT_D16 ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle,
                      dataType, funcResult););
    return funcResult;
  } /* my_SQLGetTypeInfoW */



static SQLRETURN my_SQLNumParams (SQLHSTMT statementHandle,
    SQLSMALLINT *parameterCountPtr)

  {
    SQLRETURN funcResult;

  /* my_SQLNumParams */
    logCliFunc(printf("SQLNumParams(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                      (memSizeType) statementHandle,
                      (memSizeType) parameterCountPtr););
    funcResult = SQLNumParams(statementHandle, parameterCountPtr);
    logCliFunc(printf("SQLNumParams(" FMT_U_MEM ", " FMT_D16
                      ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle,
                      *parameterCountPtr, funcResult););
    return funcResult;
  } /* my_SQLNumParams */



static SQLRETURN my_SQLNumResultCols (SQLHSTMT statementHandle,
    SQLSMALLINT *columnCount)

  {
    SQLRETURN funcResult;

  /* my_SQLNumResultCols */
    logCliFunc(printf("SQLNumResultCols(" FMT_U_MEM ", "
                      FMT_U_MEM ")\n",
                      (memSizeType) statementHandle,
                      (memSizeType) columnCount););
    funcResult = SQLNumResultCols(statementHandle, columnCount);
    logCliFunc(printf("SQLNumResultCols(" FMT_U_MEM ", "
                      FMT_U_MEM ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle,
                      (memSizeType) columnCount, funcResult););
    return funcResult;
  } /* my_SQLNumResultCols */



static SQLRETURN my_SQLPrepareW (SQLHSTMT statementHandle,
    SQLWCHAR *statementText, SQLINTEGER textLength)

  {
    SQLRETURN funcResult;

  /* my_SQLPrepareW */
    logCliFunc(printf("SQLPrepareW(" FMT_U_MEM ", \"%s\", "
                      FMT_D32 ")\n",
                      (memSizeType) statementHandle,
                      sqlwstriAsUnquotedCStri(statementText),
                      textLength););
    funcResult = SQLPrepareW(statementHandle, statementText,
                             textLength);
    logCliFunc(printf("SQLPrepareW(" FMT_U_MEM ", \"%s\", "
                      FMT_D32 ") --> " FMT_D16 "\n",
                      (memSizeType) statementHandle,
                      sqlwstriAsUnquotedCStri(statementText),
                      textLength, funcResult););
    return funcResult;
  } /* my_SQLPrepareW */



static SQLRETURN my_SQLSetConnectAttrW (SQLHDBC connectionHandle,
    SQLINTEGER attribute, SQLPOINTER valuePtr,
    SQLINTEGER stringLength)

  {
    SQLRETURN funcResult;

  /* my_SQLSetConnectAttrW */
    logCliFunc(printf("SQLSetConnectAttrW(" FMT_U_MEM ", " FMT_D32
                      ", " FMT_U_MEM ", " FMT_D32 ")\n",
                      (memSizeType) connectionHandle, attribute,
                      (memSizeType) valuePtr, stringLength););
    funcResult = SQLSetConnectAttrW(connectionHandle, attribute,
                                    valuePtr, stringLength);
    logCliFunc(printf("SQLSetConnectAttrW(" FMT_U_MEM ", " FMT_D32
                      ", " FMT_U_MEM ", " FMT_D32 ") --> "
                      FMT_D16 "\n",
                      (memSizeType) connectionHandle, attribute,
                      (memSizeType) valuePtr, stringLength,
                      funcResult););
    return funcResult;
  } /* my_SQLSetConnectAttrW */



static SQLRETURN my_SQLSetDescFieldW (SQLHDESC descriptorHandle,
    SQLSMALLINT recNumber, SQLSMALLINT fieldIdentifier,
    SQLPOINTER value, SQLINTEGER bufferLength)

  {
    SQLRETURN funcResult;

  /* my_SQLSetDescFieldW */
    logCliFunc(printf("SQLSetDescFieldW(" FMT_U_MEM ", " FMT_D16
                      ", " FMT_D16 ", " FMT_U_MEM ", " FMT_D32 ")\n",
                      (memSizeType) descriptorHandle, recNumber,
                      fieldIdentifier, (memSizeType) value,
                      bufferLength););
    funcResult = SQLSetDescFieldW(descriptorHandle, recNumber,
                                  fieldIdentifier, value,
                                  bufferLength);
    logCliFunc(printf("SQLSetDescFieldW(" FMT_U_MEM ", " FMT_D16
                      ", " FMT_D16 ", " FMT_U_MEM ", " FMT_D32
                      ") --> " FMT_D16 "\n",
                      (memSizeType) descriptorHandle, recNumber,
                      fieldIdentifier, (memSizeType) value,
                      bufferLength, funcResult););
    return funcResult;
  } /* my_SQLSetDescFieldW */



static SQLRETURN my_SQLSetEnvAttr (SQLHENV environmentHandle,
    SQLINTEGER attribute, SQLPOINTER value, SQLINTEGER stringLength)

  {
    SQLRETURN funcResult;

  /* my_SQLSetEnvAttr */
    logCliFunc(printf("SQLSetEnvAttr(" FMT_U_MEM ", " FMT_D32 ", "
                      FMT_U_MEM ", " FMT_D32 ")\n",
                      (memSizeType) environmentHandle, attribute,
                      (memSizeType) value, stringLength););
    funcResult = SQLSetEnvAttr(environmentHandle, attribute,
                               value, stringLength);
    logCliFunc(printf("SQLSetEnvAttr(" FMT_U_MEM ", " FMT_D32 ", "
                      FMT_U_MEM ", " FMT_D32 ") --> " FMT_D16 "\n",
                      (memSizeType) environmentHandle, attribute,
                      (memSizeType) value, stringLength,
                      funcResult););
    return funcResult;
  } /* my_SQLSetEnvAttr */



#ifdef CLI_DLL
#undef SQLAllocHandle
#undef SQLBindCol
#undef SQLBindParameter
#undef SQLBrowseConnectW
#undef SQLColAttributeW
#undef SQLConnectW
#undef SQLDataSources
#undef SQLDescribeColW
#undef SQLDescribeParam
#undef SQLDisconnect
#undef SQLDriverConnectW
#undef SQLDriversW
#undef SQLEndTran
#undef SQLExecute
#undef SQLFetch
#undef SQLFreeHandle
#undef SQLFreeStmt
#undef SQLGetConnectAttrW
#undef SQLGetData
#undef SQLGetDiagRecW
#undef SQLGetInfoW
#undef SQLGetStmtAttrW
#undef SQLGetTypeInfoW
#undef SQLNumParams
#undef SQLNumResultCols
#undef SQLPrepareW
#undef SQLSetConnectAttrW
#undef SQLSetDescFieldW
#undef SQLSetEnvAttr
#endif

#define SQLAllocHandle     my_SQLAllocHandle
#define SQLBindCol         my_SQLBindCol
#define SQLBindParameter   my_SQLBindParameter
#define SQLBrowseConnectW  my_SQLBrowseConnectW
#define SQLColAttributeW   my_SQLColAttributeW
#define SQLConnectW        my_SQLConnectW
#define SQLDataSources     my_SQLDataSources
#define SQLDescribeColW    my_SQLDescribeColW
#define SQLDescribeParam   my_SQLDescribeParam
#define SQLDisconnect      my_SQLDisconnect
#define SQLDriverConnectW  my_SQLDriverConnectW
#define SQLDriversW        my_SQLDriversW
#define SQLEndTran         my_SQLEndTran
#define SQLExecute         my_SQLExecute
#define SQLFetch           my_SQLFetch
#define SQLFreeHandle      my_SQLFreeHandle
#define SQLFreeStmt        my_SQLFreeStmt
#define SQLGetConnectAttrW my_SQLGetConnectAttrW
#define SQLGetData         my_SQLGetData
#define SQLGetDiagRecW     my_SQLGetDiagRecW
#define SQLGetInfoW        my_SQLGetInfoW
#define SQLGetStmtAttrW    my_SQLGetStmtAttrW
#define SQLGetTypeInfoW    my_SQLGetTypeInfoW
#define SQLNumParams       my_SQLNumParams
#define SQLNumResultCols   my_SQLNumResultCols
#define SQLPrepareW        my_SQLPrepareW
#define SQLSetConnectAttrW my_SQLSetConnectAttrW
#define SQLSetDescFieldW   my_SQLSetDescFieldW
#define SQLSetEnvAttr      my_SQLSetEnvAttr
