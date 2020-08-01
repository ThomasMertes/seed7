/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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

#ifdef ANSI_C

objecttype flt_a2tan       (listtype arguments);
objecttype flt_abs         (listtype arguments);
objecttype flt_acos        (listtype arguments);
objecttype flt_add         (listtype arguments);
objecttype flt_asin        (listtype arguments);
objecttype flt_atan        (listtype arguments);
objecttype flt_cast        (listtype arguments);
objecttype flt_ceil        (listtype arguments);
objecttype flt_cmp         (listtype arguments);
objecttype flt_cos         (listtype arguments);
objecttype flt_cosh        (listtype arguments);
objecttype flt_cpy         (listtype arguments);
objecttype flt_create      (listtype arguments);
objecttype flt_dgts        (listtype arguments);
objecttype flt_div         (listtype arguments);
objecttype flt_div_assign  (listtype arguments);
objecttype flt_eq          (listtype arguments);
objecttype flt_exp         (listtype arguments);
objecttype flt_floor       (listtype arguments);
objecttype flt_ge          (listtype arguments);
objecttype flt_grow        (listtype arguments);
objecttype flt_gt          (listtype arguments);
objecttype flt_hashcode    (listtype arguments);
objecttype flt_icast       (listtype arguments);
objecttype flt_iconv       (listtype arguments);
objecttype flt_iflt        (listtype arguments);
objecttype flt_ipow        (listtype arguments);
objecttype flt_isnan       (listtype arguments);
objecttype flt_le          (listtype arguments);
objecttype flt_log         (listtype arguments);
objecttype flt_log10       (listtype arguments);
objecttype flt_lt          (listtype arguments);
objecttype flt_minus       (listtype arguments);
objecttype flt_mult        (listtype arguments);
objecttype flt_mult_assign (listtype arguments);
objecttype flt_ne          (listtype arguments);
objecttype flt_parse       (listtype arguments);
objecttype flt_plus        (listtype arguments);
objecttype flt_pow         (listtype arguments);
objecttype flt_rand        (listtype arguments);
objecttype flt_round       (listtype arguments);
objecttype flt_sbtr        (listtype arguments);
objecttype flt_shrink      (listtype arguments);
objecttype flt_sin         (listtype arguments);
objecttype flt_sinh        (listtype arguments);
objecttype flt_sqrt        (listtype arguments);
objecttype flt_str         (listtype arguments);
objecttype flt_tan         (listtype arguments);
objecttype flt_tanh        (listtype arguments);
objecttype flt_trunc       (listtype arguments);
objecttype flt_value       (listtype arguments);

#else

objecttype flt_a2tan ();
objecttype flt_abs ();
objecttype flt_acos ();
objecttype flt_add ();
objecttype flt_asin ();
objecttype flt_atan ();
objecttype flt_cast ();
objecttype flt_ceil ();
objecttype flt_cmp ();
objecttype flt_cos ();
objecttype flt_cosh ();
objecttype flt_cpy ();
objecttype flt_create ();
objecttype flt_dgts ();
objecttype flt_div ();
objecttype flt_div_assign ();
objecttype flt_eq ();
objecttype flt_exp ();
objecttype flt_floor ();
objecttype flt_ge ();
objecttype flt_grow ();
objecttype flt_gt ();
objecttype flt_hashcode ();
objecttype flt_icast ();
objecttype flt_iconv ();
objecttype flt_iflt ();
objecttype flt_ipow ();
objecttype flt_isnan ();
objecttype flt_le ();
objecttype flt_log ();
objecttype flt_log10 ();
objecttype flt_lt ();
objecttype flt_minus ();
objecttype flt_mult ();
objecttype flt_mult_assign ();
objecttype flt_ne ();
objecttype flt_parse ();
objecttype flt_plus ();
objecttype flt_pow ();
objecttype flt_rand ();
objecttype flt_round ();
objecttype flt_sbtr ();
objecttype flt_shrink ();
objecttype flt_sin ();
objecttype flt_sinh ();
objecttype flt_sqrt ();
objecttype flt_str ();
objecttype flt_tan ();
objecttype flt_tanh ();
objecttype flt_trunc ();
objecttype flt_value ();

#endif
