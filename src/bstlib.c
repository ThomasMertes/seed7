/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2010  Thomas Mertes                        */
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
/*  Changes: 2007, 2010  Thomas Mertes                              */
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
#include "bst_rtl.h"

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
    bstritype new_bstr;
    memsizetype new_size;
    memsizetype bstr_to_size;

  /* bst_append */
    bstr_variable = arg_1(arguments);
    isit_bstri(bstr_variable);
    is_variable(bstr_variable);
    bstr_to = take_bstri(bstr_variable);
    isit_bstri(arg_3(arguments));
    bstr_from = take_bstri(arg_3(arguments));
    if (bstr_from->size != 0) {
      bstr_to_size = bstr_to->size;
      new_size = bstr_to_size + bstr_from->size;
      REALLOC_BSTRI(new_bstr, bstr_to, bstr_to_size, new_size);
      if (new_bstr == NULL) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        /* It is possible that bstr_to == bstr_from holds. */
        /* In this case 'bstr_from' must be corrected      */
        /* after realloc() enlarged 'bstr_to'.             */
        if (bstr_to == bstr_from) {
          bstr_from = new_bstr;
        } /* if */
        COUNT3_BSTRI(bstr_to_size, new_size);
        memcpy(&new_bstr->mem[bstr_to_size], bstr_from->mem,
            bstr_from->size * sizeof(uchartype));
        new_bstr->size = new_size;
        bstr_variable->value.bstrivalue = new_bstr;
      } /* if */
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
          (size_t) bstr2->size * sizeof(uchartype));
      arg_1(arguments)->value.bstrivalue = NULL;
      return(bld_bstri_temp(result));
    } else {
      if (!ALLOC_BSTRI(result, result_size)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        result->size = result_size;
        memcpy(result->mem, bstr1->mem,
            (size_t) bstr1_size * sizeof(uchartype));
        memcpy(&result->mem[bstr1_size], bstr2->mem,
            (size_t) bstr2->size * sizeof(uchartype));
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
    bstritype bstri_dest;

  /* bst_cpy */
    bstri_to = arg_1(arguments);
    bstri_from = arg_3(arguments);
    isit_bstri(bstri_to);
    isit_bstri(bstri_from);
    is_variable(bstri_to);
    bstri_dest = take_bstri(bstri_to);
    if (TEMP_OBJECT(bstri_from)) {
      FREE_BSTRI(bstri_dest, bstri_dest->size);
      bstri_to->value.bstrivalue = take_bstri(bstri_from);
      bstri_from->value.bstrivalue = NULL;
    } else {
      new_size = take_bstri(bstri_from)->size;
      if (bstri_dest->size == new_size) {
        /* It is possible that bstr_to == bstr_from holds. The */
        /* behavior of memcpy() is undefined when source and   */
        /* destination areas overlap (or are identical).       */
        /* Therefore a check for this case is necessary.       */
        if (bstri_dest != take_stri(bstri_from)) {
          memcpy(bstri_dest->mem, take_stri(bstri_from)->mem,
              new_size * sizeof(uchartype));
        } /* if */
      } else {
        if (!ALLOC_BSTRI(bstri_dest, new_size)) {
          return(raise_exception(SYS_MEM_EXCEPTION));
        } else {
          FREE_BSTRI(take_bstri(bstri_to), take_bstri(bstri_to)->size);
          bstri_to->value.bstrivalue = bstri_dest;
          bstri_dest->size = new_size;
        } /* if */
        memcpy(bstri_dest->mem, take_bstri(bstri_from)->mem,
            (size_t) new_size * sizeof(uchartype));
      } /* if */
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
          (size_t) new_size * sizeof(uchartype));
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



#ifdef ANSI_C

objecttype bst_eq (listtype arguments)
#else

objecttype bst_eq (arguments)
listtype arguments;
#endif

  {
    bstritype str1;
    bstritype str2;

  /* bst_eq */
    isit_bstri(arg_1(arguments));
    isit_bstri(arg_3(arguments));
    str1 = take_bstri(arg_1(arguments));
    str2 = take_bstri(arg_3(arguments));
    if (str1->size == str2->size && memcmp(str1->mem, str2->mem,
        str1->size * sizeof(uchartype)) == 0) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* bst_eq */



#ifdef ANSI_C

objecttype bst_lng (listtype arguments)
#else

objecttype bst_lng (arguments)
listtype arguments;
#endif

  { /* bst_lng */
    isit_bstri(arg_1(arguments));
    return(bld_int_temp((inttype) take_bstri(arg_1(arguments))->size));
  } /* bst_lng */



#ifdef ANSI_C

objecttype bst_ne (listtype arguments)
#else

objecttype bst_ne (arguments)
listtype arguments;
#endif

  {
    bstritype str1;
    bstritype str2;

  /* bst_ne */
    isit_bstri(arg_1(arguments));
    isit_bstri(arg_3(arguments));
    str1 = take_bstri(arg_1(arguments));
    str2 = take_bstri(arg_3(arguments));
    if (str1->size != str2->size || memcmp(str1->mem, str2->mem,
        str1->size * sizeof(uchartype)) != 0) {
      return(SYS_TRUE_OBJECT);
    } else {
      return(SYS_FALSE_OBJECT);
    } /* if */
  } /* bst_ne */



#ifdef ANSI_C

objecttype bst_parse (listtype arguments)
#else

objecttype bst_parse (arguments)
listtype arguments;
#endif

  { /* bst_parse */
    isit_stri(arg_3(arguments));
    return(bld_bstri_temp(bstParse(
        take_stri(arg_3(arguments)))));
  } /* bst_parse */



#ifdef ANSI_C

objecttype bst_value (listtype arguments)
#else

objecttype bst_value (arguments)
listtype arguments;
#endif

  {
    objecttype obj_arg;
    bstritype str1;
    bstritype result;

  /* bst_value */
    isit_reference(arg_1(arguments));
    obj_arg = take_reference(arg_1(arguments));
    if (obj_arg == NULL || CATEGORY_OF_OBJ(obj_arg) != BSTRIOBJECT ||
        take_bstri(obj_arg) == NULL) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } else {
      str1 = take_bstri(obj_arg);
      if (!ALLOC_BSTRI(result, str1->size)) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        result->size = str1->size;
        memcpy(result->mem, str1->mem,
            result->size * sizeof(uchartype));
        return(bld_bstri_temp(result));
      } /* if */
    } /* if */
  } /* bst_value */
