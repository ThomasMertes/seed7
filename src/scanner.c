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
/*  File: seed7/src/scanner.c                                       */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Read the next symbol from the source file.             */
/*                                                                  */
/*  The scan_symbol is tuned for maximum performance.               */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

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
#include "option.h"

#undef EXTERN
#define EXTERN
#include "scanner.h"



#ifdef ANSI_C

static INLINE void scan_comment (void)
#else

static INLINE void scan_comment ()
#endif

  {
    register int character;
    linenumtype start_line;

  /* scan_comment */
#ifdef TRACE_SCANNER
    printf("BEGIN scan_comment\n");
#endif
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
/*        character = next_line(); */
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
      err_at_line(COMMENTOPEN, start_line);
    } else {
      in_file.character = next_character();
    } /* if */
#ifdef WITH_STATISTIC
    comment_count++;
#endif
#ifdef TRACE_SCANNER
    printf("END scan_comment\n");
#endif
  } /* scan_comment */



#ifdef ANSI_C

static INLINE void scan_line_comment (void)
#else

static INLINE void scan_line_comment ()
#endif

  {
    register int character;

  /* scan_line_comment */
#ifdef TRACE_SCANNER
    printf("BEGIN scan_line_comment\n");
#endif
    SKIP_TO_NL(character);
    in_file.character = character;
#ifdef WITH_STATISTIC
    comment_count++;
#endif
#ifdef TRACE_SCANNER
    printf("END scan_line_comment\n");
#endif
  } /* scan_line_comment */



#ifdef ANSI_C

void scan_byte_order_mark (void)
#else

void scan_byte_order_mark ()
#endif

  {
    chartype unicode_char;

  /* scan_byte_order_mark */
#ifdef TRACE_SCANNER
    printf("BEGIN scan_byte_order_mark\n");
#endif
    if ((in_file.character & 0xC0) == 0xC0) {
      unicode_char = utf8_char(in_file.character);
      if (unicode_char != 0xFEFF /* Byte-order mark */) {
        err_char(CHAR_ILLEGAL, unicode_char);
        while (char_class(in_file.character) == ILLEGALCHAR) {
          in_file.character = next_character();
        } /* while */
      } /* if */
    } /* if */
#ifdef TRACE_SCANNER
    printf("END scan_byte_order_mark\n");
#endif
  } /* scan_byte_order_mark */



#ifdef ANSI_C

static void scan_illegal (void)
#else

static void scan_illegal ()
#endif

  {
    chartype unicode_char;

  /* scan_illegal */
#ifdef TRACE_SCANNER
    printf("BEGIN scan_illegal\n");
#endif
    if ((in_file.character & 0xC0) == 0xC0) {
      unicode_char = utf8_char(in_file.character);
      err_char(CHAR_ILLEGAL, unicode_char);
      while (char_class(in_file.character) == ILLEGALCHAR) {
        in_file.character = next_character();
      } /* while */
    } else {
      err_cchar(CHAR_ILLEGAL, in_file.character);
      do {
        in_file.character = next_character();
      } while (char_class(in_file.character) == ILLEGALCHAR);
    } /* if */
#ifdef TRACE_SCANNER
    printf("END scan_illegal\n");
#endif
  } /* scan_illegal */



#ifdef ANSI_C

static INLINE void scan_eof (void)
#else

static INLINE void scan_eof ()
#endif

  { /* scan_eof */
#ifdef TRACE_SCANNER
    printf("BEGIN scan_eof\n");
#endif
    if (symbol.sycategory == STOPSYMBOL) {
      err_warning(EOF_ENCOUNTERED);
    } else {
      next_file();
      if (in_file.end_of_file) {
        strcpy((cstritype) symbol.name, "END OF FILE");
        in_file.character = EOF;
        find_eof_ident();
        symbol.sycategory = STOPSYMBOL;
        symbol.syNumberInLine++;
      } else {
        scan_symbol();
      } /* if */
    } /* if */
#ifdef TRACE_SCANNER
    printf("END scan_eof\n");
#endif
  } /* scan_eof */



#ifdef ANSI_C

void scan_symbol (void)
#else

void scan_symbol ()
#endif

  {                                                             /*  1.89%  1.94% */
    register int character;
    register unsigned int position;

  /* scan_symbol */
#ifdef TRACE_SCANNER
    printf("BEGIN scan_symbol\n");
#endif
    character = in_file.character;                              /*  0.51%  0.22% */
    if (character == ' ' || character == '\t') {                /*  0.88%  0.73% */
      SKIP_SPACE(character);                                    /*  1.73%  1.93% */
    } else if (character == '\n') {                             /*  0.88%  0.44% */
/*    character = next_line(); */
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
        symbol.name[0] = (uchartype) character;                 /*  0.24%  0.25% */
        position = 1;                                           /*  0.12%  0.12% */

        do {
          while (position != symbol.name_length &&
              name_character(character = next_character())) {   /*  2.87%  3.45% */
            symbol.name[position++] = (uchartype) character;    /*  2.31%  2.36% */
          } /* while */                                         /*  9.71% 11.78% */
          check_symb_length(position);
        } while (name_character(character));

#ifdef OUT_OF_ORDER
        while (name_character(character = next_character())) {  /*  2.87%  3.45% */
          check_symb_length(position);                          /*  1.85%  1.88% */
          symbol.name[position++] = (uchartype) character;      /*  2.31%  2.36% */
        } /* while */                                           /*  9.71% 11.78% */
#endif

        symbol.name[position] = '\0';                           /*  0.36%  0.37% */
        find_normal_ident(position);                            /*  0.24%  0.25% */
        in_file.character = character;                          /*  0.12%  0.12% */
        symbol.sycategory = NAMESYMBOL;                         /*  0.24%  0.25% */
        symbol.syNumberInLine++;
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
        symbol.name[0] = (uchartype) character;                 /*  0.16%  0.16% */
        if (op_character(character = next_character())) {       /*  1.26%  1.46% */
          position = 1;
          do {
            check_symb_length(position);
            symbol.name[position++] = (uchartype) character;
          } while (op_character(character = next_character())); /*  0.07%  0.03% */
          symbol.name[position] = '\0';
          find_normal_ident(position);
        } else {
          symbol.name[1] = '\0';                                /*  0.10%  0.10% */
          find_1_ch_ident((int) symbol.name[0]);                /*  0.49%  0.51% */
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
        symbol.name[0] = (uchartype) character;                 /*  0.05%  0.05% */
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
        /* ILLEGALCHAR */
        in_file.character = character;
        scan_illegal();
        scan_symbol();                                          /*  1.46%  1.28% */
        break;
    } /* switch */
#ifdef TRACE_SCANNER
    printf("END scan_symbol\n");
#endif
  } /* scan_symbol */
