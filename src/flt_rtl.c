/********************************************************************/
/*                                                                  */
/*  flt_rtl.c     Primitive actions for the float type.             */
/*  Copyright (C) 1989 - 2019  Thomas Mertes                        */
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
/*  Changes: 1993, 1994, 2005, 2010 - 2018  Thomas Mertes           */
/*  Content: Primitive actions for the float type.                  */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"
#include "math.h"
#include "float.h"
#include "ctype.h"
#include "errno.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"
#include "int_rtl.h"

#undef EXTERN
#define EXTERN
#include "flt_rtl.h"


#define USE_STRTOD 1
#define MAX_CSTRI_BUFFER_LEN 25
#define IPOW_EXPONENTIATION_BY_SQUARING 1
#define PRECISION_BUFFER_LEN 1000
/* The 3 additional chars below are for: "-1.". */
#define FLT_DGTS_ADDITIONAL_CHARS STRLEN("-1.")
#define FLT_DGTS_LEN (FLT_DGTS_ADDITIONAL_CHARS + DOUBLE_MAX_EXP10)
/* The 5 additional chars below are for: "-1.e+". */
#define FLT_SCI_ADDITIONAL_CHARS STRLEN("-1.e+")
#define FLT_SCI_LEN (FLT_SCI_ADDITIONAL_CHARS + MAX_PRINTED_EXPONENT_DIGITS)

#if FLOATTYPE_DOUBLE
#define MIN_NEGATIVE_SUBNORMAL -2.2250738585072009e-308
#define MAX_POSITIVE_SUBNORMAL  2.2250738585072009e-308
#define FREXP_SUBNORMAL_FACTOR  1.8446744073709552e+19 /* == 0x1p64 */
#else
#define MIN_NEGATIVE_SUBNORMAL -1.1754942106924411e-38
#define MAX_POSITIVE_SUBNORMAL  1.1754942106924411e-38
#define FREXP_SUBNORMAL_FACTOR  4.294967296e+09 /* == 0x1p32 */
#endif

/* Natural logarithm of 2: */
#define LN2 0.693147180559945309417232121458176568075500134360255254120680009493393

#if FLOAT_ZERO_DIV_ERROR
const rtlValueUnion f_const[] =
#if FLOATTYPE_DOUBLE
    {{GENERIC_SUFFIX(0xfff8000000000000)},
     {GENERIC_SUFFIX(0x7ff0000000000000)},
     {GENERIC_SUFFIX(0xfff0000000000000)}};
#else
    {{0xffc00000}, {0x7f800000}, {0xff800000}};
#endif
#endif

#if USE_NEGATIVE_ZERO_BITPATTERN
static const rtlValueUnion neg_zero_const =
#if FLOATTYPE_DOUBLE
    {GENERIC_SUFFIX(0x8000000000000000)};
#else
    {0x80000000};
#endif
#endif

floatType negativeZero;

#if !PRINTF_SUPPORTS_VARIABLE_FORMATS
static const const_cstriType fmt_e[] = {
    "%1.0e",  "%1.1e",  "%1.2e",  "%1.3e",  "%1.4e",  "%1.5e",
    "%1.6e",  "%1.7e",  "%1.8e",  "%1.9e",  "%1.10e", "%1.11e",
    "%1.12e", "%1.13e", "%1.14e", "%1.15e", "%1.16e", "%1.17e",
    "%1.18e", "%1.19e", "%1.20e", "%1.21e", "%1.22e", "%1.23e",
    "%1.24e", "%1.25e", "%1.26e", "%1.27e", "%1.28e"};

static const const_cstriType fmt_f[] = {
    "%1.0f",  "%1.1f",  "%1.2f",  "%1.3f",  "%1.4f",  "%1.5f",
    "%1.6f",  "%1.7f",  "%1.8f",  "%1.9f",  "%1.10f", "%1.11f",
    "%1.12f", "%1.13f", "%1.14f", "%1.15f", "%1.16f", "%1.17f",
    "%1.18f", "%1.19f", "%1.20f", "%1.21f", "%1.22f", "%1.23f",
    "%1.24f", "%1.25f", "%1.26f", "%1.27f", "%1.28f"};
#endif



/**
 *  Setup floating point computations and initialize constants.
 *  This function must be called before doing any floating
 *  point computations.
 */
void setupFloat (void)

  {
#if !USE_NEGATIVE_ZERO_BITPATTERN
    floatType zero = 0.0;
    floatType plusInf;
#endif

  /* setupFloat */
#ifdef TURN_OFF_FP_EXCEPTIONS
    _control87(MCW_EM, MCW_EM);
#endif
#if USE_NEGATIVE_ZERO_BITPATTERN
    negativeZero = neg_zero_const.floatValue;
#else
    plusInf = 1.0 / zero;
    negativeZero = -1.0 / plusInf;
#endif
  } /* setupFloat */



#ifdef DEFINE_MATHERR_FUNCTION
int matherr (struct _exception *a)

  { /* matherr */
    a->retval = a->retval;
    return 1;
  } /* matherr */
#endif



#ifdef DEFINE__MATHERR_FUNCTION
int _matherr (struct _exception *a)

  { /* _matherr */
    a->retval = a->retval;
    return 1;
  } /* _matherr */
#endif



/**
 *  Get the integer mantissa and a binary exponent from a double.
 *  @param doubleValue Value that should be split into mantissa
 *         and exponent (NaN, Infinity and -Infinity are not allowed).
 *  @param binaryExponent Destination for the binary exponent.
 *  @return the integer mantissa.
 */
int64Type getMantissaAndExponent (double doubleValue, int *binaryExponent)

  {
    double mantissa;
#if FREXP_FUNCTION_OKAY
    int exponent;
#else
    intType exponent;
#endif
    int64Type intMantissa;

  /* getMantissaAndExponent */
#if FREXP_FUNCTION_OKAY
    mantissa = frexp(doubleValue, &exponent);
#else
    mantissa = fltDecompose(doubleValue, &exponent);
#endif
    intMantissa = (int64Type) (mantissa * DOUBLE_MANTISSA_FACTOR);
    *binaryExponent = (int) (exponent - DOUBLE_MANTISSA_SHIFT);
    return intMantissa;
  } /* getMantissaAndExponent */



/**
 *  Create double value from an integer mantissa and a binary exponent.
 *  @param intMantissa Integer mantissa of double value.
 *  @param binaryExponent Binary exponent of double value.
 *  @return the result of flt(intMantissa) * 2.0 ** binaryExponent.
 */
double setMantissaAndExponent (int64Type intMantissa, int binaryExponent)

  {
    double mantissa;
    int exponent;
    double doubleValue;

  /* setMantissaAndExponent */
    mantissa = (double) intMantissa / DOUBLE_MANTISSA_FACTOR;
    exponent = binaryExponent + DOUBLE_MANTISSA_SHIFT;
    doubleValue = fltLdexp(mantissa, exponent);
    return doubleValue;
  } /* setMantissaAndExponent */



