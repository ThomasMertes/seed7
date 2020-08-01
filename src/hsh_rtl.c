/********************************************************************/
/*                                                                  */
/*  hsh_rtl.c     Primitive actions for the string type.            */
/*  Copyright (C) 1989 - 2009  Thomas Mertes                        */
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
/*  Changes: 2005, 2006, 2007  Thomas Mertes                        */
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
#define TABLE_SIZE(bits) (1 << (bits))
#define TABLE_MASK(bits) (TABLE_SIZE(bits)-1)
#define ARRAY_SIZE_INCREMENT 512



#ifdef ANSI_C

static void free_helem (rtlHelemtype old_helem, destrfunctype key_destr_func,
    destrfunctype data_destr_func)
#else

static void free_helem (old_helem, key_destr_func, data_destr_func)
rtlHelemtype old_helem;
destrfunctype key_destr_func;
destrfunctype data_destr_func;
#endif

  { /* free_helem */
    if (old_helem != NULL) {
      key_destr_func(old_helem->key.value.genericvalue);
      data_destr_func(old_helem->data.value.genericvalue);
      free_helem(old_helem->next_less, key_destr_func,
          data_destr_func);
      free_helem(old_helem->next_greater, key_destr_func,
          data_destr_func);
      FREE_RECORD(old_helem, rtlHelemrecord, count.helem);
    } /* if */
  } /* free_helem */



#ifdef ANSI_C

static void free_hash (const const_rtlHashtype old_hash, destrfunctype key_destr_func,
    destrfunctype data_destr_func)
#else

static void free_hash (old_hash, key_destr_func, data_destr_func)
rtlHashtype old_hash;
destrfunctype key_destr_func;
destrfunctype data_destr_func;
#endif

  {
    int number;
    const rtlHelemtype *curr_helem;

  /* free_hash */
    if (old_hash != NULL) {
      number = old_hash->table_size;
      curr_helem = &old_hash->table[0];
      while (number > 0) {
        free_helem(*curr_helem, key_destr_func, data_destr_func);
        number--;
        curr_helem++;
      } /* while */
      FREE_RTL_HASH(old_hash, old_hash->table_size);
    } /* if */
  } /* free_hash */



#ifdef ANSI_C

static rtlHelemtype new_helem (rtlGenerictype key, rtlGenerictype data,
    createfunctype key_create_func, createfunctype data_create_func, errinfotype *err_info)
#else

