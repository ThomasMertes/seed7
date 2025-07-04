/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2008, 2010, 2013, 2015  Thomas Mertes      */
/*                2018, 2019, 2021, 2025  Thomas Mertes             */
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
/*  File: seed7/src/match.c                                         */
/*  Changes: 1995, 1999, 2000, 2007, 2008, 2010  Thomas Mertes      */
/*           2013, 2015, 2018, 2019, 2021, 2025  Thomas Mertes      */
/*  Content: Functions to do static match on expressions.           */
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
#include "flistutl.h"
#include "syvarutl.h"
#include "identutl.h"
#include "listutl.h"
#include "entutl.h"
#include "traceutl.h"
#include "executl.h"
#include "objutl.h"
#include "infile.h"
#include "name.h"
#include "error.h"
#include "prclib.h"
#include "dcllib.h"

#undef EXTERN
#define EXTERN
#include "match.h"

#define TRACE_SUBSTITUTE_PARAMS 0



static objectType match_subexpr (objectType, const_nodeType, listType,
     boolType, boolType);



#ifdef OUT_OF_ORDER
static int variable_obj (objectType object)

  {
    int result;

  /* variable_obj */
    logFunction(printf("variable_obj\n"););
    if (CATEGORY_OF_OBJ(object) == CALLOBJECT ||
        CATEGORY_OF_OBJ(object) == MATCHOBJECT) {
      result = (object->type_of != NULL &&
          object->type_of->is_varfunc_type);
    } else {
      result = VAR_OBJECT(object);
    } /* if */
    /* printf("variable_obj %d ", result);
    trace1(object);
    printf("\n"); */
    logFunction(printf("variable_obj -->\n"););
    return result;
  } /* variable_obj */



static void check_parameter (objectType expr_object, objectType actual_param)

  { /* check_parameter */
    if (CATEGORY_OF_OBJ(actual_param) == CALLOBJECT) {
      if (actual_param->type_of != NULL &&
          !actual_param->type_of->is_varfunc_type) {
        err_expr_obj(WRONGACCESSRIGHT, expr_object, actual_param);
      } /* if */
    } else {
      if (!VAR_OBJECT(actual_param)) {
        /* printf("category: %s\n", category_cstri(CATEGORY_OF_OBJ(actual_param))); */
        err_expr_obj(WRONGACCESSRIGHT, expr_object, actual_param);
      } /* if */
    } /* if */
  } /* check_parameter */
#endif



