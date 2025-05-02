/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2025  Thomas Mertes                        */
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
/*  File: seed7/src/sellib.c                                        */
/*  Changes: 2025  Thomas Mertes                                    */
/*  Content: All primitive actions for the structElement type.      */
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
#include "objutl.h"
#include "ref_data.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "sellib.h"



/**
 *  Compare two struct elements.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objectType sel_cmp (listType arguments)

  {
    memSizeType ref1;
    memSizeType ref2;
    intType signumValue;

  /* sel_cmp */
    isit_structelem(arg_1(arguments));
    isit_structelem(arg_2(arguments));
    ref1 = (memSizeType) take_structelem(arg_1(arguments));
    ref2 = (memSizeType) take_structelem(arg_2(arguments));
    if (ref1 < ref2) {
      signumValue = -1;
    } else {
      signumValue = ref1 > ref2;
    } /* if */
    return bld_int_temp(signumValue);
  } /* sel_cmp */



/**
 *  Convert a reference to a struct element object.
 *  @return the referenced object as struct element object.
 */
objectType sel_conv1 (listType arguments)

  { /* sel_conv1 */
    isit_reference(arg_1(arguments));
    return bld_structelem_temp(take_reference(arg_1(arguments)));
  } /* sel_conv1 */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType sel_cpy (listType arguments)

  {
    objectType dest;

  /* sel_cpy */
    dest = arg_1(arguments);
    isit_structelem(dest);
    is_variable(dest);
    isit_structelem(arg_3(arguments));
    dest->value.objValue = take_structelem(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* sel_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType sel_create (listType arguments)

  {
    objectType dest;
    objectType source;

  /* sel_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_structelem(source);
    SET_CATEGORY_OF_OBJ(dest, STRUCTELEMOBJECT);
    dest->value.objValue = take_structelem(source);
    return SYS_EMPTY_OBJECT;
  } /* sel_create */



/**
 *  Check if two struct element objects are equal.
 *  @return TRUE if both struct elements are equal,
 *          FALSE otherwise.
 */
objectType sel_eq (listType arguments)

  { /* sel_eq */
    isit_structelem(arg_1(arguments));
    isit_structelem(arg_3(arguments));
    if (take_structelem(arg_1(arguments)) ==
        take_structelem(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* sel_eq */



/**
 *  Compute the hash value of a struct element object.
 *  @return the hash value.
 */
objectType sel_hashcode (listType arguments)

  { /* sel_hashcode */
    isit_structelem(arg_1(arguments));
    return bld_int_temp((intType)
        (((memSizeType) take_structelem(arg_1(arguments))) >> 6));
  } /* sel_hashcode */



/**
 *  Check if two struct element objects are not equal.
 *  @return FALSE if both struct elements are equal,
 *          TRUE otherwise.
 */
objectType sel_ne (listType arguments)

  { /* sel_ne */
    isit_structelem(arg_1(arguments));
    isit_structelem(arg_3(arguments));
    if (take_structelem(arg_1(arguments)) !=
        take_structelem(arg_3(arguments))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* sel_ne */



objectType sel_str (listType arguments)

  { /* sel_str */
    isit_structelem(arg_1(arguments));
    return bld_stri_temp(refStr(
        take_structelem(arg_1(arguments))));
  } /* sel_str */



objectType sel_symb (listType arguments)

  {
    objectType symb_object;

  /* sel_symb */
    isit_structelem(arg_2(arguments));
    symb_object = take_structelem(arg_2(arguments));
    logFunction(printf("sel_symb(" FMT_U_MEM " ",
                       (memSizeType) GET_ENTITY(symb_object));
                trace1(symb_object);
                printf(")\n"););
    if (HAS_ENTITY(symb_object) &&
        GET_ENTITY(symb_object)->syobject != NULL) {
      symb_object = GET_ENTITY(symb_object)->syobject;
    } else {
      logError(printf("ref symb (" FMT_U_MEM " ",
                      (memSizeType) GET_ENTITY(symb_object));
               trace1(symb_object);
               printf("): Error\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    logFunction(printf("sel_symb --> " FMT_U_MEM " ",
                       (memSizeType) GET_ENTITY(symb_object));
                trace1(symb_object);
                printf("\n"););
    return bld_param_temp(symb_object);
  } /* sel_symb */



/**
 *  Get the type of a struct element.
 *  @return the type of the struct element obj_arg/arg_1.
 */
objectType sel_type (listType arguments)

  {
    objectType obj_arg;

  /* sel_type */
    isit_structelem(arg_1(arguments));
    obj_arg = take_structelem(arg_1(arguments));
    if (obj_arg == NULL) {
      logError(printf("sel_type(0): Null argument.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else if (obj_arg->type_of == NULL) {
      logError(printf("sel_type(" FMT_U_MEM "): Type is null.\n",
                      (memSizeType) obj_arg););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_type_temp(obj_arg->type_of);
    } /* if */
  } /* sel_type */
