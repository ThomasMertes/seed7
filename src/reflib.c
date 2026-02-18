/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2007, 2010, 2012, 2013  Thomas Mertes      */
/*                2015, 2016, 2019 - 2021, 2024  Thomas Mertes      */
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
/*  Module: Library                                                 */
/*  File: seed7/src/reflib.c                                        */
/*  Changes: 1991 - 1994, 2004, 2007, 2010, 2012  Thomas Mertes     */
/*           2013, 2015, 2016, 2019 - 2021, 2024  Thomas Mertes     */
/*           2025  Thomas Mertes                                    */
/*  Content: All primitive actions for the reference type.          */
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
#include "heaputl.h"
#include "flistutl.h"
#include "striutl.h"
#include "listutl.h"
#include "syvarutl.h"
#include "identutl.h"
#include "entutl.h"
#include "blockutl.h"
#include "executl.h"
#include "objutl.h"
#include "findid.h"
#include "traceutl.h"
#include "exec.h"
#include "runerr.h"
#include "name.h"
#include "infile.h"
#include "str_rtl.h"
#include "ref_data.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "reflib.h"



objectType ref_alloc (listType arguments)

  { /* ref_alloc */
    isit_reference(arg_1(arguments));
    logFunction(printf("ref_alloc(" FMT_U_MEM " ",
                       (memSizeType) take_reference(arg_1(arguments)));
                trace1(take_reference(arg_1(arguments)));
                printf(")\n"););
    return bld_reference_temp(
        refAlloc(take_reference(arg_1(arguments))));
  } /* ref_alloc */



objectType ref_alloc_int (listType arguments)

  { /* ref_alloc_int */
    isit_bool(arg_1(arguments));
    isit_type(arg_2(arguments));
    isit_int(arg_3(arguments));
    logFunction(printf("ref_alloc_int(%s, ",
                       take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT ?
                       "TRUE" : "FALSE");
                printtype(take_type(arg_2(arguments)));
                printf(", " FMT_D ")\n",
                       take_int(arg_3(arguments))););
    return bld_reference_temp(
        refAllocInt(take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT,
                     take_type(arg_2(arguments)),
                     take_int(arg_3(arguments))));
  } /* ref_alloc_int */



objectType ref_alloc_stri (listType arguments)

  { /* ref_alloc_stri */
    isit_bool(arg_1(arguments));
    isit_type(arg_2(arguments));
    isit_stri(arg_3(arguments));
    logFunction(printf("ref_alloc_stri(%s, ",
                       take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT ?
                       "TRUE" : "FALSE");
                printtype(take_type(arg_2(arguments)));
                printf(", \"%s\")\n", striAsUnquotedCStri(
                       take_stri(arg_3(arguments)))););
    return bld_reference_temp(
        refAllocStri(take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT,
                     take_type(arg_2(arguments)),
                     take_stri(arg_3(arguments))));
  } /* ref_alloc_stri */



