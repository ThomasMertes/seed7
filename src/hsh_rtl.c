/********************************************************************/
/*                                                                  */
/*  hsh_rtl.c     Primitive actions for the string type.            */
/*  Copyright (C) 1989 - 2007  Thomas Mertes                        */
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

static void free_hash (rtlHashtype old_hash, destrfunctype key_destr_func,
    destrfunctype data_destr_func)
#else

static void free_hash (old_hash, key_destr_func, data_destr_func)
rtlHashtype old_hash;
destrfunctype key_destr_func;
destrfunctype data_destr_func;
#endif

  {
    int number;
    rtlHelemtype *curr_helem;

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
    if (!ALLOC_RECORD(helem, rtlHelemrecord)) {
      *err_info = MEMORY_ERROR;
    } else {
      COUNT_RECORD(rtlHelemrecord, count.helem);
      helem->key.value.genericvalue = key_create_func(key);
      helem->data.value.genericvalue = data_create_func(data);
      helem->next_less = NULL;
      helem->next_greater = NULL;
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
      COUNT_RTL_HASH(TABLE_SIZE(bits));
      hash->bits = bits;
      hash->mask = TABLE_MASK(bits);
      hash->table_size = TABLE_SIZE(bits);
      hash->size = 0;
      memset(hash->table, 0, hash->table_size * sizeof(rtlHelemtype));
    } /* if */
    return(hash);
  } /* new_hash */



#ifdef ANSI_C

