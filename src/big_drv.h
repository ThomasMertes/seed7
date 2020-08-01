/********************************************************************/
/*                                                                  */
/*  big_drv.h     Driver functions for the bigInteger type.         */
/*  Copyright (C) 1989 - 2008  Thomas Mertes                        */
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
/*  Changes: 2005, 2006, 2008  Thomas Mertes                        */
/*  Content: Driver functions for the bigInteger type.              */
/*                                                                  */
/********************************************************************/

#ifndef HAS_INT64TYPE
#define bigFromUInt64(x) 0
#define bigToInt64(x) 0
#endif


#ifdef ANSI_C

cstritype bigHexCStri (const_biginttype big1);
biginttype bigAbs (const const_biginttype);
biginttype bigAdd (const_biginttype, const_biginttype);
biginttype bigAddTemp (biginttype, const_biginttype);
inttype bigBitLength (const const_biginttype);
stritype bigCLit (const const_biginttype);
inttype bigCmp (const const_biginttype, const const_biginttype);
inttype bigCmpSignedDigit (const const_biginttype big1, inttype number);
void bigCpy (biginttype *const, const const_biginttype);
biginttype bigCreate (const const_biginttype);
void bigDecr (biginttype *const);
void bigDestr (const biginttype);
biginttype bigDiv (const const_biginttype, const const_biginttype);
booltype bigEq (const const_biginttype, const const_biginttype);
#ifdef HAS_INT64TYPE
  biginttype bigFromInt64 (int64type);
#endif
biginttype bigFromUInt32 (uinttype);
#ifdef HAS_INT64TYPE
  biginttype bigFromUInt64 (uint64type);
#endif
biginttype bigGcd (const const_biginttype, const const_biginttype);
void bigGrow (biginttype *const, const const_biginttype);
inttype bigHashCode (const const_biginttype);
biginttype bigIConv (inttype);
void bigIncr (biginttype *const);
biginttype bigIPow (const const_biginttype, inttype);
biginttype bigLog2 (const const_biginttype);
inttype bigLowestSetBit (const const_biginttype);
biginttype bigLShift (const const_biginttype big1, const inttype lshift);
void bigLShiftAssign (biginttype *const big_variable, inttype lshift);
biginttype bigMDiv (const const_biginttype, const const_biginttype);
biginttype bigMinus (const const_biginttype);
biginttype bigMod (const const_biginttype, const const_biginttype);
biginttype bigMult (const_biginttype, const_biginttype);
void bigMultAssign (biginttype *const, const_biginttype);
booltype bigNe (const const_biginttype, const const_biginttype);
booltype bigOdd (const const_biginttype big1);
inttype bigOrd (const const_biginttype);
biginttype bigParse (const const_stritype);
biginttype bigPred (const const_biginttype);
biginttype bigRand (const const_biginttype, const const_biginttype);
biginttype bigRem (const const_biginttype, const const_biginttype);
biginttype bigRShift (const const_biginttype big1, const inttype rshift);
void bigRShiftAssign (biginttype *const big_variable, inttype rshift);
biginttype bigSbtr (const const_biginttype, const const_biginttype);
biginttype bigSbtrTemp (biginttype, const_biginttype);
void bigShrink (biginttype *const, const const_biginttype);
stritype bigStr (const const_biginttype);
biginttype bigSucc (const const_biginttype);
int32type bigToInt32 (const const_biginttype big1);
#ifdef HAS_INT64TYPE
  int64type bigToInt64 (const const_biginttype big1);
#endif

#else

cstritype bigHexCStri ();
biginttype bigAbs ();
biginttype bigAdd ();
biginttype bigAddTemp ();
inttype bigBitLength ();
stritype bigCLit ();
inttype bigCmp ();
inttype bigCmpSignedDigit ();
void bigCpy ();
biginttype bigCreate ();
void bigDecr ();
void bigDestr ();
biginttype bigDiv ();
booltype bigEq ();
#ifdef HAS_INT64TYPE
  biginttype bigFromInt64 ();
#endif
biginttype bigFromUInt32 ();
#ifdef HAS_INT64TYPE
  biginttype bigFromUInt64 ();
#endif
biginttype bigGcd ();
void bigGrow ();
inttype bigHashCode ();
biginttype bigIConv ();
void bigIncr ();
biginttype bigIPow ();
biginttype bigLog2 ();
inttype bigLowestSetBit ();
biginttype bigLShift ();
void bigLShiftAssign ();
biginttype bigMDiv ();
biginttype bigMinus ();
biginttype bigMod ();
biginttype bigMult ();
void bigMultAssign ();
booltype bigNe ();
booltype bigOdd ();
inttype bigOrd ();
biginttype bigParse ();
biginttype bigPred ();
biginttype bigRand ();
biginttype bigRem ();
biginttype bigRShift ();
void bigRShiftAssign ();
biginttype bigSbtr ();
biginttype bigSbtrTemp ();
void bigShrink ();
stritype bigStr ();
biginttype bigSucc ();
int32type bigToInt32 ();
#ifdef HAS_INT64TYPE
  int64type bigToInt64 ();
#endif

#endif
