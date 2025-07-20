/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013, 2015, 2021  Thomas Mertes            */
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
/*  File: seed7/src/typeutl.c                                       */
/*  Changes: 1999, 2013, 2015, 2021  Thomas Mertes                  */
/*  Content: Functions to maintain objects of type typeType.        */
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
#include "identutl.h"
#include "entutl.h"
#include "traceutl.h"

#undef EXTERN
#define EXTERN
#include "typeutl.h"



typeType new_type (progType owningProg, typeType meta_type, typeType result_type)

  {
    objectType match_obj;
    listType list_elem;
    typeType created_type;

  /* new_type */
    logFunction(printf("new_type(" FMT_U_MEM ", ", (memSizeType) owningProg);
                printtype(meta_type);
                printf(", ");
                printtype(result_type);
                printf(")\n"););
    if (ALLOC_OBJECT(match_obj)) {
      if (ALLOC_L_ELEM(list_elem)) {
        if (ALLOC_RECORD(created_type, typeRecord, count.type)) {
          match_obj->type_of = NULL;
          match_obj->descriptor.property = NULL;
          match_obj->value.typeValue = created_type;
          INIT_CATEGORY_OF_OBJ(match_obj, TYPEOBJECT);
          created_type->match_obj = match_obj;
          created_type->meta = meta_type;
          created_type->func_type = NULL;
          created_type->varfunc_type = NULL;
          created_type->result_type = result_type;
          created_type->is_varfunc_type = FALSE;
          created_type->is_type_type = FALSE;
          if (meta_type != NULL) {
            created_type->in_param_type = meta_type->in_param_type;
          } else {
            created_type->in_param_type = PARAM_UNDEFINED;
          } /* if */
          created_type->interfaces = NULL;
          created_type->name = NULL;
          created_type->owningProg = owningProg;
          created_type->inout_f_param_prototype = NULL;
          created_type->other_f_param_prototype = NULL;
          created_type->create_call_obj = NULL;
          created_type->destroy_call_obj = NULL;
          created_type->copy_call_obj = NULL;
          created_type->ord_call_obj = NULL;
          created_type->in_call_obj = NULL;
          created_type->value_obj = NULL;
          list_elem->obj = match_obj;
          list_elem->next = owningProg->types;
          owningProg->types = list_elem;
        } else {
          FREE_L_ELEM(list_elem);
          FREE_OBJECT(match_obj);
          created_type = NULL;
        } /* if */
      } else {
        FREE_OBJECT(match_obj);
        created_type = NULL;
      } /* if */
    } else {
      created_type = NULL;
    } /* if */
    logFunction(printf("new_type --> ");
                printtype(created_type);
                printf("\n"););
    return created_type;
  } /* new_type */



static void free_type (typeType old_type)

  { /* free_type */
    logFunction(printf("free_type(");
                printtype(old_type);
                printf(")\n"););
    FREE_OBJECT(old_type->match_obj);
    free_list(old_type->interfaces);
    FREE_RECORD(old_type, typeRecord, count.type);
    logFunction(printf("free_type -->\n"););
  } /* free_type */



void close_type (progType currentProg)

  {
    listType type_elem;
    listType next_elem;

  /* close_type */
    logFunction(printf("close_type\n"););
    type_elem = currentProg->types;
    while (type_elem != NULL) {
      next_elem = type_elem->next;
      free_type(type_elem->obj->value.typeValue);
      type_elem = next_elem;
    } /* while */
    free_list(currentProg->types);
    logFunction(printf("close_type -->\n"););
  } /* close_type */



typeType get_func_type (typeType meta_type, typeType basic_type)

  {
    typeType func_type;

  /* get_func_type */
    logFunction(printf("get_func_type(");
                printtype(meta_type);
                printf(", ");
                printtype(basic_type);
                printf(")\n"););
    if (basic_type->func_type != NULL) {
      func_type = basic_type->func_type;
    } else {
      func_type = new_type(basic_type->owningProg, meta_type, basic_type);
      basic_type->func_type = func_type;
    } /* if */
    logFunction(printf("get_func_type --> ");
                printtype(func_type);
                printf("\n"););
    return func_type;
  } /* get_func_type */



typeType get_varfunc_type (typeType meta_type, typeType basic_type)

  {
    typeType varfunc_type;

  /* get_varfunc_type */
    logFunction(printf("get_varfunc_type(");
                printtype(meta_type);
                printf(", ");
                printtype(basic_type);
                printf(")\n"););
    if (basic_type->varfunc_type != NULL) {
      varfunc_type = basic_type->varfunc_type;
    } else {
      varfunc_type = new_type(basic_type->owningProg, meta_type, basic_type);
      if (varfunc_type != NULL) {
        varfunc_type->is_varfunc_type = TRUE;
      } /* if */
      basic_type->varfunc_type = varfunc_type;
    } /* if */
    logFunction(printf("get_varfunc_type --> ");
                printtype(varfunc_type);
                printf("\n"););
    return varfunc_type;
  } /* get_varfunc_type */



void add_interface (typeType basic_type, typeType interface_type)

  {
    listType new_list_elem;
    listType last_elem;

  /* add_interface */
    logFunction(printf("add_interface(");
                printtype(basic_type);
                printf(", ");
                printtype(interface_type);
                printf(")\n"););
    if (ALLOC_L_ELEM(new_list_elem)) {
      new_list_elem->next = NULL;
      new_list_elem->obj = interface_type->match_obj;
      if (basic_type->interfaces == NULL) {
        basic_type->interfaces = new_list_elem;
      } else {
        last_elem = basic_type->interfaces;
        while (last_elem->next != NULL) {
          last_elem = last_elem->next;
        } /* while */
        last_elem->next = new_list_elem;
      } /* if */
    } /* if */
    logFunction(printf("add_interface\n"););
  } /* add_interface */
