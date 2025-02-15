/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000, 2011 - 2013  Thomas Mertes           */
/*                2015 - 2017, 2020, 2021  Thomas Mertes            */
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
/*  Module: Interpreter                                             */
/*  File: seed7/src/executl.c                                       */
/*  Changes: 1993, 1994, 2011 - 2013, 2015 - 2017  Thomas Mertes    */
/*           2020, 2021  Thomas Mertes                              */
/*  Content: Initialization operation functions used at runtime.    */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "sigutl.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "listutl.h"
#include "traceutl.h"
#include "objutl.h"
#include "error.h"
#include "doany.h"
#include "exec.h"
#include "match.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "executl.h"



objectType get_create_call_obj (objectType obj, errInfoType *err_info)

  {
    objectRecord expr_object;
    listRecord expr_list[3];
    objectType match_result;
    objectType create_call_obj;
    progType progBackup;

  /* get_create_call_obj */
#ifdef WITH_PROTOCOL
    if (trace.executil) {
      prot_cstri("match - get_create_call_obj: obj= ");
      trace1(obj);
      prot_nl();
    } /* if */
#endif
    create_call_obj = NULL;

    progBackup = prog;
    prog = obj->type_of->owningProg;
    expr_object.type_of = take_type(SYS_EXPR_TYPE);
    expr_object.descriptor.property = NULL;
    expr_object.value.listValue = expr_list;
    INIT_CATEGORY_OF_OBJ(&expr_object, EXPROBJECT);

    expr_list[0].next = &expr_list[1];
    expr_list[1].next = &expr_list[2];
    expr_list[2].next = NULL;
    expr_list[0].obj = obj;
    expr_list[1].obj = SYS_CREA_OBJECT;
    expr_list[2].obj = obj;

    match_result = match_expression(&expr_object);
    if (match_result != NULL) {
      match_result = match_object(match_result);
      if (match_result != NULL) {
        create_call_obj = match_result->value.listValue->obj;
        FREE_L_ELEM(match_result->value.listValue);
        /* FREE_OBJECT(match_result) is not necessary, */
        /* because match_result == &expr_object holds. */
      } /* if */
    } /* if */
    prog = progBackup;

    return create_call_obj;
  } /* get_create_call_obj */



objectType get_destroy_call_obj (objectType obj, errInfoType *err_info)

  {
    objectRecord expr_object;
    listRecord expr_list[2];
    objectType match_result;
    objectType destroy_call_obj;
    progType progBackup;

  /* get_destroy_call_obj */
#ifdef WITH_PROTOCOL
    if (trace.executil) {
      prot_cstri("match - get_destroy_call_obj: obj= ");
      trace1(obj);
      prot_nl();
    } /* if */
#endif
    destroy_call_obj = NULL;

    progBackup = prog;
    prog = obj->type_of->owningProg;
    expr_object.type_of = take_type(SYS_EXPR_TYPE);
    expr_object.descriptor.property = NULL;
    expr_object.value.listValue = expr_list;
    INIT_CATEGORY_OF_OBJ(&expr_object, EXPROBJECT);

    expr_list[0].next = &expr_list[1];
    expr_list[1].next = NULL;
    expr_list[0].obj = obj;
    expr_list[1].obj = SYS_DESTR_OBJECT;

    match_result = match_expression(&expr_object);
    if (match_result != NULL) {
      match_result = match_object(match_result);
      if (match_result != NULL) {
        destroy_call_obj = match_result->value.listValue->obj;
        FREE_L_ELEM(match_result->value.listValue);
        /* FREE_OBJECT(match_result) is not necessary, */
        /* because match_result == &expr_object holds. */
      } /* if */
    } /* if */
    prog = progBackup;

    return destroy_call_obj;
  } /* get_destroy_call_obj */



static void type_create_call_obj (objectType destination,
    objectType source, errInfoType *err_info)

  {
    objectRecord expr_object;
    listRecord expr_list[3];
    objectType match_result;

  /* type_create_call_obj */
#ifdef WITH_PROTOCOL
    if (trace.executil) {
      prot_cstri("match - type_create_call_obj: destination= ");
      trace1(destination);
      prot_nl();
      prot_cstri("source= ");
      trace1(source);
      prot_nl();
    } /* if */
#endif
    expr_object.type_of = take_type(SYS_EXPR_TYPE);
    expr_object.descriptor.property = NULL;
    expr_object.value.listValue = expr_list;
    INIT_CATEGORY_OF_OBJ(&expr_object, EXPROBJECT);

    expr_list[0].next = &expr_list[1];
    expr_list[1].next = &expr_list[2];
    expr_list[2].next = NULL;
    expr_list[0].obj = destination;
    expr_list[1].obj = SYS_CREA_OBJECT;
    expr_list[2].obj = source;

    match_result = match_expression(&expr_object);
    if (match_result != NULL) {
      match_result = match_object(match_result);
      if (match_result != NULL) {
        destination->type_of->create_call_obj =
            match_result->value.listValue->obj;
        FREE_L_ELEM(match_result->value.listValue);
        /* FREE_OBJECT(match_result) is not necessary, */
        /* because match_result == &expr_object holds. */
      } /* if */
    } /* if */
  } /* type_create_call_obj */



