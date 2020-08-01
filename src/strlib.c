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
/*  File: seed7/src/strlib.c                                        */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: All primitive actions for the string type.             */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "chclsutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "objutl.h"
#include "runerr.h"
#include "str_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "strlib.h"



static inline int strelem_memcmp (const strElemType *mem1,
    const strElemType *mem2, size_t number)

  { /* strelem_memcmp */
    for (; number > 0; mem1++, mem2++, number--) {
      if (*mem1 != *mem2) {
        return *mem1 < *mem2 ? -1 : 1;
      } /* if */
    } /* for */
    return 0;
  } /* strelem_memcmp */



static inline const strElemType *search_strelem (const strElemType *mem,
    const strElemType ch, size_t number)

  { /* search_strelem */
    for (; number > 0; mem++, number--) {
      if (*mem == ch) {
        return mem;
      } /* if */
    } /* for */
    return NULL;
  } /* search_strelem */



static arrayType add_stri_to_array (const strElemType *stri_elems, memSizeType length,
    arrayType work_array, intType *used_max_position)

  {
    striType new_stri;
    arrayType resized_work_array;
    memSizeType position;

  /* add_stri_to_array */
    if (ALLOC_STRI_SIZE_OK(new_stri, length)) {
      new_stri->size = length;
      memcpy(new_stri->mem, stri_elems,
          length * sizeof(strElemType));
      if (*used_max_position >= work_array->max_position) {
        if (work_array->max_position >= MAX_MEM_INDEX - 256) {
          resized_work_array = NULL;
        } else {
          resized_work_array = REALLOC_ARRAY(work_array,
              (uintType) work_array->max_position, (uintType) work_array->max_position + 256);
        } /* if */
        if (resized_work_array == NULL) {
          FREE_STRI(new_stri, new_stri->size);
          new_stri = NULL;
        } else {
          work_array = resized_work_array;
          COUNT3_ARRAY((uintType) work_array->max_position, (uintType) work_array->max_position + 256);
          work_array->max_position += 256;
        } /* if */
      } /* if */
    } /* if */
    if (new_stri != NULL) {
      work_array->arr[*used_max_position].type_of = take_type(SYS_STRI_TYPE);
      work_array->arr[*used_max_position].descriptor.property = NULL;
      work_array->arr[*used_max_position].value.striValue = new_stri;
      INIT_CATEGORY_OF_VAR(&work_array->arr[*used_max_position], STRIOBJECT);
      (*used_max_position)++;
    } else {
      for (position = 0; position < (uintType) *used_max_position; position++) {
        FREE_STRI(work_array->arr[position].value.striValue,
            work_array->arr[position].value.striValue->size);
      } /* for */
      FREE_ARRAY(work_array, (uintType) work_array->max_position);
      work_array = NULL;
    } /* if */
    return work_array;
  } /* add_stri_to_array */



