/********************************************************************/
/*                                                                  */
/*  flt_rtl.c     Primitive actions for the float type.             */
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
/*  File: seed7/src/flt_rtl.c                                       */
/*  Changes: 1993, 1994, 2005  Thomas Mertes                        */
/*  Content: Primitive actions for the float type.                  */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"
#include "int_rtl.h"

#undef EXTERN
#define EXTERN
#include "flt_rtl.h"


#define USE_STRTOD
#define IPOW_EXPONENTIATION_BY_SQUARING

#ifdef FLOAT_ZERO_DIV_ERROR
union { unsigned long l_val; float f_val; } f_const[] = {0xffc00000, 0x7f800000, 0xff800000};
#define NOT_A_NUMBER      f_const[0].f_val
#define POSITIVE_INFINITY f_const[1].f_val
#define NEGATIVE_INFINITY f_const[2].f_val
#else
#define NOT_A_NUMBER      ( 0.0 / 0.0)
#define POSITIVE_INFINITY ( 1.0 / 0.0)
#define NEGATIVE_INFINITY (-1.0 / 0.0)
#endif

#ifndef USE_VARIABLE_FORMATS
#define MAX_FORM 28

static char *form[] = {"%1.0f", "%1.1f", "%1.2f", "%1.3f", "%1.4f",
    "%1.5f",  "%1.6f",  "%1.7f",  "%1.8f",  "%1.9f",  "%1.10f",
    "%1.11f", "%1.12f", "%1.13f", "%1.14f", "%1.15f", "%1.16f",
    "%1.17f", "%1.18f", "%1.19f", "%1.20f", "%1.21f", "%1.22f",
    "%1.23f", "%1.24f", "%1.25f", "%1.26f", "%1.27f", "%1.28f"};
#endif



#ifdef ANSI_C

inttype fltCmp (floattype number1, floattype number2)
#else

inttype fltCmp (number1, number2)
floattype number1;
floattype number2;
#endif

  { /* fltCmp */
    if (number1 < number2) {
      return(-1);
    } else if (number1 > number2) {
      return(1);
    } else {
      return(0);
    } /* if */
  } /* fltCmp */



#ifdef ANSI_C

void fltCpy (floattype *dest, floattype source)
#else

void fltCpy (dest, source)
floattype *dest;
floattype source;
#endif

  { /* fltCpy */
    *dest = source;
  } /* fltCpy */



#ifdef ANSI_C

generictype fltCreate (generictype source)
#else

generictype fltCreate (source)
generictype source;
#endif

  { /* fltCreate */
    return(source);
  } /* fltCreate */



#ifdef ANSI_C

stritype fltDgts (floattype number, inttype digits_precision)
#else

stritype fltDgts (number, digits_precision)
floattype number;
inttype digits_precision;
#endif

  {
    char buffer[2001];
    char *buffer_ptr;
#ifndef USE_VARIABLE_FORMATS
    char form_buffer[10];
#endif
    memsizetype len;
    stritype result;

  /* fltDgts */
    if (digits_precision < 0) {
      digits_precision = 0;
    } /* if */
    if (digits_precision > 1000) {
      digits_precision = 1000;
    } /* if */
    if (isnan(number)) {
      strcpy(buffer, "NaN");
    } else if (number == POSITIVE_INFINITY) {
      strcpy(buffer, "Infinity");
    } else if (number == NEGATIVE_INFINITY) {
      strcpy(buffer, "-Infinity");
    } else {
#ifdef USE_VARIABLE_FORMATS
      sprintf(buffer, "%1.*f", (int) digits_precision, number);
#else
      if (digits_precision > MAX_FORM) {
        sprintf(form_buffer, "%%1.%ldf", digits_precision);
        sprintf(buffer, form_buffer, number);
      } else {
        sprintf(buffer, form[digits_precision], number);
      } /* if */
#endif
    } /* if */
    buffer_ptr = buffer;
    if (buffer[0] == '-' && buffer[1] == '0') {
      /* All forms of -0 are converted to 0 */
      if (buffer[2] == '.') {
        len = 3;
        while (buffer[len] == '0') {
          len++;
        } /* while */
        if (buffer[len] == '\0') {
          buffer_ptr++;
        } /* if */
      } else if (buffer[2] == '\0') {
        buffer_ptr++;
      } /* if */
    } /* if */
    len = strlen(buffer_ptr);
    if (!ALLOC_STRI(result, len)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->size = len;
      cstri_expand(result->mem, buffer_ptr, len);
      return(result);
    } /* if */
  } /* fltDgts */



#ifdef ANSI_C

floattype fltIPow (floattype base, inttype exponent)
#else

