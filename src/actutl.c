/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2014  Thomas Mertes                        */
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
/*  Changes: 1992, 1993, 1994, 2014  Thomas Mertes                  */
/*  Content: Conversion of strings to ACTIONs and back.             */
/*                                                                  */
/*  Actions are searched with a binary search. Normally a detailed  */
/*  implementation of the binary search algorithm is used. With the */
/*  USE_BSEARCH flag it is possible to use the bsearch procedure of */
/*  the c-library instead.                                          */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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


#define USE_BSEARCH 0
#define MAX_CSTRI_BUFFER_LEN 40


typedef struct {
    unsigned int size;
    primActType *primitive_ptr;
} actPtrTableType;

static actPtrTableType act_ptr_table = {0, NULL};



#if USE_BSEARCH
#ifdef USE_CDECL
static int _cdecl act_strcmp (char *strg1, char *strg2)
#else
static int act_strcmp (void const *strg1, void const *strg2)
#endif

  {
    int signumValue;

  /* act_strcmp */
    logFunction(printf("act_strcmp(\"%s\", \"%s\")\n",
                       strg1, ((primActType) strg2)->name););
    signumValue = strcmp(strg1, ((primActType) strg2)->name);
    logFunction(printf("act_strcmp --> %d\n", signumValue););
    return signumValue;
  } /* act_strcmp */
#endif



#ifdef USE_CDECL
static int _cdecl action_ptr_compare (char *act_ptr1, char *act_ptr2)
#else
static int action_ptr_compare (const void *act_ptr1, const void *act_ptr2)
#endif

  {
    int signumValue;

  /* action_ptr_compare */
    logFunction(printf("action_ptr_compare(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) (*(const primActType *) act_ptr1)->action,
                       (memSizeType) (*(const primActType *) act_ptr2)->action););
    if ((memSizeType) (*(const primActType *) act_ptr1)->action <
        (memSizeType) (*(const primActType *) act_ptr2)->action) {
      signumValue = -1;
    } else if ((*(const primActType *) act_ptr1)->action ==
               (*(const primActType *) act_ptr2)->action) {
      signumValue = 0;
    } else {
      signumValue = 1;
    } /* if */
    logFunction(printf("action_ptr_compare --> %d\n", signumValue););
    return signumValue;
  } /* action_ptr_compare */



