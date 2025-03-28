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



objectType ref_addr (listType arguments)

  { /* ref_addr */
    logFunction(printf("ref_addr(");
                trace1(arg_2(arguments));
                printf(")\n"););
    return bld_reference_temp(arg_2(arguments));
  } /* ref_addr */



objectType ref_alloc (listType arguments)

  {
    objectType obj1;
    objectType created_object;

  /* ref_alloc */
    isit_reference(arg_1(arguments));
    obj1 = take_reference(arg_1(arguments));
    logFunction(printf("ref_alloc(");
                trace1(obj1);
                printf(")\n"););
    if (ALLOC_OBJECT(created_object)) {
      created_object->type_of = obj1->type_of;
      memcpy(&created_object->descriptor, &obj1->descriptor,
          sizeof(descriptorUnion));
      /* Copies the POSINFO flag (and all other flags): */
      INIT_CATEGORY_OF_OBJ(created_object, obj1->objcategory);
      created_object->value.objValue = NULL;
      return bld_reference_temp(created_object);
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
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



objectType ref_alloc_var (listType arguments)

  { /* ref_alloc_var */
    isit_type(arg_1(arguments));
    isit_int(arg_2(arguments));
    logFunction(printf("ref_alloc_var(");
                printtype(take_type(arg_1(arguments)));
                printf(", ");
                printcategory(take_int(arg_2(arguments)));
                printf(")\n"););
    return bld_reference_temp(
        refAllocVar(take_type(arg_1(arguments)),
                    take_int(arg_2(arguments))));
  } /* ref_alloc_var */



/**
 *  Obtain the maximum index of the array referenced by arrayRef/arg_1.
 *  @param arrayRef Reference to an array object.
 *  @return the maximum index of the array.
 *  @exception RANGE_ERROR If arrayRef/arg_1 does not refer to an array.
 */
objectType ref_arrmaxidx (listType arguments)

  { /* ref_arrmaxidx */
    isit_reference(arg_1(arguments));
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



objectType ref_cast (listType arguments)

  { /* ref_cast */
    /* The reference value is taken as int on purpose */
    return bld_int_temp(take_int(arg_3(arguments)));
  } /* ref_cast */



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



objectType ref_content (listType arguments)

  {
    objectType obj_arg1;
    listType result;

  /* ref_content */
    obj_arg1 = arg_1(arguments);
    isit_reference(obj_arg1);
    result = NULL;
    return bld_reflist_temp(result);
  } /* ref_content */



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



objectType ref_deref (listType arguments)

  { /* ref_deref */
    isit_reference(arg_1(arguments));
    return take_reference(arg_1(arguments));
  } /* ref_deref */



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
    objectType symb_object;

  /* ref_issymb */
    isit_reference(arg_1(arguments));
    symb_object = take_reference(arg_1(arguments));
    logFunction(printf("ref_issymb(" FMT_U_MEM " ",
                       (memSizeType) GET_ENTITY(symb_object));
                trace1(symb_object);
                printf(")\n"););
    if (HAS_ENTITY(symb_object) &&
        GET_ENTITY(symb_object)->syobject != NULL) {
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



objectType ref_mkref (listType arguments)

  {
    objectType refe_to;
    objectType refe_from;

  /* ref_mkref */
    refe_to = arg_1(arguments);
    refe_from = arg_3(arguments);
    isit_reference(refe_to);
/*  is_variable(refe_to); */
/*
printf("ASSIGN ");
trace2(arg_3(arguments));
printf("\nTO ");
trace2(refe_to);
*/
/* FIX !!!!! @@@@@ ##### $$$$$ %%%%% ^^^^^ &&&&& */
    refe_to->value.objValue = refe_from;
    return SYS_EMPTY_OBJECT;
  } /* ref_mkref */



/**
 *  Check if two references are equal.
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



objectType ref_prog (listType arguments)

  { /* ref_prog */
    return bld_reference_temp(NULL);
  } /* ref_prog */



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



objectType ref_scan (listType arguments)

  {
    striType str1;
    objectType obj_variable;
    cstriType name;
    identType ident_found;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* ref_scan */
    isit_stri(arg_1(arguments));
    str1 = take_stri(arg_1(arguments));
    obj_variable = arg_2(arguments);
    isit_reference(obj_variable);
    is_variable(obj_variable);
    name = stri_to_cstri8(str1, &err_info);
    if (name == NULL) {
      raise_error(err_info);
      result = NULL;
    } else {
      ident_found = get_ident(prog, (const_ustriType) name);
      free_cstri8(name, str1);
      if (ident_found == NULL ||
          ident_found->entity == NULL ||
          ident_found->entity->data.owner == NULL) {
        result = raise_exception(SYS_MEM_EXCEPTION);
      } else {
        if (ident_found->entity->data.owner->obj != NULL) {
          obj_variable->value.objValue = ident_found->entity->data.owner->obj;
          result = SYS_TRUE_OBJECT;
        } else {
          obj_variable->value.objValue = ident_found->entity->syobject;
          result = SYS_TRUE_OBJECT;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* ref_scan */



objectType ref_scttolist (listType arguments)

  { /* ref_scttolist */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refSctToList(
        take_reference(arg_1(arguments))));
  } /* ref_scttolist */



objectType ref_select (listType arguments)

  {
    objectType refer;
    structType stru1;
    objectType selector;
    objectType selector_syobject;
    memSizeType position;
    objectType struct_pointer;
    objectType result;

  /* ref_select */
    isit_reference(arg_1(arguments));
    refer = take_reference(arg_1(arguments));
    if (refer != NULL) {
      isit_struct(refer);
      stru1 = take_struct(refer);
      selector = arg_3(arguments);
/*
printf("stru1 ");
trace1(arg_1(arguments));
printf("\n");
printf("selector ");
trace1(selector);
printf("\n");
*/
      if (HAS_ENTITY(selector) && GET_ENTITY(selector)->syobject != NULL) {
        selector_syobject = GET_ENTITY(selector)->syobject;
        position = stru1->size;
        struct_pointer = &stru1->stru[position - 1];
        while (position > 0) {
/*
printf("test " FMT_U_MEM ": ", position);
trace1(struct_pointer);
printf("\n");
*/
          if (HAS_ENTITY(struct_pointer) &&
              GET_ENTITY(struct_pointer)->syobject == selector_syobject) {
            if (TEMP_OBJECT(arg_1(arguments))) {
/*
              printf("ref_select of TEMP_OBJECT\n");
              printf("stru1 ");
              trace1(arg_1(arguments));
              printf("\n");
              printf("selector ");
              trace1(selector);
              printf("\n");
*/
              /* The struct will be destroyed after selecting. */
              /* A copy is necessary here to avoid a crash !!!!! */
              if (!ALLOC_OBJECT(result)) {
                result = raise_exception(SYS_MEM_EXCEPTION);
              } else {
                memcpy(result, struct_pointer, sizeof(objectRecord));
                SET_TEMP_FLAG(result);
                destr_struct(stru1->stru,
                    (memSizeType) (struct_pointer - stru1->stru));
                destr_struct(&struct_pointer[1],
                    (stru1->size - (memSizeType) (struct_pointer - stru1->stru) - 1));
                FREE_STRUCT(stru1, stru1->size);
                arg_1(arguments)->value.structValue = NULL;
              } /* if */
              return result;
            } else {
              return struct_pointer;
            } /* if */
          } /* if */
          position--;
          struct_pointer--;
        } /* while */
      } /* if */
    } /* if */
    logError(printf("ref_select(");
             trace1(arg_1(arguments));
             printf(", ");
             trace1(arg_3(arguments));
             printf("): Selector not found.\n"););
    return raise_exception(SYS_RNG_EXCEPTION);
  } /* ref_select */



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



objectType ref_symb (listType arguments)

  {
    objectType symb_object;

  /* ref_symb */
    isit_reference(arg_2(arguments));
    symb_object = take_reference(arg_2(arguments));
    logFunction(printf("ref_symb(" FMT_U_MEM " ",
                       (memSizeType) GET_ENTITY(symb_object));
                trace1(symb_object);
                printf(")\n"););
    if (HAS_ENTITY(symb_object) &&
        GET_ENTITY(symb_object)->syobject != NULL) {
      symb_object = GET_ENTITY(symb_object)->syobject;
    } else {
      logError(printf("ref symb (" FMT_U_MEM " ",
                      (memSizeType) GET_ENTITY(symb_object));
               trace1(symb_object);
               printf("): Error\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    logFunction(printf("ref_symb --> " FMT_U_MEM " ",
                       (memSizeType) GET_ENTITY(symb_object));
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
 *  @return the type of the object referenced by aReference/arg_1.
 */
objectType ref_type (listType arguments)

  {
    objectType obj_arg;

  /* ref_type */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL) {
      logError(printf("ref_type(0): Null argument.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else if (obj_arg->type_of == NULL) {
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
