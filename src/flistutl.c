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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
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



#ifdef DO_HEAP_STATISTIK
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
    memsizetype bytes_free;

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
      printf("%9lu bytes in %8lu string records of  %4d bytes\n",
          count.stri * SIZ_STRI(0),
          count.stri,
          SIZ_STRI(0));
      bytes_used += count.stri * SIZ_STRI(0);
      printf("%9lu bytes in %8lu strings of average %4lu bytes\n",
          count.stri_elems * sizeof(strelemtype),
          count.stri,
          count.stri_elems * sizeof(strelemtype) / count.stri);
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
      bytes_used += count.bstri_elems * sizeof(strelemtype);
    } /* if */
    if (count.array != 0) {
      printf("%9lu bytes in %8lu arrays of          %4d bytes\n",
          count.array * SIZ_ARR(0),
          count.array,
          SIZ_ARR(0));
      bytes_used += count.array * SIZ_ARR(0);
    } /* if */
    if (count.arr_elems != 0) {
      printf("%9lu bytes in %8lu array elements of  %4d bytes\n",
          count.arr_elems * SIZ_REC(objectrecord),
          count.arr_elems,
          SIZ_REC(objectrecord));
      bytes_used += count.arr_elems * SIZ_REC(objectrecord);
    } /* if */
    if (count.hash != 0) {
      printf("%9lu bytes in %8lu hashtables of      %4d bytes\n",
          count.hash * SIZ_HSH(0),
          count.hash,
          SIZ_HSH(0));
      bytes_used += count.hash * SIZ_HSH(0);
    } /* if */
    if (count.hsh_elems != 0) {
      printf("%9lu bytes in %8lu hashtable elems of %4d bytes\n",
          count.hsh_elems * SIZ_REC(helemtype),
          count.hsh_elems,
          SIZ_REC(helemtype));
      bytes_used += count.hsh_elems * SIZ_REC(helemtype);
    } /* if */
    if (count.helem != 0) {
      printf("%9lu bytes in %8lu helems of          %4d bytes\n",
          count.helem * SIZ_REC(helemrecord),
          count.helem,
          SIZ_REC(helemrecord));
      bytes_used += count.hsh_elems * SIZ_REC(helemrecord);
    } /* if */
    if (count.set != 0) {
      printf("%9lu bytes in %8lu sets of            %4d bytes\n",
          count.set * SIZ_SET(0),
          count.set,
          SIZ_SET(0));
      bytes_used += count.set * SIZ_SET(0);
    } /* if */
    if (count.set_elems != 0) {
      printf("%9lu bytes in %8lu set elements of    %4d bytes\n",
          count.set_elems * SIZ_REC(bitsettype),
          count.set_elems,
          SIZ_REC(bitsettype));
      bytes_used += count.set_elems * SIZ_REC(bitsettype);
    } /* if */
    if (count.stru != 0) {
      printf("%9lu bytes in %8lu structs of         %4d bytes\n",
          count.stru * SIZ_SCT(0),
          count.stru,
          SIZ_SCT(0));
      bytes_used += count.stru * SIZ_SCT(0);
    } /* if */
    if (count.sct_elems != 0) {
      printf("%9lu bytes in %8lu struct elements of %4d bytes\n",
          count.sct_elems * SIZ_REC(objectrecord),
          count.sct_elems,
          SIZ_REC(objectrecord));
      bytes_used += count.sct_elems * SIZ_REC(objectrecord);
    } /* if */
    if (count.ident != 0) {
      printf("%9lu bytes in %8lu ident records of   %4d bytes\n",
          count.ident * SIZ_REC(identrecord),
          count.ident,
          SIZ_REC(identrecord));
      bytes_used += count.ident * SIZ_REC(identrecord);
    } /* if */
    if (count.idt != 0) {
      printf("%9lu bytes in %8lu idents of average  %4lu bytes\n",
          count.idt_bytes + count.idt,
          count.idt,
          (count.idt_bytes + count.idt) / count.idt);
      bytes_used += count.idt_bytes + count.idt;
    } /* if */
    if (count.entity != 0) {
      printf("%9lu bytes in %8lu entitys of         %4d bytes\n",
          count.entity * SIZ_REC(entityrecord),
          count.entity,
          SIZ_REC(entityrecord));
      bytes_used += count.entity * SIZ_REC(entityrecord);
    } /* if */
    if (count.object - num_flist_objects != 0) {
      printf("%9lu bytes in %8lu objects of         %4d bytes\n",
          (count.object - num_flist_objects) * SIZ_REC(objectrecord),
          count.object - num_flist_objects,
          SIZ_REC(objectrecord));
      bytes_used += (count.object - num_flist_objects) * SIZ_REC(objectrecord);
    } /* if */
    if (count.node - num_flist_nodes != 0) {
      printf("%9lu bytes in %8lu nodes of           %4d bytes\n",
          (count.node - num_flist_nodes) * SIZ_REC(noderecord),
          count.node - num_flist_nodes,
          SIZ_REC(noderecord));
      bytes_used += (count.node - num_flist_nodes) * SIZ_REC(noderecord);
    } /* if */
    if (count.token != 0) {
      printf("%9lu bytes in %8lu tokens of          %4d bytes\n",
          count.token * SIZ_REC(tokenrecord),
          count.token,
          SIZ_REC(tokenrecord));
      bytes_used += count.token * SIZ_REC(tokenrecord);
    } /* if */
    if (count.owner != 0) {
      printf("%9lu bytes in %8lu owners of          %4d bytes\n",
          count.owner * SIZ_REC(ownerrecord),
          count.owner,
          SIZ_REC(ownerrecord));
      bytes_used += count.owner * SIZ_REC(ownerrecord);
    } /* if */
    if (count.stack != 0) {
      printf("%9lu bytes in %8lu stacks of          %4d bytes\n",
          count.stack * SIZ_REC(stackrecord),
          count.stack,
          SIZ_REC(stackrecord));
      bytes_used += count.stack * SIZ_REC(stackrecord);
    } /* if */
    if (count.type != 0) {
      printf("%9lu bytes in %8lu types of           %4d bytes\n",
          count.type * SIZ_REC(typerecord),
          count.type,
          SIZ_REC(typerecord));
      bytes_used += count.type * SIZ_REC(typerecord);
    } /* if */
    if (count.list_elem - num_flist_list_elems != 0) {
      printf("%9lu bytes in %8lu list_elems of      %4d bytes\n",
          (count.list_elem - num_flist_list_elems) * SIZ_REC(listrecord),
          count.list_elem - num_flist_list_elems,
          SIZ_REC(listrecord));
      bytes_used += (count.list_elem - num_flist_list_elems) * SIZ_REC(listrecord);
    } /* if */
    if (count.block != 0) {
      printf("%9lu bytes in %8lu blocks of          %4d bytes\n",
          count.block * SIZ_REC(blockrecord),
          count.block,
          SIZ_REC(blockrecord));
      bytes_used += count.block * SIZ_REC(blockrecord);
    } /* if */
    if (count.loclist != 0) {
      printf("%9lu bytes in %8lu loclists of        %4d bytes\n",
          count.loclist * SIZ_REC(loclistrecord),
          count.loclist,
          SIZ_REC(loclistrecord));
      bytes_used += count.loclist * SIZ_REC(loclistrecord);
    } /* if */
    if (count.infil - num_flist_infiles != 0) {
      printf("%9lu bytes in %8lu files of           %4d bytes\n",
          (count.infil - num_flist_infiles) * SIZ_REC(infilrecord),
          count.infil - num_flist_infiles,
          SIZ_REC(infilrecord));
      bytes_used += (count.infil - num_flist_infiles) * SIZ_REC(infilrecord);
    } /* if */
    if (count.prog != 0) {
      printf("%9lu bytes in %8lu progs of           %4d bytes\n",
          count.prog * SIZ_REC(progrecord),
          count.prog,
          SIZ_REC(progrecord));
      bytes_used += count.prog * SIZ_REC(progrecord);
    } /* if */
    printf("%9lu bytes in buffers\n",
        count.fnam_bytes + count.fnam +
        count.symb_bytes + count.symb +
        count.byte);
    bytes_used += count.fnam_bytes + count.fnam +
        count.symb_bytes + count.symb +
        count.byte;
    printf("%9lu bytes in use\n", bytes_used);
    if (bytes_used != heapsize()) {
      printf("*** \nbytes_used=%lu heapsize=%lu diff=%ld\n",
          bytes_used, heapsize(), bytes_used - heapsize());
    } /* if */
    bytes_free = 0;
    if (num_flist_objects != 0) {
      printf("%9lu bytes in %8lu free objects of    %4d bytes\n",
          num_flist_objects * SIZ_REC(objectrecord),
          num_flist_objects,
          SIZ_REC(objectrecord));
      bytes_free += num_flist_objects * SIZ_REC(objectrecord);
    } /* if */
    if (num_flist_list_elems != 0) {
      printf("%9lu bytes in %8lu free list_elems of %4d bytes\n",
          num_flist_list_elems * SIZ_REC(listrecord),
          num_flist_list_elems,
          SIZ_REC(listrecord));
      bytes_free += num_flist_list_elems * SIZ_REC(listrecord);
    } /* if */
    if (num_flist_nodes != 0) {
      printf("%9lu bytes in %8lu nodes of           %4d bytes\n",
          num_flist_nodes * SIZ_REC(noderecord),
          num_flist_nodes,
          SIZ_REC(noderecord));
      bytes_free += num_flist_nodes * SIZ_REC(noderecord);
    } /* if */
    if (num_flist_infiles != 0) {
      printf("%9lu bytes in %8lu infiles of         %4d bytes\n",
          num_flist_infiles * SIZ_REC(infilrecord),
          num_flist_infiles,
          SIZ_REC(infilrecord));
      bytes_free += num_flist_infiles * SIZ_REC(infilrecord);
    } /* if */
    bytes_used += bytes_free;
    printf("%9lu bytes total (with %lu bytes in free lists)\n",
        bytes_used,
        bytes_free);
    if (bytes_used != hs) {
      printf("*** \nbytes_used=%lu hs=%lu diff=%ld\n",
          bytes_used, hs, bytes_used - hs);
    } /* if */
