/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2019  Thomas Mertes                        */
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
/*  Changes: 2004, 2013, 2016, 2018  Thomas Mertes                  */
/*  Content: All primitive actions for the set type.                */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "objutl.h"
#include "runerr.h"
#include "int_rtl.h"
#include "set_rtl.h"

#undef EXTERN
#define EXTERN
#include "setlib.h"



objectType set_arrlit (listType arguments)

  {
    arrayType arr1;
    memSizeType length;
    memSizeType array_index;
    intType number;
    intType position;
    unsigned int bit_index;
    setType result;

  /* set_arrlit */
    isit_array(arg_2(arguments));
    arr1 = take_array(arg_2(arguments));
    length = arraySize(arr1);
    if (!ALLOC_SET(result, 1)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      if (length == 0) {
        result->min_position = 0;
        result->max_position = 0;
        memset(result->bitset, 0, sizeof(bitSetType));
      } else {
        number = take_int(&arr1->arr[0]);
        position = bitset_pos(number);
        result->min_position = position;
        result->max_position = position;
        bit_index = ((unsigned int) number) & bitset_mask;
        result->bitset[0] = (((bitSetType) 1) << bit_index);
        for (array_index = 1; array_index < length; array_index++) {
          setIncl(&result, take_int(&arr1->arr[array_index]));
          if (fail_flag) {
            FREE_SET(result, bitsetSize(result));
            return fail_value;
          } /* if */
        } /* for */
      } /* if */
      return bld_set_temp(result);
    } /* if */
  } /* set_arrlit */



objectType set_baselit (listType arguments)

  {
    intType number;
    intType position;
    unsigned int bit_index;
    setType result;

  /* set_baselit */
    isit_int(arg_2(arguments));
    number = take_int(arg_2(arguments));
    if (!ALLOC_SET(result, 1)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      position = bitset_pos(number);
      result->min_position = position;
      result->max_position = position;
      bit_index = ((unsigned int) number) & bitset_mask;
      result->bitset[0] = (((bitSetType) 1) << bit_index);
      return bld_set_temp(result);
    } /* if */
  } /* set_baselit */



/**
 *  Compute the cardinality of a set.
 *  @return the number of elements in 'aSet'.
 *  @exception RANGE_ERROR Result does not fit into an integer.
 */
objectType set_card (listType arguments)

  { /* set_card */
    isit_set(arg_1(arguments));
    return bld_int_temp(
        setCard(take_set(arg_1(arguments))));
  } /* set_card */



/**
 *  Compares two sets to make them useable as key in a hash table.
 *  The sets are compared by determining the biggest element that is
 *  not present or absent in both sets. The set in which this element
 *  is not present is the smaller one. Note that the set comparison
 *  is not related to the concepts of subset or superset. With the
 *  comparison function setCmp it is possible to sort an array of
 *  sets or to use sets as key in a hash table. The functions
 *  setIsSubset and setIsProperSubset are used to check if a set is
 *  a (proper) subset or superset of another set.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objectType set_cmp (listType arguments)

  { /* set_cmp */
    isit_set(arg_1(arguments));
    isit_set(arg_2(arguments));
    return bld_int_temp(
        setCmp(take_set(arg_1(arguments)), take_set(arg_2(arguments))));
  } /* set_cmp */



objectType set_conv1 (listType arguments)

  {
    objectType set_arg;
    setType result;

  /* set_conv1 */
    set_arg = arg_1(arguments);
    isit_set(set_arg);
    if (TEMP_OBJECT(set_arg)) {
      result = take_set(set_arg);
      set_arg->value.setValue = NULL;
      return bld_set_temp(result);
    } else {
      return set_arg;
    } /* if */
  } /* set_conv1 */



