/********************************************************************/
/*                                                                  */
/*  hsh_rtl.c     Primitive actions for the hash map type.          */
/*  Copyright (C) 1989 - 2011, 2013 - 2016, 2018  Thomas Mertes     */
/*                2021, 2024  Thomas Mertes                         */
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
/*  File: seed7/src/hsh_rtl.c                                       */
/*  Changes: 2005, 2006, 2007, 2013, 2016, 2018  Thomas Mertes      */
/*           2021, 2024  Thomas Mertes                              */
/*  Content: Primitive actions for the hash map type.               */
/*                                                                  */
/*  The functions from this file should only be used in compiled    */
/*  Seed7 programs. The interpreter should not use functions of     */
/*  this file.                                                      */
/*                                                                  */
/*  The functions in this file use type declarations from the       */
/*  include file data_rtl.h instead of data.h. Therefore the types  */
/*  rtlHashElemType and rtlHashType are declared different from the */
/*  types hashElemType and hashType in the interpreter.             */
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
#include "striutl.h"
#include "rtl_err.h"
#include "int_rtl.h"

#undef EXTERN
#define EXTERN
#include "hsh_rtl.h"


#define TABLE_BITS 10
#define TABLE_SIZE(bits) ((unsigned int) 1 << (bits))
#define TABLE_MASK(bits) (TABLE_SIZE(bits)-1)



static memSizeType free_helem (const const_rtlHashElemType old_helem,
    const destrFuncType key_destr_func, const destrFuncType data_destr_func)

  {
    memSizeType freed = 1;

  /* free_helem */
    key_destr_func(old_helem->key.value.genericValue);
    data_destr_func(old_helem->data.value.genericValue);
    if (old_helem->next_less != NULL) {
      freed += free_helem(old_helem->next_less, key_destr_func,
                         data_destr_func);
    } /* if */
    if (old_helem->next_greater != NULL) {
      freed += free_helem(old_helem->next_greater, key_destr_func,
                         data_destr_func);
    } /* if */
    FREE_RECORD(old_helem, rtlHashElemRecord, count.rtl_helem);
    return freed;
  } /* free_helem */



static void free_hash (const const_rtlHashType old_hash,
    const destrFuncType key_destr_func, const destrFuncType data_destr_func)

  {
    memSizeType to_free;
    unsigned int number;
    const rtlHashElemType *table;

  /* free_hash */
    if (old_hash != NULL) {
      to_free = old_hash->size;
      if (to_free != 0) {
        number = old_hash->table_size;
        table = old_hash->table;
        do {
          do {
            number--;
          } while (table[number] == NULL);
          to_free -= free_helem(table[number], key_destr_func, data_destr_func);
        } while (to_free != 0);
      } /* if */
      FREE_RTL_HASH(old_hash, old_hash->table_size);
    } /* if */
  } /* free_hash */



static rtlHashElemType new_helem (genericType key, genericType data,
    const createFuncType key_create_func, const createFuncType data_create_func,
    errInfoType *err_info)

  {
    rtlHashElemType helem;

  /* new_helem */
    /* printf("new_helem(" FMT_U_GEN ", " FMT_U_GEN ")\n", key, data); */
    if (unlikely(!ALLOC_RECORD(helem, rtlHashElemRecord, count.rtl_helem))) {
      *err_info = MEMORY_ERROR;
    } else {
      helem->key.value.genericValue = key_create_func(key);
      helem->data.value.genericValue = data_create_func(data);
      helem->next_less = NULL;
      helem->next_greater = NULL;
      /* printf("new_helem(" FMT_U_GEN ", " FMT_U_GEN ")\n",
          helem->key.value.genericValue,
          helem->data.value.genericValue); */
    } /* if */
    return helem;
  } /* new_helem */



static rtlHashType new_hash (unsigned int bits)

  {
    rtlHashType hash;

  /* new_hash */
    if (likely(ALLOC_RTL_HASH(hash, TABLE_SIZE(bits)))) {
      hash->bits = bits;
      hash->mask = TABLE_MASK(bits);
      hash->table_size = TABLE_SIZE(bits);
      hash->size = 0;
      memset(hash->table, 0, hash->table_size * sizeof(rtlHashElemType));
    } /* if */
    return hash;
  } /* new_hash */



static rtlHashElemType create_helem (const const_rtlHashElemType source_helem,
    const createFuncType key_create_func, const createFuncType data_create_func,
    errInfoType *err_info)

  {
    rtlHashElemType dest_helem;

  /* create_helem */
    if (unlikely(!ALLOC_RECORD(dest_helem, rtlHashElemRecord, count.rtl_helem))) {
      *err_info = MEMORY_ERROR;
    } else {
      dest_helem->key.value.genericValue =
          key_create_func(source_helem->key.value.genericValue);
      dest_helem->data.value.genericValue =
          data_create_func(source_helem->data.value.genericValue);
      if (source_helem->next_less != NULL) {
        dest_helem->next_less = create_helem(source_helem->next_less,
            key_create_func, data_create_func, err_info);
      } else {
        dest_helem->next_less = NULL;
      } /* if */
      if (source_helem->next_greater != NULL) {
        dest_helem->next_greater = create_helem(source_helem->next_greater,
            key_create_func, data_create_func, err_info);
      } else {
        dest_helem->next_greater = NULL;
      } /* if */
    } /* if */
    return dest_helem;
  } /* create_helem */



