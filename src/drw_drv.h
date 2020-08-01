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
booltype gkbButtonPressed (chartype);
inttype gkbButtonXpos (void);
inttype gkbButtonYpos (void);
inttype drwPointerXpos (const_wintype);
inttype drwPointerYpos (const_wintype);
void drwArc (const_wintype, inttype, inttype, inttype, floattype, floattype);
void drwPArc (const_wintype, inttype, inttype, inttype, floattype, floattype, inttype);
void drwFArcChord (const_wintype, inttype, inttype, inttype, floattype, floattype);
void drwPFArcChord (const_wintype, inttype, inttype, inttype, floattype, floattype, inttype);
void drwFArcPieSlice (const_wintype, inttype, inttype, inttype, floattype, floattype);
void drwPFArcPieSlice (const_wintype, inttype, inttype, inttype, floattype, floattype, inttype);
void drwArc2 (const_wintype, inttype, inttype, inttype, inttype, inttype);
void drwCircle (const_wintype, inttype, inttype, inttype);
void drwPCircle (const_wintype, inttype, inttype, inttype, inttype);
void drwClear (const_wintype, inttype);
void drwCopyArea (const_wintype, const_wintype, inttype, inttype, inttype, inttype,
                  inttype, inttype);
void drwFCircle (const_wintype, inttype, inttype, inttype);
void drwPFCircle (const_wintype, inttype, inttype, inttype, inttype);
void drwFEllipse (const_wintype, inttype, inttype, inttype, inttype);
void drwPFEllipse (const_wintype, inttype, inttype, inttype, inttype, inttype);
void drwFlush (void);
void drwFree (wintype);
wintype drwGet (const_wintype, inttype, inttype, inttype, inttype);
inttype drwHeight (const_wintype);
wintype drwImage (const_wintype, inttype *, inttype, inttype);
void drwLine (const_wintype, inttype, inttype, inttype, inttype);
void drwPLine (const_wintype, inttype, inttype, inttype, inttype, inttype);
wintype drwNewPixmap (const_wintype, inttype, inttype);
wintype drwNewBitmap (const_wintype, inttype, inttype);
wintype drwOpen (inttype, inttype, inttype, inttype, stritype);
wintype drwOpenSubWindow (const_wintype, inttype, inttype, inttype, inttype);
void drwPoint (const_wintype, inttype, inttype);
void drwPPoint (const_wintype, inttype, inttype, inttype);
void drwConvPointList (bstritype, inttype *);
bstritype drwGenPointList (const const_rtlArraytype xyArray);
inttype drwLngPointList (bstritype);
void drwPolyLine (const_wintype, inttype, inttype, bstritype, inttype);
void drwFPolyLine (const_wintype, inttype, inttype, bstritype, inttype);
void drwPut (const_wintype, const_wintype, inttype, inttype);
void drwRect (const_wintype, inttype, inttype, inttype, inttype);
void drwPRect (const_wintype, inttype, inttype, inttype, inttype, inttype);
inttype drwRgbColor (inttype, inttype, inttype);
void drwBackground (inttype);
void drwColor (inttype);
void drwSetPos (const_wintype, inttype, inttype);
void drwSetTransparentColor (wintype, inttype);
void drwText (const_wintype, inttype, inttype, stritype, inttype, inttype);
inttype drwWidth (const_wintype);
inttype drwXPos (const_wintype);
inttype drwYPos (const_wintype);

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
void drwSetPos ();
void drwSetTransparentColor ();
void drwText ();
inttype drwWidth ();
inttype drwXPos ();
inttype drwYPos ();

#endif