static void type_copy_call_obj (objectType destination,
    objectType source, errInfoType *err_info)

  {
    objectRecord expr_object;
    listRecord expr_list[3];
    objectType match_result;

  /* type_copy_call_obj */
#ifdef WITH_PROTOCOL
    if (trace.executil) {
      prot_cstri("match - type_copy_call_obj: destination= ");
      trace1(destination);
      prot_nl();
      prot_cstri("source= ");
      trace1(source);
      prot_nl();
    } /* if */
#endif
    expr_object.type_of = take_type(SYS_EXPR_TYPE);
    expr_object.descriptor.property = NULL;
    expr_object.value.listValue = expr_list;
    INIT_CATEGORY_OF_OBJ(&expr_object, EXPROBJECT);

    expr_list[0].next = &expr_list[1];
    expr_list[1].next = &expr_list[2];
    expr_list[2].next = NULL;
    expr_list[0].obj = destination;
    expr_list[1].obj = SYS_ASSIGN_OBJECT;
    expr_list[2].obj = source;

    match_result = match_expression(&expr_object);
    if (match_result != NULL) {
      match_result = match_object(match_result);
      if (match_result != NULL) {
        destination->type_of->copy_call_obj =
            match_result->value.listValue->obj;
        FREE_L_ELEM(match_result->value.listValue);
        /* FREE_OBJECT(match_result) is not necessary, */
        /* because match_result == &expr_object holds. */
      } /* if */
    } /* if */
  } /* type_copy_call_obj */



static void type_ord_call_obj (objectType any_obj,
    errInfoType *err_info)

  {
    objectRecord expr_object;
    listRecord expr_list[2];
    objectType match_result;

  /* type_ord_call_obj */
#ifdef WITH_PROTOCOL
    if (trace.executil) {
      prot_cstri("match - type_ord_call_obj: any_obj= ");
      trace1(any_obj);
      prot_nl();
    } /* if */
#endif
    expr_object.type_of = take_type(SYS_EXPR_TYPE);
    expr_object.descriptor.property = NULL;
    expr_object.value.listValue = expr_list;
    INIT_CATEGORY_OF_OBJ(&expr_object, EXPROBJECT);

    expr_list[0].next = &expr_list[1];
    expr_list[1].next = NULL;
    expr_list[0].obj = any_obj;
    expr_list[1].obj = SYS_ORD_OBJECT;

    match_result = match_expression(&expr_object);
    if (match_result != NULL) {
      match_result = match_object(match_result);
      if (match_result != NULL) {
        any_obj->type_of->ord_call_obj =
            match_result->value.listValue->obj;
        FREE_L_ELEM(match_result->value.listValue);
        /* FREE_OBJECT(match_result) is not necessary, */
        /* because match_result == &expr_object holds. */
      } /* if */
    } /* if */
  } /* type_ord_call_obj */



static void type_in_call_obj (objectType elem_obj,
    objectType set_obj, errInfoType *err_info)

  {
    objectRecord expr_object;
    listRecord expr_list[3];
    objectType match_result;

  /* type_in_call_obj */
#ifdef WITH_PROTOCOL
    if (trace.executil) {
      prot_cstri("match - type_in_call_obj: elem_obj= ");
      trace1(elem_obj);
      prot_nl();
      prot_cstri("set_obj= ");
      trace1(set_obj);
      prot_nl();
    } /* if */
#endif
    expr_object.type_of = take_type(SYS_EXPR_TYPE);
    expr_object.descriptor.property = NULL;
    expr_object.value.listValue = expr_list;
    INIT_CATEGORY_OF_OBJ(&expr_object, EXPROBJECT);

    expr_list[0].next = &expr_list[1];
    expr_list[1].next = &expr_list[2];
    expr_list[2].next = NULL;
    expr_list[0].obj = elem_obj;
    expr_list[1].obj = SYS_IN_OBJECT;
    expr_list[2].obj = set_obj;

    match_result = match_expression(&expr_object);
    if (match_result != NULL) {
      match_result = match_object(match_result);
      if (match_result != NULL) {
        elem_obj->type_of->in_call_obj =
            match_result->value.listValue->obj;
        FREE_L_ELEM(match_result->value.listValue);
        /* FREE_OBJECT(match_result) is not necessary, */
        /* because match_result == &expr_object holds. */
      } /* if */
    } /* if */
  } /* type_in_call_obj */



static objectType type_value_call_obj (objectType type_obj, errInfoType *err_info)

  {
    objectRecord expr_object;
    listRecord expr_list[3];
    objectType match_result;
    objectType value_call_obj = NULL;

  /* type_value_call_obj */
#ifdef WITH_PROTOCOL
    if (trace.executil) {
      prot_cstri("match - type_value_call_obj: type_obj= ");
      trace1(type_obj);
      prot_nl();
    } /* if */
#endif
    expr_object.type_of = take_type(SYS_EXPR_TYPE);
    expr_object.descriptor.property = NULL;
    expr_object.value.listValue = expr_list;
    INIT_CATEGORY_OF_OBJ(&expr_object, EXPROBJECT);

    expr_list[0].next = &expr_list[1];
    expr_list[1].next = &expr_list[2];
    expr_list[2].next = NULL;
    expr_list[0].obj = type_obj;
    expr_list[1].obj = SYS_DOT_OBJECT;
    expr_list[2].obj = SYS_VALUE_OBJECT;

    match_result = match_expression(&expr_object);
    if (match_result != NULL) {
      match_result = match_object(match_result);
      if (match_result != NULL) {
        value_call_obj = match_result->value.listValue->obj;
        FREE_L_ELEM(match_result->value.listValue);
        /* FREE_OBJECT(match_result) is not necessary, */
        /* because match_result == &expr_object holds. */
      } /* if */
    } /* if */
    logFunction(printf("type_value_call_obj -> " FMT_U_MEM "\n",
                       (memSizeType) value_call_obj););
    return value_call_obj;
  } /* type_value_call_obj */



