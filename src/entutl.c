/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2015, 2021, 2025  Thomas Mertes            */
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
/*  File: seed7/src/entutl.c                                        */
/*  Changes: 2000, 2013 - 2015, 2021, 2025  Thomas Mertes           */
/*  Content: Functions to maintain objects of type entityType.      */
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
#include "listutl.h"
#include "datautl.h"
#include "traceutl.h"

#undef EXTERN
#define EXTERN
#include "entutl.h"


  /* The macro PTR_LESS is used to generate a less than comparison  */
  /* for pointers. Since the comparison is used to build a binary   */
  /* tree a true less operation would have the danger of            */
  /* degenerating the tree to a linear list. But a true less        */
  /* comparison works since I have used it for years at this place. */
  /* If you want a true less comparison replace this macro by:      */
  /*                                                                */
  /*   #define PTR_LESS(P1,P2) ((P1) < (P2))                        */
  /*                                                                */
  /* The K&R standard allows pointer comparisons only inside        */
  /* arrays. So the true less comparison does not conform to the    */
  /* standard. The macro below casts the pointers to memSizeType,   */
  /* so it is conforming to the standard. Taking only the lower     */
  /* bits for the comparison has a "random" effect for the binary   */
  /* tree. This speeds the tree up a measurable amount of time.     */

#define PTR_LESS(P1,P2) (((memSizeType) (P1) & 0377L) < ((memSizeType) (P2) & 0377L))

/* #define PTR_LESS(P1,P2) (((memSizeType) (P1) & 0177400L) < ((memSizeType) (P2) & 0177400L)) */
/* #define PTR_LESS(P1,P2) ((P1) < (P2)) */



/**
 *  Frees all nodes below the root 'node'.
 */
static void free_nodes (nodeType node)

  { /* free_nodes */
    logFunction(printf("free_nodes(");
                printnodes(node);
                printf(")\n"););
    if (node != NULL) {
      /* node->entity is not removed here */
      free_nodes(node->next1);
      free_nodes(node->next2);
      free_nodes(node->symbol);
      free_nodes(node->inout_param);
      free_nodes(node->other_param);
      free_nodes(node->attr);
      FREE_NODE(node);
    } /* if */
    logFunction(printf("free_nodes -->\n"););
  } /* free_nodes */



/**
 *  Create a new node with the given match_obj 'obj'.
 *  @return the new node or NULL, if a memory error occurred.
 */
static nodeType new_node (objectType obj)

  {
    nodeType created_node;

  /* new_node */
    logFunction(printf("new_node(");
                trace1(obj);
                printf(")\n"););
    if (ALLOC_NODE(created_node)) {
      created_node->usage_count = 1;
      created_node->match_obj = obj;
      created_node->next1 = NULL;
      created_node->next2 = NULL;
      created_node->entity = NULL;
      created_node->symbol = NULL;
      created_node->inout_param = NULL;
      created_node->other_param = NULL;
      created_node->attr = NULL;
    } /* if */
    logFunction(printf("new_node --> ");
                trace_node(created_node);
                printf("\n"););
    return created_node;
  } /* new_node */



/**
 *  Searches node_tree for a node matching object_searched.
 *  If a node is found it is returned. If no matching node is
 *  found a new node is generated and entered into node_tree.
 *  In this case the new node is returned.
 *  @return the node found or the new generated node or
 *          NULL, if a memory error occurred.
 */
static nodeType get_node (nodeType *node_tree,
    register objectType object_searched)

  {
    register nodeType search_node;
    register boolType searching;
    nodeType node_found;

  /* get_node */
    logFunction(printf("get_node({ ");
                printnodes(*node_tree);
                printf("}, ");
                trace1(object_searched);
                printf(")\n"););
    if ((search_node = *node_tree) == NULL) {
      node_found = new_node(object_searched);
      *node_tree = node_found;
    } else {
      node_found = NULL;
      searching = TRUE;
      do {
        if (PTR_LESS(object_searched, search_node->match_obj)) {
          if (search_node->next1 == NULL) {
            node_found = new_node(object_searched);
            search_node->next1 = node_found;
            searching = FALSE;
          } else {
            search_node = search_node->next1;
          } /* if */
        } else if (object_searched == search_node->match_obj) {
          node_found = search_node;
          node_found->usage_count++;
          searching = FALSE;
        } else {    /* (object_searched > search_node->match_obj) */
          if (search_node->next2 == NULL) {
            node_found = new_node(object_searched);
            search_node->next2 = node_found;
            searching = FALSE;
          } else {
            search_node = search_node->next2;
          } /* if */
        } /* if */
      } while (searching);
    } /* if */
    /* printf("get_node >%s<\n", object_searched->entity->ident->name); */
    logFunction(printf("get_node --> ");
                trace_node(node_found);
                printf("\n"););
    return node_found;
  } /* get_node */



