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

#include "stdlib.h"
#include "stdio.h"

#include "version.h"
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

  {
    filetype fil1;
    cstritype file_name;
    memsizetype length;
    stritype result;

  /* fil_lit */
    isit_file(arg_1(arguments));
    fil1 = take_file(arg_1(arguments));
    if (fil1 == NULL) {
      file_name = "NULL";
    } else if (fil1 == stdin) {
      file_name = "stdin";
    } else if (fil1 == stdout) {
      file_name = "stdout";
    } else if (fil1 == stderr) {
      file_name = "stderr";
    } else {
      file_name = "file";
    } /* if */
    length = strlen(file_name);
    if (!ALLOC_STRI(result, length)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
    COUNT_STRI(length);
    result->size = length;
    stri_expand(result->mem, file_name, (SIZE_TYPE) length);
    return(bld_stri_temp(result));
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

objecttype fil_l_rd (listtype arguments)
#else

objecttype fil_l_rd (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;

  /* fil_l_rd */
    isit_file(arg_1(arguments));
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    return(bld_stri_temp(
        filLineRead(take_file(arg_1(arguments)), &ch_variable->value.charvalue)));
  } /* fil_l_rd */



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

  {
    stritype str1;
    stritype str2;
    cstritype file_name;
    cstritype file_mode;
    objecttype result;

  /* fil_open */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    str1 = take_stri(arg_1(arguments));
    str2 = take_stri(arg_2(arguments));
    file_name = cp_to_cstri(str1);
    if (file_name == NULL) {
      result = raise_exception(SYS_MEM_EXCEPTION);
    } else {
      file_mode = cp_to_cstri(str2);
      if (file_mode == NULL) {
        result = raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result = bld_file_temp(fopen(file_name, file_mode));
        free_cstri(file_mode, str2);
      } /* if */
      free_cstri(file_name, str1);
    } /* if */
    return(result);
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
    if (take_int(arg_2(arguments)) <= 0) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } else {
      fseek(take_file(arg_1(arguments)),
          take_int(arg_2(arguments)) - 1, SEEK_SET);
      return(SYS_EMPTY_OBJECT);
    } /* if */
  } /* fil_seek */



#ifdef ANSI_C

objecttype fil_stri_read (listtype arguments)
#else

objecttype fil_stri_read (arguments)
listtype arguments;
#endif

  { /* fil_stri_read */
    isit_file(arg_1(arguments));
    isit_int(arg_2(arguments));
    return(bld_stri_temp(
        filStriRead(take_file(arg_1(arguments)), take_int(arg_2(arguments)))));
  } /* fil_stri_read */



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
