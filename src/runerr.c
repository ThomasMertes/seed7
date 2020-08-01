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
/*  Module: Runtime                                                 */
/*  File: seed7/src/runerr.c                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Runtime error and exception handling procedures.       */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "signal.h"
#include "setjmp.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
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


static long_jump_position sigsegv_occurred;



void continue_question (void)

  {
    int ch;
    int position;
    char buffer[10];
    long unsigned int exception_num;

  /* continue_question */
    printf("\n*** The following commands are possible:\n"
           "  RETURN  Continue\n"
           "  *       Terminate\n"
           "  /       Trigger SIGFPE\n"
           "  !n      Raise exception with number (e.g.: !1 raises MEMORY_ERROR)\n");
    ch = fgetc(stdin);
    if (ch == (int) '*') {
      shut_drivers();
      exit(1);
    } else if (ch == (int) '/') {
      /* signal(SIGFPE, SIG_DFL); */
      position = 0;
#ifdef DO_SIGFPE_WITH_DIV_BY_ZERO
      printf("%d", 1 / position); /* trigger SIGFPE on purpose */
#else
      raise(SIGFPE);
#endif
      printf("\n*** Raising SIGFPE failed.\n");
    } /* if */
    position = 0;
    while (ch >= (int) ' ' && ch <= (int) '~' && position < 9) {
      buffer[position] = (char) ch;
      position++;
      ch = fgetc(stdin);
    } /* while */
    buffer[position] = '\0';
    if (position > 0 && buffer[0] == '!') {
      if (buffer[1] >= '0' && buffer[1] <= '9') {
        exception_num = strtoul(&buffer[1], NULL, 10);
        if (exception_num > OKAY_NO_ERROR && exception_num <= ACTION_ERROR) {
          raise_error((int) exception_num);
        } /* if */
      } else {
        mapTraceFlags2(&buffer[1], &prog->option_flags);
        set_trace(prog->option_flags);
      } /* if */
    } /* if */
    while (ch != EOF && ch != '\n') {
      ch = fgetc(stdin);
    } /* while */
  } /* continue_question */



static void write_position_info (objectType currObject, boolType writeNoInfo)

  { /* write_position_info */
    if (currObject != NULL) {
      if (HAS_POSINFO(currObject)) {
        prot_cstri("at ");
        prot_string(get_file_name(GET_FILE_NUM(currObject)));
        prot_cstri("(");
        prot_int((intType) GET_LINE_NUM(currObject));
        prot_cstri(")");
      } else if (HAS_PROPERTY(currObject) &&
          currObject->descriptor.property->line != 0) {
        prot_cstri("at ");
        prot_string(get_file_name(currObject->descriptor.property->file_number));
        prot_cstri("(");
        prot_int((intType) currObject->descriptor.property->line);
        prot_cstri(")");
      } else if (writeNoInfo) {
        prot_cstri("no POSITION INFORMATION ");
        /* trace1(currObject); */
      } /* if */
    } /* if */
  } /* write_position_info */



static void write_call_stack_element (const_listType stack_elem)

  {
    objectType func_object;

  /* write_call_stack_element */
    if (stack_elem->obj != NULL) {
      if (stack_elem->next != NULL) {
        if (CATEGORY_OF_OBJ(stack_elem->obj) == CALLOBJECT ||
            CATEGORY_OF_OBJ(stack_elem->obj) == MATCHOBJECT) {
          func_object = stack_elem->obj->value.listValue->obj;
        } else {
          func_object = stack_elem->obj;
        } /* if */
        if (HAS_ENTITY(func_object)) {
          prot_cstri("in ");
          if (GET_ENTITY(func_object)->ident != NULL) {
            prot_cstri8(id_string(GET_ENTITY(func_object)->ident));
            prot_cstri(" ");
          } else if (func_object->descriptor.property->params != NULL) {
            prot_params(func_object->descriptor.property->params);
            prot_cstri(" ");
          } else if (GET_ENTITY(func_object)->fparam_list != NULL) {
            prot_name(GET_ENTITY(func_object)->fparam_list);
            prot_cstri(" ");
          } /* if */
        } /* if */
        write_position_info(stack_elem->next->obj, TRUE);
        prot_nl();
      } /* if */
    } else {
      prot_cstri("NULL");
      prot_nl();
    } /* if */
  } /* write_call_stack_element */



#if HAS_SIGACTION || HAS_SIGNAL
static void sigsegv_handler (int sig_num)

  { /* sigsegv_handler */
#if HAS_SIGNAL
    signal(SIGSEGV, sigsegv_handler);
#endif
    do_longjmp(sigsegv_occurred, 1);
  } /* sigsegv_handler */
#endif



void write_call_stack (const_listType stack_elem)

  {
#if HAS_SIGACTION
    struct sigaction action;
    struct sigaction old_action;
#endif
#if HAS_SIGACTION || HAS_SIGNAL
    void (*old_sigsegv_handler) (int sig_num);
#endif

  /* write_call_stack */
    if (stack_elem != NULL) {
      write_call_stack(stack_elem->next);
#if HAS_SIGACTION
      action.sa_handler = &sigsegv_handler;
      sigemptyset(&action.sa_mask);
      action.sa_flags = 0;
      if (sigaction(SIGSEGV, &action, &old_action) == 0) {
        old_sigsegv_handler = old_action.sa_handler;
#elif HAS_SIGNAL
      if ((old_sigsegv_handler = signal(SIGSEGV, sigsegv_handler)) != SIG_ERR) {
#endif
        if (do_setjmp(sigsegv_occurred) == 0) {
          write_call_stack_element(stack_elem);
        } else {
          prot_cstri("unaccessable stack data");
          prot_nl();
        } /* if */
#if HAS_SIGACTION
        action.sa_handler = old_sigsegv_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGSEGV, &action, NULL);
      } /* if */
#elif HAS_SIGNAL
        signal(SIGSEGV, old_sigsegv_handler);
      } /* if */
#endif
    } /* if */
  } /* write_call_stack */



