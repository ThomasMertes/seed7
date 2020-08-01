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
/*  File: seed7/src/actutl.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Conversion of strings to ACTIONs and back.             */
/*                                                                  */
/*  Actions are searched with a binary search. Normally a detailed  */
/*  implementation of the binary search algorithm is used. With the */
/*  USE_BSEARCH flag it is possible to use the bsearch procedure of */
/*  the c-library instead.                                          */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "striutl.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "actutl.h"


#undef USE_BSEARCH


static primacttype *action_ptr_table = NULL;



#ifdef USE_BSEARCH
#ifdef ANSI_C

#ifdef USE_CDECL
static int _cdecl act_strcmp (char *strg1, char *strg2)
#else
static int act_strcmp (void const *strg1, void const *strg2)
#endif
#else

static int act_strcmp (strg1, strg2)
char *strg1;
char *strg2;
#endif

  { /* act_strcmp */
#ifdef TRACE_ACTUTIL
    printf("BEGIN act_strcmp\n");
#endif
    /* printf("strcmp(\"%s\", \"%s\")\n", strg1, ((primacttype) strg2)->name); */
#ifdef TRACE_ACTUTIL
    printf("END act_strcmp\n");
#endif
    return(strcmp(strg1, ((primacttype) strg2)->name));
  } /* act_strcmp */
#endif



#ifdef ANSI_C

#ifdef USE_CDECL
static int _cdecl action_ptr_compare (char *act_ptr1, char *act_ptr2)
#else
static int action_ptr_compare (const void *act_ptr1, const void *act_ptr2)
#endif
#else

static int action_ptr_compare (act_ptr1, act_ptr2)
char *act_ptr1;
char *act_ptr2;
#endif

  { /* action_ptr_compare */
#ifdef TRACE_ACTUTIL
    printf("BEGIN action_ptr_compare\n");
#endif
    if (((unsigned long) (*((primacttype *) act_ptr1))->action) <
        ((unsigned long) (*((primacttype *) act_ptr2))->action)) {
      return -1;
    } else if (((unsigned long) (*((primacttype *) act_ptr1))->action) ==
               ((unsigned long) (*((primacttype *) act_ptr2))->action)) {
      return 0;
    } else {
      return 1;
    } /* if */
#ifdef TRACE_ACTUTIL
    printf("END action_ptr_compare\n");
#endif
  } /* action_ptr_compare */



#ifdef ANSI_C

static booltype search_action (ustritype stri, acttype *action_found)
#else

static booltype search_action (stri, action_found)
ustritype stri;
acttype *action_found;
#endif

  {
#ifdef USE_BSEARCH
    primacttype found;
#else
    int lower;
    int upper;
    int middle;
    int comparison;
#endif
    int action_number;
    booltype result;

  /* search_action */
#ifdef TRACE_ACTUTIL
    printf("BEGIN search_action\n");
#endif
#ifdef USE_BSEARCH
    if ((found = (primacttype) bsearch(stri, &act_table.primitive[1],
        act_table.size - 1, sizeof(primactrecord), act_strcmp)) != NULL) {
      action_number = found - &act_table.primitive[0];
    } else {
      action_number = 0;
    } /* if */
/*  printf("action number: %d\n", action_number); */
#else
    action_number = 0;
    lower = 0;
    upper = act_table.size;
    while (lower < upper - 1) {
      middle = (lower + upper) >> 1;
      /* printf("%d %d %d >%s< >%s<\n", lower, middle, upper,
         act_table.primitive[middle].name, stri); */
      if ((comparison = strcmp(act_table.primitive[middle].name,
          (cstritype) stri)) < 0) {
        lower = middle;
      } else if (comparison == 0) {
        lower = upper - 1;
        action_number = middle;
      } else {
        upper = middle;
      } /* if */
    } /* while */
#endif
    if (act_table.primitive != NULL) {
/* printf("action(\"%s\")\n", act_table.primitive[action_number].name); */
      *action_found = act_table.primitive[action_number].action;
    } else {
      *action_found = NULL;
    } /* if */
    result = (booltype) (action_number != 0);
#ifdef TRACE_ACTUTIL
    printf("END search_action\n");
#endif
    return(result);
  } /* search_action */



#ifdef ANSI_C

booltype find_action (stritype action_name, acttype *action_found)
#else

booltype find_action (action_name, action_found)
stritype action_name;
acttype *action_found;
#endif

  {
    uchartype act_name[250];
    booltype result;

 /* find_action */
#ifdef TRACE_ACTUTIL
    printf("BEGIN find_action\n");
#endif
    if (compr_size(action_name) + 1 > 250) {
      if (act_table.primitive != NULL) {
        *action_found = act_table.primitive[0].action;
      } else {
        *action_found = NULL;
      } /* if */
      result = FALSE;
    } else {
      stri_export(act_name, action_name);
      result = search_action(act_name, action_found);
    } /* if */
#ifdef TRACE_ACTUTIL
    printf("END find_action\n");
#endif
    return(result);
  } /* find_action */



