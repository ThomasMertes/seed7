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



/**
 *  Read a string with maximum length from the graphic keyboard file.
 *  @return the string read.
 */
striType gkbGets (intType length)

  {
    memSizeType bytes_requested;
    memSizeType position;
    striType result;

  /* gkbGets */
    if (length < 0) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      if ((uintType) length > MAX_MEMSIZETYPE) {
        bytes_requested = MAX_MEMSIZETYPE;
      } else {
        bytes_requested = (memSizeType) length;
      } /* if */
      if (!ALLOC_STRI_CHECK_SIZE(result, bytes_requested)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        for (position = 0; position < bytes_requested; position++) {
          result->mem[position] = (strElemType) gkbGetc();
        } /* for */
        result->size = bytes_requested;
        return result;
      } /* if */
    } /* if */
  } /* gkbGets */



/**
 *  Read a line from the console keyboard file.
 *  The function accepts lines ending with "\n", "\r\n" or EOF.
 *  The line ending characters are not copied into the string.
 *  That means that the "\r" of a "\r\n" sequence is silently removed.
 *  When the function is left terminationChar contains '\n' or EOF.
 *  @return the line read.
 */
striType gkbLineRead (charType *terminationChar)

  {
    register charType ch;
    register memSizeType position;
    strElemType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    striType resized_result;
    striType result;

  /* gkbLineRead */
    memlength = READ_STRI_INIT_SIZE;
    if (!ALLOC_STRI_SIZE_OK(result, memlength)) {
      raise_error(MEMORY_ERROR);
    } else {
      memory = result->mem;
      position = 0;
      while ((ch = gkbGetc()) != '\n' && ch != (charType) ((scharType) EOF)) {
        if (position >= memlength) {
          newmemlength = memlength + READ_STRI_SIZE_DELTA;
          REALLOC_STRI_CHECK_SIZE2(resized_result, result, memlength, newmemlength);
          if (resized_result == NULL) {
            FREE_STRI2(result, memlength);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
          result = resized_result;
          memory = result->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strElemType) ch;
      } /* while */
      if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      REALLOC_STRI_SIZE_SMALLER2(resized_result, result, memlength, position);
      if (resized_result == NULL) {
        FREE_STRI2(result, memlength);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        result->size = position;
        *terminationChar = ch;
      } /* if */
    } /* if */
    return result;
  } /* gkbLineRead */



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
striType gkbWordRead (charType *terminationChar)

  {
    register charType ch;
    register memSizeType position;
    strElemType *memory;
    memSizeType memlength;
    memSizeType newmemlength;
    striType resized_result;
    striType result;

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
          ch != '\n' && ch != (charType) ((scharType) EOF)) {
        if (position >= memlength) {
          newmemlength = memlength + READ_STRI_SIZE_DELTA;
          REALLOC_STRI_CHECK_SIZE2(resized_result, result, memlength, newmemlength);
          if (resized_result == NULL) {
            FREE_STRI2(result, memlength);
            raise_error(MEMORY_ERROR);
            return NULL;
          } /* if */
          result = resized_result;
          memory = result->mem;
          memlength = newmemlength;
        } /* if */
        memory[position++] = (strElemType) ch;
        ch = gkbGetc();
      } /* while */
      if (ch == '\n' && position != 0 && memory[position - 1] == '\r') {
        position--;
      } /* if */
      REALLOC_STRI_SIZE_SMALLER2(resized_result, result, memlength, position);
      if (resized_result == NULL) {
        FREE_STRI2(result, memlength);
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        result = resized_result;
        result->size = position;
        *terminationChar = ch;
      } /* if */
    } /* if */
    return result;
  } /* gkbWordRead */
