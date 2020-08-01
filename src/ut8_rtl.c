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
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/ut8_rtl.c                                       */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: Primitive actions for the UTF-8 file type.             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "cmd_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "ut8_rtl.h"


#define BUFFER_SIZE 2048



#ifdef ANSI_C

chartype ut8Getc (filetype aFile)
#else

chartype ut8Getc (aFile)
filetype aFile;
#endif

  {
    int character;
    chartype result;

  /* ut8Getc */
    character = getc(aFile);
    if (character != EOF && character > 0x7F) {
      if ((character & 0xE0) == 0xC0) {
        result = (character & 0x1F) << 6;
        character = getc(aFile);
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
        character = getc(aFile);
        if ((character & 0xC0) == 0x80) {
          result |= (character & 0x3F) << 6;
          character = getc(aFile);
          if ((character & 0xC0) == 0x80) {
            result |= character & 0x3F;
            if (result <= 0x7FF) {  /* (result >= 0xD800 && result <= 0xDFFF)) */
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
        character = getc(aFile);
        if ((character & 0xC0) == 0x80) {
          result |= (character & 0x3F) << 12;
          character = getc(aFile);
          if ((character & 0xC0) == 0x80) {
            result |= (character & 0x3F) << 6;
            character = getc(aFile);
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
        character = getc(aFile);
        if ((character & 0xC0) == 0x80) {
          result |= (character & 0x3F) << 18;
          character = getc(aFile);
          if ((character & 0xC0) == 0x80) {
            result |= (character & 0x3F) << 12;
            character = getc(aFile);
            if ((character & 0xC0) == 0x80) {
              result |= (character & 0x3F) << 6;
              character = getc(aFile);
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
        character = getc(aFile);
        if ((character & 0xC0) == 0x80) {
          result |= (character & 0x3F) << 24;
          character = getc(aFile);
          if ((character & 0xC0) == 0x80) {
            result |= (character & 0x3F) << 18;
            character = getc(aFile);
            if ((character & 0xC0) == 0x80) {
              result |= (character & 0x3F) << 12;
              character = getc(aFile);
              if ((character & 0xC0) == 0x80) {
                result |= (character & 0x3F) <<  6;
                character = getc(aFile);
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

stritype ut8Gets (filetype aFile, inttype length)
#else

stritype ut8Gets (aFile, length)
filetype aFile;
inttype length;
#endif

  {
    long current_file_position;
    memsizetype bytes_there;
    memsizetype chars_requested;
    memsizetype chars_missing;
    uchartype buffer[BUFFER_SIZE + 6];
    memsizetype bytes_remaining;
    memsizetype old_bytes_remaining;
    memsizetype bytes_missing;
    memsizetype bytes_in_buffer;
    memsizetype chars_read;
    memsizetype chars_there;
    strelemtype *stri_dest;
    stritype resized_result;
    stritype result;

  /* ut8Gets */
    if (length < 0) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      chars_requested = (memsizetype) length;
      if (!ALLOC_STRI(result, chars_requested)) {
        if ((current_file_position = ftell(aFile)) != -1) {
          fseek(aFile, 0, SEEK_END);
          bytes_there = (memsizetype) (ftell(aFile) - current_file_position);
          fseek(aFile, current_file_position, SEEK_SET);
          if (bytes_there < chars_requested) {
            chars_requested = bytes_there;
            if (!ALLOC_STRI(result, chars_requested)) {
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
      /* printf("#0-A#\n"); */
      bytes_remaining = 0;
      bytes_missing = 0;
      chars_there = 0;
      chars_read = 1;
      for (stri_dest = result->mem, chars_missing = chars_requested;
          chars_missing >= BUFFER_SIZE - bytes_missing + chars_there && (chars_read > 0 || chars_there);
          stri_dest += chars_read, chars_missing -= chars_read) {
        bytes_in_buffer = (memsizetype) fread(&buffer[bytes_remaining], 1,
            BUFFER_SIZE, aFile);
        /* printf("#1-A# bytes_in_buffer=%d result->size=%d\n", bytes_in_buffer, stri_dest - result->mem); */
        if (bytes_in_buffer != 0) {
          old_bytes_remaining = bytes_remaining;
          bytes_in_buffer += bytes_remaining;
          /* printf("#1-A# bytes_in_buffer=%d %X %X\n", bytes_in_buffer, buffer[0], buffer[1]); */
          bytes_remaining = utf8_to_stri(stri_dest, &chars_read, buffer, bytes_in_buffer);
          if (bytes_remaining != 0) {
            /* printf("#1-A# bytes_remaining=%d %X\n", bytes_remaining, buffer[bytes_in_buffer - bytes_remaining]); */
            bytes_missing = utf8_bytes_missing(&buffer[bytes_in_buffer - bytes_remaining], bytes_remaining);
            /* printf("#1-A# bytes_missing=%d\n", bytes_missing); */
            if (bytes_missing != 0) {
              memmove(buffer, &buffer[bytes_in_buffer - bytes_remaining], bytes_remaining);
              /* printf("#2-A# %X %X\n", buffer[0], buffer[1]); */
              chars_there = 1;
            } else {
              /* printf("#3-A# bytes_in_buffer=%d bytes_remaining=%d old_bytes_remaining=%d bytes_missing=%d result->size=%d chars_requested=%d, chars_missing=%d %X ftell=%ld\n",
                  bytes_in_buffer, bytes_remaining, old_bytes_remaining, bytes_missing, stri_dest - result->mem,
                  chars_requested, chars_missing, buffer[bytes_in_buffer - bytes_remaining], ftell(aFile)); */
              FREE_STRI(result, chars_requested);
              raise_error(RANGE_ERROR);
              return(NULL);
            } /* if */
          } else {
            bytes_missing = 0;
            chars_there = 0;
          } /* if */
        } else {
          chars_read = 0;
        } /* if */
        /* printf("#1-B# chars_read=%d\n", chars_read); */
      } /* for */
      for (; chars_missing > 0 && (chars_read > 0 || chars_there);
          stri_dest += chars_read, chars_missing -= chars_read) {
        bytes_in_buffer = (memsizetype) fread(&buffer[bytes_remaining], 1,
            chars_missing - chars_there + bytes_missing, aFile);
        /* printf("#1-B# bytes_in_buffer=%d chars_missing=%d chars_read=%d chars_there=%d bytes_missing=%d result->size=%d\n",
            bytes_in_buffer, chars_missing, chars_read, chars_there, bytes_missing, stri_dest - result->mem); */
        if (bytes_in_buffer != 0) {
          old_bytes_remaining = bytes_remaining;
          bytes_in_buffer += bytes_remaining;
          /* printf("#1-B# bytes_in_buffer=%d %X %X\n", bytes_in_buffer, buffer[0], buffer[1]); */
          bytes_remaining = utf8_to_stri(stri_dest, &chars_read, buffer, bytes_in_buffer);
          if (bytes_remaining != 0) {
            /* printf("#1-B# bytes_remaining=%d %X\n", bytes_remaining, buffer[bytes_in_buffer - bytes_remaining]); */
            bytes_missing = utf8_bytes_missing(&buffer[bytes_in_buffer - bytes_remaining], bytes_remaining);
            /* printf("#1-B# bytes_missing=%d\n", bytes_missing); */
            if (bytes_missing != 0) {
              memmove(buffer, &buffer[bytes_in_buffer - bytes_remaining], bytes_remaining);
              /* printf("#2-B# %X %X\n", buffer[0], buffer[1]); */
              chars_there = 1;
            } else {
              /* printf("#3-B# bytes_in_buffer=%d bytes_remaining=%d old_bytes_remaining=%d bytes_missing=%d result->size=%d chars_requested=%d, chars_missing=%d %X ftell=%ld\n",
                  bytes_in_buffer, bytes_remaining, old_bytes_remaining, bytes_missing, stri_dest - result->mem,
                  chars_requested, chars_missing, buffer[bytes_in_buffer - bytes_remaining], ftell(aFile)); */
              FREE_STRI(result, chars_requested);
              raise_error(RANGE_ERROR);
              return(NULL);
            } /* if */
          } else {
            bytes_missing = 0;
            chars_there = 0;
          } /* if */
        } else {
          chars_read = 0;
        } /* if */
        /* printf("#1-B# chars_read=%d\n", chars_read); */
      } /* for */
      result->size = stri_dest - result->mem;
      REALLOC_STRI(resized_result, result, chars_requested, result->size);
      if (resized_result == NULL) {
        FREE_STRI(result, chars_requested);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(chars_requested, result->size);
      } /* if */
    } /* if */
    return(result);
  } /* ut8Gets */



#ifdef ANSI_C

stritype ut8LineRead (filetype aFile, chartype *termination_char)
#else

stritype ut8LineRead (aFile, termination_char)
filetype aFile;
chartype *termination_char;
#endif

  {
    register int ch;
    register memsizetype position;
    uchartype *memory;
    memsizetype memlength;
    memsizetype newmemlength;
    bstritype resized_buffer;
    bstritype buffer;
    stritype resized_result;
    stritype result;

  /* ut8LineRead */
    memlength = 256;
    if (!ALLOC_BSTRI(buffer, memlength)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      memory = buffer->mem;
      position = 0;
      while ((ch = getc(aFile)) != '\n' && ch != EOF) {
        if (position >= memlength) {
          newmemlength = memlength + 2048;
          REALLOC_BSTRI(resized_buffer, buffer, memlength, newmemlength);
          if (resized_buffer == NULL) {
            FREE_BSTRI(buffer, memlength);
            raise_error(MEMORY_ERROR);
            return(NULL);
          } /* if */
          buffer = resized_buffer;
          COUNT3_BSTRI(memlength, newmemlength);
          memory = buffer->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strelemtype) ch;
      } /* while */
      if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      if (!ALLOC_STRI(result, position)) {
        FREE_BSTRI(buffer, memlength);
        raise_error(MEMORY_ERROR);
      } else {
        if (utf8_to_stri(result->mem, &result->size, buffer->mem, position) != 0) {
          FREE_BSTRI(buffer, memlength);
          FREE_STRI(result, position);
          raise_error(RANGE_ERROR);
          result = NULL;
        } else {
          FREE_BSTRI(buffer, memlength);
          REALLOC_STRI(resized_result, result, position, result->size);
          if (resized_result == NULL) {
            FREE_STRI(result, position);
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            result = resized_result;
            COUNT3_STRI(position, result->size);
            *termination_char = (chartype) ch;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return(result);
  } /* ut8LineRead */



#ifdef ANSI_C

void ut8Seek (filetype aFile, inttype file_position)
#else

void ut8Seek (aFile, file_position)
filetype aFile;
inttype file_position;
#endif

  {
    int ch;

  /* ut8Seek */
    if (file_position <= 0) {
      raise_error(RANGE_ERROR);
      return;
    } else {
      if (fseek(aFile, file_position - 1, SEEK_SET) == 0) {
        while ((ch = getc(aFile)) != EOF &&
               (ch & 0xC0) == 0x80) ;
        if (ch != EOF) {
          fseek(aFile, -1, SEEK_CUR);
        } /* if */
      } /* if */
    } /* if */
  } /* ut8Seek */



#ifdef ANSI_C

stritype ut8WordRead (filetype aFile, chartype *termination_char)
#else

stritype ut8WordRead (aFile, termination_char)
filetype aFile;
chartype *termination_char;
#endif

  {
    register int ch;
    register memsizetype position;
    uchartype *memory;
    memsizetype memlength;
    memsizetype newmemlength;
    bstritype resized_buffer;
    bstritype buffer;
    stritype resized_result;
    stritype result;

  /* ut8WordRead */
    memlength = 256;
    if (!ALLOC_BSTRI(buffer, memlength)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      memory = buffer->mem;
      position = 0;
      do {
        ch = getc(aFile);
      } while (ch == ' ' || ch == '\t');
      while (ch != ' ' && ch != '\t' &&
          ch != '\n' && ch != EOF) {
        if (position >= memlength) {
          newmemlength = memlength + 2048;
          REALLOC_BSTRI(resized_buffer, buffer, memlength, newmemlength);
          if (resized_buffer == NULL) {
            FREE_BSTRI(buffer, memlength);
            raise_error(MEMORY_ERROR);
            return(NULL);
          } /* if */
          buffer = resized_buffer;
          COUNT3_BSTRI(memlength, newmemlength);
          memory = buffer->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strelemtype) ch;
        ch = getc(aFile);
      } /* while */
      if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      if (!ALLOC_STRI(result, position)) {
        FREE_BSTRI(buffer, memlength);
        raise_error(MEMORY_ERROR);
      } else {
        if (utf8_to_stri(result->mem, &result->size, buffer->mem, position) != 0) {
          FREE_BSTRI(buffer, memlength);
          FREE_STRI(result, position);
          raise_error(RANGE_ERROR);
          result = NULL;
        } else {
          FREE_BSTRI(buffer, memlength);
          REALLOC_STRI(resized_result, result, position, result->size);
          if (resized_result == NULL) {
            FREE_STRI(result, position);
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            result = resized_result;
            COUNT3_STRI(position, result->size);
            *termination_char = (chartype) ch;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return(result);
  } /* ut8WordRead */



#ifdef ANSI_C

void ut8Write (filetype aFile, stritype stri)
#else

void ut8Write (stri)
filetype aFile;
stritype stri;
#endif

  {
    bstritype out_bstri;

  /* ut8Write */
    out_bstri = stri_to_bstri8(stri);
    if (out_bstri != NULL) {
      if (out_bstri->size != fwrite(out_bstri->mem, sizeof(uchartype),
          (SIZE_TYPE) out_bstri->size, aFile)) {
        FREE_BSTRI(out_bstri, out_bstri->size);
        raise_error(FILE_ERROR);
      } else {
        FREE_BSTRI(out_bstri, out_bstri->size);
      } /* if */
    } /* if */
  } /* ut8Write */
