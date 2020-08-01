/********************************************************************/
/*                                                                  */
/*  hsh_rtl.c     Primitive actions for the string type.            */
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
/*  File: seed7/src/hsh_rtl.c                                       */
/*  Changes: 2005, 2006, 2007, 2013  Thomas Mertes                  */
/*  Content: Primitive actions for the hash type.                   */
/*                                                                  */
/*  The functions from this file should only be used in compiled    */
/*  Seed7 programs. The interpreter should not use functions of     */
/*  this file.                                                      */
/*                                                                  */
/*  The functions in this file use type declarations from the       */
/*  include file data_rtl.h instead of data.h. Therefore the types  */
/*  rtlHelemtype and rtlHashtype are declared different than the    */
/*  types helemtype and hashtype in the interpreter.                */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "hsh_rtl.h"

#undef TRACE_HSH_RTL


#define TABLE_BITS 10
#define TABLE_SIZE(bits) ((unsigned int) 1 << (bits))
#define TABLE_MASK(bits) (TABLE_SIZE(bits)-1)
#define ARRAY_SIZE_INCREMENT 512



static void free_helem (const const_rtlHelemtype old_helem,
    const destrfunctype key_destr_func, const destrfunctype data_destr_func)

  { /* free_helem */
    key_destr_func(old_helem->key.value.genericvalue);
    data_destr_func(old_helem->data.value.genericvalue);
    if (old_helem->next_less != NULL) {
      free_helem(old_helem->next_less, key_destr_func,
          data_destr_func);
    } /* if */
    if (old_helem->next_greater != NULL) {
      free_helem(old_helem->next_greater, key_destr_func,
          data_destr_func);
    } /* if */
    FREE_RECORD(old_helem, rtlHelemrecord, count.rtl_helem);
  } /* free_helem */



static void free_hash (const const_rtlHashtype old_hash,
    const destrfunctype key_destr_func, const destrfunctype data_destr_func)

  {
    unsigned int number;
    const rtlHelemtype *curr_helem;

  /* free_hash */
    if (old_hash != NULL) {
      number = old_hash->table_size;
      curr_helem = &old_hash->table[0];
      while (number > 0) {
        if (*curr_helem != NULL) {
          free_helem(*curr_helem, key_destr_func, data_destr_func);
        } /* if */
        number--;
        curr_helem++;
      } /* while */
      FREE_RTL_HASH(old_hash, old_hash->table_size);
    } /* if */
  } /* free_hash */



static rtlHelemtype new_helem (rtlGenerictype key, rtlGenerictype data,
    const createfunctype key_create_func, const createfunctype data_create_func,
    errinfotype *err_info)

  {
    rtlHelemtype helem;

  /* new_helem */
    /* printf("new_helem(%llX, %llX)\n",
        (unsigned long long) key,
        (unsigned long long) data); */
    if (!ALLOC_RECORD(helem, rtlHelemrecord, count.rtl_helem)) {
      *err_info = MEMORY_ERROR;
    } else {
      helem->key.value.genericvalue = key_create_func(key);
      helem->data.value.genericvalue = data_create_func(data);
      helem->next_less = NULL;
      helem->next_greater = NULL;
      /* printf("new_helem(%llX, %llX)\n",
          (unsigned long long) helem->key.value.genericvalue,
          (unsigned long long) helem->data.value.genericvalue); */
    } /* if */
    return helem;
  } /* new_helem */



static rtlHashtype new_hash (unsigned int bits)

  {
    rtlHashtype hash;

  /* new_hash */
    if (ALLOC_RTL_HASH(hash, TABLE_SIZE(bits))) {
      hash->bits = bits;
      hash->mask = TABLE_MASK(bits);
      hash->table_size = TABLE_SIZE(bits);
      hash->size = 0;
      memset(hash->table, 0, hash->table_size * sizeof(rtlHelemtype));
    } /* if */
    return hash;
  } /* new_hash */



