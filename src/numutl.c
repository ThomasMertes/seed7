/********************************************************************/
/*                                                                  */
/*  numutl.c      Numeric utility functions.                        */
/*  Copyright (C) 1989 - 2020  Thomas Mertes                        */
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
/*  File: seed7/src/numutl.c                                        */
/*  Changes: 2014 - 2016, 2019 - 2020  Thomas Mertes                */
/*  Content: Numeric utility functions.                             */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "float.h"
#include "math.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "flt_rtl.h"
#include "str_rtl.h"
#include "rtl_err.h"
#include "big_drv.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "numutl.h"

#define MAX_DECIMAL_BUFFER_LENGTH 128

/* DECIMAL_WITH_LIMIT provides parameters for the format string \"%.*s%s\" */
#define DECIMAL_WITH_LIMIT(decimal, length) \
    (int) ((length) <= AND_SO_ON_LIMIT ? (length) : AND_SO_ON_LIMIT), \
    decimal, (length) > AND_SO_ON_LIMIT ? "\\ *AND_SO_ON* " : ""



/**
 *  Convert a bigInteger number to a double.
 *  @return the bigInteger as double
 */
double bigIntToDouble (const const_bigIntType number)

  {
    bigIntType absNumber;
    bigIntType mantissa;
    int64Type intMantissa;
    int sign;
    int exponent;
    double doubleValue = 0.0;

  /* bigIntToDouble */
    logFunction(printf("bigIntToDouble(%s)\n",
                       striAsUnquotedCStri(bigStr(number))););
    sign = (int) bigCmpSignedDigit(number, 0);
    if (sign != 0) {
      absNumber = bigAbs(number);
      if (absNumber != NULL) {
        exponent = (int) bigBitLength(absNumber) - 1;
        if (DOUBLE_MANTISSA_SHIFT + 1 >= exponent) {
          bigLShiftAssign(&absNumber, DOUBLE_MANTISSA_SHIFT - exponent + 1);
        } else {
          bigRShiftAssign(&absNumber, exponent - DOUBLE_MANTISSA_SHIFT - 1);
        } /* if */
        bigIncr(&absNumber);
        if (absNumber != NULL) {
          mantissa = bigRShift(absNumber, 1);
          if (mantissa != NULL) {
            intMantissa = bigToInt64(mantissa, NULL);
            bigDestr(mantissa);
            if (sign < 0) {
              intMantissa = -intMantissa;
            } /* if */
            doubleValue = setMantissaAndExponent(intMantissa, exponent - DOUBLE_MANTISSA_SHIFT);
          } /* if */
        } /* if */
      } /* if */
      bigDestr(absNumber);
    } /* if */
    logFunction(printf("bigIntToDouble --> " FMT_E "\n", doubleValue););
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



/**
 *  Convert a bigRational number to a double.
 *  @return the bigRational as double
 */
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
    logFunction(printf("bigRatToDouble(%s, ",
                       striAsUnquotedCStri(bigStr(numerator)));
                printf("%s)\n",
                       striAsUnquotedCStri(bigStr(denominator))););
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
            intMantissa = bigToInt64(mantissa, NULL);
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
    logFunction(printf("bigRatToDouble --> " FMT_E "\n", doubleValue););
    return doubleValue;
  } /* bigRatToDouble */



/**
 *  Convert a double to a bigRational number with numerator and denominator.
 *  @param denominator The address of a variable that receives the denominator.
 *  @return the nominator of the converted double.
 */
