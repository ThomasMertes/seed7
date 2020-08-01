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
/*  File: seed7/src/name.c                                          */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Enter an object in a specified declaration level.      */
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
#include "identutl.h"
#include "objutl.h"
#include "entutl.h"
#include "typeutl.h"
#include "listutl.h"
#include "traceutl.h"
#include "executl.h"
#include "exec.h"
#include "dcllib.h"
#include "findid.h"
#include "match.h"
#include "error.h"

#undef EXTERN
#define EXTERN
#include "name.h"

#undef TRACE_NAME


static int data_depth = 0;
static int depth = 0;



static void push_owner (ownerType *owner, objectType obj_to_push,
    errInfoType *err_info)

  {
    ownerType created_owner;

  /* push_owner */
#ifdef TRACE_NAME
    printf("BEGIN push_owner ");
    printf("%lu ", (unsigned long) obj_to_push);
    trace1(obj_to_push);
    printf("\n");
#endif
    if (ALLOC_RECORD(created_owner, ownerRecord, count.owner)) {
      created_owner->obj = obj_to_push;
      created_owner->decl_level = prog.stack_current;
      created_owner->next = *owner;
      *owner = created_owner;
    } else {
      *err_info = MEMORY_ERROR;
    } /* if */
#ifdef TRACE_NAME
    printf("END push_owner ");
    printf("%lu ", (unsigned long) obj_to_push);
    trace1(obj_to_push);
    printf("\n");
#endif
  } /* push_owner */



static void free_params (listType params)

  {
    listType param_elem;
    objectType param;

  /* free_params */
#ifdef TRACE_NAME
    printf("BEGIN free_params\n");
#endif
    param_elem = params;
    while (param_elem != NULL) {
      param = param_elem->obj;
      if (CATEGORY_OF_OBJ(param) == VALUEPARAMOBJECT ||
          CATEGORY_OF_OBJ(param) == REFPARAMOBJECT) {
        /* printf("free_params %lx: ", (unsigned long int) param);
        trace1(param);
        printf("\n"); */
        if (HAS_PROPERTY(param) && param->descriptor.property != prog.property.literal) {
          FREE_RECORD(param->descriptor.property, propertyRecord, count.property);
        } /* if */
        FREE_OBJECT(param);
      } /* if */
      param_elem = param_elem->next;
    } /* while */
    free_list(params);
#ifdef TRACE_NAME
    printf("END free_params\n");
#endif
  } /* free_params */



static objectType get_object (entityType entity, listType params,
    fileNumType file_number, lineNumType line, errInfoType *err_info)

  {
    objectType defined_object;
    objectType forward_reference;
    propertyType defined_property;

  /* get_object */
#ifdef TRACE_NAME
    printf("BEGIN get_object\n");
#endif
    if (entity->data.owner != NULL &&
        entity->data.owner->decl_level == prog.stack_current) {
      defined_object = entity->data.owner->obj;
      if (CATEGORY_OF_OBJ(defined_object) != FORWARDOBJECT) {
        err_object(OBJTWICEDECLARED, entity->data.owner->obj);
        SET_CATEGORY_OF_OBJ(defined_object, DECLAREDOBJECT);
      } else {
        SET_CATEGORY_OF_OBJ(defined_object, DECLAREDOBJECT);
        /* The old parameter names could be checked against the new ones. */
        free_params(defined_object->descriptor.property->params);
        defined_object->descriptor.property->params = params;
        defined_object->descriptor.property->file_number = file_number;
        defined_object->descriptor.property->line = line;
        /* defined_object->descriptor.property->syNumberInLine = symbol.syNumberInLine; */
        if (ALLOC_OBJECT(forward_reference)) {
          forward_reference->type_of = NULL;
          forward_reference->descriptor.property = NULL;
          INIT_CATEGORY_OF_OBJ(forward_reference, FWDREFOBJECT);
          forward_reference->value.objValue = defined_object;
          replace_list_elem(prog.stack_current->local_object_list,
              defined_object, forward_reference);
          prog.stack_current->object_list_insert_place = append_element_to_list(
              prog.stack_current->object_list_insert_place, defined_object, err_info);
        } else {
          *err_info = MEMORY_ERROR;
        } /* if */
      } /* if */
    } else {
      if (ALLOC_OBJECT(defined_object)) {
        if (ALLOC_RECORD(defined_property, propertyRecord, count.property)) {
          defined_property->entity = entity;
          defined_property->params = params;
          defined_property->file_number = file_number;
          defined_property->line = line;
          /* defined_property->syNumberInLine = symbol.syNumberInLine; */
          defined_object->type_of = NULL;
          defined_object->descriptor.property = defined_property;
          INIT_CATEGORY_OF_OBJ(defined_object, DECLAREDOBJECT);
          defined_object->value.objValue = NULL;
          push_owner(&entity->data.owner, defined_object, err_info);
          prog.stack_current->object_list_insert_place = append_element_to_list(
              prog.stack_current->object_list_insert_place, defined_object, err_info);
        } else {
          FREE_OBJECT(defined_object);
          *err_info = MEMORY_ERROR;
        } /* if */
      } else {
        *err_info = MEMORY_ERROR;
      } /* if */
    } /* if */
#ifdef TRACE_NAME
    printf("END get_object ");
    printf("%lu ", (unsigned long) defined_object);
    trace1(defined_object);
    printf("\n");
#endif
    return defined_object;
  } /* get_object */



