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
/*  File: seed7/src/chrlib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions for the char type.               */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "ctype.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "objutl.h"
#include "runerr.h"
#include "chr_rtl.h"

#undef EXTERN
#define EXTERN
#include "chrlib.h"



objectType chr_clit (listType arguments)

  { /* chr_clit */
    isit_char(arg_1(arguments));
    return bld_stri_temp(
        chrCLit(take_char(arg_1(arguments))));
  } /* chr_clit */



/**
 *  Compare two characters.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objectType chr_cmp (listType arguments)

  {
    charType char1;
    charType char2;
    intType signumValue;

  /* chr_cmp */
    isit_char(arg_1(arguments));
    isit_char(arg_2(arguments));
    char1 = take_char(arg_1(arguments));
    char2 = take_char(arg_2(arguments));
    if (char1 < char2) {
      signumValue = -1;
    } else {
      signumValue = char1 > char2;
    } /* if */
    return bld_int_temp(signumValue);
  } /* chr_cmp */



objectType chr_conv (listType arguments)

  { /* chr_conv */
    isit_char(arg_3(arguments));
    return bld_char_temp(take_char(arg_3(arguments)));
  } /* chr_conv */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType chr_cpy (listType arguments)

  {
    objectType dest;

  /* chr_cpy */
    dest = arg_1(arguments);
    isit_char(dest);
    is_variable(dest);
    isit_char(arg_3(arguments));
    dest->value.charValue = take_char(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* chr_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType chr_create (listType arguments)

  { /* chr_create */
    isit_char(arg_3(arguments));
    SET_CATEGORY_OF_OBJ(arg_1(arguments), CHAROBJECT);
    arg_1(arguments)->value.charValue = take_char(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* chr_create */



/**
 *  Decrement a character.
 *  This is equivalent to:
 *   ch = pred(ch);
 */
objectType chr_decr (listType arguments)

  {
    objectType ch_variable;

  /* chr_decr */
    ch_variable = arg_1(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    ch_variable->value.charValue = take_char(ch_variable) - 1;
    return SYS_EMPTY_OBJECT;
  } /* chr_decr */



/**
 *  Check if two characters are equal.
 *  @return TRUE if both characters are equal,
 *          FALSE otherwise.
 */
objectType chr_eq (listType arguments)

  { /* chr_eq */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) ==
        take_char(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* chr_eq */



/**
 *  Check if ch1 is greater than or equal to ch2.
 *  @return TRUE if ch1 is greater than or equal to ch2,
 *          FALSE otherwise.
 */
objectType chr_ge (listType arguments)

  { /* chr_ge */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) >=
        take_char(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* chr_ge */



/**
 *  Check if ch1 is greater than ch2.
 *  @return TRUE if ch1 is greater than ch2,
 *          FALSE otherwise.
 */
objectType chr_gt (listType arguments)

  { /* chr_gt */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) >
        take_char(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* chr_gt */



/**
 *  Compute the hash value of a character.
 *  @return the hash value.
 */
objectType chr_hashcode (listType arguments)

  { /* chr_hashcode */
    isit_char(arg_1(arguments));
    return bld_int_temp((intType) (
        (scharType) take_char(arg_1(arguments))));
  } /* chr_hashcode */



/**
 *  Convert an integer number to a character.
 *  For values between 0 and 1114111 a Unicode character is returned.
 *  For values between 1114112 and 1114368 a cursor or function key
 *  is returned. For -1 the value EOF is returned.
 *  @return a character which corresponds to the given integer.
 *  @exception RANGE_ERROR When the number does not fit into a 32-bit 'char'.
 */
objectType chr_iconv1 (listType arguments)

  {
    intType number;

  /* chr_iconv1 */
    isit_int(arg_1(arguments));
    number = take_int(arg_1(arguments));
#if INTTYPE_SIZE > 32
    if (unlikely(number < INT32TYPE_MIN || number > INT32TYPE_MAX)) {
      logError(printf("chr_iconv1(" FMT_D "): "
                      "Number does not fit into a 32-bit char.\n",
                      number););
      return raise_exception(SYS_RNG_EXCEPTION) ;
    } else {
      return bld_char_temp((charType) number);
    } /* if */
#else
    return bld_char_temp((charType) number);
#endif
  } /* chr_iconv1 */



/**
 *  Convert an integer number to a character.
 *  For values between 0 and 1114111 a Unicode character is returned.
 *  For values between 1114112 and 1114368 a cursor or function key
 *  is returned. For -1 the value EOF is returned.
 *  @return a character which corresponds to the given integer.
 *  @exception RANGE_ERROR When the number does not fit into a 32-bit 'char'.
 */
objectType chr_iconv3 (listType arguments)

  {
    intType number;

  /* chr_iconv3 */
    isit_int(arg_3(arguments));
    number = take_int(arg_3(arguments));
#if INTTYPE_SIZE > 32
    if (unlikely(number < INT32TYPE_MIN || number > INT32TYPE_MAX)) {
      logError(printf("chr_iconv3(" FMT_D "): "
                      "Number does not fit into a 32-bit char.\n",
                      number););
      return raise_exception(SYS_RNG_EXCEPTION) ;
    } else {
      return bld_char_temp((charType) number);
    } /* if */
#else
    return bld_char_temp((charType) number);
#endif
  } /* chr_iconv3 */



/**
 *  Increment a character.
 *  This is equivalent to:
 *   ch = succ(ch);
 */
objectType chr_incr (listType arguments)

  {
    objectType ch_variable;

  /* chr_incr */
    ch_variable = arg_1(arguments);
    isit_char(ch_variable);
    is_variable(ch_variable);
    ch_variable->value.charValue = take_char(ch_variable) + 1;
    return SYS_EMPTY_OBJECT;
  } /* chr_incr */



/**
 * Check whether a character is an alphabetic Unicode character.
 * @return TRUE if alphabetic property holds,
 *         FALSE otherwise
 */
objectType chr_is_letter (listType arguments)

  { /* chr_is_letter */
    isit_char(arg_1(arguments));
    if (chrIsLetter(take_char(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    }
  } /* chr_is_letter */



/**
 *  Check if ch1 is less than or equal to ch2.
 *  @return TRUE if ch1 is less than or equal to ch2,
 *          FALSE otherwise.
 */
objectType chr_le (listType arguments)

  { /* chr_le */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) <=
        take_char(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* chr_le */



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
objectType chr_low (listType arguments)

  { /* chr_low */
    isit_char(arg_1(arguments));
    return bld_char_temp(chrLow(take_char(arg_1(arguments))));
  } /* chr_low */



/**
 *  Check if ch1 is less than ch2.
 *  @return TRUE if ch1 is less than than ch2,
 *          FALSE otherwise.
 */
objectType chr_lt (listType arguments)

  { /* chr_lt */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) <
        take_char(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* chr_lt */



/**
 *  Check if two characters are not equal.
 *  @return FALSE if both characters are equal,
 *          TRUE otherwise.
 */
objectType chr_ne (listType arguments)

  { /* chr_ne */
    isit_char(arg_1(arguments));
    isit_char(arg_3(arguments));
    if (take_char(arg_1(arguments)) !=
        take_char(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* chr_ne */



/**
 *  Get the ordinal number of a character.
 *  For Unicode characters a value between 0 and 1114111 is returned.
 *  For EOF the value -1 is returned.
 *  @return the ordinal number of a character.
 */
objectType chr_ord (listType arguments)

  { /* chr_ord */
    isit_char(arg_1(arguments));
    return bld_int_temp((intType) (
        (scharType) take_char(arg_1(arguments))));
  } /* chr_ord */



/**
 *  Predecessor of a character.
 *  @return chr(ord(ch) - 1)
 */
objectType chr_pred (listType arguments)

  { /* chr_pred */
    isit_char(arg_1(arguments));
    return bld_char_temp(take_char(arg_1(arguments)) - 1);
  } /* chr_pred */



/**
 *  Create a string with one character.
 *  @return a string with the character 'ch'.
 */
objectType chr_str (listType arguments)

  {
    striType result;

  /* chr_str */
    isit_char(arg_1(arguments));
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, 1))) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      result->size = 1;
      result->mem[0] = (strElemType) take_char(arg_1(arguments));
      return bld_stri_temp(result);
    } /* if */
  } /* chr_str */



/**
 *  Successor of a character.
 *  @return chr(ord(ch) + 1)
 */
objectType chr_succ (listType arguments)

  { /* chr_succ */
    isit_char(arg_1(arguments));
    return bld_char_temp(take_char(arg_1(arguments)) + 1);
  } /* chr_succ */



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
objectType chr_up (listType arguments)

  { /* chr_up */
    isit_char(arg_1(arguments));
    return bld_char_temp(chrUp(take_char(arg_1(arguments))));
  } /* chr_up */



objectType chr_value (listType arguments)

  {
    objectType obj_arg;

  /* chr_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (unlikely(obj_arg == NULL ||
                 CATEGORY_OF_OBJ(obj_arg) != CHAROBJECT)) {
      logError(printf("chr_value(");
               trace1(obj_arg);
               printf("): Category is not CHAROBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_char_temp(take_char(obj_arg));
    } /* if */
  } /* chr_value */



/**
 * Calculate the displayed width of a Unicode character.
 * Non-spacing characters and control characters have width of 0.
 * Most ideographic symbols have width 2.
 * @return 0,1 or 2
 */
objectType chr_width (listType arguments)

  { /* chr_width */
    isit_char(arg_1(arguments));
    return bld_int_temp(chrWidth(take_char(arg_1(arguments))));
  } /* chr_width */
