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
/*  Module: General                                                 */
/*  File: seed7/src/identutl.c                                      */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Procedures to maintain objects of type identtype.      */
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
#include "chclsutl.h"
#include "entutl.h"

#undef EXTERN
#define EXTERN
#include "identutl.h"



identtype new_ident (const_ustritype name, sysizetype length)

  {
    register identtype created_ident;

  /* new_ident */
#ifdef TRACE_IDENTUTL
    printf("BEGIN new_ident\n");
#endif
    if (ALLOC_RECORD(created_ident, identrecord, count.ident)) {
      if (!ALLOC_ID_NAME(created_ident->name, length)) {
        FREE_RECORD(created_ident, identrecord, count.ident);
        created_ident = (identtype) NULL;
      } else {
        COUNT_ID_NAME(length);
        memcpy(created_ident->name, name, (size_t) length);
        created_ident->name[length] = '\0';
        created_ident->next1 = NULL;
        created_ident->next2 = NULL;
        created_ident->prefix_priority = 0;
        created_ident->infix_priority = 0;
        created_ident->left_token_priority = -1;
        created_ident->prefix_token = NULL;
        created_ident->infix_token = NULL;
        created_ident->entity = NULL;
      } /* if */
    } /* if */
#ifdef TRACE_IDENTUTL
    printf("END new_ident\n");
#endif
    return created_ident;
  } /* new_ident */



static void free_ident (const_progtype currentProg, identtype old_ident)

  {
    sysizetype length;

  /* free_ident */
#ifdef TRACE_IDENTUTL
    printf("BEGIN free_ident\n");
#endif
    if (old_ident != NULL) {
      length = strlen((cstritype) old_ident->name);
      FREE_ID_NAME(old_ident->name, length);
      free_ident(currentProg, old_ident->next1);
      free_ident(currentProg, old_ident->next2);
      free_entity(currentProg, old_ident->entity);
      FREE_RECORD(old_ident, identrecord, count.ident);
    } /* if */
#ifdef TRACE_IDENTUTL
    printf("END free_ident\n");
#endif
  } /* free_ident */



identtype get_ident (const_ustritype name)

  {
    register identtype ident_found;
    register int comparison;
    register booltype searching;
    sysizetype length;

  /* get_ident */
#ifdef TRACE_IDENTUTL
    printf("BEGIN get_ident\n");
#endif
    length = strlen((const_cstritype) name);
    if (length == 1 && (op_character(name[0]) ||
        char_class(name[0]) == LEFTPARENCHAR ||
        char_class(name[0]) == PARENCHAR)) {
      ident_found = prog.ident.table1[(int) name[0]];
    } else {
      if (IDENT_TABLE(name, length) == NULL) {
        ident_found = new_ident(name, length);
        IDENT_TABLE(name, length) = ident_found;
      } else {
        ident_found = IDENT_TABLE(name, length);
        searching = TRUE;
        do {
          if ((comparison = strncmp((const_cstritype) name,
              (cstritype) ident_found->name, length)) == 0) {
            if (ident_found->name[length] == '\0') {
              searching = FALSE;
            } else {
              if (ident_found->next1 == NULL) {
                ident_found->next1 = new_ident(name, length);
                searching = FALSE;
              } /* if */
              ident_found = ident_found->next1;
            } /* if */
          } else if (comparison < 0) {
            if (ident_found->next1 == NULL) {
              ident_found->next1 = new_ident(name, length);
              searching = FALSE;
            } /* if */
            ident_found = ident_found->next1;
          } else {
            if (ident_found->next2 == NULL) {
              ident_found->next2 = new_ident(name, length);
              searching = FALSE;
            } /* if */
            ident_found = ident_found->next2;
          } /* if */
        } while (searching);
      } /* if */
    } /* if */
#ifdef TRACE_IDENTUTL
    printf("END get_ident\n");
#endif
    return ident_found;
  } /* get_ident */



void close_idents (const_progtype currentProg)

  {
    int position;
    int character;

  /* close_idents */
#ifdef TRACE_FINDID
    printf("BEGIN close_idents\n");
#endif
    for (position = 0; position < ID_TABLE_SIZE; position++) {
      free_ident(currentProg, currentProg->ident.table[position]);
    } /* for */
    for (character = '!'; character <= '~'; character++) {
      if (op_character(character) ||
          char_class(character) == LEFTPARENCHAR ||
          char_class(character) == PARENCHAR) {
        free_ident(currentProg, currentProg->ident.table1[character]);
      } /* if */
    } /* for */
    free_ident(currentProg, currentProg->ident.literal);
    free_ident(currentProg, currentProg->ident.end_of_file);;
#ifdef TRACE_FINDID
    printf("END close_idents\n");
#endif
  } /* close_idents */



void init_idents (progtype currentProg, errinfotype *err_info)

  {
    int position;
    uchartype character;

  /* init_idents */
#ifdef TRACE_IDENTUTL
    printf("BEGIN init_ident\n");
#endif
    for (position = 0; position < ID_TABLE_SIZE; position++) {
      currentProg->ident.table[position] = NULL;
    } /* for */
    for (character = '!'; character <= '~'; character++) {
      if (op_character(character) ||
          char_class(character) == LEFTPARENCHAR ||
          char_class(character) == PARENCHAR) {
        if ((currentProg->ident.table1[(int) character] = new_ident(&character, 1)) == NULL) {
          *err_info = MEMORY_ERROR;
        } /* if */
      } /* if */
    } /* for */
    if ((currentProg->ident.literal = new_ident((const_ustritype) " *SIMPLE_IDENT* ", 16)) == NULL) {
      *err_info = MEMORY_ERROR;
    } /* if */
    if ((currentProg->ident.end_of_file = new_ident((const_ustritype) "END OF FILE", 11)) == NULL) {
      *err_info = MEMORY_ERROR;
    } /* if */
    if (*err_info != OKAY_NO_ERROR) {
      close_idents(currentProg);
    } /* if */
#ifdef TRACE_IDENTUTL
    printf("END init_ident\n");
#endif
  } /* init_idents */
