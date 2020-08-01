/********************************************************************/
/*                                                                  */
/*  bst_rtl.c     Primitive actions for the byte string type.       */
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
/*  File: seed7/src/bst_rtl.c                                       */
/*  Changes: 2007, 2010  Thomas Mertes                              */
/*  Content: Primitive actions for the byte string type.            */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "bst_rtl.h"



/**
 *  Append the bstring 'extension' to 'destination'.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             bstring.
 */
void bstAppend (bstriType *const destination, const_bstriType extension)

  {
    memSizeType new_size;
    bstriType bstri_dest;
    bstriType new_bstri;

  /* bstAppend */
    bstri_dest = *destination;
    if (unlikely(bstri_dest->size > MAX_BSTRI_LEN - extension->size)) {
      /* number of bytes does not fit into memSizeType */
      raise_error(MEMORY_ERROR);
    } else {
      new_size = bstri_dest->size + extension->size;
      REALLOC_BSTRI_SIZE_OK(new_bstri, bstri_dest, bstri_dest->size, new_size);
      if (unlikely(new_bstri == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        if (bstri_dest == extension) {
          /* It is possible that bstri_dest == extension holds. */
          /* In this case 'extension' must be corrected         */
          /* after realloc() enlarged 'bstri_dest'.             */
          extension = new_bstri;
        } /* if */
        COUNT3_BSTRI(new_bstri->size, new_size);
        memcpy(&new_bstri->mem[new_bstri->size], extension->mem,
            (size_t) extension->size * sizeof(ucharType));
        new_bstri->size = new_size;
        *destination = new_bstri     ;
      } /* if */
    } /* if */
  } /* bstAppend */



/**
 *  Concatenate two bstrings.
 *  @return the result of the concatenation.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             bstring.
 */
bstriType bstCat (const const_bstriType bstri1, const const_bstriType bstri2)

  {
    memSizeType result_size;
    bstriType result;

  /* bstCat */
    if (unlikely(bstri1->size > MAX_BSTRI_LEN - bstri2->size)) {
      /* number of bytes does not fit into memSizeType */
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = bstri1->size + bstri2->size;
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        memcpy(result->mem, bstri1->mem,
            bstri1->size * sizeof(ucharType));
        memcpy(&result->mem[bstri1->size], bstri2->mem,
            bstri2->size * sizeof(ucharType));
      } /* if */
    } /* if */
    return result;
  } /* bstCat */



/**
 *  Compare two bstrings.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType bstCmp (const const_bstriType bstri1, const const_bstriType bstri2)

  {
    intType result;

  /* bstCmp */
    if (bstri1->size < bstri2->size) {
      if (memcmp(bstri1->mem, bstri2->mem, bstri1->size * sizeof(ucharType)) <= 0) {
        result = -1;
      } else {
        result = 1;
      } /* if */
    } else {
      result = memcmp(bstri1->mem, bstri2->mem, bstri2->size * sizeof(ucharType));
      if (result == 0) {
        if (bstri1->size > bstri2->size) {
          result = 1;
        } /* if */
      } else if (result > 0) {
        result = 1;
      } else {
        result = -1;
      } /* if */
    } /* if */
    return result;
  } /* bstCmp */



/**
 *  Reinterpret the generic parameters as bstriType and call bstCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(bstriType).
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType bstCmpGeneric (const genericType value1, const genericType value2)

  { /* bstCmpGeneric */
    return bstCmp(((const_rtlObjectType *) &value1)->value.bstriValue,
                  ((const_rtlObjectType *) &value2)->value.bstriValue);
  } /* bstCmpGeneric */



/**
 *  Assign source to *dest.
 *  A copy function assumes that *dest contains a legal value.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void bstCpy (bstriType *const dest, const const_bstriType source)

  {
    memSizeType new_size;
    bstriType bstri_dest;

  /* bstCpy */
    bstri_dest = *dest;
    new_size = source->size;
    if (bstri_dest->size != new_size) {
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(bstri_dest, new_size))) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        FREE_BSTRI(*dest, (*dest)->size);
        bstri_dest->size = new_size;
        *dest = bstri_dest;
      } /* if */
    } /* if */
    /* It is possible that *dest == source holds. The    */
    /* behavior of memcpy() is undefined when source and */
    /* destination areas overlap (or are identical).     */
    /* Therefore memmove() is used instead of memcpy().  */
    memmove(bstri_dest->mem, source->mem,
        new_size * sizeof(ucharType));
  } /* bstCpy */



/**
 *  Reinterpret the generic parameters as bstriType and call bstCpy.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(bstriType).
 */
