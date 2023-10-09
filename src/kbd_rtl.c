/********************************************************************/
/*                                                                  */
/*  kbd_rtl.c     Platform independent console keyboard support.    */
/*  Copyright (C) 1989 - 2012, 2021  Thomas Mertes                  */
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
/*  File: seed7/src/kbd_rtl.c                                       */
/*  Changes: 1992 - 1994, 2006, 2010 - 2012, 2021  Thomas Mertes    */
/*  Content: Platform independent console keyboard support.         */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"
#include "kbd_drv.h"

#undef EXTERN
#define EXTERN
#include "kbd_rtl.h"


#define READ_STRI_INIT_SIZE      256
#define READ_STRI_SIZE_DELTA    2048



/**
 *  Read a string with maximum length from the console keyboard file.
 *  @return the string read.
 */
striType kbdGets (intType length)

  {
    memSizeType bytes_requested;
    memSizeType position;
    striType result;

  /* kbdGets */
    logFunction(printf("kbdGets(" FMT_D ")\n", length););
    if (length < 0) {
      logError(printf("kbdGets(" FMT_D "): Negative length.\n", length););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if ((uintType) length > MAX_MEMSIZETYPE) {
        bytes_requested = MAX_MEMSIZETYPE;
      } else {
        bytes_requested = (memSizeType) length;
      } /* if */
      if (!ALLOC_STRI_CHECK_SIZE(result, bytes_requested)) {
        raise_error(MEMORY_ERROR);
      } else {
        for (position = 0; position < bytes_requested; position++) {
          result->mem[position] = (strElemType) kbdGetc();
        } /* for */
        result->size = bytes_requested;
      } /* if */
    } /* if */
    logFunction(printf("kbdGets(" FMT_D ") --> \"%s\"\n",
                       length, striAsUnquotedCStri(result)););
    return result;
  } /* kbdGets */



/**
 *  Read a line from the console keyboard file.
 *  The function accepts lines ending with "\n", "\r\n" or EOF.
 *  The line ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left terminationChar contains '\n' or EOF.
 *  @return the line read.
 */
striType kbdLineRead (charType *terminationChar)

  {
    register charType ch;
    register memSizeType position;
    strElemType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    striType resized_result;
    striType result;

  /* kbdLineRead */
    logFunction(printf("kbdLineRead('\\" FMT_U32 ";')\n", *terminationChar););
    memlength = READ_STRI_INIT_SIZE;
    if (!ALLOC_STRI_SIZE_OK(result, memlength)) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      while ((ch = kbdGetc()) != '\n' && ch != (charType) ((scharType) EOF)) {
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
        memory[position++] = (strElemType) ch;
      } /* while */
      if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      REALLOC_STRI_SIZE_SMALLER(resized_result, result, memlength, position);
      if (resized_result == NULL) {
        FREE_STRI(result, memlength);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(memlength, position);
        result->size = position;
        *terminationChar = ch;
      } /* if */
    } /* if */
    logFunction(printf("kbdLineRead('\\" FMT_U32 ";') --> \"%s\"\n",
                       *terminationChar, striAsUnquotedCStri(result)););
    return result;
  } /* kbdLineRead */



/**
 *  Read a word from the console keyboard file.
 *  Before reading the word it skips spaces and tabs. The function
 *  accepts words ending with " ", "\t", "\n", "\r\n" or EOF.
 *  The word ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left terminationChar contains ' ',
 *  '\t', '\n' or EOF.
 *  @return the word read.
 */
striType kbdWordRead (charType *terminationChar)

  {
    register charType ch;
    register memSizeType position;
    strElemType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    striType resized_result;
    striType result;

  /* kbdWordRead */
    logFunction(printf("kbdWordRead('\\" FMT_U32 ";')\n", *terminationChar););
    memlength = READ_STRI_INIT_SIZE;
    if (!ALLOC_STRI_SIZE_OK(result, memlength)) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      do {
        ch = kbdGetc();
      } while (ch == ' ' || ch == '\t');
      while (ch != ' ' && ch != '\t' &&
             ch != '\n' && ch != (charType) ((scharType) EOF)) {
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
        memory[position++] = (strElemType) ch;
        ch = kbdGetc();
      } /* while */
      if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      REALLOC_STRI_SIZE_SMALLER(resized_result, result, memlength, position);
      if (resized_result == NULL) {
        FREE_STRI(result, memlength);
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        result = resized_result;
        COUNT3_STRI(memlength, position);
        result->size = position;
        *terminationChar = ch;
      } /* if */
    } /* if */
    logFunction(printf("kbdWordRead('\\" FMT_U32 ";') --> \"%s\"\n",
                       *terminationChar, striAsUnquotedCStri(result)););
    return result;
  } /* kbdWordRead */