/**
 *  Write the decimal representation of a double to a buffer.
 *  The result in buffer uses the style [-]ddd.ddd where there is at least
 *  one digit before and after the decimal point. The number of digits
 *  after the decimal point is determined automatically. Except for the
 *  case if there is only one zero digit after the decimal point,
 *  the last digit is never zero. Negative zero (-0.0) and positive
 *  zero (+0.0) are both converted to "0.0".
 *  @param doubleValue Number to be converted (NaN, Infinity and
 *         -Infinity are not allowed).
 *  @param largeNumber If abs(doubleValue) > largeValue holds
 *         the format %1.1f is used (largeNumber is either
 *         DOUBLE_STR_LARGE_NUMBER or FLOAT_STR_LARGE_NUMBER).
 *  @param format Format to be used if abs(doubleValue) <= largeValue
 *         holds (format is eitner FMT_E_DBL or FMT_E_FLT).
 *  @param buffer Destination buffer for the decimal representation.
 *  @return the number of characters in the destination buffer.
 */
memSizeType doubleToCharBuffer (const double doubleValue,
    const double largeNumber, const char *format, char *buffer)

  {
    int decimalExponent;
    memSizeType pos;
    memSizeType start;
    memSizeType scale;
    memSizeType len;

  /* doubleToCharBuffer */
    logFunction(printf("doubleToCharBuffer(" FMT_E_DBL ", " FMT_E_DBL
                       ", \"%s\", *)\n",
                       doubleValue, largeNumber, format););
#if FLOAT_ZERO_COMPARISON_OKAY
    if (doubleValue == 0.0) {
#else
    if (doubleValue == 0.0 || fltIsNegativeZero(doubleValue)) {
#endif
      memcpy(buffer, "0.0", 3);
      len = 3;
    } else if (doubleValue < -largeNumber || doubleValue > largeNumber) {
      len = (memSizeType) sprintf(buffer, "%1.1f", doubleValue);
    } else {
      len = (memSizeType) sprintf(buffer, format, doubleValue);
      /* printf("buffer: \"%s\"\n", buffer); */
      /* Subtract two more chars for sign and letter 'e': */
      len -= MIN_PRINTED_EXPONENT_DIGITS + 2;
      while (buffer[len] != 'e') {
        len--;
      } /* while */
      pos = len + 2; /* skip the exponent sign */
      decimalExponent = (int) buffer[pos] - '0';
      pos++;
      while (buffer[pos] >= '0') {
        decimalExponent *= 10;
        decimalExponent += (int) buffer[pos] - '0';
        pos++;
      } /* while */
      if (buffer[len + 1] == '-') {
        decimalExponent = -decimalExponent;
      } /* if */
      /* printf("decimalExponent: %ld\n", decimalExponent); */
      do {
        len--;
      } while (buffer[len] == '0');
      len++;
      /* printf("len: " FMT_U_MEM "\n", len); */
      start = buffer[0] == '-';
      if (decimalExponent > 0) {
        scale = (memSizeType) decimalExponent;
        if (scale >= len - start - 2) {
          /* No digits <> '0' after the decimal point */
          memmove(&buffer[start + 1], &buffer[start + 2], len - start - 2);
          memset(&buffer[len - 1], '0', scale + 2 - len + start);
          memcpy(&buffer[start + scale + 1], ".0", 2);
          len = start + scale + 3;
        } else {
          memmove(&buffer[start + 1], &buffer[start + 2], scale);
          buffer[start + scale + 1] = '.';
        } /* if */
      } else if (decimalExponent < 0) {
        /* No digits <> '0' before the decimal point */
        scale = (memSizeType) -decimalExponent;
        memmove(&buffer[start + 2 + scale], &buffer[start + 2], len - start - 2);
        buffer[start + 1 + scale] = buffer[start];
        memset(&buffer[start + 2], '0', scale - 1);
        memcpy(&buffer[start], "0.", 2);
        len += scale;
      } else if (buffer[len - 1] == '.') {
        /* Make sure that there is one digit after the decimal point */
        len++;
      } /* if */
      /* printf("len: " FMT_U_MEM "\n", len);
         buffer[len] = '\0';
         printf("buffer: \"%s\"\n", buffer); */
    } /* if */
    logFunction(printf("doubleToCharBuffer(" FMT_E_DBL ", " FMT_E_DBL
                       ", \"%s\", \"%.*s\") --> " FMT_U_MEM "\n",
                       doubleValue, largeNumber, format, (int) len,
                       buffer, len););
    return len;
  } /* doubleToCharBuffer */



/**
 *  Compare two float numbers.
 *  Because fltCmp is used to sort float values, a unique
 *  sort sequence of all values is needed. Therefore fltCmp
 *  considers NaN as equal to itself and greater than Infinity.
 *  Negative zero (-0.0) is considered by fltCmp to be equal to
 *  positive zero (+0.0). This conforms to the behavior of all
 *  other float comparisons with zero.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType fltCmp (floatType number1, floatType number2)

  {
    intType signumValue;

  /* fltCmp */
    logFunction(printf("fltCmp(" FMT_E ", " FMT_E ")\n",
                       number1, number2););
#if FLOAT_COMPARISON_OKAY
    if (number1 < number2) {
      signumValue = -1;
    } else if (number1 > number2) {
      signumValue = 1;
    } else {
      /* The expression isnan(NaN) can return any value except 0. */
      signumValue = (os_isnan(number1) != 0) - (os_isnan(number2) != 0);
    } /* if */
#else
    if (unlikely(os_isnan(number1))) {
      /* The expression isnan(NaN) can return any value except 0. */
      signumValue = os_isnan(number2) == 0;
    } else if (unlikely(os_isnan(number2))) {
      signumValue = -1;
#if !FLOAT_ZERO_COMPARISON_OKAY
    } else if (fltLt(number1, number2)) {
      signumValue = -1;
    } else {
      signumValue = fltGt(number1, number2);
    } /* if */
#else
    } else if (number1 < number2) {
      signumValue = -1;
    } else {
      signumValue = number1 > number2;
    } /* if */
#endif
#endif
    logFunction(printf("fltCmp --> " FMT_D "\n", signumValue););
    return signumValue;
  } /* fltCmp */



/**
 *  Reinterpret the generic parameters as floatType and call fltCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(floatType).
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType fltCmpGeneric (const genericType value1, const genericType value2)

  { /* fltCmpGeneric */
    return fltCmp(((const_rtlObjectType *) &value1)->value.floatValue,
                  ((const_rtlObjectType *) &value2)->value.floatValue);
  } /* fltCmpGeneric */



/**
 *  Reinterpret the generic parameters as floatType and assign source to dest.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(floatType).
 */
void fltCpyGeneric (genericType *const dest, const genericType source)

  { /* fltCpyGeneric */
    ((rtlObjectType *) dest)->value.floatValue =
        ((const_rtlObjectType *) &source)->value.floatValue;
  } /* fltCpyGeneric */



#if !FREXP_FUNCTION_OKAY
/**
 *  Decompose float into normalized fraction and integral exponent for 2.
 *  If the argument (number) is 0.0, -0.0, Infinity, -Infinity or NaN the
 *  fraction is set to the argument and the exponent is set to 0.
 *  For all other arguments the fraction is set to an absolute value
 *  between 0.5(included) and 1.0(excluded) and the exponent is set such
 *  that number = fraction * 2.0 ** exponent holds.
 *  @param exponent Destination for the exponent of the decomposed number.
 *  @param number Number to be decomposed into fraction and exponent.
 *  @return the normalized fraction.
 */
