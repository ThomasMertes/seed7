/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
#include "big_rtl.h"

#undef EXTERN
#define EXTERN
#include "numlit.h"

#undef TRACE_LITERAL



#ifdef ANSI_C

static void readdecimal (register unsigned int position)
#else

static void readdecimal (position)
register unsigned int position;
#endif

  {
    register int character;

  /* readdecimal */
#ifdef TRACE_LITERAL
    printf("BEGIN readdecimal\n");
#endif
    check_symb_length(position);
    symbol.name[position++] = (uchartype) in_file.character;
    while (char_class(character = next_character()) == DIGITCHAR) {
      check_symb_length(position);
      symbol.name[position++] = (uchartype) character;
    } /* while */
    symbol.name[position] = '\0';
    in_file.character = character;
#ifdef TRACE_LITERAL
    printf("END readdecimal\n");
#endif
  } /* readdecimal */



#ifdef ANSI_C

static inttype decimal_value (ustritype digits)
#else

static inttype decimal_value (digits)
ustritype digits;
#endif

  {
    inttype intvalue;
    unsigned int position;
    inttype digitval;
    booltype okay;

  /* decimal_value */
    okay = TRUE;
    intvalue = 0;
    position = 0;
    while (digits[position] != '\0') {
      digitval = ((inttype) digits[position]) - ((inttype) '0');
      if (intvalue < MAX_DIV_10 ||
          (intvalue == MAX_DIV_10 &&
          digitval <= MAX_REM_10)) {
        intvalue = ((inttype) 10) * intvalue + digitval;
      } else {
        okay = FALSE;
      } /* if */
      position++;
    } /* while */
    if (!okay) {
      err_string(CARD_DECIMAL_TOO_BIG, digits);
      intvalue = 0;
    } /* if */
    return(intvalue);
  } /* decimal_value */



#ifdef ANSI_C

static INLINE inttype based_value (inttype base, ustritype digits)
#else

static INLINE inttype based_value (base, digits)
inttype base;
ustritype digits;
#endif

  {
    inttype intvalue;
    unsigned int position;
    inttype div_base;
    inttype mod_base;
    inttype digitval;
    booltype okay;

  /* based_value */
    okay = TRUE;
    intvalue = 0;
    div_base = (inttype) (MAX_INTEGER / base);
    mod_base = (inttype) (MAX_INTEGER % base);
    position = 0;
    while (digits[position] != '\0') {
      digitval = digit_value[(int) digits[position]];
      if (digitval >= base) {
        if (okay) {
          err_num_stri(ILLEGALBASEDDIGIT,
              (int) digits[position], (int) base, digits);
          okay = FALSE;
        } /* if */
      } /* if */
      if (intvalue < div_base ||
          (intvalue == div_base &&
          digitval <= mod_base)) {
        intvalue = base * intvalue + digitval;
      } else {
        if (okay) {
          err_num_stri(CARD_BASED_TOO_BIG,
              0, (int) base, digits);
          okay = FALSE;
        } /* if */
      } /* if */
      position++;
    } /* while */
    if (!okay) {
      intvalue = 0;
    } /* if */
    return(intvalue);
  } /* based_value */



#ifdef ANSI_C

static INLINE booltype readbased (void)
#else

