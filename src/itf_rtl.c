/********************************************************************/
/*                                                                  */
/*  itf_rtl.c     Primitive actions for the interface type.         */
/*  Copyright (C) 1989 - 2012  Thomas Mertes                        */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/itf_rtl.c                                       */
/*  Changes: 2012  Thomas Mertes                                    */
/*  Content: Primitive actions for the interface type.              */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "itf_rtl.h"



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 */
rtlInterfaceType itfCreate (const rtlInterfaceType source)

  { /* itfCreate */
    if (source->usage_count != 0) {
      source->usage_count++;
    } /* if */
    return source;
  } /* itfCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  when sizeof(genericType) != sizeof(rtlInterfaceType).
 */
genericType itfCreateGeneric (const genericType from_value)

  {
    rtlObjectType result;

  /* itfCreateGeneric */
    INIT_GENERIC_PTR(result.value.genericValue);
    result.value.interfaceValue =
        itfCreate(((const_rtlObjectType *) &from_value)->value.interfaceValue);
    return result.value.genericValue;
  } /* itfCreateGeneric */



#ifdef OUT_OF_ORDER
objectType itfToHeap (listType arguments)

  {
    objectType modu_from;
    objectType result;

  /* itfToHeap */
    modu_from = arg_1(arguments);
    printf("itfToHeap: ");
       trace1(modu_from);
       printf("\n");
    if (CATEGORY_OF_OBJ(modu_from) == INTERFACEOBJECT) {
      result = take_reference(modu_from);
    } else if (CATEGORY_OF_OBJ(modu_from) == STRUCTOBJECT) {
      if (TEMP2_OBJECT(modu_from)) {
        if (!ALLOC_OBJECT(result)) {
          return raise_exception(SYS_MEM_EXCEPTION);
        } else {
          memcpy(result, modu_from, sizeof(objectRecord));
          CLEAR_TEMP2_FLAG(result);
          result->value.structValue = take_struct(modu_from);
          modu_from->value.structValue = NULL;
        } /* if */
      } else {
        result = modu_from;
      } /* if */
    } else {
      return raise_exception(SYS_RNG_EXCEPTION);
    } /* if */
    result = bld_interface_temp(result);
    printf("itfToHeap --> ");
       trace1(result);
       printf("\n");
    return result;
  } /* itfToHeap */
#endif



rtlInterfaceType itfToInterface (rtlStructType *stru_arg)

  {
    rtlInterfaceType result;

  /* itfToInterface */
    result = *stru_arg;
    *stru_arg = NULL;
    return result;
  } /* itfToInterface */
