/********************************************************************/
/*                                                                  */
/*  ut8_rtl.c     Primitive actions for the UTF-8 file type.        */
/*  Copyright (C) 1989 - 2015, 2018  Thomas Mertes                  */
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
/*  Changes: 2005, 2010, 2013, 2014  Thomas Mertes                  */
/*  Content: Primitive actions for the UTF-8 file type.             */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sys/types.h"
#include "errno.h"

#include "common.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "fil_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "ut8_rtl.h"


#define BUFFER_SIZE             2048
#define GETS_DEFAULT_SIZE    1048576
#define GETS_STRI_SIZE_DELTA    4096
#define READ_STRI_INIT_SIZE      256
#define READ_STRI_SIZE_DELTA    2048
#define WRITE_STRI_BLOCK_SIZE    512


typedef struct {
    memSizeType bytes_remaining;
    memSizeType bytes_missing;
    memSizeType chars_read;
    memSizeType chars_there;
  } readStateType;



static inline void bytes_to_strelements (ustriType buffer, memSizeType bytes_in_buffer,
    strElemType *stri_dest, readStateType *state, errInfoType *err_info)

  { /* bytes_to_strelements */
    if (bytes_in_buffer != 0) {
      bytes_in_buffer += state->bytes_remaining;
      state->bytes_remaining = utf8_to_stri(stri_dest, &state->chars_read, buffer,
                                            bytes_in_buffer);
      if (state->bytes_remaining != 0) {
        state->bytes_missing = utf8_bytes_missing(
            &buffer[bytes_in_buffer - state->bytes_remaining],
            state->bytes_remaining);
        if (state->bytes_missing != 0) {
          memmove(buffer, &buffer[bytes_in_buffer - state->bytes_remaining],
                  state->bytes_remaining);
          state->chars_there = 1;
        } else {
          logError(printf("bytes_to_strelements:\n"
                          "bytes_in_buffer=" FMT_U_MEM ", "
                          "bytes_remaining=" FMT_U_MEM ", "
                          "bytes_missing=" FMT_U_MEM ", %X\n",
                          bytes_in_buffer, state->bytes_remaining,
                          state->bytes_missing,
                          buffer[bytes_in_buffer - state->bytes_remaining]););
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
  } /* bytes_to_strelements */



/**
 *  Read UTF-8 characters from 'inFile' into the allocated string.
 *  Read until the string 'stri' is filled or 'inFile' reaches EOF.
 *  @param inFile File from which UTF-8 encoded characters are read.
 *  @param stri An allocated string for the requested number of chars.
 *  @param err_info Unchanged if the function succeeds, and
 *                  RANGE_ERROR if inFile contains invalid encodings, and
 *                  FILE_ERROR if a system function returns an error.
 *  @return the actual number of characters read.
 */
static memSizeType read_utf8_string (cFileType inFile, striType stri, errInfoType *err_info)

  {
    ucharType buffer[BUFFER_SIZE + 6];
    memSizeType bytes_in_buffer;
    memSizeType stri_pos;
    memSizeType chars_missing;
    readStateType state = {0, 0, 1, 0};

  /* read_utf8_string */
    for (stri_pos = 0, chars_missing = stri->size;
        chars_missing >= BUFFER_SIZE - state.bytes_missing + state.chars_there &&
        (state.chars_read > 0 || state.chars_there) &&
        *err_info == OKAY_NO_ERROR;
        stri_pos += state.chars_read, chars_missing -= state.chars_read) {
      bytes_in_buffer = (memSizeType) fread(&buffer[state.bytes_remaining], 1,
          BUFFER_SIZE, inFile);
      if (bytes_in_buffer == 0 && stri_pos == 0 && ferror(inFile)) {
        logError(printf("read_utf8_string: fread(*, 1, " FMT_U_MEM ", %d) failed.\n",
                        (memSizeType) BUFFER_SIZE, safe_fileno(inFile)););
        *err_info = FILE_ERROR;
      } else {
        bytes_to_strelements(buffer, bytes_in_buffer, &stri->mem[stri_pos],
             &state, err_info);
      } /* if */
    } /* for */
    for (; chars_missing > 0 && (state.chars_read > 0 || state.chars_there) &&
        *err_info == OKAY_NO_ERROR;
        stri_pos += state.chars_read, chars_missing -= state.chars_read) {
      bytes_in_buffer = (memSizeType) fread(&buffer[state.bytes_remaining], 1,
          chars_missing - state.chars_there + state.bytes_missing, inFile);
      if (bytes_in_buffer == 0 && stri_pos == 0 && ferror(inFile)) {
        logError(printf("read_utf8_string: fread(*, 1, " FMT_U_MEM ", %d) failed.\n",
                        chars_missing - state.chars_there + state.bytes_missing,
                        safe_fileno(inFile)););
        *err_info = FILE_ERROR;
      } else {
        bytes_to_strelements(buffer, bytes_in_buffer, &stri->mem[stri_pos],
             &state, err_info);
      } /* if */
    } /* for */
    return stri_pos;
  } /* read_utf8_string */



/**
 *  Read up to 'chars_missing' UTF-8 characters from 'inFile'.
 *  Read until 'chars_missing' characters are read or 'inFile' reaches EOF.
 *  @param inFile File from which UTF-8 encoded characters are read.
 *  @param chars_missing Maximum number of characters to be read.
 *  @param num_of_chars_read Address to which the actual number of
 *         characters read is assigned.
 *  @param err_info Unchanged if the function succeeds, and
 *                  RANGE_ERROR if inFile contains invalid encodings, and
 *                  FILE_ERROR if a system function returns an error, and
 *                  MEMORY_ERROR if there was not enough memory.
 */
static striType read_and_alloc_utf8_stri (cFileType inFile, memSizeType chars_missing,
    memSizeType *num_of_chars_read, errInfoType *err_info)

  {
    ucharType buffer[BUFFER_SIZE + 6];
    memSizeType bytes_in_buffer;
    memSizeType result_pos;
    memSizeType new_size;
    striType resized_result;
    readStateType state = {0, 0, 1, 0};
    striType result;

  /* read_and_alloc_utf8_stri */
    logFunction(printf("read_and_alloc_utf8_stri(%d, " FMT_U_MEM ", *, *)\n",
                       safe_fileno(inFile), chars_missing););
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, GETS_STRI_SIZE_DELTA))) {
      *err_info = MEMORY_ERROR;
      result = NULL;
    } else {
      result->size = GETS_STRI_SIZE_DELTA;
      for (result_pos = 0;
          chars_missing >= BUFFER_SIZE - state.bytes_missing + state.chars_there &&
          (state.chars_read > 0 || state.chars_there) &&
          *err_info == OKAY_NO_ERROR;
          result_pos += state.chars_read, chars_missing -= state.chars_read) {
        bytes_in_buffer = (memSizeType) fread(&buffer[state.bytes_remaining], 1,
            BUFFER_SIZE, inFile);
        if (bytes_in_buffer == 0 && result_pos == 0 && ferror(inFile)) {
          logError(printf("read_and_alloc_utf8_stri: "
                          "fread(*, 1, " FMT_U_MEM ", %d) failed.\n",
                          (memSizeType) LIST_BUFFER_SIZE, safe_fileno(inFile)););
          *err_info = FILE_ERROR;
        } else {
          if (result_pos + bytes_in_buffer > result->size) {
            new_size = result->size + GETS_STRI_SIZE_DELTA;
            REALLOC_STRI_CHECK_SIZE(resized_result, result, result->size, new_size);
            if (resized_result == NULL) {
              *err_info = MEMORY_ERROR;
              return result;
            } else {
              result = resized_result;
              COUNT3_STRI(result->size, new_size);
              result->size = new_size;
            } /* if */
          } /* if */
          bytes_to_strelements(buffer, bytes_in_buffer, &result->mem[result_pos],
              &state, err_info);
        } /* if */
      } /* for */
      for (; chars_missing > 0 && (state.chars_read > 0 || state.chars_there) &&
          *err_info == OKAY_NO_ERROR;
          result_pos += state.chars_read, chars_missing -= state.chars_read) {
        bytes_in_buffer = (memSizeType) fread(&buffer[state.bytes_remaining], 1,
            chars_missing - state.chars_there + state.bytes_missing, inFile);
        if (bytes_in_buffer == 0 && result_pos == 0 && ferror(inFile)) {
          logError(printf("read_and_alloc_utf8_stri: "
                          "fread(*, 1, " FMT_U_MEM ", %d) failed.\n",
                          chars_missing - state.chars_there + state.bytes_missing,
                          safe_fileno(inFile)););
          *err_info = FILE_ERROR;
        } else {
          if (result_pos + bytes_in_buffer > result->size) {
            new_size = result->size + GETS_STRI_SIZE_DELTA;
            REALLOC_STRI_CHECK_SIZE(resized_result, result, result->size, new_size);
            if (resized_result == NULL) {
              *err_info = MEMORY_ERROR;
              return result;
            } else {
              result = resized_result;
              COUNT3_STRI(result->size, new_size);
              result->size = new_size;
            } /* if */
          } /* if */
          bytes_to_strelements(buffer, bytes_in_buffer, &result->mem[result_pos],
              &state, err_info);
        } /* if */
      } /* for */
      *num_of_chars_read = result_pos;
    } /* if */
    logFunction(printf("read_and_alloc_utf8_stri(%d, " FMT_U_MEM ", " FMT_U_MEM ", %d) -->\n",
                       safe_fileno(inFile), chars_missing, *num_of_chars_read, *err_info););
    return result;
  } /* read_and_alloc_utf8_stri */



