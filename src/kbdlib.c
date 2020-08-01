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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/kbdlib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do keyboard input.            */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "runerr.h"
#include "memory.h"
#include "kbd_drv.h"
#include "scr_drv.h"

#undef EXTERN
#define EXTERN
#include "kbdlib.h"



#ifdef ANSI_C

objecttype kbd_busy_getc (listtype arguments)
#else

objecttype kbd_busy_getc (arguments)
listtype arguments;
#endif

  { /* kbd_busy_getc */
    if (kbdKeyPressed()) {
      return(bld_char_temp(kbdGetc()));
    } else {
      return(bld_char_temp((chartype) K_NONE));
    } /* if */
  } /* kbd_busy_getc */



#ifdef ANSI_C

objecttype kbd_getc (listtype arguments)
#else

objecttype kbd_getc (arguments)
listtype arguments;
#endif

  { /* kbd_getc */
    return(bld_char_temp(kbdGetc()));
  } /* kbd_getc */



#ifdef ANSI_C

objecttype kbd_keypressed (listtype arguments)
#else

objecttype kbd_keypressed (arguments)
listtype arguments;
#endif

  { /* kbd_keypressed */
    if (kbdKeyPressed()) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* kbd_keypressed */



#ifdef ANSI_C

objecttype kbd_line_read (listtype arguments)
#else

objecttype kbd_line_read (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;
    char line[2049];
    memsizetype position;
    stritype result;
    int ch;

  /* kbd_line_read */
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    position = 0;
    ch = kbdGetc();
    while (ch != '\n' && ch != EOF && position < 2048) {
      line[position] = (char) ch;
      ch = kbdGetc();
      position++;
    } /* while */
    if (!ALLOC_STRI(result, position)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      COUNT_STRI(position);
      result->size = position;
      memcpy(result->mem, line, (SIZE_TYPE) position);
      ch_variable->value.charvalue = (chartype) ch;
      return(bld_stri_temp(result));
    } /* if */
  } /* kbd_line_read */



#ifdef ANSI_C

objecttype kbd_raw_getc (listtype arguments)
#else

objecttype kbd_raw_getc (arguments)
listtype arguments;
#endif

  { /* kbd_raw_read */
    return(bld_char_temp(kbdRawGetc()));
  } /* kbd_raw_read */



#ifdef ANSI_C

objecttype kbd_stri_read (listtype arguments)
#else

objecttype kbd_stri_read (arguments)
listtype arguments;
#endif

  {
    inttype length;
    memsizetype bytes_requested;
    memsizetype position;
    stritype result;

  /* kbd_stri_read */
    isit_int(arg_2(arguments));
    length = take_int(arg_2(arguments));
    if (length < 0) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } else {
      bytes_requested = (memsizetype) length;
      if (!ALLOC_STRI(result, bytes_requested)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        COUNT_STRI(bytes_requested);
        for (position = 0; position < bytes_requested; position++) {
          result->mem[position] = (strelemtype) kbdGetc();
        } /* for */
        result->size = bytes_requested;
        return(bld_stri_temp(result));
      } /* if */
    } /* if */
  } /* kbd_stri_read */



#ifdef ANSI_C

objecttype kbd_word_read (listtype arguments)
#else

objecttype kbd_word_read (arguments)
listtype arguments;
#endif

  {
    objecttype ch_variable;
    char line[2049];
    memsizetype position;
    stritype result;
    int ch;

  /* kbd_word_read */
    ch_variable = arg_2(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    do {
      ch = kbdGetc();
    } while (ch == ' ' || ch == '\t');
    position = 0;
    while (ch != ' ' && ch != '\t' &&
        ch != '\n' && ch != EOF && position < 2048) {
      line[position] = (char) ch;
      ch = kbdGetc();
      position++;
    } /* while */
    if (!ALLOC_STRI(result, position)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      COUNT_STRI(position);
      result->size = position;
      memcpy(result->mem, line, (SIZE_TYPE) position);
      ch_variable->value.charvalue = (chartype) ch;
      return(bld_stri_temp(result));
    } /* if */
  } /* kbd_word_read */
