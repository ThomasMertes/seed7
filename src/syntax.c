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
/*  Module: Analyzer - Syntax                                       */
/*  File: seed7/src/syntax.c                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Generate new syntax descriptions out of expressions.   */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "identutl.h"
#include "listutl.h"
#include "stat.h"
#include "scanner.h"
#include "symbol.h"
#include "error.h"
#include "token.h"
#include "findid.h"
#include "expr.h"
#include "object.h"
#include "memory.h"

#undef EXTERN
#define EXTERN
#include "syntax.h"


typedef enum {XFX, XFY, YFX, YFY} assoctype;



#ifdef WITH_PRINT_TOKENS
#ifdef ANSI_C

static void print_tokens (tokentype tokens)
#else

static void print_tokens (tokens)
tokentype tokens;
#endif

  { /* print_tokens */
#ifdef TRACE_SYNTAX
    printf("BEGIN print_tokens\n");
#endif
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
#ifdef TRACE_SYNTAX
    printf("END print_tokens\n");
#endif
  } /* print_tokens */
#endif



#ifdef ANSI_C

static tokentype def_single_token (const_objecttype statement_token,
    prior_type token_priority, tokentype *formal_tokens,
    booltype *after_expr_token, ustritype *name_of_last_sy_token)
#else

static tokentype def_single_token (statement_token,
    token_priority, formal_tokens, after_expr_token,
    name_of_last_sy_token)
objecttype statement_token;
prior_type token_priority;
tokentype *formal_tokens;
booltype *after_expr_token;
ustritype *name_of_last_sy_token;
#endif

  {
    tokentype new_token;
    objecttype type_object;
    typetype typeof_token;
    identtype identifier;

  /* def_single_token */
#ifdef TRACE_SYNTAX
    printf("BEGIN def_single_token\n");
#endif
    if (CATEGORY_OF_OBJ(statement_token) == EXPROBJECT) {
/*  printf("  >[]<\n"); */
      if (statement_token->value.listvalue != NULL) {
        type_object = statement_token->value.listvalue->obj;
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
#ifdef TRACE_SYNTAX
    printf("END def_single_token\n");
#endif
    return(new_token);
  } /* def_single_token */



#ifdef ANSI_C

static INLINE int count_inner_tokens (const_listtype statement_tokens)
#else

static INLINE int count_inner_tokens (statement_tokens)
listtype statement_tokens;
#endif

  {
    int number_of_inner_tokens;
    int token_number;

  /* count_inner_tokens */
#ifdef TRACE_SYNTAX
    printf("BEGIN count_inner_tokens\n");
#endif
    number_of_inner_tokens = 0;
    token_number = 1;
    while (statement_tokens != NULL) {
      if (CATEGORY_OF_OBJ(statement_tokens->obj) != EXPROBJECT) {
        number_of_inner_tokens = token_number;
      } /* if */
      statement_tokens = statement_tokens->next;
      token_number++;
    } /* while */
#ifdef TRACE_SYNTAX
    printf("END count_inner_tokens\n");
#endif
    return(number_of_inner_tokens);
  } /* count_inner_tokens */



#ifdef ANSI_C

static tokentype def_token_list (const_listtype statement_tokens,
    prior_type right_token_priority, tokentype *formal_tokens,
    ustritype name_of_last_sy_token)
#else

static tokentype def_token_list (statement_tokens,
    right_token_priority, formal_tokens,
    name_of_last_sy_token)
listtype statement_tokens;
prior_type right_token_priority;
tokentype *formal_tokens;
ustritype name_of_last_sy_token;
#endif

  {
    int number_of_inner_tokens;
    int token_number;
    tokentype current_token;
    booltype after_expr_token;
    tokentype token_list_end;

  /* def_token_list */
#ifdef TRACE_SYNTAX
    printf("BEGIN def_token_list\n");
#endif
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
#ifdef TRACE_SYNTAX
    printf("END def_token_list\n");
#endif
    return(token_list_end);
  } /* def_token_list */



#ifdef ANSI_C

static INLINE tokentype def_infix_syntax (const_listtype statement_syntax,
    prior_type statement_priority, assoctype statement_associativity)
#else

static INLINE tokentype def_infix_syntax (statement_syntax,
    statement_priority, statement_associativity)
listtype statement_syntax;
prior_type statement_priority;
assoctype statement_associativity;
#endif

  {
    identtype identifier;
    tokentype token_list_end;

  /* def_infix_syntax */
#ifdef TRACE_SYNTAX
    printf("BEGIN def_infix_syntax\n");
#endif
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
            statement_priority - (prior_type) 1;
      } /* if */
      if (statement_associativity == XFY ||
          statement_associativity == YFY) {
        token_list_end = def_token_list(statement_syntax->next,
            statement_priority,
            &identifier->infix_token, identifier->name);
      } else {
        token_list_end = def_token_list(statement_syntax->next,
            (prior_type) (((int) statement_priority) - 1),
            &identifier->infix_token, identifier->name);
      } /* if */
/*  printf("[%d] %s ", identifier->left_token_priority, identifier->name);
    print_tokens(identifier->infix_token);
    printf("\n"); */
    } else {
      err_warning(TWO_PARAMETER_SYNTAX);
      token_list_end = NULL;
    } /* if */
#ifdef TRACE_SYNTAX
    printf("END def_infix_syntax\n");
#endif
    return(token_list_end);
  } /* def_infix_syntax */



