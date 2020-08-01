/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  Changes: 2005, 2006  Thomas Mertes                              */
/*  Content: All primitive actions for the bigInteger type.         */
/*                                                                  */
/********************************************************************/

objecttype big_abs            (listtype arguments);
objecttype big_add            (listtype arguments);
objecttype big_bit_length     (listtype arguments);
objecttype big_clit           (listtype arguments);
objecttype big_cmp            (listtype arguments);
objecttype big_conv           (listtype arguments);
objecttype big_cpy            (listtype arguments);
objecttype big_create         (listtype arguments);
objecttype big_decr           (listtype arguments);
objecttype big_destr          (listtype arguments);
objecttype big_div            (listtype arguments);
objecttype big_eq             (listtype arguments);
objecttype big_fromBStriBe    (listtype arguments);
objecttype big_fromBStriLe    (listtype arguments);
objecttype big_gcd            (listtype arguments);
objecttype big_ge             (listtype arguments);
objecttype big_grow           (listtype arguments);
objecttype big_gt             (listtype arguments);
objecttype big_hashcode       (listtype arguments);
objecttype big_iconv          (listtype arguments);
objecttype big_incr           (listtype arguments);
objecttype big_ipow           (listtype arguments);
objecttype big_le             (listtype arguments);
objecttype big_log2           (listtype arguments);
objecttype big_lowest_set_bit (listtype arguments);
objecttype big_lshift         (listtype arguments);
objecttype big_lshift_assign  (listtype arguments);
objecttype big_lt             (listtype arguments);
objecttype big_mdiv           (listtype arguments);
objecttype big_minus          (listtype arguments);
objecttype big_mod            (listtype arguments);
objecttype big_mult           (listtype arguments);
objecttype big_mult_assign    (listtype arguments);
objecttype big_ne             (listtype arguments);
objecttype big_odd            (listtype arguments);
objecttype big_ord            (listtype arguments);
objecttype big_parse          (listtype arguments);
objecttype big_parse_based    (listtype arguments);
objecttype big_plus           (listtype arguments);
objecttype big_pred           (listtype arguments);
objecttype big_rand           (listtype arguments);
objecttype big_rem            (listtype arguments);
objecttype big_rshift         (listtype arguments);
objecttype big_rshift_assign  (listtype arguments);
objecttype big_sbtr           (listtype arguments);
objecttype big_shrink         (listtype arguments);
objecttype big_str            (listtype arguments);
objecttype big_succ           (listtype arguments);
objecttype big_toBStriBe      (listtype arguments);
objecttype big_toBStriLe      (listtype arguments);
objecttype big_value          (listtype arguments);
