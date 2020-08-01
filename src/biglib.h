/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2006  Thomas Mertes                        */
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

#ifdef ANSI_C

objecttype big_abs (listtype);
objecttype big_add (listtype);
objecttype big_bit_length (listtype);
objecttype big_clit (listtype);
objecttype big_cmp (listtype);
objecttype big_cpy (listtype);
objecttype big_create (listtype);
objecttype big_decr (listtype);
objecttype big_destr (listtype);
objecttype big_div (listtype);
objecttype big_eq (listtype);
objecttype big_gcd (listtype);
objecttype big_ge (listtype);
objecttype big_grow (listtype);
objecttype big_gt (listtype);
objecttype big_hashcode (listtype);
objecttype big_iconv (listtype);
objecttype big_incr (listtype);
objecttype big_ipow (listtype);
objecttype big_le (listtype);
objecttype big_log2 (listtype);
objecttype big_lowest_set_bit (listtype);
objecttype big_lshift (listtype);
objecttype big_lshift_assign (listtype);
objecttype big_lt (listtype);
objecttype big_mdiv (listtype);
objecttype big_minus (listtype);
objecttype big_mod (listtype);
objecttype big_mult (listtype);
objecttype big_mult_assign (listtype);
objecttype big_ne (listtype);
objecttype big_odd (listtype);
objecttype big_ord (listtype);
objecttype big_parse (listtype);
objecttype big_plus (listtype);
objecttype big_pred (listtype);
objecttype big_rand (listtype);
objecttype big_rem (listtype);
objecttype big_rshift (listtype);
objecttype big_rshift_assign (listtype);
objecttype big_sbtr (listtype);
objecttype big_shrink (listtype);
objecttype big_str (listtype);
objecttype big_succ (listtype);
objecttype big_value (listtype);

#else

objecttype big_abs ();
objecttype big_add ();
objecttype big_bit_length ();
objecttype big_clit ();
objecttype big_cmp ();
objecttype big_cpy ();
objecttype big_create ();
objecttype big_decr ();
objecttype big_destr ();
objecttype big_div ();
objecttype big_eq ();
objecttype big_gcd ();
objecttype big_ge ();
objecttype big_grow ();
objecttype big_gt ();
objecttype big_hashcode ();
objecttype big_iconv ();
objecttype big_incr ();
objecttype big_ipow ();
objecttype big_le ();
objecttype big_log2 ();
objecttype big_lowest_set_bit ();
objecttype big_lshift ();
objecttype big_lshift_assign ();
objecttype big_lt ();
objecttype big_mdiv ();
objecttype big_minus ();
objecttype big_mod ();
objecttype big_mult ();
objecttype big_mult_assign ();
objecttype big_ne ();
objecttype big_odd ();
objecttype big_ord ();
objecttype big_parse ();
objecttype big_plus ();
objecttype big_pred ();
objecttype big_rand ();
objecttype big_rem ();
objecttype big_rshift ();
objecttype big_rshift_assign ();
objecttype big_sbtr ();
objecttype big_shrink ();
objecttype big_str ();
objecttype big_succ ();
objecttype big_value ();

#endif
