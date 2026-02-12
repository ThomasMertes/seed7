/********************************************************************/
/*                                                                  */
/*  heaputl.c     Functions for heap allocation and maintenance.    */
/*  Copyright (C) 1989 - 2011  Thomas Mertes                        */
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
/*  File: seed7/src/heaputl.c                                       */
/*  Changes: 1992 - 1994, 2008, 2010  Thomas Mertes                 */
/*  Content: Functions for heap allocation and maintenance.         */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#if HAS_GETRLIMIT
/* In FreeBSD it is necessary to include <sys/types.h> before <sys/resource.h> */
#include "sys/types.h"
#include "sys/resource.h"
#endif
#include "signal.h"
#include "setjmp.h"

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "sigutl.h"
#include "striutl.h"
#include "str_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "heaputl.h"

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

#define CATCH_STACK_INCREMENT 128
typedef longjmpPosition catch_type;
catch_type *catch_stack;
size_t catch_stack_pos;
size_t max_catch_stack;



#if HAS_SIGALTSTACK && !SIGNAL_STACK_ENABLED
static void *signalStack = NULL;



static void alternateSignalStack (void)

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
  } /* alternateSignalStack */

#endif



static void closeStack (void)

  { /* closeStack */
    logFunction(printf("closeStack\n"););
#if HAS_SIGALTSTACK
    free(signalStack);
#endif
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
        setrlimit(RLIMIT_STACK, &rlim);
        /* if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
          printf("new stack limit: %ld/%ld\n", (long) rlim.rlim_cur, (long) rlim.rlim_max);
        } ** if */
      } /* if */
    } /* if */
#endif
#if HAS_SIGALTSTACK && !SIGNAL_STACK_ENABLED
    alternateSignalStack();
#endif
    catch_stack_pos = 0;
    max_catch_stack = CATCH_STACK_INCREMENT;
    catch_stack = (catch_type *) malloc(max_catch_stack * sizeof(catch_type));
#if CHECK_STACK
    stack_base = &aVariable;
    /* printf("base:  " F_U_MEM(8) "\n", (memSizeType) stack_base); */
#endif
    os_atexit(closeStack);
    logFunction(printf("setupStack -->\n"););
  } /* setupStack */



boolType resizeCatchStackOkay (void)

  {
    catch_type *resized_stack;
    boolType okay = TRUE;

  /* resizeCatchStackOkay */
    max_catch_stack += CATCH_STACK_INCREMENT;
    resized_stack = (catch_type *) realloc(catch_stack, max_catch_stack * sizeof(catch_type));
    if (unlikely(resized_stack == NULL)) {
      catch_stack_pos--;
      okay = FALSE;
    } else {
      catch_stack = resized_stack;
    } /* if */
    return okay;
  } /* resizeCatchStackOkay */



void resize_catch_stack (void)

  {
    catch_type *resized_stack;

  /* resize_catch_stack */
    max_catch_stack += CATCH_STACK_INCREMENT;
    resized_stack = (catch_type *) realloc(catch_stack, max_catch_stack * sizeof(catch_type));
    if (unlikely(resized_stack == NULL)) {
      catch_stack_pos--;
      raise_error(MEMORY_ERROR);
    } else {
      catch_stack = resized_stack;
    } /* if */
  } /* resize_catch_stack */



void no_memory (const_cstriType source_file, int source_line)

  { /* no_memory */
    logFunction(printf("no_memory(\"%s\", %d)\n", source_file, source_line););
    exception_number = MEMORY_ERROR;
    error_file = source_file;
    error_line = source_line;
    if (catch_stack_pos > 0) {
      logFunction(printf("no_memory(\"%s\", %d) --> longjmp\n",
                         source_file, source_line););
      do_longjmp(catch_stack[catch_stack_pos], MEMORY_ERROR);
    } else {
      /* shutDrivers(); */
      logFunction(printf("no_memory(\"%s\", %d) --> exit\n",
                         source_file, source_line););
      os_exit(1);
    } /* if */
  } /* no_memory */



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
      strDestr(*fileName);
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



#if WITH_STRI_CAPACITY
/**
 *  Enlarge the capacity of a string.
 *  This function is called from the macro GROW_STRI, if the
 *  capacity of a string is not sufficient. GrowStri enlarges the
 *  capacity such that at least 'len' characters fit into it.
 *  It is assumed that 'stri' will grow further, therefore the
 *  capacity is usually doubled.
 *  @param stri String for which the capacity is enlarged.
 *  @param len Length of the string that will be assigned.
 *  @return the enlarged string, or NULL if the allocation failed.
 */