#ifdef USE_CHUNK_ALLOCS
    printf("%9lu bytes in %8u chunks\n",
        chunk.total_size, chunk.number_of_chunks);
    printf("%9u unused bytes in last chunk\n",
        (unsigned) (chunk.byond - chunk.freemem));
    printf("%9lu lost bytes in chunks\n", chunk.lost_bytes);
    printf("%9lu bytes total requested\n", hs +
        (memsizetype) (chunk.byond - chunk.freemem) + chunk.lost_bytes);
#endif
#ifdef TRACE_HEAPUTIL
    printf("END heap_statistic\n");
#endif
  } /* heap_statistic */
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

void *flist_alloc (SIZE_TYPE size)
#else

void *flist_alloc (size)
SIZE_TYPE size;
#endif

  {
    SIZE_TYPE size_of_rec;
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
      FREE_RECORD(help_obj, objectrecord, OBJT_COUNT);
    } /* while */
    while (flist.list_elems != NULL) {
      help_list = flist.list_elems;
      flist.list_elems = flist.list_elems->next;
      FREE_RECORD(help_list, listrecord, LIST_ELEMENT_COUNT);
    } /* while */
    while (flist.infiles != NULL) {
      help_infile = flist.infiles;
      flist.infiles = flist.infiles->next;
      FREE_RECORD(help_infile, infilrecord, INFIL_COUNT);
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
    ALLOC_STRI(result, len2);
/*  printf("X"); */
    memcpy(result, stri, len2 > len1 ? SIZ_STRI(len1) : SIZ_STRI(len2));
    free_stri(stri, len1);
    return(result);
  } /* resize_s */
#endif



#ifdef USE_CHUNK_ALLOCS
#ifdef ANSI_C

void *heap_chunk (SIZE_TYPE size)
#else

void *heap_chunk (size)
SIZE_TYPE size;
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
        if (realloc(chunk.start, (SIZE_TYPE) (chunk.freemem - chunk.start)) != chunk.start) {
          no_memory(SOURCE_POSITION(1000));
        } /* if */
#else
        while (sizeof(listrecord) <= chunk.byond - chunk.freemem) {
          if (OLD_CHUNK(list_elem, listtype, sizeof(listrecord))) {
            COUNT_FLISTELEM(listrecord, count.list_elem);
            FREE_L_ELEM(list_elem);
          } /* if */
        } /* while */
        chunk.lost_bytes += (memsizetype) (chunk.byond - chunk.freemem);
#endif
      } /* if */
      chunk.size = chunk_size[index];
      chunk.total_size += chunk.size;
      chunk.start = (char *) result;
      chunk.byond = chunk.start + chunk.size;
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
