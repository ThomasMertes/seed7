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
/*  File: seed7/src/token.c                                         */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Procedures to maintain objects of type tokentype.      */
/*                                                                  */
/********************************************************************/

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



#ifdef ANSI_C

void free_tokens (tokentype token)
#else

void free_tokens (token)
tokentype token;
#endif

  { /* free_tokens */
#ifdef TRACE_ENTITY
    printf("BEGIN free_tokens\n");
#endif
    if (token != NULL) {
      free_tokens(token->next);
      free_tokens(token->alternative);
      FREE_RECORD(token, tokenrecord, count.token);
    } /* if */
#ifdef TRACE_ENTITY
    printf("END free_tokens\n");
#endif
  } /* free_tokens */



#ifdef ANSI_C

static tokentype new_sy_token (identtype identifier)
#else

static tokentype new_sy_token (identifier)
identtype identifier;
#endif

  {
    register tokentype created_token;

  /* new_sy_token */
#ifdef TRACE_TOKEN
    printf("BEGIN new_sy_token\n");
#endif
    if (!ALLOC_RECORD(created_token, tokenrecord, count.token)) {
      fatal_memory_error(SOURCE_POSITION(2101));
    } /* if */
    created_token->next = NULL;
    created_token->alternative = NULL;
    created_token->token_category = SY_TOKEN;
    created_token->token_value.ident = identifier;
#ifdef TRACE_TOKEN
    printf("END new_sy_token\n");
#endif
    return created_token;
  } /* new_sy_token */



#ifdef ANSI_C

static tokentype new_expr_token (prior_type pri, typetype type_of)
#else

static tokentype new_expr_token (pri, type_of)
prior_type pri;
typetype type_of;
#endif

  {
    tokentype created_token;

  /* new_expr_token */
#ifdef TRACE_TOKEN
    printf("BEGIN new_expr_token\n");
#endif
    if (!ALLOC_RECORD(created_token, tokenrecord, count.token)) {
      fatal_memory_error(SOURCE_POSITION(2102));
    } /* if */
    created_token->next = NULL;
    created_token->alternative = NULL;
    created_token->token_category = EXPR_TOKEN;
    created_token->token_value.expr_par.priority = pri;
    created_token->token_value.expr_par.type_of = type_of;
#ifdef TRACE_TOKEN
    printf("END new_expr_token\n");
#endif
    return created_token;
  } /* new_expr_token */



#ifdef ANSI_C

static tokentype new_syntax_description (void)
#else

static tokentype new_syntax_description ()
#endif

  {
    tokentype created_token;

  /* new_syntax_description */
#ifdef TRACE_TOKEN
    printf("BEGIN new_syntax_description\n");
#endif
    if (!ALLOC_RECORD(created_token, tokenrecord, count.token)) {
      fatal_memory_error(SOURCE_POSITION(2103));
    } /* if */
    created_token->next = NULL;
    created_token->alternative = NULL;
    created_token->token_category = UNDEF_SYNTAX;
    created_token->token_value.type_of = NULL;
#ifdef TRACE_TOKEN
    printf("END new_syntax_description\n");
#endif
    return created_token;
  } /* new_syntax_description */



#ifdef ANSI_C

tokentype get_sy_token (tokentype *tokens, identtype identifier)
#else

tokentype get_sy_token (tokens, identifier)
tokentype *tokens;
identtype identifier;
#endif

  {
    tokentype token_found;
    booltype searching;

  /* get_sy_token */
#ifdef TRACE_TOKEN
    printf("BEGIN get_sy_token\n");
#endif
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
#ifdef TRACE_TOKEN
    printf("END get_sy_token\n");
#endif
    return token_found;
  } /* get_sy_token */



#ifdef ANSI_C

tokentype get_expr_token (tokentype *tokens, prior_type pri,
    typetype type_of)
#else

tokentype get_expr_token (tokens, pri, type_of)
tokentype *tokens;
prior_type pri;
typetype type_of;
#endif

  {
    tokentype token_found;
    tokentype previous_token;

  /* get_expr_token */
#ifdef TRACE_TOKEN
    printf("BEGIN get_expr_token\n");
#endif
    if (*tokens == NULL) {
      token_found = new_expr_token(pri, type_of);
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
          token_found->alternative = new_expr_token(pri, type_of);
          token_found = token_found->alternative;
          break;
        case EXPR_TOKEN:
          break;
        case UNDEF_SYNTAX:
        case LIST_WITH_TYPEOF_SYNTAX:
        case SELECT_ELEMENT_FROM_LIST_SYNTAX:
          if (previous_token == NULL) {
            *tokens = new_expr_token(pri, type_of);
            (*tokens)->alternative = token_found;
            token_found = *tokens;
          } else {
            previous_token->alternative = new_expr_token(pri, type_of);
            previous_token->alternative->alternative = token_found;
            token_found = previous_token->alternative;
          } /* if */
      } /* switch */
    } /* if */
#ifdef TRACE_TOKEN
    printf("END get_expr_token\n");
#endif
    return token_found;
  } /* get_expr_token */



#ifdef ANSI_C

tokentype get_syntax_description (tokentype *tokens)
#else

tokentype get_syntax_description (tokens)
tokentype *tokens;
#endif

  {
    tokentype token_found;

  /* get_syntax_description */
#ifdef TRACE_TOKEN
    printf("BEGIN get_syntax_description\n");
#endif
    if (*tokens == NULL) {
      token_found = new_syntax_description();
      *tokens = token_found;
    } else {
      token_found = *tokens;
      while (token_found->alternative != NULL) {
        token_found = token_found->alternative;
      } /* while */
      if (token_found->token_category != LIST_WITH_TYPEOF_SYNTAX ||
          token_found->token_category != SELECT_ELEMENT_FROM_LIST_SYNTAX) {
        token_found->alternative = new_syntax_description();
        token_found = token_found->alternative;
      } /* if */
    } /* if */
#ifdef TRACE_TOKEN
    printf("END get_syntax_description\n");
#endif
    return token_found;
  } /* get_syntax_description */
