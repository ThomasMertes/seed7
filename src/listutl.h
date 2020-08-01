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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: General                                                 */
/*  File: seed7/src/listutl.h                                       */
/*  Changes: 1990, 1991, 1992, 1993, 1994, 2002  Thomas Mertes      */
/*  Content: Procedures to maintain objects of type listtype.       */
/*                                                                  */
/********************************************************************/

#define append_to_list(insert_place, object, act_param_list) { \
    listtype help_element;                                      \
    if (ALLOC_L_ELEM(help_element)) {                           \
      help_element->next = NULL;                                \
      help_element->obj = object;                               \
      *insert_place = help_element;                             \
      insert_place = &help_element->next;                       \
    } else if (!fail_flag) {                                    \
      raise_with_arguments(SYS_MEM_EXCEPTION, act_param_list);  \
    } }


#ifdef ANSI_C

void emptylist (listtype);
listtype *append_element_to_list (listtype *, objecttype, errinfotype *);
void copy_expression (objecttype, objecttype *, errinfotype *);
void concat_lists (listtype *, listtype);
void incl_list (listtype *, objecttype, errinfotype *);
void excl_list (listtype *, objecttype);
void copy_list (listtype, listtype *, errinfotype *err_info);
booltype array_to_list (arraytype, listtype *);

#else

void emptylist ();
listtype *append_element_to_list ();
void copy_expression ();
void concat_lists ();
void incl_list ();
void excl_list ();
void copy_list ();
booltype array_to_list ();

#endif
