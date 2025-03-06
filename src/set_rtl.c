/********************************************************************/
/*                                                                  */
/*  set_rtl.c     Primitive actions for the set type.               */
/*  Copyright (C) 1989 - 2019  Thomas Mertes                        */
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
#include "limits.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "rtl_err.h"
#include "int_rtl.h"

#undef EXTERN
#define EXTERN
#include "set_rtl.h"



#if ANY_LOG_ACTIVE
void printSet (const_setType setValue)

  {
    intType position;
    bitSetType bitset_elem;
    unsigned int bit_index;
    boolType first_elem;

  /* printSet */
    if (setValue != NULL) {
      printf("set[" FMT_D "/" FMT_D "]{",
             setValue->min_position, setValue->max_position);
      first_elem = TRUE;
      for (position = setValue->min_position; position <= setValue->max_position; position++) {
        bitset_elem = setValue->bitset[position - setValue->min_position];
        if (bitset_elem != 0) {
          for (bit_index = 0; bit_index < CHAR_BIT * sizeof(bitSetType); bit_index++) {
            if (bitset_elem & ((bitSetType) 1) << bit_index) {
              if (first_elem) {
                first_elem = FALSE;
              } else {
                printf(", ");
              } /* if */
              printf(FMT_D, position << bitset_shift | (intType) bit_index);
            } /* if */
          } /* for */
        } /* if */
      } /* for */
      printf("}");
    } else {
      printf(" *NULL_SET* ");
    } /* if */
  } /* printSet */
