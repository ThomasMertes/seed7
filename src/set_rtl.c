/********************************************************************/
/*                                                                  */
/*  set_rtl.c     Primitive actions for the set type.               */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
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
/*  Changes: 2004, 2005, 2010, 2012, 2013  Thomas Mertes            */
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



settype setArrlit (const_rtlArraytype arr1)

  {
    memsizetype length;
    inttype number;
    inttype position;
    unsigned int bit_index;
    memsizetype array_index;
    settype result;

  /* setArrlit */
    length = arraySize(arr1);
    if (unlikely(!ALLOC_SET(result, 1))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      if (length == 0) {
        result->min_position = 0;
        result->max_position = 0;
        result->bitset[0] = (bitsettype) 0;
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
            FREE_SET(result, bitsetSize(result));
            return fail_value;
          } /* if */
#endif
        } /* for */
      } /* if */
      return result;
    } /* if */
  } /* setArrlit */



settype setBaselit (const inttype number)

  {
    inttype position;
    unsigned int bit_index;
    settype result;

  /* setBaselit */
    if (unlikely(!ALLOC_SET(result, 1))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      position = bitset_pos(number);
      result->min_position = position;
      result->max_position = position;
      bit_index = ((unsigned int) number) & bitset_mask;
      result->bitset[0] = (bitsettype) 1 << bit_index;
      return result;
    } /* if */
  } /* setBaselit */



/**
 *  Compute the cardinality of a set.
 *  @return the number of elements in 'aSet'.
 */
inttype setCard (const const_settype aSet)

  {
    const unsigned char *byte;
    memsizetype byteCount;
    uinttype card = 0;
    inttype result;

  /* setCard */
    byte = (const unsigned char *) aSet->bitset;
    for (byteCount = sizeof(bitsettype) * bitsetSize(aSet);
        byteCount != 0; byteCount--, byte++) {
      card += (uinttype) card_byte[(int) *byte];
    } /* for */
    if (card > INTTYPE_MAX) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = (inttype) card;
    } /* if */
    /* printf(" = %d\n", result); */
    return result;
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
inttype setCmp (const const_settype set1, const const_settype set2)

  {
    memsizetype index_byond;
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
        bitset_index = bitsetIndex(set1, set2->max_position + 1);
        if (set1->min_position >= set2->min_position) {
          size = bitset_index;
        } else {
          size = bitsetSize(set2);
        } /* if */
        bitset1 = &set1->bitset[bitset_index - 1];
        bitset2 = &set2->bitset[set2->max_position - set2->min_position];
      } /* if */
      index_byond = bitsetSize(set1);
      for (; bitset_index < index_byond; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return 1;
        } /* if */
      } /* for */
    } else {
      if (set1->max_position < set2->min_position) {
        bitset_index = 0;
        size = 0;
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        bitset_index = bitsetIndex(set2, set1->max_position + 1);
        if (set1->min_position <= set2->min_position) {
          size = bitset_index;
        } else {
          size = bitsetSize(set1);
        } /* if */
        bitset1 = &set1->bitset[set1->max_position - set1->min_position];
        bitset2 = &set2->bitset[bitset_index - 1];
      } /* if */
      index_byond = bitsetSize(set2);
      for (; bitset_index < index_byond; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          return -1;
        } /* if */
      } /* for */
    } /* if */
    for (bitset_index = 0; bitset_index < size; bitset_index++, bitset1--, bitset2--) {
      if (*bitset1 != *bitset2) {
        if (*bitset1 > *bitset2) {
          return 1;
        } else {
          return -1;
        } /* if */
      } /* if */
    } /* for */
    if (set1->min_position < set2->min_position) {
      if (set1->max_position < set2->min_position) {
        index_byond = bitsetSize(set1);
      } else {
        index_byond = bitsetIndex(set1, set2->min_position);
      } /* if */
      for (bitset_index = 0; bitset_index < index_byond; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return 1;
        } /* if */
      } /* for */
    } else if (set1->min_position > set2->min_position) {
      if (set1->min_position > set2->max_position) {
        index_byond = bitsetSize(set2);
      } else {
        index_byond = bitsetIndex(set2, set1->min_position);
      } /* if */
      for (bitset_index = 0; bitset_index < index_byond; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          return -1;
        } /* if */
      } /* for */
    } /* if */
    return 0;
  } /* setCmp */



