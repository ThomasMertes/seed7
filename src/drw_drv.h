/********************************************************************/
/*                                                                  */
/*  drw_drv.h     Prototypes for drawing functions.                 */
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
/*  File: seed7/src/drw_drv.h                                       */
/*  Changes: 1994, 2000, 2004  Thomas Mertes                        */
/*  Content: Prototypes for drawing functions.                      */
/*                                                                  */
/********************************************************************/

charType gkbGetc (void);
boolType gkbKeyPressed (void);
charType gkbRawGetc (void);
winType gkbWindow (void);
boolType gkbButtonPressed (charType button);
intType gkbButtonXpos (void);
intType gkbButtonYpos (void);
intType drwPointerXpos (const_winType actual_window);
intType drwPointerYpos (const_winType actual_window);
void drwArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle);
void drwPArc (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col);
void drwFArcChord (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle);
void drwPFArcChord (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col);
void drwFArcPieSlice (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle);
void drwPFArcPieSlice (const_winType actual_window, intType x, intType y,
    intType radius, floatType startAngle, floatType sweepAngle, intType col);
void drwArc2 (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2, intType radius);
void drwCircle (const_winType actual_window,
    intType x, intType y, intType radius);
void drwPCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col);
void drwClear (winType actual_window, intType col);
void drwColor (intType col);
rtlArrayType drwConvPointList (const const_bstriType pointList);
void drwCopyArea (const_winType src_window, const_winType dest_window,
    intType src_x, intType src_y, intType width, intType height,
    intType dest_x, intType dest_y);
void drwFCircle (const_winType actual_window,
    intType x, intType y, intType radius);
void drwPFCircle (const_winType actual_window,
    intType x, intType y, intType radius, intType col);
void drwFEllipse (const_winType actual_window,
    intType x, intType y, intType width, intType height);
void drwPFEllipse (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col);
winType drwEmpty (void);
void drwFlush (void);
void drwFree (winType old_window);
winType drwGet (const_winType actual_window, intType left, intType upper,
    intType width, intType height);
bstriType drwGetImage (const_winType actual_window);
intType drwGetPixel (const_winType actual_window, intType x, intType y);
void drwPixelToRgb (intType col, intType *redLight, intType *greenLight, intType *blueLight);
intType drwHeight (const_winType actual_window);
winType drwImage (int32Type *image_data, memSizeType width, memSizeType height);
void drwLine (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2);
void drwPLine (const_winType actual_window,
    intType x1, intType y1, intType x2, intType y2, intType col);
winType drwNewPixmap (intType width, intType height);
winType drwNewBitmap (const_winType actual_window, intType width, intType height);
winType drwOpen (intType xPos, intType yPos,
    intType width, intType height, const const_striType window_name);
winType drwOpenSubWindow (const_winType parent_window, intType xPos, intType yPos,
    intType width, intType height);
void drwPoint (const_winType actual_window, intType x, intType y);
void drwPPoint (const_winType actual_window, intType x, intType y, intType col);
bstriType drwGenPointList (const const_rtlArrayType xyArray);
intType drwLngPointList (bstriType point_list);
void drwPolyLine (const_winType actual_window,
    intType x, intType y, bstriType point_list, intType col);
void drwFPolyLine (const_winType actual_window,
    intType x, intType y, bstriType point_list, intType col);
void drwPut (const_winType actual_window, const_winType pixmap,
    intType x, intType y);
void drwRect (const_winType actual_window,
    intType x, intType y, intType width, intType height);
void drwPRect (const_winType actual_window,
    intType x, intType y, intType width, intType height, intType col);
intType drwRgbColor (intType redLight, intType greenLight, intType blueLight);
void drwBackground (intType col);
void drwSetContent (const_winType actual_window, const_winType pixmap);
void drwSetPos (const_winType actual_window, intType xPos, intType yPos);
void drwSetTransparentColor (winType pixmap, intType col);
void drwText (const_winType actual_window, intType x, intType y,
    const const_striType stri, intType col, intType bkcol);
void drwToBottom (const_winType actual_window);
void drwToTop (const_winType actual_window);
intType drwWidth (const_winType actual_window);
intType drwXPos (const_winType actual_window);
intType drwYPos (const_winType actual_window);