floatType fltDecompose (const floatType number, intType *const exponent)

  {
    int exponent_var = 0;
    floatType fraction;

  /* fltDecompose */
    logFunction(printf("fltDecompose(" FMT_E ", *)\n", number););
#if !FREXP_INFINITY_NAN_OKAY
    if (unlikely(os_isnan(number) ||
                 number == POSITIVE_INFINITY ||
                 number == NEGATIVE_INFINITY)) {
      fraction = number;
      *exponent = 0;
    } else
#endif
#if !FREXP_SUBNORMAL_OKAY
    if (number >= MIN_NEGATIVE_SUBNORMAL &&
        number <= MAX_POSITIVE_SUBNORMAL && number != 0.0) {
      /* Subnormal number */
      fraction = frexp(number * FREXP_SUBNORMAL_FACTOR, &exponent_var);
      *exponent = (intType) exponent_var - FLOATTYPE_SIZE;
    } else
#endif
    {
      fraction = frexp(number, &exponent_var);
      *exponent = (intType) exponent_var;
    } /* if */
    logFunction(printf("fltDecompose(" FMT_E ", " FMT_D ") --> " FMT_E "\n",
                       number, *exponent, fraction););
    return fraction;
  } /* fltDecompose */
#endif



/**
 *  Convert a float to a string in decimal fixed point notation.
 *  The number is rounded to the specified number of digits ('precision').
 *  Halfway cases are rounded away from zero. Except for a 'precision' of
 *  zero the representation has a decimal point and at least one digit
 *  before and after the decimal point. Negative numbers are preceeded by
 *  a minus sign (e.g.: "-1.25"). If all digits in the result are 0 a
 *  possible negative sign is omitted.
 *  @param precision Number of digits after the decimal point.
 *         If the 'precision' is zero the decimal point is omitted.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR If the 'precision' is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType fltDgts (floatType number, intType precision)

  {
    char buffer_1[PRECISION_BUFFER_LEN + FLT_DGTS_LEN + NULL_TERMINATION_LEN];
    char *buffer = buffer_1;
    const_cstriType buffer_ptr;
    /* The 4 additional chars below are for "%1.f". */
    char form_buffer[INTTYPE_DECIMAL_SIZE + STRLEN("%1.f") + NULL_TERMINATION_LEN];
    memSizeType pos;
    memSizeType len;
    striType result;

  /* fltDgts */
    logFunction(printf("fltDgts(" FMT_E ", " FMT_D ")\n", number, precision););
    if (unlikely(precision < 0)) {
      logError(printf("fltDgts(" FMT_E ", " FMT_D "): Precision negative.\n",
                      number, precision););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (unlikely(precision > PRECISION_BUFFER_LEN &&
                        ((uintType) precision > MAX_CSTRI_LEN - FLT_DGTS_LEN ||
                        !ALLOC_CSTRI(buffer, (memSizeType) precision + FLT_DGTS_LEN)))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      if (os_isnan(number)) {
        buffer_ptr = "NaN";
        len = STRLEN("NaN");
      } else if (number == POSITIVE_INFINITY) {
        buffer_ptr = "Infinity";
        len = STRLEN("Infinity");
      } else if (number == NEGATIVE_INFINITY) {
        buffer_ptr = "-Infinity";
        len = STRLEN("-Infinity");
      } else {
#ifdef LIMIT_FMT_F_MAXIMUM_FLOAT_PRECISION
        if (unlikely(precision > PRINTF_FMT_F_MAXIMUM_FLOAT_PRECISION)) {
          len = (memSizeType) sprintf(buffer, "%1."
              LIMIT_FMT_F_MAXIMUM_FLOAT_PRECISION "f", number);
        } else
#endif
#if PRINTF_SUPPORTS_VARIABLE_FORMATS
        if (unlikely(precision > INT_MAX)) {
          sprintf(form_buffer, "%%1." FMT_D "f", precision);
          len = (memSizeType) sprintf(buffer, form_buffer, number);
        } else {
          len = (memSizeType) sprintf(buffer, "%1.*f", (int) precision, number);
        } /* if */
#else
        if (unlikely(precision >= sizeof(fmt_f) / sizeof(char *))) {
          sprintf(form_buffer, "%%1." FMT_D "f", precision);
          len = (memSizeType) sprintf(buffer, form_buffer, number);
        } else {
          len = (memSizeType) sprintf(buffer, fmt_f[precision], number);
        } /* if */
#endif
#ifdef PRINTF_FMT_F_MAXIMUM_FLOAT_PRECISION
        /* Some C run-time libraries do not have a fixed maximum   */
        /* for the float precision of printf(). Instead the actual */
        /* precision varies with each call of printf(). Up to a    */
        /* precision of PRINTF_FMT_F_MAXIMUM_FLOAT_PRECISION       */
        /* prinf() will always work ok.                            */
        if (precision > PRINTF_FMT_F_MAXIMUM_FLOAT_PRECISION) {
          pos = (memSizeType) (strchr(buffer, '.') - buffer);
          if ((memSizeType) precision >= len - pos) {
            memSizeType numZeros = (memSizeType) precision - (len - pos) + 1;
            memset(&buffer[len], '0', numZeros);
            len += numZeros;
            buffer[len] = '\0';
          } /* if */
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
              len--;
            } /* if */
          } else if (buffer[2] == '\0') {
            buffer_ptr++;
            len--;
          } /* if */
        } /* if */
      } /* if */
      result = cstri_buf_to_stri(buffer_ptr, len);
      if (buffer != buffer_1) {
        UNALLOC_CSTRI(buffer, (memSizeType) precision + FLT_DGTS_LEN);
      } /* if */
      if (unlikely(result == NULL)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("fltDgts(" FMT_E ", " FMT_D ") --> \"%s\"\n",
                       number, precision, striAsUnquotedCStri(result)););
    return result;
  } /* fltDgts */



#if !FLOAT_COMPARISON_OKAY
/**
 *  Check if two float numbers are equal.
 *  According to IEEE 754 a NaN is not equal to any float value.
 *  Therefore 'NaN = any_value' and 'any_value = NaN'
 *  always return FALSE. Even 'NaN = NaN' returns FALSE.
 *  @return TRUE if both numbers are equal, FALSE otherwise.
 */
boolType fltEq (floatType number1, floatType number2)

  {
    boolType isEqual;

  /* fltEq */
    logFunction(printf("fltEq(" FMT_E ", " FMT_E ")\n",
                       number1, number2););
#if !FLOAT_NAN_COMPARISON_OKAY
    if (os_isnan(number1) || os_isnan(number2)) {
      isEqual = FALSE;
    } else
#endif
#if !FLOAT_ZERO_COMPARISON_OKAY
    if (fltIsNegativeZero(number1)) {
      if (fltIsNegativeZero(number2)) {
        isEqual = TRUE;
      } else {
        isEqual = 0.0 == number2;
      } /* if */
    } else if (fltIsNegativeZero(number2)) {
      isEqual = number1 == 0.0;
    } else
#endif
    {
      isEqual = number1 == number2;
    } /* if */
    logFunction(printf("fltEq --> %d\n", isEqual););
    return isEqual;
  } /* fltEq */
