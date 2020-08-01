/********************************************************************/
/*                                                                  */
/*  set_rtl.c     Primitive actions for the set type.               */
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
/*  File: seed7/src/set_rtl.c                                       */
/*  Changes: 2004, 2005  Thomas Mertes                              */
/*  Content: Primitive actions for the set type.                    */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"


#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "rtl_err.h"
#include "int_rtl.h"

#undef EXTERN
#define EXTERN
#include "set_rtl.h"


static int card_byte[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
  };



#ifdef ANSI_C

settype setArrlit (const_rtlArraytype arr1)
#else

settype setArrlit (arr1)
rtlArraytype arr1;
#endif

  {
    memsizetype length;
    inttype number;
    inttype position;
    unsigned int bit_index;
    memsizetype array_index;
    settype result;

  /* setArrlit */
    length = (uinttype) (arr1->max_position - arr1->min_position + 1);
    if (!ALLOC_SET(result, 1)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      if (length == 0) {
        result->min_position = 0;
        result->max_position = 0;
        memset(result->bitset, 0, sizeof(bitsettype));
      } else {
        number = arr1->arr[0].value.intvalue;
        position = bitset_pos(number);
        result->min_position = position;
        result->max_position = position;
        bit_index = ((unsigned int) number) & bitset_mask;
        result->bitset[0] = (bitsettype) 1 << bit_index;
        for (array_index = 1; array_index < length; array_index++) {
          setIncl(&result, arr1->arr[array_index].value.intvalue);
#ifdef OUT_OF_ORDER
          if (fail_flag) {
            FREE_SET(result, (uinttype) (result->max_position - result->min_position + 1));
            return(fail_value);
          } /* if */
#endif
        } /* for */
      } /* if */
      return(result);
    } /* if */
  } /* setArrlit */



#ifdef ANSI_C

settype setBaselit (const inttype number)
#else

settype setBaselit (number)
inttype number;
#endif

  {
    inttype position;
    unsigned int bit_index;
    settype result;

  /* setBaselit */
    if (!ALLOC_SET(result, 1)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      position = bitset_pos(number);
      result->min_position = position;
      result->max_position = position;
      bit_index = ((unsigned int) number) & bitset_mask;
      result->bitset[0] = (bitsettype) 1 << bit_index;
      return(result);
    } /* if */
  } /* setBaselit */



#ifdef ANSI_C

inttype setCard (const const_settype set1)
#else

inttype setCard (set1)
settype set1;
#endif

  {
    memsizetype bitset_index;
    unsigned char *byte;
    unsigned int idx;
    inttype result;

  /* setCard */
    result = 0;
    for (bitset_index = 0;
        bitset_index <= (uinttype) (set1->max_position - set1->min_position);
        bitset_index++) {
      byte = (unsigned char *) &set1->bitset[bitset_index];
      for (idx = 0; idx < sizeof(bitsettype); idx++) {
        /* printf("%c%c%c%c%c%c%c%c [%d] ",
               *byte & 0x80 ? '1' : '0',
               *byte & 0x40 ? '1' : '0',
               *byte & 0x20 ? '1' : '0',
               *byte & 0x10 ? '1' : '0',
               *byte & 0x08 ? '1' : '0',
               *byte & 0x04 ? '1' : '0',
               *byte & 0x02 ? '1' : '0',
               *byte & 0x01 ? '1' : '0',
               card_byte[(int) *byte]); */
        result += card_byte[(int) *byte];
        byte++;
      } /* for */
    } /* for */
    /* printf(" = %d\n", result); */
    return(result);
  } /* setCard */



/**
 *  Compares two sets to make them useable as key in a hash table.
 *  The sets are compared by determining the biggest element that is
 *  not present or absent in both sets. The set in which this element
 *  is not present is the smaller one. Note that the set comparison
 *  is not related to the concepts of subset or superset. With the
 *  comparison function setCmp it is posible to sort an array of
 *  sets or to use sets as key in a hash table. The functions
 *  setIsSubset and setIsProperSubset are used to check if a set is
 *  a (proper) subset or superset of another set.
 */
#ifdef ANSI_C

inttype setCmp (const const_settype set1, const const_settype set2)
#else