static void old_do_create (objectType destination, objectType source,
    errInfoType *err_info)

  {
    listRecord crea_expr[3];
    objectType call_result;

  /* old_do_create */
    logFunction(printf("old_do_create ");
                trace1(destination);
                printf("\nas ");
                trace1(source);
                printf("\n"););
#ifdef WITH_PROTOCOL
    if (trace.executil) {
      prot_cstri("match - old_do_create: destination= ");
      trace1(destination);
      prot_nl();
      prot_cstri("source= ");
      trace1(source);
      prot_nl();
    } /* if */
#endif
    crea_expr[0].next = &crea_expr[1];
    crea_expr[1].next = &crea_expr[2];
    crea_expr[2].next = NULL;
    crea_expr[0].obj = destination;
    crea_expr[1].obj = SYS_CREA_OBJECT;
    crea_expr[2].obj = source;
    call_result = exec1(crea_expr);
    if (call_result != SYS_EMPTY_OBJECT) {
      if (fail_flag && trace.exceptions) {
        write_exception_info();
      } /* if */
      set_fail_flag(FALSE);
      if (call_result != NULL) {
        *err_info = getErrInfoFromFailValue(call_result);
      } else {
        *err_info = CREATE_ERROR;
      } /* if */
    } /* if */
    /* printf("free callobjects ");
    trace1(crea_expr[0].obj);
    printf("\n"); */
    /* The function match_expression, called from exec1, may */
    /* allocate CALLOBJECT objects which can be freed now.   */
    free_expression(crea_expr[0].obj);
    logFunction(printf("old_do_create --> err_info=%d\n", *err_info););
  } /* old_do_create */



void do_create (objectType destination, objectType source,
    errInfoType *err_info)

  {
    objectRecord call_object;
    listRecord call_list[4];
    objectType call_result;

  /* do_create */
    logFunction(printf("do_create ");
                trace1(destination);
                printf("\nas ");
                trace1(source);
                printf("\n"););
    if (destination->type_of == source->type_of) {
      if (destination->type_of->destroy_call_obj == NULL) {
        /* prot_cstri("do_create search for destroy(");
        trace1(destination);
        prot_cstri(")");
        prot_nl(); */
        destination->type_of->destroy_call_obj =
            get_destroy_call_obj(destination, err_info);
      } /* if */
      if (destination->type_of->create_call_obj == NULL) {
        /* prot_cstri("do_create ");
        trace1(destination);
        prot_nl();
        prot_cstri("from ");
        trace1(source);
        prot_nl(); */
        type_create_call_obj(destination, source, err_info);
        /* prot_cstri("is ");
        trace1(destination->type_of->create_call_obj);
        prot_nl(); */
      } /* if */
      if (destination->type_of->create_call_obj != NULL) {
        call_object.type_of = NULL;
        call_object.descriptor.property = NULL;
        call_object.value.listValue = call_list;
        INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

        call_list[0].next = &call_list[1];
        call_list[1].next = &call_list[2];
        call_list[2].next = &call_list[3];
        call_list[3].next = NULL;
        call_list[0].obj = destination->type_of->create_call_obj;
        call_list[1].obj = destination;
        call_list[2].obj = SYS_CREA_OBJECT;
        call_list[3].obj = source;

        /* printf("do_create: before exec_call\n");
           fflush(stdout); */
        call_result = exec_call(&call_object);
        /* printf("do_create: after exec_call\n");
           fflush(stdout); */
        if (call_result != SYS_EMPTY_OBJECT) {
          if (fail_flag && trace.exceptions) {
            write_exception_info();
          } /* if */
          set_fail_flag(FALSE);
          if (call_result != NULL) {
            *err_info = getErrInfoFromFailValue(call_result);
          } else {
            *err_info = CREATE_ERROR;
          } /* if */
        } /* if */
      } else {
        *err_info = CREATE_ERROR;
      } /* if */
    } else {
      old_do_create(destination, source, err_info);
    } /* if */
    logFunction(printf("do_create --> err_info=%d\n", *err_info););
  } /* do_create */



