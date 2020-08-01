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



actType actIConv (intType ordinal)

  {
    actType anAction;

  /* actIConv */
    if (ordinal < 0 || ordinal >= act_table.size) {
      raise_error(RANGE_ERROR);
      anAction = NULL;
    } else {
      anAction = act_table.primitive[ordinal].action;
    } /* if */
    return anAction;
  } /* actIConv */



intType actOrd (actType anAction)

  {
    intType result;

  /* actOrd */
    result = get_primact(anAction) - act_table.primitive;
    return result;
  } /* actOrd */



striType actStr (actType anAction)

  {
    striType result;

  /* actStr */
    result = cstri_to_stri(get_primact(anAction)->name);
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* actStr */