inttype setCmp (set1, set2)
settype set1;
settype set2;
#endif

  {
    memsizetype max_index;
    memsizetype bitset_index;
    memsizetype size;
    const bitsettype *bitset1;
    const bitsettype *bitset2;

  /* setCmp */
    if (set1->max_position >= set2->max_position) {
      if (set1->min_position > set2->max_position) {
        bitset_index = 0;
        size = 0;
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        bitset_index = (uinttype) (set2->max_position - set1->min_position + 1);
        if (set1->min_position >= set2->min_position) {
          size = bitset_index;
        } else {
          size = (uinttype) (set2->max_position - set2->min_position + 1);
        } /* if */
        bitset1 = &set1->bitset[bitset_index - 1];
        bitset2 = &set2->bitset[set2->max_position - set2->min_position];
      } /* if */
      max_index = (uinttype) (set1->max_position - set1->min_position);
      for (; bitset_index <= max_index; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return(1);
        } /* if */
      } /* for */
    } else {
      if (set1->max_position < set2->min_position) {
        bitset_index = 0;
        size = 0;
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        bitset_index = (uinttype) (set1->max_position - set2->min_position + 1);
        if (set1->min_position <= set2->min_position) {
          size = bitset_index;
        } else {
          size = (uinttype) (set1->max_position - set1->min_position + 1);
        } /* if */
        bitset1 = &set1->bitset[set1->max_position - set1->min_position];
        bitset2 = &set2->bitset[bitset_index - 1];
      } /* if */
      max_index = (uinttype) (set2->max_position - set2->min_position);
      for (; bitset_index <= max_index; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          return(-1);
        } /* if */
      } /* for */
    } /* if */
    for (bitset_index = 0; bitset_index < size; bitset_index++, bitset1--, bitset2--) {
      if (*bitset1 != *bitset2) {
        if (*bitset1 > *bitset2) {
          return(1);
        } else {
          return(-1);
        } /* if */
      } /* if */
    } /* for */
    if (set1->min_position < set2->min_position) {
      if (set1->max_position < set2->min_position) {
        max_index = (uinttype) (set1->max_position - set1->min_position);
      } else {
        max_index = (uinttype) (set2->min_position - set1->min_position - 1);
      } /* if */
      for (bitset_index = 0; bitset_index <= max_index; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return(1);
        } /* if */
      } /* for */
    } else if (set1->min_position > set2->min_position) {
      if (set1->min_position > set2->max_position) {
        max_index = (uinttype) (set2->max_position - set2->min_position);
      } else {
        max_index = (uinttype) (set1->min_position - set2->min_position - 1);
      } /* if */
      for (bitset_index = 0; bitset_index <= max_index; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          return(-1);
        } /* if */
      } /* for */
    } /* if */
    return(0);
  } /* setCmp */



#ifdef ANSI_C

void setCpy (settype *const set_to, const const_settype set_from)
#else

void setCpy (set_to, set_from)
settype *set_to;
settype set_from;
#endif

  {
    settype set_dest;
    memsizetype set_dest_size;
    memsizetype set_source_size;

  /* setCpy */
    set_dest = *set_to;
    set_source_size = (uinttype) (set_from->max_position - set_from->min_position + 1);
    if (set_dest->min_position != set_from->min_position ||
        set_dest->max_position != set_from->max_position) {
      set_dest_size = (uinttype) (set_dest->max_position - set_dest->min_position + 1);
      if (set_dest_size != set_source_size) {
        if (!ALLOC_SET(set_dest, set_source_size)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          FREE_SET(*set_to, set_dest_size);
          *set_to = set_dest;
        } /* if */
      } /* if */
      set_dest->min_position = set_from->min_position;
      set_dest->max_position = set_from->max_position;
    } /* if */
    memcpy(set_dest->bitset, set_from->bitset,
        (size_t) set_source_size * sizeof(bitsettype));
  } /* setCpy */



#ifdef ANSI_C

settype setCreate (const const_settype set_from)
#else