static rtlHelemtype create_helem (const const_rtlHelemtype source_helem,
    const createfunctype key_create_func, const createfunctype data_create_func,
    errinfotype *err_info)

  {
    rtlHelemtype dest_helem;

  /* create_helem */
    if (!ALLOC_RECORD(dest_helem, rtlHelemrecord, count.rtl_helem)) {
      *err_info = MEMORY_ERROR;
    } else {
      dest_helem->key.value.genericvalue = key_create_func(source_helem->key.value.genericvalue);
      dest_helem->data.value.genericvalue = data_create_func(source_helem->data.value.genericvalue);
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



static rtlHashtype create_hash (const const_rtlHashtype source_hash,
    const createfunctype key_create_func, const createfunctype data_create_func,
    errinfotype *err_info)

  {
    unsigned int new_size;
    unsigned int number;
    const rtlHelemtype *source_helem;
    rtlHelemtype *dest_helem;
    rtlHashtype dest_hash;

  /* create_hash */
    new_size = source_hash->table_size;
    if (!ALLOC_RTL_HASH(dest_hash, new_size)) {
      *err_info = MEMORY_ERROR;
    } else {
      dest_hash->bits = source_hash->bits;
      dest_hash->mask = source_hash->mask;
      dest_hash->table_size = source_hash->table_size;
      dest_hash->size = source_hash->size;
      number = source_hash->table_size;
      source_helem = &source_hash->table[0];
      dest_helem = &dest_hash->table[0];
      while (number > 0 && *err_info == OKAY_NO_ERROR) {
        while (number > 0 && *source_helem == NULL) {
          *dest_helem = NULL;
          number--;
          source_helem++;
          dest_helem++;
        } /* while */
        if (number > 0 && *source_helem != NULL) {
          *dest_helem = create_helem(*source_helem, key_create_func, data_create_func, err_info);
          number--;
          source_helem++;
          dest_helem++;
        } /* if */
      } /* while */
    } /* if */
    return dest_hash;
  } /* create_hash */



static void copy_hash (const rtlHashtype dest_hash, const const_rtlHashtype source_hash,
    const createfunctype key_create_func, const createfunctype data_create_func,
    const destrfunctype key_destr_func, const destrfunctype data_destr_func,
    errinfotype *err_info)

  {
    unsigned int number;
    const rtlHelemtype *source_helem;
    rtlHelemtype *dest_helem;

  /* copy_hash */
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
        *dest_helem = create_helem(*source_helem, key_create_func, data_create_func, err_info);
      } else {
        *dest_helem = NULL;
      } /* if */
      number--;
      source_helem++;
      dest_helem++;
    } /* while */
  } /* copy_hash */



static void keys_helem (rtlArraytype *key_array, memsizetype *arr_pos,
    const_rtlHelemtype curr_helem, const createfunctype key_create_func,
    errinfotype *err_info)

  {
    memsizetype array_size;
    rtlArraytype resized_key_array;

  /* keys_helem */
    array_size = arraySize(*key_array);
    if (*arr_pos + 1 >= array_size) {
      if (array_size > MAX_RTL_ARR_LEN - ARRAY_SIZE_INCREMENT ||
          (*key_array)->max_position > MAX_MEM_INDEX - ARRAY_SIZE_INCREMENT) {
        resized_key_array = NULL;
      } else {
        resized_key_array = REALLOC_RTL_ARRAY(*key_array,
            array_size, array_size + ARRAY_SIZE_INCREMENT);
      } /* if */
      if (resized_key_array == NULL) {
        *err_info = MEMORY_ERROR;
        return;
      } else {
        *key_array = resized_key_array;
        COUNT3_RTL_ARRAY(array_size, array_size + ARRAY_SIZE_INCREMENT);
        (*key_array)->max_position += ARRAY_SIZE_INCREMENT;
      } /* if */
    } /* if */
    (*key_array)->arr[*arr_pos].value.genericvalue =
        key_create_func(curr_helem->key.value.genericvalue);
    (*arr_pos)++;
    if (curr_helem->next_less != NULL) {
      keys_helem(key_array, arr_pos, curr_helem->next_less, key_create_func, err_info);
    } /* if */
    if (curr_helem->next_greater != NULL) {
      keys_helem(key_array, arr_pos, curr_helem->next_greater, key_create_func, err_info);
    } /* if */
  } /* keys_helem */



