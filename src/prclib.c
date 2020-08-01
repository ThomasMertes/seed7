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



static void fix_posinfo (objectType block_body, const const_objectType block_body_list)

  { /* fix_posinfo */
    if (block_body != NULL && block_body_list != NULL &&
        CATEGORY_OF_OBJ(block_body) == CALLOBJECT &&
        !HAS_POSINFO(block_body)) {
      block_body->descriptor.posinfo = block_body_list->descriptor.posinfo;
      SET_POSINFO_FLAG(block_body);
    } /* if */
  } /* fix_posinfo */



/**
 *  Return the argument vector of the program as array of strings.
 *  The name of the program is not part of the argument vector.
 *  @return an array of strings containing the argument vector.
 */
objectType prc_args (listType arguments)

  { /* prc_args */
    return prog->arg_v;
  } /* prc_args */



objectType prc_begin (listType arguments)

  {
    objectType block_body;
    objectType block_body_list = NULL;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_begin */
    block_body = arg_3(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
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
    if (unlikely(err_info != OKAY_NO_ERROR ||
                 (block = new_block(NULL, NULL, NULL, NULL, block_body)) == NULL)) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_block_temp(block);
    } /* if */
  } /* prc_begin */



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
    if (unlikely(fail_flag)) {
      searching = TRUE;
      current_catch = arg_4(arguments);
      while (current_catch != NULL && searching &&
          CATEGORY_OF_OBJ(current_catch) == MATCHOBJECT &&
          current_catch->value.listValue->next->next->next->next != NULL) {
        catch_value = arg_3(current_catch->value.listValue);
        if (catch_value == fail_value) {
          catch_statement = arg_5(current_catch->value.listValue);
          set_fail_flag(FALSE);
          fail_value = NULL;
          free_list(fail_stack);
          fail_stack = NULL;
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
    if (unlikely(fail_flag)) {
      default_statement = arg_6(arguments);
      set_fail_flag(FALSE);
      fail_value = NULL;
      free_list(fail_stack);
      fail_stack = NULL;
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
    if (unlikely(fail_flag)) {
      searching = TRUE;
      current_catch = arg_4(arguments);
      while (current_catch != NULL && searching &&
          CATEGORY_OF_OBJ(current_catch) == MATCHOBJECT &&
          current_catch->value.listValue->next->next->next->next != NULL) {
        catch_value = arg_3(current_catch->value.listValue);
        if (catch_value == fail_value) {
          catch_statement = arg_5(current_catch->value.listValue);
          set_fail_flag(FALSE);
          fail_value = NULL;
          free_list(fail_stack);
          fail_stack = NULL;
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
        set_fail_flag(FALSE);
        fail_value = NULL;
        free_list(fail_stack);
        fail_stack = NULL;
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
        isit_set(when_set);
        set_value = take_set(when_set);
        if (TEMP_OBJECT(when_set)) {
          when_values->type_of = NULL;
          when_values->descriptor.property = NULL;
          SET_CATEGORY_OF_OBJ(when_values, SETOBJECT);
          when_values->value.setValue = set_value;
          current_when->value.listValue->next->next->obj = when_values;
        } /* if */
      } else {
        set_value = take_set(when_values);
      } /* if */
      if (setElem(switch_value, set_value)) {
        if (when_statement != NULL) {
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
        isit_set(when_set);
        set_value = take_set(when_set);
        if (TEMP_OBJECT(when_set)) {
          when_values->type_of = NULL;
          when_values->descriptor.property = NULL;
          SET_CATEGORY_OF_OBJ(when_values, SETOBJECT);
          when_values->value.setValue = set_value;
          current_when->value.listValue->next->next->obj = when_values;
        } /* if */
      } else {
        set_value = take_set(when_values);
      } /* if */
      if (setElem(switch_value, set_value)) {
        if (when_statement != NULL) {
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
        isit_hash(when_set);
        hashMap_value = take_hash(when_set);
        if (TEMP_OBJECT(when_set)) {
          when_values->type_of = when_set->type_of;
          when_values->descriptor.property = NULL;
          SET_CATEGORY_OF_OBJ(when_values, HASHOBJECT);
          when_values->value.hashValue = hashMap_value;
        } /* if */
      } /* if */
      if (do_in(switch_object, when_values, &err_info)) {
        if (unlikely(when_statement != NULL)) {
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
        isit_hash(when_set);
        hashMap_value = take_hash(when_set);
        if (TEMP_OBJECT(when_set)) {
          when_values->type_of = when_set->type_of;
          when_values->descriptor.property = NULL;
          SET_CATEGORY_OF_OBJ(when_values, HASHOBJECT);
          when_values->value.hashValue = hashMap_value;
        } /* if */
      } /* if */
      if (do_in(switch_object, when_values, &err_info)) {
        if (unlikely(when_statement != NULL)) {
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
    objectType block_value;
    errInfoType err_info = OKAY_NO_ERROR;

  /* prc_cpy */
    dest = arg_1(arguments);
    isit_proc(dest);
    is_variable(dest);
    isit_proc(arg_3(arguments));
    source = arg_3(arguments);
    /* printf("\nsrc: ");
    trace1(source);
    printf("\n"); */
    if (CATEGORY_OF_OBJ(source) == BLOCKOBJECT) {
      if (ALLOC_OBJECT(block_value)) {
        memcpy(block_value, source, sizeof(objectRecord));
        SET_CATEGORY_OF_OBJ(dest, MATCHOBJECT);
        dest->value.listValue = NULL;
        incl_list(&dest->value.listValue, block_value, &err_info);
      } else {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
    } else {
      SET_CATEGORY_OF_OBJ(dest, CATEGORY_OF_OBJ(source));
      dest->value = source->value;
    } /* if */
    /* printf("dst: ");
    trace1(dest);
    printf("\n"); */
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

  /* prc_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_proc(source);
    SET_CATEGORY_OF_OBJ(dest, CATEGORY_OF_OBJ(source));
    dest->value = source->value;
    if (TEMP_OBJECT(source)) {
      source->value.blockValue = NULL;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_create */



objectType prc_decls (listType arguments)

  { /* prc_decls */
    trace_nodes();
    return SYS_EMPTY_OBJECT;
  } /* prc_decls */



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
      shut_drivers();
      exit((int) status);
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
    if (lower_limit == INTTYPE_MIN) {
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
    if (upper_limit == INTTYPE_MAX) {
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
#if DO_HEAP_STATISTIC
    heap_statistic();
#else
    printf("heap statistic not supported - compile the s7 interpreter with DO_HEAP_STATISTIC\n");
#endif
    return SYS_EMPTY_OBJECT;
  } /* prc_heapstat */



objectType prc_hsize (listType arguments)

  { /* prc_hsize */
    /* heap_statistic(); */
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



objectType prc_include (listType arguments)

  {
    striType include_file_name;
    errInfoType err_info = OKAY_NO_ERROR;

  /* prc_include */
    isit_stri(arg_1(arguments));
    include_file_name = take_stri(arg_1(arguments));
    if (strChPos(include_file_name, (charType) '\\') != 0) {
      err_stri(WRONG_PATH_DELIMITER, include_file_name);
    } else {
      find_include_file(include_file_name, &err_info);
      if (err_info == MEMORY_ERROR) {
        err_warning(OUT_OF_HEAP_SPACE);
      } else if (unlikely(err_info != OKAY_NO_ERROR)) {
        /* FILE_ERROR or RANGE_ERROR */
        err_stri(FILENOTFOUND, include_file_name);
      } else {
        scan_byte_order_mark();
        scan_symbol();
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_include */



objectType prc_local (listType arguments)

  {
    objectType local_decls;
    objectType block_body;
    objectType block_body_list = NULL;
    listType *local_object_insert_place;
    locListType local_vars;
    listType local_consts;
    objectType decl_res;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_local */
    local_decls = arg_3(arguments);
    block_body = arg_5(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
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
    if (unlikely(err_info != OKAY_NO_ERROR ||
                 (block = new_block(NULL, NULL, local_vars, local_consts, block_body)) == NULL)) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_block_temp(block);
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
          if (TEMP_OBJECT(cond_value)) {
            dump_any_temp(cond_value);
          } /* if */
        } /* if */
      } /* if */
    } while (!fail_flag && cond);
    return SYS_EMPTY_OBJECT;
  } /* prc_repeat */



objectType prc_res_begin (listType arguments)

  {
    typeType result_type;
    objectType result_var_name;
    locObjRecord result_var;
    objectType result_init;
    objectType block_body;
    objectType block_body_list = NULL;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_res_begin */
    isit_type(arg_4(arguments));
    result_type = take_type(arg_4(arguments));
    result_var_name = arg_6(arguments);
    result_init = arg_8(arguments);
    block_body = arg_10(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
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
      result_var.object = entername(prog->declaration_root, result_var_name, &err_info);
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
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_res_local */
    isit_type(arg_4(arguments));
    result_type = take_type(arg_4(arguments));
    result_var_name = arg_6(arguments);
    result_init = arg_8(arguments);
    local_decls = arg_10(arguments);
    block_body = arg_12(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
    } /* if */
    block_body = copy_expression(block_body, &err_info);
    push_stack();
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      result_var.object = entername(prog->declaration_root, result_var_name, &err_info);
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



objectType prc_return (listType arguments)

  {
    objectType block_body;
    objectType block_body_list = NULL;
    locObjRecord return_var;
    typeType return_type;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_return */
    block_body = arg_2(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
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
    if (unlikely(err_info != OKAY_NO_ERROR ||
                 (block = new_block(NULL, &return_var, NULL, NULL, block_body)) == NULL)) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_block_temp(block);
    } /* if */
  } /* prc_return */



objectType prc_return2 (listType arguments)

  {
    objectType block_body;
    objectType block_body_list = NULL;
    locObjRecord return_var;
    typeType return_type;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_return2 */
    block_body = arg_3(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
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
    if (unlikely(err_info != OKAY_NO_ERROR ||
                 (block = new_block(NULL, &return_var, NULL, NULL, block_body)) == NULL)) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_block_temp(block);
    } /* if */
  } /* prc_return2 */



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



objectType prc_varfunc (listType arguments)

  {
    objectType block_body;
    objectType block_body_list = NULL;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_varfunc */
    block_body = arg_3(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
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
    if (unlikely(err_info != OKAY_NO_ERROR ||
                 (block = new_block(NULL, NULL, NULL, NULL, block_body)) == NULL)) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_block_temp(block);
    } /* if */
  } /* prc_varfunc */



objectType prc_varfunc2 (listType arguments)

  {
    objectType block_body;
    objectType block_body_list = NULL;
    errInfoType err_info = OKAY_NO_ERROR;
    blockType block;

  /* prc_varfunc2 */
    block_body = arg_4(arguments);
    if (CATEGORY_OF_OBJ(block_body) == EXPROBJECT &&
        block_body->value.listValue != NULL &&
        block_body->value.listValue->next == NULL) {
      block_body_list = block_body;
      block_body = block_body->value.listValue->obj;
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
    if (unlikely(err_info != OKAY_NO_ERROR ||
                 (block = new_block(NULL, NULL, NULL, NULL, block_body)) == NULL)) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return bld_block_temp(block);
    } /* if */
  } /* prc_varfunc2 */



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
      if (TEMP_OBJECT(cond_value)) {
        dump_any_temp(cond_value);
      } /* if */
      while (cond && !fail_flag) {
        evaluate(statement);
        if (likely(!fail_flag)) {
          cond_value = evaluate(condition);
          if (likely(!fail_flag)) {
            isit_bool(cond_value);
            cond = (boolType) (take_bool(cond_value) == SYS_TRUE_OBJECT);
            if (TEMP_OBJECT(cond_value)) {
              dump_any_temp(cond_value);
            } /* if */
          } /* if */
        } /* if */
      } /* while */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prc_while */
