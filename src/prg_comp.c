/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013, 2015, 2016, 2021  Thomas Mertes      */
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
/*  Changes: 1991 - 1994, 2008, 2013, 2015, 2016 Thomas Mertes      */
/*           2021  Thomas Mertes                                    */
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
#include "error.h"
#include "name.h"
#include "exec.h"
#include "option.h"
#include "match.h"
#include "objutl.h"
#include "runerr.h"
#include "str_rtl.h"
#include "set_rtl.h"
#include "fil_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "prg_comp.h"


extern boolType interpreter_exception;



/**
 *  Generate an object with an arrayType array as value.
 *  The elements of the original argv array are copied with assignments.
 *  Create functions are not called.
 *  The returned objectType must be freed with free_args().
 */
static objectType copy_args (const const_rtlArrayType argv,
    const memSizeType start)

  {
    memSizeType argc;
    arrayType arg_array;
    memSizeType arg_idx;
    objectType result;

  /* copy_args */
    logFunction(if (argv == NULL) {
                  printf("copy_args(NULL, " FMT_U_MEM ")\n",
                         start);
                } else {
                  printf("copy_args(array[" FMT_D " .. " FMT_D "], "
                         FMT_U_MEM ")\n", argv->min_position,
                         argv->max_position, start);
                });
    if (argv == NULL || argv->max_position < 0) {
      argc = 0;
    } else {
      argc = (memSizeType) argv->max_position - start;
    } /* if */
    /* printf("argc = %d\n", argc); */
    if (unlikely(!ALLOC_ARRAY(arg_array, argc))) {
      result = NULL;
    } else {
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
      if (unlikely(!ALLOC_OBJECT(result))) {
        FREE_ARRAY(arg_array, argc);
      } else {
        result->type_of = NULL;
        result->descriptor.property = NULL;
        INIT_CATEGORY_OF_OBJ(result, ARRAYOBJECT);
        result->value.arrayValue = arg_array;
      } /* if */
    } /* if */
    logFunction(printf("copy_args --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* copy_args */



static void free_args (objectType arg_v)

  {
    arrayType arg_array;
    memSizeType arg_array_size;

  /* free_args */
    logFunction(printf("free_args(");
                trace1(arg_v);
                printf("\n"););
    arg_array = take_array(arg_v);
    arg_array_size = arraySize(arg_array);
    FREE_ARRAY(arg_array, arg_array_size);
    FREE_OBJECT(arg_v);
  } /* free_args */



static void free_obj_and_prop (listType list)

  {
    listType list_elem;
    listType list_end;

  /* free_obj_and_prop */
    if (list != NULL) {
      list_elem = list;
      do {
        FREE_RECORD(list_elem->obj->descriptor.property, propertyRecord, count.property);
        FREE_OBJECT(list_elem->obj);
        list_end = list_elem;
        list_elem = list_elem->next;
      } while (list_elem != NULL);
      free_list2(list, list_end);
    } /* if */
  } /* free_obj_and_prop */



void interpret (const const_progType currentProg, const const_rtlArrayType argv,
    memSizeType argvStart, uintType options, const const_striType protFileName)

  {
    progType progBackup;
    boolType backup_interpreter_exception;

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
        /* Cast to mutable structure */
        prog = (progType) currentProg;
        prog->option_flags = options;
        setupSignalHandlers((options & HANDLE_SIGNALS) != 0,
                            (options & TRACE_SIGNALS) != 0,
                            FALSE, FALSE, doSuspendInterpreter);
        set_trace(prog->option_flags);
        set_protfile_name(protFileName);
        if (prog->arg_v != NULL) {
          free_args(prog->arg_v);
        } /* if */
        prog->arg_v = copy_args(argv, argvStart);
        if (unlikely(prog->arg_v == NULL)) {
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
          backup_interpreter_exception = interpreter_exception;
          interpreter_exception = TRUE;
          curr_exec_object = NULL;
          evaluate(prog->main_object);
          interpreter_exception = backup_interpreter_exception;
          free_args(prog->arg_v);
          prog->arg_v = NULL;
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
#ifdef OUT_OF_ORDER
          shutDrivers();
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



/**
 *  Parse the given 'bstring'.
 *  @param bstri 'BString' to be parsed.
 *  @param options Options to be used when the file is parsed.
 *  @param libraryDirs Search path for include/library files.
 *  @param errorFile File for the error messages.
 *  @return the parsed program.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
progType prgBStriParse (const bstriType bstri, const const_setType options,
    const const_rtlArrayType libraryDirs, const fileType errorFile)

  {
    uintType int_options;
    errInfoType err_info = OKAY_NO_ERROR;
    progType resultProg;

  /* prgBStriParse */
    logFunction(printf("prgBStriParse(\"%s\")\n", bstriAsUnquotedCStri(bstri)););
    int_options = (uintType) setSConv(options);
    resultProg = analyzeBString(bstri, int_options, libraryDirs, errorFile, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("prgBStriParse(\"%s\"): analyzeBString() failed:\n"
                      "int_options=" F_X(03) "\nerr_info=%d\n",
                      bstriAsUnquotedCStri(bstri), int_options, err_info););
      raise_error(err_info);
    } /* if */
    logFunction(printf("prgBStriParse --> " FMT_X_MEM " (error_count=%u)\n",
                       (memSizeType) resultProg,
                       resultProg != 0 ? resultProg->error_count : 0););
    return resultProg;
  } /* prgBStriParse */



/**
 *  Assign source to *dest.
 *  A copy function assumes that *dest contains a legal value.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void prgCpy (progType *const dest, const progType source)

  {
    progType old_prog;

  /* prgCpy */
    logFunction(printf("prgCpy(" FMT_X_MEM " (usage=" FMT_U "), "
                               FMT_X_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) *dest,
                       *dest != NULL ? (*dest)->usage_count : (uintType) 0,
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    old_prog = *dest;
    if (old_prog != source) {
      prgDestr(old_prog);
      *dest = source;
      if (source != NULL) {
        source->usage_count++;
      } /* if */
    } /* if */
    logFunction(printf("prgCpy(" FMT_X_MEM " (usage=" FMT_U "), "
                               FMT_X_MEM " (usage=" FMT_U ")) -->\n",
                       (memSizeType) *dest,
                       *dest != NULL ? (*dest)->usage_count : (uintType) 0,
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
  } /* prgCpy */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
progType prgCreate (const progType source)

  {

  /* prgCreate */
    logFunction(printf("prgCreate(" FMT_X_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    if (source != NULL) {
      source->usage_count++;
    } /* if */
    logFunction(printf("prgCreate(" FMT_X_MEM " (usage=" FMT_U ")) -->\n",
                       (memSizeType) source,
                       source != NULL ? source->usage_count : (uintType) 0););
    return source;
  } /* prgCreate */



/**
 *  Free the memory referred by 'old_prog'.
 *  After prgDestr is left 'old_prog' refers to not existing memory.
 *  The memory where 'old_prog' is stored can be freed afterwards.
 */
void prgDestr (progType old_prog)

  {
    progType progBackup;

  /* prgDestr */
    logFunction(printf("prgDestr(" FMT_X_MEM " (usage=" FMT_U "))\n",
                       (memSizeType) old_prog,
                       old_prog != NULL ? old_prog->usage_count
                                        : (uintType) 0););
    if (old_prog != NULL) {
      /* printf("prgDestr: usage_count=%d\n", old_prog->usage_count); */
      old_prog->usage_count--;
      if (old_prog->usage_count == 0) {
        /* printf("prgDestr: old progType: %lx\n", old_prog); */
        progBackup = prog;
        prog = old_prog;
        /* printf("heapsize: %ld\n", heapsize()); */
        /* heapStatistic(); */
        dump_list(old_prog->exec_expr_temp_results);
        close_stack(old_prog);
        close_declaration_root(old_prog);
        close_entity(old_prog);
        close_idents(old_prog);
        close_type(old_prog);
        removeProgFiles(old_prog);
        dump_list(old_prog->literals);
        free_entity(old_prog, old_prog->entity.literal);
        if (old_prog->property.literal != NULL) {
          FREE_RECORD(old_prog->property.literal, propertyRecord, count.property);
        } /* if */
        strDestr(old_prog->arg0);
        strDestr(old_prog->program_name);
        strDestr(old_prog->program_path);
        if (old_prog->arg_v != NULL) {
          free_args(old_prog->arg_v);
        } /* if */
        if (old_prog->stack_global != NULL) {
          FREE_RECORD(old_prog->stack_global, stackRecord, count.stack);
        } /* if */
        filDestr(old_prog->errorFile);
        freeErrorList(old_prog->errorList);
        dump_list(old_prog->substituted_objects);
        free_obj_and_prop(old_prog->struct_objects);
        FREE_RECORD(old_prog, progRecord, count.prog);
        prog = progBackup;
        /* printf("heapsize: %ld\n", heapsize()); */
        /* heapStatistic(); */
      } /* if */
    } /* if */
    logFunction(printf("prgDestr(" FMT_X_MEM ") -->\n",
                       (memSizeType) old_prog););
  } /* prgDestr */



/**
 *  Determine the number of errors in 'aProgram'.
 *  @return the number of errors.
 */
intType prgErrorCount (const const_progType aProgram)

  {
    intType result;

  /* prgErrorCount */
    logFunction(printf("prgErrorCount(" FMT_X_MEM ")\n",
                       (memSizeType) aProgram););
    if (unlikely(aProgram == NULL)) {
      logError(printf("prgErrorCount(" FMT_X_MEM "): Program empty.\n",
                      (memSizeType) aProgram););
      raise_error(RANGE_ERROR);
      result = 0;
    } else if (unlikely(aProgram->error_count > INTTYPE_MAX)) {
      logError(printf("prgErrorCount(" FMT_X_MEM "): "
                      "Error count %u too big.\n",
                      (memSizeType) aProgram, aProgram->error_count););
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = (intType) aProgram->error_count;
    } /* if */
    logFunction(printf("prgErrorCount(" FMT_X_MEM ") --> " FMT_D "\n",
                       (memSizeType) aProgram, result););
    return result;
  } /* prgErrorCount */



/**
 *  Evaluate 'anExpression' which is part of 'aProgram'.
 *  @return the result of the evaluation.
 */
objectType prgEval (progType aProgram, objectType anExpression)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* prgEval */
    logFunction(printf("prgEval(" FMT_X_MEM ")\n",
                       (memSizeType) aProgram););
    result = exec_expr(aProgram, anExpression, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
      result = NULL;
    } /* if */
    return result;
  } /* prgEval */



/**
 *  Execute the program referred by 'aProgram'.
 */
void prgExec (const const_progType aProgram, const const_rtlArrayType parameters,
    const const_setType options, const const_striType protFileName)

  {
    uintType int_options;

  /* prgExec */
    logFunction(printf("prgExec(\"%s\")\n",
                       striAsUnquotedCStri(aProgram->program_path)););
    int_options = (uintType) setSConv(options);
    interpret(aProgram, parameters, 0, int_options, protFileName);
    set_fail_flag(FALSE);
    fail_value = NULL;
    fail_expression = NULL;
    logFunction(printf("prgExec(\"%s\") -->\n",
                       striAsUnquotedCStri(aProgram->program_path)););
  } /* prgExec */



/**
 *  Parse the file with the name 'fileName'.
 *  @param fileName File name of the file to be parsed.
 *  @param options Options to be used when the file is parsed.
 *  @param libraryDirs Search path for include/library files.
 *  @param errorFile File for the error messages.
 *  @return the parsed program.
 *  @exception RANGE_ERROR 'fileName' does not use the standard path
 *             representation or 'fileName' is not representable in
 *             the system path type.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
progType prgFilParse (const const_striType fileName, const const_setType options,
    const const_rtlArrayType libraryDirs, const fileType errorFile)

  {
    uintType int_options;
    errInfoType err_info = OKAY_NO_ERROR;
    progType resultProg;

  /* prgFilParse */
    logFunction(printf("prgFilParse(\"%s\")\n", striAsUnquotedCStri(fileName)););
    int_options = (uintType) setSConv(options);
    /* printf("options: 0x" F_X(016) "\n", int_options); */
    resultProg = analyzeFile(fileName, int_options, libraryDirs, errorFile, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("prgFilParse(\"%s\"): analyzeFile() failed:\n"
                      "int_options=0x" F_X(016) "\nerr_info=%d\n",
                      striAsUnquotedCStri(fileName), int_options, err_info););
      raise_error(err_info);
    } /* if */
    logFunction(printf("prgFilParse --> " FMT_X_MEM " (error_count=%u)\n",
                       (memSizeType) resultProg,
                       resultProg != 0 ? resultProg->error_count : 0););
    return resultProg;
  } /* prgFilParse */



void prgGetError (const const_progType aProg, intType errorIndex,
    intType *errorNumber, striType *fileName, intType *lineNumber,
    intType *columnNumber, striType *msg, striType *errorLine)

  {
    parseErrorType error;
    uintType index;

  /* prgGetError */
    logFunction(printf("prgGetError(" FMT_X_MEM ", " FMT_D ", ...)\n",
                       (memSizeType) aProg, errorIndex););
    if (unlikely(aProg == NULL)) {
      logError(printf("prgGetError(" FMT_X_MEM ", " FMT_D ", ...): "
                      "Program empty.\n",
                      (memSizeType) aProg, errorIndex););
      raise_error(RANGE_ERROR);
    } else if (unlikely(errorIndex <= 0 ||
                        errorIndex > aProg->error_count)) {
      logError(printf("prgGetError(" FMT_X_MEM ", " FMT_D ", ...): "
                      "Error index not in allowed range (1 .. %u).\n",
                      (memSizeType) aProg, errorIndex,
                      aProg->error_count););
      raise_error(RANGE_ERROR);
    } else {
      error = aProg->errorList;
      index = (uintType) errorIndex;
      while (--index > 0 && error != NULL) {
        error = error->next;
      } /* while */
      if (unlikely(error == NULL)) {
        logError(printf("prgGetError(" FMT_X_MEM ", " FMT_D ", ...): "
                        "Error data is NULL.\n",
                        (memSizeType) aProg, errorIndex););
        raise_error(RANGE_ERROR);
      } else if (unlikely(error->columnNumber > INTTYPE_MAX)) {
        logError(printf("prgGetError(" FMT_X_MEM ", " FMT_D ", ...): "
                        "Column number " FMT_U_MEM " too big.\n",
                        (memSizeType) aProg, errorIndex,
                        error->columnNumber););
        raise_error(RANGE_ERROR);
      } else {
        *errorNumber = (intType) error->err;
        strCopy(fileName, error->fileName);
        *lineNumber = error->lineNumber;
        *columnNumber = (intType) error->columnNumber;
        strCopy(msg, error->msg);
        strCopy(errorLine, error->errorLine);
      } /* if */
    } /* if */
    logFunction(printf("prgGetError(" FMT_X_MEM ", " FMT_D ", " FMT_D
                                    ", \"%s\", " FMT_D ", " FMT_D,
                       (memSizeType) aProg, errorIndex, *errorNumber,
                       striAsUnquotedCStri(*fileName), *lineNumber,
                       *columnNumber);
                printf(", \"%s\"", striAsUnquotedCStri(*msg));
                printf(", \"%s\") -->\n",
                       striAsUnquotedCStri(*errorLine)););
  } /* prgGetError */



/**
 *  Determine the list of global defined objects in 'aProgram'.
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
    logFunction(printf("prgGlobalObjects(" FMT_X_MEM ")\n",
                       (memSizeType) aProgram););
    if (unlikely(aProgram == NULL)) {
      logError(printf("prgGlobalObjects(" FMT_X_MEM "): Program empty.\n",
                      (memSizeType) aProgram););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (aProgram->stack_current != NULL) {
      result = copy_list(aProgram->stack_global->local_object_list, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
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
    logFunction(printf("prgMatch(" FMT_X_MEM ", ",
                       (memSizeType) aProg);
                prot_list(curr_expr);
                printf(")\n"););
    if (unlikely(aProg == NULL)) {
      logError(printf("prgMatch(" FMT_X_MEM ", ",
                       (memSizeType) aProg);
                prot_list(curr_expr);
                printf("): Program empty.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
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
    } /* if */
    return result;
  } /* prgMatch */



objectType prgMatchExpr (const const_progType aProg, listType curr_expr)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* prgMatchExpr */
    logFunction(printf("prgMatchExpr(" FMT_X_MEM ", ",
                       (memSizeType) aProg);
                prot_list(curr_expr);
                printf(")\n"););
    if (unlikely(aProg == NULL)) {
      logError(printf("prgMatchExpr(" FMT_X_MEM ", ",
                       (memSizeType) aProg);
                prot_list(curr_expr);
                printf("): Program empty.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (unlikely(!ALLOC_OBJECT(result))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result->type_of = NULL;
        result->descriptor.property = NULL;
        INIT_CATEGORY_OF_OBJ(result, EXPROBJECT);
        result->value.listValue = copy_list(curr_expr, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
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
    } /* if */
    return result;
  } /* prgMatchExpr */



/**
 *  Returns the name of the program without path and extension.
 *  This function does not follow symbolic links.
 *  It determines, with which name a program was called.
 *  If a symbolic link refers to a program, the name of
 *  the symbolic link is returned.
 *  @return the name of the program.
 */
const_striType prgName (const const_progType aProg)

  {
    const_striType name;

  /* prgName */
    logFunction(printf("prgName(" FMT_X_MEM ")\n",
                       (memSizeType) aProg););
    if (unlikely(aProg == NULL)) {
      logError(printf("prgName(" FMT_X_MEM "): Program empty.\n",
                      (memSizeType) aProg););
      raise_error(RANGE_ERROR);
      name = NULL;
    } else {
      name = aProg->program_name;
    } /* if */
    logFunction(printf("prgName(" FMT_X_MEM ") --> \"%s\"\n",
                       (memSizeType) aProg,
                       striAsUnquotedCStri(name)););
    return name;
  } /* prgName */



/**
 *  Return the absolute path of the program 'aProg'.
 *  This function does follow symbolic links.
 *  @return the absolute path of the program.
 */
const_striType prgPath (const const_progType aProg)

  {
    const_striType path;

  /* prgPath */
    logFunction(printf("prgPath(" FMT_X_MEM ")\n",
                       (memSizeType) aProg););
    if (unlikely(aProg == NULL)) {
      logError(printf("prgPath(" FMT_X_MEM "): Program empty.\n",
                      (memSizeType) aProg););
      raise_error(RANGE_ERROR);
      path = NULL;
    } else {
      path = aProg->program_path;
    } /* if */
    logFunction(printf("prgName(" FMT_X_MEM ") --> \"%s\"\n",
                       (memSizeType) aProg,
                       striAsUnquotedCStri(path)););
    return path;
  } /* prgPath */



/**
 *  Parse the given 'string'.
 *  @param stri 'String' to be parsed.
 *  @param options Options to be used when the file is parsed.
 *  @param libraryDirs Search path for include/library files.
 *  @param errorFile File for the error messages.
 *  @return the parsed program.
 *  @exception MEMORY_ERROR An out of memory situation occurred.
 */
progType prgStrParse (const const_striType stri, const const_setType options,
    const const_rtlArrayType libraryDirs, const fileType errorFile)

  {
    uintType int_options;
    errInfoType err_info = OKAY_NO_ERROR;
    progType resultProg;

  /* prgStrParse */
    logFunction(printf("prgStrParse(\"%s\")\n", striAsUnquotedCStri(stri)););
    int_options = (uintType) setSConv(options);
    resultProg = analyzeString(stri, int_options, libraryDirs, errorFile, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      logError(printf("prgStrParse(\"%s\"): analyzeString() failed:\n"
                      "int_options=" F_X(03) "\nerr_info=%d\n",
                      striAsUnquotedCStri(stri), int_options, err_info););
      raise_error(err_info);
    } /* if */
    logFunction(printf("prgStrParse --> " FMT_X_MEM " (error_count=%u)\n",
                       (memSizeType) resultProg,
                       resultProg != 0 ? resultProg->error_count : 0););
    return resultProg;
  } /* prgStrParse */



/**
 *  Determine object with 'syobjectName' from program 'aProgram'.
 *  @return a reference to the object, and
 *          NIL if no object 'syobjectName' exists.
 *  @exception MEMORY_ERROR If 'syobjectName' cannot be converted to
 *             the internal representation.
 */
objectType prgSyobject (const progType aProgram, const const_striType syobjectName)

  {
    cstriType name;
    identType ident_found;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* prgSyobject */
    logFunction(printf("prgSyobject(" FMT_X_MEM ", \"%s\")\n",
                       (memSizeType) aProgram,
                       striAsUnquotedCStri(syobjectName)););
    if (unlikely(aProgram == NULL)) {
      logError(printf("prgSyobject(" FMT_X_MEM ", \"%s\"): "
                      "Program empty.\n", (memSizeType) aProgram,
                      striAsUnquotedCStri(syobjectName)););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      name = stri_to_cstri8(syobjectName, &err_info);
      if (unlikely(name == NULL)) {
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
    } /* if */
    return result;
  } /* prgSyobject */



/**
 *  Determine the value of the system variable 'name' in 'aProgram'.
 *  @return a reference to the value of the system variable, and
 *          NIL if no system variable 'name' exists.
 */
objectType prgSysvar (const const_progType aProgram, const const_striType name)

  {
    int index_found;
    objectType result;

  /* prgSysvar */
    logFunction(printf("prgSysvar(" FMT_X_MEM ", \"%s\")\n",
                       (memSizeType) aProgram,
                       striAsUnquotedCStri(name)););
    if (unlikely(aProgram == NULL)) {
      logError(printf("prgSysvar(" FMT_X_MEM ", \"%s\"): "
                      "Program empty.\n", (memSizeType) aProgram,
                      striAsUnquotedCStri(name)););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      index_found = findSysvar(name);
      if (index_found != -1) {
        result = aProgram->sys_var[index_found];
      } else {
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* prgSysvar */