/**
 *  Append params/arg_2 to the formal parameters of funcRef/arg_1.
 *  @exception RANGE_ERROR If funcRef/arg_1 is NIL.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
objectType ref_append_params (listType arguments)

  { /* ref_append_params */
    isit_reference(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    refAppendParams(take_reference(arg_1(arguments)), take_reflist(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* ref_append_params */



/**
 *  Obtain the maximum index of the array referenced by arrayRef/arg_1.
 *  @param arrayRef Reference to an array object.
 *  @return the maximum index of the array.
 *  @exception RANGE_ERROR If arrayRef/arg_1 does not refer to an array.
 */
objectType ref_arrmaxidx (listType arguments)

  { /* ref_arrmaxidx */
    isit_reference(arg_1(arguments));
    logFunction(printf("ref_arrmaxidx(");
                trace1(take_reference(arg_1(arguments)));
                printf(")\n"););
    return bld_int_temp(refArrMaxIdx(
        take_reference(arg_1(arguments))));
  } /* ref_arrmaxidx */



/**
 *  Obtain the minimum index of the array referenced by arrayRef/arg_1.
 *  @param arrayRef Reference to an array object.
 *  @return the minimum index of the array.
 *  @exception RANGE_ERROR If arrayRef/arg_1 does not refer to an array.
 */
objectType ref_arrminidx (listType arguments)

  { /* ref_arrminidx */
    isit_reference(arg_1(arguments));
    logFunction(printf("ref_arrminidx(");
                trace1(take_reference(arg_1(arguments)));
                printf(")\n"););
    return bld_int_temp(refArrMinIdx(
        take_reference(arg_1(arguments))));
  } /* ref_arrminidx */



objectType ref_arrtolist (listType arguments)

  { /* ref_arrtolist */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refArrToList(
        take_reference(arg_1(arguments))));
  } /* ref_arrtolist */



/**
 *  Gets the body of the function referenced by funcRef/arg_1.
 *  @return the body expression of funcRef/arg_1.
 *  @exception RANGE_ERROR If funcRef/arg_1 is NIL or
 *                         category(funcRef) <> BLOCKOBJECT holds.
 */
objectType ref_body (listType arguments)

  { /* ref_body */
    isit_reference(arg_1(arguments));
    return bld_reference_temp(refBody(
        take_reference(arg_1(arguments))));
  } /* ref_body */



/**
 *  Get the category of a referenced object.
 *  @return the category of the referenced object.
 *  @exception RANGE_ERROR If aReference/arg_1 is NIL.
 */
objectType ref_category (listType arguments)

  { /* ref_category */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refCategory(
        take_reference(arg_1(arguments))));
  } /* ref_category */



/**
 *  Convert a string to a 'category'.
 *  @param catName/arg_3 Name of a category to be converted.
 *  @return the 'category' result fo the conversion.
 *  @exception RANGE_ERROR If there is no corresponding 'category'.
 */
objectType ref_cat_parse (listType arguments)

  { /* ref_cat_parse */
    isit_stri(arg_1(arguments));
    return bld_int_temp(refCatParse(
        take_stri(arg_1(arguments))));
  } /* ref_cat_parse */



/**
 *  Convert a category to a string.
 *  @param aCategory/arg_1 Category to be converted.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType ref_cat_str (listType arguments)

  { /* ref_cat_str */
    isit_int(arg_1(arguments));
    return bld_stri_temp(refCatStr(
        take_int(arg_1(arguments))));
  } /* ref_cat_str */



