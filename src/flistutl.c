/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1989 - 2019, 2021  Thomas Mertes                  */
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
/*  Changes: 1993, 1994, 2010, 2013, 2015, 2019  Thomas Mertes      */
/*           2021  Thomas Mertes                                    */
/*  Content: Procedures for free memory list maintenance.           */
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

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "stdlib.h"
#include "stdio.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "data_rtl.h"
#include "sql_drv.h"
#include "heaputl.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "flistutl.h"


#if USE_CHUNK_ALLOCS
static const unsigned int chunk_size[] = { 32768, 16384, 8192, 4096,
    2048, 1024, 512, 256, 128, 64, 0 };
#endif



#ifdef OUT_OF_ORDER
boolType check_obj_flist (objectType object)

  {
    objectType help_obj;
    objectType next_obj;

  /* check_obj_flist */
    help_obj = flist.objects;
    while (help_obj != NULL) {
      next_obj = help_obj->value.objValue;
      if (help_obj == object) {
        help_obj->value.objValue = NULL;
        printf("****************************************\n");
        printf("object in flist\n%ld %08lx ", refNum(help_obj), (unsigned long int) help_obj);
        trace1(help_obj);
        printf("\n");
        help_obj->value.objValue = next_obj;
        /* printf("%d", 1/0); */
        return FALSE;
      } /* if */
      help_obj = next_obj;
    } /* while */
    return TRUE;
  } /* check_obj_flist */
#endif



#if DO_HEAPSIZE_COMPUTATION || DO_HEAP_STATISTIC
static unsigned long object_flist_count (void)

  {
    register objectType help_obj;
    register unsigned long num_objects = 0;

  /* object_flist_count */
    help_obj = flist.objects;
    while (help_obj != NULL) {
      help_obj = help_obj->value.objValue;
      num_objects++;
    } /* while */
    return num_objects;
  } /* object_flist_count */



static unsigned long list_elem_flist_count (void)

  {
    register listType help_list;
    register unsigned long num_list_elems = 0;

  /* list_elem_flist_count */
    help_list = flist.list_elems;
    while (help_list != NULL) {
      help_list = help_list->next;
      num_list_elems++;
    } /* while */
    return num_list_elems;
  } /* list_elem_flist_count */



static unsigned long node_flist_count (void)

  {
    register nodeType help_node;
    register unsigned long num_nodes = 0;

  /* node_flist_count */
    help_node = flist.nodes;
    while (help_node != NULL) {
      help_node = help_node->next1;
      num_nodes++;
    } /* while */
    return num_nodes;
  } /* node_flist_count */



static unsigned long infile_flist_count (void)

  {
    register inFileType help_infile;
    register unsigned long num_infiles = 0;

  /* infile_flist_count */
    help_infile = flist.infiles;
    while (help_infile != NULL) {
      help_infile = help_infile->next;
      num_infiles++;
    } /* while */
    return num_infiles;
  } /* infile_flist_count */



static unsigned long stri_flist_count (unsigned long *stri_chars)

  {
#if WITH_STRI_FREELIST
    register freeListElemType help_elem;
    register unsigned long num_elems;
#endif
    register unsigned long num_stris = 0;

  /* stri_flist_count */
#if WITH_STRI_FREELIST
#if WITH_STRI_CAPACITY
    {
      unsigned int index;

      *stri_chars = 0;
      for (index = 0; index < STRI_FREELIST_ARRAY_SIZE; index++) {
        num_elems = 0;
        help_elem = sflist[index];
        while (help_elem != NULL) {
#if 0
          striType stri;
          memSizeType saved_size;

          stri = (striType) help_elem;
          saved_size = stri->size;
          stri->size = stri->capacity;
          prot_stri(stri);
          prot_nl();
          stri->size = saved_size;
#endif
          help_elem = help_elem->next;
          num_elems++;
        } /* while */
        num_stris += num_elems;
        *stri_chars += index * num_elems;
        /* printf("sflist[%d]=%lu %lu\n", index, num_elems, index * num_elems); */
      } /* for */
      /* printf("num_stris=%lu\n", num_stris);
         printf("stri_chars=%lu\n", *stri_chars);
         printf("count.stri=%lu\n", count.stri);
         printf("count.stri_elems=%lu\n", count.stri_elems); */
    }
#else
    num_elems = 0;
    help_elem = sflist;
    while (help_elem != NULL) {
      help_elem = help_elem->next;
      num_elems++;
    } /* while */
    num_stris = num_elems;
    *stri_chars = num_elems;
#endif
#else
    *stri_chars = 0;
#endif
    return num_stris;
  } /* stri_flist_count */
