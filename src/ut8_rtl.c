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


typedef struct {
    memsizetype bytes_remaining;
    memsizetype bytes_missing;
    memsizetype chars_read;
    memsizetype chars_there;
  } read_state;



#ifdef ANSI_C

static INLINE void bytes_to_strelements (ustritype buffer, memsizetype bytes_in_buffer,
    strelemtype *stri_dest, read_state *state, errinfotype *err_info)
#else

static INLINE void bytes_to_strelements (buffer, bytes_in_buffer, stri_dest,
    state, err_info)
ustritype buffer;
memsizetype bytes_in_buffer;
strelemtype *stri_dest;
read_state *state;
errinfotype *err_info;
#endif

  { /* bytes_to_strelements */
    if (bytes_in_buffer != 0) {
      bytes_in_buffer += state->bytes_remaining;
      /* printf("#1# bytes_in_buffer=%d %X %X\n", bytes_in_buffer, buffer[0], buffer[1]); */
      state->bytes_remaining = utf8_to_stri(stri_dest, &state->chars_read, buffer, bytes_in_buffer);
      if (state->bytes_remaining != 0) {
        /* printf("#2# bytes_remaining=%d %X\n", state->bytes_remaining,
            buffer[bytes_in_buffer - state->bytes_remaining]); */
        state->bytes_missing = utf8_bytes_missing(&buffer[bytes_in_buffer - state->bytes_remaining],
                                                  state->bytes_remaining);
        /* printf("#3# bytes_missing=%d\n", state->bytes_missing); */
        if (state->bytes_missing != 0) {
          memmove(buffer, &buffer[bytes_in_buffer - state->bytes_remaining], state->bytes_remaining);
          /* printf("#4# %X %X\n", buffer[0], buffer[1]); */
          state->chars_there = 1;
        } else {
          /* printf("#5# bytes_in_buffer=%d bytes_remaining=%d bytes_missing=%d chars_requested=%d chars_missing=%d %X ftell=%ld\n",
              bytes_in_buffer, state->bytes_remaining, state->bytes_missing,
              chars_requested, chars_missing, buffer[bytes_in_buffer - state->bytes_remaining],
              ftell(aFile)); */
          *err_info = RANGE_ERROR;
          return;
        } /* if */
      } else {
        state->bytes_missing = 0;
        state->chars_there = 0;
      } /* if */
    } else {
      state->chars_read = 0;
    } /* if */
    /* printf("#6# chars_read=%d\n", state->chars_read); */
  } /* bytes_to_strelements */



#ifdef ANSI_C

static memsizetype read_strelements (filetype aFile, memsizetype chars_requested,
    stritype result, errinfotype *err_info)
#else

static memsizetype read_strelements (aFile, chars_requested, result, err_info)
filetype aFile;
memsizetype chars_requested;
stritype result;
errinfotype *err_info;
#endif

  {
    memsizetype chars_missing;
    uchartype buffer[BUFFER_SIZE + 6];
    memsizetype bytes_in_buffer;
    strelemtype *stri_dest;
    read_state state = {0, 0, 1, 0};

  /* read_strelements */
    for (stri_dest = result->mem, chars_missing = chars_requested;
        chars_missing >= BUFFER_SIZE - state.bytes_missing + state.chars_there &&
        (state.chars_read > 0 || state.chars_there);
        stri_dest += state.chars_read, chars_missing -= state.chars_read) {
      bytes_in_buffer = (memsizetype) fread(&buffer[state.bytes_remaining], 1,
          BUFFER_SIZE, aFile);
      /* printf("#A# bytes_in_buffer=%d num_of_chars_read=%d\n",
          bytes_in_buffer, (memsizetype) (stri_dest - result->mem)); */
      bytes_to_strelements(buffer, bytes_in_buffer, stri_dest,
           &state, err_info);
    } /* for */
    for (; chars_missing > 0 && (state.chars_read > 0 || state.chars_there);
        stri_dest += state.chars_read, chars_missing -= state.chars_read) {
      bytes_in_buffer = (memsizetype) fread(&buffer[state.bytes_remaining], 1,
          chars_missing - state.chars_there + state.bytes_missing, aFile);
      /* printf("#B# bytes_in_buffer=%d chars_missing=%d chars_read=%d chars_there=%d bytes_missing=%d num_of_chars_read=%d\n",
          bytes_in_buffer, chars_missing, chars_read, chars_there,
          state.bytes_missing, (memsizetype) (stri_dest - result->mem)); */

      bytes_to_strelements(buffer, bytes_in_buffer, stri_dest,
           &state, err_info);
    } /* for */
    return (memsizetype) (stri_dest - result->mem);
  } /* read_strelements */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

