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
/*  Module: Library                                                 */
/*  File: seed7/src/kbdlib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do keyboard input.            */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "objutl.h"
#include "runerr.h"
#include "kbd_rtl.h"
#include "kbd_drv.h"
#include "con_drv.h"

#undef EXTERN
#define EXTERN
#include "kbdlib.h"



/**
 *  Read a char from keyboard, but don't wait if no key was pressed.
 *  @return the char read from the console keyboard file, or
 *          KEY_NONE if no char is available.
 */
objectType kbd_busy_getc (listType arguments)

  { /* kbd_busy_getc */
    if (kbdKeyPressed()) {
      return bld_char_temp(kbdGetc());
    } else {
      return bld_char_temp((charType) K_NONE);
    } /* if */
  } /* kbd_busy_getc */



/**
 *  Read a character from the console keyboard file.
 *  @return the character read.
 */
objectType kbd_getc (listType arguments)

  { /* kbd_getc */
    return bld_char_temp(kbdGetc());
  } /* kbd_getc */



/**
 *  Read a string with maximum length from the console keyboard file.
 *  @return the string read.
 */
objectType kbd_gets (listType arguments)

  { /* kbd_gets */
    isit_int(arg_2(arguments));
    return bld_stri_temp(
        kbdGets(take_int(arg_2(arguments))));
  } /* kbd_gets */



/**
 *  Determine if at least one character can be read without waiting.
 *  @return TRUE if a character is available at the console keyboard file
 *          FALSE otherwise.
 */
objectType kbd_keypressed (listType arguments)

  { /* kbd_keypressed */
    if (kbdKeyPressed()) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* kbd_keypressed */



/**
 *  Read a line from the console keyboard file.
 *  The function accepts lines ending with "\n", "\r\n" or EOF.
 *  The line ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left terminationChar/arg_2 contains '\n' or EOF.
 *  @return the line read.
 */
objectType kbd_line_read (listType arguments)

  {
    objectType ch_variable;

  /* kbd_line_read */
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return bld_stri_temp(
        kbdLineRead(&ch_variable->value.charValue));
  } /* kbd_line_read */



objectType kbd_raw_getc (listType arguments)

  { /* kbd_raw_read */
    return bld_char_temp(kbdRawGetc());
  } /* kbd_raw_read */



/**
 *  Read a word from the console keyboard file.
 *  Before reading the word it skips spaces and tabs. The function
 *  accepts words ending with " ", "\t", "\n", "\r\n" or EOF.
 *  The word ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left terminationChar/arg_2 contains ' ',
 *  '\t', '\n' or EOF.
 *  @return the word read.
 */
objectType kbd_word_read (listType arguments)

  {
    objectType ch_variable;

  /* kbd_word_read */
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return bld_stri_temp(
        kbdWordRead(&ch_variable->value.charValue));
  } /* kbd_word_read */
