/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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



objectType bst_append (listType arguments)

  {
    objectType bstr_variable;
    bstriType bstr_to;
    bstriType bstr_from;
    bstriType new_bstr;
    memSizeType new_size;
    memSizeType bstr_to_size;

  /* bst_append */
    bstr_variable = arg_1(arguments);
    isit_bstri(bstr_variable);
    is_variable(bstr_variable);
    bstr_to = take_bstri(bstr_variable);
    isit_bstri(arg_3(arguments));
    bstr_from = take_bstri(arg_3(arguments));
    if (bstr_from->size != 0) {
      bstr_to_size = bstr_to->size;
      if (unlikely(bstr_to_size > MAX_BSTRI_LEN - bstr_from->size)) {
        /* number of bytes does not fit into memSizeType */
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_size = bstr_to_size + bstr_from->size;
        REALLOC_BSTRI_SIZE_OK(new_bstr, bstr_to, bstr_to_size, new_size);
        if (unlikely(new_bstr == NULL)) {
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
              bstr_from->size * sizeof(ucharType));
          new_bstr->size = new_size;
          bstr_variable->value.bstriValue = new_bstr;
        } /* if */
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* bst_append */



objectType bst_cat (listType arguments)

  {
    bstriType bstri1;
    bstriType bstri2;
    memSizeType bstri1_size;
    memSizeType result_size;
    bstriType result;

  /* bst_cat */
    isit_bstri(arg_1(arguments));
    isit_bstri(arg_3(arguments));
    bstri1 = take_bstri(arg_1(arguments));
    bstri2 = take_bstri(arg_3(arguments));
    bstri1_size = bstri1->size;
    if (unlikely(bstri1_size > MAX_BSTRI_LEN - bstri2->size)) {
      /* number of bytes does not fit into memSizeType */
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result_size = bstri1_size + bstri2->size;
      if (TEMP_OBJECT(arg_1(arguments))) {
        REALLOC_BSTRI_SIZE_OK(result, bstri1, bstri1_size, result_size);
        if (unlikely(result == NULL)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          COUNT3_STRI(bstri1_size, result_size);
          result->size = result_size;
          memcpy(&result->mem[bstri1_size], bstri2->mem,
              bstri2->size * sizeof(ucharType));
          arg_1(arguments)->value.bstriValue = NULL;
          return bld_bstri_temp(result);
        } /* if */
      } else {
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, result_size))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          result->size = result_size;
          memcpy(result->mem, bstri1->mem,
              bstri1_size * sizeof(ucharType));
          memcpy(&result->mem[bstri1_size], bstri2->mem,
              bstri2->size * sizeof(ucharType));
          return bld_bstri_temp(result);
        } /* if */
      } /* if */
    } /* if */
  } /* bst_cat */



/**
 *  Compare two bstrings.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objectType bst_cmp (listType arguments)

  {
    bstriType bstri1;
    bstriType bstri2;
    intType signumValue;

  /* bst_cmp */
    isit_bstri(arg_1(arguments));
    isit_bstri(arg_2(arguments));
    bstri1 = take_bstri(arg_1(arguments));
    bstri2 = take_bstri(arg_2(arguments));
    if (bstri1->size < bstri2->size) {
      if (memcmp(bstri1->mem, bstri2->mem, bstri1->size * sizeof(ucharType)) <= 0) {
        signumValue = -1;
      } else {
        signumValue = 1;
      } /* if */
    } else {
      signumValue = memcmp(bstri1->mem, bstri2->mem, bstri2->size * sizeof(ucharType));
      if (signumValue == 0) {
        if (bstri1->size > bstri2->size) {
          signumValue = 1;
        } /* if */
      } else if (signumValue > 0) {
        signumValue = 1;
      } else {
        signumValue = -1;
      } /* if */
    } /* if */
    return bld_int_temp(signumValue);
  } /* bst_cmp */



objectType bst_cpy (listType arguments)

  {
    objectType bstri_to;
    objectType bstri_from;
    memSizeType new_size;
    bstriType bstri_dest;

  /* bst_cpy */
    bstri_to = arg_1(arguments);
    bstri_from = arg_3(arguments);
    isit_bstri(bstri_to);
    isit_bstri(bstri_from);
    is_variable(bstri_to);
    bstri_dest = take_bstri(bstri_to);
    if (TEMP_OBJECT(bstri_from)) {
      FREE_BSTRI(bstri_dest, bstri_dest->size);
      bstri_to->value.bstriValue = take_bstri(bstri_from);
      bstri_from->value.bstriValue = NULL;
    } else {
      new_size = take_bstri(bstri_from)->size;
      if (bstri_dest->size == new_size) {
        if (bstri_dest != take_bstri(bstri_from)) {
          /* It is possible that bstr_to == bstr_from holds. The */
          /* behavior of memcpy() is undefined when source and   */
          /* destination areas overlap (or are identical).       */
          /* Therefore a check for this case is necessary.       */
          memcpy(bstri_dest->mem, take_bstri(bstri_from)->mem,
              new_size * sizeof(ucharType));
        } /* if */
      } else {
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(bstri_dest, new_size))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          FREE_BSTRI(take_bstri(bstri_to), take_bstri(bstri_to)->size);
          bstri_to->value.bstriValue = bstri_dest;
          bstri_dest->size = new_size;
        } /* if */
        memcpy(bstri_dest->mem, take_bstri(bstri_from)->mem,
            new_size * sizeof(ucharType));
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* bst_cpy */



objectType bst_create (listType arguments)

  {
    objectType bstri_to;
    objectType bstri_from;
    memSizeType new_size;
    bstriType new_bstri;

  /* bst_create */
    bstri_to = arg_1(arguments);
    bstri_from = arg_3(arguments);
    isit_bstri(bstri_from);
    SET_CATEGORY_OF_OBJ(bstri_to, BSTRIOBJECT);
    if (TEMP_OBJECT(bstri_from)) {
      bstri_to->value.bstriValue = take_bstri(bstri_from);
      bstri_from->value.bstriValue = NULL;
    } else {
/*    printf("bstri_create %d !!!\n", in_file.line); */
      new_size = take_bstri(bstri_from)->size;
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(new_bstri, new_size))) {
        bstri_to->value.bstriValue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      bstri_to->value.bstriValue = new_bstri;
      new_bstri->size = new_size;
      memcpy(new_bstri->mem, take_bstri(bstri_from)->mem,
          new_size * sizeof(ucharType));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* bst_create */



objectType bst_destr (listType arguments)

  {
    bstriType old_bstring;

  /* bst_destr */
    isit_bstri(arg_1(arguments));
    old_bstring = take_bstri(arg_1(arguments));
    if (old_bstring != NULL) {
      FREE_BSTRI(old_bstring, old_bstring->size);
      arg_1(arguments)->value.bstriValue = NULL;
    } /* if */
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* bst_destr */



objectType bst_empty (listType arguments)

  {
    bstriType result;

  /* bst_empty */
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, 0))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->size = 0;
      return bld_bstri_temp(result);
    } /* if */
  } /* bst_empty */



