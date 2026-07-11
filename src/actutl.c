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
/*  USE_BSEARCH flag it is possible to use the bsearch function of  */
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
#include "listutl.h"
#include "traceutl.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "actutl.h"


#define USE_BSEARCH 0
#define MAX_CSTRI_BUFFER_LEN 40


typedef struct {
    unsigned int size;
    const_actEntryType *table;
} actEntryMapType;

static actEntryMapType actEntryMap = {0, NULL};



#if USE_BSEARCH
#ifdef USE_CDECL
static int _cdecl actTableCmp (char *strg1, char *strg2)
#else
static int actTableCmp (void const *strg1, void const *strg2)
#endif

  {
    int comparison;

  /* actTableCmp */
    logFunction(printf("actTableCmp(\"%s\", \"%s\")\n",
                       strg1, ((actEntryType) strg2)->name););
    comparison = strcmp(strg1, ((actEntryType) strg2)->name);
    logFunction(printf("actTableCmp --> %d\n", comparison););
    return comparison;
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
static const_actEntryType searchAction (cstriType actionName)

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
    const_actEntryType actEntryFound;

  /* searchAction */
    logFunction(printf("searchAction(\"%s\")\n", actionName););
    if (likely(actTable.table != NULL)) {
#if USE_BSEARCH
      if ((found = (actEntryType) bsearch(actionName, &actTable.table[1],
          actTable.size - 1, sizeof(actEntryRecord), actTableCmp)) != NULL) {
        actionNumber = (unsigned int) (found - &actTable.table[0]);
      } else {
        actionNumber = 0;
      } /* if */
#else
      actionNumber = 0;
      lower = 0;
      upper = actTable.size;
      while (lower + 1 < upper) {
        middle = (lower + upper) >> 1;
        logMessage(printf("%u %u %u >%s< >%s<\n",
                          lower, middle, upper,
                          actTable.table[middle].name, actionName););
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
      logMessage(printf("action number: %u\n", actionNumber););
      logMessage(printf("action name: \"%s\"\n",
                        actTable.table[actionNumber].name););
      if (likely(actionNumber != 0)) {
        actEntryFound = &actTable.table[actionNumber];
      } else if (strcmp(actTable.table[0].name, actionName) == 0) {
        actEntryFound = &actTable.table[0];
      } else {
        logError(printf("searchAction(\"%s\"): "
                        "No such action exists.\n",
                        actionName););
        actEntryFound = NULL;
      } /* if */
    } else {
      logError(printf("searchAction(\"%s\"): "
                      "actTable.table does not exist.\n",
                      actionName););
      actEntryFound = NULL;
    } /* if */
    logFunction(printf("searchAction --> " FMT_U_MEM "\n",
                       (memSizeType) actEntryFound););
    return actEntryFound;
  } /* searchAction */



/**
 *  Find the action entry with the given ''actionName''.
 *  @param actionName Name of the action searched.
 *  @return The action entry found, or NULL if no such action exists.
 */
const_actEntryType findActEntry (const const_striType actionName)

  {
    char actName[MAX_CSTRI_BUFFER_LEN + NULL_TERMINATION_LEN];
    const_actEntryType actEntryFound;

  /* findActEntry */
    logFunction(printf("findActEntry(\"%s\")\n",
                       striAsUnquotedCStri(actionName)););
    if (unlikely(actionName->size > MAX_CSTRI_BUFFER_LEN)) {
      actEntryFound = NULL;
    } else {
      if (unlikely(conv_to_cstri(actName, actionName) == NULL)) {
        actEntryFound = NULL;
      } else {
        actEntryFound = searchAction(actName);
      } /* if */
    } /* if */
    logFunction(printf("findActEntry --> " FMT_U_MEM "\n",
                       (memSizeType) actEntryFound););
    return actEntryFound;
  } /* findActEntry */



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
      logError(printf("getActIllegal: actTable.table is NULL.\n"););
      actionFound = NULL;
    } /* if */
    return actionFound;
  } /* getActIllegal */



