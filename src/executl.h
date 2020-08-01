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
/*  Module: Interpreter                                             */
/*  File: seed7/src/executl.h                                       */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Initalisation operation procedures used at runtime.    */
/*                                                                  */
/********************************************************************/

objecttype get_create_call_obj (objecttype obj, errinfotype *err_info);
objecttype get_destroy_call_obj (objecttype obj, errinfotype *err_info);
void do_create (objecttype destination, objecttype source,
    errinfotype *err_info);
void do_destroy (objecttype old_obj, errinfotype *err_info);
inttype do_ord (objecttype any_obj, errinfotype *err_info);
booltype do_in (objecttype elem_obj, objecttype set_obj,
    errinfotype *err_info);
objecttype param1_call (objecttype function_obj, objecttype param1);
objecttype param2_call (objecttype function_obj, objecttype param1,
    objecttype param2);
objecttype param3_call (objecttype function_obj, objecttype param1,
    objecttype param2, objecttype param3);
objecttype create_return_object (const_locobjtype local, objecttype init_value,
    errinfotype *err_info);
void create_local_object (const_locobjtype local, objecttype init_value,
    errinfotype *err_info);
void destroy_local_object (const_locobjtype local, errinfotype *err_info);
void destroy_local_init_value (const_locobjtype local, errinfotype *err_info);
void destr_struct (objecttype old_elem, memsizetype old_size);
booltype crea_struct (objecttype elem_to, objecttype elem_from,
    memsizetype new_size);
booltype arr_elem_initialisation (typetype dest_type, objecttype obj_to,
    objecttype obj_from);
void destr_array (objecttype old_elem, memsizetype old_size);
booltype crea_array (objecttype elem_to, objecttype elem_from,
    memsizetype new_size);
void cpy_array (objecttype elem_to, objecttype elem_from,
    memsizetype new_size);
