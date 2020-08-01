/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  Module: General                                                 */
/*  File: seed7/src/flistutl.c                                      */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Procedures for free memory list maintainance.          */
/*                                                                  */
/*  This file contains the heapsize procedure which calculates      */
/*  the size of the currently used heap memory. This is done by     */
/*  subtracting the size of the free lists from the size of all     */
/*  memory requested with ALLOC_ macros. Note that the memory       */
/*  totally requested from the system is more than the memory       */
/*  requested with the ALLOC_ macros because of the chunk           */
/*  mechanism.                                                      */
/*  There is also the procedure reuse_free_lists which gives the    */
/*  memory occupied by the free lists back to the system. This      */
/*  makes only sense when the CHUNK_ALLOCS are turned off. This is  */
/*  because the chunk alloc mechanism is tuned for speed (and to    */
/*  work with free lists) and does not recycle any freed memory.    */
/*                                                                  */
/*  Here is the chunk allocation mechanism. All memory allocations  */
/*  with fixed small sizes are allocated out of chunks and are      */
/*  maintained in free lists. Only strings and large buffers        */
/*  are directly allocated with malloc. All heap requests run       */
/*  via macros defined in heaputil.h. This macros call malloc or    */
/*  use the chunk mechanism. Requests which run via the chunk       */
/*  mechanism should be done via macros defined in flistutl.h.      */
/*  This macros use free lists to maintain the free memory.         */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "flistutl.h"


#ifdef USE_CHUNK_ALLOCS
static unsigned int chunk_size[] = { 32768, 16384, 8192, 4096,
    2048, 1024, 512, 256, 128, 64, 0 };
#endif



#ifdef DO_HEAP_STATISTIC
#ifdef ANSI_C

static unsigned long object_flist_count (void)
#else

static unsigned long object_flist_count ()
#endif

  {
    register objecttype help_obj;
    register unsigned long num_objects;

  /* object_flist_count */
    num_objects = 0;
    help_obj = flist.objects;
    while (help_obj != NULL) {
      help_obj = help_obj->value.objvalue;
      num_objects++;
    } /* while */
    return(num_objects);
  } /* object_flist_count */



#ifdef ANSI_C

static unsigned long list_elem_flist_count (void)
#else

static unsigned long list_elem_flist_count ()
#endif

  {
    register listtype help_list;
    register unsigned long num_list_elems;

  /* list_elem_flist_count */
    num_list_elems = 0;
    help_list = flist.list_elems;
    while (help_list != NULL) {
      help_list = help_list->next;
      num_list_elems++;
    } /* while */
    return(num_list_elems);
  } /* list_elem_flist_count */



#ifdef ANSI_C

static unsigned long node_flist_count (void)
#else

static unsigned long node_flist_count ()
#endif

  {
    register nodetype help_node;
    register unsigned long num_nodes;

  /* node_flist_count */
    num_nodes = 0;
    help_node = flist.nodes;
    while (help_node != NULL) {
      help_node = help_node->next1;
      num_nodes++;
    } /* while */
    return(num_nodes);
  } /* node_flist_count */



#ifdef ANSI_C

static unsigned long infile_flist_count (void)
#else

static unsigned long infile_flist_count ()
#endif

  {
    register infiltype help_infile;
    register unsigned long num_infiles;

  /* infile_flist_count */
    num_infiles = 0;
    help_infile = flist.infiles;
    while (help_infile != NULL) {
      help_infile = help_infile->next;
      num_infiles++;
    } /* while */
    return(num_infiles);
  } /* infile_flist_count */



#ifdef ANSI_C

void heap_statistic (void)
#else

