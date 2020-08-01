/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2014  Thomas Mertes                        */
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
/*  File: seed7/src/biglib.h                                        */
/*  Changes: 2005, 2006, 2013, 2014  Thomas Mertes                  */
/*  Content: All primitive actions for the bigInteger type.         */
/*                                                                  */
/********************************************************************/

objectType big_abs            (listType arguments);
objectType big_add            (listType arguments);
objectType big_add_assign     (listType arguments);
objectType big_bit_length     (listType arguments);
objectType big_cmp            (listType arguments);
objectType big_conv           (listType arguments);
objectType big_cpy            (listType arguments);
objectType big_create         (listType arguments);
objectType big_decr           (listType arguments);
objectType big_destr          (listType arguments);
objectType big_div            (listType arguments);
objectType big_eq             (listType arguments);
objectType big_fromBStriBe    (listType arguments);
objectType big_fromBStriLe    (listType arguments);
objectType big_gcd            (listType arguments);
objectType big_ge             (listType arguments);
objectType big_gt             (listType arguments);
objectType big_hashcode       (listType arguments);
objectType big_iconv          (listType arguments);
objectType big_incr           (listType arguments);
objectType big_ipow           (listType arguments);
objectType big_le             (listType arguments);
objectType big_log10          (listType arguments);
objectType big_log2           (listType arguments);
objectType big_lowest_set_bit (listType arguments);
objectType big_lshift         (listType arguments);
objectType big_lshift_assign  (listType arguments);
objectType big_lt             (listType arguments);
objectType big_mdiv           (listType arguments);
objectType big_mod            (listType arguments);
objectType big_mult           (listType arguments);
objectType big_mult_assign    (listType arguments);
objectType big_ne             (listType arguments);
objectType big_negate         (listType arguments);
objectType big_odd            (listType arguments);
objectType big_ord            (listType arguments);
objectType big_parse          (listType arguments);
objectType big_parse_based    (listType arguments);
objectType big_plus           (listType arguments);
objectType big_pred           (listType arguments);
objectType big_radix          (listType arguments);
objectType big_RADIX          (listType arguments);
objectType big_rand           (listType arguments);
objectType big_rem            (listType arguments);
objectType big_rshift         (listType arguments);
objectType big_rshift_assign  (listType arguments);
objectType big_sbtr           (listType arguments);
objectType big_sbtr_assign    (listType arguments);
objectType big_str            (listType arguments);
objectType big_succ           (listType arguments);
objectType big_toBStriBe      (listType arguments);
objectType big_toBStriLe      (listType arguments);
objectType big_value          (listType arguments);