#endif



#if !EXP_FUNCTION_OKAY
/**
 *  Compute Euler's number e raised to the power of x.
 *  @return e raised to the power of x.
 */
floatType fltExp (floatType exponent)

  {
    floatType power;

  /* fltExp */
    logFunction(printf("fltExp(" FMT_E ", *)\n", exponent););
#if !EXP_OF_NAN_OKAY
    if (unlikely(os_isnan(exponent))) {
      power = NOT_A_NUMBER;
    } else
#endif
    {
      power = exp(exponent);
    } /* if */
    logFunction(printf("fltExp --> " FMT_E "\n", power););
    return power;
  } /* fltExp */
#endif



#if !FLOAT_COMPARISON_OKAY
/**
 *  Check if 'number1' is greater than or equal to 'number2'.
 *  According to IEEE 754 a NaN is neither less than,
 *  equal to, nor greater than any value, including itself.
 *  If 'number1' or 'number2' is NaN, the result is FALSE.
 *  @return TRUE if 'number1' is greater than or equal to 'number2',
 *          FALSE otherwise.
 */
boolType fltGe (floatType number1, floatType number2)

  {
    boolType isGreaterEqual;

  /* fltGe */
    logFunction(printf("fltGe(" FMT_E ", " FMT_E ")\n",
                       number1, number2););
#if !FLOAT_NAN_COMPARISON_OKAY
    if (os_isnan(number1) || os_isnan(number2)) {
      isGreaterEqual = FALSE;
    } else
#endif
#if !FLOAT_ZERO_COMPARISON_OKAY
    if (fltIsNegativeZero(number1)) {
      if (fltIsNegativeZero(number2)) {
        isGreaterEqual = TRUE;
      } else {
        isGreaterEqual = 0.0 >= number2;
      } /* if */
    } else if (fltIsNegativeZero(number2)) {
      isGreaterEqual = number1 >= 0.0;
    } else
#endif
    {
      isGreaterEqual = number1 >= number2;
    } /* if */
    logFunction(printf("fltGe --> %d\n", isGreaterEqual););
    return isGreaterEqual;
  } /* fltGe */
#endif



#if !FLOAT_COMPARISON_OKAY
/**
 *  Check if 'number1' is greater than 'number2'.
 *  According to IEEE 754 a NaN is neither less than,
 *  equal to, nor greater than any value, including itself.
 *  If 'number1' or 'number2' is NaN, the result is FALSE.
 *  @return TRUE if 'number1' is greater than 'number2',
 *          FALSE otherwise.
 */
boolType fltGt (floatType number1, floatType number2)

  {
    boolType isGreaterThan;

  /* fltGt */
    logFunction(printf("fltGt(" FMT_E ", " FMT_E ")\n",
                       number1, number2););
#if !FLOAT_NAN_COMPARISON_OKAY
    if (os_isnan(number1) || os_isnan(number2)) {
      isGreaterThan = FALSE;
    } else
#endif
#if !FLOAT_ZERO_COMPARISON_OKAY
    if (fltIsNegativeZero(number1)) {
      if (fltIsNegativeZero(number2)) {
        isGreaterThan = FALSE;
      } else {
        isGreaterThan = 0.0 > number2;
      } /* if */
    } else if (fltIsNegativeZero(number2)) {
      isGreaterThan = number1 > 0.0;
    } else
#endif
    {
      isGreaterThan = number1 > number2;
    } /* if */
    logFunction(printf("fltGt --> %d\n", isGreaterThan););
    return isGreaterThan;
  } /* fltGt */
#endif



/**
 *  Compute the exponentiation of a float 'base' with an integer 'exponent'.
 *     A    ** 0  returns 1.0
 *     NaN  ** 0  returns 1.0
 *     NaN  ** B  returns NaN              for B <> 0
 *     0.0  ** B  returns 0.0              for B > 0
 *     0.0  ** 0  returns 1.0
 *     0.0  ** B  returns Infinity         for B < 0
 *   (-0.0) ** B  returns -Infinity        for B < 0 and odd(B)
 *     A    ** B  returns 1.0 / A ** (-B)  for B < 0
 *  @return the result of the exponentation.
 */
floatType fltIPow (floatType base, intType exponent)

  {
#if IPOW_EXPONENTIATION_BY_SQUARING
    uintType unsignedExponent;
    boolType neg_exp;
#endif
    floatType power;

  /* fltIPow */
    logFunction(printf("fltIPow(" FMT_E ", " FMT_D ")\n", base, exponent););
#if !FLOAT_NAN_COMPARISON_OKAY
    /* This is checked first on purpose. NaN should not be equal  */
    /* to any value. E.g.: NaN == x should always return FALSE.   */
    /* Beyond that NaN should not be equal to itself also. Some   */
    /* C compilers do not compute comparisons with NaN correctly. */
    /* As a consequence the NaN check is done first.              */
    if (unlikely(os_isnan(base))) {
      if (unlikely(exponent == 0)) {
        power = 1.0;
      } else {
        power = base;
      } /* if */
    } else
#endif
    if (unlikely(base == 0.0)) {
      if (exponent < 0) {
        if (fltIsNegativeZero(base) && ((-(uintType) exponent) & 1)) {
          power = NEGATIVE_INFINITY;
        } else {
          power = POSITIVE_INFINITY;
        } /* if */
      } else if (exponent == 0) {
        power = 1.0;
      } else { /* exponent > 0 */
        if (exponent & 1) {
          power = base; /* +0.0 respectively -0.0 are left as is. */
        } else {
          power = 0.0;
        } /* if */
      } /* if */
    } else
#if IPOW_EXPONENTIATION_BY_SQUARING
    {
      if (exponent < 0) {
        /* The unsigned value is negated to avoid a signed integer */
        /* overflow if the smallest signed integer is negated.     */
        unsignedExponent = -(uintType) exponent;
        neg_exp = TRUE;
      } else {
        unsignedExponent = (uintType) exponent;
        neg_exp = FALSE;
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
#if CHECK_FLOAT_DIV_BY_ZERO
        if (power == 0.0) {
          if (fltIsNegativeZero(power)) {
            power = NEGATIVE_INFINITY;
          } else {
            power = POSITIVE_INFINITY;
          } /* if */
        } else {
          power = 1.0 / power;
        } /* if */
#else
        power = 1.0 / power;
#endif
      } /* if */
    }
#else
    if (base < 0.0) {
      if (exponent & 1) {
        power = -pow(-base, (floatType) exponent);
      } else {
        power = pow(-base, (floatType) exponent);
      } /* if */
    } else { /* base > 0.0 */
      power = pow(base, (floatType) exponent);
    } /* if */
#endif
    logFunction(printf("fltIPow --> " FMT_E "\n", power););
    return power;
  } /* fltIPow */



/**
 *  Determine if a number is -0.0.
 *  This function is the only possibility to determine if a number
 *  is -0.0. The comparison operators (=, <>, <, >, <=, >=) and
 *  the function 'compare' treat 0.0 and -0.0 as equal. The
 *  operators ''digits'' and ''sci'' and the function ''str''
 *  return the same [[string]] for -0.0 and +0.0.
 *  @return TRUE if the number is -0.0,
 *          FALSE otherwise.
 */