/**
 *  Read a character from an UTF-8 file.
 *  @return the character read, or EOF at the end of the file.
 *  @exception RANGE_ERROR The file contains an invalid encoding.
 */
charType ut8Getc (fileType inFile)

  {
    cFileType cInFile;
    int character;
    charType result;

  /* ut8Getc */
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("ut8Getc: Attempt to read from closed file.\n"););
      raise_error(FILE_ERROR);
      return 0;
    } /* if */
    character = getc(cInFile);
    if (character != EOF && character >= 0x80) {
      /* character range 0x80 to 0xFF (128 to 255) */
      if (unlikely(character <= 0xBF)) {
        /* character range 0xC0 to 0xBF (128 to 191) */
        logError(printf("ut8Getc(%d): "
                        "Unexpected UTF-8 continuation byte ('\\16#%02x;').\n",
                        safe_fileno(cInFile), character););
        raise_error(RANGE_ERROR);
        return 0;
      } else if (character <= 0xDF) {
        /* character range 192 to 223 (leading bits 110.....) */
        result = (charType) (character & 0x1F) << 6;
        character = getc(cInFile);
        if (character >= 0x80 && character <= 0xBF) {
          /* character range 128 to 191 (leading bits 10......) */
          result |= character & 0x3F;
          if (unlikely(result <= 0x7F)) {
            logError(printf("ut8Getc(%d): "
                            "Overlong encodings are invalid "
                            "('\\16#" FMT_X32 ";').\n",
                            safe_fileno(cInFile), result););
            raise_error(RANGE_ERROR);
            return 0;
          } else {
            /* correct encodings are in the range */
            /* 0x80 to 0x07FF (128 to 2047)       */
          } /* if */
        } else {
          logError(printf("ut8Getc(%d): "
                          "UTF-8 continuation byte expected "
                          "(found '\\16#%02x;').\n",
                          safe_fileno(cInFile), character););
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } else if (character <= 0xEF) {
        /* character range 224 to 239 (leading bits 1110....) */
        result = (charType) (character & 0x0F) << 12;
        character = getc(cInFile);
        if (character >= 0x80 && character <= 0xBF) {
          /* character range 128 to 191 (leading bits 10......) */
          result |= (charType) (character & 0x3F) << 6;
          character = getc(cInFile);
          if (character >= 0x80 && character <= 0xBF) {
            result |= character & 0x3F;
            if (unlikely(result <= 0x7FF)) {
              /* (result >= 0xD800 && result <= 0xDFFF)) */
              logError(printf("ut8Getc(%d): "
                              "Overlong encodings are invalid "
                              "('\\16#" FMT_X32 ";').\n",
                              safe_fileno(cInFile), result););
              raise_error(RANGE_ERROR);
              return 0;
            } else {
              /* correct encodings are in the range */
              /* 0x800 to 0xFFFF (2048 to 65535)    */
            } /* if */
          } else {
            logError(printf("ut8Getc(%d): "
                            "UTF-8 continuation byte expected "
                            "(found '\\16#%02x;').\n",
                            safe_fileno(cInFile), character););
            raise_error(RANGE_ERROR);
            return 0;
          } /* if */
        } else {
          logError(printf("ut8Getc(%d): "
                          "UTF-8 continuation byte expected "
                          "(found '\\16#%02x;').\n",
                          safe_fileno(cInFile), character););
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } else if (character <= 0xF7) {
        /* character range 240 to 247 (leading bits 11110...) */
        result = (charType) (character & 0x07) << 18;
        character = getc(cInFile);
        if (character >= 0x80 && character <= 0xBF) {
          /* character range 128 to 191 (leading bits 10......) */
          result |= (charType) (character & 0x3F) << 12;
          character = getc(cInFile);
          if (character >= 0x80 && character <= 0xBF) {
            result |= (charType) (character & 0x3F) << 6;
            character = getc(cInFile);
            if (character >= 0x80 && character <= 0xBF) {
              result |= character & 0x3F;
              if (unlikely(result <= 0xFFFF)) {
                logError(printf("ut8Getc(%d): "
                                "Overlong encodings are invalid "
                                "('\\16#" FMT_X32 ";').\n",
                                safe_fileno(cInFile), result););
                raise_error(RANGE_ERROR);
                return 0;
              } else {
                /* correct encodings are in the range        */
                /* 0x10000 to 0x10FFFF (65536 to 1114111)    */
                /* allowed encodings are in the range        */
                /* 0x110000 to 0x1FFFFF (1114112 to 2097151) */
              } /* if */
            } else {
              logError(printf("ut8Getc(%d): "
                              "UTF-8 continuation byte expected "
                              "(found '\\16#%02x;').\n",
                              safe_fileno(cInFile), character););
              raise_error(RANGE_ERROR);
              return 0;
            } /* if */
          } else {
            logError(printf("ut8Getc(%d): "
                            "UTF-8 continuation byte expected "
                            "(found '\\16#%02x;').\n",
                            safe_fileno(cInFile), character););
            raise_error(RANGE_ERROR);
            return 0;
          } /* if */
        } else {
          logError(printf("ut8Getc(%d): "
                          "UTF-8 continuation byte expected "
                          "(found '\\16#%02x;').\n",
                          safe_fileno(cInFile), character););
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } else if (character <= 0xFB) {
        /* character range 248 to 251 (leading bits 111110..) */
        result = (charType) (character & 0x03) << 24;
        character = getc(cInFile);
        if (character >= 0x80 && character <= 0xBF) {
          /* character range 128 to 191 (leading bits 10......) */
          result |= (charType) (character & 0x3F) << 18;
          character = getc(cInFile);
          if (character >= 0x80 && character <= 0xBF) {
            result |= (charType) (character & 0x3F) << 12;
            character = getc(cInFile);
            if (character >= 0x80 && character <= 0xBF) {
              result |= (charType) (character & 0x3F) << 6;
              character = getc(cInFile);
              if (character >= 0x80 && character <= 0xBF) {
                result |= character & 0x3F;
                if (unlikely(result <= 0x1FFFFF)) {
                  logError(printf("ut8Getc(%d): "
                                  "Overlong encodings are invalid "
                                  "('\\16#" FMT_X32 ";').\n",
                                  safe_fileno(cInFile), result););
                  raise_error(RANGE_ERROR);
                  return 0;
                } else {
                  /* allowed encodings are in the range          */
                  /* 0x200000 to 0x3FFFFFF (2097152 to 67108863) */
                } /* if */
              } else {
                logError(printf("ut8Getc(%d): "
                                "UTF-8 continuation byte expected "
                                "(found '\\16#%02x;').\n",
                                safe_fileno(cInFile), character););
                raise_error(RANGE_ERROR);
                return 0;
              } /* if */
            } else {
              logError(printf("ut8Getc(%d): "
                              "UTF-8 continuation byte expected "
                              "(found '\\16#%02x;').\n",
                              safe_fileno(cInFile), character););
              raise_error(RANGE_ERROR);
              return 0;
            } /* if */
          } else {
            logError(printf("ut8Getc(%d): "
                            "UTF-8 continuation byte expected "
                            "(found '\\16#%02x;').\n",
                            safe_fileno(cInFile), character););
            raise_error(RANGE_ERROR);
            return 0;
          } /* if */
        } else {
          logError(printf("ut8Getc(%d): "
                          "UTF-8 continuation byte expected "
                          "(found '\\16#%02x;').\n",
                          safe_fileno(cInFile), character););
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } else { /* if (character <= 0xFF) { */
        /* character range 252 to 255 (leading bits 111111..) */
        result = (charType) (character & 0x03) << 30;
        character = getc(cInFile);
        if (character >= 0x80 && character <= 0xBF) {
          /* character range 128 to 191 (leading bits 10......) */
          result |= (charType) (character & 0x3F) << 24;
          character = getc(cInFile);
          if (character >= 0x80 && character <= 0xBF) {
            result |= (charType) (character & 0x3F) << 18;
            character = getc(cInFile);
            if (character >= 0x80 && character <= 0xBF) {
              result |= (charType) (character & 0x3F) << 12;
              character = getc(cInFile);
              if (character >= 0x80 && character <= 0xBF) {
                result |= (charType) (character & 0x3F) <<  6;
                character = getc(cInFile);
                if (character >= 0x80 && character <= 0xBF) {
                  result |= character & 0x3F;
                  if (unlikely(result <= 0x3FFFFFF)) {
                    logError(printf("ut8Getc(%d): "
                                    "Overlong encodings are invalid "
                                    "('\\16#" FMT_X32 ";').\n",
                                    safe_fileno(cInFile), result););
                    raise_error(RANGE_ERROR);
                    return 0;
                  } else {
                    /* allowed encodings are in the range               */
                    /* 0x4000000 to 0xFFFFFFFF (67108864 to 4294967295) */
                  } /* if */
                } else {
                  logError(printf("ut8Getc(%d): "
                                  "UTF-8 continuation byte expected "
                                  "(found '\\16#%02x;').\n",
                                  safe_fileno(cInFile), character););
                  raise_error(RANGE_ERROR);
                  return 0;
                } /* if */
              } else {
                logError(printf("ut8Getc(%d): "
                                "UTF-8 continuation byte expected "
                                "(found '\\16#%02x;').\n",
                                safe_fileno(cInFile), character););
                raise_error(RANGE_ERROR);
                return 0;
              } /* if */
            } else {
              logError(printf("ut8Getc(%d): "
                              "UTF-8 continuation byte expected "
                              "(found '\\16#%02x;').\n",
                              safe_fileno(cInFile), character););
              raise_error(RANGE_ERROR);
              return 0;
            } /* if */
          } else {
            logError(printf("ut8Getc(%d): "
                            "UTF-8 continuation byte expected "
                            "(found '\\16#%02x;').\n",
                            safe_fileno(cInFile), character););
            raise_error(RANGE_ERROR);
            return 0;
          } /* if */
        } else {
          logError(printf("ut8Getc(%d): "
                          "UTF-8 continuation byte expected "
                          "(found '\\16#%02x;').\n",
                          safe_fileno(cInFile), character););
          raise_error(RANGE_ERROR);
          return 0;
        } /* if */
      } /* if */
    } else {
      result = (charType) (scharType) character;
    } /* if */
    return result;
  } /* ut8Getc */



/**
 *  Read a string with 'length' characters from an UTF-8 file.
 *  In order to work reasonable good for the common case (reading
 *  just some characters) memory for 'length' characters is requested
 *  with malloc(). After the data is read the result string is
 *  shrunk to the actual size (with realloc()). If 'length' is
 *  larger than GETS_DEFAULT_SIZE or the memory cannot be requested
 *  a different strategy is used. In this case the function tries to
 *  find out the number of available characters (this is possible
 *  for a regular file but not for a pipe). If this fails a third
 *  strategy is used. In this case a smaller block is requested. This
 *  block is filled with data, resized and filled in a loop.
 *  @return the string read.
 *  @exception RANGE_ERROR The length is negative or the file
 *             contains an invalid encoding.
 */
striType ut8Gets (fileType inFile, intType length)

  {
    cFileType cInFile;
    memSizeType chars_requested;
    memSizeType bytes_there;
    memSizeType allocated_size;
    errInfoType err_info = OKAY_NO_ERROR;
    memSizeType num_of_chars_read;
    striType resized_result;
    striType result;

  /* ut8Gets */
    logFunction(printf("ut8Gets(%s%d, " FMT_D ")\n",
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       length););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("ut8Gets: Attempt to read from closed file.\n"););
      raise_error(FILE_ERROR);
      result = NULL;
    } else if (unlikely(length <= 0)) {
      if (unlikely(length != 0)) {
        logError(printf("ut8Gets(%d, " FMT_D "): Negative length.\n",
                        safe_fileno(cInFile), length););
        raise_error(RANGE_ERROR);
        result = NULL;
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, 0))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 0;
        } /* if */
      } /* if */