floattype fltIPow (base, exponent)
floattype base;
inttype exponent;
#endif

  {
    booltype neg_exp = FALSE;
    floattype result;

  /* fltIPow */
#ifdef IPOW_EXPONENTIATION_BY_SQUARING
    if (base == 0.0) {
      if (exponent < 0) {
        return(POSITIVE_INFINITY);
      } else if (exponent == 0) {
        return(1.0);
      } else {
        return(0.0);
      } /* if */
    } else {
      if (exponent < 0) {
        exponent = -exponent;
        if (exponent < 0) {
          return(0.0);
        } /* if */
        neg_exp = TRUE;
      } /* if */
      if (exponent & 1) {
        result = base;
      } else {
        result = 1;
      } /* if */
      exponent >>= 1;
      while (exponent != 0) {
        base *= base;
        if (exponent & 1) {
          result *= base;
        } /* if */
        exponent >>= 1;
      } /* while */
      if (neg_exp) {
        return(1.0 / result);
      } else {
        return(result);
      } /* if */
    } /* if */
#else
    if (base < 0.0) {
      if (exponent & 1) {
        return(-pow(-base, (floattype) exponent));
      } else {
        return(pow(-base, (floattype) exponent));
      } /* if */
    } else if (base == 0.0) {
      if (exponent < 0) {
        return(POSITIVE_INFINITY);
      } else if (exponent == 0) {
        return(1.0);
      } else {
        return(0.0);
      } /* if */
    } else { /* base > 0.0 */
      return(pow(base, (floattype) exponent));
    } /* if */
#endif
  } /* fltIPow */



#ifdef ANSI_C

floattype fltParse (stritype stri)
#else

floattype fltParse (stri)
stritype stri;
#endif

  {
    booltype okay;
#ifdef USE_STRTOD
    char buffer[150];
    char *next_ch;
#else
    memsizetype position;
    floattype digitval;
#endif
    floattype result;

  /* fltParse */
    okay = TRUE;
#ifdef USE_STRTOD
    if (compr_size(stri) + 1 <= 150) {
      stri_export((ustritype) buffer, stri);
/*    result = (floattype) atof(buffer); */
      result = (floattype) strtod(buffer, &next_ch);
      if (next_ch == buffer) {
        if (strcmp(buffer, "NaN") == 0) {
          result = NOT_A_NUMBER;
        } else if (strcmp(buffer, "Infinity") == 0) {
          result = POSITIVE_INFINITY;
        } else if (strcmp(buffer, "-Infinity") == 0) {
          result = NEGATIVE_INFINITY;
        } else {
          okay = FALSE;
        } /* if */
      } else if (next_ch != &buffer[stri->size]) {
        okay = FALSE;
      } /* if */
    } else {
      okay = FALSE;
    } /* if */
#else
    position = 0;
    result = 0.0;
    while (position < stri->size &&
        stri->mem[position] >= ((strelemtype) '0') &&
        stri->mem[position] <= ((strelemtype) '9')) {
      digitval = ((inttype) stri->mem[position]) - ((inttype) '0');
      if (result < MAX_DIV_10 ||
          (result == MAX_DIV_10 &&
          digitval <= MAX_REM_10)) {
        result = ((floattype) 10.0) * result + digitval;
      } else {
        okay = FALSE;
      } /* if */
      position++;
    } /* while */
    if (position == 0 || position < stri->size) {
      okay = FALSE;
    } /* if */
#endif
    if (okay) {
      return(result);
    } else {
      raise_error(RANGE_ERROR);
      return(0.0);
    } /* if */
  } /* fltParse */



#ifdef ANSI_C

floattype fltRand (floattype lower_limit, floattype upper_limit)
#else

floattype fltRand (lower_limit, upper_limit)
floattype lower_limit;
floattype upper_limit;
#endif

  {
    floattype factor;
    floattype result;

  /* fltRand */
    if (lower_limit > upper_limit) {
      raise_error(RANGE_ERROR);
      return(0.0);
    } else {
      factor = upper_limit - lower_limit;
      do {
        result = ((floattype) rand_32()) / ((floattype) ULONG_MAX);
        result = lower_limit + factor * result;
      } while (result < lower_limit || result > upper_limit);
      return(result);
    } /* if */
  } /* fltRand */



#ifdef ANSI_C

stritype fltStr (floattype number)
#else

stritype fltStr (number)
floattype number;
#endif

  {
    char buffer[2001];
    memsizetype len;
    stritype result;

  /* fltStr */
    if (isnan(number)) {
      strcpy(buffer, "NaN");
    } else if (number == POSITIVE_INFINITY) {
      strcpy(buffer, "Infinity");
    } else if (number == NEGATIVE_INFINITY) {
      strcpy(buffer, "-Infinity");
    } else {
      sprintf(buffer, "%1.25f", number);
    } /* if */
    len = strlen(buffer);
    do {
      len--;
    } while (len >= 1 && buffer[len] == '0');
    if (buffer[len] == '.') {
      len++;
    } /* if */
    len++;
    if (!ALLOC_STRI(result, len)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->size = len;
      cstri_expand(result->mem, buffer, (SIZE_TYPE) len);
      return(result);
    } /* if */
  } /* fltStr */
