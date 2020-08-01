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
/*  Module: General                                                 */
/*  File: seed7/src/blockutl.h                                      */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Procedures to maintain objects of type blocktype.      */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

void free_block (blocktype block);
blocktype new_block (loclisttype block_params, const_locobjtype block_result,
    loclisttype block_local_vars, listtype block_local_consts,
    objecttype block_body);
void get_result_var (locobjtype result_var, typetype result_type,
    objecttype result_init, errinfotype *err_info);
void get_return_var (locobjtype return_var, typetype return_type,
    errinfotype *err_info);
loclisttype get_param_list (const_listtype param_object_list,
    errinfotype *err_info);
loclisttype get_local_var_list (const_listtype local_object_list,
    errinfotype *err_info);
listtype get_local_const_list (const_listtype local_object_list,
    errinfotype *err_info);

#else

void free_block ();
blocktype new_block ();
void get_result_var ();
void get_return_var ();
loclisttype get_param_list ();
loclisttype get_local_var_list ();
listtype get_local_const_list ();

#endif