static rtlHashType create_hash (const const_rtlHashType source_hash,
    const createFuncType key_create_func, const createFuncType data_create_func,
    errInfoType *err_info)

  {
    unsigned int table_size;
    unsigned int number;
    const rtlHashElemType *source_helem;
    rtlHashElemType *dest_helem;
    rtlHashType dest_hash;

  /* create_hash */
    table_size = source_hash->table_size;
    if (unlikely(!ALLOC_RTL_HASH(dest_hash, table_size))) {
      *err_info = MEMORY_ERROR;
    } else {
      dest_hash->bits = source_hash->bits;
      dest_hash->mask = source_hash->mask;
      dest_hash->table_size = table_size;
      dest_hash->size = source_hash->size;
      if (source_hash->size == 0) {
        memset(dest_hash->table, 0, table_size * sizeof(rtlHashElemType));
      } else {
        number = table_size;
        source_helem = &source_hash->table[0];
        dest_helem = &dest_hash->table[0];
        while (number > 0) {
          while (number > 0 && *source_helem == NULL) {
            *dest_helem = NULL;
            number--;
            source_helem++;
            dest_helem++;
          } /* while */
          if (number > 0 && *source_helem != NULL) {
            *dest_helem = create_helem(*source_helem, key_create_func,
                                       data_create_func, err_info);
            number--;
            source_helem++;
            dest_helem++;
          } /* if */
        } /* while */
      } /* if */
    } /* if */
    return dest_hash;
  } /* create_hash */



static void copy_hash (const rtlHashType dest_hash, const const_rtlHashType source_hash,
    const createFuncType key_create_func, const createFuncType data_create_func,
    const destrFuncType key_destr_func, const destrFuncType data_destr_func,
    errInfoType *err_info)

  {
    unsigned int number;
    const rtlHashElemType *source_helem;
    rtlHashElemType *dest_helem;

  /* copy_hash */
    logFunction(printf("copy_hash(" FMT_X_MEM ", " FMT_X_MEM ")\n",
                       (memSizeType) dest_hash, (memSizeType) source_hash););
    dest_hash->bits = source_hash->bits;
    dest_hash->mask = source_hash->mask;
    dest_hash->size = source_hash->size;
    number = source_hash->table_size;
    source_helem = &source_hash->table[0];
    dest_helem = &dest_hash->table[0];
    while (number > 0 && *err_info == OKAY_NO_ERROR) {
      if (*dest_helem != NULL) {
        free_helem(*dest_helem, key_destr_func, data_destr_func);
      } /* if */
      if (*source_helem != NULL) {
        *dest_helem = create_helem(*source_helem, key_create_func, data_create_func,
                                   err_info);
      } else {
        *dest_helem = NULL;
      } /* if */
      number--;
      source_helem++;
      dest_helem++;
    } /* while */
    logFunction(printf("copy_hash -->\n"););
  } /* copy_hash */



static memSizeType keys_helem (const rtlArrayType key_array,
    memSizeType arr_pos, const_rtlHashElemType curr_helem,
    const createFuncType key_create_func)

  { /* keys_helem */
    do {
      arr_pos--;
      key_array->arr[arr_pos].value.genericValue =
          key_create_func(curr_helem->key.value.genericValue);
      if (curr_helem->next_less != NULL) {
        arr_pos = keys_helem(key_array, arr_pos, curr_helem->next_less,
                             key_create_func);
      } /* if */
      curr_helem = curr_helem->next_greater;
    } while (curr_helem != NULL);
    return arr_pos;
  } /* keys_helem */



static inline rtlArrayType keys_hash (const const_rtlHashType curr_hash,
    const createFuncType key_create_func)

  {
    memSizeType arr_pos;
    memSizeType number;
    const rtlHashElemType *table;
    rtlArrayType key_array;

  /* keys_hash */
    if (unlikely(curr_hash->size > INTTYPE_MAX ||
                 !ALLOC_RTL_ARRAY(key_array, curr_hash->size))) {
      raise_error(MEMORY_ERROR);
      key_array = NULL;
    } else {
      key_array->min_position = 1;
      key_array->max_position = (intType) curr_hash->size;
      if (curr_hash->size != 0) {
        arr_pos = curr_hash->size;
        number = curr_hash->table_size;
        table = curr_hash->table;
        do {
          do {
            number--;
          } while (table[number] == NULL);
          arr_pos = keys_helem(key_array, arr_pos, table[number],
                               key_create_func);
        } while (arr_pos != 0);
      } /* if */
    } /* if */
    return key_array;
  } /* keys_hash */



static memSizeType values_helem (const rtlArrayType value_array,
    memSizeType arr_pos, const_rtlHashElemType curr_helem,
    const createFuncType value_create_func)

  { /* values_helem */
    do {
      arr_pos--;
      value_array->arr[arr_pos].value.genericValue =
          value_create_func(curr_helem->data.value.genericValue);
      if (curr_helem->next_less != NULL) {
        arr_pos = values_helem(value_array, arr_pos, curr_helem->next_less,
                               value_create_func);
      } /* if */
      curr_helem = curr_helem->next_greater;
    } while (curr_helem != NULL);
    return arr_pos;
  } /* values_helem */



