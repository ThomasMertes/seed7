/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000, 2011, 2013, 2015  Thomas Mertes      */
/*                2017, 2024, 2025  Thomas Mertes                   */
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
/*  Module: General                                                 */
/*  File: seed7/src/blockutl.c                                      */
/*  Changes: 1992, 1993, 1994, 2011, 2013, 2015  Thomas Mertes      */
/*           2017, 2024, 2025  Thomas Mertes                        */
/*  Content: Functions to maintain objects of type blockType.       */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "listutl.h"
#include "entutl.h"
#include "syvarutl.h"
#include "executl.h"
#include "traceutl.h"
#include "objutl.h"
#include "match.h"

#undef EXTERN
#define EXTERN
#include "blockutl.h"



void free_locobj (const_locObjType locobj)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* free_locobj */
    logFunction(printf("free_locobj(");
                trace1(locobj->object);
                printf(")\n"););
    if (locobj->object != NULL) {
      if (CATEGORY_OF_OBJ(locobj->object) == VALUEPARAMOBJECT ||
          CATEGORY_OF_OBJ(locobj->object) == REFPARAMOBJECT ||
          CATEGORY_OF_OBJ(locobj->object) == RESULTOBJECT ||
          CATEGORY_OF_OBJ(locobj->object) == LOCALVOBJECT) {
        if (locobj->init_value != NULL) {
          if (CATEGORY_OF_OBJ(locobj->object) == RESULTOBJECT) {
            /* printf("free_locobj: RESULTOBJECT init category %u\n",
               CATEGORY_OF_OBJ(locobj->init_value)); */
            free_expression(locobj->init_value);
          } else {
            /* prot_cstri("free_locobj value ");
            prot_int((intType) locobj->init_value);
            prot_cstri(" ");
            trace1(locobj->init_value);
            prot_nl(); */
            destroy_local_init_value(locobj, &err_info);
            FREE_OBJECT(locobj->init_value);
            /* dump_any_temp(locobj->init_value); */
          } /* if */
        } /* if */
        /* if (locobj->object->value.objValue != NULL &&
            CATEGORY_OF_OBJ(locobj->object->value.objValue) != SYMBOLOBJECT) {
          trace1(locobj->object);
          prot_nl();
        } * if */
        if (CATEGORY_OF_OBJ(locobj->object) != VALUEPARAMOBJECT &&
            CATEGORY_OF_OBJ(locobj->object) != REFPARAMOBJECT) {
          /* Parameters are freed by the function free_params() */
          /* which is called from close_current_stack() and     */
          /* free_local_consts().                               */
          if (HAS_PROPERTY(locobj->object) &&
              locobj->object->descriptor.property != prog->property.literal) {
            FREE_PROPERTY(locobj->object->descriptor.property);
          } /* if */
          FREE_OBJECT(locobj->object);
        } /* if */
      /* } else if (CATEGORY_OF_OBJ(locobj->object) != SYMBOLOBJECT) {
        printf("####### ");
        trace1(locobj->object);
        prot_nl(); */
      } /* if */
    } /* if */
  } /* free_locobj */



static locListType reverse_loclist (locListType loclist_element)

  {
    locListType next_elem;
    locListType reversed_loclist;

  /* reverse_loclist */
    if (loclist_element != NULL) {
      reversed_loclist = loclist_element;
      loclist_element = loclist_element->next;
      reversed_loclist->next = NULL;
      while (loclist_element != NULL) {
        next_elem = loclist_element->next;
        loclist_element->next = reversed_loclist;
        reversed_loclist = loclist_element;
        loclist_element = next_elem;
      } /* while */
    } else {
      reversed_loclist = NULL;
    } /* if */
    return reversed_loclist;
  } /* reverse_loclist */



