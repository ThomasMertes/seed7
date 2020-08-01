/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2014  Thomas Mertes                        */
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
/*  File: seed7/src/literal.c                                       */
/*  Changes: 1990 - 1994, 2012 - 2014  Thomas Mertes                */
/*  Content: Read next char or string literal from the source file. */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"

#include "common.h"
#include "data.h"
#include "chclsutl.h"
#include "identutl.h"
#include "error.h"
#include "info.h"
#include "infile.h"
#include "symbol.h"
#include "numlit.h"
#include "stat.h"
#include "findid.h"

#undef EXTERN
#define EXTERN
#include "literal.h"



static char esc_tab[] = {
    /*"*/ '\"',   /*#*/ ' ',    /*$*/ ' ',    /*%*/ ' ',    /*&*/ ' ',
    /*'*/ '\'',   /*(*/ ' ',    /*)*/ ' ',    /***/ ' ',    /*+*/ ' ',
    /*,*/ ' ',    /*-*/ ' ',    /*.*/ ' ',  /* / */ ' ',    /*0*/ ' ',
    /*1*/ ' ',    /*2*/ ' ',    /*3*/ ' ',    /*4*/ ' ',    /*5*/ ' ',
    /*6*/ ' ',    /*7*/ ' ',    /*8*/ ' ',    /*9*/ ' ',    /*:*/ ' ',
    /*;*/ ' ',    /*<*/ ' ',    /*=*/ ' ',    /*>*/ ' ',    /*?*/ ' ',
    /*@*/ ' ',    /*A*/ '\001', /*B*/ '\002', /*C*/ '\003', /*D*/ '\004',
    /*E*/ '\005', /*F*/ '\006', /*G*/ '\007', /*H*/ '\010', /*I*/ '\011',
    /*J*/ '\012', /*K*/ '\013', /*L*/ '\014', /*M*/ '\015', /*N*/ '\016',
    /*O*/ '\017', /*P*/ '\020', /*Q*/ '\021', /*R*/ '\022', /*S*/ '\023',
    /*T*/ '\024', /*U*/ '\025', /*V*/ '\026', /*W*/ '\027', /*X*/ '\030',
    /*Y*/ '\031', /*Z*/ '\032', /*[*/ ' ',    /*\*/ '\\',   /*]*/ ' ',
    /*^*/ ' ',    /*_*/ ' ',    /*`*/ ' ',    /*a*/ '\007', /*b*/ '\010',
    /*c*/ ' ',    /*d*/ ' ',    /*e*/ '\033', /*f*/ '\014', /*g*/ ' ',
    /*h*/ ' ',    /*i*/ ' ',    /*j*/ ' ',    /*k*/ ' ',    /*l*/ ' ',
    /*m*/ ' ',    /*n*/ '\012', /*o*/ ' ',    /*p*/ ' ',    /*q*/ ' ',
    /*r*/ '\015', /*s*/ ' ',    /*t*/ '\011', /*u*/ ' ',    /*v*/ '\013' };



