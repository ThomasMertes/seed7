/********************************************************************/
/*                                                                  */
/*  drw_dos.c     Graphic access using the dos capabilities.        */
/*  Copyright (C) 1989 - 2006  Thomas Mertes                        */
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
/*  File: seed7/src/drw_dos.c                                       */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Graphic access using the dos capabilities.             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "drw_drv.h"



#ifdef ANSI_C

chartype gkbGetc (void)
#else

chartype gkbGetc ()
#endif

  { /* gkbGetc */
    return (chartype) EOF;
  } /* gkbGetc */



#ifdef ANSI_C

booltype gkbKeyPressed (void)
#else

booltype gkbKeyPressed ()
#endif

  { /* gkbKeyPressed */
    return FALSE;
  } /* gkbKeyPressed */



#ifdef ANSI_C

booltype gkbButtonPressed (chartype button)
#else

booltype gkbButtonPressed (button)
chartype button;
#endif

  { /* gkbButtonPressed */
    return FALSE;
  } /* gkbButtonPressed */



#ifdef ANSI_C

chartype gkbRawGetc (void)
#else

chartype gkbRawGetc ()
#endif

  { /* gkbRawGetc */
    return gkbGetc();
  } /* gkbRawGetc */



#ifdef ANSI_C

wintype gkbWindow (void)
#else

wintype gkbWindow ()
#endif

  { /* gkbWindow */
    return NULL;
  } /* gkbWindow */



#ifdef ANSI_C

inttype gkbButtonXpos (void)
#else

inttype gkbButtonXpos ()
#endif

  { /* gkbButtonXpos */
    return 0;
  } /* gkbButtonXpos */



#ifdef ANSI_C

inttype gkbButtonYpos (void)
#else

inttype gkbButtonYpos ()
#endif

  { /* gkbButtonYpos */
    return 0;
  } /* gkbButtonYpos */



#ifdef ANSI_C

inttype drwPointerXpos (const_wintype actual_window)
#else

inttype drwPointerXpos (actual_window)
wintype actual_window;
#endif

  { /* drwPointerXpos */
    return 0;
  } /* drwPointerXpos */



#ifdef ANSI_C

inttype drwPointerYpos (const_wintype actual_window)
#else

inttype drwPointerYpos (actual_window)
wintype actual_window;
#endif

  { /* drwPointerYpos */
    return 0;
  } /* drwPointerYpos */



#ifdef ANSI_C

void drwArc (const_wintype actual_window, inttype x, inttype y,
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

void drwPArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2, inttype col)
#else

void drwPArc (actual_window, x, y, radius, ang1, ang2, col)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
inttype col;
#endif

  { /* drwPArc */
  } /* drwPArc */



#ifdef ANSI_C

void drwFArcChord (const_wintype actual_window, inttype x, inttype y,
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

void drwPFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2, inttype col)
#else

void drwPFArcChord (actual_window, x, y, radius, ang1, ang2, col)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
inttype col;
#endif

  { /* drwPFArcChord */
  } /* drwPFArcChord */



#ifdef ANSI_C

void drwFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
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

void drwPFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2, inttype col)
#else

void drwPFArcPieSlice (actual_window, x, y, radius, ang1, ang2, col)
wintype actual_window;
inttype x, y, radius;
floattype ang1, ang2;
inttype col;
#endif

  { /* drwPFArcPieSlice */
  } /* drwPFArcPieSlice */



#ifdef ANSI_C

void drwArc2 (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype radius)
#else

void drwArc2 (actual_window, x1, y1, x2, y2, radius)
wintype actual_window;
inttype x1, y1, x2, y2, radius;
#endif

  { /* drwArc2 */
  } /* drwArc2 */



#ifdef ANSI_C

void drwCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius)
#else

void drwCircle (actual_window, x, y, radius)
wintype actual_window;
inttype x, y, radius;
#endif

  { /* drwCircle */
  } /* drwCircle */



#ifdef ANSI_C

void drwPCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)
#else

void drwPCircle (actual_window, x, y, col)
wintype actual_window;
inttype x, y, radius;
inttype col;
#endif

  { /* drwPCircle */
  } /* drwPCircle */



#ifdef ANSI_C

void drwClear (wintype actual_window, inttype col)
#else

void drwClear (actual_window, col)
wintype actual_window;
inttype col;
#endif

  { /* drwClear */
  } /* drwClear */



#ifdef ANSI_C

void drwCopyArea (const_wintype src_window, const_wintype dest_window,
    inttype src_x, inttype src_y, inttype width, inttype height,
    inttype dest_x, inttype dest_y)
#else

void drwCopyArea (src_window, dest_window, src_x, src_y, width, height,
    dest_x, dest_y)
wintype src_window;
wintype dest_window;
inttype src_x;
inttype src_y;
inttype width;
inttype height;
inttype dest_x;
inttype dest_y;
#endif

  { /* drwCopyArea */
  } /* drwCopyArea */



