/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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

#include "common.h"
#include "sigutl.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "striutl.h"
#include "entutl.h"
#include "syvarutl.h"
#include "traceutl.h"
#include "typeutl.h"
#include "listutl.h"
#include "executl.h"
#include "findid.h"
#include "match.h"
#include "name.h"
#include "exec.h"
#include "runerr.h"
#include "memory.h"
#include "blockutl.h"
#include "option.h"
#include "scanner.h"
#include "infile.h"
#include "error.h"
#include "set_rtl.h"

#undef EXTERN
#define EXTERN
#include "prclib.h"



#ifdef ANSI_C

static void fix_posinfo (objecttype block_body, objecttype block_body_list)
#else

static void fix_posinfo (block_body, block_body_list)
objecttype block_body;
objecttype block_body_list;
#endif

  { /* fix_posinfo */
    if (block_body_list != NULL &&
        CATEGORY_OF_OBJ(block_body) == CALLOBJECT &&
        !HAS_POSINFO(block_body)) {
      block_body->descriptor.posinfo = block_body_list->descriptor.posinfo;
      SET_POSINFO_FLAG(block_body);
    } /* if */
  } /* fix_posinfo */



#ifdef ANSI_C

static INLINE arraytype read_args (int arg_c, char **arg_v)
#else

static INLINE arraytype read_args (arg_c, arg_v)
int arg_c;
char **arg_v;
#endif

  {
    arraytype arg_array;
    arraytype resized_arg_array;
    memsizetype max_array_size;
    memsizetype used_array_size;
    memsizetype position;
    stritype str1;
    booltype okay;
    int arg_idx;

  /* read_args */
    max_array_size = 256;
    if (ALLOC_ARRAY(arg_array, max_array_size)) {
      used_array_size = 0;
      okay = TRUE;
      arg_idx = 0;
      while (okay && arg_idx < arg_c) {
        if (used_array_size >= max_array_size) {
          resized_arg_array = REALLOC_ARRAY(arg_array,
              max_array_size, max_array_size + 256);
          if (resized_arg_array == NULL) {
            okay = FALSE;
          } else {
            arg_array = resized_arg_array;
            COUNT3_ARRAY(max_array_size, max_array_size + 256);
            max_array_size = max_array_size + 256;
          } /* if */
        } /* if */
        if (okay) {
          str1 = cstri_to_stri(arg_v[arg_idx]);
          if (str1 == NULL) {
            okay = FALSE;
          } else {
            arg_array->arr[(int) used_array_size].type_of = take_type(SYS_STRI_TYPE);
            arg_array->arr[(int) used_array_size].descriptor.property = NULL;
            arg_array->arr[(int) used_array_size].value.strivalue = str1;
            INIT_CATEGORY_OF_VAR(&arg_array->arr[(int) used_array_size],
                STRIOBJECT);
            used_array_size++;
            arg_idx++;
          } /* if */
        } /* if */
      } /* while */
      if (okay) {
        resized_arg_array = REALLOC_ARRAY(arg_array,
            max_array_size, used_array_size);
        if (resized_arg_array == NULL) {
          okay = FALSE;
        } else {
          arg_array = resized_arg_array;
          COUNT3_ARRAY(max_array_size, used_array_size);
          arg_array->min_position = 1;
          arg_array->max_position = used_array_size;
        } /* if */
      } /* if */
      if (!okay) {
        for (position = 0; position < used_array_size; position++) {
          FREE_STRI(arg_array->arr[(int) position].value.strivalue,
              arg_array->arr[(int) position].value.strivalue->size);
        } /* for */
        FREE_ARRAY(arg_array, max_array_size);
        arg_array = NULL;
      } /* if */
    } /* if */
    return(arg_array);
  } /* read_args */



#ifdef ANSI_C

objecttype prc_args (listtype arguments)
#else

objecttype prc_args (arguments)
listtype arguments;
#endif

  {
    arraytype result;

  /* prc_args */
    if ((result = read_args(option.arg_c, option.arg_v)) == NULL) {
      return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
    } else {
      return(bld_array_temp(result));
    } /* if */
  } /* prc_args */



#ifdef ANSI_C

objecttype prc_begin (listtype arguments)
#else

objecttype prc_begin (arguments)
listtype arguments;
#endif

  {
    objecttype block_body;
    objecttype block_body_list = NULL;
    blocktype block;

  /* prc_begin */
    block_body = arg_3(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listvalue != NULL &&
        block_body->value.listvalue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listvalue->obj;
    } /* if */
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
    if ((block = new_block(NULL, NULL, NULL, NULL, block_body)) == NULL) {
      return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
    } else {
      return(bld_block_temp(block));
    } /* if */
  } /* prc_begin */



#ifdef ANSI_C

objecttype prc_block (listtype arguments)
#else