#endif



#if DO_HEAP_STATISTIC
void heapStatistic (void)

  {
    unsigned long num_flist_objects;
    unsigned long num_flist_list_elems;
    unsigned long num_flist_nodes;
    unsigned long num_flist_infiles;
    unsigned long num_flist_stris;
    unsigned long num_flist_stri_elems;
    memSizeType bytes_used;
    memSizeType bytes_in_buffers;
    memSizeType bytes_free;
    memSizeType bytes_total;

  /* heapStatistic */
    logFunction(printf("heapStatistic\n"););
    num_flist_objects    = object_flist_count();
    num_flist_list_elems = list_elem_flist_count();
    num_flist_nodes      = node_flist_count();
    num_flist_infiles    = infile_flist_count();
    num_flist_stris      = stri_flist_count(&num_flist_stri_elems);
    bytes_used = 0;
    if (count.stri > num_flist_stris) {
      printf(F_U_MEM(9) " bytes in %8lu string records of      %4u bytes\n",
          (count.stri - num_flist_stris) * SIZ_STRI(0),
          count.stri - num_flist_stris,
          (unsigned int) SIZ_STRI(0));
      bytes_used += (count.stri - num_flist_stris) * SIZ_STRI(0);
      printf(F_U_MEM(9) " bytes in " F_U_MEM(8) " string chars of        %4u bytes\n",
          (count.stri_elems - num_flist_stri_elems) * sizeof(strElemType),
          count.stri_elems - num_flist_stri_elems,
          (unsigned int) sizeof(strElemType));
      bytes_used += (count.stri_elems - num_flist_stri_elems) * sizeof(strElemType);
    } /* if */
    if (count.bstri != 0) {
      printf(F_U_MEM(9) " bytes in %8lu bstring records of     %4u bytes\n",
          count.bstri * SIZ_BSTRI(0),
          count.bstri,
          (unsigned int) SIZ_BSTRI(0));
      bytes_used += count.bstri * SIZ_BSTRI(0);
      printf(F_U_MEM(9) " bytes in %8lu bstrings of average    %4lu bytes\n",
          count.bstri_elems * sizeof(ucharType),
          count.bstri,
          (unsigned long) ((count.bstri_elems * sizeof(ucharType)) / count.bstri));
      bytes_used += count.bstri_elems * sizeof(ucharType);
    } /* if */
    if (count.array != 0) {
      printf(F_U_MEM(9) " bytes in %8lu arrays of              %4u bytes\n",
          count.array * SIZ_ARR(0),
          count.array,
          (unsigned int) SIZ_ARR(0));
      bytes_used += count.array * SIZ_ARR(0);
    } /* if */
    if (count.arr_elems != 0) {
      printf(F_U_MEM(9) " bytes in " F_U_MEM(8) " array elements of      %4u bytes\n",
          count.arr_elems * SIZ_REC(objectRecord),
          count.arr_elems,
          (unsigned int) SIZ_REC(objectRecord));
      bytes_used += count.arr_elems * SIZ_REC(objectRecord);
    } /* if */
    if (count.rtl_arr_elems != 0) {
      printf(F_U_MEM(9) " bytes in " F_U_MEM(8) " rtl array elems of     %4u bytes\n",
          count.rtl_arr_elems * SIZ_REC(rtlObjectType),
          count.rtl_arr_elems,
          (unsigned int) SIZ_REC(rtlObjectType));
      bytes_used += count.rtl_arr_elems * SIZ_REC(rtlObjectType);
    } /* if */
    if (count.hash != 0) {
      printf(F_U_MEM(9) " bytes in %8lu hashtables of          %4u bytes\n",
          count.hash * SIZ_HSH(0),
          count.hash,
          (unsigned int) SIZ_HSH(0));
      bytes_used += count.hash * SIZ_HSH(0);
    } /* if */
    if (count.hsh_elems != 0) {
      printf(F_U_MEM(9) " bytes in " F_U_MEM(8) " hashtable elems of     %4u bytes\n",
          count.hsh_elems * SIZ_REC(hashElemType),
          count.hsh_elems,
          (unsigned int) SIZ_REC(hashElemType));
      bytes_used += count.hsh_elems * SIZ_REC(hashElemType);
    } /* if */
    if (count.helem != 0) {
      printf(F_U_MEM(9) " bytes in %8lu helems of              %4u bytes\n",
          count.helem * SIZ_REC(hashElemRecord),
          count.helem,
          (unsigned int) SIZ_REC(hashElemRecord));
      bytes_used += count.helem * SIZ_REC(hashElemRecord);
    } /* if */
    if (count.rtl_helem != 0) {
      printf(F_U_MEM(9) " bytes in %8lu rtl helems of          %4u bytes\n",
          count.rtl_helem * SIZ_REC(rtlHashElemRecord),
          count.rtl_helem,
          (unsigned int) SIZ_REC(rtlHashElemRecord));
      bytes_used += count.rtl_helem * SIZ_REC(rtlHashElemRecord);
    } /* if */
    if (count.set != 0) {
      printf(F_U_MEM(9) " bytes in %8lu sets of                %4u bytes\n",
          count.set * SIZ_SET(0),
          count.set,
          (unsigned int) SIZ_SET(0));
      bytes_used += count.set * SIZ_SET(0);
    } /* if */
    if (count.set_elems != 0) {
      printf(F_U_MEM(9) " bytes in " F_U_MEM(8) " set elements of        %4u bytes\n",
          count.set_elems * SIZ_REC(bitSetType),
          count.set_elems,
          (unsigned int) SIZ_REC(bitSetType));
      bytes_used += count.set_elems * SIZ_REC(bitSetType);
    } /* if */
    if (count.stru != 0) {
      printf(F_U_MEM(9) " bytes in %8lu structs of             %4u bytes\n",
          count.stru * SIZ_SCT(0),
          count.stru,
          (unsigned int) SIZ_SCT(0));
      bytes_used += count.stru * SIZ_SCT(0);
    } /* if */
    if (count.sct_elems != 0) {
      printf(F_U_MEM(9) " bytes in " F_U_MEM(8) " struct elements of     %4u bytes\n",
          count.sct_elems * SIZ_REC(objectRecord),
          count.sct_elems,
          (unsigned int) SIZ_REC(objectRecord));
      bytes_used += count.sct_elems * SIZ_REC(objectRecord);
    } /* if */
#if BIGINT_LIB == BIG_RTL_LIBRARY
    if (count.big != 0) {
      printf(F_U_MEM(9) " bytes in %8lu bigIntegers of         %4u bytes\n",
          count.big * SIZ_BIG(0),
          count.big,
          (unsigned int) SIZ_BIG(0));
      bytes_used += count.big * SIZ_BIG(0);
    } /* if */
    if (count.big_elems != 0) {
      printf(F_U_MEM(9) " bytes in " F_U_MEM(8) " bigdigits of           %4u bytes\n",
          count.big_elems * sizeof_bigDigitType,
          count.big_elems,
          (unsigned int) sizeof_bigDigitType);
      bytes_used += count.big_elems * sizeof_bigDigitType;
    } /* if */
#endif
    if (count.ident != 0) {
      printf(F_U_MEM(9) " bytes in %8lu ident records of       %4u bytes\n",
          count.ident * SIZ_REC(identRecord),
          count.ident,
          (unsigned int) SIZ_REC(identRecord));
      bytes_used += count.ident * SIZ_REC(identRecord);
    } /* if */
    if (count.idt != 0) {
      printf(F_U_MEM(9) " bytes in %8lu idents of average      %4lu bytes\n",
          count.idt_bytes + count.idt,
          count.idt,
          (unsigned long) ((count.idt_bytes + count.idt) / count.idt));
      bytes_used += count.idt_bytes + count.idt;
    } /* if */
    if (count.entity != 0) {
      printf(F_U_MEM(9) " bytes in %8lu entities of            %4u bytes\n",
          count.entity * SIZ_REC(entityRecord),
          count.entity,
          (unsigned int) SIZ_REC(entityRecord));
      bytes_used += count.entity * SIZ_REC(entityRecord);
    } /* if */
    if (count.property != 0) {
      printf(F_U_MEM(9) " bytes in %8lu properties of          %4u bytes\n",
          count.property * SIZ_REC(propertyRecord),
          count.property,
          (unsigned int) SIZ_REC(propertyRecord));
      bytes_used += count.property * SIZ_REC(propertyRecord);
    } /* if */
    if (count.object > num_flist_objects) {
      printf(F_U_MEM(9) " bytes in %8lu objects of             %4u bytes\n",
          (count.object - num_flist_objects) * SIZ_REC(objectRecord),
          count.object - num_flist_objects,
          (unsigned int) SIZ_REC(objectRecord));
      bytes_used += (count.object - num_flist_objects) * SIZ_REC(objectRecord);
    } /* if */
    if (count.node > num_flist_nodes) {
      printf(F_U_MEM(9) " bytes in %8lu nodes of               %4u bytes\n",
          (count.node - num_flist_nodes) * SIZ_REC(nodeRecord),
          count.node - num_flist_nodes,
          (unsigned int) SIZ_REC(nodeRecord));
      bytes_used += (count.node - num_flist_nodes) * SIZ_REC(nodeRecord);
    } /* if */
    if (count.token != 0) {
      printf(F_U_MEM(9) " bytes in %8lu tokens of              %4u bytes\n",
          count.token * SIZ_REC(tokenRecord),
          count.token,
          (unsigned int) SIZ_REC(tokenRecord));
      bytes_used += count.token * SIZ_REC(tokenRecord);
    } /* if */
    if (count.owner != 0) {
      printf(F_U_MEM(9) " bytes in %8lu owners of              %4u bytes\n",
          count.owner * SIZ_REC(ownerRecord),
          count.owner,
          (unsigned int) SIZ_REC(ownerRecord));
      bytes_used += count.owner * SIZ_REC(ownerRecord);
    } /* if */
    if (count.stack != 0) {
      printf(F_U_MEM(9) " bytes in %8lu stacks of              %4u bytes\n",
          count.stack * SIZ_REC(stackRecord),
          count.stack,
          (unsigned int) SIZ_REC(stackRecord));
      bytes_used += count.stack * SIZ_REC(stackRecord);
    } /* if */
    if (count.typelist_elems != 0) {
      printf(F_U_MEM(9) " bytes in %8lu typelist elems of      %4u bytes\n",
          count.typelist_elems * SIZ_REC(typeListRecord),
          count.typelist_elems,
          (unsigned int) SIZ_REC(typeListRecord));
      bytes_used += count.typelist_elems * SIZ_REC(typeListRecord);
    } /* if */
    if (count.type != 0) {
      printf(F_U_MEM(9) " bytes in %8lu types of               %4u bytes\n",
          count.type * SIZ_REC(typeRecord),
          count.type,
          (unsigned int) SIZ_REC(typeRecord));
      bytes_used += count.type * SIZ_REC(typeRecord);
    } /* if */
    if (count.list_elem > num_flist_list_elems) {
      printf(F_U_MEM(9) " bytes in %8lu list_elems of          %4u bytes\n",
          (count.list_elem - num_flist_list_elems) * SIZ_REC(listRecord),
          count.list_elem - num_flist_list_elems,
          (unsigned int) SIZ_REC(listRecord));
      bytes_used += (count.list_elem - num_flist_list_elems) * SIZ_REC(listRecord);
    } /* if */
    if (count.block != 0) {
      printf(F_U_MEM(9) " bytes in %8lu blocks of              %4u bytes\n",
          count.block * SIZ_REC(blockRecord),
          count.block,
          (unsigned int) SIZ_REC(blockRecord));
      bytes_used += count.block * SIZ_REC(blockRecord);
    } /* if */
    if (count.loclist != 0) {
      printf(F_U_MEM(9) " bytes in %8lu loclists of            %4u bytes\n",
          count.loclist * SIZ_REC(locListRecord),
          count.loclist,
          (unsigned int) SIZ_REC(locListRecord));
      bytes_used += count.loclist * SIZ_REC(locListRecord);
    } /* if */
    if (count.infil > num_flist_infiles) {
      printf(F_U_MEM(9) " bytes in %8lu files of               %4u bytes\n",
          (count.infil - num_flist_infiles) * SIZ_REC(inFileRecord),
          count.infil - num_flist_infiles,
          (unsigned int) SIZ_REC(inFileRecord));
      bytes_used += (count.infil - num_flist_infiles) * SIZ_REC(inFileRecord);
    } /* if */
    if (count.polldata != 0) {
      printf(F_U_MEM(9) " bytes in %8lu pollData elements of   %4u bytes\n",
          count.polldata * sizeof_pollRecord,
          count.polldata,
          (unsigned int) sizeof_pollRecord);
      bytes_used += count.polldata * sizeof_pollRecord;
    } /* if */
    if (count.win != 0) {
      printf(F_U_MEM(9) " bytes in %8lu windows of             %4u bytes\n",
          count.win_bytes,
          count.win,
          (unsigned int) (count.win_bytes / count.win));
      bytes_used += count.win_bytes;
    } /* if */
    if (count.process != 0) {
      printf(F_U_MEM(9) " bytes in %8lu processes of           %4u bytes\n",
          count.process * sizeof_processRecord,
          count.process,
          (unsigned int) sizeof_processRecord);
      bytes_used += count.process * sizeof_processRecord;
    } /* if */
    if (count.sql_func != 0) {
      printf(F_U_MEM(9) " bytes in %8lu sqlFunc records of     %4u bytes\n",
          count.sql_func * SIZ_REC(sqlFuncRecord),
          count.sql_func,
          (unsigned int) SIZ_REC(sqlFuncRecord));
      bytes_used += count.sql_func * SIZ_REC(sqlFuncRecord);
    } /* if */
    if (count.database != 0) {
      printf(F_U_MEM(9) " bytes in %8lu database records of    %4u bytes\n",
          count.database_bytes,
          count.database,
          (unsigned int) (count.database_bytes / count.database));
      bytes_used += count.database_bytes;
    } /* if */
    if (count.prepared_stmt != 0) {
      printf(F_U_MEM(9) " bytes in %8lu prepared statements of %4u bytes\n",
          count.prepared_stmt_bytes,
          count.prepared_stmt,
          (unsigned int) (count.prepared_stmt_bytes / count.prepared_stmt));
      bytes_used += count.prepared_stmt_bytes;
    } /* if */
    if (count.fetch_data != 0) {
      printf(F_U_MEM(9) " bytes in %8lu fetchData records of   %4u bytes\n",
          count.fetch_data_bytes,
          count.fetch_data,
          (unsigned int) (count.fetch_data_bytes / count.fetch_data));
      bytes_used += count.fetch_data_bytes;
    } /* if */
    if (count.prog != 0) {
      printf(F_U_MEM(9) " bytes in %8lu progs of               %4u bytes\n",
          count.prog * SIZ_REC(progRecord),
          count.prog,
          (unsigned int) SIZ_REC(progRecord));
      bytes_used += count.prog * SIZ_REC(progRecord);
    } /* if */
    bytes_in_buffers =
        count.fnam_bytes + count.fnam +
        count.symb_bytes + count.symb +
        count.byte;
    if (bytes_in_buffers != 0) {
      printf(F_U_MEM(9) " bytes in buffers\n", bytes_in_buffers);
      bytes_used += bytes_in_buffers;
    } /* if */
    if (bytes_used != 0) {
      printf(F_U_MEM(9) " bytes in use\n", bytes_used);
    } /* if */
    if (bytes_used != heapsize()) {
      printf("*** \nbytes_used=" FMT_U_MEM " heapsize=" FMT_U_MEM " diff=" FMT_U_MEM "\n",
          bytes_used, heapsize(), bytes_used - heapsize());
    } /* if */
    bytes_free = 0;
    if (num_flist_objects != 0) {
      printf(F_U_MEM(9) " bytes in %8lu free objects of        %4u bytes\n",
          num_flist_objects * SIZ_REC(objectRecord),
          num_flist_objects,
          (unsigned int) SIZ_REC(objectRecord));
      bytes_free += num_flist_objects * SIZ_REC(objectRecord);
    } /* if */
    if (num_flist_list_elems != 0) {
      printf(F_U_MEM(9) " bytes in %8lu free list_elems of     %4u bytes\n",
          num_flist_list_elems * SIZ_REC(listRecord),
          num_flist_list_elems,
          (unsigned int) SIZ_REC(listRecord));
      bytes_free += num_flist_list_elems * SIZ_REC(listRecord);
    } /* if */
    if (num_flist_nodes != 0) {
      printf(F_U_MEM(9) " bytes in %8lu free nodes of          %4u bytes\n",
          num_flist_nodes * SIZ_REC(nodeRecord),
          num_flist_nodes,
          (unsigned int) SIZ_REC(nodeRecord));
      bytes_free += num_flist_nodes * SIZ_REC(nodeRecord);
    } /* if */
    if (num_flist_infiles != 0) {
      printf(F_U_MEM(9) " bytes in %8lu free infiles of        %4u bytes\n",
          num_flist_infiles * SIZ_REC(inFileRecord),
          num_flist_infiles,
          (unsigned int) SIZ_REC(inFileRecord));
      bytes_free += num_flist_infiles * SIZ_REC(inFileRecord);
    } /* if */
    if (num_flist_stris != 0) {
      printf(F_U_MEM(9) " bytes in %8lu free string records of %4u bytes\n",
          num_flist_stris * SIZ_STRI(0),
          num_flist_stris,
          (unsigned int) SIZ_STRI(0));
      bytes_free += num_flist_stris * SIZ_STRI(0);
      printf(F_U_MEM(9) " bytes in %8lu free string chars of   %4u bytes\n",
          num_flist_stri_elems * sizeof(strElemType),
          num_flist_stri_elems,
          (unsigned int) sizeof(strElemType));
      bytes_free += num_flist_stri_elems * sizeof(strElemType);
    } /* if */
    bytes_total = bytes_used + bytes_free;
    printf(F_U_MEM(9) " bytes total (with " FMT_U_MEM " bytes in free lists)\n",
        bytes_total,
        bytes_free);
#if DO_HEAPSIZE_COMPUTATION
    if (bytes_total != hs) {
      printf("*** \nbytes_total=" FMT_U_MEM " hs=" FMT_U_MEM " diff=" FMT_U_MEM "\n",
          bytes_total, hs, bytes_total - hs);
    } /* if */
#endif
#if USE_CHUNK_ALLOCS
    if (chunk.number_of_chunks != 0) {
      printf(F_U_MEM(9) " bytes in %8u chunks\n",
          chunk.total_size, chunk.number_of_chunks);
      printf(F_U_MEM(9) " unused bytes in last chunk\n",
          (memSizeType) (chunk.beyond - chunk.freemem));
      printf(F_U_MEM(9) " lost bytes in chunks\n", chunk.lost_bytes);
      printf(F_U_MEM(9) " bytes total requested\n", bytes_total +
          (memSizeType) (chunk.beyond - chunk.freemem) + chunk.lost_bytes);
    } /* if */
#endif
#if DO_HEAP_CHECK
    /* check_heap(0, __FILE__, __LINE__); */
#endif
    logFunction(printf("heapStatistic -->\n"););
  } /* heapStatistic */
