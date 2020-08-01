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
#include "objutl.h"
#include "infile.h"    /* from the compiler library */
#include "name.h"      /* from the compiler library */
#include "blockutl.h"  /* from the compiler library */
#include "syvarutl.h"  /* from the compiler library */
#include "hsh_rtl.h"
#include "int_rtl.h"
#include "big_drv.h"
#include "pol_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "ref_data.h"


#define TABLE_INCREMENT 127



objecttype refAlloc (objecttype obj_arg)

  {
    objecttype created_object;

  /* refAlloc */
    if (ALLOC_OBJECT(created_object)) {
      created_object->type_of = obj_arg->type_of;
      memcpy(&created_object->descriptor, &obj_arg->descriptor,
          sizeof(descriptorunion));
      /* Copies the POSINFO flag (and all other flags): */
      INIT_CATEGORY_OF_OBJ(created_object, obj_arg->objcategory);
      created_object->value.objvalue = NULL;
    } else {
      raise_error(MEMORY_ERROR);
    } /* if */
    return created_object;
  } /* refAlloc */



inttype refArrMaxIdx (objecttype obj_arg)

  { /* refArrMaxIdx */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != ARRAYOBJECT) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      return take_array(obj_arg)->max_position;
    } /* if */
  } /* refArrMaxIdx */



inttype refArrMinIdx (objecttype obj_arg)

  { /* refArrMinIdx */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != ARRAYOBJECT) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      return take_array(obj_arg)->min_position;
    } /* if */
  } /* refArrMinIdx */



listtype refArrToList (objecttype obj_arg)

  {
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* refArrToList */
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
  } /* refArrToList */



objecttype refBody (objecttype obj_arg)

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



inttype refCategory (objecttype obj_arg)

  {
    inttype result;

  /* refCategory */
    if (unlikely(obj_arg == NULL)) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = CATEGORY_OF_OBJ(obj_arg);
    } /* if */
    return result;
  } /* refCategory */



inttype refCatParse (stritype category_name)

  {
    cstritype name;
    errinfotype err_info = OKAY_NO_ERROR;
    inttype result;

  /* refCatParse */
    name = stri_to_cstri(category_name, &err_info);
    if (name == NULL) {
      raise_error(err_info);
      result = -1;
    } else {
      result = category_value(name);
      free_cstri(name, category_name);
      if (result == -1) {
        raise_error(RANGE_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* refCatParse */



stritype refCatStr (inttype aCategory)

  {
    stritype result;

  /* refCatStr */
    result = cstri_to_stri(category_cstri((objectcategory) aCategory));
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* refCatStr */



stritype refFile (objecttype obj_arg)

  {
    filenumtype file_number;
    stritype result;

  /* refFile */
    if (unlikely(obj_arg == NULL)) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (HAS_POSINFO(obj_arg)) {
        file_number = GET_FILE_NUM(obj_arg);
      } else if (HAS_PROPERTY(obj_arg)) {
        /* trace1(obj_arg);
        printf(" %u %u %u\n",
            obj_arg->descriptor.property->file_number,
            obj_arg->descriptor.property->line,
            obj_arg->descriptor.property->syNumberInLine); */
        file_number = obj_arg->descriptor.property->file_number;
      } else {
        file_number = 0;
      } /* if */
      result = get_file_name(file_number);
      if (result == NULL) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* refFile */



listtype refHshDataToList (objecttype obj_arg)

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



listtype refHshKeysToList (objecttype obj_arg)

  {
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* refHshKeysToList */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != HASHOBJECT) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = hash_keys_to_list(take_hash(obj_arg), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* refHshKeysToList */



booltype refIsVar (objecttype obj_arg)

  { /* refIsVar */
    /* printf("refIsvar(%lu)\n", obj_arg); */
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
      return FALSE;
    } else {
      return VAR_OBJECT(obj_arg) ? TRUE : FALSE;
    } /* if */
  } /* refIsVar */



objecttype refItfToSct (objecttype obj_arg)

  {
    objecttype result;

  /* refItfToSct */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != INTERFACEOBJECT ||
        take_reference(obj_arg) == NULL) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = take_reference(obj_arg);
    } /* if */
    return result;
  } /* refItfToSct */



inttype refLine (objecttype obj_arg)

  {
    inttype result;

  /* refLine */
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
      result = 0;
    } else if (HAS_POSINFO(obj_arg)) {
      /* GET_LINE_NUM delivers an unsigned integer in the range 0 to 1048575 */
      result = (inttype) GET_LINE_NUM(obj_arg);
    } else if (HAS_PROPERTY(obj_arg)) {
      /* trace1(obj_arg);
      printf(" %s %u %u\n",
          get_file_name_ustri(obj_arg->descriptor.property->file_number),
          obj_arg->descriptor.property->line,
          obj_arg->descriptor.property->syNumberInLine); */
      /* Cast to inttype: The line is probably in the range 0 to 2147483647 */
      result = (inttype) obj_arg->descriptor.property->line;
    } else {
      result = 0;
    } /* if */
    return result;
  } /* refLine */



listtype refLocalConsts (objecttype obj_arg)

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
        free_list(result);
        result = NULL;
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* refLocalConsts */



listtype refLocalVars (objecttype obj_arg)

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
        free_list(result);
        result = NULL;
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* refLocalVars */



inttype refNum (objecttype obj_arg)

  {
    static rtlHashtype obj_table = NULL;
    static inttype next_free_number = 1;
    inttype result;

  /* refNum */
    /* printf("refNum(%lx)\n", obj_arg); */
    if (unlikely(obj_arg == NULL)) {
      result = 0;
    } else {
      if (unlikely(obj_table == NULL)) {
        obj_table = hshEmpty();
      } /* if */
      if (unlikely(obj_table == NULL)) {
        raise_error(MEMORY_ERROR);
        result = 0;
      } else {
        result = (inttype) hshIdxEnterDefault(obj_table, (generictype) (memsizetype) obj_arg,
            (generictype) next_free_number,
            (inttype) (((memsizetype) obj_arg) >> 6), (comparetype) &genericCmp,
            (createfunctype) &genericCreate, (createfunctype) &genericCreate);
        if (result == next_free_number) {
          next_free_number++;
        } /* if */
      } /* if */
    } /* if */
    /* printf("refNum => %ld\n", result); */
    return result;
  } /* refNum */



listtype refParams (objecttype obj_arg)

  {
    listtype result;

  /* refParams */
    result = NULL;
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      if (HAS_PROPERTY(obj_arg)) {
        result = obj_arg->descriptor.property->params;
      } /* if */
    } /* if */
    return result;
  } /* refParams */



objecttype refResini (objecttype obj_arg)

  {
    objecttype result;

  /* refResini */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != BLOCKOBJECT) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = obj_arg->value.blockvalue->result.init_value;
    } /* if */
    return result;
  } /* refResini */



