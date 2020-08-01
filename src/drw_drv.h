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

#ifdef ANSI_C

chartype gkbGetc (void);
booltype gkbKeyPressed (void);
chartype gkbRawGetc (void);
wintype gkbWindow (void);
booltype gkbButtonPressed (chartype button);
inttype gkbButtonXpos (void);
inttype gkbButtonYpos (void);
inttype drwPointerXpos (const_wintype actual_window);
inttype drwPointerYpos (const_wintype actual_window);
void drwArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle);
void drwPArc (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle, inttype col);
void drwFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle);
void drwPFArcChord (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle, inttype col);
void drwFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle);
void drwPFArcPieSlice (const_wintype actual_window, inttype x, inttype y,
    inttype radius, floattype startAngle, floattype sweepAngle, inttype col);
void drwArc2 (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype radius);
void drwCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius);
void drwPCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col);
void drwClear (wintype actual_window, inttype col);
void drwCopyArea (const_wintype src_window, const_wintype dest_window,
    inttype src_x, inttype src_y, inttype width, inttype height,
    inttype dest_x, inttype dest_y);
void drwFCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius);
void drwPFCircle (const_wintype actual_window,
    inttype x, inttype y, inttype radius, inttype col);
void drwFEllipse (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height);
void drwPFEllipse (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height, inttype col);
void drwFlush (void);
void drwFree (wintype old_window);
wintype drwGet (const_wintype actual_window, inttype left, inttype upper,
    inttype width, inttype height);
inttype drwGetPixel (const_wintype actual_window, inttype x, inttype y);
void drwPixelToRgb (inttype col, inttype *red_val, inttype *green_val, inttype *blue_val);
inttype drwHeight (const_wintype actual_window);
wintype drwImage (inttype *image_data, inttype width, inttype height);
void drwLine (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2);
void drwPLine (const_wintype actual_window,
    inttype x1, inttype y1, inttype x2, inttype y2, inttype col);
wintype drwNewPixmap (inttype width, inttype height);
wintype drwNewBitmap (const_wintype actual_window, inttype width, inttype height);
wintype drwOpen (inttype xPos, inttype yPos,
    inttype width, inttype height, const const_stritype window_name);
wintype drwOpenSubWindow (const_wintype parent_window, inttype xPos, inttype yPos,
    inttype width, inttype height);
void drwPoint (const_wintype actual_window, inttype x, inttype y);
void drwPPoint (const_wintype actual_window, inttype x, inttype y, inttype col);
void drwConvPointList (bstritype pointList, inttype *xy);
bstritype drwGenPointList (const const_rtlArraytype xyArray);
inttype drwLngPointList (bstritype point_list);
void drwPolyLine (const_wintype actual_window,
    inttype x1, inttype y1, bstritype point_list, inttype col);
void drwFPolyLine (const_wintype actual_window,
    inttype x1, inttype y1, bstritype point_list, inttype col);
void drwPut (const_wintype actual_window, const_wintype pixmap,
    inttype x, inttype y);
void drwRect (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height);
void drwPRect (const_wintype actual_window,
    inttype x, inttype y, inttype width, inttype height, inttype col);
inttype drwRgbColor (inttype red_val, inttype green_val, inttype blue_val);
void drwBackground (inttype col);
void drwColor (inttype col);
void drwSetContent (const_wintype actual_window, const_wintype pixmap);
void drwSetPos (const_wintype actual_window, inttype xPos, inttype yPos);
void drwSetTransparentColor (wintype pixmap, inttype col);
void drwText (const_wintype actual_window, inttype x, inttype y,
    const const_stritype stri, inttype col, inttype bkcol);
void drwToBottom (const_wintype actual_window);
void drwToTop (const_wintype actual_window);
inttype drwWidth (const_wintype actual_window);
inttype drwXPos (const_wintype actual_window);
inttype drwYPos (const_wintype actual_window);

#else

chartype gkbGetc ();
booltype gkbKeyPressed ();
chartype gkbRawGetc ();
wintype gkbWindow ();
booltype gkbButtonPressed ();
inttype gkbButtonXpos ();
inttype gkbButtonYpos ();
inttype drwPointerXpos ();
inttype drwPointerYpos ();
void drwArc ();
void drwPArc ();
void drwFArcChord ();
void drwPFArcChord ();
void drwFArcPieSlice ();
void drwPFArcPieSlice ();
void drwArc2 ();
void drwCircle ();
void drwPCircle ();
void drwClear ();
void drwCopyArea ();
void drwFCircle ();
void drwPFCircle ();
void drwFEllipse ();
void drwPFEllipse ();
void drwFlush ();
void drwFree ();
wintype drwGet ();
inttype drwGetPixel ();
void drwPixelToRgb ();
inttype drwHeight ();
wintype drwImage ();
void drwLine ();
void drwPLine ();
wintype drwNewPixmap ();
wintype drwNewBitmap ();
wintype drwOpen ();
wintype drwOpenSubWindow ();
void drwPoint ();
void drwPPoint ();
void drwConvPointList ();
bstritype drwGenPointList ();
inttype drwLngPointList ();
void drwPolyLine ();
void drwFPolyLine ();
void drwPut ();
void drwRect ();
void drwPRect ();
inttype drwRgbColor ();
void drwBackground ();
void drwColor ();
void drwSetContent ();
void drwSetPos ();
void drwSetTransparentColor ();
void drwText ();
void drwToBottom ();
void drwToTop ();
inttype drwWidth ();
inttype drwXPos ();
inttype drwYPos ();

#endif
