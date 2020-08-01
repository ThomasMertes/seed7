/********************************************************************/
/*                                                                  */
/*  heaputl.c     Procedures for heap allocation and maintenance.   */
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
/*  Content: Procedures for heap allocation and maintenance.        */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#if HAS_GETRLIMIT && defined STACK_SIZE
/* In FreeBSD it is necessary to include <sys/types.h> before <sys/resource.h> */
#include "sys/types.h"
#include "sys/resource.h"
#endif

#include "common.h"
#include "data_rtl.h"
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



/**
 *  Set the stack size of the program.
 *  On some operating systems the default stack size is too small.
 *  On such systems 'setupStack' is used to request a bigger stack.
 */
void setupStack (void)

  {
#if HAS_GETRLIMIT && defined STACK_SIZE
    struct rlimit rlim;
#endif
#if CHECK_STACK
    char aVariable;
#endif

  /* setupStack */
    logFunction(printf("setupStack\n"););
#if HAS_GETRLIMIT && defined STACK_SIZE
    /* printf("STACK_SIZE:      %ld\n", STACK_SIZE); */
    if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
      /* printf("old stack limit: %ld/%ld\n", (long) rlim.rlim_cur, (long) rlim.rlim_max); */
      if (rlim.rlim_cur != RLIM_INFINITY && (rlim_t) STACK_SIZE > rlim.rlim_cur) {
        if (rlim.rlim_max == RLIM_INFINITY || (rlim_t) STACK_SIZE <= rlim.rlim_max) {
          rlim.rlim_cur = (rlim_t) STACK_SIZE;
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
#if CHECK_STACK
    stack_base = &aVariable;
    /* printf("base:  " F_U_MEM(8) "\n", (memSizeType) stack_base); */
#endif
    logFunction(printf("setupStack -->\n"););
  } /* setupStack */



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
      /* The logFunctions (when inLogMacro is TRUE) should */
      /* only trigger an exeption for compiled programs    */
      /* (when interpreter_exception is FALSE). In the     */
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
 *  This function is called from the macro GROW_STRI, when the
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
             ") size=%u, capacity=%u, newCapacity=%u, siz_stri=%lu, sizeof=%lu\n",
             (memSizeType) stri, len, stri->size, stri->capacity, newCapacity,
             SIZ_STRI(newCapacity), sizeof(striRecord));
      fflush(stdout); */
      result = REALLOC_HEAP(stri, striType, SIZ_STRI(newCapacity));
      if (unlikely(result == NULL)) {
        do {
          newCapacity = (newCapacity + len) / 2;
          /* printf("newCapacity: %u, siz_stri=%lu\n", newCapacity, SIZ_STRI(newCapacity));
          fflush(stdout); */
          result = REALLOC_HEAP(stri, striType, SIZ_STRI(newCapacity));
        } while (result == NULL && newCapacity != len);
      } /* if */
      if (likely(result != NULL)) {
#if ALLOW_STRITYPE_SLICES
        result->mem = result->mem1;
#endif
        result->capacity = newCapacity;
      } else {
        logError(printf("growStri(" FMT_X_MEM ", " FMT_U_MEM ") failed\n",
                        (memSizeType) stri, len););
      } /* if */
    } /* if */
    logFunction(printf("growStri --> " FMT_X_MEM "\n", (memSizeType) result);
                fflush(stdout););
    return result;
  } /* growStri */



/**
 *  Reduce the capacity of a string.
 *  This function is called from the macro SHRINK_STRI, when the
 *  capacity of a string is much too large. ShrinkStri reduces the
 *  capacity, but it leaves room, such that it can grow aggain.
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
    result = REALLOC_HEAP(stri, striType, SIZ_STRI(newCapacity));
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



#if DO_HEAP_STATISTIC
void rtlHeapStatistic (void)

  {
    memSizeType bytes_used;
    memSizeType bytes_in_buffers;
    memSizeType bytes_total;

  /* rtlHeapStatistic */
    logFunction(printf("rtlHeapStatistic\n"););
    bytes_used = 0;
    if (count.stri != 0) {
      printf("%9lu bytes in %8lu string records of      %4u bytes\n",
          count.stri * SIZ_STRI(0),
          count.stri,
          (unsigned int) SIZ_STRI(0));
      bytes_used += count.stri * SIZ_STRI(0);
      printf("%9lu bytes in %8lu string chars of        %4u bytes\n",
          count.stri_elems * sizeof(strElemType),
          count.stri_elems,
          (unsigned int) sizeof(strElemType));
      bytes_used += count.stri_elems * sizeof(strElemType);
    } /* if */
    if (count.bstri != 0) {
      printf("%9lu bytes in %8lu bstring records of     %4u bytes\n",
          count.bstri * SIZ_BSTRI(0),
          count.bstri,
          (unsigned int) SIZ_BSTRI(0));
      bytes_used += count.bstri * SIZ_BSTRI(0);
      printf("%9lu bytes in %8lu bstrings of average    %4lu bytes\n",
          count.bstri_elems * sizeof(ucharType),
          count.bstri,
          count.bstri_elems * sizeof(ucharType) / count.bstri);
      bytes_used += count.bstri_elems * sizeof(ucharType);
    } /* if */
    if (count.array != 0) {
      printf("%9lu bytes in %8lu arrays of              %4u bytes\n",
          count.array * SIZ_RTL_ARR(0),
          count.array,
          (unsigned int) SIZ_RTL_ARR(0));
      bytes_used += count.array * SIZ_RTL_ARR(0);
    } /* if */
    if (count.rtl_arr_elems != 0) {
      printf("%9lu bytes in %8lu array elements of      %4u bytes\n",
          count.rtl_arr_elems * SIZ_REC(rtlObjectType),
          count.rtl_arr_elems,
          (unsigned int) SIZ_REC(rtlObjectType));
      bytes_used += count.rtl_arr_elems * SIZ_REC(rtlObjectType);
    } /* if */
    if (count.hash != 0) {
      printf("%9lu bytes in %8lu hashtables of          %4u bytes\n",
          count.hash * SIZ_RTL_HSH(0),
          count.hash,
          (unsigned int) SIZ_RTL_HSH(0));
      bytes_used += count.hash * SIZ_RTL_HSH(0);
    } /* if */
    if (count.hsh_elems != 0) {
      printf("%9lu bytes in %8lu hashtable elems of     %4u bytes\n",
          count.hsh_elems * SIZ_REC(rtlHashElemType),
          count.hsh_elems,
          (unsigned int) SIZ_REC(rtlHashElemType));
      bytes_used += count.hsh_elems * SIZ_REC(rtlHashElemType);
    } /* if */
    if (count.rtl_helem != 0) {
      printf("%9lu bytes in %8lu helems of              %4u bytes\n",
          count.rtl_helem * SIZ_REC(rtlHashElemRecord),
          count.rtl_helem,
          (unsigned int) SIZ_REC(rtlHashElemRecord));
      bytes_used += count.rtl_helem * SIZ_REC(rtlHashElemRecord);
    } /* if */
    if (count.set != 0) {
      printf("%9lu bytes in %8lu sets of                %4u bytes\n",
          count.set * SIZ_SET(0),
          count.set,
          (unsigned int) SIZ_SET(0));
      bytes_used += count.set * SIZ_SET(0);
    } /* if */
    if (count.set_elems != 0) {
      printf("%9lu bytes in %8lu set elements of        %4u bytes\n",
          count.set_elems * SIZ_REC(bitSetType),
          count.set_elems,
          (unsigned int) SIZ_REC(bitSetType));
      bytes_used += count.set_elems * SIZ_REC(bitSetType);
    } /* if */