static inline rtlArrayType values_hash (const const_rtlHashType curr_hash,
    const createFuncType value_create_func)

  {
    memSizeType arr_pos;
    memSizeType number;
    const rtlHashElemType *table;
    rtlArrayType value_array;

  /* values_hash */
    if (unlikely(curr_hash->size > INTTYPE_MAX ||
                 !ALLOC_RTL_ARRAY(value_array, curr_hash->size))) {
      raise_error(MEMORY_ERROR);
      value_array = NULL;
    } else {
      value_array->min_position = 1;
      value_array->max_position = (intType) curr_hash->size;
      if (curr_hash->size != 0) {
        arr_pos = curr_hash->size;
        number = curr_hash->table_size;
        table = curr_hash->table;
        do {
          do {
            number--;
          } while (table[number] == NULL);
          arr_pos = values_helem(value_array, arr_pos, table[number],
                                 value_create_func);
        } while (arr_pos != 0);
      } /* if */
    } /* if */
    return value_array;
  } /* values_hash */



static memSizeType get_helem_elem (const_rtlHashElemType *hash_elem,
    memSizeType arr_pos, const_rtlHashElemType curr_helem)

  { /* get_helem_elem */
    do {
      arr_pos--;
      if (arr_pos == 0) {
        *hash_elem = curr_helem;
      } else {
        if (curr_helem->next_less != NULL) {
          arr_pos = get_helem_elem(hash_elem, arr_pos, curr_helem->next_less);
        } /* if */
        curr_helem = curr_helem->next_greater;
      } /* if */
    } while (curr_helem != NULL && arr_pos != 0);
    return arr_pos;
  } /* get_helem_elem */



static inline const_rtlHashElemType get_hash_elem (const const_rtlHashType curr_hash,
    memSizeType arr_pos)

  {
    memSizeType number;
    const rtlHashElemType *table;
    const_rtlHashElemType hash_elem = NULL;

  /* get_hash_elem */
    if (arr_pos >= 1 && arr_pos <= curr_hash->size) {
      number = curr_hash->table_size;
      table = curr_hash->table;
      do {
        do {
          number--;
        } while (table[number] == NULL);
        arr_pos = get_helem_elem(&hash_elem, arr_pos, table[number]);
      } while (arr_pos != 0);
    } /* if */
    return hash_elem;
  } /* get_hash_elem */



#ifdef OUT_OF_ORDER
static void dump_helem (const_rtlHashElemType curr_helem)

  { /* dump_helem */
    printf("key: %ld, %lx, %f / value: %ld, %lx, %f\n",
           curr_helem->key.value.intValue,
           curr_helem->key.value.genericValue,
           curr_helem->key.value.floatValue,
           curr_helem->data.value.intValue,
           curr_helem->data.value.genericValue,
           curr_helem->data.value.floatValue);
    if (curr_helem->next_less != NULL) {
      dump_helem(curr_helem->next_less);
    } /* if */
    if (curr_helem->next_greater != NULL) {
      dump_helem(curr_helem->next_greater);
    } /* if */
  } /* dump_helem */



static void dump_hash (const const_rtlHashType curr_hash)

  {
    memSizeType number;
    const rtlHashElemType *curr_helem;

  /* dump_hash */
    number = curr_hash->table_size;
    curr_helem = &curr_hash->table[0];
    while (number > 0) {
      if (*curr_helem != NULL) {
        dump_helem(*curr_helem);
      } /* if */
      number--;
      curr_helem++;
    } /* while */
  } /* dump_hash */
#endif



rtlHashElemType hshConcatKeyValue (rtlHashElemType element1,
    rtlHashElemType element2)

  {
    rtlHashElemType lastElement1;
    rtlHashElemType lastElement2;

  /* hshConcatKeyValue */
    logFunction(printf("hshConcatKeyValue(" FMT_X_MEM ", " FMT_X_MEM ")\n",
                       (memSizeType) element1, (memSizeType) element2););
    /* The concatenation uses the pointers of a hash element in a   */
    /* special way. This allows the creation of hash element lists. */
    /* next_greater ... Pointer to the next element in the list.    */
    /* next_less    ... Pointer to the last element of the list.    */
    /* Next_less is only set in the first element of the list.      */
    /* In all other elements next_less is NULL.                     */
    if (element1->next_less != NULL) {
      lastElement1 = element1->next_less;
    } else {
      lastElement1 = element1;
    } /* if */
    if (element2->next_less != NULL) {
      lastElement2 = element2->next_less;
      element2->next_less = NULL;
    } else {
      lastElement2 = element2;
    } /* if */
    lastElement1->next_greater = element2;
    element1->next_less = lastElement2;
    logFunction(printf("hshConcatKeyValue --> " FMT_X_MEM "\n",
                       (memSizeType) element1););
    return element1;
  } /* hshConcatKeyValue */



/**
 *  Hash membership test.
 *  Determine if 'aKey' is a member of the hash map 'aHashMap'.
 *  @return TRUE if 'aKey' is a member of 'aHashMap',
 *          FALSE otherwise.
 */
