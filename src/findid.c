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
/*  Module: Analyzer                                                */
/*  File: seed7/src/findid.c                                        */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Procedures to maintain the identifier table.           */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "datautl.h"
#include "chclsutl.h"
#include "identutl.h"
#include "entutl.h"
#include "traceutl.h"
#include "token.h"
#include "fatal.h"
#include "stat.h"
#include "symbol.h"
#include "object.h"
#include "error.h"

#undef EXTERN
#define EXTERN
#include "findid.h"



#ifdef ANSI_C

static INLINE identtype id_generation (ustritype name, unsigned int length)
#else

static INLINE identtype id_generation (name, length)
ustritype name;
unsigned int length;
#endif

  {
    register identtype created_ident;

  /* id_generation */
#ifdef TRACE_FINDID
    printf("BEGIN id_generation\n");
#endif
    if ((created_ident = new_ident(name, length)) == NULL) {
      fatal_memory_error(SOURCE_POSITION(2041));
    } /* if */
#ifdef TRACE_FINDID
    printf("END id_generation\n");
#endif
    return(created_ident);
  } /* id_generation */



#ifdef ANSI_C

void find_normal_ident (unsigned int length)
#else

void find_normal_ident (length)
unsigned int length;
#endif

  {                                                             /*  0.62% */
    register identtype search_ident;
    register int comparison;

  /* find_normal_ident */
#ifdef TRACE_FINDID
    printf("BEGIN find_normal_ident\n");
#endif
    if ((search_ident = IDENT_TABLE(symbol.name, length)) != NULL) { /*  1.49% */
      if ((comparison = strcmp((cstritype) symbol.name,
          (cstritype) search_ident->name)) != 0) {              /*  0.73% */
        do {
          if (comparison < 0) {                                 /*  0.07% */
            if (search_ident->next1 == NULL) {                  /*  0.08% */
              search_ident->next1 = id_generation(symbol.name, length);
              search_ident = search_ident->next1;
              current_ident = search_ident;
              return;
              /* The following statement can be used instead of the       */
              /* above two statements to avoid middle function returns.   */
#ifdef OUT_OF_ORDER
              comparison = 0;
#endif
            } else {
              search_ident = search_ident->next1;               /*  0.02% */
              comparison = strcmp((cstritype) symbol.name,
                  (cstritype) search_ident->name);              /*  0.09% */
            } /* if */
          } else {                                              /*  0.02% */
            if (search_ident->next2 == NULL) {                  /*  0.06% */
              search_ident->next2 = id_generation(symbol.name, length); /*  0.01% */
              search_ident = search_ident->next2;
              current_ident = search_ident;
              return;
              /* The following statement can be used instead of the       */
              /* above two statements to avoid middle function returns.   */
#ifdef OUT_OF_ORDER
              comparison = 0;
#endif
            } else {
              search_ident = search_ident->next2;               /*  0.01% */
              comparison = strcmp((cstritype) symbol.name,
                  (cstritype) search_ident->name);              /*  0.07% */
            } /* if */
          } /* if */
        } while (comparison != 0);                              /*  0.07% */
      } /* if */
      current_ident = search_ident;                             /*  0.12% */
    } else {
      current_ident = id_generation(symbol.name, length);       /*  0.01% */
      IDENT_TABLE(symbol.name, length) = current_ident;         /*  0.02% */
    } /* if */
#ifdef TRACE_FINDID
    printf("END find_normal_ident\n");
#endif
  } /* find_normal_ident */                                     /*  0.62% */



#ifdef ANSI_C

static identtype put_ident (cstritype stri, errinfotype *err_info)
#else

static identtype put_ident (stri, err_info)
cstritype stri;
errinfotype *err_info;
#endif

  {
    register identtype ident_found;

  /* put_ident */
#ifdef TRACE_FINDID
    printf("BEGIN put_ident\n");
#endif
    if ((ident_found = get_ident((ustritype) stri, strlen(stri))) == NULL) {
      *err_info = MEMORY_ERROR;
    } /* if */
#ifdef TRACE_FINDID
    printf("END put_ident\n");
#endif
    return(ident_found);
  } /* put_ident */



#ifdef ANSI_C

void check_list_of_syntax_elements (const_listtype elem_list)
#else

void check_list_of_syntax_elements (elem_list)
listtype elem_list;
#endif

  { /* check_list_of_syntax_elements */
#ifdef TRACE_FINDID
    printf("BEGIN check_list_of_syntax_elements\n");
#endif
    while (elem_list != NULL) {
      if (!HAS_ENTITY(elem_list->obj)) {
        if (CATEGORY_OF_OBJ(elem_list->obj) != EXPROBJECT) {
          err_object(IDENT_EXPECTED, elem_list->obj);
        } /* if */
      } /* if */
#ifdef OUT_OF_ORDER
      if (GET_ENTITY(elem_list->obj) == entity.literal) {
        if (CATEGORY_OF_OBJ(elem_list->obj) != EXPROBJECT) {
          err_object(IDENT_EXPECTED, elem_list->obj);
        } /* if */
      } /* if */
#endif
      elem_list = elem_list->next;
    } /* while */
#ifdef TRACE_FINDID
    printf("END check_list_of_syntax_elements\n");
#endif
  } /* check_list_of_syntax_elements */



#ifdef ANSI_C

static void clean_ident_tree (identtype actual_ident)
#else

static void clean_ident_tree (actual_ident)
identtype actual_ident;
#endif

  { /* clean_ident_tree */
#ifdef TRACE_FINDID
    printf("BEGIN clean_ident_tree\n");
#endif
    if (actual_ident != NULL) {
      clean_ident_tree(actual_ident->next1);
      free_tokens(actual_ident->prefix_token);
      actual_ident->prefix_token = NULL;
      free_tokens(actual_ident->infix_token);
      actual_ident->infix_token = NULL;
      clean_ident_tree(actual_ident->next2);
    } /* if */
#ifdef TRACE_FINDID
    printf("END clean_ident_tree\n");
#endif
  } /* clean_ident_tree */



