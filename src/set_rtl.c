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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
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
    settype result;

  /* setArrlit */
    length = arr1->max_position - arr1->min_position + 1;
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
        position = number >> bitset_shift;
        result->min_position = position;
        result->max_position = position;
        bit_index = ((unsigned int) number) & bitset_mask;
        result->bitset[0] = (1 << bit_index);
        for (number = 1; number < length; number++) {
          setIncl(&result, arr1->arr[number].value.intvalue);
#ifdef OUT_OF_ORDER
          if (fail_flag) {
            FREE_SET(result, result->max_position - result->min_position + 1);
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
      position = number >> bitset_shift;
      result->min_position = position;
      result->max_position = position;
      bit_index = ((unsigned int) number) & bitset_mask;
      result->bitset[0] = (1 << bit_index);
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
    int idx;
    inttype result;

  /* setCard */
    result = 0;
    for (bitset_index = 0; bitset_index <= set1->max_position - set1->min_position; bitset_index++) {
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



#ifdef ANSI_C

inttype setCmp (const const_settype set1, const const_settype set2)
#else

inttype setCmp (set1, set2)
settype set1;
settype set2;
#endif

  {
    memsizetype min1_index;
    memsizetype min2_index;
    memsizetype bitset_index;
    memsizetype size;
    const bitsettype *bitset1;
    const bitsettype *bitset2;

  /* setCmp */
    if (set1->min_position <= set2->min_position) {
      min1_index = set2->min_position - set1->min_position;
      min2_index = 0;
      for (bitset_index = 0; bitset_index < min1_index; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return(-1);
        } /* if */
      } /* for */
    } else if (set1->min_position > set2->min_position) {
      min1_index = 0;
      min2_index = set1->min_position - set2->min_position;
      for (bitset_index = 0; bitset_index < min2_index; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          return(1);
        } /* if */
      } /* for */
    } /* if */
    if (set1->max_position >= set2->max_position) {
      size = set2->max_position - set2->min_position - min2_index + 1;
    } else {
      size = set1->max_position - set1->min_position - min1_index + 1;
    } /* if */
    bitset1 = &set1->bitset[min1_index];
    bitset2 = &set2->bitset[min2_index];
    bitset_index = 0;
    while (bitset_index < size) {
      if (*bitset1 != *bitset2) {
        if (least_significant_bit(*bitset1) < least_significant_bit(*bitset2)) {
          return(-1);
        } else {
          return(1);
        } /* if */
      } /* if */
      bitset1++;
      bitset2++;
      bitset_index++;
    } /* while */
    if (set1->max_position >= set2->max_position) {
      for (bitset_index = set2->max_position - set1->min_position + 1;
          bitset_index <= set1->max_position - set1->min_position; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return(1);
        } /* if */
      } /* for */
    } else {
      for (bitset_index = set1->max_position - set2->min_position + 1;
          bitset_index <= set2->max_position - set2->min_position; bitset_index++) {
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
    set_source_size = set_from->max_position - set_from->min_position + 1;
    if (set_dest->min_position != set_from->min_position ||
        set_dest->max_position != set_from->max_position) {
      set_dest_size = set_dest->max_position - set_dest->min_position + 1;
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
        (SIZE_TYPE) set_source_size * sizeof(bitsettype));
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
    new_size = set_from->max_position - set_from->min_position + 1;
    if (!ALLOC_SET(result, new_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = set_from->min_position;
      result->max_position = set_from->max_position;
      memcpy(result->bitset, set_from->bitset,
          (SIZE_TYPE) new_size * sizeof(bitsettype));
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
      FREE_SET(old_set, old_set->max_position - old_set->min_position + 1);
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
    memsizetype start_index;
    memsizetype stop_index;
    memsizetype bitset_index;
    settype result;

  /* setDiff */
    if (!ALLOC_SET(result, set1->max_position - set1->min_position + 1)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->min_position = set1->min_position;
      result->max_position = set1->max_position;
      if (set2->max_position >= set1->min_position ||
          set2->min_position <= set1->max_position) {
        if (set2->min_position > set1->min_position) {
          start_index = set2->min_position - set1->min_position;
          memcpy(result->bitset, set1->bitset, start_index * sizeof(bitsettype));
        } else {
          start_index = 0;
        } /* if */
        if (set1->max_position > set2->max_position) {
          stop_index = set2->max_position - set1->min_position;
          memcpy(&result->bitset[stop_index + 1], &set1->bitset[stop_index + 1],
              (set1->max_position - set2->max_position) * sizeof(bitsettype));
        } else {
          stop_index = set1->max_position - set1->min_position;
        } /* if */
        for (bitset_index = start_index; bitset_index <= stop_index; bitset_index++) {
          result->bitset[bitset_index] = set1->bitset[bitset_index] &
              ~ set2->bitset[bitset_index + set1->min_position - set2->min_position];
        } /* for */
      } else {
        memcpy(result->bitset, set1->bitset,
            (set1->max_position - set1->min_position + 1) * sizeof(bitsettype));
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
    position = number >> bitset_shift;
    if (position >= set1->min_position && position <= set1->max_position) {
      bitset_index = (memsizetype) (position - set1->min_position);
      bit_index = ((unsigned int) number) & bitset_mask;
      if (set1->bitset[bitset_index] & (1 << bit_index)) {
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
    memsizetype min1_index;
    memsizetype min2_index;
    memsizetype bitset_index;
    memsizetype size;

  /* setEq */
    if (set1->min_position == set2->min_position &&
        set1->max_position == set2->max_position) {
      return(memcmp(set1->bitset, set2->bitset,
          (set1->max_position - set1->min_position + 1) *
          sizeof(bitsettype)) == 0);
    } else {
      if (set1->min_position <= set2->min_position) {
        min1_index = set2->min_position - set1->min_position;
        min2_index = 0;
        for (bitset_index = 0; bitset_index < min1_index; bitset_index++) {
          if (set1->bitset[bitset_index] != 0) {
            return(FALSE);
          } /* if */
        } /* for */
      } else {
        min1_index = 0;
        min2_index = set1->min_position - set2->min_position;
        for (bitset_index = 0; bitset_index < min2_index; bitset_index++) {
          if (set2->bitset[bitset_index] != 0) {
            return(FALSE);
          } /* if */
        } /* for */
      } /* if */
      if (set1->max_position >= set2->max_position) {
        size = set2->max_position - set2->min_position - min2_index + 1;
        for (bitset_index = set2->max_position - set1->min_position + 1;
            bitset_index <= set1->max_position - set1->min_position; bitset_index++) {
          if (set1->bitset[bitset_index] != 0) {
            return(FALSE);
          } /* if */
        } /* for */
      } else {
        size = set1->max_position - set1->min_position - min1_index + 1;
        for (bitset_index = set1->max_position - set2->min_position + 1;
            bitset_index <= set2->max_position - set2->min_position; bitset_index++) {
          if (set2->bitset[bitset_index] != 0) {
            return(FALSE);
          } /* if */
        } /* for */
      } /* if */
      return(memcmp(&set1->bitset[min1_index],
          &set2->bitset[min2_index], size * sizeof(bitsettype)) == 0);
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
    position = number >> bitset_shift;
    if (position >= set_dest->min_position && position <= set_dest->max_position) {
      bitset_index = (memsizetype) (position - set_dest->min_position);
      bit_index = ((unsigned int) number) & bitset_mask;
      set_dest->bitset[bitset_index] &= ~((bitsettype)(1 << bit_index));
#ifdef OUT_OF_ORDER
      if (set_dest->bitset[bitset_index] == 0) {
        if 
      } /* if */
#endif
    } /* if */
  } /* setExcl */



#ifdef ANSI_C

booltype setGe (const const_settype set1, const const_settype set2)
#else

booltype setGe (set1, set2)
settype set1;
settype set2;
#endif

  {
    memsizetype min1_index;
    memsizetype min2_index;
    memsizetype bitset_index;
    memsizetype size;
    const bitsettype *bitset1;
    const bitsettype *bitset2;
    booltype greater_equal;

  /* setGe */
    greater_equal = TRUE;
    if (set1->min_position <= set2->min_position) {
      min1_index = set2->min_position - set1->min_position;
      min2_index = 0;
    } else {
      min1_index = 0;
      min2_index = set1->min_position - set2->min_position;
      for (bitset_index = 0; bitset_index < min2_index; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          greater_equal = FALSE;
        } /* if */
      } /* for */
    } /* if */
    if (greater_equal) {
      if (set1->max_position >= set2->max_position) {
        size = set2->max_position - set2->min_position - min2_index + 1;
      } else {
        size = set1->max_position - set1->min_position - min1_index + 1;
        for (bitset_index = set1->max_position - set2->min_position + 1;
            bitset_index <= set2->max_position - set2->min_position; bitset_index++) {
          if (set2->bitset[bitset_index] != 0) {
            greater_equal = FALSE;
          } /* if */
        } /* for */
      } /* if */
      if (greater_equal) {
        bitset1 = &set1->bitset[min1_index];
        bitset2 = &set2->bitset[min2_index];
        bitset_index = 0;
        while (greater_equal && bitset_index < size) {
          greater_equal = (*bitset1 | ~ *bitset2) == ~((bitsettype) 0);
          bitset1++;
          bitset2++;
          bitset_index++;
        } /* while */
      } /* if */
    } /* if */
    return(greater_equal);
  } /* setGe */



#ifdef ANSI_C

booltype setGt (const const_settype set1, const const_settype set2)
#else

booltype setGt (set1, set2)
settype set1;
settype set2;
#endif

  {
    memsizetype min1_index;
    memsizetype min2_index;
    memsizetype bitset_index;
    memsizetype size;
    const bitsettype *bitset1;
    const bitsettype *bitset2;
    booltype greater_equal;
    booltype equal;

  /* setGt */
    greater_equal = TRUE;
    equal = TRUE;
    if (set1->min_position <= set2->min_position) {
      min1_index = set2->min_position - set1->min_position;
      min2_index = 0;
      for (bitset_index = 0; bitset_index < min1_index; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          equal = FALSE;
        } /* if */
      } /* for */
    } else {
      min1_index = 0;
      min2_index = set1->min_position - set2->min_position;
      for (bitset_index = 0; bitset_index < min2_index; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          greater_equal = FALSE;
        } /* if */
      } /* for */
    } /* if */
    if (greater_equal) {
      if (set1->max_position >= set2->max_position) {
        size = set2->max_position - set2->min_position - min2_index + 1;
        for (bitset_index = set2->max_position - set1->min_position + 1;
            bitset_index <= set1->max_position - set1->min_position; bitset_index++) {
          if (set1->bitset[bitset_index] != 0) {
            equal = FALSE;
          } /* if */
        } /* for */
      } else {
        size = set1->max_position - set1->min_position - min1_index + 1;
        for (bitset_index = set1->max_position - set2->min_position + 1;
            bitset_index <= set2->max_position - set2->min_position; bitset_index++) {
          if (set2->bitset[bitset_index] != 0) {
            greater_equal = FALSE;
          } /* if */
        } /* for */
      } /* if */
      if (greater_equal) {
        bitset1 = &set1->bitset[min1_index];
        bitset2 = &set2->bitset[min2_index];
        bitset_index = 0;
        while (greater_equal && bitset_index < size) {
          greater_equal = (*bitset1 | ~ *bitset2) == ~((bitsettype) 0);
          if (*bitset1 != *bitset2) {
            equal = FALSE;
          } /* if */
          bitset1++;
          bitset2++;
          bitset_index++;
        } /* while */
      } /* if */
    } /* if */
    return(greater_equal && !equal);
  } /* setGt */



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
        bitset_index < set1->max_position - set1->min_position; bitset_index++) {
      result ^= set1->bitset[bitset_index];
    } /* for */
    return(result);
  } /* setHashCode */



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
    position = number >> bitset_shift;
    if (position > set_dest->max_position) {
      old_size = set_dest->max_position - set_dest->min_position + 1;
      new_size = position - set_dest->min_position + 1;
      if (!RESIZE_SET(set_dest, old_size, new_size)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        COUNT3_SET(old_size, new_size);
        *set_to = set_dest;
        set_dest->max_position = position;
        memset(&set_dest->bitset[old_size], 0, (new_size - old_size) * sizeof(bitsettype));
      } /* if */
    } else if (position < set_dest->min_position) {
      old_size = set_dest->max_position - set_dest->min_position + 1;
      new_size = set_dest->max_position - position + 1;
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
    bitset_index = (memsizetype) (position - set_dest->min_position);
    bit_index = ((unsigned int) number) & bitset_mask;
    set_dest->bitset[bitset_index] |= (1 << bit_index);
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
      if (!ALLOC_SET(result, max_position - min_position + 1)) {
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

booltype setLe (const const_settype set1, const const_settype set2)
#else

booltype setLe (set1, set2)
settype set1;
settype set2;
#endif

  {
    memsizetype min1_index;
    memsizetype min2_index;
    memsizetype bitset_index;
    memsizetype size;
    const bitsettype *bitset1;
    const bitsettype *bitset2;
    booltype less_equal;

  /* setLe */
    less_equal = TRUE;
    if (set1->min_position < set2->min_position) {
      min1_index = set2->min_position - set1->min_position;
      min2_index = 0;
      for (bitset_index = 0; bitset_index < min1_index; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          less_equal = FALSE;
        } /* if */
      } /* for */
    } else {
      min1_index = 0;
      min2_index = set1->min_position - set2->min_position;
    } /* if */
    if (less_equal) {
      if (set1->max_position > set2->max_position) {
        size = set2->max_position - set2->min_position - min2_index + 1;
        for (bitset_index = set2->max_position - set1->min_position + 1;
            bitset_index <= set1->max_position - set1->min_position; bitset_index++) {
          if (set1->bitset[bitset_index] != 0) {
            less_equal = FALSE;
          } /* if */
        } /* for */
      } else {
        size = set1->max_position - set1->min_position - min1_index + 1;
      } /* if */
      if (less_equal) {
        bitset1 = &set1->bitset[min1_index];
        bitset2 = &set2->bitset[min2_index];
        bitset_index = 0;
        while (less_equal && bitset_index < size) {
          less_equal = (~ *bitset1 | *bitset2) == ~((bitsettype) 0);
          bitset1++;
          bitset2++;
          bitset_index++;
        } /* while */
      } /* if */
    } /* if */
    return(less_equal);
  } /* setLe */



#ifdef ANSI_C

booltype setLt (const const_settype set1, const const_settype set2)
#else

booltype setLt (set1, set2)
settype set1;
settype set2;
#endif

  {
    memsizetype min1_index;
    memsizetype min2_index;
    memsizetype bitset_index;
    memsizetype size;
    const bitsettype *bitset1;
    const bitsettype *bitset2;
    booltype less_equal;
    booltype equal;

  /* setLt */
    less_equal = TRUE;
    equal = TRUE;
    if (set1->min_position < set2->min_position) {
      min1_index = set2->min_position - set1->min_position;
      min2_index = 0;
      for (bitset_index = 0; bitset_index < min1_index; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          less_equal = FALSE;
        } /* if */
      } /* for */
    } else {
      min1_index = 0;
      min2_index = set1->min_position - set2->min_position;
      for (bitset_index = 0; bitset_index < min2_index; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          equal = FALSE;
        } /* if */
      } /* for */
    } /* if */
    if (less_equal) {
      if (set1->max_position > set2->max_position) {
        size = set2->max_position - set2->min_position - min2_index + 1;
        for (bitset_index = set2->max_position - set1->min_position + 1;
            bitset_index <= set1->max_position - set1->min_position; bitset_index++) {
          if (set1->bitset[bitset_index] != 0) {
            less_equal = FALSE;
          } /* if */
        } /* for */
      } else {
        size = set1->max_position - set1->min_position - min1_index + 1;
        for (bitset_index = set1->max_position - set2->min_position + 1;
            bitset_index <= set2->max_position - set2->min_position; bitset_index++) {
          if (set2->bitset[bitset_index] != 0) {
            equal = FALSE;
          } /* if */
        } /* for */
      } /* if */
      if (less_equal) {
        bitset1 = &set1->bitset[min1_index];
        bitset2 = &set2->bitset[min2_index];
        bitset_index = 0;
        while (less_equal && bitset_index < size) {
          less_equal = (~ *bitset1 | *bitset2) == ~((bitsettype) 0);
          if (*bitset1 != *bitset2) {
            equal = FALSE;
          } /* if */
          bitset1++;
          bitset2++;
          bitset_index++;
        } /* while */
      } /* if */
    } /* if */
    return(less_equal && !equal);
  } /* setLt */



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
    bitset_index = set1->max_position - set1->min_position + 1;
    while (bitset_index > 0) {
      bitset_index--;
      curr_bitset = set1->bitset[bitset_index];
      if (curr_bitset) {
        result = most_significant_bit(curr_bitset);
        result += (set1->min_position + bitset_index) << bitset_shift;
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
    bitset_size = set1->max_position - set1->min_position + 1;
    bitset_index = 0;
    while (bitset_index < bitset_size) {
      curr_bitset = set1->bitset[bitset_index];
      if (curr_bitset) {
        result = least_significant_bit(curr_bitset);
        result += (set1->min_position + bitset_index) << bitset_shift;
        return(result);
      } /* if */
      bitset_index++;
    } /* while */
    raise_error(RANGE_ERROR);
    return(0);
  } /* setMin */



#ifdef ANSI_C

booltype setNe (const const_settype set1, const const_settype set2)
#else

booltype setNe (set1, set2)
settype set1;
settype set2;
#endif

  {
    memsizetype min1_index;
    memsizetype min2_index;
    memsizetype bitset_index;
    memsizetype size;

  /* setNe */
    if (set1->min_position == set2->min_position &&
        set1->max_position == set2->max_position) {
      return(memcmp(set1->bitset, set2->bitset,
          (set1->max_position - set1->min_position + 1) *
          sizeof(bitsettype)) != 0);
    } else {
      if (set1->min_position <= set2->min_position) {
        min1_index = set2->min_position - set1->min_position;
        min2_index = 0;
        for (bitset_index = 0; bitset_index < min1_index; bitset_index++) {
          if (set1->bitset[bitset_index] != 0) {
            return(TRUE);
          } /* if */
        } /* for */
      } else {
        min1_index = 0;
        min2_index = set1->min_position - set2->min_position;
        for (bitset_index = 0; bitset_index < min2_index; bitset_index++) {
          if (set2->bitset[bitset_index] != 0) {
            return(TRUE);
          } /* if */
        } /* for */
      } /* if */
      if (set1->max_position >= set2->max_position) {
        size = set2->max_position - set2->min_position - min2_index + 1;
        for (bitset_index = set2->max_position - set1->min_position + 1;
            bitset_index <= set1->max_position - set1->min_position; bitset_index++) {
          if (set1->bitset[bitset_index] != 0) {
            return(TRUE);
          } /* if */
        } /* for */
      } else {
        size = set1->max_position - set1->min_position - min1_index + 1;
        for (bitset_index = set1->max_position - set2->min_position + 1;
            bitset_index <= set2->max_position - set2->min_position; bitset_index++) {
          if (set2->bitset[bitset_index] != 0) {
            return(TRUE);
          } /* if */
        } /* for */
      } /* if */
      return(memcmp(&set1->bitset[min1_index],
          &set2->bitset[min2_index], size * sizeof(bitsettype)) != 0);
    } /* if */
  } /* setNe */



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
    position = number >> bitset_shift;
    if (position >= set1->min_position && position <= set1->max_position) {
      bitset_index = (memsizetype) (position - set1->min_position);
      bit_index = ((unsigned int) number) & bitset_mask;
      if (set1->bitset[bitset_index] & (1 << bit_index)) {
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
      bitset_size = set1->max_position - set1->min_position + 1;
      bitset_index = 0;
      while (bitset_index < bitset_size) {
        curr_bitset = set1->bitset[bitset_index];
        while (curr_bitset) {
          result = least_significant_bit(curr_bitset);
          elem_index--;
          if (elem_index == 0) {
            result += (set1->min_position + bitset_index) << bitset_shift;
            return(result);
          } /* if */
          curr_bitset &= ~((bitsettype)(1 << result));
        } /* while */
        bitset_index++;
      } /* while */
    } /* if */
    raise_error(RANGE_ERROR);
    return(0);
  } /* setRand */



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
    if (!ALLOC_SET(result, max_position - min_position + 1)) {
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
    if (!ALLOC_SET(result, max_position - min_position + 1)) {
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
