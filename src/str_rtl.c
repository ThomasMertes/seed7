/********************************************************************/
/*                                                                  */
/*  str_rtl.c     Primitive actions for the string type.            */
/*  Copyright (C) 1989 - 2011  Thomas Mertes                        */
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
/*  Changes: 1991, 1992, 1993, 1994, 2005, 2010  Thomas Mertes      */
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


#define CHAR_DELTA_BEYOND 128

static const_cstritype stri_escape_sequence[] = {
    "\\0\\",  "\\1\\",  "\\2\\",  "\\3\\",  "\\4\\",
    "\\5\\",  "\\6\\",  "\\a",    "\\b",    "\\t",
    "\\n",    "\\v",    "\\f",    "\\r",    "\\14\\",
    "\\15\\", "\\16\\", "\\17\\", "\\18\\", "\\19\\",
    "\\20\\", "\\21\\", "\\22\\", "\\23\\", "\\24\\",
    "\\25\\", "\\26\\", "\\e",    "\\28\\", "\\29\\",
    "\\30\\", "\\31\\"};



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
    const strelemtype ch, const strelemtype *beyond)
#else

static INLINE strelemtype *search_strelem (mem, ch, beyond)
strelemtype *mem;
strelemtype ch;
strelemtype *beyond;
#endif

  { /* search_strelem */
    for (; mem != beyond; mem++) {
      if (*mem == ch) {
        return mem;
      } /* if */
    } /* for */
    return NULL;
  } /* search_strelem */



#ifdef ANSI_C

static INLINE const strelemtype *search_strelem2 (const strelemtype *mem,
    const strelemtype ch, const strelemtype *beyond, const memsizetype charDelta[])
#else

static INLINE strelemtype *search_strelem2 (mem, ch, beyond, charDelta)
strelemtype *mem;
strelemtype ch;
strelemtype *beyond;
memsizetype charDelta[];
#endif

  { /* search_strelem2 */
    while (mem < beyond) {
      if (*mem == ch) {
        return mem;
      } else if (*mem < CHAR_DELTA_BEYOND) {
        mem += charDelta[*mem];
      } else {
        mem += charDelta[CHAR_DELTA_BEYOND];
      } /* if */
    } /* while */
    return NULL;
  } /* search_strelem2 */



#ifdef ANSI_C

static INLINE const strelemtype *rsearch_strelem (const strelemtype *mem,
    const strelemtype ch, size_t number)
#else

static INLINE strelemtype *rsearch_strelem (mem, ch, number)
strelemtype *mem;
strelemtype ch;
size_t number;
#endif

  { /* rsearch_strelem */
    for (; number > 0; mem--, number--) {
      if (*mem == ch) {
        return mem;
      } /* if */
    } /* for */
    return NULL;
  } /* rsearch_strelem */



#ifdef ANSI_C

static rtlArraytype add_stri_to_array (const strelemtype *const stri_elems,
    const memsizetype length, rtlArraytype work_array, inttype *used_max_position)
#else

static rtlArraytype add_stri_to_array (stri_elems, length,
    work_array, used_max_position)
strelemtype *stri_elems;
memsizetype length;
rtlArraytype work_array;
inttype *used_max_position;
#endif

  {
    stritype new_stri;
    rtlArraytype resized_work_array;
    memsizetype position;

  /* add_stri_to_array */
    if (ALLOC_STRI_SIZE_OK(new_stri, length)) {
      new_stri->size = length;
      memcpy(new_stri->mem, stri_elems, length * sizeof(strelemtype));
      if (*used_max_position >= work_array->max_position) {
        if (work_array->max_position >= MAX_MEM_INDEX - 256) {
          resized_work_array = NULL;
        } else {
          resized_work_array = REALLOC_RTL_ARRAY(work_array,
              (uinttype) work_array->max_position, (uinttype) work_array->max_position + 256);
        } /* if */
        if (resized_work_array == NULL) {
          FREE_STRI(new_stri, new_stri->size);
          new_stri = NULL;
        } else {
          work_array = resized_work_array;
          COUNT3_RTL_ARRAY((uinttype) work_array->max_position, (uinttype) work_array->max_position + 256);
          work_array->max_position += 256;
        } /* if */
      } /* if */
    } /* if */
    if (new_stri != NULL) {
      work_array->arr[*used_max_position].value.strivalue = new_stri;
      (*used_max_position)++;
    } else {
      for (position = 0; position < (uinttype) *used_max_position; position++) {
        FREE_STRI(work_array->arr[position].value.strivalue,
            work_array->arr[position].value.strivalue->size);
      } /* for */
      FREE_RTL_ARRAY(work_array, (uinttype) work_array->max_position);
      work_array = NULL;
    } /* if */
    return work_array;
  } /* add_stri_to_array */



#ifdef ANSI_C

stritype concat_path (const const_stritype absolutePath,
    const const_stritype relativePath)
#else

stritype concat_path (absolutePath, relativePath)
stritype absolutePath;
stritype relativePath;
#endif

  {
    memsizetype abs_path_length;
    memsizetype estimated_result_size;
    strelemtype *abs_path_end;
    strelemtype *rel_path_pos;
    strelemtype *rel_path_beyond;
    memsizetype result_size;
    stritype resized_result;
    stritype result;

  /* concat_path */
    /* printf("concat_path(");
    prot_stri(absolutePath);
    printf(", ");
    prot_stri(relativePath);
    printf(")\n"); */
    /* absolutePath->mem[0] is always '/'. */
    if (absolutePath->size == 1) {
      abs_path_length = 0;
    } else {
      abs_path_length = absolutePath->size;
    } /* if */
    if (unlikely(abs_path_length > MAX_STRI_LEN - relativePath->size - 2)) {
      result = NULL;
    } else {
      /* There is one slash (/) between the two paths. Temporarily    */
      /* there is also a slash at the end of the intermediate result. */
      estimated_result_size = abs_path_length + relativePath->size + 2;
      if (ALLOC_STRI_SIZE_OK(result, estimated_result_size)) {
        memcpy(result->mem, absolutePath->mem, abs_path_length * sizeof(strelemtype));
        result->mem[abs_path_length] = '/';
        abs_path_end = &result->mem[abs_path_length];
        rel_path_pos = relativePath->mem;
        rel_path_beyond = &relativePath->mem[relativePath->size];
        while (rel_path_pos < rel_path_beyond) {
          if (&rel_path_pos[1] < rel_path_beyond &&
              rel_path_pos[0] == '.' && rel_path_pos[1] == '.' &&
              (&rel_path_pos[2] >= rel_path_beyond || rel_path_pos[2] == '/')) {
            rel_path_pos += 2;
            if (abs_path_end > result->mem) {
              do {
                abs_path_end--;
              } while (*abs_path_end != '/');
            } /* if */
          } else if (&rel_path_pos[0] < rel_path_beyond &&
                     rel_path_pos[0] == '.' &&
                     (&rel_path_pos[1] >= rel_path_beyond || rel_path_pos[1] == '/')) {
            rel_path_pos++;
          } else if (*rel_path_pos == '/') {
            rel_path_pos++;
          } else {
            do {
              abs_path_end++;
              *abs_path_end = *rel_path_pos;
              rel_path_pos++;
            } while (&rel_path_pos[0] < rel_path_beyond && rel_path_pos[0] != '/');
            abs_path_end++;
            /* The line below adds a temporary slash (/) to the end   */
            /* of the intermediate result. Therefore + 2 is used to   */
            /* compute the estimated_result_size.                     */
            *abs_path_end = '/';
          } /* if */
        } /* while */
        if (abs_path_end == result->mem) {
          result->mem[0] = '/';
          result_size = 1;
        } else {
          result_size = (memsizetype) (abs_path_end - result->mem);
        } /* if */
        REALLOC_STRI_SIZE_OK(resized_result, result, estimated_result_size, result_size);
        if (unlikely(resized_result == NULL)) {
          FREE_STRI(result, estimated_result_size);
          result = NULL;
        } else {
          result = resized_result;
          COUNT3_STRI(estimated_result_size, result_size);
          result->size = result_size;
        } /* if */
      } /* if */
    } /* if */
    /* printf("concat_path --> ");
    prot_stri(result);
    printf("\n"); */
    return result;
  } /* concat_path */