bigIntType doubleToBigRat (const double doubleValue, bigIntType *denominator)

  {
    int64Type intMantissa;
    int exponent;
    bigIntType numerator;

  /* doubleToBigRat */
    logFunction(printf("doubleToBigRat(" FMT_E ", *)\n",
                       doubleValue););
    if (os_isnan(doubleValue)) {
      numerator = bigFromInt32(0);
      *denominator = bigFromInt32(0);
    } else if (os_isinf(doubleValue)) {
      if (doubleValue < 0.0) {
        numerator = bigFromInt32(-1);
      } else {
        numerator = bigFromInt32(1);
      } /* if */
      *denominator = bigFromInt32(0);
    } else {
      intMantissa = getMantissaAndExponent(doubleValue, &exponent);
      /* printf("intMantissa: " FMT_D64 "\n", intMantissa);
      printf("exponent: %d\n", exponent); */
      numerator = bigFromInt64(intMantissa);
      if (exponent > 0) {
        bigLShiftAssign(&numerator, exponent);
        *denominator = bigFromInt32(1);
      } else {
        *denominator = bigLShiftOne(-exponent);
      } /* if */
    } /* if */
    logFunction(printf("doubleToBigRat --> %s",
                       striAsUnquotedCStri(bigStr(numerator)));
                printf(" (denominator = %s)\n",
                       striAsUnquotedCStri(bigStr(*denominator))););
    return numerator;
  } /* doubleToBigRat */



striType doubleToStri (const double doubleValue, boolType roundDouble)

  {
    char buffer[DOUBLE_TO_CHAR_BUFFER_SIZE];
    memSizeType len;
    striType result;

  /* doubleToStri */
    logFunction(printf("doubleToStri(" FMT_E ", %d)\n",
                       doubleValue, roundDouble););
    if (roundDouble) {
      len = doubleToCharBuffer(doubleValue, DOUBLE_STR_LARGE_NUMBER,
                               FMT_E_DBL, buffer);
    } else {
      len = doubleToCharBuffer(doubleValue, FLOAT_STR_LARGE_NUMBER,
                               FMT_E_FLT, buffer);
    } /* if */
    result = cstri_buf_to_stri(buffer, len);
    if (unlikely(result == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("doubleToStri --> \"%s\"\n",
                       striAsUnquotedCStri(result)););
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
    logFunction(printf("roundDoubleToBigRat(" FMT_E ", %d, *)\n",
                       doubleValue, roundDouble););
    if (os_isnan(doubleValue)) {
      numerator = bigFromInt32(0);
      *denominator = bigFromInt32(0);
    } else if (os_isinf(doubleValue)) {
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
        stri->size = savedSize;
        if (numerator != NULL) {
          if (scale == 0) {
            *denominator = bigFromInt32(1);
          } else {
            *denominator = bigIPowSignedDigit(10, (intType) scale);
          } /* if */
        } /* if */
        FREE_STRI(stri);
      } /* if */
    } /* if */
    logFunction(printf("roundDoubleToBigRat --> %s",
                       striAsUnquotedCStri(bigStr(numerator)));
                printf(" (denominator = %s)\n",
                       striAsUnquotedCStri(bigStr(*denominator))););
    return numerator;
  } /* roundDoubleToBigRat */