#endif



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
      blockCount = (len + 31) >> 5;
      switch (len & 31) {
        case  0: do { if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 31:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 30:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 29:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 28:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 27:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 26:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 25:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 24:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 23:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 22:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 21:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 20:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 19:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 18:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 17:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 16:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 15:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 14:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 13:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 12:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 11:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case 10:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case  9:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case  8:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case  7:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case  6:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case  5:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case  4:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case  3:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case  2:      if (unlikely(*bitset != 0)) return bitset; bitset++;
        case  1:      if (unlikely(*bitset != 0)) return bitset; bitset++;
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
 *  @exception RANGE_ERROR Result does not fit into an integer.
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
 *  comparison function setCmp it is possible to sort an array of
 *  sets or to use sets as key in a hash table. The functions
 *  setIsSubset and setIsProperSubset are used to check if a set is
 *  a (proper) subset or superset of another set.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType setCmp (const const_setType set1, const const_setType set2)

  {
    memSizeType index_beyond;
    memSizeType bitset_index;
    memSizeType size;
    const bitSetType *bitset1;
    const bitSetType *bitset2;

  /* setCmp */
    logFunction(printf("setCmp(");
                printSet(set1);
                printf(", ");
                printSet(set2);
                printf(")\n"););
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
 *  if sizeof(genericType) != sizeof(setType).
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType setCmpGeneric (const genericType value1, const genericType value2)

  { /* setCmpGeneric */
    return setCmp(((const_rtlObjectType *) &value1)->value.setValue,
                  ((const_rtlObjectType *) &value2)->value.setValue);
  } /* setCmpGeneric */



/**
 *  Assign source to *dest.
 *  A copy function assumes that *dest contains a legal value.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void setCpy (setType *const dest, const const_setType source)

  {
    setType set_dest;
    memSizeType set_dest_size;
    memSizeType set_source_size;

  /* setCpy */
    set_dest = *dest;
    set_source_size = bitsetSize(source);
    if (set_dest->min_position != source->min_position ||
        set_dest->max_position != source->max_position) {
      set_dest_size = bitsetSize(set_dest);
      if (set_dest_size != set_source_size) {
        if (unlikely(!ALLOC_SET(set_dest, set_source_size))) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          FREE_SET(*dest, set_dest_size);
          *dest = set_dest;
        } /* if */
      } /* if */
      set_dest->min_position = source->min_position;
      set_dest->max_position = source->max_position;
    } /* if */
    /* It is possible that *dest == source holds. The    */
    /* behavior of memcpy() is undefined if source and   */
    /* destination areas overlap (or are identical).     */
    /* Therefore memmove() is used instead of memcpy().  */
    memmove(set_dest->bitset, source->bitset,
            (size_t) set_source_size * sizeof(bitSetType));
  } /* setCpy */



/**
 *  Reinterpret the generic parameters as setType and call setCpy.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(setType).
 */
void setCpyGeneric (genericType *const dest, const genericType source)

  { /* setCpyGeneric */
    setCpy(&((rtlObjectType *) dest)->value.setValue,
           ((const_rtlObjectType *) &source)->value.setValue);
  } /* setCpyGeneric */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
setType setCreate (const const_setType source)

  {
    memSizeType new_size;
    setType result;

  /* setCreate */
    new_size = bitsetSize(source);
    if (unlikely(!ALLOC_SET(result, new_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->min_position = source->min_position;
      result->max_position = source->max_position;
      memcpy(result->bitset, source->bitset,
             (size_t) new_size * sizeof(bitSetType));
    } /* if */
    return result;
  } /* setCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(setType).
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



/**
 *  Free the memory referred by 'old_set'.
 *  After setDestr is left 'old_set' refers to not existing memory.
 *  The memory where 'old_set' is stored can be freed afterwards.
 */
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
 *  if sizeof(genericType) != sizeof(setType).
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
    setType difference;

  /* setDiff */
    logFunction(printf("setDiff(");
                printSet(set1);
                printf(", ");
                printSet(set2);
                printf(")\n"););
    if (unlikely(!ALLOC_SET(difference, bitsetSize(set1)))) {
      raise_error(MEMORY_ERROR);
    } else {
      difference->min_position = set1->min_position;
      difference->max_position = set1->max_position;
      if (set1->max_position < set2->min_position ||
          set1->min_position > set2->max_position) {
        memcpy(difference->bitset, set1->bitset, bitsetSize(set1) * sizeof(bitSetType));
      } else {
        if (set2->min_position > set1->min_position) {
          bitset_index = bitsetIndex(set1, set2->min_position);
          bitset_index2 = 0;
          memcpy(difference->bitset, set1->bitset, bitset_index * sizeof(bitSetType));
        } else {
          bitset_index = 0;
          bitset_index2 = bitsetIndex(set2, set1->min_position);
        } /* if */
        if (set1->max_position > set2->max_position) {
          index_beyond = bitsetIndex(set1, set2->max_position + 1);
          memcpy(&difference->bitset[index_beyond], &set1->bitset[index_beyond],
                 (size_t) (uintType) (set1->max_position - set2->max_position) *
                 sizeof(bitSetType));
        } else {
          index_beyond = bitsetSize(set1);
        } /* if */
        for (; bitset_index < index_beyond; bitset_index++, bitset_index2++) {
          difference->bitset[bitset_index] = set1->bitset[bitset_index] &
              ~ set2->bitset[bitset_index2];
        } /* for */
      } /* if */
    } /* if */
    logFunction(printf("setDiff --> ");
                printSet(difference);
                printf("\n"););
    return difference;
  } /* setDiff */



/**
 *  Assign the difference of *dest and delta to *dest.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void setDiffAssign (setType *const dest, const const_setType delta)

  {
    setType set1;
    intType min_position;
    intType max_position;
    intType position;
    setType resized_set;

  /* setDiffAssign */
    logFunction(printf("setDiffAssign(");
                printSet(*dest);
                printf(", ");
                printSet(delta);
                printf(")\n"););
    set1 = *dest;
    min_position = set1->min_position;
    max_position = set1->max_position;
    while (min_position <= max_position &&
           (set1->bitset[min_position - set1->min_position] == 0 ||
            (min_position >= delta->min_position &&
             min_position <= delta->max_position &&
             (set1->bitset[min_position - set1->min_position] &
              ~ delta->bitset[min_position - delta->min_position]) == 0))) {
      min_position++;
    } /* while */
    while (min_position <= max_position &&
           (set1->bitset[max_position - set1->min_position] == 0 ||
            (max_position >= delta->min_position &&
             max_position <= delta->max_position &&
             (set1->bitset[max_position - set1->min_position] &
              ~ delta->bitset[max_position - delta->min_position]) == 0))) {
      max_position--;
    } /* while */
    if (min_position > max_position) {
      resized_set = REALLOC_SET(set1, bitsetSize(set1), 1);
      if (unlikely(resized_set == NULL)) {
        /* Strange case if a 'realloc', which shrinks memory, fails. */
        /* The destination set stays unchanged. */
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_SET(bitsetSize(resized_set), 1);
        resized_set->min_position = 0;
        resized_set->max_position = 0;
        resized_set->bitset[0] = (bitSetType) 0;
        *dest = resized_set;
      } /* if */
    } else if (min_position == set1->min_position) {
      if (max_position != set1->max_position) {
        resized_set = REALLOC_SET(set1, bitsetSize(set1),
                                  bitsetSize2(min_position, max_position));
        if (unlikely(resized_set == NULL)) {
          /* Strange case if a 'realloc', which shrinks memory, fails. */
          /* The destination set stays unchanged. */
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT3_SET(bitsetSize(resized_set),
                     bitsetSize2(min_position, max_position));
          set1 = resized_set;
          set1->max_position = max_position;
          *dest = set1;
        } /* if */
      } /* if */
      for (position = min_position; position <= max_position; position++) {
        if (position >= delta->min_position &&
            position <= delta->max_position) {
          set1->bitset[position - min_position] &=
              ~ delta->bitset[position - delta->min_position];
        } /* if */
      } /* for */
    } else {
      for (position = min_position; position <= max_position; position++) {
        if (position >= delta->min_position &&
            position <= delta->max_position) {
          set1->bitset[position - min_position] =
              set1->bitset[position - set1->min_position] &
              ~ delta->bitset[position - delta->min_position];
        } else {
          set1->bitset[position - min_position] =
              set1->bitset[position - set1->min_position];
        } /* if */
      } /* for */
      resized_set = REALLOC_SET(set1, bitsetSize(set1),
                                bitsetSize2(min_position, max_position));
      if (unlikely(resized_set == NULL)) {
        /* Strange case if a 'realloc', which shrinks memory, fails. */
        /* Deliver the result in the original set (that is too big). */
        set1->min_position = min_position;
        set1->max_position = max_position;
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_SET(bitsetSize(resized_set),
                   bitsetSize2(min_position, max_position));
        resized_set->min_position = min_position;
        resized_set->max_position = max_position;
        *dest = resized_set;
      } /* if */
    } /* if */
    logFunction(printf("setDiffAssign --> ");
                printSet(*dest);
                printf("\n"););
  } /* setDiffAssign */



/**
 *  Set membership test.
 *  Determine if 'number' is a member of the set 'aSet'.
 *  @return TRUE if 'number' is a member of  'aSet',
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
    logFunction(printf("setEq(");
                printSet(set1);
                printf(", ");
                printSet(set2);
                printf(")\n"););
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
      /* 'bitset1' and 'bitset2' might be NULL. In this case 'size' is 0. */
      /* According to the specification memcmp() returns 0 when size is 0. */
      return memcmp(bitset1, bitset2, size * sizeof(bitSetType)) == 0;
    } /* if */
  } /* setEq */



