/********************************************************************/
/*                                                                  */
/*  drw_win.c     Graphic access using the windows capabilitys.     */
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
/*  File: seed7/src/drw_win.c                                       */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: Graphic access using the windows capabilitys.          */
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

chartype gkbGetc (void)
#else

chartype gkbGetc ()
#endif

  {
    chartype result;

  /* gkbGetc */
    result = ' ';
    return(result);
  } /* gkbGetc */



#ifdef ANSI_C

booltype gkbKeyPressed (void)
#else

booltype gkbKeyPressed ()
#endif

  {
    booltype result;

  /* gkbKeyPressed */
    result = TRUE;
    return(result);
  } /* gkbKeyPressed */



#ifdef ANSI_C

chartype gkbRawGetc (void)
#else

chartype gkbRawGetc ()
#endif

  { /* gkbRawGetc */
    return(gkbGetc());
  } /* gkbRawGetc */



#ifdef ANSI_C

inttype gkbXpos (void)
#else

inttype gkbXpos ()
#endif

  { /* gkbXpos */
    return(0);
  } /* gkbXpos */



#ifdef ANSI_C

inttype gkbYpos (void)
#else

inttype gkbYpos ()
#endif

  { /* gkbYpos */
    return(0);
  } /* gkbYpos */



#ifdef ANSI_C

void drwArc (wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)
#else

void drwArc (actual_window, x, y, radius, ang1, ang2)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
#endif

  { /* drwArc */
  } /* drwArc */



#ifdef ANSI_C

void drwFArcChord (wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)
#else

void drwFArcChord (actual_window, x, y, radius, ang1, ang2)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
#endif

  { /* drwFArcChord */
  } /* drwFArcChord */



#ifdef ANSI_C

void drwFArcPieSlice (wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)
#else

void drwFArcPieSlice (actual_window, x, y, radius, ang1, ang2)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
#endif

  { /* drwFArcPieSlice */
  } /* drwFArcPieSlice */



#ifdef ANSI_C

void drwArc2 (wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype radius)
#else

void drwArc2 (actual_window, x1, y1, x2, y2, radius)
wintype actual_window;
inttype x1, y1, x2, y2, radius;
#endif

  { /* drwArc2 */
  } /* drwArc2 */



#ifdef ANSI_C

void drwCircle (wintype actual_window,
    inttype x, inttype y, inttype radius)
#else

void drwCircle (actual_window, x, y, radius)
wintype actual_window;
inttype x, y, radius;
#endif

  { /* drwCircle */
  } /* drwCircle */



#ifdef ANSI_C

void drwClear (wintype actual_window)
#else

void drwClear (actual_window)
wintype actual_window;
#endif

  { /* drwClear */
  } /* drwClear */



#ifdef ANSI_C

void drwFCircle (wintype actual_window,
    inttype x, inttype y, inttype radius)
#else

void drwFCircle (actual_window, x, y, radius)
wintype actual_window;
inttype x, y, radius;
#endif

  { /* drwFCircle */
  } /* drwFCircle */



#ifdef ANSI_C

void drwFEllipse (wintype actual_window,
    inttype x, inttype y, inttype width, inttype height)
#else

void drwFEllipse (actual_window, x, y, width, height)
wintype actual_window;
inttype x, y, width, height;
#endif

  { /* drwFEllipse */
  } /* drwFEllipse */



#ifdef ANSI_C

void drwFlush (void)
#else

void drwFlush ()
#endif

  { /* drwFlush */
  } /* drwFlush */



#ifdef ANSI_C

void drwFree (wintype pixmap)
#else

void drwFree (pixmap)
wintype pixmap;
#endif

  { /* drwFree */
  } /* drwFree */



#ifdef ANSI_C

wintype drwGet (wintype actual_window, inttype left, inttype upper,
    inttype width, inttype height)
#else

wintype drwGet (actual_window, left, upper, width, height)
wintype actual_window;
inttype left;
inttype upper;
inttype width;
inttype height;
#endif

  { /* drwGet */
    return((wintype) NULL);
  } /* drwGet */



#ifdef ANSI_C

inttype drwHeight (wintype actual_window)
#else

inttype drwHeight (actual_window)
wintype actual_window;
#endif

  { /* drwHeight */
    return(0);
  } /* drwHeight */



#ifdef ANSI_C

wintype drwImage (wintype actual_window, inttype *image_data,
    inttype width, inttype height)
