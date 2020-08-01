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
/*  Module: Analyzer                                                */
/*  File: seed7/src/atom.c                                          */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Read the next object from the source file.             */
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

#undef EXTERN
#define EXTERN
#include "atom.h"

#undef TRACE_OBJECT
#undef TRACE_ATOM



static objectType gen_object (void)

  {
    register entityType atomic_entity;
    register propertyType atomic_property;
    register objectType atomic_object;

  /* gen_object */
#ifdef TRACE_OBJECT
    printf("BEGIN gen_object\n");
#endif
    if (!ALLOC_OBJECT(atomic_object)) {
      fatal_memory_error(SOURCE_POSITION(2051));
    } else {
      if (!ALLOC_RECORD(atomic_property, propertyRecord, count.property)) {
        FREE_OBJECT(atomic_object);
        atomic_object = NULL;
        fatal_memory_error(SOURCE_POSITION(2052));
      } else {
        if (!ALLOC_RECORD(atomic_entity, entityRecord, count.entity)) {
          FREE_RECORD(atomic_property, propertyRecord, count.property);
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
#ifdef TRACE_OBJECT
    printf("END gen_object ");
    printf("%lu ", (unsigned long) atomic_object);
    trace1(atomic_object);
    printf("\n");
#endif
    return atomic_object;
  } /* gen_object */



static objectType gen_literal_object (const_objectType typeof_object,
    objectCategory category)

  {
    register objectType literal_object;
    register listType list_elem;

  /* gen_literal_object */
#ifdef TRACE_OBJECT
    printf("BEGIN gen_literal_object\n");
#endif
    if (!ALLOC_OBJECT(literal_object)) {
      fatal_memory_error(SOURCE_POSITION(2054));
    } else {
      if (!ALLOC_L_ELEM(list_elem)) {
        FREE_OBJECT(literal_object);
        literal_object = NULL;
        fatal_memory_error(SOURCE_POSITION(2055));
      } else {
        if (typeof_object == NULL) {
          err_warning(LITERAL_TYPE_UNDEFINED);
          literal_object->type_of = NULL;
        } else {
          literal_object->type_of = take_type(typeof_object);
        } /* if */
        literal_object->descriptor.property = prog.property.literal;
        INIT_CATEGORY_OF_OBJ(literal_object, category);
        list_elem->obj = literal_object;
        list_elem->next = prog.literals;
        prog.literals = list_elem;
      } /* if */
    } /* if */
#ifdef TRACE_OBJECT
    printf("END gen_literal_object ");
    printf("%lu ", (unsigned long) literal_object);
    trace1(literal_object);
    printf("\n");
#endif
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
#ifdef TRACE_ATOM
    printf("BEGIN new_string\n");
#endif
#ifdef ALTERNATE_STRI_LITERALS
    {
      memSizeType stri_created_size = symbol.striValue->size;

      REALLOC_STRI_SIZE_OK(stri_created, symbol.striValue,
          symbol.stri_max, stri_created_size);
    }
    if (stri_created == NULL) {
      fatal_memory_error(SOURCE_POSITION(2056));
    } /* if */
    COUNT3_STRI(symbol.stri_max, stri_created->size);
    if (!ALLOC_STRI_SIZE_OK(symbol.striValue, symbol.stri_max)) {
      fatal_memory_error(SOURCE_POSITION(2057));
    } /* if */
#else
    if (!ALLOC_STRI_SIZE_OK(stri_created, symbol.striValue->size)) {
      fatal_memory_error(SOURCE_POSITION(2058));
    } /* if */
    stri_created->size = symbol.striValue->size;
    memcpy(stri_created->mem, symbol.striValue->mem,
        (size_t) symbol.striValue->size * sizeof(strElemType));
#endif
#ifdef TRACE_ATOM
    printf("END new_string\n");
#endif
    return stri_created;
  } /* new_string */



objectType read_atom (void)

  {
    register objectType atomic_object;

  /* read_atom */
#ifdef TRACE_ATOM
    printf("BEGIN read_atom %s\n", symbol.NAME);
#endif
    switch (symbol.sycategory) {
      case NAMESYMBOL:
      case SPECIALSYMBOL:
      case PARENSYMBOL:
        if (current_ident->entity == NULL) {
          atomic_object = gen_object();
          /* printf("read_atom a: %s  %lu  %lu  %lu\n", id_string(current_ident), (unsigned long) current_ident->entity, (unsigned long) GET_ENTITY(atomic_object), (unsigned long) atomic_object); */
        } else if (current_ident->entity->data.owner != NULL) {
          atomic_object = current_ident->entity->data.owner->obj;
        } else {
          atomic_object = current_ident->entity->syobject;
          /* if (HAS_PROPERTY(atomic_object)) {
            atomic_object->descriptor.property->file_number = in_file.file_number;
            atomic_object->descriptor.property->line = in_file.line;
            atomic_object->descriptor.property->syNumberInLine = symbol.syNumberInLine;
          } $$ if */
          /* printf("read_atom b: %s  %lu  %lu  %lu\n", id_string(current_ident), (unsigned long) current_ident->entity, (unsigned long) GET_ENTITY(atomic_object), (unsigned long) atomic_object); */
        } /* if */
        break;
      case INTLITERAL:
        atomic_object = gen_literal_object(SYS_INT_TYPE, INTOBJECT);
        atomic_object->value.intValue = symbol.intValue;
        break;
      case BIGINTLITERAL:
        atomic_object = gen_literal_object(SYS_BIGINT_TYPE, BIGINTOBJECT);
        atomic_object->value.bigIntValue = symbol.bigIntValue;
        symbol.bigIntValue = NULL;
        break;
      case CHARLITERAL:
        atomic_object = gen_literal_object(SYS_CHAR_TYPE, CHAROBJECT);
        atomic_object->value.charValue = symbol.charValue;
        break;
      case STRILITERAL:
        atomic_object = gen_literal_object(SYS_STRI_TYPE, STRIOBJECT);
        atomic_object->value.striValue = new_string();
        break;
#ifdef WITH_FLOAT
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
#ifdef TRACE_ATOM
    printf("END read_atom ");
    trace1(atomic_object);
    printf("\n");
#endif
    return atomic_object;
  } /* read_atom */



objectType read_name (void)

  {
    register objectType atomic_object;

  /* read_name */
#ifdef TRACE_ATOM
    printf("BEGIN read_name\n");
#endif
    if (current_ident->entity == NULL) {
      atomic_object = gen_object();
    } else if (current_ident->entity->data.owner != NULL) {
      atomic_object = current_ident->entity->data.owner->obj;
    } else {
      atomic_object = current_ident->entity->syobject;
      /* printf("read_name b: %s  %lu  %lu  %lu\n", id_string(current_ident), (unsigned long) current_ident->entity, (unsigned long) GET_ENTITY(atomic_object), (unsigned long) atomic_object); */
    } /* if */
    scan_symbol();
#ifdef TRACE_ATOM
    printf("END read_name\n");
#endif
    return atomic_object;
  } /* read_name */
