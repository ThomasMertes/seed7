/********************************************************************/
/*                                                                  */
/*  con_rtl.h     Primitive actions for console/terminal output.    */
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
/*  File: seed7/src/con_rtl.h                                       */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: Primitive actions for console/terminal output.         */
/*                                                                  */
/********************************************************************/

void conHScroll (intType startlin, intType startcol,
                 intType stoplin, intType stopcol, intType numCols);
void conSetpos (intType lin, intType col);
void conVScroll (intType startlin, intType startcol,
                 intType stoplin, intType stopcol, intType numLines);
#ifdef CONSOLE_USES_CON_TEXT
intType conColumn (void);
intType conLine (void);
void conWrite (const const_striType stri);
#endif
