/********************************************************************/
/*                                                                  */
/*  common.h      Basic type definitions and settings.              */
/*  Copyright (C) 1989 - 2017  Thomas Mertes                        */
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

#if BIGINT_LIBRARY == BIG_GMP_LIBRARY
#include "gmp.h"
#endif


#if defined(__cplusplus) || defined(c_plusplus)
#define C_PLUS_PLUS
#endif

typedef BOOLTYPE boolType;

#ifdef FALSE
#undef FALSE
#endif
#define FALSE    ((boolType) 0)
#ifdef TRUE
#undef TRUE
#endif
#define TRUE     ((boolType) 1)

#define EXTERN          extern


typedef INT8TYPE           int8Type;
typedef UINT8TYPE          uint8Type;

#define INT8TYPE_MAX        127
#define INT8TYPE_MIN      (-128)
#define UINT8TYPE_MAX      0xff
#define F_D8(width) "%" #width "hhd"
#define F_U8(width) "%" #width "hhu"
#define F_X8(width) "%" #width "hhx"
#define FMT_D8      "%hhd"
#define FMT_U8      "%hhu"
#define FMT_X8      "%hhx"


typedef INT16TYPE          int16Type;
typedef UINT16TYPE         uint16Type;

#define INT16TYPE_MAX     32767
#define INT16TYPE_MIN   (-32768)
#define UINT16TYPE_MAX   0xffff
#define F_D16(width) "%" #width "hd"
#define F_U16(width) "%" #width "hu"
#define F_X16(width) "%" #width "hx"
#define FMT_D16      "%hd"
#define FMT_U16      "%hu"
#define FMT_X16      "%hx"


typedef INT32TYPE          int32Type;
typedef UINT32TYPE         uint32Type;

#define INT32TYPE_MAX            INT32_SUFFIX(2147483647)
#if TWOS_COMPLEMENT_INTTYPE
#define INT32TYPE_MIN            (-INT32TYPE_MAX - INT32_SUFFIX(1))
#else
#define INT32TYPE_MIN            INT32_SUFFIX(-2147483647)
#endif
#define UINT32TYPE_MAX           UINT32_SUFFIX(0xffffffff)

#define F_D32(width) "%" #width INT32TYPE_FORMAT_LENGTH_MODIFIER "d"
#define F_U32(width) "%" #width INT32TYPE_FORMAT_LENGTH_MODIFIER "u"
#define F_X32(width) "%" #width INT32TYPE_FORMAT_LENGTH_MODIFIER "x"
#define FMT_D32      "%" INT32TYPE_FORMAT_LENGTH_MODIFIER "d"
#define FMT_U32      "%" INT32TYPE_FORMAT_LENGTH_MODIFIER "u"
#define FMT_X32      "%" INT32TYPE_FORMAT_LENGTH_MODIFIER "x"

/* INT32TYPE_DECIMAL_SIZE includes space for the sign */
#define INT32TYPE_DECIMAL_SIZE      11
/* UINT32TYPE_DECIMAL_SIZE does not need space for a sign */
#define UINT32TYPE_DECIMAL_SIZE     10
#define DECIMAL_DIGITS_IN_INT32TYPE  9


#ifdef INT64TYPE
typedef INT64TYPE          int64Type;
typedef UINT64TYPE         uint64Type;

#ifdef INT64TYPE_NO_SUFFIX_BUT_CAST
#define INT64TYPE_MAX   ((int64Type)  9223372036854775807)
#if TWOS_COMPLEMENT_INTTYPE
#define INT64TYPE_MIN   ((int64Type) (-INT64TYPE_MAX - 1))
#else
#define INT64TYPE_MIN   ((int64Type) -9223372036854775807)
#endif
#define UINT64TYPE_MAX  ((uint64Type)  0xffffffffffffffff)
#else
#define INT64TYPE_MAX            INT64_SUFFIX(9223372036854775807)
#if TWOS_COMPLEMENT_INTTYPE
#define INT64TYPE_MIN            (-INT64TYPE_MAX - INT64_SUFFIX(1))
#else
#define INT64TYPE_MIN            INT64_SUFFIX(-9223372036854775807)
#endif
#define UINT64TYPE_MAX           UINT64_SUFFIX(0xffffffffffffffff)
#endif