#ifdef ANSI_C

primacttype get_primact (acttype action_searched)
#else

primacttype get_primact (action_searched)
register acttype action_searched;
#endif

  {
    unsigned int number;
    int lower;
    int upper;
    int middle;
    primacttype result;

  /* get_primact */
#ifdef TRACE_ACTUTIL
    printf("BEGIN get_primact\n");
#endif
    if (action_ptr_table == NULL) {
      if (!ALLOC_TABLE(action_ptr_table, primacttype, act_table.size)) {
        action_searched = act_table.primitive[0].action;
      } else {
        for (number = 0; number < act_table.size; number++) {
          action_ptr_table[number] = &act_table.primitive[number];
        } /* for */
        qsort(action_ptr_table, act_table.size, sizeof(primacttype),
            action_ptr_compare);
        for (number = 1; number < act_table.size; number++) {
          if (action_ptr_table[number]->action == action_ptr_table[number - 1]->action) {
            printf("*** Actions %s and %s implemented by the same function\n",
                action_ptr_table[number]->name,
                action_ptr_table[number - 1]->name);
          } /* if */
        } /* for */
      } /* if */
    } /* if */

    result = &act_table.primitive[0];
    if (action_searched != act_table.primitive[0].action) {
      lower = -1;
      upper = act_table.size;
      while (lower < upper - 1) {
        middle = (lower + upper) >> 1;
        /* printf("%d %d %d >%lu< >%lu<\n", lower, middle, upper,
           action_ptr_table[middle]->action, action_searched); */

        if (((unsigned long) action_ptr_table[middle]->action) <
            ((unsigned long) action_searched)) {
          lower = middle;
        } else if (action_ptr_table[middle]->action == action_searched) {
          lower = upper - 1;
          result = action_ptr_table[middle];
        } else {
          upper = middle;
        } /* if */
      } /* while */
    } /* if */
#ifdef TRACE_ACTUTIL
    printf("END get_primact\n");
#endif
    return(result);
  } /* get_primact */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

cstritype act_string (register acttype action_searched)
#else

cstritype act_string (action_searched)
register acttype action_searched;
#endif

  {
    register acttype *actual_action;

  /* act_string */
#ifdef TRACE_ACTUTIL
    printf("BEGIN act_string\n");
#endif
    actual_action = &act_table.primitive[act_table.size - 1].action;
    while (*actual_action != action_searched &&
        actual_action != &act_table.primitive[0].action) {
      actual_action--;
    } /* while */
#ifdef TRACE_ACTUTIL
    printf("END act_string\n");
#endif
    return(act_table.primitive[actual_action - &act_table.primitive[0].action].name);
  } /* act_string */



#ifdef ANSI_C

cstritype act_string (acttype action_searched)
#else

cstritype act_string (action_searched)
acttype action_searched;
#endif

  {
    register int action_number;

  /* act_string */
#ifdef TRACE_ACTUTIL
    printf("BEGIN act_string\n");
#endif
    action_number = act_table.size - 1;
    while (action_searched != act_table.primitive[action_number].action &&
        action_number != 0) {
      action_number--;
    } /* while */
#ifdef TRACE_ACTUTIL
    printf("END act_string\n");
#endif
    return(act_table.primitive[action_number].name);
  } /* act_string */
#endif



#ifdef WITH_ACTION_CHECK
#ifdef ANSI_C

booltype act_okay (acttype action_searched)
#else

booltype act_okay (action_searched)
acttype action_searched;
#endif

  {
    register acttype *actual_action;

  /* act_okay */
#ifdef TRACE_ACTUTIL
    printf("BEGIN act_okay\n");
#endif
    actual_action = &act_table.primitive[act_table.size - 1].action;
    while (*actual_action != action_searched &&
        actual_action != &act_table.primitive[0].action) {
      actual_action--;
    } /* while */
#ifdef TRACE_ACTUTIL
    printf("END act_okay\n");
#endif
    return(actual_action != &act_table.primitive[0].action);
  } /* act_okay */
#endif



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

booltype act_okay (acttype action_searched)
#else

booltype act_okay (action_searched)
acttype action_searched;
#endif

  {
    register int action_number;

  /* act_okay */
#ifdef TRACE_ACTUTIL
    printf("BEGIN act_okay\n");
#endif
    action_number = act_table.size - 1;
    while (action_searched != act_table.primitive[action_number].action &&
        action_number != 0) {
      action_number--;
    } /* while */
#ifdef TRACE_ACTUTIL
    printf("END act_okay\n");
#endif
    return(action_number != 0);
  } /* act_okay */
#endif
