/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
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
/*  File: seed7/src/typlib.h                                        */
/*  Changes: 1993, 1994, 1999, 2000  Thomas Mertes                  */
/*  Content: All primitive actions for the type type.               */
/*                                                                  */
/********************************************************************/

objectType typ_addinterface (listType arguments);
objectType typ_cmp          (listType arguments);
objectType typ_cpy          (listType arguments);
objectType typ_create       (listType arguments);
objectType typ_destr        (listType arguments);
objectType typ_eq           (listType arguments);
objectType typ_func         (listType arguments);
objectType typ_gensub       (listType arguments);
objectType typ_gentype      (listType arguments);
objectType typ_hashcode     (listType arguments);
objectType typ_isdeclared   (listType arguments);
objectType typ_isderived    (listType arguments);
objectType typ_isforward    (listType arguments);
objectType typ_isfunc       (listType arguments);
objectType typ_isvarfunc    (listType arguments);
objectType typ_matchobj     (listType arguments);
objectType typ_meta         (listType arguments);
objectType typ_ne           (listType arguments);
objectType typ_num          (listType arguments);
objectType typ_result       (listType arguments);
objectType typ_str          (listType arguments);
objectType typ_value        (listType arguments);
objectType typ_varconv      (listType arguments);
objectType typ_varfunc      (listType arguments);
