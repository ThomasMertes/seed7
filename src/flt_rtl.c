/********************************************************************/
/*                                                                  */
/*  flt_rtl.c     Primitive actions for the float type.             */
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
/*  File: seed7/src/flt_rtl.c                                       */
/*  Changes: 1993, 1994, 2005, 2010  Thomas Mertes                  */
/*  Content: Primitive actions for the float type.                  */
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
#include "rtl_err.h"
#include "int_rtl.h"

#undef EXTERN
#define EXTERN
#include "flt_rtl.h"


#define USE_STRTOD
#define MAX_CSTRI_BUFFER_LEN 25
#define IPOW_EXPONENTIATION_BY_SQUARING

#ifdef FLOAT_ZERO_DIV_ERROR
const rtlValueUnion f_const[] =
#ifdef FLOATTYPE_DOUBLE
    {{0xfff8000000000000}, {0x7ff0000000000000}, {0xfff0000000000000}};
#else
    {{0xffc00000}, {0x7f800000}, {0xff800000}};
#endif
#endif

#ifdef USE_NEGATIVE_ZERO_BITPATTERN
static const rtlValueUnion neg_zero_const =
#ifdef FLOATTYPE_DOUBLE
    {0x8000000000000000};
#else
    {0x80000000};
#endif
#endif

floatType negativeZero;

#ifndef USE_VARIABLE_FORMATS
#define MAX_FORM 28

static char *form[] = {"%1.0f", "%1.1f", "%1.2f", "%1.3f", "%1.4f",
    "%1.5f",  "%1.6f",  "%1.7f",  "%1.8f",  "%1.9f",  "%1.10f",
    "%1.11f", "%1.12f", "%1.13f", "%1.14f", "%1.15f", "%1.16f",
    "%1.17f", "%1.18f", "%1.19f", "%1.20f", "%1.21f", "%1.22f",
    "%1.23f", "%1.24f", "%1.25f", "%1.26f", "%1.27f", "%1.28f"};
#endif



void setupFloat (void)

  {
#ifndef USE_NEGATIVE_ZERO_BITPATTERN
    floatType zero = 0.0;
    floatType plusInf;
#endif

  /* setupFloat */
#ifdef TURN_OFF_FP_EXCEPTIONS
    _control87(MCW_EM, MCW_EM);
#endif
#ifdef USE_NEGATIVE_ZERO_BITPATTERN
    negativeZero = neg_zero_const.floatValue;
#else
    plusInf = 1.0 / zero;
    negativeZero = -1.0 / plusInf;
#endif
  } /* setupFloat */



#ifdef DEFINE_MATHERR_FUNCTION
int _matherr (struct _exception *a)

  { /* _matherr */
    a->retval = a->retval;
    return 1;
  } /* _matherr */
#endif



int64Type getMantissaAndExponent (double doubleValue, int *binaryExponent)

  {
    double mantissa;
    int exponent;
    int64Type intMantissa;

  /* getMantissaAndExponent */
    mantissa = frexp(doubleValue, &exponent);
    intMantissa = (int64Type) (mantissa * DOUBLE_MANTISSA_FACTOR);
    *binaryExponent = exponent - DOUBLE_MANTISSA_SHIFT;
    return intMantissa;
  } /* getMantissaAndExponent */



double setMantissaAndExponent (int64Type intMantissa, int binaryExponent)

  {
    double mantissa;
    int exponent;
    double doubleValue;

  /* setMantissaAndExponent */
    mantissa = (double) intMantissa / DOUBLE_MANTISSA_FACTOR;
    exponent = binaryExponent + DOUBLE_MANTISSA_SHIFT;
    doubleValue = ldexp(mantissa, exponent);
    return doubleValue;
  } /* setMantissaAndExponent */



