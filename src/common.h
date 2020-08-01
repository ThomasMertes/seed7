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


#ifdef RENAMED_POSIX_FUNCTIONS
#ifndef fileno
#define fileno   _fileno
#endif
#define kbhit    _kbhit
#define setmode  _setmode
#define fdopen   _fdopen
#define isatty   _isatty
#define fseeki64 _fseeki64
#define ftelli64 _ftelli64
#endif


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

#define F_D32(width) "%" #width INT32TYPE_FORMAT "d"
#define F_U32(width) "%" #width INT32TYPE_FORMAT "u"
#define F_X32(width) "%" #width INT32TYPE_FORMAT "x"
#define FMT_D32      "%" INT32TYPE_FORMAT "d"
#define FMT_U32      "%" INT32TYPE_FORMAT "u"
#define FMT_X32      "%" INT32TYPE_FORMAT "x"

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

#define F_D64(width) "%" #width INT64TYPE_FORMAT "d"
#define F_U64(width) "%" #width INT64TYPE_FORMAT "u"
#define F_X64(width) "%" #width INT64TYPE_FORMAT "x"
#define FMT_D64      "%" INT64TYPE_FORMAT "d"
#define FMT_U64      "%" INT64TYPE_FORMAT "u"
#define FMT_X64      "%" INT64TYPE_FORMAT "x"

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


#if TIME_T_SIZE == 32
#define TIME_T_MAX INT32TYPE_MAX
#if TIME_T_SIGNED
#define FMT_T                     FMT_D32
#else
#define FMT_T                     FMT_U32
#endif
#elif TIME_T_SIZE == 64
#define TIME_T_MAX INT64TYPE_MAX
#if TIME_T_SIGNED
#define FMT_T                     FMT_D64
#else
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
#define castToShort(num)  (inShortRange(num) ? (short int) (num) : (raise_error(RANGE_ERROR), (short int) 0))
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
#if INT_RANGE_IN_DOUBLE_MAX <= INTTYPE_MAX
#define INT_RANGE_IN_FLOATTYPE_MAX INT_RANGE_IN_DOUBLE_MAX
#else
#define INT_RANGE_IN_FLOATTYPE_MAX INTTYPE_MAX
#endif
#define FLOATTYPE_TO_INT_CONVERSION_LIMIT DOUBLE_MANTISSA_FACTOR
#define FLOATTYPE_MANTISSA_FACTOR DOUBLE_MANTISSA_FACTOR
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
#define FLOATTYPE_MANTISSA_FACTOR FLOAT_MANTISSA_FACTOR
#define FMT_E FMT_E_FLT
#endif

#define POW_FUNCTION_OKAY (POW_OF_NAN_OKAY && POW_OF_ZERO_OKAY && POW_OF_ONE_OKAY && POW_EXP_NAN_OKAY && POW_EXP_MINUS_INFINITY_OKAY)


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
#endif

#if OS_OFF_T_SIZE == 32
#define F_D_OFF(width)     F_D32(width)
#define F_U_OFF(width)     F_U32(width)
#define F_X_OFF(width)     F_X32(width)
#define FMT_D_OFF          FMT_D32
#define FMT_U_OFF          FMT_U32
#define FMT_X_OFF          FMT_X32
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
typedef FILE              *fileType;
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
#define FILE_ERROR      5
#define ACTION_ERROR    6
#define CREATE_ERROR    7
#define DESTROY_ERROR   8
#define COPY_ERROR      9
#define IN_ERROR       10


#if HAS_SIGSETJMP
#define do_setjmp(env)        sigsetjmp(env, 1)
#define do_longjmp(env, val)  siglongjmp(env, val);
#define long_jump_position sigjmp_buf
#else
#define do_setjmp(env)        setjmp(env)
#define do_longjmp(env, val)  longjmp(env, val);
#define long_jump_position jmp_buf
#endif


#ifndef likely
#define likely(x) (x)
#endif
#ifndef unlikely
#define unlikely(x) (x)
#endif
#ifndef NORETURN
#define NORETURN
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
typedef struct processStruct  *processType;
typedef struct databaseStruct *databaseType;
typedef struct sqlStmtStruct  *sqlStmtType;

typedef const struct setStruct      *const_setType;
typedef const struct striStruct     *const_striType;
typedef const struct bstriStruct    *const_bstriType;
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

typedef struct processStruct {
    uintType usage_count;
    fileType stdIn;
    fileType stdOut;
    fileType stdErr;
    /* The rest of the structure is only accessable for the driver */
  } processRecord;

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

#define LIST_BUFFER_SIZE 4096

typedef struct bufferStruct {
    struct bufferStruct *next;
    ucharType buffer[LIST_BUFFER_SIZE];
  } *bufferList;


#ifdef USE_BIG_RTL_LIBRARY

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


#else
#ifdef USE_BIG_GMP_LIBRARY

/***************************************/
/*                                     */
/*   Define bigIntType for big_gmp.c   */
/*                                     */
/***************************************/

typedef mpz_ptr     bigIntType;
typedef mpz_srcptr  const_bigIntType;


#endif
#endif

/* Logging */

#if LOG_FUNCTIONS_EVERYWHERE || (defined LOG_FUNCTIONS && LOG_FUNCTIONS)
#define logFunction(logStatements) printf(__FILE__ ": "); logStatements
#define logFunctionResult(logStatements) printf(" --> "); logStatements
#else
#define logFunction(logStatements)
#define logFunctionResult(logStatements)
#endif

#define logMessage(logStatements)

#if VERBOSE_EXCEPTIONS_EVERYWHERE || (defined VERBOSE_EXCEPTIONS && VERBOSE_EXCEPTIONS)
#define logError(logStatements) printf(" *** "); logStatements
#define logErrorIsActive(logSupportStatements) logSupportStatements
#else
#define logError(logStatements)
#define logErrorIsActive(logSupportStatements)
#endif

#define ANY_LOG_ACTIVE (LOG_FUNCTIONS || LOG_FUNCTIONS_EVERYWHERE || \
                        VERBOSE_EXCEPTIONS || VERBOSE_EXCEPTIONS_EVERYWHERE)

/* Allow to activate selected logging functions by adding X. */

#define logFunctionX(logStatements) printf(__FILE__ ": "); logStatements
#define logFunctionResultX(logStatements) printf(" --> "); logStatements
#define logMessageX(logStatements) logStatements
#define logErrorX(logStatements) printf(" *** "); logStatements
