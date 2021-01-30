/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000, 2012, 2013, 2015  Thomas Mertes      */
/*                2021  Thomas Mertes                               */
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
/*  Changes: 1991 - 1994, 2012, 2013, 2015, 2021  Thomas Mertes     */
/*  Content: Procedures to maintain the identifier table.           */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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
#include "infile.h"

#undef EXTERN
#define EXTERN
#include "findid.h"



static inline identType id_generation (ustriType name, sySizeType length)

  {
    register identType created_ident;

  /* id_generation */
    logFunction(printf("id_generation\n"););
    if ((created_ident = new_ident(name, length)) == NULL) {
      fatal_memory_error(SOURCE_POSITION(2041));
    } /* if */
    logFunction(printf("id_generation -->\n"););
    return created_ident;
  } /* id_generation */



void find_normal_ident (sySizeType length)

  {                                                             /*  0.62% */
    register identType search_ident;
    register int comparison;

  /* find_normal_ident */
    logFunction(printf("find_normal_ident\n"););
    if ((search_ident = IDENT_TABLE(prog, symbol.name, length)) != NULL) { /*  1.49% */
      if ((comparison = strcmp((cstriType) symbol.name,
          (cstriType) search_ident->name)) != 0) {              /*  0.73% */
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
              comparison = strcmp((cstriType) symbol.name,
                  (cstriType) search_ident->name);              /*  0.09% */
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
              comparison = strcmp((cstriType) symbol.name,
                  (cstriType) search_ident->name);              /*  0.07% */
            } /* if */
          } /* if */
        } while (comparison != 0);                              /*  0.07% */
      } /* if */
      current_ident = search_ident;                             /*  0.12% */
    } else {
      current_ident = id_generation(symbol.name, length);       /*  0.01% */
      IDENT_TABLE(prog, symbol.name, length) = current_ident;   /*  0.02% */
    } /* if */
    logFunction(printf("find_normal_ident -->\n"););
  } /* find_normal_ident */                                     /*  0.62% */



static identType put_ident (progType aProgram, const_cstriType stri,
    errInfoType *err_info)

  {
    register identType ident_found;

  /* put_ident */
    logFunction(printf("put_ident\n"););
    if ((ident_found = get_ident(aProgram, (const_ustriType) stri)) == NULL) {
      *err_info = MEMORY_ERROR;
    } /* if */
    logFunction(printf("put_ident -->\n"););
    return ident_found;
  } /* put_ident */



void check_list_of_syntax_elements (const_listType elem_list)

  { /* check_list_of_syntax_elements */
    logFunction(printf("check_list_of_syntax_elements\n"););
    while (elem_list != NULL) {
      if (!HAS_ENTITY(elem_list->obj)) {
        if (CATEGORY_OF_OBJ(elem_list->obj) != EXPROBJECT) {
          err_object(IDENT_EXPECTED, elem_list->obj);
        } /* if */
      } /* if */
#ifdef OUT_OF_ORDER
      if (GET_ENTITY(elem_list->obj) == prog->entity.literal) {
        if (CATEGORY_OF_OBJ(elem_list->obj) != EXPROBJECT) {
          err_object(IDENT_EXPECTED, elem_list->obj);
        } /* if */
      } /* if */
#endif
      elem_list = elem_list->next;
    } /* while */
    logFunction(printf("check_list_of_syntax_elements -->\n"););
  } /* check_list_of_syntax_elements */



static void clean_ident_tree (identType actual_ident)

  { /* clean_ident_tree */
    logFunction(printf("clean_ident_tree\n"););
    if (actual_ident != NULL) {
      clean_ident_tree(actual_ident->next1);
      free_tokens(actual_ident->prefix_token);
      actual_ident->prefix_token = NULL;
      free_tokens(actual_ident->infix_token);
      actual_ident->infix_token = NULL;
      clean_ident_tree(actual_ident->next2);
    } /* if */
    logFunction(printf("clean_ident_tree -->\n"););
  } /* clean_ident_tree */



void clean_idents (progType currentProg)

  {
    int position;
    int character;
    identType actual_ident;

  /* clean_idents */
    logFunction(printf("clean_idents\n"););
    for (position = 0; position < ID_TABLE_SIZE; position++) {
      clean_ident_tree(currentProg->ident.table[position]);
    } /* for */
    for (character = '!'; character <= '~'; character++) {
      if (op_character(character) ||
          char_class(character) == LEFTPARENCHAR ||
          char_class(character) == PARENCHAR) {
        actual_ident = currentProg->ident.table1[character];
        if (actual_ident != NULL) {
          free_tokens(actual_ident->prefix_token);
          actual_ident->prefix_token = NULL;
          free_tokens(actual_ident->infix_token);
          actual_ident->infix_token = NULL;
        } /* if */
      } /* if */
    } /* for */
    logFunction(printf("clean_idents -->\n"););
  } /* clean_idents */



