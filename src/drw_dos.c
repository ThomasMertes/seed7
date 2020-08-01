/********************************************************************/
/*                                                                  */
/*  drw_dos.c     Graphic access using the dos capabilitys.         */
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
/*  File: seed7/src/drw_dos.c                                       */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Graphic access using the dos capabilitys.              */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"

#include "version.h"
#include "common.h"

#undef EXTERN
#define EXTERN
#include "drw_drv.h"



#ifdef ANSI_C

void dra_arc (wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)
#else

void dra_arc (actual_window, x, y, radius, ang1, ang2)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
#endif

  { /* dra_arc */
  } /* dra_arc */



#ifdef ANSI_C

void dra_arcchord (wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)
#else

void dra_arcchord (actual_window, x, y, radius, ang1, ang2)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
#endif

  { /* dra_arcchord */
  } /* dra_arcchord */



#ifdef ANSI_C

void dra_arcpieslice (wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)
#else

void dra_arcpieslice (actual_window, x, y, radius, ang1, ang2)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
#endif

  { /* dra_arcpieslice */
  } /* dra_arcpieslice */



#ifdef ANSI_C

void dra_arc2 (wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype radius)
#else

void dra_arc2 (actual_window, x1, y1, x2, y2, radius)
wintype actual_window;
inttype x1, y1, x2, y2, radius;
#endif

  { /* dra_arc2 */
  } /* dra_arc2 */



#ifdef ANSI_C

void dra_circle (wintype actual_window,
    inttype x, inttype y, inttype radius)
#else

void dra_circle (actual_window, x, y, radius)
wintype actual_window;
inttype x, y, radius;
#endif

  { /* dra_circle */
  } /* dra_circle */



#ifdef ANSI_C

void dra_fcircle (wintype actual_window,
    inttype x, inttype y, inttype radius)
#else

void dra_fcircle (actual_window, x, y, radius)
wintype actual_window;
inttype x, y, radius;
#endif

  { /* dra_fcircle */
  } /* dra_fcircle */



#ifdef ANSI_C

void dra_fellipse (wintype actual_window,
    inttype x, inttype y, inttype width, inttype height)
#else

void dra_fellipse (actual_window, x, y, width, height)
wintype actual_window;
inttype x, y, width, height;
#endif

  { /* dra_fellipse */
  } /* dra_fellipse */



#ifdef ANSI_C

int dra_height (wintype actual_window)
#else

int dra_height (actual_window)
wintype actual_window;
#endif

  { /* dra_height */
    return(0);
  } /* dra_height */



#ifdef ANSI_C

void dra_line (wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2)
#else

void dra_line (actual_window, x1, y1, x2, y2)
wintype actual_window;
inttype x1, y1, x2, y2;
#endif

  { /* dra_line */
  } /* dra_line */



#ifdef ANSI_C

wintype dra_open (inttype xPos, inttype yPos,
    inttype width, inttype height, char *window_name)
#else

wintype dra_open (xPos, yPos, width, height, window_name)
inttype xPos;
inttype yPos;
inttype width;
inttype height;
char *window_name;
#endif

  {
  /* dra_open */
    return(0);
  } /* dra_open */



#ifdef ANSI_C

void dra_rect (wintype actual_window,
    inttype x1, inttype y1, inttype length_x, inttype length_y)
#else

void dra_rect (actual_window, x1, y1, length_x, length_y)
wintype actual_window;
inttype x1, y1, length_x, length_y;
#endif

  { /* dra_rect */
  } /* dra_rect */



#ifdef ANSI_C

inttype dra_rgb_color (inttype red_val, inttype green_val, inttype blue_val)
#else

inttype dra_rgb_color (red_val, green_val, blue_val)
inttype red_val;
inttype green_val;
inttype blue_val;
#endif

  { /* dra_rgb_color */
    return(0);
  } /* dra_rgb_color */



#ifdef ANSI_C

void dra_background (inttype col)
#else

void dra_background (col)
inttype col;
#endif

  { /* dra_background */
  } /* dra_background */



#ifdef ANSI_C

void dra_color (inttype col)
#else

void dra_color (col)
inttype col;
#endif

  { /* dra_color */
  } /* dra_color */



#ifdef ANSI_C

void dra_text (wintype actual_window, stritype stri, inttype x1, inttype y1)
#else

void dra_text (actual_window, stri, x1, y1)
wintype actual_window;
stritype stri;
inttype x1, y1;
#endif

  { /* dra_text */
  } /* dra_text */



#ifdef ANSI_C

int dra_width (wintype actual_window)
#else

int dra_width (actual_window)
wintype actual_window;
#endif

  { /* dra_width */
    return(0);
  } /* dra_width */
