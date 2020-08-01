/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
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

#undef EXTERN
#define EXTERN
#include "typlib.h"


#define TYPE_TABLE_INCREMENT 127



#ifdef ANSI_C

objecttype typ_addinterface (listtype arguments)
#else

objecttype typ_addinterface (arguments)
listtype arguments;
#endif

  {
    typetype typ1;
    typetype typ2;

  /* typ_addinterface */
    isit_type(arg_1(arguments));
    isit_type(arg_2(arguments));
    typ1 = take_type(arg_1(arguments));
    typ2 = take_type(arg_2(arguments));
    add_interface(typ1, typ2);
    return(SYS_EMPTY_OBJECT);
  } /* typ_addinterface */



#ifdef ANSI_C

objecttype typ_cmp (listtype arguments)
#else

objecttype typ_cmp (arguments)
listtype arguments;
#endif

  {
    memsizetype typ1;
    memsizetype typ2;
    inttype result;

  /* typ_cmp */
    isit_type(arg_1(arguments));
    isit_type(arg_2(arguments));
    typ1 = (memsizetype) take_type(arg_1(arguments));
    typ2 = (memsizetype) take_type(arg_2(arguments));
    if (typ1 < typ2) {
      result = -1;
    } else if (typ1 > typ2) {
      result = 1;
    } else {
      result = 0;
    } /* if */
    return(bld_int_temp(result));
  } /* typ_cmp */



#ifdef ANSI_C

objecttype typ_cpy (listtype arguments)
#else

