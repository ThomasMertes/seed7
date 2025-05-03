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
/*  Module: Library                                                 */
/*  File: seed7/src/reflib.h                                        */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: All primitive actions for the reference type.          */
/*                                                                  */
/********************************************************************/

objectType prg_bstri_parse     (listType arguments);
objectType prg_cpy             (listType arguments);
objectType prg_create          (listType arguments);
objectType prg_destr           (listType arguments);
objectType prg_empty           (listType arguments);
objectType prg_eq              (listType arguments);
objectType prg_error_count     (listType arguments);
objectType prg_eval            (listType arguments);
objectType prg_exec            (listType arguments);
objectType prg_fil_parse       (listType arguments);
objectType prg_get_error       (listType arguments);
objectType prg_global_objects  (listType arguments);
objectType prg_match           (listType arguments);
objectType prg_match_expr      (listType arguments);
objectType prg_name            (listType arguments);
objectType prg_ne              (listType arguments);
objectType prg_own_name        (listType arguments);
objectType prg_own_path        (listType arguments);
objectType prg_path            (listType arguments);
objectType prg_struct_elements (listType arguments);
objectType prg_str_parse       (listType arguments);
objectType prg_syobject        (listType arguments);
objectType prg_sysvar          (listType arguments);
objectType prg_value           (listType arguments);
