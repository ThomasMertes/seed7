/********************************************************************/
/*                                                                  */
/*  big_drv.h     Driver functions for the bigInteger type.         */
/*  Copyright (C) 1989 - 2014  Thomas Mertes                        */
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
/*  File: seed7/src/big_drv.h                                       */
/*  Changes: 2005, 2006, 2008 - 2010, 2013, 2014  Thomas Mertes     */
/*  Content: Driver functions for the bigInteger type.              */
/*                                                                  */
/********************************************************************/

#ifndef INT64TYPE
#define bigFromUInt64(x) 0
#define bigToInt64(x) 0
#endif

#if   INTTYPE_SIZE == 64
#define bigIConv bigFromInt64
#define bigOrd   bigToInt64
#elif INTTYPE_SIZE == 32
#define bigIConv bigFromInt32
#define bigOrd   bigToInt32
#endif


cstriType bigHexCStri (const const_bigIntType big1);
bigIntType bigAbs (const const_bigIntType big1);
bigIntType bigAdd (const_bigIntType summand1, const_bigIntType summand2);
bigIntType bigAddTemp (bigIntType summand1, const const_bigIntType summand2);
bigIntType bigAnd (const_bigIntType big1, const_bigIntType big2);
intType bigBitLength (const const_bigIntType big1);
intType bigCmp (const const_bigIntType big1, const const_bigIntType big2);
intType bigCmpSignedDigit (const const_bigIntType big1, intType number);
void bigCpy (bigIntType *const big_to, const const_bigIntType big_from);
bigIntType bigCreate (const const_bigIntType big_from);
void bigDecr (bigIntType *const big_variable);
void bigDestr (const const_bigIntType old_bigint);
bigIntType bigDiv (const const_bigIntType dividend, const const_bigIntType divisor);
boolType bigEq (const const_bigIntType big1, const const_bigIntType big2);
boolType bigEqSignedDigit (const const_bigIntType big1, intType number);
bigIntType bigFromByteBufferBe (const memSizeType size,
    const const_ustriType buffer, const boolType isSigned);
bigIntType bigFromByteBufferLe (const memSizeType size,
    const const_ustriType buffer, const boolType isSigned);
bigIntType bigFromBStriBe (const const_bstriType bstri, const boolType isSigned);
bigIntType bigFromBStriLe (const const_bstriType bstri, const boolType isSigned);
bigIntType bigFromInt32 (int32Type number);
#ifdef INT64TYPE
  bigIntType bigFromInt64 (int64Type number);
#endif
bigIntType bigFromUInt32 (uint32Type number);
#ifdef INT64TYPE
  bigIntType bigFromUInt64 (uint64Type number);
#endif
bigIntType bigGcd (const const_bigIntType big1, const const_bigIntType big2);
void bigGrow (bigIntType *const big_variable, const const_bigIntType delta);
void bigGrowSignedDigit (bigIntType *const big_variable, const intType delta);
intType bigHashCode (const const_bigIntType big1);
void bigIncr (bigIntType *const big_variable);
bigIntType bigIPow (const const_bigIntType base, intType exponent);
bigIntType bigIPowSignedDigit (intType base, intType exponent);
bigIntType bigLog10 (const const_bigIntType big1);
bigIntType bigLog2 (const const_bigIntType big1);
bigIntType bigLowerBits (const const_bigIntType big1, const intType bits);
bigIntType bigLowerBitsTemp (const bigIntType big1, const intType bits);
intType bigLowestSetBit (const const_bigIntType big1);
bigIntType bigLShift (const const_bigIntType big1, const intType lshift);
void bigLShiftAssign (bigIntType *const big_variable, intType lshift);
bigIntType bigLShiftOne (const intType lshift);
bigIntType bigLog2BaseIPow (const intType log2base, const intType exponent);
bigIntType bigMDiv (const const_bigIntType dividend, const const_bigIntType divisor);
bigIntType bigMod (const const_bigIntType dividend, const const_bigIntType divisor);
bigIntType bigMult (const_bigIntType factor1, const_bigIntType factor2);
void bigMultAssign (bigIntType *const big_variable, const_bigIntType factor);
bigIntType bigMultSignedDigit (const_bigIntType factor1, intType factor2);
bigIntType bigNegate (const const_bigIntType big1);
bigIntType bigNegateTemp (bigIntType big1);
boolType bigOdd (const const_bigIntType big1);
bigIntType bigOr (const_bigIntType big1, const_bigIntType big2);
bigIntType bigParse (const const_striType stri);
bigIntType bigParseBased (const const_striType stri, intType base);
bigIntType bigPred (const const_bigIntType big1);
bigIntType bigPredTemp (bigIntType big1);
striType bigRadix (const const_bigIntType big1, intType base, boolType upperCase);
bigIntType bigRand (const const_bigIntType low, const const_bigIntType high);
bigIntType bigRem (const const_bigIntType dividend, const const_bigIntType divisor);
bigIntType bigRShift (const const_bigIntType big1, const intType rshift);
void bigRShiftAssign (bigIntType *const big_variable, intType rshift);
bigIntType bigSbtr (const const_bigIntType minuend, const const_bigIntType subtrahend);
bigIntType bigSbtrTemp (bigIntType minuend, const_bigIntType subtrahend);
void bigShrink (bigIntType *const big_variable, const const_bigIntType delta);
bigIntType bigSquare (const_bigIntType big1);
striType bigStr (const const_bigIntType big1);
bigIntType bigSucc (const const_bigIntType big1);
bigIntType bigSuccTemp (bigIntType big1);
bstriType bigToBStriBe (const const_bigIntType big1, const boolType isSigned);
bstriType bigToBStriLe (const const_bigIntType big1, const boolType isSigned);
int16Type bigToInt16 (const const_bigIntType big1);
int32Type bigToInt32 (const const_bigIntType big1);
#ifdef INT64TYPE
  int64Type bigToInt64 (const const_bigIntType big1);
#endif
bigIntType bigXor (const_bigIntType big1, const_bigIntType big2);
bigIntType bigZero (void);
