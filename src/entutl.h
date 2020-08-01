/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: General                                                 */
/*  File: seed7/src/endutl.h                                        */
/*  Changes: 2000  Thomas Mertes                                    */
/*  Content: Procedures to maintain objects of type entitytype.     */
/*                                                                  */
/********************************************************************/

typedef struct entrootstruct {
    entitytype literal;
  } entroottype;

EXTERN entroottype entity;

#ifdef ANSI_C

nodetype find_node (nodetype, objecttype);
void init_declaration_root (progtype, errinfotype *);
void close_declaration_root (progtype);
void free_entity (entitytype);
entitytype get_entity (nodetype, listtype);
entitytype find_entity (nodetype, listtype);
entitytype search_entity (nodetype, listtype);
void pop_entity (nodetype, entitytype);
void init_entity (errinfotype *);

#else

nodetype find_node ();
void init_declaration_root ();
void close_declaration_root ();
void free_entity ();
entitytype get_entity ();
entitytype find_entity ();
entitytype search_entity ();
void pop_entity ();
void init_entity ();

#endif
