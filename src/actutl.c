/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2016  Thomas Mertes                        */
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
/*  Changes: 1992, 1993, 1994, 2014, 2016  Thomas Mertes            */
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
    actEntryType *table;
} actEntryMapType;

static actEntryMapType actEntryMap = {0, NULL};



#if USE_BSEARCH
#ifdef USE_CDECL
static int _cdecl actTableCmp (char *strg1, char *strg2)
#else
static int actTableCmp (void const *strg1, void const *strg2)
#endif

  {
    int signumValue;

  /* actTableCmp */
    logFunction(printf("actTableCmp(\"%s\", \"%s\")\n",
                       strg1, ((actEntryType) strg2)->name););
    signumValue = strcmp(strg1, ((actEntryType) strg2)->name);
    logFunction(printf("actTableCmp --> %d\n", signumValue););
    return signumValue;
  } /* actTableCmp */
#endif



#ifdef USE_CDECL
static int _cdecl actEntryMapCmp (char *act_ptr1, char *act_ptr2)
#else
static int actEntryMapCmp (const void *act_ptr1, const void *act_ptr2)
#endif

  {
    int signumValue;

  /* actEntryMapCmp */
    logFunction(printf("actEntryMapCmp(" FMT_U_MEM ", " FMT_U_MEM ")\n",
                       (memSizeType) (*(const actEntryType *) act_ptr1)->action,
                       (memSizeType) (*(const actEntryType *) act_ptr2)->action););
    if ((memSizeType) (*(const actEntryType *) act_ptr1)->action <
        (memSizeType) (*(const actEntryType *) act_ptr2)->action) {
      signumValue = -1;
    } else {
      signumValue = (memSizeType) (*(const actEntryType *) act_ptr1)->action >
                    (memSizeType) (*(const actEntryType *) act_ptr2)->action;
    } /* if */
    logFunction(printf("actEntryMapCmp --> %d\n", signumValue););
    return signumValue;
  } /* actEntryMapCmp */



/**
 *  Search for the action with the given ''actionName''.
 *  @param actionName Name of the action searched.
 *  @return The action found, or NULL if no such action exists.
 */
static actType searchAction (cstriType actionName)

  {
#if USE_BSEARCH
    actEntryType found;
#else
    unsigned int lower;
    unsigned int upper;
    unsigned int middle;
    int comparison;
#endif
    unsigned int actionNumber;
    actType actionFound;

  /* searchAction */
    logFunction(printf("searchAction(\"%s\")\n", actionName););
#if USE_BSEARCH
    if ((found = (actEntryType) bsearch(actionName, &actTable.table[1],
        actTable.size - 1, sizeof(actEntryRecord), actTableCmp)) != NULL) {
      actionNumber = (unsigned int) (found - &actTable.table[0]);
    } else {
      actionNumber = 0;
    } /* if */
    /* printf("action number: %u\n", actionNumber); */
#else
    actionNumber = 0;
    lower = 0;
    upper = actTable.size;
    while (lower + 1 < upper) {
      middle = (lower + upper) >> 1;
      /* printf("%u %u %u >%s< >%s<\n", lower, middle, upper,
         actTable.table[middle].name, actionName); */
      if ((comparison = strcmp(actTable.table[middle].name, actionName)) < 0) {
        lower = middle;
      } else if (comparison == 0) {
        lower = upper - 1;
        actionNumber = middle;
      } else {
        upper = middle;
      } /* if */
    } /* while */
#endif
    if (actTable.table != NULL && actionNumber != 0) {
      /* printf("action(\"%s\")\n", actTable.table[actionNumber].name); */
      actionFound = actTable.table[actionNumber].action;
    } else {
      actionFound = NULL;
    } /* if */
    logFunction(printf("searchAction --> " FMT_U_MEM "\n",
                       (memSizeType) actionFound););
    return actionFound;
  } /* searchAction */



/**
 *  Find the action with the given ''actionName''.
 *  @param actionName Name of the action searched.
 *  @return The action found, or NULL if no such action exists.
 */
actType findAction (const const_striType actionName)

  {
    char actName[MAX_CSTRI_BUFFER_LEN + NULL_TERMINATION_LEN];
    errInfoType err_info = OKAY_NO_ERROR;
    actType actionFound;

 /* findAction */
    logFunction(printf("findAction(\"%s\")\n",
                       striAsUnquotedCStri(actionName)););
    if (unlikely(actionName->size > MAX_CSTRI_BUFFER_LEN)) {
      actionFound = NULL;
    } else {
      conv_to_cstri(actName, actionName, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        actionFound = NULL;
      } else {
        actionFound = searchAction(actName);
      } /* if */
    } /* if */
    logFunction(printf("findAction --> " FMT_U_MEM "\n",
                       (memSizeType) actionFound););
    return actionFound;
  } /* findAction */



