/********************************************************************/
/*                                                                  */
/*  sql_util.c    Database utility functions.                       */
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
/*  File: seed7/src/sql_util.c                                      */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: Database utility functions.                            */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "float.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "int_rtl.h"
#include "flt_rtl.h"
#include "str_rtl.h"
#include "rtl_err.h"
#include "big_drv.h"

#define MAX_DECIMAL_BUFFER_LENGTH 128



double bigIntToDouble (const const_bigIntType number)

  {
    bigIntType absNumber;
    bigIntType two;
    bigIntType mantissa;
    int64Type intMantissa;
    int exponent;
    double doubleValue = 0.0;

  /* bigIntToDouble */
    /* printf("bigIntToDouble(");
       prot_stri_unquoted(bigStr(number));
       printf(")\n"); */
    if (!bigEqSignedDigit(number, 0)) {
      absNumber = bigAbs(number);
      two = bigFromInt32(2);
      if (absNumber != NULL && two != NULL) {
        exponent = (int) bigBitLength(absNumber) - 1;
        if (DOUBLE_MANTISSA_SHIFT + 1 >= exponent) {
          bigLShiftAssign(&absNumber, DOUBLE_MANTISSA_SHIFT - exponent + 1);
        } else {
          bigRShiftAssign(&absNumber, exponent - DOUBLE_MANTISSA_SHIFT - 1);
        } /* if */
        bigIncr(&absNumber);
        if (absNumber != NULL) {
          mantissa = bigDiv(absNumber, two);
          if (mantissa != NULL) {
            intMantissa = bigToInt64(mantissa);
            bigDestr(mantissa);
            if (bigCmpSignedDigit(number, 0) < 0) {
              intMantissa = -intMantissa;
            } /* if */
            doubleValue = setMantissaAndExponent(intMantissa, exponent - DOUBLE_MANTISSA_SHIFT);
          } /* if */
        } /* if */
      } /* if */
      bigDestr(absNumber);
      bigDestr(two);
    } /* if */
    /* printf("bigIntToDouble -> %f\n", doubleValue); */
    return doubleValue;
  } /* bigIntToDouble */



static int binaryExponent (const const_bigIntType absNumerator,
    const const_bigIntType denominator)

  {
    bigIntType quotient;
    int exponent = 0;

  /* binaryExponent */
    if (bigCmp(absNumerator, denominator) >= 0) {
      quotient = bigDiv(absNumerator, denominator);
      if (quotient != NULL) {
        exponent = (int) bigBitLength(quotient) - 1;
        bigDestr(quotient);
      } /* if */
    } else {
      quotient = bigDiv(denominator, absNumerator);
      if (quotient != NULL) {
        exponent = (int) -bigBitLength(quotient);
        bigDestr(quotient);
      } /* if */
    } /* if */
    return exponent;
  } /* binaryExponent */



