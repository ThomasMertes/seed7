/********************************************************************/
/*                                                                  */
/*  str_rtl.c     Primitive actions for the string type.            */
/*  Copyright (C) 1989 - 2005  Thomas Mertes                        */
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



#ifdef WIDE_CHAR_STRINGS
#ifdef ANSI_C

static INLINE int strelem_memcmp (strelemtype *mem1, strelemtype *mem2,
    SIZE_TYPE number)
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

static INLINE strelemtype *search_strelem (strelemtype *mem, strelemtype ch,
    SIZE_TYPE number)
#else

static INLINE strelemtype *search_strelem (mem, ch, number)
strelemtype *mem;
strelemtype ch;
SIZE_TYPE number;
#endif

  { /* search_strelem */
    for (; number > 0; mem++, number--) {
      if (*mem == ch) {
        return(mem);
      } /* if */
    } /* for */
    return(NULL);
  } /* search_strelem */



#else

#define strelem_memcmp(mem1,mem2,len) memcmp(mem1, mem2, (len) * sizeof(strelemtype))
#define search_strelem memchr

#endif



#ifdef ANSI_C

static INLINE strelemtype *rsearch_strelem (strelemtype *mem, strelemtype ch,
    SIZE_TYPE number)
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

void strAppend (stritype *stri_to, stritype stri_from)
#else

void strAppend (stri_to, stri_from)
stritype *stri_to;
stritype stri_from;
#endif

  {
    memsizetype new_size;
    stritype stri_dest;

  /* strAppend */
    if (stri_from->size != 0) {
      stri_dest = *stri_to;
      new_size = stri_dest->size + stri_from->size;
      if (!RESIZE_STRI(stri_dest, stri_dest->size, new_size)) {
        raise_error(MEMORY_ERROR);
        return;
      } /* if */
      COUNT3_STRI(stri_dest->size, new_size);
      memcpy(&stri_dest->mem[stri_dest->size], stri_from->mem,
          (SIZE_TYPE) stri_from->size * sizeof(strelemtype));
      stri_dest->size = new_size;
      *stri_to = stri_dest;
    } /* if */
  } /* strAppend */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

stritype strCLit (stritype str1)
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
    stritype result;

  /* strCLit */
    length = str1->size;
    if (!ALLOC_STRI(result, (memsizetype) (4 * length + 2))) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } /* if */
    COUNT_STRI(4 * length + 2);
    result->mem[0] = (strelemtype) '"';
    pos = 1;
    for (position = 0; position < length; position++) {
      character = (int) str1->mem[position];
      /* The following comparison uses 255 instead of '\377',       */
      /* because chars might be signed and this can produce wrong   */
      /* code when '\377' is sign extended.                         */
      if (character > 255) {
        result->mem[pos] = (strelemtype) '?';
        pos++;
      } else if (no_escape_char(character)) {
        result->mem[pos] = (strelemtype) character;
        pos++;
      } else if (character < ' ') {
        len = strlen(cstri_escape_sequence[character]);
        stri_expand(&result->mem[pos],
            cstri_escape_sequence[character], len);
        pos = pos + len;
      } else if (character <= '~') {
        result->mem[pos] = (strelemtype) '\\';
        result->mem[pos + 1] = (strelemtype) character;
        pos = pos + 2;
      } else if (character == '\177') {
        stri_expand(&result->mem[pos],
            "\\177", (SIZE_TYPE) 4);
        pos = pos + 4;
      } else {
        result->mem[pos] = (strelemtype) character;
        pos++;
      } /* if */
    } /* for */
    result->mem[pos] = (strelemtype) '"';
    result->size = pos + 1;
    if (!RESIZE_STRI(result, (memsizetype) (4 * length + 2),
        (memsizetype) (pos + 1))) {
      FREE_STRI(result, (memsizetype) (4 * length + 2));
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT3_STRI(4 * length + 2, pos + 1);
      return(result);
    } /* if */
  } /* strCLit */
#endif



#ifdef ANSI_C

inttype strCompare (stritype stri1, stritype stri2)
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

