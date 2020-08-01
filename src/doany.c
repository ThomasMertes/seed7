/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Interpreter                                             */
/*  File: seed7/src/doany.c                                         */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Procedures to call several Seed7 functions from C.     */
/*                                                                  */
/*  This File contains a set of do_.. functions that allow to       */
/*  call several Seed7-expressions from C. The expressions are      */
/*  interpreted with the same mechanism that is used to execute     */
/*  the MAIN program at runtime. The procedures are used during     */
/*  the analysation phase and during the execution phase.           */
/*                                                                  */
/********************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "traceutl.h"
#include "runerr.h"
#include "exec.h"
#include "memory.h"
#include "match.h"

#undef EXTERN
#define EXTERN
#include "doany.h"


#undef TRACE_DOANY


static listrecord flush_expr[2];
static listrecord wrnl_expr[2];
static listrecord wrstri_expr[3];



#ifdef ANSI_C

objecttype exec1 (listtype list)
#else

objecttype exec1 (listtype)
listtype list;
#endif

  {
    objectrecord expr_object;
    objecttype object;
    objecttype result;

  /* exec1 */
#ifdef TRACE_EXEC
    printf("BEGIN exec1\n");
#endif
#ifdef DEBUG_EXEC
    printf("before matching\n");
    trace1(list->OBJ);
    printf("\n");
    trace1(list->next->OBJ);
    printf("\n");
    trace1(list->next->next->OBJ);
    printf("\n");
#endif
    /* printf(".");
       fflush(stdout); */
    expr_object.type_of = take_type(SYS_EXPR_TYPE);
    expr_object.descriptor.entity = NULL;
    expr_object.value.listvalue = list;
    INIT_CLASS_OF_OBJ(&expr_object, EXPROBJECT);
    if ((object = match_expression(&expr_object)) != NULL) {
#ifdef DEBUG_EXEC
    printf("before executing\n");
    trace1(list->OBJ);
    printf("\n");
    trace1(list->next->OBJ);
    printf("\n");
    trace1(list->next->next->OBJ);
    printf("\n");
#endif
      result = exec_call(object);
    } else {
      result = NULL;
    } /* if */
#ifdef DEBUG_EXEC
    printf("after executing\n");
    trace1(list->OBJ);
    printf("\n");
    trace1(list->next->OBJ);
    printf("\n");
    trace1(list->next->next->OBJ);
    printf("\n");
#endif
#ifdef TRACE_EXEC
    printf("END exec1\n");
#endif
    return(result);
  } /* exec1 */



#ifdef ANSI_C

booltype do_flush (objecttype outfile)
#else

booltype do_flush (outfile)
objecttype outfile;
#endif

  {
    booltype result;

  /* do_flush */
#ifdef TRACE_DOANY
    printf("BEGIN do_flush\n");
#endif
    flush_expr[0].obj = outfile;
    flush_expr[1].obj = SYS_FLUSH_OBJECT;
    result = (booltype) (exec1(flush_expr) == SYS_EMPTY_OBJECT);
    fail_flag = FALSE;
#ifdef TRACE_DOANY
    printf("END do_flush\n");
#endif
    return(result);
  } /* do_flush */



#ifdef ANSI_C

booltype do_wrnl (objecttype outfile)
#else

booltype do_wrnl (outfile)
objecttype outfile;
#endif

  {
    booltype result;

  /* do_wrnl */
#ifdef TRACE_DOANY
    printf("BEGIN do_wrnl\n");
#endif
    wrnl_expr[0].obj = outfile;
    wrnl_expr[1].obj = SYS_WRLN_OBJECT;
    result = (booltype) (exec1(wrnl_expr) == SYS_EMPTY_OBJECT);
    fail_flag = FALSE;
#ifdef TRACE_DOANY
    printf("END do_wrnl\n");
#endif
    return(result);
  } /* do_wrnl */



#ifdef ANSI_C

booltype do_wrstri (objecttype outfile, stritype stri)
#else

booltype do_wrstri (outfile, stri)
objecttype outfile;
stritype stri;
#endif

  {
    objecttype out_stri;
    booltype result;

  /* do_wrstri */
#ifdef TRACE_DOANY
    printf("BEGIN do_wrstri\n");
#endif
    result = FALSE;
    if ((out_stri = bld_stri_temp(stri)) != SYS_MEM_EXCEPTION) {
      wrstri_expr[0].obj = outfile;
      wrstri_expr[1].obj = out_stri;
      wrstri_expr[2].obj = SYS_WRITE_OBJECT;
      result = (booltype) (exec1(wrstri_expr) == SYS_EMPTY_OBJECT);
      fail_flag = FALSE;
      FREE_OBJECT(out_stri);
    } /* if */
    fail_flag = FALSE;
#ifdef TRACE_DOANY
    printf("BEGIN do_wrstri\n");
#endif
    return(result);
  } /* do_wrstri */



#ifdef ANSI_C

booltype do_wrcstri (objecttype outfile, cstritype stri)
#else

booltype do_wrcstri (outfile, stri)
objecttype outfile;
cstritype stri;
#endif

  {
    memsizetype LEN;
    stritype out_stri;
    booltype result;

  /* do_wrcstri */
#ifdef TRACE_DOANY
    printf("BEGIN do_wrcstri\n");
#endif
    result = FALSE;
    LEN = (memsizetype) strlen(stri);
    if (ALLOC_STRI(out_stri, LEN)) {
      COUNT_STRI(LEN);
      out_stri->size = LEN;
      memcpy(out_stri->mem, stri, (SIZE_TYPE) LEN);
      result = do_wrstri(outfile, out_stri);
      FREE_STRI(out_stri, out_stri->size);
    } /* if */
#ifdef TRACE_DOANY
    printf("END do_wrcstri\n");
#endif
    return(result);
  } /* do_wrcstri */



#ifdef ANSI_C

void init_do_any (void)
#else

void init_do_any ()
#endif

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
