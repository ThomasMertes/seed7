/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/setlib.h                                        */
/*  Changes: 2004  Thomas Mertes                                    */
/*  Content: All primitive actions for the set type.                */
/*                                                                  */
/********************************************************************/

objectType set_arrlit           (listType arguments);
objectType set_baselit          (listType arguments);
objectType set_card             (listType arguments);
objectType set_cmp              (listType arguments);
objectType set_conv1            (listType arguments);
objectType set_conv3            (listType arguments);
objectType set_cpy              (listType arguments);
objectType set_create           (listType arguments);
objectType set_destr            (listType arguments);
objectType set_diff             (listType arguments);
objectType set_diff_assign      (listType arguments);
objectType set_elem             (listType arguments);
objectType set_empty            (listType arguments);
objectType set_eq               (listType arguments);
objectType set_excl             (listType arguments);
objectType set_ge               (listType arguments);
objectType set_gt               (listType arguments);
objectType set_hashcode         (listType arguments);
objectType set_iconv1           (listType arguments);
objectType set_iconv3           (listType arguments);
objectType set_incl             (listType arguments);
objectType set_intersect        (listType arguments);
objectType set_intersect_assign (listType arguments);
objectType set_le               (listType arguments);
objectType set_lt               (listType arguments);
objectType set_max              (listType arguments);
objectType set_min              (listType arguments);
objectType set_ne               (listType arguments);
objectType set_next             (listType arguments);
objectType set_not_elem         (listType arguments);
objectType set_rand             (listType arguments);
objectType set_rangelit         (listType arguments);
objectType set_sconv1           (listType arguments);
objectType set_sconv3           (listType arguments);
objectType set_symdiff          (listType arguments);
objectType set_union            (listType arguments);
objectType set_union_assign     (listType arguments);
objectType set_value            (listType arguments);
