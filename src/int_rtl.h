/********************************************************************/
/*                                                                  */
/*  int_rtl.h     Primitive actions for the integer type.           */
/*  Copyright (C) 1989 - 2010  Thomas Mertes                        */
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
/*  Changes: 1992, 1993, 1994, 2000, 2005, 2010  Thomas Mertes      */
/*  Content: Primitive actions for the integer type.                */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

uinttype uint_rand (void);
int uint8MostSignificantBit (uint8type number);
int uint16MostSignificantBit (uint16type number);
int uint32MostSignificantBit (uint32type number);
#ifdef INT64TYPE
int uint64MostSignificantBit (uint64type number);
#endif
int uint8LeastSignificantBit (uint8type number);
int uint16LeastSignificantBit (uint16type number);
int uint32LeastSignificantBit (uint32type number);
#ifdef INT64TYPE
int uint64LeastSignificantBit (uint64type number);
#endif
inttype intBinom (inttype n_number, inttype k_number);
inttype intBitLength (inttype number);
inttype intCmp (inttype number1, inttype number2);
void intCpy (inttype *dest, inttype source);
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

uinttype uint_rand ();
int uint8MostSignificantBit ();
int uint16MostSignificantBit ();
int uint32MostSignificantBit ();
#ifdef INT64TYPE
int uint64MostSignificantBit ();
#endif
int uint8LeastSignificantBit ();
int uint16LeastSignificantBit ();
int uint32LeastSignificantBit ();
#ifdef INT64TYPE
int uint64LeastSignificantBit ();
#endif
inttype intBinom ();
inttype intBitLength ();
inttype intCmp ();
void intCpy ();
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
