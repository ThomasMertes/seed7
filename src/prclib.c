/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2007  Thomas Mertes                        */
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
/*  Changes: 1991, 1992, 1993, 1994, 2007  Thomas Mertes            */
/*  Content: Primitive actions to implement simple statements.      */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"

#include "common.h"
#include "sigutl.h"
#include "data.h"
#include "data_rtl.h"
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
#include "scanner.h"
#include "libpath.h"
#include "error.h"
#include "set_rtl.h"
#include "str_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "prclib.h"



static void fix_posinfo (objecttype block_body, const const_objecttype block_body_list)

  { /* fix_posinfo */
    if (block_body_list != NULL &&
        CATEGORY_OF_OBJ(block_body) == CALLOBJECT &&
        !HAS_POSINFO(block_body)) {
      block_body->descriptor.posinfo = block_body_list->descriptor.posinfo;
      SET_POSINFO_FLAG(block_body);
    } /* if */
  } /* fix_posinfo */



objecttype prc_args (listtype arguments)

  { /* prc_args */
    return prog.arg_v;
  } /* prc_args */



objecttype prc_begin (listtype arguments)

  {
    objecttype block_body;
    objecttype block_body_list = NULL;
    errinfotype err_info = OKAY_NO_ERROR;
    blocktype block;

  /* prc_begin */
    block_body = arg_3(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listvalue != NULL &&
        block_body->value.listvalue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listvalue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    push_stack();
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
      update_owner(block_body);
      block_body = match_expression(block_body);
    } /* if */
    if (block_body != NULL) {
      block_body = match_object(block_body);
      fix_posinfo(block_body, block_body_list);
    } /* if */
    pop_stack();
    if (block_body != NULL && block_body->type_of != take_type(SYS_PROC_TYPE)) {
      err_type(PROC_EXPECTED, block_body->type_of);
    } /* if */
    if (err_info != OKAY_NO_ERROR ||
        (block = new_block(NULL, NULL, NULL, NULL, block_body)) == NULL) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_block_temp(block);
    } /* if */
  } /* prc_begin */



