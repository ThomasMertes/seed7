/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  Module: Runtime                                                 */
/*  File: seed7/src/objutl.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: isit_.. and bld_.. functions for primitive data types. */
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
#include "flistutl.h"
#include "syvarutl.h"
#include "listutl.h"
#include "traceutl.h"
#include "executl.h"
#include "blockutl.h"
#include "identutl.h"
#include "entutl.h"
#include "name.h"
#include "runerr.h"
#include "prg_comp.h"
#include "typ_data.h"
#include "big_drv.h"
#include "drw_drv.h"
#include "pol_drv.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "objutl.h"



#ifdef WITH_TYPE_CHECK
void isit_bool (objecttype argument)

  {
    objecttype arg;

  /* isit_bool */
    if (CATEGORY_OF_OBJ(argument) == CONSTENUMOBJECT ||
        CATEGORY_OF_OBJ(argument) == VARENUMOBJECT) {
      arg = argument->value.objvalue;
    } else {
      arg = argument;
    } /* if */
    if (arg != SYS_TRUE_OBJECT &&
        arg != SYS_FALSE_OBJECT) {
      printf("\n-----------\n");
      trace1(argument);
      printf("\n-----------\n");
      trace1(arg);
      printf("\n-----------\n");
      run_error(ENUMLITERALOBJECT, argument);
    } /* if */
  } /* isit_bool */



void isit_enum (objecttype argument)

  { /* isit_enum */
    if ((CATEGORY_OF_OBJ(argument) != ENUMLITERALOBJECT &&
        CATEGORY_OF_OBJ(argument) != CONSTENUMOBJECT &&
        CATEGORY_OF_OBJ(argument) != VARENUMOBJECT)) {
      run_error(CONSTENUMOBJECT, argument);
    } /* if */
  } /* isit_enum */



void isit_list (objecttype argument)

  { /* isit_list */
    if (CATEGORY_OF_OBJ(argument) != LISTOBJECT &&
        CATEGORY_OF_OBJ(argument) != EXPROBJECT) {
      run_error(LISTOBJECT, argument);
    } /* if */
  } /* isit_list */
#endif



