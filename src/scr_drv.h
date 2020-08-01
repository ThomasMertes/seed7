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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
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
void scrCursor (booltype);
void scrSetpos (inttype, inttype);
void scrWrite (inttype, inttype, ustritype, memsizetype);
void scrClear (inttype, inttype, inttype, inttype);
void scrUpScroll (inttype, inttype, inttype, inttype, inttype);
void scrDownScroll (inttype, inttype, inttype, inttype, inttype);
void scrLeftScroll (inttype, inttype, inttype, inttype, inttype);
void scrRightScroll (inttype, inttype, inttype, inttype, inttype);
void scrShut (void);
int scrOpen (void);

#else

int scrHeight ();
int scrWidth ();
void scrFlush ();
void scrCursor ();
void scrSetpos ();
void scrWrite ();
void scrClear ();
void scrUpScroll ();
void scrDownScroll ();
void scrLeftScroll ();
void scrRightScroll ();
void scrShut ();
int scrOpen ();

#endif
