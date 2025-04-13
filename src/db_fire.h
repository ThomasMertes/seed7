/********************************************************************/
/*                                                                  */
/*  db_fire.h     Firebird/InterBase interfaces used by Seed7.      */
/*  Copyright (C) 1989 - 2018  Thomas Mertes                        */
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
/*  Changes: 2018  Thomas Mertes                                    */
/*  Content: Firebird/InterBase interfaces used by Seed7.           */
/*                                                                  */
/********************************************************************/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define ISC_FAR

#define METADATALENGTH 68

#define isc_dpb_version1         1
#define isc_tpb_version3         3
#define isc_tpb_concurrency      2
#define isc_tpb_wait             6
#define isc_tpb_write            9
#define isc_dpb_user_name       28
#define isc_dpb_password        29
#define isc_dpb_set_db_charset  68
#define isc_dpb_utf8_filename   77

#define isc_info_sql_stmt_type  21

#define isc_info_sql_stmt_select 1
#define isc_info_sql_stmt_insert 2
#define isc_info_sql_stmt_update 3
#define isc_info_sql_stmt_delete 4
#define isc_info_sql_stmt_ddl    5

#define SQLDA_VERSION1  1

#define DSQL_close 1
#define DSQL_drop  2

#define SQL_TEXT        452
#define SQL_VARYING     448
#define SQL_SHORT       500
#define SQL_LONG        496
#define SQL_FLOAT       482
#define SQL_DOUBLE      480
#define SQL_D_FLOAT     530
#define SQL_TIMESTAMP   510
#define SQL_BLOB        520
#define SQL_ARRAY       540
#define SQL_QUAD        550
#define SQL_TYPE_TIME   560
#define SQL_TYPE_DATE   570
#define SQL_INT64       580
#define SQL_BOOLEAN   32764
#define SQL_NULL      32766

typedef unsigned char  ISC_UCHAR;
typedef char           ISC_SCHAR;

typedef INT16TYPE      ISC_SHORT;
typedef UINT16TYPE     ISC_USHORT;

typedef INT32TYPE      ISC_LONG;
typedef UINT32TYPE     ISC_ULONG;

typedef INT64TYPE      ISC_INT64;
typedef UINT64TYPE     ISC_UINT64;

typedef INT32TYPE      ISC_DATE;
typedef UINT32TYPE     ISC_TIME;

typedef struct{
    ISC_DATE timestamp_date;
    ISC_TIME timestamp_time;
  } ISC_TIMESTAMP;

typedef struct {
    ISC_LONG isc_quad_high;
    ISC_ULONG isc_quad_low;
  } ISC_QUAD;

typedef UINT32TYPE     FB_API_HANDLE;

#if POINTER_SIZE == 32
typedef INT32TYPE  ISC_STATUS;
#elif POINTER_SIZE == 64
typedef INT64TYPE  ISC_STATUS;
#endif

const ISC_STATUS isc_segment = 335544366L;

typedef FB_API_HANDLE isc_blob_handle;
typedef FB_API_HANDLE isc_db_handle;
typedef FB_API_HANDLE isc_stmt_handle;
typedef FB_API_HANDLE isc_tr_handle;

#define isc_info_end                1
#define isc_info_truncated          2
#define isc_info_blob_total_length  6

typedef struct {
    short  sqltype;
    short  sqlscale;
    short  sqlsubtype;
    short  sqllen;
    char  *sqldata;
    short *sqlind;
    short  sqlname_length;
    char   sqlname[METADATALENGTH];
    short  relname_length;
    char   relname[METADATALENGTH];
    short  ownname_length;
    char   ownname[METADATALENGTH];
    short  aliasname_length;
    char   aliasname[METADATALENGTH];
  } XSQLVAR;

typedef struct {
    short    version;
    char     sqldaid[8];
    ISC_LONG sqldabc;
    short    sqln;
    short    sqld;
    XSQLVAR  sqlvar[1];
  } XSQLDA;

#define XSQLDA_LENGTH(n)  (sizeof(XSQLDA) + ((n) - 1) * sizeof(XSQLVAR))

#ifndef STDCALL
#if defined(_WIN32) && HAS_STDCALL
#define STDCALL __stdcall
#else
#define STDCALL
#endif
#endif

ISC_STATUS STDCALL isc_attach_database (ISC_STATUS *status_vector,
                                        short db_name_length,
                                        char *db_name,
                                        isc_db_handle *db_handle,
                                        short parm_buffer_length,
                                        char *parm_buffer);
ISC_STATUS STDCALL isc_blob_info (ISC_STATUS *status_vector,
                                  isc_blob_handle *blob_handle,
                                  short item_list_buffer_length,
                                  char *item_list_buffer,
                                  short result_buffer_length,
                                  char *result_buffer);
ISC_STATUS STDCALL isc_close_blob (ISC_STATUS *status_vector,
                                   isc_blob_handle *blob_handle);
ISC_STATUS STDCALL isc_commit_transaction (ISC_STATUS *status_vector,
                                           isc_tr_handle *trans_handle);
