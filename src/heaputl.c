/********************************************************************/
/*                                                                  */
/*  heaputl.c     Procedures for heap allocation and maintainance.  */
/*  Copyright (C) 1989 - 2005  Thomas Mertes                        */
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
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Procedures for heap allocation and maintainance.       */
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



#ifdef WITH_STRI_CAPACITY
#ifdef ANSI_C

stritype growStri (stritype stri, memsizetype len)
#else

stritype growStri (stri, len)
stritype stri;
memsizetype len;
#endif

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
        result->capacity = new_len;
      } /* if */
    } /* if */
    /* printf(" => %lX\n", result);
    fflush(stdout); */
    return(result);
  } /* growStri */



#ifdef ANSI_C

stritype shrinkStri (stritype stri, memsizetype len)
#else

stritype shrinkStri (stri, len)
stritype stri;
memsizetype len;
#endif

  {
    memsizetype new_len;
    stritype result;

  /* shrinkStri */
    new_len = 2 * len;
    /* printf("shrinkStri(%lX, %lu) size=%u, capacity=%u, new_len=%u, siz_stri=%u, sizeof=%u\n",
         stri, len, stri->size, stri->capacity, new_len, SIZ_STRI(new_len), sizeof(strirecord));
    fflush(stdout); */
    result = REALLOC_HEAP(stri, stritype, SIZ_STRI(new_len));
    result->capacity = new_len;
    return(result);
  } /* shrinkStri */
#endif



#ifdef DO_HEAP_STATISTIC
#ifdef ANSI_C

void rtlHeapStatistic (void)
#else

void rtlHeapStatistic ()
#endif

  {
    memsizetype bytes_used;
    memsizetype bytes_total;

  /* rtlHeapStatistic */
#ifdef TRACE_HEAPUTIL
    printf("BEGIN heap_statistic\n");
#endif
    bytes_used = 0;
    if (count.stri != 0) {
      printf("%9lu bytes in %8lu string records of   %4d bytes\n",
          count.stri * SIZ_STRI(0),
          count.stri,
          SIZ_STRI(0));
      bytes_used += count.stri * SIZ_STRI(0);
      printf("%9lu bytes in %8lu string chars of     %4d bytes\n",
          count.stri_elems * sizeof(strelemtype),
          count.stri_elems,
          sizeof(strelemtype));
      bytes_used += count.stri_elems * sizeof(strelemtype);
    } /* if */
    if (count.bstri != 0) {
      printf("%9lu bytes in %8lu bstring records of  %4d bytes\n",
          count.bstri * SIZ_BSTRI(0),
          count.bstri,
          SIZ_BSTRI(0));
      bytes_used += count.bstri * SIZ_BSTRI(0);
      printf("%9lu bytes in %8lu bstrings of average %4lu bytes\n",
          count.bstri_elems * sizeof(uchartype),
          count.bstri,
          count.bstri_elems * sizeof(uchartype) / count.bstri);
      bytes_used += count.bstri_elems * sizeof(uchartype);
    } /* if */
    if (count.array != 0) {
      printf("%9lu bytes in %8lu arrays of           %4d bytes\n",
          count.array * SIZ_RTL_ARR(0),
          count.array,
          SIZ_RTL_ARR(0));
      bytes_used += count.array * SIZ_RTL_ARR(0);
    } /* if */
    if (count.rtl_arr_elems != 0) {
      printf("%9lu bytes in %8lu array elements of   %4d bytes\n",
          count.rtl_arr_elems * SIZ_REC(rtlObjecttype),
          count.rtl_arr_elems,
          SIZ_REC(rtlObjecttype));
      bytes_used += count.rtl_arr_elems * SIZ_REC(rtlObjecttype);
    } /* if */
    if (count.hash != 0) {
      printf("%9lu bytes in %8lu hashtables of       %4d bytes\n",
          count.hash * SIZ_RTL_HSH(0),
          count.hash,
          SIZ_RTL_HSH(0));
      bytes_used += count.hash * SIZ_RTL_HSH(0);
    } /* if */
    if (count.hsh_elems != 0) {
      printf("%9lu bytes in %8lu hashtable elems of  %4d bytes\n",
          count.hsh_elems * SIZ_REC(rtlHelemtype),
          count.hsh_elems,
          SIZ_REC(rtlHelemtype));
      bytes_used += count.hsh_elems * SIZ_REC(rtlHelemtype);
    } /* if */
    if (count.rtl_helem != 0) {
      printf("%9lu bytes in %8lu helems of           %4d bytes\n",
          count.rtl_helem * SIZ_REC(rtlHelemrecord),
          count.rtl_helem,
          SIZ_REC(rtlHelemrecord));
      bytes_used += count.rtl_helem * SIZ_REC(rtlHelemrecord);
    } /* if */
    if (count.set != 0) {
      printf("%9lu bytes in %8lu sets of             %4d bytes\n",
          count.set * SIZ_SET(0),
          count.set,
          SIZ_SET(0));
      bytes_used += count.set * SIZ_SET(0);
    } /* if */
    if (count.set_elems != 0) {
      printf("%9lu bytes in %8lu set elements of     %4d bytes\n",
          count.set_elems * SIZ_REC(bitsettype),
          count.set_elems,
          SIZ_REC(bitsettype));
      bytes_used += count.set_elems * SIZ_REC(bitsettype);
    } /* if */
#if 0
    if (count.stru != 0) {
      printf("%9lu bytes in %8lu structs of          %4d bytes\n",
          count.stru * SIZ_SCT(0),
          count.stru,
          SIZ_SCT(0));
      bytes_used += count.stru * SIZ_SCT(0);
    } /* if */
    if (count.sct_elems != 0) {
      printf("%9lu bytes in %8lu struct elements of  %4d bytes\n",
          count.sct_elems * SIZ_REC(objectrecord),
          count.sct_elems,
          SIZ_REC(objectrecord));
      bytes_used += count.sct_elems * SIZ_REC(objectrecord);
    } /* if */
#endif
#ifdef USE_BIG_RTL_LIBRARY
    if (count.big != 0) {
      printf("%9lu bytes in %8lu bigIntegers of      %4d bytes\n",
          count.big * SIZ_BIG(0),
          count.big,
          SIZ_BIG(0));
      bytes_used += count.big * SIZ_BIG(0);
    } /* if */
    if (count.big_elems != 0) {
      printf("%9lu bytes in %8lu bigdigits of        %4d bytes\n",
          count.big_elems * sizeof_bigdigittype,
          count.big_elems,
          sizeof_bigdigittype);
      bytes_used += count.big_elems * sizeof_bigdigittype;
    } /* if */
#endif
    if (count.win != 0) {
      printf("%9lu bytes in %8lu windows of          %4d bytes\n",
          count.win * count.size_winrecord,
          count.win,
          count.size_winrecord);
      bytes_used += count.win * count.size_winrecord;
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
    printf("%9lu bytes in %8u chunks\n",
        chunk.total_size, chunk.number_of_chunks);
    printf("%9u unused bytes in last chunk\n",
        (unsigned) (chunk.beyond - chunk.freemem));
    printf("%9lu lost bytes in chunks\n", chunk.lost_bytes);
    printf("%9lu bytes total requested\n", bytes_total +
        (memsizetype) (chunk.beyond - chunk.freemem) + chunk.lost_bytes);
#endif
#ifdef TRACE_HEAPUTIL
    printf("END heap_statistic\n");
#endif
  } /* rtlHeapStatistic */
#endif
