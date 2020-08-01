/********************************************************************/
/*                                                                  */
/*  scr_drv.h     Prototypes for screen access functions.           */
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
/*  File: seed7/src/scr_drv.h                                       */
/*  Changes: 1990, 1991, 1992, 1993, 1994, 2005  Thomas Mertes      */
/*  Content: Prototypes for screen access functions.                */
/*                                                                  */
/********************************************************************/

#define black 0
#define green 2
#define lightgray 7
#define darkgray 8
#define yellow 14
#define white 15


#ifdef ANSI_C

int scrHeight (void);
int scrWidth (void);
void scrFlush (void);
void scrCursor (booltype on);
void scrSetCursor (inttype lin, inttype col);
void scrText (inttype lin, inttype col, ustritype stri,
    memsizetype length);
void scrClear (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol);
void scrUpScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count);
void scrDownScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count);
void scrLeftScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count);
void scrRightScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count);
void scrShut (void);
int scrOpen (void);

#else

int scrHeight ();
int scrWidth ();
void scrFlush ();
void scrCursor ();
void scrSetCursor ();
void scrText ();
void scrClear ();
void scrUpScroll ();
void scrDownScroll ();
void scrLeftScroll ();
void scrRightScroll ();
void scrShut ();
int scrOpen ();

#endif
