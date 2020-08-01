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
/*  Content: Procedures to maintain objects of type listtype.       */
/*                                                                  */
/********************************************************************/

#define append_to_list(insert_place, object, act_param_list) { \
    listtype help_element;                                     \
    if (ALLOC_L_ELEM(help_element)) {                          \
      help_element->next = NULL;                               \
      help_element->obj = object;                              \
      *insert_place = help_element;                            \
      insert_place = &help_element->next;                      \
    } else if (!fail_flag) {                                   \
      raise_with_arguments(SYS_MEM_EXCEPTION, act_param_list); \
    } }

#ifdef WITH_LIST_FREELIST
#define free_list2(list_begin, list_end) { \
    list_end->next = flist.list_elems;        \
    flist.list_elems = list_begin;            \
    }
#else
#define free_list2(list_begin, list_end) free_list(list_begin)
#endif


#ifdef ANSI_C

void free_list (listtype list);
listtype *append_element_to_list (listtype *list_insert_place, objecttype object,
                                  errinfotype *err_info);
objecttype copy_expression (objecttype object_from, errinfotype *err_info);
void free_expression (objecttype object);
void concat_lists (listtype *list1, listtype list2);
void incl_list (listtype *list, objecttype element_object,
                errinfotype *err_info);
void excl_list (listtype *list, const_objecttype elementobject);
void pop_list (listtype *list);
void replace_list_elem (listtype list, const_objecttype elem1,
                        objecttype elem2);
listtype copy_list (const_listtype list_from, errinfotype *err_info);
listtype array_to_list (arraytype arr_from, errinfotype *err_info);
listtype struct_to_list (structtype stru_from, errinfotype *err_info);
listtype hash_data_to_list (hashtype hash, errinfotype *err_info);
listtype hash_keys_to_list (hashtype hash, errinfotype *err_info);

#else

void free_list ();
listtype *append_element_to_list ();
objecttype copy_expression ();
void free_expression ();
void concat_lists ();
void incl_list ();
void excl_list ();
void pop_list ();
void replace_list_elem ();
listtype copy_list ();
listtype array_to_list ();
listtype struct_to_list ();
listtype hash_data_to_list ();
listtype hash_keys_to_list ();

#endif