void do_destroy (objectType old_obj, errInfoType *err_info)

  {
    objectRecord call_object;
    listRecord call_list[3];
    objectType call_result;

  /* do_destroy */
    if (old_obj->type_of != NULL) {
      if (old_obj->type_of->destroy_call_obj == NULL) {
        /* prot_cstri("do_destroy ");
        trace1(old_obj);
        prot_nl(); */
        old_obj->type_of->destroy_call_obj =
            get_destroy_call_obj(old_obj, err_info);
        /* prot_cstri("is ");
        trace1(old_obj->type_of->destroy_call_obj);
        prot_nl(); */
      } /* if */
      if (old_obj->type_of->destroy_call_obj != NULL) {
        call_object.type_of = NULL;
        call_object.descriptor.property = NULL;
        call_object.value.listValue = call_list;
        INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

        /* prot_cstri("old_obj=[");
        prot_int((intType) old_obj);
        prot_cstri("] ");
        prot_flush();
        trace1(old_obj);
        prot_nl(); */

        call_list[0].next = &call_list[1];
        call_list[1].next = &call_list[2];
        call_list[2].next = NULL;
        call_list[0].obj = old_obj->type_of->destroy_call_obj;
        call_list[1].obj = old_obj;
        call_list[2].obj = SYS_DESTR_OBJECT;

        /* prot_cstri("call_list[0].obj=[");
        prot_int((intType) call_list[0].obj);
        prot_cstri("] ");
        prot_flush();
        trace1(call_list[0].obj);
        prot_nl();
        prot_cstri("call_list[1].obj=[");
        prot_int((intType) call_list[0].obj);
        prot_cstri("] ");
        prot_flush();
        trace1(call_list[1].obj);
        prot_nl();
        prot_cstri("call_list[2].obj=[");
        prot_int((intType) call_list[0].obj);
        prot_cstri("] ");
        prot_flush();
        trace1(call_list[2].obj);
        prot_nl(); */
        /* printf("do_destroy: before exec_call\n");
           fflush(stdout);
        if (TEMP_OBJECT(old_obj)) {
          prot_cstri("is temp ");
          trace1(old_obj);
        } */
        call_result = exec_call(&call_object);
        /* printf("do_destroy: after exec_call\n");
           fflush(stdout); */
        if (call_result != SYS_EMPTY_OBJECT) {
          if (fail_flag && trace.exceptions) {
            write_exception_info();
          } /* if */
          set_fail_flag(FALSE);
          *err_info = DESTROY_ERROR;
        } /* if */
      } else {
        *err_info = DESTROY_ERROR;
      } /* if */
    } else {
      *err_info = DESTROY_ERROR;
    } /* if */
  } /* do_destroy */



void old_do_copy (objectType destination, objectType source,
    errInfoType *err_info)

  {
    listRecord copy_expr[3];

  /* old_do_copy */
    logFunction(printf("old_do_copy\n"););
/*
printobject(destination);
printf(" := ");
printobject(source);
printf("\n");
*/
#ifdef WITH_PROTOCOL
    if (trace.executil) {
      prot_cstri("match - old_do_copy: destination= ");
      trace1(destination);
      prot_nl();
      prot_cstri("source= ");
      trace1(source);
      prot_nl();
    } /* if */
#endif
    copy_expr[0].next = &copy_expr[1];
    copy_expr[1].next = &copy_expr[2];
    copy_expr[2].next = NULL;
    copy_expr[0].obj = destination;
    copy_expr[1].obj = SYS_ASSIGN_OBJECT;
    copy_expr[2].obj = source;
    if (exec1(copy_expr) != SYS_EMPTY_OBJECT) {
      if (fail_flag && trace.exceptions) {
        write_exception_info();
      } /* if */
      set_fail_flag(FALSE);
      *err_info = COPY_ERROR;
    } /* if */
    logFunction(printf("old_do_copy --> err_info=%d\n", *err_info););
  } /* old_do_copy */



static void do_copy (objectType destination, objectType source,
    errInfoType *err_info)

  {
    objectRecord call_object;
    listRecord call_list[4];
    objectType call_result;

  /* do_copy */
    logFunction(printf("do_copy ");
                trace1(destination);
                printf("\nfrom ");
                trace1(source);
                printf("\n"););
    if (destination->type_of == source->type_of) {
      if (destination->type_of->copy_call_obj == NULL) {
        /* prot_cstri("do_copy ");
        trace1(destination);
        prot_nl();
        prot_cstri("from ");
        trace1(source);
        prot_nl(); */
        type_copy_call_obj(destination, source, err_info);
        /* prot_cstri("is ");
        trace1(destination->type_of->copy_call_obj);
        prot_nl(); */
      } /* if */
      if (destination->type_of->copy_call_obj != NULL) {
        call_object.type_of = NULL;
        call_object.descriptor.property = NULL;
        call_object.value.listValue = call_list;
        INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

        call_list[0].next = &call_list[1];
        call_list[1].next = &call_list[2];
        call_list[2].next = &call_list[3];
        call_list[3].next = NULL;
        call_list[0].obj = destination->type_of->copy_call_obj;
        call_list[1].obj = destination;
        call_list[2].obj = SYS_ASSIGN_OBJECT;
        call_list[3].obj = source;

        /* printf("copy_local_object: before exec_call\n");
           fflush(stdout); */
        call_result = exec_call(&call_object);
        /* printf("copy_local_object: after exec_call\n");
           fflush(stdout); */
        if (call_result != SYS_EMPTY_OBJECT) {
          if (fail_flag && trace.exceptions) {
            write_exception_info();
          } /* if */
          set_fail_flag(FALSE);
          *err_info = COPY_ERROR;
        } /* if */
      } else {
        *err_info = COPY_ERROR;
      } /* if */
    } else {
      old_do_copy(destination, source, err_info);
    } /* if */
    logFunction(printf("do_copy --> err_info=%d\n", *err_info););
  } /* do_copy */



