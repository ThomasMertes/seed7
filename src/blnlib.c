/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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
/*  Changes: 1999  Thomas Mertes                                    */
/*  Content: All primitive actions for the boolean type.            */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "exec.h"
#include "objutl.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "blnlib.h"



/**
 *  Logical 'and' for two boolean values.
 *  The logical 'and' operator works strictly left to right.
 *  When the result can be determined after the evaluation of
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



objectType bln_cpy (listType arguments)

  {
    objectType bool_variable;

  /* bln_cpy */
    bool_variable = arg_1(arguments);
    isit_bool(bool_variable);
    is_variable(bool_variable);
    isit_bool(arg_3(arguments));
    bool_variable->value.objValue = take_bool(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* bln_cpy */



objectType bln_create (listType arguments)

  {
    objectType bool_to;
    objectType bool_from;

  /* bln_create */
    bool_to = arg_1(arguments);
    isit_bool(arg_3(arguments));
    bool_from = take_bool(arg_3(arguments));
    if (VAR_OBJECT(bool_to)) {
      SET_CATEGORY_OF_OBJ(bool_to, VARENUMOBJECT);
    } else {
      SET_CATEGORY_OF_OBJ(bool_to, CONSTENUMOBJECT);
    } /* if */
    bool_to->value.objValue = bool_from;
    return SYS_EMPTY_OBJECT;
  } /* bln_create */



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
 *  @return TRUE when number is odd,
 *          FALSE otherwise.
 */
objectType bln_iconv (listType arguments)

  { /* bln_iconv */
    isit_int(arg_3(arguments));
    if (take_int(arg_3(arguments)) & 1) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* bln_iconv */



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
 *  When the result can be determined after the evaluation of
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
 *  @return 0 when boolValue is FALSE and
 *          1 when boolValue is TRUE.
 */
objectType bln_ord (listType arguments)

  { /* bln_ord */
    isit_bool(arg_1(arguments));
    if (take_bool(arg_1(arguments)) == SYS_TRUE_OBJECT) {
      return bld_int_temp(1);
    } else {
      return bld_int_temp(0);
    } /* if */
  } /* bln_ord */



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
    return raise_exception(SYS_RNG_EXCEPTION);
  } /* bln_value */