objectType set_conv3 (listType arguments)

  {
    objectType set_arg;
    setType result;

  /* set_conv3 */
    set_arg = arg_3(arguments);
    isit_set(set_arg);
    if (TEMP_OBJECT(set_arg)) {
      result = take_set(set_arg);
      set_arg->value.setValue = NULL;
      return bld_set_temp(result);
    } else {
      return set_arg;
    } /* if */
  } /* set_conv3 */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType set_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    setType set_dest;
    setType set_source;
    memSizeType set_dest_size;
    memSizeType set_source_size;

  /* set_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_set(dest);
    isit_set(source);
    is_variable(dest);
    set_dest = take_set(dest);
    set_source = take_set(source);
    if (TEMP_OBJECT(source)) {
      set_dest_size = bitsetSize(set_dest);
      FREE_SET(set_dest, set_dest_size);
      dest->value.setValue = set_source;
      source->value.setValue = NULL;
    } else {
      set_source_size = bitsetSize(set_source);
      if (set_dest->min_position != set_source->min_position ||
          set_dest->max_position != set_source->max_position) {
        set_dest_size = bitsetSize(set_dest);
        if (set_dest_size != set_source_size) {
          if (!ALLOC_SET(set_dest, set_source_size)) {
            return raise_exception(SYS_MEM_EXCEPTION);
          } else {
            FREE_SET(dest->value.setValue, set_dest_size);
            dest->value.setValue = set_dest;
          } /* if */
        } /* if */
        set_dest->min_position = set_source->min_position;
        set_dest->max_position = set_source->max_position;
      } /* if */
      /* It is possible that dest == source holds. The     */
      /* behavior of memcpy() is undefined if source and   */
      /* destination areas overlap (or are identical).     */
      /* Therefore memmove() is used instead of memcpy().  */
      memmove(set_dest->bitset, set_source->bitset,
              (size_t) set_source_size * sizeof(bitSetType));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* set_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType set_create (listType arguments)

  {
    objectType dest;
    objectType source;
    setType set_source;
    memSizeType new_size;
    setType new_set;

  /* set_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_set(source);
    set_source = take_set(source);
    SET_CATEGORY_OF_OBJ(dest, SETOBJECT);
    if (TEMP_OBJECT(source)) {
      dest->value.setValue = set_source;
      source->value.setValue = NULL;
    } else {
      new_size = bitsetSize(set_source);
      if (!ALLOC_SET(new_set, new_size)) {
        dest->value.setValue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        dest->value.setValue = new_set;
        new_set->min_position = set_source->min_position;
        new_set->max_position = set_source->max_position;
        memcpy(new_set->bitset, set_source->bitset,
               (size_t) new_size * sizeof(bitSetType));
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* set_create */



/**
 *  Free the memory referred by 'old_set/arg_1'.
 *  After set_destr is left 'old_set/arg_1' is NULL.
 *  The memory where 'old_set/arg_1' is stored can be
 *  freed afterwards.
 */
objectType set_destr (listType arguments)

  {
    setType old_set;

  /* set_destr */
    isit_set(arg_1(arguments));
    old_set = take_set(arg_1(arguments));
    if (old_set != NULL) {
      FREE_SET(old_set, bitsetSize(old_set));
      arg_1(arguments)->value.setValue = NULL;
    } /* if */
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* set_destr */



/**
 *  Difference of two sets.
 *  @return the difference of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
objectType set_diff (listType arguments)

  { /* set_diff */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    return bld_set_temp(
        setDiff(take_set(arg_1(arguments)), take_set(arg_3(arguments))));
  } /* set_diff */



/**
 *  Set membership test.
 *  Determine if 'number' is a member of the set 'aSet'.
 *  @return TRUE if 'number' is a member of  'aSet',
 *          FALSE otherwise.
 */
objectType set_elem (listType arguments)

  {
    setType aSet;
    intType number;
    intType position;
    memSizeType bitset_index;
    unsigned int bit_index;

  /* set_elem */
    isit_int(arg_1(arguments));
    isit_set(arg_3(arguments));
    number = take_int(arg_1(arguments));
    aSet = take_set(arg_3(arguments));
    position = bitset_pos(number);
    if (position >= aSet->min_position && position <= aSet->max_position) {
      bitset_index = bitsetIndex(aSet, position);
      bit_index = ((unsigned int) number) & bitset_mask;
      if (aSet->bitset[bitset_index] & (((bitSetType) 1) << bit_index)) {
        return SYS_TRUE_OBJECT;
      } else {
        return SYS_FALSE_OBJECT;
      } /* if */
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* set_elem */



/**
 *  Create an empty set.
 *  @return an empty set.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
objectType set_empty (listType arguments)

  {
    setType result;

  /* set_empty */
    if (!ALLOC_SET(result, 1)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->min_position = 0;
      result->max_position = 0;
      memset(result->bitset, 0, sizeof(bitSetType));
      return bld_set_temp(result);
    } /* if */
  } /* set_empty */



/**
 *  Check if two sets are equal.
 *  @return TRUE if the two sets are equal,
 *          FALSE otherwise.
 */
objectType set_eq (listType arguments)

  { /* set_eq */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    if (setEq(take_set(arg_1(arguments)), take_set(arg_3(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* set_eq */



/**
 *  Remove 'number' from the set 'set_to'.
 *  If 'number' is not element of 'set_to' then 'set_to' stays unchanged.
 */
objectType set_excl (listType arguments)

  {
    objectType set_to;
    setType set_dest;
    intType number;
    intType position;
    memSizeType bitset_index;
    unsigned int bit_index;

  /* set_excl */
    set_to = arg_1(arguments);
    isit_set(set_to);
    is_variable(set_to);
    set_dest = take_set(set_to);
    isit_int(arg_2(arguments));
    number = take_int(arg_2(arguments));
    position = bitset_pos(number);
    if (position >= set_dest->min_position && position <= set_dest->max_position) {
      bitset_index = bitsetIndex(set_dest, position);
      bit_index = ((unsigned int) number) & bitset_mask;
      set_dest->bitset[bitset_index] &= ~(((bitSetType) 1) << bit_index);
#ifdef OUT_OF_ORDER
      if (set_dest->bitset[bitset_index] == 0) {
        if
      } /* if */
#endif
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* set_excl */



/**
 *  Determine if 'set1' is a superset of 'set2'.
 *  'set1' is a superset of 'set2' if no element X exists for which
 *   X in set2 and X not in set1
 *  holds.
 *  @return TRUE if 'set1' is a superset of 'set2',
 *          FALSE otherwise.
 */
objectType set_ge (listType arguments)

  { /* set_ge */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    if (setIsSubset(take_set(arg_3(arguments)), take_set(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* set_ge */



/**
 *  Determine if 'set1' is a proper superset of 'set2'.
 *  'set1' is a proper superset of 'set2' if
 *   set1 >= set2 and set1 <> set2
 *  holds.
 *  @return TRUE if 'set1' is a proper superset of 'set2',
 *          FALSE otherwise.
 */
objectType set_gt (listType arguments)

  { /* set_gt */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    if (setIsProperSubset(take_set(arg_3(arguments)), take_set(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* set_gt */



/**
 *  Compute the hash value of a set.
 *  @return the hash value.
 */
objectType set_hashcode (listType arguments)

  { /* set_hashcode */
    isit_set(arg_1(arguments));
    return bld_int_temp(
        setHashCode(take_set(arg_1(arguments))));
  } /* set_hashcode */



/**
 *  Convert an integer number to a bitset.
 *  @return a bitset which corresponds to the given integer.
 *  @exception RANGE_ERROR Number is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType set_iconv1 (listType arguments)

  { /* set_iconv1 */
    isit_int(arg_1(arguments));
    return bld_set_temp(
        setIConv(take_int(arg_1(arguments))));
  } /* set_iconv1 */



/**
 *  Convert an integer number to a bitset.
 *  @return a bitset which corresponds to the given integer.
 *  @exception RANGE_ERROR Number is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType set_iconv3 (listType arguments)

  { /* set_iconv3 */
    isit_int(arg_3(arguments));
    return bld_set_temp(
        setIConv(take_int(arg_3(arguments))));
  } /* set_iconv3 */



objectType set_idx (listType arguments)

  {
    setType aSet;
    intType number;
    intType position;
    memSizeType bitset_index;
    unsigned int bit_index;

  /* set_idx */
    isit_set(arg_1(arguments));
    isit_int(arg_3(arguments));
    aSet = take_set(arg_1(arguments));
    number = take_int(arg_3(arguments));
    position = bitset_pos(number);
    if (position >= aSet->min_position && position <= aSet->max_position) {
      bitset_index = bitsetIndex(aSet, position);
      bit_index = ((unsigned int) number) & bitset_mask;
      if (aSet->bitset[bitset_index] & (((bitSetType) 1) << bit_index)) {
        return SYS_TRUE_OBJECT;
      } else {
        return SYS_FALSE_OBJECT;
      } /* if */
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* set_idx */



/**
 *  Add 'number' to the set 'set_to'.
 *  If 'number' is already in 'set_to' then 'set_to' stays unchanged.
 *  @exception MEMORY_ERROR If there is not enough memory.
 */
objectType set_incl (listType arguments)

  {
    objectType set_to;
    setType set_dest;
    intType number;
    intType position;
    memSizeType old_size;
    memSizeType new_size;
    setType old_set;
    memSizeType bitset_index;
    unsigned int bit_index;

  /* set_incl */
    set_to = arg_1(arguments);
    isit_set(set_to);
    is_variable(set_to);
    set_dest = take_set(set_to);
    isit_int(arg_2(arguments));
    number = take_int(arg_2(arguments));
    position = bitset_pos(number);
    if (position > set_dest->max_position) {
      old_size = bitsetSize(set_dest);
      if (unlikely((uintType) (position - set_dest->min_position + 1) > MAX_SET_LEN)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_size = bitsetSize2(set_dest->min_position, position);
        set_dest = REALLOC_SET(set_dest, old_size, new_size);
        if (set_dest == NULL) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          COUNT3_SET(old_size, new_size);
          set_to->value.setValue = set_dest;
          set_dest->max_position = position;
          memset(&set_dest->bitset[old_size], 0,
                 (new_size - old_size) * sizeof(bitSetType));
        } /* if */
      } /* if */
    } else if (position < set_dest->min_position) {
      old_size = bitsetSize(set_dest);
      if (unlikely((uintType) (set_dest->max_position - position + 1) > MAX_SET_LEN)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_size = bitsetSize2(position, set_dest->max_position);
        old_set = set_dest;
        if (!ALLOC_SET(set_dest, new_size)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          set_to->value.setValue = set_dest;
          set_dest->min_position = position;
          set_dest->max_position = old_set->max_position;
          memset(set_dest->bitset, 0, (new_size - old_size) * sizeof(bitSetType));
          memcpy(&set_dest->bitset[new_size - old_size], old_set->bitset,
                 old_size * sizeof(bitSetType));
          FREE_SET(old_set, old_size);
        } /* if */
      } /* if */
    } /* if */
    bitset_index = bitsetIndex(set_dest, position);
    bit_index = ((unsigned int) number) & bitset_mask;
    set_dest->bitset[bitset_index] |= (((bitSetType) 1) << bit_index);
    return SYS_EMPTY_OBJECT;
  } /* set_incl */



/**
 *  Intersection of two sets.
 *  @return the intersection of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
objectType set_intersect (listType arguments)

  { /* set_intersect */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    return bld_set_temp(
        setIntersect(take_set(arg_1(arguments)), take_set(arg_3(arguments))));
  } /* set_intersect */



/**
 *  Assign the intersection of delta/arg_3 and dest/arg_1 to dest/arg_1.
 *  @exception MEMORY_ERROR Not enough memory to create dest/arg_1.
 */
objectType set_intersect_assign (listType arguments)

  {
    objectType set_variable;

  /* set_intersect_assign */
    set_variable = arg_1(arguments);
    isit_set(set_variable);
    is_variable(set_variable);
    isit_set(arg_3(arguments));
    setIntersectAssign(&set_variable->value.setValue, take_set(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* set_intersect_assign */



/**
 *  Determine if 'set1' is a subset of 'set2'.
 *  'set1' is a subset of 'set2' if no element X exists for which
 *   X in set1 and X not in set2
 *  holds.
 *  @return TRUE if 'set1' is a subset of 'set2',
 *          FALSE otherwise.
 */
objectType set_le (listType arguments)

  { /* set_le */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    if (setIsSubset(take_set(arg_1(arguments)), take_set(arg_3(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* set_le */



/**
 *  Determine if 'set1' is a proper subset of 'set2'.
 *  'set1' is a proper subset of 'set2' if
 *   set1 <= set2 and set1 <> set2
 *  holds.
 *  @return TRUE if 'set1' is a proper subset of 'set2',
 *          FALSE otherwise.
 */
objectType set_lt (listType arguments)

  { /* set_lt */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    if (setIsProperSubset(take_set(arg_1(arguments)), take_set(arg_3(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* set_lt */



/**
 *  Maximum element of a set.
 *  Delivers the element from 'aSet' for which the following condition holds:
 *   element >= X
 *  for all X which are in the set.
 *  @return the maximal element of 'aSet'.
 *  @exception RANGE_ERROR If 'aSet' is the empty set.
 */
objectType set_max (listType arguments)

  { /* set_max */
    isit_set(arg_1(arguments));
    return bld_int_temp(
        setMax(take_set(arg_1(arguments))));
  } /* set_max */



/**
 *  Minimum element of a set.
 *  Delivers the element from 'aSet' for which the following condition holds:
 *   element <= X
 *  for all X which are in the set.
 *  @return the minimum element of 'aSet'.
 *  @exception RANGE_ERROR If 'aSet' is the empty set.
 */
objectType set_min (listType arguments)

  { /* set_min */
    isit_set(arg_1(arguments));
    return bld_int_temp(
        setMin(take_set(arg_1(arguments))));
  } /* set_min */



/**
 *  Check if two sets are not equal.
 *  @return FALSE if the two sets are equal,
 *          TRUE otherwise.
 */
objectType set_ne (listType arguments)

  { /* set_ne */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    if (setEq(take_set(arg_1(arguments)), take_set(arg_3(arguments)))) {
      return SYS_FALSE_OBJECT;
    } else {
      return SYS_TRUE_OBJECT;
    } /* if */
  } /* set_ne */



/**
 *  Minimum element of aSet/arg_1 that is larger than number/arg_2.
 *  @return the minimum element of aSet/arg_1 that is larger than number/arg_2.
 *  @exception RANGE_ERROR If aSet/arg_1 has no element larger than number/arg_2.
 */
objectType set_next (listType arguments)

  { /* set_next */
    isit_set(arg_1(arguments));
    isit_int(arg_2(arguments));
    return bld_int_temp(
        setNext(take_set(arg_1(arguments)), take_int(arg_2(arguments))));
  } /* set_next */



/**
 *  Negated set membership test.
 *  Determine if 'number' is not a member of the set 'aSet'.
 *  @return FALSE if 'number' is a member of  'aSet',
 *          TRUE otherwise.
 */
objectType set_not_elem (listType arguments)

  {
    setType aSet;
    intType number;
    intType position;
    memSizeType bitset_index;
    unsigned int bit_index;

  /* set_not_elem */
    isit_int(arg_1(arguments));
    isit_set(arg_4(arguments));
    number = take_int(arg_1(arguments));
    aSet = take_set(arg_4(arguments));
    position = bitset_pos(number);
    if (position >= aSet->min_position && position <= aSet->max_position) {
      bitset_index = bitsetIndex(aSet, position);
      bit_index = ((unsigned int) number) & bitset_mask;
      if (aSet->bitset[bitset_index] & (((bitSetType) 1) << bit_index)) {
        return SYS_FALSE_OBJECT;
      } else {
        return SYS_TRUE_OBJECT;
      } /* if */
    } else {
      return SYS_TRUE_OBJECT;
    } /* if */
  } /* set_not_elem */



/**
 *  Compute pseudo-random number which is element of 'aSet'.
 *  The random values are uniform distributed.
 *  @return a random number such that rand(aSet) in aSet holds.
 *  @exception RANGE_ERROR If 'aSet' is empty.
 */
objectType set_rand (listType arguments)

  { /* set_rand */
    isit_set(arg_1(arguments));
    return bld_int_temp(
        setRand(take_set(arg_1(arguments))));
  } /* set_rand */



/**
 *  Create set with all values from lowValue/arg_2 to highValue/arg_4 inclusive.
 *  @param lowValue/arg_2 lowest value to be added to the result set.
 *  @param highValue/arg_4 highest value to be added to the result set.
 *  @return set with all values from lowValue/arg_2 to highValue/arg_4 inclusive, or
 *          an empty set if lowValue/arg_2 is greater than highValue/arg_4.
 */
objectType set_rangelit (listType arguments)

  { /* set_rangelit */
    isit_int(arg_2(arguments));
    isit_int(arg_4(arguments));
    return bld_set_temp(
        setRangelit(take_int(arg_2(arguments)), take_int(arg_4(arguments))));
  } /* set_rangelit */



/**
 *  Convert a bitset to integer.
 *  @return an integer which corresponds to the given bitset.
 *  @exception RANGE_ERROR If 'aSet' contains negative values or
 *             if it does not fit into a non-negative integer.
 */
objectType set_sconv1 (listType arguments)

  { /* set_sconv1 */
    isit_set(arg_1(arguments));
    return bld_int_temp(
        setSConv(take_set(arg_1(arguments))));
  } /* set_sconv1 */



/**
 *  Convert a bitset to integer.
 *  @return an integer which corresponds to the given bitset.
 *  @exception RANGE_ERROR If 'aSet' contains negative values or
 *             if it does not fit into a non-negative integer.
 */
objectType set_sconv3 (listType arguments)

  { /* set_sconv3 */
    isit_set(arg_3(arguments));
    return bld_int_temp(
        setSConv(take_set(arg_3(arguments))));
  } /* set_sconv3 */



/**
 *  Symmetric difference of two sets.
 *  @return the symmetric difference of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
objectType set_symdiff (listType arguments)

  { /* set_symdiff */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    return bld_set_temp(
        setSymdiff(take_set(arg_1(arguments)), take_set(arg_3(arguments))));
  } /* set_symdiff */



/**
 *  Union of two sets.
 *  @return the union of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
objectType set_union (listType arguments)

  { /* set_union */
    isit_set(arg_1(arguments));
    isit_set(arg_3(arguments));
    return bld_set_temp(
        setUnion(take_set(arg_1(arguments)), take_set(arg_3(arguments))));
  } /* set_union */



/**
 *  Assign the union of delta/arg_3 and dest/arg_1 to dest/arg_1.
 *  @exception MEMORY_ERROR Not enough memory to create dest/arg_1.
 */
objectType set_union_assign (listType arguments)

  {
    objectType set_variable;

  /* set_union_assign */
    set_variable = arg_1(arguments);
    isit_set(set_variable);
    is_variable(set_variable);
    isit_set(arg_3(arguments));
    setUnionAssign(&set_variable->value.setValue, take_set(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* set_union_assign */



objectType set_value (listType arguments)

  {
    objectType obj_arg;
    setType aSet;
    memSizeType set_size;
    setType result;

  /* set_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (unlikely(obj_arg == NULL ||
                 CATEGORY_OF_OBJ(obj_arg) != SETOBJECT ||
                 take_set(obj_arg) == NULL)) {
      logError(printf("set_value(");
               trace1(obj_arg);
               printf("): Category is not SETOBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      aSet = take_set(obj_arg);
      set_size = bitsetSize(aSet);
      if (!ALLOC_SET(result, set_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->min_position = aSet->min_position;
        result->max_position = aSet->max_position;
        memcpy(result->bitset, aSet->bitset, set_size * sizeof(bitSetType));
        return bld_set_temp(result);
      } /* if */
    } /* if */
  } /* set_value */
