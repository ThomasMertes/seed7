/********************************************************************/
/*                                                                  */
/*  db_oci.h      OCI interfaces used by Seed7.                     */
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
/*  File: seed7/src/db_oci.h                                        */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: OCI interfaces used by Seed7.                          */
/*                                                                  */
/********************************************************************/

#define boolean int
#define dvoid void

typedef unsigned int uword;
typedef signed int   sword;

typedef UINT8TYPE  ub1;
typedef UINT16TYPE ub2;
typedef UINT32TYPE ub4;
typedef INT8TYPE   sb1;
typedef INT16TYPE  sb2;
typedef INT32TYPE  sb4;

#define UB4MAXVAL UINT32TYPE_MAX
#define SB4MAXVAL INT32TYPE_MAX

typedef UINT64TYPE     oraub8;
typedef unsigned char oratext;
typedef oratext       OraText;

typedef ub2 OCIDuration;

typedef struct { int dummy; } OCIBind;
typedef struct { int dummy; } OCIDateTime;
typedef struct { int dummy; } OCIDefine;
typedef struct { int dummy; } OCIEnv;
typedef struct { int dummy; } OCIError;
typedef struct { int dummy; } OCIInterval;
typedef struct { int dummy; } OCILobLocator;
typedef struct { int dummy; } OCIParam;
typedef struct { int dummy; } OCIRef;
typedef struct { int dummy; } OCIRowid;
typedef struct { int dummy; } OCIServer;
typedef struct { int dummy; } OCISession;
typedef struct { int dummy; } OCISnapshot;
typedef struct { int dummy; } OCIStmt;
typedef struct { int dummy; } OCIString;
typedef struct { int dummy; } OCISvcCtx;
typedef struct { int dummy; } OCIType;

#define OCI_NUMBER_SIZE 22
typedef struct {
    ub1 OCINumberPart[OCI_NUMBER_SIZE];
  } OCINumber;

typedef sb4 (*OCICallbackDefine) (void  *octxp, OCIDefine *defnp, ub4 iter,
                                  void  **bufpp, ub4 **alenp, ub1 *piecep,
                                  void  **indp, ub2 **rcodep);
typedef sb4 (*OCICallbackLobRead2) (void  *ctxp, const void  *bufp, oraub8 len,
                                    ub1 piece, void  **changed_bufpp,
                                    oraub8 *changed_lenp);
typedef sb4 (*OCICallbackLobWrite2) (void  *ctxp, void  *bufp, oraub8 *lenp,
                                     ub1 *piece, void  **changed_bufpp,
                                     oraub8 *changed_lenp);

#define OCI_CRED_RDBMS 1

#define OCI_SUCCESS       0
#define OCI_NO_DATA     100
#define OCI_CONTINUE -24200

#define OCI_DURATION_SESSION (OCIDuration) 10

#define OCI_HTYPE_ENV      1
#define OCI_HTYPE_ERROR    2
#define OCI_HTYPE_SVCCTX   3
#define OCI_HTYPE_STMT     4
#define OCI_HTYPE_SERVER   8
#define OCI_HTYPE_SESSION  9

#define OCI_ATTR_SERVER       6
#define OCI_ATTR_SESSION      7
#define OCI_ATTR_PARAM_COUNT 18
#define OCI_ATTR_USERNAME    22
#define OCI_ATTR_PASSWORD    23
#define OCI_ATTR_STMT_TYPE   24

#define OCI_ATTR_DATA_SIZE  1
#define OCI_ATTR_DATA_TYPE  2
#define OCI_ATTR_NAME       4

#define OCI_DTYPE_LOB           50
#define OCI_DTYPE_PARAM         53
#define OCI_DTYPE_ROWID         54
#define OCI_DTYPE_FILE          56
#define OCI_DTYPE_INTERVAL_YM   62
#define OCI_DTYPE_INTERVAL_DS   63
#define OCI_DTYPE_DATE          65
#define OCI_DTYPE_TIMESTAMP     68
#define OCI_DTYPE_TIMESTAMP_TZ  69
#define OCI_DTYPE_TIMESTAMP_LTZ 70

#define OCI_TEMP_BLOB 1

#define SQLCS_IMPLICIT 1

#define OCI_ONE_PIECE   0
#define OCI_FIRST_PIECE 1
#define OCI_NEXT_PIECE  2
#define OCI_LAST_PIECE  3

#define OCI_DESCRIBE_ONLY 0x10

#define OCI_STMT_SELECT 1

