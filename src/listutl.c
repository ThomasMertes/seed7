/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000, 2008, 2013  Thomas Mertes            */
/*                2015 - 2018, 2021  Thomas Mertes                  */
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
/*  Module: General                                                 */
/*  File: seed7/src/listutl.c                                       */
/*  Changes: 1990 - 1994, 2002, 2008, 2013  Thomas Mertes           */
/*           2015 - 2018, 2021  Thomas Mertes                       */
/*  Content: Procedures to maintain objects of type listType.       */
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

#undef EXTERN
#define EXTERN
#include "listutl.h"



#if WITH_LIST_FREELIST
void free_list (listType list)

  {
    register listType list_end;

  /* free_list */
    if (list != NULL) {
      list_end = list;
      while (list_end->next != NULL) {
        list_end = list_end->next;
      } /* while */
      list_end->next = flist.list_elems;
      flist.list_elems = list;
    } /* if */
  } /* free_list */

#else



void free_list (listType list)

  {
    register listType list_elem;
    register listType old_elem;

  /* free_list */
    list_elem = list;
    while (list_elem != NULL) {
      old_elem = list_elem;
      list_elem = list_elem->next;
      FREE_L_ELEM(old_elem);
    } /* while */
  } /* free_list */
#endif



memSizeType list_length (const_listType list)

  {
    memSizeType length = 0;

  /* list_length */
    while (list != NULL) {
      list = list->next;
      length++;
    } /* while */
    return length;
  } /* list_length */



listType *append_element_to_list (listType *list_insert_place, objectType object,
    errInfoType *err_info)

  {
    listType help_element;

  /* append_element_to_list */
    if (unlikely(!ALLOC_L_ELEM(help_element))) {
      *err_info = MEMORY_ERROR;
      return list_insert_place;
    } else {
      help_element->next = NULL;
      help_element->obj = object;
      *list_insert_place = help_element;
      return &help_element->next;
    } /* if */
  } /* append_element_to_list */



static objectType copy_expression2 (objectType object_from, errInfoType *err_info)

  {
    register listType list_from_elem;
    register listType list_to_elem;
    register listType new_elem;
    objectType object_to;

  /* copy_expression2 */
    if (unlikely(!ALLOC_OBJECT(object_to))) {
      *err_info = MEMORY_ERROR;
    } else {
      object_to->type_of = object_from->type_of;
      object_to->descriptor.property = object_from->descriptor.property;
      object_to->value.listValue = NULL;
      INIT_CATEGORY_OF_OBJ(object_to, CATEGORY_OF_OBJ(object_from));
      SET_ANY_FLAG(object_to, HAS_POSINFO(object_from));
      list_from_elem = object_from->value.listValue;
      if (list_from_elem != NULL) {
        if (likely(ALLOC_L_ELEM(list_to_elem))) {
          object_to->value.listValue = list_to_elem;
          list_to_elem->obj = copy_expression(list_from_elem->obj, err_info);
          list_from_elem = list_from_elem->next;
          while (list_from_elem != NULL) {
            if (likely(ALLOC_L_ELEM(new_elem))) {
              list_to_elem->next = new_elem;
              list_to_elem = new_elem;
              if (CATEGORY_OF_OBJ(list_from_elem->obj) == EXPROBJECT ||
                  CATEGORY_OF_OBJ(list_from_elem->obj) == CALLOBJECT ||
                  CATEGORY_OF_OBJ(list_from_elem->obj) == MATCHOBJECT ||
                  CATEGORY_OF_OBJ(list_from_elem->obj) == LISTOBJECT) {
                list_to_elem->obj = copy_expression(list_from_elem->obj, err_info);
              } else {
                list_to_elem->obj = list_from_elem->obj;
              } /* if */
              list_from_elem = list_from_elem->next;
            } else {
              *err_info = MEMORY_ERROR;
              list_from_elem = NULL;
            } /* if */
          } /* while */
          list_to_elem->next = NULL;
        } else {
          *err_info = MEMORY_ERROR;
        } /* if */
      } /* if */
    } /* if */
    return object_to;
  } /* copy_expression2 */



