/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1991 - 1994, 2007, 2009, 2010  Thomas Mertes      */
/*  Copyright (C) 2012, 2013, 2015 - 2025  Thomas Mertes            */
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
/*  File: seed7/src/prclib.c                                        */
/*  Changes: 1991 - 1994, 2007, 2009, 2010, 2012  Thomas Mertes     */
/*           2013, 2015 - 2025  Thomas Mertes                       */
/*  Content: Primitive actions to implement simple statements.      */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"

#include "common.h"
#include "sigutl.h"
#include "data.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "heaputl.h"
#include "flistutl.h"
#include "striutl.h"
#include "entutl.h"
#include "syvarutl.h"
#include "traceutl.h"
#include "typeutl.h"
#include "listutl.h"
#include "executl.h"
#include "objutl.h"
#include "findid.h"
#include "match.h"
#include "name.h"
#include "exec.h"
#include "runerr.h"
#include "blockutl.h"
#include "actutl.h"
#include "scanner.h"
#include "libpath.h"
#include "error.h"
#include "set_rtl.h"
#include "str_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "prclib.h"



static void fix_posinfo (objectType block_body, const const_objectType block_body_list)

  { /* fix_posinfo */
    if (block_body != NULL && block_body_list != NULL &&
        CATEGORY_OF_OBJ(block_body) == CALLOBJECT &&
        !HAS_POSINFO(block_body)) {
      block_body->descriptor.posinfo = block_body_list->descriptor.posinfo;
      SET_POSINFO_FLAG(block_body);
    } /* if */
  } /* fix_posinfo */



static objectType process_local_decl (objectType local_decl,
    listType *local_object_list, errInfoType *err_info)

  {
    const_listType local_element;
    objectType local_var;
    objectType init_value;
    objectType result;

  /* process_local_decl */
    logFunction(printf("process_local_decl(");
                trace1(local_decl);
                printf(", " FMT_X_MEM ")\n",
                       (memSizeType) local_object_list););
    result = do_exec_call(local_decl, err_info);
    if (result == SYS_EMPTY_OBJECT) {
      local_element = *local_object_list;
      while (local_element != NULL) {
        if (VAR_OBJECT(local_element->obj)) {
          local_var = local_element->obj;
          if (CATEGORY_OF_OBJ(local_var) != LOCALVOBJECT) {
            if (likely(ALLOC_OBJECT(init_value))) {
              init_value->type_of =     local_var->type_of;
              init_value->descriptor.property = NULL;
              init_value->value =       local_var->value;
              init_value->objcategory = local_var->objcategory;
              SET_CATEGORY_OF_OBJ(local_var, LOCALVOBJECT);
              local_var->value.objValue = init_value; /* was NULL; changed for s7c.sd7 */
            } else {
              *err_info = MEMORY_ERROR;
            } /* if */
          } /* if */
        } /* if */
        local_element = local_element->next;
      } /* while */
    } /* if */
    logFunction(printf("process_local_decl --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* process_local_decl */



static objectType evaluate_local_decls (objectType local_decls,
    listType *local_object_list, errInfoType *err_info)

  {
    listType semicol_params;
    boolType finished = FALSE;
    objectType result;

  /* evaluate_local_decls */
    logFunction(printf("evaluate_local_decls(");
                trace1(local_decls);
                printf(", " FMT_X_MEM ")\n",
                       (memSizeType) local_object_list););
    do {
      if (CATEGORY_OF_OBJ(local_decls) == MATCHOBJECT ||
          CATEGORY_OF_OBJ(local_decls) == CALLOBJECT) {
        semicol_params = local_decls->value.listValue;
        if (list_length(semicol_params) == 4 &&
            CATEGORY_OF_OBJ(arg_1(semicol_params)) == ACTOBJECT &&
            take_action(arg_1(semicol_params)) == &prc_semicolon) {
          result = process_local_decl(arg_2(semicol_params),
              local_object_list, err_info);
          local_decls = arg_4(semicol_params);
        } else {
          result = process_local_decl(local_decls,
              local_object_list, err_info);
          finished = TRUE;
        } /* if */
      } else {
        result = process_local_decl(local_decls,
              local_object_list, err_info);
        finished = TRUE;
      } /* if */
    } while (!finished && result == SYS_EMPTY_OBJECT);
    logFunction(printf("evaluate_local_decls --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* evaluate_local_decls */



/**
 *  Return the argument vector of the program as array of strings.
 *  The name of the program is not part of the argument vector.
 *  @return an array of strings containing the argument vector.
 */
objectType prc_args (listType arguments)

  { /* prc_args */
    logFunction(printf("prc_args\n"););
    if (unlikely(prog->arg_v == NULL)) {
      logError(printf("prc_args: Argument vector is NULL.\n"););
      return raise_with_arguments(SYS_RNG_EXCEPTION, arguments);
    } else {
      logFunction(printf("prc_args --> ");
                  trace1(prog->arg_v);
                  printf("\n"););
      return prog->arg_v;
    } /* if */
  } /* prc_args */



objectType prc_begin (listType arguments)

  {
    objectType block_body;
    objectType block_body_list = NULL;
    objectType proc_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_begin */
    logFunction(printf("prc_begin\n"););
    block_body = arg_3(arguments);
    proc_exec_object = curr_exec_object;
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("prc_begin: No memory\n"););
      return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                     proc_exec_object, arguments);
    } else {
      push_stack();
      if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
        update_owner(block_body);
        if (match_expression(block_body) == NULL) {
          free_expression(block_body);
          block_body = NULL;
        } /* if */
      } /* if */
      if (block_body != NULL) {
        block_body = match_object(block_body);
        fix_posinfo(block_body, block_body_list);
      } /* if */
      if (block_body != NULL && block_body->type_of != take_type(SYS_PROC_TYPE)) {
        err_type(PROC_EXPECTED, block_body);
      } /* if */
      pop_stack();
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        logError(printf("prc_begin: error - err_info: %d\n", err_info););
        free_expression(block_body);
        return raise_with_obj_and_args(prog->sys_var[err_info],
                                       proc_exec_object, arguments);
      } else if (unlikely(block_body == NULL)) {
        logError(printf("prc_begin: Create error\n"););
        free_expression(block_body);
        return raise_with_obj_and_args(prog->sys_var[CREATE_ERROR],
                                       proc_exec_object, arguments);
      } else if (unlikely((block =
          new_block(NULL, NULL, NULL, NULL, block_body)) == NULL)) {
        logError(printf("prc_begin: No memory\n"););
        free_expression(block_body);
        return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                       proc_exec_object, arguments);
      } else {
        logFunction(printf("prc_begin -->\n"););
        return bld_block_temp(block);
      } /* if */
    } /* if */
  } /* prc_begin */



objectType prc_begin_noop (listType arguments)

  { /* prc_begin_noop */
    return bld_action_temp(prc_noop);
  } /* prc_begin_noop */