#ifdef ALLOW_STRITYPE_SLICES
#ifdef ANSI_C

void strAppend (stritype *const destination, const_stritype extension)
#else

void strAppend (destination, extension)
stritype *destination;
stritype extension;
#endif

  {
    memsizetype new_size;
    stritype stri_dest;
    stritype new_stri;
    memsizetype extension_size;
    strelemtype *extension_mem;
    const strelemtype *extension_origin;

  /* strAppend */
    stri_dest = *destination;
    extension_size = extension->size;
    extension_mem = extension->mem;
    if (unlikely(stri_dest->size > MAX_STRI_LEN - extension_size)) {
      /* number of bytes does not fit into memsizetype */
      raise_error(MEMORY_ERROR);
    } else {
      new_size = stri_dest->size + extension_size;
#ifdef WITH_STRI_CAPACITY
      if (new_size > stri_dest->capacity) {
        if (SLICE_OVERLAPPING(extension, stri_dest)) {
          extension_origin = stri_dest->mem;
        } else {
          extension_origin = NULL;
        } /* if */
        new_stri = growStri(stri_dest, new_size);
        if (unlikely(new_stri == NULL)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          if (extension_origin != NULL) {
            /* It is possible that 'extension' is identical to    */
            /* 'stri_dest' or a slice of it. This can be checked  */
            /* with the origin. In this case 'extension_mem' must */
            /* be corrected after realloc() enlarged 'stri_dest'. */
            extension_mem = &new_stri->mem[extension_mem - extension_origin];
            /* Correcting extension->mem is not necessary, since  */
            /* a slice will not be used afterwards. In case when  */
            /* 'extension is identical to 'stri_dest' changing    */
            /* extension->mem is dangerous since 'extension'      */
            /* could have been released.                          */
          } /* if */
          stri_dest = new_stri;
          *destination = stri_dest;
        } /* if */
      } /* if */
      COUNT3_STRI(stri_dest->size, new_size);
      memcpy(&stri_dest->mem[stri_dest->size], extension_mem,
          extension_size * sizeof(strelemtype));
      stri_dest->size = new_size;
#else
      if (SLICE_OVERLAPPING(extension, stri_dest)) {
        extension_origin = stri_dest->mem;
      } else {
        extension_origin = NULL;
      } /* if */
      GROW_STRI(new_stri, stri_dest, stri_dest->size, new_size);
      if (unlikely(new_stri == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        if (extension_origin != NULL) {
          /* It is possible that 'extension' is identical to    */
          /* 'stri_dest' or a slice of it. This can be checked  */
          /* with the origin. In this case 'extension_mem' must */
          /* be corrected after realloc() enlarged 'stri_dest'. */
          extension_mem = &new_stri->mem[extension_mem - extension_origin];
          /* Correcting extension->mem is not necessary, since  */
          /* a slice will not be used afterwards. In case when  */
          /* 'extension is identical to 'stri_dest' changing    */
          /* extension->mem is dangerous since 'extension'      */
          /* could have been released.                          */
        } /* if */
        COUNT3_STRI(new_stri->size, new_size);
        memcpy(&new_stri->mem[new_stri->size], extension_mem,
            extension_size * sizeof(strelemtype));
        new_stri->size = new_size;
        *destination = new_stri;
      } /* if */
#endif
    } /* if */
  } /* strAppend */

#else



#ifdef ANSI_C

void strAppend (stritype *const destination, const_stritype extension)
#else

void strAppend (destination, extension)
stritype *destination;
stritype extension;
#endif

  {
    memsizetype new_size;
    stritype stri_dest;
    stritype new_stri;

  /* strAppend */
    stri_dest = *destination;
    if (unlikely(stri_dest->size > MAX_STRI_LEN - extension->size)) {
      /* number of bytes does not fit into memsizetype */
      raise_error(MEMORY_ERROR);
    } else {
      new_size = stri_dest->size + extension->size;
#ifdef WITH_STRI_CAPACITY
      if (new_size > stri_dest->capacity) {
        new_stri = growStri(stri_dest, new_size);
        if (unlikely(new_stri == NULL)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          if (stri_dest == extension) {
            /* It is possible that stri_dest == extension holds. */
            /* In this case 'extension' must be corrected        */
            /* after realloc() enlarged 'stri_dest'.             */
            extension = new_stri;
          } /* if */
          stri_dest = new_stri;
          *destination = stri_dest;
        } /* if */
      } /* if */
      COUNT3_STRI(stri_dest->size, new_size);
      memcpy(&stri_dest->mem[stri_dest->size], extension->mem,
          extension->size * sizeof(strelemtype));
      stri_dest->size = new_size;
#else
      GROW_STRI(new_stri, stri_dest, stri_dest->size, new_size);
      if (unlikely(new_stri == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        if (stri_dest == extension) {
          /* It is possible that stri_dest == extension holds. */
          /* In this case 'extension' must be corrected        */
          /* after realloc() enlarged 'stri_dest'.             */
          extension = new_stri;
        } /* if */
        COUNT3_STRI(new_stri->size, new_size);
        memcpy(&new_stri->mem[new_stri->size], extension->mem,
            extension->size * sizeof(strelemtype));
        new_stri->size = new_size;
        *destination = new_stri;
      } /* if */
#endif
    } /* if */
  } /* strAppend */

#endif



#ifdef ANSI_C

void strAppendTemp (stritype *const destination, const stritype extension)
#else

void strAppendTemp (destination, extension)
stritype *destination;
stritype extension;
#endif

  {
    memsizetype new_size;
    stritype stri_dest;

  /* strAppendTemp */
    /* printf("strAppendTemp(dest->size=%lu, from->size=%lu)\n",
       (*destination)->size, extension->size); */
    stri_dest = *destination;
    if (unlikely(stri_dest->size > MAX_STRI_LEN - extension->size)) {
      /* number of bytes does not fit into memsizetype */
      raise_error(MEMORY_ERROR);
    } else {
      new_size = stri_dest->size + extension->size;
#ifdef WITH_STRI_CAPACITY
      if (new_size <= stri_dest->capacity) {
        COUNT3_STRI(stri_dest->size, new_size);
        memcpy(&stri_dest->mem[stri_dest->size], extension->mem,
            extension->size * sizeof(strelemtype));
        stri_dest->size = new_size;
        FREE_STRI(extension, extension->size);
      } else if (new_size <= extension->capacity) {
        if (stri_dest->size != 0) {
          COUNT3_STRI(extension->size, new_size);
          memmove(&extension->mem[stri_dest->size], extension->mem,
              extension->size * sizeof(strelemtype));
          memcpy(extension->mem, stri_dest->mem,
              stri_dest->size * sizeof(strelemtype));
          extension->size = new_size;
        } /* if */
        *destination = extension;
        FREE_STRI(stri_dest, stri_dest->size);
      } else {
        stri_dest = growStri(stri_dest, new_size);
        if (unlikely(stri_dest == NULL)) {
          FREE_STRI(extension, extension->size);
          raise_error(MEMORY_ERROR);
        } else {
          *destination = stri_dest;
          COUNT3_STRI(stri_dest->size, new_size);
          memcpy(&stri_dest->mem[stri_dest->size], extension->mem,
              extension->size * sizeof(strelemtype));
          stri_dest->size = new_size;
          FREE_STRI(extension, extension->size);
        } /* if */
      } /* if */
#else
      GROW_STRI(stri_dest, stri_dest, stri_dest->size, new_size);
      if (unlikely(stri_dest == NULL)) {
        FREE_STRI(extension, extension->size);
        raise_error(MEMORY_ERROR);
      } else {
        *destination = stri_dest;
        COUNT3_STRI(stri_dest->size, new_size);
        memcpy(&stri_dest->mem[stri_dest->size], extension->mem,
            extension->size * sizeof(strelemtype));
        stri_dest->size = new_size;
        FREE_STRI(extension, extension->size);
      } /* if */
#endif
    } /* if */
    /* printf("strAppendTemp() => dest->size=%lu\n", (*destination)->size); */
  } /* strAppendTemp */



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
    inttype used_max_position;
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
    if (unlikely(delimiter == escape)) {
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
            memcpy(stri_pos, old_pos, (memsizetype) (curr_pos - old_pos));
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
              (uinttype) result_array->max_position, (uinttype) used_max_position);
          if (resized_result_array == NULL) {
            for (pos = 0; pos < (uinttype) used_max_position; pos++) {
              FREE_STRI(result_array->arr[pos].value.strivalue,
                  result_array->arr[pos].value.strivalue->size);
            } /* for */
            FREE_RTL_ARRAY(result_array, (uinttype) result_array->max_position);
            result_array = NULL;
          } else {
            result_array = resized_result_array;
            COUNT3_RTL_ARRAY((uinttype) result_array->max_position, (uinttype) used_max_position);
            result_array->max_position = used_max_position;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(result_array == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result_array;
  } /* strChEscSplit */
#endif



#ifdef ANSI_C

inttype strChIPos (const const_stritype main_stri, const chartype searched,
    const inttype from_index)
#else

inttype strChIPos (main_stri, searched, from_index)
stritype main_stri;
chartype searched;
inttype from_index;
#endif

  {
    const strelemtype *main_mem;
    const strelemtype *found_pos;

  /* strChIPos */
    if (unlikely(from_index <= 0)) {
      raise_error(RANGE_ERROR);
    } else {
      if ((uinttype) from_index <= main_stri->size) {
        main_mem = main_stri->mem;
        found_pos = search_strelem(&main_mem[from_index - 1], searched,
            &main_mem[main_stri->size]);
        if (found_pos != NULL) {
          return ((inttype) (found_pos - main_mem)) + 1;
        } /* if */
      } /* if */
    } /* if */
    return 0;
  } /* strChIPos */



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
      found_pos = search_strelem(main_mem, searched, &main_mem[main_stri->size]);
      if (found_pos != NULL) {
        return ((inttype) (found_pos - main_mem)) + 1;
      } /* if */
    } /* if */
    return 0;
  } /* strChPos */



#ifdef ANSI_C

rtlArraytype strChSplit (const const_stritype main_stri, const chartype delimiter)
#else

rtlArraytype strChSplit (main_stri, delimiter)
stritype main_stri;
chartype delimiter;
#endif

  {
    inttype used_max_position;
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
      while ((found_pos = search_strelem(search_start, delimiter, search_end)) != NULL &&
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
              (uinttype) result_array->max_position, (uinttype) used_max_position);
          if (resized_result_array == NULL) {
            for (pos = 0; pos < (uinttype) used_max_position; pos++) {
              FREE_STRI(result_array->arr[pos].value.strivalue,
                  result_array->arr[pos].value.strivalue->size);
            } /* for */
            FREE_RTL_ARRAY(result_array, (uinttype) result_array->max_position);
            result_array = NULL;
          } else {
            result_array = resized_result_array;
            COUNT3_RTL_ARRAY((uinttype) result_array->max_position, (uinttype) used_max_position);
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



#ifdef ANSI_C

stritype strCLit (const const_stritype stri)
#else

stritype strCLit (stri)
stritype stri;
#endif

  {
    register strelemtype character;
    register memsizetype position;
    memsizetype length;
    memsizetype pos;
    size_t len;
    char buffer[25];
    stritype resized_result;
    stritype result;

  /* strCLit */
    length = stri->size;
    if (unlikely(length > (MAX_STRI_LEN - 2) / 4 ||
                 !ALLOC_STRI_SIZE_OK(result, 4 * length + 2))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } /* if */
    result->mem[0] = (strelemtype) '"';
    pos = 1;
    for (position = 0; position < length; position++) {
      character = stri->mem[position];
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
        sprintf(buffer, "\\%o", (int) character);
        len = strlen(buffer);
        cstri_expand(&result->mem[pos], buffer, len);
        pos = pos + len;
      } else {
        FREE_STRI(result, 4 * length + 2);
        raise_error(RANGE_ERROR);
        return NULL;
      } /* if */
    } /* for */
    result->mem[pos] = (strelemtype) '"';
    result->size = pos + 1;
    REALLOC_STRI_SIZE_OK(resized_result, result, 4 * length + 2, pos + 1);
    if (unlikely(resized_result == NULL)) {
      FREE_STRI(result, 4 * length + 2);
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result = resized_result;
      COUNT3_STRI(4 * length + 2, pos + 1);
      return result;
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
    return result;
  } /* strCompare */



/**
 *  Reinterpret the generic parameters as stritype and call strCompare.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(rtlGenerictype) != sizeof(stritype).
 */
#ifdef ANSI_C

inttype strCmpGeneric (const rtlGenerictype value1, const rtlGenerictype value2)
#else

inttype strCmpGeneric (value1, value2)
rtlGenerictype value1;
rtlGenerictype value2;
#endif

  { /* strCmpGeneric */
    return strCompare((const_stritype) value1, (const_stritype) value2);
  } /* strCmpGeneric */



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
    if (unlikely(stri1->size > MAX_STRI_LEN - stri2->size)) {
      /* number of bytes does not fit into memsizetype */
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = stri1->size + stri2->size;
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        memcpy(result->mem, stri1->mem,
            stri1->size * sizeof(strelemtype));
        memcpy(&result->mem[stri1->size], stri2->mem,
            stri2->size * sizeof(strelemtype));
      } /* if */
    } /* if */
    return result;
  } /* strConcat */



#ifdef ANSI_C

stritype strConcatN (const const_stritype striArray[], memsizetype arraySize)
#else

stritype strConcatN (striArray, arraySize)
const const_stritype striArray[];
memsizetype arraySize;
#endif

  {
    memsizetype pos;
    memsizetype result_size = 0;
    stritype result;

  /* strConcatN */
    for (pos = 0; pos < arraySize; pos++) {
      if (unlikely(result_size > MAX_STRI_LEN - striArray[pos]->size)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        result_size += striArray[pos]->size;
      } /* if */
    } /* for */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = result_size;
      result_size = 0;
      for (pos = 0; pos < arraySize; pos++) {
        memcpy(&result->mem[result_size], striArray[pos]->mem,
            striArray[pos]->size * sizeof(strelemtype));
        result_size += striArray[pos]->size;
      } /* for */
    } /* if */
    return result;
  } /* strConcatN */



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
    if (unlikely(stri1->size > MAX_STRI_LEN - stri2->size)) {
      /* number of bytes does not fit into memsizetype */
      FREE_STRI(stri1, stri1->size);
      raise_error(MEMORY_ERROR);
      stri1 = NULL;
    } else {
      result_size = stri1->size + stri2->size;
#ifdef WITH_STRI_CAPACITY
      if (result_size > stri1->capacity) {
        resized_stri1 = growStri(stri1, result_size);
        if (unlikely(resized_stri1 == NULL)) {
          FREE_STRI(stri1, stri1->size);
          raise_error(MEMORY_ERROR);
          return NULL;
        } else {
          stri1 = resized_stri1;
        } /* if */
      } /* if */
      COUNT3_STRI(stri1->size, result_size);
      memcpy(&stri1->mem[stri1->size], stri2->mem,
          stri2->size * sizeof(strelemtype));
      stri1->size = result_size;
#else
      GROW_STRI(resized_stri1, stri1, stri1->size, result_size);
      if (unlikely(resized_stri1 == NULL)) {
        FREE_STRI(stri1, stri1->size);
        raise_error(MEMORY_ERROR);
        stri1 = NULL;
      } else {
        stri1 = resized_stri1;
        COUNT3_STRI(stri1->size, result_size);
        memcpy(&stri1->mem[stri1->size], stri2->mem,
            stri2->size * sizeof(strelemtype));
        stri1->size = result_size;
      } /* if */
#endif
    } /* if */
    return stri1;
  } /* strConcatTemp */



