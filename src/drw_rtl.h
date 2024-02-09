/********************************************************************/
/*                                                                  */
/*  drw_rtl.h     Platform independent drawing functions.           */
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
/*  File: seed7/src/drw_rtl.h                                       */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: Platform independent drawing functions.                */
/*                                                                  */
/********************************************************************/

void drwCpy (winType *const dest, const winType source);
winType drwCreate (const winType source);
void drwDestr (const winType old_win);
intType drwGetImagePixel (const_bstriType image, intType width,
    intType height, intType x, intType y);
winType drwGetPixmapFromPixels (const const_rtlArrayType image);
const_bstriType pltAlign (const const_bstriType pointList);
