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
#include "datautl.h"
#include "listutl.h"
#include "sigutl.h"
#include "actutl.h"
#include "traceutl.h"
#include "infile.h"
#include "exec.h"
#include "rtl_err.h"

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
    if (ch == (int) '*') {
      shut_drivers();
      exit(1);
    } /* if */
    position = 0;
    while (ch >= (int) ' ' && ch <= (int) '~' && position < 9) {
      buffer[position] = (char) ch;
      position++;
      ch = fgetc(stdin);
    } /* while */
    buffer[position] = '\0';
    if (position > 0) {
      set_trace(buffer, position, NULL);
    } /* if */
    while (ch != EOF && ch != (int) '\n') {
      ch = fgetc(stdin);
    } /* while */
  } /* continue_question */



#ifdef ANSI_C

static void write_call_stack_element (const_listtype stack_elem)
#else

static void write_call_stack_element (stack_elem)
listtype stack_elem;
#endif

  {
    const_listtype position_stack_elem;
    objecttype func_object;

  /* write_call_stack_element */
    if (stack_elem->obj != NULL) {
      if (stack_elem->next != NULL) {
        if (CATEGORY_OF_OBJ(stack_elem->obj) == CALLOBJECT ||
            CATEGORY_OF_OBJ(stack_elem->obj) == MATCHOBJECT) {
          func_object = stack_elem->obj->value.listvalue->obj;
        } else {
          func_object = stack_elem->obj;
        } /* if */
        if (HAS_ENTITY(func_object)) {
          printf("in ");
          if (GET_ENTITY(func_object)->ident != NULL) {
            printf("%s ",
                id_string(GET_ENTITY(func_object)->ident));
          } else if (GET_ENTITY(func_object)->name_list != NULL) {
            prot_name(GET_ENTITY(func_object)->name_list);
            printf(" ");
          } /* if */
        } /* if */
        position_stack_elem = stack_elem->next;
        if (HAS_POSINFO(position_stack_elem->obj)) {
          /*
          printf("\n");
          trace1(position_stack_elem->obj);
          printf("\n");
          trace1(func_object);
          printf("\n");
          */
          printf("at %s(%u)\n",
              file_name(GET_FILE_NUM(position_stack_elem->obj)),
              GET_LINE_NUM(position_stack_elem->obj));
        } else if (HAS_PROPERTY(position_stack_elem->obj) &&
            position_stack_elem->obj->descriptor.property->line != 0) {
          printf("at %s(%u)\n",
              file_name(position_stack_elem->obj->descriptor.property->file_number),
              position_stack_elem->obj->descriptor.property->line);
        } else {
          printf("no POSITION INFORMATION ");
          /* trace1(position_stack_elem->obj); */
          printf("\n");
        } /* if */
      } /* if */
    } else {
      printf("NULL\n");
    } /* if */
  } /* write_call_stack_element */



#ifdef ANSI_C

void write_call_stack (const_listtype stack_elem)
#else

void write_call_stack (stack_elem)
listtype stack_elem;
#endif

  { /* write_call_stack */
    if (stack_elem != NULL) {
      write_call_stack(stack_elem->next);
      write_call_stack_element(stack_elem);
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
    if (list == curr_argument_list) {
      if (curr_exec_object != NULL && curr_exec_object->value.listvalue != NULL) {
        curr_action_object = curr_exec_object->value.listvalue->obj;
        incl_list(&fail_stack, curr_action_object, &err_info);
      } /* if */
    } /* if */
    if (trace.exceptions) {
      prot_nl();
      prot_cstri("*** EXCEPTION ");
      printobject(exception);
      printf(" raised");
      if (list == curr_argument_list) {
        if (curr_action_object != NULL) {
          if (HAS_POSINFO(curr_action_object)) {
            printf(" at %s(%u)",
                file_name(GET_FILE_NUM(curr_action_object)),
                GET_LINE_NUM(curr_action_object));
          } else if (HAS_PROPERTY(curr_action_object) &&
              curr_action_object->descriptor.property->line != 0) {
            printf(" at %s(%u)",
                file_name(curr_action_object->descriptor.property->file_number),
                curr_action_object->descriptor.property->line);
          } /* if */
        } /* if */
        printf("\n");
        prot_list(list);
        if (curr_exec_object != NULL) {
          if (HAS_POSINFO(curr_exec_object)) {
            printf(" at %s(%u)",
                file_name(GET_FILE_NUM(curr_exec_object)),
                GET_LINE_NUM(curr_exec_object));
          } else if (HAS_PROPERTY(curr_exec_object) &&
              curr_exec_object->descriptor.property->line != 0) {
            printf(" at %s(%u)",
                file_name(curr_exec_object->descriptor.property->file_number),
                curr_exec_object->descriptor.property->line);
          } /* if */
        } /* if */
        printf("\n");
        if (curr_action_object->value.actvalue != NULL) {
          printf("*** ACTION \"%s\"\n",
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
      exception->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(exception, SYMBOLOBJECT);
      exception->value.intvalue = 0;
    } /* if */
    incl_list(&fail_stack, curr_exec_object, &err_info);
    if (!fail_flag || fail_value == NULL) {
      fail_value = exception;
      fail_expression = copy_list(list, &err_info);
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

void raise_error3 (int exception_num, const_cstritype filename, int line)
#else

void raise_error3 (exception_num, filename, line)
int exception_num;
char *filename;
int line;
#endif

  { /* raise_error3 */
    raise_exception(prog.sys_var[exception_num]);
  } /* raise_error3 */



#ifdef ANSI_C

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
