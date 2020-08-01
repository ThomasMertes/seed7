/********************************************************************/
/*                                                                  */
/*  str_rtl.c     Primitive actions for the string type.            */
/*  Copyright (C) 1989 - 2008  Thomas Mertes                        */
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
/*  File: seed7/src/str_rtl.c                                       */
/*  Changes: 1991, 1992, 1993, 1994, 2005  Thomas Mertes            */
/*  Content: Primitive actions for the string type.                 */
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
#include "str_rtl.h"


static char *stri_escape_sequence[] = {
    "\\0\\",  "\\1\\",  "\\2\\",  "\\3\\",  "\\4\\",
    "\\5\\",  "\\6\\",  "\\a",    "\\b",    "\\t",
    "\\n",    "\\v",    "\\f",    "\\r",    "\\14\\",
    "\\15\\", "\\16\\", "\\17\\", "\\18\\", "\\19\\",
    "\\20\\", "\\21\\", "\\22\\", "\\23\\", "\\24\\",
    "\\25\\", "\\26\\", "\\e",    "\\28\\", "\\29\\",
    "\\30\\", "\\31\\"};

static char *cstri_escape_sequence[] = {
    "\\000", "\\001", "\\002", "\\003", "\\004",
    "\\005", "\\006", "\\007", "\\b",   "\\t",
    "\\n",   "\\013", "\\f",   "\\r",   "\\016",
    "\\017", "\\020", "\\021", "\\022", "\\023",
    "\\024", "\\025", "\\026", "\\027", "\\030",
    "\\031", "\\032", "\\033", "\\034", "\\035",
    "\\036", "\\037"};



#ifdef WIDE_CHAR_STRINGS
#ifdef ANSI_C

static INLINE int strelem_memcmp (const strelemtype *mem1,
    const strelemtype *mem2, SIZE_TYPE number)
#else

static INLINE int strelem_memcmp (mem1, mem2, number)
strelemtype *mem1;
strelemtype *mem2;
SIZE_TYPE number;
#endif

  { /* strelem_memcmp */
    for (; number > 0; mem1++, mem2++, number--) {
      if (*mem1 != *mem2) {
        return(*mem1 - *mem2);
      } /* if */
    } /* for */
    return(0);
  } /* strelem_memcmp */



#ifdef ANSI_C

static INLINE const strelemtype *search_strelem (const strelemtype *mem,
    strelemtype ch, SIZE_TYPE number)
#else

static INLINE strelemtype *search_strelem (mem, ch, number)
strelemtype *mem;
strelemtype ch;
SIZE_TYPE number;
#endif

  {
    const strelemtype *byond;

  /* search_strelem */
    for (byond = &mem[number]; mem != byond; mem++) {
      if (*mem == ch) {
        return(mem);
      } /* if */
    } /* for */
    return(NULL);
  } /* search_strelem */



#else

#define strelem_memcmp(mem1,mem2,len) memcmp(mem1, mem2, (len) * sizeof(strelemtype))
#define search_strelem (const strelemtype *) memchr

#endif



#ifdef ANSI_C

static INLINE const strelemtype *rsearch_strelem (const strelemtype *mem,
    strelemtype ch, SIZE_TYPE number)
#else

static INLINE strelemtype *rsearch_strelem (mem, ch, number)
strelemtype *mem;
strelemtype ch;
SIZE_TYPE number;
#endif

  { /* rsearch_strelem */
    for (; number > 0; mem--, number--) {
      if (*mem == ch) {
        return(mem);
      } /* if */
    } /* for */
    return(NULL);
  } /* rsearch_strelem */



#ifdef ANSI_C

static rtlArraytype add_stri_to_array (const strelemtype *stri_elems,
    memsizetype length, rtlArraytype work_array, memsizetype *used_max_position)
#else

static rtlArraytype add_stri_to_array (stri_elems, length,
    work_array, used_max_position)
strelemtype *stri_elems;
memsizetype length;
rtlArraytype work_array;
memsizetype *used_max_position;
#endif

  {
    stritype new_stri;
    rtlArraytype resized_work_array;
    memsizetype position;

  /* add_stri_to_array */
    if (ALLOC_STRI(new_stri, length)) {
      new_stri->size = length;
      memcpy(new_stri->mem, stri_elems,
          (SIZE_TYPE) length * sizeof(strelemtype));
      if (*used_max_position >= work_array->max_position) {
        resized_work_array = REALLOC_RTL_ARRAY(work_array,
            work_array->max_position, work_array->max_position + 256);
        if (resized_work_array == NULL) {
          FREE_STRI(new_stri, new_stri->size);
          new_stri = NULL;
        } else {
          work_array = resized_work_array;
          COUNT3_RTL_ARRAY(work_array->max_position, work_array->max_position + 256);
          work_array->max_position += 256;
        } /* if */
      } /* if */
    } /* if */
    if (new_stri != NULL) {
      work_array->arr[*used_max_position].value.strivalue = new_stri;
      (*used_max_position)++;
    } else {
      for (position = 0; position < *used_max_position; position++) {
        FREE_STRI(work_array->arr[position].value.strivalue,
            work_array->arr[position].value.strivalue->size);
      } /* for */
      FREE_RTL_ARRAY(work_array, work_array->max_position);
      work_array = NULL;
    } /* if */
    return(work_array);
  } /* add_stri_to_array */



#ifdef ANSI_C

void strAppend (stritype *const stri_to, const const_stritype stri_from)
#else

void strAppend (stri_to, stri_from)
stritype *stri_to;
stritype stri_from;
#endif

  {
    memsizetype new_size;
    stritype stri_dest;

  /* strAppend */
    stri_dest = *stri_to;
    new_size = stri_dest->size + stri_from->size;
    GROW_STRI(stri_dest, stri_dest, stri_dest->size, new_size);
    if (stri_dest == NULL) {
      raise_error(MEMORY_ERROR);
    } else {
      COUNT3_STRI(stri_dest->size, new_size);
      memcpy(&stri_dest->mem[stri_dest->size], stri_from->mem,
          (SIZE_TYPE) stri_from->size * sizeof(strelemtype));
      stri_dest->size = new_size;
      *stri_to = stri_dest;
    } /* if */
  } /* strAppend */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

rtlArraytype strChEscSplit (const const_stritype main_stri, const chartype delimiter,
    const chartype escape)
#else

