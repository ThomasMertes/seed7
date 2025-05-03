/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2007, 2008, 2010 - 2013  Thomas Mertes     */
/*                2015, 2018 - 2020, 2022, 2024  Thomas Mertes      */
/*                2025  Thomas Mertes                               */
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
/*  File: seed7/src/ref_data.c                                      */
/*  Changes: 1991 - 1994, 2004, 2007, 2008  Thomas Mertes           */
/*           2010 - 2013, 2015, 2018 - 2020, 2022  Thomas Mertes    */
/*           2024, 2025  Thomas Mertes                              */
/*  Content: Primitive actions for the reference type.              */
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
#include "flistutl.h"
#include "datautl.h"
#include "striutl.h"
#include "listutl.h"
#include "identutl.h"
#include "objutl.h"
#include "infile.h"    /* from the compiler library */
#include "name.h"      /* from the compiler library */
#include "blockutl.h"  /* from the compiler library */
#include "syvarutl.h"  /* from the compiler library */
#include "hsh_rtl.h"
#include "int_rtl.h"
#include "str_rtl.h"
#include "big_drv.h"
#include "pol_drv.h"
#include "rtl_err.h"
#if ANY_LOG_ACTIVE
#include "traceutl.h"
#endif

#undef EXTERN
#define EXTERN
#include "ref_data.h"


#define MAX_CSTRI_BUFFER_LEN 40



objectType refAlloc (const const_objectType aReference)

  {
    objectType created_object;

  /* refAlloc */
    logFunction(printf("refAlloc(");
                trace1(aReference);
                printf(")\n"););
    if (unlikely(!ALLOC_OBJECT(created_object))) {
      raise_error(MEMORY_ERROR);
    } else {
      created_object->type_of = aReference->type_of;
      memcpy(&created_object->descriptor, &aReference->descriptor,
          sizeof(descriptorUnion));
      /* Copies the POSINFO flag (and all other flags): */
      INIT_CATEGORY_OF_OBJ(created_object, aReference->objcategory);
      created_object->value.objValue = NULL;
    } /* if */
    return created_object;
  } /* refAlloc */



objectType refAllocInt (boolType isVar, typeType aType,
    const intType number)

  {
    objectType created_object;

  /* refAllocInt */
    logFunction(printf("refAllocInt(%s, ",
                       isVar ? "TRUE" : "FALSE");
                printtype(aType);
                printf(", " FMT_D ")\n", number););
    if (unlikely(!ALLOC_OBJECT(created_object))) {
      raise_error(MEMORY_ERROR);
    } else {
      created_object->type_of = aType;
      created_object->descriptor.property = NULL;
      INIT_CATEGORY_OF_OBJ(created_object, INTOBJECT);
      if (isVar) {
        SET_VAR_FLAG(created_object);
      } /* if */
      created_object->value.intValue = number;
    } /* if */
    return created_object;
  } /* refAllocInt */



objectType refAllocStri (boolType isVar, typeType aType,
    const const_striType stri)

  {
    objectType created_object;

  /* refAllocStri */
    logFunction(printf("refAllocStri(%s, ",
                       isVar ? "TRUE" : "FALSE");
                printtype(aType);
                printf(", \"%s\")\n",
                       striAsUnquotedCStri(stri)););
    if (unlikely(!ALLOC_OBJECT(created_object))) {
      raise_error(MEMORY_ERROR);
    } else {
      created_object->type_of = aType;
      created_object->descriptor.property = NULL;
      INIT_CATEGORY_OF_OBJ(created_object, STRIOBJECT);
      if (isVar) {
        SET_VAR_FLAG(created_object);
      } /* if */
      created_object->value.striValue = strCreate(stri);
    } /* if */
    return created_object;
  } /* refAllocStri */



objectType refAllocVar (typeType aType, const intType aCategory)

  {
    objectType created_object;

  /* refAllocVar */
    logFunction(printf("refAllocVar(");
                printtype(aType);
                printf(", ");
                printcategory(aCategory);
                printf(")\n"););
    if (unlikely(!ALLOC_OBJECT(created_object))) {
      raise_error(MEMORY_ERROR);
    } else {
      created_object->type_of = aType;
      created_object->descriptor.property = NULL;
      INIT_CATEGORY_OF_OBJ(created_object, aCategory);
      SET_VAR_FLAG(created_object);
      created_object->value.intValue = 0;
    } /* if */
    return created_object;
  } /* refAllocVar */



/**
 *  Obtain the maximum index of the array referenced by 'arrayRef'.
 *  @param arrayRef Reference to an array object.
 *  @return the maximum index of the array.
 *  @exception RANGE_ERROR If 'arrayRef' does not refer to an array.
 */
intType refArrMaxIdx (const const_objectType arrayRef)

  { /* refArrMaxIdx */
    if (unlikely(arrayRef == NULL ||
                 CATEGORY_OF_OBJ(arrayRef) != ARRAYOBJECT)) {
      logError(printf("refArrMaxIdx(");
               trace1(arrayRef);
               printf("): Category is not ARRAYOBJECT.\n"););
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      return take_array(arrayRef)->max_position;
    } /* if */
  } /* refArrMaxIdx */