static unsigned int escape_sequence (unsigned int position)

  {
    register int character;

  /* escape_sequence */
    character = next_character();                               /*  0.05% */
    if (character == '\n') {
      do {
        SKIP_CR_SP(character);
        if (character == '#') {
          SKIP_TO_NL(character);
#ifdef WITH_STATISTIC
          comment_count++;
#endif
        } /* if */
        INCR_LINE_COUNT(in_file.line);
      } while (character == '\n');
      symbol.syNumberInLine = 0;
      if (character == '\\') {
        character = next_character();
      } else {
        err_cchar(BACKSLASHEXPECTED, character);
      } /* if */
    } else if (character == ' ' || character == '\t' ||
        character == '\r') {
      SKIP_CR_SP(character);
      if (character == '#') {
        SKIP_TO_NL(character);
#ifdef WITH_STATISTIC
        comment_count++;
#endif
      } /* if */
      while (character == '\n') {
        SKIP_CR_SP(character);
        if (character == '#') {
          SKIP_TO_NL(character);
#ifdef WITH_STATISTIC
          comment_count++;
#endif
        } /* if */
        INCR_LINE_COUNT(in_file.line);
        symbol.syNumberInLine = 0;
      } /* while */
      if (character == '\\') {
        character = next_character();
      } else {
        err_cchar(BACKSLASHEXPECTED, character);
      } /* if */
    } else if (char_class(character) == DIGITCHAR) {
      in_file.character = character;
      lit_number();
      if (symbol.sycategory != INTLITERAL) {
        err_string(CARD_EXPECTED, symbol.name);
        symbol.charvalue = ' ';
      } else {
        symbol.charvalue = (chartype) symbol.intvalue;
      } /* if */
      if (in_file.character != ';' && in_file.character != '\\') {
        character = in_file.character;
        err_cchar(WRONGNUMERICALESCAPE, character);
      } else {
        character = next_character();
      } /* if */
      check_stri_length(position);
      symbol.strivalue->mem[position++] = (strelemtype) symbol.charvalue;
    } else if (character != EOF) {
      if (character >= (int) '\"' && character <= (int) 'v') {
        symbol.charvalue = (chartype) esc_tab[character - '\"'];
        if (symbol.charvalue == ' ') {
          err_cchar(STRINGESCAPE, character);
        } /* if */
      } else {
        err_cchar(STRINGESCAPE, character);
        symbol.charvalue = ' ';
      } /* if */
      check_stri_length(position);
      symbol.strivalue->mem[position++] = (strelemtype) symbol.charvalue;
      character = next_character();
    } /* if */
    in_file.character = character;
    return position;
  } /* escape_sequence */



/**
 *  Read an UTF-8 character.
 *  The caller must assure that 'character' contains an
 *  UTF-8 start byte (range 192 to 255 (leading bits 11......).
 *  @param character UTF-8 start byte.
 *  @return an UTF-32 character.
 */