stritype strConcat (stritype stri1, stritype stri2)
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
      COUNT_STRI(result_size);
      result->size = result_size;
      memcpy(result->mem, stri1->mem,
          (SIZE_TYPE) stri1->size * sizeof(strelemtype));
      memcpy(&result->mem[stri1->size], stri2->mem,
          (SIZE_TYPE) stri2->size * sizeof(strelemtype));
      return(result);
    } /* if */
  } /* strConcat */



#ifdef ANSI_C

void strCopy (stritype *stri_to, stritype stri_from)
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
        COUNT_STRI(new_size);
        FREE_STRI(*stri_to, (*stri_to)->size);
        stri_dest->size = new_size;
        *stri_to = stri_dest;
      } /* if */
    } /* if */
    memcpy(stri_dest->mem, stri_from->mem,
        (SIZE_TYPE) new_size * sizeof(strelemtype));
  } /* strCopy */



#ifdef ANSI_C

void strCreate (stritype *stri_to, stritype stri_from)
#else

void strCreate (stri_to, stri_from)
stritype *stri_to;
stritype stri_from;
#endif

  {
    memsizetype new_size;
    stritype new_str;

  /* strCreate */
    new_size = stri_from->size;
    if (!ALLOC_STRI(new_str, new_size)) {
      *stri_to = NULL;
      raise_error(MEMORY_ERROR);
      return;
    } else {
      COUNT_STRI(new_size);
      new_str->size = new_size;
      memcpy(new_str->mem, stri_from->mem,
          (SIZE_TYPE) new_size * sizeof(strelemtype));
      *stri_to = new_str;
    } /* if */
  } /* strCreate */



#ifdef ANSI_C

void strDestr (stritype old_string)
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

booltype strGe (stritype stri1, stritype stri2)
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

booltype strGt (stritype stri1, stritype stri2)
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

inttype strHashCode (stritype stri)
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

stritype strHead (stritype stri, inttype stop)
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
      COUNT_STRI(result_size);
      result->size = result_size;
      memcpy(result->mem, stri->mem,
          (SIZE_TYPE) result_size * sizeof(strelemtype));
    } else {
      if (!ALLOC_STRI(result, (memsizetype) 0)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      COUNT_STRI(0);
      result->size = 0;
    } /* if */
    return(result);
  } /* strHead */



#ifdef ANSI_C

inttype strIpos (stritype main_stri, stritype searched, inttype from_index)
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
    strelemtype *main_mem;
    strelemtype *searched_mem;
    strelemtype *search_start;
    strelemtype *search_end;

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
        main_size -= from_index;
        search_start = main_mem;
        search_end = &main_mem[main_size - searched_size + 1];
        while ((search_start = (strelemtype *) search_strelem(search_start,
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

booltype strLe (stritype stri1, stritype stri2)
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

stritype strLit (stritype stri)
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
    uchartype buffer[25];
    stritype result;

  /* strLit */
    length = stri->size;
    if (!ALLOC_STRI(result, (memsizetype) (12 * length + 2))) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } /* if */
    COUNT_STRI(12 * length + 2);
    result->mem[0] = (strelemtype) '"';
    pos = 1;
    for (position = 0; position < length; position++) {
      character = (int) stri->mem[position];
      if (character < ' ') {
        len = strlen(stri_escape_sequence[character]);
        stri_expand(&result->mem[pos],
            stri_escape_sequence[character], len);
        pos = pos + len;
      } else if (character >= '\177' && character < '\237') {
        sprintf(buffer, "\\%d\\", (int) character);
        len = strlen(buffer);
        stri_expand(&result->mem[pos], buffer, len);
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
    if (!RESIZE_STRI(result, (memsizetype) (5 * length + 2),
        (memsizetype) (pos + 1))) {
      FREE_STRI(result, (memsizetype) (5 * length + 2));
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT3_STRI(5 * length + 2, pos + 1);
      return(result);
    } /* if */
  } /* strLit */



#ifdef ANSI_C

stritype strLow (stritype stri)
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
      COUNT_STRI(length);
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

stritype strLpad (stritype stri, inttype pad_size)
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
        return(NULL);
      } /* if */
      COUNT_STRI(f_size);
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
    } else {
      if (!ALLOC_STRI(result, length)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      COUNT_STRI(length);
      result->size = length;
      memcpy(result->mem, stri->mem,
          (SIZE_TYPE) length * sizeof(strelemtype));
    } /* if */
    return(result);
  } /* strLpad */



#ifdef ANSI_C

booltype strLt (stritype stri1, stritype stri2)
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

stritype strMult (stritype stri, inttype factor)
#else

stritype strMult (stri, factor)
stritype stri;
inttype factor;
#endif

  {
    memsizetype len;
    inttype number;
    strelemtype *result_pointer;
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
        COUNT_STRI(result_size);
        result->size = result_size;
        if (len != 0) {
          if (len == 1) {
#ifdef WIDE_CHAR_STRINGS
            result_pointer = result->mem;
            for (number = factor; number > 0; number--) {
              *result_pointer++ = stri->mem[0];
            } /* for */
#else
            memset(result->mem, (int) stri->mem[0], (SIZE_TYPE) factor);
#endif
          } else {
            result_pointer = result->mem;
            for (number = factor; number > 0; number--) {
              memcpy(result_pointer, stri->mem,
                  (SIZE_TYPE) len * sizeof(strelemtype));
              result_pointer += (SIZE_TYPE) len;
            } /* for */
          } /* if */
        } /* if */
        return(result);
      } /* if */
    } /* if */
  } /* strMult */