/**
 *  Obtain the minimum index of the array referenced by 'arrayRef'.
 *  @param arrayRef Reference to an array object.
 *  @return the minimum index of the array.
 *  @exception RANGE_ERROR If 'arrayRef' does not refer to an array.
 */
intType refArrMinIdx (const const_objectType arrayRef)

  { /* refArrMinIdx */
    if (unlikely(arrayRef == NULL ||
                 CATEGORY_OF_OBJ(arrayRef) != ARRAYOBJECT)) {
      logError(printf("refArrMinIdx(");
               trace1(arrayRef);
               printf("): Category is not ARRAYOBJECT.\n"););
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      return take_array(arrayRef)->min_position;
    } /* if */
  } /* refArrMinIdx */



listType refArrToList (const const_objectType arrayRef)

  {
    arrayType arrayValue;
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* refArrToList */
    if (unlikely(arrayRef == NULL ||
                 CATEGORY_OF_OBJ(arrayRef) != ARRAYOBJECT)) {
      logError(printf("refArrToList(");
               trace1(arrayRef);
               printf("): Category is not ARRAYOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      arrayValue = take_array(arrayRef);
      if (unlikely(arrayValue->min_position > arrayValue->max_position &&
                   arraySize(arrayValue) != 0)) {
        logError(printf("refArrToList(");
                 trace1(arrayRef);
                 printf("): Illegal array value.\n"););
        raise_error(RANGE_ERROR);
        result = NULL;
      } else {
        result = array_to_list(arrayValue, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
          result = NULL;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* refArrToList */



/**
 *  Gets the body of the function referenced by 'funcRef'.
 *  @return the body expression of 'funcRef'.
 *  @exception RANGE_ERROR If 'funcRef' is NIL or
 *             category(funcRef) <> BLOCKOBJECT holds.
 */
objectType refBody (const const_objectType funcRef)

  {
    objectType result;

  /* refBody */
    if (unlikely(funcRef == NULL ||
                 CATEGORY_OF_OBJ(funcRef) != BLOCKOBJECT)) {
      logError(printf("refBody(");
               trace1(funcRef);
               printf("): Category is not BLOCKOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = funcRef->value.blockValue->body;
    } /* if */
    return result;
  } /* refBody */



/**
 *  Get the category of a referenced object.
 *  @return the category of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL.
 */
intType refCategory (const const_objectType aReference)

  {
    intType result;

  /* refCategory */
    if (unlikely(aReference == NULL)) {
      logError(printf("refCategory(NULL): Object is NULL.\n"););
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = CATEGORY_OF_OBJ(aReference);
    } /* if */
    return result;
  } /* refCategory */



/**
 *  Convert a [[string]] to a 'category'.
 *  @param catName Name of a category to be converted.
 *  @return the 'category' result fo the conversion.
 *  @exception RANGE_ERROR If there is no corresponding 'category'.
 */
intType refCatParse (const const_striType catName)

  {
    char name[MAX_CSTRI_BUFFER_LEN + NULL_TERMINATION_LEN];
    intType category;

  /* refCatParse */
    if (unlikely(catName->size > MAX_CSTRI_BUFFER_LEN)) {
      category = -1;
    } else {
      if (unlikely(conv_to_cstri(name, catName) == NULL)) {
        category = -1;
      } else {
        category = category_value(name);
      } /* if */
    } /* if */
    if (unlikely(category == -1)) {
      logError(printf("refCatParse(\"%s\"): No corresponding category.\n",
                      striAsUnquotedCStri(catName)););
      raise_error(RANGE_ERROR);
    } /* if */
    return category;
  } /* refCatParse */



/**
 *  Convert a category to a string.
 *  @param aCategory Category to be converted.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType refCatStr (intType aCategory)

  {
    striType result;

  /* refCatStr */
    result = cstri_to_stri(category_cstri((objectCategory) aCategory));
    if (unlikely(result == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* refCatStr */



/**
 *  Determine the file name of a referenced object.
 *  @return the file name of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType refFile (const const_objectType aReference)

  {
    striType fileName;

  /* refFile */
    logFunction(printf("refFile(");
                trace1(aReference);
                printf(")\n"););
    if (unlikely(aReference == NULL)) {
      logError(printf("refFile(NULL): Object is NULL.\n"););
      raise_error(RANGE_ERROR);
      fileName = NULL;
    } else {
      fileName = objectFileName(aReference);
    } /* if */
    logFunction(printf("refFile -> \"%s\"\n",
                       striAsUnquotedCStri(fileName)););
    return fileName;
  } /* refFile */



listType refHshDataToList (const const_objectType aReference)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* refHshDataToList */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != HASHOBJECT)) {
      logError(printf("refHshDataToList(");
               trace1(aReference);
               printf("): Category is not HASHOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = hash_data_to_list(take_hash(aReference), &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* refHshDataToList */



listType refHshKeysToList (const const_objectType aReference)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* refHshKeysToList */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != HASHOBJECT)) {
      logError(printf("refHshKeysToList(");
               trace1(aReference);
               printf("): Category is not HASHOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = hash_keys_to_list(take_hash(aReference), &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* refHshKeysToList */



intType refHshLength (const const_objectType aReference)

  {
    intType length;

  /* refHshLength */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != HASHOBJECT)) {
      logError(printf("refHshLength(");
               trace1(aReference);
               printf("): Category is not HASHOBJECT.\n"););
      raise_error(RANGE_ERROR);
      length = 0;
    } else {
      length = (intType) take_hash(aReference)->size;
    } /* if */
    return length;
  } /* refHshLength */



/**
 *  Determine if the referenced object is temporary.
 *  @return TRUE if ''aReference'' is temporary, FALSE otherwise.
 *  @exception RANGE_ERROR If ''aReference'' is NIL.
 */
boolType refIsTemp (const const_objectType aReference)

  { /* refIsTemp */
    logFunction(printf("refIsTemp(" FMT_U_MEM ")\n",
                       (memSizeType) aReference););
    if (unlikely(aReference == NULL)) {
      logError(printf("refIsTemp(NULL): Object is NULL.\n"););
      raise_error(RANGE_ERROR);
      return FALSE;
    } else {
      return TEMP_OBJECT(aReference) ? TRUE : FALSE;
    } /* if */
  } /* refIsTemp */



/**
 *  Determine if the referenced object is variable.
 *  @return TRUE if ''aReference'' is a variable, FALSE otherwise.
 *  @exception RANGE_ERROR If ''aReference'' is NIL.
 */
boolType refIsVar (const const_objectType aReference)

  { /* refIsVar */
    logFunction(printf("refIsVar(" FMT_U_MEM ")\n",
                       (memSizeType) aReference););
    if (unlikely(aReference == NULL)) {
      logError(printf("refIsVar(NULL): Object is NULL.\n"););
      raise_error(RANGE_ERROR);
      return FALSE;
    } else {
      return VAR_OBJECT(aReference) ? TRUE : FALSE;
    } /* if */
  } /* refIsVar */



objectType refItfToSct (const const_objectType aReference)

  {
    objectType result;

  /* refItfToSct */
    logFunction(printf("refItfToSct(" FMT_U_MEM ")\n",
                       (memSizeType) aReference););
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != INTERFACEOBJECT ||
                 take_reference(aReference) == NULL)) {
      logError(printf("refItfToSct(");
               trace1(aReference);
               printf("): Category is not INTERFACEOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = take_reference(aReference);
    } /* if */
    logFunction(printf("refItfToSct --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* refItfToSct */



/**
 *  Determine the line number of a referenced object.
 *  @return the line number of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL.
 */
intType refLine (const const_objectType aReference)

  {
    intType lineNumber;

  /* refLine */
    if (unlikely(aReference == NULL)) {
      logError(printf("refLine(NULL): Object is NULL.\n"););
      raise_error(RANGE_ERROR);
      lineNumber = 0;
    } else if (HAS_POSINFO(aReference)) {
      /* GET_LINE_NUM delivers an unsigned integer in the range 0 to 1048575 */
      lineNumber = (intType) GET_LINE_NUM(aReference);
    } else if (HAS_PROPERTY(aReference)) {
      /* trace1(aReference);
      printf(" %u %u\n",
          aReference->descriptor.property->line,
          aReference->descriptor.property->syNumberInLine); */
      /* Cast to intType: The line is probably in the range 0 to 2147483647 */
      lineNumber = (intType) aReference->descriptor.property->line;
    } else {
      lineNumber = 0;
    } /* if */
    return lineNumber;
  } /* refLine */



/**
 *  Gets the local constants of 'funcRef'.
 *  @return the local constants as ref_list.
 *  @exception RANGE_ERROR If 'funcRef' is NIL or
 *             refCategory(funcRef) <> BLOCKOBJECT holds.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
listType refLocalConsts (const const_objectType funcRef)

  {
    listType local_elem;
    listType *list_insert_place;
    errInfoType err_info = OKAY_NO_ERROR;
    listType localConsts = NULL;

  /* refLocalConsts */
    if (unlikely(funcRef == NULL ||
                 CATEGORY_OF_OBJ(funcRef) != BLOCKOBJECT)) {
      logError(printf("refLocalConsts(");
               trace1(funcRef);
               printf("): Category is not BLOCKOBJECT.\n"););
      raise_error(RANGE_ERROR);
    } else {
      list_insert_place = &localConsts;
      local_elem = funcRef->value.blockValue->local_consts;
      while (local_elem != NULL) {
        list_insert_place = append_element_to_list(list_insert_place,
            local_elem->obj, &err_info);
        local_elem = local_elem->next;
      } /* while */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        free_list(localConsts);
        localConsts = NULL;
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return localConsts;
  } /* refLocalConsts */



/**
 *  Gets the local variables of 'funcRef'.
 *  @return the local variables as ref_list.
 *  @exception RANGE_ERROR If 'funcRef' is NIL or
 *             refCategory(funcRef) <> BLOCKOBJECT holds.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
listType refLocalVars (const const_objectType funcRef)

  {
    locListType local_elem;
    listType *list_insert_place;
    errInfoType err_info = OKAY_NO_ERROR;
    listType localVars = NULL;

  /* refLocalVars */
    if (unlikely(funcRef == NULL ||
                 CATEGORY_OF_OBJ(funcRef) != BLOCKOBJECT)) {
      logError(printf("refLocalVars(");
               trace1(funcRef);
               printf("): Category is not BLOCKOBJECT.\n"););
      raise_error(RANGE_ERROR);
    } else {
      list_insert_place = &localVars;
      local_elem = funcRef->value.blockValue->local_vars;
      while (local_elem != NULL) {
        list_insert_place = append_element_to_list(list_insert_place,
            local_elem->local.object, &err_info);
        local_elem = local_elem->next;
      } /* while */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        free_list(localVars);
        localVars = NULL;
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return localVars;
  } /* refLocalVars */



/**
 *  Delivers an unique number for each object
 *  @return a unique object number.
 *  @exception RANGE_ERROR The type has no owning program or number map.
 *  @exception MEMORY_ERROR Not enough memory to maintain the object table.
 */
intType refNum (const const_objectType aReference)

  {
    intType objectNumber;

  /* refNum */
    logFunction(printf("refNum(" FMT_U_MEM ")\n",
                       (memSizeType) aReference););
    if (unlikely(aReference == NULL || aReference->type_of == NULL)) {
      objectNumber = 0;
    } else if (unlikely(aReference->type_of->owningProg == NULL ||
                        aReference->type_of->owningProg->objectNumberMap == NULL)) {
      logError(printf("refNum(" FMT_U_MEM "): "
                      "No owning program or object number map.\n",
                      (memSizeType) aReference););
      raise_error(RANGE_ERROR);
      objectNumber = 0;
    } else {
      objectNumber = (intType) hshIdxEnterDefault(
          aReference->type_of->owningProg->objectNumberMap,
          (genericType) (memSizeType) aReference,
          (genericType) aReference->type_of->owningProg->nextFreeObjectNumber,
          (intType) (((memSizeType) aReference) >> 6));
      if (objectNumber == aReference->type_of->owningProg->nextFreeObjectNumber) {
        aReference->type_of->owningProg->nextFreeObjectNumber++;
      } /* if */
    } /* if */
    logFunction(printf("refNum --> " FMT_D "\n", objectNumber););
    return objectNumber;
  } /* refNum */



/**
 *  Get the formal parameters of the function referenced by 'funcRef'.
 *  For objects without parameters an empty list is returned.
 *  @return the formal parameters as ref_list.
 *  @exception RANGE_ERROR If 'funcRef' is NIL.
 */
listType refParams (const const_objectType funcRef)

  {
    listType params;

  /* refParams */
    if (unlikely(funcRef == NULL)) {
      logError(printf("refParams(NULL): Object is NULL.\n"););
      raise_error(RANGE_ERROR);
      params = NULL;
    } else {
      if (likely(HAS_PROPERTY(funcRef))) {
        params = funcRef->descriptor.property->params;
      } else {
        params = NULL;
      } /* if */
    } /* if */
    return params;
  } /* refParams */



/**
 *  Determine the absolute file path of a referenced object.
 *  @return the absolute file path of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType refPath (const const_objectType aReference)

  {
    striType fileName;

  /* refPath */
    logFunction(printf("refPath(");
                trace1(aReference);
                printf(")\n"););
    if (unlikely(aReference == NULL)) {
      logError(printf("refPath(NULL): Object is NULL.\n"););
      raise_error(RANGE_ERROR);
      fileName = NULL;
    } else {
      fileName = objectFilePath(aReference);
    } /* if */
    logFunction(printf("refPath -> \"%s\"\n",
                       striAsUnquotedCStri(fileName)););
    return fileName;
  } /* refPath */



/**
 *  Gets the initialization value of the result variable of 'funcRef'.
 *  @return a reference to the initialization value.
 *  @exception RANGE_ERROR If 'funcRef' is NIL or
 *             refCategory(funcRef) <> BLOCKOBJECT holds.
 */
objectType refResini (const const_objectType funcRef)

  {
    objectType result;

  /* refResini */
    if (unlikely(funcRef == NULL ||
                 CATEGORY_OF_OBJ(funcRef) != BLOCKOBJECT)) {
      logError(printf("refResini(");
               trace1(funcRef);
               printf("): Category is not BLOCKOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = funcRef->value.blockValue->result.init_value;
    } /* if */
    return result;
  } /* refResini */



/**
 *  Gets the result variable of 'funcRef'.
 *  @return a reference to the result variable.
 *  @exception RANGE_ERROR If 'funcRef' is NIL or
 *             refCategory(funcRef) <> BLOCKOBJECT holds.
 */
objectType refResult (const const_objectType funcRef)

  {
    objectType result;

  /* refResult */
    if (unlikely(funcRef == NULL ||
                 CATEGORY_OF_OBJ(funcRef) != BLOCKOBJECT)) {
      logError(printf("refResult(");
               trace1(funcRef);
               printf("): Category is not BLOCKOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = funcRef->value.blockValue->result.object;
    } /* if */
    return result;
  } /* refResult */



listType refSctToList (const const_objectType aReference)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* refSctToList */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != STRUCTOBJECT)) {
      logError(printf("refSctToList(");
               trace1(aReference);
               printf("): Category is not STRUCTOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = struct_to_list(take_struct(aReference), &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* refSctToList */



/**
 *  Set the category of 'aReference' to 'aCategory'.
 *  @exception RANGE_ERROR If 'aReference' is NIL.
 */
void refSetCategory (objectType aReference, intType aCategory)

  { /* refSetCategory */
    logFunction(printf("refSetCategory(");
                trace1(aReference);
                printf(", " FMT_D ")\n", aCategory););
    if (unlikely(aReference == NULL)) {
      logError(printf("refSetCategory(NULL, " FMT_D "): Object is NULL.\n",
                      aCategory););
      raise_error(RANGE_ERROR);
    } else {
      SET_CATEGORY_OF_OBJ(aReference, aCategory);
    } /* if */
  } /* refSetCategory */



/**
 *  Set the formal parameters of 'funcRef' to 'params'.
 *  @exception RANGE_ERROR If 'funcRef' is NIL.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
void refSetParams (objectType funcRef, const_listType params)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* refSetParams */
    logFunction(printf("refSetParams(");
                trace1(funcRef);
                printf(", ");
                prot_list(params);
                printf(")\n"););
    if (unlikely(funcRef == NULL)) {
      logError(printf("refSetParams(NULL, " FMT_U_MEM "): Object is NULL.\n",
                      (memSizeType) params););
      raise_error(RANGE_ERROR);
    } else {
      if (HAS_PROPERTY(funcRef)) {
        free_list(funcRef->descriptor.property->params);
        funcRef->descriptor.property->params = copy_list(params, &err_info);
      } /* if */
      if (CATEGORY_OF_OBJ(funcRef) == BLOCKOBJECT) {
        funcRef->value.blockValue->params =
            get_param_list(params, &err_info);
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
  } /* refSetParams */



/**
 *  Set the type of 'aReference' to 'aType'.
 *  @exception RANGE_ERROR If 'aReference' is NIL.
 */
void refSetType (objectType aReference, typeType aType)

  { /* refSetType */
    if (unlikely(aReference == NULL)) {
      logError(printf("refSetType(NULL, " FMT_U_MEM "): Object is NULL.\n",
                      (memSizeType) aType););
      raise_error(RANGE_ERROR);
    } else {
      aReference->type_of = aType;
    } /* if */
  } /* refSetType */



/**
 *  Set var flag of a referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL.
 */
void refSetVar (objectType aReference, boolType var_flag)

  { /* refSetVar */
    if (unlikely(aReference == NULL)) {
      logError(printf("refSetVar(NULL, %s): Object is NULL.\n",
                      var_flag ? "TRUE" : "FALSE"););
      raise_error(RANGE_ERROR);
    } else if (var_flag) {
      SET_VAR_FLAG(aReference);
    } else {
      CLEAR_VAR_FLAG(aReference);
    } /* if */
  } /* refSetVar */



striType refStr (const const_objectType aReference)

  {
    const_cstriType stri;
    memSizeType buffer_len;
    char *buffer = NULL;
    listType name_elem;
    objectType param_obj;
    errInfoType err_info = OKAY_NO_ERROR;
    striType result;

  /* refStr */
    if (aReference == NULL) {
      stri = " *NULL_OBJECT* ";
    } else if (HAS_POSINFO(aReference)) {
      if (aReference->type_of != NULL &&
          aReference->type_of->owningProg != NULL) {
        stri = (const_cstriType) get_file_name_ustri(
            aReference->type_of->owningProg, GET_FILE_NUM(aReference));
      } else {
        stri = "?";
      } /* if */
      buffer_len = (memSizeType) strlen(stri) + 32;
      if (unlikely(!ALLOC_CSTRI(buffer, buffer_len))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        sprintf(buffer, "%s(%u)", stri, GET_LINE_NUM(aReference));
        stri = buffer;
      } /* if */
    } else if (!HAS_ENTITY(aReference)) {
      stri = " *NULL_ENTITY_OBJECT* ";
    } else if (GET_ENTITY(aReference)->ident != NULL) {
      stri = id_string2(GET_ENTITY(aReference)->ident);
    } else {
      stri = NULL;
      name_elem = GET_ENTITY(aReference)->fparam_list;
      while (name_elem != NULL && stri == NULL) {
        if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
          param_obj = name_elem->obj->value.objValue;
          if (CATEGORY_OF_OBJ(param_obj) != VALUEPARAMOBJECT &&
              CATEGORY_OF_OBJ(param_obj) != REFPARAMOBJECT &&
              CATEGORY_OF_OBJ(param_obj) != TYPEOBJECT) {
            stri = id_string2(GET_ENTITY(param_obj)->ident);
          } /* if */
        } else {
          stri = id_string2(GET_ENTITY(name_elem->obj)->ident);
        } /* if */
        name_elem = name_elem->next;
      } /* while */
      if (stri == NULL) {
        stri = " *UNKNOWN_NAME* ";
      } /* if */
    } /* if */
    result = cstri8_to_stri(stri, &err_info);
    if (buffer != NULL) {
      UNALLOC_CSTRI(buffer, buffer_len);
    } /* if */
    if (unlikely(result == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* refStr */



/**
 *  Get the type of the referenced object.
 *  @return the type of the object referenced by 'aReference'.
 */
typeType refType (const const_objectType aReference)

  {
    typeType result;

  /* refType */
    logFunction(printf("refType(");
                trace1(aReference);
                printf(")\n"););
    if (unlikely(aReference == NULL ||
                 aReference->type_of == NULL)) {
      logError(printf("refType(NULL): Object is NULL.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = aReference->type_of;
    } /* if */
    logFunction(printf("refType --> " FMT_X_MEM "\n", (memSizeType) result););
    return result;
  } /* refType */



/**
 *  Get 'ACTION' value of the object referenced by 'aReference'.
 *  @return the 'ACTION' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> ACTOBJECT holds.
 */
actType actValue (const const_objectType aReference)

  { /* actValue */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != ACTOBJECT)) {
      logError(printf("actValue(");
               trace1(aReference);
               printf("): Category is not ACTOBJECT.\n"););
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return take_action(aReference);
    } /* if */
  } /* actValue */



/**
 *  Get 'bigInteger' value of the object referenced by 'aReference'.
 *  @return the 'bigInteger' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> BIGINTOBJECT holds.
 */
bigIntType bigValue (const const_objectType aReference)

  { /* bigValue */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != BIGINTOBJECT)) {
      logError(printf("bigValue(");
               trace1(aReference);
               printf("): Category is not BIGINTOBJECT.\n"););
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return bigCreate(take_bigint(aReference));
    } /* if */
  } /* bigValue */



/**
 *  Get 'boolean' value of the object referenced by 'aReference'.
 *  @return the 'boolean' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             if it references neither TRUE nor FALSE.
 */
boolType blnValue (const_objectType aReference)

  { /* blnValue */
    if (aReference != NULL) {
      if (CATEGORY_OF_OBJ(aReference) == CONSTENUMOBJECT ||
          CATEGORY_OF_OBJ(aReference) == VARENUMOBJECT) {
        aReference = aReference->value.objValue;
      } /* if */
      if (aReference->type_of != NULL &&
          aReference->type_of->owningProg != NULL) {
        if (aReference == TRUE_OBJECT(aReference->type_of->owningProg)) {
          return TRUE;
        } else if (aReference == FALSE_OBJECT(aReference->type_of->owningProg)) {
          return FALSE;
        } /* if */
      } /* if */
    } /* if */
    logError(printf("blnValue(");
             trace1(aReference);
             printf("): Value neither TRUE nor FALSE.\n"););
    raise_error(RANGE_ERROR);
    return FALSE;
  } /* blnValue */



/**
 *  Get 'bstring' value of the object referenced by 'aReference'.
 *  @return the 'bstring' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> BSTRIOBJECT holds.
 */
bstriType bstValue (const const_objectType aReference)

  {
    bstriType bstri;
    bstriType result;

  /* bstValue */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != BSTRIOBJECT ||
                 (bstri = take_bstri(aReference)) == NULL)) {
      logError(printf("bstValue(");
               trace1(aReference);
               printf("): Category is not BSTRIOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, bstri->size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = bstri->size;
        memcpy(result->mem, bstri->mem, (size_t) (result->size));
      } /* if */
    } /* if */
    return result;
  } /* bstValue */



/**
 *  Get 'char' value of the object referenced by 'aReference'.
 *  @return the 'char' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> CHAROBJECT holds.
 */
charType chrValue (const const_objectType aReference)

  { /* chrValue */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != CHAROBJECT)) {
      logError(printf("chrValue(");
               trace1(aReference);
               printf("): Category is not CHAROBJECT.\n"););
      raise_error(RANGE_ERROR);
      return '\0';
    } else {
      return take_char(aReference);
    } /* if */
  } /* chrValue */



/**
 *  Get 'PRIMITIVE_WINDOW' value of the object referenced by 'aReference'.
 *  @return the 'PRIMITIVE_WINDOW' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> WINOBJECT holds.
 */
winType drwValue (const const_objectType aReference)

  {
    winType win_value;

  /* drwValue */
    logFunction(printf("drwValue(" FMT_U_MEM " (category=%d))\n",
                       (memSizeType) aReference,
                       aReference != NULL ? CATEGORY_OF_OBJ(aReference)
                                          : 0););
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != WINOBJECT)) {
      logError(printf("drwValue(");
               trace1(aReference);
               printf("): Category is not WINOBJECT.\n"););
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      win_value = take_win(aReference);
      if (win_value != NULL && win_value->usage_count != 0) {
        win_value->usage_count++;
      } /* if */
      logFunction(printf("drwValue --> " FMT_U_MEM " (usage=" FMT_U ")\n",
                         (memSizeType) win_value,
                         win_value != NULL ? win_value->usage_count
                                           : (uintType) 0););
      return win_value;
    } /* if */
  } /* drwValue */



/**
 *  Get 'clib_file' value of the object referenced by 'aReference'.
 *  @return the 'clib_file' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> FILEOBJECT holds.
 */
fileType filValue (const const_objectType aReference)

  { /* filValue */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != FILEOBJECT)) {
      logError(printf("filValue(");
               trace1(aReference);
               printf("): Category is not FILEOBJECT.\n"););
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return take_file(aReference);
    } /* if */
  } /* filValue */



/**
 *  Get 'float' value of the object referenced by 'aReference'.
 *  @return the 'float' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> FLOATOBJECT holds.
 */
floatType fltValue (const const_objectType aReference)

  { /* fltValue */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != FLOATOBJECT)) {
      logError(printf("fltValue(");
               trace1(aReference);
               printf("): Category is not FLOATOBJECT.\n"););
      raise_error(RANGE_ERROR);
      return 0.0;
    } else {
      return take_float(aReference);
    } /* if */
  } /* fltValue */



/**
 *  Get 'integer' value of the object referenced by 'aReference'.
 *  @return the 'integer' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> INTOBJECT holds.
 */
intType intValue (const const_objectType aReference)

  { /* intValue */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != INTOBJECT)) {
      logError(printf("intValue(");
               trace1(aReference);
               printf("): Category is not INTOBJECT.\n"););
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      return take_int(aReference);
    } /* if */
  } /* intValue */



/**
 *  Get 'process' value of the object referenced by 'aReference'.
 *  @return the 'process' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> PROCESSOBJECT holds.
 */
processType pcsValue (const const_objectType aReference)

  {
    processType process_value;

  /* pcsValue */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != PROCESSOBJECT)) {
      logError(printf("pcsValue(");
               trace1(aReference);
               printf("): Category is not PROCESSOBJECT.\n"););
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      process_value = take_process(aReference);
      if (process_value != NULL) {
        process_value->usage_count++;
      } /* if */
      return process_value;
    } /* if */
  } /* pcsValue */



/**
 *  Get 'pointList' value of the object referenced by 'aReference'.
 *  @return the 'pointList' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> POINTLISTOBJECT holds.
 */
bstriType pltValue (const const_objectType aReference)

  {
    bstriType plist;
    bstriType result;

  /* pltValue */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != POINTLISTOBJECT ||
                 (plist = take_pointlist(aReference)) == NULL)) {
      logError(printf("pltValue(");
               trace1(aReference);
               printf("): Category is not POINTLISTOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, plist->size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = plist->size;
        memcpy(result->mem, plist->mem, (size_t) (result->size));
      } /* if */
    } /* if */
    return result;
  } /* pltValue */



/**
 *  Get 'pollData' value of the object referenced by 'aReference'.
 *  @return the 'pollData' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> POLLOBJECT holds.
 */
pollType polValue (const const_objectType aReference)

  { /* polValue */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != POLLOBJECT)) {
      logError(printf("polValue(");
               trace1(aReference);
               printf("): Category is not POLLOBJECT.\n"););
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return polCreate(take_poll(aReference));
    } /* if */
  } /* polValue */



/**
 *  Get 'program' value of the object referenced by 'aReference'.
 *  @return the 'program' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> PROGOBJECT holds.
 */
progType prgValue (const const_objectType aReference)

  { /* prgValue */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != PROGOBJECT)) {
      logError(printf("prgValue(");
               trace1(aReference);
               printf("): Category is not PROGOBJECT.\n"););
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return take_prog(aReference);
    } /* if */
  } /* prgValue */



