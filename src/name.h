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
/*  Module: Analyzer                                                */
/*  File: seed7/src/name.h                                          */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Enter an object in a specified declaration level.      */
/*                                                                  */
/********************************************************************/

void disconnect_param_entities (const const_objectType objWithParams);
void init_stack (progType currentProg, errInfoType *err_info);
void close_stack (progType currentProg);
void grow_stack (errInfoType *err_info);
void shrink_stack (void);
void push_stack (void);
void pop_stack (void);
listType *get_local_object_insert_place (void);
void pop_object_list (listType list_element);
objectType entername (nodeType declaration_base, objectType object_name,
    errInfoType *err_info);
objectType find_name (nodeType declaration_base, const_objectType object_name,
    errInfoType *err_info);
objectType search_name (const_nodeType declaration_base,
    const_objectType object_name, errInfoType *err_info);
objectType dollar_entername (nodeType declaration_base, objectType object_name,
    errInfoType *err_info);