/**
 *  Searches node_tree for a node matching object_searched.
 *  @return the node found or NULL if no matching node is found.
 */
nodeType find_node (register nodeType node_tree,
    register objectType object_searched)

  {
    nodeType node_found = NULL;

  /* find_node */
    logFunction(printf("find_node({ ");
                printnodes(node_tree);
                printf("}, ");
                trace1(object_searched);
                printf("=" FMT_U_MEM ")\n",
                       (memSizeType) object_searched););
    while (node_tree != NULL) {
      if (PTR_LESS(object_searched, node_tree->match_obj)) {
        node_tree = node_tree->next1;
      } else if (object_searched == node_tree->match_obj) {
        if (node_tree->usage_count > 0) {
          /* Just consider nodes that are in use now. */
          node_found = node_tree;
        } /* if */
        node_tree = NULL;
      } else {    /* (object_searched > node_tree->match_obj) */
        node_tree = node_tree->next2;
      } /* if */
    } /* while */
    /* printf("%s\n", (node_found != NULL ? "found" : "not found")); */
    logFunction(printf("find_node --> ");
                if (node_found != NULL) {
                  trace1(node_found->match_obj);
                  printf("\n");
                } else {
                  printf("*NULL_NODE*\n");
                });
    return node_found;
  } /* find_node */



/**
 *  Search node_tree for object_searched and decrement node usage count.
 *  Searches node_tree for a node matching object_searched. If a node
 *  is found its usage_count is decremented. If the node is unused now
 *  it is kept, but its symbol, inout, other and attr subnode trees are
 *  removed and set to NULL. For an unused node the next1 and next2
 *  subnodes and the match_obj are kept intact and the function returns
 *  NULL. The function find_node() ignores nodes with usage_count zero
 *  and the function get_node() just reactivates such a node. If the
 *  node found is still in use it is returned. If no matching node is
 *  found NULL is returned.
 *  @return a node that is still in use after it has been popped, or
 *          NULL if the last usage of a node has been removed, or
 *          NULL if no matching node has been found.
 */
static nodeType pop_node (register nodeType node_tree,
    register objectType object_searched)

  {
    nodeType node_found = NULL;

  /* pop_node */
    logFunction(printf("pop_node({ ");
                printnodes(node_tree);
                printf("}, ");
                trace1(object_searched);
                printf("=" FMT_U_MEM ")\n",
                       (memSizeType) object_searched););
    while (node_tree != NULL) {
      if (PTR_LESS(object_searched, node_tree->match_obj)) {
        node_tree = node_tree->next1;
      } else if (object_searched == node_tree->match_obj) {
        if (node_tree->usage_count > 0) {
          /* Just consider nodes that are in use now. */
          node_tree->usage_count--;
          if (node_tree->usage_count == 0) {
            node_tree->entity = NULL;
            free_nodes(node_tree->symbol);
            free_nodes(node_tree->inout_param);
            free_nodes(node_tree->other_param);
            free_nodes(node_tree->attr);
            node_tree->symbol = NULL;
            node_tree->inout_param = NULL;
            node_tree->other_param = NULL;
            node_tree->attr = NULL;
          } else {
            node_found = node_tree;
          } /* if */
        } /* if */
        node_tree = NULL;
      } else {    /* (object_searched > node_tree->match_obj) */
        node_tree = node_tree->next2;
      } /* if */
    } /* while */
    /* printf("%s\n", (node_found != NULL ? "found" : "not found")); */
    logFunction(printf("pop_node --> ");
                if (node_found != NULL) {
                  trace1(node_found->match_obj);
                  printf("\n");
                } else {
                  printf("*NULL_NODE*\n");
                });
    return node_found;
  } /* pop_node */