ISC_STATUS STDCALL isc_create_blob2 (ISC_STATUS *status_vector,
                                     isc_db_handle *db_handle,
                                     isc_tr_handle *trans_handle,
                                     isc_blob_handle *blob_handle,
                                     ISC_QUAD *blob_id,
                                     short bpb_length,
                                     char *bpb_address);
ISC_STATUS STDCALL isc_create_database (ISC_STATUS *status_vector,
                                        short db_name_length,
                                        char *db_name,
                                        isc_db_handle *db_handle,
                                        short parm_buffer_length,
                                        char *parm_buffer,
                                        short db_type);
void STDCALL isc_decode_sql_date (ISC_DATE *ib_date,
                                  void *tm_date);
void STDCALL isc_decode_sql_time (ISC_TIME *ib_time,
                                  void *tm_date);
void STDCALL isc_decode_timestamp (ISC_TIMESTAMP *ib_date,
                                   void *tm_date);
ISC_STATUS STDCALL isc_detach_database (ISC_STATUS *status_vector,
                                        isc_db_handle *db_handle);
ISC_STATUS STDCALL isc_dsql_allocate_statement (ISC_STATUS *status_vector,
                                               isc_db_handle *db_handle,
                                               isc_stmt_handle *stmt_handle);
ISC_STATUS STDCALL isc_dsql_describe (ISC_STATUS *status_vector,
                                      isc_stmt_handle *stmt_handle,
                                      unsigned short da_version,
                                      XSQLDA *xsqlda);
ISC_STATUS STDCALL isc_dsql_describe_bind (ISC_STATUS *status_vector,
                                           isc_stmt_handle *stmt_handle,
                                           unsigned short da_version,
                                           XSQLDA *xsqlda);
ISC_STATUS STDCALL isc_dsql_execute2 (ISC_STATUS *status_vector,
                                      isc_tr_handle *trans_handle,
                                      isc_stmt_handle *stmt_handle,
                                      unsigned short da_version,
                                      XSQLDA *in_xsqlda,
                                      XSQLDA *out_xsqlda);
ISC_STATUS STDCALL isc_dsql_fetch (ISC_STATUS *status_vector,
                                   isc_stmt_handle *stmt_handle,
                                   unsigned short da_version,
                                   XSQLDA *xsqlda);
ISC_STATUS STDCALL isc_dsql_free_statement (ISC_STATUS *status_vector,
                                            isc_stmt_handle *stmt_handle,
                                            unsigned short option);
ISC_STATUS STDCALL isc_dsql_prepare (ISC_STATUS *status_vector,
                                     isc_tr_handle *trans_handle,
                                     isc_stmt_handle *stmt_handle,
                                     unsigned short length,
                                     char *statement,
                                     unsigned short dialect,
                                     XSQLDA *xsqlda);
ISC_STATUS STDCALL isc_dsql_sql_info (ISC_STATUS *status_vector,
                                      isc_stmt_handle *stmt_handle,
                                      unsigned short item_length,
                                      char *items,
                                      unsigned short buffer_length,
                                      char *buffer);
void STDCALL isc_encode_sql_date (void *tm_date,
                                  ISC_DATE *ib_date);
void STDCALL isc_encode_sql_time (void *tm_date,
                                  ISC_TIME *ib_time);
void STDCALL isc_encode_timestamp (void *tm_date,
                                   ISC_TIMESTAMP *ib_timestamp);
ISC_STATUS STDCALL isc_get_segment (ISC_STATUS *status_vector,
                                    isc_blob_handle *blob_handle,
                                    unsigned short *actual_seg_length,
                                    unsigned short seg_buffer_length,
                                    char *seg_buffer);
ISC_STATUS STDCALL isc_interprete (char *buffer, ISC_STATUS **status_vector);
ISC_STATUS STDCALL isc_open_blob2 (ISC_STATUS *status_vector,
                                   isc_db_handle *db_handle,
                                   isc_tr_handle *trans_handle,
                                   isc_blob_handle *blob_handle,
                                   ISC_QUAD *blob_id,
                                   short bpb_length,
                                   char *bpb_address);
ISC_INT64 STDCALL isc_portable_integer (char *buffer,
                                        short length);
ISC_STATUS STDCALL isc_print_status (ISC_STATUS *status_vector);
ISC_STATUS STDCALL isc_put_segment (ISC_STATUS *status_vector,
                                    isc_blob_handle *blob_handle,
                                    unsigned short seg_buffer_length,
                                    char *seg_buffer);
ISC_STATUS STDCALL isc_rollback_transaction (ISC_STATUS *status_vector,
                                             isc_tr_handle *trans_handle);
ISC_STATUS STDCALL isc_start_transaction (ISC_STATUS *status_vector,
                                          isc_tr_handle *trans_handle,
                                          short db_handle_count,
                                          isc_db_handle *db_handle,
                                          unsigned short tpb_length,
                                          char *tpb_address);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