static void read_strelements (filetype aFile, memsizetype chars_requested, strelemtype *result_mem,
    memsizetype *result_size, errinfotype *err_info)
#else

static void read_strelements (aFile, chars_requested, result_mem, result_size, err_info)
filetype aFile;
memsizetype chars_requested;
strelemtype *result_mem;
memsizetype *result_size;
errinfotype *err_info;
#endif

  {
    memsizetype chars_missing;
    uchartype buffer[BUFFER_SIZE + 6];
    memsizetype bytes_remaining = 0;
    /* memsizetype old_bytes_remaining; */
    memsizetype bytes_missing = 0;
    memsizetype bytes_in_buffer;
    memsizetype chars_read = 1;
    memsizetype chars_there = 0;
    strelemtype *stri_dest;

  /* read_strelements */
    /* printf("#0-A#\n"); */
    for (stri_dest = result_mem, chars_missing = chars_requested;
        chars_missing >= BUFFER_SIZE - bytes_missing + chars_there && (chars_read > 0 || chars_there);
        stri_dest += chars_read, chars_missing -= chars_read) {
      bytes_in_buffer = (memsizetype) fread(&buffer[bytes_remaining], 1,
          BUFFER_SIZE, aFile);
      /* printf("#1-A# bytes_in_buffer=%d result_size=%d\n", bytes_in_buffer, stri_dest - result_mem); */
      if (bytes_in_buffer != 0) {
        /* old_bytes_remaining = bytes_remaining; */
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
            /* printf("#3-A# bytes_in_buffer=%d bytes_remaining=%d old_bytes_remaining=%d bytes_missing=%d result_size=%d chars_requested=%d, chars_missing=%d %X ftell=%ld\n",
                bytes_in_buffer, bytes_remaining, old_bytes_remaining, bytes_missing, stri_dest - result_mem,
                chars_requested, chars_missing, buffer[bytes_in_buffer - bytes_remaining], ftell(aFile)); */
            *err_info = RANGE_ERROR;
            return;
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
      /* printf("#1-B# bytes_in_buffer=%d chars_missing=%d chars_read=%d chars_there=%d bytes_missing=%d result_size=%d\n",
          bytes_in_buffer, chars_missing, chars_read, chars_there, bytes_missing, stri_dest - result_mem); */
      if (bytes_in_buffer != 0) {
        /* old_bytes_remaining = bytes_remaining; */
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
            /* printf("#3-B# bytes_in_buffer=%d bytes_remaining=%d old_bytes_remaining=%d bytes_missing=%d result_size=%d chars_requested=%d, chars_missing=%d %X ftell=%ld\n",
                bytes_in_buffer, bytes_remaining, old_bytes_remaining, bytes_missing, stri_dest - result_mem,
                chars_requested, chars_missing, buffer[bytes_in_buffer - bytes_remaining], ftell(aFile)); */
            *err_info = RANGE_ERROR;
            return;
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
    *result_size = stri_dest - result_mem;
  } /* read_strelements */
#endif



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
      /* character range 0x80 to 0xFF (128 to 255) */
      if ((character & 0xE0) == 0xC0) {
        /* character range 0xC0 to 0xDF (192 to 223) */
        result = (chartype) (character & 0x1F) << 6;
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
        /* character range 0xE0 to 0xEF (224 to 239) */
        result = (chartype) (character & 0x0F) << 12;
        character = getc(aFile);
        if ((character & 0xC0) == 0x80) {
          result |= (chartype) (character & 0x3F) << 6;
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
        /* character range 0xF0 to 0xF7 (240 to 247) */
        result = (chartype) (character & 0x07) << 18;
        character = getc(aFile);
        if ((character & 0xC0) == 0x80) {
          result |= (chartype) (character & 0x3F) << 12;
          character = getc(aFile);
          if ((character & 0xC0) == 0x80) {
            result |= (chartype) (character & 0x3F) << 6;
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
        /* character range 0xF8 to 0xFB (248 to 251) */
        result = (chartype) (character & 0x03) << 24;
        character = getc(aFile);
        if ((character & 0xC0) == 0x80) {
          result |= (chartype) (character & 0x3F) << 18;
          character = getc(aFile);
          if ((character & 0xC0) == 0x80) {
            result |= (chartype) (character & 0x3F) << 12;
            character = getc(aFile);
            if ((character & 0xC0) == 0x80) {
              result |= (chartype) (character & 0x3F) << 6;
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
      } else if ((character & 0xFC) == 0xFC) {
        /* character range 0xFC to 0xFF (252 to 255) */
        result = (chartype) (character & 0x03) << 30;
        character = getc(aFile);
        if ((character & 0xC0) == 0x80) {
          result |= (chartype) (character & 0x3F) << 24;
          character = getc(aFile);
          if ((character & 0xC0) == 0x80) {
            result |= (chartype) (character & 0x3F) << 18;
            character = getc(aFile);
            if ((character & 0xC0) == 0x80) {
              result |= (chartype) (character & 0x3F) << 12;
              character = getc(aFile);
              if ((character & 0xC0) == 0x80) {
                result |= (chartype) (character & 0x3F) <<  6;
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
      } else {
        /* character not in range 0xC0 to 0xFF (192 to 255) */
        raise_error(RANGE_ERROR);
        return(0);
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
    errinfotype err_info = OKAY_NO_ERROR;
    memsizetype num_of_chars_read;
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
      result->size = chars_requested;
      num_of_chars_read = read_strelements(aFile, chars_requested, result, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        FREE_STRI(result, chars_requested);
        raise_error(err_info);
        result = NULL;
      } else {
        REALLOC_STRI(resized_result, result, chars_requested, num_of_chars_read);
        if (resized_result == NULL) {
          FREE_STRI(result, chars_requested);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result = resized_result;
          COUNT3_STRI(chars_requested, num_of_chars_read);
          result->size = num_of_chars_read;
        } /* if */
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
    memsizetype result_size;
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
      while ((ch = getc(aFile)) != (int) '\n' && ch != EOF) {
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
        memory[position++] = (uchartype) ch;
      } /* while */
      if (ch == (int) '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      if (!ALLOC_STRI(result, position)) {
        FREE_BSTRI(buffer, memlength);
        raise_error(MEMORY_ERROR);
      } else {
        if (utf8_to_stri(result->mem, &result_size, buffer->mem, position) != 0) {
          FREE_BSTRI(buffer, memlength);
          FREE_STRI(result, position);
          raise_error(RANGE_ERROR);
          result = NULL;
        } else {
          FREE_BSTRI(buffer, memlength);
          REALLOC_STRI(resized_result, result, position, result_size);
          if (resized_result == NULL) {
            FREE_STRI(result, position);
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            result = resized_result;
            COUNT3_STRI(position, result_size);
            result->size = result_size;
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
    memsizetype result_size;
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
      } while (ch == (int) ' ' || ch == (int) '\t');
      while (ch != (int) ' ' && ch != (int) '\t' &&
          ch != (int) '\n' && ch != EOF) {
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
        memory[position++] = (uchartype) ch;
        ch = getc(aFile);
      } /* while */
      if (ch == (int) '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      if (!ALLOC_STRI(result, position)) {
        FREE_BSTRI(buffer, memlength);
        raise_error(MEMORY_ERROR);
      } else {
        if (utf8_to_stri(result->mem, &result_size, buffer->mem, position) != 0) {
          FREE_BSTRI(buffer, memlength);
          FREE_STRI(result, position);
          raise_error(RANGE_ERROR);
          result = NULL;
        } else {
          FREE_BSTRI(buffer, memlength);
          REALLOC_STRI(resized_result, result, position, result_size);
          if (resized_result == NULL) {
            FREE_STRI(result, position);
            raise_error(MEMORY_ERROR);
            result = NULL;
          } else {
            result = resized_result;
            COUNT3_STRI(position, result_size);
            result->size = result_size;
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

void ut8Write (aFile, stri)
filetype aFile;
stritype stri;
#endif

  {
    bstritype out_bstri;

  /* ut8Write */
    out_bstri = stri_to_bstri8(stri);
    if (out_bstri != NULL) {
      if (out_bstri->size != fwrite(out_bstri->mem, sizeof(uchartype),
          (size_t) out_bstri->size, aFile)) {
        FREE_BSTRI(out_bstri, out_bstri->size);
        raise_error(FILE_ERROR);
      } else {
        FREE_BSTRI(out_bstri, out_bstri->size);
      } /* if */
    } /* if */
  } /* ut8Write */
