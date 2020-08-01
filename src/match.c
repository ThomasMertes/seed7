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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
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
#include "executl.h"
#include "memory.h"
#include "error.h"
#include "prclib.h"
#include "dcllib.h"

#undef EXTERN
#define EXTERN
#include "match.h"


#undef TRACE_MATCH



#ifdef ANSI_C
static objecttype match_subexpr (objecttype, const_nodetype, listtype,
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



#ifdef ANSI_C

static void check_access_rights (const_objecttype object)
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
        if (HAS_ENTITY(object->value.listvalue->obj)) {
          name_elem = GET_ENTITY(object->value.listvalue->obj)->name_list;
          expr_list = object->value.listvalue->next;
          while (name_elem != NULL && expr_list != NULL) {
            if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
              /* printf("param_obj/param ");
              trace1(name_elem->obj->value.objvalue);
              printf("\n"); */
              if (CATEGORY_OF_OBJ(name_elem->obj->value.objvalue) == REFPARAMOBJECT &&
                  VAR_OBJECT(name_elem->obj->value.objvalue)) {
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
#ifdef TRACE_MATCH
    printf("END check_access_rights\n");
#endif
  } /* check_access_rights */



#ifdef ANSI_C

void update_owner (const_objecttype expr_object)
#else

void update_owner (expr_object)
objecttype expr_object;
#endif

  {
    listtype expr_list;
    objecttype current_element;
    objecttype created_object;
    errinfotype err_info = OKAY_NO_ERROR;

  /* update_owner */
#ifdef TRACE_MATCH
    printf("BEGIN update_owner\n");
#endif
    expr_list = expr_object->value.listvalue;
    while (expr_list != NULL) {
      current_element = expr_list->obj;
      if (HAS_ENTITY(current_element) &&
          GET_ENTITY(current_element)->owner != NULL) {
        expr_list->obj = GET_ENTITY(current_element)->owner->obj;
        current_element = expr_list->obj;
      } /* if */
      if (CATEGORY_OF_OBJ(current_element) == EXPROBJECT) {
        update_owner(current_element);
      } else if (CATEGORY_OF_OBJ(current_element) == VALUEPARAMOBJECT ||
          CATEGORY_OF_OBJ(current_element) == REFPARAMOBJECT) {
        if (current_element->value.objvalue != NULL) {
#ifdef TRACE_MATCH_extended
          if (HAS_POSINFO(expr_object)) {
            printf("%s(%u): ",
                   file_name(GET_FILE_NUM(expr_object)),
                   GET_LINE_NUM(expr_object));
          }
          printf("Parameter already has value: ");
          prot_int((inttype) current_element);
          prot_cstri(" ");
          trace1(current_element);
          printf("\n");
#endif
          current_element = current_element->value.objvalue;
          if (ALLOC_OBJECT(created_object)) {
            created_object->type_of = current_element->type_of;
            created_object->descriptor.property = NULL;
            INIT_CATEGORY_OF_OBJ(created_object, DECLAREDOBJECT);
            created_object->value.objvalue = NULL;
            do_create(created_object, current_element, &err_info);
            if (err_info == CREATE_ERROR) {
              printf("*** do_create failed ");
              printf("\n");
            } /* if */
          } /* if */
          expr_list->obj = created_object;
#ifdef TRACE_MATCH_extended
          printf("Value is now: ");
          prot_int((inttype) expr_list->obj);
          prot_cstri(" ");
          trace1(expr_list->obj);
          printf("\n");
#endif
          if (CATEGORY_OF_OBJ(expr_list->obj) == VALUEPARAMOBJECT ||
              CATEGORY_OF_OBJ(expr_list->obj) == REFPARAMOBJECT) {
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
      case VALUEPARAMOBJECT:
      case REFPARAMOBJECT:
      case DECLAREDOBJECT:
      case FORWARDOBJECT:
        if (ALLOC_OBJECT(result)) {
          result->type_of = object->type_of;
          result->descriptor.property = property.literal;
          result->value.listvalue = NULL;
          INIT_CATEGORY_OF_OBJ(result, CALLOBJECT);
          incl_list(&result->value.listvalue, object, &err_info);
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

static objecttype match_object2 (objecttype object, const_objecttype expr_object)
#else

static objecttype match_object2 (object, expr_object)
objecttype object;
objecttype expr_object;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    objecttype result;

  /* match_object2 */
#ifdef TRACE_MATCH
    printf("BEGIN match_object2\n");
#endif
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
            INIT_CATEGORY_OF_POSINFO(result, CALLOBJECT);
          } else {
            result->descriptor.property = property.literal;
            INIT_CATEGORY_OF_OBJ(result, CALLOBJECT);
          } /* if */
          result->value.listvalue = NULL;
          incl_list(&result->value.listvalue, object, &err_info);
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
#ifdef TRACE_MATCH
    printf("END match_object2\n");
#endif
    return(result);
  } /* match_object2 */



#ifdef ANSI_C

static objecttype match_subexpr_var (objecttype expr_object,
    const_nodetype start_node, typetype object_type,
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
    return(matched_object);
  } /* match_subexpr_var */



#ifdef ANSI_C

static objecttype match_subexpr_const (objecttype expr_object,
    const_nodetype start_node, typetype object_type, listtype rest_of_expression,
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
    return(matched_object);
  } /* match_subexpr_const */



#ifdef ANSI_C

static objecttype match_subexpr_type (objecttype expr_object,
    const_nodetype start_node, typetype object_type, booltype is_variable_obj,
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
    typetype current_object_type;
    typelisttype interface_list;
    booltype non_dynamic_match_removed;
    objecttype matched_object;

  /* match_subexpr_type */
#ifdef TRACE_MATCH
    printf("BEGIN match_subexpr_type\n");
#endif
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
            matched_object->value.listvalue != NULL &&
            matched_object->value.listvalue->obj != NULL) {
          if (CATEGORY_OF_OBJ(matched_object->value.listvalue->obj) != ACTOBJECT ||
              matched_object->value.listvalue->obj->value.actvalue != prc_dynamic) {
            pop_list(&matched_object->value.listvalue);
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
            matched_object->value.listvalue != NULL &&
            matched_object->value.listvalue->obj != NULL) {
          if (CATEGORY_OF_OBJ(matched_object->value.listvalue->obj) != ACTOBJECT ||
              matched_object->value.listvalue->obj->value.actvalue != prc_dynamic) {
            pop_list(&matched_object->value.listvalue);
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
#ifdef TRACE_MATCH
    printf("END match_subexpr_type\n");
#endif
    return(matched_object);
  } /* match_subexpr_type */



#ifdef ANSI_C

static objecttype match_subexpr_attr (objecttype expr_object,
    const_nodetype start_node, const_typetype object_type,
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

static objecttype match_subexpr_param_attr (objecttype expr_object,
    const_nodetype start_node, const_typetype f_param_type,
    booltype is_inout_f_param, listtype rest_of_expression,
    booltype check_access_right, booltype look_for_interfaces)
#else

static objecttype match_subexpr_param_attr (expr_object, start_node,
    f_param_type, is_inout_f_param, rest_of_expression, check_access_right,
    look_for_interfaces)
objecttype expr_object;
nodetype start_node;
const_typetype f_param_type;
booltype is_inout_f_param;
listtype rest_of_expression;
booltype check_access_right;
booltype look_for_interfaces;
#endif

  {
    nodetype node_found;
    objecttype matched_object;

  /* match_subexpr_param_attr */
#ifdef TRACE_MATCH
    printf("BEGIN match_subexpr_param_attr\n");
#endif
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
#ifdef TRACE_MATCH
    printf("END match_subexpr_param_attr\n");
#endif
    return(matched_object);
  } /* match_subexpr_param_attr */



#ifdef ANSI_C

static objecttype match_subexpr (objecttype expr_object,
    const_nodetype start_node, listtype match_expr, booltype check_access_right,
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
          printcategory(CATEGORY_OF_OBJ(current_element));
          fflush(stdout);
        } /* if */
        incl_list(&expr_object->value.listvalue, current_element, &err_info);
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
          printf("%ld", (long) current_element);
          printf("/");
          printf("%ld", (long) GET_ENTITY(current_element)->syobject);
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
            current_element->value.objvalue != NULL) {
          attribute_object = current_element->value.objvalue;
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
          printf("%ld", (long) current_element);
          fflush(stdout);
        } /* if */
        object_type = current_element->type_of;
        if (object_type != NULL) {
          if (trace.match) {
            printf("//T1//");
            trace1(object_type->match_obj);
            printf("=");
            printf("%ld", (long) object_type);
            fflush(stdout);
          } /* if */
          matched_object = match_subexpr_type(expr_object, start_node,
              object_type, (booltype) VAR_OBJECT(current_element),
              rest_of_expression, check_access_right, look_for_interfaces);
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
                      object_type, (booltype) VAR_OBJECT(match_expr->obj),
                      rest_of_expression, check_access_right, look_for_interfaces);
                } /* if */
              } else {
                return(NULL);
              } /* if */
            } /* if */
            if (matched_object == NULL) {
              if (CATEGORY_OF_OBJ(current_element) == CALLOBJECT) {
                if (CATEGORY_OF_OBJ(current_element->value.listvalue->obj) == TYPEOBJECT) {
                  object_type = take_type(current_element->value.listvalue->obj);
                  /* Attribute */
                  matched_object = match_subexpr_attr(expr_object, start_node,
                      object_type, rest_of_expression, check_access_right,
                      look_for_interfaces);
                /* } else {
                  trace1(current_element->value.listvalue->obj);
                  printf("\n"); */
                } /* if */
              } else if (CATEGORY_OF_OBJ(current_element) == MATCHOBJECT) {
                if (CATEGORY_OF_OBJ(current_element->value.listvalue->obj) == ACTOBJECT) {
                  /* if (current_element->value.listvalue->obj->type_of->result_type == */
                  /* trace1(current_element->value.listvalue->obj);
                     printf("\n"); */
                  if (current_element->value.listvalue->obj->value.actvalue == dcl_val1 ||
                      current_element->value.listvalue->obj->value.actvalue == dcl_val2 ||
                      current_element->value.listvalue->obj->value.actvalue == dcl_ref1 ||
                      current_element->value.listvalue->obj->value.actvalue == dcl_ref2 ||
                      current_element->value.listvalue->obj->value.actvalue == dcl_attr) {
                    object_type = take_type(current_element->value.listvalue->next->next->obj);
                    /* trace1(current_element->value.listvalue->obj);
                       printf(" **** other\n");
                       trace1(current_element->value.listvalue->next->next->obj);
                       printf(" #### other\n"); */
                    matched_object = match_subexpr_param_attr(expr_object, start_node,
                        object_type, FALSE, rest_of_expression, check_access_right,
                        look_for_interfaces);
                    /* printf(" ++++ other\n"); */
                  } else if (current_element->value.listvalue->obj->value.actvalue == dcl_inout1 ||
                      current_element->value.listvalue->obj->value.actvalue == dcl_inout2) {
                    object_type = take_type(current_element->value.listvalue->next->next->obj);
                    /* trace1(current_element->value.listvalue->obj);
                       printf(" **** inout\n");
                       trace1(current_element->value.listvalue->next->next->obj);
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
    if (matched_object == NULL && !HAS_MATCH_ERR(expr_object)) {
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

objecttype match_prog_expression (const_nodetype start_node,
    objecttype expr_object)
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