boolType fltIsNegativeZero (floatType number)

  {
    boolType isNegativeZero;

  /* fltIsNegativeZero */
    isNegativeZero = memcmp(&number, &negativeZero, sizeof(floatType)) == 0;
    logFunction(printf("fltIsNegativeZero(" FMT_E ") --> %d\n",
                       number, isNegativeZero););
    return isNegativeZero;
  } /* fltIsNegativeZero */



#if !LDEXP_FUNCTION_OKAY
floatType fltLdexp (floatType number, int exponent)

  {
    floatType product;

  /* fltLdexp */
    logFunction(printf("fltLdexp(" FMT_E ", %d)\n", number, exponent););
#if !LDEXP_OF_NAN_OKAY
    if (unlikely(os_isnan(number))) {
      product = NOT_A_NUMBER;
    } else
#endif
    {
      product = ldexp(number, exponent);
    } /* if */
    logFunction(printf("fltLdexp --> " FMT_E "\n", product););
    return product;
  } /* fltLdexp */
#endif



#if !FLOAT_COMPARISON_OKAY
/**
 *  Check if 'number1' is less than or equal to 'number2'.
 *  According to IEEE 754 a NaN is neither less than,
 *  equal to, nor greater than any value, including itself.
 *  If 'number1' or 'number2' is NaN, the result is FALSE.
 *  @return TRUE if 'number1' is less than or equal to 'number2',
 *          FALSE otherwise.
 */
boolType fltLe (floatType number1, floatType number2)

  {
    boolType isLessEqual;

  /* fltLe */
    logFunction(printf("fltLe(" FMT_E ", " FMT_E ")\n",
                       number1, number2););
#if !FLOAT_NAN_COMPARISON_OKAY
    if (os_isnan(number1) || os_isnan(number2)) {
      isLessEqual = FALSE;
    } else
#endif
#if !FLOAT_ZERO_COMPARISON_OKAY
    if (fltIsNegativeZero(number1)) {
      if (fltIsNegativeZero(number2)) {
        isLessEqual = TRUE;
      } else {
        isLessEqual = 0.0 <= number2;
      } /* if */
    } else if (fltIsNegativeZero(number2)) {
      isLessEqual = number1 <= 0.0;
    } else
#endif
    {
      isLessEqual = number1 <= number2;
    } /* if */
    logFunction(printf("fltLe --> %d\n", isLessEqual););
    return isLessEqual;
  } /* fltLe */
#endif



#if !LOG_FUNCTION_OKAY
/**
 *  Return the natural logarithm (base e) of x.
 *   log(NaN)       returns NaN
 *   log(1.0)       returns 0.0
 *   log(Infinity)  returns Infinity
 *   log(0.0)       returns -Infinity
 *   log(X)         returns NaN        for X < 0.0
 *  @return the natural logarithm of x.
 */
floatType fltLog (floatType number)

  {
    floatType logarithm;

  /* fltLog */
    logFunction(printf("fltLog(" FMT_E ")\n", number););
#if !LOG_OF_NAN_OKAY || !FLOAT_NAN_COMPARISON_OKAY
    /* This is checked first on purpose. NaN should not be equal  */
    /* to any value. E.g.: NaN == x should always return FALSE.   */
    /* Beyond that NaN should not be equal to itself also. Some   */
    /* C compilers do not compute comparisons with NaN correctly. */
    /* As a consequence the NaN check is done first.              */
    if (unlikely(os_isnan(number))) {
      logarithm = number;
    } else
#endif
#if !LOG_OF_ZERO_OKAY
    if (unlikely(number == 0.0)) {
      logarithm = NEGATIVE_INFINITY;
    } else
#endif
#if !LOG_OF_NEGATIVE_OKAY
    if (unlikely(number < 0.0)) {
      logarithm = NOT_A_NUMBER;
    } else
#endif
    {
      logarithm = log(number);
    } /* if */
    logFunction(printf("fltLog(" FMT_E ") --> " FMT_E "\n",
                number, logarithm););
    return logarithm;
  } /* fltLog */
#endif



#if !LOG10_FUNCTION_OKAY
/**
 *  Returns the base 10 logarithm of x.
 *   log10(NaN)       returns NaN
 *   log10(1.0)       returns 0.0
 *   log10(Infinity)  returns Infinity
 *   log10(0.0)       returns -Infinity
 *   log10(X)         returns NaN        for X < 0.0
 *  @return the base 10 logarithm of x.
 */
floatType fltLog10 (floatType number)

  {
    floatType logarithm;

  /* fltLog10 */
    logFunction(printf("fltLog10(" FMT_E ")\n", number););
#if !LOG10_OF_NAN_OKAY || !FLOAT_NAN_COMPARISON_OKAY
    /* This is checked first on purpose. NaN should not be equal  */
    /* to any value. E.g.: NaN == x should always return FALSE.   */
    /* Beyond that NaN should not be equal to itself also. Some   */
    /* C compilers do not compute comparisons with NaN correctly. */
    /* As a consequence the NaN check is done first.              */
    if (unlikely(os_isnan(number))) {
      logarithm = number;
    } else
#endif
#if !LOG10_OF_ZERO_OKAY
    if (unlikely(number == 0.0)) {
      logarithm = NEGATIVE_INFINITY;
    } else
#endif
#if !LOG10_OF_NEGATIVE_OKAY
    if (unlikely(number < 0.0)) {
      logarithm = NOT_A_NUMBER;
    } else
#endif
    {
      logarithm = log10(number);
    } /* if */
    logFunction(printf("fltLog10(" FMT_E ") --> " FMT_E "\n",
                number, logarithm););
    return logarithm;
  } /* fltLog10 */
#endif



#if !LOG2_FUNCTION_OKAY
/**
 *  Returns the base 2 logarithm of x.
 *   log2(NaN)       returns NaN
 *   log2(1.0)       returns 0.0
 *   log2(Infinity)  returns Infinity
 *   log2(0.0)       returns -Infinity
 *   log2(X)         returns NaN        for X < 0.0
 *  @return the base 2 logarithm of x.
 */
floatType fltLog2 (floatType number)

  {
    floatType logarithm;

  /* fltLog2 */
    logFunction(printf("fltLog2(" FMT_E ")\n", number););
#if HAS_LOG2
#if !LOG2_OF_NAN_OKAY || !FLOAT_NAN_COMPARISON_OKAY
    /* This is checked first on purpose. NaN should not be equal  */
    /* to any value. E.g.: NaN == x should always return FALSE.   */
    /* Beyond that NaN should not be equal to itself also. Some   */
    /* C compilers do not compute comparisons with NaN correctly. */
    /* As a consequence the NaN check is done first.              */
    if (unlikely(os_isnan(number))) {
      logarithm = number;
    } else
#endif
#if !LOG2_OF_ZERO_OKAY
    if (unlikely(number == 0.0)) {
      logarithm = NEGATIVE_INFINITY;
    } else
#endif
#if !LOG2_OF_NEGATIVE_OKAY
    if (unlikely(number < 0.0)) {
      logarithm = NOT_A_NUMBER;
    } else
#endif
    {
      logarithm = log2(number);
    } /* if */
#else
    logarithm = fltLog(number) / LN2;
#endif
    logFunction(printf("fltLog2(" FMT_E ") --> " FMT_E "\n",
                number, logarithm););
    return logarithm;
  } /* fltLog2 */