static arrayType strChSplit (const const_striType main_stri, const charType delimiter)

  {
    intType used_max_position;
    const strElemType *search_start;
    const strElemType *search_end;
    const strElemType *found_pos;
    memSizeType pos;
    arrayType resized_result_array;
    arrayType result_array;

  /* strChSplit */
    if (ALLOC_ARRAY(result_array, 256)) {
      result_array->min_position = 1;
      result_array->max_position = 256;
      used_max_position = 0;
      search_start = main_stri->mem;
      search_end = &main_stri->mem[main_stri->size];
      while ((found_pos = search_strelem(search_start,
          delimiter, (memSizeType) (search_end - search_start))) != NULL &&
          result_array != NULL) {
        result_array = add_stri_to_array(search_start,
            (memSizeType) (found_pos - search_start), result_array,
            &used_max_position);
        search_start = found_pos + 1;
      } /* while */
      if (result_array != NULL) {
        result_array = add_stri_to_array(search_start,
            (memSizeType) (search_end - search_start), result_array,
            &used_max_position);
        if (result_array != NULL) {
          resized_result_array = REALLOC_ARRAY(result_array,
              (uintType) result_array->max_position, (uintType) used_max_position);
          if (resized_result_array == NULL) {
            for (pos = 0; pos < (uintType) used_max_position; pos++) {
              FREE_STRI(result_array->arr[pos].value.striValue,
                  result_array->arr[pos].value.striValue->size);
            } /* for */
            FREE_ARRAY(result_array, (uintType) result_array->max_position);
            result_array = NULL;
          } else {
            result_array = resized_result_array;
            COUNT3_ARRAY((uintType) result_array->max_position, (uintType) used_max_position);
            result_array->max_position = used_max_position;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(result_array == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result_array;
  } /* strChSplit */



static arrayType strSplit (const const_striType main_stri,
    const const_striType delimiter)

  {
    memSizeType delimiter_size;
    const strElemType *delimiter_mem;
    strElemType ch_1;
    intType used_max_position;
    const strElemType *search_start;
    const strElemType *segment_start;
    const strElemType *search_end;
    const strElemType *found_pos;
    memSizeType pos;
    arrayType resized_result_array;
    arrayType result_array;

  /* strSplit */
    if (ALLOC_ARRAY(result_array, 256)) {
      result_array->min_position = 1;
      result_array->max_position = 256;
      used_max_position = 0;
      delimiter_size = delimiter->size;
      delimiter_mem = delimiter->mem;
      search_start = main_stri->mem;
      segment_start = search_start;
      if (delimiter_size != 0 && main_stri->size >= delimiter_size) {
        ch_1 = delimiter_mem[0];
        search_end = &main_stri->mem[main_stri->size - delimiter_size + 1];
        while ((found_pos = search_strelem(search_start,
            ch_1, (memSizeType) (search_end - search_start))) != NULL &&
            result_array != NULL) {
          if (memcmp(found_pos, delimiter_mem,
              delimiter_size * sizeof(strElemType)) == 0) {
            result_array = add_stri_to_array(segment_start,
                (memSizeType) (found_pos - segment_start), result_array,
                &used_max_position);
            search_start = found_pos + delimiter_size;
            segment_start = search_start;
          } else {
            search_start = found_pos + 1;
          } /* if */
        } /* while */
      } /* if */
      if (result_array != NULL) {
        result_array = add_stri_to_array(segment_start,
            (memSizeType) (&main_stri->mem[main_stri->size] - segment_start), result_array,
            &used_max_position);
        if (result_array != NULL) {
          resized_result_array = REALLOC_ARRAY(result_array,
              (uintType) result_array->max_position, (uintType) used_max_position);
          if (resized_result_array == NULL) {
            for (pos = 0; pos < (uintType) used_max_position; pos++) {
              FREE_STRI(result_array->arr[pos].value.striValue,
                  result_array->arr[pos].value.striValue->size);
            } /* for */
            FREE_ARRAY(result_array, (uintType) result_array->max_position);
            result_array = NULL;
          } else {
            result_array = resized_result_array;
            COUNT3_ARRAY((uintType) result_array->max_position, (uintType) used_max_position);
            result_array->max_position = used_max_position;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(result_array == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result_array;
  } /* strSplit */



/**
 *  Append the string 'extension' to 'destination'.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             string.
 */
objectType str_append (listType arguments)

  {
    objectType str_variable;
    striType str_to;
    striType str_from;
    striType new_str;
    memSizeType new_size;
    memSizeType str_to_size;

  /* str_append */
    str_variable = arg_1(arguments);
    isit_stri(str_variable);
    is_variable(str_variable);
    str_to = take_stri(str_variable);
    isit_stri(arg_3(arguments));
    str_from = take_stri(arg_3(arguments));
    if (str_from->size != 0) {
      str_to_size = str_to->size;
      if (unlikely(str_to_size > MAX_STRI_LEN - str_from->size)) {
        /* number of bytes does not fit into memSizeType */
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_size = str_to_size + str_from->size;
        GROW_STRI(new_str, str_to, str_to_size, new_size);
        if (unlikely(new_str == NULL)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          if (str_to == str_from) {
            /* It is possible that str_to == str_from holds. */
            /* In this case 'str_from' must be corrected     */
            /* after realloc() enlarged 'str_to'.            */
            str_from = new_str;
          } /* if */
          COUNT3_STRI(str_to_size, new_size);
          memcpy(&new_str->mem[str_to_size], str_from->mem,
              str_from->size * sizeof(strElemType));
          new_str->size = new_size;
          str_variable->value.striValue = new_str;
        } /* if */
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_append */



/**
 *  Concatenate two strings.
 *  @return the result of the concatenation.
 */
objectType str_cat (listType arguments)

  {
    striType stri1;
    striType stri2;
    memSizeType stri1_size;
    memSizeType result_size;
    striType result;

  /* str_cat */
    isit_stri(arg_1(arguments));
    isit_stri(arg_3(arguments));
    stri1 = take_stri(arg_1(arguments));
    stri2 = take_stri(arg_3(arguments));
    stri1_size = stri1->size;
    if (unlikely(stri1_size > MAX_STRI_LEN - stri2->size)) {
      /* number of bytes does not fit into memSizeType */
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result_size = stri1_size + stri2->size;
      if (TEMP_OBJECT(arg_1(arguments))) {
        GROW_STRI(result, stri1, stri1_size, result_size);
        if (unlikely(result == NULL)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          COUNT3_STRI(stri1_size, result_size);
          result->size = result_size;
          memcpy(&result->mem[stri1_size], stri2->mem,
              stri2->size * sizeof(strElemType));
          arg_1(arguments)->value.striValue = NULL;
          return bld_stri_temp(result);
        } /* if */
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          result->size = result_size;
          memcpy(result->mem, stri1->mem,
              stri1_size * sizeof(strElemType));
          memcpy(&result->mem[stri1_size], stri2->mem,
              stri2->size * sizeof(strElemType));
          return bld_stri_temp(result);
        } /* if */
      } /* if */
    } /* if */
  } /* str_cat */



/**
 *  Search char 'searched' in 'mainStri' at or after 'fromIndex'.
 *  The search starts at 'fromIndex' and proceeds to the right.
 *  The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 when 'mainStri'
 *          does not contain 'searched' at or after 'fromIndex'.
 *  @exception RANGE_ERROR 'fromIndex' <= 0 holds.
 */
objectType str_chipos (listType arguments)

  { /* str_chipos */
    isit_stri(arg_1(arguments));
    isit_char(arg_2(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        strChIPos(take_stri(arg_1(arguments)), take_char(arg_2(arguments)),
                  take_int(arg_3(arguments))));
  } /* str_chipos */



/**
 *  Determine leftmost position of char 'searched' in 'mainStri'.
 *  The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 when 'mainStri'
 *          does not contain 'searched'.
 */
objectType str_chpos (listType arguments)

  { /* str_chpos */
    isit_stri(arg_1(arguments));
    isit_char(arg_2(arguments));
    return bld_int_temp(
        strChPos(take_stri(arg_1(arguments)), take_char(arg_2(arguments))));
  } /* str_chpos */



objectType str_chsplit (listType arguments)

  { /* str_chsplit */
    isit_stri(arg_1(arguments));
    isit_char(arg_2(arguments));
    return bld_array_temp(
        strChSplit(take_stri(arg_1(arguments)), take_char(arg_2(arguments))));
  } /* str_chsplit */



objectType str_clit (listType arguments)

  { /* str_clit */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        strCLit(take_stri(arg_1(arguments))));
  } /* str_clit */



/**
 *  Compare two strings.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objectType str_cmp (listType arguments)

  {
    striType stri1;
    striType stri2;
    intType signumValue;

  /* str_cmp */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    stri1 = take_stri(arg_1(arguments));
    stri2 = take_stri(arg_2(arguments));
    if (stri1->size < stri2->size) {
      signumValue = strelem_memcmp(stri1->mem, stri2->mem, stri1->size);
      if (signumValue == 0) {
        signumValue = -1;
      } /* if */
    } else {
      signumValue = strelem_memcmp(stri1->mem, stri2->mem, stri2->size);
      if (signumValue == 0 && stri1->size > stri2->size) {
        signumValue = 1;
      } /* if */
    } /* if */
    return bld_int_temp(signumValue);
  } /* str_cmp */



objectType str_cpy (listType arguments)

  {
    objectType str_to;
    objectType str_from;
    memSizeType new_size;
    striType stri_dest;

  /* str_cpy */
    str_to = arg_1(arguments);
    str_from = arg_3(arguments);
    isit_stri(str_to);
    isit_stri(str_from);
    is_variable(str_to);
    stri_dest = take_stri(str_to);
    if (TEMP_OBJECT(str_from)) {
      FREE_STRI(stri_dest, stri_dest->size);
      str_to->value.striValue = take_stri(str_from);
      str_from->value.striValue = NULL;
    } else {
      new_size = take_stri(str_from)->size;
      if (stri_dest->size == new_size) {
        if (stri_dest != take_stri(str_from)) {
          /* It is possible that str_to == str_from holds. The */
          /* behavior of memcpy() is undefined when source and */
          /* destination areas overlap (or are identical).     */
          /* Therefore a check for this case is necessary.     */
          memcpy(stri_dest->mem, take_stri(str_from)->mem,
              new_size * sizeof(strElemType));
        } /* if */
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(stri_dest, new_size))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          FREE_STRI(take_stri(str_to), take_stri(str_to)->size);
          str_to->value.striValue = stri_dest;
          stri_dest->size = new_size;
        } /* if */
        memcpy(stri_dest->mem, take_stri(str_from)->mem,
            new_size * sizeof(strElemType));
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_cpy */



objectType str_create (listType arguments)

  {
    objectType str_to;
    objectType str_from;
    memSizeType new_size;
    striType new_str;

  /* str_create */
    str_to = arg_1(arguments);
    str_from = arg_3(arguments);
    isit_stri(str_from);
    SET_CATEGORY_OF_OBJ(str_to, STRIOBJECT);
    if (TEMP_OBJECT(str_from)) {
      str_to->value.striValue = take_stri(str_from);
      str_from->value.striValue = NULL;
    } else {
/*    printf("str_create %d !!!\n", in_file.line); */
      new_size = take_stri(str_from)->size;
      if (unlikely(!ALLOC_STRI_SIZE_OK(new_str, new_size))) {
        str_to->value.striValue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      str_to->value.striValue = new_str;
      new_str->size = new_size;
      memcpy(new_str->mem, take_stri(str_from)->mem,
          new_size * sizeof(strElemType));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_create */



objectType str_destr (listType arguments)

  {
    striType old_string;

  /* str_destr */
    isit_stri(arg_1(arguments));
    old_string = take_stri(arg_1(arguments));
    if (old_string != NULL) {
      FREE_STRI(old_string, old_string->size);
      arg_1(arguments)->value.striValue = NULL;
    } /* if */
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* str_destr */



/**
 *  Assign char 'source' to the 'position' of the 'destination'.
 *   A @:= [B] C;
 *  is equivalent to
 *   A := A[..pred(B)] & str(C) & A[succ(B)..];
 *  @exception RANGE_ERROR When 'position' is negative or zero.
 *  @exception RANGE_ERROR A character beyond 'destination' would be
 *             overwritten ('position' > length('destination') holds).
 */
objectType str_elemcpy (listType arguments)

  {
    striType stri;
    intType position;

  /* str_elemcpy */
    isit_stri(arg_1(arguments));
    isit_int(arg_4(arguments));
    isit_char(arg_6(arguments));
    is_variable(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    position = take_int(arg_4(arguments));
    if (unlikely(position <= 0 || (uintType) position > stri->size)) {
      logError(printf("str_elemcpy(\"%s\", " FMT_D ", '\\" FMT_U32 ";'): "
                      "Position %s.\n",
                      striAsUnquotedCStri(stri), position,
                      take_char(arg_6(arguments)),
                      position <= 0 ? "<= 0" : "> length(destination)"););
     return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      stri->mem[position - 1] = (strElemType) take_char(arg_6(arguments));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_elemcpy */



/**
 *  Check if two strings are equal.
 *  @return TRUE if both strings are equal,
 *          FALSE otherwise.
 */
objectType str_eq (listType arguments)

  {
    striType stri1;
    striType stri2;

  /* str_eq */
    isit_stri(arg_1(arguments));
    isit_stri(arg_3(arguments));
    stri1 = take_stri(arg_1(arguments));
    stri2 = take_stri(arg_3(arguments));
    if (stri1->size == stri2->size && memcmp(stri1->mem, stri2->mem,
        stri1->size * sizeof(strElemType)) == 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* str_eq */



/**
 *  Check if stri1 is greater than or equal to stri2.
 *  @return TRUE if stri1 is greater than or equal to stri2,
 *          FALSE otherwise.
 */
objectType str_ge (listType arguments)

  {
    striType stri1;
    striType stri2;
    objectType result;

  /* str_ge */
    isit_stri(arg_1(arguments));
    isit_stri(arg_3(arguments));
    stri1 = take_stri(arg_1(arguments));
    stri2 = take_stri(arg_3(arguments));
    if (stri1->size >= stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri2->size) >= 0) {
        result = SYS_TRUE_OBJECT;
      } else {
        result = SYS_FALSE_OBJECT;
      } /* if */
    } else {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri1->size) > 0) {
        result = SYS_TRUE_OBJECT;
      } else {
        result = SYS_FALSE_OBJECT;
      } /* if */
    } /* if */
    return result;
  } /* str_ge */



/**
 *  Check if stri1 is greater than stri2.
 *  @return TRUE if stri1 is greater than stri2,
 *          FALSE otherwise.
 */
objectType str_gt (listType arguments)

  {
    striType stri1;
    striType stri2;
    objectType result;

  /* str_gt */
    isit_stri(arg_1(arguments));
    isit_stri(arg_3(arguments));
    stri1 = take_stri(arg_1(arguments));
    stri2 = take_stri(arg_3(arguments));
    if (stri1->size > stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri2->size) >= 0) {
        result = SYS_TRUE_OBJECT;
      } else {
        result = SYS_FALSE_OBJECT;
      } /* if */
    } else {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri1->size) > 0) {
        result = SYS_TRUE_OBJECT;
      } else {
        result = SYS_FALSE_OBJECT;
      } /* if */
    } /* if */
    return result;
  } /* str_gt */



/**
 *  Compute the hash value of a string.
 *  @return the hash value.
 */
objectType str_hashcode (listType arguments)

  {
    striType stri;

  /* str_hashcode */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    return bld_int_temp(hashCode(stri));
  } /* str_hashcode */



/**
 *  Get a substring ending at a stop position.
 *  The first character in a string has the position 1.
 *  @return the substring ending at the stop position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType str_head (listType arguments)

  {
    striType stri;
    intType stop;
    memSizeType striSize;
    memSizeType result_size;
    striType result;

  /* str_head */
    isit_stri(arg_1(arguments));
    isit_int(arg_4(arguments));
    stri = take_stri(arg_1(arguments));
    stop = take_int(arg_4(arguments));
    striSize = stri->size;
    if (stop >= 1 && striSize >= 1) {
      if (striSize <= (uintType) stop) {
        result_size = striSize;
      } else {
        result_size = (memSizeType) stop;
      } /* if */
      if (TEMP_OBJECT(arg_1(arguments))) {
        SHRINK_STRI(result, stri, striSize, result_size);
        if (unlikely(result == NULL)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        COUNT3_STRI(striSize, result_size);
        result->size = result_size;
        arg_1(arguments)->value.striValue = NULL;
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->size = result_size;
        memcpy(result->mem, stri->mem,
            result_size * sizeof(strElemType));
      } /* if */
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memSizeType) 0))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->size = 0;
    } /* if */
    return bld_stri_temp(result);
  } /* str_head */



/**
 *  Get a character, identified by an index, from a 'string'.
 *  The first character has the index 1.
 *  @return the character specified with the index.
 *  @exception RANGE_ERROR When the index is less than 1 or
 *             greater than the length of the 'string'.
 */
objectType str_idx (listType arguments)

  {
    striType stri;
    intType position;

  /* str_idx */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    stri = take_stri(arg_1(arguments));
    position = take_int(arg_3(arguments));
    if (unlikely(position <= 0 || (uintType) position > stri->size)) {
      logError(printf("str_idx(\"%s\", " FMT_D "): Position %s.\n",
                      striAsUnquotedCStri(stri), position,
                      position <= 0 ? "<= 0" : "> length(string)"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_char_temp((charType) stri->mem[position - 1]);
    } /* if */
  } /* str_idx */



/**
 *  Search string 'searched' in 'mainStri' at or after 'fromIndex'.
 *  The search starts at 'fromIndex' and proceeds to the right.
 *  The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 when 'mainStri'
 *          does not contain 'searched' at or after 'fromIndex'.
 *  @exception RANGE_ERROR 'fromIndex' <= 0 holds.
 */
objectType str_ipos (listType arguments)

  { /* str_ipos */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        strIPos(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)),
                take_int(arg_3(arguments))));
  } /* str_ipos */



/**
 *  Check if stri1 is less than or equal to stri2.
 *  @return TRUE if stri1 is less than or equal to stri2,
 *          FALSE otherwise.
 */
objectType str_le (listType arguments)

  {
    striType stri1;
    striType stri2;
    objectType result;

  /* str_le */
    isit_stri(arg_1(arguments));
    isit_stri(arg_3(arguments));
    stri1 = take_stri(arg_1(arguments));
    stri2 = take_stri(arg_3(arguments));
    if (stri1->size <= stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri1->size) <= 0) {
        result = SYS_TRUE_OBJECT;
      } else {
        result = SYS_FALSE_OBJECT;
      } /* if */
    } else {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri2->size) < 0) {
        result = SYS_TRUE_OBJECT;
      } else {
        result = SYS_FALSE_OBJECT;
      } /* if */
    } /* if */
    return result;
  } /* str_le */



objectType str_lit (listType arguments)

  { /* str_lit */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        strLit(take_stri(arg_1(arguments))));
  } /* str_lit */



/**
 *  Determine the length of a 'string'.
 *  @return the length of the 'string'.
 */
objectType str_lng (listType arguments)

  {
    striType stri;

  /* str_lng */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
#if POINTER_SIZE > INTTYPE_SIZE
    if (unlikely(stri->size > MAX_MEM_INDEX)) {
      logError(printf("str_lng(\"%s\"): Length does not fit into integer.\n",
                      striAsUnquotedCStri(stri)););
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
#endif
    return bld_int_temp((intType) stri->size);
  } /* str_lng */



/**
 *  Convert a string to lower case.
 *  The conversion uses the default Unicode case mapping,
 *  where each character is considered in isolation.
 *  Characters without case mapping are left unchanged.
 *  The mapping is independend from the locale. Individual
 *  character case mappings cannot be reversed, because some
 *  characters have multiple characters that map to them.
 *  @return the string converted to lower case.
 */
objectType str_low (listType arguments)

  { /* str_low */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        strLow(take_stri(arg_1(arguments))));
  } /* str_low */



/**
 *  Pad a string with spaces at the left side up to pad_size.
 *  @return the string left padded with spaces.
 */
objectType str_lpad (listType arguments)

  {
    striType stri;
    intType pad_size;
    memSizeType striSize;
    striType result;

  /* str_lpad */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    stri = take_stri(arg_1(arguments));
    pad_size = take_int(arg_3(arguments));
    striSize = stri->size;
    if (pad_size > 0 && (uintType) pad_size > striSize) {
      if (unlikely((uintType) pad_size > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memSizeType) pad_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = (memSizeType) pad_size;
        {
          strElemType *elem = result->mem;
          memSizeType idx = (memSizeType) pad_size - striSize - 1;

          do {
            elem[idx] = (strElemType) ' ';
          } while (idx-- != 0);
        }
        memcpy(&result->mem[(memSizeType) pad_size - striSize], stri->mem,
            striSize * sizeof(strElemType));
      } /* if */
    } else {
      if (TEMP_OBJECT(arg_1(arguments))) {
        result = stri;
        arg_1(arguments)->value.striValue = NULL;
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->size = striSize;
        memcpy(result->mem, stri->mem,
            striSize * sizeof(strElemType));
      } /* if */
    } /* if */
    return bld_stri_temp(result);
  } /* str_lpad */



/**
 *  Pad a string with zeroes at the left side up to pad_size.
 *  @return the string left padded with zeroes.
 */
objectType str_lpad0 (listType arguments)

  {
    striType stri;
    intType pad_size;
    memSizeType striSize;
    strElemType *sourceElem;
    strElemType *destElem;
    memSizeType len;
    striType result;

  /* str_lpad0 */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    stri = take_stri(arg_1(arguments));
    pad_size = take_int(arg_3(arguments));
    striSize = stri->size;
    if (pad_size > 0 && (uintType) pad_size > striSize) {
      if (unlikely((uintType) pad_size > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memSizeType) pad_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = (memSizeType) pad_size;
        sourceElem = stri->mem;
        destElem = result->mem;
        len = (memSizeType) pad_size - striSize;
        if (striSize != 0 && (sourceElem[0] == '-' || sourceElem[0] == '+')) {
          *destElem++ = sourceElem[0];
          sourceElem++;
          striSize--;
        } /* if */
        while (len--) {
          *destElem++ = (strElemType) '0';
        } /* while */
        memcpy(destElem, sourceElem, striSize * sizeof(strElemType));
      } /* if */
    } else {
      if (TEMP_OBJECT(arg_1(arguments))) {
        result = stri;
        arg_1(arguments)->value.striValue = NULL;
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->size = striSize;
        memcpy(result->mem, stri->mem,
            striSize * sizeof(strElemType));
      } /* if */
    } /* if */
    return bld_stri_temp(result);
  } /* str_lpad0 */



/**
 *  Check if stri1 is less than stri2.
 *  @return TRUE if stri1 is less than stri2,
 *          FALSE otherwise.
 */
objectType str_lt (listType arguments)

  {
    striType stri1;
    striType stri2;
    objectType result;

  /* str_lt */
    isit_stri(arg_1(arguments));
    isit_stri(arg_3(arguments));
    stri1 = take_stri(arg_1(arguments));
    stri2 = take_stri(arg_3(arguments));
    if (stri1->size < stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri1->size) <= 0) {
        result = SYS_TRUE_OBJECT;
      } else {
        result = SYS_FALSE_OBJECT;
      } /* if */
    } else {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri2->size) < 0) {
        result = SYS_TRUE_OBJECT;
      } else {
        result = SYS_FALSE_OBJECT;
      } /* if */
    } /* if */
    return result;
  } /* str_lt */



