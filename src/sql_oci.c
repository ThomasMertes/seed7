/********************************************************************/
/*                                                                  */
/*  sql_oci.c     Database access functions for OCI.                */
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
/*  File: seed7/src/sql_oci.c                                       */
/*  Changes: 2013, 2014, 2015, 2017 - 2020  Thomas Mertes           */
/*  Content: Database access functions for OCI.                     */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "math.h"
#include "float.h"
#include "limits.h"
#ifdef OCI_INCLUDE
#include OCI_INCLUDE
#endif

#include "common.h"
#include "data_rtl.h"
#include "striutl.h"
#include "heaputl.h"
#include "numutl.h"
#include "int_rtl.h"
#include "flt_rtl.h"
#include "str_rtl.h"
#include "tim_rtl.h"
#include "cmd_rtl.h"
#include "big_drv.h"
#include "rtl_err.h"
#include "dll_drv.h"
#include "sql_base.h"
#include "sql_drv.h"

#ifdef OCI_INCLUDE


typedef struct {
    uintType     usage_count;
    boolType     isOpen;
    sqlFuncType  sqlFunc;
    intType      driver;
    OCIEnv      *oci_environment;
    OCIServer   *oci_server;
    OCIError    *oci_error;
    OCISession  *oci_session;
    OCISvcCtx   *oci_service_context;
    ub2          charSetId;
    boolType     autoCommit;
  } dbRecordOci, *dbType;

typedef struct {
    OCIBind     *bind_handle;
    uint16Type   buffer_type;
    memSizeType  buffer_length;
    memSizeType  buffer_capacity;
    void        *buffer;
    void        *descriptor;
    int16Type    indicator;
    boolType     bound;
  } bindDataRecordOci, *bindDataType;

typedef struct {
    OCIParam    *column_handle;
    OCIDefine   *define_handle;
    uint16Type   buffer_type;
    memSizeType  buffer_length;
    void        *buffer;
    uint16Type   length;
    memSizeType  long_data_buf_usage;
    uint32Type   long_data_piece_size;
    uint16Type   long_data_return_code;
    void        *descriptor;
    OCIRef      *ref;
    int16Type    indicator;
  } resultDataRecordOci, *resultDataType;

typedef struct {
    uintType       usage_count;
    dbType         db;
    sqlFuncType    sqlFunc;
    OCIEnv        *oci_environment;
    OCIError      *oci_error;
    OCISvcCtx     *oci_service_context;
    ub2            charSetId;
    OCIStmt       *ppStmt;
    uint16Type     statementType;
    memSizeType    param_array_size;
    bindDataType   param_array;
    memSizeType    result_array_size;
    resultDataType result_array;
    boolType       executeSuccessful;
    boolType       fetchOkay;
    boolType       fetchFinished;
  } preparedStmtRecordOci, *preparedStmtType;

typedef struct {
    memSizeType tnsNameLength;
    cstriType tnsName;
    memSizeType connectStringServiceLength;
    cstriType connectStringService;
    memSizeType connectStringSidLength;
    cstriType connectStringSid;
  } connectDataRecordOci, *connectDataType;

static sqlFuncType sqlFunc = NULL;

#define DEFAULT_PORT 1521
#define CONNECT_STRING_SERVICE_TEMPLATE "(DESCRIPTION=(ADDRESS_LIST=" \
            "(ADDRESS=(PROTOCOL=TCP)(HOST=%s)(PORT=%s)))" \
            "(CONNECT_DATA=(SERVER=DEDICATED)(SERVICE_NAME=%s)))"
#define CONNECT_STRING_SID_TEMPLATE "(DESCRIPTION=(ADDRESS_LIST=" \
            "(ADDRESS=(PROTOCOL=TCP)(HOST=%s)(PORT=%s)))" \
            "(CONNECT_DATA=(SERVER=DEDICATED)(SID=%s)))"
#define MAX_BIND_VAR_SIZE 4
#define MIN_BIND_VAR_NUM 0
#define MAX_BIND_VAR_NUM (26 * 26 * 26 - 1)
#define SIZEOF_SQLT_NUM 22
#define SIZEOF_SQLT_TIMESTAMP_TZ 13
#define MANTISSA_LEN 40
#define LONG_DATA_BUFFER_SIZE_INCREMENT 4096
#define ERROR_MESSAGE_BUFFER_SIZE 1024
#define DEBUG_LONG_CALLBACK 0
#define SHOW_DETAILS 0

#define SET_NUMBER_TO_POSITIVE_INFINITY(buffer, length) \
        length = 2; \
        ((unsigned char *) (buffer))[0] = 255; \
        ((unsigned char *) (buffer))[1] = 101;

#define SET_NUMBER_TO_NEGATIVE_INFINITY(buffer, length) \
        length = 1; \
        ((unsigned char *) (buffer))[0] = 0;

/* Not a Number (NaN), defined by this driver. */
#define SET_NUMBER_TO_NAN(buffer, length) \
        length = 1; \
        ((unsigned char *) (buffer))[0] = 255;


#ifdef OCI_DLL
typedef sword (*tp_OCIAttrGet) (const void *trgthndlp, ub4 trghndltyp,
                                void *attributep, ub4 *sizep, ub4 attrtype,
                                OCIError *errhp);
typedef sword (*tp_OCIAttrSet) (void *trgthndlp, ub4 trghndltyp, void *attributep,
                                ub4 size, ub4 attrtype, OCIError *errhp);
typedef sword (*tp_OCIBindByPos) (OCIStmt *stmtp, OCIBind **bindp, OCIError *errhp,
                                  ub4 position, void *valuep, sb4 value_sz,
                                  ub2 dty, void *indp, ub2 *alenp, ub2 *rcodep,
                                  ub4 maxarr_len, ub4 *curelep, ub4 mode);
typedef sword (*tp_OCIDateTimeConstruct) (void *hndl,OCIError *err,OCIDateTime *datetime,
                                          sb2 yr,ub1 mnth,ub1 dy,ub1 hr,ub1 mm,ub1 ss,ub4 fsec,
                                          OraText *timezone, size_t timezone_length);
typedef sword (*tp_OCIDateTimeGetDate) (void *hndl, OCIError *err,  const OCIDateTime *date,
                                        sb2 *yr, ub1 *mnth, ub1 *dy);
typedef sword (*tp_OCIDateTimeGetTime) (void *hndl, OCIError *err, OCIDateTime *datetime,
                                        ub1 *hr, ub1 *mm, ub1 *ss, ub4 *fsec);
typedef sword (*tp_OCIDateTimeGetTimeZoneOffset) (void *hndl, OCIError *err,
                                                  OCIDateTime *datetime, sb1 *hour, sb1 *min);
typedef sword (*tp_OCIDefineByPos) (OCIStmt *stmtp, OCIDefine **defnp, OCIError *errhp,
                                    ub4 position, void *valuep, sb4 value_sz, ub2 dty,
                                    void *indp, ub2 *rlenp, ub2 *rcodep, ub4 mode);
typedef sword (*tp_OCIDefineDynamic) (OCIDefine *defnp, OCIError *errhp, void *octxp,
                                      OCICallbackDefine ocbfp);
typedef sword (*tp_OCIDefineObject) (OCIDefine *defnp, OCIError *errhp,
                                     const OCIType *type, void **pgvpp,
                                     ub4 *pvszsp, void **indpp, ub4 *indszp);
typedef sword (*tp_OCIDescriptorAlloc) (const void *parenth, void **descpp,
                                        const ub4 type, const size_t xtramem_sz,
                                        void **usrmempp);
typedef sword (*tp_OCIDescriptorFree) (void *descp, const ub4 type);
typedef sword (*tp_OCIEnvCreate) (OCIEnv **envp, ub4 mode, void *ctxp,
                                  void *(*malocfp)(void *ctxp, size_t size),
                                  void *(*ralocfp)(void *ctxp, void *memptr, size_t newsize),
                                  void   (*mfreefp)(void *ctxp, void *memptr),
                                  size_t xtramem_sz, void **usrmempp);
typedef sword (*tp_OCIEnvNlsCreate) (OCIEnv **envp, ub4 mode, void *ctxp,
                                     void *(*malocfp)(void *ctxp, size_t size),
                                     void *(*ralocfp)(void *ctxp, void *memptr, size_t newsize),
                                     void   (*mfreefp)(void *ctxp, void *memptr),
                                     size_t xtramem_sz, void **usrmempp,
                                     ub2 charset, ub2 ncharset);
typedef sword (*tp_OCIErrorGet) (void *hndlp, ub4 recordno, OraText *sqlstate,
                                 sb4 *errcodep, OraText *bufp, ub4 bufsiz, ub4 type);
typedef sword (*tp_OCIHandleAlloc) (const void *parenth, void **hndlpp, const ub4 type,
                                    const size_t xtramem_sz, void **usrmempp);
typedef sword (*tp_OCIHandleFree) (void *hndlp, const ub4 type);
typedef sword (*tp_OCIIntervalSetDaySecond) (void *hndl, OCIError *err, sb4 dy, sb4 hr,
                                             sb4 mm, sb4 ss, sb4 fsec, OCIInterval *result);
typedef sword (*tp_OCIIntervalSetYearMonth) (void *hndl, OCIError *err, sb4 yr, sb4 mnth,
                                             OCIInterval *result);
typedef sword (*tp_OCIIntervalGetDaySecond) (void *hndl, OCIError *err, sb4 *dy, sb4 *hr,
                                             sb4 *mm, sb4 *ss, sb4 *fsec, const OCIInterval *result);
typedef sword (*tp_OCIIntervalGetYearMonth) (void *hndl, OCIError *err, sb4 *yr, sb4 *mnth,
                                             const OCIInterval *result);
typedef sword (*tp_OCILobCreateTemporary) (OCISvcCtx *svchp,
                                           OCIError *errhp,
                                           OCILobLocator *locp,
                                           ub2 csid,
                                           ub1 csfrm,
                                           ub1 lobtype,
                                           boolean cache,
                                           OCIDuration duration);
typedef sword (*tp_OCILobFreeTemporary) (OCISvcCtx *svchp,
                                         OCIError *errhp,
                                         OCILobLocator *locp);
typedef sword (*tp_OCILobGetLength2) (OCISvcCtx *svchp, OCIError *errhp,
                                      OCILobLocator *locp, oraub8 *lenp);
typedef sword (*tp_OCILobIsTemporary) (OCIEnv *envp,
                                       OCIError *errhp,
                                       OCILobLocator *locp,
                                       boolean *is_temporary);
typedef sword (*tp_OCILobRead2) (OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                                 oraub8 *byte_amtp, oraub8 *char_amtp, oraub8 offset,
                                 void *bufp, oraub8 bufl, ub1 piece, void *ctxp,
                                 OCICallbackLobRead2 cbfp, ub2 csid, ub1 csfrm);
typedef sword (*tp_OCILobTrim2) (OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                                 oraub8 newlen);
typedef sword (*tp_OCILobWrite2) (OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                                  oraub8 *byte_amtp, oraub8 *char_amtp, oraub8 offset,
                                  void *bufp, oraub8 buflen, ub1 piece, void *ctxp,
                                  OCICallbackLobWrite2 cbfp, ub2 csid, ub1 csfrm);
typedef ub2 (*tp_OCINlsCharSetNameToId) (void *envhp, const oratext *name);
typedef sword (*tp_OCINumberToReal) (OCIError *err, const OCINumber *number,
                                     uword rsl_length, void *rsl);
typedef sword (*tp_OCIParamGet) (const void *hndlp, ub4 htype, OCIError *errhp,
                                 void **parmdpp, ub4 pos);
typedef ub4 (*tp_OCIRefHexSize) (OCIEnv *env, const OCIRef *ref);
typedef sword (*tp_OCIRefToHex) (OCIEnv *env, OCIError *err, const OCIRef *ref,
                                 oratext *hex, ub4 *hex_length);
typedef sword (*tp_OCIRowidToChar) (OCIRowid *rowidDesc, OraText *outbfp, ub2 *outbflp,
                                    OCIError *errhp);
typedef sword (*tp_OCIServerAttach) (OCIServer *srvhp, OCIError *errhp,
                                     const OraText *dblink, sb4 dblink_len, ub4 mode);
typedef sword (*tp_OCIServerDetach) (OCIServer *srvhp, OCIError *errhp, ub4 mode);
typedef sword (*tp_OCISessionBegin) (OCISvcCtx *svchp, OCIError *errhp, OCISession *usrhp,
                                     ub4 credt, ub4 mode);
typedef sword (*tp_OCISessionEnd) (OCISvcCtx *svchp, OCIError *errhp, OCISession *usrhp,
                                   ub4 mode);
typedef sword (*tp_OCIStmtExecute) (OCISvcCtx *svchp, OCIStmt *stmtp, OCIError *errhp,
                                    ub4 iters, ub4 rowoff, const OCISnapshot *snap_in,
                                    OCISnapshot *snap_out, ub4 mode);
typedef sword (*tp_OCIStmtFetch2) (OCIStmt *stmtp, OCIError *errhp, ub4 nrows,
                                   ub2 orientation, sb4 scrollOffset, ub4 mode);
typedef sword (*tp_OCIStmtPrepare) (OCIStmt *stmtp, OCIError *errhp, const OraText *stmt,
                                    ub4 stmt_len, ub4 language, ub4 mode);
typedef oratext *(*tp_OCIStringPtr) (OCIEnv *env, const OCIString *vs);
typedef ub4 (*tp_OCIStringSize) (OCIEnv *env, const OCIString *vs);
typedef sword (*tp_OCITransCommit) (OCISvcCtx *svchp, OCIError *errhp, ub4 flags);
typedef sword (*tp_OCITransRollback) (dvoid *svchp,  OCIError *errhp, ub4 flags);

static tp_OCIAttrGet                   ptr_OCIAttrGet;
static tp_OCIAttrSet                   ptr_OCIAttrSet;
static tp_OCIBindByPos                 ptr_OCIBindByPos;
static tp_OCIDateTimeConstruct         ptr_OCIDateTimeConstruct;
static tp_OCIDateTimeGetDate           ptr_OCIDateTimeGetDate;
static tp_OCIDateTimeGetTime           ptr_OCIDateTimeGetTime;
static tp_OCIDateTimeGetTimeZoneOffset ptr_OCIDateTimeGetTimeZoneOffset;
static tp_OCIDefineByPos               ptr_OCIDefineByPos;
static tp_OCIDefineDynamic             ptr_OCIDefineDynamic;
static tp_OCIDefineObject              ptr_OCIDefineObject;
static tp_OCIDescriptorAlloc           ptr_OCIDescriptorAlloc;
static tp_OCIDescriptorFree            ptr_OCIDescriptorFree;
static tp_OCIEnvCreate                 ptr_OCIEnvCreate;
static tp_OCIEnvNlsCreate              ptr_OCIEnvNlsCreate;
static tp_OCIErrorGet                  ptr_OCIErrorGet;
static tp_OCIHandleAlloc               ptr_OCIHandleAlloc;
static tp_OCIHandleFree                ptr_OCIHandleFree;
static tp_OCIIntervalSetDaySecond      ptr_OCIIntervalSetDaySecond;
static tp_OCIIntervalSetYearMonth      ptr_OCIIntervalSetYearMonth;
static tp_OCIIntervalGetDaySecond      ptr_OCIIntervalGetDaySecond;
static tp_OCIIntervalGetYearMonth      ptr_OCIIntervalGetYearMonth;
static tp_OCILobCreateTemporary        ptr_OCILobCreateTemporary;
static tp_OCILobFreeTemporary          ptr_OCILobFreeTemporary;
static tp_OCILobGetLength2             ptr_OCILobGetLength2;
static tp_OCILobIsTemporary            ptr_OCILobIsTemporary;
static tp_OCILobRead2                  ptr_OCILobRead2;
static tp_OCILobTrim2                  ptr_OCILobTrim2;
static tp_OCILobWrite2                 ptr_OCILobWrite2;
static tp_OCINlsCharSetNameToId        ptr_OCINlsCharSetNameToId;
static tp_OCINumberToReal              ptr_OCINumberToReal;
static tp_OCIParamGet                  ptr_OCIParamGet;
static tp_OCIRefHexSize                ptr_OCIRefHexSize;
static tp_OCIRefToHex                  ptr_OCIRefToHex;
static tp_OCIRowidToChar               ptr_OCIRowidToChar;
static tp_OCIServerAttach              ptr_OCIServerAttach;
static tp_OCIServerDetach              ptr_OCIServerDetach;
static tp_OCISessionBegin              ptr_OCISessionBegin;
static tp_OCISessionEnd                ptr_OCISessionEnd;
static tp_OCIStmtExecute               ptr_OCIStmtExecute;
static tp_OCIStmtFetch2                ptr_OCIStmtFetch2;
static tp_OCIStmtPrepare               ptr_OCIStmtPrepare;
static tp_OCIStringPtr                 ptr_OCIStringPtr;
static tp_OCIStringSize                ptr_OCIStringSize;
static tp_OCITransCommit               ptr_OCITransCommit;
static tp_OCITransRollback             ptr_OCITransRollback;

#define OCIAttrGet                   ptr_OCIAttrGet
#define OCIAttrSet                   ptr_OCIAttrSet
#define OCIBindByPos                 ptr_OCIBindByPos
#define OCIDateTimeConstruct         ptr_OCIDateTimeConstruct
#define OCIDateTimeGetDate           ptr_OCIDateTimeGetDate
#define OCIDateTimeGetTime           ptr_OCIDateTimeGetTime
#define OCIDateTimeGetTimeZoneOffset ptr_OCIDateTimeGetTimeZoneOffset
#define OCIDefineByPos               ptr_OCIDefineByPos
#define OCIDefineDynamic             ptr_OCIDefineDynamic
#define OCIDefineObject              ptr_OCIDefineObject
#define OCIDescriptorAlloc           ptr_OCIDescriptorAlloc
#define OCIDescriptorFree            ptr_OCIDescriptorFree
#define OCIEnvCreate                 ptr_OCIEnvCreate
#define OCIEnvNlsCreate              ptr_OCIEnvNlsCreate
#define OCIErrorGet                  ptr_OCIErrorGet
#define OCIHandleAlloc               ptr_OCIHandleAlloc
#define OCIHandleFree                ptr_OCIHandleFree
#define OCIIntervalSetDaySecond      ptr_OCIIntervalSetDaySecond
#define OCIIntervalSetYearMonth      ptr_OCIIntervalSetYearMonth
#define OCIIntervalGetDaySecond      ptr_OCIIntervalGetDaySecond
#define OCIIntervalGetYearMonth      ptr_OCIIntervalGetYearMonth
#define OCILobCreateTemporary        ptr_OCILobCreateTemporary
#define OCILobFreeTemporary          ptr_OCILobFreeTemporary
#define OCILobGetLength2             ptr_OCILobGetLength2
#define OCILobIsTemporary            ptr_OCILobIsTemporary
#define OCILobRead2                  ptr_OCILobRead2
#define OCILobTrim2                  ptr_OCILobTrim2
#define OCILobWrite2                 ptr_OCILobWrite2
#define OCINlsCharSetNameToId        ptr_OCINlsCharSetNameToId
#define OCINumberToReal              ptr_OCINumberToReal
#define OCIParamGet                  ptr_OCIParamGet
#define OCIRefHexSize                ptr_OCIRefHexSize
#define OCIRefToHex                  ptr_OCIRefToHex
#define OCIRowidToChar               ptr_OCIRowidToChar
#define OCIServerAttach              ptr_OCIServerAttach
#define OCIServerDetach              ptr_OCIServerDetach
#define OCISessionBegin              ptr_OCISessionBegin
#define OCISessionEnd                ptr_OCISessionEnd
#define OCIStmtExecute               ptr_OCIStmtExecute
#define OCIStmtFetch2                ptr_OCIStmtFetch2
#define OCIStmtPrepare               ptr_OCIStmtPrepare
#define OCIStringPtr                 ptr_OCIStringPtr
#define OCIStringSize                ptr_OCIStringSize
#define OCITransCommit               ptr_OCITransCommit
#define OCITransRollback             ptr_OCITransRollback



