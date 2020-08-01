/********************************************************************/
/*                                                                  */
/*  common.h      Basic type definitions and settings.              */
/*  Copyright (C) 1989 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/common.h                                        */
/*  Changes: 1992, 1993, 1994, 2005  Thomas Mertes                  */
/*  Content: Basic type definitions and settings.                   */
/*                                                                  */
/********************************************************************/

typedef int booltype;

#ifdef FALSE
#undef FALSE
#endif
#define FALSE    ((booltype) 0)
#ifdef TRUE
#undef TRUE
#endif
#define TRUE     ((booltype) 1)

#define EXTERN          extern

#define WITH_FLOAT
#define WITH_REFERENCE
#define WITH_DRAW
#undef  WITH_STATISTIC
#define WITH_COMPILATION_INFO
#define WITH_TYPE_CHECK
#undef  WITH_ACTION_CHECK
#define WITH_PROTOCOL
#define USE_ALTERNATE_NEXT_CHARACTER
#define USE_INFILE_BUFFER
#define USE_CHUNK_ALLOCS
#undef  USE_ALTERNATE_CHUNK_ALLOCS
#define USE_VARIABLE_FORMATS
#undef  USE_MAXIMUM_MALLOC_CHECK
#define DO_HEAPSIZE_COMPUTATION
#undef  DO_HEAP_STATISTIC
#undef  DO_HEAP_LOG
#undef  DO_HEAP_CHECK
#define UTF32_STRINGS
#define WITH_STRI_CAPACITY
#undef  INTTYPE_64BIT


typedef signed char        int8type;
typedef unsigned char      uint8type;

typedef short int          int16type;
typedef unsigned short int uint16type;

typedef long int           int32type;
typedef unsigned long int  uint32type;

#ifdef INT64TYPE
typedef INT64TYPE          int64type;
typedef UINT64TYPE         uint64type;
#endif

#ifdef INTTYPE_64BIT
#ifdef INT64TYPE
typedef int64type          inttype;
typedef uint64type         uinttype;
#define MAX_INTEGER 9223372036854775807
#ifdef INT64TYPE_SUFFIX_LL
#define INTTYPE_LITERAL_SUFFIX "LL"
#else
#define INTTYPE_LITERAL_SUFFIX ""
#endif
#endif
#else
typedef int32type          inttype;
typedef uint32type         uinttype;
#define MAX_INTEGER 2147483647
#define INTTYPE_LITERAL_SUFFIX "L"
#endif

typedef float              floattype;
typedef uint32type         chartype;
typedef int32type          schartype;
typedef uinttype           bitsettype;

#ifdef UTF32_STRINGS
typedef uint32type         strelemtype;
#else
typedef unsigned char      strelemtype;
#endif

typedef uint32type         memsizetype;
typedef int8type           smallpriortype;
typedef int                prior_type;

typedef unsigned char      uchartype;
typedef char *             cstritype;
typedef unsigned char *    ustritype;
typedef uint16type *       wstritype;
typedef FILE *             filetype;
typedef int                sockettype;

typedef const char *           const_cstritype;
typedef const unsigned char *  const_ustritype;
typedef const uint16type *     const_wstritype;

#define MAX_DIV_10 ((inttype) (MAX_INTEGER / 10))
#define MAX_REM_10 ((inttype) (MAX_INTEGER % 10))

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000
#endif

#define SOURCE_POSITION(POS_NR) __FILE__, __LINE__


#ifdef ANSI_C
typedef size_t SIZE_TYPE;
#else
typedef unsigned int SIZE_TYPE;
#define const
#endif

#ifdef C_PLUS_PLUS
#define INLINE inline
#else
#define INLINE
#endif


typedef int errinfotype;

/* errinfo values: */
#define OKAY_NO_ERROR 0
#define MEMORY_ERROR  1
#define NUMERIC_ERROR 2
#define RANGE_ERROR   3
#define FILE_ERROR    4
#define CREATE_ERROR  5
#define DESTROY_ERROR 6
#define COPY_ERROR    7
#define IN_ERROR      8


#ifndef os_chdir
#define os_chdir chdir
#endif
#ifndef os_getcwd
#define os_getcwd getcwd
#endif
#ifndef os_mkdir
#define os_mkdir mkdir
#endif
#ifndef os_rmdir
#define os_rmdir rmdir
#endif
#ifndef os_opendir
#define os_opendir  opendir
#endif
#ifndef os_readdir
#define os_readdir  readdir
#endif
#ifndef os_closedir
#define os_closedir closedir
#endif
#ifndef os_DIR
#define os_DIR DIR
#endif
#ifndef os_dirent_struct
#define os_dirent_struct struct dirent
#endif
#ifndef os_fstat
#define os_fstat fstat
#endif
#ifndef os_lstat
#define os_lstat lstat
#endif
#ifndef os_stat
#define os_stat stat
#endif
#ifndef os_stat_struct
#define os_stat_struct struct stat
#endif
#ifndef os_chown
#define os_chown chown
#endif
#ifndef os_chmod
#define os_chmod chmod
#endif
#ifndef os_utime
#define os_utime utime
#endif
#ifndef os_utimbuf_struct
#define os_utimbuf_struct struct utimbuf
#endif
#ifndef os_remove
#define os_remove remove
#endif
#ifndef os_rename
#define os_rename rename
#endif


typedef struct setstruct     *settype;
typedef struct stristruct    *stritype;
typedef struct bstristruct   *bstritype;
typedef struct winstruct     *wintype;
typedef struct bigintstruct  *biginttype;

typedef const struct setstruct     *const_settype;
typedef const struct stristruct    *const_stritype;
typedef const struct bstristruct   *const_bstritype;
typedef const struct winstruct     *const_wintype;
typedef const struct bigintstruct  *const_biginttype;

typedef struct setstruct {
    inttype min_position;
    inttype max_position;
    bitsettype bitset[1];
  } setrecord;

typedef struct stristruct {
    memsizetype size;
#ifdef WITH_STRI_CAPACITY
    memsizetype capacity;
#endif
    strelemtype mem[1];
  } strirecord;

typedef struct bstristruct {
    memsizetype size;
#ifdef MMAP_ABLE_BSTRI
    uchartype *mem;
    uchartype mem1[1];
#else
    uchartype mem[1];
#endif
  } bstrirecord;

typedef struct winstruct {
    uinttype usage_count;
    /* The rest of the structure is only accessable for the driver */
  } winrecord;

typedef struct bigintstruct {
#ifdef NO_EMPTY_STRUCTS
    int dummy;
#endif
  } bigintrecord;