static rtlArraytype keys_hash (const const_rtlHashtype curr_hash,
    const createfunctype key_create_func, const destrfunctype key_destr_func,
    errinfotype *err_info)

  {
    memsizetype arr_pos;
    memsizetype number;
    const rtlHelemtype *curr_helem;
    memsizetype array_size;
    rtlArraytype resized_key_array;
    rtlArraytype key_array;

  /* keys_hash */
    if (!ALLOC_RTL_ARRAY(key_array, ARRAY_SIZE_INCREMENT)) {
      *err_info = MEMORY_ERROR;
    } else {
      key_array->min_position = 1;
      key_array->max_position = ARRAY_SIZE_INCREMENT;
      arr_pos = 0;
      number = curr_hash->table_size;
      curr_helem = &curr_hash->table[0];
      while (number > 0 && *err_info == OKAY_NO_ERROR) {
        if (*curr_helem != NULL) {
          keys_helem(&key_array, &arr_pos, *curr_helem, key_create_func, err_info);
        } /* if */
        number--;
        curr_helem++;
      } /* while */
      array_size = arraySize(key_array);
      if (*err_info == OKAY_NO_ERROR) {
        resized_key_array = REALLOC_RTL_ARRAY(key_array, array_size, arr_pos);
        if (resized_key_array == NULL) {
          *err_info = MEMORY_ERROR;
        } else {
          key_array = resized_key_array;
          COUNT3_RTL_ARRAY(array_size, arr_pos);
          key_array->max_position = (inttype) arr_pos;
        } /* if */
      } /* if */
      if (*err_info != OKAY_NO_ERROR) {
        for (number = 0; number < arr_pos; number++) {
          key_destr_func(key_array->arr[number].value.genericvalue);
        } /* for */
        FREE_RTL_ARRAY(key_array, array_size);
        key_array = NULL;
      } /* if */
    } /* if */
    return key_array;
  } /* keys_hash */



static void values_helem (rtlArraytype *value_array, memsizetype *arr_pos,
    const_rtlHelemtype curr_helem, const createfunctype value_create_func,
    errinfotype *err_info)

  {
    memsizetype array_size;
    rtlArraytype resized_value_array;

  /* values_helem */
    array_size = arraySize(*value_array);
    if (*arr_pos + 1 >= array_size) {
      if (array_size > MAX_RTL_ARR_LEN - ARRAY_SIZE_INCREMENT ||
          (*value_array)->max_position > MAX_MEM_INDEX - ARRAY_SIZE_INCREMENT) {
        resized_value_array = NULL;
      } else {
        resized_value_array = REALLOC_RTL_ARRAY(*value_array,
            array_size, array_size + ARRAY_SIZE_INCREMENT);
      } /* if */
      if (resized_value_array == NULL) {
        *err_info = MEMORY_ERROR;
        return;
      } else {
        *value_array = resized_value_array;
        COUNT3_RTL_ARRAY(array_size, array_size + ARRAY_SIZE_INCREMENT);
        (*value_array)->max_position += ARRAY_SIZE_INCREMENT;
      } /* if */
    } /* if */
    (*value_array)->arr[*arr_pos].value.genericvalue =
        value_create_func(curr_helem->data.value.genericvalue);
    (*arr_pos)++;
    if (curr_helem->next_less != NULL) {
      values_helem(value_array, arr_pos, curr_helem->next_less, value_create_func, err_info);
    } /* if */
    if (curr_helem->next_greater != NULL) {
      values_helem(value_array, arr_pos, curr_helem->next_greater, value_create_func, err_info);
    } /* if */
  } /* values_helem */



static rtlArraytype values_hash (const const_rtlHashtype curr_hash,
    const createfunctype value_create_func, const destrfunctype value_destr_func,
    errinfotype *err_info)

  {
    memsizetype arr_pos;
    memsizetype number;
    const rtlHelemtype *curr_helem;
    memsizetype array_size;
    rtlArraytype resized_value_array;
    rtlArraytype value_array;

  /* values_hash */
    if (!ALLOC_RTL_ARRAY(value_array, ARRAY_SIZE_INCREMENT)) {
      *err_info = MEMORY_ERROR;
    } else {
      value_array->min_position = 1;
      value_array->max_position = ARRAY_SIZE_INCREMENT;
      arr_pos = 0;
      number = curr_hash->table_size;
      curr_helem = &curr_hash->table[0];
      while (number > 0 && *err_info == OKAY_NO_ERROR) {
        if (*curr_helem != NULL) {
          values_helem(&value_array, &arr_pos, *curr_helem, value_create_func, err_info);
        } /* if */
        number--;
        curr_helem++;
      } /* while */
      array_size = arraySize(value_array);
      if (*err_info == OKAY_NO_ERROR) {
        resized_value_array = REALLOC_RTL_ARRAY(value_array, array_size, arr_pos);
        if (resized_value_array == NULL) {
          *err_info = MEMORY_ERROR;
        } else {
          value_array = resized_value_array;
          COUNT3_RTL_ARRAY(array_size, arr_pos);
          value_array->max_position = (inttype) arr_pos;
        } /* if */
      } /* if */
      if (*err_info != OKAY_NO_ERROR) {
        for (number = 0; number < arr_pos; number++) {
          value_destr_func(value_array->arr[number].value.genericvalue);
        } /* for */
        FREE_RTL_ARRAY(value_array, array_size);
        value_array = NULL;
      } /* if */
    } /* if */
    return value_array;
  } /* values_hash */



