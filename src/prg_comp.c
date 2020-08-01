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
/*  Module: Seed7 compiler library                                  */
/*  File: seed7/src/prg_comp.c                                      */
/*  Changes: 1991 - 1994, 2008, 2013  Thomas Mertes                 */
/*  Content: Primitive actions for the program type.                */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "sigutl.h"
#include "data.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "flistutl.h"
#include "striutl.h"
#include "listutl.h"
#include "entutl.h"
#include "typeutl.h"
#include "syvarutl.h"
#include "identutl.h"
#include "traceutl.h"
#include "infile.h"
#include "analyze.h"
#include "name.h"
#include "exec.h"
#include "option.h"
#include "match.h"
#include "objutl.h"
#include "runerr.h"
#include "set_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "prg_comp.h"


extern boolType interpreter_exception;



static objectType copy_args (const const_rtlArrayType argv, const memSizeType start)

  {
    memSizeType argc;
    arrayType arg_array;
    memSizeType arg_idx;
    objectType result;

  /* copy_args */
    /* printf("start = %d\n", start); */
    if (argv == NULL || argv->max_position < 0) {
      argc = 0;
    } else {
      argc = (memSizeType) argv->max_position - start;
    } /* if */
    /* printf("argc = %d\n", argc); */
    if (ALLOC_ARRAY(arg_array, argc)) {
      arg_idx = 0;
      while (arg_idx < argc) {
        /* printf("arg_idx = %d\n", arg_idx);
           printf("argv[%d] = ", start + arg_idx);
           prot_stri(argv->arr[start + arg_idx].value.striValue);
           printf("\n"); */
        arg_array->arr[arg_idx].type_of = take_type(SYS_STRI_TYPE);
        arg_array->arr[arg_idx].descriptor.property = NULL;
        arg_array->arr[arg_idx].value.striValue =
            argv->arr[start + arg_idx].value.striValue;
        INIT_CATEGORY_OF_OBJ(&arg_array->arr[arg_idx], STRIOBJECT);
        arg_idx++;
      } /* while */
      arg_array->min_position = 1;
      arg_array->max_position = (intType) arg_idx;
    } /* if */
    if (arg_array != NULL) {
      if (ALLOC_OBJECT(result)) {
        result->type_of = NULL;
        result->descriptor.property = NULL;
        INIT_CATEGORY_OF_OBJ(result, ARRAYOBJECT);
        result->value.arrayValue = arg_array;
      } else {
        FREE_ARRAY(arg_array, argc);
      } /* if */
    } else {
      result = NULL;
    } /* if */
    return result;
  } /* copy_args */



static void free_args (objectType arg_v)

  {
    arrayType arg_array;
    memSizeType arg_array_size;

  /* free_args */
    arg_array = arg_v->value.arrayValue;
    arg_array_size = arraySize(arg_array);
    FREE_ARRAY(arg_array, arg_array_size);
    FREE_OBJECT(arg_v);
  } /* free_args */



void interpret (const progType currentProg, const const_rtlArrayType argv,
    memSizeType argvStart, uintType options, const const_striType protFileName)

  {
    progType progBackup;

  /* interpret */
    logFunction(printf("interpret(\"%s\")\n",
                       striAsUnquotedCStri(currentProg->program_path)););
    if (currentProg != NULL) {
      set_fail_flag(FALSE);
      fail_value = NULL;
      fail_expression = NULL;
      fail_stack = NULL;
      if (currentProg->main_object != NULL) {
        progBackup = prog;
        prog = currentProg;
        prog->option_flags = options;
        setup_signal_handlers((options & HANDLE_SIGNALS) != 0,
                              (options & TRACE_SIGNALS) != 0);
        set_trace(prog->option_flags);
        set_protfile_name(protFileName);
        prog->arg_v = copy_args(argv, argvStart);
        if (prog->arg_v == NULL) {
          raise_error(MEMORY_ERROR);
        } else {
          /* printf("main defined as: ");
          trace1(prog->main_object);
          printf("\n"); */
#ifdef WITH_PROTOCOL
          if (trace.actions) {
            if (trace.heapsize) {
              prot_heapsize();
              prot_cstri(" ");
            } /* if */
            prot_cstri("begin main");
            prot_nl();
          } /* if */
#endif
          interpreter_exception = TRUE;
          exec_call(prog->main_object);
          interpreter_exception = FALSE;
#ifdef WITH_PROTOCOL
          if (trace.actions) {
            if (trace.heapsize) {
              prot_heapsize();
              prot_cstri(" ");
            } /* if */
            prot_cstri("end main");
            prot_nl();
          } /* if */
#endif
          free_args(prog->arg_v);
#ifdef OUT_OF_ORDER
          shut_drivers();
          if (fail_flag) {
            printf("\n*** Uncaught EXCEPTION ");
            printobject(fail_value);
            printf(" raised with\n");
            prot_list(fail_expression);
            printf("\n");
            write_call_stack(fail_stack);
          } /* if */
#endif
        } /* if */
        prog = progBackup;
      } /* if */
    } /* if */
    logFunction(printf("interpret(\"%s\") -->\n",
                       striAsUnquotedCStri(currentProg->program_path)););
  } /* interpret */



