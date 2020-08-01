/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Analyzer                                                */
/*  File: seed7/src/stat.h                                          */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Procedures for maintaining an analyze phase statistic. */
/*                                                                  */
/********************************************************************/

#ifdef WITH_COMPILATION_INFO
EXTERN unsigned long total_lines;
#endif
#ifdef WITH_STATISTIK
EXTERN unsigned int literal_count;
EXTERN unsigned int list_count;
EXTERN unsigned int comment_count;
#endif


#ifdef ANSI_C

void show_statistic (void);
void reset_statistic (void);

#else

void show_statistic ();
void reset_statistic ();

#endif