#endif



#if !FLOAT_COMPARISON_OKAY
/**
 *  Check if 'number1' is less than 'number2'.
 *  According to IEEE 754 a NaN is neither less than,
 *  equal to, nor greater than any value, including itself.
 *  If 'number1' or 'number2' is NaN, the result is FALSE.
 *  @return TRUE if 'number1' is less than 'number2',
 *          FALSE otherwise.
 */
boolType fltLt (floatType number1, floatType number2)

  {
    boolType isLessThan;

  /* fltLt */
    logFunction(printf("fltLt(" FMT_E ", " FMT_E ")\n",
                       number1, number2););
#if !FLOAT_NAN_COMPARISON_OKAY
    if (os_isnan(number1) || os_isnan(number2)) {
      isLessThan = FALSE;
    } else
#endif
#if !FLOAT_ZERO_COMPARISON_OKAY
    if (fltIsNegativeZero(number1)) {
      if (fltIsNegativeZero(number2)) {
        isLessThan = FALSE;
      } else {
        isLessThan = 0.0 < number2;
      } /* if */
    } else if (fltIsNegativeZero(number2)) {
      isLessThan = number1 < 0.0;
    } else
#endif
    {
      isLessThan = number1 < number2;
    } /* if */
    logFunction(printf("fltLt --> %d\n", isLessThan););
    return isLessThan;
  } /* fltLt */
#endif



/**
 *  Compute the floating-point modulo of a division.
 *  The modulo has the same sign as the divisor.
 *  The modulo is dividend - floor(dividend / divisor) * divisor
 *    A        mod  NaN       returns  NaN
 *    NaN      mod  B         returns  NaN
 *    A        mod  0.0       returns  NaN
 *    Infinity mod  B         returns  NaN
 *   -Infinity mod  B         returns  NaN
 *    0.0      mod  B         returns  0.0         for B &lt;> 0.0
 *    A        mod  Infinity  returns  A           for A > 0
 *    A        mod  Infinity  returns  Infinity    for A < 0
 *    A        mod -Infinity  returns  A           for A < 0
 *    A        mod -Infinity  returns -Infinity    for A > 0
 *  @return the floating-point modulo of the division.
 */