objectType prc_block (listType arguments)

  {
    objectType statement;
    objectType current_catch;
    objectType catch_value;
    objectType catch_statement;
    boolType searching;

  /* prc_block */
    statement = arg_2(arguments);
    evaluate(statement);
    if (unlikely(fail_flag && catch_exceptions)) {
      searching = TRUE;
      current_catch = arg_4(arguments);
      while (current_catch != NULL && searching &&
          CATEGORY_OF_OBJ(current_catch) == MATCHOBJECT &&
          current_catch->value.listValue->next->next->next->next != NULL) {
        catch_value = arg_3(current_catch->value.listValue);
        if (catch_value == fail_value) {
          catch_statement = arg_5(current_catch->value.listValue);
          leaveExceptionHandling();
          evaluate(catch_statement);
          searching = FALSE;
        } else {
          if (current_catch->value.listValue->next->next->next->next->next != NULL) {
            current_catch = arg_6(current_catch->value.listValue);
          } else {
            current_catch = NULL;
          } /* if */
        } /* if */
      } /* while */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_block */



objectType prc_block_catch_all (listType arguments)

  {
    objectType statement;
    objectType default_statement;

  /* prc_block_catch_all */
    statement = arg_2(arguments);
    evaluate(statement);
    if (unlikely(fail_flag && catch_exceptions)) {
      default_statement = arg_6(arguments);
      leaveExceptionHandling();
      evaluate(default_statement);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_block_catch_all */



objectType prc_block_otherwise (listType arguments)

  {
    objectType statement;
    objectType otherwise_statement;
    objectType current_catch;
    objectType catch_value;
    objectType catch_statement;
    boolType searching;

  /* prc_block_otherwise */
    statement = arg_2(arguments);
    evaluate(statement);
    if (unlikely(fail_flag && catch_exceptions)) {
      searching = TRUE;
      current_catch = arg_4(arguments);
      while (current_catch != NULL && searching &&
          CATEGORY_OF_OBJ(current_catch) == MATCHOBJECT &&
          current_catch->value.listValue->next->next->next->next != NULL) {
        catch_value = arg_3(current_catch->value.listValue);
        if (catch_value == fail_value) {
          catch_statement = arg_5(current_catch->value.listValue);
          leaveExceptionHandling();
          evaluate(catch_statement);
          searching = FALSE;
        } else {
          if (current_catch->value.listValue->next->next->next->next->next != NULL) {
            current_catch = arg_6(current_catch->value.listValue);
          } else {
            current_catch = NULL;
          } /* if */
        } /* if */
      } /* while */
      if (searching) {
        otherwise_statement = arg_7(arguments);
        leaveExceptionHandling();
        evaluate(otherwise_statement);
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_block_otherwise */



objectType prc_case (listType arguments)

  {
    objectType switch_object;
    intType switch_value;
    objectType when_objects;
    objectType current_when;
    objectType when_values;
    objectType when_set;
    setType set_value;
    objectType when_statement = NULL;
    errInfoType err_info = OKAY_NO_ERROR;
    listType err_arguments;

  /* prc_case */
    logFunction(printf("prc_case\n"););
    switch_object = arg_2(arguments);
    when_objects = arg_4(arguments);
    current_when = when_objects;
    err_arguments = arguments;
    switch_value = do_ord(switch_object, &err_info);
    while (err_info == OKAY_NO_ERROR && current_when != NULL &&
        CATEGORY_OF_OBJ(current_when) == MATCHOBJECT &&
        current_when->value.listValue->next->next->next->next != NULL) {
      when_values = arg_3(current_when->value.listValue);
      if (CATEGORY_OF_OBJ(when_values) != SETOBJECT) {
        when_set = exec_object(when_values);
        isit_not_null(when_set);
        isit_set(when_set);
        set_value = take_set(when_set);
        if (TEMP_OBJECT(when_set)) {
          if (CATEGORY_OF_OBJ(when_values) == CALLOBJECT) {
            free_expression_list(take_list(when_values));
          } /* if */
          when_values->type_of = NULL;
          when_values->descriptor.property = NULL;
          SET_CATEGORY_OF_OBJ(when_values, SETOBJECT);
          when_values->value.setValue = set_value;
          current_when->value.listValue->next->next->obj = when_values;
          incl_list(&prog->when_value_objects, when_values, &err_info);
          incl_list(&prog->when_set_objects, when_set, &err_info);
        } /* if */
      } else {
        set_value = take_set(when_values);
      } /* if */
      if (setElem(switch_value, set_value)) {
        if (unlikely(when_statement != NULL)) {
          logError(printf("prc_case(" FMT_D "): "
                          FMT_D " is in more then one \"when\" set.\n",
                          switch_value, switch_value););
          err_info = ACTION_ERROR;
          err_arguments = current_when->value.listValue->next;
        } else {
          when_statement = arg_5(current_when->value.listValue);
        } /* if */
      } /* if */
      if (current_when->value.listValue->next->next->next->next->next != NULL) {
        current_when = arg_6(current_when->value.listValue);
      } else {
        current_when = NULL;
      } /* if */
    } /* while */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_with_arguments(prog->sys_var[err_info], err_arguments);
    } else if (when_statement != NULL) {
      evaluate(when_statement);
    } /* if */
    logFunction(printf("prc_case -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* prc_case */



objectType prc_case_def (listType arguments)

  {
    objectType switch_object;
    intType switch_value;
    objectType when_objects;
    objectType default_statement;
    objectType current_when;
    objectType when_values;
    objectType when_set;
    setType set_value;
    objectType when_statement = NULL;
    errInfoType err_info = OKAY_NO_ERROR;
    listType err_arguments;

  /* prc_case_def */
    logFunction(printf("prc_case_def\n"););
    switch_object = arg_2(arguments);
    when_objects = arg_4(arguments);
    current_when = when_objects;
    err_arguments = arguments;
    switch_value = do_ord(switch_object, &err_info);
    while (err_info == OKAY_NO_ERROR && current_when != NULL &&
        CATEGORY_OF_OBJ(current_when) == MATCHOBJECT &&
        current_when->value.listValue->next->next->next->next != NULL) {
      when_values = arg_3(current_when->value.listValue);
      if (CATEGORY_OF_OBJ(when_values) != SETOBJECT) {
        when_set = exec_object(when_values);
        isit_not_null(when_set);
        isit_set(when_set);
        set_value = take_set(when_set);
        if (TEMP_OBJECT(when_set)) {
          if (CATEGORY_OF_OBJ(when_values) == CALLOBJECT) {
            free_expression_list(take_list(when_values));
          } /* if */
          when_values->type_of = NULL;
          when_values->descriptor.property = NULL;
          SET_CATEGORY_OF_OBJ(when_values, SETOBJECT);
          when_values->value.setValue = set_value;
          current_when->value.listValue->next->next->obj = when_values;
          incl_list(&prog->when_value_objects, when_values, &err_info);
          incl_list(&prog->when_set_objects, when_set, &err_info);
        } /* if */
      } else {
        set_value = take_set(when_values);
      } /* if */
      if (setElem(switch_value, set_value)) {
        if (unlikely(when_statement != NULL)) {
          logError(printf("prc_case_def(" FMT_D "): "
                          FMT_D " is in more then one \"when\" set.\n",
                          switch_value, switch_value););
          err_info = ACTION_ERROR;
          err_arguments = current_when->value.listValue->next;
        } else {
          when_statement = arg_5(current_when->value.listValue);
        } /* if */
      } /* if */
      if (current_when->value.listValue->next->next->next->next->next != NULL) {
        current_when = arg_6(current_when->value.listValue);
      } else {
        current_when = NULL;
      } /* if */
    } /* while */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_with_arguments(prog->sys_var[err_info], err_arguments);
    } else if (when_statement != NULL) {
      evaluate(when_statement);
    } else {
      default_statement = arg_7(arguments);
      evaluate(default_statement);
    } /* if */
    logFunction(printf("prc_case_def -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* prc_case_def */



objectType prc_case_hashset (listType arguments)

  {
    objectType switch_object;
    objectType when_objects;
    objectType current_when;
    objectType when_values;
    objectType when_set;
    hashType hashMap_value;
    objectType when_statement = NULL;
    errInfoType err_info = OKAY_NO_ERROR;
    listType err_arguments;

  /* prc_case_hashset */
    logFunction(printf("prc_case_hashset\n"););
    switch_object = arg_2(arguments);
    when_objects = arg_4(arguments);
    current_when = when_objects;
    err_arguments = arguments;
    while (err_info == OKAY_NO_ERROR && current_when != NULL &&
        CATEGORY_OF_OBJ(current_when) == MATCHOBJECT &&
        current_when->value.listValue->next->next->next->next != NULL) {
      when_values = arg_3(current_when->value.listValue);
      if (CATEGORY_OF_OBJ(when_values) != HASHOBJECT) {
        when_set = exec_object(when_values);
        isit_not_null(when_set);
        isit_hash(when_set);
        hashMap_value = take_hash(when_set);
        if (TEMP_OBJECT(when_set)) {
          if (CATEGORY_OF_OBJ(when_values) == CALLOBJECT) {
            free_expression_list(take_list(when_values));
          } /* if */
          when_values->type_of = when_set->type_of;
          when_values->descriptor.property = NULL;
          SET_CATEGORY_OF_OBJ(when_values, HASHOBJECT);
          when_values->value.hashValue = hashMap_value;
          current_when->value.listValue->next->next->obj = when_values;
          incl_list(&prog->when_value_objects, when_values, &err_info);
          incl_list(&prog->when_set_objects, when_set, &err_info);
        } /* if */
      } /* if */
      if (do_in(switch_object, when_values, &err_info)) {
        if (unlikely(when_statement != NULL)) {
          logError(printf("prc_case_hashset: "
                          "Switch value in more then one \"when\" set.\n"););
          err_info = ACTION_ERROR;
          err_arguments = current_when->value.listValue->next;
        } else {
          when_statement = arg_5(current_when->value.listValue);
        } /* if */
      } /* if */
      if (current_when->value.listValue->next->next->next->next->next != NULL) {
        current_when = arg_6(current_when->value.listValue);
      } else {
        current_when = NULL;
      } /* if */
    } /* while */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_with_arguments(prog->sys_var[err_info], err_arguments);
    } else if (when_statement != NULL) {
      evaluate(when_statement);
    } /* if */
    logFunction(printf("prc_case_hashset -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* prc_case_hashset */



objectType prc_case_hashset_def (listType arguments)

  {
    objectType switch_object;
    objectType when_objects;
    objectType default_statement;
    objectType current_when;
    objectType when_values;
    objectType when_set;
    hashType hashMap_value;
    objectType when_statement = NULL;
    errInfoType err_info = OKAY_NO_ERROR;
    listType err_arguments;

  /* prc_case_hashset_def */
    logFunction(printf("prc_case_hashset_def\n"););
    switch_object = arg_2(arguments);
    when_objects = arg_4(arguments);
    current_when = when_objects;
    err_arguments = arguments;
    while (err_info == OKAY_NO_ERROR && current_when != NULL &&
        CATEGORY_OF_OBJ(current_when) == MATCHOBJECT &&
        current_when->value.listValue->next->next->next->next != NULL) {
      when_values = arg_3(current_when->value.listValue);
      if (CATEGORY_OF_OBJ(when_values) != HASHOBJECT) {
        when_set = exec_object(when_values);
        isit_not_null(when_set);
        isit_hash(when_set);
        hashMap_value = take_hash(when_set);
        if (TEMP_OBJECT(when_set)) {
          if (CATEGORY_OF_OBJ(when_values) == CALLOBJECT) {
            free_expression_list(take_list(when_values));
          } /* if */
          when_values->type_of = when_set->type_of;
          when_values->descriptor.property = NULL;
          SET_CATEGORY_OF_OBJ(when_values, HASHOBJECT);
          when_values->value.hashValue = hashMap_value;
          current_when->value.listValue->next->next->obj = when_values;
          incl_list(&prog->when_value_objects, when_values, &err_info);
          incl_list(&prog->when_set_objects, when_set, &err_info);
        } /* if */
      } /* if */
      if (do_in(switch_object, when_values, &err_info)) {
        if (unlikely(when_statement != NULL)) {
          logError(printf("prc_case_hashset_def: "
                          "Switch value in more then one \"when\" set.\n"););
          err_info = ACTION_ERROR;
          err_arguments = current_when->value.listValue->next;
        } else {
          when_statement = arg_5(current_when->value.listValue);
        } /* if */
      } /* if */
      if (current_when->value.listValue->next->next->next->next->next != NULL) {
        current_when = arg_6(current_when->value.listValue);
      } else {
        current_when = NULL;
      } /* if */
    } /* while */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_with_arguments(prog->sys_var[err_info], err_arguments);
    } else if (when_statement != NULL) {
      evaluate(when_statement);
    } else {
      default_statement = arg_7(arguments);
      evaluate(default_statement);
    } /* if */
    logFunction(printf("prc_case_hashset_def -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* prc_case_hashset_def */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType prc_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    blockType block_source;
    blockType old_block;

  /* prc_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_proc(dest);
    isit_proc(source);
    logFunction(printf("prc_cpy(");
                if (CATEGORY_OF_OBJ(take_act_obj(dest)) == BLOCKOBJECT) {
                  printf("block " FMT_U_MEM " (usage=" FMT_U_MEM "), ",
                         (memSizeType) take_block(take_act_obj(dest)),
                         take_block(take_act_obj(dest)) != NULL ?
                             take_block(take_act_obj(dest))->usage_count :
                             (memSizeType) 0);
                } else if (CATEGORY_OF_OBJ(take_act_obj(dest)) == ACTOBJECT) {
                  printf("action \"%s\", ",
                         getActEntry(take_action(dest))->name);
                } else {
                  printf("category %u, ", CATEGORY_OF_OBJ(dest));
                }
                if (CATEGORY_OF_OBJ(take_act_obj(source)) == BLOCKOBJECT) {
                  printf("block " FMT_U_MEM " (usage=" FMT_U_MEM "))\n",
                         (memSizeType) take_block(take_act_obj(source)),
                         take_block(take_act_obj(source)) != NULL ?
                             take_block(take_act_obj(source))->usage_count :
                             (memSizeType) 0);
                } else if (CATEGORY_OF_OBJ(take_act_obj(source)) == ACTOBJECT) {
                  printf("action \"%s\")\n",
                         getActEntry(take_action(source))->name);
                } else {
                  printf("category %u)\n", CATEGORY_OF_OBJ(source));
                });
    if (CATEGORY_OF_OBJ(dest) == MATCHOBJECT) {
      if (unlikely(dest->value.listValue->next != NULL)) {
        logError(printf("prc_cpy: Destination with parameters.\n"););
        return raise_exception(SYS_ACT_ILLEGAL_EXCEPTION);
      } else {
        dest = dest->value.listValue->obj;
      } /* if */
    } /* if */
    is_variable(dest);
    if (CATEGORY_OF_OBJ(source) == MATCHOBJECT) {
      if (unlikely(source->value.listValue->next != NULL)) {
        logError(printf("prc_cpy: Source with parameters.\n"););
        return raise_exception(SYS_ACT_ILLEGAL_EXCEPTION);
      } else {
        source = source->value.listValue->obj;
      } /* if */
    } /* if */
    if (CATEGORY_OF_OBJ(source) == BLOCKOBJECT) {
      block_source = take_block(source);
      if (TEMP_OBJECT(source)) {
        source->value.blockValue = NULL;
      } else {
        if (block_source != NULL && block_source->usage_count != 0) {
          block_source->usage_count++;
        } /* if */
      } /* if */
      if (CATEGORY_OF_OBJ(dest) == BLOCKOBJECT) {
        old_block = take_block(dest);
        if (old_block != NULL && old_block->usage_count != 0) {
          old_block->usage_count--;
          if (old_block->usage_count == 0) {
            free_block(old_block);
          } /* if */
        } /* if */
      } else if (CATEGORY_OF_OBJ(dest) == ACTOBJECT) {
        SET_CATEGORY_OF_OBJ(dest, BLOCKOBJECT);
      } else {
        return raise_exception(SYS_ACT_ILLEGAL_EXCEPTION);
      } /* if */
      dest->value.blockValue = block_source;
    } else if (CATEGORY_OF_OBJ(source) == ACTOBJECT) {
      if (CATEGORY_OF_OBJ(dest) == BLOCKOBJECT) {
        old_block = take_block(dest);
        if (old_block != NULL && old_block->usage_count != 0) {
          old_block->usage_count--;
          if (old_block->usage_count == 0) {
            free_block(old_block);
          } /* if */
        } /* if */
        SET_CATEGORY_OF_OBJ(dest, ACTOBJECT);
      } else if (CATEGORY_OF_OBJ(dest) != ACTOBJECT) {
        return raise_exception(SYS_ACT_ILLEGAL_EXCEPTION);
      } /* if */
      dest->value.actValue = source->value.actValue;
    } else {
      logError(printf("prc_cpy: source category %d neither "
                       "BLOCKOBJECT nor ACTOBJECT.\n",
                       CATEGORY_OF_OBJ(source)););
      return raise_exception(SYS_ACT_ILLEGAL_EXCEPTION);
    } /* if */
    logFunction(printf("prc_cpy(");
                if (CATEGORY_OF_OBJ(take_act_obj(dest)) == BLOCKOBJECT) {
                  printf("block " FMT_U_MEM " (usage=" FMT_U_MEM "), ",
                         (memSizeType) take_block(take_act_obj(dest)),
                         take_block(take_act_obj(dest)) != NULL ?
                             take_block(take_act_obj(dest))->usage_count :
                             (memSizeType) 0);
                } else if (CATEGORY_OF_OBJ(take_act_obj(dest)) == ACTOBJECT) {
                  printf("action \"%s\", ",
                         getActEntry(take_action(dest))->name);
                } else {
                  printf("category %u, ", CATEGORY_OF_OBJ(dest));
                }
                if (CATEGORY_OF_OBJ(take_act_obj(source)) == BLOCKOBJECT) {
                  printf("block " FMT_U_MEM " (usage=" FMT_U_MEM ")) -->\n",
                         (memSizeType) take_block(take_act_obj(source)),
                         take_block(take_act_obj(source)) != NULL ?
                             take_block(take_act_obj(source))->usage_count :
                             (memSizeType) 0);
                } else if (CATEGORY_OF_OBJ(take_act_obj(source)) == ACTOBJECT) {
                  printf("action \"%s\") -->\n",
                         getActEntry(take_action(source))->name);
                } else {
                  printf("category %u) -->\n", CATEGORY_OF_OBJ(source));
                });
    return SYS_EMPTY_OBJECT;
  } /* prc_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType prc_create (listType arguments)

  {
    objectType dest;
    objectType source;
    blockType block_value;

  /* prc_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    logFunction(printf("prc_create(" FMT_U_MEM ", ",
                        (memSizeType) dest);
                if (CATEGORY_OF_OBJ(take_act_obj(source)) == BLOCKOBJECT) {
                  printf("block " FMT_U_MEM " (usage=" FMT_U_MEM "))\n",
                         (memSizeType) take_block(take_act_obj(source)),
                         take_block(take_act_obj(source)) != NULL ?
                             take_block(take_act_obj(source))->usage_count :
                             (memSizeType) 0);
                } else if (CATEGORY_OF_OBJ(take_act_obj(source)) == ACTOBJECT) {
                  printf("action \"%s\")\n",
                         getActEntry(take_action(source))->name);
                } else {
                  printf("category %u)\n", CATEGORY_OF_OBJ(source));
                });
    if (CATEGORY_OF_OBJ(source) == BLOCKOBJECT) {
      SET_CATEGORY_OF_OBJ(dest, BLOCKOBJECT);
      block_value = take_block(source);
      dest->value.blockValue = block_value;
      if (TEMP_OBJECT(source)) {
        source->value.blockValue = NULL;
      } else {
        if (block_value != NULL && block_value->usage_count != 0) {
          block_value->usage_count++;
        } /* if */
      } /* if */
    } else if (CATEGORY_OF_OBJ(source) == ACTOBJECT) {
      SET_CATEGORY_OF_OBJ(dest, ACTOBJECT);
      dest->value.actValue = source->value.actValue;
    } else {
      logError(printf("prc_create: source category %d neither "
                       "BLOCKOBJECT nor ACTOBJECT.\n",
                       CATEGORY_OF_OBJ(source)););
      return raise_exception(SYS_ACT_ILLEGAL_EXCEPTION);
    } /* if */
    logFunction(printf("prc_create(");
                if (CATEGORY_OF_OBJ(take_act_obj(dest)) == BLOCKOBJECT) {
                  printf("block " FMT_U_MEM " (usage=" FMT_U_MEM "), ",
                         (memSizeType) take_block(take_act_obj(dest)),
                         take_block(take_act_obj(dest)) != NULL ?
                             take_block(take_act_obj(dest))->usage_count :
                             (memSizeType) 0);
                } else if (CATEGORY_OF_OBJ(take_act_obj(dest)) == ACTOBJECT) {
                  printf("action \"%s\", ",
                         getActEntry(take_action(dest))->name);
                } else {
                  printf("category %u, ", CATEGORY_OF_OBJ(dest));
                }
                if (CATEGORY_OF_OBJ(take_act_obj(source)) == BLOCKOBJECT) {
                  printf("block " FMT_U_MEM " (usage=" FMT_U_MEM ")) -->\n",
                         (memSizeType) take_block(take_act_obj(source)),
                         take_block(take_act_obj(source)) != NULL ?
                             take_block(take_act_obj(source))->usage_count :
                             (memSizeType) 0);
                } else if (CATEGORY_OF_OBJ(take_act_obj(source)) == ACTOBJECT) {
                  printf("action \"%s\") -->\n",
                         getActEntry(take_action(source))->name);
                } else {
                  printf("category %u) -->\n", CATEGORY_OF_OBJ(source));
                });
    return SYS_EMPTY_OBJECT;
  } /* prc_create */



objectType prc_decls (listType arguments)

  { /* prc_decls */
    trace_nodes();
    return SYS_EMPTY_OBJECT;
  } /* prc_decls */



/**
 *  Free the memory referred by 'old_prc/arg_1'.
 *  After prc_destr is left 'old_prc/arg_1' is NULL.
 *  The memory where 'old_prc/arg_1' is stored can be freed afterwards.
 */
objectType prc_destr (listType arguments)

  {
    objectType old_proc;
    blockType old_block;

  /* prc_destr */
    old_proc = arg_1(arguments);
    logFunction(printf("prc_destr(");
                if (CATEGORY_OF_OBJ(take_act_obj(old_proc)) == BLOCKOBJECT) {
                  printf("block " FMT_U_MEM " (usage=" FMT_U_MEM "))\n",
                         (memSizeType) take_block(take_act_obj(old_proc)),
                         take_block(take_act_obj(old_proc)) != NULL ?
                             take_block(take_act_obj(old_proc))->usage_count :
                             (memSizeType) 0);
                } else if (CATEGORY_OF_OBJ(take_act_obj(old_proc)) == ACTOBJECT) {
                  printf("action \"%s\")\n",
                         getActEntry(take_action(old_proc))->name);
                } else {
                  printf("category %u)\n", CATEGORY_OF_OBJ(old_proc));
                });
    if (CATEGORY_OF_OBJ(old_proc) == BLOCKOBJECT) {
      old_block = take_block(old_proc);
      if (old_block != NULL && old_block->usage_count != 0) {
        old_block->usage_count--;
        if (old_block->usage_count == 0) {
          free_block(old_block);
        } /* if */
        arg_1(arguments)->value.blockValue = NULL;
      } /* if */
    } /* if */
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* prc_destr */



objectType prc_dynamic (listType arguments)

  {
    objectType result;

  /* prc_dynamic */
    result = exec_dynamic(arguments);
    return result;
  } /* prc_dynamic */



objectType prc_exit (listType arguments)

  {
    intType status;

  /* prc_exit */
    isit_int(arg_1(arguments));
    status = take_int(arg_1(arguments));
    if (!inIntRange(status)) {
      logError(printf("prc_exit(" FMT_D "): "
                      "Exit status not in allowed range (%d .. %d).\n",
                      status, INT_MIN, INT_MAX););
      raise_error(RANGE_ERROR);
    } else {
      shutDrivers();
      os_exit((int) status);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_exit */



objectType prc_for_downto (listType arguments)

  {
    objectType for_variable;
    intType upper_limit;
    intType lower_limit;
    objectType statement;

  /* prc_for_downto */
    for_variable = arg_2(arguments);
    is_variable(for_variable);
    isit_int(for_variable);
    isit_int(arg_4(arguments));
    isit_int(arg_6(arguments));
    upper_limit = take_int(arg_4(arguments));
    lower_limit = take_int(arg_6(arguments));
    statement = arg_8(arguments);
    if (unlikely(lower_limit == INTTYPE_MIN)) {
      logError(printf("prc_for_downto(var1, " FMT_D ", " FMT_D "): "
                      "Lower limit of integer.first not allowed.\n",
                      upper_limit, lower_limit););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      for_variable->value.intValue = upper_limit;
      while (take_int(for_variable) >= lower_limit && !fail_flag) {
        evaluate(statement);
        if (!fail_flag) {
          for_variable->value.intValue--;
        } /* if */
      } /* while */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_for_downto */



objectType prc_for_downto_step (listType arguments)

  {
    objectType for_variable;
    intType upper_limit;
    intType lower_limit;
    intType incr_step;
    objectType statement;

  /* prc_for_downto_step */
    for_variable = arg_2(arguments);
    is_variable(for_variable);
    isit_int(for_variable);
    isit_int(arg_4(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_8(arguments));
    upper_limit = take_int(arg_4(arguments));
    lower_limit = take_int(arg_6(arguments));
    incr_step = take_int(arg_8(arguments));
    statement = arg_10(arguments);
    for_variable->value.intValue = upper_limit;
    while (take_int(for_variable) >= lower_limit && !fail_flag) {
      evaluate(statement);
      if (!fail_flag) {
        for_variable->value.intValue -= incr_step;
      } /* if */
    } /* while */
    return SYS_EMPTY_OBJECT;
  } /* prc_for_downto_step */



objectType prc_for_to (listType arguments)

  {
    objectType for_variable;
    intType lower_limit;
    intType upper_limit;
    objectType statement;

  /* prc_for_to */
    for_variable = arg_2(arguments);
    is_variable(for_variable);
    isit_int(for_variable);
    isit_int(arg_4(arguments));
    isit_int(arg_6(arguments));
    lower_limit = take_int(arg_4(arguments));
    upper_limit = take_int(arg_6(arguments));
    statement = arg_8(arguments);
    if (unlikely(upper_limit == INTTYPE_MAX)) {
      logError(printf("prc_for_to(var1, " FMT_D ", " FMT_D "): "
                      "Upper limit of integer.last not allowed.\n",
                      lower_limit, upper_limit););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      for_variable->value.intValue = lower_limit;
      while (take_int(for_variable) <= upper_limit && !fail_flag) {
        evaluate(statement);
        if (!fail_flag) {
          for_variable->value.intValue++;
        } /* if */
      } /* while */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_for_to */



objectType prc_for_to_step (listType arguments)

  {
    objectType for_variable;
    intType lower_limit;
    intType upper_limit;
    intType incr_step;
    objectType statement;

  /* prc_for_to_step */
    for_variable = arg_2(arguments);
    is_variable(for_variable);
    isit_int(for_variable);
    isit_int(arg_4(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_8(arguments));
    lower_limit = take_int(arg_4(arguments));
    upper_limit = take_int(arg_6(arguments));
    incr_step = take_int(arg_8(arguments));
    statement = arg_10(arguments);
    for_variable->value.intValue = lower_limit;
    while (take_int(for_variable) <= upper_limit && !fail_flag) {
      evaluate(statement);
      if (!fail_flag) {
        for_variable->value.intValue += incr_step;
      } /* if */
    } /* while */
    return SYS_EMPTY_OBJECT;
  } /* prc_for_to_step */



objectType prc_heapstat (listType arguments)

  { /* prc_heapstat */
    heapStatistic();
    return SYS_EMPTY_OBJECT;
  } /* prc_heapstat */



objectType prc_hsize (listType arguments)

  { /* prc_hsize */
    /* heapStatistic(); */
    return bld_int_temp((intType) heapsize());
  } /* prc_hsize */



objectType prc_if (listType arguments)

  {
    objectType condition;

  /* prc_if */
    isit_bool(arg_2(arguments));
    condition = take_bool(arg_2(arguments));
    if (condition == SYS_TRUE_OBJECT) {
      evaluate(arg_4(arguments));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_if */



objectType prc_if_elsif (listType arguments)

  {
    objectType condition;

  /* prc_if_elsif */
    isit_bool(arg_2(arguments));
    condition = take_bool(arg_2(arguments));
    if (condition == SYS_TRUE_OBJECT) {
      evaluate(arg_4(arguments));
    } else {
      evaluate(arg_5(arguments));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_if_elsif */



objectType prc_if_noop (listType arguments)

  {
    objectType condition;

  /* prc_if_noop */
    isit_bool(arg_2(arguments));
    condition = take_bool(arg_2(arguments));
    if (condition != SYS_TRUE_OBJECT) {
      evaluate(arg_4(arguments));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_if_noop */



objectType prc_include (listType arguments)

  {
    striType includeFileName;
    includeResultType includeResult;
    errInfoType err_info = OKAY_NO_ERROR;

  /* prc_include */
    isit_stri(arg_2(arguments));
    includeFileName = take_stri(arg_2(arguments));
    logFunction(printf("prc_include(\"%s\")\n",
                       striAsUnquotedCStri(includeFileName)));
    if (strChPos(includeFileName, (charType) '\\') != 0) {
      err_stri(WRONG_PATH_DELIMITER, includeFileName);
    } else {
      includeResult = findIncludeFile((rtlHashType) prog->includeFileHash,
                                      includeFileName, &err_info);
      if (unlikely(includeResult == INCLUDE_FAILED)) {
        if (err_info == ACTION_ERROR) {
          /* This is a compile-time function and it is called at run-time. */
          return raise_with_arguments(SYS_ACT_ILLEGAL_EXCEPTION, arguments);
        } else if (err_info == MEMORY_ERROR) {
          err_warning(OUT_OF_HEAP_SPACE);
        } else {
          /* FILE_ERROR or RANGE_ERROR */
          err_stri(INCLUDE_FILE_NOT_FOUND, includeFileName);
        } /* if */
      } else if (includeResult == INCLUDE_SUCCESS) {
        scan_byte_order_mark();
        scan_symbol();
      } /* if */
    } /* if */
    logFunction(printf("prc_include -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* prc_include */



objectType prc_line (listType arguments)

  { /* prc_line */
    return bld_int_temp((intType) POSINFO_LINE_NUM(curr_exec_object));
  } /* prc_line */



objectType prc_local (listType arguments)

  {
    objectType local_decls;
    objectType block_body;
    objectType block_body_list = NULL;
    listType *local_object_insert_place;
    locListType local_vars;
    listType local_consts;
    objectType decl_res;
    objectType proc_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_local */
    logFunction(printf("prc_local\n"););
    local_decls = arg_3(arguments);
    block_body = arg_5(arguments);
    proc_exec_object = curr_exec_object;
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("prc_local: No memory\n"););
      return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                     proc_exec_object, arguments);
    } else {
      push_stack();
      local_object_insert_place = get_local_object_insert_place();
      decl_res = evaluate_local_decls(local_decls, local_object_insert_place, &err_info);
      if (decl_res != SYS_EMPTY_OBJECT) {
        /* printf("eval local decls --> ");
        trace1(decl_res);
        printf("\n");
        trace1(SYS_EMPTY_OBJECT);
        printf("\n"); */
        err_object(PROC_EXPECTED, decl_res);
      } /* if */
      local_vars = get_local_var_list(*local_object_insert_place, &err_info);
      local_consts = get_local_const_list(*local_object_insert_place, &err_info);
      if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
        update_owner(block_body);
        if (match_expression(block_body) == NULL) {
          free_expression(block_body);
          block_body = NULL;
        } /* if */
      } /* if */
      if (block_body != NULL) {
        block_body = match_object(block_body);
        fix_posinfo(block_body, block_body_list);
      } /* if */
      if (block_body != NULL && block_body->type_of != take_type(SYS_PROC_TYPE)) {
        err_type(PROC_EXPECTED, block_body);
      } /* if */
      pop_stack();
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        logError(printf("prc_local: error - err_info: %d\n", err_info););
        free_expression(block_body);
        free_local_consts(local_consts);
        free_loclist(local_vars);
        return raise_with_obj_and_args(prog->sys_var[err_info],
                                       proc_exec_object, arguments);
      } else if (unlikely(block_body == NULL)) {
        logError(printf("prc_local: Create error\n"););
        free_expression(block_body);
        free_local_consts(local_consts);
        free_loclist(local_vars);
        return raise_with_obj_and_args(prog->sys_var[CREATE_ERROR],
                                       proc_exec_object, arguments);
      } else if (unlikely((block =
          new_block(NULL, NULL, local_vars, local_consts, block_body)) == NULL)) {
        logError(printf("prc_local: No memory\n"););
        free_expression(block_body);
        free_local_consts(local_consts);
        free_loclist(local_vars);
        return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                       proc_exec_object, arguments);
      } else {
        logFunction(printf("prc_local -->\n"););
        return bld_block_temp(block);
      } /* if */
    } /* if */
  } /* prc_local */



objectType prc_noop (listType arguments)

  { /* prc_noop */
    return SYS_EMPTY_OBJECT;
  } /* prc_noop */



objectType prc_raise (listType arguments)

  { /* prc_raise */
    isit_enum(arg_2(arguments));
    return raise_exception(take_enum(arg_2(arguments)));
  } /* prc_raise */



objectType prc_repeat (listType arguments)

  {
    objectType statement;
    objectType condition;
    objectType cond_value;
    boolType cond;

  /* prc_repeat */
    statement = arg_2(arguments);
    condition = arg_4(arguments);
    do {
      evaluate(statement);
      if (likely(!fail_flag)) {
        cond_value = evaluate(condition);
        if (likely(!fail_flag)) {
          isit_bool(cond_value);
          cond = (boolType) (take_bool(cond_value) == SYS_FALSE_OBJECT);
          if (TEMP_OBJECT(cond_value) &&
              (CATEGORY_OF_OBJ(cond_value) == CONSTENUMOBJECT ||
               CATEGORY_OF_OBJ(cond_value) == VARENUMOBJECT)) {
            FREE_OBJECT(cond_value);
          } /* if */
        } /* if */
      } /* if */
    } while (!fail_flag && cond);
    return SYS_EMPTY_OBJECT;
  } /* prc_repeat */



objectType prc_repeat_noop (listType arguments)

  {
    objectType condition;
    objectType cond_value;
    boolType cond;

  /* prc_repeat_noop */
    condition = arg_3(arguments);
    do {
      cond_value = evaluate(condition);
      if (likely(!fail_flag)) {
        isit_bool(cond_value);
        cond = (boolType) (take_bool(cond_value) == SYS_FALSE_OBJECT);
        if (TEMP_OBJECT(cond_value) &&
            (CATEGORY_OF_OBJ(cond_value) == CONSTENUMOBJECT ||
             CATEGORY_OF_OBJ(cond_value) == VARENUMOBJECT)) {
          FREE_OBJECT(cond_value);
        } /* if */
      } /* if */
    } while (!fail_flag && cond);
    return SYS_EMPTY_OBJECT;
  } /* prc_repeat_noop */



objectType prc_res_begin (listType arguments)

  {
    typeType result_type;
    objectType result_var_name;
    locObjRecord result_var;
    objectType result_init;
    objectType block_body;
    objectType block_body_list = NULL;
    objectType proc_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_res_begin */
    logFunction(printf("prc_res_begin\n"););
    isit_type(arg_4(arguments));
    result_type = take_type(arg_4(arguments));
    result_var_name = arg_6(arguments);
    result_init = arg_8(arguments);
    block_body = arg_10(arguments);
    proc_exec_object = curr_exec_object;
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("prc_res_begin: No memory\n"););
      return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                     proc_exec_object, arguments);
    } else {
      push_stack();
      grow_stack(&err_info);
      memset(&result_var, 0, sizeof(locObjRecord));
      if (err_info == OKAY_NO_ERROR) {
        result_var.object = entername(prog->declaration_root, result_var_name, &err_info);
        shrink_stack();
      } /* if */
      if (err_info == OKAY_NO_ERROR && result_var.object != NULL) {
        get_result_var(&result_var, result_type, result_init, &err_info);
        /* printf("result_var.object ");
        trace1(result_var.object);
        printf("\n");
        printf("result_var.init_value ");
        trace1(result_var.init_value);
        printf("\n"); */
        if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
          update_owner(block_body);
          if (match_expression(block_body) == NULL) {
            free_expression(block_body);
            block_body = NULL;
          } /* if */
        } /* if */
        if (block_body != NULL) {
          block_body = match_object(block_body);
          fix_posinfo(block_body, block_body_list);
        } /* if */
        if (block_body != NULL && block_body->type_of != take_type(SYS_PROC_TYPE)) {
          err_type(PROC_EXPECTED, block_body);
        } /* if */
      } /* if */
      pop_stack();
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        logError(printf("prc_res_begin: error - err_info: %d\n", err_info););
        free_expression(block_body);
        free_locobj(&result_var);
        return raise_with_obj_and_args(prog->sys_var[err_info],
                                       proc_exec_object, arguments);
      } else if (unlikely(result_var.object == NULL || block_body == NULL)) {
        logError(printf("prc_res_begin: Create error\n"););
        free_expression(block_body);
        free_locobj(&result_var);
        return raise_with_obj_and_args(prog->sys_var[CREATE_ERROR],
                                       proc_exec_object, arguments);
      } else if (unlikely((block =
          new_block(NULL, &result_var, NULL, NULL, block_body)) == NULL)) {
        logError(printf("prc_res_begin: No memory\n"););
        free_expression(block_body);
        free_locobj(&result_var);
        return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                       proc_exec_object, arguments);
      } else {
        logFunction(printf("prc_res_begin -->\n"););
        return bld_block_temp(block);
      } /* if */
    } /* if */
  } /* prc_res_begin */



objectType prc_res_local (listType arguments)

  {
    typeType result_type;
    objectType result_var_name;
    locObjRecord result_var;
    objectType result_init;
    objectType local_decls;
    objectType block_body;
    objectType block_body_list = NULL;
    listType *local_object_insert_place;
    locListType local_vars;
    listType local_consts;
    objectType decl_res;
    objectType proc_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_res_local */
    logFunction(printf("prc_res_local\n"););
    isit_type(arg_4(arguments));
    result_type = take_type(arg_4(arguments));
    result_var_name = arg_6(arguments);
    result_init = arg_8(arguments);
    local_decls = arg_10(arguments);
    block_body = arg_12(arguments);
    proc_exec_object = curr_exec_object;
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("prc_res_local: No memory\n"););
      return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                     proc_exec_object, arguments);
    } else {
      push_stack();
      grow_stack(&err_info);
      memset(&result_var, 0, sizeof(locObjRecord));
      if (err_info == OKAY_NO_ERROR) {
        result_var.object = entername(prog->declaration_root, result_var_name, &err_info);
        shrink_stack();
      } /* if */
      if (err_info == OKAY_NO_ERROR && result_var.object != NULL) {
        get_result_var(&result_var, result_type, result_init, &err_info);
        local_object_insert_place = get_local_object_insert_place();
        decl_res = evaluate_local_decls(local_decls, local_object_insert_place, &err_info);
        if (decl_res != SYS_EMPTY_OBJECT) {
          /* printf("eval local decls --> ");
          trace1(decl_res);
          printf("\n");
          trace1(SYS_EMPTY_OBJECT);
          printf("\n"); */
          err_object(PROC_EXPECTED, decl_res);
        } /* if */
        local_vars = get_local_var_list(*local_object_insert_place, &err_info);
        local_consts = get_local_const_list(*local_object_insert_place, &err_info);
        if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
          update_owner(block_body);
          if (match_expression(block_body) == NULL) {
            free_expression(block_body);
            block_body = NULL;
          } /* if */
        } /* if */
        if (block_body != NULL) {
          block_body = match_object(block_body);
          fix_posinfo(block_body, block_body_list);
        } /* if */
        if (block_body != NULL && block_body->type_of != take_type(SYS_PROC_TYPE)) {
          err_type(PROC_EXPECTED, block_body);
        } /* if */
      } else {
        local_vars = NULL;
        local_consts = NULL;
      } /* if */
      pop_stack();
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        logError(printf("prc_res_local: error - err_info: %d\n", err_info););
        free_expression(block_body);
        free_local_consts(local_consts);
        free_loclist(local_vars);
        free_locobj(&result_var);
        return raise_with_obj_and_args(prog->sys_var[err_info],
                                       proc_exec_object, arguments);
      } else if (unlikely(result_var.object == NULL || block_body == NULL)) {
        logError(printf("prc_res_local: Create error\n"););
        free_expression(block_body);
        free_local_consts(local_consts);
        free_loclist(local_vars);
        free_locobj(&result_var);
        return raise_with_obj_and_args(prog->sys_var[CREATE_ERROR],
                                       proc_exec_object, arguments);
      } else if (unlikely((block =
          new_block(NULL, &result_var, local_vars, local_consts, block_body)) == NULL)) {
        logError(printf("prc_res_local: No memory\n"););
        free_expression(block_body);
        free_local_consts(local_consts);
        free_loclist(local_vars);
        free_locobj(&result_var);
        return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                       proc_exec_object, arguments);
      } else {
        logFunction(printf("prc_res_local -->\n"););
        return bld_block_temp(block);
      } /* if */
    } /* if */
  } /* prc_res_local */



objectType prc_return (listType arguments)

  {
    objectType block_body;
    objectType block_body_list = NULL;
    locObjRecord return_var;
    typeType return_type;
    objectType proc_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_return */
    logFunction(printf("prc_return\n"););
    block_body = arg_2(arguments);
    proc_exec_object = curr_exec_object;
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("prc_return: No memory\n"););
      return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                     proc_exec_object, arguments);
    } else {
      push_stack();
      if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
        update_owner(block_body);
        if (match_expression(block_body) == NULL) {
          free_expression(block_body);
          block_body = NULL;
        } /* if */
      } /* if */
      if (block_body != NULL) {
        block_body = match_object(block_body);
        fix_posinfo(block_body, block_body_list);
      } /* if */
      pop_stack();
#ifdef OUT_OF_ORDER
      printf("prc_return block_body=");
      trace1(block_body);
      printf("\n");
#endif
      if (block_body != NULL) {
        return_type = block_body->type_of;
        if (return_type->result_type != NULL) {
          return_type = return_type->result_type;
        } /* if */
      } else {
        return_type = NULL;
      } /* if */
#ifdef OUT_OF_ORDER
      printf("return_type=");
      trace1(return_type->match_obj);
      printf("\n");
#endif
      get_return_var(&return_var, return_type, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        logError(printf("prc_return: error - err_info: %d\n", err_info););
        free_expression(block_body);
        return raise_with_obj_and_args(prog->sys_var[err_info],
                                       proc_exec_object, arguments);
      } else if (unlikely(block_body == NULL)) {
        logError(printf("prc_return: Create error\n"););
        free_expression(block_body);
        return raise_with_obj_and_args(prog->sys_var[CREATE_ERROR],
                                       proc_exec_object, arguments);
      } else if (unlikely((block =
          new_block(NULL, &return_var, NULL, NULL, block_body)) == NULL)) {
        logError(printf("prc_return: No memory\n"););
        free_expression(block_body);
        return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                       proc_exec_object, arguments);
      } else {
        logFunction(printf("prc_return -->\n"););
        return bld_block_temp(block);
      } /* if */
    } /* if */
  } /* prc_return */



objectType prc_return2 (listType arguments)

  {
    objectType block_body;
    objectType block_body_list = NULL;
    locObjRecord return_var;
    typeType return_type;
    objectType proc_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_return2 */
    logFunction(printf("prc_return2\n"););
    block_body = arg_3(arguments);
    proc_exec_object = curr_exec_object;
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("prc_return2: No memory\n"););
      return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                     proc_exec_object, arguments);
    } else {
      push_stack();
      if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
        update_owner(block_body);
        if (match_expression(block_body) == NULL) {
          free_expression(block_body);
          block_body = NULL;
        } /* if */
      } /* if */
      if (block_body != NULL) {
        block_body = match_object(block_body);
        fix_posinfo(block_body, block_body_list);
      } /* if */
      pop_stack();
#ifdef OUT_OF_ORDER
      printf("prc_return2 block_body=");
      trace1(block_body);
      printf("\n");
#endif
      if (block_body != NULL) {
        return_type = block_body->type_of;
        if (return_type->result_type != NULL) {
          return_type = return_type->result_type;
        } /* if */
      } else {
        return_type = NULL;
      } /* if */
#ifdef OUT_OF_ORDER
      printf("return_type=");
      trace1(return_type->match_obj);
      printf("\n");
#endif
      get_return_var(&return_var, return_type, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        logError(printf("prc_return2: error - err_info: %d\n", err_info););
        free_expression(block_body);
        return raise_with_obj_and_args(prog->sys_var[err_info],
                                       proc_exec_object, arguments);
      } else if (unlikely(block_body == NULL)) {
        logError(printf("prc_return2: Create error\n"););
        free_expression(block_body);
        return raise_with_obj_and_args(prog->sys_var[CREATE_ERROR],
                                       proc_exec_object, arguments);
      } else if (unlikely((block =
          new_block(NULL, &return_var, NULL, NULL, block_body)) == NULL)) {
        logError(printf("prc_return2: No memory\n"););
        free_expression(block_body);
        return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                       proc_exec_object, arguments);
      } else {
        logFunction(printf("prc_return2 -->\n"););
        return bld_block_temp(block);
      } /* if */
    } /* if */
  } /* prc_return2 */



