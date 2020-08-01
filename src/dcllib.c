/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/dcllib.c                                        */
/*  Changes: 1999  Thomas Mertes                                    */
/*  Content: Primitive actions to for simple declarations.          */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "sigutl.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "traceutl.h"
#include "listutl.h"
#include "entutl.h"
#include "blockutl.h"
#include "executl.h"
#include "objutl.h"
#include "exec.h"
#include "runerr.h"
#include "name.h"
#include "match.h"
#include "error.h"

#undef EXTERN
#define EXTERN
#include "dcllib.h"

#undef TRACE_DCL



#ifdef ANSI_C

objecttype dcl_attr (listtype arguments)
#else

objecttype dcl_attr (arguments)
listtype arguments;
#endif

  {
    typetype attribute_type;

  /* dcl_attr */
    isit_type(arg_2(arguments));
    attribute_type = take_type(arg_2(arguments));
/*    printf("decl attr ");
    trace1(attribute_type->match_obj);
    printf(":\n"); */
    return bld_param_temp(attribute_type->match_obj);
  } /* dcl_attr */



#ifdef ANSI_C

objecttype dcl_const (listtype arguments)
#else

objecttype dcl_const (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype name_expr;
    objecttype value_expr;
    objecttype value;
    objecttype matched_value;
    objecttype current_object;
    errinfotype err_info = OKAY_NO_ERROR;

  /* dcl_const */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
    value_expr = arg_6(arguments);
#ifdef TRACE_DCL
    printf("\ndecl const object_type = ");
    trace1(object_type->match_obj);
    printf("\ndecl const name_expr = ");
    trace1(name_expr);
    printf("\ndecl const value_expr = ");
    trace1(value_expr);
    printf("\n");
#endif
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      if (CATEGORY_OF_OBJ(value_expr) == EXPROBJECT &&
          value_expr->value.listvalue != NULL &&
          value_expr->value.listvalue->next == NULL) {
        value_expr = value_expr->value.listvalue->obj;
      } /* if */
#ifdef TRACE_DCL
      printf("decl const value_expr = ");
      trace1(value_expr);
      printf("\n");
#endif
      current_object = entername(prog.declaration_root, name_expr, &err_info);
      /* printf(":%lu\n", (long unsigned) GET_ENTITY(current_object)); */
      value = copy_expression(value_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        current_object->type_of = object_type;
#ifdef TRACE_DCL
        printf("decl const current_object = ");
        trace1(current_object);
        printf("\n");
#endif
        if (CATEGORY_OF_OBJ(value) == EXPROBJECT) {
          substitute_params(value);
          if (match_expression(value) != NULL &&
              (matched_value = match_object(value)) != NULL) {
            do_create(current_object, matched_value, &err_info);
            if (err_info == CREATE_ERROR) {
              err_object(DECL_FAILED, current_object);
              err_info = OKAY_NO_ERROR;
#ifdef TRACE_DCL
              printf("*** do_create failed ");
              prot_list(arguments);
              printf("\n");
#endif
            } /* if */
#ifdef TRACE_DCL
          } else {
            printf("match value failed: ");
            trace1(value);
            printf("\n");
            printf("value_expr: ");
            trace1(value_expr);
            printf("\n");
            printf("object: ");
            trace1(current_object);
            printf("\n");
            printf("name_expr: ");
            trace1(name_expr);
            printf("\n");
#endif
          } /* if */
        } else {
          do_create(current_object, value, &err_info);
          if (err_info == CREATE_ERROR) {
            err_object(DECL_FAILED, current_object);
            err_info = OKAY_NO_ERROR;
#ifdef TRACE_DCL
            printf("*** do_create failed ");
            prot_list(arguments);
            printf("\n");
#endif
          } /* if */
        } /* if */
        free_expression(value);
        if (CATEGORY_OF_OBJ(current_object) == BLOCKOBJECT) {
          current_object->value.blockvalue->params =
              get_param_list(current_object->descriptor.property->params, &err_info);
        } /* if */
      } /* if */
      shrink_stack();
    } /* if */
#ifdef TRACE_DCL
    printf("entity=%lu ", (unsigned long) GET_ENTITY(current_object));
    printf("%lu ", (unsigned long) current_object);
    printf("decl const current_object = ");
    trace1(current_object);
    printf("\n");
