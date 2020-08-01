/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2007  Thomas Mertes                        */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Analyzer                                                */
/*  File: seed7/src/match.c                                         */
/*  Changes: 1995, 1999, 2000, 2007  Thomas Mertes                  */
/*  Content: Procedures to do static match on expressions.          */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "identutl.h"
#include "listutl.h"
#include "entutl.h"
#include "traceutl.h"
#include "memory.h"
#include "error.h"
#include "prclib.h"

#undef EXTERN
#define EXTERN
#include "match.h"


#undef TRACE_MATCH



#ifdef ANSI_C
static objecttype match_subexpr (objecttype, nodetype, listtype,
     booltype, booltype);
#else
static objecttype match_subexpr ();
#endif



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

static int variable_obj (objecttype object)
#else

static int variable_obj (object)
objecttype object;
#endif

  {
    int result;

  /* variable_obj */
#ifdef TRACE_MATCH
    printf("BEGIN variable_obj\n");
#endif
    if (CLASS_OF_OBJ(object) == CALLOBJECT ||
        CLASS_OF_OBJ(object) == MATCHOBJECT) {
      result = (object->type_of != NULL &&
          object->type_of->is_varfunc_type);
    } else {
      result = VAR_OBJECT(object);
    } /* if */
    /* printf("variable_obj %d ", result);
    trace1(object);
    printf("\n"); */
#ifdef TRACE_MATCH
    printf("END variable_obj\n");
#endif
    return(result);
  } /* variable_obj */



#ifdef ANSI_C

static void check_parameter (objecttype expr_object, objecttype actual_param)
#else

static void check_parameter (expr_object, actual_param)
objecttype expr_object;
objecttype actual_param;
#endif

  { /* check_parameter */
    if (CLASS_OF_OBJ(actual_param) == CALLOBJECT) {
      if (actual_param->type_of != NULL &&
          !actual_param->type_of->is_varfunc_type) {
        err_expr_obj(WRONGACCESSRIGHT, expr_object, actual_param);
      } /* if */
    } else {
      if (!VAR_OBJECT(actual_param)) {
        /* printf("class: %s\n", class_stri(CLASS_OF_OBJ(actual_param))); */
        err_expr_obj(WRONGACCESSRIGHT, expr_object, actual_param);
      } /* if */
    } /* if */
  } /* check_parameter */
#endif



#ifdef ANSI_C

static void check_access_rights (objecttype object)
#else