/**
 *  Hash membership test.
 *  Determine if 'aKey' is a member of the hash map 'aHashMap'.
 *  @return TRUE when 'aKey' is a member of 'aHashMap',
 *          FALSE otherwise.
 */
booltype hshContains (const const_rtlHashtype aHashMap, const rtlGenerictype aKey,
    inttype hashcode, comparetype cmp_func)

  {
    const_rtlHelemtype hashelem;
    inttype cmp;
    booltype result;

  /* hshContains */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshContains(%lX, %lu, %lu)\n",
        (long unsigned) aHashMap, (long unsigned) aKey, (long unsigned) hashcode);
#endif
    result = FALSE;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
/*
printf("sizeof(hashelem->key.value.genericvalue)=%lu\n", sizeof(hashelem->key.value.genericvalue));
printf("sizeof(aKey)=%lu\n", sizeof(aKey));
printf("%llX\n", hashelem->key.value.genericvalue);
printf("%lX\n", (long unsigned) hashelem->key.value.genericvalue);
printf("%llX\n", aKey);
printf("%lX\n", (long unsigned) aKey);
*/
      cmp = cmp_func(hashelem->key.value.genericvalue, aKey);
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (unlikely(cmp == 0)) {
        result = TRUE;
        hashelem = NULL;
      } else {
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
#ifdef TRACE_HSH_RTL
    printf("END hshContains(%lX, %lu, %lu) ==> %d\n",
        (long unsigned) aHashMap, (long unsigned) aKey, (long unsigned) hashcode,
        result);
#endif
    return result;
  } /* hshContains */



void hshCpy (rtlHashtype *const hash_to, const const_rtlHashtype hash_from,
    const createfunctype key_create_func, const destrfunctype key_destr_func,
    const createfunctype data_create_func, const destrfunctype data_destr_func)

  {
    errinfotype err_info = OKAY_NO_ERROR;

  /* hshCpy */
    if ((*hash_to)->table_size == hash_from->table_size) {
      copy_hash(*hash_to, hash_from,
          key_create_func, data_create_func,
          key_destr_func, data_destr_func, &err_info);
    } else {
      free_hash(*hash_to, key_destr_func, data_destr_func);
      *hash_to = create_hash(hash_from,
          key_create_func, data_create_func, &err_info);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      free_hash(*hash_to, key_destr_func, data_destr_func);
      *hash_to = NULL;
      raise_error(MEMORY_ERROR);
    } /* if */
  } /* hshCpy */