void free_loclist (locListType loclist)

  {
    locListType reversed_loclist;
    locListType old_elem;

  /* free_loclist */
    logFunction(printf("free_loclist(" FMT_U_MEM ")\n",
                       (memSizeType) loclist););
    reversed_loclist = reverse_loclist(loclist);
    while (reversed_loclist != NULL) {
      free_locobj(&reversed_loclist->local);
      old_elem = reversed_loclist;
      reversed_loclist = reversed_loclist->next;
      FREE_RECORD(old_elem, locListRecord, count.loclist);
    } /* while */
    logFunction(printf("free_loclist -->\n"););
  } /* free_loclist */



void free_local_consts (listType list)

  {
    listType list_end;
    listType reversed_list;
    listType list_element;

  /* free_local_consts */
    logFunction(printf("free_local_consts(" FMT_U_MEM ")\n",
                       (memSizeType) list););
    if (list != NULL) {
      list_end = list;
      reversed_list= reverse_list(list);
      list_element = reversed_list;
      do {
        dump_temp_value(list_element->obj);
        list_element = list_element->next;
      } while (list_element != NULL);
      list_element = reversed_list;
      do {
        if (HAS_PROPERTY(list_element->obj) &&
            list_element->obj->descriptor.property !=
            prog->property.literal) {
          /* In itf_destr() the STRUCTOBJECT value of an        */
          /* interface is freed when usage_count reaches zero.  */
          /* STRUCTOBJECT values with properties are treated    */
          /* special by itf_destr(). In this case the struct    */
          /* value of the STRUCTOBJECT is freed and set to NULL */
          /* but the STRUCTOBJECT and its properties are left   */
          /* intact. In this case sct_destr() just sets the     */
          /* UNUSED flag of the STRUCTOBJECT. The object and    */
          /* the descriptor.property are freed here.            */
          free_params(prog,
                      list_element->obj->descriptor.property->params);
          FREE_PROPERTY(list_element->obj->descriptor.property);
        } /* if */
        FREE_OBJECT(list_element->obj);
        list_element = list_element->next;
      } while (list_element != NULL);
      free_list2(reversed_list, list_end);
    } /* if */
  } /* free_local_consts */



void free_block (blockType block)

  { /* free_block */
    logFunction(printf("free_block(" FMT_U_MEM ")\n",
                       (memSizeType) block););
    free_expression(block->body);
    free_local_consts(block->local_consts);
    free_loclist(block->params);
    free_locobj(&block->result);
    free_loclist(block->local_vars);
    FREE_RECORD(block, blockRecord, count.block);
    logFunction(printf("free_block(" FMT_U_MEM ") -->\n",
                       (memSizeType) block););
  } /* free_block */



blockType new_block (locListType block_params, const_locObjType block_result,
    locListType block_local_vars, listType block_local_consts,
    objectType block_body)

  {
    register blockType created_block;

  /* new_block */
    logFunction(printf("new_block(" FMT_U_MEM ")\n", (memSizeType) block_params););
    if (ALLOC_RECORD(created_block, blockRecord, count.block)) {
      created_block->usage_count = 1;
      created_block->params = block_params;
      if (block_result == NULL) {
        created_block->result.object           = NULL;
        created_block->result.init_value       = NULL;
        created_block->result.create_call_obj  = NULL;
        created_block->result.destroy_call_obj = NULL;
      } else {
        created_block->result.object           = block_result->object;
        created_block->result.init_value       = block_result->init_value;
        created_block->result.create_call_obj  = block_result->create_call_obj;
        created_block->result.destroy_call_obj = block_result->destroy_call_obj;
      } /* if */
      created_block->local_vars = block_local_vars;
      created_block->local_consts = block_local_consts;
      created_block->body = block_body;
    } /* if */
    logFunction(printf("new_block --> " FMT_U_MEM "\n", (memSizeType) created_block););
    return created_block;
  } /* new_block */