chartype utf8_char (register int character)

  {
    chartype result;

  /* utf8_char */
    if (character <= 0xDF) {
      /* character range 192 to 223 (leading bits 110.....) */
      result = (chartype) (character & 0x1F) << 6;
      character = next_character();
      if (character >= 0x80 && character <= 0xBF) {
        /* character range 128 to 191 (leading bits 10......) */
        result |= character & 0x3F;
        if (result <= 0x9F) {
          if (result <= 0x7F) {
            /* Overlong encodings are illegal */
            err_char(OVERLONG_UTF8_ENCODING, result);
          } else {
            /* Extended control codes from the C1 set are illegal */
            err_char(CHAR_ILLEGAL, result);
          } /* if */
        } else {
          /* correct encodings are in the range */
          /* 0xA0 to 0x07FF (160 to 2047)       */
        } /* if */
        in_file.character = next_character();
      } else {
        result = 0xC0 | result >> 6; /* Restore 8 bit char */
        err_char(SOLITARY_UTF8_START_BYTE, result);
        in_file.character = character;
      } /* if */
    } else if (character <= 0xEF) {
      /* character range 224 to 239 (leading bits 1110....) */
      result = (chartype) (character & 0x0F) << 12;
      character = next_character();
      if (character >= 0x80 && character <= 0xBF) {
        /* character range 128 to 191 (leading bits 10......) */
        result |= (chartype) (character & 0x3F) << 6;
        character = next_character();
        if (character >= 0x80 && character <= 0xBF) {
          result |= character & 0x3F;
          if (result <= 0x7FF) {
            /* Overlong encodings are illegal */
            err_char(OVERLONG_UTF8_ENCODING, result);
          } else if (result >= 0xD800 && result <= 0xDFFF) {
            /* UTF-16 surrogate characters are illegal */
            err_char(UTF16_SURROGATE_CHAR_FOUND, result);
          } else {
            /* correct encodings are in the range */
            /* 0x800 to 0xFFFF (2048 to 65535)    */
          } /* if */
          in_file.character = next_character();
        } else {
          err_cchar(UTF8_CONTINUATION_BYTE_EXPECTED, character);
          in_file.character = character;
          result = '?';
        } /* if */
      } else {
        result = 0xE0 | result >> 12; /* Restore 8 bit char */
        err_char(SOLITARY_UTF8_START_BYTE, result);
        in_file.character = character;
      } /* if */
    } else if (character <= 0xF7) {
      /* character range 240 to 247 (leading bits 11110...) */
      result = (chartype) (character & 0x07) << 18;
      character = next_character();
      if (character >= 0x80 && character <= 0xBF) {
        /* character range 128 to 191 (leading bits 10......) */
        result |= (chartype) (character & 0x3F) << 12;
        character = next_character();
        if (character >= 0x80 && character <= 0xBF) {
          result |= (chartype) (character & 0x3F) << 6;
          character = next_character();
          if (character >= 0x80 && character <= 0xBF) {
            result |= character & 0x3F;
            if (result <= 0xFFFF) {
              /* Overlong encodings are illegal */
              err_char(OVERLONG_UTF8_ENCODING, result);
            } else if (result > 0x10FFFF) {
              /* result 0x110000 to 0x1FFFFF (1114112 to 2097151) */
              err_char(CHAR_NOT_UNICODE, result);
            } else {
              /* correct encodings are in the range     */
              /* 0x10000 to 0x10FFFF (65536 to 1114111) */
            } /* if */
            in_file.character = next_character();
          } else {
            err_cchar(UTF8_CONTINUATION_BYTE_EXPECTED, character);
            in_file.character = character;
            result = '?';
          } /* if */
        } else {
          err_cchar(UTF8_CONTINUATION_BYTE_EXPECTED, character);
          in_file.character = character;
          result = '?';
        } /* if */
      } else {
        result = 0xF0 | result >> 18; /* Restore 8 bit char */
        err_char(SOLITARY_UTF8_START_BYTE, result);
        in_file.character = character;
      } /* if */
    } else if (character <= 0xFB) {
      /* character range 248 to 251 (leading bits 111110..) */
      result = (chartype) (character & 0x03) << 24;
      character = next_character();
      if (character >= 0x80 && character <= 0xBF) {
        /* character range 128 to 191 (leading bits 10......) */
        result |= (chartype) (character & 0x3F) << 18;
        character = next_character();
        if (character >= 0x80 && character <= 0xBF) {
          result |= (chartype) (character & 0x3F) << 12;
          character = next_character();
          if (character >= 0x80 && character <= 0xBF) {
            result |= (chartype) (character & 0x3F) << 6;
            character = next_character();
            if (character >= 0x80 && character <= 0xBF) {
              result |= character & 0x3F;
              if (result <= 0x1FFFFF) {
                /* Overlong encodings are illegal */
                err_char(OVERLONG_UTF8_ENCODING, result);
              } else {
                /* result range 0x200000 to 0x3FFFFFF (2097152 to 67108863) */
                err_char(CHAR_NOT_UNICODE, result);
              } /* if */
              in_file.character = next_character();
            } else {
              err_cchar(UTF8_CONTINUATION_BYTE_EXPECTED, character);
              in_file.character = character;
              result = '?';
            } /* if */
          } else {
            err_cchar(UTF8_CONTINUATION_BYTE_EXPECTED, character);
            in_file.character = character;
            result = '?';
          } /* if */
        } else {
          err_cchar(UTF8_CONTINUATION_BYTE_EXPECTED, character);
          in_file.character = character;
          result = '?';
        } /* if */
      } else {
        result = 0xF8 | result >> 24; /* Restore 8 bit char */
        err_char(SOLITARY_UTF8_START_BYTE, result);
        in_file.character = character;
      } /* if */
    } else { /* if (character <= 0xFF) */
      /* character range 252 to 255 (leading bits 111111..) */
      result = (chartype) (character & 0x03) << 30;
      character = next_character();
      if (character >= 0x80 && character <= 0xBF) {
        /* character range 128 to 191 (leading bits 10......) */
        result |= (chartype) (character & 0x3F) << 24;
        character = next_character();
        if (character >= 0x80 && character <= 0xBF) {
          result |= (chartype) (character & 0x3F) << 18;
          character = next_character();
          if (character >= 0x80 && character <= 0xBF) {
            result |= (chartype) (character & 0x3F) << 12;
            character = next_character();
            if (character >= 0x80 && character <= 0xBF) {
              result |= (chartype) (character & 0x3F) << 6;
              character = next_character();
              if (character >= 0x80 && character <= 0xBF) {
                result |= character & 0x3F;
                if (result <= 0x3FFFFFF) {
                  /* Overlong encodings are illegal */
                  err_char(OVERLONG_UTF8_ENCODING, result);
                } else {
                  /* result range 0x4000000 to 0xFFFFFFFF (67108864 to 4294967295) */
                  err_char(CHAR_NOT_UNICODE, result);
                } /* if */
                in_file.character = next_character();
              } else {
                err_cchar(UTF8_CONTINUATION_BYTE_EXPECTED, character);
                in_file.character = character;
                result = '?';
              } /* if */
            } else {
              err_cchar(UTF8_CONTINUATION_BYTE_EXPECTED, character);
              in_file.character = character;
              result = '?';
            } /* if */
          } else {
            err_cchar(UTF8_CONTINUATION_BYTE_EXPECTED, character);
            in_file.character = character;
            result = '?';
          } /* if */
        } else {
          err_cchar(UTF8_CONTINUATION_BYTE_EXPECTED, character);
          in_file.character = character;
          result = '?';
        } /* if */
      } else {
        result = 0xFC | result >> 30; /* Restore 8 bit char */
        err_char(SOLITARY_UTF8_START_BYTE, result);
        in_file.character = character;
      } /* if */
    } /* if */
    return result;
  } /* utf8_char */