objectType prc_return_var (listType arguments)

  {
    objectType block_body;
    objectType block_body_list = NULL;
    objectType proc_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_return_var */
    logFunction(printf("prc_return_var\n"););
    block_body = arg_3(arguments);
    proc_exec_object = curr_exec_object;
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("prc_return_var: No memory\n"););
      return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                     proc_exec_object, arguments);
    } else {
      push_stack();
      if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
        update_owner(block_body);
        if (match_expression(block_body) == NULL) {
          free_expression(block_body);
          block_body = NULL;
        } /* if */
      } /* if */
      if (block_body != NULL) {
        block_body = match_object(block_body);
        fix_posinfo(block_body, block_body_list);
      } /* if */
      pop_stack();
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        logError(printf("prc_return_var: error - err_info: %d\n", err_info););
        free_expression(block_body);
        return raise_with_obj_and_args(prog->sys_var[err_info],
                                       proc_exec_object, arguments);
      } else if (unlikely(block_body == NULL)) {
        logError(printf("prc_return_var: Create error\n"););
        free_expression(block_body);
        return raise_with_obj_and_args(prog->sys_var[CREATE_ERROR],
                                       proc_exec_object, arguments);
      } else if (unlikely((block =
          new_block(NULL, NULL, NULL, NULL, block_body)) == NULL)) {
        logError(printf("prc_return_var: No memory\n"););
        free_expression(block_body);
        return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                       proc_exec_object, arguments);
      } else {
        logFunction(printf("prc_return_var -->\n"););
        return bld_block_temp(block);
      } /* if */
    } /* if */
  } /* prc_return_var */