/**
 *  Remove 'number' from the set 'set_to'.
 *  If 'number' is not element of 'set_to' then 'set_to' stays unchanged.
 */
void setExcl (setType *const set_to, const intType number)

  {
    setType set_dest;
    intType position;
    memSizeType bitset_index;
    unsigned int bit_index;
    intType min_position;
    intType max_position;
    setType resized_set;

  /* setExcl */
    logFunction(printf("setExcl(");
                printSet(*set_to);
                printf(", " FMT_D ")\n", number););
    set_dest = *set_to;
    position = bitset_pos(number);
    if (position >= set_dest->min_position && position <= set_dest->max_position) {
      bitset_index = bitsetIndex(set_dest, position);
      bit_index = ((unsigned int) number) & bitset_mask;
      set_dest->bitset[bitset_index] &= ~((bitSetType) 1 << bit_index);
      if (set_dest->bitset[bitset_index] == 0 &&
          set_dest->min_position != set_dest->max_position) {
        if (position == set_dest->min_position) {
          min_position = position + 1;
          while (min_position <= set_dest->max_position &&
                 set_dest->bitset[min_position - set_dest->min_position] == 0) {
            min_position++;
          } /* while */
          if (min_position > set_dest->max_position) {
            min_position = set_dest->max_position;
          } else {
            memmove(set_dest->bitset,
                    &set_dest->bitset[min_position - set_dest->min_position],
                    (size_t) (set_dest->max_position - min_position + 1) *
                        sizeof(bitSetType));
          } /* if */
          resized_set = REALLOC_SET(set_dest, bitsetSize(set_dest),
                                    bitsetSize2(min_position, set_dest->max_position));
          if (unlikely(resized_set == NULL)) {
            /* Strange case if a 'realloc', which shrinks memory, fails. */
            /* Deliver the result in the original set (that is too big). */
            set_dest->min_position = min_position;
            raise_error(MEMORY_ERROR);
          } else {
            COUNT3_SET(bitsetSize(resized_set),
                       bitsetSize2(min_position, resized_set->max_position));
            *set_to = resized_set;
            resized_set->min_position = min_position;
          } /* if */
        } else if (position == set_dest->max_position) {
          max_position = position - 1;
          while (max_position >= set_dest->min_position &&
                 set_dest->bitset[max_position - set_dest->min_position] == 0) {
            max_position--;
          } /* while */
          if (max_position < set_dest->min_position) {
            max_position = set_dest->min_position;
          } /* if */
          resized_set = REALLOC_SET(set_dest, bitsetSize(set_dest),
                                    bitsetSize2(set_dest->min_position, max_position));
          if (unlikely(resized_set == NULL)) {
            /* Strange case if a 'realloc', which shrinks memory, fails. */
            /* Deliver the result in the original set (that is too big). */
            set_dest->max_position = max_position;
            raise_error(MEMORY_ERROR);
          } else {
            COUNT3_SET(bitsetSize(resized_set),
                       bitsetSize2(resized_set->min_position, max_position));
            *set_to = resized_set;
            resized_set->max_position = max_position;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setExcl(");
                printSet(*set_to);
                printf(", " FMT_D ") -->\n", number););
  } /* setExcl */



/**
 *  Compute the hash value of a set.
 *  @return the hash value.
 */
intType setHashCode (const const_setType set1)

  {
    memSizeType bitset_size;
    memSizeType bitset_index;
    intType hashCode;

  /* setHashCode */
    hashCode = 0;
    bitset_size = bitsetSize(set1);
    for (bitset_index = 0; bitset_index < bitset_size; bitset_index++) {
      hashCode ^= (intType) set1->bitset[bitset_index];
    } /* for */
    return hashCode;
  } /* setHashCode */



/**
 *  Generic hashCode function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(setType).
 */
intType setHashCodeGeneric (const genericType aValue)

  { /* setHashCodeGeneric */
    return setHashCode(((const_rtlObjectType *) &aValue)->value.setValue);
  } /* setHashCodeGeneric */



/**
 *  Convert an integer number to a bitset.
 *  @return a bitset which corresponds to the given integer.
 *  @exception RANGE_ERROR Number is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
setType setIConv (intType number)

  {
    memSizeType result_size;
    setType result;

  /* setIConv */
    logFunction(printf("setIConv(" FMT_D ")\n", number););
    if (unlikely(number < 0)) {
      logError(printf("setIConv(" FMT_D "): Number is negative.\n",
                      number););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
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
            number >>= CHAR_BIT * sizeof(bitSetType);
            result->bitset[pos] = (bitSetType) number;
          } /* for */
        }
#endif
      } /* if */
    } /* if */
    logFunction(printf("setIConv(" FMT_D ") --> ", number);
                printSet(result);
                printf("\n"););
    return result;
  } /* setIConv */