objecttype refResult (objecttype obj_arg)

  {
    objecttype result;

  /* refResult */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != BLOCKOBJECT) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = obj_arg->value.blockvalue->result.object;
    } /* if */
    return result;
  } /* refResult */



listtype refSctToList (objecttype obj_arg)

  {
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* refSctToList */
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
  } /* refSctToList */



void refSetCategory (objecttype obj_arg, inttype aCategory)

  { /* refSetCategory */
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      SET_CATEGORY_OF_OBJ(obj_arg, aCategory);
    } /* if */
  } /* refSetCategory */



void refSetParams (objecttype obj_arg, const_listtype params)

  {
    errinfotype err_info = OKAY_NO_ERROR;

  /* refSetParams */
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      if (HAS_PROPERTY(obj_arg)) {
        free_list(obj_arg->descriptor.property->params);
        obj_arg->descriptor.property->params = copy_list(params, &err_info);
      } /* if */
      if (CATEGORY_OF_OBJ(obj_arg) == BLOCKOBJECT) {
        obj_arg->value.blockvalue->params =
            get_param_list(params, &err_info);
      } /* if */
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
  } /* refSetParams */



void refSetType (objecttype obj_arg, typetype any_type)

  { /* refSetType */
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      obj_arg->type_of = any_type;
    } /* if */
  } /* refSetType */



void refSetVar (objecttype obj_arg, booltype var_flag)

  { /* refSetVar */
    if (obj_arg == NULL) {
      raise_error(RANGE_ERROR);
    } else if (var_flag) {
      SET_VAR_FLAG(obj_arg);
    } else {
      CLEAR_VAR_FLAG(obj_arg);
    } /* if */
  } /* refSetVar */



