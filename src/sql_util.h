/********************************************************************/
/*                                                                  */
/*  sql_util.h    Database utility functions.                       */
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
/*  File: seed7/src/sql_util.h                                      */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: Database utility functions.                            */
/*                                                                  */
/********************************************************************/

double bigIntToDouble (const const_bigIntType number);
double bigRatToDouble (const const_bigIntType numerator,
                       const const_bigIntType denominator);
bigIntType doubleToBigRat (const double doubleValue, bigIntType *denominator);
striType doubleToStri (const double doubleValue, boolType roundDouble);
bigIntType roundDoubleToBigRat (const double doubleValue, boolType roundDouble,
                                bigIntType *denominator);
intType getDecimalInt (const void *buffer, memSizeType length);
bigIntType getDecimalBigInt (const void *buffer, memSizeType length);
bigIntType getDecimalBigRational (const void *buffer, memSizeType length,
                                  bigIntType *denominator);
floatType getDecimalFloat (const void *buffer, memSizeType length);
ustriType bigIntToDecimal (const const_bigIntType bigIntValue,
                           memSizeType *length, errInfoType *err_info);
ustriType bigRatToDecimal (const const_bigIntType numerator,
                           const const_bigIntType denominator, memSizeType scale,
                           memSizeType *length, errInfoType *err_info);
