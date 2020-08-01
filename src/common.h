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

typedef int boolType;

#ifdef FALSE
#undef FALSE
#endif
#define FALSE    ((boolType) 0)
#ifdef TRUE
#undef TRUE
#endif
#define TRUE     ((boolType) 1)

#define EXTERN          extern

typedef signed char        int8Type;
typedef unsigned char      uint8Type;

#define INT8TYPE_MAX        127
#define INT8TYPE_MIN      (-128)
#define UINT8TYPE_MAX      0xff
#define F_D8(width) "%" #width "hhd"
#define F_U8(width) "%" #width "hhu"
#define F_X8(width) "%" #width "hhx"
#define FMT_D8  "%hhd"
#define FMT_U8  "%hhu"
#define FMT_X8  "%hhx"


typedef short int          int16Type;
typedef unsigned short int uint16Type;

#define INT16TYPE_MAX     32767
#define INT16TYPE_MIN   (-32768)
#define UINT16TYPE_MAX   0xffff
#define F_D16(width) "%" #width "hd"
#define F_U16(width) "%" #width "hu"
#define F_X16(width) "%" #width "hx"
#define FMT_D16  "%hd"
#define FMT_U16  "%hu"
#define FMT_X16  "%hx"


typedef INT32TYPE          int32Type;
typedef UINT32TYPE         uint32Type;

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
#define UINT32TYPE_MAX ((uint32Type) 0xffffffffL)
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
#define UINT32TYPE_MAX ((uint32Type) 0xffffffff)
#endif

#if   defined INT32TYPE_FORMAT_L
#define F_D32(width) "%" #width "ld"
#define F_U32(width) "%" #width "lu"
#define F_X32(width) "%" #width "lx"
#define FMT_D32  "%ld"
#define FMT_U32  "%lu"
#define FMT_X32  "%lx"
#else
#define F_D32(width) "%" #width "d"
#define F_U32(width) "%" #width "u"
#define F_X32(width) "%" #width "x"
#define FMT_D32  "%d"
#define FMT_U32  "%u"
#define FMT_X32  "%x"
#endif


#ifdef INT64TYPE
typedef INT64TYPE          int64Type;
typedef UINT64TYPE         uint64Type;

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
#define UINT64TYPE_MAX ((uint64Type)   0xffffffffffffffffLL)
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
#define UINT64TYPE_MAX ((uint64Type)   0xffffffffffffffffL)
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
#define UINT64TYPE_MAX ((uint64Type)   0xffffffffffffffff)
#endif

#if   defined INT64TYPE_FORMAT_L
#define F_D64(width) "%" #width "ld"
#define F_U64(width) "%" #width "lu"
#define F_X64(width) "%" #width "lx"
#define FMT_D64  "%ld"
#define FMT_U64  "%lu"
#define FMT_X64  "%lx"
#elif defined INT64TYPE_FORMAT_LL
#define F_D64(width) "%" #width "lld"
#define F_U64(width) "%" #width "llu"
#define F_X64(width) "%" #width "llx"
#define FMT_D64  "%lld"
#define FMT_U64  "%llu"
#define FMT_X64  "%llx"
#elif defined INT64TYPE_FORMAT_CAPITAL_L
#define F_D64(width) "%" #width "Ld"
#define F_U64(width) "%" #width "Lu"
#define F_X64(width) "%" #width "Lx"
#define FMT_D64  "%Ld"
#define FMT_U64  "%Lu"
#define FMT_X64  "%Lx"
#elif defined INT64TYPE_FORMAT_I64
#define F_D64(width) "%" #width "I64d"
#define F_U64(width) "%" #width "I64u"
#define F_X64(width) "%" #width "I64x"
#define FMT_D64  "%I64d"
#define FMT_U64  "%I64u"
#define FMT_X64  "%I64x"
#else
#define F_D64(width) "%" #width "d"
#define F_U64(width) "%" #width "u"
#define F_X64(width) "%" #width "x"
#define FMT_D64  "%d"
#define FMT_U64  "%u"
#define FMT_X64  "%x"
#endif
#endif