static void wri_binary_ident_tree (const_identType actual_ident)

  { /* wri_binary_ident_tree */
    logFunction(printf("wri_binary_ident_tree\n"););
    if (actual_ident != NULL) {
      wri_binary_ident_tree(actual_ident->next1);
      prot_cstri8(id_string(actual_ident));
      prot_cstri(" ");
      if (actual_ident->entity != NULL &&
          actual_ident->entity->syobject != NULL) {
        if (CATEGORY_OF_OBJ(actual_ident->entity->syobject) == SYMBOLOBJECT) {
          prot_cstri(" ");
          prot_string(get_file_name(GET_POS_FILE_NUM(actual_ident->entity->syobject)));
          prot_cstri("(");
          prot_int((intType) GET_POS_LINE_NUM(actual_ident->entity->syobject));
          prot_cstri(")");
        } /* if */
      } /* if */
      prot_nl();
      wri_binary_ident_tree(actual_ident->next2);
    } /* if */
    logFunction(printf("wri_binary_ident_tree -->\n"););
  } /* wri_binary_ident_tree */



void write_idents (progType currentProg)

  {
    int position;
    int character;
    identType actual_ident;

  /* write_idents */
    logFunction(printf("write_idents\n"););
    for (position = 0; position < ID_TABLE_SIZE; position++) {
      prot_cstri(" ====== ");
      prot_int((intType) position);
      prot_cstri(" ======\n");
      wri_binary_ident_tree(currentProg->ident.table[position]);
    } /* for */
    for (character = '!'; character <= '~'; character++) {
      if (op_character(character) ||
          char_class(character) == LEFTPARENCHAR ||
          char_class(character) == PARENCHAR) {
        actual_ident = currentProg->ident.table1[character];
        if (actual_ident != NULL) {
          prot_cstri8(id_string(actual_ident));
          prot_cstri(" ");
          if (actual_ident->entity != NULL &&
              actual_ident->entity->syobject != NULL) {
            if (CATEGORY_OF_OBJ(actual_ident->entity->syobject) == SYMBOLOBJECT) {
              prot_cstri(" ");
              prot_string(get_file_name(GET_POS_FILE_NUM(actual_ident->entity->syobject)));
              prot_cstri("(");
              prot_int((intType) GET_POS_LINE_NUM(actual_ident->entity->syobject));
              prot_cstri(")");
            } /* if */
          } /* if */
          prot_nl();
        } /* if */
      } /* if */
    } /* for */
    logFunction(printf("write_idents -->\n"););
  } /* write_idents */



void init_findid (progType aProg, errInfoType *err_info)

  { /* init_findid */
    logFunction(printf("init_findid\n"););
    aProg->id_for.lparen =    put_ident(aProg, "(",       err_info);
    aProg->id_for.lbrack =    put_ident(aProg, "[",       err_info);
    aProg->id_for.lbrace =    put_ident(aProg, "{",       err_info);
    aProg->id_for.rparen =    put_ident(aProg, ")",       err_info);
    aProg->id_for.rbrack =    put_ident(aProg, "]",       err_info);
    aProg->id_for.rbrace =    put_ident(aProg, "}",       err_info);
    aProg->id_for.dot =       put_ident(aProg, ".",       err_info);
    aProg->id_for.colon =     put_ident(aProg, ":",       err_info);
    aProg->id_for.comma =     put_ident(aProg, ",",       err_info);
    aProg->id_for.semicolon = put_ident(aProg, ";",       err_info);
    aProg->id_for.dollar =    put_ident(aProg, "$",       err_info);
    aProg->id_for.r_arrow =   put_ident(aProg, "->",      err_info);
    aProg->id_for.l_arrow =   put_ident(aProg, "<-",      err_info);
    aProg->id_for.out_arrow = put_ident(aProg, "<->",     err_info);
    aProg->id_for.in_arrow =  put_ident(aProg, "-><-",    err_info);
    aProg->id_for.type =      put_ident(aProg, "type",    err_info);
    aProg->id_for.constant =  put_ident(aProg, "const",   err_info);
    aProg->id_for.ref =       put_ident(aProg, "ref",     err_info);
    aProg->id_for.syntax =    put_ident(aProg, "syntax",  err_info);
    aProg->id_for.system =    put_ident(aProg, "system",  err_info);
    aProg->id_for.include =   put_ident(aProg, "include", err_info);
    aProg->id_for.is =        put_ident(aProg, "is",      err_info);
    aProg->id_for.func =      put_ident(aProg, "func",    err_info);
    aProg->id_for.param =     put_ident(aProg, "param",   err_info);
    aProg->id_for.enumlit =   put_ident(aProg, "enumlit", err_info);
    aProg->id_for.subtype =   put_ident(aProg, "subtype", err_info);
    aProg->id_for.newtype =   put_ident(aProg, "newtype", err_info);
    aProg->id_for.action =    put_ident(aProg, "action",  err_info);
    logFunction(printf("init_findid -->\n"););
  } /* init_findid */
