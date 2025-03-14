/********************************************************************/
/*                                                                  */
/*  con_drv.h     Prototypes for console access functions.          */
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
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/con_drv.h                                       */
/*  Changes: 1990, 1991, 1992, 1993, 1994, 2005  Thomas Mertes      */
/*  Content: Prototypes for console access functions.               */
/*                                                                  */
/********************************************************************/

#define black 0
#define green 2
#define lightgray 7
#define darkgray 8
#define yellow 14
#define white 15


#ifdef CONSOLE_WCHAR
typedef utf16striType console_striType;
#else
typedef ustriType console_striType;
#endif


int conHeight (void);
int conWidth (void);
void conFlush (void);
void conCursor (boolType on);
void conSetCursor (intType lin, intType col);
#ifdef CONSOLE_USES_CON_TEXT
void conText (intType lin, intType col, console_striType stri,
    memSizeType length);
#else
intType conColumn (void);
intType conLine (void);
void conWrite (const const_striType stri);
#endif
void conClear (intType startlin, intType startcol,
    intType stoplin, intType stopcol);
void conUpScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numLines);
void conDownScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numLines);
void conLeftScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numCols);
void conRightScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numCols);
void conShut (void);
int conOpen (void);