boolType hshContains (const const_rtlHashType aHashMap, const genericType aKey,
    intType hashcode, compareType cmp_func)

  {
    const_rtlHashElemType hashelem;
    intType cmp;
    boolType result = FALSE;

  /* hshContains */
    logFunction(printf("hshContains(" FMT_X_MEM ", " FMT_U_GEN ", " FMT_U ")\n",
                       (memSizeType) aHashMap, aKey, hashcode););
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
/*
printf("sizeof(hashelem->key.value.genericValue)=%lu\n",
    sizeof(hashelem->key.value.genericValue));
printf("sizeof(aKey)=%lu\n", sizeof(aKey));
printf("%llX\n", hashelem->key.value.genericValue);
printf("%lX\n", (long unsigned) hashelem->key.value.genericValue);
printf("%llX\n", aKey);
printf("%lX\n", (long unsigned) aKey);
*/
      cmp = cmp_func(hashelem->key.value.genericValue, aKey);
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (unlikely(cmp == 0)) {
        result = TRUE;
        hashelem = NULL;
      } else {
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
    logFunction(printf("hshContains(" FMT_X_MEM ", " FMT_U_GEN ", " FMT_U ") --> %d\n",
                       (memSizeType) aHashMap, aKey, hashcode, result););
    return result;
  } /* hshContains */



/**
 *  Assign source to *dest.
 *  A copy function assumes that *dest contains a legal value.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void hshCpy (rtlHashType *const dest, const const_rtlHashType source,
    const createFuncType key_create_func, const destrFuncType key_destr_func,
    const createFuncType data_create_func, const destrFuncType data_destr_func)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* hshCpy */
    logFunction(printf("hshCpy(" FMT_X_MEM ", " FMT_X_MEM ")\n",
                       (memSizeType) *dest, (memSizeType) source););
    if ((*dest)->table_size == source->table_size) {
      /* The following check avoids an error for: aHash := aHash; */
      if (*dest != source) {
        copy_hash(*dest, source,
            key_create_func, data_create_func,
            key_destr_func, data_destr_func, &err_info);
      } /* if */
    } else {
      free_hash(*dest, key_destr_func, data_destr_func);
      *dest = create_hash(source,
          key_create_func, data_create_func, &err_info);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      free_hash(*dest, key_destr_func, data_destr_func);
      *dest = NULL;
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("hshCpy -->\n"););
  } /* hshCpy */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
