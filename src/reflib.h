/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2015  Thomas Mertes                        */
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
/*  Changes: 1991 - 1994, 2004, 2007, 2015  Thomas Mertes           */
/*  Content: All primitive actions for the reference type.          */
/*                                                                  */
/********************************************************************/

objectType ref_addr          (listType arguments);
objectType ref_alloc         (listType arguments);
objectType ref_alloc_int     (listType arguments);
objectType ref_alloc_stri    (listType arguments);
objectType ref_alloc_var     (listType arguments);
objectType ref_arrmaxidx     (listType arguments);
objectType ref_arrminidx     (listType arguments);
objectType ref_arrtolist     (listType arguments);
objectType ref_body          (listType arguments);
objectType ref_cast          (listType arguments);
objectType ref_category      (listType arguments);
objectType ref_cat_parse     (listType arguments);
objectType ref_cat_str       (listType arguments);
objectType ref_cmp           (listType arguments);
objectType ref_content       (listType arguments);
objectType ref_conv          (listType arguments);
objectType ref_cpy           (listType arguments);
objectType ref_create        (listType arguments);
objectType ref_deref         (listType arguments);
objectType ref_eq            (listType arguments);
objectType ref_file          (listType arguments);
objectType ref_getref        (listType arguments);
objectType ref_hashcode      (listType arguments);
objectType ref_hshdatatolist (listType arguments);
objectType ref_hshkeystolist (listType arguments);
objectType ref_hshlength     (listType arguments);
objectType ref_issymb        (listType arguments);
objectType ref_istemp        (listType arguments);
objectType ref_isvar         (listType arguments);
objectType ref_itftosct      (listType arguments);
objectType ref_line          (listType arguments);
objectType ref_local_consts  (listType arguments);
objectType ref_local_vars    (listType arguments);
objectType ref_mkref         (listType arguments);
objectType ref_ne            (listType arguments);
objectType ref_nil           (listType arguments);
objectType ref_num           (listType arguments);
objectType ref_params        (listType arguments);
objectType ref_path          (listType arguments);
objectType ref_prog          (listType arguments);
objectType ref_resini        (listType arguments);
objectType ref_result        (listType arguments);
objectType ref_scan          (listType arguments);
objectType ref_scttolist     (listType arguments);
objectType ref_select        (listType arguments);
objectType ref_setcategory   (listType arguments);
objectType ref_setparams     (listType arguments);
objectType ref_settype       (listType arguments);
objectType ref_setvar        (listType arguments);
objectType ref_str           (listType arguments);
objectType ref_symb          (listType arguments);
objectType ref_trace         (listType arguments);
objectType ref_type          (listType arguments);
objectType ref_value         (listType arguments);
