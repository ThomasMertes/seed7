/********************************************************************/
/*                                                                  */
/*  data_rtl.h    Basic type definitions and settings.              */
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
/*  File: seed7/src/data_rtl.h                                      */
/*  Changes: 1990 - 1994, 2000, 2005  Thomas Mertes                 */
/*  Content: Type definitions for general data structures.          */
/*                                                                  */
/********************************************************************/

/* In classic C a function cannot return structs by value and */
/* cannot have struct value parameters. Because of this the   */
/* unsigned integer type genericType is used, instead of      */
/* rtlObjectType. To work reliably it must be assured that    */
/* sizeof(genericType) == sizeof(rtlObjectType)               */

#if INTTYPE_SIZE >= POINTER_SIZE
#if INTTYPE_SIZE >= FLOATTYPE_SIZE
#define GENERIC_SIZE INTTYPE_SIZE
#else
#define GENERIC_SIZE FLOATTYPE_SIZE
#endif
#elif POINTER_SIZE >= FLOATTYPE_SIZE
#define GENERIC_SIZE POINTER_SIZE
#else
#define GENERIC_SIZE FLOATTYPE_SIZE
#endif

#if   GENERIC_SIZE == 64
typedef uint64Type genericType;
#define GENERIC_SUFFIX(num) UINT64_SUFFIX(num)
#define FMT_D_GEN  FMT_D64
#define FMT_U_GEN  FMT_U64
#define FMT_X_GEN  FMT_X64
#elif GENERIC_SIZE == 32
typedef uint32Type genericType;
#define GENERIC_SUFFIX(num) UINT32_SUFFIX(num)
#define FMT_D_GEN  FMT_D32
#define FMT_U_GEN  FMT_U32
#define FMT_X_GEN  FMT_X32
#endif

#if GENERIC_SIZE != POINTER_SIZE
#define INIT_GENERIC_PTR(gen) (gen) = GENERIC_SUFFIX(0)
#else
/* Initialization is not necessary because it will be overwritten. */
#define INIT_GENERIC_PTR(gen)
#endif

#if GENERIC_SIZE != INTTYPE_SIZE
#define INIT_GENERIC_INT(gen) (gen) = GENERIC_SUFFIX(0)
#else
/* Initialization is not necessary because it will be overwritten. */
#define INIT_GENERIC_INT(gen)
#endif

typedef struct rtlTypeStruct     *rtlTypeType;
typedef struct rtlListStruct     *rtlListType;
typedef struct rtlArrayStruct    *rtlArrayType;
typedef struct rtlStructStruct   *rtlStructType;
typedef struct rtlStructStruct   *rtlInterfaceType;
typedef struct rtlHashElemStruct *rtlHashElemType;
typedef struct rtlHashStruct     *rtlHashType;
typedef void *rtlPtrType;

typedef const struct rtlTypeStruct     *const_rtlTypeType;
typedef const struct rtlListStruct     *const_rtlListType;
typedef const struct rtlArrayStruct    *const_rtlArrayType;
typedef const struct rtlStructStruct   *const_rtlStructType;
typedef const struct rtlStructStruct   *const_rtlInterfaceType;
typedef const struct rtlHashElemStruct *const_rtlHashElemType;
typedef const struct rtlHashStruct     *const_rtlHashType;
typedef const void *const_rtlPtrType;

typedef union {
  /* Genericvalue must be the first element in the union. */
  /* This allows initialisations of rtlValueUnion values. */
    genericType      genericValue;
/*    posType          pos;            ** SYMBOLOBJECT */
/*    nodeType         nodeValue;      ** MDULEOBJECT */
    boolType         boolValue;
    rtlTypeType      typeValue;      /* TYPEOBJECT */
    intType          intValue;       /* INTOBJECT */
    bigIntType       bigIntValue;    /* BIGINTOBJECT */
    charType         charValue;      /* CHAROBJECT */
    striType         striValue;      /* STRIOBJECT */
    bstriType        bstriValue;     /* BSTRIOBJECT */
    rtlArrayType     arrayValue;     /* ARRAYOBJECT */
    rtlHashType      hashValue;      /* HASHOBJECT */
    setType          setValue;       /* SETOBJECT */
    rtlStructType    structValue;    /* STRUCTOBJECT */
    rtlInterfaceType interfaceValue; /* INTERFACEOBJECT */
    fileType         fileValue;      /* FILEOBJECT */
    fileDesType      fileDesValue;   /* FILEDESOBJECT */
    socketType       socketValue;    /* SOCKETOBJECT */
    rtlListType      listValue;      /* LISTOBJECT, EXPROBJECT */
    winType          winValue;       /* WINOBJECT */
    rtlPtrType       ptrValue;
/*    enumType         enumValue;      */
/*    pollType         pollValue;      */
/*    objRefType       objRefValue;    */
/*    rtlObjectType    objValue;       ** ENUMLITERALOBJECT, CONSTENUMOBJECT */
                                     /* VARENUMOBJECT, VALUEPARAMOBJECT */
                                     /* REFPARAMOBJECT, RESULTOBJECT */
                                     /* LOCALVOBJECT, FORMPARAMOBJECT */
/*    blockType        blockValue;     ** BLOCKOBJECT */
/*    actType          actValue;       ** ACTOBJECT */
/*    progType         progValue;      ** PROGOBJECT */
#ifdef WITH_FLOAT
    floatType      floatValue;   /* FLOATOBJECT */
#endif
  } rtlValueUnion;

typedef struct rtlObjectStruct {
    rtlValueUnion value;
  } rtlObjectType;

typedef const struct rtlObjectStruct const_rtlObjectType;

typedef struct rtlTypeStruct {
    int dummy;
  } rtlTypeRecord;

typedef struct rtlListStruct {
    rtlListType next;
    rtlObjectType obj;
  } rtlListRecord;

typedef struct rtlArrayStruct {
    intType min_position;
    intType max_position;
    rtlObjectType arr[1];
  } rtlArrayRecord;

typedef struct rtlStructStruct {
    memSizeType usage_count;
    uint32Type type_num;
    rtlObjectType stru[1];
  } rtlStructRecord;

typedef struct rtlHashElemStruct {
    rtlHashElemType next_less;
    rtlHashElemType next_greater;
    rtlObjectType key;
    rtlObjectType data;
  } rtlHashElemRecord;

typedef struct rtlHashStruct {
    unsigned int bits;
    unsigned int mask;
    unsigned int table_size;
    memSizeType size;
    rtlHashElemType table[1];
  } rtlHashRecord;

typedef struct rtlTimeStruct {
    rtlObjectType year;
    rtlObjectType month;
    rtlObjectType day;
    rtlObjectType hour;
    rtlObjectType minute;
    rtlObjectType second;
    rtlObjectType micro_second;
    rtlObjectType timeZone;
    rtlObjectType daylightSavingTime;
  } rtlTimeRecord;

typedef intType (*compareType) (genericType, genericType);
typedef genericType (*createFuncType) (genericType);
typedef void (*destrFuncType) (genericType);
typedef void (*copyFuncType) (genericType *, genericType);


intType genericCmp (const genericType value1, const genericType value2);
void genericCpy (genericType *const dest, const genericType source);
genericType genericCreate (genericType source);
void genericDestr (genericType old_value);
