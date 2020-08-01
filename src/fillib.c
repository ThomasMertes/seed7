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
/*  File: seed7/src/fillib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions for the primitive file type.     */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "sys/types.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "objutl.h"
#include "runerr.h"
#include "fil_rtl.h"
#include "fil_drv.h"

#undef EXTERN
#define EXTERN
#include "fillib.h"


#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif



/**
 *  Determine the size of a file and return it as bigInteger.
 *  The file length is measured in bytes.
 *  @return the size of the given file.
 *  @exception FILE_ERROR A system function returns an error or the
 *             file length reported by the system is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType fil_big_lng (listType arguments)

  { /* fil_big_lng */
    isit_file(arg_1(arguments));
    return bld_bigint_temp(
        filBigLng(take_file(arg_1(arguments))));
  } /* fil_big_lng */



/**
 *  Set the current file position.
 *  The file position is measured in bytes from the start of the file.
 *  The first byte in the file has the position 1.
 *  @exception RANGE_ERROR The file position is negative or zero or
 *             the file position is not representable in the system
 *             file position type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType fil_big_seek (listType arguments)

  { /* fil_big_seek */
    isit_file(arg_1(arguments));
    isit_bigint(arg_2(arguments));
    filBigSeek(take_file(arg_1(arguments)),
        take_bigint(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_big_seek */



/**
 *  Obtain the current file position.
 *  The file position is measured in bytes from the start of the file.
 *  The first byte in the file has the position 1.
 *  @return the current file position.
 *  @exception FILE_ERROR A system function returns an error or the
 *             file position reported by the system is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType fil_big_tell (listType arguments)

  { /* fil_big_tell */
    isit_file(arg_1(arguments));
    return bld_bigint_temp(
        filBigTell(take_file(arg_1(arguments))));
  } /* fil_big_tell */



/**
 *  Close a clib_file.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType fil_close (listType arguments)

  { /* fil_close */
    isit_file(arg_1(arguments));
    filClose(take_file(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_close */



objectType fil_cpy (listType arguments)

  {
    objectType file_variable;

  /* fil_cpy */
    file_variable = arg_1(arguments);
    isit_file(file_variable);
    is_variable(file_variable);
    isit_file(arg_3(arguments));
    file_variable->value.fileValue = take_file(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* fil_cpy */



objectType fil_create (listType arguments)

  { /* fil_create */
    isit_file(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), FILEOBJECT);
    arg_1(arguments)->value.fileValue = take_file(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* fil_create */



objectType fil_empty (listType arguments)

  { /* fil_empty */
    return bld_file_temp(NULL);
  } /* fil_empty */



/**
 *  Determine the end-of-file indicator.
 *  The end-of-file indicator is set when at least one request to read
 *  from the file failed.
 *  @return TRUE if the end-of-file indicator is set, FALSE otherwise.
 */
objectType fil_eof (listType arguments)

  { /* fil_eof */
    isit_file(arg_1(arguments));
    if (feof(take_file(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* fil_eof */



/**
 *  Check if two files are equal.
 *  @return TRUE if the two files are equal,
 *          FALSE otherwise.
 */
objectType fil_eq (listType arguments)

  { /* fil_eq */
    isit_file(arg_1(arguments));
    isit_file(arg_3(arguments));
    if (take_file(arg_1(arguments)) ==
        take_file(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* fil_eq */



objectType fil_err (listType arguments)

  { /* fil_err */
    return bld_file_temp(stderr);
  } /* fil_err */



/**
 *  Forces that all buffered data of 'outFile' is sent to its destination.
 *  This causes data to be sent to the file system of the OS.
 */
objectType fil_flush (listType arguments)

  { /* fil_flush */
    isit_file(arg_1(arguments));
    fflush(take_file(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_flush */



/**
 *  Read a character from a clib_file.
 *  @return the character read, or EOF at the end of the file.
 */
objectType fil_getc (listType arguments)

  { /* fil_getc */
    isit_file(arg_1(arguments));
    return bld_char_temp(
        filGetcChkCtrlC(take_file(arg_1(arguments))));
  } /* fil_getc */



/**
 *  Read a string with a maximum length from an clib_file.
 *  @return the string read.
 *  @exception RANGE_ERROR The length is negative.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType fil_gets (listType arguments)

  { /* fil_gets */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    return bld_stri_temp(
        filGetsChkCtrlC(take_file(arg_1(arguments)),
                        take_int(arg_2(arguments))));
  } /* fil_gets */



/**
 *  Determine if at least one character can be read successfully.
 *  This function allows a file to be handled like an iterator.
 *  @return FALSE if 'getc' would return EOF, TRUE otherwise.
 */
objectType fil_has_next (listType arguments)

  { /* fil_has_next */
    isit_file(arg_1(arguments));
    if (filHasNextChkCtrlC(take_file(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* fil_has_next */



objectType fil_in (listType arguments)

  { /* fil_in */
    return bld_file_temp(stdin);
  } /* fil_in */



objectType fil_input_ready (listType arguments)

  { /* fil_input_ready */
    isit_file(arg_1(arguments));
    if (filInputReady(take_file(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* fil_input_ready */



/**
 *  Read a line from a clib_file.
 *  The function accepts lines ending with "\n", "\r\n" or EOF.
 *  The line ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left the 2nd parameter (terminationChar)
 *  contains '\n' or EOF.
 *  @return the line read.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType fil_line_read (listType arguments)

  {
    objectType terminationChar;

  /* fil_line_read */
    isit_file(arg_1(arguments));
    terminationChar = arg_2(arguments);
    isit_char(terminationChar);
    is_variable(terminationChar);
    return bld_stri_temp(
        filLineReadChkCtrlC(take_file(arg_1(arguments)),
                            &terminationChar->value.charValue));
  } /* fil_line_read */



objectType fil_lit (listType arguments)

  { /* fil_lit */
    isit_file(arg_1(arguments));
    return bld_stri_temp(
        filLit(take_file(arg_1(arguments))));
  } /* fil_lit */



/**
 *  Obtain the length of a clib_file.
 *  The file length is measured in bytes.
 *  @return the size of the given file.
 *  @exception RANGE_ERROR The file length does not fit into
 *             an integer value.
 *  @exception FILE_ERROR A system function returns an error or the
 *             file length reported by the system is negative.
 */
objectType fil_lng (listType arguments)

  { /* fil_lng */
    isit_file(arg_1(arguments));
    return bld_int_temp(
        filLng(take_file(arg_1(arguments))));
  } /* fil_lng */



/**
 *  Check if two files are not equal.
 *  @return FALSE if both files are equal,
 *          TRUE otherwise.
 */
objectType fil_ne (listType arguments)

  { /* fil_ne */
    isit_file(arg_1(arguments));
    isit_file(arg_3(arguments));
    if (take_file(arg_1(arguments)) !=
        take_file(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* fil_ne */



/**
 *  Opens a file with the specified 'path' and 'mode'.
 *  There are text modes and binary modes:
 *  *Binary modes:
 *  ** "r"   Open file for reading.
 *  ** "w"   Truncate to zero length or create file for writing.
 *  ** "a"   Append; open or create file for writing at end-of-file.
 *  ** "r+"  Open file for update (reading and writing).
 *  ** "w+"  Truncate to zero length or create file for update.
 *  ** "a+"  Append; open or create file for update, writing at end-of-file.
 *  *Text modes:
 *  ** "rt"  Open file for reading.
 *  ** "wt"  Truncate to zero length or create file for writing.
 *  ** "at"  Append; open or create file for writing at end-of-file.
 *  ** "rt+" Open file for update (reading and writing).
 *  ** "wt+" Truncate to zero length or create file for update.
 *  ** "at+" Append; open or create file for update, writing at end-of-file.
 *  Note that this modes differ from the ones used by the C function
 *  fopen().
 *  @param path/arg_1 Path of the file to be opened. The path must
 *         use the standard path representation.
 *  @param mode/arg_2 Mode of the file to be opened.
 *  @return the file opened, or NULL if it could not be opened or
 *          when 'path' refers to a directory.
 *  @exception MEMORY_ERROR Not enough memory to convert the path
 *             to the system path type.
 *  @exception RANGE_ERROR The 'mode' is not one of the allowed
 *             values or 'path' does not use the standard path
 *             representation or 'path' cannot be converted
 *             to the system path type.
 */
objectType fil_open (listType arguments)

  { /* fil_open */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_file_temp(
        filOpen(take_stri(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* fil_open */



objectType fil_out (listType arguments)

  { /* fil_out */
    return bld_file_temp(stdout);
  } /* fil_out */



objectType fil_pclose (listType arguments)

  { /* fil_pclose */
    isit_file(arg_1(arguments));
    filPclose(take_file(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_pclose */



/**
 *  Open a pipe to a shell 'command', with 'parameters'.
 *  The pipe can be used to read, respectively write data
 *  with Latin-1 or UTF-8 encoding. Parameters which contain
 *  a space must be enclosed in double quotes. The commands
 *  supported and the format of the 'parameters' are not
 *  covered by the description of the 'fil_popen' function.
 *  Due to the usage of the operating system shell and external
 *  programs, it is hard to write portable programs, which use
 *  the 'fil_popen' function.
 *  @param command/arg_1 Name of the command to be executed. A path must
 *         use the standard path representation.
 *  @param parameters/arg_2 Space separated list of parameters for
 *         the 'command', or "" when there are no parameters.
 *  @param mode/arg_3 A pipe can only be opened with "r" (read) or "w" (write).
 *  @return the pipe file opened, or NULL if it could not be opened.
 *  @exception RANGE_ERROR An illegal mode was used.
 */
objectType fil_popen (listType arguments)

  { /* fil_popen */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_stri(arg_3(arguments));
    return bld_file_temp(
        filPopen(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)),
                 take_stri(arg_3(arguments))));
  } /* fil_popen */



objectType fil_print (listType arguments)

  { /* fil_print */
    isit_stri(arg_1(arguments));
    filPrint(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_print */



/**
 *  Set the current file position.
 *  The file position is measured in bytes from the start of the file.
 *  The first byte in the file has the position 1.
 *  @exception RANGE_ERROR The file position is negative or zero or
 *             the file position is not representable in the system
 *             file position type.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType fil_seek (listType arguments)

  { /* fil_seek */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    filSeek(take_file(arg_1(arguments)),
        take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_seek */



objectType fil_setbuf (listType arguments)

  { /* fil_setbuf */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    filSetbuf(take_file(arg_1(arguments)),
        take_int(arg_2(arguments)), take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_setbuf */



/**
 *  Obtain the current file position.
 *  The file position is measured in bytes from the start of the file.
 *  The first byte in the file has the position 1.
 *  @return the current file position.
 *  @exception RANGE_ERROR The file position does not fit into
 *             an integer value.
 *  @exception FILE_ERROR A system function returns an error or the
 *             file position reported by the system is negative.
 */
objectType fil_tell (listType arguments)

  { /* fil_tell */
    isit_file(arg_1(arguments));
    return bld_int_temp(
        filTell(take_file(arg_1(arguments))));
  } /* fil_tell */



objectType fil_value (listType arguments)

  {
    objectType obj_arg;

  /* fil_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != FILEOBJECT) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_file_temp(take_file(obj_arg));
    } /* if */
  } /* fil_value */



/**
 *  Read a word from a clib_file.
 *  Before reading the word it skips spaces and tabs. The function
 *  accepts words ending with " ", "\t", "\n", "\r\n" or EOF.
 *  The word ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left the 2nd parameter (terminationChar)
 *  contains ' ', '\t', '\n' or EOF.
 *  @return the word read.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 *  @exception FILE_ERROR A system function returns an error.
 */
objectType fil_word_read (listType arguments)

  {
    objectType terminationChar;

  /* fil_word_read */
    isit_file(arg_1(arguments));
    terminationChar = arg_2(arguments);
    isit_char(terminationChar);
    is_variable(terminationChar);
    return bld_stri_temp(
        filWordReadChkCtrlC(take_file(arg_1(arguments)),
                            &terminationChar->value.charValue));
  } /* fil_word_read */



/**
 *  Write a string to a clib_file.
 *  @exception FILE_ERROR A system function returns an error.
 *  @exception RANGE_ERROR The string contains a character that does
 *             not fit into a byte.
 */
objectType fil_write (listType arguments)

  { /* fil_write */
    isit_file(arg_1(arguments));
    isit_stri(arg_2(arguments));
    filWrite(take_file(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_write */
