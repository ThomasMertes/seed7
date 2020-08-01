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

objecttype prg_cpy            (listtype arguments);
objecttype prg_create         (listtype arguments);
objecttype prg_destr          (listtype arguments);
objecttype prg_empty          (listtype arguments);
objecttype prg_eq             (listtype arguments);
objecttype prg_error_count    (listtype arguments);
objecttype prg_eval           (listtype arguments);
objecttype prg_exec           (listtype arguments);
objecttype prg_fil_parse      (listtype arguments);
objecttype prg_find           (listtype arguments);
objecttype prg_global_objects (listtype arguments);
objecttype prg_match          (listtype arguments);
objecttype prg_match_expr     (listtype arguments);
objecttype prg_name           (listtype arguments);
objecttype prg_ne             (listtype arguments);
objecttype prg_path           (listtype arguments);
objecttype prg_prog           (listtype arguments);
objecttype prg_str_parse      (listtype arguments);
objecttype prg_syobject       (listtype arguments);
objecttype prg_sysvar         (listtype arguments);
objecttype prg_value          (listtype arguments);