listType create_parameter_list (listType name_list, errInfoType *err_info)

  {
    listType name_elem;
    objectType param_obj;
    listType parameter_list;
    listType *list_insert_place;

  /* create_parameter_list */
#ifdef TRACE_NAME
    printf("BEGIN create_parameter_list(");
    prot_list(name_list);
    printf(")\n");
#endif
    name_elem = name_list;
    parameter_list = NULL;
    list_insert_place = &parameter_list;
    while (name_elem != NULL) {
      if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
/* printf("create paramobject ");
trace1(name_elem->obj);
printf(" %lu\n", (long unsigned) name_elem->obj); */
        param_obj = name_elem->obj->value.objValue;
        if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
            CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT) {
/* printf("create in or ref param ");
trace1(param_obj);
printf(" %lu\n", (long unsigned) param_obj); */
          list_insert_place = append_element_to_list(list_insert_place,
              param_obj, err_info);
        } else if (CATEGORY_OF_OBJ(param_obj) == TYPEOBJECT) {
/* printf("create attr param ");
trace1(param_obj);
printf(" %lu\n", (long unsigned) param_obj); */
          list_insert_place = append_element_to_list(list_insert_place,
              param_obj, err_info);
        } else {
/* printf("create symbol param ");
trace1(param_obj);
printf("\n"); */
          list_insert_place = append_element_to_list(list_insert_place,
              param_obj, err_info);
        } /* if */
      } else {
/* printf("create symbol param ");
trace1(name_elem->obj);
printf(" %lu\n", (long unsigned) name_elem->obj); */
        list_insert_place = append_element_to_list(list_insert_place,
            name_elem->obj, err_info);
      } /* if */
      name_elem = name_elem->next;
    } /* while */
#ifdef TRACE_NAME
    printf("END create_parameter_list\n");
#endif
    return parameter_list;
  } /* create_parameter_list */



static void free_name_list (listType name_list, boolType freeParamObject)

  {
    listType name_elem;
    objectType param_obj;

  /* free_name_list */
#ifdef TRACE_NAME
    printf("BEGIN free_name_list\n");
#endif
    name_elem = name_list;
    while (name_elem != NULL) {
      if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
        /* printf("free_name_list: ");
        trace1(name_elem->obj);
        printf("\n"); */
        param_obj = name_elem->obj->value.objValue;
        if (freeParamObject && !HAS_PROPERTY(param_obj)) {
          if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
              CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT) {
            /* printf("formparam: ");
            trace1(name_elem->obj);
            printf("\n");
            printf("free param_obj %lx %d: ", (unsigned long int) param_obj, HAS_PROPERTY(param_obj));
            trace1(param_obj);
            printf("\n"); */
            /* if (HAS_PROPERTY(param_obj) && param_obj->descriptor.property != prog.property.literal) {
              * free_params(param_obj->descriptor.property->params); *
              FREE_RECORD(param_obj->descriptor.property, propertyRecord, count.property);
            } * if */
            FREE_OBJECT(param_obj);
          } /* if */
        } /* if */
        FREE_OBJECT(name_elem->obj);
      } /* if */
      name_elem = name_elem->next;
    } /* while */
    free_list(name_list);
#ifdef TRACE_NAME
    printf("END free_name_list\n");
