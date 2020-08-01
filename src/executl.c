/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  Module: Interpreter                                             */
/*  File: seed7/src/executl.c                                       */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Initalisation operation procedures used at runtime.    */
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

#undef TRACE_EXECUTL



#ifdef ANSI_C

objecttype get_create_call_obj (objecttype obj, errinfotype *err_info)
#else

objecttype get_create_call_obj (obj, err_info)
objecttype obj;
errinfotype *err_info;
#endif

  {
    objectrecord expr_object;
    listrecord expr_list[3];
    objecttype match_result;
    objecttype create_call_obj;

  /* get_create_call_obj */
#ifdef WITH_PROTOCOL
    if (trace.executil) {
      prot_cstri("match - get_create_call_obj: obj= ");
      trace1(obj);
      prot_nl();
    } /* if */
#endif
    create_call_obj = NULL;

    expr_object.type_of = take_type(SYS_EXPR_TYPE);
    expr_object.descriptor.property = NULL;
    expr_object.value.listvalue = expr_list;
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
        create_call_obj = match_result->value.listvalue->obj;
      } /* if */
    } /* if */

    return(create_call_obj);
  } /* get_create_call_obj */



#ifdef ANSI_C

objecttype get_destroy_call_obj (objecttype obj, errinfotype *err_info)
#else

objecttype get_destroy_call_obj (obj, err_info)
objecttype obj;
errinfotype *err_info;
#endif

  {
    objectrecord expr_object;
    listrecord expr_list[2];
    objecttype match_result;
    objecttype destroy_call_obj;

  /* get_destroy_call_obj */
#ifdef WITH_PROTOCOL
    if (trace.executil) {
      prot_cstri("match - get_destroy_call_obj: obj= ");
      trace1(obj);
      prot_nl();
    } /* if */
#endif
    destroy_call_obj = NULL;

    expr_object.type_of = take_type(SYS_EXPR_TYPE);
    expr_object.descriptor.property = NULL;
    expr_object.value.listvalue = expr_list;
    INIT_CATEGORY_OF_OBJ(&expr_object, EXPROBJECT);

    expr_list[0].next = &expr_list[1];
    expr_list[1].next = NULL;
    expr_list[0].obj = obj;
    expr_list[1].obj = SYS_DESTR_OBJECT;

    match_result = match_expression(&expr_object);
    if (match_result != NULL) {
      match_result = match_object(match_result);
      if (match_result != NULL) {
        destroy_call_obj = match_result->value.listvalue->obj;
      } /* if */
    } /* if */

    return(destroy_call_obj);
  } /* get_destroy_call_obj */



#ifdef ANSI_C

static void type_create_call_obj (objecttype destination,
    objecttype source, errinfotype *err_info)
#else

static void type_create_call_obj (destination, source, err_info)
objecttype destination;
objecttype source;
errinfotype *err_info;
#endif

  {
    objectrecord expr_object;
    listrecord expr_list[3];
    objecttype match_result;

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
    expr_object.value.listvalue = expr_list;
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
            match_result->value.listvalue->obj;
      } /* if */
    } /* if */
  } /* type_create_call_obj */



#ifdef ANSI_C

static void type_copy_call_obj (objecttype destination,
    objecttype source, errinfotype *err_info)
#else

static void type_copy_call_obj (destination, source, err_info)
objecttype destination;
objecttype source;
errinfotype *err_info;
#endif

  {
    objectrecord expr_object;
    listrecord expr_list[3];
    objecttype match_result;

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
    expr_object.value.listvalue = expr_list;
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
            match_result->value.listvalue->obj;
      } /* if */
    } /* if */
  } /* type_copy_call_obj */



#ifdef ANSI_C

static void type_ord_call_obj (objecttype any_obj,
    errinfotype *err_info)
#else

static void type_ord_call_obj (any_obj, err_info)
objecttype any_obj;
errinfotype *err_info;
#endif

  {
    objectrecord expr_object;
    listrecord expr_list[2];
    objecttype match_result;

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
    expr_object.value.listvalue = expr_list;
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
            match_result->value.listvalue->obj;
      } /* if */
    } /* if */
  } /* type_ord_call_obj */



#ifdef ANSI_C

static void type_in_call_obj (objecttype elem_obj,
    objecttype set_obj, errinfotype *err_info)
#else

