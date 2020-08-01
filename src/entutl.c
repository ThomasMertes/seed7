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
/*  Module: General                                                 */
/*  File: seed7/src/entutl.c                                        */
/*  Changes: 2000  Thomas Mertes                                    */
/*  Content: Procedures to maintain objects of type entitytype.     */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "traceutl.h"
#include "flistutl.h"
#include "identutl.h"

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
  /* The K&R standard allowes pointer comparisons only inside of    */
  /* arrays. So the true less comparison does not conform to the    */
  /* standard. The macro below converts the pointers to long so it  */
  /* is confoming to the standard. Taking only the lower bits for   */
  /* the comparison has a "random" effect for the binary tree.      */
  /* This speeds the tree up a measurable amount of time.           */

#define PTR_LESS(P1,P2) (((long) (P1) & 0377L) < ((long) (P2) & 0377L))

/* #define PTR_LESS(P1,P2) (((long) (P1) & 0177400L) < ((long) (P2) & 0177400L)) */
/* #define PTR_LESS(P1,P2) ((P1) < (P2)) */


#undef TRACE_ENTITY



#ifdef ANSI_C

static void free_nodes (nodetype node)
#else

static void free_nodes (node)
nodetype node;
#endif

  { /* free_nodes */
#ifdef TRACE_ENTITY
    printf("BEGIN free_nodes(");
    printnodes(node);
    printf(")\n");
#endif
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
#ifdef TRACE_ENTITY
    printf("END free_nodes\n");
#endif
  } /* free_nodes */



#ifdef ANSI_C

static nodetype new_node (objecttype obj)
#else

static nodetype new_node (obj)
objecttype obj;
#endif

  {
    nodetype created_node;

  /* new_node */
#ifdef TRACE_ENTITY
    printf("BEGIN new_node(");
    trace1(obj);
    printf(")\n");
#endif
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
#ifdef TRACE_ENTITY
    printf("END new_node\n");
#endif
    return(created_node);
  } /* new_node */



#ifdef ANSI_C

static nodetype get_node (nodetype *node_tree,
    register objecttype object_searched)
#else

static nodetype get_node (node_tree, object_searched)
nodetype *node_tree;
register objecttype object_searched;
#endif

  /* Searches for a node matching the object object_searched in the */
  /* specified node_tree. If a node is found it is returned. If no  */
  /* matching node is found a new node is generated and entered     */
  /* into the node_tree. In this case the new node is returned.     */
  /* If a memory error occours NULL is returned.                    */

  {
    register nodetype search_node;
    register booltype searching;
    nodetype node_found;

  /* get_node */
#ifdef TRACE_ENTITY
    printf("BEGIN get_node({ ");
    printnodes(*node_tree);
    printf("}, ");
    trace1(object_searched);
    printf(")\n");
#endif
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
#ifdef TRACE_ENTITY
    printf("END get_node ==> ");
    trace_node(node_found);
    printf("\n");
#endif
    return(node_found);
  } /* get_node */



#ifdef ANSI_C

nodetype find_node (register nodetype node_tree,
    register objecttype object_searched)
#else

nodetype find_node (node_tree, object_searched)
register nodetype node_tree;
register objecttype object_searched;
#endif

  /* Searches for a node matching the object object_searched in the */
  /* specified node_tree. If a node is found it is returned. If no  */
  /* matching node is found NULL is returned.                       */

  {
    nodetype node_found;

  /* find_node */
#ifdef TRACE_ENTITY
    printf("BEGIN find_node({ ");
    printnodes(node_tree);
    printf("}, ");
    trace1(object_searched);
    prot_cstri("=");
    prot_int((inttype) object_searched);
    printf(")\n");
#endif
    node_found = NULL;
    while (node_tree != NULL) {
      if (PTR_LESS(object_searched, node_tree->match_obj)) {
        node_tree = node_tree->next1;
      } else if (object_searched == node_tree->match_obj) {
        if (node_tree->usage_count > 0) {
          node_found = node_tree;
        } /* if */
        node_tree = NULL;
      } else {    /* (object_searched > node_tree->match_obj) */
        node_tree = node_tree->next2;
      } /* if */
    } /* while */
/*  printf("%s\n", (node_found != NULL ? "found" : "not found")); */
#ifdef TRACE_ENTITY
    printf("END find_node --> ");
    if (node_found != NULL) {
      trace1(node_found->match_obj);
      printf("\n");
    } else {
      printf("*NULL_NODE*\n");
    } /* if */
#endif
    return(node_found);
  } /* find_node */