#ifdef ANSI_C

void clean_idents (void)
#else

void clean_idents ()
#endif

  {
    int position;
    int character;
    identtype actual_ident;

  /* clean_idents */
#ifdef TRACE_FINDID
    printf("BEGIN clean_idents\n");
#endif
    for (position = 0; position < ID_TABLE_SIZE; position++) {
      clean_ident_tree(prog.ident.table[position]);
    } /* for */
    for (character = '!'; character <= '~'; character++) {
      if (op_character(character) ||
          char_class(character) == LEFTPARENCHAR ||
          char_class(character) == PARENCHAR) {
        actual_ident = prog.ident.table1[character];
        free_tokens(actual_ident->prefix_token);
        actual_ident->prefix_token = NULL;
        free_tokens(actual_ident->infix_token);
        actual_ident->infix_token = NULL;
      } /* if */
    } /* for */
#ifdef TRACE_FINDID
    printf("END clean_idents\n");
#endif
  } /* clean_idents */



#ifdef ANSI_C

static void wri_binary_ident_tree (const_identtype actual_ident)
#else

static void wri_binary_ident_tree (actual_ident)
identtype actual_ident;
#endif

  { /* wri_binary_ident_tree */
#ifdef TRACE_FINDID
    printf("BEGIN wri_binary_ident_tree\n");
#endif
    if (actual_ident != NULL) {
      wri_binary_ident_tree(actual_ident->next1);
      printf("%s ", id_string(actual_ident));
      if (actual_ident->entity != NULL &&
          actual_ident->entity->syobject != NULL) {
        if (CATEGORY_OF_OBJ(actual_ident->entity->syobject) == SYMBOLOBJECT) {
          printf(" %ld", actual_ident->entity->syobject->value.pos);
        } /* if */
      } /* if */
      printf("\n");
      wri_binary_ident_tree(actual_ident->next2);
    } /* if */
#ifdef TRACE_FINDID
    printf("END wri_binary_ident_tree\n");
#endif
  } /* wri_binary_ident_tree */



#ifdef ANSI_C

void write_idents (void)
#else

void write_idents ()
#endif

  {
    int position;
    int character;

  /* write_idents */
#ifdef TRACE_FINDID
    printf("BEGIN write_idents\n");
#endif
    for (position = 0; position < ID_TABLE_SIZE; position++) {
      printf(" ====== %d ======\n", position);
      wri_binary_ident_tree(prog.ident.table[position]);
    } /* for */
    for (character = '!'; character <= '~'; character++) {
      if (op_character(character) ||
          char_class(character) == LEFTPARENCHAR ||
          char_class(character) == PARENCHAR) {
        printf("%s ", id_string(prog.ident.table1[character]));
        if (prog.ident.table1[character]->entity != NULL &&
            prog.ident.table1[character]->entity->syobject != NULL) {
          if (CATEGORY_OF_OBJ(prog.ident.table1[character]->entity->syobject) == SYMBOLOBJECT) {
            printf(" %ld", prog.ident.table1[character]->entity->syobject->value.pos);
          } /* if */
        } /* if */
        printf("\n");
      } /* if */
    } /* for */
#ifdef TRACE_FINDID
    printf("END write_idents\n");
#endif
  } /* write_idents */



#ifdef ANSI_C

void init_findid (errinfotype *err_info)
#else

void init_findid (err_info)
errinfotype *err_info;
#endif

  { /* init_findid */
#ifdef TRACE_FINDID
    printf("BEGIN init_findid\n");
#endif
    prog.id_for.lparen =    put_ident("(",       err_info);
    prog.id_for.lbrack =    put_ident("[",       err_info);
    prog.id_for.lbrace =    put_ident("{",       err_info);
    prog.id_for.rparen =    put_ident(")",       err_info);
    prog.id_for.rbrack =    put_ident("]",       err_info);
    prog.id_for.rbrace =    put_ident("}",       err_info);
    prog.id_for.dot =       put_ident(".",       err_info);
    prog.id_for.colon =     put_ident(":",       err_info);
    prog.id_for.comma =     put_ident(",",       err_info);
    prog.id_for.semicolon = put_ident(";",       err_info);
    prog.id_for.dollar =    put_ident("$",       err_info);
    prog.id_for.r_arrow =   put_ident("->",      err_info);
    prog.id_for.l_arrow =   put_ident("<-",      err_info);
    prog.id_for.out_arrow = put_ident("<->",     err_info);
    prog.id_for.in_arrow =  put_ident("-><-",    err_info);
    prog.id_for.type =      put_ident("type",    err_info);
    prog.id_for.constant =  put_ident("const",   err_info);
    prog.id_for.ref =       put_ident("ref",     err_info);
    prog.id_for.syntax =    put_ident("syntax",  err_info);
    prog.id_for.system =    put_ident("system",  err_info);
    prog.id_for.include =   put_ident("include", err_info);
    prog.id_for.is =        put_ident("is",      err_info);
    prog.id_for.func =      put_ident("func",    err_info);
    prog.id_for.param =     put_ident("param",   err_info);
    prog.id_for.enumlit =   put_ident("enumlit", err_info);
    prog.id_for.subtype =   put_ident("subtype", err_info);
    prog.id_for.newtype =   put_ident("newtype", err_info);
    prog.id_for.action =    put_ident("action",  err_info);
#ifdef TRACE_FINDID
    printf("END init_findid\n");
#endif
  } /* init_findid */
