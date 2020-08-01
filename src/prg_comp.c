/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2008  Thomas Mertes                        */
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
/*  Changes: 1991, 1992, 1993, 1994, 2008  Thomas Mertes            */
/*  Content: Primitive actions for the program type.                */
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
#include "striutl.h"
#include "listutl.h"
#include "option.h"
#include "syvarutl.h"
#include "identutl.h"
#include "analyze.h"
#include "exec.h"
#include "match.h"
#include "runerr.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "prg_comp.h"



#ifdef ANSI_C

void prgCpy (progtype *dest, progtype source)
#else

void prgCpy (dest, source)
progtype *dest;
progtype source;
#endif

  {
    progtype old_prog;

  /* prgCpy */
    old_prog = *dest;
    if (old_prog != NULL) {
      old_prog->usage_count--;
      if (old_prog->usage_count == 0) {
        FREE_RECORD(old_prog, progrecord, count.prog);
      } /* if */
    } /* if */
    *dest = source;
    if (source != NULL) {
      source->usage_count++;
    } /* if */
  } /* prgCpy */



#ifdef ANSI_C

listtype prgDeclObjects (progtype aProg)
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
    return(result);
  } /* prgDeclObjects */



#ifdef ANSI_C

void prgDestr (progtype old_prog)
#else

void prgDestr (old_prog)
progtype old_prog;
#endif

  { /* prgDestr */
    if (old_prog != NULL) {
      old_prog->usage_count--;
      if (old_prog->usage_count == 0) {
        FREE_RECORD(old_prog, progrecord, count.prog);
      } /* if */
    } /* if */
  } /* prgDestr */



#ifdef ANSI_C

inttype prgErrorCount (progtype aProg)
#else

inttype prgErrorCount (aProg)
progtype aProg;
#endif

  { /* prgErrorCount */
    return(aProg->error_count);
  } /* prgErrorCount */



#ifdef ANSI_C

objecttype prgEval (progtype currentProg, objecttype object)
#else

objecttype prgEval (currentProg, object)
progtype currentProg;
objecttype object;
#endif

  {
    objecttype result;

  /* prgEval */
    result = exec_expr(currentProg, object);
    fail_flag = FALSE;
    fail_value = (objecttype) NULL;
    fail_expression = (listtype) NULL;
    return(result);
  } /* prgEval */



#ifdef ANSI_C

void prgExec (progtype currentProg)
#else

void prgExec (currentProg)
progtype currentProg;
#endif

  { /* prgExec */
    interpr(currentProg);
    fail_flag = FALSE;
    fail_value = (objecttype) NULL;
    fail_expression = (listtype) NULL;
  } /* prgExec */



#ifdef ANSI_C

progtype prgFilParse (stritype stri)
#else

progtype prgFilParse (stri)
stritype stri;
#endif

  {
    ustritype file_name;
    progtype result;

  /* prgFilParse */
    file_name = cp_to_cstri(stri);
    if (file_name == NULL) {
      free_cstri(file_name, stri);
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      option.source_file_name = file_name;
      result = analyze(file_name);
      /* ?? set_trace(option.exec_trace_level, -1, NULL); */
    } /* if */
    return(result);
  } /* prgFilParse */



#ifdef ANSI_C

objecttype prgMatch (progtype currentProg, listtype curr_expr)
#else

objecttype prgMatch (currentProg, curr_expr)
progtype currentProg;
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

    result = match_prog_expression(currentProg->declaration_root, &expr_object);
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
    return(result);
  } /* prgMatch */



#ifdef ANSI_C

objecttype prgMatchExpr (progtype currentProg, listtype curr_expr)
#else

objecttype prgMatchExpr (currentProg, curr_expr)
progtype currentProg;
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
        result = match_prog_expression(currentProg->declaration_root, result);
        /* printf("result == %lx\n", result);
        trace1(result);
        printf("\n");
        prot_list(curr_expr);
        printf("\n");
        prot_list(result->value.listvalue);
        printf("\n"); */
      } /* if */
    } /* if */
    return(result);
  } /* prgMatchExpr */



#ifdef ANSI_C

progtype prgStrParse (stritype stri)
#else

progtype prgStrParse (stri)
stritype stri;
#endif

  {
    progtype result;

  /* prgStrParse */
    option.source_file_name = "STRING";
    result = analyze_string(stri);
    /* ?? set_trace(option.exec_trace_level, -1, NULL); */
    return(result);
  } /* prgStrParse */



#ifdef ANSI_C

objecttype prgSyobject (progtype currentProg, stritype stri1)
#else

objecttype prgSyobject (currentProg, stri1)
progtype currentProg;
stritype stri1;
#endif

  {
    ustritype name;
    progrecord prog_backup;
    identtype ident_found;
    objecttype result;

  /* prgSyobject */
    name = (ustritype) cp_to_cstri(stri1);
    if (name == NULL) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      memcpy(&prog_backup, &prog, sizeof(progrecord));
      memcpy(&prog, currentProg, sizeof(progrecord));
      ident_found = get_ident(name, strlen(name));
      if (ident_found == NULL ||
          ident_found->entity == NULL) {
        result = NULL;
      } else {
        result = ident_found->entity->syobject;
      } /* if */
      memcpy(&prog, &prog_backup, sizeof(progrecord));
      free_cstri(name, stri1);
    } /* if */
    return(result);
  } /* prgSyobject */



#ifdef ANSI_C

objecttype prgSysvar (progtype aProg, stritype sysvarName)
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
    return(result);
  } /* prgSysvar */
