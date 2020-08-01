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

#ifdef ANSI_C

void disconnect_param_entities (const const_objecttype objWithParams);
listtype create_parameter_list (listtype name_list, errinfotype *err_info);
void init_stack (progtype currentProg, errinfotype *err_info);
void close_stack (const_progtype currentProg);
void grow_stack (errinfotype *err_info);
void shrink_stack (void);
void push_stack (void);
void pop_stack (void);
listtype *get_local_object_insert_place (void);
objecttype entername (nodetype declaration_base, objecttype object_name,
    errinfotype *err_info);
objecttype find_name (nodetype declaration_base, const_objecttype object_name,
    errinfotype *err_info);
objecttype search_name (const_nodetype declaration_base,
    const_objecttype object_name, errinfotype *err_info);
objecttype dollar_entername (nodetype declaration_base, objecttype object_name,
    errinfotype *err_info);

#else

void disconnect_param_entities ();
listtype create_parameter_list ();
void init_stack ();
void close_stack ();
void grow_stack ();
void shrink_stack ();
void push_stack ();
void pop_stack ();
listtype *get_local_object_insert_place ();
objecttype entername ();
objecttype find_name ();
objecttype search_name ();
objecttype dollar_entername ();

#endif