void prgCpy (progType *const prog_to, const progType prog_from)

  {
    progType old_prog;

  /* prgCpy */
    old_prog = *prog_to;
    if (old_prog != prog_from) {
      prgDestr(old_prog);
      *prog_to = prog_from;
      if (prog_from != NULL) {
        prog_from->usage_count++;
      } /* if */
    } /* if */
    /* printf("prgCpy: usage_count=%d\n", (*prog_to)->usage_count); */
  } /* prgCpy */



progType prgCreate (const progType prog_from)

  {

  /* prgCreate */
    if (prog_from != NULL) {
      prog_from->usage_count++;
    } /* if */
    /* printf("prgCreate: usage_count=%d\n", prog_from->usage_count); */
    return prog_from;
  } /* prgCreate */



void prgDestr (progType old_prog)

  {
    progType progBackup;

  /* prgDestr */
    logFunction(printf("prgDestr(%lx)\n", (unsigned long) old_prog););
    if (old_prog != NULL) {
      /* printf("prgDestr: usage_count=%d\n", old_prog->usage_count); */
      old_prog->usage_count--;
      if (old_prog->usage_count == 0) {
        /* printf("prgDestr: old progType: %lx\n", old_prog); */
        progBackup = prog;
        prog = old_prog;
        /* printf("heapsize: %ld\n", heapsize()); */
        /* heap_statistic(); */
        close_stack(old_prog);
        close_declaration_root(old_prog);
        close_entity(old_prog);
        close_idents(old_prog);
        close_type(old_prog);
        remove_prog_files(old_prog);
        dump_list(old_prog->literals);
        free_entity(old_prog, old_prog->entity.literal);
        FREE_RECORD(old_prog->property.literal, propertyRecord, count.property);
        prog = progBackup;
        FREE_STRI(old_prog->arg0, old_prog->arg0->size);
        FREE_STRI(old_prog->program_name, old_prog->program_name->size);
        FREE_STRI(old_prog->program_path, old_prog->program_path->size);
        FREE_RECORD(old_prog->stack_global, stackRecord, count.stack);
        FREE_RECORD(old_prog, progRecord, count.prog);
        /* printf("heapsize: %ld\n", heapsize()); */
        /* heap_statistic(); */
      } /* if */
    } /* if */
    logFunction(printf("prgDestr\n"););
  } /* prgDestr */



intType prgErrorCount (const const_progType aProg)

  {
    intType result;

  /* prgErrorCount */
    if (aProg->error_count > INTTYPE_MAX) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = (intType) aProg->error_count;
    } /* if */
    return result;
  } /* prgErrorCount */



/**
 *  Evaluate ''anExpression'' which is part of ''aProgram''.
 *  @return the result of the evaluation.
 */
objectType prgEval (progType aProgram, objectType anExpression)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* prgEval */
    result = exec_expr(aProgram, anExpression, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
      result = NULL;
    } /* if */
    return result;
  } /* prgEval */



/**
 *  Execute the program referred by ''aProgram''.
 */
void prgExec (const progType aProgram, const const_rtlArrayType parameters,
    const const_setType options, const const_striType protFileName)

  {
    uintType int_options;

  /* prgExec */
    int_options = (uintType) setSConv(options);
    interpret(aProgram, parameters, 0, int_options, protFileName);
    set_fail_flag(FALSE);
    fail_value = NULL;
    fail_expression = NULL;
  } /* prgExec */



/**
 *  Parse the file with the name ''fileName''.
 *  @param fileName File name of the file to be parsed.
 *  @param options Options to be used when the file is parsed.
 *  @param libraryDirs Search path for include/library files.
 *  @param protFileName Name of the protocol file.
 *  @return the parsed program.
 *  @exception RANGE_ERROR ''fileName'' does not use the standard path
 *             representation or ''fileName'' is not representable in
 *             the system path type.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
progType prgFilParse (const const_striType fileName, const const_setType options,
    const const_rtlArrayType libraryDirs, const const_striType protFileName)

  {
    uintType int_options;
    errInfoType err_info = OKAY_NO_ERROR;
    progType result;

  /* prgFilParse */
    logFunction(printf("prgFilParse(\"%s\")\n", striAsUnquotedCStri(fileName)););
    int_options = (uintType) setSConv(options);
    /* printf("options: %03x\n", int_options); */
    result = analyzeFile(fileName, int_options, libraryDirs, protFileName, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      logError(printf("prgFilParse(\"%s\"): analyzeFile() failed:\n"
                      "int_options=" F_X(03) "\nerr_info=%d\n",
                      striAsUnquotedCStri(fileName), int_options, err_info););
      raise_error(err_info);
    } /* if */
    return result;
  } /* prgFilParse */