#ifdef ALLOW_STRITYPE_SLICES
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
    strelemtype *stri_from_mem;
#ifndef WITH_STRI_CAPACITY
    const strelemtype *stri_from_origin;
#endif

  /* strCopy */
    /* printf("begin strCopy(");
    filWrite(stdout, *stri_to);
    printf(", %ld)\n", stri_from->size); */
    stri_dest = *stri_to;
    new_size = stri_from->size;
#ifdef WITH_STRI_CAPACITY
    if (stri_dest->capacity >= new_size && !SHRINK_REASON(stri_dest, new_size)) {
      COUNT3_STRI(stri_dest->size, new_size);
      stri_dest->size = new_size;
      stri_from_mem = stri_from->mem;
#else
    if (stri_dest->size > new_size) {
      stri_from_mem = stri_from->mem;
      if (SLICE_OVERLAPPING(stri_from, stri_dest)) {
        stri_from_origin = stri_dest->mem;
      } else {
        stri_from_origin = NULL;
      } /* if */
      SHRINK_STRI(stri_dest, stri_dest, stri_dest->size, new_size);
      /* printf("strCopy(old_size=%lu, new_size=%lu)\n", stri_dest->size, new_size); */
      if (unlikely(stri_dest == NULL)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        COUNT3_STRI(stri_dest->size, new_size);
        stri_dest->size = new_size;
        if (stri_from_origin != NULL) {
          /* It is possible that 'stri_from' is identical to    */
          /* '*stri_to' or a slice of it. This can be checked   */
          /* with the origin. In this case 'stri_from_mem' must */
          /* be corrected after realloc() enlarged 'stri_dest'. */
          stri_from_mem = &stri_dest->mem[stri_from_mem - stri_from_origin];
          /* Correcting stri_from->mem is not necessary, since  */
          /* a slice will not be used afterwards. In case when  */
          /* 'stri_from' is identical to '*stri_to' changing    */
          /* stri_from->mem is dangerous since 'stri_from'      */
          /* could have been released.                          */
        } /* if */
        *stri_to = stri_dest;
      } /* if */
#endif
      /* It is possible that stri_dest and stri_from overlap. */
      /* The behavior of memcpy() is undefined when source    */
      /* and destination areas overlap (or are identical).    */
      /* Therefore memmove() is used instead of memcpy().     */
      memmove(stri_dest->mem, stri_from_mem,
          new_size * sizeof(strelemtype));
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(stri_dest, new_size))) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        stri_dest->size = new_size;
        memcpy(stri_dest->mem, stri_from->mem,
            new_size * sizeof(strelemtype));
        FREE_STRI(*stri_to, (*stri_to)->size);
        *stri_to = stri_dest;
      } /* if */
    } /* if */
  } /* strCopy */

