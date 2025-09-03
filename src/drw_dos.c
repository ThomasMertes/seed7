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
#include "heaputl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "drw_drv.h"



charType gkbGetc (void)

  { /* gkbGetc */
    return (charType) EOF;
  } /* gkbGetc */



boolType gkbInputReady (void)

  { /* gkbInputReady */
    return FALSE;
  } /* gkbInputReady */



boolType gkbButtonPressed (charType button)

  { /* gkbButtonPressed */
    return FALSE;
  } /* gkbButtonPressed */



charType gkbRawGetc (void)

  { /* gkbRawGetc */
    return gkbGetc();
  } /* gkbRawGetc */



void gkbSelectInput (winType aWindow, charType aKey, boolType active)

  { /* gkbSelectInput */
  } /* gkbSelectInput */



winType gkbWindow (void)

  { /* gkbWindow */
    return NULL;
  } /* gkbWindow */



intType gkbClickedXpos (void)

  { /* gkbClickedXpos */
    return 0;
  } /* gkbClickedXpos */



intType gkbClickedYpos (void)

  { /* gkbClickedYpos */
    return 0;
  } /* gkbClickedYpos */



void drawClose (void)

  { /* drawClose */
  } /* drawClose */



void drawInit (void)

  { /* drawInit */
  } /* drawInit */



intType drwPointerXpos (const_winType actual_window)

  { /* drwPointerXpos */
    return 0;
  } /* drwPointerXpos */



intType drwPointerYpos (const_winType actual_window)

  { /* drwPointerYpos */
    return 0;
  } /* drwPointerYpos */



void drwPArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col)

  { /* drwPArc */
  } /* drwPArc */



void drwPFArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle,
    intType width, intType col)

  { /* drwPFArc */
  } /* drwPFArc */



void drwPFArcChord (const_winType actual_window, intType x, intType y,
    intType radius, floatType ang1, floatType ang2, intType col)

  { /* drwPFArcChord */
  } /* drwPFArcChord */



void drwPFArcPieSlice (const_winType actual_window, intType x, intType y,
    intType radius, floatType ang1, floatType ang2, intType col)

  { /* drwPFArcPieSlice */
  } /* drwPFArcPieSlice */



rtlArrayType drwBorder (const_winType actual_window)

  { /* drwBorder */
    return NULL;
  } /* drwBorder */



void drwPCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col)

  { /* drwPCircle */
  } /* drwPCircle */



void drwClear (winType actual_window, intType col)

  { /* drwClear */
  } /* drwClear */



void drwCopyArea (const_winType src_window, const_winType dest_window,
    intType src_x, intType src_y, intType width, intType height,
    intType dest_x, intType dest_y)

  { /* drwCopyArea */
  } /* drwCopyArea */



void drwPFCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col)

  { /* drwPFCircle */
  } /* drwPFCircle */



void drwPFEllipse (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col)

  { /* drwPFEllipse */
  } /* drwPFEllipse */



void drwFlush (void)

  { /* drwFlush */
  } /* drwFlush */



winType drwEmpty (void)

  { /* drwEmpty */
    return NULL;
  } /* drwEmpty */



void drwFree (winType old_window)

  { /* drwFree */
  } /* drwFree */



winType drwCapture (intType left, intType upper,
    intType width, intType height)

  { /* drwCapture */
    return NULL;
  } /* drwCapture */



intType drwGetPixel (const_winType source_window, intType x, intType y)

  { /* drwGetPixel */
    return 0;
  } /* drwGetPixel */



bstriType drwGetPixelData (const_winType source_window)

  {
    bstriType result;

  /* drwGetPixelData */
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, 0))) {
      raise_error(RANGE_ERROR);
    } else {
      result->size = 0;
    } /* if */
    return result;
  } /* drwGetPixelData */



winType drwGetPixmap (const_winType source_window, intType left, intType upper,
    intType width, intType height)

  { /* drwGetPixmap */
    return NULL;
  } /* drwGetPixmap */



intType drwHeight (const_winType actual_window)

  { /* drwHeight */
    return 0;
  } /* drwHeight */



winType drwImage (int32Type *image_data, memSizeType width, memSizeType height,
    boolType hasAlphaChannel)

  { /* drwImage */
    return NULL;
  } /* drwImage */



void drwPLine (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2, intType col)

  { /* drwPLine */
  } /* drwPLine */



