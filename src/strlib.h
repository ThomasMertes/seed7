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
/*  File: seed7/src/strlib.h                                        */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: All primitive actions for the string type.             */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype str_append (listtype);
objecttype str_cat (listtype);
objecttype str_chipos (listtype);
objecttype str_chpos (listtype);
objecttype str_chsplit (listtype);
objecttype str_clit (listtype);
objecttype str_cmp (listtype);
objecttype str_cpy (listtype);
objecttype str_create (listtype);
objecttype str_destr (listtype);
objecttype str_elemcpy (listtype);
objecttype str_eq (listtype);
objecttype str_ge (listtype);
objecttype str_gt (listtype);
objecttype str_hashcode (listtype);
objecttype str_head (listtype);
objecttype str_idx (listtype);
objecttype str_ipos (listtype);
objecttype str_le (listtype);
objecttype str_lit (listtype);
objecttype str_lng (listtype);
objecttype str_low (listtype);
objecttype str_lpad (listtype);
objecttype str_lt (listtype);
objecttype str_mult (listtype);
objecttype str_ne (listtype);
objecttype str_pos (listtype);
objecttype str_range (listtype);
objecttype str_repl (listtype);
objecttype str_rchpos (listtype);
objecttype str_rpad (listtype);
objecttype str_rpos (listtype);
objecttype str_split (listtype);
objecttype str_str (listtype);
objecttype str_substr (listtype);
objecttype str_tail (listtype);
objecttype str_trim (listtype);
objecttype str_up (listtype);
objecttype str_value (listtype);

#else

objecttype str_append ();
objecttype str_cat ();
objecttype str_chipos ();
objecttype str_chpos ();
objecttype str_chsplit ();
objecttype str_clit ();
objecttype str_cmp ();
objecttype str_cpy ();
objecttype str_create ();
objecttype str_destr ();
objecttype str_elemcpy ();
objecttype str_eq ();
objecttype str_ge ();
objecttype str_gt ();
objecttype str_hashcode ();
objecttype str_head ();
objecttype str_idx ();
objecttype str_ipos ();
objecttype str_le ();
objecttype str_lit ();
objecttype str_lng ();
objecttype str_low ();
objecttype str_lpad ();
objecttype str_lt ();
objecttype str_mult ();
objecttype str_ne ();
objecttype str_pos ();
objecttype str_range ();
objecttype str_rchpos ();
objecttype str_repl ();
objecttype str_rpad ();
objecttype str_rpos ();
objecttype str_split ();
objecttype str_str ();
objecttype str_substr ();
objecttype str_tail ();
objecttype str_trim ();
objecttype str_up ();
objecttype str_value ();

#endif
