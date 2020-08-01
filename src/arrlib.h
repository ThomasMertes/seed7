/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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

objecttype arr_append (listtype);
objecttype arr_arrlit (listtype);
objecttype arr_arrlit2 (listtype);
objecttype arr_baselit (listtype);
objecttype arr_baselit2 (listtype);
objecttype arr_cat (listtype);
objecttype arr_conv (listtype);
objecttype arr_cpy (listtype);
objecttype arr_create (listtype);
objecttype arr_destr (listtype);
objecttype arr_empty (listtype);
objecttype arr_extend (listtype);
objecttype arr_gen (listtype);
objecttype arr_head (listtype);
objecttype arr_idx (listtype);
objecttype arr_lng (listtype);
objecttype arr_maxidx (listtype);
objecttype arr_minidx (listtype);
objecttype arr_range (listtype);
objecttype arr_remove (listtype);
objecttype arr_sort (listtype);
objecttype arr_tail (listtype);
objecttype arr_times (listtype);

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
objecttype arr_range ();
objecttype arr_remove ();
objecttype arr_sort ();
objecttype arr_tail ();
objecttype arr_times ();

#endif