#ifdef OUT_OF_ORDER
static char lit_escapechar (void)

  {
    char ch;

  /* lit_escapechar */
#ifdef TRACE_LITERAL
  printf("BEGIN lit_escapechar\n");
#endif
    if (in_file.character >= '\"' && in_file.character <= 'v') {
      ch = esc_tab[in_file.character - '\"'];
      if (ch != ' ') {
        in_file.character = next_character();
      } else {
        err_cchar(STRINGESCAPE, in_file.character);
      } /* if */
    } else {
      err_cchar(STRINGESCAPE, in_file.character);
      ch = ' ';
    } /* if */
#ifdef TRACE_LITERAL
  printf("END lit_escapechar\n");
#endif
    return ch;
  } /* lit_escapechar */
#endif



#ifdef OUT_OF_ORDER
static char lit_escapechar (void)

  {
    char ch;
    booltype read_next_character;

  /* lit_escapechar */
#ifdef TRACE_LITERAL
  printf("BEGIN lit_escapechar\n");
#endif
    read_next_character = TRUE;
    switch (in_file.character) {
      case 'a':  ch = '\007'; break; /* ALERT */
      case 'b':  ch = '\010'; break; /* BACKSPACE */
      case 't':  ch = '\011'; break; /* HORIZONTAL TAB */
      case 'n':  ch = '\012'; break; /* NEW LINE */
      case 'v':  ch = '\013'; break; /* VERTICAL TAB */
      case 'f':  ch = '\014'; break; /* FORM FEED */
      case 'r':  ch = '\015'; break; /* CARRIAGE RETURN */
      case '\"': ch = '\"';   break;
      case '\'': ch = '\'';   break;
      case '\\': ch = '\\';   break;
      case 'A':  ch = '\001'; break; /* SOH */
      case 'B':  ch = '\002'; break; /* STX */
      case 'C':  ch = '\003'; break; /* ETX */
      case 'D':  ch = '\004'; break; /* EOT */
      case 'E':  ch = '\005'; break; /* ENQ */
      case 'F':  ch = '\006'; break; /* ACK */
      case 'G':  ch = '\007'; break; /* BEL */
      case 'H':  ch = '\010'; break; /* BS  */
      case 'I':  ch = '\011'; break; /* HT  */
      case 'J':  ch = '\012'; break; /* LF  */
      case 'K':  ch = '\013'; break; /* VT  */
      case 'L':  ch = '\014'; break; /* FF  */
      case 'M':  ch = '\015'; break; /* CR  */
      case 'N':  ch = '\016'; break; /* SO  */
      case 'O':  ch = '\017'; break; /* SI  */
      case 'P':  ch = '\020'; break; /* DLE */
      case 'Q':  ch = '\021'; break; /* DC1 */
      case 'R':  ch = '\022'; break; /* DC2 */
      case 'S':  ch = '\023'; break; /* DC3 */
      case 'T':  ch = '\024'; break; /* DC4 */
      case 'U':  ch = '\025'; break; /* NAK */
      case 'V':  ch = '\026'; break; /* SYN */
      case 'W':  ch = '\027'; break; /* ETB */
      case 'X':  ch = '\030'; break; /* CAN */
      case 'Y':  ch = '\031'; break; /* EM  */
      case 'Z':  ch = '\032'; break; /* SUB */
      default:
        err_cchar(STRINGESCAPE, in_file.character);
        ch = ' ';
        read_next_character = FALSE;
        break;
    } /* switch */
    if (read_next_character) {
      in_file.character = next_character();
    } /* if */
#ifdef TRACE_LITERAL
  printf("END lit_escapechar\n");
#endif
    return ch;
  } /* lit_escapechar */