/**
 *  Return string with leading whitespace omitted.
 *  All characters less than or equal to ' ' (space) count as whitespace.
 *  @return string with leading whitespace omitted.
 */
objectType str_ltrim (listType arguments)

  {
    striType stri;
    memSizeType start;
    memSizeType striSize;
    striType result;

  /* str_ltrim */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    start = 0;
    striSize = stri->size;
    if (striSize >= 1) {
      while (start < striSize && stri->mem[start] <= ' ') {
        start++;
      } /* while */
      striSize -= start;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->size = striSize;
      memcpy(result->mem, &stri->mem[start],
          striSize * sizeof(strElemType));
      return bld_stri_temp(result);
    } /* if */
  } /* str_ltrim */



objectType str_mult (listType arguments)

  { /* str_mult */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        strMult(take_stri(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* str_mult */



/**
 *  Check if two strings are not equal.
 *  @return FALSE if both strings are equal,
 *          TRUE otherwise.
 */
objectType str_ne (listType arguments)

  {
    striType stri1;
    striType stri2;

  /* str_ne */
    isit_stri(arg_1(arguments));
    isit_stri(arg_3(arguments));
    stri1 = take_stri(arg_1(arguments));
    stri2 = take_stri(arg_3(arguments));
    if (stri1->size != stri2->size || memcmp(stri1->mem, stri2->mem,
        stri1->size * sizeof(strElemType)) != 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* str_ne */



/**
 *  Determine leftmost position of string 'searched' in 'mainStri'.
 *  When the string is found the position of its first character
 *  is the result. The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 when 'mainStri'
 *          does not contain 'searched'.
 */
objectType str_pos (listType arguments)

  { /* str_pos */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_int_temp(
        strPos(take_stri(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* str_pos */



/**
 *  Assign string 'source' to the 'position' of the 'destination'.
 *   A @:= [B] C;
 *  is equivalent to
 *   A := A[..pred(B)] & C & A[B+length(C)..];
 *  @exception RANGE_ERROR When 'position' is negative or zero.
 *  @exception RANGE_ERROR When 'destination' is smaller than 'source'.
 *  @exception RANGE_ERROR Characters beyond 'destination' would be
 *             overwritten ('position' + length('source') >
 *             succ(length('destination')) holds).
 */
objectType str_poscpy (listType arguments)

  {
    striType destStri;
    striType sourceStri;
    intType position;

  /* str_poscpy */
    isit_stri(arg_1(arguments));
    isit_int(arg_4(arguments));
    isit_stri(arg_6(arguments));
    is_variable(arg_1(arguments));
    destStri = take_stri(arg_1(arguments));
    position = take_int(arg_4(arguments));
    sourceStri = take_stri(arg_6(arguments));
    if (unlikely(position <= 0 || destStri->size < sourceStri->size ||
                 (uintType) position > destStri->size - sourceStri->size + 1)) {
      logError(printf("str_poscpy(\"%s\", " FMT_D ", ",
                      striAsUnquotedCStri(destStri), position);
               printf("\"%s\"): Position not in allowed range.\n",
                      striAsUnquotedCStri(sourceStri)););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      /* It is possible that destStri and sourceStri overlap. */
      /* E.g. for the expression: stri @:= [idx] stri;        */
      /* The behavior of memcpy() is undefined when source    */
      /* and destination areas overlap (or are identical).    */
      /* Therefore memmove() is used instead of memcpy().     */
      memmove(&destStri->mem[position - 1], sourceStri->mem,
          sourceStri->size * sizeof(strElemType));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_poscpy */



/**
 *  Append the char 'extension' to 'destination'.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             string.
 */
objectType str_push (listType arguments)

  {
    objectType str_variable;
    striType str_to;
    charType char_from;
    memSizeType new_size;

  /* str_push */
    str_variable = arg_1(arguments);
    isit_stri(str_variable);
    is_variable(str_variable);
    str_to = take_stri(str_variable);
    isit_char(arg_3(arguments));
    char_from = take_char(arg_3(arguments));
    new_size = str_to->size + 1;
    GROW_STRI(str_to, str_to, str_to->size, new_size);
    if (unlikely(str_to == NULL)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      COUNT3_STRI(str_to->size, new_size);
      str_to->mem[str_to->size] = char_from;
      str_to->size = new_size;
      str_variable->value.striValue = str_to;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_push */



/**
 *  Get a substring from a start position to a stop position.
 *  The first character in a string has the position 1.
 *  @return the substring from position start to stop.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType str_range (listType arguments)

  {
    striType stri;
    intType start;
    intType stop;
    memSizeType striSize;
    memSizeType result_size;
    striType result;

  /* str_range */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_5(arguments));
    stri = take_stri(arg_1(arguments));
    start = take_int(arg_3(arguments));
    stop = take_int(arg_5(arguments));
    striSize = stri->size;
    if (start < 1) {
      start = 1;
    } /* if */
    if (stop >= start && (uintType) start <= striSize) {
      if ((uintType) stop > striSize) {
        result_size = striSize - (memSizeType) start + 1;
      } else {
        result_size = (memSizeType) stop - (memSizeType) start + 1;
      } /* if */
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      /* Reversing the order of the following two statements    */
      /* causes an "Internal Compiler Error" with MSC 6.0       */
      /* when using the -Ozacegilt optimisation option in the   */
      /* large memory model (-AL). Note that the order of the   */
      /* two statements make no difference to the logic of the  */
      /* program.                                               */
      memcpy(result->mem, &stri->mem[start - 1],
          result_size * sizeof(strElemType));
      result->size = result_size;
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memSizeType) 0))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->size = 0;
    } /* if */
    return bld_stri_temp(result);
  } /* str_range */



/**
 *  Search char 'searched' in 'mainStri' at or before 'fromIndex'.
 *  The search starts at 'fromIndex' and proceeds to the left.
 *  The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 when 'mainStri'
 *          does not contain 'searched' at or before 'fromIndex'.
 *  @exception RANGE_ERROR 'fromIndex' > length(stri) holds.
 */
objectType str_rchipos (listType arguments)

  { /* str_rchipos */
    isit_stri(arg_1(arguments));
    isit_char(arg_2(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        strRChIPos(take_stri(arg_1(arguments)), take_char(arg_2(arguments)),
                   take_int(arg_3(arguments))));
  } /* str_rchipos */



/**
 *  Determine rightmost position of char 'searched' in 'mainStri'.
 *  The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 when 'mainStri'
 *          does not contain 'searched'.
 */
objectType str_rchpos (listType arguments)

  { /* str_rchpos */
    isit_stri(arg_1(arguments));
    isit_char(arg_2(arguments));
    return bld_int_temp(
        strRChPos(take_stri(arg_1(arguments)), take_char(arg_2(arguments))));
  } /* str_rchpos */



/**
 *  Replace all occurrences of 'searched' in 'mainStri' by 'replacement'.
 *  @return the result of the replacement.
 */
objectType str_repl (listType arguments)

  { /* str_repl */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_stri(arg_3(arguments));
    return bld_stri_temp(
        strRepl(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)),
                take_stri(arg_3(arguments))));
  } /* str_repl */



/**
 *  Search string 'searched' in 'mainStri' at or before 'fromIndex'.
 *  The search starts at 'fromIndex' and proceeds to the left.
 *  The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 when 'mainStri'
 *          does not contain 'searched' at or before 'fromIndex'.
 *  @exception RANGE_ERROR 'fromIndex' > length(stri) holds.
 */
objectType str_ripos (listType arguments)

  { /* str_ripos */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        strRIPos(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)),
                 take_int(arg_3(arguments))));
  } /* str_ripos */



/**
 *  Pad a string with spaces at the right side up to pad_size.
 *  @return the string right padded with spaces.
 */
objectType str_rpad (listType arguments)

  {
    striType stri;
    intType pad_size;
    memSizeType striSize;
    striType result;

  /* str_rpad */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    stri = take_stri(arg_1(arguments));
    pad_size = take_int(arg_3(arguments));
    striSize = stri->size;
    if (pad_size > 0 && (uintType) pad_size > striSize) {
      if (unlikely((uintType) pad_size > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memSizeType) pad_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = (memSizeType) pad_size;
        memcpy(result->mem, stri->mem, striSize * sizeof(strElemType));
        {
          strElemType *elem = &result->mem[striSize];
          memSizeType len = (memSizeType) pad_size - striSize;

          while (len--) {
           *elem++ = (strElemType) ' ';
          } /* while */
        }
      } /* if */
    } else {
      if (TEMP_OBJECT(arg_1(arguments))) {
        result = stri;
        arg_1(arguments)->value.striValue = NULL;
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->size = striSize;
        memcpy(result->mem, stri->mem,
            striSize * sizeof(strElemType));
      } /* if */
    } /* if */
    return bld_stri_temp(result);
  } /* str_rpad */



/**
 *  Determine rightmost position of string 'searched' in 'mainStri'.
 *  When the string is found the position of its first character
 *  is the result. The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 when 'mainStri'
 *          does not contain 'searched'.
 */
objectType str_rpos (listType arguments)

  { /* str_rpos */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_int_temp(
        strRPos(take_stri(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* str_rpos */



/**
 *  Return string with trailing whitespace omitted.
 *  All characters less than or equal to ' ' (space) count as whitespace.
 *  @return string with trailing whitespace omitted.
 */
objectType str_rtrim (listType arguments)

  {
    striType stri;
    memSizeType striSize;
    memSizeType result_size;
    striType result;

  /* str_rtrim */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    result_size = stri->size;
    while (result_size > 0 && stri->mem[result_size - 1] <= ' ') {
      result_size--;
    } /* while */
    if (TEMP_OBJECT(arg_1(arguments))) {
      striSize = stri->size;
      SHRINK_STRI(result, stri, striSize, result_size);
      if (unlikely(result == NULL)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      COUNT3_STRI(striSize, result_size);
      result->size = result_size;
      arg_1(arguments)->value.striValue = NULL;
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->size = result_size;
      memcpy(result->mem, stri->mem,
          result_size * sizeof(strElemType));
    } /* if */
    return bld_stri_temp(result);
  } /* str_rtrim */



objectType str_split (listType arguments)

  { /* str_split */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_array_temp(
        strSplit(take_stri(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* str_split */



/**
 *  Convert to a string.
 *  @return its parameter unchanged.
 */
objectType str_str (listType arguments)

  {
    striType stri;
    striType result;

  /* str_str */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    if (TEMP_OBJECT(arg_1(arguments))) {
      result = stri;
      arg_1(arguments)->value.striValue = NULL;
      return bld_stri_temp(result);
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, stri->size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = stri->size;
        memcpy(result->mem, stri->mem,
            stri->size * sizeof(strElemType));
        return bld_stri_temp(result);
      } /* if */
    } /* if */
  } /* str_str */



/**
 *  Get a substring from a start position with a given length.
 *  The first character in a string has the position 1.
 *  @return the substring from the start position with a given length.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType str_substr (listType arguments)

  {
    striType stri;
    intType start;
    intType length;
    memSizeType striSize;
    memSizeType result_size;
    striType result;

  /* str_substr */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_5(arguments));
    stri = take_stri(arg_1(arguments));
    start = take_int(arg_3(arguments));
    length = take_int(arg_5(arguments));
    striSize = stri->size;
    if (unlikely(start < 1)) {
      if (length >= 1 && start > 1 - length) {
        length += start - 1;
        start = 1;
      } else {
        length = 0;
      } /* if */
    } /* if */
    if (length >= 1 && (uintType) start <= striSize) {
      if ((uintType) length > striSize - (memSizeType) start + 1) {
        result_size = striSize - (memSizeType) start + 1;
      } else {
        result_size = (memSizeType) length;
      } /* if */
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      memcpy(result->mem, &stri->mem[start - 1],
          result_size * sizeof(strElemType));
      result->size = result_size;
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memSizeType) 0))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->size = 0;
    } /* if */
    return bld_stri_temp(result);
  } /* str_substr */



/**
 *  Get a substring beginning at a start position.
 *  The first character in a 'string' has the position 1.
 *  @return the substring beginning at the start position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType str_tail (listType arguments)

  {
    striType stri;
    intType start;
    memSizeType striSize;
    memSizeType result_size;
    striType result;

  /* str_tail */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    stri = take_stri(arg_1(arguments));
    start = take_int(arg_3(arguments));
    striSize = stri->size;
    if (start < 1) {
      start = 1;
    } /* if */
    if ((uintType) start <= striSize && striSize >= 1) {
      result_size = striSize - (memSizeType) start + 1;
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      /* Reversing the order of the following two statements    */
      /* causes an "Internal Compiler Error" with MSC 6.0       */
      /* when using the -Ozacegilt optimisation option in the   */
      /* large memory model (-AL). Note that the order of the   */
      /* two statements make no difference to the logic of the  */
      /* program.                                               */
      memcpy(result->mem, &stri->mem[start - 1],
          result_size * sizeof(strElemType));
      result->size = result_size;
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memSizeType) 0))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->size = 0;
    } /* if */
    return bld_stri_temp(result);
  } /* str_tail */



/**
 *  Convert a string to an UTF-8 encoded string of bytes.
 *  @param stri/arg_1 Normal (UTF-32) string to be converted to UTF-8.
 *  @return 'stri' converted to a string of bytes with UTF-8 encoding.
 */
objectType str_toutf8 (listType arguments)

  { /* str_toutf8 */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        strToUtf8(take_stri(arg_1(arguments))));
  } /* str_toutf8 */



/**
 *  Return string with leading and trailing whitespace omitted.
 *  All characters less than or equal to ' ' (space) count as whitespace.
 *  @return string with leading and trailing whitespace omitted.
 */
objectType str_trim (listType arguments)

  {
    striType stri;
    memSizeType start;
    memSizeType striSize;
    striType result;

  /* str_trim */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    start = 0;
    striSize = stri->size;
    if (striSize >= 1) {
      while (start < striSize && stri->mem[start] <= ' ') {
        start++;
      } /* while */
      while (striSize > start && stri->mem[striSize - 1] <= ' ') {
        striSize--;
      } /* while */
      striSize -= start;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, striSize))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->size = striSize;
      memcpy(result->mem, &stri->mem[start],
          striSize * sizeof(strElemType));
      return bld_stri_temp(result);
    } /* if */
  } /* str_trim */