#define OCI_NTV_SYNTAX 1

#define OCI_FETCH_NEXT 0x02

#define OCI_DYNAMIC_FETCH 0x02

#define OCI_DEFAULT 0x00

#define OCI_OBJECT 0x02

#define SQLT_CHR  1
#define SQLT_NUM  2
#define SQLT_INT  3
#define SQLT_FLT  4
#define SQLT_STR  5
#define SQLT_LNG  8
#define SQLT_VCS  9
#define SQLT_DAT  12
#define SQLT_VBI  15
#define SQLT_BIN  23
#define SQLT_LBI  24
#define SQLT_LVC  94
#define SQLT_LVB  95
#define SQLT_AFC  96
#define SQLT_AVC  97
#define SQLT_RDD  104
#define SQLT_REF  110
#define SQLT_CLOB 112
#define SQLT_BLOB 113
#define SQLT_FILE 114
#define SQLT_VST  155
#define SQLT_DATE 184
#define SQLT_TIMESTAMP     187
#define SQLT_TIMESTAMP_TZ  188
#define SQLT_INTERVAL_YM   189
#define SQLT_INTERVAL_DS   190
#define SQLT_TIMESTAMP_LTZ 232

sword OCIAttrGet (const void  *trgthndlp, ub4 trghndltyp,
                  void  *attributep, ub4 *sizep, ub4 attrtype,
                  OCIError *errhp);
sword OCIAttrSet (void  *trgthndlp, ub4 trghndltyp, void  *attributep,
                  ub4 size, ub4 attrtype, OCIError *errhp);
sword OCIBindByPos (OCIStmt *stmtp, OCIBind **bindp, OCIError *errhp,
                    ub4 position, void  *valuep, sb4 value_sz,
                    ub2 dty, void  *indp, ub2 *alenp, ub2 *rcodep,
                    ub4 maxarr_len, ub4 *curelep, ub4 mode);
sword OCIDateTimeConstruct (void *hndl,OCIError *err,OCIDateTime *datetime,
                            sb2 yr,ub1 mnth,ub1 dy,ub1 hr,ub1 mm,ub1 ss,ub4 fsec,
                            OraText *timezone,size_t timezone_length);
sword OCIDateTimeGetDate (void  *hndl, OCIError *err,  const OCIDateTime *date,
                          sb2 *yr, ub1 *mnth, ub1 *dy);
sword OCIDateTimeGetTime (void  *hndl, OCIError *err, OCIDateTime *datetime,
                          ub1 *hr, ub1 *mm, ub1 *ss, ub4 *fsec);
sword OCIDefineByPos (OCIStmt *stmtp, OCIDefine **defnp, OCIError *errhp,
                      ub4 position, void  *valuep, sb4 value_sz, ub2 dty,
                      void  *indp, ub2 *rlenp, ub2 *rcodep, ub4 mode);
sword OCIDefineDynamic (OCIDefine *defnp, OCIError *errhp, void  *octxp,
                        OCICallbackDefine ocbfp);
sword OCIDefineObject (OCIDefine *defnp, OCIError *errhp,
                       const OCIType *type, void  **pgvpp,
                       ub4 *pvszsp, void  **indpp, ub4 *indszp);
sword OCIDescriptorAlloc (const void  *parenth, void  **descpp,
                          const ub4 type, const size_t xtramem_sz,
                          void  **usrmempp);
sword OCIDescriptorFree (void  *descp, const ub4 type);
sword OCIEnvCreate (OCIEnv **envp, ub4 mode, void  *ctxp,
                    void  *(*malocfp)(void  *ctxp, size_t size),
                    void  *(*ralocfp)(void  *ctxp, void  *memptr, size_t newsize),
                    void   (*mfreefp)(void  *ctxp, void  *memptr),
                    size_t xtramem_sz, void  **usrmempp);
sword OCIEnvNlsCreate (OCIEnv **envp, ub4 mode, void  *ctxp,
                       void  *(*malocfp)(void  *ctxp, size_t size),
                       void  *(*ralocfp)(void  *ctxp, void  *memptr, size_t newsize),
                       void   (*mfreefp)(void  *ctxp, void  *memptr),
                       size_t xtramem_sz, void  **usrmempp,
                       ub2 charset, ub2 ncharset);
sword OCIErrorGet (void  *hndlp, ub4 recordno, OraText *sqlstate,
                   sb4 *errcodep, OraText *bufp, ub4 bufsiz, ub4 type);
