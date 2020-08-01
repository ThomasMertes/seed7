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
#include "sys/resource.h"
#endif

#include "common.h"
#include "data_rtl.h"

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



void setupStack (void)

  {
#if HAS_GETRLIMIT && defined STACK_SIZE
    struct rlimit rlim;
#endif

  /* setupStack */
#if HAS_GETRLIMIT && defined STACK_SIZE
    /* printf("STACK_SIZE:      %ld\n", STACK_SIZE); */
    if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
      /* printf("old stack limit: %ld/%ld\n", rlim.rlim_cur, rlim.rlim_max); */
      if (rlim.rlim_cur != RLIM_INFINITY && (rlim_t) STACK_SIZE > rlim.rlim_cur) {
        if (rlim.rlim_max == RLIM_INFINITY || (rlim_t) STACK_SIZE <= rlim.rlim_max) {
          rlim.rlim_cur = (rlim_t) STACK_SIZE;
        } else {
          rlim.rlim_cur = rlim.rlim_max;
        } /* if */
        setrlimit(RLIMIT_STACK, &rlim);
        /* if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
          printf("new stack limit: %ld/%ld\n", rlim.rlim_cur, rlim.rlim_max);
        } ** if */
      } /* if */
    } /* if */
#endif
  } /* setupStack */



#ifdef WITH_STRI_CAPACITY
striType growStri (striType stri, memSizeType len)

  {
    memSizeType new_len;
    striType result;

  /* growStri */
    if (unlikely(len > MAX_STRI_LEN)) {
      result = NULL;
    } else {
      if (2 * stri->capacity >= len) {
        new_len = 2 * stri->capacity;
      } else {
        new_len = 2 * len;
      } /* if */
      if (new_len < 8) {
        new_len = 8;
      } else if (unlikely(new_len > MAX_STRI_LEN)) {
        new_len = MAX_STRI_LEN;
      } /* if */
      /* printf("growStri(%lX, %lu) size=%u, capacity=%u, new_len=%u, siz_stri=%u, sizeof=%u\n",
         stri, len, stri->size, stri->capacity, new_len, SIZ_STRI(new_len), sizeof(striRecord));
      fflush(stdout); */
      result = REALLOC_HEAP(stri, striType, SIZ_STRI(new_len));
      if (result == NULL) {
        new_len = len;
        result = REALLOC_HEAP(stri, striType, SIZ_STRI(new_len));
      } /* if */
      if (result != NULL) {
#ifdef ALLOW_STRITYPE_SLICES
        result->mem = result->mem1;
#endif
        result->capacity = new_len;
      } /* if */
    } /* if */
    logFunction(printf("growStri --> " FMT_X_MEM "\n", (memSizeType) result);
                fflush(stdout););
    return result;
  } /* growStri */



striType shrinkStri (striType stri, memSizeType len)

  {
    memSizeType new_len;
    striType result;

  /* shrinkStri */
    new_len = 2 * len;
    /* printf("shrinkStri(%lX, %lu) size=%u, capacity=%u, new_len=%u, siz_stri=%u, sizeof=%u\n",
         stri, len, stri->size, stri->capacity, new_len, SIZ_STRI(new_len), sizeof(striRecord));
    fflush(stdout); */
    result = REALLOC_HEAP(stri, striType, SIZ_STRI(new_len));
#ifdef ALLOW_STRITYPE_SLICES
    result->mem = result->mem1;
#endif
    result->capacity = new_len;
    return result;
  } /* shrinkStri */
#endif



#ifdef OUT_OF_ORDER
void freeStriFreelist (void)

  {
    freeListElemType elem;
    striType stri;
#ifdef WITH_STRI_CAPACITY
    memSizeType capacity;
#endif

  /* freeStriFreelist */
#ifdef WITH_STRI_CAPACITY
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
#ifdef WITH_STRI_CAPACITY
    memSizeType capacity;
    unsigned int sflist_size[STRI_FREELIST_ARRAY_SIZE];
#else
    unsigned int sflist_size;
#endif

  /* freelistStatistic */
#ifdef WITH_STRI_CAPACITY
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
#ifdef USE_BIG_RTL_LIBRARY
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
      printf("*** \nbytes_total=%lu hs=%lu diff=%ld\n",
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