#else


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
#ifdef WITH_STRI_CAPACITY
      if (stri_dest->capacity >= new_size && !SHRINK_REASON(stri_dest, new_size)) {
        stri_dest->size = new_size;
#else
      if (stri_dest->size > new_size) {
        SHRINK_STRI(stri_dest, stri_dest, stri_dest->size, new_size);
        /* printf("strCopy(old_size=%lu, new_size=%lu)\n", stri_dest->size, new_size); */
        if (unlikely(stri_dest == NULL)) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          COUNT3_STRI(stri_dest->size, new_size);
          stri_dest->size = new_size;
          *stri_to = stri_dest;
        } /* if */
#endif
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(stri_dest, new_size))) {
          raise_error(MEMORY_ERROR);
          return;
        } else {
          FREE_STRI(*stri_to, (*stri_to)->size);
          stri_dest->size = new_size;
          *stri_to = stri_dest;
        } /* if */
      } /* if */
    } /* if */
    /* It is possible that *stri_to == stri_from holds. The */
    /* behavior of memcpy() is undefined when source and    */
    /* destination areas overlap (or are identical).        */
    /* Therefore memmove() is used instead of memcpy().     */
    memmove(stri_dest->mem, stri_from->mem,
        new_size * sizeof(strelemtype));
  } /* strCopy */

#endif



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
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, new_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = new_size;
      if (new_size != 0) {
        memcpy(result->mem, stri_from->mem, new_size * sizeof(strelemtype));
      } /* if */
    } /* if */
    return result;
  } /* strCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(rtlGenerictype) != sizeof(stritype).
 */
#ifdef ANSI_C

rtlGenerictype strCreateGeneric (const rtlGenerictype from_value)
#else

rtlGenerictype strCreateGeneric (from_value)
rtlGenerictype from_value;
#endif

  { /* strCreateGeneric */
    return (rtlGenerictype) (memsizetype) strCreate((const_stritype) from_value);
  } /* strCreateGeneric */



#ifdef ANSI_C

void strDestr (const const_stritype old_string)
#else

void strDestr (old_string)
stritype old_string;
#endif

  { /* strDestr */
    /* printf("strDestr(%lX)\n", old_string); */
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
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = 0;
    } /* if */
    return result;
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
      if (strelem_memcmp(stri1->mem, stri2->mem, stri2->size) >= 0) {
        return TRUE;
      } else {
        return FALSE;
      } /* if */
    } else {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri1->size) > 0) {
        return TRUE;
      } else {
        return FALSE;
      } /* if */
    } /* if */
  } /* strGe */



#ifdef ANSI_C

booltype strGt (const const_stritype stri1, const const_stritype stri2)
#else

booltype strGt (stri1, stri2)
stritype stri1;
stritype stri2;
#endif

  { /* strGt */
    if (stri1->size > stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri2->size) >= 0) {
        return TRUE;
      } else {
        return FALSE;
      } /* if */
    } else {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri1->size) > 0) {
        return TRUE;
      } else {
        return FALSE;
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
      result = (inttype) (stri->mem[0] << 5 ^ stri->size << 3 ^ stri->mem[stri->size - 1]);
    } /* if */
    return result;
  } /* strHashCode */



#ifdef ALLOW_STRITYPE_SLICES
#ifdef ANSI_C

stritype strHeadSlice (const const_stritype stri, const inttype stop, stritype slice)
#else

stritype strHeadSlice (stri, stop, slice)
stritype stri;
inttype stop;
stritype slice;
#endif

  {
    memsizetype length;

  /* strHeadSlice */
    length = stri->size;
    if (stop >= 1 && length >= 1) {
      SET_SLICE_CAPACITY(slice, 0);
      slice->mem = stri->mem;
      if (length <= (uinttype) stop) {
        slice->size = length;
      } else {
        slice->size = (memsizetype) stop;
      } /* if */
    } else {
      SET_SLICE_CAPACITY(slice, 0);
      slice->mem = NULL;
      slice->size = 0;
    } /* if */
    return slice;
  } /* strHeadSlice */

