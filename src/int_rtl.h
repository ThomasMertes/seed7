/********************************************************************/
/*                                                                  */
/*  int_rtl.h     Primitive actions for the integer type.           */
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
/*  File: seed7/src/int_rtl.h                                       */
/*  Changes: 1992, 1993, 1994, 2000, 2005  Thomas Mertes            */
/*  Content: Primitive actions for the integer type.                */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C
#include "limits.h"
#endif


#ifndef ULONG_MAX
#define ULONG_MAX ((uinttype) 4294967295L)
#endif


#ifdef ANSI_C

uinttype rand_32 (void);
inttype most_significant_bit (uinttype);
inttype least_significant_bit (uinttype);
inttype intBinom (inttype, inttype);
void intCpy (inttype *, inttype);
void intCreate (inttype *, inttype);
inttype intLd (inttype);
inttype intParse (stritype);
inttype intPow (inttype, inttype);
inttype intRand (inttype, inttype);
inttype intSqrt (inttype);
stritype intStr (inttype);

#else

uinttype rand_32 ();
inttype most_significant_bit ();
inttype least_significant_bit ();
inttype intBinom ();
void intCpy ();
void intCreate ();
inttype intLd ();
inttype intParse ();
inttype intPow ();
inttype intRand ();
inttype intSqrt ();
stritype intStr ();

#endif