memSizeType doubleToCharBuffer (double doubleValue, double largeNumber,
    const char *format, char *buffer)

  {
    long decimalExponent;
    memSizeType start;
    memSizeType scale;
    memSizeType len;

  /* doubleToCharBuffer */
    if (doubleValue == 0.0) {
      memcpy(buffer, "0.0", 3);
      len = 3;
    } else if (doubleValue < -largeNumber || doubleValue > largeNumber) {
      sprintf(buffer, "%1.1f", doubleValue);
      len = strlen(buffer);
    } else {
      sprintf(buffer, format, doubleValue);
      /* printf("buffer: \"%s\"\n", buffer); */
      len = strlen(buffer);
      decimalExponent = strtol(&buffer[len - DOUBLE_DECIMAL_EXPONENT_DIGITS - 1], NULL, 10);
      /* printf("decimalExponent: %ld\n", decimalExponent); */
      len -= DOUBLE_DECIMAL_EXPONENT_DIGITS + 2;
      do {
        len--;
      } while (buffer[len] == '0');
      len++;
      /* printf("len: %lu\n", len); */
      start = buffer[0] == '-';
      if (decimalExponent > 0) {
        scale = (memSizeType) decimalExponent;
        if (scale >= len - start - 2) {
          memmove(&buffer[start + 1], &buffer[start + 2], len - start - 2);
          memset(&buffer[len - 1], '0', scale + 2 - len + start);
          memcpy(&buffer[start + scale + 1], ".0", 2);
          len = start + scale + 3;
        } else {
          memmove(&buffer[start + 1], &buffer[start + 2], scale);
          buffer[start + scale + 1] = '.';
        } /* if */
      } else if (decimalExponent < 0) {
        scale = (memSizeType) -decimalExponent;
        memmove(&buffer[start + 2 + scale], &buffer[start + 2], len - start - 2);
        buffer[start + 1 + scale] = buffer[start];
        memset(&buffer[start + 2], '0', scale - 1);
        memcpy(&buffer[start], "0.", 2);
        len += scale;
      } else if (buffer[len - 1] == '.') {
        len++;
      } /* if */
      /* printf("len: %lu\n", len);
         buffer[len] = '\0';
         printf("buffer: \"%s\"\n", buffer); */
    } /* if */
    return len;
  } /* doubleToCharBuffer */



/**
 *  Compare two float numbers.
 *  Because fltCmp is used to sort float values, a total
 *  order of all float values is needed. Therefore fltCmp
 *  considers NaN as equal to itself and greater than
 *  Infinity.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType fltCmp (floatType number1, floatType number2)

  { /* fltCmp */
#ifdef NAN_COMPARISON_WRONG
    if (isnan(number1)) {
      if (isnan(number2)) {
        return 0;
      } else {
        return 1;
      } /* if */
    } else if (isnan(number2)) {
      return -1;
    } else if (number1 < number2) {
      return -1;
    } else if (number1 > number2) {
      return 1;
    } else {
      return 0;
    } /* if */
#else
    if (number1 < number2) {
      return -1;
    } else if (number1 > number2) {
      return 1;
    } else if (isnan(number1)) {
      if (isnan(number2)) {
        return 0;
      } else {
        return 1;
      } /* if */
    } else if (isnan(number2)) {
      return -1;
    } else {
      return 0;
    } /* if */
#endif
  } /* fltCmp */



/**
 *  Reinterpret the generic parameters as floatType and call fltCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(floatType).
 */
intType fltCmpGeneric (const genericType value1, const genericType value2)

  { /* fltCmpGeneric */
    return fltCmp(((const_rtlObjectType *) &value1)->value.floatValue,
                  ((const_rtlObjectType *) &value2)->value.floatValue);
  } /* fltCmpGeneric */



void fltCpyGeneric (genericType *const dest, const genericType source)

  { /* fltCpyGeneric */
    ((rtlObjectType *) dest)->value.floatValue =
        ((const_rtlObjectType *) &source)->value.floatValue;
  } /* fltCpyGeneric */