rtlArraytype strChEscSplit (main_stri, delimiter, escape)
stritype main_stri;
chartype delimiter;
chartype escape;
#endif

  {
    memsizetype used_max_position;
    const strelemtype *search_start;
    const strelemtype *search_end;
    const strelemtype *curr_pos;
    const strelemtype *found_pos;
    stritype curr_stri;
    const strelemtype *stri_pos;
    memsizetype pos;
    rtlArraytype resized_result_array;
    rtlArraytype result_array;

  /* strChEscSplit */
    if (delimiter == escape) {
      raise_error(RANGE_ERROR);
    } else {
      if (ALLOC_RTL_ARRAY(result_array, 256)) {
        result_array->min_position = 1;
        result_array->max_position = 256;
        used_max_position = 0;
        search_start = main_stri->mem;
        search_end = &main_stri->mem[main_stri->size];
        old_pos = search_start;
        curr_pos = search_start;
        while (curr_pos != search_end && result_array != NULL) {
          while (curr_pos != search_end && *curr_pos != delimiter) {
            while (curr_pos != search_end && *curr_pos != delimiter && *curr_pos != escape) {
              curr_pos++;
            } /* while */
            memcpy(stri_pos, old_pos, curr_pos - old_pos);
            stri_pos += curr_pos - old_pos;
            if (curr_pos != search_end && *curr_pos == escape) {
              curr_pos++;
              if (curr_pos != search_end) {
                *stri_pos = *curr_pos;
                stri_pos++;
              } /* if */
            } /* if */
          } /* while */
          result_array = add_stri_to_array(search_start,
              (memsizetype) (found_pos - search_start), result_array,
              &used_max_position);
          search_start = found_pos + 1;
        
      if (result_array != NULL) {
        result_array = add_stri_to_array(search_start,
            (memsizetype) (search_end - search_start), result_array,
            &used_max_position);
        if (result_array != NULL) {
          resized_result_array = REALLOC_RTL_ARRAY(result_array,
              result_array->max_position, used_max_position);
          if (resized_result_array == NULL) {
            for (pos = 0; pos < used_max_position; pos++) {
              FREE_STRI(result_array->arr[pos].value.strivalue,
                  result_array->arr[pos].value.strivalue->size);
            } /* for */
            FREE_RTL_ARRAY(result_array, result_array->max_position);
            result_array = NULL;
          } else {
            result_array = resized_result_array;
            COUNT3_RTL_ARRAY(result_array->max_position, used_max_position);
            result_array->max_position = used_max_position;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (result_array == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return(result_array);
  } /* strChEscSplit */
#endif



#ifdef ANSI_C

inttype strChIpos (const const_stritype main_stri, const chartype searched,
    const inttype from_index)
#else

inttype strChIpos (main_stri, searched, from_index)
stritype main_stri;
chartype searched;
inttype from_index;
#endif

  {
    const strelemtype *main_mem;
    const strelemtype *found_pos;

  /* strChIpos */
    if (from_index <= 0) {
      raise_error(RANGE_ERROR);
    } else {
      if (from_index <= main_stri->size) {
        main_mem = main_stri->mem;
        found_pos = search_strelem(&main_mem[from_index - 1], searched,
            (SIZE_TYPE) (main_stri->size - from_index + 1));
        if (found_pos != NULL) {
          return(((inttype) (found_pos - main_mem)) + 1);
        } /* if */
      } /* if */
    } /* if */
    return(0);
  } /* strChIpos */



#ifdef ANSI_C

inttype strChPos (const const_stritype main_stri, const chartype searched)
#else

inttype strChPos (main_stri, searched)
stritype main_stri;
chartype searched;
#endif

  {
    const strelemtype *main_mem;
    const strelemtype *found_pos;

  /* strChPos */
    if (main_stri->size >= 1) {
      main_mem = main_stri->mem;
      found_pos = search_strelem(main_mem, searched,
          (SIZE_TYPE) (main_stri->size));
      if (found_pos != NULL) {
        return(((inttype) (found_pos - main_mem)) + 1);
      } /* if */
    } /* if */
    return(0);
  } /* strChPos */



#ifdef ANSI_C

rtlArraytype strChSplit (const const_stritype main_stri, const chartype delimiter)
#else

rtlArraytype strChSplit (main_stri, delimiter)
stritype main_stri;
chartype delimiter;
#endif

  {
    memsizetype used_max_position;
    const strelemtype *search_start;
    const strelemtype *search_end;
    const strelemtype *found_pos;
    memsizetype pos;
    rtlArraytype resized_result_array;
    rtlArraytype result_array;

  /* strChSplit */
    if (ALLOC_RTL_ARRAY(result_array, 256)) {
      result_array->min_position = 1;
      result_array->max_position = 256;
      used_max_position = 0;
      search_start = main_stri->mem;
      search_end = &main_stri->mem[main_stri->size];
      while ((found_pos = search_strelem(search_start,
          delimiter, (SIZE_TYPE) (search_end - search_start))) != NULL &&
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
          resized_result_array = REALLOC_RTL_ARRAY(result_array,
              result_array->max_position, used_max_position);
          if (resized_result_array == NULL) {
            for (pos = 0; pos < used_max_position; pos++) {
              FREE_STRI(result_array->arr[pos].value.strivalue,
                  result_array->arr[pos].value.strivalue->size);
            } /* for */
            FREE_RTL_ARRAY(result_array, result_array->max_position);
            result_array = NULL;
          } else {
            result_array = resized_result_array;
            COUNT3_RTL_ARRAY(result_array->max_position, used_max_position);
            result_array->max_position = used_max_position;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (result_array == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return(result_array);
  } /* strChSplit */



#ifdef ANSI_C

stritype strCLit (const const_stritype str1)
#else

stritype strCLit (str1)
stritype str1;
#endif

  {
    register strelemtype character;
    register memsizetype position;
    memsizetype length;
    memsizetype pos;
    SIZE_TYPE len;
    char buffer[25];
    stritype resized_result;
    stritype result;

  /* strCLit */
    length = str1->size;
    if (!ALLOC_STRI(result, (memsizetype) (4 * length + 2))) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } /* if */
    result->mem[0] = (strelemtype) '"';
    pos = 1;
    for (position = 0; position < length; position++) {
      character = (int) str1->mem[position];
      /* The following comparison uses 255 instead of '\377',       */
      /* because chars might be signed and this can produce wrong   */
      /* code when '\377' is sign extended.                         */
      if (character < ' ') {
        len = strlen(cstri_escape_sequence[character]);
        cstri_expand(&result->mem[pos],
            cstri_escape_sequence[character], len);
        pos = pos + len;
      } else if (character == '\\' || character == '\"') {
        result->mem[pos] = (strelemtype) '\\';
        result->mem[pos + 1] = (strelemtype) character;
        pos = pos + 2;
      } else if (character < 127) {
        result->mem[pos] = (strelemtype) character;
        pos++;
      } else if (character < 256) {
        sprintf(buffer, "\\%lo", character);
        len = strlen(buffer);
        cstri_expand(&result->mem[pos], buffer, len);
        pos = pos + len;
      } else {
        FREE_STRI(result, (memsizetype) (4 * length + 2));
        raise_error(RANGE_ERROR);
        return(NULL);
      } /* if */
    } /* for */
    result->mem[pos] = (strelemtype) '"';
    result->size = pos + 1;
    REALLOC_STRI(resized_result, result,
        (memsizetype) (4 * length + 2), (memsizetype) (pos + 1));
    if (resized_result == NULL) {
      FREE_STRI(result, (memsizetype) (4 * length + 2));
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result = resized_result;
      COUNT3_STRI(4 * length + 2, pos + 1);
      return(result);
    } /* if */
  } /* strCLit */



#ifdef ANSI_C

inttype strCompare (const const_stritype stri1, const const_stritype stri2)
#else

inttype strCompare (stri1, stri2)
stritype stri1;
stritype stri2;
#endif

  {
    inttype result;

  /* strCompare */
    if (stri1->size < stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem,
          (SIZE_TYPE) stri1->size) <= 0) {
        result = -1;
      } else {
        result = 1;
      } /* if */
    } else if (stri1->size > stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem,
          (SIZE_TYPE) stri2->size) >= 0) {
        result = 1;
      } else {
        result = -1;
      } /* if */
    } else {
      result = strelem_memcmp(stri1->mem, stri2->mem,
          (SIZE_TYPE) stri1->size);
      if (result > 0) {
        result = 1;
      } else if (result < 0) {
        result = -1;
      } /* if */
    } /* if */
    return(result);
  } /* strCompare */



#ifdef ANSI_C

stritype strConcat (const const_stritype stri1, const const_stritype stri2)
#else

stritype strConcat (stri1, stri2)
stritype stri1;
stritype stri2;
#endif

  {
    memsizetype result_size;
    stritype result;

  /* strConcat */
    result_size = stri1->size + stri2->size;
    if (!ALLOC_STRI(result, result_size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->size = result_size;
      memcpy(result->mem, stri1->mem,
          (SIZE_TYPE) stri1->size * sizeof(strelemtype));
      memcpy(&result->mem[stri1->size], stri2->mem,
          (SIZE_TYPE) stri2->size * sizeof(strelemtype));
      return(result);
    } /* if */
  } /* strConcat */



/**
 *  Concatenates two strings and returns the result.
 *  The parameter 'stri1' is resized and 'stri2' is copied to the
 *  enlarged area of 'stri1'.
 *  @return the parameter 'stri1.
 */
#ifdef ANSI_C

stritype strConcatTemp (stritype stri1, const const_stritype stri2)
#else

stritype strConcatTemp (stri1, stri2)
stritype stri1;
stritype stri2;
#endif

  {
    memsizetype result_size;
    stritype resized_stri1;

  /* strConcatTemp */
    result_size = stri1->size + stri2->size;
    GROW_STRI(resized_stri1, stri1, stri1->size, result_size);
    if (resized_stri1 == NULL) {
      FREE_STRI(stri1, stri1->size);
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      stri1 = resized_stri1;
      COUNT3_STRI(stri1->size, result_size);
      memcpy(&stri1->mem[stri1->size], stri2->mem,
          (SIZE_TYPE) stri2->size * sizeof(strelemtype));
      stri1->size = result_size;
      return(stri1);
    } /* if */
  } /* strConcatTemp */



#ifdef ANSI_C

void strCopy (stritype *const stri_to, const const_stritype stri_from)
#else

void strCopy (stri_to, stri_from)
stritype *stri_to;
stritype stri_from;
#endif

  {
    memsizetype new_size;
    stritype stri_dest;

  /* strCopy */
    /* printf("stri_to=%lu, stri_from=%lu\n", stri_to, stri_from); */
    stri_dest = *stri_to;
    /* printf("stri_dest=%lu\n", stri_dest); */
    new_size = stri_from->size;
    if (stri_dest->size != new_size) {
      if (!ALLOC_STRI(stri_dest, new_size)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        FREE_STRI(*stri_to, (*stri_to)->size);
        stri_dest->size = new_size;
        *stri_to = stri_dest;
      } /* if */
    } /* if */
    memcpy(stri_dest->mem, stri_from->mem,
        (SIZE_TYPE) new_size * sizeof(strelemtype));
  } /* strCopy */



#ifdef ANSI_C

stritype strCreate (const const_stritype stri_from)
#else

stritype strCreate (stri_from)
stritype stri_from;
#endif

  {
    memsizetype new_size;
    stritype result;

  /* strCreate */
    new_size = stri_from->size;
    if (!ALLOC_STRI(result, new_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = new_size;
      if (new_size != 0) {
        memcpy(result->mem, stri_from->mem,
            (SIZE_TYPE) new_size * sizeof(strelemtype));
      } /* if */
    } /* if */
    return(result);
  } /* strCreate */



#ifdef ANSI_C

void strDestr (const const_stritype old_string)
#else

void strDestr (old_string)
stritype old_string;
#endif

  { /* strDestr */
    if (old_string != NULL) {
      FREE_STRI(old_string, old_string->size);
    } /* if */
  } /* strDestr */



#ifdef ANSI_C

stritype strEmpty (void)
#else

stritype strEmpty ()
#endif

  {
    stritype result;

  /* strEmpty */
    if (!ALLOC_STRI(result, 0)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = 0;
    } /* if */
    return(result);
  } /* strEmpty */



#ifdef ANSI_C

booltype strGe (const const_stritype stri1, const const_stritype stri2)
#else

booltype strGe (stri1, stri2)
stritype stri1;
stritype stri2;
#endif

  { /* strGe */
    if (stri1->size >= stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem,
          (SIZE_TYPE) stri2->size) >= 0) {
        return(TRUE);
      } else {
        return(FALSE);
      } /* if */
    } else {
      if (strelem_memcmp(stri1->mem, stri2->mem,
          (SIZE_TYPE) stri1->size) > 0) {
        return(TRUE);
      } else {
        return(FALSE);
      } /* if */
    } /* if */
  } /* strGe */



#ifdef ANSI_C

stritype strGetenv (const const_stritype stri)
#else

stritype strGetenv (stri)
stritype stri;
#endif

  {
    uchartype env_name[250];
    cstritype environment;
    stritype result;

  /* strGetenv */
    if (compr_size(stri) + 1 > 250) {
      environment = "";
    } else {
      stri_export(env_name, stri);
      if ((environment = getenv(env_name)) == NULL) {
        environment = "";
      } /* if */
    } /* if */
    result = cstri_to_stri(environment);
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return(result);
  } /* strGetenv */



#ifdef ANSI_C

booltype strGt (const const_stritype stri1, const const_stritype stri2)
#else

booltype strGt (stri1, stri2)
stritype stri1;
stritype stri2;
#endif

  { /* strGt */
    if (stri1->size > stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem,
          (SIZE_TYPE) stri2->size) >= 0) {
        return(TRUE);
      } else {
        return(FALSE);
      } /* if */
    } else {
      if (strelem_memcmp(stri1->mem, stri2->mem,
          (SIZE_TYPE) stri1->size) > 0) {
        return(TRUE);
      } else {
        return(FALSE);
      } /* if */
    } /* if */
  } /* strGt */



#ifdef ANSI_C

inttype strHashCode (const const_stritype stri)
#else

inttype strHashCode (stri)
stritype stri;
#endif

  {
    inttype result;

  /* strHashCode */
    if (stri->size == 0) {
      result = 0;
    } else {
      result = stri->mem[0] << 5 ^ stri->size << 3 ^ stri->mem[stri->size - 1];
    } /* if */
    return(result);
  } /* strHashCode */



#ifdef ANSI_C

stritype strHead (const const_stritype stri, const inttype stop)
#else

stritype strHead (stri, stop)
stritype stri;
inttype stop;
#endif

  {
    memsizetype length;
    memsizetype result_size;
    stritype result;

  /* strHead */
    length = stri->size;
    if (stop >= 1 && length >= 1) {
      if (length <= (memsizetype) stop) {
        result_size = length;
      } else {
        result_size = (memsizetype) stop;
      } /* if */
      if (!ALLOC_STRI(result, result_size)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      result->size = result_size;
      memcpy(result->mem, stri->mem,
          (SIZE_TYPE) result_size * sizeof(strelemtype));
    } else {
      if (!ALLOC_STRI(result, (memsizetype) 0)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      result->size = 0;
    } /* if */
    return(result);
  } /* strHead */



#ifdef ANSI_C

inttype strIpos (const const_stritype main_stri, const const_stritype searched,
    const inttype from_index)
#else

inttype strIpos (main_stri, searched, from_index)
stritype main_stri;
stritype searched;
inttype from_index;
#endif

  {
    memsizetype main_size;
    memsizetype searched_size;
    strelemtype ch_1;
    const strelemtype *main_mem;
    const strelemtype *searched_mem;
    const strelemtype *search_start;
    const strelemtype *search_end;

  /* strIpos */
    if (from_index <= 0) {
      raise_error(RANGE_ERROR);
    } else {
      main_size = main_stri->size;
      searched_size = searched->size;
      if (searched_size != 0 && from_index + searched_size - 1 <= main_size) {
        searched_mem = searched->mem;
        ch_1 = searched_mem[0];
        main_mem = &main_stri->mem[from_index - 1];
        main_size -= from_index - 1;
        search_start = main_mem;
        search_end = &main_mem[main_size - searched_size + 1];
        while ((search_start = search_strelem(search_start,
            ch_1, (SIZE_TYPE) (search_end - search_start))) != NULL) {
          if (memcmp(search_start, searched_mem,
              (SIZE_TYPE) searched_size * sizeof(strelemtype)) == 0) {
            return(((inttype) (search_start - main_mem)) + from_index);
          } else {
            search_start++;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return(0);
  } /* strIpos */



#ifdef ANSI_C

booltype strLe (const const_stritype stri1, const const_stritype stri2)
#else

booltype strLe (stri1, stri2)
stritype stri1;
stritype stri2;
#endif

  { /* strLe */
    if (stri1->size <= stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem,
          (SIZE_TYPE) stri1->size) <= 0) {
        return(TRUE);
      } else {
        return(FALSE);
      } /* if */
    } else {
      if (strelem_memcmp(stri1->mem, stri2->mem,
          (SIZE_TYPE) stri2->size) < 0) {
        return(TRUE);
      } else {
        return(FALSE);
      } /* if */
    } /* if */
  } /* strLe */



#ifdef ANSI_C

stritype strLit (const const_stritype stri)
#else

stritype strLit (stri)
stritype stri;
#endif

  {
    register strelemtype character;
    register memsizetype position;
    memsizetype length;
    memsizetype pos;
    SIZE_TYPE len;
    char buffer[25];
    stritype resized_result;
    stritype result;

  /* strLit */
    length = stri->size;
    if (!ALLOC_STRI(result, (memsizetype) (12 * length + 2))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->mem[0] = (strelemtype) '"';
      pos = 1;
      for (position = 0; position < length; position++) {
        character = (strelemtype) stri->mem[position];
        if (character < ' ') {
          len = strlen(stri_escape_sequence[character]);
          cstri_expand(&result->mem[pos],
              stri_escape_sequence[character], len);
          pos = pos + len;
        } else if ((character >= 128 && character < 159) ||
            character >= 255) {
          sprintf(buffer, "\\%lu\\", character);
          len = strlen(buffer);
          cstri_expand(&result->mem[pos], buffer, len);
          pos = pos + len;
        } else if (character == '\\' || character == '\"') {
          result->mem[pos] = (strelemtype) '\\';
          result->mem[pos + 1] = (strelemtype) character;
          pos = pos + 2;
        } else {
          result->mem[pos] = (strelemtype) character;
          pos++;
        } /* if */
      } /* for */
      result->mem[pos] = (strelemtype) '"';
      result->size = pos + 1;
      REALLOC_STRI(resized_result, result,
          (memsizetype) (12 * length + 2), (memsizetype) (pos + 1));
      if (resized_result == NULL) {
        FREE_STRI(result, (memsizetype) (12 * length + 2));
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(5 * length + 2, pos + 1);
      } /* if */
    } /* if */
    return(result);
  } /* strLit */



#ifdef ANSI_C

stritype strLow (const const_stritype stri)
#else

stritype strLow (stri)
stritype stri;
#endif

  {
    memsizetype length;
    memsizetype pos;
    stritype result;

  /* strLow */
    length = stri->size;
    if (!ALLOC_STRI(result, length)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->size = length;
      for (pos = 0; pos < length; pos++) {
#ifdef WIDE_CHAR_STRINGS
        if (((int) stri->mem[pos]) >= 'A' && ((int) stri->mem[pos]) <= 'Z') {
          result->mem[pos] = (strelemtype) (((int) stri->mem[pos]) - 'A' + 'a');
        } else {
          result->mem[pos] = stri->mem[pos];
        } /* if */
#else
        result->mem[pos] = (strelemtype) tolower((int) stri->mem[pos]);
#endif
      } /* for */
      return(result);
    } /* if */
  } /* strLow */



#ifdef ANSI_C

stritype strLowTemp (const stritype stri)
#else

stritype strLowTemp (stri)
stritype stri;
#endif

  {
    memsizetype pos;

  /* strLowTemp */
    for (pos = 0; pos < stri->size; pos++) {
#ifdef WIDE_CHAR_STRINGS
        if (((int) stri->mem[pos]) >= 'A' && ((int) stri->mem[pos]) <= 'Z') {
          stri->mem[pos] = (strelemtype) (((int) stri->mem[pos]) - 'A' + 'a');
        } else {
          stri->mem[pos] = stri->mem[pos];
        } /* if */
#else
        stri->mem[pos] = (strelemtype) tolower((int) stri->mem[pos]);
#endif
    } /* for */
    return(stri);
  } /* strLowTemp */



#ifdef ANSI_C

stritype strLpad (const const_stritype stri, const inttype pad_size)
#else

stritype strLpad (stri, pad_size)
stritype stri;
inttype pad_size;
#endif

  {
    memsizetype f_size;
    memsizetype length;
    stritype result;

  /* strLpad */
    length = stri->size;
    if (pad_size > (inttype) length) {
      f_size = (memsizetype) pad_size;
      if (!ALLOC_STRI(result, f_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = f_size;
#ifdef WIDE_CHAR_STRINGS
        {
          strelemtype *elem = result->mem;
          memsizetype len = f_size - length;

          while (len--) {
            *elem++ = (strelemtype) ' ';
          } /* while */
        }
#else
        memset(result->mem, ' ', (SIZE_TYPE) (f_size - length));
#endif
        memcpy(&result->mem[f_size - length], stri->mem,
            (SIZE_TYPE) length * sizeof(strelemtype));
      } /* if */
    } else {
      if (!ALLOC_STRI(result, length)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = length;
        memcpy(result->mem, stri->mem,
            (SIZE_TYPE) length * sizeof(strelemtype));
      } /* if */
    } /* if */
    return(result);
  } /* strLpad */



#ifdef ANSI_C

stritype strLpad0 (const const_stritype stri, const inttype pad_size)
#else

stritype strLpad0 (stri, pad_size)
stritype stri;
inttype pad_size;
#endif

  {
    memsizetype f_size;
    memsizetype length;
    stritype result;

  /* strLpad0 */
    length = stri->size;
    if (pad_size > (inttype) length) {
      f_size = (memsizetype) pad_size;
      if (!ALLOC_STRI(result, f_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = f_size;
#ifdef WIDE_CHAR_STRINGS
        {
          strelemtype *elem = result->mem;
          memsizetype len = f_size - length;

          while (len--) {
            *elem++ = (strelemtype) '0';
          } /* while */
        }
#else
        memset(result->mem, '0', (SIZE_TYPE) (f_size - length));
#endif
        memcpy(&result->mem[f_size - length], stri->mem,
            (SIZE_TYPE) length * sizeof(strelemtype));
      } /* if */
    } else {
      if (!ALLOC_STRI(result, length)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = length;
        memcpy(result->mem, stri->mem,
            (SIZE_TYPE) length * sizeof(strelemtype));
      } /* if */
    } /* if */
    return(result);
  } /* strLpad0 */



#ifdef ANSI_C

stritype strLpad0Temp (const stritype stri, const inttype pad_size)
#else

stritype strLpad0Temp (stri, pad_size)
stritype stri;
inttype pad_size;
#endif

  {
    memsizetype f_size;
    memsizetype length;
    stritype result;

  /* strLpad0Temp */
    length = stri->size;
    if (pad_size > (inttype) length) {
      f_size = (memsizetype) pad_size;
      if (!ALLOC_STRI(result, f_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = f_size;
#ifdef WIDE_CHAR_STRINGS
        {
          strelemtype *elem = result->mem;
          memsizetype len = f_size - length;

          while (len--) {
            *elem++ = (strelemtype) '0';
          } /* while */
        }
#else
        memset(result->mem, '0', (SIZE_TYPE) (f_size - length));
#endif
        memcpy(&result->mem[f_size - length], stri->mem,
            (SIZE_TYPE) length * sizeof(strelemtype));
        FREE_STRI(stri, length);
      } /* if */
    } else {
      result = stri;
    } /* if */
    return(result);
  } /* strLpad0Temp */



#ifdef ANSI_C

booltype strLt (const const_stritype stri1, const const_stritype stri2)
#else

booltype strLt (stri1, stri2)
stritype stri1;
stritype stri2;
#endif

  { /* strLt */
    if (stri1->size < stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem,
          (SIZE_TYPE) stri1->size) <= 0) {
        return(TRUE);
      } else {
        return(FALSE);
      } /* if */
    } else {
      if (strelem_memcmp(stri1->mem, stri2->mem,
          (SIZE_TYPE) stri2->size) < 0) {
        return(TRUE);
      } else {
        return(FALSE);
      } /* if */
    } /* if */
  } /* strLt */



#ifdef ANSI_C

stritype strLtrim (const const_stritype stri)
#else

stritype strLtrim (stri)
stritype stri;
#endif

  {
    memsizetype start;
    memsizetype length;
    stritype result;

  /* strLtrim */
    start = 0;
    length = stri->size;
    if (length >= 1) {
      while (start < length && stri->mem[start] <= ' ') {
        start++;
      } /* while */
      length -= start;
    } /* if */
    if (!ALLOC_STRI(result, length)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->size = length;
      memcpy(result->mem, &stri->mem[start],
          (SIZE_TYPE) length * sizeof(strelemtype));
      return(result);
    } /* if */
  } /* strLtrim */



#ifdef ANSI_C

stritype strMult (const const_stritype stri, const inttype factor)
#else

stritype strMult (stri, factor)
stritype stri;
inttype factor;
#endif

  {
    memsizetype len;
    inttype number;
    strelemtype *result_pointer;
    strelemtype ch;
    memsizetype result_size;
    stritype result;

  /* strMult */
    if (factor < 0) {
      raise_error(RANGE_ERROR);
      return(NULL);
    } else {
      len = stri->size;
      result_size = ((memsizetype) factor) * len;
      if (!ALLOC_STRI(result, result_size)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        result->size = result_size;
        if (len == 1) {
#ifdef WIDE_CHAR_STRINGS
          ch = stri->mem[0];
          result_pointer = result->mem;
          for (number = factor; number > 0; number--) {
            *result_pointer++ = ch;
          } /* for */
#else
          memset(result->mem, (int) stri->mem[0], (SIZE_TYPE) factor);
#endif
        } else if (len != 0) {
          result_pointer = result->mem;
          for (number = factor; number > 0; number--) {
            memcpy(result_pointer, stri->mem,
                (SIZE_TYPE) len * sizeof(strelemtype));
            result_pointer += (SIZE_TYPE) len;
          } /* for */
        } /* if */
        return(result);
      } /* if */
    } /* if */
  } /* strMult */



#ifdef ANSI_C

inttype strPos (const const_stritype main_stri, const const_stritype searched)
#else

inttype strPos (main_stri, searched)
stritype main_stri;
stritype searched;
#endif

  {
    memsizetype main_size;
    memsizetype searched_size;
    strelemtype ch_1;
    const strelemtype *main_mem;
    const strelemtype *searched_mem;
    const strelemtype *search_start;
    const strelemtype *search_end;

  /* strPos */
    main_size = main_stri->size;
    searched_size = searched->size;
    if (searched_size != 0 && searched_size <= main_size) {
      searched_mem = searched->mem;
      ch_1 = searched_mem[0];
      main_mem = main_stri->mem;
      search_start = main_mem;
      search_end = &main_mem[main_size - searched_size + 1];
      while ((search_start = search_strelem(search_start,
          ch_1, (SIZE_TYPE) (search_end - search_start))) != NULL) {
        if (memcmp(search_start, searched_mem,
            (SIZE_TYPE) searched_size * sizeof(strelemtype)) == 0) {
          return(((inttype) (search_start - main_mem)) + 1);
        } else {
          search_start++;
        } /* if */
      } /* if */
    } /* if */
    return(0);
  } /* strPos */



#ifdef ANSI_C

void strPush (stritype *const stri_to, const chartype char_from)
#else

void strPush (stri_to, char_from)
stritype *stri_to;
chartype char_from;
#endif

  {
    memsizetype new_size;
    stritype stri_dest;

  /* strPush */
#ifndef WIDE_CHAR_STRINGS
    if (char_from > (chartype) 255) {
      raise_error(RANGE_ERROR);
      return(NULL);
    } else {
#endif
      stri_dest = *stri_to;
      new_size = stri_dest->size + 1;
      GROW_STRI(stri_dest, stri_dest, stri_dest->size, new_size);
      if (stri_dest == NULL) {
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_STRI(stri_dest->size, new_size);
        stri_dest->mem[stri_dest->size] = char_from;
        stri_dest->size = new_size;
        *stri_to = stri_dest;
      } /* if */
#ifndef WIDE_CHAR_STRINGS
    } /* if */
#endif
  } /* strPush */



#ifdef ANSI_C

stritype strRange (const const_stritype stri, inttype start, inttype stop)
#else

stritype strRange (stri, start, stop)
stritype stri;
inttype start;
inttype stop;
#endif

  {
    memsizetype length;
    memsizetype result_size;
    stritype result;

  /* strRange */
    length = stri->size;
    if (stop >= 1 && stop >= start && start <= ((inttype) length) &&
        length >= 1) {
      if (start < 1) {
        start = 1;
      } /* if */
      if (stop > (inttype) length) {
        stop = (inttype) length;
      } /* if */
      result_size = (memsizetype) (stop - start + 1);
      if (!ALLOC_STRI(result, result_size)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      /* Reversing the order of the following two statements    */
      /* causes an "Internal Compiler Error" with MSC 6.0       */
      /* when using the -Ozacegilt optimisation option in the   */
      /* large memory model (-AL). Note that the order of the   */
      /* two statements make no difference to the logic of the  */
      /* program.                                               */
      memcpy(result->mem, &stri->mem[start - 1],
          (SIZE_TYPE) result_size * sizeof(strelemtype));
      result->size = result_size;
    } else {
      if (!ALLOC_STRI(result, (memsizetype) 0)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      result->size = 0;
    } /* if */
    return(result);
  } /* strRange */



#ifdef ANSI_C

inttype strRChPos (const const_stritype main_stri, const chartype searched)
#else

inttype strRChPos (main_stri, searched)
stritype main_stri;
chartype searched;
#endif

  {
    const strelemtype *main_mem;
    const strelemtype *found_pos;

  /* strRChPos */
    if (main_stri->size >= 1) {
      main_mem = main_stri->mem;
      found_pos = rsearch_strelem(&main_mem[main_stri->size - 1], searched,
          (SIZE_TYPE) (main_stri->size));
      if (found_pos != NULL) {
        return(((inttype) (found_pos - main_mem)) + 1);
      } /* if */
    } /* if */
    return(0);
  } /* strRChPos */



#ifdef ANSI_C

stritype strRepl (const const_stritype main_stri,
    const const_stritype searched, const const_stritype replace)
#else

stritype strRepl (main_stri, searched, replace)
stritype main_stri;
stritype searched;
stritype replace;
#endif

  {
    memsizetype main_size;
    memsizetype searched_size;
    memsizetype guessed_result_size;
    strelemtype ch_1;
    const strelemtype *main_mem;
    const strelemtype *searched_mem;
    const strelemtype *search_start;
    const strelemtype *search_end;
    const strelemtype *copy_start;
    strelemtype *result_end;
    stritype resized_result;
    stritype result;

  /* strRepl */
    main_size = main_stri->size;
    searched_size = searched->size;
    /* printf("main_size=%ld, searched_size=%ld, replace->size=%ld\n",
       main_size, searched_size, replace->size); */
    if (searched_size != 0 && replace->size > searched_size) {
      guessed_result_size = (main_size / searched_size + 1) * replace->size;
    } else {
      guessed_result_size = main_size;
    } /* if */
    if (!ALLOC_STRI(result, guessed_result_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      copy_start = main_stri->mem;
      result_end = result->mem;
      if (searched_size != 0 && searched_size <= main_size) {
        searched_mem = searched->mem;
        ch_1 = searched_mem[0];
        main_mem = main_stri->mem;
        search_start = main_mem;
        search_end = &main_mem[main_size - searched_size + 1];
        while (search_start < search_end &&
            (search_start = search_strelem(search_start,
            ch_1, (SIZE_TYPE) (search_end - search_start))) != NULL) {
          if (memcmp(search_start, searched_mem,
              (SIZE_TYPE) searched_size * sizeof(strelemtype)) == 0) {
            memcpy(result_end, copy_start,
                (SIZE_TYPE) (search_start - copy_start) * sizeof(strelemtype));
            result_end += search_start - copy_start;
            memcpy(result_end, replace->mem,
                (SIZE_TYPE) replace->size * sizeof(strelemtype));
            result_end += replace->size;
            search_start += searched_size;
            copy_start = search_start;
          } else {
            search_start++;
          } /* if */
        } /* if */
      } /* if */
      memcpy(result_end, copy_start,
          (SIZE_TYPE) (&main_stri->mem[main_size] - copy_start) * sizeof(strelemtype));
      result_end += &main_stri->mem[main_size] - copy_start;
      result->size = result_end - result->mem;
      /* printf("result=%lu, guessed_result_size=%ld, result->size=%ld\n",
         result, guessed_result_size, result->size); */
      REALLOC_STRI(resized_result, result, guessed_result_size, result->size);
      if (resized_result == NULL) {
        FREE_STRI(result, guessed_result_size);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(guessed_result_size, result->size);
      } /* if */
    } /* if */
    return(result);
  } /* strRepl */



#ifdef ANSI_C

stritype strRpad (const const_stritype stri, const inttype pad_size)
#else

stritype strRpad (stri, pad_size)
stritype stri;
inttype pad_size;
#endif

  {
    memsizetype f_size;
    memsizetype length;
    stritype result;

  /* strRpad */
    length = stri->size;
    if (pad_size > (inttype) length) {
      f_size = (memsizetype) pad_size;
      if (!ALLOC_STRI(result, f_size)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      result->size = f_size;
      memcpy(result->mem, stri->mem,
          (SIZE_TYPE) length * sizeof(strelemtype));
#ifdef WIDE_CHAR_STRINGS
      {
        strelemtype *elem = &result->mem[length];
        memsizetype len = f_size - length;

        while (len--) {
          *elem++ = (strelemtype) ' ';
        } /* while */
      }
#else
      memset(&result->mem[length], ' ',
          (SIZE_TYPE) (f_size - length));
#endif
    } else {
      if (!ALLOC_STRI(result, length)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      result->size = length;
      memcpy(result->mem, stri->mem,
          (SIZE_TYPE) length * sizeof(strelemtype));
    } /* if */
    return(result);
  } /* strRpad */



#ifdef ANSI_C

inttype strRpos (const const_stritype main_stri, const const_stritype searched)
#else

inttype strRpos (main_stri, searched)
stritype main_stri;
stritype searched;
#endif

  {
    memsizetype main_size;
    memsizetype searched_size;
    strelemtype ch_1;
    const strelemtype *main_mem;
    const strelemtype *searched_mem;
    const strelemtype *search_start;
    const strelemtype *search_end;

  /* strRpos */
    main_size = main_stri->size;
    searched_size = searched->size;
    if (searched_size != 0 && searched_size <= main_size) {
      searched_mem = searched->mem;
      ch_1 = searched_mem[0];
      main_mem = main_stri->mem;
      search_start = &main_mem[main_size - searched_size];
      search_end = main_mem - 1;
      while ((search_start = rsearch_strelem(search_start,
          ch_1, (SIZE_TYPE) (search_start - search_end))) != NULL) {
        if (memcmp(search_start, searched_mem,
            (SIZE_TYPE) searched_size * sizeof(strelemtype)) == 0) {
          return(((inttype) (search_start - main_mem)) + 1);
        } else {
          search_start--;
        } /* if */
      } /* if */
    } /* if */
    return(0);
  } /* strRpos */



#ifdef ANSI_C

stritype strRtrim (const const_stritype stri)
#else

stritype strRtrim (stri)
stritype stri;
#endif

  {
    memsizetype length;
    stritype result;

  /* strRtrim */
    length = stri->size;
    while (length > 0 && stri->mem[length - 1] <= ' ') {
      length--;
    } /* while */
    if (!ALLOC_STRI(result, length)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->size = length;
      memcpy(result->mem, stri->mem,
          (SIZE_TYPE) length * sizeof(strelemtype));
      return(result);
    } /* if */
  } /* strRtrim */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

arraytype strSplit (const const_stritype main_stri, chartype delimiter)
#else

arraytype strSplit (main_stri, delimiter)
stritype main_stri;
chartype delimiter;
#endif

  {
    arraytype result_array;
    memsizetype used_max_position;
    errinfotype err_info = OKAY_NO_ERROR;

    memsizetype main_size;
    const strelemtype *main_mem;
    const strelemtype *search_start;
    const strelemtype *search_end;
    const strelemtype *found_pos;
    memsizetype length;
    stritype dest;

  /* strSplit */
    if (ALLOC_ARRAY(result_array, 256)) {
      result_array->min_position = 1;
      result_array->max_position = 256;
      used_max_position = 0;
      main_size = main_stri->size;
      if (main_size >= 1) {
        main_mem = main_stri->mem;
        search_start = main_mem;
        search_end = &main_mem[main_size];
        while ((found_pos = search_strelem(search_start,
            delimiter, (SIZE_TYPE) (search_end - search_start))) != NULL) {
          add_stri_to_array(search_start, found_pos - search_start,
              result_array, &used_max_position, &err_info);
          search_start = found_pos + 1
        } /* while */
        add_stri_to_array(search_start, search_end - search_start,
            result_array, &used_max_position, &err_info);
      } /* if */

      return(result_array);
    } else {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } /* if */
  } /* strSplit */



#ifdef ANSI_C

arraytype strSplit (const const_stritype stri, const const_stritype delimiter)
#else

arraytype strSplit (stri, delimiter)
stritype stri;
stritype delimiter;
#endif

  {
    memsizetype main_size;
    memsizetype delimiter_size;
    strelemtype ch_1;
    const strelemtype *main_mem;
    const strelemtype *delimiter_mem;
    const strelemtype *search_start;
    const strelemtype *search_end;
    const strelemtype *found_pos;
    errinfotype err_info = OKAY_NO_ERROR;

  /* strSplit */
    main_size = main_stri->size;
    delimiter_size = delimiter->size;
    if (delimiter->size == 1) {
      if (delimiter_size <= main_size) {
        delimiter_mem = delimiter->mem;
        ch_1 = delimiter_mem[0];
        main_mem = main_stri->mem;
        search_start = main_mem;
        search_end = &main_mem[main_size - delimiter_size + 1];
        while ((found_pos = search_strelem(search_start,
            ch_1, (SIZE_TYPE) (search_end - search_start))) != NULL) {
          memcpy(dest, search_start, found_pos - search_start);
          search_start = found_pos + 1
        } /* while */
      } /* if */
    } else {
      if (delimiter_size != 0 && delimiter_size <= main_size) {
        delimiter_mem = delimiter->mem;
        ch_1 = delimiter_mem[0];
        main_mem = main_stri->mem;
        search_start = main_mem;
        search_end = &main_mem[main_size - delimiter_size + 1];
        while ((found_pos = search_strelem(search_start,
            ch_1, (SIZE_TYPE) (search_end - search_start))) != NULL) {
          if (memcmp(search_start, delimiter_mem,
              (SIZE_TYPE) delimiter_size * sizeof(strelemtype)) == 0) {
            
            search_start = found_pos + delimiter_size;
          } else {
            search_start++;
          } /* if */
        } /* while */
      } /* if */
    } /* if */
  } /* strSplit */
#endif



#ifdef ANSI_C

rtlArraytype strSplit (const const_stritype main_stri,
    const const_stritype delimiter)
#else

rtlArraytype strSplit (main_stri, delimiter)
stritype main_stri;
stritype delimiter;
#endif

  {
    memsizetype delimiter_size;
    const strelemtype *delimiter_mem;
    strelemtype ch_1;
    memsizetype used_max_position;
    const strelemtype *search_start;
    const strelemtype *segment_start;
    const strelemtype *search_end;
    const strelemtype *found_pos;
    memsizetype pos;
    rtlArraytype resized_result_array;
    rtlArraytype result_array;

  /* strSplit */
    if (ALLOC_RTL_ARRAY(result_array, 256)) {
      result_array->min_position = 1;
      result_array->max_position = 256;
      used_max_position = 0;
      delimiter_size = delimiter->size;
      delimiter_mem = delimiter->mem;
      ch_1 = delimiter_mem[0];
      search_start = main_stri->mem;
      segment_start = search_start;
      if (delimiter_size != 0 && main_stri->size >= delimiter_size) {
        search_end = &main_stri->mem[main_stri->size - delimiter_size + 1];
        while ((found_pos = search_strelem(search_start,
            ch_1, (SIZE_TYPE) (search_end - search_start))) != NULL &&
            result_array != NULL) {
          if (memcmp(found_pos, delimiter_mem,
              (SIZE_TYPE) delimiter_size * sizeof(strelemtype)) == 0) {
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
          resized_result_array = REALLOC_RTL_ARRAY(result_array,
              result_array->max_position, used_max_position);
          if (resized_result_array == NULL) {
            for (pos = 0; pos < used_max_position; pos++) {
              FREE_STRI(result_array->arr[pos].value.strivalue,
                  result_array->arr[pos].value.strivalue->size);
            } /* for */
            FREE_RTL_ARRAY(result_array, result_array->max_position);
            result_array = NULL;
          } else {
            result_array = resized_result_array;
            COUNT3_RTL_ARRAY(result_array->max_position, used_max_position);
            result_array->max_position = used_max_position;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (result_array == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return(result_array);
  } /* strSplit */



#ifdef ANSI_C

stritype strSubstr (const const_stritype stri, inttype start, inttype len)
#else

stritype strSubstr (stri, start, len)
stritype stri;
inttype start;
inttype len;
#endif

  {
    memsizetype length;
    memsizetype result_size;
    stritype result;

  /* strSubstr */
    length = stri->size;
    if (len >= 1 && start + len > 1 && start <= ((inttype) length) &&
        length >= 1) {
      if (start < 1) {
        len += start - 1;
        start = 1;
      } /* if */
      if (start + len - 1 > (inttype) length) {
        result_size = (memsizetype) (length - start + 1);
      } else {
        result_size = (memsizetype) len;
      } /* if */
      if (!ALLOC_STRI(result, result_size)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      memcpy(result->mem, &stri->mem[start - 1],
          (SIZE_TYPE) result_size * sizeof(strelemtype));
      result->size = result_size;
    } else {
      if (!ALLOC_STRI(result, (memsizetype) 0)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      result->size = 0;
    } /* if */
    return(result);
  } /* strSubstr */



#ifdef ANSI_C

stritype strTail (const const_stritype stri, inttype start)
#else

stritype strTail (stri, start)
stritype stri;
inttype start;
#endif

  {
    memsizetype length;
    memsizetype result_size;
    stritype result;

  /* strTail */
    length = stri->size;
    if (start <= (inttype) length && length >= 1) {
      if (start < 1) {
        start = 1;
      } /* if */
      result_size = length - ((memsizetype) start) + 1;
      if (!ALLOC_STRI(result, result_size)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      /* Reversing the order of the following two statements    */
      /* causes an "Internal Compiler Error" with MSC 6.0       */
      /* when using the -Ozacegilt optimisation option in the   */
      /* large memory model (-AL). Note that the order of the   */
      /* two statements make no difference to the logic of the  */
      /* program.                                               */
      memcpy(result->mem, &stri->mem[start - 1],
          (SIZE_TYPE) result_size * sizeof(strelemtype));
      result->size = result_size;
    } else {
      if (!ALLOC_STRI(result, (memsizetype) 0)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      result->size = 0;
    } /* if */
    return(result);
  } /* strTail */



#ifdef ANSI_C

stritype strToUtf8 (const const_stritype stri)
#else

stritype strToUtf8 (stri)
stritype stri;
#endif

  {
    register strelemtype *dest;
    register const strelemtype *source;
    memsizetype len;
    stritype resized_result;
    stritype result;

  /* strToUtf8 */
    if (!ALLOC_STRI(result, compr_size(stri))) {
      raise_error(MEMORY_ERROR);
    } else {
      dest = result->mem;
      source = stri->mem;
      len = stri->size;
      for (; len > 0; source++, len--) {
        if (*source <= 0x7F) {
          *dest++ = *source;
        } else if (*source <= 0x7FF) {
          *dest++ = 0xC0 | ( *source >>  6);
          *dest++ = 0x80 | ( *source        & 0x3F);
        } else if (*source <= 0xFFFF) {
          *dest++ = 0xE0 | ( *source >> 12);
          *dest++ = 0x80 | ((*source >>  6) & 0x3F);
          *dest++ = 0x80 | ( *source        & 0x3F);
        } else if (*source <= 0x1FFFFF) {
          *dest++ = 0xF0 | ( *source >> 18);
          *dest++ = 0x80 | ((*source >> 12) & 0x3F);
          *dest++ = 0x80 | ((*source >>  6) & 0x3F);
          *dest++ = 0x80 | ( *source        & 0x3F);
        } else if (*source <= 0x3FFFFFF) {
          *dest++ = 0xF8 | ( *source >> 24);
          *dest++ = 0x80 | ((*source >> 18) & 0x3F);
          *dest++ = 0x80 | ((*source >> 12) & 0x3F);
          *dest++ = 0x80 | ((*source >>  6) & 0x3F);
          *dest++ = 0x80 | ( *source        & 0x3F);
        } else {
          *dest++ = 0xFC | ( *source >> 30);
          *dest++ = 0x80 | ((*source >> 24) & 0x3F);
          *dest++ = 0x80 | ((*source >> 18) & 0x3F);
          *dest++ = 0x80 | ((*source >> 12) & 0x3F);
          *dest++ = 0x80 | ((*source >>  6) & 0x3F);
          *dest++ = 0x80 | ( *source        & 0x3F);
        } /* if */
      } /* for */
      result->size = dest - result->mem;
      REALLOC_STRI(resized_result, result, compr_size(stri), result->size);
      if (resized_result == NULL) {
        FREE_BSTRI(result, compr_size(stri));
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_BSTRI(compr_size(stri), result->size);
      } /* if */
    } /* if */
    return(result);
  } /* strToUtf8 */



#ifdef ANSI_C

stritype strTrim (const const_stritype stri)
#else

stritype strTrim (stri)
stritype stri;
#endif

  {
    memsizetype start;
    memsizetype length;
    stritype result;

  /* strTrim */
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
    if (!ALLOC_STRI(result, length)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->size = length;
      memcpy(result->mem, &stri->mem[start],
          (SIZE_TYPE) length * sizeof(strelemtype));
      return(result);
    } /* if */
  } /* strTrim */



#ifdef ANSI_C

stritype strUp (const const_stritype stri)
#else

stritype strUp (stri)
stritype stri;
#endif

  {
    memsizetype length;
    memsizetype pos;
    stritype result;

  /* strUp */
    length = stri->size;
    if (!ALLOC_STRI(result, length)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->size = length;
      for (pos = 0; pos < length; pos++) {
#ifdef WIDE_CHAR_STRINGS
        if (((int) stri->mem[pos]) >= 'a' && ((int) stri->mem[pos]) <= 'z') {
          result->mem[pos] = (strelemtype) (((int) stri->mem[pos]) - 'a' + 'A');
        } else {
          result->mem[pos] = stri->mem[pos];
        } /* if */
#else
        result->mem[pos] = (strelemtype) toupper((int) stri->mem[pos]);
#endif
      } /* for */
      return(result);
    } /* if */
  } /* strUp */



#ifdef ANSI_C

stritype strUpTemp (const stritype stri)
#else

stritype strUpTemp (stri)
stritype stri;
#endif

  {
    memsizetype pos;

  /* strUpTemp */
    for (pos = 0; pos < stri->size; pos++) {
#ifdef WIDE_CHAR_STRINGS
      if (((int) stri->mem[pos]) >= 'a' && ((int) stri->mem[pos]) <= 'z') {
        stri->mem[pos] = (strelemtype) (((int) stri->mem[pos]) - 'a' + 'A');
      } else {
        stri->mem[pos] = stri->mem[pos];
      } /* if */
#else
      stri->mem[pos] = (strelemtype) toupper((int) stri->mem[pos]);
#endif
    } /* for */
    return(stri);
  } /* strUpTemp */



#ifdef ANSI_C

stritype strUtf8ToStri (const_stritype stri8)
#else

stritype strUtf8ToStri (stri8)
stritype stri8;
#endif

  {
    memsizetype length;
    memsizetype pos;
    const strelemtype *stri8ptr;
    inttype okay = TRUE;
    stritype resized_result;
    stritype result;

  /* strUtf8ToStri */
    length = stri8->size;
    if (!ALLOC_STRI(result, length)) {
      raise_error(MEMORY_ERROR);
    } else {
      stri8ptr = &stri8->mem[0];
      pos = 0;
      for (; length > 0; pos++) {
        if (*stri8ptr <= 0x7F) {
          result->mem[pos] = *stri8ptr++;
          length--;
        } else if ((stri8ptr[0] & 0xFFFFFFE0) == 0xC0 && length >= 2 &&
                   (stri8ptr[1] & 0xFFFFFFC0) == 0x80) {
          result->mem[pos] = (stri8ptr[0] & 0x1F) << 6 |
                             (stri8ptr[1] & 0x3F);
          stri8ptr += 2;
          length -= 2;
        } else if ((stri8ptr[0] & 0xFFFFFFF0) == 0xE0 && length >= 3 &&
                   (stri8ptr[1] & 0xFFFFFFC0) == 0x80 &&
                   (stri8ptr[2] & 0xFFFFFFC0) == 0x80) {
          result->mem[pos] = (stri8ptr[0] & 0x0F) << 12 |
                             (stri8ptr[1] & 0x3F) <<  6 |
                             (stri8ptr[2] & 0x3F);
          stri8ptr += 3;
          length -= 3;
        } else if ((stri8ptr[0] & 0xFFFFFFF8) == 0xF0 && length >= 4 &&
                   (stri8ptr[1] & 0xFFFFFFC0) == 0x80 &&
                   (stri8ptr[2] & 0xFFFFFFC0) == 0x80 &&
                   (stri8ptr[3] & 0xFFFFFFC0) == 0x80) {
          result->mem[pos] = (stri8ptr[0] & 0x07) << 18 |
                             (stri8ptr[1] & 0x3F) << 12 |
                             (stri8ptr[2] & 0x3F) <<  6 |
                             (stri8ptr[3] & 0x3F);
          stri8ptr += 4;
          length -= 4;
        } else if ((stri8ptr[0] & 0xFFFFFFFC) == 0xF8 && length >= 5 &&
                   (stri8ptr[1] & 0xFFFFFFC0) == 0x80 &&
                   (stri8ptr[2] & 0xFFFFFFC0) == 0x80 &&
                   (stri8ptr[3] & 0xFFFFFFC0) == 0x80 &&
                   (stri8ptr[4] & 0xFFFFFFC0) == 0x80) {
          result->mem[pos] = (stri8ptr[0] & 0x03) << 24 |
                             (stri8ptr[1] & 0x3F) << 18 |
                             (stri8ptr[2] & 0x3F) << 12 |
                             (stri8ptr[3] & 0x3F) <<  6 |
                             (stri8ptr[4] & 0x3F);
          stri8ptr += 5;
          length -= 5;
        } else if ((stri8ptr[0] & 0xFFFFFFFC) == 0xFC && length >= 6 &&
                   (stri8ptr[1] & 0xFFFFFFC0) == 0x80 &&
                   (stri8ptr[2] & 0xFFFFFFC0) == 0x80 &&
                   (stri8ptr[3] & 0xFFFFFFC0) == 0x80 &&
                   (stri8ptr[4] & 0xFFFFFFC0) == 0x80 &&
                   (stri8ptr[5] & 0xFFFFFFC0) == 0x80) {
          result->mem[pos] = (stri8ptr[0] & 0x03) << 30 |
                             (stri8ptr[1] & 0x3F) << 24 |
                             (stri8ptr[2] & 0x3F) << 18 |
                             (stri8ptr[3] & 0x3F) << 12 |
                             (stri8ptr[4] & 0x3F) <<  6 |
                             (stri8ptr[5] & 0x3F);
          stri8ptr += 6;
          length -= 6;
        } else {
          okay = FALSE;
          length = 0;
        } /* if */
      } /* for */
      if (okay) {
        result->size = pos;
        REALLOC_STRI(resized_result, result, stri8->size, result->size);
        if (resized_result == NULL) {
          FREE_STRI(result, stri8->size);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result = resized_result;
        } /* if */
      } else {
        FREE_STRI(result, stri8->size);
        raise_error(RANGE_ERROR);
        result = NULL;
      } /* if */
    } /* if */
    return(result);
  } /* strUtf8ToStri */
