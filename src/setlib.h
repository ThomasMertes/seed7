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

objecttype set_arrlit    (listtype arguments);
objecttype set_baselit   (listtype arguments);
objecttype set_card      (listtype arguments);
objecttype set_cmp       (listtype arguments);
objecttype set_conv      (listtype arguments);
objecttype set_cpy       (listtype arguments);
objecttype set_create    (listtype arguments);
objecttype set_destr     (listtype arguments);
objecttype set_diff      (listtype arguments);
objecttype set_elem      (listtype arguments);
objecttype set_empty     (listtype arguments);
objecttype set_eq        (listtype arguments);
objecttype set_excl      (listtype arguments);
objecttype set_ge        (listtype arguments);
objecttype set_gt        (listtype arguments);
objecttype set_hashcode  (listtype arguments);
objecttype set_iconv     (listtype arguments);
objecttype set_incl      (listtype arguments);
objecttype set_intersect (listtype arguments);
objecttype set_le        (listtype arguments);
objecttype set_lt        (listtype arguments);
objecttype set_max       (listtype arguments);
objecttype set_min       (listtype arguments);
objecttype set_ne        (listtype arguments);
objecttype set_next      (listtype arguments);
objecttype set_not_elem  (listtype arguments);
objecttype set_rand      (listtype arguments);
objecttype set_rangelit  (listtype arguments);
objecttype set_sconv     (listtype arguments);
objecttype set_symdiff   (listtype arguments);
objecttype set_union     (listtype arguments);
objecttype set_value     (listtype arguments);
