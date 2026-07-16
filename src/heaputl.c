/********************************************************************/
/*                                                                  */
/*  heaputl.c     Functions for heap allocation and maintenance.    */
/*  Copyright (C) 1989 - 2008, 2010, 2013, 2015  Thomas Mertes      */
/*                2018, 2026  Thomas Mertes                         */
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
#include "setjmp.h"

#include "common.h"
#include "os_decls.h"
#include "stackutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "heaputl.h"



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
    } /* if */
    logErrorIfTrue(result == NULL,
                   printf("growStri(" FMT_X_MEM " (capacity=" FMT_U_MEM "), "
                          FMT_U_MEM ") failed\n",
                          (memSizeType) stri, stri->capacity, len););
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
    logErrorIfTrue(result == NULL,
                   printf("shrinkStri(" FMT_X_MEM " (capacity=" FMT_U_MEM "), "
                          FMT_U_MEM ") failed\n",
                          (memSizeType) stri, stri->capacity, len););
    logFunction(printf("shrinkStri --> " FMT_X_MEM "\n", (memSizeType) result);
                fflush(stdout););
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



NORETURN void no_memory (const_cstriType source_file, int source_line)

  { /* no_memory */
    logFunction(printf("no_memory(\"%s\", %d)\n", source_file, source_line););
    exception_number = MEMORY_ERROR;
    error_file = source_file;
    error_line = source_line;
    logFunction(printf("no_memory(\"%s\", %d) --> longjmp\n",
                       source_file, source_line););
    do_longjmp(catch_stack[catch_stack_pos], MEMORY_ERROR);
  } /* no_memory */



#if !DO_HEAP_STATISTIC
void heapStatistic (void)

  { /* heapStatistic */
    printf("heap statistic not supported - Set DO_HEAP_STATISTIC in config.h, when you compile Seed7.\n");
  } /* heapStatistic */
#endif