settype setCreate (set_from)
settype set_from;
#endif

  {
    memsizetype new_size;
    settype result;

  /* setCreate */
    new_size = (uinttype) (set_from->max_position - set_from->min_position + 1);
    if (!ALLOC_SET(result, new_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = set_from->min_position;
      result->max_position = set_from->max_position;
      memcpy(result->bitset, set_from->bitset,
          (size_t) new_size * sizeof(bitsettype));
    } /* if */
    return(result);
  } /* setCreate */



#ifdef ANSI_C

void setDestr (const const_settype old_set)
#else

void setDestr (old_set)
settype old_set;
#endif

  { /* setDestr */
    if (old_set != NULL) {
      FREE_SET(old_set, (uinttype) (old_set->max_position - old_set->min_position + 1));
    } /* if */
  } /* setDestr */



#ifdef ANSI_C

settype setDiff (const const_settype set1, const const_settype set2)
#else

settype setDiff (set1, set2)
settype set1;
settype set2;
#endif

  {
    memsizetype bitset_index;
    memsizetype bitset_index2;
    memsizetype stop_index;
    settype result;

  /* setDiff */
    if (!ALLOC_SET(result, (uinttype) (set1->max_position - set1->min_position + 1))) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->min_position = set1->min_position;
      result->max_position = set1->max_position;
      if (set2->max_position >= set1->min_position ||
          set2->min_position <= set1->max_position) {
        if (set2->min_position > set1->min_position) {
          bitset_index = (uinttype) (set2->min_position - set1->min_position);
          bitset_index2 = 0;
          memcpy(result->bitset, set1->bitset, bitset_index * sizeof(bitsettype));
        } else {
          bitset_index = 0;
          bitset_index2 = (uinttype) (set1->min_position - set2->min_position);
        } /* if */
        if (set1->max_position > set2->max_position) {
          stop_index = (uinttype) (set2->max_position - set1->min_position);
          memcpy(&result->bitset[stop_index + 1], &set1->bitset[stop_index + 1],
              (uinttype) (set1->max_position - set2->max_position) * sizeof(bitsettype));
        } else {
          stop_index = (uinttype) (set1->max_position - set1->min_position);
        } /* if */
        for (; bitset_index <= stop_index; bitset_index++, bitset_index2++) {
          result->bitset[bitset_index] = set1->bitset[bitset_index] &
              ~ set2->bitset[bitset_index2];
        } /* for */
      } else {
        memcpy(result->bitset, set1->bitset,
            (uinttype) (set1->max_position - set1->min_position + 1) * sizeof(bitsettype));
      } /* if */
      return(result);
    } /* if */
  } /* setDiff */



#ifdef ANSI_C

booltype setElem (const inttype number, const const_settype set1)
#else

booltype setElem (number, set1)
inttype number;
settype set1;
#endif

  {
    inttype position;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* setElem */
    position = bitset_pos(number);
    if (position >= set1->min_position && position <= set1->max_position) {
      bitset_index = (uinttype) (position - set1->min_position);
      bit_index = ((unsigned int) number) & bitset_mask;
      if (set1->bitset[bitset_index] & (bitsettype) 1 << bit_index) {
        return(TRUE);
      } else {
        return(FALSE);
      } /* if */
    } else {
      return(FALSE);
    } /* if */
  } /* setElem */



#ifdef ANSI_C

booltype setEq (const const_settype set1, const const_settype set2)
#else

booltype setEq (set1, set2)
settype set1;
settype set2;
#endif

  {
    memsizetype max_index;
    memsizetype bitset_index;
    memsizetype size;
    const bitsettype *bitset1;
    const bitsettype *bitset2;

  /* setEq */
    if (set1->min_position == set2->min_position &&
        set1->max_position == set2->max_position) {
      return(memcmp(set1->bitset, set2->bitset,
          (uinttype) (set1->max_position - set1->min_position + 1) *
          sizeof(bitsettype)) == 0);
    } else {
      if (set1->min_position < set2->min_position) {
        if (set1->max_position < set2->min_position) {
          size = 0;
          max_index = (uinttype) (set1->max_position - set1->min_position + 1);
          bitset1 = NULL;
          bitset2 = NULL;
        } else {
          if (set1->max_position <= set2->max_position) {
            size = (uinttype) (set1->max_position - set2->min_position + 1);
          } else {
            size = (uinttype) (set2->max_position - set2->min_position + 1);
          } /* if */
          max_index = (uinttype) (set2->min_position - set1->min_position);
          bitset1 = &set1->bitset[max_index];
          bitset2 = set2->bitset;
        } /* if */
        for (bitset_index = 0; bitset_index < max_index; bitset_index++) {
          if (set1->bitset[bitset_index] != 0) {
            return(FALSE);
          } /* if */
        } /* for */
      } else {
        if (set1->min_position > set2->max_position) {
          size = 0;
          max_index = (uinttype) (set2->max_position - set2->min_position + 1);
          bitset1 = NULL;
          bitset2 = NULL;
        } else {
          if (set1->max_position >= set2->max_position) {
            size = (uinttype) (set2->max_position - set1->min_position + 1);
          } else {
            size = (uinttype) (set1->max_position - set1->min_position + 1);
          } /* if */
          max_index = (uinttype) (set1->min_position - set2->min_position);
          bitset1 = set1->bitset;
          bitset2 = &set2->bitset[max_index];
        } /* if */
        for (bitset_index = 0; bitset_index < max_index; bitset_index++) {
          if (set2->bitset[bitset_index] != 0) {
            return(FALSE);
          } /* if */
        } /* for */
      } /* if */
      if (set1->max_position > set2->max_position) {
        if (set1->min_position > set2->max_position) {
          bitset_index = 0;
        } else {
          bitset_index = (uinttype) (set2->max_position - set1->min_position + 1);
        } /* if */
        max_index = (uinttype) (set1->max_position - set1->min_position);
        for (; bitset_index <= max_index; bitset_index++) {
          if (set1->bitset[bitset_index] != 0) {
            return(FALSE);
          } /* if */
        } /* for */
      } else {
        if (set1->max_position < set2->min_position) {
          bitset_index = 0;
        } else {
          bitset_index = (uinttype) (set1->max_position - set2->min_position + 1);
        } /* if */
        max_index = (uinttype) (set2->max_position - set2->min_position);
        for (; bitset_index <= max_index; bitset_index++) {
          if (set2->bitset[bitset_index] != 0) {
            return(FALSE);
          } /* if */
        } /* for */
      } /* if */
      return(memcmp(bitset1, bitset2, size * sizeof(bitsettype)) == 0);
    } /* if */
  } /* setEq */