/**
 *  Add 'number' to the set 'set_to'.
 *  If 'number' is already in 'set_to' then 'set_to' stays unchanged.
 *  @exception MEMORY_ERROR If there is not enough memory.
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
    logFunction(printf("setIncl(");
                printSet(*set_to);
                printf(", " FMT_D ")\n", number););
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
          memcpy(&set_dest->bitset[new_size - old_size], old_set->bitset,
                 old_size * sizeof(bitSetType));
          FREE_SET(old_set, old_size);
        } /* if */
      } /* if */
    } /* if */
    bitset_index = bitsetIndex(set_dest, position);
    bit_index = ((unsigned int) number) & bitset_mask;
    set_dest->bitset[bitset_index] |= (bitSetType) 1 << bit_index;
    logFunction(printf("setIncl(");
                printSet(*set_to);
                printf(", " FMT_D ") -->\n", number););
  } /* setIncl */



/**
 *  Intersection of two sets.
 *  @return the intersection of the two sets.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
setType setIntersect (const const_setType set1, const const_setType set2)

  {
    intType min_position;
    intType max_position;
    intType position;
    setType intersection;

  /* setIntersect */
    logFunction(printf("setIntersect(\n");
                printSet(set1);
                printf(",\n");
                printSet(set2);
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
    while (min_position <= max_position &&
           (set1->bitset[min_position - set1->min_position] &
            set2->bitset[min_position - set2->min_position]) == 0) {
      min_position++;
    } /* while */
    while (min_position <= max_position &&
           (set1->bitset[max_position - set1->min_position] &
            set2->bitset[max_position - set2->min_position]) == 0) {
      max_position--;
    } /* while */
    if (min_position > max_position) {
      if (unlikely(!ALLOC_SET(intersection, 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        intersection->min_position = 0;
        intersection->max_position = 0;
        intersection->bitset[0] = (bitSetType) 0;
      } /* if */
    } else {
      if (unlikely(!ALLOC_SET(intersection, (uintType) (max_position - min_position + 1)))) {
        raise_error(MEMORY_ERROR);
      } else {
        intersection->min_position = min_position;
        intersection->max_position = max_position;
        for (position = min_position; position <= max_position; position++) {
          intersection->bitset[position - min_position] =
              set1->bitset[position - set1->min_position] &
              set2->bitset[position - set2->min_position];
        } /* for */
      } /* if */
    } /* if */
    logFunction(printf("setIntersect --> ");
                printSet(intersection);
                printf("\n"););
    return intersection;
  } /* setIntersect */



/**
 *  Assign the intersection of *dest and delta to *dest.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void setIntersectAssign (setType *const dest, const const_setType delta)

  {
    setType set1;
    intType min_position;
    intType max_position;
    intType position;
    setType resized_set;

  /* setIntersectAssign */
    logFunction(printf("setIntersectAssign(\n");
                printSet(*dest);
                printf(",\n");
                printSet(delta);
                printf(")\n"););
    set1 = *dest;
    if (set1->min_position > delta->min_position) {
      min_position = set1->min_position;
    } else {
      min_position = delta->min_position;
    } /* if */
    if (set1->max_position < delta->max_position) {
      max_position = set1->max_position;
    } else {
      max_position = delta->max_position;
    } /* if */
    while (min_position <= max_position &&
           (set1->bitset[min_position - set1->min_position] &
            delta->bitset[min_position - delta->min_position]) == 0) {
      min_position++;
    } /* while */
    while (min_position <= max_position &&
           (set1->bitset[max_position - set1->min_position] &
            delta->bitset[max_position - delta->min_position]) == 0) {
      max_position--;
    } /* while */
    if (min_position > max_position) {
      resized_set = REALLOC_SET(set1, bitsetSize(set1), 1);
      if (unlikely(resized_set == NULL)) {
        /* Strange case if a 'realloc', which shrinks memory, fails. */
        /* The destination set stays unchanged. */
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_SET(bitsetSize(resized_set), 1);
        resized_set->min_position = 0;
        resized_set->max_position = 0;
        resized_set->bitset[0] = (bitSetType) 0;
        *dest = resized_set;
      } /* if */
    } else if (min_position == set1->min_position) {
      if (max_position != set1->max_position) {
        resized_set = REALLOC_SET(set1, bitsetSize(set1),
                                  bitsetSize2(min_position, max_position));
        if (unlikely(resized_set == NULL)) {
          /* Strange case if a 'realloc', which shrinks memory, fails. */
          /* The destination set stays unchanged. */
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT3_SET(bitsetSize(resized_set),
                     bitsetSize2(min_position, max_position));
          set1 = resized_set;
          set1->max_position = max_position;
          *dest = set1;
        } /* if */
      } /* if */
      for (position = min_position; position <= max_position; position++) {
        set1->bitset[position - min_position] &=
            delta->bitset[position - delta->min_position];
      } /* for */
    } else {
      for (position = min_position; position <= max_position; position++) {
        set1->bitset[position - min_position] =
            set1->bitset[position - set1->min_position] &
            delta->bitset[position - delta->min_position];
      } /* for */
      resized_set = REALLOC_SET(set1, bitsetSize(set1),
                                bitsetSize2(min_position, max_position));
      if (unlikely(resized_set == NULL)) {
        /* Strange case if a 'realloc', which shrinks memory, fails. */
        /* Deliver the result in the original set (that is too big). */
        set1->min_position = min_position;
        set1->max_position = max_position;
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_SET(bitsetSize(resized_set),
                   bitsetSize2(min_position, max_position));
        resized_set->min_position = min_position;
        resized_set->max_position = max_position;
        *dest = resized_set;
      } /* if */
    } /* if */
    logFunction(printf("setIntersectAssign --> ");
                printSet(*dest);
                printf("\n"););
  } /* setIntersectAssign */



/**
 *  Determine if 'set1' is the empty set.
 *  @return TRUE if 'set1' is the empty set,
 *          FALSE otherwise.
 */
boolType setIsEmpty (const const_setType set1)

  {
    register memSizeType bitset_index;

  /* setIsEmpty */
    bitset_index = bitsetSize(set1);
    do {
      bitset_index--;
      if (set1->bitset[bitset_index] != 0) {
        return FALSE;
      } /* if */
    } while (bitset_index != 0);
    return TRUE;
  } /* setIsEmpty */



/**
 *  Determine if 'set1' is a proper subset of 'set2'.
 *  'set1' is a proper subset of 'set2' if
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
                printSet(set1);
                printf(", ");
                printSet(set2);
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
 *  'set1' is a subset of 'set2' if no element X exists for which
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
                printSet(set1);
                printf(", ");
                printSet(set2);
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
 *  Maximum element of a set.
 *  Delivers the element from 'aSet' for which the following condition holds:
 *   element >= X
 *  for all X which are in the set.
 *  @return the maximal element of 'aSet'.
 *  @exception RANGE_ERROR If 'aSet' is the empty set.
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
        result += lowestBitsetPosAsInteger(aSet->min_position + (intType) bitset_index);
        return result;
      } /* if */
    } /* while */
    logError(printf("setMax(): Set is empty.\n"););
    raise_error(RANGE_ERROR);
    return 0;
  } /* setMax */



