/********************************************************************/
/*                                                                  */
/*  scr_rtl.c     Primitive actions for screen output.              */
/*  Copyright (C) 1989 - 2007  Thomas Mertes                        */
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
/*  File: seed7/src/scr_rtl.c                                       */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: Primitive actions for screen output.                   */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "striutl.h"
#include "heaputl.h"
#include "scr_drv.h"

#undef EXTERN
#define EXTERN
#include "scr_rtl.h"


static inttype cursor_line = 1;
static inttype cursor_column = 1;



#ifdef ANSI_C

void scrHScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void scrHScroll (startlin, startcol, stoplin, stopcol, count)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
inttype count;
#endif

  { /* scrHScroll */
    if (count >= 0) {
      scrLeftScroll(startlin, startcol, stoplin, stopcol, count);
    } else {
      scrRightScroll(startlin, startcol, stoplin, stopcol, -count);
    } /* if */
  } /* scrHScroll */



#ifdef ANSI_C

void scrSetpos (inttype lin, inttype col)
#else

void scrSetpos (lin, col)
inttype lin;
inttype col;
#endif

  { /* scrSetpos */
    cursor_line = lin;
    cursor_column = col;
    scrSetCursor(lin, col);
  } /* scrSetpos */



#ifdef ANSI_C

void scrVScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void scrVScroll (startlin, startcol, stoplin, stopcol, count)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
inttype count;
#endif

  { /* scrVScroll */
    if (count >= 0) {
      scrUpScroll(startlin, startcol, stoplin, stopcol, count);
    } else {
      scrDownScroll(startlin, startcol, stoplin, stopcol, -count);
    } /* if */
  } /* scrVScroll */



#ifdef ANSI_C

void scrWrite (const_stritype stri)
#else

void scrWrite (stri)
stritype stri;
#endif

  /* This function writes the string stri to the screen at the      */
  /* current position. The current position must be a legal         */
  /* position of the screen. The string stri is not allowed to go   */
  /* beyond the right border of the screen. All screen output       */
  /* must be done with this function.                               */

  { /* scrWrite */
#ifdef UTF32_STRINGS
    if (stri->size <= 256) {
      memsizetype size;
      uchartype stri_buffer[MAX_UTF8_EXPANSION_FACTOR * 256];

#ifdef SCREEN_UTF8
      size = stri_to_utf8(stri_buffer, stri->mem, stri->size);
#else
      stri_compress(stri_buffer, stri->mem, stri->size);
      size = stri->size;
#endif
      scrText(cursor_line, cursor_column, stri_buffer, size);
    } else {
      bstritype bstri;

#ifdef SCREEN_UTF8
      bstri = stri_to_bstri8(stri);
#else
      bstri = stri_to_bstri(stri);
#endif
      if (bstri != NULL) {
        scrText(cursor_line, cursor_column, bstri->mem, bstri->size);
        FREE_BSTRI(bstri, bstri->size);
      } /* if */
    } /* if */
#else
    scrText(cursor_line, cursor_column,
        stri->mem, stri->size);
#endif
    cursor_column = cursor_column + stri->size;
    scrSetCursor(cursor_line, cursor_column);
  } /* scrWrite */
