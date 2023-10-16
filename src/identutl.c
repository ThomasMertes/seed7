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
/*  Module: General                                                 */
/*  File: seed7/src/identutl.c                                      */
/*  Changes: 1991 - 1994, 2012, 2013, 2015, 2021  Thomas Mertes     */
/*  Content: Functions to maintain objects of type identType.       */
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
#include "chclsutl.h"
#include "entutl.h"

#undef EXTERN
#define EXTERN
#include "identutl.h"



identType new_ident (const_ustriType name, sySizeType length)

  {
    register identType created_ident;

  /* new_ident */
    logFunction(printf("new_ident\n"););
    if (ALLOC_RECORD(created_ident, identRecord, count.ident)) {
      if (!ALLOC_ID_NAME(created_ident->name, length)) {
        FREE_RECORD(created_ident, identRecord, count.ident);
        created_ident = NULL;
      } else {
        COUNT_ID_NAME(length);
        memcpy(created_ident->name, name, (size_t) length);
        created_ident->name[length] = '\0';
        created_ident->length = length;
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
    logFunction(printf("new_ident -->\n"););
    return created_ident;
  } /* new_ident */



static void free_ident (const_progType currentProg, identType old_ident)

  { /* free_ident */
    logFunction(printf("free_ident\n"););
    if (old_ident != NULL) {
      FREE_ID_NAME(old_ident->name, old_ident->length);
      free_ident(currentProg, old_ident->next1);
      free_ident(currentProg, old_ident->next2);
      free_entity(currentProg, old_ident->entity);
      FREE_RECORD(old_ident, identRecord, count.ident);
    } /* if */
    logFunction(printf("free_ident ->\n"););
  } /* free_ident */



identType get_ident (progType currentProg, const_ustriType name)

  {
    register identType ident_found;
    register int comparison;
    register boolType searching;
    sySizeType length;

  /* get_ident */
    logFunction(printf("get_ident\n"););
    length = strlen((const_cstriType) name);
    if (length == 1 && (op_character(name[0]) ||
        char_class(name[0]) == LEFTPARENCHAR ||
        char_class(name[0]) == PARENCHAR)) {
      ident_found = currentProg->ident.table1[(int) name[0]];
    } else {
      if (IDENT_TABLE(currentProg, name, length) == NULL) {
        ident_found = new_ident(name, length);
        IDENT_TABLE(currentProg, name, length) = ident_found;
      } else {
        ident_found = IDENT_TABLE(currentProg, name, length);
        searching = TRUE;
        do {
          if ((comparison = strncmp((const_cstriType) name,
              (cstriType) ident_found->name, length)) == 0) {
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
    logFunction(printf("get_ident -->\n"););
    return ident_found;
  } /* get_ident */



void close_idents (const_progType currentProg)

  {
    int position;
    int character;

  /* close_idents */
    logFunction(printf("close_idents\n"););
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
    free_ident(currentProg, currentProg->ident.end_of_file);
    logFunction(printf("close_idents -->\n"););
  } /* close_idents */



void init_idents (progType currentProg, errInfoType *err_info)

  {
    int position;
    ucharType character;

  /* init_idents */
    logFunction(printf("init_ident\n"););
    for (position = 0; position < ID_TABLE_SIZE; position++) {
      currentProg->ident.table[position] = NULL;
    } /* for */
    for (character = '!'; character <= '~'; character++) {
      if (op_character(character) ||
          char_class(character) == LEFTPARENCHAR ||
          char_class(character) == PARENCHAR) {
        if ((currentProg->ident.table1[(int) character] =
             new_ident(&character, (sySizeType) 1)) == NULL) {
          *err_info = MEMORY_ERROR;
        } /* if */
      } /* if */
    } /* for */
    if ((currentProg->ident.literal =
         new_ident((const_ustriType) " *SIMPLE_IDENT* ", (sySizeType) 16)) == NULL) {
      *err_info = MEMORY_ERROR;
    } /* if */
    if ((currentProg->ident.end_of_file =
         new_ident((const_ustriType) "END OF FILE", (sySizeType) 11)) == NULL) {
      *err_info = MEMORY_ERROR;
    } /* if */
    logFunction(printf("init_ident -->\n"););
  } /* init_idents */
