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
typedef wstritype console_stritype;
#else
typedef ustritype console_stritype;
#endif


int conHeight (void);
int conWidth (void);
void conFlush (void);
void conCursor (booltype on);
void conSetCursor (inttype lin, inttype col);
void conText (inttype lin, inttype col, console_stritype stri,
    memsizetype length);
void conClear (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol);
void conUpScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count);
void conDownScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count);
void conLeftScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count);
void conRightScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count);
void conShut (void);
int conOpen (void);
