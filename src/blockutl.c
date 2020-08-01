/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Procedures to maintain objects of type blocktype.      */
/*                                                                  */
/********************************************************************/

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
#include "match.h"
#include "memory.h"

#undef EXTERN
#define EXTERN
#include "blockutl.h"



#ifdef ANSI_C

static void free_locobj (const_locobjtype locobj)
#else

static void free_locobj (locobj)
locobjtype locobj;
#endif

  { /* free_locobj */
    /* prot_heapsize();
    prot_cstri(" free_locobj ");
    trace1(locobj->object);
    prot_nl(); */
    if (locobj->object != NULL) {
      if (CATEGORY_OF_OBJ(locobj->object) == VALUEPARAMOBJECT ||
          CATEGORY_OF_OBJ(locobj->object) == REFPARAMOBJECT ||
          CATEGORY_OF_OBJ(locobj->object) == RESULTOBJECT ||
          CATEGORY_OF_OBJ(locobj->object) == LOCALVOBJECT) {
        if (CATEGORY_OF_OBJ(locobj->object) != RESULTOBJECT &&
            locobj->init_value != NULL) {
          if (CATEGORY_OF_OBJ(locobj->init_value) != ARRAYOBJECT &&
              CATEGORY_OF_OBJ(locobj->init_value) != STRUCTOBJECT &&
              CATEGORY_OF_OBJ(locobj->init_value) != TYPEOBJECT) {
            /* prot_heapsize();
            prot_cstri(" free_locobj value ");
            prot_int((inttype) locobj->init_value);
            prot_cstri(" ");
            trace1(locobj->init_value);
            prot_nl(); */
            dump_any_temp(locobj->init_value);
          } /* if */
        } /* if */
        FREE_OBJECT(locobj->object);
      } /* if */
    } /* if */
  } /* free_locobj */



#ifdef ANSI_C

static void free_loclist (loclisttype loclist)
#else

static void free_loclist (loclist)
loclisttype loclist;
#endif

  {
    loclisttype old_loclist;

  /* free_loclist */
    /* prot_heapsize();
    prot_cstri(" free_loclist");
    prot_nl(); */
    while (loclist != NULL) {
      free_locobj(&loclist->local);
      old_loclist = loclist;
      loclist = loclist->next;
      FREE_RECORD(old_loclist, loclistrecord, count.loclist);
    } /* if */
  } /* free_loclist */



#ifdef ANSI_C

void free_block (blocktype block)
#else

void free_block (block)
blocktype block;
#endif

  { /* free_block */
    /* prot_heapsize();
    prot_cstri(" free_block");
    prot_nl(); */
    free_loclist(block->params);
    free_locobj(&block->result);
    free_loclist(block->local_vars);
    emptylist(block->local_consts); /* objects are not freed */
    /* block->body */
    FREE_RECORD(block, blockrecord, count.block);
  } /* free_block */



#ifdef ANSI_C

blocktype new_block (loclisttype block_params, const_locobjtype block_result,
    loclisttype block_local_vars, listtype block_local_consts,
    objecttype block_body)
#else

blocktype new_block (block_params, block_result,
    block_local_vars,  block_local_consts, block_body)
