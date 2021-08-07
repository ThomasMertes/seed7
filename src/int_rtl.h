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

#if INTTYPE_SIZE == 32
#define RAND_MULTIPLIER           1103515245
#define RAND_INCREMENT                 12345
#elif INTTYPE_SIZE == 64
#define RAND_MULTIPLIER  6364136223846793005
#define RAND_INCREMENT   1442695040888963407
#endif

extern const const_ustriType digitTable[];


void setupRand (void);
uintType uintMult (uintType factor1, uintType factor2, uintType *product_high);
striType uintNBytesBe (uintType number, intType length);
striType uintNBytesLe (uintType number, intType length);
uintType uintRand (void);
uintType uintRandMantissa (void);
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
intType uintCard (uintType number);
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
striType intLpad0 (intType number, const intType padSize);
intType intMultOvfChk (intType factor1, intType factor2);
striType intNBytesBeSigned (intType number, intType length);
striType intNBytesBeUnsigned (intType number, intType length);
striType intNBytesLeSigned (intType number, intType length);
striType intNBytesLeUnsigned (intType number, intType length);
intType intParse (const const_striType stri);
intType intPow (intType base, intType exponent);
intType intPowOvfChk (intType base, intType exponent);
striType intRadix (intType number, intType base, boolType upperCase);
striType intRadixPow2 (intType number, int shift, int mask, boolType upperCase);
intType intRand (intType low, intType high);
intType intSqrt (intType number);
striType intStr (intType number);
#if ALLOW_STRITYPE_SLICES
striType intStrToBuffer (intType number, striType buffer);
#endif