static void check_access_rights (object)
objecttype object;
#endif

  {
    listtype name_elem;
    listtype expr_list;

  /* check_access_rights */
#ifdef TRACE_MATCH
    printf("BEGIN check_access_rights\n");
#endif
/*    printf("check_access_rights ");
    trace1(object);
    printf("\n"); */
    if (object->value.listvalue != NULL) {
      if (object->value.listvalue->obj != NULL) {
/*        printf("call object ");
        trace1(object->value.listvalue->obj);
        printf("\n"); */
        if (HAS_DESCRIPTOR_ENTITY(object->value.listvalue->obj)) {
          name_elem = object->value.listvalue->obj->descriptor.entity->name_list;
          expr_list = object->value.listvalue->next;
          while (name_elem != NULL && expr_list != NULL) {
            if (CLASS_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
              /* printf("param_obj/param ");
              trace1(name_elem->obj->value.objvalue);
              printf("\n"); */
              if (CLASS_OF_OBJ(name_elem->obj->value.objvalue) == REFPARAMOBJECT &&
                  VAR_OBJECT(name_elem->obj->value.objvalue)) {
                if (CLASS_OF_OBJ(expr_list->obj) == CALLOBJECT) {
                  if (expr_list->obj->type_of != NULL &&
                      !expr_list->obj->type_of->is_varfunc_type) {
                    err_expr_obj(WRONGACCESSRIGHT, object, expr_list->obj);
                  } /* if */
                } else {
                  if (!VAR_OBJECT(expr_list->obj)) {
                    /* printf("class: %s\n", class_stri(CLASS_OF_OBJ(expr_list->obj))); */
                    err_expr_obj(WRONGACCESSRIGHT, object, expr_list->obj);
                  } /* if */
                } /* if */
              } /* if */
            } else {
              /* printf("param_obj ");
              trace1(name_elem->obj);
              printf("\n"); */
            } /* if */
            /* printf("expr_obj ");
            trace1(expr_list->obj);
            printf("\n"); */
            name_elem = name_elem->next;
            expr_list = expr_list->next;
          } /* while */
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_MATCH
    printf("END check_access_rights\n");
#endif
  } /* check_access_rights */



#ifdef ANSI_C

void update_owner (objecttype expr_object)
#else

void update_owner (expr_object)
objecttype expr_object;
#endif

  {
    listtype expr_list;
    objecttype current_element;

  /* update_owner */
#ifdef TRACE_MATCH
    printf("BEGIN update_owner\n");
#endif
    expr_list = expr_object->value.listvalue;
    while (expr_list != NULL) {
      current_element = expr_list->obj;
      if (HAS_DESCRIPTOR_ENTITY(current_element) &&
          current_element->descriptor.entity->owner != NULL) {
        expr_list->obj = current_element->descriptor.entity->owner->obj;
        current_element = expr_list->obj;
      } /* if */
      if (CLASS_OF_OBJ(current_element) == EXPROBJECT) {
        update_owner(current_element);
      } else if (CLASS_OF_OBJ(current_element) == VALUEPARAMOBJECT ||
          CLASS_OF_OBJ(current_element) == REFPARAMOBJECT) {
        if (current_element->value.objvalue != NULL) {
          /* printf("Parameter already has value: ");
          trace1(current_element);
          printf("\n"); */
          expr_list->obj = current_element->value.objvalue;
          if (CLASS_OF_OBJ(expr_list->obj) == VALUEPARAMOBJECT ||
              CLASS_OF_OBJ(expr_list->obj) == REFPARAMOBJECT) {
            if (expr_list->obj->value.objvalue != NULL) {
              printf("Parameter aggain has value: ");
              trace1(expr_list->obj);
              printf("\n");
            } /* if */
          } /* if */
        } /* if */
      } /* if */
      expr_list = expr_list->next;
    } /* while */
#ifdef TRACE_MATCH
    printf("END update_owner\n");
#endif
  } /* update_owner */



#ifdef ANSI_C

objecttype match_object (objecttype object)
#else

objecttype match_object (object)
objecttype object;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    objecttype result;

  /* match_object */
#ifdef TRACE_MATCH
    printf("BEGIN match_object\n");
#endif
    if (trace.match) {
      printf("\nbegin match_object ");
      trace1(object);
      printf("\n");
    } /* if */
    switch (CLASS_OF_OBJ(object)) {
      case BLOCKOBJECT:
      case TYPEOBJECT:
      case INTOBJECT:
      case BIGINTOBJECT:
      case CHAROBJECT:
      case STRIOBJECT:
      case ARRAYOBJECT:
      case STRUCTOBJECT:
      case FILEOBJECT:
      case LISTOBJECT:
#ifdef WITH_FLOAT
      case FLOATOBJECT:
#endif
      case WINOBJECT:
      case REFOBJECT:
      case REFLISTOBJECT:
      case EXPROBJECT:
      case ACTOBJECT:
      case ENUMLITERALOBJECT:
      case VARENUMOBJECT:
      case REFPARAMOBJECT:
      case DECLAREDOBJECT:
      case FORWARDOBJECT:
        if (ALLOC_OBJECT(result)) {
          result->type_of = object->type_of;
          result->descriptor.entity = entity.literal;
          result->value.listvalue = NULL;
          INIT_CLASS_OF_OBJ(result, CALLOBJECT);
          incl_list(&result->value.listvalue, object, &err_info);
        } /* if */
        break;
      case MATCHOBJECT:
        SET_CLASS_OF_OBJ(object, CALLOBJECT);
        result = object;
        break;
      case CALLOBJECT:
        result = object;
        break;
      default:
        printf("### match_object of ");
        trace1(object);
        result = NULL;
        break;
    } /* switch */
    if (trace.match) {
      printf("end match_object ");
      trace1(object);
      printf(" ==> ");
      trace1(result);
      printf("\n");
    } /* if */
#ifdef TRACE_MATCH
    printf("END match_object\n");
#endif
    return(result);
  } /* match_object */



#ifdef ANSI_C

static objecttype match_object2 (objecttype object, objecttype expr_object)
#else

objecttype match_object2 (object, expr_object)
objecttype object;
objecttype expr_object;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    objecttype result;

  /* match_object2 */
#ifdef TRACE_MATCH
    printf("BEGIN match_object\n");
#endif
    if (trace.match) {
      printf("\nbegin match_object ");
      trace1(object);
      printf("\n");
    } /* if */
    switch (CLASS_OF_OBJ(object)) {
      case BLOCKOBJECT:
      case TYPEOBJECT:
      case INTOBJECT:
      case BIGINTOBJECT:
      case CHAROBJECT:
      case STRIOBJECT:
      case ARRAYOBJECT:
      case STRUCTOBJECT:
      case FILEOBJECT:
      case LISTOBJECT:
#ifdef WITH_FLOAT
      case FLOATOBJECT:
#endif
      case WINOBJECT:
      case REFOBJECT:
      case REFLISTOBJECT:
      case EXPROBJECT:
      case ACTOBJECT:
      case ENUMLITERALOBJECT:
      case VARENUMOBJECT:
      case REFPARAMOBJECT:
      case DECLAREDOBJECT:
      case FORWARDOBJECT:
        if (ALLOC_OBJECT(result)) {
          result->type_of = object->type_of;
          if (HAS_POSINFO(expr_object)) {
            result->descriptor.posinfo = expr_object->descriptor.posinfo;
            INIT_CLASS_OF_POSINFO(result, CALLOBJECT);
          } else {
            result->descriptor.entity = entity.literal;
            INIT_CLASS_OF_OBJ(result, CALLOBJECT);
          } /* if */
          result->value.listvalue = NULL;
          incl_list(&result->value.listvalue, object, &err_info);
        } /* if */
        break;
      case MATCHOBJECT:
        SET_CLASS_OF_OBJ(object, CALLOBJECT);
        result = object;
        break;
      case CALLOBJECT:
        result = object;
        break;
      default:
        printf("### match_object of ");
        trace1(object);
        result = NULL;
        break;
    } /* switch */
    if (trace.match) {
      printf("end match_object ");
      trace1(object);
      printf(" ==> ");
      trace1(result);
      printf("\n");
    } /* if */
#ifdef TRACE_MATCH
    printf("END match_object\n");
#endif
    return(result);
  } /* match_object2 */



#ifdef ANSI_C

static objecttype match_subexpr_var (objecttype expr_object,
    nodetype start_node, typetype object_type,
    listtype rest_of_expression, booltype check_access_right,
    booltype look_for_interfaces)
#else

static objecttype match_subexpr_var (expr_object, start_node,
    object_type, rest_of_expression, check_access_right,
    look_for_interfaces)
objecttype expr_object;
nodetype start_node;
typetype object_type;
listtype rest_of_expression;
booltype check_access_right;
booltype look_for_interfaces;
#endif

  {
    nodetype node_found;
    objecttype matched_object;

  /* match_subexpr_var */
    matched_object = NULL;
    if (trace.match) {
      printf("//ST1v//");
      trace1(object_type->match_obj);
      printf("=");
      printf("%ld", (inttype) object_type);
      fflush(stdout);
    } /* if */
    node_found = find_node(start_node->inout_param, object_type->match_obj);
    if (node_found != NULL) {
      matched_object = match_subexpr(expr_object, node_found,
          rest_of_expression, check_access_right, look_for_interfaces);
      if (matched_object == NULL) {
        node_found = find_node(start_node->other_param, object_type->match_obj);
        if (node_found != NULL) {
          matched_object = match_subexpr(expr_object, node_found,
              rest_of_expression, check_access_right, look_for_interfaces);
        } /* if */
      } /* if */
    } else {
      node_found = find_node(start_node->other_param, object_type->match_obj);
      if (node_found != NULL) {
        matched_object = match_subexpr(expr_object, node_found,
            rest_of_expression, check_access_right, look_for_interfaces);
      } /* if */
    } /* if */
    return(matched_object);
  } /* match_subexpr_var */



#ifdef ANSI_C

static objecttype match_subexpr_const (objecttype expr_object,
    nodetype start_node, typetype object_type,listtype rest_of_expression,
    booltype check_access_right, booltype look_for_interfaces)
#else

static objecttype match_subexpr_const (expr_object, start_node,
    object_type, rest_of_expression, check_access_right,
    look_for_interfaces)
objecttype expr_object;
nodetype start_node;
typetype object_type;
listtype rest_of_expression;
booltype check_access_right;
booltype look_for_interfaces;
#endif

  {
    nodetype node_found;
    objecttype matched_object;

  /* match_subexpr_const */
    matched_object = NULL;
    if (trace.match) {
      printf("//ST1o//");
      trace1(object_type->match_obj);
      printf("=");
      printf("%ld", (inttype) object_type);
      fflush(stdout);
    } /* if */
    node_found = find_node(start_node->other_param, object_type->match_obj);
    if (node_found != NULL) {
      matched_object = match_subexpr(expr_object, node_found,
          rest_of_expression, check_access_right, look_for_interfaces);
      if (matched_object == NULL && check_access_right) {
        node_found = find_node(start_node->inout_param, object_type->match_obj);
        if (node_found != NULL) {
          matched_object = match_subexpr(expr_object, node_found,
              rest_of_expression, check_access_right, look_for_interfaces);
        } /* if */
      } /* if */
    } else if (check_access_right) {
      node_found = find_node(start_node->inout_param, object_type->match_obj);
      if (node_found != NULL) {
        matched_object = match_subexpr(expr_object, node_found,
            rest_of_expression, check_access_right, look_for_interfaces);
      } /* if */
    } /* if */
    return(matched_object);
  } /* match_subexpr_const */



#ifdef ANSI_C

static objecttype match_subexpr_type (objecttype expr_object,
    nodetype start_node, typetype object_type, booltype is_variable_obj,
    listtype rest_of_expression, booltype check_access_right,
    booltype look_for_interfaces)
#else

static objecttype match_subexpr_type (expr_object, start_node,
    object_type, is_variable_obj, rest_of_expression,
    check_access_right, look_for_interfaces)
objecttype expr_object;
nodetype start_node;
typetype object_type;
booltype is_variable_obj;
listtype rest_of_expression;
booltype check_access_right;
booltype look_for_interfaces;
#endif

  {
    typelisttype interface_list;
    objecttype matched_object;

  /* match_subexpr_type */
#ifdef TRACE_MATCH
    printf("BEGIN match_subexpr_type\n");
#endif
    matched_object = NULL;
    if (is_variable_obj) {
      if (look_for_interfaces) {
        interface_list = object_type->interfaces;
        while (interface_list != NULL && matched_object == NULL) {
          matched_object = match_subexpr_var(expr_object,start_node,
              interface_list->type_elem, rest_of_expression,
              check_access_right, look_for_interfaces);
          interface_list = interface_list->next;
        } /* while */
        if (matched_object != NULL &&
            (CLASS_OF_OBJ(matched_object) == MATCHOBJECT ||
            CLASS_OF_OBJ(matched_object) == CALLOBJECT) &&
            matched_object->value.listvalue != NULL &&
            matched_object->value.listvalue->obj != NULL) {
          if (CLASS_OF_OBJ(matched_object->value.listvalue->obj) != ACTOBJECT ||
              matched_object->value.listvalue->obj->value.actvalue != prc_dynamic) {
            pop_list(&matched_object->value.listvalue);
            SET_CLASS_OF_OBJ(expr_object, EXPROBJECT);
            matched_object = NULL;
          } /* if */
        } /* if */
      } /* if */
      if (matched_object == NULL) {
        do {
          matched_object = match_subexpr_var(expr_object, start_node,
              object_type, rest_of_expression, check_access_right,
              look_for_interfaces);
          object_type = object_type->meta;
        } while (object_type != NULL && matched_object == NULL);
      } /* if */
    } else {
      if (look_for_interfaces) {
        interface_list = object_type->interfaces;
        while (interface_list != NULL && matched_object == NULL) {
          matched_object = match_subexpr_const(expr_object, start_node,
              interface_list->type_elem, rest_of_expression,
              check_access_right, look_for_interfaces);
          interface_list = interface_list->next;
        } /* while */
        if (matched_object != NULL &&
            (CLASS_OF_OBJ(matched_object) == MATCHOBJECT ||
            CLASS_OF_OBJ(matched_object) == CALLOBJECT) &&
            matched_object->value.listvalue != NULL &&
            matched_object->value.listvalue->obj != NULL) {
          if (CLASS_OF_OBJ(matched_object->value.listvalue->obj) != ACTOBJECT ||
              matched_object->value.listvalue->obj->value.actvalue != prc_dynamic) {
            pop_list(&matched_object->value.listvalue);
            SET_CLASS_OF_OBJ(expr_object, EXPROBJECT);
            matched_object = NULL;
          } /* if */
        } /* if */
      } /* if */
      if (matched_object == NULL) {
        do {
          matched_object = match_subexpr_const(expr_object, start_node,
              object_type, rest_of_expression,
              check_access_right, look_for_interfaces);
          object_type = object_type->meta;
        } while (object_type != NULL && matched_object == NULL);
      } /* if */
    } /* if */
#ifdef TRACE_MATCH
    printf("END match_subexpr_type\n");
#endif
    return(matched_object);
  } /* match_subexpr_type */



#ifdef ANSI_C

static objecttype match_subexpr_attr (objecttype expr_object,
    nodetype start_node, typetype object_type,
    listtype rest_of_expression, booltype check_access_right,
    booltype look_for_interfaces)
#else

static objecttype match_subexpr_attr (expr_object, start_node,
    object_type, rest_of_expression, check_access_right,
    look_for_interfaces)
objecttype expr_object;
nodetype start_node;
typetype object_type;
listtype rest_of_expression;
booltype check_access_right;
booltype look_for_interfaces;
#endif

  {
    nodetype node_found;
    objecttype matched_object;

  /* match_subexpr_attr */
#ifdef TRACE_MATCH
    printf("BEGIN match_subexpr_attr\n");
#endif
    matched_object = NULL;
    do {
      if (trace.match) {
        printf("//AT1//");
        trace1(object_type->match_obj);
        fflush(stdout);
      } /* if */
      node_found = find_node(start_node->attr, object_type->match_obj);
      if (node_found != NULL) {
        matched_object = match_subexpr(expr_object, node_found,
            rest_of_expression, check_access_right,
            look_for_interfaces);
      } /* if */
      object_type = object_type->meta;
    } while (object_type != NULL && matched_object == NULL);
#ifdef TRACE_MATCH
    printf("END match_subexpr_attr\n");
#endif
    return(matched_object);
  } /* match_subexpr_attr */



#ifdef ANSI_C

static objecttype match_subexpr (objecttype expr_object,
    nodetype start_node, listtype match_expr, booltype check_access_right,
    booltype look_for_interfaces)
#else

static objecttype match_subexpr (expr_object, start_node,
    match_expr, check_access_right, look_for_interfaces)
objecttype expr_object;
nodetype start_node;
listtype match_expr;
booltype check_access_right;
booltype look_for_interfaces;
#endif

  {
    nodetype node_found;
    objecttype current_element;
    listtype rest_of_expression;
    objecttype matched_object;
    objecttype attribute_object;
    typetype object_type;
    typetype result_type;
    errinfotype err_info = OKAY_NO_ERROR;

  /* match_subexpr */
#ifdef TRACE_MATCH
    printf("BEGIN match_subexpr\n");
#endif
    if (trace.match) {
      printf("\nbegin sub match ");
      prot_list(match_expr);
      printf(" in sy{ ");
      printnodes(start_node->symbol);
      printf("} attr{ ");
      printnodes(start_node->attr);
      printf("} inout_param{ ");
      printnodes(start_node->inout_param);
      printf("} other_param{ ");
      printnodes(start_node->other_param);
      printf("} does ");
      fflush(stdout);
    } /* if */
    if (match_expr == NULL) {
      if (start_node->entity == NULL) {
        if (trace.match) {
          printf("//ST0//");
          fflush(stdout);
        } /* if */
        matched_object = NULL;
      } else {
        current_element = start_node->entity->owner->obj;
        if (trace.match) {
          printf("//ST2//");
          printclass(CLASS_OF_OBJ(current_element));
          fflush(stdout);
        } /* if */
        incl_list(&expr_object->value.listvalue, current_element, &err_info);
        if (current_element->type_of != NULL &&
            current_element->type_of->result_type != NULL) {
          SET_CLASS_OF_OBJ(expr_object, MATCHOBJECT);
        } else {
          SET_CLASS_OF_OBJ(expr_object, CALLOBJECT);
        } /* if */
        expr_object->type_of = current_element->type_of;
        matched_object = expr_object;
      } /* if */
    } else {
      current_element = match_expr->obj;
      rest_of_expression = match_expr->next;
      matched_object = NULL;
      if (HAS_DESCRIPTOR_ENTITY(current_element) &&
          current_element->descriptor.entity->syobject != NULL) {
        /* Symbol */
        if (trace.match) {
          printf("//SY0//");
          trace1(current_element);
          printf("=");
          printf("%ld", (inttype) current_element);
          printf("/");
          printf("%ld", (inttype) current_element->descriptor.entity->syobject);
          fflush(stdout);
        } /* if */
        node_found = find_node(start_node->symbol, current_element->descriptor.entity->syobject);
        if (node_found != NULL) {
          matched_object = match_subexpr(expr_object, node_found,
              rest_of_expression, check_access_right, look_for_interfaces);
          if (matched_object != NULL) {
            if (trace.match) {
              printf("//SYOBJECT// ");
              trace1(current_element);
              printf("\n");
            } /* if */
            match_expr->obj = current_element->descriptor.entity->syobject;
          } /* if */
        } /* if */
      } /* if */
      if (matched_object == NULL) {
        if ((CLASS_OF_OBJ(current_element) == VALUEPARAMOBJECT ||
            CLASS_OF_OBJ(current_element) == REFPARAMOBJECT ||
            CLASS_OF_OBJ(current_element) == RESULTOBJECT ||
            CLASS_OF_OBJ(current_element) == LOCALVOBJECT) &&
            current_element->value.objvalue != NULL) {
          attribute_object = current_element->value.objvalue;
        } else {
          attribute_object = current_element;
        } /* if */
        if (CLASS_OF_OBJ(attribute_object) == TYPEOBJECT) {
          object_type = take_type(attribute_object);
          /* Attribute */
          matched_object = match_subexpr_attr(expr_object, start_node,
              object_type, rest_of_expression, check_access_right,
              look_for_interfaces);
        } /* if */
      } /* if */
      if (matched_object == NULL) {
        if (trace.match) {
          printf("//OT1//");
          trace1(current_element);
          printf("=");
          printf("%ld", (inttype) current_element);
          fflush(stdout);
        } /* if */
        object_type = current_element->type_of;
        if (object_type != NULL) {
          if (trace.match) {
            printf("//T1//");
            trace1(object_type->match_obj);
            printf("=");
            printf("%ld", (inttype) object_type);
            fflush(stdout);
          } /* if */
          matched_object = match_subexpr_type(expr_object, start_node,
              object_type, (booltype) VAR_OBJECT(current_element),
              rest_of_expression, check_access_right, look_for_interfaces);
#ifdef OUT_OF_ORDER
          if (matched_object != NULL) {
            if (CLASS_OF_OBJ(current_element) == DECLAREDOBJECT) {
              matched_object = NULL;
            } /* if */
          } else {
#endif
          if (matched_object == NULL) {
            if (CLASS_OF_OBJ(match_expr->obj) == EXPROBJECT) {
              if ((match_expression(match_expr->obj)) != NULL) {
                object_type = match_expr->obj->type_of;
                if (trace.match) {
                  printf("//T2//");
                  trace1(object_type->match_obj);
                  fflush(stdout);
                } /* if */
                if (object_type != NULL) {
                  matched_object = match_subexpr_type(expr_object, start_node,
                      object_type, (booltype) VAR_OBJECT(match_expr->obj),
                      rest_of_expression, check_access_right, look_for_interfaces);
                } /* if */
              } else {
                return(NULL);
              } /* if */
            } /* if */
            if (matched_object == NULL) {
              if (CLASS_OF_OBJ(current_element) == CALLOBJECT) {
                if (CLASS_OF_OBJ(current_element->value.listvalue->obj) == TYPEOBJECT) {
                  object_type = take_type(current_element->value.listvalue->obj);
                  /* Attribute */
                  matched_object = match_subexpr_attr(expr_object, start_node,
                      object_type, rest_of_expression, check_access_right,
                      look_for_interfaces);
                } /* if */
              } /* if */
            } /* if */
            if (matched_object == NULL) {
              object_type = current_element->type_of;
              do {
                if (trace.match) {
                  printf("//T3//");
                  trace1(object_type->match_obj);
                  fflush(stdout);
                } /* if */
                result_type = object_type->result_type;
                if (result_type != NULL) {
                  matched_object = match_subexpr_type(expr_object, start_node,
                      result_type, object_type->is_varfunc_type,
                      rest_of_expression, check_access_right, look_for_interfaces);
                } /* if */
                object_type = object_type->meta;
              } while (object_type != NULL && matched_object == NULL);
              if (matched_object != NULL) {
                if ((current_element = match_object2(current_element, expr_object)) != NULL) {
                  match_expr->obj = current_element;
                } else {
                  matched_object = NULL;
                } /* if */
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (trace.match) {
      printf("\nend sub match ");
      prot_list(match_expr);
      printf(" ==> ");
      trace1(matched_object);
      printf("\n");
      fflush(stdout);
    } /* if */
#ifdef TRACE_MATCH
    printf("END match_subexpr\n");
#endif
    return(matched_object);
  } /* match_subexpr */



#ifdef ANSI_C

objecttype match_expression (objecttype expr_object)
#else

objecttype match_expression (expr_object)
objecttype expr_object;
#endif

  {
    listtype expr_list;
    objecttype matched_object;

  /* match_expression */
#ifdef TRACE_MATCH
    printf("BEGIN match_expression\n");
#endif
    expr_list = expr_object->value.listvalue;
    if (trace.match) {
      printf("begin match ");
      prot_list(expr_list);
      fflush(stdout);
      printf(" does ");
      fflush(stdout);
    } /* if */
    if (expr_list != NULL) {
      matched_object = match_subexpr(expr_object, prog.declaration_root,
          expr_list, FALSE, TRUE);
    } else {
      matched_object = NULL;
    } /* if */
    if (matched_object == NULL) {
      if (expr_list != NULL) {
        matched_object = match_subexpr(expr_object, prog.declaration_root,
            expr_list, TRUE, TRUE);
      } /* if */
      if (matched_object != NULL) {
        check_access_rights(matched_object);
      } else {
        err_match(NO_MATCH, expr_object);
      } /* if */
      /* trace1(expr_object); */
    } /* if */
    if (trace.match) {
      printf("expr_object ");
      trace1(expr_object);
      printf("end match ");
      prot_list(expr_list);
      printf(" ==> ");
      trace1(matched_object);
      printf(" @\n");
      fflush(stdout);
    } /* if */
#ifdef TRACE_MATCH
    printf("END match_expression\n");
#endif
    return(matched_object);
  } /* match_expression */



#ifdef ANSI_C

objecttype match_prog_expression (nodetype start_node, objecttype expr_object)
#else

objecttype match_prog_expression (start_node, expr_object)
nodetype start_node;
objecttype expr_object;
#endif

  {
    listtype expr_list;
    objecttype matched_object;

  /* match_prog_expression */
#ifdef TRACE_MATCH
    printf("BEGIN match_expression\n");
#endif
    expr_list = expr_object->value.listvalue;
    if (trace.match) {
      printf("begin match ");
      prot_list(expr_list);
      fflush(stdout);
      printf(" does ");
      fflush(stdout);
    } /* if */
    if (expr_list != NULL) {
      matched_object = match_subexpr(expr_object, start_node,
          expr_list, FALSE, FALSE);
    } else {
      matched_object = NULL;
    } /* if */
    if (trace.match) {
      printf("expr_object ");
      trace1(expr_object);
      printf("end match ");
      prot_list(expr_list);
      printf(" ==> ");
      trace1(matched_object);
      printf(" @\n");
      fflush(stdout);
    } /* if */
#ifdef TRACE_MATCH
    printf("END match_expression\n");
#endif
    return(matched_object);
  } /* match_prog_expression */