#ifdef ANSI_C

void setExcl (settype *const set_to, const inttype number)
#else

void setExcl (set_to, number)
settype *set_to;
inttype number;
#endif

  {
    settype set_dest;
    inttype position;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* setExcl */
    set_dest = *set_to;
    position = bitset_pos(number);
    if (position >= set_dest->min_position && position <= set_dest->max_position) {
      bitset_index = (uinttype) (position - set_dest->min_position);
      bit_index = ((unsigned int) number) & bitset_mask;
      set_dest->bitset[bitset_index] &= ~((bitsettype) 1 << bit_index);
#ifdef OUT_OF_ORDER
      if (set_dest->bitset[bitset_index] == 0) {
        if
      } /* if */
#endif
    } /* if */
  } /* setExcl */



#ifdef ANSI_C

inttype setHashCode (const const_settype set1)
#else

inttype setHashCode (set1)
settype set1;
#endif

  {
    memsizetype bitset_index;
    inttype result;

  /* setHashCode */
    result = 0;
    for (bitset_index = 0;
        bitset_index < (uinttype) (set1->max_position - set1->min_position);
        bitset_index++) {
      result ^= (inttype) set1->bitset[bitset_index];
    } /* for */
    return(result);
  } /* setHashCode */



#ifdef ANSI_C

settype setIConv (inttype number)
#else

settype setIConv (number)
inttype number;
#endif

  {
    memsizetype pos;
    memsizetype result_size;
    settype result;

  /* setIConv */
    result_size = sizeof(inttype) / sizeof(bitsettype);
    if (!ALLOC_SET(result, result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = 0;
      result->max_position = (inttype) (result_size - 1);
      for (pos = 0; pos < result_size; pos++) {
        result->bitset[pos] = (bitsettype) number;
        number >>= 8 * sizeof(bitsettype);
      } /* for */
    } /* if */
    return(result);
  } /* setIConv */



#ifdef ANSI_C

void setIncl (settype *const set_to, const inttype number)
#else

void setIncl (set_to, number)
settype *set_to;
inttype number;
#endif

  {
    settype set_dest;
    inttype position;
    memsizetype old_size;
    memsizetype new_size;
    settype old_set;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* setIncl */
    set_dest = *set_to;
    position = bitset_pos(number);
    if (position > set_dest->max_position) {
      old_size = (uinttype) (set_dest->max_position - set_dest->min_position + 1);
      new_size = (uinttype) (position - set_dest->min_position + 1);
      set_dest = REALLOC_SET(set_dest, old_size, new_size);
      if (set_dest == NULL) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        COUNT3_SET(old_size, new_size);
        *set_to = set_dest;
        set_dest->max_position = position;
        memset(&set_dest->bitset[old_size], 0, (new_size - old_size) * sizeof(bitsettype));
      } /* if */
    } else if (position < set_dest->min_position) {
      old_size = (uinttype) (set_dest->max_position - set_dest->min_position + 1);
      new_size = (uinttype) (set_dest->max_position - position + 1);
      old_set = set_dest;
      if (!ALLOC_SET(set_dest, new_size)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        *set_to = set_dest;
        set_dest->min_position = position;
        set_dest->max_position = old_set->max_position;
        memset(set_dest->bitset, 0, (new_size - old_size) * sizeof(bitsettype));
        memcpy(&set_dest->bitset[new_size - old_size], old_set->bitset, old_size * sizeof(bitsettype));
        FREE_SET(old_set, old_size);
      } /* if */
    } /* if */
    bitset_index = (uinttype) (position - set_dest->min_position);
    bit_index = ((unsigned int) number) & bitset_mask;
    set_dest->bitset[bitset_index] |= (bitsettype) 1 << bit_index;
  } /* setIncl */



