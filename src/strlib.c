/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2007  Thomas Mertes                        */
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



#ifdef ANSI_C

static INLINE int strelem_memcmp (const strelemtype *mem1,
    const strelemtype *mem2, size_t number)
#else

static INLINE int strelem_memcmp (mem1, mem2, number)
strelemtype *mem1;
strelemtype *mem2;
size_t number;
#endif

  { /* strelem_memcmp */
    for (; number > 0; mem1++, mem2++, number--) {
      if (*mem1 != *mem2) {
        return *mem1 < *mem2 ? -1 : 1;
      } /* if */
    } /* for */
    return 0;
  } /* strelem_memcmp */



#ifdef ANSI_C

static INLINE const strelemtype *search_strelem (const strelemtype *mem,
    const strelemtype ch, size_t number)
#else

static INLINE strelemtype *search_strelem (mem, ch, number)
strelemtype *mem;
strelemtype ch;
size_t number;
#endif

  { /* search_strelem */
    for (; number > 0; mem++, number--) {
      if (*mem == ch) {
        return mem;
      } /* if */
    } /* for */
    return NULL;
  } /* search_strelem */



#ifdef ANSI_C

static arraytype add_stri_to_array (const strelemtype *stri_elems, memsizetype length,
    arraytype work_array, inttype *used_max_position)
#else

static arraytype add_stri_to_array (stri_elems, length,
    work_array, used_max_position)
strelemtype *stri_elems;
memsizetype length;
arraytype work_array;
inttype *used_max_position;
#endif

  {
    stritype new_stri;
    arraytype resized_work_array;
    memsizetype position;

  /* add_stri_to_array */
    if (ALLOC_STRI_SIZE_OK(new_stri, length)) {
      new_stri->size = length;
      memcpy(new_stri->mem, stri_elems,
          length * sizeof(strelemtype));
      if (*used_max_position >= work_array->max_position) {
        if (work_array->max_position >= MAX_MEM_INDEX - 256) {
          resized_work_array = NULL;
        } else {
          resized_work_array = REALLOC_ARRAY(work_array,
              (uinttype) work_array->max_position, (uinttype) work_array->max_position + 256);
        } /* if */
        if (resized_work_array == NULL) {
          FREE_STRI(new_stri, new_stri->size);
          new_stri = NULL;
        } else {
          work_array = resized_work_array;
          COUNT3_ARRAY((uinttype) work_array->max_position, (uinttype) work_array->max_position + 256);
          work_array->max_position += 256;
        } /* if */
      } /* if */
    } /* if */
    if (new_stri != NULL) {
      work_array->arr[*used_max_position].type_of = take_type(SYS_STRI_TYPE);
      work_array->arr[*used_max_position].descriptor.property = NULL;
      work_array->arr[*used_max_position].value.strivalue = new_stri;
      INIT_CATEGORY_OF_VAR(&work_array->arr[*used_max_position], STRIOBJECT);
      (*used_max_position)++;
    } else {
      for (position = 0; position < (uinttype) *used_max_position; position++) {
        FREE_STRI(work_array->arr[position].value.strivalue,
            work_array->arr[position].value.strivalue->size);
      } /* for */
      FREE_ARRAY(work_array, (uinttype) work_array->max_position);
      work_array = NULL;
    } /* if */
    return work_array;
  } /* add_stri_to_array */



#ifdef ANSI_C

static arraytype strChSplit (const const_stritype main_stri, const chartype delimiter)
#else

