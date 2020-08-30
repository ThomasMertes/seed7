/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2019  Thomas Mertes                        */
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
/*  Changes: 1991 - 1994, 2008, 2010, 2013 - 2019  Thomas Mertes    */
/*  Content: All primitive actions for the string type.             */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "wchar.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "chclsutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "objutl.h"
#include "exec.h"
#include "runerr.h"
#include "str_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "strlib.h"


#define INITIAL_ARRAY_SIZE 256
#define ARRAY_SIZE_FACTOR    2



#if HAS_WMEMCMP && WCHAR_T_SIZE == 32
#define memcmp_strelem(mem1, mem2, len) \
    wmemcmp((const wchar_t *) mem1, (const wchar_t *) mem2, (size_t) len)
#else



static inline int memcmp_strelem (register const strElemType *mem1,
    register const strElemType *mem2, memSizeType len)

  { /* memcmp_strelem */
    for (; len > 0; mem1++, mem2++, len--) {
      if (*mem1 != *mem2) {
        return *mem1 < *mem2 ? -1 : 1;
      } /* if */
    } /* for */
    return 0;
  } /* memcmp_strelem */

#endif



static void freeStriArray (arrayType work_array, intType used_max_position)

  {
    memSizeType position;

  /* freeStriArray */
    for (position = 0; position < (uintType) used_max_position; position++) {
      FREE_STRI(work_array->arr[position].value.striValue,
                work_array->arr[position].value.striValue->size);
    } /* for */
    FREE_ARRAY(work_array, (uintType) work_array->max_position);
  } /* freeStriArray */



static arrayType addCopiedStriToArray (const strElemType *stri_elems,
    memSizeType length, arrayType work_array, intType used_max_position)

  {
    striType new_stri;
    arrayType resized_work_array;

  /* addCopiedStriToArray */
    if (ALLOC_STRI_SIZE_OK(new_stri, length)) {
      new_stri->size = length;
      memcpy(new_stri->mem, stri_elems, length * sizeof(strElemType));
      if (used_max_position >= work_array->max_position) {
        if (unlikely(work_array->max_position >= MAX_ARR_INDEX / ARRAY_SIZE_FACTOR ||
            (resized_work_array = REALLOC_ARRAY(work_array,
                (uintType) work_array->max_position,
                (uintType) work_array->max_position * ARRAY_SIZE_FACTOR)) == NULL)) {
          FREE_STRI(new_stri, new_stri->size);
          freeStriArray(work_array, used_max_position);
          work_array = NULL;
        } else {
          work_array = resized_work_array;
          COUNT3_ARRAY((uintType) work_array->max_position,
                       (uintType) work_array->max_position * ARRAY_SIZE_FACTOR);
          work_array->max_position *= ARRAY_SIZE_FACTOR;
        } /* if */
      } /* if */
      if (likely(work_array != NULL)) {
        work_array->arr[used_max_position].type_of = take_type(SYS_STRI_TYPE);
        work_array->arr[used_max_position].descriptor.property = NULL;
        work_array->arr[used_max_position].value.striValue = new_stri;
        INIT_CATEGORY_OF_VAR(&work_array->arr[used_max_position], STRIOBJECT);
      } /* if */
    } else {
      freeStriArray(work_array, used_max_position);
      work_array = NULL;
    } /* if */
    return work_array;
  } /* addCopiedStriToArray */



static inline arrayType completeStriArray (arrayType work_array,
    intType used_max_position)

  {
    arrayType resized_work_array;

  /* completeStriArray */
    if (work_array != NULL) {
      resized_work_array = REALLOC_ARRAY(work_array,
          (uintType) work_array->max_position,
          (uintType) used_max_position);
      if (resized_work_array == NULL) {
        freeStriArray(work_array, used_max_position);
        work_array = NULL;
      } else {
        work_array = resized_work_array;
        COUNT3_ARRAY((uintType) work_array->max_position,
                     (uintType) used_max_position);
        work_array->max_position = used_max_position;
      } /* if */
    } /* if */
    return work_array;
  } /* completeStriArray */



