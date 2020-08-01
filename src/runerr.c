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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Runtime                                                 */
/*  File: seed7/src/runerr.c                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Runtime error and exception handling procedures.       */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "listutl.h"
#include "sigutl.h"
#include "actutl.h"
#include "traceutl.h"
#include "identutl.h"
#include "infile.h"
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

#ifdef ANSI_C

void write_call_stack (const_listtype stack_elem)
#else

void write_call_stack (stack_elem)
listtype stack_elem;
#endif

  {
    objecttype func_object;

  /* write_call_stack */
    if (stack_elem != NULL) {
      write_call_stack(stack_elem->next);
      if (stack_elem->obj != NULL) {
        if (CATEGORY_OF_OBJ(stack_elem->obj) == CALLOBJECT ||
            CATEGORY_OF_OBJ(stack_elem->obj) == MATCHOBJECT) {
          func_object = stack_elem->obj->value.listvalue->obj;
        } else {
          func_object = stack_elem->obj;
        } /* if */
        if (HAS_DESCRIPTOR_ENTITY(func_object)) {
          if (func_object->descriptor.entity->ident != NULL) {
            printf("%s ",
                id_string(func_object->descriptor.entity->ident));
          } else if (func_object->descriptor.entity->name_list != NULL) {
            prot_list(func_object->descriptor.entity->name_list);
            printf(" ");
          } /* if */
        } /* if */
        if (HAS_POSINFO(stack_elem->obj)) {
          /*
          printf("\n");
          trace1(stack_elem->obj);
          printf("\n");
          trace1(func_object);
          printf("\n");
          */
          printf("at %s(%u)\n",
              file_name(GET_FILE_NUM(stack_elem->obj)),
              GET_LINE_NUM(stack_elem->obj));
        } else {
          printf("no POSINFO ");
          /* trace1(stack_elem->obj); */
          printf("\n");
        } /* if */
      } else {
        printf("NULL\n");
      } /* if */
    } /* if */
  } /* write_call_stack */



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
      printf(" raised");
      if (list == curr_argument_list) {
        if (curr_exec_object != NULL && HAS_POSINFO(curr_exec_object)) {
          printf(" at %s(%u)",
              file_name(GET_FILE_NUM(curr_exec_object)),
              GET_LINE_NUM(curr_exec_object));
        } /* if */
        printf(" with\n");
        prot_list(list);
        if (curr_exec_object != NULL) {
          curr_action_object = curr_exec_object->value.listvalue->obj;
        } /* if */
        if (curr_action_object->value.actvalue != NULL) {
          printf("\n*** ACTION \"%s\"\n",
              get_primact(curr_action_object->value.actvalue)->name);
        } /* if */
      } else {
        printf(" with\n");
        prot_list(list);
      } /* if */
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
      INIT_CATEGORY_OF_TEMP(exception, SYMBOLOBJECT);
      exception->value.intvalue = 0;
    } /* if */
    incl_list(&fail_stack, curr_exec_object, &err_info);
    if (!fail_flag || fail_value == NULL) {
      fail_value = exception;
      copy_list(list, &fail_expression, &err_info);
    } /* if */
    fail_flag = TRUE;
    return(NULL);
  } /* raise_with_arguments */



#ifdef ANSI_C

objecttype raise_exception (objecttype exception)
#else

objecttype raise_exception (exception)
objecttype exception;
#endif

  { /* raise_exception */
    return(raise_with_arguments(exception, curr_argument_list));
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



void run_error (objectcategory required, objecttype argument)
#else

void run_error (required, argument)
objectcategory required;
objecttype argument;
#endif

  { /* run_error */
    if (curr_exec_object != NULL) {
      curr_action_object = curr_exec_object->value.listvalue->obj;
    } /* if */
    printf("\n*** ACTION $");
    if (curr_action_object->value.actvalue != NULL) {
      printf("%s", get_primact(curr_action_object->value.actvalue)->name);
    } else {
      printf("NULL");
    } /* if */
    printf(" REQUIRES ");
    printcategory(required);
    printf(" NOT ");
    printcategory(CATEGORY_OF_OBJ(argument));
    printf("\n");
    trace1(argument);
    printf("\n");
    prot_list(curr_argument_list);
    continue_question();
    raise_error(RANGE_ERROR);
  } /* run_error */



#ifdef ANSI_C

void empty_value (objecttype argument)
#else

void empty_value (argument)
objecttype argument;
#endif

  { /* empty_value */
    if (curr_exec_object != NULL) {
      curr_action_object = curr_exec_object->value.listvalue->obj;
    } /* if */
    printf("\n*** ACTION $");
    if (curr_action_object->value.actvalue != NULL) {
      printf("%s", get_primact(curr_action_object->value.actvalue)->name);
    } else {
      printf("NULL");
    } /* if */
    printf(" WITH EMPTY VALUE\n");
    trace1(argument);
    printf("\nobject_ptr=%ld\n", (unsigned long) argument);
    prot_list(curr_argument_list);
    continue_question();
    raise_error(RANGE_ERROR);
  } /* empty_value */



#ifdef ANSI_C

void var_required (objecttype argument)
#else

void var_required (argument)
objecttype argument;
#endif

  { /* var_required */
    if (curr_exec_object != NULL) {
      curr_action_object = curr_exec_object->value.listvalue->obj;
    } /* if */
    printf("\n*** ACTION $");
    if (curr_action_object->value.actvalue != NULL) {
      printf("%s", get_primact(curr_action_object->value.actvalue)->name);
    } else {
      printf("NULL");
    } /* if */
    printf(" REQUIRES VARIABLE ");
    printcategory(CATEGORY_OF_OBJ(argument));
    printf(" NOT CONSTANT\n");
    trace1(argument);
    printf("\nobject_ptr=%ld\n", (unsigned long) argument);
    prot_list(curr_argument_list);
    continue_question();
    raise_error(RANGE_ERROR);
  } /* var_required */
