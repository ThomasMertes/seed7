/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/name.c                                          */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Enter an object in a specified declaration level.      */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "identutl.h"
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



#ifdef ANSI_C

static void push_owner (ownertype *owner, objecttype obj_to_push,
    listtype params, errinfotype *err_info)
#else

static void push_owner (owner, obj_to_push, params, err_info)
ownertype *owner;
objecttype obj_to_push;
listtype params;
errinfotype *err_info;
#endif

  {
    ownertype created_owner;

  /* push_owner */
#ifdef TRACE_NAME
    printf("BEGIN push_owner\n");
#endif
    if (ALLOC_RECORD(created_owner, ownerrecord)) {
      COUNT_RECORD(ownerrecord, count.owner);
      created_owner->obj = obj_to_push;
      created_owner->params = params;
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



#ifdef ANSI_C

static objecttype get_object (entitytype ent, listtype params, errinfotype *err_info)
#else

static objecttype get_object (ent, params, err_info)
entitytype ent;
listtype params;
errinfotype *err_info;
#endif

  {
    objecttype defined_object;
    objecttype forward_reference;

  /* get_object */
#ifdef TRACE_NAME
    printf("BEGIN get_object\n");
#endif
    if (ent->owner != NULL &&
        ent->owner->decl_level == prog.stack_current) {
      defined_object = ent->owner->obj;
      ent->owner->params = params;
      if (CLASS_OF_OBJ(defined_object) != FORWARDOBJECT) {
        err_object(OBJTWICEDECLARED, ent->owner->obj);
        SET_CLASS_OF_OBJ(defined_object, DECLAREDOBJECT);
      } else {
        SET_CLASS_OF_OBJ(defined_object, DECLAREDOBJECT);
        if (ALLOC_OBJECT(forward_reference)) {
          forward_reference->type_of = NULL;
          forward_reference->entity = NULL;
          INIT_CLASS_OF_OBJ(forward_reference, FWDREFOBJECT);
          forward_reference->value.objvalue = defined_object;
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
        defined_object->type_of = NULL;
        defined_object->entity = ent;
        INIT_CLASS_OF_OBJ(defined_object, DECLAREDOBJECT);
        defined_object->value.objvalue = NULL;
        push_owner(&ent->owner, defined_object, params, err_info);
        prog.stack_current->object_list_insert_place = append_element_to_list(
            prog.stack_current->object_list_insert_place, defined_object, err_info);
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
    return(defined_object);
  } /* get_object */



#ifdef ANSI_C

listtype create_parameter_list (listtype name_list, errinfotype *err_info)
#else

listtype create_parameter_list (name_list, err_info)
listtype name_list;
errinfotype *err_info;
#endif

  {
    listtype name_elem;
    objecttype param_obj;
    listtype parameter_list;
    listtype *list_insert_place;

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
      if (CLASS_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
/* printf("create paramobject ");
trace1(name_elem->obj);
printf(" %lu\n", (long unsigned) name_elem->obj); */
        param_obj = name_elem->obj->value.objvalue;
        if (CLASS_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
            CLASS_OF_OBJ(param_obj) == REFPARAMOBJECT) {
/* printf("create in or ref param ");
trace1(param_obj);
printf(" %lu\n", (long unsigned) param_obj); */
          list_insert_place = append_element_to_list(list_insert_place,
              param_obj, err_info);
        } else if (CLASS_OF_OBJ(param_obj) == TYPEOBJECT) {
/* printf("create attr param ");
trace1(param_obj);
printf(" %lu\n", (long unsigned) param_obj); */
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
    return(parameter_list);
  } /* create_parameter_list */



#ifdef ANSI_C

static objecttype push_name (nodetype declaration_base,
    listtype name_list, errinfotype *err_info)
#else

static objecttype push_name (declaration_base, name_list, err_info)
nodetype declaration_base;
listtype name_list;
errinfotype *err_info;
#endif

  {
    entitytype ent;
    listtype params;
    objecttype defined_object;

  /* push_name */
#ifdef TRACE_NAME
    printf("BEGIN push_name(%ld, ", (long) declaration_base);
    prot_list(name_list);
    printf(")\n");
#endif
    ent = get_entity(declaration_base, name_list);
    if (ent == NULL) {
      *err_info = MEMORY_ERROR;
      defined_object = NULL;
    } else {
      params = create_parameter_list(name_list, err_info);
      defined_object = get_object(ent, params, err_info);
    } /* if */
#ifdef TRACE_NAME
    printf("END push_name(");
    printf("%lu ", (unsigned long) defined_object);
    /* prot_list(defined_object->entity->params); */
    printf(")\n");
#endif
    return(defined_object);
  } /* push_name */



#ifdef ANSI_C

static void pop_object (objecttype obj_to_pop)
#else

static void pop_object (obj_to_pop)
objecttype obj_to_pop;
#endif

  {
    entitytype ent;
    ownertype owner;

  /* pop_object */
#ifdef TRACE_NAME
    printf("BEGIN pop_object(");
    trace1(obj_to_pop);
    printf(")\n");
    fflush(stdout);
#endif
    ent = obj_to_pop->entity;
    if (ent != NULL) {
      owner = ent->owner;
      if (owner != NULL) {
        ent->owner = owner->next;
        FREE_RECORD(owner, ownerrecord, count.owner);
        if (ent->owner == NULL && ent->name_list != NULL) {
          pop_entity(prog.declaration_root, ent);
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_NAME
    printf("END pop_object\n");
#endif
  } /* pop_object */



#ifdef ANSI_C

void init_stack (progtype currentProg, errinfotype *err_info)
#else

void init_stack (currentProg, err_info)
progtype currentProg;
errinfotype *err_info;
#endif

  {
    stacktype created_stack_element;

  /* init_stack */
#ifdef TRACE_NAME
    printf("BEGIN init_stack\n");
#endif
    if (ALLOC_RECORD(created_stack_element, stackrecord)) {
      COUNT_RECORD(stackrecord, count.stack);
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



#ifdef ANSI_C

void close_stack (progtype currentProg)
#else

void close_stack (currentProg)
progtype currentProg
#endif

  {
    listtype list_element;

  /* close_stack */
#ifdef TRACE_NAME
    printf("BEGIN close_stack %d\n", data_depth);
#endif
    list_element = currentProg->stack_data->local_object_list;
    while (list_element != NULL) {
      pop_object(list_element->obj);
      list_element = list_element->next;
    } /* while */
    destroy_object_list(currentProg->stack_data->local_object_list);
    emptylist(currentProg->stack_data->local_object_list);
    currentProg->stack_current->local_object_list = NULL;
#ifdef TRACE_NAME
    printf("END close_stack %d\n", data_depth);
#endif
  } /* close_stack */



#ifdef ANSI_C

void grow_stack (errinfotype *err_info)
#else

void grow_stack (err_info)
errinfotype *err_info;
#endif

  {
    stacktype created_stack_element;

  /* grow_stack */
#ifdef TRACE_NAME
    printf("BEGIN grow_stack %d\n", data_depth);
#endif
    if (ALLOC_RECORD(created_stack_element, stackrecord)) {
      COUNT_RECORD(stackrecord, count.stack);
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



#ifdef ANSI_C

void shrink_stack (void)
#else

void shrink_stack ()
#endif

  {
    listtype list_element;
    stacktype old_stack_element;

  /* shrink_stack */
#ifdef TRACE_NAME
    printf("BEGIN shrink_stack %d\n", data_depth);
#endif
    list_element = prog.stack_data->local_object_list;
    while (list_element != NULL) {
      pop_object(list_element->obj);
      list_element = list_element->next;
    } /* while */
    emptylist(prog.stack_data->local_object_list);
    old_stack_element = prog.stack_data;
    prog.stack_data = prog.stack_data->downward;
    prog.stack_data->upward = NULL;
    FREE_RECORD(old_stack_element, stackrecord, count.stack);
    data_depth--;
#ifdef TRACE_NAME
    printf("END shrink_stack %d\n", data_depth);
#endif
  } /* shrink_stack */



#ifdef ANSI_C

void push_stack (void)
#else

void push_stack ()
#endif

  { /* push_stack */
#ifdef TRACE_NAME
    printf("BEGIN push_stack %d\n", depth);
#endif
    if (prog.stack_current->upward != NULL) {
      prog.stack_current = prog.stack_current->upward;
      depth++;
    } else {
      printf("cannot go up");
    } /* if */
#ifdef TRACE_NAME
    printf("END push_stack %d\n", depth);
#endif
  } /* push_stack */



#ifdef ANSI_C

void pop_stack (void)
#else

void pop_stack ()
#endif

  {
    listtype list_element;

  /* pop_stack */
#ifdef TRACE_NAME
    printf("BEGIN pop_stack %d\n", depth);
#endif
    list_element = prog.stack_current->local_object_list;
    while (list_element != NULL) {
      pop_object(list_element->obj);
      list_element = list_element->next;
    } /* while */
    emptylist(prog.stack_current->local_object_list);
    prog.stack_current->local_object_list = NULL;
    prog.stack_current->object_list_insert_place =
        &prog.stack_current->local_object_list;
    if (prog.stack_current->downward != NULL) {
      prog.stack_current = prog.stack_current->downward;
      depth--;
    } else {
      printf("cannot go down");
    } /* if */
#ifdef TRACE_NAME
    printf("END pop_stack %d\n", depth);
#endif
  } /* pop_stack */



#ifdef ANSI_C

static void down_stack (void)
#else

static void down_stack ()
#endif

  { /* down_stack */
#ifdef TRACE_NAME
    printf("BEGIN down_stack %d\n", depth);
#endif
    if (prog.stack_current->downward != NULL) {
      prog.stack_current = prog.stack_current->downward;
      depth--;
    } else {
      printf("cannot go down");
    } /* if */
#ifdef TRACE_NAME
    printf("END down_stack %d\n", depth);
#endif
  } /* down_stack */



#ifdef ANSI_C

listtype *get_local_object_insert_place (void)
#else

listtype *get_local_object_insert_place ()
#endif

  { /* get_local_object_insert_place */
    return(prog.stack_current->object_list_insert_place);
  } /* get_local_object_insert_place */



#ifdef ANSI_C

static void match_name_list (listtype raw_name_list)
#else

static void match_name_list (raw_name_list)
listtype raw_name_list;
#endif

  {
    listtype name_elem;

  /* match_name_list */
#ifdef TRACE_NAME
    printf("BEGIN match_name_list\n");
#endif
    name_elem = raw_name_list;
    while (name_elem != NULL) {
      if (CLASS_OF_OBJ(name_elem->obj) == EXPROBJECT) {
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



#ifdef ANSI_C

static listtype eval_name_list (listtype raw_name_list, errinfotype *err_info)
#else

static listtype eval_name_list (raw_name_list, err_info)
listtype raw_name_list;
errinfotype *err_info;
#endif

  {
    listtype name_elem;
    listtype name_list;
    listtype *list_insert_place;
    objecttype parameter;

  /* eval_name_list */
#ifdef TRACE_NAME
    printf("BEGIN eval_name_list\n");
#endif
    name_elem = raw_name_list;
    name_list = NULL;
    list_insert_place = &name_list;
    while (name_elem != NULL) {
      if (CLASS_OF_OBJ(name_elem->obj) == MATCHOBJECT) {
        if ((parameter = evaluate(name_elem->obj)) != NULL) {
          list_insert_place = append_element_to_list(list_insert_place,
              parameter, err_info);
        } else {
          err_object(PARAM_DECL_FAILED, name_elem->obj);
          list_insert_place = append_element_to_list(list_insert_place,
              name_elem->obj, err_info);
        } /* if */
      } else {
        if (name_elem->obj->entity != NULL && name_elem->obj->entity->syobject != NULL) {
          parameter = name_elem->obj->entity->syobject;
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
    return(name_list);
  } /* eval_name_list */



#ifdef ANSI_C

static objecttype inst_list (nodetype declaration_base, objecttype object_name,
    errinfotype *err_info)
#else

static objecttype inst_list (declaration_base, object_name, err_info)
nodetype declaration_base;
objecttype object_name;
errinfotype *err_info;
#endif

  {
    listtype name_list;
    objecttype defined_object;

  /* inst_list */
#ifdef TRACE_NAME
    printf("BEGIN inst_list(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    match_name_list(object_name->value.listvalue);
    push_stack();
    name_list = eval_name_list(object_name->value.listvalue, err_info);
    down_stack();
    defined_object = push_name(declaration_base, name_list, err_info);
#ifdef TRACE_NAME
    printf("END inst_list --> ");
    trace1(defined_object);
    printf("\n");
#endif
    return(defined_object);
  } /* inst_list */



#ifdef ANSI_C

static objecttype inst_object (nodetype declaration_base, objecttype name_object,
    errinfotype *err_info)
#else

static objecttype inst_object (declaration_base, name_object, err_info)
nodetype declaration_base;
objecttype name_object;
errinfotype *err_info;
#endif

  {
    objecttype defined_object;

  /* inst_object */
#ifdef TRACE_NAME
    printf("BEGIN inst_object(");
    trace1(name_object);
    printf(")\n");
#endif
    if (name_object->entity == entity.literal) {
      err_object(IDENT_EXPECTED, name_object);
    } /* if */
    defined_object = get_object(name_object->entity, NULL, err_info);
#ifdef TRACE_NAME
    printf("END inst_object --> ");
    trace1(defined_object);
    printf("\n");
#endif
    return(defined_object);
  } /* inst_object */



#ifdef ANSI_C

static objecttype inst_object_expr (nodetype declaration_base, objecttype object_name,
    errinfotype *err_info)
#else

static objecttype inst_object_expr (declaration_base, object_name, err_info)
nodetype declaration_base;
objecttype object_name;
errinfotype *err_info;
#endif

  {
    listtype name_list;
    objecttype param_obj;
    objecttype defined_object;

  /* inst_object_expr */
#ifdef TRACE_NAME
    printf("BEGIN inst_object_expr(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    defined_object = NULL;
    match_name_list(object_name->value.listvalue);
    push_stack();
    name_list = eval_name_list(object_name->value.listvalue, err_info);
    down_stack();
    /* printf("name_list ");
    prot_list(name_list);
    printf("\n");
    fflush(stdout);
    printf("name_list->obj ");
    trace1(name_list->obj);
    printf("\n");
    fflush(stdout); */
    if (CLASS_OF_OBJ(name_list->obj) == FORMPARAMOBJECT) {
      param_obj = name_list->obj->value.objvalue;
      if (CLASS_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
          CLASS_OF_OBJ(param_obj) == REFPARAMOBJECT ||
          CLASS_OF_OBJ(param_obj) == TYPEOBJECT) {
        err_object(IDENT_EXPECTED, object_name);
      } else {
        /* printf("param_obj ");
        trace1(param_obj);
        printf("\n"); */
        defined_object = inst_object(declaration_base, param_obj, err_info);        
     } /* if */
    } else {
      err_object(IDENT_EXPECTED, object_name);
    } /* if */
#ifdef TRACE_NAME
    printf("END inst_object_expr --> ");
    trace1(defined_object);
    printf("\n");
#endif
    return(defined_object);
  } /* inst_object_expr */



#ifdef ANSI_C

objecttype entername (nodetype declaration_base, objecttype object_name,
    errinfotype *err_info)
#else

objecttype entername (declaration_base, object_name, err_info)
nodetype declaration_base;
objecttype object_name;
errinfotype *err_info;
#endif

  {
    objecttype defined_object;

  /* entername */
#ifdef TRACE_NAME
    printf("BEGIN entername(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    if (CLASS_OF_OBJ(object_name) == EXPROBJECT) {
      if (object_name->value.listvalue->next != NULL) {
        defined_object = inst_list(declaration_base, object_name, err_info);
      } else if (CLASS_OF_OBJ(object_name->value.listvalue->obj) == EXPROBJECT ||
          CLASS_OF_OBJ(object_name->value.listvalue->obj) == MATCHOBJECT) {
        defined_object = inst_object_expr(declaration_base, object_name, err_info);
      } else {
        /* printf("listvalue->obj ");
        trace1(object_name->value.listvalue->obj);
        printf(")\n"); */
        defined_object = inst_object(declaration_base,
            object_name->value.listvalue->obj, err_info);
      } /* if */
    } else {
      defined_object = inst_object(declaration_base, object_name, err_info);
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
    return(defined_object);
  } /* entername */



#ifdef ANSI_C

objecttype find_name (nodetype declaration_base, objecttype object_name,
    errinfotype *err_info)
#else

objecttype find_name (declaration_base, object_name, err_info)
nodetype declaration_base;
objecttype object_name;
errinfotype *err_info;
#endif

  {
    listtype name_list;
    objecttype param_obj;
    entitytype ent;
    objecttype defined_object;

  /* find_name */
#ifdef TRACE_NAME
    printf("BEGIN find_name(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    if (CLASS_OF_OBJ(object_name) == EXPROBJECT) {
      if (object_name->value.listvalue->next != NULL) {
        match_name_list(object_name->value.listvalue);
        push_stack();
        name_list = eval_name_list(object_name->value.listvalue, err_info);
        down_stack();
        ent = find_entity(declaration_base, name_list);
        emptylist(name_list);
      } else if (CLASS_OF_OBJ(object_name->value.listvalue->obj) == EXPROBJECT ||
          CLASS_OF_OBJ(object_name->value.listvalue->obj) == MATCHOBJECT) {
        match_name_list(object_name->value.listvalue);
        push_stack();
        name_list = eval_name_list(object_name->value.listvalue, err_info);
        down_stack();
        if (CLASS_OF_OBJ(name_list->obj) == FORMPARAMOBJECT) {
          param_obj = name_list->obj->value.objvalue;
          if (CLASS_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
              CLASS_OF_OBJ(param_obj) == REFPARAMOBJECT ||
              CLASS_OF_OBJ(param_obj) == TYPEOBJECT) {
            ent = NULL;
          } else {
            ent = param_obj->entity;
         } /* if */
        } else {
          ent = NULL;
        } /* if */
      } else {
        ent = object_name->value.listvalue->obj->entity;
      } /* if */
    } else {
      ent = object_name->entity;
    } /* if */
    if (ent != NULL && ent->owner != NULL) {
      defined_object = ent->owner->obj;
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
    return(defined_object);
  } /* find_name */



#ifdef ANSI_C

objecttype search_name (nodetype declaration_base, objecttype object_name,
    errinfotype *err_info)
#else

objecttype search_name (declaration_base, object_name, err_info)
nodetype declaration_base;
objecttype object_name;
errinfotype *err_info;
#endif

  {
    listtype name_list;
    objecttype param_obj;
    entitytype ent;
    objecttype defined_object;

  /* search_name */
#ifdef TRACE_NAME
    printf("BEGIN search_name(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    if (CLASS_OF_OBJ(object_name) == EXPROBJECT) {
      if (object_name->value.listvalue->next != NULL) {
        match_name_list(object_name->value.listvalue);
        push_stack();
        name_list = eval_name_list(object_name->value.listvalue, err_info);
        down_stack();
        ent = search_entity(declaration_base, name_list);
        emptylist(name_list);
      } else if (CLASS_OF_OBJ(object_name->value.listvalue->obj) == EXPROBJECT ||
          CLASS_OF_OBJ(object_name->value.listvalue->obj) == MATCHOBJECT) {
        match_name_list(object_name->value.listvalue);
        push_stack();
        name_list = eval_name_list(object_name->value.listvalue, err_info);
        down_stack();
        if (CLASS_OF_OBJ(name_list->obj) == FORMPARAMOBJECT) {
          param_obj = name_list->obj->value.objvalue;
          if (CLASS_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
              CLASS_OF_OBJ(param_obj) == REFPARAMOBJECT ||
              CLASS_OF_OBJ(param_obj) == TYPEOBJECT) {
            ent = NULL;
          } else {
            ent = param_obj->entity;
         } /* if */
        } else {
          ent = NULL;
        } /* if */
      } else {
        ent = object_name->value.listvalue->obj->entity;
      } /* if */
    } else {
      ent = object_name->entity;
    } /* if */
    if (ent != NULL && ent->owner != NULL) {
      defined_object = ent->owner->obj;
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
    return(defined_object);
  } /* search_name */



#ifdef ANSI_C

static objecttype dollar_parameter (objecttype param_object)
#else

static objecttype dollar_parameter (param_object)
objecttype param_object;
#endif

  {
    listtype param_descr;
    objecttype type_of_parameter;

  /* dollar_parameter */
#ifdef TRACE_NAME
    printf("BEGIN dollar_parameter(");
    trace1(param_object);
    printf(")\n");
#endif
    param_descr = param_object->value.listvalue;
    if (param_descr != NULL) {
      if (param_descr->obj->entity->ident == prog.id_for.ref) {
        /* printf("### ref param\n"); */
        if (param_descr->next != NULL) {
          type_of_parameter = param_descr->next->obj;
          if (CLASS_OF_OBJ(type_of_parameter) == EXPROBJECT) {
            param_descr->next->obj = eval_expression(type_of_parameter);
          } /* if */
          if (param_descr->next->next != NULL) {
            FREE_OBJECT(param_object);
            if (param_descr->next->next->obj->entity->ident == prog.id_for.colon) {
              param_object = dcl_ref2(param_descr);
            } else {
              param_object = dcl_ref1(param_descr);
            } /* if */
            emptylist(param_descr);
          } /* if */
        } /* if */
      } else {
        err_ident(PARAM_SPECIFIER_EXPECTED, param_descr->obj->entity->ident);
      } /* if */
    } /* if */
#ifdef TRACE_NAME
    printf("END dollar_parameter --> ");
    trace1(param_object);
    printf("\n");
#endif
    return(param_object);
  } /* dollar_parameter */



#ifdef ANSI_C

static objecttype dollar_inst_list (nodetype declaration_base, objecttype object_name,
    errinfotype *err_info)
#else

static objecttype dollar_inst_list (declaration_base, object_name, err_info)
nodetype declaration_base;
objecttype object_name;
errinfotype *err_info;
#endif

  {
    listtype name_elem;
    objecttype defined_object;

  /* dollar_inst_list */
#ifdef TRACE_NAME
    printf("BEGIN dollar_inst_list(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    name_elem = object_name->value.listvalue;
    while (name_elem != NULL) {
      if (CLASS_OF_OBJ(name_elem->obj) == EXPROBJECT) {
        name_elem->obj = dollar_parameter(name_elem->obj);
      } /* if */
      name_elem = name_elem->next;
    } /* while */
    defined_object = push_name(declaration_base,
        object_name->value.listvalue, err_info);
#ifdef TRACE_NAME
    printf("END dollar_inst_list\n");
#endif
    return(defined_object);
  } /* dollar_inst_list */



#ifdef ANSI_C

objecttype dollar_entername (nodetype declaration_base, objecttype object_name,
    errinfotype *err_info)
#else

objecttype dollar_entername (declaration_base, object_name, err_info)
nodetype declaration_base;
objecttype object_name;
errinfotype *err_info;
#endif

  {
    objecttype defined_object;

  /* dollar_entername */
#ifdef TRACE_NAME
    printf("BEGIN dollar_entername(%ld, ", (long) declaration_base);
    trace1(object_name);
    printf(")\n");
#endif
    if (CLASS_OF_OBJ(object_name) == EXPROBJECT) {
      defined_object = dollar_inst_list(declaration_base, object_name, err_info);
    } else {
      defined_object = inst_object(declaration_base, object_name, err_info);
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
    return(defined_object);
  } /* dollar_entername */
