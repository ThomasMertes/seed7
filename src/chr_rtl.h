/********************************************************************/
/*                                                                  */
/*  chr_rtl.h     Primitive actions for the integer type.           */
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
/*  File: seed7/src/chr_rtl.h                                       */
/*  Changes: 1992, 1993, 1994, 2005  Thomas Mertes                  */
/*  Content: Primitive actions for the char type.                   */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

inttype chrCmp (chartype, chartype);
void chrCpy (chartype *, chartype);
void chrCreate (chartype *, chartype);
stritype chrStr (chartype);

#else

inttype chrCmp ();
void chrCpy ();
void chrCreate ();
stritype chrStr ();

#endif
