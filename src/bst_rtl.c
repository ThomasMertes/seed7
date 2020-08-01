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



void bstAppend (bstritype *const bstri_to, const_bstritype bstri_from)

  {
    memsizetype new_size;
    bstritype bstri_dest;
    bstritype new_bstri;

  /* bstAppend */
    bstri_dest = *bstri_to;
    if (unlikely(bstri_dest->size > MAX_BSTRI_LEN - bstri_from->size)) {
      /* number of bytes does not fit into memsizetype */
      raise_error(MEMORY_ERROR);
    } else {
      new_size = bstri_dest->size + bstri_from->size;
      REALLOC_BSTRI_SIZE_OK(new_bstri, bstri_dest, bstri_dest->size, new_size);
      if (unlikely(new_bstri == NULL)) {
        raise_error(MEMORY_ERROR);
      } else {
        if (bstri_dest == bstri_from) {
          /* It is possible that bstri_dest == bstri_from holds. */
          /* In this case 'bstri_from' must be corrected         */
          /* after realloc() enlarged 'bstri_dest'.              */
          bstri_from = new_bstri;
        } /* if */
        COUNT3_BSTRI(new_bstri->size, new_size);
        memcpy(&new_bstri->mem[new_bstri->size], bstri_from->mem,
            (size_t) bstri_from->size * sizeof(uchartype));
        new_bstri->size = new_size;
        *bstri_to = new_bstri     ;
      } /* if */
    } /* if */
  } /* bstAppend */



bstritype bstCat (const const_bstritype bstri1, const const_bstritype bstri2)

  {
    memsizetype result_size;
    bstritype result;

  /* bstCat */
    if (unlikely(bstri1->size > MAX_BSTRI_LEN - bstri2->size)) {
      /* number of bytes does not fit into memsizetype */
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = bstri1->size + bstri2->size;
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        memcpy(result->mem, bstri1->mem,
            bstri1->size * sizeof(uchartype));
        memcpy(&result->mem[bstri1->size], bstri2->mem,
            bstri2->size * sizeof(uchartype));
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
inttype bstCmp (const const_bstritype bstri1, const const_bstritype bstri2)

  {
    inttype result;

  /* bstCmp */
    if (bstri1->size < bstri2->size) {
      if (memcmp(bstri1->mem, bstri2->mem, bstri1->size * sizeof(uchartype)) <= 0) {
        result = -1;
      } else {
        result = 1;
      } /* if */
    } else {
      result = memcmp(bstri1->mem, bstri2->mem, bstri2->size * sizeof(uchartype));
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
 *  Reinterpret the generic parameters as bstritype and call bstCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(generictype) != sizeof(bstritype).
 */
inttype bstCmpGeneric (const generictype value1, const generictype value2)

  { /* bstCmpGeneric */
    return bstCmp(((const_rtlObjecttype *) &value1)->value.bstrivalue,
                  ((const_rtlObjecttype *) &value2)->value.bstrivalue);
  } /* bstCmpGeneric */



void bstCpy (bstritype *const bstri_to, const const_bstritype bstri_from)

  {
    memsizetype new_size;
    bstritype bstri_dest;

  /* bstCpy */
    bstri_dest = *bstri_to;
    new_size = bstri_from->size;
    if (bstri_dest->size != new_size) {
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(bstri_dest, new_size))) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        FREE_BSTRI(*bstri_to, (*bstri_to)->size);
        bstri_dest->size = new_size;
        *bstri_to = bstri_dest;
      } /* if */
    } /* if */
    /* It is possible that *bstri_to == bstri_from holds. The */
    /* behavior of memcpy() is undefined when source and      */
    /* destination areas overlap (or are identical).          */
    /* Therefore memmove() is used instead of memcpy().       */
    memmove(bstri_dest->mem, bstri_from->mem,
        new_size * sizeof(uchartype));
  } /* bstCpy */



void bstCpyGeneric (generictype *const dest, const generictype source)

  { /* bstCpyGeneric */
    bstCpy(&((rtlObjecttype *) dest)->value.bstrivalue,
           ((const_rtlObjecttype *) &source)->value.bstrivalue);
  } /* bstCpyGeneric */



bstritype bstCreate (const const_bstritype bstri_from)

  {
    memsizetype new_size;
    bstritype result;

  /* bstCreate */
    new_size = bstri_from->size;
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, new_size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = new_size;
      memcpy(result->mem, bstri_from->mem,
          (size_t) new_size * sizeof(uchartype));
    } /* if */
    return result;
  } /* bstCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(generictype) != sizeof(bstritype).
 */
generictype bstCreateGeneric (const generictype from_value)

  {
    rtlObjecttype result;

  /* bstCreateGeneric */
    result.value.bstrivalue =
        bstCreate(((const_rtlObjecttype *) &from_value)->value.bstrivalue);
    return result.value.genericvalue;
  } /* bstCreateGeneric */



void bstDestr (const const_bstritype old_bstring)

  { /* bstDestr */
    if (old_bstring != NULL) {
      FREE_BSTRI(old_bstring, old_bstring->size);
    } /* if */
  } /* bstDestr */



/**
 *  Generic Destr function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(generictype) != sizeof(bstritype).
 */
void bstDestrGeneric (const generictype old_value)

  { /* bstDestrGeneric */
    bstDestr(((const_rtlObjecttype *) &old_value)->value.bstrivalue);
  } /* bstDestrGeneric */



/**
 *  Compute the hash value of a 'bstring'.
 *  @return the hash value.
 */
inttype bstHashCode (const const_bstritype bstri)

  {
    inttype result;

  /* bstHashCode */
    if (bstri->size == 0) {
      result = 0;
    } else {
      result = (inttype) ((memsizetype) bstri->mem[0] << 5 ^
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
bstritype bstParse (const const_stritype stri)

  {
    register const strelemtype *str;
    register uchartype *ustri;
    register memsizetype pos;
    bstritype result;

  /* bstParse */
    if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(result, stri->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = stri->size;
      str = stri->mem;
      ustri = result->mem;
      for (pos = 0; pos < stri->size; pos++) {
        if (unlikely(str[pos] >= 256)) {
          FREE_BSTRI(result, result->size);
          raise_error(RANGE_ERROR);
          return NULL;
        } /* if */
        ustri[pos] = (uchartype) str[pos];
      } /* for */
    } /* if */
    return result;
  } /* bstParse */



/**
 *  Convert a 'bstring' value to a string.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
stritype bstStr (const const_bstritype bstri)

  {
    register const uchartype *from = bstri->mem;
    register strelemtype *to;
    stritype result;

  /* bstStr */
    if (unlikely(!ALLOC_STRI_CHECK_SIZE(result, bstri->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = bstri->size;
      to = result->mem;
      memcpy_to_strelem(to, from, bstri->size);
    } /* if */
    return result;
  } /* bstStr */



bstritype bstTail (const const_bstritype stri, inttype start)

  {
    memsizetype length;
    memsizetype result_size;
    bstritype result;

  /* bstTail */
    length = stri->size;
    if (start < 1) {
      start = 1;
    } /* if */
    if ((uinttype) start <= length && length >= 1) {
      result_size = length - (memsizetype) start + 1;
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
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, (memsizetype) 0))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } /* if */
      result->size = 0;
    } /* if */
    return result;
  } /* bstTail */
