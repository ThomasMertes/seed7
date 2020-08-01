/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/arrutl.c                                        */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: Procedures to maintain objects of type arraytype.      */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "executl.h"
#include "runerr.h"
#include "memory.h"

#undef EXTERN
#define EXTERN
#include "arrutl.h"



#ifdef ANSI_C

void destroy_array (objecttype old_elem, memsizetype destr_size,
    objecttype destr_func)
#else

void destroy_array (old_elem, destr_size, destr_func)
objecttype old_elem;
memsizetype destr_size;
objecttype destr_func;
#endif

  { /* destroy_array */
    for (; destr_size > 0; destr_size--) {
      param2_call(destr_func, old_elem, SYS_DESTR_OBJECT);
      old_elem++;
    } /* for */
  } /* destroy_array */



#ifdef ANSI_C

void create_array (objecttype elem_to, objecttype elem_from,
    memsizetype new_size, objecttype create_func, objecttype destr_func)
#else

void create_array (elem_to, elem_from, new_size, create_func, destr_func)
objecttype elem_to;
objecttype elem_from;
memsizetype new_size;
objecttype create_func;
objecttype destr_func;
#endif

  {
    memsizetype position;

  /* create_array */
    position = 0;
    while (position < new_size && !fail_flag) {
      elem_to[position].descriptor.entity = elem_from->descriptor.entity;
      INIT_CLASS_OF_VAR(&elem_to[position], DECLAREDOBJECT);
      elem_to[position].type_of = elem_from->type_of;
      param3_call(create_func, &elem_to[position], SYS_CREA_OBJECT, elem_from);
      if (fail_flag) {
        /* When one create fails (mostly no memory) all elements */
        /* created up to this point must be destroyed to recycle */
        /* the memory correct. */
        fail_flag = FALSE;
        destroy_array(elem_to, position, destr_func);
        fail_flag = TRUE;
      } else {
        elem_from++;
        position++;
      } /* if */
    } /* for */
  } /* create_array */



#ifdef ANSI_C

void copy_array (objecttype elem_to, objecttype elem_from,
    memsizetype copy_size, objecttype copy_func)
#else

void copy_array (elem_to, elem_from, copy_size, copy_func)
objecttype elem_to;
objecttype elem_from;
memsizetype copy_size;
objecttype copy_func;
#endif

  { /* copy_array */
    for (; copy_size > 0; copy_size--) {
      param3_call(copy_func, elem_to, SYS_ASSIGN_OBJECT, elem_from);
      elem_to++;
      elem_from++;
    } /* for */
  } /* copy_array */



#ifdef ANSI_C

void qsort_array (objecttype begin_sort, objecttype end_sort,
    objecttype cmp_func)
#else

void qsort_array (begin_sort, end_sort, cmp_func)
objecttype begin_sort;
objecttype end_sort;
objecttype cmp_func;
#endif

  {
    objectrecord compare_elem;
    objectrecord help_element;
    objecttype middle_elem;
    objecttype less_elem;
    objecttype greater_elem;
    objecttype cmp_obj;
    int cmp;

  /* qsort_array */
    if (end_sort - begin_sort < 8) {
      for (middle_elem = begin_sort + 1; middle_elem <= end_sort; middle_elem++) {
        memcpy(&compare_elem, middle_elem, sizeof(objectrecord));
        less_elem = begin_sort - 1;
        do {
          less_elem++;
          cmp_obj = param2_call(cmp_func, less_elem, &compare_elem);
          isit_int(cmp_obj);
          cmp = take_int(cmp_obj);
          FREE_OBJECT(cmp_obj);
        } while (cmp < 0);
        memmove(&less_elem[1], less_elem, (middle_elem - less_elem) * sizeof(objectrecord));
        memcpy(less_elem, &compare_elem, sizeof(objectrecord));
      } /* for */
    } else {
      middle_elem = &begin_sort[(end_sort - begin_sort) >> 1];
      memcpy(&compare_elem, middle_elem, sizeof(objectrecord));
      memcpy(middle_elem, end_sort, sizeof(objectrecord));
      memcpy(end_sort, &compare_elem, sizeof(objectrecord));
      less_elem = begin_sort - 1;
      greater_elem = end_sort;
      do {
        do {
          less_elem++;
          cmp_obj = param2_call(cmp_func, less_elem, &compare_elem);
          isit_int(cmp_obj);
          cmp = take_int(cmp_obj);
          FREE_OBJECT(cmp_obj);
        } while (cmp < 0);
        do {
          greater_elem--;
          cmp_obj = param2_call(cmp_func, greater_elem, &compare_elem);
          isit_int(cmp_obj);
          cmp = take_int(cmp_obj);
          FREE_OBJECT(cmp_obj);
        } while (cmp > 0 && greater_elem != begin_sort);
        memcpy(&help_element, less_elem, sizeof(objectrecord));
        memcpy(less_elem, greater_elem, sizeof(objectrecord));
        memcpy(greater_elem, &help_element, sizeof(objectrecord));
      } while (greater_elem > less_elem);
      memcpy(greater_elem, less_elem, sizeof(objectrecord));
      memcpy(less_elem, &compare_elem, sizeof(objectrecord));
      memcpy(end_sort, &help_element, sizeof(objectrecord));
      qsort_array(begin_sort, less_elem - 1, cmp_func);
      qsort_array(less_elem + 1, end_sort, cmp_func);
    } /* if */
  } /* qsort_array */
