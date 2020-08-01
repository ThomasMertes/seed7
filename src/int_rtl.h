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

void uint2_mult (uinttype a_high, uinttype a_low,
                 uinttype b_high, uinttype b_low,
                 uinttype *c_high, uinttype *c_low);
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
inttype intParse (const const_stritype stri);
inttype intPow (inttype base, inttype exponent);
stritype intRadix (inttype number, inttype base, booltype upperCase);
stritype intRadixPow2 (inttype number, int shift, int mask, booltype upperCase);
inttype intRand (inttype low, inttype high);
inttype intSqrt (inttype number);
stritype intStr (inttype number);
#ifdef ALLOW_STRITYPE_SLICES
stritype intStrToBuffer (inttype number, stritype buffer);
#endif
