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
/*  File: seed7/src/chrlib.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions for the char type.               */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype chr_chr (listtype);
objecttype chr_cmp (listtype);
objecttype chr_conv (listtype);
objecttype chr_cpy (listtype);
objecttype chr_create (listtype);
objecttype chr_decr (listtype);
objecttype chr_eq (listtype);
objecttype chr_ge (listtype);
objecttype chr_gt (listtype);
objecttype chr_hashcode (listtype);
objecttype chr_iconv (listtype);
objecttype chr_incr (listtype);
objecttype chr_le (listtype);
objecttype chr_low (listtype);
objecttype chr_lt (listtype);
objecttype chr_ne (listtype);
objecttype chr_ord (listtype);
objecttype chr_pred (listtype);
objecttype chr_str (listtype);
objecttype chr_succ (listtype);
objecttype chr_up (listtype);
objecttype chr_value (listtype);

#else

objecttype chr_chr ();
objecttype chr_cmp ();
objecttype chr_conv ();
objecttype chr_cpy ();
objecttype chr_create ();
objecttype chr_decr ();
objecttype chr_eq ();
objecttype chr_ge ();
objecttype chr_gt ();
objecttype chr_hashcode ();
objecttype chr_iconv ();
objecttype chr_incr ();
objecttype chr_le ();
objecttype chr_low ();
objecttype chr_lt ();
objecttype chr_ne ();
objecttype chr_ord ();
objecttype chr_pred ();
objecttype chr_str ();
objecttype chr_succ ();
objecttype chr_up ();
objecttype chr_value ();

#endif
