/********************************************************************/
/*                                                                  */
/*  con_rtl.c     Primitive actions for console/terminal output.    */
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
/*  File: seed7/src/con_rtl.c                                       */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: Primitive actions for console/terminal output.         */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "con_drv.h"

#undef EXTERN
#define EXTERN
#include "con_rtl.h"


#define WRITE_STRI_BLOCK_SIZE    256


#ifdef CONSOLE_USES_CON_TEXT
static intType cursor_line = 1;
static intType cursor_column = 1;
#endif



#ifdef CONSOLE_USES_CON_TEXT
intType conColumn (void)

  { /* conColumn */
    return cursor_column;
  } /* conColumn */
#endif



void conHScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numCols)

  { /* conHScroll */
    if (numCols > 0) {
      conLeftScroll(startlin, startcol, stoplin, stopcol, numCols);
    } else if (numCols < 0) {
      conRightScroll(startlin, startcol, stoplin, stopcol, -numCols);
    } /* if */
  } /* conHScroll */



#ifdef CONSOLE_USES_CON_TEXT
intType conLine (void)

  { /* conLine */
    return cursor_line;
  } /* conLine */
#endif



void conSetpos (intType lin, intType col)

  { /* conSetpos */
#ifdef CONSOLE_USES_CON_TEXT
    cursor_line = lin;
    cursor_column = col;
#endif
    conSetCursor(lin, col);
  } /* conSetpos */



void conVScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numLines)

  { /* conVScroll */
    if (numLines > 0) {
      conUpScroll(startlin, startcol, stoplin, stopcol, numLines);
    } else if (numLines < 0) {
      conDownScroll(startlin, startcol, stoplin, stopcol, -numLines);
    } /* if */
  } /* conVScroll */



#ifdef CONSOLE_USES_CON_TEXT
/**
 *  This function writes the string stri to the console at the
 *  current position. The current position must be a legal
 *  position of the console. The string stri is not allowed to go
 *  beyond the right border of the console.
 */
void conWrite (const const_striType stri)

  {
    memSizeType size;
    errInfoType err_info = OKAY_NO_ERROR;

  /* conWrite */
    if (stri->size <= WRITE_STRI_BLOCK_SIZE) {
#ifdef CONSOLE_UTF8
      ucharType stri_buffer[max_utf8_size(WRITE_STRI_BLOCK_SIZE)];

      size = stri_to_utf8(stri_buffer, stri->mem, stri->size);
#elif defined CONSOLE_WCHAR
      wchar_t stri_buffer[WRITE_STRI_BLOCK_SIZE * SURROGATE_PAIR_FACTOR];

      size = stri_to_utf16(stri_buffer, stri->mem, stri->size, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
        return;
      } /* if */
#else
      ucharType stri_buffer[WRITE_STRI_BLOCK_SIZE + NULL_TERMINATION_LEN];

      if (unlikely(conv_to_cstri(stri_buffer, stri) == NULL)) {
        raise_error(RANGE_ERROR);
        return;
      } else {
        size = stri->size;
      } /* if */
#endif
      conText(cursor_line, cursor_column, stri_buffer, size);
    } else {
      bstriType bstri;

#ifdef CONSOLE_UTF8
      bstri = stri_to_bstri8(stri);
#elif defined CONSOLE_WCHAR
      bstri = stri_to_bstriw(stri, &err_info);
#else
      bstri = stri_to_bstri(stri, &err_info);
#endif
      if (unlikely(bstri == NULL)) {
#ifdef CONSOLE_UTF8
        raise_error(MEMORY_ERROR);
#else
        raise_error(err_info);
#endif
      } else {
#if defined CONSOLE_WCHAR
        size = bstri->size / sizeof(os_charType);
#else
        size = bstri->size;
#endif
        conText(cursor_line, cursor_column, (console_striType) bstri->mem, size);
        FREE_BSTRI(bstri, bstri->size);
      } /* if */
    } /* if */
    cursor_column = cursor_column + stri->size;
    conSetCursor(cursor_line, cursor_column);
  } /* conWrite */
#endif