objectType copy_expression (objectType object_from, errInfoType *err_info)

  {
    objectType object_to;

  /* copy_expression */
    if (CATEGORY_OF_OBJ(object_from) == EXPROBJECT ||
        CATEGORY_OF_OBJ(object_from) == CALLOBJECT ||
        CATEGORY_OF_OBJ(object_from) == MATCHOBJECT ||
        CATEGORY_OF_OBJ(object_from) == LISTOBJECT) {
      object_to = copy_expression2(object_from, err_info);
    } else {
      object_to = object_from;
    } /* if */
    return object_to;
  } /* copy_expression */



void free_expression (objectType object)

  {
    listType list_elem;

  /* free_expression */
    logFunction(printf("free_expression\n"););
    if (object != NULL) {
      switch (CATEGORY_OF_OBJ(object)) {
        case CALLOBJECT:
        case MATCHOBJECT:
        case EXPROBJECT:
        case LISTOBJECT:
          /* printf("free_expression: \n");
          trace1(object);
          printf("\n"); */
          list_elem = object->value.listValue;
          if (list_elem != NULL) {
            while (list_elem->next != NULL) {
              free_expression(list_elem->obj);
              list_elem = list_elem->next;
            } /* while */
            free_expression(list_elem->obj);
            free_list2(object->value.listValue, list_elem);
          } /* if */
          FREE_OBJECT(object);
          break;
        default:
          break;
      } /* switch */
    } /* if */
  } /* free_expression */



void concat_lists (listType *list1, listType list2)

  {
    listType help_element;

  /* concat_lists */
    logFunction(printf("concat_lists(" FMT_U_MEM ", " FMT_U_MEM ") \n",
                       (memSizeType) *list1, (memSizeType) list2););
    /* prot_list(list2);
    printf("\n"); */
    if (*list1 == NULL) {
      *list1 = list2;
    } else {
      if (list2 != NULL) {
        help_element = *list1;
        while (help_element->next != NULL) {
          help_element = help_element->next;
        } /* while */
        help_element->next = list2;
      } /* if */
    } /* if */
    logFunction(printf("concat_lists -->\n"););
  } /* concat_lists */



void incl_list (listType *list, objectType element_object,
    errInfoType *err_info)

  {
    listType help_element;

  /* incl_list */
    logFunction(printf("incl_list(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) *list,
                       (memSizeType) element_object););
    if (unlikely(!ALLOC_L_ELEM(help_element))) {
      *err_info = MEMORY_ERROR;
    } else {
      help_element->next = *list;
      help_element->obj = element_object;
      *list = help_element;
    } /* if */
    logFunction(printf("incl_list --> " FMT_U_MEM "\n",
                       (memSizeType) *list););
  } /* incl_list */



