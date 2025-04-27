/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2014, 2015, 2024, 2025  Thomas Mertes      */
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
/*  File: seed7/src/scanner.c                                       */
/*  Changes: 1990, 1991, 1992, 1993, 1994, 2014  Thomas Mertes      */
/*           2015, 2024, 2025  Thomas Mertes                        */
/*  Content: Read the next symbol from the source file.             */
/*                                                                  */
/*  The scan_symbol is tuned for maximum performance.               */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "chclsutl.h"
#include "identutl.h"
#include "striutl.h"
#include "traceutl.h"
#include "literal.h"
#include "numlit.h"
#include "info.h"
#include "infile.h"
#include "symbol.h"
#include "error.h"
#include "stat.h"
#include "findid.h"
#include "chr_rtl.h"

#undef EXTERN
#define EXTERN
#include "scanner.h"



static void scan_comment (void)

  {
    register int character;
    lineNumType start_line;

  /* scan_comment */
    logFunction(printf("scan_comment\n"););
    start_line = in_file.line;
    character = next_character();
    do {
      do {
        while (std_comment_char(character)) {
          character = next_character();
        } /* while */
        if (character == '(') {
          character = next_character();
          if (character == '*') {
            scan_comment();
            character = in_file.character;
          } /* if */
        } else if (character == '\n') {
          SKIP_CR_SP(character);
          INCR_LINE_COUNT(in_file.line);
          symbol.syNumberInLine = 0;
        } /* if */
      } while (character != '*' && character != EOF);
      if (character != EOF) {
        character = next_character();
      } /* if */
    } while (character != ')' && character != EOF);
    if (character == EOF) {
      in_file.character = EOF;
      err_at_line(UNCLOSED_COMMENT, start_line);
    } else {
      in_file.character = next_character();
    } /* if */
#if WITH_STATISTIC
    comment_count++;
#endif
    logFunction(printf("scan_comment -->\n"););
  } /* scan_comment */



static inline void scan_line_comment (void)

  {
    register int character;

  /* scan_line_comment */
    logFunction(printf("scan_line_comment\n"););
    SKIP_TO_NL(character);
    in_file.character = character;
#if WITH_STATISTIC
    comment_count++;
#endif
    logFunction(printf("scan_line_comment -->\n"););
  } /* scan_line_comment */



/**
 *  Accept an UTF-8 byte order mark at the beginning of a file.
 *  UTF-16 byte order marks cause an error and the whole file is
 *  skipped.
 */
void scan_byte_order_mark (void)

  {
    charType unicode_char;

  /* scan_byte_order_mark */
    logFunction(printf("scan_byte_order_mark\n"););
    if (in_file.character == 0xEF) {
      /* Start of UTF-8 BOM */
      unicode_char = utf8_char(in_file.character);
      if (unicode_char != 0xFEFF /* Byte-order mark */) {
        err_char(CHAR_ILLEGAL, unicode_char);
        while (char_class(in_file.character) == ILLEGALCHAR) {
          in_file.character = next_character();
        } /* while */
      } /* if */
    } else if (in_file.character == 0xFE || in_file.character == 0xFF) {
      /* Start of UTF-16 BOM */
      unicode_char = (charType) in_file.character << CHAR_BIT;
      in_file.character = next_character();
      unicode_char |= (charType) in_file.character;
      if (unicode_char == 0xFEFF || unicode_char == 0xFFFE) {
        err_char(UTF16_BYTE_ORDER_MARK_FOUND, unicode_char);
        while (char_class(in_file.character) != EOFCHAR) {
          in_file.character = next_character();
        } /* while */
      } else {
        err_char(CHAR_ILLEGAL, unicode_char >> CHAR_BIT);
        while (char_class(in_file.character) == ILLEGALCHAR) {
          in_file.character = next_character();
        } /* while */
      } /* if */
    } /* if */
    logFunction(printf("scan_byte_order_mark -->\n"););
  } /* scan_byte_order_mark */



static void scan_illegal (void)

  {
    charType unicode_char;

  /* scan_illegal */
    logFunction(printf("scan_illegal\n"););
    if (in_file.character >= 0xC0 && in_file.character <= 0xFF) {
      /* character range 192 to 255 (leading bits 11......) */
      unicode_char = utf8_char(in_file.character);
      err_char(CHAR_ILLEGAL, unicode_char);
      while (char_class(in_file.character) == ILLEGALCHAR) {
        in_file.character = next_character();
      } /* while */
    } else {
      err_cchar(CHAR_ILLEGAL, in_file.character);
      do {
        in_file.character = next_character();
      } while (char_class(in_file.character) == ILLEGALCHAR &&
               (!symbol.unicodeNames || in_file.character < 0xC0));
    } /* if */
    logFunction(printf("scan_illegal -->\n"););
  } /* scan_illegal */



