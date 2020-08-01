/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/scrlib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do text screen output.        */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "runerr.h"
#include "memory.h"
#include "scr_drv.h"

#undef EXTERN
#define EXTERN
#include "scrlib.h"


static inttype current_line = 1;
static inttype current_column = 1;



#ifdef ANSI_C

objecttype scr_clear (listtype arguments)
#else

objecttype scr_clear (arguments)
listtype arguments;
#endif

  { /* scr_clear */
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    scrClear(take_int(arg_2(arguments)),
        take_int(arg_3(arguments)),
        take_int(arg_4(arguments)),
        take_int(arg_5(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* scr_clear */



#ifdef ANSI_C

objecttype scr_cursor (listtype arguments)
#else

objecttype scr_cursor (arguments)
listtype arguments;
#endif

  { /* scr_cursor */
    isit_bool(arg_2(arguments));
    if (take_bool(arg_2(arguments)) == SYS_TRUE_OBJECT) {
      scrCursor(TRUE);
    } else if (take_bool(arg_2(arguments)) == SYS_FALSE_OBJECT) {
      scrCursor(FALSE);
    } else {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* scr_cursor */



#ifdef ANSI_C

objecttype scr_flush (listtype arguments)
#else

objecttype scr_flush (arguments)
listtype arguments;
#endif

  { /* scr_flush */
/*  scrSetCursor(current_line, current_column); */
    scrFlush();
    return(SYS_EMPTY_OBJECT);
  } /* scr_flush */



#ifdef ANSI_C

objecttype scr_height (listtype arguments)
#else

objecttype scr_height (arguments)
listtype arguments;
#endif

  { /* scr_height */
    return(bld_int_temp(scrHeight()));
  } /* scr_height */



#ifdef ANSI_C

objecttype scr_h_scl (listtype arguments)
#else

objecttype scr_h_scl (arguments)
listtype arguments;
#endif

  {
    inttype scroll_amount;

  /* scr_h_scl */
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    scroll_amount = take_int(arg_6(arguments));
    if (scroll_amount >= 0) {
      scrLeftScroll(take_int(arg_2(arguments)),
          take_int(arg_3(arguments)),
          take_int(arg_4(arguments)),
          take_int(arg_5(arguments)),
          scroll_amount);
    } else {
      scrRightScroll(take_int(arg_2(arguments)),
          take_int(arg_3(arguments)),
          take_int(arg_4(arguments)),
          take_int(arg_5(arguments)),
          -scroll_amount);
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* scr_h_scl */



#ifdef ANSI_C

objecttype scr_open (listtype arguments)
#else

objecttype scr_open (arguments)
listtype arguments;
#endif

  { /* scr_open */
    scrOpen();
    return(SYS_EMPTY_OBJECT);
  } /* scr_open */



#ifdef ANSI_C

objecttype scr_setpos (listtype arguments)
#else

objecttype scr_setpos (arguments)
listtype arguments;
#endif

  { /* scr_setpos */
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    current_line = take_int(arg_2(arguments));
    current_column = take_int(arg_3(arguments));
    scrSetCursor(current_line, current_column);
    return(SYS_EMPTY_OBJECT);
  } /* scr_setpos */



#ifdef ANSI_C

objecttype scr_v_scl (listtype arguments)
#else

objecttype scr_v_scl (arguments)
listtype arguments;
#endif

  {
    inttype scroll_amount;

  /* scr_v_scl */
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    scroll_amount = take_int(arg_6(arguments));
    if (scroll_amount >= 0) {
      scrUpScroll(take_int(arg_2(arguments)),
          take_int(arg_3(arguments)),
          take_int(arg_4(arguments)),
          take_int(arg_5(arguments)),
          scroll_amount);
    } else {
      scrDownScroll(take_int(arg_2(arguments)),
          take_int(arg_3(arguments)),
          take_int(arg_4(arguments)),
          take_int(arg_5(arguments)),
          -scroll_amount);
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* scr_v_scl */



#ifdef ANSI_C

objecttype scr_width (listtype arguments)
#else

objecttype scr_width (arguments)
listtype arguments;
#endif

  { /* scr_width */
    return(bld_int_temp(scrWidth()));
  } /* scr_width */



#ifdef ANSI_C

objecttype scr_write (listtype arguments)
#else

objecttype scr_write (arguments)
listtype arguments;
#endif

  {
    stritype stri;

  /* scr_write */
    isit_stri(arg_2(arguments));
    stri = take_stri(arg_2(arguments));
#ifdef UTF32_STRINGS
    if (stri->size <= 256) {
      memsizetype size;
      uchartype stri_buffer[6 * 256];

#ifdef SCREEN_UTF8
      size = stri_to_utf8(stri_buffer, stri);
#else
      stri_compress(stri_buffer, stri->mem, stri->size);
      size = stri->size;
#endif
      scrText(current_line, current_column, stri_buffer, size);
    } else {
      bstritype bstri;

#ifdef SCREEN_UTF8
      bstri = stri_to_bstri8(stri);
#else
      bstri = stri_to_bstri(stri);
#endif
      if (bstri != NULL) {
        scrText(current_line, current_column, bstri->mem, bstri->size);
        FREE_BSTRI(bstri, bstri->size);
      } /* if */
    } /* if */
#else
    scrText(current_line, current_column,
        stri->mem, stri->size);
#endif
    current_column += stri->size;
    scrSetCursor(current_line, current_column);
    return(SYS_EMPTY_OBJECT);
  } /* scr_write */
