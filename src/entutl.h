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
/*  Module: General                                                 */
/*  File: seed7/src/endutl.h                                        */
/*  Changes: 2000  Thomas Mertes                                    */
/*  Content: Procedures to maintain objects of type entitytype.     */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

nodetype find_node (register nodetype node_tree,
    register objecttype object_searched);
void init_declaration_root (progtype currentProg, errinfotype *err_info);
void close_declaration_root (progtype currentProg);
void free_entity (const_progtype currentProg, entitytype old_entity);
entitytype get_entity (nodetype declaration_base, listtype name_list);
entitytype find_entity (nodetype declaration_base, listtype name_list);
entitytype search_entity (const_nodetype start_node, const_listtype name_list);
void pop_entity (nodetype declaration_base, const_entitytype entity);
void close_entity (progtype currentProg);
void init_entity (errinfotype *err_info);

#else

nodetype find_node ();
void init_declaration_root ();
void close_declaration_root ();
void free_entity ();
entitytype get_entity ();
entitytype find_entity ();
entitytype search_entity ();
void pop_entity ();
void close_entity ();
void init_entity ();

#endif
