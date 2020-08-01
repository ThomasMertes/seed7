/********************************************************************/
/*                                                                  */
/*  flt_rtl.h     Primitive actions for the float type.             */
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
/*  File: seed7/src/flt_rtl.h                                       */
/*  Changes: 1993, 1994, 2005, 2010  Thomas Mertes                  */
/*  Content: Primitive actions for the float type.                  */
/*                                                                  */
/********************************************************************/

#ifdef FLOAT_ZERO_DIV_ERROR
extern const rtlValueUnion f_const[];
#define NOT_A_NUMBER      f_const[0].floatValue
#define POSITIVE_INFINITY f_const[1].floatValue
#define NEGATIVE_INFINITY f_const[2].floatValue
#else
#define NOT_A_NUMBER      ( 0.0 / 0.0)
#define POSITIVE_INFINITY ( 1.0 / 0.0)
#define NEGATIVE_INFINITY (-1.0 / 0.0)
#endif


void setupFloat (void);
int64Type getMantissaAndExponent (double doubleValue, int *binaryExponent);
double setMantissaAndExponent (int64Type intMantissa, int binaryExponent);
memSizeType doubleToCharBuffer (double doubleValue, double largeNumber,
                                const char *format, char *buffer);
intType fltCmp (floatType number1, floatType number2);
striType fltDgts (floatType number, intType precision);
#ifdef NAN_COMPARISON_WRONG
boolType fltEq (floatType number1, floatType number2);
boolType fltGe (floatType number1, floatType number2);
boolType fltGt (floatType number1, floatType number2);
#endif
floatType fltIPow (floatType base, intType exponent);
boolType fltIsNegativeZero (floatType number);
#ifdef NAN_COMPARISON_WRONG
boolType fltLe (floatType number1, floatType number2);
boolType fltLt (floatType number1, floatType number2);
#endif
floatType fltParse (const const_striType stri);
#ifdef POWER_OF_ZERO_WRONG
floatType fltPow (floatType base, floatType exponent);
#endif
floatType fltRand (floatType lower_limit, floatType upper_limit);
striType fltSci (floatType number, intType precision);
striType fltStr (floatType number);