#ifdef ANSI_C

inttype strPos (stritype main_stri, stritype searched)
#else

inttype strPos (main_stri, searched)
stritype main_stri;
stritype searched;
#endif

  {
    memsizetype main_size;
    memsizetype searched_size;
    strelemtype ch_1;
    strelemtype *main_mem;
    strelemtype *searched_mem;
    strelemtype *search_start;
    strelemtype *search_end;

  /* strPos */
    main_size = main_stri->size;
    searched_size = searched->size;
    if (searched_size != 0 && searched_size <= main_size) {
      searched_mem = searched->mem;
      ch_1 = searched_mem[0];
      main_mem = main_stri->mem;
      search_start = main_mem;
      search_end = &main_mem[main_size - searched_size + 1];
      while ((search_start = (strelemtype *) search_strelem(search_start,
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

stritype strRange (stritype stri, inttype start, inttype stop)
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
      COUNT_STRI(result_size);
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
      COUNT_STRI(0);
      result->size = 0;
    } /* if */
    return(result);
  } /* strRange */



#ifdef ANSI_C

stritype strRepl (stritype main_stri, stritype searched, stritype replace)
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
    strelemtype *main_mem;
    strelemtype *searched_mem;
    strelemtype *search_start;
    strelemtype *search_end;
    strelemtype *copy_start;
    strelemtype *result_end;
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
      return(NULL);
    } /* if */
    COUNT_STRI(guessed_result_size);
    copy_start = main_stri->mem;
    result_end = result->mem;
    if (searched_size != 0 && searched_size <= main_size) {
      searched_mem = searched->mem;
      ch_1 = searched_mem[0];
      main_mem = main_stri->mem;
      search_start = main_mem;
      search_end = &main_mem[main_size - searched_size + 1];
      while (search_start < search_end &&
          (search_start = (strelemtype *) search_strelem(search_start,
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
    if (!RESIZE_STRI(result, guessed_result_size, result->size)) {
      FREE_STRI(result, guessed_result_size);
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT3_STRI(guessed_result_size, result->size);
      return(result);
    } /* if */
  } /* strRepl */



#ifdef ANSI_C

stritype strRpad (stritype stri, inttype pad_size)
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
      COUNT_STRI(f_size);
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
      COUNT_STRI(length);
      result->size = length;
      memcpy(result->mem, stri->mem,
          (SIZE_TYPE) length * sizeof(strelemtype));
    } /* if */
    return(result);
  } /* strRpad */



#ifdef ANSI_C

inttype strRpos (stritype main_stri, stritype searched)
#else

inttype strRpos (main_stri, searched)
stritype main_stri;
stritype searched;
#endif

  {
    memsizetype main_size;
    memsizetype searched_size;
    strelemtype ch_1;
    strelemtype *main_mem;
    strelemtype *searched_mem;
    strelemtype *search_start;
    strelemtype *search_end;

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



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

void add_stri_to_array (strelemtype *stri_elems, memsizetype length,
    arraytype *work_array, memsizetype *used_max_position, errinfotype *err_info)
#else

void add_stri_to_array (stri_elems, length,
    work_array, used_max_position, err_info)
strelemtype *stri_elems;
memsizetype length;
arraytype *work_array;
memsizetype *used_max_position;
errinfotype *err_info;
#endif

  {
    stritype new_stri;
    memsizetype position;

  /* add_stri_to_array */
    if (ALLOC_STRI(new_stri, length)) {
      COUNT_STRI(length);
      new_stri->size = length;
      memcpy(new_stri->mem, stri_elems, length);
      if (*used_max_position >= (*work_array)->max_position) {
        if (!RESIZE_ARRAY(*work_array, (*work_array)->max_position,
            (*work_array)->max_position + 256)) {
          FREE_STRI(new_stri, new_stri->size);
          *err_info = MEMORY_ERROR;
        } else {
          COUNT3_ARRAY((*work_array)->max_position, (*work_array)->max_position + 256);
          (*work_array)->max_position += 256;
        } /* if */
      } /* if */
      if (*err_info == OKAY_NO_ERROR) {
        (*work_array)->arr[*used_max_position].type_of = take_type(SYS_STRI_TYPE);
        (*work_array)->arr[*used_max_position].descriptor.entity = NULL;
        (*work_array)->arr[*used_max_position].value.strivalue = new_stri;
        INIT_CLASS_OF_VAR(&(*work_array)->arr[*used_max_position], STRIOBJECT);
        (*used_max_position)++;
      } else {
        for (position = 0; position < *used_max_position; position++) {
          FREE_STRI((*work_array)->arr[position].value.strivalue,
              (*work_array)->arr[position].value.strivalue->size);
        } /* for */
        FREE_ARRAY(*work_array, max_array_size);
        *work_array = NULL;
      } /* if */
    } /* if */
  } /* add_stri_to_array */



#ifdef ANSI_C

arraytype strSplit (stritype main_stri, chartype delimiter)
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
    strelemtype *main_mem;
    strelemtype *search_start;
    strelemtype *search_end;
    strelemtype *found_pos;
    memsizetype length;
    stritype dest;

  /* strSplit */
    if (ALLOC_ARRAY(result_array, 256)) {
      COUNT_ARRAY(256);
      result_array->min_position = 1;
      result_array->max_position = 256;
      used_max_position = 0;
      main_size = main_stri->size;
      if (main_size >= 1) {
        main_mem = main_stri->mem;
        search_start = main_mem;
        search_end = &main_mem[main_size];
        while ((found_pos = (strelemtype *) search_strelem(search_start,
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

arraytype strSplit (stritype stri, stritype delimiter)
#else

arraytype strSplit (stri, delimiter)
stritype stri;
stritype delimiter;
#endif

  {
    memsizetype main_size;
    memsizetype delimiter_size;
    strelemtype ch_1;
    strelemtype *main_mem;
    strelemtype *delimiter_mem;
    strelemtype *search_start;
    strelemtype *search_end;
    strelemtype *found_pos;
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
        while ((found_pos = (strelemtype *) search_strelem(search_start,
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
        while ((found_pos = (strelemtype *) search_strelem(search_start,
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

stritype strSubstr (stritype stri, inttype start, inttype len)
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
      COUNT_STRI(result_size);
      memcpy(result->mem, &stri->mem[start - 1],
          (SIZE_TYPE) result_size * sizeof(strelemtype));
      result->size = result_size;
    } else {
      if (!ALLOC_STRI(result, (memsizetype) 0)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } /* if */
      COUNT_STRI(0);
      result->size = 0;
    } /* if */
    return(result);
  } /* strSubstr */



#ifdef ANSI_C

stritype strTail (stritype stri, inttype start)
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
      COUNT_STRI(result_size);
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
      COUNT_STRI(0);
      result->size = 0;
    } /* if */
    return(result);
  } /* strTail */



#ifdef ANSI_C

stritype strTrim (stritype stri)
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
      COUNT_STRI(length);
      result->size = length;
      memcpy(result->mem, &stri->mem[start],
          (SIZE_TYPE) length * sizeof(strelemtype));
      return(result);
    } /* if */
  } /* strTrim */



#ifdef ANSI_C

stritype strUp (stritype stri)
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
      COUNT_STRI(length);
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
