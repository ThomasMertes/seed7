/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2015  Thomas Mertes                        */
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
/*  Changes: 1991 - 1994, 2004, 2007, 2015  Thomas Mertes           */
/*  Content: All primitive actions for the reference type.          */
/*                                                                  */
/********************************************************************/

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


#define TABLE_INCREMENT 127



#ifdef OUT_OF_ORDER
static void qsort_list (listType *begin_sorted, listType *end_sorted)

  {
    listType input_list, key_element;
    listType begin_less, end_less, begin_greater, end_greater;
    ustriType key;

  /* qsort_list */
    key_element = *begin_sorted;
    input_list = key_element->next;
    key = GET_ENTITY(key_element->obj)->ident->name;
    begin_less = NULL;
    begin_greater = NULL;
    do {
      if (strcmp((cstriType) GET_ENTITY(input_list->obj)->ident->name,
          (cstriType) key) < 0) {
        if (begin_less == NULL) {
          begin_less = input_list;
          end_less = input_list;
        } else {
          end_less->next = input_list;
          end_less = input_list;
        } /* if */
        input_list = input_list->next;
      } else {
        if (begin_greater == NULL) {
          begin_greater = input_list;
          end_greater = input_list;
        } else {
          end_greater->next = input_list;
          end_greater = input_list;
        } /* if */
        input_list = input_list->next;
      } /* if */
    } while (input_list != NULL);
    if (begin_less != NULL) {
      if (begin_less != end_less) {
        end_less->next = NULL;
        qsort_list(&begin_less, &end_less);
      } /* if */
      end_less->next = key_element;
      *begin_sorted = begin_less;
    } else {
      *begin_sorted = key_element;
    } /* if */
    if (begin_greater != NULL) {
      if (begin_greater != end_greater) {
        end_greater->next = NULL;
        qsort_list(&begin_greater, &end_greater);
      } /* if */
      key_element->next = begin_greater;
      *end_sorted = end_greater;
    } else {
      *end_sorted = key_element;
    } /* if */
  } /* qsort_list */



static void sort_list (listType *any_list)

  {
    listType list_end;

  /* sort_list */
    if (*any_list != NULL && (*any_list)->next != NULL) {
      qsort_list(any_list, &list_end);
      list_end->next = NULL;
    } /* if */
  } /* sort_list */
#endif



objectType ref_addr (listType arguments)

  { /* ref_addr */
    return bld_reference_temp(arg_2(arguments));
  } /* ref_addr */



objectType ref_alloc (listType arguments)

  {
    objectType obj1;
    objectType created_object;

  /* ref_alloc */
    isit_reference(arg_1(arguments));
    obj1 = take_reference(arg_1(arguments));
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



objectType ref_alloc_stri (listType arguments)

  { /* ref_alloc_stri */
    isit_bool(arg_1(arguments));
    isit_type(arg_2(arguments));
    isit_stri(arg_3(arguments));
    return bld_reference_temp(
        refAllocStri(take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT,
                     take_type(arg_2(arguments)),
                     take_stri(arg_3(arguments))));
  } /* ref_alloc_stri */



objectType ref_arrmaxidx (listType arguments)

  { /* ref_arrmaxidx */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refArrMaxIdx(
        take_reference(arg_1(arguments))));
  } /* ref_arrmaxidx */



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



objectType ref_category (listType arguments)

  { /* ref_category */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refCategory(
        take_reference(arg_1(arguments))));
  } /* ref_category */



objectType ref_cat_parse (listType arguments)

  { /* ref_cat_parse */
    isit_stri(arg_3(arguments));
    return bld_int_temp(refCatParse(
        take_stri(arg_3(arguments))));
  } /* ref_cat_parse */



