/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2008  Thomas Mertes                        */
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
/*  File: seed7/src/rfllib.c                                        */
/*  Changes: 1991, 1992, 1993, 1994, 1995  Thomas Mertes            */
/*  Content: All primitive actions for the ref_list type.           */
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
#include "traceutl.h"
#include "listutl.h"
#include "objutl.h"
#include "exec.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "rfllib.h"



/**
 *  Append the ref_list 'extension/arg_3' to 'dest/arg_1'.
 *  @exception MEMORY_ERROR Not enough memory for the concatenated
 *             ref_list.
 */
objectType rfl_append (listType arguments)

  {
    objectType rfl_variable;
    listType rfl_to;
    listType rfl_from;
    listType list1_end;
    listType list2_start;
    errInfoType err_info = OKAY_NO_ERROR;

  /* rfl_append */
    rfl_variable = arg_1(arguments);
    isit_reflist(rfl_variable);
    is_variable(rfl_variable);
    rfl_to = take_reflist(rfl_variable);
    isit_reflist(arg_3(arguments));
    rfl_from = take_reflist(arg_3(arguments));
    if (TEMP_OBJECT(arg_3(arguments))) {
      list2_start = rfl_from;
      arg_3(arguments)->value.listValue = NULL;
    } else {
      list2_start = copy_list(rfl_from, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
    } /* if */
    if (rfl_to != NULL) {
      list1_end = rfl_to;
      while (list1_end->next != NULL) {
        list1_end = list1_end->next;
      } /* while */
      list1_end->next = list2_start;
    } else {
      rfl_variable->value.listValue = list2_start;
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* rfl_append */



/**
 *  Concatenate two ref_lists ('list1/arg_1' and 'list2/arg_3').
 *  @return the result of the concatenation.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType rfl_cat (listType arguments)

  {
    objectType arg1;
    objectType arg2;
    listType list1_end;
    listType list2_start;
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* rfl_cat */
    arg1 = arg_1(arguments);
    arg2 = arg_3(arguments);
    isit_reflist(arg1);
    isit_reflist(arg2);
    if (TEMP_OBJECT(arg1)) {
      result = take_reflist(arg1);
    } else {
      result = copy_list(take_reflist(arg1), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
    } /* if */
    if (TEMP_OBJECT(arg2)) {
      list2_start = take_reflist(arg2);
      arg2->value.listValue = NULL;
    } else {
      list2_start = copy_list(take_reflist(arg2), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
    } /* if */
    if (TEMP_OBJECT(arg1)) {
      /* Necessary at this place: */
      /* If an exception is raised the arguments must */
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
    return bld_reflist_temp(result);
  } /* rfl_cat */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType rfl_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    listType help_list;
    errInfoType err_info = OKAY_NO_ERROR;

  /* rfl_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_reflist(dest);
    isit_reflist(source);
    is_variable(dest);
    if (source != dest) {
      if (TEMP_OBJECT(source)) {
        free_list(take_reflist(dest));
        dest->value.listValue = take_reflist(source);
        source->value.listValue = NULL;
      } else {
        help_list = copy_list(take_reflist(source), &err_info);
        if (err_info != OKAY_NO_ERROR) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          free_list(take_reflist(dest));
          dest->value.listValue = help_list;
        } /* if */
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* rfl_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType rfl_create (listType arguments)

  {
    objectType dest;
    objectType source;
    errInfoType err_info = OKAY_NO_ERROR;

  /* rfl_create */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    SET_CATEGORY_OF_OBJ(dest, REFLISTOBJECT);
    isit_reflist(source);
    if (TEMP_OBJECT(source)) {
      dest->value.listValue = take_reflist(source);
      source->value.listValue = NULL;
    } else {
      dest->value.listValue = copy_list(take_reflist(source), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        dest->value.listValue = NULL;
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* rfl_create */



/**
 *  Free the memory referred by 'old_list/arg_1'.
 *  After rfl_destr is left 'old_list/arg_1' is NULL.
 *  The memory where 'old_list/arg_1' is stored can be freed afterwards.
 */
objectType rfl_destr (listType arguments)

  { /* rfl_destr */
    isit_reflist(arg_1(arguments));
    free_list(take_reflist(arg_1(arguments)));
    arg_1(arguments)->value.listValue = NULL;
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* rfl_destr */



objectType rfl_elem (listType arguments)

  {
    objectType searched_object;
    listType list_element;

  /* rfl_elem */
    isit_reference(arg_1(arguments));
    isit_reflist(arg_3(arguments));
    searched_object = take_reference(arg_1(arguments));
    list_element = take_reflist(arg_3(arguments));
    while (list_element != NULL && list_element->obj != searched_object) {
      list_element = list_element->next;
    } /* while */
    if (list_element != NULL) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* rfl_elem */



/**
 *  Assign reference 'source/arg_6' to the 'position/arg_4' of the 'dest/arg_1'.
 *   A @:= [B] C;
 *  is equivalent to
 *   A := A[..pred(B)] & make_list(C) & A[succ(B)..];
 *  @exception RANGE_ERROR If 'position/arg_4' is negative or zero.
 *  @exception RANGE_ERROR An element beyond 'dest/arg_1' would be
 *             overwritten ('position/arg_4' > length('dest/arg_1') holds).
 */
objectType rfl_elemcpy (listType arguments)

  {
    intType position;
    listType list_element;

  /* rfl_elemcpy */
    isit_reflist(arg_1(arguments));
    isit_int(arg_4(arguments));
    isit_reference(arg_6(arguments));
    is_variable(arg_1(arguments));
    list_element = take_reflist(arg_1(arguments));
    position = take_int(arg_4(arguments));
    if (unlikely(position <= 0)) {
      logError(printf("rfl_elemcpy(" FMT_U_MEM ", " FMT_D ", " FMT_U_MEM "): "
                      "Index <= 0.\n",
                      (memSizeType) take_reflist(arg_1(arguments)),
                      position,
                      (memSizeType) take_reference(arg_6(arguments))););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      position--;
      while (position != 0 && list_element != NULL) {
        position--;
        list_element = list_element->next;
      } /* while */
      if (unlikely(list_element == NULL)) {
        logError(printf("rfl_elemcpy(" FMT_U_MEM ", " FMT_D ", " FMT_U_MEM "): "
                        "Index > length(dest).\n",
                        (memSizeType) take_reflist(arg_1(arguments)),
                        position,
                        (memSizeType) take_reference(arg_6(arguments))););
        return raise_exception(SYS_RNG_EXCEPTION);
      } else {
        list_element->obj = take_reference(arg_6(arguments));
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* rfl_elemcpy */



objectType rfl_empty (listType arguments)

  { /* rfl_empty */
    return bld_reflist_temp(NULL);
  } /* rfl_empty */



/**
 *  Check if two ref_lists are equal.
 *  @return TRUE if both ref_lists are equal,
 *          FALSE otherwise.
 */
objectType rfl_eq (listType arguments)

  {
    listType list1;
    listType list2;
    objectType result;

  /* rfl_eq */
    isit_reflist(arg_1(arguments));
    isit_reflist(arg_3(arguments));
    list1 = take_reflist(arg_1(arguments));
    list2 = take_reflist(arg_3(arguments));
    while (list1 != NULL && list2 != NULL &&
        list1->obj == list2->obj) {
      list1 = list1->next;
      list2 = list2->next;
    } /* while */
    if (list1 == NULL && list2 == NULL) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* rfl_eq */



objectType rfl_excl (listType arguments)

  { /* rfl_excl */
    isit_reflist(arg_1(arguments));
    isit_reference(arg_2(arguments));
    excl_list(&arg_1(arguments)->value.listValue,
        take_reference(arg_2(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* rfl_excl */



objectType rfl_expr (listType arguments)

  {
    listType list1;
    objectType arg1_object;
    objectType arg3_object;
    boolType okay;
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* rfl_expr */
    isit_list(arg_1(arguments));
    list1 = take_list(arg_1(arguments));
    logFunction(printf("rfl_expr(");
                prot_list(list1);
                printf(")\n"););
    result = NULL;
    if (list1 != NULL &&
        list1->next == NULL) {
      arg1_object = arg_1(list1);
      incl_list(&result, arg1_object, &err_info);
    } else {
      okay = TRUE;
      do {
        if (list1 != NULL &&
            list1->next != NULL &&
            list1->next->next != NULL &&
            list1->next->next->next == NULL) {
          arg1_object = arg_1(list1);
          arg3_object = arg_3(list1);
          incl_list(&result, arg3_object, &err_info);
          if (CATEGORY_OF_OBJ(arg1_object) == EXPROBJECT) {
            list1 = take_list(arg1_object);
          } else {
            incl_list(&result, arg1_object, &err_info);
            okay = FALSE;
          } /* if */
        } else {
          logError(printf("rfl_expr: Not okay\n");
                   printf("list1: " FMT_X_MEM "\n", (memSizeType) list1);
                   printf("list1->next: " FMT_X_MEM "\n",
                          (memSizeType) list1->next);
                   printf("list1->next->next: " FMT_X_MEM "\n",
                          (memSizeType) list1->next->next);
                   printf("list1->next->next->next: " FMT_X_MEM "\n",
                          (memSizeType) list1->next->next->next););
          return raise_exception(SYS_RNG_EXCEPTION);
        } /* if */
      } while (okay);
    } /* if */
    logFunction(printf("rfl_expr --> ");
                prot_list(result);
                printf("\n"););
    return bld_reflist_temp(result);
  } /* rfl_expr */



objectType rfl_for (listType arguments)

  {
    objectType statement;
    objectType elementlist;
    objectType for_variable;
    listType helplist;
    listType listelement;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* rfl_for */
/*  prot_list(take_list(arg_4(arguments))); */
    for_variable = arg_2(arguments);
    elementlist = arg_4(arguments);
    statement = arg_6(arguments);
    isit_reference (for_variable);
    isit_reflist(elementlist);
    helplist = copy_list(take_list(elementlist), &err_info);
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      listelement = helplist;
      result = SYS_EMPTY_OBJECT;
      while (listelement != NULL && result != NULL) {
        for_variable->value.objValue = listelement->obj;
        result = evaluate(statement);
        listelement = listelement->next;
      } /* while */
      free_list(helplist);
      return result;
    } /* if */
  } /* rfl_for */



objectType rfl_for_until (listType arguments)

  {
    objectType for_variable;
    objectType elementlist;
    objectType statement;
    objectType condition;
    objectType cond_value;
    boolType cond;
    listType helplist;
    listType listelement;
    errInfoType err_info = OKAY_NO_ERROR;

  /* rfl_for_until */
/*  prot_list(take_list(arg_4(arguments))); */
    for_variable = arg_2(arguments);
    elementlist = arg_4(arguments);
    condition = arg_6(arguments);
    statement = arg_8(arguments);
    isit_reference (for_variable);
    isit_reflist(elementlist);
    helplist = copy_list(take_list(elementlist), &err_info);
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      listelement = helplist;
      if (listelement != NULL) {
        for_variable->value.objValue = listelement->obj;
        cond_value = evaluate(condition);
        if (!fail_flag) {
          isit_bool(cond_value);
          cond = (boolType) (take_bool(cond_value) == SYS_FALSE_OBJECT);
          if (TEMP_OBJECT(cond_value)) {
            dump_any_temp(cond_value);
          } /* if */
          while (cond && listelement != NULL && !fail_flag) {
            evaluate(statement);
            if (!fail_flag) {
              listelement = listelement->next;
              if (listelement != NULL) {
                for_variable->value.objValue = listelement->obj;
                cond_value = evaluate(condition);
                if (!fail_flag) {
                  isit_bool(cond_value);
                  cond = (boolType) (take_bool(cond_value) == SYS_FALSE_OBJECT);
                  if (TEMP_OBJECT(cond_value)) {
                    dump_any_temp(cond_value);
                  } /* if */
                } /* if */
              } /* if */
            } /* if */
          } /* while */
        } /* if */
      } /* if */
      free_list(helplist);
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* rfl_for_until */



/**
 *  Get a sublist from 'list/arg_1' ending at the 'stop/arg_4' position.
 *  The first element in a 'ref_list' has the position 1.
 *  @return the substring ending at the 'stop/arg_4' position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType rfl_head (listType arguments)

  {
    objectType list;
    intType stop;
    intType number;
    listType stop_element;
    listType saved_list_rest;
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* rfl_head */
    isit_int(arg_4(arguments));
    list = arg_1(arguments);
    isit_reflist(list);
    stop = take_int(arg_4(arguments));
    if (stop >= 1) {
      number = 1;
      stop_element = take_reflist(list);
      while (number < stop && stop_element != NULL) {
        number++;
        stop_element = stop_element->next;
      } /* while */
      if (TEMP_OBJECT(list)) {
        if (stop_element != NULL) {
          free_list(stop_element->next);
          stop_element->next = NULL;
        } /* if */
        result = take_reflist(list);
        list->value.listValue = NULL;
      } else {
        if (stop_element != NULL) {
          saved_list_rest = stop_element->next;
          stop_element->next = NULL;
          result = copy_list(take_reflist(list), &err_info);
          stop_element->next = saved_list_rest;
        } else {
          result = copy_list(take_reflist(list), &err_info);
        } /* if */
      } /* if */
    } else {
      result = NULL;
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return bld_reflist_temp(result);
    } /* if */
  } /* rfl_head */



/**
 *  Access one element from the 'ref_list' 'list/arg_1'.
 *  @return the element with the specified 'position/arg_3' from 'list/arg_1'.
 *  @exception RANGE_ERROR If the index is less than 1 or
 *             greater than the length of the 'ref_list'.
 */
objectType rfl_idx (listType arguments)

  {
    intType position;
    listType list_element;
    objectType result;

  /* rfl_idx */
    isit_reflist(arg_1(arguments));
    isit_int(arg_3(arguments));
    list_element = take_reflist(arg_1(arguments));
    position = take_int(arg_3(arguments));
    if (unlikely(position <= 0)) {
      logError(printf("rfl_idx(" FMT_U_MEM ", " FMT_D "): "
                      "Index <= 0.\n",
                      (memSizeType) take_reflist(arg_1(arguments)),
                      position););
      result = raise_exception(SYS_RNG_EXCEPTION);
    } else {
      position--;
      while (position != 0 && list_element != NULL) {
        position--;
        list_element = list_element->next;
      } /* while */
      if (unlikely(list_element == NULL)) {
        logError(printf("rfl_idx(" FMT_U_MEM ", " FMT_D "): "
                        "Index > length(list).\n",
                        (memSizeType) take_reflist(arg_1(arguments)),
                        position););
        result = raise_exception(SYS_RNG_EXCEPTION);
      } else {
        result = bld_reference_temp(list_element->obj);
      } /* if */
    } /* if */
    return result;
  } /* rfl_idx */



objectType rfl_incl (listType arguments)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* rfl_incl */
    isit_reflist(arg_1(arguments));
    isit_reference(arg_2(arguments));
    incl_list(&arg_1(arguments)->value.listValue,
        take_reference(arg_2(arguments)), &err_info);
    if (err_info != OKAY_NO_ERROR) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      return SYS_EMPTY_OBJECT;
    } /* if */
  } /* rfl_incl */



objectType rfl_ipos (listType arguments)

  {
    listType list_element;
    objectType searched_object;
    intType from_index;
    intType result;

  /* rfl_ipos */
    isit_reflist(arg_1(arguments));
    isit_reference(arg_2(arguments));
    list_element = take_reflist(arg_1(arguments));
    searched_object = take_reference(arg_2(arguments));
    from_index = take_int(arg_3(arguments));
    result = 1;
    while (list_element != NULL && result < from_index) {
      list_element = list_element->next;
      result++;
    } /* while */
    while (list_element != NULL && list_element->obj != searched_object) {
      list_element = list_element->next;
      result++;
    } /* while */
    if (list_element == NULL) {
      result = 0;
    } /* if */
    return bld_int_temp(result);
  } /* rfl_ipos */



/**
 *  Determine the length of a 'ref_list'.
 *  @return the length of the 'ref_list'.
 */
objectType rfl_lng (listType arguments)

  {
    listType list_element;
    intType length = 0;

  /* rfl_lng */
    isit_reflist(arg_1(arguments));
    list_element = take_reflist(arg_1(arguments));
    while (list_element != NULL) {
      list_element = list_element->next;
      length++;
    } /* while */
    return bld_int_temp(length);
  } /* rfl_lng */



objectType rfl_mklist (listType arguments)

  {
    listType result;

  /* rfl_mklist */
    isit_reference(arg_1(arguments));
    if (ALLOC_L_ELEM(result)) {
      result->next = NULL;
      result->obj = take_reference(arg_1(arguments));
      return bld_reflist_temp(result);
    } else {
      return raise_exception(SYS_MEM_EXCEPTION);
    } /* if */
  } /* rfl_mklist */



/**
 *  Check if two ref_lists are not equal.
 *  @return FALSE if both ref_lists are equal,
 *          TRUE otherwise.
 */
objectType rfl_ne (listType arguments)

  {
    listType list1;
    listType list2;
    objectType result;

  /* rfl_ne */
    isit_reflist(arg_1(arguments));
    isit_reflist(arg_3(arguments));
    list1 = take_reflist(arg_1(arguments));
    list2 = take_reflist(arg_3(arguments));
    while (list1 != NULL && list2 != NULL &&
        list1->obj == list2->obj) {
      list1 = list1->next;
      list2 = list2->next;
    } /* while */
    if (list1 != NULL || list2 != NULL) {
      result = SYS_TRUE_OBJECT;
    } else {
      result = SYS_FALSE_OBJECT;
    } /* if */
    return result;
  } /* rfl_ne */



objectType rfl_not_elem (listType arguments)

  {
    objectType searched_object;
    listType list_element;

  /* rfl_not_elem */
    isit_reference(arg_1(arguments));
    isit_reflist(arg_4(arguments));
    searched_object = take_reference(arg_1(arguments));
    list_element = take_reflist(arg_4(arguments));
    while (list_element != NULL && list_element->obj != searched_object) {
      list_element = list_element->next;
    } /* while */
    if (list_element == NULL) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* rfl_not_elem */



objectType rfl_pos (listType arguments)

  {
    listType list_element;
    objectType searched_object;
    intType result;

  /* rfl_pos */
    isit_reflist(arg_1(arguments));
    isit_reference(arg_2(arguments));
    list_element = take_reflist(arg_1(arguments));
    searched_object = take_reference(arg_2(arguments));
    result = 1;
    while (list_element != NULL && list_element->obj != searched_object) {
      list_element = list_element->next;
      result++;
    } /* while */
    if (list_element == NULL) {
      result = 0;
    } /* if */
    return bld_int_temp(result);
  } /* rfl_pos */



/**
 *  Get a sublist from a 'start/arg_3' position to a 'stop/arg_5' position.
 *  The first element in a 'ref_list' has the position 1.
 *  @return the substring from position start to stop.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType rfl_range (listType arguments)

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

  /* rfl_range */
    isit_reflist(arg_1(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_5(arguments));
    list = arg_1(arguments);
    start = take_int(arg_3(arguments));
    stop = take_int(arg_5(arguments));
    number = 1;
    start_address = &take_reflist(list);
    start_element = take_reflist(list);
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
      return bld_reflist_temp(result);
    } /* if */
  } /* rfl_range */



objectType rfl_set_value (listType arguments)

  {
    objectType list_to;
    objectType list_from;
    listType help_list;
    errInfoType err_info = OKAY_NO_ERROR;

  /* rfl_set_value */
    isit_reference(arg_1(arguments));
    is_variable(arg_1(arguments));
    list_to = take_reference(arg_1(arguments));
    list_from = arg_2(arguments);
    isit_reflist(list_from);
    if (CATEGORY_OF_OBJ(list_to) == MATCHOBJECT ||
        CATEGORY_OF_OBJ(list_to) == CALLOBJECT ||
        CATEGORY_OF_OBJ(list_to) == REFLISTOBJECT) {
      if (list_from != list_to) {
        if (TEMP_OBJECT(list_from)) {
          free_list(take_reflist(list_to));
          list_to->value.listValue = take_reflist(list_from);
          list_from->value.listValue = NULL;
        } else {
          help_list = copy_list(take_reflist(list_from), &err_info);
          if (err_info != OKAY_NO_ERROR) {
            return raise_exception(SYS_MEM_EXCEPTION);
          } else {
            free_list(take_reflist(list_to));
            list_to->value.listValue = help_list;
          } /* if */
        } /* if */
      } /* if */
    } else {
      run_error(REFLISTOBJECT, list_to);
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* rfl_set_value */



/**
 *  Get a sublist from 'list/arg_1' beginning at a 'start/arg_3' position.
 *  The first element in a 'ref_list' has the position 1.
 *  @return the sublist beginning at the 'start/arg_3' position.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType rfl_tail (listType arguments)

  {
    objectType list;
    intType start;
    intType number;
    listType list_element;
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* rfl_tail */
    isit_reflist(arg_1(arguments));
    isit_int(arg_3(arguments));
    list = arg_1(arguments);
    start = take_int(arg_3(arguments));
    list_element = take_reflist(list);
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
      return bld_reflist_temp(result);
    } /* if */
  } /* rfl_tail */



objectType rfl_trace (listType arguments)

  { /* rfl_trace */
    isit_reflist(arg_1(arguments));
    prot_list(take_reflist(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* rfl_trace */



objectType rfl_value (listType arguments)

  {
    objectType obj_arg;
    errInfoType err_info = OKAY_NO_ERROR;
    listType result;

  /* rfl_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (unlikely(obj_arg == NULL ||
                 (CATEGORY_OF_OBJ(obj_arg) != MATCHOBJECT &&
                  CATEGORY_OF_OBJ(obj_arg) != CALLOBJECT &&
                  CATEGORY_OF_OBJ(obj_arg) != REFLISTOBJECT))) {
      logError(printf("rfl_value(");
               trace1(obj_arg);
               printf("): Category is not MATCH-, CALL- or REFLISTOBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      result = copy_list(take_reflist(obj_arg), &err_info);
      if (err_info != OKAY_NO_ERROR) {
        return raise_exception(SYS_MEM_EXCEPTION);
      } /* if */
    } /* if */
    return bld_reflist_temp(result);
  } /* rfl_value */