/**
 *  Minimum element of a set.
 *  Delivers the element from 'aSet' for which the following condition holds:
 *   element <= X
 *  for all X which are in the set.
 *  @return the minimum element of 'aSet'.
 *  @exception RANGE_ERROR If 'aSet' is the empty set.
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
        result += lowestBitsetPosAsInteger(aSet->min_position + (intType) bitset_index);
        return result;
      } /* if */
      bitset_index++;
    } /* while */
    logError(printf("setMin(): Set is empty.\n"););
    raise_error(RANGE_ERROR);
    return 0;
  } /* setMin */



/**
 *  Minimum element of 'aSet' that is larger than 'number'.
 *  @return the minimum element of 'aSet' that is larger than 'number'.
 *  @exception RANGE_ERROR If 'aSet' has no element larger than 'number'.
 */
intType setNext (const const_setType aSet, const intType number)

  {
    intType position;
    memSizeType bitset_size;
    memSizeType bitset_index;
    unsigned int bit_index;
    bitSetType curr_bitset;
    const bitSetType *bitset_ptr;
    intType nextNumber;

  /* setNext */
    logFunction(printf("setNext(");
                printSet(aSet);
                printf(", " FMT_D ")\n", number););
    if (unlikely(number == INTTYPE_MAX)) {
      logError(printf("setNext(aSet, " FMT_D "): "
                      "The maximum integer has no successor.\n",
                      number););
      raise_error(RANGE_ERROR);
      nextNumber = 0;
    } else {
      position = bitset_pos(number + 1);
      if (position < aSet->min_position) {
        position = aSet->min_position;
        bit_index = 0;
      } else {
        bit_index = ((unsigned int) (number + 1)) & bitset_mask;
      } /* if */
      bitset_size = bitsetSize(aSet);
      bitset_index = bitsetIndex(aSet, position);
      if (unlikely(bitset_index >= bitset_size)) {
        logError(printf("setNext(aSet, " FMT_D "): "
                        "The number is beyond the maximum element of the set.\n",
                        number););
        raise_error(RANGE_ERROR);
        nextNumber = 0;
      } else {
        curr_bitset = (aSet->bitset[bitset_index] >> bit_index) << bit_index;
        if (curr_bitset != 0) {
          nextNumber = bitsetLeastSignificantBit(curr_bitset);
          nextNumber += lowestBitsetPosAsInteger(aSet->min_position + (intType) bitset_index);
        } else {
          bitset_index++;
          bitset_ptr = bitsetNonZero(&aSet->bitset[bitset_index], bitset_size - bitset_index);
          if (unlikely(bitset_ptr == NULL)) {
            logError(printf("setNext(aSet, " FMT_D "): "
                            "The maximum element of a set has no next element.\n",
                            number););
            raise_error(RANGE_ERROR);
            nextNumber = 0;
          } else {
            bitset_index = (memSizeType) (bitset_ptr - aSet->bitset);
            nextNumber = bitsetLeastSignificantBit(*bitset_ptr);
            nextNumber += lowestBitsetPosAsInteger(aSet->min_position + (intType) bitset_index);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setNext --> " FMT_D "\n", nextNumber););
    return nextNumber;
  } /* setNext */



/**
 *  Compute pseudo-random element from 'aSet'.
 *  The random values are uniform distributed.
 *  @return a random number such that setRand(aSet) in aSet holds.
 *  @exception RANGE_ERROR If 'aSet' is empty.
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
        /* If elem_index > BITSETTYPE_SIZE holds */
        /* the element cannot be in curr_bitset. */
        elem_index -= (intType) bitsetPopulation(curr_bitset);
      } /* for */
      for (; bitset_index > 0; bitset_index--) {
        curr_bitset = aSet->bitset[bitset_index - 1];
        while (curr_bitset != 0) {
          elem_index--;
          if (elem_index == 0) {
            result = bitsetLeastSignificantBit(curr_bitset) +
                lowestBitsetPosAsInteger(aSet->min_position + (intType) (bitset_index - 1));
            return result;
          } /* if */
          /* Turn off the rightmost one bit of curr_bitset: */
          curr_bitset &= curr_bitset - 1;
        } /* while */
      } /* for */
      /* This place should never be reached, since the */
      /* check for an empty set has been done before.  */
    } /* if */
    raise_error(RANGE_ERROR);
    return 0;
  } /* setRand */



/**
 *  Create set with all values from 'lowValue' to 'highValue' inclusive.
 *  @param lowValue lowest value to be added to the result set.
 *  @param highValue highest value to be added to the result set.
 *  @return set with all values from 'lowValue' to 'highValue' inclusive, or
 *          an empty set if 'lowValue' is greater than 'highValue'.
 */
setType setRangelit (const intType lowValue, const intType highValue)

  {
    intType min_position;
    intType max_position;
    memSizeType bitset_size;
    unsigned int bit_index;
    setType result;

  /* setRangelit */
    logFunction(printf("setRangelit(" FMT_D ", " FMT_D ")\n",
                       lowValue, highValue););
    min_position = bitset_pos(lowValue);
    max_position = bitset_pos(highValue);
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
      bit_index = ((unsigned int) lowValue) & bitset_mask;
      result->bitset[0] = ~(bitSetType) 0 << bit_index;
      if (min_position == max_position) {
        bit_index = ((unsigned int) highValue) & bitset_mask;
        result->bitset[0] &= ~(~(bitSetType) 1 << bit_index);
      } else {
        bitset_size = bitsetSize2(min_position, max_position);
        memset(&result->bitset[1], 0xff, (bitset_size - 2) * sizeof(bitSetType));
        bit_index = ((unsigned int) highValue) & bitset_mask;
        result->bitset[bitset_size - 1] = ~(~(bitSetType) 1 << bit_index);
      } /* if */
    } /* if */
    logFunction(printf("setRangelit --> ");
                printSet(result);
                printf("\n"););
    return result;
  } /* setRangelit */