intType do_ord (objectType any_obj, errInfoType *err_info)

  {
    categoryType temp_any_obj;
    objectRecord call_object;
    listRecord call_list[3];
    objectType call_result;
    intType result;

  /* do_ord */
    if (any_obj->type_of->ord_call_obj == NULL) {
      /* prot_cstri("do_ord ");
      trace1(any_obj);
      prot_nl(); */
      type_ord_call_obj(any_obj, err_info);
      /* prot_cstri("is ");
      trace1(any_obj->type_of->ord_call_obj);
      prot_nl(); */
    } /* if */
    if (any_obj->type_of->ord_call_obj != NULL) {
      call_object.type_of = NULL;
      call_object.descriptor.property = NULL;
      call_object.value.listValue = call_list;
      INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

      /* prot_cstri("any_obj=[");
      prot_int((intType) any_obj);
      prot_cstri("] ");
      prot_flush();
      trace1(any_obj);
      prot_nl(); */

      call_list[0].next = &call_list[1];
      call_list[1].next = &call_list[2];
      call_list[2].next = NULL;
      call_list[0].obj = any_obj->type_of->ord_call_obj;
      call_list[1].obj = any_obj;
      call_list[2].obj = SYS_ORD_OBJECT;

      /* prot_cstri("call_list[0].obj=[");
      prot_int((intType) call_list[0].obj);
      prot_cstri("] ");
      prot_flush();
      trace1(call_list[0].obj);
      prot_nl();
      prot_cstri("call_list[1].obj=[");
      prot_int((intType) call_list[0].obj);
      prot_cstri("] ");
      prot_flush();
      trace1(call_list[1].obj);
      prot_nl();
      prot_cstri("call_list[2].obj=[");
      prot_int((intType) call_list[0].obj);
      prot_cstri("] ");
      prot_flush();
      trace1(call_list[2].obj);
      prot_nl(); */
      /* printf("do_ord: before exec_call\n");
         fflush(stdout);
      if (TEMP_OBJECT(any_obj)) {
        prot_cstri("is temp ");
        trace1(any_obj);
      } */
      temp_any_obj = (categoryType) TEMP_OBJECT(any_obj);
      CLEAR_TEMP_FLAG(any_obj);

      call_result = exec_call(&call_object);

      SET_ANY_FLAG(any_obj, temp_any_obj);
      /* printf("do_ord: after exec_call\n");
         fflush(stdout); */
      result = take_int(call_result);
      if (TEMP_OBJECT(call_result)) {
        dump_any_temp(call_result);
      } /* if */
    } else {
      result = 0;
      *err_info = IN_ERROR;
    } /* if */
    return result;
  } /* do_ord */



boolType do_in (objectType elem_obj, objectType set_obj,
    errInfoType *err_info)

  {
    categoryType temp_elem_obj;
    categoryType temp_set_obj;
    objectRecord call_object;
    listRecord call_list[4];
    objectType call_result;
    boolType result;

  /* do_in */
    if (elem_obj->type_of->in_call_obj == NULL) {
      /* prot_cstri("do_in ");
      trace1(elem_obj);
      prot_nl(); */
      type_in_call_obj(elem_obj, set_obj, err_info);
      /* prot_cstri("is ");
      trace1(elem_obj->type_of->in_call_obj);
      prot_nl(); */
    } /* if */
    if (elem_obj->type_of->in_call_obj != NULL) {
      call_object.type_of = NULL;
      call_object.descriptor.property = NULL;
      call_object.value.listValue = call_list;
      INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

      /* prot_cstri("elem_obj=[");
      prot_int((intType) elem_obj);
      prot_cstri("] ");
      prot_flush();
      trace1(elem_obj);
      prot_nl(); */

      call_list[0].next = &call_list[1];
      call_list[1].next = &call_list[2];
      call_list[2].next = &call_list[3];
      call_list[3].next = NULL;
      call_list[0].obj = elem_obj->type_of->in_call_obj;
      call_list[1].obj = elem_obj;
      call_list[2].obj = SYS_IN_OBJECT;
      call_list[3].obj = set_obj;

      /* prot_cstri("call_list[0].obj=[");
      prot_int((intType) call_list[0].obj);
      prot_cstri("] ");
      prot_flush();
      trace1(call_list[0].obj);
      prot_nl();
      prot_cstri("call_list[1].obj=[");
      prot_int((intType) call_list[0].obj);
      prot_cstri("] ");
      prot_flush();
      trace1(call_list[1].obj);
      prot_nl();
      prot_cstri("call_list[2].obj=[");
      prot_int((intType) call_list[0].obj);
      prot_cstri("] ");
      prot_flush();
      trace1(call_list[2].obj);
      prot_nl(); */
      /* printf("do_in: before exec_call\n");
         fflush(stdout);
      if (TEMP_OBJECT(elem_obj)) {
        prot_cstri("is temp ");
        trace1(elem_obj);
      } */
      temp_elem_obj = (categoryType) TEMP_OBJECT(elem_obj);
      temp_set_obj = (categoryType) TEMP_OBJECT(set_obj);
      CLEAR_TEMP_FLAG(elem_obj);
      CLEAR_TEMP_FLAG(set_obj);

      call_result = exec_call(&call_object);

      SET_ANY_FLAG(elem_obj, temp_elem_obj);
      SET_ANY_FLAG(set_obj, temp_set_obj);
      /* printf("do_in: after exec_call\n");
         fflush(stdout); */
      result = (boolType) (take_bool(call_result) == SYS_TRUE_OBJECT);
      if (TEMP_OBJECT(call_result)) {
        dump_any_temp(call_result);
      } /* if */
    } else {
      result = FALSE;
      *err_info = IN_ERROR;
    } /* if */
    return result;
  } /* do_in */