floatType fltMod (floatType dividend, floatType divisor)

  {
    floatType modulo;

  /* fltMod */
    logFunction(printf("fltMod(" FMT_E ", " FMT_E ")\n", dividend, divisor););
    modulo = fltRem(dividend, divisor);
#if FLOAT_COMPARISON_OKAY
    if ((dividend < 0.0) ^ (divisor < 0.0) && modulo != 0.0) {
#else
    if (fltLt(dividend, 0.0) ^ fltLt(divisor, 0.0) && !fltEq(modulo, 0.0)) {
#endif
      modulo += divisor;
    } /* if */
    logFunction(printf("fltMod --> " FMT_E "\n", modulo););
    return modulo;
  } /* fltMod */



/**
 *  Convert a string to a float number.
 *  @return the float result of the conversion.
 *  @exception RANGE_ERROR If the string contains not a float literal.
 */
floatType fltParse (const const_striType stri)

  {
#if USE_STRTOD
    char buffer[MAX_CSTRI_BUFFER_LEN + NULL_TERMINATION_LEN];
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
    logFunction(printf("fltParse(\"%s\")\n", striAsUnquotedCStri(stri)););
#if USE_STRTOD
    if (likely(stri->size <= MAX_CSTRI_BUFFER_LEN)) {
      cstri = NULL;
      buffer_ptr = conv_to_cstri(buffer, stri);
      if (unlikely(buffer_ptr == NULL)) {
        logError(printf("fltParse(\"%s\"): conv_to_cstri() failed.\n",
                        striAsUnquotedCStri(stri)););
        err_info = RANGE_ERROR;
        result = 0.0;
      } /* if */
    } else {
      cstri = stri_to_cstri(stri, &err_info);
      buffer_ptr = cstri;
    } /* if */
    if (likely(buffer_ptr != NULL)) {
      if (isspace(buffer_ptr[0])) {
        logError(printf("fltParse(\"%s\"): String starts with whitespace.\n",
                        striAsUnquotedCStri(stri)););
        err_info = RANGE_ERROR;
        result = 0.0;
      } else {
        result = (floatType) strtod(buffer_ptr, &next_ch);
        if (next_ch == buffer_ptr) {
          if (strcmp(buffer_ptr, "NaN") == 0) {
            result = NOT_A_NUMBER;
          } else if (strcmp(buffer_ptr, "Infinity") == 0) {
            result = POSITIVE_INFINITY;
          } else if (strcmp(buffer_ptr, "-Infinity") == 0) {
            result = NEGATIVE_INFINITY;
          } else {
            logError(printf("fltParse(\"%s\"): No digit or sign found.\n",
                            striAsUnquotedCStri(stri)););
            err_info = RANGE_ERROR;
          } /* if */
        } else if (next_ch != &buffer_ptr[stri->size]) {
          logError(printf("fltParse(\"%s\"): Superfluous characters after float literal.\n",
                          striAsUnquotedCStri(stri)););
          err_info = RANGE_ERROR;
#if STRTOD_ACCEPTS_HEX_NUMBERS
        } else if (buffer_ptr[0] != '\0' &&
                   (buffer_ptr[1] == 'x' || buffer_ptr[1] == 'X' ||
                    (buffer_ptr[1] == '0' &&
                     (buffer_ptr[2] == 'x' || buffer_ptr[2] == 'X')))) {
          logError(printf("fltParse(\"%s\"): Hex float literals are not supported.\n",
                          striAsUnquotedCStri(stri)););
          err_info = RANGE_ERROR;
#endif
#if !STRTOD_ACCEPTS_DENORMAL_NUMBERS && ATOF_ACCEPTS_DENORMAL_NUMBERS
        } else if (result == 0.0 && errno == ERANGE) {
          result = (floatType) atof(buffer_ptr);
#endif
        } /* if */
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
    logFunction(printf("fltParse(\"%s\") --> " FMT_E "\n",
                       striAsUnquotedCStri(stri), result););
    return result;
  } /* fltParse */



#if !POW_FUNCTION_OKAY
/**
 *  Compute the exponentiation of a float 'base' with a float 'exponent'.
 *  This function corrects errors of the C function pow().
 *     A    ** B    returns NaN        for A < 0.0 and B is not integer
 *     A    ** 0.0  returns 1.0
 *     NaN  ** 0.0  returns 1.0
 *     NaN  ** B    returns NaN        for B <> 0.0
 *     0.0  ** B    returns 0.0        for B > 0.0
 *     0.0  ** 0.0  returns 1.0
 *     0.0  ** B    returns Infinity   for B < 0.0
 *   (-0.0) ** B    returns -Infinity  for B < 0.0 and odd(B)
 *     1.0  ** B    returns 1.0
 *     1.0  ** NaN  returns 1.0
 *     A    ** NaN  returns NaN        for A <> 1.0
 *  @return the result of the exponentation.
 */
floatType fltPow (floatType base, floatType exponent)

  {
    floatType power;
#if !POW_OF_NEGATIVE_OKAY
    floatType intPart;
#endif

  /* fltPow */
    logFunction(printf("fltPow(" FMT_E ", " FMT_E ")\n", base, exponent););
#if !POW_OF_NAN_OKAY || !FLOAT_NAN_COMPARISON_OKAY
    /* This is checked first on purpose. NaN should not be equal  */
    /* to any value. E.g.: NaN == x should always return FALSE.   */
    /* Beyond that NaN should not be equal to itself also. Some   */
    /* C compilers do not compute comparisons with NaN correctly. */
    /* As a consequence the NaN check is done first.              */
    if (unlikely(os_isnan(base))) {
#if FLOAT_NAN_COMPARISON_OKAY
      if (unlikely(exponent == 0.0)) {
#else
      if (unlikely(!os_isnan(exponent) && exponent == 0.0)) {
#endif
        power = 1.0;
      } else {
        power = base;
      } /* if */
    } else
#endif
#if !POW_OF_ZERO_OKAY
    if (unlikely(base == 0.0)) {
      if (unlikely(os_isnan(exponent))) {
        power = exponent;
      } else if (exponent < 0.0) {
        if (unlikely(fltIsNegativeZero(base) &&
                     exponent >= -FLOATTYPE_TO_INT_CONVERSION_LIMIT &&
                     floor(exponent) == exponent &&      /* exponent is an integer */
                     (((uint64Type) -exponent) & 1))) {  /* exponent is odd */
          power = NEGATIVE_INFINITY;
        } else {
          power = POSITIVE_INFINITY;
        } /* if */
      } else if (exponent == 0.0) {
        power = 1.0;
      } else { /* exponent > 0.0 */
        if (unlikely(exponent <= FLOATTYPE_TO_INT_CONVERSION_LIMIT &&
                     floor(exponent) == exponent &&      /* exponent is an integer */
                     (((uint64Type) exponent) & 1))) {   /* exponent is odd */
          power = base; /* +0.0 respectively -0.0 are left as is. */
        } else {
          power = 0.0;
        } /* if */
      } /* if */
    } else
#endif
#if !POW_OF_NEGATIVE_OKAY
    if (unlikely(base < 0.0 && modf(exponent, &intPart) != 0.0)) {
      power = NOT_A_NUMBER;
    } else
#endif
#if !POW_OF_ONE_OKAY
    if (unlikely(base == 1.0)) {
      power = 1.0;
    } else
#endif
#if !POW_EXP_NAN_OKAY
    /* This is checked before checking for negative infinity on purpose. */
    if (unlikely(os_isnan(exponent))) {
      if (unlikely(base == 1.0)) {
        power = 1.0;
      } else {
        power = exponent;
      } /* if */
    } else
#endif
#if !POW_EXP_MINUS_INFINITY_OKAY
    if (unlikely(exponent == NEGATIVE_INFINITY)) {
      if (base < -1.0 || base > 1.0) {
        power = 0.0;
      } else if (base == 1.0) {
        power = 1.0;
      } else {
        power = POSITIVE_INFINITY;
      } /* if */
    } else
#endif
    {
      power = pow(base, exponent);
    } /* if */
    logFunction(printf("fltPow --> " FMT_E "\n", power););
    return power;
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
    floatType randomNumber;

  /* fltRand */
    logFunction(printf("fltRand(" FMT_E ", " FMT_E ")\n", low, high););
    if (unlikely(low > high)) {
      logError(printf("fltRand(" FMT_E ", " FMT_E "): "
                      "The range is empty (low > high holds).\n",
                      low, high););
      raise_error(RANGE_ERROR);
      randomNumber = 0.0;
    } else {
      factor = high - low;
      if (factor == POSITIVE_INFINITY) {
        do {
          randomNumber = (floatType) uintRand();
        } while (randomNumber < low || randomNumber > high);
      } else {
        do {
          randomNumber = ((floatType) uintRand()) / ((floatType) UINTTYPE_MAX);
          randomNumber = low + factor * randomNumber;
        } while (randomNumber < low || randomNumber > high);
      } /* if */
    } /* if */
    logFunction(printf("fltRand --> " FMT_E "\n", randomNumber););
    return randomNumber;
  } /* fltRand */



#if !FMOD_FUNCTION_OKAY
/**
 *  Compute the floating-point remainder of a division.
 *  The remainder has the same sign as the dividend.
 *  The remainder is dividend - flt(trunc(dividend / divisor)) * divisor
 *  The remainder is computed without a conversion to integer.
 *    A        rem NaN       returns NaN
 *    NaN      rem B         returns NaN
 *    A        rem 0.0       returns NaN
 *    Infinity rem B         returns NaN
 *   -Infinity rem B         returns NaN
 *    0.0      rem B         returns 0.0  for B &lt;> 0.0
 *    A        rem Infinity  returns A
 *  @return the floating-point remainder of the division.
 */
floatType fltRem (floatType dividend, floatType divisor)

  {
    floatType remainder;

  /* fltRem */
    logFunction(printf("fltRem(" FMT_E ", " FMT_E ")\n", dividend, divisor););
#if !FMOD_DIVIDEND_NAN_OKAY
    if (unlikely(os_isnan(dividend))) {
      remainder = NOT_A_NUMBER;
    } else
#endif
#if !FMOD_DIVISOR_NAN_OKAY
    if (unlikely(os_isnan(divisor))) {
      remainder = NOT_A_NUMBER;
    } else
#endif
#if !FMOD_DIVIDEND_INFINITY_OKAY
    if (dividend == POSITIVE_INFINITY || dividend == NEGATIVE_INFINITY) {
      remainder = NOT_A_NUMBER;
    } else
#endif
#if !FMOD_DIVISOR_INFINITY_OKAY
    if (divisor == POSITIVE_INFINITY || divisor == NEGATIVE_INFINITY) {
      if (dividend == POSITIVE_INFINITY || dividend == NEGATIVE_INFINITY) {
        remainder = NOT_A_NUMBER;
      } else {
        remainder = dividend;
      } /* if */
    } else
#endif
#if !FMOD_DIVISOR_ZERO_OKAY
    if (divisor == 0.0) {
      remainder = NOT_A_NUMBER;
    } else
#endif
    {
      remainder = fmod(dividend, divisor);
    } /* if */
    logFunction(printf("fltRem --> " FMT_E "\n", remainder););
    return remainder;
  } /* fltRem */
#endif



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
 *         If the 'precision' is zero the decimal point is omitted.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR If the 'precision' is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType fltSci (floatType number, intType precision)

  {
    char buffer_1[PRECISION_BUFFER_LEN + FLT_SCI_LEN + NULL_TERMINATION_LEN];
    char *buffer = buffer_1;
    const_cstriType buffer_ptr;
    /* The 4 additional chars below are for "%1.e". */
    char form_buffer[INTTYPE_DECIMAL_SIZE + STRLEN("%1.e") + NULL_TERMINATION_LEN];
    memSizeType startPos;
    memSizeType pos;
    memSizeType len;
    memSizeType after_zeros;
    striType result;

  /* fltSci */
    logFunction(printf("fltSci(" FMT_E ", " FMT_D ")\n", number, precision););
    if (unlikely(precision < 0)) {
      logError(printf("fltSci(" FMT_E ", " FMT_D "): Precision negative.\n",
                      number, precision););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (unlikely(precision > PRECISION_BUFFER_LEN &&
                        ((uintType) precision > MAX_CSTRI_LEN - FLT_SCI_LEN ||
                        !ALLOC_CSTRI(buffer, (memSizeType) precision + FLT_SCI_LEN)))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      if (os_isnan(number)) {
        buffer_ptr = "NaN";
        len = STRLEN("NaN");
      } else if (number == POSITIVE_INFINITY) {
        buffer_ptr = "Infinity";
        len = STRLEN("Infinity");
      } else if (number == NEGATIVE_INFINITY) {
        buffer_ptr = "-Infinity";
        len = STRLEN("-Infinity");
      } else {
#ifdef LIMIT_FMT_E_MAXIMUM_FLOAT_PRECISION
        if (unlikely(precision > PRINTF_FMT_E_MAXIMUM_FLOAT_PRECISION)) {
          len = (memSizeType) sprintf(buffer, "%1."
              LIMIT_FMT_E_MAXIMUM_FLOAT_PRECISION "e", number);
        } else
#endif
#if PRINTF_SUPPORTS_VARIABLE_FORMATS
        if (unlikely(precision > INT_MAX)) {
          sprintf(form_buffer, "%%1." FMT_D "e", precision);
          len = (memSizeType) sprintf(buffer, form_buffer, number);
        } else {
          len = (memSizeType) sprintf(buffer, "%1.*e", (int) precision, number);
        } /* if */
#else
        if (unlikely(precision >= sizeof(fmt_e) / sizeof(char *))) {
          sprintf(form_buffer, "%%1." FMT_D "e", precision);
          len = (memSizeType) sprintf(buffer, form_buffer, number);
        } else {
          len = (memSizeType) sprintf(buffer, fmt_e[precision], number);
        } /* if */
#endif
#ifdef PRINTF_FMT_E_MAXIMUM_FLOAT_PRECISION
        /* Some C run-time libraries do not have a fixed maximum   */
        /* for the float precision of printf(). Instead the actual */
        /* precision varies with each call of printf(). Up to a    */
        /* precision of PRINTF_FMT_E_MAXIMUM_FLOAT_PRECISION       */
        /* prinf() will always work ok.                            */
        if (precision > PRINTF_FMT_E_MAXIMUM_FLOAT_PRECISION) {
          pos = (memSizeType) (strchr(buffer, 'e') - buffer);
          if ((memSizeType) precision > pos - STRLEN("1.") -
              (buffer[0] == '-')){
            memSizeType numZeros = (memSizeType) precision -
                (pos - STRLEN("1.") - (buffer[0] == '-'));
            memmove(&buffer[pos + numZeros], &buffer[pos],
                len - pos + NULL_TERMINATION_LEN);
            memset(&buffer[pos], '0', numZeros);
            len += numZeros;
          } /* if */
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
                len--;
              } /* if */
            } /* if */
          } else if (buffer[2] == 'e' && buffer[4] == '0') {
            pos = 5;
            while (buffer[pos] == '0') {
              pos++;
            } /* while */
            if (buffer[pos] == '\0') {
              startPos++;
              len--;
            } /* if */
          } /* if */
        } /* if */
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
          len -= after_zeros - pos;
        } /* if */
        buffer_ptr = &buffer[startPos];
      } /* if */
      result = cstri_buf_to_stri(buffer_ptr, len);
      if (buffer != buffer_1) {
        UNALLOC_CSTRI(buffer, (memSizeType) precision + FLT_SCI_LEN);
      } /* if */
      if (unlikely(result == NULL)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("fltSci(" FMT_E ", " FMT_D ") --> \"%s\"\n",
                       number, precision, striAsUnquotedCStri(result)););
    return result;
  } /* fltSci */



#if !SQRT_FUNCTION_OKAY
/**
 *  Returns the non-negative square root of x.
 *  Used if sqrt() does not return NaN for negative numbers.
 *  @return the square root of x.
 */
floatType fltSqrt (floatType radicand)

  {
    floatType squareRoot;

  /* fltSqrt */
    logFunction(printf("fltSqrt(" FMT_E ")\n", radicand););
#if !SQRT_OF_NAN_OKAY
    if (unlikely(os_isnan(radicand))) {
      squareRoot = NOT_A_NUMBER;
    } else
#endif
#if !SQRT_OF_NEGATIVE_OKAY
    if (radicand < 0.0) {
      squareRoot = NOT_A_NUMBER;
    } else
#endif
    {
      squareRoot = sqrt(radicand);
    } /* if */
    logFunction(printf("fltSqrt(" FMT_E ") --> " FMT_E "\n",
                radicand, squareRoot););
    return squareRoot;
  } /* fltSqrt */
#endif



/**
 *  Convert a float number to a string.
 *  The number is converted to a string with decimal representation.
 *  The result string has the style [-]ddd.ddd where there is at least
 *  one digit before and after the decimal point. The number of digits
 *  after the decimal point is determined automatically. Except for the
 *  case if there is only one zero digit after the decimal point,
 *  the last digit is never zero. Negative zero (-0.0) and positive
 *  zero (+0.0) are both converted to "0.0".
 *   str(16.125)    returns "16.125"
 *   str(-0.0)      returns "0.0"
 *   str(Infinity)  returns "Infinity"
 *   str(-Infinity) returns "-Infinity"
 *   str(NaN)       returns "NaN"
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType fltStr (floatType number)

  {
    char buffer[DOUBLE_TO_CHAR_BUFFER_SIZE];
    const_cstriType buffer_ptr;
    memSizeType len;
    striType result;

  /* fltStr */
    logFunction(printf("fltStr(" FMT_E ")\n", number););
    if (os_isnan(number)) {
      buffer_ptr = "NaN";
      len = STRLEN("NaN");
    } else if (number == POSITIVE_INFINITY) {
      buffer_ptr = "Infinity";
      len = STRLEN("Infinity");
    } else if (number == NEGATIVE_INFINITY) {
      buffer_ptr = "-Infinity";
      len = STRLEN("-Infinity");
    } else {
      buffer_ptr = buffer;
#if FLOATTYPE_DOUBLE
      len = doubleToCharBuffer(number, DOUBLE_STR_LARGE_NUMBER,
                               FMT_E_DBL, buffer);
#else
      len = doubleToCharBuffer(number, FLOAT_STR_LARGE_NUMBER,
                               FMT_E_FLT, buffer);
#endif
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, len))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = len;
      memcpy_to_strelem(result->mem, (const_ustriType) buffer_ptr, len);
    } /* if */
    logFunction(printf("fltStr(" FMT_E ") --> \"%s\"\n",
                       number, striAsUnquotedCStri(result)););
    return result;
  } /* fltStr */
