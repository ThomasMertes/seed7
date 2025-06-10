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
/*  Content: Functions to maintain objects of type entityType.      */
/*                                                                  */
/********************************************************************/

nodeType find_node (register nodeType node_tree,
    register objectType object_searched);
void init_declaration_root (progType currentProg, errInfoType *err_info);
void close_declaration_root (progType currentProg);
void free_entity (const_progType currentProg, entityType old_entity);
void free_params (progType currentProg, listType params);
entityType get_entity (nodeType declaration_base, listType name_list);
entityType find_entity (nodeType declaration_base, listType name_list);
entityType search_entity (const_nodeType start_node, const_listType name_list);
void pop_entity (nodeType declaration_base, const_entityType entity);
void close_entity (progType currentProg);
void init_entity (progType aProg, errInfoType *err_info);