#define F_D64(width) "%" #width INT64TYPE_FORMAT_LENGTH_MODIFIER "d"
#define F_U64(width) "%" #width INT64TYPE_FORMAT_LENGTH_MODIFIER "u"
#define F_X64(width) "%" #width INT64TYPE_FORMAT_LENGTH_MODIFIER "x"
#define FMT_D64      "%" INT64TYPE_FORMAT_LENGTH_MODIFIER "d"
#define FMT_U64      "%" INT64TYPE_FORMAT_LENGTH_MODIFIER "u"
#define FMT_X64      "%" INT64TYPE_FORMAT_LENGTH_MODIFIER "x"

/* INT64TYPE_DECIMAL_SIZE includes space for the sign */
#define INT64TYPE_DECIMAL_SIZE      20
/* UINT64TYPE_DECIMAL_SIZE does not need space for a sign */
#define UINT64TYPE_DECIMAL_SIZE     20
#define DECIMAL_DIGITS_IN_INT64TYPE 18
#endif


#ifdef INT128TYPE
typedef INT128TYPE          int128Type;
typedef UINT128TYPE         uint128Type;
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
typedef int16Type                 halfIntType;
typedef uint16Type                halfUintType;
#ifdef INT64TYPE
#define HAS_DOUBLE_INTTYPE
typedef int64Type                 doubleIntType;
typedef uint64Type                doubleUintType;
#endif
#define HALF_INTTYPE_MIN          INT16TYPE_MIN
#define HALF_INTTYPE_MAX          INT16TYPE_MAX
#define HALF_UINTTYPE_MAX         UINT16TYPE_MAX
#define F_D(width)                F_D32(width)
#define F_U(width)                F_U32(width)
#define F_X(width)                F_X32(width)
#define FMT_D                     FMT_D32
#define FMT_U                     FMT_U32
#define FMT_X                     FMT_X32
#define INTTYPE_DECIMAL_SIZE      INT32TYPE_DECIMAL_SIZE
#define DECIMAL_DIGITS_IN_INTTYPE DECIMAL_DIGITS_IN_INT32TYPE
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
typedef int32Type                 halfIntType;
typedef uint32Type                halfUintType;
#ifdef INT128TYPE
#define HAS_DOUBLE_INTTYPE
typedef int128Type                doubleIntType;
typedef uint128Type               doubleUintType;
#endif
#define HALF_INTTYPE_MIN          INT32TYPE_MIN
#define HALF_INTTYPE_MAX          INT32TYPE_MAX
#define HALF_UINTTYPE_MAX         UINT32TYPE_MAX
#define F_D(width)                F_D64(width)
#define F_U(width)                F_U64(width)
#define F_X(width)                F_X64(width)
#define FMT_D                     FMT_D64
#define FMT_U                     FMT_U64
#define FMT_X                     FMT_X64
#define INTTYPE_DECIMAL_SIZE      INT64TYPE_DECIMAL_SIZE
#define DECIMAL_DIGITS_IN_INTTYPE DECIMAL_DIGITS_IN_INT64TYPE
#define uintMostSignificantBit    uint64MostSignificantBit
#define uintLeastSignificantBit   uint64LeastSignificantBit
#endif


/* Define intAbs macro that ignores integer overflow. */
#if INTTYPE_SIZE == INT_SIZE
#define intAbs(n) abs(n)
#elif INTTYPE_SIZE == LONG_SIZE
#define intAbs(n) labs(n)
#elif defined(LONG_LONG_SIZE) && INTTYPE_SIZE == LONG_LONG_SIZE && HAS_LLABS
#define intAbs(n) llabs(n)
#elif defined(INT64_SIZE) && INTTYPE_SIZE == INT64_SIZE && HAS_ABS64
#define intAbs(n) _abs64(n)
#else
/* The unsigned value is negated to avoid a signed integer */
/* overflow if the smallest signed integer is negated.     */
#define intAbs(n) ((n) < 0 ? (intType) -(uintType) (n) : (n))
#endif


#define BYTE_MIN  INT8TYPE_MIN
#define BYTE_MAX  INT8TYPE_MAX
#define UBYTE_MAX UINT8TYPE_MAX


