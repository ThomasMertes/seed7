/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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
/*  Module: Analyzer - Parser                                       */
/*  File: seed7/src/type.c                                          */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Parse and assign a type of an object.                  */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "typeutl.h"
#include "traceutl.h"
#include "fatal.h"
#include "scanner.h"
#include "findid.h"
#include "objutl.h"
#include "listutl.h"
#include "object.h"
#include "match.h"
#include "error.h"
#include "expr.h"
#include "exec.h"

#undef EXTERN
#define EXTERN
#include "type.h"


#undef TRACE_TYPE



objectType decl_type (int *is_dollar_type, errInfoType *err_info)

  {
    objectType basic_type;
    objectType type_expression;
    objectType type_of_object;
    typeType func_type;

  /* decl_type */
#ifdef TRACE_TYPE
    printf("BEGIN decl_type\n");
#endif
    *is_dollar_type = 0;
    scan_symbol();
    if (current_ident == prog.id_for.colon) {
      err_warning(TYPE_EXPECTED);
      type_of_object = NULL;
      scan_symbol();
    } else {
      if (current_ident == prog.id_for.dollar) {
        scan_symbol();
        if (current_ident == prog.id_for.func) {
          scan_symbol();
          basic_type = pars_infix_expression(WEAKEST_PRIORITY, TRUE);
          if (CATEGORY_OF_OBJ(basic_type) == TYPEOBJECT) {
            if ((func_type = get_func_type(NULL, take_type(basic_type))) != NULL) {
              type_of_object = bld_type_temp(func_type);
            } else {
              *err_info = MEMORY_ERROR;
              type_of_object = NULL;
            } /* if */
          } else {
            err_object(TYPE_EXPECTED, basic_type);
            type_of_object = NULL;
          } /* if */
        } else if (current_ident == prog.id_for.type) {
          type_of_object = pars_infix_expression(WEAKEST_PRIORITY, TRUE);
          *is_dollar_type = 1;
        } else {
          err_warning(DOLLAR_TYPE_WRONG);
          type_of_object = NULL;
          if (current_ident != prog.id_for.colon) {
            scan_symbol();
          } /* if */
        } /* if */
      } else {
        type_expression = pars_infix_expression(WEAKEST_PRIORITY, TRUE);
        /* printf("type_expression=%lu ", (long unsigned) type_expression);
        trace1(type_expression);
        printf("\n"); */
        if (CATEGORY_OF_OBJ(type_expression) != TYPEOBJECT) {
          /* printf("before evaluate\n"); */
          if ((type_of_object = evaluate(type_expression)) != NULL &&
              CATEGORY_OF_OBJ(type_of_object) == TYPEOBJECT) {
            /* printf("type_of_object ");
            trace1(type_of_object);
            printf("\n"); */
          } else {
            /* printf("\n type expression: ");
            trace1(type_expression);
            printf("\n evaluated: ");
            trace1(type_of_object);
            printf("\n");
            printf("&type_of_object->type_of %lu\n", (long unsigned) type_of_object->type_of); */
            err_object(TYPE_EXPECTED, type_expression);
          } /* if */
          free_expression(type_expression);
        } else {
          type_of_object = type_expression;
        } /* if */
      } /* if */
      if (current_ident == prog.id_for.colon) {
        scan_symbol();
      } else {
        err_ident(EXPECTED_SYMBOL, prog.id_for.colon);
      } /* if */
    } /* if */
#ifdef TRACE_TYPE
    printf("END decl_type --> ");
    trace1(type_of_object);
    printf("\n");
#endif
    return type_of_object;
  } /* decl_type */
