/********************************************************************/
/*                                                                  */
/*  db_tds.h      TDS interfaces used by Seed7.                     */
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
/*  File: seed7/src/db_tds.h                                        */
/*  Changes: 2018 - 2020  Thomas Mertes                             */
/*  Content: TDS interfaces used by Seed7.                          */
/*                                                                  */
/********************************************************************/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef int RETCODE;

typedef unsigned char DBBOOL;
typedef char DBCHAR;
typedef unsigned char BYTE;
typedef INT16TYPE DBSMALLINT;
typedef INT32TYPE DBINT;
typedef INT64TYPE DBBIGINT;
typedef UINT16TYPE DBUSMALLINT;
typedef UINT32TYPE DBUINT;
typedef UINT64TYPE DBUBIGINT;

typedef struct dbtypeinfo {
    DBINT precision;
    DBINT scale;
} DBTYPEINFO;

#define MAXCOLNAMELEN 512

typedef struct {
    DBINT  SizeOfStruct;
    DBCHAR Name[MAXCOLNAMELEN + 2];
    DBCHAR ActualName[MAXCOLNAMELEN + 2];
    DBCHAR TableName[MAXCOLNAMELEN + 2];
    short  Type;
    DBINT  UserType;
    DBINT  MaxLength;
    BYTE   Precision;
    BYTE   Scale;
    int    VarLength;
    BYTE   Null;
    BYTE   CaseSensitive;
    BYTE   Updatable;
    int    Identity;
  } DBCOL;

typedef struct {
    DBUBIGINT  time;
    DBINT      date;
    DBSMALLINT offset;
    DBUSMALLINT time_prec:3;
    DBUSMALLINT unused_data:10;
    DBUSMALLINT has_time:1;
    DBUSMALLINT has_date:1;
    DBUSMALLINT has_offset:1;
  } DBDATETIMEALL;

typedef struct tds_dblib_loginrec LOGINREC;

typedef struct tds_dblib_dbprocess DBPROCESS;

typedef int (*EHANDLEFUNC) (DBPROCESS *dbproc, int severity, int dberr, int oserr, char *dberrstr, char *oserrstr);

typedef int (*MHANDLEFUNC) (DBPROCESS *dbproc, DBINT msgno, int msgstate, int severity, char *msgtext, char *srvname,
                            char *proc, int line);

#define NO_MORE_ROWS    (-2)
#define MORE_ROWS       (-1)
#define FAIL              0
#define SUCCEED           1
#define NO_MORE_RESULTS   2

#define INT_EXIT      0
#define INT_CONTINUE  1
#define INT_CANCEL    2
#define INT_TIMEOUT   3

#define SYBETIME   20003 /* SQL Server connection timed out. */

#define SYBVOID             31
#define SYBIMAGE            34
#define SYBTEXT             35
#define SYBVARBINARY        37
#define SYBINTN             38
#define SYBVARCHAR          39
#define SYBMSDATE           40
#define SYBMSTIME           41
#define SYBMSDATETIME2      42
#define SYBMSDATETIMEOFFSET 43
#define SYBBINARY           45
#define SYBCHAR             47
#define SYBINT1             48
#define SYBDATE             49
#define SYBBIT              50
#define SYBTIME             51
#define SYBINT2             52
#define SYBINT4             56
#define SYBDATETIME4        58
#define SYBREAL             59
#define SYBMONEY            60
#define SYBDATETIME         61
#define SYBFLT8             62
#define SYBNTEXT            99
#define SYBNVARCHAR        103
#define SYBBITN            104
#define SYBDECIMAL         106
#define SYBNUMERIC         108
#define SYBFLTN            109
#define SYBMONEYN          110
#define SYBDATETIMN        111
#define SYBMONEY4          122
#define SYBINT8            127
#define SYBBIGDATETIME     187
#define SYBBIGTIME         188

#define CHARBIND          0
#define STRINGBIND        1
#define NTBSTRINGBIND     2
#define VARYCHARBIND      3
#define VARYBINBIND       4
#define TINYBIND          6
#define SMALLBIND         7
#define INTBIND           8
#define FLT8BIND          9
#define REALBIND          10
#define DATETIMEBIND      11
#define SMALLDATETIMEBIND 12
#define MONEYBIND         13
#define SMALLMONEYBIND    14
#define BINARYBIND        15
#define BITBIND           16
#define NUMERICBIND       17
#define DECIMALBIND       18
#define SRCNUMERICBIND    19
#define SRCDECIMALBIND    20
#define DATEBIND          21
#define TIMEBIND          22
#define BIGDATETIMEBIND   23
#define BIGTIMEBIND       24
#define BIGINTBIND        30
#define DATETIME2BIND     31
#define MAXBINDTYPES      32

#define DBSETHOST     1
#define DBSETUSER     2
#define DBSETPWD      3
#define DBSETCHARSET 10

#define DBSETLHOST(x,y)    dbsetlname((x), (y), DBSETHOST)
#define DBSETLUSER(x,y)    dbsetlname((x), (y), DBSETUSER)
#define DBSETLPWD(x,y)     dbsetlname((x), (y), DBSETPWD)
#define DBSETLCHARSET(x,y) dbsetlname((x), (y), DBSETCHARSET)

#ifdef MSDBLIB
#define   dbopen(x,y) tdsdbopen((x),(y), 1)
#else
#define   dbopen(x,y) tdsdbopen((x),(y), 0)
#endif

RETCODE dbbind (DBPROCESS *dbproc,
                int        column,
                int        vartype,
                DBINT      varlen,
                BYTE      *varaddr);
void dbclose (DBPROCESS * dbproc);
DBINT dbcollen (DBPROCESS *dbproc, int column);
char *dbcolname (DBPROCESS *dbproc, int column);
int dbcoltype (DBPROCESS *dbproc, int column);
DBTYPEINFO *dbcoltypeinfo (DBPROCESS *dbproc, int column);
RETCODE dbcmd (DBPROCESS *dbproc, const char *cmdstring);
BYTE *dbdata (DBPROCESS *dbproc, int column);
DBINT dbdatlen (DBPROCESS *dbproc, int column);
EHANDLEFUNC dberrhandle (EHANDLEFUNC handler);
void dbexit (void);
RETCODE dbinit (void);
LOGINREC *dblogin (void);
void dbloginfree(LOGINREC *loginptr);
MHANDLEFUNC dbmsghandle (MHANDLEFUNC handler);
RETCODE dbnextrow (DBPROCESS *dbproc);
int dbnumcols (DBPROCESS *dbproc);
RETCODE dbnullbind (DBPROCESS *dbproc, int column, DBINT *indicator);
RETCODE dbresults (DBPROCESS *dbproc);
RETCODE dbsetlname (LOGINREC * login, const char *value, int which);
RETCODE dbsqlexec (DBPROCESS *dbproc);
RETCODE dbtablecolinfo (DBPROCESS *dbproc,
                        DBINT      column,
                        DBCOL     *pdbcol);
RETCODE dbuse (DBPROCESS * dbproc, const char *name);
DBPROCESS *tdsdbopen (LOGINREC * login, const char *server, int msdblib);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