static void check_access_rights (const_objectType object)

  {
    listType name_elem;
    listType expr_list;

  /* check_access_rights */
    logFunction(printf("check_access_rights\n"););
/*    printf("check_access_rights ");
    trace1(object);
    printf("\n"); */
    if (object->value.listValue != NULL) {
      if (object->value.listValue->obj != NULL) {
/*        printf("call object ");
        trace1(object->value.listValue->obj);
        printf("\n"); */
        if (HAS_ENTITY(object->value.listValue->obj)) {
          name_elem = GET_ENTITY(object->value.listValue->obj)->fparam_list;
          expr_list = object->value.listValue->next;
          while (name_elem != NULL && expr_list != NULL) {
            if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
              /* printf("param_obj/param ");
              trace1(name_elem->obj->value.objValue);
              printf("\n"); */
              if (CATEGORY_OF_OBJ(name_elem->obj->value.objValue) == REFPARAMOBJECT &&
                  VAR_OBJECT(name_elem->obj->value.objValue)) {
                if (CATEGORY_OF_OBJ(expr_list->obj) == CALLOBJECT) {
                  if (expr_list->obj->type_of != NULL &&
                      !expr_list->obj->type_of->is_varfunc_type) {
                    err_expr_obj(WRONGACCESSRIGHT, object, expr_list->obj);
                  } /* if */
                } else {
                  if (!VAR_OBJECT(expr_list->obj)) {
                    /* printf("category: %s\n", category_cstri(CATEGORY_OF_OBJ(expr_list->obj))); */
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
    logFunction(printf("check_access_rights -->\n"););
  } /* check_access_rights */



/**
 *  In the expr_object all parameter objects are replaced by values.
 *  Parameters have the category VALUEPARAMOBJECT or REFPARAMOBJECT.
 *  The values of normal parameters are set with do_create().
 *  Do_create() is also used when a variable or parameter is initialized.
 *  If a parameter has an expression as value (category EXPROBJECT,
 *  CALLOBJECT, MATCHOBJECT or LISTOBJECT) the expression is copied
 *  to the place of the parameter instead. This copying is necessary,
 *  because for expressions do_create() would just creates a reference
 *  to the expression and this would leads to a double free of the
 *  expression.
 */
errInfoType substitute_params (const_objectType expr_object)

  {
    listType expr_list;
    objectType current_element;
    objectType created_object;
    listType list_elem;
    errInfoType err_info = OKAY_NO_ERROR;

  /* substitute_params */
    logFunction(printf("substitute_params(");
                trace1(expr_object);
                printf(")\n"););
    expr_list = expr_object->value.listValue;
    while (expr_list != NULL && err_info == OKAY_NO_ERROR) {
      current_element = expr_list->obj;
      if (CATEGORY_OF_OBJ(current_element) == EXPROBJECT) {
        substitute_params(current_element);
      } else if (CATEGORY_OF_OBJ(current_element) == VALUEPARAMOBJECT ||
                 CATEGORY_OF_OBJ(current_element) == REFPARAMOBJECT) {
        if (current_element->value.objValue != NULL) {
#if TRACE_SUBSTITUTE_PARAMS
          if (HAS_POSINFO(expr_object)) {
            prot_string(objectFileName(expr_object));
            prot_cstri("(");
            prot_int((intType) POSINFO_LINE_NUM(expr_object));
            prot_cstri(")");
          }
          printf("Parameter already has value: ");
          trace1(current_element);
          printf("\n");
#endif
          current_element = current_element->value.objValue;
          if (CATEGORY_OF_OBJ(current_element) == STRUCTELEMOBJECT &&
              current_element->value.objValue != NULL) {
            current_element = current_element->value.objValue;
            if (HAS_ENTITY(current_element) &&
                GET_ENTITY(current_element)->syobject != NULL) {
              created_object = GET_ENTITY(current_element)->syobject;
            } /* if */
          } else if (CATEGORY_OF_OBJ(current_element) == EXPROBJECT ||
              CATEGORY_OF_OBJ(current_element) == CALLOBJECT ||
              CATEGORY_OF_OBJ(current_element) == MATCHOBJECT ||
              CATEGORY_OF_OBJ(current_element) == LISTOBJECT) {
            created_object = copy_expression(current_element, &err_info);
          } else if (likely(ALLOC_L_ELEM(list_elem) &&
                            ALLOC_OBJECT(created_object))) {
            created_object->type_of = current_element->type_of;
            created_object->descriptor.property = NULL;
            INIT_CATEGORY_OF_OBJ(created_object, DECLAREDOBJECT);
            created_object->value.objValue = NULL;
            do_create(created_object, current_element, &err_info);
            if (unlikely(err_info != OKAY_NO_ERROR)) {
              logError(printf("substitute_params: do_create failed.\n"););
              FREE_L_ELEM(list_elem);
              FREE_OBJECT(created_object);
              created_object = NULL;
            } else {
              list_elem->obj = created_object;
              list_elem->next = prog->substituted_objects;
              prog->substituted_objects = list_elem;
            } /* if */
          } else {
            err_info = MEMORY_ERROR;
            if (list_elem != NULL) {
              FREE_L_ELEM(list_elem);
            } /* if */
            created_object = NULL;
          } /* if */
          if (created_object != NULL) {
            expr_list->obj = created_object;
#if TRACE_SUBSTITUTE_PARAMS
            printf("Value is now: ");
            trace1(expr_list->obj);
            printf("\n");
#endif
            if (CATEGORY_OF_OBJ(expr_list->obj) == VALUEPARAMOBJECT ||
                CATEGORY_OF_OBJ(expr_list->obj) == REFPARAMOBJECT) {
              if (expr_list->obj->value.objValue != NULL) {
                logError(printf("substitute_params: "
                                "Parameter again has value: ");
                         trace1(expr_list->obj);
                         printf("\n"););
                err_info = ACTION_ERROR;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } else if (HAS_ENTITY(current_element) &&
          GET_ENTITY(current_element)->data.owner != NULL) {
        expr_list->obj = GET_ENTITY(current_element)->data.owner->obj;
      } /* if */
      expr_list = expr_list->next;
    } /* while */
    logFunction(printf("substitute_params(");
                trace1(expr_object);
                printf(") --> %d\n", err_info););
    return err_info;
  } /* substitute_params */



void update_owner (const_objectType expr_object)

  {
    listType expr_list;
    objectType current_element;

  /* update_owner */
    logFunction(printf("update_owner\n"););
    expr_list = expr_object->value.listValue;
    while (expr_list != NULL) {
      current_element = expr_list->obj;
      if (HAS_ENTITY(current_element) &&
          GET_ENTITY(current_element)->data.owner != NULL) {
        expr_list->obj = GET_ENTITY(current_element)->data.owner->obj;
        current_element = expr_list->obj;
      } /* if */
      if (CATEGORY_OF_OBJ(current_element) == EXPROBJECT) {
        update_owner(current_element);
      } /* if */
      expr_list = expr_list->next;
    } /* while */
    logFunction(printf("update_owner -->\n"););
  } /* update_owner */



static objectType match_symbol (objectType object)

  {
    objectType obj_found;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* match_symbol */
    logFunction(printf("match_symbol(");
                trace1(object);
                printf(")\n"););
    if (!HAS_ENTITY(object) ||
        GET_ENTITY(object)->syobject == NULL) {
      err_match(EXPR_EXPECTED, object);
      result = NULL;
    } else {
      obj_found = find_name(prog->declaration_root, GET_ENTITY(object)->syobject, &err_info);
      if (obj_found == NULL) {
        err_match(EXPR_EXPECTED, object);
        result = NULL;
      } else {
        result = match_object(obj_found);
      } /* if */
    } /* if */
    return result;
  } /* match_symbol */



objectType match_object (objectType object)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* match_object */
    logFunction(printf("match_object(");
                trace1(object);
                printf(")\n"););
    if (trace.match) {
      printf("\nbegin match_object ");
      trace1(object);
      printf("\n");
    } /* if */
    switch (CATEGORY_OF_OBJ(object)) {
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
#if WITH_FLOAT
      case FLOATOBJECT:
#endif
      case WINOBJECT:
      case REFOBJECT:
      case REFLISTOBJECT:
      case EXPROBJECT:
      case ACTOBJECT:
      case ENUMLITERALOBJECT:
      case VARENUMOBJECT:
      case INTERFACEOBJECT:
      case VALUEPARAMOBJECT:
      case REFPARAMOBJECT:
      case DECLAREDOBJECT:
      case FORWARDOBJECT:
        if (ALLOC_OBJECT(result)) {
          result->type_of = object->type_of;
          result->descriptor.property = prog->property.literal;
          result->value.listValue = NULL;
          INIT_CATEGORY_OF_OBJ(result, CALLOBJECT);
          incl_list(&result->value.listValue, object, &err_info);
        } /* if */
        break;
      case MATCHOBJECT:
        SET_CATEGORY_OF_OBJ(object, CALLOBJECT);
        result = object;
        break;
      case CALLOBJECT:
        result = object;
        break;
      case SYMBOLOBJECT:
        result = match_symbol(object);
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
    logFunction(printf("match_object --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* match_object */



/**
 *  Assure that objects are preceded by a MATCHOBJECT.
 *  This function is used if a "match" has several parameters.
 */
static objectType match_object2 (objectType object, const_objectType expr_object)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* match_object2 */
    logFunction(printf("match_object2(");
                trace1(object);
                printf(")\n"););
    if (trace.match) {
      printf("\nbegin match_object2 ");
      trace1(object);
      printf("\n");
    } /* if */
    switch (CATEGORY_OF_OBJ(object)) {
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
#if WITH_FLOAT
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
      case LOCALVOBJECT:
      case DECLAREDOBJECT:
      case FORWARDOBJECT:
        if (ALLOC_OBJECT(result)) {
          result->type_of = object->type_of;
          if (HAS_POSINFO(expr_object)) {
            result->descriptor.posinfo = expr_object->descriptor.posinfo;
            INIT_CATEGORY_OF_POSINFO(result, CALLOBJECT);
          } else {
            result->descriptor.property = prog->property.literal;
            INIT_CATEGORY_OF_OBJ(result, CALLOBJECT);
          } /* if */
          result->value.listValue = NULL;
          incl_list(&result->value.listValue, object, &err_info);
        } /* if */
        break;
      case MATCHOBJECT:
        SET_CATEGORY_OF_OBJ(object, CALLOBJECT);
        result = object;
        break;
      case CALLOBJECT:
        result = object;
        break;
      default:
        printf("### match_object2 of ");
        trace1(object);
        printf("\n");
        result = NULL;
        break;
    } /* switch */
    if (trace.match) {
      printf("end match_object2 ");
      trace1(object);
      printf(" ==> ");
      trace1(result);
      printf("\n");
    } /* if */
    logFunction(printf("match_object2 --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* match_object2 */



/**
 *  Assure that BLOCKOBJECT and ACTOBJECT objects are preceded by a MATCHOBJECT.
 *  This function is used if a "match" has no parameters.
 */
static objectType match_object3 (objectType object, const_objectType expr_object)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result;

  /* match_object3 */
    logFunction(printf("match_object3(");
                trace1(object);
                printf(")\n"););
    if (trace.match) {
      printf("\nbegin match_object3 ");
      trace1(object);
      printf("\n");
    } /* if */
    switch (CATEGORY_OF_OBJ(object)) {
      case BLOCKOBJECT:
      case ACTOBJECT:
        if (ALLOC_OBJECT(result)) {
          result->type_of = object->type_of;
          if (HAS_POSINFO(expr_object)) {
            result->descriptor.posinfo = expr_object->descriptor.posinfo;
            INIT_CATEGORY_OF_POSINFO(result, MATCHOBJECT);
          } else {
            result->descriptor.property = prog->property.literal;
            INIT_CATEGORY_OF_OBJ(result, MATCHOBJECT);
          } /* if */
          result->value.listValue = NULL;
          incl_list(&result->value.listValue, object, &err_info);
        } /* if */
        break;
      default:
        result = object;
        break;
    } /* switch */
    if (trace.match) {
      printf("end match_object3 ");
      trace1(object);
      printf(" ==> ");
      trace1(result);
      printf("\n");
    } /* if */
    logFunction(printf("match_object3 --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* match_object3 */



static objectType match_subexpr_var (objectType expr_object,
    const_nodeType start_node, typeType object_type,
    listType rest_of_expression, boolType check_access_right,
    boolType look_for_interfaces)

  {
    nodeType node_found;
    objectType matched_object;

  /* match_subexpr_var */
    matched_object = NULL;
    if (trace.match) {
      printf("//ST1v//");
      trace1(object_type->match_obj);
      printf("=");
      printtype(object_type);
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
    return matched_object;
  } /* match_subexpr_var */



static objectType match_subexpr_const (objectType expr_object,
    const_nodeType start_node, typeType object_type, listType rest_of_expression,
    boolType check_access_right, boolType look_for_interfaces)

  {
    nodeType node_found;
    objectType matched_object;

  /* match_subexpr_const */
    matched_object = NULL;
    if (trace.match) {
      printf("//ST1o//");
      trace1(object_type->match_obj);
      printf("=");
      printtype(object_type);
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
    return matched_object;
  } /* match_subexpr_const */



static objectType match_subexpr_type (objectType expr_object,
    const_nodeType start_node, typeType object_type, boolType is_variable_obj,
    listType rest_of_expression, boolType check_access_right,
    boolType look_for_interfaces)

  {
    typeType current_object_type;
    typeListType interface_list;
    boolType non_dynamic_match_removed;
    objectType matched_object;

  /* match_subexpr_type */
    logFunction(printf("match_subexpr_type\n"););
    if (trace.match) {
      printf("//MST// ");
      printf("is_variable_obj=%s, ",     is_variable_obj     ? "TRUE" : "FALSE");
      printf("check_access_right=%s, ",  check_access_right  ? "TRUE" : "FALSE");
      printf("look_for_interfaces=%s, ", look_for_interfaces ? "TRUE" : "FALSE");
      fflush(stdout);
    } /* if */
    non_dynamic_match_removed = FALSE;
    matched_object = NULL;
    if (is_variable_obj) {
      if (look_for_interfaces) {
        interface_list = object_type->interfaces;
        while (interface_list != NULL && matched_object == NULL) {
          matched_object = match_subexpr_var(expr_object, start_node,
              interface_list->type_elem, rest_of_expression,
              check_access_right, look_for_interfaces);
          interface_list = interface_list->next;
        } /* while */
        if (matched_object != NULL &&
            (CATEGORY_OF_OBJ(matched_object) == MATCHOBJECT ||
            CATEGORY_OF_OBJ(matched_object) == CALLOBJECT) &&
            matched_object->value.listValue != NULL &&
            matched_object->value.listValue->obj != NULL) {
          if (CATEGORY_OF_OBJ(matched_object->value.listValue->obj) != ACTOBJECT ||
              matched_object->value.listValue->obj->value.actValue != prc_dynamic) {
            pop_list(&matched_object->value.listValue);
            SET_CATEGORY_OF_OBJ(expr_object, EXPROBJECT);
            non_dynamic_match_removed = TRUE;
            matched_object = NULL;
          } /* if */
        } /* if */
      } /* if */
      if (matched_object == NULL) {
        current_object_type = object_type;
        do {
          matched_object = match_subexpr_var(expr_object, start_node,
              current_object_type, rest_of_expression, check_access_right,
              look_for_interfaces);
          current_object_type = current_object_type->meta;
        } while (current_object_type != NULL && matched_object == NULL);
        if (matched_object == NULL && non_dynamic_match_removed) {
          interface_list = object_type->interfaces;
          while (interface_list != NULL && matched_object == NULL) {
            matched_object = match_subexpr_var(expr_object, start_node,
                interface_list->type_elem, rest_of_expression,
                check_access_right, look_for_interfaces);
            interface_list = interface_list->next;
          } /* while */
        } /* if */
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
            (CATEGORY_OF_OBJ(matched_object) == MATCHOBJECT ||
            CATEGORY_OF_OBJ(matched_object) == CALLOBJECT) &&
            matched_object->value.listValue != NULL &&
            matched_object->value.listValue->obj != NULL) {
          if (CATEGORY_OF_OBJ(matched_object->value.listValue->obj) != ACTOBJECT ||
              matched_object->value.listValue->obj->value.actValue != prc_dynamic) {
            pop_list(&matched_object->value.listValue);
            SET_CATEGORY_OF_OBJ(expr_object, EXPROBJECT);
            non_dynamic_match_removed = TRUE;
            matched_object = NULL;
          } /* if */
        } /* if */
      } /* if */
      if (matched_object == NULL) {
        current_object_type = object_type;
        do {
          matched_object = match_subexpr_const(expr_object, start_node,
              current_object_type, rest_of_expression,
              check_access_right, look_for_interfaces);
          current_object_type = current_object_type->meta;
        } while (current_object_type != NULL && matched_object == NULL);
        if (matched_object == NULL && non_dynamic_match_removed) {
          interface_list = object_type->interfaces;
          while (interface_list != NULL && matched_object == NULL) {
            matched_object = match_subexpr_const(expr_object, start_node,
                interface_list->type_elem, rest_of_expression,
                check_access_right, look_for_interfaces);
            interface_list = interface_list->next;
          } /* while */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("match_subexpr_type -->\n"););
    return matched_object;
  } /* match_subexpr_type */



static objectType match_subexpr_attr (objectType expr_object,
    const_nodeType start_node, const_typeType object_type,
    listType rest_of_expression, boolType check_access_right,
    boolType look_for_interfaces)

  {
    nodeType node_found;
    objectType matched_object;

  /* match_subexpr_attr */
    logFunction(printf("match_subexpr_attr\n"););
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
    logFunction(printf("match_subexpr_attr -->\n"););
    return matched_object;
  } /* match_subexpr_attr */



static objectType match_subexpr_param_attr (objectType expr_object,
    const_nodeType start_node, const_typeType f_param_type,
    boolType is_inout_f_param, listType rest_of_expression,
    boolType check_access_right, boolType look_for_interfaces)

  {
    nodeType node_found;
    objectType matched_object;

  /* match_subexpr_param_attr */
    logFunction(printf("match_subexpr_param_attr\n"););
    /* printf("inout_f_param_prototype = %lX\n", f_param_type->inout_f_param_prototype);
       trace1(f_param_type->inout_f_param_prototype);
       printf("\n");
       printf("other_f_param_prototype = %lX\n", f_param_type->other_f_param_prototype);
       trace1(f_param_type->other_f_param_prototype);
       printf("\n"); */
    matched_object = NULL;
    do {
      if (is_inout_f_param) {
        if (trace.match) {
          printf("//PA1//");
          trace1(f_param_type->inout_f_param_prototype);
          fflush(stdout);
        } /* if */
        node_found = find_node(start_node->attr, f_param_type->inout_f_param_prototype);
      } else {
        if (trace.match) {
          printf("//PA1//");
          trace1(f_param_type->other_f_param_prototype);
          fflush(stdout);
        } /* if */
        node_found = find_node(start_node->attr, f_param_type->other_f_param_prototype);
      } /* if */
      if (node_found != NULL) {
        matched_object = match_subexpr(expr_object, node_found,
            rest_of_expression, check_access_right,
            look_for_interfaces);
      } /* if */
      f_param_type = f_param_type->meta;
    } while (f_param_type != NULL && matched_object == NULL);
    logFunction(printf("match_subexpr_param_attr -->\n"););
    return matched_object;
  } /* match_subexpr_param_attr */



static objectType match_subexpr (objectType expr_object,
    const_nodeType start_node, listType match_expr, boolType check_access_right,
    boolType look_for_interfaces)

  {
    nodeType node_found;
    objectType current_element;
    listType rest_of_expression;
    objectType matched_object;
    objectType attribute_object;
    typeType object_type;
    typeType result_type;
    errInfoType err_info = OKAY_NO_ERROR;

  /* match_subexpr */
    logFunction(printf("match_subexpr\n"););
    if (trace.match) {
      printf("\nbegin sub match ");
      printf("check_access_right=%s, ",  check_access_right  ? "TRUE" : "FALSE");
      printf("look_for_interfaces=%s, ", look_for_interfaces ? "TRUE" : "FALSE");
      prot_list(match_expr);
      /*
      printf(" in sy{ ");
      printnodes(start_node->symbol);
      printf("} attr{ ");
      printnodes(start_node->attr);
      printf("} inout_param{ ");
      printnodes(start_node->inout_param);
      printf("} other_param{ ");
      printnodes(start_node->other_param);
      printf("} does ");
      */
      fflush(stdout);
    } /* if */
    if (match_expr == NULL) {
      if (start_node->entity == NULL ||
          start_node->entity->data.owner == NULL) {
        if (trace.match) {
          printf("//ST0//");
          fflush(stdout);
        } /* if */
        matched_object = NULL;
      } else {
        current_element = start_node->entity->data.owner->obj;
        if (trace.match) {
          printf("//ST2//");
          printcategory(CATEGORY_OF_OBJ(current_element));
          fflush(stdout);
        } /* if */
        incl_list(&expr_object->value.listValue, current_element, &err_info);
        if (current_element->type_of != NULL &&
            current_element->type_of->result_type != NULL) {
          SET_CATEGORY_OF_OBJ(expr_object, MATCHOBJECT);
        } else {
          SET_CATEGORY_OF_OBJ(expr_object, CALLOBJECT);
        } /* if */
        expr_object->type_of = current_element->type_of;
        matched_object = expr_object;
      } /* if */
    } else {
      current_element = match_expr->obj;
      rest_of_expression = match_expr->next;
      matched_object = NULL;
      if (HAS_ENTITY(current_element) &&
          GET_ENTITY(current_element)->syobject != NULL) {
        /* Symbol */
        if (trace.match) {
          printf("//SY0//");
          trace1(current_element);
          printf("=");
          printf(FMT_X_MEM, (memSizeType) current_element);
          printf("/");
          printf(FMT_X_MEM, (memSizeType) GET_ENTITY(current_element)->syobject);
          fflush(stdout);
        } /* if */
        node_found = find_node(start_node->symbol, GET_ENTITY(current_element)->syobject);
        if (node_found != NULL) {
          matched_object = match_subexpr(expr_object, node_found,
              rest_of_expression, check_access_right, look_for_interfaces);
          if (matched_object != NULL) {
            if (trace.match) {
              printf("//SYOBJECT// ");
              trace1(current_element);
              printf("\n");
            } /* if */
            match_expr->obj = GET_ENTITY(current_element)->syobject;
          } /* if */
        } /* if */
      } /* if */
      if (matched_object == NULL) {
        if ((CATEGORY_OF_OBJ(current_element) == VALUEPARAMOBJECT ||
            CATEGORY_OF_OBJ(current_element) == REFPARAMOBJECT ||
            CATEGORY_OF_OBJ(current_element) == RESULTOBJECT ||
            CATEGORY_OF_OBJ(current_element) == LOCALVOBJECT) &&
            current_element->value.objValue != NULL) {
          attribute_object = current_element->value.objValue;
        } else {
          attribute_object = current_element;
        } /* if */
        if (CATEGORY_OF_OBJ(attribute_object) == TYPEOBJECT) {
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
          printf(FMT_X_MEM, (memSizeType) current_element);
          fflush(stdout);
        } /* if */
        object_type = current_element->type_of;
        if (object_type != NULL) {
          if (trace.match) {
            printf("//T1//");
            trace1(object_type->match_obj);
            printf("=");
            printf(FMT_X_MEM, (memSizeType) object_type);
            fflush(stdout);
          } /* if */
          matched_object = match_subexpr_type(expr_object, start_node,
              object_type, (boolType) VAR_OBJECT(current_element),
              rest_of_expression, check_access_right, look_for_interfaces);
          if (matched_object != NULL) {
            current_element = match_object3(current_element, expr_object);
            match_expr->obj = current_element;
          } /* if */
#ifdef OUT_OF_ORDER
          if (matched_object != NULL) {
            if (CATEGORY_OF_OBJ(current_element) == DECLAREDOBJECT) {
              matched_object = NULL;
            } /* if */
          } else {
#endif
          if (matched_object == NULL) {
            if (CATEGORY_OF_OBJ(match_expr->obj) == EXPROBJECT) {
              if ((match_expression(match_expr->obj)) != NULL) {
                object_type = match_expr->obj->type_of;
                if (trace.match) {
                  printf("//T2//");
                  trace1(object_type->match_obj);
                  fflush(stdout);
                } /* if */
                if (object_type != NULL) {
                  matched_object = match_subexpr_type(expr_object, start_node,
                      object_type, (boolType) VAR_OBJECT(match_expr->obj),
                      rest_of_expression, check_access_right, look_for_interfaces);
                } /* if */
              } else {
                return NULL;
              } /* if */
            } /* if */
            if (matched_object == NULL) {
              if (CATEGORY_OF_OBJ(current_element) == CALLOBJECT) {
                if (CATEGORY_OF_OBJ(current_element->value.listValue->obj) == TYPEOBJECT) {
                  object_type = take_type(current_element->value.listValue->obj);
                  /* Attribute */
                  matched_object = match_subexpr_attr(expr_object, start_node,
                      object_type, rest_of_expression, check_access_right,
                      look_for_interfaces);
                /* } else {
                  trace1(current_element->value.listValue->obj);
                  printf("\n"); */
                } /* if */
              } else if (CATEGORY_OF_OBJ(current_element) == MATCHOBJECT) {
                if (CATEGORY_OF_OBJ(current_element->value.listValue->obj) == ACTOBJECT) {
                  /* if (current_element->value.listValue->obj->type_of->result_type == */
                  /* trace1(current_element->value.listValue->obj);
                     printf("\n"); */
                  if (current_element->value.listValue->obj->value.actValue == dcl_val1 ||
                      current_element->value.listValue->obj->value.actValue == dcl_val2 ||
                      current_element->value.listValue->obj->value.actValue == dcl_ref1 ||
                      current_element->value.listValue->obj->value.actValue == dcl_ref2 ||
                      current_element->value.listValue->obj->value.actValue == dcl_attr) {
                    object_type = take_type(current_element->value.listValue->next->next->obj);
                    /* trace1(current_element->value.listValue->obj);
                       printf(" **** other\n");
                       trace1(current_element->value.listValue->next->next->obj);
                       printf(" #### other\n"); */
                    matched_object = match_subexpr_param_attr(expr_object, start_node,
                        object_type, FALSE, rest_of_expression, check_access_right,
                        look_for_interfaces);
                    /* printf(" ++++ other\n"); */
                  } else if (current_element->value.listValue->obj->value.actValue == dcl_inout1 ||
                      current_element->value.listValue->obj->value.actValue == dcl_inout2) {
                    object_type = take_type(current_element->value.listValue->next->next->obj);
                    /* trace1(current_element->value.listValue->obj);
                       printf(" **** inout\n");
                       trace1(current_element->value.listValue->next->next->obj);
                       printf(" #### inout\n"); */
                    matched_object = match_subexpr_param_attr(expr_object, start_node,
                        object_type, TRUE, rest_of_expression, check_access_right,
                        look_for_interfaces);
                    /* printf(" ++++ inout\n"); */
                  } /* if */
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
    logFunction(printf("match_subexpr -->\n"););
    return matched_object;
  } /* match_subexpr */



objectType match_expression (objectType expr_object)

  {
    listType expr_list;
    objectType matched_object;

  /* match_expression */
    logFunction(printf("match_expression(");
                trace1(expr_object);
                printf(")\n"););
    expr_list = expr_object->value.listValue;
    if (trace.match) {
      printf("begin match ");
      prot_list(expr_list);
      fflush(stdout);
      printf(" does ");
      fflush(stdout);
    } /* if */
    if (expr_list != NULL) {
      matched_object = match_subexpr(expr_object, prog->declaration_root,
          expr_list, FALSE, TRUE);
    } else {
      matched_object = NULL;
    } /* if */
    if (matched_object == NULL && !HAS_MATCH_ERR(expr_object)) {
      if (expr_list != NULL) {
        matched_object = match_subexpr(expr_object, prog->declaration_root,
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
    logFunction(printf("match_expression --> ");
                trace1(matched_object);
                printf("\n"););
    return matched_object;
  } /* match_expression */



objectType match_prog_expression (const_nodeType start_node,
    objectType expr_object)

  {
    listType expr_list;
    objectType matched_object;

  /* match_prog_expression */
    logFunction(printf("match_prog_expression(");
                trace1(expr_object);
                printf(")\n"););
    expr_list = expr_object->value.listValue;
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
    logFunction(printf("match_prog_expression --> ");
                trace1(matched_object);
                printf("\n"););
    return matched_object;
  } /* match_prog_expression */
