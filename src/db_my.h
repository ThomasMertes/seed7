/********************************************************************/
/*                                                                  */
/*  db_my.h       MariaDB and MySQL interfaces used by Seed7.       */
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
/*  File: seed7/src/db_my.h                                         */
/*  Changes: 2014, 2020  Thomas Mertes                              */
/*  Content: MariaDB and MySQL interfaces used by Seed7.            */
/*                                                                  */
/********************************************************************/

typedef char my_bool;

typedef struct { int dummy; } MYSQL;
typedef struct { int dummy; } MYSQL_RES;
typedef struct { int dummy; } MYSQL_STMT;
typedef struct { int dummy; } NET;

enum enum_field_types {
  MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
  MYSQL_TYPE_SHORT,  MYSQL_TYPE_LONG,
  MYSQL_TYPE_FLOAT,  MYSQL_TYPE_DOUBLE,
  MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
  MYSQL_TYPE_LONGLONG,MYSQL_TYPE_INT24,
  MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
  MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
  MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
  MYSQL_TYPE_BIT,
  MYSQL_TYPE_NEWDECIMAL=246,
  MYSQL_TYPE_ENUM=247,
  MYSQL_TYPE_SET=248,
  MYSQL_TYPE_TINY_BLOB=249,
  MYSQL_TYPE_MEDIUM_BLOB=250,
  MYSQL_TYPE_LONG_BLOB=251,
  MYSQL_TYPE_BLOB=252,
  MYSQL_TYPE_VAR_STRING=253,
  MYSQL_TYPE_STRING=254,
  MYSQL_TYPE_GEOMETRY=255
};

enum mysql_option {
  MYSQL_OPT_CONNECT_TIMEOUT, MYSQL_OPT_COMPRESS, MYSQL_OPT_NAMED_PIPE,
  MYSQL_INIT_COMMAND, MYSQL_READ_DEFAULT_FILE, MYSQL_READ_DEFAULT_GROUP,
  MYSQL_SET_CHARSET_DIR, MYSQL_SET_CHARSET_NAME, MYSQL_OPT_LOCAL_INFILE,
  MYSQL_OPT_PROTOCOL, MYSQL_SHARED_MEMORY_BASE_NAME, MYSQL_OPT_READ_TIMEOUT,
  MYSQL_OPT_WRITE_TIMEOUT, MYSQL_OPT_USE_RESULT,
  MYSQL_OPT_USE_REMOTE_CONNECTION, MYSQL_OPT_USE_EMBEDDED_CONNECTION,
  MYSQL_OPT_GUESS_CONNECTION, MYSQL_SET_CLIENT_IP, MYSQL_SECURE_AUTH,
  MYSQL_REPORT_DATA_TRUNCATION, MYSQL_OPT_RECONNECT,
  MYSQL_OPT_SSL_VERIFY_SERVER_CERT, MYSQL_PLUGIN_DIR, MYSQL_DEFAULT_AUTH,
  MYSQL_ENABLE_CLEARTEXT_PLUGIN
};

enum enum_mysql_timestamp_type {
  MYSQL_TIMESTAMP_NONE= -2, MYSQL_TIMESTAMP_ERROR= -1,
  MYSQL_TIMESTAMP_DATE= 0, MYSQL_TIMESTAMP_DATETIME= 1, MYSQL_TIMESTAMP_TIME= 2
};

typedef struct {
  char *name;
  char *org_name;
  char *table;
  char *org_table;
  char *db;
  char *catalog;
  char *def;
  unsigned long length;
  unsigned long max_length;
  unsigned int name_length;
  unsigned int org_name_length;
  unsigned int table_length;
  unsigned int org_table_length;
  unsigned int db_length;
  unsigned int catalog_length;
  unsigned int def_length;
  unsigned int flags;
  unsigned int decimals;
  unsigned int charsetnr;
  enum enum_field_types type;
  void *extension;
} MYSQL_FIELD;