objectType getValue (objectType type_obj)

  {
    objectType value_call_obj;
    objectRecord call_object;
    listRecord call_list[4];
    errInfoType err_info = OKAY_NO_ERROR;

  /* getValue */
    if (take_type(type_obj)->value_obj == NULL) {
      value_call_obj = type_value_call_obj(type_obj, &err_info);
      if (value_call_obj != NULL) {
        call_object.type_of = NULL;
        call_object.descriptor.property = NULL;
        call_object.value.listValue = call_list;
        INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

        call_list[0].next = &call_list[1];
        call_list[1].next = &call_list[2];
        call_list[2].next = &call_list[3];
        call_list[3].next = NULL;
        call_list[0].obj = value_call_obj;
        call_list[1].obj = type_obj;
        call_list[2].obj = SYS_DOT_OBJECT;
        call_list[3].obj = SYS_VALUE_OBJECT;

        take_type(type_obj)->value_obj = exec_call(&call_object);
      } /* if */
    } /* if */
    return take_type(type_obj)->value_obj;
  } /* getValue */



objectType param1_call (objectType function_obj, objectType param1)

  {
    objectRecord call_object;
    listRecord call_list[2];
    objectType call_result;

  /* param1_call */
    call_object.type_of = NULL;
    call_object.descriptor.property = NULL;
    call_object.value.listValue = call_list;
    INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

    call_list[0].next = &call_list[1];
    call_list[1].next = NULL;
    call_list[0].obj = function_obj;
    call_list[1].obj = param1;

    call_result = exec_call(&call_object);
    return call_result;
  } /* param1_call */



objectType param2_call (objectType function_obj, objectType param1,
    objectType param2)

  {
    objectRecord call_object;
    listRecord call_list[3];
    objectType call_result;

  /* param2_call */
    call_object.type_of = NULL;
    call_object.descriptor.property = NULL;
    call_object.value.listValue = call_list;
    INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

    call_list[0].next = &call_list[1];
    call_list[1].next = &call_list[2];
    call_list[2].next = NULL;
    call_list[0].obj = function_obj;
    call_list[1].obj = param1;
    call_list[2].obj = param2;

    call_result = exec_call(&call_object);
    return call_result;
  } /* param2_call */



objectType param3_call (objectType function_obj, objectType param1,
    objectType param2, objectType param3)

  {
    objectRecord call_object;
    listRecord call_list[4];
    objectType call_result;

  /* param3_call */
    call_object.type_of = NULL;
    call_object.descriptor.property = NULL;
    call_object.value.listValue = call_list;
    INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

    call_list[0].next = &call_list[1];
    call_list[1].next = &call_list[2];
    call_list[2].next = &call_list[3];
    call_list[3].next = NULL;
    call_list[0].obj = function_obj;
    call_list[1].obj = param1;
    call_list[2].obj = param2;
    call_list[3].obj = param3;

    call_result = exec_call(&call_object);
    return call_result;
  } /* param3_call */



objectType create_return_object (const_locObjType local, objectType init_value,
    errInfoType *err_info)

  {
    objectRecord call_object;
    listRecord call_list[4];
    objectType new_object;
    objectType call_result;

  /* create_return_object */
    logFunction(printf("create_return_object(");
                trace1(local->object);
                printf(", ");
                trace1(init_value);
                printf(")\n"););
    if (local->create_call_obj == NULL) {
      *err_info = CREATE_ERROR;
      new_object = NULL;
    } else if (!ALLOC_OBJECT(new_object)) {
      *err_info = MEMORY_ERROR;
    } else {
      new_object->type_of = init_value->type_of;
      new_object->descriptor.property = NULL;
      if (VAR_OBJECT(init_value)) {
        INIT_CATEGORY_OF_VAR(new_object, DECLAREDOBJECT);
      } else {
        INIT_CATEGORY_OF_OBJ(new_object, DECLAREDOBJECT);
      } /* if */

      call_object.type_of = NULL;
      call_object.descriptor.property = NULL;
      call_object.value.listValue = call_list;
      INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

      call_list[0].next = &call_list[1];
      call_list[1].next = &call_list[2];
      call_list[2].next = &call_list[3];
      call_list[3].next = NULL;
      call_list[0].obj = local->create_call_obj;
      call_list[1].obj = new_object;
      call_list[2].obj = SYS_CREA_OBJECT;
      call_list[3].obj = init_value;

      /* printf("create_return_object: before exec_call\n");
         fflush(stdout); */
      call_result = exec_call(&call_object);
      /* printf("create_return_object: after exec_call\n");
         fflush(stdout); */
      if (call_result != SYS_EMPTY_OBJECT) {
        set_fail_flag(FALSE);
        *err_info = CREATE_ERROR;
      } /* if */
    } /* if */
    logFunction(printf("create_return_object -->\n"););
    return new_object;
  } /* create_return_object */



