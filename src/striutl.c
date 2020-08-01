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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/striutl.c                                       */
/*  Changes: 1991, 1992, 1993, 1994, 2005  Thomas Mertes            */
/*  Content: Procedures to work with wide char strings.             */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"

#include "version.h"
#include "common.h"
#include "heaputl.h"

#undef EXTERN
#define EXTERN
#include "striutl.h"



#ifdef WIDE_CHAR_STRINGS
#ifdef ANSI_C

static memsizetype stri_to_utf8 (ustritype out_stri, stritype in_stri)
#else

static memsizetype stri_to_utf8 (out_stri, in_stri)
ustritype out_stri;
stritype in_stri;
#endif

  {
    register ustritype ustri;
    register strelemtype *stri;
    memsizetype len;

  /* stri_to_utf8 */
    ustri = out_stri;
    stri = in_stri->mem;
    len = in_stri->size;
    for (; len > 0; stri++, len--) {
      if (*stri <= 0x7F) {
        *ustri++ = (uchartype) *stri;
      } else if (*stri <= 0x7FF) {
        *ustri++ = 0xC0 | (*stri >> 6);
        *ustri++ = 0x80 | (*stri & 0x3F);
      } else if (*stri <= 0xFFFF) {
        *ustri++ = 0xE0 | (*stri >> 12);
        *ustri++ = 0x80 | ((*stri >> 6) & 0x3F);
        *ustri++ = 0x80 | (*stri & 0x3F);
      } else if (*stri <= 0x1FFFFF) {
        *ustri++ = 0xF0 | (*stri >> 18);
        *ustri++ = 0x80 | ((*stri >> 12) & 0x3F);
        *ustri++ = 0x80 | ((*stri >> 6) & 0x3F);
        *ustri++ = 0x80 | (*stri & 0x3F);
      } else if (*stri <= 0x3FFFFFF) {
        *ustri++ = 0xF8 | (*stri >> 24);
        *ustri++ = 0x80 | ((*stri >> 18) & 0x3F);
        *ustri++ = 0x80 | ((*stri >> 12) & 0x3F);
        *ustri++ = 0x80 | ((*stri >> 6) & 0x3F);
        *ustri++ = 0x80 | (*stri & 0x3F);
      } else {
        *ustri++ = 0xFC | (*stri >> 30);
        *ustri++ = 0x80 | ((*stri >> 24) & 0x3F);
        *ustri++ = 0x80 | ((*stri >> 18) & 0x3F);
        *ustri++ = 0x80 | ((*stri >> 12) & 0x3F);
        *ustri++ = 0x80 | ((*stri >> 6) & 0x3F);
        *ustri++ = 0x80 | (*stri & 0x3F);
      } /* if */
    } /* for */
    return(ustri - out_stri);
  } /* stri_to_utf8 */



#ifdef ANSI_C

memsizetype utf8_to_stri (strelemtype *dest_stri, memsizetype *dest_len,
    ustritype ustri, SIZE_TYPE len)
#else