objectType prc_return_var2 (listType arguments)

  {
    objectType block_body;
    objectType block_body_list = NULL;
    objectType proc_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_return_var2 */
    logFunction(printf("prc_return_var2\n"););
    block_body = arg_4(arguments);
    proc_exec_object = curr_exec_object;
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("prc_return_var2: No memory\n"););
      return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                     proc_exec_object, arguments);
    } else {
      push_stack();
      if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
        update_owner(block_body);
        if (match_expression(block_body) == NULL) {
          free_expression(block_body);
          block_body = NULL;
        } /* if */
      } /* if */
      if (block_body != NULL) {
        block_body = match_object(block_body);
        fix_posinfo(block_body, block_body_list);
      } /* if */
      pop_stack();
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        logError(printf("prc_return_var2: error - err_info: %d\n", err_info););
        free_expression(block_body);
        return raise_with_obj_and_args(prog->sys_var[err_info],
                                       proc_exec_object, arguments);
      } else if (unlikely(block_body == NULL)) {
        logError(printf("prc_return_var2: Create error\n"););
        free_expression(block_body);
        return raise_with_obj_and_args(prog->sys_var[CREATE_ERROR],
                                       proc_exec_object, arguments);
      } else if (unlikely((block =
          new_block(NULL, NULL, NULL, NULL, block_body)) == NULL)) {
        logError(printf("prc_return_var2: No memory\n"););
        free_expression(block_body);
        return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                       proc_exec_object, arguments);
      } else {
        logFunction(printf("prc_return_var2 -->\n"););
        return bld_block_temp(block);
      } /* if */
    } /* if */
  } /* prc_return_var2 */



