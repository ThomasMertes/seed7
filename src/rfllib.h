/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2008  Thomas Mertes                        */
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
/*  Module: Library                                                 */
/*  File: seed7/src/rfllib.h                                        */
/*  Changes: 1991, 1992, 1993, 1994, 1995  Thomas Mertes            */
/*  Content: All primitive actions for the ref_list type.           */
/*                                                                  */
/********************************************************************/

objectType rfl_append    (listType arguments);
objectType rfl_cat       (listType arguments);
objectType rfl_cpy       (listType arguments);
objectType rfl_create    (listType arguments);
objectType rfl_destr     (listType arguments);
objectType rfl_elem      (listType arguments);
objectType rfl_elemcpy   (listType arguments);
objectType rfl_empty     (listType arguments);
objectType rfl_eq        (listType arguments);
objectType rfl_excl      (listType arguments);
objectType rfl_expr      (listType arguments);
objectType rfl_for       (listType arguments);
objectType rfl_for_until (listType arguments);
objectType rfl_head      (listType arguments);
objectType rfl_idx       (listType arguments);
objectType rfl_incl      (listType arguments);
objectType rfl_ipos      (listType arguments);
objectType rfl_lng       (listType arguments);
objectType rfl_mklist    (listType arguments);
objectType rfl_ne        (listType arguments);
objectType rfl_not_elem  (listType arguments);
objectType rfl_pos       (listType arguments);
objectType rfl_range     (listType arguments);
objectType rfl_set_value (listType arguments);
objectType rfl_tail      (listType arguments);
objectType rfl_trace     (listType arguments);
objectType rfl_value     (listType arguments);
