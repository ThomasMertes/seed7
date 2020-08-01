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
/*  Module: General                                                 */
/*  File: seed7/src/listutl.c                                       */
/*  Changes: 1990, 1991, 1992, 1993, 1994, 2002  Thomas Mertes      */
/*  Content: Procedures to maintain objects of type listtype.       */
/*                                                                  */
/********************************************************************/

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



#ifdef ANSI_C

void emptylist (listtype list)
#else

void emptylist (list)
listtype list;
#endif

  {
    register listtype list_end;

  /* emptylist */
    if (list != NULL) {
      list_end = list;
      while (list_end->next != NULL) {
        list_end = list_end->next;
      } /* while */
      list_end->next = flist.list_elems;
      flist.list_elems = list;
    } /* if */
  } /* emptylist */



#ifdef ANSI_C

listtype *append_element_to_list (listtype *list_insert_place, objecttype object,
    errinfotype *err_info)
#else

listtype *append_element_to_list (list_insert_place, object, err_info)
listtype *list_insert_place;
objecttype object;
errinfotype *err_info;
#endif

  {
    listtype help_element;

  /* append_element_to_list */
    if (ALLOC_L_ELEM(help_element)) {
      help_element->next = NULL;
      help_element->obj = object;
      *list_insert_place = help_element;
      return &help_element->next;
    } else {
      *err_info = MEMORY_ERROR;
      return NULL;
    } /* if */
  } /* append_element_to_list */



#ifdef ANSI_C

void copy_expression (objecttype object_from,
    objecttype *object_to, errinfotype *err_info)
#else

void copy_expression (object_from, object_to, err_info)
objecttype object_from;
objecttype *object_to;
errinfotype *err_info;
#endif

  {
    listtype list_from_elem;
    listtype *list_insert_place;
    objecttype object_to_elem;

  /* copy_expression */
    if (CATEGORY_OF_OBJ(object_from) == EXPROBJECT) {
      if (!ALLOC_OBJECT(*object_to)) {
        *err_info = MEMORY_ERROR;
      } else {
        (*object_to)->type_of = object_from->type_of;
        (*object_to)->descriptor.property = object_from->descriptor.property;
        (*object_to)->value.listvalue = NULL;
        list_insert_place = &(*object_to)->value.listvalue;
        INIT_CATEGORY_OF_OBJ((*object_to), CATEGORY_OF_OBJ(object_from));
        SET_ANY_FLAG((*object_to), HAS_POSINFO(object_from));
        list_from_elem = object_from->value.listvalue;
        while (list_from_elem != NULL && *err_info == OKAY_NO_ERROR) {
          copy_expression(list_from_elem->obj, &object_to_elem, err_info);
          list_insert_place = append_element_to_list(list_insert_place,
              object_to_elem, err_info);
          list_from_elem = list_from_elem->next;
        } /* while */
      } /* if */
    } else {
      *object_to = object_from;
    } /* if */
  } /* copy_expression */



#ifdef ANSI_C

void concat_lists (listtype *list1, listtype list2)
#else

