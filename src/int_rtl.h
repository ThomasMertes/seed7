/********************************************************************/
/*                                                                  */
/*  int_rtl.h     Primitive actions for the integer type.           */
/*  Copyright (C) 1989 - 2015  Thomas Mertes                        */
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
/*  Changes: 1992 - 1994, 2000, 2005, 2009 - 2015  Thomas Mertes    */
/*  Content: Primitive actions for the integer type.                */
/*                                                                  */
/********************************************************************/

extern const char lcDigits[];
extern const char ucDigits[];
extern const const_cstriType digitTable[];


void setupRand (void);
uintType uint_mult (uintType factor1, uintType factor2, uintType *product_high);
uintType uintRand (void);
uintType uintRandLimited (uintType rand_max);
int uint8MostSignificantBit (uint8Type number);
int uint16MostSignificantBit (uint16Type number);
int uint32MostSignificantBit (uint32Type number);
#ifdef INT64TYPE
int uint64MostSignificantBit (uint64Type number);
#endif
int uint8LeastSignificantBit (uint8Type number);
int uint16LeastSignificantBit (uint16Type number);
int uint32LeastSignificantBit (uint32Type number);
#ifdef INT64TYPE
int uint64LeastSignificantBit (uint64Type number);
#endif
striType uintRadix (uintType number, intType base, boolType upperCase);
striType uintRadixPow2 (uintType number, int shift, int mask, boolType upperCase);
striType uintStr (uintType number);
intType intBinom (intType n_number, intType k_number);
uintType uintBinomNoChk (uintType n_number, intType k_number);
intType intBitLength (intType number);
striType intBytesBe (intType number, boolType isSigned);
intType intBytesBe2Int (const const_striType byteStri);
intType intBytesBe2UInt (const const_striType byteStri);
striType intBytesLe (intType number, boolType isSigned);
intType intBytesLe2Int (const const_striType byteStri);
intType intBytesLe2UInt (const const_striType byteStri);
intType intCmp (intType number1, intType number2);
intType intLog10 (intType number);
intType intLog2 (intType number);
intType intLowestSetBit (intType number);
striType intLpad0 (intType number, const intType pad_size);
intType intParse (const const_striType stri);
intType intPow (intType base, intType exponent);
intType intPowOvfChk (intType base, intType exponent);
striType intRadix (intType number, intType base, boolType upperCase);
striType intRadixPow2 (intType number, int shift, int mask, boolType upperCase);
intType intRand (intType low, intType high);
intType intMultOvfChk (intType factor1, intType factor2);
intType intSqrt (intType number);
striType intStr (intType number);
#ifdef ALLOW_STRITYPE_SLICES
striType intStrToBuffer (intType number, striType buffer);
#endif