winType drwNewPixmap (intType width, intType height)

  { /* drwNewPixmap */
    return NULL;
  } /* drwNewPixmap */



winType drwOpen (intType xPos, intType yPos,
    intType width, intType height, const const_striType windowName)

  { /* drwOpen */
    raise_error(GRAPHIC_ERROR);
    return NULL;
  } /* drwOpen */



winType drwOpenSubWindow (const_winType parent_window, intType xPos, intType yPos,
    intType width, intType height)

  { /* drwOpenSubWindow */
    raise_error(GRAPHIC_ERROR);
    return NULL;
  } /* drwOpenSubWindow */



void drwSetCloseAction (winType actual_window, intType closeAction)

  { /* drwSetCloseAction */
  } /* drwSetCloseAction */



void drwSetCursorVisible (winType aWindow, boolType visible)

  { /* drwSetCursorVisible */
  } /* drwSetCursorVisible */



void drwSetPointerPos (const_winType aWindow, intType xPos, intType yPos)

  { /* drwSetPointerPos */
  } /* drwSetPointerPos */



void drwPPoint (const_winType actual_window, intType x, intType y, intType col)

  { /* drwPPoint */
  } /* drwPPoint */



rtlArrayType drwConvPointList (const const_bstriType pointList)

  { /* drwConvPointList */
    return NULL;
  } /* drwConvPointList */



bstriType drwGenPointList (const const_rtlArrayType xyArray)

  { /* drwGenPointList */
    return NULL;
  } /* drwGenPointList */



void drwPolyLine (const_winType actual_window,
    intType x, intType y, bstriType point_list, intType col)

  { /* drwPolyLine */
  } /* drwPolyLine */



void drwFPolyLine (const_winType actual_window,
    intType x, intType y, bstriType point_list, intType col)

  { /* drwFPolyLine */
  } /* drwFPolyLine */



void drwPut (const_winType destWindow, intType xDest, intType yDest,
    const_winType pixmap)

  { /* drwPut */
  } /* drwPut */



void drwPutScaled (const_winType destWindow, intType xDest, intType yDest,
    intType width, intType height, const_winType pixmap)

  { /* drwPutScaled */
  } /* drwPutScaled */



void drwPRect (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col)

  { /* drwPRect */
  } /* drwPRect */



intType drwRgbColor (intType redLight, intType greenLight, intType blueLight)

  { /* drwRgbColor */
    return 0;
  } /* drwRgbColor */



void drwPixelToRgb (intType col, intType *redLight, intType *greenLight, intType *blueLight)

  { /* drwPixelToRgb */
    *redLight   = 0;
    *greenLight = 0;
    *blueLight  = 0;
  } /* drwPixelToRgb */



void drwBackground (intType col)

  { /* drwBackground */
  } /* drwBackground */



void drwColor (intType col)

  { /* drwColor */
  } /* drwColor */



intType drwScreenHeight (void)

  { /* drwScreenHeight */
    return 0;
  } /* drwScreenHeight */



intType drwScreenWidth (void)

  { /* drwScreenWidth */
    return 0;
  } /* drwScreenWidth */



void drwSetContent (const_winType actual_window, const_winType pixmap)

  { /* drwSetContent */
  } /* drwSetContent */



void drwSetPos (const_winType actual_window, intType xPos, intType yPos)

  { /* drwSetPos */
  } /* drwSetPos */



void drwSetTransparentColor (winType pixmap, intType col)

  { /* drwSetTransparentColor */
  } /* drwSetTransparentColor */



void drwSetWindowName (winType aWindow, const const_striType windowName)

  { /* drwSetWindowName */
  } /* drwSetWindowName */



void drwText (const_winType actual_window, intType x, intType y,
    const const_striType stri, intType col, intType bkcol)

  { /* drwText */
  } /* drwText */



void drwToBottom (const_winType actual_window)

  { /* drwToBottom */
  } /* drwToBottom */



void drwToTop (const_winType actual_window)

  { /* drwToTop */
  } /* drwToTop */



intType drwWidth (const_winType actual_window)

  { /* drwWidth */
    return 0;
  } /* drwWidth */



intType drwXPos (const_winType actual_window)

  { /* drwXPos */
    return 0;
  } /* drwXPos */



intType drwYPos (const_winType actual_window)

  { /* drwYPos */
    return 0;
  } /* drwYPos */