static void append_to_loclist (locListType **list_insert_place, objectType object,
    objectType init_value, objectType create_call_obj,
    objectType destroy_call_obj, errInfoType *err_info)

  {
    register locListType help_element;

  /* append_to_loclist */
    logFunction(printf("append_to_loclist(" FMT_U_MEM ", " FMT_U_MEM ", "
                       FMT_U_MEM ", " FMT_U_MEM ", " FMT_U_MEM ", %d)\n",
                       (memSizeType) **list_insert_place,
                       (memSizeType) object,
                       (memSizeType) init_value,
                       (memSizeType) create_call_obj,
                       (memSizeType) destroy_call_obj, *err_info););
    if (ALLOC_RECORD(help_element, locListRecord, count.loclist)) {
      help_element->next = NULL;
      help_element->local.object = object;
      help_element->local.init_value = init_value;
      help_element->local.create_call_obj = create_call_obj;
      help_element->local.destroy_call_obj = destroy_call_obj;
      **list_insert_place = help_element;
      *list_insert_place = &help_element->next;
    } else {
      *err_info = MEMORY_ERROR;
    } /* if */
  } /* append_to_loclist */



void get_result_var (locObjType result_var, typeType result_type,
    objectType result_init, errInfoType *err_info)

  { /* get_result_var */
    logFunction(printf("get_result_var\n"););
    result_init = copy_expression(result_init, err_info);
    if (CATEGORY_OF_OBJ(result_init) == MATCHOBJECT) {
      SET_CATEGORY_OF_OBJ(result_init, CALLOBJECT);
    } /* if */
    result_var->object->type_of = result_type;
    INIT_CATEGORY_OF_VAR(result_var->object, RESULTOBJECT);
    result_var->object->value.objValue = NULL;
    result_var->init_value = result_init;
    result_var->create_call_obj = get_create_call_obj(result_var->object, err_info);
    result_var->destroy_call_obj = get_destroy_call_obj(result_var->object, err_info);
    logFunction(printf("get_result_var -->\n"););
  } /* get_result_var */



void get_return_var (locObjType return_var, typeType return_type,
    errInfoType *err_info)

  {
      objectRecord return_object;

  /* get_return_var */
    logFunction(printf("get_return_var\n"););
    return_object.type_of = return_type;
    return_object.descriptor.property = NULL;
    INIT_CATEGORY_OF_VAR(&return_object, RESULTOBJECT);
    return_object.value.objValue = NULL;
    return_var->object = NULL;
    return_var->init_value = NULL;
    return_var->create_call_obj = get_create_call_obj(&return_object, err_info);
    return_var->destroy_call_obj = get_destroy_call_obj(&return_object, err_info);
    logFunction(printf("get_return_var -->\n"););
  } /* get_return_var */



locListType get_param_list (const_listType param_object_list,
    errInfoType *err_info)

  {
    locListType *params_insert_place;
    const_listType param_element;
    objectType create_call_obj;
    objectType destroy_call_obj;
    locListType params;

  /* get_param_list */
    logFunction(printf("get_param_list\n"););
    params = NULL;
    params_insert_place = &params;
    param_element = param_object_list;
    while (param_element != NULL) {
      /* printf("get_param_list: ");
      trace1(param_element->obj);
      printf("\n"); */
      if (CATEGORY_OF_OBJ(param_element->obj) == VALUEPARAMOBJECT) {
        if (param_element->obj->type_of->create_call_obj != NULL) {
          create_call_obj = param_element->obj->type_of->create_call_obj;
        } else {
          create_call_obj = get_create_call_obj(param_element->obj, err_info);
        } /* if */
        if (param_element->obj->type_of->destroy_call_obj != NULL) {
          destroy_call_obj = param_element->obj->type_of->destroy_call_obj;
        } else {
          destroy_call_obj = get_destroy_call_obj(param_element->obj, err_info);
        } /* if */
      } else {
        create_call_obj = NULL;
        destroy_call_obj = NULL;
      } /* if */
      append_to_loclist(&params_insert_place,
          param_element->obj, NULL, create_call_obj, destroy_call_obj, err_info);
      param_element = param_element->next;
    } /* while */
    logFunction(printf("get_param_list -->\n"););
    return params;
  } /* get_param_list */