/**
 *  Reinterpret the generic parameters as settype and call setCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(rtlGenerictype) != sizeof(settype).
 */
inttype setCmpGeneric (const rtlGenerictype value1, const rtlGenerictype value2)

  { /* setCmpGeneric */
    return setCmp((const_settype) (memsizetype) value1,
                  (const_settype) (memsizetype) value2);
  } /* setCmpGeneric */



void setCpy (settype *const set_to, const const_settype set_from)

  {
    settype set_dest;
    memsizetype set_dest_size;
    memsizetype set_source_size;

  /* setCpy */
    set_dest = *set_to;
    set_source_size = bitsetSize(set_from);
    if (set_dest->min_position != set_from->min_position ||
        set_dest->max_position != set_from->max_position) {
      set_dest_size = bitsetSize(set_dest);
      if (set_dest_size != set_source_size) {
        if (unlikely(!ALLOC_SET(set_dest, set_source_size))) {
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
    /* It is possible that *set_to == set_from holds. The */
    /* behavior of memcpy() is undefined when source and  */
    /* destination areas overlap (or are identical).      */
    /* Therefore memmove() is used instead of memcpy().   */
    memmove(set_dest->bitset, set_from->bitset,
        (size_t) set_source_size * sizeof(bitsettype));
  } /* setCpy */



settype setCreate (const const_settype set_from)

  {
    memsizetype new_size;
    settype result;

  /* setCreate */
    new_size = bitsetSize(set_from);
    if (unlikely(!ALLOC_SET(result, new_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = set_from->min_position;
      result->max_position = set_from->max_position;
      memcpy(result->bitset, set_from->bitset,
          (size_t) new_size * sizeof(bitsettype));
    } /* if */
    return result;
  } /* setCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(rtlGenerictype) != sizeof(settype).
 */
rtlGenerictype setCreateGeneric (const rtlGenerictype from_value)

  { /* setCreateGeneric */
    return (rtlGenerictype) (memsizetype)
           setCreate((const_settype) (memsizetype) from_value);
  } /* setCreateGeneric */



void setDestr (const const_settype old_set)

  { /* setDestr */
    if (old_set != NULL) {
      FREE_SET(old_set, bitsetSize(old_set));
    } /* if */
  } /* setDestr */



/**
 *  Difference of two sets.
 *  @return the difference of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
settype setDiff (const const_settype set1, const const_settype set2)

  {
    memsizetype bitset_index;
    memsizetype bitset_index2;
    memsizetype index_byond;
    settype result;

  /* setDiff */
#ifdef OUT_OF_ORDER
    printf("setDiff(");
    prot_set(set1);
    printf(", ");
    prot_set(set2);
    printf(")\n");
#endif
    if (unlikely(!ALLOC_SET(result, bitsetSize(set1)))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = set1->min_position;
      result->max_position = set1->max_position;
      if (set1->max_position < set2->min_position ||
          set1->min_position > set2->max_position) {
        memcpy(result->bitset, set1->bitset, bitsetSize(set1) * sizeof(bitsettype));
      } else {
        if (set2->min_position > set1->min_position) {
          bitset_index = bitsetIndex(set1, set2->min_position);
          bitset_index2 = 0;
          memcpy(result->bitset, set1->bitset, bitset_index * sizeof(bitsettype));
        } else {
          bitset_index = 0;
          bitset_index2 = bitsetIndex(set2, set1->min_position);
        } /* if */
        if (set1->max_position > set2->max_position) {
          index_byond = bitsetIndex(set1, set2->max_position + 1);
          memcpy(&result->bitset[index_byond], &set1->bitset[index_byond],
              (size_t) (uinttype) (set1->max_position - set2->max_position) * sizeof(bitsettype));
        } else {
          index_byond = bitsetSize(set1);
        } /* if */
        for (; bitset_index < index_byond; bitset_index++, bitset_index2++) {
          result->bitset[bitset_index] = set1->bitset[bitset_index] &
              ~ set2->bitset[bitset_index2];
        } /* for */
      } /* if */
    } /* if */
    return result;
  } /* setDiff */



/**
 *  Set membership test.
 *  Determine if 'number' is a member of the set 'aSet'.
 *  @return TRUE when 'number' is a member of  'aSet',
 *          FALSE otherwise.
 */
booltype setElem (const inttype number, const const_settype aSet)

  {
    inttype position;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* setElem */
    position = bitset_pos(number);
    if (position >= aSet->min_position && position <= aSet->max_position) {
      bitset_index = bitsetIndex(aSet, position);
      bit_index = ((unsigned int) number) & bitset_mask;
      if (aSet->bitset[bitset_index] & (bitsettype) 1 << bit_index) {
        return TRUE;
      } else {
        return FALSE;
      } /* if */
    } else {
      return FALSE;
    } /* if */
  } /* setElem */



settype setEmpty (void)

  {
    settype result;

  /* setEmpty */
    if (unlikely(!ALLOC_SET(result, 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = 0;
      result->max_position = 0;
      result->bitset[0] = (bitsettype) 0;
    } /* if */
    return result;
  } /* setEmpty */



/**
 *  Check if two sets are equal.
 *  @return TRUE if the two sets are equal,
 *          FALSE otherwise.
 */
booltype setEq (const const_settype set1, const const_settype set2)

  {
    memsizetype index_byond;
    memsizetype bitset_index;
    memsizetype size;
    const bitsettype *bitset1;
    const bitsettype *bitset2;

  /* setEq */
    if (set1->min_position == set2->min_position &&
        set1->max_position == set2->max_position) {
      return memcmp(set1->bitset, set2->bitset,
          bitsetSize(set1) * sizeof(bitsettype)) == 0;
    } else {
      if (set1->min_position < set2->min_position) {
        if (set1->max_position < set2->min_position) {
          size = 0;
          index_byond = bitsetSize(set1);
          bitset1 = NULL;
          bitset2 = NULL;
        } else {
          if (set1->max_position <= set2->max_position) {
            size = bitsetIndex(set2, set1->max_position + 1);
          } else {
            size = bitsetSize(set2);
          } /* if */
          index_byond = bitsetIndex(set1, set2->min_position);
          bitset1 = &set1->bitset[index_byond];
          bitset2 = set2->bitset;
        } /* if */
        for (bitset_index = 0; bitset_index < index_byond; bitset_index++) {
          if (set1->bitset[bitset_index] != 0) {
            return FALSE;
          } /* if */
        } /* for */
      } else {
        if (set1->min_position > set2->max_position) {
          size = 0;
          index_byond = bitsetSize(set2);
          bitset1 = NULL;
          bitset2 = NULL;
        } else {
          if (set1->max_position >= set2->max_position) {
            size = bitsetIndex(set1, set2->max_position + 1);
          } else {
            size = bitsetSize(set1);
          } /* if */
          index_byond = bitsetIndex(set2, set1->min_position);
          bitset1 = set1->bitset;
          bitset2 = &set2->bitset[index_byond];
        } /* if */
        for (bitset_index = 0; bitset_index < index_byond; bitset_index++) {
          if (set2->bitset[bitset_index] != 0) {
            return FALSE;
          } /* if */
        } /* for */
      } /* if */
      if (set1->max_position > set2->max_position) {
        if (set1->min_position > set2->max_position) {
          bitset_index = 0;
        } else {
          bitset_index = bitsetIndex(set1, set2->max_position + 1);
        } /* if */
        index_byond = bitsetSize(set1);
        for (; bitset_index < index_byond; bitset_index++) {
          if (set1->bitset[bitset_index] != 0) {
            return FALSE;
          } /* if */
        } /* for */
      } else {
        if (set1->max_position < set2->min_position) {
          bitset_index = 0;
        } else {
          bitset_index = bitsetIndex(set2, set1->max_position + 1);
        } /* if */
        index_byond = bitsetSize(set2);
        for (; bitset_index < index_byond; bitset_index++) {
          if (set2->bitset[bitset_index] != 0) {
            return FALSE;
          } /* if */
        } /* for */
      } /* if */
      return memcmp(bitset1, bitset2, size * sizeof(bitsettype)) == 0;
    } /* if */
  } /* setEq */



/**
 *  Remove 'number' from the set 'set_to'.
 *  When 'number' is not element of 'set_to' then 'set_to' stays unchanged.
 */
void setExcl (settype *const set_to, const inttype number)

  {
    settype set_dest;
    inttype position;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* setExcl */
    set_dest = *set_to;
    position = bitset_pos(number);
    if (position >= set_dest->min_position && position <= set_dest->max_position) {
      bitset_index = bitsetIndex(set_dest, position);
      bit_index = ((unsigned int) number) & bitset_mask;
      set_dest->bitset[bitset_index] &= ~((bitsettype) 1 << bit_index);
#ifdef OUT_OF_ORDER
      if (set_dest->bitset[bitset_index] == 0) {
        if
      } /* if */
#endif
    } /* if */
  } /* setExcl */



/**
 *  Compute the hash value of a set.
 *  @return the hash value.
 */
inttype setHashCode (const const_settype set1)

  {
    memsizetype bitset_size;
    memsizetype bitset_index;
    inttype result;

  /* setHashCode */
    result = 0;
    bitset_size = bitsetSize(set1);
    for (bitset_index = 0; bitset_index < bitset_size; bitset_index++) {
      result ^= (inttype) set1->bitset[bitset_index];
    } /* for */
    return result;
  } /* setHashCode */



/**
 *  Convert an integer number to a bitset.
 *  @return a bitset which corresponds to the given integer.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
settype setIConv (inttype number)

  {
    memsizetype result_size;
    settype result;

  /* setIConv */
    result_size = sizeof(inttype) / sizeof(bitsettype);
    if (unlikely(!ALLOC_SET(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = 0;
      result->max_position = (inttype) (result_size - 1);
      result->bitset[0] = (bitsettype) number;
#if BITSETTYPE_SIZE < INTTYPE_SIZE
      {
        memsizetype pos;

        for (pos = 1; pos < result_size; pos++) {
          number >>= 8 * sizeof(bitsettype);
          result->bitset[pos] = (bitsettype) number;
        } /* for */
      }
#endif
    } /* if */
    return result;
  } /* setIConv */



/**
 *  Add 'number' to the set 'set_to'.
 *  When 'number' is already in 'set_to' then 'set_to' stays unchanged.
 *  @exception MEMORY_ERROR When there is not enough memory.
 */
void setIncl (settype *const set_to, const inttype number)

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
      old_size = bitsetSize(set_dest);
      if (unlikely((uinttype) (position - set_dest->min_position + 1) > MAX_SET_LEN)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        new_size = bitsetSize2(set_dest->min_position, position);
        set_dest = REALLOC_SET(set_dest, old_size, new_size);
        if (unlikely(set_dest == NULL)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT3_SET(old_size, new_size);
          *set_to = set_dest;
          set_dest->max_position = position;
          memset(&set_dest->bitset[old_size], 0, (new_size - old_size) * sizeof(bitsettype));
        } /* if */
      } /* if */
    } else if (position < set_dest->min_position) {
      old_size = bitsetSize(set_dest);
      if (unlikely((uinttype) (set_dest->max_position - position + 1) > MAX_SET_LEN)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        new_size = bitsetSize2(position, set_dest->max_position);
        old_set = set_dest;
        if (unlikely(!ALLOC_SET(set_dest, new_size))) {
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
    } /* if */
    bitset_index = bitsetIndex(set_dest, position);
    bit_index = ((unsigned int) number) & bitset_mask;
    set_dest->bitset[bitset_index] |= (bitsettype) 1 << bit_index;
  } /* setIncl */



/**
 *  Intersection of two sets.
 *  @return the intersection of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
settype setIntersect (const const_settype set1, const const_settype set2)

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
      if (unlikely(!ALLOC_SET(result, 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = 0;
        result->max_position = 0;
        result->bitset[0] = (bitsettype) 0;
      } /* if */
    } else {
      if (unlikely(!ALLOC_SET(result, (uinttype) (max_position - min_position + 1)))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = min_position;
        result->max_position = max_position;
        for (position = min_position; position <= max_position; position++) {
          result->bitset[position - min_position] =
              set1->bitset[position - set1->min_position] &
              set2->bitset[position - set2->min_position];
        } /* for */
      } /* if */
    } /* if */
    return result;
  } /* setIntersect */



booltype setIsEmpty (const const_settype set1)

  {
    memsizetype index_byond;
    memsizetype bitset_index;

  /* setIsEmpty */
    index_byond = bitsetSize(set1);
    for (bitset_index = 0; bitset_index < index_byond; bitset_index++) {
      if (set1->bitset[bitset_index] != 0) {
        return FALSE;
      } /* if */
    } /* for */
    return TRUE;
  } /* setIsEmpty */



/**
 *  Determine if 'set1' is a proper subset of 'set2'.
 *  'set1' is a proper subset of 'set2' when
 *   set1 <= set2 and set1 <> set2
 *  holds.
 *  @return TRUE if 'set1' is a proper subset of 'set2',
 *          FALSE otherwise.
 */
booltype setIsProperSubset (const const_settype set1, const const_settype set2)

  {
    memsizetype index_byond;
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
        index_byond = bitsetSize(set1);
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        if (set1->max_position <= set2->max_position) {
          size = bitsetIndex(set2, set1->max_position + 1);
        } else {
          size = bitsetSize(set2);
        } /* if */
        index_byond = bitsetIndex(set1, set2->min_position);
        bitset1 = &set1->bitset[index_byond];
        bitset2 = set2->bitset;
      } /* if */
      for (bitset_index = 0; bitset_index < index_byond; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return FALSE;
        } /* if */
      } /* for */
    } else {
      if (set1->min_position > set2->max_position) {
        size = 0;
        index_byond = bitsetSize(set2);
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        if (set1->max_position >= set2->max_position) {
          size = bitsetIndex(set1, set2->max_position + 1);
        } else {
          size = bitsetSize(set1);
        } /* if */
        index_byond = bitsetIndex(set2, set1->min_position);
        bitset1 = set1->bitset;
        bitset2 = &set2->bitset[index_byond];
      } /* if */
      for (bitset_index = 0; bitset_index < index_byond && equal; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          equal = FALSE;
        } /* if */
      } /* for */
    } /* if */
    if (set1->max_position > set2->max_position) {
      if (set1->min_position > set2->max_position) {
        bitset_index = 0;
      } else {
        bitset_index = bitsetIndex(set1, set2->max_position + 1);
      } /* if */
      index_byond = bitsetSize(set1);
      for (; bitset_index < index_byond; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return FALSE;
        } /* if */
      } /* for */
    } else {
      if (set1->max_position < set2->min_position) {
        bitset_index = 0;
      } else {
        bitset_index = bitsetIndex(set2, set1->max_position + 1);
      } /* if */
      index_byond = bitsetSize(set2);
      for (; bitset_index < index_byond && equal; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          equal = FALSE;
        } /* if */
      } /* for */
    } /* if */
    for (bitset_index = 0; bitset_index < size; bitset_index++, bitset1++, bitset2++) {
      if ((~ *bitset1 | *bitset2) != ~(bitsettype) 0) {
        return FALSE;
      } /* if */
      if (*bitset1 != *bitset2) {
        equal = FALSE;
      } /* if */
    } /* for */
    return !equal;
  } /* setIsProperSubset */