static rtlHelemtype new_helem (key, data, key_create_func, data_create_func, err_info)
rtlGenerictype key;
rtlGenerictype data;
createfunctype key_create_func;
createfunctype data_create_func;
errinfotype *err_info;
#endif

  {
    rtlHelemtype helem;

  /* new_helem */
    /* printf("new_helem(%llX, %llX)\n",
        (unsigned long long) key,
        (unsigned long long) data); */
    if (!ALLOC_RECORD(helem, rtlHelemrecord, count.helem)) {
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
    return(helem);
  } /* new_helem */



#ifdef ANSI_C

static rtlHashtype new_hash (int bits, errinfotype *err_info)
#else

static rtlHashtype new_hash (bits, err_info)
int bits;
errinfotype *err_info;
#endif

  {
    rtlHashtype hash;

  /* new_hash */
    if (!ALLOC_RTL_HASH(hash, TABLE_SIZE(bits))) {
      *err_info = MEMORY_ERROR;
    } else {
      hash->bits = bits;
      hash->mask = TABLE_MASK(bits);
      hash->table_size = TABLE_SIZE(bits);
      hash->size = 0;
      memset(hash->table, 0, hash->table_size * sizeof(rtlHelemtype));
    } /* if */
    return(hash);
  } /* new_hash */



#ifdef ANSI_C

static rtlHelemtype copy_helem (const_rtlHelemtype source_helem, createfunctype key_create_func,
    createfunctype data_create_func, errinfotype *err_info)
#else

static rtlHelemtype copy_helem (source_helem, key_create_func, data_create_func, err_info)
rtlHelemtype source_helem;
createfunctype key_create_func;
createfunctype data_create_func;
errinfotype *err_info;
#endif

  {
    rtlHelemtype dest_helem;

  /* copy_helem */
    if (source_helem != NULL) {
      if (!ALLOC_RECORD(dest_helem, rtlHelemrecord, count.helem)) {
        *err_info = MEMORY_ERROR;
      } else {
        dest_helem->key.value.genericvalue = key_create_func(source_helem->key.value.genericvalue);
        dest_helem->data.value.genericvalue = data_create_func(source_helem->data.value.genericvalue);
        dest_helem->next_less = copy_helem(source_helem->next_less,
            key_create_func, data_create_func, err_info);
        dest_helem->next_greater = copy_helem(source_helem->next_greater,
            key_create_func, data_create_func, err_info);
      } /* if */
    } else {
      dest_helem = NULL;
    } /* if */
    return(dest_helem);
  } /* copy_helem */



#ifdef ANSI_C

static rtlHashtype copy_hash (const const_rtlHashtype source_hash,
    createfunctype key_create_func, createfunctype data_create_func,
    errinfotype *err_info)
#else

static rtlHashtype copy_hash (source_hash, key_create_func, data_create_func,
    err_info)
rtlHashtype source_hash;
createfunctype key_create_func;
createfunctype data_create_func;
errinfotype *err_info;
#endif

  {
    int new_size;
    int number;
    const rtlHelemtype *source_helem;
    rtlHelemtype *dest_helem;
    rtlHashtype dest_hash;

  /* copy_hash */
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
        *dest_helem = copy_helem(*source_helem, key_create_func, data_create_func, err_info);
        number--;
        source_helem++;
        dest_helem++;
      } /* while */
    } /* if */
    return(dest_hash);
  } /* copy_hash */



#ifdef ANSI_C

static void keys_helem (rtlArraytype *key_array, memsizetype *arr_pos,
    const_rtlHelemtype curr_helem, createfunctype key_create_func, errinfotype *err_info)
#else