locListType get_local_var_list (const_listType local_object_list,
    errInfoType *err_info)

  {
    locListType *local_vars_insert_place;
    const_listType local_element;
    objectType local_var;
    objectType init_value;
    objectType create_call_obj;
    objectType destroy_call_obj;
    locListType local_vars;

  /* get_local_var_list */
    logFunction(printf("get_local_var_list(" FMT_X_MEM ", %d)\n",
                       (memSizeType) local_object_list,
                       *err_info););
    local_vars = NULL;
    local_vars_insert_place = &local_vars;
    local_element = local_object_list;
    while (local_element != NULL) {
      if (VAR_OBJECT(local_element->obj)) {
        local_var = local_element->obj;
#ifdef OUT_OF_ORDER
        if (CATEGORY_OF_OBJ(local_var) != LOCALVOBJECT) {
          if (ALLOC_OBJECT(init_value)) {
            init_value->type_of =     local_var->type_of;
            init_value->descriptor.property = NULL;
            init_value->value =       local_var->value;
            init_value->objcategory = local_var->objcategory;
            SET_CATEGORY_OF_OBJ(local_var, LOCALVOBJECT);
            local_var->value.objValue = init_value; /* was NULL; changed for s7c.sd7 */
          } else {
            *err_info = MEMORY_ERROR;
          } /* if */
        } else {
          /* printf("B "); trace1(local_var); printf("\n"); */
        } /* if */
#endif
        if (CATEGORY_OF_OBJ(local_var) == LOCALVOBJECT) {
          init_value = local_var->value.objValue;
          create_call_obj = get_create_call_obj(init_value, err_info);
          destroy_call_obj = get_destroy_call_obj(init_value, err_info);
          append_to_loclist(&local_vars_insert_place,
              local_var, init_value, create_call_obj, destroy_call_obj, err_info);
        } /* if */
#ifdef OUT_OF_ORDER
        } else if (ALLOC_OBJECT(init_value)) {
          init_value->type_of =     local_var->type_of;
          init_value->descriptor.property = NULL;
          init_value->value =       local_var->value;
          init_value->objcategory = local_var->objcategory;
          create_call_obj = get_create_call_obj(local_var, err_info);
          destroy_call_obj = get_destroy_call_obj(local_var, err_info);
          SET_CATEGORY_OF_OBJ(local_var, LOCALVOBJECT);
          local_var->value.objValue = init_value; /* was NULL; changed for s7c.sd7 */
          append_to_loclist(&local_vars_insert_place,
              local_var, init_value, create_call_obj, destroy_call_obj, err_info);
        } else {
          *err_info = MEMORY_ERROR;
        } /* if */
#endif
      } /* if */
      local_element = local_element->next;
    } /* while */
    logFunction(printf("get_local_var_list --> (err_info=%d)\n",
                       *err_info););
    return local_vars;
  } /* get_local_var_list */



listType get_local_const_list (const_listType local_object_list,
    errInfoType *err_info)

  {
    listType *list_insert_place;
    const_listType local_element;
    listType local_consts;

  /* get_local_const_list */
    logFunction(printf("get_local_const_list(" FMT_X_MEM ")\n",
                       (memSizeType) local_object_list););
    local_consts = NULL;
    list_insert_place = &local_consts;
    local_element = local_object_list;
    while (local_element != NULL) {
      if (!VAR_OBJECT(local_element->obj)) {
        list_insert_place = append_element_to_list(list_insert_place,
            local_element->obj, err_info);
      } /* if */
      local_element = local_element->next;
    } /* while */
    logFunction(printf("get_local_const_list -->\n"););
    return local_consts;
  } /* get_local_const_list */
