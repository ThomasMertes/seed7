/********************************************************************/
/*                                                                  */
/*  common.h      Basic type definitions and settings.              */
/*  Copyright (C) 1989 - 2011  Thomas Mertes                        */
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
/*  Changes: 1992, 1993, 1994, 2005, 2011  Thomas Mertes            */
/*  Content: Basic type definitions and settings.                   */
/*                                                                  */
/********************************************************************/

#ifdef USE_BIG_GMP_LIBRARY
#include "gmp.h"
#endif


#if defined(__cplusplus) || defined(c_plusplus)
#define C_PLUS_PLUS
#endif

#ifndef ANSI_C
typedef unsigned int SIZE_TYPE;
#define size_t SIZE_TYPE
#define const
#endif

#ifdef C_PLUS_PLUS
#define INLINE inline
#else
#define INLINE
#endif

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
#undef  DO_HEAP_CHECK
#undef  DO_HEAP_LOG
#define WITH_STRI_CAPACITY
#define ALLOW_STRITYPE_SLICES
#define WITH_STRI_FLIST
#define INTTYPE_SIZE 32
#define BITSETTYPE_SIZE INTTYPE_SIZE
#undef  FLOATTYPE_DOUBLE

#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#define EMULATE_ROOT_CWD
#endif


typedef signed char        int8type;
typedef unsigned char      uint8type;

typedef short int          int16type;
typedef unsigned short int uint16type;


typedef INT32TYPE          int32type;
typedef UINT32TYPE         uint32type;

#ifdef INT32TYPE_SUFFIX_L
#define INT32_SUFFIX(num) num ## L
#define INT32TYPE_LITERAL_SUFFIX "L"
#ifdef TWOS_COMPLEMENT_INTTYPE
#define INT32TYPE_MIN  ((int32type) -2147483648L)
#else
#define INT32TYPE_MIN              (-2147483647L)
#endif
#define INT32TYPE_MAX                2147483647L
#define UINT32TYPE_MAX ((uint32type) 0xFFFFFFFFL)
#else
#define INT32_SUFFIX(num) num
#define INT32TYPE_LITERAL_SUFFIX ""
#ifdef TWOS_COMPLEMENT_INTTYPE
#define INT32TYPE_MIN  ((int32type) -2147483648)
#else
#define INT32TYPE_MIN              (-2147483647)
#endif
#define INT32TYPE_MAX                2147483647
#define UINT32TYPE_MAX ((uint32type) 0xFFFFFFFF)
#endif

#if   defined INT32TYPE_FORMAT_L
#define INT32TYPE_FORMAT "%ld"
#else
#define INT32TYPE_FORMAT "%d"
#endif


#ifdef INT64TYPE
typedef INT64TYPE          int64type;
typedef UINT64TYPE         uint64type;

#if   defined INT64TYPE_SUFFIX_LL
#define INT64_SUFFIX(num) num ## LL
#define INT64TYPE_LITERAL_SUFFIX "LL"
#define INT64TYPE_MAX                 9223372036854775807LL
#define UINT64TYPE_MAX ((uint64type) 18446744073709551615LL)
#elif defined INT64TYPE_SUFFIX_L
#define INT64_SUFFIX(num) num ## L
#define INT64TYPE_LITERAL_SUFFIX "L"
#define INT64TYPE_MAX                 9223372036854775807L
#define UINT64TYPE_MAX ((uint64type) 18446744073709551615L)
#else
#define INT64_SUFFIX(num) num
#define INT64TYPE_LITERAL_SUFFIX ""
#define INT64TYPE_MAX                 9223372036854775807
#define UINT64TYPE_MAX ((uint64type) 18446744073709551615)
#endif

#if   defined INT64TYPE_FORMAT_L
#define INT64TYPE_FORMAT "%ld"
#elif defined INT64TYPE_FORMAT_LL
#define INT64TYPE_FORMAT "%lld"
#elif defined INT64TYPE_FORMAT_CAPITAL_L
#define INT64TYPE_FORMAT "%Ld"
#elif defined INT64TYPE_FORMAT_I64
#define INT64TYPE_FORMAT "%I64d"
#else
#define INT64TYPE_FORMAT "%d"
#endif
#endif


#if   INTTYPE_SIZE == 32
typedef int32type               inttype;
typedef uint32type              uinttype;
#ifdef ANSI_C
#define INT_SUFFIX(num)         INT32_SUFFIX(num)
#else
#define INT_SUFFIX(num)         num
#endif
#define INTTYPE_LITERAL_SUFFIX  INT32TYPE_LITERAL_SUFFIX
#define INTTYPE_MAX             INT32TYPE_MAX
#define UINTTYPE_MAX            UINT32TYPE_MAX
#define INTTYPE_FORMAT          INT32TYPE_FORMAT
#define uintMostSignificantBit  uint32MostSignificantBit
#define uintLeastSignificantBit uint32LeastSignificantBit
#elif INTTYPE_SIZE == 64
typedef int64type               inttype;
typedef uint64type              uinttype;
#ifdef ANSI_C
#define INT_SUFFIX(num)         INT64_SUFFIX(num)
#else
#define INT_SUFFIX(num)         num
#endif
#define INTTYPE_LITERAL_SUFFIX  INT64TYPE_LITERAL_SUFFIX
#define INTTYPE_MAX             INT64TYPE_MAX
#define UINTTYPE_MAX            UINT64TYPE_MAX
#define INTTYPE_FORMAT          INT64TYPE_FORMAT
#define uintMostSignificantBit  uint64MostSignificantBit
#define uintLeastSignificantBit uint64LeastSignificantBit
#endif