#ifdef ANSI_C

settype setIntersect (const const_settype set1, const const_settype set2)
#else

settype setIntersect (set1, set2)
settype set1;
settype set2;
#endif

  {
    inttype position;
    inttype min_position;
    inttype max_position;
    settype result;

  /* setIntersect */
#ifdef OUT_OF_ORDER
    printf("setIntersect(\n");
    prot_set(set1);
    printf(",\n");
    prot_set(set2);
    printf(")\n");
#endif
    if (set1->min_position > set2->min_position) {
      min_position = set1->min_position;
    } else {
      min_position = set2->min_position;
    } /* if */
    if (set1->max_position < set2->max_position) {
      max_position = set1->max_position;
    } else {
      max_position = set2->max_position;
    } /* if */
    if (min_position > max_position) {
      if (!ALLOC_SET(result, 1)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        result->min_position = 0;
        result->max_position = 0;
        memset(result->bitset, 0, sizeof(bitsettype));
        return(result);
      } /* if */
    } else {
      if (!ALLOC_SET(result, (uinttype) (max_position - min_position + 1))) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        result->min_position = min_position;
        result->max_position = max_position;
        for (position = min_position; position <= max_position; position++) {
          result->bitset[position - min_position] =
              set1->bitset[position - set1->min_position] &
              set2->bitset[position - set2->min_position];
        } /* for */
        return(result);
      } /* if */
    } /* if */
  } /* setIntersect */



#ifdef ANSI_C

booltype setIsProperSubset (const const_settype set1, const const_settype set2)
#else

booltype setIsProperSubset (set1, set2)
settype set1;
settype set2;
#endif

  {
    memsizetype max_index;
    memsizetype bitset_index;
    memsizetype size;
    const bitsettype *bitset1;
    const bitsettype *bitset2;
    booltype equal;

  /* setIsProperSubset */
#ifdef OUT_OF_ORDER
    printf("setIsProperSubset(");
    prot_set(set1);
    printf(", ");
    prot_set(set2);
    printf(")\n");
#endif
    equal = TRUE;
    if (set1->min_position < set2->min_position) {
      if (set1->max_position < set2->min_position) {
        size = 0;
        max_index = (uinttype) (set1->max_position - set1->min_position + 1);
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        if (set1->max_position <= set2->max_position) {
          size = (uinttype) (set1->max_position - set2->min_position + 1);
        } else {
          size = (uinttype) (set2->max_position - set2->min_position + 1);
        } /* if */
        max_index = (uinttype) (set2->min_position - set1->min_position);
        bitset1 = &set1->bitset[max_index];
        bitset2 = set2->bitset;
      } /* if */
      for (bitset_index = 0; bitset_index < max_index; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return(FALSE);
        } /* if */
      } /* for */
    } else {
      if (set1->min_position > set2->max_position) {
        size = 0;
        max_index = (uinttype) (set2->max_position - set2->min_position + 1);
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        if (set1->max_position >= set2->max_position) {
          size = (uinttype) (set2->max_position - set1->min_position + 1);
        } else {
          size = (uinttype) (set1->max_position - set1->min_position + 1);
        } /* if */
        max_index = (uinttype) (set1->min_position - set2->min_position);
        bitset1 = set1->bitset;
        bitset2 = &set2->bitset[max_index];
      } /* if */
      for (bitset_index = 0; bitset_index < max_index && equal; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          equal = FALSE;
        } /* if */
      } /* for */
    } /* if */
    if (set1->max_position > set2->max_position) {
      if (set1->min_position > set2->max_position) {
        bitset_index = 0;
      } else {
        bitset_index = (uinttype) (set2->max_position - set1->min_position + 1);
      } /* if */
      max_index = (uinttype) (set1->max_position - set1->min_position);
      for (; bitset_index <= max_index; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return(FALSE);
        } /* if */
      } /* for */
    } else {
      if (set1->max_position < set2->min_position) {
        bitset_index = 0;
      } else {
        bitset_index = (uinttype) (set1->max_position - set2->min_position + 1);
      } /* if */
      max_index = (uinttype) (set2->max_position - set2->min_position);
      for (; bitset_index <= max_index && equal; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          equal = FALSE;
        } /* if */
      } /* for */
    } /* if */
    for (bitset_index = 0; bitset_index < size; bitset_index++, bitset1++, bitset2++) {
      if ((~ *bitset1 | *bitset2) != ~(bitsettype) 0) {
        return(FALSE);
      } /* if */
      if (*bitset1 != *bitset2) {
        equal = FALSE;
      } /* if */
    } /* for */
    return(!equal);
  } /* setIsProperSubset */