#if   INTTYPE_SIZE == 32
typedef int32Type                 intType;
typedef uint32Type                uintType;
#define INT_SUFFIX(num)           INT32_SUFFIX(num)
#define UINT_SUFFIX(num)          UINT32_SUFFIX(num)
#define INTTYPE_LITERAL_SUFFIX    INT32TYPE_LITERAL_SUFFIX
#define INTTYPE_MIN               INT32TYPE_MIN
#define INTTYPE_MAX               INT32TYPE_MAX
#define UINTTYPE_MAX              UINT32TYPE_MAX
#define F_D(width)                F_D32(width)
#define F_U(width)                F_U32(width)
#define F_X(width)                F_X32(width)
#define FMT_D                     FMT_D32
#define FMT_U                     FMT_U32
#define FMT_X                     FMT_X32
#define INTTYPE_DECIMAL_SIZE      11
#define DECIMAL_DIGITS_IN_INTTYPE 9
#define uintMostSignificantBit    uint32MostSignificantBit
#define uintLeastSignificantBit   uint32LeastSignificantBit
#elif INTTYPE_SIZE == 64
typedef int64Type                 intType;
typedef uint64Type                uintType;
#define INT_SUFFIX(num)           INT64_SUFFIX(num)
#define UINT_SUFFIX(num)          UINT64_SUFFIX(num)
#define INTTYPE_LITERAL_SUFFIX    INT64TYPE_LITERAL_SUFFIX
#define INTTYPE_MIN               INT64TYPE_MIN
#define INTTYPE_MAX               INT64TYPE_MAX
#define UINTTYPE_MAX              UINT64TYPE_MAX
#define F_D(width)                F_D64(width)
#define F_U(width)                F_U64(width)
#define F_X(width)                F_X64(width)
#define FMT_D                     FMT_D64
#define FMT_U                     FMT_U64
#define FMT_X                     FMT_X64
#define INTTYPE_DECIMAL_SIZE      20
#define DECIMAL_DIGITS_IN_INTTYPE 18
#define uintMostSignificantBit    uint64MostSignificantBit
#define uintLeastSignificantBit   uint64LeastSignificantBit
#endif


#if TIME_T_SIZE == 32
#define TIME_T_MAX INT32TYPE_MAX
#elif TIME_T_SIZE == 64
#define TIME_T_MAX INT64TYPE_MAX
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
typedef uint32Type         bitSetType;
#define bitsetMostSignificantBit  uint32MostSignificantBit
#define bitsetLeastSignificantBit uint32LeastSignificantBit
#elif BITSETTYPE_SIZE == 64
typedef uint64Type         bitSetType;
#define bitsetMostSignificantBit  uint64MostSignificantBit
#define bitsetLeastSignificantBit uint64LeastSignificantBit
#endif


#ifdef FLOATTYPE_DOUBLE
typedef double             floatType;
#define FLOATTYPE_SIZE DOUBLE_SIZE
#define FMT_E DOUBLE_STR_FORMAT
#else
typedef float              floatType;
#define FLOATTYPE_SIZE FLOAT_SIZE
#define FMT_E FLOAT_STR_FORMAT
#endif


typedef uint32Type         charType;
typedef int32Type          scharType;
typedef charType           strElemType;

#if POINTER_SIZE == 32
typedef uint32Type         memSizeType;
#define MAX_MEMSIZETYPE    0xFFFFFFFF
#define MIN_MEM_INDEX      INT32TYPE_MIN
#define MAX_MEM_INDEX      INT32TYPE_MAX
#define F_D_MEM(width)     F_D32(width)
#define F_U_MEM(width)     F_U32(width)
#define F_X_MEM(width)     F_X32(width)
#define FMT_D_MEM          FMT_D32
#define FMT_U_MEM          FMT_U32
#define FMT_X_MEM          FMT_X32
#elif POINTER_SIZE == 64
typedef uint64Type         memSizeType;
#define MAX_MEMSIZETYPE    0xFFFFFFFFFFFFFFFF
#define MIN_MEM_INDEX      INTTYPE_MIN
#define MAX_MEM_INDEX      INTTYPE_MAX
#define F_D_MEM(width)     F_D64(width)
#define F_U_MEM(width)     F_U64(width)
#define F_X_MEM(width)     F_X64(width)
#define FMT_D_MEM          FMT_D64
#define FMT_U_MEM          FMT_U64
#define FMT_X_MEM          FMT_X64
#endif

typedef int                priorityType;

typedef unsigned char      ucharType;
typedef char *             cstriType;
typedef unsigned char *    ustriType;
typedef memSizeType        sySizeType;
typedef FILE *             fileType;
typedef int                fileDesType;

typedef uint16Type         wcharType;
typedef wcharType         *wstriType;
typedef const wcharType   *const_wstriType;

typedef int                socketType;
typedef unsigned int       usocketType;

#ifdef USE_WINSOCK
typedef unsigned int       os_socketType;
#else
typedef int                os_socketType;
#endif

typedef const char *           const_cstriType;
typedef const unsigned char *  const_ustriType;

#define MAX_DIV_10 ((intType) (INTTYPE_MAX / 10))
#define MAX_REM_10 ((intType) (INTTYPE_MAX % 10))

/* Maximum value of type sySizeType: */
#define MAX_SYMB_LENGTH MAX_MEMSIZETYPE

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000000
#endif

