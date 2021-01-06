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
/*  Module: Analyzer - Syntax                                       */
/*  File: seed7/src/syntax.c                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Generate new syntax descriptions out of expressions.   */
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
#include "identutl.h"
#include "listutl.h"
#include "objutl.h"
#include "stat.h"
#include "scanner.h"
#include "symbol.h"
#include "error.h"
#include "token.h"
#include "findid.h"
#include "expr.h"
#include "object.h"
#if ANY_LOG_ACTIVE
#include "traceutl.h"
#endif

#undef EXTERN
#define EXTERN
#include "syntax.h"


typedef enum {XFX, XFY, YFX, YFY} assocType;



#ifdef WITH_PRINT_TOKENS
static void print_tokens (tokenType tokens)

  { /* print_tokens */
    logFunction(printf("print_tokens\n"););
    printf("(");
    while (tokens != NULL) {
      if (tokens->token_category == SY_TOKEN) {
        printf(" %s", tokens->token_value.ident->name);
      } else if (tokens->token_category == EXPR_TOKEN) {
        printf(" [%d]", tokens->token_value.priority);
      } else {
        printf(" ##");
      } /* if */
      if (tokens->alternative != NULL) {
        printf(" ");
        print_tokens(tokens->alternative);
      } /* if */
      tokens = tokens->next;
    } /* while */
    printf(" )");
    logFunction(printf("print_tokens -->\n"););
  } /* print_tokens */
#endif



static tokenType def_single_token (const_objectType statement_token,
    priorityType token_priority, tokenType *formal_tokens,
    boolType *after_expr_token, ustriType *name_of_last_sy_token)

  {
    tokenType new_token;
    objectType type_object;
    typeType typeof_token;
    identType identifier;

  /* def_single_token */
    logFunction(printf("def_single_token\n"););
    if (CATEGORY_OF_OBJ(statement_token) == EXPROBJECT) {
/*  printf("  >[]<\n"); */
      if (statement_token->value.listValue != NULL) {
        type_object = statement_token->value.listValue->obj;
        if (CATEGORY_OF_OBJ(type_object) == TYPEOBJECT) {
          typeof_token = take_type(type_object);
        } else {
          err_object(TYPE_EXPECTED, type_object);
          typeof_token = NULL;
        } /* if */
      } else {
        typeof_token = NULL;
      } /* if */
      new_token = get_expr_token(formal_tokens,
          token_priority, typeof_token);
      if (new_token->token_value.expr_par.priority != token_priority) {
        err_num_stri(WRONG_EXPR_PARAM_PRIORITY,
            (int) token_priority,
            (int) new_token->token_value.expr_par.priority,
            *name_of_last_sy_token);
      } /* if */
      *after_expr_token = TRUE;
    } else {
      identifier = GET_ENTITY(statement_token)->ident;
/*  printf("  >%s<\n", identifier->name); */
      *name_of_last_sy_token = identifier->name;
      new_token = get_sy_token(formal_tokens,
          identifier);
      if (*after_expr_token) {
        if (identifier->prefix_priority == 0) {
          identifier->prefix_priority = WEAKEST_PRIORITY;
        } else {
          if (identifier->prefix_priority != WEAKEST_PRIORITY) {
            err_num_stri(FALSE_PREFIX_PRIORITY, (int) WEAKEST_PRIORITY,
                (int) identifier->prefix_priority, identifier->name);
          } /* if */
        } /* if */
        if (identifier->infix_priority == 0) {
          identifier->infix_priority = WEAKEST_PRIORITY;
        } else {
          if (identifier->infix_priority != WEAKEST_PRIORITY) {
            err_num_stri(FALSE_INFIX_PRIORITY, (int) WEAKEST_PRIORITY,
                (int) identifier->infix_priority, identifier->name);
          } /* if */
        } /* if */
      } /* if */
      *after_expr_token = FALSE;
    } /* if */
    logFunction(printf("def_single_token -->\n"););
    return new_token;
  } /* def_single_token */