rtlHashtype hshCreate (const const_rtlHashtype hash_from,
    const createfunctype key_create_func, const destrfunctype key_destr_func,
    const createfunctype data_create_func, const destrfunctype data_destr_func)

  {
    errinfotype err_info = OKAY_NO_ERROR;
    rtlHashtype result;

  /* hshCreate */
    result = create_hash(hash_from,
        key_create_func, data_create_func, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      free_hash(result, key_destr_func, data_destr_func);
      result = NULL;
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* hshCreate */



void hshDestr (const const_rtlHashtype old_hash, const destrfunctype key_destr_func,
    const destrfunctype data_destr_func)

{ /* hshDestr */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshDestr(%lX)\n", (unsigned long) old_hash);
#endif
    free_hash(old_hash, key_destr_func, data_destr_func);
#ifdef TRACE_HSH_RTL
    printf("END hshDestr\n");
#endif
  } /* hshDestr */



rtlHashtype hshEmpty (void)

  {
    rtlHashtype result;

  /* hshEmpty */
    result = new_hash(TABLE_BITS);
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* hshEmpty */



/**
 *  Remove the element with the key 'aKey' from the hash map 'aHashMap'.
 */
void hshExcl (const rtlHashtype aHashMap, const rtlGenerictype aKey,
    inttype hashcode, comparetype cmp_func, const destrfunctype key_destr_func,
    const destrfunctype data_destr_func)

  {
    rtlHelemtype *delete_pos;
    rtlHelemtype hashelem;
    rtlHelemtype greater_hashelems;
    rtlHelemtype old_hashelem;
    inttype cmp;

  /* hshExcl */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshExcl(%lX, %lu, %lu) size=%lu\n",
        (unsigned long) aHashMap, (unsigned long) aKey,
        (unsigned long) hashcode, aHashMap->size);
#endif
    delete_pos = &aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericvalue, aKey);
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
#ifdef TRACE_HSH_RTL
    printf("END hshExcl(%lX, %lu, %lu) size=%lu\n",
        (unsigned long) aHashMap, (unsigned long) aKey,
        (unsigned long) hashcode, aHashMap->size);
#endif
  } /* hshExcl */



/**
 *  Access one value from the hash table 'aHashMap'.
 *  @return the element with the key 'aKey' from 'aHashMap'.
 *  @exception RANGE_ERROR When 'aHashMap' does not have an element
 *             with the key 'aKey'.
 */
rtlGenerictype hshIdx (const const_rtlHashtype aHashMap, const rtlGenerictype aKey,
    inttype hashcode, comparetype cmp_func)

  {
    rtlHelemtype hashelem;
    rtlHelemtype result_hashelem;
    inttype cmp;
    rtlGenerictype result;

  /* hshIdx */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshIdx(%lX, %lu, %lu)\n",
        (unsigned long) aHashMap, (unsigned long) aKey, (unsigned long) hashcode);
#endif
    result_hashelem = NULL;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericvalue, aKey);
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
      result = result_hashelem->data.value.genericvalue;
    } else {
      raise_error(RANGE_ERROR);
      result = 0;
    } /* if */
#ifdef TRACE_HSH_RTL
    printf("END hshIdx(%lX, %lu, %lu) ==> %lX (%lX)\n",
        (unsigned long) aHashMap, (unsigned long) aKey, (unsigned long) hashcode,
        (unsigned long) result,
        (unsigned long) (result != NULL ? *((rtlGenerictype *)result) : 0));
#endif
    return result;
  } /* hshIdx */



/**
 *  Determine the address of a value from the hash table 'aHashMap'.
 *  @return the address of the element with the key 'aKey' from 'aHashMap'.
 *  @exception RANGE_ERROR When 'aHashMap' does not have an element
 *             with the key 'aKey'.
 */
rtlObjecttype *hshIdxAddr (const const_rtlHashtype aHashMap,
    const rtlGenerictype aKey, inttype hashcode, comparetype cmp_func)

  {
    rtlHelemtype hashelem;
    rtlHelemtype result_hashelem;
    inttype cmp;
    rtlObjecttype *result;

  /* hshIdxAddr */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshIdxAddr(%lX, %lu, %lu)\n",
        (unsigned long) aHashMap, (unsigned long) aKey, (unsigned long) hashcode);
#endif
    result_hashelem = NULL;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericvalue, aKey);
      /* printf(". %llu %llu cmp=%d\n",
          (unsigned long long) hashelem->key.value.genericvalue,
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
      raise_error(RANGE_ERROR);
      result = NULL;
    } /* if */
#ifdef TRACE_HSH_RTL
    printf("END hshIdxAddr(%lX, %lu, %lu) ==> %lX (%lX)\n",
        (unsigned long) aHashMap, (unsigned long) aKey, (unsigned long) hashcode,
        (unsigned long) result,
        (unsigned long) (result != NULL ? *((rtlGenerictype *)result) : 0));
#endif
    return result;
  } /* hshIdxAddr */



/**
 *  Determine the address of a value from the hash table 'aHashMap'.
 *  @return the address of the element with the key 'aKey' from 'aHashMap' or
 *          NULL when 'aHashMap' does not have an element with the key 'aKey'.
 */
rtlObjecttype *hshIdxAddr2 (const const_rtlHashtype aHashMap,
    const rtlGenerictype aKey, inttype hashcode, comparetype cmp_func)

  {
    rtlHelemtype hashelem;
    rtlHelemtype result_hashelem;
    inttype cmp;
    rtlObjecttype *result;

  /* hshIdxAddr2 */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshIdxAddr(%lX, %lu, %lu)\n",
        (unsigned long) aHashMap, (unsigned long) aKey, (unsigned long) hashcode);