/**
 *  Convert a float to a string in decimal fixed point notation.
 *  The number is rounded to the specified number of digits ('precision').
 *  Halfway cases are rounded away from zero. Except for a 'precision' of
 *  zero the representation has a decimal point and at least one digit
 *  before and after the decimal point. Negative numbers are preceeded by
 *  a minus sign (e.g.: "-1.25"). When all digits in the result are 0 a
 *  possible negative sign is omitted.
 *  @param precision Number of digits after the decimal point.
 *         When the 'precision' is zero the decimal point is omitted.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR When the 'precision' is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType fltDgts (floatType number, intType precision)

  {
    char buffer[2001];
    const_cstriType buffer_ptr;
#ifndef USE_VARIABLE_FORMATS
    char form_buffer[10];
#endif
    memSizeType pos;
    striType result;

  /* fltDgts */
    if (precision < 0) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (precision > 1000) {
        precision = 1000;
      } /* if */
      if (isnan(number)) {
        buffer_ptr = "NaN";
      } else if (number == POSITIVE_INFINITY) {
        buffer_ptr = "Infinity";
      } else if (number == NEGATIVE_INFINITY) {
        buffer_ptr = "-Infinity";
      } else {
#ifdef USE_VARIABLE_FORMATS
        sprintf(buffer, "%1.*f", (int) precision, number);
#else
        if (precision > MAX_FORM) {
          sprintf(form_buffer, "%%1.%ldf", precision);
          sprintf(buffer, form_buffer, number);
        } else {
          sprintf(buffer, form[precision], number);
        } /* if */
#endif
        buffer_ptr = buffer;
        if (buffer[0] == '-' && buffer[1] == '0') {
          /* All forms of -0 are converted to 0 */
          if (buffer[2] == '.') {
            pos = 3;
            while (buffer[pos] == '0') {
              pos++;
            } /* while */
            if (buffer[pos] == '\0') {
              buffer_ptr++;
            } /* if */
          } else if (buffer[2] == '\0') {
            buffer_ptr++;
          } /* if */
        } /* if */
      } /* if */
      result = cstri_to_stri(buffer_ptr);
      if (unlikely(result == NULL)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* fltDgts */



#ifdef NAN_COMPARISON_WRONG
/**
 *  Check if two float numbers are equal.
 *  According to IEEE 754 a NaN is not equal to any float value.
 *  Therefore 'NaN = any_value' and 'any_value = NaN'
 *  always return FALSE. Even 'NaN = NaN' returns FALSE.
 *  @return TRUE if both numbers are equal, FALSE otherwise.
 */
boolType fltEq (floatType number1, floatType number2)

  { /* fltEq */
    if (isnan(number1) || isnan(number2)) {
      return FALSE;
    } else {
      return number1 == number2;
    } /* if */
  } /* fltEq */
#endif



#ifdef NAN_COMPARISON_WRONG
/**
 *  Check if 'number1' is greater than or equal to 'number2'.
 *  According to IEEE 754 a NaN is neither less than,
 *  equal to, nor greater than any value, including itself.
 *  When 'number1' or 'number2' is NaN, the result
 *  is FALSE;
 *  @return TRUE if 'number1' is greater than or equal to 'number2',
 *          FALSE otherwise.
 */
boolType fltGe (floatType number1, floatType number2)

  { /* fltGe */
    if (isnan(number1) || isnan(number2)) {
      return FALSE;
    } else {
      return number1 >= number2;
    } /* if */
  } /* fltGe */
#endif



#ifdef NAN_COMPARISON_WRONG
/**
 *  Check if 'number1' is greater than 'number2'.
 *  According to IEEE 754 a NaN is neither less than,
 *  equal to, nor greater than any value, including itself.
 *  When 'number1' or 'number2' is NaN, the result
 *  is FALSE;
 *  @return TRUE if 'number1' is greater than 'number2',
 *          FALSE otherwise.
 */
boolType fltGt (floatType number1, floatType number2)

  { /* fltGt */
    if (isnan(number1) || isnan(number2)) {
      return FALSE;
    } else {
      return number1 > number2;
    } /* if */
  } /* fltGt */
#endif



/**
 *  Compute the exponentiation of a float 'base' with an integer 'exponent'.
 *  @return the result of the exponentation.
 */
floatType fltIPow (floatType base, intType exponent)

  {
    uintType unsignedExponent;
    boolType neg_exp = FALSE;
    floatType power;

  /* fltIPow */
    /* printf("fltIPow(" FMT_E ", " FMT_D ")\n", base, exponent); */
#ifdef IPOW_EXPONENTIATION_BY_SQUARING
    if (base == 0.0) {
      if (exponent < 0) {
        power = POSITIVE_INFINITY;
      } else if (exponent == 0) {
        power = 1.0;
      } else {
        power = 0.0;
      } /* if */
    } else {
      if (exponent < 0) {
        /* The unsigned value is negated to avoid a signed integer */
        /* overflow when the smallest signed integer is negated.   */
        unsignedExponent = -(uintType) exponent;
        neg_exp = TRUE;
      } else {
        unsignedExponent = (uintType) exponent;
      } /* if */
      if (unsignedExponent & 1) {
        power = base;
      } else {
        power = 1.0;
      } /* if */
      unsignedExponent >>= 1;
      while (unsignedExponent != 0) {
        base *= base;
        if (unsignedExponent & 1) {
          power *= base;
        } /* if */
        unsignedExponent >>= 1;
      } /* while */
      if (neg_exp) {
        power = 1.0 / power;
      } /* if */
    } /* if */
#else
    if (base < 0.0) {
      if (exponent & 1) {
        power = -pow(-base, (floatType) exponent);
      } else {
        power = pow(-base, (floatType) exponent);
      } /* if */
    } else if (base == 0.0) {
      if (exponent < 0) {
        power = POSITIVE_INFINITY;
      } else if (exponent == 0) {
        power = 1.0;
      } else {
        power = 0.0;
      } /* if */
    } else { /* base > 0.0 */
      power = pow(base, (floatType) exponent);
    } /* if */
#endif
    /* printf("fltIPow --> " FMT_E "\n", power); */
    return power;
  } /* fltIPow */



/**
 *  Determine if a number is -0.0.
 *  This function is the only possibility to determine if a number
 *  is -0.0. The comparison operators (=, <>, <, >, <=, >=) and
 *  the function 'compare' treat 0.0 and -0.0 as equal. The
 *  'digits' operator and the 'str' function convert -0.0 to
 *  the string "0.0".
 *  @return TRUE if the number is -0.0,
 *          FALSE otherwise.
 */
boolType fltIsNegativeZero (floatType number)

  { /* fltIsNegativeZero */
    /* printf("fltIsNegativeZero %f %08x %08x\n",
        number, *(int32Type *) &number, *(int32Type *) &negativeZero); */
    return memcmp(&number, &negativeZero, sizeof(floatType)) == 0;
  } /* fltIsNegativeZero */



#ifdef NAN_COMPARISON_WRONG
/**
 *  Check if 'number1' is less than or equal to 'number2'.
 *  According to IEEE 754 a NaN is neither less than,
 *  equal to, nor greater than any value, including itself.
 *  When 'number1' or 'number2' is NaN, the result
 *  is FALSE;
 *  @return TRUE if 'number1' is less than or equal to 'number2',
 *          FALSE otherwise.
 */
boolType fltLe (floatType number1, floatType number2)

  { /* fltLe */
    if (isnan(number1) || isnan(number2)) {
      return FALSE;
    } else {
      return number1 <= number2;
    } /* if */
  } /* fltLe */
#endif



#ifdef NAN_COMPARISON_WRONG
/**
 *  Check if 'number1' is less than 'number2'.
 *  According to IEEE 754 a NaN is neither less than,
 *  equal to, nor greater than any value, including itself.
 *  When 'number1' or 'number2' is NaN, the result
 *  is FALSE;
 *  @return TRUE if 'number1' is less than 'number2',
 *          FALSE otherwise.
 */
boolType fltLt (floatType number1, floatType number2)

  { /* fltLt */
    if (isnan(number1) || isnan(number2)) {
      return FALSE;
    } else {
      return number1 < number2;
    } /* if */
  } /* fltLt */
#endif



/**
 *  Convert a string to a float number.
 *  @return the float result of the conversion.
 *  @exception RANGE_ERROR When the string contains not a float literal.
 */
floatType fltParse (const const_striType stri)

  {
#ifdef USE_STRTOD
    char buffer[MAX_CSTRI_BUFFER_LEN + 1];
    const_cstriType buffer_ptr;
    const_cstriType cstri;
    char *next_ch;
#else
    memSizeType position;
    floatType digitval;
#endif
    errInfoType err_info = OKAY_NO_ERROR;
    floatType result;

  /* fltParse */
    /* printf("fltParse(");
        prot_stri(stri);
        printf(")\n"); */
#ifdef USE_STRTOD
    if (likely(stri->size <= MAX_CSTRI_BUFFER_LEN)) {
      cstri = NULL;
      conv_to_cstri(buffer, stri, &err_info);
      buffer_ptr = buffer;
    } else {
      cstri = stri_to_cstri(stri, &err_info);
      buffer_ptr = cstri;
    } /* if */
    if (likely(err_info == OKAY_NO_ERROR)) {
/*    result = (floatType) atof(buffer_ptr); */
      result = (floatType) strtod(buffer_ptr, &next_ch);
      if (next_ch == buffer_ptr) {
        if (strcmp(buffer_ptr, "NaN") == 0) {
          result = NOT_A_NUMBER;
        } else if (strcmp(buffer_ptr, "Infinity") == 0) {
          result = POSITIVE_INFINITY;
        } else if (strcmp(buffer_ptr, "-Infinity") == 0) {
          result = NEGATIVE_INFINITY;
        } else {
          err_info = RANGE_ERROR;
        } /* if */
      } else if (next_ch != &buffer_ptr[stri->size]) {
        err_info = RANGE_ERROR;
      } /* if */
      if (cstri != NULL) {
        free_cstri(cstri, stri);
      } /* if */
    } /* if */
#else
    position = 0;
    result = 0.0;
    while (position < stri->size &&
        stri->mem[position] >= ((strElemType) '0') &&
        stri->mem[position] <= ((strElemType) '9')) {
      digitval = ((intType) stri->mem[position]) - ((intType) '0');
      if (result < MAX_DIV_10 ||
          (result == MAX_DIV_10 &&
          digitval <= MAX_REM_10)) {
        result = ((floatType) 10.0) * result + digitval;
      } else {
        err_info = RANGE_ERROR;
      } /* if */
      position++;
    } /* while */
    if (position == 0 || position < stri->size) {
      err_info = RANGE_ERROR;
    } /* if */
#endif
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
      result = 0.0;
    } /* if */
    return result;
  } /* fltParse */