/**
 *  Create actEntryMap and remove double actType values from it.
 *  The table actEntryMap is used by getActEntry() to map actType
 *  function pointers to corresponding entries in actTable. If
 *  the C compiler recognizes that the code of two functions is
 *  identical it may decide to reuse the code. In this case two
 *  or more actType function pointers refer to the same function.
 *  To have predictable results double actType values are removed
 *  from actEntryMap. The actType function pointer that refers to
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
    if (ALLOC_TABLE(actEntryMap.table, const_actEntryType, actEntryMap.size)) {
      for (number = 0; number < actEntryMap.size; number++) {
        actEntryMap.table[number] = &actTable.table[number];
      } /* for */
      qsort(actEntryMap.table, actEntryMap.size, sizeof(actEntryType),
          actEntryMapCmp);
      for (number = 1; number < actEntryMap.size; number++) {
        if (actEntryMap.table[number]->action ==
            actEntryMap.table[number - 1]->action) {
          logMessage(printf("*** Actions %s and %s implemented "
                            "by the same function\n",
                            actEntryMap.table[number - 1]->name,
                            actEntryMap.table[number]->name););
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
          logMessage(printf("size=%u, number=%u\n",
                            actEntryMap.size, number););
        } /* if */
      } /* for */
    } /* if */
    logFunction(printf("genActPtrTable -->\n"););
  } /* genActPtrTable */



void freeActPtrTable (void)

  { /* freeActPtrTable */
    logFunction(printf("freeActPtrTable\n"););
    if (actEntryMap.table != NULL) {
      FREE_TABLE(actEntryMap.table, const_actEntryType, actEntryMap.size);
      actEntryMap.table = NULL;
    } /* if */
    logFunction(printf("freeActPtrTable -->\n"););
  } /* freeActPtrTable */