typedef int64Type timeStampType;
#define TIMESTAMPTYPE_MIN INT64TYPE_MIN
#define TIMESTAMPTYPE_MAX INT64TYPE_MAX
#define TIMESTAMPTYPE_SIZE 64

#if TIME_T_SIZE == 32
#if TIME_T_SIGNED
#define TIME_T_MIN          INT32TYPE_MIN
#define TIME_T_MAX          INT32TYPE_MAX
#define TIME_T_ERROR        INT32TYPE_MIN
#define FMT_T                     FMT_D32
#else
#define TIME_T_MIN                      0
#define TIME_T_MAX         UINT32TYPE_MAX
#define TIME_T_ERROR       UINT32TYPE_MAX
#define FMT_T                     FMT_U32
#endif
#elif TIME_T_SIZE == 64
#if TIME_T_SIGNED
#define TIME_T_MIN          INT64TYPE_MIN
#define TIME_T_MAX          INT64TYPE_MAX
#define TIME_T_ERROR        INT64TYPE_MIN
#define FMT_T                     FMT_D64
#else
#define TIME_T_MIN                      0
#define TIME_T_MAX         UINT64TYPE_MAX
#define TIME_T_ERROR       UINT64TYPE_MAX
#define FMT_T                     FMT_U64
#endif
#endif


#ifdef USE_SIMPLE_RANGE_CHECK
#define inHalfIntTypeRange(num) ((num) >= HALF_INTTYPE_MIN && (num) <= HALF_INTTYPE_MAX)
#else
#define inHalfIntTypeRange(num) ((intType) (halfIntType) (num) == (num))
#endif

#ifdef HAS_DOUBLE_INTTYPE
#ifdef USE_SIMPLE_RANGE_CHECK
#define inIntTypeRange(num) ((num) >= INTTYPE_MIN && (num) <= INTTYPE_MAX)
#else
#define inIntTypeRange(num) ((doubleIntType) (intType) (num) == (num))
#endif
#endif


#if SHORT_SIZE < INTTYPE_SIZE
#ifdef USE_SIMPLE_RANGE_CHECK
#define inShortRange(num) ((num) >= SHRT_MIN && (num) <= SHRT_MAX)
#else
#define inShortRange(num) ((intType) (short int) (num) == (num))
#endif
#define castToShort(num)  (short int) (inShortRange(num) ? (num) : (raise_error(RANGE_ERROR), 0))
#else
#define inShortRange(num) 1
#define castToShort(num)  ((short int) (num))
#endif

#if INT_SIZE < INTTYPE_SIZE
#ifdef USE_SIMPLE_RANGE_CHECK
#define inIntRange(num) ((num) >= INT_MIN && (num) <= INT_MAX)
#else
#define inIntRange(num) ((intType) (int) (num) == (num))
#endif
#define castToInt(num)  (inIntRange(num) ? (int) (num) : (raise_error(RANGE_ERROR), 0))
#else
#define inIntRange(num) 1
#define castToInt(num)  ((int) (num))
#endif

#if LONG_SIZE < INTTYPE_SIZE
#ifdef USE_SIMPLE_RANGE_CHECK
#define inLongRange(num) ((num) >= LONG_MIN && (num) <= LONG_MAX)
#else
#define inLongRange(num) ((intType) (long int) (num) == (num))
#endif
#define castToLong(num)  (inLongRange(num) ? (long) (num) : (raise_error(RANGE_ERROR), 0))
#else
#define inLongRange(num) 1
#define castToLong(num)  ((long) (num))
#endif

#if TIME_T_SIGNED
#if TIME_T_SIZE < INTTYPE_SIZE
#define inTimeTRange(timestamp) ((timestamp) >= TIME_T_MIN && (timestamp) <= TIME_T_MAX)
#else
#define inTimeTRange(timestamp) 1
#endif
#else
#if TIME_T_SIZE < INTTYPE_SIZE
#define inTimeTRange(timestamp) ((timestamp) >= 0 && (timestamp) <= TIME_T_MAX)
#else
#define inTimeTRange(timestamp) ((timestamp) >= 0)
#endif
#endif

/**
 *  Some C compilers do not support switch statements with 64-bit values.
 *  For such compilers the macro castIntTypeForSwitch() is used to cast
 *  the switch value to an int. Note that castIntTypeForSwitch() may
 *  trigger the exception RANGE_ERROR.
 */
