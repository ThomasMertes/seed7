/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2019, 2021, 2022, 2024  Thomas Mertes      */
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
/*  File: seed7/src/hshlib.c                                        */
/*  Changes: 2005, 2013, 2016, 2018, 2021, 2022  Thomas Mertes      */
/*           2024  Thomas Mertes                                    */
/*  Content: All primitive actions for hash types.                  */
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
#include "listutl.h"
#include "traceutl.h"
#include "executl.h"
#include "objutl.h"
#include "exec.h"
#include "runerr.h"
#include "rtl_err.h"
#include "int_rtl.h"

#undef EXTERN
#define EXTERN
#include "hshlib.h"


#define TABLE_BITS 10
#define TABLE_SIZE(bits) ((unsigned int) 1 << (bits))
#define TABLE_MASK(bits) (TABLE_SIZE(bits)-1)

typedef struct hashElemStruct *const hashElemType_const;



static memSizeType free_helem (hashElemType old_helem, objectType key_destr_func,
    objectType data_destr_func)

  {
    memSizeType freed = 1;

  /* free_helem */
    if (CATEGORY_OF_OBJ(&old_helem->key) != FORWARDOBJECT) {
      /* FORWARDOBJECT is used as magic category in hsh_rand_key */
      param2_call(key_destr_func, &old_helem->key, SYS_DESTR_OBJECT);
    } /* if */
    if (CATEGORY_OF_OBJ(&old_helem->data) != FORWARDOBJECT) {
      /* FORWARDOBJECT is used as magic category in hsh_idx */
      param2_call(data_destr_func, &old_helem->data, SYS_DESTR_OBJECT);
    } /* if */
    if (old_helem->next_less != NULL) {
      freed += free_helem(old_helem->next_less, key_destr_func,
                          data_destr_func);
    } /* if */
    if (old_helem->next_greater != NULL) {
      freed += free_helem(old_helem->next_greater, key_destr_func,
                          data_destr_func);
    } /* if */
    FREE_RECORD(old_helem, hashElemRecord, count.helem);
    return freed;
  } /* free_helem */



static void free_hash (hashType old_hash, objectType key_destr_func,
    objectType data_destr_func)

  {
    memSizeType to_free;
    unsigned int number;
    hashElemType *table;

  /* free_hash */
    logFunction(printf("free_hash(" FMT_X_MEM " (size=" FMT_U_MEM "), ...)\n",
                       (memSizeType) old_hash,
                       old_hash == NULL ? 0 : old_hash->size););
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
      FREE_HASH(old_hash, old_hash->table_size);
    } /* if */
    logFunction(printf("free_hash -->\n"););
  } /* free_hash */



static hashElemType new_helem (objectType key, objectType data,
    objectType key_create_func, objectType data_create_func, errInfoType *err_info)

  {
    hashElemType helem;

  /* new_helem */
    if (unlikely(!ALLOC_RECORD(helem, hashElemRecord, count.helem))) {
      *err_info = MEMORY_ERROR;
    } else {
      helem->key.descriptor.property = NULL;
      INIT_CATEGORY_OF_VAR(&helem->key, DECLAREDOBJECT);
      helem->key.type_of = key->type_of;
      param3_call(key_create_func, &helem->key, SYS_CREA_OBJECT, key);
      helem->data.descriptor.property = NULL;
      INIT_CATEGORY_OF_VAR(&helem->data, DECLAREDOBJECT);
      helem->data.type_of = data->type_of;
      param3_call(data_create_func, &helem->data, SYS_CREA_OBJECT, data);
      helem->next_less = NULL;
      helem->next_greater = NULL;
    } /* if */
    return helem;
  } /* new_helem */



static hashType new_hash (unsigned int bits)

  {
    hashType hash;

  /* new_hash */
    if (likely(ALLOC_HASH(hash, TABLE_SIZE(bits)))) {
      hash->bits = bits;
      hash->mask = TABLE_MASK(bits);
      hash->table_size = TABLE_SIZE(bits);
      hash->size = 0;
      memset(hash->table, 0, hash->table_size * sizeof(hashElemType));
    } /* if */
    return hash;
  } /* new_hash */