static arraytype strChSplit (main_stri, delimiter)
stritype main_stri;
chartype delimiter;
#endif

  {
    inttype used_max_position;
    const strelemtype *search_start;
    const strelemtype *search_end;
    const strelemtype *found_pos;
    memsizetype pos;
    arraytype resized_result_array;
    arraytype result_array;

  /* strChSplit */
    if (ALLOC_ARRAY(result_array, 256)) {
      result_array->min_position = 1;
      result_array->max_position = 256;
      used_max_position = 0;
      search_start = main_stri->mem;
      search_end = &main_stri->mem[main_stri->size];
      while ((found_pos = search_strelem(search_start,
          delimiter, (memsizetype) (search_end - search_start))) != NULL &&
          result_array != NULL) {
        result_array = add_stri_to_array(search_start,
            (memsizetype) (found_pos - search_start), result_array,
            &used_max_position);
        search_start = found_pos + 1;
      } /* while */
      if (result_array != NULL) {
        result_array = add_stri_to_array(search_start,
            (memsizetype) (search_end - search_start), result_array,
            &used_max_position);
        if (result_array != NULL) {
          resized_result_array = REALLOC_ARRAY(result_array,
              (uinttype) result_array->max_position, (uinttype) used_max_position);
          if (resized_result_array == NULL) {
            for (pos = 0; pos < (uinttype) used_max_position; pos++) {
              FREE_STRI(result_array->arr[pos].value.strivalue,
                  result_array->arr[pos].value.strivalue->size);
            } /* for */
            FREE_ARRAY(result_array, (uinttype) result_array->max_position);
            result_array = NULL;
          } else {
            result_array = resized_result_array;
            COUNT3_ARRAY((uinttype) result_array->max_position, (uinttype) used_max_position);
            result_array->max_position = used_max_position;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (result_array == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result_array;
  } /* strChSplit */



#ifdef ANSI_C

static arraytype strSplit (const const_stritype main_stri,
    const const_stritype delimiter)
#else

static arraytype strSplit (main_stri, delimiter)
stritype main_stri;
stritype delimiter;
#endif

  {
    memsizetype delimiter_size;
    const strelemtype *delimiter_mem;
    strelemtype ch_1;
    inttype used_max_position;
    const strelemtype *search_start;
    const strelemtype *segment_start;
    const strelemtype *search_end;
    const strelemtype *found_pos;
    memsizetype pos;
    arraytype resized_result_array;
    arraytype result_array;

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
            ch_1, (memsizetype) (search_end - search_start))) != NULL &&
            result_array != NULL) {
          if (memcmp(found_pos, delimiter_mem,
              delimiter_size * sizeof(strelemtype)) == 0) {
            result_array = add_stri_to_array(segment_start,
                (memsizetype) (found_pos - segment_start), result_array,
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
            (memsizetype) (&main_stri->mem[main_stri->size] - segment_start), result_array,
            &used_max_position);
        if (result_array != NULL) {
          resized_result_array = REALLOC_ARRAY(result_array,
              (uinttype) result_array->max_position, (uinttype) used_max_position);
          if (resized_result_array == NULL) {
            for (pos = 0; pos < (uinttype) used_max_position; pos++) {
              FREE_STRI(result_array->arr[pos].value.strivalue,
                  result_array->arr[pos].value.strivalue->size);
            } /* for */
            FREE_ARRAY(result_array, (uinttype) result_array->max_position);
            result_array = NULL;
          } else {
            result_array = resized_result_array;
            COUNT3_ARRAY((uinttype) result_array->max_position, (uinttype) used_max_position);
            result_array->max_position = used_max_position;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (result_array == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result_array;
  } /* strSplit */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

arraytype str1Split (stritype main_stri, stritype delimiter)
#else

arraytype str1Split (main_stri, delimiter)
stritype main_stri;
stritype delimiter;
#endif

  {
    arraytype result;

  /* str1Split */
    if (delimiter->size == 1) {
      result = strChSplit(main_stri, delimiter->mem[0]);
    } else {
      result = strSplit(main_stri, delimiter);
    } /* if */
    return result;
  } /* str1Split */
#endif



#ifdef ANSI_C

objecttype str_append (listtype arguments)
#else

objecttype str_append (arguments)
listtype arguments;
#endif

  {
    objecttype str_variable;
    stritype str_to;
    stritype str_from;
    stritype new_str;
    memsizetype new_size;
    memsizetype str_to_size;

  /* str_append */
    str_variable = arg_1(arguments);
    isit_stri(str_variable);
    is_variable(str_variable);
    str_to = take_stri(str_variable);
    isit_stri(arg_3(arguments));
    str_from = take_stri(arg_3(arguments));
    if (str_from->size != 0) {
      str_to_size = str_to->size;
      if (str_to_size > MAX_STRI_LEN - str_from->size) {
        /* number of bytes does not fit into memsizetype */
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_size = str_to_size + str_from->size;
        GROW_STRI(new_str, str_to, str_to_size, new_size);
        if (new_str == NULL) {
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
              str_from->size * sizeof(strelemtype));
          new_str->size = new_size;
          str_variable->value.strivalue = new_str;
        } /* if */
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_append */



#ifdef ANSI_C

objecttype str_cat (listtype arguments)
#else

objecttype str_cat (arguments)
listtype arguments;
#endif

  {
    stritype stri1;
    stritype stri2;
    memsizetype stri1_size;
    memsizetype result_size;
    stritype result;

  /* str_cat */
    isit_stri(arg_1(arguments));
    isit_stri(arg_3(arguments));
    stri1 = take_stri(arg_1(arguments));
    stri2 = take_stri(arg_3(arguments));
    stri1_size = stri1->size;
    if (stri1_size > MAX_STRI_LEN - stri2->size) {
      /* number of bytes does not fit into memsizetype */
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result_size = stri1_size + stri2->size;
      if (TEMP_OBJECT(arg_1(arguments))) {
        GROW_STRI(result, stri1, stri1_size, result_size);
        if (result == NULL) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          COUNT3_STRI(stri1_size, result_size);
          result->size = result_size;
          memcpy(&result->mem[stri1_size], stri2->mem,
              stri2->size * sizeof(strelemtype));
          arg_1(arguments)->value.strivalue = NULL;
          return bld_stri_temp(result);
        } /* if */
      } else {
        if (!ALLOC_STRI_SIZE_OK(result, result_size)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          result->size = result_size;
          memcpy(result->mem, stri1->mem,
              stri1_size * sizeof(strelemtype));
          memcpy(&result->mem[stri1_size], stri2->mem,
              stri2->size * sizeof(strelemtype));
          return bld_stri_temp(result);
        } /* if */
      } /* if */
    } /* if */
  } /* str_cat */



#ifdef ANSI_C

objecttype str_chipos (listtype arguments)
#else

objecttype str_chipos (arguments)
listtype arguments;
#endif

  { /* str_chipos */
    isit_stri(arg_1(arguments));
    isit_char(arg_2(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        strChIPos(take_stri(arg_1(arguments)), take_char(arg_2(arguments)),
                  take_int(arg_3(arguments))));
  } /* str_chipos */



#ifdef ANSI_C

objecttype str_chpos (listtype arguments)
#else

objecttype str_chpos (arguments)
listtype arguments;
#endif

  { /* str_chpos */
    isit_stri(arg_1(arguments));
    isit_char(arg_2(arguments));
    return bld_int_temp(
        strChPos(take_stri(arg_1(arguments)), take_char(arg_2(arguments))));
  } /* str_chpos */



#ifdef ANSI_C

objecttype str_chsplit (listtype arguments)
#else

objecttype str_chsplit (arguments)
listtype arguments;
#endif

  { /* str_chsplit */
    isit_stri(arg_1(arguments));
    isit_char(arg_2(arguments));
    return bld_array_temp(
        strChSplit(take_stri(arg_1(arguments)), take_char(arg_2(arguments))));
  } /* str_chsplit */



#ifdef ANSI_C

objecttype str_clit (listtype arguments)
#else

objecttype str_clit (arguments)
listtype arguments;
#endif

  { /* str_clit */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        strCLit(take_stri(arg_1(arguments))));
  } /* str_clit */



#ifdef ANSI_C

objecttype str_cmp (listtype arguments)
#else

objecttype str_cmp (arguments)
listtype arguments;
#endif

  {
    stritype stri1;
    stritype stri2;
    inttype result;

  /* str_cmp */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    stri1 = take_stri(arg_1(arguments));
    stri2 = take_stri(arg_2(arguments));
    if (stri1->size < stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri1->size) <= 0) {
        result = -1;
      } else {
        result = 1;
      } /* if */
    } else if (stri1->size > stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri2->size) >= 0) {
        result = 1;
      } else {
        result = -1;
      } /* if */
    } else {
      result = strelem_memcmp(stri1->mem, stri2->mem, stri1->size);
      if (result > 0) {
        result = 1;
      } else if (result < 0) {
        result = -1;
      } /* if */
    } /* if */
    return bld_int_temp(result);
  } /* str_cmp */



#ifdef ANSI_C

objecttype str_cpy (listtype arguments)
#else

objecttype str_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype str_to;
    objecttype str_from;
    memsizetype new_size;
    stritype stri_dest;

  /* str_cpy */
    str_to = arg_1(arguments);
    str_from = arg_3(arguments);
    isit_stri(str_to);
    isit_stri(str_from);
    is_variable(str_to);
    stri_dest = take_stri(str_to);
    if (TEMP_OBJECT(str_from)) {
      FREE_STRI(stri_dest, stri_dest->size);
      str_to->value.strivalue = take_stri(str_from);
      str_from->value.strivalue = NULL;
    } else {
      new_size = take_stri(str_from)->size;
      if (stri_dest->size == new_size) {
        if (stri_dest != take_stri(str_from)) {
          /* It is possible that str_to == str_from holds. The */
          /* behavior of memcpy() is undefined when source and */
          /* destination areas overlap (or are identical).     */
          /* Therefore a check for this case is necessary.     */
          memcpy(stri_dest->mem, take_stri(str_from)->mem,
              new_size * sizeof(strelemtype));
        } /* if */
      } else {
        if (!ALLOC_STRI_SIZE_OK(stri_dest, new_size)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          FREE_STRI(take_stri(str_to), take_stri(str_to)->size);
          str_to->value.strivalue = stri_dest;
          stri_dest->size = new_size;
        } /* if */
        memcpy(stri_dest->mem, take_stri(str_from)->mem,
            new_size * sizeof(strelemtype));
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_cpy */



#ifdef ANSI_C

objecttype str_create (listtype arguments)
#else

objecttype str_create (arguments)
listtype arguments;
#endif

  {
    objecttype str_to;
    objecttype str_from;
    memsizetype new_size;
    stritype new_str;

  /* str_create */
    str_to = arg_1(arguments);
    str_from = arg_3(arguments);
    isit_stri(str_from);
    SET_CATEGORY_OF_OBJ(str_to, STRIOBJECT);
    if (TEMP_OBJECT(str_from)) {
      str_to->value.strivalue = take_stri(str_from);
      str_from->value.strivalue = NULL;
    } else {
/*    printf("str_create %d !!!\n", in_file.line); */
      new_size = take_stri(str_from)->size;
      if (!ALLOC_STRI_SIZE_OK(new_str, new_size)) {
        str_to->value.strivalue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      str_to->value.strivalue = new_str;
      new_str->size = new_size;
      memcpy(new_str->mem, take_stri(str_from)->mem,
          new_size * sizeof(strelemtype));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_create */



#ifdef ANSI_C

objecttype str_destr (listtype arguments)
#else

objecttype str_destr (arguments)
listtype arguments;
#endif

  {
    stritype old_string;

  /* str_destr */
    isit_stri(arg_1(arguments));
    old_string = take_stri(arg_1(arguments));
    if (old_string != NULL) {
      FREE_STRI(old_string, old_string->size);
      arg_1(arguments)->value.strivalue = NULL;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_destr */



#ifdef ANSI_C

objecttype str_elemcpy (listtype arguments)
#else

objecttype str_elemcpy (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    inttype position;

  /* str_elemcpy */
    isit_stri(arg_1(arguments));
    isit_int(arg_4(arguments));
    isit_char(arg_6(arguments));
    is_variable(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    position = take_int(arg_4(arguments));
    if (position >= 1 && (uinttype) position <= stri->size) {
      stri->mem[position - 1] = (strelemtype) take_char(arg_6(arguments));
    } else {
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_elemcpy */



#ifdef ANSI_C

objecttype str_eq (listtype arguments)
#else

objecttype str_eq (arguments)
listtype arguments;
#endif

  {
    stritype stri1;
    stritype stri2;

  /* str_eq */
    isit_stri(arg_1(arguments));
    isit_stri(arg_3(arguments));
    stri1 = take_stri(arg_1(arguments));
    stri2 = take_stri(arg_3(arguments));
    if (stri1->size == stri2->size && memcmp(stri1->mem, stri2->mem,
        stri1->size * sizeof(strelemtype)) == 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* str_eq */



#ifdef ANSI_C

objecttype str_ge (listtype arguments)
#else

objecttype str_ge (arguments)
listtype arguments;
#endif

  {
    stritype stri1;
    stritype stri2;
    objecttype result;

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



#ifdef ANSI_C

objecttype str_gt (listtype arguments)
#else

objecttype str_gt (arguments)
listtype arguments;
#endif

  {
    stritype stri1;
    stritype stri2;
    objecttype result;

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



#ifdef ANSI_C

objecttype str_hashcode (listtype arguments)
#else

objecttype str_hashcode (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    inttype result;

  /* str_hashcode */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    if (stri->size == 0) {
      result = 0;
    } else {
      result = (inttype) (stri->mem[0] << 5 ^ stri->size << 3 ^ stri->mem[stri->size - 1]);
    } /* if */
    return bld_int_temp(result);
  } /* str_hashcode */



#ifdef ANSI_C

objecttype str_head (listtype arguments)
#else

objecttype str_head (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    inttype stop;
    memsizetype length;
    memsizetype result_size;
    stritype result;

  /* str_head */
    isit_stri(arg_1(arguments));
    isit_int(arg_4(arguments));
    stri = take_stri(arg_1(arguments));
    stop = take_int(arg_4(arguments));
    length = stri->size;
    if (stop >= 1 && length >= 1) {
      if (length <= (uinttype) stop) {
        result_size = length;
      } else {
        result_size = (memsizetype) stop;
      } /* if */
      if (TEMP_OBJECT(arg_1(arguments))) {
        SHRINK_STRI(result, stri, length, result_size);
        if (result == NULL) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        COUNT3_STRI(length, result_size);
        result->size = result_size;
        arg_1(arguments)->value.strivalue = NULL;
      } else {
        if (!ALLOC_STRI_SIZE_OK(result, result_size)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->size = result_size;
        memcpy(result->mem, stri->mem,
            result_size * sizeof(strelemtype));
      } /* if */
    } else {
      if (!ALLOC_STRI_SIZE_OK(result, (memsizetype) 0)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->size = 0;
    } /* if */
    return bld_stri_temp(result);
  } /* str_head */



#ifdef ANSI_C

objecttype str_idx (listtype arguments)
#else

objecttype str_idx (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    inttype position;

  /* str_idx */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    stri = take_stri(arg_1(arguments));
    position = take_int(arg_3(arguments));
    if (position >= 1 && (uinttype) position <= stri->size) {
      return bld_char_temp((chartype) stri->mem[position - 1]);
    } else {
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
  } /* str_idx */



#ifdef ANSI_C

objecttype str_ipos (listtype arguments)
#else

objecttype str_ipos (arguments)
listtype arguments;
#endif

  { /* str_ipos */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        strIPos(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)),
                take_int(arg_3(arguments))));
  } /* str_ipos */



#ifdef ANSI_C

objecttype str_le (listtype arguments)
#else

objecttype str_le (arguments)
listtype arguments;
#endif

  {
    stritype stri1;
    stritype stri2;
    objecttype result;

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



#ifdef ANSI_C

objecttype str_lit (listtype arguments)
#else

objecttype str_lit (arguments)
listtype arguments;
#endif

  { /* str_lit */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        strLit(take_stri(arg_1(arguments))));
  } /* str_lit */



#ifdef ANSI_C

objecttype str_lng (listtype arguments)
#else

objecttype str_lng (arguments)
listtype arguments;
#endif

  { /* str_lng */
    isit_stri(arg_1(arguments));
    return bld_int_temp((inttype) take_stri(arg_1(arguments))->size);
  } /* str_lng */



#ifdef ANSI_C

objecttype str_low (listtype arguments)
#else

objecttype str_low (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    memsizetype length;
    memsizetype pos;
    stritype result;

  /* str_low */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    length = stri->size;
    if (!ALLOC_STRI_SIZE_OK(result, length)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->size = length;
      for (pos = 0; pos < length; pos++) {
        if (stri->mem[pos] >= (strelemtype) 'A' && stri->mem[pos] <= (strelemtype) 'Z') {
          result->mem[pos] = stri->mem[pos] - (strelemtype) 'A' + (strelemtype) 'a';
        } else {
          result->mem[pos] = stri->mem[pos];
        } /* if */
      } /* for */
      return bld_stri_temp(result);
    } /* if */
  } /* str_low */



#ifdef ANSI_C

objecttype str_lpad (listtype arguments)
#else

objecttype str_lpad (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    inttype pad_size;
    memsizetype length;
    stritype result;

  /* str_lpad */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    stri = take_stri(arg_1(arguments));
    pad_size = take_int(arg_3(arguments));
    length = stri->size;
    if (pad_size > 0 && (uinttype) pad_size > length) {
      if ((uinttype) pad_size > MAX_STRI_LEN ||
          !ALLOC_STRI_SIZE_OK(result, (memsizetype) pad_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = (memsizetype) pad_size;
        {
          strelemtype *elem = result->mem;
          memsizetype len = (memsizetype) pad_size - length;

          while (len--) {
            *elem++ = (strelemtype) ' ';
          } /* while */
        }
        memcpy(&result->mem[(memsizetype) pad_size - length], stri->mem,
            length * sizeof(strelemtype));
      } /* if */
    } else {
      if (TEMP_OBJECT(arg_1(arguments))) {
        result = stri;
        arg_1(arguments)->value.strivalue = NULL;
      } else {
        if (!ALLOC_STRI_SIZE_OK(result, length)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->size = length;
        memcpy(result->mem, stri->mem,
            length * sizeof(strelemtype));
      } /* if */
    } /* if */
    return bld_stri_temp(result);
  } /* str_lpad */



#ifdef ANSI_C

objecttype str_lpad0 (listtype arguments)
#else

objecttype str_lpad0 (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    inttype pad_size;
    memsizetype length;
    strelemtype *sourceElem;
    strelemtype *destElem;
    memsizetype len;
    stritype result;

  /* str_lpad0 */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    stri = take_stri(arg_1(arguments));
    pad_size = take_int(arg_3(arguments));
    length = stri->size;
    if (pad_size > 0 && (uinttype) pad_size > length) {
      if ((uinttype) pad_size > MAX_STRI_LEN ||
          !ALLOC_STRI_SIZE_OK(result, (memsizetype) pad_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = (memsizetype) pad_size;
        sourceElem = stri->mem;
        destElem = result->mem;
        len = (memsizetype) pad_size - length;
        if (length != 0 && (sourceElem[0] == '-' || sourceElem[0] == '+')) {
          *destElem++ = sourceElem[0];
          sourceElem++;
          length--;
        } /* if */
        while (len--) {
          *destElem++ = (strelemtype) '0';
        } /* while */
        memcpy(destElem, sourceElem, length * sizeof(strelemtype));
      } /* if */
    } else {
      if (TEMP_OBJECT(arg_1(arguments))) {
        result = stri;
        arg_1(arguments)->value.strivalue = NULL;
      } else {
        if (!ALLOC_STRI_SIZE_OK(result, length)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->size = length;
        memcpy(result->mem, stri->mem,
            length * sizeof(strelemtype));
      } /* if */
    } /* if */
    return bld_stri_temp(result);
  } /* str_lpad0 */



#ifdef ANSI_C

objecttype str_lt (listtype arguments)
#else

objecttype str_lt (arguments)
listtype arguments;
#endif

  {
    stritype stri1;
    stritype stri2;
    objecttype result;

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



#ifdef ANSI_C

objecttype str_ltrim (listtype arguments)
#else

objecttype str_ltrim (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    memsizetype start;
    memsizetype length;
    stritype result;

  /* str_ltrim */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    start = 0;
    length = stri->size;
    if (length >= 1) {
      while (start < length && stri->mem[start] <= ' ') {
        start++;
      } /* while */
      length -= start;
    } /* if */
    if (!ALLOC_STRI_SIZE_OK(result, length)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->size = length;
      memcpy(result->mem, &stri->mem[start],
          length * sizeof(strelemtype));
      return bld_stri_temp(result);
    } /* if */
  } /* str_ltrim */



#ifdef ANSI_C

objecttype str_mult (listtype arguments)
#else

objecttype str_mult (arguments)
listtype arguments;
#endif

  { /* str_mult */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        strMult(take_stri(arg_1(arguments)), take_int(arg_3(arguments))));
  } /* str_mult */



#ifdef ANSI_C

objecttype str_ne (listtype arguments)
#else

objecttype str_ne (arguments)
listtype arguments;
#endif

  {
    stritype stri1;
    stritype stri2;

  /* str_ne */
    isit_stri(arg_1(arguments));
    isit_stri(arg_3(arguments));
    stri1 = take_stri(arg_1(arguments));
    stri2 = take_stri(arg_3(arguments));
    if (stri1->size != stri2->size || memcmp(stri1->mem, stri2->mem,
        stri1->size * sizeof(strelemtype)) != 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* str_ne */



#ifdef ANSI_C

objecttype str_pos (listtype arguments)
#else

objecttype str_pos (arguments)
listtype arguments;
#endif

  { /* str_pos */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_int_temp(
        strPos(take_stri(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* str_pos */



#ifdef ANSI_C

objecttype str_poscpy (listtype arguments)
#else

objecttype str_poscpy (arguments)
listtype arguments;
#endif

  {
    stritype destStri;
    stritype sourceStri;
    inttype position;

  /* str_poscpy */
    isit_stri(arg_1(arguments));
    isit_int(arg_4(arguments));
    isit_stri(arg_6(arguments));
    is_variable(arg_1(arguments));
    destStri = take_stri(arg_1(arguments));
    position = take_int(arg_4(arguments));
    sourceStri = take_stri(arg_6(arguments));
    if (position >= 1 && destStri->size >= sourceStri->size &&
        (uinttype) position <= destStri->size - sourceStri->size + 1) {
      memcpy(&destStri->mem[position - 1], sourceStri->mem,
          sourceStri->size * sizeof(strelemtype));
    } else {
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_poscpy */



#ifdef ANSI_C

objecttype str_push (listtype arguments)
#else

objecttype str_push (arguments)
listtype arguments;
#endif

  {
    objecttype str_variable;
    stritype str_to;
    chartype char_from;
    memsizetype new_size;

  /* str_push */
    str_variable = arg_1(arguments);
    isit_stri(str_variable);
    is_variable(str_variable);
    str_to = take_stri(str_variable);
    isit_char(arg_3(arguments));
    char_from = take_char(arg_3(arguments));
    new_size = str_to->size + 1;
    GROW_STRI(str_to, str_to, str_to->size, new_size);
    if (str_to == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      COUNT3_STRI(str_to->size, new_size);
      str_to->mem[str_to->size] = char_from;
      str_to->size = new_size;
      str_variable->value.strivalue = str_to;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* str_push */



#ifdef ANSI_C

objecttype str_range (listtype arguments)
#else

objecttype str_range (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    inttype start;
    inttype stop;
    memsizetype length;
    memsizetype result_size;
    stritype result;

  /* str_range */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_5(arguments));
    stri = take_stri(arg_1(arguments));
    start = take_int(arg_3(arguments));
    stop = take_int(arg_5(arguments));
    length = stri->size;
    if (start < 1) {
      start = 1;
    } /* if */
    if (stop >= 1 && stop >= start && (uinttype) start <= length &&
        length >= 1) {
      if ((uinttype) stop > length) {
        result_size = length - (memsizetype) start + 1;
      } else {
        result_size = (memsizetype) stop - (memsizetype) start + 1;
      } /* if */
      if (!ALLOC_STRI_SIZE_OK(result, result_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      /* Reversing the order of the following two statements    */
      /* causes an "Internal Compiler Error" with MSC 6.0       */
      /* when using the -Ozacegilt optimisation option in the   */
      /* large memory model (-AL). Note that the order of the   */
      /* two statements make no difference to the logic of the  */
      /* program.                                               */
      memcpy(result->mem, &stri->mem[start - 1],
          result_size * sizeof(strelemtype));
      result->size = result_size;
    } else {
      if (!ALLOC_STRI_SIZE_OK(result, (memsizetype) 0)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->size = 0;
    } /* if */
    return bld_stri_temp(result);
  } /* str_range */



#ifdef ANSI_C

objecttype str_rchipos (listtype arguments)
#else

objecttype str_rchipos (arguments)
listtype arguments;
#endif

  { /* str_rchipos */
    isit_stri(arg_1(arguments));
    isit_char(arg_2(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        strRChIPos(take_stri(arg_1(arguments)), take_char(arg_2(arguments)),
                   take_int(arg_3(arguments))));
  } /* str_rchipos */



#ifdef ANSI_C

objecttype str_rchpos (listtype arguments)
#else

objecttype str_rchpos (arguments)
listtype arguments;
#endif

  { /* str_rchpos */
    isit_stri(arg_1(arguments));
    isit_char(arg_2(arguments));
    return bld_int_temp(
        strRChPos(take_stri(arg_1(arguments)), take_char(arg_2(arguments))));
  } /* str_rchpos */



#ifdef ANSI_C

objecttype str_repl (listtype arguments)
#else

objecttype str_repl (arguments)
listtype arguments;
#endif

  { /* str_repl */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_stri(arg_3(arguments));
    return bld_stri_temp(
        strRepl(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)),
                take_stri(arg_3(arguments))));
  } /* str_repl */



#ifdef ANSI_C

objecttype str_ripos (listtype arguments)
#else

objecttype str_ripos (arguments)
listtype arguments;
#endif

  { /* str_ripos */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_int(arg_3(arguments));
    return bld_int_temp(
        strRIPos(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)),
                 take_int(arg_3(arguments))));
  } /* str_ripos */



#ifdef ANSI_C

objecttype str_rpad (listtype arguments)
#else

objecttype str_rpad (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    inttype pad_size;
    memsizetype length;
    stritype result;

  /* str_rpad */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    stri = take_stri(arg_1(arguments));
    pad_size = take_int(arg_3(arguments));
    length = stri->size;
    if (pad_size > 0 && (uinttype) pad_size > length) {
      if ((uinttype) pad_size > MAX_STRI_LEN ||
          !ALLOC_STRI_SIZE_OK(result, (memsizetype) pad_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = (memsizetype) pad_size;
        memcpy(result->mem, stri->mem, length * sizeof(strelemtype));
        {
          strelemtype *elem = &result->mem[length];
          memsizetype len = (memsizetype) pad_size - length;

          while (len--) {
           *elem++ = (strelemtype) ' ';
          } /* while */
        }
      } /* if */
    } else {
      if (TEMP_OBJECT(arg_1(arguments))) {
        result = stri;
        arg_1(arguments)->value.strivalue = NULL;
      } else {
        if (!ALLOC_STRI_SIZE_OK(result, length)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
        result->size = length;
        memcpy(result->mem, stri->mem,
            length * sizeof(strelemtype));
      } /* if */
    } /* if */
    return bld_stri_temp(result);
  } /* str_rpad */



#ifdef ANSI_C

objecttype str_rpos (listtype arguments)
#else

objecttype str_rpos (arguments)
listtype arguments;
#endif

  { /* str_rpos */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_int_temp(
        strRPos(take_stri(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* str_rpos */



#ifdef ANSI_C

objecttype str_rtrim (listtype arguments)
#else

objecttype str_rtrim (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    memsizetype length;
    stritype result;

  /* str_rtrim */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    length = stri->size;
    while (length > 0 && stri->mem[length - 1] <= ' ') {
      length--;
    } /* while */
    if (!ALLOC_STRI_SIZE_OK(result, length)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->size = length;
      memcpy(result->mem, stri->mem,
          length * sizeof(strelemtype));
      return bld_stri_temp(result);
    } /* if */
  } /* str_rtrim */



#ifdef ANSI_C

objecttype str_split (listtype arguments)
#else

objecttype str_split (arguments)
listtype arguments;
#endif

  { /* str_split */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_array_temp(
        strSplit(take_stri(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* str_split */



#ifdef ANSI_C

objecttype str_str (listtype arguments)
#else

objecttype str_str (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    stritype result;

  /* str_str */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    if (TEMP_OBJECT(arg_1(arguments))) {
      result = stri;
      arg_1(arguments)->value.strivalue = NULL;
      return bld_stri_temp(result);
    } else {
      if (!ALLOC_STRI_SIZE_OK(result, stri->size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = stri->size;
        memcpy(result->mem, stri->mem,
            stri->size * sizeof(strelemtype));
        return bld_stri_temp(result);
      } /* if */
    } /* if */
  } /* str_str */



#ifdef ANSI_C

objecttype str_substr (listtype arguments)
#else

objecttype str_substr (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    inttype start;
    inttype len;
    memsizetype length;
    memsizetype result_size;
    stritype result;

  /* str_substr */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_5(arguments));
    stri = take_stri(arg_1(arguments));
    start = take_int(arg_3(arguments));
    len = take_int(arg_5(arguments));
    length = stri->size;
    if (len >= 1 && start > 1 - len && (start < 1 || (uinttype) start <= length) &&
        length >= 1) {
      if (start < 1) {
        len += start - 1;
        start = 1;
      } /* if */
      if ((uinttype) start + (uinttype) len - 1 > length) {
        result_size = length - (memsizetype) start + 1;
      } else {
        result_size = (memsizetype) len;
      } /* if */
      if (!ALLOC_STRI_SIZE_OK(result, result_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      memcpy(result->mem, &stri->mem[start - 1],
          result_size * sizeof(strelemtype));
      result->size = result_size;
    } else {
      if (!ALLOC_STRI_SIZE_OK(result, (memsizetype) 0)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->size = 0;
    } /* if */
    return bld_stri_temp(result);
  } /* str_substr */



#ifdef ANSI_C

objecttype str_tail (listtype arguments)
#else

objecttype str_tail (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    inttype start;
    memsizetype length;
    memsizetype result_size;
    stritype result;

  /* str_tail */
    isit_stri(arg_1(arguments));
    isit_int(arg_3(arguments));
    stri = take_stri(arg_1(arguments));
    start = take_int(arg_3(arguments));
    length = stri->size;
    if (start < 1) {
      start = 1;
    } /* if */
    if ((uinttype) start <= length && length >= 1) {
      result_size = length - (memsizetype) start + 1;
      if (!ALLOC_STRI_SIZE_OK(result, result_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      /* Reversing the order of the following two statements    */
      /* causes an "Internal Compiler Error" with MSC 6.0       */
      /* when using the -Ozacegilt optimisation option in the   */
      /* large memory model (-AL). Note that the order of the   */
      /* two statements make no difference to the logic of the  */
      /* program.                                               */
      memcpy(result->mem, &stri->mem[start - 1],
          result_size * sizeof(strelemtype));
      result->size = result_size;
    } else {
      if (!ALLOC_STRI_SIZE_OK(result, (memsizetype) 0)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->size = 0;
    } /* if */
    return bld_stri_temp(result);
  } /* str_tail */



#ifdef ANSI_C

objecttype str_toutf8 (listtype arguments)
#else

objecttype str_toutf8 (arguments)
listtype arguments;
#endif

  { /* str_toutf8 */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        strToUtf8(take_stri(arg_1(arguments))));
  } /* str_toutf8 */



#ifdef ANSI_C

objecttype str_trim (listtype arguments)
#else

objecttype str_trim (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    memsizetype start;
    memsizetype length;
    stritype result;

  /* str_trim */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    start = 0;
    length = stri->size;
    if (length >= 1) {
      while (start < length && stri->mem[start] <= ' ') {
        start++;
      } /* while */
      while (length > start && stri->mem[length - 1] <= ' ') {
        length--;
      } /* while */
      length -= start;
    } /* if */
    if (!ALLOC_STRI_SIZE_OK(result, length)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->size = length;
      memcpy(result->mem, &stri->mem[start],
          length * sizeof(strelemtype));
      return bld_stri_temp(result);
    } /* if */
  } /* str_trim */



#ifdef ANSI_C

objecttype str_up (listtype arguments)
#else

objecttype str_up (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    memsizetype length;
    memsizetype pos;
    stritype result;

  /* str_up */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    length = stri->size;
    if (!ALLOC_STRI_SIZE_OK(result, length)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->size = length;
      for (pos = 0; pos < length; pos++) {
        if (stri->mem[pos] >= (strelemtype) 'a' && stri->mem[pos] <= (strelemtype) 'z') {
          result->mem[pos] = stri->mem[pos] - (strelemtype) 'a' + (strelemtype) 'A';
        } else {
          result->mem[pos] = stri->mem[pos];
        } /* if */
      } /* for */
      return bld_stri_temp(result);
    } /* if */
  } /* str_up */



#ifdef ANSI_C

objecttype str_utf8tostri (listtype arguments)
#else

objecttype str_utf8tostri (arguments)
listtype arguments;
#endif

  { /* str_utf8tostri */
    isit_stri(arg_1(arguments));
    return bld_stri_temp(
        strUtf8ToStri(take_stri(arg_1(arguments))));
  } /* str_utf8tostri */



#ifdef ANSI_C

objecttype str_value (listtype arguments)
#else

objecttype str_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;
    stritype stri;
    stritype result;

  /* str_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != STRIOBJECT ||
        take_stri(obj_arg) == NULL) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      stri = take_stri(obj_arg);
      if (!ALLOC_STRI_SIZE_OK(result, stri->size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = stri->size;
        memcpy(result->mem, stri->mem,
            result->size * sizeof(strelemtype));
        return bld_stri_temp(result);
      } /* if */
    } /* if */
  } /* str_value */