#endif



#ifdef OUT_OF_ORDER
static memSizeType compute_hs (void)

  {
    memSizeType bytes_total;

  /* compute_hs */
    logFunction(printf("compute_hs\n"););
    bytes_total = 0;
    bytes_total += count.stri * SIZ_STRI(0);
    bytes_total += count.stri_elems * sizeof(strElemType);
    bytes_total += count.bstri * SIZ_BSTRI(0);
    bytes_total += count.bstri_elems * sizeof(ucharType);
    bytes_total += count.array * SIZ_ARR(0);
    bytes_total += count.arr_elems * SIZ_REC(objectRecord);
    bytes_total += count.hash * SIZ_HSH(0);
    bytes_total += count.hsh_elems * SIZ_REC(hashElemType);
    bytes_total += count.hsh_elems * SIZ_REC(hashElemRecord);
    bytes_total += count.set * SIZ_SET(0);
    bytes_total += count.set_elems * SIZ_REC(bitSetType);
    bytes_total += count.stru * SIZ_SCT(0);
    bytes_total += count.sct_elems * SIZ_REC(objectRecord);
    bytes_total += count.big * SIZ_BIG(0);
    bytes_total += count.big_elems * SIZ_REC(bigDigitType);
    bytes_total += count.ident * SIZ_REC(identRecord);
    bytes_total += count.idt_bytes + count.idt;
    bytes_total += count.entity * SIZ_REC(entityRecord);
    bytes_total += count.property * SIZ_REC(propertyRecord);
    bytes_total += count.object * SIZ_REC(objectRecord);
    bytes_total += count.node * SIZ_REC(nodeRecord);
    bytes_total += count.token * SIZ_REC(tokenRecord);
    bytes_total += count.owner * SIZ_REC(ownerRecord);
    bytes_total += count.stack * SIZ_REC(stackRecord);
    bytes_total += count.typelist_elems * SIZ_REC(typeListRecord);
    bytes_total += count.type * SIZ_REC(typeRecord);
    bytes_total += count.list_elem * SIZ_REC(listRecord);
    bytes_total += count.block * SIZ_REC(blockRecord);
    bytes_total += count.loclist * SIZ_REC(locListRecord);
    bytes_total += count.infil * SIZ_REC(inFileRecord);
    bytes_total += count.polldata * sizeof_pollRecord;
    bytes_total += count.win_bytes;
    bytes_total += count.process * sizeof_processRecord;
    bytes_total += count.sql_func * SIZ_REC(sqlFuncRecord);
    bytes_total += count.database_bytes;
    bytes_total += count.prepared_stmt_bytes;
    bytes_total += count.fetch_data_bytes;
    bytes_total += count.prog * SIZ_REC(progRecord);
    bytes_total += count.fnam_bytes + count.fnam +
        count.symb_bytes + count.symb +
        count.byte;
    logFunction(printf("compute_hs -->\n"););
    return bytes_total;
  } /* compute_hs */
