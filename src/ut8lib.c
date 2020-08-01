/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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



#ifdef ANSI_C

objecttype ut8_getc (listtype arguments)
#else

objecttype ut8_getc (arguments)
listtype arguments;
#endif

  { /* ut8_getc */
    isit_file(arg_1(arguments));
    return(bld_char_temp(
        ut8Getc(take_file(arg_1(arguments)))));
  } /* ut8_getc */



#ifdef ANSI_C

objecttype ut8_gets (listtype arguments)
#else

objecttype ut8_gets (arguments)
listtype arguments;
#endif

  { /* ut8_gets */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    return(bld_stri_temp(
        ut8Gets(take_file(arg_1(arguments)), take_int(arg_2(arguments)))));
  } /* ut8_gets */



#ifdef ANSI_C

objecttype ut8_line_read (listtype arguments)
#else

objecttype ut8_line_read (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* ut8_line_read */
    isit_file(arg_1(arguments));
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return(bld_stri_temp(
        ut8LineRead(take_file(arg_1(arguments)), &ch_variable->value.charvalue)));
  } /* ut8_line_read */



#ifdef ANSI_C

objecttype ut8_seek (listtype arguments)
#else

objecttype ut8_seek (arguments)
listtype arguments;
#endif

  { /* ut8_seek */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    ut8Seek(take_file(arg_1(arguments)), take_int(arg_2(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* ut8_seek */



#ifdef ANSI_C

objecttype ut8_word_read (listtype arguments)
#else

objecttype ut8_word_read (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* ut8_word_read */
    isit_file(arg_1(arguments));
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return(bld_stri_temp(
        ut8WordRead(take_file(arg_1(arguments)), &ch_variable->value.charvalue)));
  } /* ut8_word_read */



#ifdef ANSI_C

objecttype ut8_write (listtype arguments)
#else

objecttype ut8_write (arguments)
listtype arguments;
#endif

  { /* ut8_write */
    isit_file(arg_1(arguments));
    isit_stri(arg_2(arguments));
    ut8Write(take_file(arg_1(arguments)), take_stri(arg_2(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* ut8_write */
