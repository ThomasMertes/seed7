/********************************************************************/
/*                                                                  */
/*  chr_rtl.c     Primitive actions for the integer type.           */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
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
/*  Changes: 1992, 1993, 1994, 2005, 2010  Thomas Mertes            */
/*  Content: Primitive actions for the char type.                   */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "int_rtl.h"
#include "str_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "chr_rtl.h"



striType chrCLit (charType character)

  {
    memSizeType len;
    striType result;

  /* chrCLit */
    /* printf("chrCLit(%lu)\n", character); */
    if (character < 127) {
      if (character < ' ') {
        len = strlen(cstri_escape_sequence[character]);
        if (!ALLOC_STRI_SIZE_OK(result, len + 2)) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = len + 2;
          result->mem[0] = '\'';
          cstri_expand(&result->mem[1],
              cstri_escape_sequence[character], len);
          result->mem[len + 1] = '\'';
        } /* if */
      } else if (character == '\\' || character == '\'') {
        if (!ALLOC_STRI_SIZE_OK(result, (memSizeType) 4)) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 4;
          result->mem[0] = '\'';
          result->mem[1] = (strElemType) '\\';
          result->mem[2] = (strElemType) character;
          result->mem[3] = '\'';
        } /* if */
      } else {
        if (!ALLOC_STRI_SIZE_OK(result, (memSizeType) 3)) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 3;
          result->mem[0] = '\'';
          result->mem[1] = (strElemType) character;
          result->mem[2] = '\'';
        } /* if */
      } /* if */
    } else {
      result = intStr((intType) character);
    } /* if */
    return result;
  } /* chrCLit */



#ifdef ALLOW_STRITYPE_SLICES
striType chrCLitToBuffer (charType character, striType buffer)

  { /* chrCLitToBuffer */
    /* printf("chrCLitToBuffer(%lu)\n", (unsigned long) character); */
    if (character < 127) {
      buffer->mem = buffer->mem1;
      buffer->mem1[0] = (strElemType) '\'';
      if (character < ' ') {
        buffer->mem1[1] = (strElemType) '\\';
        if (cstri_escape_sequence[character][1] == '0') {
          /* Always write three octal digits as strCLit does. */
          buffer->mem1[2] = (strElemType) '0';
          /* Write the character as two octal digits. */
          /* This code is much faster than sprintf(). */
          buffer->mem1[3] = (strElemType) ((character >> 3 & 0x7) + '0');
          buffer->mem1[4] = (strElemType) ((character      & 0x7) + '0');
          buffer->mem1[5] = (strElemType) '\'';
          buffer->size = 6;
        } else {
          buffer->mem1[2] = (strElemType) cstri_escape_sequence[character][1];
          buffer->mem1[3] = (strElemType) '\'';
          buffer->size = 4;
        } /* if */
      } else if (character == '\\' || character == '\'') {
        buffer->mem1[1] = (strElemType) '\\';
        buffer->mem1[2] = (strElemType) character;
        buffer->mem1[3] = (strElemType) '\'';
        buffer->size = 4;
      } else {
        buffer->mem1[1] = (strElemType) character;
        buffer->mem1[2] = (strElemType) '\'';
        buffer->size = 3;
      } /* if */
    } else {
      intStrToBuffer((intType) character, buffer);
    } /* if */
    return buffer;
  } /* chrCLitToBuffer */
#endif



/**
 *  Compare two characters.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
inline intType chrCmp (charType char1, charType char2)

  { /* chrCmp */
    if (char1 < char2) {
      return -1;
    } else if (char1 > char2) {
      return 1;
    } else {
      return 0;
    } /* if */
  } /* chrCmp */



/**
 *  Reinterpret the generic parameters as charType and call chrCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(charType).
 */
intType chrCmpGeneric (const genericType value1, const genericType value2)

  { /* chrCmpGeneric */
    return chrCmp(((const_rtlObjectType *) &value1)->value.charValue,
                  ((const_rtlObjectType *) &value2)->value.charValue);
  } /* chrCmpGeneric */



void chrCpy (charType *dest, charType source)

  { /* chrCpy */
    *dest = source;
  } /* chrCpy */



/**
 *  Convert a character to lower case.
 *  The conversion uses the default Unicode case mapping,
 *  where each character is considered in isolation.
 *  Characters without case mapping are left unchanged.
 *  The mapping is independend from the locale. Individual
 *  character case mappings cannot be reversed, because some
 *  characters have multiple characters that map to them.
 *  @return the character converted to lower case.
 */
charType chrLow (charType ch)

  { /* chrLow */
    toLower(&ch, 1, &ch);
    return ch;
  } /* chrLow */



/**
 *  Create a string with one character.
 *  @return a string with the character 'ch'.
 */
striType chrStr (charType ch)

  {
    striType result;

  /* chrStr */
    if (!ALLOC_STRI_SIZE_OK(result, (memSizeType) 1)) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = 1;
      result->mem[0] = (strElemType) ch;
      return result;
    } /* if */
  } /* chrStr */



/**
 *  Convert a character to upper case.
 *  The conversion uses the default Unicode case mapping,
 *  where each character is considered in isolation.
 *  Characters without case mapping are left unchanged.
 *  The mapping is independend from the locale. Individual
 *  character case mappings cannot be reversed, because some
 *  characters have multiple characters that map to them.
 *  @return the character converted to upper case.
 */
charType chrUp (charType ch)

  { /* chrUp */
    toUpper(&ch, 1, &ch);
    return ch;
  } /* chrUp */