typedef struct st_mysql_bind {
  unsigned long *length;
  my_bool       *is_null;
  void          *buffer;
  my_bool       *error;
  unsigned char *row_ptr;
  void (*store_param_func)(NET *net, struct st_mysql_bind *param);
  void (*fetch_result)(struct st_mysql_bind *, MYSQL_FIELD *,
                       unsigned char **row);
  void (*skip_result)(struct st_mysql_bind *, MYSQL_FIELD *,
                      unsigned char **row);
  unsigned long buffer_length;
  unsigned long offset;
  unsigned long length_value;
  unsigned int  param_number;
  unsigned int  pack_length;
  enum enum_field_types buffer_type;
  my_bool error_value;
  my_bool is_unsigned;
  my_bool long_data_used;
  my_bool is_null_value;
  void *extension;
} MYSQL_BIND;

typedef struct {
  unsigned int  year, month, day, hour, minute, second;
  unsigned long second_part;
  my_bool       neg;
  enum enum_mysql_timestamp_type time_type;
} MYSQL_TIME;

#define MYSQL_NO_DATA        100
#define MYSQL_DATA_TRUNCATED 101

#ifndef STDCALL
#if defined(_WIN32) && HAS_STDCALL
#define STDCALL __stdcall
#else
#define STDCALL
#endif
#endif

my_bool STDCALL mysql_autocommit (MYSQL *mysql, my_bool mode);
void STDCALL mysql_close (MYSQL *sock);
my_bool STDCALL mysql_commit (MYSQL *mysql);
unsigned int STDCALL mysql_errno (MYSQL *mysql);
const char *STDCALL mysql_error (MYSQL *mysql);
MYSQL_FIELD *STDCALL mysql_fetch_field_direct (MYSQL_RES *res,
                                               unsigned int fieldnr);
void STDCALL mysql_free_result (MYSQL_RES *result);
MYSQL *STDCALL mysql_init (MYSQL *mysql);
unsigned int STDCALL mysql_num_fields (MYSQL_RES *res);
int STDCALL mysql_options (MYSQL *mysql,
                           enum mysql_option option,
                           const void *arg);
MYSQL *STDCALL mysql_real_connect (MYSQL *mysql,
                                   const char *host,
                                   const char *user,
                                   const char *passwd,
                                   const char *db,
                                   unsigned int port,
                                   const char *unix_socket,
                                   unsigned long clientflag);
my_bool STDCALL mysql_rollback (MYSQL *mysql);
int STDCALL mysql_set_character_set (MYSQL *mysql,
                                     const char *csname);
my_bool STDCALL mysql_stmt_bind_param (MYSQL_STMT *stmt,
                                       MYSQL_BIND *bnd);
my_bool STDCALL mysql_stmt_bind_result (MYSQL_STMT *stmt,
                                        MYSQL_BIND *bnd);
my_bool STDCALL mysql_stmt_close (MYSQL_STMT *stmt);
unsigned int STDCALL mysql_stmt_errno (MYSQL_STMT *stmt);
const char *STDCALL mysql_stmt_error (MYSQL_STMT *stmt);
int STDCALL mysql_stmt_execute (MYSQL_STMT *stmt);
int STDCALL mysql_stmt_fetch (MYSQL_STMT *stmt);
int STDCALL mysql_stmt_fetch_column (MYSQL_STMT *stmt,
                                     MYSQL_BIND *bnd,
                                     unsigned int column,
                                     unsigned long offset);
MYSQL_STMT *STDCALL mysql_stmt_init (MYSQL *mysql);
unsigned long STDCALL mysql_stmt_param_count (MYSQL_STMT *stmt);
int STDCALL mysql_stmt_prepare (MYSQL_STMT *stmt,
                                const char *query,
                                unsigned long length);
MYSQL_RES *STDCALL mysql_stmt_result_metadata (MYSQL_STMT *stmt);
int STDCALL mysql_stmt_store_result (MYSQL_STMT *stmt);
