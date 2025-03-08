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
/*  File: seed7/src/blockutl.h                                      */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Functions to maintain objects of type blockType.       */
/*                                                                  */
/********************************************************************/

void free_loclist (locListType loclist);
void free_block (blockType block);
blockType new_block (locListType block_params, const_locObjType block_result,
    locListType block_local_vars, listType block_local_consts,
    objectType block_body);
void get_result_var (locObjType result_var, typeType result_type,
    objectType result_init, errInfoType *err_info);
void get_return_var (locObjType return_var, typeType return_type,
    errInfoType *err_info);
locListType get_param_list (const_listType param_object_list,
    errInfoType *err_info);
locListType get_local_var_list (const_listType local_object_list,
    errInfoType *err_info);
listType get_local_const_list (const_listType local_object_list,
    errInfoType *err_info);
