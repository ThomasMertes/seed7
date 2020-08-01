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
#include "sigutl.h"
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
#include "pcs_drv.h"
#include "pol_drv.h"
#ifdef WITH_SQL
#include "sql_rtl.h"
#endif

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "objutl.h"



#ifdef OUT_OF_ORDER
void isit_bool (objectType argument)

  {
    objectType arg;

  /* isit_bool */
    if (CATEGORY_OF_OBJ(argument) == CONSTENUMOBJECT ||
        CATEGORY_OF_OBJ(argument) == VARENUMOBJECT) {
      arg = argument->value.objValue;
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
#endif



#ifdef WITH_TYPE_CHECK
void isit_enum (objectType argument)

  { /* isit_enum */
    if ((CATEGORY_OF_OBJ(argument) != ENUMLITERALOBJECT &&
        CATEGORY_OF_OBJ(argument) != CONSTENUMOBJECT &&
        CATEGORY_OF_OBJ(argument) != VARENUMOBJECT)) {
      run_error(CONSTENUMOBJECT, argument);
    } /* if */
  } /* isit_enum */



void isit_list (objectType argument)

  { /* isit_list */
    if (CATEGORY_OF_OBJ(argument) != LISTOBJECT &&
        CATEGORY_OF_OBJ(argument) != EXPROBJECT) {
      run_error(LISTOBJECT, argument);
    } /* if */
  } /* isit_list */
#endif



objectType bld_action_temp (actType temp_action)

  {
    register objectType result;

  /* bld_action_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, ACTOBJECT);
      result->value.actValue = temp_action;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_action_temp */



objectType bld_array_temp (arrayType temp_array)

  {
    register objectType result;

  /* bld_array_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, ARRAYOBJECT);
      result->value.arrayValue = temp_array;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_array_temp */



objectType bld_bigint_temp (bigIntType temp_bigint)

  {
    register objectType result;

  /* bld_bigint_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, BIGINTOBJECT);
      result->value.bigIntValue = temp_bigint;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_bigint_temp */



objectType bld_binary_temp (uintType temp_binary)

  {
    register objectType result;

  /* bld_binary_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, INTOBJECT);
      result->value.binaryValue = temp_binary;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_binary_temp */



objectType bld_block_temp (blockType temp_block)

  {
    register objectType result;

  /* bld_block_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, BLOCKOBJECT);
      result->value.blockValue = temp_block;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_block_temp */



objectType bld_bstri_temp (bstriType temp_bstri)

  {
    register objectType result;

  /* bld_bstri_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, BSTRIOBJECT);
      result->value.bstriValue = temp_bstri;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_bstri_temp */



objectType bld_char_temp (charType temp_char)

  {
    register objectType result;

  /* bld_char_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, CHAROBJECT);
      result->value.charValue = temp_char;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_char_temp */



objectType bld_database_temp (databaseType temp_database)

  {
    register objectType result;

  /* bld_database_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, DATABASEOBJECT);
      result->value.databaseValue = temp_database;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_database_temp */



objectType bld_interface_temp (objectType temp_interface)

  {
    register objectType result;

  /* bld_interface_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, INTERFACEOBJECT);
      result->value.objValue = temp_interface;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_interface_temp */



objectType bld_file_temp (fileType temp_file)

  {
    register objectType result;

  /* bld_file_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, FILEOBJECT);
      result->value.fileValue = temp_file;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_file_temp */



objectType bld_float_temp (double temp_float)

  {
    register objectType result;

  /* bld_float_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, FLOATOBJECT);
      result->value.floatValue = (floatType) temp_float;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_float_temp */



objectType bld_hash_temp (hashType temp_hash)

  {
    register objectType result;

  /* bld_hash_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, HASHOBJECT);
      result->value.hashValue = temp_hash;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_hash_temp */



objectType bld_int_temp (intType temp_int)

  {
    register objectType result;

  /* bld_int_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, INTOBJECT);
      result->value.intValue = temp_int;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_int_temp */



objectType bld_list_temp (listType temp_list)

  {
    register objectType result;

  /* bld_list_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, LISTOBJECT);
      result->value.listValue = temp_list;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_list_temp */



objectType bld_param_temp (objectType temp_param)

  {
    register objectType result;

  /* bld_param_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, FORMPARAMOBJECT);
      result->value.objValue = temp_param;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_param_temp */



objectType bld_poll_temp (pollType temp_poll)

  {
    register objectType result;

  /* bld_poll_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, POLLOBJECT);
      result->value.pollValue = temp_poll;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_poll_temp */



objectType bld_prog_temp (progType temp_prog)

  {
    register objectType result;

  /* bld_prog_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, PROGOBJECT);
      result->value.progValue = temp_prog;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_prog_temp */



objectType bld_reference_temp (objectType temp_reference)

  {
    register objectType result;

  /* bld_reference_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, REFOBJECT);
      result->value.objValue = temp_reference;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_reference_temp */



objectType bld_reflist_temp (listType temp_reflist)

  {
    register objectType result;

  /* bld_reflist_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, REFLISTOBJECT);
      result->value.listValue = temp_reflist;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_reflist_temp */



objectType bld_set_temp (setType temp_set)

  {
    register objectType result;

  /* bld_set_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, SETOBJECT);
      result->value.setValue = temp_set;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_set_temp */



objectType bld_socket_temp (socketType temp_socket)

  {
    register objectType result;

  /* bld_socket_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, SOCKETOBJECT);
      result->value.socketValue = temp_socket;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_socket_temp */



objectType bld_sqlstmt_temp (sqlStmtType temp_sqlstmt)

  {
    register objectType result;

  /* bld_sqlstmt_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, SQLSTMTOBJECT);
      result->value.sqlStmtValue = temp_sqlstmt;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_sqlstmt_temp */



objectType bld_stri_temp (striType temp_stri)

  {
    register objectType result;

  /* bld_stri_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, STRIOBJECT);
      result->value.striValue = temp_stri;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_stri_temp */



objectType bld_struct_temp (structType temp_struct)

  {
    register objectType result;

  /* bld_struct_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, STRUCTOBJECT);
      result->value.structValue = temp_struct;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_struct_temp */



objectType bld_type_temp (typeType temp_type)

  {
    register objectType result;

  /* bld_type_temp */
    result = temp_type->match_obj;
    return result;
  } /* bld_type_temp */



objectType bld_win_temp (winType temp_win)

  {
    register objectType result;

  /* bld_win_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, WINOBJECT);
      result->value.winValue = temp_win;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_win_temp */



objectType bld_process_temp (processType temp_process)

  {
    register objectType result;

  /* bld_process_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.property = NULL;
      INIT_CATEGORY_OF_TEMP(result, PROCESSOBJECT);
      result->value.processValue = temp_process;
      return result;
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* bld_process_temp */



void dump_temp_value (objectType object)

  {
    boolType save_interrupt_flag;
    boolType save_fail_flag;
    errInfoType err_info = OKAY_NO_ERROR;

  /* dump_temp_value */
#ifdef TRACE_DUMP_TEMP_VALUE
    if (trace.actions) {
      prot_heapsize();
      prot_cstri(" ");
      prot_cstri("dump_temp_value ");
      printcategory(CATEGORY_OF_OBJ(object));
      prot_cstri(" ");
      prot_int((intType) object);
      prot_cstri(" ");
      trace1(object);
      prot_nl();
    } /* if */
#endif
    save_interrupt_flag = interrupt_flag;
    save_fail_flag = fail_flag;
    set_fail_flag(FALSE);
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
      case DECLAREDOBJECT:
        SET_UNUSED_FLAG(object);
        break;
      case BIGINTOBJECT:
        bigDestr(object->value.bigIntValue);
        SET_UNUSED_FLAG(object);
        break;
      case STRIOBJECT:
        if (object->value.striValue != NULL) {
          FREE_STRI(object->value.striValue, object->value.striValue->size);
        } /* if */
        SET_UNUSED_FLAG(object);
        break;
      case BSTRIOBJECT:
        if (object->value.bstriValue != NULL) {
          FREE_BSTRI(object->value.bstriValue, object->value.bstriValue->size);
        } /* if */
        SET_UNUSED_FLAG(object);
        break;
      case SETOBJECT:
        if (object->value.setValue != NULL) {
          FREE_SET(object->value.setValue,
              (memSizeType) (object->value.setValue->max_position -
              object->value.setValue->min_position + 1));
        } /* if */
        SET_UNUSED_FLAG(object);
        break;
      case ARRAYOBJECT:
        if (object->value.arrayValue != NULL) {
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
        if (object->value.hashValue != NULL) {
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
        if (object->value.structValue != NULL) {
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
        polDestr(object->value.pollValue);
        SET_UNUSED_FLAG(object);
        break;
      case REFLISTOBJECT:
        free_list(object->value.listValue);
        SET_UNUSED_FLAG(object);
        break;
      case LISTOBJECT:
        free_list(object->value.listValue);
        SET_UNUSED_FLAG(object);
        break;
      case BLOCKOBJECT:
        if (object->value.blockValue != NULL) {
          /* printf("free_block: ");
          trace1(object);
          printf("\n"); */
          free_block(object->value.blockValue);
        } /* if */
        SET_UNUSED_FLAG(object);
        break;
      case PROGOBJECT:
        prgDestr(object->value.progValue);
        SET_UNUSED_FLAG(object);
        break;
      case WINOBJECT:
        if (object->value.winValue != NULL) {
          object->value.winValue->usage_count--;
          if (object->value.winValue->usage_count == 0) {
            drwFree(object->value.winValue);
          } /* if */
        } /* if */
        SET_UNUSED_FLAG(object);
        break;
      case PROCESSOBJECT:
        if (object->value.processValue != NULL) {
          object->value.processValue->usage_count--;
          if (object->value.processValue->usage_count == 0) {
            pcsFree(object->value.processValue);
          } /* if */
        } /* if */
        SET_UNUSED_FLAG(object);
        break;
      case INTERFACEOBJECT:
        if (object->value.objValue != NULL) {
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
#ifdef WITH_SQL
      case DATABASEOBJECT:
        sqlDestrDb(object->value.databaseValue);
        SET_UNUSED_FLAG(object);
        break;
      case SQLSTMTOBJECT:
        sqlDestrStmt(object->value.sqlStmtValue);
        SET_UNUSED_FLAG(object);
        break;
#endif
      default:
        if (trace.heapsize) {
          prot_heapsize();
          prot_cstri(" ");
        } /* if */
        prot_cstri("dump_temp_value ");
        /* prot_int((intType) CATEGORY_OF_OBJ(object)); */
        /* prot_int((intType) object);
        printf("%lx", object);
        prot_cstri(" "); */
        trace1(object);
        prot_nl();
        /* CLEAR_TEMP_FLAG(object);
        do_destroy(object, &err_info); */
        break;
    } /* switch */
    interrupt_flag = save_interrupt_flag;
    fail_flag = save_fail_flag;
#ifdef TRACE_DUMP_TEMP_VALUE
    if (trace.actions) {
      prot_heapsize();
      prot_cstri(" end dump_temp_value ");
      prot_nl();
    } /* if */
#endif
  } /* dump_temp_value */



void dump_any_temp (objectType object)

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



void dump_list (listType list)

  {
    register listType list_end;

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
