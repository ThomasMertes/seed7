/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  Changes: 2005, 2013  Thomas Mertes                              */
/*  Content: All primitive actions for hash types.                  */
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
#include "listutl.h"
#include "traceutl.h"
#include "executl.h"
#include "objutl.h"
#include "exec.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "hshlib.h"


#define TABLE_BITS 10
#define TABLE_SIZE(bits) ((unsigned int) 1 << (bits))
#define TABLE_MASK(bits) (TABLE_SIZE(bits)-1)
#define ARRAY_SIZE_INCREMENT 512



static void free_helem (hashElemType old_helem, objectType key_destr_func,
    objectType data_destr_func)

  { /* free_helem */
    param2_call(key_destr_func, &old_helem->key, SYS_DESTR_OBJECT);
    if (CATEGORY_OF_OBJ(&old_helem->data) != FORWARDOBJECT) {
      /* FORWARDOBJECT is used as magic category in hsh_idx */
      param2_call(data_destr_func, &old_helem->data, SYS_DESTR_OBJECT);
    } /* if */
    if (old_helem->next_less != NULL) {
      free_helem(old_helem->next_less, key_destr_func,
          data_destr_func);
    } /* if */
    if (old_helem->next_greater != NULL) {
      free_helem(old_helem->next_greater, key_destr_func,
          data_destr_func);
    } /* if */
    FREE_RECORD(old_helem, hashElemRecord, count.helem);
  } /* free_helem */



static void free_hash (hashType old_hash, objectType key_destr_func,
    objectType data_destr_func)

  {
    unsigned int number;
    hashElemType *curr_helem;

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
      FREE_HASH(old_hash, old_hash->table_size);
    } /* if */
  } /* free_hash */