#endif



memSizeType heapsize (void)

  {
#if DO_HEAPSIZE_COMPUTATION
    memSizeType flist_bytes;
    unsigned long num_flist_stri_elems;
#endif
    memSizeType result;

  /* heapsize */
#if DO_HEAPSIZE_COMPUTATION
    flist_bytes = object_flist_count() * sizeof(objectRecord);
    flist_bytes += list_elem_flist_count() * sizeof(listRecord);
    flist_bytes += node_flist_count() * sizeof(nodeRecord);
    flist_bytes += infile_flist_count() * sizeof(inFileRecord);
    flist_bytes += stri_flist_count(&num_flist_stri_elems) * SIZ_STRI(0);
    flist_bytes += num_flist_stri_elems * sizeof(strElemType);
/*  printf(" %ld ", hs); */
    result = hs - flist_bytes;
#else
    result = 0;
#endif
    return result;
  } /* heapsize */



#if USE_CHUNK_ALLOCS
#ifdef USE_FLIST_ALLOC
void *flist_alloc (size_t size)

  {
    size_t size_of_rec;
    void *result;

  /* flist_alloc */
    if (size <= sizeof(listRecord) && flist.list_elems != NULL) {
      result = (void *) flist.list_elems;
      flist.list_elems = flist.list_elems->next;
      size_of_rec = sizeof(listRecord);
    } else if (size <= sizeof(objectRecord) && flist.objects != NULL) {
      result = (void *) flist.objects;
      flist.objects = flist.objects->value.objValue;
      size_of_rec = sizeof(objectRecord);
    } else if (size <= sizeof(inFileRecord) && flist.infiles != NULL) {
      result = (void *) flist.infiles;
      flist.infiles = flist.infiles->next;
      size_of_rec = sizeof(inFileRecord);
    } else {
      result = NULL;
      size_of_rec = size;
    } /* if */
    LOST_B += size_of_rec - size;
/*  printf("flist_alloc(%d) --> " FMT_X_MEM "\n", size, (memSizeType) result); */
    return result;
  } /* flist_alloc */
