/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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

#ifdef ANSI_C

objecttype int_abs (listtype);
objecttype int_add (listtype);
objecttype int_binom (listtype);
objecttype int_bit_length (listtype);
objecttype int_cmp (listtype);
objecttype int_conv (listtype);
objecttype int_cpy (listtype);
objecttype int_create (listtype);
objecttype int_decr (listtype);
objecttype int_div (listtype);
objecttype int_eq (listtype);
objecttype int_fact (listtype);
objecttype int_ge (listtype);
objecttype int_grow (listtype);
objecttype int_gt (listtype);
objecttype int_hashcode (listtype);
objecttype int_incr (listtype);
objecttype int_le (listtype);
objecttype int_log2 (listtype);
objecttype int_lowest_set_bit (listtype);
objecttype int_lpad0 (listtype);
objecttype int_lshift (listtype);
objecttype int_lshift_assign (listtype);
objecttype int_lt (listtype);
objecttype int_mcpy (listtype);
objecttype int_mdiv (listtype);
objecttype int_minus (listtype);
objecttype int_mod (listtype);
objecttype int_mult (listtype);
objecttype int_ne (listtype);
objecttype int_odd (listtype);
objecttype int_ord (listtype);
objecttype int_parse (listtype);
objecttype int_plus (listtype);
objecttype int_pow (listtype);
objecttype int_pred (listtype);
objecttype int_rand (listtype);
objecttype int_rem (listtype);
objecttype int_rshift (listtype);
objecttype int_rshift_assign (listtype);
objecttype int_sbtr (listtype);
objecttype int_shrink (listtype);
objecttype int_sqrt (listtype);
objecttype int_str (listtype);
objecttype int_str_based (listtype);
objecttype int_succ (listtype);
objecttype int_value (listtype);

#else

objecttype int_abs ();
objecttype int_add ();
objecttype int_binom ();
objecttype int_bit_length ();
objecttype int_cmp ();
objecttype int_conv ();
objecttype int_cpy ();
objecttype int_create ();
objecttype int_decr ();
objecttype int_div ();
objecttype int_eq ();
objecttype int_fact ();
objecttype int_ge ();
objecttype int_grow ();
objecttype int_gt ();
objecttype int_hashcode ();
objecttype int_incr ();
objecttype int_le ();
objecttype int_log2 ();
objecttype int_lowest_set_bit ();
objecttype int_lpad0 ();
objecttype int_lshift ();
objecttype int_lshift_assign ();
objecttype int_lt ();
objecttype int_mcpy ();
objecttype int_mdiv ();
objecttype int_minus ();
objecttype int_mod ();
objecttype int_mult ();
objecttype int_ne ();
objecttype int_odd ();
objecttype int_ord ();
objecttype int_parse ();
objecttype int_plus ();
objecttype int_pow ();
objecttype int_pred ();
objecttype int_rand ();
objecttype int_rem ();
objecttype int_rshift ();
objecttype int_rshift_assign ();
objecttype int_sbtr ();
objecttype int_shrink ();
objecttype int_sqrt ();
objecttype int_str ();
objecttype int_str_based ();
objecttype int_succ ();
objecttype int_value ();

#endif