#else



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
      if (length <= (uinttype) stop) {
        result_size = length;
      } else {
        result_size = (memsizetype) stop;
      } /* if */
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        memcpy(result->mem, stri->mem, result_size * sizeof(strelemtype));
      } /* if */
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memsizetype) 0))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = 0;
      } /* if */
    } /* if */
    return result;
  } /* strHead */

#endif



#ifdef ANSI_C

stritype strHeadTemp (const stritype stri, const inttype stop)
#else

stritype strHeadTemp (stri, stop)
stritype stri;
inttype stop;
#endif

  {
    memsizetype length;
    memsizetype result_size;
    stritype result;

  /* strHeadTemp */
    length = stri->size;
    if (stop >= 1 && length >= 1) {
      if (length <= (uinttype) stop) {
        return stri;
      } else {
        result_size = (memsizetype) stop;
      } /* if */
    } else {
      result_size = 0;
    } /* if */
#ifdef WITH_STRI_CAPACITY
    if (!SHRINK_REASON(stri, result_size)) {
      COUNT3_STRI(length, result_size);
      result = stri;
      result->size = result_size;
    } else {
      result = shrinkStri(stri, result_size);
      if (unlikely(result == NULL)) {
        FREE_STRI(stri, stri->size);
        raise_error(MEMORY_ERROR);
      } else {
        COUNT3_STRI(length, result_size);
        result->size = result_size;
      } /* if */
    } /* if */
#else
    SHRINK_STRI(result, stri, length, result_size);
    if (unlikely(result == NULL)) {
      FREE_STRI(stri, stri->size);
      raise_error(MEMORY_ERROR);
    } else {
      COUNT3_STRI(length, result_size);
      result->size = result_size;
    } /* if */
#endif
    return result;
  } /* strHeadTemp */



#ifdef ANSI_C

static inttype strIPos2 (const const_stritype main_stri, const const_stritype searched,
    const inttype from_index)
#else

static inttype strIPos2 (main_stri, searched, from_index)
stritype main_stri;
stritype searched;
inttype from_index;
#endif

  {
    memsizetype main_size;
    memsizetype searched_size;
    strelemtype ch_n;
    const strelemtype *ch_n_pos;
    memsizetype delta;
    memsizetype charDelta[CHAR_DELTA_BEYOND + 1];
    memsizetype pos;
    const strelemtype *main_mem;
    const strelemtype *searched_mem;
    const strelemtype *search_start;
    const strelemtype *search_end;

  /* strIPos2 */
    main_size = main_stri->size - ((memsizetype) from_index - 1);
    searched_size = searched->size;
    for (ch_n = 0; ch_n <= CHAR_DELTA_BEYOND; ch_n++) {
      charDelta[ch_n] = searched_size;
    } /* for */
    searched_mem = searched->mem;
    for (pos = 0; pos < searched_size - 1; pos++) {
      ch_n = searched_mem[pos];
      if (ch_n < CHAR_DELTA_BEYOND) {
        charDelta[ch_n] = searched_size - pos - 1;
      } else {
        charDelta[CHAR_DELTA_BEYOND] = searched_size - pos - 1;
      } /* if */
    } /* for */
    ch_n = searched_mem[searched_size - 1];
    ch_n_pos = rsearch_strelem(&searched_mem[searched_size - 2], ch_n, searched_size - 1);
    if (ch_n_pos == NULL) {
      delta = searched_size;
    } else {
      delta = (memsizetype) (&searched_mem[searched_size - 1] - ch_n_pos);
    } /* if */
    main_mem = &main_stri->mem[from_index - 1];
    search_start = &main_mem[searched_size - 1];
    search_end = &main_mem[main_size];
    while (search_start < search_end) {
      search_start = search_strelem2(search_start, ch_n, search_end, charDelta);
      if (search_start == NULL) {
        return 0;
      } else {
        if (memcmp(search_start - searched_size + 1, searched_mem,
            (searched_size - 1) * sizeof(strelemtype)) == 0) {
          return ((inttype) (search_start - searched_size + 1 - main_mem)) + from_index;
        } else {
          search_start += delta;
        } /* if */
      } /* if */
    } /* while */
    return 0;
  } /* strIPos2 */



#ifdef ANSI_C

inttype strIPos (const const_stritype main_stri, const const_stritype searched,
    const inttype from_index)
#else

inttype strIPos (main_stri, searched, from_index)
stritype main_stri;
stritype searched;
inttype from_index;
#endif

  {
    memsizetype main_size;
    memsizetype searched_size;
    strelemtype ch_n;
    const strelemtype *ch_n_pos;
    memsizetype delta;
    const strelemtype *main_mem;
    const strelemtype *searched_mem;
    const strelemtype *search_start;
    const strelemtype *search_end;

  /* strIPos */
    if (unlikely(from_index <= 0)) {
      raise_error(RANGE_ERROR);
    } else {
      main_size = main_stri->size;
      searched_size = searched->size;
      if (searched_size != 0 && main_size >= searched_size &&
          (uinttype) from_index - 1 <= main_size - searched_size) {
        main_size -= (memsizetype) from_index - 1;
        if (searched_size >= 2 && main_size >= 1400) {
          return strIPos2(main_stri, searched, from_index);
        } else {
          searched_mem = searched->mem;
          ch_n = searched_mem[searched_size - 1];
          ch_n_pos = rsearch_strelem(&searched_mem[searched_size - 2], ch_n, searched_size - 1);
          if (ch_n_pos == NULL) {
            delta = searched_size;
          } else {
            delta = (memsizetype) (&searched_mem[searched_size - 1] - ch_n_pos);
          } /* if */
          main_mem = &main_stri->mem[from_index - 1];
          search_start = &main_mem[searched_size - 1];
          search_end = &main_mem[main_size];
          while (search_start < search_end) {
            search_start = search_strelem(search_start, ch_n, search_end);
            if (search_start == NULL) {
              return 0;
            } else {
              if (memcmp(search_start - searched_size + 1, searched_mem,
                  (searched_size - 1) * sizeof(strelemtype)) == 0) {
                return ((inttype) (search_start - searched_size + 1 - main_mem)) + from_index;
              } else {
                search_start += delta;
              } /* if */
            } /* if */
          } /* while */
        } /* if */
      } /* if */
    } /* if */
    return 0;
  } /* strIPos */



#ifdef ANSI_C

booltype strLe (const const_stritype stri1, const const_stritype stri2)
#else

booltype strLe (stri1, stri2)
stritype stri1;
stritype stri2;
#endif

  { /* strLe */
    if (stri1->size <= stri2->size) {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri1->size) <= 0) {
        return TRUE;
      } else {
        return FALSE;
      } /* if */
    } else {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri2->size) < 0) {
        return TRUE;
      } else {
        return FALSE;
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
    size_t len;
    char buffer[25];
    stritype resized_result;
    stritype result;

  /* strLit */
    length = stri->size;
    if (unlikely(length > (MAX_STRI_LEN - 2) / 12 ||
                 !ALLOC_STRI_SIZE_OK(result, 12 * length + 2))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
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
        } else if ((character >= 127 && character < 159) ||
            character >= 255) {
          sprintf(buffer, "\\%lu\\", (unsigned long) character);
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
      REALLOC_STRI_SIZE_OK(resized_result, result, 12 * length + 2, pos + 1);
      if (unlikely(resized_result == NULL)) {
        FREE_STRI(result, 12 * length + 2);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(5 * length + 2, pos + 1);
      } /* if */
    } /* if */
    return result;
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
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = length;
      for (pos = 0; pos < length; pos++) {
        if (stri->mem[pos] >= (strelemtype) 'A' && stri->mem[pos] <= (strelemtype) 'Z') {
          result->mem[pos] = stri->mem[pos] - (strelemtype) 'A' + (strelemtype) 'a';
        } else {
          result->mem[pos] = stri->mem[pos];
        } /* if */
      } /* for */
      return result;
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
      if (stri->mem[pos] >= (strelemtype) 'A' && stri->mem[pos] <= (strelemtype) 'Z') {
        stri->mem[pos] = stri->mem[pos] - (strelemtype) 'A' + (strelemtype) 'a';
      } /* if */
    } /* for */
    return stri;
  } /* strLowTemp */