objecttype prc_block (arguments)
listtype arguments;
#endif

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
          emptylist(fail_stack);
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
    return(SYS_EMPTY_OBJECT);
  } /* prc_block */



#ifdef ANSI_C

objecttype prc_block_def (listtype arguments)
#else

objecttype prc_block_def (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* prc_block_def */



#ifdef ANSI_C

objecttype prc_case (listtype arguments)
#else

objecttype prc_case (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* prc_case */



#ifdef ANSI_C

objecttype prc_case_def (listtype arguments)
#else

objecttype prc_case_def (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* prc_case_def */



#ifdef ANSI_C

objecttype prc_cpy (listtype arguments)
#else

objecttype prc_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype proc_variable;

  /* prc_cpy */
    proc_variable = arg_1(arguments);
    isit_proc(proc_variable);
    is_variable(proc_variable);
    isit_proc(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(proc_variable, CATEGORY_OF_OBJ(arg_3(arguments)));
    proc_variable->value = arg_3(arguments)->value;
    return(SYS_EMPTY_OBJECT);
  } /* prc_cpy */



#ifdef ANSI_C

objecttype prc_create (listtype arguments)
#else

objecttype prc_create (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* prc_create */



#ifdef ANSI_C

objecttype prc_decls (listtype arguments)
#else

objecttype prc_decls (arguments)
listtype arguments;
#endif

  { /* prc_decls */
    trace_nodes();
    return(SYS_EMPTY_OBJECT);
  } /* prc_decls */



#ifdef ANSI_C

objecttype prc_dynamic (listtype arguments)
#else

objecttype prc_dynamic (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* prc_dynamic */
    result = exec_dynamic(arguments);
    return(result);
  } /* prc_dynamic */



#ifdef ANSI_C

objecttype prc_exit (listtype arguments)
#else

objecttype prc_exit (arguments)
listtype arguments;
#endif

  { /* prc_exit */
    shut_drivers();
    exit(1);
    return(SYS_EMPTY_OBJECT);
  } /* prc_exit */



#ifdef ANSI_C

objecttype prc_for_downto (listtype arguments)
#else

objecttype prc_for_downto (arguments)
listtype arguments;
#endif

  {
    objecttype for_variable;
    inttype lower_limit;
    inttype upper_limit;
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
      while (take_int(for_variable) > lower_limit) {
        for_variable->value.intvalue--;
        evaluate(statement);
      } /* while */
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* prc_for_downto */



#ifdef ANSI_C

objecttype prc_for_to (listtype arguments)
#else

objecttype prc_for_to (arguments)
listtype arguments;
#endif

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
      while (take_int(for_variable) < upper_limit) {
        for_variable->value.intvalue++;
        evaluate(statement);
      } /* while */
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* prc_for_to */



#ifdef ANSI_C

objecttype prc_heapstat (listtype arguments)
#else

objecttype prc_heapstat (arguments)
listtype arguments;
#endif

  { /* prc_heapstat */
#ifdef DO_HEAP_STATISTIC
    heap_statistic();
#else
    printf("heap statistic not supported - compile the hi interpreter with DO_HEAP_STATISTIC\n");
#endif
    return(SYS_EMPTY_OBJECT);
  } /* prc_heapstat */



#ifdef ANSI_C

objecttype prc_hsize (listtype arguments)
#else

objecttype prc_hsize (arguments)
listtype arguments;
#endif

  { /* prc_hsize */
    /* heap_statistic(); */
    return(bld_int_temp((inttype) heapsize()));
  } /* prc_hsize */



#ifdef ANSI_C

objecttype prc_if (listtype arguments)
#else

objecttype prc_if (arguments)
listtype arguments;
#endif

  {
    objecttype condition;

  /* prc_if */
    isit_bool(arg_2(arguments));
    condition = take_bool(arg_2(arguments));
    if (condition == SYS_TRUE_OBJECT) {
      evaluate(arg_4(arguments));
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* prc_if */



#ifdef ANSI_C

objecttype prc_if_elsif (listtype arguments)
#else

objecttype prc_if_elsif (arguments)
listtype arguments;
#endif

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
    return(SYS_EMPTY_OBJECT);
  } /* prc_if_elsif */



#ifdef ANSI_C

objecttype prc_include (listtype arguments)
#else

objecttype prc_include (arguments)
listtype arguments;
#endif

  {
    stritype include_file_name;
    errinfotype err_info = OKAY_NO_ERROR;

  /* prc_include */
    isit_stri(arg_1(arguments));
    include_file_name = take_stri(arg_1(arguments));
    if (stri_charpos(include_file_name, '\\') != NULL) {
      err_warning(WRONG_PATH_DELIMITER);
    } else {
      find_include_file(include_file_name, &err_info);
      if (err_info == FILE_ERROR) {
        err_stri(FILENOTFOUND, include_file_name);
      } else if (err_info != OKAY_NO_ERROR) {
        err_warning(OUT_OF_HEAP_SPACE);
      } else {
        scan_symbol();
      } /* if */
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* prc_include */



#ifdef ANSI_C

objecttype prc_local (listtype arguments)
#else

objecttype prc_local (arguments)
listtype arguments;
#endif

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
    if ((block = new_block(NULL, NULL, local_vars, local_consts, block_body)) == NULL) {
      return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
    } else {
      return(bld_block_temp(block));
    } /* if */
  } /* prc_local */



#ifdef ANSI_C

objecttype prc_noop (listtype arguments)
#else

objecttype prc_noop (arguments)
listtype arguments;
#endif

  { /* prc_noop */
    return(SYS_EMPTY_OBJECT);
  } /* prc_noop */



#ifdef ANSI_C

objecttype prc_raise (listtype arguments)
#else

objecttype prc_raise (arguments)
listtype arguments;
#endif

  { /* prc_raise */
    isit_enum(arg_2(arguments));
    return(raise_exception(take_enum(arg_2(arguments))));
  } /* prc_raise */



#ifdef ANSI_C

objecttype prc_repeat (listtype arguments)
#else

objecttype prc_repeat (arguments)
listtype arguments;
#endif

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
    } while (!fail_flag & cond);
    return(SYS_EMPTY_OBJECT);
  } /* prc_repeat */



#ifdef ANSI_C

objecttype prc_res_begin (listtype arguments)
#else

objecttype prc_res_begin (arguments)
listtype arguments;
#endif

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
        return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
      } else {
        return(bld_block_temp(block));
      } /* if */
    } /* if */
    return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
  } /* prc_res_begin */



#ifdef ANSI_C

objecttype prc_res_local (listtype arguments)
#else

objecttype prc_res_local (arguments)
listtype arguments;
#endif

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
        return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
      } else {
        return(bld_block_temp(block));
      } /* if */
    } /* if */
    return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
  } /* prc_res_local */