sword OCIHandleAlloc (const void  *parenth, void  **hndlpp, const ub4 type,
                      const size_t xtramem_sz, void  **usrmempp);
sword OCIHandleFree (void  *hndlp, const ub4 type);
sword OCIIntervalSetDaySecond (void  *hndl, OCIError *err, sb4 dy, sb4 hr,
                               sb4 mm, sb4 ss, sb4 fsec, OCIInterval *result);
sword OCIIntervalSetYearMonth (void  *hndl, OCIError *err, sb4 yr, sb4 mnth,
                               OCIInterval *result);
sword OCIIntervalGetDaySecond (void  *hndl, OCIError *err, sb4 *dy, sb4 *hr,
                               sb4 *mm, sb4 *ss, sb4 *fsec, const OCIInterval *result);
sword OCIIntervalGetYearMonth (void  *hndl, OCIError *err, sb4 *yr, sb4 *mnth,
                               const OCIInterval *result);
sword OCILobCreateTemporary (OCISvcCtx *svchp,
                             OCIError *errhp,
                             OCILobLocator *locp,
                             ub2 csid,
                             ub1 csfrm,
                             ub1 lobtype,
                             boolean cache,
                             OCIDuration duration);
sword OCILobFreeTemporary (OCISvcCtx *svchp,
                           OCIError *errhp,
                           OCILobLocator *locp);
sword OCILobGetLength2 (OCISvcCtx *svchp, OCIError *errhp,
                        OCILobLocator *locp, oraub8 *lenp);
sword OCILobIsTemporary (OCIEnv *envp,
                         OCIError *errhp,
                         OCILobLocator *locp,
                         boolean *is_temporary);
sword OCILobRead2 (OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                   oraub8 *byte_amtp, oraub8 *char_amtp, oraub8 offset,
                   void  *bufp, oraub8 bufl, ub1 piece, void  *ctxp,
                   OCICallbackLobRead2 cbfp, ub2 csid, ub1 csfrm);
sword OCILobTrim2 (OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                   oraub8 newlen);
sword OCILobWrite2 (OCISvcCtx *svchp, OCIError *errhp, OCILobLocator *locp,
                    oraub8 *byte_amtp, oraub8 *char_amtp, oraub8 offset,
                    void  *bufp, oraub8 buflen, ub1 piece, void  *ctxp,
                    OCICallbackLobWrite2 cbfp, ub2 csid, ub1 csfrm);
ub2 OCINlsCharSetNameToId (void  *envhp, const oratext *name);
sword OCINumberToReal (OCIError *err, const OCINumber *number,
                       uword rsl_length, void  *rsl);
sword OCIParamGet (const void  *hndlp, ub4 htype, OCIError *errhp,
                   void  **parmdpp, ub4 pos);
ub4 OCIRefHexSize (OCIEnv *env, const OCIRef *ref);
sword OCIRefToHex (OCIEnv *env, OCIError *err, const OCIRef *ref,
                   oratext *hex, ub4 *hex_length);
sword OCIRowidToChar (OCIRowid *rowidDesc, OraText *outbfp, ub2 *outbflp,
                      OCIError *errhp);
sword OCIServerAttach (OCIServer *srvhp, OCIError *errhp,
                       const OraText *dblink, sb4 dblink_len, ub4 mode);
sword OCIServerDetach (OCIServer *srvhp, OCIError *errhp, ub4 mode);
sword OCISessionBegin (OCISvcCtx *svchp, OCIError *errhp, OCISession *usrhp,
                       ub4 credt, ub4 mode);
sword OCISessionEnd (OCISvcCtx *svchp, OCIError *errhp, OCISession *usrhp,
                     ub4 mode);
sword OCIStmtExecute (OCISvcCtx *svchp, OCIStmt *stmtp, OCIError *errhp,
                      ub4 iters, ub4 rowoff, const OCISnapshot *snap_in,
                      OCISnapshot *snap_out, ub4 mode);
sword OCIStmtFetch2 (OCIStmt *stmtp, OCIError *errhp, ub4 nrows,
                     ub2 orientation, sb4 scrollOffset, ub4 mode);
sword OCIStmtPrepare (OCIStmt *stmtp, OCIError *errhp, const OraText *stmt,
                      ub4 stmt_len, ub4 language, ub4 mode);
oratext *OCIStringPtr (OCIEnv *env, const OCIString *vs);
ub4 OCIStringSize (OCIEnv *env, const OCIString *vs);
sword OCITransCommit (OCISvcCtx *svchp, OCIError *errhp, ub4 flags);
