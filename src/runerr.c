/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000, 2010 - 2011  Thomas Mertes           */
/*                2014 - 2017, 2020, 2021  Thomas Mertes            */
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
/*  Changes: 1990 - 1994, 2010 - 2011, 2014 - 2017   Thomas Mertes  */
/*           2020, 2021   Thomas Mertes                             */
/*  Content: Runtime error and exception handling functions.        */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "signal.h"
#include "setjmp.h"

#include "common.h"
#include "data.h"
#include "os_decls.h"
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


static longjmpPosition sigsegvOccurred;



static void continue_question (objectType *exception)

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
      shutDrivers();
      os_exit(1);
    } else if (ch == (int) '/') {
      triggerSigfpe();
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
          if (exception != NULL) {
            *exception = prog->sys_var[exception_num];
          } else {
            raise_error((int) exception_num);
          } /* if */
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
    logFunction(printf("write_position_info\n"););
    if (currObject != NULL &&
        LEGAL_CATEGORY_FIELD(currObject)) {
      if (CATEGORY_OF_OBJ(currObject) == CALLOBJECT ||
          CATEGORY_OF_OBJ(currObject) == MATCHOBJECT ||
          CATEGORY_OF_OBJ(currObject) == SYMBOLOBJECT ||
          CATEGORY_OF_OBJ(currObject) == ACTOBJECT ||
          CATEGORY_OF_OBJ(currObject) == DECLAREDOBJECT) {
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
          /* printcategory(CATEGORY_OF_OBJ(currObject)); */
          /* trace1(currObject); */
        } /* if */
      } else if (writeNoInfo) {
        prot_cstri("no POSITION INFORMATION ");
        /* printcategory(CATEGORY_OF_OBJ(currObject)); */
        /* trace1(currObject); */
      } /* if */
    } else if (writeNoInfo) {
      prot_cstri("no POSITION INFORMATION ");
    } /* if */
    logFunction(printf("write_position_info -->\n"););
  } /* write_position_info */



static void write_call_stack_element (const_listType stack_elem)

  {
    objectType func_object;

  /* write_call_stack_element */
    logFunction(printf("write_call_stack_element(" FMT_U_MEM ")\n",
                       (memSizeType) stack_elem););
    if (stack_elem->obj != NULL &&
        LEGAL_CATEGORY_FIELD(stack_elem->obj)) {
      if (stack_elem->next != NULL) {
        if (CATEGORY_OF_OBJ(stack_elem->obj) == CALLOBJECT ||
            CATEGORY_OF_OBJ(stack_elem->obj) == MATCHOBJECT) {
          func_object = stack_elem->obj->value.listValue->obj;
        } else if (CATEGORY_OF_OBJ(stack_elem->obj) == FWDREFOBJECT) {
          /* prot_cstri("(");
          printcategory(CATEGORY_OF_OBJ(stack_elem->obj));
          prot_cstri(") "); */
          func_object = stack_elem->obj->value.objValue;
        } else {
          /* prot_cstri("(");
          printcategory(CATEGORY_OF_OBJ(stack_elem->obj));
          prot_cstri(") "); */
          func_object = stack_elem->obj;
        } /* if */
        if (func_object != NULL &&
            LEGAL_CATEGORY_FIELD(func_object)) {
          if (CATEGORY_OF_OBJ(func_object) == ACTOBJECT ||
              CATEGORY_OF_OBJ(func_object) == BLOCKOBJECT) {
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
          } else {
            prot_cstri("in ");
            printcategory(CATEGORY_OF_OBJ(func_object));
            prot_cstri(" ");
          } /* if */
        } else {
          prot_cstri("in *NULL* ");
        } /* if */
        write_position_info(stack_elem->next->obj, TRUE);
        prot_nl();
      } /* if */
    } /* if */
    logFunction(printf("write_call_stack_element(" FMT_U_MEM ") -->\n",
                       (memSizeType) stack_elem););
  } /* write_call_stack_element */



#if HAS_SIGACTION

static void sigsegv_handler (int sig, siginfo_t *info, void *context)

  { /* sigsegv_handler */
    do_longjmp(sigsegvOccurred, 1);
  } /* sigsegv_handler */

#elif HAS_SIGNAL

static void sigsegv_handler (int sig)

  { /* sigsegv_handler */
#if SIGNAL_RESETS_HANDLER
    signal(SIGSEGV, sigsegv_handler);
#endif
    do_longjmp(sigsegvOccurred, 1);
  } /* sigsegv_handler */

#endif