rtlHashType hshCreate (const const_rtlHashType source,
    const createFuncType key_create_func, const destrFuncType key_destr_func,
    const createFuncType data_create_func, const destrFuncType data_destr_func)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    rtlHashType result;

  /* hshCreate */
    result = create_hash(source,
        key_create_func, data_create_func, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      free_hash(result, key_destr_func, data_destr_func);
      result = NULL;
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* hshCreate */



/**
 *  Free the memory referred by 'old_hash'.
 *  After hshDestr is left 'old_hash' refers to not existing memory.
 *  The memory where 'old_hash' is stored can be freed afterwards.
 */
void hshDestr (const const_rtlHashType old_hash, const destrFuncType key_destr_func,
    const destrFuncType data_destr_func)

  { /* hshDestr */
    logFunction(printf("hshDestr(" FMT_X_MEM ")\n", (memSizeType) old_hash););
    free_hash(old_hash, key_destr_func, data_destr_func);
    logFunction(printf("hshDestr -->\n"););
  } /* hshDestr */



/**
 *  Create an empty hash table.
 *  @return an empty hash table.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
rtlHashType hshEmpty (void)

  {
    rtlHashType result;

  /* hshEmpty */
    result = new_hash(TABLE_BITS);
    if (unlikely(result == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* hshEmpty */



/**
 *  Remove the element with the key 'aKey' from the hash map 'aHashMap'.
 */
void hshExcl (const rtlHashType aHashMap, const genericType aKey,
    intType hashcode, compareType cmp_func, const destrFuncType key_destr_func,
    const destrFuncType data_destr_func)

  {
    rtlHashElemType *delete_pos;
    rtlHashElemType hashelem;
    rtlHashElemType greater_hashelems;
    rtlHashElemType old_hashelem;
    intType cmp;

  /* hshExcl */
    logFunction(printf("hshExcl(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U ") size=" FMT_U_MEM "\n",
                       (memSizeType) aHashMap, aKey, hashcode, aHashMap->size););
    delete_pos = &aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericValue, aKey);
      if (cmp < 0) {
        delete_pos = &hashelem->next_less;
        hashelem = hashelem->next_less;
      } else if (unlikely(cmp == 0)) {
        old_hashelem = hashelem;
        if (hashelem->next_less == NULL) {
          *delete_pos = hashelem->next_greater;
        } else if (hashelem->next_greater == NULL) {
          *delete_pos = hashelem->next_less;
        } else {
          *delete_pos = hashelem->next_less;
          greater_hashelems = hashelem->next_greater;
          hashelem = hashelem->next_less;
          while (hashelem->next_greater != NULL) {
            hashelem = hashelem->next_greater;
          } /* while */
          hashelem->next_greater = greater_hashelems;
        } /* if */
        old_hashelem->next_less = NULL;
        old_hashelem->next_greater = NULL;
        free_helem(old_hashelem, key_destr_func, data_destr_func);
        aHashMap->size--;
        hashelem = NULL;
      } else {
        delete_pos = &hashelem->next_greater;
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
    logFunction(printf("hshExcl(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U ") size=" FMT_U_MEM " -->\n",
                       (memSizeType) aHashMap, aKey, hashcode, aHashMap->size););
  } /* hshExcl */



rtlHashType hshGenHash (rtlHashElemType keyValuePairs,
    const hashCodeFuncType key_hash_code_func, compareType cmp_func,
    const destrFuncType key_destr_func, const destrFuncType data_destr_func)

  {
    rtlHashElemType currentKeyValue;
    unsigned int hashCode;
    rtlHashElemType hashElem;
    intType cmp;
    errInfoType err_info = OKAY_NO_ERROR;
    rtlHashType aHashMap;

  /* hshGenHash */
    logFunction(printf("hshGenHash(" FMT_X_MEM ", ...)\n",
                       (memSizeType) keyValuePairs););
    aHashMap = new_hash(TABLE_BITS);
    if (unlikely(aHashMap == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      while (keyValuePairs != NULL) {
        currentKeyValue = keyValuePairs;
        keyValuePairs = keyValuePairs->next_greater;
        currentKeyValue->next_less = NULL;
        currentKeyValue->next_greater = NULL;
        hashCode = (unsigned int) key_hash_code_func(
            currentKeyValue->key.value.genericValue);
        hashElem = aHashMap->table[hashCode & aHashMap->mask];
        if (hashElem == NULL) {
          aHashMap->table[hashCode & aHashMap->mask] = currentKeyValue;
          aHashMap->size++;
        } else {
          do {
            cmp = cmp_func(hashElem->key.value.genericValue,
                           currentKeyValue->key.value.genericValue);
            if (cmp < 0) {
              if (hashElem->next_less == NULL) {
                hashElem->next_less = currentKeyValue;
                aHashMap->size++;
                hashElem = NULL;
              } else {
                hashElem = hashElem->next_less;
              } /* if */
            } else if (cmp == 0) {
              logError(printf("hshGenHash: A key is used twice.\n"););
              key_destr_func(currentKeyValue->key.value.genericValue);
              data_destr_func(currentKeyValue->data.value.genericValue);
              FREE_RECORD(currentKeyValue, rtlHashElemRecord, count.rtl_helem);
              err_info = RANGE_ERROR;
              hashElem = NULL;
            } else {
              if (hashElem->next_greater == NULL) {
                hashElem->next_greater = currentKeyValue;
                aHashMap->size++;
                hashElem = NULL;
              } else {
                hashElem = hashElem->next_greater;
              } /* if */
            } /* if */
          } while (hashElem != NULL);
        } /* if */
      } /* while */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        free_hash(aHashMap, key_destr_func, data_destr_func);
        raise_error(err_info);
        aHashMap = NULL;
      } /* if */
    } /* if */
    return aHashMap;
  } /* hshGenHash */



rtlHashElemType hshGenKeyValue (const genericType aKey, const genericType aValue)

  {
    rtlHashElemType keyValue;

  /* hshGenKeyValue */
    logFunction(printf("hshGenKeyValue(" FMT_X_MEM ", " FMT_X_MEM ")\n",
                       (memSizeType) aKey, (memSizeType) aValue););
    if (unlikely(!ALLOC_RECORD(keyValue, rtlHashElemRecord, count.rtl_helem))) {
      raise_error(MEMORY_ERROR);
    } else {
      keyValue->next_less = NULL;
      keyValue->next_greater = NULL;
      keyValue->key.value.genericValue = aKey;
      keyValue->data.value.genericValue = aValue;
    } /* if */
    logFunction(printf("hshGenKeyValue --> " FMT_X_MEM ")\n",
                       (memSizeType) keyValue););
    return keyValue;
  } /* hshGenKeyValue */



/**
 *  Access one value from the hash table 'aHashMap'.
 *  @return the element with the key 'aKey' from 'aHashMap'.
 *  @exception INDEX_ERROR If 'aHashMap' does not have an element
 *             with the key 'aKey'.
 */
genericType hshIdx (const const_rtlHashType aHashMap, const genericType aKey,
    intType hashcode, compareType cmp_func)

  {
    rtlHashElemType hashelem;
    rtlHashElemType result_hashelem = NULL;
    intType cmp;
    genericType result;

  /* hshIdx */
    logFunction(printf("hshIdx(" FMT_X_MEM ", " FMT_U_GEN ", " FMT_U ")\n",
                       (memSizeType) aHashMap, aKey, hashcode););
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericValue, aKey);
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (unlikely(cmp == 0)) {
        result_hashelem = hashelem;
        hashelem = NULL;
      } else {
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
    if (unlikely(result_hashelem == NULL)) {
      logError(printf("hshIdx(" FMT_X_MEM ", " FMT_U_GEN ", " FMT_U "): "
                      "Hashmap does not have an element with the key.\n",
                      (memSizeType) aHashMap, aKey, hashcode););
      raise_error(INDEX_ERROR);
      result = 0;
    } else {
      result = result_hashelem->data.value.genericValue;
    } /* if */
    logFunction(printf("hshIdx(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U ") --> " FMT_X_GEN " (" FMT_X_GEN ")\n",
                       (memSizeType) aHashMap, aKey, hashcode, result,
                       (result != 0 ?
                           ((const_rtlObjectType *) &result)->value.genericValue :
                           (genericType) 0)););
    return result;
  } /* hshIdx */



