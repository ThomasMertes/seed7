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
/*  Module: General                                                 */
/*  File: seed7/src/chclsutl.c                                      */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Compute the type of a character very quickly.          */
/*                                                                  */
/*  The function init_chclass initializes several arrays to obtain  */
/*  the type of a character very quickly. The is.. macros from      */
/*  the c-library give not exact the results we need here. Our      */
/*  functions are also defined for EOF and need no bit operation    */
/*  with a mask. Our arrays contain direct boolean values, so only  */
/*  one array access is necessary and this brings speed. No joke:   */
/*  The character class is used in the scanner which consumes       */
/*  approx 40 to 50 % of analyze time. The arrays are accessed via  */
/*  macros defined in chclsutl.h. These macros use the EOF value    */
/*  because it can be (0) or (-1) (see K&R 1).                      */
/*  Don't fear the runtime penalty: The compiler can do the         */
/*  calculation with EOF at compile time.                           */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdio.h"

#include "common.h"

#undef EXTERN
#define EXTERN
#include "chclsutl.h"



void init_chclass (void)

  {
    int character;

  /* init_chclass */
    logFunction(printf("init_chclass\n"););
    for (character = 0; character <= 255; character++) {
      char_class(character) = ILLEGALCHAR;
      name_character(character) = FALSE;
      op_character(character) = FALSE;
      no_escape_char(character) = FALSE;
      std_comment_char(character) = TRUE;
    } /* for */

    for (character = '!'; character <= '/'; character++) {
      char_class(character) = SPECIALCHAR;
    } /* for */
    for (character = '0'; character <= '9'; character++) {
      char_class(character) = DIGITCHAR;
    } /* for */
    for (character = ':'; character <= '@'; character++) {
      char_class(character) = SPECIALCHAR;
    } /* for */
    for (character = 'A'; character <= 'Z'; character++) {
      char_class(character) = LETTERCHAR;
    } /* for */
    for (character = '['; character <= '`'; character++) {
      char_class(character) = SPECIALCHAR;
    } /* for */
    for (character = 'a'; character <= 'z'; character++) {
      char_class(character) = LETTERCHAR;
    } /* for */
    for (character = '{'; character <= '~'; character++) {
      char_class(character) = SPECIALCHAR;
    } /* for */

    char_class(' ') = SPACECHAR;
    char_class('_') = UNDERLINECHAR;
    char_class('#') = SHARPCHAR;
    char_class('\'') = APOSTROPHECHAR;
    char_class('\"') = QUOTATIONCHAR;
    char_class('(') = LEFTPARENCHAR;
    char_class(')') = PARENCHAR;
    char_class('[') = PARENCHAR;
    char_class(']') = PARENCHAR;
    char_class('{') = PARENCHAR;
    char_class('}') = PARENCHAR;
    char_class('\t') = SPACECHAR;
    char_class('\n') = NEWLINECHAR;
    char_class('\r') = SPACECHAR;
    char_class(EOF) = EOFCHAR;

    for (character = ' '; character <= '~'; character++) {
      if (char_class(character) == DIGITCHAR ||
          char_class(character) == LETTERCHAR) {
        name_character(character) = TRUE;
      } /* if */
      if (char_class(character) == SPECIALCHAR) {
        op_character(character) = TRUE;
      } /* if */
      no_escape_char(character) = TRUE;
    } /* for */

    name_character('_') = TRUE;
    name_character(EOF) = FALSE;

    op_character(EOF) = FALSE;

    no_escape_char('\\') = FALSE;
    no_escape_char('\"') = FALSE;
    no_escape_char(EOF) = FALSE;

    std_comment_char('(') = FALSE;
    std_comment_char('*') = FALSE;
    std_comment_char('\n') = FALSE;
    std_comment_char(EOF) = FALSE;

    for (character = '0'; character <= '9'; character++) {
      digit_value[character] = (uintType) character - (uintType) '0';
    } /* for */
    for (character = 'A'; character <= 'Z'; character++) {
      digit_value[character] = (uintType) 10 +
          (uintType) character - (uintType) 'A';
    } /* for */
    for (character = 'a'; character <= 'z'; character++) {
      digit_value[character] = (uintType) 10 +
          (uintType) character - (uintType) 'a';
    } /* for */
    logFunction(printf("init_chclass -->\n"););
  } /* init_chclass */