loclisttype block_params;
locobjtype block_result;
loclisttype block_local_vars;
listtype block_local_consts;
objecttype block_body;
#endif

  {
    register blocktype created_block;

  /* new_block */
#ifdef TRACE_BLOCK
    printf("BEGIN new_block\n");
#endif
    if (ALLOC_RECORD(created_block, blockrecord, count.block)) {
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
#ifdef TRACE_BLOCK
    printf("END new_block\n");
#endif
    return(created_block);
  } /* new_block */



#ifdef ANSI_C

static void append_to_loclist (loclisttype **list_insert_place, objecttype object,
    objecttype init_value, objecttype create_call_obj,
    objecttype destroy_call_obj, errinfotype *err_info)
#else

static void append_to_loclist (list_insert_place, object,
    init_value, create_call_obj, destroy_call_obj, err_info)
loclisttype **list_insert_place;
objecttype object;
objecttype init_value;
objecttype create_call_obj;
objecttype destroy_call_obj;
errinfotype *err_info;
#endif

  {
    register loclisttype help_element;

  /* append_to_loclist */
    if (ALLOC_RECORD(help_element, loclistrecord, count.loclist)) {
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



#ifdef ANSI_C

void get_result_var (locobjtype result_var, typetype result_type,
    objecttype result_init, errinfotype *err_info)
#else

void get_result_var (result_var, result_type, result_init, err_info)
locobjtype result_var;
typetype result_type;
objecttype result_init;
errinfotype *err_info;
#endif

  { /* get_result_var */
#ifdef TRACE_BLOCK
    printf("BEGIN get_result_var\n");
#endif
    if (CATEGORY_OF_OBJ(result_init) == MATCHOBJECT) {
      SET_CATEGORY_OF_OBJ(result_init, CALLOBJECT);
    } /* if */
    result_var->object->type_of = result_type;
    INIT_CATEGORY_OF_VAR(result_var->object, RESULTOBJECT);
    result_var->object->value.objvalue = NULL;
    result_var->init_value = result_init;
    result_var->create_call_obj = get_create_call_obj(result_var->object, err_info);
    result_var->destroy_call_obj = get_destroy_call_obj(result_var->object, err_info);
#ifdef TRACE_BLOCK
    printf("END get_result_var\n");
#endif
  } /* get_result_var */



#ifdef ANSI_C

void get_return_var (locobjtype return_var, typetype return_type,
    errinfotype *err_info)
#else

void get_return_var (return_var, return_type, err_info)
locobjtype return_var;
typetype return_type;
errinfotype *err_info;
#endif

  {
      objectrecord return_object;

  /* get_return_var */
#ifdef TRACE_BLOCK
    printf("BEGIN get_return_var\n");
#endif
    return_object.type_of = return_type;
    return_object.descriptor.property = NULL;
    INIT_CATEGORY_OF_VAR(&return_object, RESULTOBJECT);
    return_object.value.objvalue = NULL;
    return_var->object = NULL;
    return_var->init_value = NULL;
    return_var->create_call_obj = get_create_call_obj(&return_object, err_info);
    return_var->destroy_call_obj = get_destroy_call_obj(&return_object, err_info);
#ifdef TRACE_BLOCK
    printf("END get_return_var\n");
#endif
  } /* get_return_var */



#ifdef ANSI_C

loclisttype get_param_list (const_listtype param_object_list,
    errinfotype *err_info)
#else

loclisttype get_param_list (param_object_list, err_info)
listtype param_object_list;
errinfotype *err_info;
#endif

  {
    loclisttype *params_insert_place;
    const_listtype param_element;
    objecttype create_call_obj;
    objecttype destroy_call_obj;
    loclisttype params;

  /* get_param_list */
#ifdef TRACE_BLOCK
    printf("BEGIN get_param_list\n");
#endif
    params = NULL;
    params_insert_place = &params;
    param_element = param_object_list;
    while (param_element != NULL) {
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
#ifdef TRACE_BLOCK
    printf("END get_param_list\n");
#endif
    return(params);
  } /* get_param_list */



#ifdef ANSI_C

loclisttype get_local_var_list (const_listtype local_object_list,
    errinfotype *err_info)
#else

loclisttype get_local_var_list (local_object_list, err_info)
listtype local_object_list;
errinfotype *err_info;
#endif

  {
    loclisttype *local_vars_insert_place;
    const_listtype local_element;
    objecttype local_var;
    objecttype init_value;
    objecttype create_call_obj;
    objecttype destroy_call_obj;
    loclisttype local_vars;

  /* get_local_var_list */
#ifdef TRACE_BLOCK
    printf("BEGIN get_local_var_list\n");
#endif
    local_vars = NULL;
    local_vars_insert_place = &local_vars;
    local_element = local_object_list;
    while (local_element != NULL) {
      if (VAR_OBJECT(local_element->obj)) {
        local_var = local_element->obj;
        if (ALLOC_OBJECT(init_value)) {
          init_value->type_of =     local_var->type_of;
          init_value->descriptor.property = NULL;
          init_value->value =       local_var->value;
          init_value->objcategory = local_var->objcategory;
          create_call_obj = get_create_call_obj(local_var, err_info);
          destroy_call_obj = get_destroy_call_obj(local_var, err_info);
          SET_CATEGORY_OF_OBJ(local_var, LOCALVOBJECT);
          local_var->value.objvalue = init_value; /* was NULL; changed for comp.sd7 */
          append_to_loclist(&local_vars_insert_place,
              local_var, init_value, create_call_obj, destroy_call_obj, err_info);
        } else {
          *err_info = MEMORY_ERROR;
        } /* if */
      } /* if */
      local_element = local_element->next;
    } /* while */
#ifdef TRACE_BLOCK
    printf("END get_local_var_list\n");
#endif
    return(local_vars);
  } /* get_local_var_list */



#ifdef ANSI_C

listtype get_local_const_list (const_listtype local_object_list,
    errinfotype *err_info)
#else

listtype get_local_const_list (local_object_list, err_info)
listtype local_object_list;
errinfotype *err_info;
#endif

  {
    listtype *list_insert_place;
    const_listtype local_element;
    listtype local_consts;

  /* get_local_const_list */
#ifdef TRACE_BLOCK
    printf("BEGIN get_local_const_list\n");
#endif
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
#ifdef TRACE_BLOCK
    printf("END get_local_const_list\n");
#endif
    return(local_consts);
  } /* get_local_const_list */