static hashElemType create_helem (hashElemType source_helem,
    objectType key_create_func, objectType data_create_func,
    errInfoType *err_info)

  {
    hashElemType dest_helem;

  /* create_helem */
    if (unlikely(!ALLOC_RECORD(dest_helem, hashElemRecord, count.helem))) {
      *err_info = MEMORY_ERROR;
    } else {
      memcpy(&dest_helem->key.descriptor, &source_helem->key.descriptor,
             sizeof(descriptorUnion));
      INIT_CATEGORY_OF_VAR(&dest_helem->key, DECLAREDOBJECT);
      SET_ANY_FLAG(&dest_helem->key, HAS_POSINFO(&source_helem->key));
      dest_helem->key.type_of = source_helem->key.type_of;
      param3_call(key_create_func, &dest_helem->key, SYS_CREA_OBJECT, &source_helem->key);
      memcpy(&dest_helem->data.descriptor, &source_helem->data.descriptor,
             sizeof(descriptorUnion));
      INIT_CATEGORY_OF_VAR(&dest_helem->data, DECLAREDOBJECT);
      SET_ANY_FLAG(&dest_helem->data, HAS_POSINFO(&source_helem->data));
      dest_helem->data.type_of = source_helem->data.type_of;
      param3_call(data_create_func, &dest_helem->data, SYS_CREA_OBJECT, &source_helem->data);
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



static hashType create_hash (hashType source_hash,
    objectType key_create_func, objectType data_create_func,
    errInfoType *err_info)

  {
    unsigned int table_size;
    unsigned int number;
    hashElemType *source_helem;
    hashElemType *dest_helem;
    hashType dest_hash;

  /* create_hash */
    table_size = source_hash->table_size;
    if (unlikely(!ALLOC_HASH(dest_hash, table_size))) {
      *err_info = MEMORY_ERROR;
    } else {
      dest_hash->bits = source_hash->bits;
      dest_hash->mask = source_hash->mask;
      dest_hash->table_size = table_size;
      dest_hash->size = source_hash->size;
      if (source_hash->size == 0) {
        memset(dest_hash->table, 0, table_size * sizeof(hashElemType));
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
    logFunction(printf("create_hash --> " FMT_X_MEM "\n",
                       (memSizeType) dest_hash););
    return dest_hash;
  } /* create_hash */



static memSizeType keys_helem (const arrayType key_array, memSizeType arr_pos,
    hashElemType curr_helem, objectType key_create_func)

  {
    objectType dest_obj;

  /* keys_helem */
    do {
      arr_pos--;
      dest_obj = &key_array->arr[arr_pos];
      memcpy(&dest_obj->descriptor, &curr_helem->key.descriptor, sizeof(descriptorUnion));
      INIT_CATEGORY_OF_VAR(dest_obj, DECLAREDOBJECT);
      SET_ANY_FLAG(dest_obj, HAS_POSINFO(&curr_helem->key));
      dest_obj->type_of = curr_helem->key.type_of;
      param3_call(key_create_func, dest_obj, SYS_CREA_OBJECT, &curr_helem->key);
      if (curr_helem->next_less != NULL) {
        arr_pos = keys_helem(key_array, arr_pos, curr_helem->next_less,
                             key_create_func);
      } /* if */
      curr_helem = curr_helem->next_greater;
    } while (curr_helem != NULL);
    return arr_pos;
  } /* keys_helem */



static inline arrayType keys_hash (const const_hashType curr_hash,
    objectType key_create_func, objectType key_destr_func)

  {
    memSizeType arr_pos;
    memSizeType number;
    const hashElemType *table;
    arrayType key_array;

  /* keys_hash */
    if (unlikely(curr_hash->size > INTTYPE_MAX ||
                 !ALLOC_ARRAY(key_array, curr_hash->size))) {
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
        } while (arr_pos != 0 && !fail_flag);
        if (unlikely(fail_flag)) {
          for (; arr_pos < curr_hash->size; arr_pos++) {
            param2_call(key_destr_func, &key_array->arr[arr_pos], SYS_DESTR_OBJECT);
          } /* for */
          FREE_ARRAY(key_array, curr_hash->size);
          key_array = NULL;
        } /* if */
      } /* if */
    } /* if */
    return key_array;
  } /* keys_hash */



static memSizeType values_helem (const arrayType value_array, memSizeType arr_pos,
    hashElemType curr_helem, const objectType value_create_func)

  {
    objectType dest_obj;

  /* values_helem */
    do {
      arr_pos--;
      dest_obj = &value_array->arr[arr_pos];
      memcpy(&dest_obj->descriptor, &curr_helem->data.descriptor, sizeof(descriptorUnion));
      INIT_CATEGORY_OF_VAR(dest_obj, DECLAREDOBJECT);
      SET_ANY_FLAG(dest_obj, HAS_POSINFO(&curr_helem->data));
      dest_obj->type_of = curr_helem->data.type_of;
      param3_call(value_create_func, dest_obj, SYS_CREA_OBJECT, &curr_helem->data);
      if (curr_helem->next_less != NULL) {
        arr_pos = values_helem(value_array, arr_pos, curr_helem->next_less,
                               value_create_func);
      } /* if */
      curr_helem = curr_helem->next_greater;
    } while (curr_helem != NULL);
    return arr_pos;
  } /* values_helem */



static inline arrayType values_hash (const const_hashType curr_hash,
    const objectType value_create_func, const objectType value_destr_func)

  {
    memSizeType arr_pos;
    memSizeType number;
    const hashElemType *table;
    arrayType value_array;

  /* values_hash */
    if (unlikely(curr_hash->size > INTTYPE_MAX ||
                 !ALLOC_ARRAY(value_array, curr_hash->size))) {
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
        } while (arr_pos != 0 && !fail_flag);
        if (unlikely(fail_flag)) {
          for (; arr_pos < curr_hash->size; arr_pos++) {
            param2_call(value_destr_func, &value_array->arr[arr_pos], SYS_DESTR_OBJECT);
          } /* for */
          FREE_ARRAY(value_array, curr_hash->size);
          value_array = NULL;
        } /* if */
      } /* if */
    } /* if */
    return value_array;
  } /* values_hash */



static memSizeType get_helem_elem (hashElemType *hash_elem,
    memSizeType arr_pos, hashElemType curr_helem)

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



