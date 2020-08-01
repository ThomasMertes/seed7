/********************************************************************/
/*                                                                  */
/*  striutl.c     Procedures to work with wide char strings.        */
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
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/striutl.c                                       */
/*  Changes: 1991, 1992, 1993, 1994, 2005  Thomas Mertes            */
/*  Content: Procedures to work with wide char strings.             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

#include "common.h"
#include "heaputl.h"

#undef EXTERN
#define EXTERN
#include "striutl.h"


const_cstritype cstri_escape_sequence[] = {
    "\\000", "\\001", "\\002", "\\003", "\\004",
    "\\005", "\\006", "\\007", "\\b",   "\\t",
    "\\n",   "\\013", "\\f",   "\\r",   "\\016",
    "\\017", "\\020", "\\021", "\\022", "\\023",
    "\\024", "\\025", "\\026", "\\027", "\\030",
    "\\031", "\\032", "\\033", "\\034", "\\035",
    "\\036", "\\037"};



#ifdef ANSI_C

memsizetype utf8_to_stri (strelemtype *dest_stri, memsizetype *dest_len,
    const_ustritype ustri, size_t len)
#else

memsizetype utf8_to_stri (dest_stri, dest_len, ustri, len)
strelemtype *dest_stri;
memsizetype *dest_len;
ustritype ustri;
size_t len;
#endif

  {
    strelemtype *stri;

  /* utf8_to_stri */
    stri = dest_stri;
    for (; len > 0; len--) {
      if (*ustri <= 0x7F) {
        *stri++ = (strelemtype) *ustri++;
      } else if ((ustri[0] & 0xE0) == 0xC0 && len > 1 &&
                 (ustri[1] & 0xC0) == 0x80) {
        /* ustri[0]   range 0xC0 to 0xDF (192 to 223) */
        /* ustri[1]   range 0x80 to 0xBF (128 to 191) */
        *stri++ = (strelemtype) (ustri[0] & 0x1F) << 6 |
                  (strelemtype) (ustri[1] & 0x3F);
        ustri += 2;
        len--;
      } else if ((ustri[0] & 0xF0) == 0xE0 && len > 2 &&
                 (ustri[1] & 0xC0) == 0x80 &&
                 (ustri[2] & 0xC0) == 0x80) {
        /* ustri[0]   range 0xE0 to 0xEF (224 to 239) */
        /* ustri[1..] range 0x80 to 0xBF (128 to 191) */
        *stri++ = (strelemtype) (ustri[0] & 0x0F) << 12 |
                  (strelemtype) (ustri[1] & 0x3F) <<  6 |
                  (strelemtype) (ustri[2] & 0x3F);
        ustri += 3;
        len -= 2;
      } else if ((ustri[0] & 0xF8) == 0xF0 && len > 3 &&
                 (ustri[1] & 0xC0) == 0x80 &&
                 (ustri[2] & 0xC0) == 0x80 &&
                 (ustri[3] & 0xC0) == 0x80) {
        /* ustri[0]   range 0xF0 to 0xF7 (240 to 247) */
        /* ustri[1..] range 0x80 to 0xBF (128 to 191) */
        *stri++ = (strelemtype) (ustri[0] & 0x07) << 18 |
                  (strelemtype) (ustri[1] & 0x3F) << 12 |
                  (strelemtype) (ustri[2] & 0x3F) <<  6 |
                  (strelemtype) (ustri[3] & 0x3F);
        ustri += 4;
        len -= 3;
      } else if ((ustri[0] & 0xFC) == 0xF8 && len > 4 &&
                 (ustri[1] & 0xC0) == 0x80 &&
                 (ustri[2] & 0xC0) == 0x80 &&
                 (ustri[3] & 0xC0) == 0x80 &&
                 (ustri[4] & 0xC0) == 0x80) {
        /* ustri[0]   range 0xF8 to 0xFB (248 to 251) */
        /* ustri[1..] range 0x80 to 0xBF (128 to 191) */
        *stri++ = (strelemtype) (ustri[0] & 0x03) << 24 |
                  (strelemtype) (ustri[1] & 0x3F) << 18 |
                  (strelemtype) (ustri[2] & 0x3F) << 12 |
                  (strelemtype) (ustri[3] & 0x3F) <<  6 |
                  (strelemtype) (ustri[4] & 0x3F);
        ustri += 5;
        len -= 4;
      } else if ((ustri[0] & 0xFC) == 0xFC && len > 5 &&
                 (ustri[1] & 0xC0) == 0x80 &&
                 (ustri[2] & 0xC0) == 0x80 &&
                 (ustri[3] & 0xC0) == 0x80 &&
                 (ustri[4] & 0xC0) == 0x80 &&
                 (ustri[5] & 0xC0) == 0x80) {
        /* ustri[0]   range 0xFC to 0xFF (252 to 255) */
        /* ustri[1..] range 0x80 to 0xBF (128 to 191) */
        *stri++ = (strelemtype) (ustri[0] & 0x03) << 30 |
                  (strelemtype) (ustri[1] & 0x3F) << 24 |
                  (strelemtype) (ustri[2] & 0x3F) << 18 |
                  (strelemtype) (ustri[3] & 0x3F) << 12 |
                  (strelemtype) (ustri[4] & 0x3F) <<  6 |
                  (strelemtype) (ustri[5] & 0x3F);
        ustri += 6;
        len -= 5;
      } else {
        /* ustri[0] not in range 0xC0 to 0xFF (192 to 255) */
        *dest_len = (memsizetype) (stri - dest_stri);
        return len;
      } /* if */
    } /* for */
    *dest_len = (memsizetype) (stri - dest_stri);
    return 0;
  } /* utf8_to_stri */