/**
 *  Get the action ACT_ILLEGAL.
 *  @return a function pointer to the function act_illegal().
 */
actType getActIllegal (void)

  {
    actType actionFound;

  /* getActIllegal */
    if (actTable.table != NULL) {
      actionFound = actTable.table[0].action;
    } else {
      actionFound = NULL;
    } /* if */
    return actionFound;
  } /* getActIllegal */



/**
 *  Create actEntryMap and remove double actType values from it.
 *  The table actEntryMap is used by getActEntry() to map actType
 *  function pointers to corresponding entries in actTable. When
 *  the C compiler recognizes that the code of two functions is
 *  identical it may decide to reuse the code. In this case two
 *  or more actType function pointers refer to the same function.
 *  To have predictable results double actType values are removed
 *  from actEntryMap. The actType function pointer that referes to
 *  the action with the alphabetically lower action name is kept.
 *  Since the action names in actTable are sorted alphabetically
 *  this is the entry that is nearer to the beginning of actTable.
 */
static void genActPtrTable (void)

  {
    unsigned int number;

  /* genActPtrTable */
    logFunction(printf("genActPtrTable\n"););
    actEntryMap.size = actTable.size;
    if (ALLOC_TABLE(actEntryMap.table, actEntryType, actEntryMap.size)) {
      for (number = 0; number < actEntryMap.size; number++) {
        actEntryMap.table[number] = (actEntryType) &actTable.table[number];
      } /* for */
      qsort(actEntryMap.table, actEntryMap.size, sizeof(actEntryType),
          actEntryMapCmp);
      for (number = 1; number < actEntryMap.size; number++) {
        if (actEntryMap.table[number]->action ==
            actEntryMap.table[number - 1]->action) {
          /* printf("*** Actions %s and %s implemented by the same function\n",
              actEntryMap.table[number - 1]->name,
              actEntryMap.table[number]->name); */
          /* Remove double entries */
          if ((memSizeType) actEntryMap.table[number - 1] >
              (memSizeType) actEntryMap.table[number]) {
            memmove(&actEntryMap.table[number - 1],
                    &actEntryMap.table[number],
                    (actEntryMap.size - number) * sizeof(actEntryType));
          } else {
            memmove(&actEntryMap.table[number],
                    &actEntryMap.table[number + 1],
                    (actEntryMap.size - number - 1) * sizeof(actEntryType));
          } /* if */
          actEntryMap.size--;
          number--;
          /* printf("size=%u, number=%u\n", actEntryMap.size, number); */
        } /* if */
      } /* for */
    } /* if */
    logFunction(printf("genActPtrTable -->\n"););
  } /* genActPtrTable */



/**
 *  Get an actEntry that corresponds to actionSearched.
 *  @param actionSearched The action to be searched in actTable.
 *  @return a pointer to an actEntry. When actionSearched
 *          is not found a pointer to the actEntry of
 *          the action ACT_ILLEGAL is returned.
 */
const_actEntryType getActEntry (actType actionSearched)

  {
    int lower;
    int upper;
    int middle;
    const_actEntryType entryFound;

  /* getActEntry */
    logFunction(printf("getActEntry(" FMT_U_MEM ")\n",
                       (memSizeType) actionSearched););
    if (unlikely(actEntryMap.table == NULL)) {
      genActPtrTable();
      if (unlikely(actEntryMap.table == NULL)) {
        actionSearched = actTable.table[0].action;
      } /* if */
    } /* if */

    entryFound = &actTable.table[0];
    if (likely(actionSearched != actTable.table[0].action)) {
      lower = -1;
      upper = (int) actEntryMap.size;
      while (lower + 1 < upper) {
        middle = (lower + upper) >> 1;
        /* printf("%d %d %d <" FMT_U_MEM "> <" FMT_U_MEM ">\n", lower, middle, upper,
            (memSizeType) actEntryMap.table[middle]->action,
            (memSizeType) actionSearched); */
        if (((memSizeType) actEntryMap.table[middle]->action) <
            ((memSizeType) actionSearched)) {
          lower = middle;
        } else if (actEntryMap.table[middle]->action == actionSearched) {
          lower = upper - 1;
          entryFound = actEntryMap.table[middle];
        } else {
          upper = middle;
        } /* if */
      } /* while */
    } /* if */
    logFunction(printf("getActEntry --> %s\n", entryFound->name););
    return entryFound;
  } /* getActEntry */
