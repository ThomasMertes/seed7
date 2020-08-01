/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
#include "object.h"
#include "match.h"
#include "error.h"
#include "expr.h"
#include "exec.h"

#undef EXTERN
#define EXTERN
#include "type.h"


#undef TRACE_TYPE



#ifdef ANSI_C

objecttype decl_type (int *is_dollar_type, errinfotype *err_info)
#else

objecttype decl_type (is_dollar_type, err_info)
int *is_dollar_type;
errinfotype *err_info;
#endif

  {
    objecttype basic_type;
    objecttype evaluated_type;
    objecttype typeof_object;
    typetype func_type;

  /* decl_type */
#ifdef TRACE_TYPE
    printf("BEGIN decl_type\n");
#endif
    *is_dollar_type = 0;
    scan_symbol();
    if (current_ident == prog.id_for.colon) {
      err_warning(TYPE_EXPECTED);
      typeof_object = NULL;
      scan_symbol();
    } else {
      if (current_ident == prog.id_for.dollar) {
        scan_symbol();
        if (current_ident == prog.id_for.func) {
          scan_symbol();
          basic_type = pars_infix_expression(WEAKEST_PRIORITY, TRUE);
          if (CATEGORY_OF_OBJ(basic_type) == TYPEOBJECT) {
            if ((func_type = get_func_type(NULL, take_type(basic_type))) != NULL) {
              typeof_object = bld_type_temp(func_type);
            } else {
              *err_info = MEMORY_ERROR;
              typeof_object = NULL;
            } /* if */
          } else {
            err_object(TYPE_EXPECTED, basic_type);
            typeof_object = NULL;
          } /* if */
        } else if (current_ident == prog.id_for.type) {
          typeof_object = pars_infix_expression(WEAKEST_PRIORITY, TRUE);
          *is_dollar_type = 1;
        } else {
          err_warning(DOLLAR_TYPE_WRONG);
          typeof_object = NULL;
          if (current_ident != prog.id_for.colon) {
            scan_symbol();
          } /* if */
        } /* if */
      } else {
        typeof_object = pars_infix_expression(WEAKEST_PRIORITY, TRUE);
        /* printf("typeof_object=%lu ", (long unsigned) typeof_object);
        trace1(typeof_object);
        printf("\n"); */
        if (CATEGORY_OF_OBJ(typeof_object) != TYPEOBJECT) {
          /* printf("before evaluate\n"); */
          if ((evaluated_type = evaluate(typeof_object)) != NULL &&
              CATEGORY_OF_OBJ(evaluated_type) == TYPEOBJECT) {
            /* printf("evaluated_type ");
            trace1(evaluated_type);
            printf("\n"); */
            typeof_object = evaluated_type;
          } else {
            printf("\n typeof: ");
            trace1(typeof_object);
            printf("\n evaluated: ");
            trace1(evaluated_type);
            printf("\n");
            printf("&evaluated_type->type_of %lu\n", (long unsigned) evaluated_type->type_of);
            err_object(TYPE_EXPECTED, typeof_object);
          } /* if */
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
    trace1(typeof_object);
    printf("\n");
#endif
    return(typeof_object);
  } /* decl_type */