/**
 *  Determine the address of a value from the hash table 'aHashMap'.
 *  @return the address of the element with the key 'aKey' from 'aHashMap'.
 *  @exception INDEX_ERROR If 'aHashMap' does not have an element
 *             with the key 'aKey'.
 */
rtlObjectType *hshIdxAddr (const const_rtlHashType aHashMap,
    const genericType aKey, intType hashcode, compareType cmp_func)

  {
    rtlHashElemType hashelem;
    rtlHashElemType result_hashelem = NULL;
    intType cmp;
    rtlObjectType *result;

  /* hshIdxAddr */
    logFunction(printf("hshIdxAddr(" FMT_X_MEM ", " FMT_U_GEN ", " FMT_U ")\n",
                       (memSizeType) aHashMap, aKey, hashcode););
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericValue, aKey);
      /* printf(". %llu %llu cmp=%d\n",
          (unsigned long long) hashelem->key.value.genericValue,
          (unsigned long long) aKey, cmp); */
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (unlikely(cmp == 0)) {
        result_hashelem = hashelem;
        hashelem = NULL;
      } else {
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
    if (unlikely(result_hashelem == NULL)) {
      logError(printf("hshIdxAddr(" FMT_X_MEM ", " FMT_U_GEN ", " FMT_U "): "
                      "Hashmap does not have an element with the key.\n",
                      (memSizeType) aHashMap, aKey, hashcode););
      raise_error(INDEX_ERROR);
      result = NULL;
    } else {
      result = &result_hashelem->data;
    } /* if */
    logFunction(printf("hshIdxAddr(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U ") --> " FMT_U_MEM " (" FMT_X_GEN ", %f)\n",
                       (memSizeType) aHashMap, aKey, hashcode, (memSizeType) result,
                       result != NULL ? result->value.genericValue : (genericType) 0,
                       result != NULL ? result->value.floatValue : 0.0););
    return result;
  } /* hshIdxAddr */



/**
 *  Determine the address of a value from the hash table 'aHashMap'.
 *  @return the address of the element with the key 'aKey' from 'aHashMap' or
 *          NULL if 'aHashMap' does not have an element with the key 'aKey'.
 */
rtlObjectType *hshIdxAddr2 (const const_rtlHashType aHashMap,
    const genericType aKey, intType hashcode, compareType cmp_func)

  {
    rtlHashElemType hashelem;
    rtlHashElemType result_hashelem = NULL;
    intType cmp;
    rtlObjectType *result;

  /* hshIdxAddr2 */
    logFunction(printf("hshIdxAddr2(" FMT_X_MEM ", " FMT_U_GEN ", " FMT_U ")\n",
                       (memSizeType) aHashMap, aKey, hashcode););
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericValue, aKey);
      /* printf(". %llu %llu cmp=%d\n",
          (unsigned long long) hashelem->key.value.genericValue,
          (unsigned long long) aKey, cmp); */
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (unlikely(cmp == 0)) {
        result_hashelem = hashelem;
        hashelem = NULL;
      } else {
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
    if (result_hashelem != NULL) {
      result = &result_hashelem->data;
    } else {
      result = NULL;
    } /* if */
    logFunction(printf("hshIdxAddr2(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U ") --> " FMT_U_MEM " (" FMT_X_GEN ", %f)\n",
                       (memSizeType) aHashMap, aKey, hashcode, (memSizeType) result,
                       result != NULL ? result->value.genericValue : (genericType) 0,
                       result != NULL ? result->value.floatValue : 0.0););
    return result;
  } /* hshIdxAddr2 */



/**
 *  Search for 'aKey' in 'aHashMap'.
 *  If 'aKey' is element of 'aHashMap' the corresponding value is returned.
 *  If 'aKey' is not element of 'aHashMap' then 'defaultData' is stored
 *  as value of 'aKey' in 'aHashMap' and 'defaultData' is returned.
 *  @return the value stored for 'aKey' in 'aHashMap', or
 *          'defaultData', if 'aKey' is not a member of 'aHashMap'.
 */
genericType hshIdxEnterDefault (const rtlHashType aHashMap,
    const genericType aKey, const genericType defaultData, intType hashcode)

  {
    rtlHashElemType hashelem;
    rtlHashElemType result_hashelem;
    errInfoType err_info = OKAY_NO_ERROR;
    genericType result;

  /* hshIdxEnterDefault */
    logFunction(printf("hshIdxEnterDefault(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U_GEN ", " FMT_U ")\n",
                       (memSizeType) aHashMap, aKey, defaultData, hashcode););
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    if (hashelem == NULL) {
      result_hashelem = new_helem(aKey, defaultData,
          (createFuncType) &genericCreate,
          (createFuncType) &genericCreate, &err_info);
      aHashMap->table[(unsigned int) hashcode & aHashMap->mask] = result_hashelem;
      aHashMap->size++;
    } else {
      do {
        /* printf("key=%llX\n", hashelem->key.value.genericValue); */
        if (hashelem->key.value.genericValue < aKey) {
          if (hashelem->next_less == NULL) {
            result_hashelem = new_helem(aKey, defaultData,
                (createFuncType) &genericCreate,
                (createFuncType) &genericCreate, &err_info);
            hashelem->next_less = result_hashelem;
            aHashMap->size++;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_less;
          } /* if */
        } else if (unlikely(hashelem->key.value.genericValue == aKey)) {
          result_hashelem = hashelem;
          hashelem = NULL;
        } else {
          if (hashelem->next_greater == NULL) {
            result_hashelem = new_helem(aKey, defaultData,
                (createFuncType) &genericCreate,
                (createFuncType) &genericCreate, &err_info);
            hashelem->next_greater = result_hashelem;
            aHashMap->size++;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_greater;
          } /* if */
        } /* if */
      } while (hashelem != NULL);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      aHashMap->size--;
      raise_error(MEMORY_ERROR);
      result = 0;
    } else {
      result = result_hashelem->data.value.genericValue;
    } /* if */
    logFunction(printf("hshIdxEnterDefault(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U_GEN ", " FMT_U ") --> " FMT_U_GEN "\n",
                       (memSizeType) aHashMap, aKey, defaultData,
                       hashcode, result););
    return result;
  } /* hshIdxEnterDefault */



