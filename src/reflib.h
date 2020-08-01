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
/*  File: seed7/src/reflib.h                                        */
/*  Changes: 1991, 1992, 1993, 1994, 2004, 2007  Thomas Mertes      */
/*  Content: All primitive actions for the reference type.          */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype ref_addr          (listtype arguments);
objecttype ref_alloc         (listtype arguments);
objecttype ref_arrmaxidx     (listtype arguments);
objecttype ref_arrminidx     (listtype arguments);
objecttype ref_arrtolist     (listtype arguments);
objecttype ref_body          (listtype arguments);
objecttype ref_build         (listtype arguments);
objecttype ref_category      (listtype arguments);
objecttype ref_cat_parse     (listtype arguments);
objecttype ref_cat_str       (listtype arguments);
objecttype ref_cmp           (listtype arguments);
objecttype ref_content       (listtype arguments);
objecttype ref_conv          (listtype arguments);
objecttype ref_cpy           (listtype arguments);
objecttype ref_create        (listtype arguments);
objecttype ref_deref         (listtype arguments);
objecttype ref_eq            (listtype arguments);
objecttype ref_file          (listtype arguments);
objecttype ref_find          (listtype arguments);
objecttype ref_hashcode      (listtype arguments);
objecttype ref_hshdatatolist (listtype arguments);
objecttype ref_hshkeytolist  (listtype arguments);
objecttype ref_issymb        (listtype arguments);
objecttype ref_isvar         (listtype arguments);
objecttype ref_itftosct      (listtype arguments);
objecttype ref_line          (listtype arguments);
objecttype ref_local_consts  (listtype arguments);
objecttype ref_local_vars    (listtype arguments);
objecttype ref_mkref         (listtype arguments);
objecttype ref_name          (listtype arguments);
objecttype ref_ne            (listtype arguments);
objecttype ref_nil           (listtype arguments);
objecttype ref_num           (listtype arguments);
objecttype ref_params        (listtype arguments);
objecttype ref_prog          (listtype arguments);
objecttype ref_resini        (listtype arguments);
objecttype ref_result        (listtype arguments);
objecttype ref_scan          (listtype arguments);
objecttype ref_scttolist     (listtype arguments);
objecttype ref_select        (listtype arguments);
objecttype ref_setcategory   (listtype arguments);
objecttype ref_setparams     (listtype arguments);
objecttype ref_settype       (listtype arguments);
objecttype ref_setvar        (listtype arguments);
objecttype ref_str           (listtype arguments);
objecttype ref_symb          (listtype arguments);
objecttype ref_trace         (listtype arguments);
objecttype ref_type          (listtype arguments);
objecttype ref_value         (listtype arguments);

#else

objecttype ref_addr ();
objecttype ref_alloc ();
objecttype ref_arrmaxidx ();
objecttype ref_arrminidx ();
objecttype ref_arrtolist ();
objecttype ref_body ();
objecttype ref_build ();
objecttype ref_category ();
objecttype ref_cat_parse ();
objecttype ref_cat_str ();
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
objecttype ref_hshdatatolist ();
objecttype ref_hshkeytolist ();
objecttype ref_issymb ();
objecttype ref_isvar ();
objecttype ref_itftosct ();
objecttype ref_line ();
objecttype ref_local_consts ();
objecttype ref_local_vars ();
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
objecttype ref_setvar ();
objecttype ref_str ();
objecttype ref_symb ();
objecttype ref_trace ();
objecttype ref_type ();
objecttype ref_value ();

#endif