#ifdef ANSI_C

objecttype prc_return (listtype arguments)
#else

objecttype prc_return (arguments)
listtype arguments;
#endif

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
    if ((block = new_block(NULL, &return_var, NULL, NULL, block_body)) == NULL) {
      return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
    } else {
      return(bld_block_temp(block));
    } /* if */
  } /* prc_return */



#ifdef ANSI_C

objecttype prc_return2 (listtype arguments)
#else

objecttype prc_return2 (arguments)
listtype arguments;
#endif

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
    if ((block = new_block(NULL, &return_var, NULL, NULL, block_body)) == NULL) {
      return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
    } else {
      return(bld_block_temp(block));
    } /* if */
  } /* prc_return2 */



#ifdef ANSI_C

objecttype prc_settrace (listtype arguments)
#else

objecttype prc_settrace (arguments)
listtype arguments;
#endif

  { /* prc_settrace */
    isit_stri(arg_1(arguments));
    set_trace2(take_stri(arg_1(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* prc_settrace */



#ifdef ANSI_C

objecttype prc_trace (listtype arguments)
#else

objecttype prc_trace (arguments)
listtype arguments;
#endif

  { /* prc_trace */
    while (arguments != NULL) {
      trace1(arguments->obj);
      prot_nl();
      arguments = arguments->next;
    } /* while */
    return(SYS_EMPTY_OBJECT);
  } /* prc_trace */



#ifdef ANSI_C

objecttype prc_varfunc (listtype arguments)
#else

objecttype prc_varfunc (arguments)
listtype arguments;
#endif

  {
    objecttype block_body;
    objecttype block_body_list = NULL;
    blocktype block;

  /* prc_varfunc */
    block_body = arg_3(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listvalue != NULL &&
        block_body->value.listvalue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listvalue->obj;
    } /* if */
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
    if ((block = new_block(NULL, NULL, NULL, NULL, block_body)) == NULL) {
      return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
    } else {
      return(bld_block_temp(block));
    } /* if */
  } /* prc_varfunc */



#ifdef ANSI_C

objecttype prc_varfunc2 (listtype arguments)
#else

objecttype prc_varfunc2 (arguments)
listtype arguments;
#endif

  {
    objecttype block_body;
    objecttype block_body_list = NULL;
    blocktype block;

  /* prc_varfunc2 */
    block_body = arg_4(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listvalue != NULL &&
        block_body->value.listvalue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listvalue->obj;
    } /* if */
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
    if ((block = new_block(NULL, NULL, NULL, NULL, block_body)) == NULL) {
      return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
    } else {
      return(bld_block_temp(block));
    } /* if */
  } /* prc_varfunc2 */



#ifdef ANSI_C

objecttype prc_while (listtype arguments)
#else

objecttype prc_while (arguments)
listtype arguments;
#endif

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
      while (cond & !fail_flag) {
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
    return(SYS_EMPTY_OBJECT);
  } /* prc_while */
