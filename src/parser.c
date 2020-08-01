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
/*  Module: Analyzer - Parser                                       */
/*  File: seed7/src/parser.c                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Main procedures of the parser.                         */
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
#include "identutl.h"
#include "typeutl.h"
#include "blockutl.h"
#include "actutl.h"
#include "traceutl.h"
#include "executl.h"
#include "objutl.h"
#include "fatal.h"
#include "scanner.h"
#include "object.h"
#include "symbol.h"
#include "findid.h"
#include "error.h"
#include "atom.h"
#include "expr.h"
#include "stat.h"
#include "type.h"
#include "name.h"

#undef EXTERN
#define EXTERN
#include "parser.h"


#undef TRACE_PARSER



#ifdef ANSI_C

static void init_dollar_type (objecttype declared_object,
    typetype meta_type, errinfotype *err_info)
#else

static void init_dollar_type (declared_object, meta_type, err_info)
objecttype declared_object;
typetype meta_type;
errinfotype *err_info;
#endif

  {
    typetype generated_type;

  /* init_dollar_type */
#ifdef TRACE_PARSER
    printf("BEGIN init_dollar_type\n");
#endif
    if ((generated_type = new_type(&prog, meta_type, NULL)) == NULL) {
      *err_info = MEMORY_ERROR;
    } else {
      SET_CATEGORY_OF_OBJ(declared_object, TYPEOBJECT);
      declared_object->value.typevalue = generated_type;
      /* printf("generated_type->name=%lu\n", generated_type->name); */
      if (generated_type->name == NULL &&
          HAS_ENTITY(declared_object) &&
          GET_ENTITY(declared_object)->ident != NULL) {
        generated_type->name = GET_ENTITY(declared_object)->ident;
        /* printf("typ_create: Define ");
        trace1(declared_object);
        printf("\n"); */
      } /* if */
    } /* if */
#ifdef TRACE_PARSER
    printf("END init_dollar_type\n");
#endif
  } /* init_dollar_type */



#ifdef ANSI_C

static INLINE void init_dollar (objecttype declared_object,
    errinfotype *err_info)
#else

static INLINE void init_dollar (declared_object, err_info)
objecttype declared_object;
errinfotype *err_info;
#endif

  {
    objecttype meta_type;
    objecttype basic_type;
    typetype generated_type;

  /* init_dollar */
#ifdef TRACE_PARSER
    printf("BEGIN init_dollar\n");
#endif
    scan_symbol();
    if (current_ident == prog.id_for.newtype) {
      scan_symbol();
      init_dollar_type(declared_object, NULL, err_info);
      /* printf("newtype: declared_object: <%lx> ", declared_object);
      trace1(declared_object);
      printf("\n"); */
    } else if (current_ident == prog.id_for.subtype) {
      scan_symbol();
      meta_type = pars_infix_expression(SCOL_PRIORITY, TRUE);
      if (CATEGORY_OF_OBJ(meta_type) == TYPEOBJECT) {
        init_dollar_type(declared_object, take_type(meta_type), err_info);
      } else {
        err_object(TYPE_EXPECTED, meta_type);
      } /* if */
    } else if (current_ident == prog.id_for.func) {
      scan_symbol();
      basic_type = pars_infix_expression(SCOL_PRIORITY, TRUE);
      if (CATEGORY_OF_OBJ(basic_type) == TYPEOBJECT) {
        if ((generated_type = get_func_type(NULL,
            take_type(basic_type))) == NULL) {
          *err_info = MEMORY_ERROR;
        } else {
          SET_CATEGORY_OF_OBJ(declared_object, TYPEOBJECT);
          declared_object->value.typevalue = generated_type;
          if (generated_type->name == NULL &&
              HAS_ENTITY(declared_object) &&
              GET_ENTITY(declared_object)->ident != NULL) {
            generated_type->name = GET_ENTITY(declared_object)->ident;
            /* printf("typ_create: Define ");
            trace1(declared_object);
            printf("\n"); */
          } /* if */
        } /* if */
      } else {
        err_object(TYPE_EXPECTED, basic_type);
      } /* if */
    } else if (current_ident == prog.id_for.enumlit) {
      scan_symbol();
      SET_CATEGORY_OF_OBJ(declared_object, ENUMLITERALOBJECT);
      declared_object->value.nodevalue = NULL;
    } else if (current_ident == prog.id_for.action) {
      scan_symbol();
      if (symbol.sycategory == STRILITERAL) {
        SET_CATEGORY_OF_OBJ(declared_object, ACTOBJECT);
        if (!find_action(symbol.strivalue, &declared_object->value.actvalue)) {
          err_stri(WRONGACTION, symbol.strivalue);
        } /* if */
        scan_symbol();
      } else {
        err_warning(STRI_EXPECTED);
      } /* if */
    } else {
      err_warning(DOLLAR_VALUE_WRONG);
      if (current_ident != prog.id_for.semicolon) {
        scan_symbol();
      } /* if */
    } /* if */
#ifdef TRACE_PARSER
    printf("END init_dollar ");
    printf("%lu ", (unsigned long) declared_object);
    trace1(declared_object);
    printf("\n");
#endif
  } /* init_dollar */



