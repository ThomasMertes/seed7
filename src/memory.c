/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Runtime                                                 */
/*  File: seed7/src/memory.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: isit_.. and bld_.. functions for primitive datatypes.  */
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
#include "executl.h"
#include "blockutl.h"
#include "identutl.h"
#include "entutl.h"
#include "name.h"
#include "runerr.h"
#include "drw_drv.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "memory.h"



#ifdef WITH_TYPE_CHECK
#ifdef ANSI_C

void isit_bool (objecttype argument)
#else

void isit_bool (argument)
objecttype argument;
#endif

  {
    objecttype arg;

  /* isit_bool */
    if (CLASS_OF_OBJ(argument) == CONSTENUMOBJECT ||
        CLASS_OF_OBJ(argument) == VARENUMOBJECT) {
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



#ifdef ANSI_C

void isit_enum (objecttype argument)
#else

void isit_enum (argument)
objecttype argument;
#endif

  { /* isit_enum */
    if ((CLASS_OF_OBJ(argument) != ENUMLITERALOBJECT &&
        CLASS_OF_OBJ(argument) != CONSTENUMOBJECT &&
        CLASS_OF_OBJ(argument) != VARENUMOBJECT)) {
      run_error(CONSTENUMOBJECT, argument);
    } /* if */
  } /* isit_enum */



#ifdef ANSI_C

void isit_list (objecttype argument)
#else

void isit_list (argument)
objecttype argument;
#endif

  { /* isit_list */
    if (CLASS_OF_OBJ(argument) != LISTOBJECT &&
        CLASS_OF_OBJ(argument) != EXPROBJECT) {
      run_error(LISTOBJECT, argument);
    } /* if */
  } /* isit_list */
#endif



#ifdef ANSI_C

objecttype bld_action_temp (acttype temp_value)
#else

objecttype bld_action_temp (temp_value)
acttype temp_value;
#endif

  {
    register objecttype result;

  /* bld_action_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, ACTOBJECT);
      result->value.actvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_action_temp */



#ifdef ANSI_C

objecttype bld_array_temp (arraytype temp_value)
#else

objecttype bld_array_temp (temp_value)
arraytype temp_value;
#endif

  {
    register objecttype result;

  /* bld_array_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, ARRAYOBJECT);
      result->value.arrayvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_array_temp */



#ifdef ANSI_C

objecttype bld_bigint_temp (biginttype temp_value)
#else

objecttype bld_bigint_temp (temp_value)
biginttype temp_value;
#endif

  {
    register objecttype result;

  /* bld_bigint_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, BIGINTOBJECT);
      result->value.bigintvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_bigint_temp */



#ifdef ANSI_C

objecttype bld_block_temp (blocktype temp_value)
#else

objecttype bld_block_temp (temp_value)
blocktype temp_value;
#endif

  {
    register objecttype result;

  /* bld_block_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, BLOCKOBJECT);
      result->value.blockvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_block_temp */



#ifdef ANSI_C

objecttype bld_char_temp (chartype temp_value)
#else

objecttype bld_char_temp (temp_value)
chartype temp_value;
#endif

  {
    register objecttype result;

  /* bld_char_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, CHAROBJECT);
      result->value.charvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_char_temp */



#ifdef ANSI_C

objecttype bld_class_temp (objecttype temp_value)
#else

objecttype bld_class_temp (temp_value)
objecttype temp_value;
#endif

  {
    register objecttype result;

  /* bld_class_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, CLASSOBJECT);
      result->value.objvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_class_temp */



#ifdef ANSI_C

objecttype bld_file_temp (filetype temp_value)
#else

objecttype bld_file_temp (temp_value)
filetype temp_value;
#endif

  {
    register objecttype result;

  /* bld_file_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, FILEOBJECT);
      result->value.filevalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_file_temp */



#ifdef ANSI_C

objecttype bld_float_temp (double temp_value)
#else

objecttype bld_float_temp (temp_value)
double temp_value;
#endif

  {
    register objecttype result;

  /* bld_float_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, FLOATOBJECT);
      result->value.floatvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_float_temp */



#ifdef ANSI_C

objecttype bld_hash_temp (hashtype temp_value)
#else

objecttype bld_hash_temp (temp_value)
hashtype temp_value;
#endif

  {
    register objecttype result;

  /* bld_hash_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, HASHOBJECT);
      result->value.hashvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_hash_temp */



#ifdef ANSI_C

objecttype bld_int_temp (inttype temp_value)
#else

objecttype bld_int_temp (temp_value)
inttype temp_value;
#endif

  {
    register objecttype result;

  /* bld_int_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, INTOBJECT);
      result->value.intvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_int_temp */



#ifdef ANSI_C

objecttype bld_list_temp (listtype temp_value)
#else

objecttype bld_list_temp (temp_value)
listtype temp_value;
#endif

  {
    register objecttype result;

  /* bld_list_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, LISTOBJECT);
      result->value.listvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_list_temp */



#ifdef ANSI_C

objecttype bld_param_temp (objecttype temp_value)
#else

objecttype bld_param_temp (temp_value)
objecttype temp_value;
#endif

  {
    register objecttype result;

  /* bld_param_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, FORMPARAMOBJECT);
      result->value.objvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_param_temp */



#ifdef ANSI_C

objecttype bld_prog_temp (progtype temp_value)
#else

objecttype bld_prog_temp (temp_value)
progtype temp_value;
#endif

  {
    register objecttype result;

  /* bld_prog_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, PROGOBJECT);
      result->value.progvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_prog_temp */



#ifdef ANSI_C

objecttype bld_reference_temp (objecttype temp_value)
#else

objecttype bld_reference_temp (temp_value)
objecttype temp_value;
#endif

  {
    register objecttype result;

  /* bld_reference_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, REFOBJECT);
      result->value.objvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_reference_temp */



#ifdef ANSI_C

objecttype bld_reflist_temp (listtype temp_value)
#else

objecttype bld_reflist_temp (temp_value)
listtype temp_value;
#endif

  {
    register objecttype result;

  /* bld_reflist_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, REFLISTOBJECT);
      result->value.listvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_reflist_temp */



#ifdef ANSI_C

objecttype bld_set_temp (settype temp_value)
#else

objecttype bld_set_temp (temp_value)
settype temp_value;
#endif

  {
    register objecttype result;

  /* bld_set_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, SETOBJECT);
      result->value.setvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_set_temp */



#ifdef ANSI_C

objecttype bld_stri_temp (stritype temp_value)
#else

objecttype bld_stri_temp (temp_value)
stritype temp_value;
#endif

  {
    register objecttype result;

  /* bld_stri_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, STRIOBJECT);
      result->value.strivalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_stri_temp */



#ifdef ANSI_C

objecttype bld_struct_temp (structtype temp_value)
#else

objecttype bld_struct_temp (temp_value)
structtype temp_value;
#endif

  {
    register objecttype result;

  /* bld_struct_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, STRUCTOBJECT);
      result->value.structvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_struct_temp */



#ifdef ANSI_C

objecttype bld_type_temp (typetype temp_value)
#else

objecttype bld_type_temp (temp_value)
typetype temp_value;
#endif

  {
    register objecttype result;

  /* bld_type_temp */
    result = temp_value->match_obj;
    return(result);
  } /* bld_type_temp */



#ifdef ANSI_C

objecttype bld_win_temp (wintype temp_value)
#else

objecttype bld_win_temp (temp_value)
wintype temp_value;
#endif

  {
    register objecttype result;

  /* bld_win_temp */
    if (ALLOC_OBJECT(result)) {
      result->type_of = NULL;
      result->descriptor.entity = NULL;
      INIT_CLASS_OF_TEMP(result, WINOBJECT);
      result->value.winvalue = temp_value;
      return(result);
    } else {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } /* if */
  } /* bld_win_temp */



#ifdef ANSI_C

void dump_any_temp (objecttype object)
#else

void dump_any_temp (object)
objecttype object;
#endif

  {
    booltype save_fail_flag;
    errinfotype err_info = OKAY_NO_ERROR;

  /* dump_any_temp */
#ifdef TRACE_DUMP_ANY_TEMP
    if (trace.actions) {
      prot_heapsize();
      prot_cstri(" ");
      prot_cstri("dump_any_temp ");
      printclass(CLASS_OF_OBJ(object));
      prot_cstri(" ");
      prot_int((inttype) object);
      prot_cstri(" ");
      trace1(object);
      prot_nl();
    } /* if */
#endif
    save_fail_flag = fail_flag;
    fail_flag = FALSE;
    switch (CLASS_OF_OBJ(object)) {
      case INTOBJECT:
      case CHAROBJECT:
      case FILEOBJECT:
      case FLOATOBJECT:
      case REFOBJECT:
      case ACTOBJECT:
      case CONSTENUMOBJECT:
      case VARENUMOBJECT:
      case ENUMLITERALOBJECT:
      case FWDREFOBJECT:
        FREE_OBJECT(object);
        break;
      case TYPEOBJECT:
        CLEAR_TEMP_FLAG(object);
        do_destroy(object, &err_info);
        FREE_OBJECT(object);
        break;
      case BIGINTOBJECT:
        if (object->value.bigintvalue != NULL) {
          FREE_BIG(object->value.bigintvalue, object->value.bigintvalue->size);
        } /* if */
        FREE_OBJECT(object);
        break;
      case STRIOBJECT:
        if (object->value.strivalue != NULL) {
          FREE_STRI(object->value.strivalue, object->value.strivalue->size);
        } /* if */
        FREE_OBJECT(object);
        break;
      case SETOBJECT:
        if (object->value.setvalue != NULL) {
          FREE_SET(object->value.setvalue,
              object->value.setvalue->max_position -
              object->value.setvalue->min_position + 1);
        } /* if */
        FREE_OBJECT(object);
        break;
      case ARRAYOBJECT:
        if (object->value.arrayvalue != NULL) {
#ifdef TRACE_DUMP_ANY_TEMP
          if (trace.actions) {
            prot_cstri("before do_destroy: ");
            trace1(object);
            prot_nl();
          } /* if */
#endif
          CLEAR_TEMP_FLAG(object);
          do_destroy(object, &err_info);
        } /* if */
        FREE_OBJECT(object);
        break;
      case HASHOBJECT:
        if (object->value.hashvalue != NULL) {
#ifdef TRACE_DUMP_ANY_TEMP
          if (trace.actions) {
            prot_cstri("before do_destroy: ");
            trace1(object);
            prot_nl();
          } /* if */
#endif
          CLEAR_TEMP_FLAG(object);
          do_destroy(object, &err_info);
        } /* if */
        FREE_OBJECT(object);
        break;
      case STRUCTOBJECT:
        if (object->value.structvalue != NULL) {
#ifdef TRACE_DUMP_ANY_TEMP
          if (trace.actions) {
            prot_cstri("before do_destroy: ");
            trace1(object);
            prot_nl();
          } /* if */
#endif
          CLEAR_TEMP_FLAG(object);
          do_destroy(object, &err_info);
        } /* if */
        FREE_OBJECT(object);
        break;
      case REFLISTOBJECT:
        emptylist(object->value.listvalue);
        FREE_OBJECT(object);
        break;
      case LISTOBJECT:
        emptylist(object->value.listvalue);
        FREE_OBJECT(object);
        break;
      case BLOCKOBJECT:
        if (object->value.blockvalue != NULL) {
          free_block(object->value.blockvalue);
        } /* if */
        FREE_OBJECT(object);
        break;
      case PROGOBJECT:
        if (object->value.progvalue != NULL) {
          object->value.progvalue->usage_count--;
          if (object->value.progvalue->usage_count == 0) {
            close_stack(object->value.progvalue);
            close_declaration_root(object->value.progvalue);
            close_idents(object->value.progvalue);
            FREE_RECORD(object->value.progvalue, progrecord, count.prog);
          } /* if */
        } /* if */
        FREE_OBJECT(object);
        break;
      case WINOBJECT:
        if (object->value.winvalue != NULL) {
          object->value.winvalue->usage_count--;
          if (object->value.winvalue->usage_count == 0) {
            drwFree(object->value.winvalue);
          } /* if */
        } /* if */
        FREE_OBJECT(object);
        break;
      case CLASSOBJECT:
        break;
      default:
        prot_heapsize();
        prot_cstri(" dump_any_temp ");
        trace1(object);
        prot_nl();
        /* CLEAR_TEMP_FLAG(object);
        do_destroy(object, &err_info);
        FREE_OBJECT(object); */
        break;
    } /* switch */
    fail_flag = save_fail_flag;
#ifdef TRACE_DUMP_ANY_TEMP
    if (trace.actions) {
      prot_heapsize();
      prot_cstri(" end dump_any_temp ");
      prot_nl();
    } /* if */
#endif
  } /* dump_any_temp */