/**
 *  Convert a bitset to integer.
 *  @return an integer which corresponds to the given bitset.
 *  @exception RANGE_ERROR If 'aSet' contains negative values or
 *             if it does not fit into a non-negative integer.
 */
intType setSConv (const const_setType aSet)

  {
    intType number;

  /* setSConv */
    if (likely(aSet->min_position == 0 && aSet->max_position == 0)) {
#if BITSETTYPE_SIZE > INTTYPE_SIZE
      if (unlikely(aSet->bitset[0] >> INTTYPE_SIZE != 0)) {
        logError(printf("setSConv(): "
                        "Set does not fit into an integer.\n"););
        raise_error(RANGE_ERROR);
        return 0;
      } /* if */
#endif
      number = (intType) aSet->bitset[0];
#if BITSETTYPE_SIZE >= INTTYPE_SIZE
      if (unlikely(number < 0)) {
        logError(printf("setSConv(): "
                        "Set does not fit into a non-negative integer.\n"););
        raise_error(RANGE_ERROR);
        return 0;
      } /* if */
#endif
    } else {
      logError(printf("setSConv(): "
                      "Set contains negative values or does not fit into an integer.\n"););
      raise_error(RANGE_ERROR);
      return 0;
    } /* if */
    return number;
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
    setType symDiff;

  /* setSymdiff */
    logFunction(printf("setSymdiff(\n");
                printSet(set1);
                printf(",\n");
                printSet(set2);
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
        !ALLOC_SET(symDiff, (uintType) (max_position - min_position + 1)))) {
      raise_error(MEMORY_ERROR);
      symDiff = NULL;
    } else {
      symDiff->min_position = min_position;
      symDiff->max_position = max_position;
      if (set1->max_position < set2->min_position ||
          set1->min_position > set2->max_position) {
        memcpy(&symDiff->bitset[set1->min_position - min_position], set1->bitset,
               bitsetSize(set1) * sizeof(bitSetType));
        memcpy(&symDiff->bitset[set2->min_position - min_position], set2->bitset,
               bitsetSize(set2) * sizeof(bitSetType));
        memset(&symDiff->bitset[stop_position - min_position + 1], 0,
               (size_t) (uintType) (start_position - stop_position - 1) *
               sizeof(bitSetType));
      } else {
        if (set2->min_position > set1->min_position) {
          memcpy(&symDiff->bitset[set1->min_position - min_position], set1->bitset,
                 (size_t) (uintType) (set2->min_position - set1->min_position) *
                 sizeof(bitSetType));
        } else {
          memcpy(&symDiff->bitset[set2->min_position - min_position], set2->bitset,
                 (size_t) (uintType) (set1->min_position - set2->min_position) *
                 sizeof(bitSetType));
        } /* if */
        if (set2->max_position > set1->max_position) {
          memcpy(&symDiff->bitset[set1->max_position - min_position + 1],
                 &set2->bitset[set1->max_position - set2->min_position + 1],
                 (size_t) (uintType) (set2->max_position - set1->max_position) *
                 sizeof(bitSetType));
        } else {
          memcpy(&symDiff->bitset[set2->max_position - min_position + 1],
                 &set1->bitset[set2->max_position - set1->min_position + 1],
                 (size_t) (uintType) (set1->max_position - set2->max_position) *
                 sizeof(bitSetType));
        } /* if */
        for (position = start_position; position <= stop_position; position++) {
          symDiff->bitset[position - min_position] =
              set1->bitset[position - set1->min_position] ^
              set2->bitset[position - set2->min_position];
        } /* for */
      } /* if */
    } /* if */
    logFunction(printf("setSymdiff --> ");
                printSet(symDiff);
                printf("\n"););
    return symDiff;
  } /* setSymdiff */