#if SWITCH_WORKS_FOR_INT64TYPE
#define castIntTypeForSwitch(num) num
#else
#define castIntTypeForSwitch(num) castToInt(num)
#endif


#if   BITSETTYPE_SIZE == 32
typedef uint32Type         bitSetType;
#define bitsetMostSignificantBit  uint32MostSignificantBit
#define bitsetLeastSignificantBit uint32LeastSignificantBit
#define FMT_U_SET  FMT_D32
#define FMT_D_SET  FMT_U32
#define FMT_X_SET  FMT_X32
#elif BITSETTYPE_SIZE == 64
typedef uint64Type         bitSetType;
#define bitsetMostSignificantBit  uint64MostSignificantBit
#define bitsetLeastSignificantBit uint64LeastSignificantBit
#define FMT_U_SET  FMT_D64
#define FMT_D_SET  FMT_U64
#define FMT_X_SET  FMT_X64
#endif


#define FLOAT_MANTISSA_BITS           (FLOAT_MANTISSA_SHIFT  - 1)
#define FLOAT_EXPONENT_AND_SIGN_BITS  (FLOAT_SIZE - FLOAT_MANTISSA_BITS )
#define DOUBLE_MANTISSA_BITS          (DOUBLE_MANTISSA_SHIFT - 1)
#define DOUBLE_EXPONENT_AND_SIGN_BITS (DOUBLE_SIZE - DOUBLE_MANTISSA_BITS)

#if FLOATTYPE_DOUBLE
typedef double             floatType;
#define FLOATTYPE_SIZE DOUBLE_SIZE
#if INT_RANGE_IN_DOUBLE_MAX <= INTTYPE_MAX
#define INT_RANGE_IN_FLOATTYPE_MAX INT_RANGE_IN_DOUBLE_MAX
#else
#define INT_RANGE_IN_FLOATTYPE_MAX INTTYPE_MAX
#endif
#define FLOATTYPE_TO_INT_CONVERSION_LIMIT DOUBLE_MANTISSA_FACTOR
#define FLOATTYPE_MANTISSA_FACTOR         DOUBLE_MANTISSA_FACTOR
#define FLOATTYPE_MANTISSA_BITS           DOUBLE_MANTISSA_BITS
#define FLOATTYPE_EXPONENT_AND_SIGN_BITS  DOUBLE_EXPONENT_AND_SIGN_BITS
#define FLOATTYPE_EXPONENT_OFFSET         DOUBLE_EXPONENT_OFFSET
#define FMT_E FMT_E_DBL
#else
typedef float              floatType;
#define FLOATTYPE_SIZE FLOAT_SIZE
#if INT_RANGE_IN_FLOAT_MAX <= INTTYPE_MAX
#define INT_RANGE_IN_FLOATTYPE_MAX INT_RANGE_IN_FLOAT_MAX
#else
#define INT_RANGE_IN_FLOATTYPE_MAX INTTYPE_MAX
#endif
#define FLOATTYPE_TO_INT_CONVERSION_LIMIT FLOAT_MANTISSA_FACTOR
#define FLOATTYPE_MANTISSA_FACTOR         FLOAT_MANTISSA_FACTOR
#define FLOATTYPE_MANTISSA_BITS           FLOAT_MANTISSA_BITS
#define FLOATTYPE_EXPONENT_AND_SIGN_BITS  FLOAT_EXPONENT_AND_SIGN_BITS
#define FLOATTYPE_EXPONENT_OFFSET         FLOAT_EXPONENT_OFFSET
#define FMT_E FMT_E_FLT
#endif