#ifdef POWER_OF_ZERO_WRONG
/**
 *  Compute the exponentiation of a float 'base' with a float 'exponent'.
 *  This function is also correct with a zero base and a negative exponent.
 *  @return the result of the exponentation.
 */
floatType fltPow (floatType base, floatType exponent)

  { /* fltPow */
    if (base == 0.0 && exponent < 0.0) {
      if (memcmp(&base, &negativeZero, sizeof(floatType)) == 0) {
        if (floor(exponent) - exponent == 0.0) {
          /* integer exponent */
          if (floor(exponent / 2.0) * 2.0 - exponent < -0.5) {
            /* odd exponent */
            return NEGATIVE_INFINITY;
          } else {
            return POSITIVE_INFINITY;
          } /* if */
        } else {
          return POSITIVE_INFINITY;
        } /* if */
      } else {
        return POSITIVE_INFINITY;
      } /* if */
    } else {
      return pow(base, exponent);
    } /* if */
  } /* fltPow */
#endif



/**
 *  Compute pseudo-random number in the range [low, high].
 *  The random values are uniform distributed.
 *  @return the computed pseudo-random number.
 *  @exception RANGE_ERROR The range is empty (low > high holds).
 */
floatType fltRand (floatType low, floatType high)

  {
    double factor;
    floatType result;

  /* fltRand */
    /* printf("fltRand(%f, %f)\n", low, high); */
    if (unlikely(low > high)) {
      raise_error(RANGE_ERROR);
      return 0.0;
    } else {
      factor = high - low;
      if (factor == POSITIVE_INFINITY) {
        do {
          result = (floatType) uint_rand();
        } while (result < low || result > high);
      } else {
        do {
          result = ((floatType) uint_rand()) / ((floatType) UINTTYPE_MAX);
          result = low + factor * result;
        } while (result < low || result > high);
      } /* if */
      return result;
    } /* if */
  } /* fltRand */



