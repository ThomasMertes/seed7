/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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

#ifdef ANSI_C

objecttype rfl_append (listtype);
objecttype rfl_cat (listtype);
objecttype rfl_cpy (listtype);
objecttype rfl_create (listtype);
objecttype rfl_destr (listtype);
objecttype rfl_elem (listtype);
objecttype rfl_elemcpy (listtype);
objecttype rfl_empty (listtype);
objecttype rfl_eq (listtype);
objecttype rfl_excl (listtype);
objecttype rfl_expr (listtype);
objecttype rfl_for (listtype);
objecttype rfl_head (listtype);
objecttype rfl_idx (listtype);
objecttype rfl_incl (listtype);
objecttype rfl_lng (listtype);
objecttype rfl_mklist (listtype);
objecttype rfl_ne (listtype);
objecttype rfl_not_elem (listtype);
objecttype rfl_pos (listtype);
objecttype rfl_range (listtype);
objecttype rfl_setvalue (listtype);
objecttype rfl_tail (listtype);
objecttype rfl_trace (listtype);
objecttype rfl_value (listtype);

#else

objecttype rfl_append ();
objecttype rfl_cat ();
objecttype rfl_cpy ();
objecttype rfl_create ();
objecttype rfl_destr ();
objecttype rfl_elem ();
objecttype rfl_elemcpy ();
objecttype rfl_empty ();
objecttype rfl_eq ();
objecttype rfl_excl ();
objecttype rfl_expr ();
objecttype rfl_for ();
objecttype rfl_head ();
objecttype rfl_idx ();
objecttype rfl_incl ();
objecttype rfl_lng ();
objecttype rfl_mklist ();
objecttype rfl_ne ();
objecttype rfl_not_elem ();
objecttype rfl_pos ();
objecttype rfl_range ();
objecttype rfl_setvalue ();
objecttype rfl_tail ();
objecttype rfl_trace ();
objecttype rfl_value ();

#endif