#ifdef ANSI_C

booltype setIsSubset (const const_settype set1, const const_settype set2)
#else

booltype setIsSubset (set1, set2)
settype set1;
settype set2;
#endif

  {
    memsizetype max_index;
    memsizetype bitset_index;
    memsizetype size;
    const bitsettype *bitset1;
    const bitsettype *bitset2;

  /* setIsSubset */
#ifdef OUT_OF_ORDER
    printf("setIsSubset(");
    prot_set(set1);
    printf(", ");
    prot_set(set2);
    printf(")\n");
#endif
    if (set1->min_position < set2->min_position) {
      if (set1->max_position < set2->min_position) {
        size = 0;
        max_index = (uinttype) (set1->max_position - set1->min_position + 1);
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        if (set1->max_position <= set2->max_position) {
          size = (uinttype) (set1->max_position - set2->min_position + 1);
        } else {
          size = (uinttype) (set2->max_position - set2->min_position + 1);
        } /* if */
        max_index = (uinttype) (set2->min_position - set1->min_position);
        bitset1 = &set1->bitset[max_index];
        bitset2 = set2->bitset;
      } /* if */
      for (bitset_index = 0; bitset_index < max_index; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return(FALSE);
        } /* if */
      } /* for */
    } else {
      if (set1->min_position > set2->max_position) {
        size = 0;
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        if (set1->max_position >= set2->max_position) {
          size = (uinttype) (set2->max_position - set1->min_position + 1);
        } else {
          size = (uinttype) (set1->max_position - set1->min_position + 1);
        } /* if */
        bitset1 = set1->bitset;
        bitset2 = &set2->bitset[set1->min_position - set2->min_position];
      } /* if */
    } /* if */
    if (set1->max_position > set2->max_position) {
      if (set1->min_position > set2->max_position) {
        bitset_index = 0;
      } else {
        bitset_index = (uinttype) (set2->max_position - set1->min_position + 1);
      } /* if */
      max_index = (uinttype) (set1->max_position - set1->min_position);
      for (; bitset_index <= max_index; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return(FALSE);
        } /* if */
      } /* for */
    } /* if */
    for (bitset_index = 0; bitset_index < size; bitset_index++, bitset1++, bitset2++) {
      if ((~ *bitset1 | *bitset2) != ~(bitsettype) 0) {
        return(FALSE);
      } /* if */
    } /* for */
    return(TRUE);
  } /* setIsSubset */



#ifdef ANSI_C

inttype setMax (const const_settype set1)
#else

inttype setMax (set1)
settype set1;
#endif

  {
    memsizetype bitset_index;
    bitsettype curr_bitset;
    inttype result;

  /* setMax */
    bitset_index = (uinttype) (set1->max_position - set1->min_position + 1);
    while (bitset_index > 0) {
      bitset_index--;
      curr_bitset = set1->bitset[bitset_index];
      if (curr_bitset) {
        result = uintMostSignificantBit(curr_bitset);
        result += (set1->min_position + (inttype) bitset_index) << bitset_shift;
        return(result);
      } /* if */
    } /* while */
    raise_error(RANGE_ERROR);
    return(0);
  } /* setMax */



#ifdef ANSI_C

inttype setMin (const const_settype set1)
#else

inttype setMin (set1)
settype set1;
#endif

  {
    memsizetype bitset_size;
    memsizetype bitset_index;
    bitsettype curr_bitset;
    inttype result;

  /* setMin */
    bitset_size = (uinttype) (set1->max_position - set1->min_position + 1);
    bitset_index = 0;
    while (bitset_index < bitset_size) {
      curr_bitset = set1->bitset[bitset_index];
      if (curr_bitset) {
        result = uintLeastSignificantBit(curr_bitset);
        result += (set1->min_position + (inttype) bitset_index) << bitset_shift;
        return(result);
      } /* if */
      bitset_index++;
    } /* while */
    raise_error(RANGE_ERROR);
    return(0);
  } /* setMin */



#ifdef ANSI_C

booltype setNotElem (const inttype number, const const_settype set1)
#else

booltype setNotElem (number, set1)
inttype number;
settype set1;
#endif

  {
    inttype position;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* setNotElem */
    position = bitset_pos(number);
    if (position >= set1->min_position && position <= set1->max_position) {
      bitset_index = (uinttype) (position - set1->min_position);
      bit_index = ((unsigned int) number) & bitset_mask;
      if (set1->bitset[bitset_index] & (bitsettype) 1 << bit_index) {
        return(FALSE);
      } else {
        return(TRUE);
      } /* if */
    } else {
      return(TRUE);
    } /* if */
  } /* setNotElem */



