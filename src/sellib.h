/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2025  Thomas Mertes                        */
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
/*  File: seed7/src/sellib.c                                        */
/*  Changes: 2025  Thomas Mertes                                    */
/*  Content: All primitive actions for the structElement type.      */
/*                                                                  */
/********************************************************************/

objectType sel_cmp      (listType arguments);
objectType sel_conv1    (listType arguments);
objectType sel_cpy      (listType arguments);
objectType sel_create   (listType arguments);
objectType sel_eq       (listType arguments);
objectType sel_hashcode (listType arguments);
objectType sel_ne       (listType arguments);
objectType sel_str      (listType arguments);
objectType sel_symb     (listType arguments);
objectType sel_type     (listType arguments);
