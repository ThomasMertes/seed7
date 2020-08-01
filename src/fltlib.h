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

objecttype flt_a2tan (listtype);
objecttype flt_abs (listtype);
objecttype flt_acos (listtype);
objecttype flt_add (listtype);
objecttype flt_asin (listtype);
objecttype flt_atan (listtype);
objecttype flt_cast (listtype);
objecttype flt_ceil (listtype);
objecttype flt_cmp (listtype);
objecttype flt_cos (listtype);
objecttype flt_cpy (listtype);
objecttype flt_create (listtype);
objecttype flt_dgts (listtype);
objecttype flt_div (listtype);
objecttype flt_eq (listtype);
objecttype flt_exp (listtype);
objecttype flt_floor (listtype);
objecttype flt_ge (listtype);
objecttype flt_grow (listtype);
objecttype flt_gt (listtype);
objecttype flt_hashcode (listtype);
objecttype flt_hcos (listtype);
objecttype flt_hsin (listtype);
objecttype flt_htan (listtype);
objecttype flt_icast (listtype);
objecttype flt_iflt (listtype);
objecttype flt_ipow (listtype);
objecttype flt_isnan (listtype);
objecttype flt_le (listtype);
objecttype flt_log (listtype);
objecttype flt_log10 (listtype);
objecttype flt_lt (listtype);
objecttype flt_mcpy (listtype);
objecttype flt_minus (listtype);
objecttype flt_mult (listtype);
objecttype flt_ne (listtype);
objecttype flt_parse (listtype);
objecttype flt_plus (listtype);
objecttype flt_pow (listtype);
objecttype flt_rand (listtype);
objecttype flt_round (listtype);
objecttype flt_sbtr (listtype);
objecttype flt_shrink (listtype);
objecttype flt_sin (listtype);
objecttype flt_sqrt (listtype);
objecttype flt_str (listtype);
objecttype flt_tan (listtype);
objecttype flt_trunc (listtype);
objecttype flt_value (listtype);

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
objecttype flt_cpy ();
objecttype flt_create ();
objecttype flt_dgts ();
objecttype flt_div ();
objecttype flt_eq ();
objecttype flt_exp ();
objecttype flt_floor ();
objecttype flt_ge ();
objecttype flt_grow ();
objecttype flt_gt ();
objecttype flt_hashcode ();
objecttype flt_hcos ();
objecttype flt_hsin ();
objecttype flt_htan ();
objecttype flt_icast ();
objecttype flt_iflt ();
objecttype flt_ipow ();
objecttype flt_isnan ();
objecttype flt_le ();
objecttype flt_log ();
objecttype flt_log10 ();
objecttype flt_lt ();
objecttype flt_mcpy ();
objecttype flt_minus ();
objecttype flt_mult ();
objecttype flt_ne ();
objecttype flt_parse ();
objecttype flt_plus ();
objecttype flt_pow ();
objecttype flt_rand ();
objecttype flt_round ();
objecttype flt_sbtr ();
objecttype flt_shrink ();
objecttype flt_sin ();
objecttype flt_sqrt ();
objecttype flt_str ();
objecttype flt_tan ();
objecttype flt_trunc ();
objecttype flt_value ();

#endif