#ifdef ANSI_C

void drwFCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius)
#else

void drwFCircle (actual_window, x, y, radius)
wintype actual_window;
inttype x, y, radius;
#endif

  { /* drwFCircle */
  } /* drwFCircle */



#ifdef ANSI_C

void drwPFCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)
#else

void drwPFCircle (actual_window, x, y, radius, col)
wintype actual_window;
inttype x, y, radius;
inttype col;
#endif

  { /* drwPFCircle */
  } /* drwPFCircle */



#ifdef ANSI_C

void drwFEllipse (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height)
#else

void drwFEllipse (actual_window, x, y, width, height)
wintype actual_window;
inttype x, y, width, height;
#endif

  { /* drwFEllipse */
  } /* drwFEllipse */



#ifdef ANSI_C

void drwPFEllipse (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height, inttype col)
#else

void drwPFEllipse (actual_window, x, y, width, height, col)
wintype actual_window;
inttype x, y, width, height;
inttype col;
#endif

  { /* drwPFEllipse */
  } /* drwPFEllipse */



#ifdef ANSI_C

void drwFlush (void)
#else

void drwFlush ()
#endif

  { /* drwFlush */
  } /* drwFlush */



#ifdef ANSI_C

void drwFree (wintype old_window)
#else

void drwFree (old_window)
wintype old_window;
#endif

  { /* drwFree */
  } /* drwFree */



#ifdef ANSI_C

wintype drwGet (const_wintype actual_window, inttype left, inttype upper,
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
    return NULL;
  } /* drwGet */



#ifdef ANSI_C

inttype drwGetPixel (const_wintype actual_window, inttype x, inttype y)
#else

inttype drwGetPixel (actual_window, x, y)
wintype actual_window;
inttype x;
inttype y;
#endif

  { /* drwGetPixel */
  } /* drwGetPixel */



#ifdef ANSI_C

inttype drwHeight (const_wintype actual_window)
#else

inttype drwHeight (actual_window)
wintype actual_window;
#endif

  { /* drwHeight */
    return 0;
  } /* drwHeight */



#ifdef ANSI_C

wintype drwImage (const_wintype actual_window, inttype *image_data,
    inttype width, inttype height)
#else

wintype drwImage (actual_window, image_data, width, height)
wintype actual_window;
inttype *image_data;
inttype width;
inttype height;
#endif

  { /* drwImage */
    return NULL;
  } /* drwImage */



#ifdef ANSI_C

void drwLine (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2)
#else

void drwLine (actual_window, x1, y1, x2, y2)
wintype actual_window;
inttype x1, y1, x2, y2;
#endif

  { /* drwLine */
  } /* drwLine */



#ifdef ANSI_C

void drwPLine (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype col)
#else

void drwPLine (actual_window, x1, y1, x2, y2, col)
wintype actual_window;
inttype x1, y1, x2, y2;
inttype col;
#endif

  { /* drwPLine */
  } /* drwPLine */



#ifdef ANSI_C

wintype drwNewPixmap (inttype width, inttype height)
#else

wintype drwNewPixmap (width, height)
inttype width;
inttype height;
#endif

  { /* drwNewPixmap */
    return NULL;
  } /* drwNewPixmap */



#ifdef ANSI_C

wintype drwOpen (inttype xPos, inttype yPos,
    inttype width, inttype height, const const_stritype window_name)
#else

wintype drwOpen (xPos, yPos, width, height, window_name)
inttype xPos;
inttype yPos;
inttype width;
inttype height;
stritype window_name;
#endif

  { /* drwOpen */
    raise_error(FILE_ERROR);
    return NULL;
  } /* drwOpen */



#ifdef ANSI_C

wintype drwOpenSubWindow (const_wintype parent_window, inttype xPos, inttype yPos,
    inttype width, inttype height)
#else

  wintype drwOpenSubWindow (parent_window, xPos, yPos, width, height)
wintype parent_window;
inttype xPos;
inttype yPos;
inttype width;
inttype height;
#endif

  { /* drwOpenSubWindow */
    raise_error(FILE_ERROR);
    return NULL;
  } /* drwOpenSubWindow */



#ifdef ANSI_C

void drwPoint (const_wintype actual_window, inttype x, inttype y)
#else

void drwPoint (actual_window, x, y)
wintype actual_window;
inttype x, y;
#endif

  { /* drwPoint */
  } /* drwPoint */



#ifdef ANSI_C

void drwPPoint (const_wintype actual_window, inttype x, inttype y, inttype col)
#else

void drwPPoint (actual_window, x, y, col)
wintype actual_window;
inttype x, y;
inttype col;
#endif

  { /* drwPPoint */
  } /* drwPPoint */



#ifdef ANSI_C

bstritype drwGenPointList (const const_rtlArraytype xyArray)
#else

