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

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

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



#ifdef WITH_STRI_CAPACITY
stritype growStri (stritype stri, memsizetype len)

  {
    memsizetype new_len;
    stritype result;

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
         stri, len, stri->size, stri->capacity, new_len, SIZ_STRI(new_len), sizeof(strirecord));
      fflush(stdout); */
      result = REALLOC_HEAP(stri, stritype, SIZ_STRI(new_len));
      if (result == NULL) {
        new_len = len;
        result = REALLOC_HEAP(stri, stritype, SIZ_STRI(new_len));
      } /* if */
      if (result != NULL) {
#ifdef ALLOW_STRITYPE_SLICES
        result->mem = result->mem1;
#endif
        result->capacity = new_len;
      } /* if */
    } /* if */
    /* printf(" => %lX\n", result);
    fflush(stdout); */
    return result;
  } /* growStri */



stritype shrinkStri (stritype stri, memsizetype len)

  {
    memsizetype new_len;
    stritype result;

  /* shrinkStri */
    new_len = 2 * len;
    /* printf("shrinkStri(%lX, %lu) size=%u, capacity=%u, new_len=%u, siz_stri=%u, sizeof=%u\n",
         stri, len, stri->size, stri->capacity, new_len, SIZ_STRI(new_len), sizeof(strirecord));
    fflush(stdout); */
    result = REALLOC_HEAP(stri, stritype, SIZ_STRI(new_len));
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
    flisttype elem;
    stritype stri;
#ifdef WITH_STRI_CAPACITY
    memsizetype capacity;
#endif

  /* freeStriFreelist */
#ifdef WITH_STRI_CAPACITY
    for (capacity = 0; capacity < STRI_FREELIST_ARRAY_SIZE; capacity++) {
      elem = sflist[capacity];
      while (elem != NULL) {
        stri = (stritype) elem;
        elem = elem->next;
        HEAP_FREE_STRI(stri, capacity);
        sflist_allowed[capacity]++;
      } /* while */
      sflist[capacity] = NULL;
    } /* for */
#else
    elem = sflist;
    while (elem != NULL) {
      stri = (stritype) elem;
      elem = elem->next;
      HEAP_FREE_STRI(stri, 1);
      sflist_allowed++;
    } /* while */
    sflist = NULL;
#endif
  } /* freeStriFreelist */
#endif



#ifdef DO_HEAP_STATISTIC
void rtlHeapStatistic (void)

  {
    memsizetype bytes_used;
    memsizetype bytes_in_buffers;
    memsizetype bytes_total;

  /* rtlHeapStatistic */
#ifdef TRACE_HEAPUTIL
    printf("BEGIN rtlHeapStatistic\n");
#endif
    bytes_used = 0;
    if (count.stri != 0) {
      printf("%9lu bytes in %8lu string records of      %4u bytes\n",
          count.stri * SIZ_STRI(0),
          count.stri,
          (unsigned int) SIZ_STRI(0));
      bytes_used += count.stri * SIZ_STRI(0);
      printf("%9lu bytes in %8lu string chars of        %4u bytes\n",
          count.stri_elems * sizeof(strelemtype),
          count.stri_elems,
          (unsigned int) sizeof(strelemtype));
      bytes_used += count.stri_elems * sizeof(strelemtype);
    } /* if */
    if (count.bstri != 0) {
      printf("%9lu bytes in %8lu bstring records of     %4u bytes\n",
          count.bstri * SIZ_BSTRI(0),
          count.bstri,
          (unsigned int) SIZ_BSTRI(0));
      bytes_used += count.bstri * SIZ_BSTRI(0);
      printf("%9lu bytes in %8lu bstrings of average    %4lu bytes\n",
          count.bstri_elems * sizeof(uchartype),
          count.bstri,
          count.bstri_elems * sizeof(uchartype) / count.bstri);
      bytes_used += count.bstri_elems * sizeof(uchartype);
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
          count.rtl_arr_elems * SIZ_REC(rtlObjecttype),
          count.rtl_arr_elems,
          (unsigned int) SIZ_REC(rtlObjecttype));
      bytes_used += count.rtl_arr_elems * SIZ_REC(rtlObjecttype);
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
          count.hsh_elems * SIZ_REC(rtlHelemtype),
          count.hsh_elems,
          (unsigned int) SIZ_REC(rtlHelemtype));
      bytes_used += count.hsh_elems * SIZ_REC(rtlHelemtype);
    } /* if */
    if (count.rtl_helem != 0) {
      printf("%9lu bytes in %8lu helems of              %4u bytes\n",
          count.rtl_helem * SIZ_REC(rtlHelemrecord),
          count.rtl_helem,
          (unsigned int) SIZ_REC(rtlHelemrecord));
      bytes_used += count.rtl_helem * SIZ_REC(rtlHelemrecord);
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
          count.set_elems * SIZ_REC(bitsettype),
          count.set_elems,
          (unsigned int) SIZ_REC(bitsettype));
      bytes_used += count.set_elems * SIZ_REC(bitsettype);
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
          count.sct_elems * SIZ_REC(objectrecord),
          count.sct_elems,
          (unsigned int) SIZ_REC(objectrecord));
      bytes_used += count.sct_elems * SIZ_REC(objectrecord);
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
          count.big_elems * sizeof_bigdigittype,
          count.big_elems,
          (unsigned int) sizeof_bigdigittype);
      bytes_used += count.big_elems * sizeof_bigdigittype;
    } /* if */
#endif
    if (count.polldata != 0) {
      printf("%9lu bytes in %8lu pollData elements of   %4u bytes\n",
          count.polldata * sizeof_pollrecord,
          count.polldata,
          (unsigned int) sizeof_pollrecord);
      bytes_used += count.polldata * sizeof_pollrecord;
    } /* if */
    if (count.win != 0) {
      printf("%9lu bytes in %8lu windows of             %4u bytes\n",
          count.win * sizeof_winrecord,
          count.win,
          (unsigned int) sizeof_winrecord);
      bytes_used += count.win * sizeof_winrecord;
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
#ifdef DO_HEAPSIZE_COMPUTATION
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
        (memsizetype) (chunk.beyond - chunk.freemem) + chunk.lost_bytes);
#endif
#ifdef TRACE_HEAPUTIL
    printf("END rtlHeapStatistic\n");
#endif
  } /* rtlHeapStatistic */
#endif
