/********************************************************************/
/*                                                                  */
/*  bst_rtl.c     Primitive actions for the byte string type.       */
/*  Copyright (C) 1989 - 2007  Thomas Mertes                        */
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
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: Primitive actions for the byte string type.            */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "heaputl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "bst_rtl.h"



#ifdef ANSI_C

void bstAppend (bstritype *const bstri_to, const const_bstritype bstri_from)
#else

void bstAppend (bstri_to, bstri_from)
bstritype *bstri_to;
bstritype bstri_from;
#endif

  {
    memsizetype new_size;
    bstritype bstri_dest;

  /* bstAppend */
    if (bstri_from->size != 0) {
      bstri_dest = *bstri_to;
      new_size = bstri_dest->size + bstri_from->size;
      REALLOC_BSTRI(bstri_dest, bstri_dest, bstri_dest->size, new_size);
      if (bstri_dest == NULL) {
        raise_error(MEMORY_ERROR);
        return;
      } /* if */
      COUNT3_BSTRI(bstri_dest->size, new_size);
      memcpy(&bstri_dest->mem[bstri_dest->size], bstri_from->mem,
          (SIZE_TYPE) bstri_from->size * sizeof(uchartype));
      bstri_dest->size = new_size;
      *bstri_to = bstri_dest;
    } /* if */
  } /* bstAppend */



#ifdef ANSI_C

bstritype bstCat (const const_bstritype bstri1, const const_bstritype bstri2)
#else

bstritype bstCat (stri1, stri2)
bstritype stri1;
bstritype stri2;
#endif

  {
    memsizetype result_size;
    bstritype result;

  /* bstCat */
    result_size = bstri1->size + bstri2->size;
    if (!ALLOC_BSTRI(result, result_size)) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result->size = result_size;
      memcpy(result->mem, bstri1->mem,
          (SIZE_TYPE) bstri1->size * sizeof(uchartype));
      memcpy(&result->mem[bstri1->size], bstri2->mem,
          (SIZE_TYPE) bstri2->size * sizeof(uchartype));
      return(result);
    } /* if */
  } /* bstCat */



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
      if (!ALLOC_BSTRI(bstri_dest, new_size)) {
        raise_error(MEMORY_ERROR);
        return;
      } else {
        FREE_BSTRI(*bstri_to, (*bstri_to)->size);
        bstri_dest->size = new_size;
        *bstri_to = bstri_dest;
      } /* if */
    } /* if */
    memcpy(bstri_dest->mem, bstri_from->mem,
        (SIZE_TYPE) new_size * sizeof(uchartype));
  } /* bstCpy */



#ifdef ANSI_C

bstritype bstCreate (const const_bstritype bstri_from)
#else

bstritype bstCreate (bstri_from)
bstritype bstri_from;
#endif

  {
    memsizetype new_size;
    bstritype result;

  /* bstCreate */
    new_size = bstri_from->size;
    if (!ALLOC_BSTRI(result, new_size)) {
      raise_error(MEMORY_ERROR);
    } else {
      result->size = new_size;
      memcpy(result->mem, bstri_from->mem,
          (SIZE_TYPE) new_size * sizeof(uchartype));
    } /* if */
    return(result);
  } /* bstCreate */



#ifdef ANSI_C

void bstDestr (const const_bstritype old_bstring)
#else

void bstDestr (old_bstring)
bstritype old_bstring;
#endif

  { /* bstDestr */
    if (old_bstring != NULL) {
      FREE_BSTRI(old_bstring, old_bstring->size);
    } /* if */
  } /* bstDestr */
