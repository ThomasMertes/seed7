/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2010  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Compiler data reflection                                */
/*  File: seed7/src/typ_data.c                                      */
/*  Changes: 1993, 1994, 1999, 2000, 2008, 2010  Thomas Mertes      */
/*  Content: Primitive actions for the type type.                   */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "datautl.h"
#include "typeutl.h"
#include "striutl.h"
#include "listutl.h"
#include "hsh_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "typ_data.h"



static striType funcTypeName (typeType funcType, errInfoType *err_info)

  {
    typeType aType;
    memSizeType length = 0;
    memSizeType pos;
    striType resultTypeName;
    striType funcStri;
    striType varfuncStri;
    striType typeName;

  /* funcTypeName */
    aType = funcType;
    do {
      if (aType->is_varfunc_type) {
        length += STRLEN("varfunc ");
      } else {
        length += STRLEN("func ");
      } /* if */
      aType = aType->result_type;
    } while (aType != NULL &&
             aType->name == NULL && aType->result_type != NULL);
    if (aType == NULL) {
      resultTypeName = cstri_to_stri(" *NULL_TYPE* ");
    } else if (aType->name != NULL) {
      resultTypeName = cstri8_to_stri(id_string2(aType->name), err_info);
    } else {
      resultTypeName = cstri_to_stri(" *ANONYM_TYPE* ");
    } /* if */
    funcStri = cstri_to_stri("func ");
    varfuncStri = cstri_to_stri("varfunc ");
    if (unlikely(resultTypeName == NULL ||
                 funcStri == NULL || varfuncStri == NULL ||
                 !ALLOC_STRI_CHECK_SIZE(typeName,
                                        length + resultTypeName->size))) {
      if (*err_info == OKAY_NO_ERROR) {
        *err_info = MEMORY_ERROR;
      } /* if */
      typeName = NULL;
    } else {
      typeName->size = length + resultTypeName->size;
      aType = funcType;
      pos = 0;
      do {
        if (aType->is_varfunc_type) {
          memcpy(&typeName->mem[pos], varfuncStri->mem,
                 varfuncStri->size * sizeof(strElemType));
          pos += varfuncStri->size;
        } else {
          memcpy(&typeName->mem[pos], funcStri->mem,
                 funcStri->size * sizeof(strElemType));
          pos += funcStri->size;
        } /* if */
        aType = aType->result_type;
      } while (aType != NULL &&
               aType->name == NULL && aType->result_type != NULL);
      memcpy(&typeName->mem[pos], resultTypeName->mem,
             resultTypeName->size * sizeof(strElemType));
    } /* if */
    if (resultTypeName != NULL) {
      FREE_STRI(resultTypeName);
    } /* if */
    if (funcStri != NULL) {
      FREE_STRI(funcStri);
    } /* if */
    if (varfuncStri != NULL) {
      FREE_STRI(varfuncStri);
    } /* if */
    return typeName;
  } /* funcTypeName */



/**
 *  Compare two types.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType typCmp (const const_typeType type1, const const_typeType type2)

  {
    intType signumValue;

  /* typCmp */
    if ((memSizeType) type1 < (memSizeType) type2) {
      signumValue = -1;
    } else {
      signumValue = (memSizeType) type1 > (memSizeType) type2;
    } /* if */
    return signumValue;
  } /* typCmp */



/**
 *  Reinterpret the generic parameters as typeType and call typCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(typeType).
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType typCmpGeneric (const genericType value1, const genericType value2)

  { /* typCmpGeneric */
    return typCmp((const_typeType) ((const_rtlObjectType *) &value1)->value.typeValue,
                  (const_typeType) ((const_rtlObjectType *) &value2)->value.typeValue);
  } /* typCmpGeneric */