static void type_in_call_obj (elem_obj, set_obj, err_info)
objecttype elem_obj;
objecttype set_obj;
errinfotype *err_info;
#endif

  {
    objectrecord expr_object;
    listrecord expr_list[3];
    objecttype match_result;

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
    expr_object.value.listvalue = expr_list;
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
            match_result->value.listvalue->obj;
      } /* if */
    } /* if */
  } /* type_in_call_obj */



#ifdef ANSI_C

static void old_do_create (objecttype destination, objecttype source,
    errinfotype *err_info)
#else

static void old_do_create (destination, source, err_info)
objecttype destination;
objecttype source;
errinfotype *err_info;
#endif

  {
    listrecord crea_expr[3];

  /* old_do_create */
#ifdef TRACE_EXECUTL
    printf("BEGIN old_do_create ");
    trace1(destination);
    printf("\nas ");
    trace1(source);
    printf("\n");
#endif
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
    if (exec1(crea_expr) != SYS_EMPTY_OBJECT) {
      fail_flag = FALSE;
      *err_info = CREATE_ERROR;
    } /* if */
#ifdef TRACE_EXECUTL
    printf("END old_do_create\n");
#endif
  } /* old_do_create */



#ifdef ANSI_C

void do_create (objecttype destination, objecttype source,
    errinfotype *err_info)
#else

void do_create (destination, source, err_info)
objecttype destination;
objecttype source;
errinfotype *err_info;
#endif

  {
    objectrecord call_object;
    listrecord call_list[4];
    objecttype call_result;

  /* do_create */
#ifdef TRACE_EXECUTL
    printf("BEGIN do_create ");
    trace1(destination);
    printf("\nas ");
    trace1(source);
    printf("\n");
#endif
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
        call_object.value.listvalue = call_list;
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
          fail_flag = FALSE;
          *err_info = CREATE_ERROR;
        } /* if */
      } else {
        *err_info = CREATE_ERROR;
      } /* if */
    } else {
      old_do_create(destination, source, err_info);
    } /* if */
#ifdef TRACE_EXECUTL
    printf("END do_create\n");
#endif
  } /* do_create */



#ifdef ANSI_C

void do_destroy (objecttype old_obj, errinfotype *err_info)
#else

void do_destroy (old_obj, err_info)
objecttype old_obj;
errinfotype *err_info;
#endif

  {
    objectrecord call_object;
    listrecord call_list[3];
    objecttype call_result;

  /* do_destroy */
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
      call_object.value.listvalue = call_list;
      INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

      /* prot_cstri("old_obj=[");
      prot_int((inttype) old_obj);
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
      prot_int((inttype) call_list[0].obj);
      prot_cstri("] ");
      prot_flush();
      trace1(call_list[0].obj);
      prot_nl();
      prot_cstri("call_list[1].obj=[");
      prot_int((inttype) call_list[0].obj);
      prot_cstri("] ");
      prot_flush();
      trace1(call_list[1].obj);
      prot_nl();
      prot_cstri("call_list[2].obj=[");
      prot_int((inttype) call_list[0].obj);
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
        fail_flag = FALSE;
        *err_info = DESTROY_ERROR;
      } /* if */
    } else {
      *err_info = DESTROY_ERROR;
    } /* if */
  } /* do_destroy */



#ifdef ANSI_C

void old_do_copy (objecttype destination, objecttype source,
    errinfotype *err_info)
#else

void old_do_copy (destination, source, err_info)
objecttype destination;
objecttype source;
errinfotype *err_info;
#endif

  {
    listrecord copy_expr[3];

  /* old_do_copy */
#ifdef TRACE_EXECUTL
    printf("BEGIN do_copy\n");
#endif
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
      fail_flag = FALSE;
      *err_info = CREATE_ERROR;
    } /* if */
#ifdef TRACE_EXECUTL
    printf("END do_copy\n");
#endif
  } /* old_do_copy */



#ifdef ANSI_C

static void do_copy (objecttype destination, objecttype source,
    errinfotype *err_info)
#else

static void do_copy (destination, source, err_info)
objecttype destination;
objecttype source;
errinfotype *err_info;
#endif

  {
    objectrecord call_object;
    listrecord call_list[4];
    objecttype call_result;

  /* do_copy */
#ifdef TRACE_EXECUTL
    printf("BEGIN do_copy ");
    trace1(destination);
    printf("\nfrom ");
    trace1(source);
    printf("\n");
#endif
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
        call_object.value.listvalue = call_list;
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
          fail_flag = FALSE;
          *err_info = COPY_ERROR;
        } /* if */
      } else {
        *err_info = COPY_ERROR;
      } /* if */
    } else {
      old_do_copy(destination, source, err_info);
    } /* if */
