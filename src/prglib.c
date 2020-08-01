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
#include "data_rtl.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "identutl.h"
#include "listutl.h"
#include "arrutl.h"
#include "entutl.h"
#include "findid.h"
#include "traceutl.h"
#include "objutl.h"
#include "analyze.h"
#include "exec.h"
#include "runerr.h"
#include "match.h"
#include "name.h"
#include "str_rtl.h"
#include "prg_comp.h"

#undef EXTERN
#define EXTERN
#include "prglib.h"


#define TABLE_INCREMENT 127



objectType prg_cpy (listType arguments)

  {
    objectType prog_to;
    objectType prog_from;
    progType old_prog;
    progType prog_value;

  /* prg_cpy */
    prog_to = arg_1(arguments);
    prog_from = arg_3(arguments);
    isit_prog(prog_to);
    isit_prog(prog_from);
    is_variable(prog_to);
    old_prog = take_prog(prog_to);
    prog_value = take_prog(prog_from);
    if (old_prog != prog_value) {
      prgDestr(old_prog);
      prog_to->value.progValue = prog_value;
      if (TEMP_OBJECT(prog_from)) {
        prog_from->value.progValue = NULL;
      } else {
        if (prog_value != NULL) {
          prog_value->usage_count++;
        } /* if */
      } /* if */
    } /* if */
    /* printf("prg_cpy: usage_count=%d\n", prog_value->usage_count); */
    return SYS_EMPTY_OBJECT;
  } /* prg_cpy */



