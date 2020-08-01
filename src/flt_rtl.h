/********************************************************************/
/*                                                                  */
/*  flt_rtl.h     Primitive actions for the float type.             */
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
/*  File: seed7/src/flt_rtl.h                                       */
/*  Changes: 1993, 1994, 2005  Thomas Mertes                        */
/*  Content: Primitive actions for the float type.                  */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

inttype fltCmp (floattype, floattype);
void fltCpy (floattype *, floattype);
void fltCreate (floattype *, floattype);
stritype fltDgts (floattype, inttype);
floattype fltIpow (floattype, inttype);
floattype fltParse (stritype);
floattype fltPow (floattype, floattype);
floattype fltRand (floattype, floattype);
stritype fltStr (floattype);

#else

inttype fltCmp ();
void fltCpy ();
void fltCreate ();
stritype fltDgts ();
floattype fltIpow ();
floattype fltParse ();
floattype fltPow ();
floattype fltRand ();
stritype fltStr ();

#endif