#ifdef ANSI_C

memsizetype utf8_bytes_missing (const_ustritype ustri, size_t len)
#else

memsizetype utf8_bytes_missing (ustri, len)
ustritype ustri;
size_t len;
#endif

  {
    memsizetype result;

  /* utf8_bytes_missing */
    result = 0;
    if (len >= 1 && *ustri > 0x7F) {
      if ((ustri[0] & 0xE0) == 0xC0) {
        if (len == 1) {
          result = 1;
        } /* if */
      } else if ((ustri[0] & 0xF0) == 0xE0) {
        if (len == 1) {
          result = 2;
        } else if ((ustri[1] & 0xC0) == 0x80) {
          if (len == 2) {
            result = 1;
          } /* if */
        } /* if */
      } else if ((ustri[0] & 0xF8) == 0xF0) {
        if (len == 1) {
          result = 3;
        } else if ((ustri[1] & 0xC0) == 0x80) {
          if (len == 2) {
            result = 2;
          } else if ((ustri[2] & 0xC0) == 0x80) {
            if (len == 3) {
              result = 1;
            } /* if */
          } /* if */
        } /* if */
      } else if ((ustri[0] & 0xFC) == 0xF8) {
        if (len == 1) {
          result = 4;
        } else if ((ustri[1] & 0xC0) == 0x80) {
          if (len == 2) {
            result = 3;
          } else if ((ustri[2] & 0xC0) == 0x80) {
            if (len == 3) {
              result = 2;
            } else if ((ustri[3] & 0xC0) == 0x80) {
              if (len == 4) {
                result = 1;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } else if ((ustri[0] & 0xFC) == 0xFC) {
        if (len == 1) {
          result = 5;
        } else if ((ustri[1] & 0xC0) == 0x80) {
          if (len == 2) {
            result = 4;
          } else if ((ustri[2] & 0xC0) == 0x80) {
            if (len == 3) {
              result = 3;
            } else if ((ustri[3] & 0xC0) == 0x80) {
              if (len == 4) {
                result = 2;
              } else if ((ustri[4] & 0xC0) == 0x80) {
                if (len == 5) {
                  result = 1;
                } /* if */
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* utf8_bytes_missing */



#ifdef UTF32_STRINGS
#ifdef ANSI_C

memsizetype stri_to_utf8 (ustritype out_stri, register const strelemtype *stri,
    memsizetype len)
#else

memsizetype stri_to_utf8 (out_stri, stri, len)
ustritype out_stri;
strelemtype *stri;
memsizetype len;
#endif

  {
    register ustritype ustri;

  /* stri_to_utf8 */
    ustri = out_stri;
    for (; len > 0; stri++, len--) {
      if (*stri <= 0x7F) {
        *ustri++ = (uchartype) *stri;
      } else if (*stri <= 0x7FF) {
        *ustri++ = (uchartype) (0xC0 | (*stri >>  6));
        *ustri++ = (uchartype) (0x80 |( *stri        & 0x3F));
      } else if (*stri <= 0xFFFF) {
        *ustri++ = (uchartype) (0xE0 | (*stri >> 12));
        *ustri++ = (uchartype) (0x80 |((*stri >>  6) & 0x3F));
        *ustri++ = (uchartype) (0x80 |( *stri        & 0x3F));
      } else if (*stri <= 0x1FFFFF) {
        *ustri++ = (uchartype) (0xF0 | (*stri >> 18));
        *ustri++ = (uchartype) (0x80 |((*stri >> 12) & 0x3F));
        *ustri++ = (uchartype) (0x80 |((*stri >>  6) & 0x3F));
        *ustri++ = (uchartype) (0x80 |( *stri        & 0x3F));
      } else if (*stri <= 0x3FFFFFF) {
        *ustri++ = (uchartype) (0xF8 | (*stri >> 24));
        *ustri++ = (uchartype) (0x80 |((*stri >> 18) & 0x3F));
        *ustri++ = (uchartype) (0x80 |((*stri >> 12) & 0x3F));
        *ustri++ = (uchartype) (0x80 |((*stri >>  6) & 0x3F));
        *ustri++ = (uchartype) (0x80 |( *stri        & 0x3F));
      } else {
        *ustri++ = (uchartype) (0xFC | (*stri >> 30));
        *ustri++ = (uchartype) (0x80 |((*stri >> 24) & 0x3F));
        *ustri++ = (uchartype) (0x80 |((*stri >> 18) & 0x3F));
        *ustri++ = (uchartype) (0x80 |((*stri >> 12) & 0x3F));
        *ustri++ = (uchartype) (0x80 |((*stri >>  6) & 0x3F));
        *ustri++ = (uchartype) (0x80 |( *stri        & 0x3F));
      } /* if */
    } /* for */
    return (memsizetype) (ustri - out_stri);
  } /* stri_to_utf8 */



#ifdef ANSI_C

void ustri_expand (strelemtype *stri, const_ustritype ustri, size_t len)
#else

void ustri_expand (stri, ustri, len)
strelemtype *stri;
ustritype ustri;
size_t len;
#endif

  { /* ustri_expand */
    for (; len > 0; stri++, ustri++, len--) {
      *stri = (strelemtype) *ustri;
    } /* while */
  } /* ustri_expand */



#ifdef ANSI_C

void stri_compress (ustritype ustri, const strelemtype *stri, size_t len)
#else

void stri_compress (ustri, stri, len)
ustritype ustri;
strelemtype *stri;
size_t len;
#endif

  { /* stri_compress */
    for (; len > 0; stri++, ustri++, len--) {
      *ustri = (uchartype) *stri;
    } /* for */
  } /* stri_compress */



#ifdef ANSI_C

void stri_export (ustritype out_stri, const_stritype in_stri)
#else

void stri_export (out_stri, in_stri)
ustritype out_stri;
stritype in_stri;
#endif

  {
    memsizetype len;

  /* stri_export */
    len = stri_to_utf8(out_stri, in_stri->mem, in_stri->size);
    out_stri[len] = '\0';
  } /* stri_export */

#endif



#ifdef OS_PATH_WCHAR
#ifdef ANSI_C

static void wstri_expand (strelemtype *stri, const_wstritype wstri, size_t len)
#else

static void wstri_expand (stri, wstri, len)
strelemtype *stri;
wstritype wstri;
size_t len;
#endif

  { /* wstri_expand */
    for (; len > 0; stri++, wstri++, len--) {
      *stri = (strelemtype) *wstri;
    } /* while */
  } /* wstri_expand */



#ifdef ANSI_C

static stritype wstri_to_stri (const_wstritype wstri)
#else

static stritype wstri_to_stri (wstri)
wstritype wstri;
#endif

  {
    memsizetype length;
    stritype stri;

  /* wstri_to_stri */
    length = 0;
    while (wstri[length] != 0) {
      length++;
    } /* while */
    if (ALLOC_STRI_CHECK_SIZE(stri, length)) {
      stri->size = length;
      wstri_expand(stri->mem, wstri, (size_t) length);
    } /* if */
    return stri;
  } /* wstri_to_stri */



#ifdef ANSI_C

static void stri_to_wstri (wstritype wstri, const_stritype stri,
    errinfotype *err_info)
#else

static void stri_to_wstri (wstri, stri, err_info)
wstritype wstri;
stritype stri;
errinfotype *err_info;
#endif

  {
    const strelemtype *strelem;
    memsizetype len;

  /* stri_to_wstri */
    strelem = stri->mem;
    len = stri->size;
    for (; len > 0; wstri++, strelem++, len--) {
      if (likely(*strelem <= 0xFFFF)) {
        *wstri = (uint16type) *strelem;
      } else if (*strelem <= 0x10FFFF) {
        strelemtype currChar = *strelem - 0x10000;
        *wstri = (uint16type) (0xD800 | (currChar >> 10));
        wstri++;
        *wstri = (uint16type) (0xDC00 | (currChar & 0x3FF));
      } else {
        *err_info = RANGE_ERROR;
        len = 1;
      } /* if */
    } /* for */
    *wstri = 0;
  } /* stri_to_wstri */



#ifdef ANSI_C

os_stritype stri_to_os_stri (const_stritype stri, errinfotype *err_info)
#else

os_stritype stri_to_os_stri (stri, err_info)
const_stritype stri;
errinfotype *err_info;
#endif

  {
    os_stritype result;

  /* stri_to_os_stri */
    if (unlikely(stri->size > ((MAX_MEMSIZETYPE / sizeof(os_chartype)) - 1) / 2)) {
      *err_info = MEMORY_ERROR;
      result = NULL;
    } else {
      result = (os_stritype) malloc(sizeof(os_chartype) * (stri->size * 2 + 1));
      if (unlikely(result == NULL)) {
        *err_info = MEMORY_ERROR;
      } else {
        stri_to_wstri(result, stri, err_info);
        if (unlikely(*err_info != OKAY_NO_ERROR)) {
          os_stri_free(result);
          result = NULL;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* stri_to_os_stri */

#else



#ifdef ANSI_C

os_stritype stri_to_os_stri (const_stritype stri, errinfotype *err_info)
#else

os_stritype stri_to_os_stri (stri, err_info)
const_stritype stri;
errinfotype *err_info;
#endif

  {
    os_stritype result;

  /* stri_to_os_stri */
    if (unlikely(stri->size > MAX_CSTRI_LEN / MAX_UTF8_EXPANSION_FACTOR ||
                 !ALLOC_CSTRI(result, max_utf8_size(stri)))) {
      *err_info = MEMORY_ERROR;
      result = NULL;
    } else {
      stri_export((ustritype) result, stri);
    } /* if */
    return result;
  } /* stri_to_os_stri */

#endif



#ifdef ANSI_C

cstritype cp_to_cstri (const_stritype stri)
#else

cstritype cp_to_cstri (stri)
stritype stri;
#endif

  {
    cstritype cstri;

  /* cp_to_cstri */
    if (stri->size > MAX_CSTRI_LEN / MAX_UTF8_EXPANSION_FACTOR) {
      cstri = NULL;
    } else if (ALLOC_CSTRI(cstri, max_utf8_size(stri))) {
      stri_export((ustritype) cstri, stri);
    } /* if */
    return cstri;
  } /* cp_to_cstri */



#ifdef ANSI_C

bstritype stri_to_bstri (const_stritype stri)
#else

bstritype stri_to_bstri (stri)
stritype stri;
#endif

  {
    register const strelemtype *str;
    register uchartype *ustri;
    register memsizetype len;
    bstritype resized_bstri;
    bstritype bstri;

  /* stri_to_bstri */
    if (ALLOC_BSTRI_CHECK_SIZE(bstri, stri->size)) {
      bstri->size = stri->size;
#ifdef UTF32_STRINGS
      for (str = stri->mem, ustri = bstri->mem, len = stri->size;
          len > 0; str++, ustri++, len--) {
        if (*str >= 256) {
          bstri->size -= len;
          REALLOC_BSTRI_SIZE_OK(resized_bstri, bstri, stri->size, bstri->size);
          if (resized_bstri == NULL) {
            FREE_BSTRI(bstri, stri->size);
            bstri = NULL;
          } else {
            bstri = resized_bstri;
            COUNT3_BSTRI(stri->size, bstri->size);
          } /* if */
          return bstri;
        } /* if */
        *ustri = (uchartype) *str;
      } /* for */
#else
      memcpy(bstri->mem, stri->mem, stri->size);
#endif
    } /* if */
    return bstri;
  } /* stri_to_bstri */



#ifdef ANSI_C

bstritype stri_to_bstri8 (const_stritype stri)
#else

bstritype stri_to_bstri8 (stri)
stritype stri;
#endif

  {
    bstritype resized_bstri;
    bstritype bstri;

  /* stri_to_bstri8 */
    if (stri->size > MAX_BSTRI_LEN / MAX_UTF8_EXPANSION_FACTOR) {
      bstri = NULL;
    } else if (ALLOC_BSTRI_SIZE_OK(bstri, max_utf8_size(stri))) {
#ifdef UTF32_STRINGS
      bstri->size = stri_to_utf8(bstri->mem, stri->mem, stri->size);
#else
      memcpy(bstri->mem, stri->mem, stri->size);
      bstri->size = stri->size;
#endif
      REALLOC_BSTRI_SIZE_OK(resized_bstri, bstri, max_utf8_size(stri), bstri->size);
      if (resized_bstri == NULL) {
        FREE_BSTRI(bstri, max_utf8_size(stri));
        bstri = NULL;
      } else {
        bstri = resized_bstri;
        COUNT3_BSTRI(max_utf8_size(stri), bstri->size);
      } /* if */
    } /* if */
    return bstri;
  } /* stri_to_bstri8 */



#ifdef ANSI_C

stritype cstri_to_stri (const_cstritype cstri)
#else

stritype cstri_to_stri (cstri)
cstritype cstri;
#endif

  {
    memsizetype length;
    stritype result;

  /* cstri_to_stri */
    length = strlen(cstri);
    if (ALLOC_STRI_CHECK_SIZE(result, length)) {
      result->size = length;
      cstri_expand(result->mem, cstri, length);
    } /* if */
    return result;
  } /* cstri_to_stri */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

stritype cstri_to_stri (const_cstritype cstri)
#else

stritype cstri_to_stri (cstri)
cstritype cstri;
#endif

  {
    register strelemtype *stri;
    register const_ustritype ustri;
    memsizetype length;
    stritype result;

  /* cstri_to_stri */
    length = strlen(cstri);
    if (ALLOC_STRI_CHECK_SIZE(result, length)) {
      result->size = length;
      stri = result->mem;
      ustri = (const_ustritype) cstri;
      for (; *ustri != '\0'; stri++, ustri++) {
        *stri = (strelemtype) *ustri;
      } /* while */
    } /* if */
    return result;
  } /* cstri_to_stri */
#endif



#ifdef ANSI_C

static stritype cstri8_to_stri (const_cstritype cstri)
#else

static stritype cstri8_to_stri (cstri)
cstritype cstri;
#endif

  {
    memsizetype length;
    memsizetype stri_size;
    stritype resized_stri;
    stritype stri;

  /* cstri8_to_stri */
    length = strlen(cstri);
    if (ALLOC_STRI_CHECK_SIZE(stri, length)) {
      if (utf8_to_stri(stri->mem, &stri_size, (const_ustritype) cstri, length) == 0) {
        REALLOC_STRI_SIZE_OK(resized_stri, stri, length, stri_size);
        if (resized_stri == NULL) {
          FREE_STRI(stri, length);
          stri = NULL;
        } else {
          stri = resized_stri;
          COUNT3_STRI(length, stri_size);
          stri->size = stri_size;
        } /* if */
      } else {
        FREE_STRI(stri, length);
        stri = NULL;
      } /* if */
    } /* if */
    return stri;
  } /* cstri8_to_stri */



#ifdef ANSI_C

stritype cstri8_or_cstri_to_stri (const_cstritype cstri)
#else

stritype cstri8_or_cstri_to_stri (cstri)
cstritype cstri;
#endif

  {
    stritype stri;

  /* cstri8_or_cstri_to_stri */
    stri = cstri8_to_stri(cstri);
    if (stri == NULL) {
      stri = cstri_to_stri(cstri);
    } /* if */
    return stri;
  } /* cstri8_or_cstri_to_stri */



#ifdef ANSI_C

stritype os_stri_to_stri (const_os_stritype os_stri)
#else

stritype os_stri_to_stri (os_stri)
os_stritype os_stri;
#endif

  {
    stritype stri;

  /* os_stri_to_stri */
#ifdef OS_PATH_WCHAR
    stri = wstri_to_stri(os_stri);
#else
#ifdef OS_PATH_UTF8
    stri = cstri8_or_cstri_to_stri(os_stri);
#else
    stri = cstri_to_stri(os_stri);
#endif
#endif
    return stri;
  } /* os_stri_to_stri */



#ifdef ANSI_C

stritype stri_to_standard_path (stritype stri)
#else

stritype stri_to_standard_path (stri)
stritype stri;
#endif

  {
    stritype result;

  /* stri_to_standard_path */
    result = stri;
    if (result != NULL) {
#if PATH_DELIMITER != '/'
      {
        unsigned int pos;

        for (pos = 0; pos < result->size; pos++) {
          if (result->mem[pos] == PATH_DELIMITER) {
            result->mem[pos] = (strelemtype) '/';
          } /* if */
        } /* for */
      }
#endif
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (result->size >= 2 && result->mem[1] == ':' &&
          ((result->mem[0] >= 'a' && result->mem[0] <= 'z') ||
           (result->mem[0] >= 'A' && result->mem[0] <= 'Z'))) {
        if (result->size >= 3 && result->mem[2] == '/') {
          result->mem[1] = (strelemtype) tolower((int) result->mem[0]);
          result->mem[0] = (strelemtype) '/';
          if (result->size == 3) {
            result->size = 2;
          } /* if */
        } /* if */
      } /* if */
#endif
    } /* if */
    return result;
  } /* stri_to_standard_path */



#ifdef ANSI_C

stritype cp_from_os_path (const_os_stritype os_stri)
#else

stritype cp_from_os_path (os_stri)
os_stritype os_stri;
#endif

  {
    stritype result;

  /* cp_from_os_path */
    result = stri_to_standard_path(os_stri_to_stri(os_stri));
    return result;
  } /* cp_from_os_path */



#ifdef OS_PATH_WCHAR
#ifdef ANSI_C

static void path_to_wstri (wstritype wstri, const strelemtype *strelem,
    memsizetype len, errinfotype *err_info)
#else

static void path_to_wstri (wstri, strelem, len, err_info)
wstritype wstri;
strelemtype *strelem;
memsizetype len;
errinfotype *err_info;
#endif

  { /* path_to_wstri */
    for (; len > 0; wstri++, strelem++, len--) {
      if (likely(*strelem <= 0xFFFF)) {
        if (unlikely(*strelem == (strelemtype) '\\')) {
          *err_info = RANGE_ERROR;
          len = 1;
        } else {
          *wstri = (os_chartype) *strelem;
        } /* if */
      } else if (*strelem <= 0x10FFFF) {
        strelemtype currChar = *strelem - 0x10000;
        *wstri = (os_chartype) (0xD800 | (currChar >> 10));
        wstri++;
        *wstri = (os_chartype) (0xDC00 | (currChar & 0x3FF));
      } else {
        *err_info = RANGE_ERROR;
        len = 1;
      } /* if */
    } /* for */
    *wstri = 0;
  } /* path_to_wstri */



#ifdef ANSI_C

os_stritype cp_to_os_path (const_stritype stri, errinfotype *err_info)
#else

os_stritype cp_to_os_path (stri, err_info)
stritype stri;
errinfotype *err_info;
#endif

  {
    os_stritype result;

  /* cp_to_os_path */
#ifdef TRACE_STRIUTL
    printf("BEGIN cp_to_os_path(%lx, %d)\n", stri, *err_info);
#endif
#ifdef ALLOW_DRIVE_LETTERS
    if (unlikely(stri->size >= 2 && stri->mem[stri->size - 1] == '/' &&
                 (stri->size != 3 || stri->mem[1] != ':' ||
                 ((stri->mem[0] < 'a' || stri->mem[0] > 'z') &&
                  (stri->mem[0] < 'A' || stri->mem[0] > 'Z'))))) {
#else
    if (unlikely(stri->size >= 2 && (stri->mem[stri->size - 1] == '/' ||
                 (stri->mem[1] == ':' &&
                 ((stri->mem[0] >= 'a' && stri->mem[0] <= 'z') ||
                  (stri->mem[0] >= 'A' && stri->mem[0] <= 'Z')))))) {
#endif
      *err_info = RANGE_ERROR;
      result = NULL;
    } else {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (stri->size >= 1 && stri->mem[0] == '/') {
        /* Absolute path: Try to map the path to a drive letter */
        if (unlikely(stri->size == 1)) {
          /* "/"    cannot be mapped to a drive letter */
          *err_info = RANGE_ERROR;
          result = NULL;
        } else if (stri->mem[1] >= 'a' && stri->mem[1] <= 'z') {
          if (stri->size == 2) {
            /* "/c"   is mapped to "c:\"  */
            result = (os_stritype) malloc(sizeof(os_chartype) * 4);
            if (unlikely(result == NULL)) {
              *err_info = MEMORY_ERROR;
            } else {
              result[0] = (os_chartype) stri->mem[1];
              result[1] = ':';
              result[2] = '\\';
              result[3] = '\0';
            } /* if */
          } else if (unlikely(stri->mem[2] != '/')) {
            /* "/cd"  cannot be mapped to a drive letter */
            *err_info = RANGE_ERROR;
            result = NULL;
          } else {
            /* "/c/d" is mapped to "c:\d" */
            result = (os_stritype) malloc(sizeof(os_chartype) * ((stri->size - 3) * 2 + 4));
            if (unlikely(result == NULL)) {
              *err_info = MEMORY_ERROR;
            } else {
              result[0] = (os_chartype) stri->mem[1];
              result[1] = ':';
              result[2] = '\\';
              path_to_wstri(&result[3], &stri->mem[3], stri->size - 3, err_info);
            } /* if */
          } /* if */
        } else {
          /* "/C"  cannot be mapped to a drive letter */
          *err_info = RANGE_ERROR;
          result = NULL;
        } /* if */
      } else {
#endif
        result = (os_stritype) malloc(sizeof(os_chartype) * (stri->size * 2 + 1));
        if (unlikely(result == NULL)) {
          *err_info = MEMORY_ERROR;
        } else {
          path_to_wstri(result, stri->mem, stri->size, err_info);
        } /* if */
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      } /* if */
#endif
      if (result != NULL && *err_info != OKAY_NO_ERROR) {
        os_stri_free(result);
        result = NULL;
      } /* if */
    } /* if */
#ifdef TRACE_STRIUTL
    printf("END cp_to_os_path(%lx, %d) ==> %lx\n", stri, *err_info, result);
#endif
    return result;
  } /* cp_to_os_path */

#else



#ifdef ANSI_C

os_stritype cp_to_os_path (const_stritype stri, errinfotype *err_info)
#else

os_stritype cp_to_os_path (stri, err_info)
stritype stri;
errinfotype *err_info;
#endif

  {
    os_stritype result;

  /* cp_to_os_path */
#ifdef TRACE_STRIUTL
    printf("BEGIN cp_to_os_path(%lx, %d)\n", stri, *err_info);
#endif
    if (unlikely(stri->size >= 2 && stri->mem[stri->size - 1] == '/')) {
      *err_info = RANGE_ERROR;
      result = NULL;
    } else if (unlikely(stri->size > MAX_CSTRI_LEN / MAX_UTF8_EXPANSION_FACTOR ||
                        !ALLOC_CSTRI(result, max_utf8_size(stri)))) {
      *err_info = MEMORY_ERROR;
      result = NULL;
    } else {
      stri_export((ustritype) result, stri);
    } /* if */
#ifdef TRACE_STRIUTL
    printf("END cp_to_os_path(%lx, %d) ==> %lx\n", stri, *err_info, result);
#endif
    return result;
  } /* cp_to_os_path */

#endif



#ifdef ANSI_C

const strelemtype *stri_charpos (const_stritype stri, strelemtype ch)
#else

strelemtype *stri_charpos (stri, ch)
stritype stri;
strelemtype ch;
#endif

  {
    const strelemtype *mem;
    size_t number;

  /* stri_charpos */
    mem = stri->mem;
    number = stri->size;
    for (; number > 0; mem++, number--) {
      if (*mem == ch) {
        return mem;
      } /* if */
    } /* for */
    return NULL;
  } /* stri_charpos */



/**
 *  Returns a command string suitable for the operating system shell.
 *  The string 'stri' is converted to os_stritype. The command part of
 *  'stri' (which extends from the beginning up to the first blank)
 *  is treated special: All occurances of slash ( / ) are replaced
 *  by PATH_DELIMITER (which is defined as the path delimiter used
 *  by the operating system shell). Note that some operating systems
 *  accept / in paths used by C system calls but insist on \ in paths
 *  used by the system shell. Additionally the command part is also
 *  searched for the sequence "\ " which is treated special (Note
 *  that Seed7 string literals need a double backslash to represend
 "  one backslash. E.g.: "usr/home/tm/My\\ Dir/myCommand"). The
 *  sequence "\ " is used to allow space characters in the command
 *  path. Depending on the operating system the sequence "\ " is left
 *  as is or forces the command path to be surrounded by double
 *  quotes (in that case "\ " is replaced by " ").
 */
#ifdef ANSI_C

os_stritype cp_to_command (const_stritype stri, errinfotype *err_info)
#else

os_stritype cp_to_command (stri, err_info)
stritype stri;
errinfotype *err_info;
#endif

  {
    memsizetype inPos;
    memsizetype outPos;
    booltype quote_path;
    os_stritype cmd;

  /* cp_to_command */
    cmd = stri_to_os_stri(stri, err_info);
    if (*err_info == OKAY_NO_ERROR) {
      quote_path = FALSE;
      /* replace "/" by PATH_DELIMITER in cmd */
      for (inPos = 0, outPos = 0; cmd[inPos] != ' ' && cmd[inPos] != '\0'; inPos++, outPos++) {
        if (cmd[inPos] == '/') {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
          if (inPos == 0 && cmd[1] >= 'a' && cmd[1] <= 'z' && cmd[2] == '/') {
            /* Absolute path: Try to map the path to a drive letter */
            inPos++;
            cmd[outPos] = cmd[inPos];
            outPos++;
            cmd[outPos] = ':';
          } else {
#endif
            cmd[outPos] = PATH_DELIMITER;
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
          } /* if */
#endif
        } else if (cmd[inPos] == '\\') {
          inPos++;
          if (cmd[inPos] == ' ') {
#ifdef ESCAPE_SPACES_IN_COMMANDS
              cmd[outPos] = '\\';
              outPos++;
#else
              quote_path = TRUE;
#endif
            cmd[outPos] = ' ';
          } else {
            cmd[outPos] = '\\';
            if (cmd[inPos] != '\0') {
              outPos++;
              cmd[outPos] = cmd[inPos];
            } /* if */
          } /* if */
        } else {
          cmd[outPos] = cmd[inPos];
        } /* if */
      } /* for */
      if (quote_path) {
#ifdef OS_PATH_WCHAR
        memmove(&cmd[outPos + 2], &cmd[inPos], sizeof(os_chartype) * (wcslen(&cmd[inPos]) + 1));
        memmove(&cmd[1], cmd, sizeof(os_chartype) * outPos);
#else
        memmove(&cmd[outPos + 2], &cmd[inPos], strlen(&cmd[inPos]) + 1);
        memmove(&cmd[1], cmd, outPos);
#endif
        cmd[0] = '"';
        cmd[outPos + 1] = '"';
      } /* if */
#ifdef TRACE_CP_TO_COMMAND
      for (inPos = 0; cmd[inPos] != '\0'; inPos++) {
        printf("%c", cmd[inPos]);
      } /* for */
      printf("\n");
#endif
    } /* if */
    return cmd;
  } /* cp_to_command */



#ifdef PATHS_RELATIVE_TO_EXECUTABLE
#ifdef ANSI_C

stritype relativeToProgramPath (const const_stritype basePath,
    const const_cstritype dir, const const_cstritype library_name)
#else

stritype relativeToProgramPath (basePath, dir, library_name)
stritype basePath;
cstritype dir;
cstritype library_name;
#endif

  {
    memsizetype dir_path_size;
    memsizetype position;
    stritype result;

  /* relativeToProgramPath */
    dir_path_size = 0;
    for (position = 0; position < basePath->size; position++) {
      if (basePath->mem[position] == '/') {
        dir_path_size = position;
      } /* if */
    } /* for */
    if (dir_path_size >= 4 &&
        basePath->mem[dir_path_size - 4] == '/' && ((
        basePath->mem[dir_path_size - 3] == 'b' &&
        basePath->mem[dir_path_size - 2] == 'i' &&
        basePath->mem[dir_path_size - 1] == 'n') || (
        basePath->mem[dir_path_size - 3] == 'p' &&
        basePath->mem[dir_path_size - 2] == 'r' &&
        basePath->mem[dir_path_size - 1] == 'g'))) {
      if (likely(ALLOC_STRI_SIZE_OK(result, dir_path_size - 3 + strlen(dir) + strlen(library_name)))) {
        result->size = dir_path_size - 3 + strlen(dir) + strlen(library_name);
        memcpy(result->mem, basePath->mem, (dir_path_size - 3) * sizeof(strelemtype));
        cstri_expand(&result->mem[dir_path_size - 3], dir, strlen(dir));
        cstri_expand(&result->mem[dir_path_size - 3 + strlen(dir)], library_name, strlen(library_name));
      } /* if */
    } else {
      result = cstri_to_stri(library_name);
    } /* if */
    return result;
  } /* relativeToProgramPath */
#endif