/**
 *  Determine the list of global defined objects in ''aProgram''.
 *  The returned list contains constant and variable objects
 *  in the same order as the definitions of the source program.
 *  Literal objects and local objects are not part of this list.
 *  @return the list of global defined objects.
 */
listType prgGlobalObjects (const const_progType aProgram)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* prgGlobalObjects */
    if (aProgram->stack_current != NULL) {
      result = copy_list(aProgram->stack_global->local_object_list, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } else {
      result = NULL;
    } /* if */
    return result;
  } /* prgGlobalObjects */



objectType prgMatch (const const_progType aProg, listType curr_expr)

  {
    objectRecord expr_object;
    objectType result;

  /* prgMatch */
    /* prot_list(curr_expr);
    printf("\n"); */
    expr_object.type_of = NULL;
    expr_object.descriptor.property = NULL;
    expr_object.value.listValue = curr_expr;
    INIT_CATEGORY_OF_OBJ(&expr_object, EXPROBJECT);

    result = match_prog_expression(aProg->declaration_root, &expr_object);
    if (result != NULL) {
      if (CATEGORY_OF_OBJ(result) == MATCHOBJECT ||
          CATEGORY_OF_OBJ(result) == CALLOBJECT) {
        curr_expr = expr_object.value.listValue->next;
        result = expr_object.value.listValue->obj;
        expr_object.value.listValue->next = NULL;
        free_list(expr_object.value.listValue);
      } else {
        run_error(MATCHOBJECT, result);
      } /* if */
    } /* if */
    /* printf("result == %lx\n", result);
    trace1(result);
    printf("\n");
    prot_list(curr_expr);
    printf("\n"); */
    return result;
  } /* prgMatch */



objectType prgMatchExpr (const const_progType aProg, listType curr_expr)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* prgMatchExpr */
    /* prot_list(curr_expr);
    printf("\n"); */
    if (!ALLOC_OBJECT(result)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_OBJ(result, EXPROBJECT);
      result->value.listValue = copy_list(curr_expr, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = match_prog_expression(aProg->declaration_root, result);
        /* printf("result == %lx\n", result);
        trace1(result);
        printf("\n");
        prot_list(curr_expr);
        printf("\n");
        prot_list(result->value.listValue);
        printf("\n"); */
      } /* if */
    } /* if */
    return result;
  } /* prgMatchExpr */



const_striType prgName (const const_progType aProg)

  { /* prgName */
    return aProg->program_name;
  } /* prgName */



const_striType prgPath (const const_progType aProg)

  { /* prgPath */
    return aProg->program_path;
  } /* prgPath */



/**
 *  Parse the given ''string''.
 *  @param stri ''String'' to be parsed.
 *  @param options Options to be used when the file is parsed.
 *  @param libraryDirs Search path for include/library files.
 *  @param protFileName Name of the protocol file.
 *  @return the parsed program.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
progType prgStrParse (const const_striType stri, const const_setType options,
    const const_rtlArrayType libraryDirs, const const_striType protFileName)

  {
    uintType int_options;
    errInfoType err_info = OKAY_NO_ERROR;
    progType result;

  /* prgStrParse */
    logFunction(printf("prgStrParse(\"%s\")\n", striAsUnquotedCStri(stri)););
    int_options = (uintType) setSConv(options);
    result = analyzeString(stri, int_options, libraryDirs, protFileName, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      logError(printf("prgStrParse(\"%s\"): analyzeString() failed:\n"
                      "int_options=" F_X(03) "\nerr_info=%d\n",
                      striAsUnquotedCStri(stri), int_options, err_info););
      raise_error(err_info);
    } /* if */
    return result;
  } /* prgStrParse */



/**
 *  Determine object with ''syobjectName'' from program ''aProgram''.
 *  @return a reference to the object or NIL when no object ''syobjectName'' exists.
 *  @exception MEMORY_ERROR When ''syobjectName'' cannot be converted to
 *             the internal representation.
 */
objectType prgSyobject (const progType aProgram, const const_striType syobjectName)

  {
    cstriType name;
    identType ident_found;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* prgSyobject */
    name = stri_to_cstri8(syobjectName, &err_info);
    if (name == NULL) {
      raise_error(err_info);
      result = NULL;
    } else {
      ident_found = get_ident(aProgram, (const_ustriType) name);
      if (ident_found == NULL ||
          ident_found->entity == NULL) {
        result = NULL;
      } else {
        result = ident_found->entity->syobject;
      } /* if */
      free_cstri8(name, syobjectName);
    } /* if */
    return result;
  } /* prgSyobject */



objectType prgSysvar (const const_progType aProg, const const_striType sysvarName)

  {
    int index_found;
    objectType result;

  /* prgSysvar */
    index_found = find_sysvar(sysvarName);
    if (index_found != -1) {
      result = aProg->sys_var[index_found];
    } else {
      result = NULL;
    } /* if */
    return result;
  } /* prgSysvar */