/**
 *  Search for 'aKey' in 'aHashMap'.
 *  If 'aKey' is element of 'aHashMap' the corresponding value is returned.
 *  If 'aKey' is not element of 'aHashMap' then 'defaultData' is returned.
 *  @return the value stored for 'aKey' in 'aHashMap', or
 *          'defaultData', if 'aKey' is not a member of 'aHashMap'.
 */
genericType hshIdxWithDefault (const const_rtlHashType aHashMap, const genericType aKey,
    const genericType defaultData, intType hashcode, compareType cmp_func)

  {
    rtlHashElemType hashelem;
    rtlHashElemType result_hashelem = NULL;
    intType cmp;
    genericType result;

  /* hshIdxWithDefault */
    logFunction(printf("hshIdxWithDefault(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U_GEN ", " FMT_U ")\n",
                       (memSizeType) aHashMap, aKey, defaultData, hashcode););
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericValue, aKey);
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (unlikely(cmp == 0)) {
        result_hashelem = hashelem;
        hashelem = NULL;
      } else {
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
    if (result_hashelem != NULL) {
      result = result_hashelem->data.value.genericValue;
    } else {
      result = defaultData;
    } /* if */
    logFunction(printf("hshIdxWithDefault(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U_GEN ", " FMT_U ") --> " FMT_U_GEN "\n",
                       (memSizeType) aHashMap, aKey, defaultData,
                       hashcode, result););
    return result;
  } /* hshIdxWithDefault */



/**
 *  Search for 'aKey' in 'aHashMap'.
 *  If 'aKey' is element of 'aHashMap' the corresponding value is returned.
 *  If 'aKey' is not element of 'aHashMap' then 0 is returned.
 *  @return the value stored for 'aKey' in 'aHashMap', or
 *          0, if 'aKey' is not a member of 'aHashMap'.
 */
genericType hshIdxDefault0 (const const_rtlHashType aHashMap, const genericType aKey,
    intType hashcode, compareType cmp_func)

  {
    rtlHashElemType hashelem;
    intType cmp;
    genericType result;

  /* hshIdxDefault0 */
    logFunction(printf("hshIdxDefault0(" FMT_X_MEM ", " FMT_U_GEN ", " FMT_U ")\n",
                       (memSizeType) aHashMap, aKey, hashcode););
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericValue, aKey);
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (unlikely(cmp == 0)) {
        result = hashelem->data.value.genericValue;
        logFunction(printf("hshIdxDefault0(" FMT_X_MEM ", " FMT_U_GEN ", "
                           FMT_U ") --> " FMT_U_GEN "\n",
                           (memSizeType) aHashMap, aKey, hashcode, result););
        return result;
      } else {
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
    logFunction(printf("hshIdxDefault0(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U ") --> 0\n",
                       (memSizeType) aHashMap, aKey, hashcode););
    return 0;
  } /* hshIdxDefault0 */



/**
 *  Add 'data' with the key 'aKey' to the hash map 'aHashMap'.
 *  If an element with the key 'aKey' already exists,
 *  it is overwritten with 'data'.
 *  @exception MEMORY_ERROR If there is not enough memory.
 */
void hshIncl (const rtlHashType aHashMap, const genericType aKey,
    const genericType data, intType hashcode, compareType cmp_func,
    const createFuncType key_create_func, const createFuncType data_create_func,
    const copyFuncType data_copy_func)

  {
    rtlHashElemType hashelem;
    intType cmp;
    errInfoType err_info = OKAY_NO_ERROR;

  /* hshIncl */
    logFunction(printf("hshIncl(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U_GEN ", " FMT_U ") size=" FMT_U_MEM "\n",
                       (memSizeType) aHashMap, aKey, data, hashcode, aHashMap->size););
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    if (hashelem == NULL) {
      aHashMap->table[(unsigned int) hashcode & aHashMap->mask] = new_helem(aKey, data,
          key_create_func, data_create_func, &err_info);
      /*
      hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
      printf("aKey=%llX\n", (unsigned long long) aKey);
      printf("new hashelem: aKey=%llX, data=%llX\n",
          hashelem->key.value.intValue, hashelem->data.value.intValue);
      printf("cmp = %d\n", (int) cmp_func(hashelem->key.value.genericValue, aKey));
      */
      aHashMap->size++;
    } else {
      do {
        cmp = cmp_func(hashelem->key.value.genericValue, aKey);
        if (cmp < 0) {
          if (hashelem->next_less == NULL) {
            hashelem->next_less = new_helem(aKey, data,
                key_create_func, data_create_func, &err_info);
            aHashMap->size++;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_less;
          } /* if */
        } else if (unlikely(cmp == 0)) {
          data_copy_func(&hashelem->data.value.genericValue, data);
          hashelem = NULL;
        } else {
          if (hashelem->next_greater == NULL) {
            hashelem->next_greater = new_helem(aKey, data,
                key_create_func, data_create_func, &err_info);
            aHashMap->size++;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_greater;
          } /* if */
        } /* if */
      } while (hashelem != NULL);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      aHashMap->size--;
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("hshIncl(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U_GEN ", " FMT_U ") size=" FMT_U_MEM " -->\n",
                       (memSizeType) aHashMap, aKey, data, hashcode, aHashMap->size););
  } /* hshIncl */



