/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  Module: General                                                 */
/*  File: seed7/src/typeutl.c                                       */
/*  Changes: 1999  Thomas Mertes                                    */
/*  Content: Procedures to maintain objects of type typetype.       */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "identutl.h"
#include "entutl.h"

#undef EXTERN
#define EXTERN
#include "typeutl.h"



#ifdef ANSI_C

typetype new_type (typetype meta_type, typetype result_type)
#else

typetype new_type (meta_type, result_type)
typetype meta_type;
typetype result_type;
#endif

  {
    objecttype match_obj;
    typetype created_type;

  /* new_type */
#ifdef TRACE_TYPEUTIL
  printf("BEGIN new_type\n");
#endif
    if (ALLOC_OBJECT(match_obj)) {
      if (ALLOC_RECORD(created_type, typerecord)) {
        COUNT_RECORD(typerecord, count.type);
        match_obj->type_of = NULL;
        match_obj->entity = NULL;
        match_obj->value.typevalue = created_type;
        INIT_CLASS_OF_OBJ(match_obj, TYPEOBJECT);
        created_type->match_obj = match_obj;
        created_type->meta = meta_type;
        created_type->func_type = NULL;
        created_type->varfunc_type = NULL;
        created_type->result_type = result_type;
        created_type->is_varfunc_type = FALSE;
        created_type->name = NULL;
        created_type->create_call_obj = NULL;
        created_type->destroy_call_obj = NULL;
        created_type->copy_call_obj = NULL;
        created_type->ord_call_obj = NULL;
        created_type->in_call_obj = NULL;
      } else {
        FREE_OBJECT(match_obj);
        created_type = NULL;
      } /* if */
    } else {
      created_type = NULL;
    } /* if */
#ifdef TRACE_TYPEUTIL
  printf("END new_type\n");
#endif
    return(created_type);
  } /* new_type */



#ifdef ANSI_C

typetype get_func_type (typetype meta_type, typetype basic_type)
#else

typetype get_func_type (meta_type, basic_type)
typetype meta_type;
typetype basic_type;
#endif

  {
    typetype func_type;

  /* get_func_type */
    if (basic_type->func_type != NULL) {
      func_type = basic_type->func_type;
    } else {
      func_type = new_type(meta_type, basic_type);
      basic_type->func_type = func_type;
    } /* if */
    return(func_type);
  } /* get_func_type */



#ifdef ANSI_C

typetype get_varfunc_type (typetype meta_type, typetype basic_type)
#else

typetype get_varfunc_type (meta_type, basic_type)
typetype meta_type;
typetype basic_type;
#endif

  {
    typetype varfunc_type;

  /* get_varfunc_type */
    if (basic_type->varfunc_type != NULL) {
      varfunc_type = basic_type->varfunc_type;
    } else {
      varfunc_type = new_type(meta_type, basic_type);
      varfunc_type->is_varfunc_type = TRUE;
      basic_type->varfunc_type = varfunc_type;
    } /* if */
    return(varfunc_type);
  } /* get_varfunc_type */