#ifdef ANSI_C

static INLINE tokentype def_prefix_syntax (const_listtype statement_syntax,
    prior_type statement_priority, assoctype statement_associativity)
#else

static INLINE tokentype def_prefix_syntax (statement_syntax,
    statement_priority, statement_associativity)
listtype statement_syntax;
prior_type statement_priority;
assoctype statement_associativity;
#endif

  {
    identtype identifier;
    tokentype token_list_end;

  /* def_prefix_syntax */
#ifdef TRACE_SYNTAX
    printf("BEGIN def_prefix_syntax\n");
#endif
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
          (prior_type) (((int) statement_priority) - 1),
          &identifier->prefix_token, identifier->name);
    } /* if */
/*  printf("%s ", identifier->name);
    print_tokens(identifier->prefix_token);
    printf("\n"); */
#ifdef TRACE_SYNTAX
    printf("END def_prefix_syntax\n");
#endif
    return(token_list_end);
  } /* def_prefix_syntax */



#ifdef ANSI_C

static INLINE tokentype def_statement_syntax (objecttype syntax_expression,
    prior_type statement_priority, assoctype statement_associativity)
#else

static INLINE tokentype def_statement_syntax (syntax_expression,
    statement_priority, statement_associativity)
objecttype syntax_expression;
prior_type statement_priority;
assoctype statement_associativity;
#endif

  {
    listtype statement_syntax;
    identtype identifier;
    tokentype token_list_end;

  /* def_statement_syntax */
#ifdef TRACE_SYNTAX
    printf("BEGIN def_statement_syntax(");
    trace1(syntax_expression);
    printf(")\n");
#endif
    /* printcategory(CATEGORY_OF_OBJ(syntax_expression)); */
    if (CATEGORY_OF_OBJ(syntax_expression) == LISTOBJECT) {
      /* printf("SYNTAX: ");
      prot_list(syntax_expression->value.listvalue);
      printf("\n"); */
      statement_syntax = syntax_expression->value.listvalue;
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
      if (identifier == prog.ident.literal) {
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
#ifdef TRACE_SYNTAX
    printf("END def_statement_syntax\n");
#endif
    return(token_list_end);
  } /* def_statement_syntax */



#ifdef ANSI_C

void decl_syntax (void)
#else

void decl_syntax ()
#endif

  {
    objecttype type_object;
    typetype typeof_object;
    objecttype expression;
    assoctype assoc;
    tokentype token_list_end;

  /* decl_syntax */
#ifdef TRACE_SYNTAX
    printf("BEGIN decl_syntax\n");
#endif
    scan_symbol();
    if (current_ident == prog.id_for.colon) {
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
      if (current_ident == prog.id_for.colon) {
        scan_symbol();
      } else {
        err_ident(EXPECTED_SYMBOL, prog.id_for.colon);
      } /* if */
    } /* if */
    expression = pars_infix_expression(WEAKEST_PRIORITY, FALSE);
    if (current_ident == prog.id_for.is) {
      scan_symbol();
    } else {
      err_ident(EXPECTED_SYMBOL, prog.id_for.is);
    } /* if */
    if (current_ident == prog.id_for.r_arrow) {            /*  ->   */
      assoc = YFX;
    } else if (current_ident == prog.id_for.l_arrow) {     /*  <-   */
      assoc = XFY;
    } else if (current_ident == prog.id_for.out_arrow) {   /*  <->  */
      assoc = XFX;
    } else if (current_ident == prog.id_for.in_arrow) {    /*  -><- */
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
      if (symbol.intvalue > WEAKEST_PRIORITY) {
        err_integer(ILLEGAL_PRIORITY, symbol.intvalue);
        scan_symbol();
      } else {
        token_list_end = def_statement_syntax(expression,
            (prior_type) symbol.intvalue, assoc);
        scan_symbol();
        if (token_list_end != NULL) {
          if (current_ident == prog.id_for.lbrack) {
            scan_symbol();
            if (symbol.sycategory != INTLITERAL) {
              err_string(CARD_EXPECTED, symbol.name);
            } else {
              token_list_end->token_category = SELECT_ELEMENT_FROM_LIST_SYNTAX;
              token_list_end->token_value.select = symbol.intvalue;
            } /* if */
            scan_symbol();
            if (current_ident == prog.id_for.rbrack) {
              scan_symbol();
            } else {
              err_ident(EXPECTED_SYMBOL, prog.id_for.rbrack);
            } /* if */
          } else {
            token_list_end->token_category = LIST_WITH_TYPEOF_SYNTAX;
            token_list_end->token_value.type_of = typeof_object;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (current_ident == prog.id_for.semicolon) {
      scan_symbol();
    } else {
      err_ident(EXPECTED_SYMBOL, prog.id_for.semicolon);
    } /* if */
/*  printcategory(CATEGORY_OF_OBJ(expression)); */
/* Loeschen der expression wenn EXPROBJECT: */
  if (CATEGORY_OF_OBJ(expression) == EXPROBJECT) {
    emptylist(expression->value.listvalue);
    FREE_OBJECT(expression);
/*  FREE_RECORD(expression, objectrecord, count.object); */
  } /* if */
#ifdef TRACE_SYNTAX
    printf("END decl_syntax\n");
#endif
  } /* decl_syntax */
