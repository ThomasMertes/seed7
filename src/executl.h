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
/*  Content: Initialization operation functions used at runtime.    */
/*                                                                  */
/********************************************************************/

objectType get_create_call_obj (objectType obj, errInfoType *err_info);
objectType get_destroy_call_obj (objectType obj, errInfoType *err_info);
void do_create (objectType destination, objectType source,
    errInfoType *err_info);
void do_destroy (objectType old_obj, errInfoType *err_info);
intType do_ord (objectType any_obj, errInfoType *err_info);
boolType do_in (objectType elem_obj, objectType set_obj,
    errInfoType *err_info);
objectType getValue (objectType type_obj);
objectType param1_call (objectType function_obj, objectType param1);
objectType param2_call (objectType function_obj, objectType param1,
    objectType param2);
objectType param3_call (objectType function_obj, objectType param1,
    objectType param2, objectType param3);
objectType create_return_object (const_locObjType local, objectType init_value,
    errInfoType *err_info);
void create_local_object (const_locObjType local, objectType init_value,
    errInfoType *err_info);
void destroy_local_object (const_locObjType local, boolType ignoreError);
void destroy_local_init_value (const_locObjType local, errInfoType *err_info);
void destr_struct (objectType old_elem, memSizeType old_size);
boolType crea_struct (objectType elem_to, objectType elem_from,
    memSizeType new_size);
void destr_interface (objectType old_value);
boolType arr_elem_initialisation (typeType dest_type, objectType obj_to,
    objectType obj_from);
void destr_array (objectType old_elem, memSizeType old_size);
void free_array (arrayType old_arr);
boolType crea_array (objectType elem_to, objectType elem_from,
    memSizeType new_size);
void cpy_array (objectType elem_to, objectType elem_from,
    memSizeType new_size);