#endif



void lit_char (void)

  {
    unsigned int position;

  /* lit_char */
#ifdef TRACE_LITERAL
  printf("BEGIN lit_char\n");
#endif
    in_file.character = next_character();
    if (in_file.character == '\n' || in_file.character == '\r' ||
        in_file.character == EOF) {
      symbol.charvalue = ' ';
      err_warning(CHAREXCEEDS);
    } else {
      if (in_file.character == '\\') {
        do {
          position = escape_sequence(0);
        } while (position == 0 && in_file.character == '\\');
      } else {
        position = 0;
      } /* if */
      if (position == 0) {
        if (in_file.character >= (int) ' ' && in_file.character <= (int) '~') {
          symbol.charvalue = (chartype) in_file.character;
          in_file.character = next_character();
        } else if (in_file.character >= 0xC0 && in_file.character <= 0xFF) {
          /* character range 192 to 255 (leading bits 11......) */
          symbol.charvalue = utf8_char(in_file.character);
        } else {
          if (in_file.character >= 0x80 && in_file.character <= 0xBF) {
            /* character range 128 to 191 (leading bits 10......) */
            err_cchar(UNEXPECTED_UTF8_CONTINUATION_BYTE, in_file.character);
          } else {
            err_cchar(CHAR_ILLEGAL, in_file.character);
          } /* if */
          symbol.charvalue = ' ';
          in_file.character = next_character();
        } /* if */
      } /* if */
      if (in_file.character != '\'') {
        if (in_file.character == '\n' || in_file.character == '\r' ||
            in_file.character == EOF) {
          err_warning(CHAREXCEEDS);
        } else {
          err_cchar(APOSTROPHEXPECTED, in_file.character);
          do {
            in_file.character = next_character();
          } while (in_file.character != '\'' &&
              in_file.character != '\n' &&
              in_file.character != '\r' &&
              in_file.character != EOF);
          if (in_file.character == '\'') {
            in_file.character = next_character();
          } /* if */
        } /* if */
      } else {
        in_file.character = next_character();
      } /* if */
    } /* if */
    find_literal_ident();
    symbol.sycategory = CHARLITERAL;
    symbol.syNumberInLine++;
#ifdef WITH_STATISTIC
    literal_count++;
#endif
#ifdef TRACE_LITERAL
  printf("END lit_char\n");
#endif
  } /* lit_char */