void create_local_object (const_locObjType local, objectType init_value,
    errInfoType *err_info)

  {
    objectRecord call_object;
    listRecord call_list[4];
    objectType new_object;
    objectType call_result;

  /* create_local_object */
    logFunction(printf("create_local_object(");
                trace1(local->object);
                printf(", ");
                trace1(init_value);
                printf(")\n"););
    if (local->create_call_obj == NULL) {
      *err_info = CREATE_ERROR;
    } else if (!ALLOC_OBJECT(new_object)) {
      *err_info = MEMORY_ERROR;
    } else {
      new_object->type_of = local->object->type_of;
      new_object->descriptor.property = NULL;
      if (VAR_OBJECT(local->object)) {
        INIT_CATEGORY_OF_VAR(new_object, DECLAREDOBJECT);
      } else {
        INIT_CATEGORY_OF_OBJ(new_object, DECLAREDOBJECT);
      } /* if */

      call_object.type_of = NULL;
      call_object.descriptor.property = NULL;
      call_object.value.listValue = call_list;
      INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

      call_list[0].next = &call_list[1];
      call_list[1].next = &call_list[2];
      call_list[2].next = &call_list[3];
      call_list[3].next = NULL;
      call_list[0].obj = local->create_call_obj;
      call_list[1].obj = new_object;
      call_list[2].obj = SYS_CREA_OBJECT;
      call_list[3].obj = init_value;

      /* printf("create_local_object: before exec_call\n");
         fflush(stdout); */
      call_result = exec_call(&call_object);
      /* printf("create_local_object: after exec_call\n");
         fflush(stdout); */
      if (call_result != SYS_EMPTY_OBJECT) {
        set_fail_flag(FALSE);
        *err_info = CREATE_ERROR;
      } /* if */

      local->object->value.objValue = new_object;
    } /* if */
    logFunction(printf("create_local_object -->\n"););
  } /* create_local_object */



void destroy_local_object (const_locObjType local, boolType ignoreError)

  {
    objectRecord call_object;
    listRecord call_list[3];
    objectType call_result;
    boolType okay = TRUE;

  /* destroy_local_object */
    logFunction(printf("destroy_local_object(");
               /* trace1(local->object); */
               printf(", %d)\n", ignoreError););
    if (local->object->value.objValue != NULL) {
      switch (CATEGORY_OF_OBJ(local->object->value.objValue)) {
        case INTOBJECT:
        case CHAROBJECT:
        case FLOATOBJECT:
        case REFOBJECT:
        case ACTOBJECT:
        case CONSTENUMOBJECT:
        case VARENUMOBJECT:
        case ENUMLITERALOBJECT:
          SET_UNUSED_FLAG(local->object->value.objValue);
          break;
        default:
          if (local->destroy_call_obj != NULL) {
            call_object.type_of = NULL;
            call_object.descriptor.property = NULL;
            call_object.value.listValue = call_list;
            INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

            call_list[0].next = &call_list[1];
            call_list[1].next = &call_list[2];
            call_list[2].next = NULL;
            call_list[0].obj = local->destroy_call_obj;
            call_list[1].obj = local->object->value.objValue;
            call_list[2].obj = SYS_DESTR_OBJECT;

            /* printf("destroy_local_object: local->destroy_call_obj ");
            trace1(local->destroy_call_obj);
            printf("\n");
            printf("destroy_local_object: local->object ");
            trace1(local->object);
            printf("\n");
            printf("destroy_local_object: before exec_call ");
            trace1(&call_object);
            printf("\n");
            fflush(stdout); */
            call_result = exec_call(&call_object);
            /* printf("destroy_local_object: after exec_call\n");
               fflush(stdout); */
            if (unlikely(call_result != SYS_EMPTY_OBJECT)) {
              okay = FALSE;
              if (ignoreError) {
                leaveExceptionHandling();
              /* } else if (!fail_flag) {
                 raise_error(DESTROY_ERROR); */
              } /* if */
            } /* if */
          } /* if */
          break;
      } /* switch */
      if (IS_UNUSED(local->object->value.objValue)) {
        FREE_OBJECT(local->object->value.objValue);
      } else if (unlikely(okay &&
          CATEGORY_OF_OBJ(local->object->value.objValue) != STRUCTOBJECT)) {
        printf("loc not dumped: ");
        trace1(local->object);
        printf("\n");
        trace1(local->object->value.objValue);
        printf("\n");
      } /* if */
    } /* if */
    logFunction(printf("destroy_local_object -->\n"););
  } /* destroy_local_object */



