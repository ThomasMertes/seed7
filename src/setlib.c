/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  Module: Library                                                 */
/*  File: seed7/src/setlib.c                                        */
/*  Changes: 2004  Thomas Mertes                                    */
/*  Content: All primitive actions for the set type.                */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "runerr.h"
#include "memory.h"
#include "int_rtl.h"
#include "set_rtl.h"

#undef EXTERN
#define EXTERN
#include "setlib.h"


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

objecttype set_baselit (listtype arguments)
#else

objecttype set_baselit (arguments)
listtype arguments;
#endif

  {
    inttype number;
    inttype position;
    unsigned int bit_index;
    settype result;

  /* set_baselit */
    isit_int(arg_2(arguments));
    number = take_int(arg_2(arguments));
    if (!ALLOC_SET(result, 1)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      COUNT_SET(1);
      position = number >> bitset_shift;
      result->min_position = position;
      result->max_position = position;
      bit_index = ((unsigned int) number) & bitset_mask;
      result->bitset[0] = (1 << bit_index);
      return(bld_set_temp(result));
    } /* if */
  } /* set_baselit */



#ifdef ANSI_C

objecttype set_card (listtype arguments)
#else

objecttype set_card (arguments)
listtype arguments;
#endif

  {
    settype set1;
    memsizetype bitset_index;
    unsigned char *byte;
    int idx;
    inttype result;

  /* set_card */
    isit_set(arg_1(arguments));
    set1 = take_set(arg_1(arguments));
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
    return(bld_int_temp(result));
  } /* set_card */



#ifdef ANSI_C

objecttype set_conv (listtype arguments)
#else

objecttype set_conv (arguments)
listtype arguments;
#endif

  {
    objecttype set_arg;
    settype result;

  /* set_conv */
    set_arg = arg_3(arguments);
    isit_set(set_arg);
    if (TEMP_OBJECT(set_arg)) {
      result = take_set(set_arg);
      set_arg->value.setvalue = NULL;
      return(bld_set_temp(result));
    } else {
      return(set_arg);
    } /* if */
  } /* set_conv */



#ifdef ANSI_C

objecttype set_cpy (listtype arguments)
#else