/**
 *  Compare two references.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objectType ref_cmp (listType arguments)

  {
    memSizeType ref1;
    memSizeType ref2;
    intType signumValue;

  /* ref_cmp */
    isit_reference(arg_1(arguments));
    isit_reference(arg_2(arguments));
    ref1 = (memSizeType) take_reference(arg_1(arguments));
    ref2 = (memSizeType) take_reference(arg_2(arguments));
    if (ref1 < ref2) {
      signumValue = -1;
    } else {
      signumValue = ref1 > ref2;
    } /* if */
    return bld_int_temp(signumValue);
  } /* ref_cmp */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType ref_cpy (listType arguments)

  {
    objectType dest;

  /* ref_cpy */
    dest = arg_1(arguments);
    isit_reference(dest);
    is_variable(dest);
    isit_reference(arg_3(arguments));
    dest->value.objValue = take_reference(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* ref_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType ref_create (listType arguments)

  {
    objectType dest;
    objectType source;

  /* ref_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_reference(source);
    SET_CATEGORY_OF_OBJ(dest, REFOBJECT);
    dest->value.objValue = take_reference(source);
    return SYS_EMPTY_OBJECT;
  } /* ref_create */



/**
 *  Check if two references are equal.
 *  @return TRUE if both references are equal,
 *          FALSE otherwise.
 */
objectType ref_eq (listType arguments)

  { /* ref_eq */
    isit_reference(arg_1(arguments));
    isit_reference(arg_3(arguments));
    if (take_reference(arg_1(arguments)) ==
        take_reference(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* ref_eq */



/**
 *  Determine the file name of a referenced object.
 *  @return the file name of the referenced object.
 *  @exception RANGE_ERROR If aReference/arg_1 is NIL.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType ref_file (listType arguments)

  {
    const_striType name;
    objectType result;

  /* ref_file */
    isit_reference(arg_1(arguments));
    name = refFile(take_reference(arg_1(arguments)));
    if (name == NULL) {
      result = NULL;
    } else {
      result = bld_stri_temp(strCreate(name));
    } /* if */
    return result;
  } /* ref_file */



objectType ref_getref (listType arguments)

  { /* ref_getref */
    return bld_reference_temp(arg_1(arguments));
  } /* ref_getref */



/**
 *  Compute the hash value of a 'reference'.
 *  @return the hash value.
 */
objectType ref_hashcode (listType arguments)

  { /* ref_hashcode */
    isit_reference(arg_1(arguments));
    return bld_int_temp((intType)
        (((memSizeType) take_reference(arg_1(arguments))) >> 6));
  } /* ref_hashcode */



objectType ref_hshdatatolist (listType arguments)

  { /* ref_hshdatatolist */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refHshDataToList(
        take_reference(arg_1(arguments))));
  } /* ref_hshdatatolist */



objectType ref_hshkeystolist (listType arguments)

  { /* ref_hshkeystolist */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refHshKeysToList(
        take_reference(arg_1(arguments))));
  } /* ref_hshkeystolist */



objectType ref_hshlength (listType arguments)

  { /* ref_hshlength */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refHshLength(
        take_reference(arg_1(arguments))));
  } /* ref_hshlength */



objectType ref_issymb (listType arguments)

  {
    objectType obj_arg;

  /* ref_issymb */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    logFunction(printf("ref_issymb(");
                trace1(obj_arg);
                printf(")\n"););
    if (unlikely(obj_arg == NULL)) {
      logError(printf("ref_issymb: Null argument.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else if (HAS_ENTITY(obj_arg) &&
               GET_ENTITY(obj_arg)->syobject != NULL) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* ref_issymb */



/**
 *  Determine if the referenced object is temporary.
 *  @return TRUE if aReference/arg_1 is temporary, FALSE otherwise.
 *  @exception RANGE_ERROR If aReference/arg_1 is NIL.
 */
objectType ref_istemp (listType arguments)

  { /* ref_istemp */
    isit_reference(arg_1(arguments));
    if (refIsTemp(take_reference(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* ref_istemp */



/**
 *  Determine if the referenced object is variable.
 *  @return TRUE if aReference/arg_1 is a variable, FALSE otherwise.
 *  @exception RANGE_ERROR If aReference/arg_1 is NIL.
 */
objectType ref_isvar (listType arguments)

  { /* ref_isvar */
    isit_reference(arg_1(arguments));
    if (refIsVar(take_reference(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* ref_isvar */



objectType ref_itftosct (listType arguments)

  { /* ref_itftosct */
    isit_reference(arg_1(arguments));
    return bld_reference_temp(refItfToSct(
        take_reference(arg_1(arguments))));
  } /* ref_itftosct */



/**
 *  Determine the line number of a referenced object.
 *  @return the line number of the referenced object.
 *  @exception RANGE_ERROR If aReference/arg_1 is NIL.
 */
objectType ref_line (listType arguments)

  { /* ref_line */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refLine(
        take_reference(arg_1(arguments))));
  } /* ref_line */



/**
 *  Gets the local constants of funcRef/arg_1.
 *  @return the local constants as ref_list.
 *  @exception RANGE_ERROR If funcRef/arg_1 is NIL or
 *                         refCategory(funcRef) <> BLOCKOBJECT holds.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
objectType ref_local_consts (listType arguments)

  { /* ref_local_consts */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refLocalConsts(
        take_reference(arg_1(arguments))));
  } /* ref_local_consts */



/**
 *  Gets the local variables of funcRef/arg_1.
 *  @return the local variables as ref_list.
 *  @exception RANGE_ERROR If funcRef/arg_1 is NIL or
 *                         refCategory(funcRef) <> BLOCKOBJECT holds.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
objectType ref_local_vars (listType arguments)

  { /* ref_local_vars */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refLocalVars(
        take_reference(arg_1(arguments))));
  } /* ref_local_vars */



/**
 *  Check if two references are not equal.
 *  @return FALSE if both references are equal,
 *          TRUE otherwise.
 */
objectType ref_ne (listType arguments)

  { /* ref_ne */
    isit_reference(arg_1(arguments));
    isit_reference(arg_3(arguments));
    if (take_reference(arg_1(arguments)) !=
        take_reference(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* ref_ne */



objectType ref_nil (listType arguments)

  { /* ref_nil */
    return bld_reference_temp(NULL);
  } /* ref_nil */



/**
 *  Delivers an unique number for each object
 *  @return a unique object number.
 *  @exception RANGE_ERROR The type has no owning program or number map.
 *  @exception MEMORY_ERROR Not enough memory to maintain the object table.
 */
objectType ref_num (listType arguments)

  { /* ref_num */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refNum(
        take_reference(arg_1(arguments))));
  } /* ref_num */



/**
 *  Get the formal parameters of the function referenced by 'funcRef'.
 *  For objects without parameters an empty list is returned.
 *  @return the formal parameters as ref_list.
 *  @exception RANGE_ERROR If 'funcRef' is NIL.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
objectType ref_params (listType arguments)

  {
    listType params;
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* ref_params */
    isit_reference(arg_1(arguments));
    params = refParams(take_reference(arg_1(arguments)));
    result = copy_list(params, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_reflist_temp(result);
    } /* if */
  } /* ref_params */



/**
 *  Determine the absolute file path of a referenced object.
 *  @return the absolute file path of the referenced object.
 *  @exception RANGE_ERROR If aReference/arg_1 is NIL.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType ref_path (listType arguments)

  {
    const_striType path;
    objectType result;

  /* ref_path */
    isit_reference(arg_1(arguments));
    path = refPath(take_reference(arg_1(arguments)));
    if (path == NULL) {
      result = NULL;
    } else {
      result = bld_stri_temp(strCreate(path));
    } /* if */
    return result;
  } /* ref_path */



/**
 *  Gets the initialization value of the result variable of funcRef/arg_1.
 *  @return a reference to the initialization value.
 *  @exception RANGE_ERROR If funcRef/arg_1 is NIL or
 *                         refCategory(funcRef) <> BLOCKOBJECT holds.
 */
objectType ref_resini (listType arguments)

  { /* ref_resini */
    isit_reference(arg_1(arguments));
    return bld_reference_temp(refResini(
        take_reference(arg_1(arguments))));
  } /* ref_resini */



/**
 *  Gets the result variable of funcRef/arg_1.
 *  @return a reference to the result variable.
 *  @exception RANGE_ERROR If funcRef/arg_1 is NIL or
 *                         refCategory(funcRef) <> BLOCKOBJECT holds.
 */
objectType ref_result (listType arguments)

  { /* ref_result */
    isit_reference(arg_1(arguments));
    return bld_reference_temp(refResult(
        take_reference(arg_1(arguments))));
  } /* ref_result */



objectType ref_scttolist (listType arguments)

  { /* ref_scttolist */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refSctToList(
        take_reference(arg_1(arguments))));
  } /* ref_scttolist */



/**
 *  Set the category of aReference/arg_1 to aCategory/arg_2.
 *  Set the category of a referenced object.
 *  @exception RANGE_ERROR If aReference/arg_1 is NIL.
 */
objectType ref_setcategory (listType arguments)

  { /* ref_setcategory */
    isit_reference(arg_1(arguments));
    isit_int(arg_2(arguments));
    refSetCategory(take_reference(arg_1(arguments)),
                   take_int(      arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* ref_setcategory */



/**
 *  Set the formal parameters of funcRef/arg_1 to params/arg_2.
 *  @exception RANGE_ERROR If funcRef/arg_1 is NIL.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
objectType ref_setparams (listType arguments)

  { /* ref_setparams */
    isit_reference(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    refSetParams(take_reference(arg_1(arguments)), take_reflist(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* ref_setparams */



/**
 *  Set the type of aReference/arg_1 to aType/arg_2.
 *  @exception RANGE_ERROR If aReference/arg_1 is NIL.
 */
objectType ref_settype (listType arguments)

  { /* ref_settype */
    isit_reference(arg_1(arguments));
    isit_type(arg_2(arguments));
    refSetType(take_reference(arg_1(arguments)),
               take_type(     arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* ref_settype */



/**
 *  Set var flag of a referenced object.
 *  @exception RANGE_ERROR If aReference/arg_1 is NIL.
 */
objectType ref_setvar (listType arguments)

  { /* ref_setvar */
    isit_reference(arg_1(arguments));
    isit_bool(arg_2(arguments));
    if (take_bool(arg_2(arguments)) == SYS_TRUE_OBJECT) {
      refSetVar(take_reference(arg_1(arguments)), TRUE);
    } else {
      refSetVar(take_reference(arg_1(arguments)), FALSE);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* ref_setvar */



objectType ref_str (listType arguments)

  { /* ref_str */
    isit_reference(arg_1(arguments));
    return bld_stri_temp(refStr(
        take_reference(arg_1(arguments))));
  } /* ref_str */



/**
 *  Convert a 'reference' into a symbol parameter.
 *  @return a symbol parameter which corresponds to obj_arg/arg_2.
 *  @exception RANGE_ERROR If obj_arg/arg_2 is NIL or
 *                         it has no symbol object.
 */
objectType ref_symb (listType arguments)

  {
    objectType obj_arg;
    objectType symb_object;

  /* ref_symb */
    isit_reference(arg_2(arguments));
    obj_arg = take_reference(arg_2(arguments));
    logFunction(printf("ref_symb(");
                trace1(obj_arg);
                printf(")\n"););
    if (unlikely(obj_arg == NULL)) {
      logError(printf("ref_symb: Null argument.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else if (unlikely(!HAS_ENTITY(obj_arg) ||
                        GET_ENTITY(obj_arg)->syobject == NULL)) {
      logError(printf("ref symb(");
               trace1(obj_arg);
               printf("): No symbol object.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      symb_object = GET_ENTITY(obj_arg)->syobject;
    } /* if */
    logFunction(printf("ref_symb --> " FMT_U_MEM " ",
                       symb_object != NULL && HAS_ENTITY(symb_object) ?
                           (memSizeType) GET_ENTITY(symb_object) :
                           (memSizeType) 0);
                trace1(symb_object);
                printf("\n"););
    return bld_param_temp(symb_object);
  } /* ref_symb */



objectType ref_trace (listType arguments)

  { /* ref_trace */
    trace1(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* ref_trace */



/**
 *  Get the type of the referenced object.
 *  @return the type of the object referenced by obj_arg/arg_1.
 *  @exception RANGE_ERROR If obj_arg/arg_1 is NIL or
 *                         the type is NULL.
 */
objectType ref_type (listType arguments)

  {
    objectType obj_arg;

  /* ref_type */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    logFunction(printf("ref_type(");
                trace1(obj_arg);
                printf(")\n"););
    if (unlikely(obj_arg == NULL)) {
      logError(printf("ref_type: Null argument.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else if (unlikely(obj_arg->type_of == NULL)) {
      logError(printf("ref_type(" FMT_U_MEM "): Type is null.\n",
                      (memSizeType) obj_arg););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_type_temp(obj_arg->type_of);
    } /* if */
  } /* ref_type */



objectType ref_value (listType arguments)

  { /* ref_value */
    isit_reference(arg_1(arguments));
    return bld_reference_temp(refValue(
        take_reference(arg_1(arguments))));
  } /* ref_value */
