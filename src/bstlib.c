/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2010  Thomas Mertes                        */
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
/*  Module: Library                                                 */
/*  File: seed7/src/bstlib.c                                        */
/*  Changes: 2007, 2010  Thomas Mertes                              */
/*  Content: All primitive actions for the byte string type.        */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "chclsutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "objutl.h"
#include "runerr.h"
#include "bst_rtl.h"

#undef EXTERN
#define EXTERN
#include "bstlib.h"



#ifdef ANSI_C

objecttype bst_append (listtype arguments)
#else

objecttype bst_append (arguments)
listtype arguments;
#endif

  {
    objecttype bstr_variable;
    bstritype bstr_to;
    bstritype bstr_from;
    bstritype new_bstr;
    memsizetype new_size;
    memsizetype bstr_to_size;

  /* bst_append */
    bstr_variable = arg_1(arguments);
    isit_bstri(bstr_variable);
    is_variable(bstr_variable);
    bstr_to = take_bstri(bstr_variable);
    isit_bstri(arg_3(arguments));
    bstr_from = take_bstri(arg_3(arguments));
    if (bstr_from->size != 0) {
      bstr_to_size = bstr_to->size;
      if (bstr_to_size > MAX_BSTRI_LEN - bstr_from->size) {
        /* number of bytes does not fit into memsizetype */
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_size = bstr_to_size + bstr_from->size;
        REALLOC_BSTRI_SIZE_OK(new_bstr, bstr_to, bstr_to_size, new_size);
        if (new_bstr == NULL) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          if (bstr_to == bstr_from) {
            /* It is possible that bstr_to == bstr_from holds. */
            /* In this case 'bstr_from' must be corrected      */
            /* after realloc() enlarged 'bstr_to'.             */
            bstr_from = new_bstr;
          } /* if */
          COUNT3_BSTRI(bstr_to_size, new_size);
          memcpy(&new_bstr->mem[bstr_to_size], bstr_from->mem,
              bstr_from->size * sizeof(uchartype));
          new_bstr->size = new_size;
          bstr_variable->value.bstrivalue = new_bstr;
        } /* if */
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* bst_append */



#ifdef ANSI_C

objecttype bst_cat (listtype arguments)
#else

objecttype bst_cat (arguments)
listtype arguments;
#endif

  {
    bstritype bstri1;
    bstritype bstri2;
    memsizetype bstri1_size;
    memsizetype result_size;
    bstritype result;

  /* bst_cat */
    isit_bstri(arg_1(arguments));
    isit_bstri(arg_3(arguments));
    bstri1 = take_bstri(arg_1(arguments));
    bstri2 = take_bstri(arg_3(arguments));
    bstri1_size = bstri1->size;
    if (bstri1_size > MAX_BSTRI_LEN - bstri2->size) {
      /* number of bytes does not fit into memsizetype */
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result_size = bstri1_size + bstri2->size;
      if (TEMP_OBJECT(arg_1(arguments))) {
        REALLOC_BSTRI_SIZE_OK(result, bstri1, bstri1_size, result_size);
        if (result == NULL) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          COUNT3_STRI(bstri1_size, result_size);
          result->size = result_size;
          memcpy(&result->mem[bstri1_size], bstri2->mem,
              bstri2->size * sizeof(uchartype));
          arg_1(arguments)->value.bstrivalue = NULL;
          return bld_bstri_temp(result);
        } /* if */
      } else {
        if (!ALLOC_BSTRI_SIZE_OK(result, result_size)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          result->size = result_size;
          memcpy(result->mem, bstri1->mem,
              bstri1_size * sizeof(uchartype));
          memcpy(&result->mem[bstri1_size], bstri2->mem,
              bstri2->size * sizeof(uchartype));
          return bld_bstri_temp(result);
        } /* if */
      } /* if */
    } /* if */
  } /* bst_cat */



#ifdef ANSI_C

objecttype bst_cmp (listtype arguments)
#else

objecttype bst_cmp (arguments)
listtype arguments;
#endif

  {
    bstritype bstri1;
    bstritype bstri2;
    inttype result;

  /* bst_cmp */
    isit_bstri(arg_1(arguments));
    isit_bstri(arg_2(arguments));
    bstri1 = take_bstri(arg_1(arguments));
    bstri2 = take_bstri(arg_2(arguments));
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
    return bld_int_temp(result);
  } /* bst_cmp */



#ifdef ANSI_C

objecttype bst_cpy (listtype arguments)
#else

objecttype bst_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype bstri_to;
    objecttype bstri_from;
    memsizetype new_size;
    bstritype bstri_dest;

  /* bst_cpy */
    bstri_to = arg_1(arguments);
    bstri_from = arg_3(arguments);
    isit_bstri(bstri_to);
    isit_bstri(bstri_from);
    is_variable(bstri_to);
    bstri_dest = take_bstri(bstri_to);
    if (TEMP_OBJECT(bstri_from)) {
      FREE_BSTRI(bstri_dest, bstri_dest->size);
      bstri_to->value.bstrivalue = take_bstri(bstri_from);
      bstri_from->value.bstrivalue = NULL;
    } else {
      new_size = take_bstri(bstri_from)->size;
      if (bstri_dest->size == new_size) {
        if (bstri_dest != take_bstri(bstri_from)) {
          /* It is possible that bstr_to == bstr_from holds. The */
          /* behavior of memcpy() is undefined when source and   */
          /* destination areas overlap (or are identical).       */
          /* Therefore a check for this case is necessary.       */
          memcpy(bstri_dest->mem, take_bstri(bstri_from)->mem,
              new_size * sizeof(uchartype));
        } /* if */
      } else {
        if (!ALLOC_BSTRI_SIZE_OK(bstri_dest, new_size)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          FREE_BSTRI(take_bstri(bstri_to), take_bstri(bstri_to)->size);
          bstri_to->value.bstrivalue = bstri_dest;
          bstri_dest->size = new_size;
        } /* if */
        memcpy(bstri_dest->mem, take_bstri(bstri_from)->mem,
            new_size * sizeof(uchartype));
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* bst_cpy */



#ifdef ANSI_C

objecttype bst_create (listtype arguments)
#else

objecttype bst_create (arguments)
listtype arguments;
#endif

  {
    objecttype bstri_to;
    objecttype bstri_from;
    memsizetype new_size;
    bstritype new_bstri;

  /* bst_create */
    bstri_to = arg_1(arguments);
    bstri_from = arg_3(arguments);
    isit_bstri(bstri_from);
    SET_CATEGORY_OF_OBJ(bstri_to, BSTRIOBJECT);
    if (TEMP_OBJECT(bstri_from)) {
      bstri_to->value.bstrivalue = take_bstri(bstri_from);
      bstri_from->value.bstrivalue = NULL;
    } else {
/*    printf("bstri_create %d !!!\n", in_file.line); */
      new_size = take_bstri(bstri_from)->size;
      if (!ALLOC_BSTRI_SIZE_OK(new_bstri, new_size)) {
        bstri_to->value.bstrivalue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      bstri_to->value.bstrivalue = new_bstri;
      new_bstri->size = new_size;
      memcpy(new_bstri->mem, take_bstri(bstri_from)->mem,
          new_size * sizeof(uchartype));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* bst_create */



#ifdef ANSI_C

objecttype bst_destr (listtype arguments)
#else

objecttype bst_destr (arguments)
listtype arguments;
#endif

  {
    bstritype old_bstring;

  /* bst_destr */
    isit_bstri(arg_1(arguments));
    old_bstring = take_bstri(arg_1(arguments));
    if (old_bstring != NULL) {
      FREE_BSTRI(old_bstring, old_bstring->size);
      arg_1(arguments)->value.bstrivalue = NULL;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* bst_destr */



#ifdef ANSI_C

objecttype bst_empty (listtype arguments)
#else

objecttype bst_empty (arguments)
listtype arguments;
#endif

  {
    bstritype result;

  /* bst_empty */
    if (!ALLOC_BSTRI_SIZE_OK(result, 0)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->size = 0;
      return bld_bstri_temp(result);
    } /* if */
  } /* bst_empty */



#ifdef ANSI_C

objecttype bst_eq (listtype arguments)
#else

objecttype bst_eq (arguments)
listtype arguments;
#endif

  {
    bstritype bstri1;
    bstritype bstri2;

  /* bst_eq */
    isit_bstri(arg_1(arguments));
    isit_bstri(arg_3(arguments));
    bstri1 = take_bstri(arg_1(arguments));
    bstri2 = take_bstri(arg_3(arguments));
    if (bstri1->size == bstri2->size && memcmp(bstri1->mem, bstri2->mem,
        bstri1->size * sizeof(uchartype)) == 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* bst_eq */



#ifdef ANSI_C

objecttype bst_hashcode (listtype arguments)
#else

objecttype bst_hashcode (arguments)
listtype arguments;
#endif

  {
    bstritype bstri1;
    inttype result;

  /* bst_hashcode */
    isit_bstri(arg_1(arguments));
    bstri1 = take_bstri(arg_1(arguments));
    if (bstri1->size == 0) {
      result = 0;
    } else {
      result = (inttype) ((memsizetype) bstri1->mem[0] << 5 ^
          bstri1->size << 3 ^ bstri1->mem[bstri1->size - 1]);
    } /* if */
    return bld_int_temp(result);
  } /* bst_hashcode */



#ifdef ANSI_C

objecttype bst_idx (listtype arguments)
#else

objecttype bst_idx (arguments)
listtype arguments;
#endif

  {
    bstritype bstri;
    inttype position;

  /* bst_idx */
    isit_bstri(arg_1(arguments));
    isit_int(arg_3(arguments));
    bstri = take_bstri(arg_1(arguments));
    position = take_int(arg_3(arguments));
    if (position >= 1 && (uinttype) position <= bstri->size) {
      return bld_char_temp((chartype) bstri->mem[position - 1]);
    } else {
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
  } /* bst_idx */



#ifdef ANSI_C

objecttype bst_lng (listtype arguments)
#else

objecttype bst_lng (arguments)
listtype arguments;
#endif

  {
    bstritype bstri;

  /* bst_lng */
    isit_bstri(arg_1(arguments));
    bstri = take_bstri(arg_1(arguments));
#if POINTER_SIZE > INTTYPE_SIZE
    if (bstri->size > MAX_MEM_INDEX) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_int_temp((inttype) bstri->size);
    } /* if */
#else
    return bld_int_temp((inttype) bstri->size);
#endif
  } /* bst_lng */



#ifdef ANSI_C

objecttype bst_ne (listtype arguments)
#else

objecttype bst_ne (arguments)
listtype arguments;
#endif

  {
    bstritype bstri1;
    bstritype bstri2;

  /* bst_ne */
    isit_bstri(arg_1(arguments));
    isit_bstri(arg_3(arguments));
    bstri1 = take_bstri(arg_1(arguments));
    bstri2 = take_bstri(arg_3(arguments));
    if (bstri1->size != bstri2->size || memcmp(bstri1->mem, bstri2->mem,
        bstri1->size * sizeof(uchartype)) != 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* bst_ne */



#ifdef ANSI_C

objecttype bst_parse (listtype arguments)
#else

objecttype bst_parse (arguments)
listtype arguments;
#endif

  { /* bst_parse */
    isit_stri(arg_3(arguments));
    return bld_bstri_temp(
        bstParse(take_stri(arg_3(arguments))));
  } /* bst_parse */



#ifdef ANSI_C

objecttype bst_str (listtype arguments)
#else

objecttype bst_str (arguments)
listtype arguments;
#endif

  { /* bst_str */
    isit_bstri(arg_1(arguments));
    return bld_stri_temp(
        bstStr(take_bstri(arg_1(arguments))));
  } /* bst_str */



#ifdef ANSI_C

objecttype bst_value (listtype arguments)
#else

objecttype bst_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;
    bstritype bstri;
    bstritype result;

  /* bst_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != BSTRIOBJECT ||
        take_bstri(obj_arg) == NULL) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      bstri = take_bstri(obj_arg);
      if (!ALLOC_BSTRI_SIZE_OK(result, bstri->size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = bstri->size;
        memcpy(result->mem, bstri->mem,
            result->size * sizeof(uchartype));
        return bld_bstri_temp(result);
      } /* if */
    } /* if */
  } /* bst_value */
