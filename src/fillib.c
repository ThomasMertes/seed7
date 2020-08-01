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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
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
    SET_CLASS_OF_OBJ(arg_1(arguments), FILEOBJECT);
    arg_1(arguments)->value.filevalue = take_file(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* fil_create */



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

objecttype fil_in (listtype arguments)
#else

objecttype fil_in (arguments)
listtype arguments;
#endif

  { /* fil_in */
    return(bld_file_temp(stdin));
  } /* fil_in */



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



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

objecttype fil_l_rd (listtype arguments)
#else

objecttype fil_l_rd (arguments)
listtype arguments;
#endif

  {
    char line[2049];
    memsizetype length;
    stritype result;
    filetype fil1;
    objecttype ch_variable;

  /* fil_l_rd */
    isit_file(arg_1(arguments));
    fil1 = take_file(arg_1(arguments));
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    line[0] = '\0';
    fgets(line, 2049, fil1);
    length = strlen(line);
    if (length > 0) {
      length--;
    } /* if */
    if (!ALLOC_STRI(result, length)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      COUNT_STRI(length);
      result->size = length;
      memcpy(result->mem, line, (SIZE_TYPE) length);
      ch_variable->value.charvalue = (chartype) line[length];
      return(bld_stri_temp(result));
    } /* if */
  } /* fil_l_rd */
#endif



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

objecttype fil_l_rd (listtype arguments)
#else

objecttype fil_l_rd (arguments)
listtype arguments;
#endif

  {
    memsizetype size;
    memsizetype memlength;
    memsizetype newmemlength;
    stritype result;
    strelemtype *memory;

  /* fil_l_rd */
    isit_file(arg_1(arguments));
    memlength = 2049;
    if (!ALLOC_STRI(result, memlength)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      COUNT_STRI(memlength);
      memory = result->mem;
      result->mem[0] = '\0';
      printf("/%d/", feof(take_file(arg_1(arguments))));
      fgets(memory, memlength, take_file(arg_1(arguments)));
      size = strlen(memory);
/*    printf("<%d,%d,%s>", memlength, size, result->mem);
      fflush(stdout); */
      printf("(%d)", feof(take_file(arg_1(arguments))));
      while (size > 0 && result->mem[size - 1] != '\n' &&
          ! feof(take_file(arg_1(arguments)))) {
        newmemlength = memlength + 2048;
        if (!RESIZE_STRI(result, memlength, newmemlength)) {
          return(raise_exception(SYS_MEM_EXCEPTION));
        } /* if */
        COUNT3_STRI(memlength, newmemlength);
        memory = &result->mem[memlength - 1];
        fgets(memory, 2049, take_file(arg_1(arguments)));
        size = memlength - 1 + strlen(memory);
        memlength = newmemlength;
/*      printf("<%d,%d,%s,%s>", memlength, size, result->mem, memory);
        fflush(stdout); */
        printf("(%d)", feof(take_file(arg_1(arguments))));
      } /* while */
      if (size > 0 && result->mem[size - 1] == '\n') {
        size--;
        if (!RESIZE_STRI(result, memlength, size)) {
          return(raise_exception(SYS_MEM_EXCEPTION));
        } /* if */
        COUNT3_STRI(memlength, size);
        result->size = size;
      } /* if */
      return(bld_stri_temp(result));
    } /* if */
  } /* fil_l_rd */
#endif



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

objecttype fil_nil (listtype arguments)
#else

objecttype fil_nil (arguments)
listtype arguments;
#endif

  { /* fil_nil */
    return(bld_file_temp(NULL));
  } /* fil_nil */



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
        (inttype) ftell(take_file(arg_1(arguments))) + 1));
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
    isit_reference(arg_3(arguments));
    obj_arg = take_reference(arg_3(arguments));
    isit_file(obj_arg);
    return(bld_file_temp(take_file(obj_arg)));
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