#ifdef ANSI_C

static void decl_value (objecttype typeof_object, objecttype declared_object,
    int is_dollar_type, errinfotype *err_info)
#else

static void decl_value (typeof_object, declared_object, is_dollar_type, err_info)
objecttype typeof_object;
objecttype declared_object;
int is_dollar_type;
errinfotype *err_info;
#endif

  {
    objecttype init_expression;

  /* decl_value */
#ifdef TRACE_PARSER
    printf("BEGIN decl_value\n");
#endif
    if (current_ident == prog.id_for.is) {
      scan_symbol();
      if (current_ident == prog.id_for.dollar) {
        init_dollar(declared_object, err_info);
        /* printf("A declared_object: <%lx> ", declared_object);
        trace1(declared_object);
        printf("\n");
        printf("type before: <%lx> ", typeof_object);
        trace1(typeof_object);
        printf("\n"); */
        if (is_dollar_type) {
          typeof_object = declared_object;
        } /* if */
#ifdef OUT_OF_ORDER
        if (HAS_ENTITY(typeof_object) &&
            GET_ENTITY(typeof_object)->owner != NULL) {
          typeof_object = GET_ENTITY(typeof_object)->owner->obj;
          /* printf("type after: <%lx> ", typeof_object);
          trace1(typeof_object);
          printf("\n"); */
        } /* if */
#endif
        if (typeof_object != NULL) {
          if (CATEGORY_OF_OBJ(typeof_object) == TYPEOBJECT) {
            declared_object->type_of = take_type(typeof_object);
            /* printf("X declared_object->type_of: <%lx> ", declared_object->type_of);
            trace1(declared_object->type_of);
            printf("\n"); */
          } else {
            err_object(TYPE_EXPECTED, typeof_object);
          } /* if */
        } /* if */
#ifdef OUT_OF_ORDER
        if (is_dollar_type) {
          printf("const $ type is: ");
          trace1(declared_object);
          printf("\n");
        } /* if */
#endif
      } else {
        init_expression = pars_infix_expression(SCOL_PRIORITY, TRUE);
        if (typeof_object != NULL) {
          if (CATEGORY_OF_OBJ(typeof_object) == TYPEOBJECT) {
            declared_object->type_of = take_type(typeof_object);
            do_create(declared_object, init_expression, err_info);
            if (*err_info == CREATE_ERROR) {
              err_object(DECL_FAILED, declared_object);
              *err_info = OKAY_NO_ERROR;
            } /* if */
            if (TEMP_OBJECT(init_expression)) {
/*            printf("destroy ");
              printobject(init_expression->type_of);
              printf(": ");
              printobject(init_expression);
              printf(" ");
              fflush(stdout); */
              do_destroy(init_expression, err_info);
              if (*err_info == OKAY_NO_ERROR) {
                FREE_OBJECT(init_expression);
/*            } else {
                printf("destroy failed\n"); */
              } /* if */
/*            printf("after destroy\n");
            } else {
              printf("not obsolete ");
              printobject(init_expression->type_of);
              printf(": ");
              printobject(init_expression);
              printf("\n"); */
            } /* if */
          } else {
            err_object(TYPE_EXPECTED, typeof_object);
          } /* if */
        } /* if */
      } /* if */
    } else {
      err_ident(EXPECTED_SYMBOL, prog.id_for.is);
    } /* if */
    if (current_ident == prog.id_for.semicolon) {
      scan_symbol();
    } else {
      err_ident(EXPECTED_SYMBOL, prog.id_for.semicolon);
      do {
        scan_symbol();
      } while (current_ident != prog.id_for.semicolon &&
          symbol.sycategory != STOPSYMBOL);
      if (current_ident == prog.id_for.semicolon) {
        scan_symbol();
      } /* if */
    } /* if */
#ifdef TRACE_PARSER
    printf("END decl_value\n");
#endif
  } /* decl_value */



