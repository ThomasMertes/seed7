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


#ifdef OS_STRI_WCHAR

#define MAX_OS_STRI_SIZE (((MAX_MEMSIZETYPE / sizeof(os_chartype)) - 1) / 2)
#define OS_STRI_SIZE(size) ((size) * 2)

#elif defined OS_STRI_USES_CODEPAGE

#define MAX_OS_STRI_SIZE ((MAX_MEMSIZETYPE / sizeof(os_chartype)) - 1)
#define OS_STRI_SIZE(size) (size)
int codepage = DEFAULT_CODEPAGE;

#elif defined OS_STRI_UTF8

#define MAX_OS_STRI_SIZE (MAX_CSTRI_LEN / MAX_UTF8_EXPANSION_FACTOR)
#define OS_STRI_SIZE(size) max_utf8_size(size)

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



#ifdef OS_STRI_WCHAR
#ifdef ANSI_C

static INLINE void conv_to_os_stri (os_stritype os_stri, const strelemtype *strelem,
    memsizetype len, errinfotype *err_info)
#else

static INLINE void conv_to_os_stri (os_stri, strelem, len, err_info)
os_stritype os_stri;
strelemtype *strelem;
memsizetype len;
errinfotype *err_info;
#endif

  { /* conv_to_os_stri */
    for (; len > 0; os_stri++, strelem++, len--) {
      if (likely(*strelem <= 0xFFFF)) {
        *os_stri = (os_chartype) *strelem;
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
    *os_stri = 0;
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



#ifdef ANSI_C

static INLINE void conv_to_os_stri (os_stritype os_stri, const strelemtype *strelem,
    memsizetype len, errinfotype *err_info)
#else

static INLINE void conv_to_os_stri (os_stri, strelem, len, err_info)
os_stritype os_stri;
strelemtype *strelem;
memsizetype len;
errinfotype *err_info;
#endif

  {
    unsigned char ch;

  /* conv_to_os_stri */
    if (codepage == 437) {
      for (; len > 0; os_stri++, strelem++, len--) {
        if (*strelem <= 127) {
          *os_stri = (os_chartype) *strelem;
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
          *os_stri = (os_chartype) *strelem;
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
    *os_stri = (os_chartype) 0;
  } /* conv_to_os_stri */

#elif defined OS_STRI_UTF8



#ifdef ANSI_C

static INLINE void conv_to_os_stri (os_stritype os_stri, const strelemtype *strelem,
    memsizetype len, errinfotype *err_info)
#else

static INLINE void conv_to_os_stri (os_stri, strelem, len, err_info)
os_stritype os_stri;
strelemtype *strelem;
memsizetype len;
errinfotype *err_info;
#endif

  {
    memsizetype length;

  /* conv_to_os_stri */
    length = stri_to_utf8((ustritype) os_stri, strelem, len);
    os_stri[length] = '\0';
  } /* conv_to_os_stri */

#endif



#if defined OS_STRI_WCHAR
#ifdef ANSI_C

static memsizetype wstri_expand (strelemtype *dest_stri, const_wstritype wstri, memsizetype len)
#else

static memsizetype wstri_expand dest_(stri, wstri, len)
strelemtype *dest_stri;
wstritype wstri;
memsizetype len;
errinfotype *err_info;
#endif

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



#ifdef ANSI_C

static stritype conv_from_os_stri (const const_os_stritype os_stri)
#else

static stritype conv_from_os_stri (os_stri)
os_stritype os_stri;
#endif

  {
    memsizetype length;
    stritype stri;

  /* conv_from_os_stri */
    length = 0;
    while (os_stri[length] != 0) {
      length++;
    } /* while */
    if (ALLOC_STRI_CHECK_SIZE(stri, length)) {
      stri->size = wstri_expand(stri->mem, os_stri, length);
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


#ifdef ANSI_C

static stritype conv_from_os_stri (const const_os_stritype os_stri)
#else

static stritype conv_from_os_stri (os_stri)
os_stritype os_stri;
#endif

  {
    memsizetype length;
    memsizetype pos;
    stritype stri;

  /* conv_from_os_stri */
    length = 0;
    while (os_stri[length] != 0) {
      length++;
    } /* while */
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
    } else if (ALLOC_CSTRI(cstri, max_utf8_size(stri->size))) {
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
    } else if (ALLOC_BSTRI_SIZE_OK(bstri, max_utf8_size(stri->size))) {
#ifdef UTF32_STRINGS
      bstri->size = stri_to_utf8(bstri->mem, stri->mem, stri->size);
#else
      memcpy(bstri->mem, stri->mem, stri->size);
      bstri->size = stri->size;
#endif
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



#if defined OS_STRI_WCHAR || defined OS_STRI_USES_CODEPAGE



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
    if (unlikely(stri->size > MAX_OS_STRI_SIZE)) {
      *err_info = MEMORY_ERROR;
      result = NULL;
    } else {
      if (unlikely(!os_stri_alloc(result, OS_STRI_SIZE(stri->size)))) {
        *err_info = MEMORY_ERROR;
      } else {
        stri_export((ustritype) result, stri);
      } /* if */
    } /* if */
    return result;
  } /* stri_to_os_stri */

#endif



#ifdef ANSI_C

stritype os_stri_to_stri (const_os_stritype os_stri, errinfotype *err_info)
#else

stritype os_stri_to_stri (os_stri, err_info)
os_stritype os_stri;
errinfotype *err_info;
#endif

  {
    stritype stri;

  /* os_stri_to_stri */
#if defined OS_STRI_WCHAR || defined OS_STRI_USES_CODEPAGE
    stri = conv_from_os_stri(os_stri);
#elif defined OS_STRI_UTF8
    stri = cstri8_or_cstri_to_stri(os_stri);
#else
    stri = cstri_to_stri(os_stri);
#endif
    if (unlikely(stri == NULL)) {
      *err_info = MEMORY_ERROR;
    } /* if */
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

stritype cp_from_os_path (const_os_stritype os_stri, errinfotype *err_info)
#else

stritype cp_from_os_path (os_stri, err_info)
os_stritype os_stri;
errinfotype *err_info;
#endif

  {
    stritype result;

  /* cp_from_os_path */
    result = os_stri_to_stri(os_stri, err_info);
    if (likely(result != NULL)) {
      result = stri_to_standard_path(result);
      if (unlikely(result == NULL)) {
        *err_info = MEMORY_ERROR;
      } /* if */
    } /* if */
    return result;
  } /* cp_from_os_path */



#if defined OS_STRI_WCHAR || defined OS_STRI_USES_CODEPAGE



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
    } else if (unlikely(stri_charpos(stri, '\\') != NULL)) {
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
            if (unlikely(!os_stri_alloc(result, 3))) {
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
            if (unlikely(!os_stri_alloc(result, OS_STRI_SIZE(stri->size - 3) + 3))) {
              *err_info = MEMORY_ERROR;
            } else {
              result[0] = (os_chartype) stri->mem[1];
              result[1] = ':';
              result[2] = '\\';
              conv_to_os_stri(&result[3], &stri->mem[3], stri->size - 3, err_info);
            } /* if */
          } /* if */
        } else {
          /* "/C"  cannot be mapped to a drive letter */
          *err_info = RANGE_ERROR;
          result = NULL;
        } /* if */
      } else {
#endif
        if (unlikely(!os_stri_alloc(result, OS_STRI_SIZE(stri->size)))) {
          *err_info = MEMORY_ERROR;
        } else {
          conv_to_os_stri(result, stri->mem, stri->size, err_info);
        } /* if */
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      } /* if */
#endif
      if (unlikely(*err_info != OKAY_NO_ERROR && result != NULL)) {
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
    } else if (unlikely(stri->size > MAX_OS_STRI_SIZE ||
                        !os_stri_alloc(result, OS_STRI_SIZE(stri->size)))) {
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



#ifdef OUT_OF_ORDER
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
#ifdef ESCAPE_SHELL_COMMANDS
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
        memmove(&cmd[outPos + 2], &cmd[inPos], sizeof(os_chartype) * (os_stri_strlen(&cmd[inPos]) + 1));
        memmove(&cmd[1], cmd, sizeof(os_chartype) * outPos);
        cmd[0] = '"';
        cmd[outPos + 1] = '"';
        if (os_stri_strchr(&cmd[outPos + 2], '\"') != NULL) {
          outPos = os_stri_strlen(cmd);
          memmove(&cmd[1], cmd, sizeof(os_chartype) * outPos);
          cmd[0] = '"';
          cmd[outPos + 1] = '"';
          cmd[outPos + 2] = '\0';
        } /* if */
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
#endif



#ifdef ANSI_C

static void escape_command (const const_os_stritype inBuffer, os_stritype outBuffer,
    errinfotype *err_info)
#else

static void escape_command (inBuffer, outBuffer, err_info)
os_stritype inBuffer;
os_stritype outBuffer;
errinfotype *err_info;
#endif

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
        case ';':  case '<':  case '>':  case '?':  case '\\':
        case '`':  case '|':
          outBuffer[outPos] = '\\';
          outPos++;
          outBuffer[outPos] = inBuffer[inPos];
          break;
        case '\n':
          *err_info = RANGE_ERROR;
          break;
#else
        case ' ':  case '&':  case ',':  case ';':  case '=':
        case '^':  case '~':  case 160:
          quote_path = TRUE;
          outBuffer[outPos] = inBuffer[inPos];
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



#ifdef ANSI_C

os_stritype cp_to_command (const const_stritype commandPath,
    const const_stritype parameters, errinfotype *err_info)
#else

os_stritype cp_to_command (commandPath, parameters, err_info)
stritype commandPath;
stritype parameters;
errinfotype *err_info;
#endif

  {
    os_stritype os_commandPath;
    os_stritype os_parameters;
    memsizetype param_len;
    memsizetype result_len;
    os_stritype result;

  /* cp_to_command */
    os_commandPath = cp_to_os_path(commandPath, err_info);
    if (unlikely(os_commandPath == NULL)) {
      result = NULL;
    } else {
      os_parameters = stri_to_os_stri(parameters, err_info);
      if (unlikely(os_parameters == NULL)) {
        result = NULL;
      } else {
        param_len = os_stri_strlen(os_parameters);
        if (unlikely(MAX_OS_STRI_SIZE - 4 < param_len ||
                     os_stri_strlen(os_commandPath) > (MAX_OS_STRI_SIZE - 4 - param_len) >> 1)) {
          *err_info = MEMORY_ERROR;
          result = NULL;
        } else {
          result_len = 2 * os_stri_strlen(os_commandPath) +
                       os_stri_strlen(os_parameters) + 4;
          if (unlikely(!os_stri_alloc(result, result_len))) {
            *err_info = MEMORY_ERROR;
          } else {
            escape_command(os_commandPath, result, err_info);
            if (*err_info != OKAY_NO_ERROR) {
              os_stri_free(result);
              result = NULL;
            } else if (os_parameters[0] != '\0') {
              result_len = os_stri_strlen(result);
#ifndef ESCAPE_SHELL_COMMANDS
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
#ifndef ESCAPE_SHELL_COMMANDS
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
    return result;
  } /* cp_to_command */



#ifdef PATHS_RELATIVE_TO_EXECUTABLE
#ifdef ANSI_C

stritype relativeToProgramPath (const const_stritype basePath,
    const const_cstritype dir)
#else

stritype relativeToProgramPath (basePath, dir)
stritype basePath;
cstritype dir;
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
      if (likely(ALLOC_STRI_SIZE_OK(result, dir_path_size - 3 + strlen(dir)))) {
        result->size = dir_path_size - 3 + strlen(dir);
        memcpy(result->mem, basePath->mem, (dir_path_size - 3) * sizeof(strelemtype));
        cstri_expand(&result->mem[dir_path_size - 3], dir, strlen(dir));
      } /* if */
    } else {
      result = cstri_to_stri("");
    } /* if */
    return result;
  } /* relativeToProgramPath */
#endif