/**
 *  Convert a 'float' number to a [[string]] in scientific notation.
 *  Scientific notation uses a decimal significand and a decimal exponent.
 *  The significand has an optional sign and exactly one digit before the
 *  decimal point. The fractional part of the significand is rounded
 *  to the specified number of digits ('precision'). Halfway cases are
 *  rounded away from zero. The fractional part is followed by the
 *  letter e and an exponent, which is always signed. The value zero is
 *  never written with a negative sign.
 *  @param precision Number of digits after the decimal point.
 *         When the 'precision' is zero the decimal point is omitted.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR When the 'precision' is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType fltSci (floatType number, intType precision)

  {
    char buffer[2001];
    const_cstriType buffer_ptr;
#ifndef USE_VARIABLE_FORMATS
    char form_buffer[10];
#endif
    memSizeType startPos;
    memSizeType pos;
    memSizeType len;
    memSizeType after_zeros;
    striType result;

  /* fltSci */
    if (precision < 0) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (precision > 1000) {
        precision = 1000;
      } /* if */
      if (isnan(number)) {
        buffer_ptr = "NaN";
      } else if (number == POSITIVE_INFINITY) {
        buffer_ptr = "Infinity";
      } else if (number == NEGATIVE_INFINITY) {
        buffer_ptr = "-Infinity";
      } else {
#ifdef USE_VARIABLE_FORMATS
        sprintf(buffer, "%1.*e", (int) precision, number);
#else
        if (precision > MAX_FORM) {
          sprintf(form_buffer, "%%1.%lde", precision);
          sprintf(buffer, form_buffer, number);
        } else {
          sprintf(buffer, form[precision], number);
        } /* if */
#endif
        startPos = 0;
        if (buffer[0] == '-' && buffer[1] == '0') {
          /* All forms of -0 are converted to 0 */
          if (buffer[2] == '.') {
            pos = 3;
            while (buffer[pos] == '0') {
              pos++;
            } /* while */
            if (buffer[pos] == 'e' && buffer[pos + 2] == '0') {
              pos += 3;
              while (buffer[pos] == '0') {
                pos++;
              } /* while */
              if (buffer[pos] == '\0') {
                startPos++;
              } /* if */
            } /* if */
          } else if (buffer[2] == 'e' && buffer[4] == '0') {
            pos = 5;
            while (buffer[pos] == '0') {
              pos++;
            } /* while */
            if (buffer[pos] == '\0') {
              startPos++;
            } /* if */
          } /* if */
        } /* if */
        len = strlen(buffer);
        if (len > startPos) {
          pos = len;
          do {
            pos--;
          } while (pos > startPos && buffer[pos] != 'e');
          pos += 2;
          after_zeros = pos;
          while (buffer[after_zeros] == '0') {
            after_zeros++;
          } /* while */
          if (buffer[after_zeros] == '\0') {
            after_zeros--;
          } /* if */
          memmove(&buffer[pos], &buffer[after_zeros],
              sizeof(char) * (len - after_zeros + 1));
        } /* if */
        buffer_ptr = &buffer[startPos];
      } /* if */
      result = cstri_to_stri(buffer_ptr);
      if (unlikely(result == NULL)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* fltSci */



/**
 *  Convert a float number to a string.
 *  The number is converted to a string with decimal representation.
 *  The sign of negative zero (-0.0) is ignored.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType fltStr (floatType number)

  {
    char buffer[1024];
    memSizeType len;
    striType result;

  /* fltStr */
    if (isnan(number)) {
      strcpy(buffer, "NaN");
      len = 3;
    } else if (number == POSITIVE_INFINITY) {
      strcpy(buffer, "Infinity");
      len = 8;
    } else if (number == NEGATIVE_INFINITY) {
      strcpy(buffer, "-Infinity");
      len = 9;
    } else {
#ifdef FLOATTYPE_DOUBLE
      len = doubleToCharBuffer(number, DOUBLE_STR_LARGE_NUMBER,
                               DOUBLE_STR_FORMAT, buffer);
#else
      len = doubleToCharBuffer(number, FLOAT_STR_LARGE_NUMBER,
                               FLOAT_STR_FORMAT, buffer);
#endif
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, len))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = len;
      memcpy_to_strelem(result->mem, (const_ustriType) buffer, len);
    } /* if */
    return result;
  } /* fltStr */
