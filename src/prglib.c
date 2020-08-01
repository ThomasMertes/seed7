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
/*  Module: Library                                                 */
/*  File: seed7/src/prglib.c                                        */
/*  Changes: 1991 - 1994, 2008, 2013 - 2016  Thomas Mertes          */
/*  Content: All primitive actions for the program type.            */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType prg_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    progType old_prog;
    progType prog_value;

  /* prg_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_prog(dest);
    isit_prog(source);
    is_variable(dest);
    old_prog = take_prog(dest);
    prog_value = take_prog(source);
    if (old_prog != prog_value) {
      prgDestr(old_prog);
      dest->value.progValue = prog_value;
      if (TEMP_OBJECT(source)) {
        source->value.progValue = NULL;
      } else {
        if (prog_value != NULL) {
          prog_value->usage_count++;
        } /* if */
      } /* if */
    } /* if */
    /* printf("prg_cpy: usage_count=%d\n", prog_value->usage_count); */
    return SYS_EMPTY_OBJECT;
  } /* prg_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType prg_create (listType arguments)

  {
    objectType source;
    progType prog_value;

  /* prg_create */
    source = arg_3(arguments);
    isit_prog(source);
    SET_CATEGORY_OF_OBJ(arg_1(arguments), PROGOBJECT);
    prog_value = take_prog(source);
    arg_1(arguments)->value.progValue = prog_value;
    if (TEMP_OBJECT(source)) {
      source->value.progValue = NULL;
    } else {
      if (prog_value != NULL) {
        prog_value->usage_count++;
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* prg_create */



/**
 *  Free the memory referred by 'old_prog/arg_1'.
 *  After prg_destr is left 'old_prog/arg_1' is NULL.
 *  The memory where 'old_prog/arg_1' is stored can be
 *  freed afterwards.
 */
objectType prg_destr (listType arguments)

  { /* prg_destr */
    isit_prog(arg_1(arguments));
    prgDestr(take_prog(arg_1(arguments)));
    arg_1(arguments)->value.progValue = NULL;
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* prg_destr */



/**
 *  Get an empty program (a program that does not exist).
 *  @return an empty program.
 */
objectType prg_empty (listType arguments)

  { /* prg_empty */
    return bld_prog_temp(NULL);
  } /* prg_empty */



/**
 *  Check if two program values are equal.
 *  @return TRUE if both values are equal, FALSE otherwise.
 */
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



/**
 *  Determine the number of errors in 'aProgram/arg_1'.
 *  @return the number of errors.
 */
objectType prg_error_count (listType arguments)

  { /* prg_error_count */
    isit_prog(arg_1(arguments));
    return bld_int_temp(prgErrorCount(
        take_prog(arg_1(arguments))));
  } /* prg_error_count */



/**
 *  Evaluate 'anExpression/arg_2' which is part of 'aProgram/arg_1'.
 *  @return the result of the evaluation.
 */
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
      /* The global variable curr_argument_list does not contain */
      /* the arguments of this function. Therefore the parameter */
      /* arguments is used instead.                              */
      return raise_with_arguments(prog->sys_var[err_info], arguments);
    } else {
      return bld_reference_temp(result);
    } /* if */
  } /* prg_eval */



/**
 *  Execute the program referred by 'aProgram/arg_1'.
 */
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



/**
 *  Parse the file with the name 'fileName/arg_1'.
 *  @param fileName/arg_1 File name of the file to be parsed.
 *  @param options/arg_2 Options to be used when the file is parsed.
 *  @param libraryDirs/arg_3 Search path for include/library files.
 *  @param protFileName/arg_4 Name of the protocol file.
 *  @return the parsed program.
 *  @exception RANGE_ERROR 'fileName/arg_1' does not use the standard path
 *             representation or 'fileName/arg_1' is not representable in
 *             the system path type.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
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



/**
 *  Determine the list of global defined objects in 'aProgram/arg_1'.
 *  The returned list contains constant and variable objects
 *  in the same order as the definitions of the source program.
 *  Literal objects and local objects are not part of this list.
 *  @return the list of global defined objects.
 */
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



/**
 *  Returns the name of 'aProg/arg_1' without path and extension.
 *  This function does not follow symbolic links.
 *  It determines, with which name a program was called.
 *  If a symbolic link refers to a program, the name of
 *  the symbolic link is returned.
 *  @return the name of the program.
 */
objectType prg_name (listType arguments)

  { /* prg_name */
    isit_prog(arg_1(arguments));
    return bld_stri_temp(strCreate(take_prog(arg_1(arguments))->program_name));
  } /* prg_name */



/**
 *  Check if two program values are not equal.
 *  @return FALSE if both values are equal, TRUE otherwise.
 */
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



/**
 *  Returns the name of the program without path and extension.
 *  This function does not follow symbolic links.
 *  It determines, with which name a program was called.
 *  If a symbolic link refers to a program, the name of
 *  the symbolic link is returned.
 *  @return the name of the program.
 */
objectType prg_own_name (listType arguments)

  { /* prg_own_name */
    return bld_stri_temp(strCreate(prog->program_name));
  } /* prg_own_name */



/**
 *  Return the absolute path of the program.
 *  For an interpreted program this is the absolute path of the source file.
 *  For a compiled program this is the absolute path of the executable.
 *  The function 'prg_own_path' does follow symbolic links.
 *  @return the absolute path of the program.
 */
objectType prg_own_path (listType arguments)

  { /* prg_own_path */
    return bld_stri_temp(strCreate(prog->program_path));
  } /* prg_own_path */



/**
 *  Return the absolute path of the program 'aProg/arg_1'.
 *  This function does follow symbolic links.
 *  @return the absolute path of the program.
 */
objectType prg_path (listType arguments)

  { /* prg_path */
    isit_prog(arg_1(arguments));
    return bld_stri_temp(strCreate(take_prog(arg_1(arguments))->program_path));
  } /* prg_path */



/**
 *  Parse the given string 'stri/arg_1'.
 *  @param stri/arg_1 'String' to be parsed.
 *  @param options/arg_2 Options to be used when the file is parsed.
 *  @param libraryDirs/arg_3 Search path for include/library files.
 *  @param protFileName/arg_4 Name of the protocol file.
 *  @return the parsed program.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
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



/**
 *  Determine object with 'syobjectName/arg_2' from program 'aProgram/arg_1'.
 *  @return a reference to the object, and
 *          NIL if no object 'syobjectName/arg_2' exists.
 *  @exception MEMORY_ERROR If 'syobjectName/arg_2' cannot be converted to
 *             the internal representation.
 */
objectType prg_syobject (listType arguments)

  { /* prg_syobject */
    isit_prog(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_reference_temp(prgSyobject(
        take_prog(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* prg_syobject */



/**
 *  Determine the value of the system variable 'name/arg_2' in 'aProgram/arg_1'.
 *  @return a reference to the value of the system variable, and
 *          NIL if no system variable 'name/arg_2' exists.
 */
objectType prg_sysvar (listType arguments)

  { /* prg_sysvar */
    isit_prog(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_reference_temp(prgSysvar(
        take_prog(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* prg_sysvar */



/**
 *  Get 'program' value of the object referenced by 'aReference/arg_1'.
 *  @return the 'program' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference/arg_1' is NIL or
 *             category(aReference) <> PROGOBJECT holds.
 */
objectType prg_value (listType arguments)

  {
    objectType aReference;

  /* prg_value */
    isit_reference(arg_1(arguments));
    aReference = take_reference(arg_1(arguments));
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != PROGOBJECT)) {
      logError(printf("prg_value(");
               trace1(aReference);
               printf("): Category is not PROGOBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_prog_temp(take_prog(aReference));
    } /* if */
  } /* prg_value */
