/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2010  Thomas Mertes                        */
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
/*  File: seed7/src/bstlib.h                                        */
/*  Changes: 2007, 2010  Thomas Mertes                              */
/*  Content: All primitive actions for the byte string type.        */
/*                                                                  */
/********************************************************************/

objectType bst_append   (listType arguments);
objectType bst_cat      (listType arguments);
objectType bst_cmp      (listType arguments);
objectType bst_cpy      (listType arguments);
objectType bst_create   (listType arguments);
objectType bst_destr    (listType arguments);
objectType bst_empty    (listType arguments);
objectType bst_eq       (listType arguments);
objectType bst_hashcode (listType arguments);
objectType bst_idx      (listType arguments);
objectType bst_lng      (listType arguments);
objectType bst_ne       (listType arguments);
objectType bst_parse    (listType arguments);
objectType bst_str      (listType arguments);
objectType bst_value    (listType arguments);