/**
 *  Determine if 'set1' is a subset of 'set2'.
 *  'set1' is a subset of 'set2' when no element X exists for which
 *   X in set1 and X not in set2
 *  holds.
 *  @return TRUE if 'set1' is a subset of 'set2',
 *          FALSE otherwise.
 */
booltype setIsSubset (const const_settype set1, const const_settype set2)

  {
    memsizetype index_byond;
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
        index_byond = bitsetSize(set1);
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        if (set1->max_position <= set2->max_position) {
          size = bitsetIndex(set2, set1->max_position + 1);
        } else {
          size = bitsetSize(set2);
        } /* if */
        index_byond = bitsetIndex(set1, set2->min_position);
        bitset1 = &set1->bitset[index_byond];
        bitset2 = set2->bitset;
      } /* if */
      for (bitset_index = 0; bitset_index < index_byond; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return FALSE;
        } /* if */
      } /* for */
    } else {
      if (set1->min_position > set2->max_position) {
        size = 0;
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        if (set1->max_position >= set2->max_position) {
          size = bitsetIndex(set1, set2->max_position + 1);
        } else {
          size = bitsetSize(set1);
        } /* if */
        bitset1 = set1->bitset;
        bitset2 = &set2->bitset[set1->min_position - set2->min_position];
      } /* if */
    } /* if */
    if (set1->max_position > set2->max_position) {
      if (set1->min_position > set2->max_position) {
        bitset_index = 0;
      } else {
        bitset_index = bitsetIndex(set1, set2->max_position + 1);
      } /* if */
      index_byond = bitsetSize(set1);
      for (; bitset_index < index_byond; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return FALSE;
        } /* if */
      } /* for */
    } /* if */
    for (bitset_index = 0; bitset_index < size; bitset_index++, bitset1++, bitset2++) {
      if ((~ *bitset1 | *bitset2) != ~(bitsettype) 0) {
        return FALSE;
      } /* if */
    } /* for */
    return TRUE;
  } /* setIsSubset */



