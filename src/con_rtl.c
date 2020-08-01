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


static inttype cursor_line = 1;
static inttype cursor_column = 1;



void conHScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)

  { /* conHScroll */
    if (count > 0) {
      conLeftScroll(startlin, startcol, stoplin, stopcol, count);
    } else if (count < 0) {
      conRightScroll(startlin, startcol, stoplin, stopcol, -count);
    } /* if */
  } /* conHScroll */



void conSetpos (inttype lin, inttype col)

  { /* conSetpos */
    cursor_line = lin;
    cursor_column = col;
    conSetCursor(lin, col);
  } /* conSetpos */



void conVScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)

  { /* conVScroll */
    if (count > 0) {
      conUpScroll(startlin, startcol, stoplin, stopcol, count);
    } else if (count < 0) {
      conDownScroll(startlin, startcol, stoplin, stopcol, -count);
    } /* if */
  } /* conVScroll */



#ifdef CONSOLE_USES_CON_TEXT
void conWrite (const_stritype stri)

  /* This function writes the string stri to the console at the     */
  /* current position. The current position must be a legal         */
  /* position of the console. The string stri is not allowed to go  */
  /* beyond the right border of the console. All console output     */
  /* must be done with this function.                               */

  {
    memsizetype size;
    errinfotype err_info = OKAY_NO_ERROR;

  /* conWrite */
    if (stri->size <= WRITE_STRI_BLOCK_SIZE) {
#ifdef CONSOLE_UTF8
      uchartype stri_buffer[max_utf8_size(WRITE_STRI_BLOCK_SIZE)];

      size = stri_to_utf8(stri_buffer, stri->mem, stri->size);
#elif defined CONSOLE_WCHAR
      wchar_t stri_buffer[2 * WRITE_STRI_BLOCK_SIZE];

      size = stri_to_wstri(stri_buffer, stri->mem, stri->size, &err_info);
#else
      uchartype stri_buffer[WRITE_STRI_BLOCK_SIZE + 1];

      conv_to_cstri(stri_buffer, stri->mem, stri->size, &err_info);
      size = stri->size;
#endif
      conText(cursor_line, cursor_column, stri_buffer, size);
    } else {
      bstritype bstri;

#ifdef CONSOLE_UTF8
      bstri = stri_to_bstri8(stri);
#elif defined CONSOLE_WCHAR
      bstri = stri_to_bstriw(stri);
#else
      bstri = stri_to_bstri(stri, &err_info);
#endif
      if (bstri != NULL) {
#if defined CONSOLE_WCHAR
        size = bstri->size / sizeof(os_chartype);
#else
        size = bstri->size;
#endif
        conText(cursor_line, cursor_column, (console_stritype) bstri->mem, size);
        FREE_BSTRI(bstri, bstri->size);
      } /* if */
    } /* if */
    cursor_column = cursor_column + stri->size;
    conSetCursor(cursor_line, cursor_column);
  } /* conWrite */
#endif