double bigRatToDouble (const const_bigIntType numerator,
    const const_bigIntType denominator)

  {
    bigIntType absNumerator;
    bigIntType shiftedDenominator;
    bigIntType mantissa;
    int64Type intMantissa;
    int exponent;
    double doubleValue = 0.0;

  /* bigRatToDouble */
    /* printf("bigRatToDouble(");
       prot_stri_unquoted(bigStr(numerator));
       printf(", ");
       prot_stri_unquoted(bigStr(denominator));
       printf(")\n"); */
    if (bigEqSignedDigit(denominator, 0)) {
      if (bigCmpSignedDigit(numerator, 0) > 0) {
        doubleValue = POSITIVE_INFINITY;
      } else if (bigEqSignedDigit(numerator, 0)) {
        doubleValue = NOT_A_NUMBER;
      } else {
        doubleValue = NEGATIVE_INFINITY;
      } /* if */
    } else if (!bigEqSignedDigit(numerator, 0)) {
      absNumerator = bigAbs(numerator);
      shiftedDenominator = bigLShift(denominator, 1);
      if (absNumerator != NULL && shiftedDenominator != NULL) {
        exponent = binaryExponent(absNumerator, denominator);
        if (DOUBLE_MANTISSA_SHIFT + 1 >= exponent) {
          bigLShiftAssign(&absNumerator, DOUBLE_MANTISSA_SHIFT - exponent + 1);
        } else {
          bigRShiftAssign(&absNumerator, exponent - DOUBLE_MANTISSA_SHIFT - 1);
        } /* if */
        bigAddAssign(&absNumerator, denominator);
        if (absNumerator != NULL) {
          mantissa = bigDiv(absNumerator, shiftedDenominator);
          if (mantissa != NULL) {
            intMantissa = bigToInt64(mantissa);
            bigDestr(mantissa);
            if (bigCmpSignedDigit(numerator, 0) < 0) {
              intMantissa = -intMantissa;
            } /* if */
            doubleValue = setMantissaAndExponent(intMantissa, exponent - DOUBLE_MANTISSA_SHIFT);
          } /* if */
        } /* if */
      } /* if */
      bigDestr(absNumerator);
      bigDestr(shiftedDenominator);
    } /* if */
    /* printf("bigRatToDouble -> %f\n", doubleValue); */
    return doubleValue;
  } /* bigRatToDouble */



bigIntType doubleToBigRat (const double doubleValue, bigIntType *denominator)

  {
    int64Type intMantissa;
    int exponent;
    bigIntType numerator;

  /* doubleToBigRat */
    /* printf("doubleToBigRat(%f)\n", doubleValue); */
    if (isnan(doubleValue)) {
      numerator = bigFromInt32(0);
      *denominator = bigFromInt32(0);
    } else if (isinf(doubleValue)) {
      if (doubleValue < 0.0) {
        numerator = bigFromInt32(-1);
      } else {
        numerator = bigFromInt32(1);
      } /* if */
      *denominator = bigFromInt32(0);
    } else {
      intMantissa = getMantissaAndExponent(doubleValue, &exponent);
      /* printf("intMantissa: %ld\n", intMantissa);
      printf("exponent: %d\n", exponent); */
      numerator = bigFromInt64(intMantissa);
      if (exponent > 0) {
        bigLShiftAssign(&numerator, exponent);
        *denominator = bigFromInt32(1);
      } else {
        *denominator = bigLShiftOne(-exponent);
      } /* if */
    } /* if */
    /* printf("doubleToBigRat -> ");
       prot_stri_unquoted(bigStr(numerator));
       printf(" / ");
       prot_stri_unquoted(bigStr(*denominator));
       printf("\n"); */
    return numerator;
  } /* doubleToBigRat */