static arrayType strChSplit (const const_striType mainStri,
    const charType delimiter)

  {
    intType used_max_position;
    const strElemType *search_start;
    const strElemType *search_end;
    const strElemType *found_pos;
    arrayType result_array;

  /* strChSplit */
    logFunction(printf("strChSplit(\"%s\", '\\" FMT_U32 ";')\n",
                       striAsUnquotedCStri(mainStri), delimiter););
    if (ALLOC_ARRAY(result_array, INITIAL_ARRAY_SIZE)) {
      result_array->min_position = 1;
      result_array->max_position = INITIAL_ARRAY_SIZE;
      used_max_position = 0;
      search_start = mainStri->mem;
      search_end = &mainStri->mem[mainStri->size];
      while ((found_pos = memchr_strelem(search_start, delimiter,
          (memSizeType) (search_end - search_start))) != NULL &&
          result_array != NULL) {
        result_array = addCopiedStriToArray(search_start,
            (memSizeType) (found_pos - search_start), result_array,
            used_max_position);
        used_max_position++;
        search_start = found_pos + 1;
      } /* while */
      if (likely(result_array != NULL)) {
        result_array = addCopiedStriToArray(search_start,
            (memSizeType) (search_end - search_start), result_array,
            used_max_position);
        used_max_position++;
        result_array = completeStriArray(result_array, used_max_position);
      } /* if */
    } /* if */
    if (unlikely(result_array == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("strChSplit -->\n"););
    return result_array;
  } /* strChSplit */



static arrayType strSplit (const const_striType mainStri,
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
    arrayType result_array;

  /* strSplit */
    logFunction(printf("strChSplit(\"%s\", ",
                       striAsUnquotedCStri(mainStri));
                printf("\"%s\")\n", striAsUnquotedCStri(delimiter)););
    if (ALLOC_ARRAY(result_array, INITIAL_ARRAY_SIZE)) {
      result_array->min_position = 1;
      result_array->max_position = INITIAL_ARRAY_SIZE;
      used_max_position = 0;
      delimiter_size = delimiter->size;
      delimiter_mem = delimiter->mem;
      search_start = mainStri->mem;
      segment_start = search_start;
      if (delimiter_size != 0 && mainStri->size >= delimiter_size) {
        ch_1 = delimiter_mem[0];
        search_end = &mainStri->mem[mainStri->size - delimiter_size + 1];
        while ((found_pos = memchr_strelem(search_start, ch_1,
            (memSizeType) (search_end - search_start))) != NULL &&
            result_array != NULL) {
          if (memcmp(found_pos, delimiter_mem,
              delimiter_size * sizeof(strElemType)) == 0) {
            result_array = addCopiedStriToArray(segment_start,
                (memSizeType) (found_pos - segment_start), result_array,
                used_max_position);
            used_max_position++;
            search_start = found_pos + delimiter_size;
            segment_start = search_start;
            if (search_start > search_end) {
              search_start = search_end;
            } /* if */
          } else {
            search_start = found_pos + 1;
          } /* if */
        } /* while */
      } /* if */
      if (likely(result_array != NULL)) {
        result_array = addCopiedStriToArray(segment_start,
            (memSizeType) (&mainStri->mem[mainStri->size] - segment_start),
            result_array, used_max_position);
        used_max_position++;
        result_array = completeStriArray(result_array, used_max_position);
      } /* if */
    } /* if */
    if (unlikely(result_array == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("strSplit -->\n"););
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
          COUNT_GROW_STRI(str_to_size, new_size);
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
    logFunction(printf("str_cat(\"%s\", ",
                       striAsUnquotedCStri(stri1));
                printf("\"%s\")\n",
                       striAsUnquotedCStri(stri2)););
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
          COUNT_GROW_STRI(stri1_size, result_size);
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
 *  @return the position of 'searched' or 0 if 'mainStri'
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
 *  @return the position of 'searched' or 0 if 'mainStri'
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
#if !HAS_WMEMCMP || WCHAR_T_SIZE != 32 || WMEMCMP_RETURNS_SIGNUM
    if (stri1->size < stri2->size) {
      signumValue = memcmp_strelem(stri1->mem, stri2->mem, stri1->size);
      if (signumValue == 0) {
        signumValue = -1;
      } /* if */
    } else {
      signumValue = memcmp_strelem(stri1->mem, stri2->mem, stri2->size);
      if (signumValue == 0 && stri1->size > stri2->size) {
        signumValue = 1;
      } /* if */
    } /* if */
#else
    if (stri1->size < stri2->size) {
      if (memcmp_strelem(stri1->mem, stri2->mem, stri1->size) <= 0) {
        signumValue = -1;
      } else {
        signumValue = 1;
      } /* if */
    } else {
      signumValue = memcmp_strelem(stri1->mem, stri2->mem, stri2->size);
      if (signumValue == 0) {
        if (stri1->size > stri2->size) {
          signumValue = 1;
        } /* if */
      } else if (signumValue > 0) {
        signumValue = 1;
      } else {
        signumValue = -1;
      } /* if */
    } /* if */
#endif
    return bld_int_temp(signumValue);
  } /* str_cmp */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType str_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    memSizeType new_size;
    striType stri_dest;

  /* str_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_stri(dest);
    isit_stri(source);
    is_variable(dest);
    stri_dest = take_stri(dest);
    if (TEMP_OBJECT(source)) {
      FREE_STRI(stri_dest, stri_dest->size);
      dest->value.striValue = take_stri(source);
      source->value.striValue = NULL;
    } else {
      new_size = take_stri(source)->size;
      if (stri_dest->size == new_size) {
        if (stri_dest != take_stri(source)) {
          /* It is possible that dest == source holds. The   */
          /* behavior of memcpy() is undefined if source and */
          /* destination areas overlap (or are identical).   */
          /* Therefore a check for this case is necessary.   */
          memcpy(stri_dest->mem, take_stri(source)->mem,
                 new_size * sizeof(strElemType));
        } /* if */
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(stri_dest, new_size))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          FREE_STRI(take_stri(dest), take_stri(dest)->size);
          dest->value.striValue = stri_dest;
          stri_dest->size = new_size;
        } /* if */
        memcpy(stri_dest->mem, take_stri(source)->mem,
               new_size * sizeof(strElemType));
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType str_create (listType arguments)

  {
    objectType dest;
    objectType source;
    memSizeType new_size;
    striType new_str;

  /* str_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_stri(source);
    logFunction(printf("str_create(*, \"%s\")\n",
                       striAsUnquotedCStri(take_stri(source))););
    SET_CATEGORY_OF_OBJ(dest, STRIOBJECT);
    if (TEMP_OBJECT(source)) {
      dest->value.striValue = take_stri(source);
      source->value.striValue = NULL;
    } else {
/*    printf("str_create %d !!!\n", in_file.line); */
      new_size = take_stri(source)->size;
      if (unlikely(!ALLOC_STRI_SIZE_OK(new_str, new_size))) {
        dest->value.striValue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      dest->value.striValue = new_str;
      new_str->size = new_size;
      memcpy(new_str->mem, take_stri(source)->mem,
             new_size * sizeof(strElemType));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_create */



/**
 *  Free the memory referred by 'old_string/arg_1'.
 *  After str_destr is left 'old_string/arg_1' is NULL.
 *  The memory where 'old_string/arg_1' is stored can be
 *  freed afterwards.
 */
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
 *  @exception INDEX_ERROR If 'position' is negative or zero, or
 *             a character beyond 'destination' would be overwritten
 *             ('position' > length('destination') holds).
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
      return raise_exception(SYS_IDX_EXCEPTION);
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
 *  For-loop which loops over the characters of a 'string'.
 */
objectType str_for (listType arguments)

  {
    objectType for_variable;
    striType stri;
    objectType statement;
    memSizeType pos;

  /* str_for */
    for_variable = arg_2(arguments);
    isit_char(for_variable);
    is_variable(for_variable);
    isit_stri(arg_4(arguments));
    stri = take_stri(arg_4(arguments));
    statement = arg_6(arguments);
    for (pos = 0; pos < stri->size && !fail_flag; pos++) {
      for_variable->value.charValue = stri->mem[pos];
      evaluate(statement);
    } /* for */
    return SYS_EMPTY_OBJECT;
  } /* str_for */



/**
 *  For-loop which loops over the keys (indices) of a 'string'.
 */
objectType str_for_key (listType arguments)

  {
    objectType key_variable;
    striType stri;
    objectType statement;
    memSizeType pos;

  /* str_for_key */
    key_variable = arg_3(arguments);
    is_variable(key_variable);
    isit_int(key_variable);
    isit_stri(arg_5(arguments));
    stri = take_stri(arg_5(arguments));
    statement = arg_7(arguments);
    for (pos = 0; pos < stri->size && !fail_flag; pos++) {
      key_variable->value.intValue = (intType) (pos + 1);
      evaluate(statement);
    } /* for */
    return SYS_EMPTY_OBJECT;
  } /* str_for_key */



/**
 *  For-loop which loops over characters and keys (indices) of a 'string'.
 */
objectType str_for_var_key (listType arguments)

  {
    objectType for_variable;
    objectType key_variable;
    striType stri;
    objectType statement;
    memSizeType pos;

  /* str_for_var_key */
    for_variable = arg_2(arguments);
    isit_char(for_variable);
    is_variable(for_variable);
    key_variable = arg_4(arguments);
    is_variable(key_variable);
    isit_int(key_variable);
    isit_stri(arg_6(arguments));
    stri = take_stri(arg_6(arguments));
    statement = arg_8(arguments);
    for (pos = 0; pos < stri->size && !fail_flag; pos++) {
      for_variable->value.charValue = stri->mem[pos];
      key_variable->value.intValue = (intType) (pos + 1);
      evaluate(statement);
    } /* for */
    return SYS_EMPTY_OBJECT;
  } /* str_for_var_key */



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
      if (memcmp_strelem(stri1->mem, stri2->mem, stri2->size) >= 0) {
        result = SYS_TRUE_OBJECT;
      } else {
        result = SYS_FALSE_OBJECT;
      } /* if */
    } else {
      if (memcmp_strelem(stri1->mem, stri2->mem, stri1->size) > 0) {
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
      if (memcmp_strelem(stri1->mem, stri2->mem, stri2->size) >= 0) {
        result = SYS_TRUE_OBJECT;
      } else {
        result = SYS_FALSE_OBJECT;
      } /* if */
    } else {
      if (memcmp_strelem(stri1->mem, stri2->mem, stri1->size) > 0) {
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
 *  @exception INDEX_ERROR The stop position is negative.
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
        COUNT_SHRINK_STRI(striSize, result_size);
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
    } else if (unlikely(stop < 0)) {
      return raise_exception(SYS_IDX_EXCEPTION);
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memSizeType) 0))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      /* Note that the size of the allocated memory is smaller, */
      /* than the struct. But this is okay, because the element */
      /* 'mem' respectively 'mem1' is not used. */
      result->size = 0;
    } /* if */
    return bld_stri_temp(result);
  } /* str_head */



