/********************************************************************/
/*                                                                  */
/*  ut8_rtl.c     Primitive actions for the UTF-8 file type.        */
/*  Copyright (C) 1989 - 2005  Thomas Mertes                        */
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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/ut8_rtl.c                                       */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: Primitive actions for the UTF-8 file type.             */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"

#include "version.h"
#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "ut8_rtl.h"



#ifdef ANSI_C

chartype ut8Getc (filetype fil1)
#else

chartype ut8Getc (fil1)
filetype fil1;
#endif

  {
    int character;
    chartype result;

  /* ut8Getc */
    character = getc(fil1);
    if (character != EOF && character > 0x7F) {
      if ((character & 0xE0) == 0xC0) {
        result = (character & 0x1F) << 6;
        character = getc(fil1);
        if ((character & 0xC0) == 0x80) {
          result |= character & 0x3F;
          if (result <= 0x7F) {
            raise_error(RANGE_ERROR);
            return(0);
          } /* if */
        } else {
          raise_error(RANGE_ERROR);
          return(0);
        } /* if */
      } else if ((character & 0xF0) == 0xE0) {
        result = (character & 0x0F) << 12;
        character = getc(fil1);
        if ((character & 0xC0) == 0x80) {
          result |= (character & 0x3F) << 6;
          character = getc(fil1);
          if ((character & 0xC0) == 0x80) {
            result |= character & 0x3F;
            if (result <= 0x7FF || (result >= 0xD800 && result <= 0xDFFF)) {
              raise_error(RANGE_ERROR);
              return(0);
            } /* if */
          } else {
            raise_error(RANGE_ERROR);
            return(0);
          } /* if */
        } else {
          raise_error(RANGE_ERROR);
          return(0);
        } /* if */
      } else if ((character & 0xF8) == 0xF0) {
        result = (character & 0x07) << 18;
        character = getc(fil1);
        if ((character & 0xC0) == 0x80) {
          result |= (character & 0x3F) << 12;
          character = getc(fil1);
          if ((character & 0xC0) == 0x80) {
            result |= (character & 0x3F) << 6;
            character = getc(fil1);
            if ((character & 0xC0) == 0x80) {
              result |= character & 0x3F;
              if (result <= 0xFFFF) {
                raise_error(RANGE_ERROR);
                return(0);
              } /* if */
            } else {
              raise_error(RANGE_ERROR);
              return(0);
            } /* if */
          } else {
            raise_error(RANGE_ERROR);
            return(0);
          } /* if */
        } else {
          raise_error(RANGE_ERROR);
          return(0);
        } /* if */
      } else if ((character & 0xFC) == 0xF8) {
        result = (character & 0x03) << 24;
        character = getc(fil1);
        if ((character & 0xC0) == 0x80) {
          result |= (character & 0x3F) << 18;
          character = getc(fil1);
          if ((character & 0xC0) == 0x80) {
            result |= (character & 0x3F) << 12;
            character = getc(fil1);
            if ((character & 0xC0) == 0x80) {
              result |= (character & 0x3F) << 6;
              character = getc(fil1);
              if ((character & 0xC0) == 0x80) {
                result |= character & 0x3F;
                if (result <= 0x1FFFFF) {
                  raise_error(RANGE_ERROR);
                  return(0);
                } /* if */
              } else {
                raise_error(RANGE_ERROR);
                return(0);
              } /* if */
            } else {
              raise_error(RANGE_ERROR);
              return(0);
            } /* if */
          } else {
            raise_error(RANGE_ERROR);
            return(0);
          } /* if */
        } else {
          raise_error(RANGE_ERROR);
          return(0);
        } /* if */
      } else { /* (character & 0xFC) == 0xFC */
	result = (character & 0x03) << 30;
        character = getc(fil1);
        if ((character & 0xC0) == 0x80) {
	  result |= (character & 0x3F) << 24;
          character = getc(fil1);
          if ((character & 0xC0) == 0x80) {
	    result |= (character & 0x3F) << 18;
            character = getc(fil1);
            if ((character & 0xC0) == 0x80) {
	      result |= (character & 0x3F) << 12;
              character = getc(fil1);
              if ((character & 0xC0) == 0x80) {
		result |= (character & 0x3F) <<  6;
                character = getc(fil1);
                if ((character & 0xC0) == 0x80) {
		  result |= character & 0x3F;
                  if (result <= 0x3FFFFFF) {
                    raise_error(RANGE_ERROR);
                    return(0);
                  } /* if */
                } else {
                  raise_error(RANGE_ERROR);
                  return(0);
                } /* if */
              } else {
                raise_error(RANGE_ERROR);
                return(0);
              } /* if */
            } else {
              raise_error(RANGE_ERROR);
              return(0);
            } /* if */
          } else {
            raise_error(RANGE_ERROR);
            return(0);
          } /* if */
        } else {
          raise_error(RANGE_ERROR);
          return(0);
        } /* if */
      } /* if */
    } else {
      result = (chartype) character;
    } /* if */
    return(result);
  } /* ut8Getc */



#ifdef ANSI_C

stritype ut8LineRead (filetype fil1, chartype *termination_char)
#else