objecttype set_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype set_to;
    objecttype set_from;
    settype set_dest;
    settype set_source;
    memsizetype set_dest_size;
    memsizetype set_source_size;

  /* set_cpy */
    set_to = arg_1(arguments);
    set_from = arg_3(arguments);
    isit_set(set_to);
    isit_set(set_from);
    is_variable(set_to);
    set_dest = take_set(set_to);
    set_source = take_set(set_from);
    if (TEMP_OBJECT(set_from)) {
      set_dest_size = set_dest->max_position - set_dest->min_position + 1;
      FREE_SET(set_dest, set_dest_size);
      set_to->value.setvalue = set_source;
      set_from->value.setvalue = NULL;
    } else {
      set_source_size = set_source->max_position - set_source->min_position + 1;
      if (set_dest->min_position != set_source->min_position ||
          set_dest->max_position != set_source->max_position) {
        set_dest_size = set_dest->max_position - set_dest->min_position + 1;
        if (set_dest_size != set_source_size) {
          if (!ALLOC_SET(set_dest, set_source_size)) {
            return(raise_exception(SYS_MEM_EXCEPTION));
          } else {
            COUNT_SET(set_source_size);
            FREE_SET(set_to->value.setvalue, set_dest_size);
            set_to->value.setvalue = set_dest;
          } /* if */
        } /* if */
        set_dest->min_position = set_source->min_position;
        set_dest->max_position = set_source->max_position;
      } /* if */
      memcpy(set_dest->bitset, set_source->bitset,
          (SIZE_TYPE) set_source_size * sizeof(bitsettype));
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* set_cpy */



#ifdef ANSI_C

objecttype set_create (listtype arguments)
#else

objecttype set_create (arguments)
listtype arguments;
#endif

  {
    objecttype set_to;
    objecttype set_from;
    settype set_source;
    memsizetype new_size;
    settype new_set;

  /* set_create */
    set_to = arg_1(arguments);
    set_from = arg_3(arguments);
    isit_set(set_from);
    set_source = take_set(set_from);
    SET_CLASS_OF_OBJ(set_to, SETOBJECT);
    if (TEMP_OBJECT(set_from)) {
      set_to->value.setvalue = set_source;
      set_from->value.setvalue = NULL;
    } else {
      new_size = set_source->max_position - set_source->min_position + 1;
      if (!ALLOC_SET(new_set, new_size)) {
        set_to->value.setvalue = NULL;
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        COUNT_SET(new_size);
        set_to->value.setvalue = new_set;
        new_set->min_position = set_source->min_position;
        new_set->max_position = set_source->max_position;
        memcpy(new_set->bitset, set_source->bitset,
            (SIZE_TYPE) new_size * sizeof(bitsettype));
      } /* if */
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* set_create */



#ifdef ANSI_C

objecttype set_destr (listtype arguments)
#else

objecttype set_destr (arguments)
listtype arguments;
#endif

  {
    settype old_set;

  /* set_destr */
    isit_set(arg_1(arguments));
    old_set = take_set(arg_1(arguments));
    if (old_set != NULL) {
      FREE_SET(old_set, old_set->max_position - old_set->min_position + 1);
      arg_1(arguments)->value.setvalue = NULL;
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* set_destr */



#ifdef ANSI_C

objecttype set_diff (listtype arguments)
#else

objecttype set_diff (arguments)
listtype arguments;
#endif

  {
    settype set1;
    settype set2;
    memsizetype start_index;
    memsizetype stop_index;
    memsizetype bitset_index;
    settype result;

  /* set_diff */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    set1 = take_set(arg_1(arguments));
    set2 = take_set(arg_3(arguments));
    if (!ALLOC_SET(result, set1->max_position - set1->min_position + 1)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      COUNT_SET(set1->max_position - set1->min_position + 1);
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
      return(bld_set_temp(result));
    } /* if */
  } /* set_diff */



#ifdef ANSI_C

objecttype set_elem (listtype arguments)
#else

objecttype set_elem (arguments)
listtype arguments;
#endif

  {
    settype set1;
    inttype number;
    inttype position;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* set_elem */
    isit_int(arg_1(arguments));
    isit_set(arg_3(arguments));
    number = take_int(arg_1(arguments));
    set1 = take_set(arg_3(arguments));
    position = number >> bitset_shift;
    if (position >= set1->min_position && position <= set1->max_position) {
      bitset_index = (memsizetype) (position - set1->min_position);
      bit_index = ((unsigned int) number) & bitset_mask;
      if (set1->bitset[bitset_index] & (1 << bit_index)) {
        return(SYS_TRUE_OBJECT);
      } else {
        return(SYS_FALSE_OBJECT);
      } /* if */
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* set_elem */



#ifdef ANSI_C

objecttype set_empty (listtype arguments)
#else

objecttype set_empty (arguments)
listtype arguments;
#endif

  {
    settype result;

  /* set_empty */
    if (!ALLOC_SET(result, 1)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      COUNT_SET(1);
      result->min_position = 0;
      result->max_position = 0;
      memset(result->bitset, 0, sizeof(bitsettype));
      return(bld_set_temp(result));
    } /* if */
  } /* set_empty */



#ifdef ANSI_C

objecttype set_eq (listtype arguments)
#else

objecttype set_eq (arguments)
listtype arguments;
#endif

  { /* set_eq */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    if (setEq(take_set(arg_1(arguments)), take_set(arg_3(arguments)))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* set_eq */



#ifdef ANSI_C

objecttype set_excl (listtype arguments)
#else

objecttype set_excl (arguments)
listtype arguments;
#endif

  {
    objecttype set_to;
    settype set_dest;
    inttype number;
    inttype position;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* set_excl */
    set_to = arg_1(arguments);
    isit_set(set_to);
    is_variable(set_to);
    set_dest = take_set(set_to);
    isit_int(arg_2(arguments));
    number = take_int(arg_2(arguments));
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
    return(SYS_EMPTY_OBJECT);
  } /* set_excl */



#ifdef ANSI_C

objecttype set_ge (listtype arguments)
#else

objecttype set_ge (arguments)
listtype arguments;
#endif

  {
    settype set1;
    settype set2;
    memsizetype min1_index;
    memsizetype min2_index;
    memsizetype bitset_index;
    memsizetype size;
    bitsettype *bitset1;
    bitsettype *bitset2;
    booltype greater_equal;

  /* set_ge */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    set1 = take_set(arg_1(arguments));
    set2 = take_set(arg_3(arguments));
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
    if (greater_equal) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* set_ge */



#ifdef ANSI_C

objecttype set_gt (listtype arguments)
#else

objecttype set_gt (arguments)
listtype arguments;
#endif

  {
    settype set1;
    settype set2;
    memsizetype min1_index;
    memsizetype min2_index;
    memsizetype bitset_index;
    memsizetype size;
    bitsettype *bitset1;
    bitsettype *bitset2;
    booltype greater_equal;
    booltype equal;

  /* set_gt */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    set1 = take_set(arg_1(arguments));
    set2 = take_set(arg_3(arguments));
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
    if (greater_equal && !equal) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* set_gt */



#ifdef ANSI_C

objecttype set_idx (listtype arguments)
#else

objecttype set_idx (arguments)
listtype arguments;
#endif

  {
    settype set1;
    inttype number;
    inttype position;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* set_idx */
    isit_set(arg_1(arguments));
    isit_int(arg_3(arguments));
    set1 = take_set(arg_1(arguments));
    number = take_int(arg_3(arguments));
    position = number >> bitset_shift;
    if (position >= set1->min_position && position <= set1->max_position) {
      bitset_index = (memsizetype) (position - set1->min_position);
      bit_index = ((unsigned int) number) & bitset_mask;
      if (set1->bitset[bitset_index] & (1 << bit_index)) {
        return(SYS_TRUE_OBJECT);
      } else {
        return(SYS_FALSE_OBJECT);
      } /* if */
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* set_idx */



#ifdef ANSI_C

objecttype set_incl (listtype arguments)
#else

objecttype set_incl (arguments)
listtype arguments;
#endif

  {
    objecttype set_to;
    settype set_dest;
    inttype number;
    inttype position;
    memsizetype old_size;
    memsizetype new_size;
    settype old_set;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* set_incl */
    set_to = arg_1(arguments);
    isit_set(set_to);
    is_variable(set_to);
    set_dest = take_set(set_to);
    isit_int(arg_2(arguments));
    number = take_int(arg_2(arguments));
    position = number >> bitset_shift;
    if (position > set_dest->max_position) {
      old_size = set_dest->max_position - set_dest->min_position + 1;
      new_size = position - set_dest->min_position + 1;
      if (!RESIZE_SET(set_dest, old_size, new_size)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        COUNT3_SET(old_size, new_size);
        set_to->value.setvalue = set_dest;
        set_dest->max_position = position;
        memset(&set_dest->bitset[old_size], 0, (new_size - old_size) * sizeof(bitsettype));
      } /* if */
    } else if (position < set_dest->min_position) {
      old_size = set_dest->max_position - set_dest->min_position + 1;
      new_size = set_dest->max_position - position + 1;
      old_set = set_dest;
      if (!ALLOC_SET(set_dest, new_size)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        COUNT_SET(new_size);
        set_to->value.setvalue = set_dest;
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
    return(SYS_EMPTY_OBJECT);
  } /* set_incl */



#ifdef ANSI_C

objecttype set_intersect (listtype arguments)
#else

objecttype set_intersect (arguments)
listtype arguments;
#endif

  {
    settype set1;
    settype set2;
    inttype position;
    inttype min_position;
    inttype max_position;
    settype result;

  /* set_intersect */
#ifdef OUT_OF_ORDER
    printf("set_intersect(\n");
    trace1(arg_1(arguments));
    printf(",\n");
    trace1(arg_3(arguments));
    printf(")\n");
#endif
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    set1 = take_set(arg_1(arguments));
    set2 = take_set(arg_3(arguments));
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
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        COUNT_SET(1);
        result->min_position = 0;
        result->max_position = 0;
        memset(result->bitset, 0, sizeof(bitsettype));
        return(bld_set_temp(result));
      } /* if */
    } else {
      if (!ALLOC_SET(result, max_position - min_position + 1)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        COUNT_SET(max_position - min_position + 1);
        result->min_position = min_position;
        result->max_position = max_position;
        for (position = min_position; position <= max_position; position++) {
          result->bitset[position - min_position] =
              set1->bitset[position - set1->min_position] &
              set2->bitset[position - set2->min_position];
        } /* for */
        return(bld_set_temp(result));
      } /* if */
    } /* if */
  } /* set_intersect */



#ifdef ANSI_C

objecttype set_le (listtype arguments)
#else

objecttype set_le (arguments)
listtype arguments;
#endif

  {
    settype set1;
    settype set2;
    memsizetype min1_index;
    memsizetype min2_index;
    memsizetype bitset_index;
    memsizetype size;
    bitsettype *bitset1;
    bitsettype *bitset2;
    booltype less_equal;

  /* set_le */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    set1 = take_set(arg_1(arguments));
    set2 = take_set(arg_3(arguments));
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
    if (less_equal) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* set_le */



#ifdef ANSI_C

objecttype set_lt (listtype arguments)
#else

objecttype set_lt (arguments)
listtype arguments;
#endif

  {
    settype set1;
    settype set2;
    memsizetype min1_index;
    memsizetype min2_index;
    memsizetype bitset_index;
    memsizetype size;
    bitsettype *bitset1;
    bitsettype *bitset2;
    booltype less_equal;
    booltype equal;

  /* set_lt */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    set1 = take_set(arg_1(arguments));
    set2 = take_set(arg_3(arguments));
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
    if (less_equal && !equal) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* set_lt */



#ifdef ANSI_C

objecttype set_max (listtype arguments)
#else

objecttype set_max (arguments)
listtype arguments;
#endif

  {
    settype set1;
    memsizetype bitset_index;
    bitsettype curr_bitset;
    inttype result;

  /* set_max */
    isit_set(arg_1(arguments));
    set1 = take_set(arg_1(arguments));
    bitset_index = set1->max_position - set1->min_position + 1;
    while (bitset_index > 0) {
      bitset_index--;
      curr_bitset = set1->bitset[bitset_index];
      if (curr_bitset) {
        result = most_significant_bit(curr_bitset);
        result += (set1->min_position + bitset_index) << bitset_shift;
        return(bld_int_temp(result));
      } /* if */
    } /* while */
    return(raise_exception(SYS_RNG_EXCEPTION));
  } /* set_max */



#ifdef ANSI_C

objecttype set_min (listtype arguments)
#else

objecttype set_min (arguments)
listtype arguments;
#endif

  {
    settype set1;
    memsizetype bitset_size;
    memsizetype bitset_index;
    bitsettype curr_bitset;
    inttype result;

  /* set_min */
    isit_set(arg_1(arguments));
    set1 = take_set(arg_1(arguments));
    bitset_size = set1->max_position - set1->min_position + 1;
    bitset_index = 0;
    while (bitset_index < bitset_size) {
      curr_bitset = set1->bitset[bitset_index];
      if (curr_bitset) {
        result = least_significant_bit(curr_bitset);
        result += (set1->min_position + bitset_index) << bitset_shift;
        return(bld_int_temp(result));
      } /* if */
      bitset_index++;
    } /* while */
    return(raise_exception(SYS_RNG_EXCEPTION));
  } /* set_min */



#ifdef ANSI_C

objecttype set_ne (listtype arguments)
#else

objecttype set_ne (arguments)
listtype arguments;
#endif

  { /* set_ne */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    if (setNe(take_set(arg_1(arguments)), take_set(arg_3(arguments)))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* set_ne */



#ifdef ANSI_C

objecttype set_not_elem (listtype arguments)
#else

objecttype set_not_elem (arguments)
listtype arguments;
#endif

  {
    settype set1;
    inttype number;
    inttype position;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* set_not_elem */
    isit_int(arg_1(arguments));
    isit_set(arg_4(arguments));
    number = take_int(arg_1(arguments));
    set1 = take_set(arg_4(arguments));
    position = number >> bitset_shift;
    if (position >= set1->min_position && position <= set1->max_position) {
      bitset_index = (memsizetype) (position - set1->min_position);
      bit_index = ((unsigned int) number) & bitset_mask;
      if (set1->bitset[bitset_index] & (1 << bit_index)) {
        return(SYS_FALSE_OBJECT);
      } else {
        return(SYS_TRUE_OBJECT);
      } /* if */
    } else {
      return(SYS_TRUE_OBJECT);
    } /* if */
  } /* set_not_elem */



#ifdef ANSI_C

objecttype set_symdiff (listtype arguments)
#else

objecttype set_symdiff (arguments)
listtype arguments;
#endif

  {
    settype set1;
    settype set2;
    inttype position;
    inttype min_position;
    inttype max_position;
    inttype start_position;
    inttype stop_position;
    settype result;

  /* set_symdiff */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    set1 = take_set(arg_1(arguments));
    set2 = take_set(arg_3(arguments));
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
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      COUNT_SET(max_position - min_position + 1);
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
      return(bld_set_temp(result));
    } /* if */
  } /* set_symdiff */



#ifdef ANSI_C

objecttype set_union (listtype arguments)
#else

objecttype set_union (arguments)
listtype arguments;
#endif

  {
    settype set1;
    settype set2;
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

  /* set_union */
#ifdef OUT_OF_ORDER
    printf("set_union(\n");
    trace1(arg_1(arguments));
    printf(",\n");
    trace1(arg_3(arguments));
    printf(")\n");
#endif
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    set1 = take_set(arg_1(arguments));
    set2 = take_set(arg_3(arguments));
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
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      COUNT_SET(max_position - min_position + 1);
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
      printf("set_union ==> ");
      prot_set(result);
      printf("\n");
#endif
      return(bld_set_temp(result));
    } /* if */
  } /* set_union */



#ifdef ANSI_C

objecttype set_value (listtype arguments)
#else

objecttype set_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;
    settype set1;
    memsizetype set_size;
    settype result;

  /* set_value */
    isit_reference(arg_3(arguments));
    obj_arg = take_reference(arg_3(arguments));
    isit_set(obj_arg);
    set1 = take_set(obj_arg);

    set_size = set1->max_position - set1->min_position + 1;
    if (!ALLOC_SET(result, set_size)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      COUNT_SET(set_size);
      result->min_position = set1->min_position;
      result->max_position = set1->max_position;
      memcpy(result->bitset, set1->bitset, set_size * sizeof(bitsettype));
      return(bld_set_temp(result));
    } /* if */
  } /* set_value */