static inline hashElemType get_hash_elem (const const_hashType curr_hash,
    memSizeType arr_pos)

  {
    memSizeType number;
    const hashElemType *table;
    hashElemType hash_elem = NULL;

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



static void for_helem (objectType for_variable, hashElemType curr_helem,
    objectType statement, objectType data_copy_func)

  { /* for_helem */
    param3_call(data_copy_func, for_variable, SYS_ASSIGN_OBJECT, &curr_helem->data);
    evaluate(statement);
    if (curr_helem->next_less != NULL) {
      for_helem(for_variable, curr_helem->next_less, statement, data_copy_func);
    } /* if */
    if (curr_helem->next_greater != NULL) {
      for_helem(for_variable, curr_helem->next_greater, statement, data_copy_func);
    } /* if */
  } /* for_helem */



static void for_hash (objectType for_variable, hashType curr_hash,
    objectType statement, objectType data_copy_func)

  {
    unsigned int number;
    hashElemType_const *table_elem;

  /* for_hash */
    number = curr_hash->table_size;
    table_elem = curr_hash->table;
    while (number != 0) {
      if (*table_elem != NULL) {
        for_helem(for_variable, *table_elem, statement, data_copy_func);
      } /* if */
      number--;
      table_elem++;
    } /* while */
  } /* for_hash */



static void for_key_helem (objectType key_variable, hashElemType curr_helem,
    objectType statement, objectType key_copy_func)

  { /* for_key_helem */
    param3_call(key_copy_func, key_variable, SYS_ASSIGN_OBJECT, &curr_helem->key);
    evaluate(statement);
    if (curr_helem->next_less != NULL) {
      for_key_helem(key_variable, curr_helem->next_less, statement, key_copy_func);
    } /* if */
    if (curr_helem->next_greater != NULL) {
      for_key_helem(key_variable, curr_helem->next_greater, statement, key_copy_func);
    } /* if */
  } /* for_key_helem */



static void for_key_hash (objectType key_variable, hashType curr_hash,
    objectType statement, objectType key_copy_func)

  {
    unsigned int number;
    hashElemType_const *table_elem;

  /* for_key_hash */
    number = curr_hash->table_size;
    table_elem = curr_hash->table;
    while (number != 0) {
      if (*table_elem != NULL) {
        for_key_helem(key_variable, *table_elem, statement, key_copy_func);
      } /* if */
      number--;
      table_elem++;
    } /* while */
  } /* for_key_hash */



static void for_data_key_helem (objectType for_variable, objectType key_variable,
    hashElemType curr_helem, objectType statement, objectType data_copy_func,
    objectType key_copy_func)

  { /* for_data_key_helem */
    param3_call(data_copy_func, for_variable, SYS_ASSIGN_OBJECT, &curr_helem->data);
    param3_call(key_copy_func, key_variable, SYS_ASSIGN_OBJECT, &curr_helem->key);
    evaluate(statement);
    if (curr_helem->next_less != NULL) {
      for_data_key_helem(for_variable, key_variable, curr_helem->next_less, statement,
          data_copy_func, key_copy_func);
    } /* if */
    if (curr_helem->next_greater != NULL) {
      for_data_key_helem(for_variable, key_variable, curr_helem->next_greater, statement,
          data_copy_func, key_copy_func);
    } /* if */
  } /* for_data_key_helem */



static void for_data_key_hash (objectType for_variable, objectType key_variable,
    hashType curr_hash, objectType statement, objectType data_copy_func,
    objectType key_copy_func)

  {
    unsigned int number;
    hashElemType_const *table_elem;

  /* for_data_key_hash */
    number = curr_hash->table_size;
    table_elem = curr_hash->table;
    while (number != 0) {
      if (*table_elem != NULL) {
        for_data_key_helem(for_variable, key_variable, *table_elem, statement,
            data_copy_func, key_copy_func);
      } /* if */
      number--;
      table_elem++;
    } /* while */
  } /* for_data_key_hash */



objectType hsh_concat_key_value (listType arguments)

  {
    hashElemType element1;
    hashElemType element2;
    hashElemType lastElement1;
    hashElemType lastElement2;

  /* hsh_concat_key_value */
    isit_hashelem(arg_1(arguments));
    isit_hashelem(arg_3(arguments));
    element1 = take_hashelem(arg_1(arguments));
    element2 = take_hashelem(arg_3(arguments));
    logFunction(printf("hsh_concat_key_value(" FMT_X_MEM ", " FMT_X_MEM ")\n",
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
    if (TEMP_OBJECT(arg_1(arguments))) {
      arg_1(arguments)->value.hashElemValue = NULL;
    } else {
      logError(printf("hsh_concat_key_value: The keyValuePairs must be temporary.\n"););
      return raise_exception(SYS_ACT_ILLEGAL_EXCEPTION);
    } /* if */
    if (TEMP_OBJECT(arg_3(arguments))) {
      arg_3(arguments)->value.hashElemValue = NULL;
    } else {
      logError(printf("hsh_concat_key_value: The keyValuePairs must be temporary.\n"););
      return raise_exception(SYS_ACT_ILLEGAL_EXCEPTION);
    } /* if */
    logFunction(printf("hsh_concat_key_value --> " FMT_X_MEM "\n",
                       (memSizeType) element1););
    return bld_hashelem_temp(element1);
  } /* hsh_concat_key_value */



/**
 *  Hash membership test.
 *  Determine if 'aKey/arg_2' is a member of the hash map 'aHashMap/arg_1'.
 *  @return TRUE if 'aKey/arg_2' is a member of 'aHashMap/arg_1',
 *          FALSE otherwise.
 */
objectType hsh_contains (listType arguments)

  {
    hashType aHashMap;
    intType hashcode;
    objectType aKey;
    objectType cmp_func;
    hashElemType hashelem;
    objectType cmp_obj;
    intType cmp;
    objectType result;

  /* hsh_contains */
    isit_hash(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_reference(arg_4(arguments));
    aHashMap =      take_hash(arg_1(arguments));
    aKey     =                arg_2(arguments);
    hashcode =       take_int(arg_3(arguments));
    cmp_func = take_reference(arg_4(arguments));
    isit_not_null(cmp_func);
    result = SYS_FALSE_OBJECT;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
      isit_not_null(cmp_obj);
      isit_int(cmp_obj);
      cmp = take_int(cmp_obj);
      FREE_OBJECT(cmp_obj);
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (cmp == 0) {
        result = SYS_TRUE_OBJECT;
        hashelem = NULL;
      } else {
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
    return result;
  } /* hsh_contains */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType hsh_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    hashType hsh_dest;
    hashType hsh_source;
    objectType key_create_func;
    objectType key_destr_func;
    objectType data_create_func;
    objectType data_destr_func;
    objectType hash_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;

  /* hsh_cpy */
    dest   = arg_1(arguments);
    source = arg_2(arguments);
    hash_exec_object = curr_exec_object;
    isit_hash(dest);
    isit_hash(source);
    is_variable(dest);
    hsh_dest = take_hash(dest);
    hsh_source = take_hash(source);
    /* The following check avoids an error for: aHash := aHash; */
    if (hsh_dest != hsh_source) {
      key_create_func  = take_reference(arg_3(arguments));
      key_destr_func   = take_reference(arg_4(arguments));
      data_create_func = take_reference(arg_5(arguments));
      data_destr_func  = take_reference(arg_6(arguments));
      isit_not_null(key_create_func);
      isit_not_null(key_destr_func);
      isit_not_null(data_create_func);
      isit_not_null(data_destr_func);
      free_hash(hsh_dest, key_destr_func, data_destr_func);
      if (TEMP2_OBJECT(source)) {
        dest->value.hashValue = hsh_source;
        source->value.hashValue = NULL;
      } else {
        dest->value.hashValue = create_hash(hsh_source,
            key_create_func, data_create_func, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          free_hash(dest->value.hashValue, key_destr_func,
              data_destr_func);
          dest->value.hashValue = NULL;
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         hash_exec_object, arguments);
        } /* if */
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* hsh_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType hsh_create (listType arguments)

  {
    objectType dest;
    objectType source;
    hashType hsh_source;
    objectType key_create_func;
    objectType key_destr_func;
    objectType data_create_func;
    objectType data_destr_func;
    objectType hash_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;

  /* hsh_create */
    dest   = arg_1(arguments);
    source = arg_2(arguments);
    isit_hash(source);
    hsh_source = take_hash(source);
    key_create_func  = take_reference(arg_3(arguments));
    key_destr_func   = take_reference(arg_4(arguments));
    data_create_func = take_reference(arg_5(arguments));
    data_destr_func  = take_reference(arg_6(arguments));
    hash_exec_object = curr_exec_object;
    isit_not_null(key_create_func);
    isit_not_null(key_destr_func);
    isit_not_null(data_create_func);
    isit_not_null(data_destr_func);
    logFunction(printf("hsh_create(" FMT_U_MEM ", " FMT_X_MEM ", "
                       FMT_X_MEM ", " FMT_X_MEM ", " FMT_X_MEM ", "
                       FMT_X_MEM ")\n",
                       (memSizeType) dest, (memSizeType) hsh_source,
                       (memSizeType) key_create_func,
                       (memSizeType) key_destr_func,
                       (memSizeType) data_create_func,
                       (memSizeType) data_destr_func););
    SET_CATEGORY_OF_OBJ(dest, HASHOBJECT);
    if (TEMP2_OBJECT(source)) {
      dest->value.hashValue = hsh_source;
      source->value.hashValue = NULL;
    } else {
      dest->value.hashValue = create_hash(hsh_source,
          key_create_func, data_create_func, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        free_hash(dest->value.hashValue, key_destr_func,
            data_destr_func);
        dest->value.hashValue = NULL;
        return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                       hash_exec_object, arguments);
      } /* if */
    } /* if */
    logFunction(printf("hsh_create --> " FMT_X_MEM "\n",
                       (memSizeType) dest->value.hashValue););
    return SYS_EMPTY_OBJECT;
  } /* hsh_create */



/**
 *  Free the memory referred by 'old_hash/arg_1'.
 *  After hsh_destr is left 'old_hash/arg_1' is NULL.
 *  The memory where 'old_hash/arg_1' is stored can be freed afterwards.
 */
objectType hsh_destr (listType arguments)

  {
    hashType old_hash;
    objectType key_destr_func;
    objectType data_destr_func;

  /* hsh_destr */
    isit_hash(arg_1(arguments));
    old_hash        =      take_hash(arg_1(arguments));
    key_destr_func  = take_reference(arg_2(arguments));
    data_destr_func = take_reference(arg_3(arguments));
    isit_not_null(key_destr_func);
    isit_not_null(data_destr_func);
    logFunction(printf("hsh_destr(" FMT_X_MEM ", " FMT_X_MEM ", "
                       FMT_X_MEM ")\n",
                       (memSizeType) old_hash,
                       (memSizeType) key_destr_func,
                       (memSizeType) data_destr_func););
    free_hash(old_hash, key_destr_func, data_destr_func);
    arg_1(arguments)->value.hashValue = NULL;
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* hsh_destr */



/**
 *  Create an empty hash table.
 *  @return an empty hash table.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
objectType hsh_empty (listType arguments)

  {
    hashType result;

  /* hsh_empty */
    result = new_hash(TABLE_BITS);
    if (unlikely(result == NULL)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_hash_temp(result);
    } /* if */
  } /* hsh_empty */



/**
 *  Remove the element with the key 'aKey/arg_2' from the hash map 'aHashMap/arg_1'.
 */
objectType hsh_excl (listType arguments)

  {
    hashType aHashMap;
    intType hashcode;
    objectType aKey;
    objectType cmp_func;
    objectType key_destr_func;
    objectType data_destr_func;
    hashElemType *delete_pos;
    hashElemType hashelem;
    hashElemType greater_hashelems;
    hashElemType old_hashelem;
    objectType cmp_obj;
    intType cmp;

  /* hsh_excl */
    isit_hash(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_reference(arg_4(arguments));
    aHashMap        =      take_hash(arg_1(arguments));
    aKey            =                arg_2(arguments);
    hashcode        =       take_int(arg_3(arguments));
    cmp_func        = take_reference(arg_4(arguments));
    key_destr_func  = take_reference(arg_5(arguments));
    data_destr_func = take_reference(arg_6(arguments));
    isit_not_null(cmp_func);
    isit_not_null(key_destr_func);
    isit_not_null(data_destr_func);
    delete_pos = &aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
      isit_not_null(cmp_obj);
      isit_int(cmp_obj);
      cmp = take_int(cmp_obj);
      FREE_OBJECT(cmp_obj);
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
        aHashMap->size--;
        hashelem = NULL;
      } else {
        delete_pos = &hashelem->next_greater;
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
    return SYS_EMPTY_OBJECT;
  } /* hsh_excl */



/**
 *  For-loop where 'forVar/arg_1' loops over the values of the hash map 'aHashMap/arg_2'.
 */
objectType hsh_for (listType arguments)

  {
    objectType for_variable;
    hashType aHashMap;
    objectType statement;
    objectType data_copy_func;

  /* hsh_for */
    isit_hash(arg_2(arguments));
    for_variable = arg_1(arguments);
    aHashMap = take_hash(arg_2(arguments));
    statement = arg_3(arguments);
    data_copy_func = take_reference(arg_4(arguments));
    isit_not_null(data_copy_func);
    for_hash(for_variable, aHashMap, statement, data_copy_func);
    return SYS_EMPTY_OBJECT;
  } /* hsh_for */



/**
 *  For-loop where 'forVar/arg_1' and 'keyVar/arg_2' loop over the hash map 'aHashMap/arg_3'.
 */
objectType hsh_for_data_key (listType arguments)

  {
    objectType key_variable;
    objectType for_variable;
    hashType aHashMap;
    objectType statement;
    objectType data_copy_func;
    objectType key_copy_func;

  /* hsh_for_data_key */
    isit_hash(arg_3(arguments));
    for_variable = arg_1(arguments);
    key_variable = arg_2(arguments);
    aHashMap = take_hash(arg_3(arguments));
    statement = arg_4(arguments);
    data_copy_func = take_reference(arg_5(arguments));
    key_copy_func = take_reference(arg_6(arguments));
    isit_not_null(data_copy_func);
    isit_not_null(key_copy_func);
    for_data_key_hash(for_variable, key_variable, aHashMap, statement,
        data_copy_func, key_copy_func);
    return SYS_EMPTY_OBJECT;
  } /* hsh_for_data_key */



/**
 *  For-loop where 'keyVar/arg_1' loops over the keys of the hash map 'aHashMap/arg_2'.
 */
objectType hsh_for_key (listType arguments)

  {
    objectType key_variable;
    hashType aHashMap;
    objectType statement;
    objectType key_copy_func;

  /* hsh_for_key */
    isit_hash(arg_2(arguments));
    key_variable = arg_1(arguments);
    aHashMap = take_hash(arg_2(arguments));
    statement = arg_3(arguments);
    key_copy_func = take_reference(arg_4(arguments));
    isit_not_null(key_copy_func);
    for_key_hash(key_variable, aHashMap, statement, key_copy_func);
    return SYS_EMPTY_OBJECT;
  } /* hsh_for_key */



objectType hsh_gen_hash (listType arguments)

  {
    hashElemType keyValuePairs;
    objectType key_hash_code_func;
    objectType cmp_func;
    objectType key_destr_func;
    objectType data_destr_func;
    hashElemType currentKeyValue;
    objectType hashCodeObj;
    unsigned int hashCode;
    hashElemType hashElem;
    objectType cmp_obj;
    intType cmp;
    objectType hash_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;
    hashType aHashMap;

  /* hsh_gen_hash */
    isit_hashelem(arg_1(arguments));
    keyValuePairs      = take_hashelem(arg_1(arguments));
    logFunction(printf("hsh_gen_hash(" FMT_X_MEM ", ...)\n",
                       (memSizeType) keyValuePairs););
    key_hash_code_func = take_reference(arg_2(arguments));
    cmp_func           = take_reference(arg_3(arguments));
    key_destr_func     = take_reference(arg_4(arguments));
    data_destr_func    = take_reference(arg_5(arguments));
    hash_exec_object = curr_exec_object;
    isit_not_null(key_hash_code_func);
    isit_not_null(cmp_func);
    isit_not_null(key_destr_func);
    isit_not_null(data_destr_func);
    if (TEMP_OBJECT(arg_1(arguments))) {
      arg_1(arguments)->value.hashElemValue = NULL;
    } else {
      logError(printf("hsh_gen_hash: The keyValuePairs must be temporary.\n"););
      return raise_exception(SYS_ACT_ILLEGAL_EXCEPTION);
    } /* if */
    aHashMap = new_hash(TABLE_BITS);
    if (unlikely(aHashMap == NULL)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      while (keyValuePairs != NULL) {
        currentKeyValue = keyValuePairs;
        keyValuePairs = keyValuePairs->next_greater;
        currentKeyValue->next_less = NULL;
        currentKeyValue->next_greater = NULL;
        hashCodeObj = param2_call(key_hash_code_func, &currentKeyValue->key, key_hash_code_func);
        isit_not_null(hashCodeObj);
        isit_int(hashCodeObj);
        hashCode = (unsigned int) take_int(hashCodeObj);
        FREE_OBJECT(hashCodeObj);
        hashElem = aHashMap->table[hashCode & aHashMap->mask];
        if (hashElem == NULL) {
          aHashMap->table[hashCode & aHashMap->mask] = currentKeyValue;
          aHashMap->size++;
        } else {
          do {
            cmp_obj = param3_call(cmp_func, &hashElem->key, &currentKeyValue->key, cmp_func);
            isit_not_null(cmp_obj);
            isit_int(cmp_obj);
            cmp = take_int(cmp_obj);
            FREE_OBJECT(cmp_obj);
            if (cmp < 0) {
              if (hashElem->next_less == NULL) {
                hashElem->next_less = currentKeyValue;
                aHashMap->size++;
                hashElem = NULL;
              } else {
                hashElem = hashElem->next_less;
              } /* if */
            } else if (cmp == 0) {
              logError(printf("hsh_gen_hash: A key is used twice.\n"););
              param2_call(key_destr_func, &currentKeyValue->key, SYS_DESTR_OBJECT);
              param2_call(data_destr_func, &currentKeyValue->data, SYS_DESTR_OBJECT);
              FREE_RECORD(currentKeyValue, hashElemRecord, count.helem);
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
      logFunction(printf("hsh_gen_hash -->\n"););
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        free_hash(aHashMap, key_destr_func, data_destr_func);
        return raise_with_obj_and_args(SYS_RNG_EXCEPTION,
                                       hash_exec_object, arguments);
      } else {
        return bld_hash_temp(aHashMap);
      } /* if */
    } /* if */
  } /* hsh_gen_hash */



objectType hsh_gen_key_value (listType arguments)

  {
    objectType aKey;
    objectType aValue;
    objectType hash_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;
    hashElemType keyValue;

  /* hsh_gen_key_value */
    aKey   = arg_2(arguments);
    aValue = arg_4(arguments);
    hash_exec_object = curr_exec_object;
    logFunction(printf("hsh_gen_key_value(" FMT_X_MEM ", " FMT_X_MEM ")\n",
                       (memSizeType) aKey, (memSizeType) aValue););
    if (unlikely(!ALLOC_RECORD(keyValue, hashElemRecord, count.helem))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      keyValue->next_less = NULL;
      keyValue->next_greater = NULL;
      if (TEMP_OBJECT(aKey)) {
        CLEAR_TEMP_FLAG(aKey);
        SET_VAR_FLAG(aKey);
        memcpy(&keyValue->key, aKey, sizeof(objectRecord));
        FREE_OBJECT(aKey);
        arg_2(arguments) = NULL;
      } else {
        if (unlikely(!arr_elem_initialisation(aKey->type_of,
                                              &keyValue->key, aKey))) {
          FREE_RECORD(keyValue, hashElemRecord, count.helem);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         hash_exec_object, arguments);
        } /* if */
      } /* if */
      if (TEMP_OBJECT(aValue)) {
        CLEAR_TEMP_FLAG(aValue);
        SET_VAR_FLAG(aValue);
        memcpy(&keyValue->data, aValue, sizeof(objectRecord));
        FREE_OBJECT(aValue);
        arg_4(arguments) = NULL;
      } else {
        if (unlikely(!arr_elem_initialisation(aValue->type_of,
                                              &keyValue->data, aValue))) {
          do_destroy(&keyValue->key, &err_info);
          FREE_RECORD(keyValue, hashElemRecord, count.helem);
          return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         hash_exec_object, arguments);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("hsh_gen_key_value --> " FMT_X_MEM "\n",
                       (memSizeType) keyValue););
    return bld_hashelem_temp(keyValue);
  } /* hsh_gen_key_value */



/**
 *  Access one value from the hash table 'aHashMap/arg_1'.
 *  @return the element with the key 'aKey/arg_2' from 'aHashMap/arg_1'.
 *  @exception INDEX_ERROR If 'aHashMap/arg_1' does not have an element
 *             with the key 'aKey/arg_2'.
 */
objectType hsh_idx (listType arguments)

  {
    hashType aHashMap;
    objectType aKey;
    intType hashcode;
    objectType cmp_func;
    hashElemType hashelem;
    hashElemType result_hashelem;
    objectType cmp_obj;
    intType cmp;
    objectType hash_exec_object;
    objectType result;

  /* hsh_idx */
    isit_hash(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_reference(arg_4(arguments));
    aHashMap =      take_hash(arg_1(arguments));
    aKey     =                arg_2(arguments);
    hashcode =       take_int(arg_3(arguments));
    cmp_func = take_reference(arg_4(arguments));
    hash_exec_object = curr_exec_object;
    isit_not_null(cmp_func);
    logFunction(printf("hsh_idx(" FMT_X_MEM ", " FMT_X_MEM ", " FMT_U ", " FMT_X_MEM ")\n",
                       (memSizeType) aHashMap, (memSizeType) aKey, hashcode,
                       (memSizeType) cmp_func););
    result_hashelem = NULL;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
      isit_not_null(cmp_obj);
      isit_int(cmp_obj);
      cmp = take_int(cmp_obj);
      FREE_OBJECT(cmp_obj);
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (cmp == 0) {
        result_hashelem = hashelem;
        hashelem = NULL;
      } else {
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
    if (unlikely(result_hashelem == NULL)) {
      logError(printf("hsh_idx(" FMT_X_MEM ", " FMT_X_MEM ", " FMT_U "): "
                      "Hashmap does not have an element with the key.\n",
                      (memSizeType) aHashMap, (memSizeType) aKey, hashcode););
      result = raise_with_obj_and_args(SYS_IDX_EXCEPTION,
                                       hash_exec_object, arguments);
    } else {
      if (TEMP2_OBJECT(arg_1(arguments))) {
        /* The hash will be destroyed after indexing. */
        /* Therefore it is necessary here to remove it */
        /* from the hashtable to avoid a crash !!!!! */
        if (unlikely(!ALLOC_OBJECT(result))) {
          result = raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                           hash_exec_object, arguments);
        } else {
          memcpy(result, &result_hashelem->data, sizeof(objectRecord));
          SET_TEMP_FLAG(result);
          /* Overwrite the data element in the hash with a FORWARDOBJECT value. */
          /* The function free_helem uses FORWARDOBJECT as magic value */
          /* and does not call a destructor for it. */
          SET_CATEGORY_OF_OBJ(&result_hashelem->data, FORWARDOBJECT);
          result_hashelem->data.value.intValue = 1234567890;
        } /* if */
      } else {
        result = &result_hashelem->data;
      } /* if */
    } /* if */
    logFunction(printf("hsh_idx(" FMT_X_MEM ", " FMT_X_MEM ", " FMT_U ", " FMT_X_MEM ") --> \n",
                       (memSizeType) aHashMap, (memSizeType) aKey, hashcode,
                       (memSizeType) cmp_func);
                trace1(result);
                printf("\n"););
    return result;
  } /* hsh_idx */



/**
 *  Access one value from the hash table 'aHashMap/arg_1'.
 *  @return the element with the key 'aKey/arg_2' from 'aHashMap/arg_1' or
 *          'defaultValue/arg_4' if 'aHashMap/arg_1' does not have an element
 *          with the key 'aKey/arg_2'.
 */
objectType hsh_idx2 (listType arguments)

  {
    hashType aHashMap;
    objectType aKey;
    intType hashcode;
    objectType defaultValue;
    objectType cmp_func;
    objectType data_create_func;
    hashElemType hashelem;
    hashElemType result_hashelem;
    objectType cmp_obj;
    intType cmp;
    objectType hash_exec_object;
    objectType result;

  /* hsh_idx2 */
    isit_hash(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_reference(arg_5(arguments));
    isit_reference(arg_6(arguments));
    aHashMap         =      take_hash(arg_1(arguments));
    aKey             =                arg_2(arguments);
    hashcode         =       take_int(arg_3(arguments));
    defaultValue     =                arg_4(arguments);
    cmp_func         = take_reference(arg_5(arguments));
    data_create_func = take_reference(arg_6(arguments));
    hash_exec_object = curr_exec_object;
    isit_not_null(cmp_func);
    isit_not_null(data_create_func);
    logFunction(printf("hsh_idx2(" FMT_X_MEM ", " FMT_X_MEM ", " FMT_U ", " FMT_X_MEM ")\n",
                       (memSizeType) aHashMap, (memSizeType) aKey, hashcode,
                       (memSizeType) cmp_func););
    result_hashelem = NULL;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
      isit_not_null(cmp_obj);
      isit_int(cmp_obj);
      cmp = take_int(cmp_obj);
      FREE_OBJECT(cmp_obj);
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
      if (TEMP2_OBJECT(arg_1(arguments))) {
        /* The hash will be destroyed after indexing. */
        /* Therefore it is necessary here to remove it */
        /* from the hashtable to avoid a crash !!!!! */
        if (unlikely(!ALLOC_OBJECT(result))) {
          result = raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                           hash_exec_object, arguments);
        } else {
          memcpy(result, &result_hashelem->data, sizeof(objectRecord));
          SET_TEMP_FLAG(result);
          /* Overwrite the data element in the hash with a FORWARDOBJECT value. */
          /* The function free_helem uses FORWARDOBJECT as magic value */
          /* and does not call a destructor for it. */
          SET_CATEGORY_OF_OBJ(&result_hashelem->data, FORWARDOBJECT);
          result_hashelem->data.value.intValue = 1234567890;
        } /* if */
      } else {
        result = &result_hashelem->data;
      } /* if */
    } else {
      if (unlikely(!ALLOC_OBJECT(result))) {
        result = raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                         hash_exec_object, arguments);
      } else {
        result->type_of = defaultValue->type_of;
        result->descriptor.property = NULL;
        if (TEMP2_OBJECT(arg_3(arguments))) {
          INIT_CATEGORY_OF_TEMP(result, CATEGORY_OF_OBJ(defaultValue));
          result->value = defaultValue->value;
          memset(&arg_3(arguments)->value, 0, sizeof(valueUnion));
        } else {
          INIT_CATEGORY_OF_OBJ(result, DECLAREDOBJECT);
          param3_call(data_create_func, result, SYS_CREA_OBJECT, defaultValue);
          SET_TEMP_FLAG(result);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("hsh_idx2(" FMT_X_MEM ", " FMT_X_MEM ", "
                       FMT_U ", " FMT_X_MEM ") --> \n",
                       (memSizeType) aHashMap, (memSizeType) aKey, hashcode,
                       (memSizeType) cmp_func);
                trace1(result);
                printf("\n"););
    return result;
  } /* hsh_idx2 */



/**
 *  Add 'data/arg_3' with the key 'aKey/arg_2' to the hash map 'aHashMap/arg_1'.
 *  If an element with the key 'aKey/arg_2' already exists,
 *  it is overwritten with 'data/arg_3'.
 *  @exception MEMORY_ERROR If there is not enough memory.
 */
objectType hsh_incl (listType arguments)

  {
    hashType aHashMap;
    intType hashcode;
    objectType aKey;
    objectType data;
    objectType cmp_func;
    objectType key_create_func;
    objectType data_create_func;
    objectType data_copy_func;
    hashElemType hashelem;
    objectType cmp_obj;
    intType cmp;
    objectType hash_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;

  /* hsh_incl */
    isit_hash(arg_1(arguments));
    isit_int(arg_4(arguments));
    isit_reference(arg_5(arguments));
    aHashMap         =      take_hash(arg_1(arguments));
    aKey             =                arg_2(arguments);
    data             =                arg_3(arguments);
    hashcode         =       take_int(arg_4(arguments));
    cmp_func         = take_reference(arg_5(arguments));
    key_create_func  = take_reference(arg_6(arguments));
    data_create_func = take_reference(arg_7(arguments));
    data_copy_func   = take_reference(arg_8(arguments));
    hash_exec_object = curr_exec_object;
    isit_not_null(cmp_func);
    isit_not_null(key_create_func);
    isit_not_null(data_create_func);
    isit_not_null(data_copy_func);
    logFunction(printf("hsh_incl(" FMT_X_MEM ", " FMT_X_MEM ", " FMT_X_MEM ", "
                       FMT_U ", " FMT_X_MEM ", " FMT_X_MEM ", " FMT_X_MEM ", "
                       FMT_X_MEM ")\n",
                       (memSizeType) aHashMap, (memSizeType) aKey, (memSizeType) data,
                       hashcode, (memSizeType) cmp_func, (memSizeType) key_create_func,
                       (memSizeType) data_create_func, (memSizeType) data_copy_func););
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    if (hashelem == NULL) {
      aHashMap->table[(unsigned int) hashcode & aHashMap->mask] = new_helem(aKey, data,
          key_create_func, data_create_func, &err_info);
      aHashMap->size++;
    } else if (unlikely(cmp_func == NULL)) {
      logError(printf("hsh_incl: cmp_func == NULL\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      do {
        cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
        isit_not_null(cmp_obj);
        isit_int(cmp_obj);
        cmp = take_int(cmp_obj);
        FREE_OBJECT(cmp_obj);
        if (cmp < 0) {
          if (hashelem->next_less == NULL) {
            hashelem->next_less = new_helem(aKey, data,
                key_create_func, data_create_func, &err_info);
            aHashMap->size++;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_less;
          } /* if */
        } else if (cmp == 0) {
          param3_call(data_copy_func, &hashelem->data, SYS_ASSIGN_OBJECT, data);
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
      return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                     hash_exec_object, arguments);
    } else {
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* hsh_incl */



/**
 *  Obtain the keys of the hash map 'aHashMap/arg_1'.
 *  @return the keys of the hash map.
 */
objectType hsh_keys (listType arguments)

  {
    hashType aHashMap;
    objectType key_create_func;
    objectType key_destr_func;
    arrayType key_array;

  /* hsh_keys */
    isit_hash(arg_1(arguments));
    aHashMap = take_hash(arg_1(arguments));
    key_create_func = take_reference(arg_2(arguments));
    key_destr_func = take_reference(arg_3(arguments));
    isit_not_null(key_create_func);
    isit_not_null(key_destr_func);
    key_array = keys_hash(aHashMap, key_create_func, key_destr_func);
    return bld_array_temp(key_array);
  } /* hsh_keys */



/**
 *  Number of elements in the hash map 'aHashMap/arg_1'.
 *  @return the number of elements in 'aHashMap/arg_1'.
 */
objectType hsh_lng (listType arguments)

  { /* hsh_lng */
    isit_hash(arg_1(arguments));
    return bld_int_temp(
        (intType) take_hash(arg_1(arguments))->size);
  } /* hsh_lng */



/**
 *  Compute pseudo-random key from 'aHashMap/arg_1'.
 *  The random values are uniform distributed.
 *  @return a random key such that hsh_rand_key(aHashMap) in aHashMap holds.
 *  @exception RANGE_ERROR If 'aHashMap/arg_1' is empty.
 */
objectType hsh_rand_key (listType arguments)

  {
    const_hashType aHashMap;
    memSizeType num_elements;
    memSizeType elem_index;
    hashElemType hash_elem;
    objectType result;

  /* hsh_rand_key */
    isit_hash(arg_1(arguments));
    aHashMap = take_hash(arg_1(arguments));
    logFunction(printf("hsh_rand_key(" FMT_X_MEM ")\n",
                       (memSizeType) aHashMap););
    num_elements = aHashMap->size;
    /* printf("num_elements " FMT_U_MEM "\n", num_elements); */
    if (unlikely(num_elements == 0)) {
      logError(printf("hsh_rand_key(): Hash map is empty.\n"););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      elem_index = (memSizeType) (uintType)
          intRand((intType) 1, (intType) num_elements);
      /* printf("elem_index " FMT_U_MEM "\n", elem_index); */
      hash_elem = get_hash_elem(aHashMap, elem_index);
      /* printf(FMT_U_MEM "\n", (memSizeType) hash_elem); */
      if (TEMP_OBJECT(arg_1(arguments))) {
        /* The hash will be destroyed after indexing. */
        /* Therefore it is necessary here to remove it */
        /* from the hashtable to avoid a crash !!!!! */
        if (unlikely(!ALLOC_OBJECT(result))) {
          result = raise_exception(SYS_MEM_EXCEPTION);
        } else {
          memcpy(result, &hash_elem->key, sizeof(objectRecord));
          SET_TEMP_FLAG(result);
          /* Overwrite the key element in the hash with a FORWARDOBJECT value. */
          /* The function free_helem uses FORWARDOBJECT as magic value */
          /* and does not call a destructor for it. */
          SET_CATEGORY_OF_OBJ(&hash_elem->key, FORWARDOBJECT);
          hash_elem->key.value.intValue = 1234567890;
        } /* if */
      } else {
        result = &hash_elem->key;
      } /* if */
    } /* if */
    logFunction(printf("hsh_rand_key --> " FMT_X_MEM "\n",
                       (memSizeType) result););
    return result;
  } /* hsh_rand_key */



objectType hsh_refidx (listType arguments)

  {
    hashType aHashMap;
    intType hashcode;
    objectType aKey;
    objectType cmp_func;
    hashElemType hashelem;
    hashElemType result_hashelem;
    objectType cmp_obj;
    intType cmp;
    objectType hash_exec_object;
    objectType result;

  /* hsh_refidx */
    isit_hash(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_reference(arg_4(arguments));
    aHashMap =      take_hash(arg_1(arguments));
    aKey     =                arg_2(arguments);
    hashcode =       take_int(arg_3(arguments));
    cmp_func = take_reference(arg_4(arguments));
    hash_exec_object = curr_exec_object;
    isit_not_null(cmp_func);
    result_hashelem = NULL;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
      isit_not_null(cmp_obj);
      isit_int(cmp_obj);
      cmp = take_int(cmp_obj);
      FREE_OBJECT(cmp_obj);
      if (cmp < 0) {
        hashelem = hashelem->next_less;
      } else if (cmp == 0) {
        result_hashelem = hashelem;
        hashelem = NULL;
      } else {
        hashelem = hashelem->next_greater;
      } /* if */
    } /* while */
    if (unlikely(result_hashelem != NULL)) {
      result = &result_hashelem->data;
      if (unlikely(TEMP_OBJECT(arg_1(arguments)))) {
        /* The hash will be destroyed after indexing. */
        /* Therefore it is necessary here to remove it */
        /* from the hashtable to avoid a crash !!!!! */
        return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                       hash_exec_object, arguments);
      } /* if */
    } else {
      result = NULL;
    } /* if */
    return bld_reference_temp(result);
  } /* hsh_refidx */



/**
 *  Add 'data/arg_3' with the key 'aKey/arg_2' to the hash map 'aHashMap/arg_1'.
 *  If an element with the key 'aKey/arg_2' already exists,
 *  it is overwritten with 'data/arg_3'.
 *  @param aHashMap/arg_1 Hash map to be updated.
 *  @param aKey/arg_2 Key of the value to be updated.
 *  @param data/arg_3 Data value to be added to the hash map.
 *  @return the old element with the key 'aKey/arg_2' or
 *          the new data value if no old element existed.
 *  @exception MEMORY_ERROR If there is not enough memory.
 */
objectType hsh_update (listType arguments)

  {
    hashType aHashMap;
    intType hashcode;
    objectType aKey;
    objectType data;
    objectType cmp_func;
    objectType key_create_func;
    objectType data_create_func;
    hashElemType hashelem;
    objectType cmp_obj;
    intType cmp;
    objectType hash_exec_object;
    errInfoType err_info = OKAY_NO_ERROR;
    valueUnion value;

  /* hsh_update */
    /* printf("hsh_update\n"); */
    isit_hash(arg_1(arguments));
    isit_int(arg_4(arguments));
    isit_reference(arg_5(arguments));
    aHashMap         =      take_hash(arg_1(arguments));
    aKey             =                arg_2(arguments);
    data             =                arg_3(arguments);
    hashcode         =       take_int(arg_4(arguments));
    cmp_func         = take_reference(arg_5(arguments));
    key_create_func  = take_reference(arg_6(arguments));
    data_create_func = take_reference(arg_7(arguments));
    hash_exec_object = curr_exec_object;
    isit_not_null(cmp_func);
    isit_not_null(key_create_func);
    isit_not_null(data_create_func);
    logFunction(printf("hsh_update(" FMT_X_MEM ", " FMT_X_MEM ", " FMT_X_MEM ", "
                       FMT_U ", " FMT_X_MEM ", " FMT_X_MEM ", " FMT_X_MEM ")\n",
                       (memSizeType) aHashMap, (memSizeType) aKey, (memSizeType) data,
                       hashcode, (memSizeType) cmp_func, (memSizeType) key_create_func,
                       (memSizeType) data_create_func););
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    if (hashelem == NULL) {
      aHashMap->table[(unsigned int) hashcode & aHashMap->mask] = new_helem(aKey, data,
          key_create_func, data_create_func, &err_info);
      aHashMap->size++;
    } else if (unlikely(cmp_func == NULL)) {
      logError(printf("hsh_update: cmp_func == NULL\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      do {
        cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
        isit_not_null(cmp_obj);
        isit_int(cmp_obj);
        cmp = take_int(cmp_obj);
        FREE_OBJECT(cmp_obj);
        if (cmp < 0) {
          if (hashelem->next_less == NULL) {
            hashelem->next_less = new_helem(aKey, data,
                key_create_func, data_create_func, &err_info);
            aHashMap->size++;
            hashelem = NULL;
          } else {
            hashelem = hashelem->next_less;
          } /* if */
        } else if (cmp == 0) {
          value = hashelem->data.value;
          hashelem->data.value = data->value;
          data->value = value;
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
      return raise_with_obj_and_args(SYS_MEM_EXCEPTION,
                                     hash_exec_object, arguments);
    } /* if */
    /* printf("hsh_update -> ");
    trace1(data);
    printf("\n"); */
    return data;
  } /* hsh_update */



/**
 *  Obtain the values of the hash map 'aHashMap/arg_1'.
 *  @return the values of the hash map.
 */
objectType hsh_values (listType arguments)

  {
    hashType aHashMap;
    objectType value_create_func;
    objectType value_destr_func;
    arrayType value_array;

  /* hsh_values */
    isit_hash(arg_1(arguments));
    aHashMap = take_hash(arg_1(arguments));
    value_create_func = take_reference(arg_2(arguments));
    value_destr_func = take_reference(arg_3(arguments));
    isit_not_null(value_create_func);
    isit_not_null(value_destr_func);
    value_array = values_hash(aHashMap, value_create_func, value_destr_func);
    return bld_array_temp(value_array);
  } /* hsh_values */
