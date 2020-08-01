/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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

#ifdef ANSI_C

objecttype prc_args       (listtype arguments);
objecttype prc_begin      (listtype arguments);
objecttype prc_block      (listtype arguments);
objecttype prc_block_def  (listtype arguments);
objecttype prc_case       (listtype arguments);
objecttype prc_case_def   (listtype arguments);
objecttype prc_cpy        (listtype arguments);
objecttype prc_create     (listtype arguments);
objecttype prc_decls      (listtype arguments);
objecttype prc_dynamic    (listtype arguments);
objecttype prc_exit       (listtype arguments);
objecttype prc_for_downto (listtype arguments);
objecttype prc_for_to     (listtype arguments);
objecttype prc_heapstat   (listtype arguments);
objecttype prc_hsize      (listtype arguments);
objecttype prc_if         (listtype arguments);
objecttype prc_if_elsif   (listtype arguments);
objecttype prc_include    (listtype arguments);
objecttype prc_local      (listtype arguments);
objecttype prc_noop       (listtype arguments);
objecttype prc_raise      (listtype arguments);
objecttype prc_repeat     (listtype arguments);
objecttype prc_res_begin  (listtype arguments);
objecttype prc_res_local  (listtype arguments);
objecttype prc_return     (listtype arguments);
objecttype prc_return2    (listtype arguments);
objecttype prc_settrace   (listtype arguments);
objecttype prc_trace      (listtype arguments);
objecttype prc_varfunc    (listtype arguments);
objecttype prc_varfunc2   (listtype arguments);
objecttype prc_while      (listtype arguments);

#else

objecttype prc_args ();
objecttype prc_begin ();
objecttype prc_block ();
objecttype prc_block_def ();
objecttype prc_case ();
objecttype prc_case_def ();
objecttype prc_cpy ();
objecttype prc_create ();
objecttype prc_decls ();
objecttype prc_dynamic ();
objecttype prc_exit ();
objecttype prc_for_downto ();
objecttype prc_for_to ();
objecttype prc_heapstat ();
objecttype prc_hsize ();
objecttype prc_if ();
objecttype prc_if_elsif ();
objecttype prc_include ();
objecttype prc_local ();
objecttype prc_noop ();
objecttype prc_raise ();
objecttype prc_repeat ();
objecttype prc_res_begin ();
objecttype prc_res_local ();
objecttype prc_return ();
objecttype prc_return2 ();
objecttype prc_settrace ();
objecttype prc_trace ();
objecttype prc_varfunc ();
objecttype prc_varfunc2 ();
objecttype prc_while ();

#endif
