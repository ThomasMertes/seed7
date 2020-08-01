/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  File: seed7/src/reflib.c                                        */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: All primitive actions for the program type.            */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "identutl.h"
#include "listutl.h"
#include "entutl.h"
#include "findid.h"
#include "traceutl.h"
#include "analyze.h"
#include "exec.h"
#include "runerr.h"
#include "memory.h"
#include "match.h"
#include "name.h"
#include "option.h"

#undef EXTERN
#define EXTERN
#include "prglib.h"


#define TABLE_INCREMENT 127



#ifdef ANSI_C

objecttype prg_cpy (listtype arguments)
#else

objecttype prg_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype prog_to;
    objecttype prog_from;
    progtype prog_value;

  /* prg_cpy */
    prog_to = arg_1(arguments);
    prog_from = arg_3(arguments);
    isit_prog(prog_to);
    isit_prog(prog_from);
    is_variable(prog_to);
    prog_value = take_prog(prog_to);
    if (prog_value != NULL) {
      prog_value->usage_count--;
      if (prog_value->usage_count == 0) {
        FREE_RECORD(prog_value, progrecord, count.prog);
      } /* if */
    } /* if */
    prog_value = take_prog(prog_from);
    prog_to->value.progvalue = prog_value;
    if (TEMP_OBJECT(prog_from)) {
      prog_from->value.progvalue = NULL;
    } else {
      if (prog_value != NULL) {
        prog_value->usage_count++;
      } /* if */
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* prg_cpy */



#ifdef ANSI_C

objecttype prg_create (listtype arguments)
#else

objecttype prg_create (arguments)
listtype arguments;
#endif

  {
    objecttype prog_from;
    progtype prog_value;

  /* prg_create */
    prog_from = arg_3(arguments);
    isit_prog(prog_from);
    SET_CATEGORY_OF_OBJ(arg_1(arguments), PROGOBJECT);
    prog_value = take_prog(prog_from);
    arg_1(arguments)->value.progvalue = prog_value;
    if (TEMP_OBJECT(prog_from)) {
      prog_from->value.progvalue = NULL;
    } else {
      if (prog_value != NULL) {
        prog_value->usage_count++;
      } /* if */
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* prg_create */



#ifdef ANSI_C

objecttype prg_decl_objects (listtype arguments)
#else

objecttype prg_decl_objects (arguments)
listtype arguments;
#endif

  {
    progtype currentProg;
    listtype object_list;
    errinfotype err_info = OKAY_NO_ERROR;
    objecttype result;

  /* prg_decl_objects */
    isit_prog(arg_1(arguments));
    currentProg = take_prog(arg_1(arguments));
    if (currentProg->stack_current != NULL) {
      object_list = copy_list(currentProg->stack_current->local_object_list, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } /* if */
    } else {
      object_list = NULL;
    } /* if */
    result = bld_reflist_temp(object_list);
    return(result);
  } /* prg_decl_objects */



#ifdef ANSI_C

objecttype prg_destr (listtype arguments)
#else

objecttype prg_destr (arguments)
listtype arguments;
#endif

  {
    progtype old_prog;

  /* prg_destr */
    old_prog = take_prog(arg_1(arguments));
    if (old_prog != NULL) {
      old_prog->usage_count--;
      if (old_prog->usage_count == 0) {
        close_stack(old_prog);
        close_declaration_root(old_prog);
        close_idents(old_prog);
        FREE_RECORD(old_prog, progrecord, count.prog);
      } /* if */
      arg_1(arguments)->value.progvalue = NULL;
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* prg_destr */



#ifdef ANSI_C

objecttype prg_empty (listtype arguments)
#else

objecttype prg_empty (arguments)
listtype arguments;
#endif

  { /* prg_empty */
    return(bld_prog_temp(NULL));
  } /* prg_empty */



#ifdef ANSI_C

objecttype prg_eq (listtype arguments)
#else

objecttype prg_eq (arguments)
listtype arguments;
#endif

  { /* prg_eq */
    isit_prog(arg_1(arguments));
    isit_prog(arg_3(arguments));
    if (take_prog(arg_1(arguments)) ==
        take_prog(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* prg_eq */



#ifdef ANSI_C

objecttype prg_error_count (listtype arguments)
#else

objecttype prg_error_count (arguments)
listtype arguments;
#endif

  { /* prg_error_count */
    isit_prog(arg_1(arguments));
    return(bld_int_temp((inttype) take_prog(arg_1(arguments))->error_count));
  } /* prg_error_count */



#ifdef ANSI_C

objecttype prg_eval (listtype arguments)
#else

objecttype prg_eval (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* prg_eval */
    isit_prog(arg_1(arguments));
    isit_reference(arg_2(arguments));
    result = exec_expr(take_prog(arg_1(arguments)), take_reference(arg_2(arguments)));
    fail_flag = FALSE;
    fail_value = (objecttype) NULL;
    fail_expression = (listtype) NULL;
    return(bld_reference_temp(result));
  } /* prg_eval */



#ifdef ANSI_C

objecttype prg_exec (listtype arguments)
#else

objecttype prg_exec (arguments)
listtype arguments;
#endif

  { /* prg_exec */
    isit_prog(arg_1(arguments));
    interpr(take_prog(arg_1(arguments)));
    fail_flag = FALSE;
    fail_value = (objecttype) NULL;
    fail_expression = (listtype) NULL;
    return(SYS_EMPTY_OBJECT);
  } /* prg_exec */



#ifdef ANSI_C

objecttype prg_fil_parse (listtype arguments)
#else

objecttype prg_fil_parse (arguments)
listtype arguments;
#endif

  {
    stritype str1;
    cstritype file_name;
    progtype result;

  /* prg_fil_parse */
    isit_stri(arg_1(arguments));
    str1 = take_stri(arg_1(arguments));
    file_name = cp_to_cstri(str1);
    if (file_name == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      result = analyze((ustritype) file_name);
      set_trace(option.exec_trace_level, -1, NULL);
      return(bld_prog_temp(result));
    } /* if */
  } /* prg_fil_parse */



#ifdef ANSI_C

objecttype prg_find (listtype arguments)
#else

objecttype prg_find (arguments)
listtype arguments;
#endif

  {
    objecttype module_object;
    objecttype result;

  /* prg_find */
    isit_reference(arg_1(arguments));
    isit_reference(arg_3(arguments));
    module_object = take_reference(arg_1(arguments));
/*  trace2(module_object); */
#ifdef OUT_OF_ORDER
    if (CATEGORY_OF_OBJ(module_object) == MODULEOBJECT) {
/*    printf(" is module\n"); */
      result = FIND_OBJECT(module_object->value.DECLVALUE,
          take_reference(arg_3(arguments)));
      result = NULL;
/*    trace2(result); */
    } else {
/*    printf(" is not module\n"); */
      result = NULL;
    } /* if */
#else
    result = NULL;
#endif
    return(bld_reference_temp(result));
  } /* prg_find */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

objecttype prg_match (listtype arguments)
#else

objecttype prg_match (arguments)
listtype arguments;
#endif

  { /* prg_match */
    isit_prog(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    return(bld_reference_temp(prgMatch(
        take_reflist(arg_2(arguments)),
        take_prog(arg_1(arguments)))));
  } /* prg_match */
#endif



#ifdef ANSI_C

objecttype prg_match (listtype arguments)
#else

objecttype prg_match (arguments)
listtype arguments;
#endif

  {
    progtype currentProg;
    objectrecord expr_object;
    objecttype result;

  /* prg_match */
    isit_prog(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    currentProg = take_prog(arg_1(arguments));

    /* prot_list(take_reflist(arg_2(arguments)));
    printf("\n"); */
    expr_object.type_of = NULL;
    expr_object.descriptor.property = NULL;
    expr_object.value.listvalue = take_reflist(arg_2(arguments));
    INIT_CATEGORY_OF_OBJ(&expr_object, EXPROBJECT);

    result = match_prog_expression(currentProg->declaration_root, &expr_object);
    if (result != NULL) {
      if (CATEGORY_OF_OBJ(result) == MATCHOBJECT ||
          CATEGORY_OF_OBJ(result) == CALLOBJECT) {
        take_reflist(arg_2(arguments)) = expr_object.value.listvalue->next;
        result = expr_object.value.listvalue->obj;
        expr_object.value.listvalue->next = NULL;
        emptylist(expr_object.value.listvalue);
      } else {
        run_error(MATCHOBJECT, result);
      } /* if */
    } /* if */
    /* printf("result == %lx\n", result);
    trace1(result);
    printf("\n");
    prot_list(take_reflist(arg_2(arguments)));
    printf("\n"); */
    return(bld_reference_temp(result));
  } /* prg_match */



#ifdef ANSI_C

objecttype prg_match_expr (listtype arguments)
#else

objecttype prg_match_expr (arguments)
listtype arguments;
#endif

  {
    progtype currentProg;
    errinfotype err_info = OKAY_NO_ERROR;
    objecttype result;

  /* prg_match_expr */
    isit_prog(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    currentProg = take_prog(arg_1(arguments));
    /* prot_list(take_reflist(arg_2(arguments)));
    printf("\n"); */
    if (!ALLOC_OBJECT(result)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_OBJ(result, EXPROBJECT);
      result->value.listvalue = copy_list(take_reflist(arg_2(arguments)), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        result = match_prog_expression(currentProg->declaration_root, result);
        /* printf("result == %lx\n", result);
        trace1(result);
        printf("\n");
        prot_list(take_reflist(arg_2(arguments)));
        printf("\n");
        prot_list(result->value.listvalue);
        printf("\n"); */
        return(bld_reference_temp(result));
      } /* if */
    } /* if */
  } /* prg_match_expr */



#ifdef ANSI_C

objecttype prg_name (listtype arguments)
#else

objecttype prg_name (arguments)
listtype arguments;
#endif

  {
    stritype result;

  /* prg_name */
    result = cstri_to_stri(option.source_file_name);
    if (result == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      return(bld_stri_temp(result));
    } /* if */
  } /* prg_name */



#ifdef ANSI_C

objecttype prg_ne (listtype arguments)
#else

objecttype prg_ne (arguments)
listtype arguments;
#endif

  { /* prg_ne */
    isit_prog(arg_1(arguments));
    isit_prog(arg_3(arguments));
    if (take_prog(arg_1(arguments)) !=
        take_prog(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* prg_ne */



#ifdef ANSI_C

objecttype prg_prog (listtype arguments)
#else

objecttype prg_prog (arguments)
listtype arguments;
#endif

  { /* prg_prog */
    return(bld_prog_temp(NULL));
  } /* prg_prog */



#ifdef ANSI_C

objecttype prg_str_parse (listtype arguments)
#else

objecttype prg_str_parse (arguments)
listtype arguments;
#endif

  {
    stritype str1;
    progtype result;

  /* prg_str_parse */
    isit_stri(arg_1(arguments));
    str1 = take_stri(arg_1(arguments));
    result = analyze_string(str1);
    set_trace(option.exec_trace_level, -1, NULL);
    return(bld_prog_temp(result));
  } /* prg_str_parse */



#ifdef ANSI_C

objecttype prg_syobject (listtype arguments)
#else

objecttype prg_syobject (arguments)
listtype arguments;
#endif

  {
    progtype currentProg;
    stritype stri1;
    cstritype name;
    progrecord prog_backup;
    identtype ident_found;
    objecttype result;

  /* prg_syobject */
    isit_prog(arg_1(arguments));
    isit_stri(arg_2(arguments));
    currentProg = take_prog(arg_1(arguments));
    stri1 = take_stri(arg_2(arguments));
    name = cp_to_cstri(stri1);
    if (name == NULL) {
      result = raise_exception(SYS_MEM_EXCEPTION);
    } else {
      memcpy(&prog_backup, &prog, sizeof(progrecord));
      memcpy(&prog, currentProg, sizeof(progrecord));
      ident_found = get_ident((ustritype) name, strlen(name));
      if (ident_found == NULL ||
          ident_found->entity == NULL) {
        result = NULL;
      } else {
        result = ident_found->entity->syobject;
      } /* if */
      memcpy(&prog, &prog_backup, sizeof(progrecord));
      free_cstri(name, stri1);
    } /* if */
    return(bld_reference_temp(result));
  } /* prg_syobject */



#ifdef ANSI_C

objecttype prg_sysvar (listtype arguments)
#else

objecttype prg_sysvar (arguments)
listtype arguments;
#endif

  {
    progtype currentProg;
    stritype name;
    int index_found;
    objecttype result;

  /* prg_sysvar */
    isit_prog(arg_1(arguments));
    isit_stri(arg_2(arguments));
    currentProg = take_prog(arg_1(arguments));
    name = take_stri(arg_2(arguments));
    index_found = find_sysvar(name);
    if (index_found != -1) {
      result = currentProg->sys_var[index_found];
    } else {
      result = NULL;
    } /* if */
    return(bld_reference_temp(result));
  } /* prg_sysvar */



#ifdef ANSI_C

objecttype prg_value (listtype arguments)
#else

objecttype prg_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;

  /* prg_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != PROGOBJECT) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } else {
      return(bld_prog_temp(take_prog(obj_arg)));
    } /* if */
  } /* prg_value */