void heap_statistic ()
#endif

  {
    unsigned long num_flist_objects;
    unsigned long num_flist_list_elems;
    unsigned long num_flist_nodes;
    unsigned long num_flist_infiles;
    memsizetype bytes_used;
    memsizetype bytes_in_buffers;
    memsizetype bytes_free;
    memsizetype bytes_total;

  /* heap_statistic */
#ifdef TRACE_HEAPUTIL
    printf("BEGIN heap_statistic\n");
#endif
    num_flist_objects =    object_flist_count();
    num_flist_list_elems = list_elem_flist_count();
    num_flist_nodes =      node_flist_count();
    num_flist_infiles =    infile_flist_count();
    bytes_used = 0;
    if (count.stri != 0) {
      printf("%9lu bytes in %8lu string records of   %4u bytes\n",
          count.stri * SIZ_STRI(0),
          count.stri,
          SIZ_STRI(0));
      bytes_used += count.stri * SIZ_STRI(0);
      printf("%9lu bytes in %8lu string chars of     %4u bytes\n",
          count.stri_elems * sizeof(strelemtype),
          count.stri_elems,
          sizeof(strelemtype));
      bytes_used += count.stri_elems * sizeof(strelemtype);
    } /* if */
    if (count.bstri != 0) {
      printf("%9lu bytes in %8lu bstring records of  %4u bytes\n",
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
      printf("%9lu bytes in %8lu arrays of           %4u bytes\n",
          count.array * SIZ_ARR(0),
          count.array,
          SIZ_ARR(0));
      bytes_used += count.array * SIZ_ARR(0);
    } /* if */
    if (count.arr_elems != 0) {
      printf("%9lu bytes in %8lu array elements of   %4u bytes\n",
          count.arr_elems * SIZ_REC(objectrecord),
          count.arr_elems,
          SIZ_REC(objectrecord));
      bytes_used += count.arr_elems * SIZ_REC(objectrecord);
    } /* if */
    if (count.hash != 0) {
      printf("%9lu bytes in %8lu hashtables of       %4u bytes\n",
          count.hash * SIZ_HSH(0),
          count.hash,
          SIZ_HSH(0));
      bytes_used += count.hash * SIZ_HSH(0);
    } /* if */
    if (count.hsh_elems != 0) {
      printf("%9lu bytes in %8lu hashtable elems of  %4u bytes\n",
          count.hsh_elems * SIZ_REC(helemtype),
          count.hsh_elems,
          SIZ_REC(helemtype));
      bytes_used += count.hsh_elems * SIZ_REC(helemtype);
    } /* if */
    if (count.helem != 0) {
      printf("%9lu bytes in %8lu helems of           %4u bytes\n",
          count.helem * SIZ_REC(helemrecord),
          count.helem,
          SIZ_REC(helemrecord));
      bytes_used += count.helem * SIZ_REC(helemrecord);
    } /* if */
    if (count.set != 0) {
      printf("%9lu bytes in %8lu sets of             %4u bytes\n",
          count.set * SIZ_SET(0),
          count.set,
          SIZ_SET(0));
      bytes_used += count.set * SIZ_SET(0);
    } /* if */
    if (count.set_elems != 0) {
      printf("%9lu bytes in %8lu set elements of     %4u bytes\n",
          count.set_elems * SIZ_REC(bitsettype),
          count.set_elems,
          SIZ_REC(bitsettype));
      bytes_used += count.set_elems * SIZ_REC(bitsettype);
    } /* if */
    if (count.stru != 0) {
      printf("%9lu bytes in %8lu structs of          %4u bytes\n",
          count.stru * SIZ_SCT(0),
          count.stru,
          SIZ_SCT(0));
      bytes_used += count.stru * SIZ_SCT(0);
    } /* if */
    if (count.sct_elems != 0) {
      printf("%9lu bytes in %8lu struct elements of  %4u bytes\n",
          count.sct_elems * SIZ_REC(objectrecord),
          count.sct_elems,
          SIZ_REC(objectrecord));
      bytes_used += count.sct_elems * SIZ_REC(objectrecord);
    } /* if */
#ifdef USE_BIG_RTL_LIBRARY
    if (count.big != 0) {
      printf("%9lu bytes in %8lu bigIntegers of      %4u bytes\n",
          count.big * SIZ_BIG(0),
          count.big,
          SIZ_BIG(0));
      bytes_used += count.big * SIZ_BIG(0);
    } /* if */
    if (count.big_elems != 0) {
      printf("%9lu bytes in %8lu bigdigits of        %4u bytes\n",
          count.big_elems * sizeof_bigdigittype,
          count.big_elems,
          sizeof_bigdigittype);
      bytes_used += count.big_elems * sizeof_bigdigittype;
    } /* if */
#endif
    if (count.ident != 0) {
      printf("%9lu bytes in %8lu ident records of    %4u bytes\n",
          count.ident * SIZ_REC(identrecord),
          count.ident,
          SIZ_REC(identrecord));
      bytes_used += count.ident * SIZ_REC(identrecord);
    } /* if */
    if (count.idt != 0) {
      printf("%9lu bytes in %8lu idents of average   %4lu bytes\n",
          count.idt_bytes + count.idt,
          count.idt,
          (count.idt_bytes + count.idt) / count.idt);
      bytes_used += count.idt_bytes + count.idt;
    } /* if */
    if (count.entity != 0) {
      printf("%9lu bytes in %8lu entitys of          %4u bytes\n",
          count.entity * SIZ_REC(entityrecord),
          count.entity,
          SIZ_REC(entityrecord));
      bytes_used += count.entity * SIZ_REC(entityrecord);
    } /* if */
    if (count.property != 0) {
      printf("%9lu bytes in %8lu propertys of        %4u bytes\n",
          count.property * SIZ_REC(propertyrecord),
          count.property,
          SIZ_REC(propertyrecord));
      bytes_used += count.property * SIZ_REC(propertyrecord);
    } /* if */
    if (count.object > num_flist_objects) {
      printf("%9lu bytes in %8lu objects of          %4u bytes\n",
          (count.object - num_flist_objects) * SIZ_REC(objectrecord),
          count.object - num_flist_objects,
          SIZ_REC(objectrecord));
      bytes_used += (count.object - num_flist_objects) * SIZ_REC(objectrecord);
    } /* if */
    if (count.node > num_flist_nodes) {
      printf("%9lu bytes in %8lu nodes of            %4u bytes\n",
          (count.node - num_flist_nodes) * SIZ_REC(noderecord),
          count.node - num_flist_nodes,
          SIZ_REC(noderecord));
      bytes_used += (count.node - num_flist_nodes) * SIZ_REC(noderecord);
    } /* if */
    if (count.token != 0) {
      printf("%9lu bytes in %8lu tokens of           %4u bytes\n",
          count.token * SIZ_REC(tokenrecord),
          count.token,
          SIZ_REC(tokenrecord));
      bytes_used += count.token * SIZ_REC(tokenrecord);
    } /* if */
    if (count.owner != 0) {
      printf("%9lu bytes in %8lu owners of           %4u bytes\n",
          count.owner * SIZ_REC(ownerrecord),
          count.owner,
          SIZ_REC(ownerrecord));
      bytes_used += count.owner * SIZ_REC(ownerrecord);
    } /* if */
    if (count.stack != 0) {
      printf("%9lu bytes in %8lu stacks of           %4u bytes\n",
          count.stack * SIZ_REC(stackrecord),
          count.stack,
          SIZ_REC(stackrecord));
      bytes_used += count.stack * SIZ_REC(stackrecord);
    } /* if */
    if (count.typelist_elems != 0) {
      printf("%9lu bytes in %8lu typelist elems of   %4u bytes\n",
          count.typelist_elems * SIZ_REC(typelistrecord),
          count.typelist_elems,
          SIZ_REC(typelistrecord));
      bytes_used += count.typelist_elems * SIZ_REC(typelistrecord);
    } /* if */
    if (count.type != 0) {
      printf("%9lu bytes in %8lu types of            %4u bytes\n",
          count.type * SIZ_REC(typerecord),
          count.type,
          SIZ_REC(typerecord));
      bytes_used += count.type * SIZ_REC(typerecord);
    } /* if */
    if (count.list_elem > num_flist_list_elems) {
      printf("%9lu bytes in %8lu list_elems of       %4u bytes\n",
          (count.list_elem - num_flist_list_elems) * SIZ_REC(listrecord),
          count.list_elem - num_flist_list_elems,
          SIZ_REC(listrecord));
      bytes_used += (count.list_elem - num_flist_list_elems) * SIZ_REC(listrecord);
    } /* if */
    if (count.block != 0) {
      printf("%9lu bytes in %8lu blocks of           %4u bytes\n",
          count.block * SIZ_REC(blockrecord),
          count.block,
          SIZ_REC(blockrecord));
      bytes_used += count.block * SIZ_REC(blockrecord);
    } /* if */
    if (count.loclist != 0) {
      printf("%9lu bytes in %8lu loclists of         %4u bytes\n",
          count.loclist * SIZ_REC(loclistrecord),
          count.loclist,
          SIZ_REC(loclistrecord));
      bytes_used += count.loclist * SIZ_REC(loclistrecord);
    } /* if */
    if (count.infil > num_flist_infiles) {
      printf("%9lu bytes in %8lu files of            %4u bytes\n",
          (count.infil - num_flist_infiles) * SIZ_REC(infilrecord),
          count.infil - num_flist_infiles,
          SIZ_REC(infilrecord));
      bytes_used += (count.infil - num_flist_infiles) * SIZ_REC(infilrecord);
    } /* if */
    if (count.prog != 0) {
      printf("%9lu bytes in %8lu progs of            %4u bytes\n",
          count.prog * SIZ_REC(progrecord),
          count.prog,
          SIZ_REC(progrecord));
      bytes_used += count.prog * SIZ_REC(progrecord);
    } /* if */
    if (count.win != 0) {
      printf("%9lu bytes in %8lu windows of          %4lu bytes\n",
          count.win * count.size_winrecord,
          count.win,
          count.size_winrecord);
      bytes_used += count.win * count.size_winrecord;
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
    if (bytes_used != heapsize()) {
      printf("*** \nbytes_used=%lu heapsize=%lu diff=%ld\n",
          bytes_used, heapsize(), bytes_used - heapsize());
    } /* if */
    bytes_free = 0;
    if (num_flist_objects != 0) {
      printf("%9lu bytes in %8lu free objects of     %4u bytes\n",
          num_flist_objects * SIZ_REC(objectrecord),
          num_flist_objects,
          SIZ_REC(objectrecord));
      bytes_free += num_flist_objects * SIZ_REC(objectrecord);
    } /* if */
    if (num_flist_list_elems != 0) {
      printf("%9lu bytes in %8lu free list_elems of  %4u bytes\n",
          num_flist_list_elems * SIZ_REC(listrecord),
          num_flist_list_elems,
          SIZ_REC(listrecord));
      bytes_free += num_flist_list_elems * SIZ_REC(listrecord);
    } /* if */
    if (num_flist_nodes != 0) {
      printf("%9lu bytes in %8lu nodes of            %4u bytes\n",
          num_flist_nodes * SIZ_REC(noderecord),
          num_flist_nodes,
          SIZ_REC(noderecord));
      bytes_free += num_flist_nodes * SIZ_REC(noderecord);
    } /* if */
    if (num_flist_infiles != 0) {
      printf("%9lu bytes in %8lu infiles of          %4u bytes\n",
          num_flist_infiles * SIZ_REC(infilrecord),
          num_flist_infiles,
          SIZ_REC(infilrecord));
      bytes_free += num_flist_infiles * SIZ_REC(infilrecord);
    } /* if */
    bytes_total = bytes_used + bytes_free;
    printf("%9lu bytes total (with %lu bytes in free lists)\n",
        bytes_total,
        bytes_free);
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
#ifdef DO_HEAP_CHECK
    check_heap(0, __FILE__, __LINE__);
#endif
#ifdef TRACE_HEAPUTIL
    printf("END heap_statistic\n");
#endif
  } /* heap_statistic */
#endif



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

static memsizetype compute_hs (void)
#else

static memsizetype compute_hs ()
#endif

  {
    memsizetype bytes_total;

  /* compute_hs */
#ifdef TRACE_HEAPUTIL
    printf("BEGIN compute_hs\n");
#endif
    bytes_total = 0;
    bytes_total += count.stri * SIZ_STRI(0);
    bytes_total += count.stri_elems * sizeof(strelemtype);
    bytes_total += count.bstri * SIZ_BSTRI(0);
    bytes_total += count.bstri_elems * sizeof(uchartype);
    bytes_total += count.array * SIZ_ARR(0);
    bytes_total += count.arr_elems * SIZ_REC(objectrecord);
    bytes_total += count.hash * SIZ_HSH(0);
    bytes_total += count.hsh_elems * SIZ_REC(helemtype);
    bytes_total += count.hsh_elems * SIZ_REC(helemrecord);
    bytes_total += count.set * SIZ_SET(0);
    bytes_total += count.set_elems * SIZ_REC(bitsettype);
    bytes_total += count.stru * SIZ_SCT(0);
    bytes_total += count.sct_elems * SIZ_REC(objectrecord);
    bytes_total += count.big * SIZ_BIG(0);
    bytes_total += count.big_elems * SIZ_REC(bigdigittype);
    bytes_total += count.ident * SIZ_REC(identrecord);
    bytes_total += count.idt_bytes + count.idt;
    bytes_total += count.entity * SIZ_REC(entityrecord);
    bytes_total += count.property * SIZ_REC(propertyrecord);
    bytes_total += count.object * SIZ_REC(objectrecord);
    bytes_total += count.node * SIZ_REC(noderecord);
    bytes_total += count.token * SIZ_REC(tokenrecord);
    bytes_total += count.owner * SIZ_REC(ownerrecord);
    bytes_total += count.stack * SIZ_REC(stackrecord);
    bytes_total += count.typelist_elems * SIZ_REC(typelistrecord);
    bytes_total += count.type * SIZ_REC(typerecord);
    bytes_total += count.list_elem * SIZ_REC(listrecord);
    bytes_total += count.block * SIZ_REC(blockrecord);
    bytes_total += count.loclist * SIZ_REC(loclistrecord);
    bytes_total += count.infil * SIZ_REC(infilrecord);
    bytes_total += count.prog * SIZ_REC(progrecord);
    bytes_total += count.win * count.size_winrecord;
    bytes_total += count.fnam_bytes + count.fnam +
        count.symb_bytes + count.symb +
        count.byte;
#ifdef TRACE_HEAPUTIL
    printf("END compute_hs\n");
#endif
    return(bytes_total);
  } /* compute_hs */
#endif



#ifdef ANSI_C

memsizetype heapsize (void)
#else

memsizetype heapsize ()
#endif

  {
#ifdef DO_HEAPSIZE_COMPUTATION
    memsizetype flist_bytes;
#endif
    memsizetype result;

  /* heapsize */
#ifdef DO_HEAPSIZE_COMPUTATION
    {
      register objecttype help_obj;
      register memsizetype num_objects;

      num_objects = 0;
      help_obj = flist.objects;
      while (help_obj != NULL) {
        help_obj = help_obj->value.objvalue;
        num_objects++;
      } /* while */
/* printf("objects=%ld       SIZE=%d\n", num_objects, sizeof(objectrecord)); */
      flist_bytes = num_objects * sizeof(objectrecord);
    }

    {
      register listtype help_list;
      register memsizetype num_list_elems;

      num_list_elems = 0;
      help_list = flist.list_elems;
      while (help_list != NULL) {
        help_list = help_list->next;
        num_list_elems++;
      } /* while */
/* printf("list_elems=%ld    SIZE=%d\n", num_list_elems, sizeof(listrecord)); */
      flist_bytes += num_list_elems * sizeof(listrecord);
    }

    {
      register nodetype help_node;
      register memsizetype num_nodes;

      num_nodes = 0;
      help_node = flist.nodes;
      while (help_node != NULL) {
        help_node = help_node->next1;
        num_nodes++;
      } /* while */
/* printf("nodes=%ld    SIZE=%d\n", num_nodes, sizeof(noderecord)); */
      flist_bytes += num_nodes * sizeof(noderecord);
    }

    {
      register infiltype help_infile;
      register memsizetype num_infiles;

      num_infiles = 0;
      help_infile = flist.infiles;
      while (help_infile != NULL) {
        help_infile = help_infile->next;
        num_infiles++;
      } /* while */
/* printf("infiles=%ld       SIZE=%d\n", num_infiles, sizeof(infilrecord)); */
      flist_bytes += num_infiles * sizeof(infilrecord);
    }

/*  printf(" %ld ", hs); */
    result = hs - flist_bytes;
#else
    result = 0;
#endif
    return(result);
  } /* heapsize */



#ifdef USE_CHUNK_ALLOCS
#ifdef USE_FLIST_ALLOC
#ifdef ANSI_C

void *flist_alloc (size_t size)
#else

void *flist_alloc (size)
size_t size;
#endif

  {
    size_t size_of_rec;
    void *result;

  /* flist_alloc */
    if (size <= sizeof(listrecord) && flist.list_elems != NULL) {
      result = (void *) flist.list_elems;
      flist.list_elems = flist.list_elems->next;
      size_of_rec = sizeof(listrecord);
    } else if (size <= sizeof(objectrecord) && flist.objects != NULL) {
      result = (void *) flist.objects;
      flist.objects = flist.objects->TYPEOF;
      size_of_rec = sizeof(objectrecord);
    } else if (size <= sizeof(infilrecord) && flist.infiles != NULL) {
      result = (void *) flist.infiles;
      flist.infiles = flist.infiles->next;
      size_of_rec = sizeof(infilrecord);
    } else {
      result = NULL;
      size_of_rec = size;
    } /* if */
    LOST_B += size_of_rec - size;
/*  printf("flist_alloc(%d) ==> %lu\n", size, (long unsigned) result); */
    return(result);
  } /* flist_alloc */
#endif
#endif



#ifndef USE_CHUNK_ALLOCS
#ifdef ANSI_C

void reuse_free_lists (void)
#else

void reuse_free_lists ()
#endif

  {
    objecttype help_obj;
    listtype help_list;
    infiltype help_infile;

  /* reuse_free_lists */
    /* Free the flist.objects and the flist.list_elems lists */
    while (flist.objects != NULL) {
      help_obj = flist.objects;
      flist.objects = flist.objects->TYPEOF;
      FREE_RECORD(help_obj, objectrecord, count.object);
    } /* while */
    while (flist.list_elems != NULL) {
      help_list = flist.list_elems;
      flist.list_elems = flist.list_elems->next;
      FREE_RECORD(help_list, listrecord, count.list_elem);
    } /* while */
    while (flist.infiles != NULL) {
      help_infile = flist.infiles;
      flist.infiles = flist.infiles->next;
      FREE_RECORD(help_infile, infilrecord, count.infil);
    } /* while */
  } /* reuse_free_lists */
#endif



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

stritype alloc_s (memsizetype len)
#else

stritype alloc_s (len)
memsizetype len;
#endif

  {
    stritype result;

  /* alloc_s */
/*  printf("<+%ld>", len); */
    if (len >= 256 || flist.stris[len] == NULL) {
      HEAP_STRI(result, len);
    } else {
      POP_STRI(result, len);
    } /* if */
    return(result);
  } /* alloc_s */



#ifdef ANSI_C

void free_stri (stritype stri, memsizetype len)
#else

void free_stri (stri, len)
stritype stri;
memsizetype len;
#endif

  { /* free_stri */
/*  printf("<-%ld>", len); */
    if (len < 256) {
      stri->SIZE = (memsizetype) flist.stris[len];
      flist.stris[len] = stri;
    } else {
      FREE_CHUNK(stri, len);
    } /* if */
  } /* free_stri */



#ifdef ANSI_C

stritype resize_s (stritype stri, memsizetype len1, memsizetype len2)
#else

stritype resize_s (stri, len1, len2)
stritype stri;
memsizetype len1;
memsizetype len2;
#endif

  {
    stritype result;

  /* resize_s */
    ALLOC_STRI_CHECK_SIZE(result, len2);
/*  printf("X"); */
    memcpy(result, stri, len2 > len1 ? SIZ_STRI(len1) : SIZ_STRI(len2));
    free_stri(stri, len1);
    return(result);
  } /* resize_s */
#endif



#ifdef USE_CHUNK_ALLOCS
#ifdef ANSI_C

void *heap_chunk (size_t size)
#else

void *heap_chunk (size)
size_t size;
#endif

  {
    int index;
    listtype list_elem;
    void *result;

  /* heap_chunk */
#ifdef TRACE_HEAPUTIL
    printf("BEGIN heap_chunk\n");
#endif
/*  printf("%lu ", heapsize()); */
    index = 0;
    result = NULL;
    while (chunk_size[index] >= size && chunk_size[index] != 0 &&
        (result = (char *) MALLOC(chunk_size[index])) == NULL) {
      index++;
    } /* while */
    if (result == NULL) {
#ifdef USE_FLIST_ALLOC
      if ((result = flist_alloc(size)) == NULL) {
        result = MALLOC(size);
      } /* if */
#else
      result = MALLOC(size);
#endif
    } else {
      if (chunk.start != NULL) {
#ifdef RESIZE_CHUNKS
        if (realloc(chunk.start, (size_t) (chunk.freemem - chunk.start)) != chunk.start) {
          no_memory(SOURCE_POSITION(1000));
        } /* if */
#else
        while (sizeof(listrecord) <= (memsizetype) (chunk.beyond - chunk.freemem)) {
          if (OLD_CHUNK(list_elem, listtype, sizeof(listrecord))) {
            COUNT_FLISTELEM(listrecord, count.list_elem);
            FREE_L_ELEM(list_elem);
          } /* if */
        } /* while */
        chunk.lost_bytes += (memsizetype) (chunk.beyond - chunk.freemem);
#endif
      } /* if */
      chunk.size = chunk_size[index];
      chunk.total_size += chunk.size;
      chunk.start = (char *) result;
      chunk.beyond = chunk.start + chunk.size;
      chunk.freemem = chunk.start + size;
      chunk.size -= size;
      chunk.number_of_chunks++;
    } /* if */
#ifdef TRACE_HEAPUTIL
    printf("END heap_chunk\n");
#endif
    return(result);
  } /* heap_chunk */
#endif



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
        ((memsizetype) count.property)       * SIZ_REC(propertyrecord) +
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
