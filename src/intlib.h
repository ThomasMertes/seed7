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
/*  File: seed7/src/intlib.h                                        */
/*  Changes: 1992, 1993, 1994, 2000, 2005  Thomas Mertes            */
/*  Content: All primitive actions for the integer type.            */
/*                                                                  */
/********************************************************************/

objecttype gen_destr           (listtype arguments);
objecttype int_abs             (listtype arguments);
objecttype int_add             (listtype arguments);
objecttype int_and             (listtype arguments);
objecttype int_and_assign      (listtype arguments);
objecttype int_binom           (listtype arguments);
objecttype int_bit_length      (listtype arguments);
objecttype int_bytes_be_2_uint (listtype arguments);
objecttype int_bytes_le_2_uint (listtype arguments);
objecttype int_cmp             (listtype arguments);
objecttype int_conv            (listtype arguments);
objecttype int_cpy             (listtype arguments);
objecttype int_create          (listtype arguments);
objecttype int_decr            (listtype arguments);
objecttype int_div             (listtype arguments);
objecttype int_eq              (listtype arguments);
objecttype int_fact            (listtype arguments);
objecttype int_ge              (listtype arguments);
objecttype int_grow            (listtype arguments);
objecttype int_gt              (listtype arguments);
objecttype int_hashcode        (listtype arguments);
objecttype int_incr            (listtype arguments);
objecttype int_le              (listtype arguments);
objecttype int_log2            (listtype arguments);
objecttype int_lowest_set_bit  (listtype arguments);
objecttype int_lpad0           (listtype arguments);
objecttype int_lshift          (listtype arguments);
objecttype int_lshift_assign   (listtype arguments);
objecttype int_lt              (listtype arguments);
objecttype int_mdiv            (listtype arguments);
objecttype int_mod             (listtype arguments);
objecttype int_mult            (listtype arguments);
objecttype int_mult_assign     (listtype arguments);
objecttype int_ne              (listtype arguments);
objecttype int_negate          (listtype arguments);
objecttype int_odd             (listtype arguments);
objecttype int_or              (listtype arguments);
objecttype int_ord             (listtype arguments);
objecttype int_or_assign       (listtype arguments);
objecttype int_parse           (listtype arguments);
objecttype int_plus            (listtype arguments);
objecttype int_pow             (listtype arguments);
objecttype int_pred            (listtype arguments);
objecttype int_radix           (listtype arguments);
objecttype int_RADIX           (listtype arguments);
objecttype int_rand            (listtype arguments);
objecttype int_rem             (listtype arguments);
objecttype int_rshift          (listtype arguments);
objecttype int_rshift_assign   (listtype arguments);
objecttype int_sbtr            (listtype arguments);
objecttype int_shrink          (listtype arguments);
objecttype int_sqrt            (listtype arguments);
objecttype int_str             (listtype arguments);
objecttype int_succ            (listtype arguments);
objecttype int_toBStriBe       (listtype arguments);
objecttype int_toBStriLe       (listtype arguments);
objecttype int_urshift         (listtype arguments);
objecttype int_urshift_assign  (listtype arguments);
objecttype int_value           (listtype arguments);
objecttype int_xor             (listtype arguments);
objecttype int_xor_assign      (listtype arguments);