#define FLOAT_COMPARISON_OKAY (FLOAT_NAN_COMPARISON_OKAY && FLOAT_ZERO_COMPARISON_OKAY)
#define POW_FUNCTION_OKAY     (POW_OF_NAN_OKAY && POW_OF_ZERO_OKAY && POW_OF_NEGATIVE_OKAY && POW_OF_ONE_OKAY && POW_EXP_NAN_OKAY && POW_EXP_MINUS_INFINITY_OKAY)
#define SQRT_FUNCTION_OKAY    (SQRT_OF_NAN_OKAY && SQRT_OF_NEGATIVE_OKAY)
#define EXP_FUNCTION_OKAY     (EXP_OF_NAN_OKAY)
#define LOG_FUNCTION_OKAY     (LOG_OF_NAN_OKAY && LOG_OF_ZERO_OKAY && LOG_OF_NEGATIVE_OKAY)
#define LOG10_FUNCTION_OKAY   (LOG10_OF_NAN_OKAY && LOG10_OF_ZERO_OKAY && LOG10_OF_NEGATIVE_OKAY)
#define LOG1P_FUNCTION_OKAY   (HAS_LOG1P && LOG_FUNCTION_OKAY)
#define LOG2_FUNCTION_OKAY    (HAS_LOG2 && LOG2_OF_NAN_OKAY && LOG2_OF_ZERO_OKAY && LOG2_OF_NEGATIVE_OKAY)
#define LDEXP_FUNCTION_OKAY   (LDEXP_OF_NAN_OKAY && LDEXP_OF_INFINITY_OKAY)
#define FREXP_FUNCTION_OKAY   (FREXP_INFINITY_NAN_OKAY && FREXP_SUBNORMAL_OKAY)
#define FMOD_FUNCTION_OKAY    (FMOD_DIVIDEND_NAN_OKAY && FMOD_DIVISOR_NAN_OKAY && FMOD_DIVIDEND_INFINITY_OKAY && FMOD_DIVISOR_INFINITY_OKAY && FMOD_DIVISOR_ZERO_OKAY)


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
#define MEMSIZETYPE_DECIMAL_SIZE  UINT32TYPE_DECIMAL_SIZE
#define memSizeMostSignificantBit  uint32MostSignificantBit
#define memSizeLeastSignificantBit uint32LeastSignificantBit
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
#define MEMSIZETYPE_DECIMAL_SIZE  UINT64TYPE_DECIMAL_SIZE
#define memSizeMostSignificantBit  uint64MostSignificantBit
#define memSizeLeastSignificantBit uint64LeastSignificantBit
#endif

#if OS_OFF_T_SIZE == 32
#if LONG_SIZE == 32
#define F_D_OFF(width)     "%" #width "ld"
#define F_U_OFF(width)     "%" #width "lu"
#define F_X_OFF(width)     "%" #width "lx"
#define FMT_D_OFF          "%ld"
#define FMT_U_OFF          "%lu"
#define FMT_X_OFF          "%lx"
#else
#define F_D_OFF(width)     F_D32(width)
#define F_U_OFF(width)     F_U32(width)
#define F_X_OFF(width)     F_X32(width)
#define FMT_D_OFF          FMT_D32
#define FMT_U_OFF          FMT_U32
#define FMT_X_OFF          FMT_X32
#endif
#elif OS_OFF_T_SIZE == 64
#define F_D_OFF(width)     F_D64(width)
#define F_U_OFF(width)     F_U64(width)
#define F_X_OFF(width)     F_X64(width)
#define FMT_D_OFF          FMT_D64
#define FMT_U_OFF          FMT_U64
#define FMT_X_OFF          FMT_X64
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif

typedef int                priorityType;

/**
 *  NULL_TERMINATION_LEN is the length for the terminating null
 *  char (\0) of a null-terminated string. The null termination
 *  is measured in characters and not in bytes. A null-terminated
 *  wide char string is terminated with one wide null char.
 *  NULL_TERMINATION_LEN can be used for normal null-terminated
 *  strings and for null-terminated wide char strings.
 */
#define NULL_TERMINATION_LEN 1

typedef unsigned char      ucharType;
typedef char              *cstriType;
typedef unsigned char     *ustriType;
typedef memSizeType        sySizeType;
typedef FILE              *cFileType;
typedef int                fileDesType;

typedef uint16Type           utf16charType;
typedef utf16charType       *utf16striType;
typedef const utf16charType *const_utf16striType;

typedef uint32Type           utf32charType;
typedef utf32charType       *utf32striType;
typedef const utf32charType *const_utf32striType;

typedef int                socketType;
typedef unsigned int       usocketType;

/* Possible values for SOCKET_LIB: */
#define NO_SOCKETS      -1
#define UNIX_SOCKETS     1
#define WINSOCK_SOCKETS  2

#if SOCKET_LIB == UNIX_SOCKETS
typedef int                os_socketType;
#elif SOCKET_LIB == WINSOCK_SOCKETS
typedef unsigned int       os_socketType;
#endif