static void keys_helem (key_array, arr_pos, curr_helem, key_create_func, err_info)
rtlArraytype *key_array;
memsizetype *arr_pos;
rtlHelemtype curr_helem;
createfunctype key_create_func;
errinfotype *err_info;
#endif

  {
    memsizetype array_size;
    rtlArraytype resized_key_array;

  /* keys_helem */
    array_size = (*key_array)->max_position - (*key_array)->min_position;
    if (*arr_pos >= array_size) {
      resized_key_array = REALLOC_RTL_ARRAY(*key_array,
          array_size, array_size + ARRAY_SIZE_INCREMENT);
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



#ifdef ANSI_C

static rtlArraytype keys_hash (const const_rtlHashtype curr_hash,
    createfunctype key_create_func, destrfunctype key_destr_func,
    errinfotype *err_info)
#else

static rtlArraytype keys_hash (curr_hash, key_create_func, key_destr_func,
    err_info)
rtlHashtype curr_hash;
createfunctype key_create_func;
destrfunctype key_destr_func;
errinfotype *err_info;
#endif

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
      array_size = key_array->max_position - key_array->min_position;
      if (*err_info == OKAY_NO_ERROR) {
        resized_key_array = REALLOC_RTL_ARRAY(key_array, array_size, arr_pos);
        if (resized_key_array == NULL) {
          *err_info = MEMORY_ERROR;
        } else {
          key_array = resized_key_array;
          COUNT3_RTL_ARRAY(array_size, arr_pos);
          key_array->max_position = arr_pos;
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
    return(key_array);
  } /* keys_hash */



#ifdef ANSI_C

static void values_helem (rtlArraytype *value_array, memsizetype *arr_pos,
    const_rtlHelemtype curr_helem, createfunctype value_create_func, errinfotype *err_info)
#else

static void values_helem (value_array, arr_pos, curr_helem, value_create_func, err_info)
rtlArraytype *value_array;
memsizetype *arr_pos;
rtlHelemtype curr_helem;
createfunctype value_create_func;
errinfotype *err_info;
#endif

  {
    memsizetype array_size;
    rtlArraytype resized_value_array;

  /* values_helem */
    array_size = (*value_array)->max_position - (*value_array)->min_position;
    if (*arr_pos >= array_size) {
      resized_value_array = REALLOC_RTL_ARRAY(*value_array,
          array_size, array_size + ARRAY_SIZE_INCREMENT);
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



#ifdef ANSI_C

static rtlArraytype values_hash (const const_rtlHashtype curr_hash,
    createfunctype value_create_func, destrfunctype value_destr_func,
    errinfotype *err_info)
#else

static rtlArraytype values_hash (curr_hash, value_create_func, value_destr_func,
    err_info)
rtlHashtype curr_hash;
createfunctype value_create_func;
destrfunctype value_destr_func;
errinfotype *err_info;
#endif

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
      array_size = value_array->max_position - value_array->min_position;
      if (*err_info == OKAY_NO_ERROR) {
        resized_value_array = REALLOC_RTL_ARRAY(value_array, array_size, arr_pos);
        if (resized_value_array == NULL) {
          *err_info = MEMORY_ERROR;
        } else {
          value_array = resized_value_array;
          COUNT3_RTL_ARRAY(array_size, arr_pos);
          value_array->max_position = arr_pos;
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
    return(value_array);
  } /* values_hash */



#ifdef ANSI_C

booltype hshContains (const const_rtlHashtype hash1, const rtlGenerictype key,
    inttype hashcode, comparetype cmp_func)
#else

booltype hshContains (hash1, key, hashcode, cmp_func)
rtlHashtype hash1;
rtlGenerictype key;
inttype hashcode;
comparetype cmp_func;
#endif

  {
    rtlHelemtype hashelem;
    rtlHelemtype result_hashelem;
    int cmp;

  /* hshContains */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshContains(%lX, %lu, %lu)\n",
        (long unsigned) hash1, (long unsigned) key, (long unsigned) hashcode);
#endif
    result_hashelem = NULL;
    hashelem = hash1->table[hashcode & hash1->mask];
    while (hashelem != NULL) {
/*
printf("sizeof(hashelem->key.value.genericvalue)=%lu\n", sizeof(hashelem->key.value.genericvalue));
printf("sizeof(key)=%lu\n", sizeof(key));
printf("%llX\n", hashelem->key.value.genericvalue);
printf("%lX\n", (long unsigned) hashelem->key.value.genericvalue);
printf("%llX\n", key);
printf("%lX\n", (long unsigned) key);
*/
      cmp = cmp_func(hashelem->key.value.genericvalue, key);
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (cmp == 0) {
        result_hashelem = hashelem;
        hashelem = NULL;
      } else {
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
#ifdef TRACE_HSH_RTL
    printf("END hshContains(%lX, %lu, %lu) ==> %d\n",
        (long unsigned) hash1, (long unsigned) key, (long unsigned) hashcode,
        result_hashelem != NULL);
#endif
    return(result_hashelem != NULL);
  } /* hshContains */



#ifdef ANSI_C

void hshCpy (rtlHashtype *const hash_to, const const_rtlHashtype hash_from,
    createfunctype key_create_func, destrfunctype key_destr_func,
    createfunctype data_create_func, destrfunctype data_destr_func)
#else

void hshCpy (hash_to, hash_from, key_create_func, key_destr_func,
    data_create_func, data_destr_func)
rtlHashtype *hash_to;
rtlHashtype hash_from;
createfunctype key_create_func;
destrfunctype key_destr_func;
createfunctype data_create_func;
destrfunctype data_destr_func;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;

  /* hshCpy */
    free_hash(*hash_to, key_destr_func, data_destr_func);
    *hash_to = copy_hash(hash_from,
        key_create_func, data_create_func, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      free_hash(*hash_to, key_destr_func,
          data_destr_func);
      *hash_to = NULL;
      raise_error(MEMORY_ERROR);
    } /* if */
  } /* hshCpy */



#ifdef ANSI_C

rtlHashtype hshCreate (const const_rtlHashtype hash_from,
    createfunctype key_create_func, destrfunctype key_destr_func,
    createfunctype data_create_func, destrfunctype data_destr_func)
#else

rtlHashtype hshCreate (hash_from, key_create_func, key_destr_func,
    data_create_func, data_destr_func)
rtlHashtype hash_from;
createfunctype key_create_func;
destrfunctype key_destr_func;
createfunctype data_create_func;
destrfunctype data_destr_func;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    rtlHashtype result;

  /* hshCreate */
    result = copy_hash(hash_from,
        key_create_func, data_create_func, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      free_hash(result, key_destr_func,
          data_destr_func);
      result = NULL;
      raise_error(MEMORY_ERROR);
    } /* if */
    return(result);
  } /* hshCreate */



#ifdef ANSI_C

void hshDestr (const const_rtlHashtype old_hash, destrfunctype key_destr_func,
    destrfunctype data_destr_func)
#else

void hshDestr (old_hash, key_destr_func, data_destr_func)
rtlHashtype old_hash;
destrfunctype key_destr_func;
destrfunctype data_destr_func;
#endif

{ /* hshDestr */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshDestr(%lX)\n", (unsigned long) old_hash);
#endif
    free_hash(old_hash, key_destr_func, data_destr_func);
#ifdef TRACE_HSH_RTL
    printf("END hshDestr\n");
#endif
  } /* hshDestr */



#ifdef ANSI_C

rtlHashtype hshEmpty (void)
#else

rtlHashtype hshEmpty ()
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    rtlHashtype result;

  /* hshEmpty */
    result = new_hash(TABLE_BITS, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      return(result);
    } /* if */
  } /* hshEmpty */



#ifdef ANSI_C

void hshExcl (const rtlHashtype hash1, const rtlGenerictype key,
    inttype hashcode, comparetype cmp_func, destrfunctype key_destr_func,
    destrfunctype data_destr_func)
#else

void hshExcl (hash1, key, hashcode, cmp_func, key_destr_func, data_destr_func)
rtlHashtype hash1;
rtlGenerictype key;
inttype hashcode;
comparetype cmp_func;
destrfunctype key_destr_func;
destrfunctype data_destr_func;
#endif

  {
    rtlHelemtype *delete_pos;
    rtlHelemtype hashelem;
    rtlHelemtype greater_hashelems;
    rtlHelemtype old_hashelem;
    int cmp;

  /* hshExcl */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshExcl(%lX, %lu, %lu) size=%lu\n",
        (unsigned long) hash1, (unsigned long) key,
        (unsigned long) hashcode, hash1->size);
#endif
    delete_pos = &hash1->table[hashcode & hash1->mask];
    hashelem = hash1->table[hashcode & hash1->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericvalue, key);
      if (cmp < 0) {
        delete_pos = &hashelem->next_less;
        hashelem = hashelem->next_less;
      } else if (cmp == 0) {
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
        hash1->size--;
        hashelem = NULL;
      } else {
        delete_pos = &hashelem->next_greater;
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
#ifdef TRACE_HSH_RTL
    printf("END hshExcl(%lX, %lu, %lu) size=%lu\n",
        (unsigned long) hash1, (unsigned long) key,
        (unsigned long) hashcode, hash1->size);
#endif
  } /* hshExcl */



#ifdef ANSI_C

rtlGenerictype hshIdx (const const_rtlHashtype hash1, const rtlGenerictype key,
    inttype hashcode, comparetype cmp_func)
#else

rtlGenerictype hshIdx (hash1, key, hashcode, cmp_func)
rtlHashtype hash1;
rtlGenerictype key;
inttype hashcode;
comparetype cmp_func;
#endif

  {
    rtlHelemtype hashelem;
    rtlHelemtype result_hashelem;
    int cmp;
    rtlGenerictype result;

  /* hshIdx */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshIdx(%lX, %lu, %lu)\n",
        (unsigned long) hash1, (unsigned long) key, (unsigned long) hashcode);
#endif
    result_hashelem = NULL;
    hashelem = hash1->table[hashcode & hash1->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericvalue, key);
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (cmp == 0) {
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
        (unsigned long) hash1, (unsigned long) key, (unsigned long) hashcode,
	(unsigned long) result,
        (unsigned long) (result != NULL ? *((rtlGenerictype *)result) : 0));
#endif
    return(result);
  } /* hshIdx */



#ifdef ANSI_C

rtlObjecttype *hshIdxAddr (const const_rtlHashtype hash1,
    const rtlGenerictype key, inttype hashcode, comparetype cmp_func)
#else

rtlObjecttype *hshIdxAddr (hash1, key, hashcode, cmp_func)
rtlHashtype hash1;
rtlGenerictype key;
inttype hashcode;
comparetype cmp_func;
#endif

  {
    rtlHelemtype hashelem;
    rtlHelemtype result_hashelem;
    int cmp;
    rtlObjecttype *result;

  /* hshIdxAddr */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshIdxAddr(%lX, %lu, %lu)\n",
        (unsigned long) hash1, (unsigned long) key, (unsigned long) hashcode);
#endif
    result_hashelem = NULL;
    hashelem = hash1->table[hashcode & hash1->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericvalue, key);
      /* printf(". %llu %llu cmp=%d\n",
          (unsigned long long) hashelem->key.value.genericvalue,
          (unsigned long long) key, cmp); */
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (cmp == 0) {
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
        (unsigned long) hash1, (unsigned long) key, (unsigned long) hashcode,
	(unsigned long) result,
        (unsigned long) (result != NULL ? *((rtlGenerictype *)result) : 0));
#endif
    return(result);
  } /* hshIdxAddr */



#ifdef ANSI_C

rtlGenerictype hshIdxWithDefault (const rtlHashtype hash1, const rtlGenerictype key,
    const rtlGenerictype data, inttype hashcode, comparetype cmp_func,
    createfunctype key_create_func, createfunctype data_create_func)
#else

rtlGenerictype hshIdxWithDefault (hash1, key, data, hashcode, cmp_func,
    key_create_func, data_create_func)
rtlHashtype hash1;
rtlGenerictype key;
rtlGenerictype data;
inttype hashcode;
comparetype cmp_func;
createfunctype key_create_func;
createfunctype data_create_func;
#endif

  {
    rtlHelemtype hashelem;
    rtlHelemtype result_hashelem;
    int cmp;
    errinfotype err_info = OKAY_NO_ERROR;
    rtlGenerictype result;

  /* hshIdxWithDefault */
    /* printf("hshIdxWithDefault(%lX, %llX, %lld, %llX, %lX, %lx, %lX)\n",
       hash1, key, data, hashcode, cmp_func, key_create_func, data_create_func); */
    hashelem = hash1->table[hashcode & hash1->mask];
    if (hashelem == NULL) {
      result_hashelem = new_helem(key, data,
          key_create_func, data_create_func, &err_info);
      hash1->table[hashcode & hash1->mask] = result_hashelem;
      hash1->size++;
    } else {
      do {
        /* printf("key=%llX\n", hashelem->key.value.genericvalue); */
        cmp = cmp_func(hashelem->key.value.genericvalue, key);
        if (cmp < 0) {
          if (hashelem->next_less == NULL) {
            result_hashelem = new_helem(key, data,
                key_create_func, data_create_func, &err_info);
            hashelem->next_less = result_hashelem;
            hash1->size++;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_less;
          } /* if */
        } else if (cmp == 0) {
          result_hashelem = hashelem;
          hashelem = NULL;
        } else {
          if (hashelem->next_greater == NULL) {
            result_hashelem = new_helem(key, data,
                key_create_func, data_create_func, &err_info);
            hashelem->next_greater = result_hashelem;
            hash1->size++;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_greater;
          } /* if */
        } /* if */
      } while (hashelem != NULL);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      hash1->size--;
      raise_error(MEMORY_ERROR);
      result = 0;
    } else {
      result = result_hashelem->data.value.genericvalue;
    } /* if */
    return(result);
  } /* hshIdxWithDefault */



#ifdef ANSI_C

void hshIncl (const rtlHashtype hash1, const rtlGenerictype key,
    const rtlGenerictype data, inttype hashcode, comparetype cmp_func,
    createfunctype key_create_func, createfunctype data_create_func,
    copyfunctype data_copy_func)
#else

void hshIncl (hash1, key, data, hashcode, cmp_func,
    key_create_func, data_create_func, data_copy_func)
rtlHashtype hash1;
rtlGenerictype key;
rtlGenerictype data;
inttype hashcode;
comparetype cmp_func;
createfunctype key_create_func;
createfunctype data_create_func;
copyfunctype data_copy_func;
#endif

  {
    rtlHelemtype hashelem;
    int cmp;
    errinfotype err_info = OKAY_NO_ERROR;

  /* hshIncl */
#ifdef TRACE_HSH_RTL
    printf("BEGIN hshIncl(%lX, %llu, %lu, %lu) size=%lu\n",
        (unsigned long) hash1, (unsigned long long) key,
        (unsigned long) data, (unsigned long) hashcode, hash1->size);
#endif
    hashelem = hash1->table[hashcode & hash1->mask];
    if (hashelem == NULL) {
      hash1->table[hashcode & hash1->mask] = new_helem(key, data,
          key_create_func, data_create_func, &err_info);
      /*
      hashelem = hash1->table[hashcode & hash1->mask];
      printf("key=%llX\n", (unsigned long long) key);
      printf("new hashelem: key=%llX, data=%llX\n",
          hashelem->key.value.intvalue, hashelem->data.value.intvalue);
      printf("cmp = %d\n", (int) cmp_func(hashelem->key.value.genericvalue, key));
      */
      hash1->size++;
    } else {
      do {
        cmp = cmp_func(hashelem->key.value.genericvalue, key);
        if (cmp < 0) {
          if (hashelem->next_less == NULL) {
            hashelem->next_less = new_helem(key, data,
                key_create_func, data_create_func, &err_info);
            hash1->size++;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_less;
          } /* if */
        } else if (cmp == 0) {
          data_copy_func(&hashelem->data.value.genericvalue, data);
          hashelem = NULL;
        } else {
          if (hashelem->next_greater == NULL) {
            hashelem->next_greater = new_helem(key, data,
                key_create_func, data_create_func, &err_info);
            hash1->size++;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_greater;
          } /* if */
        } /* if */
      } while (hashelem != NULL);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      hash1->size--;
      raise_error(MEMORY_ERROR);
    } /* if */
#ifdef TRACE_HSH_RTL
    printf("END hshIncl(%lX, %lu, %lu, %lu) size=%lu\n",
        (unsigned long) hash1, (unsigned long) key,
        (unsigned long) data, (unsigned long) hashcode, hash1->size);
#endif
  } /* hshIncl */



#ifdef ANSI_C

rtlArraytype hshKeys (const const_rtlHashtype hash1,
    createfunctype key_create_func, destrfunctype key_destr_func)
#else

rtlArraytype hshKeys (hash1, key_create_func, key_destr_func)
rtlHashtype hash1;
createfunctype key_create_func;
destrfunctype key_destr_func;
#endif

  {
    rtlArraytype key_array;
    errinfotype err_info = OKAY_NO_ERROR;

  /* hshKeys */
    key_array = keys_hash(hash1, key_create_func, key_destr_func,
        &err_info);
    return(key_array);
  } /* hshKeys */



#ifdef ANSI_C

rtlArraytype hshValues (const const_rtlHashtype hash1,
    createfunctype value_create_func, destrfunctype value_destr_func)
#else

rtlArraytype hshValues (hash1, value_create_func, value_destr_func)
rtlHashtype hash1;
createfunctype value_create_func;
destrfunctype value_destr_func;
#endif

  {
    rtlArraytype value_array;
    errinfotype err_info = OKAY_NO_ERROR;

  /* hshValues */
    value_array = values_hash(hash1, value_create_func, value_destr_func,
        &err_info);
    return(value_array);
  } /* hshValues */
