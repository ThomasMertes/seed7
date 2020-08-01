/********************************************************************/
/*                                                                  */
/*  rtl_err.h     Runtime error and exception handling procedures.  */
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
/*  File: seed7/src/rtl_err.h                                       */
/*  Changes: 1990, 1991, 1992, 1993, 1994, 2005  Thomas Mertes      */
/*  Content: Runtime error and exception handling procedures.       */
/*                                                                  */
/********************************************************************/

#define raise_error(num) raise_error2(num, __FILE__, __LINE__)


#ifdef ANSI_C

void raise_error2 (int, char *, int);

#else

void raise_error2 ();

#endif
