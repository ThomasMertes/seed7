/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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
/*  Module: Interpreter                                             */
/*  File: seed7/src/doany.c                                         */
/*  Changes: 1993, 1994, 2015  Thomas Mertes                        */
/*  Content: Procedures to call several Seed7 functions from C.     */
/*                                                                  */
/*  This File contains a set of do_.. functions that allow to       */
/*  call several Seed7-expressions from C. The expressions are      */
/*  interpreted with the same mechanism that is used to execute     */
/*  the MAIN program at runtime. The procedures are used during     */
/*  the analysation phase and during the execution phase.           */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "common.h"
#include "sigutl.h"
#include "data.h"
#include "heaputl.h"
#include "striutl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "traceutl.h"
#include "objutl.h"
#include "runerr.h"
#include "exec.h"
#include "match.h"

#undef EXTERN
#define EXTERN
#include "doany.h"


#undef TRACE_DOANY


static listRecord flush_expr[2];
static listRecord wrnl_expr[2];
static listRecord wrstri_expr[3];



objectType exec1 (listType list)

  {
    objectRecord expr_object;
    objectType object;
    objectType result;

  /* exec1 */
#ifdef TRACE_EXEC
    printf("BEGIN exec1\n");
#endif
#ifdef DEBUG_EXEC
    printf("before matching\n");
    trace1(list->obj);
    printf("\n");
    trace1(list->next->obj);
    printf("\n");
    trace1(list->next->next->obj);
    printf("\n");
#endif
    /* printf(".");
       fflush(stdout); */
    expr_object.type_of = take_type(SYS_EXPR_TYPE);
    expr_object.descriptor.property = NULL;
    expr_object.value.listValue = list;
    INIT_CATEGORY_OF_OBJ(&expr_object, EXPROBJECT);
    if ((object = match_expression(&expr_object)) != NULL) {
#ifdef DEBUG_EXEC
      printf("before executing\n");
      trace1(object);
      printf("\n");
      trace1(list->obj);
      printf("\n");
      trace1(list->next->obj);
      printf("\n");
      trace1(list->next->next->obj);
      printf("\n");
      if (CATEGORY_OF_OBJ(list->obj) == CALLOBJECT) {
        trace1(list->obj);
        printf("\n");
      } /* if */
#endif
      result = exec_call(object);
      FREE_L_ELEM(object->value.listValue);
      /* FREE_OBJECT(object) is not necessary, */
      /* because object == &expr_object holds. */
    } else {
      result = NULL;
    } /* if */
#ifdef DEBUG_EXEC
    printf("after executing\n");
    trace1(object);
    printf("\n");
    trace1(list->obj);
    printf("\n");
    trace1(list->next->obj);
    printf("\n");
    trace1(list->next->next->obj);
    printf("\n");
#endif
#ifdef TRACE_EXEC
    printf("END exec1\n");
#endif
    return result;
  } /* exec1 */



boolType do_flush (objectType outfile)

  {
    progType outfileProg;
    progRecord prog_backup;
    boolType result;

  /* do_flush */
#ifdef TRACE_DOANY
    printf("BEGIN do_flush\n");
#endif
    outfileProg = outfile->type_of->owningProg;
    memcpy(&prog_backup, &prog, sizeof(progRecord));
    memcpy(&prog, outfileProg, sizeof(progRecord));
    flush_expr[0].obj = outfile;
    flush_expr[1].obj = SYS_FLUSH_OBJECT;
    result = (boolType) (exec1(flush_expr) == SYS_EMPTY_OBJECT);
    set_fail_flag(FALSE);
    memcpy(&prog, &prog_backup, sizeof(progRecord));
#ifdef TRACE_DOANY
    printf("END do_flush\n");
#endif
    return result;
  } /* do_flush */



boolType do_wrnl (objectType outfile)

  {
    progType outfileProg;
    progRecord prog_backup;
    boolType result;

  /* do_wrnl */
#ifdef TRACE_DOANY
    printf("BEGIN do_wrnl\n");
#endif
    outfileProg = outfile->type_of->owningProg;
    memcpy(&prog_backup, &prog, sizeof(progRecord));
    memcpy(&prog, outfileProg, sizeof(progRecord));
    wrnl_expr[0].obj = outfile;
    wrnl_expr[1].obj = SYS_WRLN_OBJECT;
    result = (boolType) (exec1(wrnl_expr) == SYS_EMPTY_OBJECT);
    set_fail_flag(FALSE);
    memcpy(&prog, &prog_backup, sizeof(progRecord));
#ifdef TRACE_DOANY
    printf("END do_wrnl\n");
#endif
    return result;
  } /* do_wrnl */



boolType do_wrstri (objectType outfile, striType stri)

  {
    progType outfileProg;
    progRecord prog_backup;
    objectType out_stri;
    boolType result;

  /* do_wrstri */
#ifdef TRACE_DOANY
    printf("BEGIN do_wrstri\n");
#endif
    outfileProg = outfile->type_of->owningProg;
    memcpy(&prog_backup, &prog, sizeof(progRecord));
    memcpy(&prog, outfileProg, sizeof(progRecord));
    result = FALSE;
    if ((out_stri = bld_stri_temp(stri)) != SYS_MEM_EXCEPTION) {
      wrstri_expr[0].obj = outfile;
      wrstri_expr[1].obj = out_stri;
      wrstri_expr[2].obj = SYS_WRITE_OBJECT;
      result = (boolType) (exec1(wrstri_expr) == SYS_EMPTY_OBJECT);
      set_fail_flag(FALSE);
      FREE_OBJECT(out_stri);
    } /* if */
    set_fail_flag(FALSE);
    memcpy(&prog, &prog_backup, sizeof(progRecord));
#ifdef TRACE_DOANY
    printf("END do_wrstri\n");
#endif
    return result;
  } /* do_wrstri */



boolType do_wrcstri (objectType outfile, const_cstriType stri)

  {
    striType out_stri;
    boolType result;

  /* do_wrcstri */
#ifdef TRACE_DOANY
    printf("BEGIN do_wrcstri\n");
#endif
    result = FALSE;
    out_stri = cstri_to_stri(stri);
    if (out_stri != NULL) {
      result = do_wrstri(outfile, out_stri);
      FREE_STRI(out_stri, out_stri->size);
    } /* if */
#ifdef TRACE_DOANY
    printf("END do_wrcstri\n");
#endif
    return result;
  } /* do_wrcstri */



void init_do_any (void)

  { /* init_do_any */
#ifdef TRACE_DOANY
    printf("BEGIN init_do_any\n");
#endif
    flush_expr[0].next = &flush_expr[1];
    flush_expr[1].next = NULL;
    wrnl_expr[0].next = &wrnl_expr[1];
    wrnl_expr[1].next = NULL;
    wrstri_expr[0].next = &wrstri_expr[1];
    wrstri_expr[1].next = &wrstri_expr[2];
    wrstri_expr[2].next = NULL;
#ifdef TRACE_DOANY
    printf("END init_do_any\n");
#endif
  } /* init_do_any */
