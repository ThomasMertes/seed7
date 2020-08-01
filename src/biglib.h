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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
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
objecttype big_cmp (listtype);
objecttype big_cpy (listtype);
objecttype big_create (listtype);
objecttype big_decr (listtype);
objecttype big_destr (listtype);
objecttype big_div (listtype);
objecttype big_eq (listtype);
objecttype big_hashcode (listtype);
objecttype big_incr (listtype);
objecttype big_minus (listtype);
objecttype big_mult (listtype);
objecttype big_ne (listtype);
objecttype big_odd (listtype);
objecttype big_parse (listtype);
objecttype big_plus (listtype);
objecttype big_pred (listtype);
objecttype big_rem (listtype);
objecttype big_sbtr (listtype);
objecttype big_str (listtype);
objecttype big_succ (listtype);

#else

objecttype big_abs ();
objecttype big_add ();
objecttype big_cmp ();
objecttype big_cpy ();
objecttype big_create ();
objecttype big_decr ();
objecttype big_destr ();
objecttype big_div ();
objecttype big_eq ();
objecttype big_hashcode ();
objecttype big_incr ();
objecttype big_minus ();
objecttype big_mult ();
objecttype big_ne ();
objecttype big_odd ();
objecttype big_parse ();
objecttype big_plus ();
objecttype big_pred ();
objecttype big_rem ();
objecttype big_sbtr ();
objecttype big_str ();
objecttype big_succ ();

#endif