#ifdef TRACE_EXECUTL
    printf("END do_copy\n");
#endif
  } /* do_copy */



#ifdef ANSI_C

inttype do_ord (objecttype any_obj, errinfotype *err_info)
#else

inttype do_ord (any_obj, err_info)
objecttype any_obj;
errinfotype *err_info;
#endif

  {
    categorytype temp_any_obj;
    objectrecord call_object;
    listrecord call_list[3];
    objecttype call_result;
    inttype result;

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
      call_object.value.listvalue = call_list;
      INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

      /* prot_cstri("any_obj=[");
      prot_int((inttype) any_obj);
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
      prot_int((inttype) call_list[0].obj);
      prot_cstri("] ");
      prot_flush();
      trace1(call_list[0].obj);
      prot_nl();
      prot_cstri("call_list[1].obj=[");
      prot_int((inttype) call_list[0].obj);
      prot_cstri("] ");
      prot_flush();
      trace1(call_list[1].obj);
      prot_nl();
      prot_cstri("call_list[2].obj=[");
      prot_int((inttype) call_list[0].obj);
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
      temp_any_obj = TEMP_OBJECT(any_obj);
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
    return(result);
  } /* do_ord */



#ifdef ANSI_C

booltype do_in (objecttype elem_obj, objecttype set_obj,
    errinfotype *err_info)
#else

booltype do_in (elem_obj, set_obj, err_info)
objecttype elem_obj;
objecttype set_obj;
errinfotype *err_info;
#endif

  {
    categorytype temp_elem_obj;
    categorytype temp_set_obj;
    objectrecord call_object;
    listrecord call_list[4];
    objecttype call_result;
    booltype result;

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
      call_object.value.listvalue = call_list;
      INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

      /* prot_cstri("elem_obj=[");
      prot_int((inttype) elem_obj);
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
      prot_int((inttype) call_list[0].obj);
      prot_cstri("] ");
      prot_flush();
      trace1(call_list[0].obj);
      prot_nl();
      prot_cstri("call_list[1].obj=[");
      prot_int((inttype) call_list[0].obj);
      prot_cstri("] ");
      prot_flush();
      trace1(call_list[1].obj);
      prot_nl();
      prot_cstri("call_list[2].obj=[");
      prot_int((inttype) call_list[0].obj);
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
      temp_elem_obj = TEMP_OBJECT(elem_obj);
      temp_set_obj = TEMP_OBJECT(set_obj);
      CLEAR_TEMP_FLAG(elem_obj);
      CLEAR_TEMP_FLAG(set_obj);

      call_result = exec_call(&call_object);

      SET_ANY_FLAG(elem_obj, temp_elem_obj);
      SET_ANY_FLAG(set_obj, temp_set_obj);
      /* printf("do_in: after exec_call\n");
         fflush(stdout); */
      result = (booltype) (take_bool(call_result) == SYS_TRUE_OBJECT);
      if (TEMP_OBJECT(call_result)) {
        dump_any_temp(call_result);
      } /* if */
    } else {
      result = FALSE;
      *err_info = IN_ERROR;
    } /* if */
    return(result);
  } /* do_in */



#ifdef ANSI_C

objecttype param1_call (objecttype function_obj, objecttype param1)
#else

objecttype param1_call (function_obj, param1)
objecttype function_obj;
objecttype param1;
#endif

  {
    objectrecord call_object;
    listrecord call_list[2];
    objecttype call_result;

  /* param1_call */
    call_object.type_of = NULL;
    call_object.descriptor.property = NULL;
    call_object.value.listvalue = call_list;
    INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

    call_list[0].next = &call_list[1];
    call_list[1].next = NULL;
    call_list[0].obj = function_obj;
    call_list[1].obj = param1;

    call_result = exec_call(&call_object);
    return(call_result);
  } /* param1_call */



#ifdef ANSI_C

objecttype param2_call (objecttype function_obj, objecttype param1,
    objecttype param2)
#else

