/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2008  Thomas Mertes                        */
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
/*  File: seed7/src/typlib.c                                        */
/*  Changes: 1993, 1994, 1999, 2000  Thomas Mertes                  */
/*  Content: All primitive actions for the type type.               */
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
#include "data_rtl.h"
#include "heaputl.h"
#include "flistutl.h"
#include "datautl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "object.h"
#include "typeutl.h"
#include "executl.h"
#include "objutl.h"
#include "runerr.h"
#include "traceutl.h"
#include "typ_data.h"

#undef EXTERN
#define EXTERN
#include "typlib.h"



objectType typ_addinterface (listType arguments)

  {
    typeType typ1;
    typeType typ2;

  /* typ_addinterface */
    isit_type(arg_1(arguments));
    isit_type(arg_2(arguments));
    typ1 = take_type(arg_1(arguments));
    typ2 = take_type(arg_2(arguments));
    add_interface(typ1, typ2);
    return SYS_EMPTY_OBJECT;
  } /* typ_addinterface */



/**
 *  Compare two types.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objectType typ_cmp (listType arguments)

  {
    memSizeType typ1;
    memSizeType typ2;
    intType signumValue;

  /* typ_cmp */
    isit_type(arg_1(arguments));
    isit_type(arg_2(arguments));
    typ1 = (memSizeType) take_type(arg_1(arguments));
    typ2 = (memSizeType) take_type(arg_2(arguments));
    if (typ1 < typ2) {
      signumValue = -1;
    } else {
      signumValue = typ1 > typ2;
    } /* if */
    return bld_int_temp(signumValue);
  } /* typ_cmp */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType typ_cpy (listType arguments)

  {
    objectType dest;

  /* typ_cpy */
    dest = arg_1(arguments);
    isit_type(dest);
    is_variable(dest);
    isit_type(arg_3(arguments));
    dest->value.typeValue = take_type(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* typ_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType typ_create (listType arguments)

  {
    objectType dest;
    typeType type_from;

  /* typ_create */
    logFunction(printf("typ_create\n"););
    isit_type(arg_3(arguments));
    dest = arg_1(arguments);
    type_from = take_type(arg_3(arguments));
#ifdef TRACE_typ_create
    printf("\nbefore type assignment\n");
    trace1(dest);
    printf("\n");
    trace1(type_from->match_obj);
    printf("\n");
#endif
    SET_CATEGORY_OF_OBJ(dest, TYPEOBJECT);
    dest->value.typeValue = type_from;
    if (!VAR_OBJECT(dest)) {
      if (type_from->name == NULL &&
          HAS_ENTITY(dest) &&
          GET_ENTITY(dest)->ident != NULL) {
        /* printf("typ_create: Define ");
        trace1(dest);
        printf("\n"); */
        type_from->name = GET_ENTITY(dest)->ident;
      } /* if */
    } /* if */
    logFunction(printf("typ_create -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* typ_create */



/**
 *  Mark the type 'old_type/arg_1' as unused.
 */
objectType typ_destr (listType arguments)

  {
    /* typeType old_type; */

  /* typ_destr */
    logFunction(printf("typ_destr\n"););
    isit_type(arg_1(arguments));
    /* old_type = take_type(arg_1(arguments)); */
    /* printf("typ_destr "); trace1(old_type->match_obj); printf("\n"); */
    SET_UNUSED_FLAG(arg_1(arguments));
    logFunction(printf("typ_destr -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* typ_destr */



/**
 *  Check if two types are equal.
 *  @return TRUE if the two types are equal,
 *          FALSE otherwise.
 */
objectType typ_eq (listType arguments)

  {
    typeType type1;
    typeType type2;

  /* typ_eq */
    isit_type(arg_1(arguments));
    isit_type(arg_3(arguments));
    type1 = take_type(arg_1(arguments));
    type2 = take_type(arg_3(arguments));
    if (type1 == type2) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* typ_eq */



objectType typ_func (listType arguments)

  {
    typeType basic_type;
    typeType result;

  /* typ_func */
    logFunction(printf("typ_func\n"););
    isit_type(arg_2(arguments));
    basic_type = take_type(arg_2(arguments));
    if ((result = get_func_type(NULL, basic_type)) == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    /* printf("typ_func ");
    printobject(result);
    prot_cstri("=");
    prot_int((intType) result);
    printf("\n"); */
    logFunction(printf("typ_func -->\n"););
    return bld_type_temp(result);
  } /* typ_func */



objectType typ_gensub (listType arguments)

  {
    typeType meta_type;
    typeType result;

  /* typ_gensub */
    logFunction(printf("typ_gensub\n"););
    isit_type(arg_1(arguments));
    meta_type = take_type(arg_1(arguments));
    if ((result = new_type(meta_type->owningProg, meta_type, NULL)) == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    logFunction(printf("typ_gensub -->\n"););
    return bld_type_temp(result);
  } /* typ_gensub */



objectType typ_gentype (listType arguments)

  {
    typeType result;

  /* typ_gentype */
    logFunction(printf("typ_gentype\n"););
    if ((result = new_type(prog, NULL, NULL)) == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    logFunction(printf("typ_gentype -->\n"););
    return bld_type_temp(result);
  } /* typ_gentype */



#ifdef OUT_OF_ORDER
objectType typ_getinterfaces (listType arguments)

  {
    typeType typ1;

  /* typ_getinterfaces */
    isit_type(arg_1(arguments));
    typ1 = take_type(arg_1(arguments));
    get_interfaces(typ1);
    return SYS_EMPTY_OBJECT;
  } /* typ_getinterfaces */
#endif



#ifdef OUT_OF_ORDER
objectType typ_getcreate (listType arguments)

  {
    typeType result;

  /* typ_getcreate */
    isit_type(arg_1(arguments));
    ;
    get_create_call_obj(take_type(arg_1(arguments)), errInfoType *err_info)
    if ((result = new_type(prog, NULL, NULL)) == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    return bld_type_temp(result);
  } /* typ_getcreate */
#endif



objectType typ_hashcode (listType arguments)

  { /* typ_hashcode */
    isit_type(arg_1(arguments));
    return bld_int_temp((intType)
        (((memSizeType) take_type(arg_1(arguments))) >> 6));
  } /* typ_hashcode */



objectType typ_isdeclared (listType arguments)

  {
    objectType result;

  /* typ_isdeclared */
    if (CATEGORY_OF_OBJ(arg_1(arguments)) == DECLAREDOBJECT) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* typ_isdeclared */



objectType typ_isderived (listType arguments)

  {
    typeType any_type;
    objectType result;

  /* typ_isderived */
    isit_type(arg_1(arguments));
    any_type = take_type(arg_1(arguments));
    if (any_type->meta != NULL) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* typ_isderived */



objectType typ_isforward (listType arguments)

  {
    objectType result;

  /* typ_isforward */
    if (CATEGORY_OF_OBJ(arg_1(arguments)) == FORWARDOBJECT) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* typ_isforward */



objectType typ_isfunc (listType arguments)

  {
    typeType any_type;
    objectType result;

  /* typ_isfunc */
    isit_type(arg_1(arguments));
    any_type = take_type(arg_1(arguments));
    if (any_type->result_type != NULL && !any_type->is_varfunc_type) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* typ_isfunc */



objectType typ_isvarfunc (listType arguments)

  {
    typeType any_type;
    objectType result;

  /* typ_isvarfunc */
    isit_type(arg_1(arguments));
    any_type = take_type(arg_1(arguments));
    if (any_type->result_type != NULL && any_type->is_varfunc_type) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* typ_isvarfunc */



objectType typ_matchobj (listType arguments)

  {
    typeType actual_type;
    objectType result;

  /* typ_matchobj */
    isit_type(arg_1(arguments));
    actual_type = take_type(arg_1(arguments));
    result = actual_type->match_obj;
    return bld_reference_temp(result);
  } /* typ_matchobj */



objectType typ_meta (listType arguments)

  {
    typeType any_type;
    typeType result;

  /* typ_meta */
    logFunction(printf("typ_meta\n"););
    isit_type(arg_1(arguments));
    any_type = take_type(arg_1(arguments));
    if (any_type->meta == NULL) {
      logError(printf("typ_meta(");
               trace1(arg_1(arguments));
               printf("): No meta type.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    result = any_type->meta;
    logFunction(printf("typ_meta -->\n"););
    return bld_type_temp(result);
  } /* typ_meta */



/**
 *  Check if two types are not equal.
 *  @return TRUE if the two types are equal,
 *          FALSE otherwise.
 */
objectType typ_ne (listType arguments)

  {
    typeType type1;
    typeType type2;

  /* typ_ne */
    isit_type(arg_1(arguments));
    isit_type(arg_3(arguments));
    type1 = take_type(arg_1(arguments));
    type2 = take_type(arg_3(arguments));
    if (type1 != type2) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* typ_ne */



objectType typ_num (listType arguments)

  { /* typ_num */
    isit_type(arg_1(arguments));
    return bld_int_temp(
        typNum(take_type(arg_1(arguments))));
  } /* typ_num */



objectType typ_result (listType arguments)

  {
    typeType any_type;
    typeType result;

  /* typ_result */
    logFunction(printf("typ_result\n"););
    isit_type(arg_1(arguments));
    any_type = take_type(arg_1(arguments));
    if (any_type->result_type == NULL) {
      logError(printf("typ_result(");
               trace1(arg_1(arguments));
               printf("): No result type.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    result = any_type->result_type;
    logFunction(printf("typ_result\n"););
    return bld_type_temp(result);
  } /* typ_result */



objectType typ_str (listType arguments)

  { /* typ_str */
    isit_type(arg_1(arguments));
    return bld_stri_temp(
        typStr(take_type(arg_1(arguments))));
  } /* typ_str */



/**
 *  Get 'type' value of the object referenced by 'aReference/arg_1'.
 *  @return the 'type' value of the referenced object.
 *  @exception RANGE_ERROR If 'aReference/arg_1' is NIL or
 *             category(aReference) <> TYPEOBJECT holds.
 */
objectType typ_value (listType arguments)

  {
    objectType aReference;

  /* typ_value */
    isit_reference(arg_1(arguments));
    aReference = take_reference(arg_1(arguments));
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != TYPEOBJECT)) {
      logError(printf("typ_value(");
               trace1(aReference);
               printf("): Category is not TYPEOBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      return aReference;
    } /* if */
  } /* typ_value */



objectType typ_varconv (listType arguments)

  { /* typ_varconv */
    is_variable(arg_3(arguments));
    if (TEMP_OBJECT(arg_3(arguments))) {
      prot_cstri("TYP_VARCONV of TEMP_OBJECT ");
      trace1(arg_3(arguments));
      prot_nl();
    } /* if */
    return arg_3(arguments);
  } /* typ_varconv */



objectType typ_varfunc (listType arguments)

  {
    typeType basic_type;
    typeType result;

  /* typ_varfunc */
    logFunction(printf("typ_varfunc\n"););
    isit_type(arg_2(arguments));
    basic_type = take_type(arg_2(arguments));
    if ((result = get_varfunc_type(NULL, basic_type)) == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    /* printf("typ_varfunc ");
    printobject(result);
    prot_cstri("=");
    prot_int((intType) result);
    printf("\n"); */
    logFunction(printf("typ_varfunc -->\n"););
    return bld_type_temp(result);
  } /* typ_varfunc */