objectType prc_semicolon (listType arguments)

  { /* prc_semicolone */
    return SYS_EMPTY_OBJECT;
  } /* prc_semicolone */



objectType prc_settrace (listType arguments)

  { /* prc_settrace */
    isit_stri(arg_1(arguments));
    mapTraceFlags(take_stri(arg_1(arguments)), &prog->option_flags);
    set_trace(prog->option_flags);
    return SYS_EMPTY_OBJECT;
  } /* prc_settrace */



objectType prc_trace (listType arguments)

  { /* prc_trace */
    while (arguments != NULL) {
      trace1(arguments->obj);
      prot_nl();
      arguments = arguments->next;
    } /* while */
    return SYS_EMPTY_OBJECT;
  } /* prc_trace */



objectType prc_while (listType arguments)

  {
    objectType condition;
    objectType statement;
    objectType cond_value;
    boolType cond;

  /* prc_while */
    condition = arg_2(arguments);
    statement = arg_4(arguments);
    cond_value = evaluate(condition);
    if (likely(!fail_flag)) {
      isit_bool(cond_value);
      cond = (boolType) (take_bool(cond_value) == SYS_TRUE_OBJECT);
      if (TEMP_OBJECT(cond_value) &&
          (CATEGORY_OF_OBJ(cond_value) == CONSTENUMOBJECT ||
           CATEGORY_OF_OBJ(cond_value) == VARENUMOBJECT)) {
        FREE_OBJECT(cond_value);
      } /* if */
      while (cond && !fail_flag) {
        evaluate(statement);
        if (likely(!fail_flag)) {
          cond_value = evaluate(condition);
          if (likely(!fail_flag)) {
            isit_bool(cond_value);
            cond = (boolType) (take_bool(cond_value) == SYS_TRUE_OBJECT);
            if (TEMP_OBJECT(cond_value) &&
                (CATEGORY_OF_OBJ(cond_value) == CONSTENUMOBJECT ||
                 CATEGORY_OF_OBJ(cond_value) == VARENUMOBJECT)) {
              FREE_OBJECT(cond_value);
            } /* if */
          } /* if */
        } /* if */
      } /* while */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_while */



objectType prc_while_noop (listType arguments)

  {
    objectType condition;
    objectType cond_value;
    boolType cond;

  /* prc_while_noop */
    condition = arg_2(arguments);
    do {
      cond_value = evaluate(condition);
      if (likely(!fail_flag)) {
        isit_bool(cond_value);
        cond = (boolType) (take_bool(cond_value) == SYS_TRUE_OBJECT);
        if (TEMP_OBJECT(cond_value) &&
            (CATEGORY_OF_OBJ(cond_value) == CONSTENUMOBJECT ||
             CATEGORY_OF_OBJ(cond_value) == VARENUMOBJECT)) {
          FREE_OBJECT(cond_value);
        } /* if */
      } /* if */
    } while (!fail_flag && cond);
    return SYS_EMPTY_OBJECT;
  } /* prc_while_noop */