/**
 *  Get 64 bits from a bitset starting with lowestBitNum.
 *  This function is used by the action BIN_GET_BINARY_FROM_SET.
 *  @return a bit pattern with 64 bits from set1.
 */
uintType setToUInt (const const_setType set1, const intType lowestBitNum)

  {
    intType position;
    memSizeType bitset_index;
    unsigned int bit_index;
    uintType bitPattern;

  /* setToUInt */
    logFunction(printf("setToUInt(");
                printSet(set1);
                printf(", " FMT_D ")\n", lowestBitNum););
    position = bitset_pos(lowestBitNum);
    if (position >= set1->min_position && position <= set1->max_position) {
      bitset_index = bitsetIndex(set1, position);
      bit_index = ((unsigned int) lowestBitNum) & bitset_mask;
      if (bit_index == 0) {
        bitPattern = (uintType) set1->bitset[bitset_index];
      } else if (position < set1->max_position) {
        bitPattern = (uintType) (set1->bitset[bitset_index] >> bit_index |
            set1->bitset[bitset_index + 1] << (CHAR_BIT * sizeof(bitSetType) - bit_index));
      } else {
        bitPattern = (uintType) (set1->bitset[bitset_index] >> bit_index);
      } /* if */
    } else if (position == set1->min_position - 1) {
      bitset_index = bitsetIndex(set1, position);
      bit_index = ((unsigned int) lowestBitNum) & bitset_mask;
      if (bit_index == 0) {
        bitPattern = 0;
      } else {
        bitPattern = (uintType)
            (set1->bitset[bitset_index + 1] << (CHAR_BIT * sizeof(bitSetType) - bit_index));
      } /* if */
    } else {
      bitPattern = 0;
    } /* if */
    logFunction(printf("setToUInt --> 0x" F_X(016) "\n", bitPattern););
    return bitPattern;
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
    setType unionOfSets;

  /* setUnion */
    logFunction(printf("setUnion(\n");
                printSet(set1);
                printf(",\n");
                printSet(set2);
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
        !ALLOC_SET(unionOfSets, (uintType) (max_position - min_position + 1)))) {
      raise_error(MEMORY_ERROR);
      unionOfSets = NULL;
    } else {
      unionOfSets->min_position = min_position;
      unionOfSets->max_position = max_position;
      if (set1->max_position < set2->min_position ||
          set1->min_position > set2->max_position) {
        memcpy(&unionOfSets->bitset[set1->min_position - min_position], set1->bitset,
               bitsetSize(set1) * sizeof(bitSetType));
        memcpy(&unionOfSets->bitset[set2->min_position - min_position], set2->bitset,
               bitsetSize(set2) * sizeof(bitSetType));
        memset(&unionOfSets->bitset[stop_position - min_position + 1], 0,
               (size_t) (uintType) (start_position - stop_position - 1) *
               sizeof(bitSetType));
      } else {
        if (set2->min_position > set1->min_position) {
          memcpy(&unionOfSets->bitset[set1->min_position - min_position], set1->bitset,
                 (size_t) (uintType) (set2->min_position - set1->min_position) *
                 sizeof(bitSetType));
        } else {
          memcpy(&unionOfSets->bitset[set2->min_position - min_position], set2->bitset,
                 (size_t) (uintType) (set1->min_position - set2->min_position) *
                 sizeof(bitSetType));
        } /* if */
        if (set2->max_position > set1->max_position) {
          memcpy(&unionOfSets->bitset[set1->max_position - min_position + 1],
                 &set2->bitset[set1->max_position - set2->min_position + 1],
                 (size_t) (uintType) (set2->max_position - set1->max_position) *
                 sizeof(bitSetType));
        } else {
          memcpy(&unionOfSets->bitset[set2->max_position - min_position + 1],
                 &set1->bitset[set2->max_position - set1->min_position + 1],
                 (size_t) (uintType) (set1->max_position - set2->max_position) *
                 sizeof(bitSetType));
        } /* if */
        for (position = start_position; position <= stop_position; position++) {
          unionOfSets->bitset[position - min_position] =
              set1->bitset[position - set1->min_position] |
              set2->bitset[position - set2->min_position];
        } /* for */
      } /* if */
    } /* if */
    logFunction(printf("setUnion --> ");
                printSet(unionOfSets);
                printf("\n"););
    return unionOfSets;
  } /* setUnion */



