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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/setlib.c                                        */
/*  Changes: 2004  Thomas Mertes                                    */
/*  Content: All primitive actions for the set type.                */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

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

objecttype set_arrlit (listtype arguments)
#else

objecttype set_arrlit (arguments)
listtype arguments;
#endif

  {
    arraytype arr1;
    memsizetype length;
    inttype number;
    inttype position;
    unsigned int bit_index;
    settype result;

  /* set_arrlit */
    isit_array(arg_2(arguments));
    arr1 = take_array(arg_2(arguments));
    length = arr1->max_position - arr1->min_position + 1;
    if (!ALLOC_SET(result, 1)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      if (length == 0) {
        result->min_position = 0;
        result->max_position = 0;
        memset(result->bitset, 0, sizeof(bitsettype));
      } else {
        number = take_int(&arr1->arr[0]);
        position = number >> bitset_shift;
        result->min_position = position;
        result->max_position = position;
        bit_index = ((unsigned int) number) & bitset_mask;
        result->bitset[0] = (1 << bit_index);
        for (number = 1; number < length; number++) {
          setIncl(&result, take_int(&arr1->arr[number]));
          if (fail_flag) {
            FREE_SET(result, result->max_position - result->min_position + 1);
            return(fail_value);
          } /* if */
        } /* for */
      } /* if */
      return(bld_set_temp(result));
    } /* if */
  } /* set_arrlit */



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

objecttype set_cmp (listtype arguments)
#else

objecttype set_cmp (arguments)
listtype arguments;
#endif

  { /* set_cmp */
    isit_set(arg_1(arguments));
    isit_set(arg_2(arguments));
    return(bld_int_temp(
        setCmp(take_set(arg_1(arguments)), take_set(arg_2(arguments)))));
  } /* set_cmp */



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
    SET_CATEGORY_OF_OBJ(set_to, SETOBJECT);
    if (TEMP_OBJECT(set_from)) {
      set_to->value.setvalue = set_source;
      set_from->value.setvalue = NULL;
    } else {
      new_size = set_source->max_position - set_source->min_position + 1;
      if (!ALLOC_SET(new_set, new_size)) {
        set_to->value.setvalue = NULL;
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
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

  { /* set_diff */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    return(bld_set_temp(
        setDiff(take_set(arg_1(arguments)), take_set(arg_3(arguments)))));
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

  { /* set_ge */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    if (setGe(take_set(arg_1(arguments)), take_set(arg_3(arguments)))) {
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

  { /* set_gt */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    if (setGt(take_set(arg_1(arguments)), take_set(arg_3(arguments)))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* set_gt */



#ifdef ANSI_C

objecttype set_hashcode (listtype arguments)
#else

objecttype set_hashcode (arguments)
listtype arguments;
#endif

  { /* set_hashcode */
    isit_set(arg_1(arguments));
    return(bld_int_temp(
        setHashCode(take_set(arg_1(arguments)))));
  } /* set_hashcode */



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
      set_dest = REALLOC_SET(set_dest, old_size, new_size);
      if (set_dest == NULL) {
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

  { /* set_intersect */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    return(bld_set_temp(
        setIntersect(take_set(arg_1(arguments)), take_set(arg_3(arguments)))));
  } /* set_intersect */



#ifdef ANSI_C

objecttype set_le (listtype arguments)
#else

objecttype set_le (arguments)
listtype arguments;
#endif

  { /* set_le */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    if (setLe(take_set(arg_1(arguments)), take_set(arg_3(arguments)))) {
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

  { /* set_lt */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    if (setLt(take_set(arg_1(arguments)), take_set(arg_3(arguments)))) {
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

  { /* set_max */
    isit_set(arg_1(arguments));
    return(bld_int_temp(
        setMax(take_set(arg_1(arguments)))));
  } /* set_max */



#ifdef ANSI_C

objecttype set_min (listtype arguments)
#else

objecttype set_min (arguments)
listtype arguments;
#endif

  { /* set_min */
    isit_set(arg_1(arguments));
    return(bld_int_temp(
        setMin(take_set(arg_1(arguments)))));
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

objecttype set_rand (listtype arguments)
#else

objecttype set_rand (arguments)
listtype arguments;
#endif

  { /* set_rand */
    isit_set(arg_1(arguments));
    return(bld_int_temp(
        setRand(take_set(arg_1(arguments)))));
  } /* set_rand */



#ifdef ANSI_C

objecttype set_symdiff (listtype arguments)
#else

objecttype set_symdiff (arguments)
listtype arguments;
#endif

  { /* set_symdiff */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    return(bld_set_temp(
        setSymdiff(take_set(arg_1(arguments)), take_set(arg_3(arguments)))));
  } /* set_symdiff */



#ifdef ANSI_C

objecttype set_union (listtype arguments)
#else

objecttype set_union (arguments)
listtype arguments;
#endif

  { /* set_union */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    return(bld_set_temp(
        setUnion(take_set(arg_1(arguments)), take_set(arg_3(arguments)))));
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
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != SETOBJECT ||
        take_set(obj_arg) == NULL) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } else {
      set1 = take_set(obj_arg);
      set_size = set1->max_position - set1->min_position + 1;
      if (!ALLOC_SET(result, set_size)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        result->min_position = set1->min_position;
        result->max_position = set1->max_position;
        memcpy(result->bitset, set1->bitset, set_size * sizeof(bitsettype));
        return(bld_set_temp(result));
      } /* if */
    } /* if */
  } /* set_value */