/**
 *  Maximal element of a set.
 *  Delivers the element from 'aSet' for which the following condition holds:
 *   element >= X
 *  for all X which are in the set.
 *  @return the maximal element of 'aSet'.
 *  @exception RANGE_ERROR When 'aSet' is the empty set.
 */
inttype setMax (const const_settype aSet)

  {
    memsizetype bitset_index;
    bitsettype curr_bitset;
    inttype result;

  /* setMax */
    bitset_index = bitsetSize(aSet);
    while (bitset_index > 0) {
      bitset_index--;
      curr_bitset = aSet->bitset[bitset_index];
      if (curr_bitset != 0) {
        result = uintMostSignificantBit(curr_bitset);
        result += (aSet->min_position + (inttype) bitset_index) << bitset_shift;
        return result;
      } /* if */
    } /* while */
    raise_error(RANGE_ERROR);
    return 0;
  } /* setMax */



/**
 *  Minimal element of a set.
 *  Delivers the element from 'aSet' for which the following condition holds:
 *   element <= X
 *  for all X which are in the set.
 *  @return the minimum element of 'aSet'.
 *  @exception RANGE_ERROR When 'aSet' is the empty set.
 */
inttype setMin (const const_settype aSet)

  {
    memsizetype bitset_size;
    memsizetype bitset_index;
    bitsettype curr_bitset;
    inttype result;

  /* setMin */
    bitset_size = bitsetSize(aSet);
    bitset_index = 0;
    while (bitset_index < bitset_size) {
      curr_bitset = aSet->bitset[bitset_index];
      if (curr_bitset != 0) {
        result = uintLeastSignificantBit(curr_bitset);
        result += (aSet->min_position + (inttype) bitset_index) << bitset_shift;
        return result;
      } /* if */
      bitset_index++;
    } /* while */
    raise_error(RANGE_ERROR);
    return 0;
  } /* setMin */



