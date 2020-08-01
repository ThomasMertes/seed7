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

#undef TRACE_LITERAL



static void readDecimal (register sySizeType position)

  {
    register int character;

  /* readDecimal */
#ifdef TRACE_LITERAL
    printf("BEGIN readDecimal\n");
#endif
    check_symb_length(position);
    symbol.name[position++] = (ucharType) in_file.character;
    while (char_class(character = next_character()) == DIGITCHAR) {
      check_symb_length(position);
      symbol.name[position++] = (ucharType) character;
    } /* while */
    symbol.name[position] = '\0';
    in_file.character = character;
#ifdef TRACE_LITERAL
    printf("END readDecimal\n");
#endif
  } /* readDecimal */



static intType decimalValue (const const_ustriType digits)

  {
    boolType tooBig = FALSE;
    unsigned int position = 0;
    uintType digitval;
    uintType uintValue = 0;

  /* decimalValue */
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
      err_string(CARD_DECIMAL_TOO_BIG, digits);
      uintValue = 0;
    } /* if */
    return (intType) uintValue;
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
    max_div_base = (uintType) INTTYPE_MAX / base;
    while (digits[position] != '\0') {
      digitval = (uintType) digit_value[(int) digits[position]];
      if (unlikely(digitval >= base)) {
        illegalDigit = TRUE;
      } else if (unlikely(uintValue > max_div_base)) {
        tooBig = TRUE;
      } else {
        uintValue = base * uintValue + digitval;
      } /* if */
      position++;
    } /* while */
    if (unlikely(illegalDigit)) {
      err_num_stri(ILLEGALBASEDDIGIT, (int) digits[position], (int) base, digits);
      uintValue = 0;
    } else if (unlikely(tooBig || uintValue > (uintType) INTTYPE_MAX)) {
      err_num_stri(CARD_BASED_TOO_BIG, 0, (int) base, digits);
      uintValue = 0;
    } /* if */
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



static inline bigIntType readBigBased (intType base)

  {
    memSizeType pos;
    boolType okay = TRUE;
    intType digitval;
    bigIntType result;

  /* readBigBased */
    in_file.character = next_character();
    pos = 0;
    do {
      while (pos != symbol.stri_max && symbol.name[pos] != '\0') {
        digitval = digit_value[(int) symbol.name[pos]];
        if (digitval >= base) {
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
      result = bigParseBased(symbol.striValue, base);
    } else {
      result = bigZero();
    } /* if */
    return result;
  } /* readBigBased */



static inline void basedInteger (intType intValue)

  { /* basedInteger */
    if (intValue < 2 || intValue > 36) {
      err_integer(BASE2TO36ALLOWED, intValue);
      intValue = 36; /* Avoid subsequent errors */
    } /* if */
    if (readBased()) {
      if (in_file.character == '_') {
        symbol.bigIntValue = readBigBased(intValue);
        symbol.sycategory = BIGINTLITERAL;
      } else {
        symbol.intValue = basedValue((uintType) intValue, symbol.name);
        symbol.sycategory = INTLITERAL;
      } /* if */
    } else {
      symbol.intValue = 0;
      symbol.sycategory = INTLITERAL;
    } /* if */
  } /* basedInteger */



static inline void intExponent (intType *ivalue)

  {
    intType intValue;
    intType exponent;

  /* intExponent */
    intValue = *ivalue;
    in_file.character = next_character();
    if (in_file.character == '+') {
      in_file.character = next_character();
    } else {
      if (in_file.character == '-') {
        err_warning(NEGATIVEEXPONENT);
        intValue = 0;
        in_file.character = next_character();
      } /* if */
    } /* if */
    if (char_class(in_file.character) == DIGITCHAR) {
      readDecimal(0);
      if (intValue != 0) {
        exponent = decimalValue(symbol.name);
        while (exponent > 0) {
          exponent--;
          if (intValue <= MAX_DIV_10) {
            intValue *= 10;
          } else {
            err_num_stri(CARD_WITH_EXPONENT_TOO_BIG,
                0, (int) *ivalue, &symbol.name[0]);
            exponent = 0;
            intValue = 0;
          } /* if */
        } /* while */
      } /* if */
    } else {
      err_cchar(DIGITEXPECTED, in_file.character);
      intValue = 0;
    } /* if */
    *ivalue = intValue;
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
#ifdef TRACE_LITERAL
    printf("BEGIN readFloat\n");
#endif
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
          err_cchar(DIGITEXPECTED, in_file.character);
        } /* if */
      } /* if */
      result = (floatType) atof((cstriType) symbol.name);
/*    result = (floatType) strtod((cstriType) symbol.name, NULL); */
      if (in_file.character == '\'') {
        in_file.character = next_character();
        if (char_class(in_file.character) == DIGITCHAR) {
          readDecimal(position);
        } else {
          err_cchar(DIGITEXPECTED, in_file.character);
        } /* if */
      } /* if */
    } else {
      result = (floatType) atof((cstriType) symbol.name);
/*    result = (floatType) strtod((cstriType) symbol.name, NULL); */
      if (in_file.character == '\'') {
        in_file.character = next_character();
        if (char_class(in_file.character) == DIGITCHAR) {
          readDecimal(position);
        } else {
          err_cchar(DIGITEXPECTED, in_file.character);
        } /* if */
      } else {
        err_cchar(DIGITEXPECTED, in_file.character);
      } /* if */
    } /* if */
    if (in_file.character == '~') {
      in_file.character = next_character();
    } /* if */
    if (in_file.character == 'E' || in_file.character == 'e') {
      in_file.character = next_character();
      if (in_file.character == '+' || in_file.character == '-') {
        in_file.character = next_character();
      } /* if */
      if (char_class(in_file.character) == DIGITCHAR) {
        readDecimal(position);
      } else {
        err_cchar(DIGITEXPECTED, in_file.character);
      } /* if */
    } /* if */
#ifdef TRACE_LITERAL
    printf("END readFloat\n");
#endif
    return result;
  } /* readFloat */



void lit_number (void)

  {
    intType number;

  /* lit_number */
#ifdef TRACE_LITERAL
    printf("BEGIN lit_number\n");
#endif
    readDecimal(0);
    if (in_file.character == '.') {
#ifdef WITH_FLOAT
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
        symbol.intValue = number;
        symbol.sycategory = INTLITERAL;
      } else {
        symbol.intValue = number;
        symbol.sycategory = INTLITERAL;
      } /* if */
    } /* if */
    find_literal_ident();
    symbol.syNumberInLine++;
#ifdef WITH_STATISTIC
    literal_count++;
#endif
#ifdef TRACE_LITERAL
    printf("END lit_number\n");
#endif
  } /* lit_number */