void bstCpyGeneric (genericType *const dest, const genericType source)

  { /* bstCpyGeneric */
    bstCpy(&((rtlObjectType *) dest)->value.bstriValue,
           ((const_rtlObjectType *) &source)->value.bstriValue);
  } /* bstCpyGeneric */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bstriType bstCreate (const const_bstriType source)

  {
    memSizeType new_size;
    bstriType result;

  /* bstCreate */
    logFunction(printf("bstCreate(\"%s\")", bstriAsUnquotedCStri(source));
                fflush(stdout););
    new_size = source->size;
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, new_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = new_size;
      memcpy(result->mem, source->mem,
          (size_t) new_size * sizeof(ucharType));
    } /* if */
    logFunctionResult(printf("\"%s\"\n", bstriAsUnquotedCStri(result)););
    return result;
  } /* bstCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(bstriType).
 */
genericType bstCreateGeneric (const genericType from_value)

  {
    rtlObjectType result;

  /* bstCreateGeneric */
    INIT_GENERIC_PTR(result.value.genericValue);
    result.value.bstriValue =
        bstCreate(((const_rtlObjectType *) &from_value)->value.bstriValue);
    return result.value.genericValue;
  } /* bstCreateGeneric */



/**
 *  Free the memory referred by 'old_bstring'.
 *  After bstDestr is left 'old_bstring' refers to not existing memory.
 *  The memory where 'old_bstring' is stored can be freed afterwards.
 */
void bstDestr (const const_bstriType old_bstring)

  { /* bstDestr */
    if (old_bstring != NULL) {
      FREE_BSTRI(old_bstring, old_bstring->size);
    } /* if */
  } /* bstDestr */



/**
 *  Generic Destr function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(bstriType).
 */
void bstDestrGeneric (const genericType old_value)

  { /* bstDestrGeneric */
    bstDestr(((const_rtlObjectType *) &old_value)->value.bstriValue);
  } /* bstDestrGeneric */



/**
 *  Compute the hash value of a 'bstring'.
 *  @return the hash value.
 */
intType bstHashCode (const const_bstriType bstri)

  {
    intType result;

  /* bstHashCode */
    if (bstri->size == 0) {
      result = 0;
    } else {
      result = (intType) ((memSizeType) bstri->mem[0] << 5 ^
          bstri->size << 3 ^ bstri->mem[bstri->size - 1]);
    } /* if */
    return result;
  } /* bstHashCode */



/**
 *  Convert a string to a 'bstring' value.
 *  @return the 'bstring' result of the conversion.
 *  @exception RANGE_ERROR When characters beyond '\255;' are present.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
bstriType bstParse (const const_striType stri)

  {
    bstriType result;

  /* bstParse */
    logFunction(printf("bstParse(\"%s\")\n", striAsUnquotedCStri(stri)););
    if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(result, stri->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = stri->size;
      if (unlikely(memcpy_from_strelem(result->mem, stri->mem, stri->size))) {
        FREE_BSTRI(result, result->size);
        logError(printf("bstParse(\"%s\"): Character beyond '\255;' present.\n",
                        striAsUnquotedCStri(stri)););
        raise_error(RANGE_ERROR);
        return NULL;
      } /* if */
    } /* if */
    logFunction(printf("bstParse --> \"%s\"\n", bstriAsUnquotedCStri(result)););
    return result;
  } /* bstParse */



/**
 *  Convert a 'bstring' value to a string.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
striType bstStr (const const_bstriType bstri)

  {
    striType result;

  /* bstStr */
    logFunction(printf("bstStr(\"%s\")\n", bstriAsUnquotedCStri(bstri)););
    if (unlikely(!ALLOC_STRI_CHECK_SIZE(result, bstri->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = bstri->size;
      memcpy_to_strelem(result->mem, bstri->mem, bstri->size);
    } /* if */
    logFunction(printf("bstStr --> \"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* bstStr */



bstriType bstTail (const const_bstriType stri, intType start)

  {
    memSizeType length;
    memSizeType result_size;
    bstriType result;

  /* bstTail */
    length = stri->size;
    if (start < 1) {
      start = 1;
    } /* if */
    if ((uintType) start <= length && length >= 1) {
      result_size = length - (memSizeType) start + 1;
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
      /* Reversing the order of the following two statements    */
      /* causes an "Internal Compiler Error" with MSC 6.0       */
      /* when using the -Ozacegilt optimisation option in the   */
      /* large memory model (-AL). Note that the order of the   */
      /* two statements make no difference to the logic of the  */
      /* program.                                               */
      memcpy(result->mem, &stri->mem[start - 1], result_size);
      result->size = result_size;
    } else {
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, (memSizeType) 0))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
      result->size = 0;
    } /* if */
    return result;
  } /* bstTail */
