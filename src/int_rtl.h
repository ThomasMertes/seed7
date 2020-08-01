/********************************************************************/
/*                                                                  */
/*  int_rtl.h     Primitive actions for the integer type.           */
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
inttype most_significant_bit (uinttype number);
inttype least_significant_bit (uinttype number);
inttype intBinom (inttype n_number, inttype k_number);
inttype intBitLength (inttype number);
inttype intCmp (inttype number1, inttype number2);
void intCpy (inttype *dest, inttype source);
inttype intCreate (inttype source);
inttype intLog2 (inttype number);
inttype intLowestSetBit (inttype number);
stritype intLpad0 (inttype number, const inttype pad_size);
inttype intParse (const_stritype stri);
inttype intPow (inttype base, inttype exponent);
inttype intRand (inttype lower_limit, inttype upper_limit);
inttype intSqrt (inttype number);
stritype intStr (inttype number);
stritype intStrBased (inttype number, inttype base);
stritype intStrHex (inttype number);

#else

uinttype rand_32 ();
inttype most_significant_bit ();
inttype least_significant_bit ();
inttype intBinom ();
inttype intBitLength ();
inttype intCmp ();
void intCpy ();
inttype intCreate ();
inttype intLog2 ();
inttype intLowestSetBit ();
stritype intLpad0 ();
inttype intParse ();
inttype intPow ();
inttype intRand ();
inttype intSqrt ();
stritype intStr ();
stritype intStrBased ();
stritype intStrHex ();

#endif