inttype setNext (const const_settype set1, const inttype number)

  {
    inttype position;
    memsizetype bitset_size;
    memsizetype bitset_index;
    unsigned int bit_index;
    bitsettype curr_bitset;
    inttype result;

  /* setNext */
    position = bitset_pos(number);
    if (position < set1->min_position) {
      position = set1->min_position;
    } /* if */
    bitset_size = bitsetSize(set1);
    bitset_index = bitsetIndex(set1, position);
    if (bitset_index < bitset_size) {
      bit_index = ((unsigned int) number) & bitset_mask;
      curr_bitset = set1->bitset[bitset_index] & (~(bitsettype) 1 << bit_index);
      if (curr_bitset != 0) {
        result = uintLeastSignificantBit(curr_bitset);
        result += (set1->min_position + (inttype) bitset_index) << bitset_shift;
        return result;
      } /* if */
      bitset_index++;
      while (bitset_index < bitset_size) {
        curr_bitset = set1->bitset[bitset_index];
        if (unlikely(curr_bitset != 0)) {
          result = uintLeastSignificantBit(curr_bitset);
          result += (set1->min_position + (inttype) bitset_index) << bitset_shift;
          return result;
        } /* if */
        bitset_index++;
      } /* while */
    } /* if */
    raise_error(RANGE_ERROR);
    return 0;
  } /* setNext */