#else

wintype drwImage (actual_window, image_data, width, height)
wintype actual_window;
inttype *image_data;
inttype width;
inttype height;
#endif

  { /* drwImage */
    return((wintype) NULL);
  } /* drwImage */



#ifdef ANSI_C

void drwLine (wintype actual_window,
    inttype x1, inttype Y1, inttype x2, inttype Y2)
#else

void drwLine (actual_window, x1, Y1, x2, Y2)
wintype actual_window;
inttype x1, Y1, x2, Y2;
#endif

  { /* drwLine */
  } /* drwLine */



#ifdef ANSI_C

wintype drwNewPixmap (wintype actual_window, inttype width, inttype height)
#else

wintype drwNewPixmap (actual_window, width, height)
wintype actual_window;
inttype width;
inttype height;
#endif

  { /* drwNewPixmap */
    return(0);
  } /* drwNewPixmap */



#ifdef ANSI_C

wintype drwNewBitmap (wintype actual_window, inttype width, inttype height)
#else

wintype drwNewBitmap (actual_window, width, height)
wintype actual_window;
inttype width;
inttype height;
#endif

  { /* drwNewBitmap */
    return(0);
  } /* drwNewBitmap */



#ifdef ANSI_C

wintype drwOpen (inttype xPos, inttype yPos,
    inttype width, inttype height, stritype window_name)
#else

wintype drwOpen (xPos, yPos, width, height, window_name)
inttype xPos;
inttype yPos;
inttype width;
inttype height;
stritype window_name;
#endif

  { /* drwOpen */
    return(0);
  } /* drwOpen */



#ifdef ANSI_C

void drwPoint (wintype actual_window, inttype x, inttype y)
#else

void drwPoint (actual_window, x, y)
wintype actual_window;
inttype x, y;
#endif

  { /* drwPoint */
  } /* drwPoint */



#ifdef ANSI_C

void drwPPoint (wintype actual_window, inttype x, inttype y, inttype col)
#else

void drwPPoint (actual_window, x, y, col)
wintype actual_window;
inttype x, y;
inttype col;
#endif

  { /* drwPPoint */
  } /* drwPPoint */



#ifdef ANSI_C

void drwPRect (wintype actual_window,
    inttype x1, inttype y1, inttype length_x, inttype length_y, inttype col)
#else

void drwPRect (actual_window, x1, y1, length_x, length_y, col)
wintype actual_window;
inttype x1, y1, length_x, length_y;
inttype col;
#endif

  { /* drwPRect */
  } /* drwPRect */



#ifdef ANSI_C

void drwPut (wintype actual_window, wintype pixmap,
    inttype x1, inttype y1)
#else

void drwPut (actual_window, pixmap, x1, y1)
wintype actual_window;
wintype pixmap;
inttype x1;
inttype y1;
#endif

  { /* drwPut */
  } /* drwPut */



#ifdef ANSI_C

void drwRect (wintype actual_window,
    inttype x1, inttype Y1, inttype length_x, inttype length_y)
#else

void drwRect (actual_window, x1, Y1, length_x, length_y)
wintype actual_window;
inttype x1, Y1, length_x, length_y;
#endif

  { /* drwRect */
  } /* drwRect */



#ifdef ANSI_C

inttype drwRgbColor (inttype red_val, inttype green_val, inttype blue_val)
#else

inttype drwRgbColor (red_val, green_val, blue_val)
inttype red_val;
inttype green_val;
inttype blue_val;
#endif

  { /* drwRgbColor */
    return(0);
  } /* drwRgbColor */



#ifdef ANSI_C

void drwBackground (inttype col)
#else

void drwBackground (col)
inttype col;
#endif

  { /* drwBackground */
  } /* drwBackground */



#ifdef ANSI_C

void drwColor (inttype col)
#else

void drwColor (col)
inttype col;
#endif

  { /* drwColor */
  } /* drwColor */



#ifdef ANSI_C

void drwText (wintype actual_window, inttype x, inttype y, stritype stri)
#else

void drwText (actual_window, x, y, stri)
wintype actual_window;
inttype x, y;
stritype stri;
#endif

  { /* drwText */
  } /* drwText */



#ifdef ANSI_C

inttype drwWidth (wintype actual_window)
#else

inttype drwWidth (actual_window)
wintype actual_window;
#endif

  { /* drwWidth */
    return(0);
  } /* drwWidth */
