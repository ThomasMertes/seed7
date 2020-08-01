/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2008  Thomas Mertes                        */
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
/*  Changes: 1991, 1992, 1993, 1994, 2004, 2007  Thomas Mertes      */
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
#include "analyze.h"
#include "exec.h"
#include "runerr.h"
#include "name.h"
#include "infile.h"
#include "str_rtl.h"
#include "ref_data.h"

#undef EXTERN
#define EXTERN
#include "reflib.h"


#define TABLE_INCREMENT 127



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

static void qsort_list (listtype *begin_sorted, listtype *end_sorted)
#else

static void qsort_list (begin_sorted, end_sorted)
listtype *begin_sorted;
listtype *end_sorted;
#endif

  {
    listtype input_list, key_element;
    listtype begin_less, end_less, begin_greater, end_greater;
    ustritype key;

  /* qsort_list */
    key_element = *begin_sorted;
    input_list = key_element->next;
    key = GET_ENTITY(key_element->obj)->ident->name;
    begin_less = NULL;
    begin_greater = NULL;
    do {
      if (strcmp((cstritype) GET_ENTITY(input_list->obj)->ident->name,
          (cstritype) key) < 0) {
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



#ifdef ANSI_C

static void sort_list (listtype *any_list)
#else

static void sort_list (any_list)
listtype *any_list;
#endif

  {
    listtype list_end;

  /* sort_list */
    if (*any_list != NULL && (*any_list)->next != NULL) {
      qsort_list(any_list, &list_end);
      list_end->next = NULL;
    } /* if */
  } /* sort_list */
#endif



#ifdef ANSI_C

objecttype ref_addr (listtype arguments)
#else

objecttype ref_addr (arguments)
listtype arguments;
#endif

  { /* ref_addr */
    return bld_reference_temp(arg_2(arguments));
  } /* ref_addr */



#ifdef ANSI_C

objecttype ref_alloc (listtype arguments)
#else

objecttype ref_alloc (arguments)
listtype arguments;
#endif

  {
    objecttype obj1;
    objecttype created_object;

  /* ref_alloc */
    isit_reference(arg_1(arguments));
    obj1 = take_reference(arg_1(arguments));
    if (ALLOC_OBJECT(created_object)) {
      created_object->type_of = obj1->type_of;
      created_object->descriptor.property = obj1->descriptor.property;
      INIT_CATEGORY_OF_OBJ(created_object, obj1->objcategory);
      created_object->value.objvalue = NULL;
      return bld_reference_temp(created_object);
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* ref_alloc */



#ifdef ANSI_C

objecttype ref_arrmaxidx (listtype arguments)
#else

objecttype ref_arrmaxidx (arguments)
listtype arguments;
#endif

  { /* ref_arrmaxidx */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refArrmaxidx(
        take_reference(arg_1(arguments))));
  } /* ref_arrmaxidx */



#ifdef ANSI_C

objecttype ref_arrminidx (listtype arguments)
#else

objecttype ref_arrminidx (arguments)
listtype arguments;
#endif

  { /* ref_arrminidx */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refArrminidx(
        take_reference(arg_1(arguments))));
  } /* ref_arrminidx */



#ifdef ANSI_C

objecttype ref_arrtolist (listtype arguments)
#else

objecttype ref_arrtolist (arguments)
listtype arguments;
#endif

  { /* ref_arrtolist */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refArrtolist(
        take_reference(arg_1(arguments))));
  } /* ref_arrtolist */



#ifdef ANSI_C

objecttype ref_body (listtype arguments)
#else

objecttype ref_body (arguments)
listtype arguments;
#endif

  { /* ref_body */
    isit_reference(arg_1(arguments));
    return bld_reference_temp(refBody(
        take_reference(arg_1(arguments))));
  } /* ref_body */



#ifdef ANSI_C

objecttype ref_build (listtype arguments)
#else

objecttype ref_build (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* ref_build */
    result = bld_reference_temp(arg_1(arguments));
    return result;
  } /* ref_build */



#ifdef ANSI_C

objecttype ref_category (listtype arguments)
#else

objecttype ref_category (arguments)
listtype arguments;
#endif

  { /* ref_category */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refCategory(
        take_reference(arg_1(arguments))));
  } /* ref_category */



#ifdef ANSI_C

objecttype ref_cat_parse (listtype arguments)
#else

objecttype ref_cat_parse (arguments)
listtype arguments;
#endif

  { /* ref_cat_parse */
    isit_stri(arg_3(arguments));
    return bld_int_temp(refCatParse(
        take_stri(arg_3(arguments))));
  } /* ref_cat_parse */



#ifdef ANSI_C

objecttype ref_cat_str (listtype arguments)
#else

objecttype ref_cat_str (arguments)
listtype arguments;
#endif

  { /* ref_cat_str */
    isit_int(arg_1(arguments));
    return bld_stri_temp(refCatStr(
        take_int(arg_1(arguments))));
  } /* ref_cat_str */



#ifdef ANSI_C

objecttype ref_cmp (listtype arguments)
#else

objecttype ref_cmp (arguments)
listtype arguments;
#endif

  {
    memsizetype ref1;
    memsizetype ref2;
    inttype result;

  /* ref_cmp */
    isit_reference(arg_1(arguments));
    isit_reference(arg_2(arguments));
    ref1 = (memsizetype) take_reference(arg_1(arguments));
    ref2 = (memsizetype) take_reference(arg_2(arguments));
    if (ref1 < ref2) {
      result = -1;
    } else if (ref1 > ref2) {
      result = 1;
    } else {
      result = 0;
    } /* if */
    return bld_int_temp(result);
  } /* ref_cmp */



#ifdef ANSI_C

objecttype ref_content (listtype arguments)
#else

objecttype ref_content (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    listtype result;

  /* ref_content */
    obj_arg1 = arg_1(arguments);
    isit_reference(obj_arg1);
    result = NULL;
    return bld_reflist_temp(result);
  } /* ref_content */



#ifdef ANSI_C

objecttype ref_conv (listtype arguments)
#else

objecttype ref_conv (arguments)
listtype arguments;
#endif

  { /* ref_conv */
    isit_reference(arg_3(arguments));
    return bld_reference_temp(take_reference(arg_3(arguments)));
  } /* ref_conv */



#ifdef ANSI_C

objecttype ref_cpy (listtype arguments)
#else

objecttype ref_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype ref_variable;

  /* ref_cpy */
    ref_variable = arg_1(arguments);
    isit_reference(ref_variable);
    is_variable(ref_variable);
    isit_reference(arg_3(arguments));
    ref_variable->value.objvalue = take_reference(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* ref_cpy */



#ifdef ANSI_C

objecttype ref_create (listtype arguments)
#else

objecttype ref_create (arguments)
listtype arguments;
#endif

  {
    objecttype refe_to;
    objecttype refe_from;

  /* ref_create */
    refe_to = arg_1(arguments);
    refe_from = arg_3(arguments);
    isit_reference(refe_from);
    SET_CATEGORY_OF_OBJ(refe_to, REFOBJECT);
    refe_to->value.objvalue = take_reference(refe_from);
    return SYS_EMPTY_OBJECT;
  } /* ref_create */



#ifdef ANSI_C

objecttype ref_deref (listtype arguments)
#else

objecttype ref_deref (arguments)
listtype arguments;
#endif

  { /* ref_deref */
    isit_reference(arg_1(arguments));
    return take_reference(arg_1(arguments));
  } /* ref_deref */



#ifdef ANSI_C

objecttype ref_eq (listtype arguments)
#else

objecttype ref_eq (arguments)
listtype arguments;
#endif

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



#ifdef ANSI_C

objecttype ref_file (listtype arguments)
#else

objecttype ref_file (arguments)
listtype arguments;
#endif

  {
    const_stritype name;
    stritype result;

  /* ref_file */
    isit_reference(arg_1(arguments));
    name = refFile(take_reference(arg_1(arguments)));
    if (name == NULL) {
      result = NULL;
    } else {
      result = strCreate(name);
    } /* if */
    if (result == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_stri_temp(result);
    } /* if */
  } /* ref_file */



#ifdef ANSI_C

objecttype ref_find (listtype arguments)
#else

objecttype ref_find (arguments)
listtype arguments;
#endif

  {
    objecttype module_object;
    objecttype result;

  /* ref_find */
    isit_reference(arg_1(arguments));
    isit_reference(arg_3(arguments));
    module_object = take_reference(arg_1(arguments));
/*  trace2(module_object); */
#ifdef OUT_OF_ORDER
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



#ifdef ANSI_C

objecttype ref_hashcode (listtype arguments)
#else

objecttype ref_hashcode (arguments)
listtype arguments;
#endif

  { /* ref_hashcode */
    isit_reference(arg_1(arguments));
    return bld_int_temp((inttype)
        (((memsizetype) take_reference(arg_1(arguments))) >> 6));
  } /* ref_hashcode */



#ifdef ANSI_C

objecttype ref_hshdatatolist (listtype arguments)
#else

objecttype ref_hshdatatolist (arguments)
listtype arguments;
#endif

  { /* ref_hshdatatolist */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refHshDataToList(
        take_reference(arg_1(arguments))));
  } /* ref_hshdatatolist */



#ifdef ANSI_C

objecttype ref_hshkeytolist (listtype arguments)
#else

objecttype ref_hshkeytolist (arguments)
listtype arguments;
#endif

  { /* ref_hshkeytolist */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refHshKeyToList(
        take_reference(arg_1(arguments))));
  } /* ref_hshkeytolist */



#ifdef ANSI_C

objecttype ref_issymb (listtype arguments)
#else

objecttype ref_issymb (arguments)
listtype arguments;
#endif

  {
    objecttype symb_object;

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



#ifdef ANSI_C

objecttype ref_isvar (listtype arguments)
#else

objecttype ref_isvar (arguments)
listtype arguments;
#endif

  { /* ref_isvar */
    isit_reference(arg_1(arguments));
    if (refIsVar(take_reference(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* ref_isvar */



#ifdef ANSI_C

objecttype ref_itftosct (listtype arguments)
#else

objecttype ref_itftosct (arguments)
listtype arguments;
#endif

  { /* ref_itftosct */
    isit_reference(arg_1(arguments));
    return bld_reference_temp(refItftosct(
        take_reference(arg_1(arguments))));
  } /* ref_itftosct */



#ifdef ANSI_C

objecttype ref_line (listtype arguments)
#else

objecttype ref_line (arguments)
listtype arguments;
#endif

  { /* ref_line */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refLine(
        take_reference(arg_1(arguments))));
  } /* ref_line */



#ifdef ANSI_C

objecttype ref_local_consts (listtype arguments)
#else

objecttype ref_local_consts (arguments)
listtype arguments;
#endif

  { /* ref_local_consts */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refLocalConsts(
        take_reference(arg_1(arguments))));
  } /* ref_local_consts */



#ifdef ANSI_C

objecttype ref_local_vars (listtype arguments)
#else

objecttype ref_local_vars (arguments)
listtype arguments;
#endif

  { /* ref_local_vars */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refLocalVars(
        take_reference(arg_1(arguments))));
  } /* ref_local_vars */



#ifdef ANSI_C

objecttype ref_mkref (listtype arguments)
#else

objecttype ref_mkref (arguments)
listtype arguments;
#endif

  {
    objecttype refe_to;
    objecttype refe_from;

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
    refe_to->value.objvalue = refe_from;
    return SYS_EMPTY_OBJECT;
  } /* ref_mkref */



#ifdef ANSI_C

objecttype ref_name (listtype arguments)
#else

objecttype ref_name (arguments)
listtype arguments;
#endif

  {
    objecttype obj;

  /* ref_name */
    isit_reference(arg_1(arguments));
    obj = take_reference(arg_1(arguments));
    if (obj == NULL) {
      return bld_reference_temp(NULL);
    } else {
      if (HAS_ENTITY(obj)) {
        return bld_reference_temp(GET_ENTITY(obj)->owner->obj);
      } else {
        return bld_reference_temp(NULL);
      } /* if */
    } /* if */
  } /* ref_name */



#ifdef ANSI_C

objecttype ref_ne (listtype arguments)
#else

objecttype ref_ne (arguments)
listtype arguments;
#endif

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



#ifdef ANSI_C

objecttype ref_nil (listtype arguments)
#else

objecttype ref_nil (arguments)
listtype arguments;
#endif

  { /* ref_nil */
    return bld_reference_temp(NULL);
  } /* ref_nil */



#ifdef ANSI_C

objecttype ref_num (listtype arguments)
#else

objecttype ref_num (arguments)
listtype arguments;
#endif

  { /* ref_num */
    isit_reference(arg_1(arguments));
    return bld_int_temp(refNum(
        take_reference(arg_1(arguments))));
  } /* ref_num */



#ifdef ANSI_C

objecttype ref_params (listtype arguments)
#else

objecttype ref_params (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    loclisttype local_elem;
    listtype *list_insert_place;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* ref_params */
    isit_reference(arg_1(arguments));
    obj_arg1 = take_reference(arg_1(arguments));
    result = NULL;
    if (CATEGORY_OF_OBJ(obj_arg1) == BLOCKOBJECT) {
      list_insert_place = &result;
      local_elem = obj_arg1->value.blockvalue->params;
      while (local_elem != NULL) {
        list_insert_place = append_element_to_list(list_insert_place,
            local_elem->local.object, &err_info);
        local_elem = local_elem->next;
      } /* while */
    } else {
      result = create_parameter_list(GET_ENTITY(obj_arg1)->name_list,
          &err_info);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      emptylist(result);
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    return bld_reflist_temp(result);
  } /* ref_params */



#ifdef ANSI_C

objecttype ref_prog (listtype arguments)
#else

objecttype ref_prog (arguments)
listtype arguments;
#endif

  { /* ref_prog */
    return bld_reference_temp(NULL);
  } /* ref_prog */



#ifdef ANSI_C

objecttype ref_resini (listtype arguments)
#else

objecttype ref_resini (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    objecttype result;

  /* ref_resini */
    isit_reference(arg_1(arguments));
    obj_arg1 = take_reference(arg_1(arguments));
    if (CATEGORY_OF_OBJ(obj_arg1) == BLOCKOBJECT) {
      result = obj_arg1->value.blockvalue->result.init_value;
    } else {
      result = NULL;
    } /* if */
    return bld_reference_temp(result);
  } /* ref_resini */



#ifdef ANSI_C

objecttype ref_result (listtype arguments)
#else

objecttype ref_result (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    objecttype result;

  /* ref_result */
    isit_reference(arg_1(arguments));
    obj_arg1 = take_reference(arg_1(arguments));
    if (CATEGORY_OF_OBJ(obj_arg1) == BLOCKOBJECT) {
      result = obj_arg1->value.blockvalue->result.object;
    } else {
      result = NULL;
    } /* if */
    return bld_reference_temp(result);
  } /* ref_result */



#ifdef ANSI_C

objecttype ref_scan (listtype arguments)
#else

objecttype ref_scan (arguments)
listtype arguments;
#endif

  {
    stritype str1;
    objecttype obj_variable;
    cstritype name;
    identtype ident_found;
    objecttype result;

  /* ref_scan */
    isit_stri(arg_1(arguments));
    str1 = take_stri(arg_1(arguments));
    obj_variable = arg_2(arguments);
    isit_reference(obj_variable);
    is_variable(obj_variable);
    name = cp_to_cstri(str1);
    if (name == NULL) {
      result = raise_exception(SYS_MEM_EXCEPTION);
    } else {
      ident_found = get_ident((ustritype) name, strlen(name));
      if (ident_found == NULL ||
          ident_found->entity == NULL ||
          ident_found->entity->owner == NULL) {
        result = raise_exception(SYS_MEM_EXCEPTION);
      } else {
        if (ident_found->entity->owner->obj != NULL) {
          obj_variable->value.objvalue = ident_found->entity->owner->obj;
          result = SYS_TRUE_OBJECT;
        } else {
          obj_variable->value.objvalue = ident_found->entity->syobject;
          result = SYS_TRUE_OBJECT;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* ref_scan */



#ifdef ANSI_C

objecttype ref_scttolist (listtype arguments)
#else

objecttype ref_scttolist (arguments)
listtype arguments;
#endif

  { /* ref_scttolist */
    isit_reference(arg_1(arguments));
    return bld_reflist_temp(refScttolist(
        take_reference(arg_1(arguments))));
  } /* ref_scttolist */



#ifdef ANSI_C

objecttype ref_select (listtype arguments)
#else

objecttype ref_select (arguments)
listtype arguments;
#endif

  {
    objecttype refer;
    structtype stru1;
    objecttype selector;
    objecttype selector_syobject;
    memsizetype position;
    objecttype struct_pointer;
    objecttype result;

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
                memcpy(result, struct_pointer, sizeof(objectrecord));
                SET_TEMP_FLAG(result);
                destr_struct(stru1->stru,
                    (memsizetype) (struct_pointer - stru1->stru));
                destr_struct(&struct_pointer[1],
                    (stru1->size - (memsizetype) (struct_pointer - stru1->stru) - 1));
                FREE_STRUCT(stru1, stru1->size);
                arg_1(arguments)->value.structvalue = NULL;
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



#ifdef ANSI_C

objecttype ref_setcategory (listtype arguments)
#else

objecttype ref_setcategory (arguments)
listtype arguments;
#endif

  { /* ref_setcategory */
    isit_reference(arg_1(arguments));
    isit_int(arg_2(arguments));
    refSetCategory(take_reference(arg_1(arguments)),
                   take_int(      arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* ref_setcategory */



#ifdef ANSI_C

objecttype ref_setparams (listtype arguments)
#else

objecttype ref_setparams (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    errinfotype err_info = OKAY_NO_ERROR;

  /* ref_setparams */
    isit_reference(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    obj_arg1 = take_reference(arg_1(arguments));
    if (CATEGORY_OF_OBJ(obj_arg1) == BLOCKOBJECT) {
      /*FIXME not ok since parameter names are important here !!! */
      /* Comment copied from dcllib.c */
      obj_arg1->value.blockvalue->params =
          get_param_list(take_reflist(arg_2(arguments)), &err_info);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* ref_setparams */



#ifdef ANSI_C

objecttype ref_settype (listtype arguments)
#else

objecttype ref_settype (arguments)
listtype arguments;
#endif

  { /* ref_settype */
    isit_reference(arg_1(arguments));
    isit_type(arg_2(arguments));
    refSetType(take_reference(arg_1(arguments)),
               take_type(     arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* ref_settype */



#ifdef ANSI_C

objecttype ref_setvar (listtype arguments)
#else

objecttype ref_setvar (arguments)
listtype arguments;
#endif

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



#ifdef ANSI_C

objecttype ref_str (listtype arguments)
#else

objecttype ref_str (arguments)
listtype arguments;
#endif

  { /* ref_str */
    isit_reference(arg_1(arguments));
    return bld_stri_temp(refStr(
        take_reference(arg_1(arguments))));
  } /* ref_str */



#ifdef ANSI_C

objecttype ref_symb (listtype arguments)
#else

objecttype ref_symb (arguments)
listtype arguments;
#endif

  {
    objecttype symb_object;

  /* ref_symb */
    isit_reference(arg_2(arguments));
    symb_object = take_reference(arg_2(arguments));
    /* printf("ref symb %lu ", (long unsigned) GET_ENTITY(symb_object));
    trace1(symb_object);
    printf(":\n"); */
    if (HAS_ENTITY(symb_object) &&
        GET_ENTITY(symb_object)->syobject != NULL) {
      symb_object = GET_ENTITY(symb_object)->syobject;
    } else {
      printf("ref symb %lu ", (long unsigned) GET_ENTITY(symb_object));
      trace1(symb_object);
      printf(":\n");
    } /* if */
    /* printf("ref symb %lu ", (long unsigned) GET_ENTITY(symb_object));
    trace1(symb_object);
    printf(":\n"); */
    return bld_param_temp(symb_object);
  } /* ref_symb */



#ifdef ANSI_C

objecttype ref_trace (listtype arguments)
#else

objecttype ref_trace (arguments)
listtype arguments;
#endif

  { /* ref_trace */
    trace1(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* ref_trace */



#ifdef ANSI_C

objecttype ref_type (listtype arguments)
#else

objecttype ref_type (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;

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



#ifdef ANSI_C

objecttype ref_value (listtype arguments)
#else

objecttype ref_value (arguments)
listtype arguments;
#endif

  { /* ref_value */
    isit_reference(arg_1(arguments));
    return bld_reference_temp(refValue(
        take_reference(arg_1(arguments))));
  } /* ref_value */
