/********************************************************************/
/*                                                                  */
/*  trm_drv.h     Driver for termcap and terminfo screen access     */
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
/*  File: seed7/src/trm_drv.h                                       */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Driver for termcap and terminfo screen access          */
/*                                                                  */
/********************************************************************/

#define putctln(s,n) ((s) != NULL ? tputs((s), (n), outch) : 0)
#define putctl(s) putctln((s), 1)
#define putstr(s) fputs((s), stdout)
#define putgoto(s,x,y) ((s) != NULL ? tputs(tgoto((s), (x), (y)), 1, outch) : 0)

#ifdef DO_INIT
booltype caps_initialized = FALSE;
#else
EXTERN booltype caps_initialized;
#endif


#ifdef ANSI_C

int getcaps (void);
#ifdef USE_TERMCAP
int outch (char ch);
#else
int outch (int ch);
#endif
void putcontrol (char *);

#else

int getcaps ();
int outch ();
void putcontrol ();

#endif
