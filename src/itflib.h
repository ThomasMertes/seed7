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
/*  File: seed7/src/itflib.h                                        */
/*  Changes: 1993, 1994, 2002, 2008  Thomas Mertes                  */
/*  Content: All primitive actions for interface types.             */
/*                                                                  */
/********************************************************************/

objectType itf_cmp          (listType arguments);
objectType itf_conv2        (listType arguments);
objectType itf_cpy          (listType arguments);
objectType itf_cpy2         (listType arguments);
objectType itf_create       (listType arguments);
objectType itf_create2      (listType arguments);
objectType itf_destr        (listType arguments);
objectType itf_eq           (listType arguments);
objectType itf_hashcode     (listType arguments);
objectType itf_ne           (listType arguments);
objectType itf_select       (listType arguments);
objectType itf_to_interface (listType arguments);
