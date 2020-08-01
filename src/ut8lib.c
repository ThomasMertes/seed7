/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/ut8lib.c                                        */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: All Primitive actions for the UTF-8 file type.         */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "objutl.h"
#include "runerr.h"
#include "ut8_rtl.h"

#undef EXTERN
#define EXTERN
#include "ut8lib.h"



/**
 *  Read a character from an UTF-8 file.
 *  @return the character read, or EOF at the end of the file.
 *  @exception RANGE_ERROR The file contains an illegal encoding.
 */
objectType ut8_getc (listType arguments)

  { /* ut8_getc */
    isit_file(arg_1(arguments));
    return bld_char_temp(
        ut8Getc(take_file(arg_1(arguments))));
  } /* ut8_getc */



/**
 *  Return a string read with a maximum length from an UTF-8 file.
 *  @return the string read.
 *  @exception RANGE_ERROR The length is negative or the file
 *             contains an illegal encoding.
 */
objectType ut8_gets (listType arguments)

  { /* ut8_gets */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    return bld_stri_temp(
        ut8Gets(take_file(arg_1(arguments)), take_int(arg_2(arguments))));
  } /* ut8_gets */



/**
 *  Read a line from an UTF-8 file.
 *  The function accepts lines ending with '\n', "\r\n" or EOF.
 *  The line ending characters are not copied into the string.
 *  That means that the '\r' of a "\r\n" sequence is silently removed.
 *  When the function is left the 2nd parameter (terminationChar)
 *  contains '\n' or EOF.
 *  @return the line read.
 *  @exception RANGE_ERROR The file contains an illegal encoding.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType ut8_line_read (listType arguments)

  {
    objectType terminationChar;

  /* ut8_line_read */
    isit_file(arg_1(arguments));
    terminationChar = arg_2(arguments);
    isit_char(terminationChar);
    is_variable(terminationChar);
    return bld_stri_temp(
        ut8LineRead(take_file(arg_1(arguments)),
                    &terminationChar->value.charValue));
  } /* ut8_line_read */



/**
 *  Set the current file position.
 *  The file position is measured in bytes from the start of the file.
 *  The first byte in the file has the position 1.
 *  If the file position would be in the middle of an UTF-8 encoded
 *  character the position is advanced to the beginning of the next
 *  UTF-8 character.
 *  @exception RANGE_ERROR The file position is negative or zero.
 *  @exception FILE_ERROR The system function returns an error.
 */
objectType ut8_seek (listType arguments)

  { /* ut8_seek */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    ut8Seek(take_file(arg_1(arguments)), take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* ut8_seek */



/**
 *  Read a word from an UTF-8 file.
 *  Before reading the word it skips spaces and tabs. The function
 *  accepts words ending with ' ', '\t', '\n', "\r\n" or EOF.
 *  The word ending characters are not copied into the string.
 *  That means that the '\r' of a "\r\n" sequence is silently removed.
 *  When the function is left the 2nd parameter (terminationChar)
 *  contains ' ', '\t', '\n' or EOF.
 *  @return the word read.
 *  @exception RANGE_ERROR The file contains an illegal encoding.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType ut8_word_read (listType arguments)

  {
    objectType terminationChar;

  /* ut8_word_read */
    isit_file(arg_1(arguments));
    terminationChar = arg_2(arguments);
    isit_char(terminationChar);
    is_variable(terminationChar);
    return bld_stri_temp(
        ut8WordRead(take_file(arg_1(arguments)),
                    &terminationChar->value.charValue));
  } /* ut8_word_read */



/**
 *  Write a string to an UTF-8 file.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType ut8_write (listType arguments)

  { /* ut8_write */
    isit_file(arg_1(arguments));
    isit_stri(arg_2(arguments));
    ut8Write(take_file(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* ut8_write */
