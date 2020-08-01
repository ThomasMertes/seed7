/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2019  Thomas Mertes                        */
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
/*  Changes: 2007, 2010, 2013, 2015, 2016, 2018  Thomas Mertes      */
/*  Content: All primitive actions for the byte string type.        */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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



/**
 *  Append the bstring extension/arg_3 to destination/arg_1.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             bstring.
 */
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
              bstr_from->size);
          new_bstr->size = new_size;
          bstr_variable->value.bstriValue = new_bstr;
        } /* if */
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* bst_append */



/**
 *  Concatenate two bstrings.
 *  @return the result of the concatenation.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             bstring.
 */
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
              bstri2->size);
          arg_1(arguments)->value.bstriValue = NULL;
          return bld_bstri_temp(result);
        } /* if */
      } else {
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, result_size))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          result->size = result_size;
          memcpy(result->mem, bstri1->mem, bstri1_size);
          memcpy(&result->mem[bstri1_size], bstri2->mem, bstri2->size);
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

  { /* bst_cmp */
    isit_bstri(arg_1(arguments));
    isit_bstri(arg_2(arguments));
    return bld_int_temp(
        bstCmp(take_bstri(arg_1(arguments)),
               take_bstri(arg_2(arguments))));
  } /* bst_cmp */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
objectType bst_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    memSizeType new_size;
    bstriType bstri_dest;

  /* bst_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_bstri(dest);
    isit_bstri(source);
    is_variable(dest);
    bstri_dest = take_bstri(dest);
    if (TEMP_OBJECT(source)) {
      FREE_BSTRI(bstri_dest, bstri_dest->size);
      dest->value.bstriValue = take_bstri(source);
      source->value.bstriValue = NULL;
    } else {
      new_size = take_bstri(source)->size;
      if (bstri_dest->size == new_size) {
        if (bstri_dest != take_bstri(source)) {
          /* It is possible that dest == source holds. The   */
          /* behavior of memcpy() is undefined if source and */
          /* destination areas overlap (or are identical).   */
          /* Therefore a check for this case is necessary.   */
          memcpy(bstri_dest->mem, take_bstri(source)->mem, new_size);
        } /* if */
      } else {
        if (unlikely(!ALLOC_BSTRI_SIZE_OK(bstri_dest, new_size))) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          FREE_BSTRI(take_bstri(dest), take_bstri(dest)->size);
          dest->value.bstriValue = bstri_dest;
          bstri_dest->size = new_size;
        } /* if */
        memcpy(bstri_dest->mem, take_bstri(source)->mem, new_size);
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* bst_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType bst_create (listType arguments)

  {
    objectType dest;
    objectType source;
    memSizeType new_size;
    bstriType new_bstri;

  /* bst_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_bstri(source);
    SET_CATEGORY_OF_OBJ(dest, BSTRIOBJECT);
    if (TEMP_OBJECT(source)) {
      dest->value.bstriValue = take_bstri(source);
      source->value.bstriValue = NULL;
    } else {
/*    printf("bstri_create %d !!!\n", in_file.line); */
      new_size = take_bstri(source)->size;
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(new_bstri, new_size))) {
        dest->value.bstriValue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      dest->value.bstriValue = new_bstri;
      new_bstri->size = new_size;
      memcpy(new_bstri->mem, take_bstri(source)->mem, new_size);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* bst_create */



/**
 *  Free the memory referred by 'old_bstring/arg_1'.
 *  After bst_destr is left 'old_bstring/arg_1' is NULL.
 *  The memory where 'old_bstring/arg_1' is stored can be
 *  freed afterwards.
 */
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



/**
 *  Get an empty bstring.
 *  @return an empty bstring.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType bst_empty (listType arguments)

  {
    bstriType result;

  /* bst_empty */
    if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, 0))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      /* Note that the size of the allocated memory is smaller, */
      /* than the struct. But this is okay, because the element */
      /* 'mem' respectively 'mem1' is not used. */
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
        bstri1->size) == 0) {
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
 *  @exception RANGE_ERROR If the index is less than 1 or
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
      logError(printf("bst_idx(\"%s\", " FMT_D "): Position %s.\n",
                      bstriAsUnquotedCStri(bstri), position,
                      position <= 0 ? "<= 0" : "> length(string)"););
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
      logError(printf("bst_lng(\"%s\"): Length does not fit into integer.\n",
                      bstriAsUnquotedCStri(bstri)););
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
        bstri1->size) != 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* bst_ne */



/**
 *  Convert a string to a 'bstring' value.
 *  @return the 'bstring' result of the conversion.
 *  @exception RANGE_ERROR If characters beyond '\255;' are present.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType bst_parse1 (listType arguments)

  { /* bst_parse1 */
    isit_stri(arg_1(arguments));
    return bld_bstri_temp(
        bstParse(take_stri(arg_1(arguments))));
  } /* bst_parse1 */



/**
 *  Convert a 'bstring' value to a string.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType bst_str (listType arguments)

  { /* bst_str */
    isit_bstri(arg_1(arguments));
    return bld_stri_temp(
        bstStr(take_bstri(arg_1(arguments))));
  } /* bst_str */



/**
 *  Get 'bstring' value of the object referenced by 'aReference/arg_1'.
 *  @return the 'bstring' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference/arg_1' is NIL or
 *             category(aReference) <> BSTRIOBJECT holds.
 */
objectType bst_value (listType arguments)

  {
    objectType aReference;
    bstriType bstri;
    bstriType result;

  /* bst_value */
    isit_reference(arg_1(arguments));
    aReference = take_reference(arg_1(arguments));
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != BSTRIOBJECT ||
                 take_bstri(aReference) == NULL)) {
      logError(printf("bst_value(");
               trace1(aReference);
               printf("): Category is not BSTRIOBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      bstri = take_bstri(aReference);
      if (unlikely(!ALLOC_BSTRI_SIZE_OK(result, bstri->size))) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        result->size = bstri->size;
        memcpy(result->mem, bstri->mem, result->size);
        return bld_bstri_temp(result);
      } /* if */
    } /* if */
  } /* bst_value */