static hashElemType new_helem (objectType key, objectType data,
    objectType key_create_func, objectType data_create_func, errInfoType *err_info)

  {
    hashElemType helem;

  /* new_helem */
    if (!ALLOC_RECORD(helem, hashElemRecord, count.helem)) {
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
    if (ALLOC_HASH(hash, TABLE_SIZE(bits))) {
      hash->bits = bits;
      hash->mask = TABLE_MASK(bits);
      hash->table_size = TABLE_SIZE(bits);
      hash->size = 0;
      memset(hash->table, 0, hash->table_size * sizeof(hashElemType));
    } /* if */
    return hash;
  } /* new_hash */



static hashElemType create_helem (hashElemType source_helem,
    objectType key_create_func, objectType data_create_func, errInfoType *err_info)

  {
    hashElemType dest_helem;

  /* create_helem */
    if (!ALLOC_RECORD(dest_helem, hashElemRecord, count.helem)) {
      *err_info = MEMORY_ERROR;
    } else {
      memcpy(&dest_helem->key.descriptor, &source_helem->key.descriptor, sizeof(descriptorUnion));
      INIT_CATEGORY_OF_VAR(&dest_helem->key, DECLAREDOBJECT);
      SET_ANY_FLAG(&dest_helem->key, HAS_POSINFO(&source_helem->key));
      dest_helem->key.type_of = source_helem->key.type_of;
      param3_call(key_create_func, &dest_helem->key, SYS_CREA_OBJECT, &source_helem->key);
      memcpy(&dest_helem->data.descriptor, &source_helem->data.descriptor, sizeof(descriptorUnion));
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
    objectType key_create_func, objectType data_create_func, errInfoType *err_info)

  {
    unsigned int new_size;
    unsigned int number;
    hashElemType *source_helem;
    hashElemType *dest_helem;
    hashType dest_hash;

  /* create_hash */
    new_size = source_hash->table_size;
    if (!ALLOC_HASH(dest_hash, new_size)) {
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
        if (*source_helem != NULL) {
          *dest_helem = create_helem(*source_helem, key_create_func, data_create_func, err_info);
        } else {
          *dest_helem = NULL;
        } /* if */
        number--;
        source_helem++;
        dest_helem++;
      } /* while */
    } /* if */
    return dest_hash;
  } /* create_hash */



static void keys_helem (arrayType *key_array, memSizeType *arr_pos,
    hashElemType curr_helem, objectType key_create_func, errInfoType *err_info)

  {
    memSizeType array_size;
    arrayType resized_key_array;
    objectType dest_obj;

  /* keys_helem */
    array_size = arraySize(*key_array);
    if (*arr_pos + 1 >= array_size) {
      if (array_size > MAX_ARR_LEN - ARRAY_SIZE_INCREMENT ||
          (*key_array)->max_position > MAX_MEM_INDEX - ARRAY_SIZE_INCREMENT) {
        resized_key_array = NULL;
      } else {
        resized_key_array = REALLOC_ARRAY(*key_array,
            array_size, array_size + ARRAY_SIZE_INCREMENT);
      } /* if */
      if (resized_key_array == NULL) {
        *err_info = MEMORY_ERROR;
        return;
      } else {
        *key_array = resized_key_array;
        COUNT3_ARRAY(array_size, array_size + ARRAY_SIZE_INCREMENT);
        (*key_array)->max_position += ARRAY_SIZE_INCREMENT;
      } /* if */
    } /* if */
    dest_obj = &(*key_array)->arr[*arr_pos];
    memcpy(&dest_obj->descriptor, &curr_helem->key.descriptor, sizeof(descriptorUnion));
    INIT_CATEGORY_OF_VAR(dest_obj, DECLAREDOBJECT);
    SET_ANY_FLAG(dest_obj, HAS_POSINFO(&curr_helem->key));
    dest_obj->type_of = curr_helem->key.type_of;
    param3_call(key_create_func, dest_obj, SYS_CREA_OBJECT, &curr_helem->key);
    (*arr_pos)++;
    if (curr_helem->next_less != NULL) {
      keys_helem(key_array, arr_pos, curr_helem->next_less, key_create_func, err_info);
    } /* if */
    if (curr_helem->next_greater != NULL) {
      keys_helem(key_array, arr_pos, curr_helem->next_greater, key_create_func, err_info);
    } /* if */
  } /* keys_helem */



static arrayType keys_hash (hashType curr_hash, objectType key_create_func,
    objectType key_destr_func, errInfoType *err_info)

  {
    memSizeType arr_pos;
    memSizeType number;
    hashElemType *curr_helem;
    memSizeType array_size;
    arrayType resized_key_array;
    arrayType key_array;

  /* keys_hash */
    if (!ALLOC_ARRAY(key_array, ARRAY_SIZE_INCREMENT)) {
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
        resized_key_array = REALLOC_ARRAY(key_array, array_size, arr_pos);
        if (resized_key_array == NULL) {
          *err_info = MEMORY_ERROR;
        } else {
          key_array = resized_key_array;
          COUNT3_ARRAY(array_size, arr_pos);
          key_array->max_position = (intType) arr_pos;
        } /* if */
      } /* if */
      if (*err_info != OKAY_NO_ERROR) {
        for (number = 0; number < arr_pos; number++) {
          param2_call(key_destr_func, &key_array->arr[number], SYS_DESTR_OBJECT);
        } /* for */
        FREE_ARRAY(key_array, array_size);
        key_array = NULL;
      } /* if */
    } /* if */
    return key_array;
  } /* keys_hash */



