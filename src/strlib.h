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

objecttype str_append     (listtype arguments);
objecttype str_cat        (listtype arguments);
objecttype str_chipos     (listtype arguments);
objecttype str_chpos      (listtype arguments);
objecttype str_chsplit    (listtype arguments);
objecttype str_clit       (listtype arguments);
objecttype str_cmp        (listtype arguments);
objecttype str_cpy        (listtype arguments);
objecttype str_create     (listtype arguments);
objecttype str_destr      (listtype arguments);
objecttype str_elemcpy    (listtype arguments);
objecttype str_eq         (listtype arguments);
objecttype str_ge         (listtype arguments);
objecttype str_gt         (listtype arguments);
objecttype str_hashcode   (listtype arguments);
objecttype str_head       (listtype arguments);
objecttype str_idx        (listtype arguments);
objecttype str_ipos       (listtype arguments);
objecttype str_le         (listtype arguments);
objecttype str_lit        (listtype arguments);
objecttype str_lng        (listtype arguments);
objecttype str_low        (listtype arguments);
objecttype str_lpad       (listtype arguments);
objecttype str_lpad0      (listtype arguments);
objecttype str_lt         (listtype arguments);
objecttype str_ltrim      (listtype arguments);
objecttype str_mult       (listtype arguments);
objecttype str_ne         (listtype arguments);
objecttype str_pos        (listtype arguments);
objecttype str_poscpy     (listtype arguments);
objecttype str_push       (listtype arguments);
objecttype str_range      (listtype arguments);
objecttype str_rchipos    (listtype arguments);
objecttype str_rchpos     (listtype arguments);
objecttype str_repl       (listtype arguments);
objecttype str_ripos      (listtype arguments);
objecttype str_rpad       (listtype arguments);
objecttype str_rpos       (listtype arguments);
objecttype str_rtrim      (listtype arguments);
objecttype str_split      (listtype arguments);
objecttype str_str        (listtype arguments);
objecttype str_substr     (listtype arguments);
objecttype str_tail       (listtype arguments);
objecttype str_toutf8     (listtype arguments);
objecttype str_trim       (listtype arguments);
objecttype str_up         (listtype arguments);
objecttype str_utf8tostri (listtype arguments);
objecttype str_value      (listtype arguments);