typeType typFunc (typeType basic_type)

  {
    typeType func_type;

  /* typFunc */
    logFunction(printf("typFunc(" FMT_X_MEM ")\n", (memSizeType) basic_type););
    if (unlikely((func_type = get_func_type(NULL, basic_type)) == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("typFunc --> " FMT_X_MEM "\n", (memSizeType) func_type););
    return func_type;
  } /* typFunc */



rtlArrayType typInterfaces (typeType any_type)

  {
    listType list_elem;
    rtlObjectType *array_pointer;
    memSizeType result_size;
    rtlArrayType result;

  /* typ_interfaces */
    result_size = list_length(any_type->interfaces);
    if (unlikely(!ALLOC_RTL_ARRAY(result, result_size))) {
      logError(printf("typInterfaces: ALLOC_RTL_ARRAY() failed.\n"););
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = 1;
      result->max_position = (intType) result_size;
      list_elem = any_type->interfaces;
      array_pointer = result->arr;
      while (list_elem != NULL) {
        array_pointer->value.typeValue = (rtlTypeType) list_elem->obj->value.typeValue;
        list_elem = list_elem->next;
        array_pointer++;
      } /* while */
    } /* if */
    return result;
  } /* typInterfaces */



boolType typIsDerived (typeType any_type)

  { /* typIsDerived */
    return any_type->meta != NULL;
  } /* typIsDerived */



boolType typIsFunc (typeType any_type)

  { /* typIsFunc */
    return any_type->result_type != NULL && !any_type->is_varfunc_type;
  } /* typIsFunc */



boolType typIsVarfunc (typeType any_type)

  { /* typIsVarfunc */
    return any_type->result_type != NULL && any_type->is_varfunc_type;
  } /* typIsVarfunc */



objectType typMatchobj (typeType actual_type)

  { /* typMatchobj */
    return actual_type->match_obj;
  } /* typMatchobj */



typeType typMeta (typeType any_type)

  {
    typeType meta;

  /* typMeta */
    logFunction(printf("typMeta(" FMT_X_MEM ")\n", (memSizeType) any_type););
    meta = any_type->meta;
    if (unlikely(meta == NULL)) {
      raise_error(RANGE_ERROR);
    } /* if */
    logFunction(printf("typMeta --> " FMT_X_MEM "\n", (memSizeType) meta););
    return meta;
  } /* typMeta */



intType typNum (typeType actual_type)

  {
    intType typeNumber;

  /* typNum */
    logFunction(printf("typNum(" FMT_X_MEM ")\n",
                       (memSizeType) actual_type););
    if (unlikely(actual_type == NULL)) {
      typeNumber = 0;
    } else if (unlikely(actual_type->owningProg == NULL ||
                        actual_type->owningProg->typeNumberMap == NULL)) {
      logError(printf("typNum(" FMT_X_MEM "): "
                      "No owning program or type number map.\n",
                      (memSizeType) actual_type););
      raise_error(RANGE_ERROR);
      typeNumber = 0;
    } else {
      typeNumber = (intType) hshIdxEnterDefault(
          (const rtlHashType) actual_type->owningProg->typeNumberMap,
          (genericType) (memSizeType) actual_type,
          (genericType) actual_type->owningProg->nextFreeTypeNumber,
          (intType) (((memSizeType) actual_type) >> 6));
      if (typeNumber == actual_type->owningProg->nextFreeTypeNumber) {
        actual_type->owningProg->nextFreeTypeNumber++;
      } /* if */
    } /* if */
    logFunction(printf("typNum --> " FMT_D "\n", typeNumber););
    return typeNumber;
  } /* typNum */



typeType typResult (typeType any_type)

  {
    typeType result_type;

  /* typResult */
    logFunction(printf("typResult(" FMT_X_MEM ")\n", (memSizeType) any_type););
    result_type = any_type->result_type;
    if (unlikely(result_type == NULL)) {
      raise_error(RANGE_ERROR);
    } /* if */
    logFunction(printf("typResult --> " FMT_X_MEM "\n", (memSizeType) result_type););
    return result_type;
  } /* typResult */



striType typStr (typeType type_arg)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    striType typeName;

  /* typStr */
    logFunction(printf("typStr(" FMT_X_MEM ")\n", (memSizeType) type_arg););
    if (type_arg != NULL) {
      if (type_arg->name != NULL) {
        typeName = cstri8_to_stri(id_string2(type_arg->name), &err_info);
        if (unlikely(typeName == NULL)) {
          raise_error(err_info);
        } /* if */
      } else if (type_arg->result_type != NULL) {
        typeName = funcTypeName(type_arg, &err_info);
        if (unlikely(typeName == NULL)) {
          raise_error(err_info);
        } /* if */
      } else {
        typeName = cstri_to_stri(" *ANONYM_TYPE* ");
        if (unlikely(typeName == NULL)) {
          raise_error(MEMORY_ERROR);
        } /* if */
      } /* if */
    } else {
      typeName = cstri_to_stri(" *NULL_TYPE* ");
      if (unlikely(typeName == NULL)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("typStr --> \"%s\"\n", striAsUnquotedCStri(typeName)););
    return typeName;
  } /* typStr */



typeType typVarfunc (typeType basic_type)

  {
    typeType result;

  /* typVarfunc */
    logFunction(printf("typVarfunc(" FMT_X_MEM ")\n", (memSizeType) basic_type););
    if (unlikely((result = get_varfunc_type(NULL, basic_type)) == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("typVarfunc --> " FMT_X_MEM "\n", (memSizeType) result););
    return result;
  } /* typVarfunc */
