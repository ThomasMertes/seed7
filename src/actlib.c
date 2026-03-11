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
/*  File: seed7/src/actlib.c                                        */
/*  Changes: 1991 - 1994, 2011, 2015, 2016, 2018  Thomas Mertes     */
/*  Content: All primitive actions for the action type.             */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "striutl.h"
#include "actutl.h"
#include "syvarutl.h"
#include "objutl.h"
#include "traceutl.h"
#include "name.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "actlib.h"



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType ace_cpy (listType arguments)

  {
    objectType dest;

  /* ace_cpy */
    dest = arg_1(arguments);
    isit_actentry(dest);
    is_variable(dest);
    isit_actentry(arg_3(arguments));
    dest->value.actEntryValue = take_actentry(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* ace_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType ace_create (listType arguments)

  {
    objectType dest;
    objectType source;

  /* ace_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_actentry(source);
    logFunction(printf("ace_create(*, action \"%s\")\n",
                       take_actentry(source)->name););
    SET_CATEGORY_OF_OBJ(dest, ACTENTRYOBJECT);
    dest->value.actEntryValue = take_actentry(source);
    return SYS_EMPTY_OBJECT;
  } /* ace_create */



/**
 *  Convert a string to an action.
 *  @param actionName/arg_2 Name of the action to be converted.
 *  @return an action which corresponds to the given string.
 *  @exception RANGE_ERROR No such action exists.
 */
objectType ace_gen (listType arguments)

  {
    striType actionName;
    const_actEntryType actEntry;

  /* ace_gen */
    isit_stri(arg_2(arguments));
    actionName = take_stri(arg_2(arguments));
    logFunction(printf("ace_gen(\"%s\")\n",
                       striAsUnquotedCStri(actionName)););
    actEntry = findActEntry(actionName);
    if (unlikely(actEntry == NULL)) {
      logError(printf("ace_gen(\"%s\"): No such action exists.\n",
                      striAsUnquotedCStri(actionName)););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_actentry_temp(actEntry);
    } /* if */
  } /* ace_gen */



/**
 *  Check if two action entries are equal.
 *  @return TRUE if both actions are equal,
 *          FALSE otherwise.
 */
objectType ace_eq (listType arguments)

  {
    const_actEntryType actEntry1;
    const_actEntryType actEntry2;

  /* ace_eq */
    isit_actentry(arg_1(arguments));
    isit_actentry(arg_3(arguments));
    actEntry1 = take_actentry(arg_1(arguments));
    actEntry2 = take_actentry(arg_3(arguments));
    if (actEntry1 == actEntry2) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* ace_eq */



/**
 *  Convert an integer number to an action.
 *  @param ordinal/arg_1 Number to be converted.
 *  @return an action which corresponds to the given integer.
 *  @exception RANGE_ERROR Number not in allowed range.
 */
objectType ace_iconv1 (listType arguments)

  {
    intType ordinal;

  /* ace_iconv1 */
    isit_int(arg_1(arguments));
    ordinal = take_int(arg_1(arguments));
    if (ordinal < 0 || (uintType) ordinal >= actTable.size) {
      logError(printf("ace_iconv(" FMT_D "): No such action exists.\n",
                      ordinal););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_actentry_temp(&actTable.table[ordinal]);
    } /* if */
  } /* ace_iconv1 */



/**
 *  Convert an integer number to an action.
 *  @param ordinal/arg_3 Number to be converted.
 *  @return an action which corresponds to the given integer.
 *  @exception RANGE_ERROR Number not in allowed range.
 */
objectType ace_iconv3 (listType arguments)

  {
    intType ordinal;

  /* ace_iconv3 */
    isit_int(arg_3(arguments));
    ordinal = take_int(arg_3(arguments));
    if (ordinal < 0 || (uintType) ordinal >= actTable.size) {
      logError(printf("ace_iconv(" FMT_D "): No such action exists.\n",
                      ordinal););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return bld_actentry_temp(&actTable.table[ordinal]);
    } /* if */
  } /* ace_iconv3 */



/**
 *  Check if two actions are not equal.
 *  @return FALSE if both actions are equal,
 *          TRUE otherwise.
 */
objectType ace_ne (listType arguments)

  {
    const_actEntryType actEntry1;
    const_actEntryType actEntry2;

  /* ace_ne */
    isit_actentry(arg_1(arguments));
    isit_actentry(arg_3(arguments));
    actEntry1 = take_actentry(arg_1(arguments));
    actEntry2 = take_actentry(arg_3(arguments));
    if (actEntry1 != actEntry2) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* ace_ne */



/**
 *  Get the ordinal number of an action.
 *  The action ACT_ILLEGAL has the ordinal number 0.
 *  @param anAction/arg_1 Action for which the ordinal number is determined.
 *  @return the ordinal number of the action.
 */
objectType ace_ord (listType arguments)

  {
    const_actEntryType actEntry;
    intType ordinal;

  /* ace_ord */
    isit_actentry(arg_1(arguments));
    actEntry = take_actentry(arg_1(arguments));
    if (unlikely(actEntry == NULL)) {
      logError(printf("ace_ord: NULL action entry.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      ordinal = actEntry - actTable.table;
      return bld_int_temp(ordinal);
    } /* if */
  } /* ace_ord */



/**
 *  Convert an action to a string.
 *  If the action is not found in the table of legal actions
 *  the string "ACT_ILLEGAL" is returned.
 *  @param anAction/arg_1 Action which is converted to a string..
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType ace_str (listType arguments)

  {
    const_actEntryType actEntry;
    striType actionName;

  /* ace_str */
    isit_actentry(arg_1(arguments));
    actEntry = take_actentry(arg_1(arguments));
    if (unlikely(actEntry == NULL)) {
      logError(printf("ace_str: NULL action entry.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      actionName = cstri_to_stri(actEntry->name);
      if (actionName == NULL) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        return bld_stri_temp(actionName);
      } /* if */
    } /* if */
  } /* ace_str */



/**
 *  Get 'ACTION' value of the object referenced by 'aReference/arg_1'.
 *  @return the 'ACTION' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference/arg_1' is NIL or
 *             category(aReference) <> ACTOBJECT holds.
 */
objectType ace_value (listType arguments)

  {
    objectType obj_arg;

  /* ace_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (unlikely(obj_arg == NULL)) {
      logError(printf("ace_value(NULL): Object is NULL.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else if (CATEGORY_OF_OBJ(obj_arg) == ACTENTRYOBJECT) {
      return bld_actentry_temp(take_actentry(obj_arg));
    } else if (CATEGORY_OF_OBJ(obj_arg) == ACTOBJECT) {
      return bld_actentry_temp(getActEntry(take_action(obj_arg)));
    } else {
      logError(printf("ace_value(");
               trace1(obj_arg);
               printf("): Category is not ACTOBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
  } /* ace_value */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType act_create (listType arguments)

  {
    objectType dest;
    objectType source;
    const_actEntryType actEntry;

  /* act_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    logFunction(printf("act_create(");
                trace1(dest);
                printf(", ");
                trace1(source);
                printf(")\n"););
    isit_actentry(source);
    actEntry = take_actentry(source);
    disconnect_param_entities(dest);
    SET_CATEGORY_OF_OBJ(dest, ACTOBJECT);
    dest->value.actValue = actEntry->action;
    return SYS_EMPTY_OBJECT;
  } /* act_create */



/**
 *  Raise the exception ILLEGAL_ACTION.
 *  @return ILLEGAL_ACTION
 */
objectType act_illegal (listType arguments)

  { /* act_illegal */
    return raise_exception(SYS_ACT_ILLEGAL_EXCEPTION);
  } /* act_illegal */