#if FREAD_WRONG_FOR_WRITE_ONLY_FILES
    } else if (unlikely(!inFile->readingAllowed)) {
      logError(printf("ut8Gets(%d, " FMT_D "): "
                      "The file is not open for reading.\n",
                      safe_fileno(cInFile), length););
      raise_error(FILE_ERROR);
      result = NULL;
#endif
    } else {
      if ((uintType) length > MAX_MEMSIZETYPE) {
        chars_requested = MAX_MEMSIZETYPE;
      } else {
        chars_requested = (memSizeType) length;
      } /* if */
      if (chars_requested > GETS_DEFAULT_SIZE) {
        /* Avoid requesting too much */
        result = NULL;
      } else {
        allocated_size = chars_requested;
        (void) ALLOC_STRI_SIZE_OK(result, allocated_size);
      } /* if */
      if (result == NULL) {
        bytes_there = remainingBytesInFile(cInFile);
        logMessage(printf("ut8Gets: bytes_there=" FMT_U_MEM "\n",
                          bytes_there););
        if (bytes_there != 0) {
          /* Now we know that bytes_there bytes are available in cInFile */
          if (chars_requested <= bytes_there) {
            allocated_size = chars_requested;
          } else {
            allocated_size = bytes_there;
          } /* if */
          logMessage(printf("ut8Gets: allocated_size=" FMT_U_MEM "\n",
                            allocated_size););
          if (unlikely(!ALLOC_STRI_CHECK_SIZE(result, allocated_size))) {
            /* printf("MAX_STRI_LEN=%lu, SIZ_STRI(MAX_STRI_LEN)=%lu\n",
                MAX_STRI_LEN, SIZ_STRI(MAX_STRI_LEN)); */
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
        } /* if */
      } /* if */
      if (result != NULL) {
        /* We have allocated a buffer for the requested number of chars
           or for the number of bytes which are available in the file */
        result->size = allocated_size;
        num_of_chars_read = read_utf8_string(cInFile, result, &err_info);
      } else {
        /* We do not know how many bytes are available therefore
           result is resized with GETS_STRI_SIZE_DELTA until we
           have read enough or we reach EOF */
        result = read_and_alloc_utf8_stri(cInFile, chars_requested, &num_of_chars_read,
                                          &err_info);
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        if (result != NULL) {
          FREE_STRI(result);
        } /* if */
        raise_error(err_info);
        result = NULL;
      } else if (num_of_chars_read < result->size) {
        REALLOC_STRI_SIZE_SMALLER(resized_result, result, result->size, num_of_chars_read);
        if (unlikely(resized_result == NULL)) {
          FREE_STRI(result);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result = resized_result;
          COUNT3_STRI(result->size, num_of_chars_read);
          result->size = num_of_chars_read;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("ut8Gets(%d, " FMT_D ") --> \"%s\"\n",
                       safe_fileno(cInFile), length, striAsUnquotedCStri(result)););
    return result;
  } /* ut8Gets */



/**
 *  Read a line from an UTF-8 file.
 *  The function accepts lines ending with '\n', "\r\n" or EOF.
 *  The line ending characters are not copied into the string.
 *  That means that the '\r' of a "\r\n" sequence is silently removed.
 *  When the function is left terminationChar contains '\n' or EOF.
 *  @return the line read.
 *  @exception RANGE_ERROR The file contains an invalid encoding.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 *  @exception FILE_ERROR A system function returns an error.
 */
striType ut8LineRead (fileType inFile, charType *terminationChar)

  {
    cFileType cInFile;
    register int ch;
    register memSizeType position;
    ucharType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    bstriType resized_buffer;
    bstriType buffer;
    memSizeType result_size;
    striType resized_result;
    striType result;

  /* ut8LineRead */
    logFunction(printf("ut8LineRead(%s%d, '\\" FMT_U32 ";')\n",
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       *terminationChar););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("ut8LineRead: Attempt to read from closed file.\n"););
      raise_error(FILE_ERROR);
      result = NULL;
#if FREAD_WRONG_FOR_WRITE_ONLY_FILES
    } else if (unlikely(!inFile->readingAllowed)) {
      logError(printf("ut8LineRead(%d, *): "
                      "The file is not open for reading.\n",
                      safe_fileno(cInFile)););
      raise_error(FILE_ERROR);
      result = NULL;
#endif
    } else {
      memlength = READ_STRI_INIT_SIZE;
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(buffer, memlength))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        memory = buffer->mem;
        position = 0;
        flockfile(cInFile);
        while ((ch = getc_unlocked(cInFile)) != (int) '\n' && ch != EOF) {
          if (position >= memlength) {
            newmemlength = memlength + READ_STRI_SIZE_DELTA;
            REALLOC_BSTRI_CHECK_SIZE(resized_buffer, buffer, memlength, newmemlength);
            if (unlikely(resized_buffer == NULL)) {
              FREE_BSTRI(buffer, memlength);
              funlockfile(cInFile);
              raise_error(MEMORY_ERROR);
              return NULL;
            } /* if */
            buffer = resized_buffer;
            COUNT3_BSTRI(memlength, newmemlength);
            memory = buffer->mem;
            memlength = newmemlength;
          } /* if */
          memory[position++] = (ucharType) ch;
        } /* while */
        funlockfile(cInFile);
        if (ch == (int) '\n' && position != 0 && memory[position - 1] == '\r') {
          position--;
        } /* if */
        if (unlikely(ch == EOF && position == 0 && ferror(cInFile))) {
          FREE_BSTRI(buffer, memlength);
          logError(printf("ut8LineRead(%d, '\\" FMT_U32 ";'): "
                          "getc_unlocked(%d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          safe_fileno(cInFile), *terminationChar,
                          safe_fileno(cInFile), errno, strerror(errno)););
          raise_error(FILE_ERROR);
          result = NULL;
        } else {
          if (unlikely(!ALLOC_STRI_CHECK_SIZE(result, position))) {
            FREE_BSTRI(buffer, memlength);
            raise_error(MEMORY_ERROR);
          } else {
            if (unlikely(utf8_to_stri(result->mem, &result_size, buffer->mem, position) != 0)) {
              FREE_BSTRI(buffer, memlength);
              FREE_STRI2(result, position);
              logError(printf("ut8LineRead(%d, '\\" FMT_U32 ";'): "
                              "The file contains an invalid encoding.\n",
                              safe_fileno(cInFile), *terminationChar););
              raise_error(RANGE_ERROR);
              result = NULL;
            } else {
              FREE_BSTRI(buffer, memlength);
              REALLOC_STRI_SIZE_OK(resized_result, result, position, result_size);
              if (unlikely(resized_result == NULL)) {
                FREE_STRI2(result, position);
                raise_error(MEMORY_ERROR);
                result = NULL;
              } else {
                result = resized_result;
                COUNT3_STRI(position, result_size);
                result->size = result_size;
                *terminationChar = (charType) ch;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("ut8LineRead(%d, '\\" FMT_U32 ";') --> \"%s\"\n",
                       safe_fileno(cInFile), *terminationChar,
                       striAsUnquotedCStri(result)););
    return result;
  } /* ut8LineRead */



/**
 *  Set the current file position.
 *  The file position is measured in bytes from the start of the file.
 *  The first byte in the file has the position 1.
 *  If the file position would be in the middle of an UTF-8 encoded
 *  character the position is advanced to the beginning of the next
 *  UTF-8 character.
 *  @exception RANGE_ERROR The file position is negative or zero or
 *             the file position is not representable in the system
 *             file position type.
 *  @exception FILE_ERROR The system function returns an error.
 */
void ut8Seek (fileType aFile, intType position)

  {
    cFileType cFile;
    int ch;
    int seekCorrection;

  /* ut8Seek */
    logFunction(printf("ut8Seek(%s%d, " FMT_D ")\n",
                       aFile == NULL ? "NULL " : "",
                       aFile != NULL ? safe_fileno(aFile->cFile) : 0,
                       position););
    cFile = aFile->cFile;
    if (unlikely(cFile == NULL)) {
      logError(printf("ut8Seek: Attempt to set the current position of a closed file.\n"););
      raise_error(FILE_ERROR);
    } else if (unlikely(position <= 0)) {
      logError(printf("ut8Seek(%d, " FMT_D "): Position <= 0.\n",
                      safe_fileno(cFile), position););
      raise_error(RANGE_ERROR);
#if OS_OFF_T_SIZE < INTTYPE_SIZE
#if OS_OFF_T_SIZE == 32
    } else if (unlikely(position > INT32TYPE_MAX)) {
      logError(printf("ut8Seek(%d, " FMT_D "): "
                      "Position not representable in the system file position type.\n",
                      safe_fileno(cFile), position););
      raise_error(RANGE_ERROR);
#elif OS_OFF_T_SIZE == 64
    } else if (unlikely(position > INT64TYPE_MAX)) {
      logError(printf("ut8Seek(%d, " FMT_D "): "
                      "Position not representable in the system file position type.\n",
                      safe_fileno(cFile), position););
      raise_error(RANGE_ERROR);
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif
#endif
    } else if (unlikely(offsetSeek(cFile, (os_off_t) (position - 1), SEEK_SET) != 0)) {
      logError(printf("ut8Seek(%d, " FMT_D "): "
                      "offsetSeek(%d, " FMT_D ", SEEK_SET) failed.\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(cFile), position,
                      safe_fileno(cFile), position - 1,
                      errno, strerror(errno)););
      raise_error(FILE_ERROR);
    } else {
      while ((ch = getc(cFile)) != EOF &&
             ch >= 0x80 && ch <= 0xBF) ;
      if (ch != EOF) {
        seekCorrection = -1;
      } else {
        seekCorrection = 0;
      } /* if */
      /* According to the specification of file I/O input   */
      /* shall not be directly followed by output without   */
      /* an intervening call to a file positioning function */
      /* (e.g. fseek()). For this reason a seek is done to  */
      /* allow that a write can directly follow ut8Seek().  */
      if (unlikely(offsetSeek(cFile, (os_off_t) seekCorrection, SEEK_CUR) != 0)) {
        logError(printf("ut8Seek(%d, " FMT_D "): "
                        "offsetSeek(%d, %d, SEEK_CUR) failed.\n"
                        "errno=%d\nerror: %s\n",
                        safe_fileno(cFile), position,
                        safe_fileno(cFile), seekCorrection,
                        errno, strerror(errno)););
        raise_error(FILE_ERROR);
      } /* if */
    } /* if */
  } /* ut8Seek */



/**
 *  Read a word from an UTF-8 file.
 *  Before reading the word it skips spaces and tabs. The function
 *  accepts words ending with ' ', '\t', '\n', "\r\n" or EOF.
 *  The word ending characters are not copied into the string.
 *  That means that the '\r' of a "\r\n" sequence is silently removed.
 *  When the function is left terminationChar contains ' ', '\t', '\n' or
 *  EOF.
 *  @return the word read.
 *  @exception RANGE_ERROR The file contains an invalid encoding.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 *  @exception FILE_ERROR A system function returns an error.
 */
striType ut8WordRead (fileType inFile, charType *terminationChar)

  {
    cFileType cInFile;
    register int ch;
    register memSizeType position;
    ucharType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    bstriType resized_buffer;
    bstriType buffer;
    memSizeType result_size;
    striType resized_result;
    striType result;

  /* ut8WordRead */
    logFunction(printf("ut8WordRead(%s%d, '\\" FMT_U32 ";')\n",
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0,
                       *terminationChar););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("ut8WordRead: Attempt to read from closed file.\n"););
      raise_error(FILE_ERROR);
      result = NULL;
#if FREAD_WRONG_FOR_WRITE_ONLY_FILES
    } else if (unlikely(!inFile->readingAllowed)) {
      logError(printf("ut8WordRead(%d, *): "
                      "The file is not open for reading.\n",
                      safe_fileno(cInFile)););
      raise_error(FILE_ERROR);
      result = NULL;
#endif
    } else {
      memlength = READ_STRI_INIT_SIZE;
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(buffer, memlength))) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        memory = buffer->mem;
        position = 0;
        flockfile(cInFile);
        do {
          ch = getc_unlocked(cInFile);
        } while (ch == (int) ' ' || ch == (int) '\t');
        while (ch != (int) ' ' && ch != (int) '\t' &&
            ch != (int) '\n' && ch != EOF) {
          if (position >= memlength) {
            newmemlength = memlength + READ_STRI_SIZE_DELTA;
            REALLOC_BSTRI_CHECK_SIZE(resized_buffer, buffer, memlength, newmemlength);
            if (unlikely(resized_buffer == NULL)) {
              FREE_BSTRI(buffer, memlength);
              funlockfile(cInFile);
              raise_error(MEMORY_ERROR);
              return NULL;
            } /* if */
            buffer = resized_buffer;
            COUNT3_BSTRI(memlength, newmemlength);
            memory = buffer->mem;
            memlength = newmemlength;
          } /* if */
          memory[position++] = (ucharType) ch;
          ch = getc_unlocked(cInFile);
        } /* while */
        funlockfile(cInFile);
        if (ch == (int) '\n' && position != 0 && memory[position - 1] == '\r') {
          position--;
        } /* if */
        if (unlikely(ch == EOF && position == 0 && ferror(cInFile))) {
          FREE_BSTRI(buffer, memlength);
          logError(printf("ut8WordRead(%d, '\\" FMT_U32 ";'): "
                          "getc_unlocked(%d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          safe_fileno(cInFile), *terminationChar,
                          safe_fileno(cInFile), errno, strerror(errno)););
          raise_error(FILE_ERROR);
          result = NULL;
        } else {
          if (unlikely(!ALLOC_STRI_CHECK_SIZE(result, position))) {
            FREE_BSTRI(buffer, memlength);
            raise_error(MEMORY_ERROR);
          } else {
            if (unlikely(utf8_to_stri(result->mem, &result_size, buffer->mem, position) != 0)) {
              FREE_BSTRI(buffer, memlength);
              FREE_STRI2(result, position);
              logError(printf("ut8WordRead(%d, '\\" FMT_U32 ";'): "
                              "The file contains an invalid encoding.\n",
                              safe_fileno(cInFile), *terminationChar););
              raise_error(RANGE_ERROR);
              result = NULL;
            } else {
              FREE_BSTRI(buffer, memlength);
              REALLOC_STRI_SIZE_OK(resized_result, result, position, result_size);
              if (unlikely(resized_result == NULL)) {
                FREE_STRI2(result, position);
                raise_error(MEMORY_ERROR);
                result = NULL;
              } else {
                result = resized_result;
                COUNT3_STRI(position, result_size);
                result->size = result_size;
                *terminationChar = (charType) ch;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("ut8WordRead(%d, '\\" FMT_U32 ";') --> \"%s\"\n",
                       safe_fileno(cInFile), *terminationChar,
                       striAsUnquotedCStri(result)););
    return result;
  } /* ut8WordRead */



/**
 *  Write a string to an UTF-8 file.
 *  @exception FILE_ERROR A system function returns an error.
 */
void ut8Write (fileType outFile, const const_striType stri)

  {
    cFileType cOutFile;
    const strElemType *str;
    memSizeType len;
    memSizeType size;
    ucharType stri_buffer[max_utf8_size(WRITE_STRI_BLOCK_SIZE)];

  /* ut8Write */
    logFunction(printf("ut8Write(%s%d, \"%s\")\n",
                       outFile == NULL ? "NULL " : "",
                       outFile != NULL ? safe_fileno(outFile->cFile) : 0,
                       striAsUnquotedCStri(stri)););
    cOutFile = outFile->cFile;
    if (unlikely(cOutFile == NULL)) {
      logError(printf("ut8Write: Attempt to write to closed file.\n"););
      raise_error(FILE_ERROR);
      return;
    } /* if */
#if FWRITE_WRONG_FOR_READ_ONLY_FILES
    if (unlikely(stri->size > 0 && !outFile->writingAllowed)) {
      logError(printf("ut8Write: Attempt to write to read only file: %d.\n",
                      safe_fileno(cOutFile)););
      raise_error(FILE_ERROR);
      return;
    } /* if */
#endif
    for (str = stri->mem, len = stri->size; len >= WRITE_STRI_BLOCK_SIZE;
        str += WRITE_STRI_BLOCK_SIZE, len -= WRITE_STRI_BLOCK_SIZE) {
      size = stri_to_utf8(stri_buffer, str, WRITE_STRI_BLOCK_SIZE);
      if (unlikely(size != fwrite(stri_buffer, 1, (size_t) size, cOutFile))) {
        logError(printf("ut8Write: fwrite(*, 1, " FMT_U_MEM ", %d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        size, safe_fileno(cOutFile),
                        errno, strerror(errno)););
        raise_error(FILE_ERROR);
        return;
      } /* if */
    } /* for */
    if (len > 0) {
      size = stri_to_utf8(stri_buffer, str, len);
      if (unlikely(size != fwrite(stri_buffer, 1, (size_t) size, cOutFile))) {
        logError(printf("ut8Write: fwrite(*, 1, " FMT_U_MEM ", %d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        size, safe_fileno(cOutFile),
                        errno, strerror(errno)););
        raise_error(FILE_ERROR);
        return;
      } /* if */
    } /* if */
    logFunction(printf("ut8Write -->\n"););
  } /* ut8Write */
