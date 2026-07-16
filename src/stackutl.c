/********************************************************************/
/*                                                                  */
/*  stackutl.c    Functions for stack, sigaltstack and catch_stack. */
/*  Copyright (C) 1989 - 2011, 2014, 2023, 2026  Thomas Mertes      */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/stackutl.c                                      */
/*  Changes: 2014, 2023, 2026  Thomas Mertes                        */
/*  Content: Functions for stack, sigaltstack and catch_stack.      */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#if HAS_GETRLIMIT
/* In FreeBSD it is necessary to include <sys/types.h> before <sys/resource.h> */
#include "sys/types.h"
#include "sys/resource.h"
#endif
#include "signal.h"
#include "setjmp.h"
#include "errno.h"

#include "common.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "stackutl.h"

#define INITIAL_CATCH_STACK_SIZE 128

/* Some compilers/linkers only support determining the stack size   */
/* by defining a global variable. This variable must be set to the  */
/* desired stack size. In this case the makefile defines the macro  */
/* STACK_SIZE_DEFINITION which contains this variable definition.   */
#ifdef STACK_SIZE_DEFINITION
STACK_SIZE_DEFINITION;
#endif

#if CHECK_STACK
char *stack_base = 0;
memSizeType max_stack_size = 0;
boolType interpreter_exception = FALSE;
#else
extern boolType interpreter_exception;
#endif



#if HAS_SIGALTSTACK && !SIGNAL_STACK_ENABLED
static void *signalStack = NULL;



static boolType alternateSignalStack (void)

  {
    stack_t ss;
    boolType okay = FALSE;

  /* alternateSignalStack */
    logFunction(printf("alternateSignalStack\n"););
    signalStack = malloc(SIGSTKSZ);
    if (signalStack != NULL) {
      ss.ss_sp = signalStack;
      ss.ss_flags = 0;
      ss.ss_size = SIGSTKSZ;
      if (sigaltstack(&ss, NULL) == 0) {
        okay = TRUE;
      } /* if */
    } /* if */
    logFunction(printf("alternateSignalStack --> %s\n",
                       okay ? "OKAY" : "NOT OKAY"););
    return okay;
  } /* alternateSignalStack */

#endif



static void closeStack (void)

  { /* closeStack */
    logFunction(printf("closeStack\n"););
#if HAS_SIGALTSTACK && !SIGNAL_STACK_ENABLED
    free(signalStack);
#endif
    free(catch_stack);
  } /* closeStack */



/**
 *  Set the stack size of the program.
 *  On some operating systems the default stack size is too small.
 *  On such systems 'setupStack' is used to request a bigger stack.
 */
void setupStack (memSizeType stackSize)

  {
#if HAS_GETRLIMIT
    struct rlimit rlim;
#endif
#if CHECK_STACK
    char aVariable;
#endif

  /* setupStack */
    logFunction(printf("setupStack(" FMT_U_MEM ")\n", stackSize););
#if HAS_GETRLIMIT
    if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
      /* printf("old stack limit: %ld/%ld\n", (long) rlim.rlim_cur, (long) rlim.rlim_max); */
      if (rlim.rlim_cur != RLIM_INFINITY && (rlim_t) stackSize > rlim.rlim_cur) {
        if (rlim.rlim_max == RLIM_INFINITY || (rlim_t) stackSize <= rlim.rlim_max) {
          rlim.rlim_cur = (rlim_t) stackSize;
        } else {
          rlim.rlim_cur = rlim.rlim_max;
        } /* if */
        if (setrlimit(RLIMIT_STACK, &rlim) != 0) {
          logError(printf("setrlimit(RLIMIT_STACK, rlim_cur=" FMT_U_MEM ") failed:\n"
                          "errno=%d\nerror: %s\n",
                          (memSizeType) rlim.rlim_cur,
                          errno, strerror(errno)););
        } /* if */
        logMessage(if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
                     printf("new stack limit: " FMT_U_MEM " / " FMT_U_MEM "\n",
                            (memSizeType) rlim.rlim_cur,
                            (memSizeType) rlim.rlim_max);
                   });
      } /* if */
    } /* if */
#endif
#if HAS_SIGALTSTACK && !SIGNAL_STACK_ENABLED
    if (alternateSignalStack()) {
#endif
      catch_stack = (catch_type *) malloc(INITIAL_CATCH_STACK_SIZE * sizeof(catch_type));
      /* If catch_stack is NULL interpreted and */
      /* compiled programs raise MEMORY_ERROR.  */
      if (likely(catch_stack != NULL)) {
        max_catch_stack = INITIAL_CATCH_STACK_SIZE;
      } /* if */
#if HAS_SIGALTSTACK && !SIGNAL_STACK_ENABLED
    } /* if */
