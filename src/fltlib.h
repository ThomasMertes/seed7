/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2016  Thomas Mertes                        */
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
/*  File: seed7/src/fltlib.h                                        */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: All primitive actions for the float type.              */
/*                                                                  */
/********************************************************************/

objectType flt_abs            (listType arguments);
objectType flt_acos           (listType arguments);
objectType flt_add            (listType arguments);
objectType flt_add_assign     (listType arguments);
objectType flt_asin           (listType arguments);
objectType flt_atan           (listType arguments);
objectType flt_atan2          (listType arguments);
objectType flt_bits2double    (listType arguments);
objectType flt_bits2single    (listType arguments);
objectType flt_cast           (listType arguments);
objectType flt_ceil           (listType arguments);
objectType flt_cmp            (listType arguments);
objectType flt_cos            (listType arguments);
objectType flt_cosh           (listType arguments);
objectType flt_cpy            (listType arguments);
objectType flt_create         (listType arguments);
objectType flt_decompose      (listType arguments);
objectType flt_dgts           (listType arguments);
objectType flt_div            (listType arguments);
objectType flt_div_assign     (listType arguments);
objectType flt_double2bits    (listType arguments);
objectType flt_eq             (listType arguments);
objectType flt_exp            (listType arguments);
objectType flt_floor          (listType arguments);
objectType flt_ge             (listType arguments);
objectType flt_gt             (listType arguments);
objectType flt_hashcode       (listType arguments);
objectType flt_icast          (listType arguments);
objectType flt_iconv1         (listType arguments);
objectType flt_iconv3         (listType arguments);
objectType flt_ipow           (listType arguments);
objectType flt_isnan          (listType arguments);
objectType flt_isnegativezero (listType arguments);
objectType flt_le             (listType arguments);
objectType flt_log            (listType arguments);
objectType flt_log10          (listType arguments);
objectType flt_log2           (listType arguments);
objectType flt_lshift         (listType arguments);
objectType flt_lt             (listType arguments);
objectType flt_mult           (listType arguments);
objectType flt_mult_assign    (listType arguments);
objectType flt_ne             (listType arguments);
objectType flt_negate         (listType arguments);
objectType flt_parse1         (listType arguments);
objectType flt_plus           (listType arguments);
objectType flt_pow            (listType arguments);
objectType flt_rand           (listType arguments);
objectType flt_round          (listType arguments);
objectType flt_rshift         (listType arguments);
objectType flt_sbtr           (listType arguments);
objectType flt_sbtr_assign    (listType arguments);
objectType flt_sci            (listType arguments);
objectType flt_sin            (listType arguments);
objectType flt_single2bits    (listType arguments);
objectType flt_sinh           (listType arguments);
objectType flt_sqrt           (listType arguments);
objectType flt_str            (listType arguments);
objectType flt_tan            (listType arguments);
objectType flt_tanh           (listType arguments);
objectType flt_trunc          (listType arguments);
objectType flt_value          (listType arguments);