#endif
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* dcl_const */



#ifdef ANSI_C

objecttype dcl_elements (listtype arguments)
#else

objecttype dcl_elements (arguments)
listtype arguments;
#endif

  {
    objecttype local_decls;
    listtype *local_object_insert_place;
    objecttype decl_res;
    listtype element_list;
    errinfotype err_info = OKAY_NO_ERROR;

  /* dcl_elements */
    local_decls = arg_1(arguments);
    push_stack();
    local_object_insert_place = get_local_object_insert_place();
    decl_res = evaluate(local_decls);
    if (decl_res != SYS_EMPTY_OBJECT) {
      printf("eval local decls --> ");
      trace1(decl_res);
      printf("\n");
      trace1(SYS_EMPTY_OBJECT);
      printf("\n");
      err_object(PROC_EXPECTED, decl_res);
    } /* if */
    element_list = copy_list(*local_object_insert_place, &err_info);
    /* printf("before pop_stack\n"); */
    pop_stack();
    /* printf("after pop_stack\n"); */
    if (err_info != OKAY_NO_ERROR) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_reflist_temp(element_list);
    } /* if */
  } /* dcl_elements */



#ifdef ANSI_C

objecttype dcl_fwd (listtype arguments)
#else

objecttype dcl_fwd (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype name_expr;
    objecttype current_object;
    errinfotype err_info = OKAY_NO_ERROR;

  /* dcl_fwd */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
#ifdef TRACE_DCL
    printf("\ndecl const object_type = ");
    trace1(object_type->match_obj);
    printf("\ndecl const name_expr = ");
    trace1(name_expr);
    printf("\n");
#endif
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      current_object = entername(prog.declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        current_object->type_of = object_type;
        INIT_CATEGORY_OF_OBJ(current_object, FORWARDOBJECT);
      } /* if */
      shrink_stack();
    } /* if */
#ifdef TRACE_DCL
    printf("entity=%lu ", (unsigned long) GET_ENTITY(current_object));
    printf("%lu ", (unsigned long) current_object);
    printf("forward decl const current_object = ");
    trace1(current_object);
    printf("\n");
#endif
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* dcl_fwd */



#ifdef ANSI_C

objecttype dcl_fwdvar (listtype arguments)
#else

objecttype dcl_fwdvar (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype name_expr;
    objecttype current_object;
    errinfotype err_info = OKAY_NO_ERROR;

  /* dcl_fwdvar */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
#ifdef TRACE_DCL
    printf("\ndecl var object_type = ");
    trace1(object_type->match_obj);
    printf("\ndecl var name_expr = ");
    trace1(name_expr);
    printf("\n");
#endif
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      current_object = entername(prog.declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        current_object->type_of = object_type;
        INIT_CATEGORY_OF_VAR(current_object, FORWARDOBJECT);
      } /* if */
      shrink_stack();
    } /* if */
#ifdef TRACE_DCL
    printf("entity=%lu ", (unsigned long) GET_ENTITY(current_object));
    printf("%lu ", (unsigned long) current_object);
    printf("forward decl var current_object = ");
    trace1(current_object);
    printf("\n");
#endif
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* dcl_fwdvar */



#ifdef ANSI_C

objecttype dcl_getfunc (listtype arguments)
#else

objecttype dcl_getfunc (arguments)
listtype arguments;
#endif

  {
    objecttype name_expr;
    objecttype object_found;
    errinfotype err_info = OKAY_NO_ERROR;

  /* dcl_getfunc */
    name_expr = arg_2(arguments);
#ifdef TRACE_DCL
    printf("decl const name_expr = ");
    trace1(name_expr);
    printf("\n");
#endif
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      object_found = search_name(prog.declaration_root, name_expr, &err_info);
      shrink_stack();
    } /* if */
#ifdef TRACE_DCL
    printf("entity=%lu ", (unsigned long) GET_ENTITY(object_found));
    printf("%lu ", (unsigned long) object_found);
    printf("getfunc object_found = ");
    trace1(object_found);
    printf("\n");
#endif
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_reference_temp(object_found);
    } /* if */
  } /* dcl_getfunc */



#ifdef ANSI_C

objecttype dcl_getobj (listtype arguments)
#else

