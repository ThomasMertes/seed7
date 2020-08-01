/********************************************************************/
/*                                                                  */
/*  gkb_rtl.c     Generic keyboard support for graphics keyboard.   */
/*  Copyright (C) 1989 - 2011  Thomas Mertes                        */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/gkb_rtl.c                                       */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Generic keyboard support for graphics keyboard.        */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "rtl_err.h"
#include "drw_drv.h"
#include "con_drv.h"

#undef EXTERN
#define EXTERN
#include "gkb_rtl.h"


#define READ_STRI_INIT_SIZE      256
#define READ_STRI_SIZE_DELTA    2048



#ifdef ANSI_C

stritype gkbGets (inttype length)
#else

stritype gkbGets (length)
inttype length;
#endif

  {
    memsizetype bytes_requested;
    memsizetype position;
    stritype result;

  /* gkbGets */
    if (length < 0) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      if ((uinttype) length > MAX_MEMSIZETYPE) {
        bytes_requested = MAX_MEMSIZETYPE;
      } else {
        bytes_requested = (memsizetype) length;
      } /* if */
      if (!ALLOC_STRI_CHECK_SIZE(result, bytes_requested)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        for (position = 0; position < bytes_requested; position++) {
          result->mem[position] = (strelemtype) gkbGetc();
        } /* for */
        result->size = bytes_requested;
        return result;
      } /* if */
    } /* if */
  } /* gkbGets */



#ifdef ANSI_C

stritype gkbLineRead (chartype *termination_char)
#else

stritype gkbLineRead (termination_char)
chartype *termination_char;
#endif

  {
    register chartype ch;
    register memsizetype position;
    strelemtype *memory;
    memsizetype memlength;
    memsizetype newmemlength;
    stritype resized_result;
    stritype result;

  /* gkbLineRead */
    memlength = READ_STRI_INIT_SIZE;
    if (!ALLOC_STRI_SIZE_OK(result, memlength)) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      while ((ch = gkbGetc()) != '\n' && ch != (chartype) ((schartype) EOF)) {
        if (position >= memlength) {
          newmemlength = memlength + READ_STRI_SIZE_DELTA;
          REALLOC_STRI_CHECK_SIZE(resized_result, result, memlength, newmemlength);
          if (resized_result == NULL) {
            FREE_STRI(result, memlength);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
          result = resized_result;
          COUNT3_STRI(memlength, newmemlength);
          memory = result->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strelemtype) ch;
      } /* while */
      if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      REALLOC_STRI_SIZE_OK(resized_result, result, memlength, position);
      if (resized_result == NULL) {
        FREE_STRI(result, memlength);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(memlength, position);
        result->size = position;
        *termination_char = ch;
      } /* if */
    } /* if */
    return result;
  } /* gkbLineRead */



#ifdef ANSI_C

stritype gkbWordRead (chartype *termination_char)
#else

stritype gkbWordRead (termination_char)
chartype *termination_char;
#endif

  {
    register chartype ch;
    register memsizetype position;
    strelemtype *memory;
    memsizetype memlength;
    memsizetype newmemlength;
    stritype resized_result;
    stritype result;

  /* gkbWordRead */
    memlength = READ_STRI_INIT_SIZE;
    if (!ALLOC_STRI_SIZE_OK(result, memlength)) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      do {
        ch = gkbGetc();
      } while (ch == ' ' || ch == '\t');
      while (ch != ' ' && ch != '\t' &&
          ch != '\n' && ch != (chartype) ((schartype) EOF)) {
        if (position >= memlength) {
          newmemlength = memlength + READ_STRI_SIZE_DELTA;
          REALLOC_STRI_CHECK_SIZE(resized_result, result, memlength, newmemlength);
          if (resized_result == NULL) {
            FREE_STRI(result, memlength);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
          result = resized_result;
          COUNT3_STRI(memlength, newmemlength);
          memory = result->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strelemtype) ch;
        ch = gkbGetc();
      } /* while */
      if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      REALLOC_STRI_SIZE_OK(resized_result, result, memlength, position);
      if (resized_result == NULL) {
        FREE_STRI(result, memlength);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(memlength, position);
        result->size = position;
        *termination_char = ch;
      } /* if */
    } /* if */
    return result;
  } /* gkbWordRead */