#ifdef ANSI_C

stritype strLpad (const const_stritype stri, const inttype pad_size)
#else

stritype strLpad (stri, pad_size)
stritype stri;
inttype pad_size;
#endif

  {
    memsizetype length;
    stritype result;

  /* strLpad */
    length = stri->size;
    if (pad_size > 0 && (uinttype) pad_size > length) {
      if (unlikely((uinttype) pad_size > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memsizetype) pad_size))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
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
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = length;
        memcpy(result->mem, stri->mem, length * sizeof(strelemtype));
      } /* if */
    } /* if */
    return result;
  } /* strLpad */



#ifdef ANSI_C

stritype strLpadTemp (const stritype stri, const inttype pad_size)
#else

stritype strLpadTemp (stri, pad_size)
stritype stri;
inttype pad_size;
#endif

  {
    memsizetype length;
    stritype result;

  /* strLpadTemp */
    length = stri->size;
    if (pad_size > 0 && (uinttype) pad_size > length) {
      if (unlikely((uinttype) pad_size > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memsizetype) pad_size))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
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
        FREE_STRI(stri, length);
      } /* if */
    } else {
      result = stri;
    } /* if */
    return result;
  } /* strLpadTemp */



#ifdef ANSI_C

stritype strLpad0 (const const_stritype stri, const inttype pad_size)
#else

stritype strLpad0 (stri, pad_size)
stritype stri;
inttype pad_size;
#endif

  {
    memsizetype length;
    strelemtype *sourceElem;
    strelemtype *destElem;
    memsizetype len;
    stritype result;

  /* strLpad0 */
    length = stri->size;
    if (pad_size > 0 && (uinttype) pad_size > length) {
      if (unlikely((uinttype) pad_size > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memsizetype) pad_size))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
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
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = length;
        memcpy(result->mem, stri->mem, length * sizeof(strelemtype));
      } /* if */
    } /* if */
    return result;
  } /* strLpad0 */



#ifdef ANSI_C

stritype strLpad0Temp (const stritype stri, const inttype pad_size)
#else

stritype strLpad0Temp (stri, pad_size)
stritype stri;
inttype pad_size;
#endif

  {
    memsizetype length;
    strelemtype *sourceElem;
    strelemtype *destElem;
    memsizetype len;
    stritype result;

  /* strLpad0Temp */
    length = stri->size;
    if (pad_size > 0 && (uinttype) pad_size > length) {
      if (unlikely((uinttype) pad_size > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memsizetype) pad_size))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
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
        FREE_STRI(stri, length);
      } /* if */
    } else {
      result = stri;
    } /* if */
    return result;
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
      if (strelem_memcmp(stri1->mem, stri2->mem, stri1->size) <= 0) {
        return TRUE;
      } else {
        return FALSE;
      } /* if */
    } else {
      if (strelem_memcmp(stri1->mem, stri2->mem, stri2->size) < 0) {
        return TRUE;
      } else {
        return FALSE;
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
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = length;
      memcpy(result->mem, &stri->mem[start], length * sizeof(strelemtype));
      return result;
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
    /* printf("strMult(stri->size=%lu, %ld)\n", stri->size, factor); */
    if (unlikely(factor < 0)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      len = stri->size;
      if (unlikely(len != 0 && (uinttype) factor > MAX_STRI_LEN / len)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result_size = (memsizetype) factor * len;
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = result_size;
          if (len == 1) {
            ch = stri->mem[0];
            result_pointer = result->mem;
            for (number = factor; number > 0; number--) {
              *result_pointer++ = ch;
            } /* for */
          } else if (len != 0) {
            result_pointer = result->mem;
            for (number = factor; number > 0; number--) {
              memcpy(result_pointer, stri->mem, len * sizeof(strelemtype));
              result_pointer += len;
            } /* for */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* strMult */



#ifdef ANSI_C

static inttype strPos2 (const const_stritype main_stri, const const_stritype searched)
#else

static inttype strPos2 (main_stri, searched)
stritype main_stri;
stritype searched;
#endif

  {
    memsizetype main_size;
    memsizetype searched_size;
    strelemtype ch_n;
    const strelemtype *ch_n_pos;
    memsizetype delta;
    memsizetype charDelta[CHAR_DELTA_BEYOND + 1];
    memsizetype pos;
    const strelemtype *main_mem;
    const strelemtype *searched_mem;
    const strelemtype *search_start;
    const strelemtype *search_end;

  /* strPos2 */
    main_size = main_stri->size;
    searched_size = searched->size;
    for (ch_n = 0; ch_n <= CHAR_DELTA_BEYOND; ch_n++) {
      charDelta[ch_n] = searched_size;
    } /* for */
    searched_mem = searched->mem;
    for (pos = 0; pos < searched_size - 1; pos++) {
      ch_n = searched_mem[pos];
      if (ch_n < CHAR_DELTA_BEYOND) {
        charDelta[ch_n] = searched_size - pos - 1;
      } else {
        charDelta[CHAR_DELTA_BEYOND] = searched_size - pos - 1;
      } /* if */
    } /* for */
    ch_n = searched_mem[searched_size - 1];
    ch_n_pos = rsearch_strelem(&searched_mem[searched_size - 2], ch_n, searched_size - 1);
    if (ch_n_pos == NULL) {
      delta = searched_size;
    } else {
      delta = (memsizetype) (&searched_mem[searched_size - 1] - ch_n_pos);
    } /* if */
    main_mem = main_stri->mem;
    search_start = &main_mem[searched_size - 1];
    search_end = &main_mem[main_size];
    while (search_start < search_end) {
      search_start = search_strelem2(search_start, ch_n, search_end, charDelta);
      if (search_start == NULL) {
        return 0;
      } else {
        if (memcmp(search_start - searched_size + 1, searched_mem,
            (searched_size - 1) * sizeof(strelemtype)) == 0) {
          return ((inttype) (search_start - searched_size + 1 - main_mem)) + 1;
        } else {
          search_start += delta;
        } /* if */
      } /* if */
    } /* while */
    return 0;
  } /* strPos2 */



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
    strelemtype ch_n;
    const strelemtype *ch_n_pos;
    memsizetype delta;
    const strelemtype *main_mem;
    const strelemtype *searched_mem;
    const strelemtype *search_start;
    const strelemtype *search_end;

  /* strPos */
    main_size = main_stri->size;
    searched_size = searched->size;
    if (searched_size != 0 && main_size >= searched_size) {
      if (searched_size >= 2 && main_size >= 1400) {
        return strPos2(main_stri, searched);
      } else {
        searched_mem = searched->mem;
        ch_n = searched_mem[searched_size - 1];
        ch_n_pos = rsearch_strelem(&searched_mem[searched_size - 2], ch_n, searched_size - 1);
        if (ch_n_pos == NULL) {
          delta = searched_size;
        } else {
          delta = (memsizetype) (&searched_mem[searched_size - 1] - ch_n_pos);
        } /* if */
        main_mem = main_stri->mem;
        search_start = &main_mem[searched_size - 1];
        search_end = &main_mem[main_size];
        while (search_start < search_end) {
          search_start = search_strelem(search_start, ch_n, search_end);
          if (search_start == NULL) {
            return 0;
          } else {
            if (memcmp(search_start - searched_size + 1, searched_mem,
                (searched_size - 1) * sizeof(strelemtype)) == 0) {
              return ((inttype) (search_start - searched_size + 1 - main_mem)) + 1;
            } else {
              search_start += delta;
            } /* if */
          } /* if */
        } /* while */
      } /* if */
    } /* if */
    return 0;
  } /* strPos */



#ifdef ANSI_C

void strPush (stritype *const destination, const chartype extension)
#else

void strPush (destination, extension)
stritype *destination;
chartype extension;
#endif

  {
    memsizetype new_size;
    stritype stri_dest;

  /* strPush */
    stri_dest = *destination;
    new_size = stri_dest->size + 1;
#ifdef WITH_STRI_CAPACITY
    if (new_size > stri_dest->capacity) {
      stri_dest = growStri(stri_dest, new_size);
      if (unlikely(stri_dest == NULL)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        *destination = stri_dest;
      } /* if */
    } /* if */
    COUNT3_STRI(stri_dest->size, new_size);
    stri_dest->mem[stri_dest->size] = extension;
    stri_dest->size = new_size;
#else
    GROW_STRI(stri_dest, stri_dest, stri_dest->size, new_size);
    if (unlikely(stri_dest == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      COUNT3_STRI(stri_dest->size, new_size);
      stri_dest->mem[stri_dest->size] = extension;
      stri_dest->size = new_size;
      *destination = stri_dest;
    } /* if */
#endif
  } /* strPush */



#ifdef ALLOW_STRITYPE_SLICES
#ifdef ANSI_C

stritype strRangeSlice (const const_stritype stri, inttype start, inttype stop, stritype slice)
#else

stritype strRangeSlice (stri, start, stop, slice)
stritype stri;
inttype start;
inttype stop;
stritype slice;
#endif

  {
    memsizetype length;

  /* strRangeSlice */
    length = stri->size;
    if (start < 1) {
      start = 1;
    } /* if */
    if (stop >= 1 && stop >= start && (uinttype) start <= length &&
        length >= 1) {
      SET_SLICE_CAPACITY(slice, 0);
      slice->mem = &stri->mem[start - 1];
      if ((uinttype) stop > length) {
        slice->size = length - (memsizetype) start + 1;
      } else {
        slice->size = (memsizetype) stop - (memsizetype) start + 1;
      } /* if */
    } else {
      SET_SLICE_CAPACITY(slice, 0);
      slice->mem = NULL;
      slice->size = 0;
    } /* if */
    return slice;
  } /* strRangeSlice */

#else



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
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
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
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memsizetype) 0))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
      result->size = 0;
    } /* if */
    return result;
  } /* strRange */

