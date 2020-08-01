/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2006  Thomas Mertes                        */
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
/*  File: seed7/src/arrlib.h                                        */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: All primitive actions for array types.                 */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype arr_append   (listtype arguments);
objecttype arr_arrlit   (listtype arguments);
objecttype arr_arrlit2  (listtype arguments);
objecttype arr_baselit  (listtype arguments);
objecttype arr_baselit2 (listtype arguments);
objecttype arr_cat      (listtype arguments);
objecttype arr_conv     (listtype arguments);
objecttype arr_cpy      (listtype arguments);
objecttype arr_create   (listtype arguments);
objecttype arr_destr    (listtype arguments);
objecttype arr_empty    (listtype arguments);
objecttype arr_extend   (listtype arguments);
objecttype arr_gen      (listtype arguments);
objecttype arr_head     (listtype arguments);
objecttype arr_idx      (listtype arguments);
objecttype arr_lng      (listtype arguments);
objecttype arr_maxidx   (listtype arguments);
objecttype arr_minidx   (listtype arguments);
objecttype arr_push     (listtype arguments);
objecttype arr_range    (listtype arguments);
objecttype arr_remove   (listtype arguments);
objecttype arr_sort     (listtype arguments);
objecttype arr_tail     (listtype arguments);
objecttype arr_times    (listtype arguments);

#else

objecttype arr_append ();
objecttype arr_arrlit ();
objecttype arr_arrlit2 ();
objecttype arr_baselit ();
objecttype arr_baselit2 ();
objecttype arr_cat ();
objecttype arr_conv ();
objecttype arr_cpy ();
objecttype arr_create ();
objecttype arr_destr ();
objecttype arr_empty ();
objecttype arr_extend ();
objecttype arr_gen ();
objecttype arr_head ();
objecttype arr_idx ();
objecttype arr_lng ();
objecttype arr_maxidx ();
objecttype arr_minidx ();
objecttype arr_push ();
objecttype arr_range ();
objecttype arr_remove ();
objecttype arr_sort ();
objecttype arr_tail ();
objecttype arr_times ();

#endif