void concat_lists (list1, list2)
listtype *list1;
listtype list2;
#endif

  {
    listtype help_element;

  /* concat_lists */
#ifdef TRACE_LIST
    printf("BEGIN concat_lists\n");
#endif
/*  printf("%ld %ld", *list1, list2);
    prot_list(list2);
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
#ifdef TRACE_LIST
    printf("END concat_lists\n");
#endif
  } /* concat_lists */



#ifdef ANSI_C

void incl_list (listtype *list, objecttype element_object,
    errinfotype *err_info)
#else

void incl_list (list, element_object, err_info)
listtype *list;
objecttype element_object;
errinfotype *err_info;
#endif

  {
    listtype help_element;

  /* incl_list */
#ifdef TRACE_RUNLIST
    printf("BEGIN incl_list\n");
#endif
    if (!ALLOC_L_ELEM(help_element)) {
      *err_info = MEMORY_ERROR;
    } else {
      help_element->next = *list;
      help_element->obj = element_object;
      *list = help_element;
    } /* if */
#ifdef TRACE_RUNLIST
    printf("END incl_list\n");
#endif
  } /* incl_list */



#ifdef ANSI_C

void excl_list (listtype *list, const_objecttype elementobject)
#else

void excl_list (list, elementobject)
listtype *list;
objecttype elementobject;
#endif

  {
    listtype listelement, disposeelement;
    booltype found;

  /* excl_list */
#ifdef TRACE_RUNLIST
    printf("BEGIN excl_list\n");
#endif
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
#ifdef TRACE_RUNLIST
    printf("END excl_list\n");
#endif
  } /* excl_list */



#ifdef ANSI_C

void pop_list (listtype *list)
#else

void pop_list (list)
listtype *list;
#endif

  {
    listtype listelement;

  /* pop_list */
#ifdef TRACE_RUNLIST
    printf("BEGIN excl_list\n");
#endif
    if (*list != NULL) {
      listelement = *list;
      *list = listelement->next;
      FREE_L_ELEM(listelement);
    } /* if */
#ifdef TRACE_RUNLIST
    printf("END excl_list\n");
#endif
  } /* pop_list */



#ifdef ANSI_C

void replace_list_elem (listtype list, const_objecttype elem1,
    objecttype elem2)
#else

void replace_list_elem (list, elem1, elem2)
listtype list;
objecttype elem1;
objecttype elem2;
#endif

  { /* replace_list_elem */
#ifdef TRACE_RUNLIST
    printf("BEGIN replace_list_elem\n");
#endif
    while (list != NULL) {
      if (list->obj == elem1) {
        list->obj = elem2;
        list = NULL;
      } else {
        list = list->next;
      } /* if */
    } /* while */
#ifdef TRACE_RUNLIST
    printf("END replace_list_elem\n");
#endif
  } /* replace_list_elem */



#ifdef ANSI_C

listtype copy_list (const_listtype list_from, errinfotype *err_info)
#else

listtype copy_list (list_from, err_info)
listtype list_from;
errinfotype *err_info;
#endif

  {
    listtype help_element;
    listtype list_to;

  /* copy_list */
#ifdef TRACE_RUNLIST
    printf("BEGIN copy_list\n");
#endif
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
        if (!HEAP_L_E(help_element, listrecord)) {
          list_to = NULL;
          *err_info = MEMORY_ERROR;
        } else {
          list_to = help_element;
          help_element->obj = list_from->obj;
          list_from = list_from->next;
        } /* if */
      } /* if */
      if (*err_info == OKAY_NO_ERROR) {
        while (list_from != NULL && *err_info == OKAY_NO_ERROR) {
          if (!HEAP_L_E(help_element->next, listrecord)) {
            *err_info = MEMORY_ERROR;
          } else {
            help_element = help_element->next;
            help_element->obj = list_from->obj;
            list_from = list_from->next;
          } /* if */
        } /* while */
        help_element->next = NULL;
      } /* if */
      if (*err_info != OKAY_NO_ERROR) {
        emptylist(list_to);
        list_to = NULL;
      } /* if */
    } else {
      list_to = NULL;
    } /* if */
#ifdef TRACE_RUNLIST
    printf("END copy_list\n");
#endif
    return list_to;
  } /* copy_list */



#ifdef ANSI_C

listtype array_to_list (arraytype arr_from, errinfotype *err_info)
#else

