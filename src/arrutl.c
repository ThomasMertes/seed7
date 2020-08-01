/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  File: seed7/src/arrutl.c                                        */
/*  Changes: 2013  Thomas Mertes                                    */
/*  Content: Procedures to maintain objects of type arraytype.      */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "data_rtl.h"
#include "heaputl.h"

#undef EXTERN
#define EXTERN
#include "arrutl.h"



#ifdef ANSI_C

rtlArraytype gen_rtl_array (arraytype anArray)
#else

rtlArraytype gen_rtl_array (anArray)
arraytype anArray;
#endif

  {
    memsizetype arraySize;
    memsizetype pos;
    rtlArraytype rtlArray;

  /* gen_rtl_array */
    arraySize = (uinttype) (anArray->max_position - anArray->min_position + 1);
    if (ALLOC_RTL_ARRAY(rtlArray, arraySize)) {
      rtlArray->min_position = anArray->min_position;
      rtlArray->max_position = anArray->max_position;
      for (pos = 0; pos < arraySize; pos++) {
        rtlArray->arr[pos].value.strivalue = anArray->arr[pos].value.strivalue;
      } /* for */
    } /* if */
    return rtlArray;
  } /* gen_rtl_array */