#define SOURCE_POSITION(POS_NR) __FILE__, __LINE__


typedef int errInfoType;

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


/* The macros below compute the array size as memSizeType    */
/* value. The computation avoids a signed integer overflow.  */
/* The computation fails when max_position is the maximum    */
/* positive value of intType and min_position is the maximum */
/* negative value of intType. All calls of the macros must   */
/* assure that they are never called with this values. Since */
/* the calls either use data from an existing array or use   */
/* 1 as min_position this condition is fulfilled.            */
#define arraySize(arr) (memSizeType) ((uintType) (arr)->max_position - (uintType) (arr)->min_position + 1)
#define arraySize2(min_position,max_position) (memSizeType) ((uintType) (max_position) - (uintType) (min_position) + 1)
#define arrayIndex(arr,pos) (memSizeType) ((uintType) (pos) - (uintType) (arr)->min_position)
#define arrayMaxPos(min_position,size) (intType) ((uintType) (min_position) + (uintType) (size) - 1)

#define bitsetSize(set) (memSizeType) ((uintType) (set)->max_position - (uintType) (set)->min_position + 1)
#define bitsetSize2(min_position,max_position) (memSizeType) ((uintType) (max_position) - (uintType) (min_position) + 1)
#define bitsetIndex(set,pos) (memSizeType) ((uintType) (pos) - (uintType) (set)->min_position)


typedef struct setStruct      *setType;
typedef struct striStruct     *striType;
typedef struct bstriStruct    *bstriType;
typedef struct pollStruct     *pollType;
typedef struct winStruct      *winType;
typedef struct databaseStruct *databaseType;
typedef struct sqlStmtStruct  *sqlStmtType;

typedef const struct setStruct      *const_setType;
typedef const struct striStruct     *const_striType;
typedef const struct bstriStruct    *const_bstriType;
typedef const struct pollStruct     *const_pollType;
typedef const struct winStruct      *const_winType;
typedef const struct databaseStruct *const_databaseType;
typedef const struct sqlStmtStruct  *const_sqlStmtType;

typedef struct setStruct {
    intType min_position;
    intType max_position;
    bitSetType bitset[1];
  } setRecord;

typedef struct striStruct {
    memSizeType size;
#ifdef WITH_STRI_CAPACITY
    memSizeType capacity;
#endif
#ifdef ALLOW_STRITYPE_SLICES
    strElemType *mem;
    strElemType  mem1[1];
#else
    strElemType mem[1];
#endif
  } striRecord;

typedef struct bstriStruct {
    memSizeType size;
#ifdef ALLOW_BSTRITYPE_SLICES
    ucharType *mem;
    ucharType  mem1[1];
#else
    ucharType mem[1];
#endif
  } bstriRecord;

typedef struct pollStruct {
#ifdef NO_EMPTY_STRUCTS
    int dummy;
#endif
  } pollRecord;

typedef struct winStruct {
    uintType usage_count;
    /* The rest of the structure is only accessable for the driver */
  } winRecord;

typedef struct databaseStruct {
    uintType usage_count;
    /* The rest of the structure is only accessable for the driver */
  } databaseRecord;

typedef struct sqlStmtStruct {
    uintType usage_count;
    /* The rest of the structure is only accessable for the driver */
  } sqlStmtRecord;

typedef struct freeListElemStruct {
    struct freeListElemStruct *next;
  } *freeListElemType;


#ifdef USE_BIG_RTL_LIBRARY

/***************************************/
/*                                     */
/*   Define bigIntType for big_rtl.c   */
/*                                     */
/***************************************/

#define WITH_BIGINT_CAPACITY

#ifdef INT64TYPE
#define BIGDIGIT_SIZE 32
#else
#define BIGDIGIT_SIZE 16
#endif

#if BIGDIGIT_SIZE == 8
  typedef uint8Type         bigDigitType;
#elif BIGDIGIT_SIZE == 16
  typedef uint16Type        bigDigitType;
#elif BIGDIGIT_SIZE == 32
  typedef uint32Type        bigDigitType;
#endif

typedef struct {
    memSizeType size;
#ifdef WITH_BIGINT_CAPACITY
    memSizeType capacity;
#endif
    bigDigitType bigdigits[1];
  } bigIntRecord;

typedef bigIntRecord             *bigIntType;
typedef const bigIntRecord *const_bigIntType;


#else
#ifdef USE_BIG_GMP_LIBRARY

/***************************************/
/*                                     */
/*   Define bigIntType for big_gmp.c   */
/*                                     */
/***************************************/

typedef mpz_ptr           bigIntType;
typedef mpz_srcptr  const_bigIntType;


#endif
#endif