static boolType search_action (cstriType stri, actType *action_found)

  {
#if USE_BSEARCH
    primActType found;
#else
    unsigned int lower;
    unsigned int upper;
    unsigned int middle;
    int comparison;
#endif
    unsigned int action_number;
    boolType result;

  /* search_action */
    logFunction(printf("search_action(\"%s\", *)\n", stri););
#if USE_BSEARCH
    if ((found = (primActType) bsearch(stri, &act_table.primitive[1],
        act_table.size - 1, sizeof(primActRecord), act_strcmp)) != NULL) {
      action_number = (unsigned int) (found - &act_table.primitive[0]);
    } else {
      action_number = 0;
    } /* if */
/*  printf("action number: %u\n", action_number); */
#else
    action_number = 0;
    lower = 0;
    upper = act_table.size;
    while (lower + 1 < upper) {
      middle = (lower + upper) >> 1;
      /* printf("%u %u %u >%s< >%s<\n", lower, middle, upper,
         act_table.primitive[middle].name, stri); */
      if ((comparison = strcmp(act_table.primitive[middle].name, stri)) < 0) {
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
    result = (boolType) (action_number != 0);
    logFunction(printf("search_action --> %d\n", result););
    return result;
  } /* search_action */



boolType find_action (const const_striType action_name, actType *action_found)

  {
    char act_name[MAX_CSTRI_BUFFER_LEN + 1];
    errInfoType err_info = OKAY_NO_ERROR;
    boolType result;

 /* find_action */
    logFunction(printf("find_action(\"%s\", *)\n",
                       striAsUnquotedCStri(action_name)););
    if (action_name->size > MAX_CSTRI_BUFFER_LEN) {
      result = FALSE;
    } else {
      conv_to_cstri(act_name, action_name, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        result = FALSE;
      } else {
        result = search_action(act_name, action_found);
      } /* if */
    } /* if */
    if (!result) {
      if (act_table.primitive != NULL) {
        *action_found = act_table.primitive[0].action;
      } else {
        *action_found = NULL;
      } /* if */
    } /* if */
    logFunction(printf("find_action --> %d\n", result););
    return result;
  } /* find_action */



/**
 *  Create act_ptr_table and remove double actType pointers from it.
 *  The table act_ptr_table is used by get_primact() to map
 *  actType pointers to corresponding entries in act_table.
 *  When the C compiler recognizes that the code of two functions
 *  is identical it may decide to reuse the code. In this case
 *  two or more actType pointers refer to the same function.
 *  To have predictable results double actType pointers are removed
 *  from act_ptr_table. The actType pointer that referes to the
 *  action with the alphabetically lower action name is kept.
 *  Since the action names in act_table are sorted alphabetically
 *  this is the entry that is nearer to the beginning of act_table.
 */
static void gen_act_ptr_table (void)

  {
    unsigned int number;

  /* gen_act_ptr_table */
    logFunction(printf("gen_act_ptr_table\n"););
    act_ptr_table.size = act_table.size;
    if (ALLOC_TABLE(act_ptr_table.primitive_ptr, primActType, act_ptr_table.size)) {
      for (number = 0; number < act_ptr_table.size; number++) {
        act_ptr_table.primitive_ptr[number] = (primActType) &act_table.primitive[number];
      } /* for */
      qsort(act_ptr_table.primitive_ptr, act_ptr_table.size, sizeof(primActType),
          action_ptr_compare);
      for (number = 1; number < act_ptr_table.size; number++) {
        if (act_ptr_table.primitive_ptr[number]->action ==
            act_ptr_table.primitive_ptr[number - 1]->action) {
          /* printf("*** Actions %s and %s implemented by the same function\n",
              act_ptr_table.primitive_ptr[number - 1]->name,
              act_ptr_table.primitive_ptr[number]->name); */
          /* Remove double entries */
          if ((memSizeType) act_ptr_table.primitive_ptr[number - 1] >
              (memSizeType) act_ptr_table.primitive_ptr[number]) {
            memmove(&act_ptr_table.primitive_ptr[number - 1], &act_ptr_table.primitive_ptr[number],
                    (act_ptr_table.size - number) * sizeof(primActType));
          } else {
            memmove(&act_ptr_table.primitive_ptr[number], &act_ptr_table.primitive_ptr[number + 1],
                    (act_ptr_table.size - number - 1) * sizeof(primActType));
          } /* if */
          act_ptr_table.size--;
          number--;
          /* printf("size=%u, number=%u\n", act_ptr_table.size, number); */
        } /* if */
      } /* for */
    } /* if */
    logFunction(printf("gen_act_ptr_table -->\n"););
  } /* gen_act_ptr_table */



/**
 *  Get an act_table entry that corresponds to an actType pointer.
 *  @return pointer to an act_table entry.
 */
const_primActType get_primact (actType action_searched)

  {
    int lower;
    int upper;
    int middle;
    const_primActType result;

  /* get_primact */
    logFunction(printf("get_primact(" FMT_U_MEM ")\n",
                       (memSizeType) action_searched););
    if (unlikely(act_ptr_table.primitive_ptr == NULL)) {
      gen_act_ptr_table();
      if (unlikely(act_ptr_table.primitive_ptr == NULL)) {
        action_searched = act_table.primitive[0].action;
      } /* if */
    } /* if */

    result = &act_table.primitive[0];
    if (likely(action_searched != act_table.primitive[0].action)) {
      lower = -1;
      upper = (int) act_ptr_table.size;
      while (lower + 1 < upper) {
        middle = (lower + upper) >> 1;
        /* printf("%d %d %d <" FMT_U_MEM "> <" FMT_U_MEM ">\n", lower, middle, upper,
            (memSizeType) act_ptr_table.primitive_ptr[middle]->action,
            (memSizeType) action_searched); */
        if (((memSizeType) act_ptr_table.primitive_ptr[middle]->action) <
            ((memSizeType) action_searched)) {
          lower = middle;
        } else if (act_ptr_table.primitive_ptr[middle]->action == action_searched) {
          lower = upper - 1;
          result = act_ptr_table.primitive_ptr[middle];
        } else {
          upper = middle;
        } /* if */
      } /* while */
    } /* if */
    logFunction(printf("get_primact --> %s\n", result->name););
    return result;
  } /* get_primact */
