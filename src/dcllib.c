/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2005, 2008, 2013, 2015  Thomas Mertes      */
/*                2019, 2021  Thomas Mertes                         */
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
/*  Changes: 1999, 2008, 2013, 2015, 2019, 2021  Thomas Mertes      */
/*  Content: Primitive actions to for simple declarations.          */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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
#include "findid.h"
#include "syntax.h"
#include "exec.h"
#include "runerr.h"
#include "name.h"
#include "match.h"
#include "error.h"

#undef EXTERN
#define EXTERN
#include "dcllib.h"

#define TRACE_DCL 0
#define TRACE_DCL_CONST 0
#define TRACE_DCL_VAR 0



objectType dcl_attr (listType arguments)

  {
    typeType attribute_type;

  /* dcl_attr */
    isit_type(arg_2(arguments));
    attribute_type = take_type(arg_2(arguments));
/*    printf("decl attr ");
    trace1(attribute_type->match_obj);
    printf(":\n"); */
    return bld_param_temp(attribute_type->match_obj);
  } /* dcl_attr */



objectType dcl_const (listType arguments)

  {
    typeType object_type;
    objectType name_expr;
    objectType value_expr;
    objectType value;
    objectType matched_value;
    objectType current_object;
    objectType decl_expr_object;
    errInfoType err_info = OKAY_NO_ERROR;

  /* dcl_const */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
    value_expr = arg_6(arguments);
    logFunction(printf("dcl_const\n"););
#if TRACE_DCL_CONST
    printf("decl const object_type = ");
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
          value_expr->value.listValue != NULL &&
          value_expr->value.listValue->next == NULL) {
        value_expr = value_expr->value.listValue->obj;
        if (HAS_ENTITY(value_expr) &&
            GET_ENTITY(value_expr)->data.owner != NULL) {
          value_expr = GET_ENTITY(value_expr)->data.owner->obj;
        } /* if */
      } /* if */
#if TRACE_DCL_CONST
      printf("decl const value_expr = ");
      trace1(value_expr);
      printf("\n");
#endif
      current_object = entername(prog->declaration_root, name_expr, &err_info);
      /* printf(":%lu\n", (long unsigned) GET_ENTITY(current_object)); */
      value = copy_expression(value_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        current_object->type_of = object_type;
#if TRACE_DCL_CONST
        printf("decl const current_object = ");
        trace1(current_object);
        printf("\n");
#endif
        decl_expr_object = curr_exec_object;
        if (CATEGORY_OF_OBJ(value) == EXPROBJECT) {
          substitute_params(value);
          if (match_expression(value) != NULL &&
              (matched_value = match_object(value)) != NULL) {
            do_create(current_object, matched_value, &err_info);
            if (unlikely(err_info != OKAY_NO_ERROR)) {
              if (err_info != CREATE_ERROR) {
                err_expr_obj(EXCEPTION_RAISED, curr_exec_object, prog->sys_var[err_info]);
              } /* if */
              err_expr_obj(DECL_FAILED, decl_expr_object, current_object);
              err_info = OKAY_NO_ERROR;
#if TRACE_DCL_CONST
              printf("*** do_create failed ");
              prot_list(arguments);
              printf("\n");
#endif
            } /* if */
#if TRACE_DCL_CONST
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
          if (unlikely(err_info != OKAY_NO_ERROR)) {
            if (err_info != CREATE_ERROR) {
              err_expr_obj(EXCEPTION_RAISED, curr_exec_object, prog->sys_var[err_info]);
            } /* if */
            err_expr_obj(DECL_FAILED, decl_expr_object, current_object);
            err_info = OKAY_NO_ERROR;
#if TRACE_DCL_CONST
            printf("*** do_create failed ");
            prot_list(arguments);
            printf("\n");
#endif
          } /* if */
        } /* if */
        free_expression(value);
        if (CATEGORY_OF_OBJ(current_object) == BLOCKOBJECT) {
          current_object->value.blockValue->params =
              get_param_list(current_object->descriptor.property->params, &err_info);
        } /* if */
      } /* if */
      shrink_stack();
    } /* if */
