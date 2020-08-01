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
/*  File: seed7/src/fillib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions for the primitive file type.     */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "runerr.h"
#include "memory.h"
#include "fil_rtl.h"

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



#ifdef ANSI_C

objecttype fil_big_lng (listtype arguments)
#else

objecttype fil_big_lng (arguments)
listtype arguments;
#endif

  { /* fil_big_lng */
    isit_file(arg_1(arguments));
    return(bld_bigint_temp(
        filBigLng(take_file(arg_1(arguments)))));
  } /* fil_big_lng */



#ifdef ANSI_C

objecttype fil_big_seek (listtype arguments)
#else

objecttype fil_big_seek (arguments)
listtype arguments;
#endif

  { /* fil_big_seek */
    isit_file(arg_1(arguments));
    isit_bigint(arg_2(arguments));
    filBigSeek(take_file(arg_1(arguments)),
        take_bigint(arg_2(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* fil_big_seek */



#ifdef ANSI_C

objecttype fil_big_tell (listtype arguments)
#else

objecttype fil_big_tell (arguments)
listtype arguments;
#endif

  { /* fil_big_tell */
    isit_file(arg_1(arguments));
    return(bld_bigint_temp(
        filBigTell(take_file(arg_1(arguments)))));
  } /* fil_big_tell */



#ifdef ANSI_C

objecttype fil_close (listtype arguments)
#else

objecttype fil_close (arguments)
listtype arguments;
#endif

  { /* fil_close */
    isit_file(arg_1(arguments));
    fclose(take_file(arg_1(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* fil_close */



#ifdef ANSI_C

objecttype fil_cpy (listtype arguments)
#else

objecttype fil_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype file_variable;

  /* fil_cpy */
    file_variable = arg_1(arguments);
    isit_file(file_variable);
    is_variable(file_variable);
    isit_file(arg_3(arguments));
    file_variable->value.filevalue = take_file(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* fil_cpy */



#ifdef ANSI_C

objecttype fil_create (listtype arguments)
#else

objecttype fil_create (arguments)
listtype arguments;
#endif

  { /* fil_create */
    isit_file(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), FILEOBJECT);
    arg_1(arguments)->value.filevalue = take_file(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* fil_create */



#ifdef ANSI_C

objecttype fil_empty (listtype arguments)
#else

objecttype fil_empty (arguments)
listtype arguments;
#endif

  { /* fil_empty */
    return(bld_file_temp(NULL));
  } /* fil_empty */



#ifdef ANSI_C

objecttype fil_eof (listtype arguments)
#else

objecttype fil_eof (arguments)
listtype arguments;
#endif

  { /* fil_eof */
    isit_file(arg_1(arguments));
    if (feof(take_file(arg_1(arguments)))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* fil_eof */



#ifdef ANSI_C

objecttype fil_eq (listtype arguments)
#else

objecttype fil_eq (arguments)
listtype arguments;
#endif

  { /* fil_eq */
    isit_file(arg_1(arguments));
    isit_file(arg_3(arguments));
    if (take_file(arg_1(arguments)) ==
        take_file(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* fil_eq */



#ifdef ANSI_C

objecttype fil_err (listtype arguments)
#else

objecttype fil_err (arguments)
listtype arguments;
#endif

  { /* fil_err */
    return(bld_file_temp(stderr));
  } /* fil_err */



#ifdef ANSI_C

objecttype fil_flush (listtype arguments)
#else

objecttype fil_flush (arguments)
listtype arguments;
#endif

  { /* fil_flush */
    isit_file(arg_1(arguments));
    fflush(take_file(arg_1(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* fil_flush */



#ifdef ANSI_C

objecttype fil_getc (listtype arguments)
#else

objecttype fil_getc (arguments)
listtype arguments;
#endif

  { /* fil_getc */
    isit_file(arg_1(arguments));
    return(bld_char_temp((chartype)
        getc(take_file(arg_1(arguments)))));
  } /* fil_getc */



#ifdef ANSI_C

objecttype fil_gets (listtype arguments)
#else

objecttype fil_gets (arguments)
listtype arguments;
#endif

  { /* fil_gets */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    return(bld_stri_temp(
        filGets(take_file(arg_1(arguments)), take_int(arg_2(arguments)))));
  } /* fil_gets */



#ifdef ANSI_C

objecttype fil_has_next (listtype arguments)
#else

objecttype fil_has_next (arguments)
listtype arguments;
#endif

  { /* fil_has_next */
    isit_file(arg_1(arguments));
    if (filHasNext(take_file(arg_1(arguments)))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* fil_has_next */



#ifdef ANSI_C

objecttype fil_in (listtype arguments)
#else

objecttype fil_in (arguments)
listtype arguments;
#endif

  { /* fil_in */
    return(bld_file_temp(stdin));
  } /* fil_in */



#ifdef ANSI_C

objecttype fil_line_read (listtype arguments)
#else

objecttype fil_line_read (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* fil_line_read */
    isit_file(arg_1(arguments));
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return(bld_stri_temp(
        filLineRead(take_file(arg_1(arguments)), &ch_variable->value.charvalue)));
  } /* fil_line_read */



#ifdef ANSI_C

objecttype fil_lit (listtype arguments)
#else

objecttype fil_lit (arguments)
listtype arguments;
#endif

  { /* fil_lit */
    isit_file(arg_1(arguments));
    return(bld_stri_temp(
        filLit(take_file(arg_1(arguments)))));
  } /* fil_lit */



#ifdef ANSI_C

objecttype fil_lng (listtype arguments)
#else

objecttype fil_lng (arguments)
listtype arguments;
#endif

  { /* fil_lng */
    isit_file(arg_1(arguments));
    return(bld_int_temp(
        filLng(take_file(arg_1(arguments)))));
  } /* fil_lng */



#ifdef ANSI_C

objecttype fil_ne (listtype arguments)
#else

objecttype fil_ne (arguments)
listtype arguments;
#endif

  { /* fil_ne */
    isit_file(arg_1(arguments));
    isit_file(arg_3(arguments));
    if (take_file(arg_1(arguments)) !=
        take_file(arg_3(arguments))) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* fil_ne */



#ifdef ANSI_C

objecttype fil_open (listtype arguments)
#else

objecttype fil_open (arguments)
listtype arguments;
#endif

  { /* fil_open */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return(bld_file_temp(
        filOpen(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)))));
  } /* fil_open */



#ifdef ANSI_C

objecttype fil_out (listtype arguments)
#else

objecttype fil_out (arguments)
listtype arguments;
#endif

  { /* fil_out */
    return(bld_file_temp(stdout));
  } /* fil_out */



#ifdef ANSI_C

objecttype fil_popen (listtype arguments)
#else

objecttype fil_popen (arguments)
listtype arguments;
#endif

  { /* fil_popen */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return(bld_file_temp(
        filPopen(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)))));
  } /* fil_popen */



#ifdef ANSI_C

objecttype fil_print (listtype arguments)
#else

objecttype fil_print (arguments)
listtype arguments;
#endif

  {
    stritype stri;
    cstritype str1;

  /* fil_print */
    isit_stri(arg_1(arguments));
    stri = take_stri(arg_1(arguments));
    str1 = cp_to_cstri(stri);
    if (str1 == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      fputs(str1, stdout);
      fflush(stdout);
      free_cstri(str1, stri);
      return(SYS_EMPTY_OBJECT);
    } /* if */
  } /* fil_print */



#ifdef ANSI_C

objecttype fil_seek (listtype arguments)
#else

objecttype fil_seek (arguments)
listtype arguments;
#endif

  { /* fil_seek */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    filSeek(take_file(arg_1(arguments)),
        take_int(arg_2(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* fil_seek */



#ifdef ANSI_C

objecttype fil_tell (listtype arguments)
#else

objecttype fil_tell (arguments)
listtype arguments;
#endif

  { /* fil_tell */
    isit_file(arg_1(arguments));
    return(bld_int_temp(
        filTell(take_file(arg_1(arguments)))));
  } /* fil_tell */



#ifdef ANSI_C

objecttype fil_value (listtype arguments)
#else

objecttype fil_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;

  /* fil_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != FILEOBJECT) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } else {
      return(bld_file_temp(take_file(obj_arg)));
    } /* if */
  } /* fil_value */



#ifdef ANSI_C

objecttype fil_word_read (listtype arguments)
#else

objecttype fil_word_read (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* fil_word_read */
    isit_file(arg_1(arguments));
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return(bld_stri_temp(
        filWordRead(take_file(arg_1(arguments)), &ch_variable->value.charvalue)));
  } /* fil_word_read */



#ifdef ANSI_C

objecttype fil_write (listtype arguments)
#else

objecttype fil_write (arguments)
listtype arguments;
#endif

  { /* fil_write */
    isit_file(arg_1(arguments));
    isit_stri(arg_2(arguments));
    filWrite(take_file(arg_1(arguments)), take_stri(arg_2(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* fil_write */
