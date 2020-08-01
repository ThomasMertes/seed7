/********************************************************************/
/*                                                                  */
/*  bst_rtl.c     Primitive actions for the byte string type.       */
/*  Copyright (C) 1989 - 2010  Thomas Mertes                        */
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



#ifdef ANSI_C

void bstAppend (bstritype *const bstri_to, const_bstritype bstri_from)
#else

void bstAppend (bstri_to, bstri_from)
bstritype *bstri_to;
bstritype bstri_from;
#endif

  {
    memsizetype new_size;
    bstritype bstri_dest;
    bstritype new_bstri;

  /* bstAppend */
    bstri_dest = *bstri_to;
    if (bstri_dest->size > MAX_BSTRI_LEN - bstri_from->size) {
      /* number of bytes does not fit into memsizetype */
      raise_error(MEMORY_ERROR);
    } else {
      new_size = bstri_dest->size + bstri_from->size;
      REALLOC_BSTRI_SIZE_OK(new_bstri, bstri_dest, bstri_dest->size, new_size);
      if (new_bstri == NULL) {
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



#ifdef ANSI_C

bstritype bstCat (const const_bstritype bstri1, const const_bstritype bstri2)
#else

bstritype bstCat (bstri1, bstri2)
bstritype bstri1;
bstritype bstri2;
#endif

  {
    memsizetype result_size;
    bstritype result;

  /* bstCat */
    if (bstri1->size > MAX_BSTRI_LEN - bstri2->size) {
      /* number of bytes does not fit into memsizetype */
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result_size = bstri1->size + bstri2->size;
      if (!ALLOC_BSTRI_SIZE_OK(result, result_size)) {
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



#ifdef ANSI_C

INLINE inttype bstCmp (const const_bstritype bstri1, const const_bstritype bstri2)
#else

INLINE inttype bstCmp (bstri1, bstri2)
bstritype bstri1;
bstritype bstri2;
#endif

  {
    inttype result;

  /* bstCmp */
    if (bstri1->size < bstri2->size) {
      if (memcmp(bstri1->mem, bstri2->mem, bstri1->size * sizeof(uchartype)) <= 0) {
        result = -1;
      } else {
        result = 1;
      } /* if */
    } else if (bstri1->size > bstri2->size) {
      if (memcmp(bstri1->mem, bstri2->mem, bstri2->size * sizeof(uchartype)) >= 0) {
        result = 1;
      } else {
        result = -1;
      } /* if */
    } else {
      result = memcmp(bstri1->mem, bstri2->mem, bstri1->size * sizeof(uchartype));
      if (result > 0) {
        result = 1;
      } else if (result < 0) {
        result = -1;
      } /* if */
    } /* if */
    return result;
  } /* bstCmp */



/**
 *  Reinterpret the generic parameters as bstritype and call bstCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(rtlGenerictype) != sizeof(bstritype).
 */
#ifdef ANSI_C

inttype bstCmpGeneric (const rtlGenerictype value1, const rtlGenerictype value2)
#else

inttype bstCmpGeneric (value1, value2)
rtlGenerictype value1;
rtlGenerictype value2;
#endif

  { /* bstCmpGeneric */
    return bstCmp((const_bstritype) value1, (const_bstritype) value2);
  } /* bstCmpGeneric */



#ifdef ANSI_C

void bstCpy (bstritype *const bstri_to, const const_bstritype bstri_from)
#else

void bstCpy (bstri_to, bstri_from)
bstritype *bstri_to;
bstritype bstri_from;
#endif

  {
    memsizetype new_size;
    bstritype bstri_dest;

  /* bstCpy */
    bstri_dest = *bstri_to;
    new_size = bstri_from->size;
    if (bstri_dest->size != new_size) {
      if (!ALLOC_BSTRI_SIZE_OK(bstri_dest, new_size)) {
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



#ifdef ANSI_C

INLINE bstritype bstCreate (const const_bstritype bstri_from)
#else

INLINE bstritype bstCreate (bstri_from)
bstritype bstri_from;
#endif

  {
    memsizetype new_size;
    bstritype result;

  /* bstCreate */
    new_size = bstri_from->size;
    if (!ALLOC_BSTRI_SIZE_OK(result, new_size)) {
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
 *  when sizeof(rtlGenerictype) != sizeof(bstritype).
 */
#ifdef ANSI_C

rtlGenerictype bstCreateGeneric (const rtlGenerictype from_value)
#else

rtlGenerictype bstCreateGeneric (from_value)
rtlGenerictype from_value;
#endif

  { /* bstCreateGeneric */
    return (rtlGenerictype) (memsizetype) bstCreate((const_bstritype) from_value);
  } /* bstCreateGeneric */



#ifdef ANSI_C

void bstDestr (const bstritype old_bstring)
#else

void bstDestr (old_bstring)
bstritype old_bstring;
#endif

  { /* bstDestr */
    if (old_bstring != NULL) {
      FREE_BSTRI(old_bstring, old_bstring->size);
    } /* if */
  } /* bstDestr */



#ifdef ANSI_C

inttype bstHashCode (const const_bstritype bstri)
#else

inttype bstHashCode (bstri)
bstritype bstri;
#endif

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



#ifdef ANSI_C

bstritype bstParse (const const_stritype stri)
#else

bstritype bstParse (stri)
stritype stri;
#endif

  {
    memsizetype len;
    const strelemtype *strelem;
    uchartype *bstrelem;
    bstritype result;

  /* bstParse */
    len = stri->size;
    if (!ALLOC_BSTRI_CHECK_SIZE(result, len)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = len;
      for (bstrelem = result->mem, strelem = stri->mem;
           len > 0; bstrelem++, strelem++, len--) {
        if (*strelem >= 256) {
          FREE_BSTRI(result, result->size);
          result = NULL;
          raise_error(RANGE_ERROR);
          len = 0;
        } else {
          *bstrelem = (uchartype) *strelem;
        } /* if */
      } /* for */
    } /* if */
    return result;
  } /* bstParse */



#ifdef ANSI_C

stritype bstStr (const const_bstritype bstri)
#else

stritype bstStr (bstri)
bstritype bstri;
#endif

  {
    stritype result;

  /* bstStr */
    if (!ALLOC_STRI_CHECK_SIZE(result, bstri->size)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = bstri->size;
      ustri_expand(result->mem, bstri->mem, bstri->size);
    } /* if */
    return result;
  } /* bstStr */