#endif
    result_hashelem = NULL;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericvalue, aKey);
      /* printf(". %llu %llu cmp=%d\n",
          (unsigned long long) hashelem->key.value.genericvalue,
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
#ifdef TRACE_HSH_RTL
    printf("END hshIdxAddr(%lX, %lu, %lu) ==> %lX (%lX)\n",
        (unsigned long) aHashMap, (unsigned long) aKey, (unsigned long) hashcode,
        (unsigned long) result,
        (unsigned long) (result != NULL ? *((rtlGenerictype *)result) : 0));
#endif
    return result;
  } /* hshIdxAddr2 */



rtlGenerictype hshIdxEnterDefault (const rtlHashtype aHashMap, const rtlGenerictype aKey,
    const rtlGenerictype defaultData, inttype hashcode, comparetype cmp_func,
    const createfunctype key_create_func, const createfunctype data_create_func)

  {
    rtlHelemtype hashelem;
    rtlHelemtype result_hashelem;
    inttype cmp;
    errinfotype err_info = OKAY_NO_ERROR;
    rtlGenerictype result;

  /* hshIdxEnterDefault */
    /* printf("hshIdxEnterDefault(%lX, %llX, %lld, %llX, %lX, %lx, %lX)\n",
       aHashMap, aKey, defaultData, hashcode, cmp_func, key_create_func, data_create_func); */
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    if (hashelem == NULL) {
      result_hashelem = new_helem(aKey, defaultData,
          key_create_func, data_create_func, &err_info);
      aHashMap->table[(unsigned int) hashcode & aHashMap->mask] = result_hashelem;
      aHashMap->size++;
    } else {
      do {
        /* printf("key=%llX\n", hashelem->key.value.genericvalue); */
        cmp = cmp_func(hashelem->key.value.genericvalue, aKey);
        if (cmp < 0) {
          if (hashelem->next_less == NULL) {
            result_hashelem = new_helem(aKey, defaultData,
                key_create_func, data_create_func, &err_info);
            hashelem->next_less = result_hashelem;
            aHashMap->size++;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_less;
          } /* if */
        } else if (unlikely(cmp == 0)) {
          result_hashelem = hashelem;
          hashelem = NULL;
        } else {
          if (hashelem->next_greater == NULL) {
            result_hashelem = new_helem(aKey, defaultData,
                key_create_func, data_create_func, &err_info);
            hashelem->next_greater = result_hashelem;
            aHashMap->size++;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_greater;
          } /* if */
        } /* if */
      } while (hashelem != NULL);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      aHashMap->size--;
      raise_error(MEMORY_ERROR);
      result = 0;
    } else {
      result = result_hashelem->data.value.genericvalue;
    } /* if */
    return result;
  } /* hshIdxEnterDefault */



rtlGenerictype hshIdxWithDefault (const const_rtlHashtype aHashMap, const rtlGenerictype aKey,
    const rtlGenerictype defaultData, inttype hashcode, comparetype cmp_func)

  {
    rtlHelemtype hashelem;
    rtlHelemtype result_hashelem;
    inttype cmp;
    rtlGenerictype result;

  /* hshIdxWithDefault */
    result_hashelem = NULL;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericvalue, aKey);
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
      result = result_hashelem->data.value.genericvalue;
    } else {
      result = defaultData;
    } /* if */
    return result;
  } /* hshIdxWithDefault */



/**
 *  Add 'anElem' with the key 'aKey' to the hash map 'aHashMap'.
 *  When an element with the key 'aKey' already exists,
 *  it is overwritten with 'anElem'.
 *  @exception MEMORY_ERROR When there is not enough memory.
 */
void hshIncl (const rtlHashtype aHashMap, const rtlGenerictype aKey,
    const rtlGenerictype data, inttype hashcode, comparetype cmp_func,
    const createfunctype key_create_func, const createfunctype data_create_func,
    const copyfunctype data_copy_func)

  {
    rtlHelemtype hashelem;
    inttype cmp;
    errinfotype err_info = OKAY_NO_ERROR;

  /* hshIncl */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshIncl(%lX, %llu, %lu, %lu) size=%lu\n",
        (unsigned long) aHashMap, (unsigned long long) aKey,
        (unsigned long) data, (unsigned long) hashcode, aHashMap->size);
