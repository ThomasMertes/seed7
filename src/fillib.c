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



objecttype fil_big_lng (listtype arguments)

  { /* fil_big_lng */
    isit_file(arg_1(arguments));
    return bld_bigint_temp(
        filBigLng(take_file(arg_1(arguments))));
  } /* fil_big_lng */



objecttype fil_big_seek (listtype arguments)

  { /* fil_big_seek */
    isit_file(arg_1(arguments));
    isit_bigint(arg_2(arguments));
    filBigSeek(take_file(arg_1(arguments)),
        take_bigint(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_big_seek */



objecttype fil_big_tell (listtype arguments)

  { /* fil_big_tell */
    isit_file(arg_1(arguments));
    return bld_bigint_temp(
        filBigTell(take_file(arg_1(arguments))));
  } /* fil_big_tell */



objecttype fil_close (listtype arguments)

  { /* fil_close */
    isit_file(arg_1(arguments));
    filClose(take_file(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_close */



objecttype fil_cpy (listtype arguments)

  {
    objecttype file_variable;

  /* fil_cpy */
    file_variable = arg_1(arguments);
    isit_file(file_variable);
    is_variable(file_variable);
    isit_file(arg_3(arguments));
    file_variable->value.filevalue = take_file(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* fil_cpy */



objecttype fil_create (listtype arguments)

  { /* fil_create */
    isit_file(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), FILEOBJECT);
    arg_1(arguments)->value.filevalue = take_file(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* fil_create */



objecttype fil_empty (listtype arguments)

  { /* fil_empty */
    return bld_file_temp(NULL);
  } /* fil_empty */



objecttype fil_eof (listtype arguments)

  { /* fil_eof */
    isit_file(arg_1(arguments));
    if (feof(take_file(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* fil_eof */



objecttype fil_eq (listtype arguments)

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



objecttype fil_err (listtype arguments)

  { /* fil_err */
    return bld_file_temp(stderr);
  } /* fil_err */



objecttype fil_flush (listtype arguments)

  { /* fil_flush */
    isit_file(arg_1(arguments));
    fflush(take_file(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_flush */



objecttype fil_getc (listtype arguments)

  { /* fil_getc */
    isit_file(arg_1(arguments));
    return bld_char_temp((chartype)
        getc(take_file(arg_1(arguments))));
  } /* fil_getc */



objecttype fil_gets (listtype arguments)

  { /* fil_gets */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    return bld_stri_temp(
        filGets(take_file(arg_1(arguments)), take_int(arg_2(arguments))));
  } /* fil_gets */



objecttype fil_has_next (listtype arguments)

  { /* fil_has_next */
    isit_file(arg_1(arguments));
    if (filHasNext(take_file(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* fil_has_next */



objecttype fil_in (listtype arguments)

  { /* fil_in */
    return bld_file_temp(stdin);
  } /* fil_in */



objecttype fil_input_ready (listtype arguments)

  { /* fil_input_ready */
    isit_file(arg_1(arguments));
    if (filInputReady(take_file(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* fil_input_ready */



objecttype fil_line_read (listtype arguments)

  {
    objecttype ch_variable;

  /* fil_line_read */
    isit_file(arg_1(arguments));
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return bld_stri_temp(
        filLineRead(take_file(arg_1(arguments)), &ch_variable->value.charvalue));
  } /* fil_line_read */



objecttype fil_lit (listtype arguments)

  { /* fil_lit */
    isit_file(arg_1(arguments));
    return bld_stri_temp(
        filLit(take_file(arg_1(arguments))));
  } /* fil_lit */



objecttype fil_lng (listtype arguments)

  { /* fil_lng */
    isit_file(arg_1(arguments));
    return bld_int_temp(
        filLng(take_file(arg_1(arguments))));
  } /* fil_lng */



objecttype fil_ne (listtype arguments)

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



objecttype fil_open (listtype arguments)

  { /* fil_open */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    return bld_file_temp(
        filOpen(take_stri(arg_1(arguments)), take_stri(arg_2(arguments))));
  } /* fil_open */



objecttype fil_out (listtype arguments)

  { /* fil_out */
    return bld_file_temp(stdout);
  } /* fil_out */



objecttype fil_pclose (listtype arguments)

  { /* fil_pclose */
    isit_file(arg_1(arguments));
    filPclose(take_file(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_pclose */



objecttype fil_popen (listtype arguments)

  { /* fil_popen */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    isit_stri(arg_3(arguments));
    return bld_file_temp(
        filPopen(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)),
                 take_stri(arg_3(arguments))));
  } /* fil_popen */



objecttype fil_print (listtype arguments)

  { /* fil_print */
    isit_stri(arg_1(arguments));
    filPrint(take_stri(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_print */



objecttype fil_seek (listtype arguments)

  { /* fil_seek */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    filSeek(take_file(arg_1(arguments)),
        take_int(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_seek */



objecttype fil_setbuf (listtype arguments)

  { /* fil_setbuf */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    filSetbuf(take_file(arg_1(arguments)),
        take_int(arg_2(arguments)), take_int(arg_3(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_setbuf */



objecttype fil_tell (listtype arguments)

  { /* fil_tell */
    isit_file(arg_1(arguments));
    return bld_int_temp(
        filTell(take_file(arg_1(arguments))));
  } /* fil_tell */



objecttype fil_value (listtype arguments)

  {
    objecttype obj_arg;

  /* fil_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != FILEOBJECT) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_file_temp(take_file(obj_arg));
    } /* if */
  } /* fil_value */



objecttype fil_word_read (listtype arguments)

  {
    objecttype ch_variable;

  /* fil_word_read */
    isit_file(arg_1(arguments));
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return bld_stri_temp(
        filWordRead(take_file(arg_1(arguments)), &ch_variable->value.charvalue));
  } /* fil_word_read */



objecttype fil_write (listtype arguments)

  { /* fil_write */
    isit_file(arg_1(arguments));
    isit_stri(arg_2(arguments));
    filWrite(take_file(arg_1(arguments)), take_stri(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* fil_write */