objecttype param2_call (function_obj, param1, param2)
objecttype function_obj;
objecttype param1;
objecttype param2;
#endif

  {
    objectrecord call_object;
    listrecord call_list[3];
    objecttype call_result;

  /* param2_call */
    call_object.type_of = NULL;
    call_object.descriptor.property = NULL;
    call_object.value.listvalue = call_list;
    INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

    call_list[0].next = &call_list[1];
    call_list[1].next = &call_list[2];
    call_list[2].next = NULL;
    call_list[0].obj = function_obj;
    call_list[1].obj = param1;
    call_list[2].obj = param2;

    call_result = exec_call(&call_object);
    return(call_result);
  } /* param2_call */



#ifdef ANSI_C

objecttype param3_call (objecttype function_obj, objecttype param1,
    objecttype param2, objecttype param3)
#else

objecttype param3_call (function_obj, param1, param2, param3)
objecttype function_obj;
objecttype param1;
objecttype param2;
objecttype param3;
#endif

  {
    objectrecord call_object;
    listrecord call_list[4];
    objecttype call_result;

  /* param3_call */
    call_object.type_of = NULL;
    call_object.descriptor.property = NULL;
    call_object.value.listvalue = call_list;
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
    return(call_result);
  } /* param3_call */



#ifdef ANSI_C

objecttype create_return_object (const_locobjtype local, objecttype init_value,
    errinfotype *err_info)
#else

objecttype create_return_object (local, init_value, err_info)
locobjtype local;
objecttype init_value;
errinfotype *err_info;
#endif

  {
    objectrecord call_object;
    listrecord call_list[4];
    objecttype new_object;
    objecttype call_result;

  /* create_return_object */
    if (ALLOC_OBJECT(new_object)) {
      new_object->type_of = init_value->type_of;
      new_object->descriptor.property = NULL;
      if (VAR_OBJECT(init_value)) {
        INIT_CATEGORY_OF_VAR(new_object, DECLAREDOBJECT);
      } else {
        INIT_CATEGORY_OF_OBJ(new_object, DECLAREDOBJECT);
      } /* if */

      call_object.type_of = NULL;
      call_object.descriptor.property = NULL;
      call_object.value.listvalue = call_list;
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
        fail_flag = FALSE;
        *err_info = CREATE_ERROR;
      } /* if */
    } else {
      fail_flag = FALSE;
      *err_info = MEMORY_ERROR;
    } /* if */
    return(new_object);
  } /* create_return_object */



#ifdef ANSI_C

void create_local_object (const_locobjtype local, objecttype init_value,
    errinfotype *err_info)
#else

void create_local_object (local, init_value, err_info)
locobjtype local;
objecttype init_value;
errinfotype *err_info;
#endif

  {
    objectrecord call_object;
    listrecord call_list[4];
    objecttype new_object;
    objecttype call_result;

  /* create_local_object */
    if (ALLOC_OBJECT(new_object)) {
      new_object->type_of = local->object->type_of;
      new_object->descriptor.property = NULL;
      if (VAR_OBJECT(local->object)) {
        INIT_CATEGORY_OF_VAR(new_object, DECLAREDOBJECT);
      } else {
        INIT_CATEGORY_OF_OBJ(new_object, DECLAREDOBJECT);
      } /* if */

      call_object.type_of = NULL;
      call_object.descriptor.property = NULL;
      call_object.value.listvalue = call_list;
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
        fail_flag = FALSE;
        *err_info = CREATE_ERROR;
      } /* if */

      local->object->value.objvalue = new_object;
    } else {
      fail_flag = FALSE;
      *err_info = MEMORY_ERROR;
    } /* if */
  } /* create_local_object */



#ifdef ANSI_C

void destroy_local_object (const_locobjtype local, errinfotype *err_info)
#else

void destroy_local_object (local, err_info)
locobjtype local;
errinfotype *err_info;
#endif

  {
    objectrecord call_object;
    listrecord call_list[3];
    objecttype call_result;

  /* destroy_local_object */
    switch (CATEGORY_OF_OBJ(local->object->value.objvalue)) {
      case INTOBJECT:
      case CHAROBJECT:
      case FILEOBJECT:
      case FLOATOBJECT:
      case REFOBJECT:
      case ACTOBJECT:
      case CONSTENUMOBJECT:
      case VARENUMOBJECT:
      case ENUMLITERALOBJECT:
        break;
      default:
        call_object.type_of = NULL;
        call_object.descriptor.property = NULL;
        call_object.value.listvalue = call_list;
        INIT_CATEGORY_OF_OBJ(&call_object, CALLOBJECT);

        call_list[0].next = &call_list[1];
        call_list[1].next = &call_list[2];
        call_list[2].next = NULL;
        call_list[0].obj = local->destroy_call_obj;
        call_list[1].obj = local->object->value.objvalue;
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
        if (call_result != SYS_EMPTY_OBJECT) {
          fail_flag = FALSE;
          *err_info = CREATE_ERROR;
        } /* if */
        break;
    } /* switch */
  } /* destroy_local_object */



