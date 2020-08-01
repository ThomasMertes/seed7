/********************************************************************/
/*                                                                  */
/*  hsh_rtl.c     Primitive actions for the string type.            */
/*  Copyright (C) 1989 - 2006  Thomas Mertes                        */
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
/*  Changes: 2005, 2006  Thomas Mertes                              */
/*  Content: Primitive actions for the hash type.                   */
/*                                                                  */
/*  The functions from this file should only be used in compiled    */
/*  Seed7 programs. The interpreter should not use functions of     */
/*  this file.                                                      */
/*                                                                  */
/*  The functions in this file use type declarations from the       */
/*  include file data_rtl.h instead of data.h. Therefore the types  */
/*  helemtype and hashtype are declared different than in the       */
/*  interpreter.                                                    */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"

#include "version.h"
#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "hsh_rtl.h"


#define ARRAY_SIZE_INCREMENT 512



#ifdef ANSI_C

static inttype size_helem (helemtype helem)
#else

static inttype size_helem (helem)
helemtype helem;
#endif

  {
    inttype result;

  /* size_helem */
    result = 1;
    if (helem->next_less != NULL) {
      result += size_helem(helem->next_less);
    } /* if */
    if (helem->next_greater != NULL) {
      result += size_helem(helem->next_greater);
    } /* if */
    return(result);
  } /* size_helem */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

static void keys_helem (arraytype *key_array, memsizetype *arr_pos,
    helemtype curr_helem, void *key_create_func(), errinfotype *err_info)
#else

static void keys_helem (key_array, arr_pos, curr_helem, key_create_func, err_info)
arraytype *key_array;
memsizetype *arr_pos;
helemtype curr_helem;
void *key_create_func();
errinfotype *err_info;
#endif

  {
    memsizetype array_size;
    objecttype dest_obj;

  /* keys_helem */
    array_size = (*key_array)->max_position - (*key_array)->min_position;
    if (*arr_pos >= array_size) {
      if (!RESIZE_ARRAY(*key_array, array_size, array_size + ARRAY_SIZE_INCREMENT)) {
        *err_info = MEMORY_ERROR;
        return;
      } else {
        COUNT3_ARRAY(array_size, array_size + ARRAY_SIZE_INCREMENT);
        (*key_array)->max_position += ARRAY_SIZE_INCREMENT;
      } /* if */
    } /* if */
    dest_obj = &(*key_array)->arr[*arr_pos];
    key_create_func(dest_obj, &curr_helem->key);
    (*arr_pos)++;
    if (curr_helem->next_less != NULL) {
      keys_helem(key_array, arr_pos, curr_helem->next_less, key_create_func, err_info);
    } /* if */
    if (curr_helem->next_greater != NULL) {
      keys_helem(key_array, arr_pos, curr_helem->next_greater, key_create_func, err_info);
    } /* if */
  } /* keys_helem */



#ifdef ANSI_C

static arraytype keys_hash (hashtype curr_hash, objecttype key_create_func,
    objecttype key_destr_func, objecttype key_cmp_func, errinfotype *err_info)
#else

static arraytype keys_hash (curr_hash, key_create_func, key_destr_func,
    key_cmp_func, err_info)
hashtype curr_hash;
objecttype key_create_func;
objecttype key_destr_func;
objecttype key_cmp_func;
errinfotype *err_info;
#endif

  {
    memsizetype arr_pos;
    int number;
    helemtype *curr_helem;
    memsizetype array_size;
    arraytype key_array;

  /* keys_hash */
    if (curr_hash != NULL) {
      if (!ALLOC_ARRAY(key_array, ARRAY_SIZE_INCREMENT)) {
        *err_info = MEMORY_ERROR;
      } else {
        COUNT_ARRAY(ARRAY_SIZE_INCREMENT);
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
            key_destr_func(&key_array->arr[number]);
          } /* for */
          FREE_ARRAY(key_array, array_size);
          key_array = NULL;
        } else {
          if (!RESIZE_ARRAY(key_array, array_size, arr_pos)) {
            FREE_ARRAY(key_array, array_size);
            key_array = NULL;
            *err_info = MEMORY_ERROR;
          } else {
            COUNT3_ARRAY(array_size, arr_pos);
            key_array->max_position = arr_pos;
            qsort_array(key_array->arr, &key_array->arr[arr_pos - 1], key_cmp_func);
          } /* if */
        } /* if */
      } /* if */
    } else {
      if (!ALLOC_ARRAY(key_array, 0)) {
        *err_info = MEMORY_ERROR;
      } else {
        COUNT_ARRAY(0);
        key_array->min_position = 1;
        key_array->max_position = 0;
      } /* if */
    } /* if */
    return(key_array);
  } /* keys_hash */



#ifdef ANSI_C

objecttype hshIdx (hashtype hash1, void *key, inttype hashcode, void *cmp_func)
#else

objecttype hshIdx (hash1, key, hashcode, cmp_func)
hashtype hash1;
void *key;
inttype hashcode;
void *cmp_func;
#endif

  {
    helemtype hashelem;
    helemtype result_hashelem;
    int cmp;
    objecttype result;

  /* hshIdx */
    if (hash1 == NULL) {
      raise_error(RANGE_ERROR);
      return(NULL);
    } else {
      result_hashelem = NULL;
      hashelem = hash1->table[hashcode & hash1->mask];
      while (hashelem != NULL) {
        cmp = cmp_func(&hashelem->key, key);
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
        return(NULL);
      } /* if */
    } /* if */
    return(result);
  } /* hshIdx */



#ifdef ANSI_C

arraytype hshKeys (hashtype hash1, objecttype key_create_func,
    objecttype key_destr_func, objecttype key_cmp_func)
#else

arraytype hshKeys (hash1, key_create_func, key_destr_func, key_cmp_func)
hashtype hash1;
objecttype key_create_func;
objecttype key_destr_func;
objecttype key_cmp_func;
#endif

  {
    arraytype key_array;
    errinfotype err_info = OKAY_NO_ERROR;

  /* hshKeys */
    key_array = keys_hash(hash1, key_create_func, key_destr_func,
        key_cmp_func, &err_info);
    return(key_array);
  } /* hshKeys */
#endif



#ifdef ANSI_C

inttype hshLng (hashtype hash)
#else

inttype hshLng (hash)
hashtype hash;
#endif

  {
    memsizetype number;
    helemtype *curr_helem;
    inttype result;

  /* hshLng */
    result = 0;
    if (hash != NULL) {
      number = hash->table_size;
      curr_helem = &hash->table[0];
      while (number > 0) {
        if (*curr_helem != NULL) {
          result += size_helem(*curr_helem);
        } /* if */
        number--;
        curr_helem++;
      } /* while */
    } /* if */
    return(result);
  } /* hshLng */
