/********************************************************************/
/*                                                                  */
/*  drw_drv.h     Prototypes for drawing functions.                 */
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
/*  File: seed7/src/drw_drv.h                                       */
/*  Changes: 1994, 2000, 2004  Thomas Mertes                        */
/*  Content: Prototypes for drawing functions.                      */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

chartype gkbGetc (void);
booltype gkbKeyPressed (void);
chartype gkbRawGetc (void);
inttype gkbXpos (void);
inttype gkbYpos (void);
void drwArc (wintype, inttype, inttype, inttype, floattype, floattype);
void drwFArcChord (wintype, inttype, inttype, inttype, floattype, floattype);
void drwFArcPieSlice (wintype, inttype, inttype, inttype, floattype, floattype);
void drwArc2 (wintype, inttype, inttype, inttype, inttype, inttype);
void drwCircle (wintype, inttype, inttype, inttype);
void drwClear (wintype, inttype);
void drwCpy (wintype *, wintype);
void drwFCircle (wintype, inttype, inttype, inttype);
void drwFEllipse (wintype, inttype, inttype, inttype, inttype);
void drwFlush (void);
void drwFree (wintype);
wintype drwGet (wintype, inttype, inttype, inttype, inttype);
inttype drwHeight (wintype);
wintype drwImage (wintype, inttype *, inttype, inttype);
void drwLine (wintype, inttype, inttype, inttype, inttype);
void drwPLine (wintype, inttype, inttype, inttype, inttype, inttype);
wintype drwNewPixmap (wintype, inttype, inttype);
wintype drwNewBitmap (wintype, inttype, inttype);
wintype drwOpen (inttype, inttype, inttype, inttype, stritype);
void drwPoint (wintype, inttype, inttype);
void drwPPoint (wintype, inttype, inttype, inttype);
void drwPut (wintype, wintype, inttype, inttype);
void drwRect (wintype, inttype, inttype, inttype, inttype);
void drwPRect (wintype, inttype, inttype, inttype, inttype, inttype);
inttype drwRgbColor (inttype, inttype, inttype);
void drwBackground (inttype);
void drwColor (inttype);
void drwText (wintype, inttype, inttype, stritype);
inttype drwWidth (wintype);

#else

chartype gkbGetc ();
booltype gkbKeyPressed ();
chartype gkbRawGetc ();
inttype gkbXpos ();
inttype gkbYpos ();
void drwArc ();
void drwFArcChord ();
void drwFArcPieSlice ();
void drwArc2 ();
void drwCircle ();
void drwClear ();
void drwCpy ();
void drwFCircle ();
void drwFEllipse ();
void drwFlush ();
void drwFree ();
inttype drwGet ();
inttype drwHeight ();
wintype drwImage ();
void drwLine ();
void drwPLine ();
wintype drwNewPixmap ();
wintype drwNewBitmap ();
wintype drwOpen ();
void drwPoint ();
void drwPPoint ();
void drwPut ();
void drwRect ();
void drwPRect ();
inttype drwRgbColor ();
void drwBackground ();
void drwColor ();
void drwText ();
inttype drwWidth ();

#endif
