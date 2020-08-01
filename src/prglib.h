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

#ifdef ANSI_C

objecttype prg_analyze (listtype);
objecttype prg_cpy (listtype);
objecttype prg_create (listtype);
objecttype prg_decl_objects (listtype);
objecttype prg_destr (listtype);
objecttype prg_empty (listtype);
objecttype prg_eq (listtype);
objecttype prg_error_count (listtype);
objecttype prg_eval (listtype);
objecttype prg_exec (listtype);
objecttype prg_find (listtype);
objecttype prg_match (listtype);
objecttype prg_name (listtype);
objecttype prg_ne (listtype);
objecttype prg_prog (listtype);
objecttype prg_str_analyze (listtype);
objecttype prg_syobject (listtype);
objecttype prg_sysvar (listtype);
objecttype prg_value (listtype);

#else

objecttype prg_analyze ();
objecttype prg_cpy ();
objecttype prg_create ();
objecttype prg_decl_objects ();
objecttype prg_destr ();
objecttype prg_empty ();
objecttype prg_eq ();
objecttype prg_error_count ();
objecttype prg_eval ();
objecttype prg_exec ();
objecttype prg_find ();
objecttype prg_match ();
objecttype prg_name ();
objecttype prg_ne ();
objecttype prg_prog ();
objecttype prg_str_analyze ();
objecttype prg_syobject ();
objecttype prg_sysvar ();
objecttype prg_value ();

#endif
