/********************************************************************/
/*                                                                  */
/*  drw_rtl.h     Generic graphic drawing functions.                */
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
/*  Content: Generic graphic drawing functions.                     */
/*                                                                  */
/********************************************************************/

void drwCpy (wintype *const win_to, const wintype win_from);
wintype drwCreate (const wintype win_from);
void drwDestr (const wintype old_win);
inttype drwGetImagePixel (const_bstritype image, inttype width,
    inttype height, inttype x, inttype y);
wintype drwRtlImage (const const_rtlArraytype image);