/**
 *  Get a character, identified by an index, from a 'string'.
 *  The first character has the index 1.
 *  @return the character specified with the index.
 *  @exception INDEX_ERROR If the index is less than 1 or
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
      return raise_exception(SYS_IDX_EXCEPTION);
    } else {
      return bld_char_temp((charType) stri->mem[position - 1]);
    } /* if */
  } /* str_idx */



/**
 *  Search string 'searched' in 'mainStri' at or after 'fromIndex'.
 *  The search starts at 'fromIndex' and proceeds to the right.
 *  The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 if 'mainStri'
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
      if (memcmp_strelem(stri1->mem, stri2->mem, stri1->size) <= 0) {
        result = SYS_TRUE_OBJECT;
      } else {
        result = SYS_FALSE_OBJECT;
      } /* if */
    } else {
      if (memcmp_strelem(stri1->mem, stri2->mem, stri2->size) < 0) {
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
 *  The mapping is independent from the locale. Individual
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
      if (memcmp_strelem(stri1->mem, stri2->mem, stri1->size) <= 0) {
        result = SYS_TRUE_OBJECT;
      } else {
        result = SYS_FALSE_OBJECT;
      } /* if */
    } else {
      if (memcmp_strelem(stri1->mem, stri2->mem, stri2->size) < 0) {
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



/**
 *  String multiplication.
 *  The string 'stri' is concatenated to itself such that in total
 *  'factor' strings are concatenated.
 *   "LA" mult 3     returns "LALALA"
 *   "WORD" mult 0   returns ""
 *  @return the result of the string multiplication.
 *  @exception RANGE_ERROR If the factor is negative.
 */
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
 *  If the string is found the position of its first character
 *  is the result. The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 if 'mainStri'
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
 *  @exception INDEX_ERROR If 'position' is negative or zero, or
 *             if 'destination' is smaller than 'source', or
 *             characters beyond 'destination' would be overwritten
 *             ('position' + length('source') > succ(length('destination'))
 *             holds).
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
      return raise_exception(SYS_IDX_EXCEPTION);
    } else {
      /* It is possible that destStri and sourceStri overlap. */
      /* E.g. for the expression: stri @:= [idx] stri;        */
      /* The behavior of memcpy() is undefined if source      */
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
      COUNT_GROW_STRI(str_to->size, new_size);
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
 *  @exception INDEX_ERROR The start position is negative or zero, or
 *                         the stop position is less than pred(start).
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
    if (unlikely(start < 1)) {
      return raise_exception(SYS_IDX_EXCEPTION);
    } else if (stop >= start && (uintType) start <= striSize) {
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
      /* if using the -Ozacegilt optimisation option in the     */
      /* large memory model (-AL). Note that the order of the   */
      /* two statements make no difference to the logic of the  */
      /* program.                                               */
      memcpy(result->mem, &stri->mem[start - 1],
             result_size * sizeof(strElemType));
      result->size = result_size;
    } else if (unlikely(stop < start - 1)) {
      return raise_exception(SYS_IDX_EXCEPTION);
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memSizeType) 0))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      /* Note that the size of the allocated memory is smaller, */
      /* than the struct. But this is okay, because the element */
      /* 'mem' respectively 'mem1' is not used. */
      result->size = 0;
    } /* if */
    return bld_stri_temp(result);
  } /* str_range */



