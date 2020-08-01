/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2002  Thomas Mertes                        */
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
/*  File: seed7/src/sctlib.c                                        */
/*  Changes: 1993, 1994, 2002  Thomas Mertes                        */
/*  Content: All primitive actions for structure types.             */
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
#include "syvarutl.h"
#include "traceutl.h"
#include "listutl.h"
#include "entutl.h"
#include "executl.h"
#include "objutl.h"
#include "runerr.h"
#include "name.h"
#include "match.h"

#undef EXTERN
#define EXTERN
#include "sctlib.h"



#ifdef ANSI_C

objecttype sct_alloc (listtype arguments)
#else

objecttype sct_alloc (arguments)
listtype arguments;
#endif

  {
    objecttype stru_from;
    memsizetype new_size;
    structtype new_stru;
    objecttype result;

  /* sct_alloc */
    stru_from = arg_1(arguments);
    isit_struct(stru_from);
    if (TEMP_OBJECT(stru_from)) {
      CLEAR_TEMP_FLAG(stru_from);
      result = stru_from;
      arg_1(arguments) = NULL;
    } else {
      if (ALLOC_OBJECT(result)) {
        new_size = take_struct(stru_from)->size;
        if (ALLOC_STRUCT(new_stru, new_size)) {
          new_stru->usage_count = 0;
          new_stru->size = new_size;
          if (!crea_array(new_stru->stru,
              take_struct(stru_from)->stru, new_size)) {
            FREE_OBJECT(result);
            FREE_STRUCT(new_stru, new_size);
            return raise_exception(SYS_MEM_EXCEPTION);
          } /* if */
          result->type_of = stru_from->type_of;
          result->descriptor.property = stru_from->descriptor.property;
          INIT_CATEGORY_OF_OBJ(result, stru_from->objcategory);
          result->value.structvalue = new_stru;
        } else {
          FREE_OBJECT(result);
          return raise_exception(SYS_MEM_EXCEPTION);
        } /* if */
      } else {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
    } /* if */
    return bld_reference_temp(result);
  } /* sct_alloc */



#ifdef ANSI_C

objecttype sct_cat (listtype arguments)
#else

objecttype sct_cat (arguments)
listtype arguments;
#endif

  {
    structtype stru1;
    structtype stru2;
    memsizetype stru1_size;
    memsizetype result_size;
    structtype result;

  /* sct_cat */
    isit_struct(arg_1(arguments));
    isit_struct(arg_3(arguments));
    stru1 = take_struct(arg_1(arguments));
    stru2 = take_struct(arg_3(arguments));
    stru1_size = stru1->size;
    result_size = stru1_size + stru2->size;
    if (TEMP_OBJECT(arg_1(arguments))) {
      result = REALLOC_STRUCT(stru1, stru1_size, result_size);
      if (result == NULL) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      COUNT3_STRUCT(stru1_size, result_size);
      result->size = result_size;
      arg_1(arguments)->value.structvalue = NULL;
    } else {
      if (!ALLOC_STRUCT(result, result_size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result->usage_count = 0;
      result->size = result_size;
      if (!crea_array(result->stru, stru1->stru, stru1->size)) {
        FREE_STRUCT(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    if (TEMP_OBJECT(arg_3(arguments))) {
      memcpy(&result->stru[stru1_size], stru2->stru,
          (size_t) (stru2->size * sizeof(objectrecord)));
      FREE_STRUCT(stru2, stru2->size);
      arg_3(arguments)->value.structvalue = NULL;
    } else {
      if (!crea_array(&result->stru[stru1_size], stru2->stru,
          stru2->size)) {
        destr_struct(result->stru, stru1->size);
        FREE_STRUCT(result, result_size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
    } /* if */
    return bld_struct_temp(result);
  } /* sct_cat */



#ifdef ANSI_C

objecttype sct_conv (listtype arguments)
#else

objecttype sct_conv (arguments)
listtype arguments;
#endif

  {
    objecttype stru_arg;
    structtype stru1;
    structtype result_struct;
    objecttype result;

  /* sct_conv */
    stru_arg = arg_3(arguments);
    isit_struct(stru_arg);
    if (TEMP_OBJECT(stru_arg)) {
      result = stru_arg;
      result->type_of = NULL;
      arg_3(arguments) = NULL;
    } else {
      stru1 = take_struct(stru_arg);
      if (!ALLOC_STRUCT(result_struct, stru1->size)) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
      result_struct->usage_count = 0;
      result_struct->size = stru1->size;
      if (!crea_array(result_struct->stru, stru1->stru, stru1->size)) {
        FREE_STRUCT(result_struct, stru1->size);
        return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
      } /* if */
      result = bld_struct_temp(result_struct);
    } /* if */
    return result;
  } /* sct_conv */



#ifdef ANSI_C

objecttype sct_cpy (listtype arguments)
#else

objecttype sct_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype stru_to;
    objecttype stru_from;
    memsizetype new_size;
    structtype new_stru;

  /* sct_cpy */
    stru_to = arg_1(arguments);
    stru_from = arg_3(arguments);
    isit_struct(stru_to);
    isit_struct(stru_from);
    is_variable(stru_to);
    new_stru = take_struct(stru_to);
    if (TEMP_OBJECT(stru_from)) {
      destr_struct(new_stru->stru, new_stru->size);
      FREE_STRUCT(new_stru, new_stru->size);
      stru_to->value.structvalue = take_struct(stru_from);
      stru_from->value.structvalue = NULL;
    } else {
      new_size = take_struct(stru_from)->size;
      if (new_stru->size != new_size) {
        if (!ALLOC_STRUCT(new_stru, new_size)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          new_stru->usage_count = 0;
          new_stru->size = new_size;
          if (!crea_array(new_stru->stru,
              take_struct(stru_from)->stru, new_size)) {
            FREE_STRUCT(new_stru, new_size);
            return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
          } /* if */
          destr_struct(take_struct(stru_to)->stru,
              take_struct(stru_to)->size);
          FREE_STRUCT(take_struct(stru_to),
              take_struct(stru_to)->size);
          stru_to->value.structvalue = new_stru;
        } /* if */
      } else {
        cpy_array(new_stru->stru, take_struct(stru_from)->stru,
            new_size);
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* sct_cpy */



#ifdef ANSI_C

objecttype sct_create (listtype arguments)
#else

objecttype sct_create (arguments)
listtype arguments;
#endif

  {
    objecttype stru_to;
    objecttype stru_from;
    memsizetype new_size;
    structtype new_stru;

  /* sct_create */
    stru_to = arg_1(arguments);
    stru_from = arg_3(arguments);
    SET_CATEGORY_OF_OBJ(stru_to, STRUCTOBJECT);
    isit_struct(stru_from);
    if (TEMP_OBJECT(stru_from)) {
/*
printf("create: pointer assignment\n");
*/
      stru_to->value.structvalue = take_struct(stru_from);
      stru_from->value.structvalue = NULL;
    } else {
      new_size = take_struct(stru_from)->size;
      if (!ALLOC_STRUCT(new_stru, new_size)) {
        stru_to->value.structvalue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } else {
        new_stru->usage_count = 0;
        new_stru->size = new_size;
        stru_to->value.structvalue = new_stru;
        if (!crea_array(new_stru->stru,
            take_struct(stru_from)->stru, new_size)) {
          FREE_STRUCT(new_stru, new_size);
          stru_to->value.structvalue = NULL;
          return raise_with_arguments(SYS_MEM_EXCEPTION, arguments);
        } /* if */
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* sct_create */



#ifdef ANSI_C

objecttype sct_destr (listtype arguments)
#else

objecttype sct_destr (arguments)
listtype arguments;
#endif

  {

    structtype old_struct;

  /* sct_destr */
    isit_struct(arg_1(arguments));
    old_struct = take_struct(arg_1(arguments));
    if (old_struct != NULL) {
      destr_struct(old_struct->stru, old_struct->size);
      FREE_STRUCT(old_struct, old_struct->size);
      arg_1(arguments)->value.structvalue = NULL;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* sct_destr */



#ifdef ANSI_C

objecttype sct_elem (listtype arguments)
#else

objecttype sct_elem (arguments)
listtype arguments;
#endif

  {
    typetype object_type;
    objecttype name_expr;
    objecttype value_expr;
    objecttype value;
    objecttype current_object;
    errinfotype err_info = OKAY_NO_ERROR;
    structtype result;

  /* sct_elem */
    isit_type(arg_2(arguments));
    object_type = take_type(arg_2(arguments));
    name_expr = arg_4(arguments);
    value_expr = arg_6(arguments);
#ifdef TRACE_DCL
    printf("\ndecl var object_type = ");
    trace1(object_type->match_obj);
    printf("\ndecl var name_expr = ");
    trace1(name_expr);
    printf("\ndecl var value_expr = ");
    trace1(value_expr);
    printf("\n");
#endif
    grow_stack(&err_info);
    grow_stack(&err_info);
    if (err_info == OKAY_NO_ERROR) {
      if (CATEGORY_OF_OBJ(value_expr) == EXPROBJECT &&
          value_expr->value.listvalue != NULL &&
          value_expr->value.listvalue->next == NULL) {
        value_expr = value_expr->value.listvalue->obj;
      } /* if */
#ifdef TRACE_DCL
      printf("decl var value_expr = ");
      trace1(value_expr);
      printf("\n");
#endif
      push_stack();
      current_object = entername(prog.declaration_root, name_expr, &err_info);
      pop_stack();
      copy_expression(value_expr, &value, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        current_object->type_of = object_type;
        SET_VAR_FLAG(current_object);
#ifdef TRACE_DCL
        printf("decl var current_object = ");
        trace1(current_object);
        printf("\n");
#endif
        if (CATEGORY_OF_OBJ(value) == EXPROBJECT) {
          if (match_expression(value) != NULL) {
            do_create(current_object, value, &err_info);
            if (err_info != OKAY_NO_ERROR) {
              printf("*** do_create failed\n");
              prot_list(arguments);
            } /* if */
          } else {
            printf("*** match value failed ");
            trace1(value);
            printf("\n");
          } /* if */
        } else {
          do_create(current_object, value, &err_info);
          if (err_info != OKAY_NO_ERROR) {
            printf("*** do_create failed\n");
            prot_list(arguments);
          } /* if */
        } /* if */
        if (!ALLOC_STRUCT(result, 1)) {
          err_info = MEMORY_ERROR;
        } else {
          result->usage_count = 0;
          result->size = 1;
          memcpy(&result->stru[0], current_object, sizeof(objectrecord));
        } /* if */
      } /* if */
      shrink_stack();
      shrink_stack();
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_struct_temp(result);
    } /* if */
  } /* sct_elem */



#ifdef ANSI_C

objecttype sct_empty (listtype arguments)
#else

objecttype sct_empty (arguments)
listtype arguments;
#endif

  {
    structtype result;

  /* sct_empty */
    if (!ALLOC_STRUCT(result, 0)) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    result->usage_count = 0;
    result->size = 0;
    return bld_struct_temp(result);
  } /* sct_empty */



#ifdef ANSI_C

objecttype sct_incl (listtype arguments)
#else

objecttype sct_incl (arguments)
listtype arguments;
#endif

  {
    objecttype stru_variable;
    structtype stru_to;
    objecttype elem;
    memsizetype stru_size;

  /* sct_incl */
    stru_variable = arg_1(arguments);
    isit_struct(stru_variable);
    is_variable(stru_variable);
    stru_to = take_struct(stru_variable);
    isit_reference(arg_2(arguments));
    elem = take_reference(arg_2(arguments));
    stru_size = stru_to->size;
    stru_to = REALLOC_STRUCT(stru_to, stru_size, stru_size + 1);
    if (stru_to == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
    COUNT3_STRUCT(stru_size, stru_size + 1);
    stru_variable->value.structvalue = stru_to;
    memcpy(&stru_to->stru[stru_size], elem, sizeof(objectrecord));
    stru_to->size = stru_size + 1;
    return SYS_EMPTY_OBJECT;
  } /* sct_incl */



#ifdef ANSI_C

objecttype sct_lng (listtype arguments)
#else

objecttype sct_lng (arguments)
listtype arguments;
#endif

  { /* sct_lng */
    isit_struct(arg_1(arguments));
    return bld_int_temp((inttype) take_struct(arg_1(arguments))->size);
  } /* sct_lng */



#ifdef ANSI_C

objecttype sct_refidx (listtype arguments)
#else

objecttype sct_refidx (arguments)
listtype arguments;
#endif

  {
    structtype stru1;
    inttype position;
    objecttype struct_pointer;
    objecttype result;

  /* sct_refidx */
    isit_struct(arg_1(arguments));
    isit_int(arg_3(arguments));
    stru1 = take_struct(arg_1(arguments));
    position = take_int(arg_3(arguments));
    struct_pointer = stru1->stru;
    if (position >= 1 && ((memsizetype) position) <= stru1->size) {
      result = bld_reference_temp(&struct_pointer[position - 1]);
    } else {
      result = raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    return result;
  } /* sct_refidx */



#ifdef ANSI_C

objecttype sct_select (listtype arguments)
#else

objecttype sct_select (arguments)
listtype arguments;
#endif

  {
    structtype stru1;
    objecttype selector;
    objecttype selector_syobject;
    memsizetype position;
    objecttype struct_pointer;
    objecttype result;

  /* sct_select */
    isit_struct(arg_1(arguments));
    stru1 = take_struct(arg_1(arguments));
    selector = arg_3(arguments);
/*
printf("stru1 ");
trace1(arg_1(arguments));
printf("\n");
printf("selector ");
trace1(selector);
printf("\n");
*/
    if (HAS_ENTITY(selector) &&
        GET_ENTITY(selector)->syobject != NULL) {
      selector_syobject = GET_ENTITY(selector)->syobject;
      position = stru1->size;
      struct_pointer = stru1->stru;
      while (position > 0) {
/*
printf("test ");
trace1(struct_pointer);
printf("\n");
*/
        if (HAS_ENTITY(struct_pointer) &&
            GET_ENTITY(struct_pointer)->syobject == selector_syobject) {
          if (TEMP_OBJECT(arg_1(arguments))) {
/*
            printf("sct_select of TEMP_OBJECT\n");
            printf("stru1 ");
            trace1(arg_1(arguments));
            printf("\n");
            printf("selector ");
            trace1(selector);
            printf("\n");
*/
            /* The struct will be destroyed after selecting. */
            /* A copy is necessary here to avoid a crash !!!!! */
            if (!ALLOC_OBJECT(result)) {
              result = raise_exception(SYS_MEM_EXCEPTION);
            } else {
              memcpy(result, struct_pointer, sizeof(objectrecord));
              SET_TEMP_FLAG(result);
              destr_struct(stru1->stru,
                  (memsizetype) (struct_pointer - stru1->stru));
              destr_struct(&struct_pointer[1],
                  (stru1->size - (memsizetype) (struct_pointer - stru1->stru) - 1));
              FREE_STRUCT(stru1, stru1->size);
              arg_1(arguments)->value.structvalue = NULL;
            } /* if */
            return result;
          } else {
            return struct_pointer;
          } /* if */
        } /* if */
        position--;
        struct_pointer++;
      } /* while */
    } /* if */
    return raise_exception(SYS_RNG_EXCEPTION);
  } /* sct_select */
