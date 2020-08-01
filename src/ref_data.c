/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  File: seed7/src/ref_data.c                                      */
/*  Changes: 1991-1994, 2004, 2007, 2008, 2010  Thomas Mertes       */
/*  Content: Primitive actions for the reference type.              */
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
#include "datautl.h"
#include "striutl.h"
#include "listutl.h"
#include "identutl.h"
#include "memory.h"
#include "infile.h"    /* from the compiler library */
#include "name.h"      /* from the compiler library */
#include "blockutl.h"  /* from the compiler library */
#include "hsh_rtl.h"
#include "int_rtl.h"
#include "big_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "ref_data.h"


#define TABLE_INCREMENT 127



#ifdef ANSI_C

objecttype refAlloc (objecttype obj_arg)
#else

objecttype refAlloc (obj_arg)
objecttype obj_arg;
#endif

  {
    objecttype created_object;

  /* refAlloc */
    if (ALLOC_OBJECT(created_object)) {
      created_object->type_of = obj_arg->type_of;
      created_object->descriptor.property = obj_arg->descriptor.property;
      INIT_CATEGORY_OF_OBJ(created_object, obj_arg->objcategory);
      created_object->value.objvalue = NULL;
    } else {
      raise_error(MEMORY_ERROR);
    } /* if */
    return created_object;
  } /* refAlloc */



#ifdef ANSI_C

inttype refArrmaxidx (objecttype obj_arg)
#else

inttype refArrmaxidx (obj_arg)
objecttype obj_arg;
#endif

  { /* refArrmaxidx */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != ARRAYOBJECT) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      return take_array(obj_arg)->max_position;
    } /* if */
  } /* refArrmaxidx */



#ifdef ANSI_C

inttype refArrminidx (objecttype obj_arg)
#else

inttype refArrminidx (obj_arg)
objecttype obj_arg;
#endif

  { /* refArrminidx */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != ARRAYOBJECT) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      return take_array(obj_arg)->min_position;
    } /* if */
  } /* refArrminidx */



#ifdef ANSI_C

listtype refArrtolist (objecttype obj_arg)
#else

listtype refArrtolist (obj_arg)
objecttype obj_arg;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* refArrtolist */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != ARRAYOBJECT) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = array_to_list(take_array(obj_arg), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* refArrtolist */



#ifdef ANSI_C

objecttype refBody (objecttype obj_arg)
#else

objecttype refBody (obj_arg)
objecttype obj_arg;
#endif

  {
    objecttype result;

  /* refBody */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != BLOCKOBJECT) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = obj_arg->value.blockvalue->body;
    } /* if */
    return result;
  } /* refBody */



#ifdef ANSI_C

inttype refCategory (objecttype obj_arg)
#else

inttype refCategory (obj_arg)
objecttype obj_arg;
#endif

  {
    inttype result;

  /* refCategory */
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = CATEGORY_OF_OBJ(obj_arg);
    } /* if */
    return result;
  } /* refCategory */



#ifdef ANSI_C

inttype refCatParse (stritype category_name)
#else