#ifdef ANSI_C

void destroy_object_list (const_listtype obj_list)
#else

void destroy_object_list (obj_list)
listtype obj_list;
#endif

  { /* destroy_object_list */
    while (obj_list != NULL) {
      /* prot_cstri("destroy_object ");
      trace1(obj_list->obj);
      prot_nl(); */
      if (CATEGORY_OF_OBJ(obj_list->obj) != ARRAYOBJECT &&
          CATEGORY_OF_OBJ(obj_list->obj) != STRUCTOBJECT &&
          CATEGORY_OF_OBJ(obj_list->obj) != TYPEOBJECT) {
        dump_any_temp(obj_list->obj);
      } /* if */
      obj_list = obj_list->next;
    } /* while */
  } /* destroy_object_list */



#ifdef ANSI_C

booltype any_var_initialisation (objecttype obj_to, objecttype obj_from)
#else

booltype any_var_initialisation (obj_to, obj_from)
objecttype obj_to;
objecttype obj_from;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;

  /* any_var_initialisation */
    obj_to->descriptor.property = obj_from->descriptor.property;
    INIT_CATEGORY_OF_VAR(obj_to, DECLAREDOBJECT);
    obj_to->type_of = obj_from->type_of;
    do_create(obj_to, obj_from, &err_info);
    return(err_info == OKAY_NO_ERROR);
  } /* any_var_initialisation */



#ifdef ANSI_C

void destr_struct (objecttype old_elem, memsizetype old_size)
#else

void destr_struct (old_elem, old_size)
objecttype old_elem;
memsizetype old_size;
#endif

  {
    memsizetype position;
    errinfotype err_info = OKAY_NO_ERROR;

  /* destr_struct */
    for (position = old_size; position > 0; position--) {
      switch (CATEGORY_OF_OBJ(old_elem)) {
        case INTOBJECT:
        case CHAROBJECT:
        case FILEOBJECT:
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



#ifdef ANSI_C

void destr_array (objecttype old_elem, memsizetype old_size)
#else

void destr_array (old_elem, old_size)
objecttype old_elem;
memsizetype old_size;
#endif

  {
    memsizetype position;
    errinfotype err_info = OKAY_NO_ERROR;

  /* destr_array */
    for (position = old_size; position > 0; position--) {
      do_destroy(old_elem, &err_info);
      old_elem++;
    } /* for */
  } /* destr_array */



#ifdef ANSI_C

booltype crea_array (objecttype elem_to, objecttype elem_from,
    memsizetype new_size)
#else

booltype crea_array (elem_to, elem_from, new_size)
objecttype elem_to;
objecttype elem_from;
memsizetype new_size;
#endif

  {
    memsizetype position;
    booltype okay;

  /* crea_array */
    okay = TRUE;
    position = 0;
    while (position < new_size && okay) {
      if (!any_var_initialisation(&elem_to[position], &elem_from[position])) {
        /* When one create fails (mostly no memory) all elements */
        /* created up to this point must be destroyed to recycle */
        /* the memory correct. */
        destr_array(elem_to, position);
        okay = FALSE;
      } else {
        position++;
      } /* if */
    } /* for */
    return(okay);
  } /* crea_array */



#ifdef ANSI_C

void cpy_array (objecttype elem_to, objecttype elem_from,
    memsizetype new_size)
#else

void cpy_array (elem_to, elem_from, new_size)
objecttype elem_to;
objecttype elem_from;
memsizetype new_size;
#endif

  {
    memsizetype position;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cpy_array */
    /* prot_cstri("cpy_array");
       prot_nl(); */
    for (position = new_size; position > 0; position--) {
      do_copy(elem_to, elem_from, &err_info);
      elem_to++;
      elem_from++;
    } /* for */
  } /* cpy_array */