striType doubleToStri (const double doubleValue, boolType roundDouble)

  {
    char buffer[1024];
    memSizeType len;
    striType result;

  /* doubleToStri */
    if (roundDouble) {
      len = doubleToCharBuffer(doubleValue, DOUBLE_STR_LARGE_NUMBER,
                               DOUBLE_STR_FORMAT, buffer);
    } else {
      len = doubleToCharBuffer(doubleValue, FLOAT_STR_LARGE_NUMBER,
                               FLOAT_STR_FORMAT, buffer);
    } /* if */
    result = cstri_buf_to_stri(buffer, len);
    if (unlikely(result == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    /* printf("doubleToStri --> ");
       prot_stri(result);
       printf("\n"); */
    return result;
  } /* doubleToStri */



bigIntType roundDoubleToBigRat (const double doubleValue, boolType roundDouble,
    bigIntType *denominator)

  {
    striType stri;
    memSizeType decimalPointPos;
    memSizeType scale;
    memSizeType savedSize;
    bigIntType numerator;

  /* roundDoubleToBigRat */
    /* printf("roundDoubleToBigRat(%f, %d)\n", doubleValue, roundDouble); */
    if (isnan(doubleValue)) {
      numerator = bigFromInt32(0);
      *denominator = bigFromInt32(0);
    } else if (isinf(doubleValue)) {
      if (doubleValue < 0.0) {
        numerator = bigFromInt32(-1);
      } else {
        numerator = bigFromInt32(1);
      } /* if */
      *denominator = bigFromInt32(0);
    } else {
      stri = doubleToStri(doubleValue, roundDouble);
      if (stri == NULL) {
        numerator = NULL;
      } else {
        decimalPointPos = (memSizeType) strChPos(stri, '.');
        scale = stri->size - decimalPointPos;
        memmove(&stri->mem[decimalPointPos - 1],
                &stri->mem[decimalPointPos],
                scale * sizeof(strElemType));
        savedSize = stri->size;
        stri->size--;
        while (scale >= 1 && stri->mem[stri->size - 1] == '0') {
          scale--;
          stri->size--;
        } /* while */
        numerator = bigParse(stri);
        if (numerator != NULL) {
          if (scale == 0) {
            *denominator = bigFromInt32(1);
          } else {
            *denominator = bigIPowSignedDigit(10, (intType) scale);
          } /* if */
        } /* if */
        FREE_STRI(stri, savedSize);
      } /* if */
    } /* if */
    /* printf("roundDoubleToBigRat -> ");
       prot_stri_unquoted(bigStr(numerator));
       printf(" / ");
       prot_stri_unquoted(bigStr(*denominator));
       printf("\n"); */
    return numerator;
  } /* roundDoubleToBigRat */



intType getDecimalInt (const void *buffer, memSizeType length)

  {
    union {
      struct striStruct striBuf;
      char charBuf[SIZ_STRI(INTTYPE_DECIMAL_SIZE)];
    } striBuffer;
    intType intValue;

  /* getDecimalInt */
#ifdef ALLOW_STRITYPE_SLICES
    striBuffer.striBuf.mem = striBuffer.striBuf.mem1;
#endif
    if (unlikely(length > INTTYPE_DECIMAL_SIZE)) {
      raise_error(RANGE_ERROR);
      intValue = 0;
    } else {
      striBuffer.striBuf.size = length;
      memcpy_to_strelem(striBuffer.striBuf.mem, (const_ustriType) buffer, length);
      /* printf("getDecimalInt: stri: ");
         prot_stri(&striBuffer.striBuf);
         printf("\n"); */
      intValue = intParse(&striBuffer.striBuf);
    } /* if */
    return intValue;
 } /* getDecimalInt */



bigIntType getDecimalBigInt (const void *buffer, memSizeType length)

  {
    striType stri;
    bigIntType bigIntValue;

  /* getDecimalBigInt */
    stri = cstri_buf_to_stri((const_cstriType) buffer, length);
    /* printf("getDecimalBigInt: stri: ");
       prot_stri(stri);
       printf("\n"); */
    if (unlikely(stri == NULL)) {
      raise_error(MEMORY_ERROR);
      bigIntValue = NULL;
    } else {
      bigIntValue = bigParse(stri);
      strDestr(stri);
    } /* if */
    /* printf("getDecimalBigInt --> ");
       if (bigIntValue == NULL) {
         printf("NULL");
       } else {
         prot_stri_unquoted(bigStr(bigIntValue));
       }
       printf("\n"); */
    return bigIntValue;
  } /* getDecimalBigInt */



bigIntType getDecimalBigRational (const void *buffer, memSizeType length,
    bigIntType *denominator)

  {
    striType stri;
    unsigned char *decimal;
    boolType okay = TRUE;
    boolType hasDecimalPoint = FALSE;
    memSizeType decimalPointPos = 0;
    memSizeType srcIndex;
    memSizeType destIndex = 0;
    memSizeType scale;
    bigIntType numerator;

  /* getDecimalBigRational */
    if (unlikely(!ALLOC_STRI_CHECK_SIZE(stri, length))) {
      *denominator = NULL;
      raise_error(MEMORY_ERROR);
      numerator = NULL;
    } else {
      decimal = (unsigned char *) buffer;
      for (srcIndex = 0; srcIndex < length && okay; srcIndex++) {
        if ((decimal[srcIndex] >= '0' && decimal[srcIndex] <= '9') ||
            (decimal[srcIndex] == '-' && srcIndex == 0)) {
          stri->mem[destIndex] = decimal[srcIndex];
          destIndex++;
        } else if (decimal[srcIndex] == '.') {
          if (hasDecimalPoint) {
            okay = FALSE;
          } else {
            decimalPointPos = destIndex;
            hasDecimalPoint = TRUE;
          } /* if */
        } else {
          okay = FALSE;
        } /* if */
      } /* for */
      stri->size = (memSizeType) destIndex;
      /* prot_stri(stri);
         printf("\n"); */
      if (unlikely(!okay)) {
        *denominator = NULL;
        raise_error(FILE_ERROR);
        numerator = NULL;
      } else {
        /* printf("decimalPointPos: " FMT_U_MEM "\n", decimalPointPos); */
        if (hasDecimalPoint) {
          scale = stri->size - decimalPointPos;
        } else {
          scale = 0;
        } /* if */
        /* printf("scale: " FMT_U_MEM "\n", scale); */
        numerator = bigParse(stri);
        if (numerator != NULL) {
          if (unlikely(scale > INTTYPE_MAX)) {
            *denominator = NULL;
            bigDestr(numerator);
            raise_error(RANGE_ERROR);
            numerator = NULL;
          } else {
            *denominator = bigIPowSignedDigit(10, (intType) scale);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return numerator;
  } /* getDecimalBigRational */



floatType getDecimalFloat (const void *buffer, memSizeType length)

  {
    char localCharBuffer[MAX_DECIMAL_BUFFER_LENGTH + 1];
    char *charBuffer;
    double doubleValue;

  /* getDecimalFloat */
    if (length > MAX_DECIMAL_BUFFER_LENGTH) {
      charBuffer = (char *) malloc(length + 1);
      if (unlikely(charBuffer == NULL)) {
        raise_error(MEMORY_ERROR);
        doubleValue = 0.0;
      } else {
        memcpy(charBuffer, (char *) buffer, length);
        charBuffer[length] = '\0';
        sscanf(charBuffer, "%lf", &doubleValue);
        free(charBuffer);
      } /* if */
    } else {
      memcpy(localCharBuffer, (char *) buffer, length);
      localCharBuffer[length] = '\0';
      sscanf(localCharBuffer, "%lf", &doubleValue);
    } /* if */
    return (floatType) doubleValue;
  } /* getDecimalFloat */



ustriType bigIntToDecimal (const const_bigIntType bigIntValue,
    memSizeType *length, errInfoType *err_info)

  {
    striType stri;
    memSizeType idx;
    ustriType decimal;

  /* bigIntToDecimal */
    /* printf("bigIntToDecimal(");
       prot_stri(bigStr(bigIntValue));
       printf(")\n"); */
    stri = bigStr(bigIntValue);
    if (stri == NULL) {
      *err_info = MEMORY_ERROR;
      decimal = NULL;
    } else if (unlikely(!ALLOC_USTRI(decimal, stri->size))) {
      FREE_STRI(stri, stri->size);
      *err_info = MEMORY_ERROR;
    } else {
      for (idx = 0; idx < stri->size; idx++) {
        decimal[idx] = (ucharType) stri->mem[idx];
      } /* for */
      decimal[idx] = '\0';
      /* printf("decimal: %s\n", decimal); */
      *length = stri->size;
      FREE_STRI(stri, stri->size);
    } /* if */
    return decimal;
  } /* bigIntToDecimal */



ustriType bigRatToDecimal (const const_bigIntType numerator,
    const const_bigIntType denominator, memSizeType scale,
    memSizeType *length, errInfoType *err_info)

  {
    bigIntType number;
    bigIntType mantissaValue;
    striType stri;
    memSizeType striSizeUsed;
    memSizeType srcIndex;
    memSizeType destIndex;
    memSizeType decimalSize;
    ustriType decimal;

  /* bigRatToDecimal */
    /* printf("bigRatToDecimal(");
       prot_stri_unquoted(bigStr(numerator));
       printf(", ");
       prot_stri_unquoted(bigStr(denominator));
       printf(", " FMT_U_MEM ")\n", scale); */
    if (bigEqSignedDigit(denominator, 0)) {
      /* Decimal values do not support Infinity and NaN. */
      /* printf("Decimal values do not support Infinity and NaN.\n"); */
      *err_info = RANGE_ERROR;
      decimal = NULL;
    } else if (scale > INTTYPE_MAX) {
      *err_info = RANGE_ERROR;
      decimal = NULL;
    } else {
      number = bigIPowSignedDigit(10, (intType) scale);
      if (unlikely(number == NULL)) {
        mantissaValue = NULL;
      } else {
        bigMultAssign(&number, numerator);
        mantissaValue = bigDiv(number, denominator);
        bigDestr(number);
      } /* if */
      if (unlikely(mantissaValue == NULL)) {
        decimal = NULL;
      } else if (bigEqSignedDigit(mantissaValue, 0)) {
        if (unlikely(!ALLOC_USTRI(decimal, 3))) {
          *err_info = MEMORY_ERROR;
        } else {
          strcpy((char *) decimal, "0.0");
          *length = 3;
        } /* if */
      } else {
        stri = bigStr(mantissaValue);
        if (stri == NULL) {
          *err_info = MEMORY_ERROR;
          decimal = NULL;
        } else {
          /* prot_stri(stri);
             printf("\n"); */
          striSizeUsed = stri->size;
          while (scale >= 2 && stri->mem[striSizeUsed - 1] == '0') {
            scale--;
            striSizeUsed--;
          } /* while */
          decimalSize = striSizeUsed;
          if (decimalSize - (stri->mem[0] == '-') > scale) {
            /* Add one character for the decimal point. */
            decimalSize += 1;
          } else {
            /* Add space for sign, zero and decimal point. */
            decimalSize = (stri->mem[0] == '-') + scale + 2;
          } /* if */
          if (unlikely(!ALLOC_USTRI(decimal, decimalSize))) {
            FREE_STRI(stri, stri->size);
            *err_info = MEMORY_ERROR;
          } else {
            srcIndex = 0;
            destIndex = 0;
            if (stri->mem[0] == '-') {
              decimal[0] = '-';
              srcIndex++;
              destIndex++;
            } /* if */
            if (striSizeUsed - srcIndex > scale) {
              for (; srcIndex < striSizeUsed - scale; srcIndex++) {
                decimal[destIndex] = (unsigned char) stri->mem[srcIndex];
                destIndex++;
              } /* for */
              decimal[destIndex] = '.';
              destIndex++;
            } else {
              decimal[destIndex] = '0';
              destIndex++;
              decimal[destIndex] = '.';
              destIndex++;
              memset(&decimal[destIndex], '0', scale - striSizeUsed + srcIndex);
              destIndex += scale - striSizeUsed + srcIndex;
            } /* if */
            for (; srcIndex < striSizeUsed; srcIndex++) {
              decimal[destIndex] = (unsigned char) stri->mem[srcIndex];
              destIndex++;
            } /* for */
            decimal[destIndex] = '\0';
            *length = destIndex;
          } /* if */
          FREE_STRI(stri, stri->size);
        } /* if */
        bigDestr(mantissaValue);
      } /* if */
    } /* if */
    /* printf("bigRatToDecimal --> %s\n", decimal == NULL ? "NULL" : (char *) decimal); */
    return decimal;
  } /* bigRatToDecimal */