/**
 *  Convert a string to upper case.
 *  The conversion uses the default Unicode case mapping,
 *  where each character is considered in isolation.
 *  Characters without case mapping are left unchanged.
 *  The mapping is independend from the locale. Individual
 *  character case mappings cannot be reversed, because some
 *  characters have multiple characters that map to them.
 *  @return the string converted to upper case.
 */
objectType str_up (listType arguments)

  { /* str_up */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        strUp(take_stri(arg_1(arguments))));
  } /* str_up */



/**
 *  Convert a string with bytes in UTF-8 encoding to UTF-32.
 *  @param utf8/arg_1 String of bytes encoded with UTF-8.
 *  @return 'utf8' converted to a normal (UTF-32) string.
 *  @exception RANGE_ERROR When characters beyond '\255;' are present or
 *                         when 'utf8' is not encoded with UTF-8.
 */
objectType str_utf8tostri (listType arguments)

  { /* str_utf8tostri */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        strUtf8ToStri(take_stri(arg_1(arguments))));
  } /* str_utf8tostri */



objectType str_value (listType arguments)

  {
    objectType obj_arg;
    striType stri;
    striType result;

  /* str_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (unlikely(obj_arg == NULL ||
                 CATEGORY_OF_OBJ(obj_arg) != STRIOBJECT ||
                 take_stri(obj_arg) == NULL)) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      stri = take_stri(obj_arg);
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, stri->size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = stri->size;
        memcpy(result->mem, stri->mem,
            result->size * sizeof(strElemType));
        return bld_stri_temp(result);
      } /* if */
    } /* if */
  } /* str_value */
