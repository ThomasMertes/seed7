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

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "flistutl.h"
#include "striutl.h"
#include "listutl.h"
#include "entutl.h"
#include "syvarutl.h"
#include "identutl.h"
#include "traceutl.h"
#include "analyze.h"
#include "name.h"
#include "exec.h"
#include "match.h"
#include "objutl.h"
#include "runerr.h"
#include "set_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "prg_comp.h"

#undef TRACE_PRG_COMP



#ifdef ANSI_C

static objecttype copy_args (const const_rtlArraytype argv, const memsizetype start)
#else

static objecttype copy_args (argv, start)
rtlArraytype argv;
memsizetype start;
#endif

  {
    memsizetype argc;
    arraytype arg_array;
    memsizetype arg_idx;
    objecttype result;

  /* copy_args */
    /* printf("start = %d\n", start); */
    if (argv == NULL || argv->max_position < 0) {
      argc = 0;
    } else {
      argc = (memsizetype) argv->max_position - start;
    } /* if */
    /* printf("argc = %d\n", argc); */
    if (ALLOC_ARRAY(arg_array, argc)) {
      arg_idx = 0;
      while (arg_idx < argc) {
        /* printf("arg_idx = %d\n", arg_idx);
           printf("argv[%d] = ", start + arg_idx);
           prot_stri(argv->arr[start + arg_idx].value.strivalue);
           printf("\n"); */
        arg_array->arr[arg_idx].type_of = take_type(SYS_STRI_TYPE);
        arg_array->arr[arg_idx].descriptor.property = NULL;
        arg_array->arr[arg_idx].value.strivalue =
            argv->arr[start + arg_idx].value.strivalue;
        INIT_CATEGORY_OF_OBJ(&arg_array->arr[arg_idx], STRIOBJECT);
        arg_idx++;
      } /* while */
      arg_array->min_position = 1;
      arg_array->max_position = (inttype) arg_idx;
    } /* if */
    if (arg_array != NULL) {
      if (ALLOC_OBJECT(result)) {
        result->type_of = NULL;
        result->descriptor.property = NULL;
        INIT_CATEGORY_OF_OBJ(result, ARRAYOBJECT);
        result->value.arrayvalue = arg_array;
      } else {
        FREE_ARRAY(arg_array, argc);
      } /* if */
    } else {
      result = NULL;
    } /* if */
    return result;
  } /* copy_args */



#ifdef ANSI_C

static void free_args (objecttype arg_v)
#else

static void free_args (arg_v)
objecttype arg_v;
#endif

  {
    arraytype arg_array;

  /* free_args */
    arg_array = arg_v->value.arrayvalue;
    FREE_ARRAY(arg_array, (uinttype) (arg_array->max_position - arg_array->min_position + 1));
    FREE_OBJECT(arg_v);
  } /* free_args */



#ifdef ANSI_C

void interpret (const const_progtype currentProg, const const_rtlArraytype argv,
                memsizetype argv_start, uinttype options, const const_stritype prot_file_name)
#else