#if TRACE_DCL_CONST
    printf("entity=%lu ", (unsigned long) GET_ENTITY(current_object));
    printf("%lu ", (unsigned long) current_object);
    printf("decl const current_object = ");
    trace1(current_object);
    printf("\n");
#endif
    logFunction(printf("dcl_const --> err_info=%d\n", err_info););
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* dcl_const */



objectType dcl_elements (listType arguments)

  {
    objectType local_decls;
    boolType can_push_stack;
    listType *local_object_insert_place;
    objectType decl_res;
    errInfoType err_info = OKAY_NO_ERROR;
    listType element_list;

  /* dcl_elements */
    local_decls = arg_1(arguments);
    /* printf("stack upward: " FMT_U_MEM "\n",
           (memSizeType) prog->stack_current->upward); */
    can_push_stack = prog->stack_current->upward != NULL;
    if (can_push_stack) {
      push_stack();
    } /* if */
    local_object_insert_place = get_local_object_insert_place();
    decl_res = evaluate(local_decls);
    if (decl_res != SYS_EMPTY_OBJECT) {
      err_object(PROC_EXPECTED, decl_res);
    } /* if */
    if (can_push_stack) {
      element_list = copy_list(*local_object_insert_place, &err_info);
      /* printf("before pop_stack\n"); */
      pop_stack();
      /* printf("after pop_stack\n"); */
    } else {
      element_list = *local_object_insert_place;
      *local_object_insert_place = NULL;
      pop_object_list(element_list);
      prog->stack_current->object_list_insert_place = local_object_insert_place;
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_reflist_temp(element_list);
    } /* if */
  } /* dcl_elements */



objectType dcl_fwd (listType arguments)

  {
    typeType object_type;
    objectType name_expr;
    objectType current_object;
    errInfoType err_info = OKAY_NO_ERROR;

  /* dcl_fwd */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
#if TRACE_DCL
    printf("\ndecl const object_type = ");
    trace1(object_type->match_obj);
    printf("\ndecl const name_expr = ");
    trace1(name_expr);
    printf("\n");
#endif
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      current_object = entername(prog->declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        current_object->type_of = object_type;
        INIT_CATEGORY_OF_OBJ(current_object, FORWARDOBJECT);
      } /* if */
      shrink_stack();
    } /* if */
#if TRACE_DCL
    printf("entity=%lu ", (unsigned long) GET_ENTITY(current_object));
    printf("%lu ", (unsigned long) current_object);
    printf("forward decl const current_object = ");
    trace1(current_object);
    printf("\n");
#endif
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* dcl_fwd */



objectType dcl_fwdvar (listType arguments)

  {
    typeType object_type;
    objectType name_expr;
    objectType current_object;
    errInfoType err_info = OKAY_NO_ERROR;

  /* dcl_fwdvar */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
#if TRACE_DCL
    printf("\ndecl var object_type = ");
    trace1(object_type->match_obj);
    printf("\ndecl var name_expr = ");
    trace1(name_expr);
    printf("\n");
#endif
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      current_object = entername(prog->declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        current_object->type_of = object_type;
        INIT_CATEGORY_OF_VAR(current_object, FORWARDOBJECT);
      } /* if */
      shrink_stack();
    } /* if */
#if TRACE_DCL
    printf("entity=%lu ", (unsigned long) GET_ENTITY(current_object));
    printf("%lu ", (unsigned long) current_object);
    printf("forward decl var current_object = ");
    trace1(current_object);
    printf("\n");
#endif
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* dcl_fwdvar */



objectType dcl_getfunc (listType arguments)

  {
    objectType name_expr;
    objectType object_found;
    errInfoType err_info = OKAY_NO_ERROR;

  /* dcl_getfunc */
    name_expr = arg_2(arguments);
#if TRACE_DCL
    printf("decl const name_expr = ");
    trace1(name_expr);
    printf("\n");
#endif
    object_found = search_name(prog->declaration_root, name_expr, &err_info);
#if TRACE_DCL
    printf("entity=%lu ", (unsigned long) GET_ENTITY(object_found));
    printf("%lu ", (unsigned long) object_found);
    printf("getfunc object_found = ");
    trace1(object_found);
    printf("\n");
#endif
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_reference_temp(object_found);
    } /* if */
  } /* dcl_getfunc */



