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
/*  Module: Analyzer - Parser                                       */
/*  File: seed7/src/expr.c                                          */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Read the next expression from the source file.         */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "listutl.h"
#include "syvarutl.h"
#include "datautl.h"
#include "traceutl.h"
#include "infile.h"
#include "scanner.h"
#include "symbol.h"
#include "object.h"
#include "findid.h"
#include "error.h"
#include "atom.h"
#include "match.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "expr.h"

#undef TRACE_EXPR



static objectType select_element (objectType expression,
    intType position)

  {
    listType list_element;
    intType number;
    objectType result;

  /* select_element */
    logFunction(printf("select_element\n"););
    list_element = expression->value.listValue;
    if (position >= 1) {
      number = 1;
      while (number < position && list_element != NULL) {
        number++;
        list_element = list_element->next;
      } /* while */
      if (list_element != NULL) {
        result = list_element->obj;
      } else {
        result = SYS_EMPTY_OBJECT;
      } /* if */
    } else {
      result = SYS_EMPTY_OBJECT;
    } /* if */
    free_list(expression->value.listValue);
    FREE_OBJECT(expression);
    logFunction(printf("select_element -->\n"););
    return result;
  } /* select_element */



static objectType read_call_expression (boolType do_match_expr)

  {
    objectType expression;
    fileNumType file_number;
    lineNumType line;
    listType helplist;
    objectType procnameobject;

  /* read_call_expression */
    logFunction(printf("read_call_expression(%d) %s\n",
                       do_match_expr, id_string(current_ident)););
    if (current_ident == prog.id_for.lparen) {
      scan_symbol();
      if (current_ident == prog.id_for.rparen) {
        err_warning(EXPR_EXPECTED);
        scan_symbol();
        expression = SYS_EMPTY_OBJECT;
      } else {
        expression = pars_infix_expression(WEAKEST_PRIORITY,
            do_match_expr);
        if (current_ident == prog.id_for.rparen) {
          scan_symbol();
        } else {
          err_ident(EXPECTED_SYMBOL, prog.id_for.rparen);
        } /* if */
      } /* if */
    } else {
      file_number = in_file.file_number;
      line = in_file.line;
      expression = read_atom();
      if (current_ident == prog.id_for.lparen) {
        scan_symbol();
        if (current_ident == prog.id_for.rparen) {
          scan_symbol();
        } else {
          procnameobject = expression;
          expression = new_nonempty_expression_object(
              pars_infix_expression(COM_PRIORITY, do_match_expr),
              &helplist, SYS_EXPR_TYPE);
          expression->descriptor.posinfo = CREATE_POSINFO(line, file_number);
          while (current_ident == prog.id_for.comma) {
            scan_symbol();
            helplist = add_element_to_list(helplist,
                pars_infix_expression(COM_PRIORITY, do_match_expr));
          } /* while */
          helplist = add_element_to_list(helplist, procnameobject);
          if (current_ident == prog.id_for.rparen) {
            scan_symbol();
          } else {
            err_ident(EXPECTED_SYMBOL, prog.id_for.rparen);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("read_call_expression --> " FMT_U_MEM ", ",
                       (memSizeType) expression);
                trace1(expression);
                printf("\n"););
    return expression;
  } /* read_call_expression */



static objectType read_dot_subexpression (boolType do_match_expr)

  {
    objectType expression;
    listType helplist;

  /* read_dot_subexpression */
    logFunction(printf("read_dot_subexpression(%d)\n", do_match_expr););
    if (current_ident == prog.id_for.lparen) {
      scan_symbol();
      if (current_ident == prog.id_for.rparen) {
        scan_symbol();
        expression = new_empty_list_object(SYS_EXPR_TYPE);
      } else {
        expression = new_nonempty_expression_object(
            pars_infix_expression(COM_PRIORITY, FALSE), &helplist,
            SYS_EXPR_TYPE);
#ifdef OUT_OF_ORDER
        SET_CATEGORY_OF_OBJ(expression, LISTOBJECT);
        while (current_ident == prog.id_for.comma) {
          scan_symbol();
          helplist = add_element_to_list(helplist,
              pars_infix_expression(COM_PRIORITY, FALSE));
        } /* while */
#endif
        if (current_ident == prog.id_for.rparen) {
          scan_symbol();
        } else {
          err_ident(EXPECTED_SYMBOL, prog.id_for.rparen);
        } /* if */
      } /* if */
    } else {
      expression = read_call_expression(do_match_expr);
    } /* if */
    logFunction(printf("read_dot_subexpression --> " FMT_U_MEM ", ",
                       (memSizeType) expression);
                trace1(expression);
                printf("\n"););
    return expression;
  } /* read_dot_subexpression */



static objectType read_dot_expression (boolType do_match_expr)

  {
    objectType expression;
    objectType sub_expr;
    listType helplist;

  /* read_dot_expression */
    logFunction(printf("read_dot_expression(%d) %s\n",
                       do_match_expr, id_string(current_ident)););
    if (current_ident == prog.id_for.dot) {
      scan_symbol();
      expression = read_dot_subexpression(do_match_expr);
      if (current_ident == prog.id_for.dot) {
        expression = new_nonempty_expression_object(expression, &helplist,
            SYS_EXPR_TYPE);
        SET_CATEGORY_OF_OBJ(expression, LISTOBJECT);
        do {
          scan_symbol();
          sub_expr = read_dot_subexpression(do_match_expr);
#ifdef TRACE_EXPR
          printf("%lu ", (unsigned long) expression);
          trace1(expression);
          printf("\n");
#endif
          helplist = add_element_to_list(helplist, sub_expr);
#ifdef TRACE_EXPR
          printf("%lu ", (unsigned long) expression);
          trace1(expression);
          printf("\n");
#endif
        } while (current_ident == prog.id_for.dot);
      } /* if */
    } else {
      expression = read_call_expression(do_match_expr);
    } /* if */
    logFunction(printf("read_dot_expression --> " FMT_U_MEM ", ",
                       (memSizeType) expression);
                trace1(expression);
                printf("\n"););
    return expression;
  } /* read_dot_expression */



static objectType pars_token (objectType expression,
    const_tokenType formal_token, listType actual_parameter)

  {
    boolType okay;
    objectType actual_param;
    listType helplist;
    posType posinfo;

  /* pars_token */
    logFunction(printf("pars_token\n"););
    okay = FALSE;
    while (formal_token != NULL) {
      switch (formal_token->token_category) {
        case SY_TOKEN:
/* printf("!SY_TOKEN  >%s< \n", formal_token->token_value.ident->name); */
          if (current_ident == formal_token->token_value.ident) {
            actual_parameter = add_element_to_list(actual_parameter, read_name());
            formal_token = formal_token->next;
          } else {
            if (formal_token->alternative != NULL) {
              formal_token = formal_token->alternative;
            } else {
              err_string(UNEXPECTED_SYMBOL,
                  formal_token->token_value.ident->name);
              scan_symbol();
              formal_token = formal_token->next;
            } /* if */
          } /* if */
          break;
        case EXPR_TOKEN:
/* printf("!EXPR_TOKEN  [] %d  >%s< %d\n", formal_token->token_value.expr_par.priority,
   symbol.name, current_ident->prefix_priority); */
          if (current_ident->prefix_priority <=
              formal_token->token_value.expr_par.priority) {
            if (formal_token->token_value.expr_par.type_of == NULL) {
              actual_param = pars_infix_expression(
                  formal_token->token_value.expr_par.priority, FALSE);
            } else {
              posinfo = CREATE_POSINFO(in_file.line, in_file.file_number);
              actual_param = pars_infix_expression(
                  formal_token->token_value.expr_par.priority, FALSE);
              if (CATEGORY_OF_OBJ(actual_param) != EXPROBJECT) {
                actual_param = new_type_of_expression_object(
                    actual_param, &helplist,
                    formal_token->token_value.expr_par.type_of);
                actual_param->descriptor.posinfo = posinfo;
              } /* if */
            } /* if */
            actual_parameter = add_element_to_list(actual_parameter, actual_param);
            formal_token = formal_token->next;
          } else {
            formal_token = formal_token->alternative;
          } /* if */
          break;
        case UNDEF_SYNTAX:
          break;
        case LIST_WITH_TYPEOF_SYNTAX:
          expression->type_of = formal_token->token_value.type_of;
          formal_token = NULL;
          okay = TRUE;
          break;
        case SELECT_ELEMENT_FROM_LIST_SYNTAX:
          expression = select_element(expression,
              formal_token->token_value.select);
          formal_token = NULL;
          okay = TRUE;
          break;
      } /* switch */
    } /* while */
    if (!okay) {
      err_warning(EXPR_EXPECTED);
    } /* if */
    logFunction(printf("pars_token --> " FMT_U_MEM ", ",
                       (memSizeType) expression);
                trace1(expression);
                printf("\n"););
    return expression;
  } /* pars_token */



objectType pars_infix_expression (priorityType priority,
    boolType do_match_expr)

{
    objectType expression;
    priorityType expr_prior;
    tokenType formal_token;
    listType helplist;

  /* pars_infix_expression */
    logFunction(printf("pars_infix_expression %d \"%s\"\n",
                       priority, id_string(current_ident)););
    expr_prior = current_ident->prefix_priority;
    if (expr_prior == STRONGEST_PRIORITY) {
      expression = read_dot_expression(do_match_expr);
    } else {
      if (expr_prior <= priority) {
        formal_token = current_ident->prefix_token;
        expression = new_expression_object(&helplist);
        helplist->obj = read_name();
        if (current_ident == prog.id_for.dot) {
          err_num_stri(DOT_EXPR_ILLEGAL,
              (int) GET_ENTITY(helplist->obj)->ident->prefix_priority,
              (int) STRONGEST_PRIORITY, GET_ENTITY(helplist->obj)->ident->name);
          expression = read_dot_expression(do_match_expr);
        } else {
          expression = pars_token(expression,
              formal_token, helplist);
        } /* if */
      } else {
        err_num_stri(WRONG_PREFIX_PRIORITY, (int) expr_prior,
            priority, symbol.name);
        scan_symbol();
        expression = pars_infix_expression(priority, do_match_expr);
      } /* if */
    } /* if */
    if (do_match_expr) {
      if (CATEGORY_OF_OBJ(expression) == EXPROBJECT) {
        if (match_expression(expression) == NULL) {
          err_object(NO_MATCH, expression);
        } /* if */
      } /* if */
    } /* if */
    while (expr_prior <= current_ident->left_token_priority &&
        current_ident->infix_priority <= priority) {
      expr_prior = current_ident->infix_priority;
      formal_token = current_ident->infix_token;
      expression = new_type_of_expression_object(expression,
          &helplist, NULL);
      helplist = add_element_to_list(helplist, read_name());
      expression = pars_token(expression,
          formal_token, helplist);
      if (do_match_expr) {
        if (CATEGORY_OF_OBJ(expression) == EXPROBJECT) {
          if (match_expression(expression) == NULL) {
            err_object(NO_MATCH, expression);
          } /* if */
        } /* if */
      } /* if */
    } /* while */
    logFunction(printf("pars_infix_expression --> " FMT_U_MEM ", ",
                       (memSizeType) expression);
                trace1(expression);
                printf("\n"););
    return expression;
  } /* pars_infix_expression */
