/********************************************************************/
/*                                                                  */
/*  chr_rtl.c     Primitive actions for the integer type.           */
/*  Copyright (C) 1989 - 2006  Thomas Mertes                        */
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
/*  File: seed7/src/chr_rtl.c                                       */
/*  Changes: 1992, 1993, 1994, 2005  Thomas Mertes                  */
/*  Content: Primitive actions for the char type.                   */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "heaputl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "chr_rtl.h"



#ifdef ANSI_C

inttype chrCmp (chartype char1, chartype char2)
#else

inttype chrCmp (char1, char2)
chartype char1;
chartype char2;
#endif

  {

  /* chrCmp */
    if (char1 < char2) {
      return(-1);
    } else if (char1 > char2) {
      return(1);
    } else {
      return(0);
    } /* if */
  } /* chrCmp */



#ifdef ANSI_C

void chrCpy (chartype *dest, chartype source)
#else

void chrCpy (dest, source)
chartype *dest;
chartype source;
#endif

  { /* chrCpy */
    *dest = source;
  } /* chrCpy */



#ifdef ANSI_C

chartype chrCreate (chartype source)
#else

chartype chrCreate (source)
chartype source;
#endif

  { /* chrCreate */
    return(source);
  } /* chrCreate */



#ifdef ANSI_C

chartype chrLow (chartype ch)
#else

chartype chrLow (ch)
chartype ch;
#endif

  { /* chrLow */
    if (ch >= (chartype) 'A' && ch <= (chartype) 'Z') {
      return(ch - (chartype) 'A' + (chartype) 'a');
    } else {
      return(ch);
    } /* if */
  } /* chrLow */



#ifdef ANSI_C

stritype chrStr (chartype ch)
#else

stritype chrStr (ch)
chartype ch;
#endif

  {
    stritype result;

  /* chrStr */
#ifndef UTF32_STRINGS
    if (ch > (chartype) 255) {
      raise_error(RANGE_ERROR);
      return(NULL);
    } else {
#endif
      if (!ALLOC_STRI(result, 1)) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        result->size = 1;
        result->mem[0] = (strelemtype) ch;
        return(result);
      } /* if */
#ifndef UTF32_STRINGS
    } /* if */
#endif
  } /* chrStr */



#ifdef ANSI_C

chartype chrUp (chartype ch)
#else

chartype chrUp (ch)
chartype ch;
#endif

  { /* chrUp */
    if (ch >= (chartype) 'a' && ch <= (chartype) 'z') {
      return(ch - (chartype) 'a' + (chartype) 'A');
    } else {
      return(ch);
    } /* if */
  } /* chrUp */