void write_fail_expression (listType failExpression)

  {
#if HAS_SIGACTION
    struct sigaction sigAct;
    struct sigaction oldSigAct;
#elif HAS_SIGNAL
    void (*oldSigHandler) (int sig);
#endif

  /* write_fail_expression */
#if HAS_SIGACTION
    sigAct.sa_sigaction = sigsegv_handler;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &sigAct, &oldSigAct) == 0) {
#elif HAS_SIGNAL
    if ((oldSigHandler = signal(SIGSEGV, sigsegv_handler)) != SIG_ERR) {
#endif
      if (do_setjmp(sigsegvOccurred) == 0) {
        prot_list(failExpression);
      } else {
        prot_cstri("unaccessable expression");
      } /* if */
#if HAS_SIGACTION
      sigaction(SIGSEGV, &oldSigAct, NULL);
    } /* if */
#elif HAS_SIGNAL
      signal(SIGSEGV, oldSigHandler);
    } /* if */
#endif
  } /* write_fail_expression */



void write_call_stack (const_listType stack_elem)

  {
#if HAS_SIGACTION
    struct sigaction sigAct;
    struct sigaction oldSigAct;
#elif HAS_SIGNAL
    void (*oldSigHandler) (int sig);
#endif

  /* write_call_stack */
    logFunction(printf("write_call_stack(" FMT_U_MEM ")\n",
                       (memSizeType) stack_elem););
    if (stack_elem != NULL) {
      write_call_stack(stack_elem->next);
#if HAS_SIGACTION
      sigAct.sa_sigaction = sigsegv_handler;
      sigemptyset(&sigAct.sa_mask);
      sigAct.sa_flags = SA_SIGINFO;
      if (sigaction(SIGSEGV, &sigAct, &oldSigAct) == 0) {
#elif HAS_SIGNAL
      if ((oldSigHandler = signal(SIGSEGV, sigsegv_handler)) != SIG_ERR) {
#endif
        if (do_setjmp(sigsegvOccurred) == 0) {
          write_call_stack_element(stack_elem);
        } else {
          prot_cstri("unaccessable stack data");
          prot_nl();
        } /* if */
#if HAS_SIGACTION
        sigaction(SIGSEGV, &oldSigAct, NULL);
      } /* if */
#elif HAS_SIGNAL
        signal(SIGSEGV, oldSigHandler);
      } /* if */
#endif
    } /* if */
    logFunction(printf("write_call_stack(" FMT_U_MEM ") -->\n",
                       (memSizeType) stack_elem););
  } /* write_call_stack */