#if 0
    if (count.stru != 0) {
      printf("%9lu bytes in %8lu structs of             %4u bytes\n",
          count.stru * SIZ_SCT(0),
          count.stru,
          (unsigned int) SIZ_SCT(0));
      bytes_used += count.stru * SIZ_SCT(0);
    } /* if */
    if (count.sct_elems != 0) {
      printf("%9lu bytes in %8lu struct elements of     %4u bytes\n",
          count.sct_elems * SIZ_REC(objectRecord),
          count.sct_elems,
          (unsigned int) SIZ_REC(objectRecord));
      bytes_used += count.sct_elems * SIZ_REC(objectRecord);
    } /* if */
#endif
#if BIGINT_LIB == BIG_RTL_LIBRARY
    if (count.big != 0) {
      printf("%9lu bytes in %8lu bigIntegers of         %4u bytes\n",
          count.big * SIZ_BIG(0),
          count.big,
          (unsigned int) SIZ_BIG(0));
      bytes_used += count.big * SIZ_BIG(0);
    } /* if */
    if (count.big_elems != 0) {
      printf("%9lu bytes in %8lu bigdigits of           %4u bytes\n",
          count.big_elems * sizeof_bigDigitType,
          count.big_elems,
          (unsigned int) sizeof_bigDigitType);
      bytes_used += count.big_elems * sizeof_bigDigitType;
    } /* if */
#endif
    if (count.polldata != 0) {
      printf("%9lu bytes in %8lu pollData elements of   %4u bytes\n",
          count.polldata * sizeof_pollRecord,
          count.polldata,
          (unsigned int) sizeof_pollRecord);
      bytes_used += count.polldata * sizeof_pollRecord;
    } /* if */
    if (count.win != 0) {
      printf("%9lu bytes in %8lu windows of             %4u bytes\n",
          count.win * sizeof_winRecord,
          count.win,
          (unsigned int) sizeof_winRecord);
      bytes_used += count.win * sizeof_winRecord;
    } /* if */
    if (count.process != 0) {
      printf("%9lu bytes in %8lu processes of           %4u bytes\n",
          count.process * sizeof_processRecord,
          count.process,
          (unsigned int) sizeof_processRecord);
      bytes_used += count.process * sizeof_processRecord;
    } /* if */
    bytes_in_buffers =
        count.fnam_bytes + count.fnam +
        count.symb_bytes + count.symb +
        count.byte;
    if (bytes_in_buffers != 0) {
      printf("%9lu bytes in buffers\n", bytes_in_buffers);
      bytes_used += bytes_in_buffers;
    } /* if */
    if (bytes_used != 0) {
      printf("%9lu bytes in use\n", bytes_used);
    } /* if */
    bytes_total = bytes_used;
#if DO_HEAPSIZE_COMPUTATION
    if (bytes_total != hs) {
      printf("*** \nbytes_total=" FMT_U_MEM " hs=" FMT_U_MEM " diff=" FMT_D_MEM "\n",
          bytes_total, hs, bytes_total - hs);
    } /* if */
#endif
#ifdef USE_CHUNK_ALLOCS
    if (chunk.total_size != 0) {
      printf("%9lu bytes in %8u chunks\n",
          chunk.total_size, chunk.number_of_chunks);
      printf("%9u unused bytes in last chunk\n",
          (unsigned) (chunk.beyond - chunk.freemem));
      printf("%9lu lost bytes in chunks\n", chunk.lost_bytes);
    } /* if */
    printf("%9lu bytes total requested\n", bytes_total +
        (memSizeType) (chunk.beyond - chunk.freemem) + chunk.lost_bytes);
#endif
    logFunction(printf("rtlHeapStatistic -->\n"););
  } /* rtlHeapStatistic */
#endif