objecttype bld_action_temp (acttype temp_action)

  {
    register objecttype result;

  /* bld_action_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, ACTOBJECT);
      result->value.actvalue = temp_action;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_action_temp */



objecttype bld_array_temp (arraytype temp_array)

  {
    register objecttype result;

  /* bld_array_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, ARRAYOBJECT);
      result->value.arrayvalue = temp_array;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_array_temp */



objecttype bld_bigint_temp (biginttype temp_bigint)

  {
    register objecttype result;

  /* bld_bigint_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, BIGINTOBJECT);
      result->value.bigintvalue = temp_bigint;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_bigint_temp */



objecttype bld_block_temp (blocktype temp_block)

  {
    register objecttype result;

  /* bld_block_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, BLOCKOBJECT);
      result->value.blockvalue = temp_block;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_block_temp */



objecttype bld_bstri_temp (bstritype temp_bstri)

  {
    register objecttype result;

  /* bld_bstri_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, BSTRIOBJECT);
      result->value.bstrivalue = temp_bstri;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_bstri_temp */



objecttype bld_char_temp (chartype temp_char)

  {
    register objecttype result;

  /* bld_char_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, CHAROBJECT);
      result->value.charvalue = temp_char;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_char_temp */



objecttype bld_database_temp (databasetype temp_database)

  {
    register objecttype result;

  /* bld_database_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, DATABASEOBJECT);
      result->value.databasevalue = temp_database;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_database_temp */



objecttype bld_interface_temp (objecttype temp_interface)

  {
    register objecttype result;

  /* bld_interface_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, INTERFACEOBJECT);
      result->value.objvalue = temp_interface;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_interface_temp */



objecttype bld_file_temp (filetype temp_file)

  {
    register objecttype result;

  /* bld_file_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, FILEOBJECT);
      result->value.filevalue = temp_file;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_file_temp */



objecttype bld_float_temp (double temp_float)

  {
    register objecttype result;

  /* bld_float_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, FLOATOBJECT);
      result->value.floatvalue = (floattype) temp_float;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_float_temp */



objecttype bld_hash_temp (hashtype temp_hash)

  {
    register objecttype result;

  /* bld_hash_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, HASHOBJECT);
      result->value.hashvalue = temp_hash;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_hash_temp */



objecttype bld_int_temp (inttype temp_int)

  {
    register objecttype result;

  /* bld_int_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, INTOBJECT);
      result->value.intvalue = temp_int;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_int_temp */



objecttype bld_list_temp (listtype temp_list)

  {
    register objecttype result;

  /* bld_list_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, LISTOBJECT);
      result->value.listvalue = temp_list;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_list_temp */



objecttype bld_param_temp (objecttype temp_param)

  {
    register objecttype result;

  /* bld_param_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, FORMPARAMOBJECT);
      result->value.objvalue = temp_param;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_param_temp */



objecttype bld_poll_temp (polltype temp_poll)

  {
    register objecttype result;

  /* bld_poll_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, POLLOBJECT);
      result->value.pollvalue = temp_poll;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_poll_temp */



objecttype bld_prog_temp (progtype temp_prog)

  {
    register objecttype result;

  /* bld_prog_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, PROGOBJECT);
      result->value.progvalue = temp_prog;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_prog_temp */



objecttype bld_reference_temp (objecttype temp_reference)

  {
    register objecttype result;

  /* bld_reference_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, REFOBJECT);
      result->value.objvalue = temp_reference;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_reference_temp */



objecttype bld_reflist_temp (listtype temp_reflist)

  {
    register objecttype result;

  /* bld_reflist_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, REFLISTOBJECT);
      result->value.listvalue = temp_reflist;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_reflist_temp */



objecttype bld_set_temp (settype temp_set)

  {
    register objecttype result;

  /* bld_set_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, SETOBJECT);
      result->value.setvalue = temp_set;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_set_temp */



objecttype bld_socket_temp (sockettype temp_socket)

  {
    register objecttype result;

  /* bld_socket_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, SOCKETOBJECT);
      result->value.socketvalue = temp_socket;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_socket_temp */



objecttype bld_sqlstmt_temp (sqlstmttype temp_sqlstmt)

  {
    register objecttype result;

  /* bld_sqlstmt_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, SQLSTMTOBJECT);
      result->value.sqlstmtvalue = temp_sqlstmt;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_sqlstmt_temp */



objecttype bld_stri_temp (stritype temp_stri)

  {
    register objecttype result;

  /* bld_stri_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, STRIOBJECT);
      result->value.strivalue = temp_stri;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_stri_temp */



objecttype bld_struct_temp (structtype temp_struct)

  {
    register objecttype result;

  /* bld_struct_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, STRUCTOBJECT);
      result->value.structvalue = temp_struct;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_struct_temp */



objecttype bld_type_temp (typetype temp_type)

  {
    register objecttype result;

  /* bld_type_temp */
    result = temp_type->match_obj;
    return result;
  } /* bld_type_temp */



objecttype bld_win_temp (wintype temp_win)

  {
    register objecttype result;

  /* bld_win_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, WINOBJECT);
      result->value.winvalue = temp_win;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_win_temp */



void dump_temp_value (objecttype object)

  {
    booltype save_fail_flag;
    errinfotype err_info = OKAY_NO_ERROR;

  /* dump_temp_value */
#ifdef TRACE_DUMP_TEMP_VALUE
    if (trace.actions) {
      prot_heapsize();
      prot_cstri(" ");
      prot_cstri("dump_temp_value ");
      printcategory(CATEGORY_OF_OBJ(object));
      prot_cstri(" ");
      prot_int((inttype) object);
      prot_cstri(" ");
      trace1(object);
      prot_nl();
    } /* if */
#endif
    save_fail_flag = fail_flag;
    fail_flag = FALSE;
    switch (CATEGORY_OF_OBJ(object)) {
      case INTOBJECT:
      case CHAROBJECT:
      case FILEOBJECT:
      case SOCKETOBJECT:
      case FLOATOBJECT:
      case REFOBJECT:
      case ACTOBJECT:
      case CONSTENUMOBJECT:
      case VARENUMOBJECT:
      case ENUMLITERALOBJECT:
      case MATCHOBJECT:
      case FWDREFOBJECT:
      case TYPEOBJECT:
        SET_UNUSED_FLAG(object);
        break;
      case BIGINTOBJECT:
        bigDestr(object->value.bigintvalue);
        SET_UNUSED_FLAG(object);
        break;
      case STRIOBJECT:
        if (object->value.strivalue != NULL) {
          FREE_STRI(object->value.strivalue, object->value.strivalue->size);
        } /* if */
        SET_UNUSED_FLAG(object);
        break;
      case BSTRIOBJECT:
        if (object->value.bstrivalue != NULL) {
          FREE_BSTRI(object->value.bstrivalue, object->value.bstrivalue->size);
        } /* if */
        SET_UNUSED_FLAG(object);
        break;
      case SETOBJECT:
        if (object->value.setvalue != NULL) {
          FREE_SET(object->value.setvalue,
              (memsizetype) (object->value.setvalue->max_position -
              object->value.setvalue->min_position + 1));
        } /* if */
        SET_UNUSED_FLAG(object);
        break;
      case ARRAYOBJECT:
        if (object->value.arrayvalue != NULL) {
#ifdef TRACE_DUMP_TEMP_VALUE
          if (trace.actions) {
            prot_cstri("before do_destroy: ");
            trace1(object);
            prot_nl();
          } /* if */
#endif
          CLEAR_TEMP_FLAG(object);
          do_destroy(object, &err_info);
        } else {
          SET_UNUSED_FLAG(object);
        } /* if */
        break;
      case HASHOBJECT:
        if (object->value.hashvalue != NULL) {
#ifdef TRACE_DUMP_TEMP_VALUE
          if (trace.actions) {
            prot_cstri("before do_destroy: ");
            trace1(object);
            prot_nl();
          } /* if */
#endif
          CLEAR_TEMP_FLAG(object);
          do_destroy(object, &err_info);
        } else {
          SET_UNUSED_FLAG(object);
        } /* if */
        break;
      case STRUCTOBJECT:
        if (object->value.structvalue != NULL) {
#ifdef TRACE_DUMP_TEMP_VALUE
          if (trace.actions) {
            prot_cstri("before do_destroy: ");
            trace1(object);
            prot_nl();
          } /* if */
#endif
          CLEAR_TEMP_FLAG(object);
          do_destroy(object, &err_info);
        } else {
          SET_UNUSED_FLAG(object);
        } /* if */
        break;
      case POLLOBJECT:
        polDestr(object->value.pollvalue);
        SET_UNUSED_FLAG(object);
        break;
      case REFLISTOBJECT:
        free_list(object->value.listvalue);
        SET_UNUSED_FLAG(object);
        break;
      case LISTOBJECT:
        free_list(object->value.listvalue);
        SET_UNUSED_FLAG(object);
        break;
      case BLOCKOBJECT:
        if (object->value.blockvalue != NULL) {
          /* printf("free_block: ");
          trace1(object);
          printf("\n"); */
          free_block(object->value.blockvalue);
        } /* if */
        SET_UNUSED_FLAG(object);
        break;
      case PROGOBJECT:
        prgDestr(object->value.progvalue);
        SET_UNUSED_FLAG(object);
        break;
      case WINOBJECT:
        if (object->value.winvalue != NULL) {
          object->value.winvalue->usage_count--;
          if (object->value.winvalue->usage_count == 0) {
            drwFree(object->value.winvalue);
          } /* if */
        } /* if */
        SET_UNUSED_FLAG(object);
        break;
      case INTERFACEOBJECT:
        if (object->value.objvalue != NULL) {
#ifdef TRACE_DUMP_TEMP_VALUE
          if (trace.actions) {
            prot_cstri("before do_destroy: ");
            trace1(object);
            prot_nl();
          } /* if */
#endif
          CLEAR_TEMP_FLAG(object);
          do_destroy(object, &err_info);
        } /* if */
        break;
      default:
        if (trace.heapsize) {
          prot_heapsize();
          prot_cstri(" ");
        } /* if */
        prot_cstri("dump_temp_value ");
        /* prot_int((inttype) CATEGORY_OF_OBJ(object)); */
        /* prot_int((inttype) object);
        printf("%lx", object);
        prot_cstri(" "); */
        trace1(object);
        prot_nl();
        /* CLEAR_TEMP_FLAG(object);
        do_destroy(object, &err_info); */
        break;
    } /* switch */
    fail_flag = save_fail_flag;
#ifdef TRACE_DUMP_TEMP_VALUE
    if (trace.actions) {
      prot_heapsize();
      prot_cstri(" end dump_temp_value ");
      prot_nl();
    } /* if */
#endif
  } /* dump_temp_value */



void dump_any_temp (objecttype object)

  { /* dump_any_temp */
    dump_temp_value(object);
    if (IS_UNUSED(object)) {
      FREE_OBJECT(object);
    } else if (CATEGORY_OF_OBJ(object) != STRUCTOBJECT) {
      printf("not dumped: ");
      trace1(object);
      printf("\n");
    } /* if */
  } /* dump_any_temp */



void dump_list (listtype list)

  {
    register listtype list_end;

  /* dump_list */
    if (list != NULL) {
      list_end = list;
      while (list_end->next != NULL) {
        dump_any_temp(list_end->obj);
        list_end = list_end->next;
      } /* while */
      dump_any_temp(list_end->obj);
      free_list2(list, list_end);
    } /* if */
  } /* dump_list */