/**
 *  Search char 'searched' in 'mainStri' at or before 'fromIndex'.
 *  The search starts at 'fromIndex' and proceeds to the left.
 *  The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 if 'mainStri'
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
 *  @return the position of 'searched' or 0 if 'mainStri'
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
 *  @return the position of 'searched' or 0 if 'mainStri'
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
 *  If the string is found the position of its first character
 *  is the result. The first character in a string has the position 1.
 *  @return the position of 'searched' or 0 if 'mainStri'
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
      COUNT_SHRINK_STRI(striSize, result_size);
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
 *  @exception INDEX_ERROR The start position is negative or zero, or
 *                         the length is negative.
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
    if (unlikely(start < 1 || length < 0)) {
      return raise_exception(SYS_IDX_EXCEPTION);
    } /* if */
    striSize = stri->size;
    if (length != 0 && (uintType) start <= striSize) {
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
      /* Note that the size of the allocated memory is smaller, */
      /* than the struct. But this is okay, because the element */
      /* 'mem' respectively 'mem1' is not used. */
      result->size = 0;
    } /* if */
    return bld_stri_temp(result);
  } /* str_substr */



/**
 *  Get a substring beginning at a start position.
 *  The first character in a 'string' has the position 1.
 *  @return the substring beginning at the start position.
 *  @exception INDEX_ERROR The start position is negative or zero.
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
    if (unlikely(start < 1)) {
      return raise_exception(SYS_IDX_EXCEPTION);
    } else if ((uintType) start <= striSize && striSize >= 1) {
      result_size = striSize - (memSizeType) start + 1;
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      /* Reversing the order of the following two statements    */
      /* causes an "Internal Compiler Error" with MSC 6.0       */
      /* if using the -Ozacegilt optimisation option in the     */
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
      /* Note that the size of the allocated memory is smaller, */
      /* than the struct. But this is okay, because the element */
      /* 'mem' respectively 'mem1' is not used. */
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
 *  The mapping is independent from the locale. Individual
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
 *  @exception RANGE_ERROR If characters beyond '\255;' are present or
 *                         if 'utf8' is not encoded with UTF-8.
 */
objectType str_utf8tostri (listType arguments)

  { /* str_utf8tostri */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        strUtf8ToStri(take_stri(arg_1(arguments))));
  } /* str_utf8tostri */



/**
 *  Get 'string' value of the object referenced by 'aReference/arg_1'.
 *  @return the 'string' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference/arg_1' is NIL or
 *             category(aReference) <> STRIOBJECT holds.
 */
objectType str_value (listType arguments)

  {
    objectType aReference;
    striType stri;
    striType result;

  /* str_value */
    isit_reference(arg_1(arguments));
    aReference = take_reference(arg_1(arguments));
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != STRIOBJECT ||
                 take_stri(aReference) == NULL)) {
      logError(printf("str_value(");
               trace1(aReference);
               printf("): Category is not STRIOBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      stri = take_stri(aReference);
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