objecttype typ_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype type_variable;

  /* typ_cpy */
    type_variable = arg_1(arguments);
    isit_type(type_variable);
    is_variable(type_variable);
    isit_type(arg_3(arguments));
    type_variable->value.typevalue = take_type(arg_3(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* typ_cpy */



#ifdef ANSI_C

objecttype typ_create (listtype arguments)
#else

objecttype typ_create (arguments)
listtype arguments;
#endif

  {
    objecttype type_to;
    typetype type_from;

  /* typ_create */
#ifdef TRACE_TYPLIB
    printf("BEGIN typ_create\n");
#endif
    isit_type(arg_3(arguments));
    type_to = arg_1(arguments);
    type_from = take_type(arg_3(arguments));
#ifdef TRACE_typ_create
    printf("\nbefore type assignment\n");
    trace1(type_to);
    printf("\n");
    trace1(type_from->match_obj);
    printf("\n");
#endif
    SET_CATEGORY_OF_OBJ(type_to, TYPEOBJECT);
    type_to->value.typevalue = type_from;
    if (!VAR_OBJECT(type_to)) {
      if (type_from->name == NULL &&
          HAS_ENTITY(type_to) &&
          GET_ENTITY(type_to)->ident != NULL) {
        /* printf("typ_create: Define ");
        trace1(type_to);
        printf("\n"); */
        type_from->name = GET_ENTITY(type_to)->ident;
      } /* if */
    } /* if */
#ifdef TRACE_TYPLIB
    printf("END typ_create\n");
#endif
    return(SYS_EMPTY_OBJECT);
  } /* typ_create */



#ifdef ANSI_C

objecttype typ_destr (listtype arguments)
#else

objecttype typ_destr (arguments)
listtype arguments;
#endif

  {
    typetype old_type;

  /* typ_destr */
#ifdef TRACE_TYPLIB
    printf("BEGIN typ_destr\n");
#endif
    isit_type(arg_1(arguments));
    old_type = take_type(arg_1(arguments));
    /* printf("typ_destr "); trace1(old_type->match_obj); printf("\n"); */
#ifdef TRACE_TYPLIB
    printf("END typ_destr\n");
#endif
    return(SYS_EMPTY_OBJECT);
  } /* typ_destr */



#ifdef ANSI_C

objecttype typ_eq (listtype arguments)
#else

objecttype typ_eq (arguments)
listtype arguments;
#endif

  {
    typetype type1;
    typetype type2;
    objecttype result;

  /* typ_eq */
    isit_type(arg_1(arguments));
    isit_type(arg_3(arguments));
    type1 = take_type(arg_1(arguments));
    type2 = take_type(arg_3(arguments));
    if (type1 == type2) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return(result);
  } /* typ_eq */



#ifdef ANSI_C

objecttype typ_func (listtype arguments)
#else

objecttype typ_func (arguments)
listtype arguments;
#endif

  {
    typetype basic_type;
    typetype result;

  /* typ_func */
#ifdef TRACE_TYPLIB
    printf("BEGIN typ_func\n");
#endif
    isit_type(arg_2(arguments));
    basic_type = take_type(arg_2(arguments));
    if ((result = get_func_type(NULL, basic_type)) == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
    /* printf("typ_func ");
    printobject(result);
    prot_cstri("=");
    prot_int((inttype) result);
    printf("\n"); */
#ifdef TRACE_TYPLIB
    printf("END typ_func\n");
#endif
    return(bld_type_temp(result));
  } /* typ_func */



#ifdef ANSI_C

objecttype typ_gensub (listtype arguments)
#else

objecttype typ_gensub (arguments)
listtype arguments;
#endif

  {
    typetype meta_type;
    typetype result;

  /* typ_gensub */
#ifdef TRACE_TYPLIB
    printf("BEGIN typ_gensub\n");
#endif
    isit_type(arg_1(arguments));
    meta_type = take_type(arg_1(arguments));
    if ((result = new_type(meta_type, NULL)) == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
#ifdef TRACE_TYPLIB
    printf("END typ_gensub\n");
#endif
    return(bld_type_temp(result));
  } /* typ_gensub */



#ifdef ANSI_C

objecttype typ_gentype (listtype arguments)
#else

objecttype typ_gentype (arguments)
listtype arguments;
#endif

  {
    typetype result;

  /* typ_gentype */
#ifdef TRACE_TYPLIB
    printf("BEGIN typ_gentype\n");
#endif
    if ((result = new_type(NULL, NULL)) == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
#ifdef TRACE_TYPLIB
    printf("END typ_gentype\n");
#endif
    return(bld_type_temp(result));
  } /* typ_gentype */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

objecttype typ_getinterfaces (listtype arguments)
#else

objecttype typ_getinterfaces (arguments)
listtype arguments;
#endif

  {
    typetype typ1;

  /* typ_getinterfaces */
    isit_type(arg_1(arguments));
    typ1 = take_type(arg_1(arguments));
    get_interfaces(typ1);
    return(SYS_EMPTY_OBJECT);
  } /* typ_getinterfaces */
#endif



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

objecttype typ_getcreate (listtype arguments)
#else

objecttype typ_getcreate (arguments)
listtype arguments;
#endif

  {
    typetype result;

  /* typ_getcreate */
    isit_type(arg_1(arguments));
    ;
    get_create_call_obj(take_type(arg_1(arguments)), errinfotype *err_info)
    if ((result = new_type(NULL, NULL)) == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
    return(bld_type_temp(result));
  } /* typ_getcreate */
#endif



#ifdef ANSI_C

objecttype typ_hashcode (listtype arguments)
#else

objecttype typ_hashcode (arguments)
listtype arguments;
#endif

  { /* typ_hashcode */
    isit_type(arg_1(arguments));
    return(bld_int_temp((inttype)
        (((memsizetype) take_type(arg_1(arguments))) >> 6)));
  } /* typ_hashcode */



#ifdef ANSI_C

objecttype typ_isdeclared (listtype arguments)
#else

objecttype typ_isdeclared (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* typ_isdeclared */
    if (CATEGORY_OF_OBJ(arg_1(arguments)) == DECLAREDOBJECT) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return(result);
  } /* typ_isdeclared */



#ifdef ANSI_C

objecttype typ_isderived (listtype arguments)
#else

objecttype typ_isderived (arguments)
listtype arguments;
#endif

  {
    typetype any_type;
    objecttype result;

  /* typ_isderived */
    isit_type(arg_1(arguments));
    any_type = take_type(arg_1(arguments));
    if (any_type->meta != NULL) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return(result);
  } /* typ_isderived */



#ifdef ANSI_C

objecttype typ_isforward (listtype arguments)
#else

objecttype typ_isforward (arguments)
listtype arguments;
#endif

  {
    objecttype result;

  /* typ_isforward */
    if (CATEGORY_OF_OBJ(arg_1(arguments)) == FORWARDOBJECT) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return(result);
  } /* typ_isforward */



#ifdef ANSI_C

objecttype typ_isfunc (listtype arguments)
#else

objecttype typ_isfunc (arguments)
listtype arguments;
#endif

  {
    typetype any_type;
    objecttype result;

  /* typ_isfunc */
    isit_type(arg_1(arguments));
    any_type = take_type(arg_1(arguments));
    if (any_type->result_type != NULL && !any_type->is_varfunc_type) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return(result);
  } /* typ_isfunc */



#ifdef ANSI_C

objecttype typ_isvarfunc (listtype arguments)
#else

objecttype typ_isvarfunc (arguments)
listtype arguments;
#endif

  {
    typetype any_type;
    objecttype result;

  /* typ_isvarfunc */
    isit_type(arg_1(arguments));
    any_type = take_type(arg_1(arguments));
    if (any_type->result_type != NULL && any_type->is_varfunc_type) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return(result);
  } /* typ_isvarfunc */



#ifdef ANSI_C

objecttype typ_matchobj (listtype arguments)
#else

objecttype typ_matchobj (arguments)
listtype arguments;
#endif

  {
    typetype actual_type;
    objecttype result;

  /* typ_matchobj */
    isit_type(arg_1(arguments));
    actual_type = take_type(arg_1(arguments));
    result = actual_type->match_obj;
    return(bld_reference_temp(result));
  } /* typ_matchobj */



#ifdef ANSI_C

objecttype typ_meta (listtype arguments)
#else

objecttype typ_meta (arguments)
listtype arguments;
#endif

  {
    typetype any_type;
    typetype result;

  /* typ_meta */
#ifdef TRACE_TYPLIB
    printf("BEGIN typ_meta\n");
#endif
    isit_type(arg_1(arguments));
    any_type = take_type(arg_1(arguments));
    if (any_type->meta == NULL) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } /* if */
    result = any_type->meta;
#ifdef TRACE_TYPLIB
    printf("END typ_meta\n");
#endif
    return(bld_type_temp(result));
  } /* typ_meta */



#ifdef ANSI_C

objecttype typ_ne (listtype arguments)
#else

objecttype typ_ne (arguments)
listtype arguments;
#endif

  {
    typetype type1;
    typetype type2;
    objecttype result;

  /* typ_ne */
    isit_type(arg_1(arguments));
    isit_type(arg_3(arguments));
    type1 = take_type(arg_1(arguments));
    type2 = take_type(arg_3(arguments));
    if (type1 != type2) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return(result);
  } /* typ_ne */



#ifdef ANSI_C

objecttype typ_num (listtype arguments)
#else

objecttype typ_num (arguments)
listtype arguments;
#endif

  {
    static unsigned int table_size = 0;
    static unsigned int table_used = 0;
    static typetype *type_table = NULL;
    register typetype *actual_type_ptr;
    register typetype actual_type;
    unsigned int result;

  /* typ_num */
    isit_type(arg_1(arguments));
    actual_type = take_type(arg_1(arguments));
    if (actual_type == NULL) {
      result = 0;
    } else {
      table_used++;
      if (table_used > table_size) {
        if (type_table == NULL) {
          ALLOC_TABLE(type_table, typetype, table_used + TYPE_TABLE_INCREMENT);
        } else {
          type_table = REALLOC_TABLE(type_table, typetype,
              table_size, table_used + TYPE_TABLE_INCREMENT);
          if (type_table != NULL) {
            COUNT3_TABLE(typetype, table_size, table_used + TYPE_TABLE_INCREMENT);
          } /* if */
        } /* if */
        if (type_table == NULL) {
          table_size = 0;
          table_used = 0;
          return(raise_exception(SYS_MEM_EXCEPTION));
        } /* if */
        table_size = table_used + TYPE_TABLE_INCREMENT;
      } /* if */
      type_table[table_used - 1] = actual_type;
      actual_type_ptr = type_table;
      while (*actual_type_ptr != actual_type) {
        actual_type_ptr++;
      } /* while */
      result = (unsigned int) (actual_type_ptr - type_table);
      if (result != table_used - 1) {
        table_used--;
      } /* if */
      result++;
    } /* if */
    /* printf("typ_num: %lx %lx %lu\n", arg_1(arguments), actual_type, result); */
    return(bld_int_temp((inttype) result));
  } /* typ_num */



#ifdef ANSI_C

objecttype typ_result (listtype arguments)
#else

objecttype typ_result (arguments)
listtype arguments;
#endif

  {
    typetype any_type;
    typetype result;

  /* typ_result */
#ifdef TRACE_TYPLIB
    printf("BEGIN typ_result\n");
#endif
    isit_type(arg_1(arguments));
    any_type = take_type(arg_1(arguments));
    if (any_type->result_type == NULL) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } /* if */
    result = any_type->result_type;
#ifdef TRACE_TYPLIB
    printf("END typ_result\n");
#endif
    return(bld_type_temp(result));
  } /* typ_result */



#ifdef ANSI_C

objecttype typ_str (listtype arguments)
#else

objecttype typ_str (arguments)
listtype arguments;
#endif

  {
    typetype type_arg;
    const_cstritype stri;
    stritype result;

  /* typ_str */
    isit_type(arg_1(arguments));
    type_arg = take_type(arg_1(arguments));
    if (type_arg->name != NULL) {
      stri = id_string(type_arg->name);
/*  } else if type_arg->result_type != NULL &&
        type_arg->result_type->name != NULL) {
      stri = id_string(type_arg->result_type->name); */
    } else {
      stri = "*ANONYM_TYPE*";
    } /* if */
    result = cstri_to_stri(stri);
    if (result == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      return(bld_stri_temp(result));
    } /* if */
  } /* typ_str */



#ifdef ANSI_C

objecttype typ_value (listtype arguments)
#else

objecttype typ_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;

  /* typ_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != TYPEOBJECT) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } else {
      return(obj_arg);
    } /* if */
  } /* typ_value */



#ifdef ANSI_C

objecttype typ_varconv (listtype arguments)
#else

objecttype typ_varconv (arguments)
listtype arguments;
#endif

  { /* typ_varconv */
    is_variable(arg_3(arguments));
    if (TEMP_OBJECT(arg_3(arguments))) {
      prot_cstri("TYP_VARCONV of TEMP_OBJECT ");
      trace1(arg_3(arguments));
      prot_nl();
    } /* if */
    return(arg_3(arguments));
  } /* typ_varconv */



#ifdef ANSI_C

objecttype typ_varfunc (listtype arguments)
#else

objecttype typ_varfunc (arguments)
listtype arguments;
#endif

  {
    typetype basic_type;
    typetype result;

  /* typ_varfunc */
#ifdef TRACE_TYPLIB
    printf("BEGIN typ_varfunc\n");
#endif
    isit_type(arg_2(arguments));
    basic_type = take_type(arg_2(arguments));
    if ((result = get_varfunc_type(NULL, basic_type)) == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
    /* printf("typ_varfunc ");
    printobject(result);
    prot_cstri("=");
    prot_int((inttype) result);
    printf("\n"); */
#ifdef TRACE_TYPLIB
    printf("END typ_varfunc\n");
#endif
    return(bld_type_temp(result));
  } /* typ_varfunc */