void lit_string (void)

  {                                                             /*  0.05% */
    register int character;
    register unsigned int position;
    booltype reading_string;

  /* lit_string */
#ifdef TRACE_LITERAL
  printf("BEGIN lit_string\n");
#endif
    reading_string = TRUE;                                      /*  0.01% */
    position = 0;                                               /*  0.01% */
    character = next_character();                               /*  0.18% */
    do {

      do {
        while (position != symbol.stri_max &&
            no_escape_char(character)) {                        /*  0.07% */
          symbol.strivalue->mem[position++] =
              (strelemtype) character;                          /*  1.46% */
          character = next_character();                         /*  5.56% */
        } /* while */                                           /*  1.76% */
        check_stri_length(position);
      } while (no_escape_char(character));

#ifdef OUT_OF_ORDER
      while (no_escape_char(character)) {                       /*  0.07% */
        check_stri_length(position);                            /*  1.17% */
        symbol.strivalue->mem[position++] =
            (strelemtype) character;                            /*  1.46% */
        character = next_character();                           /*  5.56% */
      } /* while */                                             /*  1.76% */
#endif

      if (character == '\"') {
        character = next_character();                           /*  0.17% */
        if (character == '\"') {
          err_warning(WRONG_QUOTATION_REPRESENTATION);
          check_stri_length(position);
          symbol.strivalue->mem[position++] = (strelemtype) character;
          character = next_character();                         /*  0.02% */
        } else {
          reading_string = FALSE;
        } /* if */
      } else if (character == '\\') {
        position = escape_sequence(position);
        character = in_file.character;
      } else if (character >= 0xC0 && character <= 0xFF) {
        /* character range 192 to 255 (leading bits 11......) */
        check_stri_length(position);
        symbol.strivalue->mem[position++] = utf8_char(character);
        character = in_file.character;
      } else if (character == '\n' || character == '\r' ||
          character == EOF) {
        err_warning(STRINGEXCEEDS);
        reading_string = FALSE;
      } else {
        if (character >= 0x80 && character <= 0xBF) {
          /* character range 128 to 191 (leading bits 10......) */
          err_cchar(UNEXPECTED_UTF8_CONTINUATION_BYTE, character);
        } else {
          err_cchar(CHAR_ILLEGAL, character);
        } /* if */
        do {
          check_stri_length(position);
          symbol.strivalue->mem[position++] = (strelemtype) character;
          character = next_character();                         /*  0.06% */
        } while (char_class(character) == ILLEGALCHAR);
      } /* if */                                                /*  0.14% */
    } while (reading_string);                                   /*  0.02% */
    find_literal_ident();                                       /*  0.01% */
    in_file.character = character;                              /*  0.01% */
    symbol.sycategory = STRILITERAL;                            /*  0.01% */
    symbol.syNumberInLine++;
    symbol.strivalue->size = position;                          /*  0.01% */
#ifdef WITH_STATISTIC
    literal_count++;
#endif
#ifdef TRACE_LITERAL
  printf("END lit_string\n");
#endif
  } /* lit_string */                                            /*  0.04% */



#ifdef OUT_OF_ORDER
static void lit_text (void)

  {
    register int character;
    register sysizetype position;
    register booltype reading_string;

  /* lit_text */
    reading_string = TRUE;
    position = 0;
    character = next_character();
    do {
      while (character != '\n' && character != '\r' &&
          character != EOF) {
        check_symb_length(position);
        symbol.name[position++] = (uchartype) character;
        character = next_character();
      } /* while */
      if (character == '\n') {
        character = next_line();
        if (character == '#') {
          reading_string = FALSE;
        } /* if */
      } else if (character == '\r') {
        character = next_character();
        if (character == '\n') {
          character = next_line();
          if (character == '#') {
            reading_string = FALSE;
          } /* if */
        } /* if */
      } else {
        err_warning(STRINGEXCEEDS);
        reading_string = FALSE;
      } /* if */
      check_symb_length(position);
      symbol.name[position++] = '\n';
    } while (reading_string);
    symbol.name[position] = '\0';
    in_file.character = character;
    find_literal_ident();
    symbol.sycategory = STRILITERAL;
    symbol.syNumberInLine++;
#ifdef WITH_STATISTIC
    literal_count++;
#endif
  } /* lit_text */
#endif
