/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2011  Thomas Mertes                        */
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
/*  File: seed7/src/conlib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do text console output.       */
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
#include "objutl.h"
#include "runerr.h"
#include "con_rtl.h"
#include "con_drv.h"

#undef EXTERN
#define EXTERN
#include "conlib.h"



objectType con_clear (listType arguments)

  { /* con_clear */
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    conClear(take_int(arg_2(arguments)),
        take_int(arg_3(arguments)),
        take_int(arg_4(arguments)),
        take_int(arg_5(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* con_clear */



objectType con_cursor (listType arguments)

  { /* con_cursor */
    isit_bool(arg_2(arguments));
    if (take_bool(arg_2(arguments)) == SYS_TRUE_OBJECT) {
      conCursor(TRUE);
    } else if (take_bool(arg_2(arguments)) == SYS_FALSE_OBJECT) {
      conCursor(FALSE);
    } else {
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* con_cursor */



objectType con_flush (listType arguments)

  { /* con_flush */
    conFlush();
    return SYS_EMPTY_OBJECT;
  } /* con_flush */



objectType con_height (listType arguments)

  { /* con_height */
    return bld_int_temp(conHeight());
  } /* con_height */



objectType con_h_scl (listType arguments)

  {
    intType scroll_amount;

  /* con_h_scl */
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    scroll_amount = take_int(arg_6(arguments));
    if (scroll_amount >= 0) {
      conLeftScroll(take_int(arg_2(arguments)),
          take_int(arg_3(arguments)),
          take_int(arg_4(arguments)),
          take_int(arg_5(arguments)),
          scroll_amount);
    } else {
      conRightScroll(take_int(arg_2(arguments)),
          take_int(arg_3(arguments)),
          take_int(arg_4(arguments)),
          take_int(arg_5(arguments)),
          -scroll_amount);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* con_h_scl */



objectType con_open (listType arguments)

  { /* con_open */
    conOpen();
    return SYS_EMPTY_OBJECT;
  } /* con_open */



objectType con_setpos (listType arguments)

  { /* con_setpos */
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    conSetpos(take_int(arg_2(arguments)), take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* con_setpos */



objectType con_v_scl (listType arguments)

  {
    intType scroll_amount;

  /* con_v_scl */
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    scroll_amount = take_int(arg_6(arguments));
    if (scroll_amount >= 0) {
      conUpScroll(take_int(arg_2(arguments)),
          take_int(arg_3(arguments)),
          take_int(arg_4(arguments)),
          take_int(arg_5(arguments)),
          scroll_amount);
    } else {
      conDownScroll(take_int(arg_2(arguments)),
          take_int(arg_3(arguments)),
          take_int(arg_4(arguments)),
          take_int(arg_5(arguments)),
          -scroll_amount);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* con_v_scl */



objectType con_width (listType arguments)

  { /* con_width */
    return bld_int_temp(conWidth());
  } /* con_width */



objectType con_write (listType arguments)

  { /* con_write */
    isit_stri(arg_2(arguments));
    conWrite(take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* con_write */