static void write_curr_position (listType list)

  { /* write_curr_position */
    if (list == curr_argument_list) {
      prot_cstri(" ");
      write_position_info(curr_action_object, FALSE);
      prot_nl();
      prot_list(list);
      prot_cstri(" ");
      write_position_info(curr_exec_object, FALSE);
      prot_nl();
      if (curr_action_object->value.actValue != NULL) {
        prot_cstri("*** ACTION \"");
        prot_cstri(getActEntry(curr_action_object->value.actValue)->name);
        prot_cstri("\"");
        prot_nl();
      } /* if */
    } else {
      prot_cstri(" with");
      prot_nl();
      prot_list(list);
    } /* if */
  } /* write_curr_position */



objectType raise_with_arguments (objectType exception, listType list)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* raise_with_arguments */
#ifdef WITH_PROTOCOL
    if (list == curr_argument_list) {
      if (curr_exec_object != NULL && curr_exec_object->value.listValue != NULL) {
        curr_action_object = curr_exec_object->value.listValue->obj;
        incl_list(&fail_stack, curr_action_object, &err_info);
      } /* if */
    } /* if */
    if (trace.exceptions) {
      prot_nl();
      prot_cstri("*** EXCEPTION ");
      printobject(exception);
      prot_cstri(" raised");
      write_curr_position(list);
      continue_question();
    } /* if */
#endif
#ifndef USE_CHUNK_ALLOCS
    if (exception == SYS_MEM_EXCEPTION) {
      reuse_free_lists();
    } /* if */
#endif
    if (exception == NULL) {
      if (ALLOC_OBJECT(exception)) {
        exception->type_of = NULL;
        exception->descriptor.property = NULL;
        INIT_CATEGORY_OF_TEMP(exception, SYMBOLOBJECT);
        exception->value.intValue = 0;
      } /* if */
    } /* if */
    incl_list(&fail_stack, curr_exec_object, &err_info);
    if (!fail_flag || fail_value == NULL) {
      fail_value = exception;
      fail_expression = copy_list(list, &err_info);
    } /* if */
    set_fail_flag(TRUE);
    return NULL;
  } /* raise_with_arguments */



objectType raise_exception (objectType exception)

  { /* raise_exception */
    return raise_with_arguments(exception, curr_argument_list);
  } /* raise_exception */



void interprRaiseError (int exception_num, const_cstriType filename, int line)

  { /* interprRaiseError */
    (void) raise_exception(prog->sys_var[exception_num]);
  } /* interprRaiseError */



void show_signal (void)

  { /* show_signal */
    interrupt_flag = FALSE;
    printf("\n*** Program stopped with signal %s\n", signal_name(signal_number));
    continue_question();
  } /* show_signal */



void run_error (objectCategory required, objectType argument)

  { /* run_error */
    if (curr_exec_object != NULL) {
      curr_action_object = curr_exec_object->value.listValue->obj;
    } /* if */
    printf("\n*** ACTION $");
    if (curr_action_object->value.actValue != NULL) {
      printf("%s", getActEntry(curr_action_object->value.actValue)->name);
    } else {
      printf("NULL");
    } /* if */
    prot_cstri(" ");
    write_position_info(curr_action_object, FALSE);
    printf(" requires ");
    printcategory(required);
    printf(" not ");
    printcategory(CATEGORY_OF_OBJ(argument));
    printf("\n");
    trace1(argument);
    printf("\n");
    prot_list(curr_argument_list);
    continue_question();
    if (!fail_flag) {
      raise_error(RANGE_ERROR);
    } /* if */
  } /* run_error */



void empty_value (objectType argument)

  { /* empty_value */
    if (curr_exec_object != NULL) {
      curr_action_object = curr_exec_object->value.listValue->obj;
    } /* if */
    printf("\n*** ACTION $");
    if (curr_action_object->value.actValue != NULL) {
      printf("%s", getActEntry(curr_action_object->value.actValue)->name);
    } else {
      printf("NULL");
    } /* if */
    printf(" WITH EMPTY VALUE\n");
    trace1(argument);
    printf("\nobject_ptr=" FMT_X_MEM "\n", (memSizeType) argument);
    prot_list(curr_argument_list);
    continue_question();
    if (!fail_flag) {
      raise_error(RANGE_ERROR);
    } /* if */
  } /* empty_value */



void var_required (objectType argument)

  { /* var_required */
    if (curr_exec_object != NULL) {
      curr_action_object = curr_exec_object->value.listValue->obj;
    } /* if */
    printf("\n*** ACTION $");
    if (curr_action_object->value.actValue != NULL) {
      printf("%s", getActEntry(curr_action_object->value.actValue)->name);
    } else {
      printf("NULL");
    } /* if */
    printf(" REQUIRES VARIABLE ");
    printcategory(CATEGORY_OF_OBJ(argument));
    printf(" NOT CONSTANT\n");
    trace1(argument);
    printf("\nobject_ptr=" FMT_X_MEM "\n", (memSizeType) argument);
    prot_list(curr_argument_list);
    continue_question();
    if (!fail_flag) {
      raise_error(RANGE_ERROR);
    } /* if */
  } /* var_required */