#ifdef ANSI_C

inttype setRand (const const_settype set1)
#else

inttype setRand (set1)
settype set1;
#endif

  {
    inttype num_elements;
    inttype elem_index;
    memsizetype bitset_size;
    memsizetype bitset_index;
    bitsettype curr_bitset;
    inttype result;

  /* setRand */
    num_elements = setCard(set1);
    if (num_elements == 0) {
      raise_error(RANGE_ERROR);
      return(0);
    } else {
      elem_index = intRand(1, num_elements);
      bitset_size = (uinttype) (set1->max_position - set1->min_position + 1);
      bitset_index = 0;
      while (bitset_index < bitset_size) {
        curr_bitset = set1->bitset[bitset_index];
        while (curr_bitset) {
          result = uintLeastSignificantBit(curr_bitset);
          elem_index--;
          if (elem_index == 0) {
            result += (set1->min_position + (inttype) bitset_index) << bitset_shift;
            return(result);
          } /* if */
          curr_bitset &= ~((bitsettype) 1 << result);
        } /* while */
        bitset_index++;
      } /* while */
    } /* if */
    raise_error(RANGE_ERROR);
    return(0);
  } /* setRand */



#ifdef ANSI_C

inttype setSConv (const const_settype set1)
#else

inttype setSConv (set1)
settype set1;
#endif

  { /* setSConv */
    if (set1->min_position == 0 && set1->max_position == 0) {
      return((inttype) set1->bitset[0]);
    } else {
      raise_error(RANGE_ERROR);
      return(0);
    } /* if */
  } /* setSConv */



#ifdef ANSI_C

settype setSymdiff (const const_settype set1, const const_settype set2)
#else

settype setSymdiff (set1, set2)
settype set1;
settype set2;
#endif

  {
    inttype position;
    inttype min_position;
    inttype max_position;
    inttype start_position;
    inttype stop_position;
    settype result;

  /* setSymdiff */
    if (set1->min_position < set2->min_position) {
      min_position = set1->min_position;
      start_position = set2->min_position;
    } else {
      min_position = set2->min_position;
      start_position = set1->min_position;
    } /* if */
    if (set1->max_position > set2->max_position) {
      max_position = set1->max_position;
      stop_position = set2->max_position;
    } else {
      max_position = set2->max_position;
      stop_position = set1->max_position;
    } /* if */
    if (!ALLOC_SET(result, (uinttype) (max_position - min_position + 1))) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->min_position = min_position;
      result->max_position = max_position;
      for (position = set1->min_position; position < set2->min_position; position++) {
        result->bitset[position - min_position] = set1->bitset[position - set1->min_position];
      } /* for */
      for (position = set2->min_position; position < set1->min_position; position++) {
        result->bitset[position - min_position] = set2->bitset[position - set2->min_position];
      } /* for */
      for (position = start_position; position <= stop_position; position++) {
        result->bitset[position - min_position] =
            set1->bitset[position - set1->min_position] ^
            set2->bitset[position - set2->min_position];
      } /* for */
      for (position = stop_position + 1; position < start_position; position++) {
        result->bitset[position - min_position] = 0;
      } /* for */
      for (position = set1->max_position; position < set2->max_position; position++) {
        result->bitset[position - min_position] = set1->bitset[position - set1->min_position];
      } /* for */
      for (position = set2->max_position; position < set1->max_position; position++) {
        result->bitset[position - min_position] = set2->bitset[position - set2->min_position];
      } /* for */
      return(result);
    } /* if */
  } /* setSymdiff */



#ifdef ANSI_C

inttype setToInt (const const_settype set1, const inttype lowestBitNum)
#else

inttype setToInt (set1, lowestBitNum)
settype set1;
inttype lowestBitNum;
#endif

  {
    inttype position;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* setToInt */
    position = bitset_pos(lowestBitNum);
    if (position >= set1->min_position && position <= set1->max_position) {
      bitset_index = (uinttype) (position - set1->min_position);
      bit_index = ((unsigned int) lowestBitNum) & bitset_mask;
      if (bit_index == 0) {
        return((inttype) set1->bitset[bitset_index]);
      } else if (position < set1->max_position) {
        return((inttype) (set1->bitset[bitset_index] >> bit_index |
            set1->bitset[bitset_index + 1] << (8 * sizeof(bitsettype) - bit_index)));
      } else {
        return((inttype) (set1->bitset[bitset_index] >> bit_index));
      } /* if */
    } else if (position == set1->min_position - 1) {
      bitset_index = (uinttype) (position - set1->min_position);
      bit_index = ((unsigned int) lowestBitNum) & bitset_mask;
      return((inttype) (set1->bitset[bitset_index + 1] << (8 * sizeof(bitsettype) - bit_index)));
    } else {
      return(0);
    } /* if */
  } /* setToInt */



