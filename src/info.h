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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Analyzer                                                */
/*  File: seed7/src/info.h                                          */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Procedures for compile time info.                      */
/*                                                                  */
/********************************************************************/

#ifdef WITH_COMPILATION_INFO
#define INCR_LINE_COUNT(LIN) if (LIN++ == in_file.next_msg_line) line_compilation_info()
#else
#define INCR_LINE_COUNT(LIN) (LIN++)
#define display_compilation_info()
#define open_compilation_info()
#endif

#define CR_LIN_INFO()      printf("%6u\r", in_file.line)
#define NL_LIN_INFO()      printf("%6u\n", in_file.line)
#define CR_FIL_LIN_INFO()  printf("%6u %s\r", in_file.line, in_file.name)
#define NL_FIL_LIN_INFO()  printf("%6u %s\n", in_file.line, in_file.name)


#ifdef WITH_COMPILATION_INFO

#ifdef ANSI_C

void display_compilation_info (void);
void line_compilation_info (void);
void open_compilation_info (void);

#else

void display_compilation_info ();
void line_compilation_info ();
void open_compilation_info ();

#endif

#endif