#if   BITSETTYPE_SIZE == 32
typedef uint32type         bitsettype;
#elif BITSETTYPE_SIZE == 64
typedef uint64type         bitsettype;
#endif


#ifdef FLOATTYPE_DOUBLE
typedef double             floattype;
#define FLOATTYPE_SIZE DOUBLE_SIZE
#else
typedef float              floattype;
#define FLOATTYPE_SIZE FLOAT_SIZE
#endif


typedef uint32type         chartype;
typedef int32type          schartype;
typedef uint32type         strelemtype;

#if POINTER_SIZE == 32
typedef uint32type         memsizetype;
#define MAX_MEMSIZETYPE    0xFFFFFFFF
#define MAX_MEM_INDEX      INT32TYPE_MAX
#elif POINTER_SIZE == 64
typedef uint64type         memsizetype;
#define MAX_MEMSIZETYPE    0xFFFFFFFFFFFFFFFF
#define MAX_MEM_INDEX      INTTYPE_MAX
#endif

typedef int8type           smallpriortype;
typedef int                prior_type;

typedef unsigned char      uchartype;
typedef char *             cstritype;
typedef unsigned char *    ustritype;
typedef memsizetype        sysizetype;
typedef FILE *             filetype;

#ifdef OS_STRI_WCHAR
typedef wchar_t           *wstritype;
typedef const wchar_t     *const_wstritype;
#endif

#ifdef USE_WINSOCK
typedef unsigned int       sockettype;
#else
typedef int                sockettype;
#endif

typedef const char *           const_cstritype;
typedef const unsigned char *  const_ustritype;

#define MAX_DIV_10 ((inttype) (INTTYPE_MAX / 10))
#define MAX_REM_10 ((inttype) (INTTYPE_MAX % 10))

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000
#endif

#define SOURCE_POSITION(POS_NR) __FILE__, __LINE__


typedef int errinfotype;

/* errinfo values: */
#define OKAY_NO_ERROR 0
#define MEMORY_ERROR  1
#define NUMERIC_ERROR 2
#define RANGE_ERROR   3
#define FILE_ERROR    4
#define ACTION_ERROR  5
#define CREATE_ERROR  6
#define DESTROY_ERROR 7
#define COPY_ERROR    8
#define IN_ERROR      9


#ifndef likely
#define likely(x) (x)
#endif
#ifndef unlikely
#define unlikely(x) (x)
#endif


typedef struct setstruct     *settype;
typedef struct stristruct    *stritype;
typedef struct bstristruct   *bstritype;
typedef struct pollstruct    *polltype;
typedef struct winstruct     *wintype;

typedef const struct setstruct     *const_settype;
typedef const struct stristruct    *const_stritype;
typedef const struct bstristruct   *const_bstritype;
typedef const struct pollstruct    *const_polltype;
typedef const struct winstruct     *const_wintype;

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
#ifdef ALLOW_STRITYPE_SLICES
    strelemtype *mem;
    strelemtype  mem1[1];
#else
    strelemtype mem[1];
#endif
  } strirecord;

typedef struct bstristruct {
    memsizetype size;
#ifdef MMAP_ABLE_BSTRI
    uchartype *mem;
    uchartype  mem1[1];
#else
    uchartype mem[1];
#endif
  } bstrirecord;

typedef struct pollstruct {
#ifdef NO_EMPTY_STRUCTS
    int dummy;
#endif
  } pollrecord;

typedef struct winstruct {
    uinttype usage_count;
    /* The rest of the structure is only accessable for the driver */
  } winrecord;

typedef struct fliststruct {
    struct fliststruct *next;
  } *flisttype;


#ifdef USE_BIG_RTL_LIBRARY

/***************************************/
/*                                     */
/*   Define biginttype for big_rtl.c   */
/*                                     */
/***************************************/

typedef       struct bigintstruct  *      biginttype;
typedef const struct bigintstruct  *const_biginttype;

#define WITH_BIGINT_CAPACITY

#ifdef INT64TYPE
#define BIGDIGIT_SIZE 32
#else
#define BIGDIGIT_SIZE 16
#endif

#if BIGDIGIT_SIZE == 8
  typedef uint8type         bigdigittype;
#elif BIGDIGIT_SIZE == 16
  typedef uint16type        bigdigittype;
#elif BIGDIGIT_SIZE == 32
  typedef uint32type        bigdigittype;
#endif

typedef struct bigintstruct {
    memsizetype size;
#ifdef WITH_BIGINT_CAPACITY
    memsizetype capacity;
#endif
    bigdigittype bigdigits[1];
  } bigintrecord;


#else
#ifdef USE_BIG_GMP_LIBRARY

/***************************************/
/*                                     */
/*   Define biginttype for big_gmp.c   */
/*                                     */
/***************************************/

typedef mpz_ptr           biginttype;
typedef mpz_srcptr  const_biginttype;


#endif
#endif