static inline void scan_eof (void)

  {
    long eof_position;

  /* scan_eof */
    logFunction(printf("scan_eof\n"););
    eof_position = IN_FILE_TELL();
    if (eof_position > 0) {
      IN_FILE_SEEK(eof_position - 1);
      /* Check if EOF was preceded by a newline. */
      if (next_character() == '\n') {
        /* Set in_file.line to the actual lines in the file. */
        in_file.line--;
      } /* if */
      /* Read EOF again. */
      next_character();
    } /* if */
    if (symbol.sycategory == STOPSYMBOL) {
      err_warning(EOF_ENCOUNTERED);
    } else {
      next_file();
      if (in_file.end_of_file) {
        strcpy((cstriType) symbol.name, "END OF FILE");
        in_file.character = EOF;
        find_eof_ident();
        symbol.sycategory = STOPSYMBOL;
        symbol.syNumberInLine++;
      } else {
        scan_symbol();
      } /* if */
    } /* if */
    logFunction(printf("scan_eof --> %d\n", symbol.sycategory););
  } /* scan_eof */



static void scan_symbol_utf8 (int character, register sySizeType position)

  {
    charType unicode_char;
    boolType reading_symbol = TRUE;

  /* scan_symbol_utf8 */
    logFunction(printf("scan_symbol_utf8\n"););
    unicode_char = utf8_char(character);
    if (chrIsLetter(unicode_char)) {
      check_symb_length_delta(position, symbol.utf8_length);
      memcpy(&symbol.name[position], symbol.utf8_repr, symbol.utf8_length);
      position += symbol.utf8_length;
      character = in_file.character;
      do {
        if (character >= 0xC0 && character <= 0xFF) {
          unicode_char = utf8_char(character);
          if (chrIsLetter(unicode_char)) {
            check_symb_length_delta(position, symbol.utf8_length);
            memcpy(&symbol.name[position], symbol.utf8_repr, symbol.utf8_length);
            position += symbol.utf8_length;
            character = in_file.character;
          } else {
            reading_symbol = FALSE;
          } /* if */
        } else {
          if (name_character(character)) {
            check_symb_length(position);
            symbol.name[position++] = (ucharType) character;
            character = next_character();
          } else {
            reading_symbol = FALSE;
            in_file.character = character;
            unicode_char = 0;
          } /* if */
        } /* if */
      } while (reading_symbol);
      symbol.name[position] = '\0';
      find_normal_ident(position);
      symbol.sycategory = NAMESYMBOL;
      symbol.syNumberInLine++;
    } else if (position != 0) {
      symbol.name[position] = '\0';
      find_normal_ident(position);
      in_file.character = character;
      symbol.sycategory = NAMESYMBOL;
      symbol.syNumberInLine++;
    } /* if */
    if (unicode_char != 0) {
      /* ILLEGALCHAR */
      err_char(CHAR_ILLEGAL, unicode_char);
      while (char_class(in_file.character) == ILLEGALCHAR &&
             character < 0xC0) {
        in_file.character = next_character();
      } /* while */
    } /* if */
    logFunction(printf("scan_symbol_utf8 -->\n"););
  } /* scan_symbol_utf8 */