#endif



#ifdef ANSI_C

inttype strRChIPos (const const_stritype main_stri, const chartype searched,
    const inttype from_index)
#else

inttype strRChIPos (main_stri, searched, from_index)
stritype main_stri;
chartype searched;
inttype from_index;
#endif

  {
    const strelemtype *main_mem;
    const strelemtype *found_pos;

  /* strRChIPos */
    if (likely(from_index >= 1)) {
      if (unlikely((uinttype) from_index > main_stri->size)) {
        raise_error(RANGE_ERROR);
      } else {
        if (main_stri->size >= 1) {
          main_mem = main_stri->mem;
          found_pos = rsearch_strelem(&main_mem[from_index - 1], searched,
              (memsizetype) from_index);
          if (found_pos != NULL) {
            return ((inttype) (found_pos - main_mem)) + 1;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return 0;
  } /* strRChIPos */



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
          main_stri->size);
      if (found_pos != NULL) {
        return ((inttype) (found_pos - main_mem)) + 1;
      } /* if */
    } /* if */
    return 0;
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
    memsizetype result_size;
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
      if (unlikely(main_size / searched_size + 1 > MAX_STRI_LEN / replace->size)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        guessed_result_size = (main_size / searched_size + 1) * replace->size;
      } /* if */
    } else {
      guessed_result_size = main_size;
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, guessed_result_size))) {
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
            (search_start = search_strelem(search_start, ch_1, search_end)) != NULL) {
          if (memcmp(search_start, searched_mem,
              searched_size * sizeof(strelemtype)) == 0) {
            memcpy(result_end, copy_start,
                (memsizetype) (search_start - copy_start) * sizeof(strelemtype));
            result_end += search_start - copy_start;
            memcpy(result_end, replace->mem,
                replace->size * sizeof(strelemtype));
            result_end += replace->size;
            search_start += searched_size;
            copy_start = search_start;
          } else {
            search_start++;
          } /* if */
        } /* if */
      } /* if */
      memcpy(result_end, copy_start,
          (memsizetype) (&main_stri->mem[main_size] - copy_start) * sizeof(strelemtype));
      result_end += &main_stri->mem[main_size] - copy_start;
      result_size = (memsizetype) (result_end - result->mem);
      /* printf("result=%lu, guessed_result_size=%ld, result_size=%ld\n",
         result, guessed_result_size, result_size); */
      REALLOC_STRI_SIZE_OK(resized_result, result, guessed_result_size, result_size);
      if (unlikely(resized_result == NULL)) {
        FREE_STRI(result, guessed_result_size);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(guessed_result_size, result_size);
        result->size = result_size;
      } /* if */
    } /* if */
    return result;
  } /* strRepl */



#ifdef ANSI_C

inttype strRIPos (const const_stritype main_stri, const const_stritype searched,
    const inttype from_index)
#else