void interpret (currentProg, argv, argv_start, options, prot_file_name)
progtype currentProg;
rtlArraytype argv;
memsizetype argv_start;
uinttype options;
stritype prot_file_name;
#endif

  {
    progrecord prog_backup;

  /* interpret */
#ifdef TRACE_PRG_COMP
    printf("BEGIN interpret\n");
#endif
    if (currentProg != NULL) {
      fail_flag = FALSE;
      fail_value = (objecttype) NULL;
      fail_expression = (listtype) NULL;
      fail_stack = NULL;
      if (currentProg->main_object != NULL) {
        memcpy(&prog_backup, &prog, sizeof(progrecord));
        memcpy(&prog, currentProg, sizeof(progrecord));
        prog.option_flags = options;
        set_trace(prog.option_flags);
        set_protfile_name(prot_file_name);
        prog.arg_v = copy_args(argv, argv_start);
        if (prog.arg_v == NULL) {
          raise_error(MEMORY_ERROR);
        } else {
          /* printf("main defined as: ");
          trace1(prog.main_object);
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
          exec_call(prog.main_object);
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
          free_args(prog.arg_v);
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
          memcpy(&prog, &prog_backup, sizeof(progrecord));
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_PRG_COMP
    printf("END interpr\n");
#endif
  } /* interpret */



#ifdef ANSI_C

void prgCpy (progtype *const dest, const progtype source)
#else

void prgCpy (dest, source)
progtype *dest;
progtype source;
#endif

  {
    progtype old_prog;

  /* prgCpy */
    old_prog = *dest;
    if (old_prog != source) {
      prgDestr(old_prog);
      *dest = source;
      if (source != NULL) {
        source->usage_count++;
      } /* if */
    } /* if */
    /* printf("prgCpy: usage_count=%d\n", (*dest)->usage_count); */
  } /* prgCpy */



#ifdef ANSI_C

listtype prgDeclObjects (const const_progtype aProg)
#else

listtype prgDeclObjects (aProg)
progtype aProg;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* prgDeclObjects */
    if (aProg->stack_current != NULL) {
      result = copy_list(aProg->stack_current->local_object_list, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } else {
      result = NULL;
    } /* if */
    return result;
  } /* prgDeclObjects */



#ifdef ANSI_C

void prgDestr (progtype old_prog)
#else

void prgDestr (old_prog)
progtype old_prog;
#endif

  {
    progrecord prog_backup;

  /* prgDestr */
    if (old_prog != NULL) {
      /* printf("prgDestr: usage_count=%d\n", old_prog->usage_count); */
      old_prog->usage_count--;
      if (old_prog->usage_count == 0) {
        /* printf("prgDestr: old progrecord: %lx\n", old_prog); */
        memcpy(&prog_backup, &prog, sizeof(progrecord));
        memcpy(&prog, old_prog, sizeof(progrecord));
        /* printf("heapsize: %ld\n", heapsize()); */
        /* heap_statistic(); */
        close_stack(old_prog);
        close_declaration_root(old_prog);
        close_entity(old_prog);
        close_idents(old_prog);
        free_entity(old_prog, old_prog->entity.literal);
        FREE_RECORD(old_prog->property.literal, propertyrecord, count.property);
        memcpy(&prog, &prog_backup, sizeof(progrecord));
        FREE_STRI(old_prog->arg0, old_prog->arg0->size);
        FREE_STRI(old_prog->program_name, old_prog->program_name->size);
        FREE_STRI(old_prog->program_path, old_prog->program_path->size);
        FREE_RECORD(old_prog->stack_global, stackrecord, count.stack);
        FREE_RECORD(old_prog, progrecord, count.prog);
        /* printf("heapsize: %ld\n", heapsize()); */
        /* heap_statistic(); */
      } /* if */
    } /* if */
  } /* prgDestr */



#ifdef ANSI_C

inttype prgErrorCount (const const_progtype aProg)
#else

inttype prgErrorCount (aProg)
progtype aProg;
#endif

  {
    inttype result;

  /* prgErrorCount */
    if (aProg->error_count > INTTYPE_MAX) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = (inttype) aProg->error_count;
    } /* if */
    return result;
  } /* prgErrorCount */



#ifdef ANSI_C

objecttype prgEval (progtype currentProg, objecttype object)
#else

objecttype prgEval (currentProg, object)
progtype currentProg;
objecttype object;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    objecttype result;

  /* prgEval */
    result = exec_expr(currentProg, object, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
      result = NULL;
    } /* if */
    return result;
  } /* prgEval */



#ifdef ANSI_C

void prgExec (const const_progtype currentProg, const const_rtlArraytype argv,
    const const_settype options, const const_stritype prot_file_name)
#else

void prgExec (currentProg, argv, options, prot_file_name)
progtype currentProg;
rtlArraytype argv;
settype options;
stritype prot_file_name;
#endif

  {
    uinttype int_options;

  /* prgExec */
    int_options = (uinttype) setSConv(options);
    interpret(currentProg, argv, 0, int_options, prot_file_name);
    fail_flag = FALSE;
    fail_value = (objecttype) NULL;
    fail_expression = (listtype) NULL;
  } /* prgExec */



#ifdef ANSI_C

progtype prgFilParse (const const_stritype fileName, const const_settype options,
    const const_rtlArraytype libraryDirs, const const_stritype prot_file_name)
#else

progtype prgFilParse (fileName, options, libraryDirs, prot_file_name)
stritype fileName;
settype options;
rtlArraytype libraryDirs;
stritype prot_file_name;
#endif

  {
    uinttype int_options;
    errinfotype err_info = OKAY_NO_ERROR;
    progtype result;

  /* prgFilParse */
    /* printf("prgFilParse(");
       prot_stri(fileName);
       printf(")\n"); */
    int_options = (uinttype) setSConv(options);
    /* printf("options: %03x\n", int_options); */
    result = analyze_file(fileName, int_options, libraryDirs, prot_file_name, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
    } /* if */
    return result;
  } /* prgFilParse */



#ifdef ANSI_C

objecttype prgMatch (const const_progtype aProg, listtype curr_expr)
#else

objecttype prgMatch (aProg, curr_expr)
progtype aProg;
listtype curr_expr;
#endif

  {
    objectrecord expr_object;
    objecttype result;

  /* prgMatch */
    /* prot_list(curr_expr);
    printf("\n"); */
    expr_object.type_of = NULL;
    expr_object.descriptor.property = NULL;
    expr_object.value.listvalue = curr_expr;
    INIT_CATEGORY_OF_OBJ(&expr_object, EXPROBJECT);

    result = match_prog_expression(aProg->declaration_root, &expr_object);
    if (result != NULL) {
      if (CATEGORY_OF_OBJ(result) == MATCHOBJECT ||
          CATEGORY_OF_OBJ(result) == CALLOBJECT) {
        curr_expr = expr_object.value.listvalue->next;
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
    prot_list(curr_expr);
    printf("\n"); */
    return result;
  } /* prgMatch */



#ifdef ANSI_C

objecttype prgMatchExpr (const const_progtype aProg, listtype curr_expr)
#else

objecttype prgMatchExpr (aProg, curr_expr)
progtype aProg;
listtype curr_expr;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    objecttype result;

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
      result->value.listvalue = copy_list(curr_expr, &err_info);
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
        prot_list(result->value.listvalue);
        printf("\n"); */
      } /* if */
    } /* if */
    return result;
  } /* prgMatchExpr */



#ifdef ANSI_C

progtype prgStrParse (const const_stritype stri, const const_settype options,
    const const_rtlArraytype libraryDirs, const const_stritype prot_file_name)
#else

progtype prgStrParse (stri)
stritype stri;
#endif

  {
    uinttype int_options;
    errinfotype err_info = OKAY_NO_ERROR;
    progtype result;

  /* prgStrParse */
    int_options = (uinttype) setSConv(options);
    result = analyze_string(stri, int_options, libraryDirs, prot_file_name, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
    } /* if */
    return result;
  } /* prgStrParse */



#ifdef ANSI_C

objecttype prgSyobject (const progtype aProg, const const_stritype syobjectName)
#else

objecttype prgSyobject (aProg, syobjectName)
progtype aProg;
stritype syobjectName;
#endif

  {
    cstritype name;
    progrecord prog_backup;
    identtype ident_found;
    objecttype result;

  /* prgSyobject */
    name = cp_to_cstri8(syobjectName);
    if (name == NULL) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      memcpy(&prog_backup, &prog, sizeof(progrecord));
      memcpy(&prog, aProg, sizeof(progrecord));
      ident_found = get_ident((const_ustritype) name);
      if (ident_found == NULL ||
          ident_found->entity == NULL) {
        result = NULL;
      } else {
        result = ident_found->entity->syobject;
      } /* if */
      memcpy(&prog, &prog_backup, sizeof(progrecord));
      free_cstri(name, syobjectName);
    } /* if */
    return result;
  } /* prgSyobject */



#ifdef ANSI_C

objecttype prgSysvar (const const_progtype aProg, const const_stritype sysvarName)
#else

objecttype prgSysvar (aProg, sysvarName)
progtype aProg;
stritype sysvarName;
#endif

  {
    int index_found;
    objecttype result;

  /* prgSysvar */
    index_found = find_sysvar(sysvarName);
    if (index_found != -1) {
      result = aProg->sys_var[index_found];
    } else {
      result = NULL;
    } /* if */
    return result;
  } /* prgSysvar */