/**
 *  Obtain the keys of the hash map 'aHashMap'.
 *  @return the keys of the hash map.
 */
rtlArrayType hshKeys (const const_rtlHashType aHashMap,
    const createFuncType key_create_func, const destrFuncType key_destr_func)

  {
    rtlArrayType key_array;

  /* hshKeys */
    key_array = keys_hash(aHashMap, key_create_func);
    return key_array;
  } /* hshKeys */



/**
 *  Compute pseudo-random hash table element from 'aHashMap'.
 *  The hash table element contains key and value.
 *  The random values are uniform distributed.
 *  @return a random hash table element such that hshRand(aHashMap) in aHashMap holds.
 *  @exception RANGE_ERROR If 'aHashMap' is empty.
 */
const_rtlHashElemType hshRand (const const_rtlHashType aHashMap)

  {
    memSizeType num_elements;
    memSizeType elem_index;
    const_rtlHashElemType result;

  /* hshRand */
    logFunction(printf("hshRand(" FMT_U_MEM ")\n",
                       (memSizeType) aHashMap););
    num_elements = aHashMap->size;
    if (unlikely(num_elements == 0)) {
      logError(printf("hshRand(): Hash map is empty.\n"););
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      elem_index = (memSizeType) (uintType)
          intRand((intType) 1, (intType) num_elements);
      /* printf("elem_index " FMT_U_MEM "\n", elem_index); */
      result = get_hash_elem(aHashMap, elem_index);
    } /* if */
    logFunction(printf("hshRand --> " FMT_U_MEM "\n",
                       (memSizeType) result););
    return result;
  } /* hshRand */



/**
 *  Add 'data' with the key 'aKey' to the hash map 'aHashMap'.
 *  If an element with the key 'aKey' already exists,
 *  it is overwritten with 'data'.
 *  @param data New value, a temporary value which is used or returned.
 *  @return the old element with the key 'aKey' or
 *          the new data value if no old element existed.
 *  @exception MEMORY_ERROR If there is not enough memory.
 */
genericType hshUpdate (const rtlHashType aHashMap, const genericType aKey,
    const genericType data, intType hashcode, compareType cmp_func,
    const createFuncType key_create_func, const createFuncType data_create_func)

  {
    rtlHashElemType hashelem;
    intType cmp;
    errInfoType err_info = OKAY_NO_ERROR;
    genericType result;

  /* hshUpdate */
    logFunction(printf("hshUpdate(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U_GEN ", " FMT_U ") size=" FMT_U_MEM "\n",
                       (memSizeType) aHashMap, aKey, data, hashcode, aHashMap->size););
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    if (hashelem == NULL) {
      aHashMap->table[(unsigned int) hashcode & aHashMap->mask] = new_helem(aKey, data,
          key_create_func, data_create_func, &err_info);
      aHashMap->size++;
      result = data;
    } else {
      do {
        cmp = cmp_func(hashelem->key.value.genericValue, aKey);
        if (cmp < 0) {
          if (hashelem->next_less == NULL) {
            hashelem->next_less = new_helem(aKey, data,
                key_create_func, data_create_func, &err_info);
            aHashMap->size++;
            result = data;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_less;
          } /* if */
        } else if (unlikely(cmp == 0)) {
          result = hashelem->data.value.genericValue;
          hashelem->data.value.genericValue = data;
          hashelem = NULL;
        } else {
          if (hashelem->next_greater == NULL) {
            hashelem->next_greater = new_helem(aKey, data,
                key_create_func, data_create_func, &err_info);
            aHashMap->size++;
            result = data;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_greater;
          } /* if */
        } /* if */
      } while (hashelem != NULL);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      aHashMap->size--;
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("hshUpdate(" FMT_X_MEM ", " FMT_U_GEN ", "
                       FMT_U_GEN ", " FMT_U ") size=" FMT_U_MEM " -->\n",
                       (memSizeType) aHashMap, aKey, data, hashcode, aHashMap->size););
    return result;
  } /* hshUpdate */



/**
 *  Obtain the values of the hash map 'aHashMap'.
 *  @return the values of the hash map.
 */
rtlArrayType hshValues (const const_rtlHashType aHashMap,
    const createFuncType value_create_func, const destrFuncType value_destr_func)

  {
    rtlArrayType value_array;

  /* hshValues */
    value_array = values_hash(aHashMap, value_create_func);
    return value_array;
  } /* hshValues */