inttype refCatParse (category_name)
stritype category_name;
#endif

  {
    cstritype name;
    inttype result;

  /* refCatParse */
    name = cp_to_cstri(category_name);
    if (name == NULL) {
      raise_error(MEMORY_ERROR);
      result = -1;
    } else {
      result = (inttype) category_value(name);
      free_cstri(name, category_name);
      if (result == -1) {
        raise_error(RANGE_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* refCatParse */



#ifdef ANSI_C

stritype refCatStr (inttype aCategory)
#else

stritype refCatStr (aCategory)
inttype aCategory;
#endif

  {
    stritype result;

  /* refCatStr */
    result = cstri_to_stri(category_cstri((objectcategory) aCategory));
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* refCatStr */



#ifdef ANSI_C

inttype refCmpGeneric (const rtlGenerictype ref1, const rtlGenerictype ref2)
#else

inttype refCmpGeneric (ref1, ref2)
rtlGenerictype ref1;
rtlGenerictype ref2;
#endif

  { /* refCmpGeneric */
    if (ref1 < ref2) {
      return -1;
    } else if (ref1 > ref2) {
      return 1;
    } else {
      return 0;
    } /* if */
  } /* refCmpGeneric */



#ifdef ANSI_C

void refCpy (objecttype *dest, objecttype source)
#else

void refCpy (dest, source)
objecttype *dest;
objecttype source;
#endif

  { /* refCpy */
    *dest = source;
  } /* refCpy */



#ifdef ANSI_C

rtlGenerictype genericCreate (rtlGenerictype source)
#else

rtlGenerictype genericCreate (source)
rtlGenerictype source;
#endif

  { /* genericCreate */
    return source;
  } /* genericCreate */



#ifdef ANSI_C

const_stritype refFile (objecttype obj_arg1)
#else

stritype refFile (obj_arg1)
objecttype obj_arg1;
#endif

  {
    filenumtype file_number;
    const_stritype result;

  /* refFile */
    if (HAS_POSINFO(obj_arg1)) {
      file_number = GET_FILE_NUM(obj_arg1);
    } else if (HAS_PROPERTY(obj_arg1)) {
      /* trace1(obj_arg1);
      printf(" %u %u %u\n",
          obj_arg1->descriptor.property->file_number,
          obj_arg1->descriptor.property->line,
          obj_arg1->descriptor.property->syNumberInLine); */
      file_number = obj_arg1->descriptor.property->file_number;
    } else {
      file_number = 0;
    } /* if */
    result = file_name(file_number);
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* refFile */



#ifdef ANSI_C

listtype refHshDataToList (objecttype obj_arg)
#else

listtype refHshDataToList (obj_arg)
objecttype obj_arg;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* refHshDataToList */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != HASHOBJECT) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = hash_data_to_list(take_hash(obj_arg), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* refHshDataToList */



#ifdef ANSI_C

listtype refHshKeyToList (objecttype obj_arg)
#else

listtype refHshKeyToList (obj_arg)
objecttype obj_arg;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* refHshKeyToList */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != HASHOBJECT) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = hash_key_to_list(take_hash(obj_arg), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* refHshKeyToList */



#ifdef ANSI_C

booltype refIsVar (objecttype obj_arg)
#else

booltype refIsVar (obj_arg)
objecttype obj_arg;
#endif

  { /* refIsVar */
    /* printf("refIsvar(%lu)\n", obj_arg); */
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
      return FALSE;
    } else {
      return VAR_OBJECT(obj_arg) ? TRUE : FALSE;
    } /* if */
  } /* refIsVar */



#ifdef ANSI_C

objecttype refItftosct (objecttype obj_arg)
#else

objecttype refItftosct (obj_arg)
objecttype obj_arg;
#endif

  {
    objecttype result;

  /* refItftosct */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != INTERFACEOBJECT ||
        take_reference(obj_arg) == NULL) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = take_reference(obj_arg);
    } /* if */
    return result;
  } /* refItftosct */



#ifdef ANSI_C

inttype refLine (objecttype obj_arg)
#else

inttype refLine (obj_arg)
objecttype obj_arg;
#endif

  {
    inttype result;

  /* refLine */
    if (HAS_POSINFO(obj_arg)) {
      /* GET_LINE_NUM delivers an unsigned integer in the range 0 to 1048575 */
      result = (inttype) GET_LINE_NUM(obj_arg);
    } else if (HAS_PROPERTY(obj_arg)) {
      /* trace1(obj_arg);
      printf(" %s %u %u\n",
          file_name_ustri(obj_arg->descriptor.property->file_number),
          obj_arg->descriptor.property->line,
          obj_arg->descriptor.property->syNumberInLine); */
      /* Cast to inttype: The line is probably in the range 0 to 2147483647 */
      result = (inttype) obj_arg->descriptor.property->line;
    } else {
      result = 0;
    } /* if */
    return result;
  } /* refLine */



#ifdef ANSI_C

listtype refLocalConsts (objecttype obj_arg)
#else

listtype refLocalConsts (obj_arg)
objecttype obj_arg;
#endif

  {
    listtype local_elem;
    listtype *list_insert_place;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* refLocalConsts */
    result = NULL;
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != BLOCKOBJECT) {
      raise_error(RANGE_ERROR);
    } else {
      list_insert_place = &result;
      local_elem = obj_arg->value.blockvalue->local_consts;
      while (local_elem != NULL) {
        list_insert_place = append_element_to_list(list_insert_place,
            local_elem->obj, &err_info);
        local_elem = local_elem->next;
      } /* while */
      if (err_info != OKAY_NO_ERROR) {
        emptylist(result);
        result = NULL;
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* refLocalConsts */



#ifdef ANSI_C

listtype refLocalVars (objecttype obj_arg)
#else

listtype refLocalVars (obj_arg)
objecttype obj_arg;
#endif

  {
    loclisttype local_elem;
    listtype *list_insert_place;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* refLocalVars */
    result = NULL;
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != BLOCKOBJECT) {
      raise_error(RANGE_ERROR);
    } else {
      list_insert_place = &result;
      local_elem = obj_arg->value.blockvalue->local_vars;
      while (local_elem != NULL) {
        list_insert_place = append_element_to_list(list_insert_place,
            local_elem->local.object, &err_info);
        local_elem = local_elem->next;
      } /* while */
      if (err_info != OKAY_NO_ERROR) {
        emptylist(result);
        result = NULL;
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* refLocalVars */



#ifdef ANSI_C

inttype refNum (objecttype obj_arg)
#else

inttype refNum (obj_arg)
objecttype obj_arg;
#endif

  {
    static rtlHashtype obj_table = NULL;
    static inttype next_free_number = 1;
    inttype result;

  /* refNum */
    /* printf("refNum(%lX)\n", obj_arg); */
    if (obj_arg == NULL) {
      result = 0;
    } else {
      if (obj_table == NULL) {
        obj_table = hshEmpty();
      } /* if */
      if (obj_table == NULL) {
        raise_error(MEMORY_ERROR);
        result = 0;
      } else {
        result = (inttype) hshIdxWithDefault(obj_table, (rtlGenerictype) (memsizetype) obj_arg,
            (rtlGenerictype) next_free_number,
            (inttype) (((memsizetype) obj_arg) >> 6), (comparetype) &refCmpGeneric,
            (createfunctype) &genericCreate, (createfunctype) &genericCreate);
        if (result == next_free_number) {
          next_free_number++;
        } /* if */
      } /* if */
    } /* if */
    /* printf("refNum => %ld\n", result); */
    return result;
  } /* refNum */



#ifdef ANSI_C

listtype refParams (objecttype obj_arg)
#else

listtype refParams (obj_arg)
objecttype obj_arg;
#endif

  {
    loclisttype local_elem;
    listtype *list_insert_place;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* refParams */
    result = NULL;
    if (CATEGORY_OF_OBJ(obj_arg) == BLOCKOBJECT) {
      list_insert_place = &result;
      local_elem = obj_arg->value.blockvalue->params;
      while (local_elem != NULL) {
        list_insert_place = append_element_to_list(list_insert_place,
            local_elem->local.object, &err_info);
        local_elem = local_elem->next;
      } /* while */
    } else {
      result = create_parameter_list(GET_ENTITY(obj_arg)->name_list,
          &err_info);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      emptylist(result);
      result = NULL;
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* refParams */



#ifdef ANSI_C

objecttype refResini (objecttype obj_arg)
#else

objecttype refResini (obj_arg)
objecttype obj_arg;
#endif

  {
    objecttype result;

  /* refResini */
    if (CATEGORY_OF_OBJ(obj_arg) == BLOCKOBJECT) {
      result = obj_arg->value.blockvalue->result.init_value;
    } else {
      result = NULL;
    } /* if */
    return result;
  } /* refResini */



#ifdef ANSI_C

objecttype refResult (objecttype obj_arg)
#else

objecttype refResult (obj_arg)
objecttype obj_arg;
#endif

  {
    objecttype result;

  /* refResult */
    if (CATEGORY_OF_OBJ(obj_arg) == BLOCKOBJECT) {
      result = obj_arg->value.blockvalue->result.object;
    } else {
      result = NULL;
    } /* if */
    return result;
  } /* refResult */



#ifdef ANSI_C

listtype refScttolist (objecttype obj_arg)
#else

listtype refScttolist (obj_arg)
objecttype obj_arg;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* refScttolist */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != STRUCTOBJECT) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = struct_to_list(take_struct(obj_arg), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* refScttolist */



#ifdef ANSI_C

void refSetCategory (objecttype obj_arg, inttype aCategory)
#else

void refSetCategory (obj_arg, aCategory)
objecttype obj_arg;
inttype aCategory;
#endif

  { /* refSetCategory */
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      SET_CATEGORY_OF_OBJ(obj_arg, aCategory);
    } /* if */
  } /* refSetCategory */



#ifdef ANSI_C

void refSetParams (objecttype obj_arg, listtype params)
#else

void refSetParams (obj_arg, params)
objecttype obj_arg;
listtype params;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;

  /* refSetParams */
    if (CATEGORY_OF_OBJ(obj_arg) == BLOCKOBJECT) {
      /*FIXME not ok since parameter names are important here !!! */
      /* Comment copied from dcllib.c */
      obj_arg->value.blockvalue->params =
          get_param_list(params, &err_info);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      raise_error(MEMORY_ERROR);
    } /* if */
  } /* refSetParams */



#ifdef ANSI_C

void refSetType (objecttype obj_arg, typetype any_type)
#else

void refSetType (obj_arg, any_type)
objecttype obj_arg;
typetype any_type;
#endif

  { /* refSetType */
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      obj_arg->type_of = any_type;
    } /* if */
  } /* refSetType */



#ifdef ANSI_C

void refSetVar (objecttype obj_arg, booltype var_flag)
#else

void refSetVar (obj_arg, var_flag)
objecttype obj_arg;
booltype var_flag;
#endif

  { /* refSetVar */
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      if (var_flag) {
        SET_VAR_FLAG(obj_arg);
      } else {
        CLEAR_VAR_FLAG(obj_arg);
      } /* if */
    } /* if */
  } /* refSetVar */



#ifdef ANSI_C

stritype refStr (objecttype obj_arg)
#else

stritype refStr (obj_arg)
objecttype obj_arg;
#endif

  {
    const_cstritype stri;
    memsizetype buffer_len;
    char *buffer;
    listtype name_elem;
    objecttype param_obj;
    stritype result;

  /* refStr */
    buffer = NULL;
    if (obj_arg == NULL) {
      stri = " *NULL_OBJECT* ";
    } else if (HAS_POSINFO(obj_arg)) {
      stri = (const_cstritype) file_name_ustri(GET_FILE_NUM(obj_arg));
      buffer_len = (memsizetype) strlen(stri) + 32;
      if (!ALLOC_CSTRI(buffer, buffer_len)) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        sprintf(buffer, "%s(%u)", stri, GET_LINE_NUM(obj_arg));
        stri = buffer;
      } /* if */
    } else if (!HAS_ENTITY(obj_arg)) {
      stri = " *NULL_ENTITY_OBJECT* ";
    } else if (GET_ENTITY(obj_arg)->ident != NULL) {
      stri = id_string(GET_ENTITY(obj_arg)->ident);
    } else {
      stri = NULL;
      name_elem = GET_ENTITY(obj_arg)->name_list;
      while (name_elem != NULL && stri == NULL) {
        if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
          param_obj = name_elem->obj->value.objvalue;
          if (CATEGORY_OF_OBJ(param_obj) != VALUEPARAMOBJECT &&
              CATEGORY_OF_OBJ(param_obj) != REFPARAMOBJECT &&
              CATEGORY_OF_OBJ(param_obj) != TYPEOBJECT) {
            stri = id_string(GET_ENTITY(param_obj)->ident);
          } /* if */
        } else {
          stri = id_string(GET_ENTITY(name_elem->obj)->ident);
        } /* if */
        name_elem = name_elem->next;
      } /* while */
      if (stri == NULL) {
        stri = " *UNKNOWN_NAME* ";
      } /* if */
    } /* if */
    result = cstri_to_stri(stri);
    if (buffer != NULL) {
      UNALLOC_CSTRI(buffer, buffer_len);
    } /* if */
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* refStr */



#ifdef ANSI_C

typetype refType (objecttype obj_arg)
#else

typetype refType (obj_arg)
objecttype obj_arg;
#endif

  {
    typetype result;

  /* refType */
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else if (obj_arg->type_of == NULL) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = obj_arg->type_of;
    } /* if */
    return result;
  } /* refType */



#ifdef ANSI_C

acttype actValue (objecttype obj_arg)
#else

acttype actValue (obj_arg)
objecttype obj_arg;
#endif

  { /* actValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != ACTOBJECT) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return take_action(obj_arg);
    } /* if */
  } /* actValue */



#ifdef ANSI_C

biginttype bigValue (objecttype obj_arg)
#else

biginttype bigValue (obj_arg)
objecttype obj_arg;
#endif

  { /* bigValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != BIGINTOBJECT) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return bigCreate(take_bigint(obj_arg));
    } /* if */
  } /* bigValue */



#ifdef ANSI_C

bstritype bstValue (objecttype obj_arg)
#else

bstritype bstValue (obj_arg)
objecttype obj_arg;
#endif

  {
    bstritype bstri;
    bstritype result;

  /* bstValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != BSTRIOBJECT ||
        take_bstri(obj_arg) == NULL) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      bstri = take_bstri(obj_arg);
      if (!ALLOC_BSTRI_SIZE_OK(result, bstri->size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = bstri->size;
        memcpy(result->mem, bstri->mem,
            (size_t) (result->size * sizeof(uchartype)));
      } /* if */
    } /* if */
    return result;
  } /* bstValue */



#ifdef ANSI_C

chartype chrValue (objecttype obj_arg)
#else

chartype chrValue (obj_arg)
objecttype obj_arg;
#endif

  { /* chrValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != CHAROBJECT) {
      raise_error(RANGE_ERROR);
      return '\0';
    } else {
      return take_char(obj_arg);
    } /* if */
  } /* chrValue */



#ifdef ANSI_C

filetype filValue (objecttype obj_arg)
#else

filetype filValue (obj_arg)
objecttype obj_arg;
#endif

  { /* filValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != FILEOBJECT) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return take_file(obj_arg);
    } /* if */
  } /* filValue */



#ifdef ANSI_C

floattype fltValue (objecttype obj_arg)
#else

floattype fltValue (obj_arg)
objecttype obj_arg;
#endif

  { /* fltValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != FLOATOBJECT) {
      raise_error(RANGE_ERROR);
      return 0.0;
    } else {
      return take_float(obj_arg);
    } /* if */
  } /* fltValue */



#ifdef ANSI_C

inttype intValue (objecttype obj_arg)
#else

inttype intValue (obj_arg)
objecttype obj_arg;
#endif

  { /* intValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != INTOBJECT) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      return take_int(obj_arg);
    } /* if */
  } /* intValue */



#ifdef ANSI_C

progtype prgValue (objecttype obj_arg)
#else

progtype prgValue (obj_arg)
objecttype obj_arg;
#endif

  { /* prgValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != PROGOBJECT) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return take_prog(obj_arg);
    } /* if */
  } /* prgValue */



#ifdef ANSI_C

objecttype refValue (objecttype obj_arg)
#else

objecttype refValue (obj_arg)
objecttype obj_arg;
#endif

  { /* refValue */
    if (obj_arg != NULL &&
        (CATEGORY_OF_OBJ(obj_arg) == FWDREFOBJECT ||
         CATEGORY_OF_OBJ(obj_arg) == REFOBJECT ||
         CATEGORY_OF_OBJ(obj_arg) == REFPARAMOBJECT ||
         CATEGORY_OF_OBJ(obj_arg) == RESULTOBJECT ||
         CATEGORY_OF_OBJ(obj_arg) == LOCALVOBJECT ||
         CATEGORY_OF_OBJ(obj_arg) == ENUMLITERALOBJECT ||
         CATEGORY_OF_OBJ(obj_arg) == CONSTENUMOBJECT ||
         CATEGORY_OF_OBJ(obj_arg) == VARENUMOBJECT)) {
      return take_reference(obj_arg);
    } else {
      raise_error(RANGE_ERROR);
      return NULL;
    } /* if */
  } /* refValue */



#ifdef ANSI_C

listtype rflValue (objecttype obj_arg)
#else

listtype rflValue (obj_arg)
objecttype obj_arg;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* rflValue */
    if (obj_arg != NULL &&
        (CATEGORY_OF_OBJ(obj_arg) == MATCHOBJECT ||
         CATEGORY_OF_OBJ(obj_arg) == CALLOBJECT ||
         CATEGORY_OF_OBJ(obj_arg) == REFLISTOBJECT)) {
      result = copy_list(take_reflist(obj_arg), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } else {
      raise_error(RANGE_ERROR);
      result = NULL;
    } /* if */
    return result;
  } /* rflValue */



#ifdef ANSI_C

void rflSetvalue (objecttype list_to, listtype list_from)
#else

void rflSetvalue (list_to, list_from)
objecttype list_to;
listtype list_from;
#endif

  {
    listtype help_list;
    errinfotype err_info = OKAY_NO_ERROR;

  /* rflSetvalue */
    if (CATEGORY_OF_OBJ(list_to) == MATCHOBJECT ||
        CATEGORY_OF_OBJ(list_to) == CALLOBJECT ||
        CATEGORY_OF_OBJ(list_to) == REFLISTOBJECT) {
      help_list = copy_list(list_from, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
      } else {
        emptylist(take_reflist(list_to));
        list_to->value.listvalue = help_list;
      } /* if */
    } else {
      raise_error(RANGE_ERROR);
    } /* if */
  } /* rflSetvalue */



#ifdef ANSI_C

settype setValue (objecttype obj_arg)
#else

settype setValue (obj_arg)
objecttype obj_arg;
#endif

  {
    settype set1;
    memsizetype set_size;
    settype result;

  /* setValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != SETOBJECT ||
        take_set(obj_arg) == NULL) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      set1 = take_set(obj_arg);
      set_size = (uinttype) (set1->max_position - set1->min_position + 1);
      if (!ALLOC_SET(result, set_size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->min_position = set1->min_position;
        result->max_position = set1->max_position;
        memcpy(result->bitset, set1->bitset, set_size * sizeof(bitsettype));
      } /* if */
    } /* if */
    return result;
  } /* setValue */



#ifdef ANSI_C

stritype strValue (objecttype obj_arg)
#else

stritype strValue (obj_arg)
objecttype obj_arg;
#endif

  {
    stritype stri;
    stritype result;

  /* strValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != STRIOBJECT ||
        take_stri(obj_arg) == NULL) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      stri = take_stri(obj_arg);
      if (!ALLOC_STRI_SIZE_OK(result, stri->size)) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = stri->size;
        memcpy(result->mem, stri->mem,
            (size_t) (result->size * sizeof(strelemtype)));
      } /* if */
    } /* if */
    return result;
  } /* strValue */



#ifdef ANSI_C

typetype typValue (objecttype obj_arg)
#else

typetype typValue (obj_arg)
objecttype obj_arg;
#endif

  { /* typValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != TYPEOBJECT) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return take_type(obj_arg);
    } /* if */
  } /* typValue */
