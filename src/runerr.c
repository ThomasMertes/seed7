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
/*  Module: Runtime                                                 */
/*  File: seed7/src/runerr.c                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Runtime error and exception handling procedures.       */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "listutl.h"
#include "sigutl.h"
#include "actutl.h"
#include "traceutl.h"
#include "exec.h"

#undef EXTERN
#define EXTERN
#include "runerr.h"



#ifdef ANSI_C

void continue_question (void)
#else

void continue_question ()
#endif

  {
    int ch;
    int position;
    char buffer[10];

  /* continue_question */
    printf("\n*** (Type RETURN to continue or '*' to terminate)\n");
    ch = fgetc(stdin);
    if (ch == '*') {
      shut_drivers();
      exit(1);
    } /* if */
    position = 0;
    while (ch >= ' ' && ch <= '~' && position < 9) {
      buffer[position] = (char) ch;
      position++;
      ch = fgetc(stdin);
    } /* while */
    buffer[position] = '\0';
    if (position > 0) {
      set_trace(buffer, position, NULL);
    } /* if */
    while (ch != EOF && ch != '\n') {
      ch = fgetc(stdin);
    } /* while */
  } /* continue_question */



#ifdef ANSI_C

void run_error (objectclass required, objecttype argument)
#else

void run_error (required, argument)
objectclass required;
objecttype argument;
#endif

  { /* run_error */
    printf("\n*** ACTION $");
    if (curr_exec_object->value.actvalue != NULL) {
      printf("%s", get_primact(curr_exec_object->value.actvalue)->name);
    } else {
      printf("NULL");
    } /* if */
    printf(" REQUIRES ");
    printclass(required);
    printf(" NOT ");
    printclass(CLASS_OF_OBJ(argument));
    printf("\n");
    trace1(argument);
    printf("\n");
    prot_list(curr_agument_list);
    continue_question();
  } /* run_error */



#ifdef ANSI_C

void empty_value (objecttype argument)
#else

void empty_value (argument)
objecttype argument;
#endif

  { /* empty_value */
    printf("\n*** ACTION $");
    if (curr_exec_object->value.actvalue != NULL) {
      printf("%s", get_primact(curr_exec_object->value.actvalue)->name);
    } else {
      printf("NULL");
    } /* if */
    printf(" WITH EMPTY VALUE\n");
    trace1(argument);
    printf("\nobject_ptr=%ld\n", (unsigned long) argument);
    prot_list(curr_agument_list);
    continue_question();
  } /* empty_value */



#ifdef ANSI_C

void var_required (objecttype argument)
#else

void var_required (argument)
objecttype argument;
#endif

  { /* var_required */
    printf("\n*** ACTION $");
    if (curr_exec_object->value.actvalue != NULL) {
      printf("%s", get_primact(curr_exec_object->value.actvalue)->name);
    } else {
      printf("NULL");
    } /* if */
    printf(" REQUIRES VARIABLE ");
    printclass(CLASS_OF_OBJ(argument));
    printf(" NOT CONSTANT\n");
    trace1(argument);
    printf("\nobject_ptr=%ld\n", (unsigned long) argument);
    prot_list(curr_agument_list);
    continue_question();
  } /* var_required */



#ifdef ANSI_C

objecttype raise_with_arguments (objecttype exception, listtype list)
#else

objecttype raise_with_arguments (exception, list)
objecttype exception;
listtype list;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;

  /* raise_with_arguments */
#ifdef WITH_PROTOCOL
    if (trace.exceptions) {
      prot_nl();
      prot_cstri("*** EXCEPTION ");
      printobject(exception);
      printf(" raised with\n");
      prot_list(list);
      continue_question();
    } /* if */
#endif
#ifndef USE_CHUNK_ALLOCS
    if (exception == MEM_EXCEPTION) {
      reuse_free_lists();
    } /* if */
#endif
    if (exception == NULL) {
      ALLOC_OBJECT(exception);
      exception->type_of = NULL;
      exception->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(exception, SYMBOLOBJECT);
      exception->value.intvalue = 0;
    } /* if */
    fail_flag = TRUE;
    if (fail_value == NULL) {
      fail_value = exception;
      copy_list(list, &fail_expression, &err_info);
    } /* if */
    return(NULL);
  } /* raise_with_arguments */



#ifdef ANSI_C

objecttype raise_exception (objecttype exception)
#else

objecttype raise_exception (exception)
objecttype exception;
#endif

  { /* raise_exception */
    return(raise_with_arguments(exception, curr_agument_list));
  } /* raise_exception */



#ifdef ANSI_C

void raise_error (int exception_num)
#else

void raise_error (exception_num)
int exception_num;
#endif

  { /* raise_error */
    raise_exception(prog.sys_var[exception_num]);
  } /* raise_error */