/**
 *  Assign the union of *dest and delta to *dest.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void setUnionAssign (setType *const dest, const const_setType delta)

  {
    setType set1;
    intType position;
    intType min_position;
    intType max_position;
    intType start_position;
    intType stop_position;
    setType new_dest;

  /* setUnionAssign */
    logFunction(printf("setUnionAssign(\n");
                printSet(*dest);
                printf(",\n");
                printSet(delta);
                printf(")\n"););
    set1 = *dest;
    if (set1->min_position < delta->min_position) {
      min_position = set1->min_position;
      start_position = delta->min_position;
    } else {
      min_position = delta->min_position;
      start_position = set1->min_position;
    } /* if */
    if (set1->max_position > delta->max_position) {
      max_position = set1->max_position;
      stop_position = delta->max_position;
    } else {
      max_position = delta->max_position;
      stop_position = set1->max_position;
    } /* if */
    if (set1->min_position == min_position &&
        set1->max_position == max_position) {
      for (position = start_position; position <= stop_position; position++) {
        set1->bitset[position - min_position] |=
            delta->bitset[position - delta->min_position];
      } /* for */
    } else {
      if (unlikely((uintType) (max_position - min_position + 1) > MAX_SET_LEN ||
          !ALLOC_SET(new_dest, (uintType) (max_position - min_position + 1)))) {
        raise_error(MEMORY_ERROR);
      } else {
        new_dest->min_position = min_position;
        new_dest->max_position = max_position;
        if (set1->max_position < delta->min_position ||
            set1->min_position > delta->max_position) {
          memcpy(&new_dest->bitset[set1->min_position - min_position], set1->bitset,
                 bitsetSize(set1) * sizeof(bitSetType));
          memcpy(&new_dest->bitset[delta->min_position - min_position], delta->bitset,
                 bitsetSize(delta) * sizeof(bitSetType));
          memset(&new_dest->bitset[stop_position - min_position + 1], 0,
                 (size_t) (uintType) (start_position - stop_position - 1) *
                 sizeof(bitSetType));
        } else {
          if (delta->min_position > set1->min_position) {
            memcpy(&new_dest->bitset[set1->min_position - min_position], set1->bitset,
                   (size_t) (uintType) (delta->min_position - set1->min_position) *
                   sizeof(bitSetType));
          } else {
            memcpy(&new_dest->bitset[delta->min_position - min_position], delta->bitset,
                   (size_t) (uintType) (set1->min_position - delta->min_position) *
                   sizeof(bitSetType));
          } /* if */
          if (delta->max_position > set1->max_position) {
            memcpy(&new_dest->bitset[set1->max_position - min_position + 1],
                   &delta->bitset[set1->max_position - delta->min_position + 1],
                   (size_t) (uintType) (delta->max_position - set1->max_position) *
                   sizeof(bitSetType));
          } else {
            memcpy(&new_dest->bitset[delta->max_position - min_position + 1],
                   &set1->bitset[delta->max_position - set1->min_position + 1],
                   (size_t) (uintType) (set1->max_position - delta->max_position) *
                   sizeof(bitSetType));
          } /* if */
          for (position = start_position; position <= stop_position; position++) {
            new_dest->bitset[position - min_position] =
                set1->bitset[position - set1->min_position] |
                delta->bitset[position - delta->min_position];
          } /* for */
        } /* if */
        *dest = new_dest;
        FREE_SET(set1, bitsetSize(set1));
      } /* if */
    } /* if */
    logFunction(printf("setUnionAssign --> ");
                printSet(*dest);
                printf("\n"););
 } /* setUnionAssign */