stritype refStr (objecttype obj_arg)

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
      stri = (const_cstritype) get_file_name_ustri(GET_FILE_NUM(obj_arg));
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
      stri = id_string2(GET_ENTITY(obj_arg)->ident);
    } else {
      stri = NULL;
      name_elem = GET_ENTITY(obj_arg)->fparam_list;
      while (name_elem != NULL && stri == NULL) {
        if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
          param_obj = name_elem->obj->value.objvalue;
          if (CATEGORY_OF_OBJ(param_obj) != VALUEPARAMOBJECT &&
              CATEGORY_OF_OBJ(param_obj) != REFPARAMOBJECT &&
              CATEGORY_OF_OBJ(param_obj) != TYPEOBJECT) {
            stri = id_string2(GET_ENTITY(param_obj)->ident);
          } /* if */
        } else {
          stri = id_string2(GET_ENTITY(name_elem->obj)->ident);
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



typetype refType (objecttype obj_arg)

  {
    typetype result;

  /* refType */
    if (obj_arg == NULL || obj_arg->type_of == NULL) {
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      result = obj_arg->type_of;
    } /* if */
    return result;
  } /* refType */



acttype actValue (objecttype obj_arg)

  { /* actValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != ACTOBJECT) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return take_action(obj_arg);
    } /* if */
  } /* actValue */



biginttype bigValue (objecttype obj_arg)

  { /* bigValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != BIGINTOBJECT) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return bigCreate(take_bigint(obj_arg));
    } /* if */
  } /* bigValue */



booltype blnValue (objecttype obj_arg)

  { /* bln_value */
    if (obj_arg != NULL) {
      if (CATEGORY_OF_OBJ(obj_arg) == CONSTENUMOBJECT ||
          CATEGORY_OF_OBJ(obj_arg) == VARENUMOBJECT) {
        obj_arg = obj_arg->value.objvalue;
      } /* if */
      if (obj_arg->type_of != NULL &&
          obj_arg->type_of->owningProg != NULL) {
        if (obj_arg == TRUE_OBJECT(obj_arg->type_of->owningProg)) {
          return TRUE;
        } else if (obj_arg == FALSE_OBJECT(obj_arg->type_of->owningProg)) {
          return FALSE;
        } /* if */
      } /* if */
    } /* if */
    raise_error(RANGE_ERROR);
    return FALSE;
  } /* bln_value */



bstritype bstValue (objecttype obj_arg)

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



chartype chrValue (objecttype obj_arg)

  { /* chrValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != CHAROBJECT) {
      raise_error(RANGE_ERROR);
      return '\0';
    } else {
      return take_char(obj_arg);
    } /* if */
  } /* chrValue */



wintype drwValue (objecttype obj_arg)

  {
    wintype win_value;

  /* drwValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != WINOBJECT) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      win_value = take_win(obj_arg);
      if (win_value != NULL) {
        win_value->usage_count++;
      } /* if */
      return win_value;
    } /* if */
  } /* drwValue */



filetype filValue (objecttype obj_arg)

  { /* filValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != FILEOBJECT) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return take_file(obj_arg);
    } /* if */
  } /* filValue */



floattype fltValue (objecttype obj_arg)

  { /* fltValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != FLOATOBJECT) {
      raise_error(RANGE_ERROR);
      return 0.0;
    } else {
      return take_float(obj_arg);
    } /* if */
  } /* fltValue */



inttype intValue (objecttype obj_arg)

  { /* intValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != INTOBJECT) {
      raise_error(RANGE_ERROR);
      return 0;
    } else {
      return take_int(obj_arg);
    } /* if */
  } /* intValue */



polltype polValue (objecttype obj_arg)

  { /* polValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != POLLOBJECT) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return polCreate(take_poll(obj_arg));
    } /* if */
  } /* polValue */



progtype prgValue (objecttype obj_arg)

  { /* prgValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != PROGOBJECT) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return take_prog(obj_arg);
    } /* if */
  } /* prgValue */



objecttype refValue (objecttype obj_arg)

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



listtype rflValue (objecttype obj_arg)

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



void rflSetvalue (objecttype list_to, listtype list_from)

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
        free_list(take_reflist(list_to));
        list_to->value.listvalue = help_list;
      } /* if */
    } else {
      raise_error(RANGE_ERROR);
    } /* if */
  } /* rflSetvalue */



settype setValue (objecttype obj_arg)

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
      set_size = bitsetSize(set1);
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



stritype strValue (objecttype obj_arg)

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



typetype typValue (objecttype obj_arg)

  { /* typValue */
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != TYPEOBJECT) {
      raise_error(RANGE_ERROR);
      return NULL;
    } else {
      return take_type(obj_arg);
    } /* if */
  } /* typValue */