memsizetype utf8_to_stri (dest_stri, dest_len, ustri, len)
strelemtype *dest_stri;
memsizetype *dest_len;
ustritype ustri;
SIZE_TYPE len;
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
        *stri++ = (ustri[0] & 0x1F) << 6 |
                  (ustri[1] & 0x3F);
        ustri += 2;
        len--;
      } else if ((ustri[0] & 0xF0) == 0xE0 && len > 2 &&
                 (ustri[1] & 0xC0) == 0x80 &&
                 (ustri[2] & 0xC0) == 0x80) {
        *stri++ = (ustri[0] & 0x0F) << 12 |
                  (ustri[1] & 0x3F) <<  6 |
                  (ustri[2] & 0x3F);
        ustri += 3;
        len -= 2;
      } else if ((ustri[0] & 0xF8) == 0xF0 && len > 3 &&
                 (ustri[1] & 0xC0) == 0x80 &&
                 (ustri[2] & 0xC0) == 0x80 &&
                 (ustri[3] & 0xC0) == 0x80) {
        *stri++ = (ustri[0] & 0x07) << 18 |
                  (ustri[1] & 0x3F) << 12 |
                  (ustri[2] & 0x3F) <<  6 |
                  (ustri[3] & 0x3F);
        ustri += 4;
        len -= 3;
       } else if ((ustri[0] & 0xFC) == 0xF8 && len > 4 &&
                 (ustri[1] & 0xC0) == 0x80 &&
                 (ustri[2] & 0xC0) == 0x80 &&
                 (ustri[3] & 0xC0) == 0x80 &&
                 (ustri[4] & 0xC0) == 0x80) {
        *stri++ = (ustri[0] & 0x03) << 24 |
                  (ustri[1] & 0x3F) << 18 |
                  (ustri[2] & 0x3F) << 12 |
                  (ustri[3] & 0x3F) <<  6 |
                  (ustri[4] & 0x3F);
        ustri += 5;
        len -= 4;
      } else if ((ustri[0] & 0xFC) == 0xFC && len > 5 &&
                 (ustri[1] & 0xC0) == 0x80 &&
                 (ustri[2] & 0xC0) == 0x80 &&
                 (ustri[3] & 0xC0) == 0x80 &&
                 (ustri[4] & 0xC0) == 0x80 &&
                 (ustri[5] & 0xC0) == 0x80) {
        *stri++ = (ustri[0] & 0x03) << 30 |
                  (ustri[1] & 0x3F) << 24 |
                  (ustri[2] & 0x3F) << 18 |
                  (ustri[3] & 0x3F) << 12 |
                  (ustri[4] & 0x3F) <<  6 |
                  (ustri[5] & 0x3F);
        ustri += 6;
        len -= 5;
      } else {
        *dest_len = stri - dest_stri;
        return(len);
      } /* if */
    } /* while */
    *dest_len = stri - dest_stri;
    return(0);
  } /* utf8_to_stri */



#ifdef ANSI_C

memsizetype utf8_bytes_missing (ustritype ustri, SIZE_TYPE len)
#else

memsizetype utf8_bytes_missing (ustri, len)
ustritype ustri;
SIZE_TYPE len;
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
    return(result);
  } /* utf8_bytes_missing */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

void stri_expand (strelemtype *stri, ustritype ustri, SIZE_TYPE len)
#else