void scan_symbol (void)

  {                                                             /*  1.89%  1.94% */
    register int character;
    register sySizeType position;

  /* scan_symbol */
    logFunction(printf("scan_symbol\n"););
    character = in_file.character;                              /*  0.51%  0.22% */
    if (character == ' ' || character == '\t') {                /*  0.88%  0.73% */
      SKIP_SPACE(character);                                    /*  1.73%  1.93% */
    } else if (character == '\n') {                             /*  0.88%  0.44% */
      SKIP_CR_SP(character);                                    /*  6.43%  8.02% */
      INCR_LINE_COUNT(in_file.line);                            /*  0.26%  0.28% */
      symbol.syNumberInLine = 0;
    } /* if */
    switch (character) {                                        /*  2.87%  2.55% */
      case 'A':  case 'B':  case 'C':  case 'D':  case 'E':
      case 'F':  case 'G':  case 'H':  case 'I':  case 'J':
      case 'K':  case 'L':  case 'M':  case 'N':  case 'O':
      case 'P':  case 'Q':  case 'R':  case 'S':  case 'T':
      case 'U':  case 'V':  case 'W':  case 'X':  case 'Y':
      case 'Z':
      case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
      case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
      case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
      case 'p':  case 'q':  case 'r':  case 's':  case 't':
      case 'u':  case 'v':  case 'w':  case 'x':  case 'y':
      case 'z':
      case '_':
        /* LETTERCHAR, UNDERLINECHAR */
        symbol.name[0] = (ucharType) character;                 /*  0.24%  0.25% */
        position = 1;                                           /*  0.12%  0.12% */

        do {
          while (position != symbol.name_length &&
              name_character(character = next_character())) {   /*  2.87%  3.45% */
            symbol.name[position++] = (ucharType) character;    /*  2.31%  2.36% */
          } /* while */                                         /*  9.71% 11.78% */
          check_symb_length(position);
        } while (name_character(character));

        if (symbol.unicodeNames &&
            character >= 0xC0 && character <= 0xFF) {
          /* character range 192 to 255 (leading bits 11......) */
          scan_symbol_utf8(character, position);
        } else {
          symbol.name[position] = '\0';                         /*  0.36%  0.37% */
          find_normal_ident(position);                          /*  0.24%  0.25% */
          in_file.character = character;                        /*  0.12%  0.12% */
          symbol.sycategory = NAMESYMBOL;                       /*  0.24%  0.25% */
          symbol.syNumberInLine++;
        } /* if */
        break;                                                  /*  0.12%  0.12% */
      case ' ':  case '\t':  case '\r':
        /* SPACECHAR */
        in_file.character = next_character();                   /*  0.87%        */
        scan_symbol();
        break;
      case '\n':
        /* NEWLINECHAR */
        in_file.character = character;
        scan_symbol();
        break;
      case '!':  case '$':  case '%':  case '&':  case '*':
      case '+':  case ',':  case '-':  case '.':  case '/':
      case ':':  case ';':  case '<':  case '=':  case '>':
      case '?':  case '@':  case '\\': case '^':  case '`':
      case '|':  case '~':
        /* SPECIALCHAR */
        symbol.name[0] = (ucharType) character;                 /*  0.16%  0.16% */
        if (op_character(character = next_character())) {       /*  1.26%  1.46% */
          position = 1;
          do {
            check_symb_length(position);
            symbol.name[position++] = (ucharType) character;
          } while (op_character(character = next_character())); /*  0.07%  0.03% */
          symbol.name[position] = '\0';
          find_normal_ident(position);
        } else {
          symbol.name[1] = '\0';                                /*  0.10%  0.10% */
          find_1_ch_ident(symbol.name[0]);                      /*  0.49%  0.51% */
        } /* if */
        in_file.character = character;                          /*  0.05%  0.05% */
        symbol.sycategory = SPECIALSYMBOL;                      /*  0.10%  0.10% */
        symbol.syNumberInLine++;
        break;                                                  /*  0.05%  0.10% */
      case '(':
        /* LEFTPARENCHAR */
        if ((in_file.character = next_character()) == '*') {    /*  0.32%  0.32% */
          scan_comment();
          scan_symbol();
        } else {
          symbol.name[0] = '(';                                 /*  0.04%  0.04% */
/*        symbol.name[1] = '\0';                                    0.03%  0.03% */
          find_1_ch_ident('(');                                 /*  0.04%  0.09% */
          symbol.sycategory = PARENSYMBOL;                      /*  0.03%  0.03% */
          symbol.syNumberInLine++;
        } /* if */
        break;                                                  /*         0.01% */
      case ')':
      case '[':  case ']':
      case '{':  case '}':
        /* PARENCHAR */
        symbol.name[0] = (ucharType) character;                 /*  0.05%  0.05% */
/*      symbol.name[1] = '\0';                                      0.05%  0.05% */
        find_1_ch_ident(character);                             /*  0.08%  0.11% */
        in_file.character = next_character();                   /*  0.32%  0.34% */
        symbol.sycategory = PARENSYMBOL;                        /*  0.03%  0.03% */
        symbol.syNumberInLine++;
        break;                                                  /*         0.02% */
      case '0':  case '1':  case '2':  case '3':  case '4':
      case '5':  case '6':  case '7':  case '8':  case '9':
        /* DIGITCHAR */
        in_file.character = character;
        lit_number();
        break;
      case '\'':
        /* APOSTROPHECHAR */
        lit_char();
        break;
      case '\"':
        /* QUOTATIONCHAR */
        lit_string();                                           /*         0.02% */
        break;                                                  /*         0.02% */
      case '#':
        /* SHARPCHAR */
        scan_line_comment();
        scan_symbol();
        break;
      case EOF:
        /* EOFCHAR */
        scan_eof();
        break;
      default:
        if (symbol.unicodeNames &&
            character >= 0xC0 && character <= 0xFF) {
          /* character range 192 to 255 (leading bits 11......) */
          scan_symbol_utf8(character, 0);
        } else {
          /* ILLEGALCHAR */
          in_file.character = character;
          scan_illegal();
          scan_symbol();                                        /*  1.46%  1.28% */
        } /* if */
        break;
    } /* switch */
    logFunction(printf("scan_symbol --> %d\n", symbol.sycategory););
  } /* scan_symbol */



static void skip_whitespace (void)

  {
    int character;

  /* skip_whitespace */
    character = in_file.character;
    if (character == ' ' || character == '\t') {
      SKIP_SPACE(character);
    } else if (character == '\n') {
      SKIP_CR_SP(character);
      INCR_LINE_COUNT(in_file.line);
      symbol.syNumberInLine = 0;
    } /* if */
    in_file.character = character;
  } /* skip_whitespace */



void skip_char (int character)

  { /* skip_char */
    skip_whitespace();
    if (in_file.character == character) {
      scan_symbol();
    } /* if */
  } /* skip_char */



void skip_past_char (int character)

  { /* skip_past_char */
    skip_whitespace();
    if (in_file.character == character) {
      scan_symbol();
      scan_symbol();
    } /* if */
  } /* skip_past_char */
