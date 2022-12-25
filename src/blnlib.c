/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000, 2013, 2016, 2020  Thomas Mertes      */
/*                2021  Thomas Mertes                               */
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
/*  File: seed7/src/blnlib.c                                        */
/*  Changes: 1999, 2013, 2016, 2020, 2021  Thomas Mertes            */
/*  Content: All primitive actions for the boolean type.            */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "exec.h"
#include "objutl.h"
#include "traceutl.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "blnlib.h"



/**
 *  Logical 'and' for two boolean values.
 *  The logical 'and' operator works strictly left to right.
 *  If the result can be determined after the evaluation of
 *  'boolValue1' the right operand ('boolValue2') is not evaluated.
 *  @return TRUE if boolValue1 is TRUE and boolValue2 is TRUE,
 *          FALSE otherwise.
 */
objectType bln_and (listType arguments)

  {
    objectType result;

  /* bln_and */
    isit_bool(arg_1(arguments));
    if (take_bool(arg_1(arguments)) == SYS_FALSE_OBJECT) {
      return SYS_FALSE_OBJECT;
    } else {
      result = evaluate(arg_3(arguments));
      if (!fail_flag) {
        isit_bool(result);
        result = take_bool(result);
      } /* if */
      return result;
    } /* if */
  } /* bln_and */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType bln_cpy (listType arguments)

  {
    objectType dest;

  /* bln_cpy */
    dest = arg_1(arguments);
    isit_bool(dest);
    is_variable(dest);
    isit_bool(arg_3(arguments));
    dest->value.objValue = take_bool(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* bln_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType bln_create (listType arguments)

  {
    objectType dest;
    objectType source;

  /* bln_create */
    dest = arg_1(arguments);
    isit_bool(arg_3(arguments));
    source = take_bool(arg_3(arguments));
    if (VAR_OBJECT(dest)) {
      SET_CATEGORY_OF_OBJ(dest, VARENUMOBJECT);
    } else {
      SET_CATEGORY_OF_OBJ(dest, CONSTENUMOBJECT);
    } /* if */
    dest->value.objValue = source;
    return SYS_EMPTY_OBJECT;
  } /* bln_create */



/**
 *  Check if two boolean values are equal.
 *  @return TRUE if the two boolean values are equal,
 *          FALSE otherwise.
 */
objectType bln_eq (listType arguments)

  { /* bln_eq */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == take_bool(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* bln_eq */



/**
 *  Check if boolValue1 is greater than or equal to boolValue2.
 *  @return TRUE if boolValue1 is TRUE or boolValue2 is FALSE,
 *          FALSE otherwise.
 */
objectType bln_ge (listType arguments)

  { /* bln_ge */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      return SYS_TRUE_OBJECT;
    } else {
      if (take_bool(arg_3(arguments)) == SYS_TRUE_OBJECT) {
        return SYS_FALSE_OBJECT;
      } else {
        return SYS_TRUE_OBJECT;
      } /* if */
    } /* if */
  } /* bln_ge */



/**
 *  Check if boolValue1 is greater than boolValue2.
 *  @return TRUE if boolValue1 is TRUE and boolValue2 is FALSE,
 *          FALSE otherwise.
 */
objectType bln_gt (listType arguments)

  { /* bln_gt */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == SYS_FALSE_OBJECT) {
      return SYS_FALSE_OBJECT;
    } else {
      if (take_bool(arg_3(arguments)) == SYS_TRUE_OBJECT) {
        return SYS_FALSE_OBJECT;
      } else {
        return SYS_TRUE_OBJECT;
      } /* if */
    } /* if */
  } /* bln_gt */



/**
 *  Convert an integer to a boolean value.
 *  @return FALSE if number is 0,
 *          TRUE if number is 1.
 *  @exception RANGE_ERROR If 'number/arg_1' is neither 0 nor 1.
 */
objectType bln_iconv1 (listType arguments)

  {
    intType number;

  /* bln_iconv1 */
    isit_int(arg_1(arguments));
    number = take_int(arg_1(arguments));
    if (unlikely(number < 0 || number >= 2)) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else if (number != 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* bln_iconv1 */



/**
 *  Convert an integer to a boolean value.
 *  @return FALSE if number is 0,
 *          TRUE if number is 1.
 *  @exception RANGE_ERROR If 'number/arg_3' is neither 0 nor 1.
 */
objectType bln_iconv3 (listType arguments)

  {
    intType number;

  /* bln_iconv3 */
    isit_int(arg_3(arguments));
    number = take_int(arg_3(arguments));
    if (unlikely(number < 0 || number >= 2)) {
      return raise_exception(SYS_RNG_EXCEPTION);
    } else if (number != 0) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* bln_iconv3 */



/**
 *  Check if boolValue1 is less than or equal to boolValue2.
 *  @return TRUE if boolValue1 is FALSE or boolValue2 is TRUE,
 *          FALSE otherwise.
 */
objectType bln_le (listType arguments)

  { /* bln_le */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == SYS_FALSE_OBJECT) {
      return SYS_TRUE_OBJECT;
    } else {
      return take_bool(arg_3(arguments));
    } /* if */
  } /* bln_le */



/**
 *  Check if boolValue1 is less than boolValue2.
 *  @return TRUE if boolValue1 is FALSE and boolValue2 is TRUE,
 *          FALSE otherwise.
 */
objectType bln_lt (listType arguments)

  { /* bln_lt */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      return SYS_FALSE_OBJECT;
    } else {
      return take_bool(arg_3(arguments));
    } /* if */
  } /* bln_lt */



/**
 *  Check if two boolean values are not equal.
 *  @return FALSE if the two boolean values are equal,
 *          TRUE otherwise.
 */
objectType bln_ne (listType arguments)

  { /* bln_ne */
    isit_bool(arg_1(arguments));
    isit_bool(arg_3(arguments));
    if (take_bool(arg_1(arguments)) != take_bool(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* bln_ne */



/**
 *  Negate a boolean value.
 *  @return TRUE if boolValue is FALSE,
 *          FALSE otherwise.
 */
objectType bln_not (listType arguments)

  { /* bln_not */
    isit_bool(arg_2(arguments));
    if (take_bool(arg_2(arguments)) == SYS_TRUE_OBJECT) {
      return SYS_FALSE_OBJECT;
    } else {
      return SYS_TRUE_OBJECT;
    } /* if */
  } /* bln_not */



/**
 *  Inclusive logical 'or' for two boolean values.
 *  The logical 'or' operator works strictly left to right.
 *  If the result can be determined after the evaluation of
 *  'boolValue1' the right operand ('boolValue2') is not evaluated.
 *  @return TRUE if boolValue1 is TRUE or boolValue2 is TRUE (or both are true),
 *          FALSE otherwise.
 */
objectType bln_or (listType arguments)

  {
    objectType result;

  /* bln_or */
    isit_bool(arg_1(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      return SYS_TRUE_OBJECT;
    } else {
      result = evaluate(arg_3(arguments));
      if (!fail_flag) {
        isit_bool(result);
        result = take_bool(result);
      } /* if */
      return result;
    } /* if */
  } /* bln_or */



/**
 *  Convert to integer.
 *  @return 0 if boolValue is FALSE, and
 *          1 if boolValue is TRUE.
 */
objectType bln_ord (listType arguments)

  {
    intType ordinal;

  /* bln_ord */
    isit_bool(arg_1(arguments));
    ordinal = take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT;
    return bld_int_temp(ordinal);
  } /* bln_ord */



/**
 *  Predecessor of a boolean value.
 *  @return FALSE, if 'boolValue/arg_1' is TRUE.
 *  @exception RANGE_ERROR If 'boolValue/arg_1' is FALSE.
 */
objectType bln_pred (listType arguments)

  { /* bln_pred */
    isit_bool(arg_1(arguments));
    if (likely(take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT)) {
      return SYS_FALSE_OBJECT;
    } else {
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
  } /* bln_pred */


/**
 *  Successor of a boolean value.
 *  @return TRUE, if 'boolValue/arg_1' is FALSE.
 *  @exception RANGE_ERROR If 'boolValue/arg_1' is TRUE.
 */
objectType bln_succ (listType arguments)

  { /* bln_succ */
    isit_bool(arg_1(arguments));
    if (likely(take_bool(arg_1(arguments)) == SYS_FALSE_OBJECT)) {
      return SYS_TRUE_OBJECT;
    } else {
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
  } /* bln_succ */


/**
 *  Ternary operator condition/arg_1 ? thenValue/arg_3 : elseValue/arg_5
 *  @return thenValue/arg_3 if condition/arg_1 is TRUE, and
 *          elseValue/arg_5 if condition/arg_1 is FALSE.
 */
objectType bln_ternary (listType arguments)

  {
    objectType result;

  /* bln_ternary */
    isit_bool(arg_1(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      result = evaluate(arg_3(arguments));
    } else {
      result = evaluate(arg_5(arguments));
    } /* if */
    if (result != NULL &&
        (CATEGORY_OF_OBJ(result) == CONSTENUMOBJECT ||
         CATEGORY_OF_OBJ(result) == VARENUMOBJECT)) {
      result = result->value.objValue;
    } /* if */
    return result;
  } /* bln_ternary */



/**
 *  Get 'boolean' value of the object referenced by 'aReference/arg_1'.
 *  @return the 'boolean' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference/arg_1' is NIL or
 *             if the value is not TRUE_OBJECT or FALSE_OBJECT.
 */
objectType bln_value (listType arguments)

  {
    objectType obj_arg;

  /* bln_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg != NULL) {
      if (CATEGORY_OF_OBJ(obj_arg) == CONSTENUMOBJECT ||
          CATEGORY_OF_OBJ(obj_arg) == VARENUMOBJECT) {
        obj_arg = obj_arg->value.objValue;
      } /* if */
      if (obj_arg->type_of != NULL &&
          obj_arg->type_of->owningProg != NULL) {
        if (obj_arg == TRUE_OBJECT(obj_arg->type_of->owningProg)) {
          return SYS_TRUE_OBJECT;
        } else if (obj_arg == FALSE_OBJECT(obj_arg->type_of->owningProg)) {
          return SYS_FALSE_OBJECT;
        } /* if */
      } /* if */
    } /* if */
    logError(printf("bln_value(");
             trace1(obj_arg);
             printf("): Value not TRUE_OBJECT or FALSE_OBJECT.\n"););
    return raise_exception(SYS_RNG_EXCEPTION);
  } /* bln_value */