void destroy_local_init_value (const_locObjType local, errInfoType *err_info)

  {
    objectRecord call_object;
    listRecord call_list[3];
    objectType call_result;

  /* destroy_local_init_value */
    logFunction(printf("destroy_local_init_value(");
               /* trace1(local->object); */
               printf(")\n"););
    switch (CATEGORY_OF_OBJ(local->init_value)) {
      case INTOBJECT:
      case CHAROBJECT:
      case FLOATOBJECT:
      case REFOBJECT:
      case ACTOBJECT:
      case CONSTENUMOBJECT:
      case VARENUMOBJECT:
      case ENUMLITERALOBJECT:
        break;
      default:
        if (local->destroy_call_obj != NULL) {
          call_object.type_of = NULL;
          call_object.descriptor.property = NULL;
          call_object.value.listValue = call_list;
          INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

          call_list[0].next = &call_list[1];
          call_list[1].next = &call_list[2];
          call_list[2].next = NULL;
          call_list[0].obj = local->destroy_call_obj;
          call_list[1].obj = local->init_value;
          call_list[2].obj = SYS_DESTR_OBJECT;

          /* printf("destroy_local_init_value: local->destroy_call_obj ");
          trace1(local->destroy_call_obj);
          printf("\n");
          printf("destroy_local_init_value: local->object ");
          trace1(local->object);
          printf("\n");
          printf("destroy_local_init_value: before exec_call ");
          trace1(&call_object);
          printf("\n");
          fflush(stdout); */
          call_result = exec_call(&call_object);
          /* printf("destroy_local_init_value: after exec_call\n");
             fflush(stdout); */
          if (call_result != SYS_EMPTY_OBJECT) {
            set_fail_flag(FALSE);
            *err_info = DESTROY_ERROR;
          } /* if */
        } else {
          *err_info = DESTROY_ERROR;
        } /* if */
        break;
    } /* switch */
    logFunction(printf("destroy_local_init_value -->\n"););
  } /* destroy_local_init_value */



static boolType sct_elem_initialisation (typeType dest_type, objectType obj_to, objectType obj_from)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* sct_elem_initialisation */
    memcpy(&obj_to->descriptor, &obj_from->descriptor, sizeof(descriptorUnion));
    INIT_CATEGORY_OF_VAR(obj_to, DECLAREDOBJECT);
    SET_ANY_FLAG(obj_to, HAS_POSINFO(obj_from));
    obj_to->type_of = dest_type;
    do_create(obj_to, obj_from, &err_info);
    return err_info == OKAY_NO_ERROR;
  } /* sct_elem_initialisation */



void destr_struct (objectType old_elem, memSizeType old_size)

  {
    memSizeType position;
    errInfoType err_info = OKAY_NO_ERROR;

  /* destr_struct */
    logFunction(printf("destr_struct(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) old_elem, old_size););
    for (position = old_size; position > 0; position--) {
      switch (CATEGORY_OF_OBJ(old_elem)) {
        case INTOBJECT:
        case CHAROBJECT:
        case FLOATOBJECT:
        case REFOBJECT:
        case ACTOBJECT:
        case CONSTENUMOBJECT:
        case VARENUMOBJECT:
        case ENUMLITERALOBJECT:
          break;
        default:
          do_destroy(old_elem, &err_info);
          break;
      } /* switch */
      old_elem++;
    } /* for */
  } /* destr_struct */



boolType crea_struct (objectType elem_to, objectType elem_from,
    memSizeType new_size)

  {
    memSizeType position = 0;
    boolType okay = TRUE;

  /* crea_struct */
    while (position < new_size && okay) {
      if (!sct_elem_initialisation(elem_from[position].type_of, &elem_to[position], &elem_from[position])) {
        /* If a create fails (mostly no memory) all elements */
        /* created up to this point must be destroyed to recycle */
        /* the memory correctly. */
        destr_struct(elem_to, position);
        okay = FALSE;
      } else {
        position++;
      } /* if */
    } /* for */
    return okay;
  } /* crea_struct */



boolType arr_elem_initialisation (typeType dest_type, objectType obj_to, objectType obj_from)

  {
    boolType objFromIsTemp;
    errInfoType err_info = OKAY_NO_ERROR;

  /* arr_elem_initialisation */
    obj_to->descriptor.property = NULL;
    INIT_CATEGORY_OF_VAR(obj_to, DECLAREDOBJECT);
    SET_ANY_FLAG(obj_to, HAS_POSINFO(obj_from));
    obj_to->type_of = dest_type;
    objFromIsTemp = TEMP_OBJECT(obj_from);
    CLEAR_TEMP_FLAG(obj_from);
    do_create(obj_to, obj_from, &err_info);
    if (objFromIsTemp) {
      SET_TEMP_FLAG(obj_from);
    } /* if */
    return err_info == OKAY_NO_ERROR;
  } /* arr_elem_initialisation */



void destr_array (objectType old_elem, memSizeType old_size)

  {
    memSizeType position;
    errInfoType err_info = OKAY_NO_ERROR;

  /* destr_array */
    for (position = old_size; position > 0; position--) {
      do_destroy(old_elem, &err_info);
      old_elem++;
    } /* for */
  } /* destr_array */



boolType crea_array (objectType elem_to, objectType elem_from,
    memSizeType new_size)

  {
    memSizeType position = 0;
    boolType okay = TRUE;

  /* crea_array */
    while (position < new_size && okay) {
      if (!arr_elem_initialisation(elem_from[position].type_of, &elem_to[position], &elem_from[position])) {
        /* If a create fails (mostly no memory) all elements */
        /* created up to this point must be destroyed to recycle */
        /* the memory correctly. */
        destr_array(elem_to, position);
        okay = FALSE;
      } else {
        position++;
      } /* if */
    } /* for */
    return okay;
  } /* crea_array */



void cpy_array (objectType elem_to, objectType elem_from,
    memSizeType new_size)

  {
    memSizeType position;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cpy_array */
    /* prot_cstri("cpy_array");
       prot_nl(); */
    for (position = new_size; position > 0; position--) {
      do_copy(elem_to, elem_from, &err_info);
      elem_to++;
      elem_from++;
    } /* for */
  } /* cpy_array */
