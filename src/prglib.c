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
#include "str_rtl.h"
#include "cmd_rtl.h"
#include "prg_comp.h"

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
    return SYS_EMPTY_OBJECT;
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
    return SYS_EMPTY_OBJECT;
  } /* prg_create */



#ifdef ANSI_C

objecttype prg_decl_objects (listtype arguments)
#else

objecttype prg_decl_objects (arguments)
listtype arguments;
#endif

  { /* prg_decl_objects */
    isit_prog(arg_1(arguments));
    return bld_reflist_temp(prgDeclObjects(
        take_prog(arg_1(arguments))));
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
    return SYS_EMPTY_OBJECT;
  } /* prg_destr */



#ifdef ANSI_C

objecttype prg_empty (listtype arguments)
#else

objecttype prg_empty (arguments)
listtype arguments;
#endif

  { /* prg_empty */
    return bld_prog_temp(NULL);
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
    if (take_prog(arg_1(arguments)) == take_prog(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
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
    return bld_int_temp(prgErrorCount(
        take_prog(arg_1(arguments))));
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
    return bld_reference_temp(result);
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
    return SYS_EMPTY_OBJECT;
  } /* prg_exec */



#ifdef ANSI_C

objecttype prg_fil_parse (listtype arguments)
#else

objecttype prg_fil_parse (arguments)
listtype arguments;
#endif

  { /* prg_fil_parse */
    isit_stri(arg_1(arguments));
    return bld_prog_temp(prgFilParse(
        take_stri(arg_1(arguments))));
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
    return bld_reference_temp(result);
  } /* prg_find */



#ifdef ANSI_C

objecttype prg_match (listtype arguments)
#else

objecttype prg_match (arguments)
listtype arguments;
#endif

  { /* prg_match */
    isit_prog(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    return bld_reference_temp(prgMatch(
        take_prog(arg_1(arguments)), take_reflist(arg_2(arguments))));
  } /* prg_match */



#ifdef ANSI_C

objecttype prg_match_expr (listtype arguments)
#else

objecttype prg_match_expr (arguments)
listtype arguments;
#endif

  { /* prg_match_expr */
    isit_prog(arg_1(arguments));
    isit_reflist(arg_2(arguments));
    return bld_reference_temp(prgMatchExpr(
        take_prog(arg_1(arguments)), take_reflist(arg_2(arguments))));
  } /* prg_match_expr */



#ifdef ANSI_C

objecttype prg_name (listtype arguments)
#else

objecttype prg_name (arguments)
listtype arguments;
#endif

  { /* prg_name */
    return bld_stri_temp(strCreate(prog.source_file_name));
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
    if (take_prog(arg_1(arguments)) != take_prog(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* prg_ne */



#ifdef ANSI_C

objecttype prg_path (listtype arguments)
#else

objecttype prg_path (arguments)
listtype arguments;
#endif

  { /* prg_path */
    return bld_stri_temp(strCreate(prog.source_file_path));
  } /* prg_path */



#ifdef ANSI_C

objecttype prg_prog (listtype arguments)
#else

objecttype prg_prog (arguments)
listtype arguments;
#endif

  { /* prg_prog */
    return bld_prog_temp(NULL);
  } /* prg_prog */



#ifdef ANSI_C

objecttype prg_str_parse (listtype arguments)
#else

objecttype prg_str_parse (arguments)
listtype arguments;
#endif

  { /* prg_str_parse */
    isit_stri(arg_1(arguments));
    return bld_prog_temp(prgStrParse(
        take_stri(arg_1(arguments))));
  } /* prg_str_parse */



#ifdef ANSI_C

objecttype prg_syobject (listtype arguments)
#else

objecttype prg_syobject (arguments)
listtype arguments;
#endif

  { /* prg_syobject */
    isit_prog(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_reference_temp(prgSyobject(
        take_prog(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* prg_syobject */



#ifdef ANSI_C

objecttype prg_sysvar (listtype arguments)
#else

objecttype prg_sysvar (arguments)
listtype arguments;
#endif

  { /* prg_sysvar */
    isit_prog(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_reference_temp(prgSysvar(
        take_prog(arg_1(arguments)), take_stri(arg_2(arguments))));
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
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_prog_temp(take_prog(obj_arg));
    } /* if */
  } /* prg_value */