/* Possible values for DIR_LIB: */
#define NO_DIRECTORY     -1
#define DIRENT_DIRECTORY  1
#define DIRECT_DIRECTORY  2
#define DIRDOS_DIRECTORY  3
#define DIRWIN_DIRECTORY  4

/* Possible values for BIGINT_LIBRARY: */
#define NO_BIG_LIBRARY  -1
#define BIG_RTL_LIBRARY  1
#define BIG_GMP_LIBRARY  2

typedef const char            *const_cstriType;
typedef const unsigned char   *const_ustriType;

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
#define OKAY_NO_ERROR   0
#define MEMORY_ERROR    1
#define NUMERIC_ERROR   2
#define OVERFLOW_ERROR  3
#define RANGE_ERROR     4
#define INDEX_ERROR     5
#define FILE_ERROR      6
#define DATABASE_ERROR  7
#define GRAPHIC_ERROR   8
#define ACTION_ERROR    9
#define CREATE_ERROR    9
#define DESTROY_ERROR   9
#define COPY_ERROR      9
#define IN_ERROR        9


#if HAS_SIGSETJMP
#define do_setjmp(env)        sigsetjmp(env, 1)
#define do_longjmp(env, val)  siglongjmp(env, val);
#define longjmpPosition sigjmp_buf
#else
#define do_setjmp(env)        setjmp(env)
#define do_longjmp(env, val)  longjmp(env, val);
#define longjmpPosition jmp_buf
#endif

#if FORWARD_TERM_CALLS
boolType findTermDll (void);
#else
#define findTermDll() TRUE
#endif


/* The macros below compute the array size as memSizeType    */
/* value. The computation avoids a signed integer overflow.  */
/* The computation fails if max_position is the maximum      */
/* positive value of intType and min_position is the minimum */
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
typedef struct fileStruct     *fileType;
typedef struct pollStruct     *pollType;
typedef struct winStruct      *winType;
typedef struct processStruct  *processType;
typedef struct databaseStruct *databaseType;
typedef struct sqlStmtStruct  *sqlStmtType;

typedef const struct setStruct      *const_setType;
typedef const struct striStruct     *const_striType;
typedef const struct bstriStruct    *const_bstriType;
typedef const struct fileStruct     *const_fileType;
typedef const struct pollStruct     *const_pollType;
typedef const struct winStruct      *const_winType;
typedef const struct processStruct  *const_processType;
typedef const struct databaseStruct *const_databaseType;
typedef const struct sqlStmtStruct  *const_sqlStmtType;

typedef struct setStruct {
    intType min_position;
    intType max_position;
    bitSetType bitset[1];
  } setRecord;

typedef struct striStruct {
    memSizeType size;
#if WITH_STRI_CAPACITY
    memSizeType capacity;
#endif
#if ALLOW_STRITYPE_SLICES
    strElemType *mem;
    strElemType  mem1[1];
#else
    strElemType mem[1];
#endif
  } striRecord;

typedef struct bstriStruct {
    memSizeType size;
#if ALLOW_BSTRITYPE_SLICES
    ucharType *mem;
    ucharType  mem1[1];
#else
    ucharType mem[1];
#endif
  } bstriRecord;

typedef struct fileStruct {
    cFileType cFile;
    uintType usage_count;
    boolType readingAllowed;
    boolType writingAllowed;
  } fileRecord;

typedef struct pollStruct {
#if !EMPTY_STRUCTS_ALLOWED
    int dummy;
#endif
  } pollRecord;

typedef struct winStruct {
    uintType usage_count;
    /* The rest of the structure is only accessible for the driver */
  } winRecord;

typedef struct processStruct {
    uintType usage_count;
    fileType stdIn;
    fileType stdOut;
    fileType stdErr;
    /* The rest of the structure is only accessible for the driver */
  } processRecord;

typedef struct databaseStruct {
    uintType usage_count;
    /* The rest of the structure is only accessible for the driver */
  } databaseRecord;

typedef struct sqlStmtStruct {
    uintType usage_count;
    /* The rest of the structure is only accessible for the driver */
  } sqlStmtRecord;

typedef struct freeListElemStruct {
    struct freeListElemStruct *next;
  } *freeListElemType;