objectType prg_create (listType arguments)

  {
    objectType prog_from;
    progType prog_value;

  /* prg_create */
    prog_from = arg_3(arguments);
    isit_prog(prog_from);
    SET_CATEGORY_OF_OBJ(arg_1(arguments), PROGOBJECT);
    prog_value = take_prog(prog_from);
    arg_1(arguments)->value.progValue = prog_value;
    if (TEMP_OBJECT(prog_from)) {
      prog_from->value.progValue = NULL;
    } else {
      if (prog_value != NULL) {
        prog_value->usage_count++;
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prg_create */



objectType prg_destr (listType arguments)

  { /* prg_destr */
    isit_prog(arg_1(arguments));
    prgDestr(take_prog(arg_1(arguments)));
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* prg_destr */



objectType prg_empty (listType arguments)

  { /* prg_empty */
    return bld_prog_temp(NULL);
  } /* prg_empty */



objectType prg_eq (listType arguments)

  { /* prg_eq */
    isit_prog(arg_1(arguments));
    isit_prog(arg_3(arguments));
    if (take_prog(arg_1(arguments)) == take_prog(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* prg_eq */



objectType prg_error_count (listType arguments)

  { /* prg_error_count */
    isit_prog(arg_1(arguments));
    return bld_int_temp(prgErrorCount(
        take_prog(arg_1(arguments))));
  } /* prg_error_count */



objectType prg_eval (listType arguments)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* prg_eval */
    isit_prog(arg_1(arguments));
    isit_reference(arg_2(arguments));
    result = exec_expr(take_prog(arg_1(arguments)),
                       take_reference(arg_2(arguments)),
                       &err_info);
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(prog.sys_var[err_info]);
    } else {
      return bld_reference_temp(result);
    } /* if */
  } /* prg_eval */



objectType prg_exec (listType arguments)

  {
    rtlArrayType parameters;

  /* prg_exec */
    isit_prog(arg_1(arguments));
    isit_array(arg_2(arguments));
    isit_set(arg_3(arguments));
    isit_stri(arg_4(arguments));
    parameters = gen_rtl_array(take_array(arg_2(arguments)));
    if (parameters == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      prgExec(take_prog(arg_1(arguments)),
              parameters,
              take_set(arg_3(arguments)),
              take_stri(arg_4(arguments)));
      FREE_RTL_ARRAY(parameters, ARRAY_LENGTH(parameters));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prg_exec */



objectType prg_fil_parse (listType arguments)

  {
    rtlArrayType libraryDirs;
    progType program;

  /* prg_fil_parse */
    isit_stri(arg_1(arguments));
    isit_set(arg_2(arguments));
    isit_array(arg_3(arguments));
    isit_stri(arg_4(arguments));
    libraryDirs = gen_rtl_array(take_array(arg_3(arguments)));
    if (libraryDirs == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      program = prgFilParse(take_stri(arg_1(arguments)),
                            take_set(arg_2(arguments)),
                            libraryDirs,
                            take_stri(arg_4(arguments)));
      FREE_RTL_ARRAY(libraryDirs, ARRAY_LENGTH(libraryDirs));
      return bld_prog_temp(program);
    } /* if */
  } /* prg_fil_parse */



objectType prg_find (listType arguments)

  {
    /* objectType module_object; */
    objectType result;

  /* prg_find */
    isit_reference(arg_1(arguments));
    isit_reference(arg_3(arguments));
#ifdef OUT_OF_ORDER
    module_object = take_reference(arg_1(arguments));
/*  trace2(module_object); */
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
    return bld_reference_temp(result);
  } /* prg_find */



objectType prg_global_objects (listType arguments)

  { /* prg_global_objects */
    isit_prog(arg_1(arguments));
    return bld_reflist_temp(prgGlobalObjects(
        take_prog(arg_1(arguments))));
  } /* prg_global_objects */



objectType prg_match (listType arguments)

  { /* prg_match */
    isit_prog(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    return bld_reference_temp(prgMatch(
        take_prog(arg_1(arguments)), take_reflist(arg_2(arguments))));
  } /* prg_match */



objectType prg_match_expr (listType arguments)

  { /* prg_match_expr */
    isit_prog(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    return bld_reference_temp(prgMatchExpr(
        take_prog(arg_1(arguments)), take_reflist(arg_2(arguments))));
  } /* prg_match_expr */



objectType prg_name (listType arguments)

  { /* prg_name */
    isit_prog(arg_1(arguments));
    return bld_stri_temp(strCreate(take_prog(arg_1(arguments))->program_name));
  } /* prg_name */



objectType prg_ne (listType arguments)

  { /* prg_ne */
    isit_prog(arg_1(arguments));
    isit_prog(arg_3(arguments));
    if (take_prog(arg_1(arguments)) != take_prog(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* prg_ne */



objectType prg_own_name (listType arguments)

  { /* prg_own_name */
    return bld_stri_temp(strCreate(prog.program_name));
  } /* prg_own_name */



objectType prg_own_path (listType arguments)

  { /* prg_own_path */
    return bld_stri_temp(strCreate(prog.program_path));
  } /* prg_own_path */



objectType prg_path (listType arguments)

  { /* prg_path */
    isit_prog(arg_1(arguments));
    return bld_stri_temp(strCreate(take_prog(arg_1(arguments))->program_path));
  } /* prg_path */



objectType prg_prog (listType arguments)

  { /* prg_prog */
    return bld_prog_temp(NULL);
  } /* prg_prog */



objectType prg_str_parse (listType arguments)

  {
    rtlArrayType libraryDirs;
    progType program;

  /* prg_str_parse */
    isit_stri(arg_1(arguments));
    isit_set(arg_2(arguments));
    isit_array(arg_3(arguments));
    isit_stri(arg_4(arguments));
    libraryDirs = gen_rtl_array(take_array(arg_3(arguments)));
    if (libraryDirs == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      program = prgStrParse(take_stri(arg_1(arguments)),
                            take_set(arg_2(arguments)),
                            libraryDirs,
                            take_stri(arg_4(arguments)));
      FREE_RTL_ARRAY(libraryDirs, ARRAY_LENGTH(libraryDirs));
      return bld_prog_temp(program);
    } /* if */
  } /* prg_str_parse */



objectType prg_syobject (listType arguments)

  { /* prg_syobject */
    isit_prog(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_reference_temp(prgSyobject(
        take_prog(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* prg_syobject */



objectType prg_sysvar (listType arguments)

  { /* prg_sysvar */
    isit_prog(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_reference_temp(prgSysvar(
        take_prog(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* prg_sysvar */



objectType prg_value (listType arguments)

  {
    objectType obj_arg;

  /* prg_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != PROGOBJECT) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_prog_temp(take_prog(obj_arg));
    } /* if */
  } /* prg_value */