/**
 *  Check if two bstrings are equal.
 *  @return TRUE if both bstrings are equal,
 *          FALSE otherwise.
 */
objectType bst_eq (listType arguments)

  {
    bstriType bstri1;
    bstriType bstri2;

  /* bst_eq */
    isit_bstri(arg_1(arguments));
    isit_bstri(arg_3(arguments));
    bstri1 = take_bstri(arg_1(arguments));
    bstri2 = take_bstri(arg_3(arguments));
    if (bstri1->size == bstri2->size && memcmp(bstri1->mem, bstri2->mem,
        bstri1->size * sizeof(ucharType)) == 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* bst_eq */



/**
 *  Compute the hash value of a 'bstring'.
 *  @return the hash value.
 */
objectType bst_hashcode (listType arguments)

  {
    bstriType bstri1;
    intType result;

  /* bst_hashcode */
    isit_bstri(arg_1(arguments));
    bstri1 = take_bstri(arg_1(arguments));
    if (bstri1->size == 0) {
      result = 0;
    } else {
      result = (intType) ((memSizeType) bstri1->mem[0] << 5 ^
          bstri1->size << 3 ^ bstri1->mem[bstri1->size - 1]);
    } /* if */
    return bld_int_temp(result);
  } /* bst_hashcode */



/**
 *  Get a character, identified by an index, from a 'bstring'.
 *  The first character has the index 1.
 *  @return the character specified with the index.
 *  @exception RANGE_ERROR When the index is less than 1 or
 *             greater than the length of the 'bstring'.
 */
objectType bst_idx (listType arguments)

  {
    bstriType bstri;
    intType position;

  /* bst_idx */
    isit_bstri(arg_1(arguments));
    isit_int(arg_3(arguments));
    bstri = take_bstri(arg_1(arguments));
    position = take_int(arg_3(arguments));
    if (unlikely(position < 1 || (uintType) position > bstri->size)) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_char_temp((charType) bstri->mem[position - 1]);
    } /* if */
  } /* bst_idx */



/**
 *  Determine the length of a 'bstring'.
 *  @return the length of the 'bstring'.
 */
objectType bst_lng (listType arguments)

  {
    bstriType bstri;

  /* bst_lng */
    isit_bstri(arg_1(arguments));
    bstri = take_bstri(arg_1(arguments));
#if POINTER_SIZE > INTTYPE_SIZE
    if (unlikely(bstri->size > MAX_MEM_INDEX)) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
#endif
    return bld_int_temp((intType) bstri->size);
  } /* bst_lng */



/**
 *  Check if two bstrings are not equal.
 *  @return FALSE if both bstrings are equal,
 *          TRUE otherwise.
 */
objectType bst_ne (listType arguments)

  {
    bstriType bstri1;
    bstriType bstri2;

  /* bst_ne */
    isit_bstri(arg_1(arguments));
    isit_bstri(arg_3(arguments));
    bstri1 = take_bstri(arg_1(arguments));
    bstri2 = take_bstri(arg_3(arguments));
    if (bstri1->size != bstri2->size || memcmp(bstri1->mem, bstri2->mem,
        bstri1->size * sizeof(ucharType)) != 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* bst_ne */



/**
 *  Convert a string to a 'bstring' value.
 *  @return the 'bstring' result of the conversion.
 *  @exception RANGE_ERROR When characters beyond '\255;' are present.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
objectType bst_parse (listType arguments)

  { /* bst_parse */
    isit_stri(arg_3(arguments));
    return bld_bstri_temp(
        bstParse(take_stri(arg_3(arguments))));
  } /* bst_parse */



/**
 *  Convert a 'bstring' value to a string.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
objectType bst_str (listType arguments)

  { /* bst_str */
    isit_bstri(arg_1(arguments));
    return bld_stri_temp(
        bstStr(take_bstri(arg_1(arguments))));
  } /* bst_str */



objectType bst_value (listType arguments)

  {
    objectType obj_arg;
    bstriType bstri;
    bstriType result;

  /* bst_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (unlikely(obj_arg == NULL ||
        CATEGORY_OF_OBJ(obj_arg) != BSTRIOBJECT ||
        take_bstri(obj_arg) == NULL)) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      bstri = take_bstri(obj_arg);
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, bstri->size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = bstri->size;
        memcpy(result->mem, bstri->mem,
            result->size * sizeof(ucharType));
        return bld_bstri_temp(result);
      } /* if */
    } /* if */
  } /* bst_value */
