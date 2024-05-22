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

objectType arr_append       (listType arguments);
objectType arr_arrlit       (listType arguments);
objectType arr_arrlit2      (listType arguments);
objectType arr_baselit      (listType arguments);
objectType arr_baselit2     (listType arguments);
objectType arr_cat          (listType arguments);
objectType arr_conv         (listType arguments);
objectType arr_cpy          (listType arguments);
objectType arr_create       (listType arguments);
objectType arr_destr        (listType arguments);
objectType arr_empty        (listType arguments);
objectType arr_extend       (listType arguments);
objectType arr_gen          (listType arguments);
objectType arr_head         (listType arguments);
objectType arr_idx          (listType arguments);
objectType arr_insert       (listType arguments);
objectType arr_insert_array (listType arguments);
objectType arr_lng          (listType arguments);
objectType arr_maxidx       (listType arguments);
objectType arr_minidx       (listType arguments);
objectType arr_push         (listType arguments);
objectType arr_range        (listType arguments);
objectType arr_remove       (listType arguments);
objectType arr_remove_array (listType arguments);
objectType arr_sort         (listType arguments);
objectType arr_sort_reverse (listType arguments);
objectType arr_subarr       (listType arguments);
objectType arr_tail         (listType arguments);
objectType arr_times        (listType arguments);
