/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Analyzer - Scanner                                      */
/*  File: seed7/src/numlit.c                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Read the next number from the source file.             */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "chclsutl.h"
#include "identutl.h"
#include "error.h"
#include "symbol.h"
#include "infile.h"
#include "stat.h"
#include "findid.h"
#include "big_drv.h"

#undef EXTERN
#define EXTERN
#include "numlit.h"



static void readDecimal (register sySizeType position)

  {
    register int character;

  /* readDecimal */
    logFunction(printf("readDecimal\n"););
    check_symb_length(position);
    symbol.name[position++] = (ucharType) in_file.character;
    while (char_class(character = next_character()) == DIGITCHAR) {
      check_symb_length(position);
      symbol.name[position++] = (ucharType) character;
    } /* while */
    symbol.name[position] = '\0';
    in_file.character = character;
    logFunction(printf("readDecimal -->\n"););
  } /* readDecimal */



static uintType decimalValue (const const_ustriType digits)

  {
    boolType tooBig = FALSE;
    unsigned int position = 0;
    uintType digitval;
    uintType uintValue = 0;

  /* decimalValue */
    logFunction(printf("decimalValue(\"%s\")\n", digits););
    while (digits[position] != '\0') {
      digitval = ((uintType) digits[position]) - ((uintType) '0');
      if (unlikely(uintValue > MAX_DIV_10)) {
        tooBig = TRUE;
      } else {
        uintValue = ((uintType) 10) * uintValue + digitval;
      } /* if */
      position++;
    } /* while */
    if (unlikely(tooBig || uintValue > (uintType) INTTYPE_MAX)) {
      err_ustri(CARD_DECIMAL_TOO_BIG, digits);
      uintValue = 0;
    } /* if */
    logFunction(printf("decimalValue --> " FMT_U "\n",
                       uintValue););
    return uintValue;
  } /* decimalValue */



static inline intType basedValue (const uintType base, const const_ustriType digits)

  {
    boolType illegalDigit = FALSE;
    boolType tooBig = FALSE;
    uintType max_div_base;
    unsigned int position = 0;
    uintType digitval;
    uintType uintValue = 0;

  /* basedValue */
    logFunction(printf("basedValue(" FMT_U ", \"%s\")\n",
                       base, digits););
    max_div_base = (uintType) INTTYPE_MAX / base;
    while (digits[position] != '\0') {
      digitval = digit_value[(int) digits[position]];
      if (unlikely(digitval >= base)) {
        if (!illegalDigit) {
          err_num_stri(ILLEGALBASEDDIGIT, (int) digits[position], (int) base, digits);
          illegalDigit = TRUE;
        } /* if */
      } else if (unlikely(uintValue > max_div_base)) {
        tooBig = TRUE;
      } else {
        uintValue = base * uintValue + digitval;
      } /* if */
      position++;
    } /* while */
    if (unlikely(illegalDigit)) {
      uintValue = 0;
    } else if (unlikely(tooBig || uintValue > (uintType) INTTYPE_MAX)) {
      err_num_stri(CARD_BASED_TOO_BIG, 0, (int) base, digits);
      uintValue = 0;
    } /* if */
    logFunction(printf("basedValue --> " FMT_D "\n",
                       (intType) uintValue););
    return (intType) uintValue;
  } /* basedValue */



static inline boolType readBased (void)

  {
    register sySizeType position;
    register int character;
    boolType okay;

  /* readBased */
    position = 0;
    character = next_character();
    if (char_class(character) == DIGITCHAR ||
        char_class(character) == LETTERCHAR) {
      do {
        check_symb_length(position);
        symbol.name[position++] = (ucharType) character;
        character = next_character();
      } while (char_class(character) == DIGITCHAR ||
          char_class(character) == LETTERCHAR);
      okay = TRUE;
    } else {
      err_cchar(EXTDIGITEXPECTED, character);
      okay = FALSE;
    } /* if */
    symbol.name[position] = '\0';
    in_file.character = character;
    return okay;
  } /* readBased */



static inline bigIntType readBigBased (uintType base)

  {
    memSizeType pos;
    boolType okay = TRUE;
    uintType digitval;
    bigIntType result;

  /* readBigBased */
    logFunction(printf("readBigBased(" FMT_U ")\n", base););
    in_file.character = next_character();
    pos = 0;
    do {
      while (pos != symbol.stri_max && symbol.name[pos] != '\0') {
        digitval = digit_value[(int) symbol.name[pos]];
        if (unlikely(digitval >= base)) {
          if (okay) {
            err_num_stri(ILLEGALBASEDDIGIT,
                (int) symbol.name[pos], (int) base, symbol.name);
            okay = FALSE;
          } /* if */
        } /* if */
        symbol.striValue->mem[pos] = (strElemType) symbol.name[pos];
        pos++;
      } /* while */
      check_stri_length(pos);
    } while (symbol.name[pos] != '\0');
    symbol.striValue->size = pos;
    bigDestr(symbol.bigIntValue);
    if (okay) {
      result = bigParseBased(symbol.striValue, (intType) base);
    } else {
      result = bigZero();
    } /* if */
    logFunction(printf("readBigBased -->\n"););
    return result;
  } /* readBigBased */



