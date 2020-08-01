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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/prclib.h                                        */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Primitive actions to implement simple statements.      */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype prc_args (listtype);
objecttype prc_begin (listtype);
objecttype prc_block (listtype);
objecttype prc_block_def (listtype);
objecttype prc_case (listtype);
objecttype prc_case_def (listtype);
objecttype prc_cpy (listtype);
objecttype prc_create (listtype);
objecttype prc_decls (listtype);
objecttype prc_dynamic (listtype);
objecttype prc_env (listtype);
objecttype prc_exit (listtype);
objecttype prc_for_downto (listtype);
objecttype prc_for_to (listtype);
objecttype prc_if (listtype);
objecttype prc_if_elsif (listtype);
objecttype prc_include (listtype);
objecttype prc_local (listtype);
objecttype prc_noop (listtype);
objecttype prc_print (listtype);
objecttype prc_raise (listtype);
objecttype prc_repeat (listtype);
objecttype prc_res_begin (listtype);
objecttype prc_res_local (listtype);
objecttype prc_return (listtype);
objecttype prc_return2 (listtype);
objecttype prc_settrace (listtype);
objecttype prc_trace (listtype);
objecttype prc_varfunc (listtype);
objecttype prc_varfunc2 (listtype);
objecttype prc_while (listtype);

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
objecttype prc_env ();
objecttype prc_exit ();
objecttype prc_for_downto ();
objecttype prc_for_to ();
objecttype prc_if ();
objecttype prc_if_elsif ();
objecttype prc_include ();
objecttype prc_local ();
objecttype prc_noop ();
objecttype prc_print ();
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
