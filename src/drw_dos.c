/********************************************************************/
/*                                                                  */
/*  drw_dos.c     Graphic access using the dos capabilities.        */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
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
/*  Changes: 1994, 2013  Thomas Mertes                              */
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


#ifdef DO_HEAP_STATISTIC
size_t sizeof_winrecord = sizeof(winrecord);
#endif



chartype gkbGetc (void)

  { /* gkbGetc */
    return (chartype) EOF;
  } /* gkbGetc */



booltype gkbKeyPressed (void)

  { /* gkbKeyPressed */
    return FALSE;
  } /* gkbKeyPressed */



booltype gkbButtonPressed (chartype button)

  { /* gkbButtonPressed */
    return FALSE;
  } /* gkbButtonPressed */



chartype gkbRawGetc (void)

  { /* gkbRawGetc */
    return gkbGetc();
  } /* gkbRawGetc */



wintype gkbWindow (void)

  { /* gkbWindow */
    return NULL;
  } /* gkbWindow */



inttype gkbButtonXpos (void)

  { /* gkbButtonXpos */
    return 0;
  } /* gkbButtonXpos */



inttype gkbButtonYpos (void)

  { /* gkbButtonYpos */
    return 0;
  } /* gkbButtonYpos */



inttype drwPointerXpos (const_wintype actual_window)

  { /* drwPointerXpos */
    return 0;
  } /* drwPointerXpos */



inttype drwPointerYpos (const_wintype actual_window)

  { /* drwPointerYpos */
    return 0;
  } /* drwPointerYpos */



void drwArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)

  { /* drwArc */
  } /* drwArc */



void drwPArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2, inttype col)

  { /* drwPArc */
  } /* drwPArc */



void drwFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)

  { /* drwFArcChord */
  } /* drwFArcChord */



void drwPFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2, inttype col)

  { /* drwPFArcChord */
  } /* drwPFArcChord */



void drwFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2)

  { /* drwFArcPieSlice */
  } /* drwFArcPieSlice */



void drwPFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype ang1, floattype ang2, inttype col)

  { /* drwPFArcPieSlice */
  } /* drwPFArcPieSlice */



void drwArc2 (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype radius)

  { /* drwArc2 */
  } /* drwArc2 */



void drwCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius)

  { /* drwCircle */
  } /* drwCircle */



void drwPCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)

  { /* drwPCircle */
  } /* drwPCircle */



void drwClear (wintype actual_window, inttype col)

  { /* drwClear */
  } /* drwClear */



void drwCopyArea (const_wintype src_window, const_wintype dest_window,
    inttype src_x, inttype src_y, inttype width, inttype height,
    inttype dest_x, inttype dest_y)

  { /* drwCopyArea */
  } /* drwCopyArea */



void drwFCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius)

  { /* drwFCircle */
  } /* drwFCircle */



void drwPFCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col)

  { /* drwPFCircle */
  } /* drwPFCircle */



void drwFEllipse (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height)

  { /* drwFEllipse */
  } /* drwFEllipse */



void drwPFEllipse (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height, inttype col)

  { /* drwPFEllipse */
  } /* drwPFEllipse */



void drwFlush (void)

  { /* drwFlush */
  } /* drwFlush */



void drwFree (wintype old_window)

  { /* drwFree */
  } /* drwFree */



wintype drwGet (const_wintype actual_window, inttype left, inttype upper,
    inttype width, inttype height)

  { /* drwGet */
    return NULL;
  } /* drwGet */



inttype drwGetPixel (const_wintype actual_window, inttype x, inttype y)

  { /* drwGetPixel */
    return 0;
  } /* drwGetPixel */



inttype drwHeight (const_wintype actual_window)

  { /* drwHeight */
    return 0;
  } /* drwHeight */



wintype drwImage (int32type *image_data, memsizetype width, memsizetype height)

  { /* drwImage */
    return NULL;
  } /* drwImage */



void drwLine (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2)

  { /* drwLine */
  } /* drwLine */



void drwPLine (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype col)

  { /* drwPLine */
  } /* drwPLine */



wintype drwNewPixmap (inttype width, inttype height)

  { /* drwNewPixmap */
    return NULL;
  } /* drwNewPixmap */



wintype drwOpen (inttype xPos, inttype yPos,
    inttype width, inttype height, const const_stritype window_name)

  { /* drwOpen */
    raise_error(FILE_ERROR);
    return NULL;
  } /* drwOpen */



wintype drwOpenSubWindow (const_wintype parent_window, inttype xPos, inttype yPos,
    inttype width, inttype height)

  { /* drwOpenSubWindow */
    raise_error(FILE_ERROR);
    return NULL;
  } /* drwOpenSubWindow */



void drwPoint (const_wintype actual_window, inttype x, inttype y)

  { /* drwPoint */
  } /* drwPoint */



void drwPPoint (const_wintype actual_window, inttype x, inttype y, inttype col)

  { /* drwPPoint */
  } /* drwPPoint */



bstritype drwGenPointList (const const_rtlArraytype xyArray)

  { /* drwGenPointList */
    return NULL;
  } /* drwGenPointList */



void drwPolyLine (const_wintype actual_window,
    inttype x, inttype y, bstritype point_list, inttype col)

  { /* drwPolyLine */
  } /* drwPolyLine */



void drwFPolyLine (const_wintype actual_window,
    inttype x, inttype y, bstritype point_list, inttype col)

  { /* drwFPolyLine */
  } /* drwFPolyLine */



void drwPut (const_wintype actual_window, const_wintype pixmap,
    inttype x, inttype y)

  { /* drwPut */
  } /* drwPut */



void drwRect (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height)

  { /* drwRect */
  } /* drwRect */



void drwPRect (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height, inttype col)

  { /* drwPRect */
  } /* drwPRect */



inttype drwRgbColor (inttype red_val, inttype green_val, inttype blue_val)

  { /* drwRgbColor */
    return 0;
  } /* drwRgbColor */



void drwPixelToRgb (inttype col, inttype *red_val, inttype *green_val, inttype *blue_val)

  { /* drwPixelToRgb */
    *red_val   = 0;
    *green_val = 0;
    *blue_val  = 0;
  } /* drwPixelToRgb */



void drwBackground (inttype col)

  { /* drwBackground */
  } /* drwBackground */



void drwColor (inttype col)

  { /* drwColor */
  } /* drwColor */



void drwSetContent (const_wintype actual_window, const_wintype pixmap)

  { /* drwSetContent */
  } /* drwSetContent */



void drwSetPos (const_wintype actual_window, inttype xPos, inttype yPos)

  { /* drwSetPos */
  } /* drwSetPos */



void drwSetTransparentColor (wintype pixmap, inttype col)

  { /* drwSetTransparentColor */
  } /* drwSetTransparentColor */



void drwText (const_wintype actual_window, inttype x, inttype y,
    const const_stritype stri, inttype col, inttype bkcol)

  { /* drwText */
  } /* drwText */



void drwToBottom (const_wintype actual_window)

  { /* drwToBottom */
  } /* drwToBottom */



void drwToTop (const_wintype actual_window)

  { /* drwToTop */
  } /* drwToTop */



inttype drwWidth (const_wintype actual_window)

  { /* drwWidth */
    return 0;
  } /* drwWidth */



inttype drwXPos (const_wintype actual_window)

  { /* drwXPos */
    return 0;
  } /* drwXPos */



inttype drwYPos (const_wintype actual_window)

  { /* drwYPos */
    return 0;
  } /* drwYPos */
