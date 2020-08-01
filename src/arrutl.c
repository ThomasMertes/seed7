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
/*  Content: Procedures to maintain objects of type arrayType.      */
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



/**
 *  Generate temporary rtlArrayType array.
 *  The elements of the original array are copied with assignments.
 *  Create functions are not called. The result is intended to be
 *  used for a call that requires an rtlArrayType array. 
 *  The returned rtlArrayType array must be freed with FREE_RTL_ARRAY.
 */
rtlArrayType gen_rtl_array (arrayType anArray)

  {
    memSizeType arraySize;
    memSizeType pos;
    rtlArrayType rtlArray;

  /* gen_rtl_array */
    arraySize = arraySize(anArray);
    if (ALLOC_RTL_ARRAY(rtlArray, arraySize)) {
      rtlArray->min_position = anArray->min_position;
      rtlArray->max_position = anArray->max_position;
      for (pos = 0; pos < arraySize; pos++) {
        rtlArray->arr[pos].value.striValue = anArray->arr[pos].value.striValue;
      } /* for */
    } /* if */
    return rtlArray;
  } /* gen_rtl_array */
