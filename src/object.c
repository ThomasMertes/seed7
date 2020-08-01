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
/*  Module: Analyzer - Utility                                      */
/*  File: seed7/src/object.c                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Procedures to maintain objects and lists.              */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "entutl.h"
#include "objutl.h"
#include "infile.h"
#include "fatal.h"
#include "stat.h"

#undef EXTERN
#define EXTERN
#include "object.h"


#undef TRACE_OBJECT



objectType new_empty_list_object (const_objectType typeof_object)

  {
    register objectType created_list;

  /* new_empty_list_object */
#ifdef TRACE_OBJECT
    printf("BEGIN new_empty_list_object\n");
#endif
#ifdef WITH_STATISTIC
    list_count++;
#endif
    if (!ALLOC_OBJECT(created_list)) {
      fatal_memory_error(SOURCE_POSITION(2031));
    } /* if */
    created_list->type_of = take_type(typeof_object);
    created_list->descriptor.posinfo = CREATE_POSINFO(in_file.line, in_file.file_number);
    INIT_CATEGORY_OF_POSINFO(created_list, EXPROBJECT);
    created_list->value.listValue = NULL;
#ifdef TRACE_OBJECT
    printf("END new_empty_list_object ");
    printf("%lu ", (unsigned long) created_list);
    trace1(created_list);
    printf("\n");
#endif
    return created_list;
  } /* new_empty_list_object */



objectType new_nonempty_expression_object (objectType first_element,
    listType *list, const_objectType typeof_object)

  {
    register objectType created_list;

  /* new_nonempty_expression_object */
#ifdef TRACE_OBJECT
    printf("BEGIN new_nonempty_expression_object ");
    trace1(first_element);
    printf("\n");
#endif
#ifdef WITH_STATISTIC
    list_count++;
#endif
    if (!ALLOC_OBJECT(created_list)) {
      fatal_memory_error(SOURCE_POSITION(2032));
    } /* if */
    created_list->type_of = take_type(typeof_object);
    created_list->descriptor.posinfo = CREATE_POSINFO(in_file.line, in_file.file_number);
    INIT_CATEGORY_OF_POSINFO(created_list, EXPROBJECT);
    if (!ALLOC_L_ELEM(*list)) {
      fatal_memory_error(SOURCE_POSITION(2033));
    } /* if */
    (*list)->next = NULL;
    (*list)->obj = first_element;
    created_list->value.listValue = *list;
#ifdef TRACE_OBJECT
    printf("END new_nonempty_expression_object ");
    printf("%lu ", (unsigned long) created_list);
    trace1(created_list);
    printf("\n");
#endif
    return created_list;
  } /* new_nonempty_expression_object */



objectType new_type_of_expression_object (objectType first_element,
    listType *list, typeType type_of)

  {
    register objectType created_list;

  /* new_type_of_expression_object */
#ifdef TRACE_OBJECT
    printf("BEGIN new_type_of_expression_object ");
    trace1(first_element);
    printf("\n");
#endif
#ifdef WITH_STATISTIC
    list_count++;
#endif
    if (!ALLOC_OBJECT(created_list)) {
      fatal_memory_error(SOURCE_POSITION(2034));
    } /* if */
    created_list->type_of = type_of;
    created_list->descriptor.posinfo = CREATE_POSINFO(in_file.line, in_file.file_number);
    INIT_CATEGORY_OF_POSINFO(created_list, EXPROBJECT);
    if (!ALLOC_L_ELEM(*list)) {
      fatal_memory_error(SOURCE_POSITION(2035));
    } /* if */
    (*list)->next = NULL;
    (*list)->obj = first_element;
    created_list->value.listValue = *list;
#ifdef TRACE_OBJECT
    printf("END new_type_of_expression_object ");
    printf("%lu ", (unsigned long) created_list);
    trace1(created_list);
    printf("\n");
#endif
    return created_list;
  } /* new_type_of_expression_object */



objectType new_expression_object (listType *list)

  {
    register objectType created_list;

  /* new_expression_object */
#ifdef TRACE_OBJECT
    printf("BEGIN new_expression_object\n");
#endif
#ifdef WITH_STATISTIC
    list_count++;
#endif
    if (!ALLOC_OBJECT(created_list)) {
      fatal_memory_error(SOURCE_POSITION(2034));
    } /* if */
    created_list->type_of = NULL;
    created_list->descriptor.posinfo = CREATE_POSINFO(in_file.line, in_file.file_number);
    INIT_CATEGORY_OF_POSINFO(created_list, EXPROBJECT);
    if (!ALLOC_L_ELEM(*list)) {
      fatal_memory_error(SOURCE_POSITION(2035));
    } /* if */
    (*list)->next = NULL;
    (*list)->obj = NULL;
    created_list->value.listValue = *list;
#ifdef TRACE_OBJECT
    printf("END new_expression_object ");
    printf("%lu ", (unsigned long) created_list);
    trace1(created_list);
    printf("\n");
#endif
    return created_list;
  } /* new_expression_object */



listType add_element_to_list (listType list, objectType object)

  {
    register listType help_element;

  /* add_element_to_list */
#ifdef TRACE_OBJECT
    printf("BEGIN add_element_to_list ");
    printf("%lu ", (unsigned long) object);
    trace1(object);
    printf("\n");
#endif
    if (ALLOC_L_ELEM(help_element)) {
      help_element->next = NULL;
      help_element->obj = object;
      list->next = help_element;
      return help_element;
    } else {
      fatal_memory_error(SOURCE_POSITION(2036));
      return NULL;
    } /* if */
#ifdef TRACE_OBJECT
    printf("END add_element_to_list ");
    prot_list(*list);
    printf("\n");
#endif
  } /* add_element_to_list */
