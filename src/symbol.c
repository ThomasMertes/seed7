/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000, 2021  Thomas Mertes                  */
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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Analyzer - Scanner                                      */
/*  File: seed7/src/symbol.c                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994, 2021  Thomas Mertes      */
/*  Content: Maintains the current symbol of the scanner.           */
/*                                                                  */
/*  Every time a longer symbol.name is needed it is extended by     */
/*  INCR_SYMB_LENGTH bytes. The symbol.name never shrinks. If the   */
/*  symbol.name would become longer than MAX_SYMB_LENGTH bytes      */
/*  fatal_memory_error is called to terminate the program.          */
/*  Although this is unlikely to happen it is absolutely necessary  */
/*  to be sure to avoid an overflow of symbol.name_length. Note     */
/*  that this is a misuse of fatal_memory_error. It would be        */
/*  possible to issue a normal error message instead, but this      */
/*  would pollute the scanner with additional checks.               */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "limits.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "fatal.h"
#include "big_drv.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "symbol.h"


#define INCR_SYMB_LENGTH   256



void extend_symb_length (void)

  {
    sySizeType newNameLength;
    ustriType resizedName;

  /* extend_symb_length */
    if (symbol.name_length > MAX_SYMB_LENGTH - INCR_SYMB_LENGTH) {
      fatal_memory_error(SOURCE_POSITION(2011));
    } else {
      newNameLength = symbol.name_length + INCR_SYMB_LENGTH;
      resizedName = REALLOC_USTRI(symbol.name, symbol.name_length, newNameLength);
      if (resizedName == NULL) {
        fatal_memory_error(SOURCE_POSITION(2012));
      } else {
        symbol.name_length = newNameLength;
        symbol.name = resizedName;
        COUNT3_USTRI(symbol.name_length - INCR_SYMB_LENGTH, symbol.name_length,
            count.symb, count.symb_bytes);
      } /* if */
    } /* if */
  } /* extend_symb_length */



void extend_stri_length (void)

  {
    memSizeType newStriMax;
    striType resizedStriValue;

  /* extend_stri_length */
    if (symbol.stri_max > MAX_STRI_LEN - INCR_SYMB_LENGTH) {
      fatal_memory_error(SOURCE_POSITION(2013));
    } else {
      newStriMax = symbol.stri_max + INCR_SYMB_LENGTH;
      REALLOC_STRI_SIZE_OK2(resizedStriValue, symbol.striValue,
          symbol.stri_max, newStriMax);
      if (symbol.striValue == NULL) {
        fatal_memory_error(SOURCE_POSITION(2014));
      } else {
        symbol.stri_max = newStriMax;
        symbol.striValue = resizedStriValue;
        COUNT3_STRI(symbol.stri_max - INCR_SYMB_LENGTH, symbol.stri_max);
      } /* if */
    } /* if */
  } /* extend_stri_length */



void init_symbol (errInfoType *err_info)

  { /* init_symbol */
    symbol.name_length = INCR_SYMB_LENGTH;
    if (!ALLOC_USTRI(symbol.name, symbol.name_length)) {
      *err_info = MEMORY_ERROR;
    } else {
      COUNT_USTRI(symbol.name_length, count.symb, count.symb_bytes);
      if (!ALLOC_STRI_CHECK_SIZE(symbol.striValue, symbol.stri_max)) {
        *err_info = MEMORY_ERROR;
      } /* if */
    } /* if */
    symbol.unicodeNames = FALSE;
  } /* init_symbol */



void close_symbol (void)

  { /* close_symbol */
    if (symbol.name != NULL) {
      FREE_USTRI(symbol.name, symbol.name_length, count.symb, count.symb_bytes);
    } /* if */
    symbol.name = NULL;
    symbol.name_length = 0;
    bigDestr(symbol.bigIntValue);
    symbol.bigIntValue = NULL;
    if (symbol.striValue != NULL) {
      FREE_STRI2(symbol.striValue, symbol.stri_max);
      symbol.striValue = NULL;
    } /* if */
    symbol.stri_max = 0;
  } /* close_symbol */
