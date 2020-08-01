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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
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

objecttype bst_append (listtype arguments)
#else

objecttype bst_append (arguments)
listtype arguments;
#endif

  {
    objecttype bstr_variable;
    bstritype bstr_to;
    bstritype bstr_from;
    memsizetype new_size;

  /* bst_append */
    bstr_variable = arg_1(arguments);
    isit_bstri(bstr_variable);
    is_variable(bstr_variable);
    bstr_to = take_bstri(bstr_variable);
    isit_bstri(arg_3(arguments));
    bstr_from = take_bstri(arg_3(arguments));
/*
    printf("bstr_to (%lx) %d = ", bstr_to, bstr_to->size);
    prot_stri(bstr_to);
    printf("\n");
    printf("bstr_from (%lx) %d = ", bstr_from, bstr_from->size);
    prot_stri(bstr_from);
    printf("\n");
*/
    if (bstr_from->size != 0) {
      new_size = bstr_to->size + bstr_from->size;
      REALLOC_BSTRI(bstr_to, bstr_to, bstr_to->size, new_size);
      if (bstr_to == NULL) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } /* if */
      COUNT3_BSTRI(bstr_to->size, new_size);
      bstr_variable->value.bstrivalue = bstr_to;
      memcpy(&bstr_to->mem[bstr_to->size], bstr_from->mem,
          (SIZE_TYPE) bstr_from->size * sizeof(uchartype));
      bstr_to->size = new_size;
/*
      printf("new bstr_to (%lx) %d = ", bstr_to, bstr_to->size);
      prot_stri(bstr_to);
      printf("\n");
      printf("new bstr_variable (%lx): ", take_stri(bstr_variable));
      trace1(bstr_variable);
      printf("\n");
*/
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* bst_append */



#ifdef ANSI_C

objecttype bst_cat (listtype arguments)
#else

objecttype bst_cat (arguments)
listtype arguments;
#endif

  {
    bstritype bstr1;
    bstritype bstr2;
    memsizetype bstr1_size;
    memsizetype result_size;
    bstritype result;

  /* bst_cat */
    isit_bstri(arg_1(arguments));
    isit_bstri(arg_3(arguments));
    bstr1 = take_bstri(arg_1(arguments));
    bstr2 = take_bstri(arg_3(arguments));
    bstr1_size = bstr1->size;
    result_size = bstr1_size + bstr2->size;
    if (TEMP_OBJECT(arg_1(arguments))) {
      REALLOC_BSTRI(result, bstr1, bstr1_size, result_size);
      if (result == NULL) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } /* if */
      COUNT3_STRI(bstr1_size, result_size);
      result->size = result_size;
      memcpy(&result->mem[bstr1_size], bstr2->mem,
          (SIZE_TYPE) bstr2->size * sizeof(uchartype));
      arg_1(arguments)->value.bstrivalue = NULL;
      return(bld_bstri_temp(result));
    } else {
      if (!ALLOC_BSTRI(result, result_size)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        result->size = result_size;
        memcpy(result->mem, bstr1->mem,
            (SIZE_TYPE) bstr1_size * sizeof(uchartype));
        memcpy(&result->mem[bstr1_size], bstr2->mem,
            (SIZE_TYPE) bstr2->size * sizeof(uchartype));
        return(bld_bstri_temp(result));
      } /* if */
    } /* if */
  } /* bst_cat */



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