void init_declaration_root (progType currentProg, errInfoType *err_info)

  { /* init_declaration_root */
    logFunction(printf("init_declaration_root\n"););
    currentProg->declaration_root = new_node(NULL);
    if (currentProg->declaration_root == NULL) {
      *err_info = MEMORY_ERROR;
    } /* if */
    logFunction(printf("init_declaration_root -->\n"););
  } /* init_declaration_root */



void close_declaration_root (progType currentProg)

  { /* close_declaration_root */
    logFunction(printf("close_declaration_root\n"););
    free_nodes(currentProg->declaration_root);
    currentProg->declaration_root = NULL;
    logFunction(printf("close_declaration_root -->\n"););
  } /* close_declaration_root */



void free_entity (const_progType currentProg, entityType old_entity)

  {
    listType name_elem;
    objectType param_obj;

  /* free_entity */
    logFunction(printf("free_entity\n"););
    if (old_entity != NULL) {
      if (old_entity->syobject != NULL) {
        if (HAS_PROPERTY(old_entity->syobject) &&
            old_entity->syobject->descriptor.property != currentProg->property.literal) {
          /* trace1(old_entity->syobject);
             printf("\n"); */
          FREE_PROPERTY(old_entity->syobject->descriptor.property);
        } /* if */
        FREE_OBJECT(old_entity->syobject);
      } /* if */
      if (old_entity->fparam_list != NULL) {
        name_elem = old_entity->fparam_list;
        while (name_elem != NULL) {
          if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
            param_obj = name_elem->obj->value.objValue;
            if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
                CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT) {
              FREE_OBJECT(param_obj);
            } /* if */
            FREE_OBJECT(name_elem->obj);
          } /* if */
          name_elem = name_elem->next;
        } /* if */
        free_list(old_entity->fparam_list);
      } /* if */
      FREE_RECORD(old_entity, entityRecord, count.entity);
    } /* if */
    logFunction(printf("free_entity -->\n"););
  } /* free_entity */



/**
 *  Create a new entity with the given 'id'.
 *  @return the new entity or NULL, if a memory error occurred.
 */
static entityType new_entity (identType id)

  {
    entityType created_entity;

  /* new_entity */
    logFunction(printf("new_entity\n"););
    if (ALLOC_RECORD(created_entity, entityRecord, count.entity)) {
      created_entity->ident = id;
      created_entity->syobject = NULL;
      created_entity->fparam_list = NULL;
      created_entity->data.owner = NULL;
    } /* if */
    logFunction(printf("new_entity --> ");
                prot_cstri8(id_string(created_entity->ident));
                printf("\n"););
    return created_entity;
  } /* new_entity */



void free_params (progType currentProg, listType params)

  {
    listType param_elem;
    objectType param;

  /* free_params */
    logFunction(printf("free_params\n"););
    param_elem = params;
    while (param_elem != NULL) {
      param = param_elem->obj;
      if (CATEGORY_OF_OBJ(param) == VALUEPARAMOBJECT ||
          CATEGORY_OF_OBJ(param) == REFPARAMOBJECT) {
        logMessage(printf("free_params " FMT_U_MEM ": ",
                          (memSizeType) param);
                   trace1(param);
                   printf("\n"););
        if (HAS_PROPERTY(param) && param->descriptor.property != currentProg->property.literal) {
          FREE_PROPERTY(param->descriptor.property);
        } /* if */
        FREE_OBJECT(param);
      } /* if */
      param_elem = param_elem->next;
    } /* while */
    free_list(params);
    logFunction(printf("free_params -->\n"););
  } /* free_params */



/**
 *  Copy formal value and reference parameter objects.
 *  The parameters are allocated first in a list and later assigned.
 *  This way everything can be cleaned up and name_list is unchanged,
 *  if a memory error occurs.
 */
