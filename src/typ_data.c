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
/*  Module: Compiler data reflection                                */
/*  File: seed7/src/typ_data.c                                      */
/*  Changes: 1993, 1994, 1999, 2000, 2008, 2010  Thomas Mertes      */
/*  Content: Primitive actions for the type type.                   */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "datautl.h"
#include "typeutl.h"
#include "striutl.h"
#include "hsh_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "typ_data.h"


#define TYPE_TABLE_INCREMENT 127



intType typCmp (const const_typeType type1, const const_typeType type2)

  { /* typCmp */
    if ((memSizeType) type1 < (memSizeType) type2) {
      return -1;
    } else if ((memSizeType) type1 > (memSizeType) type2) {
      return 1;
    } else {
      return 0;
    } /* if */
  } /* typCmp */



intType typCmpGeneric (const genericType value1, const genericType value2)

  { /* typCmpGeneric */
    return typCmp((const_typeType) ((const_rtlObjectType *) &value1)->value.typeValue,
                  (const_typeType) ((const_rtlObjectType *) &value2)->value.typeValue);
  } /* typCmpGeneric */



typeType typFunc (typeType basic_type)

  {
    typeType result;

  /* typFunc */
    if ((result = get_func_type(NULL, basic_type)) == NULL) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      return result;
    } /* if */
  } /* typFunc */



boolType typIsDerived (typeType any_type)

  { /* typIsDerived */
    return any_type->meta != NULL;
  } /* typIsDerived */



boolType typIsFunc (typeType any_type)

  { /* typIsFunc */
    return any_type->result_type != NULL && !any_type->is_varfunc_type;
  } /* typIsFunc */



boolType typIsVarfunc (typeType any_type)

  { /* typIsVarfunc */
    return any_type->result_type != NULL && any_type->is_varfunc_type;
  } /* typIsVarfunc */



objectType typMatchobj (typeType actual_type)

  { /* typMatchobj */
    return actual_type->match_obj;
  } /* typMatchobj */



typeType typMeta (typeType any_type)

  { /* typMeta */
    if (any_type->meta == NULL) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return any_type->meta;
    } /* if */
  } /* typMeta */



intType typNum (typeType actual_type)

  {
    static rtlHashType type_table = NULL;
    static intType next_free_number = 1;
    intType result;

  /* typNum */
    /* printf("typNum(%lx)\n", actual_type); */
    if (unlikely(actual_type == NULL)) {
      result = 0;
    } else {
      if (unlikely(type_table == NULL)) {
        type_table = hshEmpty();
      } /* if */
      if (unlikely(type_table == NULL)) {
        raise_error(MEMORY_ERROR);
        result = 0;
      } else {
        result = (intType) hshIdxEnterDefault(type_table, (genericType) (memSizeType) actual_type,
            (genericType) next_free_number,
            (intType) (((memSizeType) actual_type) >> 6), (compareType) &genericCmp,
            (createFuncType) &genericCreate, (createFuncType) &genericCreate);
        if (result == next_free_number) {
          next_free_number++;
        } /* if */
      } /* if */
    } /* if */
    /* printf("typNum => %ld\n", result); */
    return result;
  } /* typNum */



typeType typResult (typeType any_type)

  { /* typResult */
    if (any_type->result_type == NULL) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return any_type->result_type;
    } /* if */
  } /* typResult */



striType typStr (typeType type_arg)

  {
    const_cstriType stri;
    errInfoType err_info = OKAY_NO_ERROR;
    striType result;

  /* typStr */
    if (type_arg->name != NULL) {
      stri = id_string2(type_arg->name);
/*  } else if type_arg->result_type != NULL &&
        type_arg->result_type->name != NULL) {
      stri = id_string2(type_arg->result_type->name); */
    } else {
      stri = "*ANONYM_TYPE*";
    } /* if */
    result = cstri8_to_stri(stri, &err_info);
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* typStr */



typeType typVarfunc (typeType basic_type)

  {
    typeType result;

  /* typVarfunc */
    if ((result = get_varfunc_type(NULL, basic_type)) == NULL) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      return result;
    } /* if */
  } /* typVarfunc */