objecttype dcl_getobj (arguments)
listtype arguments;
#endif

  {
    objecttype name_expr;
    objecttype object_found;
    errinfotype err_info = OKAY_NO_ERROR;

  /* dcl_getobj */
    name_expr = arg_2(arguments);
#ifdef TRACE_DCL
    printf("decl const name_expr = ");
    trace1(name_expr);
    printf("\n");
#endif
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      object_found = find_name(prog.declaration_root, name_expr, &err_info);
      shrink_stack();
    } /* if */
#ifdef TRACE_DCL
    printf("entity=%lu ", (unsigned long) GET_ENTITY(object_found));
    printf("%lu ", (unsigned long) object_found);
    printf("getobj object_found = ");
    trace1(object_found);
    printf("\n");
#endif
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_reference_temp(object_found);
    } /* if */
  } /* dcl_getobj */



#ifdef ANSI_C

objecttype dcl_global (listtype arguments)
#else

objecttype dcl_global (arguments)
listtype arguments;
#endif

  {
    objecttype statement;
    stacktype stack_data_backup;
    stacktype stack_current_backup;
    stacktype stack_upward_backup;

  /* dcl_global */
    statement = arg_2(arguments);
    stack_data_backup = prog.stack_data;
    stack_current_backup = prog.stack_current;
    stack_upward_backup = prog.stack_global->upward;
    prog.stack_data = prog.stack_global;
    prog.stack_current = prog.stack_global;

    evaluate(statement);

    prog.stack_data = stack_data_backup;
    prog.stack_current = stack_current_backup;
    if (prog.stack_global->upward != NULL) {
      printf(" *** dcl_global: prog.stack_global->upward != NULL\n");
    } else {
      prog.stack_global->upward = stack_upward_backup;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* dcl_global */



#ifdef ANSI_C

objecttype dcl_in1var (listtype arguments)
#else

objecttype dcl_in1var (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype created_object;

  /* dcl_in1var */
    isit_type(arg_3(arguments));
    object_type = take_type(arg_3(arguments));
/*    printf("decl in1var ");
    trace1(object_type->match_obj);
    printf(":\n"); */
    if (ALLOC_OBJECT(created_object)) {
      created_object->type_of = object_type;
      created_object->descriptor.property = NULL;
      INIT_CATEGORY_OF_VAR(created_object, VALUEPARAMOBJECT);
      created_object->value.objvalue = NULL;
      return bld_param_temp(created_object);
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* dcl_in1var */



#ifdef ANSI_C

objecttype dcl_in2var (listtype arguments)
#else

objecttype dcl_in2var (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype name_expr;
    errinfotype err_info = OKAY_NO_ERROR;
    objecttype created_object;

  /* dcl_in2var */
    isit_type(arg_3(arguments));
    object_type = take_type(arg_3(arguments));
    name_expr = arg_5(arguments);
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      /* printf("decl in2var ");
         trace1(object_type->match_obj);
         printf(": ");
         trace1(name_expr);
         printf(";\n"); */
      created_object = entername(prog.declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        created_object->type_of = object_type;
        INIT_CATEGORY_OF_VAR(created_object, VALUEPARAMOBJECT);
        /* printf("decl in2var --> %lx ", (unsigned long int) created_object);
           trace1(created_object);
           printf(";\n"); */
      } /* if */
      shrink_stack();
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_in2var */



#ifdef ANSI_C

objecttype dcl_inout1 (listtype arguments)
#else

objecttype dcl_inout1 (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype created_object;

  /* dcl_inout1 */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    /* printf("decl inout1 ");
       trace1(object_type->match_obj);
       printf(":\n"); */
    if (ALLOC_OBJECT(created_object)) {
      created_object->type_of = object_type;
      created_object->descriptor.property = NULL;
      INIT_CATEGORY_OF_VAR(created_object, REFPARAMOBJECT);
      created_object->value.objvalue = NULL;
      /* printf("dcl_inout1 --> %lx ", (unsigned long int) created_object);
         trace1(created_object);
         printf("\n"); */
      return bld_param_temp(created_object);
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* dcl_inout1 */



#ifdef ANSI_C

objecttype dcl_inout2 (listtype arguments)
#else

objecttype dcl_inout2 (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype name_expr;
    errinfotype err_info = OKAY_NO_ERROR;
    objecttype created_object;

  /* dcl_inout2 */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      /* printf("decl inout2 ");
         trace1(object_type->match_obj);
         printf(": ");
         trace1(name_expr);
         printf(";\n"); */
      created_object = entername(prog.declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        created_object->type_of = object_type;
        INIT_CATEGORY_OF_VAR(created_object, REFPARAMOBJECT);
        /* printf("decl inout2 --> %lx ", (unsigned long int) created_object);
           trace1(created_object);
           printf(";\n"); */
      } /* if */
      shrink_stack();
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_inout2 */



#ifdef ANSI_C

objecttype dcl_param_attr (listtype arguments)
#else

objecttype dcl_param_attr (arguments)
listtype arguments;
#endif

  {
    objecttype f_param_object;
    objecttype param_object;
    objecttype *f_param_prototype;
    objecttype result;

  /* dcl_param_attr */
    isit_param(arg_2(arguments));
    f_param_object = arg_2(arguments);
    /* printf("decl param attr ");
       trace1(f_param_object);
       printf(":\n"); */
    param_object = take_param(f_param_object);
    /* printf("decl param attr ");
       trace1(param_object);
       printf(":\n"); */
    if (CATEGORY_OF_OBJ(param_object) == REFPARAMOBJECT && VAR_OBJECT(param_object)) {
      f_param_prototype = &param_object->type_of->inout_f_param_prototype;
    } else {
      f_param_prototype = &param_object->type_of->other_f_param_prototype;
    } /* if */
    if (*f_param_prototype == NULL) {
      if (ALLOC_OBJECT(result)) {
        result->type_of = NULL;
        result->descriptor.property = NULL;
        INIT_CATEGORY_OF_OBJ(result, FORMPARAMOBJECT);
        result->value.objvalue = param_object;
      } else {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      *f_param_prototype = result;
      f_param_object->value.objvalue = NULL;
    } else {
      result = *f_param_prototype;
    } /* if */
    /* trace1(param_object);
       printf("\n");
       printf("dcl_param_attr => %lX\n", result);
       trace1(result);
       printf("\n"); */
    return bld_param_temp(result);
  } /* dcl_param_attr */



#ifdef ANSI_C

objecttype dcl_ref1 (listtype arguments)
#else

objecttype dcl_ref1 (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype created_object;

  /* dcl_ref1 */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    /* printf("decl ref1 ");
       trace1(object_type->match_obj);
       printf(":\n"); */
    if (ALLOC_OBJECT(created_object)) {
      created_object->type_of = object_type;
      created_object->descriptor.property = NULL;
      INIT_CATEGORY_OF_OBJ(created_object, REFPARAMOBJECT);
      created_object->value.objvalue = NULL;
      /* printf("decl ref1 --> %lx ", (unsigned long int) created_object);
         trace1(created_object);
         printf(";\n"); */
      return bld_param_temp(created_object);
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* dcl_ref1 */



#ifdef ANSI_C

objecttype dcl_ref2 (listtype arguments)
#else

objecttype dcl_ref2 (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype name_expr;
    errinfotype err_info = OKAY_NO_ERROR;
    objecttype created_object;

  /* dcl_ref2 */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
    /* printf("decl ref2 ");
       trace1(object_type->match_obj);
       printf(": ");
       trace1(name_expr);
       printf(";\n"); */
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      created_object = entername(prog.declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        created_object->type_of = object_type;
        INIT_CATEGORY_OF_OBJ(created_object, REFPARAMOBJECT);
        /* printf("decl ref2 --> %lx ", (unsigned long int) created_object);
           trace1(created_object);
           printf(";\n"); */
      } /* if */
      shrink_stack();
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_ref2 */



#ifdef ANSI_C

objecttype dcl_symb (listtype arguments)
#else

objecttype dcl_symb (arguments)
listtype arguments;
#endif

  {
    objecttype symb_object;

  /* dcl_symb */
    symb_object = arg_2(arguments);
    /* printf("decl symb %lu ", (long unsigned) GET_ENTITY(symb_object));
    trace1(symb_object);
    printf(":\n"); */
    if (HAS_ENTITY(symb_object) &&
        GET_ENTITY(symb_object)->syobject != NULL) {
      symb_object = GET_ENTITY(symb_object)->syobject;
    } /* if */
    /* printf("decl symb %lu ", (long unsigned) GET_ENTITY(symb_object));
    trace1(symb_object);
    printf(":\n"); */
    return bld_param_temp(symb_object);
  } /* dcl_symb */



#ifdef ANSI_C

objecttype dcl_val1 (listtype arguments)
#else

objecttype dcl_val1 (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype created_object;

  /* dcl_val1 */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    /* printf("decl val1 ");
       trace1(object_type->match_obj);
       printf(":\n"); */
    if (ALLOC_OBJECT(created_object)) {
      created_object->type_of = object_type;
      created_object->descriptor.property = NULL;
      INIT_CATEGORY_OF_OBJ(created_object, VALUEPARAMOBJECT);
      created_object->value.objvalue = NULL;
      /* printf("decl val1 --> %lx ", (unsigned long int) created_object);
         trace1(created_object);
         printf("\n"); */
      return bld_param_temp(created_object);
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* dcl_val1 */



#ifdef ANSI_C

objecttype dcl_val2 (listtype arguments)
#else

objecttype dcl_val2 (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype name_expr;
    errinfotype err_info = OKAY_NO_ERROR;
    objecttype created_object;

  /* dcl_val2 */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      /* printf("decl val2 ");
         trace1(object_type->match_obj);
         printf(": ");
         trace1(name_expr);
         printf(";\n"); */
      created_object = entername(prog.declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        created_object->type_of = object_type;
        INIT_CATEGORY_OF_OBJ(created_object, VALUEPARAMOBJECT);
        /* printf("decl val2 --> %lx ", (unsigned long int) created_object);
           trace1(created_object);
           printf(";\n"); */
      } /* if */
      shrink_stack();
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_val2 */



#ifdef ANSI_C

objecttype dcl_var (listtype arguments)
#else

objecttype dcl_var (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype name_expr;
    objecttype value_expr;
    objecttype value;
    objecttype matched_value;
    objecttype current_object;
    errinfotype err_info = OKAY_NO_ERROR;

  /* dcl_var */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
    value_expr = arg_6(arguments);
#ifdef TRACE_DCL
    printf("\ndecl var object_type = ");
    trace1(object_type->match_obj);
    printf("\ndecl var name_expr = ");
    trace1(name_expr);
    printf("\ndecl var value_expr = ");
    trace1(value_expr);
    printf("\n");
#endif
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      if (CATEGORY_OF_OBJ(value_expr) == EXPROBJECT &&
          value_expr->value.listvalue != NULL &&
          value_expr->value.listvalue->next == NULL) {
        value_expr = value_expr->value.listvalue->obj;
      } /* if */
#ifdef TRACE_DCL
      printf("decl var value_expr = ");
      trace1(value_expr);
      printf("\n");
#endif
      current_object = entername(prog.declaration_root, name_expr, &err_info);
      value = copy_expression(value_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        current_object->type_of = object_type;
        SET_VAR_FLAG(current_object);
#ifdef TRACE_DCL
        printf("decl var current_object = ");
        trace1(current_object);
        printf("\n");
#endif
        if (CATEGORY_OF_OBJ(value) == EXPROBJECT) {
          substitute_params(value);
          if (match_expression(value) != NULL &&
              (matched_value = match_object(value)) != NULL) {
            do_create(current_object, matched_value, &err_info);
            if (err_info == CREATE_ERROR) {
              err_object(DECL_FAILED, current_object);
              err_info = OKAY_NO_ERROR;
#ifdef TRACE_DCL
              printf("*** do_create failed ");
              prot_list(arguments);
              printf("\n");
#endif
            } /* if */
          } else {
            printf("*** match value failed ");
            trace1(value);
            printf("\n");
          } /* if */
        } else {
          do_create(current_object, value, &err_info);
          if (err_info == CREATE_ERROR) {
            err_object(DECL_FAILED, current_object);
            err_info = OKAY_NO_ERROR;
#ifdef TRACE_DCL
            printf("*** do_create failed ");
            prot_list(arguments);
            printf("\n");
#endif
          } /* if */
        } /* if */
        free_expression(value);
      } /* if */
      shrink_stack();
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* dcl_var */