/**
 *  Get an actEntry that corresponds to actionSearched.
 *  @param actionSearched The action to be searched in actTable.
 *  @return a pointer to an actEntry. If actionSearched
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
        logError(printf("getActEntry: genActPtrTable() failed to "
                        "allocate actEntryMap.table.\n"););
        actionSearched = actTable.table[0].action;
      } /* if */
    } /* if */

    entryFound = &actTable.table[0];
    if (likely(actionSearched != actTable.table[0].action)) {
      lower = -1;
      upper = (int) actEntryMap.size;
      while (lower + 1 < upper) {
        middle = (lower + upper) >> 1;
        logMessage(printf("%d %d %d <" FMT_U_MEM "> <" FMT_U_MEM ">\n",
                          lower, middle, upper,
                          (memSizeType) actEntryMap.table[middle]->action,
                          (memSizeType) actionSearched););
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



boolType actionCreateOkay (objectType dest, const_actEntryType actEntry)

  {
    typeType destType;
    listType currParam;
    int index;
    typeType paramType;
    boolType okay = TRUE;

  /* actionCreateOkay */
    if (HAS_PROPERTY(dest)) {
      if (unlikely(list_length(dest->descriptor.property->params) <
                   actEntry->numParams)) {
        logError(printf("act_create/prc_create: action \"%s\": "
                        "Params found: " FMT_U_MEM ", expected: %u\n",
                        actEntry->name,
                        list_length(dest->descriptor.property->params),
                        actEntry->numParams););
        okay = FALSE;
      } /* if */
    } else if (unlikely(actEntry->numParams != 0)) {
      logError(printf("act_create/prc_create: action \"%s\": "
                      "Params found: 0, expected: %u\n",
                      actEntry->name,
                      actEntry->numParams););
      okay = FALSE;
    } /* if */
    if (okay) {
      destType = dest->type_of;
      if (destType->result_type != NULL) {
        destType = destType->result_type;
      } /* if */
      if (destType->value_category == ILLEGALOBJECT) {
        logMessage(printf("act_create/prc_create: action \"%s\" %d ",
                          actEntry->name, actEntry->resultCategory);
                   printtype(destType);
                   printf("\n"););
        destType->value_category = actEntry->resultCategory;
      } else if (actEntry->resultCategory != ILLEGALOBJECT) {
        if (destType->value_category != actEntry->resultCategory) {
          logError(printf("act_create/prc_create: action \"%s\": "
                          "Result category found: %d (",
                          actEntry->name, destType->value_category);
                   printtype(destType);
                   printf("), expected: %d\n",
                          actEntry->resultCategory););
          okay = FALSE;
        } /* if */
      } /* if */
    } /* if */
    if (okay) {
      if (dest->descriptor.property->params != NULL) {
        currParam = dest->descriptor.property->params;
        for (index = 0; index < actEntry->numParams; index++) {
          paramType = currParam->obj->type_of;
          if (paramType != NULL) {
            if (paramType->value_category == ILLEGALOBJECT &&
                actEntry->paramCategories[index] == BLOCKOBJECT) {
              logMessage(printf("act_create/prc_create: action \"%s\" %d %d ",
                                actEntry->name, index, actEntry->paramCategories[index]);
                         printtype(paramType);
                         printf("\n"););
              paramType->value_category = actEntry->paramCategories[index];
            } /* if */
            if (paramType->result_type != NULL) {
              paramType = paramType->result_type;
            } /* if */
            if (paramType->value_category == ILLEGALOBJECT && (
                actEntry->paramCategories[index] == TYPEOBJECT ||
                actEntry->paramCategories[index] == INTOBJECT ||
                actEntry->paramCategories[index] == BIGINTOBJECT ||
                actEntry->paramCategories[index] == CHAROBJECT ||
                actEntry->paramCategories[index] == STRIOBJECT ||
                actEntry->paramCategories[index] == BSTRIOBJECT ||
                actEntry->paramCategories[index] == ARRAYOBJECT ||
                actEntry->paramCategories[index] == HASHOBJECT ||
                actEntry->paramCategories[index] == HASHELEMOBJECT ||
                actEntry->paramCategories[index] == STRUCTOBJECT ||
                actEntry->paramCategories[index] == STRUCTELEMOBJECT ||
                actEntry->paramCategories[index] == INTERFACEOBJECT ||
                actEntry->paramCategories[index] == SETOBJECT ||
                actEntry->paramCategories[index] == FILEOBJECT ||
                actEntry->paramCategories[index] == SOCKETOBJECT ||
                actEntry->paramCategories[index] == POLLOBJECT ||
                actEntry->paramCategories[index] == FLOATOBJECT ||
                actEntry->paramCategories[index] == WINOBJECT ||
                actEntry->paramCategories[index] == POINTLISTOBJECT ||
                actEntry->paramCategories[index] == PROCESSOBJECT ||
                actEntry->paramCategories[index] == REFOBJECT ||
                actEntry->paramCategories[index] == REFLISTOBJECT ||
                actEntry->paramCategories[index] == EXPROBJECT ||
                actEntry->paramCategories[index] == ACTENTRYOBJECT ||
                actEntry->paramCategories[index] == DATABASEOBJECT ||
                actEntry->paramCategories[index] == SQLSTMTOBJECT ||
                actEntry->paramCategories[index] == PROGOBJECT ||
                actEntry->paramCategories[index] == BOOLOBJECT ||
                actEntry->paramCategories[index] == VOIDOBJECT)) {
              logMessage(printf("act_create/prc_create: action \"%s\" %d %d ",
                                actEntry->name, index, actEntry->paramCategories[index]);
                         printtype(paramType);
                         printf("\n"););
              paramType->value_category = actEntry->paramCategories[index];
            } else if (actEntry->paramCategories[index] == INTERFACEOBJECT) {
              if (paramType->value_category != INTERFACEOBJECT &&
                  paramType->value_category != STRUCTOBJECT) {
                logError(printf("act_create/prc_create: action \"%s\": "
                                "parameter %d: category found: %d (",
                                actEntry->name, index + 1,
                                paramType->value_category);
                         printtype(paramType);
                         printf(") expected INTERFACEOBJECT or STRUCTOBJECT\n"););
                okay = FALSE;
              } /* if */
            } else if (actEntry->paramCategories[index] == BLOCKOBJECT) {
              if (currParam->obj->type_of->value_category != ILLEGALOBJECT &&
                  currParam->obj->type_of->value_category != BLOCKOBJECT) {
                logError(printf("act_create/prc_create: action \"%s\": "
                                "parameter %d: category found: %d (",
                                actEntry->name, index + 1,
                                currParam->obj->type_of->value_category);
                         printtype(currParam->obj->type_of);
                         printf(") expected ILLEGALOBJECT or BLOCKOBJECT\n"););
                okay = FALSE;
              } /* if */
            } else if (actEntry->paramCategories[index] == ACTOBJECT) {
              if (currParam->obj->type_of->value_category != ILLEGALOBJECT) {
                logError(printf("act_create/prc_create: action \"%s\": "
                                "parameter %d: category found: %d (",
                                actEntry->name, index + 1,
                                currParam->obj->type_of->value_category);
                         printtype(currParam->obj->type_of);
                         printf(") expected ILLEGALOBJECT\n"););
                okay = FALSE;
              } /* if */
            } else if (actEntry->paramCategories[index] != ILLEGALOBJECT &&
                       actEntry->paramCategories[index] != SYMBOLOBJECT &&
                       actEntry->paramCategories[index] != ENUMOBJECT) {
              if (paramType->value_category != actEntry->paramCategories[index]) {
                logError(printf("act_create/prc_create: action \"%s\": "
                                "parameter %d: category found: %d (",
                                actEntry->name, index + 1,
                                paramType->value_category);
                         printtype(paramType);
                         printf(") expected %d\n",
                                actEntry->paramCategories[index]););
                okay = FALSE;
              } /* if */
            } /* if */
          } /* if */
          currParam = currParam->next;
        } /* for */
      } /* if */
    } /* if */
    logFunction(printf("actionCreateOkay --> %d\n", okay););
    return okay;
  } /* actionCreateOkay */