static inline void basedInteger (uintType base)

  { /* basedInteger */
    if (base < 2 || base > 36) {
      err_integer(BASE2TO36ALLOWED, (intType) base);
      base = 36; /* Avoid subsequent errors */
    } /* if */
    if (readBased()) {
      if (in_file.character == '_') {
        symbol.bigIntValue = readBigBased(base);
        symbol.sycategory = BIGINTLITERAL;
      } else {
        symbol.intValue = basedValue(base, symbol.name);
        symbol.sycategory = INTLITERAL;
      } /* if */
    } else {
      symbol.intValue = 0;
      symbol.sycategory = INTLITERAL;
    } /* if */
  } /* basedInteger */



static inline void intExponent (uintType *ivalue)

  {
    uintType uintValue;
    uintType exponent;

  /* intExponent */
    uintValue = *ivalue;
    in_file.character = next_character();
    if (in_file.character == '+') {
      in_file.character = next_character();
    } else {
      if (in_file.character == '-') {
        err_warning(NEGATIVEEXPONENT);
        uintValue = 0;
        in_file.character = next_character();
      } /* if */
    } /* if */
    if (char_class(in_file.character) == DIGITCHAR) {
      readDecimal(0);
      if (uintValue != 0) {
        exponent = decimalValue(symbol.name);
        while (exponent > 0) {
          exponent--;
          if (uintValue <= MAX_DIV_10) {
            uintValue *= 10;
          } else {
            err_num_stri(CARD_WITH_EXPONENT_TOO_BIG,
                0, (int) *ivalue, &symbol.name[0]);
            exponent = 0;
            uintValue = 0;
          } /* if */
        } /* while */
      } /* if */
    } else {
      err_cchar(DIGITEXPECTED, in_file.character);
      uintValue = 0;
    } /* if */
    *ivalue = uintValue;
  } /* intExponent */



static inline bigIntType readBigInteger (void)

  {
    memSizeType pos;
    bigIntType result;

  /* readBigInteger */
    in_file.character = next_character();
    pos = 0;
    do {
      while (pos != symbol.stri_max && symbol.name[pos] != '\0') {
        symbol.striValue->mem[pos] = (strElemType) symbol.name[pos];
        pos++;
      } /* while */
      check_stri_length(pos);
    } while (symbol.name[pos] != '\0');
    symbol.striValue->size = pos;
    bigDestr(symbol.bigIntValue);
    result = bigParse(symbol.striValue);
    return result;
  } /* readBigInteger */



static inline floatType readFloat (void)

  {
    register sySizeType position;
    floatType result;

  /* readFloat */
    logFunction(printf("readFloat\n"););
    position = strlen((cstriType) symbol.name);
    check_symb_length(position);
    symbol.name[position++] = (ucharType) in_file.character;
    in_file.character = next_character();
    if (char_class(in_file.character) == DIGITCHAR) {
      readDecimal(position);
      if (in_file.character == 'E' || in_file.character == 'e') {
        position += strlen((cstriType) &symbol.name[position]);
        check_symb_length(position);
        symbol.name[position++] = (ucharType) in_file.character;
        in_file.character = next_character();
        if (in_file.character == '+' || in_file.character == '-') {
          check_symb_length(position);
          symbol.name[position++] = (ucharType) in_file.character;
          in_file.character = next_character();
        } /* if */
        if (char_class(in_file.character) == DIGITCHAR) {
          readDecimal(position);
        } else {
          symbol.name[position] = '\0';
          err_cchar(DIGITEXPECTED, in_file.character);
        } /* if */
      } /* if */
#if ATOF_ACCEPTS_DENORMAL_NUMBERS
      result = (floatType) atof((cstriType) symbol.name);
#else
      result = (floatType) strtod((cstriType) symbol.name, NULL);
#endif
    } else {
      symbol.name[position] = '\0';
      err_cchar(DIGITEXPECTED, in_file.character);
      result = 0.0;
    } /* if */
    logFunction(printf("readFloat --> " FMT_E "\n", result););
    return result;
  } /* readFloat */



/**
 *  Read a numeric literal.
 *  The function assumes that the next character in in_file
 *  is the first character of the numeric literal.
 *  A numeric literal can be:
 *  - Decimal integer, e.g.: 12345
 *  - Decimal bigInteger, e.g.: 12345_
 *  - Based integer, e.g.: 16#fedcba
 *  - Based bigInteger, e.g.: 16#fedcba_
 *  - Decimal integer with exponent, e.g.: 123e3
 *  - Simple float, e.g.: 123.456
 *  - Float with exponent, e.g.: 12.3e-12
 */
void lit_number (void)

  {
    uintType number;

  /* lit_number */
    logFunction(printf("lit_number\n"););
    readDecimal(0);
    if (in_file.character == '.') {
#if WITH_FLOAT
      symbol.floatValue = readFloat();
#endif
      symbol.sycategory = FLOATLITERAL;
    } else if (in_file.character == '_') {
      symbol.bigIntValue = readBigInteger();
      symbol.sycategory = BIGINTLITERAL;
    } else {
      number = decimalValue(symbol.name);
      if (in_file.character == '#') {
        basedInteger(number);
      } else if (in_file.character == 'E' || in_file.character == 'e') {
        intExponent(&number);
        symbol.intValue = (intType) number;
        symbol.sycategory = INTLITERAL;
      } else {
        symbol.intValue = (intType) number;
        symbol.sycategory = INTLITERAL;
      } /* if */
    } /* if */
    find_literal_ident();
    symbol.syNumberInLine++;
#if WITH_STATISTIC
    literal_count++;
#endif
    logFunction(printf("lit_number -->\n"););
  } /* lit_number */
