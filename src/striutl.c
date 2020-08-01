/********************************************************************/
/*                                                                  */
/*  striutl.c     Procedures to work with wide char strings.        */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
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
/*  Changes: 1991 - 1994, 2005 - 2013  Thomas Mertes                */
/*  Content: Procedures to work with wide char strings.             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#ifdef OS_STRI_WCHAR
#include "wchar.h"
#endif

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

#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#ifdef EMULATE_ROOT_CWD
os_stritype current_emulated_cwd = NULL;
#endif
os_chartype emulated_root[] = {'/', '\0'};
#endif



#ifdef OS_STRI_WCHAR

#define MAX_OS_STRI_SIZE    (((MAX_MEMSIZETYPE / sizeof(os_chartype)) - 1) / 2)
#define MAX_OS_BSTRI_SIZE   (((MAX_BSTRI_LEN / sizeof(os_chartype)) - 1) / 2)
#define OS_STRI_SIZE(size)  ((size) * 2)
#define OS_BSTRI_SIZE(size) (((size) * 2 + 1) * sizeof(os_chartype))

#elif defined OS_STRI_USES_CODEPAGE

#define MAX_OS_STRI_SIZE    ((MAX_MEMSIZETYPE / sizeof(os_chartype)) - 1)
#define MAX_OS_BSTRI_SIZE   ((MAX_BSTRI_LEN / sizeof(os_chartype)) - 1)
#define OS_STRI_SIZE(size)  (size)
#define OS_BSTRI_SIZE(size) (((size) + 1) * sizeof(os_chartype))
int codepage = DEFAULT_CODEPAGE;

#elif defined OS_STRI_UTF8

#define MAX_OS_STRI_SIZE    (MAX_CSTRI_LEN / MAX_UTF8_EXPANSION_FACTOR)
#define MAX_OS_BSTRI_SIZE   ((MAX_BSTRI_LEN - 1) / MAX_UTF8_EXPANSION_FACTOR)
#define OS_STRI_SIZE(size)  max_utf8_size(size)
#define OS_BSTRI_SIZE(size) (max_utf8_size(size) + 1)

#else

#define MAX_OS_STRI_SIZE    ((MAX_MEMSIZETYPE / sizeof(os_chartype)) - 1)
#define MAX_OS_BSTRI_SIZE   ((MAX_BSTRI_LEN / sizeof(os_chartype)) - 1)
#define OS_STRI_SIZE(size)  (size)
#define OS_BSTRI_SIZE(size) (((size) + 1) * sizeof(os_chartype))

#endif



#if defined OS_PATH_HAS_DRIVE_LETTERS || defined EMULATE_ROOT_CWD
static const strelemtype *stri_charpos (const_stritype stri, strelemtype ch)

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
#endif



memsizetype utf8_to_stri (strelemtype *const dest_stri, memsizetype *const dest_len,
    const_ustritype ustri, size_t len)

  {
    strelemtype *stri;

  /* utf8_to_stri */
    stri = dest_stri;
    for (; len > 0; len--) {
      if (*ustri <= 0x7F) {
        *stri++ = (strelemtype) *ustri++;
      } else if (ustri[0] >= 0xC0 && ustri[0] <= 0xDF && len >= 2 &&
                 ustri[1] >= 0x80 && ustri[1] <= 0xBF) {
        /* ustri[0]   range 192 to 223 (leading bits 110.....) */
        /* ustri[1]   range 128 to 191 (leading bits 10......) */
        *stri++ = (strelemtype) (ustri[0] & 0x1F) << 6 |
                  (strelemtype) (ustri[1] & 0x3F);
        ustri += 2;
        len--;
      } else if (ustri[0] >= 0xE0 && ustri[0] <= 0xEF && len >= 3 &&
                 ustri[1] >= 0x80 && ustri[1] <= 0xBF &&
                 ustri[2] >= 0x80 && ustri[2] <= 0xBF) {
        /* ustri[0]   range 224 to 239 (leading bits 1110....) */
        /* ustri[1..] range 128 to 191 (leading bits 10......) */
        *stri++ = (strelemtype) (ustri[0] & 0x0F) << 12 |
                  (strelemtype) (ustri[1] & 0x3F) <<  6 |
                  (strelemtype) (ustri[2] & 0x3F);
        ustri += 3;
        len -= 2;
      } else if (ustri[0] >= 0xF0 && ustri[0] <= 0xF7 && len >= 4 &&
                 ustri[1] >= 0x80 && ustri[1] <= 0xBF &&
                 ustri[2] >= 0x80 && ustri[2] <= 0xBF &&
                 ustri[3] >= 0x80 && ustri[3] <= 0xBF) {
        /* ustri[0]   range 240 to 247 (leading bits 11110...) */
        /* ustri[1..] range 128 to 191 (leading bits 10......) */
        *stri++ = (strelemtype) (ustri[0] & 0x07) << 18 |
                  (strelemtype) (ustri[1] & 0x3F) << 12 |
                  (strelemtype) (ustri[2] & 0x3F) <<  6 |
                  (strelemtype) (ustri[3] & 0x3F);
        ustri += 4;
        len -= 3;
      } else if (ustri[0] >= 0xF8 && ustri[0] <= 0xFB && len >= 5 &&
                 ustri[1] >= 0x80 && ustri[1] <= 0xBF &&
                 ustri[2] >= 0x80 && ustri[2] <= 0xBF &&
                 ustri[3] >= 0x80 && ustri[3] <= 0xBF &&
                 ustri[4] >= 0x80 && ustri[4] <= 0xBF) {
        /* ustri[0]   range 248 to 251 (leading bits 111110..) */
        /* ustri[1..] range 128 to 191 (leading bits 10......) */
        *stri++ = (strelemtype) (ustri[0] & 0x03) << 24 |
                  (strelemtype) (ustri[1] & 0x3F) << 18 |
                  (strelemtype) (ustri[2] & 0x3F) << 12 |
                  (strelemtype) (ustri[3] & 0x3F) <<  6 |
                  (strelemtype) (ustri[4] & 0x3F);
        ustri += 5;
        len -= 4;
      } else if (ustri[0] >= 0xFC && ustri[0] <= 0xFF && len >= 6 &&
                 ustri[1] >= 0x80 && ustri[1] <= 0xBF &&
                 ustri[2] >= 0x80 && ustri[2] <= 0xBF &&
                 ustri[3] >= 0x80 && ustri[3] <= 0xBF &&
                 ustri[4] >= 0x80 && ustri[4] <= 0xBF &&
                 ustri[5] >= 0x80 && ustri[5] <= 0xBF) {
        /* ustri[0]   range 252 to 255 (leading bits 111111..) */
        /* ustri[1..] range 128 to 191 (leading bits 10......) */
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
        /* or not enough continuation bytes found.         */
        *dest_len = (memsizetype) (stri - dest_stri);
        return len;
      } /* if */
    } /* for */
    *dest_len = (memsizetype) (stri - dest_stri);
    return 0;
  } /* utf8_to_stri */



