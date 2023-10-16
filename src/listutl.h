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
/*  Module: General                                                 */
/*  File: seed7/src/listutl.h                                       */
/*  Changes: 1990, 1991, 1992, 1993, 1994, 2002  Thomas Mertes      */
/*  Content: Functions to maintain objects of type listType.        */
/*                                                                  */
/********************************************************************/

#define append_to_list(insert_place, object, act_param_list) { \
    listType help_element;                                     \
    if (ALLOC_L_ELEM(help_element)) {                          \
      help_element->next = NULL;                               \
      help_element->obj = object;                              \
      *insert_place = help_element;                            \
      insert_place = &help_element->next;                      \
    } else if (!fail_flag) {                                   \
      raise_with_arguments(SYS_MEM_EXCEPTION, act_param_list); \
    } }

#if WITH_LIST_FREELIST
#define free_list2(list_begin, list_end) { \
    list_end->next = flist.list_elems;        \
    flist.list_elems = list_begin;            \
    }
#else
#define free_list2(list_begin, list_end) free_list(list_begin)
#endif


void free_list (listType list);
memSizeType list_length (const_listType list);
listType *append_element_to_list (listType *list_insert_place, objectType object,
                                  errInfoType *err_info);
objectType copy_expression (objectType object_from, errInfoType *err_info);
void free_expression (objectType object);
void concat_lists (listType *list1, listType list2);
void incl_list (listType *list, objectType element_object,
                errInfoType *err_info);
void excl_list (listType *list, const_objectType elementobject);
void pop_list (listType *list);
void replace_list_elem (listType list, const_objectType elem1,
                        objectType elem2);
listType copy_list (const_listType list_from, errInfoType *err_info);
listType array_to_list (arrayType arr_from, errInfoType *err_info);
listType struct_to_list (structType stru_from, errInfoType *err_info);
listType hash_data_to_list (hashType hash, errInfoType *err_info);
listType hash_keys_to_list (hashType hash, errInfoType *err_info);
