/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  File: seed7/src/strlib.h                                        */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: All primitive actions for the string type.             */
/*                                                                  */
/********************************************************************/

objectType str_append        (listType arguments);
objectType str_cat           (listType arguments);
objectType str_chipos        (listType arguments);
objectType str_chpos         (listType arguments);
objectType str_chsplit       (listType arguments);
objectType str_clit          (listType arguments);
objectType str_cmp           (listType arguments);
objectType str_cpy           (listType arguments);
objectType str_create        (listType arguments);
objectType str_destr         (listType arguments);
objectType str_elemcpy       (listType arguments);
objectType str_eq            (listType arguments);
objectType str_for           (listType arguments);
objectType str_for_key       (listType arguments);
objectType str_for_var_key   (listType arguments);
objectType str_from_utf8     (listType arguments);
objectType str_ge            (listType arguments);
objectType str_gt            (listType arguments);
objectType str_hashcode      (listType arguments);
objectType str_head          (listType arguments);
objectType str_idx           (listType arguments);
objectType str_ipos          (listType arguments);
objectType str_le            (listType arguments);
objectType str_lit           (listType arguments);
objectType str_lng           (listType arguments);
objectType str_low           (listType arguments);
objectType str_lpad          (listType arguments);
objectType str_lpad0         (listType arguments);
objectType str_lt            (listType arguments);
objectType str_ltrim         (listType arguments);
objectType str_mult          (listType arguments);
objectType str_ne            (listType arguments);
objectType str_pos           (listType arguments);
objectType str_poscpy        (listType arguments);
objectType str_push          (listType arguments);
objectType str_range         (listType arguments);
objectType str_rchipos       (listType arguments);
objectType str_rchpos        (listType arguments);
objectType str_repl          (listType arguments);
objectType str_ripos         (listType arguments);
objectType str_rpad          (listType arguments);
objectType str_rpos          (listType arguments);
objectType str_rtrim         (listType arguments);
objectType str_split         (listType arguments);
objectType str_str           (listType arguments);
objectType str_substr        (listType arguments);
objectType str_substr_fixlen (listType arguments);
objectType str_tail          (listType arguments);
objectType str_to_utf8       (listType arguments);
objectType str_trim          (listType arguments);
objectType str_up            (listType arguments);
objectType str_value         (listType arguments);