/**
 *  Compute pseudo-random number which is element of 'aSet'.
 *  The random values are uniform distributed.
 *  @return a random number such that rand(aSet) in aSet holds.
 *  @exception RANGE_ERROR When 'aSet' is empty.
 */
inttype setRand (const const_settype aSet)

  {
    inttype num_elements;
    inttype elem_index;
    memsizetype bitset_size;
    memsizetype bitset_index;
    bitsettype curr_bitset;
    inttype result;

  /* setRand */
    num_elements = setCard(aSet);
    if (unlikely(num_elements == 0)) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      elem_index = intRand(1, num_elements);
      bitset_size = bitsetSize(aSet);
      bitset_index = 0;
      while (bitset_index < bitset_size) {
        curr_bitset = aSet->bitset[bitset_index];
        while (curr_bitset != 0) {
          result = uintLeastSignificantBit(curr_bitset);
          elem_index--;
          if (elem_index == 0) {
            result += (aSet->min_position + (inttype) bitset_index) << bitset_shift;
            return result;
          } /* if */
          curr_bitset &= ~((bitsettype) 1 << result);
        } /* while */
        bitset_index++;
      } /* while */
    } /* if */
    raise_error(RANGE_ERROR);
    return 0;
  } /* setRand */



settype setRangelit (const inttype lowerValue, const inttype upperValue)

  {
    inttype min_position;
    inttype max_position;
    memsizetype bitset_size;
    unsigned int bit_index;
    settype result;

  /* setRangelit */
    min_position = bitset_pos(lowerValue);
    max_position = bitset_pos(upperValue);
    if (min_position > max_position) {
      if (unlikely(!ALLOC_SET(result, 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = 0;
        result->max_position = 0;
        result->bitset[0] = (bitsettype) 0;
      } /* if */
    } else if (unlikely((uinttype) (max_position - min_position + 1) > MAX_SET_LEN ||
        !ALLOC_SET(result, (uinttype) (max_position - min_position + 1)))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result->min_position = min_position;
      result->max_position = max_position;
      bit_index = ((unsigned int) lowerValue) & bitset_mask;
      result->bitset[0] = ~(bitsettype) 0 << bit_index;
      if (min_position == max_position) {
        bit_index = ((unsigned int) upperValue) & bitset_mask;
        result->bitset[0] &= ~(~(bitsettype) 1 << bit_index);
      } else {
        bitset_size = bitsetSize2(min_position, max_position);
        memset(&result->bitset[1], 0xff, (bitset_size - 2) * sizeof(bitsettype));
        bit_index = ((unsigned int) upperValue) & bitset_mask;
        result->bitset[bitset_size - 1] = ~(~(bitsettype) 1 << bit_index);
      } /* if */
    } /* if */
    return result;
  } /* setRangelit */



/**
 *  Convert a bitset to integer.
 *  @return an integer which corresponds to the given bitset.
 *  @exception RANGE_ERROR When 'aSet' contains negative values or
 *             when it does not fit into an integer.
 */
inttype setSConv (const const_settype aSet)

  { /* setSConv */
    if (likely(aSet->min_position == 0 && aSet->max_position == 0)) {
      return (inttype) aSet->bitset[0];
    } else {
      raise_error(RANGE_ERROR);
      return 0;
    } /* if */
  } /* setSConv */



/**
 *  Symmetric difference of two sets.
 *  @return the symmetric difference of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
settype setSymdiff (const const_settype set1, const const_settype set2)

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
    if (unlikely((uinttype) (max_position - min_position + 1) > MAX_SET_LEN ||
        !ALLOC_SET(result, (uinttype) (max_position - min_position + 1)))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result->min_position = min_position;
      result->max_position = max_position;
      if (set1->max_position < set2->min_position ||
          set1->min_position > set2->max_position) {
        memcpy(&result->bitset[set1->min_position - min_position], set1->bitset,
               bitsetSize(set1) * sizeof(bitsettype));
        memcpy(&result->bitset[set2->min_position - min_position], set2->bitset,
               bitsetSize(set2) * sizeof(bitsettype));
        memset(&result->bitset[stop_position - min_position + 1], 0,
               (size_t) (uinttype) (start_position - stop_position - 1) * sizeof(bitsettype));
      } else {
        if (set2->min_position > set1->min_position) {
          memcpy(&result->bitset[set1->min_position - min_position], set1->bitset,
                 (size_t) (uinttype) (set2->min_position - set1->min_position) * sizeof(bitsettype));
        } else {
          memcpy(&result->bitset[set2->min_position - min_position], set2->bitset,
                 (size_t) (uinttype) (set1->min_position - set2->min_position) * sizeof(bitsettype));
        } /* if */
        if (set2->max_position > set1->max_position) {
          memcpy(&result->bitset[set1->max_position - min_position + 1],
                 &set2->bitset[set1->max_position - set2->min_position + 1],
                 (size_t) (uinttype) (set2->max_position - set1->max_position) * sizeof(bitsettype));
        } else {
          memcpy(&result->bitset[set2->max_position - min_position + 1],
                 &set1->bitset[set2->max_position - set1->min_position + 1],
                 (size_t) (uinttype) (set1->max_position - set2->max_position) * sizeof(bitsettype));
        } /* if */
        for (position = start_position; position <= stop_position; position++) {
          result->bitset[position - min_position] =
              set1->bitset[position - set1->min_position] ^
              set2->bitset[position - set2->min_position];
        } /* for */
      } /* if */
    } /* if */
    return result;
  } /* setSymdiff */