objectType ref_cat_str (listType arguments)

  { /* ref_cat_str */
    isit_int(arg_1(arguments));
    return bld_stri_temp(refCatStr(
        take_int(arg_1(arguments))));
  } /* ref_cat_str */



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
    } else if (ref1 > ref2) {
      signumValue = 1;
    } else {
      signumValue = 0;
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



objectType ref_conv (listType arguments)

  { /* ref_conv */
    isit_reference(arg_3(arguments));
    return bld_reference_temp(take_reference(arg_3(arguments)));
  } /* ref_conv */



objectType ref_cpy (listType arguments)

  {
    objectType ref_variable;

  /* ref_cpy */
    ref_variable = arg_1(arguments);
    isit_reference(ref_variable);
    is_variable(ref_variable);
    isit_reference(arg_3(arguments));
    ref_variable->value.objValue = take_reference(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* ref_cpy */



objectType ref_create (listType arguments)

  {
    objectType refe_to;
    objectType refe_from;

  /* ref_create */
    refe_to = arg_1(arguments);
    refe_from = arg_3(arguments);
    isit_reference(refe_from);
    SET_CATEGORY_OF_OBJ(refe_to, REFOBJECT);
    refe_to->value.objValue = take_reference(refe_from);
    return SYS_EMPTY_OBJECT;
  } /* ref_create */



objectType ref_deref (listType arguments)

  { /* ref_deref */
    isit_reference(arg_1(arguments));
    return take_reference(arg_1(arguments));
  } /* ref_deref */



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



objectType ref_find (listType arguments)

  {
    /* objectType module_object; */
    objectType result;

  /* ref_find */
    isit_reference(arg_1(arguments));
    isit_reference(arg_3(arguments));
#ifdef OUT_OF_ORDER
    module_object = take_reference(arg_1(arguments));
/*  trace2(module_object); */
    if (CATEGORY_OF_OBJ(module_object) == MODULEOBJECT) {
/*    printf(" is module\n"); */
      result = FIND_OBJECT(module_object->value.DECLVALUE,
          take_reference(arg_3(arguments)));
      result = NULL;
/*    trace2(result); */
    } else {
/*    printf(" is not module\n"); */
      result = NULL;
    } /* if */
#else
    result = NULL;
#endif
    return bld_reference_temp(result);
  } /* ref_find */



objectType ref_getref (listType arguments)

  { /* ref_getref */
    return bld_reference_temp(arg_1(arguments));
  } /* ref_getref */



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



objectType ref_issymb (listType arguments)

  {
    objectType symb_object;

  /* ref_issymb */
    isit_reference(arg_1(arguments));
    symb_object = take_reference(arg_1(arguments));
    /* printf("ref issymb %lu ", (long unsigned) GET_ENTITY(symb_object));
    trace1(symb_object);
    printf(":\n"); */
    if (HAS_ENTITY(symb_object) &&
        GET_ENTITY(symb_object)->syobject != NULL) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* ref_issymb */



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



objectType ref_line (listType arguments)

  { /* ref_line */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refLine(
        take_reference(arg_1(arguments))));
  } /* ref_line */



objectType ref_local_consts (listType arguments)

  { /* ref_local_consts */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refLocalConsts(
        take_reference(arg_1(arguments))));
  } /* ref_local_consts */



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



objectType ref_num (listType arguments)

  { /* ref_num */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refNum(
        take_reference(arg_1(arguments))));
  } /* ref_num */



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



objectType ref_prog (listType arguments)

  { /* ref_prog */
    return bld_reference_temp(NULL);
  } /* ref_prog */



objectType ref_resini (listType arguments)

  { /* ref_resini */
    isit_reference(arg_1(arguments));
    return bld_reference_temp(refResini(
        take_reference(arg_1(arguments))));
  } /* ref_resini */



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
      ident_found = get_ident(&prog, (const_ustriType) name);
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
        struct_pointer = stru1->stru;
        while (position > 0) {
/*
printf("test ");
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
          struct_pointer++;
        } /* while */
      } /* if */
    } /* if */
    return raise_exception(SYS_RNG_EXCEPTION);
  } /* ref_select */



objectType ref_setcategory (listType arguments)

  { /* ref_setcategory */
    isit_reference(arg_1(arguments));
    isit_int(arg_2(arguments));
    refSetCategory(take_reference(arg_1(arguments)),
                   take_int(      arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* ref_setcategory */



objectType ref_setparams (listType arguments)

  { /* ref_setparams */
    isit_reference(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    refSetParams(take_reference(arg_1(arguments)), take_reflist(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* ref_setparams */



objectType ref_settype (listType arguments)

  { /* ref_settype */
    isit_reference(arg_1(arguments));
    isit_type(arg_2(arguments));
    refSetType(take_reference(arg_1(arguments)),
               take_type(     arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* ref_settype */



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
    /* printf("ref symb " FMT_U_MEM " ", (memSizeType) GET_ENTITY(symb_object));
    trace1(symb_object);
    printf(":\n"); */
    if (HAS_ENTITY(symb_object) &&
        GET_ENTITY(symb_object)->syobject != NULL) {
      symb_object = GET_ENTITY(symb_object)->syobject;
    } else {
      printf("ref symb " FMT_U_MEM " ", (memSizeType) GET_ENTITY(symb_object));
      trace1(symb_object);
      printf(":\n");
    } /* if */
    /* printf("ref symb " FMT_U_MEM " ", (memSizeType) GET_ENTITY(symb_object));
    trace1(symb_object);
    printf(":\n"); */
    return bld_param_temp(symb_object);
  } /* ref_symb */



objectType ref_trace (listType arguments)

  { /* ref_trace */
    trace1(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* ref_trace */



objectType ref_type (listType arguments)

  {
    objectType obj_arg;

  /* ref_type */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else if (obj_arg->type_of == NULL) {
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