#ifdef ANSI_C

settype setUnion (const const_settype set1, const const_settype set2)
#else

settype setUnion (set1, set2)
settype set1;
settype set2;
#endif

  {
    inttype position;
    inttype min_position;
    inttype max_position;
    inttype start_position;
    inttype stop_position;
    inttype set1_start;
    inttype set2_start;
    inttype set1_stop;
    inttype set2_stop;
    settype result;

  /* setUnion */
#ifdef OUT_OF_ORDER
    printf("set_union(\n");
    prot_set(set1);
    printf(",\n");
    prot_set(set2);
    printf(")\n");
#endif
    if (set1->min_position < set2->min_position) {
      min_position = set1->min_position;
      start_position = set2->min_position;
    } else {
      min_position = set2->min_position;
      start_position = set1->min_position;
    } /* if */
    if (set1->max_position > set2->max_position) {
      max_position = set1->max_position;
      stop_position = set2->max_position;
    } else {
      max_position = set2->max_position;
      stop_position = set1->max_position;
    } /* if */
    if (set1->max_position < set2->min_position) {
      set1_stop = set1->max_position;
    } else {
      set1_stop = set2->min_position - 1;
    } /* if */
    if (set2->max_position < set1->min_position) {
      set2_stop = set2->max_position;
    } else {
      set2_stop = set1->min_position - 1;
    } /* if */
    if (set1->min_position > set2->max_position) {
      set1_start = set1->min_position;
    } else {
      set1_start = set2->max_position + 1;
    } /* if */
    if (set2->min_position > set1->max_position) {
      set2_start = set2->min_position;
    } else {
      set2_start = set1->max_position + 1;
    } /* if */
#ifdef OUT_OF_ORDER
    printf("min_position=%ld, max_position=%ld, start_position=%ld, stop_position=%ld\n",
           min_position, max_position, start_position, stop_position);
    printf("set1_stop=%ld, set2_stop=%ld\n", set1_stop, set2_stop);
#endif
    if (!ALLOC_SET(result, (uinttype) (max_position - min_position + 1))) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->min_position = min_position;
      result->max_position = max_position;
      for (position = set1->min_position; position <= set1_stop; position++) {
        /* printf("A position=%ld bitset[%ld] = bitset1[%ld] (%lu)\n",
               position, position - min_position,
               position - set1->min_position, set1->bitset[position - set1->min_position]); */
        result->bitset[position - min_position] = set1->bitset[position - set1->min_position];
      } /* for */
      for (position = set2->min_position; position <= set2_stop; position++) {
        /* printf("B position=%ld bitset[%ld] = bitset2[%ld] (%lu)\n",
               position, position - min_position,
               position - set2->min_position, set2->bitset[position - set2->min_position]); */
        result->bitset[position - min_position] = set2->bitset[position - set2->min_position];
      } /* for */
      for (position = start_position; position <= stop_position; position++) {
        /* printf("C position=%ld bitset[%ld]\n",
               position, position - min_position); */
        result->bitset[position - min_position] =
            set1->bitset[position - set1->min_position] |
            set2->bitset[position - set2->min_position];
      } /* for */
      for (position = stop_position + 1; position < start_position; position++) {
        /* printf("D position=%ld bitset[%ld] = 0\n",
               position, position - min_position); */
        result->bitset[position - min_position] = 0;
      } /* for */
      for (position = set1_start; position <= set1->max_position; position++) {
        /* printf("E position=%ld bitset[%ld] = bitset1[%ld] (%lu)\n",
               position, position - min_position,
               position - set1->min_position, set1->bitset[position - set1->min_position]); */
        result->bitset[position - min_position] = set1->bitset[position - set1->min_position];
      } /* for */
      for (position = set2_start; position <= set2->max_position; position++) {
        /* printf("F position=%ld bitset[%ld] = bitset2[%ld] (%lu)\n",
               position, position - min_position,
               position - set2->min_position, set2->bitset[position - set2->min_position]); */
        result->bitset[position - min_position] = set2->bitset[position - set2->min_position];
      } /* for */
#ifdef OUT_OF_ORDER
      printf("setUnion ==> ");
      prot_set(result);
      printf("\n");
#endif
      return(result);
    } /* if */
  } /* setUnion */