listtype array_to_list (arr_from, err_info)
arraytype arr_from;
errinfotype *err_info;
#endif

  {
    listtype help_element;
    memsizetype arr_from_size;
    memsizetype position;
    listtype list_to;

  /* array_to_list */
#ifdef TRACE_RUNLIST
    printf("BEGIN array_to_list\n");
#endif
    arr_from_size = (uinttype) (arr_from->max_position - arr_from->min_position + 1);
    if (arr_from_size != 0 && *err_info == OKAY_NO_ERROR) {
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
        if (!HEAP_L_E(help_element, listrecord)) {
          list_to = NULL;
          *err_info = MEMORY_ERROR;
        } else {
          list_to = help_element;
          help_element->obj = &arr_from->arr[0];
          position = 1;
        } /* if */
      } /* if */
      if (list_to != NULL) {
        while (position < arr_from_size && list_to != NULL) {
          if (!HEAP_L_E(help_element->next, listrecord)) {
            emptylist(list_to);
            list_to = NULL;
            *err_info = MEMORY_ERROR;
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
#ifdef TRACE_RUNLIST
    printf("END array_to_list\n");
#endif
    return list_to;
  } /* array_to_list */



#ifdef ANSI_C

listtype struct_to_list (structtype stru_from, errinfotype *err_info)
#else

listtype struct_to_list (stru_from, err_info)
structtype stru_from;
errinfotype *err_info;
#endif

  {
    listtype help_element;
    memsizetype position;
    listtype list_to;

  /* struct_to_list */
#ifdef TRACE_RUNLIST
    printf("BEGIN struct_to_list\n");
#endif
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
        if (!HEAP_L_E(help_element, listrecord)) {
          list_to = NULL;
          *err_info = MEMORY_ERROR;
        } else {
          list_to = help_element;
          help_element->obj = &stru_from->stru[0];
          position = 1;
        } /* if */
      } /* if */
      if (list_to != NULL) {
        while (position < stru_from->size && list_to != NULL) {
          if (!HEAP_L_E(help_element->next, listrecord)) {
            emptylist(list_to);
            list_to = NULL;
            *err_info = MEMORY_ERROR;
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
#ifdef TRACE_RUNLIST
    printf("END struct_to_list\n");
#endif
    return list_to;
  } /* struct_to_list */



#ifdef ANSI_C

static void helem_data_to_list (listtype *list_insert_place,
    helemtype helem, errinfotype *err_info)
#else

static void helem_data_to_list (list_insert_place, helem, err_info)
listtype *list_insert_place;
helemtype helem;
errinfotype *err_info;
#endif

  { /* helem_data_to_list */
    if (helem != NULL && *err_info == OKAY_NO_ERROR) {
      incl_list(list_insert_place, &helem->data, err_info);
      helem_data_to_list(list_insert_place, helem->next_less, err_info);
      helem_data_to_list(list_insert_place, helem->next_greater, err_info);
    } /* if */
  } /* helem_data_to_list */



#ifdef ANSI_C

listtype hash_data_to_list (hashtype hash, errinfotype *err_info)
#else

listtype hash_data_to_list (hash, err_info)
hashtype hash;
errinfotype *err_info;
#endif

  {
    unsigned int number;
    helemtype *helem;
    listtype result;

  /* hash_data_to_list */
    result = NULL;
    if (hash != NULL) {
      number = hash->table_size;
      helem = &hash->table[0];
      while (number > 0 && *err_info == OKAY_NO_ERROR) {
        helem_data_to_list(&result, *helem, err_info);
        number--;
        helem++;
      } /* while */
    } /* if */
    return result;
  } /* hash_data_to_list */



#ifdef ANSI_C

static void helem_key_to_list (listtype *list_insert_place,
    helemtype helem, errinfotype *err_info)
#else

static void helem_key_to_list (list_insert_place, helem, err_info)
listtype *list_insert_place;
helemtype helem;
errinfotype *err_info;
#endif

  { /* helem_key_to_list */
    if (helem != NULL && *err_info == OKAY_NO_ERROR) {
      incl_list(list_insert_place, &helem->key, err_info);
      helem_key_to_list(list_insert_place, helem->next_less, err_info);
      helem_key_to_list(list_insert_place, helem->next_greater, err_info);
    } /* if */
  } /* helem_key_to_list */



#ifdef ANSI_C

listtype hash_key_to_list (hashtype hash, errinfotype *err_info)
#else

listtype hash_key_to_list (hash, err_info)
hashtype hash;
errinfotype *err_info;
#endif

  {
    unsigned int number;
    helemtype *helem;
    listtype result;

  /* hash_key_to_list */
    result = NULL;
    if (hash != NULL) {
      number = hash->table_size;
      helem = &hash->table[0];
      while (number > 0 && *err_info == OKAY_NO_ERROR) {
        helem_key_to_list(&result, *helem, err_info);
        number--;
        helem++;
      } /* while */
    } /* if */
    return result;
  } /* hash_key_to_list */
