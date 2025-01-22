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

#if FLOAT_ZERO_DIV_ERROR
extern const rtlValueUnion f_const[];
#define NOT_A_NUMBER      f_const[0].floatValue
#define POSITIVE_INFINITY f_const[1].floatValue
#define NEGATIVE_INFINITY f_const[2].floatValue
#else
#define NOT_A_NUMBER      ( 0.0 / 0.0)
#define POSITIVE_INFINITY ( 1.0 / 0.0)
#define NEGATIVE_INFINITY (-1.0 / 0.0)
#endif

#define DOUBLE_TO_CHAR_BUFFER_SIZE DOUBLE_MAX_EXP10 + 100
#define FMT_E_BUFFER_SIZE 2 + FMT_E_PRECISION + 2 + MAX_PRINTED_EXPONENT_DIGITS + NULL_TERMINATION_LEN


void setupFloat (void);
int64Type getMantissaAndExponent (double doubleValue, int *binaryExponent);
double setMantissaAndExponent (int64Type intMantissa, int binaryExponent);
memSizeType doubleToCharBuffer (const double doubleValue,
    const double largeNumber, const char *format, char *buffer);
memSizeType doubleToFormatE (const floatType number, char *buffer);
intType fltCmp (floatType number1, floatType number2);

#if !FREXP_FUNCTION_OKAY
floatType fltDecompose (const floatType number, intType *const exponent);
#endif

striType fltDgts (floatType number, intType precision);

#if !FLOAT_COMPARISON_OKAY
boolType fltEq (floatType number1, floatType number2);
#endif

#if EXP_FUNCTION_OKAY
#define fltExp(exponent) exp(exponent)
#else
floatType fltExp (floatType exponent);
#endif

#if HAS_EXPM1
#define fltExpM1(exponent) expm1(exponent)
#else
floatType fltExpM1 (floatType exponent);
#endif

#if !FLOAT_COMPARISON_OKAY
boolType fltGe (floatType number1, floatType number2);
boolType fltGt (floatType number1, floatType number2);
#endif

floatType fltIPow (floatType base, intType exponent);
boolType fltIsNegativeZero (floatType number);

#if LDEXP_FUNCTION_OKAY
#define fltLdexp(number, exponent) ldexp(number, exponent)
#else
floatType fltLdexp (floatType number, int exponent);
#endif

#if !FLOAT_COMPARISON_OKAY
boolType fltLe (floatType number1, floatType number2);
#endif

#if LOG_FUNCTION_OKAY
#define fltLog(number) log(number)
#else
floatType fltLog (floatType number);
#endif

#if LOG10_FUNCTION_OKAY
#define fltLog10(number) log10(number)
#else
floatType fltLog10 (floatType number);
#endif

#if LOG1P_FUNCTION_OKAY
#define fltLog1p(number) log1p(number)
#else
floatType fltLog1p (floatType number);
#endif

#if LOG2_FUNCTION_OKAY
#define fltLog2(number) log2(number)
#else
floatType fltLog2 (floatType number);
#endif

#if !FLOAT_COMPARISON_OKAY
boolType fltLt (floatType number1, floatType number2);
#endif

floatType fltMod (floatType dividend, floatType divisor);
floatType fltParse (const const_striType stri);

#if POW_FUNCTION_OKAY
#define fltPow(base, exponent) pow(base, exponent)
#else
floatType fltPow (floatType base, floatType exponent);
#endif

floatType fltRand (floatType low, floatType high);
floatType fltRandNoChk (floatType low, floatType high);

#if FMOD_FUNCTION_OKAY
#define fltRem(dividend, divisor) fmod(dividend, divisor)
#else
floatType fltRem (floatType dividend, floatType divisor);
#endif

striType fltSci (floatType number, intType precision);

#if SQRT_FUNCTION_OKAY
#define fltSqrt(number) sqrt(number)
#else
floatType fltSqrt (floatType number);
#endif

striType fltStr (floatType number);
striType fltStrScientific (floatType number);