#define LIST_BUFFER_SIZE 4096

typedef struct bufferStruct {
    struct bufferStruct *next;
    ucharType buffer[LIST_BUFFER_SIZE];
  } *bufferList;


#if BIGINT_LIBRARY == BIG_RTL_LIBRARY

/***************************************/
/*                                     */
/*   Define bigIntType for big_rtl.c   */
/*                                     */
/***************************************/

#define WITH_BIGINT_CAPACITY 1

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
#if WITH_BIGINT_CAPACITY
    memSizeType capacity;
#endif
    bigDigitType bigdigits[1];
  } bigIntRecord;

typedef bigIntRecord       *bigIntType;
typedef const bigIntRecord *const_bigIntType;


#elif BIGINT_LIBRARY == BIG_GMP_LIBRARY

/***************************************/
/*                                     */
/*   Define bigIntType for big_gmp.c   */
/*                                     */
/***************************************/

typedef mpz_ptr     bigIntType;
typedef mpz_srcptr  const_bigIntType;


#endif

/* Logging */

/* The logging infrastructure uses the following definitions:
     #define LOG_FUNCTIONS 0
     #define VERBOSE_EXCEPTIONS 0
   These values must be definied at the beginning of a source file.
   Possible values are:
     0 The logging is deactivated
     1 The logging is activated
    -1 The logging is deactivated even if EVERYWHERE is active
   With these settings logging can be switched on and off in individual source files.
   The actual logging is done with the macros:
     logFunction
     logMessage
     logError
   The file config.h contains the definitions:
     #define LOG_FUNCTIONS_EVERYWHERE 0
     #define LOG_MESSAGES_EVERYWHERE 0
     #define VERBOSE_EXCEPTIONS_EVERYWHERE 0
   These settings can be used to activate logging in (almost) every file.
   Only the files with a log setting of -1 are excluded.
*/

#define LOG_FUNCTIONS_ACTIVE ((LOG_FUNCTIONS_EVERYWHERE && LOG_FUNCTIONS >= 0) || LOG_FUNCTIONS > 0)
#define VERBOSE_EXCEPTIONS_ACTIVE ((VERBOSE_EXCEPTIONS_EVERYWHERE && VERBOSE_EXCEPTIONS >= 0) || VERBOSE_EXCEPTIONS > 0)

#if LOG_FUNCTIONS_ACTIVE
#if CHECK_STACK
#define logFunction(logStatements) checkStack(TRUE); printf(__FILE__ ": "); logStatements
#else
#define logFunction(logStatements) printf(__FILE__ ": "); logStatements
#endif
#define logFunctionResult(logStatements) printf(" --> "); logStatements
#define logSignalFunction(logStatements) printf(__FILE__ ": "); logStatements
#elif CHECK_STACK
#define logFunction(logStatements) checkStack(TRUE)
#define logFunctionResult(logStatements)
#define logSignalFunction(logStatements)
#else
#define logFunction(logStatements)
#define logFunctionResult(logStatements)
#define logSignalFunction(logStatements)
#endif

#if LOG_MESSAGES_EVERYWHERE
#define logMessage(logStatements) logStatements
#else
#define logMessage(logStatements)
#endif

#if VERBOSE_EXCEPTIONS_ACTIVE
#define logError(logStatements) printf(" *** "); logStatements
#define logErrorIfTrue(cond, logStatements) if (unlikely(cond)) { logError(logStatements) }
#else
#define logError(logStatements)
#define logErrorIfTrue(cond, logStatements)
#endif

#define ANY_LOG_ACTIVE (LOG_FUNCTIONS_ACTIVE || LOG_MESSAGES_EVERYWHERE || VERBOSE_EXCEPTIONS_ACTIVE)

/* Allow to activate selected logging functions by adding X. */

#define logFunction0(logStatements)
#define logFunctionX(logStatements) printf(__FILE__ ": "); logStatements
#define logFunctionResultX(logStatements) printf(" --> "); logStatements
#define logSignalFunctionX(logStatements) printf(__FILE__ ": "); logStatements
#define logMessageX(logStatements) logStatements
#define logErrorX(logStatements) printf(" *** "); logStatements
#define logErrorIfTrueX(cond, logStatements) if (unlikely(cond)) { logErrorX(logStatements) }
