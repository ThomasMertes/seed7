/********************************************************************/
/*                                                                  */
/*  common.h      Basic type definitions and settings.              */
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
/*  File: seed7/src/common.h                                        */
/*  Changes: 1992 - 1994, 2005, 2011, 2013, 2014  Thomas Mertes     */
/*  Content: Basic type definitions and settings.                   */
/*                                                                  */
/********************************************************************/

#include "config.h"

#ifdef USE_BIG_GMP_LIBRARY
#include "gmp.h"
#endif


#if defined(__cplusplus) || defined(c_plusplus)
#define C_PLUS_PLUS
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

typedef signed char        int8type;
typedef unsigned char      uint8type;

#define INT8TYPE_MAX        127
#define INT8TYPE_MIN      (-128)
#define UINT8TYPE_MAX      0xff


typedef short int          int16type;
typedef unsigned short int uint16type;

#define INT16TYPE_MAX     32767
#define INT16TYPE_MIN   (-32768)
#define UINT16TYPE_MAX   0xffff


typedef INT32TYPE          int32type;
typedef UINT32TYPE         uint32type;

#ifdef INT32TYPE_SUFFIX_L
#define INT32_SUFFIX(num)  num ## L
#define UINT32_SUFFIX(num) num ## UL
#define INT32TYPE_LITERAL_SUFFIX "L"
#define INT32TYPE_MAX                2147483647L
#ifdef TWOS_COMPLEMENT_INTTYPE
#define INT32TYPE_MIN       (-INT32TYPE_MAX - 1L)
#else
#define INT32TYPE_MIN              (-2147483647L)
#endif
#define UINT32TYPE_MAX ((uint32type) 0xffffffffL)
#else
#define INT32_SUFFIX(num)  num
#define UINT32_SUFFIX(num) num ## U
#define INT32TYPE_LITERAL_SUFFIX ""
#define INT32TYPE_MAX                2147483647
#ifdef TWOS_COMPLEMENT_INTTYPE
#define INT32TYPE_MIN       (-INT32TYPE_MAX - 1)
#else
#define INT32TYPE_MIN              (-2147483647)
#endif
#define UINT32TYPE_MAX ((uint32type) 0xffffffff)
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
#define INT64_SUFFIX(num)  num ## LL
#define UINT64_SUFFIX(num) num ## ULL
#define INT64TYPE_LITERAL_SUFFIX "LL"
#define INT64TYPE_MAX                 9223372036854775807LL
#ifdef TWOS_COMPLEMENT_INTTYPE
#define INT64TYPE_MIN                 (-INT64TYPE_MAX - 1LL)
#else
#define INT64TYPE_MIN               (-9223372036854775807LL)
#endif
#define UINT64TYPE_MAX ((uint64type)   0xffffffffffffffffLL)
#elif defined INT64TYPE_SUFFIX_L
#define INT64_SUFFIX(num)  num ## L
#define UINT64_SUFFIX(num) num ## UL
#define INT64TYPE_LITERAL_SUFFIX "L"
#define INT64TYPE_MAX                 9223372036854775807L
#ifdef TWOS_COMPLEMENT_INTTYPE
#define INT64TYPE_MIN                 (-INT64TYPE_MAX - 1L)
#else
#define INT64TYPE_MIN               (-9223372036854775807L)
#endif
#define UINT64TYPE_MAX ((uint64type)   0xffffffffffffffffL)
#else
#define INT64_SUFFIX(num)  num
#define UINT64_SUFFIX(num) num ## U
#define INT64TYPE_LITERAL_SUFFIX ""
#define INT64TYPE_MAX                 9223372036854775807
#ifdef TWOS_COMPLEMENT_INTTYPE
#define INT64TYPE_MIN                 (-INT64TYPE_MAX - 1)
#else
#define INT64TYPE_MIN               (-9223372036854775807)
#endif
#define UINT64TYPE_MAX ((uint64type)   0xffffffffffffffff)
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
#define INT_SUFFIX(num)         INT32_SUFFIX(num)
#define UINT_SUFFIX(num)        UINT32_SUFFIX(num)
#define INTTYPE_LITERAL_SUFFIX  INT32TYPE_LITERAL_SUFFIX
#define INTTYPE_MIN             INT32TYPE_MIN
#define INTTYPE_MAX             INT32TYPE_MAX
#define UINTTYPE_MAX            UINT32TYPE_MAX
#define INTTYPE_FORMAT          INT32TYPE_FORMAT
#define INTTYPE_DECIMAL_DIGITS  10
#define uintMostSignificantBit  uint32MostSignificantBit
#define uintLeastSignificantBit uint32LeastSignificantBit
#elif INTTYPE_SIZE == 64
typedef int64type               inttype;
typedef uint64type              uinttype;
#define INT_SUFFIX(num)         INT64_SUFFIX(num)
#define UINT_SUFFIX(num)        UINT64_SUFFIX(num)
#define INTTYPE_LITERAL_SUFFIX  INT64TYPE_LITERAL_SUFFIX
#define INTTYPE_MIN             INT64TYPE_MIN
#define INTTYPE_MAX             INT64TYPE_MAX
#define UINTTYPE_MAX            UINT64TYPE_MAX
#define INTTYPE_FORMAT          INT64TYPE_FORMAT
#define INTTYPE_DECIMAL_DIGITS  19
#define uintMostSignificantBit  uint64MostSignificantBit
#define uintLeastSignificantBit uint64LeastSignificantBit
#endif

#if SHORT_SIZE < INTTYPE_SIZE
#define inShortRange(num) ((num) >= SHRT_MIN && (num) <= SHRT_MAX)
#define castToShort(num)  (inShortRange(num) ? (short int) (num) : (raise_error(RANGE_ERROR), (short int) 0))
#else
#define inShortRange(num) 1
#define castToShort(num)  ((short int) (num))
#endif