static errInfoType copy_params (listType name_list)

  {
    listType name_elem;
    objectType param_obj;
    objectType copied_param;
    listType copied_param_list = NULL;
    listType *list_insert_place;
    listType curr_param;
    errInfoType err_info = OKAY_NO_ERROR;

  /* copy_params */
    logFunction(printf("copy_params\n"););
    list_insert_place = &copied_param_list;
    name_elem = name_list;
    while (name_elem != NULL && err_info == OKAY_NO_ERROR) {
      if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
        param_obj = name_elem->obj->value.objValue;
        if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
            CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT) {
          if (ALLOC_OBJECT(copied_param)) {
            INIT_CATEGORY_OF_OBJ(copied_param, CATEGORY_OF_OBJ(param_obj));
            COPY_VAR_FLAG(copied_param, param_obj);
            copied_param->type_of = param_obj->type_of;
            copied_param->descriptor.property = NULL;
            copied_param->value.objValue = NULL;
            list_insert_place = append_element_to_list(list_insert_place,
                                                       copied_param, &err_info);
            if (err_info != OKAY_NO_ERROR) {
              FREE_OBJECT(copied_param);
            } /* if */
          } else {
            err_info = MEMORY_ERROR;
          } /* if */
        } /* if */
      } /* if */
      name_elem = name_elem->next;
    } /* while */
    if (err_info == OKAY_NO_ERROR) {
      curr_param = copied_param_list;
      name_elem = name_list;
      while (name_elem != NULL) {
        if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
          param_obj = name_elem->obj->value.objValue;
          if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
              CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT) {
            name_elem->obj->value.objValue = curr_param->obj;
            curr_param = curr_param->next;
          } /* if */
        } /* if */
        name_elem = name_elem->next;
      } /* while */
    } else {
      curr_param = copied_param_list;
      while (curr_param != NULL) {
        FREE_OBJECT(curr_param->obj);
        curr_param = curr_param->next;
      } /* while */
    } /* if */
    free_list(copied_param_list);
    logFunction(printf("copy_params --> %d\n", err_info););
    return err_info;
  } /* copy_params */



static void pop_name_list (nodeType declaration_base, listType name_list)

  {
    listType name_elem;
    objectType param_obj;
    nodeType curr_node;

  /* pop_name_list */
    logFunction(printf("pop_name_list\n"););
    name_elem = name_list;
    curr_node = declaration_base;
    while (name_elem != NULL && curr_node != NULL) {
      if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
        param_obj = name_elem->obj->value.objValue;
        if (CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT && VAR_OBJECT(param_obj)) {
          curr_node = pop_node(curr_node->inout_param, param_obj->type_of->match_obj);
        } else if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
                   CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT) {
          curr_node = pop_node(curr_node->other_param, param_obj->type_of->match_obj);
        } else if (CATEGORY_OF_OBJ(param_obj) == TYPEOBJECT) {
          curr_node = pop_node(curr_node->attr, param_obj->value.typeValue->match_obj);
        } else if (CATEGORY_OF_OBJ(param_obj) == FORMPARAMOBJECT) {
          curr_node = pop_node(curr_node->attr, param_obj);
        } else {
          curr_node = pop_node(curr_node->symbol, param_obj);
        } /* if */
      } else {
        curr_node = pop_node(curr_node->symbol, name_elem->obj);
      } /* if */
      name_elem = name_elem->next;
    } /* while */
    logFunction(printf("pop_name_list -->\n"););
  } /* pop_name_list */



/**
 *  Searches declaration_base for a node matching name_list.
 *  If a node is found it is returned. If no matching node is
 *  found new nodes are generated and entered into declaration_base.
 *  In this case the new node at the bottom is returned.
 *  @return the node found or the new generated node or
 *          NULL, if a memory error occurred.
 */