bstritype drwGenPointList (xyArray)
rtlArraytype xyArray;
#endif

  { /* drwGenPointList */
    return NULL;
  } /* drwGenPointList */



#ifdef ANSI_C

void drwPolyLine (const_wintype actual_window,
    inttype x1, inttype y1, bstritype point_list, inttype col)
#else

void drwPolyLine (actual_window, x1, y1, point_list, col)
wintype actual_window;
inttype x1, y1;
const_bstritype point_list;
inttype col;
#endif

  { /* drwPolyLine */
  } /* drwPolyLine */



#ifdef ANSI_C

void drwFPolyLine (const_wintype actual_window,
    inttype x1, inttype y1, bstritype point_list, inttype col)
#else

void drwFPolyLine (actual_window, x1, y1, point_list, col)
wintype actual_window;
inttype x1, y1;
const_bstritype point_list;
inttype col;
#endif

  { /* drwFPolyLine */
  } /* drwFPolyLine */



#ifdef ANSI_C

void drwPut (const_wintype actual_window, const_wintype pixmap,
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

void drwRect (const_wintype actual_window,
    inttype x1, inttype y1, inttype width, inttype height)
#else

void drwRect (actual_window, x1, y1, width, height)
wintype actual_window;
inttype x1, y1, width, height;
#endif

  { /* drwRect */
  } /* drwRect */



#ifdef ANSI_C

void drwPRect (const_wintype actual_window,
    inttype x1, inttype y1, inttype width, inttype height, inttype col)
#else

void drwPRect (actual_window, x1, y1, width, height, col)
wintype actual_window;
inttype x1, y1, width, height;
inttype col;
#endif

  { /* drwPRect */
  } /* drwPRect */



#ifdef ANSI_C

inttype drwRgbColor (inttype red_val, inttype green_val, inttype blue_val)
#else

inttype drwRgbColor (red_val, green_val, blue_val)
inttype red_val;
inttype green_val;
inttype blue_val;
#endif

  { /* drwRgbColor */
    return 0;
  } /* drwRgbColor */



#ifdef ANSI_C

void drwPixelToRgb (inttype col, inttype *red_val, inttype *green_val, inttype *blue_val)
#else

void drwPixelToRgb (col, red_val, green_val, blue_val)
inttype col;
inttype *red_val;
inttype *green_val;
inttype *blue_val;
#endif

  { /* drwPixelToRgb */
    *red_val   = 0;
    *green_val = 0;
    *blue_val  = 0;
  } /* drwPixelToRgb */



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

void drwSetContent (const_wintype actual_window, const_wintype pixmap)
#else

void drwSetContent (actual_window, pixmap)
wintype actual_window;
wintype pixmap;
#endif

  { /* drwSetContent */
  } /* drwSetContent */



#ifdef ANSI_C

void drwSetPos (const_wintype actual_window, inttype xPos, inttype yPos)
#else

void drwSetPos (actual_window, xPos, yPos)
wintype actual_window;
inttype xPos, yPos;
#endif

  { /* drwSetPos */
  } /* drwSetPos */



#ifdef ANSI_C

void drwSetTransparentColor (wintype pixmap, inttype col)
#else

void drwSetTransparentColor (pixmap, col)
wintype pixmap;
inttype col;
#endif

  { /* drwSetTransparentColor */
  } /* drwSetTransparentColor */



#ifdef ANSI_C

void drwText (const_wintype actual_window, inttype x, inttype y,
    const const_stritype stri, inttype col, inttype bkcol)
#else

void drwText (actual_window, x, y, stri, col, bkcol)
wintype actual_window;
inttype x, y;
stritype stri;
inttype col;
inttype bkcol;
#endif

  { /* drwText */
  } /* drwText */



#ifdef ANSI_C

void drwToBottom (const_wintype actual_window)
#else

void drwToBottom (actual_window)
wintype actual_window;
#endif

  { /* drwToBottom */
  } /* drwToBottom */



#ifdef ANSI_C

void drwToTop (const_wintype actual_window)
#else

void drwToTop (actual_window)
wintype actual_window;
#endif

  { /* drwToTop */
  } /* drwToTop */



#ifdef ANSI_C

inttype drwWidth (const_wintype actual_window)
#else

inttype drwWidth (actual_window)
wintype actual_window;
#endif

  { /* drwWidth */
    return 0;
  } /* drwWidth */



#ifdef ANSI_C

inttype drwXPos (const_wintype actual_window)
#else

inttype drwXPos (actual_window)
wintype actual_window;
#endif

  { /* drwXPos */
    return 0;
  } /* drwXPos */



#ifdef ANSI_C

inttype drwYPos (const_wintype actual_window)
#else

inttype drwYPos (actual_window)
wintype actual_window;
#endif

  { /* drwYPos */
    return 0;
  } /* drwYPos */
