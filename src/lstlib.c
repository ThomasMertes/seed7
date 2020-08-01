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
/*  Module: Library                                                 */
/*  File: seed7/src/lstlib.c                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: All primitive actions for the list type.               */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "listutl.h"
#include "executl.h"
#include "objutl.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "lstlib.h"



#ifdef ANSI_C

objecttype lst_cat (listtype arguments)
#else

objecttype lst_cat (arguments)
listtype arguments;
#endif

  {
    objecttype arg1;
    objecttype arg2;
    listtype list1_end;
    listtype list2_start;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;


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
        return(raise_exception(SYS_MEM_EXCEPTION));
      } /* if */
    } /* if */
    if (TEMP_OBJECT(arg2)) {
      list2_start = take_list(arg2);
      arg2->value.listvalue = NULL;
    } else {
      list2_start = copy_list(take_list(arg2), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } /* if */
    } /* if */
    if (TEMP_OBJECT(arg1)) {
      /* Necessary at this place: */
      /* When an exception is raised the arguments must */
      /* be unchanged to give a correct exception warning */
      /* and to give a clean state to the exception handler. */
      arg1->value.listvalue = NULL;
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
    return(bld_list_temp(result));
  } /* lst_cat */



#ifdef ANSI_C

objecttype lst_cpy (listtype arguments)
#else

objecttype lst_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype list_to;
    objecttype list_from;
    listtype help_list;
    errinfotype err_info = OKAY_NO_ERROR;

  /* lst_cpy */
    list_to = arg_1(arguments);
    list_from = arg_3(arguments);
    isit_list(list_to);
    isit_list(list_from);
    is_variable(list_to);
    if (list_from != list_to) {
      if (TEMP_OBJECT(list_from)) {
        emptylist(take_list(list_to));
        list_to->value.listvalue = take_list(list_from);
        list_from->value.listvalue = NULL;
      } else {
        help_list = copy_list(take_list(list_from), &err_info);
        if (err_info != OKAY_NO_ERROR) {
          return(raise_exception(SYS_MEM_EXCEPTION));
        } else {
          emptylist(take_list(list_to));
          list_to->value.listvalue = help_list;
        } /* if */
      } /* if */
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* lst_cpy */



#ifdef ANSI_C

objecttype lst_create (listtype arguments)
#else

objecttype lst_create (arguments)
listtype arguments;
#endif

  {
    objecttype list_to;
    objecttype list_from;
    errinfotype err_info = OKAY_NO_ERROR;

  /* lst_create */
    list_to = arg_1(arguments);
    list_from = arg_3(arguments);
    SET_CATEGORY_OF_OBJ(list_to, LISTOBJECT);
    isit_list(list_from);
    if (TEMP_OBJECT(list_from)) {
      list_to->value.listvalue = take_list(list_from);
      list_from->value.listvalue = NULL;
    } else {
      list_to->value.listvalue = copy_list(take_list(list_from), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        list_to->value.listvalue = NULL;
        return(raise_exception(SYS_MEM_EXCEPTION));
      } /* if */
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* lst_create */



#ifdef ANSI_C

objecttype lst_destr (listtype arguments)
#else

objecttype lst_destr (arguments)
listtype arguments;
#endif

  {
    objecttype old_list;
    register listtype list_end;
    errinfotype err_info = OKAY_NO_ERROR;

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
#ifdef OUT_OF_ORDER
    emptylist(take_list(old_list));
#endif
    old_list->value.listvalue = NULL;
    return(SYS_EMPTY_OBJECT);
  } /* lst_destr */



#ifdef ANSI_C

objecttype lst_elem (listtype arguments)
#else

objecttype lst_elem (arguments)
listtype arguments;
#endif

  {
#ifdef OUT_OF_ORDER
    objecttype searched_object;
    listtype list_element;
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
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
#endif
    return(raise_exception(SYS_ACT_ILLEGAL_EXCEPTION));
  } /* lst_elem */



#ifdef ANSI_C

objecttype lst_empty (listtype arguments)
#else

objecttype lst_empty (arguments)
listtype arguments;
#endif

  { /* lst_empty */
    return(bld_list_temp(NULL));
  } /* lst_empty */



#ifdef ANSI_C

objecttype lst_excl (listtype arguments)
#else

objecttype lst_excl (arguments)
listtype arguments;
#endif

  { /* lst_excl */
    isit_list(arg_1(arguments));
    excl_list(&arg_1(arguments)->value.listvalue,
        arg_2(arguments));
    return(SYS_EMPTY_OBJECT);
  } /* lst_excl */



#ifdef ANSI_C

objecttype lst_head (listtype arguments)
#else

objecttype lst_head (arguments)
listtype arguments;
#endif

  {
    objecttype list;
    inttype stop;
    inttype number;
    listtype stop_element;
    listtype saved_list_rest;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

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
          emptylist(stop_element->next);
          stop_element->next = NULL;
        } /* if */
        result = take_list(list);
        list->value.listvalue = NULL;
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
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      return(bld_list_temp(result));
    } /* if */
  } /* lst_head */



#ifdef ANSI_C

objecttype lst_idx (listtype arguments)
#else

objecttype lst_idx (arguments)
listtype arguments;
#endif

  {
    inttype position;
    inttype number;
    listtype list_element;
    objecttype result;

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
        result = raise_exception(SYS_RNG_EXCEPTION);
      } /* if */
    } else {
      result = raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    return(result);
  } /* lst_idx */



#ifdef ANSI_C

objecttype lst_incl (listtype arguments)
#else

objecttype lst_incl (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;
    errinfotype err_info = OKAY_NO_ERROR;

  /* lst_incl */
    isit_list(arg_1(arguments));
    obj_arg = arg_2(arguments);
    if (CATEGORY_OF_OBJ(obj_arg) == VARENUMOBJECT ||
        CATEGORY_OF_OBJ(obj_arg) == CONSTENUMOBJECT) {
      obj_arg = take_reference(obj_arg);
    } /* if */
    incl_list(&arg_1(arguments)->value.listvalue, obj_arg, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      return(SYS_EMPTY_OBJECT);
    } /* if */
  } /* lst_incl */



#ifdef ANSI_C

objecttype lst_lng (listtype arguments)
#else

objecttype lst_lng (arguments)
listtype arguments;
#endif

  {
    listtype list_element;
    inttype result;

  /* lst_lng */
    isit_list(arg_1(arguments));
    list_element = take_list(arg_1(arguments));
    result = 0;
    while (list_element != NULL) {
      list_element = list_element->next;
      result++;
    } /* while */
    return(bld_int_temp(result));
  } /* lst_lng */



#ifdef ANSI_C

objecttype lst_range (listtype arguments)
#else

objecttype lst_range (arguments)
listtype arguments;
#endif

  {
    objecttype list;
    inttype start;
    inttype stop;
    inttype number;
    listtype start_element;
    listtype stop_element;
    listtype *start_address;
    listtype saved_list_rest;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

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
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      return(bld_list_temp(result));
    } /* if */
  } /* lst_range */



#ifdef ANSI_C

objecttype lst_tail (listtype arguments)
#else

objecttype lst_tail (arguments)
listtype arguments;
#endif

  {
    objecttype list;
    inttype start;
    inttype number;
    listtype list_element;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

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
        list->value.listvalue = NULL;
      } else {
        result = copy_list(list_element, &err_info);
      } /* if */
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      return(bld_list_temp(result));
    } /* if */
  } /* lst_tail */
