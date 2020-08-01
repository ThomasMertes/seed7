/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2007  Thomas Mertes                        */
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
/*  File: seed7/src/prclib.h                                        */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Primitive actions to implement simple statements.      */
/*                                                                  */
/********************************************************************/

objectType prc_args             (listType arguments);
objectType prc_begin            (listType arguments);
objectType prc_block            (listType arguments);
objectType prc_block_catch_all  (listType arguments);
objectType prc_block_otherwise  (listType arguments);
objectType prc_case             (listType arguments);
objectType prc_case_def         (listType arguments);
objectType prc_case_hashset     (listType arguments);
objectType prc_case_hashset_def (listType arguments);
objectType prc_cpy              (listType arguments);
objectType prc_create           (listType arguments);
objectType prc_decls            (listType arguments);
objectType prc_dynamic          (listType arguments);
objectType prc_exit             (listType arguments);
objectType prc_for_downto       (listType arguments);
objectType prc_for_downto_step  (listType arguments);
objectType prc_for_to           (listType arguments);
objectType prc_for_to_step      (listType arguments);
objectType prc_heapstat         (listType arguments);
objectType prc_hsize            (listType arguments);
objectType prc_if               (listType arguments);
objectType prc_if_elsif         (listType arguments);
objectType prc_include          (listType arguments);
objectType prc_local            (listType arguments);
objectType prc_noop             (listType arguments);
objectType prc_raise            (listType arguments);
objectType prc_repeat           (listType arguments);
objectType prc_res_begin        (listType arguments);
objectType prc_res_local        (listType arguments);
objectType prc_return           (listType arguments);
objectType prc_return2          (listType arguments);
objectType prc_settrace         (listType arguments);
objectType prc_trace            (listType arguments);
objectType prc_varfunc          (listType arguments);
objectType prc_varfunc2         (listType arguments);
objectType prc_while            (listType arguments);
