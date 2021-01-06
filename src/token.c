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
/*  File: seed7/src/token.c                                         */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Procedures to maintain objects of type tokenType.      */
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
#include "fatal.h"
#include "stat.h"

#undef EXTERN
#define EXTERN
#include "token.h"



void free_tokens (tokenType token)

  { /* free_tokens */
    logFunction(printf("free_tokens\n"););
    if (token != NULL) {
      free_tokens(token->next);
      free_tokens(token->alternative);
      FREE_RECORD(token, tokenRecord, count.token);
    } /* if */
    logFunction(printf("free_tokens -->\n"););
  } /* free_tokens */



static tokenType new_sy_token (identType identifier)

  {
    register tokenType created_token;

  /* new_sy_token */
    logFunction(printf("new_sy_token\n"););
    if (!ALLOC_RECORD(created_token, tokenRecord, count.token)) {
      fatal_memory_error(SOURCE_POSITION(2101));
    } /* if */
    created_token->next = NULL;
    created_token->alternative = NULL;
    created_token->token_category = SY_TOKEN;
    created_token->token_value.ident = identifier;
    logFunction(printf("new_sy_token -->\n"););
    return created_token;
  } /* new_sy_token */



static tokenType new_expr_token (priorityType priority, typeType type_of)

  {
    tokenType created_token;

  /* new_expr_token */
    logFunction(printf("new_expr_token\n"););
    if (!ALLOC_RECORD(created_token, tokenRecord, count.token)) {
      fatal_memory_error(SOURCE_POSITION(2102));
    } /* if */
    created_token->next = NULL;
    created_token->alternative = NULL;
    created_token->token_category = EXPR_TOKEN;
    created_token->token_value.expr_par.priority = priority;
    created_token->token_value.expr_par.type_of = type_of;
    logFunction(printf("new_expr_token -->\n"););
    return created_token;
  } /* new_expr_token */



static tokenType new_syntax_description (void)

  {
    tokenType created_token;

  /* new_syntax_description */
    logFunction(printf("new_syntax_description\n"););
    if (!ALLOC_RECORD(created_token, tokenRecord, count.token)) {
      fatal_memory_error(SOURCE_POSITION(2103));
    } /* if */
    created_token->next = NULL;
    created_token->alternative = NULL;
    created_token->token_category = UNDEF_SYNTAX;
    created_token->token_value.type_of = NULL;
    logFunction(printf("new_syntax_description -->\n"););
    return created_token;
  } /* new_syntax_description */



tokenType get_sy_token (tokenType *tokens, identType identifier)

  {
    tokenType token_found;
    boolType searching;

  /* get_sy_token */
    logFunction(printf("get_sy_token\n"););
    if (*tokens == NULL) {
      token_found = new_sy_token(identifier);
      *tokens = token_found;
    } else {
      token_found = *tokens;
      searching = TRUE;
      do {
        if (token_found->token_category == SY_TOKEN) {
          if (identifier == token_found->token_value.ident) {
            searching = FALSE;
          } else {
            token_found = token_found->alternative;
          } /* if */
        } else {
          token_found = NULL;
        } /* if */
      } while (searching && token_found != NULL);
      if (searching) {
        token_found = new_sy_token(identifier);
        token_found->alternative = *tokens;
        *tokens = token_found;
      } /* if */
    } /* if */
    logFunction(printf("get_sy_token -->\n"););
    return token_found;
  } /* get_sy_token */



tokenType get_expr_token (tokenType *tokens, priorityType priority,
    typeType type_of)

  {
    tokenType token_found;
    tokenType previous_token;

  /* get_expr_token */
    logFunction(printf("get_expr_token\n"););
    if (*tokens == NULL) {
      token_found = new_expr_token(priority, type_of);
      *tokens = token_found;
    } else {
      token_found = *tokens;
      previous_token = NULL;
      while (token_found->token_category == SY_TOKEN  &&
          token_found->alternative != NULL) {
        previous_token = token_found;
        token_found = token_found->alternative;
      } /* while */
      switch (token_found->token_category) {
        case SY_TOKEN:
          token_found->alternative = new_expr_token(priority, type_of);
          token_found = token_found->alternative;
          break;
        case EXPR_TOKEN:
          break;
        case UNDEF_SYNTAX:
        case LIST_WITH_TYPEOF_SYNTAX:
        case SELECT_ELEMENT_FROM_LIST_SYNTAX:
          if (previous_token == NULL) {
            *tokens = new_expr_token(priority, type_of);
            (*tokens)->alternative = token_found;
            token_found = *tokens;
          } else {
            previous_token->alternative = new_expr_token(priority, type_of);
            previous_token->alternative->alternative = token_found;
            token_found = previous_token->alternative;
          } /* if */
          break;
      } /* switch */
    } /* if */
    logFunction(printf("get_expr_token -->\n"););
    return token_found;
  } /* get_expr_token */



tokenType get_syntax_description (tokenType *tokens)

  {
    tokenType token_found;

  /* get_syntax_description */
    logFunction(printf("get_syntax_description\n"););
    if (*tokens == NULL) {
      token_found = new_syntax_description();
      *tokens = token_found;
    } else {
      token_found = *tokens;
      while (token_found->alternative != NULL) {
        token_found = token_found->alternative;
      } /* while */
      if (token_found->token_category != LIST_WITH_TYPEOF_SYNTAX &&
          token_found->token_category != SELECT_ELEMENT_FROM_LIST_SYNTAX) {
        token_found->alternative = new_syntax_description();
        token_found = token_found->alternative;
      } /* if */
    } /* if */
    logFunction(printf("get_syntax_description -->\n"););
    return token_found;
  } /* get_syntax_description */