objectType dcl_getobj (listType arguments)

  {
    objectType name_expr;
    objectType object_found;
    errInfoType err_info = OKAY_NO_ERROR;

  /* dcl_getobj */
    name_expr = arg_2(arguments);
#if TRACE_DCL
    printf("decl const name_expr = ");
    trace1(name_expr);
    printf("\n");
#endif
    object_found = find_name(prog->declaration_root, name_expr, &err_info);
#if TRACE_DCL
    printf("entity=%lu ", (unsigned long) GET_ENTITY(object_found));
    printf("%lu ", (unsigned long) object_found);
    printf("getobj object_found = ");
    trace1(object_found);
    printf("\n");
#endif
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_reference_temp(object_found);
    } /* if */
  } /* dcl_getobj */



objectType dcl_global (listType arguments)

  {
    objectType statement;
    stackType stack_data_backup;
    stackType stack_current_backup;
    stackType stack_upward_backup;

  /* dcl_global */
    statement = arg_2(arguments);
    stack_data_backup = prog->stack_data;
    stack_current_backup = prog->stack_current;
    stack_upward_backup = prog->stack_global->upward;
    prog->stack_data = prog->stack_global;
    prog->stack_current = prog->stack_global;

    evaluate(statement);

    prog->stack_data = stack_data_backup;
    prog->stack_current = stack_current_backup;
    if (prog->stack_global->upward != NULL) {
      printf(" *** dcl_global: prog->stack_global->upward != NULL\n");
    } else {
      prog->stack_global->upward = stack_upward_backup;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* dcl_global */



objectType dcl_in1 (listType arguments)

  {
    typeType object_type;
    objectType created_object;

  /* dcl_in1 */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    /* printf("decl in1 ");
       trace1(object_type->match_obj);
       printf(":\n"); */
    if (unlikely(!ALLOC_OBJECT(created_object))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      created_object->type_of = object_type;
      created_object->descriptor.property = NULL;
      created_object->value.objValue = NULL;
      switch (object_type->in_param_type) {
        case PARAM_UNDEFINED:
          err_expr_type(KIND_OF_IN_PARAM_UNDEFINED, curr_exec_object, object_type);
          break;
        case PARAM_VALUE:
          INIT_CATEGORY_OF_OBJ(created_object, VALUEPARAMOBJECT);
          break;
        case PARAM_REF:
          INIT_CATEGORY_OF_OBJ(created_object, REFPARAMOBJECT);
          break;
      } /* switch */
      /* printf("decl in1 --> %lx ", (unsigned long int) created_object);
         trace1(created_object);
         printf(";\n"); */
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_in1 */



objectType dcl_in2 (listType arguments)

  {
    typeType object_type;
    objectType name_expr;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType created_object;

  /* dcl_in2 */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      /* printf("decl in2 ");
         trace1(object_type->match_obj);
         printf(": ");
         trace1(name_expr);
         printf(";\n"); */
      created_object = entername(prog->declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        created_object->type_of = object_type;
        switch (object_type->in_param_type) {
          case PARAM_UNDEFINED:
            err_expr_type(KIND_OF_IN_PARAM_UNDEFINED, curr_exec_object, object_type);
            break;
          case PARAM_VALUE:
            INIT_CATEGORY_OF_OBJ(created_object, VALUEPARAMOBJECT);
            break;
          case PARAM_REF:
            INIT_CATEGORY_OF_OBJ(created_object, REFPARAMOBJECT);
            break;
        } /* switch */
        /* printf("decl in2 --> %lx ", (unsigned long int) created_object);
           trace1(created_object);
           printf(";\n"); */
      } /* if */
      shrink_stack();
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_in2 */



objectType dcl_in1var (listType arguments)

  {
    typeType object_type;
    objectType created_object;

  /* dcl_in1var */
    isit_type(arg_3(arguments));
    object_type = take_type(arg_3(arguments));
/*    printf("decl in1var ");
    trace1(object_type->match_obj);
    printf(":\n"); */
    if (unlikely(!ALLOC_OBJECT(created_object))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      created_object->type_of = object_type;
      created_object->descriptor.property = NULL;
      INIT_CATEGORY_OF_VAR(created_object, VALUEPARAMOBJECT);
      created_object->value.objValue = NULL;
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_in1var */



objectType dcl_in2var (listType arguments)

  {
    typeType object_type;
    objectType name_expr;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType created_object;

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
      created_object = entername(prog->declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        created_object->type_of = object_type;
        INIT_CATEGORY_OF_VAR(created_object, VALUEPARAMOBJECT);
        /* printf("decl in2var --> %lx ", (unsigned long int) created_object);
           trace1(created_object);
           printf(";\n"); */
      } /* if */
      shrink_stack();
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_in2var */



objectType dcl_inout1 (listType arguments)

  {
    typeType object_type;
    objectType created_object;

  /* dcl_inout1 */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    /* printf("decl inout1 ");
       trace1(object_type->match_obj);
       printf(":\n"); */
    if (unlikely(!ALLOC_OBJECT(created_object))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      created_object->type_of = object_type;
      created_object->descriptor.property = NULL;
      INIT_CATEGORY_OF_VAR(created_object, REFPARAMOBJECT);
      created_object->value.objValue = NULL;
      /* printf("dcl_inout1 --> %lx ", (unsigned long int) created_object);
         trace1(created_object);
         printf("\n"); */
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_inout1 */



objectType dcl_inout2 (listType arguments)

  {
    typeType object_type;
    objectType name_expr;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType created_object;

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
      created_object = entername(prog->declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        created_object->type_of = object_type;
        INIT_CATEGORY_OF_VAR(created_object, REFPARAMOBJECT);
        /* printf("decl inout2 --> %lx ", (unsigned long int) created_object);
           trace1(created_object);
           printf(";\n"); */
      } /* if */
      shrink_stack();
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_inout2 */



objectType dcl_param_attr (listType arguments)

  {
    objectType f_param_object;
    objectType param_object;
    objectType *f_param_prototype;
    objectType result;

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
      if (unlikely(!ALLOC_OBJECT(result))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->type_of = NULL;
        result->descriptor.property = NULL;
        INIT_CATEGORY_OF_OBJ(result, FORMPARAMOBJECT);
        result->value.objValue = param_object;
      } /* if */
      *f_param_prototype = result;
      f_param_object->value.objValue = NULL;
    } else {
      result = *f_param_prototype;
    } /* if */
    /* trace1(param_object);
       printf("\n");
       printf("dcl_param_attr --> %lX\n", result);
       trace1(result);
       printf("\n"); */
    return bld_param_temp(result);
  } /* dcl_param_attr */



objectType dcl_ref1 (listType arguments)

  {
    typeType object_type;
    objectType created_object;

  /* dcl_ref1 */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    /* printf("decl ref1 ");
       trace1(object_type->match_obj);
       printf(":\n"); */
    if (unlikely(!ALLOC_OBJECT(created_object))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      created_object->type_of = object_type;
      created_object->descriptor.property = NULL;
      INIT_CATEGORY_OF_OBJ(created_object, REFPARAMOBJECT);
      created_object->value.objValue = NULL;
      /* printf("decl ref1 --> %lx ", (unsigned long int) created_object);
         trace1(created_object);
         printf(";\n"); */
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_ref1 */



objectType dcl_ref2 (listType arguments)

  {
    typeType object_type;
    objectType name_expr;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType created_object;

  /* dcl_ref2 */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      /* printf("decl ref2 ");
         trace1(object_type->match_obj);
         printf(": ");
         trace1(name_expr);
         printf(";\n"); */
      created_object = entername(prog->declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        created_object->type_of = object_type;
        INIT_CATEGORY_OF_OBJ(created_object, REFPARAMOBJECT);
        /* printf("decl ref2 --> %lx ", (unsigned long int) created_object);
           trace1(created_object);
           printf(";\n"); */
      } /* if */
      shrink_stack();
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_ref2 */



objectType dcl_symb (listType arguments)

  {
    objectType symb_object;

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



objectType dcl_syntax (listType arguments)

  {
    typeType object_type;
    objectType name_expr;
    objectType value_expr;
    listType assocPriority;
    objectType assocObject;
    objectType priorityObject;
    objectType unexpectedObject = NULL;
    boolType okay = TRUE;
    identType assocIdent;
    priorityType priority;
    assocType assoc;
    tokenType token_list_end;

  /* dcl_syntax */
    logFunction(printf("dcl_syntax\n"););
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
    value_expr = arg_6(arguments);
    /* printf("decl syntax ");
       trace1(object_type->match_obj);
       printf(": ");
       trace1(name_expr);
       printf(" is ");
       trace1(value_expr);
       printf("\n"); */
    if (CATEGORY_OF_OBJ(value_expr) == EXPROBJECT) {
      assocPriority = take_reflist(value_expr);
      if (assocPriority == NULL) {
        assocObject = NULL;
        priorityObject = NULL;
      } else {
        assocObject = assocPriority->obj;
        if (assocPriority->next == NULL) {
          priorityObject = NULL;
        } else {
          priorityObject = assocPriority->next->obj;
          if (assocPriority->next->next != NULL) {
            unexpectedObject = assocPriority->next->next->obj;
          } /* if */
        } /* if */
      } /* if */
    } else {
      assocObject = value_expr;
      priorityObject = NULL;
    } /* if */
    if (assocObject != NULL && HAS_ENTITY(assocObject)) {
      assocIdent = assocObject->descriptor.property->entity->ident;
      if (assocIdent == prog->id_for.r_arrow) {            /*  ->   */
        assoc = YFX;
      } else if (assocIdent == prog->id_for.l_arrow) {     /*  <-   */
        assoc = XFY;
      } else if (assocIdent == prog->id_for.out_arrow) {   /*  <->  */
        assoc = XFX;
      } else if (assocIdent == prog->id_for.in_arrow) {    /*  -><- */
        assoc = YFY;
      } else {
        err_expr_obj(ILLEGAL_ASSOCIATIVITY, curr_exec_object, assocObject);
        okay = FALSE;
      } /* if */
    } else {
      err_expr_obj(ILLEGAL_ASSOCIATIVITY, curr_exec_object, assocObject);
      okay = FALSE;
    } /* if */
    if (priorityObject == NULL ||
        CATEGORY_OF_OBJ(priorityObject) != INTOBJECT) {
      err_expr_obj(CARD_EXPECTED, curr_exec_object, priorityObject);
      okay = FALSE;
    } else if (priorityObject->value.intValue < STRONGEST_PRIORITY ||
               priorityObject->value.intValue > WEAKEST_PRIORITY) {
      err_integer(ILLEGAL_PRIORITY, priorityObject->value.intValue);
      okay = FALSE;
    } else {
      priority = (priorityType) priorityObject->value.intValue;
    } /* if */
    if (unexpectedObject != NULL) {
      err_expr_obj_stri(EXPECTED_SYMBOL, curr_exec_object,
                        unexpectedObject, ";");
      okay = FALSE;
    } /* if */
    if (okay) {
      token_list_end = def_statement_syntax(name_expr,
          (priorityType) priority, assoc);
      if (token_list_end != NULL) {
        if (token_list_end->token_category != UNDEF_SYNTAX) {
          err_object(SYNTAX_DECLARED_TWICE, name_expr);
        } else {
          token_list_end->token_category = LIST_WITH_TYPEOF_SYNTAX;
          token_list_end->token_value.type_of = object_type;
        } /* if */
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* dcl_syntax */



objectType dcl_val1 (listType arguments)

  {
    typeType object_type;
    objectType created_object;

  /* dcl_val1 */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    /* printf("decl val1 ");
       trace1(object_type->match_obj);
       printf(":\n"); */
    if (unlikely(!ALLOC_OBJECT(created_object))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      created_object->type_of = object_type;
      created_object->descriptor.property = NULL;
      INIT_CATEGORY_OF_OBJ(created_object, VALUEPARAMOBJECT);
      created_object->value.objValue = NULL;
      /* printf("decl val1 --> %lx ", (unsigned long int) created_object);
         trace1(created_object);
         printf(";\n"); */
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_val1 */



objectType dcl_val2 (listType arguments)

  {
    typeType object_type;
    objectType name_expr;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType created_object;

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
      created_object = entername(prog->declaration_root, name_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        created_object->type_of = object_type;
        INIT_CATEGORY_OF_OBJ(created_object, VALUEPARAMOBJECT);
        /* printf("decl val2 --> %lx ", (unsigned long int) created_object);
           trace1(created_object);
           printf(";\n"); */
      } /* if */
      shrink_stack();
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_param_temp(created_object);
    } /* if */
  } /* dcl_val2 */



objectType dcl_var (listType arguments)

  {
    typeType object_type;
    objectType name_expr;
    objectType value_expr;
    objectType value;
    objectType matched_value;
    objectType current_object;
    objectType decl_expr_object;
    errInfoType err_info = OKAY_NO_ERROR;

  /* dcl_var */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
    value_expr = arg_6(arguments);
    logFunction(printf("dcl_var\n"););
#if TRACE_DCL_VAR
    printf("decl var object_type = ");
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
          value_expr->value.listValue != NULL &&
          value_expr->value.listValue->next == NULL) {
        value_expr = value_expr->value.listValue->obj;
        if (HAS_ENTITY(value_expr) &&
            GET_ENTITY(value_expr)->data.owner != NULL) {
          value_expr = GET_ENTITY(value_expr)->data.owner->obj;
        } /* if */
      } /* if */
#if TRACE_DCL_VAR
      printf("decl var value_expr = ");
      trace1(value_expr);
      printf("\n");
#endif
      current_object = entername(prog->declaration_root, name_expr, &err_info);
      value = copy_expression(value_expr, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        current_object->type_of = object_type;
        SET_VAR_FLAG(current_object);
#if TRACE_DCL_VAR
        printf("decl var current_object = ");
        trace1(current_object);
        printf("\n");
#endif
        decl_expr_object = curr_exec_object;
        if (CATEGORY_OF_OBJ(value) == EXPROBJECT) {
          substitute_params(value);
          if (match_expression(value) != NULL &&
              (matched_value = match_object(value)) != NULL) {
            do_create(current_object, matched_value, &err_info);
            if (unlikely(err_info != OKAY_NO_ERROR)) {
              if (err_info != CREATE_ERROR) {
                err_expr_obj(EXCEPTION_RAISED, curr_exec_object, prog->sys_var[err_info]);
              } /* if */
              err_expr_obj(DECL_FAILED, decl_expr_object, current_object);
              err_info = OKAY_NO_ERROR;
#if TRACE_DCL_VAR
              printf("*** do_create failed ");
              prot_list(arguments);
              printf("\n");
#endif
            } /* if */
#if TRACE_DCL_VAR
          } else {
            printf("*** match value failed ");
            trace1(value);
            printf("\n");
#endif
          } /* if */
        } else {
          do_create(current_object, value, &err_info);
          if (unlikely(err_info != OKAY_NO_ERROR)) {
            if (err_info != CREATE_ERROR) {
              err_expr_obj(EXCEPTION_RAISED, curr_exec_object, prog->sys_var[err_info]);
            } /* if */
            err_expr_obj(DECL_FAILED, decl_expr_object, current_object);
            err_info = OKAY_NO_ERROR;
#if TRACE_DCL_VAR
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
    logFunction(printf("dcl_var --> err_info=%d\n", err_info););
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* dcl_var */
