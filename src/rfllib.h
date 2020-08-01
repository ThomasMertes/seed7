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

objecttype rfl_append    (listtype arguments);
objecttype rfl_cat       (listtype arguments);
objecttype rfl_cpy       (listtype arguments);
objecttype rfl_create    (listtype arguments);
objecttype rfl_destr     (listtype arguments);
objecttype rfl_elem      (listtype arguments);
objecttype rfl_elemcpy   (listtype arguments);
objecttype rfl_empty     (listtype arguments);
objecttype rfl_eq        (listtype arguments);
objecttype rfl_excl      (listtype arguments);
objecttype rfl_expr      (listtype arguments);
objecttype rfl_for       (listtype arguments);
objecttype rfl_for_until (listtype arguments);
objecttype rfl_head      (listtype arguments);
objecttype rfl_idx       (listtype arguments);
objecttype rfl_incl      (listtype arguments);
objecttype rfl_ipos      (listtype arguments);
objecttype rfl_lng       (listtype arguments);
objecttype rfl_mklist    (listtype arguments);
objecttype rfl_ne        (listtype arguments);
objecttype rfl_not_elem  (listtype arguments);
objecttype rfl_pos       (listtype arguments);
objecttype rfl_range     (listtype arguments);
objecttype rfl_setvalue  (listtype arguments);
objecttype rfl_tail      (listtype arguments);
objecttype rfl_trace     (listtype arguments);
objecttype rfl_value     (listtype arguments);