intType getDecimalInt (const const_ustriType decimal, memSizeType length)

  {
    boolType okay;
    boolType negative;
    memSizeType position = 0;
    uintType digitval;
    uintType uintValue;
    intType intResult;

  /* getDecimalInt */
    logFunction(printf("getDecimalInt(\"%.*s%s\", " FMT_U_MEM ")\n",
                       DECIMAL_WITH_LIMIT(decimal, length), length););
    if (likely(length != 0)) {
      if (decimal[0] == '-') {
        negative = TRUE;
        position++;
      } else {
        if (decimal[0] == '+') {
          position++;
        } /* if */
        negative = FALSE;
      } /* if */
    } /* if */
    if (unlikely(position >= length)) {
      logError(printf("getDecimalInt: Digit missing.\n"););
      raise_error(RANGE_ERROR);
      intResult = 0;
    } else {
      uintValue = 0;
      okay = TRUE;
#if TWOS_COMPLEMENT_INTTYPE
      while (position < length &&
             (digitval = ((uintType) decimal[position]) - ((uintType) '0')) <= 9) {
#else
      while (position < length &&
          decimal[position] >= '0' &&
          decimal[position] <= '9' && okay) {
        digitval = ((uintType) decimal[position]) - ((uintType) '0');
#endif
        if (unlikely(uintValue > MAX_DIV_10)) {
          okay = FALSE;
        } else {
          uintValue = ((uintType) 10) * uintValue + digitval;
        } /* if */
        position++;
      } /* while */
      if (unlikely(position < length)) {
        logError(printf("getDecimalInt: Illegal digit.\n"););
        raise_error(RANGE_ERROR);
        intResult = 0;
      } else if (unlikely(!okay)) {
        logError(printf("getDecimalInt: Absolute value of literal is too big.\n"););
        raise_error(RANGE_ERROR);
        intResult = 0;
      } else {
        if (negative) {
          if (unlikely(uintValue > (uintType) INTTYPE_MAX + 1)) {
            logError(printf("getDecimalInt: Literal too small.\n"););
            raise_error(RANGE_ERROR);
            intResult = 0;
          } else {
            /* The unsigned value is negated to avoid an overflow */
            /* if the most negative intType value is negated.     */
            intResult = (intType) -uintValue;
          } /* if */
        } else if (unlikely(uintValue > (uintType) INTTYPE_MAX)) {
          logError(printf("getDecimalInt: Literal too big.\n"););
          raise_error(RANGE_ERROR);
          intResult = 0;
        } else {
          intResult = (intType) uintValue;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("getDecimalInt --> " FMT_D "\n",
                       intResult););
    return intResult;
 } /* getDecimalInt */



bigIntType getDecimalBigRational (const const_ustriType decimal, memSizeType length,
    bigIntType *denominator)

  {
    striType stri;
    boolType okay = TRUE;
    boolType hasDecimalPoint = FALSE;
    memSizeType decimalPointPos = 0;
    memSizeType srcIndex;
    memSizeType destIndex = 0;
    memSizeType scale;
    bigIntType numerator;

  /* getDecimalBigRational */
    logFunction(printf("getDecimalBigRational(\"%.*s%s\", " FMT_U_MEM ")\n",
                       DECIMAL_WITH_LIMIT(decimal, length), length););
    if (unlikely(!ALLOC_STRI_CHECK_SIZE(stri, length))) {
      *denominator = NULL;
      raise_error(MEMORY_ERROR);
      numerator = NULL;
    } else {
      for (srcIndex = 0; srcIndex < length && okay; srcIndex++) {
        if ((decimal[srcIndex] >= '0' && decimal[srcIndex] <= '9') ||
            (decimal[srcIndex] == '-' && srcIndex == 0)) {
          stri->mem[destIndex] = decimal[srcIndex];
          destIndex++;
        } else if (decimal[srcIndex] == '.' || decimal[srcIndex] == ',') {
          /* Accept decimal point and decimal comma. */
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
        logError(printf("getDecimalBigRational: Decimal literal illegal.\n"););
        FREE_STRI2(stri, length);
        *denominator = NULL;
        raise_error(RANGE_ERROR);
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
        FREE_STRI2(stri, length);
        if (numerator != NULL) {
          if (unlikely(scale > INTTYPE_MAX)) {
            *denominator = NULL;
            bigDestr(numerator);
            logError(printf("getDecimalBigRational: scale > INTTYPE_MAX\n"
                            "scale = " FMT_U_MEM ", INTTYPE_MAX = " FMT_D "\n",
                            scale, INTTYPE_MAX););
            raise_error(RANGE_ERROR);
            numerator = NULL;
          } else {
            *denominator = bigIPowSignedDigit(10, (intType) scale);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("getDecimalBigRational --> %s",
                       striAsUnquotedCStri(bigStr(numerator)));
                printf(" (denominator = %s)\n",
                       striAsUnquotedCStri(bigStr(*denominator))););
    return numerator;
  } /* getDecimalBigRational */



floatType getDecimalFloat (const const_ustriType decimal, memSizeType length)

  {
    char localCharBuffer[MAX_DECIMAL_BUFFER_LENGTH + NULL_TERMINATION_LEN];
    char *charBuffer;
    char *commaPos;
    floatType floatValue;

  /* getDecimalFloat */
    logFunction(printf("getDecimalFloat(\"%.*s%s\", " FMT_U_MEM ")\n",
                       DECIMAL_WITH_LIMIT(decimal, length), length););
    if (length > MAX_DECIMAL_BUFFER_LENGTH) {
      charBuffer = (char *) malloc(length + NULL_TERMINATION_LEN);
    } else {
      charBuffer = localCharBuffer;
    } /* if */
    if (unlikely(charBuffer == NULL)) {
      raise_error(MEMORY_ERROR);
      floatValue = 0.0;
    } else {
      memcpy(charBuffer, decimal, length);
      charBuffer[length] = '\0';
      commaPos = strchr(charBuffer, ',');
      if (commaPos != NULL) {
        /* Accept decimal point and decimal comma. */
        *commaPos = '.';
      } /* if */
      floatValue = (floatType) strtod(charBuffer, NULL);
      if (charBuffer != localCharBuffer) {
        free(charBuffer);
      } /* if */
    } /* if */
    logFunction(printf("getDecimalFloat --> " FMT_E "\n",
                       floatValue););
    return floatValue;
  } /* getDecimalFloat */



ustriType bigIntToDecimal (const const_bigIntType bigIntValue,
    memSizeType *length, errInfoType *err_info)

  {
    striType stri;
    memSizeType idx;
    ustriType decimal;

  /* bigIntToDecimal */
    logFunction(printf("bigIntToDecimal(%s, *, *)\n",
                       striAsUnquotedCStri(bigStr(bigIntValue))););
    stri = bigStrDecimal(bigIntValue);
    if (unlikely(stri == NULL)) {
      *err_info = MEMORY_ERROR;
      decimal = NULL;
    } else if (unlikely(!ALLOC_USTRI(decimal, stri->size))) {
      FREE_STRI(stri);
      *err_info = MEMORY_ERROR;
    } else {
      for (idx = 0; idx < stri->size; idx++) {
        decimal[idx] = (ucharType) stri->mem[idx];
      } /* for */
      decimal[idx] = '\0';
      /* printf("decimal: %s\n", decimal); */
      *length = stri->size;
      FREE_STRI(stri);
    } /* if */
    logFunction(printf("bigIntToDecimal --> %s (length=" FMT_U_MEM ", err_info=%d)\n",
                       decimal == NULL ? "NULL" : (char *) decimal,
                       *length, *err_info););
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
    logFunction(printf("bigRatToDecimal(%s, ",
                       striAsUnquotedCStri(bigStr(numerator)));
                printf("%s, " FMT_U_MEM ", *, *)\n",
                       striAsUnquotedCStri(bigStr(denominator)),
                       scale););
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
      } else {
        if (bigEqSignedDigit(mantissaValue, 0)) {
          if (unlikely(!ALLOC_USTRI(decimal, 3))) {
            *err_info = MEMORY_ERROR;
          } else {
            memcpy(decimal, "0.0", 3);
            decimal[3] = '\0';
            *length = 3;
          } /* if */
        } else {
          stri = bigStrDecimal(mantissaValue);
          if (unlikely(stri == NULL)) {
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
            FREE_STRI(stri);
          } /* if */
        } /* if */
        bigDestr(mantissaValue);
      } /* if */
    } /* if */
    logFunction(printf("bigRatToDecimal --> %s (length=" FMT_U_MEM ", err_info=%d)\n",
                       decimal == NULL ? "NULL" : (char *) decimal,
                       *length, *err_info););
    return decimal;
  } /* bigRatToDecimal */