static INLINE booltype readbased ()
#endif

  {
    register unsigned int position;
    register int character;
    booltype okay;

  /* readbased */
    position = 0;
    character = next_character();
    if (char_class(character) == DIGITCHAR ||
        char_class(character) == LETTERCHAR) {
      do {
        check_symb_length(position);
        symbol.name[position++] = (uchartype) character;
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
    return(okay);
  } /* readbased */



#ifdef ANSI_C

static INLINE void basedint (inttype *intvalue)
#else

static INLINE void basedint (intvalue)
inttype *intvalue;
#endif

  { /* basedint */
    if (*intvalue < 2 || *intvalue > 36) {
      err_integer(BASE2TO36ALLOWED, *intvalue);
    } /* if */
    if (readbased()) {
      *intvalue = based_value(*intvalue, symbol.name);
    } else {
      *intvalue = 0;
    } /* if */
  } /* basedint */



#ifdef ANSI_C

static INLINE void intexponent (inttype *ivalue)
#else

static INLINE void intexponent (ivalue)
inttype *ivalue;
#endif

  {
    inttype intvalue;
    inttype exponent;

  /* intexponent */
    intvalue = *ivalue;
    in_file.character = next_character();
    if (in_file.character == '+') {
      in_file.character = next_character();
    } else {
      if (in_file.character == '-') {
        err_warning(NEGATIVEEXPONENT);
        intvalue = 0;
        in_file.character = next_character();
      } /* if */
    } /* if */
    if (char_class(in_file.character) == DIGITCHAR) {
      readdecimal(0);
      if (intvalue != 0) {
        exponent = decimal_value(symbol.name);
        while (exponent > 0) {
          exponent--;
          if (intvalue <= MAX_DIV_10) {
            intvalue *= 10;
          } else {
            err_num_stri(CARD_WITH_EXPONENT_TOO_BIG,
                0, (int) *ivalue, &symbol.name[0]);
            exponent = 0;
            intvalue = 0;
          } /* if */
        } /* while */
      } /* if */
    } else {
      err_cchar(DIGITEXPECTED, in_file.character);
      intvalue = 0;
    } /* if */
    *ivalue = intvalue;
  } /* intexponent */



#ifdef ANSI_C

static INLINE inttype readinteger (void)
#else

static INLINE inttype readinteger ()
#endif

  {
    inttype result;

  /* readinteger */
    result = decimal_value(symbol.name);
    if (in_file.character == '#') {
      basedint(&result);
    } else if (in_file.character == 'E' || in_file.character == 'e') {
      intexponent(&result);
    } /* if */
    return(result);
  } /* readinteger */



#ifdef ANSI_C

static INLINE biginttype readbiginteger (void)
#else

static INLINE biginttype readbiginteger ()
#endif

  {
    memsizetype pos;
    biginttype result;

  /* readbiginteger */
    in_file.character = next_character();
    pos = 0;
    do {
      while (pos != symbol.stri_max && symbol.name[pos] != '\0') {
        symbol.strivalue->mem[pos] = (strelemtype) symbol.name[pos];
        pos++;
      } /* while */
      check_stri_length(pos);
    } while (symbol.name[pos] != '\0');
    symbol.strivalue->size = pos;
    bigDestr(symbol.bigintvalue);
    result = bigParse(symbol.strivalue);
    return(result);
  } /* readbiginteger */



#ifdef ANSI_C

static INLINE floattype readfloat (void)
#else

static INLINE floattype readfloat ()
#endif

  {
    register unsigned int position;
    floattype result;

  /* readfloat */
#ifdef TRACE_LITERAL
    printf("BEGIN readfloat\n");
#endif
    position = strlen((cstritype) symbol.name);
    check_symb_length(position);
    symbol.name[position++] = (uchartype) in_file.character;
    in_file.character = next_character();
    if (char_class(in_file.character) == DIGITCHAR) {
      readdecimal(position);
      if (in_file.character == 'E' || in_file.character == 'e') {
        position += strlen((cstritype) &symbol.name[position]);
        check_symb_length(position);
        symbol.name[position++] = (uchartype) in_file.character;
        in_file.character = next_character();
        if (in_file.character == '+' || in_file.character == '-') {
          check_symb_length(position);
          symbol.name[position++] = (uchartype) in_file.character;
          in_file.character = next_character();
        } /* if */
        if (char_class(in_file.character) == DIGITCHAR) {
          readdecimal(position);
        } else {
          err_cchar(DIGITEXPECTED, in_file.character);
        } /* if */
      } /* if */
      result = (floattype) atof((cstritype) symbol.name);
/*    result = (floattype) strtod((cstritype) symbol.name, NULL); */
      if (in_file.character == '\'') {
        in_file.character = next_character();
        if (char_class(in_file.character) == DIGITCHAR) {
          readdecimal(position);
        } else {
          err_cchar(DIGITEXPECTED, in_file.character);
        } /* if */
      } /* if */
    } else {
      result = (floattype) atof((cstritype) symbol.name);
/*    result = (floattype) strtod((cstritype) symbol.name, NULL); */
      if (in_file.character == '\'') {
        in_file.character = next_character();
        if (char_class(in_file.character) == DIGITCHAR) {
          readdecimal(position);
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
        readdecimal(position);
      } else {
        err_cchar(DIGITEXPECTED, in_file.character);
      } /* if */
    } /* if */
#ifdef TRACE_LITERAL
    printf("END readfloat\n");
#endif
    return(result);
  } /* readfloat */



#ifdef ANSI_C

void lit_number (void)
#else

void lit_number ()
#endif

  { /* lit_number */
#ifdef TRACE_LITERAL
    printf("BEGIN lit_number\n");
#endif
    readdecimal(0);
    if (in_file.character == '.') {
#ifdef WITH_FLOAT
      symbol.floatvalue = readfloat();
#endif
      find_literal_ident();
      symbol.sycategory = FLOATLITERAL;
    } else if (in_file.character == '_') {
      symbol.bigintvalue = readbiginteger();
      find_literal_ident();
      symbol.sycategory = BIGINTLITERAL;
    } else {
      symbol.intvalue = readinteger();
      find_literal_ident();
      symbol.sycategory = INTLITERAL;
#ifdef WITH_STATISTIC
      literal_count++;
#endif
    } /* if */
#ifdef TRACE_LITERAL
    printf("END lit_number\n");
#endif
  } /* lit_number */