static rtlHelemtype copy_helem (rtlHelemtype source_helem, createfunctype key_create_func,
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
      if (!ALLOC_RECORD(dest_helem, rtlHelemrecord)) {
        *err_info = MEMORY_ERROR;
      } else {
        COUNT_RECORD(rtlHelemrecord, count.helem);
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

static rtlHashtype copy_hash (rtlHashtype source_hash, createfunctype key_create_func,
    createfunctype data_create_func, errinfotype *err_info)
#else

static rtlHashtype copy_hash (source_hash, key_create_func, data_create_func, err_info)
rtlHashtype source_hash;
createfunctype key_create_func;
createfunctype data_create_func;
errinfotype *err_info;
#endif

  {
    int new_size;
    int number;
    rtlHelemtype *source_helem;
    rtlHelemtype *dest_helem;
    rtlHashtype dest_hash;

  /* copy_hash */
    new_size = source_hash->table_size;
    if (!ALLOC_RTL_HASH(dest_hash, new_size)) {
      *err_info = MEMORY_ERROR;
    } else {
      COUNT_RTL_HASH(new_size);
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
    rtlHelemtype curr_helem, createfunctype key_create_func, errinfotype *err_info)
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

  /* keys_helem */
    array_size = (*key_array)->max_position - (*key_array)->min_position;
    if (*arr_pos >= array_size) {
      if (!RESIZE_RTL_ARRAY(*key_array, array_size, array_size + ARRAY_SIZE_INCREMENT)) {
        *err_info = MEMORY_ERROR;
        return;
      } else {
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

static rtlArraytype keys_hash (rtlHashtype curr_hash, createfunctype key_create_func,
    destrfunctype key_destr_func, errinfotype *err_info)
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
    rtlHelemtype *curr_helem;
    memsizetype array_size;
    rtlArraytype key_array;

  /* keys_hash */
    if (!ALLOC_RTL_ARRAY(key_array, ARRAY_SIZE_INCREMENT)) {
      *err_info = MEMORY_ERROR;
    } else {
      COUNT_RTL_ARRAY(ARRAY_SIZE_INCREMENT);
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
      if (*err_info != OKAY_NO_ERROR) {
        for (number = 0; number < arr_pos; number++) {
          key_destr_func(key_array->arr[number].value.genericvalue);
        } /* for */
        FREE_RTL_ARRAY(key_array, array_size);
        key_array = NULL;
      } else {
        if (!RESIZE_RTL_ARRAY(key_array, array_size, arr_pos)) {
          FREE_RTL_ARRAY(key_array, array_size);
          key_array = NULL;
          *err_info = MEMORY_ERROR;
        } else {
          COUNT3_RTL_ARRAY(array_size, arr_pos);
          key_array->max_position = arr_pos;
        } /* if */
      } /* if */
    } /* if */
    return(key_array);
  } /* keys_hash */



#ifdef ANSI_C

static void values_helem (rtlArraytype *value_array, memsizetype *arr_pos,
    rtlHelemtype curr_helem, createfunctype value_create_func, errinfotype *err_info)
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

  /* values_helem */
    array_size = (*value_array)->max_position - (*value_array)->min_position;
    if (*arr_pos >= array_size) {
      if (!RESIZE_RTL_ARRAY(*value_array, array_size, array_size + ARRAY_SIZE_INCREMENT)) {
        *err_info = MEMORY_ERROR;
        return;
      } else {
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

static rtlArraytype values_hash (rtlHashtype curr_hash, createfunctype value_create_func,
    destrfunctype value_destr_func, errinfotype *err_info)
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
    rtlHelemtype *curr_helem;
    memsizetype array_size;
    rtlArraytype value_array;

  /* values_hash */
    if (!ALLOC_RTL_ARRAY(value_array, ARRAY_SIZE_INCREMENT)) {
      *err_info = MEMORY_ERROR;
    } else {
      COUNT_RTL_ARRAY(ARRAY_SIZE_INCREMENT);
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
      if (*err_info != OKAY_NO_ERROR) {
        for (number = 0; number < arr_pos; number++) {
          value_destr_func(value_array->arr[number].value.genericvalue);
        } /* for */
        FREE_RTL_ARRAY(value_array, array_size);
        value_array = NULL;
      } else {
        if (!RESIZE_RTL_ARRAY(value_array, array_size, arr_pos)) {
          FREE_RTL_ARRAY(value_array, array_size);
          value_array = NULL;
          *err_info = MEMORY_ERROR;
        } else {
          COUNT3_RTL_ARRAY(array_size, arr_pos);
          value_array->max_position = arr_pos;
        } /* if */
      } /* if */
    } /* if */
    return(value_array);
  } /* values_hash */



#ifdef ANSI_C

booltype hshContains (rtlHashtype hash1, rtlGenerictype key,
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
    return(result_hashelem != NULL);
  } /* hshContains */



#ifdef ANSI_C

void hshCpy (rtlHashtype *hash_to, rtlHashtype hash_from,
    createfunctype key_create_func, createfunctype data_create_func,
    destrfunctype key_destr_func, destrfunctype data_destr_func)
#else

void hshCpy (hash_to, hash_from, key_create_func, data_create_func,
    key_destr_func, data_destr_func)
rtlHashtype *hash_to;
rtlHashtype hash_from;
createfunctype key_create_func;
createfunctype data_create_func;
destrfunctype key_destr_func;
destrfunctype data_destr_func;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;

  /* hshCpy */
    free_hash(*hash_to, key_destr_func, data_destr_func);
    if (err_info != OKAY_NO_ERROR) {
      *hash_to = NULL;
      raise_error(MEMORY_ERROR);
    } else {
      *hash_to = copy_hash(hash_from,
          key_create_func, data_create_func, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        free_hash(*hash_to, key_destr_func,
            data_destr_func);
        *hash_to = NULL;
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
  } /* hshCpy */



#ifdef ANSI_C

rtlHashtype hshCreate (rtlHashtype hash_from, createfunctype key_create_func,
    createfunctype data_create_func, destrfunctype key_destr_func,
    destrfunctype data_destr_func)
#else

rtlHashtype hshCreate (hash_from, key_create_func, data_create_func,
    key_destr_func, data_destr_func)
rtlHashtype hash_from;
createfunctype key_create_func;
createfunctype data_create_func;
destrfunctype key_destr_func;
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

void hshDestr (rtlHashtype old_hash, destrfunctype key_destr_func,
    destrfunctype data_destr_func)
#else

void hshDestr (old_hash, key_destr_func, data_destr_func)
rtlHashtype old_hash;
destrfunctype key_destr_func;
destrfunctype data_destr_func;
#endif

{ /* hshDestr */
    free_hash(old_hash, key_destr_func, data_destr_func);
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

void hshExcl (rtlHashtype hash1, rtlGenerictype key, inttype hashcode,
    comparetype cmp_func, destrfunctype key_destr_func,
    destrfunctype data_destr_func)
#else

void hshExcl (hash1, key, hashcode, cmp_func)
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
    delete_pos = &hash1->table[hashcode & hash1->mask];
    hashelem = hash1->table[hashcode & hash1->mask];
    while (hashelem != NULL) {
      cmp = cmp_func(hashelem->key.value.genericvalue, key);
      if (cmp < 0) {
        delete_pos = &hashelem->next_less;
        hashelem = hashelem->next_less;
      } else if (cmp == 0) {
        if (hashelem->next_less == NULL) {
          *delete_pos = hashelem->next_greater;
        } else if (hashelem->next_greater == NULL) {
          *delete_pos = hashelem->next_less;
        } else {
          old_hashelem = hashelem;
          *delete_pos = hashelem->next_less;
          greater_hashelems = hashelem->next_greater;
          hashelem = hashelem->next_less;
          while (hashelem->next_greater != NULL) {
            hashelem = hashelem->next_greater;
          } /* while */
          hashelem->next_greater = greater_hashelems;
          old_hashelem->next_less = NULL;
          old_hashelem->next_greater = NULL;
          free_helem(old_hashelem, key_destr_func,
              data_destr_func);
          hash1->size--;
        } /* if */
        hashelem = NULL;
      } else {
        delete_pos = &hashelem->next_greater;
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
  } /* hshExcl */



#ifdef ANSI_C

rtlGenerictype hshIdx (rtlHashtype hash1, rtlGenerictype key,
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
      return(0);
    } /* if */
    return(result);
  } /* hshIdx */



#ifdef ANSI_C

rtlObjecttype *hshIdxAddr (rtlHashtype hash1, rtlGenerictype key,
    inttype hashcode, comparetype cmp_func)
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
      result = &result_hashelem->data;
    } else {
      raise_error(RANGE_ERROR);
      return(0);
    } /* if */
    return(result);
  } /* hshIdxAddr */



#ifdef ANSI_C

void hshIncl (rtlHashtype hash1, rtlGenerictype key,
    rtlGenerictype data, inttype hashcode, comparetype cmp_func,
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
    hashelem = hash1->table[hashcode & hash1->mask];
    if (hashelem == NULL) {
      hash1->table[hashcode & hash1->mask] = new_helem(key, data,
          key_create_func, data_create_func, &err_info);
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
      return;
    } /* if */
  } /* hshIncl */



#ifdef ANSI_C

rtlArraytype hshKeys (rtlHashtype hash1, createfunctype key_create_func,
    destrfunctype key_destr_func)
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

rtlArraytype hshValues (rtlHashtype hash1, createfunctype value_create_func,
    destrfunctype value_destr_func)
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
