/********************************************************************/
/*                                                                  */
/*  set_rtl.c     Primitive actions for the set type.               */
/*  Copyright (C) 1989 - 2014  Thomas Mertes                        */
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
/*  Changes: 2004, 2005, 2010, 2012 - 2014  Thomas Mertes           */
/*  Content: Primitive actions for the set type.                    */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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



/**
 *  Determine the number of one bits in a bitset.
 *  The function uses a combination of sideways additions and
 *  a multiplication to count the bits set in a bitset element.
 *  @return the number of one bits.
 */
static inline uintType bitsetPopulation (bitSetType bitset)

  { /* bitsetPopulation */
#if BITSETTYPE_SIZE == 32
    bitset -= (bitset >> 1) & UINT32_SUFFIX(0x55555555);
    bitset =  (bitset       & UINT32_SUFFIX(0x33333333)) +
             ((bitset >> 2) & UINT32_SUFFIX(0x33333333));
    bitset = (bitset + (bitset >> 4)) & UINT32_SUFFIX(0x0f0f0f0f);
    return (uintType) ((bitset * UINT32_SUFFIX(0x01010101)) >> 24);
#elif BITSETTYPE_SIZE == 64
    bitset -= (bitset >> 1) & UINT64_SUFFIX(0x5555555555555555);
    bitset =  (bitset       & UINT64_SUFFIX(0x3333333333333333)) +
             ((bitset >> 2) & UINT64_SUFFIX(0x3333333333333333));
    bitset = (bitset + (bitset >> 4)) & UINT64_SUFFIX(0x0f0f0f0f0f0f0f0f);
    return (uintType) ((bitset * UINT64_SUFFIX(0x0101010101010101)) >> 56);
#endif
  } /* bitsetPopulation */



/**
 *  Find a a non-zero bitSet in an array of bitSets.
 *  This function uses loop unrolling inspired by Duff's device.
 *  @return a pointer to the non-zero bitSet or NULL if there was none.
 */
static inline const bitSetType *bitsetNonZero (register const bitSetType *bitset,
    const memSizeType len)

  {
    register memSizeType blockCount;

  /* bitsetNonZero */
    if (len != 0) {
      bitset--;
      blockCount = (len + 31) >> 5;
      switch (len & 31) {
        case  0: do { if (unlikely(* ++bitset != 0)) return bitset;
        case 31:      if (unlikely(* ++bitset != 0)) return bitset;
        case 30:      if (unlikely(* ++bitset != 0)) return bitset;
        case 29:      if (unlikely(* ++bitset != 0)) return bitset;
        case 28:      if (unlikely(* ++bitset != 0)) return bitset;
        case 27:      if (unlikely(* ++bitset != 0)) return bitset;
        case 26:      if (unlikely(* ++bitset != 0)) return bitset;
        case 25:      if (unlikely(* ++bitset != 0)) return bitset;
        case 24:      if (unlikely(* ++bitset != 0)) return bitset;
        case 23:      if (unlikely(* ++bitset != 0)) return bitset;
        case 22:      if (unlikely(* ++bitset != 0)) return bitset;
        case 21:      if (unlikely(* ++bitset != 0)) return bitset;
        case 20:      if (unlikely(* ++bitset != 0)) return bitset;
        case 19:      if (unlikely(* ++bitset != 0)) return bitset;
        case 18:      if (unlikely(* ++bitset != 0)) return bitset;
        case 17:      if (unlikely(* ++bitset != 0)) return bitset;
        case 16:      if (unlikely(* ++bitset != 0)) return bitset;
        case 15:      if (unlikely(* ++bitset != 0)) return bitset;
        case 14:      if (unlikely(* ++bitset != 0)) return bitset;
        case 13:      if (unlikely(* ++bitset != 0)) return bitset;
        case 12:      if (unlikely(* ++bitset != 0)) return bitset;
        case 11:      if (unlikely(* ++bitset != 0)) return bitset;
        case 10:      if (unlikely(* ++bitset != 0)) return bitset;
        case  9:      if (unlikely(* ++bitset != 0)) return bitset;
        case  8:      if (unlikely(* ++bitset != 0)) return bitset;
        case  7:      if (unlikely(* ++bitset != 0)) return bitset;
        case  6:      if (unlikely(* ++bitset != 0)) return bitset;
        case  5:      if (unlikely(* ++bitset != 0)) return bitset;
        case  4:      if (unlikely(* ++bitset != 0)) return bitset;
        case  3:      if (unlikely(* ++bitset != 0)) return bitset;
        case  2:      if (unlikely(* ++bitset != 0)) return bitset;
        case  1:      if (unlikely(* ++bitset != 0)) return bitset;
                } while (--blockCount > 0);
      } /* switch */
    } /* if */
    return NULL;
  } /* bitsetNonZero */