void excl_list (listType *list, const_objectType elementobject)

  {
    listType listelement, disposeelement;
    boolType found;

  /* excl_list */
    logFunction(printf("excl_list\n"););
    if (*list != NULL) {
      listelement = *list;
      if (listelement->obj == elementobject) {
        *list = listelement->next;
        FREE_L_ELEM(listelement);
      } else {
        found = FALSE;
        while ((listelement->next != NULL) && !found) {
          if (listelement->next->obj == elementobject) {
            found = TRUE;
          } else {
            listelement = listelement->next;
          } /* if */
        } /* while */
        if (found) {
          disposeelement = listelement->next;
          listelement->next = disposeelement->next;
          FREE_L_ELEM(disposeelement);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("excl_list -->\n"););
  } /* excl_list */



void pop_list (listType *list)

  {
    listType listelement;

  /* pop_list */
    logFunction(printf("excl_list\n"););
    if (*list != NULL) {
      listelement = *list;
      *list = listelement->next;
      FREE_L_ELEM(listelement);
    } /* if */
    logFunction(printf("excl_list -->\n"););
  } /* pop_list */



void replace_list_elem (listType list, const_objectType elem1,
    objectType elem2)

  { /* replace_list_elem */
    logFunction(printf("replace_list_elem\n"););
    while (list != NULL) {
      if (list->obj == elem1) {
        list->obj = elem2;
        list = NULL;
      } else {
        list = list->next;
      } /* if */
    } /* while */
    logFunction(printf("replace_list_elem -->\n"););
  } /* replace_list_elem */



/**
 *  Copy the given list 'list_from'.
 *  For performance reasons list elements are taken directly
 *  from the free list (flist.list_elems).
 *  @param list_from Possibly empty list to be copied.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if a memory allocation failed.
 *  @return the copied list, or
 *          NULL if an error occurred.
 */
listType copy_list (const_listType list_from, errInfoType *err_info)

  {
    listType help_element;
    listType list_to;

  /* copy_list */
    logFunction(printf("copy_list\n"););
    if (list_from != NULL) {
      if (flist.list_elems != NULL) {
        help_element = flist.list_elems;
        list_to = help_element;
        help_element->obj = list_from->obj;
        list_from = list_from->next;
        while (list_from != NULL && help_element->next != NULL) {
          help_element = help_element->next;
          help_element->obj = list_from->obj;
          list_from = list_from->next;
        } /* while */
        flist.list_elems = help_element->next;
      } else {
        if (unlikely(!HEAP_L_E(list_to, listRecord))) {
          logError(printf("copy_list: malloc failed.\n"););
          *err_info = MEMORY_ERROR;
        } else {
          help_element = list_to;
          help_element->obj = list_from->obj;
          list_from = list_from->next;
        } /* if */
      } /* if */
      if (list_to != NULL) {
        while (list_from != NULL) {
          if (unlikely(!HEAP_L_E(help_element->next, listRecord))) {
            free_list(list_to);
            logError(printf("copy_list: malloc failed.\n"););
            *err_info = MEMORY_ERROR;
            return NULL;
          } else {
            help_element = help_element->next;
            help_element->obj = list_from->obj;
            list_from = list_from->next;
          } /* if */
        } /* while */
        help_element->next = NULL;
      } /* if */
    } else {
      list_to = NULL;
    } /* if */
    logFunction(printf("copy_list -->\n"););
    return list_to;
  } /* copy_list */



/**
 *  Generate a list with the elements of the given array 'arr_from'.
 *  For performance reasons list elements are taken directly
 *  from the free list (flist.list_elems).
 *  @param arr_from Possibly empty array.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if a memory allocation failed.
 *  @return the generated list with elements from 'arr_from'.
 */
listType array_to_list (arrayType arr_from, errInfoType *err_info)

  {
    listType help_element;
    memSizeType arr_from_size;
    memSizeType position;
    listType list_to;

  /* array_to_list */
    logFunction(printf("array_to_list(array[" FMT_D ".." FMT_D "])\n",
                       arr_from->min_position, arr_from->max_position););
    arr_from_size = arraySize(arr_from);
    if (arr_from_size != 0) {
      if (flist.list_elems != NULL) {
        help_element = flist.list_elems;
        list_to = help_element;
        help_element->obj = &arr_from->arr[0];
        position = 1;
        while (position < arr_from_size && help_element->next != NULL) {
          help_element = help_element->next;
          help_element->obj = &arr_from->arr[position];
          position++;
        } /* while */
        flist.list_elems = help_element->next;
      } else {
        if (unlikely(!HEAP_L_E(list_to, listRecord))) {
          logError(printf("array_to_list: malloc failed.\n"););
          *err_info = MEMORY_ERROR;
        } else {
          help_element = list_to;
          help_element->obj = &arr_from->arr[0];
          position = 1;
        } /* if */
      } /* if */
      if (list_to != NULL) {
        while (position < arr_from_size) {
          if (unlikely(!HEAP_L_E(help_element->next, listRecord))) {
            free_list(list_to);
            logError(printf("array_to_list: malloc failed.\n"););
            *err_info = MEMORY_ERROR;
            return NULL;
          } else {
            help_element = help_element->next;
            help_element->obj = &arr_from->arr[position];
            position++;
          } /* if */
        } /* while */
        help_element->next = NULL;
      } /* if */
    } else {
      list_to = NULL;
    } /* if */
    logFunction(printf("array_to_list -->\n"););
    return list_to;
  } /* array_to_list */



/**
 *  Generate a list with the elements of the given struct 'stru_from'.
 *  For performance reasons list elements are taken directly
 *  from the free list (flist.list_elems).
 *  @param stru_from Possibly empty struct.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if a memory allocation failed.
 *  @return the generated list with elements from 'stru_from'.
 */
listType struct_to_list (structType stru_from, errInfoType *err_info)

  {
    listType help_element;
    memSizeType position;
    listType list_to;

  /* struct_to_list */
    logFunction(printf("struct_to_list\n"););
    if (stru_from->size != 0 && *err_info == OKAY_NO_ERROR) {
      if (flist.list_elems != NULL) {
        help_element = flist.list_elems;
        list_to = help_element;
        help_element->obj = &stru_from->stru[0];
        position = 1;
        while (position < stru_from->size && help_element->next != NULL) {
          help_element = help_element->next;
          help_element->obj = &stru_from->stru[position];
          position++;
        } /* while */
        flist.list_elems = help_element->next;
      } else {
        if (unlikely(!HEAP_L_E(list_to, listRecord))) {
          logError(printf("struct_to_list: malloc failed.\n"););
          *err_info = MEMORY_ERROR;
        } else {
          help_element = list_to;
          help_element->obj = &stru_from->stru[0];
          position = 1;
        } /* if */
      } /* if */
      if (list_to != NULL) {
        while (position < stru_from->size) {
          if (unlikely(!HEAP_L_E(help_element->next, listRecord))) {
            free_list(list_to);
            logError(printf("struct_to_list: malloc failed.\n"););
            *err_info = MEMORY_ERROR;
            return NULL;
          } else {
            help_element = help_element->next;
            help_element->obj = &stru_from->stru[position];
            position++;
          } /* if */
        } /* while */
        help_element->next = NULL;
      } /* if */
    } else {
      list_to = NULL;
    } /* if */
    logFunction(printf("struct_to_list -->\n"););
    return list_to;
  } /* struct_to_list */



static void helem_data_to_list (listType *list_insert_place,
    hashElemType helem, errInfoType *err_info)

  { /* helem_data_to_list */
    do {
      incl_list(list_insert_place, &helem->data, err_info);
      if (helem->next_less != NULL && *err_info == OKAY_NO_ERROR) {
        helem_data_to_list(list_insert_place, helem->next_less, err_info);
      } /* if */
      helem = helem->next_greater;
    } while (helem != NULL && *err_info == OKAY_NO_ERROR);
  } /* helem_data_to_list */



listType hash_data_to_list (hashType hash, errInfoType *err_info)

  {
    unsigned int number;
    hashElemType *table;
    listType result;

  /* hash_data_to_list */
    result = NULL;
    if (hash->size != 0) {
      number = hash->table_size;
      table = hash->table;
      while (number != 0 && *err_info == OKAY_NO_ERROR) {
        do {
          number--;
        } while (number != 0 && table[number] == NULL);
        if (number != 0 || table[number] != NULL) {
          helem_data_to_list(&result, table[number], err_info);
        } /* if */
      } /* while */
    } /* if */
    return result;
  } /* hash_data_to_list */



static void helem_key_to_list (listType *list_insert_place,
    hashElemType helem, errInfoType *err_info)

  { /* helem_key_to_list */
    do {
      incl_list(list_insert_place, &helem->key, err_info);
      if (helem->next_less != NULL && *err_info == OKAY_NO_ERROR) {
        helem_key_to_list(list_insert_place, helem->next_less, err_info);
      } /* if */
      helem = helem->next_greater;
    } while (helem != NULL && *err_info == OKAY_NO_ERROR);
  } /* helem_key_to_list */



listType hash_keys_to_list (hashType hash, errInfoType *err_info)

  {
    unsigned int number;
    hashElemType *table;
    listType result;

  /* hash_keys_to_list */
    result = NULL;
    if (hash->size != 0) {
      number = hash->table_size;
      table = hash->table;
      while (number != 0 && *err_info == OKAY_NO_ERROR) {
        do {
          number--;
        } while (number != 0 && table[number] == NULL);
        if (number != 0 || table[number] != NULL) {
          helem_key_to_list(&result, table[number], err_info);
        } /* if */
      } /* while */
    } /* if */
    return result;
  } /* hash_keys_to_list */