static void values_helem (arrayType *value_array, memSizeType *arr_pos,
    hashElemType curr_helem, objectType value_create_func, errInfoType *err_info)

  {
    memSizeType array_size;
    arrayType resized_value_array;
    objectType dest_obj;

  /* values_helem */
    array_size = arraySize(*value_array);
    if (*arr_pos + 1 >= array_size) {
      if (array_size > MAX_ARR_LEN - ARRAY_SIZE_INCREMENT ||
          (*value_array)->max_position > MAX_MEM_INDEX - ARRAY_SIZE_INCREMENT) {
        resized_value_array = NULL;
      } else {
        resized_value_array = REALLOC_ARRAY(*value_array,
            array_size, array_size + ARRAY_SIZE_INCREMENT);
      } /* if */
      if (resized_value_array == NULL) {
        *err_info = MEMORY_ERROR;
        return;
      } else {
        *value_array = resized_value_array;
        COUNT3_ARRAY(array_size, array_size + ARRAY_SIZE_INCREMENT);
        (*value_array)->max_position += ARRAY_SIZE_INCREMENT;
      } /* if */
    } /* if */
    dest_obj = &(*value_array)->arr[*arr_pos];
    memcpy(&dest_obj->descriptor, &curr_helem->data.descriptor, sizeof(descriptorUnion));
    INIT_CATEGORY_OF_VAR(dest_obj, DECLAREDOBJECT);
    SET_ANY_FLAG(dest_obj, HAS_POSINFO(&curr_helem->data));
    dest_obj->type_of = curr_helem->data.type_of;
    param3_call(value_create_func, dest_obj, SYS_CREA_OBJECT, &curr_helem->data);
    (*arr_pos)++;
    if (curr_helem->next_less != NULL) {
      values_helem(value_array, arr_pos, curr_helem->next_less, value_create_func, err_info);
    } /* if */
    if (curr_helem->next_greater != NULL) {
      values_helem(value_array, arr_pos, curr_helem->next_greater, value_create_func, err_info);
    } /* if */
  } /* values_helem */



static arrayType values_hash (hashType curr_hash, objectType value_create_func,
    objectType value_destr_func, errInfoType *err_info)

  {
    memSizeType arr_pos;
    memSizeType number;
    hashElemType *curr_helem;
    memSizeType array_size;
    arrayType resized_value_array;
    arrayType value_array;

  /* values_hash */
    if (!ALLOC_ARRAY(value_array, ARRAY_SIZE_INCREMENT)) {
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
        resized_value_array = REALLOC_ARRAY(value_array, array_size, arr_pos);
        if (resized_value_array == NULL) {
          *err_info = MEMORY_ERROR;
        } else {
          value_array = resized_value_array;
          COUNT3_ARRAY(array_size, arr_pos);
          value_array->max_position = (intType) arr_pos;
        } /* if */
      } /* if */
      if (*err_info != OKAY_NO_ERROR) {
        for (number = 0; number < arr_pos; number++) {
          param2_call(value_destr_func, &value_array->arr[number], SYS_DESTR_OBJECT);
        } /* for */
        FREE_ARRAY(value_array, array_size);
        value_array = NULL;
      } /* if */
    } /* if */
    return value_array;
  } /* values_hash */



static void for_helem (objectType for_variable, hashElemType curr_helem,
    objectType statement, objectType data_copy_func)

  { /* for_helem */
    if (curr_helem != NULL) {
      param3_call(data_copy_func, for_variable, SYS_ASSIGN_OBJECT, &curr_helem->data);
      evaluate(statement);
      for_helem(for_variable, curr_helem->next_less, statement, data_copy_func);
      for_helem(for_variable, curr_helem->next_greater, statement, data_copy_func);
    } /* if */
  } /* for_helem */



static void for_hash (objectType for_variable, hashType curr_hash,
    objectType statement, objectType data_copy_func)

  {
    unsigned int number;
    hashElemType *curr_helem;

  /* for_hash */
    number = curr_hash->table_size;
    curr_helem = &curr_hash->table[0];
    while (number > 0) {
      for_helem(for_variable, *curr_helem, statement, data_copy_func);
      number--;
      curr_helem++;
    } /* while */
  } /* for_hash */