objectType refValue (const const_objectType aReference)

  { /* refValue */
    if (likely(aReference != NULL &&
               (CATEGORY_OF_OBJ(aReference) == FWDREFOBJECT ||
                CATEGORY_OF_OBJ(aReference) == REFOBJECT ||
                CATEGORY_OF_OBJ(aReference) == STRUCTELEMOBJECT ||
                CATEGORY_OF_OBJ(aReference) == REFPARAMOBJECT ||
                CATEGORY_OF_OBJ(aReference) == RESULTOBJECT ||
                CATEGORY_OF_OBJ(aReference) == LOCALVOBJECT ||
                CATEGORY_OF_OBJ(aReference) == ENUMLITERALOBJECT ||
                CATEGORY_OF_OBJ(aReference) == CONSTENUMOBJECT ||
                CATEGORY_OF_OBJ(aReference) == VARENUMOBJECT))) {
      return take_reference(aReference);
    } else {
      logError(printf("refValue(");
               trace1(aReference);
               printf("): Category is not a reference object.\n"););
      raise_error(RANGE_ERROR);
      return NULL;
    } /* if */
  } /* refValue */



listType rflValue (const const_objectType aReference)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* rflValue */
    if (likely(aReference != NULL &&
               (CATEGORY_OF_OBJ(aReference) == MATCHOBJECT ||
                CATEGORY_OF_OBJ(aReference) == CALLOBJECT ||
                CATEGORY_OF_OBJ(aReference) == REFLISTOBJECT))) {
      result = copy_list(take_reflist(aReference), &err_info);
      if (unlikely(result == NULL && err_info != OKAY_NO_ERROR)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } else {
      logError(printf("rflValue(");
               trace1(aReference);
               printf("): Category is not a reference list object.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } /* if */
    return result;
  } /* rflValue */



void rflSetValue (objectType dest, listType source)

  {
    listType help_list;
    errInfoType err_info = OKAY_NO_ERROR;

  /* rflSetValue */
    if (likely(CATEGORY_OF_OBJ(dest) == MATCHOBJECT ||
               CATEGORY_OF_OBJ(dest) == CALLOBJECT ||
               CATEGORY_OF_OBJ(dest) == REFLISTOBJECT)) {
      help_list = copy_list(source, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(MEMORY_ERROR);
      } else {
        free_list(take_reflist(dest));
        dest->value.listValue = help_list;
      } /* if */
    } else {
      logError(printf("rflSetValue(");
               trace1(dest);
               printf(", *): Category is not a reference list object.\n"););
      raise_error(RANGE_ERROR);
    } /* if */
  } /* rflSetValue */



/**
 *  Get 'bitset' value of the object referenced by 'aReference'.
 *  @return the 'bitset' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> SETOBJECT holds.
 */
setType setValue (const const_objectType aReference)

  {
    setType set1;
    memSizeType set_size;
    setType result;

  /* setValue */
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != SETOBJECT ||
                 (set1 = take_set(aReference)) == NULL)) {
      logError(printf("setValue(");
               trace1(aReference);
               printf("): Category is not SETOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      set_size = bitsetSize(set1);
      if (unlikely(!ALLOC_SET(result, set_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = set1->min_position;
        result->max_position = set1->max_position;
        memcpy(result->bitset, set1->bitset, set_size * sizeof(bitSetType));
      } /* if */
    } /* if */
    return result;
  } /* setValue */



/**
 *  Get 'string' value of the object referenced by 'aReference'.
 *  @return the 'string' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> STRIOBJECT holds.
 */
striType strValue (const const_objectType aReference)

  {
    striType stri;
    striType result;

  /* strValue */
    logFunction(printf("strValue(");
                trace1(aReference);
                printf(")\n"););
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != STRIOBJECT ||
                 (stri = take_stri(aReference)) == NULL)) {
      logError(printf("strValue(");
               trace1(aReference);
               printf("): Category is not STRIOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, stri->size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = stri->size;
        memcpy(result->mem, stri->mem,
               (size_t) (result->size * sizeof(strElemType)));
      } /* if */
    } /* if */
    logFunction(printf("strValue --> \"%s\"\n",
                       striAsUnquotedCStri(result)););
    return result;
  } /* strValue */



/**
 *  Get 'string' value of the object referenced by 'aReference'.
 *  @return the 'string' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> STRIOBJECT holds.
 */
const_striType strValueRef (const const_objectType aReference)

  {
    striType stri;
    striType result;

  /* strValueRef */
    logFunction(printf("strValueRef(");
                trace1(aReference);
                printf(")\n"););
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != STRIOBJECT ||
                 (stri = take_stri(aReference)) == NULL)) {
      logError(printf("strValueRef(");
               trace1(aReference);
               printf("): Category is not STRIOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = stri;
    } /* if */
    logFunction(printf("strValueRef --> \"%s\"\n",
                       striAsUnquotedCStri(result)););
    return result;
  } /* strValueRef */



/**
 *  Get 'type' value of the object referenced by 'aReference'.
 *  @return the 'type' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference' is NIL or
 *             category(aReference) <> TYPEOBJECT holds.
 */
typeType typValue (const const_objectType aReference)

  {
    typeType result;

  /* typValue */
    logFunction(printf("refValue(");
                trace1(aReference);
                printf(")\n"););
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != TYPEOBJECT)) {
      logError(printf("typValue(");
               trace1(aReference);
               printf("): Category is not TYPEOBJECT.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = take_type(aReference);
    } /* if */
    logFunction(printf("typValue --> " FMT_X_MEM "\n", (memSizeType) result););
    return result;
  } /* typValue */
