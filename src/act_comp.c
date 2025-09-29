/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2008  Thomas Mertes                        */
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
/*  Module: Seed7 compiler library                                  */
/*  File: seed7/src/act_comp.c                                      */
/*  Changes: 1991, 1992, 1993, 1994, 2008  Thomas Mertes            */
/*  Content: Primitive actions for the action type.                 */
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
#include "actutl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "act_comp.h"



/**
 *  Convert a string to an action.
 *  @param actionName Name of the action to be converted.
 *  @return an action which corresponds to the given string.
 *  @exception RANGE_ERROR No such action exists.
 */
actType actGen (const const_striType actionName)

  {
    actType anAction;

  /* actGen */
    logFunction(printf("actGen(\"%s\")\n",
                       striAsUnquotedCStri(actionName)););
    anAction = findAction(actionName);
    if (unlikely(anAction == NULL)) {
      logError(printf("actGen(\"%s\"): No such action exists.\n",
                      striAsUnquotedCStri(actionName)););
      raise_error(RANGE_ERROR);
    } /* if */
    logFunction(printf("actGen --> " FMT_U_MEM "\n",
                       (memSizeType) anAction););
    return anAction;
  } /* actGen */



/**
 *  Convert an integer number to an action.
 *  @param ordinal Number to be converted.
 *  @return an action which corresponds to the given integer.
 *  @exception RANGE_ERROR Number not in allowed range.
 */
actType actIConv (intType ordinal)

  {
    actType anAction;

  /* actIConv */
    logFunction(printf("actIConv(" FMT_D ")\n", ordinal););
    if (unlikely(ordinal < 0 || (uintType) ordinal >= actTable.size)) {
      logError(printf("actIConv(" FMT_D "): "
                      "Number not in allowed range of 0 .. %lu.\n",
                      ordinal, (unsigned long) (actTable.size - 1)););
      raise_error(RANGE_ERROR);
      anAction = NULL;
    } else {
      anAction = actTable.table[ordinal].action;
    } /* if */
    logFunction(printf("actIConv --> " FMT_U_MEM "\n",
                       (memSizeType) anAction););
    return anAction;
  } /* actIConv */



/**
 *  Get the ordinal number of an action.
 *  The action ACT_ILLEGAL has the ordinal number 0.
 *  @param anAction Action for which the ordinal number is determined.
 *  @return the ordinal number of the action.
 */
intType actOrd (actType anAction)

  {
    intType ordinal;

  /* actOrd */
    logFunction(printf("actOrd(" FMT_U_MEM ")\n",
                       (memSizeType) anAction););
    ordinal = getActEntry(anAction) - actTable.table;
    logFunction(printf("actOrd --> " FMT_D "\n", ordinal););
    return ordinal;
  } /* actOrd */



/**
 *  Convert an action to a string.
 *  If the action is not found in the table of legal actions
 *  the string "ACT_ILLEGAL" is returned.
 *  @param anAction Action which is converted to a string..
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType actStr (actType anAction)

  {
    striType actionName;

  /* actStr */
    logFunction(printf("actStr(" FMT_U_MEM ")\n",
                       (memSizeType) anAction););
    actionName = cstri_to_stri(getActEntry(anAction)->name);
    if (unlikely(actionName == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("actStr --> \"%s\"\n",
                       striAsUnquotedCStri(actionName)););
    return actionName;
  } /* actStr */
