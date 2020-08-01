/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  Changes: 1999  Thomas Mertes                                    */
/*  Content: Procedures to maintain objects of type typetype.       */
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
#include "identutl.h"
#include "entutl.h"
#include "traceutl.h"

#undef EXTERN
#define EXTERN
#include "typeutl.h"

#undef TRACE_TYPEUTIL
#undef TRACE_FUNC_TYPE



typetype new_type (progtype owningProg, typetype meta_type, typetype result_type)

  {
    objecttype match_obj;
    listtype list_elem;
    typetype created_type;

  /* new_type */
#ifdef TRACE_TYPEUTIL
    printf("BEGIN new_type(%lx, ", (unsigned long) owningProg);
    printtype(meta_type);
    printf(", ");
    printtype(result_type);
    printf(")\n");
#endif
    if (ALLOC_OBJECT(match_obj)) {
      if (ALLOC_L_ELEM(list_elem)) {
        if (ALLOC_RECORD(created_type, typerecord, count.type)) {
          match_obj->type_of = NULL;
          match_obj->descriptor.property = NULL;
          match_obj->value.typevalue = created_type;
          INIT_CATEGORY_OF_OBJ(match_obj, TYPEOBJECT);
          created_type->match_obj = match_obj;
          created_type->meta = meta_type;
          created_type->func_type = NULL;
          created_type->varfunc_type = NULL;
          created_type->result_type = result_type;
          created_type->is_varfunc_type = FALSE;
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
#ifdef TRACE_TYPEUTIL
    printf("END new_type --> ");
    printtype(created_type);
    printf("\n");
#endif
    return created_type;
  } /* new_type */



static void free_type (typetype old_type)

  {
    typelisttype typelist_elem;
    typelisttype next_elem;

  /* free_type */
#ifdef TRACE_TYPEUTIL
    printf("BEGIN free_type(");
    printtype(old_type);
    printf(")\n");
#endif
    FREE_OBJECT(old_type->match_obj);
    typelist_elem = old_type->interfaces;
    while (typelist_elem != NULL) {
      next_elem = typelist_elem->next;
      FREE_RECORD(typelist_elem, typelistrecord, count.typelist_elems);
      typelist_elem = next_elem;
    } /* while */
    FREE_RECORD(old_type, typerecord, count.type);
#ifdef TRACE_TYPEUTIL
    printf("END free_type\n");
#endif
  } /* free_type */



void close_type (progtype currentProg)

  {
    listtype type_elem;
    listtype next_elem;

  /* close_type */
#ifdef TRACE_TYPEUTIL
    printf("BEGIN close_type\n");
#endif
    type_elem = currentProg->types;
    while (type_elem != NULL) {
      next_elem = type_elem->next;
      free_type(type_elem->obj->value.typevalue);
      type_elem = next_elem;
    } /* while */
    free_list(currentProg->types);
#ifdef TRACE_TYPEUTIL
    printf("END close_type\n");
#endif
  } /* close_type */



typetype get_func_type (typetype meta_type, typetype basic_type)

  {
    typetype func_type;

  /* get_func_type */
#ifdef TRACE_FUNC_TYPE
    printf("BEGIN get_func_type(");
    printtype(meta_type);
    printf(", ");
    printtype(basic_type);
    printf(")\n");
#endif
    if (basic_type->func_type != NULL) {
      func_type = basic_type->func_type;
    } else {
      func_type = new_type(basic_type->owningProg, meta_type, basic_type);
      basic_type->func_type = func_type;
    } /* if */
#ifdef TRACE_FUNC_TYPE
    printf("END get_func_type --> ");
    printtype(func_type);
    printf("\n");
#endif
    return func_type;
  } /* get_func_type */



typetype get_varfunc_type (typetype meta_type, typetype basic_type)

  {
    typetype varfunc_type;

  /* get_varfunc_type */
#ifdef TRACE_FUNC_TYPE
    printf("BEGIN get_varfunc_type(");
    printtype(meta_type);
    printf(", ");
    printtype(basic_type);
    printf(")\n");
#endif
    if (basic_type->varfunc_type != NULL) {
      varfunc_type = basic_type->varfunc_type;
    } else {
      varfunc_type = new_type(basic_type->owningProg, meta_type, basic_type);
      varfunc_type->is_varfunc_type = TRUE;
      basic_type->varfunc_type = varfunc_type;
    } /* if */
#ifdef TRACE_FUNC_TYPE
    printf("END get_varfunc_type --> ");
    printtype(varfunc_type);
    printf("\n");
#endif
    return varfunc_type;
  } /* get_varfunc_type */



void add_interface (typetype basic_type, typetype interface_type)

  {
    typelisttype typelist_elem;
    typelisttype current_elem;

  /* add_interface */
#ifdef TRACE_TYPEUTIL
    printf("BEGIN add_interface(");
    printtype(basic_type);
    printf(", ");
    printtype(interface_type);
    printf(")\n");
#endif
    if (ALLOC_RECORD(typelist_elem, typelistrecord, count.typelist_elems)) {
      typelist_elem->next = NULL;
      typelist_elem->type_elem = interface_type;
      if (basic_type->interfaces == NULL) {
        basic_type->interfaces = typelist_elem;
      } else {
        current_elem = basic_type->interfaces;
        while (current_elem->next != NULL) {
          current_elem = current_elem->next;
        } /* while */
        current_elem->next = typelist_elem;
      } /* if */
    } /* if */
#ifdef TRACE_TYPEUTIL
    printf("END add_interface\n");
#endif
  } /* add_interface */



#ifdef OUT_OF_ORDER
void get_interfaces (typetype basic_type)

  {
    typelisttype typelist_elem;
    listtype *list_insert_place;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* get_interfaces */
    result = NULL;
    list_insert_place = &result;
    typelist_elem = basic_type->interfaces;
    while (typelist_elem->next != NULL) {
      list_insert_place = append_element_to_list(list_insert_place,
          typelist_elem->type_elem, &err_info);
      typelist_elem = typelist_elem->next;
    } /* while */
    if (err_info != OKAY_NO_ERROR) {
      free_list(result);
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    return bld_reflist_temp(result);
  } /* get_interfaces */
#endif