static inline int count_inner_tokens (const_listType statement_tokens)

  {
    int number_of_inner_tokens;
    int token_number;

  /* count_inner_tokens */
    logFunction(printf("count_inner_tokens\n"););
    number_of_inner_tokens = 0;
    token_number = 1;
    while (statement_tokens != NULL) {
      if (CATEGORY_OF_OBJ(statement_tokens->obj) != EXPROBJECT) {
        number_of_inner_tokens = token_number;
      } /* if */
      statement_tokens = statement_tokens->next;
      token_number++;
    } /* while */
    logFunction(printf("count_inner_tokens -->\n"););
    return number_of_inner_tokens;
  } /* count_inner_tokens */



static tokenType def_token_list (const_listType statement_tokens,
    priorityType right_token_priority, tokenType *formal_tokens,
    ustriType name_of_last_sy_token)

  {
    int number_of_inner_tokens;
    int token_number;
    tokenType current_token;
    boolType after_expr_token;
    tokenType token_list_end;

  /* def_token_list */
    logFunction(printf("def_token_list\n"););
/*  printf(" DEF_PAR_LIST:  ");
    prot_list(statement_tokens);
    printf("\n"); */
    if (statement_tokens != NULL) {
      after_expr_token = FALSE;
      if (statement_tokens->next != NULL) {
#ifdef OUT_OF_ORDER
        current_token = def_single_token(
            statement_tokens->obj, WEAKEST_PRIORITY,
            formal_tokens, &after_expr_token,
            &name_of_last_sy_token);
        statement_tokens = statement_tokens->next;
        while (statement_tokens->next != NULL) {
          current_token = def_single_token(
              statement_tokens->obj, WEAKEST_PRIORITY,
              &current_token->next, &after_expr_token,
              &name_of_last_sy_token);
          statement_tokens = statement_tokens->next;
        } /* while */
        current_token = def_single_token(
            statement_tokens->obj, right_token_priority,
            &current_token->next, &after_expr_token,
            &name_of_last_sy_token);
#endif
        number_of_inner_tokens =
            count_inner_tokens(statement_tokens);
        if (number_of_inner_tokens >= 1) {
          current_token = def_single_token(
              statement_tokens->obj, WEAKEST_PRIORITY,
              formal_tokens, &after_expr_token,
              &name_of_last_sy_token);
        } else {
          current_token = def_single_token(
              statement_tokens->obj, right_token_priority,
              formal_tokens, &after_expr_token,
              &name_of_last_sy_token);
        } /* if */
        statement_tokens = statement_tokens->next;
        for (token_number = 2;
            token_number <= number_of_inner_tokens;
            token_number++) {
          current_token = def_single_token(
              statement_tokens->obj, WEAKEST_PRIORITY,
              &current_token->next, &after_expr_token,
              &name_of_last_sy_token);
          statement_tokens = statement_tokens->next;
        } /* for */
        while (statement_tokens != NULL) {
          current_token = def_single_token(
              statement_tokens->obj, right_token_priority,
              &current_token->next, &after_expr_token,
              &name_of_last_sy_token);
          statement_tokens = statement_tokens->next;
        } /* while */
      } else {
        current_token = def_single_token(
            statement_tokens->obj, right_token_priority,
            formal_tokens, &after_expr_token,
            &name_of_last_sy_token);
      } /* if */
      token_list_end = get_syntax_description(&current_token->next);
    } else {
      token_list_end = get_syntax_description(formal_tokens);
    } /* if */
    logFunction(printf("def_token_list -->\n"););
    return token_list_end;
  } /* def_token_list */