setType setArrlit (const_rtlArrayType arr1)

  {
    memSizeType length;
    intType number;
    intType position;
    unsigned int bit_index;
    memSizeType array_index;
    setType result;

  /* setArrlit */
    length = arraySize(arr1);
    if (unlikely(!ALLOC_SET(result, 1))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      if (length == 0) {
        result->min_position = 0;
        result->max_position = 0;
        result->bitset[0] = (bitSetType) 0;
      } else {
        number = arr1->arr[0].value.intValue;
        position = bitset_pos(number);
        result->min_position = position;
        result->max_position = position;
        bit_index = ((unsigned int) number) & bitset_mask;
        result->bitset[0] = (bitSetType) 1 << bit_index;
        for (array_index = 1; array_index < length; array_index++) {
          setIncl(&result, arr1->arr[array_index].value.intValue);
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



setType setBaselit (const intType number)

  {
    intType position;
    unsigned int bit_index;
    setType result;

  /* setBaselit */
    if (unlikely(!ALLOC_SET(result, 1))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      position = bitset_pos(number);
      result->min_position = position;
      result->max_position = position;
      bit_index = ((unsigned int) number) & bitset_mask;
      result->bitset[0] = (bitSetType) 1 << bit_index;
      return result;
    } /* if */
  } /* setBaselit */



/**
 *  Compute the cardinality of a set.
 *  The function is based on the function bitsetPopulation, which
 *  uses a combination of sideways additions and a multiplication
 *  to count the bits set in a bitset element.
 *  @return the number of elements in 'aSet'.
 */
intType setCard (const const_setType aSet)

  {
    memSizeType index_beyond;
    memSizeType bitset_index;
    bitSetType bitset;
    uintType card = 0;
    intType cardinality;

  /* setCard */
    index_beyond = bitsetSize(aSet);
    for (bitset_index = index_beyond; bitset_index > 0; bitset_index--) {
      bitset = aSet->bitset[bitset_index - 1];
      card += bitsetPopulation(bitset);
    } /* for */
    if (unlikely(card > INTTYPE_MAX)) {
      logError(printf("setCard(): Result does not fit into an integer.\n"););
      raise_error(RANGE_ERROR);
      cardinality = 0;
    } else {
      cardinality = (intType) card;
    } /* if */
    logFunction(printf("setCard --> " FMT_D "\n", cardinality););
    return cardinality;
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
intType setCmp (const const_setType set1, const const_setType set2)

  {
    memSizeType index_beyond;
    memSizeType bitset_index;
    memSizeType size;
    const bitSetType *bitset1;
    const bitSetType *bitset2;

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
      index_beyond = bitsetSize(set1);
      for (; bitset_index < index_beyond; bitset_index++) {
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
      index_beyond = bitsetSize(set2);
      for (; bitset_index < index_beyond; bitset_index++) {
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
        index_beyond = bitsetSize(set1);
      } else {
        index_beyond = bitsetIndex(set1, set2->min_position);
      } /* if */
      for (bitset_index = 0; bitset_index < index_beyond; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return 1;
        } /* if */
      } /* for */
    } else if (set1->min_position > set2->min_position) {
      if (set1->min_position > set2->max_position) {
        index_beyond = bitsetSize(set2);
      } else {
        index_beyond = bitsetIndex(set2, set1->min_position);
      } /* if */
      for (bitset_index = 0; bitset_index < index_beyond; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          return -1;
        } /* if */
      } /* for */
    } /* if */
    return 0;
  } /* setCmp */



/**
 *  Reinterpret the generic parameters as setType and call setCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(setType).
 */
intType setCmpGeneric (const genericType value1, const genericType value2)

  { /* setCmpGeneric */
    return setCmp(((const_rtlObjectType *) &value1)->value.setValue,
                  ((const_rtlObjectType *) &value2)->value.setValue);
  } /* setCmpGeneric */



void setCpy (setType *const set_to, const const_setType set_from)

  {
    setType set_dest;
    memSizeType set_dest_size;
    memSizeType set_source_size;

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
        (size_t) set_source_size * sizeof(bitSetType));
  } /* setCpy */



void setCpyGeneric (genericType *const dest, const genericType source)

  { /* setCpyGeneric */
    setCpy(&((rtlObjectType *) dest)->value.setValue,
           ((const_rtlObjectType *) &source)->value.setValue);
  } /* setCpyGeneric */



setType setCreate (const const_setType set_from)

  {
    memSizeType new_size;
    setType result;

  /* setCreate */
    new_size = bitsetSize(set_from);
    if (unlikely(!ALLOC_SET(result, new_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = set_from->min_position;
      result->max_position = set_from->max_position;
      memcpy(result->bitset, set_from->bitset,
          (size_t) new_size * sizeof(bitSetType));
    } /* if */
    return result;
  } /* setCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(setType).
 */
genericType setCreateGeneric (const genericType from_value)

  {
    rtlObjectType result;

  /* setCreateGeneric */
    INIT_GENERIC_PTR(result.value.genericValue);
    result.value.setValue =
        setCreate(((const_rtlObjectType *) &from_value)->value.setValue);
    return result.value.genericValue;
  } /* setCreateGeneric */



void setDestr (const const_setType old_set)

  { /* setDestr */
    if (old_set != NULL) {
      FREE_SET(old_set, bitsetSize(old_set));
    } /* if */
  } /* setDestr */



/**
 *  Generic Destr function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(setType).
 */
void setDestrGeneric (const genericType old_value)

  { /* setDestrGeneric */
    setDestr(((const_rtlObjectType *) &old_value)->value.setValue);
  } /* setDestrGeneric */



/**
 *  Difference of two sets.
 *  @return the difference of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
setType setDiff (const const_setType set1, const const_setType set2)

  {
    memSizeType bitset_index;
    memSizeType bitset_index2;
    memSizeType index_beyond;
    setType result;

  /* setDiff */
    logFunction(printf("setDiff(");
                prot_set(set1);
                printf(", ");
                prot_set(set2);
                printf(")\n"););
    if (unlikely(!ALLOC_SET(result, bitsetSize(set1)))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = set1->min_position;
      result->max_position = set1->max_position;
      if (set1->max_position < set2->min_position ||
          set1->min_position > set2->max_position) {
        memcpy(result->bitset, set1->bitset, bitsetSize(set1) * sizeof(bitSetType));
      } else {
        if (set2->min_position > set1->min_position) {
          bitset_index = bitsetIndex(set1, set2->min_position);
          bitset_index2 = 0;
          memcpy(result->bitset, set1->bitset, bitset_index * sizeof(bitSetType));
        } else {
          bitset_index = 0;
          bitset_index2 = bitsetIndex(set2, set1->min_position);
        } /* if */
        if (set1->max_position > set2->max_position) {
          index_beyond = bitsetIndex(set1, set2->max_position + 1);
          memcpy(&result->bitset[index_beyond], &set1->bitset[index_beyond],
              (size_t) (uintType) (set1->max_position - set2->max_position) * sizeof(bitSetType));
        } else {
          index_beyond = bitsetSize(set1);
        } /* if */
        for (; bitset_index < index_beyond; bitset_index++, bitset_index2++) {
          result->bitset[bitset_index] = set1->bitset[bitset_index] &
              ~ set2->bitset[bitset_index2];
        } /* for */
      } /* if */
    } /* if */
    logFunction(printf("setDiff --> ");
                prot_set(result);
                printf("\n"););
    return result;
  } /* setDiff */



/**
 *  Set membership test.
 *  Determine if 'number' is a member of the set 'aSet'.
 *  @return TRUE when 'number' is a member of  'aSet',
 *          FALSE otherwise.
 */
boolType setElem (const intType number, const const_setType aSet)

  {
    intType position;
    memSizeType bitset_index;
    unsigned int bit_index;

  /* setElem */
    position = bitset_pos(number);
    if (position >= aSet->min_position && position <= aSet->max_position) {
      bitset_index = bitsetIndex(aSet, position);
      bit_index = ((unsigned int) number) & bitset_mask;
      if (aSet->bitset[bitset_index] & (bitSetType) 1 << bit_index) {
        return TRUE;
      } else {
        return FALSE;
      } /* if */
    } else {
      return FALSE;
    } /* if */
  } /* setElem */



setType setEmpty (void)

  {
    setType result;

  /* setEmpty */
    if (unlikely(!ALLOC_SET(result, 1))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = 0;
      result->max_position = 0;
      result->bitset[0] = (bitSetType) 0;
    } /* if */
    return result;
  } /* setEmpty */



/**
 *  Check if two sets are equal.
 *  @return TRUE if the two sets are equal,
 *          FALSE otherwise.
 */
boolType setEq (const const_setType set1, const const_setType set2)

  {
    memSizeType index_beyond;
    memSizeType bitset_index;
    memSizeType size;
    const bitSetType *bitset1;
    const bitSetType *bitset2;

  /* setEq */
    if (set1->min_position == set2->min_position &&
        set1->max_position == set2->max_position) {
      return memcmp(set1->bitset, set2->bitset,
          bitsetSize(set1) * sizeof(bitSetType)) == 0;
    } else {
      if (set1->min_position < set2->min_position) {
        if (set1->max_position < set2->min_position) {
          size = 0;
          index_beyond = bitsetSize(set1);
          bitset1 = NULL;
          bitset2 = NULL;
        } else {
          if (set1->max_position <= set2->max_position) {
            size = bitsetIndex(set2, set1->max_position + 1);
          } else {
            size = bitsetSize(set2);
          } /* if */
          index_beyond = bitsetIndex(set1, set2->min_position);
          bitset1 = &set1->bitset[index_beyond];
          bitset2 = set2->bitset;
        } /* if */
        for (bitset_index = 0; bitset_index < index_beyond; bitset_index++) {
          if (set1->bitset[bitset_index] != 0) {
            return FALSE;
          } /* if */
        } /* for */
      } else {
        if (set1->min_position > set2->max_position) {
          size = 0;
          index_beyond = bitsetSize(set2);
          bitset1 = NULL;
          bitset2 = NULL;
        } else {
          if (set1->max_position >= set2->max_position) {
            size = bitsetIndex(set1, set2->max_position + 1);
          } else {
            size = bitsetSize(set1);
          } /* if */
          index_beyond = bitsetIndex(set2, set1->min_position);
          bitset1 = set1->bitset;
          bitset2 = &set2->bitset[index_beyond];
        } /* if */
        for (bitset_index = 0; bitset_index < index_beyond; bitset_index++) {
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
        index_beyond = bitsetSize(set1);
        for (; bitset_index < index_beyond; bitset_index++) {
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
        index_beyond = bitsetSize(set2);
        for (; bitset_index < index_beyond; bitset_index++) {
          if (set2->bitset[bitset_index] != 0) {
            return FALSE;
          } /* if */
        } /* for */
      } /* if */
      return memcmp(bitset1, bitset2, size * sizeof(bitSetType)) == 0;
    } /* if */
  } /* setEq */



/**
 *  Remove 'number' from the set 'set_to'.
 *  When 'number' is not element of 'set_to' then 'set_to' stays unchanged.
 */
void setExcl (setType *const set_to, const intType number)

  {
    setType set_dest;
    intType position;
    memSizeType bitset_index;
    unsigned int bit_index;

  /* setExcl */
    set_dest = *set_to;
    position = bitset_pos(number);
    if (position >= set_dest->min_position && position <= set_dest->max_position) {
      bitset_index = bitsetIndex(set_dest, position);
      bit_index = ((unsigned int) number) & bitset_mask;
      set_dest->bitset[bitset_index] &= ~((bitSetType) 1 << bit_index);
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
intType setHashCode (const const_setType set1)

  {
    memSizeType bitset_size;
    memSizeType bitset_index;
    intType result;

  /* setHashCode */
    result = 0;
    bitset_size = bitsetSize(set1);
    for (bitset_index = 0; bitset_index < bitset_size; bitset_index++) {
      result ^= (intType) set1->bitset[bitset_index];
    } /* for */
    return result;
  } /* setHashCode */



/**
 *  Convert an integer number to a bitset.
 *  @return a bitset which corresponds to the given integer.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
setType setIConv (intType number)

  {
    memSizeType result_size;
    setType result;

  /* setIConv */
    result_size = sizeof(intType) / sizeof(bitSetType);
    if (unlikely(!ALLOC_SET(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = 0;
      result->max_position = (intType) (result_size - 1);
      result->bitset[0] = (bitSetType) number;
#if BITSETTYPE_SIZE < INTTYPE_SIZE
      {
        memSizeType pos;

        for (pos = 1; pos < result_size; pos++) {
          number >>= 8 * sizeof(bitSetType);
          result->bitset[pos] = (bitSetType) number;
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
void setIncl (setType *const set_to, const intType number)

  {
    setType set_dest;
    intType position;
    memSizeType old_size;
    memSizeType new_size;
    setType old_set;
    memSizeType bitset_index;
    unsigned int bit_index;

  /* setIncl */
    set_dest = *set_to;
    position = bitset_pos(number);
    if (position > set_dest->max_position) {
      old_size = bitsetSize(set_dest);
      if (unlikely((uintType) (position - set_dest->min_position + 1) > MAX_SET_LEN)) {
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
          memset(&set_dest->bitset[old_size], 0, (new_size - old_size) * sizeof(bitSetType));
        } /* if */
      } /* if */
    } else if (position < set_dest->min_position) {
      old_size = bitsetSize(set_dest);
      if (unlikely((uintType) (set_dest->max_position - position + 1) > MAX_SET_LEN)) {
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
          memset(set_dest->bitset, 0, (new_size - old_size) * sizeof(bitSetType));
          memcpy(&set_dest->bitset[new_size - old_size], old_set->bitset, old_size * sizeof(bitSetType));
          FREE_SET(old_set, old_size);
        } /* if */
      } /* if */
    } /* if */
    bitset_index = bitsetIndex(set_dest, position);
    bit_index = ((unsigned int) number) & bitset_mask;
    set_dest->bitset[bitset_index] |= (bitSetType) 1 << bit_index;
  } /* setIncl */



/**
 *  Intersection of two sets.
 *  @return the intersection of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
setType setIntersect (const const_setType set1, const const_setType set2)

  {
    intType position;
    intType min_position;
    intType max_position;
    setType result;

  /* setIntersect */
    logFunction(printf("setIntersect(\n");
                prot_set(set1);
                printf(",\n");
                prot_set(set2);
                printf(")\n"););
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
        result->bitset[0] = (bitSetType) 0;
      } /* if */
    } else {
      if (unlikely(!ALLOC_SET(result, (uintType) (max_position - min_position + 1)))) {
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
    logFunction(printf("setIntersect --> ");
                prot_set(result);
                printf("\n"););
    return result;
  } /* setIntersect */



boolType setIsEmpty (const const_setType set1)

  {
    memSizeType index_beyond;
    memSizeType bitset_index;

  /* setIsEmpty */
    index_beyond = bitsetSize(set1);
    for (bitset_index = 0; bitset_index < index_beyond; bitset_index++) {
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
boolType setIsProperSubset (const const_setType set1, const const_setType set2)

  {
    memSizeType index_beyond;
    memSizeType bitset_index;
    memSizeType size;
    const bitSetType *bitset1;
    const bitSetType *bitset2;
    boolType equal;

  /* setIsProperSubset */
    logFunction(printf("setIsProperSubset(");
                prot_set(set1);
                printf(", ");
                prot_set(set2);
                printf(")\n"););
    equal = TRUE;
    if (set1->min_position < set2->min_position) {
      if (set1->max_position < set2->min_position) {
        size = 0;
        index_beyond = bitsetSize(set1);
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        if (set1->max_position <= set2->max_position) {
          size = bitsetIndex(set2, set1->max_position + 1);
        } else {
          size = bitsetSize(set2);
        } /* if */
        index_beyond = bitsetIndex(set1, set2->min_position);
        bitset1 = &set1->bitset[index_beyond];
        bitset2 = set2->bitset;
      } /* if */
      for (bitset_index = 0; bitset_index < index_beyond; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return FALSE;
        } /* if */
      } /* for */
    } else {
      if (set1->min_position > set2->max_position) {
        size = 0;
        index_beyond = bitsetSize(set2);
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        if (set1->max_position >= set2->max_position) {
          size = bitsetIndex(set1, set2->max_position + 1);
        } else {
          size = bitsetSize(set1);
        } /* if */
        index_beyond = bitsetIndex(set2, set1->min_position);
        bitset1 = set1->bitset;
        bitset2 = &set2->bitset[index_beyond];
      } /* if */
      for (bitset_index = 0; bitset_index < index_beyond && equal; bitset_index++) {
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
      index_beyond = bitsetSize(set1);
      for (; bitset_index < index_beyond; bitset_index++) {
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
      index_beyond = bitsetSize(set2);
      for (; bitset_index < index_beyond && equal; bitset_index++) {
        if (set2->bitset[bitset_index] != 0) {
          equal = FALSE;
        } /* if */
      } /* for */
    } /* if */
    for (bitset_index = 0; bitset_index < size; bitset_index++, bitset1++, bitset2++) {
      if ((~ *bitset1 | *bitset2) != ~(bitSetType) 0) {
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
boolType setIsSubset (const const_setType set1, const const_setType set2)

  {
    memSizeType index_beyond;
    memSizeType bitset_index;
    memSizeType size;
    const bitSetType *bitset1;
    const bitSetType *bitset2;

  /* setIsSubset */
    logFunction(printf("setIsSubset(");
                prot_set(set1);
                printf(", ");
                prot_set(set2);
                printf(")\n"););
    if (set1->min_position < set2->min_position) {
      if (set1->max_position < set2->min_position) {
        size = 0;
        index_beyond = bitsetSize(set1);
        bitset1 = NULL;
        bitset2 = NULL;
      } else {
        if (set1->max_position <= set2->max_position) {
          size = bitsetIndex(set2, set1->max_position + 1);
        } else {
          size = bitsetSize(set2);
        } /* if */
        index_beyond = bitsetIndex(set1, set2->min_position);
        bitset1 = &set1->bitset[index_beyond];
        bitset2 = set2->bitset;
      } /* if */
      for (bitset_index = 0; bitset_index < index_beyond; bitset_index++) {
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
      index_beyond = bitsetSize(set1);
      for (; bitset_index < index_beyond; bitset_index++) {
        if (set1->bitset[bitset_index] != 0) {
          return FALSE;
        } /* if */
      } /* for */
    } /* if */
    for (bitset_index = 0; bitset_index < size; bitset_index++, bitset1++, bitset2++) {
      if ((~ *bitset1 | *bitset2) != ~(bitSetType) 0) {
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
intType setMax (const const_setType aSet)

  {
    memSizeType bitset_index;
    bitSetType curr_bitset;
    intType result;

  /* setMax */
    bitset_index = bitsetSize(aSet);
    while (bitset_index > 0) {
      bitset_index--;
      curr_bitset = aSet->bitset[bitset_index];
      if (curr_bitset != 0) {
        result = bitsetMostSignificantBit(curr_bitset);
        result += (aSet->min_position + (intType) bitset_index) << bitset_shift;
        return result;
      } /* if */
    } /* while */
    logError(printf("setMax(): Set is empty.\n"););
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
intType setMin (const const_setType aSet)

  {
    memSizeType bitset_size;
    memSizeType bitset_index;
    bitSetType curr_bitset;
    intType result;

  /* setMin */
    bitset_size = bitsetSize(aSet);
    bitset_index = 0;
    while (bitset_index < bitset_size) {
      curr_bitset = aSet->bitset[bitset_index];
      if (curr_bitset != 0) {
        result = bitsetLeastSignificantBit(curr_bitset);
        result += (aSet->min_position + (intType) bitset_index) << bitset_shift;
        return result;
      } /* if */
      bitset_index++;
    } /* while */
    logError(printf("setMin(): Set is empty.\n"););
    raise_error(RANGE_ERROR);
    return 0;
  } /* setMin */



intType setNext (const const_setType set1, const intType number)

  {
    intType position;
    memSizeType bitset_size;
    memSizeType bitset_index;
    unsigned int bit_index;
    bitSetType curr_bitset;
    const bitSetType *bitset_ptr;
    intType result;

  /* setNext */
    position = bitset_pos(number);
    if (position < set1->min_position) {
      position = set1->min_position;
    } /* if */
    bitset_size = bitsetSize(set1);
    bitset_index = bitsetIndex(set1, position);
    if (bitset_index < bitset_size) {
      bit_index = ((unsigned int) number) & bitset_mask;
      curr_bitset = set1->bitset[bitset_index] & (~(bitSetType) 1 << bit_index);
      if (curr_bitset != 0) {
        result = bitsetLeastSignificantBit(curr_bitset);
        result += (set1->min_position + (intType) bitset_index) << bitset_shift;
        return result;
      } /* if */
      bitset_index++;
      /* printf("min_position=%ld\n", set1->min_position);
         printf("max_position=%ld\n", set1->max_position);
         printf("index=%lu\n", bitset_index);
         printf("size=%lu\n", bitset_size - bitset_index); */
      bitset_ptr = bitsetNonZero(&set1->bitset[bitset_index], bitset_size - bitset_index);
      if (bitset_ptr != NULL) {
        bitset_index = (memSizeType) (bitset_ptr - set1->bitset);
        result = bitsetLeastSignificantBit(*bitset_ptr);
        result += (set1->min_position + (intType) bitset_index) << bitset_shift;
        return result;
      } /* if */
    } /* if */
    logError(printf("setNext(set1, " FMT_D "): "
                    "The maximal element of a set has no next element.\n",
                    number););
    raise_error(RANGE_ERROR);
    return 0;
  } /* setNext */



/**
 *  Compute pseudo-random number which is element of 'aSet'.
 *  The random values are uniform distributed.
 *  @return a random number such that rand(aSet) in aSet holds.
 *  @exception RANGE_ERROR When 'aSet' is empty.
 */
intType setRand (const const_setType aSet)

  {
    intType num_elements;
    intType elem_index;
    memSizeType bitset_index;
    bitSetType curr_bitset;
    intType result;

  /* setRand */
    num_elements = setCard(aSet);
    if (unlikely(num_elements == 0)) {
      logError(printf("setRand(): Set is empty.\n"););
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      elem_index = intRand(1, num_elements);
      for (bitset_index = bitsetSize(aSet);
           bitset_index > 0 && elem_index > BITSETTYPE_SIZE; bitset_index--) {
        curr_bitset = aSet->bitset[bitset_index - 1];
        /* When elem_index > BITSETTYPE_SIZE holds */
        /* the element cannot be in curr_bitset.   */
        elem_index -= (intType) bitsetPopulation(curr_bitset);
      } /* for */
      for (; bitset_index > 0; bitset_index--) {
        curr_bitset = aSet->bitset[bitset_index - 1];
        while (curr_bitset != 0) {
          elem_index--;
          if (elem_index == 0) {
            result = bitsetLeastSignificantBit(curr_bitset) +
                ((aSet->min_position + (intType) (bitset_index - 1)) << bitset_shift);
            return result;
          } /* if */
          /* Turn off the rightmost one bit of curr_bitset: */
          curr_bitset &= curr_bitset - 1;
        } /* while */
      } /* for */
    } /* if */
    raise_error(RANGE_ERROR);
    return 0;
  } /* setRand */



setType setRangelit (const intType lowerValue, const intType upperValue)

  {
    intType min_position;
    intType max_position;
    memSizeType bitset_size;
    unsigned int bit_index;
    setType result;

  /* setRangelit */
    logFunction(printf("setRangelit(" FMT_D ", " FMT_D ")\n",
                       lowerValue, upperValue););
    min_position = bitset_pos(lowerValue);
    max_position = bitset_pos(upperValue);
    if (min_position > max_position) {
      if (unlikely(!ALLOC_SET(result, 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = 0;
        result->max_position = 0;
        result->bitset[0] = (bitSetType) 0;
      } /* if */
    } else if (unlikely((uintType) (max_position - min_position + 1) > MAX_SET_LEN ||
        !ALLOC_SET(result, (uintType) (max_position - min_position + 1)))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result->min_position = min_position;
      result->max_position = max_position;
      bit_index = ((unsigned int) lowerValue) & bitset_mask;
      result->bitset[0] = ~(bitSetType) 0 << bit_index;
      if (min_position == max_position) {
        bit_index = ((unsigned int) upperValue) & bitset_mask;
        result->bitset[0] &= ~(~(bitSetType) 1 << bit_index);
      } else {
        bitset_size = bitsetSize2(min_position, max_position);
        memset(&result->bitset[1], 0xff, (bitset_size - 2) * sizeof(bitSetType));
        bit_index = ((unsigned int) upperValue) & bitset_mask;
        result->bitset[bitset_size - 1] = ~(~(bitSetType) 1 << bit_index);
      } /* if */
    } /* if */
    logFunction(printf("setRangelit --> ");
                prot_set(result);
                printf("\n"););
    return result;
  } /* setRangelit */



/**
 *  Convert a bitset to integer.
 *  @return an integer which corresponds to the given bitset.
 *  @exception RANGE_ERROR When 'aSet' contains negative values or
 *             when it does not fit into an integer.
 */
intType setSConv (const const_setType aSet)

  { /* setSConv */
    if (likely(aSet->min_position == 0 && aSet->max_position == 0)) {
      return (intType) aSet->bitset[0];
    } else {
      logError(printf("setSConv(): "
                      "Set contains negative values or does not fit into an integer.\n"););
      raise_error(RANGE_ERROR);
      return 0;
    } /* if */
  } /* setSConv */



/**
 *  Symmetric difference of two sets.
 *  @return the symmetric difference of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
setType setSymdiff (const const_setType set1, const const_setType set2)

  {
    intType position;
    intType min_position;
    intType max_position;
    intType start_position;
    intType stop_position;
    setType result;

  /* setSymdiff */
    logFunction(printf("setSymdiff(\n");
                prot_set(set1);
                printf(",\n");
                prot_set(set2);
                printf(")\n"););
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
    if (unlikely((uintType) (max_position - min_position + 1) > MAX_SET_LEN ||
        !ALLOC_SET(result, (uintType) (max_position - min_position + 1)))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result->min_position = min_position;
      result->max_position = max_position;
      if (set1->max_position < set2->min_position ||
          set1->min_position > set2->max_position) {
        memcpy(&result->bitset[set1->min_position - min_position], set1->bitset,
               bitsetSize(set1) * sizeof(bitSetType));
        memcpy(&result->bitset[set2->min_position - min_position], set2->bitset,
               bitsetSize(set2) * sizeof(bitSetType));
        memset(&result->bitset[stop_position - min_position + 1], 0,
               (size_t) (uintType) (start_position - stop_position - 1) * sizeof(bitSetType));
      } else {
        if (set2->min_position > set1->min_position) {
          memcpy(&result->bitset[set1->min_position - min_position], set1->bitset,
                 (size_t) (uintType) (set2->min_position - set1->min_position) * sizeof(bitSetType));
        } else {
          memcpy(&result->bitset[set2->min_position - min_position], set2->bitset,
                 (size_t) (uintType) (set1->min_position - set2->min_position) * sizeof(bitSetType));
        } /* if */
        if (set2->max_position > set1->max_position) {
          memcpy(&result->bitset[set1->max_position - min_position + 1],
                 &set2->bitset[set1->max_position - set2->min_position + 1],
                 (size_t) (uintType) (set2->max_position - set1->max_position) * sizeof(bitSetType));
        } else {
          memcpy(&result->bitset[set2->max_position - min_position + 1],
                 &set1->bitset[set2->max_position - set1->min_position + 1],
                 (size_t) (uintType) (set1->max_position - set2->max_position) * sizeof(bitSetType));
        } /* if */
        for (position = start_position; position <= stop_position; position++) {
          result->bitset[position - min_position] =
              set1->bitset[position - set1->min_position] ^
              set2->bitset[position - set2->min_position];
        } /* for */
      } /* if */
    } /* if */
    logFunction(printf("setSymdiff --> ");
                prot_set(result);
                printf("\n"););
    return result;
  } /* setSymdiff */



uintType setToUInt (const const_setType set1, const intType lowestBitNum)

  {
    intType position;
    memSizeType bitset_index;
    unsigned int bit_index;

  /* setToUInt */
    position = bitset_pos(lowestBitNum);
    if (position >= set1->min_position && position <= set1->max_position) {
      bitset_index = bitsetIndex(set1, position);
      bit_index = ((unsigned int) lowestBitNum) & bitset_mask;
      if (bit_index == 0) {
        return (uintType) set1->bitset[bitset_index];
      } else if (position < set1->max_position) {
        return (uintType) (set1->bitset[bitset_index] >> bit_index |
            set1->bitset[bitset_index + 1] << (8 * sizeof(bitSetType) - bit_index));
      } else {
        return (uintType) (set1->bitset[bitset_index] >> bit_index);
      } /* if */
    } else if (position == set1->min_position - 1) {
      bitset_index = bitsetIndex(set1, position);
      bit_index = ((unsigned int) lowestBitNum) & bitset_mask;
      return (uintType) (set1->bitset[bitset_index + 1] << (8 * sizeof(bitSetType) - bit_index));
    } else {
      return 0;
    } /* if */
  } /* setToUInt */



/**
 *  Union of two sets.
 *  @return the union of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
setType setUnion (const const_setType set1, const const_setType set2)

  {
    intType position;
    intType min_position;
    intType max_position;
    intType start_position;
    intType stop_position;
    setType result;

  /* setUnion */
    logFunction(printf("setUnion(\n");
                prot_set(set1);
                printf(",\n");
                prot_set(set2);
                printf(")\n"););
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
    if (unlikely((uintType) (max_position - min_position + 1) > MAX_SET_LEN ||
        !ALLOC_SET(result, (uintType) (max_position - min_position + 1)))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result->min_position = min_position;
      result->max_position = max_position;
      if (set1->max_position < set2->min_position ||
          set1->min_position > set2->max_position) {
        memcpy(&result->bitset[set1->min_position - min_position], set1->bitset,
               bitsetSize(set1) * sizeof(bitSetType));
        memcpy(&result->bitset[set2->min_position - min_position], set2->bitset,
               bitsetSize(set2) * sizeof(bitSetType));
        memset(&result->bitset[stop_position - min_position + 1], 0,
               (size_t) (uintType) (start_position - stop_position - 1) * sizeof(bitSetType));
      } else {
        if (set2->min_position > set1->min_position) {
          memcpy(&result->bitset[set1->min_position - min_position], set1->bitset,
                 (size_t) (uintType) (set2->min_position - set1->min_position) * sizeof(bitSetType));
        } else {
          memcpy(&result->bitset[set2->min_position - min_position], set2->bitset,
                 (size_t) (uintType) (set1->min_position - set2->min_position) * sizeof(bitSetType));
        } /* if */
        if (set2->max_position > set1->max_position) {
          memcpy(&result->bitset[set1->max_position - min_position + 1],
                 &set2->bitset[set1->max_position - set2->min_position + 1],
                 (size_t) (uintType) (set2->max_position - set1->max_position) * sizeof(bitSetType));
        } else {
          memcpy(&result->bitset[set2->max_position - min_position + 1],
                 &set1->bitset[set2->max_position - set1->min_position + 1],
                 (size_t) (uintType) (set1->max_position - set2->max_position) * sizeof(bitSetType));
        } /* if */
        for (position = start_position; position <= stop_position; position++) {
          result->bitset[position - min_position] =
              set1->bitset[position - set1->min_position] |
              set2->bitset[position - set2->min_position];
        } /* for */
      } /* if */
    } /* if */
    logFunction(printf("setUnion --> ");
                prot_set(result);
                printf("\n"););
    return result;
  } /* setUnion */