objecttype prc_block (listtype arguments)

  {
    objecttype statement;
    objecttype current_catch;
    objecttype catch_value;
    objecttype catch_statement;
    booltype searching;

  /* prc_block */
    statement = arg_2(arguments);
    evaluate(statement);
    if (fail_flag) {
      searching = TRUE;
      current_catch = arg_4(arguments);
      while (current_catch != NULL && searching &&
          CATEGORY_OF_OBJ(current_catch) == MATCHOBJECT &&
          current_catch->value.listvalue->next->next->next->next != NULL) {
        catch_value = arg_3(current_catch->value.listvalue);
        if (catch_value == fail_value) {
          catch_statement = arg_5(current_catch->value.listvalue);
          fail_flag = FALSE;
          fail_value = NULL;
          free_list(fail_stack);
          fail_stack = NULL;
          evaluate(catch_statement);
          searching = FALSE;
        } else {
          if (current_catch->value.listvalue->next->next->next->next->next != NULL) {
            current_catch = arg_6(current_catch->value.listvalue);
          } else {
            current_catch = NULL;
          } /* if */
        } /* if */
      } /* while */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_block */



objecttype prc_block_def (listtype arguments)

  {
    objecttype statement;
    objecttype default_statement;
    objecttype current_catch;
    objecttype catch_value;
    objecttype catch_statement;
    booltype searching;

  /* prc_block_def */
    statement = arg_2(arguments);
    evaluate(statement);
    if (fail_flag) {
      searching = TRUE;
      current_catch = arg_4(arguments);
      while (current_catch != NULL && searching &&
          CATEGORY_OF_OBJ(current_catch) == MATCHOBJECT &&
          current_catch->value.listvalue->next->next->next->next != NULL) {
        catch_value = arg_3(current_catch->value.listvalue);
        if (catch_value == fail_value) {
          catch_statement = arg_5(current_catch->value.listvalue);
          fail_flag = FALSE;
          fail_value = NULL;
          evaluate(catch_statement);
          searching = FALSE;
        } else {
          if (current_catch->value.listvalue->next->next->next->next->next != NULL) {
            current_catch = arg_6(current_catch->value.listvalue);
          } else {
            current_catch = NULL;
          } /* if */
        } /* if */
      } /* while */
      if (searching) {
        default_statement = arg_7(arguments);
        fail_flag = FALSE;
        fail_value = NULL;
        evaluate(default_statement);
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_block_def */



objecttype prc_case (listtype arguments)

  {
    objecttype switch_object;
    inttype switch_value;
    objecttype when_objects;
    objecttype current_when;
    objecttype when_values;
    objecttype when_set;
    settype set_value;
    objecttype when_statement;
    errinfotype err_info = OKAY_NO_ERROR;
    booltype searching;

  /* prc_case */
    searching = TRUE;
    switch_object = arg_2(arguments);
    when_objects = arg_4(arguments);
    current_when = when_objects;
    switch_value = do_ord(switch_object, &err_info);
    while (searching && current_when != NULL &&
        CATEGORY_OF_OBJ(current_when) == MATCHOBJECT &&
        current_when->value.listvalue->next->next->next->next != NULL) {
      when_values = arg_3(current_when->value.listvalue);
      if (CATEGORY_OF_OBJ(when_values) != SETOBJECT) {
        when_set = exec_object(when_values);
        isit_set(when_set);
        set_value = take_set(when_set);
        if (TEMP_OBJECT(when_set)) {
          when_values->type_of = NULL;
          when_values->descriptor.property = NULL;
          SET_CATEGORY_OF_OBJ(when_values, SETOBJECT);
          when_values->value.setvalue = set_value;
        } /* if */
      } else {
        set_value = take_set(when_values);
      } /* if */
      if (setElem(switch_value, set_value)) {
        when_statement = arg_5(current_when->value.listvalue);
        evaluate(when_statement);
        searching = FALSE;
      } else {
        if (current_when->value.listvalue->next->next->next->next->next != NULL) {
          current_when = arg_6(current_when->value.listvalue);
        } else {
          current_when = NULL;
        } /* if */
      } /* if */
    } /* while */
    return SYS_EMPTY_OBJECT;
  } /* prc_case */



objecttype prc_case_def (listtype arguments)

  {
    objecttype switch_object;
    inttype switch_value;
    objecttype when_objects;
    objecttype default_statement;
    objecttype current_when;
    objecttype when_values;
    objecttype when_set;
    settype set_value;
    objecttype when_statement;
    errinfotype err_info = OKAY_NO_ERROR;
    booltype searching;

  /* prc_case_def */
    searching = TRUE;
    switch_object = arg_2(arguments);
    when_objects = arg_4(arguments);
    current_when = when_objects;
    switch_value = do_ord(switch_object, &err_info);
    while (searching && current_when != NULL &&
        CATEGORY_OF_OBJ(current_when) == MATCHOBJECT &&
        current_when->value.listvalue->next->next->next->next != NULL) {
      when_values = arg_3(current_when->value.listvalue);
      if (CATEGORY_OF_OBJ(when_values) != SETOBJECT) {
        when_set = exec_object(when_values);
        isit_set(when_set);
        set_value = take_set(when_set);
        if (TEMP_OBJECT(when_set)) {
          when_values->type_of = NULL;
          when_values->descriptor.property = NULL;
          SET_CATEGORY_OF_OBJ(when_values, SETOBJECT);
          when_values->value.setvalue = set_value;
        } /* if */
      } else {
        set_value = take_set(when_values);
      } /* if */
      if (setElem(switch_value, set_value)) {
        when_statement = arg_5(current_when->value.listvalue);
        evaluate(when_statement);
        searching = FALSE;
      } else {
        if (current_when->value.listvalue->next->next->next->next->next != NULL) {
          current_when = arg_6(current_when->value.listvalue);
        } else {
          current_when = NULL;
        } /* if */
      } /* if */
    } /* while */
    if (searching) {
      default_statement = arg_7(arguments);
      evaluate(default_statement);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_case_def */



objecttype prc_cpy (listtype arguments)

  {
    objecttype proc_variable;
    objecttype source_value;
    objecttype block_value;
    errinfotype err_info = OKAY_NO_ERROR;

  /* prc_cpy */
    proc_variable = arg_1(arguments);
    isit_proc(proc_variable);
    is_variable(proc_variable);
    isit_proc(arg_3(arguments));
    source_value = arg_3(arguments);
    /* printf("\nsrc: ");
    trace1(source_value);
    printf("\n"); */
    if (CATEGORY_OF_OBJ(source_value) == BLOCKOBJECT) {
      if (ALLOC_OBJECT(block_value)) {
        memcpy(block_value, source_value, sizeof(struct objectstruct));
        SET_CATEGORY_OF_OBJ(proc_variable, MATCHOBJECT);
        proc_variable->value.listvalue = NULL;
        incl_list(&proc_variable->value.listvalue, block_value, &err_info);
      } else {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
    } else {
      SET_CATEGORY_OF_OBJ(proc_variable, CATEGORY_OF_OBJ(source_value));
      proc_variable->value = source_value->value;
    } /* if */
    /* printf("dst: ");
    trace1(proc_variable);
    printf("\n"); */
    return SYS_EMPTY_OBJECT;
  } /* prc_cpy */



objecttype prc_create (listtype arguments)

  {
    objecttype proc_to;
    objecttype proc_from;

  /* prc_create */
    proc_to = arg_1(arguments);
    proc_from = arg_3(arguments);
    isit_proc(proc_from);
    SET_CATEGORY_OF_OBJ(proc_to, CATEGORY_OF_OBJ(proc_from));
    proc_to->value = proc_from->value;
    if (TEMP_OBJECT(proc_from)) {
      proc_from->value.blockvalue = NULL;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_create */



objecttype prc_decls (listtype arguments)

  { /* prc_decls */
    trace_nodes();
    return SYS_EMPTY_OBJECT;
  } /* prc_decls */



objecttype prc_dynamic (listtype arguments)

  {
    objecttype result;

  /* prc_dynamic */
    result = exec_dynamic(arguments);
    return result;
  } /* prc_dynamic */



objecttype prc_exit (listtype arguments)

  {
    inttype status;

  /* prc_exit */
    isit_int(arg_1(arguments));
    status = take_int(arg_1(arguments));
    if (!inIntRange(status)) {
      raise_error(RANGE_ERROR);
    } else {
      shut_drivers();
      exit((int) status);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_exit */



objecttype prc_for_downto (listtype arguments)

  {
    objecttype for_variable;
    inttype upper_limit;
    inttype lower_limit;
    objecttype statement;

  /* prc_for_downto */
    for_variable = arg_2(arguments);
    is_variable(for_variable);
    isit_int(for_variable);
    isit_int(arg_4(arguments));
    isit_int(arg_6(arguments));
    upper_limit = take_int(arg_4(arguments));
    lower_limit = take_int(arg_6(arguments));
    statement = arg_8(arguments);
    if (upper_limit >= lower_limit) {
      for_variable->value.intvalue = upper_limit;
      evaluate(statement);
      while (take_int(for_variable) > lower_limit && !fail_flag) {
        for_variable->value.intvalue--;
        evaluate(statement);
      } /* while */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_for_downto */



objecttype prc_for_to (listtype arguments)

  {
    objecttype for_variable;
    inttype lower_limit;
    inttype upper_limit;
    objecttype statement;

  /* prc_for_to */
    for_variable = arg_2(arguments);
    is_variable(for_variable);
    isit_int(for_variable);
    isit_int(arg_4(arguments));
    isit_int(arg_6(arguments));
    lower_limit = take_int(arg_4(arguments));
    upper_limit = take_int(arg_6(arguments));
    statement = arg_8(arguments);
    if (lower_limit <= upper_limit) {
      for_variable->value.intvalue = lower_limit;
      evaluate(statement);
      while (take_int(for_variable) < upper_limit && !fail_flag) {
        for_variable->value.intvalue++;
        evaluate(statement);
      } /* while */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_for_to */



objecttype prc_heapstat (listtype arguments)

  { /* prc_heapstat */
#ifdef DO_HEAP_STATISTIC
    heap_statistic();
#else
    printf("heap statistic not supported - compile the s7 interpreter with DO_HEAP_STATISTIC\n");
#endif
    return SYS_EMPTY_OBJECT;
  } /* prc_heapstat */



objecttype prc_hsize (listtype arguments)

  { /* prc_hsize */
    /* heap_statistic(); */
    return bld_int_temp((inttype) heapsize());
  } /* prc_hsize */



objecttype prc_if (listtype arguments)

  {
    objecttype condition;

  /* prc_if */
    isit_bool(arg_2(arguments));
    condition = take_bool(arg_2(arguments));
    if (condition == SYS_TRUE_OBJECT) {
      evaluate(arg_4(arguments));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_if */



objecttype prc_if_elsif (listtype arguments)

  {
    objecttype condition;

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



objecttype prc_include (listtype arguments)

  {
    stritype include_file_name;
    errinfotype err_info = OKAY_NO_ERROR;

  /* prc_include */
    isit_stri(arg_1(arguments));
    include_file_name = take_stri(arg_1(arguments));
    if (strChPos(include_file_name, (chartype) '\\') != 0) {
      err_stri(WRONG_PATH_DELIMITER, include_file_name);
    } else {
      find_include_file(include_file_name, &err_info);
      if (err_info == MEMORY_ERROR) {
        err_warning(OUT_OF_HEAP_SPACE);
      } else if (err_info != OKAY_NO_ERROR) {
        /* FILE_ERROR or RANGE_ERROR */
        err_stri(FILENOTFOUND, include_file_name);
      } else {
        scan_byte_order_mark();
        scan_symbol();
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_include */



objecttype prc_local (listtype arguments)

  {
    objecttype local_decls;
    objecttype block_body;
    objecttype block_body_list = NULL;
    listtype *local_object_insert_place;
    loclisttype local_vars;
    listtype local_consts;
    objecttype decl_res;
    errinfotype err_info = OKAY_NO_ERROR;
    blocktype block;

  /* prc_local */
    local_decls = arg_3(arguments);
    block_body = arg_5(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listvalue != NULL &&
        block_body->value.listvalue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listvalue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
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
    local_vars = get_local_var_list(*local_object_insert_place, &err_info);
    local_consts = get_local_const_list(*local_object_insert_place, &err_info);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
      update_owner(block_body);
      block_body = match_expression(block_body);
    } /* if */
    if (block_body != NULL) {
      block_body = match_object(block_body);
      fix_posinfo(block_body, block_body_list);
    } /* if */
    pop_stack();
    if (block_body != NULL && block_body->type_of != take_type(SYS_PROC_TYPE)) {
      err_type(PROC_EXPECTED, block_body->type_of);
    } /* if */
    if (err_info != OKAY_NO_ERROR ||
        (block = new_block(NULL, NULL, local_vars, local_consts, block_body)) == NULL) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_block_temp(block);
    } /* if */
  } /* prc_local */



objecttype prc_noop (listtype arguments)

  { /* prc_noop */
    return SYS_EMPTY_OBJECT;
  } /* prc_noop */



objecttype prc_raise (listtype arguments)

  { /* prc_raise */
    isit_enum(arg_2(arguments));
    return raise_exception(take_enum(arg_2(arguments)));
  } /* prc_raise */



objecttype prc_repeat (listtype arguments)

  {
    objecttype statement;
    objecttype condition;
    objecttype cond_value;
    booltype cond;

  /* prc_repeat */
    statement = arg_2(arguments);
    condition = arg_4(arguments);
    do {
      evaluate(statement);
      if (!fail_flag) {
        cond_value = evaluate(condition);
        if (!fail_flag) {
          isit_bool(cond_value);
          cond = (booltype) (take_bool(cond_value) == SYS_FALSE_OBJECT);
          if (TEMP_OBJECT(cond_value)) {
            dump_any_temp(cond_value);
          } /* if */
        } /* if */
      } /* if */
    } while (!fail_flag && cond);
    return SYS_EMPTY_OBJECT;
  } /* prc_repeat */



objecttype prc_res_begin (listtype arguments)

  {
    typetype result_type;
    objecttype result_var_name;
    locobjrecord result_var;
    objecttype result_init;
    objecttype block_body;
    objecttype block_body_list = NULL;
    errinfotype err_info = OKAY_NO_ERROR;
    blocktype block;

  /* prc_res_begin */
    isit_type(arg_4(arguments));
    result_type = take_type(arg_4(arguments));
    result_var_name = arg_6(arguments);
    result_init = arg_8(arguments);
    block_body = arg_10(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listvalue != NULL &&
        block_body->value.listvalue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listvalue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    push_stack();
/*    printf("result_type ");
    trace1(result_type->match_obj);
    printf("\n");
    printf("result_var_name ");
    trace1(result_var_name);
    printf("\n"); */
    /* printf("result_init %lu ", (long unsigned) result_init);
    trace1(result_init);
    printf("\n"); */
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      result_var.object = entername(prog.declaration_root, result_var_name, &err_info);
      shrink_stack();
    } /* if */
    if (err_info == OKAY_NO_ERROR) {
      get_result_var(&result_var, result_type, result_init, &err_info);
/*      printf("result_var.object ");
      trace1(result_var.object);
      printf("\n");
      printf("result_var.init_value ");
      trace1(result_var.init_value);
      printf("\n"); */
      if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
        update_owner(block_body);
        block_body = match_expression(block_body);
      } /* if */
      if (block_body != NULL) {
        block_body = match_object(block_body);
        fix_posinfo(block_body, block_body_list);
      } /* if */
      pop_stack();
      if (block_body != NULL && block_body->type_of != take_type(SYS_PROC_TYPE)) {
        err_type(PROC_EXPECTED, block_body->type_of);
      } /* if */
      if ((block = new_block(NULL, &result_var, NULL, NULL, block_body)) == NULL) {
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } else {
        return bld_block_temp(block);
      } /* if */
    } /* if */
    return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
  } /* prc_res_begin */



objecttype prc_res_local (listtype arguments)

  {
    typetype result_type;
    objecttype result_var_name;
    locobjrecord result_var;
    objecttype result_init;
    objecttype local_decls;
    objecttype block_body;
    objecttype block_body_list = NULL;
    listtype *local_object_insert_place;
    loclisttype local_vars;
    listtype local_consts;
    objecttype decl_res;
    errinfotype err_info = OKAY_NO_ERROR;
    blocktype block;

  /* prc_res_local */
    isit_type(arg_4(arguments));
    result_type = take_type(arg_4(arguments));
    result_var_name = arg_6(arguments);
    result_init = arg_8(arguments);
    local_decls = arg_10(arguments);
    block_body = arg_12(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listvalue != NULL &&
        block_body->value.listvalue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listvalue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    push_stack();
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      result_var.object = entername(prog.declaration_root, result_var_name, &err_info);
      shrink_stack();
    } /* if */
    if (err_info == OKAY_NO_ERROR) {
      get_result_var(&result_var, result_type, result_init, &err_info);
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
      local_vars = get_local_var_list(*local_object_insert_place, &err_info);
      local_consts = get_local_const_list(*local_object_insert_place, &err_info);
      if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
        update_owner(block_body);
        block_body = match_expression(block_body);
      } /* if */
      if (block_body != NULL) {
        block_body = match_object(block_body);
        fix_posinfo(block_body, block_body_list);
      } /* if */
      pop_stack();
      if (block_body != NULL && block_body->type_of != take_type(SYS_PROC_TYPE)) {
        err_type(PROC_EXPECTED, block_body->type_of);
      } /* if */
      if ((block = new_block(NULL, &result_var, local_vars, local_consts, block_body)) == NULL) {
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } else {
        return bld_block_temp(block);
      } /* if */
    } /* if */
    return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
  } /* prc_res_local */



objecttype prc_return (listtype arguments)

  {
    objecttype block_body;
    objecttype block_body_list = NULL;
    locobjrecord return_var;
    typetype return_type;
    errinfotype err_info = OKAY_NO_ERROR;
    blocktype block;

  /* prc_return */
    block_body = arg_2(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listvalue != NULL &&
        block_body->value.listvalue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listvalue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    push_stack();
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
      update_owner(block_body);
      block_body = match_expression(block_body);
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
    return_type = block_body->type_of;
    if (return_type->result_type != NULL) {
      return_type = return_type->result_type;
    } /* if */
#ifdef OUT_OF_ORDER
    printf("return_type=");
    trace1(return_type->match_obj);
    printf("\n");
#endif
    get_return_var(&return_var, return_type, &err_info);
    if (err_info != OKAY_NO_ERROR ||
        (block = new_block(NULL, &return_var, NULL, NULL, block_body)) == NULL) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_block_temp(block);
    } /* if */
  } /* prc_return */



objecttype prc_return2 (listtype arguments)

  {
    objecttype block_body;
    objecttype block_body_list = NULL;
    locobjrecord return_var;
    typetype return_type;
    errinfotype err_info = OKAY_NO_ERROR;
    blocktype block;

  /* prc_return2 */
    block_body = arg_3(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listvalue != NULL &&
        block_body->value.listvalue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listvalue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    push_stack();
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
      update_owner(block_body);
      block_body = match_expression(block_body);
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
    return_type = block_body->type_of;
    if (return_type->result_type != NULL) {
      return_type = return_type->result_type;
    } /* if */
#ifdef OUT_OF_ORDER
    printf("return_type=");
    trace1(return_type->match_obj);
    printf("\n");
#endif
    get_return_var(&return_var, return_type, &err_info);
    if (err_info != OKAY_NO_ERROR ||
        (block = new_block(NULL, &return_var, NULL, NULL, block_body)) == NULL) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_block_temp(block);
    } /* if */
  } /* prc_return2 */



objecttype prc_settrace (listtype arguments)

  { /* prc_settrace */
    isit_stri(arg_1(arguments));
    mapTraceFlags(take_stri(arg_1(arguments)), &prog.option_flags);
    set_trace(prog.option_flags);
    return SYS_EMPTY_OBJECT;
  } /* prc_settrace */



objecttype prc_trace (listtype arguments)

  { /* prc_trace */
    while (arguments != NULL) {
      trace1(arguments->obj);
      prot_nl();
      arguments = arguments->next;
    } /* while */
    return SYS_EMPTY_OBJECT;
  } /* prc_trace */



objecttype prc_varfunc (listtype arguments)

  {
    objecttype block_body;
    objecttype block_body_list = NULL;
    errinfotype err_info = OKAY_NO_ERROR;
    blocktype block;

  /* prc_varfunc */
    block_body = arg_3(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listvalue != NULL &&
        block_body->value.listvalue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listvalue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    push_stack();
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
      update_owner(block_body);
      block_body = match_expression(block_body);
    } /* if */
    if (block_body != NULL) {
      block_body = match_object(block_body);
      fix_posinfo(block_body, block_body_list);
    } /* if */
    pop_stack();
    if (err_info != OKAY_NO_ERROR ||
        (block = new_block(NULL, NULL, NULL, NULL, block_body)) == NULL) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_block_temp(block);
    } /* if */
  } /* prc_varfunc */



objecttype prc_varfunc2 (listtype arguments)

  {
    objecttype block_body;
    objecttype block_body_list = NULL;
    errinfotype err_info = OKAY_NO_ERROR;
    blocktype block;

  /* prc_varfunc2 */
    block_body = arg_4(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listvalue != NULL &&
        block_body->value.listvalue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listvalue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    push_stack();
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT) {
      update_owner(block_body);
      block_body = match_expression(block_body);
    } /* if */
    if (block_body != NULL) {
      block_body = match_object(block_body);
      fix_posinfo(block_body, block_body_list);
    } /* if */
    pop_stack();
    if (err_info != OKAY_NO_ERROR ||
        (block = new_block(NULL, NULL, NULL, NULL, block_body)) == NULL) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_block_temp(block);
    } /* if */
  } /* prc_varfunc2 */



objecttype prc_while (listtype arguments)

  {
    objecttype condition;
    objecttype statement;
    objecttype cond_value;
    booltype cond;

  /* prc_while */
    condition = arg_2(arguments);
    statement = arg_4(arguments);
    cond_value = evaluate(condition);
    if (!fail_flag) {
      isit_bool(cond_value);
      cond = (booltype) (take_bool(cond_value) == SYS_TRUE_OBJECT);
      if (TEMP_OBJECT(cond_value)) {
        dump_any_temp(cond_value);
      } /* if */
      while (cond && !fail_flag) {
        evaluate(statement);
        if (!fail_flag) {
          cond_value = evaluate(condition);
          if (!fail_flag) {
            isit_bool(cond_value);
            cond = (booltype) (take_bool(cond_value) == SYS_TRUE_OBJECT);
            if (TEMP_OBJECT(cond_value)) {
              dump_any_temp(cond_value);
            } /* if */
          } /* if */
        } /* if */
      } /* while */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_while */
