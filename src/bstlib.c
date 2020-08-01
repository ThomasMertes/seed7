/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2007  Thomas Mertes                        */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/bstlib.c                                        */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: All primitive actions for the byte string type.        */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "chclsutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "runerr.h"
#include "memory.h"
#include "str_rtl.h"

#undef EXTERN
#define EXTERN
#include "strlib.h"



#ifdef ANSI_C

objecttype bst_cpy (listtype arguments)
#else

objecttype bst_cpy (arguments)
listtype arguments;
#endif

  {
    objecttype bstri_to;
    objecttype bstri_from;
    memsizetype new_size;
    bstritype new_bstri;

  /* bst_cpy */
    bstri_to = arg_1(arguments);
    bstri_from = arg_3(arguments);
    isit_bstri(bstri_to);
    isit_bstri(bstri_from);
    is_variable(bstri_to);
    new_bstri = take_bstri(bstri_to);
    if (TEMP_OBJECT(bstri_from)) {
      FREE_BSTRI(new_bstri, new_bstri->size);
      bstri_to->value.bstrivalue = take_bstri(bstri_from);
      bstri_from->value.bstrivalue = NULL;
    } else {
      new_size = take_bstri(bstri_from)->size;
      if (new_bstri->size != new_size) {
        if (!ALLOC_BSTRI(new_bstri, new_size)) {
          return(raise_exception(SYS_MEM_EXCEPTION));
        } else {
          FREE_BSTRI(take_bstri(bstri_to), take_bstri(bstri_to)->size);
          bstri_to->value.bstrivalue = new_bstri;
          new_bstri->size = new_size;
        } /* if */
      } /* if */
      memcpy(new_bstri->mem, take_bstri(bstri_from)->mem,
          (SIZE_TYPE) new_size * sizeof(uchartype));
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* bst_cpy */



#ifdef ANSI_C

objecttype bst_create (listtype arguments)
#else

objecttype bst_create (arguments)
listtype arguments;
#endif

  {
    objecttype bstri_to;
    objecttype bstri_from;
    memsizetype new_size;
    bstritype new_bstri;

  /* bst_create */
    bstri_to = arg_1(arguments);
    bstri_from = arg_3(arguments);
    isit_bstri(bstri_from);
    SET_CATEGORY_OF_OBJ(bstri_to, BSTRIOBJECT);
    if (TEMP_OBJECT(bstri_from)) {
      bstri_to->value.bstrivalue = take_bstri(bstri_from);
      bstri_from->value.bstrivalue = NULL;
    } else {
/*    printf("bstri_create %d !!!\n", in_file.line); */
      new_size = take_bstri(bstri_from)->size;
      if (!ALLOC_BSTRI(new_bstri, new_size)) {
        bstri_to->value.bstrivalue = NULL;
        return(raise_exception(SYS_MEM_EXCEPTION));
      } /* if */
      bstri_to->value.bstrivalue = new_bstri;
      new_bstri->size = new_size;
      memcpy(new_bstri->mem, take_bstri(bstri_from)->mem,
          (SIZE_TYPE) new_size * sizeof(uchartype));
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* bst_create */



#ifdef ANSI_C

objecttype bst_destr (listtype arguments)
#else

objecttype bst_destr (arguments)
listtype arguments;
#endif

  {
    bstritype old_bstring;

  /* bst_destr */
    isit_bstri(arg_1(arguments));
    old_bstring = take_bstri(arg_1(arguments));
    if (old_bstring != NULL) {
      FREE_BSTRI(old_bstring, old_bstring->size);
      arg_1(arguments)->value.bstrivalue = NULL;
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* bst_destr */



#ifdef ANSI_C

objecttype bst_empty (listtype arguments)
#else

objecttype bst_empty (arguments)
listtype arguments;
#endif

  {
    bstritype result;

  /* bst_empty */
    if (!ALLOC_BSTRI(result, 0)) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      result->size = 0;
      return(bld_bstri_temp(result));
    } /* if */
  } /* bst_empty */
