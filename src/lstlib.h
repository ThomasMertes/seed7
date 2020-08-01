/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
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
/*  File: seed7/src/lstlib.h                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: All primitive actions for the list type.               */
/*                                                                  */
/********************************************************************/

objectType lst_cat    (listType arguments);
objectType lst_cpy    (listType arguments);
objectType lst_create (listType arguments);
objectType lst_destr  (listType arguments);
objectType lst_elem   (listType arguments);
objectType lst_empty  (listType arguments);
objectType lst_excl   (listType arguments);
objectType lst_head   (listType arguments);
objectType lst_idx    (listType arguments);
objectType lst_incl   (listType arguments);
objectType lst_lng    (listType arguments);
objectType lst_range  (listType arguments);
objectType lst_tail   (listType arguments);