#endif
#if CHECK_STACK
    stack_base = &aVariable;
    /* printf("base:  " F_U_MEM(8) "\n", (memSizeType) stack_base); */
#endif
    os_atexit(closeStack);
    logFunction(printf("setupStack -->\n"););
  } /* setupStack */



boolType resizeCatchStackOkay (void)

  {
    memSizeType new_max_catch_stack;
    catch_type *resized_stack;
    boolType okay = TRUE;

  /* resizeCatchStackOkay */
    if (unlikely(max_catch_stack >
                 MAX_MEMSIZETYPE / sizeof(catch_type) - max_catch_stack)) {
      catch_stack_pos--;
      okay = FALSE;
    } else {
      new_max_catch_stack = max_catch_stack << 1;
      resized_stack = (catch_type *) realloc(catch_stack,
          new_max_catch_stack * sizeof(catch_type));
      if (unlikely(resized_stack == NULL)) {
        catch_stack_pos--;
        okay = FALSE;
      } else {
        catch_stack = resized_stack;
        max_catch_stack = new_max_catch_stack;
      } /* if */
    } /* if */
    return okay;
  } /* resizeCatchStackOkay */



void resize_catch_stack (void)

  {
    memSizeType new_max_catch_stack;
    catch_type *resized_stack;

  /* resize_catch_stack */
    if (unlikely(max_catch_stack >
                 MAX_MEMSIZETYPE / sizeof(catch_type) - max_catch_stack)) {
      catch_stack_pos--;
      raise_error(MEMORY_ERROR);
    } else {
      new_max_catch_stack = max_catch_stack << 1;
      resized_stack = (catch_type *) realloc(catch_stack,
          new_max_catch_stack * sizeof(catch_type));
      if (unlikely(resized_stack == NULL)) {
        catch_stack_pos--;
        raise_error(MEMORY_ERROR);
      } else {
        catch_stack = resized_stack;
        max_catch_stack = new_max_catch_stack;
      } /* if */
    } /* if */
  } /* resize_catch_stack */



void prcGetRunError (intType *exceptionNumber, striType *fileName,
    intType *lineNumber)

  {
    striType file_name;

  /* prcGetRunError */
    logMessage(printf("prcGetRunError\n"););
    if (error_file != NULL) {
      file_name = cstri8_or_cstri_to_stri(error_file);
    } else {
      file_name = cstri_to_stri("");
    } /* if */
    if (unlikely(file_name == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      *exceptionNumber = (intType) exception_number;
      FREE_STRI(*fileName);
      *fileName = file_name;
      *lineNumber = (intType) error_line;
    } /* if */
    logFunction(printf("prcGetRunError --> \n"););
  } /* prcGetRunError */



#if CHECK_STACK
boolType checkStack (boolType inLogMacro)

  {
    char aVariable;
    boolType stackOverflow = FALSE;

  /* checkStack */
#if STACK_GROWS_UPWARD
    if (&aVariable - stack_base > max_stack_size &&
        stack_base != 0) {
      max_stack_size = (memSizeType) (&aVariable - stack_base);
    } /* if */
#else
    if (stack_base - &aVariable > max_stack_size &&
        stack_base != 0) {
      max_stack_size = (memSizeType) (stack_base - &aVariable);
    } /* if */
#endif
    /* This check is outside the maximum check on purpose. */
    /* A new maximum can occur from a logFunction, but the */
    /* check for the stack size limit can happen later,    */
    /* when the function is called from the interpreter.   */
    if (unlikely(max_stack_size > CHECKED_STACK_SIZE_LIMIT &&
                 inLogMacro != interpreter_exception)) {
      /* The logFunctions (if inLogMacro is TRUE) should   */
      /* only trigger an exception for compiled programs   */
      /* (if interpreter_exception is FALSE). In the       */
      /* interpreter the stack checking is called from the */
      /* function exec_action() (in this case inLogMacro   */
      /* is FALSE and interpreter_exception is TRUE).      */
      printf("\n*** Stack size above limit\n");
      printf("size:    " F_U_MEM(8) "\n", max_stack_size);
      printf("limit:   " F_U_MEM(8) "\n", (memSizeType) CHECKED_STACK_SIZE_LIMIT);
      printf("base:    " F_U_MEM(8) "\n", (memSizeType) stack_base);
      printf("current: " F_U_MEM(8) "\n", (memSizeType) &aVariable);
      if (inLogMacro) {
        raise_error(MEMORY_ERROR);
      } /* if */
      stackOverflow = TRUE;
    } /* if */
    return stackOverflow;
  } /* checkStack */



memSizeType getMaxStackSize (void)

  { /* getMaxStackSize */
    return max_stack_size;
  } /* getMaxStackSize */

#endif