#ifdef ANSI_C

static objecttype decl_name (nodetype node_level, errinfotype *err_info)
#else

static objecttype decl_name (node_level, err_info)
nodetype node_level;
errinfotype *err_info;
#endif

  {
    objecttype object_name;
    objecttype defined_object;

  /* decl_name */
#ifdef TRACE_PARSER
    printf("BEGIN decl_name\n");
#endif
    if (current_ident == prog.id_for.dollar) {
      scan_symbol();
      object_name = pars_infix_expression(COM_PRIORITY, FALSE);
#ifdef OUT_OF_ORDER
if (CATEGORY_OF_OBJ(object_name) == EXPROBJECT) {
  printf("$ object_name/list ");
  prot_list(object_name->value.listvalue);
} else {
  printf("$ object_name/obj ");
  trace1(object_name);
} /* if */
#endif
      defined_object = dollar_entername(node_level, object_name, err_info);
    } else {
      object_name = pars_infix_expression(COM_PRIORITY, FALSE);
#ifdef OUT_OF_ORDER
if (CATEGORY_OF_OBJ(object_name) == EXPROBJECT) {
  printf("n object_name/list ");
  prot_list(object_name->value.listvalue);
} else {
  printf("n object_name/obj ");
  trace1(object_name);
} /* if */
#endif
      defined_object = entername(node_level, object_name, err_info);
    } /* if */
    if (CATEGORY_OF_OBJ(object_name) == EXPROBJECT) {
      object_name->value.listvalue = NULL;
      FREE_OBJECT(object_name);
    } /* if */
#ifdef TRACE_PARSER
    printf("END decl_name\n");
#endif
    return defined_object;
  } /* decl_name */



#ifdef ANSI_C

void decl_const (nodetype node_level, errinfotype *err_info)
#else

void decl_const (node_level, err_info)
nodetype node_level;
errinfotype *err_info;
#endif

  {
    objecttype typeof_object;
    int is_dollar_type;
    objecttype declared_object;

  /* decl_const */
#ifdef TRACE_PARSER
    printf("BEGIN decl_const\n");
#endif
/*    printf("decl_const\n"); */
    grow_stack(err_info);
    if (*err_info == OKAY_NO_ERROR) {
      typeof_object = decl_type(&is_dollar_type, err_info);
      declared_object = decl_name(node_level, err_info);
      if (*err_info == OKAY_NO_ERROR) {
/*        printf("decl_name ");
        trace_entity(GET_ENTITY(declared_object)); */
#ifdef OUT_OF_ORDER
        printf("type before: <%lx> ", typeof_object);
        trace1(typeof_object);
        printf("\n");
        if (GET_ENTITY(typeof_object)->objects != NULL) {
          typeof_object = GET_ENTITY(typeof_object)->objects->obj;
          printf("type after: <%lx> ", typeof_object);
          trace1(typeof_object);
          printf("\n");
        } /* if */
#endif
/*        printf("object: ");
        trace1(declared_object);
        printf("\n"); */
        /* printf("before decl_value\n");
          printf("typeof_object: <%lx> ", typeof_object);
          trace1(typeof_object);
          printf("\n");
          printf("declared_object: <%lx> ", declared_object);
          trace1(declared_object);
          printf("\n");
          printf("declared_object->type_of: <%lx> ", declared_object->type_of);
          printtype(declared_object->type_of);
          printf("\n"); */
        decl_value(typeof_object, declared_object, is_dollar_type, err_info);
        /* printf("after decl_value\n");
          printf("typeof_object: <%lx> ", typeof_object);
          trace1(typeof_object);
          printf("\n");
          printf("declared_object: <%lx> ", declared_object);
          trace1(declared_object);
          printf("\n");
          printf("declared_object->type_of: <%lx> ", declared_object->type_of);
          printtype(declared_object->type_of);
          printf("\n"); */
      } /* if */
      shrink_stack();
    } /* if */
#ifdef TRACE_PARSER
    printf("END decl_const\n");
#endif
  } /* decl_const */