inttype strRIPos (main_stri, searched, from_index)
stritype main_stri;
stritype searched;
inttype from_index;
#endif

  {
    memsizetype main_size;
    memsizetype searched_size;
    strelemtype ch_1;
    const strelemtype *searched_mem;
    const strelemtype *main_mem;
    const strelemtype *search_start;
    const strelemtype *search_end;

  /* strRIPos */
    if (likely(from_index >= 1)) {
      if (unlikely((uinttype) from_index > main_stri->size)) {
        raise_error(RANGE_ERROR);
      } else {
        main_size = main_stri->size;
        searched_size = searched->size;
        if (searched_size != 0 && main_size >= searched_size) {
          searched_mem = searched->mem;
          ch_1 = searched_mem[0];
          main_mem = main_stri->mem;
          if ((uinttype) from_index - 1 <= main_size - searched_size) {
            search_start = &main_mem[from_index - 1];
          } else {
            search_start = &main_mem[main_size - searched_size];
          } /* if */
          search_end = &main_mem[-1];
          while ((search_start = rsearch_strelem(search_start,
              ch_1, (memsizetype) (search_start - search_end))) != NULL) {
            if (memcmp(search_start, searched_mem,
                searched_size * sizeof(strelemtype)) == 0) {
              return ((inttype) (search_start - main_mem)) + 1;
            } else {
              search_start--;
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return 0;
  } /* strRIPos */



#ifdef ANSI_C

stritype strRpad (const const_stritype stri, const inttype pad_size)
#else

stritype strRpad (stri, pad_size)
stritype stri;
inttype pad_size;
#endif

  {
    memsizetype length;
    stritype result;

  /* strRpad */
    length = stri->size;
    if (pad_size > 0 && (uinttype) pad_size > length) {
      if (unlikely((uinttype) pad_size > MAX_STRI_LEN ||
                   !ALLOC_STRI_SIZE_OK(result, (memsizetype) pad_size))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
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
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
      result->size = length;
      memcpy(result->mem, stri->mem, length * sizeof(strelemtype));
    } /* if */
    return result;
  } /* strRpad */



#ifdef ANSI_C

inttype strRPos (const const_stritype main_stri, const const_stritype searched)
#else

inttype strRPos (main_stri, searched)
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

  /* strRPos */
    main_size = main_stri->size;
    searched_size = searched->size;
    if (searched_size != 0 && searched_size <= main_size) {
      searched_mem = searched->mem;
      ch_1 = searched_mem[0];
      main_mem = main_stri->mem;
      search_start = &main_mem[main_size - searched_size];
      search_end = main_mem - 1;
      while ((search_start = rsearch_strelem(search_start,
          ch_1, (memsizetype) (search_start - search_end))) != NULL) {
        if (memcmp(search_start, searched_mem,
            searched_size * sizeof(strelemtype)) == 0) {
          return ((inttype) (search_start - main_mem)) + 1;
        } else {
          search_start--;
        } /* if */
      } /* if */
    } /* if */
    return 0;
  } /* strRPos */



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
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = length;
      memcpy(result->mem, stri->mem, length * sizeof(strelemtype));
      return result;
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
    inttype used_max_position;
    errinfotype err_info = OKAY_NO_ERROR;

    memsizetype main_size;
    const strelemtype *main_mem;
    const strelemtype *search_start;
    const strelemtype *search_end;
    const strelemtype *found_pos;
    memsizetype length;
    stritype dest;

  /* strSplit */
    if (unlikely(!ALLOC_ARRAY(result_array, 256))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result_array->min_position = 1;
      result_array->max_position = 256;
      used_max_position = 0;
      main_size = main_stri->size;
      if (main_size >= 1) {
        main_mem = main_stri->mem;
        search_start = main_mem;
        search_end = &main_mem[main_size];
        while ((found_pos = search_strelem(search_start, delimiter, search_end)) != NULL) {
          add_stri_to_array(search_start, found_pos - search_start,
              result_array, &used_max_position, &err_info);
          search_start = found_pos + 1
        } /* while */
        add_stri_to_array(search_start, search_end - search_start,
            result_array, &used_max_position, &err_info);
      } /* if */

      return result_array;
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
        while ((found_pos = search_strelem(search_start, ch_1, search_end)) != NULL) {
          memcpy(dest, search_start, (memsizetype) (found_pos - search_start));
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
        while ((found_pos = search_strelem(search_start, ch_1, search_end)) != NULL) {
          if (memcmp(search_start, delimiter_mem,
              delimiter_size * sizeof(strelemtype)) == 0) {

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
    inttype used_max_position;
    const strelemtype *search_start;
    const strelemtype *segment_start;
    const strelemtype *search_end;
    const strelemtype *found_pos;
    memsizetype pos;
    rtlArraytype resized_result_array;
    rtlArraytype result_array;

  /* strSplit */
    if (likely(ALLOC_RTL_ARRAY(result_array, 256))) {
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
        while ((found_pos = search_strelem(search_start, ch_1, search_end)) != NULL &&
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
          resized_result_array = REALLOC_RTL_ARRAY(result_array,
              (uinttype) result_array->max_position, (uinttype) used_max_position);
          if (resized_result_array == NULL) {
            for (pos = 0; pos < (uinttype) used_max_position; pos++) {
              FREE_STRI(result_array->arr[pos].value.strivalue,
                  result_array->arr[pos].value.strivalue->size);
            } /* for */
            FREE_RTL_ARRAY(result_array, (uinttype) result_array->max_position);
            result_array = NULL;
          } else {
            result_array = resized_result_array;
            COUNT3_RTL_ARRAY((uinttype) result_array->max_position, (uinttype) used_max_position);
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



#ifdef ALLOW_STRITYPE_SLICES
#ifdef ANSI_C

stritype strSubstrSlice (const const_stritype stri, inttype start, inttype len, stritype slice)
#else

stritype strSubstrSlice (stri, start, len, slice)
stritype stri;
inttype start;
inttype len;
stritype slice;
#endif

  {
    memsizetype length;

  /* strSubstrSlice */
    length = stri->size;
    if (len >= 1 && start > 1 - len && (start < 1 || (uinttype) start <= length) &&
        length >= 1) {
      if (start < 1) {
        len += start - 1;
        start = 1;
      } /* if */
      SET_SLICE_CAPACITY(slice, 0);
      slice->mem = &stri->mem[start - 1];
      if ((uinttype) start + (uinttype) len - 1 > length) {
        slice->size = length - (memsizetype) start + 1;
      } else {
        slice->size = (memsizetype) len;
      } /* if */
    } else {
      SET_SLICE_CAPACITY(slice, 0);
      slice->mem = NULL;
      slice->size = 0;
    } /* if */
    return slice;
  } /* strSubstrSlice */

#else



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
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
      memcpy(result->mem, &stri->mem[start - 1],
          result_size * sizeof(strelemtype));
      result->size = result_size;
    } else {
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memsizetype) 0))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
      result->size = 0;
    } /* if */
    return result;
  } /* strSubstr */

#endif



#ifdef ALLOW_STRITYPE_SLICES
#ifdef ANSI_C

stritype strTailSlice (const const_stritype stri, inttype start, stritype slice)
#else

stritype strTailSlice (stri, start, slice)
stritype stri;
inttype start;
stritype slice;
#endif

  {
    memsizetype length;

  /* strTailSlice */
    length = stri->size;
    if (start < 1) {
      start = 1;
    } /* if */
    if ((uinttype) start <= length && length >= 1) {
      SET_SLICE_CAPACITY(slice, 0);
      slice->mem = &stri->mem[start - 1];
      slice->size = length - (memsizetype) start + 1;
    } else {
      SET_SLICE_CAPACITY(slice, 0);
      slice->mem = NULL;
      slice->size = 0;
    } /* if */
    return slice;
  } /* strTailSlice */

#else



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
    if (start < 1) {
      start = 1;
    } /* if */
    if ((uinttype) start <= length && length >= 1) {
      result_size = length - (memsizetype) start + 1;
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
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
      if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memsizetype) 0))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
      result->size = 0;
    } /* if */
    return result;
  } /* strTail */

#endif



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
    memsizetype result_size;
    stritype resized_result;
    stritype result;

  /* strToUtf8 */
    if (unlikely(stri->size > MAX_STRI_LEN / MAX_UTF8_EXPANSION_FACTOR ||
                 !ALLOC_STRI_SIZE_OK(result, max_utf8_size(stri->size)))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
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
      result_size = (memsizetype) (dest - result->mem);
      REALLOC_STRI_SIZE_OK(resized_result, result, max_utf8_size(stri->size), result_size);
      if (unlikely(resized_result == NULL)) {
        FREE_STRI(result, max_utf8_size(stri->size));
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(max_utf8_size(stri->size), result_size);
        result->size = result_size;
      } /* if */
    } /* if */
    return result;
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
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = length;
      memcpy(result->mem, &stri->mem[start], length * sizeof(strelemtype));
      return result;
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
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = length;
      for (pos = 0; pos < length; pos++) {
        if (stri->mem[pos] >= (strelemtype) 'a' && stri->mem[pos] <= (strelemtype) 'z') {
          result->mem[pos] = stri->mem[pos] - (strelemtype) 'a' + (strelemtype) 'A';
        } else {
          result->mem[pos] = stri->mem[pos];
        } /* if */
      } /* for */
      return result;
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
      if (stri->mem[pos] >= (strelemtype) 'a' && stri->mem[pos] <= (strelemtype) 'z') {
        stri->mem[pos] = stri->mem[pos] - (strelemtype) 'a' + (strelemtype) 'A';
      } /* if */
    } /* for */
    return stri;
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
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, length))) {
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
      if (likely(okay)) {
        REALLOC_STRI_SIZE_OK(resized_result, result, stri8->size, pos);
        if (unlikely(resized_result == NULL)) {
          FREE_STRI(result, stri8->size);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result = resized_result;
          COUNT3_STRI(stri8->size, pos);
          result->size = pos;
        } /* if */
      } else {
        FREE_STRI(result, stri8->size);
        raise_error(RANGE_ERROR);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* strUtf8ToStri */