static nodeType get_entity_node (nodeType declaration_base, listType name_list)

  {
    listType name_elem;
    objectType param_obj;
    nodeType curr_node;

  /* get_entity_node */
    logFunction(printf("get_entity_node(");
                prot_list(name_list);
                printf(")\n"););
    name_elem = name_list;
    curr_node = declaration_base;
    while (name_elem != NULL && curr_node != NULL) {
      if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
        param_obj = name_elem->obj->value.objValue;
        if (CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT && VAR_OBJECT(param_obj)) {
          curr_node = get_node(&curr_node->inout_param, param_obj->type_of->match_obj);
        } else if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
                   CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT) {
          curr_node = get_node(&curr_node->other_param, param_obj->type_of->match_obj);
        } else if (CATEGORY_OF_OBJ(param_obj) == TYPEOBJECT) {
          curr_node = get_node(&curr_node->attr, param_obj->value.typeValue->match_obj);
        } else if (CATEGORY_OF_OBJ(param_obj) == FORMPARAMOBJECT) {
          curr_node = get_node(&curr_node->attr, param_obj);
        } else {
          curr_node = get_node(&curr_node->symbol, param_obj);
        } /* if */
      } else {
        curr_node = get_node(&curr_node->symbol, name_elem->obj);
      } /* if */
      name_elem = name_elem->next;
    } /* while */
    logFunction(printf("get_entity_node --> ");
                if (curr_node != NULL) {
                  trace1(curr_node->match_obj);
                  printf("\n");
                } else {
                  printf("*NULL_NODE*\n");
                });
    return curr_node;
  } /* get_entity_node */



/**
 *  Searches declaration_base for an entity matching name_list.
 *  If an entity is found it is returned. If no matching entity is
 *  found a new entity is generated and entered into declaration_base.
 *  In this case the new entity is returned.
 *  @return the entity found or the new generated entity or
 *          NULL, if a memory error occurred.
 */