static void for_key_helem (objectType key_variable, hashElemType curr_helem,
    objectType statement, objectType key_copy_func)

  { /* for_key_helem */
    if (curr_helem != NULL) {
      param3_call(key_copy_func, key_variable, SYS_ASSIGN_OBJECT, &curr_helem->key);
      evaluate(statement);
      for_key_helem(key_variable, curr_helem->next_less, statement, key_copy_func);
      for_key_helem(key_variable, curr_helem->next_greater, statement, key_copy_func);
    } /* if */
  } /* for_key_helem */



static void for_key_hash (objectType key_variable, hashType curr_hash,
    objectType statement, objectType key_copy_func)

  {
    unsigned int number;
    hashElemType *curr_helem;

  /* for_key_hash */
    number = curr_hash->table_size;
    curr_helem = &curr_hash->table[0];
    while (number > 0) {
      for_key_helem(key_variable, *curr_helem, statement, key_copy_func);
      number--;
      curr_helem++;
    } /* while */
  } /* for_key_hash */



static void for_data_key_helem (objectType for_variable, objectType key_variable,
    hashElemType curr_helem, objectType statement, objectType data_copy_func,
    objectType key_copy_func)

  { /* for_data_key_helem */
    if (curr_helem != NULL) {
      param3_call(data_copy_func, for_variable, SYS_ASSIGN_OBJECT, &curr_helem->data);
      param3_call(key_copy_func, key_variable, SYS_ASSIGN_OBJECT, &curr_helem->key);
      evaluate(statement);
      for_data_key_helem(for_variable, key_variable, curr_helem->next_less, statement,
          data_copy_func, key_copy_func);
      for_data_key_helem(for_variable, key_variable, curr_helem->next_greater, statement,
          data_copy_func, key_copy_func);
    } /* if */
  } /* for_data_key_helem */



static void for_data_key_hash (objectType for_variable, objectType key_variable,
    hashType curr_hash, objectType statement, objectType data_copy_func,
    objectType key_copy_func)

  {
    unsigned int number;
    hashElemType *curr_helem;

  /* for_data_key_hash */
    number = curr_hash->table_size;
    curr_helem = &curr_hash->table[0];
    while (number > 0) {
      for_data_key_helem(for_variable, key_variable, *curr_helem, statement,
          data_copy_func, key_copy_func);
      number--;
      curr_helem++;
    } /* while */
  } /* for_data_key_hash */



