/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  Module: Compiler data reflection                                */
/*  File: seed7/src/rfl_data.c                                      */
/*  Changes: 1991-1995, 2007, 2008  Thomas Mertes                   */
/*  Content: Primitive actions for the ref_list type.               */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "listutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "rfl_data.h"



#ifdef ANSI_C

void rflAppend (listtype *list_to, listtype list_from)
#else

void rflAppend (list_to, list_from)
listtype *list_to;
listtype list_from;
#endif

  {
    listtype list1_end;

  /* rflAppend */
    if (*list_to != NULL) {
      list1_end = *list_to;
      while (list1_end->next != NULL) {
        list1_end = list1_end->next;
      } /* while */
      list1_end->next = list_from;
    } else {
      *list_to = list_from;
    } /* if */
  } /* rflAppend */



#ifdef ANSI_C

listtype rflCat (listtype list1, listtype list2)
#else

listtype rflCat (list1, list2)
listtype list1;
listtype list2;
#endif

  {
    listtype result;

  /* rflCat */
    if (list1 != NULL) {
      result = list1;
      while (list1->next != NULL) {
        list1 = list1->next;
      } /* while */
      list1->next = list2;
    } else {
      result = list2;
    } /* if */
    return(result);
  } /* rflCat */



#ifdef ANSI_C

void rflCpy (listtype *list_to, listtype list_from)
#else

void rflCpy (list_to, list_from)
listtype *list_to;
listtype list_from;
#endif

  {
    listtype help_list;
    errinfotype err_info = OKAY_NO_ERROR;

  /* rflCpy */
    if (list_from != *list_to) {
      help_list = copy_list(list_from, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
      } else {
        emptylist(*list_to);
        *list_to = help_list;
      } /* if */
    } /* if */
  } /* rflCpy */



#ifdef ANSI_C

listtype rflCreate (listtype list_from)
#else

listtype rflCreate (list_from)
listtype list_from;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* rflCreate */
    result = copy_list(list_from, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } /* if */
    return(result);
  } /* rflCreate */



#ifdef ANSI_C

void rflDestr (listtype old_list)
#else

void rflDestr (old_list)
listtype old_list;
#endif

  { /* rflDestr */
    emptylist(old_list);
  } /* rflDestr */



#ifdef ANSI_C

booltype rflElem (objecttype searched_object, listtype list_element)
#else

booltype rflElem (searched_object, list_element)
objecttype searched_object;
listtype list_element;
#endif

  { /* rflElem */
    /* printf("rflElem(%lx, %lx)\n",
        (unsigned long) searched_object, (unsigned long) list_element); */
    while (list_element != NULL && list_element->obj != searched_object) {
      list_element = list_element->next;
    } /* while */
    if (list_element != NULL) {
      return(TRUE);
    } else {
      return(FALSE);
    } /* if */
  } /* rflElem */



#ifdef ANSI_C

void rflElemcpy (listtype list, inttype position, objecttype elem)
#else

void rflElemcpy (list, position, elem)
listtype list;
inttype position;
objecttype elem;
#endif

  {
    inttype number;

  /* rflElemcpy */
    if (position >= 1) {
      number = 1;
      while (number < position && list != NULL) {
        number++;
        list = list->next;
      } /* while */
      if (list != NULL) {
        list->obj = elem;
      } else {
        raise_error(RANGE_ERROR);
      } /* if */
    } else {
      raise_error(RANGE_ERROR);
    } /* if */
  } /* rflElemcpy */



#ifdef ANSI_C

booltype rflEq (listtype list1, listtype list2)
#else

booltype rflEq (list1, list2)
listtype list1;
listtype list2;
#endif

  {
    booltype result;

  /* rflEq */
    while (list1 != NULL && list2 != NULL &&
        list1->obj == list2->obj) {
      list1 = list1->next;
      list2 = list2->next;
    } /* while */
    if (list1 == NULL && list2 == NULL) {
      result = TRUE;
    } else {
      result = FALSE;
    } /* if */
    return(result);
  } /* rflEq */



#ifdef ANSI_C

listtype rflHead (listtype list, inttype stop)
#else

listtype rflHead (list, stop)
listtype list;
inttype stop;
#endif

  {
    inttype number;
    listtype stop_element;
    listtype saved_list_rest;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* rflHead */
    if (stop >= 1) {
      number = 1;
      stop_element = list;
      while (number < stop && stop_element != NULL) {
        number++;
        stop_element = stop_element->next;
      } /* while */
      if (stop_element != NULL) {
        saved_list_rest = stop_element->next;
        stop_element->next = NULL;
        result = copy_list(list, &err_info);
        stop_element->next = saved_list_rest;
      } else {
        result = copy_list(list, &err_info);
      } /* if */
      if (err_info != OKAY_NO_ERROR) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } /* if */
    } else {
      result = NULL;
    } /* if */
    return(result);
  } /* rflHead */



#ifdef ANSI_C

objecttype rflIdx (listtype list, inttype position)
#else