#endif
#endif



#if !USE_CHUNK_ALLOCS
void reuse_free_lists (void)

  {
    objectType help_obj;
    listType help_list;
    inFileType help_infile;

  /* reuse_free_lists */
    /* Free the flist.objects and the flist.list_elems lists */
    while (flist.objects != NULL) {
      help_obj = flist.objects;
      flist.objects = flist.objects->value.objValue;
      FREE_RECORD(help_obj, objectRecord, count.object);
    } /* while */
    while (flist.list_elems != NULL) {
      help_list = flist.list_elems;
      flist.list_elems = flist.list_elems->next;
      FREE_RECORD(help_list, listRecord, count.list_elem);
    } /* while */
    while (flist.infiles != NULL) {
      help_infile = flist.infiles;
      flist.infiles = flist.infiles->next;
      FREE_RECORD(help_infile, inFileRecord, count.infil);
    } /* while */
  } /* reuse_free_lists */
#endif



#if USE_CHUNK_ALLOCS
void *heap_chunk (size_t size)

  {
    int index;
    listType list_elem;
    void *result;

  /* heap_chunk */
    logFunction(printf("heap_chunk(" FMT_U_MEM ")\n", size););
/*  printf("%lu ", heapsize()); */
    index = 0;
    result = NULL;
    while (chunk_size[index] >= size && chunk_size[index] != 0 &&
        (result = (void *) MALLOC(chunk_size[index])) == NULL) {
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
        /* There might be unused memory in the old chunk. A realloc() */
        /* of the old chunk could be dangerous. Realloc() could move  */
        /* the memory area of the chunk. Such a move would invalidate */
        /* all pointers into the old chunk. Instead remaining unused  */
        /* memory from the old chunk is used as free list elements.   */
        /* Elements of listRecord have been chosen, because they are  */
        /* common and the free list elements with the smallest size.  */
        while (sizeof(listRecord) <= (memSizeType) (chunk.beyond - chunk.freemem)) {
          if (OLD_CHUNK(list_elem, listType, sizeof(listRecord))) {
            COUNT_FLISTELEM(listRecord, count.list_elem);
            FREE_L_ELEM(list_elem);
          } /* if */
        } /* while */
        chunk.lost_bytes += (memSizeType) (chunk.beyond - chunk.freemem);
      } /* if */
      chunk.size = chunk_size[index];
      chunk.total_size += chunk.size;
      chunk.start = (char *) result;
      chunk.beyond = chunk.start + chunk.size;
      chunk.freemem = chunk.start + size;
      chunk.size -= size;
      chunk.number_of_chunks++;
    } /* if */
    logFunction(printf("heap_chunk --> " FMT_U_MEM "\n", (memSizeType) result););
    return result;
  } /* heap_chunk */