entityType get_entity (nodeType declaration_base, listType name_list)

  {
    nodeType node_found;
    entityType entity_found;

  /* get_entity */
    logFunction(printf("get_entity(");
                prot_list(name_list);
                printf(")\n"););
    node_found = get_entity_node(declaration_base, name_list);
    if (node_found == NULL) {
      /* A memory error occurred. */
      pop_name_list(declaration_base, name_list);
      entity_found = NULL;
    } else if (node_found->entity != NULL) {
      entity_found = node_found->entity;
    } else {
      if ((entity_found = new_entity(NULL)) == NULL) {
        pop_name_list(declaration_base, name_list);
      } else {
        if (copy_params(name_list) != OKAY_NO_ERROR) {
          pop_name_list(declaration_base, name_list);
          FREE_RECORD(entity_found, entityRecord, count.entity);
          entity_found = NULL;
        } else {
          entity_found->fparam_list = name_list;
          node_found->entity = entity_found;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("get_entity -->\n");
                trace_entity(entity_found););
    return entity_found;
  } /* get_entity */



/**
 *  Searches declaration_base for an entity matching name_list.
 *  @return the entity found or NULL if no matching entity is found.
 */
entityType find_entity (nodeType declaration_base, listType name_list)

  {
    listType name_elem;
    objectType param_obj;
    nodeType curr_node;
    entityType entity_found;

  /* find_entity */
    logFunction(printf("find_entity(");
                prot_list(name_list);
                printf(")\n"););
    name_elem = name_list;
    curr_node = declaration_base;
    while (name_elem != NULL && curr_node != NULL) {
      if (CATEGORY_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
        param_obj = name_elem->obj->value.objValue;
        if (CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT && VAR_OBJECT(param_obj)) {
          curr_node = find_node(curr_node->inout_param, param_obj->type_of->match_obj);
        } else if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
                   CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT) {
          curr_node = find_node(curr_node->other_param, param_obj->type_of->match_obj);
        } else if (CATEGORY_OF_OBJ(param_obj) == TYPEOBJECT) {
          curr_node = find_node(curr_node->attr, param_obj->value.typeValue->match_obj);
        } else if (CATEGORY_OF_OBJ(param_obj) == FORMPARAMOBJECT) {
          curr_node = find_node(curr_node->attr, param_obj);
        } else {
          curr_node = find_node(curr_node->symbol, param_obj);
        } /* if */
      } else {
        curr_node = find_node(curr_node->symbol, name_elem->obj);
      } /* if */
      name_elem = name_elem->next;
    } /* while */
    if (curr_node != NULL) {
      entity_found = curr_node->entity;
    } else {
      entity_found = NULL;
    } /* if */
    logFunction(printf("find_entity -->\n");
                trace_entity(entity_found););
    return entity_found;
  } /* find_entity */



entityType search_entity (const_nodeType start_node, const_listType name_list)

  {
    objectType param_obj;
    nodeType node_found;
    typeType object_type;
    entityType entity_found;

  /* search_entity */
    logFunction(printf("search_entity(");
                prot_list(name_list);
                printf(")\n"););
    if (name_list == NULL) {
      if (start_node != NULL) {
        entity_found = start_node->entity;
      } else {
        entity_found = NULL;
      } /* if */
    } else {
      entity_found = NULL;
      if (start_node != NULL) {
        if (CATEGORY_OF_OBJ(name_list->obj) == FORMPARAMOBJECT) {
/* printf("paramobject x\n"); */
          param_obj = name_list->obj->value.objValue;
          if (CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT && VAR_OBJECT(param_obj)) {
/* printf("inout param ");
trace1(param_obj);
printf("\n"); */
            object_type = param_obj->type_of;
            do {
              node_found = find_node(start_node->inout_param, object_type->match_obj);
              if (node_found != NULL) {
                entity_found = search_entity(node_found, name_list->next);
              } /* if */
              object_type = object_type->meta;
            } while (object_type != NULL && entity_found == NULL);
          } else if (CATEGORY_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
                     CATEGORY_OF_OBJ(param_obj) == REFPARAMOBJECT) {
/* printf("value or ref param ");
trace1(param_obj);
printf("\n"); */
            object_type = param_obj->type_of;
            do {
              node_found = find_node(start_node->other_param, object_type->match_obj);
              if (node_found != NULL) {
                entity_found = search_entity(node_found, name_list->next);
              } /* if */
              object_type = object_type->meta;
            } while (object_type != NULL && entity_found == NULL);
          } else if (CATEGORY_OF_OBJ(param_obj) == TYPEOBJECT) {
/* printf("attr param ");
trace1(param_obj);
printf("\n"); */
            object_type = param_obj->value.typeValue;
            do {
              node_found = find_node(start_node->attr, object_type->match_obj);
              if (node_found != NULL) {
                entity_found = search_entity(node_found, name_list->next);
              } /* if */
              object_type = object_type->meta;
            } while (object_type != NULL && entity_found == NULL);
          } else {
/* printf("symbol param ");
trace1(param_obj);
printf("\n"); */
            node_found = find_node(start_node->symbol, param_obj);
            if (node_found != NULL) {
              entity_found = search_entity(node_found, name_list->next);
            } /* if */
          } /* if */
        } else {
/* printf("symbol param ");
trace1(name_list->obj);
printf("\n"); */
          node_found = find_node(start_node->symbol, name_list->obj);
          if (node_found != NULL) {
            entity_found = search_entity(node_found, name_list->next);
          } /* if */
/* printf("after symbol param\n"); */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("search_entity ->\n"););
    return entity_found;
  } /* search_entity */



void pop_entity (nodeType declaration_base, const_entityType entity)

  { /* pop_entity */
    logFunction(printf("pop_entity\n"););
    /* trace_entity(entity); */
    pop_name_list(declaration_base, entity->fparam_list);
    logFunction(printf("pop_entity -->\n"););
  } /* pop_entity */



void close_entity (progType currentProg)

  {
    entityType entity;
    entityType old_entity;

  /* close_entity */
    logFunction(printf("close_entity\n"););
    entity = currentProg->entity.inactive_list;
    while (entity != NULL) {
      old_entity = entity;
      entity = entity->data.next;
      free_entity(currentProg, old_entity);
    } /* while */
    currentProg->entity.inactive_list = NULL;
    logFunction(printf("close_entity -->\n"););
  } /* close_entity */



void init_entity (progType aProg, errInfoType *err_info)

  { /* init_entity */
    logFunction(printf("init_entity\n"););
    aProg->entity.inactive_list = NULL;
    if ((aProg->entity.literal = new_entity(aProg->ident.literal)) == NULL) {
      *err_info = MEMORY_ERROR;
    } /* if */
    if (!ALLOC_PROPERTY(aProg->property.literal)) {
      *err_info = MEMORY_ERROR;
    } else {
      aProg->property.literal->entity = aProg->entity.literal;
      aProg->property.literal->params = NULL;
      aProg->property.literal->file_number = 0;
      aProg->property.literal->line = 0;
      aProg->property.literal->syNumberInLine = 0;
    } /* if */
    logFunction(printf("init_entity -->\n"););
  } /* init_entity */
