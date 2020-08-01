/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2015  Thomas Mertes                        */
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
/*  Changes: 1992 - 1994, 2000, 2005, 2013 - 2015  Thomas Mertes    */
/*  Content: All primitive actions for the integer type.            */
/*                                                                  */
/********************************************************************/

objectType gen_destr             (listType arguments);
objectType int_abs               (listType arguments);
objectType int_add               (listType arguments);
objectType int_add_assign        (listType arguments);
objectType int_binom             (listType arguments);
objectType int_bit_length        (listType arguments);
objectType int_bytes_be_2_int    (listType arguments);
objectType int_bytes_be_2_uint   (listType arguments);
objectType int_bytes_be_signed   (listType arguments);
objectType int_bytes_be_unsigned (listType arguments);
objectType int_bytes_le_2_int    (listType arguments);
objectType int_bytes_le_2_uint   (listType arguments);
objectType int_bytes_le_signed   (listType arguments);
objectType int_bytes_le_unsigned (listType arguments);
objectType int_cmp               (listType arguments);
objectType int_cpy               (listType arguments);
objectType int_create            (listType arguments);
objectType int_decr              (listType arguments);
objectType int_div               (listType arguments);
objectType int_eq                (listType arguments);
objectType int_fact              (listType arguments);
objectType int_ge                (listType arguments);
objectType int_gt                (listType arguments);
objectType int_hashcode          (listType arguments);
objectType int_iconv1            (listType arguments);
objectType int_iconv3            (listType arguments);
objectType int_incr              (listType arguments);
objectType int_le                (listType arguments);
objectType int_log10             (listType arguments);
objectType int_log2              (listType arguments);
objectType int_lowest_set_bit    (listType arguments);
objectType int_lpad0             (listType arguments);
objectType int_lshift            (listType arguments);
objectType int_lshift_assign     (listType arguments);
objectType int_lt                (listType arguments);
objectType int_mdiv              (listType arguments);
objectType int_mod               (listType arguments);
objectType int_mult              (listType arguments);
objectType int_mult_assign       (listType arguments);
objectType int_ne                (listType arguments);
objectType int_negate            (listType arguments);
objectType int_odd               (listType arguments);
objectType int_parse1            (listType arguments);
objectType int_plus              (listType arguments);
objectType int_pow               (listType arguments);
objectType int_pred              (listType arguments);
objectType int_radix             (listType arguments);
objectType int_RADIX             (listType arguments);
objectType int_rand              (listType arguments);
objectType int_rem               (listType arguments);
objectType int_rshift            (listType arguments);
objectType int_rshift_assign     (listType arguments);
objectType int_sbtr              (listType arguments);
objectType int_sbtr_assign       (listType arguments);
objectType int_sqrt              (listType arguments);
objectType int_str               (listType arguments);
objectType int_succ              (listType arguments);
objectType int_value             (listType arguments);