#ifdef ANSI_C

static nodetype pop_node (register nodetype node_tree,
    register objecttype object_searched)
#else

static nodetype pop_node (node_tree, object_searched)
register nodetype node_tree;
register objecttype object_searched;
#endif

  /* Searches for a node matching the object object_searched in the */
  /* specified node_tree. If a node is found it is returned. If no  */
  /* matching node is found NULL is returned.                       */

  {
    nodetype node_found;

  /* pop_node */
#ifdef TRACE_ENTITY
    printf("BEGIN pop_node({ ");
    printnodes(node_tree);
    printf("}, ");
    trace1(object_searched);
    prot_cstri("=");
    prot_int((inttype) object_searched);
    printf(")\n");
#endif
    node_found = NULL;
    while (node_tree != NULL) {
      if (PTR_LESS(object_searched, node_tree->match_obj)) {
        node_tree = node_tree->next1;
      } else if (object_searched == node_tree->match_obj) {
        if (node_tree->usage_count > 0) {
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
/*  printf("%s\n", (node_found != NULL ? "found" : "not found")); */
#ifdef TRACE_ENTITY
    printf("END pop_node --> ");
    if (node_found != NULL) {
      trace1(node_found->match_obj);
      printf("\n");
    } else {
      printf("*NULL_NODE*\n");
    } /* if */
#endif
    return(node_found);
  } /* pop_node */



#ifdef ANSI_C

void init_declaration_root (progtype currentProg, errinfotype *err_info)
#else

void init_declaration_root (currentProg, err_info)
progtype currentProg;
errinfotype *err_info;
#endif

  { /* init_declaration_root */
#ifdef TRACE_ENTITY
    printf("BEGIN init_declaration_root\n");
#endif
    currentProg->declaration_root = new_node(NULL);
#ifdef TRACE_ENTITY
    printf("END init_declaration_root\n");
#endif
  } /* init_declaration_root */



#ifdef ANSI_C

void close_declaration_root (progtype currentProg)
#else

void close_declaration_root (currentProg)
progtype currentProg;
#endif

  { /* close_declaration_root */
#ifdef TRACE_ENTITY
    printf("BEGIN close_declaration_root\n");
#endif
    free_nodes(currentProg->declaration_root);
    currentProg->declaration_root = NULL;
#ifdef TRACE_ENTITY
    printf("END close_declaration_root\n");
#endif
  } /* close_declaration_root */



#ifdef ANSI_C

void free_entity (entitytype old_entity)
#else

void free_entity (old_entity)
entitytype old_entity;
#endif

  { /* free_entity */
#ifdef TRACE_ENTITY
    printf("BEGIN free_entity\n");
#endif
    if (old_entity != NULL) {
      if (old_entity->owner != NULL) {
        printf("entity has owner\n");
      } /* if */
      FREE_RECORD(old_entity, entityrecord, count.entity);
    } /* if */
#ifdef TRACE_ENTITY
    printf("END free_entity\n");
#endif
  } /* free_entity */



#ifdef ANSI_C

static entitytype new_entity (identtype id)
#else

static entitytype new_entity (id)
identtype id;
#endif

  {
    entitytype created_entity;

  /* new_entity */
#ifdef TRACE_ENTITY
    printf("BEGIN new_entity\n");
#endif
    if (ALLOC_RECORD(created_entity, entityrecord, count.entity)) {
      created_entity->ident = id;
      created_entity->syobject = NULL;
      created_entity->name_list = NULL;
      created_entity->owner = NULL;
    } /* if */
#ifdef TRACE_ENTITY
    printf("END new_entity --> ");
    prot_cstri(id_string(created_entity->ident));
    printf("\n");
#endif
    return(created_entity);
  } /* new_entity */



#ifdef ANSI_C

entitytype get_entity (nodetype declaration_base, listtype name_list)
#else

entitytype get_entity (declaration_base, name_list)
nodetype declaration_base;
listtype name_list;
#endif

  {
    listtype name_elem;
    objecttype param_obj;
    nodetype curr_node;
    entitytype entity_found;

  /* get_entity */
#ifdef TRACE_ENTITY
    printf("BEGIN get_entity(");
    prot_list(name_list);
    printf(")\n");
#endif
    name_elem = name_list;
    curr_node = declaration_base;
    while (name_elem != NULL && curr_node != NULL) {
      if (CLASS_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
/* printf("paramobject x\n"); */
        param_obj = name_elem->obj->value.objvalue;
        if (CLASS_OF_OBJ(param_obj) == REFPARAMOBJECT && VAR_OBJECT(param_obj)) {
/* printf("inout param ");
trace1(param_obj);
printf("\n"); */
          curr_node = get_node(&curr_node->inout_param, param_obj->type_of->match_obj);
        } else if (CLASS_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
            CLASS_OF_OBJ(param_obj) == REFPARAMOBJECT) {
/* printf("value or ref param ");
trace1(param_obj);
printf("\n"); */
          curr_node = get_node(&curr_node->other_param, param_obj->type_of->match_obj);
        } else if (CLASS_OF_OBJ(param_obj) == TYPEOBJECT) {
/* printf("attr param ");
trace1(param_obj);
printf("\n"); */
          curr_node = get_node(&curr_node->attr, param_obj->value.typevalue->match_obj);
        } else {
/* printf("symbol param ");
trace1(param_obj);
printf("\n"); */
          curr_node = get_node(&curr_node->symbol, param_obj);
        } /* if */
      } else {
/* printf("symbol param ");
trace1(name_elem->obj);
printf("\n"); */
        curr_node = get_node(&curr_node->symbol, name_elem->obj);
/* printf("after symbol param\n"); */
      } /* if */
      name_elem = name_elem->next;
    } /* while */
    if (curr_node == NULL) {
      entity_found = NULL;
    } else {
      if (curr_node->entity == NULL) {
        if ((entity_found = new_entity(NULL)) != NULL) {
          entity_found->name_list = name_list;
          curr_node->entity = entity_found;
        } /* if */
      } else {
        entity_found = curr_node->entity;
      } /* if */
    } /* if */
#ifdef TRACE_ENTITY
    printf("END get_entity\n");
#endif
    return(entity_found);
  } /* get_entity */



#ifdef ANSI_C

entitytype find_entity (nodetype declaration_base, listtype name_list)
#else

entitytype find_entity (declaration_base, name_list)
nodetype declaration_base;
listtype name_list;
#endif

  {
    listtype name_elem;
    objecttype param_obj;
    nodetype curr_node;
    entitytype entity_found;

  /* find_entity */
#ifdef TRACE_ENTITY
    printf("BEGIN find_entity(");
    prot_list(name_list);
    printf(")\n");
#endif
    name_elem = name_list;
    curr_node = declaration_base;
    while (name_elem != NULL && curr_node != NULL) {
      if (CLASS_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
/* printf("paramobject x\n"); */
        param_obj = name_elem->obj->value.objvalue;
        if (CLASS_OF_OBJ(param_obj) == REFPARAMOBJECT && VAR_OBJECT(param_obj)) {
/* printf("inout param ");
trace1(param_obj);
printf("\n"); */
          curr_node = find_node(curr_node->inout_param, param_obj->type_of->match_obj);
        } else if (CLASS_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
            CLASS_OF_OBJ(param_obj) == REFPARAMOBJECT) {
/* printf("value or ref param ");
trace1(param_obj);
printf("\n"); */
          curr_node = find_node(curr_node->other_param, param_obj->type_of->match_obj);
        } else if (CLASS_OF_OBJ(param_obj) == TYPEOBJECT) {
/* printf("attr param ");
trace1(param_obj);
printf("\n"); */
          curr_node = find_node(curr_node->attr, param_obj->value.typevalue->match_obj);
        } else {
/* printf("symbol param ");
trace1(param_obj);
printf("\n"); */
          curr_node = find_node(curr_node->symbol, param_obj);
        } /* if */
      } else {
/* printf("symbol param ");
trace1(name_elem->obj);
printf("\n"); */
        curr_node = find_node(curr_node->symbol, name_elem->obj);
/* printf("after symbol param\n"); */
      } /* if */
      name_elem = name_elem->next;
    } /* while */
    if (curr_node != NULL) {
      entity_found = curr_node->entity;
    } else {
      entity_found = NULL;
    } /* if */
#ifdef TRACE_ENTITY
    printf("END find_entity\n");
#endif
    return(entity_found);
  } /* find_entity */



#ifdef ANSI_C

entitytype search_entity (nodetype start_node, listtype name_list)
#else

entitytype search_entity (start_node, name_list)
nodetype start_node;
listtype name_list;
#endif

  {
    objecttype param_obj;
    nodetype node_found;
    typetype object_type;
    entitytype entity_found;

  /* search_entity */
#ifdef TRACE_ENTITY
    printf("BEGIN search_entity(");
    prot_list(name_list);
    printf(")\n");
#endif
    if (name_list == NULL) {
      if (start_node != NULL) {
        entity_found = start_node->entity;
      } else {
        entity_found = NULL;
      } /* if */
    } else {
      entity_found = NULL;
      if (start_node != NULL) {
        if (CLASS_OF_OBJ(name_list->obj) == FORMPARAMOBJECT) {
/* printf("paramobject x\n"); */
          param_obj = name_list->obj->value.objvalue;
          if (CLASS_OF_OBJ(param_obj) == REFPARAMOBJECT && VAR_OBJECT(param_obj)) {
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
          } else if (CLASS_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
              CLASS_OF_OBJ(param_obj) == REFPARAMOBJECT) {
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
          } else if (CLASS_OF_OBJ(param_obj) == TYPEOBJECT) {
/* printf("attr param ");
trace1(param_obj);
printf("\n"); */
            object_type = param_obj->value.typevalue;
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
#ifdef TRACE_ENTITY
    printf("END search_entity\n");
#endif
    return(entity_found);
  } /* search_entity */



#ifdef ANSI_C

void pop_entity (nodetype declaration_base, entitytype ent)
#else

void pop_entity (declaration_base, ent)
nodetype declaration_base;
entitytype ent;
#endif

  {
    listtype name_elem;
    objecttype param_obj;
    nodetype curr_node;

  /* pop_entity */
#ifdef TRACE_ENTITY
    printf("BEGIN pop_entity\n");
#endif
    name_elem = ent->name_list;
    if (name_elem != NULL) {
      curr_node = declaration_base;
      while (name_elem != NULL && curr_node != NULL) {
        if (CLASS_OF_OBJ(name_elem->obj) == FORMPARAMOBJECT) {
          param_obj = name_elem->obj->value.objvalue;
          if (CLASS_OF_OBJ(param_obj) == REFPARAMOBJECT && VAR_OBJECT(param_obj)) {
            curr_node = pop_node(curr_node->inout_param, param_obj->type_of->match_obj);
          } else if (CLASS_OF_OBJ(param_obj) == VALUEPARAMOBJECT ||
              CLASS_OF_OBJ(param_obj) == REFPARAMOBJECT) {
            curr_node = pop_node(curr_node->other_param, param_obj->type_of->match_obj);
          } else if (CLASS_OF_OBJ(param_obj) == TYPEOBJECT) {
            curr_node = pop_node(curr_node->attr, param_obj->value.typevalue->match_obj);
          } /* if */
        } else {
          curr_node = pop_node(curr_node->symbol, name_elem->obj);
        } /* if */
        name_elem = name_elem->next;
      } /* while */
    } /* if */
#ifdef TRACE_ENTITY
    printf("END pop_entity\n");
#endif
  } /* pop_entity */



#ifdef ANSI_C

void init_entity (errinfotype *err_info)
#else

void init_entity (err_info)
errinfotype *err_info;
#endif

  { /* init_entity */
#ifdef TRACE_ENTITY
    printf("BEGIN init_entity\n");
#endif
#ifdef OUT_OF_ORDER
    entity.literal = NULL;
#endif
    if ((entity.literal = new_entity(prog.ident.literal)) == NULL) {
      *err_info = MEMORY_ERROR;
    } /* if */
#ifdef TRACE_ENTITY
    printf("END init_entity\n");
#endif
  } /* init_entity */