/**
 *  Hash membership test.
 *  Determine if 'aKey' is a member of the hash map 'aHashMap'.
 *  @return TRUE when 'aKey' is a member of 'aHashMap',
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
    result = SYS_FALSE_OBJECT;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
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



#ifdef OUT_OF_ORDER
objectType hsh_conv (listType arguments)

  {
    objectType hsh_arg;
    hashType arr1;
    memSizeType result_size;
    hashType result_hash;
    objectType result;

  /* hsh_conv */
    hsh_arg = arg_3(arguments);
    isit_hash(hsh_arg);
    if (TEMP_OBJECT(hsh_arg)) {
      result = hsh_arg;
      result->type_of = NULL;
      arg_3(arguments) = NULL;
    } else {
      arr1 = take_hash(hsh_arg);
      result_size = arr1->max_position - arr1->min_position + 1;
      if (!ALLOC_HASH(result_hash, result_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result_hash->min_position = arr1->min_position;
      result_hash->max_position = arr1->max_position;
      if (!crea_hash(result_hash->arr, arr1->arr, result_size)) {
        FREE_HASH(result_hash, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
      result = bld_hash_temp(result_hash);
    } /* if */
    return result;
  } /* hsh_conv */
#endif



objectType hsh_cpy (listType arguments)

  {
    objectType hsh_to;
    objectType hsh_from;
    hashType hsh_dest;
    hashType hsh_source;
    objectType key_create_func;
    objectType key_destr_func;
    objectType data_create_func;
    objectType data_destr_func;
    errInfoType err_info = OKAY_NO_ERROR;

  /* hsh_cpy */
    hsh_to   = arg_1(arguments);
    hsh_from = arg_2(arguments);
    isit_hash(hsh_to);
    isit_hash(hsh_from);
    is_variable(hsh_to);
    hsh_dest = take_hash(hsh_to);
    hsh_source = take_hash(hsh_from);
    key_create_func  = take_reference(arg_3(arguments));
    key_destr_func   = take_reference(arg_4(arguments));
    data_create_func = take_reference(arg_5(arguments));
    data_destr_func  = take_reference(arg_6(arguments));
    free_hash(hsh_dest, key_destr_func, data_destr_func);
    if (TEMP2_OBJECT(hsh_from)) {
      hsh_to->value.hashValue = hsh_source;
      hsh_from->value.hashValue = NULL;
    } else {
      hsh_to->value.hashValue = create_hash(hsh_source,
          key_create_func, data_create_func, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        free_hash(hsh_to->value.hashValue, key_destr_func,
            data_destr_func);
        hsh_to->value.hashValue = NULL;
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* hsh_cpy */



objectType hsh_create (listType arguments)

  {
    objectType hsh_to;
    objectType hsh_from;
    hashType hsh_source;
    objectType key_create_func;
    objectType key_destr_func;
    objectType data_create_func;
    objectType data_destr_func;
    errInfoType err_info = OKAY_NO_ERROR;

  /* hsh_create */
    hsh_to   = arg_1(arguments);
    hsh_from = arg_2(arguments);
    isit_hash(hsh_from);
    hsh_source = take_hash(hsh_from);
    key_create_func  = take_reference(arg_3(arguments));
    key_destr_func   = take_reference(arg_4(arguments));
    data_create_func = take_reference(arg_5(arguments));
    data_destr_func  = take_reference(arg_6(arguments));
    /* printf("hsh_create(%lX, %lX, %lX, %lX, %lX, %lX)\n",
        hsh_to, hsh_from, key_create_func, key_destr_func,
        data_create_func, data_destr_func); */
    SET_CATEGORY_OF_OBJ(hsh_to, HASHOBJECT);
    if (TEMP2_OBJECT(hsh_from)) {
      hsh_to->value.hashValue = hsh_source;
      hsh_from->value.hashValue = NULL;
    } else {
      hsh_to->value.hashValue = create_hash(hsh_source,
          key_create_func, data_create_func, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        free_hash(hsh_to->value.hashValue, key_destr_func,
            data_destr_func);
        hsh_to->value.hashValue = NULL;
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* hsh_create */



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
    free_hash(old_hash, key_destr_func, data_destr_func);
    arg_1(arguments)->value.hashValue = NULL;
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* hsh_destr */



objectType hsh_empty (listType arguments)

  {
    hashType result;

  /* hsh_empty */
    result = new_hash(TABLE_BITS);
    if (result == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_hash_temp(result);
    } /* if */
  } /* hsh_empty */



/**
 *  Remove the element with the key 'aKey' from the hash map 'aHashMap'.
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
    delete_pos = &aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
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
 *  For-loop where 'forVar' loops over the values of the hash map 'aHashMap'.
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
    for_hash(for_variable, aHashMap, statement, data_copy_func);
    return SYS_EMPTY_OBJECT;
  } /* hsh_for */



/**
 *  For-loop where 'forVar' and 'keyVar' loop over the hash map 'aHashMap'.
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
    for_data_key_hash(for_variable, key_variable, aHashMap, statement,
        data_copy_func, key_copy_func);
    return SYS_EMPTY_OBJECT;
  } /* hsh_for_data_key */



/**
 *  For-loop where 'keyVar' loops over the keys of the hash map 'aHashMap'.
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
    for_key_hash(key_variable, aHashMap, statement, key_copy_func);
    return SYS_EMPTY_OBJECT;
  } /* hsh_for_key */



/**
 *  Access one value from the hash table 'aHashMap'.
 *  @return the element with the key 'aKey' from 'aHashMap'.
 *  @exception RANGE_ERROR When 'aHashMap' does not have an element
 *             with the key 'aKey'.
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
    objectType result;

  /* hsh_idx */
    isit_hash(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_reference(arg_4(arguments));
    aHashMap =      take_hash(arg_1(arguments));
    aKey     =                arg_2(arguments);
    hashcode =       take_int(arg_3(arguments));
    cmp_func = take_reference(arg_4(arguments));
    /* printf("hsh_idx(%lX, %lX, %lu, %lX)\n", aHashMap, aKey, hashcode, cmp_func); */
    result_hashelem = NULL;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
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
        if (!ALLOC_OBJECT(result)) {
          result = raise_exception(SYS_MEM_EXCEPTION);
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
      result = raise_with_arguments(SYS_RNG_EXCEPTION, arguments);
    } /* if */
    /* printf("hsh_idx(%lX, %lX, %lu, %lX) => ", aHashMap, aKey, hashcode, cmp_func);
       trace1(result);
       printf("\n"); */
    return result;
  } /* hsh_idx */



/**
 *  Access one value from the hash table 'aHashMap'.
 *  @return the element with the key 'aKey' from 'aHashMap'.
 *  @exception RANGE_ERROR When 'aHashMap' does not have an element
 *             with the key 'aKey'.
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
    /* printf("hsh_idx(%lX, %lX, %lu, %lX)\n", aHashMap, aKey, hashcode, cmp_func); */
    result_hashelem = NULL;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
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
        if (!ALLOC_OBJECT(result)) {
          result = raise_exception(SYS_MEM_EXCEPTION);
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
      if (!ALLOC_OBJECT(result)) {
        result = raise_exception(SYS_MEM_EXCEPTION);
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
    /* printf("hsh_idx(%lX, %lX, %lu, %lX) => ", aHashMap, aKey, hashcode, cmp_func);
       trace1(result);
       printf("\n"); */
    return result;
  } /* hsh_idx2 */



#ifdef OUT_OF_ORDER
objectType hsh_idx2 (listType arguments)

  {
    hashType aHashMap;
    intType hashcode;
    objectType aKey;
    objectType data;
    objectType cmp_func;
    objectType key_create_func;
    objectType data_create_func;
    hashElemType hashelem;
    hashElemType result_hashelem;
    objectType cmp_obj;
    intType cmp;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* hsh_idx2 */
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
    result_hashelem = NULL;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
      isit_int(cmp_obj);
      cmp = take_int(cmp_obj);
      FREE_OBJECT(cmp_obj);
      if (cmp < 0) {
        if (hashelem->next_less == NULL) {
          result_hashelem = new_helem(aKey, data,
              key_create_func, data_create_func, &err_info);
          aHashMap->size++;
          hashelem->next_less = result_hashelem;
          hashelem = NULL;
        } else {
          hashelem = hashelem->next_less;
        } /* if */
      } else if (cmp == 0) {
        result_hashelem = hashelem;
        hashelem = NULL;
      } else {
        if (hashelem->next_greater == NULL) {
          result_hashelem = new_helem(aKey, data,
              key_create_func, data_create_func, &err_info);
          aHashMap->size++;
          hashelem->next_greater = result_hashelem;
          hashelem = NULL;
        } else {
          hashelem = hashelem->next_greater;
        } /* if */
      } /* if */
    } /* while */
    if (err_info != OKAY_NO_ERROR) {
      aHashMap->size--;
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      result = &result_hashelem->data;
      if (TEMP_OBJECT(arg_1(arguments))) {
        /* The hash will be destroyed after indexing. */
        /* Therefore it is necessary here to remove it */
        /* from the hashtable to avoid a crash !!!!! */
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    return result;
  } /* hsh_idx2 */
#endif



/**
 *  Add 'anElem' with the key 'aKey' to the hash map 'aHashMap'.
 *  When an element with the key 'aKey' already exists,
 *  it is overwritten with 'anElem'.
 *  @exception MEMORY_ERROR When there is not enough memory.
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
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    if (hashelem == NULL) {
      aHashMap->table[(unsigned int) hashcode & aHashMap->mask] = new_helem(aKey, data,
          key_create_func, data_create_func, &err_info);
      aHashMap->size++;
    } else {
      do {
        cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
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
    if (err_info != OKAY_NO_ERROR) {
      aHashMap->size--;
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } else {
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* hsh_incl */



/**
 *  Obtain the keys of the hash map 'aHashMap'.
 *  @return the keys of the hash map.
 */
objectType hsh_keys (listType arguments)

  {
    hashType aHashMap;
    objectType key_create_func;
    objectType key_destr_func;
    arrayType key_array;
    errInfoType err_info = OKAY_NO_ERROR;

  /* hsh_keys */
    isit_hash(arg_1(arguments));
    aHashMap = take_hash(arg_1(arguments));
    key_create_func = take_reference(arg_2(arguments));
    key_destr_func = take_reference(arg_3(arguments));
    key_array = keys_hash(aHashMap, key_create_func, key_destr_func,
        &err_info);
    return bld_array_temp(key_array);
  } /* hsh_keys */



/**
 *  Number of elements in the hash map 'aHashMap'.
 *  @return the number of elements in 'aHashMap'.
 */
objectType hsh_lng (listType arguments)

  { /* hsh_lng */
    isit_hash(arg_1(arguments));
    return bld_int_temp(
        (intType) take_hash(arg_1(arguments))->size);
  } /* hsh_lng */



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
    objectType result;

  /* hsh_refidx */
    isit_hash(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_reference(arg_4(arguments));
    aHashMap =      take_hash(arg_1(arguments));
    aKey     =                arg_2(arguments);
    hashcode =       take_int(arg_3(arguments));
    cmp_func = take_reference(arg_4(arguments));
    result_hashelem = NULL;
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    while (hashelem != NULL) {
      cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
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
      result = &result_hashelem->data;
      if (TEMP_OBJECT(arg_1(arguments))) {
        /* The hash will be destroyed after indexing. */
        /* Therefore it is necessary here to remove it */
        /* from the hashtable to avoid a crash !!!!! */
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } else {
      result = NULL;
    } /* if */
    return bld_reference_temp(result);
  } /* hsh_refidx */



/**
 *  Add 'anElem' with the key 'aKey' to the hash map 'aHashMap'.
 *  When an element with the key 'aKey' already exists,
 *  it is overwritten with 'anElem'.
 *  @exception MEMORY_ERROR When there is not enough memory.
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
    hashelem = aHashMap->table[(unsigned int) hashcode & aHashMap->mask];
    if (hashelem == NULL) {
      aHashMap->table[(unsigned int) hashcode & aHashMap->mask] = new_helem(aKey, data,
          key_create_func, data_create_func, &err_info);
      aHashMap->size++;
    } else {
      do {
        cmp_obj = param3_call(cmp_func, &hashelem->key, aKey, cmp_func);
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
    if (err_info != OKAY_NO_ERROR) {
      aHashMap->size--;
      return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
    } /* if */
    /* printf("hsh_update -> ");
    trace1(data);
    printf("\n"); */
    return data;
  } /* hsh_update */



/**
 *  Obtain the values of the hash map 'aHashMap'.
 *  @return the values of the hash map.
 */
objectType hsh_values (listType arguments)

  {
    hashType aHashMap;
    objectType value_create_func;
    objectType value_destr_func;
    arrayType value_array;
    errInfoType err_info = OKAY_NO_ERROR;

  /* hsh_values */
    isit_hash(arg_1(arguments));
    aHashMap = take_hash(arg_1(arguments));
    value_create_func = take_reference(arg_2(arguments));
    value_destr_func = take_reference(arg_3(arguments));
    value_array = values_hash(aHashMap, value_create_func, value_destr_func,
        &err_info);
    return bld_array_temp(value_array);
  } /* hsh_values */