#if INT_SIZE < INTTYPE_SIZE
#define inIntRange(num) ((num) >= INT_MIN && (num) <= INT_MAX)
#define castToInt(num)  (inIntRange(num) ? (int) (num) : (raise_error(RANGE_ERROR), 0))
#else
#define inIntRange(num) 1
#define castToInt(num)  ((int) (num))
#endif

#if LONG_SIZE < INTTYPE_SIZE
#define inLongRange(num) ((num) >= LONG_MIN && (num) <= LONG_MAX)
#define castToLong(num)  (inLongRange(num) ? (long) (num) : (raise_error(RANGE_ERROR), 0))
#else
#define inLongRange(num) 1
#define castToLong(num)  ((long) (num))
#endif

#ifdef TIME_T_SIGNED
#if TIME_T_SIZE < INTTYPE_SIZE
#if TIME_T_SIZE == 32
#define inTimeTRange(timestamp) ((timestamp) >= INT32TYPE_MIN && (timestamp) <= INT32TYPE_MAX)
#elif TIME_T_SIZE == 64
#define inTimeTRange(timestamp) ((timestamp) >= INT64TYPE_MIN && (timestamp) <= INT64TYPE_MAX)
#endif
#else
#define inTimeTRange(timestamp) 1
#endif
#else
#if TIME_T_SIZE < INTTYPE_SIZE
#if TIME_T_SIZE == 32
#define inTimeTRange(timestamp) ((timestamp) >= 0 && (timestamp) <= UINT32TYPE_MAX)
#elif TIME_T_SIZE == 64
#define inTimeTRange(timestamp) ((timestamp) >= 0 && (timestamp) <= UINT64TYPE_MAX)
#endif
#else
#define inTimeTRange(timestamp) ((timestamp) >= 0)
#endif
#endif


#if   BITSETTYPE_SIZE == 32
typedef uint32type         bitsettype;
#define bitsetMostSignificantBit  uint32MostSignificantBit
#define bitsetLeastSignificantBit uint32LeastSignificantBit
#elif BITSETTYPE_SIZE == 64
typedef uint64type         bitsettype;
#define bitsetMostSignificantBit  uint64MostSignificantBit
#define bitsetLeastSignificantBit uint64LeastSignificantBit
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
typedef chartype           strelemtype;

#if POINTER_SIZE == 32
typedef uint32type         memsizetype;
#define MAX_MEMSIZETYPE    0xFFFFFFFF
#define MIN_MEM_INDEX      INT32TYPE_MIN
#define MAX_MEM_INDEX      INT32TYPE_MAX
#elif POINTER_SIZE == 64
typedef uint64type         memsizetype;
#define MAX_MEMSIZETYPE    0xFFFFFFFFFFFFFFFF
#define MIN_MEM_INDEX      INTTYPE_MIN
#define MAX_MEM_INDEX      INTTYPE_MAX
#endif

typedef int                prioritytype;

typedef unsigned char      uchartype;
typedef char *             cstritype;
typedef unsigned char *    ustritype;
typedef memsizetype        sysizetype;
typedef FILE *             filetype;
typedef int                filedestype;

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

/* Maximum value of type sysizetype: */
#define MAX_SYMB_LENGTH MAX_MEMSIZETYPE

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


/* The macros below compute the array size as memsizetype    */
/* value. The computation avoids a signed integer overflow.  */
/* The computation fails when max_position is the maximum    */
/* positive value of inttype and min_position is the maximum */
/* negative value of inttype. All calls of the macros must   */
/* assure that they are never called with this values. Since */
/* the calls either use data from an existing array or use   */
/* 1 as min_position this condition is fulfilled.            */
#define arraySize(arr) (memsizetype) ((uinttype) (arr)->max_position - (uinttype) (arr)->min_position + 1)
#define arraySize2(min_position,max_position) (memsizetype) ((uinttype) (max_position) - (uinttype) (min_position) + 1)
#define arrayIndex(arr,pos) (memsizetype) ((uinttype) (pos) - (uinttype) (arr)->min_position)
#define arrayMaxPos(min_position,size) (inttype) ((uinttype) (min_position) + (uinttype) (size) - 1)

#define bitsetSize(set) (memsizetype) ((uinttype) (set)->max_position - (uinttype) (set)->min_position + 1)
#define bitsetSize2(min_position,max_position) (memsizetype) ((uinttype) (max_position) - (uinttype) (min_position) + 1)
#define bitsetIndex(set,pos) (memsizetype) ((uinttype) (pos) - (uinttype) (set)->min_position)


typedef struct setstruct      *settype;
typedef struct stristruct     *stritype;
typedef struct bstristruct    *bstritype;
typedef struct pollstruct     *polltype;
typedef struct winstruct      *wintype;
typedef struct databasestruct *databasetype;
typedef struct sqlstmtstruct  *sqlstmttype;

typedef const struct setstruct      *const_settype;
typedef const struct stristruct     *const_stritype;
typedef const struct bstristruct    *const_bstritype;
typedef const struct pollstruct     *const_polltype;
typedef const struct winstruct      *const_wintype;
typedef const struct databasestruct *const_databasetype;
typedef const struct sqlstmtstruct  *const_sqlstmttype;

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
#ifdef ALLOW_BSTRITYPE_SLICES
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

typedef struct databasestruct {
#ifdef NO_EMPTY_STRUCTS
    int dummy;
#endif
  } databaserecord;

typedef struct sqlstmtstruct {
#ifdef NO_EMPTY_STRUCTS
    int dummy;
#endif
  } sqlstmtrecord;

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
