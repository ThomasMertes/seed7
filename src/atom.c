/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013, 2015, 2021, 2025  Thomas Mertes      */
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
/*  File: seed7/src/atom.c                                          */
/*  Changes: 1990 - 1994, 2013, 2015, 2021, 2025  Thomas Mertes     */
/*  Content: Read the next object from the source file.             */
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
#include "syvarutl.h"
#include "striutl.h"
#include "entutl.h"
#include "objutl.h"
#include "fatal.h"
#include "scanner.h"
#include "symbol.h"
#include "object.h"
#include "findid.h"
#include "error.h"
#include "stat.h"
#include "infile.h"
#if ANY_LOG_ACTIVE
#include "traceutl.h"
#endif

#undef EXTERN
#define EXTERN
#include "atom.h"



static objectType gen_object (void)

  {
    register entityType atomic_entity;
    register propertyType atomic_property;
    register objectType atomic_object;

  /* gen_object */
    logFunction(printf("gen_object\n"););
    if (unlikely(!ALLOC_OBJECT(atomic_object))) {
      fatal_memory_error(SOURCE_POSITION(2051));
    } else {
      if (unlikely(!ALLOC_PROPERTY(atomic_property))) {
        FREE_OBJECT(atomic_object);
        atomic_object = NULL;
        fatal_memory_error(SOURCE_POSITION(2052));
      } else {
        if (unlikely(!ALLOC_RECORD(atomic_entity, entityRecord, count.entity))) {
          FREE_PROPERTY(atomic_property);
          FREE_OBJECT(atomic_object);
          atomic_object = NULL;
          fatal_memory_error(SOURCE_POSITION(2053));
        } else {
          atomic_entity->ident = current_ident;
          atomic_entity->syobject = atomic_object;
          atomic_entity->fparam_list = NULL;
          atomic_entity->data.owner = NULL;
          current_ident->entity = atomic_entity;
          atomic_property->entity = atomic_entity;
          atomic_property->params = NULL;
          atomic_property->file_number = in_file.file_number;
          atomic_property->line = in_file.line;
          atomic_property->syNumberInLine = symbol.syNumberInLine;
          atomic_object->type_of = NULL;
          atomic_object->descriptor.property = atomic_property;
          INIT_POS(atomic_object, in_file.line, in_file.file_number);
          INIT_CATEGORY_OF_OBJ(atomic_object, SYMBOLOBJECT);
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("gen_object --> " FMT_U_MEM ", ",
                       (memSizeType) atomic_object);
                trace1(atomic_object);
                printf("\n"););
    return atomic_object;
  } /* gen_object */



static objectType gen_literal_object (const_objectType typeof_object,
    objectCategory category)

  {
    register objectType literal_object;
    register listType list_elem;

  /* gen_literal_object */
    logFunction(printf("gen_literal_object\n"););
    if (unlikely(!ALLOC_OBJECT(literal_object))) {
      fatal_memory_error(SOURCE_POSITION(2054));
    } else {
      if (unlikely(!ALLOC_L_ELEM(list_elem))) {
        FREE_OBJECT(literal_object);
        literal_object = NULL;
        fatal_memory_error(SOURCE_POSITION(2055));
      } else {
        if (typeof_object == NULL ||
            CATEGORY_OF_OBJ(typeof_object) != TYPEOBJECT) {
          err_warning(LITERAL_TYPE_UNDEFINED);
          literal_object->type_of = NULL;
        } else {
          literal_object->type_of = take_type(typeof_object);
        } /* if */
        literal_object->descriptor.property = prog->property.literal;
        INIT_CATEGORY_OF_OBJ(literal_object, category);
        list_elem->obj = literal_object;
        list_elem->next = prog->literals;
        prog->literals = list_elem;
      } /* if */
    } /* if */
    logFunction(printf("gen_literal_object --> " FMT_U_MEM "\n",
                       (memSizeType) literal_object););
    return literal_object;
  } /* gen_literal_object */



/**
 *  Allocates memory for a new string and assigns the current
 *  string symbol to it.
 *  @return the created string.
 */
static inline striType new_string (void)

  {
    striType stri_created;

  /* new_string */
    logFunction(printf("new_string\n"););
#ifdef ALTERNATE_STRI_LITERALS
    {
      memSizeType stri_created_size = symbol.striValue->size;

      REALLOC_STRI_SIZE_OK(stri_created, symbol.striValue,
          symbol.stri_max, stri_created_size);
    }
    if (unlikely(stri_created == NULL)) {
      fatal_memory_error(SOURCE_POSITION(2056));
    } /* if */
    if (unlikely(!ALLOC_STRI_SIZE_OK(symbol.striValue, symbol.stri_max))) {
      fatal_memory_error(SOURCE_POSITION(2057));
    } /* if */
#else
    if (unlikely(!ALLOC_STRI_SIZE_OK(stri_created, symbol.striValue->size))) {
      fatal_memory_error(SOURCE_POSITION(2058));
    } /* if */
    stri_created->size = symbol.striValue->size;
    memcpy(stri_created->mem, symbol.striValue->mem,
        (size_t) symbol.striValue->size * sizeof(strElemType));
#endif
    logFunction(printf("new_string --> \"%s\"\n",
                       striAsUnquotedCStri(stri_created)););
    return stri_created;
  } /* new_string */



objectType read_atom (void)

  {
    striType stri_created;
    register objectType atomic_object;

  /* read_atom */
    logFunction(printf("read_atom %s\n", symbol.name););
    switch (symbol.sycategory) {
      case NAMESYMBOL:
      case SPECIALSYMBOL:
      case PARENSYMBOL:
        if (current_ident->entity == NULL) {
          atomic_object = gen_object();
        } else if (current_ident->entity->data.owner != NULL) {
          atomic_object = current_ident->entity->data.owner->obj;
        } else {
          atomic_object = current_ident->entity->syobject;
        } /* if */
        break;
      case INTLITERAL:
        atomic_object = gen_literal_object(SYS_INT_TYPE, INTOBJECT);
        atomic_object->value.intValue = symbol.intValue;
        break;
      case BIGINTLITERAL:
        if (unlikely(symbol.bigIntValue == NULL)) {
          fatal_memory_error(SOURCE_POSITION(2059));
          atomic_object = NULL;
        } else {
          atomic_object = gen_literal_object(SYS_BIGINT_TYPE, BIGINTOBJECT);
          atomic_object->value.bigIntValue = symbol.bigIntValue;
          symbol.bigIntValue = NULL;
        } /* if */
        break;
      case CHARLITERAL:
        atomic_object = gen_literal_object(SYS_CHAR_TYPE, CHAROBJECT);
        atomic_object->value.charValue = symbol.charValue;
        break;
      case STRILITERAL:
        /* Because new_string() may trigger a fatal memory error   */
        /* gen_literal_object() is called after it. Otherwise a    */
        /* literal STRIOBJECT without initialized striValue could  */
        /* be created. This would trigger an invalid pointer error */
        /* when prgDestr() calls dump_list(old_prog->literals).    */
        stri_created = new_string();
        atomic_object = gen_literal_object(SYS_STRI_TYPE, STRIOBJECT);
        atomic_object->value.striValue = stri_created;
        break;
#if WITH_FLOAT
      case FLOATLITERAL:
        atomic_object = gen_literal_object(SYS_FLT_TYPE, FLOATOBJECT);
        atomic_object->value.floatValue = symbol.floatValue;
        break;
#endif
      default:
        if (current_ident->entity == NULL) {
          atomic_object = gen_object();
        } else {
          atomic_object = current_ident->entity->syobject;
        } /* if */
        break;
    } /* switch */
    scan_symbol();
    logFunction(printf("read_atom --> " FMT_U_MEM ", ",
                       (memSizeType) atomic_object);
                trace1(atomic_object);
                printf("\n"););
    return atomic_object;
  } /* read_atom */



objectType read_name (void)

  {
    register objectType atomic_object;

  /* read_name */
    logFunction(printf("read_name\n"););
    if (current_ident->entity == NULL) {
      atomic_object = gen_object();
    } else if (current_ident->entity->data.owner != NULL) {
      atomic_object = current_ident->entity->data.owner->obj;
    } else {
      atomic_object = current_ident->entity->syobject;
    } /* if */
    scan_symbol();
    logFunction(printf("read_name -->\n"););
    return atomic_object;
  } /* read_name */