objecttype rflIdx (list, position)
listtype list;
inttype position;
#endif

  {
    inttype number;
    objecttype result;

  /* rflIdx */
    if (position >= 1) {
      number = 1;
      while (number < position && list != NULL) {
        number++;
        list = list->next;
      } /* while */
      if (list != NULL) {
        result = list->obj;
      } else {
        raise_error(RANGE_ERROR);
        result = 0;
      } /* if */
    } else {
      raise_error(RANGE_ERROR);
      result = 0;
    } /* if */
    return(result);
  } /* rflIdx */



#ifdef ANSI_C

void rflIncl (listtype *list, objecttype elem)
#else

void rflIncl (list, elem)
listtype *list;
objecttype elem;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;

  /* rflIncl */
    incl_list(list, elem, &err_info);
    if (err_info != OKAY_NO_ERROR) {
      raise_error(MEMORY_ERROR);
    } /* if */
  } /* rflIncl */



#ifdef ANSI_C

inttype rflLng (listtype list)
#else

inttype rflLng (list)
listtype list;
#endif

  {
    inttype result;

  /* rflLng */
    result = 0;
    while (list != NULL) {
      list = list->next;
      result++;
    } /* while */
    return(result);
  } /* rflLng */



#ifdef ANSI_C

listtype rflMklist (objecttype elem)
#else

listtype rflMklist (elem)
objecttype elem;
#endif

  {
    listtype result;

  /* rflMklist */
    if (ALLOC_L_ELEM(result)) {
      result->next = NULL;
      result->obj = elem;
    } else {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } /* if */
    return(result);
  } /* rflMklist */



#ifdef ANSI_C

booltype rflNe (listtype list1, listtype list2)
#else

booltype rflNe (list1, list2)
listtype list1;
listtype list2;
#endif

  {
    booltype result;

  /* rflNe */
    while (list1 != NULL && list2 != NULL &&
        list1->obj == list2->obj) {
      list1 = list1->next;
      list2 = list2->next;
    } /* while */
    if (list1 != NULL || list2 != NULL) {
      result = TRUE;
    } else {
      result = FALSE;
    } /* if */
    return(result);
  } /* rflNe */



#ifdef ANSI_C

inttype rflPos (listtype list_element, objecttype searched_object)
#else

inttype rflPos (list_element, searched_object)
listtype list_element;
objecttype searched_object;
#endif

  {
    inttype result;

  /* rflPos */
    result = 1;
    while (list_element != NULL &&
        list_element->obj != searched_object) {
      list_element = list_element->next;
      result++;
    } /* while */
    if (list_element == NULL) {
      result = 0;
    } /* if */
    return(result);
  } /* rflPos */



#ifdef ANSI_C

inttype rflIpos (listtype list_element, objecttype searched_object,
    const inttype from_index)
#else

inttype rflIpos (list_element, searched_object, from_index)
listtype list_element;
objecttype searched_object;
inttype from_index;
#endif

  {
    inttype result;

  /* rflIpos */
    result = 1;
    while (list_element != NULL && result < from_index) {
      list_element = list_element->next;
      result++;
    } /* while */
    while (list_element != NULL &&
        list_element->obj != searched_object) {
      list_element = list_element->next;
      result++;
    } /* while */
    if (list_element == NULL) {
      result = 0;
    } /* if */
    return(result);
  } /* rflIpos */



#ifdef ANSI_C

listtype rflRange (listtype list, inttype start, inttype stop)
#else

listtype rflRange (list, start, stop)
listtype list;
inttype start;
inttype stop;
#endif

  {
    inttype number;
    listtype start_element;
    listtype stop_element;
    listtype saved_list_rest;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* rflRange */
    number = 1;
    start_element = list;
    while (number < start && start_element != NULL) {
      number++;
      start_element = start_element->next;
    } /* while */
    if (start_element != NULL && stop >= start) {
      stop_element = start_element;
      while (number < stop && stop_element != NULL) {
        number++;
        stop_element = stop_element->next;
      } /* while */
      if (stop_element != NULL) {
        saved_list_rest = stop_element->next;
        stop_element->next = NULL;
        result = copy_list(start_element, &err_info);
        stop_element->next = saved_list_rest;
      } else {
        result = copy_list(start_element, &err_info);
      } /* if */
    } else {
      result = NULL;
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } /* if */
    return(result);
  } /* rflRange */



#ifdef ANSI_C

listtype rflTail (listtype list, inttype start)
#else

listtype rflTail (list, start)
listtype list;
inttype start;
#endif

  {
    inttype number;
    listtype list_element;
    errinfotype err_info = OKAY_NO_ERROR;
    listtype result;

  /* rflTail */
    list_element = list;
    if (start > 1 && list_element != NULL) {
      number = 2;
      while (number < start && list_element->next != NULL) {
        number++;
        list_element = list_element->next;
      } /* while */
      if (number >= start) {
        result = copy_list(list_element->next, &err_info);
      } else {
        result = NULL;
      } /* if */
    } else {
      result = copy_list(list_element, &err_info);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } /* if */
    return(result);
  } /* rflTail */
