/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
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
/*  Module: Library                                                 */
/*  File: seed7/src/lstlib.c                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: All primitive actions for the list type.               */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "listutl.h"
#include "executl.h"
#include "objutl.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "lstlib.h"



objectType lst_cat (listType arguments)

  {
    objectType arg1;
    objectType arg2;
    listType list1_end;
    listType list2_start;
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;


  /* lst_cat */
    arg1 = arg_1(arguments);
    arg2 = arg_3(arguments);
    isit_list(arg1);
    isit_list(arg2);
    if (TEMP_OBJECT(arg1)) {
      result = take_list(arg1);
    } else {
      result = copy_list(take_list(arg1), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
    } /* if */
    if (TEMP_OBJECT(arg2)) {
      list2_start = take_list(arg2);
      arg2->value.listValue = NULL;
    } else {
      list2_start = copy_list(take_list(arg2), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
    } /* if */
    if (TEMP_OBJECT(arg1)) {
      /* Necessary at this place: */
      /* When an exception is raised the arguments must */
      /* be unchanged to give a correct exception warning */
      /* and to give a clean state to the exception handler. */
      arg1->value.listValue = NULL;
    } /* if */
    if (result != NULL) {
      list1_end = result;
      while (list1_end->next != NULL) {
        list1_end = list1_end->next;
      } /* while */
      list1_end->next = list2_start;
    } else {
      result = list2_start;
    } /* if */
    return bld_list_temp(result);
  } /* lst_cat */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType lst_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    listType help_list;
    errInfoType err_info = OKAY_NO_ERROR;

  /* lst_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_list(dest);
    isit_list(source);
    is_variable(dest);
    if (source != dest) {
      if (TEMP_OBJECT(source)) {
        free_list(take_list(dest));
        dest->value.listValue = take_list(source);
        source->value.listValue = NULL;
      } else {
        help_list = copy_list(take_list(source), &err_info);
        if (err_info != OKAY_NO_ERROR) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          free_list(take_list(dest));
          dest->value.listValue = help_list;
        } /* if */
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* lst_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType lst_create (listType arguments)

  {
    objectType dest;
    objectType source;
    errInfoType err_info = OKAY_NO_ERROR;

  /* lst_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    SET_CATEGORY_OF_OBJ(dest, LISTOBJECT);
    isit_list(source);
    if (TEMP_OBJECT(source)) {
      dest->value.listValue = take_list(source);
      source->value.listValue = NULL;
    } else {
      dest->value.listValue = copy_list(take_list(source), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        dest->value.listValue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* lst_create */



objectType lst_destr (listType arguments)

  {
    objectType old_list;
    register listType list_end;
    errInfoType err_info = OKAY_NO_ERROR;

  /* lst_destr */
    old_list = arg_1(arguments);
    isit_list(old_list);
    list_end = take_list(old_list);
    if (list_end != NULL) {
      while (list_end->next != NULL) {
        if (TEMP_OBJECT(list_end->obj)) {
          do_destroy(list_end->obj, &err_info);
          FREE_OBJECT(list_end->obj);
        } /* if */
        list_end = list_end->next;
      } /* while */
      if (TEMP_OBJECT(list_end->obj)) {
        do_destroy(list_end->obj, &err_info);
        FREE_OBJECT(list_end->obj);
      } /* if */
      list_end->next = flist.list_elems;
      flist.list_elems = take_list(old_list);
    } /* if */
    old_list->value.listValue = NULL;
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* lst_destr */



objectType lst_elem (listType arguments)

  {
#ifdef OUT_OF_ORDER
    objectType searched_object;
    listType list_element;
#endif

  /* lst_elem */
#ifdef OUT_OF_ORDER
    isit_list(arg_3(arguments));
    searched_object = arg_1(arguments);
    if (CATEGORY_OF_OBJ(searched_object) == VARENUMOBJECT ||
        CATEGORY_OF_OBJ(searched_object) == CONSTENUMOBJECT) {
      searched_object = take_reference(searched_object);
    } /* if */
    list_element = take_list(arg_3(arguments));
    while (list_element != NULL && list_element->obj != searched_object) {
      list_element = list_element->next;
    } /* while */
    if (list_element != NULL) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
#endif
    return raise_exception(SYS_ACT_ILLEGAL_EXCEPTION);
  } /* lst_elem */



objectType lst_empty (listType arguments)

  { /* lst_empty */
    return bld_list_temp(NULL);
  } /* lst_empty */



objectType lst_excl (listType arguments)

  { /* lst_excl */
    isit_list(arg_1(arguments));
    excl_list(&arg_1(arguments)->value.listValue,
        arg_2(arguments));
    return SYS_EMPTY_OBJECT;
  } /* lst_excl */



objectType lst_head (listType arguments)

  {
    objectType list;
    intType stop;
    intType number;
    listType stop_element;
    listType saved_list_rest;
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* lst_head */
    isit_int(arg_4(arguments));
    list = arg_1(arguments);
    isit_list(list);
    stop = take_int(arg_4(arguments));
    if (stop >= 1) {
      number = 1;
      stop_element = take_list(list);
      while (number < stop && stop_element != NULL) {
        number++;
        stop_element = stop_element->next;
      } /* while */
      if (TEMP_OBJECT(list)) {
        if (stop_element != NULL) {
          free_list(stop_element->next);
          stop_element->next = NULL;
        } /* if */
        result = take_list(list);
        list->value.listValue = NULL;
      } else {
        if (stop_element != NULL) {
          saved_list_rest = stop_element->next;
          stop_element->next = NULL;
          result = copy_list(take_list(list), &err_info);
          stop_element->next = saved_list_rest;
        } else {
          result = copy_list(take_list(list), &err_info);
        } /* if */
      } /* if */
    } else {
      result = NULL;
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_list_temp(result);
    } /* if */
  } /* lst_head */



objectType lst_idx (listType arguments)

  {
    intType position;
    intType number;
    listType list_element;
    objectType result;

  /* lst_idx */
    isit_list(arg_1(arguments));
    isit_int(arg_3(arguments));
    list_element = take_list(arg_1(arguments));
    position = take_int(arg_3(arguments));
    if (position >= 1) {
      number = 1;
      while (number < position && list_element != NULL) {
        number++;
        list_element = list_element->next;
      } /* while */
      if (list_element != NULL) {
        result = list_element->obj;
      } else {
        logError(printf("lst_idx(list1, " FMT_D "): "
                        "Index larger than maximum (" FMT_D ").\n",
                        position, number - 1););
        result = raise_exception(SYS_RNG_EXCEPTION);
      } /* if */
    } else {
      logError(printf("lst_idx(list1, " FMT_D "): "
                      "Index is negative or zero.\n", position););
      result = raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    return result;
  } /* lst_idx */



objectType lst_incl (listType arguments)

  {
    objectType obj_arg;
    errInfoType err_info = OKAY_NO_ERROR;

  /* lst_incl */
    isit_list(arg_1(arguments));
    obj_arg = arg_2(arguments);
    if (CATEGORY_OF_OBJ(obj_arg) == VARENUMOBJECT ||
        CATEGORY_OF_OBJ(obj_arg) == CONSTENUMOBJECT) {
      obj_arg = take_reference(obj_arg);
    } /* if */
    incl_list(&arg_1(arguments)->value.listValue, obj_arg, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* lst_incl */



objectType lst_lng (listType arguments)

  {
    listType list_element;
    intType length;

  /* lst_lng */
    isit_list(arg_1(arguments));
    list_element = take_list(arg_1(arguments));
    length = 0;
    while (list_element != NULL) {
      list_element = list_element->next;
      length++;
    } /* while */
    return bld_int_temp(length);
  } /* lst_lng */



objectType lst_range (listType arguments)

  {
    objectType list;
    intType start;
    intType stop;
    intType number;
    listType start_element;
    listType stop_element;
    listType *start_address;
    listType saved_list_rest;
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* lst_range */
    isit_list(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_5(arguments));
    list = arg_1(arguments);
    start = take_int(arg_3(arguments));
    stop = take_int(arg_5(arguments));
    number = 1;
    start_address = &take_list(list);
    start_element = take_list(list);
    while (number < start && start_element != NULL) {
      number++;
      start_address = &start_element->next;
      start_element = start_element->next;
    } /* while */
    if (start_element != NULL && stop >= start) {
      stop_element = start_element;
      while (number < stop && stop_element != NULL) {
        number++;
        stop_element = stop_element->next;
      } /* while */
      if (TEMP_OBJECT(list)) {
        *start_address = stop_element;
        result = start_element;
      } else {
        if (stop_element != NULL) {
          saved_list_rest = stop_element->next;
          stop_element->next = NULL;
          result = copy_list(start_element, &err_info);
          stop_element->next = saved_list_rest;
        } else {
          result = copy_list(start_element, &err_info);
        } /* if */
      } /* if */
    } else {
      result = NULL;
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_list_temp(result);
    } /* if */
  } /* lst_range */



objectType lst_tail (listType arguments)

  {
    objectType list;
    intType start;
    intType number;
    listType list_element;
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* lst_tail */
    isit_list(arg_1(arguments));
    isit_int(arg_3(arguments));
    list = arg_1(arguments);
    start = take_int(arg_3(arguments));
    list_element = take_list(list);
    if (start > 1 && list_element != NULL) {
      number = 2;
      while (number < start && list_element->next != NULL) {
        number++;
        list_element = list_element->next;
      } /* while */
      if (number >= start) {
        if (TEMP_OBJECT(list)) {
          result = list_element->next;
          list_element->next = NULL;
        } else {
          result = copy_list(list_element->next, &err_info);
        } /* if */
      } else {
        result = NULL;
      } /* if */
    } else {
      if (TEMP_OBJECT(list)) {
        result = list_element;
        list->value.listValue = NULL;
      } else {
        result = copy_list(list_element, &err_info);
      } /* if */
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_list_temp(result);
    } /* if */
  } /* lst_tail */