static inline tokenType def_infix_syntax (const_listType statement_syntax,
    priorityType statement_priority, assocType statement_associativity)

  {
    identType identifier;
    tokenType token_list_end;

  /* def_infix_syntax */
    logFunction(printf("def_infix_syntax\n"););
    if (CATEGORY_OF_OBJ(statement_syntax->obj) != EXPROBJECT) {
      identifier = GET_ENTITY(statement_syntax->obj)->ident;
      if (identifier->infix_priority == 0) {
        identifier->infix_priority = statement_priority;
      } else {
        if (identifier->infix_priority != statement_priority) {
          err_num_stri(FALSE_INFIX_PRIORITY, (int) statement_priority,
              (int) identifier->infix_priority, identifier->name);
        } /* if */
      } /* if */
      if (statement_associativity == YFX ||
          statement_associativity == YFY) {
        identifier->left_token_priority = statement_priority;
      } else {
        identifier->left_token_priority =
            statement_priority - (priorityType) 1;
      } /* if */
      if (statement_associativity == XFY ||
          statement_associativity == YFY) {
        token_list_end = def_token_list(statement_syntax->next,
            statement_priority,
            &identifier->infix_token, identifier->name);
      } else {
        token_list_end = def_token_list(statement_syntax->next,
            (priorityType) (((int) statement_priority) - 1),
            &identifier->infix_token, identifier->name);
      } /* if */
/*  printf("[%d] %s ", identifier->left_token_priority, identifier->name);
    print_tokens(identifier->infix_token);
    printf("\n"); */
    } else {
      err_warning(TWO_PARAMETER_SYNTAX);
      token_list_end = NULL;
    } /* if */
    logFunction(printf("def_infix_syntax\n"););
    return token_list_end;
  } /* def_infix_syntax */



static inline tokenType def_prefix_syntax (const_listType statement_syntax,
    priorityType statement_priority, assocType statement_associativity)

  {
    identType identifier;
    tokenType token_list_end;

  /* def_prefix_syntax */
    logFunction(printf("def_prefix_syntax\n"););
    identifier = GET_ENTITY(statement_syntax->obj)->ident;
    if (identifier->prefix_priority == 0) {
      identifier->prefix_priority = statement_priority;
    } else {
      if (identifier->prefix_priority != statement_priority) {
        err_num_stri(FALSE_PREFIX_PRIORITY, (int) statement_priority,
            (int) identifier->prefix_priority, identifier->name);
      } /* if */
    } /* if */
    if (statement_associativity == XFY ||
        statement_associativity == YFY) {
      token_list_end = def_token_list(statement_syntax->next,
          statement_priority,
          &identifier->prefix_token, identifier->name);
    } else {
      token_list_end = def_token_list(statement_syntax->next,
          (priorityType) (((int) statement_priority) - 1),
          &identifier->prefix_token, identifier->name);
    } /* if */
/*  printf("%s ", identifier->name);
    print_tokens(identifier->prefix_token);
    printf("\n"); */
    logFunction(printf("def_prefix_syntax -->\n"););
    return token_list_end;
  } /* def_prefix_syntax */



static inline tokenType def_statement_syntax (objectType syntax_expression,
    priorityType statement_priority, assocType statement_associativity)

  {
    listType statement_syntax;
    identType identifier;
    tokenType token_list_end;

  /* def_statement_syntax */
    logFunction(printf("def_statement_syntax(");
                trace1(syntax_expression);
                printf(")\n"););
    /* printcategory(CATEGORY_OF_OBJ(syntax_expression)); */
    if (CATEGORY_OF_OBJ(syntax_expression) == LISTOBJECT) {
      /* printf("SYNTAX: ");
      prot_list(syntax_expression->value.listValue);
      printf("\n"); */
      statement_syntax = syntax_expression->value.listValue;
      if (statement_syntax != NULL) {
        check_list_of_syntax_elements(statement_syntax);
        if (CATEGORY_OF_OBJ(statement_syntax->obj) == EXPROBJECT) {
          if (statement_syntax->next != NULL) {
            token_list_end = def_infix_syntax(statement_syntax->next,
                statement_priority, statement_associativity);
          } else {
            err_warning(EMPTY_SYNTAX);
            token_list_end = NULL;
          } /* if */
        } else {
          token_list_end = def_prefix_syntax(statement_syntax,
              statement_priority, statement_associativity);
        } /* if */
      } else {
        err_warning(EMPTY_SYNTAX);
        token_list_end = NULL;
      } /* if */
    } else if (CATEGORY_OF_OBJ(syntax_expression) == EXPROBJECT) {
      err_warning(DOT_EXPR_REQUESTED);
      token_list_end = NULL;
    } else {
      identifier = GET_ENTITY(syntax_expression)->ident;
      if (identifier == prog->ident.literal) {
        err_object(IDENT_EXPECTED, syntax_expression);
      } /* if */
      if (identifier->prefix_priority == 0) {
        identifier->prefix_priority = statement_priority;
      } else {
        if (identifier->prefix_priority != statement_priority) {
          err_num_stri(FALSE_PREFIX_PRIORITY, (int) statement_priority,
              (int) identifier->prefix_priority, identifier->name);
        } /* if */
      } /* if */
      token_list_end = get_syntax_description(&identifier->prefix_token);
/*  printf("%s\n", identifier->name); */
    } /* if */
    logFunction(printf("def_statement_syntax -->\n"););
    return token_list_end;
  } /* def_statement_syntax */