#endif



#if DO_HEAP_CHECK
void check_heap (long sizediff, const char *file_name, unsigned int line_num)

  {
    memSizeType bytes_used;
    /* static long last_sizediff = 0;
       static unsigned int last_line_num = 0; */

  /* check_heap */
    logFunction(printf("check_heap\n"););
    bytes_used =
        ((memSizeType) count.stri) * SIZ_STRI(0) +
        count.stri_elems * sizeof(strElemType) +
        ((memSizeType) count.bstri) * SIZ_BSTRI(0) +
        count.bstri_elems * sizeof(ucharType) +
        ((memSizeType) count.array) * SIZ_ARR(0) +
        count.arr_elems * SIZ_REC(objectRecord) +
        count.rtl_arr_elems * SIZ_REC(rtlObjectType) +
        ((memSizeType) count.hash) * SIZ_HSH(0) +
        count.hsh_elems * SIZ_REC(hashElemType) +
        ((memSizeType) count.helem) * SIZ_REC(hashElemRecord) +
        ((memSizeType) count.rtl_helem) * SIZ_REC(rtlHashElemRecord) +
        ((memSizeType) count.set) * SIZ_SET(0) +
        count.set_elems * SIZ_REC(bitSetType) +
        ((memSizeType) count.stru) * SIZ_SCT(0) +
        count.sct_elems * SIZ_REC(objectRecord) +
        ((memSizeType) count.big) * SIZ_BIG(0) +
        count.big_elems * SIZ_REC(bigDigitType) +
        ((memSizeType) count.ident) * SIZ_REC(identRecord) +
        count.idt_bytes + ((memSizeType) count.idt) +
        ((memSizeType) count.entity)         * SIZ_REC(entityRecord) +
        ((memSizeType) count.property)       * SIZ_REC(propertyRecord) +
        ((memSizeType) count.object)         * SIZ_REC(objectRecord) +
        ((memSizeType) count.node)           * SIZ_REC(nodeRecord) +
        ((memSizeType) count.token)          * SIZ_REC(tokenRecord) +
        ((memSizeType) count.owner)          * SIZ_REC(ownerRecord) +
        ((memSizeType) count.stack)          * SIZ_REC(stackRecord) +
        ((memSizeType) count.typelist_elems) * SIZ_REC(typeListRecord) +
        ((memSizeType) count.type)           * SIZ_REC(typeRecord) +
        ((memSizeType) count.list_elem)      * SIZ_REC(listRecord) +
        ((memSizeType) count.block)          * SIZ_REC(blockRecord) +
        ((memSizeType) count.loclist)        * SIZ_REC(locListRecord) +
        ((memSizeType) count.infil)          * SIZ_REC(inFileRecord) +
        ((memSizeType) count.polldata)       * sizeof_pollRecord +
        ((memSizeType) count.win_bytes) +
        ((memSizeType) count.process)        * sizeof_processRecord +
        ((memSizeType) count.sql_func)       * SIZ_REC(sqlFuncRecord) +
        ((memSizeType) count.database_bytes) +
        ((memSizeType) count.prepared_stmt_bytes) +
        ((memSizeType) count.fetch_data_bytes) +
        ((memSizeType) count.prog)           * SIZ_REC(progRecord) +
        count.fnam_bytes + ((memSizeType) count.fnam) +
        count.symb_bytes + ((memSizeType) count.symb) +
        count.byte;
    if (bytes_used != hs) {
      printf("*** %s(%u)\n" FMT_U_MEM " " FMT_U_MEM " " FMT_U_MEM " %ld \n",
          file_name, line_num, bytes_used, hs, bytes_used - hs, sizediff);
/*    heapStatistic();
      fflush(stdout);
      printf("should not happen\n");
      exit(1); */
/*  } else {
      printf("\n%lu %ld %d \n", hs, sizediff, in_file.line); */
    } /* if */
    /* last_sizediff = sizediff;
       last_line_num = line_num; */
    /* if (sizediff > 0) {
      printf("\nalloc(%ld)\n", sizediff);
    } else {
      printf("\nfree(%ld)\n", -sizediff);
    } if */
/*  show_statistic(); */
    logFunction(printf("check_heap -->\n"););
  } /* check_heap */
#endif