static boolType setupDll (const char *dllName)

  {
    static void *dbDll = NULL;

  /* setupDll */
    logFunction(printf("setupDll(\"%s\")\n", dllName););
    if (dbDll == NULL) {
      dbDll = dllOpen(dllName);
      if (dbDll != NULL) {
        if ((OCIAttrGet                   = (tp_OCIAttrGet)                   dllFunc(dbDll, "OCIAttrGet"))                   == NULL ||
            (OCIAttrSet                   = (tp_OCIAttrSet)                   dllFunc(dbDll, "OCIAttrSet"))                   == NULL ||
            (OCIBindByPos                 = (tp_OCIBindByPos)                 dllFunc(dbDll, "OCIBindByPos"))                 == NULL ||
            (OCIDateTimeConstruct         = (tp_OCIDateTimeConstruct)         dllFunc(dbDll, "OCIDateTimeConstruct"))         == NULL ||
            (OCIDateTimeGetDate           = (tp_OCIDateTimeGetDate)           dllFunc(dbDll, "OCIDateTimeGetDate"))           == NULL ||
            (OCIDateTimeGetTime           = (tp_OCIDateTimeGetTime)           dllFunc(dbDll, "OCIDateTimeGetTime"))           == NULL ||
            (OCIDateTimeGetTimeZoneOffset = (tp_OCIDateTimeGetTimeZoneOffset) dllFunc(dbDll, "OCIDateTimeGetTimeZoneOffset")) == NULL ||
            (OCIDefineByPos               = (tp_OCIDefineByPos)               dllFunc(dbDll, "OCIDefineByPos"))               == NULL ||
            (OCIDefineDynamic             = (tp_OCIDefineDynamic)             dllFunc(dbDll, "OCIDefineDynamic"))             == NULL ||
            (OCIDefineObject              = (tp_OCIDefineObject)              dllFunc(dbDll, "OCIDefineObject"))              == NULL ||
            (OCIDescriptorAlloc           = (tp_OCIDescriptorAlloc)           dllFunc(dbDll, "OCIDescriptorAlloc"))           == NULL ||
            (OCIDescriptorFree            = (tp_OCIDescriptorFree)            dllFunc(dbDll, "OCIDescriptorFree"))            == NULL ||
            (OCIEnvCreate                 = (tp_OCIEnvCreate)                 dllFunc(dbDll, "OCIEnvCreate"))                 == NULL ||
            (OCIEnvNlsCreate              = (tp_OCIEnvNlsCreate)              dllFunc(dbDll, "OCIEnvNlsCreate"))              == NULL ||
            (OCIErrorGet                  = (tp_OCIErrorGet)                  dllFunc(dbDll, "OCIErrorGet"))                  == NULL ||
            (OCIHandleAlloc               = (tp_OCIHandleAlloc)               dllFunc(dbDll, "OCIHandleAlloc"))               == NULL ||
            (OCIHandleFree                = (tp_OCIHandleFree)                dllFunc(dbDll, "OCIHandleFree"))                == NULL ||
            (OCIIntervalSetDaySecond      = (tp_OCIIntervalSetDaySecond)      dllFunc(dbDll, "OCIIntervalSetDaySecond"))      == NULL ||
            (OCIIntervalSetYearMonth      = (tp_OCIIntervalSetYearMonth)      dllFunc(dbDll, "OCIIntervalSetYearMonth"))      == NULL ||
            (OCIIntervalGetDaySecond      = (tp_OCIIntervalGetDaySecond)      dllFunc(dbDll, "OCIIntervalGetDaySecond"))      == NULL ||
            (OCIIntervalGetYearMonth      = (tp_OCIIntervalGetYearMonth)      dllFunc(dbDll, "OCIIntervalGetYearMonth"))      == NULL ||
            (OCILobCreateTemporary        = (tp_OCILobCreateTemporary)        dllFunc(dbDll, "OCILobCreateTemporary"))        == NULL ||
            (OCILobFreeTemporary          = (tp_OCILobFreeTemporary)          dllFunc(dbDll, "OCILobFreeTemporary"))          == NULL ||
            (OCILobGetLength2             = (tp_OCILobGetLength2)             dllFunc(dbDll, "OCILobGetLength2"))             == NULL ||
            (OCILobIsTemporary            = (tp_OCILobIsTemporary)            dllFunc(dbDll, "OCILobIsTemporary"))            == NULL ||
            (OCILobRead2                  = (tp_OCILobRead2)                  dllFunc(dbDll, "OCILobRead2"))                  == NULL ||
            (OCILobTrim2                  = (tp_OCILobTrim2)                  dllFunc(dbDll, "OCILobTrim2"))                  == NULL ||
            (OCILobWrite2                 = (tp_OCILobWrite2)                 dllFunc(dbDll, "OCILobWrite2"))                 == NULL ||
            (OCINlsCharSetNameToId        = (tp_OCINlsCharSetNameToId)        dllFunc(dbDll, "OCINlsCharSetNameToId"))        == NULL ||
            (OCINumberToReal              = (tp_OCINumberToReal)              dllFunc(dbDll, "OCINumberToReal"))              == NULL ||
            (OCIParamGet                  = (tp_OCIParamGet)                  dllFunc(dbDll, "OCIParamGet"))                  == NULL ||
            (OCIRefHexSize                = (tp_OCIRefHexSize)                dllFunc(dbDll, "OCIRefHexSize"))                == NULL ||
            (OCIRefToHex                  = (tp_OCIRefToHex)                  dllFunc(dbDll, "OCIRefToHex"))                  == NULL ||
            (OCIRowidToChar               = (tp_OCIRowidToChar)               dllFunc(dbDll, "OCIRowidToChar"))               == NULL ||
            (OCIServerAttach              = (tp_OCIServerAttach)              dllFunc(dbDll, "OCIServerAttach"))              == NULL ||
            (OCIServerDetach              = (tp_OCIServerDetach)              dllFunc(dbDll, "OCIServerDetach"))              == NULL ||
            (OCISessionBegin              = (tp_OCISessionBegin)              dllFunc(dbDll, "OCISessionBegin"))              == NULL ||
            (OCISessionEnd                = (tp_OCISessionEnd)                dllFunc(dbDll, "OCISessionEnd"))                == NULL ||
            (OCIStmtExecute               = (tp_OCIStmtExecute)               dllFunc(dbDll, "OCIStmtExecute"))               == NULL ||
            (OCIStmtFetch2                = (tp_OCIStmtFetch2)                dllFunc(dbDll, "OCIStmtFetch2"))                == NULL ||
            (OCIStmtPrepare               = (tp_OCIStmtPrepare)               dllFunc(dbDll, "OCIStmtPrepare"))               == NULL ||
            (OCIStringPtr                 = (tp_OCIStringPtr)                 dllFunc(dbDll, "OCIStringPtr"))                 == NULL ||
            (OCIStringSize                = (tp_OCIStringSize)                dllFunc(dbDll, "OCIStringSize"))                == NULL ||
            (OCITransCommit               = (tp_OCITransCommit)               dllFunc(dbDll, "OCITransCommit"))               == NULL ||
            (OCITransRollback             = (tp_OCITransRollback)             dllFunc(dbDll, "OCITransRollback"))             == NULL) {
          dbDll = NULL;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setupDll --> %d\n", dbDll != NULL););
    return dbDll != NULL;
  } /* setupDll */



static boolType findDll (void)

  {
    const char *dllList[] = { OCI_DLL };
    unsigned int pos;
    boolType found = FALSE;

  /* findDll */
    for (pos = 0; pos < sizeof(dllList) / sizeof(char *) && !found; pos++) {
      found = setupDll(dllList[pos]);
    } /* for */
    if (!found) {
      dllErrorMessage("sqlOpenOci", "findDll", dllList,
                      sizeof(dllList) / sizeof(char *));
    } /* if */
    return found;
  } /* findDll */

#else

#define findDll() TRUE

#endif



static void sqlClose (databaseType database);



#if VERBOSE_EXCEPTIONS || VERBOSE_EXCEPTIONS_EVERYWHERE
static void printError (OCIError *oci_error)

  {
    sb4 errcode;
    char messageText[ERROR_MESSAGE_BUFFER_SIZE];

  /* printError */
    if (OCIErrorGet(oci_error, 1, NULL, &errcode, (OraText *) messageText,
                    ERROR_MESSAGE_BUFFER_SIZE, OCI_HTYPE_ERROR) != OCI_SUCCESS) {
      printf(" *** Failed to get error description.\n");
    } else {
      printf("errcode: " FMT_D32 "\n", errcode);
      printf("%s\n", messageText);
    } /* if */
  } /* printError */
#endif



static void setDbErrorMsg (const char *funcName, const char *dbFuncName,
    OCIError *oci_error)

  {
    sb4 errCode;

  /* setDbErrorMsg */
    dbError.funcName = funcName;
    dbError.dbFuncName = dbFuncName;
    if (OCIErrorGet(oci_error, 1, NULL, &errCode,
                    (OraText *) dbError.message,
                    DB_ERR_MESSAGE_SIZE, OCI_HTYPE_ERROR) != OCI_SUCCESS) {
      dbError.errorCode = 0;
      snprintf(dbError.message, DB_ERR_MESSAGE_SIZE,
               " *** Failed to get error description.");
    } else {
      dbError.errorCode = errCode;
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
    FREE_RECORD2(db, dbRecordOci, count.database, count.database_bytes);
    logFunction(printf("freeDatabase -->\n"););
  } /* freeDatabase */



static void freeDescriptor (preparedStmtType preparedStmt,
    void *descriptor, uint16Type buffer_type)

  {
    boolean is_temporary;

  /* freeDescriptor */
    switch (buffer_type) {
      case SQLT_RDD:
        OCIDescriptorFree(descriptor, OCI_DTYPE_ROWID);  /* ROWID */
        break;
      case SQLT_BLOB:
      case SQLT_CLOB:
        if (unlikely(OCILobIsTemporary(preparedStmt->oci_environment,
                                       preparedStmt->oci_error,
                                       (OCILobLocator *) descriptor,
                                       &is_temporary) != OCI_SUCCESS)) {
          setDbErrorMsg("freeDescriptor", "OCILobIsTemporary",
                        preparedStmt->oci_error);
          logError(printf("freeDescriptor: OCILobIsTemporary:\n%s\n",
                          dbError.message););
          raise_error(DATABASE_ERROR);
        } else if (is_temporary) {
          if (unlikely(OCILobFreeTemporary(preparedStmt->oci_service_context,
                                           preparedStmt->oci_error,
                                           (OCILobLocator *) descriptor) != OCI_SUCCESS)) {
            setDbErrorMsg("freeDescriptor", "OCILobFreeTemporary",
                          preparedStmt->oci_error);
            logError(printf("freeDescriptor: OCILobFreeTemporary:\n%s\n",
                            dbError.message););
            raise_error(DATABASE_ERROR);
          } /* if */
        } /* if */
        OCIDescriptorFree(descriptor, OCI_DTYPE_LOB);  /* Binary LOB or Character LOB */
        break;
      case SQLT_FILE:
        OCIDescriptorFree(descriptor, OCI_DTYPE_FILE);  /* Binary FILE */
        break;
      case SQLT_DATE:
        OCIDescriptorFree(descriptor, OCI_DTYPE_DATE);  /* ANSI DATE */
        break;
      case SQLT_TIMESTAMP:
        OCIDescriptorFree(descriptor, OCI_DTYPE_TIMESTAMP);  /* TIMESTAMP */
        break;
      case SQLT_TIMESTAMP_TZ:
        OCIDescriptorFree(descriptor, OCI_DTYPE_TIMESTAMP_TZ);  /* TIMESTAMP WITH TIME ZONE */
        break;
      case SQLT_TIMESTAMP_LTZ:
        OCIDescriptorFree(descriptor, OCI_DTYPE_TIMESTAMP_LTZ);  /* TIMESTAMP WITH LOCAL TIME ZONE */
        break;
      case SQLT_INTERVAL_YM:
        OCIDescriptorFree(descriptor, OCI_DTYPE_INTERVAL_YM);  /* INTERVAL YEAR TO MONTH descriptor */
        break;
      case SQLT_INTERVAL_DS:
        OCIDescriptorFree(descriptor, OCI_DTYPE_INTERVAL_DS);  /* INTERVAL DAY TO SECOND descriptor */
        break;
      default:
        logError(printf("freeDescriptor: Unspecified buffer_type: %d\n", buffer_type););
        break;
    } /* switch */
  } /* freeDescriptor */



static void freeBindData (preparedStmtType preparedStmt, bindDataType bindData)

  { /* freeBindData */
    /* The bind handle is freed implicitly when the statement handle is deallocated. */
    if (bindData->buffer != NULL) {
      free(bindData->buffer);
    } /* if */
    if (bindData->descriptor != NULL) {
      freeDescriptor(preparedStmt, bindData->descriptor, bindData->buffer_type);
    } /* if */
  } /* freeBindData */



static void freeResultData (preparedStmtType preparedStmt, resultDataType resultData)

  { /* freeResultData */
    if (resultData->column_handle != NULL) {
      OCIDescriptorFree(resultData->column_handle, OCI_DTYPE_PARAM);
    } /* if */
    /* It seems that the define_handle is implicitly freed with the statement handle. */
    if (resultData->buffer != NULL && resultData->buffer != &resultData->descriptor) {
      free(resultData->buffer);
    } /* if */
    if (resultData->descriptor != NULL) {
      freeDescriptor(preparedStmt, resultData->descriptor, resultData->buffer_type);
    } /* if */
  } /* freeResultData */



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
        freeBindData(preparedStmt, &preparedStmt->param_array[pos]);
      } /* for */
      FREE_TABLE(preparedStmt->param_array, bindDataRecordOci, preparedStmt->param_array_size);
    } /* if */
    if (preparedStmt->result_array != NULL) {
      for (pos = 0; pos < preparedStmt->result_array_size; pos++) {
        freeResultData(preparedStmt, &preparedStmt->result_array[pos]);
      } /* for */
      FREE_TABLE(preparedStmt->result_array, resultDataRecordOci, preparedStmt->result_array_size);
    } /* if */
    OCIHandleFree(preparedStmt->ppStmt, OCI_HTYPE_STMT);
    if (preparedStmt->db->usage_count != 0) {
      preparedStmt->db->usage_count--;
      if (preparedStmt->db->usage_count == 0) {
        logMessage(printf("FREE " FMT_U_MEM "\n", (memSizeType) preparedStmt->db););
        freeDatabase((databaseType) preparedStmt->db);
      } /* if */
    } /* if */
    FREE_RECORD2(preparedStmt, preparedStmtRecordOci,
                 count.prepared_stmt, count.prepared_stmt_bytes);
    logFunction(printf("freePreparedStmt -->\n"););
  } /* freePreparedStmt */



/**
 *  Convert a SQLT_NUM number to a decimal integer.
 *  @param dataLen Length of the OCINumber (ociNumber[0]).
 *  @param ociNumberData Data bytes of the OCINumber (&ociNumber[1]).
 *  @param stri Destination string for the decimal digits
 *              (Needs a capacity of at least 127 characters).
 *  @param err_info Unchanged if the function succeeds, and
 *                  RANGE_ERROR if the number cannot be converted
 *                  to a decimal integer.
 */
static errInfoType sqltNumberToDecimalInt (memSizeType dataLen,
    const uint8Type *ociNumberData, striType stri)

  {
    signed char exponent;
    signed char mantissa[SIZEOF_SQLT_NUM]; /* Terminated by a negative number */
    int idx;
    int twoDigits;
    unsigned int pos = 0;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqltNumberToDecimalInt */
    if (dataLen == 0) {
      logError(printf("sqltNumberToDecimalInt: Too short\n"););
      err_info = RANGE_ERROR;
    } else if (dataLen == 1) {
      if (ociNumberData[0] == 0x80) {
        /* Zero */
        stri->mem[pos++] = '0';
      } else if (ociNumberData[0] == 0) {
        logError(printf("sqltNumberToDecimalInt: Negative infinity\n"););
        err_info = RANGE_ERROR;
      } else if (ociNumberData[0] == 0xff) {
        /* Not a Number (NaN), defined by this driver. */
        logError(printf("sqltNumberToDecimalInt: Not a Number\n"););
        err_info = RANGE_ERROR;
      } else {
        logError(printf("sqltNumberToDecimalInt: Unexpected format\n"););
        err_info = RANGE_ERROR;
      } /* if */
    } else if (dataLen == 2) {
      if (ociNumberData[0] == 255 && ociNumberData[1] == 101) {
        logError(printf("sqltNumberToDecimalInt: Positive infinity\n"););
        err_info = RANGE_ERROR;
      } /* if */
    } else if (dataLen > SIZEOF_SQLT_NUM) {
      logError(printf("sqltNumberToDecimalInt: Too long\n"););
      err_info = RANGE_ERROR;
    } /* if */
    if (pos == 0 && err_info == OKAY_NO_ERROR) {
      /* Normalize exponent and mantissa */
      if (ociNumberData[0] >= 128) {
        /* Positive number */
        exponent = (signed char) ((int) ociNumberData[0] - 193);  /* Range -65 to 62 */
        for (idx = 0; idx < (int) dataLen - 1; idx++) {
          mantissa[idx] = (signed char) (ociNumberData[idx + 1] - 1);
        } /* for */
        mantissa[idx] = -1;
      } else {
        /* Negative number */
        exponent = (signed char) (62 - (int) ociNumberData[0]);  /* Range -65 to 62 */
        for (idx = 0; idx < (int) dataLen - 1; idx++) {
          mantissa[idx] = (signed char) (101 - ociNumberData[idx + 1]);
        } /* for */
        mantissa[idx] = -1;
        stri->mem[pos++] = '-';
      } /* if */
      /* Convert exponent and mantissa to decimal integer */
      idx = 0;
      twoDigits = mantissa[idx];
      if (exponent >= 0) {
        /* integer part */
        exponent--;
        if (twoDigits / 10 != 0) {
          stri->mem[pos++] = (strElemType) ('0' + twoDigits / 10);
        } /* if */
        stri->mem[pos++] = (strElemType) ('0' + twoDigits % 10);
        idx++;
        twoDigits = mantissa[idx];
        while (exponent >= 0) {
          exponent--;
          if (twoDigits < 0) {
            stri->mem[pos++] = '0';
            stri->mem[pos++] = '0';
          } else {
            stri->mem[pos++] = (strElemType) ('0' + twoDigits / 10);
            stri->mem[pos++] = (strElemType) ('0' + twoDigits % 10);
            idx++;
            twoDigits = mantissa[idx];
          } /* if */
        } /* while */
      } else {
        stri->mem[pos++] = '0';
      } /* if */
      if (exponent < -1 || twoDigits >= 0) {
        logError(printf("sqltNumberToDecimalInt: Fractional number part present\n"););
        err_info = RANGE_ERROR;
      } /* if */
    } /* if */
    stri->size = (memSizeType) pos;
    return err_info;
  } /* sqltNumberToDecimalInt */



/**
 *  Convert an OCINumber to a decimal integer.
 *  @param ociNumber OCINumber to be converted.
 *  @param stri Destination string for the decimal digits
 *              (Needs a capacity of at least 127 characters).
 *  @param err_info Unchanged if the function succeeds, and
 *                  RANGE_ERROR if the number cannot be converted
 *                  to a decimal integer.
 */
static void ociNumberToDecimalInt (const OCINumber *ociNumber, striType stri,
    errInfoType *err_info)

  { /* ociNumberToDecimalInt */
    *err_info = sqltNumberToDecimalInt(ociNumber->OCINumberPart[0],
                                       &ociNumber->OCINumberPart[1],
                                       stri);
  } /* ociNumberToDecimalInt */



/**
 *  Convert a SQLT_NUM number to a decimal fraction (integer + scale).
 *  The number format can be used for normal numbers and infinity values.
 *  This function additionally recognizes the value NaN (Not a Number).
 *  @param dataLen Length of the OCINumber (ociNumber[0]).
 *  @param ociNumberData Data bytes of the OCINumber (&ociNumber[1]).
 *  @param stri Destination string for the decimal digits
 *              (Needs a capacity of at least 127 characters).
 *  @return the scale of the number. The scale is the number of digits
 *          to the right (positive) or left (negative) of the decimal point.
 *          For finite values the scale is greater or equal 0.
 *          For positive and negative infinite values the scale is -1.
 *  @param err_info Unchanged if the function succeeds, and
 *                  RANGE_ERROR if the number cannot be converted
 *                  to a decimal fraction.
 */
static int sqltNumberToDecimalFraction (memSizeType dataLen,
    const uint8Type *ociNumberData, striType stri, errInfoType *err_info)

  {
    signed char exponent;
    signed char mantissa[SIZEOF_SQLT_NUM]; /* Terminated by a negative number */
    int idx;
    int twoDigits;
    unsigned int pos = 0;
    int scale = 0;

  /* sqltNumberToDecimalFraction */
    if (dataLen == 0) {
      logError(printf("sqltNumberToDecimalFraction: Too short\n"););
      *err_info = RANGE_ERROR;
    } else if (dataLen == 1) {
      if (ociNumberData[0] == 0x80) {
        /* Zero */
        stri->mem[pos++] = '0';
      } else if (ociNumberData[0] == 0) {
        /* Negative infinity */
        stri->mem[pos++] = '-';
        stri->mem[pos++] = '1';
        scale = -1;
      } else if (ociNumberData[0] == 0xff) {
        /* Not a Number (NaN), defined by this driver. */
        stri->mem[pos++] = '0';
        scale = -1;
      } else {
        logError(printf("sqltNumberToDecimalFraction: Unexpected format %u\n",
                        ociNumberData[0]););
        *err_info = RANGE_ERROR;
      } /* if */
    } else if (dataLen == 2) {
      if (ociNumberData[0] == 255 && ociNumberData[1] == 101) {
        /* Positive infinity */
        stri->mem[pos++] = '1';
        scale = -1;
      } /* if */
    } else if (dataLen > SIZEOF_SQLT_NUM) {
      logError(printf("sqltNumberToDecimalFraction: Too long\n"););
      *err_info = RANGE_ERROR;
    } /* if */
    if (pos == 0 && *err_info == OKAY_NO_ERROR) {
      /* Normalize exponent and mantissa */
      if (ociNumberData[0] >= 128) {
        /* Positive number */
        exponent = (signed char) ((int) ociNumberData[0] - 193);  /* Range -65 to 62 */
        for (idx = 0; idx < (int) dataLen - 1; idx++) {
          mantissa[idx] = (signed char) (ociNumberData[idx + 1] - 1);
        } /* for */
        mantissa[idx] = -1;
      } else {
        /* Negative number */
        exponent = (signed char) (62 - (int) ociNumberData[0]);  /* Range -65 to 62 */
        for (idx = 0; idx < (int) dataLen - 1; idx++) {
          mantissa[idx] = (signed char) (101 - ociNumberData[idx + 1]);
        } /* for */
        mantissa[idx] = -1;
        stri->mem[pos++] = '-';
      } /* if */
      /* Convert exponent and mantissa to decimal integer */
      idx = 0;
      twoDigits = mantissa[idx];
      if (exponent >= 0) {
        /* integer part */
        exponent--;
        if (twoDigits / 10 != 0) {
          stri->mem[pos++] = (strElemType) ('0' + twoDigits / 10);
        } /* if */
        stri->mem[pos++] = (strElemType) ('0' + twoDigits % 10);
        idx++;
        twoDigits = mantissa[idx];
        while (exponent >= 0) {
          exponent--;
          if (twoDigits < 0) {
            stri->mem[pos++] = '0';
            stri->mem[pos++] = '0';
          } else {
            stri->mem[pos++] = (strElemType) ('0' + twoDigits / 10);
            stri->mem[pos++] = (strElemType) ('0' + twoDigits % 10);
            idx++;
            twoDigits = mantissa[idx];
          } /* if */
        } /* while */
      } else {
        scale = 2 * (-exponent - 1);
        if (twoDigits >= 0) {
          if (twoDigits / 10 != 0) {
            stri->mem[pos++] = (strElemType) ('0' + twoDigits / 10);
          } /* if */
          stri->mem[pos++] = (strElemType) ('0' + twoDigits % 10);
          scale += 2;
          idx++;
          twoDigits = mantissa[idx];
        } /* if */
      } /* if */
      /* Fractional number part */
      while (twoDigits >= 0) {
        stri->mem[pos++] = (strElemType) ('0' + twoDigits / 10);
        stri->mem[pos++] = (strElemType) ('0' + twoDigits % 10);
        scale += 2;
        idx++;
        twoDigits = mantissa[idx];
      } /* while */
      if (scale > 0 && stri->mem[pos - 1] == '0') {
        scale--;
        pos--;
      } /* if */
    } /* if */
    stri->size = (memSizeType) pos;
    return scale;
  } /* sqltNumberToDecimalFraction */



/**
 *  Convert an OCINumber to a decimal fraction (integer + scale).
 *  @param ociNumber OCINumber to be converted.
 *  @param stri Destination string for the decimal digits
 *              (Needs a capacity of at least 127 characters).
 *  @return the scale of the number. The scale is the number of digits
 *          to the right (positive) or left (negative) of the decimal point.
 *          For finite values the scale is greater or equal 0.
 *          For positive and negative infinite values the scale is -1.
 *  @param err_info Unchanged if the function succeeds, and
 *                  RANGE_ERROR if the number cannot be converted
 *                  to a decimal fraction.
 */
static int ociNumberToDecimalFraction (const OCINumber *ociNumber, striType stri,
    errInfoType *err_info)

  { /* ociNumberToDecimalFraction */
    return sqltNumberToDecimalFraction(ociNumber->OCINumberPart[0],
                                       &ociNumber->OCINumberPart[1],
                                       stri, err_info);
  } /* ociNumberToDecimalFraction */



/**
 *  Convert a decimal integer and a scale to a SQLT_NUM number.
 *  @param ociNumberData Place to which the SQLT_NUM number is written.
 *  @param decimal String with a decimal integer (with possible - sign).
 *  @param scale Number of digits to the right (positive) or
 *               left (negative) of the decimal point.
 *  @return the size of the SQLT_NUM number.
 *  @param err_info Unchanged if the function succeeds, and
 *                  RANGE_ERROR if the decimal integer + scale
 *                  cannot be converted to a SQLT_NUM number.
 */
static int sqltNumberFromDecimalInt (uint8Type *ociNumberData,
    const const_striType decimal, int scale, errInfoType *err_info)

  {
    memSizeType pos = 0;
    int negative = 0;
    int decimalExponent;
    char mantissa[MANTISSA_LEN + 1];
    int startIdx = 1;
    int idx = 1;
    int length = 0;

  /* sqltNumberFromDecimalInt */
    /* Read sign */
    if (pos < decimal->size && decimal->mem[pos] == '-') {
      pos++;
      negative = 1;
    } /* if */
    if (pos < decimal->size && (decimal->mem[pos] < '0' || decimal->mem[pos] > '9')) {
      logError(printf("sqltNumberFromDecimalInt: No digit found\n"););
      *err_info = RANGE_ERROR;
    } else {
      while (pos < decimal->size && decimal->mem[pos] == '0') {
        pos++;
      } /* while */
      /* Read decimal integer */
      while (pos < decimal->size && decimal->mem[pos] >= '0' && decimal->mem[pos] <= '9' && idx <= MANTISSA_LEN) {
        mantissa[idx] = (char) ((char) decimal->mem[pos] - '0');
        idx++;
        pos++;
      } /* while */
      while (pos < decimal->size && decimal->mem[pos] == '0') {
        idx++;
        pos++;
      } /* while */
      if (pos != decimal->size) {
        logError(printf("sqltNumberFromDecimalInt: Mantissa too long or trailing nondigit chars found\n"););
        *err_info = RANGE_ERROR;
      } else {
        decimalExponent = -scale + idx - 2;
        if (decimalExponent % 2 == 0) {
          if (idx <= MANTISSA_LEN || mantissa[MANTISSA_LEN] == 0) {
            startIdx = 0;
            mantissa[0] = 0;
          } else {
            logError(printf("sqltNumberFromDecimalInt: Mantissa too long\n"););
            *err_info = RANGE_ERROR;
          } /* if */
        } /* if */
        if (*err_info == OKAY_NO_ERROR) {
          if (idx > MANTISSA_LEN + startIdx) {
            idx = MANTISSA_LEN + startIdx;
          } /* if */
          /* Shrink mantissa size */
          while (idx > startIdx && mantissa[idx - 1] == 0) {
            idx--;
          } /* while */
          if (idx == startIdx) {
            /* Zero */
            length = 1;
            ociNumberData[0] = 0x80;
          } else if (decimalExponent > 125) {
            logError(printf("sqltNumberFromDecimalInt: Overflow: decimalExponent=%d\n",
                            decimalExponent););
            *err_info = RANGE_ERROR;
          } else if (decimalExponent < -130) {
            logError(printf("sqltNumberFromDecimalInt: Underflow: decimalExponent=%d\n",
                            decimalExponent););
            *err_info = RANGE_ERROR;
          } else {
            /* Change the base from 10 to 100 */
            if ((idx - startIdx) % 2 == 1) {
              mantissa[idx++] = 0;
            } /* if */
            length = (idx - startIdx) / 2;
            for (idx = 0; idx < length; idx++) {
              mantissa[idx] = (char) (10 * mantissa[startIdx + idx * 2] + mantissa[startIdx + idx * 2 + 1]);
            } /* for */
            /* Add terminator for negative value */
            if (negative && length < 20) {
              mantissa[length++] = (char) -1;
            } /* if */
#if RSHIFT_DOES_SIGN_EXTEND
            decimalExponent >>= 1;
#else
            if (decimalExponent < 0) {
              decimalExponent = ~(~decimalExponent >> 1);
            } else {
              decimalExponent >>= 1;
            } /* if */
#endif
            if (negative) {
              ociNumberData[0] = (ub1) (62 - decimalExponent);
              for (idx = 0; idx < length; idx++) {
                ociNumberData[idx + 1] = (ub1) (101 - mantissa[idx]);
              } /* for */
            } else {
              ociNumberData[0] = (ub1) (decimalExponent + 193);
              for (idx = 0; idx < length; idx++) {
                ociNumberData[idx + 1] = (ub1) (mantissa[idx] + 1);
              } /* for */
            } /* if */
            length++;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return length;
  } /* sqltNumberFromDecimalInt */



/**
 *  Convert a decimal integer and a scale to an OCINumber.
 *  @param ociNumber Place to which the OCINumber is written.
 *  @param decimal String with a decimal integer (with possible - sign).
 *  @param scale Number of digits to the right (positive) or
 *               left (negative) of the decimal point.
 *  @param err_info Unchanged if the function succeeds, and
 *                  RANGE_ERROR if the decimal integer + scale
 *                  cannot be converted to a SQLT_NUM number.
 */
static void ociNumberFromDecimalInt (OCINumber *ociNumber, const const_striType decimal,
    int scale, errInfoType *err_info)

  { /* ociNumberFromDecimalInt */
    ociNumber->OCINumberPart[0] = (ub1) sqltNumberFromDecimalInt(&ociNumber->OCINumberPart[1],
                                                                 decimal, scale, err_info);
  } /* ociNumberFromDecimalInt */



#if ANY_LOG_ACTIVE || SHOW_DETAILS
static void showSqltNumber (memSizeType dataLen, const uint8Type *ociNumberData)

  {
    memSizeType pos;

  /* showSqltNumber */
    printf("dataLen: " FMT_U_MEM ", \"", dataLen);
    for (pos = 0; pos < SIZEOF_SQLT_NUM; pos++) {
      printf("%02x", ociNumberData[pos]);
    } /* for */
    printf("\", ");
    if (dataLen > SIZEOF_SQLT_NUM) {
      dataLen = SIZEOF_SQLT_NUM;
    } /* if */
    if (ociNumberData[0] >= 128) {
      /* Positive number */
      printf("exponent: %d", (int) ociNumberData[0] - 193);  /* Range -65 to 62 */
      if (dataLen != 0) {
        printf(", (%02d", (int) ociNumberData[1] - 1);
        for (pos = 1; pos < dataLen - 1 && ociNumberData[pos + 1] - 1 != -1; pos++) {
          printf(" %02d", (int) ociNumberData[pos + 1] - 1);
        } /* for */
        printf(")");
      } /* if */
      printf(", positive");
    } else {
      /* Negative number */
      printf("exponent: %d", 62 - (int) ociNumberData[0]);  /* Range -65 to 62 */
      if (dataLen != 0) {
        printf(", (%02d", 101 - (int) ociNumberData[1]);
        for (pos = 1; pos < dataLen - 1 && 101 - (int) ociNumberData[pos + 1] != -1; pos++) {
          printf(" %02d", 101 - (int) ociNumberData[pos + 1]);
        } /* for */
        printf(")");
      } /* if */
      printf(", negative");
    } /* if */
  } /* showSqltNumber */
#endif



#if SHOW_DETAILS
static void printSqltNumber (memSizeType dataLen, const uint8Type *ociNumberData)

  {
    union {
      struct striStruct striBuf;
      char charBuf[SIZ_STRI(127)];
    } striBuffer;
    int scale = 0;
    errInfoType err_info = OKAY_NO_ERROR;

  /* printSqltNumber */
#if ALLOW_STRITYPE_SLICES
    striBuffer.striBuf.mem = striBuffer.striBuf.mem1;
#endif
    scale = sqltNumberToDecimalFraction(dataLen, ociNumberData,
                                        &striBuffer.striBuf, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      /* printf(" *OCINumber conversion failed*"); */
      showSqltNumber(dataLen, ociNumberData);
    } else if (scale == -1) {
      /* For positive and negative infinite values the scale is -1. */
      if (striBuffer.striBuf.mem[0] == '-') {
        printf("-Infinity");
      } else if (striBuffer.striBuf.mem[0] == '0') {
        /* Not a Number (NaN), defined by this driver. */
        printf("NaN");
      } else {
        printf("Infinity");
      } /* if */
    } else {
      /* printf(" "); */
      striAsUnquotedCStri(&striBuffer.striBuf);
      if (scale != 0) {
        printf("E%d", -scale);
      } /* if */
    } /* if */
  } /* printSqltNumber */



static void printBuffer (uint16Type buffer_type, memSizeType length, void *buffer)

  { /* printBuffer */
    switch (buffer_type) {
      case SQLT_INT:
        switch (length) {
          case 1:
            printf("%d", *(int8Type *) buffer);
            break;
          case 2:
            printf("%d", *(int16Type *) buffer);
            break;
          case 4:
            printf(FMT_D32, *(int32Type *) buffer);
            break;
          case 8:
            printf(FMT_D64, *(int64Type *) buffer);
            break;
        } /* switch */
        break;
      case SQLT_FLT:
        switch (length) {
          case FLOAT_SIZE / 8:
            printf("%f", *(float *) buffer);
            break;
          case DOUBLE_SIZE / 8:
            printf("%f", *(double *) buffer);
            break;
        } /* switch */
        break;
      case SQLT_CHR:
      case SQLT_AFC:
        fwrite(buffer, length, 1, stdout);
        break;
      case SQLT_NUM:
        printSqltNumber(length, (uint8Type *) buffer);
        break;
    } /* switch */
  } /* printBuffer */
#endif



#if ANY_LOG_ACTIVE || SHOW_DETAILS
static const char *nameOfBufferType (uint16Type buffer_type)

  {
    static char buffer[UINT16TYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    static const char *const typeNameTable[] = {
      "0",           "SQLT_CHR",    "SQLT_NUM",    "SQLT_INT",    "SQLT_FLT",
      "SQLT_STR",    "SQLT_VNU",    "SQLT_PDN",    "SQLT_LNG",    "SQLT_VCS",
      "SQLT_NON",    "SQLT_RID",    "SQLT_DAT",    "13",          "14",
      "SQLT_VBI",    "16",          "17",          "18",          "19",
      "20",          "SQLT_BFLOAT", "SQLT_BDOUBLE","SQLT_BIN",    "SQLT_LBI",
      "25",  "26",   "27",  "28",   "29",  "30",   "31",  "32",   "33",  "34",
      "35",  "36",   "37",  "38",   "39",  "40",   "41",  "42",   "43",  "44",
      "45",  "46",   "47",  "48",   "49",  "50",   "51",  "52",   "53",  "54",
      "55",  "56",   "57",  "58",   "59",  "60",   "61",  "62",   "63",  "64",
      "65",          "66",          "67",          "SQLT_UIN",    "69",
      "70",  "71",   "72",  "73",   "74",  "75",   "76",  "77",   "78",  "79",
      "80",  "81",   "82",  "83",   "84",  "85",   "86",  "87",   "88",  "89",
      "90",          "SQLT_SLS",    "92",          "93",          "SQLT_LVC",
      "SQLT_LVB",    "SQLT_AFC",    "SQLT_AVC",    "98",          "99",
      "SQLT_IBFLOAT","SQLT_IBDOUBLE","SQLT_CUR",   "103",         "SQLT_RDD",
      "SQLT_LAB",    "SQLT_OSL",    "107",         "SQLT_NTY",    "109",
      "SQLT_REF",    "111",         "SQLT_CLOB",   "SQLT_BLOB",   "SQLT_FILE",
      "SQLT_CFILE",  "SQLT_RSET",   "117",         "118",         "119",
      "120",         "121",         "SQLT_NCO",    "123",         "124",
      "125", "126",  "127", "128",  "129", "130",  "131", "132",  "133", "134",
      "135", "136",  "137", "138",  "139", "140",  "141", "142",  "143", "144",
      "145", "146",  "147", "148",  "149", "150",  "151", "152",  "153", "154",
      "SQLT_VST",    "SQLT_ODT",    "157",         "158",         "159",
      "160", "161",  "162", "163",  "164", "165",  "166", "167",  "168", "169",
      "170", "171",  "172", "173",  "174", "175",  "176", "177",  "178", "179",
      "180",         "181",         "182",         "183",         "SQLT_DATE",
      "SQLT_TIME",   "SQLT_TIME_TZ","SQLT_TIMESTAMP", "SQLT_TIMESTAMP_TZ", "SQLT_INTERVAL_YM",
      "SQLT_INTERVAL_DS", "191",    "192",         "193",         "194",
      "195", "196",  "197", "198",  "199", "200",  "201", "202",  "203", "204"
      "205", "206",  "207", "208",  "209", "210",  "211", "212",  "213", "214",
      "215", "216",  "217", "218",  "219", "220",  "221", "222",  "223", "224",
      "225", "226",  "227", "228",  "229", "230",  "231", "SQLT_TIMESTAMP_LTZ",
    };
    const char *typeName;

  /* nameOfBufferType */
    logFunction(printf("nameOfBufferType(%d)\n", buffer_type););
    if (buffer_type <= 232) {
      typeName = typeNameTable[buffer_type];
    } else {
      sprintf(buffer, "%d", buffer_type);
      typeName = buffer;
    } /* if */
    logFunction(printf("nameOfBufferType --> %s\n", typeName););
    return typeName;
  } /* nameOfBufferType */
#endif



/**
 *  Process the bind variables in a statement string.
 *  Literals and comments are processed to avoid the misinterpretation
 *  of question marks (?).
 */
static striType processStatementStri (const const_striType sqlStatementStri,
    memSizeType *numBindParameters, errInfoType *err_info)

  {
    memSizeType pos = 0;
    strElemType ch;
    strElemType delimiter;
    memSizeType destPos = 0;
    unsigned int varNum = MIN_BIND_VAR_NUM;
    striType processed;

  /* processStatementStri */
    logFunction(printf("processStatementStri(\"%s\")\n",
                       striAsUnquotedCStri(sqlStatementStri)););
    if (unlikely(sqlStatementStri->size > MAX_STRI_LEN / MAX_BIND_VAR_SIZE ||
                 !ALLOC_STRI_SIZE_OK(processed, sqlStatementStri->size * MAX_BIND_VAR_SIZE))) {
      *err_info = MEMORY_ERROR;
      processed = NULL;
    } else {
      while (pos < sqlStatementStri->size && *err_info == OKAY_NO_ERROR) {
        ch = sqlStatementStri->mem[pos];
        if (ch == '?') {
          if (varNum > MAX_BIND_VAR_NUM) {
            logError(printf("processStatementStri: Too many variables\n"););
            *err_info = RANGE_ERROR;
            FREE_STRI2(processed, sqlStatementStri->size * MAX_BIND_VAR_SIZE);
            processed = NULL;
          } else {
            processed->mem[destPos++] = ':';
            processed->mem[destPos++] = 'a' +  varNum / (26 * 26);
            processed->mem[destPos++] = 'a' + (varNum /       26) % 26;
            processed->mem[destPos++] = 'a' +  varNum             % 26;
            varNum++;
          } /* if */
          pos++;
        } else if (ch == '\'' || ch == '"') {
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
    *numBindParameters = varNum - MIN_BIND_VAR_NUM;
    logFunction(printf("processStatementStri --> \"%s\"\n",
                       striAsUnquotedCStri(processed)););
    return processed;
  } /* processStatementStri */



static sb4 longCallback (dvoid *octxp, OCIDefine *define_handle, ub4 iter,
    dvoid **bufpp, ub4 **alenp, ub1 *piecep, dvoid **indp, ub2 **rcodep)

  {
    resultDataType resultData;
    memSizeType newBufferLength;
    void *resizedBuffer;
    sb4 returnCode = OCI_CONTINUE;

  /* longCallback */
    resultData = (resultDataType) octxp;
#if DEBUG_LONG_CALLBACK
    printf("iter: %u\n", iter);
    printf("longCallback: Field: %s, ind: %d, len %u, buf_len: " FMT_U_MEM "\n",
           nameOfBufferType(resultData->buffer_type),
           resultData->indicator,
           resultData->length,
           resultData->buffer_length);
    if (bufpp == NULL) {
      printf("IN bufp: NULL\n");
    } else if (*bufpp == NULL) {
      printf("IN bufp: *NULL*\n");
    } else {
      printf("IN bufp: " FMT_U_MEM "\n", (memSizeType) *bufpp);
    } /* if */
    if (alenp == NULL) {
      printf("IN alen: NULL\n");
    } else if (*alenp == NULL) {
      printf("IN alen: *NULL*\n");
    } else {
      printf("IN alen: " FMT_U_MEM "\n", (memSizeType) **alenp);
    } /* if */
    if (piecep == NULL) {
      printf("IN piecep: NULL\n");
    } else if (*piecep == OCI_ONE_PIECE) {
      printf("IN piecep: OCI_ONE_PIECE\n");
    } else if (*piecep == OCI_FIRST_PIECE) {
      printf("IN piecep: OCI_FIRST_PIECE\n");
    } else if (*piecep == OCI_NEXT_PIECE) {
      printf("IN piecep: OCI_NEXT_PIECE\n");
    } else {
      printf("IN piecep: %u\n", *piecep);
    } /* if */
    if (indp == NULL) {
      printf("IN ind: NULL\n");
    } else if (*indp == NULL) {
      printf("IN ind: *NULL*\n");
    } else {
      printf("IN ind: %lu\n", *(unsigned long *) *indp);
    } /* if */
    printf("buffer: " FMT_U_MEM "\n", (memSizeType) resultData->buffer);
    fwrite(resultData->buffer, 1, resultData->long_data_buf_usage, stdout);
    printf("\n");
    printf("long_data_buf_usage: " FMT_U_MEM "\n", resultData->long_data_buf_usage);
#endif
    switch (*piecep) {
      case OCI_ONE_PIECE:
        /* OCI_ONE_PIECE */
        resultData->long_data_buf_usage = 0;
        break;
      case OCI_FIRST_PIECE:
        /* OCI_ONE_PIECE or OCI_FIRST_PIECE */
        resultData->long_data_buf_usage = 0;
        *piecep = OCI_FIRST_PIECE;
        break;
      case OCI_NEXT_PIECE:
        /* OCI_NEXT_PIECE or OCI_LAST_PIECE */
        *piecep = OCI_NEXT_PIECE;
        break;
    } /* switch */
    if (resultData->buffer_length > resultData->long_data_buf_usage) {
      resultData->long_data_piece_size = (uint32Type)
          (resultData->buffer_length - resultData->long_data_buf_usage);
      *bufpp = &((char *) resultData->buffer)[resultData->long_data_buf_usage];
      resultData->long_data_buf_usage = resultData->buffer_length;
      resultData->long_data_return_code = OCI_SUCCESS;
    } else if (resultData->buffer_length == resultData->long_data_buf_usage) {
      newBufferLength = resultData->buffer_length + LONG_DATA_BUFFER_SIZE_INCREMENT;
      resizedBuffer = realloc(resultData->buffer, newBufferLength);
      if (resizedBuffer != NULL) {
        resultData->buffer_length = newBufferLength;
        resultData->buffer = resizedBuffer;
        resultData->long_data_piece_size = (uint32Type)
            (resultData->buffer_length - resultData->long_data_buf_usage);
        *bufpp = &((char *) resultData->buffer)[resultData->long_data_buf_usage];
        resultData->long_data_buf_usage = resultData->buffer_length;
        resultData->long_data_return_code = OCI_SUCCESS;
      } else {
        resultData->long_data_piece_size = 0;
        *bufpp = NULL;
        resultData->long_data_return_code = OCI_NO_DATA;
        returnCode = OCI_NO_DATA;
      } /* if */
    } /* if */
    *alenp = &resultData->long_data_piece_size;
    *indp = &resultData->indicator;
    *rcodep = &resultData->long_data_return_code;
#if DEBUG_LONG_CALLBACK
    printf("long_data_piece_size: %u\n", resultData->long_data_piece_size);
    printf("buffer_length: " FMT_U_MEM "\n", resultData->buffer_length);
    if (bufpp == NULL) {
      printf("OUT bufp: NULL\n");
    } else if (*bufpp == NULL) {
      printf("OUT bufp: *NULL*\n");
    } else {
      printf("OUT bufp: " FMT_U_MEM "\n", (memSizeType) *bufpp);
    } /* if */
    if (alenp == NULL) {
      printf("OUT alen: NULL\n");
    } else if (*alenp == NULL) {
      printf("OUT alen: *NULL*\n");
    } else {
      printf("OUT alen: " FMT_U_MEM "\n", (memSizeType) **alenp);
    } /* if */
    if (piecep == NULL) {
      printf("OUT piecep: NULL\n");
    } else if (*piecep == OCI_ONE_PIECE) {
      printf("OUT piecep: OCI_ONE_PIECE\n");
    } else if (*piecep == OCI_FIRST_PIECE) {
      printf("OUT piecep: OCI_FIRST_PIECE\n");
    } else if (*piecep == OCI_NEXT_PIECE) {
      printf("OUT piecep: OCI_NEXT_PIECE\n");
    } else {
      printf("OUT piecep: %u\n", *piecep);
    } /* if */
    if (indp == NULL) {
      printf("OUT ind: NULL\n");
    } else if (*indp == NULL) {
      printf("OUT ind: *NULL*\n");
    } else {
      printf("OUT ind: %lu\n", *(unsigned long *) *indp);
    } /* if */
#endif
    return returnCode;
  } /* longCallback */



static errInfoType setupParameters (preparedStmtType preparedStmt,
    memSizeType numBindParameters)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupParameters */
    logFunction(printf("setupParameters\n"););
    if (numBindParameters == 0) {
      /* malloc(0) may return NULL, which would wrongly trigger a MEMORY_ERROR. */
      preparedStmt->param_array_size = 0;
      preparedStmt->param_array = NULL;
    } else if (unlikely(!ALLOC_TABLE(preparedStmt->param_array,
                                     bindDataRecordOci, numBindParameters))) {
      err_info = MEMORY_ERROR;
    } else {
      preparedStmt->param_array_size = numBindParameters;
      memset(preparedStmt->param_array, 0,
             numBindParameters * sizeof(bindDataRecordOci));
    } /* if */
    logFunction(printf("setupParameters --> %d\n", err_info););
    return err_info;
  } /* setupParameters */



static errInfoType setupResultColumn (preparedStmtType preparedStmt,
    unsigned int column_num, resultDataType resultData)

  {
    ub4 column_size = 0;
    ub4 value_size;
    ub4 descriptor_type;
    enum {USE_BUFFER, USE_DESCRIPTOR, USE_REF} strategy = USE_BUFFER;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupResultColumn */
    if (OCIParamGet(preparedStmt->ppStmt, OCI_HTYPE_STMT,
                    preparedStmt->oci_error,
                    (dvoid **) &resultData->column_handle,
                    (ub4) column_num) != OCI_SUCCESS) {
      setDbErrorMsg("setupResultColumn", "OCIParamGet",
                    preparedStmt->oci_error);
      logError(printf("setupResultColumn: OCIParamGet:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } else if (OCIAttrGet(resultData->column_handle,
                          OCI_DTYPE_PARAM,
                          (dvoid *) &resultData->buffer_type,
                          NULL, OCI_ATTR_DATA_TYPE,
                          preparedStmt->oci_error) != OCI_SUCCESS) {
      setDbErrorMsg("setupResultColumn", "OCIAttrGet",
                    preparedStmt->oci_error);
      logError(printf("setupResultColumn: OCIAttrGet OCI_ATTR_DATA_TYPE:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } else if (OCIAttrGet(resultData->column_handle,
                          OCI_DTYPE_PARAM,
                          (dvoid *) &column_size, NULL, OCI_ATTR_DATA_SIZE,
                          preparedStmt->oci_error) != OCI_SUCCESS) {
      setDbErrorMsg("setupResultColumn", "OCIAttrGet",
                    preparedStmt->oci_error);
      logError(printf("setupResultColumn: OCIAttrGet OCI_ATTR_DATA_SIZE:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } else {
      value_size = column_size;
      switch (resultData->buffer_type) {
        case SQLT_LNG: /* LONG */
        case SQLT_LBI: /* LONG RAW */
          column_size = LONG_DATA_BUFFER_SIZE_INCREMENT;
          value_size = 2147483647; /* Maximum length of LONG data: 2**31-1 */
          strategy = USE_BUFFER;
          break;
        case SQLT_RDD: /* ROWID */
          descriptor_type = OCI_DTYPE_ROWID;  /* ROWID */
          column_size = 0;
          strategy = USE_DESCRIPTOR;
          break;
        case SQLT_BLOB: /* Binary LOB */
        case SQLT_CLOB: /* Character LOB  */
          descriptor_type = OCI_DTYPE_LOB;  /* Binary LOB or Character LOB */
          column_size = 0;
          strategy = USE_DESCRIPTOR;
          break;
        case SQLT_FILE: /* Binary FILE */
          descriptor_type = OCI_DTYPE_FILE;  /* Binary FILE */
          column_size = 0;
          strategy = USE_DESCRIPTOR;
          break;
        case SQLT_DATE:
          descriptor_type = OCI_DTYPE_DATE;  /* ANSI DATE */
          column_size = 0;
          strategy = USE_DESCRIPTOR;
          break;
        case SQLT_TIMESTAMP:
          descriptor_type = OCI_DTYPE_TIMESTAMP;  /* TIMESTAMP */
          column_size = 0;
          strategy = USE_DESCRIPTOR;
          break;
        case SQLT_TIMESTAMP_TZ:
          descriptor_type = OCI_DTYPE_TIMESTAMP_TZ;  /* TIMESTAMP WITH TIME ZONE */
          column_size = 0;
          strategy = USE_DESCRIPTOR;
          break;
        case SQLT_TIMESTAMP_LTZ:
          descriptor_type = OCI_DTYPE_TIMESTAMP_LTZ;  /* TIMESTAMP WITH LOCAL TIME ZONE */
          column_size = 0;
          strategy = USE_DESCRIPTOR;
          break;
        case SQLT_INTERVAL_YM:
          descriptor_type = OCI_DTYPE_INTERVAL_YM;  /* INTERVAL YEAR TO MONTH descriptor */
          column_size = 0;
          strategy = USE_DESCRIPTOR;
          break;
        case SQLT_INTERVAL_DS:
          descriptor_type = OCI_DTYPE_INTERVAL_DS;  /* INTERVAL DAY TO SECOND descriptor */
          column_size = 0;
          strategy = USE_DESCRIPTOR;
          break;
        case SQLT_REF: /* REF */
          resultData->buffer = NULL;
          resultData->ref = NULL;
          value_size = 0;
          strategy = USE_REF;
          break;
        case SQLT_CHR: /* VARCHAR2 */
        case SQLT_NUM: /* NUMBER */
        case SQLT_STR: /* NULL-terminated STRING */
        case SQLT_VCS: /* VARCHAR */
        case SQLT_DAT: /* DATE */
        case SQLT_VBI: /* VARRAW */
        case SQLT_BFLOAT:  /* BINARY_FLOAT */
        case SQLT_BDOUBLE: /* BINARY_DOUBLE */
        case SQLT_BIN: /* RAW */
        case SQLT_LVC: /* LONG VARCHAR */
        case SQLT_LVB: /* LONG VARRAW */
        case SQLT_AFC: /* CHAR */
        case SQLT_AVC: /* CHARZ */
        case SQLT_VST: /* OCI STRING type */
          strategy = USE_BUFFER;
          break;
        case SQLT_IBFLOAT:  /* BINARY_FLOAT */
          resultData->buffer_type = SQLT_BFLOAT;
          strategy = USE_BUFFER;
          break;
        case SQLT_IBDOUBLE: /* BINARY_DOUBLE */
          resultData->buffer_type = SQLT_BDOUBLE;
          strategy = USE_BUFFER;
          break;
        default:
          logError(printf("setupResultColumn: Column %u has the unknown type %s.\n",
                          column_num, nameOfBufferType(resultData->buffer_type)););
          err_info = RANGE_ERROR;
         break;
      } /* switch */
      if (err_info == OKAY_NO_ERROR) {
        if (strategy == USE_DESCRIPTOR) {
          if (OCIDescriptorAlloc(preparedStmt->oci_environment,
                                 &resultData->descriptor,
                                 descriptor_type,
                                 0, NULL) != OCI_SUCCESS) {
            logError(printf("setupResultColumn: OCIDescriptorAlloc: Out of memory.\n"););
            err_info = MEMORY_ERROR;
          } else {
            resultData->buffer = &resultData->descriptor;
          } /* if */
        } else if (strategy == USE_BUFFER) {
          /* printf ("column_size: " FMT_U32 "\n", column_size); */
          resultData->buffer = malloc(column_size);
          if (unlikely(resultData->buffer == NULL)) {
            err_info = MEMORY_ERROR;
          } else {
            memset(resultData->buffer, 0, column_size);
          } /* if */
        } /* if */
      } /* if */
      if (err_info == OKAY_NO_ERROR) {
        resultData->buffer_length = (memSizeType) column_size;
        /* printf("Define column %u: %s, len " FMT_U_MEM "\n", column_num,
               nameOfBufferType(resultData->buffer_type),
               resultData->buffer_length); */
        if (OCIDefineByPos(preparedStmt->ppStmt,
                           &resultData->define_handle,
                           preparedStmt->oci_error,
                           column_num,
                           resultData->buffer,
                           (sb4) value_size,
                           resultData->buffer_type,
                           &resultData->indicator,
                           &resultData->length,
                           NULL,
                           resultData->buffer_type == SQLT_LNG ||
                           resultData->buffer_type == SQLT_LBI ?
                           OCI_DYNAMIC_FETCH : OCI_DEFAULT) != OCI_SUCCESS) {
                           /* OCI_DEFAULT) != OCI_SUCCESS) { */
          setDbErrorMsg("setupResultColumn", "OCIDefineByPos",
                        preparedStmt->oci_error);
          logError(printf("setupResultColumn: OCIDefineByPos:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else if (resultData->buffer_type == SQLT_LNG ||
                   resultData->buffer_type == SQLT_LBI) {
          /* printf("OCIDefineDynamic\n"); */
          if (OCIDefineDynamic(resultData->define_handle,
                               preparedStmt->oci_error,
                               resultData,
                               longCallback) != OCI_SUCCESS) {
            setDbErrorMsg("setupResultColumn", "OCIDefineDynamic",
                          preparedStmt->oci_error);
            logError(printf("setupResultColumn: OCIDefineDynamic:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } /* if */
        } else if (resultData->buffer_type == SQLT_REF) {
          /* printf("OCIDefineObject\n"); */
          if (OCIDefineObject(resultData->define_handle,
                              preparedStmt->oci_error,
                              NULL,
                              (dvoid **) &resultData->ref,
                              NULL, NULL, NULL) != OCI_SUCCESS) {
            setDbErrorMsg("setupResultColumn", "OCIDefineObject",
                          preparedStmt->oci_error);
            logError(printf("setupResultColumn: OCIDefineObject:\n%s\n",
                            dbError.message););
            err_info = DATABASE_ERROR;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return err_info;
  } /* setupResultColumn */



static errInfoType setupResult (preparedStmtType preparedStmt)

  {
    ub4 num_columns = 0;
    unsigned int column_index;
    errInfoType err_info = OKAY_NO_ERROR;

  /* setupResult */
    logFunction(printf("setupResult\n"););
    if (OCIAttrGet(preparedStmt->ppStmt,
                   OCI_HTYPE_STMT,
                   &preparedStmt->statementType,
                   NULL,
                   OCI_ATTR_STMT_TYPE,
                   preparedStmt->oci_error) != OCI_SUCCESS) {
      setDbErrorMsg("setupResult", "OCIAttrGet",
                    preparedStmt->oci_error);
      logError(printf("setupResult: OCIAttrGet OCI_ATTR_STMT_TYPE:\n%s\n",
                      dbError.message););
      err_info = DATABASE_ERROR;
    } else if (preparedStmt->statementType == OCI_STMT_SELECT) {
      if (OCIStmtExecute(preparedStmt->oci_service_context,
                         preparedStmt->ppStmt,
                         preparedStmt->oci_error,
                         0, /* iters */
                         0, NULL, NULL, OCI_DESCRIBE_ONLY) != OCI_SUCCESS) {
        setDbErrorMsg("setupResult", "OCIStmtExecute",
                      preparedStmt->oci_error);
        logError(printf("setupResult: OCIStmtExecute OCI_DESCRIBE_ONLY:\n%s\n",
                      dbError.message););
        err_info = DATABASE_ERROR;
      } else if (OCIAttrGet(preparedStmt->ppStmt, OCI_HTYPE_STMT,
                            &num_columns, NULL, OCI_ATTR_PARAM_COUNT,
                            preparedStmt->oci_error) != OCI_SUCCESS) {
        setDbErrorMsg("setupResult", "OCIAttrGet",
                      preparedStmt->oci_error);
        logError(printf("setupResult: OCIAttrGet OCI_ATTR_PARAM_COUNT:\n%s\n",
                        dbError.message););
        err_info = DATABASE_ERROR;
      } else if (num_columns == 0) {
        /* malloc(0) may return NULL, which would wrongly trigger a MEMORY_ERROR. */
        preparedStmt->result_array_size = 0;
        preparedStmt->result_array = NULL;
      } else if (!ALLOC_TABLE(preparedStmt->result_array, resultDataRecordOci, num_columns)) {
        err_info = MEMORY_ERROR;
      } else {
        preparedStmt->result_array_size = num_columns;
        memset(preparedStmt->result_array, 0, num_columns * sizeof(resultDataRecordOci));
        for (column_index = 0; column_index < num_columns &&
             err_info == OKAY_NO_ERROR; column_index++) {
          err_info = setupResultColumn(preparedStmt, column_index + 1,
                                       &preparedStmt->result_array[column_index]);
        } /* for */
      } /* if */
    } /* if */
    logFunction(printf("setupResult --> %d\n", err_info););
    return err_info;
  } /* setupResult */



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



#if SHOW_DETAILS
static void showBindVars (preparedStmtType preparedStmt)

  {
    memSizeType pos;
    sb2 year;
    ub1 month;
    ub1 day;
    ub1 hour;
    ub1 min;
    ub1 sec;
    ub4 fsec;

  /* showBindVars */
    printf("Bind variables:\n");
    for (pos = 0; pos < preparedStmt->param_array_size; pos++) {
      printf(FMT_U_MEM ": type: %s, buf_len: " FMT_U_MEM ", ",
             pos + 1,
             nameOfBufferType(preparedStmt->param_array[pos].buffer_type),
             preparedStmt->param_array[pos].buffer_length);
      if (preparedStmt->param_array[pos].buffer == NULL) {
        printf("NULL");
      } else {
        if (preparedStmt->param_array[pos].buffer_type == SQLT_TIMESTAMP_TZ) {
          OCIDateTimeGetDate(preparedStmt->oci_environment,
                             preparedStmt->oci_error,
                             (OCIDateTime *) preparedStmt->param_array[pos].buffer,
                             &year,
                             &month,
                             &day);
          OCIDateTimeGetTime(preparedStmt->oci_environment,
                             preparedStmt->oci_error,
                             (OCIDateTime *) preparedStmt->param_array[pos].buffer,
                             &hour,
                             &min,
                             &sec,
                             &fsec);
          printf("%d-%02u-%02u %02u:%02u:%02u.%06u", year, month, day, hour, min, sec, fsec);
        } else {
          printBuffer(preparedStmt->param_array[pos].buffer_type,
                      preparedStmt->param_array[pos].buffer_length,
                      preparedStmt->param_array[pos].buffer);
        } /* if */
      } /* if */
      printf("\n");
    } /* for */
  } /* showBindVars */



static void showResultVars (preparedStmtType preparedStmt)

  {
    memSizeType pos;

  /* showResultVars */
    printf("Result variables:\n");
    for (pos = 0; pos < preparedStmt->result_array_size; pos++) {
      printf(FMT_U_MEM ": type: %s, ind: %d, len: %u, buf_len: " FMT_U_MEM ", ",
             pos + 1,
             nameOfBufferType(preparedStmt->result_array[pos].buffer_type),
             preparedStmt->result_array[pos].indicator,
             preparedStmt->result_array[pos].length,
             preparedStmt->result_array[pos].buffer_length);
      printBuffer(preparedStmt->result_array[pos].buffer_type,
                  preparedStmt->result_array[pos].length,
                  preparedStmt->result_array[pos].buffer);
      printf("\n");
    } /* for */
  } /* showResultVars */

#else
#define showBindVars(preparedStmt)
#define showResultVars(preparedStmt)
#endif



static intType getInt (const void *buffer, memSizeType length)

  {
    union {
      struct striStruct striBuf;
      char charBuf[SIZ_STRI(127)];
    } striBuffer;
    errInfoType err_info = OKAY_NO_ERROR;
    intType intValue;

  /* getInt */
    logFunction(printf("getInt(buffer, " FMT_U_MEM ")\n",
                       (memSizeType) length););
#if ALLOW_STRITYPE_SLICES
    striBuffer.striBuf.mem = striBuffer.striBuf.mem1;
#endif
    err_info = sqltNumberToDecimalInt(length,
                                      (const uint8Type *) buffer,
                                      &striBuffer.striBuf);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
      intValue = 0;
    } else {
      /* printf("getInt: %s\n", striAsUnquotedCStri(&striBuffer.striBuf)); */
      intValue = intParse(&striBuffer.striBuf);
    } /* if */
    logFunction(printf("getInt --> " FMT_D "\n", intValue););
    return intValue;
  } /* getInt */



static bigIntType getBigInt (const void *buffer, memSizeType length)

  {
    union {
      struct striStruct striBuf;
      char charBuf[SIZ_STRI(127)];
    } striBuffer;
    errInfoType err_info = OKAY_NO_ERROR;
    bigIntType bigIntValue;

  /* getBigInt */
#if ALLOW_STRITYPE_SLICES
    striBuffer.striBuf.mem = striBuffer.striBuf.mem1;
#endif
    err_info = sqltNumberToDecimalInt(length,
                                      (const uint8Type *) buffer,
                                      &striBuffer.striBuf);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
      bigIntValue = NULL;
    } else {
      bigIntValue = bigParse(&striBuffer.striBuf);
    } /* if */
    return bigIntValue;
  } /* getBigInt */



static bigIntType getBigRational (const void *buffer, memSizeType length,
    bigIntType *denominator)

  {
    union {
      struct striStruct striBuf;
      char charBuf[SIZ_STRI(127)];
    } striBuffer;
    int scale;
    errInfoType err_info = OKAY_NO_ERROR;
    bigIntType numerator;

  /* getBigRational */
    logFunction(printf("getBigRational(");
                showSqltNumber(length, (const uint8Type *) buffer);
                printf(")\n"););
#if ALLOW_STRITYPE_SLICES
    striBuffer.striBuf.mem = striBuffer.striBuf.mem1;
#endif
    scale = sqltNumberToDecimalFraction(length,
                                        (const uint8Type *) buffer,
                                        &striBuffer.striBuf, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      *denominator = NULL;
      raise_error(err_info);
      numerator = NULL;
    } else {
      numerator = bigParse(&striBuffer.striBuf);
      if (numerator != NULL) {
        if (scale == -1) {
          /* For positive and negative infinite values the scale is -1. */
          /* Not a Number (NaN), defined by this driver also uses -1. */
          *denominator = bigZero();
        } else {
          *denominator = bigIPowSignedDigit(10, (intType) scale);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("getBigRational --> %s, ",
                       bigHexCStri(numerator));
                printf("%s\n", bigHexCStri(*denominator)););
    return numerator;
  } /* getBigRational */



static floatType getFloat (const void *buffer, memSizeType length)

  {
    union {
      struct striStruct striBuf;
      char charBuf[SIZ_STRI(127)];
    } striBuffer;
    int scale;
    char cstriBuf[127 + 2 + NULL_TERMINATION_LEN];
    memSizeType precision;
    memSizeType pos = 0;
    memSizeType destPos = 0;
    errInfoType err_info = OKAY_NO_ERROR;
    floatType floatValue;

  /* getFloat */
    logFunction(printf("getFloat(");
                showSqltNumber(length, (const uint8Type *) buffer);
                printf(")\n"););
#if ALLOW_STRITYPE_SLICES
    striBuffer.striBuf.mem = striBuffer.striBuf.mem1;
#endif
    scale = sqltNumberToDecimalFraction(length,
                                        (const uint8Type *) buffer,
                                        &striBuffer.striBuf, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
      floatValue = 0.0;
    } else if (scale < 0) {
      /* For positive and negative infinite values the scale is -1. */
      /* Not a Number (NaN), defined by this driver also uses -1. */
      /* printf("scale = -1: %c\n", striBuffer.striBuf.mem[0]); */
      if (striBuffer.striBuf.mem[0] == '1') {
        floatValue = POSITIVE_INFINITY;
      } else if (striBuffer.striBuf.mem[0] == '0') {
        floatValue = NOT_A_NUMBER;
      } else {
        floatValue = NEGATIVE_INFINITY;
      } /* if */
    } else {
      /* printf("digits: %s\n", striAsUnquotedCStri(&striBuffer.striBuf));
         printf("scale: %d\n", scale); */
      if (striBuffer.striBuf.mem[0] == '-') {
        cstriBuf[destPos++] = '-';
        pos++;
      } /* if */
      precision = striBuffer.striBuf.size - pos;
      if ((memSizeType) scale < precision) {
        for (; pos < striBuffer.striBuf.size - (memSizeType) scale; pos++) {
          cstriBuf[destPos++] = (char) striBuffer.striBuf.mem[pos];
        } /* for */
        cstriBuf[destPos++] = '.';
      } else {
        cstriBuf[destPos++] = '0';
        cstriBuf[destPos++] = '.';
        memset(&cstriBuf[destPos], '0', (memSizeType) scale - precision);
        destPos += (memSizeType) scale - precision;
      } /* if */
      for (; pos < striBuffer.striBuf.size; pos++) {
        cstriBuf[destPos++] = (char) striBuffer.striBuf.mem[pos];
      } /* for */
      cstriBuf[destPos] = '\0';
      /* printf("cstri: %s\n", cstriBuf); */
      floatValue = (floatType) strtod(cstriBuf, NULL);
    } /* if */
    logFunction(printf("getFloat --> " FMT_E "\n", floatValue););
    return floatValue;
  } /* getFloat */



static striType getRowid (preparedStmtType preparedStmt,
    OCIRowid *rowidDescriptor, errInfoType *err_info)

  {
    ub2 length = 0;
    striType stri;

  /* getRowid */
    logFunction(printf("getRowid(" FMT_U_MEM ", " FMT_X_MEM ")\n",
                       (memSizeType) preparedStmt,
                       (memSizeType) rowidDescriptor););
    OCIRowidToChar(rowidDescriptor, NULL, &length, preparedStmt->oci_error);
    /* printf("length: %u\n", length); */
    if (unlikely(!ALLOC_STRI_SIZE_OK(stri, length))) {
      *err_info = MEMORY_ERROR;
    } else if (OCIRowidToChar(rowidDescriptor,
                              (OraText *) stri->mem, &length,
                              preparedStmt->oci_error) != OCI_SUCCESS) {
      setDbErrorMsg("getRowid", "OCIRowidToChar",
                    preparedStmt->oci_error);
      logError(printf("getRowid: OCIRowidToChar:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
      FREE_STRI2(stri, length);
      stri = NULL;
    } else {
      stri->size = length;
      memcpy_to_strelem(stri->mem, (ustriType) stri->mem, (memSizeType) length);
    } /* if */
    return stri;
  } /* getRowid */



static striType getRef (preparedStmtType preparedStmt, OCIRef *ref,
    errInfoType *err_info)

  {
    ub4 length;
    striType stri;

  /* getRef */
    length = OCIRefHexSize(preparedStmt->oci_environment, ref);
    if (unlikely(!ALLOC_STRI_CHECK_SIZE(stri, length))) {
      *err_info = MEMORY_ERROR;
    } else if (OCIRefToHex(preparedStmt->oci_environment,
                           preparedStmt->oci_error,
                           ref, (OraText *) stri->mem,
                           &length) != OCI_SUCCESS) {
      setDbErrorMsg("getRef", "OCIRefToHex",
                    preparedStmt->oci_error);
      logError(printf("getRef: OCIRefToHex:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
      FREE_STRI2(stri, length);
      stri = NULL;
    } else {
      stri->size = length;
      memcpy_to_strelem(stri->mem, (ustriType) stri->mem,
                        (memSizeType) length);
    } /* if */
    return stri;
  } /* getRef */



static bstriType getBlob (preparedStmtType preparedStmt,
    OCILobLocator *lobLocator, errInfoType *err_info)

  {
    oraub8 lobLength;
    oraub8 byte_amount;
    bstriType bstri;

  /* getBlob */
    logFunction(printf("getBlob(" FMT_U_MEM ", " FMT_X_MEM ")\n",
                       (memSizeType) preparedStmt,
                       (memSizeType) lobLocator););
    if (OCILobGetLength2(preparedStmt->oci_service_context,
                         preparedStmt->oci_error,
                         lobLocator,
                         &lobLength) != OCI_SUCCESS) {
      setDbErrorMsg("getBlob", "OCILobGetLength2",
                    preparedStmt->oci_error);
      logError(printf("getBlob: OCILobGetLength2:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
      bstri = NULL;
    } else if (unlikely(lobLength > MAX_BSTRI_LEN ||
                        !ALLOC_BSTRI_SIZE_OK(bstri, (memSizeType) lobLength))) {
      *err_info = MEMORY_ERROR;
      bstri = NULL;
    } else {
      /* printf("lobLength: " FMT_U64 "\n", (uint64Type) lobLength); */
      byte_amount = lobLength;
      if (OCILobRead2(preparedStmt->oci_service_context,
                      preparedStmt->oci_error,
                      lobLocator,
                      &byte_amount,
                      NULL,
                      1,  /* Offsets should be greater than or equal to one. */
                      bstri->mem,
                      lobLength,
                      OCI_ONE_PIECE,
                      NULL, NULL, preparedStmt->charSetId, SQLCS_IMPLICIT) != OCI_SUCCESS) {
        setDbErrorMsg("getBlob", "OCILobRead2",
                      preparedStmt->oci_error);
        logError(printf("getBlob: OCILobRead2:\n%s\n",
                        dbError.message););
        *err_info = DATABASE_ERROR;
        FREE_BSTRI(bstri, (memSizeType) lobLength);
        bstri = NULL;
      } else {
        bstri->size = (memSizeType) lobLength;
      } /* if */
    } /* if */
    return bstri;
  } /* getBlob */



static striType getBlobAsStri (preparedStmtType preparedStmt,
    OCILobLocator *lobLocator, errInfoType *err_info)

  {
    oraub8 lobLength;
    oraub8 byte_amount;
    striType stri;

  /* getBlobAsStri */
    logFunction(printf("getBlobAsStri(" FMT_U_MEM ", " FMT_X_MEM ")\n",
                       (memSizeType) preparedStmt,
                       (memSizeType) lobLocator););
    if (OCILobGetLength2(preparedStmt->oci_service_context,
                         preparedStmt->oci_error,
                         lobLocator,
                         &lobLength) != OCI_SUCCESS) {
      setDbErrorMsg("getBlobAsStri", "OCILobGetLength2",
                    preparedStmt->oci_error);
      logError(printf("getBlobAsStri: OCILobGetLength2:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
      stri = NULL;
    } else if (unlikely(lobLength > MAX_STRI_LEN ||
                        !ALLOC_STRI_SIZE_OK(stri, (memSizeType) lobLength))) {
      *err_info = MEMORY_ERROR;
      stri = NULL;
    } else {
      /* printf("lobLength: " FMT_U64 "\n", (uint64Type) lobLength); */
      byte_amount = lobLength;
      if (OCILobRead2(preparedStmt->oci_service_context,
                      preparedStmt->oci_error,
                      lobLocator,
                      &byte_amount,
                      NULL,
                      1,  /* Offsets should be greater than or equal to one. */
                      stri->mem,
                      lobLength,
                      OCI_ONE_PIECE,
                      NULL, NULL, preparedStmt->charSetId, SQLCS_IMPLICIT) != OCI_SUCCESS) {
        setDbErrorMsg("getBlobAsStri", "OCILobRead2",
                      preparedStmt->oci_error);
        logError(printf("getBlobAsStri: OCILobRead2:\n%s\n",
                        dbError.message););
        *err_info = DATABASE_ERROR;
        FREE_STRI2(stri, (memSizeType) lobLength);
        stri = NULL;
      } else {
        memcpy_to_strelem(stri->mem, (ustriType) stri->mem, (memSizeType) lobLength);
        stri->size = (memSizeType) lobLength;
      } /* if */
    } /* if */
    return stri;
  } /* getBlobAsStri */



static striType getClob (preparedStmtType preparedStmt,
    OCILobLocator *lobLocator, errInfoType *err_info)

  {
    oraub8 lobLength;
    oraub8 byte_amount;
    char *buffer;
    char *resized_buffer;
    memSizeType buffer_size;
    striType stri;

  /* getClob */
    logFunction(printf("getClob(" FMT_U_MEM ", " FMT_X_MEM ")\n",
                       (memSizeType) preparedStmt,
                       (memSizeType) lobLocator););
    if (OCILobGetLength2(preparedStmt->oci_service_context,
                         preparedStmt->oci_error,
                         lobLocator,
                         &lobLength) != OCI_SUCCESS) {
      setDbErrorMsg("getClob", "OCILobGetLength2",
                    preparedStmt->oci_error);
      logError(printf("getClob: OCILobGetLength2:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
      stri = NULL;
    } else if (unlikely(lobLength > MAX_MEMSIZETYPE / MAX_UTF8_EXPANSION_FACTOR)) {
      *err_info = MEMORY_ERROR;
      stri = NULL;
    } else {
      /* printf("connection charset_id: %u\n", preparedStmt->charSetId);
         printf("SQLCS_IMPLICIT: %u\n", SQLCS_IMPLICIT);
         printf("lobLength: " FMT_U64 "\n", (uint64Type) lobLength); */
      buffer_size = (memSizeType) lobLength * MAX_UTF8_EXPANSION_FACTOR;
      if (unlikely((buffer = (char *) malloc(buffer_size)) == NULL)) {
        *err_info = MEMORY_ERROR;
        stri = NULL;
      } else {
        byte_amount = buffer_size;
        if (OCILobRead2(preparedStmt->oci_service_context,
                        preparedStmt->oci_error,
                        lobLocator,
                        &byte_amount,
                        NULL, /* Make sure that byte_amount is used. */
                        1,  /* Offsets should be greater than or equal to one. */
                        buffer,
                        buffer_size,
                        OCI_ONE_PIECE,
                        NULL, NULL, preparedStmt->charSetId, SQLCS_IMPLICIT) != OCI_SUCCESS) {
          setDbErrorMsg("getClob", "OCILobRead2",
                        preparedStmt->oci_error);
          logError(printf("getClob: OCILobRead2:\n%s\n",
                          dbError.message););
          *err_info = DATABASE_ERROR;
          stri = NULL;
        } else if (byte_amount > buffer_size) {
          *err_info = MEMORY_ERROR;
          stri = NULL;
        } else {
          /* printf("buffer_size: " FMT_U_MEM "\n", buffer_size);
             printf("byte_amount: " FMT_U64 "\n", (uint64Type) byte_amount); */
          if (lobLength > 1048576) {
            /* Reduce memory pressure. */
            resized_buffer = (char *) realloc(buffer, (memSizeType) byte_amount);
            if (resized_buffer != NULL) {
              buffer = resized_buffer;
            } /* if */
          } /* if */
          stri = cstri8_buf_to_stri(buffer, (memSizeType) byte_amount, err_info);
          /* printf("stri->size: " FMT_U_MEM "\n", stri->size); */
        } /* if */
        free(buffer);
      } /* if */
    } /* if */
    return stri;
  } /* getClob */



static int setBigInt (void *const buffer, const const_bigIntType bigIntValue,
    errInfoType *err_info)

  {
    striType stri;
    int length = 0;

  /* setBigInt */
    logFunction(printf("setBigInt(*, %s, *)\n", bigHexCStri(bigIntValue)););
    stri = bigStrDecimal(bigIntValue);
    if (unlikely(stri == NULL)) {
      *err_info = MEMORY_ERROR;
    } else {
      length = sqltNumberFromDecimalInt((uint8Type *) buffer, stri, 0, err_info);
      FREE_STRI(stri);
    } /* if */
    return length;
  } /* setBigInt */



static int setBigRat (void *const buffer, const const_bigIntType numerator,
    const const_bigIntType denominator, errInfoType *err_info)

  {
    bigIntType number;
    bigIntType mantissaValue = NULL;
    striType stri;
    int length = 0;

  /* setBigRat */
    logFunction(printf("setBigRat(*, %s, ", bigHexCStri(numerator));
                printf("%s, *)\n", bigHexCStri(denominator)););
    if (bigEqSignedDigit(denominator, 0)) {
      if (bigCmpSignedDigit(numerator, 0) > 0) {
        /* printf("Positive infinity\n"); */
        SET_NUMBER_TO_POSITIVE_INFINITY(buffer, length);
      } else if (bigEqSignedDigit(numerator, 0)) {
        /* Not a Number (NaN), defined by this driver. */
        /* printf("NaN\n"); */
        SET_NUMBER_TO_NAN(buffer, length);
      } else {
        /* printf("Negative infinity\n"); */
        SET_NUMBER_TO_NEGATIVE_INFINITY(buffer, length);
      } /* if */
    } else {
      number = bigIPowSignedDigit(10, 128);
      if (number != NULL) {
        bigMultAssign(&number, numerator);
        mantissaValue = bigDiv(number, denominator);
        bigDestr(number);
      } /* if */
      if (mantissaValue != NULL) {
        stri = bigStrDecimal(mantissaValue);
        if (unlikely(stri == NULL)) {
          *err_info = MEMORY_ERROR;
        } else {
          length = sqltNumberFromDecimalInt((uint8Type *) buffer, stri, 128, err_info);
          FREE_STRI(stri);
        } /* if */
        bigDestr(mantissaValue);
      } /* if */
    } /* if */
    logFunction(printf("setBigRat --> ");
                showSqltNumber((memSizeType) length, (const uint8Type *) buffer);
                printf("\n"););
    return length;
  } /* setBigRat */



static boolType setBlob (preparedStmtType preparedStmt,
    OCILobLocator *lobLocator, const const_bstriType bstri,
    errInfoType *err_info)

  {
    oraub8 byte_amount;

  /* setBlob */
    logFunction(printf("setBlob(" FMT_U_MEM ", " FMT_X_MEM ", *, *)\n",
                       (memSizeType) preparedStmt,
                       (memSizeType) lobLocator););
    if (OCILobTrim2(preparedStmt->oci_service_context,
                    preparedStmt->oci_error,
                    lobLocator,
                    0) != OCI_SUCCESS) {
      setDbErrorMsg("setBlob", "OCILobTrim2",
                    preparedStmt->oci_error);
      logError(printf("setBlob: OCILobTrim2:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
    } /* if */
    byte_amount = bstri->size;
    if (OCILobWrite2(preparedStmt->oci_service_context,
                     preparedStmt->oci_error,
                     lobLocator,
                     &byte_amount,
                     NULL,
                     1,  /* Offsets should be greater than or equal to one. */
                     (dvoid *) bstri->mem,
                     (oraub8) bstri->size,
                     OCI_ONE_PIECE, NULL, NULL,
                     preparedStmt->charSetId, SQLCS_IMPLICIT) != OCI_SUCCESS) {
      setDbErrorMsg("setBlob", "OCILobWrite2",
                    preparedStmt->oci_error);
      logError(printf("setBlob: OCILobWrite2:\n%s\n",
                      dbError.message););
      *err_info = DATABASE_ERROR;
    } /* if */
    return *err_info == OKAY_NO_ERROR;
  } /* setBlob */



#ifndef SQLT_BFLOAT
static int setFloat (void *const buffer, const floatType floatValue,
    errInfoType *err_info)

  {
    striType stri;
    memSizeType decimalPointPos;
    memSizeType scale;
    memSizeType savedSize;
    int length = 0;

  /* setFloat */
    logFunction(printf("setFloat(" FMT_E ")\n", floatValue););
    if (os_isnan(floatValue)) {
      /* Not a Number (NaN), defined by this driver. */
      /* printf("NaN\n"); */
      SET_NUMBER_TO_NAN(buffer, length);
    } else if (os_isinf(floatValue)) {
      if (floatValue < 0.0) {
        /* printf("Negative infinity\n"); */
        SET_NUMBER_TO_NEGATIVE_INFINITY(buffer, length);
      } else {
        /* printf("Positive infinity\n"); */
        SET_NUMBER_TO_POSITIVE_INFINITY(buffer, length);
      } /* if */
    } else {
#if FLOATTYPE_SIZE == 32
      stri = doubleToStri(floatValue, FALSE);
#elif FLOATTYPE_SIZE == 64
      stri = doubleToStri(floatValue, TRUE);
#else
      stri = NULL;
#endif
      if (stri == NULL) {
        *err_info = MEMORY_ERROR;
      } else {
        /* printf("stri: ");
           prot_stri(stri);
           printf("\n"); */
        decimalPointPos = (memSizeType) strChPos(stri, '.');
        scale = stri->size - decimalPointPos;
        memmove(&stri->mem[decimalPointPos - 1],
                &stri->mem[decimalPointPos],
                scale * sizeof(strElemType));
        savedSize = stri->size;
        stri->size--;
        while (scale >= 1 && stri->mem[stri->size - 1] == '0') {
          scale--;
          stri->size--;
        } /* while */
        if (scale > INT_MAX) {
          /* It is not possible to cast scale to int. */
          *err_info = MEMORY_ERROR;
        } else {
          length = sqltNumberFromDecimalInt((uint8Type *) buffer, stri, (int) scale, err_info);
        } /* if */
        FREE_STRI2(stri, savedSize);
      } /* if */
    } /* if */
    logFunction(printf("setFloat --> ");
                showSqltNumber((memSizeType) length, (const uint8Type *) buffer);
                printf("\n"););
    return length;
  } /* setFloat */
#endif



static void sqlBindBigInt (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType value)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    int length;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBigInt */
    logFunction(printf("sqlBindBigInt(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, bigHexCStri(value)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBigInt: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      if (param->buffer_capacity < SIZEOF_SQLT_NUM) {
        free(param->buffer);
        if ((param->buffer = malloc(SIZEOF_SQLT_NUM)) == NULL) {
          param->buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          param->buffer_capacity = SIZEOF_SQLT_NUM;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        param->buffer_type = SQLT_NUM;
        param->buffer_length = SIZEOF_SQLT_NUM;
        length = setBigInt(param->buffer, value, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          if (unlikely(OCIBindByPos(preparedStmt->ppStmt,
                                    &param->bind_handle,
                                    preparedStmt->oci_error,
                                    (ub4) pos,
                                    param->buffer,
                                    length,
                                    SQLT_NUM, NULL, NULL, NULL, 0, NULL,
                                    OCI_DEFAULT) != OCI_SUCCESS)) {
            setDbErrorMsg("sqlBindBigInt", "OCIBindByPos",
                          preparedStmt->oci_error);
            logError(printf("sqlBindBigInt: OCIBindByPos:\n%s\n",
                            dbError.message););
            raise_error(DATABASE_ERROR);
          } else {
            preparedStmt->executeSuccessful = FALSE;
            preparedStmt->fetchOkay = FALSE;
            param->bound = TRUE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBigInt */



static void sqlBindBigRat (sqlStmtType sqlStatement, intType pos,
    const const_bigIntType numerator, const const_bigIntType denominator)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    int length;
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
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      if (param->buffer_capacity < SIZEOF_SQLT_NUM) {
        free(param->buffer);
        if ((param->buffer = malloc(SIZEOF_SQLT_NUM)) == NULL) {
          param->buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          param->buffer_capacity = SIZEOF_SQLT_NUM;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        param->buffer_type = SQLT_NUM;
        param->buffer_length = SIZEOF_SQLT_NUM;
        length = setBigRat(param->buffer, numerator, denominator, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          if (unlikely(OCIBindByPos(preparedStmt->ppStmt,
                                    &param->bind_handle,
                                    preparedStmt->oci_error,
                                    (ub4) pos,
                                    param->buffer,
                                    length,
                                    SQLT_NUM, NULL, NULL, NULL, 0, NULL,
                                    OCI_DEFAULT) != OCI_SUCCESS)) {
            setDbErrorMsg("sqlBindBigRat", "OCIBindByPos",
                          preparedStmt->oci_error);
            logError(printf("sqlBindBigRat: OCIBindByPos:\n%s\n",
                            dbError.message););
            raise_error(DATABASE_ERROR);
          } else {
            preparedStmt->executeSuccessful = FALSE;
            preparedStmt->fetchOkay = FALSE;
            param->bound = TRUE;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBigRat */



static void sqlBindBool (sqlStmtType sqlStatement, intType pos, boolType value)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBool */
    logFunction(printf("sqlBindBool(" FMT_U_MEM ", " FMT_D ", %s)\n",
                       (memSizeType) sqlStatement, pos, value ? "TRUE" : "FALSE"););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBool: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      if (param->buffer_capacity < 1) {
        if ((param->buffer = malloc(1)) == NULL) {
          param->buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          param->buffer_capacity = 1;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        param->buffer_type = SQLT_AFC;
        param->buffer_length = 1;
        ((char *) param->buffer)[0] = (char) ('0' + value);
        if (unlikely(OCIBindByPos(preparedStmt->ppStmt,
                                  &param->bind_handle,
                                  preparedStmt->oci_error,
                                  (ub4) pos,
                                  param->buffer,
                                  (sb4) param->buffer_length,
                                  SQLT_AFC, NULL, NULL, NULL, 0, NULL,
                                  OCI_DEFAULT) != OCI_SUCCESS)) {
          setDbErrorMsg("sqlBindBool", "OCIBindByPos",
                        preparedStmt->oci_error);
          logError(printf("sqlBindBool: OCIBindByPos:\n%s\n",
                          dbError.message););
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
          param->bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindBool */



static void sqlBindBStri (sqlStmtType sqlStatement, intType pos,
    const const_bstriType bstri)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindBStri */
    logFunction(printf("sqlBindBStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, bstriAsUnquotedCStri(bstri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindBStri: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      if (param->buffer_capacity < sizeof(void *)) {
        free(param->buffer);
        if ((param->buffer = malloc(sizeof(void *))) == NULL) {
          param->buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          param->buffer_capacity = sizeof(void *);
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        if (param->buffer_type != SQLT_BLOB && param->descriptor != NULL) {
          freeDescriptor(preparedStmt,
                         param->descriptor,
                         param->buffer_type);
          param->descriptor = NULL;
        } /* if */
        if (param->descriptor == NULL) {
          if (OCIDescriptorAlloc(preparedStmt->oci_environment,
                                 &param->descriptor,
                                 OCI_DTYPE_LOB,  /* Binary LOB or Character LOB */
                                 0, NULL) != OCI_SUCCESS) {
            logError(printf("sqlBindBStri: OCIDescriptorAlloc: Out of memory.\n"););
            err_info = MEMORY_ERROR;
          } /* if */
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        if (OCILobCreateTemporary(preparedStmt->oci_service_context,
                                  preparedStmt->oci_error,
                                  (OCILobLocator *) param->descriptor,
                                  OCI_DEFAULT,
                                  OCI_DEFAULT,
                                  OCI_TEMP_BLOB,
                                  FALSE,
                                  OCI_DURATION_SESSION) != OCI_SUCCESS) {
          setDbErrorMsg("sqlBindBStri", "OCILobCreateTemporary",
                        preparedStmt->oci_error);
          logError(printf("sqlBindBStri: OCILobCreateTemporary:\n%s\n",
                          dbError.message););
          err_info = DATABASE_ERROR;
        } else {
          *(void **) param->buffer = param->descriptor;
          param->buffer_type   = SQLT_BLOB;
          param->buffer_length = 0;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else if (bstri->size == 0) {
        param->indicator = -1;
        if (unlikely(OCIBindByPos(preparedStmt->ppStmt,
                                  &param->bind_handle,
                                  preparedStmt->oci_error,
                                  (ub4) pos,
                                  param->buffer,
                                  (sb4) param->buffer_length,
                                  SQLT_BLOB,
                                  &param->indicator,
                                  NULL, NULL, 0, NULL, OCI_DEFAULT) != OCI_SUCCESS))  {
          setDbErrorMsg("sqlBindBStri", "OCIBindByPos",
                        preparedStmt->oci_error);
          logError(printf("sqlBindBStri: OCIBindByPos:\n%s\n",
                          dbError.message););
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
          param->bound = TRUE;
        } /* if */
      } else if (unlikely(!setBlob(preparedStmt,
                                   (OCILobLocator *) param->descriptor,
                                   bstri, &err_info))) {
        raise_error(err_info);
      } else if (unlikely(OCIBindByPos(preparedStmt->ppStmt,
                                       &param->bind_handle,
                                       preparedStmt->oci_error,
                                       (ub4) pos,
                                       param->buffer,
                                       (sb4) param->buffer_length,
                                       SQLT_BLOB,
                                       NULL, NULL, NULL, 0, NULL,
                                       OCI_DEFAULT) != OCI_SUCCESS))  {
        setDbErrorMsg("sqlBindBStri", "OCIBindByPos",
                      preparedStmt->oci_error);
        logError(printf("sqlBindBStri: OCIBindByPos:\n%s\n",
                        dbError.message););
        raise_error(DATABASE_ERROR);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindBStri */



static void sqlBindDuration (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    uint16Type buffer_type;
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
      raise_error(RANGE_ERROR);
    } else if (unlikely(year < INT32TYPE_MIN || year > INT32TYPE_MAX ||
                        month < -12 || month > 12 ||
                        day < -31 || day > 31 ||
                        hour <= -24 || hour >= 24 ||
                        minute <= -60 || minute >= 60 ||
                        second <= -60 || second >= 60 ||
                        micro_second <= -1000000 || micro_second >= 1000000)) {
      logError(printf("sqlBindDuration: Duration not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else if (unlikely(!((year >= 0 && month >= 0 && day >= 0 && hour >= 0 &&
                           minute >= 0 && second >= 0 && micro_second >= 0) ||
                          (year <= 0 && month <= 0 && day <= 0 && hour <= 0 &&
                           minute <= 0 && second <= 0 && micro_second <= 0)))) {
      logError(printf("sqlBindDuration: Duration neither clearly positive nor negative.\n"););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      if (param->buffer_capacity < sizeof(void *)) {
        free(param->buffer);
        if ((param->buffer = malloc(sizeof(void *))) == NULL) {
          param->buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          param->buffer_capacity = sizeof(void *);
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        if (day == 0 && hour == 0 && minute == 0 && second == 0 && micro_second == 0) {
          buffer_type = SQLT_INTERVAL_YM;
        } else if (year == 0 && month == 0) {
          buffer_type = SQLT_INTERVAL_DS;
        } else {
          logError(printf("sqlBindDuration: Neither SQLT_INTERVAL_YM nor SQLT_INTERVAL_DS.\n"););
          err_info = RANGE_ERROR;
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        if (param->buffer_type != buffer_type && param->descriptor != NULL) {
          freeDescriptor(preparedStmt,
                         param->descriptor,
                         param->buffer_type);
          param->descriptor = NULL;
        } /* if */
        if (param->descriptor == NULL) {
          if (buffer_type == SQLT_INTERVAL_YM) {
            if (OCIDescriptorAlloc(preparedStmt->oci_environment,
                                   &param->descriptor,
                                   OCI_DTYPE_INTERVAL_YM,  /* INTERVAL YEAR TO MONTH descriptor */
                                   0, NULL) != OCI_SUCCESS) {
              logError(printf("sqlBindDuration: OCIDescriptorAlloc: Out of memory.\n"););
              err_info = MEMORY_ERROR;
            } /* if */
          } else if (buffer_type == SQLT_INTERVAL_DS) {
            if (OCIDescriptorAlloc(preparedStmt->oci_environment,
                                   &param->descriptor,
                                   OCI_DTYPE_INTERVAL_DS,  /* INTERVAL DAY TO SECOND descriptor */
                                   0, NULL) != OCI_SUCCESS) {
              logError(printf("sqlBindDuration: OCIDescriptorAlloc: Out of memory.\n"););
              err_info = MEMORY_ERROR;
            } /* if */
          } /* if */
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        param->buffer_type = buffer_type;
        param->buffer_length = 0;
        *(void **) param->buffer = param->descriptor;
        if (buffer_type == SQLT_INTERVAL_YM) {
          if (OCIIntervalSetYearMonth(preparedStmt->oci_environment,
                                      preparedStmt->oci_error,
                                      (sb4) year,
                                      (sb4) month,
                                      (OCIInterval *) param->descriptor) != OCI_SUCCESS) {
            logError(printf("sqlBindDuration: OCIIntervalSetYearMonth:\n");
                     printError(preparedStmt->oci_error););
            err_info = RANGE_ERROR;
          } /* if */
        } else if (buffer_type == SQLT_INTERVAL_DS) {
          if (OCIIntervalSetDaySecond(preparedStmt->oci_environment,
                                      preparedStmt->oci_error,
                                      (sb4) day,
                                      (sb4) hour,
                                      (sb4) minute,
                                      (sb4) second,
                                      (sb4) micro_second,
                                      (OCIInterval *) param->descriptor) != OCI_SUCCESS) {
            logError(printf("sqlBindDuration: OCIIntervalSetDaySecond:\n");
                     printError(preparedStmt->oci_error););
            err_info = RANGE_ERROR;
          } /* if */
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else if (unlikely(OCIBindByPos(preparedStmt->ppStmt,
                                       &param->bind_handle,
                                       preparedStmt->oci_error,
                                       (ub4) pos,
                                       param->buffer,
                                       (sb4) param->buffer_length,
                                       buffer_type,
                                       NULL, NULL, NULL, 0, NULL,
                                       OCI_DEFAULT) != OCI_SUCCESS))  {
        setDbErrorMsg("sqlBindDuration", "OCIBindByPos",
                      preparedStmt->oci_error);
        logError(printf("sqlBindDuration: OCIBindByPos:\n%s\n",
                        dbError.message););
        raise_error(DATABASE_ERROR);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindDuration */



static void sqlBindFloat (sqlStmtType sqlStatement, intType pos, floatType value)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
#ifdef SQLT_BFLOAT
#if FLOATTYPE_SIZE == 32
    const uint16Type buffer_type = SQLT_BFLOAT;
#elif FLOATTYPE_SIZE == 64
    const uint16Type buffer_type = SQLT_BDOUBLE;
#endif
    const memSizeType buffer_length = sizeof(floatType);
#else
    const uint16Type buffer_type = SQLT_NUM;
    const memSizeType buffer_length = SIZEOF_SQLT_NUM;
    int length;
#endif
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindFloat */
    logFunction(printf("sqlBindFloat(" FMT_U_MEM ", " FMT_D ", " FMT_E ")\n",
                       (memSizeType) sqlStatement, pos, value););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindFloat: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      if (param->buffer_capacity < buffer_length) {
        free(param->buffer);
        if ((param->buffer = malloc(buffer_length)) == NULL) {
          param->buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          param->buffer_capacity = buffer_length;
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        param->buffer_type = buffer_type;
        param->buffer_length = buffer_length;
#ifdef SQLT_BFLOAT
        *(floatType *) param->buffer = value;
        if (unlikely(OCIBindByPos(preparedStmt->ppStmt,
                                  &param->bind_handle,
                                  preparedStmt->oci_error,
                                  (ub4) pos,
                                  param->buffer,
                                  sizeof(floatType),
                                  buffer_type, NULL, NULL, NULL, 0, NULL,
                                  OCI_DEFAULT) != OCI_SUCCESS)) {
          setDbErrorMsg("sqlBindFloat", "OCIBindByPos",
                        preparedStmt->oci_error);
          logError(printf("sqlBindFloat: OCIBindByPos:\n%s\n",
                          dbError.message););
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
          param->bound = TRUE;
        } /* if */
      } /* if */
#else
        length = setFloat(param->buffer, value, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } else {
          if (unlikely(OCIBindByPos(preparedStmt->ppStmt,
                                    &param->bind_handle,
                                    preparedStmt->oci_error,
                                    (ub4) pos,
                                    param->buffer,
                                    length,
                                    SQLT_NUM, NULL, NULL, NULL, 0, NULL,
                                    OCI_DEFAULT) != OCI_SUCCESS)) {
            setDbErrorMsg("sqlBindFloat", "OCIBindByPos",
                          preparedStmt->oci_error);
            logError(printf("sqlBindFloat: OCIBindByPos:\n%s\n",
                            dbError.message););
            raise_error(DATABASE_ERROR);
          } else {
            preparedStmt->executeSuccessful = FALSE;
            preparedStmt->fetchOkay = FALSE;
            param->bound = TRUE;
          } /* if */
        } /* if */
      } /* if */
#endif
    } /* if */
  } /* sqlBindFloat */



static void sqlBindInt (sqlStmtType sqlStatement, intType pos, intType value)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    errInfoType err_info = OKAY_NO_ERROR;

  /* sqlBindInt */
    logFunction(printf("sqlBindInt(" FMT_U_MEM ", " FMT_D ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos, value););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindInt: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      if (param->buffer_capacity < sizeof(int64Type)) {
        free(param->buffer);
        if ((param->buffer = malloc(sizeof(int64Type))) == NULL) {
          param->buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          param->buffer_capacity = sizeof(int64Type);
        } /* if */
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else {
        param->buffer_type = SQLT_INT;
        param->buffer_length = sizeof(int64Type);
        *(int64Type *) param->buffer = (int64Type) value;
        if (unlikely(OCIBindByPos(preparedStmt->ppStmt,
                                  &param->bind_handle,
                                  preparedStmt->oci_error,
                                  (ub4) pos,
                                  param->buffer,
                                  sizeof(int64Type),
                                  SQLT_INT, NULL, NULL, NULL, 0, NULL,
                                  OCI_DEFAULT) != OCI_SUCCESS)) {
          setDbErrorMsg("sqlBindInt", "OCIBindByPos",
                        preparedStmt->oci_error);
          logError(printf("sqlBindInt: OCIBindByPos:\n%s\n",
                          dbError.message););
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
          param->bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindInt */



static void sqlBindNull (sqlStmtType sqlStatement, intType pos)

  {
    preparedStmtType preparedStmt;
    bindDataType param;

  /* sqlBindNull */
    logFunction(printf("sqlBindNull(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, pos););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindNull: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      param->indicator = -1;
      if (unlikely(OCIBindByPos(preparedStmt->ppStmt,
                                &param->bind_handle,
                                preparedStmt->oci_error,
                                (ub4) pos,
                                NULL,
                                0,
                                SQLT_CHR,
                                &param->indicator,
                                NULL, NULL, 0, NULL,
                                OCI_DEFAULT) != OCI_SUCCESS)) {
        setDbErrorMsg("sqlBindNull", "OCIBindByPos",
                      preparedStmt->oci_error);
        logError(printf("sqlBindNull: OCIBindByPos:\n%s\n",
                        dbError.message););
        raise_error(DATABASE_ERROR);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindNull */



static void sqlBindStri (sqlStmtType sqlStatement, intType pos,
    const const_striType stri)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    cstriType stri8;
    cstriType resized_stri8;
    memSizeType length;

  /* sqlBindStri */
    logFunction(printf("sqlBindStri(" FMT_U_MEM ", " FMT_D ", \"%s\")\n",
                       (memSizeType) sqlStatement, pos, striAsUnquotedCStri(stri)););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(pos < 1 || (uintType) pos > preparedStmt->param_array_size)) {
      logError(printf("sqlBindStri: pos: " FMT_D ", max pos: " FMT_U_MEM ".\n",
                      pos, preparedStmt->param_array_size););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      stri8 = stri_to_cstri8_buf(stri, &length);
      if (unlikely(stri8 == NULL)) {
        raise_error(MEMORY_ERROR);
      } else if (unlikely(length > INT32TYPE_MAX)) {
        /* It is not possible to cast length to sb4. */
        free(stri8);
        raise_error(MEMORY_ERROR);
      } else {
        resized_stri8 = REALLOC_CSTRI(stri8, length);
        if (likely(resized_stri8 != NULL)) {
          stri8 = resized_stri8;
        } /* if */
        free(param->buffer);
        /* For the buffer_type SQLT_CHR trailing blanks are stripped */
        /* and the resulting value is passed to the database.        */
        /* For the buffer_type SQLT_AFC the entire contents of the   */
        /* buffer (param->buffer_length) is passed to the database,  */
        /* including any trailing blanks or NULLs.                   */
        /* SQLT_AFC is used to keep trailing blanks and NULLs.       */
        param->buffer_type = SQLT_AFC;
        param->buffer_length = length;
        param->buffer_capacity = length;
        param->buffer = stri8;
        if (unlikely(OCIBindByPos(preparedStmt->ppStmt,
                                  &param->bind_handle,
                                  preparedStmt->oci_error,
                                  (ub4) pos,
                                  param->buffer,
                                  (sb4) param->buffer_length,
                                  SQLT_AFC,
                                  NULL, NULL, NULL, 0, NULL,
                                  OCI_DEFAULT) != OCI_SUCCESS))  {
          setDbErrorMsg("sqlBindStri", "OCIBindByPos",
                        preparedStmt->oci_error);
          logError(printf("sqlBindStri: OCIBindByPos:\n%s\n",
                          dbError.message););
          raise_error(DATABASE_ERROR);
        } else {
          preparedStmt->executeSuccessful = FALSE;
          preparedStmt->fetchOkay = FALSE;
          param->bound = TRUE;
        } /* if */
      } /* if */
    } /* if */
  } /* sqlBindStri */



static void sqlBindTime (sqlStmtType sqlStatement, intType pos,
    intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType micro_second,
    intType time_zone)

  {
    preparedStmtType preparedStmt;
    bindDataType param;
    char timezoneStri[3 + 2 * INTTYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    memSizeType timezoneStriLength;
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
      raise_error(RANGE_ERROR);
    } else if (unlikely(year <= INT16TYPE_MIN || year > INT16TYPE_MAX ||
                        month < 1 || month > 12 ||
                        day < 1 || day > 31 ||
                        hour < 0 || hour >= 24 ||
                        minute < 0 || minute >= 60 ||
                        second < 0 || second >= 60 ||
                        micro_second < 0 || micro_second >= 1000000)) {
      logError(printf("sqlBindTime: Time not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      param = &preparedStmt->param_array[pos - 1];
      if (param->buffer_capacity < sizeof(void *)) {
        free(param->buffer);
        if ((param->buffer = malloc(sizeof(void *))) == NULL) {
          param->buffer_capacity = 0;
          err_info = MEMORY_ERROR;
        } else {
          param->buffer_capacity = sizeof(void *);
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        if (param->buffer_type != SQLT_TIMESTAMP_TZ && param->descriptor != NULL) {
          freeDescriptor(preparedStmt,
                         param->descriptor,
                         param->buffer_type);
          param->descriptor = NULL;
        } /* if */
        if (param->descriptor == NULL) {
          if (OCIDescriptorAlloc(preparedStmt->oci_environment,
                                 &param->descriptor,
                                 OCI_DTYPE_TIMESTAMP_TZ,  /* TIMESTAMP WITH TIME ZONE */
                                 0, NULL) != OCI_SUCCESS) {
            logError(printf("sqlBindTime: OCIDescriptorAlloc: Out of memory.\n"););
            err_info = MEMORY_ERROR;
          } /* if */
        } /* if */
      } /* if */
      if (likely(err_info == OKAY_NO_ERROR)) {
        *(void **) param->buffer = param->descriptor;
        param->buffer_type   = SQLT_TIMESTAMP_TZ;
        param->buffer_length = 0;
      } /* if */
      if (year <= 0) {
        /* In the database BC dates are represented with negative numbers */
        /* and the year 0 does not exist. The year 0 of the proleptic     */
        /* Gregorian calendar corresponds to 1 BC (= -1) in the database. */
        year--;
      } /* if */
      if (time_zone < 0) {
        if (unlikely(year == -1 && month == 12 && day == 31 &&
                     (hour + (-time_zone) / 60 > 23 ||
                     (hour + (-time_zone) / 60 == 23 &&
                     minute + (-time_zone) % 60 > 59)))) {
          /* When the time_zone would move the year from 1 BC to 1 AD the */
          /* function OCIDateTimeConstruct() fails. This is avoided by    */
          /* setting time_zone to zero. Luckily all time values at the    */
          /* default date 0-1-1 are not affected by this special case.    */
          time_zone = 0;
          /* printf(F_D(04) "-" F_D(02) "-" F_D(02) " "
                 F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) ", " FMT_D "\n",
                 year, month, day, hour, minute, second, micro_second, time_zone); */
        } /* if */
        timezoneStriLength =
            (memSizeType) sprintf(timezoneStri, "-" F_D(02) ":" F_D(02),
                                  (-time_zone) / 60, (-time_zone) % 60);
      } else {
        if (unlikely((year == 1 || year == -4712) && month == 1 && day == 1 &&
                     (time_zone / 60 > hour ||
                     (time_zone / 60 == hour && time_zone % 60 > minute)))) {
          /* When the time_zone would move the year from 1 AD to 1 BC the */
          /* function OCIDateTimeConstruct() fails. This is avoided by    */
          /* setting time_zone to zero. For the minimum allowed time      */
          /* (January 1, 4712 BC) OCIDateTimeConstruct() does also not    */
          /* work correctly, if the time_zone is greater than zero.       */
          time_zone = 0;
          /* printf(F_D(04) "-" F_D(02) "-" F_D(02) " "
                 F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) ", " FMT_D "\n",
                 year, month, day, hour, minute, second, micro_second, time_zone); */
        } /* if */
        timezoneStriLength =
            (memSizeType) sprintf(timezoneStri, F_D(02) ":" F_D(02),
                                  time_zone / 60, time_zone % 60);
      } /* if */
      /* printf("timezone: %s\n", timezoneStri); */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } else if (unlikely(OCIDateTimeConstruct(preparedStmt->oci_environment,
                                               preparedStmt->oci_error,
                                               (OCIDateTime *) param->descriptor,
                                               (sb2) year, (ub1) month, (ub1) day,
                                               (ub1) hour, (ub1) minute, (ub1) second,
                                               (ub4) (micro_second * 1000),
                                               (OraText *) timezoneStri,
                                               timezoneStriLength) != OCI_SUCCESS)) {
        logError(printf("sqlBindTime: OCIDateTimeConstruct:\n");
                 printError(preparedStmt->oci_error););
        raise_error(RANGE_ERROR);
      } else if (unlikely(OCIBindByPos(preparedStmt->ppStmt,
                                       &param->bind_handle,
                                       preparedStmt->oci_error,
                                       (ub4) pos,
                                       param->buffer,
                                       (sb4) param->buffer_length,
                                       SQLT_TIMESTAMP_TZ,
                                       NULL, NULL, NULL, 0, NULL,
                                       OCI_DEFAULT) != OCI_SUCCESS))  {
        setDbErrorMsg("sqlBindTime", "OCIBindByPos",
                      preparedStmt->oci_error);
        logError(printf("sqlBindTime: OCIBindByPos:\n%s\n",
                        dbError.message););
        raise_error(DATABASE_ERROR);
      } else {
        preparedStmt->executeSuccessful = FALSE;
        preparedStmt->fetchOkay = FALSE;
        param->bound = TRUE;
      } /* if */
    } /* if */
  } /* sqlBindTime */



static void sqlClose (databaseType database)

  {
    dbType db;

  /* sqlClose */
    logFunction(printf("sqlClose(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (db->oci_service_context != NULL &&
        db->oci_error != NULL &&
        db->oci_session != NULL) {
      OCISessionEnd(db->oci_service_context, db->oci_error,
                    db->oci_session, OCI_DEFAULT);
    } /* if */
    if (db->oci_server != NULL &&
        db->oci_error != NULL) {
      OCIServerDetach(db->oci_server, db->oci_error,
                      OCI_DEFAULT);
    } /* if */
    if (db->oci_session != NULL) {
      OCIHandleFree(db->oci_session, OCI_HTYPE_SESSION);
      db->oci_session = NULL;
    } /* if */
    if (db->oci_service_context != NULL) {
      OCIHandleFree(db->oci_service_context, OCI_HTYPE_SVCCTX);
      db->oci_service_context = NULL;
    } /* if */
    if (db->oci_server != NULL) {
      OCIHandleFree(db->oci_server, OCI_HTYPE_SERVER);
      db->oci_server = NULL;
    } /* if */
    if (db->oci_error != NULL) {
      OCIHandleFree(db->oci_error, OCI_HTYPE_ERROR);
      db->oci_error = NULL;
    } /* if */
    if (db->oci_environment != NULL) {
      OCIHandleFree(db->oci_environment, OCI_HTYPE_ENV);
      db->oci_environment = NULL;
    } /* if */
    db->isOpen = FALSE;
    logFunction(printf("sqlClose -->\n"););
  } /* sqlClose */



static bigIntType sqlColumnBigInt (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
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
      columnData = &preparedStmt->result_array[column - 1];
      if (columnData->indicator == -1) {
        logMessage(printf("Column is NULL -> Use default value: 0\n"););
        columnValue = bigZero();
      } else if (unlikely(columnData->indicator != 0)) {
        dbInconsistent("sqlColumnBigInt", "OCIDefineByPos");
        logError(printf("sqlColumnBigInt: Column " FMT_D " has indicator: %d\n",
                        column, columnData->indicator););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else {
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SQLT_NUM:
            columnValue = getBigInt(columnData->buffer, columnData->length);
            break;
          default:
            logError(printf("sqlColumnBigInt: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
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
    resultDataType columnData;

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
      columnData = &preparedStmt->result_array[column - 1];
      if (columnData->indicator == -1) {
        logMessage(printf("Column is NULL -> Use default value: 0\n"););
        *numerator = bigZero();
        *denominator = bigFromInt32(1);
      } else if (unlikely(columnData->indicator != 0)) {
        dbInconsistent("sqlColumnBigRat", "OCIDefineByPos");
        logError(printf("sqlColumnBigRat: Column " FMT_D " has indicator: %d\n",
                        column, columnData->indicator););
        raise_error(DATABASE_ERROR);
      } else {
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SQLT_NUM:
            *numerator = getBigRational(columnData->buffer,
                                        columnData->length,
                                        denominator);
            break;
          default:
            logError(printf("sqlColumnBigRat: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
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
      columnData = &preparedStmt->result_array[column - 1];
      if (columnData->indicator == -1) {
        logMessage(printf("Column is NULL -> Use default value: FALSE\n"););
        columnValue = 0;
      } else if (unlikely(columnData->indicator != 0)) {
        dbInconsistent("sqlColumnBool", "OCIDefineByPos");
        logError(printf("sqlColumnBool: Column " FMT_D " has indicator: %d\n",
                        column, columnData->indicator););
        raise_error(DATABASE_ERROR);
        columnValue = 0;
      } else {
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SQLT_CHR: /* VARCHAR2 */
          case SQLT_VCS: /* VARCHAR */
          case SQLT_AFC: /* CHAR */
            if (unlikely(columnData->length != 1)) {
              logError(printf("sqlColumnBool: Column " FMT_D ": "
                              "The size of a boolean field must be 1.\n", column););
              raise_error(RANGE_ERROR);
              columnValue = 0;
            } else {
              columnValue = *(const_cstriType) columnData->buffer - '0';
            } /* if */
            break;
          case SQLT_NUM:
            columnValue = getInt(columnData->buffer, columnData->length);
            break;
          default:
            logError(printf("sqlColumnBool: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
            raise_error(RANGE_ERROR);
            columnValue = 0;
            break;
        } /* switch */
        if (unlikely((uintType) columnValue >= 2)) {
          logError(printf("sqlColumnBool: Boolean value " FMT_D " not in allowed range.\n",
                          columnValue););
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
    resultDataType columnData;
    memSizeType length;
    errInfoType err_info = OKAY_NO_ERROR;
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
      columnData = &preparedStmt->result_array[column - 1];
      if (columnData->indicator == -1) {
        logMessage(printf("Column is NULL -> Use default value: \"\"\n"););
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(columnValue, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          columnValue->size = 0;
        } /* if */
      } else if (unlikely(columnData->indicator != 0)) {
        dbInconsistent("sqlColumnBStri", "OCIDefineByPos");
        logError(printf("sqlColumnBStri: Column " FMT_D " has indicator: %d\n",
                        column, columnData->indicator););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else {
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SQLT_BIN: /* RAW */
          case SQLT_VBI: /* VARRAW */
          case SQLT_LBI: /* LONG RAW */
          case SQLT_LVB: /* LONG VARRAW */
            length = columnData->length;
            if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(columnValue, length))) {
              raise_error(MEMORY_ERROR);
            } else {
              columnValue->size = length;
              memcpy(columnValue->mem, (ustriType) columnData->buffer, length);
            } /* if */
            break;
          case SQLT_BLOB: /* Binary LOB */
          case SQLT_FILE: /* Binary FILE */
            columnValue = getBlob(preparedStmt,
                                  (OCILobLocator *) columnData->descriptor,
                                  &err_info);
            if (unlikely(columnValue == NULL)) {
              raise_error(err_info);
            } /* if */
            break;
          default:
            logError(printf("sqlColumnBStri: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
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
    resultDataType columnData;
    sb4 ociYear;
    sb4 ociMonth;
    sb4 ociDay;
    sb4 ociHour;
    sb4 ociMinute;
    sb4 ociSecond;
    sb4 ociFsec;
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
      columnData = &preparedStmt->result_array[column - 1];
      if (columnData->indicator == -1) {
        logMessage(printf("Column is NULL -> Use default value: P0D\n"););
        *year         = 0;
        *month        = 0;
        *day          = 0;
        *hour         = 0;
        *minute       = 0;
        *second       = 0;
        *micro_second = 0;
      } else if (columnData->indicator != 0) {
        dbInconsistent("sqlColumnDuration", "OCIDefineByPos");
        logError(printf("sqlColumnDuration: Column " FMT_D " has indicator: %d\n",
                        column, columnData->indicator););
        err_info = DATABASE_ERROR;
      } else {
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SQLT_INTERVAL_YM:
            /* printf("SQLT_INTERVAL_YM\n"); */
            if (unlikely(OCIIntervalGetYearMonth(preparedStmt->oci_environment,
                                                 preparedStmt->oci_error,
                                                 &ociYear,
                                                 &ociMonth,
                                                 (OCIInterval *) columnData->descriptor) != OCI_SUCCESS)) {
              logError(printf("sqlColumnDuration: OCIIntervalGetYearMonth:\n");
                       printError(preparedStmt->oci_error););
              raise_error(RANGE_ERROR);
            } else {
              *year   = ociYear;
              *month  = ociMonth;
              *day    = 0;
              *hour   = 0;
              *minute = 0;
              *second = 0;
              *micro_second = 0;
            } /* if */
            break;
          case SQLT_INTERVAL_DS:
            /* printf("SQLT_INTERVAL_DS\n"); */
            if (unlikely(OCIIntervalGetDaySecond(preparedStmt->oci_environment,
                                                 preparedStmt->oci_error,
                                                 &ociDay,
                                                 &ociHour,
                                                 &ociMinute,
                                                 &ociSecond,
                                                 &ociFsec,
                                                 (OCIInterval *) columnData->descriptor) != OCI_SUCCESS)) {
              logError(printf("sqlColumnDuration: OCIIntervalGetDaySecond:\n");
                       printError(preparedStmt->oci_error););
              raise_error(RANGE_ERROR);
            } else {
              *year   = 0;
              *month  = 0;
              *day    = ociDay;
              *hour   = ociHour;
              *minute = ociMinute;
              *second = ociSecond;
              *micro_second = ociFsec;
            } /* if */
            break;
          default:
            logError(printf("sqlColumnDuration: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
            err_info = RANGE_ERROR;
            break;
        } /* switch */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("sqlColumnDuration(" FMT_U_MEM ", " FMT_D ") -> P"
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
      columnData = &preparedStmt->result_array[column - 1];
      if (columnData->indicator == -1) {
        logMessage(printf("Column is NULL -> Use default value: 0.0\n"););
        columnValue = 0.0;
      } else if (unlikely(columnData->indicator != 0)) {
        dbInconsistent("sqlColumnFloat", "OCIDefineByPos");
        logError(printf("sqlColumnFloat: Column " FMT_D " has indicator: %d\n",
                        column, columnData->indicator););
        raise_error(DATABASE_ERROR);
        columnValue = 0.0;
      } else {
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SQLT_BFLOAT:
            columnValue = *(float *) columnData->buffer;
            break;
          case SQLT_BDOUBLE:
            columnValue = *(double *) columnData->buffer;
            break;
          case SQLT_NUM:
            columnValue = getFloat(columnData->buffer, columnData->length);
            break;
          default:
            logError(printf("sqlColumnFloat: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
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
      columnData = &preparedStmt->result_array[column - 1];
      if (columnData->indicator == -1) {
        logMessage(printf("Column is NULL -> Use default value: 0\n"););
        columnValue = 0;
      } else if (unlikely(columnData->indicator != 0)) {
        dbInconsistent("sqlColumnInt", "OCIDefineByPos");
        logError(printf("sqlColumnInt: Column " FMT_D " has indicator: %d\n",
                        column, columnData->indicator););
        raise_error(DATABASE_ERROR);
        columnValue = 0;
      } else {
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SQLT_NUM:
            columnValue = getInt(columnData->buffer, columnData->length);
            break;
          default:
            logError(printf("sqlColumnInt: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
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
    resultDataType columnData;
    const_cstriType stri8;
    memSizeType length;
    const_cstriType zeroPos;
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
      columnData = &preparedStmt->result_array[column - 1];
      if (columnData->indicator == -1) {
        logMessage(printf("Column is NULL -> Use default value: \"\"\n"););
        columnValue = strEmpty();
      } else if (unlikely(columnData->indicator != 0)) {
        dbInconsistent("sqlColumnStri", "OCIDefineByPos");
        logError(printf("sqlColumnStri: Column " FMT_D " has indicator: %d\n",
                        column, columnData->indicator););
        raise_error(DATABASE_ERROR);
        columnValue = NULL;
      } else {
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SQLT_CHR: /* VARCHAR2 */
          case SQLT_VCS: /* VARCHAR */
          case SQLT_LNG: /* LONG */
          case SQLT_LVC: /* LONG VARCHAR */
            stri8 = (cstriType) columnData->buffer;
            length = columnData->length;
            /* printf("length: " FMT_U_MEM "\n", length); */
            /* printf("buffer: \"");
               fwrite(stri8, 1, length, stdout);
               printf("\"\n"); */
            columnValue = cstri8_buf_to_stri(stri8, (memSizeType) length, &err_info);
            if (unlikely(columnValue == NULL)) {
              raise_error(err_info);
            } /* if */
            break;
          case SQLT_AFC: /* CHAR */
            stri8 = (cstriType) columnData->buffer;
            length = columnData->length;
            /* printf("buffer: \"");
               fwrite(stri8, 1, length, stdout);
               printf("\"\n"); */
            while (length > 0 && stri8[length - 1] == ' ') {
              length--;
            } /* if */
            columnValue = cstri8_buf_to_stri(stri8, (memSizeType) length, &err_info);
            if (unlikely(columnValue == NULL)) {
              raise_error(err_info);
            } /* if */
            break;
          case SQLT_STR: /* NULL-terminated STRING */
          case SQLT_AVC: /* CHARZ */
            stri8 = (cstriType) columnData->buffer;
            length = columnData->length;
            zeroPos = (const_cstriType) memchr(stri8, '\0', length);
            if (zeroPos != NULL) {
              length = (memSizeType) (zeroPos - stri8);
            } /* if */
            columnValue = cstri8_buf_to_stri(stri8, (memSizeType) length, &err_info);
            if (unlikely(columnValue == NULL)) {
              raise_error(err_info);
            } /* if */
            break;
          case SQLT_VST: /* OCI STRING type */
            stri8 = (cstriType) OCIStringPtr(preparedStmt->oci_environment,
                                             (OCIString *) columnData->buffer);
            if (unlikely(stri8 == NULL)) {
              dbInconsistent("sqlColumnStri", "OCIStringPtr");
              logError(printf("sqlColumnStri: Column " FMT_D ": "
                              "OCIStringPtr returned NULL.\n", column););
              raise_error(DATABASE_ERROR);
              columnValue = NULL;
            } else {
              length = OCIStringSize(preparedStmt->oci_environment,
                                     (OCIString *) columnData->buffer);
              columnValue = cstri8_buf_to_stri(stri8, (memSizeType) length, &err_info);
              if (unlikely(columnValue == NULL)) {
                raise_error(err_info);
              } /* if */
            } /* if */
            break;
          case SQLT_CLOB: /* Character LOB  */
            columnValue = getClob(preparedStmt,
                                  (OCILobLocator *) columnData->descriptor,
                                  &err_info);
            if (unlikely(columnValue == NULL)) {
              raise_error(err_info);
            } /* if */
            break;
          case SQLT_BIN: /* RAW */
          case SQLT_VBI: /* VARRAW */
          case SQLT_LBI: /* LONG RAW */
          case SQLT_LVB: /* LONG VARRAW */
            columnValue = cstri_buf_to_stri((const_cstriType) columnData->buffer,
                                            columnData->length);
            if (unlikely(columnValue == NULL)) {
              raise_error(MEMORY_ERROR);
            } /* if */
            break;
          case SQLT_BLOB: /* Binary LOB */
          case SQLT_FILE: /* Binary FILE */
            columnValue = getBlobAsStri(preparedStmt,
                                        (OCILobLocator *) columnData->descriptor,
                                        &err_info);
            if (unlikely(columnValue == NULL)) {
              raise_error(err_info);
            } /* if */
            break;
          case SQLT_RDD: /* ROWID */
            columnValue = getRowid(preparedStmt,
                                   (OCIRowid *) columnData->descriptor,
                                   &err_info);
            if (unlikely(columnValue == NULL)) {
              raise_error(err_info);
            } /* if */
            break;
          case SQLT_REF:
            columnValue = getRef(preparedStmt,
                                 columnData->ref,
                                 &err_info);
            if (unlikely(columnValue == NULL)) {
              raise_error(err_info);
            } /* if */
            break;
          default:
            logError(printf("sqlColumnStri: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
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
    resultDataType columnData;
    unsigned char *sqltDat;
    sb2 ociYear;
    ub1 ociMonth;
    ub1 ociDay;
    ub1 ociHour;
    ub1 ociMinute;
    ub1 ociSecond;
    ub4 ociFsec;
    sb1 tzHour;
    sb1 tzMinute;
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
      columnData = &preparedStmt->result_array[column - 1];
      if (columnData->indicator == -1) {
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
      } else if (columnData->indicator != 0) {
        dbInconsistent("sqlColumnTime", "OCIDefineByPos");
        logError(printf("sqlColumnTime: Column " FMT_D " has indicator: %d\n",
                        column, columnData->indicator););
        err_info = DATABASE_ERROR;
      } else {
        logMessage(printf("buffer_type: %s\n",
                          nameOfBufferType(columnData->buffer_type)););
        switch (columnData->buffer_type) {
          case SQLT_DAT: /* DATE */
            sqltDat = (unsigned char *) columnData->buffer;
            ociYear = (sb2) ((sqltDat[0] - 101) * 100 + sqltDat[1]);
            if (ociYear <= -1) {
              /* In the proleptic Gregorian calendar the year preceding 1 is 0. */
              ociYear++;
            } /* if */
            *year   = ociYear;
            *month  = sqltDat[2];
            *day    = sqltDat[3];
            *hour   = sqltDat[4] - 1;
            *minute = sqltDat[5] - 1;
            *second = sqltDat[6] - 1;
            *micro_second = 0;
            timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                          time_zone, is_dst);
            break;
          case SQLT_DATE:
          case SQLT_TIMESTAMP:
          case SQLT_TIMESTAMP_TZ:
          case SQLT_TIMESTAMP_LTZ:
            OCIDateTimeGetDate(preparedStmt->oci_environment,
                               preparedStmt->oci_error,
                               (OCIDateTime *) columnData->descriptor,
                               &ociYear,
                               &ociMonth,
                               &ociDay);
            if (ociYear <= -1) {
              /* In the proleptic Gregorian calendar the year preceding 1 is 0. */
              ociYear++;
            } /* if */
            *year  = ociYear;
            *month = ociMonth;
            *day   = ociDay;
            OCIDateTimeGetTime(preparedStmt->oci_environment,
                               preparedStmt->oci_error,
                               (OCIDateTime *) columnData->descriptor,
                               &ociHour,
                               &ociMinute,
                               &ociSecond,
                               &ociFsec);
            *hour   = ociHour;
            *minute = ociMinute;
            *second = ociSecond;
            *micro_second = ociFsec / 1000;
            if (columnData->buffer_type == SQLT_TIMESTAMP_TZ || columnData->buffer_type == SQLT_DATE) {
              OCIDateTimeGetTimeZoneOffset(preparedStmt->oci_environment,
                                           preparedStmt->oci_error,
                                           (OCIDateTime *) columnData->descriptor,
                                           &tzHour, &tzMinute);
              /* printf(F_D16(02) ":" F_D16(02) "\n", tzHour, tzMinute); */
              *time_zone = tzHour * 60 + tzMinute;
              *is_dst = FALSE;
            } else {
              timSetLocalTZ(*year, *month, *day, *hour, *minute, *second,
                            time_zone, is_dst);
            } /* if */
            break;
          default:
            logError(printf("sqlColumnTime: Column " FMT_D " has the unknown type %s.\n",
                            column, nameOfBufferType(columnData->buffer_type)););
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
    if (unlikely(db->oci_environment == NULL ||
                 db->oci_service_context == NULL ||
                 db->oci_error == NULL)) {
      dbNotOpen("sqlCommit");
      logError(printf("sqlCommit: Database is not open.\n"););
      raise_error(DATABASE_ERROR);
    } else {
      OCITransCommit(db->oci_service_context,
                     db->oci_error,
                     OCI_DEFAULT);
    } /* if */
    logFunction(printf("sqlCommit -->\n"););
  } /* sqlCommit */



static void sqlExecute (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    ub4 executeMode;

  /* sqlExecute */
    logFunction(printf("sqlExecute(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!allParametersBound(preparedStmt))) {
      dbLibError("sqlExecute", "SQLExecute",
                 "Unbound statement parameter(s).\n");
      raise_error(DATABASE_ERROR);
    } else {
      /* printf("ppStmt: " FMT_X_MEM "\n", (memSizeType) preparedStmt->ppStmt); */
      preparedStmt->fetchOkay = FALSE;
      showBindVars(preparedStmt);
      showResultVars(preparedStmt);
      executeMode = preparedStmt->db->autoCommit ? OCI_COMMIT_ON_SUCCESS : OCI_DEFAULT;
      if (unlikely(OCIStmtExecute(preparedStmt->oci_service_context,
                                  preparedStmt->ppStmt,
                                  preparedStmt->oci_error,
                                  preparedStmt->statementType != OCI_STMT_SELECT, /* iters */
                                  0, NULL, NULL,
                                  executeMode) != OCI_SUCCESS)) {
        setDbErrorMsg("sqlExecute", "OCIStmtExecute",
                      preparedStmt->oci_error);
        logError(printf("sqlExecute: OCIStmtExecute:\n%s\n",
                        dbError.message););
        preparedStmt->executeSuccessful = FALSE;
        raise_error(DATABASE_ERROR);
      } else {
        preparedStmt->executeSuccessful = TRUE;
        preparedStmt->fetchFinished = FALSE;
      } /* if */
    } /* if */
    logFunction(printf("sqlExecute -->\n"););
  } /* sqlExecute */



static boolType sqlFetch (sqlStmtType sqlStatement)

  {
    preparedStmtType preparedStmt;
    int fetch_result;

  /* sqlFetch */
    logFunction(printf("sqlFetch(" FMT_U_MEM ")\n",
                       (memSizeType) sqlStatement););
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(!preparedStmt->executeSuccessful)) {
      dbLibError("sqlFetch", "OCIStmtExecute",
                 "Execute was not successful.\n");
      logError(printf("sqlFetch: Execute was not successful.\n"););
      preparedStmt->fetchOkay = FALSE;
      raise_error(DATABASE_ERROR);
    } else if (preparedStmt->result_array_size == 0) {
      preparedStmt->fetchOkay = FALSE;
    } else if (!preparedStmt->fetchFinished) {
      /* printf("ppStmt: " FMT_X_MEM "\n", (memSizeType) preparedStmt->ppStmt); */
      fetch_result = OCIStmtFetch2(preparedStmt->ppStmt,
                                   preparedStmt->oci_error,
                                   1, /* Number of rows */
                                   OCI_FETCH_NEXT,
                                   0, /* Fetch offset */
                                   OCI_DEFAULT);
      if (fetch_result == OCI_SUCCESS) {
        /* printf("fetch success\n");
        showResultVars(preparedStmt); */
        preparedStmt->fetchOkay = TRUE;
      /* } else if (fetch_result == -1) {
        printf("fetch_result: %d\n", fetch_result);
        printError(preparedStmt->oci_error);
        showResultVars(preparedStmt);
        preparedStmt->fetchOkay = TRUE; */
      } else if (fetch_result == OCI_NO_DATA) {
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->fetchFinished = TRUE;
      } else {
        setDbErrorMsg("sqlFetch", "OCIStmtFetch2",
                      preparedStmt->oci_error);
        logError(printf("sqlFetch: OCIStmtFetch2 fetch_result: %d:\n%s\n",
                        fetch_result, dbError.message););
        preparedStmt->fetchOkay = FALSE;
        preparedStmt->fetchFinished = TRUE;
        raise_error(DATABASE_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("sqlFetch --> %d\n", preparedStmt->fetchOkay););
    return preparedStmt->fetchOkay;
  } /* sqlFetch */



static boolType sqlGetAutoCommit (databaseType database)

  {
    dbType db;
    boolType autoCommit;

  /* sqlGetAutoCommit */
    logFunction(printf("sqlGetAutoCommit(" FMT_U_MEM ")\n",
                       (memSizeType) database););
    db = (dbType) database;
    if (unlikely(db->oci_environment == NULL ||
                 db->oci_service_context == NULL ||
                 db->oci_error == NULL)) {
      dbNotOpen("sqlGetAutoCommit");
      logError(printf("sqlGetAutoCommit: Database is not open.\n"););
      raise_error(DATABASE_ERROR);
      autoCommit = FALSE;
    } else {
      autoCommit = db->autoCommit;
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
      isNull = preparedStmt->result_array[column - 1].indicator == -1;
    } /* if */
    logFunction(printf("sqlIsNull --> %s\n", isNull ? "TRUE" : "FALSE"););
    return isNull;
  } /* sqlIsNull */



static sqlStmtType sqlPrepare (databaseType database,
    const const_striType sqlStatementStri)

  {
    dbType db;
    striType statementStri;
    memSizeType numBindParameters;
    cstriType query;
    memSizeType queryLength;
    errInfoType err_info = OKAY_NO_ERROR;
    preparedStmtType preparedStmt;

  /* sqlPrepare */
    logFunction(printf("sqlPrepare(" FMT_U_MEM ", \"%s\")\n",
                       (memSizeType) database,
                       striAsUnquotedCStri(sqlStatementStri)););
    db = (dbType) database;
    if (unlikely(db->oci_environment == NULL ||
                 db->oci_service_context == NULL ||
                 db->oci_error == NULL)) {
      dbNotOpen("sqlPrepare");
      logError(printf("sqlPrepare: Database is not open.\n"););
      err_info = DATABASE_ERROR;
      preparedStmt = NULL;
    } else {
      statementStri = processStatementStri(sqlStatementStri, &numBindParameters, &err_info);
      if (statementStri == NULL) {
        preparedStmt = NULL;
      } else {
        query = stri_to_cstri8_buf(statementStri, &queryLength);
        if (unlikely(query == NULL)) {
          err_info = MEMORY_ERROR;
          preparedStmt = NULL;
        } else {
          /* printf("query: \"%s\"\n", query); */
          if (queryLength > UB4MAXVAL) {
            /* It is not possible to cast queryLength to ub4. */
            logError(printf("sqlPrepare: Statement string too long (length = " FMT_U_MEM ")\n",
                            queryLength););
            err_info = RANGE_ERROR;
            preparedStmt = NULL;
          } else if (!ALLOC_RECORD2(preparedStmt, preparedStmtRecordOci,
                                    count.prepared_stmt, count.prepared_stmt_bytes)) {
            err_info = MEMORY_ERROR;
          } else {
            /* printf("sqlPrepare: query: %s\n", query); */
            memset(preparedStmt, 0, sizeof(preparedStmtRecordOci));
            if (OCIHandleAlloc(db->oci_environment, (void **) &preparedStmt->ppStmt,
                               OCI_HTYPE_STMT, 0, NULL) != OCI_SUCCESS ||
                OCIStmtPrepare(preparedStmt->ppStmt, db->oci_error,
                               (OraText *) query, (ub4) queryLength,
                               OCI_NTV_SYNTAX, OCI_DEFAULT) != OCI_SUCCESS) {
              setDbErrorMsg("sqlPrepare", "OCIStmtPrepare", db->oci_error);
              logError(printf("sqlPrepare: OCIStmtPrepare:\n%s\n",
                              dbError.message););
              FREE_RECORD2(preparedStmt, preparedStmtRecordOci,
                           count.prepared_stmt, count.prepared_stmt_bytes);
              err_info = DATABASE_ERROR;
              preparedStmt = NULL;
            } else {
              preparedStmt->usage_count = 1;
              preparedStmt->sqlFunc = db->sqlFunc;
              preparedStmt->oci_environment = db->oci_environment;
              preparedStmt->oci_error = db->oci_error;
              preparedStmt->oci_service_context = db->oci_service_context;
              preparedStmt->charSetId = db->charSetId;
              preparedStmt->executeSuccessful = FALSE;
              preparedStmt->fetchOkay = FALSE;
              preparedStmt->fetchFinished = TRUE;
              preparedStmt->db = db;
              if (db->usage_count != 0) {
                db->usage_count++;
              } /* if */
              err_info = setupParameters(preparedStmt, numBindParameters);
              if (unlikely(err_info != OKAY_NO_ERROR)) {
                preparedStmt->result_array = NULL;
              } else {
                err_info = setupResult(preparedStmt);
              } /* if */
              if (unlikely(err_info != OKAY_NO_ERROR)) {
                freePreparedStmt((sqlStmtType) preparedStmt);
                preparedStmt = NULL;
              } /* if */
            } /* if */
          } /* if */
          free_cstri8(query, statementStri);
        } /* if */
        FREE_STRI2(statementStri, sqlStatementStri->size * MAX_BIND_VAR_SIZE);
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
    if (unlikely(db->oci_environment == NULL ||
                 db->oci_service_context == NULL ||
                 db->oci_error == NULL)) {
      dbNotOpen("sqlRollback");
      logError(printf("sqlRollback: Database is not open.\n"););
      raise_error(DATABASE_ERROR);
    } else {
      OCITransRollback(db->oci_service_context,
                       db->oci_error,
                       OCI_DEFAULT);
    } /* if */
    logFunction(printf("sqlRollback -->\n"););
  } /* sqlRollback */



static void sqlSetAutoCommit (databaseType database, boolType autoCommit)

  {
    dbType db;

  /* sqlSetAutoCommit */
    logFunction(printf("sqlSetAutoCommit(" FMT_U_MEM ", %d)\n",
                       (memSizeType) database, autoCommit););
    db = (dbType) database;
    if (unlikely(db->oci_environment == NULL ||
                 db->oci_service_context == NULL ||
                 db->oci_error == NULL)) {
      dbNotOpen("sqlSetAutoCommit");
      logError(printf("sqlSetAutoCommit: Database is not open.\n"););
      raise_error(DATABASE_ERROR);
    } else {
      db->autoCommit = autoCommit;
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
      logError(printf("sqlStmtColumnCount: "
                      "result_array_size (=" FMT_U_MEM ") > INTTYPE_MAX\n",
                      preparedStmt->result_array_size););
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
    cstriType col_name;
    ub4 col_name_len = 0;
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
      raise_error(RANGE_ERROR);
      name = NULL;
    } else if (unlikely(OCIAttrGet(preparedStmt->result_array[column - 1].column_handle,
                                   OCI_DTYPE_PARAM,
                                   &col_name,
                                   &col_name_len,
                                   OCI_ATTR_NAME,
                                   preparedStmt->oci_error) != OCI_SUCCESS)) {
      setDbErrorMsg("sqlStmtColumnName", "OCIAttrGet",
                    preparedStmt->oci_error);
      logError(printf("sqlStmtColumnName: OCIAttrGet OCI_ATTR_NAME:\n%s\n",
                      dbError.message););
      raise_error(DATABASE_ERROR);
      name = NULL;
    } else {
      name = cstri8_buf_to_stri(col_name, (memSizeType) col_name_len, &err_info);
      if (unlikely(name == NULL)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunction(printf("sqlStmtColumnName --> \"%s\"\n",
                       striAsUnquotedCStri(name)););
    return name;
  } /* sqlStmtColumnName */



#if 0
static rtlTypeType sqlStmtColumnType (sqlStmtType sqlStatement, intType column)

  {
    preparedStmtType preparedStmt;
    resultDataType columnData;
    sb2 precision; /* This is for an implicit describe with OCIStmtExecute() */
    sb1 scale;
    rtlTypeType columnType;

  /* sqlStmtColumnType */
    logFunction(printf("sqlStmtColumnType(" FMT_U_MEM ", " FMT_D ")\n",
                       (memSizeType) sqlStatement, column));
    preparedStmt = (preparedStmtType) sqlStatement;
    if (unlikely(column < 1 ||
                 (uintType) column > preparedStmt->result_array_size)) {
      logError(printf("sqlStmtColumnType: column: " FMT_D
                      ", max column: " FMT_U_MEM ".\n",
                      column, preparedStmt->result_array_size););
      raise_error(RANGE_ERROR);
      columnType = SYS_VOID_TYPE;
    } else {
      columnData = &preparedStmt->result_array[column - 1];
      switch (columnData->buffer_type) {
        case SQLT_CHR: /* VARCHAR2 */
        case SQLT_VCS: /* VARCHAR */
        case SQLT_LNG: /* LONG */
        case SQLT_LVC: /* LONG VARCHAR */
        case SQLT_AFC: /* CHAR */
        case SQLT_STR: /* NULL-terminated STRING */
        case SQLT_AVC: /* CHARZ */
        case SQLT_VST: /* OCI STRING type */
        case SQLT_CLOB: /* Character LOB  */
          columnType = SYS_STRI_TYPE;
          break;
        case SQLT_BIN: /* RAW */
        case SQLT_VBI: /* VARRAW */
        case SQLT_LBI: /* LONG RAW */
        case SQLT_LVB: /* LONG VARRAW */
        case SQLT_BLOB: /* Binary LOB */
        case SQLT_FILE: /* Binary FILE */
          columnType = SYS_BSTRI_TYPE;
          break;
        case SQLT_RDD: /* ROWID */
          columnType = SYS_STRI_TYPE;
          break;
        case SQLT_NUM:
          if (unlikely(OCIAttrGet(columnData->column_handle,
                                  OCI_DTYPE_PARAM,
                                  (dvoid *) &precision, NULL, OCI_ATTR_PRECISION,
                                  preparedStmt->oci_error) != OCI_SUCCESS)) {
            setDbErrorMsg("sqlStmtColumnType", "OCIAttrGet",
                          preparedStmt->oci_error);
            logError(printf("sqlStmtColumnType: OCIAttrGet OCI_ATTR_PRECISION:\n%s\n",
                            dbError.message););
            raise_error(DATABASE_ERROR);
          } else if (unlikely(OCIAttrGet(columnData->column_handle,
                                         OCI_DTYPE_PARAM,
                                         (dvoid *) &scale, NULL, OCI_ATTR_SCALE,
                                         preparedStmt->oci_error) != OCI_SUCCESS)) {
            setDbErrorMsg("sqlStmtColumnType", "OCIAttrGet",
                          preparedStmt->oci_error);
            logError(printf("sqlStmtColumnType: OCIAttrGet OCI_ATTR_SCALE:\n%s\n",
                            dbError.message););
            raise_error(DATABASE_ERROR);
          } else {
            printf("precision: %u\n", precision);
            printf("scale: %u\n", scale);
            if (scale <= 0) {
              if (precision - scale <= DECIMAL_DIGITS_IN_INTTYPE) {
                columnType = SYS_INT_TYPE;
              } else {
                columnType = SYS_BIGINT_TYPE;
              } /* if */
            } else {
              columnType = SYS_BIGRAT_TYPE;
            } /* if */
          } /* if */
          break;
        default:
          logError(printf("sqlStmtColumnType: Column " FMT_D " has the unknown type %s.\n",
                          column, nameOfBufferType(
                          columnData->buffer_type)););
          raise_error(RANGE_ERROR);
          columnType = SYS_VOID_TYPE;
          break;
      } /* switch */
    } /* if */
    return columnType;
  } /* sqlStmtColumnType */
#endif



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



static boolType setupConnectData (const_cstriType host8, intType port, const_cstriType dbName8, connectDataType connectData)

  {
    boolType possibleTnsName;
    memSizeType portNameLength;
    char portName[INTTYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    memSizeType host8Length;
    memSizeType dbName8Length;
    boolType okay = FALSE;

  /* setupConnectData */
    logFunction(printf("setupConnectData(\"%s\", " FMT_D ", \"%s\", *)\n",
                       host8, port, dbName8););
    possibleTnsName = host8[0] == '\0' && port == 0;
    if (host8[0] == '\0') {
      host8 = "localhost";
    } /* if */
    if (port == 0) {
      port = DEFAULT_PORT;
    } /* if */
    portNameLength = (memSizeType) sprintf(portName, FMT_D, port);
    host8Length = strlen(host8);
    dbName8Length = strlen(dbName8);
    if (likely(host8Length <= MAX_CSTRI_LEN / 4 && dbName8Length <= MAX_CSTRI_LEN / 4)) {
      /* The computation of the connect string lengths is safe. */
      /* printf("host8: \"%s\"\n", host8);
         printf("port: \"%s\"\n", portName);
         printf("dbName8: \"%s\"\n", dbName8); */
      memset(connectData, 0, sizeof(connectDataRecordOci));
      if (possibleTnsName &&
          ALLOC_CSTRI(connectData->tnsName, dbName8Length)) {
        connectData->tnsNameLength = dbName8Length;
        memcpy(connectData->tnsName, dbName8, dbName8Length + NULL_TERMINATION_LEN);
      } /* if */
      if (likely(!possibleTnsName || connectData->tnsName != NULL)) {
        connectData->connectStringServiceLength = STRLEN(CONNECT_STRING_SERVICE_TEMPLATE) - 3 * 2 +
            host8Length + portNameLength + dbName8Length;
        connectData->connectStringSidLength = STRLEN(CONNECT_STRING_SID_TEMPLATE) - 3 * 2 +
            host8Length + portNameLength + dbName8Length;
        if (unlikely(!ALLOC_CSTRI(connectData->connectStringService,
                                  connectData->connectStringServiceLength) ||
                     !ALLOC_CSTRI(connectData->connectStringSid,
                                  connectData->connectStringSidLength))) {
          UNALLOC_CSTRI(connectData->tnsName, connectData->tnsNameLength);
          UNALLOC_CSTRI(connectData->connectStringService, connectData->connectStringServiceLength);
          UNALLOC_CSTRI(connectData->connectStringSid, connectData->connectStringSidLength);
        } else {
          sprintf(connectData->connectStringService, CONNECT_STRING_SERVICE_TEMPLATE,
                  host8, portName, dbName8);
          sprintf(connectData->connectStringSid, CONNECT_STRING_SID_TEMPLATE,
                  host8, portName, dbName8);
          /* printf("connectStringService: %s\n", connectData->connectStringService);
             printf("connectStringServiceLength: " FMT_U_MEM "\n",
                    connectData->connectStringServiceLength); */
          /* printf("connectStringSid: %s\n", connectData->connectStringSid);
             printf("connectStringSidLength: " FMT_U_MEM "\n",
                    connectData->connectStringSidLength); */
          okay = TRUE;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setupConnectData --> \"%d\"\n", okay););
    return okay;
  } /* setupConnectData */



databaseType sqlOpenOci (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  {
    const_cstriType host8;
    const_cstriType dbName8;
    connectDataRecordOci connectData;
    cstriType user8;
    memSizeType user8Length;
    cstriType password8;
    memSizeType password8Length;
    dbRecordOci db;
    errInfoType err_info = OKAY_NO_ERROR;
    dbType database;

  /* sqlOpenOci */
    logFunction(printf("sqlOpenOci(\"%s\", ",
                       striAsUnquotedCStri(host));
                printf(FMT_D ", \"%s\", ",
                       port, striAsUnquotedCStri(dbName));
                printf("\"%s\", *)\n", striAsUnquotedCStri(user)););
    if (!findDll()) {
      logError(printf("sqlOpenOci: findDll() failed\n"););
      err_info = DATABASE_ERROR;
      database = NULL;
    } else if (unlikely((host8 = stri_to_cstri8(host, &err_info)) == NULL)) {
      database = NULL;
    } else {
      if (unlikely((dbName8 = stri_to_cstri8(dbName, &err_info)) == NULL)) {
        database = NULL;
      } else {
        if (unlikely(!setupConnectData(host8, port, dbName8, &connectData))) {
          err_info = MEMORY_ERROR;
          database = NULL;
        } else {
          user8 = stri_to_cstri8_buf(user, &user8Length);
          if (unlikely(user8 == NULL)) {
            err_info = MEMORY_ERROR;
            database = NULL;
          } else {
            password8 = stri_to_cstri8_buf(password, &password8Length);
            if (unlikely(password8 == NULL)) {
              err_info = MEMORY_ERROR;
              database = NULL;
            } else {
              memset(&db, 0, sizeof(dbRecordOci));
              /* printf("dbName:         %s\n", connectData.tnsName);
                 printf("connectString1: %s\n", connectData.connectStringService);
                 printf("connectString2: %s\n", connectData.connectStringSid);
                 printf("user:           %s\n", user8);
                 printf("password:       *\n"); */
              if (connectData.tnsNameLength              > SB4MAXVAL ||
                  connectData.connectStringServiceLength > SB4MAXVAL ||
                  connectData.connectStringSidLength     > SB4MAXVAL ||
                  user8Length         > UB4MAXVAL ||
                  password8Length     > UB4MAXVAL) {
                /* It is not possible to cast the lengths to sb4 and ub4. */
                err_info = MEMORY_ERROR;
                database = NULL;
              } else if (OCIEnvCreate(&db.oci_environment, OCI_DEFAULT,
                                      NULL, NULL, NULL, NULL, 0, NULL) != OCI_SUCCESS) {
                dbLibError("sqlOpenOci", "OCIEnvCreate",
                           "OCIEnvCreate failed.\n");
                logError(printf("sqlOpenOci: OCIEnvCreate failed\n"););
                sqlClose((databaseType) &db);
                err_info = DATABASE_ERROR;
                database = NULL;
              } else if ((db.charSetId = OCINlsCharSetNameToId(db.oci_environment,
                                                               (const oratext *) "AL32UTF8")) == 0) {
                dbLibError("sqlOpenOci", "OCINlsCharSetNameToId",
                           "OCINlsCharSetNameToId(*, \"AL32UTF8\") failed.\n");
                logError(printf("sqlOpenOci: OCINlsCharSetNameToId(*, \"AL32UTF8\") failed\n"););
                sqlClose((databaseType) &db);
                err_info = DATABASE_ERROR;
                database = NULL;
              } else if (OCIHandleFree(db.oci_environment, OCI_HTYPE_ENV) != OCI_SUCCESS) {
                dbLibError("sqlOpenOci", "OCIHandleFree",
                           "OCIHandleFree failed.\n");
                logError(printf("sqlOpenOci: OCIHandleFree failed\n"););
                sqlClose((databaseType) &db);
                err_info = DATABASE_ERROR;
                database = NULL;
              } else if (OCIEnvNlsCreate(&db.oci_environment, OCI_OBJECT, /* OCI_DEFAULT, */
                                         NULL, NULL, NULL, NULL, 0, NULL,
                                         db.charSetId, db.charSetId) != OCI_SUCCESS) {
                dbLibError("sqlOpenOci", "OCIEnvNlsCreate",
                           "OCIEnvNlsCreate failed.\n");
                logError(printf("sqlOpenOci: OCIEnvNlsCreate failed\n"););
                err_info = DATABASE_ERROR;
                sqlClose((databaseType) &db);
                database = NULL;
              } else if (OCIHandleAlloc(db.oci_environment,
                                        (dvoid **) &db.oci_server,
                                        OCI_HTYPE_SERVER, 0, NULL) != OCI_SUCCESS ||
                         OCIHandleAlloc(db.oci_environment,
                                        (void **) &db.oci_error,
                                        OCI_HTYPE_ERROR, 0, NULL) != OCI_SUCCESS ||
                         OCIHandleAlloc(db.oci_environment,
                                        (void **) &db.oci_service_context,
                                        OCI_HTYPE_SVCCTX, 0, NULL) != OCI_SUCCESS ||
                         OCIHandleAlloc(db.oci_environment,
                                        (dvoid **) &db.oci_session,
                                        OCI_HTYPE_SESSION, 0, NULL) != OCI_SUCCESS) {
                dbLibError("sqlOpenOci", "OCIHandleAlloc",
                           "OCIHandleAlloc failed.\n");
                logError(printf("sqlOpenOci: OCIHandleAlloc failed\n"););
                err_info = DATABASE_ERROR;
                sqlClose((databaseType) &db);
                database = NULL;
              } else if ((connectData.tnsName == NULL ||
                          /* Possible TNS name: No host and no port has been specified. */
                          OCIServerAttach(db.oci_server, db.oci_error,
                                          (OraText *) connectData.tnsName,
                                          (sb4) connectData.tnsNameLength,
                                          OCI_DEFAULT) != OCI_SUCCESS) &&
                         OCIServerAttach(db.oci_server, db.oci_error,
                                         (OraText *) connectData.connectStringService,
                                         (sb4) connectData.connectStringServiceLength,
                                         OCI_DEFAULT) != OCI_SUCCESS &&
                         OCIServerAttach(db.oci_server, db.oci_error,
                                         (OraText *) connectData.connectStringSid,
                                         (sb4) connectData.connectStringSidLength,
                                         OCI_DEFAULT) != OCI_SUCCESS) {
                setDbErrorMsg("sqlOpenOci", "OCIServerAttach", db.oci_error);
                logError(printf("sqlOpenOci: OCIServerAttach(*, \"%s\", " FMT_U_MEM ", OCI_DEFAULT):\n%s\n",
                                connectData.connectStringService,
                                connectData.connectStringServiceLength, dbError.message););
                err_info = DATABASE_ERROR;
                sqlClose((databaseType) &db);
                database = NULL;
              } else if (OCIAttrSet(db.oci_service_context, OCI_HTYPE_SVCCTX,
                                    db.oci_server, (ub4) 0,
                                    OCI_ATTR_SERVER, db.oci_error) != OCI_SUCCESS) {
                setDbErrorMsg("sqlOpenOci", "OCIAttrSet", db.oci_error);
                logError(printf("sqlOpenOci: OCIAttrSet OCI_ATTR_SERVER:\n%s\n",
                                dbError.message););
                err_info = DATABASE_ERROR;
                sqlClose((databaseType) &db);
                database = NULL;
              } else if (OCIAttrSet(db.oci_session, OCI_HTYPE_SESSION,
                                    user8, (ub4) user8Length,
                                    OCI_ATTR_USERNAME, db.oci_error) != OCI_SUCCESS) {
                setDbErrorMsg("sqlOpenOci", "OCIAttrSet", db.oci_error);
                logError(printf("sqlOpenOci: OCIAttrSet OCI_ATTR_USERNAME:\n%s\n",
                                dbError.message););
                err_info = DATABASE_ERROR;
                sqlClose((databaseType) &db);
                database = NULL;
              } else if (OCIAttrSet(db.oci_session, OCI_HTYPE_SESSION,
                                    password8, (ub4) password8Length,
                                    OCI_ATTR_PASSWORD, db.oci_error) != OCI_SUCCESS) {
                setDbErrorMsg("sqlOpenOci", "OCIAttrSet", db.oci_error);
                logError(printf("sqlOpenOci: OCIAttrSet OCI_ATTR_PASSWORD:\n%s\n",
                                dbError.message););
                err_info = DATABASE_ERROR;
                sqlClose((databaseType) &db);
                database = NULL;
              } else if (OCISessionBegin(db.oci_service_context, db.oci_error, db.oci_session,
                                         OCI_CRED_RDBMS, OCI_DEFAULT) != OCI_SUCCESS) {
                setDbErrorMsg("sqlOpenOci", "OCISessionBegin", db.oci_error);
                logError(printf("sqlOpenOci: OCISessionBegin:\n%s\n",
                                dbError.message););
                err_info = DATABASE_ERROR;
                sqlClose((databaseType) &db);
                database = NULL;
              } else if (OCIAttrSet(db.oci_service_context, OCI_HTYPE_SVCCTX,
                                    db.oci_session, (ub4) 0,
                                    OCI_ATTR_SESSION, db.oci_error) != OCI_SUCCESS) {
                setDbErrorMsg("sqlOpenOci", "OCIAttrSet", db.oci_error);
                logError(printf("sqlOpenOci: OCIAttrSet OCI_ATTR_SESSION:\n%s\n",
                                dbError.message););
                err_info = DATABASE_ERROR;
                sqlClose((databaseType) &db);
                database = NULL;
              } else if (unlikely(!setupFuncTable() ||
                                  !ALLOC_RECORD2(database, dbRecordOci,
                                                 count.database, count.database_bytes))) {
                err_info = MEMORY_ERROR;
                sqlClose((databaseType) &db);
                database = NULL;
              } else {
                memset(database, 0, sizeof(dbRecordOci));
                database->usage_count = 1;
                database->isOpen = TRUE;
                database->sqlFunc = sqlFunc;
                database->driver = DB_CATEGORY_OCI;
                database->oci_environment     = db.oci_environment;
                database->oci_server          = db.oci_server;
                database->oci_error           = db.oci_error;
                database->oci_session         = db.oci_session;
                database->oci_service_context = db.oci_service_context;
                database->charSetId           = db.charSetId;
                database->autoCommit = TRUE;
              } /* if */
              free_cstri8(password8, password);
            } /* if */
            free_cstri8(user8, user);
          } /* if */
          UNALLOC_CSTRI(connectData.tnsName, connectData.tnsNameLength);
          UNALLOC_CSTRI(connectData.connectStringService, connectData.connectStringServiceLength);
          UNALLOC_CSTRI(connectData.connectStringSid, connectData.connectStringSidLength);
        } /* if */
        free_cstri8(dbName8, dbName);
      } /* if */
      free_cstri8(host8, host);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("sqlOpenOci --> " FMT_U_MEM "\n",
                       (memSizeType) database););
    return (databaseType) database;
  } /* sqlOpenOci */

#else



databaseType sqlOpenOci (const const_striType host, intType port,
    const const_striType dbName, const const_striType user,
    const const_striType password)

  { /* sqlOpenOci */
    logError(printf("sqlOpenOci(\"%s\", ",
                    striAsUnquotedCStri(host));
             printf(FMT_D ", \"%s\", ",
                    port, striAsUnquotedCStri(dbName));
             printf("\"%s\", *): OCI driver not present.\n",
                    striAsUnquotedCStri(user)););
    raise_error(RANGE_ERROR);
    return NULL;
  } /* sqlOpenOci */

#endif