#endif
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    if (hashelem == NULL) {
      aHashMap->table[(unsigned int) hashcode & aHashMap->mask] = new_helem(aKey, data,
          key_create_func, data_create_func, &err_info);
      /*
      hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
      printf("aKey=%llX\n", (unsigned long long) aKey);
      printf("new hashelem: aKey=%llX, data=%llX\n",
          hashelem->key.value.intvalue, hashelem->data.value.intvalue);
      printf("cmp = %d\n", (int) cmp_func(hashelem->key.value.genericvalue, aKey));
      */
      aHashMap->size++;
    } else {
      do {
        cmp = cmp_func(hashelem->key.value.genericvalue, aKey);
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
          data_copy_func(&hashelem->data.value.genericvalue, data);
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
    if (err_info != OKAY_NO_ERROR) {
      aHashMap->size--;
      raise_error(MEMORY_ERROR);
    } /* if */
#ifdef TRACE_HSH_RTL
    printf("END hshIncl(%lX, %lu, %lu, %lu) size=%lu\n",
        (unsigned long) aHashMap, (unsigned long) aKey,
        (unsigned long) data, (unsigned long) hashcode, aHashMap->size);
#endif
  } /* hshIncl */



/**
 *  Obtain the keys of the hash map 'aHashMap'.
 *  @return the keys of the hash map.
 */
rtlArraytype hshKeys (const const_rtlHashtype aHashMap,
    const createfunctype key_create_func, const destrfunctype key_destr_func)

  {
    rtlArraytype key_array;
    errinfotype err_info = OKAY_NO_ERROR;

  /* hshKeys */
    key_array = keys_hash(aHashMap, key_create_func, key_destr_func,
        &err_info);
    return key_array;
  } /* hshKeys */



/**
 *  Add 'anElem' with the key 'aKey' to the hash map 'aHashMap'.
 *  When an element with the key 'aKey' already exists,
 *  it is overwritten with 'anElem'.
 *  @param data New value, a temporary value which is used or returned.
 *  @exception MEMORY_ERROR When there is not enough memory.
 *  @return the old element with the key 'aKey' or
 *          the new data value when no old element existed.
 */
rtlGenerictype hshUpdate (const rtlHashtype aHashMap, const rtlGenerictype aKey,
    const rtlGenerictype data, inttype hashcode, comparetype cmp_func,
    const createfunctype key_create_func, const createfunctype data_create_func)

  {
    rtlHelemtype hashelem;
    inttype cmp;
    errinfotype err_info = OKAY_NO_ERROR;
    rtlGenerictype result;

  /* hshUpdate */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshUpdate(%lX, %llu, %lu, %lu) size=%lu\n",
        (unsigned long) aHashMap, (unsigned long long) aKey,
        (unsigned long) data, (unsigned long) hashcode, aHashMap->size);
#endif
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    if (hashelem == NULL) {
      aHashMap->table[(unsigned int) hashcode & aHashMap->mask] = new_helem(aKey, data,
          key_create_func, data_create_func, &err_info);
      aHashMap->size++;
      result = data;
    } else {
      do {
        cmp = cmp_func(hashelem->key.value.genericvalue, aKey);
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
          result = hashelem->data.value.genericvalue;
          hashelem->data.value.genericvalue = data;
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
    if (err_info != OKAY_NO_ERROR) {
      aHashMap->size--;
      raise_error(MEMORY_ERROR);
    } /* if */
#ifdef TRACE_HSH_RTL
    printf("END hshUpdate(%lX, %lu, %lu, %lu) size=%lu\n",
        (unsigned long) aHashMap, (unsigned long) aKey,
        (unsigned long) data, (unsigned long) hashcode, aHashMap->size);
#endif
    return result;
  } /* hshUpdate */



/**
 *  Obtain the values of the hash map 'aHashMap'.
 *  @return the values of the hash map.
 */
rtlArraytype hshValues (const const_rtlHashtype aHashMap,
    const createfunctype value_create_func, const destrfunctype value_destr_func)

  {
    rtlArraytype value_array;
    errinfotype err_info = OKAY_NO_ERROR;

  /* hshValues */
    value_array = values_hash(aHashMap, value_create_func, value_destr_func,
        &err_info);
    return value_array;
  } /* hshValues */
