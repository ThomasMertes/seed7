/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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



#ifdef ANSI_C

stritype actStr (acttype anAction)
#else

stritype actStr (anAction)
acttype anAction;
#endif

  {
    char *stri;
    memsizetype len;
    stritype result;

  /* actStr */
    stri = get_primact(anAction)->name;
    len = (memsizetype) strlen(stri);
    if (!ALLOC_STRI(result, len)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result->size = len;
      cstri_expand(result->mem, stri, (size_t) len);
    } /* if */
    return(result);
  } /* actStr */