memsizetype utf8_bytes_missing (const const_ustritype ustri, const size_t len)

  {
    memsizetype result;

  /* utf8_bytes_missing */
    result = 0;
    if (len >= 1 && *ustri > 0x7F) {
      if (ustri[0] >= 0xC0 && ustri[0] <= 0xDF) {
        /* ustri[0]   range 192 to 223 (leading bits 110.....) */
        if (len == 1) {
          result = 1;
        } /* if */
      } else if (ustri[0] >= 0xE0 && ustri[0] <= 0xEF) {
        /* ustri[0]   range 224 to 239 (leading bits 1110....) */
        if (len == 1) {
          result = 2;
        } else if (ustri[1] >= 0x80 && ustri[1] <= 0xBF) {
          /* ustri[1]   range 128 to 191 (leading bits 10......) */
          if (len == 2) {
            result = 1;
          } /* if */
        } /* if */
      } else if (ustri[0] >= 0xF0 && ustri[0] <= 0xF7) {
        /* ustri[0]   range 240 to 247 (leading bits 11110...) */
        if (len == 1) {
          result = 3;
        } else if (ustri[1] >= 0x80 && ustri[1] <= 0xBF) {
          /* ustri[1]   range 128 to 191 (leading bits 10......) */
          if (len == 2) {
            result = 2;
          } else if (ustri[2] >= 0x80 && ustri[2] <= 0xBF) {
            if (len == 3) {
              result = 1;
            } /* if */
          } /* if */
        } /* if */
      } else if (ustri[0] >= 0xF8 && ustri[0] <= 0xFB) {
        /* ustri[0]   range 248 to 251 (leading bits 111110..) */
        if (len == 1) {
          result = 4;
        } else if (ustri[1] >= 0x80 && ustri[1] <= 0xBF) {
          /* ustri[1]   range 128 to 191 (leading bits 10......) */
          if (len == 2) {
            result = 3;
          } else if (ustri[2] >= 0x80 && ustri[2] <= 0xBF) {
            if (len == 3) {
              result = 2;
            } else if (ustri[3] >= 0x80 && ustri[3] <= 0xBF) {
              if (len == 4) {
                result = 1;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } else if (ustri[0] >= 0xFC && ustri[0] <= 0xFF) {
        /* ustri[0]   range 252 to 255 (leading bits 111111..) */
        if (len == 1) {
          result = 5;
        } else if (ustri[1] >= 0x80 && ustri[1] <= 0xBF) {
          /* ustri[1]   range 128 to 191 (leading bits 10......) */
          if (len == 2) {
            result = 4;
          } else if (ustri[2] >= 0x80 && ustri[2] <= 0xBF) {
            if (len == 3) {
              result = 3;
            } else if (ustri[3] >= 0x80 && ustri[3] <= 0xBF) {
              if (len == 4) {
                result = 2;
              } else if (ustri[4] >= 0x80 && ustri[4] <= 0xBF) {
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



memsizetype stri_to_utf8 (const ustritype out_stri,
    const strelemtype *strelem, memsizetype len)

  {
    register ustritype ustri;
    register strelemtype ch;

  /* stri_to_utf8 */
    ustri = out_stri;
    for (; len > 0; strelem++, len--) {
      ch = *strelem;
      if (ch <= 0x7F) {
        *ustri++ = (uchartype) ch;
      } else if (ch <= 0x7FF) {
        ustri[0] = (uchartype) (0xC0 | (ch >>  6));
        ustri[1] = (uchartype) (0x80 |( ch        & 0x3F));
        ustri += 2;
      } else if (ch <= 0xFFFF) {
        ustri[0] = (uchartype) (0xE0 | (ch >> 12));
        ustri[1] = (uchartype) (0x80 |((ch >>  6) & 0x3F));
        ustri[2] = (uchartype) (0x80 |( ch        & 0x3F));
        ustri += 3;
      } else if (ch <= 0x1FFFFF) {
        ustri[0] = (uchartype) (0xF0 | (ch >> 18));
        ustri[1] = (uchartype) (0x80 |((ch >> 12) & 0x3F));
        ustri[2] = (uchartype) (0x80 |((ch >>  6) & 0x3F));
        ustri[3] = (uchartype) (0x80 |( ch        & 0x3F));
        ustri += 4;
      } else if (ch <= 0x3FFFFFF) {
        ustri[0] = (uchartype) (0xF8 | (ch >> 24));
        ustri[1] = (uchartype) (0x80 |((ch >> 18) & 0x3F));
        ustri[2] = (uchartype) (0x80 |((ch >> 12) & 0x3F));
        ustri[3] = (uchartype) (0x80 |((ch >>  6) & 0x3F));
        ustri[4] = (uchartype) (0x80 |( ch        & 0x3F));
        ustri += 5;
      } else {
        ustri[0] = (uchartype) (0xFC | (ch >> 30));
        ustri[1] = (uchartype) (0x80 |((ch >> 24) & 0x3F));
        ustri[2] = (uchartype) (0x80 |((ch >> 18) & 0x3F));
        ustri[3] = (uchartype) (0x80 |((ch >> 12) & 0x3F));
        ustri[4] = (uchartype) (0x80 |((ch >>  6) & 0x3F));
        ustri[5] = (uchartype) (0x80 |( ch        & 0x3F));
        ustri += 6;
      } /* if */
    } /* for */
    return (memsizetype) (ustri - out_stri);
  } /* stri_to_utf8 */



static inline void stri_to_os_utf8 (register ustritype out_stri,
    register const strelemtype *strelem, register memsizetype len,
    errinfotype *err_info)

  {
    register strelemtype ch;

  /* stri_to_os_utf8 */
    for (; len > 0; strelem++, len--) {
      ch = *strelem;
      if (ch <= 0x7F) {
        if (unlikely(ch == '\0')) {
          *err_info = RANGE_ERROR;
          return;
        } else {
          *out_stri++ = (uchartype) ch;
        } /* if */
      } else if (ch <= 0x7FF) {
        out_stri[0] = (uchartype) (0xC0 | (ch >>  6));
        out_stri[1] = (uchartype) (0x80 |( ch        & 0x3F));
        out_stri += 2;
      } else if (ch <= 0xFFFF) {
        out_stri[0] = (uchartype) (0xE0 | (ch >> 12));
        out_stri[1] = (uchartype) (0x80 |((ch >>  6) & 0x3F));
        out_stri[2] = (uchartype) (0x80 |( ch        & 0x3F));
        out_stri += 3;
      } else if (ch <= 0x10FFFF) {
        out_stri[0] = (uchartype) (0xF0 | (ch >> 18));
        out_stri[1] = (uchartype) (0x80 |((ch >> 12) & 0x3F));
        out_stri[2] = (uchartype) (0x80 |((ch >>  6) & 0x3F));
        out_stri[3] = (uchartype) (0x80 |( ch        & 0x3F));
        out_stri += 4;
      } else {
        *err_info = RANGE_ERROR;
        return;
      } /* if */
    } /* for */
    *out_stri = '\0';
  } /* stri_to_os_utf8 */



void ustri_expand (strelemtype *stri, const_ustritype ustri, size_t len)

  { /* ustri_expand */
    for (; len > 0; stri++, ustri++, len--) {
      *stri = (strelemtype) *ustri;
    } /* while */
  } /* ustri_expand */



size_t ustri_expand2 (strelemtype *const stri, const_ustritype ustri)

  {
    size_t pos = 0;

  /* ustri_expand2 */
    for (; *ustri != '\0'; ustri++, pos++) {
      stri[pos] = (strelemtype) *ustri;
    } /* while */
    return pos;
  } /* ustri_expand2 */



/**
 *  Copy a Seed7 UTF-32 string to an ISO-8859-1 encoded C string buffer.
 *  The buffer 'cstri' must be provided by the caller. The
 *  size of the 'cstri' buffer can be calculated (in bytes) with
 *  stri->size+1. When a fixed size 'cstri' buffer is used
 *  (e.g.: char out_buffer[BUF_SIZE];) the condition
 *    stri->size < BUF_SIZE
 *  must hold. The C string written to 'out_buf' is zero byte
 *  terminated. This function is intended to copy to temporary
 *  string buffers, that are used as parameters. This function
 *  is useful, when stri->size is somehow limited, such that
 *  a fixed size 'cstri' buffer can be used.
 *  @param cstri Caller provided buffer to which an ISO-8859-1
 *         encoded null terminated C string is written.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param err_info Unchanged when the function succeeds or
 *                  RANGE_ERROR when stri contains a null character
 *                        or a character that is higher than the
 *                        highest allowed ISO-8859-1 character (255).
 */
void conv_to_cstri (cstritype cstri, const const_stritype stri,
    errinfotype *err_info)

  {
    const strelemtype *str;
    memsizetype pos;

  /* conv_to_cstri */
    str = stri->mem;
    for (pos = stri->size; pos > 0; pos--) {
      if (unlikely(str[pos - 1] == 0 || str[pos - 1] >= 256)) {
        *err_info = RANGE_ERROR;
        return;
      } /* if */
      cstri[pos - 1] = (char) (uchartype) str[pos - 1];
    } /* for */
    cstri[stri->size] = '\0';
  } /* conv_to_cstri */



/**
 *  Copy a Seed7 UTF-32 string to an UTF-8 encoded C string buffer.
 *  The buffer 'cstri' must be provided by the caller. The
 *  size of the 'cstri' buffer can be calculated (in bytes) with
 *  max_utf8_size(stri->size)+1. When a fixed size 'cstri'
 *  buffer is used (e.g.: char out_buffer[BUF_SIZE];) the condition
 *    stri->size < BUF_SIZE / MAX_UTF8_EXPANSION_FACTOR
 *  must hold. The C string written to 'out_buf' is zero byte
 *  terminated. This function is intended to copy to temporary
 *  string buffers, that are used as parameters. This function
 *  is useful, when stri->size is somehow limited, such that
 *  a fixed size 'cstri' buffer can be used.
 *  @param cstri Caller provided buffer to which an UTF-8 encoded
 *         null terminated C string is written.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param err_info Unchanged when the function succeeds or
 *                  RANGE_ERROR when stri contains a null character 
 *                        or a character that is higher than the
 *                        highest allowed Unicode character (U+10FFFF).
 */
void conv_to_cstri8 (cstritype cstri, const const_stritype stri,
    errinfotype *err_info)

  { /* conv_to_cstri8 */
    stri_to_os_utf8((ustritype) cstri, stri->mem, stri->size, err_info);
  } /* conv_to_cstri8 */



#ifdef OS_STRI_WCHAR
memsizetype stri_to_wstri (const wstritype out_wstri,
    register const strelemtype *strelem, memsizetype len,
    errinfotype *const err_info)

  {
    register wstritype wstri;

  /* stri_to_wstri */
    wstri = out_wstri;
    for (; len > 0; wstri++, strelem++, len--) {
      if (likely(*strelem <= 0xFFFF)) {
        *wstri = (os_chartype) *strelem;
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
    return (memsizetype) (wstri - out_wstri);
  } /* stri_to_wstri */



static inline void conv_to_os_stri (register os_stritype os_stri,
    register const strelemtype *strelem, memsizetype len,
    errinfotype *const err_info)

  { /* conv_to_os_stri */
    for (; len > 0; os_stri++, strelem++, len--) {
      if (likely(*strelem <= 0xFFFF)) {
        if (*strelem == '\0') {
          *err_info = RANGE_ERROR;
          len = 1;
        } else {
          *os_stri = (os_chartype) *strelem;
        } /* if */
      } else if (*strelem <= 0x10FFFF) {
        strelemtype currChar = *strelem - 0x10000;
        *os_stri = (os_chartype) (0xD800 | (currChar >> 10));
        os_stri++;
        *os_stri = (os_chartype) (0xDC00 | (currChar & 0x3FF));
      } else {
        *err_info = RANGE_ERROR;
        len = 1;
      } /* if */
    } /* for */
    *os_stri = '\0';
  } /* conv_to_os_stri */



#elif defined OS_STRI_USES_CODEPAGE

static unsigned char map_to_437_160[] = {
/*  160 */  255,  173,  155,  156,  '?',  157,  '?',  '?',  '?',  '?',
/*  170 */  166,  174,  170,  '?',  '?',  '?',  248,  241,  253,  '?',
/*  180 */  '?',  230,  '?',  250,  '?',  '?',  167,  175,  172,  171,
/*  190 */  '?',  168,  '?',  '?',  '?',  '?',  142,  143,  146,  128,
/*  200 */  '?',  144,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  165,
/*  210 */  '?',  '?',  '?',  '?',  153,  '?',  '?',  '?',  '?',  '?',
/*  220 */  154,  '?',  '?',  225,  133,  160,  131,  '?',  132,  134,
/*  230 */  145,  135,  138,  130,  136,  137,  141,  161,  140,  139,
/*  240 */  '?',  164,  149,  162,  147,  '?',  148,  246,  '?',  151,
/*  250 */  163,  150,  129,  '?',  '?',  152};

static unsigned char map_to_437_915[] = {
/*  910 */                                226,  '?',  '?',  '?',  '?',
/*  920 */  233,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/*  930 */  '?',  228,  '?',  '?',  232,  '?',  '?',  234,  '?',  '?',
/*  940 */  '?',  '?',  '?',  '?',  '?',  224,  '?',  '?',  235,  238,
/*  950 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/*  960 */  227,  '?',  '?',  229,  231,  '?',  237};

static unsigned char map_to_437_9472[] = {
/* 9470 */              196,  '?',  179,  '?',  '?',  '?',  '?',  '?',
/* 9480 */  '?',  '?',  '?',  '?',  218,  '?',  '?',  '?',  191,  '?',
/* 9490 */  '?',  '?',  192,  '?',  '?',  '?',  217,  '?',  '?',  '?',
/* 9500 */  195,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  180,  '?',
/* 9510 */  '?',  '?',  '?',  '?',  '?',  '?',  194,  '?',  '?',  '?',
/* 9520 */  '?',  '?',  '?',  '?',  193,  '?',  '?',  '?',  '?',  '?',
/* 9530 */  '?',  '?',  197,  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9540 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9550 */  '?',  '?',  205,  186,  213,  214,  201,  184,  183,  187,
/* 9560 */  212,  211,  200,  190,  189,  188,  198,  199,  204,  181,
/* 9570 */  182,  185,  209,  210,  203,  207,  208,  202,  216,  215,
/* 9580 */  206,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9590 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9600 */  223,  '?',  '?',  '?',  220,  '?',  '?',  '?',  219,  '?',
/* 9610 */  '?',  '?',  221,  '?',  '?',  '?',  222,  176,  177,  178,
/* 9620 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9630 */  '?',  '?',  254};

static unsigned char map_to_850_160[] = {
/*  160 */  255,  173,  189,  156,  207,  190,  221,  245,  249,  184,
/*  170 */  166,  174,  170,  240,  169,  238,  248,  241,  253,  252,
/*  180 */  239,  230,  244,  250,  247,  251,  167,  175,  172,  171,
/*  190 */  243,  168,  183,  181,  182,  199,  142,  143,  146,  128,
/*  200 */  212,  144,  210,  211,  222,  214,  215,  216,  209,  165,
/*  210 */  227,  224,  226,  229,  153,  158,  157,  235,  233,  234,
/*  220 */  154,  237,  232,  225,  133,  160,  131,  198,  132,  134,
/*  230 */  145,  135,  138,  130,  136,  137,  141,  161,  140,  139,
/*  240 */  208,  164,  149,  162,  147,  228,  148,  246,  155,  151,
/*  250 */  163,  150,  129,  236,  231,  152};

static unsigned char map_to_850_9472[] = {
/* 9470 */              196,  '?',  179,  '?',  '?',  '?',  '?',  '?',
/* 9480 */  '?',  '?',  '?',  '?',  218,  '?',  '?',  '?',  191,  '?',
/* 9490 */  '?',  '?',  192,  '?',  '?',  '?',  217,  '?',  '?',  '?',
/* 9500 */  195,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  180,  '?',
/* 9510 */  '?',  '?',  '?',  '?',  '?',  '?',  194,  '?',  '?',  '?',
/* 9520 */  '?',  '?',  '?',  '?',  193,  '?',  '?',  '?',  '?',  '?',
/* 9530 */  '?',  '?',  197,  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9540 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9550 */  '?',  '?',  205,  186,  '?',  '?',  201,  '?',  '?',  187,
/* 9560 */  '?',  '?',  200,  '?',  '?',  188,  '?',  '?',  204,  '?',
/* 9570 */  '?',  185,  '?',  '?',  203,  '?',  '?',  202,  '?',  '?',
/* 9580 */  206,  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9590 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9600 */  223,  '?',  '?',  '?',  220,  '?',  '?',  '?',  219,  '?',
/* 9610 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  176,  177,  178,
/* 9620 */  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',  '?',
/* 9630 */  '?',  '?',  254};



static inline void conv_to_os_stri (os_stritype os_stri,
    const strelemtype *strelem, memsizetype len, errinfotype *err_info)

  {
    unsigned char ch;

  /* conv_to_os_stri */
    if (codepage == 437) {
      for (; len > 0; os_stri++, strelem++, len--) {
        if (*strelem <= 127) {
          if (*strelem == '\0') {
            *err_info = RANGE_ERROR;
            len = 1;
          } else {
            *os_stri = (os_chartype) *strelem;
          } /* if */
        } else {
          if (*strelem >= 160 && *strelem <= 255) {
            ch = map_to_437_160[*strelem - 160];
          } else if (*strelem >= 915 && *strelem <= 966) {
            ch = map_to_437_915[*strelem - 915];
          } else if (*strelem >= 9472 && *strelem <= 9632) {
            ch = map_to_437_9472[*strelem - 9472];
          } else {
            switch (*strelem) {
              case  402: ch = 159; break;
              case 8319: ch = 252; break;
              case 8359: ch = 158; break;
              case 8729: ch = 249; break;
              case 8730: ch = 251; break;
              case 8734: ch = 236; break;
              case 8745: ch = 239; break;
              case 8776: ch = 247; break;
              case 8801: ch = 240; break;
              case 8804: ch = 243; break;
              case 8805: ch = 242; break;
              case 8976: ch = 169; break;
              case 8992: ch = 244; break;
              case 8993: ch = 245; break;
              default:   ch = '?'; break;
            } /* switch */
          } /* if */
          *os_stri = (os_chartype) ch;
          if (ch == '?') {
            *err_info = RANGE_ERROR;
            /* The conversion continues. The caller  */
            /* can decide to use the question marks. */
          } /* if */
        } /* if */
      } /* for */
    } else if (codepage == 850) {
      for (; len > 0; os_stri++, strelem++, len--) {
        if (*strelem <= 127) {
          if (*strelem == '\0') {
            *err_info = RANGE_ERROR;
            len = 1;
          } else {
            *os_stri = (os_chartype) *strelem;
          } /* if */
        } else {
          if (*strelem >= 160 && *strelem <= 255) {
            ch = map_to_850_160[*strelem - 160];
          } else if (*strelem >= 9472 && *strelem <= 9632) {
            ch = map_to_850_9472[*strelem - 9472];
          } else {
            switch (*strelem) {
              case 8215: ch = 242; break;
              case 305:  ch = 213; break;
              case 402:  ch = 159; break;
              default:   ch = '?'; break;
            } /* switch */
          } /* if */
          *os_stri = (os_chartype) ch;
          if (ch == '?') {
            *err_info = RANGE_ERROR;
            /* The conversion continues. The caller  */
            /* can decide to use the question marks. */
          } /* if */
        } /* if */
      } /* for */
    } else {
      *err_info = RANGE_ERROR;
    } /* if */
    *os_stri = '\0';
  } /* conv_to_os_stri */

#elif defined OS_STRI_UTF8



static inline void conv_to_os_stri (const os_stritype os_stri,
    const strelemtype *const strelem, const memsizetype len,
    errinfotype *err_info)

  { /* conv_to_os_stri */
    stri_to_os_utf8((ustritype) os_stri, strelem, len, err_info);
  } /* conv_to_os_stri */

#else



static inline void conv_to_os_stri (os_stritype os_stri,
    const strelemtype *strelem, memsizetype len, errinfotype *err_info)

  { /* conv_to_os_stri */
    for (; len > 0; strelem++, os_stri++, len--) {
      if (unlikely(*strelem == '\0' || *strelem >= 256)) {
        *err_info = RANGE_ERROR;
      } /* if */
      *os_stri = (os_chartype) *strelem;
    } /* for */
    *os_stri = '\0';
  } /* conv_to_os_stri */

#endif



#if defined OS_STRI_WCHAR
static memsizetype wstri_expand (strelemtype *const dest_stri,
    const_wstritype wstri, memsizetype len)

  {
    strelemtype *stri;
    os_chartype ch1;
    os_chartype ch2;

  /* wstri_expand */
    stri = dest_stri;
    for (; len > 0; stri++, wstri++, len--) {
      ch1 = *wstri;
      if (unlikely(ch1 >= 0xD800 && ch1 <= 0xDBFF)) {
        ch2 = wstri[1];
        if (likely(ch2 >= 0xDC00 && ch2 <= 0xDFFF)) {
          *stri = ((((strelemtype) ch1 - 0xD800) << 10) +
                    ((strelemtype) ch2 - 0xDC00) + 0x10000);
          wstri++;
          len--;
        } else {
          *stri = (strelemtype) ch1;
        } /* if */
      } else {
        *stri = (strelemtype) ch1;
      } /* if */
    } /* for */
    return (memsizetype) (stri - dest_stri);
  } /* wstri_expand */



/**
 *  Convert an os_stritype string with length to a Seed7 UTF-32 string.
 *  Many system calls return os_stritype data with length. System calls
 *  are defined in "version.h" and "os_decls.h". They are prefixed
 *  with os_ and use strings of the type os_stritype. Depending on the
 *  operating system os_stritype can describe byte or wide char strings.
 *  The encoding can be Latin-1, UTF-8, UTF-16 or it can use a code page.
 *  @param os_stri Possibly binary string (may contain null characters).
 *  @param length Length of os_stri.
 *  @return a Seed7 UTF-32 string or
 *          NULL, when an error occurred.
 */
stritype conv_from_os_stri (const const_os_stritype os_stri,
    memsizetype length)

  {
    memsizetype stri_size;
    stritype resized_stri;
    stritype stri;

  /* conv_from_os_stri */
    if (ALLOC_STRI_CHECK_SIZE(stri, length)) {
      stri_size = wstri_expand(stri->mem, os_stri, length);
      stri->size = stri_size;
      if (stri_size != length) {
        REALLOC_STRI_SIZE_SMALLER(resized_stri, stri, length, stri_size);
        if (resized_stri == NULL) {
          FREE_STRI(stri, length);
          stri = NULL;
        } else {
          stri = resized_stri;
          COUNT3_STRI(length, stri_size);
        } /* if */
      } /* if */
    } /* if */
    return stri;
  } /* conv_from_os_stri */



#elif defined OS_STRI_USES_CODEPAGE

static strelemtype map_from_437[] = {
/*   0 */    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,
/*  10 */   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
/*  20 */   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
/*  30 */   30,   31,  ' ',  '!',  '"',  '#',  '$',  '%',  '&', '\'',
/*  40 */  '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',  '0',  '1',
/*  50 */  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',
/*  60 */  '<',  '=',  '>',  '?',  '@',  'A',  'B',  'C',  'D',  'E',
/*  70 */  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/*  80 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',
/*  90 */  'Z',  '[', '\\',  ']',  '^',  '_',  '`',  'a',  'b',  'c',
/* 100 */  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',
/* 110 */  'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
/* 120 */  'x',  'y',  'z',  '{',  '|',  '}',  '~',  127,  199,  252,
/* 130 */  233,  226,  228,  224,  229,  231,  234,  235,  232,  239,
/* 140 */  238,  236,  196,  197,  201,  230,  198,  244,  246,  242,
/* 150 */  251,  249,  255,  214,  220,  162,  163,  165, 8359,  402,
/* 160 */  225,  237,  243,  250,  241,  209,  170,  186,  191, 8976,
/* 170 */  172,  189,  188,  161,  171,  187, 9617, 9618, 9619, 9474,
/* 180 */ 9508, 9569, 9570, 9558, 9557, 9571, 9553, 9559, 9565, 9564,
/* 190 */ 9563, 9488, 9492, 9524, 9516, 9500, 9472, 9532, 9566, 9567,
/* 200 */ 9562, 9556, 9577, 9574, 9568, 9552, 9580, 9575, 9576, 9572,
/* 210 */ 9573, 9561, 9560, 9554, 9555, 9579, 9578, 9496, 9484, 9608,
/* 220 */ 9604, 9612, 9616, 9600,  945,  223,  915,  960,  931,  963,
/* 230 */  181,  964,  934,  920,  937,  948, 8734,  966,  949, 8745,
/* 240 */ 8801,  177, 8805, 8804, 8992, 8993,  247, 8776,  176, 8729,
/* 250 */  183, 8730, 8319,  178, 9632,  160};

static strelemtype map_from_850[] = {
/*   0 */    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,
/*  10 */   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
/*  20 */   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
/*  30 */   30,   31,  ' ',  '!',  '"',  '#',  '$',  '%',  '&', '\'',
/*  40 */  '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',  '0',  '1',
/*  50 */  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',
/*  60 */  '<',  '=',  '>',  '?',  '@',  'A',  'B',  'C',  'D',  'E',
/*  70 */  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/*  80 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',
/*  90 */  'Z',  '[', '\\',  ']',  '^',  '_',  '`',  'a',  'b',  'c',
/* 100 */  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',
/* 110 */  'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
/* 120 */  'x',  'y',  'z',  '{',  '|',  '}',  '~',  127,  199,  252,
/* 130 */  233,  226,  228,  224,  229,  231,  234,  235,  232,  239,
/* 140 */  238,  236,  196,  197,  201,  230,  198,  244,  246,  242,
/* 150 */  251,  249,  255,  214,  220,  248,  163,  216,  215,  402,
/* 160 */  225,  237,  243,  250,  241,  209,  170,  186,  191,  174,
/* 170 */  172,  189,  188,  161,  171,  187, 9617, 9618, 9619, 9474,
/* 180 */ 9508,  193,  194,  192,  169, 9571, 9553, 9559, 9565,  162,
/* 190 */  165, 9488, 9492, 9524, 9516, 9500, 9472, 9532,  227,  195,
/* 200 */ 9562, 9556, 9577, 9574, 9568, 9552, 9580,  164,  240,  208,
/* 210 */  202,  203,  200,  305,  205,  206,  207, 9496, 9484, 9608,
/* 220 */ 9604,  166,  204, 9600,  211,  223,  212,  210,  245,  213,
/* 230 */  181,  254,  222,  218,  219,  217,  253,  221,  175,  180,
/* 240 */  173,  177, 8215,  190,  182,  167,  247,  184,  176,  168,
/* 250 */  183,  185,  179,  178, 9632,  160};



/**
 *  Convert an os_stritype string with length to a Seed7 UTF-32 string.
 *  Many system calls return os_stritype data with length. System calls
 *  are defined in "version.h" and "os_decls.h". They are prefixed
 *  with os_ and use strings of the type os_stritype. Depending on the
 *  operating system os_stritype can describe byte or wide char strings.
 *  The encoding can be Latin-1, UTF-8, UTF-16 or it can use a code page.
 *  @param os_stri Possibly binary string (may contain null characters).
 *  @param length Length of os_stri.
 *  @return a Seed7 UTF-32 string or
 *          NULL, when an error occurred.
 */
stritype conv_from_os_stri (const const_os_stritype os_stri,
    memsizetype length)

  {
    memsizetype pos;
    stritype stri;

  /* conv_from_os_stri */
    if (ALLOC_STRI_CHECK_SIZE(stri, length)) {
      stri->size = length;
      if (codepage == 437) {
        for (pos = 0; pos < length; pos++) {
          stri->mem[pos] = map_from_437[(unsigned char) os_stri[pos]];
        } /* for */
      } else if (codepage == 850) {
        for (pos = 0; pos < length; pos++) {
          stri->mem[pos] = map_from_850[(unsigned char) os_stri[pos]];
        } /* for */
      } else {
        FREE_STRI(stri, length);
        stri = NULL;
      } /* if */
    } /* if */
    return stri;
  } /* conv_from_os_stri */

#elif defined OS_STRI_UTF8



/**
 *  Convert an os_stritype string with length to a Seed7 UTF-32 string.
 *  Many system calls return os_stritype data with length. System calls
 *  are defined in "version.h" and "os_decls.h". They are prefixed
 *  with os_ and use strings of the type os_stritype. Depending on the
 *  operating system os_stritype can describe byte or wide char strings.
 *  The encoding can be Latin-1, UTF-8, UTF-16 or it can use a code page.
 *  @param os_stri Possibly binary string (may contain null characters).
 *  @param length Length of os_stri.
 *  @return a Seed7 UTF-32 string or
 *          NULL, when an error occurred.
 */
stritype conv_from_os_stri (const const_os_stritype os_stri,
    memsizetype length)

  {
    memsizetype stri_size;
    stritype resized_stri;
    stritype stri;

  /* conv_from_os_stri */
    if (ALLOC_STRI_CHECK_SIZE(stri, length)) {
      if (utf8_to_stri(stri->mem, &stri_size, (const_ustritype) os_stri, length) == 0) {
        REALLOC_STRI_SIZE_SMALLER(resized_stri, stri, length, stri_size);
        if (resized_stri == NULL) {
          FREE_STRI(stri, length);
          stri = NULL;
        } else {
          stri = resized_stri;
          COUNT3_STRI(length, stri_size);
          stri->size = stri_size;
        } /* if */
      } else {
        stri->size = length;
        cstri_expand(stri->mem, os_stri, length);
      } /* if */
    } /* if */
    return stri;
  } /* conv_from_os_stri */

#else



/**
 *  Convert an os_stritype string with length to a Seed7 UTF-32 string.
 *  Many system calls return os_stritype data with length. System calls
 *  are defined in "version.h" and "os_decls.h". They are prefixed
 *  with os_ and use strings of the type os_stritype. Depending on the
 *  operating system os_stritype can describe byte or wide char strings.
 *  The encoding can be Latin-1, UTF-8, UTF-16 or it can use a code page.
 *  @param os_stri Possibly binary string (may contain null characters).
 *  @param length Length of os_stri.
 *  @return a Seed7 UTF-32 string or
 *          NULL, when an error occurred.
 */
stritype conv_from_os_stri (const const_os_stritype os_stri,
    memsizetype length)

  {
    stritype stri;

  /* conv_from_os_stri */
    if (ALLOC_STRI_CHECK_SIZE(stri, length)) {
      ustri_expand(stri->mem, (const_ustritype) os_stri, length);
      stri->size = length;
    } /* if */
    return stri;
  } /* conv_from_os_stri */

#endif



/**
 *  Create an ISO-8859-1 encoded C string from a Seed7 UTF-32 string.
 *  The memory for the zero byte terminated C string is allocated.
 *  The C string result must be freed with the macro free_cstri().
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param err_info Unchanged when the function succeeds or
 *                  MEMORY_ERROR when the memory allocation failed or
 *                  RANGE_ERROR when stri contains a null character
 *                        or a character that is higher than the
 *                        highest allowed ISO-8859-1 character (255).
 *  @return an ISO-8859-1 encoded null terminated C string or
 *          NULL, when the memory allocation failed or when the
 *          conversion failed (the error is indicated by err_info).
 */
cstritype stri_to_cstri (const const_stritype stri, errinfotype *err_info)

  {
    const strelemtype *str;
    memsizetype pos;
    cstritype cstri;

  /* stri_to_cstri */
    if (unlikely(!ALLOC_CSTRI(cstri, stri->size))) {
      *err_info = MEMORY_ERROR;
    } else {
      str = stri->mem;
      for (pos = stri->size; pos > 0; pos--) {
        if (unlikely(str[pos - 1] == 0 || str[pos - 1] >= 256)) {
          UNALLOC_CSTRI(cstri, stri->size);
          *err_info = RANGE_ERROR;
          return NULL;
        } /* if */
        cstri[pos - 1] = (char) (uchartype) str[pos - 1];
      } /* for */
      cstri[stri->size] = '\0';
    } /* if */
    return cstri;
  } /* stri_to_cstri */



/**
 *  Create an UTF-8 encoded C string from a Seed7 UTF-32 string.
 *  The memory for the zero byte terminated C string is allocated.
 *  The C string result must be freed with the macro free_cstri8().
 *  This function is intended to create temporary strings, that
 *  are used as parameters. To get good performance the allocated
 *  memory for the C string is oversized.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param err_info Unchanged when the function succeeds or
 *                  MEMORY_ERROR when the memory allocation failed or
 *                  RANGE_ERROR when stri contains a null character 
 *                        or a character that is higher than the
 *                        highest allowed Unicode character (U+10FFFF).
 *  @return an UTF-8 encoded null terminated C string or
 *          NULL, when the memory allocation failed or when the
 *          conversion failed (the error is indicated by err_info).
 */
cstritype stri_to_cstri8 (const const_stritype stri, errinfotype *err_info)

  {
    cstritype cstri;

  /* stri_to_cstri8 */
    if (unlikely(stri->size > MAX_CSTRI_LEN / MAX_UTF8_EXPANSION_FACTOR ||
                 !ALLOC_CSTRI(cstri, max_utf8_size(stri->size)))) {
      *err_info = MEMORY_ERROR;
      cstri = NULL;
    } else {
      stri_to_os_utf8((ustritype) cstri, stri->mem, stri->size, err_info);
      if (unlikely(*err_info != OKAY_NO_ERROR)) {
        free_cstri8(cstri, stri);
        cstri = NULL;
      } /* if */
    } /* if */
    return cstri;
  } /* stri_to_cstri8 */



/**
 *  Create an ISO-8859-1 encoded bstring from a Seed7 UTF-32 string.
 *  The memory for the bstring is allocated. No zero byte is added
 *  to the end of the bstring. No special action is done, when the
 *  UTF-32 string contains a null character.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param err_info Unchanged when the function succeeds or
 *                  MEMORY_ERROR when the memory allocation failed or
 *                  RANGE_ERROR when stri contains a character
 *                        that is higher than the highest
 *                        allowed ISO-8859-1 character (255).
 *  @return an ISO-8859-1 encoded bstring or
 *          NULL, when the memory allocation failed or when the
 *          conversion failed (the error is indicated by err_info).
 */
bstritype stri_to_bstri (const const_stritype stri, errinfotype *err_info)

  {
    register const strelemtype *str;
    register uchartype *ustri;
    register memsizetype pos;
    bstritype bstri;

  /* stri_to_bstri */
    if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(bstri, stri->size))) {
      *err_info = MEMORY_ERROR;
    } else {
      bstri->size = stri->size;
      str = stri->mem;
      ustri = bstri->mem;
      for (pos = 0; pos < stri->size; pos++) {
        if (unlikely(str[pos] >= 256)) {
          FREE_BSTRI(bstri, bstri->size);
          *err_info = RANGE_ERROR;
          return NULL;
        } /* if */
        ustri[pos] = (uchartype) str[pos];
      } /* for */
    } /* if */
    return bstri;
  } /* stri_to_bstri */



/**
 *  Create an UTF-8 encoded bstring from a Seed7 UTF-32 string.
 *  The memory for the bstring is allocated. No zero byte is added
 *  to the end of the bstring. No special action is done, when
 *  the original string contains a null character.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @return an UTF-8 encoded bstring or
 *          NULL, when the memory allocation failed.
 */
bstritype stri_to_bstri8 (const_stritype stri)

  {
    bstritype resized_bstri;
    bstritype bstri;

  /* stri_to_bstri8 */
    if (stri->size > MAX_BSTRI_LEN / MAX_UTF8_EXPANSION_FACTOR) {
      bstri = NULL;
    } else if (ALLOC_BSTRI_SIZE_OK(bstri, max_utf8_size(stri->size))) {
      bstri->size = stri_to_utf8(bstri->mem, stri->mem, stri->size);
      REALLOC_BSTRI_SIZE_OK(resized_bstri, bstri, max_utf8_size(stri->size), bstri->size);
      if (resized_bstri == NULL) {
        FREE_BSTRI(bstri, max_utf8_size(stri->size));
        bstri = NULL;
      } else {
        bstri = resized_bstri;
        COUNT3_BSTRI(max_utf8_size(stri->size), bstri->size);
      } /* if */
    } /* if */
    return bstri;
  } /* stri_to_bstri8 */



#ifdef CONSOLE_WCHAR
bstritype stri_to_bstriw (const_stritype stri)

  {
    errinfotype err_info = OKAY_NO_ERROR;
    memsizetype wstri_size;
    bstritype resized_bstri;
    bstritype bstri;

  /* stri_to_bstriw */
    if (stri->size > ((MAX_BSTRI_LEN / sizeof(os_chartype)) / 2)) {
      bstri = NULL;
    } else if (ALLOC_BSTRI_SIZE_OK(bstri, stri->size * 2 * sizeof(os_chartype))) {
      wstri_size = stri_to_wstri((wstritype) bstri->mem, stri->mem, stri->size, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        FREE_BSTRI(bstri, stri->size * 2 * sizeof(os_chartype));
        bstri = NULL;
      } else {
        REALLOC_BSTRI_SIZE_OK(resized_bstri, bstri,
            stri->size * 2 * sizeof(os_chartype), wstri_size * sizeof(os_chartype));
        if (resized_bstri == NULL) {
          FREE_BSTRI(bstri, stri->size * 2 * sizeof(os_chartype));
          bstri = NULL;
        } else {
          bstri = resized_bstri;
          COUNT3_BSTRI(stri->size * 2 * sizeof(os_chartype), wstri_size * sizeof(os_chartype));
          bstri->size = wstri_size * sizeof(os_chartype);
        } /* if */
      } /* if */
    } /* if */
    return bstri;
  } /* stri_to_bstriw */
#endif



#ifdef OUT_OF_ORDER
bstritype stri_to_os_bstri (const_stritype stri)

  {
    bstritype bstri;

  /* stri_to_os_bstri */
    if (unlikely(stri->size > MAX_OS_BSTRI_SIZE)) {
      *err_info = MEMORY_ERROR;
      result = NULL;
    } else {
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(bstri, OS_BSTRI_SIZE(stri->size)))) {
        *err_info = MEMORY_ERROR;
      } else {
        conv_to_os_stri((os_stritype) bstri->mem, stri->mem, stri->size, err_info);
        if (unlikely(*err_info != OKAY_NO_ERROR)) {
          FREE_BSTRI(bstri, OS_BSTRI_SIZE(stri->size));
          result = NULL;
        } /* if */
      } /* if */
    } /* if */
    return bstri;
  } /* stri_to_os_bstri */
#endif



/**
 *  Copy an ISO-8859-1 (Latin-1) encoded C string to a Seed7 string.
 *  The memory for the UTF-32 encoded Seed7 string is allocated.
 *  @param cstri Null terminated ISO-8859-1 encoded C string.
 *  @return an UTF-32 encoded Seed7 string or
 *          NULL, when the memory allocation failed.
 */
stritype cstri_to_stri (const_cstritype cstri)

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
stritype cstri_to_stri (const_cstritype cstri)

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



/**
 *  Copy an UTF-8 encoded C string to a Seed7 string.
 *  The memory for the UTF-32 encoded Seed7 string is allocated.
 *  @param cstri Null terminated UTF-8 encoded C string.
 *  @return an UTF-32 encoded Seed7 string or
 *          NULL, when the memory allocation failed or when
 *          illegal UTF-8 encodings are used.
 */
stritype cstri8_to_stri (const_cstritype cstri)

  {
    memsizetype length;
    memsizetype stri_size;
    stritype resized_stri;
    stritype stri;

  /* cstri8_to_stri */
    length = strlen(cstri);
    if (ALLOC_STRI_CHECK_SIZE(stri, length)) {
      if (utf8_to_stri(stri->mem, &stri_size, (const_ustritype) cstri, length) == 0) {
        REALLOC_STRI_SIZE_SMALLER(resized_stri, stri, length, stri_size);
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



/**
 *  Copy an UTF-8 or ISO-8859-1 encoded C string to a Seed7 string.
 *  The memory for the UTF-32 encoded Seed7 string is allocated.
 *  @param cstri Null terminated UTF-8 or ISO-8859-1 encoded C string.
 *  @return an UTF-32 encoded Seed7 string or
 *          NULL, when the memory allocation failed.
 */
stritype cstri8_or_cstri_to_stri (const_cstritype cstri)

  {
    stritype stri;

  /* cstri8_or_cstri_to_stri */
    stri = cstri8_to_stri(cstri);
    if (stri == NULL) {
      stri = cstri_to_stri(cstri);
    } /* if */
    return stri;
  } /* cstri8_or_cstri_to_stri */



/**
 *  Convert a Seed7 UTF-32 string to a null terminated os_stritype string.
 *  The memory for the null terminated os_stritype string is allocated.
 *  The os_stritype result must be freed with the macro os_stri_free().
 *  Many system calls have parameters with null terminated os_stritype
 *  strings. System calls are defined in "version.h" and "os_decls.h".
 *  They are prefixed with os_ and use strings of the type os_stritype.
 *  Depending on the operating system os_stritype can describe byte or
 *  wide char strings. The encoding can be Latin-1, UTF-8, UTF-16 or
 *  it can use a code page.
 *  @param stri Seed7 UTF-32 string to be converted.
 *  @param err_info Unchanged when the function succeeds or
 *                  MEMORY_ERROR when the memory allocation failed or
 *                  RANGE_ERROR when the conversion failed.
 *  @return a null terminated os_stritype value used by system calls or
 *          NULL, when an error occurred.
 */
os_stritype stri_to_os_stri (const_stritype stri, errinfotype *err_info)

  {
    os_stritype result;

  /* stri_to_os_stri */
    if (unlikely(stri->size > MAX_OS_STRI_SIZE)) {
      *err_info = MEMORY_ERROR;
      result = NULL;
    } else {
      if (unlikely(!os_stri_alloc(result, OS_STRI_SIZE(stri->size)))) {
        *err_info = MEMORY_ERROR;
      } else {
        conv_to_os_stri(result, stri->mem, stri->size, err_info);
        if (unlikely(*err_info != OKAY_NO_ERROR)) {
          os_stri_free(result);
          result = NULL;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* stri_to_os_stri */



/**
 *  Convert a null terminated os_stritype string to a Seed7 UTF-32 string.
 *  Many system calls return null terminated os_stritype strings. System
 *  calls are defined in "version.h" and "os_decls.h". They are prefixed
 *  with os_ and use strings of the type os_stritype. Depending on the
 *  operating system os_stritype can describe byte or wide char strings.
 *  The encoding can be Latin-1, UTF-8, UTF-16 or it can use a code page.
 *  @param os_stri null terminated os_stritype string to be converted.
 *  @param err_info Unchanged when the function succeeds or
 *                  MEMORY_ERROR when the memory allocation failed.
 *  @return a Seed7 UTF-32 string or
 *          NULL, when an error occurred.
 */
stritype os_stri_to_stri (const_os_stritype os_stri, errinfotype *err_info)

  {
    stritype stri;

  /* os_stri_to_stri */
    stri = conv_from_os_stri(os_stri, os_stri_strlen(os_stri));
    if (unlikely(stri == NULL)) {
      *err_info = MEMORY_ERROR;
    } /* if */
    return stri;
  } /* os_stri_to_stri */



stritype stri_to_standard_path (const stritype stri)

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



/**
 *  Convert a path returned by a system call to a Seed7 standard path.
 *  System calls are defined in "version.h" and "os_decls.h". They are
 *  prefixed with os_ and use system paths of the type os_stritype.
 *  Depending on the operating system os_stritype can describe byte or
 *  wide char strings. The encoding can be Latin-1, UTF-8, UTF-16 or
 *  it can use a code page. Beyond the conversion from os_stritype a
 *  mapping from drive letters might take place on some operating
 *  systems.
 *  @param os_path null terminated os_stritype path to be converted.
 *  @param err_info Unchanged when the function succeeds or
 *                  MEMORY_ERROR when the memory allocation failed.
 *  @return an UTF-32 encoded Seed7 standard path or
 *          NULL, when the memory allocation failed.
 */
stritype cp_from_os_path (const_os_stritype os_path, errinfotype *err_info)

  {
    stritype result;

  /* cp_from_os_path */
    result = os_stri_to_stri(os_path, err_info);
    if (likely(result != NULL)) {
      result = stri_to_standard_path(result);
      if (unlikely(result == NULL)) {
        *err_info = MEMORY_ERROR;
      } /* if */
    } /* if */
    return result;
  } /* cp_from_os_path */



#ifdef OS_PATH_HAS_DRIVE_LETTERS



#ifdef EMULATE_ROOT_CWD
void setEmulatedCwd (const os_stritype os_path)

  {
    memsizetype position;

  /* setEmulatedCwd */
    /* printf("setEmulatedCwd(\"%ls\")\n", os_path); */
    if (!IS_EMULATED_ROOT(os_path)) {
      for (position = 0; os_path[position] != '\0'; position++) {
        if (os_path[position] == '\\') {
          os_path[position] = '/';
        } /* if */
      } /* for */
      if (position >= 2 && os_path[position - 1] == '/') {
        os_path[position - 1] = '\0';
      } /* if */
      if (((os_path[0] >= 'a' && os_path[0] <= 'z') ||
           (os_path[0] >= 'A' && os_path[0] <= 'Z')) &&
          os_path[1] == ':') {
        os_path[1] = (os_chartype) tolower(os_path[0]);
        os_path[0] = '/';
      } /* if */
    } /* if */
    if (current_emulated_cwd != NULL &&
        current_emulated_cwd != emulated_root) {
      os_stri_free(current_emulated_cwd);
    } /* if */
    current_emulated_cwd = os_path;
    /* printf("current_emulated_cwd=\"%ls\"\n", current_emulated_cwd); */
  } /* setEmulatedCwd */



static os_stritype append_path (const const_os_stritype absolutePath,
    const strelemtype *const relativePathChars,
    memsizetype relativePathSize, int *path_info, errinfotype *err_info)

  {
    memsizetype abs_path_length;
    memsizetype result_len;
    os_stritype abs_path_end;
    const_os_stritype rel_path_start;
    os_stritype result;

  /* append_path */
    /* printf("append_path(\"%s\", \"%s\")\n", absolutePath, relative_path); */
    /* absolutePath[0] is always '/'. */
    if (absolutePath[1] == '\0') {
      abs_path_length = 0;
    } else {
      abs_path_length = os_stri_strlen(absolutePath);
    } /* if */
    result_len = abs_path_length + OS_STRI_SIZE(relativePathSize) + 3;
    if (unlikely(!os_stri_alloc(result, result_len))) {
      *err_info = MEMORY_ERROR;
    } else {
      memcpy(result, absolutePath, abs_path_length * sizeof(os_chartype));
      result[abs_path_length] = '/';
      conv_to_os_stri(&result[abs_path_length + 2], relativePathChars,
          relativePathSize, err_info);
      if (unlikely(*err_info != OKAY_NO_ERROR)) {
        os_stri_free(result);
        result = NULL;
      } else {
        abs_path_end = &result[abs_path_length];
        rel_path_start = &result[abs_path_length + 2];
        while (*rel_path_start != '\0') {
          if (rel_path_start[0] == '.' && rel_path_start[1] == '.' &&
              (rel_path_start[2] == '/' || rel_path_start[2] == '\0')) {
            rel_path_start += 2;
            if (abs_path_end > result) {
              do {
                abs_path_end--;
              } while (*abs_path_end != '/');
            } /* if */
          } else if (rel_path_start[0] == '.' &&
              (rel_path_start[1] == '/' || rel_path_start[1] == '\0')) {
            rel_path_start++;
          } else if (*rel_path_start == '/') {
            rel_path_start++;
          } else {
            do {
              abs_path_end++;
              *abs_path_end = *rel_path_start;
              rel_path_start++;
            } while (*rel_path_start != '/' && *rel_path_start != '\0');
            abs_path_end++;
            *abs_path_end = '/';
          } /* if */
        } /* while */
        if (abs_path_end == result) {
          *err_info = RANGE_ERROR;
          *path_info = PATH_IS_EMULATED_ROOT;
          os_stri_free(result);
          result = NULL;
        } else {
          *abs_path_end = '\0';
          if (result[1] >= 'a' && result[1] <= 'z') {
            if (result[2] == '\0') {
              /* "/c"   is mapped to "c:\"  */
              result[0] = result[1];
              result[1] = ':';
              result[2] = '\\';
              result[3] = '\0';
            } else if (unlikely(result[2] != '/')) {
              /* "/cd"  cannot be mapped to a drive letter */
              *err_info = RANGE_ERROR;
              *path_info = PATH_NOT_MAPPED;
              os_stri_free(result);
              result = NULL;
            } else {
              /* "/c/d" is mapped to "c:\d" */
              result[0] = result[1];
              result[1] = ':';
              result[2] = '\\';
            } /* if */
          } else {
            /* "/C"  cannot be mapped to a drive letter */
            *err_info = RANGE_ERROR;
            *path_info = PATH_NOT_MAPPED;
            os_stri_free(result);
            result = NULL;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    /* printf("append_path(\"%s\", \"%s\") --> \"%s\"\n",
        absolutePath, relative_path, result); */
    return result;
  } /* append_path */

#else



static os_stritype map_to_drive_letter (const strelemtype *const pathChars,
    memsizetype pathSize, int *path_info, errinfotype *err_info)

  {
    os_stritype result;

  /* map_to_drive_letter */
    if (unlikely(pathSize == 0)) {
      /* "/"    cannot be mapped to a drive letter */
      *err_info = RANGE_ERROR;
      *path_info = PATH_IS_EMULATED_ROOT;
      result = NULL;
    } else if (pathChars[0] >= 'a' && pathChars[0] <= 'z') {
      if (pathSize == 1) {
        /* "/c"   is mapped to "c:\"  */
        if (unlikely(!os_stri_alloc(result, 3))) {
          *err_info = MEMORY_ERROR;
        } else {
          result[0] = (os_chartype) pathChars[0];
          result[1] = ':';
          result[2] = '\\';
          result[3] = '\0';
        } /* if */
      } else if (unlikely(pathChars[1] != '/')) {
        /* "/cd"  cannot be mapped to a drive letter */
        *err_info = RANGE_ERROR;
        *path_info = PATH_NOT_MAPPED;
        result = NULL;
      } else {
        /* "/c/d" is mapped to "c:\d" */
        if (unlikely(!os_stri_alloc(result, OS_STRI_SIZE(pathSize - 2) + 3))) {
          *err_info = MEMORY_ERROR;
        } else {
          result[0] = (os_chartype) pathChars[0];
          result[1] = ':';
          result[2] = '\\';
          conv_to_os_stri(&result[3], &pathChars[2], pathSize - 2, err_info);
          if (unlikely(*err_info != OKAY_NO_ERROR)) {
            os_stri_free(result);
            result = NULL;
          } /* if */
        } /* if */
      } /* if */
    } else {
      /* "/C"  cannot be mapped to a drive letter */
      *err_info = RANGE_ERROR;
      *path_info = PATH_NOT_MAPPED;
      result = NULL;
    } /* if */
    return result;
  } /* map_to_drive_letter */

#endif



/**
 *  Convert a Seed7 standard path to a path used by system calls.
 *  The memory for the null terminated os_stritype path is allocated.
 *  The os_stritype result must be freed with the macro os_stri_free().
 *  System calls are defined in "version.h" and "os_decls.h". They are
 *  prefixed with os_ and use system paths of the type os_stritype.
 *  Depending on the operating system os_stritype can describe byte or
 *  wide char strings. The encoding can be Latin-1, UTF-8, UTF-16 or
 *  it can use a code page. Beyond the conversion to os_stritype a
 *  mapping to drive letters might take place on some operating systems.
 *  @param std_path UTF-32 encoded Seed7 standard path to be converted.
 *  @param path_info Unchanged when the function succeeds or
 *                   PATH_IS_EMULATED_ROOT when the path is "/".
 *                   PATH_NOT_MAPPED when the path cannot be mapped.
 *  @param err_info Unchanged when the function succeeds or
 *                  MEMORY_ERROR when the memory allocation failed or
 *                  RANGE_ERROR when the path is not a standard path.
 *  @return a null terminated os_stritype path used by system calls or
 *          NULL, when an error occurred.
 */
os_stritype cp_to_os_path (const_stritype std_path, int *path_info,
    errinfotype *err_info)

  {
    os_stritype result;

  /* cp_to_os_path */
#ifdef TRACE_STRIUTL
    printf("BEGIN cp_to_os_path(%lx, %d)\n", std_path, *err_info);
#endif
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#ifdef FORBID_DRIVE_LETTERS
    if (unlikely(std_path->size >= 2 && (std_path->mem[std_path->size - 1] == '/' ||
                 (std_path->mem[1] == ':' &&
                 ((std_path->mem[0] >= 'a' && std_path->mem[0] <= 'z') ||
                  (std_path->mem[0] >= 'A' && std_path->mem[0] <= 'Z')))))) {
#else
    if (unlikely(std_path->size >= 2 && std_path->mem[std_path->size - 1] == '/')) {
#endif
#else
    if (unlikely(std_path->size >= 2 && std_path->mem[std_path->size - 1] == '/' &&
                 (std_path->size != 3 || std_path->mem[1] != ':' ||
                 ((std_path->mem[0] < 'a' || std_path->mem[0] > 'z') &&
                  (std_path->mem[0] < 'A' || std_path->mem[0] > 'Z'))))) {
#endif
      *err_info = RANGE_ERROR;
      result = NULL;
    } else if (unlikely(stri_charpos(std_path, '\\') != NULL)) {
      *err_info = RANGE_ERROR;
      result = NULL;
    } else if (std_path->size == 0) {
      if (unlikely(!os_stri_alloc(result, 0))) {
        *err_info = MEMORY_ERROR;
      } else {
        result[0] = '\0';
      } /* if */
    } else {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (std_path->size >= 1 && std_path->mem[0] == '/') {
        /* Absolute path: Try to map the path to a drive letter */
#ifdef EMULATE_ROOT_CWD
        result = append_path(emulated_root, &std_path->mem[1], std_path->size - 1,
                             path_info, err_info);
#else
        result = map_to_drive_letter(&std_path->mem[1], std_path->size - 1, err_info);
#endif
      } else {
#ifdef EMULATE_ROOT_CWD
        result = append_path(current_emulated_cwd, std_path->mem, std_path->size,
                             path_info, err_info);
#else
        if (unlikely(!os_stri_alloc(result, OS_STRI_SIZE(std_path->size)))) {
          *err_info = MEMORY_ERROR;
        } else {
          conv_to_os_stri(result, std_path->mem, std_path->size, err_info);
          if (unlikely(*err_info != OKAY_NO_ERROR)) {
            os_stri_free(result);
            result = NULL;
          } /* if */
        } /* if */
#endif
      } /* if */
#else
      if (unlikely(!os_stri_alloc(result, OS_STRI_SIZE(std_path->size)))) {
        *err_info = MEMORY_ERROR;
      } else {
        conv_to_os_stri(result, std_path->mem, std_path->size, err_info);
        if (unlikely(*err_info != OKAY_NO_ERROR)) {
          os_stri_free(result);
          result = NULL;
        } /* if */
      } /* if */
#endif
    } /* if */
#ifdef TRACE_STRIUTL
    printf("END cp_to_os_path(%lx, %d) ==> %lx\n", std_path, *err_info, result);
#endif
    return result;
  } /* cp_to_os_path */

#else



/**
 *  Convert a Seed7 standard path to a path used by system calls.
 *  The memory for the null terminated os_stritype path is allocated.
 *  The os_stritype result must be freed with the macro os_stri_free().
 *  System calls are defined in "version.h" and "os_decls.h". They are
 *  prefixed with os_ and use system paths of the type os_stritype.
 *  Depending on the operating system os_stritype can describe byte or
 *  wide char strings. The encoding can be Latin-1, UTF-8, UTF-16 or
 *  it can use a code page. Beyond the conversion to os_stritype a
 *  mapping to drive letters might take place on some operating systems.
 *  @param std_path UTF-32 encoded Seed7 standard path to be converted.
 *  @param path_info Unchanged when the function succeeds or
 *                   PATH_IS_EMULATED_ROOT when the path is "/".
 *                   PATH_NOT_MAPPED when the path cannot be mapped.
 *  @param err_info Unchanged when the function succeeds or
 *                  MEMORY_ERROR when the memory allocation failed or
 *                  RANGE_ERROR when the path is not a standard path.
 *  @return a null terminated os_stritype path used by system calls or
 *          NULL, when an error occurred.
 */
os_stritype cp_to_os_path (const_stritype std_path, int *path_info,
    errinfotype *err_info)

  {
    os_stritype result;

  /* cp_to_os_path */
#ifdef TRACE_STRIUTL
    printf("BEGIN cp_to_os_path(%lx, %d)\n", std_path, *err_info);
#endif
    if (unlikely(std_path->size >= 2 && std_path->mem[std_path->size - 1] == '/')) {
      *err_info = RANGE_ERROR;
      result = NULL;
    } else if (unlikely(std_path->size > MAX_OS_STRI_SIZE ||
                        !os_stri_alloc(result, OS_STRI_SIZE(std_path->size)))) {
      *err_info = MEMORY_ERROR;
      result = NULL;
    } else {
      conv_to_os_stri(result, std_path->mem, std_path->size, err_info);
      if (unlikely(*err_info != OKAY_NO_ERROR)) {
        os_stri_free(result);
        result = NULL;
      } /* if */
    } /* if */
#ifdef TRACE_STRIUTL
    printf("END cp_to_os_path(%lx, %d) ==> %lx\n", std_path, *err_info, result);
#endif
    return result;
  } /* cp_to_os_path */

#endif



static void escape_command (const const_os_stritype inBuffer, os_stritype outBuffer,
    errinfotype *err_info)

  {
    memsizetype inPos;
    memsizetype outPos;
    booltype quote_path;

  /* escape_command */
    quote_path = FALSE;
    for (inPos = 0, outPos = 0; inBuffer[inPos] != '\0'; inPos++, outPos++) {
      switch (inBuffer[inPos]) {
#ifdef ESCAPE_SHELL_COMMANDS
        case '\t': case ' ':  case '!':  case '\"': case '$':
        case '&':  case '\'': case '(':  case ')':  case '*':
        case ',':  case ':':  case ';':  case '<':  case '=':
        case '>':  case '?':  case '[':  case '\\': case ']':
        case '^':  case '`':  case '{':  case '|':  case '}':
        case '~':
          outBuffer[outPos] = '\\';
          outPos++;
          outBuffer[outPos] = inBuffer[inPos];
          break;
        case '\n':
          *err_info = RANGE_ERROR;
          break;
#else
        case ' ':  case '%':  case '&':  case '\'': case '(':
        case ')':  case ',':  case ';':  case '=':  case '^':
        case '~':  case 160:
          quote_path = TRUE;
          outBuffer[outPos] = inBuffer[inPos];
          break;
        case ':':
          if (inPos == 1 &&
              inBuffer[0] >= 'a' && inBuffer[0] <= 'z') {
            outBuffer[outPos] = inBuffer[inPos];
          } else {
            *err_info = RANGE_ERROR;
          } /* if */
          break;
        case '\"': case '*':  case '<':  case '>':  case '?':
        case '|':  case '\n': case '\r':
          *err_info = RANGE_ERROR;
          break;
#endif
#if PATH_DELIMITER != '/'
        case '/':
          outBuffer[outPos] = PATH_DELIMITER;
          break;
#endif
        default:
          outBuffer[outPos] = inBuffer[inPos];
          break;
      } /* switch */
    } /* for */
    if (quote_path) {
      memmove(&outBuffer[1], outBuffer, sizeof(os_chartype) * outPos);
      outBuffer[0] = '\"';
      outBuffer[outPos + 1] = '\"';
      outBuffer[outPos + 2] = '\0';
    } else {
      outBuffer[outPos] = '\0';
    } /* if */
  } /* escape_command */



os_stritype cp_to_command (const const_stritype commandPath,
    const const_stritype parameters, errinfotype *err_info)

  {
    os_stritype os_commandPath;
    os_stritype os_parameters;
    memsizetype param_len;
    memsizetype result_len;
    int path_info;
    os_stritype result;

  /* cp_to_command */
#ifdef EMULATE_ROOT_CWD
    if (stri_charpos(commandPath, '/') != NULL) {
      os_commandPath = cp_to_os_path(commandPath, &path_info, err_info);
    } else if (unlikely(stri_charpos(commandPath, '\\') != NULL)) {
      *err_info = RANGE_ERROR;
      result = NULL;
    } else {
      os_commandPath = stri_to_os_stri(commandPath, err_info);
    } /* if */
#else
    os_commandPath = cp_to_os_path(commandPath, &path_info, err_info);
#endif
    if (unlikely(*err_info != OKAY_NO_ERROR)) {
      result = NULL;
    } else {
      os_parameters = stri_to_os_stri(parameters, err_info);
      if (unlikely(os_parameters == NULL)) {
        result = NULL;
      } else {
        param_len = os_stri_strlen(os_parameters);
        if (unlikely(MAX_OS_STRI_SIZE - 4 < param_len ||
                     os_stri_strlen(os_commandPath) > (MAX_OS_STRI_SIZE - 4 - param_len) / 3)) {
          *err_info = MEMORY_ERROR;
          result = NULL;
        } else {
          result_len = 3 * os_stri_strlen(os_commandPath) +
                       os_stri_strlen(os_parameters) + 4;
          if (unlikely(!os_stri_alloc(result, result_len))) {
            *err_info = MEMORY_ERROR;
          } else {
            escape_command(os_commandPath, result, err_info);
            if (*err_info != OKAY_NO_ERROR) {
              os_stri_free(result);
              result = NULL;
            } else {
              result_len = os_stri_strlen(result);
#ifdef QUOTE_WHOLE_SHELL_COMMAND
              if (result[0] == '\"') {
                memmove(&result[1], result, sizeof(os_chartype) * result_len);
                result[0] = '\"';
                result_len++;
              } /* if */
#endif
              if (os_parameters[0] != ' ' && os_parameters[0] != '\0') {
                result[result_len] = ' ';
                result_len++;
              } /* if */
              memcpy(&result[result_len], os_parameters,
                  sizeof(os_chartype) * (param_len + 1));
#ifdef QUOTE_WHOLE_SHELL_COMMAND
              if (result[0] == '\"' && result[1] == '\"') {
                result_len = os_stri_strlen(result);
                result[result_len] = '\"';
                result[result_len + 1] = '\0';
              } /* if */
#endif
            } /* if */
          } /* if */
        } /* if */
        os_stri_free(os_parameters);
      } /* if */
      os_stri_free(os_commandPath);
    } /* if */
    /* printf("cp_to_command -> %ls\n", result); */
    return result;
  } /* cp_to_command */



#ifdef PATHS_RELATIVE_TO_EXECUTABLE
stritype relativeToProgramPath (const const_stritype basePath,
    const const_cstritype dir)

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
      if (likely(ALLOC_STRI_SIZE_OK(result, dir_path_size - 3 + strlen(dir)))) {
        result->size = dir_path_size - 3 + strlen(dir);
        memcpy(result->mem, basePath->mem, (dir_path_size - 3) * sizeof(strelemtype));
        cstri_expand2(&result->mem[dir_path_size - 3], dir);
      } /* if */
    } else {
      result = cstri_to_stri("");
    } /* if */
    return result;
  } /* relativeToProgramPath */
#endif
