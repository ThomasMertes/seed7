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

objecttype ref_addr (listtype);
objecttype ref_alloc (listtype);
objecttype ref_arrmaxpos (listtype);
objecttype ref_arrminpos (listtype);
objecttype ref_arrtolist (listtype);
objecttype ref_body (listtype);
objecttype ref_build (listtype);
objecttype ref_category (listtype);
objecttype ref_cmp (listtype);
objecttype ref_content (listtype);
objecttype ref_conv (listtype);
objecttype ref_cpy (listtype);
objecttype ref_create (listtype);
objecttype ref_deref (listtype);
objecttype ref_eq (listtype);
objecttype ref_file (listtype);
objecttype ref_find (listtype);
objecttype ref_hashcode (listtype);
objecttype ref_issymb (listtype);
objecttype ref_isvar (listtype);
objecttype ref_itftosct (listtype);
objecttype ref_line (listtype);
objecttype ref_local_consts (listtype);
objecttype ref_local_vars (listtype);
objecttype ref_locini (listtype);
objecttype ref_mkref (listtype);
objecttype ref_name (listtype);
objecttype ref_ne (listtype);
objecttype ref_nil (listtype);
objecttype ref_num (listtype);
objecttype ref_params (listtype);
objecttype ref_prog (listtype);
objecttype ref_resini (listtype);
objecttype ref_result (listtype);
objecttype ref_scan (listtype);
objecttype ref_scttolist (listtype);
objecttype ref_select (listtype);
objecttype ref_setcategory (listtype);
objecttype ref_setparams (listtype);
objecttype ref_settype (listtype);
objecttype ref_str (listtype);
objecttype ref_symb (listtype);
objecttype ref_trace (listtype);
objecttype ref_type (listtype);
objecttype ref_value (listtype);

#else

objecttype ref_addr ();
objecttype ref_alloc ();
objecttype ref_arrmaxpos ();
objecttype ref_arrminpos ();
objecttype ref_arrtolist ();
objecttype ref_body ();
objecttype ref_build ();
objecttype ref_category ();
objecttype ref_cmp ();
objecttype ref_content ();
objecttype ref_conv ();
objecttype ref_cpy ();
objecttype ref_create ();
objecttype ref_deref ();
objecttype ref_eq ();
objecttype ref_file ();
objecttype ref_find ();
objecttype ref_hashcode ();
objecttype ref_issymb ();
objecttype ref_isvar ();
objecttype ref_itftosct ();
objecttype ref_line ();
objecttype ref_local_consts ();
objecttype ref_local_vars ();
objecttype ref_locini ();
objecttype ref_mkref ();
objecttype ref_name ();
objecttype ref_ne ();
objecttype ref_nil ();
objecttype ref_num ();
objecttype ref_params ();
objecttype ref_prog ();
objecttype ref_resini ();
objecttype ref_result ();
objecttype ref_scan ();
objecttype ref_scttolist ();
objecttype ref_select ();
objecttype ref_setcategory ();
objecttype ref_setparams ();
objecttype ref_settype ();
objecttype ref_str ();
objecttype ref_symb ();
objecttype ref_trace ();
objecttype ref_type ();
objecttype ref_value ();

#endif