void uncaught_exception (void)

  { /* uncaught_exception */
    prot_nl();
    prot_cstri("*** Uncaught exception ");
    printobject(fail_value);
    prot_cstri(" raised with");
    prot_nl();
    write_fail_expression(fail_expression);
    prot_nl();
    prot_nl();
    prot_cstri("Stack:\n");
    write_call_stack(fail_stack);
  } /* uncaught_exception */



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
      if (curr_action_object != NULL &&
          curr_action_object->value.actValue != NULL) {
        prot_cstri("*** Action \"");
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



void write_exception_info (void)

  { /* write_exception_info */
    prot_nl();
    prot_cstri("*** Exception ");
    printobject(fail_value);
    prot_cstri(" raised");
    write_curr_position(fail_expression);
    prot_nl();
  } /* write_exception_info */



objectType raise_with_arguments (objectType exception, listType list)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* raise_with_arguments */
#ifdef WITH_PROTOCOL
    if (list == curr_argument_list) {
      if (curr_exec_object != NULL &&
          CATEGORY_OF_OBJ(curr_exec_object) == CALLOBJECT &&
          curr_exec_object->value.listValue != NULL) {
        curr_action_object = curr_exec_object->value.listValue->obj;
        incl_list(&fail_stack, curr_action_object, &err_info);
      } /* if */
    } /* if */
    if (trace.exceptions) {
      prot_nl();
      prot_cstri("*** Exception ");
      printobject(exception);
      prot_cstri(" raised");
      write_curr_position(list);
      continue_question(&exception);
    } /* if */
#endif
#if !USE_CHUNK_ALLOCS
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
      /* printf("New fail_expression: ");
      prot_list(fail_expression);
      prot_nl(); */
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



void leaveExceptionHandling (void)

  { /* leaveExceptionHandling */
    logFunction(printf("leaveExceptionHandling\n"););
    set_fail_flag(FALSE);
    fail_value = NULL;
    free_list(fail_expression);
    fail_expression = NULL;
    free_list(fail_stack);
    fail_stack = NULL;
  } /* leaveExceptionHandling */



void saveFailState (failStateStruct *failState)

  { /* saveFailState */
    logFunction(printf("saveFailState\n"););
    failState->fail_flag = fail_flag;
    failState->fail_value = fail_value;
    failState->fail_expression = fail_expression;
    failState->fail_stack = fail_stack;
    set_fail_flag(FALSE);
    fail_value = NULL;
    fail_expression = NULL;
    fail_stack = NULL;
  } /* saveFailState */



void restoreFailState (failStateStruct *failState)

  { /* restoreFailState */
    logFunction(printf("restoreFailState\n"););
    free_list(fail_expression);
    free_list(fail_stack);
    interrupt_flag = TRUE;
    fail_flag = failState->fail_flag;
    fail_value = failState->fail_value;
    fail_expression = failState->fail_expression;
    fail_stack = failState->fail_stack;
  } /* restoreFailState */



errInfoType getErrInfoFromFailValue (objectType failValue)

  {
    errInfoType err_info;

  /* getErrInfoFromFailValue */
    if (failValue == SYS_MEM_EXCEPTION) {
      err_info = MEMORY_ERROR;
    } else if (failValue == SYS_NUM_EXCEPTION) {
      err_info = NUMERIC_ERROR;
    } else if (failValue == SYS_OVF_EXCEPTION) {
      err_info = OVERFLOW_ERROR;
    } else if (failValue == SYS_RNG_EXCEPTION) {
      err_info = RANGE_ERROR;
    } else if (failValue == SYS_IDX_EXCEPTION) {
      err_info = INDEX_ERROR;
    } else if (failValue == SYS_FIL_EXCEPTION) {
      err_info = FILE_ERROR;
    } else if (failValue == SYS_DB_EXCEPTION) {
      err_info = DATABASE_ERROR;
    } else if (failValue == SYS_GRAPHIC_EXCEPTION) {
      err_info = GRAPHIC_ERROR;
    } else {  /* if (failValue == SYS_ACT_ILLEGAL_EXCEPTION) { */
      err_info = ACTION_ERROR;
    } /* if */
    return err_info;
  } /* getErrInfoFromFailValue */



void show_signal (void)

  { /* show_signal */
    interrupt_flag = FALSE;
    printf("\n*** Program suspended with signal %s\n", signalName(signal_number));
    continue_question(NULL);
  } /* show_signal */



void run_error (objectCategory required, objectType argument)

  { /* run_error */
    if (!fail_flag) {
#ifdef WITH_PROTOCOL
      if (trace.exceptions) {
        if (curr_exec_object != NULL &&
            CATEGORY_OF_OBJ(curr_exec_object) == CALLOBJECT &&
            curr_exec_object->value.listValue != NULL) {
          curr_action_object = curr_exec_object->value.listValue->obj;
        } /* if */
        printf("\n*** Action $");
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
        printf("\n");
        continue_question(NULL);
      } /* if */
#endif
      raise_error(ACTION_ERROR);
    } /* if */
  } /* run_error */



void empty_value (objectType argument)

  { /* empty_value */
    if (!fail_flag) {
#ifdef WITH_PROTOCOL
      if (trace.exceptions) {
        if (curr_exec_object != NULL &&
            CATEGORY_OF_OBJ(curr_exec_object) == CALLOBJECT &&
            curr_exec_object->value.listValue != NULL) {
          curr_action_object = curr_exec_object->value.listValue->obj;
        } /* if */
        printf("\n*** Action $");
        if (curr_action_object->value.actValue != NULL) {
          printf("%s", getActEntry(curr_action_object->value.actValue)->name);
        } else {
          printf("NULL");
        } /* if */
        printf(" with empty value\n");
        trace1(argument);
        /* printf("\nobject_ptr=" FMT_X_MEM "\n", (memSizeType) argument); */
        prot_list(curr_argument_list);
        printf("\n");
        continue_question(NULL);
      } /* if */
#endif
      raise_error(ACTION_ERROR);
    } /* if */
  } /* empty_value */



void var_required (objectType argument)

  { /* var_required */
    if (!fail_flag) {
#ifdef WITH_PROTOCOL
      if (trace.exceptions) {
        if (curr_exec_object != NULL &&
            CATEGORY_OF_OBJ(curr_exec_object) == CALLOBJECT &&
            curr_exec_object->value.listValue != NULL) {
          curr_action_object = curr_exec_object->value.listValue->obj;
        } /* if */
        printf("\n*** Action $");
        if (curr_action_object->value.actValue != NULL) {
          printf("%s", getActEntry(curr_action_object->value.actValue)->name);
        } else {
          printf("NULL");
        } /* if */
        printf(" requires variable ");
        printcategory(CATEGORY_OF_OBJ(argument));
        printf(" not constant\n");
        trace1(argument);
        /* printf("\nobject_ptr=" FMT_X_MEM "\n", (memSizeType) argument); */
        prot_list(curr_argument_list);
        printf("\n");
        continue_question(NULL);
      } /* if */
#endif
      raise_error(ACTION_ERROR);
    } /* if */
  } /* var_required */
