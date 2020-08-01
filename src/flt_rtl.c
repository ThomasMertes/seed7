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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
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


#define USE_ATOF

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

void fltCreate (floattype *dest, floattype source)
#else

void fltCreate (dest, source)
floattype *dest;
floattype source;
#endif

  { /* fltCreate */
    *dest = source;
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
    len = strlen(buffer);
    if (!ALLOC_STRI(result, len)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      COUNT_STRI(len);
      result->size = len;
      stri_expand(result->mem, buffer, len);
      return(result);
    } /* if */
  } /* fltDgts */



#ifdef ANSI_C

floattype fltIpow (floattype base, inttype exponent)
#else

floattype fltIpow (base, exponent)
floattype base;
inttype exponent;
#endif

  { /* fltIpow */
    if (base < 0.0) {
      if (exponent & 1) {
        return(-pow(-base, (floattype) exponent));
      } else {
        return(pow(-base, (floattype) exponent));
      } /* if */
    } else if (base == 0.0) {
      if (exponent <= 0) {
        raise_error(NUMERIC_ERROR);
        return(0.0);
      } else {
        return(0.0);
      } /* if */
    } else { /* base > 0.0 */
      return(pow(base, (floattype) exponent));
    } /* if */
  } /* fltIpow */



#ifdef ANSI_C

floattype fltParse (stritype stri)
#else

floattype fltParse (stri)
stritype stri;
#endif

  {
    booltype okay;
#ifdef USE_ATOF
    char buffer[150];
    char *next_ch;
#else
    memsizetype position;
    floattype digitval;
#endif
    floattype result;

  /* fltParse */
    okay = TRUE;
#ifdef USE_ATOF
    if (compr_size(stri) + 1 <= 150) {
      stri_export(buffer, stri);
/*    result = (floattype) atof((cstritype) buffer); */
      result = (floattype) strtod((cstritype) buffer, &next_ch);
      if (next_ch == buffer) {
        okay = FALSE;
      } /* if */
    } else {
      result = 0.0;
      okay = FALSE;
    } /* if */
#else
    position = 0;
    result = 0.0;
    while (position < stri->SIZE &&
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
    if (position == 0 || position < stri->SIZE) {
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

floattype fltPow (floattype base, floattype exponent)
#else

floattype fltPow (base, exponent)
floattype base;
floattype exponent;
#endif

  { /* fltPow */
    if (base < 0.0) {
      raise_error(NUMERIC_ERROR);
      return(0.0);
    } else if (base == 0.0) {
      if (exponent <= 0.0) {
        raise_error(NUMERIC_ERROR);
        return(0.0);
      } else {
        return(0.0);
      } /* if */
    } else { /* base > 0.0 */
      return(pow(base, exponent));
    } /* if */
  } /* fltPow */



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
    sprintf(buffer, "%1.25f", number);
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
      COUNT_STRI(len);
      result->size = len;
      stri_expand(result->mem, buffer, (SIZE_TYPE) len);
      return(result);
    } /* if */
  } /* fltStr */