#endif
  } /* free_name_list */



static objectType push_name (nodeType declaration_base,
    listType name_list, fileNumType file_number, lineNumType line,
    errInfoType *err_info)

  {
    entityType entity;
    listType params;
    objectType defined_object;

  /* push_name */
#ifdef TRACE_NAME
    printf("BEGIN push_name(%ld, ", (long) declaration_base);
    prot_list(name_list);
    printf(")\n");
#endif
    params = create_parameter_list(name_list, err_info);
    entity = get_entity(declaration_base, name_list);
    if (entity == NULL) {
      *err_info = MEMORY_ERROR;
      defined_object = NULL;
    } else {
      defined_object = get_object(entity, params,
          file_number, line, err_info);
      if (entity->fparam_list != name_list) {
        /* An existing entity is used */
        free_name_list(name_list, FALSE);
      } /* if */
    } /* if */
#ifdef TRACE_NAME
    printf("END push_name(");
    printf("%lu ", (unsigned long) defined_object);
    /* prot_list(GET_ENTITY(defined_object)->params); */
    printf(")\n");
#endif
    return defined_object;
  } /* push_name */



static void pop_object (progType currentProg, const_objectType obj_to_pop)

  {
    entityType entity;
    ownerType owner;

  /* pop_object */
#ifdef TRACE_NAME
    printf("BEGIN pop_object(");
    trace1(obj_to_pop);
    printf(")\n");
    fflush(stdout);
#endif
    if (HAS_ENTITY(obj_to_pop)) {
      entity = GET_ENTITY(obj_to_pop);
      owner = entity->data.owner;
      if (owner != NULL) {
        entity->data.owner = owner->next;
        FREE_RECORD(owner, ownerRecord, count.owner);
        if (entity->data.owner == NULL && entity->fparam_list != NULL) {
          pop_entity(currentProg->declaration_root, entity);
          entity->data.next = currentProg->entity.inactive_list;
          currentProg->entity.inactive_list = entity;
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_NAME
    printf("END pop_object\n");
#endif
  } /* pop_object */



static void disconnect_entity (const objectType anObject)

  {
    entityType entity;
    stackType decl_lev;
    listType *lstptr;
    listType lst;
    listType old_elem;

  /* disconnect_entity */
#ifdef TRACE_NAME
    printf("BEGIN disconnect_entity\n");
#endif
    entity = GET_ENTITY(anObject);
    if (entity->data.owner != NULL && entity->data.owner->obj == anObject) {
      /* printf("disconnect_entity ");
         trace1(anObject);
         printf("\n"); */
      decl_lev = entity->data.owner->decl_level;
      lstptr = &decl_lev->local_object_list;
      lst = decl_lev->local_object_list;
      while (lst != NULL) {
        if (lst->obj == anObject) {
          if (decl_lev->object_list_insert_place == &lst->next) {
            decl_lev->object_list_insert_place = lstptr;
          } /* if */
          old_elem = lst;
          *lstptr = lst->next;
          lst = lst->next;
          FREE_L_ELEM(old_elem);
        } else {
          lstptr = &lst->next;
          lst = lst->next;
        } /* if */
      } /* while */
      pop_object(&prog, anObject);
      FREE_RECORD(anObject->descriptor.property, propertyRecord, count.property);
      anObject->descriptor.property = NULL;
    } /* if */
#ifdef TRACE_NAME
    printf("END disconnect_entity\n");
#endif
  } /* disconnect_entity */



void disconnect_param_entities (const const_objectType objWithParams)

  {
    listType param_elem;
    objectType param_obj;

  /* disconnect_param_entities */
#ifdef TRACE_NAME
    printf("BEGIN disconnect_param_entities(");
    trace1(objWithParams);
    printf(")\n");
#endif
    if (FALSE && HAS_PROPERTY(objWithParams)) {
      param_elem = objWithParams->descriptor.property->params;
      while (param_elem != NULL) {
        param_obj = param_elem->obj;
        if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
            CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT) {
          if (HAS_ENTITY(param_obj)) {
            disconnect_entity(param_obj);
          } /* if */
        } /* if */
        param_elem = param_elem->next;
      } /* while */
    } /* if */
#ifdef TRACE_NAME
    printf("END disconnect_param_entities\n");
#endif
  } /* disconnect_param_entities */



void init_stack (progType currentProg, errInfoType *err_info)

  {
    stackType created_stack_element;

  /* init_stack */
#ifdef TRACE_NAME
    printf("BEGIN init_stack\n");
#endif
    if (ALLOC_RECORD(created_stack_element, stackRecord, count.stack)) {
      created_stack_element->upward = NULL;
      created_stack_element->downward = NULL;
      created_stack_element->local_object_list = NULL;
      created_stack_element->object_list_insert_place =
          &created_stack_element->local_object_list;
      currentProg->stack_global = created_stack_element;
      currentProg->stack_data = created_stack_element;
      currentProg->stack_current = created_stack_element;
      data_depth = 1;
      depth = 1;
    } else {
      *err_info = MEMORY_ERROR;
    } /* if */
#ifdef TRACE_NAME
    printf("END init_stack\n");
#endif
  } /* init_stack */



void close_stack (progType currentProg)

  {
    listType reversed_list = NULL;
    listType next_elem;
    listType list_element;

  /* close_stack */
#ifdef TRACE_NAME
    printf("BEGIN close_stack %d\n", data_depth);
#endif
    /* The list of objects is reversed to free the objects in    */
    /* the opposite way of their definition.                     */
    list_element = currentProg->stack_data->local_object_list;
    if (list_element != NULL) {
      reversed_list = list_element;
      list_element = list_element->next;
      reversed_list->next = NULL;
      while (list_element != NULL) {
        next_elem = list_element->next;
        list_element->next = reversed_list;
        reversed_list = list_element;
        list_element = next_elem;
      } /* while */
    } /* if */
    list_element = reversed_list;
    while (list_element != NULL) {
      /* printf("%lx ", (unsigned long int) list_element->obj);
      trace1(list_element->obj);
      printf("\n"); */
      dump_temp_value(list_element->obj);
      /* memset(&list_element->obj->value, 0, sizeof(valueUnion)); */
      pop_object(currentProg, list_element->obj);
      list_element = list_element->next;
    } /* while */
    list_element = reversed_list;
    /* Freeing objects in an extra loop avoids accessing freed   */
    /* object data. In case of forward declared objects the      */
    /* category of a freed object would be accessed.             */
    while (list_element != NULL) {
      if (HAS_PROPERTY(list_element->obj) && list_element->obj->descriptor.property != currentProg->property.literal) {
        /* Properties are removed here because itf_destr uses    */
        /* !HAS_PROPERTY to determine if an object can be freed. */
        free_params(list_element->obj->descriptor.property->params);
        FREE_RECORD(list_element->obj->descriptor.property, propertyRecord, count.property);
        /* list_element->obj->descriptor.property = NULL; */
      } /* if */
      FREE_OBJECT(list_element->obj);
      list_element = list_element->next;
    } /* while */
    free_list(reversed_list);
    currentProg->stack_current->local_object_list = NULL;
#ifdef TRACE_NAME
    printf("END close_stack %d\n", data_depth);
#endif
  } /* close_stack */



void grow_stack (errInfoType *err_info)

  {
    stackType created_stack_element;

  /* grow_stack */
#ifdef TRACE_NAME
    printf("BEGIN grow_stack %d\n", data_depth);
#endif
    if (ALLOC_RECORD(created_stack_element, stackRecord, count.stack)) {
      /* printf("%lx grow_stack %d\n", created_stack_element, data_depth + 1); */
      created_stack_element->upward = NULL;
      created_stack_element->downward = prog.stack_data;
      created_stack_element->local_object_list = NULL;
      created_stack_element->object_list_insert_place =
          &created_stack_element->local_object_list;
      prog.stack_data->upward = created_stack_element;
      prog.stack_data = created_stack_element;
      data_depth++;
    } else {
      *err_info = MEMORY_ERROR;
    } /* if */
#ifdef TRACE_NAME
    printf("END grow_stack %d\n", data_depth);
#endif
  } /* grow_stack */



void shrink_stack (void)

  {
    listType list_element;
    stackType old_stack_element;

  /* shrink_stack */
#ifdef TRACE_NAME
    printf("BEGIN shrink_stack %d\n", data_depth);
#endif
    /* printf("%lx shrink_stack %d\n", prog.stack_data, data_depth); */
    list_element = prog.stack_data->local_object_list;
    while (list_element != NULL) {
      pop_object(&prog, list_element->obj);
      list_element = list_element->next;
    } /* while */
    free_list(prog.stack_data->local_object_list);
    old_stack_element = prog.stack_data;
    prog.stack_data = prog.stack_data->downward;
    prog.stack_data->upward = NULL;
    FREE_RECORD(old_stack_element, stackRecord, count.stack);
    data_depth--;
#ifdef TRACE_NAME
    printf("END shrink_stack %d\n", data_depth);
#endif
  } /* shrink_stack */



void push_stack (void)

  { /* push_stack */
#ifdef TRACE_NAME
    printf("BEGIN push_stack %d\n", depth);
#endif
    if (prog.stack_current->upward != NULL) {
      /* printf("%lx push_stack %d\n", prog.stack_current->upward, depth + 1); */
      prog.stack_current = prog.stack_current->upward;
      depth++;
    } else {
      /* printf("cannot go up\n"); */
    } /* if */
#ifdef TRACE_NAME
    printf("END push_stack %d\n", depth);
#endif
  } /* push_stack */



void pop_stack (void)

  {
    listType list_element;

  /* pop_stack */
#ifdef TRACE_NAME
    printf("BEGIN pop_stack %d\n", depth);
#endif
    if (prog.stack_current->downward != NULL) {
      /* printf("%lx pop_stack %d\n", prog.stack_current, depth); */
      list_element = prog.stack_current->local_object_list;
      while (list_element != NULL) {
        pop_object(&prog, list_element->obj);
        list_element = list_element->next;
      } /* while */
      free_list(prog.stack_current->local_object_list);
      prog.stack_current->local_object_list = NULL;
      prog.stack_current->object_list_insert_place =
          &prog.stack_current->local_object_list;
      prog.stack_current = prog.stack_current->downward;
      depth--;
    } else {
      /* printf("cannot go down\n"); */
    } /* if */
#ifdef TRACE_NAME
    printf("END pop_stack %d\n", depth);
#endif
  } /* pop_stack */



static void down_stack (void)

  { /* down_stack */
#ifdef TRACE_NAME
    printf("BEGIN down_stack %d\n", depth);
#endif
    if (prog.stack_current->downward != NULL) {
      /* printf("%lx down_stack %d\n", prog.stack_current, depth); */
      prog.stack_current = prog.stack_current->downward;
      depth--;
    } else {
      printf("cannot go down");
    } /* if */
#ifdef TRACE_NAME
    printf("END down_stack %d\n", depth);
#endif
  } /* down_stack */



listType *get_local_object_insert_place (void)

  { /* get_local_object_insert_place */
    return prog.stack_current->object_list_insert_place;
  } /* get_local_object_insert_place */



static void match_name_list (listType raw_name_list)

  {
    listType name_elem;

  /* match_name_list */
#ifdef TRACE_NAME
    printf("BEGIN match_name_list\n");
#endif
    name_elem = raw_name_list;
    while (name_elem != NULL) {
      if (CATEGORY_OF_OBJ(name_elem->obj) == EXPROBJECT) {
        if (match_expression(name_elem->obj) == NULL) {
          err_match(NO_MATCH, name_elem->obj);
        } /* if */
      } /* if */
      name_elem = name_elem->next;
    } /* while */
#ifdef TRACE_NAME
    printf("END match_name_list\n");
#endif
  } /* match_name_list */



static listType eval_name_list (listType raw_name_list, errInfoType *err_info)

  {
    listType name_elem;
    listType name_list;
    listType *list_insert_place;
    objectType parameter;

  /* eval_name_list */
#ifdef TRACE_NAME
    printf("BEGIN eval_name_list\n");
#endif
    name_elem = raw_name_list;
    name_list = NULL;
    list_insert_place = &name_list;
    while (name_elem != NULL) {
      if (CATEGORY_OF_OBJ(name_elem->obj) == MATCHOBJECT) {
        if ((parameter = evaluate(name_elem->obj)) != NULL) {
          list_insert_place = append_element_to_list(list_insert_place,
              parameter, err_info);
        } else {
          err_object(PARAM_DECL_FAILED, name_elem->obj);
          list_insert_place = append_element_to_list(list_insert_place,
              name_elem->obj, err_info);
        } /* if */
      } else {
        if (HAS_ENTITY(name_elem->obj) &&
            GET_ENTITY(name_elem->obj)->syobject != NULL) {
          parameter = GET_ENTITY(name_elem->obj)->syobject;
        } else {
          parameter = name_elem->obj;
        } /* if */
        list_insert_place = append_element_to_list(list_insert_place,
            parameter, err_info);
      } /* if */
      name_elem = name_elem->next;
    } /* while */
#ifdef TRACE_NAME
    printf("END eval_name_list\n");
#endif
    return name_list;
  } /* eval_name_list */



static objectType inst_list (nodeType declaration_base, const_objectType object_name,
    errInfoType *err_info)

  {
    listType name_list;
    objectType defined_object;

  /* inst_list */
#ifdef TRACE_NAME
    printf("BEGIN inst_list(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    match_name_list(object_name->value.listValue);
    push_stack();
    name_list = eval_name_list(object_name->value.listValue, err_info);
    down_stack();
    defined_object = push_name(declaration_base, name_list,
        GET_FILE_NUM(object_name), GET_LINE_NUM(object_name), err_info);
#ifdef TRACE_NAME
    printf("END inst_list --> ");
    trace1(defined_object);
    printf("\n");
#endif
    return defined_object;
  } /* inst_list */



static objectType inst_object (const_nodeType declaration_base, objectType name_object,
    fileNumType file_number, lineNumType line, errInfoType *err_info)

  {
    objectType defined_object;

  /* inst_object */
#ifdef TRACE_NAME
    printf("BEGIN inst_object(");
    trace1(name_object);
    printf(")\n");
#endif
    if (name_object->descriptor.property == prog.property.literal) {
      err_object(IDENT_EXPECTED, name_object);
    } /* if */
    defined_object = get_object(GET_ENTITY(name_object), NULL,
        file_number, line, err_info);
#ifdef TRACE_NAME
    printf("END inst_object --> ");
    trace1(defined_object);
    printf("\n");
#endif
    return defined_object;
  } /* inst_object */



static objectType inst_object_expr (const_nodeType declaration_base,
    objectType object_name, errInfoType *err_info)

  {
    listType name_list;
    objectType param_obj;
    objectType defined_object;

  /* inst_object_expr */
#ifdef TRACE_NAME
    printf("BEGIN inst_object_expr(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    defined_object = NULL;
    match_name_list(object_name->value.listValue);
    push_stack();
    name_list = eval_name_list(object_name->value.listValue, err_info);
    down_stack();
    /* printf("name_list ");
    prot_list(name_list);
    printf("\n");
    fflush(stdout);
    printf("name_list->obj ");
    trace1(name_list->obj);
    printf("\n");
    fflush(stdout); */
    if (CATEGORY_OF_OBJ(name_list->obj) == FORMPARAMOBJECT) {
      param_obj = name_list->obj->value.objValue;
      if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
          CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT ||
          CATEGORY_OF_OBJ(param_obj) == TYPEOBJECT) {
        err_object(IDENT_EXPECTED, object_name);
      } else {
        /* printf("param_obj ");
        trace1(param_obj);
        printf("\n"); */
        defined_object = inst_object(declaration_base, param_obj, 0, 0, err_info);
      } /* if */
    } else {
      err_object(IDENT_EXPECTED, object_name);
    } /* if */
    free_name_list(name_list, FALSE);
#ifdef TRACE_NAME
    printf("END inst_object_expr --> ");
    trace1(defined_object);
    printf("\n");
#endif
    return defined_object;
  } /* inst_object_expr */



objectType entername (nodeType declaration_base, objectType object_name,
    errInfoType *err_info)

  {
    objectType defined_object;

  /* entername */
#ifdef TRACE_NAME
    printf("BEGIN entername(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    if (CATEGORY_OF_OBJ(object_name) == EXPROBJECT) {
      if (object_name->value.listValue->next != NULL) {
        defined_object = inst_list(declaration_base, object_name, err_info);
      } else if (CATEGORY_OF_OBJ(object_name->value.listValue->obj) == EXPROBJECT ||
          CATEGORY_OF_OBJ(object_name->value.listValue->obj) == MATCHOBJECT) {
        defined_object = inst_object_expr(declaration_base, object_name, err_info);
      } else {
        /* printf("listValue->obj ");
        trace1(object_name->value.listValue->obj);
        printf(")\n"); */
        defined_object = inst_object(declaration_base,
            object_name->value.listValue->obj,
            GET_FILE_NUM(object_name), GET_LINE_NUM(object_name), err_info);
      } /* if */
    } else {
      defined_object = inst_object(declaration_base, object_name, 0, 0, err_info);
/* printf(" %s\n", defined_object->IDENT->NAME);
   printf("o%d_%s declared \n", defined_object->NUMBER,
       defined_object->IDENT->NAME); */
    } /* if */
#ifdef TRACE_NAME
    trace_nodes();
    printf("END entername(");
    trace1(object_name);
    printf(") --> ");
    trace1(defined_object);
    printf("\n");
#endif
    return defined_object;
  } /* entername */



objectType find_name (nodeType declaration_base, const_objectType object_name,
    errInfoType *err_info)

  {
    listType name_list;
    objectType param_obj;
    entityType entity;
    objectType defined_object;

  /* find_name */
#ifdef TRACE_NAME
    printf("BEGIN find_name(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    grow_stack(err_info);
    if (*err_info == OKAY_NO_ERROR) {
      if (CATEGORY_OF_OBJ(object_name) == EXPROBJECT) {
        if (object_name->value.listValue->next != NULL) {
          match_name_list(object_name->value.listValue);
          push_stack();
          name_list = eval_name_list(object_name->value.listValue, err_info);
          down_stack();
          entity = find_entity(declaration_base, name_list);
          shrink_stack();
          free_name_list(name_list, TRUE);
        } else if (CATEGORY_OF_OBJ(object_name->value.listValue->obj) == EXPROBJECT ||
            CATEGORY_OF_OBJ(object_name->value.listValue->obj) == MATCHOBJECT) {
          match_name_list(object_name->value.listValue);
          push_stack();
          name_list = eval_name_list(object_name->value.listValue, err_info);
          down_stack();
          if (CATEGORY_OF_OBJ(name_list->obj) == FORMPARAMOBJECT) {
            param_obj = name_list->obj->value.objValue;
            if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
                CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT ||
                CATEGORY_OF_OBJ(param_obj) == TYPEOBJECT) {
              entity = NULL;
            } else {
              entity = GET_ENTITY(param_obj);
            } /* if */
          } else {
            entity = NULL;
          } /* if */
          shrink_stack();
          free_name_list(name_list, FALSE);
        } else {
          entity = GET_ENTITY(object_name->value.listValue->obj);
          shrink_stack();
        } /* if */
      } else {
        entity = GET_ENTITY(object_name);
        shrink_stack();
      } /* if */
    } else {
      entity = NULL;
    } /* if */
    if (entity != NULL && entity->data.owner != NULL) {
      defined_object = entity->data.owner->obj;
    } else {
      defined_object = NULL;
    } /* if */

/* printf(" %s\n", defined_object->IDENT->NAME);
   printf("o%d_%s declared \n", defined_object->NUMBER,
       defined_object->IDENT->NAME); */

#ifdef TRACE_NAME
    trace_nodes();
    printf("END find_name(");
    trace1(object_name);
    printf(") --> ");
    trace1(defined_object);
    printf("\n");
#endif
    return defined_object;
  } /* find_name */



objectType search_name (const_nodeType declaration_base,
    const_objectType object_name, errInfoType *err_info)

  {
    listType name_list;
    objectType param_obj;
    entityType entity;
    objectType defined_object;

  /* search_name */
#ifdef TRACE_NAME
    printf("BEGIN search_name(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    if (CATEGORY_OF_OBJ(object_name) == EXPROBJECT) {
      if (object_name->value.listValue->next != NULL) {
        match_name_list(object_name->value.listValue);
        push_stack();
        name_list = eval_name_list(object_name->value.listValue, err_info);
        down_stack();
        entity = search_entity(declaration_base, name_list);
        /* printf("search_name: free_name_list\n"); */
        free_name_list(name_list, TRUE);
        /* free_list(name_list); */
      } else if (CATEGORY_OF_OBJ(object_name->value.listValue->obj) == EXPROBJECT ||
          CATEGORY_OF_OBJ(object_name->value.listValue->obj) == MATCHOBJECT) {
        match_name_list(object_name->value.listValue);
        push_stack();
        name_list = eval_name_list(object_name->value.listValue, err_info);
        down_stack();
        if (CATEGORY_OF_OBJ(name_list->obj) == FORMPARAMOBJECT) {
          param_obj = name_list->obj->value.objValue;
          if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
              CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT ||
              CATEGORY_OF_OBJ(param_obj) == TYPEOBJECT) {
            entity = NULL;
          } else {
            entity = GET_ENTITY(param_obj);
          } /* if */
        } else {
          entity = NULL;
        } /* if */
        free_name_list(name_list, FALSE);
      } else {
        entity = GET_ENTITY(object_name->value.listValue->obj);
      } /* if */
    } else {
      entity = GET_ENTITY(object_name);
    } /* if */
    if (entity != NULL && entity->data.owner != NULL) {
      defined_object = entity->data.owner->obj;
    } else {
      defined_object = NULL;
    } /* if */

/* printf(" %s\n", defined_object->IDENT->NAME);
   printf("o%d_%s declared \n", defined_object->NUMBER,
       defined_object->IDENT->NAME); */

#ifdef TRACE_NAME
    trace_nodes();
    printf("END search_name(");
    trace1(object_name);
    printf(") --> ");
    trace1(defined_object);
    printf("\n");
#endif
    return defined_object;
  } /* search_name */



static objectType dollar_parameter (objectType param_object)

  {
    listType param_descr;
    objectType type_of_parameter;

  /* dollar_parameter */
#ifdef TRACE_NAME
    printf("BEGIN dollar_parameter(");
    trace1(param_object);
    printf(")\n");
#endif
    param_descr = param_object->value.listValue;
    if (param_descr != NULL) {
      if (GET_ENTITY(param_descr->obj)->ident == prog.id_for.ref) {
        /* printf("### ref param\n"); */
        if (param_descr->next != NULL) {
          type_of_parameter = param_descr->next->obj;
          if (CATEGORY_OF_OBJ(type_of_parameter) == EXPROBJECT) {
            param_descr->next->obj = eval_expression(type_of_parameter);
          } /* if */
          if (param_descr->next->next != NULL) {
            FREE_OBJECT(param_object);
            if (GET_ENTITY(param_descr->next->next->obj)->ident == prog.id_for.colon) {
              param_object = dcl_ref2(param_descr);
            } else {
              param_object = dcl_ref1(param_descr);
            } /* if */
            free_list(param_descr);
          } /* if */
        } /* if */
      } else {
        err_ident(PARAM_SPECIFIER_EXPECTED, GET_ENTITY(param_descr->obj)->ident);
      } /* if */
    } /* if */
#ifdef TRACE_NAME
    printf("END dollar_parameter --> ");
    trace1(param_object);
    printf("\n");
#endif
    return param_object;
  } /* dollar_parameter */



static objectType dollar_inst_list (nodeType declaration_base,
    const_objectType object_name, errInfoType *err_info)

  {
    listType name_elem;
    objectType defined_object;

  /* dollar_inst_list */
#ifdef TRACE_NAME
    printf("BEGIN dollar_inst_list(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    name_elem = object_name->value.listValue;
    while (name_elem != NULL) {
      if (CATEGORY_OF_OBJ(name_elem->obj) == EXPROBJECT) {
        name_elem->obj = dollar_parameter(name_elem->obj);
      } /* if */
      name_elem = name_elem->next;
    } /* while */
    defined_object = push_name(declaration_base,
        object_name->value.listValue,
        GET_FILE_NUM(object_name), GET_LINE_NUM(object_name), err_info);
#ifdef TRACE_NAME
    printf("END dollar_inst_list\n");
#endif
    return defined_object;
  } /* dollar_inst_list */



objectType dollar_entername (nodeType declaration_base, objectType object_name,
    errInfoType *err_info)

  {
    objectType defined_object;

  /* dollar_entername */
#ifdef TRACE_NAME
    printf("BEGIN dollar_entername(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    if (CATEGORY_OF_OBJ(object_name) == EXPROBJECT) {
      defined_object = dollar_inst_list(declaration_base, object_name, err_info);
    } else {
      defined_object = inst_object(declaration_base, object_name, 0, 0, err_info);
/* printf(" %s\n", defined_object->IDENT->NAME);
   printf("o%d_%s declared \n", defined_object->NUMBER,
       defined_object->IDENT->NAME); */
    } /* if */
#ifdef TRACE_NAME
    trace_nodes();
    printf("END dollar_entername(");
    trace1(object_name);
    printf(") --> ");
    trace1(defined_object);
    printf("\n");
#endif
    return defined_object;
  } /* dollar_entername */