inttype setToInt (const const_settype set1, const inttype lowestBitNum)

  {
    inttype position;
    memsizetype bitset_index;
    unsigned int bit_index;

  /* setToInt */
    position = bitset_pos(lowestBitNum);
    if (position >= set1->min_position && position <= set1->max_position) {
      bitset_index = bitsetIndex(set1, position);
      bit_index = ((unsigned int) lowestBitNum) & bitset_mask;
      if (bit_index == 0) {
        return (inttype) set1->bitset[bitset_index];
      } else if (position < set1->max_position) {
        return (inttype) (set1->bitset[bitset_index] >> bit_index |
            set1->bitset[bitset_index + 1] << (8 * sizeof(bitsettype) - bit_index));
      } else {
        return (inttype) (set1->bitset[bitset_index] >> bit_index);
      } /* if */
    } else if (position == set1->min_position - 1) {
      bitset_index = bitsetIndex(set1, position);
      bit_index = ((unsigned int) lowestBitNum) & bitset_mask;
      return (inttype) (set1->bitset[bitset_index + 1] << (8 * sizeof(bitsettype) - bit_index));
    } else {
      return 0;
    } /* if */
  } /* setToInt */



/**
 *  Union of two sets.
 *  @return the union of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
settype setUnion (const const_settype set1, const const_settype set2)

  {
    inttype position;
    inttype min_position;
    inttype max_position;
    inttype start_position;
    inttype stop_position;
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
    if (unlikely((uinttype) (max_position - min_position + 1) > MAX_SET_LEN ||
        !ALLOC_SET(result, (uinttype) (max_position - min_position + 1)))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result->min_position = min_position;
      result->max_position = max_position;
      if (set1->max_position < set2->min_position ||
          set1->min_position > set2->max_position) {
        memcpy(&result->bitset[set1->min_position - min_position], set1->bitset,
               bitsetSize(set1) * sizeof(bitsettype));
        memcpy(&result->bitset[set2->min_position - min_position], set2->bitset,
               bitsetSize(set2) * sizeof(bitsettype));
        memset(&result->bitset[stop_position - min_position + 1], 0,
               (size_t) (uinttype) (start_position - stop_position - 1) * sizeof(bitsettype));
      } else {
        if (set2->min_position > set1->min_position) {
          memcpy(&result->bitset[set1->min_position - min_position], set1->bitset,
                 (size_t) (uinttype) (set2->min_position - set1->min_position) * sizeof(bitsettype));
        } else {
          memcpy(&result->bitset[set2->min_position - min_position], set2->bitset,
                 (size_t) (uinttype) (set1->min_position - set2->min_position) * sizeof(bitsettype));
        } /* if */
        if (set2->max_position > set1->max_position) {
          memcpy(&result->bitset[set1->max_position - min_position + 1],
                 &set2->bitset[set1->max_position - set2->min_position + 1],
                 (size_t) (uinttype) (set2->max_position - set1->max_position) * sizeof(bitsettype));
        } else {
          memcpy(&result->bitset[set2->max_position - min_position + 1],
                 &set1->bitset[set2->max_position - set1->min_position + 1],
                 (size_t) (uinttype) (set1->max_position - set2->max_position) * sizeof(bitsettype));
        } /* if */
        for (position = start_position; position <= stop_position; position++) {
          result->bitset[position - min_position] =
              set1->bitset[position - set1->min_position] |
              set2->bitset[position - set2->min_position];
        } /* for */
      } /* if */
#ifdef OUT_OF_ORDER
      printf("setUnion ==> ");
      prot_set(result);
      printf("\n");
#endif
    } /* if */
    return result;
  } /* setUnion */
