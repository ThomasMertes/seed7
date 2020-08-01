/********************************************************************/
/*                                                                  */
/*  chr_rtl.h     Primitive actions for the integer type.           */
/*  Copyright (C) 1989 - 2006  Thomas Mertes                        */
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
/*  File: seed7/src/chr_rtl.h                                       */
/*  Changes: 1992, 1993, 1994, 2005  Thomas Mertes                  */
/*  Content: Primitive actions for the char type.                   */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

stritype chrCLit (chartype character);
inttype chrCmp (chartype char1, chartype char2);
void chrCpy (chartype *dest, chartype source);
chartype chrCreate (chartype source);
chartype chrLow (chartype ch);
stritype chrStr (chartype ch);
chartype chrUp (chartype ch);

#else

stritype chrCLit ();
inttype chrCmp ();
void chrCpy ();
chartype chrCreate ();
chartype chrLow ();
stritype chrStr ();
chartype chrUp ();

#endif