void stri_expand (stri, ustri, len)
strelemtype *stri;
ustritype ustri;
SIZE_TYPE len;
#endif

  { /* stri_expand */
    for (; len > 0; len--) {
      if (*ustri <= 0x7F) {
        *stri++ = (strelemtype) *ustri++;
      } else if ((ustri[0] & 0xE0) == 0xC0 && len > 1 &&
                 (ustri[1] & 0xC0) == 0x80) {
        *stri++ = (ustri[0] & 0x1F) << 6 |
                   ustri[1] & 0x3F;
        ustri += 2;
      } else if ((ustri[0] & 0xF0) == 0xE0 && len > 2 &&
                 (ustri[1] & 0xC0) == 0x80 &&
                 (ustri[2] & 0xC0) == 0x80) {
        *stri++ = (ustri[0] & 0x0F) << 12 |
                  (ustri[1] & 0x3F) <<  6 |
                   ustri[2] & 0x3F;
        ustri += 3;
      } else if ((ustri[0] & 0xF8) == 0xF0 && len > 3 &&
                 (ustri[1] & 0xC0) == 0x80 &&
                 (ustri[2] & 0xC0) == 0x80 &&
                 (ustri[3] & 0xC0) == 0x80) {
        *stri++ = (ustri[0] & 0x07) << 18 |
                  (ustri[1] & 0x3F) << 12 |
                  (ustri[2] & 0x3F) <<  6 |
                   ustri[3] & 0x3F;
        ustri += 4;
      } else if ((ustri[0] & 0xFC) == 0xF8 && len > 4 &&
                 (ustri[1] & 0xC0) == 0x80 &&
                 (ustri[2] & 0xC0) == 0x80 &&
                 (ustri[3] & 0xC0) == 0x80 &&
                 (ustri[4] & 0xC0) == 0x80) {
        *stri++ = (ustri[0] & 0x03) << 24 |
                  (ustri[1] & 0x3F) << 18 |
                  (ustri[2] & 0x3F) << 12 |
                  (ustri[3] & 0x3F) <<  6 |
                   ustri[4] & 0x3F;
        ustri += 5;
      } else if ((ustri[0] & 0xFC) == 0xFC && len > 5 &&
                 (ustri[1] & 0xC0) == 0x80 &&
                 (ustri[2] & 0xC0) == 0x80 &&
                 (ustri[3] & 0xC0) == 0x80 &&
                 (ustri[4] & 0xC0) == 0x80 &&
                 (ustri[5] & 0xC0) == 0x80) {
        *stri++ = (ustri[0] & 0x03) << 30 |
                  (ustri[1] & 0x3F) << 24 |
                  (ustri[2] & 0x3F) << 18 |
                  (ustri[3] & 0x3F) << 12 |
                  (ustri[4] & 0x3F) <<  6 |
                   ustri[5] & 0x3F;
        ustri += 6;
    } /* while */
  } /* stri_expand */
#endif



#ifdef ANSI_C

void stri_expand (strelemtype *stri, ustritype ustri, SIZE_TYPE len)
#else

void stri_expand (stri, ustri, len)
strelemtype *stri;
ustritype ustri;
SIZE_TYPE len;
#endif

  { /* stri_expand */
    for (; len > 0; stri++, ustri++, len--) {
      *stri = (strelemtype) *ustri;
    } /* while */
  } /* stri_expand */



#ifdef ANSI_C

void stri_compress (ustritype ustri, strelemtype *stri, SIZE_TYPE len)
#else

void stri_compress (ustri, stri, len)
ustritype ustri;
strelemtype *stri;
SIZE_TYPE len;
#endif

  { /* stri_compress */
    for (; len > 0; stri++, ustri++, len--) {
      *ustri = (uchartype) *stri;
    } /* for */
  } /* stri_compress */



#ifdef ANSI_C

void stri_export (ustritype out_stri, stritype in_stri)
#else

void stri_export (out_stri, in_stri)
ustritype out_stri;
stritype in_stri;
#endif

  {
    memsizetype len;

  /* stri_export */
    len = stri_to_utf8(out_stri, in_stri);
    out_stri[len] = '\0';
  } /* stri_export */
#endif



#ifdef ANSI_C

cstritype cp_to_cstri (stritype stri)
#else

cstritype cp_to_cstri (stri)
stritype stri;
#endif

  {
    cstritype cstri;

  /* cp_to_cstri */
    if (ALLOC_CSTRI(cstri, compr_size(stri))) {
      stri_export(cstri, stri);
    } /* if */
    return(cstri);
  } /* cp_to_cstri */



#ifdef ANSI_C

bstritype cp_to_bstri (stritype stri)
#else

bstritype cp_to_bstri (stri)
stritype stri;
#endif

  {
    bstritype bstri;

  /* cp_to_bstri */
    if (ALLOC_BSTRI(bstri, compr_size(stri))) {
      COUNT_BSTRI(compr_size(stri));
#ifdef WIDE_CHAR_STRINGS
      bstri->size = stri_to_utf8(bstri->mem, stri);
#else
      memcpy(bstri->mem, stri->mem, stri->size);
      bstri->size = stri->size;
#endif
      if (!RESIZE_BSTRI(bstri, compr_size(stri), bstri->size)) {
        bstri = NULL;
      } else {
        COUNT3_BSTRI(compr_size(stri), bstri->size);
      } /* if */
    } /* if */
    return(bstri);
  } /* cp_to_bstri */



#ifdef ANSI_C

stritype cp_to_stri (ustritype ustri)
#else

stritype cp_to_stri (ustri)
ustritype stri;
#endif

  {
    memsizetype length;
    stritype stri;

  /* cp_to_stri */
    length = strlen(ustri);
    if (ALLOC_STRI(stri, length)) {
      COUNT_STRI(length);
      stri->size = length;
      stri_expand(stri->mem, ustri, (SIZE_TYPE) length);
    } /* if */
    return(stri);
  } /* cp_to_stri */
