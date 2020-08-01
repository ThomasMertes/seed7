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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
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

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "heaputl.h"



#ifdef DO_HEAP_CHECK
#ifdef ANSI_C

void check_heap (long sizediff, char *file_name, unsigned int line_num)
#else

void check_heap (sizediff)
long sizediff;
#endif

  {
    memsizetype bytes_used;

  /* check_heap */
#ifdef TRACE_HEAPUTIL
    printf("BEGIN check_heap\n");
#endif
    bytes_used =
        ((memsizetype) count.stri) * SIZ_STRI(0) +
        count.stri_elems * sizeof(strelemtype) +
        ((memsizetype) count.bstri) * SIZ_BSTRI(0) +
        count.bstri_elems * sizeof(uchartype) +
        ((memsizetype) count.array) * SIZ_ARR(0) +
        count.arr_elems * SIZ_REC(objectrecord) +
        ((memsizetype) count.hash) * SIZ_HSH(0) +
        count.hsh_elems * SIZ_REC(helemtype) +
        ((memsizetype) count.helem) * SIZ_REC(helemrecord) +
        ((memsizetype) count.set) * SIZ_SET(0) +
        count.set_elems * SIZ_REC(bitsettype) +
        ((memsizetype) count.stru) * SIZ_SCT(0) +
        count.sct_elems * SIZ_REC(objectrecord) +
        ((memsizetype) count.big) * SIZ_BIG(0) +
        count.big_elems * SIZ_REC(bigdigittype) +
        ((memsizetype) count.ident) * SIZ_REC(identrecord) +
        count.idt_bytes + ((memsizetype) count.idt) +
        ((memsizetype) count.entity)         * SIZ_REC(entityrecord) +
        ((memsizetype) count.object)         * SIZ_REC(objectrecord) +
        ((memsizetype) count.node)           * SIZ_REC(noderecord) +
        ((memsizetype) count.token)          * SIZ_REC(tokenrecord) +
        ((memsizetype) count.owner)          * SIZ_REC(ownerrecord) +
        ((memsizetype) count.stack)          * SIZ_REC(stackrecord) +
        ((memsizetype) count.typelist_elems) * SIZ_REC(typelistrecord) +
        ((memsizetype) count.type)           * SIZ_REC(typerecord) +
        ((memsizetype) count.list_elem)      * SIZ_REC(listrecord) +
        ((memsizetype) count.block)          * SIZ_REC(blockrecord) +
        ((memsizetype) count.loclist)        * SIZ_REC(loclistrecord) +
        ((memsizetype) count.infil)          * SIZ_REC(infilrecord) +
        ((memsizetype) count.prog)           * SIZ_REC(progrecord) +
        ((memsizetype) count.win)            * count.size_winrecord +
        count.fnam_bytes + ((memsizetype) count.fnam) +
        count.symb_bytes + ((memsizetype) count.symb) +
        count.byte;
    if (bytes_used != hs) {
      printf("*** %s(%u)\n%lu %lu %ld %ld \n",
          file_name, line_num, bytes_used, hs, bytes_used - hs, sizediff);
/*  } else {
      printf("\n%lu %ld %d \n", hs, sizediff, in_file.line);
*/  } /* if */
    /* if (sizediff > 0) {
      printf("\nalloc(%ld)\n", sizediff);
    } else {
      printf("\nfree(%ld)\n", -sizediff);
    } if */
/*  show_statistic(); */
#ifdef TRACE_HEAPUTIL
    printf("END check_heap\n");
#endif
  } /* check_heap */
#endif