striType growStri (striType stri, memSizeType len)

  {
    memSizeType newCapacity;
    striType result;

  /* growStri */
    if (unlikely(len > MAX_STRI_LEN)) {
      result = NULL;
    } else {
      if (2 * stri->capacity >= len) {
        newCapacity = 2 * stri->capacity;
      } else {
        newCapacity = 2 * len;
      } /* if */
      if (newCapacity < MIN_GROW_SHRINK_CAPACITY) {
        newCapacity = MIN_GROW_SHRINK_CAPACITY;
      } else if (unlikely(newCapacity > MAX_STRI_LEN)) {
        newCapacity = MAX_STRI_LEN;
      } /* if */
      /* printf("growStri(" FMT_X_MEM ", " FMT_U_MEM
             ") size=" FMT_U_MEM ", capacity=" FMT_U_MEM ", newCapacity="
             FMT_U_MEM ", siz_stri=" FMT_U_MEM ", sizeof=" FMT_U_MEM "\n",
             (memSizeType) stri, len, stri->size, stri->capacity, newCapacity,
             SIZ_STRI(newCapacity), sizeof(striRecord));
      fflush(stdout); */
      REALLOC_STRI_SIZE_OK(result, stri, newCapacity);
      if (unlikely(result == NULL)) {
        do {
          newCapacity = (newCapacity + len) / 2;
          /* printf("newCapacity: " FMT_U_MEM ", siz_stri=" FMT_U_MEM "\n",
              newCapacity, SIZ_STRI(newCapacity));
          fflush(stdout); */
          REALLOC_STRI_SIZE_OK(result, stri, newCapacity);
        } while (result == NULL && newCapacity != len);
      } /* if */
      if (likely(result != NULL)) {
#if ALLOW_STRITYPE_SLICES
        result->mem = result->mem1;
#endif
        result->capacity = newCapacity;
      } else {
        logError(printf("growStri(" FMT_X_MEM " (capacity=" FMT_U_MEM "), "
                        FMT_U_MEM ") failed\n",
                        (memSizeType) stri, stri->capacity, len););
        /* heapStatistic(); */
      } /* if */
    } /* if */
    logFunction(printf("growStri --> " FMT_X_MEM "\n", (memSizeType) result);
                fflush(stdout););
    return result;
  } /* growStri */



/**
 *  Reduce the capacity of a string.
 *  This function is called from the macro SHRINK_STRI, if the
 *  capacity of a string is much too large. ShrinkStri reduces the
 *  capacity, but it leaves room, such that it can grow again.
 *  @param stri String for which the capacity is reduced.
 *  @param len Length of the string that will be assigned.
 *  @return the reduced string.
 */
striType shrinkStri (striType stri, memSizeType len)

  {
    memSizeType newCapacity;
    striType result;

  /* shrinkStri */
    newCapacity = 2 * len;
    /* printf("shrinkStri(%lX, %lu) size=%u, capacity=%u, newCapacity=%u, siz_stri=%u, sizeof=%u\n",
         stri, len, stri->size, stri->capacity, newCapacity, SIZ_STRI(newCapacity), sizeof(striRecord));
    fflush(stdout); */
    REALLOC_STRI_SIZE_OK(result, stri, newCapacity);
#if ALLOW_STRITYPE_SLICES
    result->mem = result->mem1;
#endif
    result->capacity = newCapacity;
    return result;
  } /* shrinkStri */
#endif



#ifdef OUT_OF_ORDER
void freeStriFreelist (void)

  {
    freeListElemType elem;
    striType stri;
#if WITH_STRI_CAPACITY
    memSizeType capacity;
#endif

  /* freeStriFreelist */
#if WITH_STRI_CAPACITY
    for (capacity = 0; capacity < STRI_FREELIST_ARRAY_SIZE; capacity++) {
      elem = sflist[capacity];
      while (elem != NULL) {
        stri = (striType) elem;
        elem = elem->next;
        HEAP_FREE_STRI(stri, capacity);
        sflist_allowed[capacity]++;
      } /* while */
      sflist[capacity] = NULL;
    } /* for */
#else
    elem = sflist;
    while (elem != NULL) {
      stri = (striType) elem;
      elem = elem->next;
      HEAP_FREE_STRI(stri, 1);
      sflist_allowed++;
    } /* while */
    sflist = NULL;
#endif
  } /* freeStriFreelist */



void freelistStatistic (void)

  {
    freeListElemType elem;
#if WITH_STRI_CAPACITY
    memSizeType capacity;
    unsigned int sflist_size[STRI_FREELIST_ARRAY_SIZE];
#else
    unsigned int sflist_size;
#endif

  /* freelistStatistic */
#if WITH_STRI_CAPACITY
    for (capacity = 0; capacity < STRI_FREELIST_ARRAY_SIZE; capacity++) {
      sflist_size[capacity] = sflist_allowed[capacity];
      elem = sflist[capacity];
      while (elem != NULL) {
        elem = elem->next;
        sflist_size[capacity]++;
      } /* while */
      printf("sflist_size[" FMT_U_MEM "]: %u\n", capacity, sflist_size[capacity]);
    } /* for */
#else
    sflist_size = sflist_allowed;
    elem = sflist;
    while (elem != NULL) {
      elem = elem->next;
      sflist_size++;
    } /* while */
    printf("sflist_size: %u\n", sflist_size);
#endif
  } /* freelistStatistic */
#endif



#if !DO_HEAP_STATISTIC
void heapStatistic (void)

  { /* heapStatistic */
    printf("heap statistic not supported - Set DO_HEAP_STATISTIC in config.h, when you compile Seed7.\n");
  } /* heapStatistic */
#endif