void decl_syntax (void)

  {
    objectType type_object;
    typeType typeof_object;
    objectType expression;
    assocType assoc;
    tokenType token_list_end;

  /* decl_syntax */
    logFunction(printf("decl_syntax\n"););
    scan_symbol();
    if (current_ident == prog->id_for.colon) {
      typeof_object = NULL;
      scan_symbol();
    } else {
      type_object = pars_infix_expression(WEAKEST_PRIORITY, TRUE);
      if (CATEGORY_OF_OBJ(type_object) == TYPEOBJECT) {
        typeof_object = take_type(type_object);
      } else {
        err_object(TYPE_EXPECTED, type_object);
        typeof_object = NULL;
      } /* if */
      if (current_ident == prog->id_for.colon) {
        scan_symbol();
      } else {
        err_ident(EXPECTED_SYMBOL, prog->id_for.colon);
      } /* if */
    } /* if */
    expression = pars_infix_expression(WEAKEST_PRIORITY, FALSE);
    if (current_ident == prog->id_for.is) {
      scan_symbol();
    } else {
      err_ident(EXPECTED_SYMBOL, prog->id_for.is);
    } /* if */
    if (current_ident == prog->id_for.r_arrow) {            /*  ->   */
      assoc = YFX;
    } else if (current_ident == prog->id_for.l_arrow) {     /*  <-   */
      assoc = XFY;
    } else if (current_ident == prog->id_for.out_arrow) {   /*  <->  */
      assoc = XFX;
    } else if (current_ident == prog->id_for.in_arrow) {    /*  -><- */
      assoc = YFY;
    } else {
      err_warning(ILLEGAL_ASSOCIATIVITY);
      assoc = YFX;
    } /* if */
    scan_symbol();
    if (symbol.sycategory != INTLITERAL) {
      err_string(CARD_EXPECTED, symbol.name);
      scan_symbol();
    } else {
      if (symbol.intValue > WEAKEST_PRIORITY) {
        err_integer(ILLEGAL_PRIORITY, symbol.intValue);
        scan_symbol();
      } else {
        token_list_end = def_statement_syntax(expression,
            (priorityType) symbol.intValue, assoc);
        scan_symbol();
        if (token_list_end != NULL) {
          if (token_list_end->token_category != UNDEF_SYNTAX) {
            err_warning(SYNTAX_DECLARED_TWICE);
          } else if (current_ident == prog->id_for.lbrack) {
            scan_symbol();
            if (symbol.sycategory != INTLITERAL) {
              err_string(CARD_EXPECTED, symbol.name);
            } else {
              token_list_end->token_category = SELECT_ELEMENT_FROM_LIST_SYNTAX;
              token_list_end->token_value.select = symbol.intValue;
            } /* if */
            scan_symbol();
            if (current_ident == prog->id_for.rbrack) {
              scan_symbol();
            } else {
              err_ident(EXPECTED_SYMBOL, prog->id_for.rbrack);
            } /* if */
          } else {
            token_list_end->token_category = LIST_WITH_TYPEOF_SYNTAX;
            token_list_end->token_value.type_of = typeof_object;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (current_ident == prog->id_for.semicolon) {
      scan_symbol();
    } else {
      err_ident(EXPECTED_SYMBOL, prog->id_for.semicolon);
    } /* if */
/*  printcategory(CATEGORY_OF_OBJ(expression)); */
    free_expression(expression);
    logFunction(printf("decl_syntax\n"););
  } /* decl_syntax */
