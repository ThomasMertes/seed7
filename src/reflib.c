/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2004  Thomas Mertes                        */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/reflib.c                                        */
/*  Changes: 1991, 1992, 1993, 1994, 2004  Thomas Mertes            */
/*  Content: All primitive actions for the reference type.          */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "identutl.h"
#include "listutl.h"
#include "entutl.h"
#include "executl.h"
#include "findid.h"
#include "listutl.h"
#include "traceutl.h"
#include "analyze.h"
#include "exec.h"
#include "runerr.h"
#include "memory.h"
#include "name.h"
#include "option.h"

#undef EXTERN
#define EXTERN
#include "reflib.h"


#define TABLE_INCREMENT 127



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
    key = key_element->obj->entity->ident->name;
    begin_less = NULL;
    begin_greater = NULL;
    do {
      if (strcmp((cstritype) input_list->obj->entity->ident->name,
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



#ifdef ANSI_C

objecttype ref_addr (listtype arguments)
#else

objecttype ref_addr (arguments)
listtype arguments;
#endif

  { /* ref_addr */
    return(bld_reference_temp(arg_2(arguments)));
  } /* ref_addr */



#ifdef ANSI_C

objecttype ref_arrtolist (listtype arguments)
#else

objecttype ref_arrtolist (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* ref_arrtolist */
    isit_reference(arg_1(arguments));
    obj_arg1 = take_reference(arg_1(arguments));
    isit_array(obj_arg1);
    array_to_list(take_array(obj_arg1), &result, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
    return(bld_reflist_temp(result));
  } /* ref_arrtolist */



#ifdef ANSI_C

objecttype ref_body (listtype arguments)
#else

objecttype ref_body (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    objecttype result;

  /* ref_body */
    obj_arg1 = arg_1(arguments);
    isit_reference(obj_arg1);
    if (CLASS_OF_OBJ(take_reference(obj_arg1)) == BLOCKOBJECT) {
      result = take_reference(obj_arg1)->value.blockvalue->body;
    } else {
      result = NULL;
    } /* if */
    return(bld_reference_temp(result));
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
    return(result);
  } /* ref_build */



#ifdef ANSI_C

objecttype ref_cmp (listtype arguments)
#else

objecttype ref_cmp (arguments)
listtype arguments;
#endif

  {
    uinttype ref1;
    uinttype ref2;

  /* ref_cmp */
    isit_reference(arg_1(arguments));
    isit_reference(arg_2(arguments));
    ref1 = (uinttype) take_reference(arg_1(arguments));
    ref2 = (uinttype) take_reference(arg_2(arguments));
    if (ref1 < ref2) {
      return(bld_int_temp(-1));
    } else if (ref1 > ref2) {
      return(bld_int_temp(1));
    } else {
      return(bld_int_temp(0));
    } /* if */
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
    return(bld_reflist_temp(result));
  } /* ref_content */



#ifdef ANSI_C

objecttype ref_conv (listtype arguments)
#else

objecttype ref_conv (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* ref_conv */
    isit_reference(arg_3(arguments));
    result = take_reference(arg_3(arguments));
    return(bld_reference_temp(result));
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
    return(SYS_EMPTY_OBJECT);
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
    SET_CLASS_OF_OBJ(refe_to, REFOBJECT);
    refe_to->value.objvalue = take_reference(refe_from);
    return(SYS_EMPTY_OBJECT);
  } /* ref_create */



#ifdef ANSI_C

objecttype ref_deref (listtype arguments)
#else

objecttype ref_deref (arguments)
listtype arguments;
#endif

  { /* ref_deref */
    isit_reference(arg_1(arguments));
    return(take_reference(arg_1(arguments)));
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
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* ref_eq */



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
    if (CLASS_OF_OBJ(module_object) == MODULEOBJECT) {
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
    return(bld_reference_temp(result));
  } /* ref_find */



#ifdef ANSI_C

objecttype ref_hashcode (listtype arguments)
#else

objecttype ref_hashcode (arguments)
listtype arguments;
#endif

  { /* ref_hashcode */
    isit_reference(arg_1(arguments));
    return(bld_int_temp((inttype) take_reference(arg_1(arguments))));
  } /* ref_hashcode */



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
    /* printf("ref issymb %lu ", (long unsigned) symb_object->entity);
    trace1(symb_object);
    printf(":\n"); */
    if (symb_object->entity != NULL &&
        symb_object->entity->syobject != NULL) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* ref_issymb */



#ifdef ANSI_C

objecttype ref_isvar (listtype arguments)
#else

objecttype ref_isvar (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;

  /* ref_isvar */
    obj_arg1 = arg_1(arguments);
    isit_reference(obj_arg1);
    if (VAR_OBJECT(take_reference(obj_arg1))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* ref_isvar */



#ifdef ANSI_C

objecttype ref_locals (listtype arguments)
#else

objecttype ref_locals (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    loclisttype local_elem;
    listtype *list_insert_place;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* ref_locals */
    isit_reference(arg_1(arguments));
    obj_arg1 = take_reference(arg_1(arguments));
    result = NULL;
    if (CLASS_OF_OBJ(obj_arg1) == BLOCKOBJECT) {
      list_insert_place = &result;
      local_elem = obj_arg1->value.blockvalue->locals;
      while (local_elem != NULL) {
        list_insert_place = append_element_to_list(list_insert_place,
            local_elem->local.object, &err_info);
        local_elem = local_elem->next;
      } /* while */
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      emptylist(result);
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
    return(bld_reflist_temp(result));
  } /* ref_locals */



#ifdef ANSI_C

objecttype ref_locini (listtype arguments)
#else

objecttype ref_locini (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* ref_locini */
    obj_arg1 = arg_1(arguments);
    isit_reference(obj_arg1);
    if (CLASS_OF_OBJ(take_reference(obj_arg1)) == BLOCKOBJECT) {
#ifdef NEW_LOCALTYPE
      copy_list(take_reference(obj_arg1)->value.blockvalue->loc_init,
          &result, &err_info);
#endif
      if (err_info != OKAY_NO_ERROR) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } /* if */
    } else {
      result = (listtype) NULL;
    } /* if */
    return(bld_reflist_temp(result));
  } /* ref_locini */



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
    return(SYS_EMPTY_OBJECT);
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
      return(bld_reference_temp(NULL));
    } else {
      if (obj->entity == NULL) {
        return(bld_reference_temp(NULL));
      } else {
        return(bld_reference_temp(obj->entity->owner->obj));
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
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* ref_ne */



#ifdef ANSI_C

objecttype ref_nil (listtype arguments)
#else

objecttype ref_nil (arguments)
listtype arguments;
#endif

  { /* ref_nil */
    return(bld_reference_temp(NULL));
  } /* ref_nil */



#ifdef ANSI_C

objecttype ref_num (listtype arguments)
#else

objecttype ref_num (arguments)
listtype arguments;
#endif

  {
    static unsigned int table_size = 0;
    static unsigned int table_used = 0;
    static objecttype *obj_table = NULL;
    register objecttype *actual_obj_ptr;
    register objecttype obj_arg1;
    unsigned int result;

  /* ref_num */
    isit_reference(arg_1(arguments));
    obj_arg1 = take_reference(arg_1(arguments));
    if (obj_arg1 == (objecttype) NULL) {
      result = 0;
    } else {
      table_used++;
      if (table_used > table_size) {
        if (obj_table == NULL) {
          if (ALLOC_TABLE(obj_table, objecttype,
              table_used + TABLE_INCREMENT)) {
            COUNT_TABLE(objecttype, table_used + TABLE_INCREMENT);
          } /* if */
        } else {
          if (RESIZE_TABLE(obj_table, objecttype,
              table_size, table_used + TABLE_INCREMENT)) {
            COUNT3_TABLE(objecttype, table_size, table_used + TABLE_INCREMENT);
          } /* if */
        } /* if */
        if (obj_table == NULL) {
          table_size = 0;
          table_used = 0;
          return(raise_exception(SYS_MEM_EXCEPTION));
        } /* if */
        table_size = table_used + TABLE_INCREMENT;
      } /* if */
      obj_table[table_used - 1] = obj_arg1;
      actual_obj_ptr = obj_table;
      while (*actual_obj_ptr != obj_arg1) {
        actual_obj_ptr++;
      } /* while */
      result = (unsigned int) (actual_obj_ptr - obj_table);
      if (result != table_used - 1) {
        table_used--;
      } /* if */
      result++;
    } /* if */
    /* printf("ref_num: %lx %lx %lu\n", arg_1(arguments), obj_arg1, result); */
    return(bld_int_temp((inttype) result));
  } /* ref_num */



#ifdef ANSI_C

objecttype ref_objclass (listtype arguments)
#else

objecttype ref_objclass (arguments)
listtype arguments;
#endif

  {
    objecttype obj;
    cstritype name;
    memsizetype len;
    stritype result;

  /* ref_objclass */
    isit_reference(arg_1(arguments));
    obj = take_reference(arg_1(arguments));
    if (obj == NULL) {
      name = "*NULL_OBJECT*";
    } else {
      name = class_stri(CLASS_OF_OBJ(obj));
    } /* if */
    len = (memsizetype) strlen(name);
    if (ALLOC_STRI(result, len)) {
      COUNT_STRI(len);
      result->size = len;
      stri_expand(result->mem, name, len);
      return(bld_stri_temp(result));
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* ref_objclass */



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
    if (CLASS_OF_OBJ(obj_arg1) == BLOCKOBJECT) {
      list_insert_place = &result;
      local_elem = obj_arg1->value.blockvalue->params;
      while (local_elem != NULL) {
        list_insert_place = append_element_to_list(list_insert_place,
            local_elem->local.object, &err_info);
        local_elem = local_elem->next;
      } /* while */
    } else {
      result = create_parameter_list(obj_arg1->entity->name_list,
          &err_info);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      emptylist(result);
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
    return(bld_reflist_temp(result));
  } /* ref_params */



#ifdef ANSI_C

objecttype ref_prog (listtype arguments)
#else

objecttype ref_prog (arguments)
listtype arguments;
#endif

  { /* ref_prog */
    return(bld_reference_temp(NULL));
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
    if (CLASS_OF_OBJ(obj_arg1) == BLOCKOBJECT) {
      result = obj_arg1->value.blockvalue->result.init_value;
    } else {
      result = NULL;
    } /* if */
    return(bld_reference_temp(result));
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
    if (CLASS_OF_OBJ(obj_arg1) == BLOCKOBJECT) {
      result = obj_arg1->value.blockvalue->result.object;
    } else {
      result = NULL;
    } /* if */
    return(bld_reference_temp(result));
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
    ustritype name;
    identtype ident_found;
    objecttype result;

  /* ref_scan */
    isit_stri(arg_1(arguments));
    str1 = take_stri(arg_1(arguments));
    obj_variable = arg_2(arguments);
    isit_reference(obj_variable);
    is_variable(obj_variable);
    name = (ustritype) cp_to_cstri(str1);
    if (name == NULL) {
      result = raise_exception(SYS_MEM_EXCEPTION);
    } else {
      ident_found = get_ident(name, strlen(name));
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
    return(result);
  } /* ref_scan */



#ifdef ANSI_C

objecttype ref_scttolist (listtype arguments)
#else

objecttype ref_scttolist (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* ref_scttolist */
    isit_reference(arg_1(arguments));
    obj_arg1 = take_reference(arg_1(arguments));
    isit_struct(obj_arg1);
    struct_to_list(take_struct(obj_arg1), &result, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
    return(bld_reflist_temp(result));
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
    inttype position;
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
      if (selector->entity != NULL && selector->entity->syobject != NULL) {
        selector_syobject = selector->entity->syobject;
        position = stru1->size;
        struct_pointer = stru1->stru;
        while (position > 0) {
/*
printf("test ");
trace1(struct_pointer);
printf("\n");
*/
          if (struct_pointer->entity != NULL &&
              struct_pointer->entity->syobject == selector_syobject) {
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
                    (memsizetype) (stru1->size - (struct_pointer - stru1->stru) - 1));
                FREE_STRUCT(stru1, stru1->size);
                arg_1(arguments)->value.structvalue = NULL;
              } /* if */
              return(result);
            } else {
              return(struct_pointer);
            } /* if */
          } /* if */
          position--;
          struct_pointer++;
        } /* while */
      } /* if */
    } /* if */
    return(raise_exception(SYS_RNG_EXCEPTION));
  } /* ref_select */



#ifdef ANSI_C

objecttype ref_str (listtype arguments)
#else

objecttype ref_str (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    char *stri;
    listtype name_elem;
    objecttype param_obj;
    memsizetype len;
    stritype result;

  /* ref_str */
    isit_reference(arg_1(arguments));
    obj_arg1 = take_reference(arg_1(arguments));
    if (obj_arg1 == NULL) {
      stri = " *NULL_OBJECT* ";
    } else if (obj_arg1->entity == NULL) {
      stri = " *NULL_ENTITY_OBJECT* ";
    } else if (obj_arg1->entity->ident != NULL) {
      stri = id_string(obj_arg1->entity->ident);
    } else {
      stri = NULL;
      name_elem = obj_arg1->entity->name_list;
      while (name_elem != NULL && stri == NULL) {
        if (CLASS_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
          param_obj = name_elem->obj->value.objvalue;
          if (CLASS_OF_OBJ(param_obj) != VALUEPARAMOBJECT &&
              CLASS_OF_OBJ(param_obj) != REFPARAMOBJECT &&
              CLASS_OF_OBJ(param_obj) != TYPEOBJECT) {
            stri = id_string(param_obj->entity->ident);
          } /* if */
        } else {
          stri = id_string(name_elem->obj->entity->ident);
        } /* if */
        name_elem = name_elem->next;
      } /* while */
      if (stri == NULL) {
        stri = " *UNKNOWN_NAME* ";
      } /* if */
    } /* if */
    len = (memsizetype) strlen(stri);
    if (!ALLOC_STRI(result, len)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      COUNT_STRI(len);
      result->size = len;
      stri_expand(result->mem, stri, (SIZE_TYPE) len);
      return(bld_stri_temp(result));
    } /* if */
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
    /* printf("ref symb %lu ", (long unsigned) symb_object->entity);
    trace1(symb_object);
    printf(":\n"); */
    if (symb_object->entity != NULL &&
        symb_object->entity->syobject != NULL) {
      symb_object = symb_object->entity->syobject;
    } else {
      printf("ref symb %lu ", (long unsigned) symb_object->entity);
      trace1(symb_object);
      printf(":\n");
    } /* if */
    /* printf("ref symb %lu ", (long unsigned) symb_object->entity);
    trace1(symb_object);
    printf(":\n"); */
    return(bld_param_temp(symb_object));
  } /* ref_symb */



#ifdef ANSI_C

objecttype ref_trace (listtype arguments)
#else

objecttype ref_trace (arguments)
listtype arguments;
#endif

  { /* ref_trace */
    trace1(arg_1(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* ref_trace */



#ifdef ANSI_C

objecttype ref_type (listtype arguments)
#else

objecttype ref_type (arguments)
listtype arguments;
#endif

  {
    objecttype obj;
    objecttype result;

  /* ref_type */
    isit_reference(arg_1(arguments));
    obj = take_reference(arg_1(arguments));
    /* printf("obj: ");
    trace1(obj);
    printf("\n"); */
    if (obj == NULL) {
      result = raise_exception(SYS_RNG_EXCEPTION);
    } else if (obj->type_of == NULL) {
      printf("ref_type - obj: ");
      trace1(obj);
      printf("\n");
      result = raise_exception(SYS_RNG_EXCEPTION);
    } else {
      result = bld_type_temp(obj->type_of);
    } /* if */
    return(result);
  } /* ref_type */



#ifdef ANSI_C

objecttype ref_value (listtype arguments)
#else

objecttype ref_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;

  /* ref_value */
    isit_reference(arg_3(arguments));
    obj_arg = take_reference(arg_3(arguments));
    if (CLASS_OF_OBJ(obj_arg) == FWDREFOBJECT ||
        CLASS_OF_OBJ(obj_arg) == REFOBJECT ||
        CLASS_OF_OBJ(obj_arg) == REFPARAMOBJECT ||
        CLASS_OF_OBJ(obj_arg) == RESULTOBJECT ||
        CLASS_OF_OBJ(obj_arg) == LOCALVOBJECT ||
        CLASS_OF_OBJ(obj_arg) == ENUMLITERALOBJECT ||
        CLASS_OF_OBJ(obj_arg) == CONSTENUMOBJECT ||
        CLASS_OF_OBJ(obj_arg) == VARENUMOBJECT) {
      return(bld_reference_temp(take_reference(obj_arg)));
    } else {
      run_error(REFOBJECT, obj_arg);
      return(bld_reference_temp(NULL));
    } /* if */
  } /* ref_value */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

objecttype ref_value (listtype arguments)
#else

objecttype ref_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg1;
    stritype str1;
    listtype lst;
    errinfotype err_info = OKAY_NO_ERROR;
    objecttype result;

  /* ref_value */
    isit_reference(arg_1(arguments));
    obj_arg1 = take_reference(arg_1(arguments));
/*  printf("ref_value(");
    printclass(CLASS_OF_OBJ(obj_arg1));
    printf(")\n"); */
    switch (CLASS_OF_OBJ(obj_arg1)) {
      case INTOBJECT:
        result = bld_int_temp(take_int(obj_arg1));
        break;
      case CHAROBJECT:
        result = bld_char_temp(take_char(obj_arg1));
        break;
      case STRIOBJECT:
        if (!ALLOC_STRI(str1, take_stri(obj_arg1)->size)) {
          return(raise_exception(SYS_MEM_EXCEPTION));
        } /* if */
        COUNT_STRI(take_stri(obj_arg1)->size);
        str1->size = take_stri(obj_arg1)->size;
        memcpy(str1->mem, take_stri(obj_arg1)->mem,
            (SIZE_TYPE) (str1->size * sizeof(strelemtype)));
        result = bld_stri_temp(str1);
        break;
      case ARRAYOBJECT:
      case STRUCTOBJECT:
        if (!array_to_list(take_array(obj_arg1), &lst)) {
          return(raise_exception(SYS_MEM_EXCEPTION));
        } /* if */
        result = bld_reflist_temp(lst);
        break;
      case FILEOBJECT:
        result = bld_file_temp(take_file(obj_arg1));
        break;
#ifdef WITH_FLOAT
      case FLOATOBJECT:
        result = bld_float_temp((double) take_float(obj_arg1));
        break;
#endif
      case LISTOBJECT:
      case EXPROBJECT:
      case MATCHOBJECT:
      case CALLOBJECT:
        copy_list(take_list(obj_arg1), &lst, &err_info);
        if (err_info != OKAY_NO_ERROR) {
          return(raise_exception(SYS_MEM_EXCEPTION));
        } /* if */
        result = bld_reflist_temp(lst);
        break;
      case ACTOBJECT:
        result = bld_action_temp(take_action(obj_arg1));
        break;
      case REFOBJECT:
      case ENUMLITERALOBJECT:
      case CONSTENUMOBJECT:
      case VARENUMOBJECT:
      case LOCALVOBJECT:
      case RESULTOBJECT:
        result = bld_reference_temp(take_reference(obj_arg1));
        break;
      case TYPEOBJECT:
        result = obj_arg1;
        break;
      default:
        printf("value(");
        trace1(obj_arg1);
        printf(")\n");
        result = obj_arg1;
/*      result = bld_reference_temp(obj_arg1); */
/*      result = bld_reference_temp((objecttype) NULL); */
        break;
    } /* switch */
    return(result);
  } /* ref_value */
#endif
