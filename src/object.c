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
#include "infile.h"
#include "memory.h"
#include "fatal.h"
#include "stat.h"

#undef EXTERN
#define EXTERN
#include "object.h"



#ifdef ANSI_C

objecttype new_empty_list_object (objecttype typeof_object)
#else

objecttype new_empty_list_object (typeof_object)
objecttype typeof_object;
#endif

  {
    register objecttype created_list;

  /* new_empty_list_object */
#ifdef TRACE_OBJECT
    printf("BEGIN new_empty_list_object\n");
#endif
#ifdef WITH_STATISTIK
    list_count++;
#endif
    if (!ALLOC_OBJECT(created_list)) {
      fatal_memory_error(SOURCE_POSITION(2031));
    } /* if */
    created_list->type_of = take_type(typeof_object);
    created_list->descriptor.posinfo = CREATE_POSINFO(in_file.line, in_file.file_number);
    INIT_CLASS_OF_POSINFO(created_list, EXPROBJECT);
    created_list->value.listvalue = NULL;
#ifdef TRACE_OBJECT
    printf("END new_empty_list_object ");
    printf("%lu ", (unsigned long) created_list);
    trace1(created_list);
    printf("\n");
#endif
    return(created_list);
  } /* new_empty_list_object */



#ifdef ANSI_C

objecttype new_nonempty_expression_object (objecttype first_element,
    listtype *list, objecttype typeof_object)
#else

objecttype new_nonempty_expression_object (first_element, list, typeof_object)
objecttype first_element;
listtype *list;
objecttype typeof_object;
#endif

  {
    register objecttype created_list;

  /* new_nonempty_expression_object */
#ifdef TRACE_OBJECT
    printf("BEGIN new_nonempty_expression_object ");
    trace1(first_element);
    printf("\n");
#endif
#ifdef WITH_STATISTIK
    list_count++;
#endif
    if (!ALLOC_OBJECT(created_list)) {
      fatal_memory_error(SOURCE_POSITION(2032));
    } /* if */
    created_list->type_of = take_type(typeof_object);
    created_list->descriptor.posinfo = CREATE_POSINFO(in_file.line, in_file.file_number);
    INIT_CLASS_OF_POSINFO(created_list, EXPROBJECT);
    if (!ALLOC_L_ELEM(*list)) {
      fatal_memory_error(SOURCE_POSITION(2033));
    } /* if */
    (*list)->next = NULL;
    (*list)->obj = first_element;
    created_list->value.listvalue = *list;
#ifdef TRACE_OBJECT
    printf("END new_nonempty_expression_object ");
    printf("%lu ", (unsigned long) created_list);
    trace1(created_list);
    printf("\n");
#endif
    return(created_list);
  } /* new_nonempty_expression_object */



#ifdef ANSI_C

objecttype new_type_of_expression_object (objecttype first_element,
    listtype *list, typetype type_of)
#else

objecttype new_type_of_expression_object (first_element, list, type_of)
objecttype first_element;
listtype *list;
typetype type_of;
#endif

  {
    register objecttype created_list;

  /* new_type_of_expression_object */
#ifdef TRACE_OBJECT
    printf("BEGIN new_type_of_expression_object ");
    trace1(first_element);
    printf("\n");
#endif
#ifdef WITH_STATISTIK
    list_count++;
#endif
    if (!ALLOC_OBJECT(created_list)) {
      fatal_memory_error(SOURCE_POSITION(2034));
    } /* if */
    created_list->type_of = type_of;
    created_list->descriptor.posinfo = CREATE_POSINFO(in_file.line, in_file.file_number);
    INIT_CLASS_OF_POSINFO(created_list, EXPROBJECT);
    if (!ALLOC_L_ELEM(*list)) {
      fatal_memory_error(SOURCE_POSITION(2035));
    } /* if */
    (*list)->next = NULL;
    (*list)->obj = first_element;
    created_list->value.listvalue = *list;
#ifdef TRACE_OBJECT
    printf("END new_type_of_expression_object ");
    printf("%lu ", (unsigned long) created_list);
    trace1(created_list);
    printf("\n");
#endif
    return(created_list);
  } /* new_type_of_expression_object */



#ifdef ANSI_C

listtype add_element_to_list (listtype list, objecttype object)
#else

listtype add_element_to_list (list, object)
listtype list;
objecttype object;
#endif

  {
    register listtype help_element;

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
      return(help_element);
    } else {
      fatal_memory_error(SOURCE_POSITION(2036));
      return(NULL);
    } /* if */
#ifdef TRACE_OBJECT
    printf("END add_element_to_list ");
    prot_list(*list);
    printf("\n");
#endif
  } /* add_element_to_list */