stritype ut8LineRead (fil1, termination_char)
filetype fil1;
chartype *termination_char;
#endif

  {
    register int ch;
    register memsizetype position;
    uchartype *memory;
    memsizetype memlength;
    memsizetype newmemlength;
    bstritype buffer;
    stritype result;

  /* ut8LineRead */
    memlength = 256;
    if (!ALLOC_BSTRI(buffer, memlength)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BSTRI(memlength);
      memory = buffer->mem;
      position = 0;
      while ((ch = getc(fil1)) != '\n' && ch != EOF) {
        if (position >= memlength) {
          newmemlength = memlength + 2048;
          if (!RESIZE_BSTRI(buffer, memlength, newmemlength)) {
	    FREE_BSTRI(buffer, memlength);
            raise_error(MEMORY_ERROR);
            return(NULL);
          } /* if */
          COUNT3_BSTRI(memlength, newmemlength);
          memory = buffer->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strelemtype) ch;
      } /* while */
      if (!ALLOC_STRI(result, position)) {
	FREE_BSTRI(buffer, memlength);
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_STRI(position);
        result->size = utf8_to_stri(result->mem, buffer->mem, position);
	FREE_BSTRI(buffer, memlength);
        if (!RESIZE_STRI(result, result->size, position)) {
	  FREE_STRI(buffer, position);
          raise_error(MEMORY_ERROR);
          return(NULL);
        } /* if */
        COUNT3_STRI(result->size, position);
        *termination_char = (chartype) ch;
        return(result);
      } /* if */
    } /* if */
  } /* ut8LineRead */



#ifdef ANSI_C

stritype ut8StriRead (filetype fil1, inttype length)
#else

stritype ut8StriRead (fil1, length)
filetype fil1;
inttype length;
#endif

  {
    long current_file_position;
    memsizetype bytes_there;
    memsizetype bytes_requested;
    memsizetype buffer_size;
    bstritype buffer;
    stritype result;

  /* ut8StriRead */
    if (length < 0) {
      raise_error(RANGE_ERROR);
      return(NULL);
    } else {
      bytes_requested = (memsizetype) length;
      if (!ALLOC_BSTRI(buffer, bytes_requested)) {
        if ((current_file_position = ftell(fil1)) != -1) {
          fseek(fil1, 0, SEEK_END);
          bytes_there = (memsizetype) (ftell(fil1) - current_file_position);
          fseek(fil1, current_file_position, SEEK_SET);
          if (bytes_there < bytes_requested) {
            bytes_requested = bytes_there;
            if (!ALLOC_BSTRI(buffer, bytes_requested)) {
              raise_error(MEMORY_ERROR);
              return(NULL);
            } /* if */
          } else {
            raise_error(MEMORY_ERROR);
            return(NULL);
          } /* if */
        } else {
          raise_error(MEMORY_ERROR);
          return(NULL);
        } /* if */
      } /* if */
      COUNT_BSTRI(bytes_requested);
      buffer_size = (memsizetype) fread(buffer->mem, 1,
          (SIZE_TYPE) bytes_requested, fil1);
      if (!ALLOC_STRI(result, buffer_size)) {
	FREE_BSTRI(buffer, bytes_requested);
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_STRI(buffer_size);
        result->size = utf8_to_stri(result->mem, buffer->mem, buffer_size);
	FREE_BSTRI(buffer, bytes_requested);
        if (!RESIZE_STRI(result, result->size, buffer_size)) {
	  FREE_STRI(buffer, buffer_size);
          raise_error(MEMORY_ERROR);
          return(NULL);
        } /* if */
        COUNT3_STRI(result->size, buffer_size);
        return(result);
      } /* if */
    } /* if */
    return(result);
  } /* ut8StriRead */



#ifdef ANSI_C

stritype ut8WordRead (filetype fil1, chartype *termination_char)
#else

stritype ut8WordRead (fil1, termination_char)
filetype fil1;
chartype *termination_char;
#endif

  {
    register int ch;
    register memsizetype position;
    uchartype *memory;
    memsizetype memlength;
    memsizetype newmemlength;
    bstritype buffer;
    stritype result;

  /* ut8WordRead */
    memlength = 256;
    if (!ALLOC_BSTRI(buffer, memlength)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_BSTRI(memlength);
      memory = buffer->mem;
      position = 0;
      do {
        ch = getc(fil1);
      } while (ch == ' ' || ch == '\t');
      while (ch != ' ' && ch != '\t' &&
          ch != '\n' && ch != EOF) {
        if (position >= memlength) {
          newmemlength = memlength + 2048;
          if (!RESIZE_BSTRI(buffer, memlength, newmemlength)) {
	    FREE_BSTRI(buffer, memlength);
            raise_error(MEMORY_ERROR);
            return(NULL);
          } /* if */
          COUNT3_BSTRI(memlength, newmemlength);
          memory = buffer->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strelemtype) ch;
        ch = getc(fil1);
      } /* while */
      if (!ALLOC_STRI(result, position)) {
	FREE_BSTRI(buffer, memlength);
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        COUNT_STRI(position);
        result->size = utf8_to_stri(result->mem, buffer->mem, position);
	FREE_BSTRI(buffer, memlength);
        if (!RESIZE_STRI(result, result->size, position)) {
	  FREE_STRI(buffer, position);
          raise_error(MEMORY_ERROR);
          return(NULL);
        } /* if */
        COUNT3_STRI(result->size, position);
        *termination_char = (chartype) ch;
        return(result);
      } /* if */
    } /* if */
  } /* ut8WordRead */



#ifdef ANSI_C

void ut8Write (filetype fil1, stritype stri)
#else

void ut8Write (stri)
filetype fil1;
stritype stri;
#endif

  {
    bstritype out_bstri;

  /* ut8Write */
    out_bstri = cp_to_bstri(stri);
    if (out_bstri != NULL) {
      fwrite(out_bstri->mem, sizeof(uchartype),
          (SIZE_TYPE) out_bstri->size, fil1);
      FREE_BSTRI(out_bstri, out_bstri->size);
    } /* if */
  } /* ut8Write */
